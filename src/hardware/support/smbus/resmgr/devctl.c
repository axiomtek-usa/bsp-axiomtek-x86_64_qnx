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

#include <devctl.h>
#include <sys/iomsg.h>

#include "smbus_internal.h"
#include "log.h"


/**
 * @brief Check message validity
 * 
 * @param ctp       Resource manager context
 * @param cmd       Devctl command
 * @return int      errno
 */
static int io_devctl_sanity_checks(const resmgr_context_t *const ctp, const int cmd)
{
    /* Reject combine messages */
    if (ctp->offset != 0) {
        return EBADMSG;
    }

    switch (cmd) {

        case DCMD_SMBUS_CMD:
            /* Reject incomplete message */
            if((sizeof(io_devctl_t) + sizeof(smbus_devctl_cmd_t)) > ctp->size) {
                return EBADMSG;
            }
            break;

        /* Disallow unsupported commands */
        default:
            return ENOSYS;
    }
    return EOK;
}

/**
 * @brief Check command permissions against the device open flag
 *
 * @param cmd       Devctl command
 * @param payload   Devctl payload
 * @param oflag     Device open flag
 * @return int      EPERM on permission violation, EOK otherwise
 */
static int io_devctl_permissions(const int cmd, const void *const payload, const int oflag)
{
    switch (cmd) {

        case DCMD_SMBUS_CMD:
            if ((((smbus_devctl_cmd_t*)payload)->rw == SMBUS_RW_READ)
                && ((oflag & _IO_FLAG_RD) == 0)) {
                return EPERM;
            } else if ((((smbus_devctl_cmd_t*)payload)->rw == SMBUS_RW_WRITE)
                        && ((oflag & _IO_FLAG_WR) == 0)) {
                return EPERM;
            } else {
                ;   /* No actions */
            }
            break;

        /* Disallow unsupported commands */
        default:
            return EPERM;
    }
    return EOK;
}

/**
 * @brief Implement the devctl functionality for the resource manager
 *
 * @param ctp       Resource manager context
 * @param msg       Message container
 * @param ocb       Open control block
 * @return int      errno
 */
int smbus_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
{
    int err;
    const void *payload;

    /* Execute the default devctl routine */
    err = iofunc_devctl_default(ctp, msg, ocb);
    if (err != _RESMGR_DEFAULT) {
        return err;
    }

    /* Get device data pointer */
#if _NTO_VERSION <= 710
    payload = _DEVCTL_DATA(msg->i);
#else
    payload = _IO_INPUT_PAYLOAD(msg);
#endif

    /* Sanity checks on resource manager message */
    err = io_devctl_sanity_checks(ctp, msg->i.dcmd);
    if (err != EOK) {
        SMBUS_LOG_ERROR("Invalid resource manager message for command 0x%x", msg->i.dcmd);
        return err;
    }

    /* Verify that the device was opened with the proper permissions */
    err = io_devctl_permissions(msg->i.dcmd, payload, ocb->ioflag);
    if (err != EOK) {
        SMBUS_LOG_ERROR("Permission denied for command 0x%x", msg->i.dcmd);
        return EPERM;
    }

    /* Process the devctl command */
    switch (msg->i.dcmd) {

        case DCMD_SMBUS_CMD:
            return smbus_devctl_cmd(ctp, msg, ocb);

        default:
            /* Unsupported command */
            return ENOSYS;
    }

    /* NOTREACHED */
    return ENOSYS;
}

