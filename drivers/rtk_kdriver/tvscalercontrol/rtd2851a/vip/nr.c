/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 *  This file is for NR related functions.
 *
 * @author  $Author$ Flora
 * @date    $Date$ 2007/07/03
 * @version     $Revision$
 * @Dragon only has NR for CH1
 */

/**
 * @addtogroup
 * @{
 */

/*============================ Module dependency  ===========================*/
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <scaler/scalerDrvCommon.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#ifndef BUILD_QUICK_SHOW
#include <tvscalercontrol/vdc/video.h>
#else
#include <sysdefs.h>
#include <no_os/slab.h>

#endif
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"


// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))
/*================================== Variables ==============================*/
unsigned int       HistThl1, HistThl2;

unsigned short      HistHorStart;
unsigned short      HistHorEnd;
unsigned short      HistVerStart;
unsigned short      HistVerEnd;

unsigned char		SpatialY_level;
unsigned char		SpatialC_level;

/*i-Egsm*/
//static unsigned int *I_Edge_Coeff;    /*20120314 EGSM table*/
extern unsigned int *i_edge_smooth_coef;

/*DCR*/
#define dcr_arry_size  128
#define bright_avg_num 1

#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_NR_DEBUG,fmt,##args)
#define NRPprintf_Info(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_NR_info_DEBUG,fmt,##args)
/*================================ Definitions ==============================*/
/*=======================*/
/*== New_RTNR_Coef[sad][edge] ==*/
/*=======================*/

/*---------- tmp ------------*/
/*
0 ->  NM_En;
1 ->  NM_Start;
2 ->  NM_Ref_Y_En;
3 ->  NM_Fw_Yref;
4 ->  NM_Fw_Ydiff;
5 ->  NM_PixMove_En;
6 ->  NM_Debug_En;
7 ->  NM_Var_Sel;
8 ->  NM_Debug_Conti;
9 ->  NM_Sampling_Bit;
10 -> NM_Top_Margin;
11 -> NM_Bottom_Margin;
12 -> NM_Left_Margin;
13 -> NM_Right_Margin;
*/
unsigned char RTNR_NM_CONTROL_TABLE[2][14] = {
{1, 1, 0, 128, 128, 1, 0, 2, 0, 0, 0, 0, 0, 0},	/* enable*/
{0, 0, 0, 128, 128, 1, 0, 2, 0, 0, 0, 0, 0, 0}		/* disable*/
};

/* level 0~5 for less noise to more noise*/
/*
0 ->  NM_Mean_Diff_Thd;
1 ->  NM_Var_Diff_Thd;
2 ->  NM_Var_High_Thd;
3 ->  NM_Var_Low_Thd;
4 ->  NM_Noise_Level_Thd0;
5 ->  NM_Noise_Level_Thd1;
6 ->  NM_Noise_Level_Thd2;
7 ->  NM_Noise_Level_Thd3;
8 ->  NM_Noise_Level_Thd4;
9 ->  NM_Noise_Level_Thd5;
10 -> NM_Noise_Level_Thd6;
11 -> NM_Noise_Level_Thd7;
12 -> NM_Noise_Level_Thd8;
13 -> NM_Noise_Level_Thd9;
14 -> NM_Noise_Level_Thd10;
15 -> NM_Noise_Level_Thd11;
16 -> NM_Noise_Level_Thd12;
17 -> NM_Noise_Level_Thd13;
18 -> NM_Noise_Level_Thd14;
*/
unsigned int RTNR_NM_THD_TABLE[NM_LEVEL][19] = {
{3, 0x3e8, 0xbb8, 0, 4, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112},			/* LEVEL 0*/
{3, 0x3e8, 0xbb8, 0, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140},		/* LEVEL 1*/
{3, 0x3e8, 0xbb8, 0, 6, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168},		/* LEVEL 2*/
{3, 0x3e8, 0xbb8, 0, 7, 14, 28, 42, 56, 70, 84, 98, 112, 126, 140, 156, 170, 184, 198},		/* LEVEL 3*/
{3, 0x3e8, 0xbb8, 0, 8, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224},		/* LEVEL 4*/
{3, 0x1e, 0x32, 0, 1, 2, 2, 3, 3, 3, 4, 5, 6, 8, 10, 12, 14, 16, 18},		/* LEVEL 5*/
};

DRV_MPEGNR_16_SW_CTRL S_MPEGNR_SW_ctrl_H={0,0,0,0,0,0,0,0,0,0};
DRV_MPEGNR_16_SW_CTRL S_MPEGNR_SW_ctrl_V={0,0,0,0,0,0,0,0,0,0};

/* RTNR K-force*/
#define K_FORCE_MAX	15

/*================================== Function ===============================*/

void drvif_color_rtnr_grid_remover_type(UINT8 channel, UINT16 type)
{
	/*not define*/
}

/**
 * CleanPicture
 * Select impulse noise reduction level:0-255
**/
void drvif_color_noisereduction_impulse(unsigned char channel , unsigned char level, unsigned char bNoiseReductionMode)
{
	signed short value;

	/*if (!drvif_color_nrrestrict()) {*/ /*if widt > h1440 disable NR then return*/
	/*	return;*/
	/*}*/
	/*middleware: 01: no nr, 11: nr*/
	if (bNoiseReductionMode == 0x0 || bNoiseReductionMode == 0x02)
		return;

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en

	if (channel == _CHANNEL1) {
		/*dch1_CP_Ctrl_RBUS dch1_CP_Ctrl_reg;*/
		/*dch1_CP_Ctrl_reg.regValue = IoReg_Read32(NR_ICH1_CP_CTRL_ADDR);*/
		/*dch1_CP_Ctrl_reg.cp_iresultweight = 0x7;*/
		/*rtdf_outl(NR_ICH1_CP_CTRL_ADDR, dch1_CP_Ctrl_reg.regValue);*/
		/*----------------------*/
		nr_dch1_cp_ctrl_RBUS dch1_CP_Ctrl_reg;
		dch1_CP_Ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
		if (level == 0)
			dch1_CP_Ctrl_reg.cp_impulseenable = 0x0;
		else
			dch1_CP_Ctrl_reg.cp_impulseenable = 0x1;/*enable impulse NR*/
		IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_CP_Ctrl_reg.regValue);

		/*----------------------*/
		{
		nr_dch1_impulse_thl_RBUS dch1_Impulse_Thl_reg;
		dch1_Impulse_Thl_reg.regValue = IoReg_Read32(NR_DCH1_Impulse_Thl_reg);
		value = 255-level;
		dch1_Impulse_Thl_reg.cp_ipixeldiffthly = value;
		dch1_Impulse_Thl_reg.cp_ipixeldiffthlc = value;
		dch1_Impulse_Thl_reg.cp_impulsethly = value;
		dch1_Impulse_Thl_reg.cp_impulsethlc = value;
		IoReg_Write32(NR_DCH1_Impulse_Thl_reg, dch1_Impulse_Thl_reg.regValue);
		}

		/*----------------------*/
		{
		nr_dch1_impulse_smooththl_RBUS dch1_Impulse_SmoothThl_reg;
		dch1_Impulse_SmoothThl_reg.regValue = IoReg_Read32(NR_DCH1_Impulse_SmoothThl_reg);
		dch1_Impulse_SmoothThl_reg.cp_impulsesmooththly = level;
		dch1_Impulse_SmoothThl_reg.cp_impulsesmooththlc = level;
		IoReg_Write32(NR_DCH1_Impulse_SmoothThl_reg, dch1_Impulse_SmoothThl_reg.regValue);
		}
	}

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); //DB apply
}


/**
 * CleanPicture
 * Select spatial noise reduction for C level:0-255
**/

void drivf_color_rtnr_correction_set(unsigned char level)
{
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;
	di_im_di_rtnr_control_RBUS im_di_rtnr_control_RBUS_reg;
	im_di_rtnr_control_RBUS_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);
	im_di_rtnr_control_RBUS_reg.cp_rtnr_y_enable = 0x1;

	if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_400_mode)
		im_di_rtnr_control_RBUS_reg.cp_rtnr_c_enable = 0x0;
	else
		im_di_rtnr_control_RBUS_reg.cp_rtnr_c_enable = 0x1;
	im_di_rtnr_control_RBUS_reg.cp_rtnr_mad_window = 0x3;/*0: 1x3 window, 1: 1x5 window, 2: 3x3 window, 3: 3x5 window*/
		/*	VIPprintf("level = %d \n", level);*/
		switch (level) {

		case 0:
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 0;
		break;

		case 1:
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 1;
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 0;
		break;

		case 2:
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 1;
		break;

		default:
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 0;
		break;
		}


		switch (level) {
		case 2:
		/*rtnr correction bit2  setting*/
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_errorthd_y = 32;
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_errorthd_c = 16;
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_mode = 0;
		break;

		default:
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_errorthd_c = 8;
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_errorthd_y = 8;
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_mode = 0;
		break;
		}

		/*WaitFor_DEN_STOP();*/
		/*WaitFor_IVS1();*/
		IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, im_di_rtnr_control_RBUS_reg.regValue);


		db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
		db_reg_ctl_reg.db_apply = 1;
		IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);

		/*WaitFor_IVS1();*/
		/*WaitFor_IVS1();*/
		/*WaitFor_DEN_STOP();*/
		/*VIPprintf("im_di_rtnr_control_RBUS_reg.regValue = %x \n", im_di_rtnr_control_RBUS_reg.regValue);*/

}


/*====================================================================================================*/
/**
 * CleanPicture
 * Select Temporal noise reduction level:0-255
**/
void drvif_color_rtnr_init(unsigned char channel)
{
	/*===== correction1 refine =======*/
	{
	di_im_di_rtnr_refined_ctrl_RBUS im_di_rtnr_refined_ctrl_reg;
	di_im_di_rtnr_refined_ctrl2_RBUS im_di_rtnr_refined_ctrl2_reg;
	im_di_rtnr_refined_ctrl_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_REFINED_CTRL_reg);
	im_di_rtnr_refined_ctrl2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_REFINED_CTRL2_reg);

	im_di_rtnr_refined_ctrl_reg.rtnr_refined_c_en = 1;
	im_di_rtnr_refined_ctrl_reg.rtnr_th_localedgediff = 8;
	im_di_rtnr_refined_ctrl_reg.rtnr_stilledge_debug = 0;
	im_di_rtnr_refined_ctrl_reg.rtnr_stilledge_bonusbias = 0x4;
	im_di_rtnr_refined_ctrl_reg.rtnr_localedge_debug = 0;
	im_di_rtnr_refined_ctrl_reg.rtnr_act_bias = 0;
	im_di_rtnr_refined_ctrl_reg.rtnr_crct_bias = 3;
	im_di_rtnr_refined_ctrl_reg.rtnr_crct_debug = 0;
	im_di_rtnr_refined_ctrl_reg.rtnr_stilledge_en = 1;
	im_di_rtnr_refined_ctrl_reg.rtnr_localedge_en = 1;
	im_di_rtnr_refined_ctrl_reg.rtnr_new_formula_en = 0;
	im_di_rtnr_refined_ctrl2_reg.rtnr_stilledge_th_edgdiff = 5;
	im_di_rtnr_refined_ctrl2_reg.rtnr_stilledge_th_edg = 0x1e;

	IoReg_Write32(DI_IM_DI_RTNR_REFINED_CTRL_reg, im_di_rtnr_refined_ctrl_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_REFINED_CTRL2_reg, im_di_rtnr_refined_ctrl2_reg.regValue);
	}

	{
	di_im_di_rtnr_mad_y_th_RBUS di_im_di_rtnr_mad_y_th_reg;
	di_im_di_rtnr_mad_c_th_RBUS di_im_di_rtnr_mad_c_th_reg;

	di_im_di_rtnr_mad_y_th_reg.regValue = scaler_rtd_inl(DI_IM_DI_RTNR_MAD_Y_TH_reg);
	di_im_di_rtnr_mad_c_th_reg.regValue = scaler_rtd_inl(DI_IM_DI_RTNR_MAD_C_TH_reg);

	di_im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th1 = 0xf;
	di_im_di_rtnr_mad_y_th_reg.cp_temporal_mad_y_th2 = 0x7ff;
	di_im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th1 = 0xf;
	di_im_di_rtnr_mad_c_th_reg.cp_temporal_mad_c_th2 = 0x7ff;

	scaler_rtd_outl(DI_IM_DI_RTNR_MAD_Y_TH_reg, di_im_di_rtnr_mad_y_th_reg.regValue);
	scaler_rtd_outl(DI_IM_DI_RTNR_MAD_C_TH_reg, di_im_di_rtnr_mad_c_th_reg.regValue);
	}
}

void drvif_color_rtnr_c_th(unsigned char channel, unsigned char *Th_C)
{
	/*im_di_rtnr_control_RBUS im_di_rtnr_control_reg;*/
	di_im_di_rtnr_c_th0_th3_RBUS im_di_rtnr_c_th0_th3_reg;
	di_im_di_rtnr_c_th4_th6_RBUS im_di_rtnr_c_th4_th6_reg;
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;


	/*im_di_rtnr_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);*/
	im_di_rtnr_c_th0_th3_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_C_TH0_TH3_reg);
	im_di_rtnr_c_th4_th6_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_C_TH4_TH6_reg);

	if (channel == _CHANNEL1) {
		if (Th_C != 0) {/*GET_RTNR_MODE(channel) flag must be turned on by menucofig*/

			im_di_rtnr_c_th0_th3_reg.cp_temporalthlc0 = Th_C[0];
			im_di_rtnr_c_th0_th3_reg.cp_temporalthlc1 = Th_C[1];
			im_di_rtnr_c_th0_th3_reg.cp_temporalthlc2 = Th_C[2];
			im_di_rtnr_c_th0_th3_reg.cp_temporalthlc3 = Th_C[3];
			im_di_rtnr_c_th4_th6_reg.cp_temporalthlc4 = Th_C[4];
			im_di_rtnr_c_th4_th6_reg.cp_temporalthlc5 = Th_C[5];
			im_di_rtnr_c_th4_th6_reg.cp_temporalthlc6 = Th_C[6];
		}

		/*IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, im_di_rtnr_control_reg.regValue);*/
		IoReg_Write32(DI_IM_DI_RTNR_C_TH0_TH3_reg, im_di_rtnr_c_th0_th3_reg.regValue);
		IoReg_Write32(DI_IM_DI_RTNR_C_TH4_TH6_reg, im_di_rtnr_c_th4_th6_reg.regValue);
	}


	db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
	db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);

}


void drvif_color_rtnr_y_th(unsigned char channel, unsigned char *Th_Y)
{

	/*im_di_rtnr_control_RBUS im_di_rtnr_control_reg;*/
	di_im_di_rtnr_y_16_th0_th3_RBUS im_di_rtnr_y_16_th0_th3_reg;
	di_im_di_rtnr_y_16_th4_th7_RBUS im_di_rtnr_y_16_th4_th7_reg;
	di_im_di_rtnr_y_16_th8_th11_RBUS im_di_rtnr_y_16_th8_th11_reg;
	di_im_di_rtnr_y_16_th12_th14_RBUS im_di_rtnr_y_16_th12_th14_reg;

	/*im_di_rtnr_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);*/
	im_di_rtnr_y_16_th0_th3_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH0_TH3_reg);
	im_di_rtnr_y_16_th4_th7_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH4_TH7_reg);
	im_di_rtnr_y_16_th8_th11_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH8_TH11_reg);
	im_di_rtnr_y_16_th12_th14_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH12_TH14_reg);

	if (channel == _CHANNEL1) {
		if (Th_Y != 0) {

			im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly0 = Th_Y[0];
			im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly1 = Th_Y[1];
			im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly2 = Th_Y[2];
			im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly3 = Th_Y[3];
			im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly4 = Th_Y[4];
			im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly5 = Th_Y[5];
			im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly6 = Th_Y[6];
			im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly7 = Th_Y[7];
			im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly8 = Th_Y[8];
			im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly9 = Th_Y[9];
			im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly10 = Th_Y[10];
			im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly11 = Th_Y[11];
			im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly12 = Th_Y[12];
			im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly13 = Th_Y[13];
			im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly14 = Th_Y[14];
		}

		/*IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, im_di_rtnr_control_reg.regValue);*/
		IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH0_TH3_reg, im_di_rtnr_y_16_th0_th3_reg.regValue);
		IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH4_TH7_reg, im_di_rtnr_y_16_th4_th7_reg.regValue);
		IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH8_TH11_reg, im_di_rtnr_y_16_th8_th11_reg.regValue);
		IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH12_TH14_reg, im_di_rtnr_y_16_th12_th14_reg.regValue);
		/*WaitFor_IEN_STOP1();*/ /*20080516 jason for double buffer*/

	}




/*-------------------------------------------------------------------------------------*/




}


void drvif_color_rtnr_y_k(unsigned char channel, unsigned int Y_K0_K7, unsigned int Y_K8_K15)
{
/*write register*/

	if (channel == _CHANNEL1) {
		IoReg_Mask32(DI_IM_DI_RTNR_Y_16_K0_K7_reg, ~(0xffffffff), (unsigned int)(Y_K0_K7));
		IoReg_Mask32(DI_IM_DI_RTNR_Y_16_K8_K15_reg, ~(0xffffffff), (unsigned int)(Y_K8_K15));
	}

}

void drvif_color_rtnr_c_k(unsigned char channel, unsigned int K_C)
{
/*write register*/
	if (channel == _CHANNEL1)
		IoReg_Mask32(DI_IM_DI_RTNR_C_K_reg, ~(0xffffffff), (unsigned int)(K_C));


}

void drvif_color_rtnr_on(unsigned char channel)
{
	di_im_di_rtnr_control_RBUS RecursiveTemporalNRCtrl_reg;
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;

	RecursiveTemporalNRCtrl_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);
	RecursiveTemporalNRCtrl_reg.cp_rtnr_y_enable = 0x1;
	if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_400_mode)
		RecursiveTemporalNRCtrl_reg.cp_rtnr_c_enable = 0x0;
	else
		RecursiveTemporalNRCtrl_reg.cp_rtnr_c_enable = 0x1;

	IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, RecursiveTemporalNRCtrl_reg.regValue);

	db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
	db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);


}

void drvif_color_rtnr_off(unsigned char channel)
{

/*Disable old rtnr:*/
	di_im_di_rtnr_control_RBUS RecursiveTemporalNRCtrl_reg;
	di_im_di_rtnr_new_control_RBUS rtnr_new_Ctrl;
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;


	/*if (channel ==_CHANNEL1)*/
	/*{*/
		RecursiveTemporalNRCtrl_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);
		RecursiveTemporalNRCtrl_reg.cp_rtnr_y_enable = 0x0;
		RecursiveTemporalNRCtrl_reg.cp_rtnr_c_enable = 0x0;
		/*RecursiveTemporalNRCtrl_reg.cp_rtnr_progressive = 0x0;*/
		IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, RecursiveTemporalNRCtrl_reg.regValue);
/*Disable New rtnr:*/

	rtnr_new_Ctrl.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_CONTROL_reg);
	// chen 170809
	rtnr_new_Ctrl.rtnr_new_method_en = 0x0;
// end chen 170809
	rtnr_new_Ctrl.prtnr_4k2k = 0x0;

	rtd_outl(DI_IM_DI_RTNR_NEW_CONTROL_reg, rtnr_new_Ctrl.regValue);

	db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
	db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);

}

void drvif_color_rtnr_progressive_check(unsigned char interlace_flag, unsigned char channel, unsigned char db_apply)
{

/*Set RTNR Progressive mode:*/
	di_im_di_rtnr_control_RBUS RecursiveTemporalNRCtrl_reg;
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;

	RecursiveTemporalNRCtrl_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);

	/*frank@0518 mark this to avoid sub channel into DI fail*/
	/*if (channel ==_CHANNEL1)*/
	{
		if (!interlace_flag)
			RecursiveTemporalNRCtrl_reg.cp_rtnr_progressive = 0x1;
		else
			RecursiveTemporalNRCtrl_reg.cp_rtnr_progressive = 0x0;

		IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, RecursiveTemporalNRCtrl_reg.regValue);
	}

	db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
	db_reg_ctl_reg.db_apply = 1;
	if (db_apply)
		IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);

}

/**
 * CleanPicture
 * Select Temporal noise reduction level:0-255
**/
void drvif_color_noisereduction_temporal(unsigned char channel, unsigned char level)
{
	di_im_di_control_RBUS di_control_reg;
	di_im_di_tnr_th_RBUS di_tnr_th_reg;
	/*VIPprintf("NR Temporal OSD level = %d\n", level);*/

	if (channel == _CHANNEL1) {
		di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);

		if (level != 0){
			di_control_reg.cp_temporalenable = 0x01;
			di_control_reg.write_enable_6 =1;

		}
		IoReg_Write32(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);
		di_tnr_th_reg.regValue = IoReg_Read32(DI_IM_DI_TNR_TH_reg);

		di_tnr_th_reg.cp_temporalthly = level;
		di_tnr_th_reg.cp_temporalthlc = level;
/*		VIPprintf("NR Temporal thl = %d\n", level);*/
	}
}

/**
 * CleanPicture
 * Set register value for noise estimation
**/

void drvif_color_noise_estimation_init(unsigned char channel,  unsigned char enable)
{
	unsigned short UZD_outH, VGIP_outH;
	unsigned short UZD_outV, VGIP_outV;
	unsigned short sta_H, sta_V;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (channel != _CHANNEL1)
		return;

	sta_H	= VIP_system_info_structure_table->IH_Start;
	sta_V	= VIP_system_info_structure_table->IV_Start;
	VGIP_outV 	= VIP_system_info_structure_table->I_Height;
	VGIP_outH 	= VIP_system_info_structure_table->I_Width;
	UZD_outV	= VIP_system_info_structure_table->Cap_Height;
	UZD_outH	= VIP_system_info_structure_table->DI_Width;
	/*========= mpeg detection registers =================*/
	/*nick187 marked for compile @ 20101223-----------------------------------------------------*/

	/*horizotal mpeg detection region*/
	{
	mpegnr_ich1_hist_blking_hor_range_RBUS ich1_hist_blking_hor_range_reg;
	mpegnr_ich1_hist_blking_ver_range_RBUS ich1_hist_blking_ver_range_reg;

	ich1_hist_blking_hor_range_reg.regValue = IoReg_Read32(MPEGNR_ICH1_Hist_Blking_Hor_Range_reg);
	//ich1_hist_blking_hor_range_reg.cp_histhorstart = 0;//k5l hw remove /*modify, 20120910, mpeg Blking Range start have to set 0.*/
	ich1_hist_blking_hor_range_reg.cp_histhorend = VGIP_outH;
	IoReg_Write32(MPEGNR_ICH1_Hist_Blking_Hor_Range_reg, ich1_hist_blking_hor_range_reg.regValue);


	ich1_hist_blking_ver_range_reg.regValue = IoReg_Read32(MPEGNR_ICH1_Hist_Blking_Ver_Range_reg);
	//ich1_hist_blking_ver_range_reg.cp_histverstart = 0;//k5l hw remove/*modify, 20120910, mpeg Blking Range start have to set 0.*/
	ich1_hist_blking_ver_range_reg.cp_histverend = VGIP_outV;
	IoReg_Write32(MPEGNR_ICH1_Hist_Blking_Ver_Range_reg, ich1_hist_blking_ver_range_reg.regValue);
	}
	/*-------------------------------------------------------------------------------------*/

	/*vertical mpeg detection region*/



	/*nick187 marked for compile @ 20101223-----------------------------------------------------*/


	/*MPEG_H&V_his_ENABLE*/
	{
	mpegnr_ich1_mpegstart_RBUS ich1_mpegstart_reg;
	mpegnr_ich1_mpegnr2_RBUS ich1_mpegnr2_reg;

	ich1_mpegstart_reg.regValue = IoReg_Read32(MPEGNR_ICH1_MPEGStart_reg);
	ich1_mpegnr2_reg.regValue = IoReg_Read32(MPEGNR_ICH1_MPEGNR2_reg);

	ich1_mpegstart_reg.cp_histhoren = 1;/*histogram hosrizotal enable*/
	ich1_mpegnr2_reg.cp_mpegenable_x = 0;/*mpegnr_x enable*/

	IoReg_Write32(MPEGNR_ICH1_MPEGStart_reg, ich1_mpegstart_reg.regValue);
	IoReg_SetBits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_en_mask); // DB en
	IoReg_Write32(MPEGNR_ICH1_MPEGNR2_reg, ich1_mpegnr2_reg.regValue);
	IoReg_SetBits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_apply_mask); // DB apply
	}

/*-------------------------------------------------------------------------------------*/


	if (UZD_outH >= 720) {/*HD*/
		HistThl1 = (UZD_outH * UZD_outV * 6/10)>>5;
		HistThl2 = (UZD_outH * UZD_outV * 8/10)>>5;
	} else {
		HistThl1 = (UZD_outH * UZD_outV * 6/10)>>3;
		HistThl2 = (UZD_outH * UZD_outV * 8/10)>>3;
	}
}

unsigned char fwif_color_get_dnr_open(void)  /* spatial nr in d-domain*/
{
	scaleup_dm_uzu_ctrl_RBUS dm_uzu_Ctrl;
	dm_uzu_Ctrl.regValue = rtd_inl(SCALEUP_DM_UZU_Ctrl_reg);
	return dm_uzu_Ctrl.bypassfornr;

}

void drvif_color_newrtnr_mask(unsigned char mask_select)
{
}

void drvif_color_nr_HMCNR(unsigned char level)
{
	/* range from -5~+5*/

	//marked by Elsie START
	//di_im_di_rtnr_hmcnr_RBUS im_di_rtnr_hmcnr_reg;
	//im_di_rtnr_hmcnr_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_HMCNR_reg);

	//if (level <= 5)/*(level >= 0 && level <= 5)  yuan1024  mark it to avoid error!*/
	//	im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_search_range = level;

	//IoReg_Write32(DI_IM_DI_RTNR_HMCNR_reg, im_di_rtnr_hmcnr_reg.regValue);
	//marked by Elsie END

// chen 170809

	//add by Elsie START
//	di_im_new_mcnr_control2_RBUS new_mcnr_control2_reg;
	//new_mcnr_control2_reg.regValue = IoReg_Read32(DI_IM_NEW_MCNR_CONTROL2_reg);

//	if (level <= 5)/*(level >= 0 && level <= 5)  yuan1024  mark it to avoid error!*/
	//	new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = level;

//	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL2_reg, new_mcnr_control2_reg.regValue);
	//add by Elsie END

// end chen 170809
}

void drvif_color_nr_HMCNR_blending(unsigned char blending_ratio)
{
	/* blending_ratio = 0: 1/4HMCNR 1:1/2HMCNR 2:3/4HMCNR 3:All HMCNR*/
	di_im_di_rtnr_hmcnr_RBUS im_di_rtnr_hmcnr_reg;

	//drvif_color_nr_HMCNR_Top_Line(10);
	drvif_color_nr_HMCNR_UV(1);

	im_di_rtnr_hmcnr_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_HMCNR_reg);

	/*if (blending_ratio >= 0 && blending_ratio <= 3)  yuan1024 marked it for avoid error*/
	if (blending_ratio <= 3)
		im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = blending_ratio;

	IoReg_Write32(DI_IM_DI_RTNR_HMCNR_reg, im_di_rtnr_hmcnr_reg.regValue);
}

// chen 170809
//#if 0
void drvif_color_nr_HMCNR_Top_Line(unsigned char level)
{
	/*VIPprintf("drvif_color_nr_HMCNR_Top_Line =%d", level);*/
	di_im_di_rtnr_hmcnr_RBUS im_di_rtnr_hmcnr_reg;
	im_di_rtnr_hmcnr_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_HMCNR_reg);

	if (level == 0) {
		im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_en = 0;
	} else {
		im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_en = 1;
		switch (level) {
		case 1:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0x10;
			break;
		case 2:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0x30;
			break;
		case 3:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0x50;
			break;
		case 4:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0x70;
			break;
		case 5:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0x90;
			break;
		case 6:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0xb0;
			break;
		case 7:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0xc0;
			break;
		case 8:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0xd0;
			break;
		case 9:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0xe0;
			break;
		case 10:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0xff;
			break;
		default:
			im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_ref_topline_th = 0xff;
			break;

		}
	}

	IoReg_Write32(DI_IM_DI_RTNR_HMCNR_reg, im_di_rtnr_hmcnr_reg.regValue);
//#endif
}
//#endif
// end chen 170809

void drvif_color_nr_HMCNR_UV(unsigned char enable)
{
	/*VIPprintf("drvif_color_nr_HMCNR_UV =%d", enable);*/
	di_im_di_rtnr_hmcnr_RBUS im_di_rtnr_hmcnr_reg;
	im_di_rtnr_hmcnr_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_HMCNR_reg);

	im_di_rtnr_hmcnr_reg.cp_temporal_hmcnr_uv_with_y = enable;

	IoReg_Write32(DI_IM_DI_RTNR_HMCNR_reg, im_di_rtnr_hmcnr_reg.regValue);
}
/*========== Elsie 20130819: Driver for setting K-force ==========*/
/* Argument "level": 0 or invalid values = disable*/
/* Argument "offset": offset of K (0 <= offset <= 7)*/
/*================================================*/
/* 20130930 sync from Magellan*/
unsigned char drvif_color_rtnr_k_force(unsigned char level, unsigned char offset)
{
	unsigned long param;
	di_im_di_rtnr_k_force_modify_RBUS im_di_rtnr_k_force_modify_reg;
	im_di_rtnr_k_force_modify_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_K_FORCE_MODIFY_reg);

	if (level == 0 || level >= K_FORCE_MAX) {	/*K-force off*/

		im_di_rtnr_k_force_modify_reg.cp_rtnr_k_force_modify_en_y = 0;
		im_di_rtnr_k_force_modify_reg.cp_rtnr_k_force_modify_mode = 0;
		im_di_rtnr_k_force_modify_reg.cp_rtnr_k_force_modify_offset = 0;
		IoReg_Write32(DI_IM_DI_RTNR_K_FORCE_MODIFY_reg, im_di_rtnr_k_force_modify_reg.regValue);
		param = 0;
	} else {
		im_di_rtnr_k_force_modify_reg.cp_rtnr_k_force_modify_offset = offset;
		im_di_rtnr_k_force_modify_reg.cp_rtnr_k_force_modify_mode = 1;
		IoReg_Write32(DI_IM_DI_RTNR_K_FORCE_MODIFY_reg, im_di_rtnr_k_force_modify_reg.regValue);
		IoReg_WriteWord0(DI_IM_DI_RTNR_K_FORCE_MODIFY_reg, 0x7fff);	/*CP_RTNR_K_force_modify_0_00~CP_RTNR_K_force_modify_0_24 all set to 1.*/
		/* We can pass level and offset once a time by shifting level 3 bits left.*/
		param = ((level << 3)|offset);
	}

	return 0;
}

static unsigned char VLPF_Coef_0[6][8] = {

/*	0  */{0, 0, 0, 0, 0, 0, 0, 0},/* register weight*/
/*	1  */{10, 0, 0, 0, 0, 0, 0, 0},/* register weight*/
/*	2  */{14, 0, 0, 0, 0, 0, 0, 0},/* register weight*/
/*	3  */{16, 0, 0, 0, 0, 0, 0, 0},/* register weight*/
/*	4  */{0, 2, 2, 40, 6, 1,	1, 1},/* adaptive weight*/
/*	5  */{0, 2, 2, 10, 6, 1,	1, 0},/* adaptive weight*/
};

static unsigned char VLPF_Coef_1[6][4] = {

/*	0  */{0, 0, 0, 0},/* register weight*/
/*	1  */{0, 0, 0, 0},/* register weight*/
/*	2  */{0, 0, 0, 0},/* register weight*/
/*	3  */{0, 0, 0, 0},/* register weight*/
/*	4  */{5, 6, 32, 0},/* adaptive weight*/
/*	5  */{5, 6, 15, 0},/* adaptive weight*/
};

static unsigned char VLPF_Coef_2[6][5] = {

/*	0  */{0, 0, 0, 0, 0},/* register weight*/
/*	1  */{0, 0, 0, 0, 10},/* register weight*/
/*	2  */{0, 0, 0, 0, 14},/* register weight*/
/*	3  */{0, 0, 0, 0, 16},/* register weight*/
/*	4  */{45,	20, 0, 1, 0},/* adaptive weight*/
/*	5  */{45,	20, 0, 1, 0},/* adaptive weight*/
};

static unsigned char VLPF_Coef_3[6][1] = {

/*	0  */{0},
/*	1  */{0},
/*	2  */{0},
/*	3  */{0},
/*	4  */{0},
/*	5  */{0},
};

void drvif_color_Mixer_ctrl_Vfilter(unsigned char channel, int select)
{
#if 0
	iedge_smooth_mixer_ctrl_RBUS mixer_ctrl_reg;
	mixer_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MIXer_ctrl_reg);

	/* 0: no filter, 1~7: 3bit blending, 8~15:all v filter*/
	if (vfilter_weight <= 8)
		mixer_ctrl_reg.vfir_weight = vfilter_weight;
	else
		mixer_ctrl_reg.vfir_weight = 0x8;

	IoReg_Write32(IEDGE_SMOOTH_MIXer_ctrl_reg, mixer_ctrl_reg.regValue);
#else
	iedge_smooth_vfilter_0_RBUS vfilter_0_reg;
	iedge_smooth_vfilter_1_RBUS vfilter_1_reg;
	iedge_smooth_vfilter_2_RBUS vfilter_2_reg;
	iedge_smooth_vfilter_3_RBUS vfilter_3_reg;

	vfilter_0_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_0_reg);
	vfilter_1_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_1_reg);
	vfilter_2_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_2_reg);
	vfilter_3_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_3_reg);

	vfilter_0_reg.vfir_weight = VLPF_Coef_0[select][0];
	vfilter_0_reg.avlpf_mindiff_th_start = VLPF_Coef_0[select][1];
	vfilter_0_reg.avlpf_mindiff_curve_rate_start = VLPF_Coef_0[select][2];
	vfilter_0_reg.avlpf_mindiff_th_finish = VLPF_Coef_0[select][3];
	vfilter_0_reg.avlpf_mindiff_curve_rate_finish = VLPF_Coef_0[select][4];
	vfilter_0_reg.avlpf_en = VLPF_Coef_0[select][5];
	vfilter_0_reg.avlpf_correct_en = VLPF_Coef_0[select][6];
	vfilter_0_reg.avlpf_bw_pattern_en = VLPF_Coef_0[select][7];

	vfilter_1_reg.avlpf_maxmindiff_th = VLPF_Coef_1[select][0];
	vfilter_1_reg.avlpf_maxmindiff_curve_rate = VLPF_Coef_1[select][1];
	vfilter_1_reg.avlpf_max_weight = VLPF_Coef_1[select][2];
	vfilter_1_reg.avlpf_min_weight = VLPF_Coef_1[select][3];

	vfilter_2_reg.avlpf_bw_th = VLPF_Coef_2[select][0];
	vfilter_2_reg.avlpf_bw_margin = VLPF_Coef_2[select][1];
	vfilter_2_reg.avlpf_bw_curve_rate = VLPF_Coef_2[select][2];
	vfilter_2_reg.avlpf_c_en = VLPF_Coef_2[select][3];
	vfilter_2_reg.vfir_c_weight = VLPF_Coef_2[select][4];

	vfilter_3_reg.vfir_debug = VLPF_Coef_3[select][0];

	#if 0
	/* 0: no filter, 1~7: 3bit blending, 8~15:all v filter*/
	if (vfilter_weight <= 8)
		vfilter_0_reg.vfir_weight = vfilter_weight;
	else
		vfilter_0_reg.vfir_weight = 0x8;
	#endif

	IoReg_Write32(IEDGE_SMOOTH_VFILTER_0_reg, vfilter_0_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_1_reg, vfilter_1_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_2_reg, vfilter_2_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_3_reg, vfilter_3_reg.regValue);
#endif
}

void drvif_color_prelpf_snr_sel(unsigned char value)
{
	iedge_smooth_prelpf_ctrl_RBUS smooth_prelpf_ctrl_reg;
	smooth_prelpf_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_prelpf_ctrl_reg);
	smooth_prelpf_ctrl_reg.prelpf_snr_sel = value;
	IoReg_Write32(IEDGE_SMOOTH_prelpf_ctrl_reg, smooth_prelpf_ctrl_reg.regValue);
}


/*Mac2 SNR , Lydia '130524*/


/*=== SNR curve mapping mode step table === '130524, Lydia*/
static unsigned short SNR_Curve_Mapping_Step_tbl[4][8] = {
		/*  step1   step2   step3   step4     step5     step6    step7   step8*/
	/*0 */{	0x8, 0x10, 0x20,  0x40,  0x60,  0x80, 0x100, 0x100},
	/*1 */{	0x4, 0xc,  0x14,  0x1c,  0x2c,  0x3c, 0x5c,  0xdc},
	/*2 */{	0x4, 0x8,  0xb,   0x13,  0x23,  0x43, 0x83,  0xc3},
	/*3 */{	0x8, 0x18, 0x28,  0x38,  0x58,  0x78, 0xb8,  0xd8},
};

/*=== SNR curve mapping mode w1(8:SRC <-->0:LPF) table === '130524, Lydia*/
static unsigned char SNR_Curve_Mapping_W1_tbl[4][9] = {
		/*  w1_0   w1_1   w1_2   w1_3   w1_4    w1_5   w1_6   w1_7   w1_8*/
	/*0 */{	 4,    8,    8,    4,    8,    8,    8,    8,    8},
	/*1 */{	 2,    2,    4,    4,    8,    8,    8,    8,    8},
	/*2 */{	 0,    4,    4,    6,    8,    8,    8,    8,    8},
	/*3 */{	 0,    8,    8,    8,    8,    8,    8,    8,    8},
};

/*=== SNR curve mapping mode w2(8:SRC+LPF <-->0:Zoran) table === '130524, Lydia*/
static unsigned char SNR_Curve_Mapping_W2_tbl[4][9] = {
		/*  w2_0   w2_1   w2_2   w2_3   w2_4    w2_5   w2_6   w2_7   w2_8*/
	/*0 */{	 8,    0,    4,    0,    0,    4,    6,    8,    8},
	/*1 */{	 8,    8,    4,    2,    0,    2,    4,    8,    8},
	/*2 */{	 8,    8,    4,    2,    8,    8,    8,    8,    8},
	/*3 */{	 4,    0,    4,    6,    8,    8,    8,    8,    8},
};


/*=== SNR curve mapping mode === '130524, Lydia*/

void drvif_color_nr_Curvemapping_En(unsigned char enable, int select)
{
	nr_curve_mapping_ctrl_RBUS curve_mapping_ctrl_reg;
	curve_mapping_ctrl_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_CTRL_reg);

	curve_mapping_ctrl_reg.curvemappingmode_en = enable;

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en
	IoReg_Write32(NR_CURVE_MAPPING_CTRL_reg, curve_mapping_ctrl_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); //DB apply
}

void drvif_color_nr_Curvemapping_step_set(int select)
{
	nr_curve_mapping_step_0_RBUS curve_mapping_step0_reg;
	nr_curve_mapping_step_1_RBUS curve_mapping_step1_reg;
	nr_curve_mapping_step_2_RBUS curve_mapping_step2_reg;
	nr_curve_mapping_step_3_RBUS curve_mapping_step3_reg;

	curve_mapping_step0_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_0_reg);
	curve_mapping_step1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_1_reg);
	curve_mapping_step2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_2_reg);
	curve_mapping_step3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_3_reg);

	curve_mapping_step0_reg.curvemapping_step1 = SNR_Curve_Mapping_Step_tbl[select][0];
	curve_mapping_step0_reg.curvemapping_step2 = SNR_Curve_Mapping_Step_tbl[select][1];
	curve_mapping_step1_reg.curvemapping_step3 = SNR_Curve_Mapping_Step_tbl[select][2];
	curve_mapping_step1_reg.curvemapping_step4 = SNR_Curve_Mapping_Step_tbl[select][3];
	curve_mapping_step2_reg.curvemapping_step5 = SNR_Curve_Mapping_Step_tbl[select][4];
	curve_mapping_step2_reg.curvemapping_step6 = SNR_Curve_Mapping_Step_tbl[select][5];
	curve_mapping_step3_reg.curvemapping_step7 = SNR_Curve_Mapping_Step_tbl[select][6];
	curve_mapping_step3_reg.curvemapping_step8 = SNR_Curve_Mapping_Step_tbl[select][7];

	IoReg_Write32(NR_CURVE_MAPPING_STEP_0_reg, curve_mapping_step0_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_STEP_1_reg, curve_mapping_step1_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_STEP_2_reg, curve_mapping_step2_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_STEP_3_reg, curve_mapping_step3_reg.regValue);

}

void drvif_color_nr_Curvemapping_weight_set(int w1_select, int w2_select)
{
	nr_curve_mapping_w1_1_RBUS curve_mapping_w1_1_reg;
	nr_curve_mapping_w1_2_RBUS curve_mapping_w1_2_reg;
	nr_curve_mapping_w1_3_RBUS curve_mapping_w1_3_reg;
	nr_curve_mapping_w2_1_RBUS curve_mapping_w2_1_reg;
	nr_curve_mapping_w2_2_RBUS curve_mapping_w2_2_reg;
	nr_curve_mapping_w2_3_RBUS curve_mapping_w2_3_reg;

	curve_mapping_w1_1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_1_reg);
	curve_mapping_w1_2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_2_reg);
	curve_mapping_w1_3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_3_reg);
	curve_mapping_w2_1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W2_1_reg);
	curve_mapping_w2_2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W2_2_reg);
	curve_mapping_w2_3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W2_3_reg);

	curve_mapping_w1_1_reg.curvemapping_w1_0 = SNR_Curve_Mapping_W1_tbl[w1_select][0];
	curve_mapping_w1_1_reg.curvemapping_w1_1 = SNR_Curve_Mapping_W1_tbl[w1_select][1];
	curve_mapping_w1_1_reg.curvemapping_w1_2 = SNR_Curve_Mapping_W1_tbl[w1_select][2];
	curve_mapping_w1_1_reg.curvemapping_w1_3 = SNR_Curve_Mapping_W1_tbl[w1_select][3];
	curve_mapping_w1_2_reg.curvemapping_w1_4 = SNR_Curve_Mapping_W1_tbl[w1_select][4];
	curve_mapping_w1_2_reg.curvemapping_w1_5 = SNR_Curve_Mapping_W1_tbl[w1_select][5];
	curve_mapping_w1_2_reg.curvemapping_w1_6 = SNR_Curve_Mapping_W1_tbl[w1_select][6];
	curve_mapping_w1_2_reg.curvemapping_w1_7 = SNR_Curve_Mapping_W1_tbl[w1_select][7];
	curve_mapping_w1_3_reg.curvemapping_w1_8 = SNR_Curve_Mapping_W1_tbl[w1_select][8];

	curve_mapping_w2_1_reg.curvemapping_w2_0 = SNR_Curve_Mapping_W2_tbl[w2_select][0];
	curve_mapping_w2_1_reg.curvemapping_w2_1 = SNR_Curve_Mapping_W2_tbl[w2_select][1];
	curve_mapping_w2_1_reg.curvemapping_w2_2 = SNR_Curve_Mapping_W2_tbl[w2_select][2];
	curve_mapping_w2_1_reg.curvemapping_w2_3 = SNR_Curve_Mapping_W2_tbl[w2_select][3];
	curve_mapping_w2_2_reg.curvemapping_w2_4 = SNR_Curve_Mapping_W2_tbl[w2_select][4];
	curve_mapping_w2_2_reg.curvemapping_w2_5 = SNR_Curve_Mapping_W2_tbl[w2_select][5];
	curve_mapping_w2_2_reg.curvemapping_w2_6 = SNR_Curve_Mapping_W2_tbl[w2_select][6];
	curve_mapping_w2_2_reg.curvemapping_w2_7 = SNR_Curve_Mapping_W2_tbl[w2_select][7];
	curve_mapping_w2_3_reg.curvemapping_w2_8 = SNR_Curve_Mapping_W2_tbl[w2_select][8];

	IoReg_Write32(NR_CURVE_MAPPING_W1_1_reg, curve_mapping_w1_1_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W1_3_reg, curve_mapping_w1_3_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W2_1_reg, curve_mapping_w2_1_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W2_2_reg, curve_mapping_w2_2_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W2_3_reg, curve_mapping_w2_3_reg.regValue);

}


void drvif_color_nr_modified_lpf_En(unsigned char enable)
{
	//marked by Elsie START
	//nr_new_lpf_nearby_ctrl_RBUS new_lpf_nearby_ctrl_reg;
	//new_lpf_nearby_ctrl_reg.regValue = IoReg_Read32(NR_NEW_LPF_NEARBY_CTRL_reg);

	//new_lpf_nearby_ctrl_reg.modified_lpf_enable = enable;

	//IoReg_Write32(NR_NEW_LPF_NEARBY_CTRL_reg, new_lpf_nearby_ctrl_reg.regValue);
	//marked by Elsie END
}

void drvif_color_nr_modified_thd_set(int thd)
{
	nr_new_lpf_nearby_ctrl_RBUS new_lpf_nearby_ctrl_reg;
	new_lpf_nearby_ctrl_reg.regValue = IoReg_Read32(NR_NEW_LPF_NEARBY_CTRL_reg);

/*	new_lpf_nearby_ctrl_reg.modified_lpf_thr = thd;*/
	IoReg_Write32(NR_NEW_LPF_NEARBY_CTRL_reg, new_lpf_nearby_ctrl_reg.regValue);
}


/* for Zoran filter adjust parameter*/
/* filter type - 0:FilterSizeY (0:11 point, 1:15 point, 2:new 15 point)*/
/* spatial distance - 1:weight2Y  2:weight3Y  3:weight4Y*/
/* Y diff - 4:SpatialThlY  5:SpatialThlYx2  6:SpatialThlYx4*/
static unsigned char SNR_Zoran_filter_set_Y_tbl[5][7] = {
		/*    0   1   2    3     4     5      6*/
	/*0 */{	0, 0, 1, 2,  5, 10, 15 },
	/*1 */{	1, 1, 2, 3, 10, 20, 30 },
	/*2 */{	1, 0, 1, 2,  5, 10, 15 },
	/*3 */{	2, 1, 0, 0, 10, 20, 30 },
	/*4 */{ 2, 1, 1, 1, 10, 20, 30 },
};
/* filter type - 0:FilterSizeC (0:11 point, 1:15 point, 2:new 15 point)*/
/* spatial distance - 1:weight2C  2:weight3C  3:weight4C*/
/* C diff - 4:SpatialThlC  5:SpatialThlCx2  6:SpatialThlCx4*/
static unsigned char SNR_Zoran_filter_set_C_tbl[5][7] = {
		/*    0   1   2    3     4     5      6*/
	/*0 */{	0, 0, 1, 2,  5, 10, 15 },
	/*1 */{	1, 1, 2, 3, 10, 20, 30 },
	/*2 */{	1, 0, 1, 2,  5, 10, 15 },
	/*3 */{	2, 1, 0, 0, 10, 20, 30 },
	/*4 */{ 2, 1, 1, 1, 10, 20, 30 },
};


void drvif_color_nr_Zoran_filter_set_Y(int select)
{

	nr_dch1_cp_ctrl_RBUS nr_dch1_cp_ctrl_RBUS_reg;
	nr_dch1_spatial_thl_RBUS nr_dch1_spatial_thl_RBUS_reg;
	nr_dch1_spatial_thl2_RBUS nr_dch1_spatial_thl2_RBUS_reg;

	nr_dch1_cp_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	nr_dch1_spatial_thl_RBUS_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl_reg);
	nr_dch1_spatial_thl2_RBUS_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl2_reg);

	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranfiltersizey = SNR_Zoran_filter_set_Y_tbl[select][0];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight2y = SNR_Zoran_filter_set_Y_tbl[select][1];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight3y = SNR_Zoran_filter_set_Y_tbl[select][2];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight4y = SNR_Zoran_filter_set_Y_tbl[select][3];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthly = SNR_Zoran_filter_set_Y_tbl[select][4];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthlyx2 = SNR_Zoran_filter_set_Y_tbl[select][5];
	nr_dch1_spatial_thl2_RBUS_reg.cp_spatialthlyx4 = SNR_Zoran_filter_set_Y_tbl[select][6];

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, nr_dch1_cp_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl_reg, nr_dch1_spatial_thl_RBUS_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl2_reg, nr_dch1_spatial_thl2_RBUS_reg.regValue);

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */

}



void drvif_color_nr_Zoran_filter_set_C(int select)
{

	nr_dch1_cp_ctrl_RBUS nr_dch1_cp_ctrl_RBUS_reg;
	nr_dch1_spatial_thl_RBUS nr_dch1_spatial_thl_RBUS_reg;
	nr_dch1_spatial_thl2_RBUS nr_dch1_spatial_thl2_RBUS_reg;

	nr_dch1_cp_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	nr_dch1_spatial_thl_RBUS_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl_reg);
	nr_dch1_spatial_thl2_RBUS_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl2_reg);

	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranfiltersizec = SNR_Zoran_filter_set_C_tbl[select][0];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight2c = SNR_Zoran_filter_set_C_tbl[select][1];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight3c = SNR_Zoran_filter_set_C_tbl[select][2];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight4c = SNR_Zoran_filter_set_C_tbl[select][3];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthlc = SNR_Zoran_filter_set_C_tbl[select][4];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthlcx2 = SNR_Zoran_filter_set_C_tbl[select][5];
	nr_dch1_spatial_thl2_RBUS_reg.cp_spatialthlcx4 = SNR_Zoran_filter_set_C_tbl[select][6];

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, nr_dch1_cp_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl_reg, nr_dch1_spatial_thl_RBUS_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl2_reg, nr_dch1_spatial_thl2_RBUS_reg.regValue);

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
}

/* for mosquito NR adjust*/
/* 0:enable   1:MNR weight   2:edgethd   3:edgestep*/
/* 4:edge strength gain   5:MNR blending mode   6:zero noiseLevel mode*/
/* 7:Bi_nLevel_max   8:Bi_diffVar_direct   9:farVar_gain*/
/* 10: LR_nLevel_shift   11:Bi_nLevel_shift*/
/* 12:neg_nLevel_offset   13:neg_nLevel_shift    14:pos_nLevel_offset    15:neg_nLevel_shift*/
static unsigned char SNR_mosquitoNR_set_tbl[5][16] = {
		/*    0     1       2     3      4    5     6      7       8     9    10   11    12    13    14     15*/
	/*0 */{	0,  9,  80,  2,  4,  1,  1, 225,  0,  0,  2,  2,  0,  2,   5,  2 }, /*disable*/
	/*1 */{	1,  9,  80,  2,  4,  1,  1, 230,  0,  0,  1,  2,  0,  2,  10,  2 }, /*weak*/
	/*2 */{	1,  9,  80,  2,  4,  1,  1, 235,  0,  0,  0,  2,  0,  2,  20,  2 },
	/*3 */{	1,  9,  80,  2,  4,  1,  1, 245,  0,  0,  5,  2,  0,  2,  30,  2 },
	/*4 */{	1,	9,	80,  2,  4,  1,  1, 255,  0,  0,  6,  2,  0,  2,  40,  2 }, /* strong*/
};

void drvif_color_nr_mosquitoNR_set(unsigned char src_idx, unsigned char value)
{
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_RBUS nr_mosquito_nr_level_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_1_RBUS nr_mosquito_nr_level_ctrl_1_RBUS_reg;

	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	nr_mosquito_nr_level_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_reg);
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg);

	nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en = SNR_mosquitoNR_set_tbl[value][0];
#ifdef CONFIG_SNR_CLOCK_HW_ISSUE
	nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en = 1;
#endif //CONFIG_SNR_CLOCK_HW_ISSUE
	nr_mosquito_ctrl_RBUS_reg.mosquitonr_weight = SNR_mosquitoNR_set_tbl[value][1];
	nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd = SNR_mosquitoNR_set_tbl[value][2];
	nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd_step = SNR_mosquitoNR_set_tbl[value][3];
	nr_mosquito_ctrl_RBUS_reg.mosquitonr_edge_strength_gain = SNR_mosquitoNR_set_tbl[value][4];
	nr_mosquito_ctrl_RBUS_reg.mosquitonr_blending_mode = SNR_mosquitoNR_set_tbl[value][5];
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.noise_level_mode = SNR_mosquitoNR_set_tbl[value][6];
/*	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_bi_nlevel_max = SNR_mosquitoNR_set_tbl[value][7];*/
/*	nr_mosquito_nr_level_ctrl_1_RBUS_reg.bi_diffvar_direct = SNR_mosquitoNR_set_tbl[value][8];*/
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_gain = SNR_mosquitoNR_set_tbl[value][9];
/*	nr_mosquito_nr_level_ctrl_1_RBUS_reg.lr_noise_level_shift = SNR_mosquitoNR_set_tbl[value][10];*/
/*	nr_mosquito_nr_level_ctrl_1_RBUS_reg.bi_noise_level_shift = SNR_mosquitoNR_set_tbl[value][11];*/
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_offset = SNR_mosquitoNR_set_tbl[value][12];
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_shift = SNR_mosquitoNR_set_tbl[value][13];
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_offset = SNR_mosquitoNR_set_tbl[value][14];
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_shift = SNR_mosquitoNR_set_tbl[value][15];

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_reg, nr_mosquito_nr_level_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg, nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); //DB apply

}

/* iEdgeSmooth*/

#if 0 //mac6p removed IESM DLTI2D
void drvif_color_DLTI(unsigned char gain)
{

	iedge_smooth_dlti2d_ctrl_RBUS edgesmooth_dlti2d_reg;
	edgesmooth_dlti2d_reg.regValue = rtd_inl(IEDGE_SMOOTH_DLTI2D_CTRL_reg);

	gain = gain > 5 ? 5 : gain;

	if (gain == 0)
		edgesmooth_dlti2d_reg.dlti2d_gain = 0;
	else {
		/*enable, gain = 1-5*/
		switch (gain) {
		case 1:
			edgesmooth_dlti2d_reg.dlti2d_gain = 1;
			break;
		case 2:
			edgesmooth_dlti2d_reg.dlti2d_gain = 2;
			break;
		case 3:
			edgesmooth_dlti2d_reg.dlti2d_gain = 4;
			break;
		case 4:
			edgesmooth_dlti2d_reg.dlti2d_gain = 8;
			break;
		case 5:
			edgesmooth_dlti2d_reg.dlti2d_gain = 16;
			break;
		default:
			break;

		}
	}

	rtd_outl(IEDGE_SMOOTH_DLTI2D_CTRL_reg, edgesmooth_dlti2d_reg.regValue);

}
#endif //mac6p removed IESM DLTI2D

/*===========================================================================*/

/* I-edge smooth 3 line/5 line mode setting*/
/* 0 :line buf  (0:3line, 1:5line(only SD))*/
/* 1:crct_lineMode  2:chaos_lineMode  3:lpf_tapMode*/

/*=== SNR curve mapping mode step table === '130524, Lydia*/
#if 0
static unsigned char iEGSM_lineMode_set_tbl[4][4] = {
		/*     0      1     2     3*/
	/*0 */{  0,  0,  0,  0}, /* all 3line*/
	/*1 */{  1,  1,  1,  1}, /* all 5 line*/
	/*2 */{  1,  0,  0,  1}, /* 3 line detection, 5 tap lpf*/
	/*3 */{  1,  1,  1,  0}, /* 5 line deteciton, 3 tap lpf*/
};
#endif

#if 0
void drvif_color_iEdgeSmooth_line_mode_set(unsigned char select)
{

	iedge_smooth_edgesmooth_ex_ctrl_RBUS edgesmooth_ex_ctrl_reg;
	edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg);

	edgesmooth_ex_ctrl_reg.lbuf_linemode = iEGSM_lineMode_set_tbl[select][0];

	IoReg_Write32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg, edgesmooth_ex_ctrl_reg.regValue);
}
#endif

void drvif_color_iEdgeSmooth_prelpf_for_detection_en(bool enable)
{
// Merlin3 no
}

void drvif_color_iEdgeSmooth_prelpf_for_blending_en(unsigned char weight)
{
// Merlin3 no
}

void drvif_color_iEdgeSmooth_Addr_Boundary_ini(unsigned int addr_sta, unsigned int addr_end)
{
	IoReg_Write32(IEDGE_SMOOTH_MVF_WRRD_BoundaryAddr1_reg, addr_sta);
	IoReg_Write32(IEDGE_SMOOTH_MVF_WRRD_BoundaryAddr2_reg, addr_end);
}

void drvif_color_snr_mode_select(unsigned char mode)
{
	switch (mode) {
	case 0:
		break;
	case 1:				/*RFCVBS*/
		drvif_color_prelpf_snr_sel(2);
/*		drvif_color_iEdgeSmooth_low_angle(0);*/
/*		drvif_color_iEdgeSmooth_angle_crct(0);*/
		break;
	case 2:				/*SD*/
		drvif_color_Mixer_ctrl_iedge(0, 8);
/*start :temp mark below two function*/
/*		drvif_color_snr_manual_weight_en(0);*/
/*		drvif_color_snr_manual_weight_set(2, 2);*/
/*end mark*/
		break;
	case 3:				/*HD*/
		break;
	default:
		break;

	}

}

void drvif_color_DRV_RTNR_General_ctrl(DRV_RTNR_General_ctrl *ptr)
{

	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;
	di_im_di_rtnr_new_control_RBUS  im_di_rtnr_new_control_reg;
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;
	im_di_rtnr_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);

	im_di_rtnr_control_reg.cp_rtnr_y_enable 		= ptr->RTNR_y_enable;
	if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_400_mode)
		im_di_rtnr_control_reg.cp_rtnr_c_enable = 0x0;
	else
		im_di_rtnr_control_reg.cp_rtnr_c_enable 		= ptr->RTNR_c_enable;

	im_di_rtnr_control_reg.cp_rtnr_mad_window 	= ptr->RTNR_mad_window;

	// chen170809
	im_di_rtnr_new_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_CONTROL_reg);
	im_di_rtnr_new_control_reg.rtnr_new_method_en 	= ptr->RTNR_new_method_en;
	im_di_rtnr_new_control_reg.rtnr_moredetail_cotinue_en = ptr->RTNR_moredetail_cotinue_en;
	im_di_rtnr_new_control_reg.rtnr_k_temporal_compare_snr = ptr->RTNR_k_temporal_compare_snr;
	im_di_rtnr_new_control_reg.rtnr_k_temporal_compare_en = ptr->RTNR_k_temporal_compare_en;
	IoReg_Write32(DI_IM_DI_RTNR_NEW_CONTROL_reg, im_di_rtnr_new_control_reg.regValue);
// end chen 170809

	IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, im_di_rtnr_control_reg.regValue);


	db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
	db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);
}

void drvif_color_DRV_RTNR_Old_Y(DRV_RTNR_Old_Y *ptr)
{
	di_im_di_rtnr_y_16_th0_th3_RBUS im_di_rtnr_y_16_th0_th3_reg;
	di_im_di_rtnr_y_16_th4_th7_RBUS im_di_rtnr_y_16_th4_th7_reg;
	di_im_di_rtnr_y_16_th8_th11_RBUS im_di_rtnr_y_16_th8_th11_reg;
	di_im_di_rtnr_y_16_th12_th14_RBUS im_di_rtnr_y_16_th12_th14_reg;
	di_im_di_rtnr_y_16_k0_k7_RBUS im_di_rtnr_y_16_k0_k7_reg;
	di_im_di_rtnr_y_16_k8_k15_RBUS im_di_rtnr_y_16_k8_k15_reg;
	im_di_rtnr_y_16_th0_th3_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH0_TH3_reg);
	im_di_rtnr_y_16_th4_th7_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH4_TH7_reg);
	im_di_rtnr_y_16_th8_th11_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH8_TH11_reg);
	im_di_rtnr_y_16_th12_th14_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH12_TH14_reg);
	im_di_rtnr_y_16_k0_k7_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_K0_K7_reg);
	im_di_rtnr_y_16_k8_k15_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_K8_K15_reg);

	im_di_rtnr_y_16_k8_k15_reg.regValue = ptr->RTNR_Y_K15_Y_K8;
	im_di_rtnr_y_16_k0_k7_reg.regValue = ptr->RTNR_Y_K7_Y_K0;
	im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly0 = ptr->RTNR_MAD_Y_TH0;
	im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly1 = ptr->RTNR_MAD_Y_TH1;
	im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly2 = ptr->RTNR_MAD_Y_TH2;
	im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly3 = ptr->RTNR_MAD_Y_TH3;
	im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly4 = ptr->RTNR_MAD_Y_TH4;
	im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly5 = ptr->RTNR_MAD_Y_TH5;
	im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly6 = ptr->RTNR_MAD_Y_TH6;
	im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly7 = ptr->RTNR_MAD_Y_TH7;
	im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly8 = ptr->RTNR_MAD_Y_TH8;
	im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly9 = ptr->RTNR_MAD_Y_TH9;
	im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly10 = ptr->RTNR_MAD_Y_TH10;
	im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly11 = ptr->RTNR_MAD_Y_TH11;
	im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly12 = ptr->RTNR_MAD_Y_TH12;
	im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly13 = ptr->RTNR_MAD_Y_TH13;
	im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly14 = ptr->RTNR_MAD_Y_TH14;

	IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH0_TH3_reg, im_di_rtnr_y_16_th0_th3_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH4_TH7_reg, im_di_rtnr_y_16_th4_th7_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH8_TH11_reg, im_di_rtnr_y_16_th8_th11_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH12_TH14_reg, im_di_rtnr_y_16_th12_th14_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_Y_16_K0_K7_reg, im_di_rtnr_y_16_k0_k7_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_Y_16_K8_K15_reg, im_di_rtnr_y_16_k8_k15_reg.regValue);
}

void drvif_color_DRV_RTNR_Dark_K(DRV_RTNR_Dark_K *ptr)
{
	di_im_di_rtnr_k_dark_RBUS im_di_rtnr_k_dark_RBUS_reg;
	im_di_rtnr_k_dark_RBUS_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_K_Dark_reg);

	im_di_rtnr_k_dark_RBUS_reg.cp_temporal_y_dark_k_en = ptr->RTNR_Dark_K_en;
	im_di_rtnr_k_dark_RBUS_reg.cp_temporal_y_dark_th = ptr->RTNR_Dark_TH;
	im_di_rtnr_k_dark_RBUS_reg.cp_temporal_y_kin_dark = ptr->RTNR_Kin;
	im_di_rtnr_k_dark_RBUS_reg.cp_temporal_y_kout_dark = ptr->RTNR_Kout;
	im_di_rtnr_k_dark_RBUS_reg.cp_temporal_y_kin_dark_2 = ptr->RTNR_Kin_2;
	im_di_rtnr_k_dark_RBUS_reg.cp_temporal_y_kout_dark_2 = ptr->RTNR_Kout_2;

	IoReg_Write32(DI_IM_DI_RTNR_K_Dark_reg, im_di_rtnr_k_dark_RBUS_reg.regValue);
}

// chen 170809
//#if 0
void drvif_color_DRV_RTNR_New_Method(DRV_RTNR_New_Method *ptr)
{
	di_im_di_rtnr_new_control_RBUS  im_di_rtnr_new_control_reg;
	di_im_di_rtnr_new_sobel_th_RBUS im_di_rtnr_new_sobel_th_reg ;
	di_im_di_rtnr_new_sad_edge_th_1_RBUS im_di_rtnr_new_sad_edge_th_1_reg;
	di_im_di_rtnr_new_sad_edge_th_2_RBUS im_di_rtnr_new_sad_edge_th_2_reg;
	di_im_di_rtnr_new_sad_nonedge_th_1_RBUS im_di_rtnr_new_sad_nonedge_th_1_reg;
	di_im_di_rtnr_new_sad_nonedge_th_2_RBUS im_di_rtnr_new_sad_nonedge_th_2_reg;
	di_im_di_rtnr_new_sad_count_th_RBUS im_di_rtnr_new_sad_count_th_reg;
	di_im_di_rtnr_new_edgetypediff_th_1_RBUS im_di_rtnr_new_edgetypediff_th_1_reg;
	di_im_di_rtnr_new_edgetypediff_th_2_RBUS im_di_rtnr_new_edgetypediff_th_2_reg;
	di_im_di_rtnr_new_k_00_RBUS im_di_rtnr_new_k_00_reg;
	di_im_di_rtnr_new_k_10_RBUS im_di_rtnr_new_k_10_reg;
	di_im_di_rtnr_new_k_20_RBUS im_di_rtnr_new_k_20_reg;
	di_im_di_rtnr_new_k_30_RBUS im_di_rtnr_new_k_30_reg;
	di_im_di_rtnr_new_k_40_RBUS im_di_rtnr_new_k_40_reg;
	di_im_di_rtnr_new_k_50_RBUS im_di_rtnr_new_k_50_reg;
	di_im_di_rtnr_new_k_60_RBUS im_di_rtnr_new_k_60_reg;
	di_im_di_rtnr_new_k_70_RBUS im_di_rtnr_new_k_70_reg;
// chen 170809
//	di_im_di_rtnr_new_sta_motion_th_0_RBUS im_di_rtnr_new_sta_motion_th_0_reg;
//	di_im_di_rtnr_new_sad_count_th_2_RBUS im_di_rtnr_new_sad_count_th_2_reg;
// end chen 170809
	im_di_rtnr_new_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_CONTROL_reg);
	im_di_rtnr_new_sobel_th_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SOBEL_TH_reg);
	im_di_rtnr_new_sad_edge_th_1_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_1_reg);
	im_di_rtnr_new_sad_edge_th_2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_2_reg);
	im_di_rtnr_new_sad_nonedge_th_1_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_1_reg);
	im_di_rtnr_new_sad_nonedge_th_2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_2_reg);
	im_di_rtnr_new_sad_count_th_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_reg);
	im_di_rtnr_new_edgetypediff_th_1_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_1_reg);
	im_di_rtnr_new_edgetypediff_th_2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_2_reg);
	im_di_rtnr_new_k_00_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_K_00_reg);
	im_di_rtnr_new_k_10_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_K_10_reg);
	im_di_rtnr_new_k_20_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_K_20_reg);
	im_di_rtnr_new_k_30_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_K_30_reg);
	im_di_rtnr_new_k_40_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_K_40_reg);
	im_di_rtnr_new_k_50_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_K_50_reg);
	im_di_rtnr_new_k_60_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_K_60_reg);
	im_di_rtnr_new_k_70_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_K_70_reg);
// chen 170809
//	im_di_rtnr_new_sta_motion_th_0_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_STA_MOTION_TH_0_reg);
	//im_di_rtnr_new_sad_count_th_2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_2_reg);
// end chen 170809

	im_di_rtnr_new_sobel_th_reg.rtnr_sbl_thm_y = ptr->RTNR_sobel_thm;
	im_di_rtnr_new_sobel_th_reg.rtnr_sbl_ths_y = ptr->RTNR_sobel_ths;
	im_di_rtnr_new_sobel_th_reg.rtnr_sobel_method = ptr->RTNR_sobel_Method;
	im_di_rtnr_new_control_reg.rtnr_sad_method = ptr->RTNR_SAD_Method;
	im_di_rtnr_new_control_reg.rtnr_sad_mix_cal_weight = ptr->RTNR_SAD_Mix_cal_weight;
// chen 170809
	//marked by Elsie START
	im_di_rtnr_new_control_reg.rtnr_statistic_motion_sad_th1 = ptr->RTNR_SAD_th1;
	im_di_rtnr_new_control_reg.rtnr_statistic_motion_sad_th2 = ptr->RTNR_SAD_th2;
	im_di_rtnr_new_sobel_th_reg.rtnr_statistic_motion_sad_th3 = ptr->RTNR_SAD_th3;
	//marked by Elsie END
	//modified by Elsie START
//	im_di_rtnr_new_sta_motion_th_0_reg.rtnr_statistic_motion_sad_th1 = ptr->RTNR_SAD_th1;
	//im_di_rtnr_new_sta_motion_th_0_reg.rtnr_statistic_motion_sad_th2 = ptr->RTNR_SAD_th2;
//	im_di_rtnr_new_sta_motion_th_0_reg.rtnr_statistic_motion_sad_th3 = ptr->RTNR_SAD_th3;
	//modified by Elsie END
// end chen 170809
	im_di_rtnr_new_k_00_reg.regValue = ptr->RTNR_K_map1;
	im_di_rtnr_new_k_10_reg.regValue = ptr->RTNR_K_map2;
	im_di_rtnr_new_k_20_reg.regValue = ptr->RTNR_K_map3;
	im_di_rtnr_new_k_30_reg.regValue = ptr->RTNR_K_map4;
	im_di_rtnr_new_k_40_reg.regValue = ptr->RTNR_K_map5;
	im_di_rtnr_new_k_50_reg.regValue = ptr->RTNR_K_map6;
	im_di_rtnr_new_k_60_reg.regValue = ptr->RTNR_K_map7;
	im_di_rtnr_new_k_70_reg.regValue = ptr->RTNR_K_map8;

	im_di_rtnr_new_edgetypediff_th_1_reg.rtnr_edgetypediff_count_th1 = ptr->RTNR_ET_Diff_th1;
	im_di_rtnr_new_edgetypediff_th_1_reg.rtnr_edgetypediff_count_th2 = ptr->RTNR_ET_Diff_th2;
	im_di_rtnr_new_edgetypediff_th_1_reg.rtnr_edgetypediff_count_th3 = ptr->RTNR_ET_Diff_th3;
	im_di_rtnr_new_edgetypediff_th_1_reg.rtnr_edgetypediff_count_th4 = ptr->RTNR_ET_Diff_th4;
	im_di_rtnr_new_edgetypediff_th_2_reg.rtnr_edgetypediff_count_th5 = ptr->RTNR_ET_Diff_th5;
	im_di_rtnr_new_edgetypediff_th_2_reg.rtnr_edgetypediff_count_th6 = ptr->RTNR_ET_Diff_th6;
	im_di_rtnr_new_edgetypediff_th_2_reg.rtnr_edgetypediff_count_th7 = ptr->RTNR_ET_Diff_th7;

	im_di_rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th1 = ptr->RTNR_SC_th1;
	im_di_rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th2 = ptr->RTNR_SC_th2;
	im_di_rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th3 = ptr->RTNR_SC_th3;
	im_di_rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th4 = ptr->RTNR_SC_th4;
	im_di_rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th5 = ptr->RTNR_SC_th5;
	im_di_rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th6 = ptr->RTNR_SC_th6;
// chen 170809
	//marked by Elsie START
	im_di_rtnr_new_sad_nonedge_th_2_reg.rtnr_statistic_motion_count_th7 = ptr->RTNR_SC_th7;
	//marked by Elsie END
	//modified by Elsie START
//	im_di_rtnr_new_sad_count_th_2_reg.rtnr_statistic_motion_count_th7 = ptr->RTNR_SC_th7;
	//modified by Elsie END
// end chen 170809
	im_di_rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th1 = ptr->RTNR_SAD_Edge_th1;
	im_di_rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th2 = ptr->RTNR_SAD_Edge_th2;
	im_di_rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th3 = ptr->RTNR_SAD_Edge_th3;
	im_di_rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th4 = ptr->RTNR_SAD_Edge_th4;
	im_di_rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th5 = ptr->RTNR_SAD_Edge_th5;
	im_di_rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th6 = ptr->RTNR_SAD_Edge_th6;
	im_di_rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th7 = ptr->RTNR_SAD_Edge_th7;

	im_di_rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th1 = ptr->RTNR_SAD_NonEdge_th1;
	im_di_rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th2 = ptr->RTNR_SAD_NonEdge_th2;
	im_di_rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th3 = ptr->RTNR_SAD_NonEdge_th3;
	im_di_rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th4 = ptr->RTNR_SAD_NonEdge_th4;
	im_di_rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th5 = ptr->RTNR_SAD_NonEdge_th5;
	im_di_rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th6 = ptr->RTNR_SAD_NonEdge_th6;
	im_di_rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th7 = ptr->RTNR_SAD_NonEdge_th7;

	IoReg_Write32(DI_IM_DI_RTNR_NEW_CONTROL_reg, im_di_rtnr_new_control_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SOBEL_TH_reg, im_di_rtnr_new_sobel_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_1_reg, im_di_rtnr_new_sad_edge_th_1_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_2_reg, im_di_rtnr_new_sad_edge_th_2_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_1_reg, im_di_rtnr_new_sad_nonedge_th_1_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_2_reg, im_di_rtnr_new_sad_nonedge_th_2_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_reg, im_di_rtnr_new_sad_count_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_1_reg, im_di_rtnr_new_edgetypediff_th_1_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_2_reg, im_di_rtnr_new_edgetypediff_th_2_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_reg, im_di_rtnr_new_sad_count_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_1_reg, im_di_rtnr_new_edgetypediff_th_1_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_2_reg, im_di_rtnr_new_edgetypediff_th_2_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_K_00_reg, im_di_rtnr_new_k_00_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_K_10_reg, im_di_rtnr_new_k_10_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_K_20_reg, im_di_rtnr_new_k_20_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_K_30_reg, im_di_rtnr_new_k_30_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_K_40_reg, im_di_rtnr_new_k_40_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_K_50_reg, im_di_rtnr_new_k_50_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_K_60_reg, im_di_rtnr_new_k_60_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_K_70_reg, im_di_rtnr_new_k_70_reg.regValue);
// chen 170809
//	IoReg_Write32(DI_IM_DI_RTNR_NEW_STA_MOTION_TH_0_reg, im_di_rtnr_new_sta_motion_th_0_reg.regValue);
//	IoReg_Write32(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_2_reg, im_di_rtnr_new_sad_count_th_2_reg.regValue);
// end chen 170809
//=======
// Merlin3 no
}

void drvif_color_DRV_RTNR_Mark(DRV_RTNR_Mask *ptr)
{

	di_im_di_rtnr_new_mask1_ctrl1_RBUS im_di_rtnr_new_mask1_ctrl1_reg;
	di_im_di_rtnr_new_mask1_ctrl2_RBUS im_di_rtnr_new_mask1_ctrl2_reg;
	di_im_di_rtnr_new_mask2_ctrl1_RBUS im_di_rtnr_new_mask2_ctrl1_reg;
	di_im_di_rtnr_new_mask2_ctrl2_RBUS im_di_rtnr_new_mask2_ctrl2_reg;
	im_di_rtnr_new_mask1_ctrl1_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_MASK1_CTRL1_reg);
	im_di_rtnr_new_mask1_ctrl2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_MASK1_CTRL2_reg);
	im_di_rtnr_new_mask2_ctrl1_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_MASK2_CTRL1_reg);
	im_di_rtnr_new_mask2_ctrl2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_MASK2_CTRL2_reg);

	im_di_rtnr_new_mask1_ctrl2_reg.rtnr_mask1_en = ptr->RTNR_Mask1_Enable;
	im_di_rtnr_new_mask1_ctrl2_reg.rtnr_mask1_dir = ptr->RTNR_Mask1_Dir;
	im_di_rtnr_new_mask1_ctrl2_reg.rtnr_mask1_mapstep = ptr->RTNR_Mask1_MapStep;
	im_di_rtnr_new_mask1_ctrl2_reg.rtnr_mask1_mapthre = ptr->RTNR_Mask1_MapThd;
	im_di_rtnr_new_mask1_ctrl2_reg.rtnr_mask1_maplimit = ptr->RTNR_Mask1_MapLimit;

	im_di_rtnr_new_mask1_ctrl1_reg.rtnr_mask1_ygain = ptr->RTNR_Mask1_Y_Gain;
	im_di_rtnr_new_mask1_ctrl1_reg.rtnr_mask1_ugain = ptr->RTNR_Mask1_U_Gain;
	im_di_rtnr_new_mask1_ctrl1_reg.rtnr_mask1_vgain = ptr->RTNR_Mask1_V_Gain;
	im_di_rtnr_new_mask1_ctrl1_reg.reg_vc_nr_mask1_yref = ptr->RTNR_Mask1_Y_Ref;
	im_di_rtnr_new_mask1_ctrl1_reg.reg_vc_nr_mask1_uref = ptr->RTNR_Mask1_U_Ref;
	im_di_rtnr_new_mask1_ctrl1_reg.reg_vc_nr_mask1_vref = ptr->RTNR_Mask1_V_Ref;

	im_di_rtnr_new_mask2_ctrl2_reg.rtnr_mask2_en = ptr->RTNR_Mask2_Enable;
	im_di_rtnr_new_mask2_ctrl2_reg.rtnr_mask2_dir = ptr->RTNR_Mask2_Dir;
	im_di_rtnr_new_mask2_ctrl2_reg.rtnr_mask2_mapstep = ptr->RTNR_Mask2_MapStep;
	im_di_rtnr_new_mask2_ctrl2_reg.rtnr_mask2_mapthre = ptr->RTNR_Mask2_MapThd;
	im_di_rtnr_new_mask2_ctrl2_reg.rtnr_mask2_maplimit = ptr->RTNR_Mask2_MapLimit;

	im_di_rtnr_new_mask2_ctrl1_reg.rtnr_mask2_ygain = ptr->RTNR_Mask2_Y_Gain;
	im_di_rtnr_new_mask2_ctrl1_reg.rtnr_mask2_ugain = ptr->RTNR_Mask2_U_Gain;
	im_di_rtnr_new_mask2_ctrl1_reg.rtnr_mask2_vgain = ptr->RTNR_Mask2_V_Gain;
	im_di_rtnr_new_mask2_ctrl1_reg.reg_vc_nr_mask2_yref = ptr->RTNR_Mask2_Y_Ref;
	im_di_rtnr_new_mask2_ctrl1_reg.reg_vc_nr_mask2_uref = ptr->RTNR_Mask2_U_Ref;
	im_di_rtnr_new_mask2_ctrl1_reg.reg_vc_nr_mask2_vref = ptr->RTNR_Mask2_V_Ref;

	IoReg_Write32(DI_IM_DI_RTNR_NEW_MASK1_CTRL1_reg, im_di_rtnr_new_mask1_ctrl1_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_MASK1_CTRL2_reg, im_di_rtnr_new_mask1_ctrl2_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_MASK2_CTRL1_reg, im_di_rtnr_new_mask2_ctrl1_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_MASK2_CTRL2_reg, im_di_rtnr_new_mask2_ctrl2_reg.regValue);

// Merlin3 no
}
//#endif
// end chen 170809


void drvif_color_DRV_RTNR_LSB(DRV_RTNR_LSB *ptr)
{
	di_im_di_lsb_rtnr_RBUS im_di_lsb_rtnr_reg;
	im_di_lsb_rtnr_reg.regValue = IoReg_Read32(DI_IM_DI_LSB_RTNR_reg);

	im_di_lsb_rtnr_reg.lsb_rtnr_en = ptr->RTNR_LSB_RTNR_En;
	im_di_lsb_rtnr_reg.lsb_rtnr_channel = ptr->RTNR_LSB_RTNR_channel;
	im_di_lsb_rtnr_reg.lsb_rtnr_bitnum = ptr->RTNR_bitnum;
	im_di_lsb_rtnr_reg.lsb_rtnr_th = ptr->RTNR_LSB_RTNR_th;
	im_di_lsb_rtnr_reg.lsb_rtnr_rounding = ptr->RTNR_LSB_RTNR_Rounding;
	im_di_lsb_rtnr_reg.lsb_rtnr_curweight = ptr->RTNR_Cur_Weight;

	IoReg_Write32(DI_IM_DI_LSB_RTNR_reg, im_di_lsb_rtnr_reg.regValue);
}
// chen 170809
/*
void drvif_color_DRV_MCNR_TMV(unsigned char block_size)
{
	di_im_new_mcnr_control3_RBUS di_im_new_mcnr_control3;
	di_im_new_mcnr_control3.regValue = IoReg_Read32(DI_IM_NEW_MCNR_CONTROL3_reg);
	di_im_new_mcnr_control3.n_mcnr_tmv_bz_select = block_size;
	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL3_reg,di_im_new_mcnr_control3.regValue);


}
*/
// end chen 170809
void drvif_color_DRV_MCNR_table(DRV_MCNR_Table *ptr)
{

	di_im_di_rtnr_hmcnr_RBUS										di_im_di_rtnr_hmcnr;
	di_im_new_mcnr_control_RBUS									di_im_new_mcnr_control;
	di_im_new_mcnr_weighting_condition_RBUS			di_im_new_mcnr_weighting_condition;
	di_im_new_mcnr_clamping_value_RBUS					di_im_new_mcnr_clamping_value;
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;


// chen 170809
//	di_im_new_mcnr_control2_RBUS di_im_new_mcnr_control2;

//	di_im_new_mcnr_control3_RBUS								di_im_new_mcnr_control3;
//	di_im_new_mcnr_mv_mad_offset_RBUS						di_im_new_mcnr_mv_mad_offset;
// end chen 170809
// chen 0709
	 di_im_new_mcnr_pan_condition_RBUS      di_im_new_mcnr_pan_condition;
// end chen 0709

// chen 0724
 	di_rtnr_output_clamp_RBUS          di_rtnr_output_clamp;
 // end chen 0724

// chen 170809
/*
 // chen 160602

	 di_im_new_mcnr_control4_RBUS								 di_im_new_mcnr_control4;
	 di_im_new_mcnr_tmv_penalty_ctrl_RBUS				 di_im_new_mcnr_tmv_penalty_ctrl;
	 di_im_new_mcnr_gmv_penalty_ctrl_RBUS				 di_im_new_mcnr_gmv_penalty_ctrl;
	 di_im_new_mcnr_penalty_clamp_ctrl_RBUS 		 di_im_new_mcnr_penalty_clamp_ctrl;
	 di_im_new_mcnr_gmv_hist_ctrl_RBUS						 di_im_new_mcnr_gmv_hist_ctrl;
	 di_im_new_mcnr_statistic_boundary_RBUS 		 di_im_new_mcnr_statistic_boundary;
	 di_im_new_mcnr_tmv_diff_th_RBUS						 di_im_new_mcnr_tmv_diff_th;
	 di_im_new_mcnr_tmv_penalty_th_RBUS 				 di_im_new_mcnr_tmv_penalty_th;
	 di_im_new_mcnr_tmv_gmv_diff_th_RBUS				 di_im_new_mcnr_tmv_gmv_diff_th;
	 di_im_new_mcnr_gmv_penalty_th_RBUS 				 di_im_new_mcnr_gmv_penalty_th;

 // end chen 160602
*/
// end chen 170809

	if (!ptr) {
		VIPprintf("\n\ndrvif_color_DRV_MCNR_table,  fatal error, table NULL!!\n");
		return;
	}
/*
	VIPprintf("\n\ndrvif_color_DRV_MCNR_table:\n");
	VIPprintf("ptr->MCNR_control.cp_temporal_hmcnr_y_en = %d\n", ptr->MCNR_control.cp_temporal_hmcnr_y_en);
	VIPprintf("ptr->MCNR_control.cp_temporal_hmcnr_uv_with_y = %d\n", ptr->MCNR_control.cp_temporal_hmcnr_uv_with_y);
	VIPprintf("ptr->MCNR_control.n_mcnr_new_en = %d\n", ptr->MCNR_control.n_mcnr_new_en);
	VIPprintf("ptr->MCNR_control.n_y_zero_mv_k_en = %d\n", ptr->MCNR_control.n_y_zero_mv_k_en);
	VIPprintf("ptr->MCNR_control.n_c_zero_mv_k_en = %d\n", ptr->MCNR_control.n_c_zero_mv_k_en);
	VIPprintf("ptr->MCNR_control.cp_temporal_hmcnr_search_range = %d\n", ptr->MCNR_control.cp_temporal_hmcnr_search_range);
	VIPprintf("ptr->MCNR_control.hori_boundary_th = %d\n", ptr->MCNR_control.hori_boundary_th);
	VIPprintf("ptr->MCNR_control.cp_temporal_hmcnr_mad_window = %d\n", ptr->MCNR_control.cp_temporal_hmcnr_mad_window);
	VIPprintf("ptr->MCNR_control.n_mcnr_v_search_range = %d\n", ptr->MCNR_control.n_mcnr_v_search_range);
	VIPprintf("ptr->MCNR_control.n_mcnr_y_boundary_th = %d\n", ptr->MCNR_control.n_mcnr_y_boundary_th);
	VIPprintf("ptr->MCNR_control.n_mcnr_messlevel_th = %d\n", ptr->MCNR_control.n_mcnr_messlevel_th);
	VIPprintf("ptr->MCNR_control.n_mcnr_old_search_range_en = %d\n", ptr->MCNR_control.n_mcnr_old_search_range_en);
	*/
	// chen 170522
//	VIPprintf("ptr->MCNR_control.n_mcnr_offset = %d\n", ptr->MCNR_control.n_mcnr_offset);
	//VIPprintf("ptr->MCNR_control.n_mcnr_smp_en = %d\n", ptr->MCNR_control.n_mcnr_smp_en);
  // end chen 170522
	di_im_di_rtnr_hmcnr.regValue												= IoReg_Read32(DI_IM_DI_RTNR_HMCNR_reg);
	di_im_new_mcnr_control.regValue					 						= IoReg_Read32(DI_IM_NEW_MCNR_CONTROL_reg);
	di_im_new_mcnr_weighting_condition.regValue				 	= IoReg_Read32(DI_IM_NEW_MCNR_WEIGHTING_CONDITION_reg);
	di_im_new_mcnr_clamping_value.regValue					 		= IoReg_Read32(DI_IM_NEW_MCNR_CLAMPING_VALUE_reg);

// chen 170809
//	di_im_new_mcnr_control2.regValue					 						= IoReg_Read32(DI_IM_NEW_MCNR_CONTROL2_reg);

//	di_im_new_mcnr_control3.regValue					 					= IoReg_Read32(DI_IM_NEW_MCNR_CONTROL3_reg);
//	di_im_new_mcnr_mv_mad_offset.regValue					 			= IoReg_Read32(DI_IM_NEW_MCNR_MV_MAD_Offset_reg);
// end chen 170809
// chen 0709
	 di_im_new_mcnr_pan_condition.regValue         = IoReg_Read32(DI_IM_NEW_MCNR_PAN_CONDITION_reg);
// end chen 0709

// chen 0724
 	di_rtnr_output_clamp.regValue            = IoReg_Read32(DI_RTNR_OUTPUT_CLAMP_reg);
 // end chen 0724

// chen 170809
/*
 // chen 160602
 di_im_new_mcnr_control3.regValue												 = IoReg_Read32(DI_IM_NEW_MCNR_CONTROL3_reg);
 di_im_new_mcnr_control4.regValue												 = IoReg_Read32(DI_IM_NEW_MCNR_CONTROL4_reg);
 di_im_new_mcnr_clamping_value.regValue 								 = IoReg_Read32(DI_IM_NEW_MCNR_CLAMPING_VALUE_reg);
 di_im_new_mcnr_tmv_penalty_ctrl.regValue								 = IoReg_Read32(DI_IM_NEW_MCNR_TMV_Penalty_Ctrl_reg);
 di_im_new_mcnr_gmv_penalty_ctrl.regValue								 = IoReg_Read32(DI_IM_NEW_MCNR_GMV_Penalty_Ctrl_reg);
 di_im_new_mcnr_penalty_clamp_ctrl.regValue 						 = IoReg_Read32(DI_IM_NEW_MCNR_Penalty_Clamp_Ctrl_reg);
 di_im_new_mcnr_gmv_hist_ctrl.regValue										 = IoReg_Read32(DI_IM_NEW_MCNR_GMV_Hist_Ctrl_reg);
 di_im_new_mcnr_statistic_boundary.regValue 						 = IoReg_Read32(DI_IM_NEW_MCNR_Statistic_boundary_reg);
 di_im_new_mcnr_tmv_diff_th.regValue										 = IoReg_Read32(DI_IM_NEW_MCNR_TMV_Diff_TH_reg);
 di_im_new_mcnr_tmv_penalty_th.regValue 								 = IoReg_Read32(DI_IM_NEW_MCNR_TMV_Penalty_TH_reg);
 di_im_new_mcnr_tmv_gmv_diff_th.regValue								 = IoReg_Read32(DI_IM_NEW_MCNR_TMV_GMV_diff_TH_reg);
 di_im_new_mcnr_gmv_penalty_th.regValue 								 = IoReg_Read32(DI_IM_NEW_MCNR_GMV_Penalty_TH_reg);
 // end chen 160602
*/
// end chen 170809


	di_im_di_rtnr_hmcnr.cp_temporal_hmcnr_y_en							  	= ptr->MCNR_control.cp_temporal_hmcnr_y_en;
	di_im_di_rtnr_hmcnr.cp_temporal_hmcnr_uv_with_y				  		= ptr->MCNR_control.cp_temporal_hmcnr_uv_with_y;
	di_im_new_mcnr_control.n_mcnr_new_en							          = ptr->MCNR_control.n_mcnr_new_en;
	di_im_new_mcnr_control.n_y_zero_mv_k_en								      = ptr->MCNR_control.n_y_zero_mv_k_en;
	di_im_new_mcnr_control.n_c_zero_mv_k_en							        = ptr->MCNR_control.n_c_zero_mv_k_en;
// chen 170809
	di_im_di_rtnr_hmcnr.cp_temporal_hmcnr_search_range					= ptr->MCNR_control.cp_temporal_hmcnr_search_range;
// end chen 170809
	di_im_new_mcnr_control.hori_boundary_th							        = ptr->MCNR_control.hori_boundary_th;
	di_im_di_rtnr_hmcnr.cp_temporal_hmcnr_mad_window						= ptr->MCNR_control.cp_temporal_hmcnr_mad_window;

	di_im_new_mcnr_control.n_mcnr_offset			    = ptr->MCNR_control.n_mcnr_offset;

// chen 170809
/*
//	di_im_new_mcnr_control2.n_mcnr_v_search_range								= ptr->MCNR_control.n_mcnr_v_search_range;
	di_im_new_mcnr_control2.n_mcnr_y_boundary_th								= ptr->MCNR_control.n_mcnr_y_boundary_th;
	di_im_new_mcnr_control3.n_mcnr_messlevel_th 								= ptr->MCNR_control.n_mcnr_messlevel_th;
	di_im_new_mcnr_control.n_mcnr_old_search_range_en 					= ptr->MCNR_control.n_mcnr_old_search_range_en;
// chen 170522
//	di_im_new_mcnr_control.n_mcnr_offset                        = ptr->MCNR_control.n_mcnr_offset;
//	di_im_new_mcnr_control.n_mcnr_smp_en							 					= ptr->MCNR_control.n_mcnr_smp_en;
// end chen 170522
*/
// end chen 170809

// chen 170809
/*
// chen 160602
	di_im_new_mcnr_control4.n_mcnr_v_search_range								= ptr->MCNR_control.n_mcnr_v_search_range;
// end chen 160602
*/
// end chen 170809

// chen 0709
	 di_im_new_mcnr_control.n_h_pan_zeromv_penalty_en      =ptr->MCNR_control.n_h_pan_zeromv_penalty_en;
	 di_im_new_mcnr_pan_condition.h_pan_mv_penalty        =ptr->MCNR_control.h_pan_mv_penalty;
// end chen 0709

// chen 0724
	 di_rtnr_output_clamp.rtnr_output_clamp_en          =ptr->MCNR_control.rtnr_output_clamp_en;
	 di_rtnr_output_clamp.rtnr_output_clamp_luma         =ptr->MCNR_control.rtnr_output_clamp_luma;
	 di_rtnr_output_clamp.rtnr_output_clamp_chroma        =ptr->MCNR_control.rtnr_output_clamp_chroma;
 // end chen 0724


	di_im_new_mcnr_control.n_mv_avg_en				                  = ptr->MCNR_Mv_Avg.n_mv_avg_en;
	di_im_new_mcnr_control.n_mv_avg_mode_select				          = ptr->MCNR_Mv_Avg.n_mv_avg_mode_select;
	di_im_new_mcnr_clamping_value.avg_mv_clamp_value			  		= ptr->MCNR_Mv_Avg.avg_mv_clamp_value;


	di_im_new_mcnr_control.n_zero_mv_gain8_en                 	= ptr->MCNR_Weighting.n_zero_mv_gain8_en;
	di_im_new_mcnr_control.n_mv_diff_distance_select          	= ptr->MCNR_Weighting.n_mv_diff_distance_select;
	di_im_new_mcnr_control.n_mcnr_weight_avg_en	              	= ptr->MCNR_Weighting.n_mcnr_weight_avg_en;
	di_im_new_mcnr_control.n_mcnr_weight_clamp_en             	= ptr->MCNR_Weighting.n_mcnr_weight_clamp_en;
	di_im_new_mcnr_weighting_condition.mv_diff_th               = ptr->MCNR_Weighting.mv_diff_th;
	di_im_new_mcnr_weighting_condition.mv_diff_slope	          = ptr->MCNR_Weighting.mv_diff_slope;
	di_im_new_mcnr_weighting_condition.sad_min_th               = ptr->MCNR_Weighting.sad_min_th;
	di_im_new_mcnr_weighting_condition.sad_min_slope            = ptr->MCNR_Weighting.sad_min_slope;
	di_im_new_mcnr_clamping_value.mcnr_weight_clamp_loth	  		= ptr->MCNR_Weighting.mcnr_weight_clamp_loth;
	di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth				= ptr->MCNR_Weighting.mcnr_weight_clamp_upth;

// chen 170809
//	di_im_new_mcnr_control.n_mcnr_mv_diff_max_en								= ptr->MCNR_Weighting.n_mcnr_mv_diff_max_en;
// end chen 170809

	di_im_new_mcnr_control.n_rtnr_y_k_select            				= ptr->MCNR_Output_K_select.n_rtnr_y_k_select;
	di_im_new_mcnr_control.n_rtnr_c_k_select            				= ptr->MCNR_Output_K_select.n_rtnr_c_k_select;
	di_im_new_mcnr_control.n_mcnr_y_k_select_mc	        				= ptr->MCNR_Output_K_select.n_mcnr_y_k_select_mc;

// chen 170809
/*

// chen 170522
//	di_im_new_mcnr_control.n_mcnr_mad_select										= ptr->MCNR_MAD_control.n_mcnr_mad_select;
//	di_im_new_mcnr_control2.n_mcnr_mv_penalty										= ptr->MCNR_MAD_control.n_mcnr_mv_penalty;
	//di_im_new_mcnr_control4.n_mcnr_mad_hist_pos 							= ptr->MCNR_Control2.n_mcnr_mad_hist_pos;
// end chen 170522
	di_im_new_mcnr_control2.n_mcnr_mad_min_th										= ptr->MCNR_MAD_control.n_mcnr_mad_min_th;
	di_im_new_mcnr_control2.n_mcnr_mad_min_uv_th								= ptr->MCNR_MAD_control.n_mcnr_mad_min_uv_th;
	di_im_new_mcnr_mv_mad_offset.n_mcnr_mad_offset							= ptr->MCNR_MAD_control.n_mcnr_mad_offset;
	di_im_new_mcnr_mv_mad_offset.n_mcnr_mad_uv_offset						= ptr->MCNR_MAD_control.n_mcnr_mad_uv_offset;
	di_im_new_mcnr_control3.n_mcnr_mv_mad_blending_en						= ptr->MCNR_MAD_control.n_mcnr_mv_mad_blending_en;
	di_im_new_mcnr_control3.n_mcnr_mv_mad_blending_vertical_en	= ptr->MCNR_MAD_control.n_mcnr_mv_mad_blending_vertical_en;
	di_im_new_mcnr_control3.n_mcnr_mv_mad_mv_lo_th							= ptr->MCNR_MAD_control.n_mcnr_mv_mad_mv_lo_th;
	di_im_new_mcnr_control3.n_mcnr_mv_mad_mv_gap								= ptr->MCNR_MAD_control.n_mcnr_mv_mad_mv_gap;

	di_im_new_mcnr_control3.n_mcnr_k_factor_lpf_en							= ptr->MCNR_K_factor_control.n_mcnr_k_factor_lpf_en;
	di_im_new_mcnr_control3.n_mcnr_kfactor_lpf_large_select			= ptr->MCNR_K_factor_control.n_mcnr_kfactor_lpf_large_select;
	di_im_new_mcnr_control3.n_mcnr_kfactor_lpf_method						= ptr->MCNR_K_factor_control.n_mcnr_kfactor_lpf_method;

*/
// end chen 170809

// chen 170809
/*
	// chen 160602
	di_im_new_mcnr_control3.n_mcnr_mad_mv_nolimit_en					= ptr->MCNR_Control2.n_mcnr_mad_mv_nolimit_en;
	//di_im_new_mcnr_control3.n_mcnr_only_h_c_en								= ptr->MCNR_Control2.n_mcnr_only_h_c_en;
	//di_im_new_mcnr_control3.n_mcnr_tmv_bz_select							= ptr->MCNR_Control2.n_mcnr_tmv_bz_select;
	di_im_new_mcnr_control4.n_mcnr_mad_hist_clip							= ptr->MCNR_Control2.n_mcnr_mad_hist_clip;
	di_im_new_mcnr_control4.n_mcnr_messlevel_shift						= ptr->MCNR_Control2.n_mcnr_messlevel_shift;
	//di_im_new_mcnr_control4.tmv_no_reset_flag 								= ptr->MCNR_Control2.tmv_no_reset_flag;

	di_im_new_mcnr_clamping_value.n_mcnr_weight_avg_mode			= ptr->MCNR_Blending.n_mcnr_weight_avg_mode;
	di_im_new_mcnr_clamping_value.n_mcnr_weight_mode					= ptr->MCNR_Blending.n_mcnr_weight_mode;
	di_im_new_mcnr_clamping_value.n_mcnr_weight1							= ptr->MCNR_Blending.n_mcnr_weight1;
	di_im_new_mcnr_clamping_value.n_mcnr_weight2							= ptr->MCNR_Blending.n_mcnr_weight2;
	di_im_new_mcnr_clamping_value.n_mcnr_weight3							= ptr->MCNR_Blending.n_mcnr_weight3;

	di_im_new_mcnr_control4.n_mcnr_k_limit_en									= ptr->MCNR_K_Limit.n_mcnr_k_limit_en;
	di_im_new_mcnr_control4.n_mcnr_k_limit_lo									= ptr->MCNR_K_Limit.n_mcnr_k_limit_lo;
	di_im_new_mcnr_control4.n_mcnr_k_limit_hi									= ptr->MCNR_K_Limit.n_mcnr_k_limit_hi;
	di_im_new_mcnr_control4.n_mcnr_k_limit_mv_th							= ptr->MCNR_K_Limit.n_mcnr_k_limit_mv_th;

	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv_penalty_clamp_en = ptr->MCNR_Penalty_Gain_Clamp.n_mcnr_tmv_penalty_clamp_en;
	di_im_new_mcnr_gmv_penalty_ctrl.n_mcnr_pan_penalty_clamp_en = ptr->MCNR_Penalty_Gain_Clamp.n_mcnr_pan_penalty_clamp_en;
	di_im_new_mcnr_penalty_clamp_ctrl.n_mcnr_penalty_clamp_hi		= ptr->MCNR_Penalty_Gain_Clamp.n_mcnr_penalty_clamp_hi;
	di_im_new_mcnr_penalty_clamp_ctrl.n_mcnr_penalty_clamp_lo		= ptr->MCNR_Penalty_Gain_Clamp.n_mcnr_penalty_clamp_lo;
	di_im_new_mcnr_penalty_clamp_ctrl.n_mcnr_messlevel_hi_th		= ptr->MCNR_Penalty_Gain_Clamp.n_mcnr_messlevel_hi_th;
	di_im_new_mcnr_penalty_clamp_ctrl.n_mcnr_messlevel_lo_th		= ptr->MCNR_Penalty_Gain_Clamp.n_mcnr_messlevel_lo_th;
	di_im_new_mcnr_penalty_clamp_ctrl.n_mcnr_penalty_clamp_slope= ptr->MCNR_Penalty_Gain_Clamp.n_mcnr_penalty_clamp_slope;

	di_im_new_mcnr_gmv_hist_ctrl.n_mcnr_mv_hist_shift						= ptr->MCNR_Info_Stat.n_mcnr_mv_hist_shift;
	di_im_new_mcnr_gmv_hist_ctrl.n_mcnr_hmv_th1 								= ptr->MCNR_Info_Stat.n_mcnr_hmv_th1;
	di_im_new_mcnr_gmv_hist_ctrl.n_mcnr_hmv_th2 								= ptr->MCNR_Info_Stat.n_mcnr_hmv_th2;
	di_im_new_mcnr_gmv_hist_ctrl.n_mcnr_vmv_th1 								= ptr->MCNR_Info_Stat.n_mcnr_vmv_th1;
	di_im_new_mcnr_gmv_hist_ctrl.n_mcnr_vmv_th2 								= ptr->MCNR_Info_Stat.n_mcnr_vmv_th2;
	di_im_new_mcnr_statistic_boundary.n_mcnr_x_boundary_th2 		= ptr->MCNR_Info_Stat.n_mcnr_x_boundary_th2;
	di_im_new_mcnr_statistic_boundary.n_mcnr_y_boundary_th2 		= ptr->MCNR_Info_Stat.n_mcnr_y_boundary_th2;
	di_im_new_mcnr_control3.n_mcnr_mad_stat_pos_select					= ptr->MCNR_Info_Stat.n_mcnr_mad_stat_pos_select;
	di_im_new_mcnr_control3.n_mcnr_still_stat_disable						= ptr->MCNR_Info_Stat.n_mcnr_still_stat_disable;
	di_im_new_mcnr_control3.n_mcnr_mad_hist_shift_en						= ptr->MCNR_Info_Stat.n_mcnr_mad_hist_shift_en;

	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv0_diff_th1				= ptr->MCNR_TMV_Candidate.n_mcnr_tmv0_diff_th1;
	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv0_diff_th2				= ptr->MCNR_TMV_Candidate.n_mcnr_tmv0_diff_th2;
	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv0_diff_th3				= ptr->MCNR_TMV_Candidate.n_mcnr_tmv0_diff_th3;
	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv_diff_range1			= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_diff_range1;
	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv_diff_range2			= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_diff_range2;
	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv_diff_range3			= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_diff_range3;
	// chen 170522
//	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv_cand_select			= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_cand_select;
//	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv_penalty_select		= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_penalty_select;
// end chen 170522
	di_im_new_mcnr_tmv_diff_th.n_mcnr_diff_tmv_th1							= ptr->MCNR_TMV_Candidate.n_mcnr_diff_tmv_th1;
	di_im_new_mcnr_tmv_diff_th.n_mcnr_diff_tmv_th2							= ptr->MCNR_TMV_Candidate.n_mcnr_diff_tmv_th2;
	di_im_new_mcnr_tmv_diff_th.n_mcnr_tmv_diff_th1							= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_diff_th1;
	di_im_new_mcnr_tmv_diff_th.n_mcnr_tmv_diff_th2							= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_diff_th2;
	di_im_new_mcnr_tmv_diff_th.n_mcnr_tmv_diff_th3							= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_diff_th3;
	di_im_new_mcnr_tmv_diff_th.n_mcnr_tmv_diff_th4							= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_diff_th4;
	di_im_new_mcnr_tmv_diff_th.n_mcnr_tmv_diff_th5							= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_diff_th5;
	di_im_new_mcnr_tmv_diff_th.n_mcnr_tmv_diff_th6							= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_diff_th6;
	di_im_new_mcnr_tmv_penalty_th.n_mcnr_tmv_penalty1						= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_penalty1;
	di_im_new_mcnr_tmv_penalty_th.n_mcnr_tmv_penalty2						= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_penalty2;
	di_im_new_mcnr_tmv_penalty_th.n_mcnr_tmv_penalty3						= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_penalty3;
	di_im_new_mcnr_tmv_penalty_th.n_mcnr_tmv_penalty4						= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_penalty4;
	di_im_new_mcnr_tmv_penalty_th.n_mcnr_tmv_penalty5						= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_penalty5;
	di_im_new_mcnr_tmv_penalty_th.n_mcnr_tmv_penalty6						= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_penalty6;
	di_im_new_mcnr_tmv_penalty_th.n_mcnr_tmv_penalty7						= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_penalty7;
	// chen 170522
//	di_im_new_mcnr_tmv_penalty_ctrl.n_mcnr_tmv_penalty_add2 		= ptr->MCNR_TMV_Candidate.n_mcnr_tmv_penalty_add2;
//	di_im_new_mcnr_gmv_penalty_ctrl.n_mcnr_pan_diff_range3			= ptr->MCNR_GMV_Candidate.n_mcnr_pan_diff_range3;
//	di_im_new_mcnr_gmv_penalty_ctrl.n_mcnr_pan_penalty_th2			= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty_th2;
//	di_im_new_mcnr_gmv_penalty_ctrl.n_mcnr_pan_half_penalty_en	= ptr->MCNR_GMV_Candidate.n_mcnr_pan_half_penalty_en;
// end chen 170522
	di_im_new_mcnr_gmv_penalty_ctrl.n_mcnr_pan_diff_range1			= ptr->MCNR_GMV_Candidate.n_mcnr_pan_diff_range1;
	di_im_new_mcnr_gmv_penalty_ctrl.n_mcnr_pan_diff_range2			= ptr->MCNR_GMV_Candidate.n_mcnr_pan_diff_range2;
	di_im_new_mcnr_gmv_penalty_ctrl.n_mcnr_pan_penalty_select		= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty_select;
	di_im_new_mcnr_tmv_gmv_diff_th.n_mcnr_diff_tmv_pan_th1			= ptr->MCNR_GMV_Candidate.n_mcnr_diff_tmv_pan_th1;
	di_im_new_mcnr_tmv_gmv_diff_th.n_mcnr_diff_tmv_pan_th2			= ptr->MCNR_GMV_Candidate.n_mcnr_diff_tmv_pan_th2;
	di_im_new_mcnr_gmv_penalty_ctrl.n_mcnr_pan_diff_range1_v		= ptr->MCNR_GMV_Candidate.n_mcnr_pan_diff_range1_v;
	di_im_new_mcnr_tmv_gmv_diff_th.n_mcnr_tmv_pan_diff_th1			= ptr->MCNR_GMV_Candidate.n_mcnr_tmv_pan_diff_th1;
	di_im_new_mcnr_tmv_gmv_diff_th.n_mcnr_tmv_pan_diff_th2			= ptr->MCNR_GMV_Candidate.n_mcnr_tmv_pan_diff_th2;
	di_im_new_mcnr_tmv_gmv_diff_th.n_mcnr_tmv_pan_diff_th3			= ptr->MCNR_GMV_Candidate.n_mcnr_tmv_pan_diff_th3;
	di_im_new_mcnr_tmv_gmv_diff_th.n_mcnr_tmv_pan_diff_th4			= ptr->MCNR_GMV_Candidate.n_mcnr_tmv_pan_diff_th4;
	di_im_new_mcnr_tmv_gmv_diff_th.n_mcnr_tmv_pan_diff_th5			= ptr->MCNR_GMV_Candidate.n_mcnr_tmv_pan_diff_th5;
	di_im_new_mcnr_tmv_gmv_diff_th.n_mcnr_tmv_pan_diff_th6			= ptr->MCNR_GMV_Candidate.n_mcnr_tmv_pan_diff_th6;
	di_im_new_mcnr_gmv_penalty_th.n_mcnr_pan_penalty1						= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty1;
	di_im_new_mcnr_gmv_penalty_th.n_mcnr_pan_penalty2						= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty2;
	di_im_new_mcnr_gmv_penalty_th.n_mcnr_pan_penalty3						= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty3;
	di_im_new_mcnr_gmv_penalty_th.n_mcnr_pan_penalty4						= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty4;
	di_im_new_mcnr_gmv_penalty_th.n_mcnr_pan_penalty5						= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty5;
	di_im_new_mcnr_gmv_penalty_th.n_mcnr_pan_penalty6						= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty6;
	di_im_new_mcnr_gmv_penalty_th.n_mcnr_pan_penalty7						= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty7;
	di_im_new_mcnr_gmv_penalty_ctrl.n_mcnr_pan_penalty_th1			= ptr->MCNR_GMV_Candidate.n_mcnr_pan_penalty_th1;
	// end chen 160602
*/
// end chen 170809

	IoReg_Write32(DI_IM_DI_RTNR_HMCNR_reg, 								di_im_di_rtnr_hmcnr.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL_reg, 						di_im_new_mcnr_control.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_WEIGHTING_CONDITION_reg, di_im_new_mcnr_weighting_condition.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_CLAMPING_VALUE_reg, 			di_im_new_mcnr_clamping_value.regValue);

// chen 170809
//	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL2_reg, 						di_im_new_mcnr_control2.regValue);

//	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL3_reg, 						di_im_new_mcnr_control3.regValue);
//	IoReg_Write32(DI_IM_NEW_MCNR_MV_MAD_Offset_reg, 			di_im_new_mcnr_mv_mad_offset.regValue);
// end chen 170809


// chen 0709
	IoReg_Write32(DI_IM_NEW_MCNR_PAN_CONDITION_reg,    di_im_new_mcnr_pan_condition.regValue);
// end chen 0709


// chen 0724
	IoReg_Write32(DI_RTNR_OUTPUT_CLAMP_reg,        di_rtnr_output_clamp.regValue);
// end chen 0724

// chen 170809
/*
// chen 160602
	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL3_reg,			di_im_new_mcnr_control3.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL4_reg,			di_im_new_mcnr_control4.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_CLAMPING_VALUE_reg,		di_im_new_mcnr_clamping_value.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_TMV_Penalty_Ctrl_reg,		di_im_new_mcnr_tmv_penalty_ctrl.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_GMV_Penalty_Ctrl_reg,		di_im_new_mcnr_gmv_penalty_ctrl.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_Penalty_Clamp_Ctrl_reg,		di_im_new_mcnr_penalty_clamp_ctrl.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_GMV_Hist_Ctrl_reg,		di_im_new_mcnr_gmv_hist_ctrl.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_Statistic_boundary_reg,		di_im_new_mcnr_statistic_boundary.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_TMV_Diff_TH_reg, 		di_im_new_mcnr_tmv_diff_th.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_TMV_Penalty_TH_reg,		di_im_new_mcnr_tmv_penalty_th.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_TMV_GMV_diff_TH_reg, 		di_im_new_mcnr_tmv_gmv_diff_th.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_GMV_Penalty_TH_reg,		di_im_new_mcnr_gmv_penalty_th.regValue);
// end chen 160602
*/
// end chen 170809



	db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
	db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);

}

#if 0
void drvif_color_DRV_RTNR_SNR(DRV_RTNR_SNR *ptr)
{
/* mac3 remove new rtnr snr function.*/
/*
	di_im_di_rtnr_new_snr_control_RBUS im_di_rtnr_new_snr_control_reg;
	di_im_di_rtnr_new_snr_th_1_RBUS im_di_rtnr_new_snr_th1_reg;
	di_im_di_rtnr_new_snr_th2_RBUS im_di_rtnr_new_snr_th2_reg;
	im_di_rtnr_new_snr_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SNR_CONTROL_VADDR);
	im_di_rtnr_new_snr_th1_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SNR_TH_1_VADDR);
	im_di_rtnr_new_snr_th2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SNR_TH2_VADDR);

	im_di_rtnr_new_snr_control_reg.rtnr_snr_en = ptr->RTNR_SNR_Enable;
	im_di_rtnr_new_snr_control_reg.rtnr_snr_sobel_1_en = ptr->RTNR_Sobel_En1;
	im_di_rtnr_new_snr_control_reg.rtnr_snr_sobel_2_en = ptr->RTNR_Soble_En2;
	im_di_rtnr_new_snr_control_reg.rtnr_snr_sobel_3_en = ptr->RTNR_Sobel_En3;
	im_di_rtnr_new_snr_control_reg.rtnr_snr_mask = ptr->_RTNR_SNR_Mask;

	im_di_rtnr_new_snr_th1_reg.rtnr_snr_edgestatus_th1 = ptr->RTNR_Edge_Status_th1;
	im_di_rtnr_new_snr_th1_reg.rtnr_snr_edgestatus_th2 = ptr->RTNR_Edge_Status_th2;
	im_di_rtnr_new_snr_th1_reg.rtnr_snr_edgestatus_th3 = ptr->RTNR_Edge_Status_th3;
	im_di_rtnr_new_snr_th1_reg.rtnr_snr_edgestatus_th4 = ptr->RTNR_Edge_Status_th4;
	im_di_rtnr_new_snr_th2_reg.rtnr_snr_edgestatus_th5 = ptr->RTNR_Edge_Status_th5;
	im_di_rtnr_new_snr_th2_reg.rtnr_snr_edgestatus_th6 = ptr->RTNR_Edge_Status_th6;
	im_di_rtnr_new_snr_th2_reg.rtnr_snr_edgestatus_th7 = ptr->RTNR_Edge_Status_th7;

	im_di_rtnr_new_snr_control_reg.rtnr_snr_result_edgestatus_invert_en = ptr->RTNR_Invert_edge;
	im_di_rtnr_new_snr_control_reg.rtnr_snr_result_motion_invert_en = ptr->RTNR_Invert_K;
	im_di_rtnr_new_snr_control_reg.rtnr_snr_result_offset_invert_en = ptr->RTNR_Invert_offset;
	im_di_rtnr_new_snr_control_reg.rtnr_snr_result_edgestatus_weight = ptr->RTNR_W_edge;
	im_di_rtnr_new_snr_control_reg.rtnr_snr_result_motion_weight = ptr->RTNR_W_K;
	im_di_rtnr_new_snr_control_reg.rtnr_snr_result_offset = ptr->RTNR_W_offset;

	IoReg_Write32(DI_IM_DI_RTNR_NEW_SNR_CONTROL_VADDR, im_di_rtnr_new_snr_control_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SNR_TH_1_VADDR, im_di_rtnr_new_snr_th1_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SNR_TH2_VADDR, im_di_rtnr_new_snr_th2_reg.regValue);
*/
}
#endif

void drvif_color_DRV_RTNR_C(DRV_RTNR_C *ptr)
{

	di_im_di_rtnr_c_th0_th3_RBUS im_di_rtnr_c_th0_th3_reg;
	di_im_di_rtnr_c_th4_th6_RBUS im_di_rtnr_c_th4_th6_reg;
	di_im_di_rtnr_c_k_RBUS im_di_rtnr_c_k_reg;
	im_di_rtnr_c_th0_th3_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_C_TH0_TH3_reg);
	im_di_rtnr_c_th4_th6_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_C_TH4_TH6_reg);
	im_di_rtnr_c_k_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_C_K_reg);

	im_di_rtnr_c_th0_th3_reg.cp_temporalthlc0 = ptr->RTNR_MAD_C_TH0;
	im_di_rtnr_c_th0_th3_reg.cp_temporalthlc1 = ptr->RTNR_MAD_C_TH1;
	im_di_rtnr_c_th0_th3_reg.cp_temporalthlc2 = ptr->RTNR_MAD_C_TH2;
	im_di_rtnr_c_th0_th3_reg.cp_temporalthlc3 = ptr->RTNR_MAD_C_TH3;
	im_di_rtnr_c_th4_th6_reg.cp_temporalthlc4 = ptr->RTNR_MAD_C_TH4;
	im_di_rtnr_c_th4_th6_reg.cp_temporalthlc5 = ptr->RTNR_MAD_C_TH5;
	im_di_rtnr_c_th4_th6_reg.cp_temporalthlc6 = ptr->RTNR_MAD_C_TH6;
	im_di_rtnr_c_k_reg.regValue = ptr->RTNR_C_K7_C_K0;

	IoReg_Write32(DI_IM_DI_RTNR_C_TH0_TH3_reg, im_di_rtnr_c_th0_th3_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_C_TH4_TH6_reg, im_di_rtnr_c_th4_th6_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_C_K_reg, im_di_rtnr_c_k_reg.regValue);
}

// chen 170522
#if 0
void drvif_color_DRV_RTNR_By_Y(DRV_RTNR_By_Y *ptr)
{

	di_im_di_rtnr_by_y_ratio_1_RBUS di_im_di_rtnr_by_y_ratio_1_reg;
	di_im_di_rtnr_by_y_ratio_2_RBUS di_im_di_rtnr_by_y_ratio_2_reg;
	di_im_di_rtnr_by_y_ratio_3_RBUS di_im_di_rtnr_by_y_ratio_3_reg;

	di_im_di_rtnr_by_y_ratio_1_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_BY_Y_RATIO_1_reg);
	di_im_di_rtnr_by_y_ratio_2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_BY_Y_RATIO_2_reg);
	di_im_di_rtnr_by_y_ratio_3_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_BY_Y_RATIO_3_reg);

	di_im_di_rtnr_by_y_ratio_1_reg.rtnr_by_y_en					 =				ptr->RTNR_By_Y_EN;
	di_im_di_rtnr_by_y_ratio_1_reg.rtnr_by_y_level_0_ratio         =		        ptr->Rtnr_by_Y_level_0_ratio;
	di_im_di_rtnr_by_y_ratio_1_reg.rtnr_by_y_level_16_ratio        =			    ptr->Rtnr_by_Y_level_16_ratio;
	di_im_di_rtnr_by_y_ratio_1_reg.rtnr_by_y_level_32_ratio        =		        ptr->Rtnr_by_Y_level_32_ratio;
	di_im_di_rtnr_by_y_ratio_1_reg.rtnr_by_y_level_48_ratio        =		        ptr->Rtnr_by_Y_level_48_ratio;
	di_im_di_rtnr_by_y_ratio_1_reg.rtnr_by_y_level_64_ratio        =		        ptr->Rtnr_by_Y_level_64_ratio;
	di_im_di_rtnr_by_y_ratio_1_reg.rtnr_by_y_level_80_ratio        =		        ptr->Rtnr_by_Y_level_80_ratio;
	di_im_di_rtnr_by_y_ratio_1_reg.rtnr_by_y_level_96_ratio        =		        ptr->Rtnr_by_Y_level_96_ratio;
	di_im_di_rtnr_by_y_ratio_2_reg.rtnr_by_y_level_112_ratio       =		        ptr->Rtnr_by_Y_level_112_ratio;
	di_im_di_rtnr_by_y_ratio_2_reg.rtnr_by_y_level_128_ratio       =		        ptr->Rtnr_by_Y_level_128_ratio;
	di_im_di_rtnr_by_y_ratio_2_reg.rtnr_by_y_level_144_ratio       =		        ptr->Rtnr_by_Y_level_144_ratio;
	di_im_di_rtnr_by_y_ratio_2_reg.rtnr_by_y_level_160_ratio       =		        ptr->Rtnr_by_Y_level_160_ratio;
	di_im_di_rtnr_by_y_ratio_2_reg.rtnr_by_y_level_176_ratio       =		        ptr->Rtnr_by_Y_level_176_ratio;
	di_im_di_rtnr_by_y_ratio_2_reg.rtnr_by_y_level_192_ratio       =		        ptr->Rtnr_by_Y_level_192_ratio;
	di_im_di_rtnr_by_y_ratio_2_reg.rtnr_by_y_level_208_ratio       =		        ptr->Rtnr_by_Y_level_208_ratio;
	di_im_di_rtnr_by_y_ratio_2_reg.rtnr_by_y_level_224_ratio       =		        ptr->Rtnr_by_Y_level_224_ratio;
	di_im_di_rtnr_by_y_ratio_3_reg.rtnr_by_y_level_240_ratio       =		        ptr->Rtnr_by_Y_level_240_ratio;
	di_im_di_rtnr_by_y_ratio_3_reg.rtnr_by_y_level_255_ratio       =		        ptr->Rtnr_by_Y_level_255_ratio;

	IoReg_Write32(DI_IM_DI_RTNR_BY_Y_RATIO_1_reg, di_im_di_rtnr_by_y_ratio_1_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_BY_Y_RATIO_2_reg, di_im_di_rtnr_by_y_ratio_2_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_BY_Y_RATIO_3_reg, di_im_di_rtnr_by_y_ratio_3_reg.regValue);

// Merlin3 no
}
#endif
// end chen 170522

void drvif_color_DRV_SNR_General_ctrl(DRV_SNR_General_ctrl *ptr, unsigned char bIsVDSrc)
{
	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_dch1_local_var2_RBUS dch1_Local_Var2_Reg;
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	iedge_smooth_prelpf_ctrl_RBUS prelpf_ctrl_reg;

	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	dch1_Local_Var2_Reg.regValue = IoReg_Read32(NR_DCH1_Local_Var2_reg);
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	prelpf_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_prelpf_ctrl_reg);

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); /* DB Enable */

	/*SNR*/
	dch1_cp_ctrl_reg.cp_spatialenabley = ptr->SNR_spatialenabley;
	dch1_cp_ctrl_reg.cp_spatialenablec =  ptr->SNR_spatialenablec;
	dch1_cp_ctrl_reg.cp_impulseenable = ptr->SNR_impulseenable;
	dch1_Local_Var2_Reg.cp_en_yedgeforc = ptr->SNR_en_yedgeforc;
	/*nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en = ptr->SNR_mosquito_detect_en;*/	/* the spec is MosquitoNR Enable is controlled by MPEG NR UI */
	prelpf_ctrl_reg.prelpf_snr_sel = ptr->SNR_prelpf_snr_sel;
	dch1_cp_ctrl_reg.cp_sresultweight = ptr->SNR_sresultweight;
	//dch1_cp_ctrl_reg.vc_nr_maxmin_range_sel = ptr->SNR_maxmin_range; /*Merlin2*/

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(NR_DCH1_Local_Var2_reg, dch1_Local_Var2_Reg.regValue);
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_prelpf_ctrl_reg, prelpf_ctrl_reg.regValue);

#if 0 //mac6p modified vd snr
	if (bIsVDSrc) {
		vdpq_spnr_cp_ctrl_RBUS spnr_cp_ctrl_reg;
		spnr_cp_ctrl_reg.regValue = vdcf_inl(VDPQ_SPNR_CP_Ctrl_reg);

		/*VDSNR*/
		spnr_cp_ctrl_reg.cp_spatialenabley = ptr->SNR_VD_spatialenabley;
		spnr_cp_ctrl_reg.cp_spatialenablec = ptr->SNR_VD_spatialenablec;
		spnr_cp_ctrl_reg.cp_sresultweight = ptr->SNR_VD_sresultweight;
		vdcf_outl(VDPQ_SPNR_CP_Ctrl_reg, spnr_cp_ctrl_reg.regValue);
	}
#endif

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */

}

void drvif_color_DRV_SNR_Spatial_NR_Y(DRV_SNR_Spatial_NR_Y *ptr)
{
	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_dch1_spatial_thl_RBUS dch1_spatial_thl_reg;
	nr_dch1_spatial_thl2_RBUS dch1_spatial_thl2_reg;
	nr_dch1_spatial_weight_RBUS dch1_Spatial_Weight_Reg;
	nr_dch1_local_var2_RBUS dch1_Local_Var2_Reg;
	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	dch1_spatial_thl_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl_reg);
	dch1_spatial_thl2_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl2_reg);
	dch1_Spatial_Weight_Reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Weight_reg);
	dch1_Local_Var2_Reg.regValue = IoReg_Read32(NR_DCH1_Local_Var2_reg);
	/*SNR Blending*/
	dch1_Spatial_Weight_Reg.cp_fixedweight1y = ptr->SNR_fixedweight1y;
	dch1_Spatial_Weight_Reg.cp_fixedweight2y = ptr->SNR_fixedweight2y;
	dch1_Spatial_Weight_Reg.cp_weight1y = ptr->SNR_weight1y;
	dch1_Spatial_Weight_Reg.cp_weight2y = ptr->SNR_weight2y;
	/* 3*9 LPF*/
	dch1_cp_ctrl_reg.cp_zoranfiltersizey = ptr->SNR_zoranfiltersizey;
	dch1_cp_ctrl_reg.cp_zoranweight2y = ptr->SNR_zoranweight2y;
	dch1_cp_ctrl_reg.cp_zoranweight3y = ptr->SNR_zoranweight3y;
	dch1_cp_ctrl_reg.cp_zoranweight4y = ptr->SNR_zoranweight4y;
	dch1_spatial_thl_reg.cp_spatialthly = ptr->SNR_spatialthly;
	dch1_spatial_thl_reg.cp_spatialthlyx2 = ptr->SNR_spatialthlyx2;
	dch1_spatial_thl2_reg.cp_spatialthlyx4 = ptr->SNR_spatialthlyx4;
	/*Sw Edge Control*/
	dch1_Local_Var2_Reg.cp_edge_weight = ptr->SNR_edge_weight;
	dch1_Local_Var2_Reg.cp_en_sw_edge_thl = ptr->SNR_en_sw_edge_thl;
	dch1_Local_Var2_Reg.cp_sw_edge_thl = ptr->SNR_sw_edge_thl;

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl_reg, dch1_spatial_thl_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl2_reg, dch1_spatial_thl2_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Weight_reg, dch1_Spatial_Weight_Reg.regValue);
	IoReg_Write32(NR_DCH1_Local_Var2_reg, dch1_Local_Var2_Reg.regValue);

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */

}

void drvif_color_DRV_SNR_Spatial_NR_C(DRV_SNR_Spatial_NR_C *ptr)
{
	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_dch1_spatial_thl_RBUS dch1_spatial_thl_reg;
	nr_dch1_spatial_thl2_RBUS dch1_spatial_thl2_reg;
	nr_dch1_spatial_weight_RBUS dch1_Spatial_Weight_Reg;
	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	dch1_spatial_thl_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl_reg);
	dch1_spatial_thl2_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl2_reg);
	dch1_Spatial_Weight_Reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Weight_reg);
	/*SNR C*/
	dch1_Spatial_Weight_Reg.cp_lpresultweightc = ptr->SNR_lpresultweightc;
	dch1_cp_ctrl_reg.cp_zoranfiltersizec = ptr->SNR_zoranfiltersizec;
	dch1_cp_ctrl_reg.cp_zoranweight2c = ptr->SNR_zoranweight2c;
	dch1_cp_ctrl_reg.cp_zoranweight3c = ptr->SNR_zoranweight3c;
	dch1_cp_ctrl_reg.cp_zoranweight4c = ptr->SNR_zoranweight4c;
	dch1_spatial_thl_reg.cp_spatialthlc = ptr->SNR_spatialthlc;
	dch1_spatial_thl_reg.cp_spatialthlcx2 = ptr->SNR_spatialthlcx2;
	dch1_spatial_thl2_reg.cp_spatialthlcx4 = ptr->SNR_spatialthlcx4;

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl_reg, dch1_spatial_thl_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl2_reg, dch1_spatial_thl2_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Weight_reg, dch1_Spatial_Weight_Reg.regValue);

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */

}

void drvif_color_DRV_SNR_Modified_LPF(DRV_SNR_Modified_LPF *ptr)
{
	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_new_lpf_nearby_ctrl_RBUS new_lpf_nearby_ctrl_reg;
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	//nr_edge_filter_RBUS nr_edge_filter_reg; /*Merlin2*/
	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	new_lpf_nearby_ctrl_reg.regValue = IoReg_Read32(NR_NEW_LPF_NEARBY_CTRL_reg);
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	//nr_edge_filter_reg.regValue = IoReg_Read32(NR_EDGE_FILTER_reg);  /*Merlin2*/

	/*modified lpf*/
	dch1_cp_ctrl_reg.vc_snr_lpf_sel = ptr->SNR_LPF_sel; //mac6:0:3, 1:5, 2:mod3, 3:mod5 // 0:3, 1:5, 2:mod3, 3:mod5, 4:11, 5:mod11 (Merlin, jyyang@20150615)(Merlin2, jyyang@20160617)
	new_lpf_nearby_ctrl_reg.modified_lpf_edge_gain	= ptr->SNR_modified_lpf_edge_gain; /*Mac3, leonard wu@20140722*/
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_upbnd	= ptr->SNR_modified_lpf_thd_upbnd;
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_lowbnd = ptr->SNR_modified_lpf_thd_lowbnd;
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_step 	= ptr->SNR_modified_lpf_thd_step;

	//Merlin2, jyyang@20160617
	nr_mosquito_ctrl_RBUS_reg.mnr_lpf_sel = ptr->MNR_LPF_sel;
	//nr_edge_filter_reg.vc_edge_lpf_sel = ptr->Edge_LPF_sel;  /*Merlin2*/
	//new_lpf_nearby_ctrl_reg.vc_nr_lpf_h11tap_sel = ptr->LPF_H11_sel; /*Merlin2*/
	//new_lpf_nearby_ctrl_reg.vc_nr_lpf_v11tap_sel = ptr->LPF_V11_sel; /*Merlin2*/

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(NR_NEW_LPF_NEARBY_CTRL_reg, new_lpf_nearby_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	//IoReg_Write32(NR_EDGE_FILTER_reg, nr_edge_filter_reg.regValue);  /*Merlin2*/
}


void drvif_color_DRV_SNR_Impulse_NR(DRV_SNR_Impulse_NR *ptr)
{
	//nr_dch1_cp_ctrl_RBUS dch1_CP_Ctrl_reg;
	nr_dch1_impulse_thl_RBUS dch1_Impulse_Thl_reg;
	nr_dch1_impulse_smooththl_RBUS dch1_Impulse_SmoothThl_reg;
	//dch1_CP_Ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	dch1_Impulse_Thl_reg.regValue = IoReg_Read32(NR_DCH1_Impulse_Thl_reg);
	dch1_Impulse_SmoothThl_reg.regValue = IoReg_Read32(NR_DCH1_Impulse_SmoothThl_reg);
	//marked by Elsie START
	//dch1_CP_Ctrl_reg.cp_impulsewindow = ptr->SNR_Impulsewindow;
	//dch1_CP_Ctrl_reg.cp_iresultweight = ptr->SNR_IResultWeight;
	//makred by ELsie END
	//modified by Elsie START
	dch1_Impulse_Thl_reg.cp_impulsewindow = ptr->SNR_Impulsewindow;
	dch1_Impulse_Thl_reg.cp_iresultweight = ptr->SNR_IResultWeight;
	//modified by Elsie END
	dch1_Impulse_Thl_reg.cp_ipixeldiffthly = ptr->SNR_IPixelDiffThlY;
	dch1_Impulse_Thl_reg.cp_impulsethly = ptr->SNR_ImpulseThlY;
	dch1_Impulse_SmoothThl_reg.cp_impulsethlyx2 = ptr->SNR_ImpulseThlYx2;
	dch1_Impulse_SmoothThl_reg.cp_impulsesmooththly = ptr->SNR_ImpulseSmoothThlY;
	dch1_Impulse_Thl_reg.cp_ipixeldiffthlc = ptr->SNR_IPixelDiffThlC;
	dch1_Impulse_Thl_reg.cp_impulsethlc = ptr->SNR_ImpulseThlC;
	dch1_Impulse_SmoothThl_reg.cp_impulsethlcx2 = ptr->SNR_ImpulseThlCx2;
	dch1_Impulse_SmoothThl_reg.cp_impulsesmooththlc = ptr->SNR_ImpulseSmoothThlC;
	//IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_CP_Ctrl_reg.regValue);
	IoReg_Write32(NR_DCH1_Impulse_Thl_reg, dch1_Impulse_Thl_reg.regValue);
	IoReg_Write32(NR_DCH1_Impulse_SmoothThl_reg, dch1_Impulse_SmoothThl_reg.regValue);
}

void drvif_color_DRV_SNR_Curve_Mapping_Mode(DRV_SNR_Curve_Mapping_Mode *ptr)
{
	nr_curve_mapping_ctrl_RBUS curve_mapping_ctrl_reg;
	nr_curve_mapping_step_0_RBUS curve_mapping_step_0_reg;
	nr_curve_mapping_step_1_RBUS curve_mapping_step_1_reg;
	nr_curve_mapping_step_2_RBUS curve_mapping_step_2_reg;
	nr_curve_mapping_step_3_RBUS curve_mapping_step_3_reg;
	nr_curve_mapping_w1_1_RBUS curve_mapping_w1_1_reg;
	nr_curve_mapping_w1_2_RBUS curve_mapping_w1_2_reg;
	nr_curve_mapping_w1_3_RBUS curve_mapping_w1_3_reg;
	nr_curve_mapping_w2_1_RBUS curve_mapping_w2_1_reg;
	nr_curve_mapping_w2_2_RBUS curve_mapping_w2_2_reg;
	nr_curve_mapping_w2_3_RBUS curve_mapping_w2_3_reg;
	curve_mapping_ctrl_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_CTRL_reg);
	curve_mapping_step_0_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_0_reg);
	curve_mapping_step_1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_1_reg);
	curve_mapping_step_2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_2_reg);
	curve_mapping_step_3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_3_reg);
	curve_mapping_w1_1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_1_reg);
	curve_mapping_w1_2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_2_reg);
	curve_mapping_w1_3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_3_reg);
	curve_mapping_w2_1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W2_1_reg);
	curve_mapping_w2_2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W2_2_reg);
	curve_mapping_w2_3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W2_3_reg);
	/*Curve Mapping Mode_Global Control*/
	curve_mapping_ctrl_reg.curvemappingmode_en = ptr->SNR_curvemappingmode_en;
	/*Curve Mapping Mode_step*/
	curve_mapping_step_0_reg.curvemapping_step1 =  ptr->SNR_curvemapping_step1;
	curve_mapping_step_0_reg.curvemapping_step2 =  ptr->SNR_curvemapping_step2;
	curve_mapping_step_1_reg.curvemapping_step3 =  ptr->SNR_curvemapping_step3;
	curve_mapping_step_1_reg.curvemapping_step4 =  ptr->SNR_curvemapping_step4;
	curve_mapping_step_2_reg.curvemapping_step5 =  ptr->SNR_curvemapping_step5;
	curve_mapping_step_2_reg.curvemapping_step6 =  ptr->SNR_curvemapping_step6;
	curve_mapping_step_3_reg.curvemapping_step7 =  ptr->SNR_curvemapping_step7;
	curve_mapping_step_3_reg.curvemapping_step8 =  ptr->SNR_curvemapping_step8;
	/*Curve Mapping Mode_Weight1*/
	curve_mapping_w1_1_reg.curvemapping_w1_0 =  ptr->SNR_curvemapping_w1_0;
	curve_mapping_w1_1_reg.curvemapping_w1_1 =  ptr->SNR_curvemapping_w1_1;
	curve_mapping_w1_1_reg.curvemapping_w1_2 =  ptr->SNR_curvemapping_w1_2;
	curve_mapping_w1_1_reg.curvemapping_w1_3 =  ptr->SNR_curvemapping_w1_3;
	curve_mapping_w1_2_reg.curvemapping_w1_4 =  ptr->SNR_curvemapping_w1_4;
	curve_mapping_w1_2_reg.curvemapping_w1_5 =  ptr->SNR_curvemapping_w1_5;
	curve_mapping_w1_2_reg.curvemapping_w1_6 =  ptr->SNR_curvemapping_w1_6;
	curve_mapping_w1_2_reg.curvemapping_w1_7 =  ptr->SNR_curvemapping_w1_7;
	curve_mapping_w1_3_reg.curvemapping_w1_8 =  ptr->SNR_curvemapping_w1_8;
	/*Curve Mapping Mode_Weight2*/
	curve_mapping_w2_1_reg.curvemapping_w2_0 = ptr->SNR_curvemapping_w2_0;
	curve_mapping_w2_1_reg.curvemapping_w2_1 = ptr->SNR_curvemapping_w2_1;
	curve_mapping_w2_1_reg.curvemapping_w2_2 = ptr->SNR_curvemapping_w2_2;
	curve_mapping_w2_1_reg.curvemapping_w2_3 = ptr->SNR_curvemapping_w2_3;
	curve_mapping_w2_2_reg.curvemapping_w2_4 = ptr->SNR_curvemapping_w2_4;
	curve_mapping_w2_2_reg.curvemapping_w2_5 = ptr->SNR_curvemapping_w2_5;
	curve_mapping_w2_2_reg.curvemapping_w2_6 = ptr->SNR_curvemapping_w2_6;
	curve_mapping_w2_2_reg.curvemapping_w2_7 = ptr->SNR_curvemapping_w2_7;
	curve_mapping_w2_3_reg.curvemapping_w2_8 = ptr->SNR_curvemapping_w2_8;

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en
	IoReg_Write32(NR_CURVE_MAPPING_CTRL_reg, curve_mapping_ctrl_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_STEP_0_reg, curve_mapping_step_0_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_STEP_1_reg, curve_mapping_step_1_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_STEP_2_reg, curve_mapping_step_2_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_STEP_3_reg, curve_mapping_step_3_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W1_1_reg, curve_mapping_w1_1_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W1_3_reg, curve_mapping_w1_3_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W2_1_reg, curve_mapping_w2_1_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W2_2_reg, curve_mapping_w2_2_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W2_3_reg, curve_mapping_w2_3_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); //DB apply
}

void drvif_color_DRV_SNR_NR_Mask(DRV_SNR_NR_Mask *ptr)
{
	nr_dch1_mask1_ctrl1_RBUS dch1_MASK1_CTRL1_Reg;
	nr_dch1_mask1_ctrl2_RBUS dch1_MASK1_CTRL2_Reg;
	nr_dch1_mask2_ctrl1_RBUS dch1_MASK2_CTRL1_Reg;
	nr_dch1_mask2_ctrl2_RBUS dch1_MASK2_CTRL2_Reg;
	nr_dch1_mask3_ctrl1_RBUS dch1_MASK3_CTRL1_Reg;
	nr_dch1_mask3_ctrl2_RBUS dch1_MASK3_CTRL2_Reg;
	dch1_MASK1_CTRL1_Reg.regValue = rtd_inl(NR_DCH1_MASK1_CTRL1_reg);
	dch1_MASK1_CTRL2_Reg.regValue = rtd_inl(NR_DCH1_MASK1_CTRL2_reg);
	dch1_MASK2_CTRL1_Reg.regValue = rtd_inl(NR_DCH1_MASK2_CTRL1_reg);
	dch1_MASK2_CTRL2_Reg.regValue = rtd_inl(NR_DCH1_MASK2_CTRL2_reg);
	dch1_MASK3_CTRL1_Reg.regValue = rtd_inl(NR_DCH1_MASK3_CTRL1_reg);
	dch1_MASK3_CTRL2_Reg.regValue = rtd_inl(NR_DCH1_MASK3_CTRL2_reg);
	dch1_MASK1_CTRL2_Reg.cp_mask1_en = ptr->SNR_Mask1_En;
	dch1_MASK1_CTRL2_Reg.cp_mask1_dir = ptr->SNR_Mask1_Dir;
	dch1_MASK1_CTRL1_Reg.cp_mask1_ygain = ptr->SNR_Mask1_Y_Gain;
	dch1_MASK1_CTRL1_Reg.cp_mask1_cbgain = ptr->SNR_Mask1_Cb_Gain;
	dch1_MASK1_CTRL1_Reg.cp_mask1_crgain = ptr->SNR_Mask1_Cr_Gain;
	dch1_MASK1_CTRL2_Reg.cp_mask1_mapstep = ptr->SNR_Mask1_Step;
	dch1_MASK1_CTRL2_Reg.cp_mask1_mapthre = ptr->SNR_Mask1_Thd;
	dch1_MASK1_CTRL2_Reg.cp_mask1_maplimit = (ptr->SNR_Mask1_Limit-1);
	dch1_MASK1_CTRL1_Reg.cp_mask1_yref = ptr->SNR_Mask1_Y_Ref;
	dch1_MASK1_CTRL1_Reg.cp_mask1_cbref = ptr->SNR_Mask1_Cb_Ref;
	dch1_MASK1_CTRL1_Reg.cp_mask1_crref = ptr->SNR_Mask1_Cr_Ref;
	dch1_MASK2_CTRL2_Reg.cp_mask2_en = ptr->SNR_Mask2_En;
	dch1_MASK2_CTRL2_Reg.cp_mask2_dir = ptr->SNR_Mask2_Dir;
	dch1_MASK2_CTRL1_Reg.cp_mask2_ygain = ptr->SNR_Mask2_Y_Gain;
	dch1_MASK2_CTRL1_Reg.cp_mask2_cbgain = ptr->SNR_Mask2_Cb_Gain;
	dch1_MASK2_CTRL1_Reg.cp_mask2_crgain = ptr->SNR_Mask2_Cr_Gain;
	dch1_MASK2_CTRL2_Reg.cp_mask2_mapstep = ptr->SNR_Mask2_Step;
	dch1_MASK2_CTRL2_Reg.cp_mask2_mapthre = ptr->SNR_Mask2_Thd;
	dch1_MASK2_CTRL2_Reg.cp_mask2_maplimit = (ptr->SNR_Mask2_Limit-1);
	dch1_MASK2_CTRL1_Reg.cp_mask2_yref = ptr->SNR_Mask2_Y_Ref;
	dch1_MASK2_CTRL1_Reg.cp_mask2_cbref = ptr->SNR_Mask2_Cb_Ref;
	dch1_MASK2_CTRL1_Reg.cp_mask2_crref = ptr->SNR_Mask2_Cr_Ref;
	dch1_MASK3_CTRL2_Reg.cp_mask3_en = ptr->SNR_Mask3_En;
	dch1_MASK3_CTRL2_Reg.cp_mask3_dir = ptr->SNR_Mask3_Dir;
	dch1_MASK3_CTRL1_Reg.cp_mask3_ygain = ptr->SNR_Mask3_Y_Gain;
	dch1_MASK3_CTRL1_Reg.cp_mask3_cbgain = ptr->SNR_Mask3_Cb_Gain;
	dch1_MASK3_CTRL1_Reg.cp_mask3_crgain = ptr->SNR_Mask3_Cr_Gain;
	dch1_MASK3_CTRL2_Reg.cp_mask3_mapstep = ptr->SNR_Mask3_Step;
	dch1_MASK3_CTRL2_Reg.cp_mask3_mapthre = ptr->SNR_Mask3_Thd;
	dch1_MASK3_CTRL2_Reg.cp_mask3_maplimit = (ptr->SNR_Mask3_Limit-1);
	dch1_MASK3_CTRL1_Reg.cp_mask3_yref = ptr->SNR_Mask3_Y_Ref;
	dch1_MASK3_CTRL1_Reg.cp_mask3_cbref = ptr->SNR_Mask3_Cb_Ref;
	dch1_MASK3_CTRL1_Reg.cp_mask3_crref = ptr->SNR_Mask3_Cr_Ref;
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en
	rtd_outl(NR_DCH1_MASK1_CTRL1_reg, dch1_MASK1_CTRL1_Reg.regValue);
	rtd_outl(NR_DCH1_MASK1_CTRL2_reg, dch1_MASK1_CTRL2_Reg.regValue);
	rtd_outl(NR_DCH1_MASK2_CTRL1_reg, dch1_MASK2_CTRL1_Reg.regValue);
	rtd_outl(NR_DCH1_MASK2_CTRL2_reg, dch1_MASK2_CTRL2_Reg.regValue);
	rtd_outl(NR_DCH1_MASK3_CTRL1_reg, dch1_MASK3_CTRL1_Reg.regValue);
	rtd_outl(NR_DCH1_MASK3_CTRL2_reg, dch1_MASK3_CTRL2_Reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); //DB apply
}

void drvif_color_DRV_SNR_NR_Mask_W1W2(DRV_SNR_NR_Mask_W1W2 *ptr)  /*Mac3, leonard wu@20140722*/
{
    nr_dch1_mask_shift_ctrl1_RBUS   nr_dch1_mask_shift_ctrl1_reg;
	nr_dch1_mask_shift_ctrl2_RBUS   nr_dch1_mask_shift_ctrl2_reg;
    nr_dch1_mask_shift_ctrl1_reg.regValue = IoReg_Read32(NR_DCH1_MASK_SHIFT_CTRL1_reg);
    nr_dch1_mask_shift_ctrl2_reg.regValue = IoReg_Read32(NR_DCH1_MASK_SHIFT_CTRL2_reg);

	nr_dch1_mask_shift_ctrl1_reg.nr_mask_shift_mode 	= ptr->SNR_mask_shift_mode;
    nr_dch1_mask_shift_ctrl1_reg.nr_mask1_filter_select = ptr->SNR_nrm1_filter_select;
    nr_dch1_mask_shift_ctrl1_reg.nr_mask1_w1_maplimit 	= ptr->SNR_nrm1_w1_limit;
    nr_dch1_mask_shift_ctrl1_reg.nr_mask1_w1_mapstep 	= ptr->SNR_nrm1_w1_step;
    nr_dch1_mask_shift_ctrl1_reg.nr_mask1_w2_maplimit 	= ptr->SNR_nrm1_w2_limit;
    nr_dch1_mask_shift_ctrl1_reg.nr_mask1_w2_mapstep 	= ptr->SNR_nrm1_w2_step;

    nr_dch1_mask_shift_ctrl1_reg.nr_mask2_filter_select = ptr->SNR_nrm2_filter_select;
    nr_dch1_mask_shift_ctrl1_reg.nr_mask2_w2_maplimit 	= ptr->SNR_nrm2_w2_limit;
    nr_dch1_mask_shift_ctrl1_reg.nr_mask2_w2_mapstep 	= ptr->SNR_nrm2_w2_step;
    nr_dch1_mask_shift_ctrl2_reg.nr_mask2_w1_maplimit 	= ptr->SNR_nrm2_w1_limit;
    nr_dch1_mask_shift_ctrl2_reg.nr_mask2_w1_mapstep 	= ptr->SNR_nrm2_w1_step;

    nr_dch1_mask_shift_ctrl2_reg.nr_mask3_filter_select = ptr->SNR_nrm3_filter_select;
    nr_dch1_mask_shift_ctrl2_reg.nr_mask3_w1_maplimit 	= ptr->SNR_nrm3_w1_limit;
    nr_dch1_mask_shift_ctrl2_reg.nr_mask3_w1_mapstep 	= ptr->SNR_nrm3_w1_step;
    nr_dch1_mask_shift_ctrl2_reg.nr_mask3_w2_maplimit 	= ptr->SNR_nrm3_w2_limit;
    nr_dch1_mask_shift_ctrl2_reg.nr_mask3_w2_mapstep 	= ptr->SNR_nrm3_w2_step;

    rtd_outl(NR_DCH1_MASK_SHIFT_CTRL1_reg, nr_dch1_mask_shift_ctrl1_reg.regValue);
    rtd_outl(NR_DCH1_MASK_SHIFT_CTRL2_reg, nr_dch1_mask_shift_ctrl2_reg.regValue);
}
void drvif_color_DRV_SNR_Veritcal_NR(DRV_SNR_Veritcal_NR *ptr)
{
	iedge_smooth_vfilter_0_RBUS vfilter_0_reg;
	iedge_smooth_vfilter_1_RBUS vfilter_1_reg;
	iedge_smooth_vfilter_2_RBUS vfilter_2_reg;
	iedge_smooth_vfilter_3_RBUS vfilter_3_reg;
#if 0 //merlin2
	iedge_smooth_vfilter_4_RBUS vfilter_4_reg;
	iedge_smooth_vfilter_5_RBUS vfilter_5_reg;
	iedge_smooth_vfilter_6_RBUS vfilter_6_reg;
#endif

	vfilter_0_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_0_reg);
	vfilter_1_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_1_reg);
	vfilter_2_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_2_reg);
	vfilter_3_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_3_reg);
#if 0 //merlin2
	vfilter_4_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_4_reg);
	vfilter_5_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_5_reg);
	vfilter_6_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_6_reg);
#endif

	vfilter_0_reg.avlpf_en = ptr->SNR_Veritcal_avlpf_En;
	vfilter_2_reg.avlpf_c_en = ptr->SNR_Veritcal_C_En;
	vfilter_0_reg.avlpf_correct_en = ptr->SNR_Veritcal_correct_En;
	vfilter_0_reg.avlpf_bw_pattern_en = ptr->SNR_Veritcal_BW_En;
	vfilter_0_reg.vfir_weight = ptr->SNR_Veritcal_Y_Weight;
	vfilter_2_reg.vfir_c_weight = ptr->SNR_Veritcal_C_Weight;
	vfilter_0_reg.avlpf_mindiff_th_start = ptr->SNR_Veritcal_Th_start;
	vfilter_0_reg.avlpf_mindiff_curve_rate_start = ptr->SNR_Veritcal_Rate_start;
	vfilter_0_reg.avlpf_mindiff_th_finish = ptr->SNR_Veritcal_Th_finish;
	vfilter_0_reg.avlpf_mindiff_curve_rate_finish = ptr->SNR_Veritcal_Rate_finish;
	//vfilter_0_reg.avlpf_tap_sel = ptr->SNR_Veritcal_avlpf_Tap_Sel; /*Merlin3*/
	vfilter_1_reg.avlpf_max_weight = ptr->SNR_Veritcal_Max_MinDiff_Weight;
	vfilter_1_reg.avlpf_min_weight = ptr->SNR_Veritcal_Min_MinDiff_Weight;
	vfilter_1_reg.avlpf_maxmindiff_th = ptr->SNR_Veritcal_MaxMinDiff_Th;
	vfilter_1_reg.avlpf_maxmindiff_curve_rate = ptr->SNR_Veritcal_MaxMinDiff_Rate;
	vfilter_2_reg.avlpf_bw_th = ptr->SNR_Veritcal_BWPattern_Th;
	vfilter_2_reg.avlpf_bw_margin = ptr->SNR_Veritcal_BWPattern_Margin;
	vfilter_2_reg.avlpf_bw_curve_rate = ptr->SNR_Veritcal_BWPattern_Rate;

#if 0 //merlin2
	vfilter_4_reg.esmex_iedge_vfir_enable = ptr->SNR_Vertical_fir_En;
	vfilter_4_reg.egsm_avlpf_vmode = ptr->SNR_Veritcal_Vmode;
	vfilter_4_reg.egsm_avlpf_clamp_en = ptr->SNR_Vertical_Clamp_En;
	vfilter_4_reg.egsm_avlpf_clampth = ptr->SNR_Vertical_Clamp_Th;
	vfilter_4_reg.egsm_avlpf_mindiff_th2_start = ptr->SNR_Vertical_Th2_start;
	vfilter_6_reg.egsm_avlpf2d_en = ptr->SNR_Veritcal_avlpf2D_En;
	vfilter_6_reg.egsm_avlpf2d_tolirant_en = ptr->SNR_Veritcal_avlpf2D_Tolirant_En;
	vfilter_5_reg.egsm_avlpf2d_tolirantmode = ptr->SNR_Veritcal_avlpf2D_Tolirant_Mode;
	vfilter_6_reg.egsm_avlpf2d_tolirantoffset = ptr->SNR_Veritcal_avlpf2D_Tolirant_Offset;
	// :Vfir_En, :Vmode, :Clamp_En, :Clamp_th, :Th2_sta, :avlpf2D_En, :avlpf2D_Tol_En, :avlpf2D_Tol_Mode, :avlpf2D_Tol_Offset
	// 0, 1, 0, 6, 0, 0, 0, 0, 4
#else //mac6
	vfilter_3_reg.egsm_avlpf_clamp_en = ptr->SNR_Vertical_Clamp_En;
	vfilter_3_reg.egsm_avlpf_clampth = ptr->SNR_Vertical_Clamp_Th;
	vfilter_3_reg.egsm_avlpf_clampth_tth = ptr->SNR_Vertical_Clamp_Th_TTH;
	vfilter_3_reg.egsm_avlpf_ignorehlh = ptr->SNR_Vertical_IgnoreHLH;
#endif

	IoReg_Write32(IEDGE_SMOOTH_VFILTER_0_reg, vfilter_0_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_1_reg, vfilter_1_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_2_reg, vfilter_2_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_3_reg, vfilter_3_reg.regValue);

#if 0 //merlin2
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_4_reg, vfilter_4_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_5_reg, vfilter_5_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_6_reg, vfilter_6_reg.regValue);
#endif

}

void drvif_color_DRV_SNR_Mosquito_NR(DRV_SNR_Mosquito_NR *ptr)
{
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_RBUS nr_mosquito_nr_level_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_1_RBUS nr_mosquito_nr_level_ctrl_1_RBUS_reg;
    nr_mosquito_nr_keep_edge_ctrl_RBUS 	nr_mosquito_nr_keep_edge_ctrl_reg;
    nr_mosquito_nr_decay_ctrl_1_RBUS   	nr_mosquito_nr_decay_ctrl_1_reg;

	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	nr_mosquito_nr_level_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_reg);
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg);
	nr_mosquito_nr_keep_edge_ctrl_reg.regValue 		= IoReg_Read32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg);
	nr_mosquito_nr_decay_ctrl_1_reg.regValue		= IoReg_Read32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg);
	/*Mosquito_NR_MNR Global Control*/

	nr_mosquito_ctrl_RBUS_reg.mosquitonr_weight = ptr->SNR_mosquitonr_weight;
	nr_mosquito_ctrl_RBUS_reg.mosquitonr_blending_mode = ptr->SNR_mosquitonr_blending_mode;
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.noise_level_mode = ptr->SNR_noise_level_mode;
	nr_mosquito_ctrl_RBUS_reg.mosquitonr_vertical_en 		= ptr->SNR_vertical_en; /*Mac3*/
    	nr_mosquito_nr_level_ctrl_1_RBUS_reg.edge_lpf_en		= ptr->SNR_edge_lpf_en;
	nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_en			= ptr->SNR_mnr_decay_en;
	nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_allcomp_en			= ptr->SNR_mnr_decay_allcomp_en; // Merlin4 add
	nr_mosquito_ctrl_RBUS_reg.mnr_adjustnoiselevel_bytype = ptr->MNR_adjustNoiseLevel_byType;/*Merlin jyyang 20150615*/
	nr_mosquito_ctrl_RBUS_reg.mnr_vardiff_refine_2dsearch = ptr->MNR_varDiff_refine_2Dsearch;
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.noise_level_trim		= ptr->MNR_nLevel_trim;/*Merlin jyyang 20150707*/
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_half_range		= ptr->MNR_hdiff_frac_half_range;/*Merlin jyyang 20150707*/
	//nr_mosquito_nr_level_ctrl_1_RBUS_reg.mnr_xrange_ext = ptr->MNR_xrange_ext; /*Merlin2*/
	/*Mosquito_NR_Edge Condition*/
	nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd = ptr->SNR_mosquito_edgethd;
	nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd_step = ptr->SNR_mosquito_edgethd_step;
	nr_mosquito_ctrl_RBUS_reg.mosquitonr_edge_strength_gain = ptr->SNR_mosquitonr_edge_strength_gain;
    /*decay*/
    nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_start		= ptr->SNR_hdecay_start; /*Mac3*/
    nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_step			= ptr->SNR_hdecay_step;
    nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_start		= ptr->SNR_vdecay_start;
    nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_step			= ptr->SNR_vdecay_step;
	/*Mosquito_NR_Noise level adjust*/
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.farvar_faredge_refine_en 	= ptr->SNR_farvar_faredge_refine_en;
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_gain = ptr->SNR_far_var_gain;
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_offset				= ptr->SNR_far_var_offset;
    nr_mosquito_nr_level_ctrl_RBUS_reg.mnr_vardiff_refine_gain 		= ptr->SNR_vardiff_refine_gain;
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_shift			= ptr->SNR_hdiff_frac_shift;
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.vdiff_frac_shift			= ptr->SNR_vdiff_frac_shift;

	/*keep edge*/
	nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd				= ptr->SNR_keep_edge_threshold;
    nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd_step			= ptr->SNR_keep_edge_threshold_step;
    nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedge_shift			= ptr->SNR_keep_edge_shift;

	/*Mosquito_NR_Final MNR Level Adjust*/
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_offset = ptr->SNR_mosquitonr_nlevel_positive_offset;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_shift = ptr->SNR_mosquitonr_nlevel_positive_shift;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_offset = ptr->SNR_mosquitonr_nlevel_negative_offset;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_shift = ptr->SNR_mosquitonr_nlevel_negative_shift;

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_reg, nr_mosquito_nr_level_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg, nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue);
    IoReg_Write32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg, nr_mosquito_nr_keep_edge_ctrl_reg.regValue);
    IoReg_Write32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg, nr_mosquito_nr_decay_ctrl_1_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); //DB apply
}

void drvif_color_DRV_SNR_Mosquito_NR_En(unsigned char En_flag)	/* Mosquito NR control is in general control*/
{
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);

	nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en = En_flag;	/* the spec is MosquitoNR Enable is controlled by MPEG NR UI */
#ifdef CONFIG_SNR_CLOCK_HW_ISSUE
	nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en = 1;
#endif //CONFIG_SNR_CLOCK_HW_ISSUE
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); //DB apply
}

void drvif_color_DRV_SNR_Weight_Adjust(DRV_SNR_WeightAdjust *ptr) /*mag2*/
{
	/*SNR weight adjust shift*/
	{
	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;

	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);

	dch1_cp_ctrl_reg.sresultweight_adjust_shift = ptr->SNR_sresultWeight_adjust_shift; /*mag2*/

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	}

	/* type detection*/
	{
	nr_type_detect_ctrl0_RBUS nr_type_detect_ctrl0_RBUS_reg;
	nr_type_detect_ctrl1_RBUS nr_type_detect_ctrl1_RBUS_reg;

	nr_type_detect_ctrl0_RBUS_reg.regValue = IoReg_Read32(NR_TYPE_DETECT_CTRL0_reg);
	nr_type_detect_ctrl1_RBUS_reg.regValue = IoReg_Read32(NR_TYPE_DETECT_CTRL1_reg);


	nr_type_detect_ctrl0_RBUS_reg.typedetect_max_step = ptr->SNR_TypeDetect_MaxStep;
	nr_type_detect_ctrl0_RBUS_reg.typedetect_max_thd = ptr->SNR_TypeDetect_MaxThd;
	nr_type_detect_ctrl1_RBUS_reg.typedetect_mid_step = ptr->SNR_TypeDetect_MidStep;
	nr_type_detect_ctrl1_RBUS_reg.typedetect_mid_thd = ptr->SNR_TypeDetect_MidThd;

	IoReg_Write32(NR_TYPE_DETECT_CTRL0_reg, nr_type_detect_ctrl0_RBUS_reg.regValue);
	IoReg_Write32(NR_TYPE_DETECT_CTRL1_reg, nr_type_detect_ctrl1_RBUS_reg.regValue);
	}

	/*line keep*/
	{
	nr_line_keep_ctrl0_RBUS nr_line_keep_ctrl0_RBUS_reg;
	nr_line_keep_ctrl1_RBUS	nr_line_keep_ctrl1_RBUS_reg;

	nr_line_keep_ctrl0_RBUS_reg.regValue = IoReg_Read32(NR_LINE_KEEP_CTRL0_reg);
	nr_line_keep_ctrl1_RBUS_reg.regValue = IoReg_Read32(NR_LINE_KEEP_CTRL1_reg);

	nr_line_keep_ctrl0_RBUS_reg.linekeep_edgeflag_thd = ptr->SNR_LineKeep_EdgeFlagThd;
	nr_line_keep_ctrl0_RBUS_reg.linekeep_flatflag_thd = ptr->SNR_LineKeep_FlatFlagThd;
	nr_line_keep_ctrl0_RBUS_reg.linekeep_linestrength_shift = ptr->SNR_LineKeep_LineStrengthShift;
	nr_line_keep_ctrl0_RBUS_reg.linekeep_mode = ptr->SNR_LineKeep_Mode;
	nr_line_keep_ctrl0_RBUS_reg.linekeep_shift = ptr->SNR_LineKeep_Shift;
	nr_line_keep_ctrl0_RBUS_reg.linekeep_verhor_ctrl = ptr->SNR_LineKeep_VerHorCtrl;

	nr_line_keep_ctrl1_RBUS_reg.linekeep_horline_judgeratio = ptr->SNR_LineKeep_HorLineJudgeRatio;
	nr_line_keep_ctrl1_RBUS_reg.linekeep_verline_judgeratio = ptr->SNR_LineKeep_VerLineJudgeRatio;
	nr_line_keep_ctrl1_RBUS_reg.linekeep_yuvdiff_step = ptr->SNR_LineKeep_YuvDiffStep;
	nr_line_keep_ctrl1_RBUS_reg.linekeep_yuvdiff_thd = ptr->SNR_LineKeep_YuvDiffThd;


	IoReg_Write32(NR_LINE_KEEP_CTRL0_reg, nr_line_keep_ctrl0_RBUS_reg.regValue);
	IoReg_Write32(NR_LINE_KEEP_CTRL1_reg, nr_line_keep_ctrl1_RBUS_reg.regValue);
	}
}

void drvif_color_DRV_SNR_UVEdge(DRV_SNR_UVEdge *ptr) /*Merlin jyyang 20150615*/
{
	nr_uv_modified_mosquito_edge_ctrl0_RBUS nr_uv_modified_mosquito_edge_ctrl0_reg;
	nr_uv_modified_mosquito_edge_ctrl1_RBUS nr_uv_modified_mosquito_edge_ctrl1_reg;

	nr_uv_modified_mosquito_edge_ctrl0_reg.regValue = IoReg_Read32(NR_UV_MODIFIED_MOSQUITO_EDGE_CTRL0_reg);
	nr_uv_modified_mosquito_edge_ctrl1_reg.regValue = IoReg_Read32(NR_UV_MODIFIED_MOSQUITO_EDGE_CTRL1_reg);

	nr_uv_modified_mosquito_edge_ctrl0_reg.mnr_uvedge_en = ptr->SNR_uvedge_en;
	nr_uv_modified_mosquito_edge_ctrl0_reg.mnr_uvedge_gain0 = ptr->SNR_uvedge_gain0;
	nr_uv_modified_mosquito_edge_ctrl0_reg.mnr_uvedge_gain1 = ptr->SNR_uvedge_gain1;
	nr_uv_modified_mosquito_edge_ctrl0_reg.mnr_uvedge_step0 = ptr->SNR_uvedge_step0;
	nr_uv_modified_mosquito_edge_ctrl0_reg.mnr_uvedge_step1 = ptr->SNR_uvedge_step1;
	nr_uv_modified_mosquito_edge_ctrl1_reg.mnr_uvedge_lowbd0 = ptr->SNR_uvedge_lowbd0;
	nr_uv_modified_mosquito_edge_ctrl1_reg.mnr_uvedge_lowbd1 = ptr->SNR_uvedge_lowbd1;

	IoReg_Write32(NR_UV_MODIFIED_MOSQUITO_EDGE_CTRL0_reg, nr_uv_modified_mosquito_edge_ctrl0_reg.regValue);
	IoReg_Write32(NR_UV_MODIFIED_MOSQUITO_EDGE_CTRL1_reg, nr_uv_modified_mosquito_edge_ctrl1_reg.regValue);
}

void drvif_color_DRV_SNR_VD_Spatial_NR_Y(DRV_SNR_Spatial_NR_Y *ptr)
{
#if 0 //mac6p modified vd snr
	vdpq_spnr_cp_ctrl_RBUS spnr_cp_ctrl_reg;
	vdpq_spnr_spatial_th_RBUS spnr_spatial_th_reg;
	vdpq_spnr_spatial_weight_RBUS spnr_spatial_weight_Reg;
	vdpq_spnr_local_var2_RBUS spnr_local_var2_Reg;
	spnr_cp_ctrl_reg.regValue = vdcf_inl(VDPQ_SPNR_CP_Ctrl_reg);
	spnr_spatial_th_reg.regValue = vdcf_inl(VDPQ_SPNR_Spatial_Th_reg);
	spnr_spatial_weight_Reg.regValue = vdcf_inl(VDPQ_SPNR_Spatial_Weight_reg);
	spnr_local_var2_Reg.regValue = vdcf_inl(VDPQ_SPNR_Local_Var2_reg);
	/*SNR Blending*/
	spnr_spatial_weight_Reg.cp_fixedweight1y = ptr->SNR_fixedweight1y;
	spnr_spatial_weight_Reg.cp_fixedweight2y = ptr->SNR_fixedweight2y;
	spnr_spatial_weight_Reg.cp_weight1y = ptr->SNR_weight1y;
	spnr_spatial_weight_Reg.cp_weight2y = ptr->SNR_weight2y;
	/* 3*9 LPF*/
	spnr_cp_ctrl_reg.cp_zoranfiltersizey = ptr->SNR_zoranfiltersizey;
	spnr_cp_ctrl_reg.cp_zoranweight2y = ptr->SNR_zoranweight2y;
	spnr_cp_ctrl_reg.cp_zoranweight3y = ptr->SNR_zoranweight3y;
	spnr_cp_ctrl_reg.cp_zoranweight4y = ptr->SNR_zoranweight4y;
	spnr_spatial_th_reg.cp_spatialthly = ptr->SNR_spatialthly;
	spnr_spatial_th_reg.cp_spatialthlyx2 = ptr->SNR_spatialthlyx2;
	spnr_spatial_th_reg.cp_spatialthlyx4 = ptr->SNR_spatialthlyx4;
	/*Sw Edge Control*/
	spnr_local_var2_Reg.cp_edge_weight = ptr->SNR_edge_weight;
	spnr_local_var2_Reg.cp_sw_edge_thl = ptr->SNR_sw_edge_thl;

	vdcf_outl(VDPQ_SPNR_CP_Ctrl_reg, spnr_cp_ctrl_reg.regValue);
	vdcf_outl(VDPQ_SPNR_Spatial_Th_reg, spnr_spatial_th_reg.regValue);
	vdcf_outl(VDPQ_SPNR_Spatial_Weight_reg, spnr_spatial_weight_Reg.regValue);
	vdcf_outl(VDPQ_SPNR_Local_Var2_reg, spnr_local_var2_Reg.regValue);
#endif
}

void drvif_color_DRV_MPG_General_ctrl(DRV_MPG_General_ctrl *ptr)
{
	return;/* dummy
	mpegnr_ich1_mpegnr2_RBUS ich1_mpegnr2_reg;
	nr_dch1_mpegnr1_RBUS dch1_MPEGNR1_Reg;
	ich1_mpegnr2_reg.regValue = IoReg_Read32(MPEGNR_ICH1_MPEGNR2_reg);
	dch1_MPEGNR1_Reg.regValue = IoReg_Read32(NR_DCH1_MPEGNR1_reg);
	ich1_mpegnr2_reg.cp_mpegenable_x = ptr->MPG_Enable_H;
	dch1_MPEGNR1_Reg.cp_mpegenable_y_y = ptr->MPG_SNR_Enabe_V_Y;
	dch1_MPEGNR1_Reg.cp_mpegenable_y_c = ptr->MPG_SNR_Enable_V_C;

	IoReg_Write32(MPEGNR_ICH1_MPEGNR2_reg, ich1_mpegnr2_reg.regValue);
	IoReg_Write32(NR_DCH1_MPEGNR1_reg, dch1_MPEGNR1_Reg.regValue);

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); // DB apply
	//*/
}

void drvif_color_DRV_MPG_H(DRV_MPEGNR_16_Table *ptr)
{
	drvif_color_DRV_MPG_H_tv006(ptr,(unsigned char)0);
}

void drvif_color_DRV_MPG_H_tv006(DRV_MPEGNR_16_Table *ptr, unsigned char skipFWreg)
{

	mpegnr_ich1_blend_nr_hor_step_RBUS	mpegnr_ich1_blend_nr_hor_step_reg	;
	mpegnr_ich1_mpegnr1_RBUS	mpegnr_ich1_mpegnr1_reg	;
	mpegnr_ich1_mpegnr_ub_delta1_RBUS	mpegnr_ich1_mpegnr_ub_delta1_reg	;
	mpegnr_ich1_mpegnr_ub_delta2_RBUS	mpegnr_ich1_mpegnr_ub_delta2_reg	;
	mpegnr_ich1_mpegnr_ub_cen_th_RBUS	mpegnr_ich1_mpegnr_ub_cen_th_reg	;
	mpegnr_ich1_mpegnr2_RBUS	mpegnr_ich1_mpegnr2_reg	;
	mpegnr_ich1_mpegnr_ub_cen_gain_RBUS	mpegnr_ich1_mpegnr_ub_cen_gain_reg	;
	mpegnr_ich1_mpegnr_ub_dif_cmp_RBUS	mpegnr_ich1_mpegnr_ub_dif_cmp_reg	;
	mpegnr_ich1_hist_blking_hor_range_RBUS	mpegnr_ich1_hist_blking_hor_range_reg	;
	mpegnr_ich1_hist_blking_ver_range_RBUS	mpegnr_ich1_hist_blking_ver_range_reg	;
	mpegnr_ich1_det_hist_hor_thd_hlh_RBUS	mpegnr_ich1_det_hist_hor_thd_hlh_reg	;
	mpegnr_ich1_mpegstart_RBUS	mpegnr_ich1_mpegstart_reg	;
	mpegnr_ich1_det_hist_hor_step_RBUS	mpegnr_ich1_det_hist_hor_step_reg	;
	mpegnr_ich1_det_hist_hor_thd_RBUS	mpegnr_ich1_det_hist_hor_thd_reg	;

	mpegnr_ich1_blend_nr_hor_step_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_blend_NR_Hor_step_reg	);
	mpegnr_ich1_mpegnr1_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR1_reg	);
	mpegnr_ich1_mpegnr_ub_delta1_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR_UB_delta1_reg	);
	mpegnr_ich1_mpegnr_ub_delta2_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR_UB_delta2_reg	);
	mpegnr_ich1_mpegnr_ub_cen_th_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR_UB_Cen_Th_reg	);
	mpegnr_ich1_mpegnr2_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR2_reg	);
	mpegnr_ich1_mpegnr_ub_cen_gain_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR_UB_Cen_Gain_reg	);
	mpegnr_ich1_mpegnr_ub_dif_cmp_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR_UB_Dif_Cmp_reg	);
	mpegnr_ich1_hist_blking_hor_range_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_Hist_Blking_Hor_Range_reg	);
	mpegnr_ich1_hist_blking_ver_range_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_Hist_Blking_Ver_Range_reg	);
	mpegnr_ich1_det_hist_hor_thd_hlh_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_det_Hist_Hor_thd_hlh_reg	);
	mpegnr_ich1_mpegstart_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGStart_reg	);
	mpegnr_ich1_det_hist_hor_step_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_det_Hist_Hor_step_reg	);
	mpegnr_ich1_det_hist_hor_thd_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_det_Hist_Hor_thd_reg	);

	mpegnr_ich1_mpegstart_reg.cp_histhoren = ptr->S_MPEGNR_16_Detector.enable;
	//_reg.cp_ = ptr->S_MPEGNR_16_Detector.Reserved_01;
	//_reg.cp_ = ptr->S_MPEGNR_16_Detector.diff_th_combo;
	mpegnr_ich1_det_hist_hor_thd_hlh_reg.cp_histhor_diff_th_hlh= ptr->S_MPEGNR_16_Detector.diff_th_hlh;
	// computed_by_HW mpegnr_ich1_hist_blking_hor_range_reg.cp_histhorstart = ptr->S_MPEGNR_16_Detector.bound_left;
	// computed_by_HW mpegnr_ich1_hist_blking_hor_range_reg.cp_histhorend = ptr->S_MPEGNR_16_Detector.bound_right;
	// computed_by_HW mpegnr_ich1_hist_blking_ver_range_reg.cp_histverstart = ptr->S_MPEGNR_16_Detector.bound_up;
	// computed_by_HW mpegnr_ich1_hist_blking_ver_range_reg.cp_histverend = ptr->S_MPEGNR_16_Detector.bound_down;
	mpegnr_ich1_det_hist_hor_thd_reg.cp_histhor_diff_th_low = ptr->S_MPEGNR_16_Detector.diff_th_low;
	mpegnr_ich1_det_hist_hor_thd_reg.cp_histhor_diff_th_high = ptr->S_MPEGNR_16_Detector.diff_th_high;
	mpegnr_ich1_det_hist_hor_step_reg.cp_histhor_det_xstep = ptr->S_MPEGNR_16_Detector.scaling;
	mpegnr_ich1_det_hist_hor_step_reg.cp_histhor_det_xini = ptr->S_MPEGNR_16_Detector.offset_pixel;
	//_reg.cp_ = ptr->S_MPEGNR_16_Detector.offset_step;
	//_reg.cp_ = ptr->S_MPEGNR_16_Detector.offset_forgotten;
	//_reg.cp_ = ptr->S_MPEGNR_16_Detector.subsampling_mode;

	mpegnr_ich1_mpegnr2_reg.cp_mpegenable_x = ptr->S_MPEGNR_16_Function_Enable.enable_y;
	//_reg.cp_ = ptr->S_MPEGNR_16_Function_Enable.enable_c;

	mpegnr_ich1_blend_nr_hor_step_reg.cp_mpghor_blend_xstep = ptr->S_MPEGNR_16_Region_Separation.scaling;
	mpegnr_ich1_blend_nr_hor_step_reg.cp_mpghor_blend_xini = ptr->S_MPEGNR_16_Region_Separation.offset_pixel;
	mpegnr_ich1_mpegnr2_reg.cp_mpgidx_x = ptr->S_MPEGNR_16_Region_Separation.offset_step;
	// removed mpegnr_ich1_mpegnr2_reg.cp_mpgidxmode_x = ptr->S_MPEGNR_16_Region_Separation.small_block;
	// removed mpegnr_ich1_blend_nr_hor_step_reg.cp_mpghor_blend_range = ptr->S_MPEGNR_16_Region_Separation.skip_some_regions;
	//_reg.cp_ = ptr->S_MPEGNR_16_Region_Separation.after_interlace;

	if (!skipFWreg) mpegnr_ich1_mpegnr1_reg.cp_mpgupperbound_x = ptr->S_MPEGNR_16_Upperbound._0;

	mpegnr_ich1_mpegnr_ub_dif_cmp_reg.cp_mpgtex_range = ptr->S_MPEGNR_16_Flat_and_Dec.see_range;
	if (!skipFWreg) mpegnr_ich1_mpegnr1_reg.cp_mpgupperbound_offset = ptr->S_MPEGNR_16_Flat_and_Dec.th_high;
	if (!skipFWreg) mpegnr_ich1_mpegnr1_reg.cp_mpgupperbound_gain = ptr->S_MPEGNR_16_Flat_and_Dec.gain_high;
	//_reg.cp_ = ptr->S_MPEGNR_16_Flat_and_Dec.Reserved_03;
	mpegnr_ich1_mpegnr1_reg.cp_mpgub_dec_sel = ptr->S_MPEGNR_16_Flat_and_Dec.dec_sel;
	mpegnr_ich1_mpegnr_ub_delta1_reg.cp_mpgub_delta1 = ptr->S_MPEGNR_16_Flat_and_Dec.delta1;
	mpegnr_ich1_mpegnr_ub_delta1_reg.cp_mpgub_delta2 = ptr->S_MPEGNR_16_Flat_and_Dec.delta2;
	mpegnr_ich1_mpegnr_ub_delta2_reg.cp_mpgub_delta3 = ptr->S_MPEGNR_16_Flat_and_Dec.delta3;

	mpegnr_ich1_mpegnr1_reg.cp_mpgub_difcmp_en = ptr->S_MPEGNR_16_Difcmp.see_range;
	mpegnr_ich1_mpegnr_ub_dif_cmp_reg.cp_mpgub_difcmp_offset = ptr->S_MPEGNR_16_Difcmp.th_high;
	mpegnr_ich1_mpegnr_ub_dif_cmp_reg.cp_mpgub_difcmp_gain = ptr->S_MPEGNR_16_Difcmp.gain_high;

	mpegnr_ich1_mpegnr1_reg.cp_mpgub_difcen_en = ptr->S_MPEGNR_16_Difcen.see_range;
	mpegnr_ich1_mpegnr_ub_cen_th_reg.cp_mpghor_diff_th_high = ptr->S_MPEGNR_16_Difcen.th_high;
	mpegnr_ich1_mpegnr_ub_cen_gain_reg.cp_mpghor_diff_gain_high = ptr->S_MPEGNR_16_Difcen.gain_high;
	mpegnr_ich1_mpegnr_ub_cen_th_reg.cp_mpghor_diff_th_low = ptr->S_MPEGNR_16_Difcen.th_low;
	mpegnr_ich1_mpegnr_ub_cen_gain_reg.cp_mpghor_diff_gain_low = ptr->S_MPEGNR_16_Difcen.gain_low;

	if (!skipFWreg) mpegnr_ich1_mpegnr1_reg.cp_mpgnr_filter_sel = ptr->S_MPEGNR_16_LPF.lpf_sel;
	mpegnr_ich1_mpegnr2_reg.cp_mpegresultweight_x = ptr->S_MPEGNR_16_LPF.result_weight;
	// mpegnr_ich1_mpegnr2_reg.cp_mpegresultweight_x =7; //fixed me for demo
	IoReg_Write32(	MPEGNR_ICH1_blend_NR_Hor_step_reg	,	mpegnr_ich1_blend_nr_hor_step_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_MPEGNR1_reg	,	mpegnr_ich1_mpegnr1_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_MPEGNR_UB_delta1_reg	,	mpegnr_ich1_mpegnr_ub_delta1_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_MPEGNR_UB_delta2_reg	,	mpegnr_ich1_mpegnr_ub_delta2_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_MPEGNR_UB_Cen_Th_reg	,	mpegnr_ich1_mpegnr_ub_cen_th_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_MPEGNR2_reg	,	mpegnr_ich1_mpegnr2_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_MPEGNR_UB_Cen_Gain_reg	,	mpegnr_ich1_mpegnr_ub_cen_gain_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_MPEGNR_UB_Dif_Cmp_reg	,	mpegnr_ich1_mpegnr_ub_dif_cmp_reg	.regValue);
	// computed_by_HW IoReg_Write32(	MPEGNR_ICH1_Hist_Blking_Hor_Range_reg	,	mpegnr_ich1_hist_blking_hor_range_reg	.regValue);
	// computed_by_HW IoReg_Write32(	MPEGNR_ICH1_Hist_Blking_Ver_Range_reg	,	mpegnr_ich1_hist_blking_ver_range_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_MPEGStart_reg	,	mpegnr_ich1_mpegstart_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_det_Hist_Hor_step_reg	,	mpegnr_ich1_det_hist_hor_step_reg	.regValue);
	IoReg_Write32(	MPEGNR_ICH1_det_Hist_Hor_thd_reg	,	mpegnr_ich1_det_hist_hor_thd_reg	.regValue);

	//db apply
	IoReg_SetBits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_apply_mask);

}
void drvif_color_set_DRV_MPG_H_SW_Ctrl(DRV_MPEGNR_16_SW_CTRL *ptr)
{
	if( ptr ==NULL)
		return;

        memcpy((unsigned char *)&S_MPEGNR_SW_ctrl_H, (unsigned char *)ptr, sizeof(DRV_MPEGNR_16_SW_CTRL));

}
void drvif_color_get_DRV_MPG_H_SW_Ctrl(DRV_MPEGNR_16_SW_CTRL *ptr)
{
	if( ptr ==NULL)
		return;

        memcpy((unsigned char *)ptr, (DRV_MPEGNR_16_SW_CTRL *)&S_MPEGNR_SW_ctrl_H, sizeof(DRV_MPEGNR_16_SW_CTRL));

}
void drvif_color_set_DRV_MPG_V_SW_Ctrl(DRV_MPEGNR_16_SW_CTRL *ptr)
{
	if( ptr ==NULL)
		return;
        memcpy((unsigned char *)&S_MPEGNR_SW_ctrl_V, (unsigned char *)ptr, sizeof(DRV_MPEGNR_16_SW_CTRL));
}
void drvif_color_get_DRV_MPG_V_SW_Ctrl(DRV_MPEGNR_16_SW_CTRL *ptr)
{
	if( ptr ==NULL)
		return;

	memcpy((unsigned char *)ptr, (DRV_MPEGNR_16_SW_CTRL *)&S_MPEGNR_SW_ctrl_V, sizeof(DRV_MPEGNR_16_SW_CTRL));
}

void drvif_color_DRV_MPG_V(DRV_MPEGNR_16_Table *ptr)
{

	nr_dch1_mpegnr1_RBUS	nr_dch1_mpegnr1_reg	;
	nr_dch1_mpegnr2_RBUS	nr_dch1_mpegnr2_reg	;
	nr_dch1_mpegnr3_RBUS	nr_dch1_mpegnr3_reg	;
	nr_dch1_vist_blking_range_RBUS	nr_dch1_vist_blking_range_reg	;
	nr_dch1_det_hist_vor_thd_hlh_RBUS	nr_dch1_det_hist_vor_thd_hlh_reg	;
	nr_dch1_mpegstart_RBUS	nr_dch1_mpegstart_reg	;
	nr_dch1_det_vist_hor_step_RBUS	nr_dch1_det_vist_hor_step_reg	;
	nr_dch1_det_hist_hor_thd_RBUS	nr_dch1_det_hist_hor_thd_reg	;
	nr_dch1_blend_nr_ver_step_RBUS	nr_dch1_blend_nr_ver_step_reg	;
	nr_dch1_mpegnr_ub_delta1_RBUS	nr_dch1_mpegnr_ub_delta1_reg	;
	nr_dch1_mpegnr_ub_delta2_RBUS	nr_dch1_mpegnr_ub_delta2_reg	;
	nr_dch1_mpegnr_diff_th_RBUS	nr_dch1_mpegnr_diff_th_reg	;
	nr_dch1_mpegnr_diff_gain_RBUS	nr_dch1_mpegnr_diff_gain_reg	;
	nr_dch1_mpegnr_diff_cmp_RBUS	nr_dch1_mpegnr_diff_cmp_reg	;

	nr_dch1_mpegnr1_reg	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR1_reg	);
	nr_dch1_mpegnr2_reg	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR2_reg	);
	nr_dch1_mpegnr3_reg	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR3_reg	);
	nr_dch1_vist_blking_range_reg	.regValue=IoReg_Read32(	NR_DCH1_Vist_Blking_Range_reg	);
	nr_dch1_det_hist_vor_thd_hlh_reg	.regValue=IoReg_Read32(	NR_DCH1_det_Hist_Vor_thd_hlh_reg);
	nr_dch1_mpegstart_reg	.regValue=IoReg_Read32(	NR_DCH1_MPEGStart_reg	);
	nr_dch1_det_vist_hor_step_reg	.regValue=IoReg_Read32(	NR_DCH1_det_Vist_Hor_step_reg	);
	nr_dch1_det_hist_hor_thd_reg	.regValue=IoReg_Read32(	NR_DCH1_det_Hist_Hor_thd_reg	);
	nr_dch1_blend_nr_ver_step_reg	.regValue=IoReg_Read32(	NR_DCH1_blend_NR_Ver_step_reg	);
	nr_dch1_mpegnr_ub_delta1_reg	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR_UB_delta1_reg	);
	nr_dch1_mpegnr_ub_delta2_reg	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR_UB_delta2_reg	);
	nr_dch1_mpegnr_diff_th_reg	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR_Diff_Th_reg	);
	nr_dch1_mpegnr_diff_gain_reg	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR_Diff_Gain_reg	);
	nr_dch1_mpegnr_diff_cmp_reg	.regValue=IoReg_Read32(	NR_DCH1_MPEGNR_Diff_Cmp_reg	);

	nr_dch1_mpegstart_reg.cp_vistveren = ptr->S_MPEGNR_16_Detector.enable;
	//_reg.cp_ = ptr->S_MPEGNR_16_Detector.Reserved_01;
	//_reg.cp_ = ptr->S_MPEGNR_16_Detector.diff_th_combo;
	//_reg.cp_ = ptr->S_MPEGNR_16_Detector.cp_histhor_diff_th_hlh;
	// computed_by_HW nr_dch1_vist_blking_hor_range_reg.cp_visthorstart = ptr->S_MPEGNR_16_Detector.bound_left;
	// computed_by_HW nr_dch1_vist_blking_hor_range_reg.cp_visthorend = ptr->S_MPEGNR_16_Detector.bound_right;
	// computed_by_HW nr_dch1_vist_blking_ver_range_reg.cp_vistverstart = ptr->S_MPEGNR_16_Detector.bound_up;
	// computed_by_HW nr_dch1_vist_blking_ver_range_reg.cp_vistverend = ptr->S_MPEGNR_16_Detector.bound_down;
	nr_dch1_det_hist_hor_thd_reg.cp_visthor_diff_th_low = ptr->S_MPEGNR_16_Detector.diff_th_low;
	nr_dch1_det_hist_hor_thd_reg.cp_visthor_diff_th_high = ptr->S_MPEGNR_16_Detector.diff_th_high;
	nr_dch1_det_vist_hor_step_reg.cp_visthor_det_ystep = ptr->S_MPEGNR_16_Detector.scaling;
	nr_dch1_det_vist_hor_step_reg.cp_visthor_det_yini = ptr->S_MPEGNR_16_Detector.offset_pixel;
	nr_dch1_det_vist_hor_step_reg.cp_visthor_det_offset = ptr->S_MPEGNR_16_Detector.offset_step;
	nr_dch1_mpegstart_reg.cp_vistveroffset = ptr->S_MPEGNR_16_Detector.offset_forgotten;
	nr_dch1_mpegstart_reg.cp_vist2step = ptr->S_MPEGNR_16_Detector.subsampling_mode;

	nr_dch1_mpegnr1_reg.cp_mpegenable_y_y = ptr->S_MPEGNR_16_Function_Enable.enable_y;
	nr_dch1_mpegnr1_reg.cp_mpegenable_y_c = ptr->S_MPEGNR_16_Function_Enable.enable_c;

	nr_dch1_blend_nr_ver_step_reg.cp_mpgver_blend_ystep = ptr->S_MPEGNR_16_Region_Separation.scaling;
	nr_dch1_blend_nr_ver_step_reg.cp_mpgver_blend_yini = ptr->S_MPEGNR_16_Region_Separation.offset_pixel;
	nr_dch1_mpegnr1_reg.cp_mpgidx_y = ptr->S_MPEGNR_16_Region_Separation.offset_step;
	//_reg.cp_ = ptr->S_MPEGNR_16_Region_Separation.small_block;
	// removed nr_dch1_mpegnr1_reg.cp_mpgidxmode_y = ptr->S_MPEGNR_16_Region_Separation.skip_some_regions;
	nr_dch1_mpegnr3_reg.cp_mpgver_blend_mpegai = ptr->S_MPEGNR_16_Region_Separation.after_interlace;

	nr_dch1_mpegnr1_reg.cp_mpgupperbound = ptr->S_MPEGNR_16_Upperbound._0;

	nr_dch1_mpegnr2_reg.cp_mpgdiffsum_range = ptr->S_MPEGNR_16_Flat_and_Dec.see_range;
	nr_dch1_mpegnr2_reg.cp_mpgupperbound_offset = ptr->S_MPEGNR_16_Flat_and_Dec.th_high;
	nr_dch1_mpegnr2_reg.cp_mpgupperbound_gain = ptr->S_MPEGNR_16_Flat_and_Dec.gain_high;
	//_reg.cp_ = ptr->S_MPEGNR_16_Flat_and_Dec.Reserved_03;
	nr_dch1_mpegnr2_reg.cp_mpgub_dec_sel = ptr->S_MPEGNR_16_Flat_and_Dec.dec_sel;
	nr_dch1_mpegnr_ub_delta1_reg.cp_mpgub_delta1 = ptr->S_MPEGNR_16_Flat_and_Dec.delta1;
	nr_dch1_mpegnr_ub_delta1_reg.cp_mpgub_delta2 = ptr->S_MPEGNR_16_Flat_and_Dec.delta2;
	nr_dch1_mpegnr_ub_delta2_reg.cp_mpgub_delta3 = ptr->S_MPEGNR_16_Flat_and_Dec.delta3;

	nr_dch1_mpegnr2_reg.cp_mpgub_difcmp_en = ptr->S_MPEGNR_16_Difcmp.see_range;
	nr_dch1_mpegnr_diff_cmp_reg.cp_mpgub_difcmp_offset = ptr->S_MPEGNR_16_Difcmp.th_high;
	nr_dch1_mpegnr_diff_cmp_reg.cp_mpgub_difcmp_gain = ptr->S_MPEGNR_16_Difcmp.gain_high;

	nr_dch1_mpegnr2_reg.cp_mpgub_difcen_en = ptr->S_MPEGNR_16_Difcen.see_range;
	nr_dch1_mpegnr_diff_th_reg.cp_mpgver_diff_th_high = ptr->S_MPEGNR_16_Difcen.th_high;
	nr_dch1_mpegnr_diff_gain_reg.cp_mpgver_diff_gain_high = ptr->S_MPEGNR_16_Difcen.gain_high;
	nr_dch1_mpegnr_diff_th_reg.cp_mpgver_diff_th_low = ptr->S_MPEGNR_16_Difcen.th_low;
	nr_dch1_mpegnr_diff_gain_reg.cp_mpgver_diff_gain_low = ptr->S_MPEGNR_16_Difcen.gain_low;

	nr_dch1_mpegnr1_reg.cp_mpglpmode = ptr->S_MPEGNR_16_LPF.lpf_sel;
	nr_dch1_mpegnr1_reg.cp_mpegresultweight_y = ptr->S_MPEGNR_16_LPF.result_weight;

	IoReg_Write32(	NR_DCH1_MPEGNR1_reg	,	nr_dch1_mpegnr1_reg	.regValue);
	IoReg_Write32(	NR_DCH1_MPEGNR2_reg	,	nr_dch1_mpegnr2_reg	.regValue);
	IoReg_Write32(	NR_DCH1_MPEGNR3_reg	,	nr_dch1_mpegnr3_reg	.regValue);
	// computed_by_HW IoReg_Write32(	NR_DCH1_Vist_Blking_Range_reg	,	nr_dch1_vist_blking_range_reg	.regValue);
	IoReg_Write32(	NR_DCH1_det_Hist_Vor_thd_hlh_reg	,	nr_dch1_det_hist_vor_thd_hlh_reg	.regValue);
	IoReg_Write32(	NR_DCH1_MPEGStart_reg	,	nr_dch1_mpegstart_reg	.regValue);
	IoReg_Write32(	NR_DCH1_det_Vist_Hor_step_reg	,	nr_dch1_det_vist_hor_step_reg	.regValue);
	IoReg_Write32(	NR_DCH1_det_Hist_Hor_thd_reg	,	nr_dch1_det_hist_hor_thd_reg	.regValue);
	IoReg_Write32(	NR_DCH1_blend_NR_Ver_step_reg	,	nr_dch1_blend_nr_ver_step_reg	.regValue);
	IoReg_Write32(	NR_DCH1_MPEGNR_UB_delta1_reg	,	nr_dch1_mpegnr_ub_delta1_reg	.regValue);
	IoReg_Write32(	NR_DCH1_MPEGNR_UB_delta2_reg	,	nr_dch1_mpegnr_ub_delta2_reg	.regValue);
	IoReg_Write32(	NR_DCH1_MPEGNR_Diff_Th_reg	,	nr_dch1_mpegnr_diff_th_reg	.regValue);
	IoReg_Write32(	NR_DCH1_MPEGNR_Diff_Gain_reg	,	nr_dch1_mpegnr_diff_gain_reg	.regValue);
	IoReg_Write32(	NR_DCH1_MPEGNR_Diff_Cmp_reg	,	nr_dch1_mpegnr_diff_cmp_reg	.regValue);

	//db apply
	IoReg_SetBits(NR_V_MPEGNR_DB_CTRL_reg, NR_V_MPEGNR_DB_CTRL_mpegnr_db_apply_mask); // DB apply
}

#if 0
void drvif_color_MPEGNR_flow(DRV_NR_Item *ptr)
{
	if (!ptr) {
		VIPprintf("drvif_color_MPEGNR_flow table to NULL\n");
		return;
	}

	drvif_color_DRV_MPG_General_ctrl(&(ptr->S_MPG_TABLE.S_MPEGNR_General_ctrl));
	drvif_color_DRV_MPG_H(&(ptr->S_MPG_TABLE.S_MPEGNR_H));
	drvif_color_DRV_MPG_V(&(ptr->S_MPG_TABLE.S_MPEGNR_V));
}
#endif

void drvif_IESM_format_convert(void)// Merlin4 add
{

	iedge_smooth_format_convert_RBUS edsm_format_convert_reg;

	edsm_format_convert_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_FORMAT_CONVERT_reg);

	// mac6p: i&p mode after rtnr/di is yuv444
	if((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI))// spnr cbypass mode
	{
		edsm_format_convert_reg.main_444_to_422_en = 0;
		edsm_format_convert_reg.main_422_to_444_en = 0;
		edsm_format_convert_reg.main_444_to_422_type = 0;
		edsm_format_convert_reg.main_422_to_444_type = 0;
	}
	else
	{
		edsm_format_convert_reg.main_444_to_422_en = 1;
		edsm_format_convert_reg.main_422_to_444_en = 1;
		edsm_format_convert_reg.main_444_to_422_type = 0;
		edsm_format_convert_reg.main_422_to_444_type = 0;
	}

#ifdef CONFIG_I2RND_ENABLE
//TBD
#else
#ifdef CONFIG_DUAL_CHANNEL
	if(Get_sub_from_DI_mux_path() && (Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE) == FALSE))
	{
		edsm_format_convert_reg.sub_422_to_444_en = 0;
	}
	else if(Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_THRIP) == TRUE)// merlin4: after di path is 422
	{
		edsm_format_convert_reg.sub_422_to_444_en = 1;
	}
	else // merlin4: pass di path is 444
	{
		edsm_format_convert_reg.sub_422_to_444_en = 0;
	}
#endif
#endif

	IoReg_Write32(IEDGE_SMOOTH_FORMAT_CONVERT_reg, edsm_format_convert_reg.regValue);

	IoReg_SetBits(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_apply_mask); /* DB apply */

	NRPprintf_Info("iesm_format:main(imode=%d, rtnr=%d, %d->%d, type:%d->%d) sub(imode=%d, rtnr=%d, %d)\n ",
		Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP),
		Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_RTNR),
		edsm_format_convert_reg.main_444_to_422_en, edsm_format_convert_reg.main_422_to_444_en,
		edsm_format_convert_reg.main_444_to_422_type, edsm_format_convert_reg.main_422_to_444_type,
		Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_THRIP),
		Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_RTNR),
		edsm_format_convert_reg.sub_422_to_444_en);

	IoReg_SetBits(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_apply_mask); /* DB apply */
}

void drvif_IESM_frontporch_set(unsigned char game_mode_check_flag)
{
	UINT8 front_porch_num_th;
	int front_porch_num, linemode_limit, I_width, I_height;
	int fmt_422to444;
	iedge_smooth_format_convert_RBUS edsm_format_convert_reg;
	iedge_smooth_edsm_db_ctrl_RBUS edsm_db_ctrl_reg;
	iedge_smooth_edgesmooth_ex_ctrl_RBUS edgesmooth_ex_ctrl_reg;

	#if 0 // k6 add
	_RPC_system_setting_info *rpc_vip_sysInfo = NULL;
	unsigned char isGameMode = 255;
	#endif

	front_porch_num = Scaler_DispGetInputInfo(SLR_INPUT_V_LEN)- Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)- Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);

	#if 0 // k6 add
	rpc_vip_sysInfo = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (rpc_vip_sysInfo != NULL) {
		if (game_mode_check_flag == 1) {
			isGameMode = rpc_vip_sysInfo->Low_Delay_Status[PQ_Low_Delay_Mode];
		} else {
			isGameMode = 0;
			rpc_vip_sysInfo->Low_Delay_Status[PQ_Low_Delay_Mode] = 0; // reset low delay flag
		}
	}
	#endif

	I_width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
	I_height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);// Merlin4 add

	edsm_format_convert_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_FORMAT_CONVERT_reg);// Merlin4 add
	fmt_422to444 = edsm_format_convert_reg.main_422_to_444_en;

	if(_TRUE == Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP))
	{
		front_porch_num*=2;
	}

	/*printk(KERN_EMERG "[minchay][%s]V_LEN =%d ,LEN_PRE=%d ,ACT_STA_PRE=%d \n",__FUNCTION__,Scaler_DispGetInputInfo(SLR_INPUT_V_LEN),Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE),
		Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE));
	printk(KERN_EMERG "[minchay][%s]front_porch_num = %d I_width=%d\n",__FUNCTION__,front_porch_num, I_width);*/


#if 0 // Merlin series, for 17 line
	front_porch_num_th = 9;

	if(front_porch_num >= front_porch_num_th && I_width <= 2048 && I_height>8)
		linemode_limit = 3; // 17 line mode
	else if(front_porch_num >= 6 && (fmt_422to444==1 || I_width <= 2048) && I_height>5)
		linemode_limit = 2; // 11 line mode
	else if(front_porch_num >= 3 && I_height>2)
		linemode_limit = 1; // 5 line mode
	else
		linemode_limit = 0; // 3 line mode

	#if 0 // k6 add
	if (isGameMode == 1)
		linemode_limit = 1;
	#endif

#else // Mac series, for 11 line

	front_porch_num_th = 6;

	if (fwif_vip_source_check(3, NOT_BY_DISPLAY) == VIP_QUALITY_HDMI_1080P)
		front_porch_num_th = 5;

	if(front_porch_num >= front_porch_num_th && I_width <= 2048 && (fmt_422to444==1))
		linemode_limit = 2; // 11 line mode
	else if(front_porch_num >= 3)
		linemode_limit = 1; // 5 line mode
	else
		linemode_limit = 0; // 3 line mode

#endif

	edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	edsm_db_ctrl_reg.edsm_db_en = 1;
	edsm_db_ctrl_reg.edsm_db_rd_sel = 1;
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);

	edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg);
	edgesmooth_ex_ctrl_reg.lbuf_linemode = linemode_limit;
	IoReg_Write32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg,edgesmooth_ex_ctrl_reg.regValue);
	
	edsm_db_ctrl_reg.edsm_db_apply = 1;
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);

#if 0 // k6 add 
	rtd_pr_vpq_info("front_porch_num=vlen-actlen-actsta: %d=%d-%d-%d, game_mode=%d,size=%d-%d,flag=%d,lineMode=%d,fmt_422to444=%d,\n",
		front_porch_num, Scaler_DispGetInputInfo(SLR_INPUT_V_LEN), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE),
		isGameMode, I_width, I_height, game_mode_check_flag, linemode_limit, fmt_422to444);
#endif

	rtd_pr_vpq_info("front_porch_num=vlen-actlen-actsta: %d=%d-%d-%d, size=%d*%d,lineMode=%d,fmt_422to444=%d\n",
		front_porch_num, Scaler_DispGetInputInfo(SLR_INPUT_V_LEN), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE),
		I_width, I_height, linemode_limit, fmt_422to444);

	//NRPprintf_Info("linemode/front_porch,h/w/fmt: 17/>=9/<=2048, 11/>=6/422to444 or <=2048, 5/>=3/x, 3/else/x\n");
	//NRPprintf_Info("front_porch_num=vlen-actlen-actsta: %d=%d-%d-%d\n", Scaler_DispGetInputInfo(SLR_INPUT_V_LEN)- Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)-Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE),
	//Scaler_DispGetInputInfo(SLR_INPUT_V_LEN), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE));
	//NRPprintf_Info("limit_linemode: %d, front_porch: %d, in_w: %d, in_h:%d, 422to444:%d\n", linemode_limit, front_porch_num, I_width, I_height, fmt_422to444);

}

#if 0 //mac6p removed IESM DLTI2D
void drvif_color_DRV_DLTI_2D(DRV_DLTI_2D *ptr)
{
	iedge_smooth_dlti2d_ctrl_RBUS dlti2d_ctrl_reg;
	dlti2d_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_DLTI2D_CTRL_reg);

	dlti2d_ctrl_reg.dlti2d_gain = ptr->DLTI_2D_Gain;
	dlti2d_ctrl_reg.dlti2d_maxmin_en = ptr->DLTI_2D_MaxMin_En;
	dlti2d_ctrl_reg.dlti2d_chaos_gain = ptr->DLTI_2D_Chaos_Gain;
	dlti2d_ctrl_reg.dlti2d_vlpf_type = ptr->DLTI_2D_VLPF;
	dlti2d_ctrl_reg.dlti2d_hlpf_type = ptr->DLTI_2D_HLPF;

	IoReg_Write32(IEDGE_SMOOTH_DLTI2D_CTRL_reg, dlti2d_ctrl_reg.regValue);
}
#endif

void drvif_color_DRV_MA_ISNR_GetMotion(DRV_MA_ISNR_GetMotion *ptr)
{
	di_rtnr_ma_isnr_getmotion_main_RBUS getMotion_main_reg;
	di_rtnr_ma_isnr_getmotion_edge_th_RBUS getMotion_edge_th;
	di_rtnr_ma_isnr_getmotion_edgeslopenh_RBUS getMotion_edgeslopenh;
	di_rtnr_ma_isnr_getmotion_edgeslopemn_RBUS getMotion_edgeslopemh;
	di_rtnr_ma_isnr_getmotion_edgeslopelm_RBUS getMotion_edgeslopelm;
	di_rtnr_ma_isnr_getmotion_motionth_high_RBUS getMotion_motionh_high;
	di_rtnr_ma_isnr_getmotion_motionth_mid2_RBUS getMotion_motionh_mid2;
	di_rtnr_ma_isnr_getmotion_motionth_mid_RBUS getMotion_motionh_mid;
	di_rtnr_ma_isnr_getmotion_motionth_low_RBUS getMotion_motionh_low;
	unsigned int slope1_lm = 0, slope1_mn = 0, slope1_nh = 0, slope2_lm = 0, slope2_mn = 0, slope2_nh = 0, slope3_lm = 0, slope3_mn = 0, slope3_nh = 0;

	getMotion_main_reg.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_MAIN_reg);
	getMotion_edge_th.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_EDGE_TH_reg);
	getMotion_edgeslopenh.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_EDGESLOPENH_reg);
	getMotion_edgeslopemh.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_EDGESLOPEMN_reg);
	getMotion_edgeslopelm.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_EDGESLOPELM_reg);
	getMotion_motionh_high.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_MOTIONTH_HIGH_reg);
	getMotion_motionh_mid2.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_MOTIONTH_MID2_reg);
	getMotion_motionh_mid.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_MOTIONTH_MID_reg);
	getMotion_motionh_low.regValue = IoReg_Read32(DI_RTNR_MA_ISNR_GETMOTION_MOTIONTH_LOW_reg);

	getMotion_main_reg.rtnr_isnr_getmotion_en = ptr->RTNR_isnr_Get_Motion_En;
	getMotion_main_reg.rtnr_ma_isnr_getmotion_edgemask = ptr->MA_isnr_Get_Motion_EdgeMask;
	getMotion_main_reg.rtnr_ma_isnr_getmotion_ydiffmask = ptr->MA_isnr_Get_Motion_YDiffMask;
	getMotion_main_reg.rtnr_ma_isnr_getmotion_motionoffset = ptr->MA_isnr_Get_Motion_MotionOffset;
	getMotion_edge_th.rtnr_ma_isnr_getmotion_edgeth1 = ptr->MA_isnr_Get_Motion_EdgeTh_1;
	getMotion_edge_th.rtnr_ma_isnr_getmotion_edgeth2 = ptr->MA_isnr_Get_Motion_EdgeTh_2;
	getMotion_edge_th.rtnr_ma_isnr_getmotion_edgeth3 = ptr->MA_isnr_Get_Motion_EdgeTh_3;
	getMotion_edge_th.rtnr_ma_isnr_getmotion_edgeth4 = ptr->MA_isnr_Get_Motion_EdgeTh_4;

	getMotion_motionh_high.rtnr_ma_isnr_getmotion_motionth1_h = ptr->MA_isnr_Get_Motion_MotionTh_H_1;
	getMotion_motionh_high.rtnr_ma_isnr_getmotion_motionth2_h = ptr->MA_isnr_Get_Motion_MotionTh_H_2;
	getMotion_motionh_high.rtnr_ma_isnr_getmotion_motionth3_h = ptr->MA_isnr_Get_Motion_MotionTh_H_3;

	getMotion_motionh_mid2.rtnr_ma_isnr_getmotion_motionth1_n = ptr->MA_isnr_Get_Motion_MotionTh_N_1;
	getMotion_motionh_mid2.rtnr_ma_isnr_getmotion_motionth2_n = ptr->MA_isnr_Get_Motion_MotionTh_N_2;
	getMotion_motionh_mid2.rtnr_ma_isnr_getmotion_motionth3_n = ptr->MA_isnr_Get_Motion_MotionTh_N_3;

	getMotion_motionh_mid.rtnr_ma_isnr_getmotion_motionth1_m = ptr->MA_isnr_Get_Motion_MotionTh_M_1;
	getMotion_motionh_mid.rtnr_ma_isnr_getmotion_motionth2_m = ptr->MA_isnr_Get_Motion_MotionTh_M_2;
	getMotion_motionh_mid.rtnr_ma_isnr_getmotion_motionth3_m = ptr->MA_isnr_Get_Motion_MotionTh_M_3;

	getMotion_motionh_low.rtnr_ma_isnr_getmotion_motionth1_l = ptr->MA_isnr_Get_Motion_MotionTh_L_1;
	getMotion_motionh_low.rtnr_ma_isnr_getmotion_motionth2_l = ptr->MA_isnr_Get_Motion_MotionTh_L_2;
	getMotion_motionh_low.rtnr_ma_isnr_getmotion_motionth3_l = ptr->MA_isnr_Get_Motion_MotionTh_L_3;

	//slope1_lm = (ptr->MA_isnr_Get_Motion_MotionTh_M_1 - ptr->MA_isnr_Get_Motion_MotionTh_L_1)*16/(ptr->MA_isnr_Get_Motion_EdgeTh_2 - ptr->MA_isnr_Get_Motion_EdgeTh_1)+0.5;

	if (ptr->MA_isnr_Get_Motion_EdgeTh_2 != ptr->MA_isnr_Get_Motion_EdgeTh_1) {
		slope1_lm = ((ptr->MA_isnr_Get_Motion_MotionTh_M_1 - ptr->MA_isnr_Get_Motion_MotionTh_L_1)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_2 - ptr->MA_isnr_Get_Motion_EdgeTh_1)+1) >>1;
		slope2_lm = ((ptr->MA_isnr_Get_Motion_MotionTh_M_2 - ptr->MA_isnr_Get_Motion_MotionTh_L_2)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_2 - ptr->MA_isnr_Get_Motion_EdgeTh_1)+1) >>1;
		slope3_lm = ((ptr->MA_isnr_Get_Motion_MotionTh_M_3 - ptr->MA_isnr_Get_Motion_MotionTh_L_3)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_2 - ptr->MA_isnr_Get_Motion_EdgeTh_1)+1) >>1;
	}
	if (ptr->MA_isnr_Get_Motion_EdgeTh_3 != ptr->MA_isnr_Get_Motion_EdgeTh_2) {
		slope1_mn = ((ptr->MA_isnr_Get_Motion_MotionTh_N_1 - ptr->MA_isnr_Get_Motion_MotionTh_M_1)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_3 - ptr->MA_isnr_Get_Motion_EdgeTh_2)+1) >>1;
		slope2_mn = ((ptr->MA_isnr_Get_Motion_MotionTh_N_2 - ptr->MA_isnr_Get_Motion_MotionTh_M_2)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_3 - ptr->MA_isnr_Get_Motion_EdgeTh_2)+1) >>1;
		slope3_mn = ((ptr->MA_isnr_Get_Motion_MotionTh_N_3 - ptr->MA_isnr_Get_Motion_MotionTh_M_3)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_3 - ptr->MA_isnr_Get_Motion_EdgeTh_2)+1) >>1;
	}
	if (ptr->MA_isnr_Get_Motion_EdgeTh_4 != ptr->MA_isnr_Get_Motion_EdgeTh_3) {
		slope1_nh = ((ptr->MA_isnr_Get_Motion_MotionTh_H_1 - ptr->MA_isnr_Get_Motion_MotionTh_N_1)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_4 - ptr->MA_isnr_Get_Motion_EdgeTh_3)+1) >>1;
		slope2_nh = ((ptr->MA_isnr_Get_Motion_MotionTh_H_2 - ptr->MA_isnr_Get_Motion_MotionTh_N_2)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_4 - ptr->MA_isnr_Get_Motion_EdgeTh_3)+1) >>1;
		slope3_nh = ((ptr->MA_isnr_Get_Motion_MotionTh_H_3 - ptr->MA_isnr_Get_Motion_MotionTh_N_3)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_4 - ptr->MA_isnr_Get_Motion_EdgeTh_3)+1) >>1;
	}

	getMotion_edgeslopenh.rtnr_ma_isnr_getmotion_edgeslope1_nh = CLIP(0, 255, slope1_nh);
	getMotion_edgeslopenh.rtnr_ma_isnr_getmotion_edgeslope2_nh = CLIP(0, 255, slope2_nh);
	getMotion_edgeslopenh.rtnr_ma_isnr_getmotion_edgeslope3_nh = CLIP(0, 255, slope3_nh);

	getMotion_edgeslopemh.rtnr_ma_isnr_getmotion_edgeslope1_mn = CLIP(0, 255, slope1_mn);
	getMotion_edgeslopemh.rtnr_ma_isnr_getmotion_edgeslope2_mn = CLIP(0, 255, slope2_mn);
	getMotion_edgeslopemh.rtnr_ma_isnr_getmotion_edgeslope3_mn = CLIP(0, 255, slope3_mn);

	getMotion_edgeslopelm.rtnr_ma_isnr_getmotion_edgeslope1_lm = CLIP(0, 255, slope1_lm);
	getMotion_edgeslopelm.rtnr_ma_isnr_getmotion_edgeslope2_lm = CLIP(0, 255, slope2_lm);
	getMotion_edgeslopelm.rtnr_ma_isnr_getmotion_edgeslope3_lm = CLIP(0, 255, slope3_lm);

	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_MAIN_reg, getMotion_main_reg.regValue);
	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_EDGE_TH_reg, getMotion_edge_th.regValue);
	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_EDGESLOPENH_reg, getMotion_edgeslopenh.regValue);
	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_EDGESLOPEMN_reg, getMotion_edgeslopemh.regValue);
	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_EDGESLOPELM_reg, getMotion_edgeslopelm.regValue);
	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_MOTIONTH_HIGH_reg, getMotion_motionh_high.regValue);
	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_MOTIONTH_MID2_reg, getMotion_motionh_mid2.regValue);
	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_MOTIONTH_MID_reg, getMotion_motionh_mid.regValue);
	IoReg_Write32(DI_RTNR_MA_ISNR_GETMOTION_MOTIONTH_LOW_reg, getMotion_motionh_low.regValue);


}

void drvif_color_DRV_MA_SNR_Motion_Weight1(DRV_MA_SNR_Motion_Weight1 *ptr)
{
	// add for Merlin2 (jyyang 20160620)
	nr_motion_weight_ctrl_RBUS nr_motion_weight_ctrl_reg;
	nr_motion_weight1_1_RBUS nr_motion_weight1_1_reg;
	nr_motion_weight1_2_RBUS nr_motion_weight1_2_reg;
	nr_motion_weight1_3_RBUS nr_motion_weight1_3_reg;
	nr_motion_weight1_4_RBUS nr_motion_weight1_4_reg;

	nr_motion_weight_ctrl_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT_ctrl_reg);
	nr_motion_weight1_1_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT1_1_reg);
	nr_motion_weight1_2_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT1_2_reg);
	nr_motion_weight1_3_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT1_3_reg);
	nr_motion_weight1_4_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT1_4_reg);

	nr_motion_weight_ctrl_reg.nr_motion_en = ptr->nr_motion_en;
	#if 0//merlin3
	nr_motion_weight_ctrl_reg.vc_nr_motion_signwt1 = ptr->nr_motion_signwt1;
	nr_motion_weight_ctrl_reg.vc_nr_motion_signwt2 = ptr->nr_motion_signwt2;
	#else//mac6
	nr_motion_weight_ctrl_reg.nr_motion_signwt1 = ptr->nr_motion_signwt1;
	nr_motion_weight_ctrl_reg.nr_motion_signwt2 = ptr->nr_motion_signwt2;
	#endif
	nr_motion_weight1_1_reg.nr_motion_weight1_0 = ptr->nr_motion_weight1_0;
	nr_motion_weight1_1_reg.nr_motion_weight1_1 = ptr->nr_motion_weight1_1;
	nr_motion_weight1_1_reg.nr_motion_weight1_2 = ptr->nr_motion_weight1_2;
	nr_motion_weight1_1_reg.nr_motion_weight1_3 = ptr->nr_motion_weight1_3;
	nr_motion_weight1_1_reg.nr_motion_weight1_4 = ptr->nr_motion_weight1_4;
	nr_motion_weight1_1_reg.nr_motion_weight1_5 = ptr->nr_motion_weight1_5;
	nr_motion_weight1_1_reg.nr_motion_weight1_6 = ptr->nr_motion_weight1_6;
	nr_motion_weight1_1_reg.nr_motion_weight1_7 = ptr->nr_motion_weight1_7;
	nr_motion_weight1_2_reg.nr_motion_weight1_8 = ptr->nr_motion_weight1_8;
	nr_motion_weight1_2_reg.nr_motion_weight1_9 = ptr->nr_motion_weight1_9;
	nr_motion_weight1_2_reg.nr_motion_weight1_10 = ptr->nr_motion_weight1_10;
	nr_motion_weight1_2_reg.nr_motion_weight1_11 = ptr->nr_motion_weight1_11;
	nr_motion_weight1_2_reg.nr_motion_weight1_12 = ptr->nr_motion_weight1_12;
	nr_motion_weight1_2_reg.nr_motion_weight1_13 = ptr->nr_motion_weight1_13;
	nr_motion_weight1_2_reg.nr_motion_weight1_14 = ptr->nr_motion_weight1_14;
	nr_motion_weight1_2_reg.nr_motion_weight1_15 = ptr->nr_motion_weight1_15;
	nr_motion_weight1_3_reg.nr_motion_weight1_16 = ptr->nr_motion_weight1_16;
	nr_motion_weight1_3_reg.nr_motion_weight1_17 = ptr->nr_motion_weight1_17;
	nr_motion_weight1_3_reg.nr_motion_weight1_18 = ptr->nr_motion_weight1_18;
	nr_motion_weight1_3_reg.nr_motion_weight1_19 = ptr->nr_motion_weight1_19;
	nr_motion_weight1_3_reg.nr_motion_weight1_20 = ptr->nr_motion_weight1_20;
	nr_motion_weight1_3_reg.nr_motion_weight1_21 = ptr->nr_motion_weight1_21;
	nr_motion_weight1_3_reg.nr_motion_weight1_22 = ptr->nr_motion_weight1_22;
	nr_motion_weight1_3_reg.nr_motion_weight1_23 = ptr->nr_motion_weight1_23;
	nr_motion_weight1_4_reg.nr_motion_weight1_24 = ptr->nr_motion_weight1_24;
	nr_motion_weight1_4_reg.nr_motion_weight1_25 = ptr->nr_motion_weight1_25;
	nr_motion_weight1_4_reg.nr_motion_weight1_26 = ptr->nr_motion_weight1_26;
	nr_motion_weight1_4_reg.nr_motion_weight1_27 = ptr->nr_motion_weight1_27;
	nr_motion_weight1_4_reg.nr_motion_weight1_28 = ptr->nr_motion_weight1_28;
	nr_motion_weight1_4_reg.nr_motion_weight1_29 = ptr->nr_motion_weight1_29;
	nr_motion_weight1_4_reg.nr_motion_weight1_30 = ptr->nr_motion_weight1_30;
	nr_motion_weight1_4_reg.nr_motion_weight1_31 = ptr->nr_motion_weight1_31;

	IoReg_Write32(NR_MOTION_WEIGHT_ctrl_reg, nr_motion_weight_ctrl_reg.regValue);
	IoReg_Write32(NR_MOTION_WEIGHT1_1_reg, nr_motion_weight1_1_reg.regValue);
	IoReg_Write32(NR_MOTION_WEIGHT1_2_reg, nr_motion_weight1_2_reg.regValue);
	IoReg_Write32(NR_MOTION_WEIGHT1_3_reg, nr_motion_weight1_3_reg.regValue);
	IoReg_Write32(NR_MOTION_WEIGHT1_4_reg, nr_motion_weight1_4_reg.regValue);
}
void drvif_color_DRV_MA_SNR_Motion_Weight2(DRV_MA_SNR_Motion_Weight2 *ptr)
{
	// add for Merlin2 (jyyang 20160620)
	nr_motion_weight2_1_RBUS nr_motion_weight2_1_reg;
	nr_motion_weight2_2_RBUS nr_motion_weight2_2_reg;
	nr_motion_weight2_3_RBUS nr_motion_weight2_3_reg;
	nr_motion_weight2_4_RBUS nr_motion_weight2_4_reg;

	nr_motion_weight2_1_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT2_1_reg);
	nr_motion_weight2_2_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT2_2_reg);
	nr_motion_weight2_3_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT2_3_reg);
	nr_motion_weight2_4_reg.regValue = IoReg_Read32(NR_MOTION_WEIGHT2_4_reg);

	nr_motion_weight2_1_reg.nr_motion_weight2_0 = ptr->nr_motion_weight2_0;
	nr_motion_weight2_1_reg.nr_motion_weight2_1 = ptr->nr_motion_weight2_1;
	nr_motion_weight2_1_reg.nr_motion_weight2_2 = ptr->nr_motion_weight2_2;
	nr_motion_weight2_1_reg.nr_motion_weight2_3 = ptr->nr_motion_weight2_3;
	nr_motion_weight2_1_reg.nr_motion_weight2_4 = ptr->nr_motion_weight2_4;
	nr_motion_weight2_1_reg.nr_motion_weight2_5 = ptr->nr_motion_weight2_5;
	nr_motion_weight2_1_reg.nr_motion_weight2_6 = ptr->nr_motion_weight2_6;
	nr_motion_weight2_1_reg.nr_motion_weight2_7 = ptr->nr_motion_weight2_7;
	nr_motion_weight2_2_reg.nr_motion_weight2_8 = ptr->nr_motion_weight2_8;
	nr_motion_weight2_2_reg.nr_motion_weight2_9 = ptr->nr_motion_weight2_9;
	nr_motion_weight2_2_reg.nr_motion_weight2_10 = ptr->nr_motion_weight2_10;
	nr_motion_weight2_2_reg.nr_motion_weight2_11 = ptr->nr_motion_weight2_11;
	nr_motion_weight2_2_reg.nr_motion_weight2_12 = ptr->nr_motion_weight2_12;
	nr_motion_weight2_2_reg.nr_motion_weight2_13 = ptr->nr_motion_weight2_13;
	nr_motion_weight2_2_reg.nr_motion_weight2_14 = ptr->nr_motion_weight2_14;
	nr_motion_weight2_2_reg.nr_motion_weight2_15 = ptr->nr_motion_weight2_15;
	nr_motion_weight2_3_reg.nr_motion_weight2_16 = ptr->nr_motion_weight2_16;
	nr_motion_weight2_3_reg.nr_motion_weight2_17 = ptr->nr_motion_weight2_17;
	nr_motion_weight2_3_reg.nr_motion_weight2_18 = ptr->nr_motion_weight2_18;
	nr_motion_weight2_3_reg.nr_motion_weight2_19 = ptr->nr_motion_weight2_19;
	nr_motion_weight2_3_reg.nr_motion_weight2_20 = ptr->nr_motion_weight2_20;
	nr_motion_weight2_3_reg.nr_motion_weight2_21 = ptr->nr_motion_weight2_21;
	nr_motion_weight2_3_reg.nr_motion_weight2_22 = ptr->nr_motion_weight2_22;
	nr_motion_weight2_3_reg.nr_motion_weight2_23 = ptr->nr_motion_weight2_23;
	nr_motion_weight2_4_reg.nr_motion_weight2_24 = ptr->nr_motion_weight2_24;
	nr_motion_weight2_4_reg.nr_motion_weight2_25 = ptr->nr_motion_weight2_25;
	nr_motion_weight2_4_reg.nr_motion_weight2_26 = ptr->nr_motion_weight2_26;
	nr_motion_weight2_4_reg.nr_motion_weight2_27 = ptr->nr_motion_weight2_27;
	nr_motion_weight2_4_reg.nr_motion_weight2_28 = ptr->nr_motion_weight2_28;
	nr_motion_weight2_4_reg.nr_motion_weight2_29 = ptr->nr_motion_weight2_29;
	nr_motion_weight2_4_reg.nr_motion_weight2_30 = ptr->nr_motion_weight2_30;
	nr_motion_weight2_4_reg.nr_motion_weight2_31 = ptr->nr_motion_weight2_31;

	IoReg_Write32(NR_MOTION_WEIGHT2_1_reg, nr_motion_weight2_1_reg.regValue);
	IoReg_Write32(NR_MOTION_WEIGHT2_2_reg, nr_motion_weight2_2_reg.regValue);
	IoReg_Write32(NR_MOTION_WEIGHT2_3_reg, nr_motion_weight2_3_reg.regValue);
	IoReg_Write32(NR_MOTION_WEIGHT2_4_reg, nr_motion_weight2_4_reg.regValue);
}

void drvif_color_DRV_MA_IESM_Motion_Ctrl(DRV_MA_IESM_Motion_Ctrl *ptr)
{

	// add for Merlin2 (jyyang 20160620)
	iedge_smooth_eng_clamp_RBUS eng_clamp_reg;
	iedge_smooth_vfilter_4_RBUS vfilter_4_reg;
	eng_clamp_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_ENG_CLAMP_reg);
	vfilter_4_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_4_reg);

	eng_clamp_reg.iedsm_motion_en = ptr->IESM_Motion_En;
	vfilter_4_reg.egsm_avlpf_motion_en = ptr ->AVLPF_Motion_En ;
	vfilter_4_reg.egsm_motion_i2p_blendmode = ptr ->IESM_I2P_Blend_Mode;

	IoReg_Write32(IEDGE_SMOOTH_ENG_CLAMP_reg, eng_clamp_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_4_reg, vfilter_4_reg.regValue);
}

void drvif_color_DRV_MA_IESM_Motion_Weight(DRV_MA_IESM_Motion_Weight *ptr)
{
	// add for Merlin2 (jyyang 20160620)
	iedge_smooth_motion_weight_1_RBUS motion_weight1_reg;
	iedge_smooth_motion_weight_2_RBUS motion_weight2_reg;
	iedge_smooth_motion_weight_3_RBUS motion_weight3_reg;
	iedge_smooth_motion_weight_4_RBUS motion_weight4_reg;

	motion_weight1_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MOTION_WEIGHT_1_reg);
	motion_weight2_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MOTION_WEIGHT_2_reg);
	motion_weight3_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MOTION_WEIGHT_3_reg);
	motion_weight4_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MOTION_WEIGHT_4_reg);

	motion_weight1_reg.edsm_motion_weight_0 = ptr->IESM_Motion_Weight_0;
	motion_weight1_reg.edsm_motion_weight_1 = ptr->IESM_Motion_Weight_1;
	motion_weight1_reg.edsm_motion_weight_2 = ptr->IESM_Motion_Weight_2;
	motion_weight1_reg.edsm_motion_weight_3 = ptr->IESM_Motion_Weight_3;
	motion_weight1_reg.edsm_motion_weight_4 = ptr->IESM_Motion_Weight_4;
	motion_weight1_reg.edsm_motion_weight_5 = ptr->IESM_Motion_Weight_5;
	motion_weight1_reg.edsm_motion_weight_6 = ptr->IESM_Motion_Weight_6;
	motion_weight1_reg.edsm_motion_weight_7 = ptr->IESM_Motion_Weight_7;

	motion_weight2_reg.edsm_motion_weight_8 = ptr->IESM_Motion_Weight_8;
	motion_weight2_reg.edsm_motion_weight_9 = ptr->IESM_Motion_Weight_9;
	motion_weight2_reg.edsm_motion_weight_10= ptr->IESM_Motion_Weight_10;
	motion_weight2_reg.edsm_motion_weight_11= ptr->IESM_Motion_Weight_11;
	motion_weight2_reg.edsm_motion_weight_12= ptr->IESM_Motion_Weight_12;
	motion_weight2_reg.edsm_motion_weight_13= ptr->IESM_Motion_Weight_13;
	motion_weight2_reg.edsm_motion_weight_14= ptr->IESM_Motion_Weight_14;
	motion_weight2_reg.edsm_motion_weight_15= ptr->IESM_Motion_Weight_15;

	motion_weight3_reg.edsm_motion_weight_16= ptr->IESM_Motion_Weight_16;
	motion_weight3_reg.edsm_motion_weight_17= ptr->IESM_Motion_Weight_17;
	motion_weight3_reg.edsm_motion_weight_18= ptr->IESM_Motion_Weight_18;
	motion_weight3_reg.edsm_motion_weight_19= ptr->IESM_Motion_Weight_19;
	motion_weight3_reg.edsm_motion_weight_20= ptr->IESM_Motion_Weight_20;
	motion_weight3_reg.edsm_motion_weight_21= ptr->IESM_Motion_Weight_21;
	motion_weight3_reg.edsm_motion_weight_22= ptr->IESM_Motion_Weight_22;
	motion_weight3_reg.edsm_motion_weight_23= ptr->IESM_Motion_Weight_23;

	motion_weight4_reg.edsm_motion_weight_24= ptr->IESM_Motion_Weight_24;
	motion_weight4_reg.edsm_motion_weight_25= ptr->IESM_Motion_Weight_25;
	motion_weight4_reg.edsm_motion_weight_26= ptr->IESM_Motion_Weight_26;
	motion_weight4_reg.edsm_motion_weight_27= ptr->IESM_Motion_Weight_27;
	motion_weight4_reg.edsm_motion_weight_28= ptr->IESM_Motion_Weight_28;
	motion_weight4_reg.edsm_motion_weight_29= ptr->IESM_Motion_Weight_29;
	motion_weight4_reg.edsm_motion_weight_30= ptr->IESM_Motion_Weight_30;
	motion_weight4_reg.edsm_motion_weight_31= ptr->IESM_Motion_Weight_31;

	IoReg_Write32(IEDGE_SMOOTH_MOTION_WEIGHT_1_reg, motion_weight1_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_MOTION_WEIGHT_2_reg, motion_weight2_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_MOTION_WEIGHT_3_reg, motion_weight3_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_MOTION_WEIGHT_4_reg, motion_weight4_reg.regValue);
}

void drvif_color_DRV_MA_VLPF_Motion_Weight(DRV_MA_VLPF_Motion_Weight *ptr)
{
#if 0//merliln3
	iedge_smooth_vfilter_0_RBUS vfilter_0_reg;
	iedge_smooth_vfilter_1_RBUS vfilter_1_reg;
	iedge_smooth_vfilter_2_RBUS vfilter_2_reg;
	iedge_smooth_vfilter_3_RBUS vfilter_3_reg;
	iedge_smooth_vfilter_4_RBUS vfilter_4_reg;
	iedge_smooth_vfilter_5_RBUS vfilter_5_reg;
	iedge_smooth_vfilter_6_RBUS vfilter_6_reg;
	iedge_smooth_vfilter_7_RBUS vfilter_7_reg;
	iedge_smooth_vfilter_8_RBUS vfilter_8_reg;
	iedge_smooth_vfilter_9_RBUS vfilter_9_reg;
	iedge_smooth_vfilter_10_RBUS vfilter_10_reg;

	vfilter_0_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_0_reg);
	vfilter_1_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_1_reg);
	vfilter_2_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_2_reg);
	vfilter_3_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_3_reg);
	vfilter_4_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_4_reg);
	vfilter_5_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_5_reg);
	vfilter_6_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_6_reg);
	vfilter_7_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_7_reg);
	vfilter_8_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_8_reg);
	vfilter_9_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_9_reg);
	vfilter_10_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_10_reg);

	vfilter_5_reg.egsm_avlpf_motionweight0 = ptr->AVLPF_Motion_Weight_0;
	vfilter_5_reg.egsm_avlpf_motionweight1 = ptr->AVLPF_Motion_Weight_1;
	vfilter_5_reg.egsm_avlpf_motionweight2 = ptr->AVLPF_Motion_Weight_2;
	vfilter_5_reg.egsm_avlpf_motionweight3 = ptr->AVLPF_Motion_Weight_3;
	vfilter_5_reg.egsm_avlpf_motionweight4 = ptr->AVLPF_Motion_Weight_4;
	vfilter_10_reg.egsm_avlpf_motionweight5 = ptr->AVLPF_Motion_Weight_5;
	vfilter_10_reg.egsm_avlpf_motionweight6 = ptr->AVLPF_Motion_Weight_6;
	vfilter_3_reg.egsm_avlpf_motionweight7 = ptr->AVLPF_Motion_Weight_7;
	vfilter_3_reg.egsm_avlpf_motionweight8 = ptr->AVLPF_Motion_Weight_8;
	vfilter_3_reg.egsm_avlpf_motionweight9 = ptr->AVLPF_Motion_Weight_9;
	vfilter_3_reg.egsm_avlpf_motionweight10 = ptr->AVLPF_Motion_Weight_10;
	vfilter_3_reg.egsm_avlpf_motionweight11 = ptr->AVLPF_Motion_Weight_11;
	vfilter_7_reg.egsm_avlpf_motionweight12 = ptr->AVLPF_Motion_Weight_12;
	vfilter_7_reg.egsm_avlpf_motionweight13 = ptr->AVLPF_Motion_Weight_13;
	vfilter_9_reg.egsm_avlpf_motionweight14 = ptr->AVLPF_Motion_Weight_14;
	vfilter_9_reg.egsm_avlpf_motionweight15 = ptr->AVLPF_Motion_Weight_15;
	vfilter_9_reg.egsm_avlpf_motionweight16 = ptr->AVLPF_Motion_Weight_16;
	vfilter_9_reg.egsm_avlpf_motionweight17 = ptr->AVLPF_Motion_Weight_17;
	vfilter_9_reg.egsm_avlpf_motionweight18 = ptr->AVLPF_Motion_Weight_18;
	vfilter_9_reg.egsm_avlpf_motionweight19 = ptr->AVLPF_Motion_Weight_19;
	vfilter_10_reg.egsm_avlpf_motionweight20 = ptr->AVLPF_Motion_Weight_20;
	vfilter_10_reg.egsm_avlpf_motionweight21 = ptr->AVLPF_Motion_Weight_21;
	vfilter_8_reg.egsm_avlpf_motionweight22 = ptr->AVLPF_Motion_Weight_22;
	vfilter_8_reg.egsm_avlpf_motionweight23 = ptr->AVLPF_Motion_Weight_23;
	vfilter_8_reg.egsm_avlpf_motionweight24 = ptr->AVLPF_Motion_Weight_24;
	vfilter_8_reg.egsm_avlpf_motionweight25 = ptr->AVLPF_Motion_Weight_25;
	vfilter_8_reg.egsm_avlpf_motionweight26 = ptr->AVLPF_Motion_Weight_26;
	vfilter_8_reg.egsm_avlpf_motionweight27 = ptr->AVLPF_Motion_Weight_27;
	vfilter_10_reg.egsm_avlpf_motionweight28 = ptr->AVLPF_Motion_Weight_28;
	vfilter_10_reg.egsm_avlpf_motionweight29 = ptr->AVLPF_Motion_Weight_29;
	vfilter_7_reg.egsm_avlpf_motionweight30 = ptr->AVLPF_Motion_Weight_30;
	vfilter_7_reg.egsm_avlpf_motionweight31 = ptr->AVLPF_Motion_Weight_31;

	IoReg_Write32(IEDGE_SMOOTH_VFILTER_0_reg, vfilter_0_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_1_reg, vfilter_1_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_2_reg, vfilter_2_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_3_reg, vfilter_3_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_4_reg, vfilter_4_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_5_reg, vfilter_5_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_6_reg, vfilter_6_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_7_reg, vfilter_7_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_8_reg, vfilter_8_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_9_reg, vfilter_9_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_10_reg, vfilter_10_reg.regValue);
#else//mac6
	iedge_smooth_vfilter_4_RBUS vfilter_4_reg;
	iedge_smooth_vfilter_5_RBUS vfilter_5_reg;
	iedge_smooth_vfilter_6_RBUS vfilter_6_reg;
	iedge_smooth_vfilter_7_RBUS vfilter_7_reg;
	iedge_smooth_vfilter_8_RBUS vfilter_8_reg;
	iedge_smooth_vfilter_9_RBUS vfilter_9_reg;

	vfilter_4_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_4_reg);
	vfilter_5_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_5_reg);
	vfilter_6_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_6_reg);
	vfilter_7_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_7_reg);
	vfilter_8_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_8_reg);
	vfilter_9_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_9_reg);

	vfilter_4_reg.egsm_avlpf_motionweight0 = ptr->AVLPF_Motion_Weight_0;
	vfilter_4_reg.egsm_avlpf_motionweight1 = ptr->AVLPF_Motion_Weight_1;
	vfilter_4_reg.egsm_avlpf_motionweight2 = ptr->AVLPF_Motion_Weight_2;
	vfilter_4_reg.egsm_avlpf_motionweight3 = ptr->AVLPF_Motion_Weight_3;
	vfilter_4_reg.egsm_avlpf_motionweight4 = ptr->AVLPF_Motion_Weight_4;

	vfilter_5_reg.egsm_avlpf_motionweight5 = ptr->AVLPF_Motion_Weight_5;
	vfilter_5_reg.egsm_avlpf_motionweight6 = ptr->AVLPF_Motion_Weight_6;
	vfilter_5_reg.egsm_avlpf_motionweight7 = ptr->AVLPF_Motion_Weight_7;
	vfilter_5_reg.egsm_avlpf_motionweight8 = ptr->AVLPF_Motion_Weight_8;
	vfilter_5_reg.egsm_avlpf_motionweight9 = ptr->AVLPF_Motion_Weight_9;
	vfilter_5_reg.egsm_avlpf_motionweight10 = ptr->AVLPF_Motion_Weight_10;

	vfilter_6_reg.egsm_avlpf_motionweight11 = ptr->AVLPF_Motion_Weight_11;
	vfilter_6_reg.egsm_avlpf_motionweight12 = ptr->AVLPF_Motion_Weight_12;
	vfilter_6_reg.egsm_avlpf_motionweight13 = ptr->AVLPF_Motion_Weight_13;
	vfilter_6_reg.egsm_avlpf_motionweight14 = ptr->AVLPF_Motion_Weight_14;
	vfilter_6_reg.egsm_avlpf_motionweight15 = ptr->AVLPF_Motion_Weight_15;
	vfilter_6_reg.egsm_avlpf_motionweight16 = ptr->AVLPF_Motion_Weight_16;

	vfilter_7_reg.egsm_avlpf_motionweight17 = ptr->AVLPF_Motion_Weight_17;
	vfilter_7_reg.egsm_avlpf_motionweight18 = ptr->AVLPF_Motion_Weight_18;
	vfilter_7_reg.egsm_avlpf_motionweight19 = ptr->AVLPF_Motion_Weight_19;
	vfilter_7_reg.egsm_avlpf_motionweight20 = ptr->AVLPF_Motion_Weight_20;
	vfilter_7_reg.egsm_avlpf_motionweight21 = ptr->AVLPF_Motion_Weight_21;
	vfilter_7_reg.egsm_avlpf_motionweight22 = ptr->AVLPF_Motion_Weight_22;

	vfilter_8_reg.egsm_avlpf_motionweight23 = ptr->AVLPF_Motion_Weight_23;
	vfilter_8_reg.egsm_avlpf_motionweight24 = ptr->AVLPF_Motion_Weight_24;
	vfilter_8_reg.egsm_avlpf_motionweight25 = ptr->AVLPF_Motion_Weight_25;
	vfilter_8_reg.egsm_avlpf_motionweight26 = ptr->AVLPF_Motion_Weight_26;
	vfilter_8_reg.egsm_avlpf_motionweight27 = ptr->AVLPF_Motion_Weight_27;
	vfilter_8_reg.egsm_avlpf_motionweight28 = ptr->AVLPF_Motion_Weight_28;

	vfilter_9_reg.egsm_avlpf_motionweight29 = ptr->AVLPF_Motion_Weight_29;
	vfilter_9_reg.egsm_avlpf_motionweight30 = ptr->AVLPF_Motion_Weight_30;
	vfilter_9_reg.egsm_avlpf_motionweight31 = ptr->AVLPF_Motion_Weight_31;

	IoReg_Write32(IEDGE_SMOOTH_VFILTER_4_reg, vfilter_4_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_5_reg, vfilter_5_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_6_reg, vfilter_6_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_7_reg, vfilter_7_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_8_reg, vfilter_8_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_9_reg, vfilter_9_reg.regValue);

#endif
}

void drvif_color_MASNR_DB_Enable(void)
{
	IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_en_mask);
	IoReg_SetBits(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_en_mask);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask);
}

void drvif_color_MASNR_DB_Apply(void)
{
	IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);
	IoReg_SetBits(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_apply_mask);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask);
}

#if 0
void drvif_color_IEDGESMOOTH_flow(DRV_IEdgeSmooth_Coef *ptr)
{
	if (!ptr) {
		VIPprintf("drvif_color_IEDGESMOOTH_flow table to NULL\n");
		return;
	}

	drvif_color_DRV_IESM_Basic(&(ptr->S_IESM_Basic));
	drvif_color_DRV_DLTI_2D(&(ptr->S_DLTI_2D));
	drvif_color_DRV_IESM_Additional_setting(&(ptr->S_IESM_Additional_setting));
}
#endif

// chen 170522
#if 0
void drvif_color_DRV_NM_Control(DRV_rtnr_nm_ctrl_t *ptr)
{

	di_im_di_film_motion_sum_th_RBUS im_di_film_motion_sum_th_reg;	/* dummy control*/

	im_di_film_motion_sum_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_MOTION_SUM_TH_reg);
	/*im_di_film_motion_sum_th_reg.nm_main_enable = ptr->main_enable; */ /* struct not define this member fix@20140716*/
	if (ptr->main_enable == 1)
		im_di_film_motion_sum_th_reg.regValue |= _BIT23;
	else
		im_di_film_motion_sum_th_reg.regValue &= (~_BIT23);

	IoReg_Write32(DI_IM_DI_FILM_MOTION_SUM_TH_reg, im_di_film_motion_sum_th_reg.regValue);
	IoReg_Write32(DI_DI_RTNR_NM_CONTROL_reg, di_rtnr_nm_control_reg.regValue);
	IoReg_Write32(DI_DI_RTNR_NM_MARGIN0_reg, di_rtnr_nm_margin0_reg.regValue);
	IoReg_Write32(DI_DI_RTNR_NM_MARGIN1_reg, di_rtnr_nm_margin1_reg.regValue);
}

void drvif_color_DRV_NM_Thd(DRV_rtnr_nm_thd_t *ptr)
{

	di_di_rtnr_nm_varmean_control_RBUS di_rtnr_nm_varmean_control_reg;
	di_di_rtnr_nm_var_thd_RBUS di_rtnr_nm_var_thd_reg;
	di_di_rtnr_nm_noiselevel_th0_RBUS di_rtnr_nm_noiselevel_th0_reg;
	di_di_rtnr_nm_noiselevel_th1_RBUS di_rtnr_nm_noiselevel_th1_reg;
	di_di_rtnr_nm_noiselevel_th2_RBUS di_rtnr_nm_noiselevel_th2_reg;
	di_di_rtnr_nm_noiselevel_th3_RBUS di_rtnr_nm_noiselevel_th3_reg;

	di_rtnr_nm_varmean_control_reg.regValue = IoReg_Read32(DI_DI_RTNR_NM_VARMEAN_CONTROL_reg);
	di_rtnr_nm_varmean_control_reg.nm_meandiff_thd = ptr->meandiff_thd;
	di_rtnr_nm_varmean_control_reg.nm_vardiff_thd = ptr->vardiff_thd;

	di_rtnr_nm_var_thd_reg.regValue =  IoReg_Read32(DI_DI_RTNR_NM_VAR_THD_reg);
	di_rtnr_nm_var_thd_reg.nm_varhigh_thd = ptr->varhigh_thd;
	di_rtnr_nm_var_thd_reg.nm_varlow_thd = ptr->varlow_thd;

	di_rtnr_nm_noiselevel_th0_reg.regValue = IoReg_Read32(DI_DI_RTNR_NM_NOISELEVEL_TH0_reg);
	di_rtnr_nm_noiselevel_th0_reg.noiselevel_thd0 = ptr->noiselevel_thd0;
	di_rtnr_nm_noiselevel_th0_reg.noiselevel_thd1 = ptr->noiselevel_thd1;
	di_rtnr_nm_noiselevel_th0_reg.noiselevel_thd2 = ptr->noiselevel_thd2;
	di_rtnr_nm_noiselevel_th0_reg.noiselevel_thd3 = ptr->noiselevel_thd3;

	di_rtnr_nm_noiselevel_th1_reg.regValue = IoReg_Read32(DI_DI_RTNR_NM_NOISELEVEL_TH1_reg);
	di_rtnr_nm_noiselevel_th1_reg.noiselevel_thd4 = ptr->noiselevel_thd4;
	di_rtnr_nm_noiselevel_th1_reg.noiselevel_thd5 = ptr->noiselevel_thd5;
	di_rtnr_nm_noiselevel_th1_reg.noiselevel_thd6 = ptr->noiselevel_thd6;
	di_rtnr_nm_noiselevel_th1_reg.noiselevel_thd7 = ptr->noiselevel_thd7;

	di_rtnr_nm_noiselevel_th2_reg.regValue = IoReg_Read32(DI_DI_RTNR_NM_NOISELEVEL_TH2_reg);
	di_rtnr_nm_noiselevel_th2_reg.noiselevel_thd8 = ptr->noiselevel_thd8;
	di_rtnr_nm_noiselevel_th2_reg.noiselevel_thd9 = ptr->noiselevel_thd9;
	di_rtnr_nm_noiselevel_th2_reg.noiselevel_thd10 = ptr->noiselevel_thd10;
	di_rtnr_nm_noiselevel_th2_reg.noiselevel_thd11 = ptr->noiselevel_thd11;

	di_rtnr_nm_noiselevel_th3_reg.regValue = IoReg_Read32(DI_DI_RTNR_NM_NOISELEVEL_TH3_reg);
	di_rtnr_nm_noiselevel_th3_reg.noiselevel_thd12 = ptr->noiselevel_thd12;
	di_rtnr_nm_noiselevel_th3_reg.noiselevel_thd13 = ptr->noiselevel_thd13;
	di_rtnr_nm_noiselevel_th3_reg.noiselevel_thd14 = ptr->noiselevel_thd14;

	IoReg_Write32(DI_DI_RTNR_NM_VARMEAN_CONTROL_reg, di_rtnr_nm_varmean_control_reg.regValue);
	IoReg_Write32(DI_DI_RTNR_NM_VAR_THD_reg, di_rtnr_nm_var_thd_reg.regValue);
	IoReg_Write32(DI_DI_RTNR_NM_NOISELEVEL_TH0_reg, di_rtnr_nm_noiselevel_th0_reg.regValue);
	IoReg_Write32(DI_DI_RTNR_NM_NOISELEVEL_TH1_reg, di_rtnr_nm_noiselevel_th1_reg.regValue);
	IoReg_Write32(DI_DI_RTNR_NM_NOISELEVEL_TH2_reg, di_rtnr_nm_noiselevel_th2_reg.regValue);
	IoReg_Write32(DI_DI_RTNR_NM_NOISELEVEL_TH3_reg, di_rtnr_nm_noiselevel_th3_reg.regValue);
}

void drvif_color_DRV_NM_Init(DRV_rtnr_nm_ctrl_t *nm_ctrl, DRV_rtnr_nm_thd_t *nm_thd)
{
	drvif_color_DRV_NM_Control(nm_ctrl);
	drvif_color_DRV_NM_Thd(nm_thd);
}
#endif
//end chen 170522


/* chen 0716*/
void drvif_color_DRV_RTNR_MASNR(DRV_RTNR_MASNR *ptr)
{
	di_rtnr_ma_snr_control_RBUS						di_rtnr_ma_snr_control;
	di_rtnr_ma_snr_motion_th_low_RBUS			di_rtnr_ma_snr_motion_th_low;
	di_rtnr_ma_snr_motion_th_high_RBUS		di_rtnr_ma_snr_motion_th_high;
	di_rtnr_ma_snr_motion_edge_th_RBUS		di_rtnr_ma_snr_motion_edge_th;
	di_rtnr_ma_snr_motion_th_slope_RBUS		di_rtnr_ma_snr_motion_th_slope;
	di_rtnr_ma_snr_strength_control_RBUS	di_rtnr_ma_snr_strength_control;

	if (!ptr)
		return;

	di_rtnr_ma_snr_control.regValue 				= IoReg_Read32(DI_RTNR_MA_SNR_CONTROL_reg);
	di_rtnr_ma_snr_motion_th_low.regValue 	= IoReg_Read32(DI_RTNR_MA_SNR_MOTION_TH_LOW_reg);
	di_rtnr_ma_snr_motion_th_high.regValue 	= IoReg_Read32(DI_RTNR_MA_SNR_MOTION_TH_HIGH_reg);
	di_rtnr_ma_snr_motion_edge_th.regValue 	= IoReg_Read32(DI_RTNR_MA_SNR_MOTION_EDGE_TH_reg);
	di_rtnr_ma_snr_motion_th_slope.regValue = IoReg_Read32(DI_RTNR_MA_SNR_MOTION_TH_SLOPE_reg);
	di_rtnr_ma_snr_strength_control.regValue = IoReg_Read32(DI_RTNR_MA_SNR_STRENGTH_CONTROL_reg);


	di_rtnr_ma_snr_control.rtnr_ma_snr_en = ptr->rtnr_ma_snr_en;
	di_rtnr_ma_snr_control.rtnr_ma_snr_mask = ptr->rtnr_ma_snr_mask;
	di_rtnr_ma_snr_control.rtnr_ma_snr_blending_factor1 = ptr->rtnr_ma_snr_blending_factor1;
	di_rtnr_ma_snr_control.rtnr_ma_snr_blending_factor2 = ptr->rtnr_ma_snr_blending_factor2;
	di_rtnr_ma_snr_control.rtnr_ma_snr_blending_factor3 = ptr->rtnr_ma_snr_blending_factor3;
	di_rtnr_ma_snr_control.rtnr_ma_snr_blending_factor4 = ptr->rtnr_ma_snr_blending_factor4;
	di_rtnr_ma_snr_control.rtnr_ma_snr_blending_factor5 = ptr->rtnr_ma_snr_blending_factor5;

	di_rtnr_ma_snr_motion_th_low.rtnr_ma_snr_motion_th1_low = ptr->rtnr_ma_snr_motion_th1_low;
	di_rtnr_ma_snr_motion_th_low.rtnr_ma_snr_motion_th2_low = ptr->rtnr_ma_snr_motion_th2_low;
	di_rtnr_ma_snr_motion_th_low.rtnr_ma_snr_motion_th3_low = ptr->rtnr_ma_snr_motion_th3_low;
	di_rtnr_ma_snr_motion_th_low.rtnr_ma_snr_motion_th4_low = ptr->rtnr_ma_snr_motion_th4_low;

	di_rtnr_ma_snr_motion_th_high.rtnr_ma_snr_motion_th1_high = ptr->rtnr_ma_snr_motion_th1_high;
	di_rtnr_ma_snr_motion_th_high.rtnr_ma_snr_motion_th2_high = ptr->rtnr_ma_snr_motion_th2_high;
	di_rtnr_ma_snr_motion_th_high.rtnr_ma_snr_motion_th3_high = ptr->rtnr_ma_snr_motion_th3_high;
	di_rtnr_ma_snr_motion_th_high.rtnr_ma_snr_motion_th4_high = ptr->rtnr_ma_snr_motion_th4_high;

	di_rtnr_ma_snr_motion_edge_th.rtnr_ma_snr_edge_th_low 	= ptr->rtnr_ma_snr_edge_th_low;
	di_rtnr_ma_snr_motion_edge_th.rtnr_ma_snr_edge_th_high 	= ptr->rtnr_ma_snr_edge_th_high;

	di_rtnr_ma_snr_motion_th_slope.rtnr_ma_snr_motion_slope1 = ptr->rtnr_ma_snr_motion_slope1;
	di_rtnr_ma_snr_motion_th_slope.rtnr_ma_snr_motion_slope2 = ptr->rtnr_ma_snr_motion_slope2;
	di_rtnr_ma_snr_motion_th_slope.rtnr_ma_snr_motion_slope3 = ptr->rtnr_ma_snr_motion_slope3;
	di_rtnr_ma_snr_motion_th_slope.rtnr_ma_snr_motion_slope4 = ptr->rtnr_ma_snr_motion_slope4;

	di_rtnr_ma_snr_strength_control.rtnr_ma_snr_edge_curve_lo_th 	= ptr->rtnr_ma_snr_edge_curve_lo_th;
	di_rtnr_ma_snr_strength_control.rtnr_ma_snr_edge_curve_slope 	= ptr->rtnr_ma_snr_edge_curve_slope;
	di_rtnr_ma_snr_strength_control.rtnr_ma_snr_output_clamp 			= ptr->rtnr_ma_snr_output_clamp;
	di_rtnr_ma_snr_strength_control.rtnr_ma_snr_strength 					= ptr->rtnr_ma_snr_strength;


	IoReg_Write32(DI_RTNR_MA_SNR_CONTROL_reg, di_rtnr_ma_snr_control.regValue);
	IoReg_Write32(DI_RTNR_MA_SNR_MOTION_TH_LOW_reg, di_rtnr_ma_snr_motion_th_low.regValue);
	IoReg_Write32(DI_RTNR_MA_SNR_MOTION_TH_HIGH_reg, di_rtnr_ma_snr_motion_th_high.regValue);
	IoReg_Write32(DI_RTNR_MA_SNR_MOTION_EDGE_TH_reg, di_rtnr_ma_snr_motion_edge_th.regValue);
	IoReg_Write32(DI_RTNR_MA_SNR_MOTION_TH_SLOPE_reg, di_rtnr_ma_snr_motion_th_slope.regValue);
	IoReg_Write32(DI_RTNR_MA_SNR_STRENGTH_CONTROL_reg, di_rtnr_ma_snr_strength_control.regValue);


}

void drvif_color_Get_MAD_Value(DRV_MAD_counter *ptr)
{


	di_im_di_rtnr_mad_y_counter_RBUS im_di_rtnr_mad_y_counter_reg;
	di_im_di_rtnr_mad_u_counter_RBUS im_di_rtnr_mad_u_counter_reg;
	di_im_di_rtnr_mad_v_counter_RBUS im_di_rtnr_mad_v_counter_reg;

	im_di_rtnr_mad_y_counter_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_MAD_Y_COUNTER_reg);
	im_di_rtnr_mad_u_counter_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_MAD_U_COUNTER_reg);
	im_di_rtnr_mad_v_counter_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_MAD_V_COUNTER_reg);

	ptr->DRV_MAD_counter_y = im_di_rtnr_mad_y_counter_reg.cp_temporal_mad_countery;
	ptr->DRV_MAD_counter_u = im_di_rtnr_mad_u_counter_reg.cp_temporal_mad_counteru;
	ptr->DRV_MAD_counter_v = im_di_rtnr_mad_v_counter_reg.cp_temporal_mad_counterv;


}
/* end chen 0716*/

#ifdef CONFIG_HW_SUPPORT_I_DE_XC //mac6 removed de_xcxl
unsigned char  drvif_color_Set_I_De_XC(VIP_I_De_XC_TBL *ptr)
{
	de_xcxl_de_xcxl_ctrl_RBUS de_xcxl_de_xcxl_ctrl_reg;
	de_xcxl_de_xcxl_temporal_step_RBUS de_xcxl_de_xcxl_temporal_step_reg;
	de_xcxl_de_xcxl_temporal_step_pal_RBUS de_xcxl_de_xcxl_temporal_step_pal_reg;
	de_xcxl_de_xcxl_classify_y_RBUS de_xcxl_de_xcxl_classify_y_reg;
	de_xcxl_de_xcxl_classify_y_pal_RBUS de_xcxl_de_xcxl_classify_y_pal_reg;
	de_xcxl_de_xcxl_classify_c_RBUS de_xcxl_de_xcxl_classify_c_reg;
	de_xcxl_de_xcxl_classify_c_pal_RBUS de_xcxl_de_xcxl_classify_c_pal_reg;
	de_xcxl_de_xcxl_blend_RBUS de_xcxl_de_xcxl_blend_reg;
	de_xcxl_de_xcxl_db_reg_ctl_RBUS de_xcxl_de_xcxl_db_reg_ctl_reg;

	de_xcxl_de_xcxl_ctrl_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CTRL_reg);
	de_xcxl_de_xcxl_temporal_step_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_TEMPORAL_STEP_reg);
	de_xcxl_de_xcxl_temporal_step_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_TEMPORAL_STEP_PAL_reg);
	de_xcxl_de_xcxl_classify_y_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CLASSIFY_Y_PAL_reg);
	de_xcxl_de_xcxl_classify_c_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CLASSIFY_C_PAL_reg);
	de_xcxl_de_xcxl_classify_y_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CLASSIFY_Y_PAL_reg);
	de_xcxl_de_xcxl_classify_c_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CLASSIFY_C_PAL_reg);
	de_xcxl_de_xcxl_blend_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_BLEND_reg);

	/* db must be on*/
	de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = IoReg_Read32(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	de_xcxl_de_xcxl_db_reg_ctl_reg.db_en = 1;
	IoReg_Write32(DE_XCXL_DE_XCXL_db_reg_ctl_reg, de_xcxl_de_xcxl_db_reg_ctl_reg.regValue);

	de_xcxl_de_xcxl_ctrl_reg.dexc_en							=		ptr->dexc_en;
	de_xcxl_de_xcxl_ctrl_reg.dexc_spatial_correction_en				=		ptr->Dexc_spatial_correction_en;
	de_xcxl_de_xcxl_ctrl_reg.dexc_detect_type						=		ptr->dexc_detect_type;

	de_xcxl_de_xcxl_classify_y_reg.dexc_y_error_th1				=		ptr->dexc_y_error_th1_ntsc;
	de_xcxl_de_xcxl_classify_y_reg.dexc_y_error_th2				=		ptr->dexc_y_error_th2_ntsc;
	de_xcxl_de_xcxl_classify_y_reg.dexc_y_amp_th					=		ptr->dexc_y_amp_th_ntsc;
	de_xcxl_de_xcxl_classify_c_reg.dexc_c_error_th1				=		ptr->dexc_c_error_th1_ntsc;
	de_xcxl_de_xcxl_classify_c_reg.dexc_c_error_th2				=		ptr->dexc_c_error_th2_ntsc;
	de_xcxl_de_xcxl_classify_c_reg.dexc_c_amp_th					=		ptr->dexc_c_amp_th_ntsc;
	de_xcxl_de_xcxl_temporal_step_reg.dexc_step_lv1				=		ptr->dexc_step_lv1_ntsc;
	de_xcxl_de_xcxl_temporal_step_reg.dexc_step_lv2				=		ptr->dexc_step_lv2_ntsc;
	de_xcxl_de_xcxl_temporal_step_reg.dexc_step_still				=		ptr->dexc_step_still_ntsc;
	de_xcxl_de_xcxl_temporal_step_reg.dexc_step_notxc			=		ptr->dexc_step_notxc_ntsc;
	de_xcxl_de_xcxl_temporal_step_reg.dexc_hold_th				=		ptr->dexc_hold_th_ntsc;

	de_xcxl_de_xcxl_classify_y_pal_reg.dexc_y_error_th1_pal		=		ptr->dexc_y_error_th1_pal;
	de_xcxl_de_xcxl_classify_y_pal_reg.dexc_y_error_th2_pal		=		ptr->dexc_y_error_th2_pal;
	de_xcxl_de_xcxl_classify_y_pal_reg.dexc_y_amp_th_pal			=		ptr->dexc_y_amp_th_pal;
	de_xcxl_de_xcxl_classify_c_pal_reg.dexc_c_error_th1_pal		=		ptr->dexc_c_error_th1_pal;
	de_xcxl_de_xcxl_classify_c_pal_reg.dexc_c_error_th2_pal		=		ptr->dexc_c_error_th2_pal;
	de_xcxl_de_xcxl_classify_c_pal_reg.dexc_c_amp_th_pal			=		ptr->dexc_c_amp_th_pal;
	de_xcxl_de_xcxl_temporal_step_pal_reg.dexc_step_lv1_pal		=		ptr->dexc_step_lv1_pal;
	de_xcxl_de_xcxl_temporal_step_pal_reg.dexc_step_lv2_pal		=		ptr->dexc_step_lv2_pal;
	de_xcxl_de_xcxl_temporal_step_pal_reg.dexc_step_still_pal		=		ptr->dexc_step_still_pal;
	de_xcxl_de_xcxl_temporal_step_pal_reg.dexc_step_notxc_pal		=		ptr->dexc_step_notxc_pal;
	de_xcxl_de_xcxl_temporal_step_pal_reg.dexc_hold_th_pal		=		ptr->dexc_hold_th_pal;

	de_xcxl_de_xcxl_blend_reg.dexc_blending_table_0				=		ptr->dexc_blending_table_0;
	de_xcxl_de_xcxl_blend_reg.dexc_blending_table_1				=		ptr->dexc_blending_table_1;
	de_xcxl_de_xcxl_blend_reg.dexc_blending_table_2				=		ptr->dexc_blending_table_2;
	de_xcxl_de_xcxl_blend_reg.dexc_blending_table_3				=		ptr->dexc_blending_table_3;
	de_xcxl_de_xcxl_blend_reg.dexc_blending_table_4				=		ptr->dexc_blending_table_4;
	de_xcxl_de_xcxl_blend_reg.dexc_blending_table_5				=		ptr->dexc_blending_table_5;
	de_xcxl_de_xcxl_blend_reg.dexc_blending_table_6				=		ptr->dexc_blending_table_6;
	de_xcxl_de_xcxl_blend_reg.dexc_blending_table_7				=		ptr->dexc_blending_table_7;

	IoReg_Write32(DE_XCXL_De_XCXL_CTRL_reg, de_xcxl_de_xcxl_ctrl_reg.regValue);
	IoReg_Write32(DE_XCXL_De_XCXL_TEMPORAL_STEP_reg, de_xcxl_de_xcxl_temporal_step_reg.regValue);
	IoReg_Write32(DE_XCXL_De_XCXL_TEMPORAL_STEP_PAL_reg, de_xcxl_de_xcxl_temporal_step_pal_reg.regValue);
	IoReg_Write32(DE_XCXL_De_XCXL_CLASSIFY_Y_reg, de_xcxl_de_xcxl_classify_y_reg.regValue);
	IoReg_Write32(DE_XCXL_De_XCXL_CLASSIFY_C_reg, de_xcxl_de_xcxl_classify_c_reg.regValue);
	IoReg_Write32(DE_XCXL_De_XCXL_CLASSIFY_Y_PAL_reg, de_xcxl_de_xcxl_classify_y_pal_reg.regValue);
	IoReg_Write32(DE_XCXL_De_XCXL_CLASSIFY_C_PAL_reg, de_xcxl_de_xcxl_classify_c_pal_reg.regValue);
	IoReg_Write32(DE_XCXL_De_XCXL_BLEND_reg, de_xcxl_de_xcxl_blend_reg.regValue);

	de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = IoReg_Read32(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	de_xcxl_de_xcxl_db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DE_XCXL_DE_XCXL_db_reg_ctl_reg, de_xcxl_de_xcxl_db_reg_ctl_reg.regValue);

	return TRUE;
}
#endif //CONFIG_HW_SUPPORT_I_DE_XC

void drvif_color_Set_I_De_Contour_DB_En( bool DB_En )
{
	ipq_decontour_de_contour_db_reg_ctl_RBUS ipq_decontour_de_contour_db_reg_ctl_reg;

	ipq_decontour_de_contour_db_reg_ctl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_db_reg_ctl_reg);
	ipq_decontour_de_contour_db_reg_ctl_reg.db_en = DB_En;
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_db_reg_ctl_reg, ipq_decontour_de_contour_db_reg_ctl_reg.regValue);

}

void drvif_color_Set_I_De_Contour_DB_Apply(void)
{
	ipq_decontour_de_contour_db_reg_ctl_RBUS ipq_decontour_de_contour_db_reg_ctl_reg;

	ipq_decontour_de_contour_db_reg_ctl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_db_reg_ctl_reg);
	ipq_decontour_de_contour_db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_db_reg_ctl_reg, ipq_decontour_de_contour_db_reg_ctl_reg.regValue);
}

unsigned char  drvif_color_Set_I_De_Contour(VIP_I_De_Contour_TBL *ptr)
{
	ipq_decontour_de_contour_ctrl_RBUS ipq_decontour_de_contour_ctrl_reg;
	ipq_decontour_de_contour_contrast_RBUS ipq_decontour_de_contour_contrast_reg;
	ipq_decontour_de_contour_neighbor_diff_RBUS ipq_decontour_de_contour_neighbor_diff_reg;
	ipq_decontour_de_contour_threshold_RBUS ipq_decontour_de_contour_threshold_reg;
	ipq_decontour_de_contour_diff_th_RBUS ipq_decontour_de_contour_diff_th_reg;
	ipq_decontour_de_contour_bottleneck_RBUS ipq_decontour_de_contour_bottleneck_reg;
	ipq_decontour_de_contour_diff_clamp_RBUS ipq_decontour_de_contour_diff_clamp_reg;
	ipq_decontour_de_contour_contrast_y_RBUS ipq_decontour_de_contour_contrast_y_reg;
	ipq_decontour_de_contour_contrast_c_RBUS ipq_decontour_de_contour_contrast_c_reg;
	ipq_decontour_de_contour_contrast2_RBUS ipq_decontour_de_contour_contrast2_reg;
	ipq_decontour_de_contour_contrast3_RBUS ipq_decontour_de_contour_contrast3_reg;
	ipq_decontour_de_contour_dark_RBUS ipq_decontour_de_contour_dark_reg;
	ipq_decontour_de_contour_gradation_RBUS ipq_decontour_de_contour_gradation_reg;
	ipq_decontour_de_contour_minmax_RBUS ipq_decontour_de_contour_minmax_reg;
	ipq_decontour_de_contour_level_RBUS ipq_decontour_de_contour_level_reg;

	ipq_decontour_de_contour_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_CTRL_reg);
	ipq_decontour_de_contour_contrast_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Contrast_reg);
	ipq_decontour_de_contour_neighbor_diff_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Neighbor_Diff_reg);
	ipq_decontour_de_contour_threshold_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Threshold_reg);
	ipq_decontour_de_contour_diff_th_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Diff_th_reg);
	ipq_decontour_de_contour_bottleneck_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Bottleneck_reg);
	ipq_decontour_de_contour_diff_clamp_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Diff_Clamp_reg);
	ipq_decontour_de_contour_contrast_y_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Contrast_Y_reg);
	ipq_decontour_de_contour_contrast_c_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Contrast_C_reg);
	ipq_decontour_de_contour_contrast2_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Contrast2_reg);
	ipq_decontour_de_contour_contrast3_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Contrast3_reg);
	ipq_decontour_de_contour_dark_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Dark_reg);
	ipq_decontour_de_contour_gradation_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Gradation_reg);
	ipq_decontour_de_contour_minmax_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Minmax_reg);
	ipq_decontour_de_contour_level_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Level_reg);

	/*  DB */
	drvif_color_Set_I_De_Contour_DB_En( 1 );

	ipq_decontour_de_contour_ctrl_reg.decont_en                      	= ptr->decont_en;
	ipq_decontour_de_contour_ctrl_reg.decont_snr_blend_mode          	= ptr->decont_snr_blend_mode;
	ipq_decontour_de_contour_ctrl_reg.decont_blend_lowbd             	= ptr->decont_blend_lowbd;
	ipq_decontour_de_contour_ctrl_reg.decont_blend_step              	= ptr->decont_blend_step;
	ipq_decontour_de_contour_ctrl_reg.decont_blend_weight_lpf_en		= ptr->decont_blend_weight_lpf_en;
	ipq_decontour_de_contour_ctrl_reg.decont_neighbor_far            	= ptr->decont_neighbor_far;
	//ipq_decontour_de_contour_ctrl_reg.decont_filter_mode             	= ptr->decont_filter_mode;
	//ipq_decontour_de_contour_ctrl_reg.decont_line_mode       	     	= ptr->decont_line_mode;
	ipq_decontour_de_contour_ctrl_reg.decont_yc_mode			= ptr->decont_yc_mode;
	ipq_decontour_de_contour_ctrl_reg.decont_bottleneck_en			= ptr->decont_bottleneck_en;

	ipq_decontour_de_contour_contrast_reg.decont_contrast_th_y           	= ptr->decont_contrast_th_y;
	ipq_decontour_de_contour_contrast2_reg.decont_contrast_th2_y		= ptr->decont_contrast_th2_y;
	ipq_decontour_de_contour_contrast2_reg.decont_contrast_th3_y		= ptr->decont_contrast_th3_y;
	ipq_decontour_de_contour_contrast_reg.decont_contrast_th_c           	= ptr->decont_contrast_th_c;
	ipq_decontour_de_contour_contrast3_reg.decont_contrast_th2_c		= ptr->decont_contrast_th2_c;
	ipq_decontour_de_contour_contrast3_reg.decont_contrast_th3_c		= ptr->decont_contrast_th3_c;
	ipq_decontour_de_contour_neighbor_diff_reg.decont_neighbor_diff_th_y    = ptr->decont_neighbor_diff_th_y;
	ipq_decontour_de_contour_neighbor_diff_reg.decont_neighbor_diff_th2_y   = ptr->decont_neighbor_diff_th2_y;
	ipq_decontour_de_contour_neighbor_diff_reg.decont_neighbor_diff_th_c    = ptr->decont_neighbor_diff_th_c;
	ipq_decontour_de_contour_neighbor_diff_reg.decont_neighbor_diff_th2_c   = ptr->decont_neighbor_diff_th2_c;
	ipq_decontour_de_contour_threshold_reg.decont_dark_th                 	= ptr->decont_dark_th;
	ipq_decontour_de_contour_threshold_reg.decont_dark_contrast_th        	= ptr->decont_dark_contrast_th;
	ipq_decontour_de_contour_dark_reg.decont_dark_contrast_th2		= ptr->decont_dark_contrast_th2;
	ipq_decontour_de_contour_dark_reg.decont_dark_contrast_th3		= ptr->decont_dark_contrast_th3;

	ipq_decontour_de_contour_threshold_reg.decont_gradation_th_y		= ptr->decont_gradation_th_y;
	ipq_decontour_de_contour_gradation_reg.decont_gradation_th2_y		= ptr->decont_gradation_th2_y;
	ipq_decontour_de_contour_threshold_reg.decont_gradation_th_c		= ptr->decont_gradation_th_c;
	ipq_decontour_de_contour_gradation_reg.decont_gradation_th2_c		= ptr->decont_gradation_th2_c;
	ipq_decontour_de_contour_diff_th_reg.decont_maxmindiff_th_y		= ptr->decont_maxmindiff_th_y;
	ipq_decontour_de_contour_minmax_reg.decont_maxmindiff_th2_y		= ptr->decont_maxmindiff_th2_y;
	ipq_decontour_de_contour_minmax_reg.decont_maxmindiff_th3_y		= ptr->decont_maxmindiff_th3_y;
	ipq_decontour_de_contour_diff_th_reg.decont_maxmindiff_th_c		= ptr->decont_maxmindiff_th_c;
	ipq_decontour_de_contour_minmax_reg.decont_maxmin_lv0			= ptr->decont_maxmin_lv0;
	ipq_decontour_de_contour_minmax_reg.decont_maxmin_lv1			= ptr->decont_maxmin_lv1;
	ipq_decontour_de_contour_minmax_reg.decont_maxmin_lv2			= ptr->decont_maxmin_lv2;
	ipq_decontour_de_contour_minmax_reg.decont_maxmin_lv3			= ptr->decont_maxmin_lv3;
	ipq_decontour_de_contour_level_reg.decont_level_range_hi			= ptr->decont_level_range_hi;
	ipq_decontour_de_contour_level_reg.decont_level_range_md			= ptr->decont_level_range_md;
	ipq_decontour_de_contour_level_reg.decont_level_range_lo			= ptr->decont_level_range_lo;
	ipq_decontour_de_contour_level_reg.decont_blend_gain_md			= ptr->decont_blend_gain_md;
	ipq_decontour_de_contour_level_reg.decont_blend_gain_lo			= ptr->decont_blend_gain_lo;

	ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range0	= ptr->decont_bottleneck_range0;
	ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range1	= ptr->decont_bottleneck_range1;
	ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range2	= ptr->decont_bottleneck_range2;
	ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range3	= ptr->decont_bottleneck_range3;
	//ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range4	= ptr->decont_bottleneck_range4;

	ipq_decontour_de_contour_diff_clamp_reg.decont_diff_clamp_y 		= ptr->decont_diff_clamp_y;
	ipq_decontour_de_contour_diff_clamp_reg.decont_diff_clamp_c 		= ptr->decont_diff_clamp_c;

	// thresholds for info: set fixed values
	ipq_decontour_de_contour_contrast_y_reg.decont_contrast_th_y_info		= 200;
	ipq_decontour_de_contour_contrast_y_reg.decont_neighbor_diff_th_y_info	= 4;
	ipq_decontour_de_contour_contrast_y_reg.decont_gradation_th_y_info		= 10;
	ipq_decontour_de_contour_contrast_y_reg.decont_maxmindiff_th_y_info		= 32;
	ipq_decontour_de_contour_contrast_c_reg.decont_contrast_th_c_info		= 200;
	ipq_decontour_de_contour_contrast_c_reg.decont_neighbor_diff_th_c_info	= 4;
	ipq_decontour_de_contour_contrast_c_reg.decont_gradation_th_c_info		= 10;
	ipq_decontour_de_contour_contrast_c_reg.decont_maxmindiff_th_c_info		= 32;

	IoReg_Write32(IPQ_DECONTOUR_De_Contour_CTRL_reg, ipq_decontour_de_contour_ctrl_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Contrast_reg, ipq_decontour_de_contour_contrast_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Neighbor_Diff_reg, ipq_decontour_de_contour_neighbor_diff_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Threshold_reg, ipq_decontour_de_contour_threshold_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Diff_th_reg, ipq_decontour_de_contour_diff_th_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Bottleneck_reg, ipq_decontour_de_contour_bottleneck_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Diff_Clamp_reg, ipq_decontour_de_contour_diff_clamp_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Contrast_Y_reg, ipq_decontour_de_contour_contrast_y_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Contrast_C_reg, ipq_decontour_de_contour_contrast_c_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Contrast2_reg, ipq_decontour_de_contour_contrast2_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Contrast3_reg, ipq_decontour_de_contour_contrast3_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Dark_reg, ipq_decontour_de_contour_dark_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Gradation_reg, ipq_decontour_de_contour_gradation_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Minmax_reg, ipq_decontour_de_contour_minmax_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Level_reg, ipq_decontour_de_contour_level_reg.regValue);

	/*  DB */
	drvif_color_Set_I_De_Contour_DB_Apply();

	return TRUE;
}

unsigned char  drvif_color_Get_I_De_Contour(VIP_I_De_Contour_TBL *ptr)
{
	ipq_decontour_de_contour_ctrl_RBUS ipq_decontour_de_contour_ctrl_reg;
	ipq_decontour_de_contour_contrast_RBUS ipq_decontour_de_contour_contrast_reg;
	ipq_decontour_de_contour_neighbor_diff_RBUS ipq_decontour_de_contour_neighbor_diff_reg;
	ipq_decontour_de_contour_threshold_RBUS ipq_decontour_de_contour_threshold_reg;
	ipq_decontour_de_contour_diff_th_RBUS ipq_decontour_de_contour_diff_th_reg;
	ipq_decontour_de_contour_bottleneck_RBUS ipq_decontour_de_contour_bottleneck_reg;
	ipq_decontour_de_contour_diff_clamp_RBUS ipq_decontour_de_contour_diff_clamp_reg;
	ipq_decontour_de_contour_contrast2_RBUS ipq_decontour_de_contour_contrast2_reg;
	ipq_decontour_de_contour_contrast3_RBUS ipq_decontour_de_contour_contrast3_reg;
	ipq_decontour_de_contour_dark_RBUS ipq_decontour_de_contour_dark_reg;
	ipq_decontour_de_contour_gradation_RBUS ipq_decontour_de_contour_gradation_reg;
	ipq_decontour_de_contour_minmax_RBUS ipq_decontour_de_contour_minmax_reg;
	ipq_decontour_de_contour_level_RBUS ipq_decontour_de_contour_level_reg;

	ipq_decontour_de_contour_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_CTRL_reg);
	ipq_decontour_de_contour_contrast_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Contrast_reg);
	ipq_decontour_de_contour_neighbor_diff_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Neighbor_Diff_reg);
	ipq_decontour_de_contour_threshold_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Threshold_reg);
	ipq_decontour_de_contour_diff_th_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Diff_th_reg);
	ipq_decontour_de_contour_bottleneck_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Bottleneck_reg);
	ipq_decontour_de_contour_diff_clamp_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Diff_Clamp_reg);
	ipq_decontour_de_contour_contrast2_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Contrast2_reg);
	ipq_decontour_de_contour_contrast3_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Contrast3_reg);
	ipq_decontour_de_contour_dark_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Dark_reg);
	ipq_decontour_de_contour_gradation_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Gradation_reg);
	ipq_decontour_de_contour_minmax_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Minmax_reg);
	ipq_decontour_de_contour_level_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Level_reg);

	ptr->decont_en                   	= ipq_decontour_de_contour_ctrl_reg.decont_en;
	ptr->decont_snr_blend_mode	= ipq_decontour_de_contour_ctrl_reg.decont_snr_blend_mode;
	ptr->decont_blend_lowbd		= ipq_decontour_de_contour_ctrl_reg.decont_blend_lowbd;
	ptr->decont_blend_step		= ipq_decontour_de_contour_ctrl_reg.decont_blend_step;
	ptr->decont_blend_weight_lpf_en= ipq_decontour_de_contour_ctrl_reg.decont_blend_weight_lpf_en;
	ptr->decont_neighbor_far		= ipq_decontour_de_contour_ctrl_reg.decont_neighbor_far;
	//ptr->decont_filter_mode		= ipq_decontour_de_contour_ctrl_reg.decont_filter_mode;
	//ptr->decont_line_mode 		= ipq_decontour_de_contour_ctrl_reg.decont_line_mode;
	ptr->decont_yc_mode 			= ipq_decontour_de_contour_ctrl_reg.decont_yc_mode;
	ptr->decont_bottleneck_en 		= ipq_decontour_de_contour_ctrl_reg.decont_bottleneck_en;

	ptr->decont_contrast_th_y		= ipq_decontour_de_contour_contrast_reg.decont_contrast_th_y;
	ptr->decont_contrast_th2_y		= ipq_decontour_de_contour_contrast2_reg.decont_contrast_th2_y;
	ptr->decont_contrast_th3_y		= ipq_decontour_de_contour_contrast2_reg.decont_contrast_th3_y;
	ptr->decont_dark_th			= ipq_decontour_de_contour_threshold_reg.decont_dark_th;
	ptr->decont_dark_contrast_th	= ipq_decontour_de_contour_threshold_reg.decont_dark_contrast_th;
	ptr->decont_dark_contrast_th2	= ipq_decontour_de_contour_dark_reg.decont_dark_contrast_th2;
	ptr->decont_dark_contrast_th3	= ipq_decontour_de_contour_dark_reg.decont_dark_contrast_th3;
	ptr->decont_contrast_th_c		= ipq_decontour_de_contour_contrast_reg.decont_contrast_th_c;
	ptr->decont_contrast_th2_c		= ipq_decontour_de_contour_contrast3_reg.decont_contrast_th2_c;
	ptr->decont_contrast_th3_c		= ipq_decontour_de_contour_contrast3_reg.decont_contrast_th3_c;

	ptr->decont_neighbor_diff_th_y	= ipq_decontour_de_contour_neighbor_diff_reg.decont_neighbor_diff_th_y;
	ptr->decont_neighbor_diff_th2_y	= ipq_decontour_de_contour_neighbor_diff_reg.decont_neighbor_diff_th2_y;
	ptr->decont_neighbor_diff_th_c	= ipq_decontour_de_contour_neighbor_diff_reg.decont_neighbor_diff_th_c;
	ptr->decont_neighbor_diff_th2_c	= ipq_decontour_de_contour_neighbor_diff_reg.decont_neighbor_diff_th2_c;
	ptr->decont_gradation_th_y		= ipq_decontour_de_contour_threshold_reg.decont_gradation_th_y;
	ptr->decont_gradation_th2_y	= ipq_decontour_de_contour_gradation_reg.decont_gradation_th2_y;
	ptr->decont_gradation_th_c		= ipq_decontour_de_contour_threshold_reg.decont_gradation_th_c;
	ptr->decont_gradation_th2_c	= ipq_decontour_de_contour_gradation_reg.decont_gradation_th2_c;

	ptr->decont_maxmindiff_th_y	= ipq_decontour_de_contour_diff_th_reg.decont_maxmindiff_th_y;
	ptr->decont_maxmindiff_th2_y	= ipq_decontour_de_contour_minmax_reg.decont_maxmindiff_th2_y;
	ptr->decont_maxmindiff_th3_y	= ipq_decontour_de_contour_minmax_reg.decont_maxmindiff_th3_y;
	ptr->decont_maxmindiff_th_c 	= ipq_decontour_de_contour_diff_th_reg.decont_maxmindiff_th_c;
	ptr->decont_maxmin_lv0		= ipq_decontour_de_contour_minmax_reg.decont_maxmin_lv0;
	ptr->decont_maxmin_lv1		= ipq_decontour_de_contour_minmax_reg.decont_maxmin_lv1;
	ptr->decont_maxmin_lv2		= ipq_decontour_de_contour_minmax_reg.decont_maxmin_lv2;
	ptr->decont_maxmin_lv3		= ipq_decontour_de_contour_minmax_reg.decont_maxmin_lv3;

	ptr->decont_level_range_hi		= ipq_decontour_de_contour_level_reg.decont_level_range_hi;
	ptr->decont_level_range_md	= ipq_decontour_de_contour_level_reg.decont_level_range_md;
	ptr->decont_level_range_lo		= ipq_decontour_de_contour_level_reg.decont_level_range_lo;
	ptr->decont_blend_gain_md		= ipq_decontour_de_contour_level_reg.decont_blend_gain_md;
	ptr->decont_blend_gain_lo		= ipq_decontour_de_contour_level_reg.decont_blend_gain_lo;

	ptr->decont_bottleneck_range0    = ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range0;
	ptr->decont_bottleneck_range1    = ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range1;
	ptr->decont_bottleneck_range2    = ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range2;
	ptr->decont_bottleneck_range3    = ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range3;
	//ptr->decont_bottleneck_range4    = ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range4;
	ptr->decont_diff_clamp_y 			= ipq_decontour_de_contour_diff_clamp_reg.decont_diff_clamp_y;
	ptr->decont_diff_clamp_c			= ipq_decontour_de_contour_diff_clamp_reg.decont_diff_clamp_c;

	return TRUE;
}

unsigned char drvif_color_Get_I_De_Contour_Stats(VIP_I_De_Contour_Statistic* ptr)
{
	int i;
	unsigned short timeout;
	ipq_decontour_de_contour_counter_RBUS de_contour_counter_reg;
	ipq_decontour_de_contour_avfilter_counter_RBUS de_contour_avfilter_counter_reg;
	ipq_decontour_de_contour_hist_ctrl_RBUS de_contour_hist_ctrl_reg;
	ipq_decontour_de_contour_hist_data_RBUS de_contour_hist_data_reg;
	ipq_decontour_de_contour_ydiff_hist_RBUS de_contour_ydiff_hist_reg;
	ipq_decontour_de_contour_cdiff_hist_RBUS de_contour_cdiff_hist_reg;

	de_contour_counter_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Counter_reg);
	ptr->decont_contour_cnt = de_contour_counter_reg.decont_contour_cnt;

	de_contour_avfilter_counter_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Avfilter_Counter_reg);
	ptr->decont_avfilter_cnt = de_contour_avfilter_counter_reg.decont_avfilter_cnt;

	// contour range hist
	de_contour_hist_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Hist_Ctrl_reg);
	de_contour_hist_ctrl_reg.decont_filter_range_hist_read_mode = 0;
	de_contour_hist_ctrl_reg.decont_filter_range_hist_locken = 0;
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_Hist_Ctrl_reg, de_contour_hist_ctrl_reg.regValue);

	timeout = 255;

	for( i=0; i<16; i++ )
	{
		de_contour_hist_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Hist_Ctrl_reg);
		de_contour_hist_ctrl_reg.decont_filter_range_hist_rden = 1;
		de_contour_hist_ctrl_reg.decont_filter_range_hist_raddr = i;
		IoReg_Write32(IPQ_DECONTOUR_De_Contour_Hist_Ctrl_reg, de_contour_hist_ctrl_reg.regValue);

		do {
			de_contour_hist_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Hist_Ctrl_reg);
			timeout--;
			if( timeout <= 0 )
			{
				// read timeout!
				//rtd_pr_vpq_emerg("Read Decont Hist TIMEOUT!!!" );
				return 0;
			}

		} while( de_contour_hist_ctrl_reg.decont_filter_range_hist_rden );

		de_contour_hist_data_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_Hist_Data_reg);
		ptr->decont_range_hist[i] = de_contour_hist_data_reg.decont_filter_range_hist;
	}

	// ydiff hist
	de_contour_ydiff_hist_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_YDIFF_Hist_reg);
	de_contour_ydiff_hist_reg.decont_info_ydiff_hist_read_mode = 0;
	de_contour_ydiff_hist_reg.decont_info_ydiff_hist_locken = 0;
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_YDIFF_Hist_reg, de_contour_ydiff_hist_reg.regValue);

	timeout = 255;

	for( i=0; i<8; i++ )
	{
		de_contour_ydiff_hist_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_YDIFF_Hist_reg);
		de_contour_ydiff_hist_reg.decont_info_ydiff_hist_rden = 1;
		de_contour_ydiff_hist_reg.decont_info_ydiff_hist_raddr = i;
		IoReg_Write32(IPQ_DECONTOUR_De_Contour_YDIFF_Hist_reg, de_contour_ydiff_hist_reg.regValue);

		timeout = 255;
		do {
			de_contour_ydiff_hist_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_YDIFF_Hist_reg);
			timeout--;
			if( timeout <= 0 )
			{
				// read timeout!
				//rtd_pr_vpq_emerg("Read YDiff Hist TIMEOUT!!!" );
				return 0;
			}

		} while( de_contour_ydiff_hist_reg.decont_info_ydiff_hist_rden );

		ptr->decont_ydiff_hist[i] = de_contour_ydiff_hist_reg.decont_info_ydiff_hist;
	}
      //  rtd_pr_vpq_emerg("ydiff_hist: %d %d %d %d %d %d %d %d", ptr->decont_ydiff_hist[0], ptr->decont_ydiff_hist[1], ptr->decont_ydiff_hist[2], ptr->decont_ydiff_hist[3],
      //          ptr->decont_ydiff_hist[4], ptr->decont_ydiff_hist[5], ptr->decont_ydiff_hist[6], ptr->decont_ydiff_hist[7]);


	// cdiff hist
	de_contour_cdiff_hist_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_CDIFF_Hist_reg);
	de_contour_cdiff_hist_reg.decont_info_cdiff_hist_read_mode = 0;
	de_contour_cdiff_hist_reg.decont_info_cdiff_hist_locken = 0;
	IoReg_Write32(IPQ_DECONTOUR_De_Contour_CDIFF_Hist_reg, de_contour_cdiff_hist_reg.regValue);

	timeout = 255;

	for( i=0; i<8; i++ )
	{
		de_contour_cdiff_hist_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_CDIFF_Hist_reg);
		de_contour_cdiff_hist_reg.decont_info_cdiff_hist_rden = 1;
		de_contour_cdiff_hist_reg.decont_info_cdiff_hist_raddr = i;
		IoReg_Write32(IPQ_DECONTOUR_De_Contour_CDIFF_Hist_reg, de_contour_cdiff_hist_reg.regValue);

		timeout = 255;
		do {
			de_contour_cdiff_hist_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_CDIFF_Hist_reg);
			timeout--;
			if( timeout <= 0 )
			{
				// read timeout!
				//rtd_pr_vpq_emerg("Read CDiff Hist TIMEOUT!!!" );
				return 0;
			}

		} while( de_contour_cdiff_hist_reg.decont_info_cdiff_hist_rden );

		ptr->decont_cdiff_hist[i] = de_contour_cdiff_hist_reg.decont_info_cdiff_hist;
	}

	return 1;
}


void drvif_color_I_De_Contour_line_buffer_check(void)
{
	iedge_smooth_edgesmooth_ex_ctrl_RBUS edgesmooth_ex_ctrl_reg;
	edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg);

	// I_De_Contour only worked on SNR line buffer 11 line mode
	if (edgesmooth_ex_ctrl_reg.lbuf_linemode != 2) { //0:3 line, 1:5 line, 2:11 line
		//disable I_De_Contour
		IoReg_ClearBits(IPQ_DECONTOUR_De_Contour_CTRL_reg, IPQ_DECONTOUR_De_Contour_CTRL_decont_en_mask);
		IoReg_ClearBits(IPQ_DECONTOUR_De_Contour_db_reg_ctl_reg, IPQ_DECONTOUR_De_Contour_db_reg_ctl_db_apply_mask);
	}
}

void drvif_color_Set_DRV_SNR_Clock(SCALER_DISP_CHANNEL display, unsigned char En_flag)
{
	UINT32 mask = (display == SLR_MAIN_DISPLAY) ?
				VGIP_Data_Path_Select_s1_clk_en_mask :
				VGIP_Data_Path_Select_s2_clk_en_mask;

	if (En_flag)
		IoReg_SetBits(VGIP_Data_Path_Select_reg, mask);
	else
		IoReg_ClearBits(VGIP_Data_Path_Select_reg, mask);
}

unsigned char drvif_color_Get_DRV_SNR_Clock(SCALER_DISP_CHANNEL display, unsigned char En_flag)
{
	if (SLR_SUB_DISPLAY == display)
		return VGIP_Data_Path_Select_get_s2_clk_en(IoReg_Read32(VGIP_Data_Path_Select_reg));
	else
		return VGIP_Data_Path_Select_get_s1_clk_en(IoReg_Read32(VGIP_Data_Path_Select_reg));
}

void drvif_color_set_DI_RTNR_Timing_Measurement(void)
{
	di_ccdinr_timing_monitor_ctrl_RBUS di_ccdinr_timing_monitor_ctrl_reg;
	di_ccdinr_timing_monitor_multi_RBUS di_ccdinr_timing_monitor_multi_reg;

	di_ccdinr_timing_monitor_ctrl_1_RBUS di_ccdinr_timing_monitor_ctrl_1_reg;
	di_ccdinr_timing_monitor_multi_1_RBUS di_ccdinr_timing_monitor_multi_1_reg;
	
	// RTNR
	di_ccdinr_timing_monitor_ctrl_reg.regValue = IoReg_Read32(DI_ccdinr_Timing_monitor_ctrl_reg);
	di_ccdinr_timing_monitor_multi_reg.regValue = IoReg_Read32(DI_ccdinr_Timing_monitor_multi_reg);

	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_en = 1;
	//di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_i2rnd_mode = 0;
	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_src = 0;
	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_unit = 0;
	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_multi_mode = 1;
	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_compare = 0;

	// v
	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_sta0 = 2;	// vact rinsing
	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_end0 = 3; 	// vact falling
	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_sta1 = 3; 	// vact falling
	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_end1 = 0;	// vs rising
	di_ccdinr_timing_monitor_multi_reg.timing_monitor_sta2 = 0;		// vs rising
	di_ccdinr_timing_monitor_multi_reg.timing_monitor_end2 = 2;	 	// vact rinsing

	// h
	di_ccdinr_timing_monitor_multi_reg.timing_monitor_sta3 = 6;	// hact rising
	di_ccdinr_timing_monitor_multi_reg.timing_monitor_end3 = 7;  // hact falling 
	di_ccdinr_timing_monitor_multi_reg.timing_monitor_sta4 = 7;	// hact falling
	di_ccdinr_timing_monitor_multi_reg.timing_monitor_end4 = 6;	// hact rising
	di_ccdinr_timing_monitor_multi_reg.timing_monitor_sta5 = 8;	// den rising
	di_ccdinr_timing_monitor_multi_reg.timing_monitor_end5 = 8;	// den rising

	IoReg_Write32(DI_ccdinr_Timing_monitor_ctrl_reg, di_ccdinr_timing_monitor_ctrl_reg.regValue);
	IoReg_Write32(DI_ccdinr_Timing_monitor_multi_reg, di_ccdinr_timing_monitor_multi_reg.regValue);



	//	DI
	di_ccdinr_timing_monitor_ctrl_1_reg.regValue = IoReg_Read32(DI_ccdinr_Timing_monitor_ctrl_1_reg);
	di_ccdinr_timing_monitor_multi_1_reg.regValue = IoReg_Read32(DI_ccdinr_Timing_monitor_multi_1_reg);

	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_en_1 = 1;
	//di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_i2rnd_mode_1 = 0;
	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_src_1 = 1;
	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_unit_1 = 0;
	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_multi_mode_1 = 1;
	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_compare_1 = 0;

	// v
	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_sta0_1 = 2;	// vact rinsing
	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_end0_1 = 3; // vact falling
	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_sta1_1 = 3;
	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_end1_1 = 0;	// vs rising
	di_ccdinr_timing_monitor_multi_1_reg.timing_monitor_sta2_1 = 0;
	di_ccdinr_timing_monitor_multi_1_reg.timing_monitor_end2_1 = 2; // vact rinsing

	// h
	di_ccdinr_timing_monitor_multi_1_reg.timing_monitor_sta3_1 = 6;	// hact rising
	di_ccdinr_timing_monitor_multi_1_reg.timing_monitor_end3_1 = 7;  // hact falling 
	di_ccdinr_timing_monitor_multi_1_reg.timing_monitor_sta4_1 = 7;	// hact falling
	di_ccdinr_timing_monitor_multi_1_reg.timing_monitor_end4_1 = 6;	// hact rising
	di_ccdinr_timing_monitor_multi_1_reg.timing_monitor_sta5_1 = 8;	// den rising
	di_ccdinr_timing_monitor_multi_1_reg.timing_monitor_end5_1 = 8;	// den rising

	IoReg_Write32(DI_ccdinr_Timing_monitor_ctrl_1_reg, di_ccdinr_timing_monitor_ctrl_1_reg.regValue);
	IoReg_Write32(DI_ccdinr_Timing_monitor_multi_1_reg, di_ccdinr_timing_monitor_multi_1_reg.regValue);
	
}

char drvif_color_Get_DI_RTNR_Timing_Measurement(unsigned int *pRTNR, unsigned int *pDI)
{
	pRTNR[0] = DI_ccdinr_Timing_monitor_result1_get_t_m_result_previous1(IoReg_Read32(DI_ccdinr_Timing_monitor_result1_reg));
	pRTNR[1] = DI_ccdinr_Timing_monitor_result2_get_t_m_result_previous2(IoReg_Read32(DI_ccdinr_Timing_monitor_result2_reg));
	pRTNR[2] = DI_ccdinr_Timing_monitor_result3_get_t_m_result_previous3(IoReg_Read32(DI_ccdinr_Timing_monitor_result3_reg));
	pRTNR[3] = DI_ccdinr_Timing_monitor_result4_get_t_m_result_previous4(IoReg_Read32(DI_ccdinr_Timing_monitor_result4_reg));
	pRTNR[4] = DI_ccdinr_Timing_monitor_max_get_t_m_result_max(IoReg_Read32(DI_ccdinr_Timing_monitor_max_reg));
	pRTNR[5] = DI_ccdinr_Timing_monitor_min_get_t_m_result_min(IoReg_Read32(DI_ccdinr_Timing_monitor_min_reg));

	pDI[0] = DI_ccdinr_Timing_monitor_result1_1_get_t_m_result_previous1_1(IoReg_Read32(DI_ccdinr_Timing_monitor_result1_1_reg));
	pDI[1] = DI_ccdinr_Timing_monitor_result2_1_get_t_m_result_previous2_1(IoReg_Read32(DI_ccdinr_Timing_monitor_result2_1_reg));
	pDI[2] = DI_ccdinr_Timing_monitor_result3_1_get_t_m_result_previous3_1(IoReg_Read32(DI_ccdinr_Timing_monitor_result3_1_reg));
	pDI[3] = DI_ccdinr_Timing_monitor_result4_1_get_t_m_result_previous4_1(IoReg_Read32(DI_ccdinr_Timing_monitor_result4_1_reg));
	pDI[4] = DI_ccdinr_Timing_monitor_max_1_get_t_m_result_max_1(IoReg_Read32(DI_ccdinr_Timing_monitor_max_1_reg));
	pDI[5] = DI_ccdinr_Timing_monitor_min_1_get_t_m_result_min_1(IoReg_Read32(DI_ccdinr_Timing_monitor_min_1_reg));

	return 0;
}

DRV_SNR_Mosquito_NR mnr_demo = {9, 1, 1, 1, 1, 0, 0, 255, 0, 4, 1, 2, 1, 2, 0, 7, 73, 0, 7, 7, 50, 1, 0, 75, 0, 75, 1, 0, 1, 1, 7, 0};
DRV_SNR_Modified_LPF mnr_lpf_demo ={3, 4, 619, 0, 4, 5};
void drvif_color_DRV_SNR_Mosquito_NR_DEMO(void)
{
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_RBUS nr_mosquito_nr_level_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_1_RBUS nr_mosquito_nr_level_ctrl_1_RBUS_reg;
    nr_mosquito_nr_keep_edge_ctrl_RBUS 	nr_mosquito_nr_keep_edge_ctrl_reg;
    nr_mosquito_nr_decay_ctrl_1_RBUS   	nr_mosquito_nr_decay_ctrl_1_reg;

	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	nr_mosquito_nr_level_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_reg);
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg);
	nr_mosquito_nr_keep_edge_ctrl_reg.regValue 		= IoReg_Read32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg);
	nr_mosquito_nr_decay_ctrl_1_reg.regValue		= IoReg_Read32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg);
	/*Mosquito_NR_MNR Global Control*/

	nr_mosquito_ctrl_RBUS_reg.mosquitonr_weight = mnr_demo.SNR_mosquitonr_weight;
	nr_mosquito_ctrl_RBUS_reg.mosquitonr_blending_mode = mnr_demo.SNR_mosquitonr_blending_mode;
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.noise_level_mode = mnr_demo.SNR_noise_level_mode;
	nr_mosquito_ctrl_RBUS_reg.mosquitonr_vertical_en 		= mnr_demo.SNR_vertical_en; /*Mac3*/
    	nr_mosquito_nr_level_ctrl_1_RBUS_reg.edge_lpf_en		= mnr_demo.SNR_edge_lpf_en;
	nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_en			= mnr_demo.SNR_mnr_decay_en;
	nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_allcomp_en			= mnr_demo.SNR_mnr_decay_allcomp_en; // Merlin4 add
	nr_mosquito_ctrl_RBUS_reg.mnr_adjustnoiselevel_bytype = mnr_demo.MNR_adjustNoiseLevel_byType;/*Merlin jyyang 20150615*/
	nr_mosquito_ctrl_RBUS_reg.mnr_vardiff_refine_2dsearch = mnr_demo.MNR_varDiff_refine_2Dsearch;
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.noise_level_trim		= mnr_demo.MNR_nLevel_trim;/*Merlin jyyang 20150707*/
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_half_range		= mnr_demo.MNR_hdiff_frac_half_range;/*Merlin jyyang 20150707*/
	/*Mosquito_NR_Edge Condition*/
	//nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd = ptr->SNR_mosquito_edgethd;
	//nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd_step = ptr->SNR_mosquito_edgethd_step;
	nr_mosquito_ctrl_RBUS_reg.mosquitonr_edge_strength_gain = mnr_demo.SNR_mosquitonr_edge_strength_gain;
    /*decay*/
    nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_start		= mnr_demo.SNR_hdecay_start; /*Mac3*/
    nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_step			= mnr_demo.SNR_hdecay_step;
    nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_start		= mnr_demo.SNR_vdecay_start;
    nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_step			= mnr_demo.SNR_vdecay_step;
	/*Mosquito_NR_Noise level adjust*/
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.farvar_faredge_refine_en 	= mnr_demo.SNR_farvar_faredge_refine_en;
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_gain = mnr_demo.SNR_far_var_gain;
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_offset				= mnr_demo.SNR_far_var_offset;
    nr_mosquito_nr_level_ctrl_RBUS_reg.mnr_vardiff_refine_gain 		= mnr_demo.SNR_vardiff_refine_gain;
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_shift			= mnr_demo.SNR_hdiff_frac_shift;
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.vdiff_frac_shift			= mnr_demo.SNR_vdiff_frac_shift;

	/*keep edge*/
	nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd				= mnr_demo.SNR_keep_edge_threshold;
    nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd_step			= mnr_demo.SNR_keep_edge_threshold_step;
    nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedge_shift			= mnr_demo.SNR_keep_edge_shift;

	/*Mosquito_NR_Final MNR Level Adjust*/
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_offset = mnr_demo.SNR_mosquitonr_nlevel_positive_offset;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_shift = mnr_demo.SNR_mosquitonr_nlevel_positive_shift;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_offset = mnr_demo.SNR_mosquitonr_nlevel_negative_offset;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_shift = mnr_demo.SNR_mosquitonr_nlevel_negative_shift;

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask); //DB en
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_reg, nr_mosquito_nr_level_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg, nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg, nr_mosquito_nr_keep_edge_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg, nr_mosquito_nr_decay_ctrl_1_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); //DB apply
}
void drvif_color_DRV_SNR_Modified_LPF_DEMO(void)
{

	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_new_lpf_nearby_ctrl_RBUS new_lpf_nearby_ctrl_reg;
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	//nr_edge_filter_RBUS nr_edge_filter_reg; /*Merlin2*/
	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	new_lpf_nearby_ctrl_reg.regValue = IoReg_Read32(NR_NEW_LPF_NEARBY_CTRL_reg);
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	//nr_edge_filter_reg.regValue = IoReg_Read32(NR_EDGE_FILTER_reg);  /*Merlin2*/

	/*modified lpf*/
	dch1_cp_ctrl_reg.vc_snr_lpf_sel = mnr_lpf_demo.SNR_LPF_sel; // 0:3, 1:5, 2:mod3, 3:mod5, 4:11, 5:mod11 (Merlin, jyyang@20150615)(Merlin2, jyyang@20160617)
	new_lpf_nearby_ctrl_reg.modified_lpf_edge_gain	= mnr_lpf_demo.SNR_modified_lpf_edge_gain; /*Mac3, leonard wu@20140722*/
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_upbnd	= mnr_lpf_demo.SNR_modified_lpf_thd_upbnd;
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_lowbnd = mnr_lpf_demo.SNR_modified_lpf_thd_lowbnd;
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_step 	= mnr_lpf_demo.SNR_modified_lpf_thd_step;

	//Merlin2, jyyang@20160617
	nr_mosquito_ctrl_RBUS_reg.mnr_lpf_sel = mnr_lpf_demo.MNR_LPF_sel;

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(NR_NEW_LPF_NEARBY_CTRL_reg, new_lpf_nearby_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	//IoReg_Write32(NR_EDGE_FILTER_reg, nr_edge_filter_reg.regValue);  /*Merlin2*/
}

void drvif_color_DRV_SNR_Mosquito_NR_reset_DEMO(void)
{
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);

	nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd = mnr_demo.SNR_mosquito_edgethd;
	nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd_step = mnr_demo.SNR_mosquito_edgethd_step;

	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
}
