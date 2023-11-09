/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for scaler clock related functions.
 *
 * @author 	$Author$
 * @date		$Date$
 * @version	$Revision$
 * @ingroup	clock
 */

/**
 * @addtogroup clock
 * @{
 */

#ifndef CONFIG_VPQ_TABLE_GENERATE
#define CONFIG_VPQ_TABLE_GENERATE 0
#endif

#ifndef _ULTRAZOOM_H_
#define _ULTRAZOOM_H_

#if !CONFIG_VPQ_TABLE_GENERATE
#define printf(...)
#endif
#include <scaler/vipRPCCommon.h>	/* in common*/
/*#include "rtd_types.h"*/
#define VIP_Update_NNSR_DMA_inTSK 1
/*============================ Module dependency  ===========================*/
/*===================================  Types ================================*/
typedef struct _SZIE {
	unsigned int	nWidth;
	unsigned int	nLength;
} SIZE;

typedef struct {
    unsigned char Vcti_En;
    unsigned char Vcti_Gain;
    unsigned char Vcti_Type3_Thd;
} DRV_VipVcti_t;



typedef enum{
	NOT_PR_MODE = 0,
	PR_4TAP_MODE,
	PR_2TAP_MODE,
} UZU_PR_MODE;

/* move to vip common*/
/*
enum {
	SDFIR64_90_20 = 0,
	SDFIR64_70_20,
	SDFIR64_40_20,
	SDFIR64_35_20,
	SDFIR64_32_20,
	SDFIR64_50_25Hamm,
	SDFIR64_Blur,
	SDFIR64_Mid,
	SDFIR64_Sharp,
	SDFIR64_45_20Hamm,
	SDFIR64_30_20Hamm,
	SDFIR64_25_20Hamm,
	SDFIR64_25_15Hamm,
	SDFIR64_60_10Hamm,
	SDFIR64_2tap,
	SDFIR64_20_10Hamm,
	SDFIR64_20_10,
	SDFIR64_15_10Hamm,
	SDFIR64_MAXNUM
};
*/
#define DRV_SU_table_H12tap_t_NUM 41
typedef struct {
	unsigned char hor_mode_sel;
	short *coef_table;
	unsigned char tap_num;
} DRV_SU_table_H12tap_t;

typedef struct {
	unsigned char ver_mode_sel;
	short *coef_table;
	unsigned char tap_num;
} DRV_SU_table_V8tap_t;



typedef struct {
	char Pk_Mask_C0;
	char Pk_Mask_C1;
	char Pk_Mask_C2;
	char Pk_Mask_C3;

} DRV_SU_PK_Mask;

typedef struct {
	unsigned char Pk_En;
	unsigned char Pk_Coring;
	unsigned char Pk_X1;
	unsigned char Pk_NdLmtP;
	unsigned char Pk_NdLmtN;
	unsigned char Pk_NdG1;
	unsigned char Pk_NdG2;
	unsigned char ndOffset;

} DRV_SU_PK_Coeff;

typedef struct {
	unsigned short low_pass_filter_en;
	unsigned short vcti_lpf_lbound1;
	unsigned short vcti_lpf_hbound1;
	unsigned short vcti_lpf_lbound2;
	unsigned short vcti_lpf_hbound2;
	unsigned short vcti_lpf_mode;
} DRV_D_vcti_lpf_t;

#define DRV_FIR_Coef_4Tap_NUM 30

typedef struct{

	signed short FIR_Coef[512];

} DRV_FIR_Coef;


#if 0
typedef enum _VIP_Two_Step_UZU_MODE
{
	Two_Step_UZU_Disable = 0,
	Two_Step_UZU_Enable,
	Two_Step_UZU_Input_120Hz,

	Two_Step_UZU_MAX,

} VIP_Two_Step_UZU_MODE;
#endif
/*#define SU_PK_COEF_ROW_MAX	5*/
/*#define SU_PK_COEF_COL_MAX	6*/

/*================================== Variables ==============================*/
/*================================ Definitions ==============================*/
#define _TabSelV8_const1 0
#define _TabSelV8_const2 1
#define _TabSelV8_coeff1 2
#define _TabSelV8_coeff2 3

#define _TabSelH12_const1 0
#define _TabSelH12_const2 1
#define _TabSelH12_coeff1 2
#define _TabSelH12_coeff2 3

/*================================== Function ===============================*/
/*============================================================================*/
/**
 * UltraZoom_ConfigScaling
 * Config scaler up/down
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
void drvif_color_ultrazoom_uzd_4k_120_mode(unsigned char x);
void drvif_color_ultrazoom_uzu_4k_120_mode(unsigned char x);
void drvif_color_scaler_setds_pk(DRV_SU_PK_Coeff *ptr);
void drvif_color_scaler_setds_supk_mask(DRV_SU_PK_Mask *ptr);

void drvif_color_ultrazoom_config_scaling_down(unsigned char panorama);
void drvif_color_ultrazoom_config_scaling_up(unsigned char panorama);
void drvif_color_scaler_set_subscaleup_hcoef(unsigned char display, DRV_FIR_Coef *ptr);
void drvif_color_scaler_set_subscaleup_vcoef(unsigned char display, DRV_FIR_Coef *ptr);
void drvif_color_scaler_set_subscaleup_coef4t256p10b(int x_coef_tab_sel, DRV_FIR_Coef *ptr);
void drvif_color_scaler_set_scaleup_hcoef(unsigned char display, DRV_FIR_Coef *ptr, char CoefIdx, char TableSel);
void drvif_color_scaler_set_scaleup_vcoef(unsigned char display, DRV_FIR_Coef *ptr, char CoefIdx, char TableSel);
void drvif_color_scaler_set_scaleup_dircoef(unsigned char display, DRV_FIR_Coef *ptr);


void drvif_color_scaler_setscaleup_directionalscaler(DRV_Dirsu_Table *ptr);
void drvif_color_scaler_setscaleup_chromablur(unsigned char display, unsigned char blur_en);
// two step
void drvif_color_twostep_scaleup_mflti(DRV_TwoStepSU_Mflti *ptr);
void drvif_color_twostep_scaleup_mflti422(int x);
void drvif_color_twostep_scaleup_dering(DRV_TwoStepSU_Dering *ptr);
void drvif_color_twostep_scaleup_hcoef_12tap(DRV_SU_table_H12tap_t *ptr);
void drvif_color_twostep_scaleup_coef_4tap(DRV_FIR_Coef *ptr_HY, DRV_FIR_Coef *ptr_VY, DRV_FIR_Coef *ptr_HC, DRV_FIR_Coef *ptr_VC);

void drvif_color_scaler_setscaleup_hcoef_12tap(unsigned char display, DRV_SU_table_H12tap_t *ptr);
void drvif_color_scaler_setscaleup_vcoef_8tap(unsigned char display, DRV_SU_table_V8tap_t *ptr);

/*void drvif_color_scaler_setds_vcti(unsigned char level);*/
void drvif_color_scaler_setds_vcti(unsigned char display, VIP_D_vcti_t *ptr);
void drvif_color_scaler_getds_vcti(unsigned char display, VIP_D_vcti_t *ptr);

void drvif_color_scaler_setds_vcti_lpf(unsigned char display, DRV_D_vcti_lpf_t *ptr);
void drvif_color_ultrazoom_set_scale_down_di_hsd(unsigned char channel, SIZE *ptInSize, SIZE *ptOutSize, unsigned char panorama);
void drvif_color_ultrazoom_set_scale_down(unsigned char channel, SIZE *ptInSize, SIZE *ptOutSize, unsigned char panorama);
void drvif_color_sharpness_unsharp_mask(DRV_Un_Shp_Mask *ptr);

/*============================================================================*/
/**
 * UltraZoom_SetScaleUpInputSize
 * Enable/disable vertical or horizontal scaling up,
 * including main and sub display. It is to set scale control register.
 * @param <info> {info structure}
 * @nWid
 * @nLen
 * @return { none }
 *
 */
void UltraZoom_SetScaleUpInputSize(unsigned char display, unsigned int nWid, unsigned int nLen);

#ifdef CONFIG_IDTV_US
void UltraZoom_ConfigScaling(void);
#endif

unsigned char drvif_color_regSuPkTable(unsigned char *ptr);
void drvif_color_ultrazoom_set_scale_down(unsigned char channel, SIZE *ptInSize, SIZE *ptOutSize, unsigned char panorama);
void drvif_color_ultrazoom_setscaleup(unsigned char display, SIZE *ptInSize, SIZE *ptOutSize, unsigned char panorama);
void drvif_color_ultrazoom_Adapt8tapV(unsigned char display, unsigned char V8tap_mode, unsigned char table_sel);
void drvif_color_ultrazoom_Adapt12tapH(unsigned char display, unsigned char H8tap_en, unsigned char hor12_table_sel);
void drvif_color_ultrazoom_H12tapDeRing(unsigned char Level);
void drvif_color_ultrazoom_V8tapDeRing(unsigned char Level);
void drvif_color_ultrazoom_de_distortion(unsigned char Level);
void drvif_color_ultrazoom_chroma8H(short Lehel);
void drvif_color_ultrazoom_chroma8V(short Level);
void drvif_color_ultrazoom_chroma8VH(short Level,short Lehel);
void zoom_drvif_color_ultrazoom_calscaleup(unsigned char display, SIZE *ptInSize, SIZE *ptOutSize, unsigned char panorama);
unsigned char drvif_color_regSuPkTable_by_timing(unsigned char *ptr);
#endif


/**** 20130628 add by Ren for scaledown control by large table ****/
unsigned char fwif_color_scaler_getscaledown_hcoef(unsigned char display);
unsigned char fwif_color_scaler_getscaledown_vcoef(unsigned char display);

unsigned char fwif_color_scaler_getscaledown_444_to_422_coef(unsigned char display);

void drvif_color_scaler_setscaledown_hcoef(unsigned char display, signed short *coef_table);
void drvif_color_scaler_setscaledown_vcoef(unsigned char display, signed short *coef_table);
/*void drvif_color_setscaledown_hcoef(unsigned char display, signed short *coef_table);*/
/*void drvif_color_setscaledown_vcoef(unsigned char display, signed short *coef_table);*/
void fwif_color_scaler_setscaledown_444_to_422_coef(unsigned char display, unsigned char factor, DRV_ScalingDown_COEF_TAB *ScaleDown_COEF_TAB);
void drvif_color_scaler_setscaledown_444_to_422_coef(unsigned char display, signed short *coef_table);

/*void drvif_color_scaler_setscaledown_hcoef_table(unsigned char display, unsigned char factor);*/
/*void drvif_color_scaler_setscaledown_vcoef_table(unsigned char display, unsigned char factor);*/
void fwif_color_scaler_setscaledown_hcoef_table(unsigned char display, unsigned char *index_table, DRV_ScalingDown_COEF_TAB *ScaleDown_COEF_TAB, _RPC_system_setting_info *RPC_system_setting_info);
void fwif_color_scaler_setscaledown_vcoef_table(unsigned char display, unsigned char *index_table, DRV_ScalingDown_COEF_TAB *ScaleDown_COEF_TAB, _RPC_system_setting_info *RPC_system_setting_info);
/* get all vip table directly*/
/*void drvif_color_regSDHTable(unsigned char *ptr);*/
/*void drvif_color_regSDVTable(unsigned char *ptr);*/
/****20130628 add by Ren for scaledown control by large table end****/

/*Elsie 20131223*/
void drvif_set_uzu_coeff(int h_factor_offset, int v_factor_offset, unsigned char h_initial, unsigned char v_initial);
void drvif_disable_user_uzu_coeff(void);

void Scaler_AdjustPathAndInfo(void);/*Will add sync from MAC2*/
void Scaler_AdjustPathAndInfo_HW_SNR(void);
unsigned char Scaler_Get_di_hsd_flag(void);/*Will add for smooth toggle flow*/

unsigned char drvif_color_get_scaleup_cuthor4line(void);
unsigned char drvif_color_get_scaleup_cutver2line(void);
void drvif_color_enable_scaleup_cuthor4line(unsigned char isEnabled);
void drvif_color_enable_scaleup_cutver2line(unsigned char isEnabled);

unsigned char drvif_color_ultrazoom_scalerup_PR_mode(unsigned int LR_width);

void drvif_color_enable_scaleup_hor_start_drop(unsigned char isEnabled);

void drvif_color_scaler_set_scaleup_coefHC(unsigned char display, DRV_FIR_Coef *ptr, char CoefIdx, char TableSel);
void drvif_color_scaler_set_scaleup_coefVC(unsigned char display, DRV_FIR_Coef *ptr, char CoefIdx, char TableSel);
void drvif_color_scaler_scaleup_table_arrange(int coef_i[4], int coef_o[4], int tab_o[4]);

void drvif_color_Set_sr_mdom_sub_en(unsigned char Enable);

void drvif_color_Set_Two_Step_UZU_Mode(VIP_Two_Step_UZU_MODE Mode);
unsigned char drvif_color_Get_Two_Step_UZU_Mode(void);
char drvif_color_Access_UZU_Bypass_No_PwrSave(unsigned char En_Flag, unsigned char isSet_Flag);
void drvif_color_scaler_setscaleup_directionalscaler_weight(unsigned char weight);
void drvif_color_scaler_setscaleup120Hz_factor_H(int x);
void drvif_color_scaler_setscaleup120Hz_factor_V(int x);
void drvif_color_scaler_setscaleup120Hz_factor_bysize(int in_hsize,int in_vsize);
void drvif_color_ultrazoom_scalerup_4K120_setiniphase(unsigned char hor_ini, unsigned char ver_ini, unsigned char hvsel);
void drvif_color_ultrazoom_nnsr(SIZE *ptInSize, SIZE *ptOutSize ,unsigned char nnsr_en);

void drvif_color_ultrazoom_scalerup_setorbit(
	int x_in_width,// width before scale up
	int x_in_height,// height before scale up
	int x_out_width,// width after scale up
	int x_out_height,// height after scale up
	int x_out_left,// cropped width after scale up
	int x_out_up,// cropped height after scale up
	int display_width,//real display width
	int display_height,//real display height
	int mode
);

/*NNSR related function*/
int drvif_color_ultrazoom_sel_nnsr_mode(SIZE *ptInSize, SIZE *ptOutSize);
void drvif_srnn_mode_change(unsigned char nnsr_mode);
/*End of NNSR related function*/
/*======================== End of File =======================================*/
/**
*
* @}
*/

