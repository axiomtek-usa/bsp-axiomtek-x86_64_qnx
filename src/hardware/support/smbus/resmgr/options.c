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

#include <pci/pci.h>
#include <getopt.h>

#include "smbus_internal.h"
#include "log.h"


/**
 * @brief Process command line options
 *
 * @param opts      Pointer to the option container
 * @param argc      Number of command line arguments
 * @param argv      Array of command line arguments
 * @return int      EOK on success, EINVAL otherwise
 */
int smbus_options(smbus_options_t *opts, const int argc, char *argv[])
{
    /* Long Option List */
    static const struct option longopts[] = {
        {.name = "verbose", .has_arg = required_argument, .flag = NULL, .val = 'v' },
        {.name =NULL,       .has_arg = 0,                 .flag = NULL, .val = 0 }
    };

    int op;
    int long_idx;
    const char *token;

    /* Sanity check */
    if (opts == NULL) {
        return EINVAL;
    }

    /* Set default values */
    memset(opts, 0, sizeof(smbus_options_t));
    opts->verbose = SLOG2_ERROR;
    opts->pci_vid = PCI_VID_ANY;
    opts->pci_did = PCI_DID_ANY;

    /* Process options */
    while ((op = getopt_long(argc, argv, "D:I:i:o:P:pU:u:V:v", longopts, &long_idx)) != -1) {

        switch (op) {
            case 'D':    /* PCIe Device ID */
                if(optarg != NULL) {
                    errno = EOK;
                    opts->pci_did = (unsigned short)strtoul(optarg, NULL, 0);
                    if (errno != EOK) {
                        (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                    }
                }
                break;
            case 'I':    /* PCIe Instance */
                if(optarg != NULL) {
                    errno = EOK;
                    opts->pci_idx = (unsigned int)strtoul(optarg, NULL, 0);
                    if (errno != EOK) {
                        (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                    }
                }
                break;
            case 'i':    /* IRQ */
                if(optarg != NULL) {
                    errno = EOK;
                    opts->irq = (unsigned int)strtoul(optarg, NULL, 0);
                    if (errno != EOK) {
                        (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                    }
                }
                break;
            case 'o':    /* Device-Specific Options */
                if(optarg != NULL) {
                    opts->extra_opts = strdup(optarg);
                }
                break;
            case 'P':    /* Base Address ^ Size ^ Type */
                token = strsep(&optarg, "^");
                if (token != NULL) {
                    opts->base_addr = (paddr64_t)strtoul(token, NULL, 0);
                    token = strsep(&optarg, "^");
                    if (token != NULL) {
                        opts->base_size = (size_t)strtoul(token, NULL, 0);
                        token = strsep(&optarg, "^");
                        if (token != NULL) {
                            errno = EOK;
                            opts->base_type = (base_type_t)strtoul(token, NULL, 0);
                            if (errno != EOK) {
                                (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                            }
                        }
                    }
                }
                /*
                 * Validity check:
                 *  - Address and size are mandatory.
                 *  - Type is assumed to be MMIO if not specified
                 *  - Type should be bounded to base_type_t values
                 */
                if (((opts->base_addr != 0) && (opts->base_size == 0))
                    || ((opts->base_addr == 0) && (opts->base_size != 0))) {
                    return EINVAL;
                } else if ((opts->base_type < SMBUS_BASE_TYPE_FIRST_ENTRY)
                    || (opts->base_type > SMBUS_BASE_TYPE_LAST_ENTRY)) {
                    return EINVAL;
                } else {
                    ;   /* No actions */
                }
                break;
            case 'p':    /* Polling Mode */
                opts->polling_mode = 1;
                break;
            case 'U':    /* UID/GID */
                if(optarg != NULL) {
                    opts->uid_gid = strdup(optarg);
                }
                break;
            case 'u':     /* Unit */
                if(optarg != NULL) {
                    errno = EOK;
                    opts->unit = (unsigned int)strtoul(optarg, NULL, 0);
                    if (errno != EOK) {
                        (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                    }
                }
                break;
            case 'V':    /* PCIe Vendor ID */
                if(optarg != NULL) {
                    errno = EOK;
                    opts->pci_vid = (unsigned short)strtoul(optarg, NULL, 0);
                    if (errno != EOK) {
                        (void)fprintf(stderr, "strtoul failed with errno : %s \n", strerror(errno));
                    }
                }
                break;
            case 'v':    /* Verbosity Level */
                if (optarg != NULL) {
                    errno = EOK;
                    opts->verbose = (int)strtol(optarg, NULL, 0);
                    if (errno != EOK) {
                        (void)fprintf(stderr, "strtol failed with errno : %s \n", strerror(errno));
                    }
                } else {
                    opts->verbose++;
                }
                break;
            case 0:     /* Catch long options */
                break;
            default:
                return EINVAL;
        }
    }

    return EOK;
}

