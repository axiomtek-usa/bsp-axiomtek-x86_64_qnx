/*
 * $QNXLicenseC:
 * Copyright 2025, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef MTK_MSDC_H_INCLUDED
#define MTK_MSDC_H_INCLUDED

#include <internal.h>

#define MTK_MSDC_TUNING_RETRIES     40
#define MTK_MSDC_CLOCK_TIMEOUT      10000
#define MTK_MSDC_COMMAND_TIMEOUT    1000000 /* The card could take very long time to process the transfered data */
#define MTK_MSDC_TRANSFER_TIMEOUT   1000000

#define MTK_MSDC_SIZE           0x700
#define MTK_MAX_BD_NUM          1024
#define MTK_SG_BUF_SIZE         128

/*--------------------------------------------------------------------------*/
/* Common Definition                                                        */
/*--------------------------------------------------------------------------*/
#define MTK_MSDC_BUS_1BITS      0x0
#define MTK_MSDC_BUS_4BITS      0x1
#define MTK_MSDC_BUS_8BITS      0x2

/*--------------------------------------------------------------------------*/
/* Register Offset                                                          */
/*--------------------------------------------------------------------------*/
#define MTK_MSDC_CFG            0x0u
#define MTK_MSDC_IOCON          0x04u
#define MTK_MSDC_PS             0x08u
#define MTK_MSDC_INT            0x0cu
#define MTK_MSDC_INTEN          0x10u
#define MTK_MSDC_FIFOCS         0x14u
#define MTK_SDC_CFG             0x30u
#define MTK_SDC_CMD             0x34u
#define MTK_SDC_ARG             0x38u
#define MTK_SDC_STS             0x3cu
#define MTK_SDC_RESP0           0x40u
#define MTK_SDC_RESP1           0x44u
#define MTK_SDC_RESP2           0x48u
#define MTK_SDC_RESP3           0x4cu
#define MTK_SDC_BLK_NUM         0x50u
#define MTK_SDC_VOL_CHG         0x54u
#define MTK_SDC_CSTS            0x58u
#define MTK_SDC_CSTS_EN         0x5cu
#define MTK_SDC_DATCRC_STS      0x60u
#define MTK_SDC_ADV_CFG0        0x64u
#define MTK_EMMC_CFG0           0x70u
#define MTK_EMMC_CFG1           0x74u
#define MTK_EMMC_STS            0x78u
#define MTK_EMMC_IOCON          0x7cu
#define MTK_SDC_ACMD_RESP       0x80u
#define MTK_SDC_ACMD19_TRG      0x84u
#define MTK_SDC_ACMD19_STS      0x88u
#define MTK_MSDC_DMA_SA_H4      0x8cu
#define MTK_MSDC_DMA_SA         0x90u
#define MTK_MSDC_DMA_CA         0x94u
#define MTK_MSDC_DMA_CTRL       0x98u
#define MTK_MSDC_DMA_CFG        0x9cu
#define MTK_MSDC_SW_DBG_SEL     0xa0u
#define MTK_MSDC_SW_DBG_OUT     0xa4u
#define MTK_MSDC_DMA_LENGTH     0xa8u
#define MTK_MSDC_PATCH_BIT      0xb0u
#define MTK_MSDC_PATCH_BIT0     0xb0u
#define MTK_MSDC_PATCH_BIT1     0xb4u
#define MTK_MSDC_PATCH_BIT2     0xb8u
#define MTK_MSDC_PAD_TUNE0      0xf0u
#define MTK_MSDC_PAD_TUNE1      0xf4u
#define MTK_MSDC_DAT_RD_DLY0    0xf8u
#define MTK_MSDC_DAT_RD_DLY1    0xfcu
#define MTK_MSDC_DAT_RD_DLY2    0x100u
#define MTK_MSDC_DAT_RD_DLY3    0x104u
#define MTK_PAD_CTL0            0x180u
#define MTK_PAD_DS_CTL0         0x184u
#define MTK_PAD_DS_TUNE         0x188u
#define MTK_PAD_CMD_TUNE        0x18cu
#define MTK_PAD_DAT01_TUNE      0x190u
#define MTK_PAD_DAT23_TUNE      0x194u
#define MTK_PAD_DAT45_TUNE      0x198u
#define MTK_PAD_DAT67_TUNE      0x19cu
#define MTK_EMMC51_CFG0         0x204u
#define MTK_EMMC50_CFG0         0x208u
#define MTK_EMMC50_CFG1         0x20cu
#define MTK_EMMC50_CFG2         0x21cu
#define MTK_EMMC50_CFG3         0x220u
#define MTK_EMMC50_CFG4         0x224u
#define MTK_SDC_FIFO_CFG        0x228u

/*--------------------------------------------------------------------------*/
/* Top Pad Register Offset                                                  */
/*--------------------------------------------------------------------------*/
#define MTK_EMMC_TOP_CONTROL        0x00u
#define MTK_EMMC_TOP_CMD            0x04u
#define MTK_EMMC50_PAD_CTL0         0x08u
#define MTK_EMMC50_PAD_DS_TUNE      0x0cu
#define MTK_EMMC50_PAD_DAT0_TUNE    0x10u
#define MTK_EMMC50_PAD_DAT1_TUNE    0x14u
#define MTK_EMMC50_PAD_DAT2_TUNE    0x18u
#define MTK_EMMC50_PAD_DAT3_TUNE    0x1cu
#define MTK_EMMC50_PAD_DAT4_TUNE    0x20u
#define MTK_EMMC50_PAD_DAT5_TUNE    0x24u
#define MTK_EMMC50_PAD_DAT6_TUNE    0x28u
#define MTK_EMMC50_PAD_DAT7_TUNE    0x2cu
#define MTK_LOOP_TEST_CONTROL       0x30u
#define MTK_MSDC_TOP_NEW_RX_CFG     0x38u
/*--------------------------------------------------------------------------*/
/* Register Mask                                                            */
/*--------------------------------------------------------------------------*/

/* MTK_MSDC_CFG mask */
#define MTK_MSDC_CFG_MODE           (0x1u << 0)        /* RW */
#define MTK_MSDC_CFG_CKPDN          (0x1u << 1)        /* RW */
#define MTK_MSDC_CFG_RST            (0x1u << 2)        /* RW */
#define MTK_MSDC_CFG_PIO            (0x1u << 3)        /* RW */
#define MTK_MSDC_CFG_BV18SDT        (0x1u << 5)        /* RW */
#define MTK_MSDC_CFG_BV18PSS        (0x1u << 6)        /* R  */
#define MTK_MSDC_CFG_CKSTB          (0x1u << 7)        /* R  */
#define MTK_MSDC_CFG_CKDIV          ((uint32_t)0xfffu << 8)      /* RW */
#define MTK_MSDC_CFG_CKMOD          ((uint32_t)0x3u << 20)       /* RW */
#define MTK_MSDC_CFG_HS400_CK_MODE  ((uint32_t)0x1u << 22)       /* RW */
#define MTK_MSDC_CFG_START_BITY     ((uint32_t)0x3u << 23)       /* RW */
#define MTK_MSDC_CFG_SCLKSTOPSEL    ((uint32_t)0x1u << 25)       /* RW */

/* MTK_MSDC_IOCON mask */
#define MTK_MSDC_IOCON_sdmmc104CKS  (0x1u << 0)        /* RW */
#define MTK_MSDC_IOCON_RSPL         (0x1u << 1)        /* RW */
#define MTK_MSDC_IOCON_DSPL         (0x1u << 2)        /* RW */
#define MTK_MSDC_IOCON_DDLSEL       (0x1u << 3)        /* RW */
#define MTK_MSDC_IOCON_DDR50CKD     (0x1u << 4)        /* RW */
#define MTK_MSDC_IOCON_DSPLSEL      (0x1u << 5)        /* RW */
#define MTK_MSDC_IOCON_W_DSPL       ((uint32_t)0x1u << 8)        /* RW */
#define MTK_MSDC_IOCON_D0SPL        ((uint32_t)0x1u << 16)       /* RW */
#define MTK_MSDC_IOCON_D1SPL        ((uint32_t)0x1u << 17)       /* RW */
#define MTK_MSDC_IOCON_D2SPL        ((uint32_t)0x1u << 18)       /* RW */
#define MTK_MSDC_IOCON_D3SPL        ((uint32_t)0x1u << 19)       /* RW */
#define MTK_MSDC_IOCON_D4SPL        ((uint32_t)0x1u << 20)       /* RW */
#define MTK_MSDC_IOCON_D5SPL        ((uint32_t)0x1u << 21)       /* RW */
#define MTK_MSDC_IOCON_D6SPL        ((uint32_t)0x1u << 22)       /* RW */
#define MTK_MSDC_IOCON_D7SPL        ((uint32_t)0x1u << 23)       /* RW */
#define MTK_MSDC_IOCON_RISCSZ       ((uint32_t)0x3u << 24)       /* RW */

/* MTK_MSDC_PS mask */
#define MTK_MSDC_PS_CDEN            (0x1u << 0)       /* RW */
#define MTK_MSDC_PS_CDSTS           (0x1u << 1)       /* R  */
#define MTK_MSDC_PS_CDDEBOUNCE      ((uint32_t)0xfu << 12)      /* RW */
#define MTK_MSDC_PS_DAT             ((uint32_t)0xffu << 16)     /* R  */
#define MTK_MSDC_PS_CMD             ((uint32_t)0x1u << 24)      /* R  */
#define MTK_MSDC_PS_WP              ((uint32_t)0x1u << 31)      /* R  */

/* MTK_MSDC_INT mask */
#define MTK_MSDC_INT_MMCIRQ         (0x1u << 0)       /* W1C */
#define MTK_MSDC_INT_CDSC           (0x1u << 1)       /* W1C */
#define MTK_MSDC_INT_ACMDRDY        (0x1u << 3)       /* W1C */
#define MTK_MSDC_INT_ACMDTMO        (0x1u << 4)       /* W1C */
#define MTK_MSDC_INT_ACMDCRCERR     (0x1u << 5)       /* W1C */
#define MTK_MSDC_INT_DMAQ_EMPTY     (0x1u << 6)       /* W1C */
#define MTK_MSDC_INT_SDIOIRQ        (0x1u << 7)       /* W1C */
#define MTK_MSDC_INT_CMDRDY         ((uint32_t)0x1u << 8)       /* W1C */
#define MTK_MSDC_INT_CMDTMO         ((uint32_t)0x1u << 9)       /* W1C */
#define MTK_MSDC_INT_RSPCRCERR      ((uint32_t)0x1u << 10)      /* W1C */
#define MTK_MSDC_INT_CSTA           ((uint32_t)0x1u << 11)      /* R */
#define MTK_MSDC_INT_XFER_COMPL     ((uint32_t)0x1u << 12)      /* W1C */
#define MTK_MSDC_INT_DXFER_DONE     ((uint32_t)0x1u << 13)      /* W1C */
#define MTK_MSDC_INT_DATTMO         ((uint32_t)0x1u << 14)      /* W1C */
#define MTK_MSDC_INT_DATCRCERR      ((uint32_t)0x1u << 15)      /* W1C */
#define MTK_MSDC_INT_ACMD19_DONE    ((uint32_t)0x1u << 16)      /* W1C */
#define MTK_MSDC_INT_DMA_BDCSERR    ((uint32_t)0x1u << 17)      /* W1C */
#define MTK_MSDC_INT_DMA_GPDCSERR   ((uint32_t)0x1u << 18)      /* W1C */
#define MTK_MSDC_INT_DMA_PROTECT    ((uint32_t)0x1u << 19)      /* W1C */

/* MTK_MSDC_INTEN mask */
#define MTK_MSDC_INTEN_MMCIRQ       (0x1u << 0)       /* RW */
#define MTK_MSDC_INTEN_CDSC         (0x1u << 1)       /* RW */
#define MTK_MSDC_INTEN_ACMDRDY      (0x1u << 3)       /* RW */
#define MTK_MSDC_INTEN_ACMDTMO      (0x1u << 4)       /* RW */
#define MTK_MSDC_INTEN_ACMDCRCERR   (0x1u << 5)       /* RW */
#define MTK_MSDC_INTEN_DMAQ_EMPTY   (0x1u << 6)       /* RW */
#define MTK_MSDC_INTEN_SDIOIRQ      (0x1u << 7)       /* RW */
#define MTK_MSDC_INTEN_CMDRDY       ((uint32_t)0x1u << 8)       /* RW */
#define MTK_MSDC_INTEN_CMDTMO       ((uint32_t)0x1u << 9)       /* RW */
#define MTK_MSDC_INTEN_RSPCRCERR    ((uint32_t)0x1u << 10)      /* RW */
#define MTK_MSDC_INTEN_CSTA         ((uint32_t)0x1u << 11)      /* RW */
#define MTK_MSDC_INTEN_XFER_COMPL   ((uint32_t)0x1u << 12)      /* RW */
#define MTK_MSDC_INTEN_DXFER_DONE   ((uint32_t)0x1u << 13)      /* RW */
#define MTK_MSDC_INTEN_DATTMO       ((uint32_t)0x1u << 14)      /* RW */
#define MTK_MSDC_INTEN_DATCRCERR    ((uint32_t)0x1u << 15)      /* RW */
#define MTK_MSDC_INTEN_ACMD19_DONE  ((uint32_t)0x1u << 16)      /* RW */
#define MTK_MSDC_INTEN_DMA_BDCSERR  ((uint32_t)0x1u << 17)      /* RW */
#define MTK_MSDC_INTEN_DMA_GPDCSERR ((uint32_t)0x1u << 18)      /* RW */
#define MTK_MSDC_INTEN_DMA_PROTECT  ((uint32_t)0x1u << 19)      /* RW */
#define MTK_MSDC_INTEN_AXIRSPERR    ((uint32_t)0x1u << 23)      /* RW */

/* MTK_MSDC_FIFOCS mask */
#define MTK_MSDC_FIFOCS_RXCNT       (0xffu << 0)      /* R */
#define MTK_MSDC_FIFOCS_TXCNT       ((uint32_t)0xffu << 16)     /* R */
#define MTK_MSDC_FIFOCS_CLR         ((uint32_t)0x1u << 31)      /* RW */

/* MTK_SDC_CFG mask */
#define MTK_SDC_CFG_SDIOINTWKUP     (0x1u << 0)       /* RW */
#define MTK_SDC_CFG_INSWKUP         (0x1u << 1)       /* RW */
#define MTK_SDC_CFG_WRDTOC          (0x1fffu << 2)    /* RW */
#define MTK_SDC_CFG_BUSWIDTH        ((uint32_t)0x3u << 16)      /* RW */
#define MTK_SDC_CFG_SDIO            ((uint32_t)0x1u << 19)      /* RW */
#define MTK_SDC_CFG_SDIOIDE         ((uint32_t)0x1u << 20)      /* RW */
#define MTK_SDC_CFG_INTATGAP        ((uint32_t)0x1u << 21)      /* RW */
#define MTK_SDC_CFG_DTOC            ((uint32_t)0xffu << 24)     /* RW */

/* MTK_SDC_STS mask */
#define MTK_SDC_STS_SDCBUSY         (0x1u << 0)       /* RW */
#define MTK_SDC_STS_CMDBUSY         (0x1u << 1)       /* RW */
#define MTK_SDC_STS_SWR_COMPL       ((uint32_t)0x1u << 31)      /* RW */

/* MTK_SDC_ADV_CFG0 mask */
#define MTK_SDC_ADV_CFG0_RX_ENHANCE_EN ((uint32_t)0x1u << 20)

/* MTK_EMMC_IOCON mask */
#define MTK_EMMC_IOCON_BOOT_RST     (0x1u << 0)       /* RW */

/* MTK_MSDC_DMA_CTRL mask */
#define MTK_MSDC_DMA_CTRL_START     (0x1u << 0)       /* W */
#define MTK_MSDC_DMA_CTRL_STOP      (0x1u << 1)       /* W */
#define MTK_MSDC_DMA_CTRL_RESUME    (0x1u << 2)       /* W */
#define MTK_MSDC_DMA_CTRL_MODE      ((uint16_t)0x1u << 8)       /* RW */
#define MTK_MSDC_DMA_CTRL_LASTBUF   ((uint16_t)0x1u << 10)      /* RW */
#define MTK_MSDC_DMA_CTRL_BRSTSZ    ((uint32_t)0x7u << 12)      /* RW */
#define MTK_MSDC_DMA_CTRL_BRSTSZ_SHIFT  (12u)
#define MTK_MSDC_DMA_CTRL_BRST_8B       (3u)
#define MTK_MSDC_DMA_CTRL_BRST_16B      (4u)
#define MTK_MSDC_DMA_CTRL_BRST_32B      (5u)
#define MTK_MSDC_DMA_CTRL_BRST_64B      (6u)

/* MTK_MSDC_DMA_CFG mask */
#define MTK_MSDC_DMA_CFG_STS        (0x1u << 0)       /* R */
#define MTK_MSDC_DMA_CFG_DECSEN     (0x1u << 1)       /* RW */
#define MTK_MSDC_DMA_CFG_AHBHPROT2  ((uint16_t)0x2u << 8)       /* RW */
#define MTK_MSDC_DMA_CFG_ACTIVEEN   ((uint16_t)0x2u << 12)      /* RW */
#define MTK_MSDC_DMA_CFG_CS12B16B   ((uint32_t)0x1u << 16)      /* RW */

/* MTK_MSDC_PATCH_BIT mask */
#define MTK_MSDC_PATCH_BIT_ODDSUPP      (0x1u << 1)   /* RW */
#define MTK_MSDC_INT_DAT_LATCH_CK_SEL   ((uint16_t)0x7u << 7)   /* RW */
#define MTK_MSDC_CKGEN_MSDC_DLY_SEL     ((uint32_t)0x1fu << 10) /* RW */
#define MTK_MSDC_PATCH_BIT_IODSSEL      ((uint32_t)0x1u << 16)  /* RW */
#define MTK_MSDC_PATCH_BIT_IOINTSEL     ((uint32_t)0x1u << 17)  /* RW */
#define MTK_MSDC_PATCH_BIT_BUSYDLY      ((uint32_t)0xfu << 18)  /* RW */
#define MTK_MSDC_PATCH_BIT_WDOD         ((uint32_t)0xfu << 22)  /* RW */
#define MTK_MSDC_PATCH_BIT_IDRTSEL      ((uint32_t)0x1u << 26)  /* RW */
#define MTK_MSDC_PATCH_BIT_CMDFSEL      ((uint32_t)0x1u << 27)  /* RW */
#define MTK_MSDC_PATCH_BIT_INTDLSEL     ((uint32_t)0x1u << 28)  /* RW */
#define MTK_MSDC_PATCH_BIT_SPCPUSH      ((uint32_t)0x1u << 29)  /* RW */
#define MTK_MSDC_PATCH_BIT_DECRCTMO     ((uint32_t)0x1u << 30)  /* RW */

/* MTK_MSDC_PATCH_BIT1 mask */
#define MTK_MSDC_PATCH_BIT1_FORCE_STOP_GDMA     ((uint32_t)0x1u << 17)    /* RW */
#define MTK_MSDC_PATCH_BIT1_RESET_GDMA          ((uint32_t)0x1u << 15)    /* RW */
#define MTK_MSDC_PATCH_BIT1_STOP_DLY_SEL        ((uint16_t)0xfu << 8)     /* RW */
#define MTK_MSDC_PATCH_BIT1_BUSY_CHECK_SEL      (0x1u << 7)     /* RW */

/* MTK_MSDC_PATCH_BIT2 mask */
#define MTK_MSDC_PATCH_BIT2_RESPWAIT    (0x3u << 2)    /* RW */
#define MTK_MSDC_PB2_SUPPORT_64G        (0x1u << 1)    /* RW */
#define MTK_MSDC_PATCH_BIT2_CFGRESP     ((uint32_t)0x1u << 15)   /* RW */
#define MTK_MSDC_PATCH_BIT2_CFGCRCSTS   ((uint32_t)0x1u << 28)   /* RW */
#define MTK_MSDC_PB2_RESPSTSENSEL       ((uint32_t)0x7u << 16)   /* RW */
#define MTK_MSDC_PB2_CRCSTSENSEL        ((uint32_t)0x7u << 29)   /* RW */

/* MTK_MSDC_PAD_TUNE/MSDC_PAD_TUNE0 mask */
#define MTK_MSDC_PAD_TUNE_DATWRDLY      (0x1fu << 0)   /* RW */
#define MTK_MSDC_PAD_TUNE_DATRRDLY      ((uint32_t)0x1fu << 8)   /* RW */
#define MTK_MSDC_PAD_TUNE_RD_SEL        ((uint32_t)0x1u << 13)   /* RW */
#define MTK_MSDC_PAD_TUNE_RXDLYSEL      ((uint32_t)0x1u << 15)   /* RW */
#define MTK_MSDC_PAD_TUNE_CMDRDLY       ((uint32_t)0x1fu << 16)  /* RW */
#define MTK_MSDC_PAD_TUNE_CMD_SEL       ((uint32_t)0x1u << 21)   /* RW */
#define MTK_MSDC_PAD_TUNE_CMDRRDLY      ((uint32_t)0x1fu << 22)  /* RW */
#define MTK_MSDC_PAD_TUNE_CLKTDLY       ((uint32_t)0x1fu << 27)  /* RW */

#define MTK_PAD_DS_TUNE_DLY1            (0x1fu << 2)   /* RW */
#define MTK_PAD_DS_TUNE_DLY2            ((uint32_t)0x1fu << 7)   /* RW */
#define MTK_PAD_DS_TUNE_DLY3            ((uint32_t)0x1fu << 12)  /* RW */

#define MTK_PAD_CMD_TUNE_RX_DLY3        (0x1fu << 1)   /* RW */

/* MTK_EMMC50_CFG mask */
#define MTK_EMMC50_CFG_PADCMD_LATCHCK   (0x1u << 0)    /* RW */
#define MTK_EMMC50_CFG_CRCSTS_EDGE      (0x1u << 3)    /* RW */
#define MTK_EMMC50_CFG_CFCSTS_SEL       (0x1u << 4)    /* RW */

#define MTK_EMMC50_CFG3_OUTS_WR         (0x1fu << 0)   /* RW */

/* MTK_EMMC_TOP_CONTROL mask */
#define MTK_PAD_RXDLY_SEL               (0x1u << 0)    /* RW */
#define MTK_DELAY_EN                    (0x1u << 1)    /* RW */
#define MTK_PAD_DAT_RD_RXDLY2           (0x1fu << 2)   /* RW */
#define MTK_PAD_DAT_RD_RXDLY            ((uint32_t)0x1fu << 7)   /* RW */
#define MTK_PAD_DAT_RD_RXDLY2_SEL       ((uint32_t)0x1u << 12)   /* RW */
#define MTK_PAD_DAT_RD_RXDLY_SEL        ((uint32_t)0x1u << 13)   /* RW */
#define MTK_DATA_K_VALUE_SEL            ((uint32_t)0x1u << 14)   /* RW */
#define MTK_SDC_RX_ENH_EN               ((uint32_t)0x1u << 15)   /* TW */

/* MTK_EMMC_TOP_CMD mask */
#define MTK_PAD_CMD_RXDLY2              ((uint32_t)0x1fu << 0)   /* RW */
#define MTK_PAD_CMD_RXDLY               ((uint32_t)0x1fu << 5)   /* RW */
#define MTK_PAD_CMD_RD_RXDLY2_SEL       ((uint32_t)0x1u << 10)   /* RW */
#define MTK_PAD_CMD_RD_RXDLY_SEL        ((uint32_t)0x1u << 11)   /* RW */
#define MTK_PAD_CMD_TX_DLY              ((uint32_t)0x1fu << 12)  /* RW */

/* MTK_SDC_FIFO_CFG mask */
#define MTK_SDC_FIFO_CFG_WRVALIDSEL     ((uint32_t)0x1u << 24)   /* RW */
#define MTK_SDC_FIFO_CFG_RDVALIDSEL     ((uint32_t)0x1u << 25)   /* RW */

/* EMMC50_PAD_DS_TUNE mask */
#define MTK_PAD_DS_DLY_SEL              ((uint32_t)0x1u << 16)   /* RW */
#define MTK_PAD_DS_DLY1                 ((uint32_t)0x1fu << 10)  /* RW */
#define MTK_PAD_DS_DLY3                 (0x1fu << 0)   /* RW */

#define MTK_REQ_CMD_EIO                 (0x1u << 0u)
#define MTK_REQ_CMD_TMO                 (0x1u << 1u)
#define MTK_REQ_DAT_ERR                 (0x1u << 2u)
#define MTK_REQ_STOP_EIO                (0x1u << 3u)
#define MTK_REQ_STOP_TMO                (0x1u << 4u)
#define MTK_REQ_CMD_BUSY                (0x1u << 5u)

#define MTK_MSDC_PREPARE_FLAG           (0x1u << 0)
#define MTK_MSDC_ASYNC_FLAG             (0x1u << 1)
#define MTK_MSDC_MMAP_FLAG              (0x1u << 2)

#define MTK_MMC_AUTOSUSPEND_DELAY   50
#define MTK_CMD_TIMEOUT             (HZ / 10 * 5) /* 100ms x5 */
#define MTK_DAT_TIMEOUT             (HZ * 5)      /* 1000ms x5 */

#define MTK_PAD_DELAY_MAX           32 /* PAD delay cells */

#define TUNING_STABLE_TIMES         3
/*--------------------------------------------------------------------------*/
/* Descriptor Structure                                                     */
/*--------------------------------------------------------------------------*/
struct mt_gpdma_desc {
    uint32_t flags;
#define GPDMA_DESC_HWO          (0x1u << 0)
#define GPDMA_DESC_BDP          (0x1u << 1)
#define GPDMA_DESC_CHECKSUM     ((uint16_t)0xffu << 8) /* bit8 ~ bit15 */
#define GPDMA_DESC_INT          ((uint32_t)0x1u << 16)
#define GPDMA_DESC_NEXTH4       ((uint32_t)0xfu << 24)
#define GPDMA_DESC_PTRH4        ((uint32_t)0xfu << 28)
    uint32_t next;
    uint32_t ptr;
    uint32_t gpd_data_len;
#define GPDMA_DESC_BUFLEN       (0xffffffu)   /* bit0 ~ bit23 */
#define GPDMA_DESC_EXTLEN       ((uint32_t)0xffu << 24) /* bit24 ~ bit31 */
    uint32_t arg;
    uint32_t blknum;
    uint32_t cmd;
};

struct mt_bdma_desc {
    uint32_t flags;
#define BDMA_DESC_EOL           (0x1u << 0)
#define BDMA_DESC_CHECKSUM      ((uint16_t)0xffu << 8) /* bit8 ~ bit15 */
#define BDMA_DESC_BLKPAD        ((uint32_t)0x1u << 17)
#define BDMA_DESC_DWPAD         ((uint32_t)0x1u << 18)
#define BDMA_DESC_NEXTH4        ((uint32_t)0xfu << 24)
#define BDMA_DESC_PTRH4         ((uint32_t)0xfu << 28)
    uint32_t next;
    uint32_t ptr;
    uint32_t bd_data_len;
#define BDMA_DESC_BUFLEN        (0xffffff) /* bit0 ~ bit23 */
};

struct msdc_dma {
    struct scatterlist *sg;    /* I/O scatter list */
    struct mt_gpdma_desc *gpd; /* pointer to gpd array */
    struct mt_bdma_desc *bd;   /* pointer to bd array */
    paddr64_t gpd_addr;        /* the physical address of gpd array */
    paddr64_t bd_addr;         /* the physical address of bd array */
};

struct msdc_save_para {
    uint32_t msdc_cfg;
    uint32_t iocon;
    uint32_t sdc_cfg;
    uint32_t pad_tune;
    uint32_t emmc_top_control;
    uint32_t emmc_top_cmd;
    uint32_t patch_bit0;
    uint32_t patch_bit1;
    uint32_t pad_ds_tune;
    uint32_t pad_cmd_tune;
    uint32_t emmc50_cfg0;
};

struct msdc_tune_para {
    uint32_t iocon;
    uint32_t pad_cmd_tune;
    uint32_t emmc_top_control;
    uint32_t emmc_top_cmd;
};

struct msdc_delay_phase {
    uint8_t maxlen;
    uint8_t start;
    uint8_t final_phase;
};

typedef struct mmcsd_scc_t {
    uint32_t clk; /* clock for sdmmc104 */
    uint32_t tap; /* sampling clock position for sdmmc104 */
} sdmmc_scc_t;

typedef struct mtk_msdc_sdmmc {
    uintptr_t base;
    uintptr_t top_base;
    uint32_t basesize;
    uint32_t top_basesize;
    int error;

    int cmd_error;
    struct msdc_dma *dma;

    sdio_sge_t sgl[MTK_MAX_BD_NUM]; /*[DMA_DESC_MAX]*/

    uint32_t timeout_ns;   /* data timeout ns */
    uint32_t timeout_clks; /* data timeout clks */

    uint32_t mclk;         /* mmc subsystem clock frequency */
    uint32_t src_clk_freq; /* source clock frequency */
    uint32_t sclk;         /* SD/MS bus clock frequency */

    uint32_t hs200_cmd_int_delay;          /* cmd internal delay for HS200/SDR104 */
    uint32_t hs400_cmd_int_delay;          /* cmd internal delay for HS400 */
    struct msdc_tune_para def_tune_para;   /* default tune setting */
    struct msdc_tune_para saved_tune_para; /* tune result of CMD21/CMD19 */
} mtk_msdc_sdmmc_t;

extern int mtk_msdc_sdmmc_init(sdio_hc_t *hc);
extern int mtk_msdc_sdmmc_signal_voltage(sdio_hc_t *hc, uint32_t signal_voltage);
extern int mtk_msdc_sdmmc_dinit(sdio_hc_t *hc);

static inline uint32_t mtk_msdc_read32(const uintptr_t base)
{
    uint32_t data;

    data = *((volatile uint32_t*)(base));
    return data;
}

static inline void mtk_msdc_write32(const uintptr_t base, const uint32_t data)
{
    *((volatile uint32_t*)(base)) = data;
}

static inline void sdmmc_set_bits(const uintptr_t reg, const uint32_t val)
{
    mtk_msdc_write32(reg, val | mtk_msdc_read32(reg));
}

static inline void sdmmc_clr_bits(const uintptr_t reg, const uint32_t val)
{
    mtk_msdc_write32(reg, ~val & mtk_msdc_read32(reg));
}

static inline void sdmmc_set_field(const uintptr_t reg, const uint32_t field, const uint32_t val)
{
    unsigned int tv = mtk_msdc_read32(reg);

    tv &= ~field;
    tv |= ((val) << (ffs(field) - 1));
    mtk_msdc_write32(reg, tv);
}

static inline void sdmmc_get_field(const uintptr_t reg, const uint32_t field, uint32_t *val)
{
    const unsigned int tv = mtk_msdc_read32(reg);

    *val = ((tv & field) >> (ffs(field) - 1));
}
#endif
