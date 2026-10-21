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

#include <pci/pci_ccode.h>

#include "smbus_internal.h"
#include "log.h"
#include "hw.h"


/**
 * @brief Locate the SMBus controller on the PCI bus
 *
 * @param ctrl      SMBus controller context
 * @return int      errno
 */
int hw_init_pci(smbus_ctrl_t *ctrl)
{
    pci_err_t pci_err;
    unsigned int idx;

    /* Sanity check */
    assert(ctrl != NULL);
    if ((ctrl->dev == NULL) || (ctrl->dev->opts == NULL)) {
        return EINVAL;
    }

    /* Scan for the SMBus controller on the PCIe bus */
    ctrl->pci.bdf = pci_device_find(ctrl->dev->opts->pci_idx,
        ctrl->dev->opts->pci_vid, ctrl->dev->opts->pci_did, PCI_CCODE_SBUS_SMBUS);
    if (ctrl->pci.bdf == PCI_BDF_NONE) {
        SMBUS_LOG_ERROR("The SMBus controller was not found on the PCIe bus!");
        goto fail;
    }

    /* Attach to the SMBus controller device */
    ctrl->pci.hdl = pci_device_attach(ctrl->pci.bdf, pci_attachFlags_EXCLUSIVE_OWNER, &pci_err);
    if (ctrl->pci.hdl == NULL) {
        SMBUS_LOG_ERROR("Failed to attach to the SMBus controller %d:%d:%d! - %s",
            PCI_BUS(ctrl->pci.bdf), PCI_DEV(ctrl->pci.bdf), PCI_FUNC(ctrl->pci.bdf), pci_strerror(pci_err));
        goto fail;
    }

    /* Read the PCIe BARs */
    int_t ba_cnt = (int_t)NELEMENTS(ctrl->pci.bar);
    pci_err = pci_device_read_ba(ctrl->pci.hdl, &ba_cnt, ctrl->pci.bar, pci_reqType_e_UNSPECIFIED);
    if (pci_err != PCI_ERR_OK) {
        SMBUS_LOG_ERROR("Failed to read the BAR of the SMBus controller %d:%d:%d! - %s",
            PCI_BUS(ctrl->pci.bdf), PCI_DEV(ctrl->pci.bdf), PCI_FUNC(ctrl->pci.bdf), pci_strerror(pci_err));
        goto fail;
    }

    /* Retrieve the IRQ from the PCI device */
    pci_err = pci_device_read_irq(ctrl->pci.hdl, NULL, &ctrl->pci.irq);
    if (pci_err == PCI_ERR_IRQ_NOT_AVAIL) {
        /* Not found = driver will default to polling mode */
        ctrl->pci.irq = 0;
    } else if (pci_err == PCI_ERR_OK) {
        /* Found = driver will use interrupt unless polling mode was requested */
        ctrl->interrupt.irq = (int)ctrl->pci.irq;
    } else {
        SMBUS_LOG_ERROR("Failed to read the IRQ of the SMBus controller %d:%d:%d! - %s",
            PCI_BUS(ctrl->pci.bdf), PCI_DEV(ctrl->pci.bdf), PCI_FUNC(ctrl->pci.bdf), pci_strerror(pci_err));
        goto fail;
    }

    /* Scan for the MEMIO Base Address */
    for (idx = 0; idx < NELEMENTS(ctrl->pci.bar); idx++) {
        if (ctrl->pci.bar[idx].type == pci_asType_e_MEM) {
            if ((ctrl->pci.bar[idx].addr != 0) && (ctrl->pci.bar[idx].size > 0)) {
                /* BAR found, save BAR info */
                ctrl->base.paddr = ctrl->pci.bar[idx].addr;
                ctrl->base.size = ctrl->pci.bar[idx].size;
                ctrl->base.type = SMBUS_BASE_TYPE_MEM;
                break;
            }
        }
    }

    /* Scan for the I/O Base Address if no MMIO Base Address was found */
    if (ctrl->base.paddr == 0) {
        for (idx = 0; idx < NELEMENTS(ctrl->pci.bar); idx++) {
            if (ctrl->pci.bar[idx].type == pci_asType_e_IO) {
                /* BAR found, save BAR info */
                ctrl->base.paddr = ctrl->pci.bar[idx].addr;
                ctrl->base.size = ctrl->pci.bar[idx].size;
                ctrl->base.type = SMBUS_BASE_TYPE_IO;
                break;
            }
        }
    }

    /* Final check on base address */
    if (ctrl->base.paddr == 0) {
        SMBUS_LOG_ERROR("No valid base address was found for the SMBus controller %d:%d:%d!",
            PCI_BUS(ctrl->pci.bdf), PCI_DEV(ctrl->pci.bdf), PCI_FUNC(ctrl->pci.bdf));
        goto fail;
    }
    return EOK;

 fail:
    if (ctrl->pci.hdl != NULL) {
        pci_device_detach(ctrl->pci.hdl);
        ctrl->pci.hdl = NULL;
    }
    return EIO;
}

