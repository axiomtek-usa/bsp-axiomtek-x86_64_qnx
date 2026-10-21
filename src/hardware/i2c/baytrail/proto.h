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


#ifndef _BAYTRAIL_I2C_PROTO_H_INCLUDED
#define _BAYTRAIL_I2C_PROTO_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <fcntl.h>
#include <hw/i2c.h>
#include <pci/pci.h>
#include "baytrail_i2c.h"


#define logerr(bdf, fmt, ...) \
    (i2c_slogf((dev)->verbose, _SLOG_ERROR, \
           "i2c-baytrail (B%u:D%u:F%u) " fmt, PCI_BUS((bdf)), PCI_DEV((bdf)), PCI_FUNC((bdf)), ##__VA_ARGS__))

#define logfyi(bdf, fmt, ...) \
    (i2c_slogf((dev)->verbose, _SLOG_INFO, \
           "i2c-baytrail (B%u:D%u:F%u) " fmt, PCI_BUS((bdf)), PCI_DEV((bdf)), PCI_FUNC((bdf)), ##__VA_ARGS__))

#define BAYTRAIL_I2C_DEF_INTR_PRIORITY  21
#define BAYTRAIL_I2C_EVENT              1

typedef struct {
    /* PCI interface */
    pci_devhdl_t    pci;            // PCI device handle
    pci_ba_val_t    pbase;          // physical base address of device registers
    uintptr_t       vbase;          // mmapped virtual address of device registers
    uint64_t        map_size;       // size of mmapped region
    unsigned        idx;            // PCI device index
    pci_vid_t       vid;            // PCI Vendor ID
    pci_did_t       did;            // PCI Device ID
    pci_irq_t       irq;            // PCI interrupt request number
    pci_cap_t       msi;            // PCI interrupt msi support

    /* Interrupt */
    int             iid;

    /* Slave address */
    unsigned        slave_addr;
    i2c_addrfmt_t   slave_addr_fmt;

    /* Clock */
    uint32_t        clock_khz;
    uint32_t        ss_hcnt;
    uint32_t        ss_lcnt;
    uint32_t        fs_hcnt;
    uint32_t        fs_lcnt;
    unsigned        scl_freq;
    unsigned        sda_hold_time;

    /* transaction information */
    uint8_t         *txbuf;
    uint8_t         *rxbuf;    
    unsigned        xlen;           // how many bytes for total transaction
    unsigned        txlen;          // how many bytes for writing
    unsigned        rxlen;          // how many bytes for reading
    unsigned        wrlen;          // how many cmds have been written to TxFIFO
    unsigned        rdlen;          // how many data have been read from RxFIFO

    /* Other information */
    uint32_t        verbose;
    uint32_t        master_cfg;
    uint32_t        status;
    uint32_t        abort_source;
    uint32_t        fifo_depth;
    uint32_t        fixed_scl;	    // To set scl and hold registers to a fixed, Intel-recommended value
} baytrail_dev_t;



void baytrail_i2c_reset(baytrail_dev_t* const dev);
void *baytrail_i2c_init(int argc, char *argv[]);
void baytrail_i2c_init_registers(baytrail_dev_t *dev);
void baytrail_i2c_fini(void *hdl);
void baytrail_i2c_dw_xfer_init(baytrail_dev_t *dev);
void i2c_reg_write32(baytrail_dev_t* const dev, const uint32_t offset, const uint32_t value);
uint32_t i2c_reg_read32(baytrail_dev_t* const dev, const uint32_t offset);
i2c_status_t baytrail_i2c_wait_complete(baytrail_dev_t* const dev);
int baytrail_i2c_enable(baytrail_dev_t* const dev, const unsigned enable);
int baytrail_i2c_bus_active(baytrail_dev_t* const dev);
int baytrail_i2c_parseopts(baytrail_dev_t *dev, int argc, char *argv[]);
int baytrail_i2c_set_slave_addr(void *hdl, unsigned int addr, i2c_addrfmt_t fmt);
int baytrail_i2c_set_bus_speed(void *hdl, unsigned int speed, unsigned int *ospeed);
int baytrail_i2c_version_info(i2c_libversion_t *version);
int baytrail_i2c_driver_info(void *hdl, i2c_driver_info_t *info);
int baytrail_i2c_wait_bus_not_busy(baytrail_dev_t* const dev);
i2c_status_t baytrail_i2c_send(void *hdl, void *buf, unsigned int len, unsigned int stop);
i2c_status_t baytrail_i2c_recv(void *hdl, void *buf, unsigned int len, unsigned int stop);
i2c_status_t baytrail_i2c_xfer(void *const hdl, void *const send_buf, const uint32_t send_len,
                          void *const recv_buf, const uint32_t recv_len);

static inline
int xmit_FIFO_is_full(baytrail_dev_t *dev)
{
    return (i2c_reg_read32(dev, DW_IC_STATUS) & DW_IC_STATUS_TFNF) == 0;
}

#endif
