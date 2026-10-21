/*
* Copyright (c) 2026, BlackBerry Limited. All rights reserved.
*
* BlackBerry Limited and its licensors retain all intellectual property and
* proprietary rights in and to this software and related documentation. Any
* use, reproduction, disclosure or distribution of this software and related
* documentation without an express license agreement from BlackBerry Limited
* is strictly prohibited.
*/

/*
 * eapi.h
 * Private defines for ResMgr layer
 *
 */

#ifndef EAPI_COMM_H__
#define EAPI_COMM_H__

#include <sys/iofunc.h>
#include <sys/resmgr.h>

#include <errno.h>
#include <unistd.h>
#include <hw/inout.h>
#include <secpol/secpol.h>
#include <sys/dispatch.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/procmgr.h>

/* Shared private - lib headers */
#include <hw/eapi_lib.h>

/* Private module headers */
#include "eapi_modules.h"

#include "mec170x/mec170x_def.h"
#include "f81804/f81804_def.h"

#define NUM_ELTS(__array)	(sizeof(__array)/sizeof(__array[0]))


typedef struct eapi_ocb {
    iofunc_ocb_t ocb;            /* MUST be first */
    bool      abi_ok;            /* Cache version check per ocb */
    struct {
        bool      i2c;           /* Cache i2c abilities check per ocb */
        bool      wdt_config;    /* Cache watchdog abilities for config, start and stop */
        bool      wdt_refresh;   /* Cache watchdog abilities for refresh */
    } abils_allowed;
} eapi_ocb_t;

typedef struct eapi_resmgr_s
{
    resmgr_connect_funcs_t connect_funcs;
    resmgr_io_funcs_t io_funcs;
    dispatch_t *dpp;
    dispatch_context_t *ctp;
    int32_t resmgr_id;
} eapi_resmgr_t;

/***************************************************************************************/
/********************     Internal function Prototypes for server   ********************/
/***************************************************************************************/

/* init.c */
int32_t	eapi_chip_init(eapi_dev_t* dev, eapi_ops_g* eapi_ops);
int32_t eapi_parseopts(eapi_dev_t *dev, int32_t argc, char *argv[]);
const chip_tbl_t *find_chip_entry(const char *optarg);
int32_t eapi_perms_init(eapi_dev_t *dev);

/* Support.c */
int32_t eapi_check_integrity_watchdog(resmgr_context_t *ctp, io_devctl_t *msg, void *priv);
int32_t eapi_check_integrity_i2c(resmgr_context_t *ctp, io_devctl_t *msg, void *priv);
int32_t eapi_check_integrity_version(resmgr_context_t *ctp, io_devctl_t *msg, void *priv);
int32_t eapi_check_integrity_abi(resmgr_context_t *ctp, io_devctl_t *msg, void *priv);
int32_t eapi_check_integrity_caps(resmgr_context_t *ctp, io_devctl_t *msg, void *priv);
int32_t eapi_check_perm(resmgr_context_t *ctp, int32_t abid, bool *cached_abil);


#endif
