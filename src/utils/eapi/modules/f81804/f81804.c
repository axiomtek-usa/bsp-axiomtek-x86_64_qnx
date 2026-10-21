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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <hw/inout.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <threads.h>
#include <sys/mman.h>

#include "f81804_def.h"

static f81804_wdt_ut config_wdt_g;
static uint8_t timeout_val_g;

static uint8_t superio_inb(uint8_t reg)
{
    out8(SIO_INDEX, reg);
    return in8(SIO_DATA);
}

static void superio_outb(uint8_t reg, uint8_t val)
{
    out8(SIO_INDEX, reg);
    out8(SIO_DATA, val);
    return;
}


static void superio_set_bit(uint8_t reg, int32_t bit)
{
    uint8_t val = superio_inb(reg);
    SETBIT(val, bit);
    superio_outb(reg, val);
}

static void superio_select(uint8_t ld)
{
    out8(SIO_INDEX, SIO_REG_LDSEL);
    out8(SIO_DATA, ld);
}

static void superio_enter(void)
{
    /* Key must be sent twice */
    out8(SIO_INDEX, SIO_UNLOCK_KEY);
    out8(SIO_INDEX, SIO_UNLOCK_KEY);
    return;
}

static void superio_exit(void)
{
    out8(SIO_INDEX, SIO_LOCK_KEY);
    return;
}

static void enable_watchdog_counting(void)
{
    uint8_t config_val;

    superio_enter();
    superio_select(SIO_F81804_LD_WDT);                  /* select watchdog logic device */
    superio_set_bit(F81804_WDT_PME_ENABLE_CFG_REG, 0); /* enable watchdog timeout output via WDTRST# */

    config_val = superio_inb(F81804_WDT_CTRL_CONFIG_REG);

    /* if watchdog timeout event occurred, write 1 to clear it */
    if ((config_val & BIT(WD_CONFIG_TO_STS)) != 0U) {
        SETBIT(config_val, WD_CONFIG_TO_STS);
    }

    SETBIT(config_val, WD_CONFIG_EN); /* enable watchdog counting */

    superio_outb(F81804_WDT_CTRL_CONFIG_REG, config_val);
    superio_exit();
}

static void disable_watchdog_counting(void)
{
    uint8_t config_val;

    superio_enter();
    superio_select(SIO_F81804_LD_WDT);  /* select watchdog logic device */
    superio_set_bit(F81804_WDT_PME_ENABLE_CFG_REG, 0);  /* disable watchdog PME output */

    config_val = superio_inb(F81804_WDT_CTRL_CONFIG_REG);

    /* if watchdog timeout event occurred, write 1 to clear it */
    if ((config_val & BIT(WD_CONFIG_TO_STS)) != 0U) {
        config_val |= BIT(WD_CONFIG_TO_STS);
    }

    /* disable watchdog counting */
    config_val &= (uint8_t)(~BIT(WD_CONFIG_EN));

    superio_outb(F81804_WDT_CTRL_CONFIG_REG, config_val);
    superio_exit();
}

int32_t f81804_set_wdt_config(eapi_dev_t *dev, void *dptr, size_t *nbytes)
{
    (void)get_params_f81804((eapi_wdt_params_s *)dptr, &config_wdt_g);
    *nbytes = sizeof(config_wdt_g);
    return f81804_set_wdt_config_hw(&config_wdt_g);
}

int32_t f81804_set_wdt_config_hw(f81804_wdt_ut *wd)
{
    uint8_t config_val;

    superio_enter();
    superio_select(SIO_F81804_LD_WDT);  /* select watchdog logic device */
    superio_set_bit(F81804_WDT_PME_ENABLE_CFG_REG, 0);  /* enable watchdog timeout output via WDTRST# */

    config_val = superio_inb(F81804_WDT_CTRL_CONFIG_REG);

    if (wd->pulse_mode == 1u) {
        SETBIT(config_val, WD_CONFIG_PULSE);
        config_val &= (uint8_t)~WD_CONFIG_PULSE_WIDTH_MASK;
        config_val |= (uint8_t)(wd->pulse_width & WD_CONFIG_PULSE_WIDTH_MASK);
    } else {
        CLEARBIT(config_val, WD_CONFIG_PULSE);
        CLEARBIT(config_val, WD_CONFIG_POLARITY);
        config_val |= (uint8_t)((uint8_t)wd->output_polarity << WD_CONFIG_POLARITY);
    }

    superio_outb(F81804_WDT_CTRL_CONFIG_REG, config_val);
    superio_exit();

    return EOK;
}

int32_t f81804_get_caps(eapi_dev_t *dev, void *dptr, size_t *nbytes)
{
    (void)dev;
    eapi_caps * caps = (eapi_caps *)dptr;
    *caps = EAPI_CAPS_NONE | EAPI_CAPS_WDT_TIME_UNIT_CTRL | EAPI_CAPS_WDT_SIG_CTRL;
    *nbytes = sizeof(eapi_caps);

    return EOK;
}

/*Parser for eapi to hw modules*/
void * get_params_f81804(const eapi_wdt_params_s *cfg, f81804_wdt_ut *wdt)
{
    wdt->output_polarity = cfg->wdt_config.polarity;
    wdt->pulse_mode = cfg->wdt_config.pulse_mode;
    wdt->pulse_width = cfg->wdt_config.pulse_width;
    wdt->time_unit = cfg->time_unit;
    timeout_val_g = wdt->timeout = cfg->wdt_timeout.timeout;    /* Does not support staged timeouts*/
    return wdt;
}

int32_t f81804_start_wdt(eapi_dev_t *dev, void *dptr, size_t *nbytes)
{
    if (get_params_f81804((eapi_wdt_params_s *)dptr, &config_wdt_g) != NULL) {
        *nbytes = sizeof(config_wdt_g);
    }
    return f81804_start_wdt_hw(&config_wdt_g);
}

int32_t f81804_start_wdt_hw(f81804_wdt_ut * wd)
{
    timeout_val_g = wd->timeout;
    uint8_t config_val;

    superio_enter();
    superio_select(SIO_F81804_LD_WDT);  // access watchdog logic unit
    superio_set_bit(F81804_WDT_PME_ENABLE_CFG_REG, 0);  // enable Watchdog time out output via WDTRST#
    superio_set_bit(F81804_WDT_CTRL_CONFIG_REG, (int32_t)WD_CONFIG_EN);

    /* configure watchdog timeout */
    config_val = superio_inb(F81804_WDT_CTRL_CONFIG_REG);
    if (wd->time_unit == 1u) {
        config_val |= BIT(WD_CONFIG_UNIT);
    } else {
        config_val &= ~BIT(WD_CONFIG_UNIT);
    }
    if(wd->pulse_mode == 1u) {
        /* Select "pulse" output mode with given duration */
        config_val |= BIT(WD_CONFIG_PULSE);

        /* Set WD_PSWIDTH bits (1:0) */
        config_val &= ~((uint8_t)WD_CONFIG_PULSE_WIDTH_MASK);
        config_val |= wd->pulse_width & WD_CONFIG_PULSE_WIDTH_MASK;
    } else {
        config_val &= ~BIT(WD_CONFIG_PULSE);
        config_val &= ~BIT(WD_CONFIG_POLARITY);
        config_val |= (uint8_t)wd->output_polarity << WD_CONFIG_POLARITY;
    }

    superio_outb(F81804_WDT_CTRL_CONFIG_REG, (uint8_t)config_val);
    superio_outb(F81804_WDT_TIMER_CFG_REG, timeout_val_g);
    superio_exit();

    enable_watchdog_counting();
    return EOK;
}

int32_t f81804_stop_wdt(eapi_dev_t *dev, void *dptr, size_t *nbytes)
{
    (void)dev;
    (void)dptr;
    (void)nbytes;
    return f81804_stop_wdt_hw();
}

int32_t f81804_stop_wdt_hw(void)
{
    disable_watchdog_counting();
    return EOK;
}

bool f81804_wdt_is_running(void)
{
    superio_enter();
    superio_select(SIO_F81804_LD_WDT);
    uint8_t wdt_conf = superio_inb(F81804_WDT_CTRL_CONFIG_REG);
    superio_exit();
    return ((wdt_conf & BIT(0)) != 0U) &&
            ((wdt_conf & BIT(WD_CONFIG_EN)) != 0U);
}

int32_t f81804_reload_wdt(eapi_dev_t *dev, void *dptr, size_t *nbytes)
{
    if (f81804_wdt_is_running() == false) {
        eapi_slogf(dev->verbose, _SLOG_WARNING,
                    "watchdog refresh: not running, failed to refresh.");
        return ECANCELED;
    }
    return f81804_reload_wdt_hw();
}

int32_t f81804_reload_wdt_hw(void)
{
    /* Re-enable with existing config */
    superio_enter();
    superio_select(SIO_F81804_LD_WDT);  // access watchdog logic module
    uint8_t config_val = superio_inb(F81804_WDT_CTRL_CONFIG_REG);
    superio_outb(F81804_WDT_CTRL_CONFIG_REG, config_val);
    superio_outb(F81804_WDT_TIMER_CFG_REG, timeout_val_g);
    superio_exit();
    return EOK;
}

uint16_t detect_fintek_chip(void)
{
    uint8_t id_lo, id_hi;
    uint16_t chip_id;

    superio_enter();

    /* Fintek ID registers */
    id_hi = superio_inb(SIO_F81804_REG_CHIP_ID_1);
    id_lo = superio_inb(SIO_F81804_REG_CHIP_ID_2);

    superio_exit();

    chip_id = (uint16_t)(((uint16_t)id_hi << 8u) | (uint16_t)id_lo);

    return chip_id;
}
