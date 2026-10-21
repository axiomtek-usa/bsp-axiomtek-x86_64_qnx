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

#include <sys/mman.h>

#include "smbus_internal.h"
#include "log.h"
#include "hw.h"


/**
 * @brief Initialize the SMBus controller
 *
 * @param dev       SMBus device attributes
 * @return int      errno
 */
int smbus_hw_init(smbus_dev_t *dev)
{
    int err = EIO;
    smbus_ctrl_t *ctrl = NULL;

    /* Sanity check */
    assert(dev != NULL);
    assert(dev->opts != NULL);

    /* Request I/O privileges */
    if (ThreadCtl(_NTO_TCTL_IO, NULL) < 0) {
        SMBUS_LOG_CRITICAL("Unable to obtain I/O privileges!");
        return EPERM;
    }

    /* Create the SMBus controller context */
    ctrl = calloc(1, sizeof(smbus_ctrl_t));
    if (ctrl == NULL) {
        SMBUS_LOG_CRITICAL("Error while allocating SMBus controller context! - %s", strerror(errno));
        return errno;
    }
    /* Assign cross-reference between device context and hardware controller */
    ctrl->dev = dev;
    ctrl->pci.bdf = PCI_BDF_NONE;

    /* Parse device-specific options */
    err = hw_options(ctrl, dev->opts->extra_opts);
    if (err != EOK) {
        SMBUS_LOG_ERROR("Error while parsing device-specifc options!");
        goto fail;
    }

    /*
     * Configure the controller base address and size
     * Note: Get the base address specified in options, or scan the PCI bus.
     */
    if ((dev->opts->base_addr != 0) && (dev->opts->base_size > 0)) {
        ctrl->base.paddr = dev->opts->base_addr;
        ctrl->base.size  = dev->opts->base_size;
        ctrl->base.type  = dev->opts->base_type;
    } else {
        err = hw_init_pci(ctrl);
        if (err != EOK) {
            SMBUS_LOG_ERROR("Error while initializing PCIe device! - %s", strerror(err));
            goto fail;
        }
    }

    /* Map base address to virtual memory */
    if (ctrl->base.type == SMBUS_BASE_TYPE_IO) {
        ctrl->base.addr = (volatile uint8_t*)mmap_device_io(ctrl->base.size, ctrl->base.paddr);
    } else {
        ctrl->base.addr = mmap_device_memory(NULL, ctrl->base.size,
            (PROT_READ | PROT_WRITE | PROT_NOCACHE), 0, ctrl->base.paddr);
        /* Force base address type to MMIO */
        ctrl->base.type = SMBUS_BASE_TYPE_MEM;
    }

    /* Verify that the virtual base address is valid, fails otherwise */
    if (ctrl->base.addr == MAP_FAILED) {
        SMBUS_LOG_ERROR("No valid base address registered! - %s", strerror(errno));
        err = errno;
        goto fail;
    }

    /* Set the IRQ from options if it was specified */
    if (dev->opts->irq != 0) {
        ctrl->interrupt.irq = (int)dev->opts->irq;
    }

    /*
     * Configure the interrupt
     * Note: The interrupt comes from command line options
     *       or from the PCI device initialization.
     *       Default to polling mode:
     *          - if IRQ was not specified in options or found in PCI device
     *          - if polling mode option was set in options
     */
    if ((dev->opts->polling_mode == 0) && (ctrl->interrupt.irq != 0)) {

        /* Initialize the interrupt event data */
        SIGEV_INTR_INIT(&ctrl->interrupt.event_data);

        /* Attach the interrupt to the event */
        ctrl->interrupt.event_id = InterruptAttachEvent(ctrl->interrupt.irq,
            &ctrl->interrupt.event_data, _NTO_INTR_FLAGS_TRK_MSK);
        if (ctrl->interrupt.event_id < 0) {
            SMBUS_LOG_ERROR("Failed to attach to interrupt 0x%x - %s",
                ctrl->interrupt.irq, strerror(errno));
            err = errno;
            goto fail;
        }
    }

    /* Reset the SMBus controller */
    hw_reset_core(ctrl);

    /*
     * Display SMBus controller information
     */
    SMBUS_LOG_INFO("SMBus Controller Base Address: 0x%lx (0x%lx) -> 0x%p",
        ctrl->base.paddr, ctrl->base.size, ctrl->base.addr);
    SMBUS_LOG_INFO("SMBus Controller IRQ: 0x%x", ctrl->interrupt.irq);
    if (ctrl->interrupt.event_id == 0) {
        SMBUS_LOG_INFO("SMBus Controller is using polling mode");
    }

    dev->hdl = ctrl;
    return EOK;

 fail:
    if (ctrl->pci.hdl != NULL) {
        pci_device_detach(ctrl->pci.hdl);
    }

    if (ctrl->base.addr != NULL) {
        if (ctrl->base.type == SMBUS_BASE_TYPE_MEM) {
            munmap_device_memory((void*)ctrl->base.addr, ctrl->base.size);
        } else if (ctrl->base.type == SMBUS_BASE_TYPE_IO) {
            munmap_device_io((uintptr_t)ctrl->base.addr, ctrl->base.size);
        } else {
            ;   /* No actions */
        }
    }

    if (ctrl->interrupt.event_id > 0) {
        InterruptDetach(ctrl->interrupt.event_id);
    }

    memset(ctrl, 0, sizeof(smbus_ctrl_t));
    free(ctrl);
    return err;
}

