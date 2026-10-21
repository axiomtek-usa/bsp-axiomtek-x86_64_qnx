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
 * @brief Control the I2C transaction mode feature
 *
 * @param ctrl      SMBus controller context
 * @param enable    Enable/disable I2C mode (1=enable, 0=disable)
 * @return int      errno
 */
int hw_i2c(smbus_ctrl_t *const ctrl, const unsigned int enable)
{
    /* Sanity check */
    assert(ctrl != NULL);
    if ((ctrl->pci.bdf == PCI_BDF_NONE) || (ctrl->pci.hdl == NULL)) {
        SMBUS_LOG_WARNING("I2C transaction mode is not supported.");
        return ENOTSUP;
    }

    if (enable) {
        hw_pci_write8(ctrl, SMBUS_HCFG,
            (uint8_t)(hw_pci_read8(ctrl, SMBUS_HCFG) | SMBUS_HCFG_I2CEN));
    } else {
        hw_pci_write8(ctrl, SMBUS_HCFG,
            (uint8_t)(hw_pci_read8(ctrl, SMBUS_HCFG) & ~SMBUS_HCFG_I2CEN));
    }
    return EOK;
}

