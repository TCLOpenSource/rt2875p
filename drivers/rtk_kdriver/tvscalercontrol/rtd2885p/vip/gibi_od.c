/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2009
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version 	$Revision$
 */

/**
 * @addtogroup color
 * @{
 */

/*============================ Module dependency  ===========================*/
#ifndef BUILD_QUICK_SHOW
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <linux/auth.h>
#include <linux/semaphore.h>//for semaphore
#include <linux/version.h>
#else
#include <no_os/slab.h>
#include <include/string.h>
#include <timer.h>
#include <sysdefs.h>
#include <no_os/printk.h>
#include <no_os/semaphore.h>
#include <no_os/spinlock.h>
#include <no_os/spinlock_types.h>
#include <malloc.h>
#include <div64.h>
#include <rtd_log/rtd_module_log.h>
#include <no_os/pageremap.h>
#include <no_os/math64.h>
#include <mach/rtk_platform.h>
//#include <rtk_kdriver/rmm/rmm.h>

#endif

#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <rtd_log/rtd_module_log.h>
#include <rbus/lgd_dither_reg.h>
#include <rbus/pcid_reg.h>
#include <rbus/od_dma_reg.h>
#include "rtk_vip_logger.h"

/*===================================  Types ================================*/

/*================================== Variables ==============================*/
unsigned char bODMemCheck = 0;
DRV_od_dma oddma_g[] = {
	/*	1st structure -> DRV_od_dma_ctrl ->*/
	/*		{vact, hact, dma_mode, bit_sel, pixel_encoding,*/
	/* 		2p_enable, dma_clk}*/
	
	/*	2nd structure -> DRV_od_dma_pair ->*/
	/*		{cap_en, start_addr1, start_addr2, up_limit, low_limit,*/
	/*		line_step, line_num, burst_length, line128_num, max_outstanding,*/
	/*		rolling_en, rolling_space}*/
	
	/*	3rd structure -> DRV_od_dma_comp ->*/
	/*		{bypass_en, compensation_en, fisrt_line_more_en, guarantee_max_qp_en, first_predict_nc_mode,*/
	/*		variation_maxmin_en, pqdc_saturation_en, g_ratio, block_limit_bit, line_limit_bit,*/
	/*		frame_limit_bit, line_more_bit}*/
	
	/*	4th structure -> DRV_od_dma_pqc_pair ->*/
	/*		{comp_en, line_mode, width_div32, height, data_format,*/
	/*		data_bitwidth, data_color, line_sum_bit, old_400_mode, two_line_predict,*/
	/*		qp_mode, dic_mode, jump4_en}*/
	
	/*	5th structure -> DRV_od_dma_pqc_alloc ->*/
	/*		{dynamic_allocate_ratio_max, dynamic_allocate_ratio_min, dynamic_allocate_line, dynamic_allocate_less, dynamic_allocate_more}*/
	
	/*	6th structure -> DRV_od_dma_pqc_balance ->*/
	/*		{balance_rb_ov_th, balance_rb_ud_th, balance_rb_give, balance_g_ov_th, balance_g_ud_th,*/
	/*		balance_g_give}*/

	{
		// 4k 60, line mode, PQC, Y 4bit
		{3840, 2160, 0, 0, 0, 0, 0},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
		{0, 1, 0, 0, 0, 1, 0, 0, 0x3f, 0x4, 0x4, 0x8},
		{1, 1, 120, 2160, 3, 0, 1, 0, 0, 0, 0, 0, 0},
		{0, 0, 4, 2, 2},
		{0, 3, 3, 0, 3, 3},
	},
};

DRV_od_ctrl od_ctrl_g[] = {
	/*	1st structure -> DRV_od_ymode ->*/
	/*		{enable, coef_r, coef_g, coef_b}*/
	
	/*	2nd structure -> DRV_od_scaling ->*/
	/*		{enable, edge_mode, coeff0, coeff1, coeff2, coeff3, coeff4 */
	/*       r_diff_nr_th, g_diff_nr_th, b_diff_nr_th} */

	/*	3rd structure -> DRV_od_localgain ->*/
	/*		{enable, weight, nr_thd, edge_hhf_thd}*/

	/*	4th structure -> DRV_od_nr ->*/
	/*		{r_en, g_en, b_en, mode, r_th, g_th, b_th}*/
	/*		{rec_r_en, rec_g_en, rec_b_en, mode, rec_r_th, rec_g_th, rec_b_th}*/
	
	/*	5th structure -> od_gain ->*/
	/*		{r_gain, g_gain, b_gain}*/
	/*		{rec_r_gain, rec_g_gain, rec_b_gain}*/
	
	{ // y mode
		{1, 0x4D, 0x96, 0x1D},
		{0, 0, 32, 24, 16, 8, 0, 3, 3, 3},
		{0, 7, 3, 3},
		{{1, 1, 1, 1, 3, 3, 3}, {1, 1, 1, 1, 3, 3, 3},},
		{{60, 60, 60}, {60, 60, 60},},
	},
	{ 	// rgb mode
		{0, 0x4D, 0x96, 0x1D},
		{0, 0, 32, 24, 16, 8, 0, 3, 3, 3},
		{1, 7, 3, 3},
		{{1, 1, 1, 1, 3, 3, 3}, {1, 1, 1, 1, 3, 3, 3},},
		{{60, 60, 60}, {60, 60, 60},},
	},
};

DRV_OD_VerGain od_vergain_g[] = {
	// normal vergain LUT
	{
		1, 17476 /* (32/1920)<<20 */ , 100, // en, factor, offset
		{
			8  ,  16 ,  24 ,  32 ,  40 ,  48 ,  56 ,  64 ,  72 ,  80 ,
			88 ,  96 , 104 , 112 , 120 , 128 , 136 , 144 , 152 , 160 ,
			168 , 176 , 184 , 192 , 200 , 208 , 216 , 224 , 232 , 240 ,
			248 , 255,
		},
	},
	// recursive vergain LUT
	{
		1, 17476 /* (32/1920)<<20 */ , 100, // en, factor, offset
		{
			8  ,  16 ,  24 ,  32 ,  40 ,  48 ,  56 ,  64 ,  72 ,  80 ,
			88 ,  96 , 104 , 112 , 120 , 128 , 136 , 144 , 152 , 160 ,
			168 , 176 , 184 , 192 , 200 , 208 , 216 , 224 , 232 , 240 ,
			248 , 255,
		},
	},
};
 
static DRV_od_lut_ctrl drv_od_lut = {{0}, {0}, {{0}, {0}, {0}}};
static DRV_od_lut_ctrl drv_od_rec_lut = {{0}, {0}, {{0}, {0}, {0}}};	

unsigned char od_table1[OD_table_length] = {0}; // for VRR application using
unsigned char od_table2[OD_table_length] = {0};
unsigned char od_table3[OD_table_length] = {0};
unsigned char od_delta_table_g[OD_table_length]; // for resume to restore od table
unsigned char od_target_table_g[OD_table_length];

/*================================ Definitions ==============================*/
#define TAG_NAME "VPQ_OD"
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_GIBI_OD_DEBUG,fmt,##args)

#define TAG_OD_MEM "OD_MEM"

/*================================== Functions ===============================*/
extern unsigned int Get_DISPLAY_REFRESH_RATE(void);
unsigned int drvifget_align_value(unsigned int input, unsigned int align_value)
{
	if(align_value != 0){
		return (input + (align_value - (input % align_value)));
	}
	else{
		return input;
	}
}
unsigned char drvif_color_od_enable_get(void)
{
	od_od_ctrl_RBUS od_od_ctrl_reg;
	od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	//rtd_printk(KERN_EMERG, TAG_NAME, "get od_od_ctrl_reg.od_en = %d\n", od_od_ctrl_reg.od_en);
	return od_od_ctrl_reg.od_en;
}
void drvif_color_od_enable_set(unsigned char od_enable)
{
	int timeoutcnt = 0x032500;
	od_od_ctrl_RBUS od_od_ctrl_reg;
	
	od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	
	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, od_enable force 0,\n");
		od_enable = 0;
	}

	/* OD function db on */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
	
	/* OD function db read sel: reg */
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25
	
	/* OD enable */
	od_od_ctrl_reg.od_en = od_enable;
	IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl_reg.regValue);
	
	/* db apply */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26 
	
	/* check */
	while(drvif_color_od_enable_get() != od_enable && (timeoutcnt--) > 0) {
		//rtd_printk(KERN_EMERG, TAG_NAME, "%s, wait db apply od_enable=%d, timeout cnt=%d", __func__, od_enable, timeoutcnt);
	}
	if(timeoutcnt == 0)	{
		od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
		rtd_pr_vpq_emerg("%s, time out!, timeout cnt = %d, od_ctrl_reg.regValue=%x,\n", __func__, timeoutcnt, od_od_ctrl_reg.od_en);
	}
	else {
		rtd_pr_vpq_info("%s, timeout cnt = %d, set od_enable %d done\n", __func__, timeoutcnt, od_enable);
	}

	/* OD function db off */
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
	
	/* OD function db read sel: temp */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25
}
unsigned char drvif_color_od_rec_enable_get(void)
{
	od_od_ctrl_RBUS od_od_ctrl_reg;
	od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	//rtd_printk(KERN_EMERG, TAG_NAME, "get od_od_ctrl_reg.rec_od_en = %d\n", od_od_ctrl_reg.rec_od_en);
	return od_od_ctrl_reg.rec_od_en;
}
void drvif_color_od_rec_enable_set(unsigned char od_rec_enable)
{
	int timeoutcnt = 0x032500;
	od_od_ctrl_RBUS od_od_ctrl_reg;
	unsigned char tmp;

	tmp = drvif_color_od_enable_get();
	drvif_color_od_enable_set(0);
	
	od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);

	/* OD function db on */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
	
	/* OD function db read sel: reg */
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25
	
	/* rec OD enable */
	od_od_ctrl_reg.rec_od_en = od_rec_enable;
	IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl_reg.regValue);
	
	/* db apply */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26 
	
	/* check */
	while(drvif_color_od_rec_enable_get() != od_rec_enable && (timeoutcnt--) > 0) {
		//rtd_printk(KERN_EMERG, TAG_NAME, "%s, wait db apply od_rec_enable=%d, timeout cnt=%d", __func__, od_rec_enable, timeoutcnt);
	}
	if(timeoutcnt == 0)	{
		od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
		rtd_pr_vpq_emerg("%s, time out!, timeout cnt = %d, od_ctrl_reg.regValue=%x,\n", __func__, timeoutcnt, od_od_ctrl_reg.rec_od_en);
	}
	else {
		rtd_pr_vpq_info("%s, timeout cnt = %d, set od_rec_enable %d done\n", __func__, timeoutcnt, od_rec_enable);
	}

	/* OD function db off */
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
	
	/* OD function db read sel: temp */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25

	
	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, tmp = 0, rec OD set\n");
		tmp = 0;
	}

	rtd_pr_vpq_info("%s, set rec OD finished\n", __func__);

	drvif_color_od_enable_set(tmp);
}

unsigned char drvif_color_od_scaling_enable_get(void)
{
	od_od_ctrl_RBUS od_od_ctrl_reg;
	od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	//rtd_printk(KERN_EMERG, TAG_NAME, "get od_od_ctrl_reg.od_en = %d\n", od_od_ctrl_reg.od_en);
	return od_od_ctrl_reg.od_scaling_en;
}
void drvif_color_od_scaling_enable_set(unsigned char od_scaling_enable)
{
	int timeoutcnt = 0x032500;
	od_od_ctrl_RBUS od_od_ctrl_reg;
	unsigned char tmp;

	tmp = drvif_color_od_enable_get();
	drvif_color_od_enable_set(0);

	od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);

	/* OD function db on */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24

	/* OD function db read sel: reg */
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25

	/* rec OD enable */
	od_od_ctrl_reg.od_scaling_en = od_scaling_enable;
	IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl_reg.regValue);

	/* db apply */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26

	/* check */
	while(drvif_color_od_scaling_enable_get() != od_scaling_enable && (timeoutcnt--) > 0) {
		//rtd_printk(KERN_EMERG, TAG_NAME, "%s, wait db apply od_rec_enable=%d, timeout cnt=%d", __func__, od_rec_enable, timeoutcnt);
	}
	if(timeoutcnt == 0)	{
		od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
		rtd_pr_vpq_emerg("%s, time out!, timeout cnt = %d, od_ctrl_reg.regValue=%x,\n", __func__, timeoutcnt, od_od_ctrl_reg.od_scaling_en);
	}
	// else {
	// 	rtd_printk(KERN_EMERG, TAG_NAME, "%s, timeout cnt = %d, set od_scaling_enable %d done\n", __func__, timeoutcnt, od_rec_enable);
	// }

	/* OD function db off */
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24

	/* OD function db read sel: temp */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25


	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		// rtd_printk(KERN_INFO, TAG_NAME, "HW_MEX,  MEX_OD != 1, tmp = 0, scaling enable OD set\n");
		tmp = 0;
	}

	// rtd_printk(KERN_EMERG, TAG_NAME, "%s, set OD scaling finished\n", __func__);

	drvif_color_od_enable_set(tmp);
}

DRV_od_ymode drvif_color_od_Ymode_get(void)
{
	DRV_od_ymode ymode = {0};
	od_ymode_ctrl_RBUS od_ymode_ctrl_reg;
	
	od_ymode_ctrl_reg.regValue = IoReg_Read32(OD_YMODE_CTRL_reg);
	ymode.enable = od_ymode_ctrl_reg.od_ymode_en;
	ymode.coef_r = od_ymode_ctrl_reg.r_coeff;
	ymode.coef_g = od_ymode_ctrl_reg.g_coeff;
	ymode.coef_b = od_ymode_ctrl_reg.b_coeff;

	return ymode;
}
void drvif_color_od_Ymode_set(DRV_od_ymode od_ymode)
{
	od_ymode_ctrl_RBUS od_ymode_ctrl_reg;
	
	od_ymode_ctrl_reg.regValue = IoReg_Read32(OD_YMODE_CTRL_reg);
	od_ymode_ctrl_reg.od_ymode_en = od_ymode.enable;
	od_ymode_ctrl_reg.r_coeff = od_ymode.coef_r;
	od_ymode_ctrl_reg.g_coeff = od_ymode.coef_g;
	od_ymode_ctrl_reg.b_coeff = od_ymode.coef_b;
	IoReg_Write32(OD_YMODE_CTRL_reg, od_ymode_ctrl_reg.regValue);
}
unsigned char drvif_color_od_gain_get(unsigned char channel)
{
	od_od_ctrl2_RBUS od_od_ctrl2_reg;
	od_od_ctrl2_reg.regValue = IoReg_Read32(OD_OD_CTRL2_reg);

	switch(channel)
	{
		case 0:
		default:
			return od_od_ctrl2_reg.od_delta_gain_r;
		case 1:
			return od_od_ctrl2_reg.od_delta_gain_g;
		case 2:
			return od_od_ctrl2_reg.od_delta_gain_b;		
	}
}
void drvif_color_od_gain_set(DRV_od_gain* od_delta_gain)
{
	/* od gain only works on delta table */
	od_od_ctrl2_RBUS od_od_ctrl2_reg;
	od_rec_od_ctrl1_RBUS rec_od_od_ctrl2_reg;
	// normal
	od_od_ctrl2_reg.regValue = IoReg_Read32(OD_OD_CTRL2_reg);
	od_od_ctrl2_reg.od_delta_gain_r = od_delta_gain[0].r_gain;
	od_od_ctrl2_reg.od_delta_gain_g = od_delta_gain[0].g_gain;
	od_od_ctrl2_reg.od_delta_gain_b = od_delta_gain[0].b_gain;
	IoReg_Write32(OD_OD_CTRL2_reg, od_od_ctrl2_reg.regValue);
	// recursive
	rec_od_od_ctrl2_reg.regValue = IoReg_Read32(OD_REC_OD_CTRL1_reg);
	rec_od_od_ctrl2_reg.rec_od_delta_gain_r = od_delta_gain[1].r_gain;
	rec_od_od_ctrl2_reg.rec_od_delta_gain_g = od_delta_gain[1].g_gain;
	rec_od_od_ctrl2_reg.rec_od_delta_gain_b = od_delta_gain[1].b_gain;
	IoReg_Write32(OD_REC_OD_CTRL1_reg, rec_od_od_ctrl2_reg.regValue);
}

void drvif_color_set_od_gain_RGB(unsigned char rGain, unsigned char gGain, unsigned char bGain)
{
	od_od_ctrl2_RBUS od_ctrl2_reg;

	if (rGain > 127)
		rGain = 64;
	if (gGain > 127)
		gGain = 64;
	if (bGain > 127)
		bGain = 64;

	od_ctrl2_reg.regValue = IoReg_Read32(OD_OD_CTRL2_reg);
	od_ctrl2_reg.od_delta_gain_r = rGain;
	od_ctrl2_reg.od_delta_gain_g = gGain;
	od_ctrl2_reg.od_delta_gain_b = bGain;

	IoReg_Write32(OD_OD_CTRL2_reg, od_ctrl2_reg.regValue);
}

void drvif_color_od_nr_set(DRV_od_nr* od_nr)
{
	od_od_noise_reduction_ctrl_RBUS noise_reduction_ctrl_reg;
	od_recursive_od_noise_reduction_ctrl_RBUS recursive_noise_reduction_ctrl_reg;
	// normal
	noise_reduction_ctrl_reg.regValue = IoReg_Read32(OD_od_Noise_reduction_ctrl_reg);
	noise_reduction_ctrl_reg.od_nr_mode  = od_nr[0].mode;
	noise_reduction_ctrl_reg.od_b_nr_en  = od_nr[0].b_en;
	noise_reduction_ctrl_reg.od_g_nr_en  = od_nr[0].g_en;
	noise_reduction_ctrl_reg.od_r_nr_en  = od_nr[0].r_en;
	noise_reduction_ctrl_reg.od_b_nr_thd = od_nr[0].b_th;
	noise_reduction_ctrl_reg.od_g_nr_thd = od_nr[0].g_th;
	noise_reduction_ctrl_reg.od_r_nr_thd = od_nr[0].r_th;
	IoReg_Write32(OD_od_Noise_reduction_ctrl_reg, noise_reduction_ctrl_reg.regValue);
	// recursive
	recursive_noise_reduction_ctrl_reg.regValue = IoReg_Read32(OD_recursive_od_Noise_reduction_ctrl_reg);
	recursive_noise_reduction_ctrl_reg.rec_od_nr_mode  = od_nr[1].mode;
	recursive_noise_reduction_ctrl_reg.rec_od_b_nr_en  = od_nr[1].b_en;
	recursive_noise_reduction_ctrl_reg.rec_od_g_nr_en  = od_nr[1].g_en;
	recursive_noise_reduction_ctrl_reg.rec_od_r_nr_en  = od_nr[1].r_en;
	recursive_noise_reduction_ctrl_reg.rec_od_b_nr_thd = od_nr[1].b_th;
	recursive_noise_reduction_ctrl_reg.rec_od_g_nr_thd = od_nr[1].g_th;
	recursive_noise_reduction_ctrl_reg.rec_od_r_nr_thd = od_nr[1].r_th;
	IoReg_Write32(OD_recursive_od_Noise_reduction_ctrl_reg, recursive_noise_reduction_ctrl_reg.regValue);
}


void drvif_color_od_scaling_set(DRV_od_scaling od_scaling){
	od_od_ctrl_RBUS od_od_ctrl_reg;
	od_od_scaling_filter_RBUS od_od_scaling_filter_reg;
	od_od_scaling_filter2_RBUS od_od_scaling_filter2_reg;

	od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	od_od_ctrl_reg.od_scaling_en = od_scaling.enable;
	IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl_reg.regValue);

	od_od_scaling_filter_reg.regValue = IoReg_Read32(OD_OD_SCALING_filter_reg);
	od_od_scaling_filter_reg.scaling_filter_coeff0  = od_scaling.coeff0;
	od_od_scaling_filter_reg.scaling_filter_coeff1  = od_scaling.coeff1;
	od_od_scaling_filter_reg.scaling_filter_coeff2  = od_scaling.coeff2;
	od_od_scaling_filter_reg.scaling_filter_coeff3  = od_scaling.coeff3;
	IoReg_Write32(OD_OD_SCALING_filter_reg, od_od_scaling_filter_reg.regValue);

	od_od_scaling_filter2_reg.regValue = IoReg_Read32(OD_OD_SCALING_filter2_reg);
	od_od_scaling_filter2_reg.scaling_filter_coeff4 = od_scaling.coeff4;
	od_od_scaling_filter2_reg.scaling_edge_mode = od_scaling.edge_mode;
	od_od_scaling_filter2_reg.scaling_r_diff_nr_thd = od_scaling.r_diff_nr_th;
	od_od_scaling_filter2_reg.scaling_g_diff_nr_thd = od_scaling.g_diff_nr_th;
	od_od_scaling_filter2_reg.scaling_b_diff_nr_thd = od_scaling.b_diff_nr_th;
	IoReg_Write32(OD_OD_SCALING_filter2_reg, od_od_scaling_filter2_reg.regValue);

}

void drvif_color_od_localgain_set(DRV_od_localgain od_localgain){
	od_od_local_gain_RBUS od_od_local_gain_reg;

	od_od_local_gain_reg.localgain_enable = od_localgain.enable;
	od_od_local_gain_reg.localgain_weight = od_localgain.weight;
	od_od_local_gain_reg.localgain_nr_thd = od_localgain.nr_thd;
	od_od_local_gain_reg.localgain_edge_hhf_thd = od_localgain.edge_hhf_thd;

	IoReg_Write32(OD_OD_local_gain_reg, od_od_local_gain_reg.regValue);
}

void drvif_color_od_sat_y_set(DRV_od_sat_y od_sat_y){
	od_ymode_ctrl_RBUS od_ymode_ctrl_reg;
	od_ymode_sat_RBUS od_ymode_sat_reg;
	od_ymode_sat_th_RBUS od_ymode_sat_th_reg;
	od_ymode_sat_offset_gain_RBUS od_ymode_sat_offset_gain_reg;
	od_ymode_sat_y_RBUS od_ymode_sat_y_reg;
	od_ymode_sat_y_color2gray_RBUS od_ymode_sat_y_color2gray_reg;
	od_ymode_sat_y_slope_RBUS od_ymode_sat_y_slope_reg;

	od_ymode_ctrl_reg.regValue = IoReg_Read32(OD_YMODE_CTRL_reg);
	od_ymode_ctrl_reg.od_ymode_sat_en = od_sat_y.ctrl.bias_en;
	IoReg_Write32(OD_YMODE_SAT_reg, od_ymode_ctrl_reg.regValue);

	od_ymode_sat_reg.regValue = IoReg_Read32(OD_YMODE_SAT_reg);
	od_ymode_sat_reg.sat_r_en = od_sat_y.ctrl.bias_r_en;
	od_ymode_sat_reg.sat_g_en = od_sat_y.ctrl.bias_g_en;
	od_ymode_sat_reg.sat_b_en = od_sat_y.ctrl.bias_b_en;
	od_ymode_sat_reg.sat_c_en = od_sat_y.ctrl.bias_c_en;
	od_ymode_sat_reg.sat_y_en = od_sat_y.ctrl.bias_y_en;
	od_ymode_sat_reg.sat_m_en = od_sat_y.ctrl.bias_m_en;
	od_ymode_sat_reg.sat_color2gray_en = od_sat_y.ctrl.color2gray_en;
	od_ymode_sat_reg.sat_gray2cmy_en = od_sat_y.ctrl.gray2cmy_en;
	od_ymode_sat_reg.sat_gray2rgb_en = od_sat_y.ctrl.gray2rgb_en;
	od_ymode_sat_reg.sat_y_diff_pos_en = od_sat_y.ctrl.y_diff_pos_en;
	od_ymode_sat_reg.sat_y_diff_neg_en = od_sat_y.ctrl.y_diff_neg_en;
	od_ymode_sat_reg.sat_offset_clamp_en = od_sat_y.ctrl.offset_clamp_en;
	od_ymode_sat_reg.sat_r_diff_th = od_sat_y.thd.r_diff_th;
	od_ymode_sat_reg.sat_g_diff_th = od_sat_y.thd.g_diff_th;
	od_ymode_sat_reg.sat_b_diff_th = od_sat_y.thd.b_diff_th;
	IoReg_Write32(OD_YMODE_SAT_reg, od_ymode_sat_reg.regValue);

	od_ymode_sat_th_reg.regValue = IoReg_Read32(OD_YMODE_SAT_TH_reg);
	od_ymode_sat_th_reg.sat_color2gray_th = od_sat_y.thd.gray_diff_th;
	od_ymode_sat_th_reg.sat_rg_diff_th = od_sat_y.thd.rg_diff_th;
	od_ymode_sat_th_reg.sat_gb_diff_th = od_sat_y.thd.gb_diff_th;
	od_ymode_sat_th_reg.sat_br_diff_th = od_sat_y.thd.br_diff_th;
	od_ymode_sat_th_reg.sat_diff_th_factor = od_sat_y.thd.diff_th_factor;
	IoReg_Write32(OD_YMODE_SAT_TH_reg, od_ymode_sat_th_reg.regValue);

	od_ymode_sat_offset_gain_reg.regValue = IoReg_Read32(OD_YMODE_SAT_OFFSET_GAIN_reg);
	od_ymode_sat_offset_gain_reg.sat_color2gray_offset_gain = od_sat_y.gain.gray_ofst_gain;
	od_ymode_sat_offset_gain_reg.sat_gray2r_offset_gain = od_sat_y.gain.r_ofst_gain;
	od_ymode_sat_offset_gain_reg.sat_gray2g_offset_gain = od_sat_y.gain.g_ofst_gain;
	od_ymode_sat_offset_gain_reg.sat_gray2b_offset_gain = od_sat_y.gain.b_ofst_gain;
	od_ymode_sat_offset_gain_reg.sat_gray2c_offset_gain = od_sat_y.gain.c_ofst_gain;
	od_ymode_sat_offset_gain_reg.sat_gray2m_offset_gain = od_sat_y.gain.m_ofst_gain;
	od_ymode_sat_offset_gain_reg.sat_gray2y_offset_gain = od_sat_y.gain.y_ofst_gain;
	IoReg_Write32(OD_YMODE_SAT_OFFSET_GAIN_reg, od_ymode_sat_offset_gain_reg.regValue);

	od_ymode_sat_y_reg.regValue = IoReg_Read32(OD_YMODE_SAT_Y_reg);
	od_ymode_sat_y_reg.offset_hi_th = od_sat_y.slope.offset_hi_th;
	od_ymode_sat_y_reg.offset_lo_th = od_sat_y.slope.offset_lo_th;
	od_ymode_sat_y_reg.y_diff_hi_th = od_sat_y.slope.ydiff_hi_th;
	od_ymode_sat_y_reg.y_diff_lo_th = od_sat_y.slope.ydiff_lo_th;
	IoReg_Write32(OD_YMODE_SAT_Y_reg, od_ymode_sat_y_reg.regValue);


	od_ymode_sat_y_color2gray_reg.regValue = IoReg_Read32(OD_YMODE_SAT_Y_COLOR2GRAY_reg);
	od_ymode_sat_y_color2gray_reg.offset_hi_th_color2gray = od_sat_y.slope.offset_hi_th_gray;
	od_ymode_sat_y_color2gray_reg.offset_lo_th_color2gray = od_sat_y.slope.offset_lo_th_gray;
	od_ymode_sat_y_color2gray_reg.y_diff_hi_th_color2gray = od_sat_y.slope.ydiff_hi_th_gray;
	od_ymode_sat_y_color2gray_reg.y_diff_lo_th_color2gray = od_sat_y.slope.ydiff_lo_th_gray;
	IoReg_Write32(OD_YMODE_SAT_Y_COLOR2GRAY_reg, od_ymode_sat_y_color2gray_reg.regValue);


	od_ymode_sat_y_slope_reg.regValue = IoReg_Read32(OD_YMODE_SAT_Y_SLOPE_reg);
	od_ymode_sat_y_slope_reg.offset_slope = od_sat_y.slope.offset_slope;
	od_ymode_sat_y_slope_reg.offset_slope_color2gray = od_sat_y.slope.offset_slope_gray;
	IoReg_Write32(OD_YMODE_SAT_Y_SLOPE_reg, od_ymode_sat_y_slope_reg.regValue);
}

void drvif_color_od_set_vergain(DRV_OD_VerGain* od_vergain)
{
	// normal
	od_od_vertical_gain_setting_RBUS od_vergain_reg;
	od_od_vertical_gain_debug_mode_RBUS od_vergain_debug_mode_reg;
	od_od_vertical_gain_table0_RBUS od_vergain_table0_reg;
	od_od_vertical_gain_table1_RBUS od_vergain_table1_reg;
	od_od_vertical_gain_table2_RBUS od_vergain_table2_reg;
	od_od_vertical_gain_table3_RBUS od_vergain_table3_reg;
	od_od_vertical_gain_table4_RBUS od_vergain_table4_reg;
	od_od_vertical_gain_table5_RBUS od_vergain_table5_reg;
	od_od_vertical_gain_table6_RBUS od_vergain_table6_reg;
	od_od_vertical_gain_table7_RBUS od_vergain_table7_reg;
	// recursive
	od_od_rec_vertical_gain_setting_RBUS od_rec_vergain_reg;
	od_od_rec_vertical_gain_debug_mode_RBUS od_rec_vergain_debug_mode_reg;
	od_od_rec_vertical_gain_table0_RBUS od_rec_vergain_table0_reg;
	od_od_rec_vertical_gain_table1_RBUS od_rec_vergain_table1_reg;
	od_od_rec_vertical_gain_table2_RBUS od_rec_vergain_table2_reg;
	od_od_rec_vertical_gain_table3_RBUS od_rec_vergain_table3_reg;
	od_od_rec_vertical_gain_table4_RBUS od_rec_vergain_table4_reg;
	od_od_rec_vertical_gain_table5_RBUS od_rec_vergain_table5_reg;
	od_od_rec_vertical_gain_table6_RBUS od_rec_vergain_table6_reg;
	od_od_rec_vertical_gain_table7_RBUS od_rec_vergain_table7_reg;

	// normal
	od_vergain_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_SETTING_reg);
	od_vergain_debug_mode_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_DEBUG_MODE_reg);
	od_vergain_table0_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table0_reg);
	od_vergain_table1_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table1_reg);
	od_vergain_table2_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table2_reg);
	od_vergain_table3_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table3_reg);
	od_vergain_table4_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table4_reg);
	od_vergain_table5_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table5_reg);
	od_vergain_table6_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table6_reg);
	od_vergain_table7_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table7_reg);

	od_vergain_reg.vergain_en = od_vergain[0].enable;
	od_vergain_reg.vergain_factor = od_vergain[0].factor;
	od_vergain_debug_mode_reg.vergain_offset = od_vergain[0].offset;
	od_vergain_table0_reg.vergain_table0  = od_vergain[0].vergain[0];
	od_vergain_table0_reg.vergain_table1  = od_vergain[0].vergain[1];
	od_vergain_table0_reg.vergain_table2  = od_vergain[0].vergain[2];
	od_vergain_table0_reg.vergain_table3  = od_vergain[0].vergain[3];
	od_vergain_table1_reg.vergain_table4  = od_vergain[0].vergain[4];
	od_vergain_table1_reg.vergain_table5  = od_vergain[0].vergain[5];
	od_vergain_table1_reg.vergain_table6  = od_vergain[0].vergain[6];
	od_vergain_table1_reg.vergain_table7  = od_vergain[0].vergain[7];
	od_vergain_table2_reg.vergain_table8  = od_vergain[0].vergain[8];
	od_vergain_table2_reg.vergain_table9  = od_vergain[0].vergain[9];
	od_vergain_table2_reg.vergain_table10 = od_vergain[0].vergain[10];
	od_vergain_table2_reg.vergain_table11 = od_vergain[0].vergain[11];
	od_vergain_table3_reg.vergain_table12 = od_vergain[0].vergain[12];
	od_vergain_table3_reg.vergain_table13 = od_vergain[0].vergain[13];
	od_vergain_table3_reg.vergain_table14 = od_vergain[0].vergain[14];
	od_vergain_table3_reg.vergain_table15 = od_vergain[0].vergain[15];
	od_vergain_table4_reg.vergain_table16 = od_vergain[0].vergain[16];
	od_vergain_table4_reg.vergain_table17 = od_vergain[0].vergain[17];
	od_vergain_table4_reg.vergain_table18 = od_vergain[0].vergain[18];
	od_vergain_table4_reg.vergain_table19 = od_vergain[0].vergain[19];
	od_vergain_table5_reg.vergain_table20 = od_vergain[0].vergain[20];
	od_vergain_table5_reg.vergain_table21 = od_vergain[0].vergain[21];
	od_vergain_table5_reg.vergain_table22 = od_vergain[0].vergain[22];
	od_vergain_table5_reg.vergain_table23 = od_vergain[0].vergain[23];
	od_vergain_table6_reg.vergain_table24 = od_vergain[0].vergain[24];
	od_vergain_table6_reg.vergain_table25 = od_vergain[0].vergain[25];
	od_vergain_table6_reg.vergain_table26 = od_vergain[0].vergain[26];
	od_vergain_table6_reg.vergain_table27 = od_vergain[0].vergain[27];
	od_vergain_table7_reg.vergain_table28 = od_vergain[0].vergain[28];
	od_vergain_table7_reg.vergain_table29 = od_vergain[0].vergain[29];
	od_vergain_table7_reg.vergain_table30 = od_vergain[0].vergain[30];
	od_vergain_table7_reg.vergain_table31 = od_vergain[0].vergain[31];

	IoReg_Write32(OD_OD_VERTICAL_GAIN_SETTING_reg, od_vergain_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_DEBUG_MODE_reg, od_vergain_debug_mode_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table0_reg, od_vergain_table0_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table1_reg, od_vergain_table1_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table2_reg, od_vergain_table2_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table3_reg, od_vergain_table3_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table4_reg, od_vergain_table4_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table5_reg, od_vergain_table5_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table6_reg, od_vergain_table6_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table7_reg, od_vergain_table7_reg.regValue);

	// recursive
	od_rec_vergain_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_SETTING_reg);
	od_rec_vergain_debug_mode_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_DEBUG_MODE_reg);
	od_rec_vergain_table0_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_table0_reg);
	od_rec_vergain_table1_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_table1_reg);
	od_rec_vergain_table2_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_table2_reg);
	od_rec_vergain_table3_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_table3_reg);
	od_rec_vergain_table4_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_table4_reg);
	od_rec_vergain_table5_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_table5_reg);
	od_rec_vergain_table6_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_table6_reg);
	od_rec_vergain_table7_reg.regValue = IoReg_Read32(OD_OD_REC_VERTICAL_GAIN_table7_reg);

	od_rec_vergain_reg.rec_vergain_en = od_vergain[1].enable;
	od_rec_vergain_reg.rec_vergain_factor = od_vergain[1].factor;
	od_rec_vergain_debug_mode_reg.rec_vergain_offset = od_vergain[1].offset;
	od_rec_vergain_table0_reg.rec_vergain_table0  = od_vergain[1].vergain[0];
	od_rec_vergain_table0_reg.rec_vergain_table1  = od_vergain[1].vergain[1];
	od_rec_vergain_table0_reg.rec_vergain_table2  = od_vergain[1].vergain[2];
	od_rec_vergain_table0_reg.rec_vergain_table3  = od_vergain[1].vergain[3];
	od_rec_vergain_table1_reg.rec_vergain_table4  = od_vergain[1].vergain[4];
	od_rec_vergain_table1_reg.rec_vergain_table5  = od_vergain[1].vergain[5];
	od_rec_vergain_table1_reg.rec_vergain_table6  = od_vergain[1].vergain[6];
	od_rec_vergain_table1_reg.rec_vergain_table7  = od_vergain[1].vergain[7];
	od_rec_vergain_table2_reg.rec_vergain_table8  = od_vergain[1].vergain[8];
	od_rec_vergain_table2_reg.rec_vergain_table9  = od_vergain[1].vergain[9];
	od_rec_vergain_table2_reg.rec_vergain_table10 = od_vergain[1].vergain[10];
	od_rec_vergain_table2_reg.rec_vergain_table11 = od_vergain[1].vergain[11];
	od_rec_vergain_table3_reg.rec_vergain_table12 = od_vergain[1].vergain[12];
	od_rec_vergain_table3_reg.rec_vergain_table13 = od_vergain[1].vergain[13];
	od_rec_vergain_table3_reg.rec_vergain_table14 = od_vergain[1].vergain[14];
	od_rec_vergain_table3_reg.rec_vergain_table15 = od_vergain[1].vergain[15];
	od_rec_vergain_table4_reg.rec_vergain_table16 = od_vergain[1].vergain[16];
	od_rec_vergain_table4_reg.rec_vergain_table17 = od_vergain[1].vergain[17];
	od_rec_vergain_table4_reg.rec_vergain_table18 = od_vergain[1].vergain[18];
	od_rec_vergain_table4_reg.rec_vergain_table19 = od_vergain[1].vergain[19];
	od_rec_vergain_table5_reg.rec_vergain_table20 = od_vergain[1].vergain[20];
	od_rec_vergain_table5_reg.rec_vergain_table21 = od_vergain[1].vergain[21];
	od_rec_vergain_table5_reg.rec_vergain_table22 = od_vergain[1].vergain[22];
	od_rec_vergain_table5_reg.rec_vergain_table23 = od_vergain[1].vergain[23];
	od_rec_vergain_table6_reg.rec_vergain_table24 = od_vergain[1].vergain[24];
	od_rec_vergain_table6_reg.rec_vergain_table25 = od_vergain[1].vergain[25];
	od_rec_vergain_table6_reg.rec_vergain_table26 = od_vergain[1].vergain[26];
	od_rec_vergain_table6_reg.rec_vergain_table27 = od_vergain[1].vergain[27];
	od_rec_vergain_table7_reg.rec_vergain_table28 = od_vergain[1].vergain[28];
	od_rec_vergain_table7_reg.rec_vergain_table29 = od_vergain[1].vergain[29];
	od_rec_vergain_table7_reg.rec_vergain_table30 = od_vergain[1].vergain[30];
	od_rec_vergain_table7_reg.rec_vergain_table31 = od_vergain[1].vergain[31];

	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_SETTING_reg, od_rec_vergain_reg.regValue);
	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_DEBUG_MODE_reg, od_rec_vergain_debug_mode_reg.regValue);
	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_table0_reg, od_rec_vergain_table0_reg.regValue);
	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_table1_reg, od_rec_vergain_table1_reg.regValue);
	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_table2_reg, od_rec_vergain_table2_reg.regValue);
	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_table3_reg, od_rec_vergain_table3_reg.regValue);
	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_table4_reg, od_rec_vergain_table4_reg.regValue);
	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_table5_reg, od_rec_vergain_table5_reg.regValue);
	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_table6_reg, od_rec_vergain_table6_reg.regValue);
	IoReg_Write32(OD_OD_REC_VERTICAL_GAIN_table7_reg, od_rec_vergain_table7_reg.regValue);
}

bool drvif_color_od_dma_line_size_get(DRV_od_dma *oddma)
{
	unsigned int od_size = 0;
	unsigned long od_addr = 0;
	unsigned long align_value =0;
	unsigned int burst_length = oddma->dma_pair.burst_length;
	
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	od_size = carvedout_buf_query_secure(CARVEDOUT_SCALER_OD,(void*) &od_addr);
#else
	od_size = carvedout_buf_query(CARVEDOUT_SCALER_OD,(void*) &od_addr);
#endif

	if (od_addr == 0 || od_size == 0) {
		rtd_printk(KERN_EMERG, TAG_OD_MEM, "[%s %d] ERR : %lx %x\n", __func__, __LINE__, od_addr, od_size);
		return FALSE;
	}
	rtd_printk(KERN_EMERG, TAG_OD_MEM, "%s, get_addr=0x%lx,get_size=%d,h=%d,w=%d,Bit=%d\n",
		__func__, od_addr, od_size, oddma->dma_ctrl.vact, oddma->dma_ctrl.hact, oddma->dma_comp.frame_limit_bit);
	
#ifdef CONFIG_BW_96B_ALIGNED
	//96 Alignment for Bandwidth Request
	align_value = dvr_memory_alignment((unsigned long)od_addr, dvr_size_alignment(oddma->dma_ctrl.vact*oddma->dma_pair.line_step));
	burst_length = 0x30;
#else
    align_value = dvr_memory_alignment((unsigned long)od_addr, dvr_size_alignment(oddma->dma_ctrl.vact*oddma->dma_pqc_pair.line_sum_bit));
    burst_length = 0x20;
#endif

	if (align_value == 0)
#ifndef BUILD_QUICK_SHOW		
		BUG();
#else
		return FALSE;
#endif		
	oddma->dma_pair.burst_length = burst_length;
	oddma->dma_pair.start_addr1 = align_value + burst_length;
	oddma->dma_pair.up_limit = od_addr + od_size - burst_length;
	oddma->dma_pair.low_limit = od_addr;
	
	rtd_printk(KERN_EMERG, TAG_OD_MEM, "%s,burst=0x%x,addr=0x%x,up=0x%x,low=0x%x,ddr=0x%lx\n",
		__func__, oddma->dma_pair.burst_length, oddma->dma_pair.start_addr1, oddma->dma_pair.up_limit, oddma->dma_pair.low_limit, align_value);
		
	return TRUE;
}
bool drvif_color_od_dma_frame_size_get(DRV_od_dma *oddma)
{
	// TODO: dma size for frame mode
	return FALSE;
}
void drvif_color_od_dma_addr_set(DRV_od_dma *oddma)
{
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, oddma->dma_pair.start_addr1); //must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_up_reg, oddma->dma_pair.up_limit);
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_low_reg, oddma->dma_pair.low_limit);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, oddma->dma_pair.start_addr1);//must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_up_reg, oddma->dma_pair.up_limit);
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_low_reg, oddma->dma_pair.low_limit);
}
void drvif_color_od_dma_ctrl_set(DRV_od_dma *oddma)
{
	od_dma_od_dma_ctrl_2_RBUS od_dma_od_dma_ctrl_2_reg;
	od_dma_od_dma_ctrl_RBUS od_dma_od_dma_ctrl_reg;
	od_od_ctrl_RBUS od_od_ctrl_reg;
	
	od_dma_od_dma_ctrl_2_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_2_reg);
	od_dma_od_dma_ctrl_2_reg.vact = oddma->dma_ctrl.vact;
	od_dma_od_dma_ctrl_2_reg.hact = oddma->dma_ctrl.hact;
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_2_reg, od_dma_od_dma_ctrl_2_reg.regValue);
	
	od_dma_od_dma_ctrl_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
	od_dma_od_dma_ctrl_reg.bit_sel = oddma->dma_ctrl.bit_sel; // Merlin5 constraint: comp_bypass=0, bit_sel=2
	od_dma_od_dma_ctrl_reg.pixel_encoding = oddma->dma_ctrl.pixel_encoding; // Merlin5 constraint: comp_bypass=0, pixel_encoding=0
	od_dma_od_dma_ctrl_reg.oddma_mode_sel = oddma->dma_ctrl.dma_mode; // 0: line mode, 1: frame mode
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_reg, od_dma_od_dma_ctrl_reg.regValue);

	od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	od_od_ctrl_reg.dma_1p_2p_mode = oddma->dma_ctrl.dma_2p_enable;
	od_od_ctrl_reg.oddma_clk_sel = oddma->dma_ctrl.dma_clk_sel;
	IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl_reg.regValue);
}
void drvif_color_od_dma_pair_set(DRV_od_dma *oddma)
{
	od_dma_oddma_wr_num_bl_wrap_line_step_RBUS od_dma_oddma_wr_num_bl_wrap_line_step_reg;
	od_dma_oddma_rd_num_bl_wrap_line_step_RBUS od_dma_oddma_rd_num_bl_wrap_line_step_reg;
	od_dma_oddma_wr_num_bl_wrap_ctl_RBUS od_dma_oddma_wr_num_bl_wrap_ctl_reg;
	od_dma_oddma_rd_num_bl_wrap_ctl_RBUS od_dma_oddma_rd_num_bl_wrap_ctl_reg;
	od_dma_oddma_wr_num_bl_wrap_word_RBUS od_dma_oddma_wr_num_bl_wrap_word_reg;
	od_dma_oddma_rd_num_bl_wrap_word_RBUS od_dma_oddma_rd_num_bl_wrap_word_reg;
	od_dma_oddma_wr_num_bl_wrap_rolling_RBUS od_dma_oddma_wr_num_bl_wrap_rolling_reg;
	od_dma_oddma_rd_num_bl_wrap_rolling_RBUS od_dma_oddma_rd_num_bl_wrap_rolling_reg;
	od_dma_oddma_wr_ctrl_RBUS od_dma_oddma_wr_ctrl_reg;
	od_dma_oddma_rd_ctrl_RBUS od_dma_oddma_rd_ctrl_reg;
		
	od_dma_oddma_wr_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
	od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
	od_dma_oddma_wr_num_bl_wrap_line_step_reg.cap_line_step = oddma->dma_pair.line_step;
	od_dma_oddma_rd_num_bl_wrap_line_step_reg.disp_line_step = oddma->dma_pair.line_step;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg, od_dma_oddma_wr_num_bl_wrap_line_step_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg, od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue);

	od_dma_oddma_wr_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg);
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg);
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.cap_line_num = oddma->dma_pair.line_num;
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.disp_line_num = oddma->dma_pair.line_num;
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.cap_burst_len = oddma->dma_pair.burst_length;
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.disp_burst_len = oddma->dma_pair.burst_length;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg, od_dma_oddma_wr_num_bl_wrap_ctl_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg, od_dma_oddma_rd_num_bl_wrap_ctl_reg.regValue);

	od_dma_oddma_wr_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
	od_dma_oddma_rd_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);
	od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = oddma->dma_pair.line128_num;
	od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = oddma->dma_pair.line128_num;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg, od_dma_oddma_wr_num_bl_wrap_word_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg, od_dma_oddma_rd_num_bl_wrap_word_reg.regValue);
	
	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	od_dma_oddma_wr_ctrl_reg.cap_max_outstanding = oddma->dma_pair.max_outstanding;
	od_dma_oddma_rd_ctrl_reg.disp_max_outstanding = oddma->dma_pair.max_outstanding;
	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg, od_dma_oddma_wr_ctrl_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg, od_dma_oddma_rd_ctrl_reg.regValue);
	
	// rolling
	od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg);
	od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg);
	od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_enable = oddma->dma_pair.rolling_en;
	od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_enable = oddma->dma_pair.rolling_en;
	od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_space = oddma->dma_pair.rolling_space;
	od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_space = oddma->dma_pair.rolling_space;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg, od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg, od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue);
		
}

unsigned char drvif_color_od_dma_enable_get(void)
{
	od_dma_oddma_wr_ctrl_RBUS od_dma_oddma_wr_ctrl_reg;
	od_dma_oddma_rd_ctrl_RBUS od_dma_oddma_rd_ctrl_reg;

	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);

	if (od_dma_oddma_wr_ctrl_reg.cap_en && od_dma_oddma_rd_ctrl_reg.disp_en)
		return TRUE;
	else
		return FALSE;
}
void drvif_color_od_dma_cap_enable_set(unsigned char enable)
{
	int timeoutcnt = 0x032500;
	od_dma_oddma_wr_ctrl_RBUS od_dma_oddma_wr_ctrl_reg;

	/* disable od when disable cap and od is enabled */	
	if (!enable && drvif_color_od_enable_get())
		drvif_color_od_enable_set(0);

	/* enable cap en db */
	IoReg_SetBits(OD_DMA_OD_DMA_DB_EN_reg, OD_DMA_OD_DMA_DB_EN_cap_db_en_mask); //_BIT0

	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_wr_ctrl_reg.cap_en = enable;
	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg, od_dma_oddma_wr_ctrl_reg.regValue);
	
	/* apply cap en db */
	IoReg_SetBits(OD_DMA_OD_DMA_DB_EN_reg, OD_DMA_OD_DMA_DB_EN_cap_db_apply_mask); //_BIT2
	
	/* check */
	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	while(od_dma_oddma_wr_ctrl_reg.cap_en != enable && (timeoutcnt--) > 0) {
		rtd_pr_vpq_debug("%s, wait db apply, timeout cnt = %d", __func__, timeoutcnt);
		od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	}
	if(timeoutcnt == 0)	{
		rtd_pr_vpq_emerg("%s, time out 0, timeout cnt = %d, cap_en = %d,\n", __func__, timeoutcnt, od_dma_oddma_wr_ctrl_reg.cap_en);
	}
	else {
		rtd_pr_vpq_info("%s, timeout cnt = %d, cap_en set done", __func__, timeoutcnt);
	}
	
	/* disable cap en db */
	IoReg_ClearBits(OD_DMA_OD_DMA_DB_EN_reg, OD_DMA_OD_DMA_DB_EN_cap_db_en_mask); //_BIT0
}
void drvif_color_od_dma_disp_enable_set(unsigned char enable)
{
	int timeoutcnt = 0x032500;
	od_dma_oddma_rd_ctrl_RBUS od_dma_oddma_rd_ctrl_reg;

	/* disable od when disable cap and od is enabled */	
	if (!enable && drvif_color_od_enable_get())
		drvif_color_od_enable_set(0);
	
	/* enable disp en db */
	IoReg_SetBits(OD_DMA_OD_DMA_DB_EN_reg, OD_DMA_OD_DMA_DB_EN_disp_db_en_mask); //_BIT4

	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	od_dma_oddma_rd_ctrl_reg.disp_en = enable;
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg, od_dma_oddma_rd_ctrl_reg.regValue);
	
	/* apply cap en db */
	IoReg_SetBits(OD_DMA_OD_DMA_DB_EN_reg, OD_DMA_OD_DMA_DB_EN_disp_db_apply_mask); //_BIT6
	
	/* check */
	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	while(od_dma_oddma_rd_ctrl_reg.disp_en!= enable && (timeoutcnt--) > 0) {
		rtd_pr_vpq_debug("%s, wait db apply, timeout cnt = %d", __func__, timeoutcnt);
		od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	}
	if(timeoutcnt == 0)	{
		rtd_pr_vpq_emerg("%s, time out 0, timeout cnt = %d, disp_en = %d,\n", __func__, timeoutcnt, od_dma_oddma_rd_ctrl_reg.disp_en);
	}
	else {
		rtd_pr_vpq_info("%s, timeout cnt = %d, disp_en set done", __func__, timeoutcnt);
	}
	
	/* disable disp en db */
	IoReg_ClearBits(OD_DMA_OD_DMA_DB_EN_reg, OD_DMA_OD_DMA_DB_EN_disp_db_en_mask); //_BIT4
}
void drvif_color_od_dma_pqc_set(DRV_od_dma *oddma)
{
	od_dma_oddma_pqc_pqdc_compensation_RBUS od_dma_oddma_pqc_pqdc_compensation_reg;
	od_dma_oddma_pq_decmp_sat_en_RBUS od_dma_oddma_pq_decmp_sat_en_reg;
	od_dma_oddma_pq_cmp_enable_RBUS od_dma_oddma_pq_cmp_enable_reg;
	od_dma_oddma_pq_cmp_allocate_RBUS od_dma_oddma_pq_cmp_allocate_reg;
 	od_dma_oddma_pq_cmp_balance_RBUS od_dma_oddma_pq_cmp_balance_reg;
	od_dma_oddma_pq_cmp_qp_st_RBUS od_dma_oddma_pq_cmp_qp_st_reg;
	//od_dma_oddma_pq_cmp_fifo_st1_RBUS od_dma_oddma_pq_cmp_fifo_st1_reg;
	
	// pq_cmp_enabl
	od_dma_oddma_pq_cmp_enable_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg);
	od_dma_oddma_pq_cmp_enable_reg.g_ratio = oddma->dma_comp.g_ratio;
	od_dma_oddma_pq_cmp_enable_reg.fisrt_line_more_en = oddma->dma_comp.fisrt_line_more_en;
	od_dma_oddma_pq_cmp_enable_reg.first_predict_nc_mode = oddma->dma_comp.first_predict_nc_mode;
	od_dma_oddma_pq_cmp_enable_reg.guarantee_max_qp_en = oddma->dma_comp.guarantee_max_qp_en;
	od_dma_oddma_pq_cmp_enable_reg.variation_maxmin_en = oddma->dma_comp.variation_maxmin_en;
	od_dma_oddma_pq_cmp_enable_reg.cmp_ctrl_para0 = oddma->dma_comp.cmp_ctrl_para0;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg, od_dma_oddma_pq_cmp_enable_reg.regValue);

	od_dma_oddma_pq_cmp_qp_st_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_QP_ST_reg);
	od_dma_oddma_pq_cmp_qp_st_reg.cmp_ctrl_para2 = oddma->dma_comp.cmp_ctrl_para2;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_QP_ST_reg, od_dma_oddma_pq_cmp_qp_st_reg.regValue);

	// compensation	
	od_dma_oddma_pqc_pqdc_compensation_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg);
	od_dma_oddma_pqc_pqdc_compensation_reg.tr_en = oddma->dma_comp.compensation_en;
	IoReg_Write32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg, od_dma_oddma_pqc_pqdc_compensation_reg.regValue);

	// pqdc saturation
	od_dma_oddma_pq_decmp_sat_en_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_SAT_EN_reg);
	od_dma_oddma_pq_decmp_sat_en_reg.saturation_en = oddma->dma_comp.pqdc_saturation_en;
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_SAT_EN_reg, od_dma_oddma_pq_decmp_sat_en_reg.regValue);
	
	// allocate
	od_dma_oddma_pq_cmp_allocate_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg);
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_less = oddma->dma_pqc_alloc.dynamic_allocate_less;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_line = oddma->dma_pqc_alloc.dynamic_allocate_line;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_more = oddma->dma_pqc_alloc.dynamic_allocate_more;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = oddma->dma_pqc_alloc.dynamic_allocate_ratio_max;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = oddma->dma_pqc_alloc.dynamic_allocate_ratio_min;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg, od_dma_oddma_pq_cmp_allocate_reg.regValue);

	// balance
	od_dma_oddma_pq_cmp_balance_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BALANCE_reg);
	od_dma_oddma_pq_cmp_balance_reg.balance_rb_ov_th = oddma->dma_pqc_balance.balance_rb_ov_th;
	od_dma_oddma_pq_cmp_balance_reg.balance_rb_ud_th = oddma->dma_pqc_balance.balance_rb_ud_th;
	od_dma_oddma_pq_cmp_balance_reg.balance_rb_give = oddma->dma_pqc_balance.balance_rb_give;
	od_dma_oddma_pq_cmp_balance_reg.balance_g_ov_th = oddma->dma_pqc_balance.balance_g_ov_th;
	od_dma_oddma_pq_cmp_balance_reg.balance_g_ud_th = oddma->dma_pqc_balance.balance_g_ud_th;;
	od_dma_oddma_pq_cmp_balance_reg.balance_g_give = oddma->dma_pqc_balance.balance_g_give;	
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BALANCE_reg, od_dma_oddma_pq_cmp_balance_reg.regValue);
	
}
void drvif_color_od_dma_pqc_bit_set(DRV_od_dma * oddma)
{
	od_dma_oddma_pq_cmp_bit_RBUS od_dma_oddma_pq_cmp_bit_reg;
	
	od_dma_oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);
	od_dma_oddma_pq_cmp_bit_reg.first_line_more_bit = oddma->dma_comp.line_more_bit;
	od_dma_oddma_pq_cmp_bit_reg.block_limit_bit = oddma->dma_comp.block_limit_bit;
	od_dma_oddma_pq_cmp_bit_reg.line_limit_bit = oddma->dma_comp.line_limit_bit;
	od_dma_oddma_pq_cmp_bit_reg.frame_limit_bit = oddma->dma_comp.frame_limit_bit;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BIT_reg, od_dma_oddma_pq_cmp_bit_reg.regValue);
}
void drvif_color_od_dma_pqc_pair_set(DRV_od_dma *oddma)
{
	od_dma_oddma_pq_cmp_pair_RBUS od_dma_oddma_pq_cmp_pair_reg;
	od_dma_oddma_pq_decmp_pair_RBUS od_dma_oddma_pq_decmp_pair_reg;
	od_dma_oddma_pq_cmp_RBUS od_dma_oddma_pq_cmp_reg;
	od_dma_oddma_pq_decmp_RBUS od_dma_oddma_pq_decmp_reg;

	od_dma_oddma_pq_cmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg);
	od_dma_oddma_pq_decmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg);
	od_dma_oddma_pq_cmp_pair_reg.cmp_400_old_mode_en = oddma->dma_pqc_pair.old_400_mode;
	od_dma_oddma_pq_decmp_pair_reg.decmp_400_old_mode_en = oddma->dma_pqc_pair.old_400_mode;
	od_dma_oddma_pq_cmp_pair_reg.cmp_data_bit_width = oddma->dma_pqc_pair.data_bitwidth;
	od_dma_oddma_pq_decmp_pair_reg.decmp_data_bit_width = oddma->dma_pqc_pair.data_bitwidth;
	od_dma_oddma_pq_cmp_pair_reg.cmp_data_color = oddma->dma_pqc_pair.data_color;
	od_dma_oddma_pq_decmp_pair_reg.decmp_data_color = oddma->dma_pqc_pair.data_color;
	od_dma_oddma_pq_cmp_pair_reg.cmp_data_format= oddma->dma_pqc_pair.data_format;
	od_dma_oddma_pq_decmp_pair_reg.decmp_data_format = oddma->dma_pqc_pair.data_format;
	od_dma_oddma_pq_cmp_pair_reg.cmp_dic_mode_en = oddma->dma_pqc_pair.dic_mode;
	od_dma_oddma_pq_decmp_pair_reg.decmp_dic_mode_en = oddma->dma_pqc_pair.dic_mode;
	od_dma_oddma_pq_cmp_pair_reg.cmp_jump4_en = oddma->dma_pqc_pair.jump4_en;
	od_dma_oddma_pq_decmp_pair_reg.decmp_jump4_en = oddma->dma_pqc_pair.jump4_en;
	od_dma_oddma_pq_cmp_pair_reg.cmp_line_mode = oddma->dma_pqc_pair.line_mode;
	od_dma_oddma_pq_decmp_pair_reg.decmp_line_mode = oddma->dma_pqc_pair.line_mode;
	od_dma_oddma_pq_cmp_pair_reg.cmp_line_sum_bit = oddma->dma_pqc_pair.line_sum_bit;
	od_dma_oddma_pq_decmp_pair_reg.decmp_line_sum_bit = oddma->dma_pqc_pair.line_sum_bit;
	od_dma_oddma_pq_cmp_pair_reg.cmp_qp_mode = oddma->dma_pqc_pair.qp_mode;
	od_dma_oddma_pq_decmp_pair_reg.decmp_qp_mode = oddma->dma_pqc_pair.qp_mode;
	od_dma_oddma_pq_cmp_pair_reg.two_line_prediction_en = oddma->dma_pqc_pair.two_line_predict;
	od_dma_oddma_pq_decmp_pair_reg.decmp_two_line_prediction_en = oddma->dma_pqc_pair.two_line_predict;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg, od_dma_oddma_pq_cmp_pair_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg, od_dma_oddma_pq_decmp_pair_reg.regValue);
	
	od_dma_oddma_pq_cmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_reg);
	od_dma_oddma_pq_decmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_reg);
	od_dma_oddma_pq_cmp_reg.cmp_height = oddma->dma_pqc_pair.height;
	od_dma_oddma_pq_cmp_reg.cmp_width_div32 = oddma->dma_pqc_pair.width_div32;
	od_dma_oddma_pq_decmp_reg.decmp_height = oddma->dma_pqc_pair.height;
	od_dma_oddma_pq_decmp_reg.decmp_width_div32 = oddma->dma_pqc_pair.width_div32;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg, od_dma_oddma_pq_cmp_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg, od_dma_oddma_pq_decmp_reg.regValue);
}
void drvif_color_od_dma_pqc_enable_set(unsigned char bypass_en, unsigned char pqc_enable)
{
	od_dma_oddma_pq_cmp_RBUS od_dma_oddma_pq_cmp_reg;
	od_dma_oddma_pq_decmp_RBUS od_dma_oddma_pq_decmp_reg;
	od_dma_oddma_pqc_pqdc_compensation_RBUS od_dma_oddma_pqc_pqdc_compensation_reg;
	
	// cmp decmp enable
	od_dma_oddma_pq_cmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_reg);
	od_dma_oddma_pq_decmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_reg);
	od_dma_oddma_pq_cmp_reg.cmp_en = pqc_enable;
	od_dma_oddma_pq_decmp_reg.decmp_en = pqc_enable;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg, od_dma_oddma_pq_cmp_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg, od_dma_oddma_pq_decmp_reg.regValue);

	// bypass pqc disable
	if(bypass_en == 0)
	{
		IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_cur_422_en_mask);
		IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_pre_422_en_mask);
		IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_pdest_en_mask);
	}
	od_dma_oddma_pqc_pqdc_compensation_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg);
	od_dma_oddma_pqc_pqdc_compensation_reg.comp_de_comp_bypass = bypass_en;
	IoReg_Write32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg, od_dma_oddma_pqc_pqdc_compensation_reg.regValue);
}
void drvif_color_od_freeze(unsigned char freeze)
{
	if(freeze)
	{
		/* force pre */
		IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_force_pre_out_mask); //_BIT20
		drvif_color_od_dma_cap_enable_set(0);
	}
	else
	{
		drvif_color_od_dma_cap_enable_set(1);		
		/* disable force pre */
		IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_force_pre_out_mask); //_BIT20
	}
}

// 160615 added by henry
// 160805 modified by henry
// 160810 modified by henry - add the static memory address
// 170524 modified by henry - Merlin3 driver
// 190327 modified by henry - Merlin5 PQC setting
// === Argument Explanation ===
//
// width & height: 4K - 3840x2160, 2K - 1920x1080
//
// bit_sel: dma input bit depth = 0 : 6 bits
//   						 	= 1 : 8 bits
//   						 	= 2 : 10 bits
//
// dma_mode = 1: frame mode
//        	  0: line mode
//
// FrameLimitBit: compression ratio
//
// dataColor = 0: RGB Compression
//			 = 1: YUV Compression
//
// dataFormat = 0: 444
//			  = 1: 422
//			  = 2: 420 --> not support RGB compression
//			  = 3: 400 --> not support RGB compression
//
// === Example of using OD PQ compression function ===
//
// Video Resolution   : 4K
// bit_sel            : 6
// DMA Mode           : line mode
// FrameLimitBit      : 8
// Compression Color  : RGB
// Compression Format : 422
//
// drvif_color_od_pqc(2160, 3840, 0, 0, 8, 0, 1)
//
bool drvif_color_od_pqc(UINT16 height, UINT16 width, UINT8 bit_sel, UINT8 dma_mode, UINT8 FrameLimitBit, int dataColor, int dataFormat)
{
	unsigned int od_size = 0;
	unsigned long od_addr = 0;
	unsigned long align_value =0;
	unsigned int pqc_line_sum_bit = 0;
	unsigned int burst_length = 0x20;

	od_dma_od_dma_ctrl_RBUS od_dma_od_dma_ctrl_reg;
	od_dma_od_dma_ctrl_2_RBUS od_dma_od_dma_ctrl_2_reg;
	od_dma_oddma_pq_cmp_allocate_RBUS od_dma_oddma_pq_cmp_allocate_reg;
	od_dma_oddma_pq_cmp_RBUS oddma_pq_cmp_reg;
	od_dma_oddma_pq_decmp_RBUS oddma_pq_decmp_reg;
	od_dma_oddma_pq_decmp_sat_en_RBUS oddma_pq_decmp_sat_en_reg;
	od_dma_oddma_wr_num_bl_wrap_ctl_RBUS od_dma_oddma_wr_num_bl_wrap_ctl_reg;
	od_dma_oddma_pq_cmp_enable_RBUS od_dma_oddma_pq_cmp_enable_reg;
 	od_dma_oddma_pq_cmp_balance_RBUS od_dma_oddma_pq_cmp_balance_reg;
	od_dma_oddma_pq_cmp_pair_RBUS oddma_pq_cmp_pair_reg;
	od_dma_oddma_pq_decmp_pair_RBUS oddma_pq_decmp_pair_reg;
	od_dma_oddma_pq_cmp_bit_RBUS oddma_pq_cmp_bit_reg;
	od_dma_oddma_rd_num_bl_wrap_ctl_RBUS od_dma_oddma_rd_num_bl_wrap_ctl_reg;
	od_dma_oddma_wr_num_bl_wrap_rolling_RBUS od_dma_oddma_wr_num_bl_wrap_rolling_reg;
	od_dma_oddma_rd_num_bl_wrap_rolling_RBUS od_dma_oddma_rd_num_bl_wrap_rolling_reg;
	od_dma_oddma_wr_num_bl_wrap_word_RBUS od_dma_oddma_wr_num_bl_wrap_word_reg;
	od_dma_oddma_rd_num_bl_wrap_word_RBUS od_dma_oddma_rd_num_bl_wrap_word_reg;
	od_dma_oddma_wr_num_bl_wrap_line_step_RBUS od_dma_oddma_cap_linestep_reg;
	od_dma_oddma_rd_num_bl_wrap_line_step_RBUS od_dma_oddma_rd_num_bl_wrap_line_step_reg;
	od_dma_oddma_pqc_pqdc_compensation_RBUS od_dma_oddma_pqc_pqdc_compensation_reg;
	od_dma_oddma_wr_ctrl_RBUS od_dma_oddma_wr_ctrl_reg;
	od_dma_oddma_rd_ctrl_RBUS od_dma_oddma_rd_ctrl_reg;

	VIPprintf("[OD] Gibi_od.cpp::drvif_color_od_pqc() calling...\n");

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	od_size = carvedout_buf_query_secure(CARVEDOUT_SCALER_OD,(void*) &od_addr);
#else
	od_size = carvedout_buf_query(CARVEDOUT_SCALER_OD,(void*) &od_addr);
#endif
	if (od_addr == 0 || od_size == 0) {
		VIPprintf("[%s %d] ERR : %lx %x\n", __func__, __LINE__, od_addr, od_size);
                return false;
	}
	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, PQC\n");
		return false;
	}
	// dataFormat = 0: 444
	//	 		  = 1: 422
	//	 		  = 2: 420 --> not support RGB compression
	//	 		  = 3: 400 --> not support RGB compression
	//
	// dataColor = 0: RGB Compression
	// 			 = 1: YUV Compression
	//
	if (dataColor == 0 && (dataFormat == 2 || dataFormat == 3))
	{
		VIPprintf("[OD] Gibi_od.cpp::input value conflict...");
		VIPprintf("420 or 400 doesn't support RGB compression\n");
		return false;
	}

	rtd_pr_vpq_info("phy addr=%lx,get size=%d,h=%d,w=%d,mode=%d,Bit=%d,dataColor=%d,dataFMT=%d,\n",
		od_addr, od_size, height, width, dma_mode, FrameLimitBit, dataColor, dataFormat);

	// b802cde4 dma ctrl
	od_dma_od_dma_ctrl_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
	od_dma_od_dma_ctrl_reg.bit_sel = 2; // Merlin5 constraint: comp_bypass=0, bit_sel=2
	od_dma_od_dma_ctrl_reg.pixel_encoding = 0; // Merlin5 constraint: comp_bypass=0, pixel_encoding=0
	od_dma_od_dma_ctrl_reg.oddma_mode_sel = dma_mode; // 0: line mode, 1: frame mode
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_reg, od_dma_od_dma_ctrl_reg.regValue);

	// b802cde8 vact and hact
	od_dma_od_dma_ctrl_2_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_2_reg);
	od_dma_od_dma_ctrl_2_reg.hact = width;
	od_dma_od_dma_ctrl_2_reg.vact = height;
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_2_reg, od_dma_od_dma_ctrl_2_reg.regValue);

	// b802cd14 PQC constraint
	od_dma_oddma_pq_cmp_allocate_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg);
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;
	// Merlin5
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_line = 4;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_less = 2;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_more = 2;
	// end Merlin5
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg, od_dma_oddma_pq_cmp_allocate_reg.regValue);

	// b802cd0c g_ratio
	od_dma_oddma_pq_cmp_enable_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg);
	od_dma_oddma_pq_cmp_enable_reg.g_ratio = 14;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg, od_dma_oddma_pq_cmp_enable_reg.regValue);

	// b802CD00 PQ_CMP enable
	oddma_pq_cmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_reg);
	oddma_pq_cmp_reg.cmp_en = 1;
	oddma_pq_cmp_reg.cmp_width_div32 = width >> 5;
	oddma_pq_cmp_reg.cmp_height = height;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg, oddma_pq_cmp_reg.regValue);

	// b802cd08 Compression ratio
	oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);
	oddma_pq_cmp_bit_reg.frame_limit_bit = FrameLimitBit;
	oddma_pq_cmp_bit_reg.block_limit_bit = 0x3F;

	// DMA buffer line limit clamp
	if(dma_mode == 0)
		oddma_pq_cmp_bit_reg.line_limit_bit = oddma_pq_cmp_bit_reg.frame_limit_bit;
	else if(dma_mode == 1)
		oddma_pq_cmp_bit_reg.line_limit_bit = FrameLimitBit + 6;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BIT_reg, oddma_pq_cmp_bit_reg.regValue);

	// b802cd80 PQ_DECMP enable
	oddma_pq_decmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_reg);
	oddma_pq_decmp_reg.decmp_en = 1;
	oddma_pq_decmp_reg.decmp_width_div32 = width >> 5;
	oddma_pq_decmp_reg.decmp_height = height;
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg, oddma_pq_decmp_reg.regValue);

	// cmp pair b802cd04, decmp pair b802cd84 setting
	oddma_pq_cmp_pair_reg.regValue =  IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg);
	oddma_pq_decmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg);
	oddma_pq_cmp_pair_reg.cmp_data_format = dataFormat;
	oddma_pq_decmp_pair_reg.decmp_data_format = dataFormat;
	oddma_pq_cmp_pair_reg.cmp_data_color = dataColor;
	oddma_pq_decmp_pair_reg.decmp_data_color = dataColor;

	if(FrameLimitBit <= 6) {
		oddma_pq_cmp_pair_reg.cmp_data_bit_width = 0;
		oddma_pq_decmp_pair_reg.decmp_data_bit_width = 0;
	} else {
		oddma_pq_cmp_pair_reg.cmp_data_bit_width = 1;
		oddma_pq_decmp_pair_reg.decmp_data_bit_width = 1;
	}

	// Merlin5
	oddma_pq_cmp_pair_reg.cmp_dic_mode_en = 1;
	oddma_pq_cmp_pair_reg.cmp_jump4_en = 1;
	oddma_pq_decmp_pair_reg.decmp_dic_mode_en = 1;
	oddma_pq_decmp_pair_reg.decmp_jump4_en = 1;
	od_dma_oddma_pq_cmp_enable_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg);
	od_dma_oddma_pq_cmp_enable_reg.first_predict_nc_mode = 1;
	od_dma_oddma_pq_cmp_enable_reg.variation_maxmin_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg, od_dma_oddma_pq_cmp_enable_reg.regValue);
	od_dma_oddma_pq_cmp_balance_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BALANCE_reg);
	od_dma_oddma_pq_cmp_balance_reg.balance_rb_ov_th = 0;
	od_dma_oddma_pq_cmp_balance_reg.balance_rb_ud_th = 3;
	od_dma_oddma_pq_cmp_balance_reg.balance_rb_give = 3;
	od_dma_oddma_pq_cmp_balance_reg.balance_g_ov_th = 0;
	od_dma_oddma_pq_cmp_balance_reg.balance_g_ud_th = 3;
	od_dma_oddma_pq_cmp_balance_reg.balance_g_give = 3;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BALANCE_reg, od_dma_oddma_pq_cmp_balance_reg.regValue);
	// end Merlin5

	pqc_line_sum_bit = (width * oddma_pq_cmp_bit_reg.line_limit_bit + 256) / 128;

#ifdef CONFIG_BW_96B_ALIGNED
	// for cap/disp line_step (128 bits unit)
    align_value = drvifget_align_value(pqc_line_sum_bit, 6);
            burst_length = 0x30; //must be check 96 alignment,

#else
    align_value = pqc_line_sum_bit;
            burst_length = 0x20;
#endif
	if(dma_mode == 0) {
		oddma_pq_cmp_pair_reg.cmp_line_sum_bit = pqc_line_sum_bit;
		oddma_pq_decmp_pair_reg.decmp_line_sum_bit = pqc_line_sum_bit;
	}
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg, oddma_pq_cmp_pair_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg, oddma_pq_decmp_pair_reg.regValue);

	// b802cd88 Saturation enable
	oddma_pq_decmp_sat_en_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_SAT_EN_reg);
	oddma_pq_decmp_sat_en_reg.saturation_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_SAT_EN_reg, oddma_pq_decmp_sat_en_reg.regValue);

	// b802ce3c b802cebc cap disp line_num, burst_length //must be check 96 alignment
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg);
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.cap_burst_len = burst_length;
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.cap_line_num = height;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg, od_dma_oddma_wr_num_bl_wrap_ctl_reg.regValue);
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg);
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.disp_burst_len = burst_length;
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.disp_line_num = height;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg, od_dma_oddma_rd_num_bl_wrap_ctl_reg.regValue);

	od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg);
	od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg);
	od_dma_oddma_wr_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
	od_dma_oddma_rd_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);

	if(dma_mode == 0) { // LINE MODE
		// b802cd0c Merlin3 first_line_more_en = 0 when line mode
		od_dma_oddma_pq_cmp_enable_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg);
		od_dma_oddma_pq_cmp_enable_reg.fisrt_line_more_en = 0;
		//od_dma_oddma_pq_cmp_enable_reg.force_last2blk_pe_mode0 = 0;
		od_dma_oddma_pq_cmp_enable_reg.guarantee_max_qp_en = 0;
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg, od_dma_oddma_pq_cmp_enable_reg.regValue);

		// b802ce38 Cap LineStep //must be check 96 alignment
		od_dma_oddma_cap_linestep_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
		od_dma_oddma_cap_linestep_reg.cap_line_step = align_value; // 128 bits
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg, od_dma_oddma_cap_linestep_reg.regValue);

		// b802ceb8 Disp LineStep //must be check 96 alignment
		od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
		od_dma_oddma_rd_num_bl_wrap_line_step_reg.disp_line_step = align_value; // 128 bits
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg, od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue);

		// b802ce44 cap_line_128_num
		od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = pqc_line_sum_bit;

		// b802cec4 disp_line_128_num
		od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = pqc_line_sum_bit;

		// b802ce40 b802cec0 cap disp line rolling space
		od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_space = height + 2;
		od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_space = height + 2;
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg, od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg, od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue);

		// b802ce40 b802cec0 cap disp line rolling enable
		od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_enable = 1;
		od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_enable = 1;

		// b802cd04 b802cd84 cmp decmp line mode en
		oddma_pq_cmp_pair_reg.regValue =  IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg);
		oddma_pq_decmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg);
		oddma_pq_cmp_pair_reg.cmp_line_mode = 1;
		oddma_pq_decmp_pair_reg.decmp_line_mode = 1;
		oddma_pq_cmp_pair_reg.cmp_400_old_mode_en = 0;
		oddma_pq_decmp_pair_reg.decmp_400_old_mode_en = 0;
		oddma_pq_cmp_pair_reg.two_line_prediction_en = 1;
		oddma_pq_decmp_pair_reg.decmp_two_line_prediction_en = 1;
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg, oddma_pq_cmp_pair_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg, oddma_pq_decmp_pair_reg.regValue);
	}
	else if(dma_mode == 1) { // FRAME MODE
		// b802ce40 cap line rolling space and enable
		od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_enable = 0;

		// b802cec0 disp line rolling space and enable
		od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_enable = 0;

		// b802ce44 cap_line_128_num = 0xffff
		od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = 0xffff;

		// b802cec4 disp_line_128_num = 0xffff
		od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = 0xffff;

	}
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg, od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg, od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg, od_dma_oddma_wr_num_bl_wrap_word_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg, od_dma_oddma_rd_num_bl_wrap_word_reg.regValue);



#ifdef CONFIG_BW_96B_ALIGNED
    align_value = dvr_memory_alignment(od_addr, dvr_size_alignment((unsigned long)height*width*FrameLimitBit)); //96 Alignment for Bandwidth Request
    if (align_value == 0)
        BUG();
#else
    align_value = od_addr; //origin value
#endif

	rtd_pr_vpq_info("aligned phy addr=%lx,\n", align_value);

    //burst_length = burst_length << 4; // match register reserved issue
    //od_size = od_size << 4; // match register reserved issue
	// b802ce30 b802ce34 b802ced4 b802ced8 start address
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, align_value + burst_length); //must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_up_reg, od_addr + od_size - burst_length);
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_low_reg, od_addr);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value + burst_length);//must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_up_reg, od_addr + od_size - burst_length);
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_low_reg, od_addr);

	// pqc pqdc bypass disable
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_cur_422_en_mask);
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_pre_422_en_mask);
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_pdest_en_mask);
	od_dma_oddma_pqc_pqdc_compensation_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg);
	od_dma_oddma_pqc_pqdc_compensation_reg.comp_de_comp_bypass = 0;
	IoReg_Write32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg, od_dma_oddma_pqc_pqdc_compensation_reg.regValue);

	// b802ce08 cap enable
	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_wr_ctrl_reg.cap_en = 1;
	od_dma_oddma_wr_ctrl_reg.cap_max_outstanding = 3;
	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg, od_dma_oddma_wr_ctrl_reg.regValue);

	// b802ce88 disp enable
	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	od_dma_oddma_rd_ctrl_reg.disp_en = 1;
	od_dma_oddma_rd_ctrl_reg.disp_max_outstanding = 3; // Merlin4 - add for better performance of od dma bandwidth
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg, od_dma_oddma_rd_ctrl_reg.regValue);

	od_dma_oddma_pqc_pqdc_compensation_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg);
	od_dma_oddma_pqc_pqdc_compensation_reg.tr_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg, od_dma_oddma_pqc_pqdc_compensation_reg.regValue);
	// end henry

	VIPprintf("[OD] Gibi_od.cpp::drvif_color_od_pqc() finished...\n");

	return true;
}
// 190417 added by henry - Merlin5 120hz PQC setting
// 120hz only support y mode, 422
bool drvif_color_od_pqc_120hz(UINT16 height, UINT16 width, UINT8 bit_sel, UINT8 dma_mode, UINT8 FrameLimitBit, int dataColor, int dataFormat)
{
	unsigned int od_size = 0;
	unsigned long od_addr = 0;
	unsigned long align_value =0;
	unsigned int pqc_line_sum_bit = 0;
	unsigned int burst_length = 0x20;
	enum PLAFTORM_TYPE platform = get_platform();

	od_dma_od_dma_ctrl_RBUS od_dma_od_dma_ctrl_reg;
	od_dma_od_dma_ctrl_2_RBUS od_dma_od_dma_ctrl_2_reg;
	od_dma_oddma_pq_cmp_allocate_RBUS od_dma_oddma_pq_cmp_allocate_reg;
	od_dma_oddma_pq_cmp_RBUS oddma_pq_cmp_reg;
	od_dma_oddma_pq_decmp_RBUS oddma_pq_decmp_reg;
	od_dma_oddma_pq_decmp_sat_en_RBUS oddma_pq_decmp_sat_en_reg;
	od_dma_oddma_wr_num_bl_wrap_ctl_RBUS od_dma_oddma_wr_num_bl_wrap_ctl_reg;
	od_dma_oddma_pq_cmp_enable_RBUS od_dma_oddma_pq_cmp_enable_reg;
	od_dma_oddma_pq_cmp_balance_RBUS od_dma_oddma_pq_cmp_balance_reg;
	od_dma_oddma_pq_cmp_pair_RBUS oddma_pq_cmp_pair_reg;
	od_dma_oddma_pq_decmp_pair_RBUS oddma_pq_decmp_pair_reg;
	od_dma_oddma_pq_cmp_bit_RBUS oddma_pq_cmp_bit_reg;
	od_dma_oddma_rd_num_bl_wrap_ctl_RBUS od_dma_oddma_rd_num_bl_wrap_ctl_reg;
	od_dma_oddma_wr_num_bl_wrap_rolling_RBUS od_dma_oddma_wr_num_bl_wrap_rolling_reg;
	od_dma_oddma_rd_num_bl_wrap_rolling_RBUS od_dma_oddma_rd_num_bl_wrap_rolling_reg;
	od_dma_oddma_wr_num_bl_wrap_word_RBUS od_dma_oddma_wr_num_bl_wrap_word_reg;
	od_dma_oddma_rd_num_bl_wrap_word_RBUS od_dma_oddma_rd_num_bl_wrap_word_reg;
	od_dma_oddma_wr_num_bl_wrap_line_step_RBUS od_dma_oddma_cap_linestep_reg;
	od_dma_oddma_rd_num_bl_wrap_line_step_RBUS od_dma_oddma_rd_num_bl_wrap_line_step_reg;
	od_dma_oddma_pqc_pqdc_compensation_RBUS od_dma_oddma_pqc_pqdc_compensation_reg;
	od_dma_oddma_wr_ctrl_RBUS od_dma_oddma_wr_ctrl_reg;
	od_dma_oddma_rd_ctrl_RBUS od_dma_oddma_rd_ctrl_reg;
	od_od_ctrl_RBUS od_ctrl_reg;

	VIPprintf("[OD] Gibi_od.cpp::drvif_color_od_pqc_120hz() calling...\n");

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	od_size = carvedout_buf_query_secure(CARVEDOUT_SCALER_OD,(void*) &od_addr);
#else
	od_size = carvedout_buf_query(CARVEDOUT_SCALER_OD,(void*) &od_addr);
#endif
	if (od_addr == 0 || od_size == 0) {
		VIPprintf("[%s %d] ERR : %lx %x\n", __func__, __LINE__, od_addr, od_size);
				return false;
	}

	// dataFormat = 0: 444
	//			  = 1: 422 --> 120hz only support this format
	//			  = 2: 420
	//			  = 3: 400
	//
	// dataColor = 0: RGB Compression
	//			 = 1: YUV Compression --> 120hz only support this format
	if (dataColor != 1 || dataFormat != 1)
	{
		VIPprintf("[OD] Gibi_od.cpp::input value conflict...");
		VIPprintf("OD 120hz only support y mode, 422 !\n");
		return false;
	}
	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, PQC120\n");
		return false;
	}
	rtd_pr_vpq_info("phy addr=%lx,get size=%d,h=%d,w=%d,mode=%d,Bit=%d,dataColor=%d,dataFMT=%d,\n",
		od_addr, od_size, height, width, dma_mode, FrameLimitBit, dataColor, dataFormat);

	od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	// 120 hz
	if (platform != PLATFORM_KXLP)
	{
		od_ctrl_reg.oddma_clk_sel = 1;
		od_ctrl_reg.dma_1p_2p_mode = 1;
	}
	IoReg_Write32(OD_OD_CTRL_reg, od_ctrl_reg.regValue);

	// b802cde4 dma ctrl
	od_dma_od_dma_ctrl_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
	od_dma_od_dma_ctrl_reg.bit_sel = 2; // Merlin5 constraint: comp_bypass=0, bit_sel=2
	od_dma_od_dma_ctrl_reg.pixel_encoding = 0; // Merlin5 constraint: comp_bypass=0, pixel_encoding=0
	od_dma_od_dma_ctrl_reg.oddma_mode_sel = dma_mode; // 0: line mode, 1: frame mode
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_reg, od_dma_od_dma_ctrl_reg.regValue);

	// b802cde8 vact and hact
	od_dma_od_dma_ctrl_2_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_2_reg);
	od_dma_od_dma_ctrl_2_reg.hact = width;
	od_dma_od_dma_ctrl_2_reg.vact = height;
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_2_reg, od_dma_od_dma_ctrl_2_reg.regValue);

	// b802cd14 PQC constraint
	od_dma_oddma_pq_cmp_allocate_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg);
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;
	// Merlin5
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_line = 4;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_less = 2;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_more = 2;
	// end Merlin5
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg, od_dma_oddma_pq_cmp_allocate_reg.regValue);

	// b802cd0c g_ratio
	od_dma_oddma_pq_cmp_enable_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg);
	od_dma_oddma_pq_cmp_enable_reg.g_ratio = 14;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg, od_dma_oddma_pq_cmp_enable_reg.regValue);

	// b802CD00 PQ_CMP enable
	oddma_pq_cmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_reg);
	oddma_pq_cmp_reg.cmp_en = 1;
	oddma_pq_cmp_reg.cmp_width_div32 = width >> 5;
	oddma_pq_cmp_reg.cmp_height = height;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg, oddma_pq_cmp_reg.regValue);

	// b802cd08 Compression ratio
	oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);
	oddma_pq_cmp_bit_reg.frame_limit_bit = FrameLimitBit;
	oddma_pq_cmp_bit_reg.block_limit_bit = 0x3F;

	// DMA buffer line limit clamp
	if(dma_mode == 0)
		oddma_pq_cmp_bit_reg.line_limit_bit = oddma_pq_cmp_bit_reg.frame_limit_bit;
	else if(dma_mode == 1)
		oddma_pq_cmp_bit_reg.line_limit_bit = FrameLimitBit + 6;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BIT_reg, oddma_pq_cmp_bit_reg.regValue);

	// b802cd80 PQ_DECMP enable
	oddma_pq_decmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_reg);
	oddma_pq_decmp_reg.decmp_en = 1;
	oddma_pq_decmp_reg.decmp_width_div32 = width >> 5;
	oddma_pq_decmp_reg.decmp_height = height;
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg, oddma_pq_decmp_reg.regValue);

	// cmp pair b802cd04, decmp pair b802cd84 setting
	oddma_pq_cmp_pair_reg.regValue =  IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg);
	oddma_pq_decmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg);
	oddma_pq_cmp_pair_reg.cmp_data_format = dataFormat; // 120hz
	oddma_pq_decmp_pair_reg.decmp_data_format = dataFormat; // 120hz
	oddma_pq_cmp_pair_reg.cmp_data_color = dataColor; // 120hz
	oddma_pq_decmp_pair_reg.decmp_data_color = dataColor; // 120hz
	oddma_pq_cmp_pair_reg.cmp_data_bit_width = 1; // 120hz
	oddma_pq_decmp_pair_reg.decmp_data_bit_width = 1; // 120hz
	oddma_pq_cmp_pair_reg.cmp_qp_mode = 1; // 120hz
	oddma_pq_decmp_pair_reg.decmp_qp_mode= 1; // 120hz

	// Merlin5
	oddma_pq_cmp_pair_reg.cmp_dic_mode_en = 1;
	oddma_pq_cmp_pair_reg.cmp_jump4_en = 1;
	oddma_pq_decmp_pair_reg.decmp_dic_mode_en = 1;
	oddma_pq_decmp_pair_reg.decmp_jump4_en = 1;
	od_dma_oddma_pq_cmp_enable_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg);
	od_dma_oddma_pq_cmp_enable_reg.first_predict_nc_mode = 1;
	od_dma_oddma_pq_cmp_enable_reg.variation_maxmin_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg, od_dma_oddma_pq_cmp_enable_reg.regValue);
	od_dma_oddma_pq_cmp_balance_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BALANCE_reg);
	od_dma_oddma_pq_cmp_balance_reg.balance_rb_ov_th = 0;
	od_dma_oddma_pq_cmp_balance_reg.balance_rb_ud_th = 3;
	od_dma_oddma_pq_cmp_balance_reg.balance_rb_give = 3;
	od_dma_oddma_pq_cmp_balance_reg.balance_g_ov_th = 0;
	od_dma_oddma_pq_cmp_balance_reg.balance_g_ud_th = 3;
	od_dma_oddma_pq_cmp_balance_reg.balance_g_give = 3;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BALANCE_reg, od_dma_oddma_pq_cmp_balance_reg.regValue);
	// end Merlin5

	pqc_line_sum_bit = (width * oddma_pq_cmp_bit_reg.line_limit_bit + 256) / 128 + ((width * oddma_pq_cmp_bit_reg.line_limit_bit + 256)%128!=0);

#ifdef CONFIG_BW_96B_ALIGNED
	// for cap line_step (128 bits unit)
    align_value = drvifget_align_value(pqc_line_sum_bit, 6);
	burst_length = 0x3c; //must be check 96 alignment,

#else
    align_value = pqc_line_sum_bit;
	burst_length = 0x20;
#endif
	if(dma_mode == 0) {
		oddma_pq_cmp_pair_reg.cmp_line_sum_bit = align_value;
		oddma_pq_decmp_pair_reg.decmp_line_sum_bit = align_value;
	}
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg, oddma_pq_cmp_pair_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg, oddma_pq_decmp_pair_reg.regValue);

	// b802cd88 Saturation enable
	oddma_pq_decmp_sat_en_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_SAT_EN_reg);
	oddma_pq_decmp_sat_en_reg.saturation_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_SAT_EN_reg, oddma_pq_decmp_sat_en_reg.regValue);

	// b802ce3c b802cebc cap disp line_num, burst_length //must be check 96 alignment
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg);
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.cap_burst_len = burst_length;
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.cap_line_num = height;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg, od_dma_oddma_wr_num_bl_wrap_ctl_reg.regValue);
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg);
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.disp_burst_len = burst_length;
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.disp_line_num = height;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg, od_dma_oddma_rd_num_bl_wrap_ctl_reg.regValue);

	od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg);
	od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg);
	od_dma_oddma_wr_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
	od_dma_oddma_rd_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);

	if(dma_mode == 0) { // LINE MODE
		// b802cd0c Merlin3 first_line_more_en = 0 when line mode
		od_dma_oddma_pq_cmp_enable_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg);
		od_dma_oddma_pq_cmp_enable_reg.fisrt_line_more_en = 0;
		//od_dma_oddma_pq_cmp_enable_reg.force_last2blk_pe_mode0 = 0;
		od_dma_oddma_pq_cmp_enable_reg.guarantee_max_qp_en = 0;
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg, od_dma_oddma_pq_cmp_enable_reg.regValue);

		// b802ce38 Cap LineStep //must be check 96 alignment
		od_dma_oddma_cap_linestep_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
		od_dma_oddma_cap_linestep_reg.cap_line_step = align_value; // 128 bits
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg, od_dma_oddma_cap_linestep_reg.regValue);

		// b802ceb8 Disp LineStep //must be check 96 alignment
		od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
		od_dma_oddma_rd_num_bl_wrap_line_step_reg.disp_line_step = align_value; // 128 bits
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg, od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue);

		// b802ce44 cap_line_128_num
		od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = align_value;

		// b802cec4 disp_line_128_num
		od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = align_value;

		// b802ce40 b802cec0 cap disp line rolling space
		od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_space = height + 1;
		od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_space = height + 1;
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg, od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg, od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue);

		// b802ce40 b802cec0 cap disp line rolling enable
		od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_enable = 1;
		od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_enable = 1;

		// b802cd04 b802cd84 cmp decmp line mode en
		oddma_pq_cmp_pair_reg.regValue =  IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg);
		oddma_pq_decmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg);
		oddma_pq_cmp_pair_reg.cmp_line_mode = 1;
		oddma_pq_decmp_pair_reg.decmp_line_mode = 1;
		oddma_pq_cmp_pair_reg.cmp_400_old_mode_en = 0;
		oddma_pq_decmp_pair_reg.decmp_400_old_mode_en = 0;
		oddma_pq_cmp_pair_reg.two_line_prediction_en = 1;
		oddma_pq_decmp_pair_reg.decmp_two_line_prediction_en = 1;
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg, oddma_pq_cmp_pair_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg, oddma_pq_decmp_pair_reg.regValue);
	}
	else if(dma_mode == 1) { // FRAME MODE
		// b802ce40 cap line rolling space and enable
		od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_enable = 0;

		// b802cec0 disp line rolling space and enable
		od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_enable = 0;

		// b802ce44 cap_line_128_num = 0xffff
		od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = 0xffff;

		// b802cec4 disp_line_128_num = 0xffff
		od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = 0xffff;

	}
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg, od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg, od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg, od_dma_oddma_wr_num_bl_wrap_word_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg, od_dma_oddma_rd_num_bl_wrap_word_reg.regValue);



#ifdef CONFIG_BW_96B_ALIGNED
    align_value = dvr_memory_alignment((unsigned long)od_addr, dvr_size_alignment((unsigned long)height*width*FrameLimitBit)); //96 Alignment for Bandwidth Request
    if (align_value == 0)
        BUG();
#else
    align_value = od_addr; //origin value
#endif

	rtd_pr_vpq_info("aligned phy addr=%lx,\n", align_value);

    //burst_length = burst_length << 4; // match register reserved issue
    //od_size = od_size << 4; // match register reserved issue
	// b802ce30 b802ce34 b802ced4 b802ced8 start address
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, align_value + burst_length); //must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_up_reg, od_addr + od_size - burst_length);
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_low_reg, od_addr);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, align_value + burst_length);//must be check 96 alignment
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_up_reg, od_addr + od_size - burst_length);
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_low_reg, od_addr);

	// pqc pqdc bypass disable
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_cur_422_en_mask);
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_pre_422_en_mask);
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_pdest_en_mask);
	od_dma_oddma_pqc_pqdc_compensation_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg);
	od_dma_oddma_pqc_pqdc_compensation_reg.comp_de_comp_bypass = 0;
	IoReg_Write32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg, od_dma_oddma_pqc_pqdc_compensation_reg.regValue);

	// b802ce08 cap enable
	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_wr_ctrl_reg.cap_en = 1;
	od_dma_oddma_wr_ctrl_reg.cap_max_outstanding = 3;
	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg, od_dma_oddma_wr_ctrl_reg.regValue);

	// b802ce88 disp enable
	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	od_dma_oddma_rd_ctrl_reg.disp_en = 1;
	od_dma_oddma_rd_ctrl_reg.disp_max_outstanding = 3; // Merlin4 - add for better performance of od dma bandwidth
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg, od_dma_oddma_rd_ctrl_reg.regValue);

	od_dma_oddma_pqc_pqdc_compensation_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg);
	od_dma_oddma_pqc_pqdc_compensation_reg.tr_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg, od_dma_oddma_pqc_pqdc_compensation_reg.regValue);
	// end henry

	VIPprintf("[OD] Gibi_od.cpp::drvif_color_od_pqc_120hz() finished...\n");

	return true;
}
#if 0
// OD IP enable
void drvif_color_od(unsigned char bOD)
{
	od_od_ctrl_RBUS od_ctrl_reg;
	od_od_ctrl2_RBUS od_ctrl2_reg;
	od_od_noise_reduction_ctrl_RBUS noise_reduction_ctrl_reg;

	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, OD\n");
		return;
	}

	od_ctrl2_reg.regValue = IoReg_Read32(OD_OD_CTRL2_reg);
	od_ctrl2_reg.od_delta_gain_r = 64-4;
	od_ctrl2_reg.od_delta_gain_g = 64-4;
	od_ctrl2_reg.od_delta_gain_b = 64-4;
	od_ctrl2_reg.od_delta_gain_w = 64-4;
	IoReg_Write32(OD_OD_CTRL2_reg, od_ctrl2_reg.regValue);

	/* default nr*/
	noise_reduction_ctrl_reg.regValue = IoReg_Read32(OD_od_Noise_reduction_ctrl_reg);
	noise_reduction_ctrl_reg.od_nr_mode = 1;
	noise_reduction_ctrl_reg.od_b_nr_en = 1;
	noise_reduction_ctrl_reg.od_g_nr_en = 1;
	noise_reduction_ctrl_reg.od_r_nr_en = 1;
	noise_reduction_ctrl_reg.od_b_nr_thd = 3;
	noise_reduction_ctrl_reg.od_g_nr_thd = 3;
	noise_reduction_ctrl_reg.od_r_nr_thd = 3;
	IoReg_Write32(OD_od_Noise_reduction_ctrl_reg, noise_reduction_ctrl_reg.regValue);

	od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	if (bOD == 0) {
		od_ctrl_reg.od_en = 0;
	} else {
		od_ctrl_reg.od_en = 1;
	}
	IoReg_Write32(OD_OD_CTRL_reg, od_ctrl_reg.regValue);

}
#endif
DRV_od_lut_ctrl* drvif_color_od_access_lut_struct(bool recursive)
{
	if(recursive)
		return &drv_od_rec_lut;
	else
		return &drv_od_lut;
}

// === arguement ===
// tableType, 0:delta table, 1:target table
// color channel, 0:red, 1:green, 2:blue, 3:rgb, 4:white
//void drvif_color_od_table_seperate(unsigned int *pODtable, unsigned char tableType, unsigned char colorChannel)
void drvif_color_od_table_17x17_set(unsigned char *pODtable, unsigned char tableType, unsigned char colorChannel)
{
	int i ;
	unsigned char tmp;
	od_od_lut_addr_RBUS lut_addr_reg;

	tmp = drvif_color_od_enable_get();
	drvif_color_od_enable_set(0);

	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.table_type = tableType;
	lut_addr_reg.od_lut_ax_en = 1;
	lut_addr_reg.od_lut_ax_mode = 0;
	lut_addr_reg.od_lut_adr_mode = 0; //col(cur) indexfirst
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_row_addr = 0;
	// 180209 henry
	// 255 means called by rtice, and rtice's button will set this item
	if(colorChannel!=255)
		lut_addr_reg.od_lut_sel = colorChannel;
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

	for (i = 0; i < OD_table_length; i++) {
		if(i%17!=16) {
			VIPprintf("[OD][write] pre:%d cur:%d %d\n",i/17,i%17,*(pODtable+i)+(i%17)*16);
		}
		else {
			VIPprintf("[OD][write] pre:%d cur:%d %d\n",i/17,i%17,*(pODtable+i)+255);
		}
		IoReg_Write32(OD_OD_LUT_DATA_reg, *(pODtable+i));
	}

	/* disable table access*/
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, OD_OD_LUT_ADDR_od_lut_ax_en_mask); // _BIT16

	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, tmp = 0, table17 set\n");
		tmp = 0;
	}

	rtd_pr_vpq_info("%s, set 17x17 table finished\n", __func__);

	drvif_color_od_enable_set(tmp);
}

void drvif_color_od_table_17x17_get(unsigned char *pODtable)
{
	// CAUTION: this function always returns target table
	
    int i;
	unsigned char tmp;
	unsigned char remain;
	od_od_lut_addr_RBUS lut_addr_reg;
	unsigned int ODtable_tmp[OD_table_length];
	char ODtable_sign[OD_table_length];

	tmp = drvif_color_od_enable_get();
	drvif_color_od_enable_set(0);

	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.od_lut_row_addr = 0;
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_ax_en = 1; // auto rolling index
	lut_addr_reg.od_lut_adr_mode = 0; //cur index first
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

	// target table
	if(lut_addr_reg.table_type)
	{
		for (i = 0; i < OD_table_length; i++){
			RBus_UInt32 regValue_tmp = IoReg_Read32(OD_OD_LUT_DATA_reg);
			// 33 to 17 table
			//*(pODtable+i) = ( regValue_tmp >> 24);

			/* henry merlin3 OD table only 17x17 */
			*(pODtable+i) = regValue_tmp;
		}
	}
	// delta table
	else
	{

		for (i = 0; i < OD_table_length; i++) {
			ODtable_tmp[i] = IoReg_Read32(OD_OD_LUT_DATA_reg);

			remain = i%17;

			ODtable_sign[i] = (char)(ODtable_tmp[i]);

			// add cur pixel value (to target table)
			if(remain != 16)
				*(pODtable+i) = ODtable_sign[i] + 16 * remain;
			else
				*(pODtable+i) = ODtable_sign[i] + 255;

			VIPprintf("[OD][read]%d %d %d %d\n",i/17,i%17,*(pODtable+i), IoReg_Read32(OD_OD_LUT_DATA_reg));
		}

	}

	/* disable table access*/
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, OD_OD_LUT_ADDR_od_lut_ax_en_mask); // _BIT16

	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, tmp = 0, table17 get\n");
		tmp = 0;
	}

	rtd_pr_vpq_info("%s, get 17x17 table finished\n", __func__);

	drvif_color_od_enable_set(tmp);

}
void drvif_color_od_table_33x33_set(unsigned char *pODtable, unsigned char tableType, unsigned char colorChannel)
{
	int i;
	unsigned char tmp;
	od_od_lut_addr_RBUS lut_addr_reg;
	
	tmp = drvif_color_od_enable_get();
	drvif_color_od_enable_set(0);
	
	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.table_type = tableType;
	lut_addr_reg.od_lut_ax_en = 1;
	lut_addr_reg.od_lut_ax_mode = 0;
	lut_addr_reg.od_lut_adr_mode = 0; //col(cur) indexfirst
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_row_addr = 0;
	
	// 255 means called by rtice, and rtice's button will set this item
	if(colorChannel!=255)
		lut_addr_reg.od_lut_sel = colorChannel;
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

	drif_color_od_lut_index_get(0, (unsigned char *)&(drv_od_lut.pre_idx), (unsigned char *)&(drv_od_lut.cur_idx));

	for (i = 0; i < OD_table_length; i++) {
		#if 0
		if(i%33!=32) {
			VIPprintf("[OD][write] pre:%d cur:%d %d\n",i/33,i%33,*(pODtable+i)+(i%33)*8);
		}
		else {
			VIPprintf("[OD][write] pre:%d cur:%d %d\n",i/33,i%33,*(pODtable+i)+255);
		}
		#else
		// programable indices
		VIPprintf("[OD][write] pre:%d cur:%d %d\n",drv_od_lut.pre_idx[i/33],drv_od_lut.cur_idx[i%33],*(pODtable+i)+drv_od_lut.cur_idx[i%33]);
		#endif
		IoReg_Write32(OD_OD_LUT_DATA_reg, *(pODtable+i));
	}
	
	/* disable table access*/
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, OD_OD_LUT_ADDR_od_lut_ax_en_mask); // _BIT16

	drif_color_od_lut_struct_value_set(od_target_table_g);

	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, tmp = 0, table33 set\n");
		tmp = 0;
	}

	rtd_pr_vpq_info("%s, set 33x33 table finished\n", __func__);

	drvif_color_od_enable_set(tmp);
}
void drvif_color_od_table_33x33_get(bool recursive, unsigned char *pODtable)
{
	// CAUTION: this function always returns target table
	
    int i;
	unsigned char tmp;
	unsigned char remain;
	od_od_lut_addr_RBUS lut_addr_reg;
	unsigned int ODtable_tmp[OD_table_length];
	char ODtable_sign[OD_table_length];
	DRV_od_lut_ctrl *od_lut_local = drvif_color_od_access_lut_struct(recursive);

	tmp = drvif_color_od_enable_get();
	drvif_color_od_enable_set(0);

	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.od_lut_row_addr = 0;
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_ax_en = 1; // auto rolling index
	lut_addr_reg.od_lut_adr_mode = 0; //cur index first
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

	// target table
	if(lut_addr_reg.table_type)
	{
		for (i = 0; i < OD_table_length; i++){
			UINT32 regValue_tmp = IoReg_Read32(OD_OD_LUT_DATA_reg);
			
			*(pODtable+i) = (UINT8)regValue_tmp;
		}
	}
	// delta table
	else
	{
		drif_color_od_lut_index_get(recursive, (unsigned char *)&(od_lut_local->pre_idx), (unsigned char *)&(od_lut_local->cur_idx));

		for (i = 0; i < OD_table_length; i++) {
			ODtable_tmp[i] = IoReg_Read32(OD_OD_LUT_DATA_reg);

			remain = i%33;
			
			ODtable_sign[i] = (char)(ODtable_tmp[i]);

			// add cur pixel value (to target table)
			#if 0
			if(remain != 32)
				*(pODtable+i) = ODtable_sign[i] + 8 * remain;
			else
				*(pODtable+i) = ODtable_sign[i] + 255;
			VIPprintf("[OD][read]%d %d %d %d\n",i/33,i%33,*(pODtable+i), IoReg_Read32(OD_OD_LUT_DATA_reg));
			#else
			// programable indices
			*(pODtable+i) = ODtable_sign[i] + od_lut_local->cur_idx[remain];
			
			VIPprintf("[OD][read]%d %d %d %d\n",od_lut_local->pre_idx[i/33],od_lut_local->cur_idx[remain],*(pODtable+i), IoReg_Read32(OD_OD_LUT_DATA_reg));
			#endif
		}
	}

	/* disable table access*/
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, OD_OD_LUT_ADDR_od_lut_ax_en_mask); // _BIT16

	drif_color_od_lut_struct_value_set(pODtable);

	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, tmp = 0, table33 get\n");
		tmp = 0;
	}

	rtd_pr_vpq_info("%s, get 33x33 table finished\n", __func__);

	drvif_color_od_enable_set(tmp);
}

void drvif_color_od_table_33x33_set_dbus(unsigned char tableType, unsigned int lut_addr, unsigned char colorChannel)
{
	//unsigned char tmp;
	od_od_lut_addr_RBUS lut_addr_reg;

	//tmp = drvif_color_od_enable_get_verify();
	//drvif_color_od_enable_set_verify(0);

	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.od_lut_dma_read = 1; // 0: rbus, 1:dbus
	lut_addr_reg.table_type = tableType; // 0: delta, 1: target
	lut_addr_reg.od_lut_dma_sel = colorChannel; // 0: sram0~2, 1: sram3~5, 2: sram0~5
	lut_addr_reg.od_lut_ax_en = 1;
	lut_addr_reg.od_lut_ax_mode = 0;
	lut_addr_reg.od_lut_adr_mode = 0; // col(cur) indexfirst
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_row_addr = 0;
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

	// set dbus lut address
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_table_reg, lut_addr);

	/* disable table access*/
	lut_addr_reg.od_lut_ax_en = 0;
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

}

void drif_color_od_lut_struct_value_set(unsigned char *pODtable)
{
	/* update target table to struct */

	od_od_lut_addr_RBUS lut_addr_reg;
	
	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);

	/* copy table to lut struct */
	switch(lut_addr_reg.od_lut_sel)
	{
		case 0: // od r channel
			memcpy(drv_od_lut.value[0], pODtable, OD_table_length*sizeof(unsigned char));
			break;
		case 1:  // od g channel
		default:
			memcpy(drv_od_lut.value[1], pODtable, OD_table_length*sizeof(unsigned char));
			break;
		case 2:  // od b channel
			memcpy(drv_od_lut.value[2], pODtable, OD_table_length*sizeof(unsigned char));
			break;
		case 3:  // od all channel
			memcpy(drv_od_lut.value[0], pODtable, OD_table_length*sizeof(unsigned char));
			memcpy(drv_od_lut.value[1], pODtable, OD_table_length*sizeof(unsigned char));
			memcpy(drv_od_lut.value[2], pODtable, OD_table_length*sizeof(unsigned char));
			break;
		case 4: // rec od r channel
			memcpy(drv_od_rec_lut.value[0], pODtable, OD_table_length*sizeof(unsigned char));
			break;
		case 5:  // rec od g channel
			memcpy(drv_od_rec_lut.value[1], pODtable, OD_table_length*sizeof(unsigned char));
			break;
		case 6:  // rec od b channel
			memcpy(drv_od_rec_lut.value[2], pODtable, OD_table_length*sizeof(unsigned char));
			break;
		case 7:  // rec od all channel
			memcpy(drv_od_rec_lut.value[0], pODtable, OD_table_length*sizeof(unsigned char));
			memcpy(drv_od_rec_lut.value[1], pODtable, OD_table_length*sizeof(unsigned char));
			memcpy(drv_od_rec_lut.value[2], pODtable, OD_table_length*sizeof(unsigned char));
			break;
	}
}
void drif_color_od_lut_index_get(bool recursive, unsigned char *preIdx, unsigned char *curIdx)
{
	int i = 0;
	unsigned int preReg = 0;
	unsigned int curReg = 0;

	/* fixed index */
	preIdx[0] = 0;
	preIdx[32] = 255;
	curIdx[0] = 0;
	curIdx[32] = 255;

	/* read from register */
	for(i=1; i<32; i++)
	{
		unsigned char delta = 4*((i-1)/4); // 0, 0, 0, 0, 4, 4, 4, 4, 8, ...
		unsigned char shiftBit = ((3-((i-1)%4))*8); // 24, 16, 8, 0, 24, 16, ...
		unsigned int regMask = (0xFF) << shiftBit; // 0xFF000000, 0x00FF0000, ...
	
		if(recursive)
		{
			preReg = OD_REC_OD_LUT_ROW_INDEX1_reg + delta;
			curReg = OD_REC_OD_LUT_COL_INDEX1_reg + delta;
		}
		else
		{
			preReg = OD_OD_LUT_ROW_INDEX1_reg + delta;
			curReg = OD_OD_LUT_COL_INDEX1_reg + delta;
		}

		preIdx[i] = (unsigned char)((IoReg_Read32(preReg) & regMask) >> shiftBit);
		curIdx[i] = (unsigned char)((IoReg_Read32(curReg) & regMask) >> shiftBit);
	}
}
void drif_color_od_lut_index_set(bool recursive, unsigned char *preIdx, unsigned char *curIdx)
{
	int i = 0;
	unsigned char tmp;
	unsigned int preReg = 0;
	unsigned int curReg = 0;
	unsigned int preValue = 0;
	unsigned int curValue = 0;

	/* disable OD */
	tmp = drvif_color_od_enable_get();
	drvif_color_od_enable_set(0);

	/* write to register */
	for(i=1; i<32; i++)
	{
		unsigned char delta = 4*((i-1)/4); // 0, 0, 0, 0, 4, 4, 4, 4, 8, ...
		unsigned char shiftBit = ((3-((i-1)%4))*8); // 24, 16, 8, 0, 24, 16, ...
	
		if(recursive)
		{
			preReg = OD_REC_OD_LUT_ROW_INDEX1_reg + delta;
			curReg = OD_REC_OD_LUT_COL_INDEX1_reg + delta;
		}
		else
		{
			preReg = OD_OD_LUT_ROW_INDEX1_reg + delta;
			curReg = OD_OD_LUT_COL_INDEX1_reg + delta;
		}

		preValue += (preIdx[i] << shiftBit);
		curValue += (curIdx[i] << shiftBit);

		if((i-1)%4 == 3 || i == 31)
		{
			IoReg_Write32(preReg, preValue);
			IoReg_Write32(curReg, curValue);
			preValue = 0;
			curValue = 0;
		}
	}

	/* enable OD */
	drvif_color_od_enable_set(tmp);
}
void drvif_color_safe_od_enable(unsigned char bEnable, unsigned char bOD_OnOff_Switch)
{
	// Merlin3 fixed hw bug
	return;
#if 0

	UINT32 timeoutcnt = 0x032500;
	od_od_ctrl_RBUS od_ctrl_reg;

	// OD function db on
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
	// OD function db read sel: reg
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25

	if (!bEnable) { // OD Disable
		if (bOD_OnOff_Switch) {
			// OD off
			IoReg_ClearBits(OD_OD_CTRL_reg, OD_OD_CTRL_od_en_mask|OD_OD_CTRL_od_fun_sel_mask); //_BIT0|_BIT1

			// OD function db apply
			IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26

			// wait vsync
			od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			while((od_ctrl_reg.regValue & 0x3) != 0 && timeoutcnt-- != 0) {
				od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			}
		}

		IoReg_Write32(OD_DMA_ODDMA_Cap_BoundaryAddr1_reg, IoReg_Read32(OD_DMA_ODDMA_Cap_BoundaryAddr2_reg));
	} else { // OD Enable
		od_od_ctrl1_RBUS od_ctrl1_reg;

		IoReg_Write32(OD_DMA_ODDMA_Cap_BoundaryAddr1_reg, IoReg_Read32(OD_DMA_ODDMA_Cap_L2_Start_reg)+0x800000-0x20);

		IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26
		od_ctrl1_reg.regValue = IoReg_Read32(OD_OD_CTRL1_reg);
		while(od_ctrl1_reg.db_apply == 0x1 && timeoutcnt-- != 0) {
			od_ctrl1_reg.regValue = IoReg_Read32(OD_OD_CTRL1_reg);
		}

		if (bOD_OnOff_Switch) {
			// OD on
			IoReg_SetBits(OD_OD_CTRL_reg, OD_OD_CTRL_od_en_mask|OD_OD_CTRL_od_fun_sel_mask); //_BIT0|_BIT1

			// OD function db apply
			IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26

			// wait vsync
			timeoutcnt = 0x032500;
			od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			while((od_ctrl_reg.regValue & 0x3) != 0x3 && timeoutcnt-- != 0) {
				od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			}
		}
	}

	// OD function db off
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
	// OD function db read sel: temp
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25
#endif
}

void drvif_color_set_od_bits(unsigned char FrameLimitBit)
{
	od_od_ctrl_RBUS od_ctrl_reg;
	od_dma_od_dma_ctrl_RBUS od_dma_od_dma_ctrl_reg;
	od_dma_oddma_pq_cmp_RBUS od_dma_oddma_pq_cmp_reg;
	od_dma_oddma_pq_cmp_bit_RBUS oddma_pq_cmp_bit_reg;
	od_dma_oddma_wr_num_bl_wrap_word_RBUS od_dma_oddma_wr_num_bl_wrap_word_reg;
	od_dma_oddma_rd_num_bl_wrap_word_RBUS od_dma_oddma_rd_num_bl_wrap_word_reg;
	od_dma_oddma_wr_num_bl_wrap_line_step_RBUS od_dma_oddma_cap_linestep_reg;
	od_dma_oddma_rd_num_bl_wrap_line_step_RBUS od_dma_oddma_rd_num_bl_wrap_line_step_reg;
	od_dma_oddma_pq_cmp_pair_RBUS oddma_pq_cmp_pair_reg;
	od_dma_oddma_pq_decmp_pair_RBUS oddma_pq_decmp_pair_reg;

	unsigned char dma_mode;
	unsigned char od_enable_status = 0;
	unsigned short width;
	unsigned int align_value = 0;
	unsigned int pqc_line_sum_bit = 0;
	//enum PLAFTORM_TYPE platform = get_platform();


	rtd_pr_vpq_info("OD_MEM drvif_color_set_od_bits =%d \n",FrameLimitBit);

	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_OD != 1, OD bits\n");
		return;
	}

	od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	od_enable_status = (od_ctrl_reg.regValue & OD_OD_CTRL_od_en_mask)?1:0; // _BIT0

	oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);


        if ((Get_DISPLAY_REFRESH_RATE()!=60)){ //120hz panel
                
                if ((oddma_pq_cmp_bit_reg.frame_limit_bit/2 == FrameLimitBit )&& (od_enable_status == (FrameLimitBit?1:0))){

                  	rtd_pr_vpq_info("OD_MEM  120hz panel frame_limit_bit =%d, FrameLimitBit=%d, \n",oddma_pq_cmp_bit_reg.frame_limit_bit/2,FrameLimitBit);
      
                        return;

                }
        }
        else if ((Get_DISPLAY_REFRESH_RATE()==60)){
        	if ((oddma_pq_cmp_bit_reg.frame_limit_bit == FrameLimitBit )&& (od_enable_status == (FrameLimitBit?1:0))){
                        rtd_pr_vpq_info("OD_MEM  60 hz panel frame_limit_bit =%d, FrameLimitBit=%d, \n",oddma_pq_cmp_bit_reg.frame_limit_bit,FrameLimitBit);

                return;

        	}
        }

 
	//if ((oddma_pq_cmp_bit_reg.frame_limit_bit == FrameLimitBit )&& (od_enable_status == (FrameLimitBit?1:0)))
        //        return;

	// disable OD and dma
	if (od_enable_status) {	
		drvif_color_od_enable_set(0);
		drvif_color_od_dma_cap_enable_set(0);
		drvif_color_od_dma_disp_enable_set(0);
	}

	if (FrameLimitBit > 0) {
		// pqc hw performance limitation
                if (Get_DISPLAY_REFRESH_RATE() >= 120 && FrameLimitBit < 8){
                        FrameLimitBit = 8;
                }
                else if ((Get_DISPLAY_REFRESH_RATE()==60)&& FrameLimitBit < 2){
                        FrameLimitBit = 2;
                }

                /*if (platform != PLATFORM_KXLP && FrameLimitBit < 8)
                        FrameLimitBit = 8;
                else if (platform == PLATFORM_KXLP && FrameLimitBit < 2)
                        FrameLimitBit = 2;
                */

		// b802cdc0 dma ctrl
		od_dma_od_dma_ctrl_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
		dma_mode = od_dma_od_dma_ctrl_reg.oddma_mode_sel;

		od_dma_oddma_pq_cmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_reg);
		width = od_dma_oddma_pq_cmp_reg.cmp_width_div32;
		
		// b802cd08 Compression ratio
		//oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);
		oddma_pq_cmp_bit_reg.frame_limit_bit = FrameLimitBit;

		// DMA buffer line limit clamp
		if(dma_mode == 0)
			oddma_pq_cmp_bit_reg.line_limit_bit = oddma_pq_cmp_bit_reg.frame_limit_bit;
		else if(dma_mode == 1)
			oddma_pq_cmp_bit_reg.line_limit_bit = FrameLimitBit + 6;
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BIT_reg, oddma_pq_cmp_bit_reg.regValue);
		oddma_pq_cmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg);
		oddma_pq_decmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg);

		if(FrameLimitBit <= 6) {
			// 8 bit
			oddma_pq_cmp_pair_reg.cmp_data_bit_width = 0;
			oddma_pq_decmp_pair_reg.decmp_data_bit_width = 0;
		}
		else {
			// 10 bit
			oddma_pq_cmp_pair_reg.cmp_data_bit_width = 1;
			oddma_pq_decmp_pair_reg.decmp_data_bit_width = 1;
		}
                if (Get_DISPLAY_REFRESH_RATE() >= 120) //120hz
                {
                        oddma_pq_cmp_pair_reg.cmp_data_bit_width = 1;
                        oddma_pq_decmp_pair_reg.decmp_data_bit_width = 1;

                }
                /* //k7 remove 
                        if (platform != PLATFORM_KXLP)  // 120 hz
                        {
                        	oddma_pq_cmp_pair_reg.cmp_data_bit_width = 1;
                        	oddma_pq_decmp_pair_reg.decmp_data_bit_width = 1;
                        }
                */

		pqc_line_sum_bit = (width * 32 * oddma_pq_cmp_bit_reg.line_limit_bit + 256) / 128 + ((width * 32 * oddma_pq_cmp_bit_reg.line_limit_bit + 256)%128!=0);

#ifdef CONFIG_BW_96B_ALIGNED
		// for cap/disp line_step (128 bits unit)
        align_value = drvifget_align_value(pqc_line_sum_bit, 6);
#else
		align_value = pqc_line_sum_bit; //origin value
#endif
		if(dma_mode == 0) {
			oddma_pq_cmp_pair_reg.cmp_line_sum_bit = align_value;
			oddma_pq_decmp_pair_reg.decmp_line_sum_bit = align_value;
		}

		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg, oddma_pq_cmp_pair_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg, oddma_pq_decmp_pair_reg.regValue);

		od_dma_oddma_wr_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
		od_dma_oddma_rd_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);

		if(dma_mode == 0) { // LINE MODE
			// b802ce38 Cap LineStep //must be check 96 alignment
			od_dma_oddma_cap_linestep_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
			od_dma_oddma_cap_linestep_reg.cap_line_step = align_value; // 128bit
			IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg, od_dma_oddma_cap_linestep_reg.regValue);

			// b802ceac Disp LineStep //must be check 96 alignment
			od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
			od_dma_oddma_rd_num_bl_wrap_line_step_reg.disp_line_step = align_value; // 128bit
			IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg, od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue);

			// b802ce44 cap_line_128_num
			od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = align_value;

			// b802cec4 disp_line_128_num
			od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = align_value;
		} else if(dma_mode == 1) { // FRAME MODE
			// b802ce44 cap_line_128_num = 0xffff
			od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = 0xffff;

			// b802cec4 disp_line_128_num = 0xffff
			od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = 0xffff;
		}

		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg, od_dma_oddma_wr_num_bl_wrap_word_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg, od_dma_oddma_rd_num_bl_wrap_word_reg.regValue);

		drvif_color_od_dma_cap_enable_set(1);
		drvif_color_od_dma_disp_enable_set(1);
		drvif_color_od_enable_set(1);

	} else {
		return;
	}
}

unsigned char drvif_color_get_od_bits(void)
{
	od_dma_oddma_pq_cmp_bit_RBUS oddma_pq_cmp_bit_reg;
	oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);

	return oddma_pq_cmp_bit_reg.frame_limit_bit;
}

unsigned char drvif_color_get_od_dma_mode(void)
{
	od_dma_od_dma_ctrl_RBUS od_dma_od_dma_ctrl_reg;
	od_dma_od_dma_ctrl_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);

	return od_dma_od_dma_ctrl_reg.oddma_mode_sel;
}

char drvif_color_set_od_2p(unsigned char mode)
{
	od_od_ctrl_RBUS od_od_ctrl_reg;

	od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);

	if (mode == 1)
		od_od_ctrl_reg.od_4k120_mode = 1;
	else
		od_od_ctrl_reg.od_4k120_mode = 0;
	IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl_reg.regValue);
	/* db apply */
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26 
	return 0;
}

void drvif_color_set_od_dma_HW_reset(void)
{
	od_dma_oddma_wr_ctrl_RBUS od_dma_oddma_wr_ctrl_reg;
	od_dma_oddma_rd_ctrl_RBUS od_dma_oddma_rd_ctrl_reg;
	od_od_ctrl_RBUS od_od_ctrl_reg;

	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);

	if (drvif_color_od_enable_get()==1){ 
		drvif_color_od_enable_set(0); //disable OD and waiting db apply success

		rtd_pr_vpq_info("origin OD is En ,disable OD and OD DMA HW reset \n");

		/* reset HW OD DMA */
		od_dma_oddma_wr_ctrl_reg.cap_force_all_rst =1;
		od_dma_oddma_rd_ctrl_reg.disp_force_all_rst=1; 
		IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg,od_dma_oddma_wr_ctrl_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg,od_dma_oddma_rd_ctrl_reg.regValue);

		usleep_range(50, 500);

		od_dma_oddma_wr_ctrl_reg.cap_force_all_rst =0;
		od_dma_oddma_rd_ctrl_reg.disp_force_all_rst=0; 
		IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg,od_dma_oddma_wr_ctrl_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg,od_dma_oddma_rd_ctrl_reg.regValue);

		od_od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
		/* OD enable */
		od_od_ctrl_reg.od_en = 1;
		IoReg_Write32(OD_OD_CTRL_reg, od_od_ctrl_reg.regValue);
		/* db apply */
		IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26 
		rtd_pr_vpq_info("After OD HW reset , Enable OD \n");

	}else{

		rtd_pr_vpq_info("origin OD is Diable, Do OD DMA HW reset \n");

		/* reset HW OD DMA */
		od_dma_oddma_wr_ctrl_reg.cap_force_all_rst =1;
		od_dma_oddma_rd_ctrl_reg.disp_force_all_rst=1; 
		IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg,od_dma_oddma_wr_ctrl_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg,od_dma_oddma_rd_ctrl_reg.regValue);

		usleep_range(50, 500);
		od_dma_oddma_wr_ctrl_reg.cap_force_all_rst =0;
		od_dma_oddma_rd_ctrl_reg.disp_force_all_rst=0; 
		IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg,od_dma_oddma_wr_ctrl_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg,od_dma_oddma_rd_ctrl_reg.regValue);
	}
}


