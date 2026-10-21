/*
 * $QNXLicenseC:
 * Copyright 2025 QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http:www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <internal.h>
#include <sys/syspage.h>
#include <inttypes.h>

#ifdef SDIO_HC_MTK_MSDC_SDMMC
#include <mtk_msdc.h>

#define MTK_ERR_MASK (MTK_MSDC_INTEN_RSPCRCERR | MTK_MSDC_INTEN_CMDTMO |                                      \
                      MTK_MSDC_INTEN_ACMDCRCERR | MTK_MSDC_INTEN_ACMDTMO |                                    \
                      MTK_MSDC_INTEN_DATTMO | MTK_MSDC_INTEN_DATCRCERR |                                      \
                      MTK_MSDC_INTEN_DMA_BDCSERR | MTK_MSDC_INTEN_DMA_GPDCSERR | MTK_MSDC_INTEN_DMA_PROTECT | \
                      MTK_MSDC_INTEN_AXIRSPERR)

static void mtk_msdc_sdmmc_reset(const sdio_hc_t *const hc);

static int mtk_msdc_sdmmc_dma_setup(const sdio_hc_t *const hc, sdio_cmd_t *cmd);
static int mtk_msdc_sdmmc_timing(sdio_hc_t *hc, uint32_t timing);
static int mtk_msdc_sdmmc_hs400_tune(sdio_hc_t *hc);
static void msdc_set_timeout(const sdio_hc_t *const hc, const uint32_t ns, const uint32_t clks);
static int msdc_cmd_done(sdio_hc_t *hc, const uint32_t events);
static int msdc_data_xfer_done(sdio_hc_t *hc, const uint32_t events);
static void msdc_cmd_error(sdio_hc_t *hc, sdio_cmd_t *cmd, int status, const char *const func);
static void msdc_dma_reset(const sdio_hc_t *const hc);
static void msdc_fifo_reset(const sdio_hc_t *const hc, const char *const func);

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define MTK_SG_BUF_SIZE 128
#define MTK_MSDC_SIZE 0x700

#ifdef MTK_MSDC_DEBUG
static int mtk_msdc_reg_dump(sdio_hc_t *hc, const char *func, int line, int level)
{
    mtk_msdc_sdmmc_t *sdmmc = hc->cs_hdl;
    uintptr_t base = sdmmc->base;

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: CFG %x IOCON  %x PS %x", func,
                mtk_msdc_read32(base + MTK_MSDC_CFG),
                mtk_msdc_read32(base + MTK_MSDC_IOCON),
                mtk_msdc_read32(base + MTK_MSDC_PS));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: INT %x INTEN  %x FIFOCS %x", func,
                mtk_msdc_read32(base + MTK_MSDC_INT),
                mtk_msdc_read32(base + MTK_MSDC_INTEN),
                mtk_msdc_read32(base + MTK_MSDC_FIFOCS));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_SDC_CFG %x MTK_SDC_CMD %x MTK_SDC_ARG  %x MTK_SDC_STS %x", func,
                mtk_msdc_read32(base + MTK_SDC_CFG),
                mtk_msdc_read32(base + MTK_SDC_CMD),
                mtk_msdc_read32(base + MTK_SDC_ARG),
                mtk_msdc_read32(base + MTK_SDC_STS));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_SDC_RESP0 %x MTK_SDC_RESP1 %x MTK_SDC_RESP2 %x MTK_SDC_RESP3 %x", func,
                mtk_msdc_read32(base + MTK_SDC_RESP0),
                mtk_msdc_read32(base + MTK_SDC_RESP1),
                mtk_msdc_read32(base + MTK_SDC_RESP2),
                mtk_msdc_read32(base + MTK_SDC_RESP3));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_SDC_BLK_NUM %x MTK_SDC_VOL_CHG %x", func,
                mtk_msdc_read32(base + MTK_SDC_BLK_NUM),
                mtk_msdc_read32(base + MTK_SDC_VOL_CHG));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_SDC_CSTS %x MTK_SDC_CSTS_EN %x MTK_SDC_DATCRC_STS %x MTK_SDC_ADV_CFG0 %x", func,
                mtk_msdc_read32(base + MTK_SDC_CSTS),
                mtk_msdc_read32(base + MTK_SDC_CSTS_EN),
                mtk_msdc_read32(base + MTK_SDC_DATCRC_STS),
                mtk_msdc_read32(base + MTK_SDC_ADV_CFG0));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_EMMC_CFG0 %x MTK_EMMC_CFG1 %x MTK_EMMC_STS  %x MTK_EMMC_IOCON %x", func,
                mtk_msdc_read32(base + MTK_EMMC_CFG0),
                mtk_msdc_read32(base + MTK_EMMC_CFG1),
                mtk_msdc_read32(base + MTK_EMMC_STS),
                mtk_msdc_read32(base + MTK_EMMC_IOCON));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_SDC_ACMD_RESP %x MTK_SDC_ACMD19_TRG %x MTK_SDC_ACMD19_STS %x", func,
                mtk_msdc_read32(base + MTK_SDC_ACMD_RESP),
                mtk_msdc_read32(base + MTK_SDC_ACMD19_TRG),
                mtk_msdc_read32(base + MTK_SDC_ACMD19_STS));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: DMA_SA_H4 %x DMA_SA %x DMA_CA %x", func,
                mtk_msdc_read32(base + MTK_MSDC_DMA_SA_H4),
                mtk_msdc_read32(base + MTK_MSDC_DMA_SA),
                mtk_msdc_read32(base + MTK_MSDC_DMA_CA));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: DMA_CTRL %x DMA_CFG %x SW_DBG_SEL %x SW_DBG_OUT %x DMA_LENGTH %d", func,
                mtk_msdc_read32(base + MTK_MSDC_DMA_CTRL),
                mtk_msdc_read32(base + MTK_MSDC_DMA_CFG),
                mtk_msdc_read32(base + MTK_MSDC_SW_DBG_SEL),
                mtk_msdc_read32(base + MTK_MSDC_SW_DBG_OUT),
                mtk_msdc_read32(base + MTK_MSDC_DMA_LENGTH));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: PATCH_BIT0 %x PATCH_BIT1 %x PATCH_BIT2 %x", func,
                mtk_msdc_read32(base + MTK_MSDC_PATCH_BIT0),
                mtk_msdc_read32(base + MTK_MSDC_PATCH_BIT1),
                mtk_msdc_read32(base + MTK_MSDC_PATCH_BIT2));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_PAD_TUNE %x MTK_PAD_TUNE0 %x MTK_PAD_TUNE1 %x", func,
                mtk_msdc_read32(base + 0xec),
                mtk_msdc_read32(base + MTK_MSDC_PAD_TUNE0),
                mtk_msdc_read32(base + MTK_MSDC_PAD_TUNE1));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: DAT_RD_DLY0 %x DAT_RD_DLY1 %x DAT_RD_DLY2 %x DAT_RD_DLY3 %x", func,
                mtk_msdc_read32(base + MTK_MSDC_DAT_RD_DLY0),
                mtk_msdc_read32(base + MTK_MSDC_DAT_RD_DLY1),
                mtk_msdc_read32(base + MTK_MSDC_DAT_RD_DLY2),
                mtk_msdc_read32(base + MTK_MSDC_DAT_RD_DLY3));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_PAD_CTL0 %x MTK_PAD_DS_CTL0 %x MTK_PAD_DS_TUNE %x MTK_PAD_CMD_TUNE %x", func,
                mtk_msdc_read32(base + MTK_PAD_CTL0),
                mtk_msdc_read32(base + MTK_PAD_DS_CTL0),
                mtk_msdc_read32(base + MTK_PAD_DS_TUNE),
                mtk_msdc_read32(base + MTK_PAD_CMD_TUNE));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_PAD_DAT01_TUNE %x MTK_PAD_DAT23_TUNE %x MTK_PAD_DAT45_TUNE %x MTK_PAD_DAT67_TUNE %x", func,
                mtk_msdc_read32(base + MTK_PAD_DAT01_TUNE),
                mtk_msdc_read32(base + MTK_PAD_DAT23_TUNE),
                mtk_msdc_read32(base + MTK_PAD_DAT45_TUNE),
                mtk_msdc_read32(base + MTK_PAD_DAT67_TUNE));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_EMMC51_CFG0 %x MTK_EMMC50_CFG0 %x", func,
                mtk_msdc_read32(base + MTK_EMMC51_CFG0),
                mtk_msdc_read32(base + MTK_EMMC50_CFG0));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_EMMC50_CFG1 %x MTK_EMMC50_CFG2 %x MTK_EMMC50_CFG3 %x MTK_EMMC50_CFG4 %x", func,
                mtk_msdc_read32(base + MTK_EMMC50_CFG1),
                mtk_msdc_read32(base + MTK_EMMC50_CFG2),
                mtk_msdc_read32(base + MTK_EMMC50_CFG3),
                mtk_msdc_read32(base + MTK_EMMC50_CFG4));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, level, "%s: MTK_SDC_FIFO_CFG %x", func,
                mtk_msdc_read32(base + MTK_SDC_FIFO_CFG));

    return (EOK);
}
#endif

static int mtk_msdc_waitmask(const sdio_hc_t *const hc, const uint32_t reg, const uint32_t mask, const uint32_t val, uint32_t usec)
{
    uint32_t cnt;
    int err;
    uint32_t iter;
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

    err = ETIMEDOUT;
    if (usec == 0u) {
        usec = 1u;
    }

    /* fast poll for 1ms - 1us intervals, or first 1ms of longer intervals */
    for (cnt = min(usec, 1000u); cnt; cnt--) {
        if (((mtk_msdc_read32(base + reg)) & mask) == val) {
            err = EOK;
            break;
        }
        (void)nanospin_ns(1000L);
    }

    if ((usec > 1000u) && (err != EOK)) {
        iter = ((usec + 1000u - 1u) / 1000u) - 1u;
        for (cnt = iter; cnt; cnt--) {
            if (((mtk_msdc_read32(base + reg)) & mask) == val) {
                err = EOK;
                break;
            }
            (void)delay(1);
        }
    }

    return (err);
}

static int mtk_msdc_intr_event(sdio_hc_t *hc)
{
    sdio_cmd_t *cmd = hc->wspc.cmd;
    uint32_t events;
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

    events = mtk_msdc_read32(base + MTK_MSDC_INT);

    /* clear interrupts */
    mtk_msdc_write32(base + MTK_MSDC_INT, events);

    if (cmd != NULL) {
        if ((msdc_cmd_done(hc, events) == EOK) && (((cmd->flags & (SCF_CTYPE_ADTC | SCF_DATA_MSK)) == 0) || (msdc_data_xfer_done(hc, events) == EOK))) {
            (void)sdio_cmd_cmplt(hc, cmd, CS_CMD_CMP);
        }
    }

    mtk_msdc_write32(base + MTK_MSDC_INTEN, 0);

    return (EOK);
}

static int mtk_msdc_sdmmc_event(sdio_hc_t *hc, sdio_event_t *ev)
{
    int status;

    switch (ev->code) {
        case HC_EV_INTR:
            status = mtk_msdc_intr_event(hc);
            InterruptUnmask(0, hc->hc_iid);
            break;

        default:
            status = bs_event(hc, ev);
            break;
    }

    return (status);
}

static uint8_t msdc_dma_calcs(const uint8_t *const buf, const uint32_t len)
{
    uint32_t cnt, sum = 0;

    for (cnt = 0; cnt < len; cnt++) {
        sum += buf[cnt];
    }
    return (0xffU - (uint8_t)sum);
}

static int mtk_msdc_sdmmc_dma_setup(const sdio_hc_t *const hc, sdio_cmd_t *cmd)
{
    mtk_msdc_sdmmc_t *sdmmc = hc->cs_hdl;
    const struct msdc_dma *const dma = sdmmc->dma;
    const uintptr_t base = sdmmc->base;
    sdio_sge_t *sgp;
    int sgc;
    int sgi;
    int bdcnt = 0;
    int alen;
    int sg_count;
    uint64_t paddr;
    uint32_t dma_ctrl, flags;
    struct mt_gpdma_desc *gpd;
    struct mt_bdma_desc *bd;

    sgc = (int)cmd->sgc;
    sgp = cmd->sgl;

    if ((cmd->flags & SCF_DATA_PHYS) != SCF_DATA_PHYS) {
        sdio_vtop_sg(sgp, sdmmc->sgl, sgc, cmd->mhdl);
        sgp = sdmmc->sgl;
    }

    gpd = dma->gpd;
    bd = dma->bd;

    /* modify gpd */
    flags = gpd->flags;
    flags |= GPDMA_DESC_HWO | GPDMA_DESC_BDP;
    /* need to clear first. use these bits to calc checksum */
    flags &= ~GPDMA_DESC_CHECKSUM;
    gpd->flags = flags;
    gpd->flags |= (uint32_t)msdc_dma_calcs((uint8_t *)gpd, 16) << 8;

    /* modify bd */
    for (sgi = 0; sgi < sgc; sgi++) {
        paddr = sgp->sg_address;
        sg_count = (int)sgp->sg_count;

        while (sg_count) {
            if (bdcnt >= MTK_MAX_BD_NUM) {
                return (ENOTSUP);
            }
            alen = min(sg_count, BDMA_DESC_BUFLEN);
            /* init bd */
            flags = bd[bdcnt].flags;
            flags &= ~(BDMA_DESC_BLKPAD | BDMA_DESC_DWPAD | BDMA_DESC_PTRH4);
            flags |= (uint32_t)(paddr >> 32) << 28; /* high 4-bits of paddr */
            bd[bdcnt].ptr = (uint32_t)paddr;
            bd[bdcnt].bd_data_len = (uint32_t)alen;

            sg_count -= alen;
            paddr += (uint32_t)alen;
            if ((sgi == sgc - 1) && (sg_count == 0)) {
                flags |= BDMA_DESC_EOL; /* the last bd */
            } else {
                flags &= ~BDMA_DESC_EOL;
            }

            /* checksum need to clear first */
            flags &= ~BDMA_DESC_CHECKSUM;
            bd[bdcnt].flags = flags;
            bd[bdcnt].flags |= (uint32_t)msdc_dma_calcs((uint8_t *)(&bd[bdcnt]), 16) << 8;
            bdcnt++;
        }
        sgp++;
    }
    sdmmc_clr_bits(base + MTK_MSDC_CFG, MTK_MSDC_CFG_PIO);                /* Use dma mode */
    mtk_msdc_write32(base + MTK_MSDC_DMA_LENGTH, 0);                                 /* clear since only used for Basic DMA mode */
    sdmmc_set_field(base + MTK_MSDC_DMA_CFG, MTK_MSDC_DMA_CFG_DECSEN, 1); /* Enable descriptor checksum */

    dma_ctrl = mtk_msdc_read32(base + MTK_MSDC_DMA_CTRL);
    dma_ctrl &= ~(MTK_MSDC_DMA_CTRL_BRSTSZ | MTK_MSDC_DMA_CTRL_MODE | MTK_MSDC_DMA_CTRL_LASTBUF);
    dma_ctrl |= (((uint32_t)MTK_MSDC_DMA_CTRL_BRST_64B << MTK_MSDC_DMA_CTRL_BRSTSZ_SHIFT) | MTK_MSDC_DMA_CTRL_MODE); /* 64b burst, enable enhanced DMA descriptors */
    mtk_msdc_write32(base + MTK_MSDC_DMA_CTRL, dma_ctrl);

    mtk_msdc_write32(base + MTK_MSDC_DMA_SA, (uint32_t)dma->gpd_addr);
    mtk_msdc_write32(base + MTK_MSDC_DMA_SA_H4, (uint32_t)(dma->gpd_addr >> 32));

    return (EOK);
}

static int mtk_msdc_sdmmc_cd(sdio_hc_t *hc)
{
    return (CD_INS); /* external logic required */
}

#define mmc_resp_type(cmd) ((cmd)->flags & (SCF_RSP_PRESENT | SCF_RSP_136 | SCF_RSP_CRC | SCF_RSP_BUSY | SCF_RSP_OPCODE))
static inline uint32_t msdc_cmd_find_resp(const sdio_cmd_t *const cmd)
{
    uint32_t resp;

    switch (mmc_resp_type(cmd)) {
        case SCF_RSP_R1:
            resp = 0x1;
            break;
        case SCF_RSP_R1B:
            resp = 0x7;
            break;
        case SCF_RSP_R2:
            resp = 0x2;
            break;
        case SCF_RSP_R3:
            resp = 0x3;
            break;
        case SCF_RSP_NONE:
        default:
            resp = 0x0;
            break;
    }

    return resp;
}

/*
 * SD_IO_RW_EXTENDED argument format:
 *
 *      [31] R/W flag
 *      [30:28] Function number
 *      [27] Block mode
 *      [26] Increment address
 *      [25:9] Register address
 *      [8:0] Byte/block count
 */
static inline uint32_t msdc_cmd_prepare_raw_cmd(const sdio_hc_t *const hc, const sdio_cmd_t *const cmd)
{
    /* rawcmd :
     * vol_swt << 30 | auto_cmd << 28 | blklen << 16 | go_irq << 15 |
     * stop << 14 | rw << 13 | dtype << 11 | rsptyp << 7 | brk << 6 | opcode
     */
    const uint32_t opcode = cmd->opcode;
    const uint32_t resp = msdc_cmd_find_resp(cmd);
    uint32_t rawcmd = (opcode & 0x3fU) | ((resp & 0x7U) << 7U);

    if (opcode == (uint32_t)MMC_STOP_TRANSMISSION) {
        rawcmd |= ((uint32_t)0x1 << 14u);
    } else if (opcode == (uint32_t)SD_VOLTAGE_SWITCH) {
        rawcmd |= ((uint32_t)0x1 << 30u);
    } else {
        /* do nothing */
    }

    if ((cmd->flags & SCF_DATA_MSK) != 0u){
        if (cmd->blks > 1) {
            if ((hc->caps & (uint64_t)HC_CAP_ACMD23) && (cmd->flags & SCF_SBC)) {
                rawcmd |= ((uint32_t)0x2 << 28u); /* AutoCMD23 */
            } else if (hc->caps & (uint64_t)HC_CAP_ACMD12) {
                rawcmd |= ((uint32_t)0x1 << 28u); /* AutoCMD12 */
            }
            rawcmd |= ((uint32_t)0x2 << 11u);
        } else {
            rawcmd |= ((uint32_t)0x1 << 11u);
        }

        rawcmd |= ((cmd->blksz & 0xFFFu) << 16);
        if (cmd->flags & SCF_DIR_OUT) {
            rawcmd |= ((uint32_t)0x1 << 13u);
        }
    } else {
        /* do nothing */
    }
    return rawcmd;
}

static void msdc_fifo_reset(const sdio_hc_t *const hc, const char *const func)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

    sdmmc_set_bits(base + MTK_MSDC_FIFOCS, MTK_MSDC_FIFOCS_CLR); /* clear FIFO */
    if (mtk_msdc_waitmask(hc, MTK_MSDC_FIFOCS, MTK_MSDC_FIFOCS_CLR, 0, MTK_MSDC_CLOCK_TIMEOUT) != EOK) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: MTK_MSDC_FIFOCS_CLR error MTK_MSDC_FIFOCS 0x%x", func, mtk_msdc_read32(base + MTK_MSDC_FIFOCS));
    }
}

static void msdc_dma_reset(const sdio_hc_t *const hc)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

    sdmmc_set_bits(base + MTK_MSDC_PATCH_BIT1, MTK_MSDC_PATCH_BIT1_RESET_GDMA);
    (void)delay(1);
    sdmmc_clr_bits(base + MTK_MSDC_PATCH_BIT1, MTK_MSDC_PATCH_BIT1_RESET_GDMA);
    (void)delay(1);
}

static void msdc_dma_stop(const sdio_hc_t *const hc)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;
    int status = EOK;

    sdmmc_set_bits(base + MTK_MSDC_DMA_CTRL, MTK_MSDC_DMA_CTRL_STOP); /* stop DMA */
    status = mtk_msdc_waitmask(hc, MTK_MSDC_DMA_CFG, MTK_MSDC_DMA_CFG_STS, 0, MTK_MSDC_CLOCK_TIMEOUT);
    if (status != EOK) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: MTK_MSDC_DMA_CFG_STS failed error MTK_MSDC_DMA_CFG 0x%x MTK_MSDC_DMA_CTRL 0x%x, resetting DMA.", __func__,
        mtk_msdc_read32(base + MTK_MSDC_DMA_CFG), mtk_msdc_read32(base + MTK_MSDC_DMA_CTRL));
        msdc_dma_reset(hc);
    }
}

static void msdc_start_data(const sdio_hc_t *const hc, sdio_cmd_t *cmd)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

    if ((mtk_msdc_read32(base + MTK_MSDC_DMA_CFG) & MTK_MSDC_DMA_CFG_STS) == MTK_MSDC_DMA_CFG_STS) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: error, DMA already active  cmd=%d DMA data: %d blocks; cmd->flags=%d DMA_CFG 0x%x DMA_CTRL 0x%x",
                   __func__, cmd->opcode, cmd->blks, cmd->flags, mtk_msdc_read32(base + MTK_MSDC_DMA_CFG), mtk_msdc_read32(base + MTK_MSDC_DMA_CTRL));
        msdc_dma_stop(hc);
    }

    mtk_msdc_write32(base + MTK_SDC_BLK_NUM, cmd->blks);
    (void)mtk_msdc_sdmmc_dma_setup(hc, cmd);

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 3, "%s: DMA start  cmd=%d DMA data: %d blocks; cmd->flags=0x%x",
               __func__, cmd->opcode, cmd->blks, cmd->flags);
}

static void mtk_msdc_sdmmc_reset(const sdio_hc_t *const hc)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;
    int status, tryagain = 0;

    if ((mtk_msdc_read32(base + MTK_MSDC_DMA_CFG) & MTK_MSDC_DMA_CFG_STS) == MTK_MSDC_DMA_CFG_STS) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 0, "%s: MTK_MSDC_DMA_CFG_STS DMA active error MTK_MSDC_DMA_CFG 0x%x MTK_MSDC_DMA_CTRL 0x%x", __func__, mtk_msdc_read32(base + MTK_MSDC_DMA_CFG), mtk_msdc_read32(base + MTK_MSDC_DMA_CTRL));
        sdmmc_set_bits(base + MTK_MSDC_DMA_CTRL, MTK_MSDC_DMA_CTRL_STOP); /* stop DMA */
        status = mtk_msdc_waitmask(hc, MTK_MSDC_DMA_CFG, MTK_MSDC_DMA_CFG_STS, 0, MTK_MSDC_CLOCK_TIMEOUT);
        if (status != EOK) {
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: MTK_MSDC_DMA_CFG_STS failed error MTK_MSDC_DMA_CFG 0x%x", __func__, mtk_msdc_read32(base + MTK_MSDC_DMA_CFG));
            tryagain++;
        }
    }

    sdmmc_set_bits(base + MTK_MSDC_CFG, MTK_MSDC_CFG_RST);
    status = mtk_msdc_waitmask(hc, MTK_MSDC_CFG, MTK_MSDC_CFG_RST, 0, MTK_MSDC_CLOCK_TIMEOUT);
    if (status != EOK) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: MTK_MSDC_CFG_RST error MTK_MSDC_CFG 0x%x", __func__, mtk_msdc_read32(base + MTK_MSDC_CFG));
    }

    if (tryagain != 0) {
        status = mtk_msdc_waitmask(hc, MTK_MSDC_DMA_CFG, MTK_MSDC_DMA_CFG_STS, 0, MTK_MSDC_CLOCK_TIMEOUT);
        if (status == EOK) {
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 0, "%s: MTK_MSDC_DMA_CFG_STS wait suceeded after CFG_RST MTK_MSDC_DMA_CFG 0x%x", __func__, mtk_msdc_read32(base + MTK_MSDC_DMA_CFG));
            tryagain = 0;
        }
    }

    msdc_fifo_reset(hc, __func__);

    if (tryagain != 0) {
        status = mtk_msdc_waitmask(hc, MTK_MSDC_DMA_CFG, MTK_MSDC_DMA_CFG_STS, 0, MTK_MSDC_CLOCK_TIMEOUT);
        if (status == EOK) {
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 0, "%s: MTK_MSDC_DMA_CFG_STS wait suceeded after CFG_RST+FIFOCS_CLR MTK_MSDC_DMA_CFG 0x%x", __func__, mtk_msdc_read32(base + MTK_MSDC_DMA_CFG));
        } else {
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: MTK_MSDC_DMA_CFG_STS wait failed after CFG_RST+FIFOCS_CLR MTK_MSDC_DMA_CFG 0x%x, resetting DMA", __func__, mtk_msdc_read32(base + MTK_MSDC_DMA_CFG));
            msdc_dma_reset(hc);
        }

        /* clear FIFO again since DMA may have run between last clear and DMA STOP */
        if (((mtk_msdc_read32(base + MTK_MSDC_FIFOCS) & MTK_MSDC_FIFOCS_TXCNT) >> 16) || (mtk_msdc_read32(base + MTK_MSDC_FIFOCS) & MTK_MSDC_FIFOCS_RXCNT)) {
            msdc_fifo_reset(hc, __func__);
        }
    }
}

static void msdc_cmd_error(sdio_hc_t *hc, sdio_cmd_t *cmd, const int status, const char *const func)
{
    const mtk_msdc_sdmmc_t *sdmmc;
    uintptr_t base;
    int res;

    sdmmc = hc->cs_hdl;
    base = sdmmc->base;

    if ((cmd->flags & (SCF_CTYPE_ADTC | SCF_DATA_MSK)) != 0u) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: %s error, stop DMA: MTK_MSDC_DMA_CFG 0x%x MTK_MSDC_DMA_CTRL 0x%x", __func__, func, mtk_msdc_read32(base + MTK_MSDC_DMA_CFG), mtk_msdc_read32(base + MTK_MSDC_DMA_CTRL));
        sdmmc_set_bits(base + MTK_MSDC_DMA_CTRL, MTK_MSDC_DMA_CTRL_STOP); /* stop DMA */
        res = mtk_msdc_waitmask(hc, MTK_MSDC_DMA_CFG, MTK_MSDC_DMA_CFG_STS, 0, MTK_MSDC_CLOCK_TIMEOUT);
        if (res != EOK) {
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: %s MTK_MSDC_DMA_CFG_STS stop DMA failed error MTK_MSDC_DMA_CFG 0x%x", __func__, func, mtk_msdc_read32(base + MTK_MSDC_DMA_CFG));
            mtk_msdc_sdmmc_reset(hc);
        }
    }
    (void)sdio_cmd_cmplt(hc, cmd, (uint32_t)status);
}

static int msdc_auto_cmd_done(sdio_hc_t *hc, const uint32_t events, sdio_cmd_t *cmd)
{
    uint32_t *const rsp = cmd->rsp;
    mtk_msdc_sdmmc_t *sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

    rsp[0] = mtk_msdc_read32(base + MTK_SDC_ACMD_RESP);

    if ((events & MTK_MSDC_INT_ACMDRDY) == MTK_MSDC_INT_ACMDRDY) {
        sdmmc->cmd_error = 0;
    } else {
        if ((events & MTK_MSDC_INT_ACMDCRCERR) == MTK_MSDC_INT_ACMDCRCERR) {
            sdmmc->cmd_error = EILSEQ;
            sdmmc->error |= MTK_REQ_STOP_EIO;
            msdc_cmd_error(hc, cmd, CS_CMD_CRC_ERR, __func__);
        } else if ((events & MTK_MSDC_INT_ACMDTMO) == MTK_MSDC_INT_ACMDTMO) {
            sdmmc->cmd_error = ETIMEDOUT;
            sdmmc->error |= MTK_REQ_STOP_TMO;
            msdc_cmd_error(hc, cmd, CS_CMD_TO_ERR, __func__);
        } else {
            sdmmc->cmd_error = EIO;
            sdmmc->error |= MTK_REQ_STOP_EIO;
            msdc_cmd_error(hc, cmd, CS_CMD_CMP_ERR, __func__);
        }
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: AUTO_CMD%d arg=%08X; rsp %08X; cmd_error=%d",
                   __func__, cmd->opcode, cmd->arg, rsp[0], sdmmc->cmd_error);
    }
    return sdmmc->cmd_error;
}

static int msdc_cmd_done(sdio_hc_t *hc, const uint32_t events)
{
    sdio_cmd_t *cmd = hc->wspc.cmd;
    uint32_t *const rsp = cmd->rsp;
    mtk_msdc_sdmmc_t *sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

    if ((events & (MTK_MSDC_INT_ACMDRDY | MTK_MSDC_INT_ACMDCRCERR | MTK_MSDC_INT_ACMDTMO)) && (msdc_auto_cmd_done(hc, events, cmd) != EOK)) {
        return sdmmc->cmd_error;
    }

    if (cmd->flags & SCF_RSP_PRESENT) {
        if (cmd->flags & SCF_RSP_136) {
            rsp[0] = mtk_msdc_read32(base + MTK_SDC_RESP3);
            rsp[1] = mtk_msdc_read32(base + MTK_SDC_RESP2);
            rsp[2] = mtk_msdc_read32(base + MTK_SDC_RESP1);
            rsp[3] = mtk_msdc_read32(base + MTK_SDC_RESP0);
        } else {
            rsp[0] = mtk_msdc_read32(base + MTK_SDC_RESP0);
        }
    }

    if ((events & MTK_MSDC_INT_CMDRDY) != MTK_MSDC_INT_CMDRDY) {
        if (cmd->opcode != (uint32_t)MMC_SEND_TUNING_BLOCK) {
            /*
             * should not clear fifo/interrupt as the tune data
             * may have alreay come.
             */
            mtk_msdc_sdmmc_reset(hc);
        }

        if ((events & MTK_MSDC_INT_RSPCRCERR) == MTK_MSDC_INT_RSPCRCERR) {
            sdmmc->cmd_error = EILSEQ;
            sdmmc->error |= MTK_REQ_CMD_EIO;
            msdc_cmd_error(hc, cmd, CS_CMD_CRC_ERR, __func__);
        }  else if ((events & MTK_MSDC_INT_CMDTMO) == MTK_MSDC_INT_CMDTMO) {
            sdmmc->cmd_error = ETIMEDOUT;
            sdmmc->error |= (int)MTK_REQ_CMD_TMO;
            msdc_cmd_error(hc, cmd, CS_CMD_TO_ERR, __func__);
        } else {
            sdmmc->cmd_error = EIO;
            sdmmc->error |= (int)MTK_REQ_CMD_EIO;
            msdc_cmd_error(hc, cmd, CS_CMD_CMP_ERR, __func__);
        }
    }
    if (sdmmc->cmd_error != EOK) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: CMD%d arg=%08X; rsp %08X; cmd_error=%d; events=0x%x",
                   __func__, cmd->opcode, cmd->arg, rsp[0], sdmmc->cmd_error, events);
    }
    return sdmmc->cmd_error;
}

/* It is the core layer's responsibility to ensure card status
 * is correct before issue a request. but host design do below
 * checks recommended.
 */
static inline int msdc_cmd_is_ready(sdio_hc_t *hc, const sdio_cmd_t *const cmd)
{
    int status;
    mtk_msdc_sdmmc_t *sdmmc;

    sdmmc = hc->cs_hdl;

    /* The max busy time we can endure is 20ms */
    status = mtk_msdc_waitmask(hc, MTK_SDC_STS, MTK_SDC_STS_CMDBUSY, 0, 20000); /* 20ms */
    if (status != EOK) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: EMMC: CMD %d cannot execute because CMD busy", __func__, cmd->opcode);
        sdmmc->error |= (int)MTK_REQ_CMD_BUSY;
        (void)msdc_cmd_done(hc, (int)MTK_MSDC_INT_CMDTMO);
        return (EAGAIN); /* ETIMEDOUT false */
    }

    if ((mmc_resp_type(cmd) == SCF_RSP_R1B) || (cmd->flags & SCF_DATA_MSK)) {
        /* R1B or with data, should check SDCBUSY */
        status = mtk_msdc_waitmask(hc, MTK_SDC_STS, MTK_SDC_STS_SDCBUSY, 0, 20000); /* 20ms */
        if (status != EOK) {
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s:%d EMMC: CMD %d cannot execute because SDC busy", __func__, __LINE__, cmd->opcode);
            sdmmc->cmd_error |= (int)MTK_REQ_CMD_BUSY;
            (void)msdc_cmd_done(hc, (int)MTK_MSDC_INT_CMDTMO);
            return (EAGAIN); /* ETIMEDOUT */
        }
    }
    return (EOK);
}

static int mtk_msdc_sdmmc_cmd(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    uint32_t rawcmd;
    mtk_msdc_sdmmc_t *sdmmc;
    uintptr_t base;
    int ret;
    const uint32_t cmd_ints_mask = MTK_MSDC_INTEN_CMDRDY;
    const uint32_t data_ints_mask = MTK_MSDC_INTEN_XFER_COMPL;

    sdmmc = hc->cs_hdl;
    base = sdmmc->base;

    ret = msdc_cmd_is_ready(hc, cmd);
    if (ret != EOK) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "^^^^^^%s:%d CMD %d  not ready", __func__, __LINE__, cmd->opcode);
        return (ret);
    }

    if (((mtk_msdc_read32(base + MTK_MSDC_FIFOCS) & MTK_MSDC_FIFOCS_TXCNT) >> 16) || (mtk_msdc_read32(base + MTK_MSDC_FIFOCS) & MTK_MSDC_FIFOCS_RXCNT)) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: TX/RX FIFO non-empty before start of IO (TX %d, RX %d). Resetting", __func__,
                   (mtk_msdc_read32(base + MTK_MSDC_FIFOCS) & MTK_MSDC_FIFOCS_TXCNT) >> 16, mtk_msdc_read32(base + MTK_MSDC_FIFOCS) & MTK_MSDC_FIFOCS_RXCNT);
        mtk_msdc_sdmmc_reset(hc);
    }

    sdmmc->cmd_error = 0;
    rawcmd = msdc_cmd_prepare_raw_cmd(hc, cmd);

    mtk_msdc_write32(base + MTK_MSDC_INT, mtk_msdc_read32(base + MTK_MSDC_INT)); /* clear any pending ints */

    if (cmd->flags & (SCF_CTYPE_ADTC | SCF_DATA_MSK)) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 3, "%s:%d data CMD %d rawcmd 0x%x arg 0x%x", __func__, __LINE__, cmd->opcode, rawcmd, cmd->arg);
        msdc_start_data(hc, cmd);
        sdmmc_set_bits(base + MTK_MSDC_INTEN, MTK_ERR_MASK | data_ints_mask);
    } else {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 3, "%s:%d      CMD %d rawcmd 0x%x arg 0x%x", __func__, __LINE__, cmd->opcode, rawcmd, cmd->arg);
        sdmmc_set_bits(base + MTK_MSDC_INTEN, MTK_ERR_MASK | cmd_ints_mask);
    }

    mtk_msdc_write32(base + MTK_SDC_ARG, cmd->arg);
    mtk_msdc_write32(base + MTK_SDC_CMD, rawcmd);

    if (cmd->flags & (SCF_CTYPE_ADTC | SCF_DATA_MSK)) {
        sdmmc_set_bits(base + MTK_MSDC_DMA_CTRL, MTK_MSDC_DMA_CTRL_START);
    }

    return (EOK);
}

static int msdc_data_xfer_done(sdio_hc_t *hc, const uint32_t events)
{
    int status = EOK;
    mtk_msdc_sdmmc_t *sdmmc;
    uintptr_t base;

    sdio_cmd_t *cmd = hc->wspc.cmd;
    sdmmc = hc->cs_hdl;
    base = sdmmc->base;

    if (mtk_msdc_read32(base + MTK_MSDC_DMA_CFG) & MTK_MSDC_DMA_CFG_STS) {
        sdmmc_set_bits(base + MTK_MSDC_DMA_CTRL, MTK_MSDC_DMA_CTRL_STOP); /* stop DMA */
        status = mtk_msdc_waitmask(hc, MTK_MSDC_DMA_CFG, MTK_MSDC_DMA_CFG_STS, 0, 20000);
        if (status != EOK) {
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: error DMA stop failed: DMA_CFG: 0x%x", __func__, mtk_msdc_read32(base + MTK_MSDC_DMA_CFG));
            msdc_cmd_error(hc, cmd, CS_DATA_TO_ERR, __func__);
            return status;
        }
    }

    if (((events & MTK_MSDC_INT_XFER_COMPL) != MTK_MSDC_INT_XFER_COMPL))
    {
        mtk_msdc_sdmmc_reset(hc);
        sdmmc->error |= MTK_REQ_DAT_ERR;
        if ((events & MTK_MSDC_INT_DATTMO) == MTK_MSDC_INT_DATTMO){
            status = ETIMEDOUT;
            msdc_cmd_error(hc, cmd, CS_DATA_TO_ERR, __func__);
        } else if ((events & MTK_MSDC_INT_DATCRCERR) == MTK_MSDC_INT_DATCRCERR){
            status = EILSEQ;
            msdc_cmd_error(hc, cmd, CS_DATA_CRC_ERR, __func__);
        } else {
            status = EIO;
            msdc_cmd_error(hc, cmd, CS_DATA_END_ERR, __func__);
        }
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: cmd=%d; blocks=%d data_error=%d ", __func__, cmd->opcode, cmd->blks, cmd->status);
    }

    return (status);
}

static int mtk_msdc_sdmmc_abort(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: hc idx%d base_addr 0x%x cmd%d", __func__, hc->cfg.idx, (unsigned int)hc->cfg.base_addr[0], cmd->opcode);

#ifdef MTK_MSDC_DEBUG
    (void)mtk_msdc_reg_dump(hc, __func__, __LINE__, 0);
#endif

    mtk_msdc_sdmmc_reset(hc);

    return (EOK);
}

static void msdc_hw_reset(const sdio_hc_t *const hc)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

    /* Hardware reset */
    mtk_msdc_write32(base + MTK_EMMC_IOCON, MTK_EMMC_IOCON_BOOT_RST);
    (void)delay(1);
    mtk_msdc_write32(base + MTK_EMMC_IOCON, 0);
    (void)delay(1);

    if (mtk_msdc_waitmask(hc, MTK_EMMC_IOCON, MTK_EMMC_IOCON_BOOT_RST, 0, 20000) != EOK) { /* 20ms */
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: EMMC reset failed MTK_EMMC_IOCON 0x%x", __func__, mtk_msdc_read32(base + MTK_EMMC_IOCON));
    }
}

static int mtk_msdc_sdmmc_pwr(sdio_hc_t *hc, uint32_t vdd)
{
    hc->vdd = vdd;

    if ((vdd == 0U) && ((hc->caps & (uint64_t)HC_CAP_SLOT_TYPE_EMBEDDED) != 0U)) {
        /* Hardware reset */
        msdc_hw_reset(hc);
    }

    return (EOK);
}

static int mtk_msdc_sdmmc_bus_mode(sdio_hc_t *hc, uint32_t bus_mode)
{
    hc->bus_mode = bus_mode;
    return (EOK);
}

static int mtk_msdc_sdmmc_bus_width(sdio_hc_t *hc, uint32_t width)
{
    uint32_t val;
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

#ifdef MTK_MSDC_DEBUG
    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: width %d", __func__, width);
#endif

    val = mtk_msdc_read32(base + MTK_SDC_CFG);
    val &= ~MTK_SDC_CFG_BUSWIDTH;

    switch (width) {
        case BUS_WIDTH_1:
            val |= ((uint32_t)MTK_MSDC_BUS_1BITS << 16);
            break;
        case BUS_WIDTH_4:
            val |= ((uint32_t)MTK_MSDC_BUS_4BITS << 16);
            break;
        case BUS_WIDTH_8:
            val |= ((uint32_t)MTK_MSDC_BUS_8BITS << 16);
            break;
        default:
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: nonsuported bus width %d", __func__, width);
            break;
    }

    mtk_msdc_write32(base + MTK_SDC_CFG, val);
    hc->bus_width = width;
    return (EOK);
}

/* clock control primitives */
static void msdc_set_timeout(const sdio_hc_t *const hc, const uint32_t ns, const uint32_t clks)
{
    uint32_t timeout;
    mtk_msdc_sdmmc_t *sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;

    sdmmc->timeout_ns = ns;
    sdmmc->timeout_clks = clks;
    if (hc->clk == 0u) {
        timeout = 0;
    } else {
        /*set the timeout clock to max*/
        timeout = 0xffu;
    }
    sdmmc_set_field(base + MTK_SDC_CFG, MTK_SDC_CFG_DTOC, timeout);
}

static int mtk_msdc_sdmmc_clk(sdio_hc_t *hc, uint32_t hz)
{
    uint32_t mode, flags, div_l, sclk;
    int status;
    mtk_msdc_sdmmc_t *sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;
    const uintptr_t top_base = sdmmc->top_base;

    flags = mtk_msdc_read32(base + MTK_MSDC_INTEN);
    sdmmc_clr_bits(base + MTK_MSDC_INTEN, flags);
    sdmmc_clr_bits(base + MTK_MSDC_CFG, MTK_MSDC_CFG_HS400_CK_MODE);

    if (hz > hc->clk_max) {
        hz = hc->clk_max;
    }

    /*
     * TIMING_DDR50 should work for both EMMC and SD.
     */
    if ((hc->timing == TIMING_DDR50) ||
        (hc->timing == TIMING_HS400)) {

        if (hc->timing == TIMING_HS400) {
            mode = 0x3;
        } else {
            mode = 0x2; /* ddr mode and use divisor */
        }

        if (hz >= (sdmmc->src_clk_freq >> 2)) {
            div_l = 0;                       /* mean div_l = 1/4 in DDR */
            sclk = sdmmc->src_clk_freq >> 2; /* sclk = clk / 4 */
        } else {
            div_l = (sdmmc->src_clk_freq + ((hz << 2) - 1u)) / (hz << 2);
            sclk = (sdmmc->src_clk_freq >> 2) / div_l;
            div_l = (div_l >> 1);
        }

        if (((int)hc->timing == TIMING_HS400) && (hz >= (sdmmc->src_clk_freq >> 1))) {
            sdmmc_set_bits(base + MTK_MSDC_CFG, MTK_MSDC_CFG_HS400_CK_MODE);
            sclk = sdmmc->src_clk_freq >> 1;
            div_l = 0; /* div_l is ignored when HS400_CK_MODE is set */
        }
    } else if (hz >= sdmmc->src_clk_freq) {
        mode = 0x1; /* no divisor */
        div_l = 0;
        sclk = sdmmc->src_clk_freq;
    } else {
        mode = 0x0; /* use divisor */
        if (hz >= (sdmmc->src_clk_freq >> 1)) {
            div_l = 0;                       /* mean div_l = 1/2 */
            sclk = sdmmc->src_clk_freq >> 1; /* sclk = clk / 2 */
        } else {
            div_l = (sdmmc->src_clk_freq + ((hz << 2) - 1u)) / (hz << 2);
            sclk = (sdmmc->src_clk_freq >> 2) / div_l;
        }
    }

    sdmmc_set_field(base + MTK_MSDC_CFG, MTK_MSDC_CFG_CKMOD | MTK_MSDC_CFG_CKDIV, (mode << 12) | div_l);
    sdmmc_set_bits(base + MTK_MSDC_CFG, MTK_MSDC_CFG_CKPDN);

    status = mtk_msdc_waitmask(hc, MTK_MSDC_CFG, MTK_MSDC_CFG_CKSTB, MTK_MSDC_CFG_CKSTB, 20000); /* 20ms */
    if (status != EOK) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s:%d CFG_CKSTB Timeout", __func__, __LINE__);
        return (ETIMEDOUT);
    }

    sdmmc->sclk = sclk;
    sdmmc->mclk = hz;
    /* need because clk changed. */
    msdc_set_timeout(hc, sdmmc->timeout_ns, sdmmc->timeout_clks);
    sdmmc_set_bits(base + MTK_MSDC_INTEN, flags);

    /*
     * mmc_select_hs400() will drop to 50Mhz and High speed mode,
     * tune result of hs200/200Mhz is not suitable for 50Mhz
     */
    if (sdmmc->sclk <= 52000000u) {
        mtk_msdc_write32(base + MTK_MSDC_IOCON, sdmmc->def_tune_para.iocon);
        mtk_msdc_write32(top_base + MTK_EMMC_TOP_CONTROL, sdmmc->def_tune_para.emmc_top_control);
        mtk_msdc_write32(top_base + MTK_EMMC_TOP_CMD, sdmmc->def_tune_para.emmc_top_cmd);
    } else {
        mtk_msdc_write32(base + MTK_MSDC_IOCON, sdmmc->saved_tune_para.iocon);
        mtk_msdc_write32(top_base + MTK_EMMC_TOP_CONTROL, sdmmc->saved_tune_para.emmc_top_control);
        mtk_msdc_write32(top_base + MTK_EMMC_TOP_CMD, sdmmc->saved_tune_para.emmc_top_cmd);
        mtk_msdc_write32(base + MTK_PAD_CMD_TUNE, sdmmc->saved_tune_para.pad_cmd_tune);
    }

    if (hc->timing == TIMING_HS400) {
        sdmmc_set_field(base + MTK_PAD_CMD_TUNE, MTK_MSDC_PAD_TUNE_CMDRRDLY, sdmmc->hs400_cmd_int_delay);
        (void)mtk_msdc_sdmmc_hs400_tune(hc);
    }

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 1, "%s:%d sclk: %d, timing: %d, mode: %d, div_l %d%s", __func__, __LINE__, sdmmc->sclk, hc->timing, mode, div_l,
               (mtk_msdc_read32(base + MTK_MSDC_CFG) & MTK_MSDC_CFG_HS400_CK_MODE) ? " (HS400_CK_MODE)" : "");

    hc->clk = sclk;

    return (EOK);
}

int mtk_msdc_sdmmc_signal_voltage(sdio_hc_t *hc, uint32_t signal_voltage)
{
    mtk_msdc_sdmmc_t *sdmmc = (mtk_msdc_sdmmc_t *)hc->cs_hdl;
    uintptr_t base = sdmmc->base;

    if (SIGNAL_VOLTAGE_1_8 == signal_voltage && !(hc->flags & HC_FLAG_DEV_MMC)) {
        if (EOK == mtk_msdc_waitmask(hc, MTK_SDC_STS, MTK_SDC_STS_SDCBUSY, 0, 20000)) { /* 20ms */
            sdmmc_set_bits(base + MTK_MSDC_CFG, MTK_MSDC_CFG_SCLKSTOPSEL);
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, 1, "%s: MTK_SDC_STS 0x%x MTK_MSDC_CFG 0x%x", __func__, mtk_msdc_read32(base + MTK_SDC_STS), mtk_msdc_read32(base + MTK_MSDC_CFG));
        } else {
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, 1, "%s: MTK_SDC_STS 0x%x timeout", __func__, mtk_msdc_read32(base + MTK_SDC_STS));
        }

        delay(10); /* minmum 5ms to keep CLK low */
        sdmmc_set_field(base + MTK_MSDC_CFG, MTK_MSDC_CFG_BV18SDT, 1);

        if (EOK == mtk_msdc_waitmask(hc, MTK_MSDC_PS, 0x010f0000, 0x010f0000, 20000)) { /* 20ms */
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, 1, "%s: MTK_MSDC_PS 0x%x", __func__, mtk_msdc_read32(base + MTK_MSDC_PS));
        } else {
            (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, 1, "%s: MTK_MSDC_PS 0x%x timeout", __func__, mtk_msdc_read32(base + MTK_MSDC_PS));
        }
    }

    /* may needs to set pin for uhs */
    hc->signal_voltage = signal_voltage;
    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 3, "%s: signal_voltage %d", __func__, signal_voltage);
    return (EOK);
}

static int msdc_prepare_hs400_tuning(const sdio_hc_t *const hc)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const uintptr_t top_base = sdmmc->top_base;
    const uintptr_t base = sdmmc->base;

    // 0x10000 | (0x15 << 10) | (0 << 5) | 0x4 this magic number for enable hs400, reference vendor's suggestion
    mtk_msdc_write32(top_base + MTK_EMMC50_PAD_DS_TUNE, 0x10000u | ((uint32_t)0x13 << 10) | (0u << 5) | 0x11u);

    /* hs400 mode must set it to 0 */
    sdmmc_clr_bits(base + MTK_MSDC_PATCH_BIT2, MTK_MSDC_PATCH_BIT2_CFGCRCSTS);

    /* to improve read performance, set outstanding to 2 */
    sdmmc_set_field(base + MTK_EMMC50_CFG3, MTK_EMMC50_CFG3_OUTS_WR, 2);

    return (EOK);
}



static uint32_t test_delay_bit(const uint32_t delay_l, uint32_t bit)
{
    bit %= MTK_PAD_DELAY_MAX;
    return (delay_l & ((uint32_t)1u << bit));
}

static int get_delay_len(const uint32_t delay_l, const uint32_t start_bit)
{
    int cnt;

    for (cnt = 0; cnt < (MTK_PAD_DELAY_MAX - (int)start_bit); cnt++) {
        if (test_delay_bit(delay_l, start_bit + (uint32_t)cnt) == 0) {
            return cnt;
        }
    }
    return ((uint32_t)MTK_PAD_DELAY_MAX - start_bit);
}

static struct msdc_delay_phase get_best_delay(const sdio_hc_t *const hc, const uint32_t delay_l)
{
    int start = 0, len = 0;
    int start_final = 0, len_final = 0;
    uint8_t final_phase = 0xff;
    struct msdc_delay_phase delay_phase = {
        0,
    };

    if (delay_l == 0u) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 0, "%s: phase error: [map:%x]", __func__, delay_l);
        delay_phase.final_phase = final_phase;
        return delay_phase;
    }

    while (start < MTK_PAD_DELAY_MAX) {
        len = get_delay_len(delay_l, (uint32_t)start);
        if (len_final < len) {
            start_final = start;
            len_final = len;
        }
        start += len ? len : 1;
        if ((len >= 12) && (start_final < 4)) {
            break;
        }
    }

    /* The rule is that to find the smallest delay_l cell */
    if (start_final == 0) {
        final_phase = (uint8_t)((start_final + len_final / 3) % MTK_PAD_DELAY_MAX);
    } else {
        final_phase = (uint8_t)((start_final + len_final / 2) % MTK_PAD_DELAY_MAX);
    }
    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 3, "%s: phase: [map:%x] [maxlen:%d] [final:%d]", __func__, delay_l, len_final, final_phase);

    delay_phase.maxlen = (uint8_t)len_final;
    delay_phase.start = (uint8_t)start_final;
    delay_phase.final_phase = final_phase;
    return delay_phase;
}

static int mmc_send_tuning(sdio_hc_t *hc, const uint32_t op)
{
    struct sdio_cmd cmd;
    const uint32_t tlen = (hc->bus_width == BUS_WIDTH_8) ? 128u : 64u;
    int status;
    sdio_sge_t sge;
    uint8_t *buf = NULL;

    /* Create DMA safe buffer */
    buf = sdio_alloc(MTK_SG_BUF_SIZE);
    if (buf == NULL) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: sdio_alloc failed\n", __func__);
        status = ENOMEM;
        return status;
    }

    sge.sg_count = tlen;
    sge.sg_address = (uint64_t)buf;

    (void)memset((void *)&cmd, 0, sizeof(cmd));
    (void)sdio_setup_cmd(&cmd, SCF_CTYPE_ADTC | SCF_RSP_R1, op, 0);

    /* Seems if not read the data out of the buffer, it will be some problems */
    (void)sdio_setup_cmd_io(&cmd, SCF_DIR_IN, 1, tlen, &sge, 1, NULL);
    status = sdio_issue_cmd(&hc->device, &cmd, SDIO_TIME_DEFAULT);

    /* Command error */
    if (cmd.status > CS_CMD_CMP) {
        status = EIO;
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s error cmd.status %d", __func__, cmd.status);
    }

    (void)sdio_free(buf, MTK_SG_BUF_SIZE);

    return status;
}

static int mtk_msdc_sdmmc_hs400_tune(sdio_hc_t *hc)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    sdio_dev_t *dev;
    struct msdc_delay_phase dly1_delay;
    uint32_t val, result_dly1 = 0;
    uint8_t ext_csd[512];
    int i, ret;
    dev = &hc->device;

    sdmmc_set_bits(sdmmc->top_base + MTK_EMMC50_PAD_DS_TUNE, MTK_PAD_DS_DLY_SEL);

    for (i = 0; i < MTK_PAD_DELAY_MAX; i++) {
        sdmmc_set_field(sdmmc->top_base + MTK_EMMC50_PAD_DS_TUNE, MTK_PAD_DS_DLY1, (uint32_t)i);
        ret = mmc_send_ext_csd(dev, ext_csd);
        if (!ret) {
            result_dly1 |= ((uint32_t)1u << i);
        }
        else {
            (void)delay(1);
        }
    }

    dly1_delay = get_best_delay(hc, result_dly1);
    if (dly1_delay.maxlen == 0u) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "Failed to get DLY1 delay!");
        return -EIO;
    }

    sdmmc_set_field(sdmmc->top_base + MTK_EMMC50_PAD_DS_TUNE, MTK_PAD_DS_DLY1, dly1_delay.final_phase);

    val = mtk_msdc_read32(sdmmc->top_base + MTK_EMMC50_PAD_DS_TUNE);

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "Final PAD_DS_TUNE: 0x%x", val);
    return 0;
}

static int mtk_msdc_sdmmc_timing(sdio_hc_t *hc, uint32_t timing)
{
    hc->timing = timing;
    /* DDR50 need to re-configure the clock after timing change. */
    if (hc->timing == (uint32_t)TIMING_DDR50) {
        (void)mtk_msdc_sdmmc_clk(hc, hc->clk);
    }

    return (EOK);
}

static inline void msdc_set_cmd_delay(const mtk_msdc_sdmmc_t *const sdmmc, uint32_t value)
{
    sdmmc_set_field(sdmmc->top_base + MTK_EMMC_TOP_CMD, MTK_PAD_CMD_RXDLY, value);
}

static inline void msdc_set_data_delay(const mtk_msdc_sdmmc_t *const sdmmc, const uint32_t value)
{
    sdmmc_set_field(sdmmc->top_base + MTK_EMMC_TOP_CONTROL, MTK_PAD_DAT_RD_RXDLY, value);
}
/*
 * MSDC IP which supports data tune + async fifo can do CMD/DAT tune
 * together, which can save the tuning time.
 */
static int msdc_tune_together(sdio_hc_t *hc, uint32_t const opcode)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    uint32_t i, rise_delay = 0, fall_delay = 0;
    struct msdc_delay_phase final_rise_delay, final_fall_delay = {0,};
    uint8_t final_delay, final_maxlen;
    int ret;

    //sdmmc_set_field(sdmmc->base + MTK_MSDC_PATCH_BIT, MTK_MSDC_INT_DAT_LATCH_CK_SEL,
    //              sdmmc->latch_ck);
    sdmmc_clr_bits(sdmmc->base + MTK_MSDC_IOCON, MTK_MSDC_IOCON_RSPL);
    sdmmc_clr_bits(sdmmc->base + MTK_MSDC_IOCON,
                 MTK_MSDC_IOCON_DSPL | MTK_MSDC_IOCON_W_DSPL);
    for (i = 0; i < MTK_PAD_DELAY_MAX; i++) {
        msdc_set_cmd_delay(sdmmc, i);
        msdc_set_data_delay(sdmmc, i);
        ret = mmc_send_tuning(hc, opcode);
        if (!ret) {
            rise_delay |= ((uint32_t)1u << i);
        }
    }
    final_rise_delay = get_best_delay(hc, rise_delay);
    /* if rising edge has enough margin, then do not scan falling edge */
    if (!((final_rise_delay.maxlen >= 12u) || ((final_rise_delay.start == 0u) && (final_rise_delay.maxlen >= 4u)))) {
        sdmmc_set_bits(sdmmc->base + MTK_MSDC_IOCON, MTK_MSDC_IOCON_RSPL);
        sdmmc_set_bits(sdmmc->base + MTK_MSDC_IOCON,
                    MTK_MSDC_IOCON_DSPL | MTK_MSDC_IOCON_W_DSPL);
        for (i = 0; i < MTK_PAD_DELAY_MAX; i++) {
            msdc_set_cmd_delay(sdmmc, i);
            msdc_set_data_delay(sdmmc, i);
            ret = mmc_send_tuning(hc, opcode);
            if (!ret) {
                fall_delay |= ((uint32_t)1u << i);
            }
        }
        final_fall_delay = get_best_delay(hc, fall_delay);
    }
    final_maxlen = max(final_rise_delay.maxlen, final_fall_delay.maxlen);
    if (final_maxlen == final_rise_delay.maxlen) {
        sdmmc_clr_bits(sdmmc->base + MTK_MSDC_IOCON, MTK_MSDC_IOCON_RSPL);
        sdmmc_clr_bits(sdmmc->base + MTK_MSDC_IOCON,
                     MTK_MSDC_IOCON_DSPL | MTK_MSDC_IOCON_W_DSPL);
        final_delay = final_rise_delay.final_phase;
    } else {
        sdmmc_set_bits(sdmmc->base + MTK_MSDC_IOCON, MTK_MSDC_IOCON_RSPL);
        sdmmc_set_bits(sdmmc->base + MTK_MSDC_IOCON,
                     MTK_MSDC_IOCON_DSPL | MTK_MSDC_IOCON_W_DSPL);
        final_delay = final_fall_delay.final_phase;
    }
    msdc_set_cmd_delay(sdmmc, final_delay);
    msdc_set_data_delay(sdmmc, final_delay);
    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 1, "Final pad delay: %x\n", final_delay);
    return (final_delay == 0xff) ? -EIO : 0;
}

static int mtk_msdc_sdmmc_tune(sdio_hc_t *hc, uint32_t op)
{
    int ret;
    mtk_msdc_sdmmc_t *sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;
    const uintptr_t top_base = sdmmc->top_base;
    const sdio_dev_t *const dev = &hc->device;

    /* return if not HS200, HS400, or SDR104 */
    if ((hc->timing != TIMING_SDR104) && (hc->timing != TIMING_HS200) && (hc->timing != TIMING_HS400)) {
        return (EOK);
    }

    /* In case of HS400, it  needs to do HS200 tuning */
    if ((hc->timing == TIMING_HS200) && (dev->caps & (uint64_t)DEV_CAP_HS400)) {
        (void)msdc_prepare_hs400_tuning(hc);
    }


    ret = msdc_tune_together(hc, op);
    if (ret == EIO) {
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: Tune data fail!", __func__);
    }
    if ((hc->timing == TIMING_HS200) && (dev->caps & (uint64_t)DEV_CAP_HS400)) {
        sdmmc_clr_bits(sdmmc->base + MTK_MSDC_IOCON,
                MTK_MSDC_IOCON_DSPL | MTK_MSDC_IOCON_W_DSPL);
        msdc_set_data_delay(sdmmc, 0);
    }

    sdmmc->saved_tune_para.iocon = mtk_msdc_read32(base + MTK_MSDC_IOCON);
    sdmmc->saved_tune_para.emmc_top_control = mtk_msdc_read32(top_base + MTK_EMMC_TOP_CONTROL);
    sdmmc->saved_tune_para.emmc_top_cmd = mtk_msdc_read32(top_base + MTK_EMMC_TOP_CMD);
    sdmmc->saved_tune_para.pad_cmd_tune = mtk_msdc_read32(base + MTK_PAD_CMD_TUNE);
    return ret;
}

int mtk_msdc_sdmmc_dinit(sdio_hc_t *hc)
{
    mtk_msdc_sdmmc_t *sdmmc;
    uintptr_t base;
    uintptr_t top_base;

    if ((!hc) || (!hc->cs_hdl)) {
        return (EOK);
    }

    sdmmc = hc->cs_hdl;
    base = sdmmc->base;
    top_base = sdmmc->top_base;

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 1, "%s:%d base 0x%p", __func__, __LINE__, (void *)base);

    if (base != (uintptr_t)MAP_FAILED) {
        mtk_msdc_sdmmc_reset(hc);
        (void)mtk_msdc_sdmmc_pwr(hc, 0);
        mtk_msdc_write32(base + MTK_MSDC_INTEN, 0);

        if (sdmmc->dma != NULL) {
            if (sdmmc->dma->bd != NULL) {
                (void)sdio_free(sdmmc->dma->bd, sizeof(struct mt_bdma_desc) * MTK_MAX_BD_NUM);
            }
            if (sdmmc->dma->gpd != NULL) {
                (void)sdio_free(sdmmc->dma->gpd, sizeof(struct mt_gpdma_desc) * 2);
            }
            free(sdmmc->dma);
            sdmmc->dma = NULL;
        }

        if (hc->hc_iid != -1) {
            (void)InterruptDetach(hc->hc_iid);
        }

        (void)munmap((void *)base, sdmmc->basesize);
        (void)munmap((void *)top_base, sdmmc->top_basesize);
    }
    free(sdmmc);
    hc->cs_hdl = NULL;

    return (EOK);
}

/* init gpd and bd, should be called before hw init */
static void msdc_init_gpd_bd(const sdio_hc_t *const hc)
{
    const mtk_msdc_sdmmc_t *const sdmmc = hc->cs_hdl;
    const struct msdc_dma *const dma = sdmmc->dma;
    struct mt_gpdma_desc *gpd = dma->gpd;
    struct mt_bdma_desc *bd = dma->bd;
    int cnt;

    (void)memset(gpd, 0, sizeof(struct mt_gpdma_desc) * 2);

    gpd->flags = GPDMA_DESC_BDP;                        /* hwo, cs, bd pointer */
    gpd->flags |= (uint32_t)(dma->bd_addr >> 32) << 28; /* high 4-bits of physical address */
    gpd->ptr = (uint32_t)dma->bd_addr;                  /* low 32-bits of physical address */
    /* gpd->next is must set for desc DMA
     * That's why must alloc 2 gpd structure.
     */
    gpd->flags |= (uint32_t)((dma->gpd_addr + sizeof(struct mt_gpdma_desc)) >> 32) << 24;
    gpd->next = (uint32_t)(dma->gpd_addr + sizeof(struct mt_gpdma_desc));
    (void)memset(bd, 0, sizeof(struct mt_bdma_desc) * MTK_MAX_BD_NUM);
    for (cnt = 0; cnt < (MTK_MAX_BD_NUM - 1); cnt++) {
        bd[cnt].flags |= (uint32_t)((dma->bd_addr + sizeof(*bd) * (cnt + 1)) >> 32) << 24;
        bd[cnt].next = (uint32_t)(dma->bd_addr + sizeof(*bd) * (cnt + 1));
    }
}

static void msdc_init_hw(const sdio_hc_t *const hc)
{
    mtk_msdc_sdmmc_t *sdmmc = hc->cs_hdl;
    const uintptr_t base = sdmmc->base;
    const uintptr_t top_base = sdmmc->top_base;

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 1, "%s: init hardware", __func__);

    /* Configure to MMC/SD mode & clock free running*/
    sdmmc_set_bits(base + MTK_MSDC_CFG, MTK_MSDC_CFG_MODE | MTK_MSDC_CFG_CKPDN); /* bit0 1:sd/mmc mode */
    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 1, "%s: MTK_MSDC_CFG %x", __func__, mtk_msdc_read32(base + MTK_MSDC_CFG));

    /* Reset */
    mtk_msdc_sdmmc_reset(hc);

#ifdef MTK_MSDC_DEBUG
    mtk_msdc_reg_dump(hc, __func__, __LINE__, 1);
#endif

    /* Disable card detection */
    sdmmc_clr_bits(base + MTK_MSDC_PS, MTK_MSDC_PS_CDEN); /* no this bit, it seems to be related with CD */

    /* Disable and clear all interrupts */
    mtk_msdc_write32(base + MTK_MSDC_INTEN, 0);
    mtk_msdc_write32(base + MTK_MSDC_INT, mtk_msdc_read32(base + MTK_MSDC_INT));

    mtk_msdc_write32(top_base + MTK_EMMC_TOP_CONTROL, 0);
    mtk_msdc_write32(top_base + MTK_EMMC_TOP_CMD, 0);
    mtk_msdc_write32(base + MTK_MSDC_IOCON, 0);

    sdmmc_set_field(base + MTK_MSDC_IOCON, MTK_MSDC_IOCON_DDLSEL, 0); /* lxd: may try 1 for falling edge */
    mtk_msdc_write32(base + MTK_MSDC_PATCH_BIT, 0x403c0046u);
    sdmmc_set_field(base + MTK_MSDC_PATCH_BIT, MTK_MSDC_CKGEN_MSDC_DLY_SEL, 1);
    mtk_msdc_write32(base + MTK_MSDC_PATCH_BIT1, 0xffff4089u);
    sdmmc_set_bits(base + MTK_EMMC50_CFG0, MTK_EMMC50_CFG_CFCSTS_SEL);

    sdmmc_set_field(base + MTK_MSDC_PATCH_BIT1, MTK_MSDC_PATCH_BIT1_STOP_DLY_SEL, 3); /*stop_clk_fix*/
    /*stop_clk_fix*/
    sdmmc_clr_bits(base + MTK_SDC_FIFO_CFG, MTK_SDC_FIFO_CFG_WRVALIDSEL);
    sdmmc_clr_bits(base + MTK_SDC_FIFO_CFG, MTK_SDC_FIFO_CFG_RDVALIDSEL);

    sdmmc_clr_bits(base + MTK_MSDC_PATCH_BIT1, MTK_MSDC_PATCH_BIT1_BUSY_CHECK_SEL); /*busy_check*/

    /*async_fifo*/
    sdmmc_set_field(base + MTK_MSDC_PATCH_BIT2, MTK_MSDC_PATCH_BIT2_RESPWAIT, 3);
    sdmmc_set_bits(top_base + MTK_EMMC_TOP_CONTROL, MTK_SDC_RX_ENH_EN);
    sdmmc_clr_bits(base + MTK_MSDC_PATCH_BIT2, MTK_MSDC_PATCH_BIT2_CFGRESP);
    sdmmc_clr_bits(base + MTK_MSDC_PATCH_BIT2, MTK_MSDC_PB2_RESPSTSENSEL);
    sdmmc_clr_bits(base + MTK_MSDC_PATCH_BIT2, MTK_MSDC_PB2_CRCSTSENSEL);
    sdmmc_set_bits(base + MTK_MSDC_PATCH_BIT2, MTK_MSDC_PATCH_BIT2_CFGCRCSTS);

    /*support_64g*/
    sdmmc_set_bits(base + MTK_MSDC_PATCH_BIT2, MTK_MSDC_PB2_SUPPORT_64G);

    /*data_tune*/
    sdmmc_set_bits(top_base + MTK_EMMC_TOP_CONTROL,
                   MTK_PAD_DAT_RD_RXDLY_SEL);
    sdmmc_clr_bits(top_base + MTK_EMMC_TOP_CONTROL,
                   MTK_DATA_K_VALUE_SEL);
    sdmmc_set_bits(top_base + MTK_EMMC_TOP_CMD,
                   MTK_PAD_CMD_RD_RXDLY_SEL);

    /* Configure to enable SDIO mode.
     * it's must otherwise sdio cmd5 failed
     */
    sdmmc_set_bits(base + MTK_SDC_CFG, MTK_SDC_CFG_SDIO);

    /* disable detect SDIO device interrupt function */
    sdmmc_clr_bits(base + MTK_SDC_CFG, MTK_SDC_CFG_SDIOIDE);

    /* Configure to default data timeout */
    sdmmc_set_field(base + MTK_SDC_CFG, MTK_SDC_CFG_DTOC, 3);

    sdmmc->saved_tune_para.iocon = mtk_msdc_read32(base + MTK_MSDC_IOCON);
    sdmmc->saved_tune_para.emmc_top_control = mtk_msdc_read32(top_base + MTK_EMMC_TOP_CONTROL);
    sdmmc->saved_tune_para.emmc_top_cmd = mtk_msdc_read32(top_base + MTK_EMMC_TOP_CMD);
    sdmmc->def_tune_para.emmc_top_control = sdmmc->saved_tune_para.emmc_top_control;
    sdmmc->def_tune_para.emmc_top_cmd = sdmmc->saved_tune_para.emmc_top_cmd;
    sdmmc->def_tune_para.iocon = sdmmc->saved_tune_para.iocon;

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 1, "%s: init hardware done!", __func__);
#ifdef MTK_MSDC_DEBUG
    mtk_msdc_reg_dump(hc, __func__, __LINE__, 1);
#endif
}

int mtk_msdc_sdmmc_init(sdio_hc_t *hc)
{
    sdio_hc_cfg_t *cfg;
    struct sigevent event;
    int save_errno;
    mtk_msdc_sdmmc_t *sdmmc = hc->cs_hdl;

    const sdio_hc_entry_t mtk_msdc_sdmmc_entry = {
        .nentries = SDIO_HC_ENTRY_NFUNCS,
        .dinit = mtk_msdc_sdmmc_dinit,
        .pm = NULL,
        .cmd = mtk_msdc_sdmmc_cmd,
        .abort = mtk_msdc_sdmmc_abort,
        .event = mtk_msdc_sdmmc_event,
        .cd = mtk_msdc_sdmmc_cd,
        .pwr = mtk_msdc_sdmmc_pwr,
        .clk = mtk_msdc_sdmmc_clk,
        .bus_mode = mtk_msdc_sdmmc_bus_mode,
        .bus_width = mtk_msdc_sdmmc_bus_width,
        .timing = mtk_msdc_sdmmc_timing,
        .signal_voltage = mtk_msdc_sdmmc_signal_voltage,
        .drv_type = NULL,
        .driver_strength = NULL,
        .tune = mtk_msdc_sdmmc_tune,
        .preset = NULL,
    };

    hc->hc_iid = -1;
    cfg = &hc->cfg;

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 1, "%s:%d hc idx%d base_addr 0x%x", __func__, __LINE__, cfg->idx, (unsigned int)cfg->base_addr[0]);
    (void)memcpy(&hc->entry, &mtk_msdc_sdmmc_entry, sizeof(sdio_hc_entry_t));

    if ((!cfg->base_addr[0]) || (!cfg->base_addr[1])) {
        return (ENODEV);
    }

    if (!cfg->base_addr_size[0]) {
        cfg->base_addr_size[0] = MTK_MSDC_SIZE;
    }

    if (!cfg->base_addr_size[1]) {
        cfg->base_addr_size[1] = MTK_MSDC_SIZE;
    }

    sdmmc = calloc(1, sizeof(mtk_msdc_sdmmc_t));
    hc->cs_hdl = sdmmc;
    if (sdmmc == NULL) {
        return (ENOMEM);
    }

    sdmmc->basesize = (uint32_t)cfg->base_addr_size[0];
    sdmmc->base = (uintptr_t)mmap(NULL, sdmmc->basesize,
                                                PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_PHYS | MAP_SHARED, NOFD, (off_t)cfg->base_addr[0]);
    if (sdmmc->base == (uintptr_t)MAP_FAILED) {
        save_errno = errno;
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: base mmap (0x%" PRIx64 "x) %s", __func__, cfg->base_addr[0], strerror(errno));
        (void)mtk_msdc_sdmmc_dinit(hc);
        return (save_errno);
    }

    sdmmc->top_basesize = (uint32_t)cfg->base_addr_size[1];
    sdmmc->top_base = (uintptr_t)mmap(NULL, sdmmc->top_basesize,
                                                    PROT_READ | PROT_WRITE | PROT_NOCACHE, MAP_PHYS | MAP_SHARED, NOFD, (off_t)cfg->base_addr[1]);
    if (sdmmc->top_base == (uintptr_t)MAP_FAILED) {
        save_errno = errno;
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s: top_base mmap (0x%" PRIx64 "x) %s", __func__, cfg->base_addr[1], strerror(errno));
        (void)mtk_msdc_sdmmc_dinit(hc);
        return (save_errno);
    }

#ifdef MTK_MSDC_DEBUG
    (void)mtk_msdc_reg_dump(hc, __func__, __LINE__, 1);
#endif

    hc->caps |= (uint64_t)HC_CAP_BSY | (uint64_t)HC_CAP_BW4 | (uint64_t)HC_CAP_BW8 /*| HC_CAP_ERASE*/;
    hc->caps |= (uint64_t)HC_CAP_ACMD23;
    hc->caps |= (uint64_t)HC_CAP_DMA | (uint64_t)HC_CAP_DMA64;
    hc->caps |= (uint64_t)HC_CAP_HS;
    if ((hc->caps & (uint64_t)HC_CAP_SLOT_TYPE_EMBEDDED) == 0U) {
        hc->caps |= (uint64_t)HC_CAP_SDR12 | (uint64_t)HC_CAP_SDR25;
        hc->caps |= (uint64_t)HC_CAP_DDR50;
        hc->caps |= (uint64_t)HC_CAP_SDR50;
        hc->caps |= (uint64_t)HC_CAP_SDR104; /* requires tuning */
    } else {
        hc->caps |= (uint64_t)HC_CAP_DDR50;
        hc->caps |= (uint64_t)HC_CAP_HS200; /* requires tuning */
        hc->caps |= (uint64_t)HC_CAP_HS400; /* requires tuning */
    }

    hc->ocr |= OCR_VDD_32_33 | OCR_VDD_33_34;
    hc->caps |= HC_CAP_SV_3_3V | HC_CAP_SV_1_8V;

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 1, "%s:%d hc->caps 0x%lx cfg->caps 0x%lx masked hc->caps 0x%lx ", __func__, __LINE__, hc->caps, cfg->caps, hc->caps & cfg->caps);

    hc->caps &= cfg->caps; /* reconcile command line options */

    if ((hc->caps & (uint64_t)HC_CAP_SLOT_TYPE_EMBEDDED) != 0U) {
        sdmmc->src_clk_freq = 400 * 1000 * 1000;
    } else {
        sdmmc->src_clk_freq = 200 * 1000 * 1000;
    }

    if (cfg->clk != 0u) {
        hc->clk_max = (uint32_t)cfg->clk;
    } else {
        hc->clk_max = sdmmc->src_clk_freq;
    }

    if (hc->clk_max > 200u * 1000u * 1000u) {
        hc->clk_max = 200u * 1000u * 1000u;
    }

    hc->clk_min = DIV_ROUND_UP(sdmmc->src_clk_freq, ((4 * 4096)-1));

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 1, "%s:%d cfg->clk %d src_clk_freq %d clk_max %d clk_min %d", __func__, __LINE__,
               cfg->clk, sdmmc->src_clk_freq, hc->clk_max, hc->clk_min);

    /* MMC core transfer sizes tunable parameters */
    /*mmc->max_segs = MTK_MAX_BD_NUM;
    mmc->max_seg_size = BDMA_DESC_BUFLEN;
    mmc->max_blk_size = 2048;
    mmc->max_req_size = 512 * 1024;
    mmc->max_blk_count = mmc->max_req_size / 512;*/

    sdmmc->timeout_clks = 3 * 1024 * 1024;
    sdmmc->timeout_ns = 0;

    if (!(hc->caps & (uint64_t)HC_CAP_DMA)) {
        return (ENODEV);
    }

    hc->cfg.sg_max = MTK_MAX_BD_NUM;
    sdmmc->dma = calloc(1, sizeof(struct msdc_dma));
    if (sdmmc->dma == NULL) {
        (void)mtk_msdc_sdmmc_dinit(hc);
        return (ENOMEM);
    }
    sdmmc->dma->gpd = sdio_alloc(sizeof(struct mt_gpdma_desc) * 2);
    if (sdmmc->dma->gpd == NULL) {
        save_errno = errno;
        (void)mtk_msdc_sdmmc_dinit(hc);
        return (save_errno);
    }
    sdmmc->dma->gpd_addr = sdio_vtop(sdmmc->dma->gpd);
    sdmmc->dma->bd = sdio_alloc(sizeof(struct mt_bdma_desc) * MTK_MAX_BD_NUM);
    if (sdmmc->dma->bd == NULL) {
        save_errno = errno;
        (void)mtk_msdc_sdmmc_dinit(hc);
        return (save_errno);
    }
    sdmmc->dma->bd_addr = sdio_vtop(sdmmc->dma->bd);

    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 3, "%s:%d ==>msdc_init_gpd_bd", __func__, __LINE__);
    msdc_init_gpd_bd(hc);

    msdc_init_hw(hc);

    /* we don't want this interrupt at the driver startup */
    SIGEV_PULSE_INIT(&event, hc->hc_coid, (short)hc->priority, HC_EV_INTR, NULL);
    hc->hc_iid = InterruptAttachEvent(cfg->irq[0], &event, 0);
    if (hc->hc_iid == -1) {
        save_errno = errno;
        (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, (int)hc->cfg.verbosity, 0, "%s:%d error 0x%x", __func__, __LINE__, errno);
        (void)mtk_msdc_sdmmc_dinit(hc);
        return (save_errno);
    }
    (void)sdio_slogf(_SLOGC_SDIODI, _SLOG_INFO, (int)hc->cfg.verbosity, 3, "%s:%d EOK", __func__, __LINE__);
    return (EOK);
}
#endif
