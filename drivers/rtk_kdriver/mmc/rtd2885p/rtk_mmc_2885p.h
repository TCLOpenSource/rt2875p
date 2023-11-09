/*************************************************************************
	> File Name: rtk_mmc_6748.h
	> Author: bennychen
	> Mail: bennytschen@realtek.com
	> Created Time: Wed, 26 May 2021 13:32:11 +0800
 ************************************************************************/
#ifndef _RTK_MMC_6748_H_
#define _RTK_MMC_6748_H_

#include <linux/bitops.h>

#ifdef CONFIG_RTK_KDRIVER_SUPPORT
#include <rtk_kdriver/rtk_crt.h>
#else
#include <mach/rtk_crt.h>
#endif

#include <rbus/sb2_reg.h>
#include <rbus/pinmux_reg.h>
#include <rbus/emmc_wrap_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>

/* *** Realtek INT register *** */
/* xxx_DesignSpec_CRT.doc */
/* Check Address and Bit */
#define EMMC_SCPU_INT           (SYS_REG_INT_CTRL_SCPU_2_reg)
#define EMMC_SINT_MASK          (SYS_REG_INT_CTRL_SCPU_2_emmc_security_int_scpu_routing_en_mask)
//#define EMMC_KCPU_INT           (SYS_REG_INT_CTRL_KCPU_2_reg)
//#define EMMC_KINT_MASK          (BIT(7))

/* *** Realtek CRT register *** */
/* xxx_DesignSpec_SB2_ARCH.doc */
#define EMMC_HW_SEMAPHORE       (SB2_HD_SEM_NEW_2_reg) // must check

/* xxxx_DesignSpec_Pinmux.doc */
/* register of iopad are move into MMC IP  */
#define RTK_IOPAD_CFG1          (EMMC_WRAP_EMMCCLK_CFG_0_reg)   //EMMCCLK_CFG_0
#define RTK_IOPAD_CFG2          (EMMC_WRAP_EMMC_CFG_0_reg)      //EMMC_CFG_0
#define RTK_IOPAD_CFG3          (EMMC_WRAP_EMMC_CFG_1_reg)      //EMMC_CFG_1

#define RTK_IOPAD_SET1          (EMMC_WRAP_EMMCCLK_EXTRACFG_NP4E_0_reg)  //EMMCCLK_EXTRACFG_NP4E_0
#define RTK_IOPAD_SET2          (EMMC_WRAP_EMMC_EXTRACFG_NP4E_0_reg)    //EMMC_EXTRACFG_NP4E_0
#define RTK_IOPAD_SET3          (EMMC_WRAP_EMMC_EXTRACFG_NP4E_1_reg)    //EMMC_EXTRACFG_NP4E_1
#define RTK_IOPAD_SET4          (EMMC_WRAP_EMMC_EXTRACFG_NP4E_2_reg)    //EMMC_EXTRACFG_NP4E_2

#define RTK_IP_MBIST            (EMMC_WRAP_emmc_ip_bist_reg)  //emmc_ip_bist
#define RTK_IP_MBIST_LS_SHIFT0  (EMMC_WRAP_emmc_ip_bist_emmc_ip_ls_0_shift)
#define RTK_IP_MBIST_LS_MASK0   (EMMC_WRAP_emmc_ip_bist_emmc_ip_ls_0_mask>>RTK_IP_MBIST_LS_SHIFT0)
#define RTK_IP_MBIST_LS_SHIFT1  (EMMC_WRAP_emmc_ip_bist_emmc_ip_ls_1_shift)
#define RTK_IP_MBIST_LS_MASK1   (EMMC_WRAP_emmc_ip_bist_emmc_ip_ls_1_mask>>RTK_IP_MBIST_LS_SHIFT1)
#define RTK_WRAP_MBIST          (EMMC_WRAP_emmc_wrap_bist_and_emmc_ip_bist_reg)  //emmc_wrap_bist
#define RTK_WRAP_MBIST_LS_SHIFT (EMMC_WRAP_emmc_wrap_bist_and_emmc_ip_bist_emmc_wrap_ls_shift)  //emmc_wrap_bist
#define RTK_WRAP_MBIST_LS_MASK  (EMMC_WRAP_emmc_wrap_bist_and_emmc_ip_bist_emmc_wrap_ls_mask >> RTK_WRAP_MBIST_LS_SHIFT)  //emmc_wrap_bist

#define RTK_EM_DW_SRST          SYS_REG_SYS_SRST2_reg
#define RTK_EM_DW_CLKEN         SYS_REG_SYS_CLKEN2_reg
#define RTK_EM_SRST_SHT         SYS_REG_SYS_SRST2_rstn_emmc_shift
#define RTK_EM_CLKEN_SHT        SYS_REG_SYS_CLKEN2_clken_emmc_shift

#define RTK_EMMC_33V_SEL        EMMC_WRAP_emmc_33v_sel_reg

#define RTK_SD_SRST             SYS_REG_SYS_SRST3_reg
#define RTK_SD_CLKSEL           SYS_REG_SYS_CLKSEL_reg
#define RTK_SD_CLKEN            SYS_REG_SYS_CLKEN3_reg
#define RTK_SD_INTEN_S          SYS_REG_INT_CTRL_SCPU_reg
#define RTK_SD_INTEN_K          SYS_REG_INT_CTRL_KCPU_reg

#define RTK_SD_SRST_SHT         SYS_REG_SYS_SRST3_rstn_sd_shift
#define RTK_SD_CLKEN_SHT        SYS_REG_SYS_CLKEN3_clken_sd_shift
#define RTK_SD_INT_SHT_S        SYS_REG_INT_CTRL_SCPU_sd_int_scpu_routing_en_shift
#define RTK_SD_INT_SHT_K        SYS_REG_INT_CTRL_KCPU_sd_int_kcpu_routing_en_shift
#define RTK_CRT_WRT_MASK        BIT(0)

#define RSTN_EM_DW              BIT(RTK_EM_SRST_SHT)
#define CLKEN_EM_DW             BIT(RTK_EM_CLKEN_SHT)
#define RSTN_SD                 BIT(RTK_SD_SRST_SHT)
#define CLKEN_SD                BIT(RTK_SD_CLKEN_SHT)

#define CR_BASE_ADDR            0xb8010C00
#define EM_BASE_ADDR            (EMMC_WRAP_emmc_scpu_sel_reg)

/* PLL and Phase relation */
/* common\include\rbus\pll27x_reg_reg.h */
#define EMMC_PLL_SET            PLL27X_REG_EMMC_PLL_SET_reg
#define EMMC_PLL_CTRL           PLL27X_REG_EMMC_PLL_CTRL_reg
#define EMMC_PLLPHASE_CTRL      PLL27X_REG_EMMC_PLLPHASE_CTRL_reg
#define EMMC_PLL_PHASE_INTERPOLATION    PLL27X_REG_EMMC_PLL_PHASE_INTERPOLATION_reg
#define EMMC_PLL_SSC0           PLL27X_REG_EMMC_PLL_SSC0_reg
#define EMMC_PLL_SSC1           PLL27X_REG_EMMC_PLL_SSC1_reg
#define EMMC_PLL_SSC2           PLL27X_REG_EMMC_PLL_SSC2_reg
#define EMMC_PLL_SSC3           PLL27X_REG_EMMC_PLL_SSC3_reg
#define EMMC_PLL_SSC4           PLL27X_REG_EMMC_PLL_SSC4_reg
#define EMMC_PLL_SSC5           PLL27X_REG_EMMC_PLL_SSC5_reg
#define EMMC_PLL_SSC6           PLL27X_REG_EMMC_PLL_SSC6_reg

/* PLL_SET Mask and Shift */
#define EMMC_PLL_SET_ALL_RSTB_SHFIT     PLL27X_REG_EMMC_PLL_SET_pllallrstb_shift
#define EMMC_PLL_SET_ALL_RSTB_MASK      (PLL27X_REG_EMMC_PLL_SET_pllallrstb_mask>>EMMC_PLL_SET_ALL_RSTB_SHFIT)
#define EMMC_PLL_SET_PLLSN_SHFIT        PLL27X_REG_EMMC_PLL_SET_pllsn_shift
#define EMMC_PLL_SET_PLLSN_MASK         (PLL27X_REG_EMMC_PLL_SET_pllsn_mask>>EMMC_PLL_SET_PLLSN_SHFIT)
#define EMMC_PLL_SET_PLLSCP_SHFIT       PLL27X_REG_EMMC_PLL_SET_pllscp_shift
#define EMMC_PLL_SET_PLLSCP_MASK        (PLL27X_REG_EMMC_PLL_SET_pllscp_mask>>EMMC_PLL_SET_PLLSCP_SHFIT)
#define EMMC_PLL_SET_PLLSCS_SHFIT       PLL27X_REG_EMMC_PLL_SET_pllscs_shift
#define EMMC_PLL_SET_PLLSCS_MASK        (PLL27X_REG_EMMC_PLL_SET_pllscs_mask>>EMMC_PLL_SET_PLLSCS_SHFIT)
#define EMMC_PLL_SET_PLLSSI_SHFIT       PLL27X_REG_EMMC_PLL_SET_pllssi_shift
#define EMMC_PLL_SET_PLLSSI_MASK        (PLL27X_REG_EMMC_PLL_SET_pllssi_mask>>EMMC_PLL_SET_PLLSSI_SHFIT)
#define EMMC_PLL_SET_PLLSDIV2_SHFIT     PLL27X_REG_EMMC_PLL_SET_pllsdiv2_shift
#define EMMC_PLL_SET_PLLSDIV2_MASK      (PLL27X_REG_EMMC_PLL_SET_pllsdiv2_mask>>EMMC_PLL_SET_PLLSDIV2_SHFIT)
#define EMMC_PLL_SET_CKSSC_INV_SHFIT    PLL27X_REG_EMMC_PLL_SET_pll_emmc_ckssc_inv_shift
#define EMMC_PLL_SET_CKSSC_INV_MASK     (PLL27X_REG_EMMC_PLL_SET_pll_emmc_ckssc_inv_mask>>EMMC_PLL_SET_CKSSC_INV_SHFIT)

/* PLL_CTRL Mask and Shift */
#define EMMC_PLL_CTRL_POWER_SHFIT       PLL27X_REG_EMMC_PLL_CTRL_pllemmcpwdn_shift /* POWER = CPWDN & LDOPOW */
#define EMMC_PLL_CTRL_POWER_MASK        ((PLL27X_REG_EMMC_PLL_CTRL_pllemmcpwdn_mask|PLL27X_REG_EMMC_PLL_CTRL_pllemmcldopow_mask)>>EMMC_PLL_CTRL_POWER_SHFIT)
#define EMMC_PLL_CTRL_FREEZE_SHFIT      PLL27X_REG_EMMC_PLL_CTRL_pllemmc_freeze_shift
#define EMMC_PLL_CTRL_FREEZE_MASK       (PLL27X_REG_EMMC_PLL_CTRL_pllemmc_freeze_mask>>EMMC_PLL_CTRL_FREEZE_SHFIT)
#define EMMC_PLL_CTRL_RSVD_L_SHFIT      PLL27X_REG_EMMC_PLL_CTRL_pllemmc_rsvd_shift
#define EMMC_PLL_CTRL_RSVD_L_MASK       ((PLL27X_REG_EMMC_PLL_CTRL_pllemmc_rsvd_mask>>EMMC_PLL_CTRL_RSVD_L_SHFIT) & 0xF)
#define EMMC_PLL_CTRL_RSVD_H_SHFIT      (EMMC_PLL_CTRL_RSVD_L_SHFIT + 4)
#define EMMC_PLL_CTRL_RSVD_H_MASK       ((PLL27X_REG_EMMC_PLL_CTRL_pllemmc_rsvd_mask>>EMMC_PLL_CTRL_RSVD_H_SHFIT) & 0xF)

/* PLL_SSCx Mask and Shift */
#define EMMC_PLL_SSC0_OC_EN_SHIFT       PLL27X_REG_EMMC_PLL_SSC0_pllemmc_oc_en_shift
#define EMMC_PLL_SSC0_OC_EN_MASK        (PLL27X_REG_EMMC_PLL_SSC0_pllemmc_oc_en_mask>>EMMC_PLL_SSC0_OC_EN_SHIFT)
#define EMMC_PLL_SSC0_OC_STEP_SHIFT     PLL27X_REG_EMMC_PLL_SSC0_pllemmc_oc_step_set_shift
#define EMMC_PLL_SSC0_OC_STEP_MASK      (PLL27X_REG_EMMC_PLL_SSC0_pllemmc_oc_step_set_mask>>EMMC_PLL_SSC0_OC_STEP_SHIFT)

#define EMMC_PLL_SSC1_PI_CUR_SEL_SHIFT      PLL27X_REG_PLL_SSC1_sel_oc_mode_shift
#define EMMC_PLL_SSC1_PI_CUR_SEL_MASK       (PLL27X_REG_PLL_SSC1_sel_oc_mode_mask>>EMMC_PLL_SSC1_PI_CUR_SEL_SHIFT)

#define EMMC_PLL_SSC2_EN_PI_DEBUG_SHIFT     PLL27X_REG_EMMC_PLL_SSC2_pllemmc_en_pi_debug_shift
#define EMMC_PLL_SSC2_EN_PI_DEBUG_MASK      (PLL27X_REG_EMMC_PLL_SSC2_pllemmc_en_pi_debug_mask>>EMMC_PLL_SSC2_EN_PI_DEBUG_SHIFT)

#define EMMC_PLL_SSC3_NCODE_SHIFT       PLL27X_REG_EMMC_PLL_SSC3_pllemmc_ncode_ssc_shift
#define EMMC_PLL_SSC3_NCODE_MASK        (PLL27X_REG_EMMC_PLL_SSC3_pllemmc_ncode_ssc_mask>>EMMC_PLL_SSC3_NCODE_SHIFT)
#define EMMC_PLL_SSC3_FCODE_SHIFT       PLL27X_REG_EMMC_PLL_SSC3_pllemmc_fcode_ssc_shift
#define EMMC_PLL_SSC3_FCODE_MASK        (PLL27X_REG_EMMC_PLL_SSC3_pllemmc_fcode_ssc_mask>>EMMC_PLL_SSC3_FCODE_SHIFT)

#define EMMC_PLL_SSC4_NCODE_SHIFT       PLL27X_REG_EMMC_PLL_SSC4_pllemmc_ncode_t_shift
#define EMMC_PLL_SSC4_NCODE_MASK        (PLL27X_REG_EMMC_PLL_SSC4_pllemmc_ncode_t_mask>>EMMC_PLL_SSC4_NCODE_SHIFT)
#define EMMC_PLL_SSC4_FCODE_SHIFT       PLL27X_REG_EMMC_PLL_SSC4_pllemmc_fcode_t_shif
#define EMMC_PLL_SSC4_FCODE_MASK        (PLL27X_REG_EMMC_PLL_SSC4_pllemmc_fcode_t_mask>>EMMC_PLL_SSC4_FCODE_SHIFT)

#define EMMC_PLL_SSC5_DOT_GRAN_SHIFT       PLL27X_REG_EMMC_PLL_SSC5_pllemmc_dot_gran_shift
#define EMMC_PLL_SSC5_DOT_GRAN_MASK        (PLL27X_REG_EMMC_PLL_SSC5_pllemmc_dot_gran_mask>>EMMC_PLL_SSC5_DOT_GRAN_SHIFT)
#define EMMC_PLL_SSC5_GRAN_EST_SHIFT       PLL27X_REG_EMMC_PLL_SSC5_pllemmc_gran_est_shift
#define EMMC_PLL_SSC5_GRAN_EST_MASK        (PLL27X_REG_EMMC_PLL_SSC5_pllemmc_gran_est_mask>>EMMC_PLL_SSC5_GRAN_EST_SHIFT)
#define EMMC_PLL_SSC5_EN_SSC_SHIFT         PLL27X_REG_EMMC_PLL_SSC5_pllemmc_en_ssc_shift
#define EMMC_PLL_SSC5_EN_SSC_MASK          (PLL27X_REG_EMMC_PLL_SSC5_pllemmc_en_ssc_mask>>EMMC_PLL_SSC5_EN_SSC_SHIFT)

#define EMMC_PLL_SSC6_OC_DONE_SHIFT         PLL27X_REG_EMMC_PLL_SSC6_pllemmc_oc_done_shift
#define EMMC_PLL_SSC6_OC_DONE_MASK          (PLL27X_REG_EMMC_PLL_SSC6_pllemmc_oc_done_mask>>EMMC_PLL_SSC6_OC_DONE_SHIFT)

/* PLL27X_REG_EMMC_PLLPHASE_CTRL_reg */
#define PHASE_W_SHT             PLL27X_REG_EMMC_PLLPHASE_CTRL_pllssck_shift
#define PHASE_W_MASK            (PLL27X_REG_EMMC_PLLPHASE_CTRL_pllssck_mask>>PHASE_W_SHT)
#define PHASE_R_SHT             PLL27X_REG_EMMC_PLLPHASE_CTRL_pllssck_2_shift
#define PHASE_R_MASK            (PLL27X_REG_EMMC_PLLPHASE_CTRL_pllssck_2_mask >> PLL27X_REG_EMMC_PLLPHASE_CTRL_pllssck_2_shift)
/* PLL27X_REG_EMMC_PLL_PHASE_INTERPOLATION_reg */
#define PHASE_C_SHT             PLL27X_REG_EMMC_PLL_PHASE_INTERPOLATION_pllssck_4_shift
#define PHASE_C_MASK            (PLL27X_REG_EMMC_PLL_PHASE_INTERPOLATION_pllssck_4_mask >> PLL27X_REG_EMMC_PLL_PHASE_INTERPOLATION_pllssck_4_shift)
/* EMMC_WRAP_XXX_reg */
/* EM_HALF_CYCLE_CAL_EN=EMMC_WRAP_half_cycle_cal_en_reg */
#define DS_CYCLE_CAL_EN_SHIFT   EMMC_WRAP_half_cycle_cal_en_half_cycle_cal_en_shift
#define DS_CYCLE_CAL_EN_MASK    (EMMC_WRAP_half_cycle_cal_en_half_cycle_cal_en_mask>>DS_CYCLE_CAL_EN_SHIFT)
#define DS_CYCLE_FW_MODE_SHIFT  EMMC_WRAP_half_cycle_cal_en_fw_mode_shift
#define DS_CYCLE_FW_MODE_MASK   (EMMC_WRAP_half_cycle_cal_en_fw_mode_mask>>DS_CYCLE_FW_MODE_SHIFT)
#define DS_CAL_MODE_SHIFT       EMMC_WRAP_half_cycle_cal_en_half_cycle_res_mod_shift
#define DS_CAL_MODE_MASK        (EMMC_WRAP_half_cycle_cal_en_half_cycle_res_mod_mask>>DS_CAL_MODE_SHIFT)
#define DS_CAL_THR_SHIFT        EMMC_WRAP_half_cycle_cal_en_half_cycle_res_thr_shift
#define DS_CAL_THR_MASK         (EMMC_WRAP_half_cycle_cal_en_half_cycle_res_thr_mask>>DS_CAL_THR_SHIFT)
/* EM_HALF_CYCLE_CAL_RESULT=EMMC_WRAP_half_cycle_cal_result_reg */
#define DS_CAL_RES_SHIFT        EMMC_WRAP_half_cycle_cal_result_half_cycle_cal_result_shift
#define DS_CAL_RES_MASK         (EMMC_WRAP_half_cycle_cal_result_half_cycle_cal_result_mask>>DS_CAL_RES_SHIFT)
/* EM_DS_TUNE_CTRL=EMMC_WRAP_ds_tune_ctrl_reg*/
#define DS_VALUE_SHIFT          EMMC_WRAP_ds_tune_ctrl_bypass_ds_ph_sel_shift
#define DS_VALUE_MASK           (EMMC_WRAP_ds_tune_ctrl_bypass_ds_ph_sel_mask>>DS_VALUE_SHIFT)
/* *** Realtek CRT register &&& */

/* GPIO setting */

/* xxx_DesignSpec_MISC_GPIO.doc */
//#include <rbus/mis_gpio_reg.h>
//
//#define MIS_PINMUX_BASE     (PINMUX_GPIO_TLEFT_CFG_0_reg)
//#define MIS_GPDIR_BASE      (MIS_GPIO_GP0DIR_reg)
//#define MIS_GPDATO_BASE     (MIS_GPIO_GP0DATO_reg)
//#define MIS_GPDATI_BASE     (MIS_GPIO_GP0DATI_reg)
//#define MIS_GPIE_BASE       (MIS_GPIO_GP0IE_reg)


/* xxx_DesignSpec_ISO_MISC_GPIO.doc */
//#include <rbus/gpio_reg.h>
//
//#define ISO_PINMUX_BASE     (PINMUX_ST_STB_0_reg)
//#define ISO_GPDIR_BASE      (GPIO_GPDIR_reg)
//#define ISO_GPDATO_BASE     (GPIO_GPDATO_reg)
//#define ISO_GPDATI_BASE     (GPIO_GPDATI_reg)
//#define ISO_GPIE_BASE       (GPIO_GPIE_reg)
//
//
//#define ISO_GPIO_UNKNOW     (0xff)
//#define MIS_GPIO_UNKNOW     (0xff)
//#define MAX_ISO_GPIO_CNT    (30)
//#define MAX_MIS_GPIO_CNT    (153)

/* Pinmux setting is depended on IC */

#endif

