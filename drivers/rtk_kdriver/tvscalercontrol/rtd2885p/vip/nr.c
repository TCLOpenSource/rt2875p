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
/*#include <unistd.h>			*/ /* 'close()'*/
/*#include <fcntl.h>			*/ /* 'open()'*/
/*#include <stdio.h>*/
/*#include <sys/ioctl.h>*/
/*#include <string.h>*/

//#include <mach/io.h>
/*#include "rtd_regs.h"*/
#include <tvscalercontrol/io/ioregdrv.h>
/*#include "libs/utils/rtSystem.h"*/
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/nr.h>
/*#include "kernel/scaler/scalerDrv.h"	*/ /* kernel space scalerDrv*/
#include <tvscalercontrol/scalerdrv/scalerdrv.h>	/* user space scalerDrv*/
#include <tvscalercontrol/hdmirx/hdmifun.h>
/*#include <Platform_Lib/panel/panelAPI.h>*/
#ifndef BUILD_QUICK_SHOW
#include <linux/semaphore.h>
#else
#include <no_os/semaphore.h>
#include <malloc.h>
#endif
/* oliver+*/

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerDrvCommon.h>
#endif

/*#include <tvscalercontrol/scaler/scalerlib.h>*/
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/vdc/video.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"
#include <scaler_vbedev.h>


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

extern UINT8 TNR_Y_Store_Mad[15][10];
extern UINT8 TNR_C_Store_Mad[7][10];
extern struct semaphore VPQ_DI_RTNR_CONTROL_Semaphore;
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

// blk decont info
unsigned int flat_ratio=0, texture_ratio=0;

DRV_SNR_Sub_Table sub_snr_tbl = {
	/* 0:hfiltery, 1:hfilter_coef0_y, 2:hfilter_coef1_y, 3:hfilter_coef2_y, 4:hfilterc, 5:hfilter_coef0_c, 6:hfilter_coef1_c, 7:hfilter_coef2_c*/
	0, 2, 3, 6, 0, 3, 4, 2
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

void drvif_color_TSB_noisereduction_impulse(unsigned char channel , unsigned char level, unsigned char bNoiseReductionMode)
{
#if 0// Merlin4 remove
	UINT8 input_src_type;
	/*short value;*/
	unsigned char impulse_check[1];
	unsigned char i, impulse_check_count = 0, check_loop_count = 0;

	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*if (!drvif_color_nrrestrict()) {*/ /*if widt > h1440 disable NR then return*/
	/*	return;*/
	/*}*/
	/*middleware: 01: no nr, 11: nr*/
	if (bNoiseReductionMode == 0x0 || bNoiseReductionMode == 0x02)
		return;
	if (channel == _CHANNEL1) {
		nr_dch1_cp_ctrl_RBUS dch1_CP_Ctrl_reg;
		nr_dch1_impulse_thl_RBUS dch1_Impulse_Thl_reg;
		nr_dch1_impulse_smooththl_RBUS dch1_Impulse_SmoothThl_reg;

		dch1_CP_Ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);

		input_src_type = VIP_system_info_structure_table->Input_src_Type;

		if (input_src_type == _SRC_TV) {
			 if (level == 0) {
				dch1_Impulse_Thl_reg.cp_impulsewindow = 1;
				dch1_Impulse_Thl_reg.cp_iresultweight = 7;
				dch1_CP_Ctrl_reg.cp_impulseenable = 0x0;
			 } else {
				dch1_Impulse_Thl_reg.cp_impulsewindow = 1;
				dch1_Impulse_Thl_reg.cp_iresultweight = 7;
				dch1_CP_Ctrl_reg.cp_impulseenable = 0x1;/*enable impulse NR*/
			}
			/*RTD_Log(LOGGER_INFO,"======>TV_level =%d\n", level);*/
			/*RTD_Log(LOGGER_INFO,"======>dch1_CP_Ctrl_reg.cp_impulseenable =%d\n", dch1_CP_Ctrl_reg.cp_impulseenable);*/
		} else if ((input_src_type == _SRC_CVBS) || (input_src_type == _SRC_YPBPR) || (input_src_type == _SRC_HDMI)) {
			if ((level == 0) || (level == 1))
				dch1_CP_Ctrl_reg.cp_impulseenable = 0x0;
		}

		IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_CP_Ctrl_reg.regValue);

		/*=====================================================================*/

		dch1_Impulse_Thl_reg.regValue = IoReg_Read32(NR_DCH1_Impulse_Thl_reg);
		dch1_Impulse_SmoothThl_reg.regValue = IoReg_Read32(NR_DCH1_Impulse_SmoothThl_reg);
		dch1_Impulse_Thl_reg.cp_ipixeldiffthly = 0;
		dch1_Impulse_Thl_reg.cp_ipixeldiffthlc = 0;
		dch1_Impulse_Thl_reg.cp_impulsethly = 10;
		dch1_Impulse_Thl_reg.cp_impulsethlc = 10;
		dch1_Impulse_SmoothThl_reg.cp_impulsethlyx2 = 150;
		dch1_Impulse_SmoothThl_reg.cp_impulsethlcx2 = 150;
		dch1_Impulse_SmoothThl_reg.cp_impulsesmooththly = 20;
		dch1_Impulse_SmoothThl_reg.cp_impulsesmooththlc = 20;

		IoReg_Write32(NR_DCH1_Impulse_Thl_reg, dch1_Impulse_Thl_reg.regValue);
		IoReg_Write32(NR_DCH1_Impulse_SmoothThl_reg, dch1_Impulse_SmoothThl_reg.regValue);


		impulse_check[0] = dch1_CP_Ctrl_reg.cp_impulseenable;
		/*========= for check double buffer write error =================================*/
		do {
			IoReg_BusyCheckRegBit(VGIP_VGIP_CHN1_STATUS_reg, _BIT26, 0x0001FFFF);/*frank@0430 change wait vsync start*/
			/*WaitFor_IEN_STOP1();*/

			dch1_CP_Ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
			impulse_check_count = 0;
			check_loop_count++;

			for (i = 0; i < 1; i++) {
				switch (i) {
				case 0:
					if (impulse_check[0] == dch1_CP_Ctrl_reg.cp_impulseenable)
						impulse_check_count++;
					else {
						dch1_CP_Ctrl_reg.cp_impulseenable = impulse_check[0];
					}
					break;
				default:
					break;
				}

			}

			if (impulse_check_count < 1) {

				IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_CP_Ctrl_reg.regValue);
			}
/*			else*/
/*				VIPprintf("impuls reg write setting [OK]\n");*/

		} while (impulse_check_count < 1 && check_loop_count < 6);
	/*=================================================================*/


	}
#endif
}


/**
 * CleanPicture
 * Select impulse noise reduction level:0-255
**/
void drvif_color_noisereduction_impulse(unsigned char channel , unsigned char level, unsigned char bNoiseReductionMode)
{
#if 0// Merlin4 remove

#if  defined(IS_TV05_STYLE_PICTUREMODE)
	drvif_color_TSB_noisereduction_impulse(channel, level, bNoiseReductionMode);
	return;
#else
	signed short value;

	/*if (!drvif_color_nrrestrict()) {*/ /*if widt > h1440 disable NR then return*/
	/*	return;*/
	/*}*/
	/*middleware: 01: no nr, 11: nr*/
	if (bNoiseReductionMode == 0x0 || bNoiseReductionMode == 0x02)
		return;

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
#endif
#endif
}


/**
 * CleanPicture
 * Select spatial noise reduction for C level:0-255
**/
#if 0
void drivf_color_rtnr_correction_set(unsigned char level)
{
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;


		di_im_di_rtnr_control_RBUS im_di_rtnr_control_RBUS_reg;
		im_di_rtnr_control_RBUS_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);
		im_di_rtnr_control_RBUS_reg.cp_rtnr_y_enable = 0x1;
	if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_400_mode){
		im_di_rtnr_control_RBUS_reg.cp_rtnr_c_enable = 0x0;
	}
	else{
		im_di_rtnr_control_RBUS_reg.cp_rtnr_c_enable = 0x1;
	}
		im_di_rtnr_control_RBUS_reg.cp_rtnr_mad_window = 0x3;/*0: 1x3 window, 1: 1x5 window, 2: 3x3 window, 3: 3x5 window*/
		/*	VIPprintf("level = %d \n", level);*/
		switch (level) {

		case 0:
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
	// chen 180308 remove
	//	im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 0;
		break;

		case 1:
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 1;
	// chen 180308 remove
//		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 0;
		break;

		case 2:
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
		// chen 180308 remove
//		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 1;
		break;

		default:
		im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
		// chen 180308 remove
	//	im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en = 0;
		break;
		}

	// chen 180308 remove
/*
		switch (level) {
		case 2:
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
*/
// end chen 180308 remove

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
#endif

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
	down(&VPQ_DI_RTNR_CONTROL_Semaphore);
	RecursiveTemporalNRCtrl_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);
	RecursiveTemporalNRCtrl_reg.cp_rtnr_y_enable = 0x1;
	if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_400_mode)
		RecursiveTemporalNRCtrl_reg.cp_rtnr_c_enable = 0x0;
	else
	RecursiveTemporalNRCtrl_reg.cp_rtnr_c_enable = 0x1;

	IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, RecursiveTemporalNRCtrl_reg.regValue);
	up(&VPQ_DI_RTNR_CONTROL_Semaphore);
	db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
	db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);


}
#if 0
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
	// chen 170522
//	rtnr_new_Ctrl.rtnr_new_method_en = 0x0;
// end chen 170522
	rtnr_new_Ctrl.prtnr_4k2k = 0x0;

	rtd_outl(DI_IM_DI_RTNR_NEW_CONTROL_reg, rtnr_new_Ctrl.regValue);

	db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
	db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);

}
#endif
void drvif_color_rtnr_progressive_check(unsigned char interlace_flag, unsigned char channel)
{

/*Set RTNR Progressive mode:*/
	di_im_di_rtnr_control_RBUS RecursiveTemporalNRCtrl_reg;
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;
	down(&VPQ_DI_RTNR_CONTROL_Semaphore);
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
	IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);
	up(&VPQ_DI_RTNR_CONTROL_Semaphore);
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
	//unsigned short sta_H,sta_;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (channel != _CHANNEL1)
		return;

	//sta_H	= VIP_system_info_structure_table->IH_Start;
	//sta_V	= VIP_system_info_structure_table->IV_Start;
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
	IoReg_Write32(MPEGNR_ICH1_MPEGNR2_reg, ich1_mpegnr2_reg.regValue);
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

	//add by Elsie START
	di_im_new_mcnr_control2_RBUS new_mcnr_control2_reg;
	new_mcnr_control2_reg.regValue = IoReg_Read32(DI_IM_NEW_MCNR_CONTROL2_reg);

	if (level <= 5)/*(level >= 0 && level <= 5)  yuan1024  mark it to avoid error!*/
		new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = level;

	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL2_reg, new_mcnr_control2_reg.regValue);
	//add by Elsie END

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

// chen 170522
#if 0
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
#endif
// end chen 170522

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
	int ret;
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

	ret = Scaler_SendRPC(SCALERIOC_SET_RTNR_K_FORCE, param, RPC_SEND_VALUE_ONLY);
	if (ret < 0) {
		VIPprintf("ret = %d, Set RTNR_K_Force fail !!!\n", ret);
		return 1;
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
	vfilter_0_reg.avlpf_en = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(VLPF_Coef_0[select][5]);
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

	IoReg_Write32(NR_CURVE_MAPPING_CTRL_reg, curve_mapping_ctrl_reg.regValue);
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

void drvif_color_nr_curvemapping_weight1_set(unsigned short w1[9])
{
	nr_curve_mapping_w1_1_RBUS curve_mapping_w1_1_reg;
	nr_curve_mapping_w1_2_RBUS curve_mapping_w1_2_reg;
	nr_curve_mapping_w1_3_RBUS curve_mapping_w1_3_reg;

	curve_mapping_w1_1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_1_reg);
	curve_mapping_w1_2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_2_reg);
	curve_mapping_w1_3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_3_reg);

	curve_mapping_w1_1_reg.curvemapping_w1_0 = w1[0];
	curve_mapping_w1_1_reg.curvemapping_w1_1 = w1[1];
	curve_mapping_w1_1_reg.curvemapping_w1_2 = w1[2];
	curve_mapping_w1_1_reg.curvemapping_w1_3 = w1[3];
	curve_mapping_w1_2_reg.curvemapping_w1_4 = w1[4];
	curve_mapping_w1_2_reg.curvemapping_w1_5 = w1[5];
	curve_mapping_w1_2_reg.curvemapping_w1_6 = w1[6];
	curve_mapping_w1_2_reg.curvemapping_w1_7 = w1[7];
	curve_mapping_w1_3_reg.curvemapping_w1_8 = w1[8];

	IoReg_Write32(NR_CURVE_MAPPING_W1_1_reg, curve_mapping_w1_1_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W1_2_reg, curve_mapping_w1_2_reg.regValue);
	IoReg_Write32(NR_CURVE_MAPPING_W1_3_reg, curve_mapping_w1_3_reg.regValue);
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


#if 0//Merlin3
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
#else // Merlin4
/*0:spatial sel
0: 0,    0,		0,	 1, 1, 1, 0,   0,    0
1: 0,	 0,		0.5, 1, 1, 1, 0.5, 0,    0
2: 0,	 0.25,	0.5, 1, 1, 1, 0.5, 0.25, 0
3: 0.25, 0.25,	0.5, 1, 1, 1, 0.5, 0.25, 0.25
*/

/* for Zoran filter adjust parameter*/
/* 0: spatial sel*/
/* Y diff - 1:SpatialThlY  2:SpatialThlYx2  3:SpatialThlYx4*/
static unsigned char SNR_Zoran_filter_set_Y_tbl[5][4] = {
		/*    0   1   2    3*/
	/*0 */{	0,  5, 10, 15 },
	/*1 */{	1, 10, 20, 30 },
	/*2 */{	2,  5, 10, 15 },
	/*3 */{	3, 10, 20, 30 },
	/*4 */{ 1, 10, 20, 30 },
};
/* 0:spatial sel*/
/* C diff - 1:SpatialThlC  2:SpatialThlCx2  3:SpatialThlCx4*/
static unsigned char SNR_Zoran_filter_set_C_tbl[5][4] = {
		/*    0   1   2    3*/
	/*0 */{	0,  5, 10, 15 },
	/*1 */{	1, 10, 20, 30 },
	/*2 */{	2,  5, 10, 15 },
	/*3 */{	3, 10, 20, 30 },
	/*4 */{ 0, 10, 20, 30 },
};

#endif

void drvif_color_nr_Zoran_filter_set_Y(int select)
{
	nr_dch1_cp_ctrl_RBUS nr_dch1_cp_ctrl_RBUS_reg;
	nr_dch1_spatial_thl_RBUS nr_dch1_spatial_thl_RBUS_reg;
	nr_dch1_spatial_thl2_RBUS nr_dch1_spatial_thl2_RBUS_reg;

	nr_dch1_cp_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	nr_dch1_spatial_thl_RBUS_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl_reg);
	nr_dch1_spatial_thl2_RBUS_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl2_reg);

	#if 0//Merlin3
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranfiltersizey = SNR_Zoran_filter_set_Y_tbl[select][0];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight2y = SNR_Zoran_filter_set_Y_tbl[select][1];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight3y = SNR_Zoran_filter_set_Y_tbl[select][2];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight4y = SNR_Zoran_filter_set_Y_tbl[select][3];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthly = SNR_Zoran_filter_set_Y_tbl[select][4];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthlyx2 = SNR_Zoran_filter_set_Y_tbl[select][5];
	nr_dch1_spatial_thl2_RBUS_reg.cp_spatialthlyx4 = SNR_Zoran_filter_set_Y_tbl[select][6];
	#else//Merlin4
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweighty_sel = SNR_Zoran_filter_set_Y_tbl[select][0];// Merlin4 add
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthly = SNR_Zoran_filter_set_Y_tbl[select][1];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthlyx2 = SNR_Zoran_filter_set_Y_tbl[select][2];
	nr_dch1_spatial_thl2_RBUS_reg.cp_spatialthlyx4 = SNR_Zoran_filter_set_Y_tbl[select][3];
	#endif
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

	#if 0//Merlin3
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranfiltersizec = SNR_Zoran_filter_set_C_tbl[select][0];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight2c = SNR_Zoran_filter_set_C_tbl[select][1];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight3c = SNR_Zoran_filter_set_C_tbl[select][2];
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweight4c = SNR_Zoran_filter_set_C_tbl[select][3];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthlc = SNR_Zoran_filter_set_C_tbl[select][4];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthlcx2 = SNR_Zoran_filter_set_C_tbl[select][5];
	nr_dch1_spatial_thl2_RBUS_reg.cp_spatialthlcx4 = SNR_Zoran_filter_set_C_tbl[select][6];
	#else//Merlin4
	nr_dch1_cp_ctrl_RBUS_reg.cp_zoranweightc_sel = SNR_Zoran_filter_set_C_tbl[select][0];// Merlin4 add
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthlc = SNR_Zoran_filter_set_C_tbl[select][1];
	nr_dch1_spatial_thl_RBUS_reg.cp_spatialthlcx2 = SNR_Zoran_filter_set_C_tbl[select][2];
	nr_dch1_spatial_thl2_RBUS_reg.cp_spatialthlcx4 = SNR_Zoran_filter_set_C_tbl[select][3];
	#endif

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, nr_dch1_cp_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl_reg, nr_dch1_spatial_thl_RBUS_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl2_reg, nr_dch1_spatial_thl2_RBUS_reg.regValue);

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
}

/* iEdgeSmooth*/
/* From MacArthur, Elsie 20130206*/

void drvif_color_iEdgeSmooth_en(unsigned char enable)
{
	/*VIPprintf("EDGE_EN =%d\n", enable);*/
	iedge_smooth_edsm_db_ctrl_RBUS edsm_db_ctrl_reg;
	iedge_smooth_dejagging_ctrl0_RBUS edgesmooth_ex_ctrl_reg;

	edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_dejagging_ctrl0_reg);
	if(get_vsc_run_pc_mode() && (get_scaler_connect_source(SLR_MAIN_DISPLAY) == _SRC_HDMI))	//RGB444
		edgesmooth_ex_ctrl_reg.i_dejag_en = 0;
	else
		edgesmooth_ex_ctrl_reg.i_dejag_en=(Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(enable?1:0);

	edsm_db_ctrl_reg.edsm_db_apply = 1;

	IoReg_Write32(IEDGE_SMOOTH_dejagging_ctrl0_reg, edgesmooth_ex_ctrl_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
}
/*===========================================================================*/
/*======================= rock_rau	20130227 mark not use function =======================*/
void drvif_color_Mixer_ctrl_iedge(unsigned char channel, int mixer_weight)
{
	iedge_smooth_mixer_ctrl_RBUS mixer_ctrl_reg;
	mixer_ctrl_reg.regValue = rtd_inl(IEDGE_SMOOTH_MIXer_ctrl_reg);

	/* 0: all use snr Y, 1~7: 3bit blending, 8:all use egsm result, 9:use max(snr, iegsm)*/
	if (mixer_weight <= 8)
		mixer_ctrl_reg.iedge_mixer_weight = mixer_weight;
	else
		mixer_ctrl_reg.iedge_mixer_weight = 0x8;

	rtd_outl(IEDGE_SMOOTH_MIXer_ctrl_reg, mixer_ctrl_reg.regValue);

}
/*===========================================================================*/
/*======================= rock_rau	20130227 mark not use function =======================*/
void drvif_color_DLTI(unsigned char gain)
{
	// Merlin4 no
}
/*===========================================================================*/


void drvif_color_iEdgeSmooth_init()
{
// Merlin3 no old edgesmooth
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	UINT8  source = 255;
	iedge_smooth_edgesmooth_ex_ctrl_RBUS edgesmooth_ex_ctrl_reg;
	iedge_smooth_mixer_ctrl_RBUS mixer_ctrl_reg;
	iedge_smooth_vfilter_0_RBUS vfilter_0_reg;

	source = fwif_vip_source_check(SLR_MAIN_DISPLAY, 1);

	edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg);
	mixer_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MIXer_ctrl_reg);
	vfilter_0_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_0_reg);

	if ((VIP_system_info_structure_table->I_Width) > 1920) {	/* 4k2k source*/
		/*FIXME: Elsie 20140220 what settings should be applied for 4k2k source?*/
		edgesmooth_ex_ctrl_reg.lbuf_linemode = 0;// 2

		mixer_ctrl_reg.iedge_mixer_weight = 8;
		vfilter_0_reg.vfir_weight = 0;
		/*FIXME end*/
	} else if (((source >= VIP_QUALITY_YPbPr_720P) && (source <= VIP_QUALITY_YPbPr_1080P))
	|| ((source >= VIP_QUALITY_HDMI_720P) && (source <= VIP_QUALITY_HDMI_1080P))
	|| ((source >= VIP_QUALITY_DTV_720P) && (source <= VIP_QUALITY_DTV_1080P))
	|| ((source >= VIP_QUALITY_PVR_720P) && (source <= VIP_QUALITY_PVR_1080P))
	) {	/*HD*/
		edgesmooth_ex_ctrl_reg.lbuf_linemode = 0;// 3

		mixer_ctrl_reg.iedge_mixer_weight = 8;
		vfilter_0_reg.vfir_weight = 0;

	} else {	/*RF_CVBS SD*/
		edgesmooth_ex_ctrl_reg.lbuf_linemode = 0;// 3

		mixer_ctrl_reg.iedge_mixer_weight = 8;
		vfilter_0_reg.vfir_weight = 0;
	}

	IoReg_Write32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg, edgesmooth_ex_ctrl_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_MIXer_ctrl_reg, mixer_ctrl_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_0_reg, vfilter_0_reg.regValue);
}

void drvif_color_iEdgeSmooth_chaos_set(unsigned char level)/*0~8*/
{
	// removed
}

void drvif_color_iEdgeSmooth_t2df_set(unsigned char level)/*0~8*/
{
// Merlin3 no
}

void drvif_color_iEdgeSmooth_edge_set(unsigned char level)/*0~8*/
{
// Merlin3 no
}

void drvif_color_iEdgeSmooth_crct_set(unsigned char level)/*0~1*/
{
// Merlin3 no
}

void drvif_color_iEdgeSmooth_extension_set(unsigned char level)/*0~16, 0:disable*/
{
// Merlin3 no
}

/* I-edge smooth 3 line/5 line mode setting*/
/* 0 :line buf  (0:3line, 1:5line(only SD))*/
/* 1:crct_lineMode  2:chaos_lineMode  3:lpf_tapMode*/
#if 0
/*=== SNR curve mapping mode step table === '130524, Lydia*/
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
	di_db_reg_ctl_RBUS  db_reg_ctl_reg;
	down(&VPQ_DI_RTNR_CONTROL_Semaphore);
	im_di_rtnr_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);

	im_di_rtnr_control_reg.cp_rtnr_y_enable 		= ptr->RTNR_y_enable;
	if (drv_memory_get_ip_DMA420_mode() == SLR_DMA_400_mode)
		im_di_rtnr_control_reg.cp_rtnr_c_enable = 0x0;
	else
	im_di_rtnr_control_reg.cp_rtnr_c_enable 		= ptr->RTNR_c_enable;

	im_di_rtnr_control_reg.cp_rtnr_mad_window 	= ptr->RTNR_mad_window;

	// chen170522
//	im_di_rtnr_new_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_CONTROL_reg);
//	di_im_di_rtnr_new_control_RBUS  im_di_rtnr_new_control_reg;
//	im_di_rtnr_new_control_reg.rtnr_new_method_en 	= ptr->RTNR_new_method_en;
//	im_di_rtnr_new_control_reg.rtnr_moredetail_cotinue_en = ptr->RTNR_moredetail_cotinue_en;
	//im_di_rtnr_new_control_reg.rtnr_k_temporal_compare_snr = ptr->RTNR_k_temporal_compare_snr;
	//im_di_rtnr_new_control_reg.rtnr_k_temporal_compare_en = ptr->RTNR_k_temporal_compare_en;
//	IoReg_Write32(DI_IM_DI_RTNR_NEW_CONTROL_reg, im_di_rtnr_new_control_reg.regValue);
// end chen 170522

	IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, im_di_rtnr_control_reg.regValue);


	db_reg_ctl_reg.regValue =rtd_inl(DI_db_reg_ctl_reg);
	db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, db_reg_ctl_reg.regValue);
	up(&VPQ_DI_RTNR_CONTROL_Semaphore);
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

// chen 170522
#if 0
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
	di_im_di_rtnr_new_sta_motion_th_0_RBUS im_di_rtnr_new_sta_motion_th_0_reg;
	di_im_di_rtnr_new_sad_count_th_2_RBUS im_di_rtnr_new_sad_count_th_2_reg;
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
	im_di_rtnr_new_sta_motion_th_0_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_STA_MOTION_TH_0_reg);
	im_di_rtnr_new_sad_count_th_2_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_2_reg);

	im_di_rtnr_new_sobel_th_reg.rtnr_sbl_thm_y = ptr->RTNR_sobel_thm;
	im_di_rtnr_new_sobel_th_reg.rtnr_sbl_ths_y = ptr->RTNR_sobel_ths;
	im_di_rtnr_new_sobel_th_reg.rtnr_sobel_method = ptr->RTNR_sobel_Method;
	im_di_rtnr_new_control_reg.rtnr_sad_method = ptr->RTNR_SAD_Method;
	im_di_rtnr_new_control_reg.rtnr_sad_mix_cal_weight = ptr->RTNR_SAD_Mix_cal_weight;
	//marked by Elsie START
	//im_di_rtnr_new_control_reg.rtnr_statistic_motion_sad_th1 = ptr->RTNR_SAD_th1;
	//im_di_rtnr_new_control_reg.rtnr_statistic_motion_sad_th2 = ptr->RTNR_SAD_th2;
	//im_di_rtnr_new_sobel_th_reg.rtnr_statistic_motion_sad_th3 = ptr->RTNR_SAD_th3;
	//marked by Elsie END
	//modified by Elsie START
	im_di_rtnr_new_sta_motion_th_0_reg.rtnr_statistic_motion_sad_th1 = ptr->RTNR_SAD_th1;
	im_di_rtnr_new_sta_motion_th_0_reg.rtnr_statistic_motion_sad_th2 = ptr->RTNR_SAD_th2;
	im_di_rtnr_new_sta_motion_th_0_reg.rtnr_statistic_motion_sad_th3 = ptr->RTNR_SAD_th3;
	//modified by Elsie END

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
	//marked by Elsie START
	//im_di_rtnr_new_sad_nonedge_th_2_reg.rtnr_statistic_motion_count_th7 = ptr->RTNR_SC_th7;
	//marked by Elsie END
	//modified by Elsie START
	im_di_rtnr_new_sad_count_th_2_reg.rtnr_statistic_motion_count_th7 = ptr->RTNR_SC_th7;
	//modified by Elsie END

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
	IoReg_Write32(DI_IM_DI_RTNR_NEW_STA_MOTION_TH_0_reg, im_di_rtnr_new_sta_motion_th_0_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_2_reg, im_di_rtnr_new_sad_count_th_2_reg.regValue);
=======
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
#endif
// end chen 170522

	// chen 180308 remove
/*
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
*/
// end chen 180308 remove


void drvif_color_DRV_MCNR_TMV(unsigned char block_size)
{
	di_im_new_mcnr_control3_RBUS di_im_new_mcnr_control3;
	di_im_new_mcnr_control3.regValue = IoReg_Read32(DI_IM_NEW_MCNR_CONTROL3_reg);
	di_im_new_mcnr_control3.n_mcnr_tmv_bz_select = block_size;
	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL3_reg,di_im_new_mcnr_control3.regValue);


}
void drvif_color_DRV_MCNR_table(DRV_MCNR_Table *ptr)
{

	di_db_reg_ctl_RBUS  db_reg_ctl_reg;
	di_im_di_rtnr_hmcnr_RBUS										di_im_di_rtnr_hmcnr;
	di_im_new_mcnr_control_RBUS									di_im_new_mcnr_control;
	di_im_new_mcnr_weighting_condition_RBUS			di_im_new_mcnr_weighting_condition;
	di_im_new_mcnr_clamping_value_RBUS					di_im_new_mcnr_clamping_value;
	di_im_new_mcnr_control2_RBUS di_im_new_mcnr_control2;

/* ylchen 150617 */
	di_im_new_mcnr_control3_RBUS								di_im_new_mcnr_control3;
	di_im_new_mcnr_mv_mad_offset_RBUS						di_im_new_mcnr_mv_mad_offset;
/* end ylchen 150617 */
// chen 0709
	 di_im_new_mcnr_pan_condition_RBUS      di_im_new_mcnr_pan_condition;
// end chen 0709

// chen 0724
 	di_rtnr_output_clamp_RBUS          di_rtnr_output_clamp;
 // end chen 0724

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

	if (!ptr) {
		//VIPprintf("\n\ndrvif_color_DRV_MCNR_table,  fatal error, table NULL!!\n");
		return;
	}

	// chen 170522
//	VIPprintf("ptr->MCNR_control.n_mcnr_offset = %d\n", ptr->MCNR_control.n_mcnr_offset);
	//VIPprintf("ptr->MCNR_control.n_mcnr_smp_en = %d\n", ptr->MCNR_control.n_mcnr_smp_en);
  // end chen 170522
	di_im_di_rtnr_hmcnr.regValue												= IoReg_Read32(DI_IM_DI_RTNR_HMCNR_reg);
	di_im_new_mcnr_control.regValue					 						= IoReg_Read32(DI_IM_NEW_MCNR_CONTROL_reg);
	di_im_new_mcnr_weighting_condition.regValue				 	= IoReg_Read32(DI_IM_NEW_MCNR_WEIGHTING_CONDITION_reg);
	di_im_new_mcnr_clamping_value.regValue					 		= IoReg_Read32(DI_IM_NEW_MCNR_CLAMPING_VALUE_reg);
	di_im_new_mcnr_control2.regValue					 						= IoReg_Read32(DI_IM_NEW_MCNR_CONTROL2_reg);

/* ylchen 150617 */
	di_im_new_mcnr_control3.regValue					 					= IoReg_Read32(DI_IM_NEW_MCNR_CONTROL3_reg);
	di_im_new_mcnr_mv_mad_offset.regValue					 			= IoReg_Read32(DI_IM_NEW_MCNR_MV_MAD_Offset_reg);
/* end ylchen 150617 */
// chen 0709
	 di_im_new_mcnr_pan_condition.regValue         = IoReg_Read32(DI_IM_NEW_MCNR_PAN_CONDITION_reg);
// end chen 0709

// chen 0724
 	di_rtnr_output_clamp.regValue            = IoReg_Read32(DI_RTNR_OUTPUT_CLAMP_reg);
 // end chen 0724

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



	di_im_di_rtnr_hmcnr.cp_temporal_hmcnr_y_en							  	= ptr->MCNR_control.cp_temporal_hmcnr_y_en;
	di_im_di_rtnr_hmcnr.cp_temporal_hmcnr_uv_with_y				  		= ptr->MCNR_control.cp_temporal_hmcnr_uv_with_y;
	di_im_new_mcnr_control.n_mcnr_new_en							          = ptr->MCNR_control.n_mcnr_new_en;
	di_im_new_mcnr_control.n_y_zero_mv_k_en								      = ptr->MCNR_control.n_y_zero_mv_k_en;
	di_im_new_mcnr_control.n_c_zero_mv_k_en							        = ptr->MCNR_control.n_c_zero_mv_k_en;
	di_im_new_mcnr_control2.cp_temporal_hmcnr_search_range					= ptr->MCNR_control.cp_temporal_hmcnr_search_range;
	di_im_new_mcnr_control.hori_boundary_th							        = ptr->MCNR_control.hori_boundary_th;
	di_im_di_rtnr_hmcnr.cp_temporal_hmcnr_mad_window						= ptr->MCNR_control.cp_temporal_hmcnr_mad_window;

/* ylchen 150617 */ //fixed me mark for change rbus rord.tsao 20160614
//	di_im_new_mcnr_control2.n_mcnr_v_search_range								= ptr->MCNR_control.n_mcnr_v_search_range;
	di_im_new_mcnr_control2.n_mcnr_y_boundary_th								= ptr->MCNR_control.n_mcnr_y_boundary_th;
	di_im_new_mcnr_control3.n_mcnr_messlevel_th 								= ptr->MCNR_control.n_mcnr_messlevel_th;
	di_im_new_mcnr_control.n_mcnr_old_search_range_en 					= ptr->MCNR_control.n_mcnr_old_search_range_en;
// chen 170522
//	di_im_new_mcnr_control.n_mcnr_offset                        = ptr->MCNR_control.n_mcnr_offset;
//	di_im_new_mcnr_control.n_mcnr_smp_en							 					= ptr->MCNR_control.n_mcnr_smp_en;
// end chen 170522
/* end ylchen 150617 */
// chen 160602
	di_im_new_mcnr_control4.n_mcnr_v_search_range								= ptr->MCNR_control.n_mcnr_v_search_range;
// end chen 160602


// chen 0709
	 di_im_new_mcnr_control.n_h_pan_zeromv_penalty_en      =ptr->MCNR_control.n_h_pan_zeromv_penalty_en;
	 di_im_new_mcnr_pan_condition.h_pan_mv_penalty        =ptr->MCNR_control.h_pan_mv_penalty;
// end chen 0709

// chen 0724
	 di_rtnr_output_clamp.rtnr_output_clamp_en          =ptr->MCNR_control.rtnr_output_clamp_en;
	 di_rtnr_output_clamp.rtnr_output_clamp_luma         =ptr->MCNR_control.rtnr_output_clamp_luma;
	 di_rtnr_output_clamp.rtnr_output_clamp_chroma        =ptr->MCNR_control.rtnr_output_clamp_chroma;
 // end chen 0724

/*
	di_im_new_mcnr_control.n_mv_avg_en				                  = ptr->MCNR_Mv_Avg.n_mv_avg_en;
	di_im_new_mcnr_control.n_mv_avg_mode_select				          = ptr->MCNR_Mv_Avg.n_mv_avg_mode_select;
	di_im_new_mcnr_clamping_value.avg_mv_clamp_value			  		= ptr->MCNR_Mv_Avg.avg_mv_clamp_value;
*/
// chen 190327
	di_im_di_rtnr_hmcnr.hmcnr_mvx_avg_en												= ptr->MCNR_Mv_Avg.hmcnr_mvx_avg_en;
	di_im_di_rtnr_hmcnr.hmcnr_mvy_avg_en				  							= ptr->MCNR_Mv_Avg.hmcnr_mvy_avg_en;
// end chen 190327

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

/* ylchen 150617 */
	di_im_new_mcnr_control.n_mcnr_mv_diff_max_en								= ptr->MCNR_Weighting.n_mcnr_mv_diff_max_en;
/* end ylchen 150617 */

	di_im_new_mcnr_control.n_rtnr_y_k_select            				= ptr->MCNR_Output_K_select.n_rtnr_y_k_select;
	di_im_new_mcnr_control.n_rtnr_c_k_select            				= ptr->MCNR_Output_K_select.n_rtnr_c_k_select;
	di_im_new_mcnr_control.n_mcnr_y_k_select_mc	        				= ptr->MCNR_Output_K_select.n_mcnr_y_k_select_mc;

/* ylchen 150617 */
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
/* end ylchen 150617 */

	// chen 160602
	di_im_new_mcnr_control3.n_mcnr_mad_mv_nolimit_en					= ptr->MCNR_Control2.n_mcnr_mad_mv_nolimit_en;
	di_im_new_mcnr_control3.n_mcnr_only_h_c_en								= DI_IM_DI_RTNR_NEW_CONTROL_get_prtnr_4k2k(IoReg_Read32(DI_IM_DI_RTNR_NEW_CONTROL_reg)); //ptr->MCNR_Control2.n_mcnr_only_h_c_en;
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

	IoReg_Write32(DI_IM_DI_RTNR_HMCNR_reg, 								di_im_di_rtnr_hmcnr.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL_reg, 						di_im_new_mcnr_control.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_WEIGHTING_CONDITION_reg, di_im_new_mcnr_weighting_condition.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_CLAMPING_VALUE_reg, 			di_im_new_mcnr_clamping_value.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL2_reg, 						di_im_new_mcnr_control2.regValue);

/* ylchen 150617 */
	IoReg_Write32(DI_IM_NEW_MCNR_CONTROL3_reg, 						di_im_new_mcnr_control3.regValue);
	IoReg_Write32(DI_IM_NEW_MCNR_MV_MAD_Offset_reg, 			di_im_new_mcnr_mv_mad_offset.regValue);
/* end ylchen 150617 */


// chen 0709
	IoReg_Write32(DI_IM_NEW_MCNR_PAN_CONDITION_reg,    di_im_new_mcnr_pan_condition.regValue);
// end chen 0709


// chen 0724
	IoReg_Write32(DI_RTNR_OUTPUT_CLAMP_reg,        di_rtnr_output_clamp.regValue);
// end chen 0724

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
	iedge_smooth_prelpf_ctrl_RBUS prelpf_ctrl_reg;

	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	prelpf_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_prelpf_ctrl_reg);

	/*SNR*/
	dch1_cp_ctrl_reg.cp_spatialenabley = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(ptr->SNR_spatialenabley);
	dch1_cp_ctrl_reg.cp_spatialenablec =  (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(ptr->SNR_spatialenablec);
	prelpf_ctrl_reg.prelpf_snr_sel = ptr->SNR_prelpf_snr_sel;
	dch1_cp_ctrl_reg.cp_sresultweight = ptr->SNR_sresultweight;
	dch1_cp_ctrl_reg.vc_nr_maxmin_range_sel = ptr->SNR_maxmin_range;

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_prelpf_ctrl_reg, prelpf_ctrl_reg.regValue);

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
}

void drvif_color_DRV_SNR_Spatial_NR_Y(DRV_SNR_Spatial_NR_Y *ptr)
{

	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_dch1_spatial_thl_RBUS dch1_spatial_thl_reg;
	nr_dch1_spatial_thl2_RBUS dch1_spatial_thl2_reg;
	nr_dch1_spatial_weight_RBUS dch1_Spatial_Weight_Reg;
	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	dch1_spatial_thl_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl_reg);
	dch1_spatial_thl2_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl2_reg);
	dch1_Spatial_Weight_Reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Weight_reg);
	/*SNR Blending*/
	dch1_Spatial_Weight_Reg.cp_fixedweight1y = ptr->SNR_fixedweight1y;
	dch1_Spatial_Weight_Reg.cp_fixedweight2y = ptr->SNR_fixedweight2y;
	dch1_Spatial_Weight_Reg.cp_weight1y = ptr->SNR_weight1y;
	dch1_Spatial_Weight_Reg.cp_weight2y = ptr->SNR_weight2y;
	dch1_cp_ctrl_reg.cp_zoranweighty_sel = ptr->SNR_zoranweighty_sel;// Merlin4 add
	dch1_spatial_thl_reg.cp_spatialthly = ptr->SNR_spatialthly;
	dch1_spatial_thl_reg.cp_spatialthlyx2 = ptr->SNR_spatialthlyx2;
	dch1_spatial_thl2_reg.cp_spatialthlyx4 = ptr->SNR_spatialthlyx4;

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl_reg, dch1_spatial_thl_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl2_reg, dch1_spatial_thl2_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Weight_reg, dch1_Spatial_Weight_Reg.regValue);

	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
}

void drvif_color_DRV_SNR_Spatial_NR_C(DRV_SNR_Spatial_NR_C *ptr)
{

	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_dch1_spatial_thl_RBUS dch1_spatial_thl_reg;
	nr_dch1_spatial_thl2_RBUS dch1_spatial_thl2_reg;
	nr_dch1_spatial_weight_RBUS dch1_Spatial_Weight_Reg;
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	
	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	dch1_spatial_thl_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl_reg);
	dch1_spatial_thl2_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl2_reg);
	dch1_Spatial_Weight_Reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Weight_reg);
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	
	/*SNR C*/
	dch1_Spatial_Weight_Reg.cp_lpresultweightc = ptr->SNR_lpresultweightc;
	dch1_cp_ctrl_reg.cp_zoranweightc_sel = ptr->SNR_zoranweightc_sel;// Ml4 add
	dch1_spatial_thl_reg.cp_spatialthlc = ptr->SNR_spatialthlc;
	dch1_spatial_thl_reg.cp_spatialthlcx2 = ptr->SNR_spatialthlcx2;
	dch1_spatial_thl2_reg.cp_spatialthlcx4 = ptr->SNR_spatialthlcx4;
	nr_mosquito_ctrl_RBUS_reg.nr_uv_lpf_gain = ptr->SNR_uv_lpf_gain;// c2 add ml8_add
	
	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl_reg, dch1_spatial_thl_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Thl2_reg, dch1_spatial_thl2_reg.regValue);
	IoReg_Write32(NR_DCH1_Spatial_Weight_reg, dch1_Spatial_Weight_Reg.regValue);
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
}

void drvif_color_DRV_SNR_Modified_LPF(DRV_SNR_Modified_LPF *ptr)
{

	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_new_lpf_nearby_ctrl_RBUS new_lpf_nearby_ctrl_reg;
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_edge_filter_RBUS nr_edge_filter_reg;
	nr_new_lpf_type_thd_RBUS nr_new_lpf_type_thd_reg;
	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	new_lpf_nearby_ctrl_reg.regValue = IoReg_Read32(NR_NEW_LPF_NEARBY_CTRL_reg);
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	nr_edge_filter_reg.regValue = IoReg_Read32(NR_EDGE_FILTER_reg);
	nr_new_lpf_type_thd_reg.regValue = IoReg_Read32(NR_NEW_LPF_TYPE_THD_reg); // Merlin6 add

	/*modified lpf*/
	dch1_cp_ctrl_reg.vc_snr_lpf_sel = ptr->SNR_LPF_sel; // 0:3, 1:5, 2:mod3, 3:mod5, 4:11, 5:mod11 (Merlin, jyyang@20150615)(Merlin2, jyyang@20160617)
	new_lpf_nearby_ctrl_reg.modified_lpf_edge_gain	= ptr->SNR_modified_lpf_edge_gain; /*Mac3, leonard wu@20140722*/
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_upbnd	= ptr->SNR_modified_lpf_thd_upbnd;
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_lowbnd = ptr->SNR_modified_lpf_thd_lowbnd;
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_step 	= ptr->SNR_modified_lpf_thd_step;
	new_lpf_nearby_ctrl_reg.modified_lpf_edge_en	= ptr->SNR_modified_lpf_edge_en; // Merlin4 add
	nr_new_lpf_type_thd_reg.modified_lpf_type_mode = ptr->SNR_modified_lpf_type_mode; // Merlin6 add
	nr_new_lpf_type_thd_reg.modified_lpf_type_th = ptr->SNR_modified_lpf_type_th; // Merlin6 add

	//Merlin2, jyyang@20160617
	nr_mosquito_ctrl_RBUS_reg.mnr_lpf_sel = ptr->MNR_LPF_sel;
	nr_edge_filter_reg.vc_edge_lpf_sel = ptr->Edge_LPF_sel;

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(NR_NEW_LPF_NEARBY_CTRL_reg, new_lpf_nearby_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_EDGE_FILTER_reg, nr_edge_filter_reg.regValue);
	IoReg_Write32(NR_NEW_LPF_TYPE_THD_reg, nr_new_lpf_type_thd_reg.regValue);// Merlin6 add
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
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
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
}

void drvif_color_DRV_SNR_NR_Mask(DRV_SNR_NR_Mask *ptr)/* mk2 */
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
	dch1_MASK1_CTRL2_Reg.cp_mask1_maplimit = (ptr->SNR_Mask1_Limit);
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
	dch1_MASK2_CTRL2_Reg.cp_mask2_maplimit = (ptr->SNR_Mask2_Limit);
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
	dch1_MASK3_CTRL2_Reg.cp_mask3_maplimit = (ptr->SNR_Mask3_Limit);
	dch1_MASK3_CTRL1_Reg.cp_mask3_yref = ptr->SNR_Mask3_Y_Ref;
	dch1_MASK3_CTRL1_Reg.cp_mask3_cbref = ptr->SNR_Mask3_Cb_Ref;
	dch1_MASK3_CTRL1_Reg.cp_mask3_crref = ptr->SNR_Mask3_Cr_Ref;
	rtd_outl(NR_DCH1_MASK1_CTRL1_reg, dch1_MASK1_CTRL1_Reg.regValue);
	rtd_outl(NR_DCH1_MASK1_CTRL2_reg, dch1_MASK1_CTRL2_Reg.regValue);
	rtd_outl(NR_DCH1_MASK2_CTRL1_reg, dch1_MASK2_CTRL1_Reg.regValue);
	rtd_outl(NR_DCH1_MASK2_CTRL2_reg, dch1_MASK2_CTRL2_Reg.regValue);
	rtd_outl(NR_DCH1_MASK3_CTRL1_reg, dch1_MASK3_CTRL1_Reg.regValue);
	rtd_outl(NR_DCH1_MASK3_CTRL2_reg, dch1_MASK3_CTRL2_Reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
}

void drvif_color_DRV_SNR_NR_Mask_W1W2(DRV_SNR_NR_Mask_W1W2 *ptr)  /*Mac3, leonard wu@20140722*//* mk2 */
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
	iedge_smooth_vfilter_4_RBUS vfilter_4_reg;
	iedge_smooth_vfilter_5_RBUS vfilter_5_reg;
	iedge_smooth_vfilter_6_RBUS vfilter_6_reg;
	iedge_smooth_vfilter_11_RBUS vfilter_11_reg;
	iedge_smooth_vfilter_12_RBUS vfilter_12_reg;
	iedge_smooth_vfilter_di_info_RBUS vfilter_di_info_reg;

	vfilter_0_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_0_reg);
	vfilter_1_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_1_reg);
	vfilter_2_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_2_reg);
	vfilter_3_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_3_reg);
	vfilter_4_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_4_reg);
	vfilter_5_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_5_reg);
	vfilter_6_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_6_reg);
	vfilter_11_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_11_reg);
	vfilter_12_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_12_reg);
	vfilter_di_info_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_DI_INFO_reg);

	vfilter_0_reg.avlpf_en = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(ptr->SNR_Veritcal_avlpf_En);
	vfilter_2_reg.avlpf_c_en = ptr->SNR_Veritcal_C_En;
	vfilter_0_reg.avlpf_correct_en = ptr->SNR_Veritcal_correct_En;
	vfilter_0_reg.avlpf_bw_pattern_en = ptr->SNR_Veritcal_BW_En;
	vfilter_0_reg.vfir_weight = ptr->SNR_Veritcal_Y_Weight;
	vfilter_2_reg.vfir_c_weight = ptr->SNR_Veritcal_C_Weight;
	vfilter_0_reg.avlpf_mindiff_th_start = ptr->SNR_Veritcal_Th_start;
	vfilter_0_reg.avlpf_mindiff_curve_rate_start = ptr->SNR_Veritcal_Rate_start;
	vfilter_0_reg.avlpf_mindiff_th_finish = ptr->SNR_Veritcal_Th_finish;
	vfilter_0_reg.avlpf_mindiff_curve_rate_finish = ptr->SNR_Veritcal_Rate_finish;
	vfilter_0_reg.avlpf_tap_sel = ptr->SNR_Veritcal_avlpf_Tap_Sel;
	vfilter_1_reg.avlpf_max_weight = ptr->SNR_Veritcal_Max_MinDiff_Weight;
	vfilter_1_reg.avlpf_min_weight = ptr->SNR_Veritcal_Min_MinDiff_Weight;
	vfilter_1_reg.avlpf_maxmindiff_th = ptr->SNR_Veritcal_MaxMinDiff_Th;
	vfilter_1_reg.avlpf_maxmindiff_curve_rate = ptr->SNR_Veritcal_MaxMinDiff_Rate;
	vfilter_2_reg.avlpf_bw_th = ptr->SNR_Veritcal_BWPattern_Th;
	vfilter_2_reg.avlpf_bw_margin = ptr->SNR_Veritcal_BWPattern_Margin;
	vfilter_2_reg.avlpf_bw_curve_rate = ptr->SNR_Veritcal_BWPattern_Rate;

	if(get_vsc_run_pc_mode() && (get_scaler_connect_source(SLR_MAIN_DISPLAY) == _SRC_HDMI))
		vfilter_4_reg.esmex_iedge_vfir_enable = 0;
	else
		vfilter_4_reg.esmex_iedge_vfir_enable = ptr->SNR_Vertical_fir_En;
	vfilter_4_reg.egsm_avlpf_clamp_en = ptr->SNR_Vertical_Clamp_En;
	vfilter_4_reg.egsm_avlpf_clampth = ptr->SNR_Vertical_Clamp_Th;
	vfilter_4_reg.egsm_avlpf_mindiff_th2_start = ptr->SNR_Vertical_Th2_start;

	// Merlin4 add
	vfilter_6_reg.egsm_avlpf_clampth_tth = ptr->SNR_Vertical_Clamp_Th_TTH;
	vfilter_0_reg.egsm_avlpf_ignorehlh = ptr->SNR_Vertical_IgnoreHLH;

	if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE))
        	vfilter_di_info_reg.avlpf_di_teeth_vlpf_en =  ptr->SNR_Vertical_di_teeth_En;
        else
        	vfilter_di_info_reg.avlpf_di_teeth_vlpf_en =  0;

	vfilter_di_info_reg.avlpf_weight_teeth_th =  ptr->SNR_Vertical_di_teeth_Weight_Th;
	vfilter_di_info_reg.egsm_avlpf_clampth_tth_di =  ptr->SNR_Vertical_di_teeth_Clamp_Th;

	// ml7
	vfilter_11_reg.egsm_avlpf_win_en = ptr->SNR_Vertical_win_en;
	vfilter_11_reg.egsm_avlpf_win_in_lpf_sel = ptr->SNR_Vertical_win_lpf_in_sel;
	vfilter_11_reg.egsm_avlpf_win_out_lpf_sel = ptr->SNR_Vertical_win_lpf_out_sel;
	vfilter_11_reg.egsm_avlpf_clampth_win = ptr->SNR_Vertical_win_clampTh;
	vfilter_11_reg.esmex_iedge_vfir_weight_win = ptr->SNR_Vertical_win_weight_y;
	vfilter_11_reg.esmex_iedge_vfir_c_weight_win = ptr->SNR_Vertical_win_weight_c;	
	vfilter_12_reg.egsm_avlpf_win_up = ptr->SNR_Vertical_win_up;
	vfilter_12_reg.egsm_avlpf_win_dn = ptr->SNR_Vertical_win_dn;
	vfilter_12_reg.egsm_avlpf_win_blend_rate = ptr->SNR_Vertical_win_blend_rate;

	IoReg_Write32(IEDGE_SMOOTH_VFILTER_0_reg, vfilter_0_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_1_reg, vfilter_1_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_2_reg, vfilter_2_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_3_reg, vfilter_3_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_4_reg, vfilter_4_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_5_reg, vfilter_5_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_6_reg, vfilter_6_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_11_reg, vfilter_11_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_12_reg, vfilter_12_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_DI_INFO_reg, vfilter_di_info_reg.regValue);
	IoReg_SetBits(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_apply_mask); /* DB apply */ // mk2
}

void drvif_color_DRV_SNR_Mosquito_NR(DRV_SNR_Mosquito_NR *ptr)
{
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_RBUS nr_mosquito_nr_level_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_1_RBUS nr_mosquito_nr_level_ctrl_1_RBUS_reg;
    nr_mosquito_nr_keep_edge_ctrl_RBUS 	nr_mosquito_nr_keep_edge_ctrl_reg;
    nr_mosquito_nr_decay_ctrl_1_RBUS   	nr_mosquito_nr_decay_ctrl_1_reg;
	nr_mosquito_nr_edge_thd_ctrl_RBUS nr_mosquito_nr_edge_thd_ctrl_reg;//h5x add
	nr_mosquito_nr_edge_thd_ctrl_1_RBUS nr_mosquito_nr_edge_thd_ctrl_1_reg;//h5x add
	nr_mosquito_nr_oneside_ctrl_RBUS nr_mosquito_nr_oneside_ctrl_reg;//merlin5 add
	nr_mosquito_nr_oneside_ctrl_2_RBUS nr_mosquito_nr_oneside_ctrl_2_reg;//merlin5 add

	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	nr_mosquito_nr_level_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_reg);
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg);
	nr_mosquito_nr_keep_edge_ctrl_reg.regValue 		= IoReg_Read32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg);
	nr_mosquito_nr_decay_ctrl_1_reg.regValue		= IoReg_Read32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg);
	nr_mosquito_nr_edge_thd_ctrl_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_EDGE_THD_CTRL_reg);//h5x add
	nr_mosquito_nr_edge_thd_ctrl_1_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_EDGE_THD_CTRL_1_reg);//h5x add
	nr_mosquito_nr_oneside_ctrl_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_ONESIDE_CTRL_reg);//h5x add
	nr_mosquito_nr_oneside_ctrl_2_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_ONESIDE_CTRL_2_reg);//h5x add
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
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.mnr_xrange_ext = ptr->MNR_xrange_ext;
	/*Mosquito_NR_Edge Condition*/
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd1					= ptr->SNR_mnr_edgethd1;//h5x add
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd2					= ptr->SNR_mnr_edgethd2;
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd3					= ptr->SNR_mnr_edgethd3;
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd4					= ptr->SNR_mnr_edgethd4;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd5					= ptr->SNR_mnr_edgethd5;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd6					= ptr->SNR_mnr_edgethd6;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd7					= ptr->SNR_mnr_edgethd7;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd8					= ptr->SNR_mnr_edgethd8;
	nr_mosquito_ctrl_RBUS_reg.mnr_edge78_smooth_en = ptr->MNR_edge78_en;//merlin6 add
	nr_mosquito_ctrl_RBUS_reg.mnr_line_protect_en = ptr->MNR_line_protect_en;//merlin6 add
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
    //nr_mosquito_nr_level_ctrl_RBUS_reg.mnr_vardiff_refine_gain 		= ptr->SNR_vardiff_refine_gain;//merlin5 remove
	nr_mosquito_nr_oneside_ctrl_2_reg.mnr_vardiff_refine_gain_negative = ptr->SNR_vardiff_refine_gain_negative;//merlin5 add
	nr_mosquito_nr_oneside_ctrl_2_reg.mnr_vardiff_refine_gain_positive = ptr->SNR_vardiff_refine_gain_positive;//merlin5 add
	nr_mosquito_nr_oneside_ctrl_2_reg.mnr_var_diff_th = ptr->SNR_vardiff_th;//merlin5 add
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_shift			= ptr->SNR_hdiff_frac_shift;
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.vdiff_frac_shift			= ptr->SNR_vdiff_frac_shift;
    nr_mosquito_nr_level_ctrl_RBUS_reg.mnr_nlevel_maxfrac_mode 		= ptr->SNR_mnr_nlevel_maxfrac_mode; // Merlin4 add to driver

	// Merlin4 add
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.nlevel_diff_factor_en		= ptr->SNR_nlevel_diff_factor_en;
    //nr_mosquito_nr_level_ctrl_1_RBUS_reg.mnr_all_var2d_en			= ptr->SNR_mnr_all_var2d_en;//merlin5 remove
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.blendsnr_byedge_en			= ptr->SNR_blendsnr_byedge_en;
	// h5x add
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.mnr_shp_en					= ptr->SNR_mnr_shp_en;
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.mnr_var2d_byedge			= ptr->SNR_mnr_var2d_byedge;

	/*keep edge*/
	nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd				= ptr->SNR_keep_edge_threshold;
    nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd_step			= ptr->SNR_keep_edge_threshold_step;
    nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedge_shift			= ptr->SNR_keep_edge_shift;

	/*Mosquito_NR_Final MNR Level Adjust*/
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_offset = ptr->SNR_mosquitonr_nlevel_positive_offset;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_shift = ptr->SNR_mosquitonr_nlevel_positive_shift;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_offset = ptr->SNR_mosquitonr_nlevel_negative_offset;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_shift = ptr->SNR_mosquitonr_nlevel_negative_shift;
	nr_mosquito_nr_oneside_ctrl_reg.mnr_nlevel_mid_offset = ptr->SNR_mosquitonr_nlevel_mid_offset;//merlin5 add
	nr_mosquito_nr_oneside_ctrl_reg.mnr_nlevel_mid_shift = ptr->SNR_mosquitonr_nlevel_mid_shift;//merlin5 add

	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_reg, nr_mosquito_nr_level_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg, nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg, nr_mosquito_nr_keep_edge_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg, nr_mosquito_nr_decay_ctrl_1_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_EDGE_THD_CTRL_reg, nr_mosquito_nr_edge_thd_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_EDGE_THD_CTRL_1_reg, nr_mosquito_nr_edge_thd_ctrl_1_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_ONESIDE_CTRL_reg, nr_mosquito_nr_oneside_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_ONESIDE_CTRL_2_reg, nr_mosquito_nr_oneside_ctrl_2_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
}


// merlin5 add
void drvif_color_DRV_SNR_Mosquito_NR_Oneside_Edge(DRV_SNR_MNR_ONESIDE_EDGE *ptr)
{
	nr_mosquito_nr_oneside_ctrl_RBUS nr_mosquito_nr_oneside_ctrl_reg;
	nr_mosquito_nr_oneside_ctrl_1_RBUS nr_mosquito_nr_oneside_ctrl_1_reg;
	nr_mosquito_nr_oneside_ctrl_2_RBUS nr_mosquito_nr_oneside_ctrl_2_reg;
	nr_mosquito_nr_oneside_ctrl_3_RBUS nr_mosquito_nr_oneside_ctrl_3_reg;

	nr_mosquito_nr_oneside_ctrl_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_ONESIDE_CTRL_reg);
	nr_mosquito_nr_oneside_ctrl_1_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_ONESIDE_CTRL_1_reg);
	nr_mosquito_nr_oneside_ctrl_2_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_ONESIDE_CTRL_2_reg);
	nr_mosquito_nr_oneside_ctrl_3_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_ONESIDE_CTRL_3_reg);

	nr_mosquito_nr_oneside_ctrl_reg.mnr_oneside_edge_en = ptr->MNR_oneside_en;
	nr_mosquito_nr_oneside_ctrl_reg.mnr_oneside_edge_vertical_en = ptr->MNR_oneside_vertical_en;
	nr_mosquito_nr_oneside_ctrl_1_reg.mnr_oneside_edge_maxedgestep = ptr->MNR_oneside_maxEdgeStep;
	nr_mosquito_nr_oneside_ctrl_1_reg.mnr_oneside_edge_edgelevelgain = ptr->MNR_oneside_edgeLevelGain;
	nr_mosquito_nr_oneside_ctrl_1_reg.mnr_oneside_edge_flatvarianceshift = ptr->MNR_oneside_flatVarShift;
	nr_mosquito_nr_oneside_ctrl_1_reg.mnr_oneside_edge_flatvarianceth = ptr->MNR_oneside_flatVarTh;
	nr_mosquito_nr_oneside_ctrl_2_reg.mnr_oneside_edge_nlevel_upbnd_en = ptr->MNR_oneside_upbnd_en;
	nr_mosquito_nr_oneside_ctrl_2_reg.mnr_oneside_edge_nlevel_upbnd_startth = ptr->MNR_oneside_upbnd_startTh;
	nr_mosquito_nr_oneside_ctrl_3_reg.mnr_oneside_edge_nlevel_upbnd_gain = ptr->MNR_oneside_upbnd_gain;
	nr_mosquito_nr_oneside_ctrl_3_reg.mnr_oneside_edge_nlevel_upbnd_offset = ptr->MNR_oneside_upbnd_offset;
	nr_mosquito_nr_oneside_ctrl_3_reg.mnr_oneside_edge_nlevel_lobnd_en = ptr->MNR_oneside_lobnd_en;
	nr_mosquito_nr_oneside_ctrl_3_reg.mnr_oneside_edge_nlevel_lobnd_startth = ptr->MNR_oneside_lobnd_startTh;
	nr_mosquito_nr_oneside_ctrl_3_reg.mnr_oneside_edge_nlevel_lobnd_gain = ptr->MNR_oneside_lobnd_gain;
	nr_mosquito_nr_oneside_ctrl_3_reg.mnr_oneside_edge_nlevel_lobnd_offset = ptr->MNR_oneside_lobnd_offset;
	nr_mosquito_nr_oneside_ctrl_reg.mnr_oneside_edge_nlevel_positive_offset = ptr->MNR_oneside_nlevel_positive_offset;
	nr_mosquito_nr_oneside_ctrl_reg.mnr_oneside_edge_nlevel_positive_shift = ptr->MNR_oneside_nlevel_positive_shift;
	nr_mosquito_nr_oneside_ctrl_1_reg.mnr_oneside_edge_nlevel_negative_offset = ptr->MNR_oneside_nlevel_negative_offset;
	nr_mosquito_nr_oneside_ctrl_1_reg.mnr_oneside_edge_nlevel_negative_shift = ptr->MNR_oneside_nlevel_negative_shift;
	nr_mosquito_nr_oneside_ctrl_reg.mnr_oneside_edge_strength_gain = ptr->MNR_oneside_edge_strength_gain;
	nr_mosquito_nr_oneside_ctrl_reg.mnr_oneside_edge_keepedge_shift = ptr->MNR_oneside_keep_edge_shift;

	IoReg_Write32(NR_MOSQUITO_NR_ONESIDE_CTRL_reg, nr_mosquito_nr_oneside_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_ONESIDE_CTRL_1_reg, nr_mosquito_nr_oneside_ctrl_1_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_ONESIDE_CTRL_2_reg, nr_mosquito_nr_oneside_ctrl_2_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_ONESIDE_CTRL_3_reg, nr_mosquito_nr_oneside_ctrl_3_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
}

void drvif_color_DRV_SNR_PQ_Mask(DRV_SNR_PQ_Mask *ptr)//mk2, ml7
{
	nr_snr_pq_mask_ctrl_RBUS snr_pq_mask_ctrl_reg;
	nr_snr_curve_mapping_weight3_1_RBUS snr_curve_mapping_weight3_1_reg;
	nr_snr_curve_mapping_weight3_2_RBUS snr_curve_mapping_weight3_2_reg;

	snr_pq_mask_ctrl_reg.regValue = IoReg_Read32(NR_SNR_PQ_MASK_ctrl_reg);
	snr_curve_mapping_weight3_1_reg.regValue = IoReg_Read32(NR_snr_curve_mapping_weight3_1_reg);
	snr_curve_mapping_weight3_2_reg.regValue = IoReg_Read32(NR_snr_curve_mapping_weight3_2_reg);

	snr_pq_mask_ctrl_reg.nr_semantic_en = ptr->SNR_semantic_en;
	snr_pq_mask_ctrl_reg.nr_semantic_mode = ptr->SNR_semantic_mode;
	snr_pq_mask_ctrl_reg.nr_fixedweight3y = ptr->SNR_fixedweight3y;
	snr_pq_mask_ctrl_reg.nr_weight3y = ptr->SNR_weight13y;
	snr_pq_mask_ctrl_reg.nr_curvemapping_w3_0 = ptr->SNR_curvemapping_w3_0;
	snr_curve_mapping_weight3_1_reg.nr_curvemapping_w3_1 = ptr->SNR_curvemapping_w3_1;
	snr_curve_mapping_weight3_1_reg.nr_curvemapping_w3_2 = ptr->SNR_curvemapping_w3_2;
	snr_curve_mapping_weight3_1_reg.nr_curvemapping_w3_3 = ptr->SNR_curvemapping_w3_3;
	snr_curve_mapping_weight3_1_reg.nr_curvemapping_w3_4 = ptr->SNR_curvemapping_w3_4;
	snr_curve_mapping_weight3_2_reg.nr_curvemapping_w3_5 = ptr->SNR_curvemapping_w3_5;
	snr_curve_mapping_weight3_2_reg.nr_curvemapping_w3_6 = ptr->SNR_curvemapping_w3_6;
	snr_curve_mapping_weight3_2_reg.nr_curvemapping_w3_7 = ptr->SNR_curvemapping_w3_7;
	snr_curve_mapping_weight3_2_reg.nr_curvemapping_w3_8 = ptr->SNR_curvemapping_w3_8;

	IoReg_Write32(NR_SNR_PQ_MASK_ctrl_reg, snr_pq_mask_ctrl_reg.regValue);
	IoReg_Write32(NR_snr_curve_mapping_weight3_1_reg, snr_curve_mapping_weight3_1_reg.regValue);
	IoReg_Write32(NR_snr_curve_mapping_weight3_2_reg, snr_curve_mapping_weight3_2_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */

}

void drvif_color_DRV_SNR_PQ_Mask_Enable(unsigned char bEnable)
{
	nr_snr_pq_mask_ctrl_RBUS snr_pq_mask_ctrl_reg;

	snr_pq_mask_ctrl_reg.regValue = IoReg_Read32(NR_SNR_PQ_MASK_ctrl_reg);

	snr_pq_mask_ctrl_reg.nr_semantic_en = (bEnable == 0)? 0 : 1;

	IoReg_Write32(NR_SNR_PQ_MASK_ctrl_reg, snr_pq_mask_ctrl_reg.regValue);
	
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask);	// double buff apply
}

void drvif_color_DRV_SNR_Mosquito_NR_En(unsigned char En_flag)	/* Mosquito NR control is in general control*/
{
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);

	//nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en = En_flag;	/* the spec is MosquitoNR Enable is controlled by MPEG NR UI */
	if(get_vsc_run_pc_mode() && (get_scaler_connect_source(SLR_MAIN_DISPLAY) == _SRC_HDMI)){
		nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en = 0;	//RGB 444 disable mosquito NR
	}else{
		nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(En_flag);	/* always on for hw issue, elieli*/
	}
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask);	// double buff apply

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

void drvif_color_DRV_SNR_Sub_DB_En( bool DB_En ) // ml8_add
{
	nr_dch2_hfilter_ctrl_RBUS nr_dch2_hfilter_ctrl_reg;
    
	nr_dch2_hfilter_ctrl_reg.regValue = IoReg_Read32(NR_DCH2_hfilter_ctrl_reg);
	nr_dch2_hfilter_ctrl_reg.snr_sub_db_en = DB_En;
	IoReg_Write32(NR_DCH2_hfilter_ctrl_reg, nr_dch2_hfilter_ctrl_reg.regValue);

}

void drvif_color_DRV_SNR_Sub_DB_Apply(void) // ml8_add
{
	nr_dch2_hfilter_ctrl_RBUS nr_dch2_hfilter_ctrl_reg;
    
	nr_dch2_hfilter_ctrl_reg.regValue = IoReg_Read32(NR_DCH2_hfilter_ctrl_reg);
	nr_dch2_hfilter_ctrl_reg.snr_sub_db_apply= 1;
	IoReg_Write32(NR_DCH2_hfilter_ctrl_reg, nr_dch2_hfilter_ctrl_reg.regValue);
}

void drvif_color_DRV_SNR_Sub_Hfilter(void) // ml8_add
{
	DRV_SNR_Sub_Table *ptr = NULL;
	nr_dch2_hfilter_RBUS nr_dch2_hfilter_reg;

	nr_dch2_hfilter_reg.regValue = IoReg_Read32(NR_DCH2_hfilter_reg);

	drvif_color_DRV_SNR_Sub_DB_En(1);
	
	ptr = &sub_snr_tbl;

	nr_dch2_hfilter_reg.cp_sub_hfilterenabley = ptr->SNR_sub_hfilterenabley;
	nr_dch2_hfilter_reg.cp_sub_hfilter_coef0_y = ptr->SNR_sub_hfilter_coef0_y;
	nr_dch2_hfilter_reg.cp_sub_hfilter_coef1_y = ptr->SNR_sub_hfilter_coef1_y;
	nr_dch2_hfilter_reg.cp_sub_hfilter_coef2_y = ptr->SNR_sub_hfilter_coef2_y;
	nr_dch2_hfilter_reg.cp_sub_hfilterenablec = ptr->SNR_sub_hfilterenablec;
	nr_dch2_hfilter_reg.cp_sub_hfilter_coef0_c = ptr->SNR_sub_hfilter_coef0_c;
	nr_dch2_hfilter_reg.cp_sub_hfilter_coef1_c = ptr->SNR_sub_hfilter_coef1_c;
	nr_dch2_hfilter_reg.cp_sub_hfilter_coef2_c = ptr->SNR_sub_hfilter_coef2_c;

	IoReg_Write32(NR_DCH2_hfilter_reg, nr_dch2_hfilter_reg.regValue);

	drvif_color_DRV_SNR_Sub_DB_Apply();
	
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

	nr_dch1_mpegnr1_reg.cp_mpegenable_y_y = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(ptr->S_MPEGNR_16_Function_Enable.enable_y);
	nr_dch1_mpegnr1_reg.cp_mpegenable_y_c = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(ptr->S_MPEGNR_16_Function_Enable.enable_c);

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

void drvif_IESM_format_convert(unsigned short display)// Merlin4 add
{
	unsigned char twopixel_mode = 0, hdmi_input = 0, color_format = 0;
	iedge_smooth_format_convert_RBUS edsm_format_convert_reg;

	edsm_format_convert_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_FORMAT_CONVERT_reg);

	if (display == SLR_MAIN_DISPLAY) {
		get_vtop_input_info(&twopixel_mode, &hdmi_input, &color_format);
		if (twopixel_mode == 1)
		{
			edsm_format_convert_reg.main_444_to_422_en = 0;
			edsm_format_convert_reg.main_422_to_444_en = 0;
		}
		else if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP) == TRUE)// merlin4: after di path is 422
		{
			edsm_format_convert_reg.main_444_to_422_en = 0;
			edsm_format_convert_reg.main_422_to_444_en = 1;
		}
		else if(get_vsc_run_pc_mode() && (get_scaler_connect_source(SLR_MAIN_DISPLAY) == _SRC_HDMI))
		{
			edsm_format_convert_reg.main_444_to_422_en = 0;
			edsm_format_convert_reg.main_422_to_444_en = 0;
		}
		else // merlin4: pass di path is 444 // bring up setting cbypass=0
		{
			edsm_format_convert_reg.main_444_to_422_en = 1;
			edsm_format_convert_reg.main_422_to_444_en = 1;
		}
	}
#ifdef CONFIG_I2RND_ENABLE
//TBD
#else
#ifdef CONFIG_DUAL_CHANNEL
	else {
		#if 1
		if((Get_sub_from_DI_mux_path()) && (Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE) == FALSE))
    		{
				edsm_format_convert_reg.sub_422_to_444_en = 0;
    		} else if(Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_THRIP) == TRUE)// merlin4: after di path is 422
	        {
	                edsm_format_convert_reg.sub_422_to_444_en = 1;
	        }
			else // merlin4: pass di path is 444
	        {
				    edsm_format_convert_reg.sub_422_to_444_en = 0;
	        }
		#endif
	}
#endif
#endif

	IoReg_Write32(IEDGE_SMOOTH_FORMAT_CONVERT_reg, edsm_format_convert_reg.regValue);

	rtd_pr_vpq_info("iesm_format:main(imode=%d, rtnr=%d, %d->%d) sub(imode=%d, rtnr=%d, %d),display=%d,2Pixel=%d,hdmi_in=%d,colorFMT=%d,\n ",
		Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP),
		Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_RTNR),
		edsm_format_convert_reg.main_444_to_422_en, edsm_format_convert_reg.main_422_to_444_en,
		Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_THRIP),
		Scaler_DispGetStatus(SLR_SUB_DISPLAY, SLR_DISP_RTNR),
		edsm_format_convert_reg.sub_422_to_444_en,
		display, twopixel_mode, hdmi_input, color_format);

	IoReg_SetBits(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, IEDGE_SMOOTH_EDSM_DB_CTRL_edsm_db_apply_mask); /* DB apply */
}

static unsigned char iedgeSmooth_line_mode = 0;
void drvif_IESM_frontporch_set(unsigned char game_mode_flag)
{
	UINT8 front_porch_num_17line = 9;
	int front_porch_num, linemode_limit, I_width, I_height;
	int fmt_422to444;
	_RPC_system_setting_info *rpc_vip_sysInfo = NULL;
	unsigned char isGameMode = 255;

	iedge_smooth_format_convert_RBUS edsm_format_convert_reg;
        iedge_smooth_edsm_db_ctrl_RBUS edsm_db_ctrl_reg;
	iedge_smooth_edgesmooth_ex_ctrl_RBUS edgesmooth_ex_ctrl_reg;

	front_porch_num = Scaler_DispGetInputInfo(SLR_INPUT_V_LEN)- Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)-
	Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE);

	rpc_vip_sysInfo = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (rpc_vip_sysInfo != NULL) {
		isGameMode = game_mode_flag;//rpc_vip_sysInfo->Low_Delay_Status[PQ_Low_Delay_Mode];
		rpc_vip_sysInfo->Low_Delay_Status[PQ_Low_Delay_Mode] = game_mode_flag;
	}

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


#if 1 // Merlin4 add
	if (isGameMode == 1)
		linemode_limit = 1;
	else if(front_porch_num >= front_porch_num_17line && I_width <= 2048 && I_height>8)
		linemode_limit = 3; // 17 line mode
	else if(front_porch_num >= 6 && (fmt_422to444==1 || I_width <= 2048) && I_height>5)
		linemode_limit = 2; // 11 line mode
	else if(front_porch_num >= 3 && I_height>2)
		linemode_limit = 1; // 5 line mode
	else
		linemode_limit = 0; // 3 line mode
#endif

	edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	edsm_db_ctrl_reg.edsm_db_en = 1;
	edsm_db_ctrl_reg.edsm_db_rd_sel = 1;
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);


	edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg);

	edgesmooth_ex_ctrl_reg.lbuf_linemode = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(linemode_limit);
	iedgeSmooth_line_mode = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(linemode_limit);
	IoReg_Write32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg,edgesmooth_ex_ctrl_reg.regValue);
	edsm_db_ctrl_reg.edsm_db_apply = 1;
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);

	rtd_pr_vpq_info("front_porch_num=vlen-actlen-actsta: %d=%d-%d-%d, game_mode=%d,size=%d-%d,flag=%d,lineMode=%d,fmt_422to444=%d,\n",
	front_porch_num, Scaler_DispGetInputInfo(SLR_INPUT_V_LEN), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE),
	isGameMode, I_width, I_height, game_mode_flag, linemode_limit, fmt_422to444);

	//NRPprintf_Info("linemode/front_porch,h/w/fmt: 17/>=9/<=2048, 11/>=6/422to444 or <=2048, 5/>=3/x, 3/else/x\n");
	//NRPprintf_Info("front_porch_num=vlen-actlen-actsta: %d=%d-%d-%d\n", Scaler_DispGetInputInfo(SLR_INPUT_V_LEN)- Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE)-Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE),
	//Scaler_DispGetInputInfo(SLR_INPUT_V_LEN), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE), Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA_PRE));
	//NRPprintf_Info("limit_linemode: %d, front_porch: %d, in_w: %d, in_h:%d, 422to444:%d\n", linemode_limit, front_porch_num, I_width, I_height, fmt_422to444);

}


void drvif_color_DRV_IESM_Basic(DRV_IESM_Basic *ptr)
{


	iedge_smooth_edsm_db_ctrl_RBUS edsm_db_ctrl_reg;
	edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	edsm_db_ctrl_reg.edsm_db_en = 1;
	edsm_db_ctrl_reg.edsm_db_rd_sel = 1;
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
	{
	//iedge_smooth_edgesmooth_ex_ctrl_RBUS edgesmooth_ex_ctrl_reg;
	iedge_smooth_mixer_ctrl_RBUS mixer_ctrl_reg;
	//edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg);
	mixer_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_MIXer_ctrl_reg);
	drvif_color_iEdgeSmooth_en(ptr->IESM_En);

	//drvif_IESM_format_convert();//move to fw_scalerip_set_IEGSM()
	//edgesmooth_ex_ctrl_reg.lbuf_linemode = drvif_IESM_frontporch_check();

	mixer_ctrl_reg.iedge_mixer_weight = ptr->IESM_Mixer_Weight;

	//IoReg_Write32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg, edgesmooth_ex_ctrl_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_MIXer_ctrl_reg, mixer_ctrl_reg.regValue);
	}

	//edsm_db_ctrl_reg.edsm_db_apply = 1;
	//IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
}

void drvif_color_DRV_DLTI_2D(DRV_DLTI_2D *ptr)
{
	// Merlin4 removed
}
//extern int scalerVIP_color_noise_dejag_detect__z_dummy[9];
void drvif_color_DRV_IESM_Additional_setting(DRV_IESM_Additional_setting *ptr)
{
// Merlin3 change
	iedge_smooth_dejagging_ctrl0_RBUS iedge_smooth_dejagging_ctrl0;
	iedge_smooth_dejagging_motion_support_RBUS iedge_smooth_dejagging_motion_support;
	iedge_smooth_dejagging_smallword0_RBUS iedge_smooth_dejagging_smallword0;
	iedge_smooth_dejagging_smallword1_RBUS iedge_smooth_dejagging_smallword1;

	iedge_smooth_dejagging_ctrl0.regValue = IoReg_Read32(IEDGE_SMOOTH_dejagging_ctrl0_reg);
	iedge_smooth_dejagging_motion_support.regValue = IoReg_Read32(IEDGE_SMOOTH_dejagging_motion_support_reg);
	iedge_smooth_dejagging_smallword0.regValue = IoReg_Read32(IEDGE_SMOOTH_dejagging_smallword0_reg);
	iedge_smooth_dejagging_smallword1.regValue = IoReg_Read32(IEDGE_SMOOTH_dejagging_smallword1_reg);
	iedge_smooth_dejagging_ctrl0.i_dejag_skip_hlh = ptr->IESM_skip_hlh;
	iedge_smooth_dejagging_ctrl0.i_dejag_level= ptr->IESM_level;
	// removed iedge_smooth_dejagging_ctrl0.i_dejag_smallword_weak= ptr->IESM_smallword_weak;
	// removed iedge_smooth_dejagging_motion_support.i_dejag_delta_weak= ptr->IESM_delta_weak;
	iedge_smooth_dejagging_motion_support.i_dejag_delta_limit= ptr->IESM_delta_limit;
	iedge_smooth_dejagging_motion_support.i_dejag_delta_motion_gain= ptr->IESM_motion_gain;
	// removed iedge_smooth_dejagging_ctrl0.i_dejag_3tap_en= ptr->IESM_3tap_en;
	iedge_smooth_dejagging_ctrl0.i_dejag_delta_gain3= ptr->IESM_3tap_gain;
	iedge_smooth_dejagging_ctrl0.i_dejag_delta_gain5= ptr->IESM_5tap_gain;
	iedge_smooth_dejagging_ctrl0.i_dejag_what_the_rock= ptr->IESM_8diff_coring;
	iedge_smooth_dejagging_ctrl0.i_dejag_nondir_weight= ptr->IESM_particle2;
	iedge_smooth_dejagging_motion_support.i_dejag_delta_tex_seg= ptr->i_dejag_delta_tex_seg;
	iedge_smooth_dejagging_motion_support.i_dejag_nondir_tex_seg= ptr->i_dejag_nondir_tex_seg;
	iedge_smooth_dejagging_ctrl0.i_dejag_corner99= ptr->i_dejag_corner99;// IESM_particle2x
	iedge_smooth_dejagging_smallword0.i_dejag_smallword_5_0 = ptr->i_dejag_smallword_5_0;
	iedge_smooth_dejagging_smallword0.i_dejag_smallword_5_1 = ptr->i_dejag_smallword_5_1;
	iedge_smooth_dejagging_smallword0.i_dejag_smallword_7_0 = ptr->i_dejag_smallword_7_0;
	iedge_smooth_dejagging_smallword0.i_dejag_smallword_7_1 = ptr->i_dejag_smallword_7_1;
	iedge_smooth_dejagging_smallword0.i_dejag_smallword_7_2 = ptr->i_dejag_smallword_7_2;
	iedge_smooth_dejagging_smallword1.i_dejag_smallword_9_0 = ptr->i_dejag_smallword_9_0;
	iedge_smooth_dejagging_smallword1.i_dejag_smallword_9_1 = ptr->i_dejag_smallword_9_1;
	iedge_smooth_dejagging_smallword1.i_dejag_smallword_9_2 = ptr->i_dejag_smallword_9_2;
	iedge_smooth_dejagging_smallword1.i_dejag_smallword_9_3 = ptr->i_dejag_smallword_9_3;

	IoReg_Write32(IEDGE_SMOOTH_dejagging_ctrl0_reg, iedge_smooth_dejagging_ctrl0.regValue);
	IoReg_Write32(IEDGE_SMOOTH_dejagging_motion_support_reg, iedge_smooth_dejagging_motion_support.regValue);
	IoReg_Write32(IEDGE_SMOOTH_dejagging_smallword0_reg, iedge_smooth_dejagging_smallword0.regValue);
	IoReg_Write32(IEDGE_SMOOTH_dejagging_smallword1_reg, iedge_smooth_dejagging_smallword1.regValue);
}

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
		slope1_lm = (((unsigned int)ptr->MA_isnr_Get_Motion_MotionTh_M_1 - ptr->MA_isnr_Get_Motion_MotionTh_L_1)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_2 - ptr->MA_isnr_Get_Motion_EdgeTh_1)+1) >>1;
		slope2_lm = (((unsigned int)ptr->MA_isnr_Get_Motion_MotionTh_M_2 - ptr->MA_isnr_Get_Motion_MotionTh_L_2)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_2 - ptr->MA_isnr_Get_Motion_EdgeTh_1)+1) >>1;
		slope3_lm = (((unsigned int)ptr->MA_isnr_Get_Motion_MotionTh_M_3 - ptr->MA_isnr_Get_Motion_MotionTh_L_3)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_2 - ptr->MA_isnr_Get_Motion_EdgeTh_1)+1) >>1;
	}
	if (ptr->MA_isnr_Get_Motion_EdgeTh_3 != ptr->MA_isnr_Get_Motion_EdgeTh_2) {
		slope1_mn = (((unsigned int)ptr->MA_isnr_Get_Motion_MotionTh_N_1 - ptr->MA_isnr_Get_Motion_MotionTh_M_1)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_3 - ptr->MA_isnr_Get_Motion_EdgeTh_2)+1) >>1;
		slope2_mn = (((unsigned int)ptr->MA_isnr_Get_Motion_MotionTh_N_2 - ptr->MA_isnr_Get_Motion_MotionTh_M_2)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_3 - ptr->MA_isnr_Get_Motion_EdgeTh_2)+1) >>1;
		slope3_mn = (((unsigned int)ptr->MA_isnr_Get_Motion_MotionTh_N_3 - ptr->MA_isnr_Get_Motion_MotionTh_M_3)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_3 - ptr->MA_isnr_Get_Motion_EdgeTh_2)+1) >>1;
	}
	if (ptr->MA_isnr_Get_Motion_EdgeTh_4 != ptr->MA_isnr_Get_Motion_EdgeTh_3) {
		slope1_nh = (((unsigned int)ptr->MA_isnr_Get_Motion_MotionTh_H_1 - ptr->MA_isnr_Get_Motion_MotionTh_N_1)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_4 - ptr->MA_isnr_Get_Motion_EdgeTh_3)+1) >>1;
		slope2_nh = (((unsigned int)ptr->MA_isnr_Get_Motion_MotionTh_H_2 - ptr->MA_isnr_Get_Motion_MotionTh_N_2)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_4 - ptr->MA_isnr_Get_Motion_EdgeTh_3)+1) >>1;
		slope3_nh = (((unsigned int)ptr->MA_isnr_Get_Motion_MotionTh_H_3 - ptr->MA_isnr_Get_Motion_MotionTh_N_3)*32/(ptr->MA_isnr_Get_Motion_EdgeTh_4 - ptr->MA_isnr_Get_Motion_EdgeTh_3)+1) >>1;
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
	nr_motion_weight_ctrl_reg.vc_nr_motion_signwt1 = ptr->nr_motion_signwt1;
	nr_motion_weight_ctrl_reg.vc_nr_motion_signwt2 = ptr->nr_motion_signwt2;
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
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
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
	iedge_smooth_vfilter_6_RBUS vfilter_6_reg;
	eng_clamp_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_ENG_CLAMP_reg);
	vfilter_6_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_VFILTER_6_reg);

	eng_clamp_reg.iedsm_motion_en = ptr->IESM_Motion_En;
	vfilter_6_reg.egsm_avlpf_motion_en = ptr ->AVLPF_Motion_En ;
	vfilter_6_reg.egsm_motion_i2p_blendmode = ptr ->IESM_I2P_Blend_Mode;

	IoReg_Write32(IEDGE_SMOOTH_ENG_CLAMP_reg, eng_clamp_reg.regValue);
	IoReg_Write32(IEDGE_SMOOTH_VFILTER_6_reg, vfilter_6_reg.regValue);
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

}

void drvif_color_DRV_MA_Bdecontour_Motion_Weight(DRV_MA_Bdecont_Motion_Weight *ptr) // ml8_add
{
	ipq_decontour_blk_de_contour_ctrl_RBUS blk_de_contour_ctrl_reg;
	ipq_decontour_bdecont_motion_weight_0_RBUS bdecont_motion_weight_0_reg;
	ipq_decontour_bdecont_motion_weight_1_RBUS bdecont_motion_weight_1_reg;
	ipq_decontour_bdecont_motion_weight_2_RBUS bdecont_motion_weight_2_reg;
	ipq_decontour_bdecont_motion_weight_3_RBUS bdecont_motion_weight_3_reg;
	ipq_decontour_bdecont_motion_weight_4_RBUS bdecont_motion_weight_4_reg;
	ipq_decontour_bdecont_motion_weight_5_RBUS bdecont_motion_weight_5_reg;

	blk_de_contour_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg);
	bdecont_motion_weight_0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_0_reg);
	bdecont_motion_weight_1_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_1_reg);
	bdecont_motion_weight_2_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_2_reg);
	bdecont_motion_weight_3_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_3_reg);
	bdecont_motion_weight_4_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_4_reg);
	bdecont_motion_weight_5_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_5_reg);

	blk_de_contour_ctrl_reg.bdecont_motion_en = ptr->Bdecont_Motion_en; // ml8
	bdecont_motion_weight_0_reg.bdecont_motionweight0 = ptr->Bdecont_Motion_Weight_0;
	bdecont_motion_weight_0_reg.bdecont_motionweight1 = ptr->Bdecont_Motion_Weight_1;
	bdecont_motion_weight_0_reg.bdecont_motionweight2 = ptr->Bdecont_Motion_Weight_2;
	bdecont_motion_weight_0_reg.bdecont_motionweight3 = ptr->Bdecont_Motion_Weight_3;
	bdecont_motion_weight_0_reg.bdecont_motionweight4 = ptr->Bdecont_Motion_Weight_4;
	bdecont_motion_weight_0_reg.bdecont_motionweight5 = ptr->Bdecont_Motion_Weight_5;
	bdecont_motion_weight_1_reg.bdecont_motionweight6 = ptr->Bdecont_Motion_Weight_6;
	bdecont_motion_weight_1_reg.bdecont_motionweight7 = ptr->Bdecont_Motion_Weight_7;
	bdecont_motion_weight_1_reg.bdecont_motionweight8 = ptr->Bdecont_Motion_Weight_8;
	bdecont_motion_weight_1_reg.bdecont_motionweight9 = ptr->Bdecont_Motion_Weight_9;
	bdecont_motion_weight_1_reg.bdecont_motionweight10= ptr->Bdecont_Motion_Weight_10;
	bdecont_motion_weight_1_reg.bdecont_motionweight11= ptr->Bdecont_Motion_Weight_11;
	bdecont_motion_weight_2_reg.bdecont_motionweight12 = ptr->Bdecont_Motion_Weight_12;
	bdecont_motion_weight_2_reg.bdecont_motionweight13 = ptr->Bdecont_Motion_Weight_13;
	bdecont_motion_weight_2_reg.bdecont_motionweight14 = ptr->Bdecont_Motion_Weight_14;
	bdecont_motion_weight_2_reg.bdecont_motionweight15 = ptr->Bdecont_Motion_Weight_15;
	bdecont_motion_weight_2_reg.bdecont_motionweight16 = ptr->Bdecont_Motion_Weight_16;
	bdecont_motion_weight_2_reg.bdecont_motionweight17 = ptr->Bdecont_Motion_Weight_17;
	bdecont_motion_weight_3_reg.bdecont_motionweight18 = ptr->Bdecont_Motion_Weight_18;
	bdecont_motion_weight_3_reg.bdecont_motionweight19 = ptr->Bdecont_Motion_Weight_19;
	bdecont_motion_weight_3_reg.bdecont_motionweight20 = ptr->Bdecont_Motion_Weight_20;
	bdecont_motion_weight_3_reg.bdecont_motionweight21 = ptr->Bdecont_Motion_Weight_21;
	bdecont_motion_weight_3_reg.bdecont_motionweight22 = ptr->Bdecont_Motion_Weight_22;
	bdecont_motion_weight_3_reg.bdecont_motionweight23 = ptr->Bdecont_Motion_Weight_23;
	bdecont_motion_weight_4_reg.bdecont_motionweight24 = ptr->Bdecont_Motion_Weight_24;
	bdecont_motion_weight_4_reg.bdecont_motionweight25 = ptr->Bdecont_Motion_Weight_25;
	bdecont_motion_weight_4_reg.bdecont_motionweight26 = ptr->Bdecont_Motion_Weight_26;
	bdecont_motion_weight_4_reg.bdecont_motionweight27 = ptr->Bdecont_Motion_Weight_27;
	bdecont_motion_weight_4_reg.bdecont_motionweight28 = ptr->Bdecont_Motion_Weight_28;
	bdecont_motion_weight_4_reg.bdecont_motionweight29 = ptr->Bdecont_Motion_Weight_29;
	bdecont_motion_weight_5_reg.bdecont_motionweight30 = ptr->Bdecont_Motion_Weight_30;
	bdecont_motion_weight_5_reg.bdecont_motionweight31 = ptr->Bdecont_Motion_Weight_31;
	
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg, blk_de_contour_ctrl_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_0_reg, bdecont_motion_weight_0_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_1_reg, bdecont_motion_weight_1_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_2_reg, bdecont_motion_weight_2_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_3_reg, bdecont_motion_weight_3_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_4_reg, bdecont_motion_weight_4_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BDECONT_MOTION_WEIGHT_5_reg, bdecont_motion_weight_5_reg.regValue);
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

void drvif_color_DRV_DESM_Basic(DRV_DESM_Basic *ptr)
{
	// removed
}

void drvif_color_DRV_DESM_Dirlpf(DRV_DESM_Dirlpf *ptr)
{
	// removed
}

void drvif_color_DRV_DESM_Postshp(DRV_DESM_Postshp *ptr)
{
	// removed
}

#if 0
void drvif_color_DEDGESMOOTH_flow(DRV_DEdgeSmooth_Coef *ptr)
{
	if (!ptr) {
		VIPprintf("drvif_color_DEDGESMOOTH_flow table to NULL\n");
		return;
	}
	drvif_color_DRV_DESM_Basic(&(ptr->S_DESM_Basic));
	drvif_color_DRV_DESM_Dirlpf(&(ptr->S_DESM_Dirlpf));
	drvif_color_DRV_DESM_Postshp(&(ptr->S_DESM_Postshp));
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
#ifdef CONFIG_I2RND_ENABLE
extern unsigned char Scaler_I2rnd_get_timing_enable(void);
#endif
unsigned char  drvif_color_Set_I_De_XC_En(unsigned char value)
{
	de_xcxl_de_xcxl_ctrl_RBUS de_xcxl_de_xcxl_ctrl_reg;
	de_xcxl_de_xcxl_db_reg_ctl_RBUS de_xcxl_de_xcxl_db_reg_ctl_reg;

	/* db must be on*/
	de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = IoReg_Read32(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	de_xcxl_de_xcxl_db_reg_ctl_reg.db_en = 1;
	IoReg_Write32(DE_XCXL_DE_XCXL_db_reg_ctl_reg, de_xcxl_de_xcxl_db_reg_ctl_reg.regValue);

	de_xcxl_de_xcxl_ctrl_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CTRL_reg);
	de_xcxl_de_xcxl_ctrl_reg.dexc_en					= value;

	IoReg_Write32(DE_XCXL_De_XCXL_CTRL_reg, de_xcxl_de_xcxl_ctrl_reg.regValue);

	de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = IoReg_Read32(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	de_xcxl_de_xcxl_db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DE_XCXL_DE_XCXL_db_reg_ctl_reg, de_xcxl_de_xcxl_db_reg_ctl_reg.regValue);

	return 0;
}

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
	de_xcxl_de_xcxl_smooth_th_RBUS de_xcxl_de_xcxl_smooth_th_reg;

	de_xcxl_de_xcxl_ctrl_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CTRL_reg);
	de_xcxl_de_xcxl_temporal_step_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_TEMPORAL_STEP_reg);
	de_xcxl_de_xcxl_temporal_step_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_TEMPORAL_STEP_PAL_reg);
	de_xcxl_de_xcxl_classify_y_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CLASSIFY_Y_PAL_reg);
	de_xcxl_de_xcxl_classify_c_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CLASSIFY_C_PAL_reg);
	de_xcxl_de_xcxl_classify_y_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CLASSIFY_Y_PAL_reg);
	de_xcxl_de_xcxl_classify_c_pal_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_CLASSIFY_C_PAL_reg);
	de_xcxl_de_xcxl_blend_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_BLEND_reg);

        de_xcxl_de_xcxl_smooth_th_reg.regValue = IoReg_Read32(DE_XCXL_De_XCXL_SMOOTH_TH_reg);



	/* db must be on*/
	de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = IoReg_Read32(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	de_xcxl_de_xcxl_db_reg_ctl_reg.db_en = 1;
	IoReg_Write32(DE_XCXL_DE_XCXL_db_reg_ctl_reg, de_xcxl_de_xcxl_db_reg_ctl_reg.regValue);

#ifdef CONFIG_I2RND_ENABLE
	if(Scaler_I2rnd_get_timing_enable())
		de_xcxl_de_xcxl_ctrl_reg.dexc_en					= 0;
	else
#endif
        de_xcxl_de_xcxl_ctrl_reg.dexc_en					=		ptr->dexc_en;
	de_xcxl_de_xcxl_ctrl_reg.dexc_spatial_correction_en			=		ptr->Dexc_spatial_correction_en;
	de_xcxl_de_xcxl_ctrl_reg.dexc_detect_type				=		ptr->dexc_detect_type;

        de_xcxl_de_xcxl_smooth_th_reg.dexc_sat_th                               =		ptr->dexc_sat_th;
        de_xcxl_de_xcxl_smooth_th_reg.dexc_smooth_th                            =		ptr->dexc_smooth_th;
        de_xcxl_de_xcxl_smooth_th_reg.dexc_spatial_corr_imp_cancel              =		ptr->dexc_spatial_corr_imp_cancel;


	de_xcxl_de_xcxl_classify_y_reg.dexc_y_error_th1				=		ptr->dexc_y_error_th1_ntsc;
	de_xcxl_de_xcxl_classify_y_reg.dexc_y_error_th2				=		ptr->dexc_y_error_th2_ntsc;
	de_xcxl_de_xcxl_classify_y_reg.dexc_y_amp_th				=		ptr->dexc_y_amp_th_ntsc;
	de_xcxl_de_xcxl_classify_c_reg.dexc_c_error_th1				=		ptr->dexc_c_error_th1_ntsc;
	de_xcxl_de_xcxl_classify_c_reg.dexc_c_error_th2				=		ptr->dexc_c_error_th2_ntsc;
	de_xcxl_de_xcxl_classify_c_reg.dexc_c_amp_th				=		ptr->dexc_c_amp_th_ntsc;
	de_xcxl_de_xcxl_temporal_step_reg.dexc_step_lv1				=		ptr->dexc_step_lv1_ntsc;
	de_xcxl_de_xcxl_temporal_step_reg.dexc_step_lv2				=		ptr->dexc_step_lv2_ntsc;
	de_xcxl_de_xcxl_temporal_step_reg.dexc_step_still			=		ptr->dexc_step_still_ntsc;
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
        IoReg_Write32(DE_XCXL_De_XCXL_SMOOTH_TH_reg,de_xcxl_de_xcxl_smooth_th_reg.regValue);

	de_xcxl_de_xcxl_db_reg_ctl_reg.regValue = IoReg_Read32(DE_XCXL_DE_XCXL_db_reg_ctl_reg);
	de_xcxl_de_xcxl_db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DE_XCXL_DE_XCXL_db_reg_ctl_reg, de_xcxl_de_xcxl_db_reg_ctl_reg.regValue);

	return TRUE;
}

void drvif_color_Set_I_De_Contour_DB_En( unsigned char DB_En )
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

	ipq_decontour_de_contour_ctrl_reg.decont_en                      	= (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(ptr->decont_en);
	ipq_decontour_de_contour_ctrl_reg.decont_snr_blend_mode          	= ptr->decont_snr_blend_mode;
	ipq_decontour_de_contour_ctrl_reg.decont_blend_lowbd             	= ptr->decont_blend_lowbd;
	ipq_decontour_de_contour_ctrl_reg.decont_blend_step              	= ptr->decont_blend_step;
	ipq_decontour_de_contour_ctrl_reg.decont_blend_weight_lpf_en		= ptr->decont_blend_weight_lpf_en;
	ipq_decontour_de_contour_ctrl_reg.decont_neighbor_far            	= ptr->decont_neighbor_far;
	ipq_decontour_de_contour_ctrl_reg.decont_filter_mode             	= ptr->decont_filter_mode;
	ipq_decontour_de_contour_ctrl_reg.decont_line_mode       	     	= ptr->decont_line_mode;
	ipq_decontour_de_contour_ctrl_reg.decont_yc_mode			= ptr->decont_yc_mode;
	ipq_decontour_de_contour_ctrl_reg.decont_bottleneck_en			= ptr->decont_bottleneck_en;
	ipq_decontour_de_contour_ctrl_reg.decont_blend_semantic_en = ptr->decont_blend_semantic_en;//mk2

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
	ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range4	= ptr->decont_bottleneck_range4;

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
	ptr->decont_filter_mode		= ipq_decontour_de_contour_ctrl_reg.decont_filter_mode;
	ptr->decont_line_mode 		= ipq_decontour_de_contour_ctrl_reg.decont_line_mode;
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
	ptr->decont_bottleneck_range4    = ipq_decontour_de_contour_bottleneck_reg.decont_bottleneck_range4;
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

void drvif_color_Set_I_De_Contour_PQ_Mask_Enable(unsigned int bEnable) {

	ipq_decontour_de_contour_ctrl_RBUS ipq_decontour_de_contour_ctrl_reg;

	ipq_decontour_de_contour_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_De_Contour_CTRL_reg);

	ipq_decontour_de_contour_ctrl_reg.decont_blend_semantic_en = (bEnable == 0)? 0 : 1;

	IoReg_Write32(IPQ_DECONTOUR_De_Contour_CTRL_reg, ipq_decontour_de_contour_ctrl_reg.regValue);

	drvif_color_Set_I_De_Contour_DB_Apply();
}

void drvif_color_Set_Block_DeContour_Ctrl(DRV_Block_DeContour_Ctrl *ptr)
{
	ipq_decontour_blk_de_contour_ctrl_RBUS blk_de_contour_ctrl_reg;
	ipq_decontour_blk_de_profile_diff_dist_RBUS blk_de_profile_diff_dist_reg;

	blk_de_contour_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg);
	blk_de_profile_diff_dist_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_profile_diff_dist_reg);

	blk_de_contour_ctrl_reg.bdecont_en 			= ptr->bdecont_en;
	blk_de_contour_ctrl_reg.bdecont_en_c 		= ptr->bdecont_en_c;
	//blk_de_contour_ctrl_reg.bdecont_tenable		= ptr->bdecont_tenable;
	blk_de_contour_ctrl_reg.bdecont_dither_en 	= ptr->bdecont_dither_en;
	blk_de_contour_ctrl_reg.bdecont_profile_diff_gain_en = ptr->bdecont_profile_diff_gain_en;
	blk_de_contour_ctrl_reg.bdecont_diff_clamp_en 			= ptr->bdecont_diff_clamp_en;			// mk2
	blk_de_contour_ctrl_reg.bdecont_blend_semantic_en = ptr->bdecont_blend_semantic_en;				// mk2

	blk_de_contour_ctrl_reg.bdecont_profile_diff_right_dn_en 		= ptr->bdecont_profile_diff_right_dn_en;	// h5c2  // ml8_add
	blk_de_profile_diff_dist_reg.bdecont_profile_diff_blkv_dn 		= ptr->bdecont_profile_diff_blkv_dn;		// h5c2
	blk_de_profile_diff_dist_reg.bdecont_profile_diff_blkh_right 	= ptr->bdecont_profile_diff_blkh_right;	// h5c2

	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg, blk_de_contour_ctrl_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_profile_diff_dist_reg, blk_de_profile_diff_dist_reg.regValue);
	drvif_color_Set_I_De_Contour_DB_Apply();

}


unsigned char  drvif_color_Get_Block_DeContour_Ctrl(DRV_Block_DeContour_Ctrl *ptr)
{
	ipq_decontour_blk_de_contour_ctrl_RBUS blk_de_contour_ctrl_reg;
	ipq_decontour_blk_de_profile_diff_dist_RBUS blk_de_profile_diff_dist_reg;

	blk_de_contour_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg);
	blk_de_profile_diff_dist_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_profile_diff_dist_reg);

	ptr->bdecont_en = blk_de_contour_ctrl_reg.bdecont_en;
	ptr->bdecont_en_c = blk_de_contour_ctrl_reg.bdecont_en_c;
	//ptr->bdecont_tenable = blk_de_contour_ctrl_reg.bdecont_tenable;
	ptr->bdecont_dither_en = blk_de_contour_ctrl_reg.bdecont_dither_en;
	ptr->bdecont_profile_diff_gain_en = blk_de_contour_ctrl_reg.bdecont_profile_diff_gain_en;
	ptr->bdecont_diff_clamp_en = blk_de_contour_ctrl_reg.bdecont_diff_clamp_en;
	ptr->bdecont_blend_semantic_en = blk_de_contour_ctrl_reg.bdecont_blend_semantic_en;

	ptr->bdecont_profile_diff_right_dn_en = blk_de_contour_ctrl_reg.bdecont_profile_diff_right_dn_en;	// c2  // ml8_add
	ptr->bdecont_profile_diff_blkv_dn = blk_de_profile_diff_dist_reg.bdecont_profile_diff_blkv_dn;		// c2
	ptr->bdecont_profile_diff_blkh_right = blk_de_profile_diff_dist_reg.bdecont_profile_diff_blkh_right;	// c2

	return TRUE;
}


void drvif_color_Set_Block_DeContour_PQ_Mask_Enable(unsigned char bEnable) {

	ipq_decontour_blk_de_contour_ctrl_RBUS blk_de_contour_ctrl_reg;

	blk_de_contour_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg);

	blk_de_contour_ctrl_reg.bdecont_blend_semantic_en = (bEnable == 0)? 0 : 1;

	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg, blk_de_contour_ctrl_reg.regValue);

	drvif_color_Set_I_De_Contour_DB_Apply();
}

void drvif_color_Set_Block_DeContour_Temporal(DRV_Block_DeContour_Temporal *ptr)
{
#if 0
	ipq_decontour_blk_de_contour_tmp0_RBUS blk_de_contour_tmp0_reg;
	ipq_decontour_blk_de_contour_tmp1_RBUS blk_de_contour_tmp1_reg;
	ipq_decontour_blk_de_contour_hinit_RBUS blk_de_contour_hinit_reg;

	blk_de_contour_tmp0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Tmp0_reg);
	blk_de_contour_tmp1_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Tmp1_reg);
	blk_de_contour_hinit_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Hinit_reg);

	blk_de_contour_tmp0_reg.bdecont_tmp_pos0thd 		= ptr->bdecont_tmp_pos0thd;
	blk_de_contour_tmp0_reg.bdecont_tmp_pos1thd 		= ptr->bdecont_tmp_pos1thd;
	blk_de_contour_tmp0_reg.bdecont_tmp_neg0thd 		= ptr->bdecont_tmp_neg0thd;
	blk_de_contour_tmp0_reg.bdecont_tmp_neg1thd 		= ptr->bdecont_tmp_neg1thd;
	blk_de_contour_tmp1_reg.bdecont_tmp_maxdiff 		= ptr->bdecont_tmp_maxdiff;
	blk_de_contour_tmp1_reg.bdecont_tmp_posmaxgain 	= ptr->bdecont_tmp_posmaxgain;
	blk_de_contour_tmp1_reg.bdecont_tmp_posmingain 		= ptr->bdecont_tmp_posmingain;
	blk_de_contour_tmp1_reg.bdecont_tmp_negmaxgain		= ptr->bdecont_tmp_negmaxgain;
	blk_de_contour_tmp1_reg.bdecont_tmp_negmingain		= ptr->bdecont_tmp_negmingain;
	blk_de_contour_hinit_reg.bdecont_tmp_scenechangegain1	= ptr->bdecont_tmp_scenechangegain1;

	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Tmp0_reg, blk_de_contour_tmp0_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Tmp1_reg, blk_de_contour_tmp1_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Hinit_reg, blk_de_contour_hinit_reg.regValue);
#endif
}

void drvif_color_Set_Block_DeContour_Interpolation(DRV_Block_DeContour_Interpolation *ptr)
{
	ipq_decontour_blk_de_contour_hinit_RBUS blk_de_contour_hinit_reg;
	ipq_decontour_blk_de_contour_vinit_RBUS blk_de_contour_vinit_reg;
	ipq_decontour_block_decontour_interp_filter0_RBUS blk_de_contour_interp_filter0_reg;
	ipq_decontour_block_decontour_interp_filter1_RBUS blk_de_contour_interp_filter1_reg;
	ipq_decontour_block_decontour_interp_filter2_RBUS blk_de_contour_interp_filter2_reg;
	ipq_decontour_block_decontour_interp_filter3_RBUS blk_de_contour_interp_filter3_reg;
	ipq_decontour_block_decontour_interp_filter4_RBUS blk_de_contour_interp_filter4_reg;

	blk_de_contour_hinit_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Hinit_reg);
	blk_de_contour_vinit_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Vinit_reg);
	blk_de_contour_interp_filter0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter0_reg);
	blk_de_contour_interp_filter1_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter1_reg);
	blk_de_contour_interp_filter2_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter2_reg);
	blk_de_contour_interp_filter3_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter3_reg);
	blk_de_contour_interp_filter4_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter4_reg);

	blk_de_contour_hinit_reg.bdecont_hboundary	= ptr->bdecont_hboundary;
	blk_de_contour_hinit_reg.bdecont_hinitphase1	= ptr->bdecont_hinitphase1;
	blk_de_contour_hinit_reg.bdecont_hinitphase2	= ptr->bdecont_hinitphase2;
	blk_de_contour_hinit_reg.bdecont_hinitphase3	= ptr->bdecont_hinitphase3;
	blk_de_contour_vinit_reg.bdecont_vboundary	= ptr->bdecont_vboundary;
	blk_de_contour_vinit_reg.bdecont_vinitphase1	= ptr->bdecont_vinitphase1;
	blk_de_contour_vinit_reg.bdecont_vinitphase2	= ptr->bdecont_vinitphase2;
	blk_de_contour_vinit_reg.bdecont_vinitphase3	= ptr->bdecont_vinitphase3;

	blk_de_contour_interp_filter4_reg.bdecont_interp_filter0	= ptr->bdecont_interp_filter_y[0];
	blk_de_contour_interp_filter3_reg.bdecont_interp_filter1	= ptr->bdecont_interp_filter_y[1];
	blk_de_contour_interp_filter3_reg.bdecont_interp_filter2	= ptr->bdecont_interp_filter_y[2];
	blk_de_contour_interp_filter3_reg.bdecont_interp_filter3	= ptr->bdecont_interp_filter_y[3];
	blk_de_contour_interp_filter2_reg.bdecont_interp_filter4	= ptr->bdecont_interp_filter_y[4];
	blk_de_contour_interp_filter2_reg.bdecont_interp_filter5	= ptr->bdecont_interp_filter_y[5];
	blk_de_contour_interp_filter2_reg.bdecont_interp_filter6	= ptr->bdecont_interp_filter_y[6];
	blk_de_contour_interp_filter1_reg.bdecont_interp_filter7	= ptr->bdecont_interp_filter_y[7];
	blk_de_contour_interp_filter1_reg.bdecont_interp_filter8	= ptr->bdecont_interp_filter_y[8];
	blk_de_contour_interp_filter1_reg.bdecont_interp_filter9	= ptr->bdecont_interp_filter_y[9];
	blk_de_contour_interp_filter0_reg.bdecont_interp_filter10	= ptr->bdecont_interp_filter_y[10];
	blk_de_contour_interp_filter0_reg.bdecont_interp_filter11	= ptr->bdecont_interp_filter_y[11];
	blk_de_contour_interp_filter0_reg.bdecont_interp_filter12	= ptr->bdecont_interp_filter_y[12];

	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Hinit_reg, blk_de_contour_hinit_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Vinit_reg, blk_de_contour_vinit_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter0_reg, blk_de_contour_interp_filter0_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter1_reg, blk_de_contour_interp_filter1_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter2_reg, blk_de_contour_interp_filter2_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter3_reg, blk_de_contour_interp_filter3_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_Block_DeContour_Interp_Filter4_reg, blk_de_contour_interp_filter4_reg.regValue);

}

void drvif_color_Set_Block_DeContour_Alpha(DRV_Block_DeContour_Alpha *ptr)
{
	ipq_decontour_blk_de_contour_alpha_ydiff0_RBUS blk_de_contour_alpha_ydiff0_reg;
	ipq_decontour_blk_de_contour_alpha_ydiff1_RBUS blk_de_contour_alpha_ydiff1_reg;
	ipq_decontour_blk_de_contour_alpha_ydiff2_RBUS blk_de_contour_alpha_ydiff2_reg;
	ipq_decontour_blk_de_contour_alpha_ydiff3_RBUS blk_de_contour_alpha_ydiff3_reg;
	ipq_decontour_blk_de_contour_alpha_ydiff4_RBUS blk_de_contour_alpha_ydiff4_reg;
	ipq_decontour_blk_de_contour_alpha_cdiff0_RBUS blk_de_contour_alpha_cdiff0_reg;
	ipq_decontour_blk_de_contour_alpha_cdiff1_RBUS blk_de_contour_alpha_cdiff1_reg;
	ipq_decontour_blk_de_contour_alpha_cdiff2_RBUS blk_de_contour_alpha_cdiff2_reg;
	ipq_decontour_blk_de_contour_alpha_cdiff3_RBUS blk_de_contour_alpha_cdiff3_reg;
	ipq_decontour_blk_de_contour_alpha_cdiff4_RBUS blk_de_contour_alpha_cdiff4_reg;

	blk_de_contour_alpha_ydiff0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff0_reg);
	blk_de_contour_alpha_ydiff1_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff1_reg);
	blk_de_contour_alpha_ydiff2_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff2_reg);
	blk_de_contour_alpha_ydiff3_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff3_reg);
	blk_de_contour_alpha_ydiff4_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff4_reg);
	blk_de_contour_alpha_cdiff0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff0_reg);
	blk_de_contour_alpha_cdiff1_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff1_reg);
	blk_de_contour_alpha_cdiff2_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff2_reg);
	blk_de_contour_alpha_cdiff3_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff3_reg);
	blk_de_contour_alpha_cdiff4_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff4_reg);

	blk_de_contour_alpha_ydiff0_reg.bdecont_ydiff_shift			= ptr->bdecont_ydiff_shift;
	blk_de_contour_alpha_ydiff4_reg.bdecont_alpha_by_ydiff0	= ptr->bdecont_alpha_by_ydiff[0];
	blk_de_contour_alpha_ydiff3_reg.bdecont_alpha_by_ydiff1	= ptr->bdecont_alpha_by_ydiff[1];
	blk_de_contour_alpha_ydiff3_reg.bdecont_alpha_by_ydiff2	= ptr->bdecont_alpha_by_ydiff[2];
	blk_de_contour_alpha_ydiff3_reg.bdecont_alpha_by_ydiff3	= ptr->bdecont_alpha_by_ydiff[3];
	blk_de_contour_alpha_ydiff3_reg.bdecont_alpha_by_ydiff4	= ptr->bdecont_alpha_by_ydiff[4];
	blk_de_contour_alpha_ydiff2_reg.bdecont_alpha_by_ydiff5	= ptr->bdecont_alpha_by_ydiff[5];
	blk_de_contour_alpha_ydiff2_reg.bdecont_alpha_by_ydiff6	= ptr->bdecont_alpha_by_ydiff[6];
	blk_de_contour_alpha_ydiff2_reg.bdecont_alpha_by_ydiff7	= ptr->bdecont_alpha_by_ydiff[7];
	blk_de_contour_alpha_ydiff2_reg.bdecont_alpha_by_ydiff8	= ptr->bdecont_alpha_by_ydiff[8];
	blk_de_contour_alpha_ydiff1_reg.bdecont_alpha_by_ydiff9	= ptr->bdecont_alpha_by_ydiff[9];
	blk_de_contour_alpha_ydiff1_reg.bdecont_alpha_by_ydiff10	= ptr->bdecont_alpha_by_ydiff[10];
	blk_de_contour_alpha_ydiff1_reg.bdecont_alpha_by_ydiff11	= ptr->bdecont_alpha_by_ydiff[11];
	blk_de_contour_alpha_ydiff1_reg.bdecont_alpha_by_ydiff12	= ptr->bdecont_alpha_by_ydiff[12];
	blk_de_contour_alpha_ydiff0_reg.bdecont_alpha_by_ydiff13	= ptr->bdecont_alpha_by_ydiff[13];
	blk_de_contour_alpha_ydiff0_reg.bdecont_alpha_by_ydiff14	= ptr->bdecont_alpha_by_ydiff[14];
	blk_de_contour_alpha_ydiff0_reg.bdecont_alpha_by_ydiff15	= ptr->bdecont_alpha_by_ydiff[15];

	blk_de_contour_alpha_cdiff0_reg.bdecont_cdiff_shift			= ptr->bdecont_cdiff_shift;
	blk_de_contour_alpha_cdiff4_reg.bdecont_alpha_by_cdiff0	= ptr->bdecont_alpha_by_cdiff[0];
	blk_de_contour_alpha_cdiff3_reg.bdecont_alpha_by_cdiff1	= ptr->bdecont_alpha_by_cdiff[1];
	blk_de_contour_alpha_cdiff3_reg.bdecont_alpha_by_cdiff2	= ptr->bdecont_alpha_by_cdiff[2];
	blk_de_contour_alpha_cdiff3_reg.bdecont_alpha_by_cdiff3	= ptr->bdecont_alpha_by_cdiff[3];
	blk_de_contour_alpha_cdiff3_reg.bdecont_alpha_by_cdiff4	= ptr->bdecont_alpha_by_cdiff[4];
	blk_de_contour_alpha_cdiff2_reg.bdecont_alpha_by_cdiff5	= ptr->bdecont_alpha_by_cdiff[5];
	blk_de_contour_alpha_cdiff2_reg.bdecont_alpha_by_cdiff6	= ptr->bdecont_alpha_by_cdiff[6];
	blk_de_contour_alpha_cdiff2_reg.bdecont_alpha_by_cdiff7	= ptr->bdecont_alpha_by_cdiff[7];
	blk_de_contour_alpha_cdiff2_reg.bdecont_alpha_by_cdiff8	= ptr->bdecont_alpha_by_cdiff[8];
	blk_de_contour_alpha_cdiff1_reg.bdecont_alpha_by_cdiff9	= ptr->bdecont_alpha_by_cdiff[9];
	blk_de_contour_alpha_cdiff1_reg.bdecont_alpha_by_cdiff10	= ptr->bdecont_alpha_by_cdiff[10];
	blk_de_contour_alpha_cdiff1_reg.bdecont_alpha_by_cdiff11	= ptr->bdecont_alpha_by_cdiff[11];
	blk_de_contour_alpha_cdiff1_reg.bdecont_alpha_by_cdiff12	= ptr->bdecont_alpha_by_cdiff[12];
	blk_de_contour_alpha_cdiff0_reg.bdecont_alpha_by_cdiff13	= ptr->bdecont_alpha_by_cdiff[13];
	blk_de_contour_alpha_cdiff0_reg.bdecont_alpha_by_cdiff14	= ptr->bdecont_alpha_by_cdiff[14];
	blk_de_contour_alpha_cdiff0_reg.bdecont_alpha_by_cdiff15	= ptr->bdecont_alpha_by_cdiff[15];

	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff0_reg, blk_de_contour_alpha_ydiff0_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff1_reg, blk_de_contour_alpha_ydiff1_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff2_reg, blk_de_contour_alpha_ydiff2_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff3_reg, blk_de_contour_alpha_ydiff3_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff4_reg, blk_de_contour_alpha_ydiff4_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff0_reg, blk_de_contour_alpha_cdiff0_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff1_reg, blk_de_contour_alpha_cdiff1_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff2_reg, blk_de_contour_alpha_cdiff2_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff3_reg, blk_de_contour_alpha_cdiff3_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff4_reg, blk_de_contour_alpha_cdiff4_reg.regValue);

}

void drvif_color_Set_Block_DeContour_Region_Num_Size(DRV_Block_DeContour_Region_Num_Size *ptr)
{
	ipq_decontour_blk_de_contour_size0_RBUS blk_de_contour_size0_reg;
	ipq_decontour_blk_de_contour_size1_RBUS blk_de_contour_size1_reg;
	ipq_decontour_blk_de_contour_size2_RBUS blk_de_contour_size2_reg;
	ipq_decontour_block_decontour_vsize_remain_RBUS blk_de_contour_vsize_remain_reg;

	blk_de_contour_size0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Size0_reg);
	blk_de_contour_size1_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Size1_reg);
	blk_de_contour_size2_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Size2_reg);
	blk_de_contour_vsize_remain_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_Block_DeContour_vsize_remain_reg);

	blk_de_contour_size0_reg.bdecont_blk_hnum 	= ptr->bdecont_blk_hnum;
	blk_de_contour_size1_reg.bdecont_blk_vnum	= ptr->bdecont_blk_vnum;
	blk_de_contour_size2_reg.bdecont_blk_hsize	= ptr->bdecont_blk_hsize;
	blk_de_contour_size2_reg.bdecont_blk_vsize	= ptr->bdecont_blk_vsize;
	blk_de_contour_size0_reg.bdecont_hfactor		= ptr->bdecont_hfactor;
	blk_de_contour_size1_reg.bdecont_vfactor 		= ptr->bdecont_vfactor;
	blk_de_contour_vsize_remain_reg.bdecont_blk_vsize_remainder	= ptr->bdecont_blk_vsize_remainder;

	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Size0_reg, blk_de_contour_size0_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Size1_reg, blk_de_contour_size1_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Size2_reg, blk_de_contour_size2_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_Block_DeContour_vsize_remain_reg, blk_de_contour_vsize_remain_reg.regValue);

}

void drvif_color_Set_Block_DeContour_Dither(DRV_Block_DeContour_Dither *ptr)
{
	ipq_decontour_block_decontour_dither_y_RBUS blk_de_contour_dither_y_reg;
	ipq_decontour_block_decontour_dither_u_RBUS blk_de_contour_dither_u_reg;
	ipq_decontour_block_decontour_dither_v_RBUS blk_de_contour_dither_v_reg;

	blk_de_contour_dither_y_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_Block_DeContour_dither_y_reg);
	blk_de_contour_dither_u_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_Block_DeContour_dither_u_reg);
	blk_de_contour_dither_v_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_Block_DeContour_dither_v_reg);

	blk_de_contour_dither_y_reg.bdecont_dither_table_y00	= ptr->bdecont_dither_table_y[0];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y01	= ptr->bdecont_dither_table_y[1];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y02	= ptr->bdecont_dither_table_y[2];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y03	= ptr->bdecont_dither_table_y[3];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y10	= ptr->bdecont_dither_table_y[4];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y11	= ptr->bdecont_dither_table_y[5];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y12	= ptr->bdecont_dither_table_y[6];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y13	= ptr->bdecont_dither_table_y[7];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y20	= ptr->bdecont_dither_table_y[8];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y21	= ptr->bdecont_dither_table_y[9];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y22	= ptr->bdecont_dither_table_y[10];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y23	= ptr->bdecont_dither_table_y[11];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y30	= ptr->bdecont_dither_table_y[12];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y31	= ptr->bdecont_dither_table_y[13];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y32	= ptr->bdecont_dither_table_y[14];
	blk_de_contour_dither_y_reg.bdecont_dither_table_y33	= ptr->bdecont_dither_table_y[15];

	blk_de_contour_dither_u_reg.bdecont_dither_table_u00	= ptr->bdecont_dither_table_u[0];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u01	= ptr->bdecont_dither_table_u[1];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u02	= ptr->bdecont_dither_table_u[2];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u03	= ptr->bdecont_dither_table_u[3];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u10	= ptr->bdecont_dither_table_u[4];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u11	= ptr->bdecont_dither_table_u[5];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u12	= ptr->bdecont_dither_table_u[6];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u13	= ptr->bdecont_dither_table_u[7];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u20	= ptr->bdecont_dither_table_u[8];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u21	= ptr->bdecont_dither_table_u[9];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u22	= ptr->bdecont_dither_table_u[10];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u23	= ptr->bdecont_dither_table_u[11];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u30	= ptr->bdecont_dither_table_u[12];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u31	= ptr->bdecont_dither_table_u[13];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u32	= ptr->bdecont_dither_table_u[14];
	blk_de_contour_dither_u_reg.bdecont_dither_table_u33	= ptr->bdecont_dither_table_u[15];

	blk_de_contour_dither_v_reg.bdecont_dither_table_v00	= ptr->bdecont_dither_table_v[0];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v01	= ptr->bdecont_dither_table_v[1];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v02	= ptr->bdecont_dither_table_v[2];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v03	= ptr->bdecont_dither_table_v[3];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v10	= ptr->bdecont_dither_table_v[4];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v11	= ptr->bdecont_dither_table_v[5];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v12	= ptr->bdecont_dither_table_v[6];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v13	= ptr->bdecont_dither_table_v[7];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v20	= ptr->bdecont_dither_table_v[8];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v21	= ptr->bdecont_dither_table_v[9];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v22	= ptr->bdecont_dither_table_v[10];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v23	= ptr->bdecont_dither_table_v[11];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v30	= ptr->bdecont_dither_table_v[12];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v31	= ptr->bdecont_dither_table_v[13];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v32	= ptr->bdecont_dither_table_v[14];
	blk_de_contour_dither_v_reg.bdecont_dither_table_v33	= ptr->bdecont_dither_table_v[15];

	IoReg_Write32(IPQ_DECONTOUR_Block_DeContour_dither_y_reg, blk_de_contour_dither_y_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_Block_DeContour_dither_u_reg, blk_de_contour_dither_u_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_Block_DeContour_dither_v_reg, blk_de_contour_dither_v_reg.regValue);

}

void drvif_color_Set_Block_DeContour_Profile_Diff(DRV_Block_DeContour_Profile_Diff *ptr)
{
	ipq_decontour_blk_de_profile_diff_dist_RBUS blk_de_profile_diff_dist_reg;
	ipq_decontour_blk_de_profile_diff_th_RBUS blk_de_profile_diff_th_reg;
	ipq_decontour_blk_de_profile_diff_gain_RBUS blk_de_profile_diff_gain_reg;
	unsigned int slope;

	blk_de_profile_diff_dist_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_profile_diff_dist_reg);
	blk_de_profile_diff_th_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_profile_diff_th_reg);
	blk_de_profile_diff_gain_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_profile_diff_gain_reg);

	blk_de_profile_diff_dist_reg.bdecont_profile_diff_disth = ptr->bdecont_profile_diff_distH;
	blk_de_profile_diff_dist_reg.bdecont_profile_diff_distv = ptr->bdecont_profile_diff_distV;
	blk_de_profile_diff_th_reg.bdecont_profile_diff_th_lo = ptr->bdecont_profile_diff_th_lo;
	blk_de_profile_diff_th_reg.bdecont_profile_diff_th_hi = ptr->bdecont_profile_diff_th_hi;
	blk_de_profile_diff_gain_reg.bdecont_profile_diff_gain_lo = ptr->bdecont_profile_diff_gain_lo;
	blk_de_profile_diff_gain_reg.bdecont_profile_diff_gain_hi = ptr->bdecont_profile_diff_gain_hi;

	// slope = (gain_hi - gain_lo) * 256 / (th_hi - th_lo)
	slope = (ptr->bdecont_profile_diff_gain_hi - ptr->bdecont_profile_diff_gain_lo) * 256 / (ptr->bdecont_profile_diff_th_hi - ptr->bdecont_profile_diff_th_lo);
	blk_de_profile_diff_gain_reg.bdecont_profile_diff_gain_slope = slope;

	IoReg_Write32(IPQ_DECONTOUR_BLK_De_profile_diff_dist_reg, blk_de_profile_diff_dist_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_profile_diff_th_reg, blk_de_profile_diff_th_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_profile_diff_gain_reg, blk_de_profile_diff_gain_reg.regValue);

}

void drvif_color_Set_Block_DeContour_Clamp(DRV_Block_DeContour_Clamp *ptr) // mk2
{
	ipq_decontour_bdecont_ai_0_RBUS bdecont_ai_0_reg;

	bdecont_ai_0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BDECONT_AI_0_reg);

	bdecont_ai_0_reg.bdecont_diff_clamp_y = ptr->bdecont_diff_clamp_y;
	bdecont_ai_0_reg.bdecont_diff_clamp_c = ptr->bdecont_diff_clamp_c;

	IoReg_Write32(IPQ_DECONTOUR_BDECONT_AI_0_reg, bdecont_ai_0_reg.regValue);
}


void drvif_color_Set_Block_DeContour_PQ_Mask(DRV_Block_DeContour_Semantic_Alpha *ptr) // mk2
{
	ipq_decontour_bdecont_ai_0_RBUS bdecont_ai_0_reg;
	ipq_decontour_blk_de_alpha_diff_semantic0_RBUS blk_de_alpha_diff_semantic0_reg;
	ipq_decontour_blk_de_alpha_diff_semantic1_RBUS blk_de_alpha_diff_semantic1_reg;
	ipq_decontour_blk_de_alpha_diff_semantic2_RBUS blk_de_alpha_diff_semantic2_reg;
	ipq_decontour_blk_de_alpha_diff_semantic3_RBUS blk_de_alpha_diff_semantic3_reg;
	ipq_decontour_blk_de_alpha_diff_semantic4_RBUS blk_de_alpha_diff_semantic4_reg;
	ipq_decontour_blk_de_alpha_diff_semantic5_RBUS blk_de_alpha_diff_semantic5_reg;
	ipq_decontour_blk_de_alpha_diff_semantic6_RBUS blk_de_alpha_diff_semantic6_reg;
	ipq_decontour_blk_de_alpha_diff_semantic7_RBUS blk_de_alpha_diff_semantic7_reg;
	ipq_decontour_blk_de_alpha_diff_semantic8_RBUS blk_de_alpha_diff_semantic8_reg;
	ipq_decontour_blk_de_alpha_diff_semantic9_RBUS blk_de_alpha_diff_semantic9_reg;
	ipq_decontour_blk_de_alpha_diff_semantic10_RBUS blk_de_alpha_diff_semantic10_reg;
	ipq_decontour_blk_de_contour_alpha_ydiff4_RBUS blk_de_alpha_ydiff4_reg;	
	ipq_decontour_blk_de_contour_alpha_cdiff4_RBUS blk_de_alpha_cdiff4_reg;	

	bdecont_ai_0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BDECONT_AI_0_reg);
	blk_de_alpha_diff_semantic0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic0_reg);
	blk_de_alpha_diff_semantic1_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic1_reg);
	blk_de_alpha_diff_semantic2_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic2_reg);
	blk_de_alpha_diff_semantic3_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic3_reg);
	blk_de_alpha_diff_semantic4_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic4_reg);
	blk_de_alpha_diff_semantic5_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic5_reg);
	blk_de_alpha_diff_semantic6_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic6_reg);
	blk_de_alpha_diff_semantic7_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic7_reg);
	blk_de_alpha_diff_semantic8_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic8_reg);
	blk_de_alpha_diff_semantic9_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic9_reg);
	blk_de_alpha_diff_semantic10_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic10_reg);
	blk_de_alpha_ydiff4_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff4_reg);
	blk_de_alpha_cdiff4_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff4_reg);
	
	blk_de_alpha_ydiff4_reg.bdecont_alpha_by_ydiff_semantic_hi_0 = ptr->bdecont_alpha_by_ydiff_semantic_hi[0];
	blk_de_alpha_ydiff4_reg.bdecont_alpha_by_ydiff_semantic_hi_1 = ptr->bdecont_alpha_by_ydiff_semantic_hi[1];
	blk_de_alpha_ydiff4_reg.bdecont_alpha_by_ydiff_semantic_hi_2 = ptr->bdecont_alpha_by_ydiff_semantic_hi[2];
	blk_de_alpha_ydiff4_reg.bdecont_alpha_by_ydiff_semantic_hi_3 = ptr->bdecont_alpha_by_ydiff_semantic_hi[3];	
	blk_de_alpha_diff_semantic0_reg.bdecont_alpha_by_ydiff_semantic_hi_4 = ptr->bdecont_alpha_by_ydiff_semantic_hi[4];
	blk_de_alpha_diff_semantic0_reg.bdecont_alpha_by_ydiff_semantic_hi_5 = ptr->bdecont_alpha_by_ydiff_semantic_hi[5];
	blk_de_alpha_diff_semantic0_reg.bdecont_alpha_by_ydiff_semantic_hi_6 = ptr->bdecont_alpha_by_ydiff_semantic_hi[6];
	blk_de_alpha_diff_semantic0_reg.bdecont_alpha_by_ydiff_semantic_hi_7 = ptr->bdecont_alpha_by_ydiff_semantic_hi[7];
	blk_de_alpha_diff_semantic0_reg.bdecont_alpha_by_ydiff_semantic_hi_8 = ptr->bdecont_alpha_by_ydiff_semantic_hi[8];
	blk_de_alpha_diff_semantic1_reg.bdecont_alpha_by_ydiff_semantic_hi_9 = ptr->bdecont_alpha_by_ydiff_semantic_hi[9];
	blk_de_alpha_diff_semantic1_reg.bdecont_alpha_by_ydiff_semantic_hi_10 = ptr->bdecont_alpha_by_ydiff_semantic_hi[10];
	blk_de_alpha_diff_semantic1_reg.bdecont_alpha_by_ydiff_semantic_hi_11 = ptr->bdecont_alpha_by_ydiff_semantic_hi[11];
	blk_de_alpha_diff_semantic1_reg.bdecont_alpha_by_ydiff_semantic_hi_12 = ptr->bdecont_alpha_by_ydiff_semantic_hi[12];
	blk_de_alpha_diff_semantic1_reg.bdecont_alpha_by_ydiff_semantic_hi_13 = ptr->bdecont_alpha_by_ydiff_semantic_hi[13];
	blk_de_alpha_diff_semantic2_reg.bdecont_alpha_by_ydiff_semantic_hi_14 = ptr->bdecont_alpha_by_ydiff_semantic_hi[14];
	blk_de_alpha_diff_semantic2_reg.bdecont_alpha_by_ydiff_semantic_hi_15 = ptr->bdecont_alpha_by_ydiff_semantic_hi[15];

	blk_de_alpha_diff_semantic2_reg.bdecont_alpha_by_ydiff_semantic_lo_0 = ptr->bdecont_alpha_by_ydiff_semantic_lo[0];
	blk_de_alpha_diff_semantic2_reg.bdecont_alpha_by_ydiff_semantic_lo_1 = ptr->bdecont_alpha_by_ydiff_semantic_lo[1];
	blk_de_alpha_diff_semantic2_reg.bdecont_alpha_by_ydiff_semantic_lo_2 = ptr->bdecont_alpha_by_ydiff_semantic_lo[2];
	blk_de_alpha_diff_semantic3_reg.bdecont_alpha_by_ydiff_semantic_lo_3 = ptr->bdecont_alpha_by_ydiff_semantic_lo[3];
	blk_de_alpha_diff_semantic3_reg.bdecont_alpha_by_ydiff_semantic_lo_4 = ptr->bdecont_alpha_by_ydiff_semantic_lo[4];
	blk_de_alpha_diff_semantic3_reg.bdecont_alpha_by_ydiff_semantic_lo_5 = ptr->bdecont_alpha_by_ydiff_semantic_lo[5];
	blk_de_alpha_diff_semantic3_reg.bdecont_alpha_by_ydiff_semantic_lo_6 = ptr->bdecont_alpha_by_ydiff_semantic_lo[6];
	blk_de_alpha_diff_semantic3_reg.bdecont_alpha_by_ydiff_semantic_lo_7 = ptr->bdecont_alpha_by_ydiff_semantic_lo[7];
	blk_de_alpha_diff_semantic4_reg.bdecont_alpha_by_ydiff_semantic_lo_8 = ptr->bdecont_alpha_by_ydiff_semantic_lo[8];
	blk_de_alpha_diff_semantic4_reg.bdecont_alpha_by_ydiff_semantic_lo_9 = ptr->bdecont_alpha_by_ydiff_semantic_lo[9];
	blk_de_alpha_diff_semantic4_reg.bdecont_alpha_by_ydiff_semantic_lo_10 = ptr->bdecont_alpha_by_ydiff_semantic_lo[10];
	blk_de_alpha_diff_semantic4_reg.bdecont_alpha_by_ydiff_semantic_lo_11 = ptr->bdecont_alpha_by_ydiff_semantic_lo[11];
	blk_de_alpha_diff_semantic4_reg.bdecont_alpha_by_ydiff_semantic_lo_12 = ptr->bdecont_alpha_by_ydiff_semantic_lo[12];
	blk_de_alpha_diff_semantic5_reg.bdecont_alpha_by_ydiff_semantic_lo_13 = ptr->bdecont_alpha_by_ydiff_semantic_lo[13];
	blk_de_alpha_diff_semantic5_reg.bdecont_alpha_by_ydiff_semantic_lo_14 = ptr->bdecont_alpha_by_ydiff_semantic_lo[14];
	blk_de_alpha_diff_semantic5_reg.bdecont_alpha_by_ydiff_semantic_lo_15 = ptr->bdecont_alpha_by_ydiff_semantic_lo[15];

	blk_de_alpha_diff_semantic5_reg.bdecont_alpha_by_cdiff_semantic_hi_0 = ptr->bdecont_alpha_by_cdiff_semantic_hi[0];
	blk_de_alpha_diff_semantic5_reg.bdecont_alpha_by_cdiff_semantic_hi_1 = ptr->bdecont_alpha_by_cdiff_semantic_hi[1];
	blk_de_alpha_cdiff4_reg.bdecont_alpha_by_cdiff_semantic_hi_2 = ptr->bdecont_alpha_by_cdiff_semantic_hi[2];
	blk_de_alpha_cdiff4_reg.bdecont_alpha_by_cdiff_semantic_hi_3 = ptr->bdecont_alpha_by_cdiff_semantic_hi[3];
	blk_de_alpha_cdiff4_reg.bdecont_alpha_by_cdiff_semantic_hi_4 = ptr->bdecont_alpha_by_cdiff_semantic_hi[4];
	blk_de_alpha_cdiff4_reg.bdecont_alpha_by_cdiff_semantic_hi_5 = ptr->bdecont_alpha_by_cdiff_semantic_hi[5];
	blk_de_alpha_diff_semantic6_reg.bdecont_alpha_by_cdiff_semantic_hi_6= ptr->bdecont_alpha_by_cdiff_semantic_hi[6];
	blk_de_alpha_diff_semantic6_reg.bdecont_alpha_by_cdiff_semantic_hi_7= ptr->bdecont_alpha_by_cdiff_semantic_hi[7];
	blk_de_alpha_diff_semantic6_reg.bdecont_alpha_by_cdiff_semantic_hi_8= ptr->bdecont_alpha_by_cdiff_semantic_hi[8];
	blk_de_alpha_diff_semantic6_reg.bdecont_alpha_by_cdiff_semantic_hi_9= ptr->bdecont_alpha_by_cdiff_semantic_hi[9];
	blk_de_alpha_diff_semantic6_reg.bdecont_alpha_by_cdiff_semantic_hi_10= ptr->bdecont_alpha_by_cdiff_semantic_hi[10];
	blk_de_alpha_diff_semantic7_reg.bdecont_alpha_by_cdiff_semantic_hi_11= ptr->bdecont_alpha_by_cdiff_semantic_hi[11];
	blk_de_alpha_diff_semantic7_reg.bdecont_alpha_by_cdiff_semantic_hi_12= ptr->bdecont_alpha_by_cdiff_semantic_hi[12];
	blk_de_alpha_diff_semantic7_reg.bdecont_alpha_by_cdiff_semantic_hi_13= ptr->bdecont_alpha_by_cdiff_semantic_hi[13];
	blk_de_alpha_diff_semantic7_reg.bdecont_alpha_by_cdiff_semantic_hi_14= ptr->bdecont_alpha_by_cdiff_semantic_hi[14];
	blk_de_alpha_diff_semantic7_reg.bdecont_alpha_by_cdiff_semantic_hi_15= ptr->bdecont_alpha_by_cdiff_semantic_hi[15];

	blk_de_alpha_diff_semantic8_reg.bdecont_alpha_by_cdiff_semantic_lo_0 = ptr->bdecont_alpha_by_cdiff_semantic_lo[0];
	blk_de_alpha_diff_semantic8_reg.bdecont_alpha_by_cdiff_semantic_lo_1 = ptr->bdecont_alpha_by_cdiff_semantic_lo[1];
	blk_de_alpha_diff_semantic8_reg.bdecont_alpha_by_cdiff_semantic_lo_2 = ptr->bdecont_alpha_by_cdiff_semantic_lo[2];
	blk_de_alpha_diff_semantic8_reg.bdecont_alpha_by_cdiff_semantic_lo_3 = ptr->bdecont_alpha_by_cdiff_semantic_lo[3];
	blk_de_alpha_diff_semantic8_reg.bdecont_alpha_by_cdiff_semantic_lo_4 = ptr->bdecont_alpha_by_cdiff_semantic_lo[4];	
	blk_de_alpha_diff_semantic9_reg.bdecont_alpha_by_cdiff_semantic_lo_5= ptr->bdecont_alpha_by_cdiff_semantic_lo[5];
	blk_de_alpha_diff_semantic9_reg.bdecont_alpha_by_cdiff_semantic_lo_6= ptr->bdecont_alpha_by_cdiff_semantic_lo[6];
	blk_de_alpha_diff_semantic9_reg.bdecont_alpha_by_cdiff_semantic_lo_7= ptr->bdecont_alpha_by_cdiff_semantic_lo[7];
	blk_de_alpha_diff_semantic9_reg.bdecont_alpha_by_cdiff_semantic_lo_8= ptr->bdecont_alpha_by_cdiff_semantic_lo[8];
	blk_de_alpha_diff_semantic9_reg.bdecont_alpha_by_cdiff_semantic_lo_9= ptr->bdecont_alpha_by_cdiff_semantic_lo[9];
	blk_de_alpha_diff_semantic10_reg.bdecont_alpha_by_cdiff_semantic_lo_10= ptr->bdecont_alpha_by_cdiff_semantic_lo[10];
	blk_de_alpha_diff_semantic10_reg.bdecont_alpha_by_cdiff_semantic_lo_11= ptr->bdecont_alpha_by_cdiff_semantic_lo[11];
	blk_de_alpha_diff_semantic10_reg.bdecont_alpha_by_cdiff_semantic_lo_12= ptr->bdecont_alpha_by_cdiff_semantic_lo[12];
	blk_de_alpha_diff_semantic10_reg.bdecont_alpha_by_cdiff_semantic_lo_13= ptr->bdecont_alpha_by_cdiff_semantic_lo[13];
	blk_de_alpha_diff_semantic10_reg.bdecont_alpha_by_cdiff_semantic_lo_14= ptr->bdecont_alpha_by_cdiff_semantic_lo[14];
	bdecont_ai_0_reg.bdecont_alpha_by_cdiff_semantic_lo_15= ptr->bdecont_alpha_by_cdiff_semantic_lo[15];

	IoReg_Write32(IPQ_DECONTOUR_BDECONT_AI_0_reg, bdecont_ai_0_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic0_reg, blk_de_alpha_diff_semantic0_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic1_reg, blk_de_alpha_diff_semantic1_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic2_reg, blk_de_alpha_diff_semantic2_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic3_reg, blk_de_alpha_diff_semantic3_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic4_reg, blk_de_alpha_diff_semantic4_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic5_reg, blk_de_alpha_diff_semantic5_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic6_reg, blk_de_alpha_diff_semantic6_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic7_reg, blk_de_alpha_diff_semantic7_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic8_reg, blk_de_alpha_diff_semantic8_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic9_reg, blk_de_alpha_diff_semantic9_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Alpha_Diff_Semantic10_reg, blk_de_alpha_diff_semantic10_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Ydiff4_reg, blk_de_alpha_ydiff4_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_Alpha_Cdiff4_reg, blk_de_alpha_cdiff4_reg.regValue);

}
void drvif_color_Set_Block_DeContour_MAD(DRV_Block_DeContour_MAD *ptr) // ml7
{
	ipq_decontour_blk_de_contour_ctrl_RBUS blk_de_contour_ctrl_reg;
	ipq_decontour_blk_de_contour_mad_info_0_RBUS blk_de_contour_mad_info_0_reg;
	ipq_decontour_blk_de_contour_mad_info_1_RBUS blk_de_contour_mad_info_1_reg;
	ipq_decontour_blk_de_contour_mad_info_2_RBUS blk_de_contour_mad_info_2_reg;
	ipq_decontour_blk_de_contour_mad_info_3_RBUS blk_de_contour_mad_info_3_reg;

	blk_de_contour_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg);
	blk_de_contour_mad_info_0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_mad_info_0_reg);
	blk_de_contour_mad_info_1_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_mad_info_1_reg);
	blk_de_contour_mad_info_2_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_mad_info_2_reg);
	blk_de_contour_mad_info_3_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_mad_info_3_reg);

	blk_de_contour_ctrl_reg.bdecont_mad_gain_en = ptr->bdecont_mad_gain_en;
	blk_de_contour_ctrl_reg.bdecont_mad_mode = ptr->bdecont_mad_mode;
	blk_de_contour_mad_info_0_reg.bdecont_mad_offset = ptr->bdecont_mad_offset;
	blk_de_contour_mad_info_0_reg.bdecont_mad_shift = ptr->bdecont_mad_shift;
	blk_de_contour_mad_info_0_reg.bdecont_mad_th_hi = ptr->bdecont_mad_th_hi;
	blk_de_contour_mad_info_0_reg.bdecont_mad_th_lo = ptr->bdecont_mad_th_lo;
	blk_de_contour_mad_info_1_reg.bdecont_mad_dn_gain_hi = ptr->bdecont_mad_dn_gain_hi;
	blk_de_contour_mad_info_1_reg.bdecont_mad_dn_gain_lo = ptr->bdecont_mad_dn_gain_lo;
	blk_de_contour_mad_info_1_reg.bdecont_mad_up_gain_hi = ptr->bdecont_mad_up_gain_hi;
	blk_de_contour_mad_info_1_reg.bdecont_mad_up_gain_lo = ptr->bdecont_mad_up_gain_lo;
	blk_de_contour_mad_info_2_reg.bdecont_mad_gain_dn_shift = ptr->bdecont_mad_gain_dn_shift;
	blk_de_contour_mad_info_2_reg.bdecont_mad_gain_up_shift = ptr->bdecont_mad_gain_up_shift;
	blk_de_contour_mad_info_2_reg.bdecont_mad_hi_cnt_th = ptr->bdecont_mad_hi_cnt_th;
	blk_de_contour_mad_info_2_reg.bdecont_mad_lo_cnt_th = ptr->bdecont_mad_lo_cnt_th;
	blk_de_contour_mad_info_3_reg.bdecont_mad_diff_shift_c = ptr->bdecont_mad_diff_shift_c;
	blk_de_contour_mad_info_3_reg.bdecont_mad_diff_shift_y = ptr->bdecont_mad_diff_shift_y;
	blk_de_contour_mad_info_3_reg.bdecont_mad_diff_th_c = ptr->bdecont_mad_diff_th_c;
	blk_de_contour_mad_info_3_reg.bdecont_mad_diff_th_y = ptr->bdecont_mad_diff_th_y;	
	blk_de_contour_ctrl_reg.bdecont_mad_interp_en = ptr->bdecont_mad_interp_en; // ml8_add
	
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg, blk_de_contour_ctrl_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_mad_info_0_reg, blk_de_contour_mad_info_0_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_mad_info_1_reg, blk_de_contour_mad_info_1_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_mad_info_2_reg, blk_de_contour_mad_info_2_reg.regValue);
	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_mad_info_3_reg, blk_de_contour_mad_info_3_reg.regValue);
}

void drvif_color_Set_Block_DeContour_MultiView_En(unsigned char en) // ml8_add
{
	ipq_decontour_blk_de_contour_ctrl_RBUS blk_de_contour_ctrl_reg;
	blk_de_contour_ctrl_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg);

	blk_de_contour_ctrl_reg.bdecont_multiview_en = en;

	IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg, blk_de_contour_ctrl_reg.regValue);
}

void drvif_color_Set_RTNR_H_Rotate(unsigned char rtnr_h_flip_en, unsigned char rtnr_rgb444_en)
{
	di_im_di_control_RBUS im_di_control_reg;
	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;

	im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
	im_di_control_reg.ccdinr_h_flip_en = rtnr_h_flip_en;
	IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);

	im_di_rtnr_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);
	im_di_rtnr_control_reg.cp_rtnr_rgb444_enable = rtnr_rgb444_en;
	IoReg_Write32(DI_IM_DI_RTNR_CONTROL_reg, im_di_rtnr_control_reg.regValue);
}

static VIP_Block_DeContour_DMA_CTRL blkdecont_dma_ctrl = {0};
VIP_Block_DeContour_DMA_CTRL* drvif_fwif_color_Get_Block_DeContour_DMA_Ctrl(void)
{
	return &blkdecont_dma_ctrl;
}

unsigned int  drvif_color_Get_Block_DeContour_flat_ratio(void)// ml8_add
{
	return flat_ratio;
}

unsigned int  drvif_color_Get_Block_DeContour_texture_ratio(void)// ml8_add
{
	return texture_ratio;
}

void drvif_color_Get_Block_DeContour_MAD(void)// ml8_add
{

	unsigned int *virAddr = NULL;
	unsigned int total_blk_num=0, cnt16=0, mad=0, i=0;
	blk_decont_info *pblk_info;
	unsigned int flat_th=100, texture_th=900, flat_cnt=0, texture_cnt=0/*, flat_ratio=0, texture_ratio=0*/;
	VIP_Block_DeContour_DMA_CTRL* dma_ctrl = NULL;
	
	ipq_decontour_blk_de_contour_size0_RBUS blk_de_contour_size0_reg;
	ipq_decontour_blk_de_contour_size1_RBUS blk_de_contour_size1_reg;

	/***********/	
	blk_de_contour_size0_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Size0_reg);
	blk_de_contour_size1_reg.regValue = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_Size1_reg);

	dma_ctrl = drvif_fwif_color_Get_Block_DeContour_DMA_Ctrl();
	virAddr = dma_ctrl->pVir_addr_align;

	total_blk_num = blk_de_contour_size0_reg.bdecont_blk_hnum * blk_de_contour_size1_reg.bdecont_blk_vnum;
	cnt16 = total_blk_num>>4;

	flat_cnt = texture_cnt = 0;
	
	for(i=0; i<cnt16; i++)
	{
		pblk_info = (blk_decont_info *)((virAddr+20*i));

		mad = (pblk_info->m0_h<<2) + pblk_info->m0_l;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m1;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m2;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m3;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = (pblk_info->m4_h<<2)+ pblk_info->m4_l;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m5;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m6;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m7;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = (pblk_info->m8_h<<2)+ pblk_info->m8_l;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m9;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m10;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m11;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = (pblk_info->m12_h<<2)+ pblk_info->m12_l;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m13;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m14;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

		mad = pblk_info->m15;		
		if(mad<flat_th) flat_cnt++;
		else if(mad>texture_th) texture_cnt++;

	}

	if(total_blk_num>0)
	{
		flat_ratio = 100*flat_cnt/total_blk_num;
		texture_ratio = 100*texture_cnt/total_blk_num;
	}
}

void drvif_color_Set_DRV_SNR_Clock(SCALER_DISP_CHANNEL display, unsigned char En_flag)
{
	uint32_t mask = (display == SLR_MAIN_DISPLAY) ?
				VGIP_Data_Path_Select_s1_clk_en_mask :
				VGIP_Data_Path_Select_s2_clk_en_mask;

	if (En_flag)
		IoReg_SetBits(VGIP_Data_Path_Select_reg, mask);
	else
		IoReg_ClearBits(VGIP_Data_Path_Select_reg, mask);
}

unsigned char drvif_color_Get_DRV_SNR_Clock(SCALER_DISP_CHANNEL display, unsigned char flag)
{
	if (SLR_SUB_DISPLAY == display)
		return VGIP_Data_Path_Select_get_s2_clk_en(IoReg_Read32(VGIP_Data_Path_Select_reg));
	else
		return VGIP_Data_Path_Select_get_s1_clk_en(IoReg_Read32(VGIP_Data_Path_Select_reg));
}

void drvif_color_get_DB_NR(short *ptr )
{

        UINT8 i=0,j=0;
	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	iedge_smooth_prelpf_ctrl_RBUS prelpf_ctrl_reg;
	nr_new_lpf_nearby_ctrl_RBUS new_lpf_nearby_ctrl_reg;
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_edge_filter_RBUS nr_edge_filter_reg;
	nr_dch1_spatial_thl_RBUS dch1_spatial_thl_reg;
	nr_dch1_spatial_thl2_RBUS dch1_spatial_thl2_reg;
	nr_dch1_spatial_weight_RBUS dch1_Spatial_Weight_Reg;
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
        di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;

        nr_mosquito_nr_level_ctrl_RBUS nr_mosquito_nr_level_ctrl_RBUS_reg;
        nr_mosquito_nr_level_ctrl_1_RBUS nr_mosquito_nr_level_ctrl_1_RBUS_reg;
        nr_mosquito_nr_keep_edge_ctrl_RBUS  nr_mosquito_nr_keep_edge_ctrl_reg;
        nr_mosquito_nr_decay_ctrl_1_RBUS    nr_mosquito_nr_decay_ctrl_1_reg;
        mpegnr_ich1_mpegnr2_RBUS        mpegnr_ich1_mpegnr2_reg ;


        if(ptr == NULL)
                return;

        dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	prelpf_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_prelpf_ctrl_reg);

	new_lpf_nearby_ctrl_reg.regValue = IoReg_Read32(NR_NEW_LPF_NEARBY_CTRL_reg);
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	nr_edge_filter_reg.regValue = IoReg_Read32(NR_EDGE_FILTER_reg);


	dch1_spatial_thl_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl_reg);
	dch1_spatial_thl2_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl2_reg);
	dch1_Spatial_Weight_Reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Weight_reg);


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
        im_di_rtnr_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);

        nr_mosquito_nr_level_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_reg);
        nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg);
        nr_mosquito_nr_keep_edge_ctrl_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg);
        nr_mosquito_nr_decay_ctrl_1_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg);
	mpegnr_ich1_mpegnr2_reg.regValue = IoReg_Read32(MPEGNR_ICH1_MPEGNR2_reg);

        /*#1 CHIP_LPFY_T 9 +8 */
        ptr[0] = dch1_cp_ctrl_reg.cp_spatialenabley;
        ptr[1] = new_lpf_nearby_ctrl_reg.modified_lpf_edge_gain;
        ptr[2] = new_lpf_nearby_ctrl_reg.modified_lpf_thd_upbnd;
        ptr[3] = new_lpf_nearby_ctrl_reg.modified_lpf_thd_lowbnd;
        ptr[4] = new_lpf_nearby_ctrl_reg.modified_lpf_thd_step;
        ptr[5] = dch1_spatial_thl_reg.cp_spatialthly;
        ptr[6] = dch1_spatial_thl_reg.cp_spatialthlyx2;
        ptr[7] = dch1_spatial_thl2_reg.cp_spatialthlyx4;
        ptr[8] = dch1_cp_ctrl_reg.cp_zoranweighty_sel;


        /* CHIP_LPFC_T 6 +14 */
        ptr[9] = dch1_cp_ctrl_reg.cp_spatialenablec;
        ptr[10] = dch1_Spatial_Weight_Reg.cp_lpresultweightc;
        ptr[11] = dch1_spatial_thl_reg.cp_spatialthlc;
        ptr[12] = dch1_spatial_thl_reg.cp_spatialthlcx2;
        ptr[13] = dch1_spatial_thl2_reg.cp_spatialthlcx4;
        ptr[14] = dch1_cp_ctrl_reg.cp_zoranweightc_sel;



        /* CHIP_NR_CURVEMAPPING_STEP_T 8 +22 */
        ptr[15] = curve_mapping_step_0_reg.curvemapping_step1;
        ptr[16] = curve_mapping_step_0_reg.curvemapping_step2;
        ptr[17] = curve_mapping_step_1_reg.curvemapping_step3;
        ptr[18] = curve_mapping_step_1_reg.curvemapping_step4;
        ptr[19] = curve_mapping_step_2_reg.curvemapping_step5;
        ptr[20] = curve_mapping_step_2_reg.curvemapping_step6;
        ptr[21] = curve_mapping_step_3_reg.curvemapping_step7;
        ptr[22] = curve_mapping_step_3_reg.curvemapping_step8;


        /* CHIP_NR_CURVEMAPPING_WEIGHT_UI_T  18 +40 */
        ptr[23] = curve_mapping_w1_1_reg.curvemapping_w1_0;
        ptr[24] = curve_mapping_w1_1_reg.curvemapping_w1_1;
        ptr[25] = curve_mapping_w1_1_reg.curvemapping_w1_2;
        ptr[26] = curve_mapping_w1_1_reg.curvemapping_w1_3;
        ptr[27] = curve_mapping_w1_2_reg.curvemapping_w1_4;
        ptr[28] = curve_mapping_w1_2_reg.curvemapping_w1_5;
        ptr[29] = curve_mapping_w1_2_reg.curvemapping_w1_6;
        ptr[30] = curve_mapping_w1_2_reg.curvemapping_w1_7;
        ptr[31] = curve_mapping_w1_3_reg.curvemapping_w1_8;

        ptr[32] = curve_mapping_w2_1_reg.curvemapping_w2_0;
        ptr[33] = curve_mapping_w2_1_reg.curvemapping_w2_1;
        ptr[34] = curve_mapping_w2_1_reg.curvemapping_w2_2;
        ptr[35] = curve_mapping_w2_1_reg.curvemapping_w2_3;
        ptr[36] = curve_mapping_w2_2_reg.curvemapping_w2_4;
        ptr[37] = curve_mapping_w2_2_reg.curvemapping_w2_5;
        ptr[38] = curve_mapping_w2_2_reg.curvemapping_w2_6;
        ptr[39] = curve_mapping_w2_2_reg.curvemapping_w2_7;
        ptr[40] = curve_mapping_w2_3_reg.curvemapping_w2_8;

        /* CHIP_BLENDING_T 2 +42 */
        ptr[41] = dch1_Spatial_Weight_Reg.cp_weight1y;
        ptr[42] = dch1_Spatial_Weight_Reg.cp_weight2y;



        /* CHIP_TNRY_T 151 +193*/
        ptr[43] = im_di_rtnr_control_reg.cp_rtnr_y_enable;

        for(i=0;i<15;i++)
        {
                for(j=0;j<10;j++)
                        ptr[44+(i*10+j)]= TNR_Y_Store_Mad[i][j];
        }

        /* CHIP_TNRY_T 151 +344*/

        ptr[194]= im_di_rtnr_control_reg.cp_rtnr_c_enable;
        for(i=0;i<7;i++)
        {
                for(j=0;j<10;j++)
                        ptr[195+(i*10+j)]= TNR_C_Store_Mad[i][j];
        }

        /* CHIP_MNR_T 19  +363*/

        ptr[265] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.edge_lpf_en;
        //ptr[266] = nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd;//merlin5 remove
        //ptr[267] = nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd_step;//merlin5 remove
        ptr[268] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.farvar_faredge_refine_en;
        ptr[269] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_half_range;
        ptr[270] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_gain;
        ptr[271] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_offset;
        ptr[272] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_shift;
        //ptr[273] = nr_mosquito_nr_level_ctrl_RBUS_reg.mnr_vardiff_refine_gain;//merlin5 remove
        ptr[274] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.vdiff_frac_shift;
        ptr[275] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_en;
        ptr[276] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_start;
        ptr[277] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_step;
        ptr[278] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_start;
        ptr[279] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_step;
        ptr[280] = nr_mosquito_ctrl_RBUS_reg.mosquitonr_weight;
        ptr[281] = nr_mosquito_ctrl_RBUS_reg.mosquitonr_blending_mode;
        ptr[282] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.blendsnr_byedge_en;
        ptr[283] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_allcomp_en;

        /* CHIP_MNR_UI_T 9  +372*/
        ptr[284] = nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en;
        ptr[285] = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_offset;
        ptr[286] = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_shift;
        ptr[287] = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_offset;
        ptr[288] = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_shift;
        ptr[289] = nr_mosquito_ctrl_RBUS_reg.mosquitonr_edge_strength_gain;
        ptr[290] = nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd;
        ptr[291] = nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd_step;
        ptr[292] = nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedge_shift;




        /* CHIP_MPEGNR_UI_T 2 +374*/
        ptr[293] = mpegnr_ich1_mpegnr2_reg.cp_mpegresultweight_x ;
        ptr[294] = 0 ;

#if 0
        /* CHIP_LPFY_T */
        ptr1->LPF_Y.Spatial_Enable_Y = dch1_cp_ctrl_reg.cp_spatialenabley;
        ptr1->LPF_Y.Gain = new_lpf_nearby_ctrl_reg.modified_lpf_edge_gain;
        ptr1->LPF_Y.Thd_UpBnd = new_lpf_nearby_ctrl_reg.modified_lpf_thd_upbnd;
        ptr1->LPF_Y.Thd_LowBnd = new_lpf_nearby_ctrl_reg.modified_lpf_thd_lowbnd
        ptr1->LPF_Y.Thd_Step = new_lpf_nearby_ctrl_reg.modified_lpf_thd_step
        ptr1->LPF_Y.SpatialThlY = dch1_spatial_thl_reg.cp_spatialthly;
        ptr1->LPF_Y.SpatialThl2Y = dch1_spatial_thl_reg.cp_spatialthlyx2;
        ptr1->LPF_Y.SpatialThl4Y = dch1_spatial_thl2_reg.cp_spatialthlyx4;
        ptr1->LPF_Y.WeightYSel = dch1_cp_ctrl_reg.cp_zoranweighty_sel;


        /* CHIP_LPFC_T*/
        ptr1->LPF_C.Spatial_Enable_C = dch1_cp_ctrl_reg.cp_spatialenablec;
        ptr1->LPF_C.LPResultWeightC = dch1_Spatial_Weight_Reg.cp_lpresultweightc;
        ptr1->LPF_C.SpatialThlC = dch1_spatial_thl_reg.cp_spatialthlc;
        ptr1->LPF_C.SpatialThl2C = dch1_spatial_thl_reg.cp_spatialthlcx2;
        ptr1->LPF_C.SpatialThl4C = dch1_spatial_thl2_reg.cp_spatialthlcx4;
        ptr1->LPF_C.WeightCSel = dch1_cp_ctrl_reg.cp_zoranweightc_sel;

        /* CHIP_NR_CURVEMAPPING_STEP_T */
        ptr1->CurveMappingStep.Curve_step1 = curve_mapping_step_0_reg.curvemapping_step1;
        ptr1->CurveMappingStep.Curve_step2 = curve_mapping_step_0_reg.curvemapping_step2;
        ptr1->CurveMappingStep.Curve_step3 = curve_mapping_step_1_reg.curvemapping_step3;
        ptr1->CurveMappingStep.Curve_step4 = curve_mapping_step_1_reg.curvemapping_step4;
        ptr1->CurveMappingStep.Curve_step5 = curve_mapping_step_2_reg.curvemapping_step5;
        ptr1->CurveMappingStep.Curve_step6 = curve_mapping_step_2_reg.curvemapping_step6;
        ptr1->CurveMappingStep.Curve_step7 = curve_mapping_step_3_reg.curvemapping_step7;
        ptr1->CurveMappingStep.Curve_step8 = curve_mapping_step_3_reg.curvemapping_step8;


        /* CHIP_NR_CURVEMAPPING_WEIGHT_UI_T */
        ptr1->CurveMappingWeight.Weight1_0 = curve_mapping_w1_1_reg.curvemapping_w1_0;
        ptr1->CurveMappingWeight.Weight1_1 = curve_mapping_w1_1_reg.curvemapping_w1_1;
        ptr1->CurveMappingWeight.Weight1_2 = curve_mapping_w1_1_reg.curvemapping_w1_2;
        ptr1->CurveMappingWeight.Weight1_3 = curve_mapping_w1_1_reg.curvemapping_w1_3;
        ptr1->CurveMappingWeight.Weight1_4 = curve_mapping_w1_2_reg.curvemapping_w1_4;
        ptr1->CurveMappingWeight.Weight1_5 = curve_mapping_w1_2_reg.curvemapping_w1_5;
        ptr1->CurveMappingWeight.Weight1_6 = curve_mapping_w1_2_reg.curvemapping_w1_6;
        ptr1->CurveMappingWeight.Weight1_7 = curve_mapping_w1_2_reg.curvemapping_w1_7;
        ptr1->CurveMappingWeight.Weight1_8 = curve_mapping_w1_3_reg.curvemapping_w1_8;

        ptr1->CurveMappingWeight.Weight2_0 = curve_mapping_w2_1_reg.curvemapping_w2_0;
        ptr1->CurveMappingWeight.Weight2_1 = curve_mapping_w2_1_reg.curvemapping_w2_1;
        ptr1->CurveMappingWeight.Weight2_2 = curve_mapping_w2_1_reg.curvemapping_w2_2;
        ptr1->CurveMappingWeight.Weight2_3 = curve_mapping_w2_1_reg.curvemapping_w2_3;
        ptr1->CurveMappingWeight.Weight2_4 = curve_mapping_w2_2_reg.curvemapping_w2_4;
        ptr1->CurveMappingWeight.Weight2_5 = curve_mapping_w2_2_reg.curvemapping_w2_5;
        ptr1->CurveMappingWeight.Weight2_6 = curve_mapping_w2_2_reg.curvemapping_w2_6;
        ptr1->CurveMappingWeight.Weight2_7 = curve_mapping_w2_2_reg.curvemapping_w2_7;
        ptr1->CurveMappingWeight.Weight2_8 = curve_mapping_w2_3_reg.curvemapping_w2_8;

        /* CHIP_BLENDING_T */
        ptr1->BLENDING.Weight1Y = dch1_Spatial_Weight_Reg.cp_weight1y
        ptr1->BLENDING.Weight2Y = dch1_Spatial_Weight_Reg.cp_weight2y



	pNR_Table = fwif_color_Get_NR_Table_tv006();
	if (pNR_Table == NULL) {
		VIPprintf("~get NR_Table_tv006 Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

        /* CHIP_TNRY_T */
        ptr1->TNR_Y.Y_enable = im_di_rtnr_control_reg.cp_rtnr_y_enable;

        for(i=0;i<15;i++)
        {
                for(j=0;j<10;j++)
                        ptr1->TNR_Y.Mad[i][j]= pNR_Table->TNR_Y.Mad[i][j];
        }

        ptr1->TNR_C.C_enable= im_di_rtnr_control_reg.cp_rtnr_c_enable;
        for(i=0;i<15;i++)
        {
                for(j=0;j<10;j++)
                        ptr1->TNR_C.Mad[i][j]= pNR_Table->TNR_C.Mad[i][j];
        }


        ptr2->MNR_T.Edge_LPF_enable = nr_mosquito_nr_level_ctrl_1_RBUS_reg.edge_lpf_en;
        ptr2->MNR_T.Edge_Theshold = nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd;
        ptr2->MNR_T.Edge_Theshold_Step = nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd_step;
        ptr2->MNR_T.Far_Var_farEdge_refine_Enable = nr_mosquito_nr_level_ctrl_1_RBUS_reg.farvar_faredge_refine_en
        ptr2->MNR_T.Range = nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_half_range;
        ptr2->MNR_T.Far_Var_Gain = nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_gain;
        ptr2->MNR_T.Far_Var_offset = nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_offset;
        ptr2->MNR_T.Hdiff_Frac_Shift = nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_shift;
        ptr2->MNR_T.varDiff_refine_gain = nr_mosquito_nr_level_ctrl_RBUS_reg.mnr_vardiff_refine_gain;
        ptr2->MNR_T.Vdiff_Frac_Shift = nr_mosquito_nr_level_ctrl_1_RBUS_reg.vdiff_frac_shift;
        ptr2->MNR_T.Decay_enable = nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_en;
        ptr2->MNR_T.Hdecay_Start = nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_start;
        ptr2->MNR_T.Hdecay_Step = nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_step;
        ptr2->MNR_T.Vdecay_Start = nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_start;
        ptr2->MNR_T.Vdecay_Step = nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_step;
        ptr2->MNR_T.MNR_Weight = nr_mosquito_ctrl_RBUS_reg.mosquitonr_weight;
        ptr2->MNR_T.Blending_Mode = nr_mosquito_ctrl_RBUS_reg.mosquitonr_blending_mode;
        ptr2->MNR_T.BlendByEdge_enable = nr_mosquito_nr_level_ctrl_1_RBUS_reg.blendsnr_byedge_en;
        ptr2->MNR_T.Decay_all_enable = nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_allcomp_en;

        ptr2->MNR_UI_T.MosquitoNR_Enable = nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en;
        ptr2->MNR_UI_T.Positive_nLevel_Offset = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_offset
        ptr2->MNR_UI_T.Positive_nLevel_Shift = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_shift
        ptr2->MNR_UI_T.Negative_nLevel_Offset = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_offset
        ptr2->MNR_UI_T.Negative_nLevel_Shift = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_shift
        ptr2->MNR_UI_T.Edge_Strength_Gain = nr_mosquito_ctrl_RBUS_reg.mosquitonr_edge_strength_gain
        ptr2->MNR_UI_T.Threshold = nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd
        ptr2->MNR_UI_T.Threshold_Step = nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd_step
        ptr2->MNR_UI_T.Shift = nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedge_shift

        ptr2->MPEGNR_UI_T.MpegResultWeight_H =mpegnr_ich1_mpegnr2_reg.cp_mpegresultweight_x ;
        ptr2->MPEGNR_UI_T.MpegUpperbound_client =0 ;

#endif
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
	di_ccdinr_timing_monitor_ctrl_reg.timing_monitor_i2rnd_mode = 0;
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
	di_ccdinr_timing_monitor_ctrl_1_reg.timing_monitor_i2rnd_mode_1 = 0;
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

unsigned char drvif_color_Get_DI_field_confuse(void)
{
	if (DI_IM_DI_ACTIVE_WINDOW_CONTROL_get_singlefieldintra(IoReg_Read32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg)) == 0)
		return 2;
	else
		return DI_IM_DI_DEBUG_MODE_get_field_confuse(IoReg_Read32(DI_IM_DI_DEBUG_MODE_reg));
}

unsigned char drvif_color_Get_RTNR_progressive_input(void)
{
	return DI_IM_DI_RTNR_CONTROL_get_cp_rtnr_progressive(IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg));
}

void two_pixel_mode_edge_smooth_update(void)
{//two pixel mode edge smooth setting force update
	iedge_smooth_edsm_db_ctrl_RBUS edsm_db_ctrl_reg;
	unsigned char db_seeting;
	edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	db_seeting = edsm_db_ctrl_reg.edsm_db_en;
	edsm_db_ctrl_reg.edsm_db_en = 0;
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);//force update

	edsm_db_ctrl_reg.edsm_db_en = db_seeting;
	IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
}

void drvif_color_Set_VE_Only_Sel(unsigned char ve_only_sel)
{
	di_di_di_sram_self_test_RBUS di_di_di_sram_self_test_reg;

	di_di_di_sram_self_test_reg.regValue = IoReg_Read32(DI_DI_DI_SRAM_SELF_TEST_reg);

	di_di_di_sram_self_test_reg.ve_only_sel = ve_only_sel;
	
	IoReg_Write32(DI_DI_DI_SRAM_SELF_TEST_reg, di_di_di_sram_self_test_reg.regValue);
	
}

void drvif_color_set_iEdgeSmooth_linemode_disable(unsigned char disable_flag, unsigned char skipDB)
{
	iedge_smooth_edgesmooth_ex_ctrl_RBUS edgesmooth_ex_ctrl_reg;
	iedge_smooth_edsm_db_ctrl_RBUS edsm_db_ctrl_reg;
	
	edgesmooth_ex_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg);
	//edgesmooth_ex_ctrl_reg.lbuf_disable = disable_flag;
	edgesmooth_ex_ctrl_reg.lbuf_linemode = (disable_flag == 1)?(0):((Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(iedgeSmooth_line_mode));
	
	IoReg_Write32(IEDGE_SMOOTH_EdgeSmooth_EX_CTRL_reg, edgesmooth_ex_ctrl_reg.regValue);

	edsm_db_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg);
	if ((skipDB == 1) && (edsm_db_ctrl_reg.edsm_db_en == 1)) {
		edsm_db_ctrl_reg.edsm_db_en = 0;
		IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
		edsm_db_ctrl_reg.edsm_db_en = 1;
		IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
	} else {
		edsm_db_ctrl_reg.edsm_db_apply = 1;
		IoReg_Write32(IEDGE_SMOOTH_EDSM_DB_CTRL_reg, edsm_db_ctrl_reg.regValue);
	}
}

DRV_SNR_Mosquito_NR mnr_demo = {9, 1, 1, 1, 1, 0, 0, 0, 4, 1, 2, 1, 2, 0, 7, 73, 0, 7, 7, 50, 1, 0, 75, 0, 75, 1, 0, 1, 1, 7, 1, 0, 0, 0, 0, 0, 0, 1, 255, 255, 255, 255, 255, 255, 255, 255, 7, 7, 0, 75, 1};
DRV_SNR_Modified_LPF mnr_lpf_demo ={3, 4, 619, 0, 4, 5, 3, 0, };
void drvif_color_DRV_SNR_Mosquito_NR_DEMO(void)
{
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_RBUS nr_mosquito_nr_level_ctrl_RBUS_reg;
	nr_mosquito_nr_level_ctrl_1_RBUS nr_mosquito_nr_level_ctrl_1_RBUS_reg;
    nr_mosquito_nr_keep_edge_ctrl_RBUS 	nr_mosquito_nr_keep_edge_ctrl_reg;
    nr_mosquito_nr_decay_ctrl_1_RBUS   	nr_mosquito_nr_decay_ctrl_1_reg;
	//nr_mosquito_nr_edge_thd_ctrl_RBUS nr_mosquito_nr_edge_thd_ctrl_reg;//h5x add
	//nr_mosquito_nr_edge_thd_ctrl_1_RBUS nr_mosquito_nr_edge_thd_ctrl_1_reg;//h5x add
	nr_mosquito_nr_oneside_ctrl_RBUS nr_mosquito_nr_oneside_ctrl_reg;//merlin5 add
	nr_mosquito_nr_oneside_ctrl_2_RBUS nr_mosquito_nr_oneside_ctrl_2_reg;//merlin5 add

	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	nr_mosquito_nr_level_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_reg);
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg);
	nr_mosquito_nr_keep_edge_ctrl_reg.regValue 		= IoReg_Read32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg);
	nr_mosquito_nr_decay_ctrl_1_reg.regValue		= IoReg_Read32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg);
	//nr_mosquito_nr_edge_thd_ctrl_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_EDGE_THD_CTRL_reg);//h5x add
	//nr_mosquito_nr_edge_thd_ctrl_1_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_EDGE_THD_CTRL_1_reg);//h5x add
	nr_mosquito_nr_oneside_ctrl_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_ONESIDE_CTRL_reg);//h5x add
	nr_mosquito_nr_oneside_ctrl_2_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_ONESIDE_CTRL_2_reg);//h5x add
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
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.mnr_xrange_ext = mnr_demo.MNR_xrange_ext;
	/*Mosquito_NR_Edge Condition*/
	#if 0
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd1					= mnr_demo.SNR_mnr_edgethd1;//h5x add
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd2					= mnr_demo.SNR_mnr_edgethd2;
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd3					= mnr_demo.SNR_mnr_edgethd3;
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd4					= mnr_demo.SNR_mnr_edgethd4;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd5					= mnr_demo.SNR_mnr_edgethd5;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd6					= mnr_demo.SNR_mnr_edgethd6;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd7					= mnr_demo.SNR_mnr_edgethd7;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd8					= mnr_demo.SNR_mnr_edgethd8;
	#endif
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
    //nr_mosquito_nr_level_ctrl_RBUS_reg.mnr_vardiff_refine_gain 		= mnr_demo.SNR_vardiff_refine_gain;//merlin5 remove
	nr_mosquito_nr_oneside_ctrl_2_reg.mnr_vardiff_refine_gain_negative = mnr_demo.SNR_vardiff_refine_gain_negative;//merlin5 add
	nr_mosquito_nr_oneside_ctrl_2_reg.mnr_vardiff_refine_gain_positive = mnr_demo.SNR_vardiff_refine_gain_positive;//merlin5 add
	nr_mosquito_nr_oneside_ctrl_2_reg.mnr_var_diff_th = mnr_demo.SNR_vardiff_th;//merlin5 add
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_shift			= mnr_demo.SNR_hdiff_frac_shift;
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.vdiff_frac_shift			= mnr_demo.SNR_vdiff_frac_shift;
    nr_mosquito_nr_level_ctrl_RBUS_reg.mnr_nlevel_maxfrac_mode 		= mnr_demo.SNR_mnr_nlevel_maxfrac_mode; // Merlin4 add to driver

	// Merlin4 add
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.nlevel_diff_factor_en		= mnr_demo.SNR_nlevel_diff_factor_en;
    //nr_mosquito_nr_level_ctrl_1_RBUS_reg.mnr_all_var2d_en			= mnr_demo.SNR_mnr_all_var2d_en;//merlin5 remove
    nr_mosquito_nr_level_ctrl_1_RBUS_reg.blendsnr_byedge_en			= mnr_demo.SNR_blendsnr_byedge_en;
	// h5x add
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.mnr_shp_en					= mnr_demo.SNR_mnr_shp_en;
	nr_mosquito_nr_level_ctrl_1_RBUS_reg.mnr_var2d_byedge			= mnr_demo.SNR_mnr_var2d_byedge;

	/*keep edge*/
	nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd				= mnr_demo.SNR_keep_edge_threshold;
    nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd_step			= mnr_demo.SNR_keep_edge_threshold_step;
    nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedge_shift			= mnr_demo.SNR_keep_edge_shift;

	/*Mosquito_NR_Final MNR Level Adjust*/
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_offset = mnr_demo.SNR_mosquitonr_nlevel_positive_offset;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_shift = mnr_demo.SNR_mosquitonr_nlevel_positive_shift;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_offset = mnr_demo.SNR_mosquitonr_nlevel_negative_offset;
	nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_shift = mnr_demo.SNR_mosquitonr_nlevel_negative_shift;
	nr_mosquito_nr_oneside_ctrl_reg.mnr_nlevel_mid_offset = mnr_demo.SNR_mosquitonr_nlevel_mid_offset;//merlin5 add
	nr_mosquito_nr_oneside_ctrl_reg.mnr_nlevel_mid_shift = mnr_demo.SNR_mosquitonr_nlevel_mid_shift;//merlin5 add

	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_reg, nr_mosquito_nr_level_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg, nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg, nr_mosquito_nr_keep_edge_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg, nr_mosquito_nr_decay_ctrl_1_reg.regValue);
	//IoReg_Write32(NR_MOSQUITO_NR_EDGE_THD_CTRL_reg, nr_mosquito_nr_edge_thd_ctrl_reg.regValue);
	//IoReg_Write32(NR_MOSQUITO_NR_EDGE_THD_CTRL_1_reg, nr_mosquito_nr_edge_thd_ctrl_1_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_ONESIDE_CTRL_reg, nr_mosquito_nr_oneside_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_ONESIDE_CTRL_2_reg, nr_mosquito_nr_oneside_ctrl_2_reg.regValue);
}

void drvif_color_DRV_SNR_Modified_LPF_DEMO(void)
{

	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	nr_new_lpf_nearby_ctrl_RBUS new_lpf_nearby_ctrl_reg;
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;
	nr_edge_filter_RBUS nr_edge_filter_reg;
	dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	new_lpf_nearby_ctrl_reg.regValue = IoReg_Read32(NR_NEW_LPF_NEARBY_CTRL_reg);
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	nr_edge_filter_reg.regValue = IoReg_Read32(NR_EDGE_FILTER_reg);

	/*modified lpf*/
	dch1_cp_ctrl_reg.vc_snr_lpf_sel = mnr_lpf_demo.SNR_LPF_sel; // 0:3, 1:5, 2:mod3, 3:mod5, 4:11, 5:mod11 (Merlin, jyyang@20150615)(Merlin2, jyyang@20160617)
	new_lpf_nearby_ctrl_reg.modified_lpf_edge_gain	= mnr_lpf_demo.SNR_modified_lpf_edge_gain; /*Mac3, leonard wu@20140722*/
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_upbnd	= mnr_lpf_demo.SNR_modified_lpf_thd_upbnd;
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_lowbnd = mnr_lpf_demo.SNR_modified_lpf_thd_lowbnd;
	new_lpf_nearby_ctrl_reg.modified_lpf_thd_step 	= mnr_lpf_demo.SNR_modified_lpf_thd_step;
	new_lpf_nearby_ctrl_reg.modified_lpf_edge_en	= mnr_lpf_demo.SNR_modified_lpf_edge_en; // Merlin4 add

	//Merlin2, jyyang@20160617
	nr_mosquito_ctrl_RBUS_reg.mnr_lpf_sel = mnr_lpf_demo.MNR_LPF_sel;
	nr_edge_filter_reg.vc_edge_lpf_sel = mnr_lpf_demo.Edge_LPF_sel;

	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_Write32(NR_NEW_LPF_NEARBY_CTRL_reg, new_lpf_nearby_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	IoReg_Write32(NR_EDGE_FILTER_reg, nr_edge_filter_reg.regValue);
}

void drvif_color_DRV_SNR_Mosquito_NR_reset_DEMO(void)
{
	nr_mosquito_nr_edge_thd_ctrl_RBUS nr_mosquito_nr_edge_thd_ctrl_reg;//h5x add
	nr_mosquito_nr_edge_thd_ctrl_1_RBUS nr_mosquito_nr_edge_thd_ctrl_1_reg;//h5x add

	nr_mosquito_nr_edge_thd_ctrl_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_EDGE_THD_CTRL_reg);//h5x add
	nr_mosquito_nr_edge_thd_ctrl_1_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_EDGE_THD_CTRL_1_reg);//h5x add

	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd1					= mnr_demo.SNR_mnr_edgethd1;//h5x add
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd2					= mnr_demo.SNR_mnr_edgethd2;
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd3					= mnr_demo.SNR_mnr_edgethd3;
	nr_mosquito_nr_edge_thd_ctrl_1_reg.mnr_edgethd4					= mnr_demo.SNR_mnr_edgethd4;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd5					= mnr_demo.SNR_mnr_edgethd5;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd6					= mnr_demo.SNR_mnr_edgethd6;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd7					= mnr_demo.SNR_mnr_edgethd7;
	nr_mosquito_nr_edge_thd_ctrl_reg.mnr_edgethd8					= mnr_demo.SNR_mnr_edgethd8;

	IoReg_Write32(NR_MOSQUITO_NR_EDGE_THD_CTRL_reg, nr_mosquito_nr_edge_thd_ctrl_reg.regValue);
	IoReg_Write32(NR_MOSQUITO_NR_EDGE_THD_CTRL_1_reg, nr_mosquito_nr_edge_thd_ctrl_1_reg.regValue);
}
