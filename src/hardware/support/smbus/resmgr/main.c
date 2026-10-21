/*
 * Copyright (c) 2018,2021,2025 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <signal.h>
#include <login.h>
#include <secpol/secpol.h>
#include <secpol/ids.h>

#include "smbus_internal.h"
#include "log.h"


/* Maximum string length that the resource manager path can hold */
#define RESMGR_PATH_MAX     (sizeof(SMBUS_RESMGR_PATH) + 10)

/* Local flag to notify dispatcher loop to terminate */
static volatile unsigned done = 0;
/* Local pointer to the dispatch context for the exit handler */
static dispatch_context_t *local_ctp = NULL;


/**
 * @brief Handle signal that terminates the main dispatcher loop
 *
 * @param signo     Signal ID that triggered execution of the handler
 */
static void exit_handler(int __attribute__((unused)) signo)
{
    /* Set the flag to terminate the main loop */
    done = 1;
    if (local_ctp != NULL) {
        dispatch_unblock(local_ctp);
    }
}

/**
 * @brief Attach the exit handler to the specified signal
 *
 * @param signo     Signal ID that will trigger execution of the handler
 * @param func      Function of the signal handler
 * @return int      errno
 */
static int register_signal_handler(const int signo, void (*func)(int))
{
    int err;
    struct sigaction sa = {0};

    /* Assign the exit handler */
    sa.sa_handler = func;
    err = sigaction(signo, &sa, NULL);
    if (err < 0) {
        return errno;
    }
    return EOK;
}

/**
 * @brief Main routine for the SMBus resource manager
 *
 * @param argc      Number of command line arguments
 * @param argv      Array of command line arguments
 * @return int      EXIT_SUCCESS on success, EXIT_FAILURE otherwise
 */
int main(const int argc, char *argv[])
{
    int err;
    smbus_dev_t dev;
    smbus_options_t opts;
    resmgr_connect_funcs_t connect_funcs;
    resmgr_io_funcs_t io_funcs;
    resmgr_attr_t attr;
    char resmgr_path[RESMGR_PATH_MAX];

    /* Process program arguments */
    if (smbus_options(&opts, argc, argv) != 0) {
        SMBUS_LOG_ERROR("Error while parsing options!");
        err = EXIT_FAILURE;
        goto fail;
    }

    /* Initialize error logging */
    smbus_log_init(&opts.verbose);

    SMBUS_LOG_INFO("Starting SMBus resource manager...");
    memset(&dev, 0, sizeof(dev));
    dev.opts = &opts;

    /* Create dispatcher */
    dev.dpp = dispatch_create_channel(-1, DISPATCH_FLAG_NOLOCK);
    if (dev.dpp == NULL) {
        SMBUS_LOG_ERROR("Failed to create dispatcher!");
        err = EXIT_FAILURE;
        goto fail;
    }

    /* Initialize the default IO function */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
                     _RESMGR_IO_NFUNCS, &io_funcs);

    /* Override IO functions with local definitions */
    io_funcs.devctl = smbus_devctl;
    io_funcs.lock = NULL;
    io_funcs.read = NULL;
    io_funcs.write = NULL;

    /* Set access permission */
    iofunc_attr_init(&dev.hdr, S_IFCHR | 0664, NULL, NULL);

    /* Format resource manager path */
    (void)snprintf(resmgr_path, sizeof(resmgr_path), SMBUS_RESMGR_PATH"%u", opts.unit);

    /* Configure the resource manager attributes */
    memset(&attr, 0, sizeof(attr));     /* Set attributes to defaults */
    attr.nparts_max = 2;                /* Use 2 IOVs */

    /* Attach to the process manager and publish IO functions */
    dev.id = resmgr_attach(dev.dpp, &attr, resmgr_path, _FTYPE_ANY, 0,
                           &connect_funcs, &io_funcs, &dev);
    if (dev.id < 0) {
        SMBUS_LOG_ERROR("Failed to attach to pathname %s: %s", resmgr_path, strerror(errno));
        err = EXIT_FAILURE;
        goto fail;
    }

    /* Get the dispatch context */
    dev.ctp = dispatch_context_alloc(dev.dpp);
    if (dev.ctp == NULL) {
        SMBUS_LOG_ERROR("Failed to allocate dispatch context: %s", strerror(errno));
        err = EXIT_FAILURE;
        goto fail;
    }
    /* Save context for the exit handler */
    local_ctp = dev.ctp;

    /* Register the exit handler against SIGTERM (kill or slay) and SIGINT (Ctrl-C) */
    err = register_signal_handler(SIGTERM, exit_handler);
    if (err != EOK) {
        SMBUS_LOG_ERROR("Failed to register signal handler: %s", strerror(err));
        err = EXIT_FAILURE;
        goto fail;
    }
    err = register_signal_handler(SIGINT, exit_handler);
    if (err != EOK) {
        SMBUS_LOG_ERROR("Failed to register signal handler: %s", strerror(err));
        err = EXIT_FAILURE;
        goto fail;
    }

    /* Execute platform-specific initialization */
    if (smbus_hw_init(&dev) != 0) {
        SMBUS_LOG_ERROR("Failed to initialize the SMBus");
        err = EXIT_FAILURE;
        goto fail;
    }

    /* Detach process as a daemon */
    err = procmgr_daemon(EXIT_SUCCESS,
        (unsigned int)(PROCMGR_DAEMON_NOCLOSE | ((opts.verbose < 0) ? PROCMGR_DAEMON_NODEVNULL : 0)));
    if (err < 0) {
        SMBUS_LOG_ERROR("Failed to detach process as a daemon: %s", strerror(errno));
        err = EXIT_FAILURE;
        goto fail;
    }

    /* Switch process to the proper security type */
    secpol_transition_type(NULL, NULL, 0);

    /* Set the UID/GID specified via the command line */
    if (opts.uid_gid != NULL) {
        err = set_ids_from_arg(opts.uid_gid);
        if (err != 0) {
            SMBUS_LOG_ERROR("Failed to set the specified User/Group ID %s: %s",
                opts.uid_gid, strerror(errno));
            err = EXIT_FAILURE;
            goto fail;
        }
        /* Update to the final UID/GID */
        dev.hdr.uid = getuid();
        dev.hdr.gid = getgid();
    }

    /* Main loop: waiting for messages */
    while (done == 0) {
        if (dispatch_block(dev.ctp) != NULL) {
            dispatch_handler(dev.ctp);
        } else {
            SMBUS_LOG_INFO("Dispatcher stopped: %s", strerror(errno));
            done = 1;
        }
    }
    err = EXIT_SUCCESS;

 fail:
    SMBUS_LOG_INFO("SMBus resource manager has terminated.");
    return err;
}

