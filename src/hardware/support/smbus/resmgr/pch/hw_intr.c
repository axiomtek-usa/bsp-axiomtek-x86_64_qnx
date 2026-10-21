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


/*
 * Period (in nanoseconds) for polling mode.
 * The maximum SMBus bus frequency is 100kHz, which represents a period of 10 us.
 */
#define INTR_POLL_DELAY_NS  (10000ULL)

/*
 * Transaction timeout for polling mode.
 * The SMBus specification dictates a transaction timeout of 35ms.
 * The worst case scenario is a total of 3500 periods for a maximum bus freqency
 * of 100kHz. (i.e.: 35ms = 10us x 3500 = INTR_POLL_DELAY_NS x INTR_POLL_TIMEOUT)
 */
#define INTR_POLL_TIMEOUT   (SMBUS_TRANSACTION_TIMEOUT_MS * 100ULL)

/**
 * @brief Transaction Handler Completion Status
 */
typedef enum _intr_completion_t {
    INTR_COMPLETION_DONE,           /* Transaction completed */
    INTR_COMPLETION_NEXT,           /* Request/wait for next data byte/interrupt */
    INTR_COMPLETION_NEXT_RELOAD,    /* Reload timeout and request/wait for next data byte */
} intr_completion_t;


/**
 * @brief Process the controller status and interrupt status
 *
 * @param ctrl          SMBus controller context
 * @param rw            SMBus read/write access
 * @param size          Remaining size of data to transfer
 * @param data          Pointer to the remaining data to transfer
 * @param completion    Pointer to the interrupt handler completion status
 * @return int          errno
 */
static int intr_status(smbus_ctrl_t *const ctrl, const smbus_rw_t rw,
    const size_t size, uint8_t *data, intr_completion_t *completion)
{
    int err = EOK;
    uint8_t status = 0;

    if (completion == NULL) {
        return EINVAL;
    }

    status = hw_reg_read8(ctrl, SMBUS_HSTS);

    /*
     * Handle controller status
     */
    if (status & (SMBUS_HSTS_DERR | SMBUS_HSTS_BERR | SMBUS_HSTS_FAIL)) {
        /* Catch errors on the bus */

        SMBUS_LOG_DEBUG("ERR - Controller status on error: 0x%02x", status);
        if (status & SMBUS_HSTS_DERR) {
            SMBUS_LOG_WARNING("Failed on a device error!");
        }
        if (status & SMBUS_HSTS_BERR) {
            SMBUS_LOG_WARNING("Failed on a transaction collision!");
        }
        if (status & SMBUS_HSTS_FAIL) {
            SMBUS_LOG_WARNING("Failed on a failed bus transaction!");
        }

        *completion = INTR_COMPLETION_DONE;
        err = EIO;

    } else if (status & SMBUS_HSTS_INTR) {
        /* Catch successful completion of the last command */

        SMBUS_LOG_DEBUG("INTR - Controller status: 0x%02x", status);
        *completion = INTR_COMPLETION_DONE;

    } else if (status & SMBUS_HSTS_BDS) {
        /* Catch if a byte was successful received or sent (block transactions only) */

        SMBUS_LOG_DEBUG("BDS - Controller status: 0x%02x", status);

        if ((data != NULL) && (size > 0)) {

            if (rw == SMBUS_RW_READ) {
                *data = hw_reg_read8(ctrl, SMBUS_HDB);
            } else {
                hw_reg_write8(ctrl, SMBUS_HDB, *data);
            }
            /* Notify controller that the next byte is the last */
            if (size <= 2) {
                SMBUS_LOG_DEBUG("BDS - Notify controller for last byte");
                hw_reg_write_mask8(ctrl, SMBUS_HCTL, SMBUS_HCTL_LAST_BYTE,
                    SMBUS_HCTL_LAST_BYTE);
            }
        }

        /* Clear the 'Byte Done Status' flag */
        hw_reg_write_mask8(ctrl, SMBUS_HSTS, SMBUS_HSTS_BDS, SMBUS_HSTS_BDS);

        /* Request to reset timeout and wait for the next byte */
        *completion = INTR_COMPLETION_NEXT_RELOAD;

    } else if (status & SMBUS_HSTS_HBSY) {
        /* Catch if the controller is still busy */

        /* Request to wait for the next byte/interrupt */
        *completion = INTR_COMPLETION_NEXT;

    } else {
        /* Catch invalid controller state */

        /* Fatal error, abort the transaction */
        SMBUS_LOG_ERROR("UNKNOWN - Invalid controller state: 0x%02x", status);
        *completion = INTR_COMPLETION_DONE;
        err = EIO;
    }

    return err;
}

/**
 * @brief Interrupt-based transaction handler
 *
 * @param ctrl      SMBus controller context
 * @param rw        SMBus read/write access
 * @param size      Number of bytes of data to transfer
 * @param data      Pointer to the data to transfer
 * @return int      errno
 */
int hw_intr_irq(smbus_ctrl_t *const ctrl, const smbus_rw_t rw,
    size_t size, uint8_t *data)
{
    int err = EOK;
    uint64_t timeout;
    intr_completion_t completion = INTR_COMPLETION_NEXT;

    /* Sanity checks */
    assert(ctrl != NULL);
    assert(ctrl->interrupt.event_id != 0);

    /* Enable interrupt and start transaction */
    hw_reg_write_mask8(ctrl, SMBUS_HCTL, SMBUS_HCTL_START | SMBUS_HCTL_INTREN,
        SMBUS_HCTL_START | SMBUS_HCTL_INTREN);

    /* Wait for transaction completion, errors, or timeout */
    while ((completion != INTR_COMPLETION_DONE) && (err == EOK)) {

        /* Arm the interrupt timeout */
        timeout = SMBUS_TRANSACTION_TIMEOUT_NS;
        TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_INTR, NULL, &timeout, NULL);

        /* Wait for interrupts, or timeout */
        if (InterruptWait(0, NULL) == EOK) {

            /* Check transaction status */
            err = intr_status(ctrl, rw, size, data, &completion);
            if (err == EOK) {
                if (completion == INTR_COMPLETION_NEXT_RELOAD) {
                    /*
                     * Doing a block transaction:
                     *  - The last byte was transferred successfully
                     *  - Move the data pointer to the next byte to transfer
                     */
                    if (size > 0) {
                        size--;
                        data++;
                    }
                }
            }

            if (InterruptUnmask(ctrl->interrupt.irq, ctrl->interrupt.event_id) < 0) {
                SMBUS_LOG_ERROR("Error while unmasking the interrupt 0x%02x - %s",
                    ctrl->interrupt.irq, strerror(errno));
                err = EIO;
            }

        } else {
            SMBUS_LOG_ERROR("Error while waiting on interrupt 0x%02x - %s",
                ctrl->interrupt.irq, strerror(errno));
            err = EIO;
        }
    }
    return err;
}

/**
 * @brief Polling-based transaction handler
 *
 * @param ctrl      SMBus controller context
 * @param rw        SMBus read/write access
 * @param size      Number of bytes of data to transfer
 * @param data      Pointer to the data to transfer
 * @return int      errno
 */
int hw_intr_poll(smbus_ctrl_t *const ctrl, const smbus_rw_t rw,
    size_t size, uint8_t *data)
{
    int err = EBUSY;
    uint64_t timeout = INTR_POLL_TIMEOUT;
    intr_completion_t completion = INTR_COMPLETION_NEXT;

    /* Sanity check */
    assert(ctrl != NULL);

    /* Disable interrupt and start transaction */
    hw_reg_write_mask8(ctrl, SMBUS_HCTL, SMBUS_HCTL_START | SMBUS_HCTL_INTREN,
        SMBUS_HCTL_START);

    /* Wait for transaction completion, or timeout */
    while ((completion != INTR_COMPLETION_DONE) && (timeout > 0)) {

        nanospin_ns(INTR_POLL_DELAY_NS);
        timeout--;

        /* Check transaction status */
        err = intr_status(ctrl, rw, size, data, &completion);
        if (err == EOK) {
            if (completion == INTR_COMPLETION_NEXT_RELOAD) {
                /*
                 * Doing a block transaction:
                 *  - The last byte was transferred successfully
                 *  - Reload the timeout for the next transaction
                 *  - Move the data pointer to the next byte to transfer
                 */
                timeout = INTR_POLL_TIMEOUT;
                if (size > 0) {
                    size--;
                    data++;
                }
            }
        }
    }

    if (timeout == 0) {
        err = ETIMEDOUT;
    }
    return err;
}

