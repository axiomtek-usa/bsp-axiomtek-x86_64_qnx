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

#include "smbus_internal.h"
#include "log.h"
#include "hw.h"
#include "reg.h"


/* Delay before returning to normal operation after force-stopping a transaction */
#define CMD_KILL_DELAY      (1000000ULL)    /* 1 ms */


/**
 * @brief Force stop the current SMBus transaction
 *
 * @param ctrl      SMBus controller context
 */
static void cmd_kill(smbus_ctrl_t *const ctrl)
{
    hw_reg_write_mask8(ctrl, SMBUS_HCTL, SMBUS_HCTL_KILL, SMBUS_HCTL_KILL);
    nanospin_ns(CMD_KILL_DELAY);
    hw_reg_write_mask8(ctrl, SMBUS_HCTL, SMBUS_HCTL_KILL, 0);
}

/**
 * @brief Reset the controller registers to a known state
 *
 * @param ctrl      SMBus controller context
 */
static void cmd_reset_regs(smbus_ctrl_t *const ctrl)
{
    hw_reg_write8(ctrl, SMBUS_HSTS,
        (uint8_t)(hw_reg_read8(ctrl, SMBUS_HSTS) & ~SMBUS_HSTS_IUS));
    hw_reg_write8(ctrl, SMBUS_HCTL, 0);
    hw_reg_write8(ctrl, SMBUS_HCMD, 0);
    hw_reg_write8(ctrl, SMBUS_HD0, 0);
    hw_reg_write8(ctrl, SMBUS_HD1, 0);
    hw_reg_write8(ctrl, SMBUS_HDB, 0);
    hw_reg_write8(ctrl, SMBUS_AUXC, 0);
    hw_reg_write_mask8(ctrl, SMBUS_AUXS, SMBUS_AUXS_CRCE, 0);
}

/**
 * @brief Execute the specified SMBus transaction
 *
 * @param hdl       SMBus controller context
 * @param hdr       SMBus transaction command
 * @param len       Number of bytes to transfer (send or/and receive)
 * @param data      Buffer of the data to transfer (send or/and receive)
 * @return int      errno
 */
int smbus_hw_cmd(void *const hdl, smbus_devctl_cmd_t *hdr, const size_t len, uint8_t *data)
{
    int err = EOK;
    size_t xfer_size = 0;
    uint8_t *xfer_data = NULL;
    smbus_ctrl_t *const ctrl = hdl;

    /* Sanity check */
    if ((hdl == NULL) || (hdr == NULL)) {
        return EINVAL;
    }
    if ((hdr->cmd != SMBUS_CMD_QUICK) && ((len == 0) || (data == NULL))) {
        return EINVAL;
    }

    /* Check for the unsupported PEC option */
    if (hdr->opt & SMBUS_OPT_PEC_EN) {
        SMBUS_LOG_ERROR("The PEC feature is not supported!");
        return ENOTSUP;
    }

    /* Lock the hardware semaphore */
    err = hw_lock(ctrl, 0);
    if (err != EOK) {
        SMBUS_LOG_ERROR("Failed to lock the hardware semaphore!");
        return err;
    }

    /* Kill current transaction if controller is busy */
    if (hw_reg_read8(ctrl, SMBUS_HSTS) & SMBUS_HSTS_HBSY) {
        cmd_kill(ctrl);
    }

    /* Reset controller registers */
    cmd_reset_regs(ctrl);

    /* Set slave address */
    hw_reg_write_mask8(ctrl, SMBUS_TSA, SMBUS_TSA_ADDR_MASK, (uint8_t)SMBUS_TSA_ADDR(hdr->addr));
    /* Set transaction access */
    hw_reg_write_mask8(ctrl, SMBUS_TSA, SMBUS_TSA_RW_MASK, (uint8_t)SMBUS_TSA_RW(hdr->rw));
    /* Set SMBus command */
    hw_reg_write8(ctrl, SMBUS_HCTL, (uint8_t)SMBUS_HCTL_SMB_CMD(hdr->cmd));
    /* Set I2C transaction mode */
    hw_i2c(ctrl, hdr->opt & SMBUS_OPT_I2C_EN);

    /* Prepare the data registers */
    switch(hdr->cmd) {

        case SMBUS_CMD_QUICK:
            /* Nothing to do - use solely TSA register */
            break;

        case SMBUS_CMD_BYTE:
            if (hdr->rw == SMBUS_RW_WRITE) {
                /* Use TSA and HCMD registers */
                hw_reg_write8(ctrl, SMBUS_HCMD, data[0]);   /* Command Code */
            }
            break;

        case SMBUS_CMD_BYTE_DATA:
            /* Use TSA, HCMD and HD0 registers */
            hw_reg_write8(ctrl, SMBUS_HCMD, data[0]);       /* Command Code */
            if (hdr->rw == SMBUS_RW_WRITE) {
                hw_reg_write8(ctrl, SMBUS_HD0, data[1]);    /* Data Byte */
            }
            break;

        case SMBUS_CMD_WORD_DATA:
            /* Use TSA, HCMD, HD0 and HD1 registers */
            hw_reg_write8(ctrl, SMBUS_HCMD, data[0]);       /* Command Code */
            if (hdr->rw == SMBUS_RW_WRITE) {
                hw_reg_write8(ctrl, SMBUS_HD0, data[1]);    /* Data Byte Low */
                hw_reg_write8(ctrl, SMBUS_HD1, data[2]);    /* Data Byte High */
            }
            break;

        case SMBUS_CMD_BLOCK:
            /* Use TSA, HCMD, HD0 and HDB registers */
            hw_reg_write8(ctrl, SMBUS_HCMD, data[0]);       /* Command Code */
            if (hdr->rw == SMBUS_RW_WRITE) {
                hw_reg_write8(ctrl, SMBUS_HD0, data[1]);    /* Byte Count */
                hw_reg_write8(ctrl, SMBUS_HDB, data[2]);    /* Data Byte 1 */
                xfer_data = &data[3];                       /* Data Byte 2-n */
                xfer_size = hdr->send_len - 3;
            } else {
                xfer_data = &data[1];
                xfer_size = hdr->recv_len - 1;
            }
            break;

        case SMBUS_CMD_I2C_READ:
            /* Use TSA, HD1 and HDB registers */
            hw_reg_write8(ctrl, SMBUS_HD1, data[0]);        /* Register offset */
            xfer_data = &data[0];
            xfer_size = hdr->recv_len;
            break;

        case SMBUS_CMD_PROC_CALL:   /* FALLTHRU */
        case SMBUS_CMD_BLOCK_PROC:  /* FALLTHRU */
        default:
            err = ENOTSUP;
            goto fail;
    }

    /* Reset the return buffer on read access */
    if ((hdr->cmd != SMBUS_CMD_QUICK) && (hdr->rw == SMBUS_RW_READ)) {
        memset(data, 0, hdr->recv_len);
    }

    /* Select the transaction handler */
    if (ctrl->interrupt.event_id > 0) {
        err = hw_intr_irq(ctrl, hdr->rw, xfer_size, xfer_data);
    } else {
        err = hw_intr_poll(ctrl, hdr->rw, xfer_size, xfer_data);
    }
    if (err != EOK) {
        SMBUS_LOG_ERROR("Transaction handler failed! - %s", strerror(err));
        err = EIO;
        goto fail;
    }

    /* Populate the data buffer from the data register */
    switch(hdr->cmd) {
        case SMBUS_CMD_QUICK:
            /* Nothing to do */
            break;

        case SMBUS_CMD_BYTE:        /* FALLTHRU */
        case SMBUS_CMD_BYTE_DATA:
            if (hdr->rw == SMBUS_RW_READ) {
                data[0] = hw_reg_read8(ctrl, SMBUS_HD0);    /* Data Byte */
            }
            break;

        case SMBUS_CMD_WORD_DATA:
            if (hdr->rw == SMBUS_RW_READ) {
                data[0] = hw_reg_read8(ctrl, SMBUS_HD0);    /* Data Byte Low */
                data[1] = hw_reg_read8(ctrl, SMBUS_HD1);    /* Data Byte High */
            }
            break;

        case SMBUS_CMD_BLOCK:
            if (hdr->rw == SMBUS_RW_READ) {
                data[0] = hw_reg_read8(ctrl, SMBUS_HD0);    /* Byte Count */
                if (hdr->recv_len > 1) {
                    data[hdr->recv_len - 1] = hw_reg_read8(ctrl, SMBUS_HDB);
                }
            }
            break;

        case SMBUS_CMD_I2C_READ:
            if (hdr->recv_len > 1) {
                data[hdr->recv_len - 1] = hw_reg_read8(ctrl, SMBUS_HDB);
            }
            break;

        /* NOTREACHED */
        case SMBUS_CMD_PROC_CALL:   /* FALLTHRU */
        case SMBUS_CMD_BLOCK_PROC:  /* FALLTHRU */
        default:
            err = ENOTSUP;
            goto fail;
    }

 fail:
    /* Disable I2C transaction mode if it was enabled */
    if (hdr->opt & SMBUS_OPT_I2C_EN) {
        hw_i2c(ctrl, 0);
    }

    /* Unlock the hardware semaphore */
    hw_unlock(ctrl);

    return err;
}

