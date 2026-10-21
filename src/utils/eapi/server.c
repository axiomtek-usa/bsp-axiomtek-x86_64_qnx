/*
* Copyright (c) 2026, BlackBerry Limited. All rights reserved.
*
* BlackBerry Limited and its licensors retain all intellectual property and
* proprietary rights in and to this software and related documentation. Any
* use, reproduction, disclosure or distribution of this software and related
* documentation without an express license agreement from BlackBerry Limited
* is strictly prohibited.
*/

#include "eapi.h"

/* EAPI Device global structure, holds io def and permissions */
static eapi_dev_t dev;

/**
 * Allocate our own custom ocb to cache per ocb checks.
 *
 * @param ctp           Dispatch context
 * @param attr          iofunc attribute structure
 * @return              void.
 */
static RESMGR_OCB_T * eapi_ocb_calloc(resmgr_context_t *ctp, iofunc_attr_t *attr)
{
    eapi_ocb_t *ocb = calloc(1, sizeof(*ocb));
    if (ocb == NULL) {
        return NULL;
    }

    return (RESMGR_OCB_T *)ocb;
}

/**
 * free ocb.
 *
 * @param ocb   Open Control Block structure.
 *
 * @return      void.
 */
static void eapi_ocb_free(RESMGR_OCB_T *ocb)
{
    free(ocb);
}

/**
 *  @brief             EAPI resmgr r/w mode check function.
 *  @param ocb         Open Control Block structure.
 *  @param mode        client mode.
 *
 *  @return            EOK for success, err for failure.
 */
static inline int32_t eapi_check_access(eapi_ocb_t *eocb, uint32_t mode)
{
    int32_t status;

    if ((eocb->ocb.ioflag & (int32_t)mode) != (int32_t)mode) {
        eapi_slogf(_SLOG_ERROR, _SLOG_ERROR,
                       "Bad perm: %s failed", __func__);
        status = EPERM;
    } else {
        status = EOK;
    }
    return status;
}

/**
 *  @brief             devctl handling.
 *  @param ctp         Resource manager context info.
 *  @param msg         devctl message.
 *  @param eocb_i      Open Control Block structure.
 *
 *  @return            _RESMGR_NOREPLY for success, errno for failure.
 */
static int32_t io_devctl(resmgr_context_t *ctp, io_devctl_t *msg,  RESMGR_OCB_T *eocb_i)
{
    int32_t status = 0;
    void *dptr;
    size_t nbytes;
    int32_t dev_status = 0;
    eapi_ocb_t *eocb = (eapi_ocb_t *)eocb_i;
    eapi_ops_g *eapi_ops = &dev.eapi_ops;
    status = iofunc_devctl_default(ctp, msg, &eocb->ocb);
    if (status != _RESMGR_DEFAULT) {
        return status;
    }

    nbytes = 0;
    dptr = _IO_INPUT_PAYLOAD(msg);
    switch (msg->i.dcmd) {
    case DCMD_EAPI_GET_ABI: {
        status = eapi_check_integrity_abi(ctp, msg, NULL);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_access(eocb, _IO_FLAG_RD);
        if (status != EOK) {
            return status;
        }

        uint32_t *abi = (uint32_t *)dptr;

        abi[0] = EAPI_MAJOR_VER;
        abi[1] = EAPI_MINOR_VER;

        eocb->abi_ok = true;

        status = MsgReply(ctp->rcvid, EOK, &msg->o, sizeof(msg->o) + sizeof(uint32_t) * 2u);
        if (status != EOK) {
            eapi_slogf(dev.verbose, _SLOG_ERROR,
              "MsgReply failed: %s\n", strerror(errno));
        }
        return _RESMGR_NOREPLY;
    }
    default:
        /* ABI must be queried first */
        if (!eocb->abi_ok) {
            eapi_slogf(dev.verbose, _SLOG_ERROR,
                       "Versioning not verified [EAPI] ver:%u.%u",
                       EAPI_MAJOR_VER, EAPI_MINOR_VER);
            return EINVAL;
        }
        break;
    }

    switch (msg->i.dcmd) {
    case DCMD_EAPI_I2C_RD_WR:
        status = eapi_check_integrity_i2c(ctp, msg, NULL);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_access(eocb, _IO_FLAG_RD | _IO_FLAG_WR);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_perm(ctp, dev.abids[EAPI_ABIL_IDX_I2C_XFER], &eocb->abils_allowed.i2c);
        if (status != EOK) {
            return status;
        }

        dev_status = eapi_ops->access_i2c_cmd(&dev, dptr, &nbytes);
        break;

    case DCMD_EAPI_GETVER:
        status = eapi_check_integrity_version(ctp, NULL, NULL);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_access(eocb, _IO_FLAG_RD);
        if (status != EOK) {
            return status;
        }

        dev_status = eapi_ops->get_firmware_version_cmd(&dev, dptr, &nbytes);
        break;

    case DCMD_EAPI_GET_CAPS: {
        status = eapi_check_integrity_caps(ctp, NULL, NULL);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_access(eocb, _IO_FLAG_RD);
        if (status != EOK) {
            return status;
        }

        dev_status = eapi_ops->device_get_caps_cmd(&dev, dptr, &nbytes);
        break;
    }

    case DCMD_EAPI_WDT_CONFIG:
        status = eapi_check_integrity_watchdog(ctp, NULL, NULL);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_access(eocb, _IO_FLAG_WR);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_perm(ctp, dev.abids[EAPI_ABIL_IDX_WDT_CONFIG],
                                            &eocb->abils_allowed.wdt_config);
        if (status != EOK) {
            return status;
        }

        dev_status = eapi_ops->wdt_config_cmd(&dev, dptr, &nbytes);
        break;

    case DCMD_EAPI_WDT_START:
        status = eapi_check_integrity_watchdog(ctp, NULL, NULL);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_access(eocb, _IO_FLAG_WR);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_perm(ctp, dev.abids[EAPI_ABIL_IDX_WDT_CONFIG],
                                            &eocb->abils_allowed.wdt_config);
        if (status != EOK) {
            return status;
        }

        dev_status = eapi_ops->wdt_start_cmd(&dev, dptr, &nbytes);
        break;

    case DCMD_EAPI_WDT_TRIGGER:
        status = eapi_check_access(eocb, _IO_FLAG_WR);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_perm(ctp, dev.abids[EAPI_ABIL_IDX_WDT_REFRESH],
                                            &eocb->abils_allowed.wdt_refresh);
        if (status != EOK) {
            return status;
        }

        dev_status = eapi_ops->wdt_refresh_cmd(&dev, NULL, NULL);
        break;

    case DCMD_EAPI_WDT_STOP:
        status = eapi_check_access(eocb, _IO_FLAG_WR);
        if (status != EOK) {
            return status;
        }

        status = eapi_check_perm(ctp, dev.abids[EAPI_ABIL_IDX_WDT_CONFIG],
                                            &eocb->abils_allowed.wdt_config);
        if (status != EOK) {
            return status;
        }

        dev_status = eapi_ops->wdt_stop_cmd(&dev, dptr, &nbytes);
        break;

    default:

        return ENOTTY;
    }

    msg->o.ret_val = dev_status;
    status = MsgReply(ctp->rcvid, EOK, msg, sizeof(*msg) + nbytes);
    if (status != EOK) {
            eapi_slogf(dev.verbose, _SLOG_ERROR,
              "MsgReply failed: %s\n", strerror(errno));
    }
    return _RESMGR_NOREPLY;
}

/**
 *  @brief             Main function.
 *  @param argc        Cmndline arg count
 *  @param argv        Cmndline arg list
 *  @return            EXIT_SUCCESS on success or EXIT_FAILURE for failures.
 */


int32_t main(int32_t argc, char **argv)
{
    int32_t ret;
    iofunc_attr_t ioattr;
    eapi_resmgr_t eapi_resmgr;

    dev.priority = -1;
    uint32_t loglevel = _SLOG_ERROR;

    ret = ThreadCtl_r(_NTO_TCTL_IO_LEVEL, (void*)_NTO_IO_LEVEL_1);
    if (ret != EOK) {
        eapi_slogf(loglevel, _SLOG_ERROR,
              "%s: ThreadCtl failed\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ret = eapi_parseopts(&dev, argc, argv);
    if (ret != EOK) {
        exit(EXIT_FAILURE);
    }

    ret = eapi_chip_init(&dev, &dev.eapi_ops);
    if (ret != EOK) {
        eapi_slogf(loglevel, _SLOG_ERROR, "eapi entry init failed\n");
        exit(EXIT_FAILURE);
    }

    if (dev.priority > 0) {
        if (pthread_setschedprio(pthread_self(), dev.priority) != EOK) {
            eapi_slogf(loglevel, _SLOG_ERROR, "Can't change priority");
            exit(EXIT_FAILURE);
        }
    }

    secpol_file_t *const secpol_hdl =
        secpol_open(NULL, (uint32_t)SECPOL_USE_AS_DEFAULT);

    ret = eapi_perms_init(&dev);
    if (ret != EOK) {
        eapi_slogf(loglevel, _SLOG_ERROR, "eapi perm init failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    iofunc_mount_t eapi_mount;
    /* set up the mount structure */
    ret = iofunc_mount_init(&eapi_mount, sizeof(eapi_mount));
    if (ret != EOK) {
        eapi_slogf(loglevel, _SLOG_ERROR,
              "setting up the mount structure failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    iofunc_funcs_t eapi_mount_funcs;

    eapi_mount_funcs.nfuncs = (uint32_t)_IOFUNC_NFUNCS;
    eapi_mount_funcs.ocb_calloc = &eapi_ocb_calloc;
    eapi_mount_funcs.ocb_free = &eapi_ocb_free;

    eapi_mount.conf |= (uint32_t)IOFUNC_PC_ACL;

    eapi_mount.funcs = &eapi_mount_funcs;

    eapi_resmgr.dpp = dispatch_create_channel(-1, DISPATCH_FLAG_NOLOCK);
    if (eapi_resmgr.dpp == NULL) {
        eapi_slogf(loglevel, _SLOG_ERROR,
              "dispatch_create_channel failed%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    eapi_slogf(dev.verbose, _SLOG_INFO, "Starting EAPI Resmgr\n");

    /* Setup the default handlers, then take over I/O msg */
    iofunc_func_init((uint32_t)_RESMGR_CONNECT_NFUNCS, &eapi_resmgr.connect_funcs,
                     (uint32_t)_RESMGR_IO_NFUNCS, &eapi_resmgr.io_funcs);

    eapi_resmgr.io_funcs.devctl = &io_devctl;
    eapi_resmgr.io_funcs.lock = NULL;
    eapi_resmgr.io_funcs.read = NULL;
    eapi_resmgr.io_funcs.write = NULL;
    eapi_resmgr.io_funcs.unblock = NULL;
    eapi_resmgr.io_funcs.lock_ocb = NULL;
    eapi_resmgr.io_funcs.unlock_ocb = NULL;

    /* Initialize device description structure, allow everyone to open for read/write */
    iofunc_attr_init(&ioattr, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IFNAM, NULL, NULL);

    ioattr.mount = &eapi_mount;

    /* Set path name */
    const char *name = NULL;
    switch (dev.chip_name) {
    case CHIP_MEC170X:
    case CHIP_F81804:
        name = EAPI_DEVICE_PATH;
        break;
    /* In case a chip need its own node name, make it here */
    case CHIP_UNKNOWN:
    /* falls through */
    default:
        exit(EXIT_FAILURE);
    }

    eapi_resmgr.resmgr_id =
    secpol_resmgr_attach(NULL, eapi_resmgr.dpp,
                         NULL, name, _FTYPE_ANY, 0, &eapi_resmgr.connect_funcs,
                         &eapi_resmgr.io_funcs, &ioattr, NULL);
    if (eapi_resmgr.resmgr_id == -1) {
        eapi_slogf(loglevel, _SLOG_ERROR,
              "secpol resmgr attach failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    eapi_resmgr.ctp = dispatch_context_alloc(eapi_resmgr.dpp);
    if (eapi_resmgr.ctp == NULL) {
        eapi_slogf(loglevel, _SLOG_ERROR,
              "dispatch_context_alloc failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (procmgr_daemon(EXIT_SUCCESS,
                       PROCMGR_DAEMON_NOCLOSE | PROCMGR_DAEMON_NODEVNULL) == -1) {
        eapi_slogf(loglevel, _SLOG_ERROR, "procmgr_daemon failed ");
        exit(EXIT_FAILURE);
    }
#if 0
    /*
    * If the hardware module reports EAPI_CAPS_INTERRUPT_MODE, spawn a dedicated
    * thread using the handler provided by the module. The handler is
    * responsible for attaching and handling the interrupt.
    * Note: No hardware currently requires this path.
    */

    int32_t caps = 0;
    size_t nbytes = 0;
    if (dev.eapi_ops.device_get_caps_cmd(&dev, &caps, &nbytes) == EOK) {
        if(caps & EAPI_CAPS_INTERRUPT_MODE) {
            /* Setup Interrupt */
            dev.mode = EAPI_CAPS_INTERRUPT_MODE;

            eapi_slogf(dev.verbose, _SLOG_INFO,
                    "Module started in interrupt mode\n");
        }
    }
#endif

    /* Transition security type before entering main loop */
    if (secpol_transition_type(NULL, NULL, 0) == -1) {
        eapi_slogf(dev.verbose, _SLOG_INFO,
              "secpol_transition_type failed: %s\n", strerror(errno));
    }

    secpol_close(secpol_hdl);

    /* Server listening */
    while (true) {
        if (dispatch_block(eapi_resmgr.ctp) != NULL) {
            if (dispatch_handler(eapi_resmgr.ctp) == -1) {
                break;
            }
        }
        else {
            eapi_slogf(loglevel, _SLOG_ERROR,
                  " dispatch_block failed: %s\n", strerror(errno));
            break;
        }
    }

    return EXIT_FAILURE;
}
