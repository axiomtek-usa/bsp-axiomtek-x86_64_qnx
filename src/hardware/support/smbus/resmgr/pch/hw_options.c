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
#include "hw.h"


/**
 * @brief Process command line device-specific options
 *
 * @param ctrl      SMBus controller context
 * @param options   String of device-specifc options
 * @return int      errno
 */
int hw_options(smbus_ctrl_t *const ctrl, char *const options)
{
    /*  Device-Specific Option List */
    static char *const hw_option_list[] = {
    #define HW_OPTION_LOCK   0
        "lock",         /* Lock timeout in us, 0 to disable */
        NULL
    };

    int err = EOK;
    char *optcpy;
    char *opt;
    char *value;

    /* Sanity check */
    assert(ctrl != NULL);

    /* Set default values */
    ctrl->lock_timeout = SMBUS_TRANSACTION_TIMEOUT_US;

    /* Check if any options are available */
    if (options == NULL) {
        /* No options */
        return EOK;
    }
    SMBUS_LOG_INFO("Device-Specific Options: %s", options);

    optcpy = strdup((const char*)options);
    if (optcpy == NULL) {
        return ENOMEM;
    }
    opt = optcpy;

    /* Process options */
    while ((opt != NULL) && (*opt != '\0')) {

        switch(getsubopt(&opt, hw_option_list, &value)) {

            case HW_OPTION_LOCK:    /* Lock Timeout */
                errno = EOK;
                ctrl->lock_timeout = (uint64_t)strtoul(value, NULL, 0);
                if (errno != EOK) {
                    (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                }
                SMBUS_LOG_INFO("Set Lock Timeout to %lu", ctrl->lock_timeout);
                break;

            default:
                SMBUS_LOG_ERROR("Unsupported device-specific option '%s'", value);
                err = EINVAL;
                break;
        }
    }

    free(optcpy);
    return err;
}

