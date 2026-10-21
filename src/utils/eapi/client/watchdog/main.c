/*
 * Copyright (c) 2026, BlackBerry Limited. All rights reserved.
 *
 * BlackBerry Limited and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation. Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from BlackBerry Limited
 * is strictly prohibited.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <sys/procmgr.h>
#include <sys/sched.h> 
#include <secpol/secpol.h>

#include "hw/eapi_client.h"
#include "eapi_wdtkick.h"

#define WDT_ROUND_UP_DIV(x, y) (((x) + ((y) - 1)) / (y))
#define WDT_SEC_TO_MIN_ROUND_UP(sec)    WDT_ROUND_UP_DIV((sec), 60U)


static int validate_options_wdt(const wdt_options_t *opts,
                                 bool event_timeout_provided,
                                 bool timeout_ms_provided)
{
    if (event_timeout_provided && (opts->sleep_ms >= opts->event_timeout_ms)) {
        LOGE("Kick time (%u ms) >= event timeout (%u ms), "
             "watchdog will expire before being kicked",
             opts->sleep_ms, opts->event_timeout_ms);
        return EINVAL;
    }

    if (timeout_ms_provided && (opts->sleep_ms >= opts->timeout_ms)) {
        LOGE("Kick time (%u ms) >= watchdog timeout (%u ms), "
             "watchdog will expire before being kicked",
             opts->sleep_ms, opts->timeout_ms);
        return EINVAL;
    }

    return EOK;
}


static int parse_options_wdt(int argc, char * const argv[], wdt_options_t *opts)
{
    int opt;
    bool event_timeout_provided = false;
    bool timeout_ms_provided   = false;

    if (opts == NULL) {
        return EINVAL;
    }

    /* Set defaults */
    opts->priority         = -1;
    opts->timeout_ms       = WDT_DEFAULT_TIMEOUT_MS;
    opts->initial_delay_ms = WDT_DEFAULT_DELAY_MS;
    opts->event_timeout_ms = WDT_DEFAULT_EVENT_TIMEOUT_MS;
    opts->reset_timeout_ms = WDT_DEFAULT_RESET_TIMEOUT_MS;
    opts->sleep_ms         = WDT_DEFAULT_SLEEP_MS;
    opts->reset_config     = WDT_EVENT_RESET;
    opts->refresh_pin      = 0xFF;
    
    opt = getopt(argc, argv, WDTKICK_OPTIONS);
    while (opt != -1) {
        switch ((char) opt) {
            case 'd':
            {
                errno = EOK;
                opts->initial_delay_ms = (uint32_t)strtoul(optarg, NULL, 0);
                if ((errno != EOK) ||
                    (opts->initial_delay_ms == 0) ||
                    (opts->initial_delay_ms > WDT_MAX_DELAY)) {
                    LOGE("Invalid initial delay parameter");
                    return EINVAL;
                }
                break;
            }

            case 'e':
            {
                errno = EOK;
                opts->event_timeout_ms = (uint32_t)strtoul(optarg, NULL, 0);
                if ((errno != EOK) ||
                    (opts->event_timeout_ms == 0) ||
                    (opts->event_timeout_ms > WDT_MAX_EVENT_TIMEOUT)) {
                    LOGE("Invalid event timeout parameter");
                    return EINVAL;
                }
                event_timeout_provided = true;
                break;
            }

            case 'g':
            {
                errno = EOK;
                uint8_t trigger_type = (uint8_t)strtoul(optarg, NULL, 0);
                if (errno != EOK) {
                    LOGE("invalid trigger_mode parameter");
                    return EINVAL;
                }

                switch (trigger_type)
                {
                    case WTD_MODE_ENABLE_SW_TRIGGER:
                        opts->refresh_pin = 0xFF;
                        break;
                    case WTD_MODE_ENABLE_HW_TRIGGER:
                        opts->refresh_pin = UNSUPPORTED;
                        break;
                    default:
                        LOGI("trigger_mode not supported");
                        break;
                }
                break;
            }

            case 'm':
            {
                errno = EOK;
                uint8_t mode = (uint8_t)strtoul(optarg, NULL, 0);
                if (errno != EOK) {
                    LOGE("Invalid mode-event parameter");
                    return EINVAL;
                }

                switch (mode)
                {
                    case WTD_MODE_SIGNAL:
                        opts->reset_config = WDT_EVENT_NONE;
                        break;
                    case WTD_MODE_PWRBTN_1SEC:
                        opts->reset_config = WDT_EVENT_PWRBTN_PULSE;
                        break;
                    case WTD_MODE_PWRBTN_4SEC:
                        opts->reset_config = WDT_EVENT_PWRBTN_FORCE;
                        break;
                    case WTD_MODE_SYSTEM_RESET:
                        opts->reset_config = WDT_EVENT_RESET;
                        break;
                    case WTD_MODE_COLD_RESET:
                    case WTD_MODE_WARM_RESET:
                    default:
                        LOGI("mode_event is not supported");
                        break;
                }
                break;
            }

            case 'p':
            {
                errno = EOK;
                opts->priority = (int32_t)strtol(optarg, NULL, 0);
                if ((errno != EOK) || (opts->priority <= 1) || (opts->priority >= 254)) {
                    LOGE("Invalid priority parameter");
                    return EINVAL;
                }
                break;
            }

            case 'r':
            {
                errno = EOK;
                opts->reset_timeout_ms = (uint32_t)strtoul(optarg, NULL, 0);
                if ((errno != EOK) ||
                    (opts->reset_timeout_ms == 0) ||
                    (opts->reset_timeout_ms > WDT_MAX_RESET_TIMEOUT)) {
                    LOGE("Invalid reset timeout parameter");
                    return EINVAL;
                }
                break;
            }

            case 's':
            {
                errno = EOK;
                opts->sleep_ms = (uint32_t)strtoul(optarg, NULL, 0);
                if ((errno != EOK) || (opts->sleep_ms == 0)) {
                    LOGE("Invalid sleep-period parameter");
                    return EINVAL;
                }
                break;
            }

            case 't':
            {
                errno = EOK;
                opts->timeout_ms = (uint32_t)strtoul(optarg, NULL, 0);
                if ((errno != EOK) ||
                    (opts->timeout_ms == 0) ||
                    (opts->timeout_ms > WDT_MAX_TIMEOUT_MS)) {
                    LOGE("Invalid timeout parameter");
                    return EINVAL;
                }
                timeout_ms_provided = true;
                break;
            }

            default:
                LOGI("Using default watchdog parameters");
                break;
        }

        opt = getopt(argc, argv, WDTKICK_OPTIONS);
    }

    return validate_options_wdt(opts, event_timeout_provided, timeout_ms_provided);
}


static void prepare_wdt_timeout(eapi_wdt_params_s *cfg, uint32_t timeout_ms)
{
    uint32_t timeout_sec = timeout_ms / 1000U;

    if (timeout_sec > WDT_MAX_SEC_VALUE) {
        cfg->time_unit = WDT_UNIT_MINUTES;
        cfg->wdt_timeout.timeout = (uint8_t)WDT_SEC_TO_MIN_ROUND_UP(timeout_sec);
    } else {
        cfg->time_unit = WDT_UNIT_SECONDS;
        cfg->wdt_timeout.timeout = (uint8_t)timeout_sec;
    }
}


static int32_t config_wdt(eapi_handle handle, uint32_t caps, wdt_options_t opts)
{
    eapi_wdt_params_s wdt_cfg_params = {0};

    if (caps & EAPI_CAPS_WDT_SIG_CTRL) {
        wdt_cfg_params.wdt_config.pulse_mode = EAPI_WDT_MODE_LEVEL;
        wdt_cfg_params.wdt_config.pulse_width = EAPI_WDT_PWIDTH_125MS;
        wdt_cfg_params.wdt_config.polarity = EAPI_WDT_POLARITY_ACTIVE_LOW;
    }

    if (caps & EAPI_CAPS_WDT_PIN_CTRL) {
        wdt_cfg_params.wdt_config.refresh_pin = opts.refresh_pin;
        wdt_cfg_params.wdt_config.signal_pin = WDT_TRIG_PIN;
    }

    if (caps & EAPI_CAPS_WDT_EVENT_CTRL) {
        wdt_cfg_params.wdt_config.reset_cfg = opts.reset_config;
        wdt_cfg_params.wdt_config.event_cfg = WDT_EVENT_NONE;
    }

    if (eapi_wdt_config(handle, &wdt_cfg_params, sizeof(wdt_cfg_params), NULL) == EOK) {
        LOGI("watchdog configuration successful");
    } else {
        LOGE("watchdog configuration failed");
        return -1;
    }

    return EOK;
}


static int32_t start_wdt(eapi_handle handle, uint32_t caps, wdt_options_t opts)
{
    eapi_wdt_params_s wdt_start_params = {0};

    if (caps & EAPI_CAPS_WDT_TIMEOUT_CTRL) {
        wdt_start_params.wdt_timeout.delay_timeout = opts.initial_delay_ms;
        wdt_start_params.wdt_timeout.event_timeout = opts.event_timeout_ms;
        wdt_start_params.wdt_timeout.reset_timeout = opts.reset_timeout_ms;
    } else {
        prepare_wdt_timeout(&wdt_start_params, opts.timeout_ms);
    }

    if (eapi_wdt_start(handle, &wdt_start_params, sizeof(wdt_start_params), NULL) == EOK) {
        LOGI("watchdog started successfully");
    } else {
        LOGE("watchdog start failed");
        return -1;
    }

    return EOK;
}

/**
 * @brief Entry point. Parses options, opens the eapi device,
 *        configures and starts the watchdog, then periodically kicks
 *        it until a trigger failure occurs.
 */
int main(int argc, char * const argv[])
{
    wdt_options_t opts;

    if (parse_options_wdt(argc, argv, &opts) != EOK) {
        exit(EXIT_FAILURE);
    }

    if (procmgr_daemon(EXIT_SUCCESS,
            PROCMGR_DAEMON_NOCLOSE | PROCMGR_DAEMON_NODEVNULL) == -1) {
        LOGE("procmgr_daemon failed");
        exit(EXIT_FAILURE);
    }

    if (opts.priority > 0) {
        if (pthread_setschedprio(pthread_self(), opts.priority) != EOK) {
            LOGE("cannot change thread priority");
            exit(EXIT_FAILURE);
        }
    }

    /* Open the eAPI device node */
    eapi_handle handle = calloc(1, sizeof(*handle));
    if (handle == NULL) {
        LOGE("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    if (eapi_open(handle, O_RDWR, NULL) != EOK) {
        LOGE("failed to open eAPI node: %s", strerror(errno));
        (void) eapi_close(handle);
        free(handle);      
        exit(EXIT_FAILURE);
    }

    /* Stop any running watchdog first */
    if (eapi_wdt_stop(handle) == EOK) {
        LOGI("watchdog timer stopped");
    } else {
        LOGE("failed to stop watchdog timer: %s", strerror(errno));
        (void) eapi_close(handle);
        free(handle);
        exit(EXIT_FAILURE);
    }

    /* Fetch watchdog capabilities from server */
    eapi_caps caps = 0U;
    if (eapi_get_module_caps(handle, &caps) == EOK) {
        LOGI("successfully retrieved caps");
    } else {
        LOGE("failed to retrieve caps: %s", strerror(errno));
        (void) eapi_close(handle);
        free(handle);
        exit(EXIT_FAILURE);
    }

    /* Configure watchdog timer */
    if (config_wdt(handle, caps, opts) == EOK) {
        LOGI("watchdog timer configuration done");
    } else {
        LOGE("failed to configure watchdog timer: %s", strerror(errno));
        (void) eapi_close(handle);
        free(handle);
        exit(EXIT_FAILURE);
    }

    /* Start watchdog timer with requested timeout */
    if (start_wdt(handle, caps, opts) == EOK) {
        LOGI("watchdog timer enabled");
    } else {
        LOGE("failed to enable watchdog timer: %s", strerror(errno));
        (void) eapi_close(handle);
        free(handle);
        exit(EXIT_FAILURE);
    }

    (void) secpol_transition_type(NULL, NULL, 0);

    /*
     * Periodically reload the watchdog timer.
     * This loop should never exit under normal operation.
     * Exiting means the watchdog trigger failed — return failure
     * to signal abnormal termination to the system.
     */
    while (true) {
        int ret = eapi_wdt_trigger(handle);
        LOGI("watchdog triggered");
        if (ret != EOK) {
            LOGE("watchdog reload failed: %s", strerror(errno));
            (void) eapi_wdt_stop(handle);
            break;
        }
        /* delay() is in milliseconds */
        (void) delay(opts.sleep_ms);
    }

    (void) eapi_close(handle);
    free(handle);

    return EXIT_FAILURE;
}
