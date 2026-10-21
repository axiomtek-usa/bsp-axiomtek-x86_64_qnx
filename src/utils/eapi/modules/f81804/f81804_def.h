/*
* Copyright (c) 2026, BlackBerry Limited. All rights reserved.
*
* BlackBerry Limited and its licensors retain all intellectual property and
* proprietary rights in and to this software and related documentation. Any
* use, reproduction, disclosure or distribution of this software and related
* documentation without an express license agreement from BlackBerry Limited
* is strictly prohibited.
*/

#ifndef EAPI_M_F81804_H_
#define EAPI_M_F81804_H_

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

#include "eapi_modules.h"

/* Helper defines */
#define BIT(i) ((uint8_t)((uint8_t)1 << (i)))

#define SETBIT(val, bit)   ((val) |= BIT(bit))
#define CLEARBIT(val, bit) ((val) &= (uint8_t)~BIT(bit))

typedef struct f81804_wdt {
       uint8_t    timeout;          /* 1-255*/
       uint8_t    time_unit;        /* 0: second, 1: minute */
       uint8_t    pulse_width;      /* pulse width flag: 0-1ms, 1-25ms, 2-125ms, 3-5s */
       uint8_t    pulse_mode;       /* 0:level, 1: pulse */
       uint8_t    output_polarity;  /* 0: active low, 1: active high*/
}f81804_wdt_ut;

#define SIO_INDEX                  0x2E
#define SIO_DATA                   (SIO_INDEX + 1)

#define SIO_F81804_LD_WDT          0x07   /* Watchdog timer logical device */
#define SIO_UNLOCK_KEY             0x87   /* Key to enable Super-I/O */
#define SIO_LOCK_KEY               0xAA   /* Key to disable Super-I/O */

#define SIO_REG_LDSEL              0x07   /* Logical device select */
#define SIO_F81804_REG_CHIP_ID_1   0x20   /* Chip ID 1, default: 0x15h  */
#define SIO_F81804_REG_CHIP_ID_2   0x21   /* Chip ID 2, default: 0x02h  */
#define SIO_F81804_ID              0x1502u /* Chipset ID */


#define SIO_F81804_REG_CLOCK_SEL   0x26   /* Clock select  */


#define SIO_F81804_REG_PORT_SEL    0x27	/* Port select register*/


#define SIO_REG_TSI_LEVEL_SEL      0x28   /* TSI Level select */
#define SIO_REG_MFUNCT1            0x29   /* Multi function select 1 */
#define SIO_REG_MFUNCT2            0x2a   /* Multi function select 2 */
#define SIO_REG_MFUNCT3            0x2b   /* Multi function select 3 */

#define F81804_WDT_BASE_ADDR_REG_EN       0x30
#define WD_BASE_ADDR_EN_BIT               0

#define F81804_WDT_CTRL_CONFIG_REG 0xF5

#define WD_CONFIG_TO_STS           6u    /* This bit wil1 be set to 1 if watchdog
                                           timeout event occurred, write to clear */

#define WD_CONFIG_EN               5    /* counting of wdog: 0: disabled [default]
                                                             1: enabled         */

#define WD_CONFIG_PULSE            4u    /* select output mode: 0: level [default]
                                                                1: pulse        */
#define WD_CONFIG_UNIT             3u    /* select time unit 0: 1 sec [default]
                                                             1: 60 sec       */
#define WD_CONFIG_POLARITY         2u    /* 0: low active [default]
                                            1: high active       */

#define WD_OUTPUT_PULSE_WIDTH_1_MSEC      0u
#define WD_OUTPUT_PULSE_WIDTH_25_MSEC     1u
#define WD_OUTPUT_PULSE_WIDTH_125_MSEC    2u
#define WD_OUTPUT_PULSE_WIDTH_5_SEC       3u

#define WD_CONFIG_PULSE_WIDTH_MASK        0x03u

#define F81804_WDT_TIMER_CFG_REG          0xF6

#define F81804_WDT_PME_ENABLE_CFG_REG     0xFA

#define WD_WDOUT_EN                       0u

#define MAX_f81804_WDT_TIMEOUT            255u  // in minutes

/**
 * @brief            Detects Fintek chip and reads its ID register.
 * @return           Chip ID if detected, 0 otherwise.
 */
uint16_t detect_fintek_chip(void);

/**
 * @brief            Configures F81804 watchdog hardware settings.
 * @param wd         Watchdog configuration structure.
 *
 * @return           EOK on success, error code on failure.
 */
int32_t f81804_set_wdt_config_hw(f81804_wdt_ut *wd);

/**
 * @brief            Starts F81804 watchdog timer.
 * @param wd         Watchdog configuration with timeout (1-15300 seconds).
 * @return           EOK on success, error code on failure.
 */
int32_t f81804_start_wdt_hw(f81804_wdt_ut *wd);
/**
 * @brief            Stops F81804 watchdog timer.
 * @return           EOK
 */
int32_t f81804_stop_wdt_hw(void);

/**
 * @brief            Converts EAPI watchdog parameters to F81804 configuration.
 * @param cfg        Source EAPI watchdog parameters.
 * @param config_wdt Destination F81804 configuration.
 * @return           Pointer to config_wdt on success, NULL on failure.
 */
void * get_params_f81804(const eapi_wdt_params_s *cfg, f81804_wdt_ut *wdt);

/**
 * @brief            Checks if watchdog is running.
 * @return           true if running, false otherwise.
 */
bool f81804_wdt_is_running(void);

/**
 * @brief            Reloads watchdog timer to prevent timeout.
 * @return           EOK.
 */
int32_t f81804_reload_wdt_hw(void);

/**
 * @brief            Extracts watchdog parameters and invokes the hardware routine
 *                   to configure F81804 watchdog.
 * @param dev        EAPI device structure.
 * @param dptr       Kernel data buffer.
 * @param nbytes     Number of bytes.
 * @return    EOK on success, error code on failure.
 */
int32_t f81804_set_wdt_config(eapi_dev_t *dev, void *dptr, size_t *nbytes);

/**
 * @brief            Extracts watchdog parameters and invokes the hardware routine
 *                   to start F81804 watchdog.
 * @param dev        EAPI device structure.
 * @param dptr       Kernel data buffer.
 * @param nbytes     Number of bytes.
 * @return           EOK on success, error code on failure.
 */
int32_t f81804_start_wdt(eapi_dev_t *dev, void *dptr, size_t *nbytes);

/**
 * @brief            Invokes the hardware routine to reload F81804 watchdog.
 * @param dev        EAPI device structure.
 * @param dptr       Kernel data buffer.
 * @param nbytes     Number of bytes.
 * @return           EOK on success, error code on failure.
 */
int32_t f81804_reload_wdt(eapi_dev_t *dev, void *dptr, size_t *nbytes);

/**
 * @brief            Invokes the hardware routine to stop F81804 watchdog.
 * @param dev        EAPI device structure.
 * @param dptr       Kernel data buffer.
 * @param nbytes     Number of bytes.
 * @return           EOK on success, error code on failure.
 */
int32_t f81804_stop_wdt(eapi_dev_t *dev, void *dptr, size_t *nbytes);

/**
 *  @brief            capability retrieval function for f81804 module
 *
 *  @param dev        EAPI device structure.
 *  @param dptr       kernel data pointer.
 *  @param nbytes     number of bytes.
 *
 *  @return           EOK.
 */
int32_t f81804_get_caps(eapi_dev_t *dev, void *dptr, size_t *nbytes);

#endif
