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
 * eapi_modules.h
 *
 * Shared definitions for the interface between the EAPI resource manager
 * and hardware modules
 *
 */
#ifndef EAPI_MODULES_COMMON_H__
#define EAPI_MODULES_COMMON_H__

#include <devctl.h>
#include <stdio.h>
#include <hw/inout.h>
#include <sys/iomgr.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include <stdint.h>
#include <stdbool.h>

#include <hw/eapi_client.h>

typedef struct eapi_dev_s eapi_dev_t;

typedef struct eapi_cmd_ops
{
    int32_t (*request_mbx_access_cmd)        (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*release_mbx_access_cmd)        (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_firmware_version_cmd)      (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*assert_system_reset_cmd)       (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*rstbtn_reset_cmd)              (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*pwrbtn_shutdown_cmd)           (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_status_register)           (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*clr_status_register)           (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_enable_register)           (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*set_enable_register)           (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*clr_enable_register)           (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*set_feature_variable)          (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_feature_variable)          (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_uart_config)               (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*set_uart_config)               (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*access_i2c_cmd)                (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_eeprom_capacity_cmd)       (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_eeprom_reserved_cmd)       (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*eeprom_read_cmd)               (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*eeprom_write_cmd)              (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*utc_read_cmd)                  (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*utc_write_cmd)                 (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_after_g3_cache)            (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_after_g3_state)            (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_last_state)                (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*set_after_g3_state)            (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*get_reset_causes_cmd)          (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*clear_reset_causes_cmd)        (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*set_port_80_debug_cmd)         (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_rpm_read_cmd)              (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_dc_read_cmd)               (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_dc_write_cmd)              (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_freq_read_cmd)             (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_freq_write_cmd)            (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_type_write_cmd)            (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_temp_read_cmd)             (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_temp_write_cmd)            (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_therm_mgmt_cfg_cmd)        (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_ramp_read_cmd)             (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_ramp_write_cmd)            (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_enable_cfg_cmd)            (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*fan_disable_cmd)               (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*spi_access_cmd)                (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*gpio_access_cmd)               (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*gpio_int_clear_cmd)            (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*wdt_config_cmd)                (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*wdt_start_cmd)                 (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*wdt_refresh_cmd)               (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*wdt_stop_cmd)                  (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*access_adc_cmd)                (eapi_dev_t* dev, void *data, size_t* nbytes);
    int32_t (*access_rcid_cmd)               (eapi_dev_t* dev, void *data, size_t* nbytes);

    /* Capabilities of the EC */
    int32_t (*device_get_caps_cmd)           (eapi_dev_t* dev, void *data, size_t* nbytes);
/*
* Interrupt handling is not validated on any of the supported hardware.
* EAPI_CAPS_INTERRUPT_MODE must be set by hw modules.
*/
    void (*device_interrupt_handler)      (void * arg);

} eapi_ops_g;

/* Additional EC / Super I/O families may be added here in future
 * platform variants (Nuvoton NCT677x/NPCX7xx, ITE IT86xx, etc.).
 *
 * [0 - Not initialized]
 */
typedef enum {
    CHIP_UNKNOWN,

    /* Microchip MEC family */
    CHIP_MEC170X,

    /* Fintek Super I/O */
    CHIP_F81804,

}chip_name_et;

typedef struct
{
    char *name_str;
    chip_name_et chip_name;
} chip_tbl_t;

/* Procmgr custom ability indices
 *
 * EAPI_ABILITY_COUNT is the total number of abilities
 * and must remain the last entry.
 */
typedef enum
{
    EAPI_ABIL_IDX_I2C_XFER,
    EAPI_ABIL_IDX_WDT_CONFIG,
    EAPI_ABIL_IDX_WDT_REFRESH,

    EAPI_ABILITY_COUNT
} ability_num_et;

typedef struct eapi_dev_s
{
    uintptr_t io;
    uint32_t irq;
    int32_t priority;
    chip_name_et chip_name;
    int32_t abids[EAPI_ABILITY_COUNT];
    int32_t mode;
    eapi_ops_g eapi_ops;
    uint32_t verbose;
} eapi_dev_t;

/***************************************************************************************/
/************ Internal function Prototypes for modules and server   ********************/
/***************************************************************************************/
void eapi_slogf(const unsigned verbosity, const unsigned level, const char *fmt, ...);

#endif
