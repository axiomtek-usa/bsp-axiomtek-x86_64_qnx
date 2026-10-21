/*
 * Copyright (c) 2025, BlackBerry Limited. All rights reserved.
 *
 * BlackBerry Limited and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation. Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from BlackBerry Limited
 * is strictly prohibited.
 */

#include "proto.h"
i2c_status_t mec170_i2c_xfer(void *const hdl,
                             void *const send_buf, const uint32_t send_len,
                             void *const recv_buf, const uint32_t recv_len)
{
    return mec170_i2c_smbus_xfer(hdl, READWRITE, send_buf, send_len, recv_buf, recv_len);
}
