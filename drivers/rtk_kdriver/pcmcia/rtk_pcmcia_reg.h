/*
  *Copyright (C) 2007-2013 Realtek Semiconductor Corporation.
  */
#ifndef __RTK_PCMCIA_REG_H__
#define __RTK_PCMCIA_REG_H__

/* CRT */
#include <rtk_kdriver/rtk_crt.h>
#include "rtk_pcmcia.h"

#define CONFIG_ENABLE_PCMCIA_FIFO_MODE

#define PCMCIA_SOFT_RESET      SYS_REG_SYS_SRST2_reg
#define PCMCIA_CLOCK_ENABLE    SYS_REG_SYS_CLKEN2_reg

/* TP/PCMCIA pin share */
#ifndef CONFIG_RTK_KDRV_MIO
#include <tp/tp_def.h>
#include <rbus/tp_reg.h>
#define TP_OUT_CTRL_REG             (TP_TP_OUT_CTRL_reg)
#define TP_OUT_SHARE_MODE(x)        TP_TP_OUT_CTRL_share_mode(x)

#define TP_OUT_SHARE_HW_MODE        TP_OUT_SHARE_MODE(1)
#define TP_OUT_SHARE_EN             TP_TP_OUT_CTRL_out_share_en(1)
#define TP_OUT_STOP_MODE            TP_TP_OUT_CTRL_out_stop_mode(1)
#define TP_OUT_DATA_SEL(x)          (((x) & 0x01) << TP_TP_OUT_CTRL_data_sel_shift)
#define TP_OUT_DATA_SEL_PCMCIA      TP_OUT_DATA_SEL(1)

#define TP_OUT_INT_REG              (TP_TP_OUT_INT_reg)
#define TP_PAUSE_INT                (1 << TP_TP_OUT_INT_pause_int_shift)
#define TP_OUT_INT_WRITE_DATA       (1 << TP_TP_OUT_INT_write_data_shift)

#define SET_TP_OUT_CTRL_REG(x)       WRITE_REG32(TP_OUT_CTRL_REG, x)
#define GET_TP_OUT_CTRL_REG()        READ_REG32(TP_OUT_CTRL_REG)
#define GET_TP_OUT_INT_REG()         READ_REG32(TP_OUT_INT_REG)
#define SET_TP_OUT_INT_REG(x)        WRITE_REG32(TP_OUT_INT_REG, x)
#define CLEAR_TP_PUASE_INT()         SET_TP_OUT_INT_REG(TP_PAUSE_INT)
#endif
/* Interrupt control detection */
#include <rbus/misc_reg.h>
#define PCMCIA_INT                  (MISC_isr_reg)
#define PCMCIA_INT_MASK             (MISC_isr_pcmcia_int_mask)

/* JTAG mode detection */
#include <rbus/stb_reg.h>
#define EJTAG_MODE_EN_MASK          (STB_SC_POL_porl_ejtag_en_n_mask)
#define EJTAG_MODE_REG              (STB_SC_POL_reg)
#define GET_EJTAG_MODE_REG()        (rtd_inl(EJTAG_MODE_REG))
#define IS_EJTAG_MODE_ENABLE()      (!(GET_EJTAG_MODE_REG() & EJTAG_MODE_EN_MASK))


/* PCMCIA registers detection */
#if defined(CONFIG_RTK_KDRV_MIO) && ( defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2885P) || defined(CONFIG_ARCH_RTK2851F) )
/**********************************************************************************/
#include <rbus/pcmcia1_reg.h>

#define RTK_PCMCIA1_REG

#define PCMCIA_REG_CMDFF            PCMCIA1_MIS_PCMCIA0_CMDFF_reg
#define PCMCIA_REG_CTRL             PCMCIA1_MIS_PCMCIA0_CR_reg
#define PCMCIA_REG_STS              PCMCIA1_MIS_PCMCIA0_STS_reg
#define PCMCIA_REG_AMTC_0           PCMCIA1_MIS_PCMCIA0_AMTC_0_reg
#define PCMCIA_REG_AMTC_1           PCMCIA1_MIS_PCMCIA0_AMTC_1_reg
#define PCMCIA_REG_IOMTC            PCMCIA1_MIS_PCMCIA0_IOMTC_reg
#define PCMCIA_REG_MATC_0           PCMCIA1_MIS_PCMCIA0_MATC_0_reg
#define PCMCIA_REG_MATC_1           PCMCIA1_MIS_PCMCIA0_MATC_1_reg
#define PCMCIA_REG_ACTRL            PCMCIA1_MIS_PCMCIA0_ACTRL_reg
#define PCMCIA_REG_FIFO_CTRL        PCMCIA1_MIS_PCMCIA0_FIFO_CTRL_reg
#define PCMCIA_REG_NUM_TOTAL        PCMCIA1_MIS_PCMCIA0_NUM_TOTAL_reg
#define PCMCIA_REG_FIFO_STATUS      PCMCIA1_MIS_PCMCIA0_FIFO_STATUS_reg
#define PCMCIA_REG_PIN_SEL          PCMCIA1_MIS_PCMCIA0_PIN_SEL_reg

/*-- PCMCIA_CMDFF --*/
#define PC_CT(x)                   (((x) & 0x01) << PCMCIA1_MIS_PCMCIA0_CMDFF_ct_shift)
#define PC_CT_READ                 PC_CT(0)
#define PC_CT_WRITE                PC_CT(1)
#define PC_AT(x)                   (((x) & 0x01) << PCMCIA1_MIS_PCMCIA0_CMDFF_at_shift)
#define PC_AT_IO                   PC_AT(0)
#define PC_AT_ATTRMEM              PC_AT(1)
#define PC_PA(x)                   (((x) & 0x7FFF) << PCMCIA1_MIS_PCMCIA0_CMDFF_pa_shift)
#define PC_DF(x)                   (((x) & 0xFF) << PCMCIA1_MIS_PCMCIA0_CMDFF_df_shift)

/*-- PCMCIA_CTRL --*/
#define PC_PSR                    (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_psr_shift)
#define PC_CE1_CARD1              (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_ce1_card1_shift)
#define PC_CE1_CARD2              (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_ce1_card2_shift)
#define PC_CE2_CARD1              (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_ce2_card1_shift)
#define PC_CE2_CARD2              (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_ce2_card2_shift)
#define PC_CE_MASK                (PC_CE1_CARD1 | PC_CE1_CARD2 | PC_CE2_CARD1 | PC_CE2_CARD2)

#define PC_PCR1                   (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_pcr1_shift)
#define PC_PCR2                   (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_pcr2_shift)
#define PC_PCR1_OE                (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_pcr1_oe_shift)
#define PC_PCR2_OE                (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_pcr2_oe_shift)
#define PC_PCR_SEL(x)             (((x) & 0x3) << PCMCIA1_MIS_PCMCIA0_CR_pcr_sel_shift)
#define PC_PCR_SEL_MASK           (0x3 << PCMCIA1_MIS_PCMCIA0_CR_pcr_sel_shift)
#define PC_FIFO_R_EN              (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_fifo_r_en_shift)
#define PC_FIFO_W_EN              (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_fifo_w_en_shift)
#define PC_FIFO_ERROR_EN          (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_fifo_erro_en_shift)
#define PC_FIFO_WR_OVER_EN        (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_fifo_wr_over_en_shift)
#define PC_PIIE1                  (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_piie1_shift)
#define PC_PCIIE1                 (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_pciie1_shift)
#define PC_PCRIE1                 (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_pcrie1_shift)
#define PC_PIIE2                  (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_piie2_shift)
#define PC_PCIIE2                 (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_pciie2_shift)
#define PC_PCRIE2                 (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_pcrie2_shift)
#define PC_AFIE                   (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_afie_shift)
#define PC_APFIE                  (0x00000001 << PCMCIA1_MIS_PCMCIA0_CR_apfie_shift)

#define PC_ACCESS_INT             (PC_AFIE | PC_APFIE)
#define PC_CARD_DETECT1_INT       (PC_PCIIE1 | PC_PCRIE1)
#define PC_CARD_DETECT2_INT       (PC_PCIIE2 | PC_PCRIE2)
#define PC_FIFO_INT               (PC_FIFO_R_IE | PC_FIFO_W_IE | PC_FIFO_ERRO_IE | PC_FIFO_WR_OVER_IE)
#define PC_FIFO_RX_INT            (PC_FIFO_R_IE | PC_FIFO_ERRO_IE | PC_FIFO_WR_OVER_IE)
#define PC_FIFO_TX_INT            (PC_FIFO_W_IE | PC_FIFO_ERRO_IE | PC_FIFO_WR_OVER_IE)

/*-- PCMCIA_STS --*/
#define PC_PRES1                  (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_pres1_shift)
#define PC_PRES2                  (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_pres2_shift)
#define PC_FIFO_R_IE              (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_fifo_r_shift)
#define PC_FIFO_W_IE              (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_fifo_w_shift)
#define PC_FIFO_ERRO_IE           (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_fifo_erro_shift)
#define PC_FIFO_WR_OVER_IE        (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_fifo_wr_over_shift)
#define PC_PII1                   (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_pii1_shift)
#define PC_PCII1                  (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_pcii1_shift)
#define PC_PCRI1                  (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_pcri1_shift)
#define PC_PII2                   (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_pii2_shift)
#define PC_PCII2                  (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_pcii2_shift)
#define PC_PCRI2                  (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_pcri2_shift)
#define PC_AFI                    (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_afi_shift)
#define PC_APFI                   (0x00000001 << PCMCIA1_MIS_PCMCIA0_STS_apfi_shift)

/*-- PCMCIA_DEB --*/
#ifdef RTK_PCMCIA_CD_DEBOUNCE
#ifdef CONFIG_RTK_KDRV_MIO
#define PC_CD0_DEB_EN               PCMCIA1_MIS_PCMCIA_CD_DEB2_cd0_debounce_en_mask
#define PC_CD1_DEB_EN               PCMCIA1_MIS_PCMCIA_CD_DEB2_cd1_debounce_en_mask
#define PC_CD0_DEB_INSERT_ISREN     PCMCIA1_MIS_PCMCIA_CD_DEB2_cd0_insert_interrupt_en_mask
#define PC_CD1_DEB_INSERT_ISREN     PCMCIA1_MIS_PCMCIA_CD_DEB2_cd1_insert_interrupt_en_mask
#define PC_CD0_DEB_REMOVE_ISREN     PCMCIA1_MIS_PCMCIA_CD_DEB2_cd0_remove_interrupt_en_mask
#define PC_CD1_DEB_REMOVE_ISREN     PCMCIA1_MIS_PCMCIA_CD_DEB2_cd1_remove_interrupt_en_mask
#else
#define PC_CD0_DEB_EN               PCMCIA1_MIS_PCMCIA_CD_DEB1_cd0_debounce_en_mask
#define PC_CD1_DEB_EN               PCMCIA1_MIS_PCMCIA_CD_DEB1_cd1_debounce_en_mask
#define PC_CD0_DEB_INSERT_ISREN     PCMCIA1_MIS_PCMCIA_CD_DEB1_cd0_insert_interrupt_en_mask
#define PC_CD1_DEB_INSERT_ISREN     PCMCIA1_MIS_PCMCIA_CD_DEB1_cd1_insert_interrupt_en_mask
#define PC_CD0_DEB_REMOVE_ISREN     PCMCIA1_MIS_PCMCIA_CD_DEB1_cd0_remove_interrupt_en_mask
#define PC_CD1_DEB_REMOVE_ISREN     PCMCIA1_MIS_PCMCIA_CD_DEB1_cd1_remove_interrupt_en_mask
#endif
#define PC_CD0_DEB_OUT              PCMCIA1_MIS_PCMCIA_CD_DEB1_cd0_debounce_out_mask
#define PC_CD1_DEB_OUT              PCMCIA1_MIS_PCMCIA_CD_DEB1_cd1_debounce_out_mask
#define PC_CD0_DEB_INSERT_FLAG      PCMCIA1_MIS_PCMCIA_CD_DEB1_cd0_insert_flag_mask
#define PC_CD1_DEB_INSERT_FLAG      PCMCIA1_MIS_PCMCIA_CD_DEB1_cd1_insert_flag_mask
#define PC_CD0_DEB_REMOVE_FLAG      PCMCIA1_MIS_PCMCIA_CD_DEB1_cd0_remove_flag_mask
#define PC_CD1_DEB_REMOVE_FLAG      PCMCIA1_MIS_PCMCIA_CD_DEB1_cd1_remove_flag_mask
#endif
#ifdef CONFIG_MACH_RTK_294X
/* in Mac3 demo board , the interrupt pin is connnected with Int1. */
/* other control pins are connected with Card0. */
#undef PC_PII1
#undef PC_PII2
#undef PC_PIIE1
#undef PC_PIIE2

#define PC_PII1         (0x00000001 << 4)
#define PC_PII2         (0x00000001 << 7)

#define PC_PIIE1        (0x00000001 << 4)
#define PC_PIIE2        (0x00000001 << 7)

#endif    /* */

/*-- PCMCIA_AMTC_0 --*/
#define PC_TWE(x)       PCMCIA1_MIS_PCMCIA0_AMTC_0_twe(x)
#define PC_THD(x)       PCMCIA1_MIS_PCMCIA0_AMTC_0_thd(x)
#define PC_TSU(x)       PCMCIA1_MIS_PCMCIA0_AMTC_0_tsu(x)

/*-- PCMCIA_AMTC_1 --*/
#define PC_TAOE(x)      PCMCIA1_MIS_PCMCIA0_AMTC_1_taoe(x)
#define PC_THCE(x)      PCMCIA1_MIS_PCMCIA0_AMTC_1_thce(x)


/*-- PCMCIA_IOMTC --*/
#define PC_TDIORD(x)     PCMCIA1_MIS_PCMCIA0_IOMTC_tdiord(x)
#define PC_TSUIO(x)      PCMCIA1_MIS_PCMCIA0_IOMTC_tsuio(x)
#define PC_TDINPACK(x)   PCMCIA1_MIS_PCMCIA0_IOMTC_tdinpack(x)
#define PC_TDWT(x)       PCMCIA1_MIS_PCMCIA0_IOMTC_tdwt(x)

/*-- PCMCIA_MATC_0 --*/
#define PC_TC(x)         PCMCIA1_MIS_PCMCIA0_MATC_0_tc(x)
#define PC_THDIO(x)      PCMCIA1_MIS_PCMCIA0_MATC_0_thdio(x)

/*-- PCMCIA_MATC_1 --*/
#define PC_TCIO(x)       PCMCIA1_MIS_PCMCIA0_MATC_1_tcio(x)
#define PC_TWIOWR(x)     PCMCIA1_MIS_PCMCIA0_MATC_1_twiowr(x)

/*-- PCMCIA_ACTRL --*/
#define PC_OE(x)         PCMCIA1_MIS_PCMCIA0_ACTRL_oe(x)


/*-- PCMCIA_FIFO_CTRL --*/
#define PC_ADDR_STEP(x)                     PCMCIA1_MIS_PCMCIA0_FIFO_CTRL_addr_step(x)
#define PC_WR_NUM(x)                        PCMCIA1_MIS_PCMCIA0_FIFO_CTRL_wr_num(x)
#define PC_FIFO_R_TRIG(x)                   PCMCIA1_MIS_PCMCIA0_FIFO_CTRL_fifo_r_trig(x)
#define PC_PIN_SHARE_SEL(x)                 PCMCIA1_MIS_PCMCIA0_FIFO_CTRL_pin_share_sel(x)
#define PC_PIN_SHARE_SEL_SOFT_MODE          PC_PIN_SHARE_SEL(1)
#define PC_FIFO_WR_MODE(x)                  PCMCIA1_MIS_PCMCIA0_FIFO_CTRL_fifo_wr_mode(x)
#define PC_FIFO_WR_MODE_ADDRESS_NOCHANGE    PC_FIFO_WR_MODE(0)
#define PC_FIFO_WR_MODE_ADDRESS_INCREASE    PC_FIFO_WR_MODE(1)
#define PC_FIFO_WR_MODE_ADDRESS_DECREASE    PC_FIFO_WR_MODE(2)
#define PC_FIFO_EN(x)                       PCMCIA1_MIS_PCMCIA0_FIFO_CTRL_fifo_en(x)

/*-- PCMCIA_NUM_TOTAL --*/
#define PC_WR_NUM_TOTAL(x)        PCMCIA1_MIS_PCMCIA0_NUM_TOTAL_wr_num_total(x)

/*-- PCMCIA_FIFO_STATUS --*/
#define PC_FIFO_LEN(x)        PCMCIA1_MIS_PCMCIA0_FIFO_STATUS_get_fifo_len(x)
#define PC_FIFO_EMPTY(x)      PCMCIA1_MIS_PCMCIA0_FIFO_STATUS_get_fifo_empty(x)
#define PC_FIFO_FULL(x)       PCMCIA1_MIS_PCMCIA0_FIFO_STATUS_get_fifo_full(x)


/*-- PCMCIA_PIN_SEL  --*/
#define PC_INPACK1_DIS(x)     PCMCIA1_MIS_PCMCIA0_PIN_SEL_inpack1_dis(x)
#define PC_INPACK0_DIS(x)     PCMCIA1_MIS_PCMCIA0_PIN_SEL_inpack0_dis(x)
#define PC_WAIT1_DIS(x)       PCMCIA1_MIS_PCMCIA0_PIN_SEL_wait1_dis(x)
#define PC_WAIT0_DIS(x)       PCMCIA1_MIS_PCMCIA0_PIN_SEL_wait0_dis(x)

#define SET_PCMCIA_CMDFF(x)                   rtd_outl(PCMCIA_REG_CMDFF,   x)
#define SET_PCMCIA_CTRL(x)                    rtd_outl(PCMCIA_REG_CTRL,    x)
#define SET_PCMCIA_STS(x)                     rtd_outl(PCMCIA_REG_STS,     x)
#define SET_PCMCIA_AMTC_0(x)                  rtd_outl(PCMCIA_REG_AMTC_0,  x)
#define SET_PCMCIA_AMTC_1(x)                  rtd_outl(PCMCIA_REG_AMTC_1,  x)
#define SET_PCMCIA_IOMTC(x)                   rtd_outl(PCMCIA_REG_IOMTC,   x)
#define SET_PCMCIA_MATC_0(x)                  rtd_outl(PCMCIA_REG_MATC_0,  x)
#define SET_PCMCIA_MATC_1(x)                  rtd_outl(PCMCIA_REG_MATC_1,  x)
#define SET_PCMCIA_ACTRL(x)                   rtd_outl(PCMCIA_REG_ACTRL,   x)
#define SET_PCMCIA_FIFO_CTRL(x)               rtd_outl(PCMCIA_REG_FIFO_CTRL,   x)
#define SET_PCMCIA_NUM_TOTAL(x)               rtd_outl(PCMCIA_REG_NUM_TOTAL,   x)
#define SET_PCMCIA_FIFO_STATUS(x)             rtd_outl(PCMCIA_REG_FIFO_STATUS, x)
#define SET_PCMCIA_FIFO_DATA( reg , data )    ( reg = (reg & 0xFFFFFF00) | (data) )


#define GET_PCMCIA_CMDFF()          rtd_inl(PCMCIA_REG_CMDFF)
#define GET_PCMCIA_CTRL()           rtd_inl(PCMCIA_REG_CTRL)
/*#define GET_PCMCIA_STS()          rtd_inl(PCMCIA_REG_STS)*/
#define GET_PCMCIA_AMTC_0()         rtd_inl(PCMCIA_REG_AMTC_0)
#define GET_PCMCIA_AMTC_1()         rtd_inl(PCMCIA_REG_AMTC_1)
#define GET_PCMCIA_IOMTC()          rtd_inl(PCMCIA_REG_IOMTC)
#define GET_PCMCIA_MATC_0()         rtd_inl(PCMCIA_REG_MATC_0)
#define GET_PCMCIA_MATC_1()         rtd_inl(PCMCIA_REG_MATC_1)
#define GET_PCMCIA_ACTRL()          rtd_inl(PCMCIA_REG_ACTRL)
#define GET_PCMCIA_FIFO_CTRL()      rtd_inl(PCMCIA_REG_FIFO_CTRL)
#define GET_PCMCIA_NUM_TOTAL()      rtd_inl(PCMCIA_REG_NUM_TOTAL)
#ifdef RTK_PCMCIA_DIRECT_READ_MIO
#define GET_PCMCIA_FIFO_STATUS()    rtd_inl(PCMCIA_REG_FIFO_STATUS + 0x80)
#else
#define GET_PCMCIA_FIFO_STATUS()    rtd_inl(PCMCIA_REG_FIFO_STATUS)
#endif
#define GET_PCMCIA_FIFO_DATA()      ( (unsigned char)(GET_PCMCIA_CMDFF() & 0xFF) )

//for mark2/mio
#ifdef CONFIG_RTK_KDRV_MIO
#define PCMCIA_DF0 0x1
#define PCMCIA_DF1 0x2
#define PCMCIA_DF2 0x4
#define PCMCIA_DF3 0x8
#define PCMCIA_READ_MODE 0x10000

#define SET_PCMCIA_MIO_FIFO_VALID(x)     rtd_outl(PCMCIA1_MIS_PCMCIA_DFF0CTRL_reg, x)
#define GET_PCMCIA_MIO_FIFO_VALID()      rtd_inl(PCMCIA1_MIS_PCMCIA_DFF0CTRL_reg)

#define SET_PCMCIA_MIO_DATA_FIFO(x)      rtd_outl(PCMCIA1_MIS_PCMCIA_DFF_reg, x)
#ifdef RTK_PCMCIA_DIRECT_READ_MIO
#define GET_PCMCIA_MIO_DATA_FIFO()      rtd_inl(PCMCIA1_MIS_PCMCIA_DFF_reg + 0x80)
#else
#define GET_PCMCIA_MIO_DATA_FIFO()      rtd_inl(PCMCIA1_MIS_PCMCIA_DFF_reg)
#endif //RTK_PCMCIA_DIRECT_READ_MIO
#endif  //CONFIG_RTK_KDRV_MIO
/**********************************************************************************/

#else
/**********************************************************************************/
#include <rbus/pcmcia_reg.h>

#define PCMCIA_REG_CMDFF            PCMCIA_MIS_PCMCIA0_CMDFF_reg
#define PCMCIA_REG_CTRL             PCMCIA_MIS_PCMCIA0_CR_reg
#define PCMCIA_REG_STS              PCMCIA_MIS_PCMCIA0_STS_reg
#define PCMCIA_REG_AMTC_0           PCMCIA_MIS_PCMCIA0_AMTC_0_reg
#define PCMCIA_REG_AMTC_1           PCMCIA_MIS_PCMCIA0_AMTC_1_reg
#define PCMCIA_REG_IOMTC            PCMCIA_MIS_PCMCIA0_IOMTC_reg
#define PCMCIA_REG_MATC_0           PCMCIA_MIS_PCMCIA0_MATC_0_reg
#define PCMCIA_REG_MATC_1           PCMCIA_MIS_PCMCIA0_MATC_1_reg
#define PCMCIA_REG_ACTRL            PCMCIA_MIS_PCMCIA0_ACTRL_reg
#define PCMCIA_REG_FIFO_CTRL        PCMCIA_MIS_PCMCIA0_FIFO_CTRL_reg
#define PCMCIA_REG_NUM_TOTAL        PCMCIA_MIS_PCMCIA0_NUM_TOTAL_reg
#define PCMCIA_REG_FIFO_STATUS      PCMCIA_MIS_PCMCIA0_FIFO_STATUS_reg
#define PCMCIA_REG_PIN_SEL          PCMCIA_MIS_PCMCIA0_PIN_SEL_reg

/*-- PCMCIA_CMDFF --*/
#define PC_CT(x)                   (((x) & 0x01) << PCMCIA_MIS_PCMCIA0_CMDFF_ct_shift)
#define PC_CT_READ                 PC_CT(0)
#define PC_CT_WRITE                PC_CT(1)
#define PC_AT(x)                   (((x) & 0x01) << PCMCIA_MIS_PCMCIA0_CMDFF_at_shift)
#define PC_AT_IO                   PC_AT(0)
#define PC_AT_ATTRMEM              PC_AT(1)
#define PC_PA(x)                   (((x) & 0x7FFF) << PCMCIA_MIS_PCMCIA0_CMDFF_pa_shift)
#define PC_DF(x)                   (((x) & 0xFF) << PCMCIA_MIS_PCMCIA0_CMDFF_df_shift)

/*-- PCMCIA_CTRL --*/
#define PC_PSR                    (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_psr_shift)
#define PC_CE1_CARD1              (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_ce1_card1_shift)
#define PC_CE1_CARD2              (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_ce1_card2_shift)
#define PC_CE2_CARD1              (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_ce2_card1_shift)
#define PC_CE2_CARD2              (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_ce2_card2_shift)
#define PC_CE_MASK                (PC_CE1_CARD1 | PC_CE1_CARD2 | PC_CE2_CARD1 | PC_CE2_CARD2)

#define PC_PCR1                   (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_pcr1_shift)
#define PC_PCR2                   (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_pcr2_shift)
#define PC_PCR1_OE                (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_pcr1_oe_shift)
#define PC_PCR2_OE                (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_pcr2_oe_shift)
#define PC_PCR_SEL(x)             (((x) & 0x3) << PCMCIA_MIS_PCMCIA0_CR_pcr_sel_shift)
#define PC_PCR_SEL_MASK           (0x3 << PCMCIA_MIS_PCMCIA0_CR_pcr_sel_shift)
#define PC_FIFO_R_EN              (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_fifo_r_en_shift)
#define PC_FIFO_W_EN              (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_fifo_w_en_shift)
#define PC_FIFO_ERROR_EN          (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_fifo_erro_en_shift)
#define PC_FIFO_WR_OVER_EN        (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_fifo_wr_over_en_shift)
#define PC_PIIE1                  (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_piie1_shift)
#define PC_PCIIE1                 (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_pciie1_shift)
#define PC_PCRIE1                 (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_pcrie1_shift)
#define PC_PIIE2                  (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_piie2_shift)
#define PC_PCIIE2                 (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_pciie2_shift)
#define PC_PCRIE2                 (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_pcrie2_shift)
#define PC_AFIE                   (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_afie_shift)
#define PC_APFIE                  (0x00000001 << PCMCIA_MIS_PCMCIA0_CR_apfie_shift)

#define PC_ACCESS_INT             (PC_AFIE | PC_APFIE)
#define PC_CARD_DETECT1_INT       (PC_PCIIE1 | PC_PCRIE1)
#define PC_CARD_DETECT2_INT       (PC_PCIIE2 | PC_PCRIE2)
#define PC_FIFO_INT               (PC_FIFO_R_IE | PC_FIFO_W_IE | PC_FIFO_ERRO_IE | PC_FIFO_WR_OVER_IE)
#define PC_FIFO_RX_INT            (PC_FIFO_R_IE | PC_FIFO_ERRO_IE | PC_FIFO_WR_OVER_IE)
#define PC_FIFO_TX_INT            (PC_FIFO_W_IE | PC_FIFO_ERRO_IE | PC_FIFO_WR_OVER_IE)

/*-- PCMCIA_STS --*/
#define PC_PRES1                  (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_pres1_shift)
#define PC_PRES2                  (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_pres2_shift)
#define PC_FIFO_R_IE              (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_fifo_r_shift)
#define PC_FIFO_W_IE              (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_fifo_w_shift)
#define PC_FIFO_ERRO_IE           (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_fifo_erro_shift)
#define PC_FIFO_WR_OVER_IE        (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_fifo_wr_over_shift)
#define PC_PII1                   (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_pii1_shift)
#define PC_PCII1                  (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_pcii1_shift)
#define PC_PCRI1                  (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_pcri1_shift)
#define PC_PII2                   (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_pii2_shift)
#define PC_PCII2                  (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_pcii2_shift)
#define PC_PCRI2                  (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_pcri2_shift)
#define PC_AFI                    (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_afi_shift)
#define PC_APFI                   (0x00000001 << PCMCIA_MIS_PCMCIA0_STS_apfi_shift)

/*-- PCMCIA_DEB --*/
#ifdef RTK_PCMCIA_CD_DEBOUNCE
#ifdef CONFIG_RTK_KDRV_MIO
#define PC_CD0_DEB_EN               PCMCIA_MIS_PCMCIA_CD_DEB2_cd0_debounce_en_mask
#define PC_CD1_DEB_EN               PCMCIA_MIS_PCMCIA_CD_DEB2_cd1_debounce_en_mask
#define PC_CD0_DEB_INSERT_ISREN     PCMCIA_MIS_PCMCIA_CD_DEB2_cd0_insert_interrupt_en_mask
#define PC_CD1_DEB_INSERT_ISREN     PCMCIA_MIS_PCMCIA_CD_DEB2_cd1_insert_interrupt_en_mask
#define PC_CD0_DEB_REMOVE_ISREN     PCMCIA_MIS_PCMCIA_CD_DEB2_cd0_remove_interrupt_en_mask
#define PC_CD1_DEB_REMOVE_ISREN     PCMCIA_MIS_PCMCIA_CD_DEB2_cd1_remove_interrupt_en_mask
#else
#define PC_CD0_DEB_EN               PCMCIA_MIS_PCMCIA_CD_DEB1_cd0_debounce_en_mask
#define PC_CD1_DEB_EN               PCMCIA_MIS_PCMCIA_CD_DEB1_cd1_debounce_en_mask
#define PC_CD0_DEB_INSERT_ISREN     PCMCIA_MIS_PCMCIA_CD_DEB1_cd0_insert_interrupt_en_mask
#define PC_CD1_DEB_INSERT_ISREN     PCMCIA_MIS_PCMCIA_CD_DEB1_cd1_insert_interrupt_en_mask
#define PC_CD0_DEB_REMOVE_ISREN     PCMCIA_MIS_PCMCIA_CD_DEB1_cd0_remove_interrupt_en_mask
#define PC_CD1_DEB_REMOVE_ISREN     PCMCIA_MIS_PCMCIA_CD_DEB1_cd1_remove_interrupt_en_mask
#endif
#define PC_CD0_DEB_OUT              PCMCIA_MIS_PCMCIA_CD_DEB1_cd0_debounce_out_mask
#define PC_CD1_DEB_OUT              PCMCIA_MIS_PCMCIA_CD_DEB1_cd1_debounce_out_mask
#define PC_CD0_DEB_INSERT_FLAG      PCMCIA_MIS_PCMCIA_CD_DEB1_cd0_insert_flag_mask
#define PC_CD1_DEB_INSERT_FLAG      PCMCIA_MIS_PCMCIA_CD_DEB1_cd1_insert_flag_mask
#define PC_CD0_DEB_REMOVE_FLAG      PCMCIA_MIS_PCMCIA_CD_DEB1_cd0_remove_flag_mask
#define PC_CD1_DEB_REMOVE_FLAG      PCMCIA_MIS_PCMCIA_CD_DEB1_cd1_remove_flag_mask
#endif
#ifdef CONFIG_MACH_RTK_294X
/* in Mac3 demo board , the interrupt pin is connnected with Int1. */
/* other control pins are connected with Card0. */
#undef PC_PII1
#undef PC_PII2
#undef PC_PIIE1
#undef PC_PIIE2

#define PC_PII1         (0x00000001 << 4)
#define PC_PII2         (0x00000001 << 7)

#define PC_PIIE1        (0x00000001 << 4)
#define PC_PIIE2        (0x00000001 << 7)

#endif    /* */

/*-- PCMCIA_AMTC_0 --*/
#define PC_TWE(x)       PCMCIA_MIS_PCMCIA0_AMTC_0_twe(x)
#define PC_THD(x)       PCMCIA_MIS_PCMCIA0_AMTC_0_thd(x)
#define PC_TSU(x)       PCMCIA_MIS_PCMCIA0_AMTC_0_tsu(x)

/*-- PCMCIA_AMTC_1 --*/
#define PC_TAOE(x)      PCMCIA_MIS_PCMCIA0_AMTC_1_taoe(x)
#define PC_THCE(x)      PCMCIA_MIS_PCMCIA0_AMTC_1_thce(x)


/*-- PCMCIA_IOMTC --*/
#define PC_TDIORD(x)     PCMCIA_MIS_PCMCIA0_IOMTC_tdiord(x)
#define PC_TSUIO(x)      PCMCIA_MIS_PCMCIA0_IOMTC_tsuio(x)
#define PC_TDINPACK(x)   PCMCIA_MIS_PCMCIA0_IOMTC_tdinpack(x)
#define PC_TDWT(x)       PCMCIA_MIS_PCMCIA0_IOMTC_tdwt(x)

/*-- PCMCIA_MATC_0 --*/
#define PC_TC(x)         PCMCIA_MIS_PCMCIA0_MATC_0_tc(x)
#define PC_THDIO(x)      PCMCIA_MIS_PCMCIA0_MATC_0_thdio(x)

/*-- PCMCIA_MATC_1 --*/
#define PC_TCIO(x)       PCMCIA_MIS_PCMCIA0_MATC_1_tcio(x)
#define PC_TWIOWR(x)     PCMCIA_MIS_PCMCIA0_MATC_1_twiowr(x)

/*-- PCMCIA_ACTRL --*/
#define PC_OE(x)         PCMCIA_MIS_PCMCIA0_ACTRL_oe(x)


/*-- PCMCIA_FIFO_CTRL --*/
#define PC_ADDR_STEP(x)                     PCMCIA_MIS_PCMCIA0_FIFO_CTRL_addr_step(x)
#define PC_WR_NUM(x)                        PCMCIA_MIS_PCMCIA0_FIFO_CTRL_wr_num(x)
#define PC_FIFO_R_TRIG(x)                   PCMCIA_MIS_PCMCIA0_FIFO_CTRL_fifo_r_trig(x)
#define PC_PIN_SHARE_SEL(x)                 PCMCIA_MIS_PCMCIA0_FIFO_CTRL_pin_share_sel(x)
#define PC_PIN_SHARE_SEL_SOFT_MODE          PC_PIN_SHARE_SEL(1)
#define PC_FIFO_WR_MODE(x)                  PCMCIA_MIS_PCMCIA0_FIFO_CTRL_fifo_wr_mode(x)
#define PC_FIFO_WR_MODE_ADDRESS_NOCHANGE    PC_FIFO_WR_MODE(0)
#define PC_FIFO_WR_MODE_ADDRESS_INCREASE    PC_FIFO_WR_MODE(1)
#define PC_FIFO_WR_MODE_ADDRESS_DECREASE    PC_FIFO_WR_MODE(2)
#define PC_FIFO_EN(x)                       PCMCIA_MIS_PCMCIA0_FIFO_CTRL_fifo_en(x)

/*-- PCMCIA_NUM_TOTAL --*/
#define PC_WR_NUM_TOTAL(x)        PCMCIA_MIS_PCMCIA0_NUM_TOTAL_wr_num_total(x)

/*-- PCMCIA_FIFO_STATUS --*/
#define PC_FIFO_LEN(x)        PCMCIA_MIS_PCMCIA0_FIFO_STATUS_get_fifo_len(x)
#define PC_FIFO_EMPTY(x)      PCMCIA_MIS_PCMCIA0_FIFO_STATUS_get_fifo_empty(x)
#define PC_FIFO_FULL(x)       PCMCIA_MIS_PCMCIA0_FIFO_STATUS_get_fifo_full(x)


/*-- PCMCIA_PIN_SEL  --*/
#define PC_INPACK1_DIS(x)     PCMCIA_MIS_PCMCIA0_PIN_SEL_inpack1_dis(x)
#define PC_INPACK0_DIS(x)     PCMCIA_MIS_PCMCIA0_PIN_SEL_inpack0_dis(x)
#define PC_WAIT1_DIS(x)       PCMCIA_MIS_PCMCIA0_PIN_SEL_wait1_dis(x)
#define PC_WAIT0_DIS(x)       PCMCIA_MIS_PCMCIA0_PIN_SEL_wait0_dis(x)

#define SET_PCMCIA_CMDFF(x)                   rtd_outl(PCMCIA_REG_CMDFF,   x)
#define SET_PCMCIA_CTRL(x)                    rtd_outl(PCMCIA_REG_CTRL,    x)
#define SET_PCMCIA_STS(x)                     rtd_outl(PCMCIA_REG_STS,     x)
#define SET_PCMCIA_AMTC_0(x)                  rtd_outl(PCMCIA_REG_AMTC_0,  x)
#define SET_PCMCIA_AMTC_1(x)                  rtd_outl(PCMCIA_REG_AMTC_1,  x)
#define SET_PCMCIA_IOMTC(x)                   rtd_outl(PCMCIA_REG_IOMTC,   x)
#define SET_PCMCIA_MATC_0(x)                  rtd_outl(PCMCIA_REG_MATC_0,  x)
#define SET_PCMCIA_MATC_1(x)                  rtd_outl(PCMCIA_REG_MATC_1,  x)
#define SET_PCMCIA_ACTRL(x)                   rtd_outl(PCMCIA_REG_ACTRL,   x)
#define SET_PCMCIA_FIFO_CTRL(x)               rtd_outl(PCMCIA_REG_FIFO_CTRL,   x)
#define SET_PCMCIA_NUM_TOTAL(x)               rtd_outl(PCMCIA_REG_NUM_TOTAL,   x)
#define SET_PCMCIA_FIFO_STATUS(x)             rtd_outl(PCMCIA_REG_FIFO_STATUS, x)
#define SET_PCMCIA_FIFO_DATA( reg , data )    ( reg = (reg & 0xFFFFFF00) | (data) )


#define GET_PCMCIA_CMDFF()          rtd_inl(PCMCIA_REG_CMDFF)
#define GET_PCMCIA_CTRL()           rtd_inl(PCMCIA_REG_CTRL)
/*#define GET_PCMCIA_STS()          rtd_inl(PCMCIA_REG_STS)*/
#define GET_PCMCIA_AMTC_0()         rtd_inl(PCMCIA_REG_AMTC_0)
#define GET_PCMCIA_AMTC_1()         rtd_inl(PCMCIA_REG_AMTC_1)
#define GET_PCMCIA_IOMTC()          rtd_inl(PCMCIA_REG_IOMTC)
#define GET_PCMCIA_MATC_0()         rtd_inl(PCMCIA_REG_MATC_0)
#define GET_PCMCIA_MATC_1()         rtd_inl(PCMCIA_REG_MATC_1)
#define GET_PCMCIA_ACTRL()          rtd_inl(PCMCIA_REG_ACTRL)
#define GET_PCMCIA_FIFO_CTRL()      rtd_inl(PCMCIA_REG_FIFO_CTRL)
#define GET_PCMCIA_NUM_TOTAL()      rtd_inl(PCMCIA_REG_NUM_TOTAL)
#ifdef RTK_PCMCIA_DIRECT_READ_MIO
#define GET_PCMCIA_FIFO_STATUS()    rtd_inl(PCMCIA_REG_FIFO_STATUS + 0x80)
#else
#define GET_PCMCIA_FIFO_STATUS()    rtd_inl(PCMCIA_REG_FIFO_STATUS)
#endif
#define GET_PCMCIA_FIFO_DATA()      ( (unsigned char)(GET_PCMCIA_CMDFF() & 0xFF) )

//for mark2/mio
#ifdef CONFIG_RTK_KDRV_MIO
#define PCMCIA_DF0 0x1
#define PCMCIA_DF1 0x2
#define PCMCIA_DF2 0x4
#define PCMCIA_DF3 0x8
#define PCMCIA_READ_MODE 0x10000

#define SET_PCMCIA_MIO_FIFO_VALID(x)     rtd_outl(PCMCIA_MIS_PCMCIA_DFF0CTRL_reg, x)
#define GET_PCMCIA_MIO_FIFO_VALID()      rtd_inl(PCMCIA_MIS_PCMCIA_DFF0CTRL_reg)

#define SET_PCMCIA_MIO_DATA_FIFO(x)      rtd_outl(PCMCIA_MIS_PCMCIA_DFF_reg, x)
#ifdef RTK_PCMCIA_DIRECT_READ_MIO
#define GET_PCMCIA_MIO_DATA_FIFO()      rtd_inl(PCMCIA_MIS_PCMCIA_DFF_reg + 0x80)
#else
#define GET_PCMCIA_MIO_DATA_FIFO()      rtd_inl(PCMCIA_MIS_PCMCIA_DFF_reg)
#endif //RTK_PCMCIA_DIRECT_READ_MIO
#endif  //CONFIG_RTK_KDRV_MIO

#endif  //pcmcia_reg.h
/**********************************************************************************/

#endif
