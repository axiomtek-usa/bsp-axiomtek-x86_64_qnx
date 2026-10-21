/*
* Copyright (c) 2026, BlackBerry Limited. All rights reserved.
*
* BlackBerry Limited and its licensors retain all intellectual property and
* proprietary rights in and to this software and related documentation. Any
* use, reproduction, disclosure or distribution of this software and related
* documentation without an express license agreement from BlackBerry Limited
* is strictly prohibited.
*/

/***************************************************************************
 *
 * Support functions for EAPI ResMgr
 *
 **************************************************************************/
#include "eapi.h"

/**
 *  @brief             EAPI resmgr permission check function.
 *  @param ctp         Resource manager context info.
 *  @param abid        procmgr ability id.
 *  @param cached_abil ability cached per ocb.
 *
 *  @return            EOK for success otherwise errno for failure.
 */
int32_t eapi_check_perm(resmgr_context_t *ctp, int32_t abid, bool *cached_abil)
{
    if (*cached_abil) {
      return EOK;
    }

    struct _client_able able_check = {.ability = (uint32_t)abid};

    int32_t const ret =
        ConnectClientInfoAble(ctp->info.scoid, NULL, 0, &able_check, 1);
    if (ret == -1) {
        /* If anything goes wrong, default to denying access */
        eapi_slogf(0, _SLOG_ERROR,
              "ConnectClientInfoAble failed: %s", strerror(errno));
        return EPERM;
    }

    if (able_check.flags == 1U) {
        *cached_abil = true;
        return EOK;
    }
    else {
        return EPERM;
    }
}

/**
 *  @brief             EAPI resmgr buffer integrity check function, for get caps
 * devctl command.
 *  @param ctp         Resource manager context info.
 *  @param msg         IO message.
 *
 *  @return            EOK for success, err for failure.
 */
int32_t eapi_check_integrity_caps(resmgr_context_t *ctp, io_devctl_t *msg, void *priv)
{
    ssize_t need = (ssize_t)(sizeof(io_devctl_t) + (sizeof(eapi_caps)));

    if (need > ctp->info.dstmsglen) {
        eapi_slogf(_SLOG_ERROR, _SLOG_ERROR,
                       "Bad message: %s failed", __func__);
        return EBADMSG;
    }

    return EOK;
}

/**
 *  @brief             EAPI resmgr buffer integrity check function, for get ABI
 * devctl command.
 *  @param ctp         Resource manager context info.
 *  @param msg         IO message.
 *
 *  @return            EOK for success, err for failure.
 */
int32_t eapi_check_integrity_abi(resmgr_context_t *ctp, io_devctl_t *msg, void *priv)
{
    ssize_t need = (ssize_t)(sizeof(io_devctl_t) + (sizeof(uint32_t) * 2u));

    if (need > ctp->info.dstmsglen) {
        eapi_slogf(_SLOG_ERROR, _SLOG_ERROR,
                       "Bad message: %s failed", __func__);
        return EBADMSG;
    }

    return EOK;
}

/**
 *  @brief             EAPI resmgr buffer integrity check function, for i2c
 * devctl command.
 *  @param ctp         Resource manager context info.
 *  @param msg         IO message.
 *
 *  @return            EOK for success, err for failure.
 */
int32_t eapi_check_integrity_i2c(resmgr_context_t *ctp, io_devctl_t *msg, void *priv)
{
    /*Header-complete*/
    size_t need = sizeof(io_devctl_t) + sizeof(eapi_access_i2c_u);
    if (need > ctp->size) {
        eapi_slogf(_SLOG_ERROR, _SLOG_ERROR,
                       "Bad message: %s failed", __func__);
        return EBADMSG;
    }

    /*Payload-complete*/
    eapi_access_i2c_u *ih = _IO_INPUT_PAYLOAD(msg);
    need += ih->parameters.i2cWriteCount;
    if (need > ctp->size) {
        return EBADMSG;
    }

    return EOK;
}

/**
 *  @brief             EAPI resmgr buffer integrity check function, for watchdog
 * devctl command.
 *  @param ctp         Resource manager context info.
 *
 *  @return            EOK for success, err for failure.
 */
int32_t eapi_check_integrity_watchdog(resmgr_context_t *ctp, io_devctl_t *msg, void *priv)
{
    size_t need = sizeof(io_devctl_t) + sizeof(eapi_wdt_params_s);
    if (need > ctp->size) {
        eapi_slogf(_SLOG_ERROR, _SLOG_ERROR,
                       "Bad message: %s failed", __func__);
        return EBADMSG;
    }

    return EOK;
}

/**
 *  @brief             EAPI resmgr buffer integrity check function, for
 * version devctl command.
 *  @param ctp         Resource manager context info.
 *
 *  @return            EOK for success, err for failure.
 */
int32_t eapi_check_integrity_version(resmgr_context_t *ctp, io_devctl_t *msg, void *priv)
{
    ssize_t need = (ssize_t)(sizeof(io_devctl_t) + sizeof(get_firmware_version_s));
    if (need > ctp->info.dstmsglen) {
    eapi_slogf(_SLOG_ERROR, _SLOG_ERROR,
                        "Bad message: %s failed", __func__);
        return EBADMSG;
    }

    return EOK;
}

/**
 *  @brief             eapi slog print function.
 *  @param verbosity   The verbosity.
 *  @param fmt         The format string.
 *
 *  @details           This function logs a message using slog while
 *                     preserving the value of errno.
 *
 *  @return            Void.
 */
void eapi_slogf(const unsigned verbosity, const unsigned level, const char *fmt, ...)
{
    /* Save errno */
    int32_t const  saved_errno = errno;

    va_list     arg;

    if (level > verbosity) {
        return;
    }

    va_start(arg, fmt);
    (void)vslogf(__C(_SLOG_SYSLOG, (int32_t)0), (int32_t)level, fmt, arg);
    va_end(arg);

    /* Restore errno */
    errno = saved_errno;
    return;
}
