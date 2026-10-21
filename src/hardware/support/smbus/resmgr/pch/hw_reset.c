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
 * @brief Reset the SMBus PCIe host controller
 *
 * @param ctrl      SMBus controller context
 * @return int      errno
 */
int hw_reset_host(smbus_ctrl_t *const ctrl)
{
    /* Sanity check */
    assert(ctrl != NULL);
    if (ctrl->pci.hdl == NULL) {
        SMBUS_LOG_WARNING("Reset of SMBus PCIe host is not supported.");
        return ENOTSUP;
    }

    pci_device_reset(ctrl->pci.hdl, pci_resetType_e_BUS);
    return EOK;
}

/**
 * @brief Reset the SMBus core controller
 *
 * @param ctrl      SMBus controller context
 * @return int      errno
 */
int hw_reset_core(smbus_ctrl_t *const ctrl)
{
    /* Sanity check */
    assert(ctrl != NULL);
    if ((ctrl->pci.bdf == PCI_BDF_NONE) || (ctrl->pci.hdl == NULL)) {
        SMBUS_LOG_WARNING("Reset of SMBus core is not supported.");
        return ENOTSUP;
    }

    /* Assert the reset bit, and wait for completion  */
    hw_pci_write8(ctrl, SMBUS_HCFG, (uint8_t)(hw_pci_read8(ctrl, SMBUS_HCFG) | SMBUS_HCFG_SSRESET));
    while (hw_pci_read8(ctrl, SMBUS_HCFG) & SMBUS_HCFG_SSRESET);

    /* Re-enable the controller */
    hw_pci_write8(ctrl, SMBUS_HCFG, (uint8_t)(hw_pci_read8(ctrl, SMBUS_HCFG) | SMBUS_HCFG_HSTEN));

    /* Disable System Management Interrupts (SMI) */
    hw_pci_write8(ctrl, SMBUS_HCFG, (uint8_t)(hw_pci_read8(ctrl, SMBUS_HCFG) & ~SMBUS_HCFG_SSEN));

    return EOK;
}

