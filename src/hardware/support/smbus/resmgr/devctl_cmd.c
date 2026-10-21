/*
 * Copyright (c) 2021,2025 BlackBerry Limited.
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

/* Maximum 7-bit SMBus address */
#define MAX_SMBUS_ADDR      (0x7F)


/**
 * @brief Validate the SMBus command and options
 *
 * This function verifies that the format of the SMBus command complies with
 * the SMBus specification.
 *
 * @param ctp       resource manager context
 * @param hdr       devctl header
 * @param msg       devctl message buffer
 * @return int      errno
 */
static int verify_args(const resmgr_context_t *const ctp,
    const smbus_devctl_cmd_t *const hdr, const io_devctl_t *const msg)
{
    /* Check buffer sizes */
    if ((sizeof(*hdr) + sizeof(*msg) + hdr->send_len) > ctp->size) {
        SMBUS_LOG_ERROR("Incomplete resource manager buffer for sent data!");
        return EBADMSG;
    }
    if ((sizeof(*hdr) + hdr->send_len) > msg->i.nbytes) {
        SMBUS_LOG_ERROR("Incomplete message buffer for sent data!");
        return EBADMSG;
    }
    if ((sizeof(*hdr) + sizeof(*msg) + hdr->recv_len) > ctp->msg_max_size) {
        SMBUS_LOG_ERROR("Receive resource manager buffer too small!");
        return EBADMSG;
    }
    if ((sizeof(*hdr) + hdr->recv_len) > msg->i.nbytes) {
        SMBUS_LOG_ERROR("Receive message buffer too small!");
        return EBADMSG;
    }

    /* Check command validity */
    if ((hdr->cmd < SMBUS_CMD_FIRST_ENTRY) || (hdr->cmd > SMBUS_CMD_LAST_ENTRY)) {
        SMBUS_LOG_ERROR("SMBus command %d is invalid!", hdr->cmd);
        return EINVAL;
    }

    /* Check access validity */
    if ((hdr->rw != SMBUS_RW_WRITE) && (hdr->rw != SMBUS_RW_READ)) {
        SMBUS_LOG_ERROR("SMBus read/write flag %d in invalid!", hdr->rw);
        return EINVAL;
    }

    /* Check slave address validity */
    if (hdr->addr > MAX_SMBUS_ADDR) {
        SMBUS_LOG_ERROR("Slave address 0x%02x is out of range!", hdr->addr);
        return EINVAL;
    }

    switch(hdr->cmd) {
        case SMBUS_CMD_QUICK:
            /*
             * Name:  Quick Command
             * Rules: Ignore send_len, recv_len, and data
             */
            break;

        case SMBUS_CMD_BYTE:
            /*
             * Name: Send/Receive Byte
             * Rules:
             *  - Write
             *      send_len >= 1
             *      Ignore recv_len
             *  - Read
             *      Ignore send_len
             *      recv_len >= 1
             */
            if (hdr->rw == SMBUS_RW_WRITE) {
                if (hdr->send_len < 1) {
                    SMBUS_LOG_ERROR("Invalid send_len for command SMBUS_CMD_BYTE");
                    return EINVAL;
                }
            }
            if (hdr->rw == SMBUS_RW_READ) {
                if (hdr->recv_len < 1) {
                    SMBUS_LOG_ERROR("Invalid recv_len for command SMBUS_CMD_BYTE");
                    return EINVAL;
                }
            }
            break;

        case SMBUS_CMD_BYTE_DATA:
            /*
             * Name: Read/Write Byte
             * Rules:
             *  - Write
             *      send_len >= 2
             *      Ignore recv_len
             *  - Read
             *      send_len = 1
             *      recv_len >= 1
             */
            if (hdr->rw == SMBUS_RW_WRITE) {
                if (hdr->send_len < 2) {
                    SMBUS_LOG_ERROR("Invalid send_len for command SMBUS_CMD_BYTE_DATA");
                    return EINVAL;
                }
            }
            if (hdr->rw == SMBUS_RW_READ) {
                if (hdr->send_len != 1) {
                    SMBUS_LOG_ERROR("Invalid send_len for command SMBUS_CMD_BYTE_DATA");
                    return EINVAL;
                }
                if (hdr->recv_len < 1) {
                    SMBUS_LOG_ERROR("Invalid recv_len for command SMBUS_CMD_BYTE_DATA");
                    return EINVAL;
                }
            }
            break;

        case SMBUS_CMD_WORD_DATA:
            /*
             * Name: Read/Write Word
             * Rules:
             *  - Write
             *      send_len >= 3
             *      Ignore recv_len
             *  - Read
             *      send_len = 1
             *      recv_len >= 2
             */
            if (hdr->rw == SMBUS_RW_WRITE) {
                if (hdr->send_len < 3) {
                    SMBUS_LOG_ERROR("Invalid send_len for command SMBUS_CMD_WORD_DATA");
                    return EINVAL;
                }
            }
            if (hdr->rw == SMBUS_RW_READ) {
                if (hdr->send_len != 1) {
                    SMBUS_LOG_ERROR("Invalid send_len for command SMBUS_CMD_WORD_DATA");
                    return EINVAL;
                }
                if (hdr->recv_len < 2) {
                    SMBUS_LOG_ERROR("Invalid recv_len for command SMBUS_CMD_WORD_DATA");
                    return EINVAL;
                }
            }
            break;

        case SMBUS_CMD_BLOCK:
            /*
             * Name: Block Read/Write
             * Rules:
             *  - Write
             *      send_len >= 3 (1 (command) + 1 (count) + n (data))
             *      Ignore recv_len
             *  - Read
             *      send_len = 1
             *      recv_len >= 2 (1 (count) + n (data))
             */
            if (hdr->rw == SMBUS_RW_WRITE) {
                if (hdr->send_len < 3) {
                    SMBUS_LOG_ERROR("Invalid send_len for command SMBUS_CMD_BLOCK");
                    return EINVAL;
                }
            }
            if (hdr->rw == SMBUS_RW_READ) {
                if (hdr->send_len != 1) {
                    SMBUS_LOG_ERROR("Invalid send_len for command SMBUS_CMD_BLOCK");
                    return EINVAL;
                }
                if (hdr->recv_len < 2) {
                    SMBUS_LOG_ERROR("Invalid recv_len for command SMBUS_CMD_BLOCK");
                    return EINVAL;
                }
            }
            break;

        case SMBUS_CMD_I2C_READ:
            /*
             * Name: I2C Read
             * Rules:
             *  - Write
             *      Unsupported
             *  - Read
             *      send_len = 1
             *      recv_len >= 1
             */
            if (hdr->rw == SMBUS_RW_WRITE) {
                SMBUS_LOG_ERROR("SMBUS_RW_WRITE not supported by command SMBUS_CMD_I2C_READ");
                return ENOTSUP;
            }
            if (hdr->rw == SMBUS_RW_READ) {
                if (hdr->send_len != 1) {
                    SMBUS_LOG_ERROR("Invalid send_len for command SMBUS_CMD_I2C_READ");
                    return EINVAL;
                }
                if (hdr->recv_len < 1) {
                    SMBUS_LOG_ERROR("Invalid recv_len for command SMBUS_CMD_I2C_READ");
                    return EINVAL;
                }
            }
            break;

        case SMBUS_CMD_BLOCK_PROC:  /* FALLTHRU */
        case SMBUS_CMD_PROC_CALL:   /* FALLTHRU */
        default:
            SMBUS_LOG_ERROR("SMBus command %d is unsupported!", hdr->cmd);
            return ENOTSUP;
    }
    return EOK;
}

/**
 * @brief Implement the devctl SMBus command functionality
 *
 * This function implements the DCMD_SMBUS_CMD devctl.
 *
 * @param ctp       Resource manager context
 * @param msg       Message container
 * @param ocb       Open control block
 * @return int      errno
 */
int smbus_devctl_cmd(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
{
    int err;
    smbus_dev_t *dev;
    smbus_devctl_cmd_t *hdr;
    uint8_t *data = NULL;
    size_t len = 0;

    /* Get device pointer */
    dev = ocb->attr;
    /* Get device data pointer */
#if _NTO_VERSION <= 710
    hdr = _DEVCTL_DATA(msg->i);
#else
    hdr = _IO_INPUT_PAYLOAD(msg);
#endif

    /* Verify command parameters */
    err = verify_args(ctp, hdr, msg);
    if (err != EOK) {
        return err;
    }

    /* Locate data buffer */
    len = msg->i.nbytes - sizeof(*hdr);
    data = (uint8_t*)hdr + sizeof(*hdr);

    /* Execute the SMBus command */
    err = smbus_hw_cmd(dev->hdl, hdr, len, data);
    if (err != EOK) {
        return err;
    }

    /*
     * Set IOVs
     * Return the devctl header for all the commands in iov[0].
     * Only return the data buffer in iov[1] for read transaction.
     */
    SETIOV(&ctp->iov[0], msg, sizeof(*msg) + sizeof(*hdr));
    memset(&msg->o, 0, sizeof(msg->o));

    if (hdr->recv_len > 0) {
        SETIOV(&ctp->iov[1], data, hdr->recv_len);
        msg->o.ret_val = (int32_t)hdr->recv_len;
        msg->o.nbytes = (uint32_t)(sizeof(*hdr) + hdr->recv_len);
    }
    return (hdr->recv_len > 0) ? _RESMGR_NPARTS(2) : _RESMGR_NPARTS(1);
}

