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
 * eapi_lib.h
 *
 * Shared private definitions for the interface between the EAPI resource manager
 * and client library
 *
 */
#ifndef EAPI_LIB_COMMON_H__
#define EAPI_LIB_COMMON_H__

#include "hw/eapi_client.h"

 /*
 * ABI compatibility
 *
 * Major version:
 *   Incremented on incompatible changes (e.g., devctl or payload format changes).
 *   Enforced to match exactly between client and server.
 *
 * Minor version:
 *   Incremented on backward-compatible changes.
 *   Enforced that clients must be less than or equal to eapi server version.
 */
#define EAPI_MAJOR_VER  1u
#define EAPI_MINOR_VER  0u

/* devctl command ids */

#define MAX_FORMAT_SIZE 64U
#define DCMD_ACCESS_I2C_CMD 0x10U
#define DCMD_GET_FIRMWARE_VERSION_CMD 0U
#define DCMD_EAPI_GET_ABI_CMD 0x65U
#define DCMD_EAPI_WD_CAPS_CMD 0x66U
#define DCMD_EAPI_WD_CONFIG 0x60U
#define DCMD_EAPI_WD_START 0x61U
#define DCMD_WDT_REFRESH_CMD 0x62U
#define DCMD_WDT_STOP_CMD 0x63U

/* devctl commands defs */
#define DCMD_EAPI    (_DCMD_MISC)

#define DCMD_EAPI_GET_ABI  \
    __DIOF(DCMD_EAPI, DCMD_EAPI_GET_ABI_CMD, uint32_t[2])

#define DCMD_EAPI_I2C_RD_WR \
    __DIOTF(DCMD_EAPI, DCMD_ACCESS_I2C_CMD, \
            eapi_access_i2c_u[MAX_FORMAT_SIZE])

#define DCMD_EAPI_GETVER \
    __DIOF(DCMD_EAPI, DCMD_GET_FIRMWARE_VERSION_CMD, \
           get_firmware_version_s[MAX_FORMAT_SIZE])

#define DCMD_EAPI_GET_CAPS \
    __DIOF(DCMD_EAPI, DCMD_EAPI_WD_CAPS_CMD, eapi_caps)

#define DCMD_EAPI_WDT_CONFIG \
    __DIOT(DCMD_EAPI, DCMD_EAPI_WD_CONFIG, \
           eapi_wdt_params_s[MAX_FORMAT_SIZE])

#define DCMD_EAPI_WDT_START \
    __DIOT(DCMD_EAPI, DCMD_EAPI_WD_START, \
           eapi_wdt_params_s[MAX_FORMAT_SIZE])

#define DCMD_EAPI_WDT_TRIGGER \
    __DION(DCMD_EAPI, DCMD_WDT_REFRESH_CMD)

#define DCMD_EAPI_WDT_STOP \
    __DION(DCMD_EAPI, DCMD_WDT_STOP_CMD)


#endif /* eapi_lib.h */