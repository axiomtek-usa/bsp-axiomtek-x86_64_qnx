/*
 * Copyright (c) 2015, 2022, 2025 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 */


#include "proto.h"

#define _i2c_slogf(dev, level, fmt, ...) \
    (i2c_slogf((dev)->verbosity, (level), (fmt), ##__VA_ARGS__))

i2c_status_t baytrail_i2c_xfer(void *const hdl,
                          void *const send_buf, const uint32_t send_len,
                          void *const recv_buf, const uint32_t recv_len)
{
    baytrail_dev_t  *dev = hdl;
    i2c_status_t    ret = I2C_STATUS_ERROR;

    dev->txlen  = send_len;
    dev->rxlen  = recv_len;
    dev->xlen   = dev->txlen + dev->rxlen;
    dev->wrlen  = 0;
    dev->rdlen  = 0;
    dev->txbuf  = send_buf;
    dev->rxbuf  = recv_buf;
    dev->status = 0;

    /* Active I2C bus */
    if (baytrail_i2c_bus_active(dev) != 0) {
        return I2C_STATUS_BUSY;
    }

    /* Clear and enable interrupts */
    (void)i2c_reg_read32(dev, DW_IC_CLR_INTR);
    i2c_reg_write32(dev, DW_IC_INTR_MASK, DW_IC_INTR_DEFAULT_MASK);

    /* Wait for transaction complete event */
    ret = baytrail_i2c_wait_complete(dev);

    /* Disabled interrupts */
    i2c_reg_write32(dev, DW_IC_INTR_MASK, 0);

    /* Disable the I2C adapter */
    if (baytrail_i2c_enable(dev, DW_IC_ENABLE_STATUS_DISABLE) != 0) {
        return I2C_STATUS_BUSY;
    }

    return ret;
}
