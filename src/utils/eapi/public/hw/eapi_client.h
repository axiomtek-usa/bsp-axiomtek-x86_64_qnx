/*
* Copyright (c) 2026, BlackBerry Limited. All rights reserved.
*
* BlackBerry Limited and its licensors retain all intellectual property and
* proprietary rights in and to this software and related documentation. Any
* use, reproduction, disclosure or distribution of this software and related
* documentation without an express license agreement from BlackBerry Limited
* is strictly prohibited.
*/
/**
 * @file eapi_client.h
 * @brief EAPI Client Helper API
 */

#ifndef EAPI_CLIENT_H_
#define EAPI_CLIENT_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>
#include <devctl.h>
#include <sys/iomgr.h>
#include <sys/iomsg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>

/* Interface Level Defs */
#define EC_MBX_SIZE 0x20U /* MBX size info for further checks */
#define FIRMWARE_TIME_STAMP_SIZE (EC_MBX_SIZE - sizeof(uint32_t))
typedef struct get_firmware_version_struct {
    union {
        uint32_t versions;
        struct {
            union {
                uint16_t firmwareVersion;
                struct {
                    uint8_t firmwareMinorVersion;
                    uint8_t firmwareMajorVersion;
                };
            };
            union {
                uint16_t libraryVersion;
                struct {
                    uint8_t libraryMinorVersion;
                    uint8_t libraryMajorVersion;
            };
            };
        };
    } version;
    uint8_t firmwareTimeStamp[FIRMWARE_TIME_STAMP_SIZE];
} get_firmware_version_s;

/* I2C interface Level Defs */
typedef struct access_i2c_parameter
{
    uint8_t i2cBus;
    uint8_t i2cSpeed;
    uint8_t i2cAddress;
    uint8_t i2cWriteCount;
    uint8_t i2cReadCount;
} access_i2c_parameter_s;

#define ACCESS_I2C_CMD_OVERHEAD (sizeof(access_i2c_parameter_s))
#define I2C_MAX_WRITE_LENGTH (EC_MBX_SIZE - ACCESS_I2C_CMD_OVERHEAD)
#define I2C_MAX_READ_LENGTH EC_MBX_SIZE

typedef union access_i2c_struct
{
    struct
    {
        access_i2c_parameter_s parameters;
        uint8_t i2cWriteData[I2C_MAX_WRITE_LENGTH];
    };
    uint8_t i2cReadData[I2C_MAX_READ_LENGTH];
} eapi_access_i2c_u;

/* WatchDog interface Level Defs */
typedef union eapi_wdt_timeout {
    struct {
        uint8_t timeout;
    };
/* If device supports staged timeout, CAPS_WDT_TIMEOUT_CTRL SET */
    struct {
        uint32_t delay_timeout;
        uint32_t event_timeout;
        uint32_t reset_timeout;
    };
} eapi_wdt_timeout_u;

typedef struct eapi_wdt_conf {
    uint8_t status;
    uint8_t config;

    /* If device supports event control, CAPS_WDT_EVENT_CTRL SET */
    uint8_t event_cfg;
    uint8_t reset_cfg;

    /* If device supports pin control, CAPS_WDT_PIN_CTRL SET */
    uint8_t refresh_pin;
    uint8_t signal_pin;

    /* If device supports signal control, CAPS_WDT_SIG_CTRL SET */
    uint8_t pulse_width;
    uint8_t pulse_mode;
    uint8_t polarity;

} eapi_wdt_config_s;

/* Capabalities owned by hw modules */
#define EAPI_CAPS_NONE               (0u)
#define EAPI_CAPS_WDT_TIMEOUT_CTRL   (1u << 0u)
#define EAPI_CAPS_WDT_TIME_UNIT_CTRL (1u << 1u)
#define EAPI_CAPS_WDT_EVENT_CTRL     (1u << 2u)
#define EAPI_CAPS_WDT_PIN_CTRL       (1u << 3u)
#define EAPI_CAPS_WDT_SIG_CTRL       (1u << 4u)
#define EAPI_CAPS_WDT_CUSTOM         (1u << 5u)
#define EAPI_CAPS_I2C                (1u << 6u)
#define EAPI_CAPS_I2C_CUSTOM_2       (1u << 7u)
#define EAPI_CAPS_I2C_CUSTOM_3       (1u << 8u)
#define EAPI_CAPS_INTERRUPT_MODE     (1u << 9u)

typedef uint32_t eapi_caps;

typedef struct {
    uint32_t wdt_caps;
    eapi_wdt_config_s wdt_config;
    eapi_wdt_timeout_u  wdt_timeout;

    /* If device supports time unit control, CAPS_WDT_TIME_UNIT_CTRL SET */
    uint8_t  time_unit;

    /* Extras */
    int32_t flags;
    void *extra[8];

} eapi_wdt_params_s;

/* EAPI device path */
#define EAPI_DEVICE_PATH    "/dev/eapi"

/* Client ABI version.
 * Must match the server major version.
 * Server minor version must be greater than or equal to client.
 */
#define EAPI_CLIENT_ABI_MAJOR   EAPI_MAJOR_VER
#define EAPI_CLIENT_ABI_MINOR   EAPI_MINOR_VER

/* EAPI ability names
 * Control access to specific devctl operations:
 *
 *  - I2C_XFER:
 *      Allows I2C read/write transactions
 *      (DCMD_EAPI_I2C_RD_WR)
 *      Access: root only
 *
 *  - WDT_CONFIG:
 *      Allows watchdog configuration and start
 *      (DCMD_EAPI_WDT_CONFIG, DCMD_EAPI_WDT_START)
 *      Access: root only
 *
 *  - WDT_REFRESH:
 *      Allows watchdog trigger/refresh
 *      (DCMD_EAPI_WDT_TRIGGER)
 *      Access: root and non-root
 */
#define EAPI_ABILITY_PREFIX        "EAPI_ops/"
#define EAPI_ABILITY_I2C_XFER      EAPI_ABILITY_PREFIX "i2c_xfer"
#define EAPI_ABILITY_WDT_CONFIG    EAPI_ABILITY_PREFIX "wdt_config"
#define EAPI_ABILITY_WDT_REFRESH   EAPI_ABILITY_PREFIX "wdt_start"

/* lib context structure */
typedef struct eapi_ctx {
    int32_t  fd;                /**< File descriptor, set by eapi_open() on success           */
    uint32_t abi_major;         /**< Server ABI major version, set by eapi_open() on success  */
    uint32_t abi_minor;         /**< Server ABI minor version, set by eapi_open() on success  */
    eapi_caps eapi_module_caps; /**< Module capabilities, set by eapi_get_module_caps() on success */
    bool abi_valid;             /**< ABI flag, set by eapi_open() on success                  */
    uint8_t reserved[3];        /**< Reserved for alignment and potential extension */
} *eapi_handle;

/**
 * @brief Opens the EAPI device and validates ABI versioning.
 *
 * @param eapi_dev  EAPI device handle.
 * @param flags     open() flags.
 *                  Supported flags: [O_RDONLY, O_RDWR, O_WRONLY].
 * @note            O_WRONLY is silently promoted to O_RDWR for ABI validation.
 * @param path      Custom server path.
 * @note            If path is NULL, the default EAPI path is used.
 *
 * @return          EOK on success, -1 on failure (errno is set).
 *
 * @note            A major version mismatch is fatal.
 *                  A minor version mismatch is logged as a warning.
 */
int32_t eapi_open(eapi_handle eapi_dev, int32_t flags, const char *path);

/**
 * @brief Close EAPI device
 *
 * @param eapi_dev  eapi device handle
 * @return          EOK on success or error code
 */
int32_t eapi_close(eapi_handle eapi_dev);

/**
 * @brief Get server ABI version
 *
 * @param eapi_dev  eapi device handle
 * @param major     Buffer to store major version
 * @param minor     Buffer to store minor version
 * @return          EOK on success or error code
 */
int32_t eapi_get_abi_version(eapi_handle eapi_dev, uint32_t *major, uint32_t *minor);

/**
 * @brief Get firmware version
 *
 * @param eapi_dev  eapi device handle
 * @param fw        Buffer to store version string
 * @return          EOK on success or error code
 */
int32_t eapi_get_firmware_version(eapi_handle eapi_dev, get_firmware_version_s *fw);

/**
 * @brief I2C transfer
 *
 * @param eapi_dev          eapi device handle
 * @param dev_data_ptr      Tx/Rx Data Buffer
 * @param nbytes            Transmit - Recieve length
 * @param dev_info_ptr      devctl status
 * @return                  EOK on success or error code
 */
int32_t eapi_i2c_smbus_transfer(eapi_handle eapi_dev, void *dev_data_ptr, size_t nbytes, int32_t *dev_info_ptr);

/**
 * @brief Configure watchdog timer
 *
 * @param eapi_dev          eapi device handle
 * @param dev_data_ptr      Tx/Rx Data Buffer
 * @param nbytes            Transmit - Recieve length
 * @param dev_info_ptr      devctl status
 * @return                  EOK on success or error code
 */
int32_t eapi_wdt_config(eapi_handle eapi_dev, void *dev_data_ptr, size_t nbytes, int32_t *dev_info_ptr);

/**
 * @brief Start watchdog timer
 *
 * @param eapi_dev          eapi device handle
 * @param dev_data_ptr      Tx/Rx Data Buffer
 * @param nbytes            Transmit - Recieve length
 * @param dev_info_ptr      devctl status
 * @return                  EOK on success or error code
 */
int32_t eapi_wdt_start(eapi_handle eapi_dev, void *dev_data_ptr, size_t nbytes, int32_t *dev_info_ptr);

/**
 * @brief Stop watchdog timer
 *
 * @param eapi_dev  eapi device handle
 * @return          EOK on success or error code
 */
int32_t eapi_wdt_stop(eapi_handle eapi_dev);

/**
 * @brief Trigger/refresh watchdog timer
 *
 * @param eapi_dev  eapi device handle
 * @return          EOK on success or error code
 */
int32_t eapi_wdt_trigger(eapi_handle eapi_dev);

/**
 * @brief Get capablities exposed by eapi modules
 *
 * @param eapi_dev  eapi device handle
 * @param caps      Optional Buffer to store capabilities
 * @return          EOK on success or error code
 */
int32_t eapi_get_module_caps(eapi_handle eapi_dev, eapi_caps *caps);

#endif
