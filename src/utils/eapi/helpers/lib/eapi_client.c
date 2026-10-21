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
 * @file eapi_client.c
 * @brief EAPI Client Helper Implementation
 */

#include "hw/eapi_lib.h"

int32_t eapi_open(eapi_handle eapi_dev, int32_t flags, const char *path)
{
    const char *name = EAPI_DEVICE_PATH;

    if (eapi_dev == NULL) {
        errno = EINVAL;
        return -1;
    }

    int32_t accmode = flags & O_ACCMODE;

    /* Reject unsupported flags */
    if ((flags & ~O_ACCMODE) != 0) {
        errno = ENOTSUP;
        return -1;
    }

    /* O_WRONLY → O_RDWR: requires read perms for ABI validation */
    if (accmode == O_WRONLY) {
        accmode = O_RDWR;
    }

    /* Check if a custom path is provided */
    if (path != NULL) {
        name = path;
    }

    eapi_dev->fd = open(name, accmode);
    if (eapi_dev->fd == -1) {
        return -1;
    }

    /* Check Versioning */
    struct {
        uint32_t abi[2];
    } abi_data = {0};

    int32_t ret = devctl(eapi_dev->fd, DCMD_EAPI_GET_ABI, &abi_data, sizeof(abi_data), NULL);
    if (ret != EOK) {
        (void)close(eapi_dev->fd);
        errno = ret;
        return -1;
    }

    eapi_dev->abi_major = abi_data.abi[0];
    eapi_dev->abi_minor = abi_data.abi[1];


    if (eapi_dev->abi_major != EAPI_CLIENT_ABI_MAJOR) {
        (void)slogf(__C(_SLOG_SYSLOG, (int32_t)0), _SLOG_ERROR,
                                "Major version mismatch");
        (void)close(eapi_dev->fd);

        errno = ENOTSUP;
        return -1;
    }

    if (eapi_dev->abi_minor != EAPI_CLIENT_ABI_MINOR) {
        if (eapi_dev->abi_minor < EAPI_CLIENT_ABI_MINOR) {
            /* Server older than client → not supported */
            (void)slogf(__C(_SLOG_SYSLOG, 0), _SLOG_ERROR,
                            "Incompatible ABI: server %u.%u older than client %u.%u",
                            eapi_dev->abi_major, eapi_dev->abi_minor,
                            EAPI_CLIENT_ABI_MAJOR, EAPI_CLIENT_ABI_MINOR);

            (void)close(eapi_dev->fd);
            errno = ENOTSUP;
            return -1;
        } else {
            (void)slogf(__C(_SLOG_SYSLOG, 0), _SLOG_WARNING,
                            "Minor version mismatch: server %u.%u newer than client %u.%u",
                            eapi_dev->abi_major, eapi_dev->abi_minor,
                            EAPI_CLIENT_ABI_MAJOR, EAPI_CLIENT_ABI_MINOR);
        }
    }

    eapi_dev->abi_valid = true;

    return EOK;
}

int32_t eapi_close(eapi_handle eapi_dev)
{
    if (eapi_dev == NULL) {
        return EINVAL;
    }

    return close(eapi_dev->fd);
}

int32_t eapi_get_abi_version(eapi_handle eapi_dev, uint32_t *major, uint32_t *minor)
{
    if ((eapi_dev == NULL) || (major == NULL)
                           || (minor == NULL)) {
        return EINVAL;
    }

    if (!eapi_dev->abi_valid) {
        return EINVAL;
    }

    *major = eapi_dev->abi_major;
    *minor = eapi_dev->abi_minor;

    return EOK;
}

int32_t eapi_get_module_caps(eapi_handle eapi_dev, eapi_caps *caps)
{
    if (eapi_dev == NULL) {
        return EINVAL;
    }

    if (!eapi_dev->abi_valid) {
        return EINVAL;
    }

    eapi_caps caps_data = EAPI_CAPS_NONE;

    int32_t ret = devctl(eapi_dev->fd, DCMD_EAPI_GET_CAPS, &caps_data, sizeof(caps_data), NULL);
    if (ret != EOK) {

        return ret;
    }
    if (caps != NULL) {
        *caps = caps_data;
    }

    eapi_dev->eapi_module_caps = caps_data;

    return EOK;
}

int32_t eapi_get_firmware_version(eapi_handle eapi_dev, get_firmware_version_s *fw)
{
    if ((eapi_dev == NULL) || (fw == NULL)) {
        return EINVAL;
    }

    if (!eapi_dev->abi_valid) {
        return EINVAL;
    }

    return devctl(eapi_dev->fd, DCMD_EAPI_GETVER, fw, sizeof(*fw), NULL);
}

int32_t eapi_i2c_smbus_transfer(eapi_handle eapi_dev, void *dev_data_ptr, size_t nbytes, int32_t *dev_info_ptr)
{
    if (eapi_dev == NULL) {
        return EINVAL;
    }

    if (!eapi_dev->abi_valid) {
        return EINVAL;
    }

    return devctl(eapi_dev->fd, DCMD_EAPI_I2C_RD_WR, dev_data_ptr, nbytes, dev_info_ptr);
}

int32_t eapi_wdt_config(eapi_handle eapi_dev, void *dev_data_ptr, size_t nbytes, int32_t *dev_info_ptr)
{
    if (eapi_dev == NULL) {
        return EINVAL;
    }

    if (!eapi_dev->abi_valid) {
        return EINVAL;
    }

    return devctl(eapi_dev->fd, DCMD_EAPI_WDT_CONFIG, dev_data_ptr, nbytes, dev_info_ptr);
}

int32_t eapi_wdt_start(eapi_handle eapi_dev, void *dev_data_ptr, size_t nbytes, int32_t *dev_info_ptr)
{
    if (eapi_dev == NULL) {
        return EINVAL;
    }

    if (!eapi_dev->abi_valid) {
        return EINVAL;
    }

    return devctl(eapi_dev->fd, DCMD_EAPI_WDT_START, dev_data_ptr, nbytes, dev_info_ptr);
}

int32_t eapi_wdt_stop(eapi_handle eapi_dev)
{
    if (eapi_dev == NULL) {
        return EINVAL;
    }

    if (!eapi_dev->abi_valid) {
        return EINVAL;
    }

    return devctl(eapi_dev->fd, DCMD_EAPI_WDT_STOP, NULL, 0, NULL);
}

int32_t eapi_wdt_trigger(eapi_handle eapi_dev)
{
    if (eapi_dev == NULL) {
        return EINVAL;
    }

    if (!eapi_dev->abi_valid) {
        return EINVAL;
    }

    return devctl(eapi_dev->fd, DCMD_EAPI_WDT_TRIGGER, NULL, 0, NULL);
}
