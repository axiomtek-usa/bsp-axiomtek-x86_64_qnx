/*
* Copyright (c) 2026, BlackBerry Limited. All rights reserved.
*
* BlackBerry Limited and its licensors retain all intellectual property and
* proprietary rights in and to this software and related documentation. Any
* use, reproduction, disclosure or distribution of this software and related
* documentation without an express license agreement from BlackBerry Limited
* is strictly prohibited.
*/

#include "eapi.h"

/* Supported chips */
static const chip_tbl_t chip_tbl[] =
{
    { "Fintek",   CHIP_F81804  },
    { "F81804",   CHIP_F81804  },
    { "MEC",      CHIP_MEC170X },
    { "MEC170X",  CHIP_MEC170X },
};
/**
 * @brief              Find chip name to be used from the table.
 * @param optarg       string with chip name to search for
 *
 * @return             chip entry or NULL if a chip is not found.
 */
const chip_tbl_t *find_chip_entry(const char *optarg)
{
    uint32_t i;

    for (i = 0; i < NUM_ELTS(chip_tbl); i++) {
        if (strcasecmp(chip_tbl[i].name_str, optarg) == 0) {
            return &chip_tbl[i];
        }
    }
    return NULL;
}

/**
 *  @brief             Parses command-line options for the EAPI resmgr.
 *  @param eapi_dev_t     A pointer to the device structure where parsed
 * values are stored.
 *  @param argc        The number of command-line arguments
 *  @param argv        The array of command-line arguments.
 *
 *  @return            EOK on success, or an error code.
 */
int32_t eapi_parseopts(eapi_dev_t *dev, int32_t argc, char *argv[])
{
    int32_t c;
    int32_t status = EOK;
    uint32_t loglevel = _SLOG_ERROR;
    while (true) {
        c = getopt(argc, argv, "d:p:v");
        if (c == -1) {
            break; /* Opt list done */
        }
        switch ((char)c) {
        case 'd':
        {
            const chip_tbl_t *entry;

            entry = find_chip_entry(optarg);
            if (entry != NULL) {
                dev->chip_name = entry->chip_name;
            } else {
                eapi_slogf(loglevel, _SLOG_ERROR, "%s: Invalid entry name '%s' see usefile for supported chips\n",
                                                                        argv[0], optarg);
                status = EINVAL;
            }
            break;
        }
        case 'p':
        {
            errno = EOK;
            dev->priority = (int32_t)strtol(optarg, NULL, 0);
            if ((errno != 0) || (dev->priority <= 1) || (dev->priority >= 254))
            {
                eapi_slogf(loglevel, _SLOG_ERROR,
                      "%s: Invalid priority parameter\n", argv[0]);
                status = EINVAL;
            }
            break;
        }
        case 'v':
        {
            dev->verbose++;
            break;
        }
        default:
            eapi_slogf(loglevel, _SLOG_ERROR, "Unsupported option -%c\n", optopt);
            status = EINVAL;
            break;
        }
        if (status != EOK) {
            break;
        }
    }
    return status;
}

/**
 * @brief             EAPI resmgr modules init function.
 * @param eapi_dev_t  EAPI resmgr control struct.
 *
 * @return            EOK for success otherwise err code.
 */
int32_t eapi_chip_init(eapi_dev_t *dev, eapi_ops_g *eapi_ops)
{
    int32_t status = EOK;

    switch (dev->chip_name)
    {
        case CHIP_MEC170X:
        {
            get_firmware_version_s fw = {0};
            size_t nbytes = sizeof(fw);

            dev->io = MBX_BASE;
            status = mec_get_version(dev, &fw, &nbytes);
            if (status != EOK) {
                eapi_slogf(dev->verbose, _SLOG_ERROR,
                                "Requested chip not available, See usefile\n");
                return ENODEV;
            }
            eapi_slogf(dev->verbose, _SLOG_INFO,
                    "Firmware: v%d.%d\n", fw.version.firmwareMajorVersion,
                                            fw.version.firmwareMinorVersion);
            eapi_slogf(dev->verbose, _SLOG_INFO,
                    "Library : v%d.%d\n", fw.version.libraryMajorVersion,
                                            fw.version.libraryMinorVersion);
            eapi_slogf(dev->verbose, _SLOG_INFO,
                    "Timestamp: %s\n", fw.firmwareTimeStamp);

            eapi_ops->device_get_caps_cmd      = &mec_get_caps;
            eapi_ops->get_firmware_version_cmd = &mec_get_version;
            eapi_ops->wdt_config_cmd           = &mec_wdt_config;
            eapi_ops->wdt_start_cmd            = &mec_wdt_start;
            eapi_ops->wdt_refresh_cmd          = &mec_wdt_trigger;
            eapi_ops->wdt_stop_cmd             = &mec_wdt_stop;
            eapi_ops->access_i2c_cmd           = &mec_i2c_xfer;
            break;
            }
        case CHIP_F81804:
        {
            uint16_t id;

            id = detect_fintek_chip();

            /* F81804 chip ID check */
            if (id != SIO_F81804_ID) {
                eapi_slogf(dev->verbose, _SLOG_ERROR,
                    "F81804 not detected (found ID=0x%04X)\n", id);
                return ENODEV;
            }

            eapi_slogf(dev->verbose, _SLOG_INFO, "SuperIO Chip ID = 0x%04X (F81804)\n", id);

            dev->io = SIO_INDEX;
            eapi_ops->device_get_caps_cmd      = &f81804_get_caps;
            eapi_ops->wdt_config_cmd           = &f81804_set_wdt_config;
            eapi_ops->wdt_start_cmd            = &f81804_start_wdt;
            eapi_ops->wdt_refresh_cmd          = &f81804_reload_wdt;
            eapi_ops->wdt_stop_cmd             = &f81804_stop_wdt;
            break;
        }
        case CHIP_UNKNOWN:
        /* Other chips are not currently supported */
        default:
            eapi_slogf(dev->verbose, _SLOG_INFO,
                  "chip %d not supported\n", dev->chip_name);
            return ENODEV;
    }

    return status;
}

/**
 *  @brief             EAPI resmgr permission init function, create promgr
 *                     custom abilities.
 *  @param eapi_dev_t   Pointer to EAPI resmgr control struct.
 *
 *  @return            EOK for success otherwise errno for failure.
 */
int32_t eapi_perms_init(eapi_dev_t *dev)
{
    const char *const ability_names[EAPI_ABILITY_COUNT] = {
        [EAPI_ABIL_IDX_I2C_XFER] = EAPI_ABILITY_I2C_XFER,
        [EAPI_ABIL_IDX_WDT_CONFIG] = EAPI_ABILITY_WDT_CONFIG,
        [EAPI_ABIL_IDX_WDT_REFRESH] = EAPI_ABILITY_WDT_REFRESH,
    };

    /* Custom ability domains */
    const uint32_t ability_domains[EAPI_ABILITY_COUNT] = {
        [EAPI_ABIL_IDX_I2C_XFER] = PROCMGR_ADN_ROOT,
        [EAPI_ABIL_IDX_WDT_CONFIG] = PROCMGR_ADN_ROOT,
        [EAPI_ABIL_IDX_WDT_REFRESH] = PROCMGR_ADN_ROOT | PROCMGR_ADN_NONROOT,
    };

    for (int32_t i = 0; i < (int32_t)EAPI_ABILITY_COUNT; i++) {
        dev->abids[i] =
            procmgr_ability_create(ability_names[i], ability_domains[i]);
        if (dev->abids[i] <= -1) {
            eapi_slogf(dev->verbose, _SLOG_ERROR,
                  "procmgr_ability_create for '%s' failed: %s(%d)", ability_names[i],
                  strerror(-dev->abids[i]), -dev->abids[i]);
            return (-dev->abids[i]);
        }
    }

    return EOK;
}