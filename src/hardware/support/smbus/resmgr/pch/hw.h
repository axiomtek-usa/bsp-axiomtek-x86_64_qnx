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

#ifndef _HW_H_INCLUDED
#define _HW_H_INCLUDED

#include <pci/pci.h>


/**
 * @brief SMBus Controller Context
 */
typedef struct _smbus_ctrl_t {

    /* PCI Context */
    struct {
        pci_bdf_t           bdf;            /* PCI Bus-Device-Function */
        pci_devhdl_t        hdl;            /* PCI connection handle */
        pci_ba_t            bar[2];         /* PCI MMIO and I/O BARs */
        pci_irq_t           irq;            /* PCI IRQ (MSI or Legacy IRQ) */
    } pci;

    /* Base Address Context */
    struct {
        volatile uint8_t    *addr;          /* Virtual base address of the SMBus registers */
        paddr64_t           paddr;          /* Physical base address of the SMBus registers */
        size_t              size;           /* Length in bytes of the SMBus registers */
        base_type_t         type;           /* Base address type (MMIO or I/O) */
    } base;

    /* Interrupt Context */
    struct {
        int                 irq;            /* IRQ number */
        int                 event_id;       /* IRQ event ID */
        struct sigevent     event_data;     /* IRQ event data */
    } interrupt;

    uint64_t                lock_timeout;   /* Timeout to lock hardware semaphore */

    smbus_dev_t             *dev;           /* SMBus device attributes */

} smbus_ctrl_t;


int hw_init_pci(smbus_ctrl_t *ctrl);
int hw_options(smbus_ctrl_t *const ctrl, char *const options);

int hw_reset_host(smbus_ctrl_t *const ctrl);
int hw_reset_core(smbus_ctrl_t *const ctrl);

int hw_i2c(smbus_ctrl_t *const ctrl, const unsigned int enable);
int hw_pec(smbus_ctrl_t *const ctrl, const unsigned int enable);

int hw_intr_irq(smbus_ctrl_t *const ctrl, const smbus_rw_t rw, size_t size, uint8_t *data);
int hw_intr_poll(smbus_ctrl_t *const ctrl, const smbus_rw_t rw, size_t size, uint8_t *data);

int hw_lock(smbus_ctrl_t *const ctrl, unsigned long timeout_us);
int hw_unlock(smbus_ctrl_t *const ctrl);

uint8_t hw_reg_read8(smbus_ctrl_t *const ctrl, const unsigned int reg);
uint16_t hw_reg_read16(smbus_ctrl_t *const ctrl, const unsigned int reg);
void hw_reg_write8(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint8_t data);
void hw_reg_write16(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint16_t data);
void hw_reg_write_mask8(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint8_t mask, const uint8_t data);
void hw_reg_write_mask16(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint16_t mask, const uint16_t data);

uint8_t hw_pci_read8(smbus_ctrl_t *const ctrl, const unsigned int reg);
uint16_t hw_pci_read16(smbus_ctrl_t *const ctrl, const unsigned int reg);
uint8_t hw_pci_write8(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint8_t data);
uint16_t hw_pci_write16(smbus_ctrl_t *const ctrl, const unsigned int reg, const uint16_t data);


#endif /*_HW_H_INCLUDED*/

