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

#include "smbus_internal.h"
#include "log.h"


/* Buffer to send messages to slogger2 */
static slog2_buffer_t slog2_buffer;

/*
 * Maximum verbosity allowed to be logged
 * Log to stderr by default with SLOG2_ERROR level
 * until smbus_log_init() is called.
 */
static int log_verbosity = -SLOG2_ERROR;


/**
 * @brief Initialize the slogger2 context for the resource manager
 *
 * @return int      0 on success, -1 otherwise
 */
static int init_slog2(uint8_t const verbosity)
{
    slog2_buffer_set_config_t config;
    int err = 0;

    memset(&config, 0, sizeof(slog2_buffer_set_config_t));
    config.buffer_set_name = __progname;            /* Use the program name */
    config.num_buffers = 1;                         /* Configure one slog buffer */
    config.verbosity_level = verbosity;             /* Allow maximum verbosity */
    config.buffer_config[0].buffer_name = "slog";   /* Default buffer name */
    config.buffer_config[0].num_pages = 1;          /* Use a page-size buffer */

    /* Register the slog2 buffer */
    if (slog2_register(&config, &slog2_buffer, SLOG2_QUIET) < 0) {
        /* Exit upon fatal error */
        (void)fprintf(stderr, "%s: Failed to initialize slog2 buffer: %s. Abort!\n",
            __progname, strerror(errno));
        err = -1;
    }
    return err;
}

/**
 * @brief Initialize resource manager logger
 *
 * The verbosity is capped to the slogger2 verbosity levels.
 * The verbosity shall be positive or null to log messages to slogger2.
 * If the verbosity is negative, then messages are logged to stderr
 * instead of slogger2.
 * If slogger2 was not started, and that the maximum verbosity level is greater
 * than 0, then the messages will be sent to stderr.
 *
 * @param verbosity      Maximum verbosity allowed to be logged.
 */
void smbus_log_init(int *verbosity)
{
    if (verbosity != NULL) {
        /* Cap verbosity level to valid slog2 levels */
        log_verbosity = min(max(abs(*verbosity), SLOG2_SHUTDOWN), SLOG2_DEBUG2);

        if (*verbosity >= 0) {
            if (init_slog2((uint8_t)log_verbosity) != EOK) {
                /* Default to stderr for logging */
                log_verbosity = -log_verbosity;
            }
        } else {
            /* Select stderr for logging */
            log_verbosity = -log_verbosity;
        }
        /* Return verbosity level */
        *verbosity = log_verbosity;
    }
}

/**
 * @brief Log a formatted message in slog buffer or to stderr
 *
 * @param severity      Verbosity level for the message
 * @param fmt           Formatted character string
 * @param ...
 */
void smbus_log(const uint8_t severity, const char *const fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);

    /*
     * Log message to stderr or slogger2
     */
    if (log_verbosity < 0) {
        /* Log message to stderr when verbosity is negative */

        if (severity <= (uint8_t)abs(log_verbosity)) {
            (void)vfprintf(stderr, fmt, arglist);
            (void)fprintf(stderr, "\n");
        }

    } else {
        /* Log message to slogger2 when verbosity is positive (or null) */

        if (slog2_buffer != NULL) {
            vslog2f(slog2_buffer, 0, severity, fmt, arglist);
        }
    }

    va_end(arglist);
}

