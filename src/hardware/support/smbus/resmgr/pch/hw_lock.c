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


/**
 * @brief Lock the hardware semaphore in the SMBus controller
 *
 * The function allows to get exclusive access to the controller through
 * the hardware semaphore. The functions tries to lock the semaphore until
 * it is locked successfully, or until the timeout is reached.
 * The lock feature can be disabled by setting the global variable
 * 'lock_timeout' to zero. The timeout used by the function can be passed via
 * the parameter 'timeout_us'. If the the parameter 'timeout_us' is null, then
 * the fuction uses the global variable 'lock_timeout' instead.
 *
 * Note that the semaphore is locked by reading the status register, and
 * unlocked by writing a 1 in the 'In Use Status' bit in the status register.
 *
 * @param ctrl          SMBus controller context
 * @param timeout_us    Time spent trying to lock the semaphore (in microseconds)
 * @return int          errno
 */
int hw_lock(smbus_ctrl_t *const ctrl, unsigned long timeout_us)
{
    uint8_t status;

    /* Sanity check */
    assert(ctrl != NULL);

    /* Check if the lock feature was disabled during driver initialization */
    if (ctrl->lock_timeout == 0) {
        SMBUS_LOG_DEBUG("HW lock is disabled.");
        return EOK;
    }

    /* If passed timeout is null use the global setting */
    if (timeout_us == 0) {
        timeout_us = ctrl->lock_timeout;
    }

    do {
        /* Try to lock the HW semaphore */
        status = hw_reg_read8(ctrl, SMBUS_HSTS);
        if ((status & SMBUS_HSTS_IUS) == 0) {
            return EOK;
        }

        nanospin_ns(1000); /* 1 us */
    } while (timeout_us--);

    return EBUSY;
}

/**
 * @brief Release the hardware semaphore in the SMBus controller
 *
 * Note that the semaphore is locked by reading the status register, and
 * unlocked by writing a 1 in the 'In Use Status' bit in the status register.
 * This function must NOT read the status register to avoid accidentally locking
 * the semaphore again.
 *
 * @param ctrl      SMBus controller context
 * @return int      errno
 */
int hw_unlock(smbus_ctrl_t *const ctrl)
{
    /* Sanity check */
    assert(ctrl != NULL);

    /* Check if the lock feature was disabled during driver initialization */
    if (ctrl->lock_timeout == 0) {
        return EOK;
    }

    /* Release the HW semaphore */
    hw_reg_write8(ctrl, SMBUS_HSTS, SMBUS_HSTS_IUS);
    return EOK;
}

