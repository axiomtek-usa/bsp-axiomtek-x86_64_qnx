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


/**
 * @brief Read 8-bit SMBus PCI register
 *
 * @param ctrl      SMBus controller context
 * @param reg       PCI configuration space register offset (>=0x40)
 * @return uint8_t  PCI configuration space register value
 */
uint8_t hw_pci_read8(smbus_ctrl_t *const ctrl, const unsigned int reg)
{
    uint8_t data = 0;
    pci_err_t err;

    assert(ctrl != NULL);
    assert(ctrl->pci.bdf != PCI_BDF_NONE);
    assert(reg >= 0x40);

    err = pci_device_cfg_rd8(ctrl->pci.bdf, (uint_t)reg, &data);
    if (err != PCI_ERR_OK) {
        SMBUS_LOG_CRITICAL("Failed to read PCI device %d:%d:%d at 0x%x : %s",
            PCI_BUS(ctrl->pci.bdf), PCI_DEV(ctrl->pci.bdf), PCI_FUNC(ctrl->pci.bdf), reg, pci_strerror(err));
        abort();
    }
    return data;
}

/**
 * @brief Read 16-bit SMBus PCI register
 *
 * @param ctrl      SMBus controller context
 * @param reg       PCI configuration space register offset (>=0x40)
 * @return uint16_t PCI configuration space register value
 */
uint16_t hw_pci_read16(smbus_ctrl_t *const ctrl, const unsigned int reg)
{
    uint16_t data = 0;
    pci_err_t err;

    assert(ctrl != NULL);
    assert(ctrl->pci.bdf != PCI_BDF_NONE);
    assert(reg >= 0x40);

    err = pci_device_cfg_rd16(ctrl->pci.bdf, (uint_t)reg, &data);
    if (err != PCI_ERR_OK) {
        SMBUS_LOG_CRITICAL("Failed to read PCI device %d:%d:%d at 0x%x : %s",
            PCI_BUS(ctrl->pci.bdf), PCI_DEV(ctrl->pci.bdf), PCI_FUNC(ctrl->pci.bdf), reg, pci_strerror(err));
        abort();
    }
    return data;
}

/**
 * @brief Write 8-bit SMBus PCI register
 *
 * @param ctrl      SMBus controller context
 * @param reg       PCI configuration space register offset (>=0x40)
 * @param data      Value to write to PCI configuration space register
 * @return uint8_t  PCI configuration space register value
 */
uint8_t hw_pci_write8(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint8_t data)
{
    uint8_t ret = 0;
    pci_err_t err;

    assert(ctrl != NULL);
    assert(ctrl->pci.hdl != NULL);
    assert(reg >= 0x40);

    err = pci_device_cfg_wr8(ctrl->pci.hdl, (uint_t)reg, data, &ret);
    if (err != PCI_ERR_OK) {
        SMBUS_LOG_CRITICAL("Failed to write PCI device %d:%d:%d at 0x%x : %s",
            PCI_BUS(ctrl->pci.bdf), PCI_DEV(ctrl->pci.bdf), PCI_FUNC(ctrl->pci.bdf), reg, pci_strerror(err));
        abort();
    }
    return ret;
}

/**
 * @brief Write 16-bit SMBus PCI register
 *
 * @param ctrl      SMBus controller context
 * @param reg       PCI configuration space register offset (>=0x40)
 * @param data      Value to write to PCI configuration space register
 * @return uint16_t PCI configuration space register value
 */
uint16_t hw_pci_write16(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint16_t data)
{
    uint16_t ret = 0;
    pci_err_t err;

    assert(ctrl != NULL);
    assert(ctrl->pci.hdl != NULL);
    assert(reg >= 0x40);

    err = pci_device_cfg_wr16(ctrl->pci.hdl, (uint_t)reg, data, &ret);
    if (err != PCI_ERR_OK) {
        SMBUS_LOG_CRITICAL("Failed to write PCI device %d:%d:%d at 0x%x : %s",
            PCI_BUS(ctrl->pci.bdf), PCI_DEV(ctrl->pci.bdf), PCI_FUNC(ctrl->pci.bdf), reg, pci_strerror(err));
        abort();
    }
    return ret;
}

