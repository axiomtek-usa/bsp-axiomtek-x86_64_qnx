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
 * eapi_wdtkick.h
 * Client-side watchdog utility helper header.
 */

#ifndef EAPI_WDTKICK_H_
#define EAPI_WDTKICK_H_

#include <stdbool.h>
#include <stdint.h>
#include <devctl.h>
#include <sys/iomsg.h>
#include <sys/iomgr.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>


#define LOGI(fmt, ...) \
    (void) slogf(_SLOG_SETCODE(_SLOG_SYSLOG, _SLOG_INFO), _SLOG_INFO, \
          "[eapi-wdtkick] " fmt, ##__VA_ARGS__)

#define LOGE(fmt, ...) \
    (void) slogf(_SLOG_SETCODE(_SLOG_SYSLOG, _SLOG_ERROR), _SLOG_ERROR, \
          "[eapi-wdtkick] ERROR: " fmt, ##__VA_ARGS__)

#define LOGW(fmt, ...) \
    (void) slogf(_SLOG_SETCODE(_SLOG_SYSLOG, _SLOG_WARNING), _SLOG_WARNING, \
          "[eapi-wdtkick] WARN: " fmt, ##__VA_ARGS__)


/*
 * Command-line options for the watchdog utility
 */
#define WDTKICK_OPTIONS "d:e:g:m:p:r:s:t:"
typedef struct {
    int32_t  priority;            /**< Thread scheduling priority                        */
    uint8_t  reset_config;        /**< Reset behaviour (WDT_EVENT_*)                     */
    uint8_t  refresh_pin;         /**< Hardware refresh pin                              */
    uint32_t timeout_ms;          /**< Watchdog timeout, how long the wdog is armed for. */
    uint32_t initial_delay_ms;    /**< Delay before watchdog monitoring begins.          */
    uint32_t event_timeout_ms;    /**< Time until watchdog event is triggered.           */
    uint32_t reset_timeout_ms;    /**< Additional delay before system reset occurs.      */
    uint32_t sleep_ms;            /**< Kick/reload interval, how often the wdog reloads  */
} wdt_options_t;


#define WDT_TRIG_PIN          0x50             /**< If 0xFF not used */
#define WDT_MAX_TIMEOUT_MS    (255U * 60000U)

/*
 * Default values if user does not override via CLI
 */
#define WDT_DEFAULT_TIMEOUT_MS          80000U
#define WDT_DEFAULT_SLEEP_MS            40000U
#define WDT_MAX_SEC_VALUE               255U    /**< Max timeout in seconds; above this, minutes unit is used */

#define WDT_DEFAULT_DELAY_MS            100U
#define WDT_MAX_DELAY  		            0x7FFFFFFFU
#define WDT_DEFAULT_EVENT_TIMEOUT_MS    60000U
#define WDT_MAX_EVENT_TIMEOUT  	        0x7FFFFFFFU
#define WDT_DEFAULT_RESET_TIMEOUT_MS    29000U
#define WDT_MAX_RESET_TIMEOUT  	        0x7FFFFFFFU

/* Time unit */
#define WDT_UNIT_SECONDS    0
#define WDT_UNIT_MINUTES    1

/* Output mode */
#define EAPI_WDT_MODE_LEVEL    0    /**< Level output */
#define EAPI_WDT_MODE_PULSE    1    /**< Pulse output */

/* Output polarity (for level mode) */
#define EAPI_WDT_POLARITY_ACTIVE_LOW     0
#define EAPI_WDT_POLARITY_ACTIVE_HIGH    1

/* Pulse width (when in pulse mode) */
#define EAPI_WDT_PWIDTH_1MS      0
#define EAPI_WDT_PWIDTH_25MS     1
#define EAPI_WDT_PWIDTH_125MS    2
#define EAPI_WDT_PWIDTH_5S       3

/* Watchdog Event */
#define WTD_MODE_SIGNAL          0x01
#define WTD_MODE_SYSTEM_RESET    0x02
#define WTD_MODE_PWRBTN_1SEC     0x03
#define WTD_MODE_PWRBTN_4SEC     0x04
#define WTD_MODE_COLD_RESET      0x05
#define WTD_MODE_WARM_RESET      0x06

/* Watchdog Timeout/Delay time unit */
#define WTD_MODE_IN_MSEC               0x00
#define WTD_MODE_DELAY_IN_MINUTES      0x80
#define WTD_MODE_TIMEOUT_IN_MINUTES    0x40

/* Watchdog trigger type */
#define WTD_MODE_ENABLE_HW_TRIGGER     0x20
#define WTD_MODE_ENABLE_SW_TRIGGER     0x10

/* Watchdog Event mode*/
#define WDT_EVENT_NONE            0
#define WDT_EVENT_PWRBTN_PULSE    1
#define WDT_EVENT_PWRBTN_FORCE    2
#define WDT_EVENT_RESET           3

#define UNSUPPORTED    0XFF


/**
 * @brief Validates parsed watchdog options.
 *
 * @param opts                    Pointer to the options to validate.
 * @param event_timeout_provided  True if -e flag was provided.
 * @param timeout_ms_provided     True if -t flag was provided.
 * @return                        EOK on success, EINVAL on invalid options.
 */
static int validate_options_wdt(const wdt_options_t *opts, bool event_timeout_provided, bool timeout_ms_provided);

/**
 * @brief Parses command-line arguments into watchdog options.
 *
 * @param argc  Argument count from main().
 * @param argv  Argument vector from main().
 * @param opts  Output structure populated with parsed options.
 * @return      EOK on success, EINVAL on invalid or missing arguments.
 */
static int parse_options_wdt(int argc, char * const argv[], wdt_options_t *opts);

/**
 * @brief Converts timeout in milliseconds to the appropriate unit and value for
 *        the watchdog hardware.
 *
 * @param cfg         Watchdog parameter structure to populate.
 * @param timeout_ms  Timeout value in milliseconds.
 */
static void prepare_wdt_timeout(eapi_wdt_params_s *cfg, uint32_t timeout_ms);

/**
 * @brief Populates the watchdog payload struct (eapi_wdt_params_s) and then sends
 *        the configuration to the eAPI server via eapi_wdt_config().
 *
 * @param handle  handle to the eAPI device node.
 * @param caps    device capability flags (EAPI_CAPS_WDT_*).
 * @param opts    watchdog options.
 * @return        EOK on success, -1 on failure.
 */
static int32_t config_wdt(eapi_handle handle, uint32_t caps, wdt_options_t opts);

/**
 * @brief Populates eapi_wdt_params_s with timeout values and then starts the
 *        watchdog via eapi_wdt_start().
 *
 * @param handle  handle to the eAPI device node.
 * @param caps    device capability flags (EAPI_CAPS_WDT_*).
 * @param opts    watchdog options.
 * @return        EOK on success, -1 on failure.
 */
static int32_t start_wdt(eapi_handle handle, uint32_t caps, wdt_options_t opts);

#endif
