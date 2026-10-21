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
 * @brief Control the Packet Error Check (PEC) feature
 *
 * @param ctrl      SMBus controller context
 * @param enable    Enable/disable PEC (1=enable, 0=disable)
 * @return int      errno
 */
int hw_pec(smbus_ctrl_t *const ctrl, const unsigned int enable)
{
    /* Sanity check */
    assert(ctrl != NULL);

    if (enable) {
        /* Unsupported feature for now! */
        return ENOTSUP;
    } else {
        hw_reg_write_mask8(ctrl, SMBUS_HCTL, SMBUS_HCTL_PEC_EN, 0);
    }
    return EOK;
}

