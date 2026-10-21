/*
 * Copyright (c) 2018,2021,2025 BlackBerry Limited.
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

#ifndef _SMBUS_INTERNAL_H_INCLUDED
#define _SMBUS_INTERNAL_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include <pci/pci.h>

struct _smbus_dev_t;
#define IOFUNC_ATTR_T       struct _smbus_dev_t
#define THREAD_POOL_PARAM_T dispatch_context_t

#include <sys/iofunc.h>
#include <sys/procmgr.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>

#include <hw/dcmd_smbus.h>


/**
 * @brief Default Resource Manager Path
 */
#define SMBUS_RESMGR_PATH               "/dev/smb"

/**
 * @brief SMBus Clock Low Timeout
 *
 * The SMBus specification defines a fixed clock low timeout of 35 ms.
 */
#define SMBUS_TRANSACTION_TIMEOUT_MS    (35ULL)
#define SMBUS_TRANSACTION_TIMEOUT_US    (35000ULL)
#define SMBUS_TRANSACTION_TIMEOUT_NS    (35000000ULL)

/**
 * @brief SMBus Physical Base Address Type
 */
typedef enum _base_type_t {
    SMBUS_BASE_TYPE_NONE = 0,           /* Default Type */
    SMBUS_BASE_TYPE_MEM = 1,            /* MMIO Address */
    SMBUS_BASE_TYPE_IO = 2,             /* I/O Address */

    SMBUS_BASE_TYPE_FIRST_ENTRY = SMBUS_BASE_TYPE_NONE,
    SMBUS_BASE_TYPE_LAST_ENTRY = SMBUS_BASE_TYPE_IO,
} base_type_t;

/**
 * @brief SMBus Options for Resource Manager
 */
typedef struct _smbus_options_t {
    unsigned int        unit;
    char                *uid_gid;
    unsigned int        polling_mode;
    int                 verbose;

    unsigned int        pci_idx;
    pci_vid_t           pci_vid;
    pci_did_t           pci_did;

    paddr64_t           base_addr;
    size_t              base_size;
    base_type_t         base_type;

    unsigned int        irq;

    char                *extra_opts;
} smbus_options_t;

/**
 * @brief SMBus Device Attributes For Resource Manager
 */
typedef struct _smbus_dev_t {
    iofunc_attr_t       hdr;
    dispatch_t          *dpp;
    dispatch_context_t  *ctp;
    int                 id;

    smbus_options_t     *opts;
    void                *hdl;
} smbus_dev_t;

/*
 * Resource Manager Functions
 */

int smbus_options(smbus_options_t *opts, int argc, char *argv[]);
int smbus_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
int smbus_devctl_cmd(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);

/*
 * Platform-Specific Functions
 */

int smbus_hw_init(smbus_dev_t *dev);
int smbus_hw_cmd(void *const hdl, smbus_devctl_cmd_t *hdr, const size_t len, uint8_t *data);

#endif /*_SMBUS_INTERNAL_H_INCLUDED*/

