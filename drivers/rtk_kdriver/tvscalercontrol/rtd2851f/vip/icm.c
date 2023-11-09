/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
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
#include <linux/delay.h>
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>
#include <linux/mm.h>
#else
#include <no_os/printk.h>
#include <no_os/slab.h>
#endif
/*#include "rtd_regs.h"*/
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/scalerColor.h>

#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scaler/scalertimer.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"

#include <rbus/dmato3dtable_reg.h>
#include <rbus/dmato3dlut_reg.h>
#include <rbus/timer_reg.h>

/*===================================  Types ================================*/

/*================================== Variables ==============================*/

/*global adjust for mac3 icm, leonard_wu@20140402*/
static int g_dhue;
static int g_dsatbysat[SATSEGMAX];
static int g_ditnbyitn[ITNSEGMAX];

/* vip table value*/

static int g_dhue_tab;
static int g_dsatbysat_tab[SATSEGMAX];
static int g_ditnbyitn_tab[ITNSEGMAX];

int h_sta1, h_sta2, h_end1, h_end2;


/*================================ Definitions ==============================*/
#define _clamp(_in, _min, _max)		((_in < _min) ? _min : (_in > _max ? _max : _in))

#define DataActualValue(val, bitvalid)	((val >> bitvalid) ? (val | (signed short)(~((1 << bitvalid) - 1))) : (val)) /*leonard wu, 20120917*/

#define tICM_Red		0
#define tICM_Green		1
#define tICM_Blue		2
#define tICM_Yellow		3
#define tICM_Magenta	4
#define tICM_Cyan		5

#define tICM_Hue		0
#define tICM_Sat		1
#define tICM_Bri		2

#define CLIP(min, max, val) (((val) < (min)) ? (min) : ((val) > (max) ? (max) : (val)))
typedef struct _ICM_GLOBAL_BIT_MAP {
	int value:12;
	int reservd:20;
} ICM_GLOBAL_BIT_MAP;

#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_ICM_DEBUG,fmt,##args)

#define	vip_malloc(x)	kmalloc(x, GFP_KERNEL)
#define	vip_free(x)	kfree(x)
/*================================== Function ===============================*/

void drvif_color_icm_hue_histo_init(void)
{
	/*hue histogram*/
	{
	color_icm_icm_auto_ctrl_RBUS 		icm_auto_ctrl_reg;
	color_icm_hue_hist_settings0_RBUS 	hue_hist_settings0_reg;
	color_icm_hue_hist_settings1_RBUS 	hue_hist_settings1_reg;

	icm_auto_ctrl_reg.regValue		= IoReg_Read32(COLOR_ICM_ICM_AUTO_CTRL_reg);
	hue_hist_settings0_reg.regValue	= IoReg_Read32(COLOR_ICM_HUE_HIST_SETTINGS0_reg);
	hue_hist_settings1_reg.regValue	= IoReg_Read32(COLOR_ICM_HUE_HIST_SETTINGS1_reg);

	hue_hist_settings1_reg.hue_hist_itn_hb = 0xfff;
	hue_hist_settings1_reg.hue_hist_itn_lb = 0x0;
	hue_hist_settings0_reg.hue_hist_sat_lb = 0x20;

	icm_auto_ctrl_reg.inside_enable		= 1;
	icm_auto_ctrl_reg.hist_hue_enable	= 1;
	icm_auto_ctrl_reg.icm_auto_ctrl_hue_hist_mode_sel	= 1;

	IoReg_Write32(COLOR_ICM_ICM_AUTO_CTRL_reg,		icm_auto_ctrl_reg.regValue);
	IoReg_Write32(COLOR_ICM_HUE_HIST_SETTINGS0_reg,	hue_hist_settings0_reg.regValue);
	IoReg_Write32(COLOR_ICM_HUE_HIST_SETTINGS1_reg,	hue_hist_settings1_reg.regValue);
	}

	/*sat histogram*/
	{
	color_icm_d_auto_sat_hist_ctrl_RBUS		d_auto_sat_hist_ctrl_reg;
	color_icm_d_auto_sat_hist0_int_thrd_RBUS	d_auto_sat_hist0_int_thrd_reg;
	color_icm_d_auto_sat_hist1_int_thrd_RBUS	d_auto_sat_hist1_int_thrd_reg;
	color_icm_d_auto_sat_hist2_int_thrd_RBUS	d_auto_sat_hist2_int_thrd_reg;
	color_icm_d_auto_sat_hist3_int_thrd_RBUS	d_auto_sat_hist3_int_thrd_reg;

	color_icm_d_auto_sat_hist01_sat_thrd_RBUS d_auto_sat_hist01_sat_thrd_reg;
	color_icm_d_auto_sat_hist23_sat_thrd_RBUS d_auto_sat_hist23_sat_thrd_reg;

	color_icm_d_auto_sat_hist_ctrl0_RBUS  d_auto_sat_hist_ctrl0_reg;
	color_icm_d_auto_sat_hist_ctrl1_RBUS  d_auto_sat_hist_ctrl1_reg;

	/*mode*/
	d_auto_sat_hist_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_D_AUTO_SAT_HIST_CTRL_reg);
	d_auto_sat_hist_ctrl_reg.sat_hist_mode = 0;
	d_auto_sat_hist_ctrl_reg.d_auto_sat_ctrl_sat_hist_mode_sel = 1;
	d_auto_sat_hist_ctrl_reg.hist_sat_enable = 1;
	IoReg_Write32(COLOR_ICM_D_AUTO_SAT_HIST_CTRL_reg, d_auto_sat_hist_ctrl_reg.regValue);

	/*bound*/
	d_auto_sat_hist0_int_thrd_reg.regValue = IoReg_Read32(COLOR_ICM_D_AUTO_SAT_HIST0_INT_THRD_reg);
	d_auto_sat_hist1_int_thrd_reg.regValue = IoReg_Read32(COLOR_ICM_D_AUTO_SAT_HIST1_INT_THRD_reg);
	d_auto_sat_hist2_int_thrd_reg.regValue = IoReg_Read32(COLOR_ICM_D_AUTO_SAT_HIST2_INT_THRD_reg);
	d_auto_sat_hist3_int_thrd_reg.regValue = IoReg_Read32(COLOR_ICM_D_AUTO_SAT_HIST3_INT_THRD_reg);
	d_auto_sat_hist01_sat_thrd_reg.regValue = IoReg_Read32(COLOR_ICM_D_AUTO_SAT_HIST01_SAT_THRD_reg);
	d_auto_sat_hist23_sat_thrd_reg.regValue = IoReg_Read32(COLOR_ICM_D_AUTO_SAT_HIST23_SAT_THRD_reg);

	/*hist0*/
	d_auto_sat_hist0_int_thrd_reg.d_hist0_thrd_auto_sat_i_u_thre0 = 5840;
	d_auto_sat_hist0_int_thrd_reg.d_hist0_thrd_auto_sat_i_l_thre0 = -1752;
	d_auto_sat_hist01_sat_thrd_reg.d_hist0_thrd_auto_sat_s_thre0 = 0;
	/*hist1*/
	d_auto_sat_hist1_int_thrd_reg.d_hist1_thrd_auto_sat_i_u_thre1 = 5840;
	d_auto_sat_hist1_int_thrd_reg.d_hist1_thrd_auto_sat_i_l_thre1 = -1752;
	d_auto_sat_hist01_sat_thrd_reg.d_hist1_thrd_auto_sat_s_thre1 = 0;
	/*hist2*/
	d_auto_sat_hist2_int_thrd_reg.d_hist2_thrd_auto_sat_i_u_thre2 = 5840;
	d_auto_sat_hist2_int_thrd_reg.d_hist2_thrd_auto_sat_i_l_thre2 = -1752;
	d_auto_sat_hist23_sat_thrd_reg.d_hist2_thrd_auto_sat_s_thre2 = 0;
	/*hist3*/
	d_auto_sat_hist3_int_thrd_reg.d_hist3_thrd_auto_sat_i_u_thre3 = 5840;
	d_auto_sat_hist3_int_thrd_reg.d_hist3_thrd_auto_sat_i_l_thre3 = -1752;
	d_auto_sat_hist23_sat_thrd_reg.d_hist3_thrd_auto_sat_s_thre3 = 0;

	IoReg_Write32(COLOR_ICM_D_AUTO_SAT_HIST0_INT_THRD_reg, d_auto_sat_hist0_int_thrd_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_AUTO_SAT_HIST1_INT_THRD_reg, d_auto_sat_hist1_int_thrd_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_AUTO_SAT_HIST2_INT_THRD_reg, d_auto_sat_hist2_int_thrd_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_AUTO_SAT_HIST3_INT_THRD_reg, d_auto_sat_hist3_int_thrd_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_AUTO_SAT_HIST01_SAT_THRD_reg, d_auto_sat_hist01_sat_thrd_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_AUTO_SAT_HIST23_SAT_THRD_reg, d_auto_sat_hist23_sat_thrd_reg.regValue);

	/*hue start/end*/
	d_auto_sat_hist_ctrl0_reg.regValue = IoReg_Read32(COLOR_ICM_D_AUTO_SAT_HIST_CTRL0_reg);
	d_auto_sat_hist_ctrl1_reg.regValue = IoReg_Read32(COLOR_ICM_D_AUTO_SAT_HIST_CTRL1_reg);

	d_auto_sat_hist_ctrl0_reg.sat_hist0_hue_sta = 0;
	d_auto_sat_hist_ctrl0_reg.sat_hist0_hue_end = 192;
	d_auto_sat_hist_ctrl0_reg.sat_hist1_hue_sta = 0;
	d_auto_sat_hist_ctrl0_reg.sat_hist1_hue_end = 192;
	d_auto_sat_hist_ctrl1_reg.sat_hist2_hue_sta = 0;
	d_auto_sat_hist_ctrl1_reg.sat_hist2_hue_end = 192;
	d_auto_sat_hist_ctrl1_reg.sat_hist3_hue_sta = 0;
	d_auto_sat_hist_ctrl1_reg.sat_hist3_hue_end = 192;

	IoReg_Write32(COLOR_ICM_D_AUTO_SAT_HIST_CTRL0_reg, d_auto_sat_hist_ctrl0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_AUTO_SAT_HIST_CTRL1_reg, d_auto_sat_hist_ctrl1_reg.regValue);
	}
}



/*re-create this function for mac3 icm, leonard_wu@20140401*/
/*write icm table*/
void drvif_color_icm_table_select(unsigned char display, unsigned short *tICM_ini, unsigned char which_table)
{
	int h, si, i;
	int h_sta, h_end, si_sta, si_end;
	unsigned char if_write_ic;
	int timeout;
	int LayerSize;

	unsigned char ICM_Num_Limite	= 255;/*30;*/ /*planck changed to 255 for the max of icm tables.20101111*/

	/* Global Setting*/
	unsigned int baseAddr			= which_table * VIP_ICM_TBL_Y * VIP_ICM_TBL_Z;
	unsigned int baseAddr_global	= baseAddr + 0 * VIP_ICM_TBL_Z;		/*global control*/
	unsigned int baseAddr_pillar	= baseAddr + 1 * VIP_ICM_TBL_Z;		/*pillar*/

	unsigned int baseAddr_hue_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 0) * VIP_ICM_TBL_Z;		/*hue table*/
	unsigned int baseAddr_sat_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 1) * VIP_ICM_TBL_Z;		/*sat table*/
	unsigned int baseAddr_itn_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 2) * VIP_ICM_TBL_Z;		/*itn table*/

	VIPprintf("drvif_color_icm_table_select Initialize  which_table= %d\n", which_table);

	if (which_table >= ICM_Num_Limite) /* overflow */
		return;

	/*enable icm*/
	drvif_color_Icm_Enable(1);

	if (display == SLR_MAIN_DISPLAY) {
		color_icm_dm_icm_ctrl_RBUS                dm_icm_ctrl_reg;
		color_icm_dm_uv_coring_RBUS               dm_uv_coring_reg;
		color_icm_dm_icm_hue_segment_0_RBUS       dm_icm_hue_segment_0_reg;
		color_icm_dm_icm_hue_segment_1_RBUS       dm_icm_hue_segment_1_reg;
		color_icm_dm_icm_hue_segment_2_RBUS       dm_icm_hue_segment_2_reg;
		color_icm_dm_icm_hue_segment_3_RBUS       dm_icm_hue_segment_3_reg;
		color_icm_dm_icm_hue_segment_4_RBUS       dm_icm_hue_segment_4_reg;
		color_icm_dm_icm_hue_segment_5_RBUS       dm_icm_hue_segment_5_reg;
		color_icm_dm_icm_hue_segment_6_RBUS       dm_icm_hue_segment_6_reg;
		color_icm_dm_icm_hue_segment_7_RBUS       dm_icm_hue_segment_7_reg;
		color_icm_dm_icm_hue_segment_8_RBUS       dm_icm_hue_segment_8_reg;
		color_icm_dm_icm_hue_segment_9_RBUS       dm_icm_hue_segment_9_reg;
		color_icm_dm_icm_hue_segment_10_RBUS      dm_icm_hue_segment_10_reg;
		color_icm_dm_icm_hue_segment_11_RBUS      dm_icm_hue_segment_11_reg;
		color_icm_dm_icm_sat_segment_0_RBUS       dm_icm_sat_segment_0_reg;
		color_icm_dm_icm_int_segment_0_RBUS       dm_icm_int_segment_0_reg;

		/*global*/
		dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);
		dm_uv_coring_reg.regValue = IoReg_Read32(COLOR_ICM_DM_UV_Coring_reg);

		dm_icm_ctrl_reg.table_mode          = tICM_ini[baseAddr_global + 0];
		dm_icm_ctrl_reg.hsi_grid_sel        = tICM_ini[baseAddr_global + 1];
		dm_icm_ctrl_reg.out_grid_process    = tICM_ini[baseAddr_global + 2];
		dm_icm_ctrl_reg.y_ctrl              = tICM_ini[baseAddr_global + 3];

		dm_uv_coring_reg.u_coring = tICM_ini[baseAddr_global + 4];
		dm_uv_coring_reg.v_coring = tICM_ini[baseAddr_global + 5];


		IoReg_Write32(COLOR_ICM_DM_ICM_CTRL_reg, dm_icm_ctrl_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_UV_Coring_reg, dm_uv_coring_reg.regValue);

		/*g_dhue*/
		g_dhue			= tICM_ini[baseAddr_global + 6];
		g_dhue			= ((ICM_GLOBAL_BIT_MAP *)&g_dhue)->value;
		/*g_dsatbysat*/
		/*
		g_dsatbysat[0]	= tICM_ini[baseAddr_global + 7];
		g_dsatbysat[1]	= tICM_ini[baseAddr_global + 8];
		g_dsatbysat[2]	= tICM_ini[baseAddr_global + 9];
		g_dsatbysat[3]	= tICM_ini[baseAddr_global + 10];
		g_dsatbysat[4]	= tICM_ini[baseAddr_global + 11];
		g_dsatbysat[5]	= tICM_ini[baseAddr_global + 12];
		g_dsatbysat[6]	= tICM_ini[baseAddr_global + 13];
		g_dsatbysat[7]	= tICM_ini[baseAddr_global + 14];
		g_dsatbysat[8]	= tICM_ini[baseAddr_global + 15];
		g_dsatbysat[9]	= tICM_ini[baseAddr_global + 16];
		g_dsatbysat[10]	= tICM_ini[baseAddr_global + 17];
		g_dsatbysat[11]	= tICM_ini[baseAddr_global + 18];
		*/
		for (i = 0; i <= 11; i++) {
			g_dsatbysat[i]	= tICM_ini[baseAddr_global + 7 + i];
			g_dsatbysat[i] = ((ICM_GLOBAL_BIT_MAP *)(g_dsatbysat+i))->value;
		}
		/*g_ditnbyitn*/
		/*
		g_ditnbyitn[0]	= tICM_ini[baseAddr_global + 19];
		g_ditnbyitn[1]	= tICM_ini[baseAddr_global + 20];
		g_ditnbyitn[2]	= tICM_ini[baseAddr_global + 21];
		g_ditnbyitn[3]	= tICM_ini[baseAddr_global + 22];
		g_ditnbyitn[4]	= tICM_ini[baseAddr_global + 23];
		g_ditnbyitn[5]	= tICM_ini[baseAddr_global + 24];
		g_ditnbyitn[6]	= tICM_ini[baseAddr_global + 25];
		g_ditnbyitn[7]	= tICM_ini[baseAddr_global + 26];
		g_ditnbyitn[8]	= tICM_ini[baseAddr_global + 27];
		g_ditnbyitn[9]	= tICM_ini[baseAddr_global + 28];
		g_ditnbyitn[10]	= tICM_ini[baseAddr_global + 29];
		g_ditnbyitn[11]	= tICM_ini[baseAddr_global + 30];
		*/
		for (i = 0; i <= 11; i++) {
			g_ditnbyitn[i]	= tICM_ini[baseAddr_global + 19 + i];
			g_ditnbyitn[i] = ((ICM_GLOBAL_BIT_MAP *)(g_ditnbyitn+i))->value;
		}

		/*record vip table value*/
		/*g_dhue*/
		g_dhue_tab			= g_dhue;
		/*g_dsatbysat*/
		memcpy(g_dsatbysat_tab, g_dsatbysat, sizeof(g_dsatbysat_tab));
		/*g_ditnbyitn*/
		memcpy(g_ditnbyitn_tab, g_ditnbyitn, sizeof(g_ditnbyitn_tab));

		/*pillar*/
		dm_icm_hue_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_0_reg);
		dm_icm_hue_segment_1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_1_reg);
		dm_icm_hue_segment_2_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_2_reg);
		dm_icm_hue_segment_3_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_3_reg);
		dm_icm_hue_segment_4_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_4_reg);
		dm_icm_hue_segment_5_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_5_reg);
		dm_icm_hue_segment_6_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_6_reg);
		dm_icm_hue_segment_7_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_7_reg);
		dm_icm_hue_segment_8_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_8_reg);
		dm_icm_hue_segment_9_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_9_reg);
		dm_icm_hue_segment_10_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_10_reg);
		dm_icm_hue_segment_11_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_11_reg);

		dm_icm_sat_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg);
		dm_icm_int_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg);
		/*hue pillar*/
		dm_icm_hue_segment_0_reg.h_pillar_1 = tICM_ini[baseAddr_pillar + 0];
		dm_icm_hue_segment_0_reg.h_pillar_2 = tICM_ini[baseAddr_pillar + 1];
		dm_icm_hue_segment_0_reg.h_pillar_3 = tICM_ini[baseAddr_pillar + 2];
		dm_icm_hue_segment_0_reg.h_pillar_4 = tICM_ini[baseAddr_pillar + 3];

		dm_icm_hue_segment_1_reg.h_pillar_5 = tICM_ini[baseAddr_pillar + 4];
		dm_icm_hue_segment_1_reg.h_pillar_6 = tICM_ini[baseAddr_pillar + 5];
		dm_icm_hue_segment_1_reg.h_pillar_7 = tICM_ini[baseAddr_pillar + 6];
		dm_icm_hue_segment_1_reg.h_pillar_8 = tICM_ini[baseAddr_pillar + 7];

		dm_icm_hue_segment_2_reg.h_pillar_9  = tICM_ini[baseAddr_pillar + 8];
		dm_icm_hue_segment_2_reg.h_pillar_10 = tICM_ini[baseAddr_pillar + 9];
		dm_icm_hue_segment_2_reg.h_pillar_11 = tICM_ini[baseAddr_pillar + 10];
		dm_icm_hue_segment_2_reg.h_pillar_12 = tICM_ini[baseAddr_pillar + 11];

		dm_icm_hue_segment_3_reg.h_pillar_13 = tICM_ini[baseAddr_pillar + 12];
		dm_icm_hue_segment_3_reg.h_pillar_14 = tICM_ini[baseAddr_pillar + 13];
		dm_icm_hue_segment_3_reg.h_pillar_15 = tICM_ini[baseAddr_pillar + 14];
		dm_icm_hue_segment_3_reg.h_pillar_16 = tICM_ini[baseAddr_pillar + 15];

		dm_icm_hue_segment_4_reg.h_pillar_17 = tICM_ini[baseAddr_pillar + 16];
		dm_icm_hue_segment_4_reg.h_pillar_18 = tICM_ini[baseAddr_pillar + 17];
		dm_icm_hue_segment_4_reg.h_pillar_19 = tICM_ini[baseAddr_pillar + 18];
		dm_icm_hue_segment_4_reg.h_pillar_20 = tICM_ini[baseAddr_pillar + 19];

		dm_icm_hue_segment_5_reg.h_pillar_21 = tICM_ini[baseAddr_pillar + 20];
		dm_icm_hue_segment_5_reg.h_pillar_22 = tICM_ini[baseAddr_pillar + 21];
		dm_icm_hue_segment_5_reg.h_pillar_23 = tICM_ini[baseAddr_pillar + 22];
		dm_icm_hue_segment_5_reg.h_pillar_24 = tICM_ini[baseAddr_pillar + 23];

		dm_icm_hue_segment_6_reg.h_pillar_25 = tICM_ini[baseAddr_pillar + 24];
		dm_icm_hue_segment_6_reg.h_pillar_26 = tICM_ini[baseAddr_pillar + 25];
		dm_icm_hue_segment_6_reg.h_pillar_27 = tICM_ini[baseAddr_pillar + 26];
		dm_icm_hue_segment_6_reg.h_pillar_28 = tICM_ini[baseAddr_pillar + 27];

		dm_icm_hue_segment_7_reg.h_pillar_29 = tICM_ini[baseAddr_pillar + 28];
		dm_icm_hue_segment_7_reg.h_pillar_30 = tICM_ini[baseAddr_pillar + 29];
		dm_icm_hue_segment_7_reg.h_pillar_31 = tICM_ini[baseAddr_pillar + 30];
		dm_icm_hue_segment_7_reg.h_pillar_32 = tICM_ini[baseAddr_pillar + 31];

		dm_icm_hue_segment_8_reg.h_pillar_33 = tICM_ini[baseAddr_pillar + 32];
		dm_icm_hue_segment_8_reg.h_pillar_34 = tICM_ini[baseAddr_pillar + 33];
		dm_icm_hue_segment_8_reg.h_pillar_35 = tICM_ini[baseAddr_pillar + 34];
		dm_icm_hue_segment_8_reg.h_pillar_36 = tICM_ini[baseAddr_pillar + 35];

		dm_icm_hue_segment_9_reg.h_pillar_37 = tICM_ini[baseAddr_pillar + 36];
		dm_icm_hue_segment_9_reg.h_pillar_38 = tICM_ini[baseAddr_pillar + 37];
		dm_icm_hue_segment_9_reg.h_pillar_39 = tICM_ini[baseAddr_pillar + 38];
		dm_icm_hue_segment_9_reg.h_pillar_40 = tICM_ini[baseAddr_pillar + 39];

		dm_icm_hue_segment_10_reg.h_pillar_41 = tICM_ini[baseAddr_pillar + 40];
		dm_icm_hue_segment_10_reg.h_pillar_42 = tICM_ini[baseAddr_pillar + 41];
		dm_icm_hue_segment_10_reg.h_pillar_43 = tICM_ini[baseAddr_pillar + 42];
		dm_icm_hue_segment_10_reg.h_pillar_44 = tICM_ini[baseAddr_pillar + 43];

		dm_icm_hue_segment_11_reg.h_pillar_45 = tICM_ini[baseAddr_pillar + 44];
		dm_icm_hue_segment_11_reg.h_pillar_46 = tICM_ini[baseAddr_pillar + 45];
		dm_icm_hue_segment_11_reg.h_pillar_47 = tICM_ini[baseAddr_pillar + 46];
		dm_icm_hue_segment_11_reg.h_pillar_48 = tICM_ini[baseAddr_pillar + 47];

		/*sat pillar*/
		dm_icm_sat_segment_0_reg.s_pillar_1 = tICM_ini[baseAddr_pillar + 48];
		dm_icm_sat_segment_0_reg.s_pillar_2 = tICM_ini[baseAddr_pillar + 49];
		dm_icm_sat_segment_0_reg.s_pillar_3 = tICM_ini[baseAddr_pillar + 50];
		dm_icm_sat_segment_0_reg.s_pillar_4 = tICM_ini[baseAddr_pillar + 51];
		dm_icm_sat_segment_0_reg.s_pillar_5 = tICM_ini[baseAddr_pillar + 52];
		dm_icm_sat_segment_0_reg.s_pillar_6 = tICM_ini[baseAddr_pillar + 53];
		dm_icm_sat_segment_0_reg.s_pillar_7 = tICM_ini[baseAddr_pillar + 54];
		dm_icm_sat_segment_0_reg.s_pillar_8 = tICM_ini[baseAddr_pillar + 55];
		/*itn pillar*/
		dm_icm_int_segment_0_reg.i_pillar_1 = tICM_ini[baseAddr_pillar + 56];
		dm_icm_int_segment_0_reg.i_pillar_2 = tICM_ini[baseAddr_pillar + 57];
		dm_icm_int_segment_0_reg.i_pillar_3 = tICM_ini[baseAddr_pillar + 58];
		dm_icm_int_segment_0_reg.i_pillar_4 = tICM_ini[baseAddr_pillar + 59];
		dm_icm_int_segment_0_reg.i_pillar_5 = tICM_ini[baseAddr_pillar + 60];
		dm_icm_int_segment_0_reg.i_pillar_6 = tICM_ini[baseAddr_pillar + 61];
		dm_icm_int_segment_0_reg.i_pillar_7 = tICM_ini[baseAddr_pillar + 62];
		dm_icm_int_segment_0_reg.i_pillar_8 = tICM_ini[baseAddr_pillar + 63];

		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_0_reg, dm_icm_hue_segment_0_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_1_reg, dm_icm_hue_segment_1_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_2_reg, dm_icm_hue_segment_2_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_3_reg, dm_icm_hue_segment_3_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_4_reg, dm_icm_hue_segment_4_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_5_reg, dm_icm_hue_segment_5_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_6_reg, dm_icm_hue_segment_6_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_7_reg, dm_icm_hue_segment_7_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_8_reg, dm_icm_hue_segment_8_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_9_reg, dm_icm_hue_segment_9_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_10_reg, dm_icm_hue_segment_10_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_11_reg, dm_icm_hue_segment_11_reg.regValue);

		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg, dm_icm_sat_segment_0_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg, dm_icm_int_segment_0_reg.regValue);

		/* icm table, hue/sat/itn*/
		{
		color_icm_dm_icm_3d_tab_sram_ctrl_RBUS        dm_icm_3d_tab_sram_ctrl_reg;
		color_icm_dm_icm_3d_tab_write_hsi_00_RBUS     dm_icm_3d_tab_write_hsi_00_reg;
		color_icm_dm_icm_3d_tab_write_hsi_01_RBUS     dm_icm_3d_tab_write_hsi_01_reg;
		color_icm_dm_icm_3d_tab_write_hsi_10_RBUS     dm_icm_3d_tab_write_hsi_10_reg;
		color_icm_dm_icm_3d_tab_write_hsi_11_RBUS     dm_icm_3d_tab_write_hsi_11_reg;
		color_icm_dm_icm_3d_tab_write_hsi_20_RBUS     dm_icm_3d_tab_write_hsi_20_reg;
		color_icm_dm_icm_3d_tab_write_hsi_21_RBUS     dm_icm_3d_tab_write_hsi_21_reg;
		color_icm_dm_icm_3d_tab_write_hsi_30_RBUS     dm_icm_3d_tab_write_hsi_30_reg;
		color_icm_dm_icm_3d_tab_write_hsi_31_RBUS     dm_icm_3d_tab_write_hsi_31_reg;
		color_icm_dm_icm_3d_tab_write_hsi_40_RBUS     dm_icm_3d_tab_write_hsi_40_reg;
		color_icm_dm_icm_3d_tab_write_hsi_41_RBUS     dm_icm_3d_tab_write_hsi_41_reg;
		color_icm_dm_icm_3d_tab_write_hsi_50_RBUS     dm_icm_3d_tab_write_hsi_50_reg;
		color_icm_dm_icm_3d_tab_write_hsi_51_RBUS     dm_icm_3d_tab_write_hsi_51_reg;
		color_icm_dm_icm_3d_tab_write_hsi_60_RBUS     dm_icm_3d_tab_write_hsi_60_reg;
		color_icm_dm_icm_3d_tab_write_hsi_61_RBUS     dm_icm_3d_tab_write_hsi_61_reg;
		color_icm_dm_icm_3d_tab_write_hsi_70_RBUS     dm_icm_3d_tab_write_hsi_70_reg;
		color_icm_dm_icm_3d_tab_write_hsi_71_RBUS     dm_icm_3d_tab_write_hsi_71_reg;
		color_icm_dm_icm_3d_tab_write_hsi_80_RBUS     dm_icm_3d_tab_write_hsi_80_reg;
		color_icm_dm_icm_3d_tab_write_hsi_81_RBUS     dm_icm_3d_tab_write_hsi_81_reg;
		color_icm_dm_icm_3d_tab_write_hsi_90_RBUS     dm_icm_3d_tab_write_hsi_90_reg;
		color_icm_dm_icm_3d_tab_write_hsi_91_RBUS     dm_icm_3d_tab_write_hsi_91_reg;
		color_icm_dm_icm_3d_tab_write_hsi_a0_RBUS     dm_icm_3d_tab_write_hsi_a0_reg;
		color_icm_dm_icm_3d_tab_write_hsi_a1_RBUS     dm_icm_3d_tab_write_hsi_a1_reg;
		color_icm_dm_icm_3d_tab_write_hsi_b0_RBUS     dm_icm_3d_tab_write_hsi_b0_reg;
		color_icm_dm_icm_3d_tab_write_hsi_b1_RBUS     dm_icm_3d_tab_write_hsi_b1_reg;

		h_sta = 0;
		h_end = HUESEGMAX - 1;
		si_sta = 0;
		si_end = SATSEGMAX - 1;
		if_write_ic = 1;
		LayerSize = HUESEGMAX * SATSEGMAX;

		/*write*/
		dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
		dm_icm_3d_tab_sram_ctrl_reg.sram_type = 0; /* base on H, S axis => Read/Write to I axis*/
		dm_icm_3d_tab_sram_ctrl_reg.write_random_access_en = 0; /*block write*/
		dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt = 0;
		IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
		for (si = si_sta; si <= si_end; si++) {
			for (h = h_sta; h <= h_end; h++) {
				dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_h = h;
				dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_s = si;
				dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_i = 0;

				dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = tICM_ini[baseAddr_hue_tab + LayerSize * 0  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = tICM_ini[baseAddr_sat_tab + LayerSize * 0  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = tICM_ini[baseAddr_itn_tab + LayerSize * 0  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_10_reg.write_table_h1 = tICM_ini[baseAddr_hue_tab + LayerSize * 1  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_10_reg.write_table_s1 = tICM_ini[baseAddr_sat_tab + LayerSize * 1  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_11_reg.write_table_i1 = tICM_ini[baseAddr_itn_tab + LayerSize * 1  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_20_reg.write_table_h2 = tICM_ini[baseAddr_hue_tab + LayerSize * 2  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_20_reg.write_table_s2 = tICM_ini[baseAddr_sat_tab + LayerSize * 2  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_21_reg.write_table_i2 = tICM_ini[baseAddr_itn_tab + LayerSize * 2  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_30_reg.write_table_h3 = tICM_ini[baseAddr_hue_tab + LayerSize * 3  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_30_reg.write_table_s3 = tICM_ini[baseAddr_sat_tab + LayerSize * 3  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_31_reg.write_table_i3 = tICM_ini[baseAddr_itn_tab + LayerSize * 3  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_40_reg.write_table_h4 = tICM_ini[baseAddr_hue_tab + LayerSize * 4  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_40_reg.write_table_s4 = tICM_ini[baseAddr_sat_tab + LayerSize * 4  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_41_reg.write_table_i4 = tICM_ini[baseAddr_itn_tab + LayerSize * 4  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_50_reg.write_table_h5 = tICM_ini[baseAddr_hue_tab + LayerSize * 5  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_50_reg.write_table_s5 = tICM_ini[baseAddr_sat_tab + LayerSize * 5  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_51_reg.write_table_i5 = tICM_ini[baseAddr_itn_tab + LayerSize * 5  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_60_reg.write_table_h6 = tICM_ini[baseAddr_hue_tab + LayerSize * 6  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_60_reg.write_table_s6 = tICM_ini[baseAddr_sat_tab + LayerSize * 6  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_61_reg.write_table_i6 = tICM_ini[baseAddr_itn_tab + LayerSize * 6  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_70_reg.write_table_h7 = tICM_ini[baseAddr_hue_tab + LayerSize * 7  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_70_reg.write_table_s7 = tICM_ini[baseAddr_sat_tab + LayerSize * 7  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_71_reg.write_table_i7 = tICM_ini[baseAddr_itn_tab + LayerSize * 7  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_80_reg.write_table_h8 = tICM_ini[baseAddr_hue_tab + LayerSize * 8  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_80_reg.write_table_s8 = tICM_ini[baseAddr_sat_tab + LayerSize * 8  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_81_reg.write_table_i8 = tICM_ini[baseAddr_itn_tab + LayerSize * 8  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_90_reg.write_table_h9 = tICM_ini[baseAddr_hue_tab + LayerSize * 9  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_90_reg.write_table_s9 = tICM_ini[baseAddr_sat_tab + LayerSize * 9  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_91_reg.write_table_i9 = tICM_ini[baseAddr_itn_tab + LayerSize * 9  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_a0_reg.write_table_ha = tICM_ini[baseAddr_hue_tab + LayerSize * 10 + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_a0_reg.write_table_sa = tICM_ini[baseAddr_sat_tab + LayerSize * 10 + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_a1_reg.write_table_ia = tICM_ini[baseAddr_itn_tab + LayerSize * 10 + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_b0_reg.write_table_hb = tICM_ini[baseAddr_hue_tab + LayerSize * 11 + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_b0_reg.write_table_sb = tICM_ini[baseAddr_sat_tab + LayerSize * 11 + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_b1_reg.write_table_ib = tICM_ini[baseAddr_itn_tab + LayerSize * 11 + si * HUESEGMAX + h];

				/*write*/
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_00_reg, dm_icm_3d_tab_write_hsi_00_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_01_reg, dm_icm_3d_tab_write_hsi_01_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_10_reg, dm_icm_3d_tab_write_hsi_10_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_11_reg, dm_icm_3d_tab_write_hsi_11_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_20_reg, dm_icm_3d_tab_write_hsi_20_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_21_reg, dm_icm_3d_tab_write_hsi_21_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_30_reg, dm_icm_3d_tab_write_hsi_30_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_31_reg, dm_icm_3d_tab_write_hsi_31_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_40_reg, dm_icm_3d_tab_write_hsi_40_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_41_reg, dm_icm_3d_tab_write_hsi_41_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_50_reg, dm_icm_3d_tab_write_hsi_50_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_51_reg, dm_icm_3d_tab_write_hsi_51_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_60_reg, dm_icm_3d_tab_write_hsi_60_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_61_reg, dm_icm_3d_tab_write_hsi_61_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_70_reg, dm_icm_3d_tab_write_hsi_70_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_71_reg, dm_icm_3d_tab_write_hsi_71_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_80_reg, dm_icm_3d_tab_write_hsi_80_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_81_reg, dm_icm_3d_tab_write_hsi_81_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_90_reg, dm_icm_3d_tab_write_hsi_90_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_91_reg, dm_icm_3d_tab_write_hsi_91_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_A0_reg, dm_icm_3d_tab_write_hsi_a0_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_A1_reg, dm_icm_3d_tab_write_hsi_a1_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_B0_reg, dm_icm_3d_tab_write_hsi_b0_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_B1_reg, dm_icm_3d_tab_write_hsi_b1_reg.regValue);

				dm_icm_3d_tab_sram_ctrl_reg.write_en = 1;
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
				timeout = 20;
				do {
					dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
				} while (dm_icm_3d_tab_sram_ctrl_reg.write_en && (timeout-- > 0));

				if (timeout <= 0) {
					VIPprintf("Color_ICM_Table_Select write icm table error: sram_addrres_h = %d, sram_addrres_s = %d\n", h, si);
					return;
				}
			}
		}
		}
	}   /* end main*/
#ifdef CONFIG_DUAL_CHANNEL
	else {	/* For Sub*/
		/**/
	}
#endif
}

unsigned char drvif_color_icm_set_ctrl(unsigned short table_mode, unsigned short hsi_grid_sel, unsigned short out_grid_process, unsigned short y_ctrl)
{
	color_icm_dm_icm_ctrl_RBUS				  dm_icm_ctrl_reg;
	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS	      dm_icm_3d_tab_sram_ctrl_reg;

	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	dm_icm_ctrl_reg.table_mode			= table_mode;
	dm_icm_ctrl_reg.hsi_grid_sel		= hsi_grid_sel;
	dm_icm_ctrl_reg.out_grid_process	= out_grid_process;
	dm_icm_ctrl_reg.y_ctrl				= y_ctrl;

	dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
	dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt = 0;

	IoReg_Write32(COLOR_ICM_DM_ICM_CTRL_reg, dm_icm_ctrl_reg.regValue);
	IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);

	return true;
}

unsigned char drvif_color_icm_set_uv_coring(unsigned short u_coring, unsigned short v_coring)
{
	color_icm_dm_uv_coring_RBUS 			  dm_uv_coring_reg;
	dm_uv_coring_reg.regValue = IoReg_Read32(COLOR_ICM_DM_UV_Coring_reg);

	dm_uv_coring_reg.u_coring = u_coring;
	dm_uv_coring_reg.v_coring = v_coring;

	IoReg_Write32(COLOR_ICM_DM_UV_Coring_reg, dm_uv_coring_reg.regValue);

	return true;
}

unsigned char drvif_color_icm_Set_global(int g_dhue_cur, int *g_dsatbysat_cur, int *g_ditnbyitn_cur)
{
	/* 3. update current global hue/sat/itn*/
	/*g_dhue*/
	g_dhue = g_dhue_cur;
	/*g_dsatbysat*/
	g_dsatbysat[0]	= g_dsatbysat_cur[0];
	g_dsatbysat[1]	= g_dsatbysat_cur[1];
	g_dsatbysat[2]	= g_dsatbysat_cur[2];
	g_dsatbysat[3]	= g_dsatbysat_cur[3];
	g_dsatbysat[4]	= g_dsatbysat_cur[4];
	g_dsatbysat[5]	= g_dsatbysat_cur[5];
	g_dsatbysat[6]	= g_dsatbysat_cur[6];
	g_dsatbysat[7]	= g_dsatbysat_cur[7];
	g_dsatbysat[8]	= g_dsatbysat_cur[8];
	g_dsatbysat[9]	= g_dsatbysat_cur[9];
	g_dsatbysat[10]	= g_dsatbysat_cur[10];
	g_dsatbysat[11]	= g_dsatbysat_cur[11];
	/*g_ditnbyitn*/
	g_ditnbyitn[0]	= g_ditnbyitn_cur[0];
	g_ditnbyitn[1]	= g_ditnbyitn_cur[1];
	g_ditnbyitn[2]	= g_ditnbyitn_cur[2];
	g_ditnbyitn[3]	= g_ditnbyitn_cur[3];
	g_ditnbyitn[4]	= g_ditnbyitn_cur[4];
	g_ditnbyitn[5]	= g_ditnbyitn_cur[5];
	g_ditnbyitn[6]	= g_ditnbyitn_cur[6];
	g_ditnbyitn[7]	= g_ditnbyitn_cur[7];
	g_ditnbyitn[8]	= g_ditnbyitn_cur[8];
	g_ditnbyitn[9]	= g_ditnbyitn_cur[9];
	g_ditnbyitn[10]	= g_ditnbyitn_cur[10];
	g_ditnbyitn[11]	= g_ditnbyitn_cur[11];

	return true;
}

unsigned char drvif_color_icm_save_global_of_viptable(unsigned short dhue_cur, unsigned short *dsatbysat_cur, unsigned short *ditnbyitn_cur)
{
	int i;
	/*record vip table value*/
	/*g_dhue*/
	g_dhue_tab			= dhue_cur;
	g_dhue_tab			= ((ICM_GLOBAL_BIT_MAP *)&g_dhue_tab)->value;

	for (i = 0; i <= 11; i++) {
		/*g_dsatbysat*/
		g_dsatbysat_tab[i]	= dsatbysat_cur[i];
		g_dsatbysat_tab[i] = ((ICM_GLOBAL_BIT_MAP *)(g_dsatbysat_tab+i))->value;
		/*g_ditnbyitn*/
		g_ditnbyitn_tab[i]	= ditnbyitn_cur[i];
		g_ditnbyitn_tab[i] = ((ICM_GLOBAL_BIT_MAP *)(g_ditnbyitn_tab+i))->value;
	}

	return true;
}

unsigned char drvif_color_icm_set_pillar(unsigned short *h_pillar, unsigned short *s_pillar, unsigned short *i_pillar)
{
	color_icm_dm_icm_hue_segment_0_RBUS	dm_icm_hue_segment_0_reg;
	color_icm_dm_icm_hue_segment_1_RBUS	dm_icm_hue_segment_1_reg;
	color_icm_dm_icm_hue_segment_2_RBUS	dm_icm_hue_segment_2_reg;
	color_icm_dm_icm_hue_segment_3_RBUS	dm_icm_hue_segment_3_reg;
	color_icm_dm_icm_hue_segment_4_RBUS	dm_icm_hue_segment_4_reg;
	color_icm_dm_icm_hue_segment_5_RBUS	dm_icm_hue_segment_5_reg;
	color_icm_dm_icm_hue_segment_6_RBUS	dm_icm_hue_segment_6_reg;
	color_icm_dm_icm_hue_segment_7_RBUS	dm_icm_hue_segment_7_reg;
	color_icm_dm_icm_hue_segment_8_RBUS	dm_icm_hue_segment_8_reg;
	color_icm_dm_icm_hue_segment_9_RBUS	dm_icm_hue_segment_9_reg;
	color_icm_dm_icm_hue_segment_10_RBUS	dm_icm_hue_segment_10_reg;
	color_icm_dm_icm_hue_segment_11_RBUS	dm_icm_hue_segment_11_reg;
	color_icm_dm_icm_sat_segment_0_RBUS	dm_icm_sat_segment_0_reg;
	color_icm_dm_icm_int_segment_0_RBUS	dm_icm_int_segment_0_reg;

	#if 0 /* write full register, don't need read*/
	dm_icm_hue_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_0_reg);
	dm_icm_hue_segment_1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_1_reg);
	dm_icm_hue_segment_2_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_2_reg);
	dm_icm_hue_segment_3_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_3_reg);
	dm_icm_hue_segment_4_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_4_reg);
	dm_icm_hue_segment_5_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_5_reg);
	dm_icm_hue_segment_6_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_6_reg);
	dm_icm_hue_segment_7_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_7_reg);
	dm_icm_hue_segment_8_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_8_reg);
	dm_icm_hue_segment_9_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_9_reg);
	dm_icm_hue_segment_10_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_10_reg);
	dm_icm_hue_segment_11_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_11_reg);

	dm_icm_sat_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg);
	dm_icm_int_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg);
	#endif

	/*hue pillar*/
	if (NULL != h_pillar) {
		dm_icm_hue_segment_0_reg.h_pillar_1 = h_pillar[0];
		dm_icm_hue_segment_0_reg.h_pillar_2 = h_pillar[1];
		dm_icm_hue_segment_0_reg.h_pillar_3 = h_pillar[2];
		dm_icm_hue_segment_0_reg.h_pillar_4 = h_pillar[3];

		dm_icm_hue_segment_1_reg.h_pillar_5 = h_pillar[4];
		dm_icm_hue_segment_1_reg.h_pillar_6 = h_pillar[5];
		dm_icm_hue_segment_1_reg.h_pillar_7 = h_pillar[6];
		dm_icm_hue_segment_1_reg.h_pillar_8 = h_pillar[7];

		dm_icm_hue_segment_2_reg.h_pillar_9  = h_pillar[8];
		dm_icm_hue_segment_2_reg.h_pillar_10 = h_pillar[9];
		dm_icm_hue_segment_2_reg.h_pillar_11 = h_pillar[10];
		dm_icm_hue_segment_2_reg.h_pillar_12 = h_pillar[11];

		dm_icm_hue_segment_3_reg.h_pillar_13 = h_pillar[12];
		dm_icm_hue_segment_3_reg.h_pillar_14 = h_pillar[13];
		dm_icm_hue_segment_3_reg.h_pillar_15 = h_pillar[14];
		dm_icm_hue_segment_3_reg.h_pillar_16 = h_pillar[15];

		dm_icm_hue_segment_4_reg.h_pillar_17 = h_pillar[16];
		dm_icm_hue_segment_4_reg.h_pillar_18 = h_pillar[17];
		dm_icm_hue_segment_4_reg.h_pillar_19 = h_pillar[18];
		dm_icm_hue_segment_4_reg.h_pillar_20 = h_pillar[19];

		dm_icm_hue_segment_5_reg.h_pillar_21 = h_pillar[20];
		dm_icm_hue_segment_5_reg.h_pillar_22 = h_pillar[21];
		dm_icm_hue_segment_5_reg.h_pillar_23 = h_pillar[22];
		dm_icm_hue_segment_5_reg.h_pillar_24 = h_pillar[23];

		dm_icm_hue_segment_6_reg.h_pillar_25 = h_pillar[24];
		dm_icm_hue_segment_6_reg.h_pillar_26 = h_pillar[25];
		dm_icm_hue_segment_6_reg.h_pillar_27 = h_pillar[26];
		dm_icm_hue_segment_6_reg.h_pillar_28 = h_pillar[27];

		dm_icm_hue_segment_7_reg.h_pillar_29 = h_pillar[28];
		dm_icm_hue_segment_7_reg.h_pillar_30 = h_pillar[29];
		dm_icm_hue_segment_7_reg.h_pillar_31 = h_pillar[30];
		dm_icm_hue_segment_7_reg.h_pillar_32 = h_pillar[31];

		dm_icm_hue_segment_8_reg.h_pillar_33 = h_pillar[32];
		dm_icm_hue_segment_8_reg.h_pillar_34 = h_pillar[33];
		dm_icm_hue_segment_8_reg.h_pillar_35 = h_pillar[34];
		dm_icm_hue_segment_8_reg.h_pillar_36 = h_pillar[35];

		dm_icm_hue_segment_9_reg.h_pillar_37 = h_pillar[36];
		dm_icm_hue_segment_9_reg.h_pillar_38 = h_pillar[37];
		dm_icm_hue_segment_9_reg.h_pillar_39 = h_pillar[38];
		dm_icm_hue_segment_9_reg.h_pillar_40 = h_pillar[39];

		dm_icm_hue_segment_10_reg.h_pillar_41 = h_pillar[40];
		dm_icm_hue_segment_10_reg.h_pillar_42 = h_pillar[41];
		dm_icm_hue_segment_10_reg.h_pillar_43 = h_pillar[42];
		dm_icm_hue_segment_10_reg.h_pillar_44 = h_pillar[43];

		dm_icm_hue_segment_11_reg.h_pillar_45 = h_pillar[44];
		dm_icm_hue_segment_11_reg.h_pillar_46 = h_pillar[45];
		dm_icm_hue_segment_11_reg.h_pillar_47 = h_pillar[46];
		dm_icm_hue_segment_11_reg.h_pillar_48 = h_pillar[47];

		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_0_reg, dm_icm_hue_segment_0_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_1_reg, dm_icm_hue_segment_1_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_2_reg, dm_icm_hue_segment_2_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_3_reg, dm_icm_hue_segment_3_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_4_reg, dm_icm_hue_segment_4_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_5_reg, dm_icm_hue_segment_5_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_6_reg, dm_icm_hue_segment_6_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_7_reg, dm_icm_hue_segment_7_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_8_reg, dm_icm_hue_segment_8_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_9_reg, dm_icm_hue_segment_9_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_10_reg, dm_icm_hue_segment_10_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_11_reg, dm_icm_hue_segment_11_reg.regValue);
	}

	/*sat pillar*/
	if (NULL != s_pillar) {
		dm_icm_sat_segment_0_reg.s_pillar_1 = s_pillar[0];
		dm_icm_sat_segment_0_reg.s_pillar_2 = s_pillar[1];
		dm_icm_sat_segment_0_reg.s_pillar_3 = s_pillar[2];
		dm_icm_sat_segment_0_reg.s_pillar_4 = s_pillar[3];
		dm_icm_sat_segment_0_reg.s_pillar_5 = s_pillar[4];
		dm_icm_sat_segment_0_reg.s_pillar_6 = s_pillar[5];
		dm_icm_sat_segment_0_reg.s_pillar_7 = s_pillar[6];
		dm_icm_sat_segment_0_reg.s_pillar_8 = s_pillar[7];

		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg, dm_icm_sat_segment_0_reg.regValue);
	}

	/*itn pillar*/
	if (NULL != i_pillar) {
		dm_icm_int_segment_0_reg.i_pillar_1 = i_pillar[0];
		dm_icm_int_segment_0_reg.i_pillar_2 = i_pillar[1];
		dm_icm_int_segment_0_reg.i_pillar_3 = i_pillar[2];
		dm_icm_int_segment_0_reg.i_pillar_4 = i_pillar[3];
		dm_icm_int_segment_0_reg.i_pillar_5 = i_pillar[4];
		dm_icm_int_segment_0_reg.i_pillar_6 = i_pillar[5];
		dm_icm_int_segment_0_reg.i_pillar_7 = i_pillar[6];
		dm_icm_int_segment_0_reg.i_pillar_8 = i_pillar[7];

		IoReg_Write32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg, dm_icm_int_segment_0_reg.regValue);
	}

	return true;
}
/*write global hue/sat/itn*/
/*flow:*/
/*1.read back icm table*/
/*  (1)subtract previous g_dhue/g_dsatbysat/g_ditnbyitn from icm table*/
/*  (2)add current g_dhue/g_dsatbysat/g_ditnbyitn to icm table*/
/*2.write icm table*/
/*3.update current global hue/sat/itn*/
unsigned char drvif_color_icm_global_adjust(int g_dhue_cur, int *g_dsatbysat_cur, int *g_ditnbyitn_cur, unsigned char bSubtractPrevious)
{
	int k, timeout;
	int h, si;
	int h_sta, h_end, si_sta, si_end;

	static COLORELEM ICM_TAB[ITNSEGMAX][SATSEGMAX][HUESEGMAX];

	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS        dm_icm_3d_tab_sram_ctrl_reg;
	color_icm_dm_icm_3d_tab_write_hsi_00_RBUS     dm_icm_3d_tab_write_hsi_00_reg;
	color_icm_dm_icm_3d_tab_write_hsi_01_RBUS     dm_icm_3d_tab_write_hsi_01_reg;
	color_icm_dm_icm_3d_tab_write_hsi_10_RBUS     dm_icm_3d_tab_write_hsi_10_reg;
	color_icm_dm_icm_3d_tab_write_hsi_11_RBUS     dm_icm_3d_tab_write_hsi_11_reg;
	color_icm_dm_icm_3d_tab_write_hsi_20_RBUS     dm_icm_3d_tab_write_hsi_20_reg;
	color_icm_dm_icm_3d_tab_write_hsi_21_RBUS     dm_icm_3d_tab_write_hsi_21_reg;
	color_icm_dm_icm_3d_tab_write_hsi_30_RBUS     dm_icm_3d_tab_write_hsi_30_reg;
	color_icm_dm_icm_3d_tab_write_hsi_31_RBUS     dm_icm_3d_tab_write_hsi_31_reg;
	color_icm_dm_icm_3d_tab_write_hsi_40_RBUS     dm_icm_3d_tab_write_hsi_40_reg;
	color_icm_dm_icm_3d_tab_write_hsi_41_RBUS     dm_icm_3d_tab_write_hsi_41_reg;
	color_icm_dm_icm_3d_tab_write_hsi_50_RBUS     dm_icm_3d_tab_write_hsi_50_reg;
	color_icm_dm_icm_3d_tab_write_hsi_51_RBUS     dm_icm_3d_tab_write_hsi_51_reg;
	color_icm_dm_icm_3d_tab_write_hsi_60_RBUS     dm_icm_3d_tab_write_hsi_60_reg;
	color_icm_dm_icm_3d_tab_write_hsi_61_RBUS     dm_icm_3d_tab_write_hsi_61_reg;
	color_icm_dm_icm_3d_tab_write_hsi_70_RBUS     dm_icm_3d_tab_write_hsi_70_reg;
	color_icm_dm_icm_3d_tab_write_hsi_71_RBUS     dm_icm_3d_tab_write_hsi_71_reg;
	color_icm_dm_icm_3d_tab_write_hsi_80_RBUS     dm_icm_3d_tab_write_hsi_80_reg;
	color_icm_dm_icm_3d_tab_write_hsi_81_RBUS     dm_icm_3d_tab_write_hsi_81_reg;
	color_icm_dm_icm_3d_tab_write_hsi_90_RBUS     dm_icm_3d_tab_write_hsi_90_reg;
	color_icm_dm_icm_3d_tab_write_hsi_91_RBUS     dm_icm_3d_tab_write_hsi_91_reg;
	color_icm_dm_icm_3d_tab_write_hsi_a0_RBUS     dm_icm_3d_tab_write_hsi_a0_reg;
	color_icm_dm_icm_3d_tab_write_hsi_a1_RBUS     dm_icm_3d_tab_write_hsi_a1_reg;
	color_icm_dm_icm_3d_tab_write_hsi_b0_RBUS     dm_icm_3d_tab_write_hsi_b0_reg;
	color_icm_dm_icm_3d_tab_write_hsi_b1_RBUS     dm_icm_3d_tab_write_hsi_b1_reg;

	color_icm_dm_icm_3d_tab_read_hsi_00_RBUS      dm_icm_3d_tab_read_hsi_00_reg;
	color_icm_dm_icm_3d_tab_read_hsi_01_RBUS      dm_icm_3d_tab_read_hsi_01_reg;
	color_icm_dm_icm_3d_tab_read_hsi_10_RBUS      dm_icm_3d_tab_read_hsi_10_reg;
	color_icm_dm_icm_3d_tab_read_hsi_11_RBUS      dm_icm_3d_tab_read_hsi_11_reg;
	color_icm_dm_icm_3d_tab_read_hsi_20_RBUS      dm_icm_3d_tab_read_hsi_20_reg;
	color_icm_dm_icm_3d_tab_read_hsi_21_RBUS      dm_icm_3d_tab_read_hsi_21_reg;
	color_icm_dm_icm_3d_tab_read_hsi_30_RBUS      dm_icm_3d_tab_read_hsi_30_reg;
	color_icm_dm_icm_3d_tab_read_hsi_31_RBUS      dm_icm_3d_tab_read_hsi_31_reg;
	color_icm_dm_icm_3d_tab_read_hsi_40_RBUS      dm_icm_3d_tab_read_hsi_40_reg;
	color_icm_dm_icm_3d_tab_read_hsi_41_RBUS      dm_icm_3d_tab_read_hsi_41_reg;
	color_icm_dm_icm_3d_tab_read_hsi_50_RBUS      dm_icm_3d_tab_read_hsi_50_reg;
	color_icm_dm_icm_3d_tab_read_hsi_51_RBUS      dm_icm_3d_tab_read_hsi_51_reg;

	h_sta = 0;
	h_end = HUESEGMAX - 1;
	si_sta = 0;
	si_end = SATSEGMAX - 1;

	/* 1.read back*/
	dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
	dm_icm_3d_tab_sram_ctrl_reg.sram_type = 0; /* base on H, S axis => Read/Write to I axis*/
	dm_icm_3d_tab_sram_ctrl_reg.write_random_access_en = 0; /*clear*/
	dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt = 0;
	for (si = si_sta; si <= si_end; si++) {
		for (h = h_sta; h <= h_end; h++) {
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_h = h;
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_s = si;
			for (k = 0; k < 2; k++) {
				dm_icm_3d_tab_sram_ctrl_reg.read_sel = k;
				dm_icm_3d_tab_sram_ctrl_reg.read_en = 1;
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
				timeout = 20;
				do {
					dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
				} while (dm_icm_3d_tab_sram_ctrl_reg.read_en && (timeout-- > 0));
				if (timeout <= 0) {
					VIPprintf("drvif_color_icm_global_adjust read icm table error: sram_addrres_h = %d, sram_addrres_s = %d\n", h, si);
					return false;
				}

				dm_icm_3d_tab_read_hsi_00_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_00_reg);
				dm_icm_3d_tab_read_hsi_01_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_01_reg);
				dm_icm_3d_tab_read_hsi_10_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_10_reg);
				dm_icm_3d_tab_read_hsi_11_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_11_reg);
				dm_icm_3d_tab_read_hsi_20_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_20_reg);
				dm_icm_3d_tab_read_hsi_21_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_21_reg);
				dm_icm_3d_tab_read_hsi_30_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_30_reg);
				dm_icm_3d_tab_read_hsi_31_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_31_reg);
				dm_icm_3d_tab_read_hsi_40_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_40_reg);
				dm_icm_3d_tab_read_hsi_41_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_41_reg);
				dm_icm_3d_tab_read_hsi_50_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_50_reg);
				dm_icm_3d_tab_read_hsi_51_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_51_reg);

				ICM_TAB[k * 6 + 0][si][h].H = dm_icm_3d_tab_read_hsi_00_reg.read_table_h0;
				ICM_TAB[k * 6 + 0][si][h].S = dm_icm_3d_tab_read_hsi_00_reg.read_table_s0;
				ICM_TAB[k * 6 + 0][si][h].I = dm_icm_3d_tab_read_hsi_01_reg.read_table_i0;
				ICM_TAB[k * 6 + 1][si][h].H = dm_icm_3d_tab_read_hsi_10_reg.read_table_h1;
				ICM_TAB[k * 6 + 1][si][h].S = dm_icm_3d_tab_read_hsi_10_reg.read_table_s1;
				ICM_TAB[k * 6 + 1][si][h].I = dm_icm_3d_tab_read_hsi_11_reg.read_table_i1;
				ICM_TAB[k * 6 + 2][si][h].H = dm_icm_3d_tab_read_hsi_20_reg.read_table_h2;
				ICM_TAB[k * 6 + 2][si][h].S = dm_icm_3d_tab_read_hsi_20_reg.read_table_s2;
				ICM_TAB[k * 6 + 2][si][h].I = dm_icm_3d_tab_read_hsi_21_reg.read_table_i2;
				ICM_TAB[k * 6 + 3][si][h].H = dm_icm_3d_tab_read_hsi_30_reg.read_table_h3;
				ICM_TAB[k * 6 + 3][si][h].S = dm_icm_3d_tab_read_hsi_30_reg.read_table_s3;
				ICM_TAB[k * 6 + 3][si][h].I = dm_icm_3d_tab_read_hsi_31_reg.read_table_i3;
				ICM_TAB[k * 6 + 4][si][h].H = dm_icm_3d_tab_read_hsi_40_reg.read_table_h4;
				ICM_TAB[k * 6 + 4][si][h].S = dm_icm_3d_tab_read_hsi_40_reg.read_table_s4;
				ICM_TAB[k * 6 + 4][si][h].I = dm_icm_3d_tab_read_hsi_41_reg.read_table_i4;
				ICM_TAB[k * 6 + 5][si][h].H = dm_icm_3d_tab_read_hsi_50_reg.read_table_h5;
				ICM_TAB[k * 6 + 5][si][h].S = dm_icm_3d_tab_read_hsi_50_reg.read_table_s5;
				ICM_TAB[k * 6 + 5][si][h].I = dm_icm_3d_tab_read_hsi_51_reg.read_table_i5;



				/* (1)subtract previous global hue/sat/itn*/
				if (bSubtractPrevious) {
					ICM_TAB[k * 6 + 0][si][h].H -= g_dhue;
					ICM_TAB[k * 6 + 1][si][h].H -= g_dhue;
					ICM_TAB[k * 6 + 2][si][h].H -= g_dhue;
					ICM_TAB[k * 6 + 3][si][h].H -= g_dhue;
					ICM_TAB[k * 6 + 4][si][h].H -= g_dhue;
					ICM_TAB[k * 6 + 5][si][h].H -= g_dhue;

					ICM_TAB[k * 6 + 0][si][h].S -= g_dsatbysat[si];
					ICM_TAB[k * 6 + 1][si][h].S -= g_dsatbysat[si];
					ICM_TAB[k * 6 + 2][si][h].S -= g_dsatbysat[si];
					ICM_TAB[k * 6 + 3][si][h].S -= g_dsatbysat[si];
					ICM_TAB[k * 6 + 4][si][h].S -= g_dsatbysat[si];
					ICM_TAB[k * 6 + 5][si][h].S -= g_dsatbysat[si];

					ICM_TAB[k * 6 + 0][si][h].I -= g_ditnbyitn[k * 6 + 0];
					ICM_TAB[k * 6 + 1][si][h].I -= g_ditnbyitn[k * 6 + 1];
					ICM_TAB[k * 6 + 2][si][h].I -= g_ditnbyitn[k * 6 + 2];
					ICM_TAB[k * 6 + 3][si][h].I -= g_ditnbyitn[k * 6 + 3];
					ICM_TAB[k * 6 + 4][si][h].I -= g_ditnbyitn[k * 6 + 4];
					ICM_TAB[k * 6 + 5][si][h].I -= g_ditnbyitn[k * 6 + 5];
				}

				/* (2)add current global hue/sat/itn*/
				ICM_TAB[k * 6 + 0][si][h].H = (ICM_TAB[k * 6 + 0][si][h].H + g_dhue_cur + HUERANGE) % HUERANGE;
				ICM_TAB[k * 6 + 1][si][h].H = (ICM_TAB[k * 6 + 1][si][h].H + g_dhue_cur + HUERANGE) % HUERANGE;
				ICM_TAB[k * 6 + 2][si][h].H = (ICM_TAB[k * 6 + 2][si][h].H + g_dhue_cur + HUERANGE) % HUERANGE;
				ICM_TAB[k * 6 + 3][si][h].H = (ICM_TAB[k * 6 + 3][si][h].H + g_dhue_cur + HUERANGE) % HUERANGE;
				ICM_TAB[k * 6 + 4][si][h].H = (ICM_TAB[k * 6 + 4][si][h].H + g_dhue_cur + HUERANGE) % HUERANGE;
				ICM_TAB[k * 6 + 5][si][h].H = (ICM_TAB[k * 6 + 5][si][h].H + g_dhue_cur + HUERANGE) % HUERANGE;

				ICM_TAB[k * 6 + 0][si][h].S = CLIP(0, SATRANGEEX, ICM_TAB[k * 6 + 0][si][h].S + g_dsatbysat_cur[si]);
				ICM_TAB[k * 6 + 1][si][h].S = CLIP(0, SATRANGEEX, ICM_TAB[k * 6 + 1][si][h].S + g_dsatbysat_cur[si]);
				ICM_TAB[k * 6 + 2][si][h].S = CLIP(0, SATRANGEEX, ICM_TAB[k * 6 + 2][si][h].S + g_dsatbysat_cur[si]);
				ICM_TAB[k * 6 + 3][si][h].S = CLIP(0, SATRANGEEX, ICM_TAB[k * 6 + 3][si][h].S + g_dsatbysat_cur[si]);
				ICM_TAB[k * 6 + 4][si][h].S = CLIP(0, SATRANGEEX, ICM_TAB[k * 6 + 4][si][h].S + g_dsatbysat_cur[si]);
				ICM_TAB[k * 6 + 5][si][h].S = CLIP(0, SATRANGEEX, ICM_TAB[k * 6 + 5][si][h].S + g_dsatbysat_cur[si]);

				ICM_TAB[k * 6 + 0][si][h].I = CLIP(0, ITNRANGEEX, ICM_TAB[k * 6 + 0][si][h].I + g_ditnbyitn_cur[k * 6 + 0]);
				ICM_TAB[k * 6 + 1][si][h].I = CLIP(0, ITNRANGEEX, ICM_TAB[k * 6 + 1][si][h].I + g_ditnbyitn_cur[k * 6 + 1]);
				ICM_TAB[k * 6 + 2][si][h].I = CLIP(0, ITNRANGEEX, ICM_TAB[k * 6 + 2][si][h].I + g_ditnbyitn_cur[k * 6 + 2]);
				ICM_TAB[k * 6 + 3][si][h].I = CLIP(0, ITNRANGEEX, ICM_TAB[k * 6 + 3][si][h].I + g_ditnbyitn_cur[k * 6 + 3]);
				ICM_TAB[k * 6 + 4][si][h].I = CLIP(0, ITNRANGEEX, ICM_TAB[k * 6 + 4][si][h].I + g_ditnbyitn_cur[k * 6 + 4]);
				ICM_TAB[k * 6 + 5][si][h].I = CLIP(0, ITNRANGEEX, ICM_TAB[k * 6 + 5][si][h].I + g_ditnbyitn_cur[k * 6 + 5]);
			}
		}
	}
	/* 2.write*/
	dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
	dm_icm_3d_tab_sram_ctrl_reg.sram_type = 0; /* base on H, S axis => Read/Write to I axis*/
	dm_icm_3d_tab_sram_ctrl_reg.write_random_access_en = 0; /*block write*/
	IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
	for (si = si_sta; si <= si_end; si++) {
		for (h = h_sta; h <= h_end; h++) {
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_h = h;
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_s = si;
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_i = 0;

			dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = ICM_TAB[0][si][h].H;
			dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = ICM_TAB[0][si][h].S;
			dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = ICM_TAB[0][si][h].I;
			dm_icm_3d_tab_write_hsi_10_reg.write_table_h1 = ICM_TAB[1][si][h].H;
			dm_icm_3d_tab_write_hsi_10_reg.write_table_s1 = ICM_TAB[1][si][h].S;
			dm_icm_3d_tab_write_hsi_11_reg.write_table_i1 = ICM_TAB[1][si][h].I;
			dm_icm_3d_tab_write_hsi_20_reg.write_table_h2 = ICM_TAB[2][si][h].H;
			dm_icm_3d_tab_write_hsi_20_reg.write_table_s2 = ICM_TAB[2][si][h].S;
			dm_icm_3d_tab_write_hsi_21_reg.write_table_i2 = ICM_TAB[2][si][h].I;
			dm_icm_3d_tab_write_hsi_30_reg.write_table_h3 = ICM_TAB[3][si][h].H;
			dm_icm_3d_tab_write_hsi_30_reg.write_table_s3 = ICM_TAB[3][si][h].S;
			dm_icm_3d_tab_write_hsi_31_reg.write_table_i3 = ICM_TAB[3][si][h].I;
			dm_icm_3d_tab_write_hsi_40_reg.write_table_h4 = ICM_TAB[4][si][h].H;
			dm_icm_3d_tab_write_hsi_40_reg.write_table_s4 = ICM_TAB[4][si][h].S;
			dm_icm_3d_tab_write_hsi_41_reg.write_table_i4 = ICM_TAB[4][si][h].I;
			dm_icm_3d_tab_write_hsi_50_reg.write_table_h5 = ICM_TAB[5][si][h].H;
			dm_icm_3d_tab_write_hsi_50_reg.write_table_s5 = ICM_TAB[5][si][h].S;
			dm_icm_3d_tab_write_hsi_51_reg.write_table_i5 = ICM_TAB[5][si][h].I;
			dm_icm_3d_tab_write_hsi_60_reg.write_table_h6 = ICM_TAB[6][si][h].H;
			dm_icm_3d_tab_write_hsi_60_reg.write_table_s6 = ICM_TAB[6][si][h].S;
			dm_icm_3d_tab_write_hsi_61_reg.write_table_i6 = ICM_TAB[6][si][h].I;
			dm_icm_3d_tab_write_hsi_70_reg.write_table_h7 = ICM_TAB[7][si][h].H;
			dm_icm_3d_tab_write_hsi_70_reg.write_table_s7 = ICM_TAB[7][si][h].S;
			dm_icm_3d_tab_write_hsi_71_reg.write_table_i7 = ICM_TAB[7][si][h].I;
			dm_icm_3d_tab_write_hsi_80_reg.write_table_h8 = ICM_TAB[8][si][h].H;
			dm_icm_3d_tab_write_hsi_80_reg.write_table_s8 = ICM_TAB[8][si][h].S;
			dm_icm_3d_tab_write_hsi_81_reg.write_table_i8 = ICM_TAB[8][si][h].I;
			dm_icm_3d_tab_write_hsi_90_reg.write_table_h9 = ICM_TAB[9][si][h].H;
			dm_icm_3d_tab_write_hsi_90_reg.write_table_s9 = ICM_TAB[9][si][h].S;
			dm_icm_3d_tab_write_hsi_91_reg.write_table_i9 = ICM_TAB[9][si][h].I;
			dm_icm_3d_tab_write_hsi_a0_reg.write_table_ha = ICM_TAB[10][si][h].H;
			dm_icm_3d_tab_write_hsi_a0_reg.write_table_sa = ICM_TAB[10][si][h].S;
			dm_icm_3d_tab_write_hsi_a1_reg.write_table_ia = ICM_TAB[10][si][h].I;
			dm_icm_3d_tab_write_hsi_b0_reg.write_table_hb = ICM_TAB[11][si][h].H;
			dm_icm_3d_tab_write_hsi_b0_reg.write_table_sb = ICM_TAB[11][si][h].S;
			dm_icm_3d_tab_write_hsi_b1_reg.write_table_ib = ICM_TAB[11][si][h].I;

			/*write*/
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_00_reg, dm_icm_3d_tab_write_hsi_00_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_01_reg, dm_icm_3d_tab_write_hsi_01_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_10_reg, dm_icm_3d_tab_write_hsi_10_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_11_reg, dm_icm_3d_tab_write_hsi_11_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_20_reg, dm_icm_3d_tab_write_hsi_20_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_21_reg, dm_icm_3d_tab_write_hsi_21_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_30_reg, dm_icm_3d_tab_write_hsi_30_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_31_reg, dm_icm_3d_tab_write_hsi_31_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_40_reg, dm_icm_3d_tab_write_hsi_40_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_41_reg, dm_icm_3d_tab_write_hsi_41_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_50_reg, dm_icm_3d_tab_write_hsi_50_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_51_reg, dm_icm_3d_tab_write_hsi_51_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_60_reg, dm_icm_3d_tab_write_hsi_60_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_61_reg, dm_icm_3d_tab_write_hsi_61_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_70_reg, dm_icm_3d_tab_write_hsi_70_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_71_reg, dm_icm_3d_tab_write_hsi_71_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_80_reg, dm_icm_3d_tab_write_hsi_80_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_81_reg, dm_icm_3d_tab_write_hsi_81_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_90_reg, dm_icm_3d_tab_write_hsi_90_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_91_reg, dm_icm_3d_tab_write_hsi_91_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_A0_reg, dm_icm_3d_tab_write_hsi_a0_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_A1_reg, dm_icm_3d_tab_write_hsi_a1_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_B0_reg, dm_icm_3d_tab_write_hsi_b0_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_B1_reg, dm_icm_3d_tab_write_hsi_b1_reg.regValue);

			dm_icm_3d_tab_sram_ctrl_reg.write_en = 1;
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
			timeout = 20;
			do {
				dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
			} while (dm_icm_3d_tab_sram_ctrl_reg.write_en && (timeout-- > 0));

			if (timeout <= 0) {
				VIPprintf("drvif_color_icm_global_adjust write icm table error: sram_addrres_h = %d, sram_addrres_s = %d\n", h, si);
				return false;
			}
		}
	}
	/* 3. update current global hue/sat/itn*/
	/*g_dhue*/
	g_dhue = g_dhue_cur;
	/*g_dsatbysat*/
	g_dsatbysat[0]	= g_dsatbysat_cur[0];
	g_dsatbysat[1]	= g_dsatbysat_cur[1];
	g_dsatbysat[2]	= g_dsatbysat_cur[2];
	g_dsatbysat[3]	= g_dsatbysat_cur[3];
	g_dsatbysat[4]	= g_dsatbysat_cur[4];
	g_dsatbysat[5]	= g_dsatbysat_cur[5];
	g_dsatbysat[6]	= g_dsatbysat_cur[6];
	g_dsatbysat[7]	= g_dsatbysat_cur[7];
	g_dsatbysat[8]	= g_dsatbysat_cur[8];
	g_dsatbysat[9]	= g_dsatbysat_cur[9];
	g_dsatbysat[10]	= g_dsatbysat_cur[10];
	g_dsatbysat[11]	= g_dsatbysat_cur[11];
	/*g_ditnbyitn*/
	g_ditnbyitn[0]	= g_ditnbyitn_cur[0];
	g_ditnbyitn[1]	= g_ditnbyitn_cur[1];
	g_ditnbyitn[2]	= g_ditnbyitn_cur[2];
	g_ditnbyitn[3]	= g_ditnbyitn_cur[3];
	g_ditnbyitn[4]	= g_ditnbyitn_cur[4];
	g_ditnbyitn[5]	= g_ditnbyitn_cur[5];
	g_ditnbyitn[6]	= g_ditnbyitn_cur[6];
	g_ditnbyitn[7]	= g_ditnbyitn_cur[7];
	g_ditnbyitn[8]	= g_ditnbyitn_cur[8];
	g_ditnbyitn[9]	= g_ditnbyitn_cur[9];
	g_ditnbyitn[10]	= g_ditnbyitn_cur[10];
	g_ditnbyitn[11]	= g_ditnbyitn_cur[11];

	return true;
}


/*============================================================================*/
unsigned char drvif_color_icm_Get_Global_Ctrl(unsigned char bFromVIPTable, int *g_dhue_cur, int *g_dsatbysat_cur, int *g_ditnbyitn_cur)
{
	unsigned char nIndex = 0;
	if (bFromVIPTable) {	/* fix value */
		*g_dhue_cur = g_dhue_tab;
		for (nIndex = 0; nIndex < SATSEGMAX; nIndex++) {
			(g_dsatbysat_cur)[nIndex] = g_dsatbysat_tab[nIndex];
		}
		for (nIndex = 0; nIndex < ITNSEGMAX; nIndex++) {
			(g_ditnbyitn_cur)[nIndex] = g_ditnbyitn_tab[nIndex];
		}
	} else {	/* Change value */

		*g_dhue_cur = g_dhue;
		for (nIndex = 0; nIndex < SATSEGMAX; nIndex++) {
			(g_dsatbysat_cur)[nIndex] = g_dsatbysat[nIndex];
		}
		for (nIndex = 0; nIndex < ITNSEGMAX; nIndex++) {
			(g_ditnbyitn_cur)[nIndex] = g_ditnbyitn[nIndex];
		}
	}

	return true;
}

void drvif_color_Icm_Enable(unsigned char bEable)
{
	//color_d_vc_global_ctrl_RBUS			d_vc_global_ctrl_REG;
	color_d_vc_global_ctrl_0_RBUS		d_vc_global_ctrl_0_REG;
	//d_vc_global_ctrl_REG.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_0_REG.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_0_reg);
	d_vc_global_ctrl_0_REG.ms_ctrl_icm = bEable;
	d_vc_global_ctrl_0_REG.write_enable_3 =1;

	/*
	if (bEable && !d_vc_global_ctrl_REG.ms_vc_en){
		d_vc_global_ctrl_REG.ms_vc_en = 1;
		d_vc_global_ctrl_REG.write_enable_11 =1;

	}

	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, 	d_vc_global_ctrl_REG.regValue);
	*/
	IoReg_Write32(COLOR_D_VC_Global_CTRL_0_reg, 	d_vc_global_ctrl_0_REG.regValue);
}

unsigned char drvif_color_Icm_Read3DTable(COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX])
{
	int k, timeout;
	int h, si;
	int h_sta, h_end, si_sta, si_end;

	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS        dm_icm_3d_tab_sram_ctrl_reg;

	color_icm_dm_icm_3d_tab_read_hsi_00_RBUS      dm_icm_3d_tab_read_hsi_00_reg;
	color_icm_dm_icm_3d_tab_read_hsi_01_RBUS      dm_icm_3d_tab_read_hsi_01_reg;
	color_icm_dm_icm_3d_tab_read_hsi_10_RBUS      dm_icm_3d_tab_read_hsi_10_reg;
	color_icm_dm_icm_3d_tab_read_hsi_11_RBUS      dm_icm_3d_tab_read_hsi_11_reg;
	color_icm_dm_icm_3d_tab_read_hsi_20_RBUS      dm_icm_3d_tab_read_hsi_20_reg;
	color_icm_dm_icm_3d_tab_read_hsi_21_RBUS      dm_icm_3d_tab_read_hsi_21_reg;
	color_icm_dm_icm_3d_tab_read_hsi_30_RBUS      dm_icm_3d_tab_read_hsi_30_reg;
	color_icm_dm_icm_3d_tab_read_hsi_31_RBUS      dm_icm_3d_tab_read_hsi_31_reg;
	color_icm_dm_icm_3d_tab_read_hsi_40_RBUS      dm_icm_3d_tab_read_hsi_40_reg;
	color_icm_dm_icm_3d_tab_read_hsi_41_RBUS      dm_icm_3d_tab_read_hsi_41_reg;
	color_icm_dm_icm_3d_tab_read_hsi_50_RBUS      dm_icm_3d_tab_read_hsi_50_reg;
	color_icm_dm_icm_3d_tab_read_hsi_51_RBUS      dm_icm_3d_tab_read_hsi_51_reg;

	h_sta = 0;
	h_end = HUESEGMAX - 1;
	si_sta = 0;
	si_end = SATSEGMAX - 1;

	/* 1.read back*/
	dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
	dm_icm_3d_tab_sram_ctrl_reg.sram_type = 0; /* base on H, S axis => Read/Write to I axis*/
	dm_icm_3d_tab_sram_ctrl_reg.write_random_access_en = 0; /*clear*/
	dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt = 0;
	for (si = si_sta; si <= si_end; si++) {
		for (h = h_sta; h <= h_end; h++) {
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_h = h;
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_s = si;
			for (k = 0; k < 2; k++) {
				dm_icm_3d_tab_sram_ctrl_reg.read_sel = k;
				dm_icm_3d_tab_sram_ctrl_reg.read_en = 1;
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
				timeout = 20;
				do {
					dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
				} while (dm_icm_3d_tab_sram_ctrl_reg.read_en && (timeout-- > 0));
				if (timeout <= 0) {
					VIPprintf("drvif_color_icm_global_adjust read icm table error: sram_addrres_h = %d, sram_addrres_s = %d\n", h, si);
					return false;
				}

				dm_icm_3d_tab_read_hsi_00_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_00_reg);
				dm_icm_3d_tab_read_hsi_01_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_01_reg);
				dm_icm_3d_tab_read_hsi_10_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_10_reg);
				dm_icm_3d_tab_read_hsi_11_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_11_reg);
				dm_icm_3d_tab_read_hsi_20_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_20_reg);
				dm_icm_3d_tab_read_hsi_21_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_21_reg);
				dm_icm_3d_tab_read_hsi_30_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_30_reg);
				dm_icm_3d_tab_read_hsi_31_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_31_reg);
				dm_icm_3d_tab_read_hsi_40_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_40_reg);
				dm_icm_3d_tab_read_hsi_41_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_41_reg);
				dm_icm_3d_tab_read_hsi_50_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_50_reg);
				dm_icm_3d_tab_read_hsi_51_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_51_reg);

				ICM_TAB_ACCESS[k * 6 + 0][si][h].H = dm_icm_3d_tab_read_hsi_00_reg.read_table_h0;
				ICM_TAB_ACCESS[k * 6 + 0][si][h].S = dm_icm_3d_tab_read_hsi_00_reg.read_table_s0;
				ICM_TAB_ACCESS[k * 6 + 0][si][h].I = dm_icm_3d_tab_read_hsi_01_reg.read_table_i0;
				ICM_TAB_ACCESS[k * 6 + 1][si][h].H = dm_icm_3d_tab_read_hsi_10_reg.read_table_h1;
				ICM_TAB_ACCESS[k * 6 + 1][si][h].S = dm_icm_3d_tab_read_hsi_10_reg.read_table_s1;
				ICM_TAB_ACCESS[k * 6 + 1][si][h].I = dm_icm_3d_tab_read_hsi_11_reg.read_table_i1;
				ICM_TAB_ACCESS[k * 6 + 2][si][h].H = dm_icm_3d_tab_read_hsi_20_reg.read_table_h2;
				ICM_TAB_ACCESS[k * 6 + 2][si][h].S = dm_icm_3d_tab_read_hsi_20_reg.read_table_s2;
				ICM_TAB_ACCESS[k * 6 + 2][si][h].I = dm_icm_3d_tab_read_hsi_21_reg.read_table_i2;
				ICM_TAB_ACCESS[k * 6 + 3][si][h].H = dm_icm_3d_tab_read_hsi_30_reg.read_table_h3;
				ICM_TAB_ACCESS[k * 6 + 3][si][h].S = dm_icm_3d_tab_read_hsi_30_reg.read_table_s3;
				ICM_TAB_ACCESS[k * 6 + 3][si][h].I = dm_icm_3d_tab_read_hsi_31_reg.read_table_i3;
				ICM_TAB_ACCESS[k * 6 + 4][si][h].H = dm_icm_3d_tab_read_hsi_40_reg.read_table_h4;
				ICM_TAB_ACCESS[k * 6 + 4][si][h].S = dm_icm_3d_tab_read_hsi_40_reg.read_table_s4;
				ICM_TAB_ACCESS[k * 6 + 4][si][h].I = dm_icm_3d_tab_read_hsi_41_reg.read_table_i4;
				ICM_TAB_ACCESS[k * 6 + 5][si][h].H = dm_icm_3d_tab_read_hsi_50_reg.read_table_h5;
				ICM_TAB_ACCESS[k * 6 + 5][si][h].S = dm_icm_3d_tab_read_hsi_50_reg.read_table_s5;
				ICM_TAB_ACCESS[k * 6 + 5][si][h].I = dm_icm_3d_tab_read_hsi_51_reg.read_table_i5;


			}
		}
	}
	return true;
}

unsigned char drvif_color_icm_Write3DTable(COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX])
{
	int timeout;
	int h, si;
	int h_sta, h_end, si_sta, si_end;

	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS        dm_icm_3d_tab_sram_ctrl_reg;
	color_icm_dm_icm_3d_tab_write_hsi_00_RBUS     dm_icm_3d_tab_write_hsi_00_reg;
	color_icm_dm_icm_3d_tab_write_hsi_01_RBUS     dm_icm_3d_tab_write_hsi_01_reg;
	color_icm_dm_icm_3d_tab_write_hsi_10_RBUS     dm_icm_3d_tab_write_hsi_10_reg;
	color_icm_dm_icm_3d_tab_write_hsi_11_RBUS     dm_icm_3d_tab_write_hsi_11_reg;
	color_icm_dm_icm_3d_tab_write_hsi_20_RBUS     dm_icm_3d_tab_write_hsi_20_reg;
	color_icm_dm_icm_3d_tab_write_hsi_21_RBUS     dm_icm_3d_tab_write_hsi_21_reg;
	color_icm_dm_icm_3d_tab_write_hsi_30_RBUS     dm_icm_3d_tab_write_hsi_30_reg;
	color_icm_dm_icm_3d_tab_write_hsi_31_RBUS     dm_icm_3d_tab_write_hsi_31_reg;
	color_icm_dm_icm_3d_tab_write_hsi_40_RBUS     dm_icm_3d_tab_write_hsi_40_reg;
	color_icm_dm_icm_3d_tab_write_hsi_41_RBUS     dm_icm_3d_tab_write_hsi_41_reg;
	color_icm_dm_icm_3d_tab_write_hsi_50_RBUS     dm_icm_3d_tab_write_hsi_50_reg;
	color_icm_dm_icm_3d_tab_write_hsi_51_RBUS     dm_icm_3d_tab_write_hsi_51_reg;
	color_icm_dm_icm_3d_tab_write_hsi_60_RBUS     dm_icm_3d_tab_write_hsi_60_reg;
	color_icm_dm_icm_3d_tab_write_hsi_61_RBUS     dm_icm_3d_tab_write_hsi_61_reg;
	color_icm_dm_icm_3d_tab_write_hsi_70_RBUS     dm_icm_3d_tab_write_hsi_70_reg;
	color_icm_dm_icm_3d_tab_write_hsi_71_RBUS     dm_icm_3d_tab_write_hsi_71_reg;
	color_icm_dm_icm_3d_tab_write_hsi_80_RBUS     dm_icm_3d_tab_write_hsi_80_reg;
	color_icm_dm_icm_3d_tab_write_hsi_81_RBUS     dm_icm_3d_tab_write_hsi_81_reg;
	color_icm_dm_icm_3d_tab_write_hsi_90_RBUS     dm_icm_3d_tab_write_hsi_90_reg;
	color_icm_dm_icm_3d_tab_write_hsi_91_RBUS     dm_icm_3d_tab_write_hsi_91_reg;
	color_icm_dm_icm_3d_tab_write_hsi_a0_RBUS     dm_icm_3d_tab_write_hsi_a0_reg;
	color_icm_dm_icm_3d_tab_write_hsi_a1_RBUS     dm_icm_3d_tab_write_hsi_a1_reg;
	color_icm_dm_icm_3d_tab_write_hsi_b0_RBUS     dm_icm_3d_tab_write_hsi_b0_reg;
	color_icm_dm_icm_3d_tab_write_hsi_b1_RBUS     dm_icm_3d_tab_write_hsi_b1_reg;

	//constrain: if h segment num is odd, e.x. 2n+1(h pillar num=2n+2), need to copy h_idx=2n layer to h_idx=2n+1 layer, for hw need even h layer sram table  
	int h_pillar_num, s_pillar_num, i_pillar_num, ii, is;
	color_icm_dm_icm_hue_segment_11_RBUS	  dm_icm_hue_segment_11_reg;
	color_icm_dm_icm_sat_segment_0_RBUS	dm_icm_sat_segment_0_reg;
	color_icm_dm_icm_int_segment_0_RBUS dm_icm_int_segment_0_reg;
	dm_icm_hue_segment_11_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_11_reg);
	dm_icm_sat_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg);
	dm_icm_int_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg);
	
	h_pillar_num=dm_icm_hue_segment_11_reg.h_pillar_48+1;
	s_pillar_num=dm_icm_sat_segment_0_reg.s_pillar_8+1;
	i_pillar_num=dm_icm_int_segment_0_reg.i_pillar_8+1;

	if(h_pillar_num % 2 == 0)
	{
		for(is = 0; is < s_pillar_num; is++)
		{
			for(ii = 0; ii < i_pillar_num; ii++)
			{
				ICM_TAB_ACCESS[ii][is][h_pillar_num - 1].H = ICM_TAB_ACCESS[ii][is][h_pillar_num - 2].H;
				ICM_TAB_ACCESS[ii][is][h_pillar_num - 1].S = ICM_TAB_ACCESS[ii][is][h_pillar_num - 2].S;
				ICM_TAB_ACCESS[ii][is][h_pillar_num - 1].I = ICM_TAB_ACCESS[ii][is][h_pillar_num - 2].I;
			}
		}
	}


	h_sta = 0;
	h_end = HUESEGMAX - 1;
	si_sta = 0;
	si_end = SATSEGMAX - 1;

	dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
	dm_icm_3d_tab_sram_ctrl_reg.sram_type = 0; /* base on H, S axis => Read/Write to I axis*/
	dm_icm_3d_tab_sram_ctrl_reg.write_random_access_en = 0; /*block write*/
	dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt = 0;
	IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
	for (si = si_sta; si <= si_end; si++) {
		for (h = h_sta; h <= h_end; h++) {
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_h = h;
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_s = si;
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_i = 0;

			dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = ICM_TAB_ACCESS[0][si][h].H;
			dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = ICM_TAB_ACCESS[0][si][h].S;
			dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = ICM_TAB_ACCESS[0][si][h].I;
			dm_icm_3d_tab_write_hsi_10_reg.write_table_h1 = ICM_TAB_ACCESS[1][si][h].H;
			dm_icm_3d_tab_write_hsi_10_reg.write_table_s1 = ICM_TAB_ACCESS[1][si][h].S;
			dm_icm_3d_tab_write_hsi_11_reg.write_table_i1 = ICM_TAB_ACCESS[1][si][h].I;
			dm_icm_3d_tab_write_hsi_20_reg.write_table_h2 = ICM_TAB_ACCESS[2][si][h].H;
			dm_icm_3d_tab_write_hsi_20_reg.write_table_s2 = ICM_TAB_ACCESS[2][si][h].S;
			dm_icm_3d_tab_write_hsi_21_reg.write_table_i2 = ICM_TAB_ACCESS[2][si][h].I;
			dm_icm_3d_tab_write_hsi_30_reg.write_table_h3 = ICM_TAB_ACCESS[3][si][h].H;
			dm_icm_3d_tab_write_hsi_30_reg.write_table_s3 = ICM_TAB_ACCESS[3][si][h].S;
			dm_icm_3d_tab_write_hsi_31_reg.write_table_i3 = ICM_TAB_ACCESS[3][si][h].I;
			dm_icm_3d_tab_write_hsi_40_reg.write_table_h4 = ICM_TAB_ACCESS[4][si][h].H;
			dm_icm_3d_tab_write_hsi_40_reg.write_table_s4 = ICM_TAB_ACCESS[4][si][h].S;
			dm_icm_3d_tab_write_hsi_41_reg.write_table_i4 = ICM_TAB_ACCESS[4][si][h].I;
			dm_icm_3d_tab_write_hsi_50_reg.write_table_h5 = ICM_TAB_ACCESS[5][si][h].H;
			dm_icm_3d_tab_write_hsi_50_reg.write_table_s5 = ICM_TAB_ACCESS[5][si][h].S;
			dm_icm_3d_tab_write_hsi_51_reg.write_table_i5 = ICM_TAB_ACCESS[5][si][h].I;
			dm_icm_3d_tab_write_hsi_60_reg.write_table_h6 = ICM_TAB_ACCESS[6][si][h].H;
			dm_icm_3d_tab_write_hsi_60_reg.write_table_s6 = ICM_TAB_ACCESS[6][si][h].S;
			dm_icm_3d_tab_write_hsi_61_reg.write_table_i6 = ICM_TAB_ACCESS[6][si][h].I;
			dm_icm_3d_tab_write_hsi_70_reg.write_table_h7 = ICM_TAB_ACCESS[7][si][h].H;
			dm_icm_3d_tab_write_hsi_70_reg.write_table_s7 = ICM_TAB_ACCESS[7][si][h].S;
			dm_icm_3d_tab_write_hsi_71_reg.write_table_i7 = ICM_TAB_ACCESS[7][si][h].I;
			dm_icm_3d_tab_write_hsi_80_reg.write_table_h8 = ICM_TAB_ACCESS[8][si][h].H;
			dm_icm_3d_tab_write_hsi_80_reg.write_table_s8 = ICM_TAB_ACCESS[8][si][h].S;
			dm_icm_3d_tab_write_hsi_81_reg.write_table_i8 = ICM_TAB_ACCESS[8][si][h].I;
			dm_icm_3d_tab_write_hsi_90_reg.write_table_h9 = ICM_TAB_ACCESS[9][si][h].H;
			dm_icm_3d_tab_write_hsi_90_reg.write_table_s9 = ICM_TAB_ACCESS[9][si][h].S;
			dm_icm_3d_tab_write_hsi_91_reg.write_table_i9 = ICM_TAB_ACCESS[9][si][h].I;
			dm_icm_3d_tab_write_hsi_a0_reg.write_table_ha = ICM_TAB_ACCESS[10][si][h].H;
			dm_icm_3d_tab_write_hsi_a0_reg.write_table_sa = ICM_TAB_ACCESS[10][si][h].S;
			dm_icm_3d_tab_write_hsi_a1_reg.write_table_ia = ICM_TAB_ACCESS[10][si][h].I;
			dm_icm_3d_tab_write_hsi_b0_reg.write_table_hb = ICM_TAB_ACCESS[11][si][h].H;
			dm_icm_3d_tab_write_hsi_b0_reg.write_table_sb = ICM_TAB_ACCESS[11][si][h].S;
			dm_icm_3d_tab_write_hsi_b1_reg.write_table_ib = ICM_TAB_ACCESS[11][si][h].I;


			/*write*/
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_00_reg, dm_icm_3d_tab_write_hsi_00_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_01_reg, dm_icm_3d_tab_write_hsi_01_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_10_reg, dm_icm_3d_tab_write_hsi_10_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_11_reg, dm_icm_3d_tab_write_hsi_11_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_20_reg, dm_icm_3d_tab_write_hsi_20_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_21_reg, dm_icm_3d_tab_write_hsi_21_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_30_reg, dm_icm_3d_tab_write_hsi_30_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_31_reg, dm_icm_3d_tab_write_hsi_31_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_40_reg, dm_icm_3d_tab_write_hsi_40_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_41_reg, dm_icm_3d_tab_write_hsi_41_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_50_reg, dm_icm_3d_tab_write_hsi_50_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_51_reg, dm_icm_3d_tab_write_hsi_51_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_60_reg, dm_icm_3d_tab_write_hsi_60_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_61_reg, dm_icm_3d_tab_write_hsi_61_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_70_reg, dm_icm_3d_tab_write_hsi_70_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_71_reg, dm_icm_3d_tab_write_hsi_71_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_80_reg, dm_icm_3d_tab_write_hsi_80_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_81_reg, dm_icm_3d_tab_write_hsi_81_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_90_reg, dm_icm_3d_tab_write_hsi_90_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_91_reg, dm_icm_3d_tab_write_hsi_91_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_A0_reg, dm_icm_3d_tab_write_hsi_a0_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_A1_reg, dm_icm_3d_tab_write_hsi_a1_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_B0_reg, dm_icm_3d_tab_write_hsi_b0_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_B1_reg, dm_icm_3d_tab_write_hsi_b1_reg.regValue);

			dm_icm_3d_tab_sram_ctrl_reg.write_en = 1;
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
			timeout = 20;
			do {
				dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
			} while (dm_icm_3d_tab_sram_ctrl_reg.write_en && (timeout-- > 0));

			if (timeout <= 0) {
				VIPprintf("drvif_color_icm_global_adjust write icm table error: sram_addrres_h = %d, sram_addrres_s = %d\n", h, si);
				return false;
			}
		}
	}

	return true;
}

// Random Table for debug
unsigned char drvif_color_icm_WriteRandomTable(void)
{
	int timeout;
	int h, si;
	int h_sta, h_end, si_sta, si_end;

	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS        dm_icm_3d_tab_sram_ctrl_reg;
	color_icm_dm_icm_3d_tab_write_hsi_00_RBUS     dm_icm_3d_tab_write_hsi_00_reg;
	color_icm_dm_icm_3d_tab_write_hsi_01_RBUS     dm_icm_3d_tab_write_hsi_01_reg;
	color_icm_dm_icm_3d_tab_write_hsi_10_RBUS     dm_icm_3d_tab_write_hsi_10_reg;
	color_icm_dm_icm_3d_tab_write_hsi_11_RBUS     dm_icm_3d_tab_write_hsi_11_reg;
	color_icm_dm_icm_3d_tab_write_hsi_20_RBUS     dm_icm_3d_tab_write_hsi_20_reg;
	color_icm_dm_icm_3d_tab_write_hsi_21_RBUS     dm_icm_3d_tab_write_hsi_21_reg;
	color_icm_dm_icm_3d_tab_write_hsi_30_RBUS     dm_icm_3d_tab_write_hsi_30_reg;
	color_icm_dm_icm_3d_tab_write_hsi_31_RBUS     dm_icm_3d_tab_write_hsi_31_reg;
	color_icm_dm_icm_3d_tab_write_hsi_40_RBUS     dm_icm_3d_tab_write_hsi_40_reg;
	color_icm_dm_icm_3d_tab_write_hsi_41_RBUS     dm_icm_3d_tab_write_hsi_41_reg;
	color_icm_dm_icm_3d_tab_write_hsi_50_RBUS     dm_icm_3d_tab_write_hsi_50_reg;
	color_icm_dm_icm_3d_tab_write_hsi_51_RBUS     dm_icm_3d_tab_write_hsi_51_reg;
	color_icm_dm_icm_3d_tab_write_hsi_60_RBUS     dm_icm_3d_tab_write_hsi_60_reg;
	color_icm_dm_icm_3d_tab_write_hsi_61_RBUS     dm_icm_3d_tab_write_hsi_61_reg;
	color_icm_dm_icm_3d_tab_write_hsi_70_RBUS     dm_icm_3d_tab_write_hsi_70_reg;
	color_icm_dm_icm_3d_tab_write_hsi_71_RBUS     dm_icm_3d_tab_write_hsi_71_reg;
	color_icm_dm_icm_3d_tab_write_hsi_80_RBUS     dm_icm_3d_tab_write_hsi_80_reg;
	color_icm_dm_icm_3d_tab_write_hsi_81_RBUS     dm_icm_3d_tab_write_hsi_81_reg;
	color_icm_dm_icm_3d_tab_write_hsi_90_RBUS     dm_icm_3d_tab_write_hsi_90_reg;
	color_icm_dm_icm_3d_tab_write_hsi_91_RBUS     dm_icm_3d_tab_write_hsi_91_reg;
	color_icm_dm_icm_3d_tab_write_hsi_a0_RBUS     dm_icm_3d_tab_write_hsi_a0_reg;
	color_icm_dm_icm_3d_tab_write_hsi_a1_RBUS     dm_icm_3d_tab_write_hsi_a1_reg;
	color_icm_dm_icm_3d_tab_write_hsi_b0_RBUS     dm_icm_3d_tab_write_hsi_b0_reg;
	color_icm_dm_icm_3d_tab_write_hsi_b1_RBUS     dm_icm_3d_tab_write_hsi_b1_reg;

	h_sta = 0;
	h_end = HUESEGMAX - 1;
	si_sta = 0;
	si_end = SATSEGMAX - 1;

	dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
	dm_icm_3d_tab_sram_ctrl_reg.sram_type = 0; /* base on H, S axis => Read/Write to I axis*/
	dm_icm_3d_tab_sram_ctrl_reg.write_random_access_en = 0; /*block write*/
	dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt = 0;
	IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
	for (si = si_sta; si <= si_end; si++) {
		for (h = h_sta; h <= h_end; h++) {
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_h = h;
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_s = si;
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_i = 0;

			dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = rand32();
			dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = rand32();
			dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = rand32();
			dm_icm_3d_tab_write_hsi_10_reg.write_table_h1 = rand32();
			dm_icm_3d_tab_write_hsi_10_reg.write_table_s1 = rand32();
			dm_icm_3d_tab_write_hsi_11_reg.write_table_i1 = rand32();
			dm_icm_3d_tab_write_hsi_20_reg.write_table_h2 = rand32();
			dm_icm_3d_tab_write_hsi_20_reg.write_table_s2 = rand32();
			dm_icm_3d_tab_write_hsi_21_reg.write_table_i2 = rand32();
			dm_icm_3d_tab_write_hsi_30_reg.write_table_h3 = rand32();
			dm_icm_3d_tab_write_hsi_30_reg.write_table_s3 = rand32();
			dm_icm_3d_tab_write_hsi_31_reg.write_table_i3 = rand32();
			dm_icm_3d_tab_write_hsi_40_reg.write_table_h4 = rand32();
			dm_icm_3d_tab_write_hsi_40_reg.write_table_s4 = rand32();
			dm_icm_3d_tab_write_hsi_41_reg.write_table_i4 = rand32();
			dm_icm_3d_tab_write_hsi_50_reg.write_table_h5 = rand32();
			dm_icm_3d_tab_write_hsi_50_reg.write_table_s5 = rand32();
			dm_icm_3d_tab_write_hsi_51_reg.write_table_i5 = rand32();
			dm_icm_3d_tab_write_hsi_60_reg.write_table_h6 = rand32();
			dm_icm_3d_tab_write_hsi_60_reg.write_table_s6 = rand32();
			dm_icm_3d_tab_write_hsi_61_reg.write_table_i6 = rand32();
			dm_icm_3d_tab_write_hsi_70_reg.write_table_h7 = rand32();
			dm_icm_3d_tab_write_hsi_70_reg.write_table_s7 = rand32();
			dm_icm_3d_tab_write_hsi_71_reg.write_table_i7 = rand32();
			dm_icm_3d_tab_write_hsi_80_reg.write_table_h8 = rand32();
			dm_icm_3d_tab_write_hsi_80_reg.write_table_s8 = rand32();
			dm_icm_3d_tab_write_hsi_81_reg.write_table_i8 = rand32();
			dm_icm_3d_tab_write_hsi_90_reg.write_table_h9 = rand32();
			dm_icm_3d_tab_write_hsi_90_reg.write_table_s9 = rand32();
			dm_icm_3d_tab_write_hsi_91_reg.write_table_i9 = rand32();
			dm_icm_3d_tab_write_hsi_a0_reg.write_table_ha = rand32();
			dm_icm_3d_tab_write_hsi_a0_reg.write_table_sa = rand32();
			dm_icm_3d_tab_write_hsi_a1_reg.write_table_ia = rand32();
			dm_icm_3d_tab_write_hsi_b0_reg.write_table_hb = rand32();
			dm_icm_3d_tab_write_hsi_b0_reg.write_table_sb = rand32();
			dm_icm_3d_tab_write_hsi_b1_reg.write_table_ib = rand32();

			/*write*/
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_00_reg, dm_icm_3d_tab_write_hsi_00_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_01_reg, dm_icm_3d_tab_write_hsi_01_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_10_reg, dm_icm_3d_tab_write_hsi_10_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_11_reg, dm_icm_3d_tab_write_hsi_11_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_20_reg, dm_icm_3d_tab_write_hsi_20_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_21_reg, dm_icm_3d_tab_write_hsi_21_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_30_reg, dm_icm_3d_tab_write_hsi_30_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_31_reg, dm_icm_3d_tab_write_hsi_31_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_40_reg, dm_icm_3d_tab_write_hsi_40_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_41_reg, dm_icm_3d_tab_write_hsi_41_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_50_reg, dm_icm_3d_tab_write_hsi_50_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_51_reg, dm_icm_3d_tab_write_hsi_51_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_60_reg, dm_icm_3d_tab_write_hsi_60_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_61_reg, dm_icm_3d_tab_write_hsi_61_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_70_reg, dm_icm_3d_tab_write_hsi_70_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_71_reg, dm_icm_3d_tab_write_hsi_71_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_80_reg, dm_icm_3d_tab_write_hsi_80_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_81_reg, dm_icm_3d_tab_write_hsi_81_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_90_reg, dm_icm_3d_tab_write_hsi_90_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_91_reg, dm_icm_3d_tab_write_hsi_91_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_A0_reg, dm_icm_3d_tab_write_hsi_a0_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_A1_reg, dm_icm_3d_tab_write_hsi_a1_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_B0_reg, dm_icm_3d_tab_write_hsi_b0_reg.regValue);
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_B1_reg, dm_icm_3d_tab_write_hsi_b1_reg.regValue);

			dm_icm_3d_tab_sram_ctrl_reg.write_en = 1;
			IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
			timeout = 20;
			do {
				dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
			} while (dm_icm_3d_tab_sram_ctrl_reg.write_en && (timeout-- > 0));

			if (timeout <= 0) {
				VIPprintf("drvif_color_icm_global_adjust write icm table error: sram_addrres_h = %d, sram_addrres_s = %d\n", h, si);
				return false;
			}
		}
	}

	return true;
}

UINT32 GetMask(UINT32 bit)
{
    UINT32 i, mask;
    mask = 0;
    for(i = 0; i < bit; i++)
    {
        mask = mask | (0x1 << i);
    }
    return mask;
}

unsigned char drvif_color_icm_Write3DTable_By_DMA(COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX], unsigned int addr, unsigned int* vir_addr, unsigned char pillar_num_info_type)
{
#ifndef BUILD_QUICK_SHOW

	//color_d_vc_global_ctrl_RBUS	d_vc_global_ctrl_REG;

	//color_icm_dm_icm_ctrl_RBUS color_icm_dm_icm_ctrl_reg;
	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS color_icm_dm_icm_3d_tab_sram_ctrl_reg;

	dmato3dlut_dmato3dlut_table0_format0_RBUS dmato3dlut_dmato3dlut_table0_format0_reg;
	//dmato3dlut_dmato3dlut_table0_addr_RBUS dmato3dlut_dmato3dlut_table0_addr_reg;
	dmato3dlut_dmato3dlut_table0_status_RBUS dmato3dlut_dmato3dlut_table0_status_reg;
	dmato3dlut_dmato3dlut_db_ctl_RBUS dmato3dlut_dmato3dlut_db_ctl_reg;

	UINT32 hue_bit = 11;
	UINT32 sat_bit = 13;
	UINT32 itn_bit = 13;
	UINT32 buf_bit = 32;
	UINT32 total_bit;
	UINT32 *buf;
	UINT32 buf_size;
	UINT32 buf_left_bit;
	UINT32 data;
	UINT32 ih, is, ii, mh, ms, mi, nh, ns, ni, idx;

	//UINT32 address=0;
	UINT32 i=0;

	//constrain: if h segment num is odd, e.x. 2n+1(h pillar num=2n+2), need to copy h_idx=2n layer to h_idx=2n+1 layer, for hw need even h layer sram table  
	int h_pillar_num, s_pillar_num, i_pillar_num;
	extern unsigned short *g_tICM_ini_s_pillar;
	extern unsigned short *g_tICM_ini_i_pillar;
	extern unsigned short *g_tICM_ini_h_pillar;
	
	if (pillar_num_info_type == PILLAR_NUM_INFO_TYPE_FROM_REG_GET || g_tICM_ini_s_pillar == NULL || g_tICM_ini_i_pillar == NULL || g_tICM_ini_h_pillar == NULL) {
		color_icm_dm_icm_hue_segment_11_RBUS	  dm_icm_hue_segment_11_reg;
		color_icm_dm_icm_sat_segment_0_RBUS	dm_icm_sat_segment_0_reg;
		color_icm_dm_icm_int_segment_0_RBUS dm_icm_int_segment_0_reg;
		dm_icm_hue_segment_11_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_11_reg);
		dm_icm_sat_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg);
		dm_icm_int_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg);
		
		h_pillar_num=dm_icm_hue_segment_11_reg.h_pillar_48+1;
		s_pillar_num=dm_icm_sat_segment_0_reg.s_pillar_8+1;
		i_pillar_num=dm_icm_int_segment_0_reg.i_pillar_8+1;
	} else {
		h_pillar_num = g_tICM_ini_h_pillar[47]+1;
		s_pillar_num = g_tICM_ini_s_pillar[7]+1;
		i_pillar_num = g_tICM_ini_i_pillar[7]+1;
	}

	if(h_pillar_num % 2 == 0)
	{
		for(is = 0; is < s_pillar_num; is++)
		{
			for(ii = 0; ii < i_pillar_num; ii++)
			{
				ICM_TAB_ACCESS[ii][is][h_pillar_num - 1].H = ICM_TAB_ACCESS[ii][is][h_pillar_num - 2].H;
				ICM_TAB_ACCESS[ii][is][h_pillar_num - 1].S = ICM_TAB_ACCESS[ii][is][h_pillar_num - 2].S;
				ICM_TAB_ACCESS[ii][is][h_pillar_num - 1].I = ICM_TAB_ACCESS[ii][is][h_pillar_num - 2].I;
			}
		}
	}

	if ((addr == 0) || (vir_addr == NULL)) {
		rtd_pr_vpq_emerg("drvif_color_icm_Write3DTable_By_DMA, table NULL, addr = %x, vi_addr = %p\n",
			 addr, vir_addr);
		return false;
	}

	// Calc DMA data format
	total_bit = (hue_bit + sat_bit + itn_bit) * (HUESEGMAX * SATSEGMAX * ITNSEGMAX);
	buf_size = (total_bit % buf_bit ) == 0 ? total_bit / buf_bit : total_bit / buf_bit + 1;

	buf = (unsigned int *)dvr_malloc_specific(buf_size * sizeof(unsigned int), GFP_DCU2_FIRST);
	if (buf == NULL) {
		rtd_pr_vpq_emerg("ICM: buf PQ warning, drvif_color_icm_Write3DTable_By_DMA memory allocate fail!\n");
		return false;
	}

	memset(buf, 0, buf_size * sizeof(unsigned int));

	buf_left_bit = 32;
	idx = 0;
	for(ni = 0; ni < ITNSEGMAX; ni += 2) {
	for(ns = 0; ns < SATSEGMAX; ns += 2) {
	for(nh = 0; nh < HUESEGMAX; nh += 2) {
		for(mi = 0; mi < 2; mi ++) {
		for(ms = 0; ms < 2; ms ++) {
		for(mh = 0; mh < 2; mh ++) {
			ii = ni + mi;
			is = ns + ms;
			ih = nh + mh;

			// hue
			data = ICM_TAB_ACCESS[ii][is][ih].H & GetMask(hue_bit);
			//data =0x789& GetMask(hue_bit);
			if(buf_left_bit >= hue_bit) {
				buf[idx] = buf[idx] | (data << (buf_left_bit - hue_bit));
				buf_left_bit -= hue_bit;
			} else {
				buf[idx] = buf[idx] | (data >> (hue_bit - buf_left_bit));
				idx += 1;
				buf_left_bit = 32 - (hue_bit - buf_left_bit);
				buf[idx] = buf[idx] | (data << buf_left_bit);
			}

			// sat
			data = ICM_TAB_ACCESS[ii][is][ih].S & GetMask(sat_bit);
			//data =0x1253& GetMask(sat_bit);
			if(buf_left_bit >= sat_bit) {
				buf[idx] = buf[idx] | (data << (buf_left_bit - sat_bit));
				buf_left_bit -= sat_bit;
			} else {
				buf[idx] = buf[idx] | (data >> (sat_bit - buf_left_bit));
				idx += 1;
				buf_left_bit = 32 - (sat_bit - buf_left_bit);
				buf[idx] = buf[idx] | (data << buf_left_bit);
			}

			// itn
			data = ICM_TAB_ACCESS[ii][is][ih].I & GetMask(itn_bit);
			//data = (ICM_TAB_ACCESS[ii][is][ih].I) & GetMask(itn_bit);

			//data =(0x1234) & GetMask(itn_bit);
			if(buf_left_bit >= itn_bit) {
				buf[idx] = buf[idx] | (data << (buf_left_bit - itn_bit));
				buf_left_bit -= itn_bit;
			} else {
				buf[idx] = buf[idx] | (data >> (itn_bit - buf_left_bit));
				idx += 1;
				buf_left_bit = 32 - (itn_bit - buf_left_bit);
				buf[idx] = buf[idx] | (data << buf_left_bit);
			}
		}
		}
		}
	}
	}
	}

	drvif_Wait_UZUDTG_start(10, -200);

	// set data to DMA
	for(i = 0; i < (buf_size); i++)
	{
		unsigned char *temp;
		temp=(unsigned char *)&buf[i];
		vir_addr[i]=temp[3]+(temp[2]<<8)+(temp[1]<<16)+(temp[0]<<24);
	}
	dvr_free((void *)buf);

	// Start ICM DMA flow
	IoReg_Write32(DMATO3DLUT_DMAto3DLUT_Table0_Addr_reg, addr);

	dmato3dlut_dmato3dlut_db_ctl_reg.regValue= IoReg_Read32(DMATO3DLUT_DMAto3DLUT_db_ctl_reg);
	dmato3dlut_dmato3dlut_db_ctl_reg.db_en =1;
	IoReg_Write32(DMATO3DLUT_DMAto3DLUT_db_ctl_reg,dmato3dlut_dmato3dlut_db_ctl_reg.regValue);

	color_icm_dm_icm_3d_tab_sram_ctrl_reg.regValue= IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
	color_icm_dm_icm_3d_tab_sram_ctrl_reg.dmato3dlut_en =1;
	color_icm_dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt=1;
	IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg,color_icm_dm_icm_3d_tab_sram_ctrl_reg.regValue);

	dmato3dlut_dmato3dlut_table0_format0_reg.regValue = IoReg_Read32(DMATO3DLUT_DMAto3DLUT_Table0_Format0_reg);
	dmato3dlut_dmato3dlut_table0_format0_reg.table0_dma_en =1;
	IoReg_Write32(DMATO3DLUT_DMAto3DLUT_Table0_Format0_reg,dmato3dlut_dmato3dlut_table0_format0_reg.regValue);

	dmato3dlut_dmato3dlut_table0_status_reg.regValue = IoReg_Read32(DMATO3DLUT_DMAto3DLUT_Table0_Status_reg);
	dmato3dlut_dmato3dlut_table0_status_reg.table0_wdone=1;
	IoReg_Write32(DMATO3DLUT_DMAto3DLUT_Table0_Status_reg,dmato3dlut_dmato3dlut_table0_status_reg.regValue);

	dmato3dlut_dmato3dlut_db_ctl_reg.regValue= IoReg_Read32(DMATO3DLUT_DMAto3DLUT_db_ctl_reg);
	dmato3dlut_dmato3dlut_db_ctl_reg.wtable_apply =1;
	IoReg_Write32(DMATO3DLUT_DMAto3DLUT_db_ctl_reg,dmato3dlut_dmato3dlut_db_ctl_reg.regValue);
#endif
	return true;
}

/**
 * Color_SetICMHSI
 * Set Hue/Satuation/Itensity for Global control of ICM
 *
 * @param <display> {main/sub display}
 * @param <Hue> { Hue Adjust 11-bit sign}
 * @param <Sat> { Sat Adjust 12-bit sign}
 * @param <Iten> { Iten Adjust 12-bit sign}
 * @return 			{ void }
 *
 */
void drvif_color_seticmhsi(unsigned char display, short Value, unsigned char which_item)
{
	unsigned char nIndex = 0;

	VIPprintf("Color_SetICMHSI:which_item=%d value=%d \n", which_item, Value);

	if (display == SLR_MAIN_DISPLAY) {

		switch (which_item) {
		case ICM_Global_hue:
			VIPprintf("icm_hue_lastvalue=%d\n", g_dhue_tab);
			/* for icm table and OSD data mixing*/
			/*if (((icm_hue_lastvalue >> 10) & 0x01) == 0x01)
				icm_hue_lastvalue |= 0xf800;*/
			g_dhue		= Value + g_dhue_tab;
			break;

		case ICM_Global_Sat:
			for (nIndex = 0; nIndex < SATSEGMAX; nIndex++)
				g_dsatbysat[nIndex] = g_dsatbysat_tab[nIndex]+Value;
			break;

		case ICM_Global_Itn:
			for (nIndex = 0; nIndex < ITNSEGMAX; nIndex++)
				g_ditnbyitn[nIndex] = g_ditnbyitn_tab[nIndex]+Value;
			break;
		}
		drvif_color_icm_global_adjust(g_dhue, g_dsatbysat, g_ditnbyitn, 0);

		/*VIPprintf("\n\n=====================%x,  %x\n\n ", Hue, icm_hue_lastvalue);*/

	}
#ifdef CONFIG_DUAL_CHANNEL
	else {/* sub*/
		/**/
	}
#endif
}

int drvif_color_icm_FormatValue(int value, unsigned char type, unsigned char tab_mode, unsigned char IfWrite)
{
	int result = 0;
	int itn_const;

	if(tab_mode == 0) // HSI
	{
	    itn_const = ITN_SUBTRACT_CONST;
	}
	else // HSV
	{
	    itn_const = 0;
	}

	if (IfWrite) {
		switch (type) {
		case _HUE_ELEM:
			result = (value + HUERANGE) % HUERANGE;
		break;
		case _SAT_ELEM:
			result = CLIP(0, SATRANGEEX - 1, value);
		break;
		case _ITN_ELEM:
			result = CLIP(0, ITNRANGEEX - 1, value + itn_const);
		break;
		}
	} else {
		switch (type) {
		case _HUE_ELEM:
		case _SAT_ELEM:
			result = value;
		break;
		case _ITN_ELEM:
			result = value - itn_const;
		break;
		}
	}
    return result;
}

/*============================================================================*/





/* KONKA user function 4in1*/
/*------------------------------------------------------------------------------
 * function:	drvif_color_icm_user_function
 * Parm:		display = [ SLR_MAIN_DISPLAY(0)  SLR_SUB_DISPLAY(1) ]
 *			which_table = [which ICM table] (vip_panel_xxx.cpp -> VIP_QUALITY_Coef -> item20)
 *			skin_tone = [0(off)  1(red)  2(yellow) 255(skip)] (default value is off)
 *			color_wheel = [0(off)  1(on) 255(skip)] (default value is on)
 *			detail_enhance =  [0(off)  1(on) 255(skip)]  (default value is off)
 *			dynamic_tint =  [0(off)  1(on) 255(skip)] (default value is on)
 * Return: 	void
 *-----------------------------------------------------------------------------*/
void drvif_color_icm_user_function(unsigned char display, unsigned short *tICM_ini, unsigned char which_table, unsigned char skin_tone, unsigned char color_wheel, unsigned char detail_enhance, unsigned dynamic_tint)
{
	/*fu:if default value don't set, it will get DDR value that compile with O2.*/
	/*it maybe get wrong value like dm_icm_ctrl_REG.regValue=268539380;*/

	unsigned char ICM_Num_Limite	= 255;/*30;*/ /*planck changed to 255 for the max of icm tables.20101111*/
	/*short base_value_1 = 0, base_value_2= 0, diff_value = 0;*/

	/* Global Setting*/
	/*unsigned int baseAddr			= which_table * VIP_ICM_TBL_Y * VIP_ICM_TBL_Z;*/

	VIPprintf("Color_ICM_Table_Select Initialize  which_table= %d\n", which_table);

	if (display == SLR_MAIN_DISPLAY) {
		color_d_vc_global_ctrl_RBUS			d_vc_global_ctrl_REG; /*icm enable*/
		color_icm_dm_icm_ctrl_RBUS                dm_icm_ctrl_reg;
		d_vc_global_ctrl_REG.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
		dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

		if (which_table >= ICM_Num_Limite) { /* table only 0~29 */
			drvif_color_Icm_Enable(0);
			return;
		} else {
			drvif_color_Icm_Enable(1);
		}
	}   /* end main*/
#ifdef CONFIG_DUAL_CHANNEL
	else {	/* For Sub*/
		/*sub channel not support this function*/
	}
#endif
}
void drvif_color_icm_SKIN_TONE(unsigned char display, unsigned short *tICM_ini, unsigned char which_table, unsigned short nValue)
{
	int h, si;
	int h_sta, h_end, si_sta, si_end;
	unsigned char if_write_ic;
	int timeout;
	int LayerSize;

	unsigned char ICM_Num_Limite	= 255;/*30;*/ /*planck changed to 255 for the max of icm tables.20101111*/

	/* Global Setting*/
	unsigned int baseAddr			= which_table * VIP_ICM_TBL_Y * VIP_ICM_TBL_Z;
	/*unsigned int baseAddr_global	= baseAddr + 0 * VIP_ICM_TBL_Z;*/		/*global control*/
	/*unsigned int baseAddr_pillar	= baseAddr + 1 * VIP_ICM_TBL_Z;*/		/*pillar*/

	unsigned int baseAddr_hue_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 0) * VIP_ICM_TBL_Z;		/*hue table*/
	/*unsigned int baseAddr_sat_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 1) * VIP_ICM_TBL_Z;*/		/*sat table*/
	/*unsigned int baseAddr_itn_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 2) * VIP_ICM_TBL_Z;*/		/*itn table*/

	VIPprintf("drvif_color_icm_table_select Initialize  which_table= %d\n", which_table);

	if (which_table >= ICM_Num_Limite) /* overflow */
		return;

	if (display == SLR_MAIN_DISPLAY) {
		/* icm table, hue/sat/itn*/
		color_icm_dm_icm_3d_tab_sram_ctrl_RBUS        dm_icm_3d_tab_sram_ctrl_reg;
		color_icm_dm_icm_3d_tab_write_hsi_00_RBUS     dm_icm_3d_tab_write_hsi_00_reg;
		color_icm_dm_icm_3d_tab_write_hsi_10_RBUS     dm_icm_3d_tab_write_hsi_10_reg;
		color_icm_dm_icm_3d_tab_write_hsi_20_RBUS     dm_icm_3d_tab_write_hsi_20_reg;
		color_icm_dm_icm_3d_tab_write_hsi_30_RBUS     dm_icm_3d_tab_write_hsi_30_reg;
		color_icm_dm_icm_3d_tab_write_hsi_40_RBUS     dm_icm_3d_tab_write_hsi_40_reg;
		color_icm_dm_icm_3d_tab_write_hsi_50_RBUS     dm_icm_3d_tab_write_hsi_50_reg;
		color_icm_dm_icm_3d_tab_write_hsi_60_RBUS     dm_icm_3d_tab_write_hsi_60_reg;
		color_icm_dm_icm_3d_tab_write_hsi_70_RBUS     dm_icm_3d_tab_write_hsi_70_reg;
		color_icm_dm_icm_3d_tab_write_hsi_80_RBUS     dm_icm_3d_tab_write_hsi_80_reg;
		color_icm_dm_icm_3d_tab_write_hsi_90_RBUS     dm_icm_3d_tab_write_hsi_90_reg;
		color_icm_dm_icm_3d_tab_write_hsi_a0_RBUS     dm_icm_3d_tab_write_hsi_a0_reg;
		color_icm_dm_icm_3d_tab_write_hsi_b0_RBUS     dm_icm_3d_tab_write_hsi_b0_reg;

		h_sta = 0;
		h_end = HUESEGMAX - 1;
		si_sta = 0;
		si_end = SATSEGMAX - 1;
		if_write_ic = 1;
		LayerSize = HUESEGMAX * SATSEGMAX;

		/*write*/
		dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
		dm_icm_3d_tab_sram_ctrl_reg.sram_type = 0; /* base on H, S axis => Read/Write to I axis*/
		dm_icm_3d_tab_sram_ctrl_reg.write_random_access_en = 0; /*block write*/
		dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt = 0;
		IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
		for (si = si_sta; si <= si_end; si++) {
			for (h = h_sta; h <= h_end; h++) {
				dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_h = h;
				dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_s = si;
				dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_i = 0;

				dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = tICM_ini[baseAddr_hue_tab + LayerSize * 0  + si * HUESEGMAX + h]+nValue;
				dm_icm_3d_tab_write_hsi_10_reg.write_table_h1 = tICM_ini[baseAddr_hue_tab + LayerSize * 1  + si * HUESEGMAX + h]+nValue;
				dm_icm_3d_tab_write_hsi_20_reg.write_table_h2 = tICM_ini[baseAddr_hue_tab + LayerSize * 2  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_30_reg.write_table_h3 = tICM_ini[baseAddr_hue_tab + LayerSize * 3  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_40_reg.write_table_h4 = tICM_ini[baseAddr_hue_tab + LayerSize * 4  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_50_reg.write_table_h5 = tICM_ini[baseAddr_hue_tab + LayerSize * 5  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_60_reg.write_table_h6 = tICM_ini[baseAddr_hue_tab + LayerSize * 6  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_70_reg.write_table_h7 = tICM_ini[baseAddr_hue_tab + LayerSize * 7  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_80_reg.write_table_h8 = tICM_ini[baseAddr_hue_tab + LayerSize * 8  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_90_reg.write_table_h9 = tICM_ini[baseAddr_hue_tab + LayerSize * 9  + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_a0_reg.write_table_ha = tICM_ini[baseAddr_hue_tab + LayerSize * 10 + si * HUESEGMAX + h];
				dm_icm_3d_tab_write_hsi_b0_reg.write_table_hb = tICM_ini[baseAddr_hue_tab + LayerSize * 11 + si * HUESEGMAX + h];


				/*write*/
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_00_reg, dm_icm_3d_tab_write_hsi_00_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_10_reg, dm_icm_3d_tab_write_hsi_10_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_20_reg, dm_icm_3d_tab_write_hsi_20_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_30_reg, dm_icm_3d_tab_write_hsi_30_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_40_reg, dm_icm_3d_tab_write_hsi_40_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_50_reg, dm_icm_3d_tab_write_hsi_50_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_60_reg, dm_icm_3d_tab_write_hsi_60_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_70_reg, dm_icm_3d_tab_write_hsi_70_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_80_reg, dm_icm_3d_tab_write_hsi_80_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_90_reg, dm_icm_3d_tab_write_hsi_90_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_A0_reg, dm_icm_3d_tab_write_hsi_a0_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_B0_reg, dm_icm_3d_tab_write_hsi_b0_reg.regValue);


				dm_icm_3d_tab_sram_ctrl_reg.write_en = 1;
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
				timeout = 20;
				do {
					dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
				} while (dm_icm_3d_tab_sram_ctrl_reg.write_en && (timeout-- > 0));

				if (timeout <= 0) {
					VIPprintf("Color_ICM_Table_Select write icm table error: sram_addrres_h = %d, sram_addrres_s = %d\n", h, si);
					return;
				}
			}
		}
	}   /* end main*/
}

/*Protection for not over near axis*/
/*h_pillar Range : 0~63 , s_pillar Range : 0~11, i_piller Range : 0~11*/
unsigned char drvif_color_icm_Adjust_Protection(unsigned int *buf, int nOffset, int adjust_len , int h_sta, int h_end, unsigned char h_total,
						int s_sta, int s_end, unsigned char s_total, int i_sta, int i_end, unsigned char i_total)
{
	int h, s, i;
	int hlen, slen, ilen, hmax;
	int hue_C = 0, Sat_C = 0, hue_LB = 0, hue_RB = 0;
	bool if_write_ic = 1;

	color_icm_dm_icm_ctrl_RBUS  dm_icm_ctrl_reg;
	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	ilen = i_end - i_sta + 1;
	hmax = (h_total-1);

	if (hmax > (HUESEGMAX-1) || hmax < 47)
		return false;

	/*VIPprintf("i_sta=%d, i_end=%d\n", i_sta, i_end);*/
	/*VIPprintf("s_sta=%d, s_end=%d\n", s_sta, s_end);*/
	/*VIPprintf("h_sta=%d, h_end=%d\n", h_sta, h_end);*/

	for (i = i_sta; i <= i_end; i++) {
		for (s = s_sta; s <= s_end; s++) {
			hue_LB = ((buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h_sta - h_sta)) * 2 + 0]) >> 16) & 0x7ff;
			hue_RB = ((buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h_end - h_sta)) * 2 + 0]) >> 16) & 0x7ff;
			hue_LB = drvif_color_icm_FormatValue((hue_LB-nOffset), _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
			hue_RB = drvif_color_icm_FormatValue((hue_RB-nOffset), _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);

			for (h = h_sta; h <= h_end; h++) {
				/*VIPprintf("i=%d, s=%d, h=%d\n", i, s, h);*/
				/*VIPprintf("hue_LB=%d, hue_RB=%d\n", hue_LB, hue_RB);*/

				hue_C = ((buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0]) >> 16) & 0x7ff;
				Sat_C = ((buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0])) & 0x1fff;

				if ((h > h_sta) && (h < h_end)) {
					/*VIPprintf("Sat_C=%d, hue_C=%d\n", Sat_C, hue_C);*/
					if (hue_LB > hue_RB) {
						if (((hue_C-hue_LB) < 0) && (hue_C > 1000)) {
							hue_C = hue_LB;
						} else if (((hue_C-hue_RB) > 0) && (hue_C < 1000)) {
							hue_C = hue_RB;
						}
					} else if (hue_LB < hue_RB) {
						if ((hue_C-hue_LB) < 0) {
							hue_C = hue_LB;
						} else if ((hue_C-hue_RB) > 0) {
							hue_C = hue_RB;
						}
					}
				} else if (h == h_sta) {
					/*VIPprintf("hue_LB=%d, hue_C=%d\n", hue_LB, hue_C);*/
					hue_C = hue_LB;

				} else if (h == h_end) {
					/*VIPprintf("hue_RB=%d, hue_C=%d\n", hue_RB, hue_C);*/
					hue_C = hue_RB ;
				}
				buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = (hue_C << 16)+Sat_C;
			}
		}
	}

	return true;

}


void drvif_color_icm_Adjust_setting(bool isRollback, unsigned int *buf, int nOffset, int adjust_len , int h_sta, int h_end, unsigned char h_total,
						int s_sta, int s_end, unsigned char s_total, int i_sta, int i_end, unsigned char i_total, unsigned char HSI, unsigned char h_part)
{

	color_icm_dm_icm_ctrl_RBUS  dm_icm_ctrl_reg;
	int h, s, i;
	int hlen, slen, ilen, hmax;
	int hsi_00, hsi_01;
	int hue_Cur, hue_Adj, sat_Cur, itn_Cur;
	int offset;
	bool if_write_ic = 1;
	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);
	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	ilen = i_end - i_sta + 1;
	hmax = (h_total-1);

	for (i = i_sta; i <= i_end; i++) {
		for (s = s_sta; s <= s_end; s++) {
			for (h = h_sta; h <= h_end; h++) {
				hsi_00 = (buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0]) ;
				hsi_01 = (buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1]) ;

				switch(HSI)
				{
					case _HUE_ELEM:
						hue_Cur = (hsi_00 >> 16) & 0x7ff;
						sat_Cur = (hsi_00) & 0x1fff;
#if 0
						offset = nOffset;
						if((h_part == 1 && h==h_sta) || (h_part ==2 && h == h_end)) //boundary
							offset = nOffset/2;
						if(h_part ==0 && (h==h_sta || h == h_end))//boundary
							offset = nOffset/2;
#endif
						offset = nOffset/2;

						//. ratio offset
						if(isRollback)
						{
							if(offset<0) //h: (h_sta+1)~ h_end; 1~8
							{
								if(h_part == 1)//R sta side
									hue_Adj = drvif_color_icm_FormatValue(hue_Cur + (h_end + h_end2 - h + 2)*offset, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
								else
									hue_Adj = drvif_color_icm_FormatValue(hue_Cur + (h_end - h + 1)*offset, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
							}
							else
							{
								if(h_part == 2)//R end side
									hue_Adj = drvif_color_icm_FormatValue(hue_Cur + (h - h_sta1 + (h_total - h_sta))*offset, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
								else
									hue_Adj = drvif_color_icm_FormatValue(hue_Cur + (h - h_sta)*offset, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
							}
						}
						else
						{
							if(offset>0) //h: (h_sta+1)~ h_end; 1~8
							{
								if(h_part == 1)//R sta side
									hue_Adj = drvif_color_icm_FormatValue(hue_Cur + (h_end + h_end2 - h + 2)*offset, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
								else
									hue_Adj = drvif_color_icm_FormatValue(hue_Cur + (h_end - h + 1)*offset, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
							}
							else
							{
								if(h_part == 2)//R end side
									hue_Adj = drvif_color_icm_FormatValue(hue_Cur + (h - h_sta1 + (h_total - h_sta))*offset, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
								else
									hue_Adj = drvif_color_icm_FormatValue(hue_Cur + (h - h_sta)*offset, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
							}
						}
						//rtd_pr_vpq_info("[MHH]offset = %d, h = %d, hue_Adj = %d, hue_Cur = %d, sat_Cur = %d\n", offset, h, hue_Adj, hue_Cur, sat_Cur);
						//rtd_pr_vpq_info("[MHH]buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = %d\n", buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0]);
						buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = (hue_Adj<< 16) + sat_Cur;
						break;

					case _SAT_ELEM:
						if(i == i_sta || s == s_sta) continue;

						offset = nOffset;
						if((h_part == 1 && h==h_sta) || (h_part ==2 && h == h_end)) offset = nOffset/2;
						if(h_part ==0 && (h==h_sta|| h == h_end)) offset = nOffset/2;

						//. ratio offset
						if(isRollback)
						{
							if(offset>0) //s: (s_sta+1)~ s_end; 1~8
								buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = hsi_00 + s*offset;
							else
								buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = hsi_00 + (s_end+1-s)*offset;
						}
						else
						{
							if(offset<0) //s: (s_sta+1)~ s_end; 1~8
								buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = hsi_00 + s*offset;
							else
								buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = hsi_00 + (s_end+1-s)*offset;
						}

						//. const offset
						buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = hsi_00 + offset;
					 	break;

					case _ITN_ELEM:
						if(i == i_sta || s == s_sta) continue;
						itn_Cur = (hsi_01) & 0x1fff;

						offset = nOffset;
						if((h_part == 1 && h==h_sta) || (h_part ==2 && h == h_end)) offset = nOffset/2;
						if(h_part ==0 && (h==h_sta|| h == h_end)) offset = nOffset/2;

						//. ratio offset
						if(isRollback)
						{
							if(offset>0) //i: (i_sta+1)~ i_end; 1~8
								buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1] = itn_Cur + i*offset;
							else
								buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1] = itn_Cur + (i_end+1-i)*offset;
						}
						else
						{
							if(offset<0) //i: (i_sta+1)~ i_end; 1~8
								buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1] = itn_Cur + i*offset;
							else
								buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1] = itn_Cur + (i_end+1-i)*offset;
						}

						//. const offset
						buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1] = itn_Cur + offset;
					 	break;

					default:
					 	break;
				}

			}
		}
	}
}

/*Protection for not over near axis*/
/*h_pillar Range : 0~63 , s_pillar Range : 0~11, i_piller Range : 0~11*/
int drvif_color_icm_Adjust_hue(unsigned int *buf, int nOffset, int adjust_len , int h_sta, int h_end, unsigned char h_total,
						int s_sta, int s_end, unsigned char s_total, int i_sta, int i_end, unsigned char i_total)
{

	int s, i;
	int hlen, slen, ilen, hmax;
	int hue_Cur = 0, hue_Bnd = 0, hue_Fmt = 0;
	bool if_write_ic = 1;
	unsigned int *buf_local;
	bool readok;
	int offset_tmp = nOffset;
	int offset_final;
	color_icm_dm_icm_ctrl_RBUS  dm_icm_ctrl_reg;

	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	ilen = i_end - i_sta + 1;
	hmax = (h_total-1);
	if (hmax > (HUESEGMAX-1) || hmax < 47)
		return 0;
	buf_local = (unsigned int*)vip_malloc (((i_end - i_sta + 1) * (s_end - s_sta + 1) * 1 * 2)*sizeof(unsigned int)); // two registers

	if (buf_local == NULL)
		return 0;

	/* get boundary */
	if(nOffset<0 )
	{
		if(h_sta == 0)
			readok = drvif_color_icm_SramBlockReadSpeedup(buf_local, h_total,  h_total,  s_sta,  s_end,  i_sta,  i_end,_HUE_ELEM, 0);
		else
			readok = drvif_color_icm_SramBlockReadSpeedup(buf_local, h_sta-1,  h_sta-1,  s_sta,  s_end,  i_sta,  i_end,_HUE_ELEM, 0);

		offset_final = -256;
	}
	else
	{
		if(h_end == h_total)
			readok = drvif_color_icm_SramBlockReadSpeedup(buf_local, 0,  0,  s_sta,  s_end,  i_sta,  i_end,_HUE_ELEM, 0);
		else
			readok = drvif_color_icm_SramBlockReadSpeedup(buf_local, h_end+1,  h_end+1,  s_sta,  s_end,  i_sta,  i_end,_HUE_ELEM, 0);

		offset_final = 256;
	}

	if(!readok){
		if (buf_local) 
			vip_free(buf_local);
		return 0;
	}

	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	/* find max/min offset */
	for (i = i_sta; i <= i_end; i++) {
		for (s = s_sta; s <= s_end; s++) {
				hue_Bnd = ((buf_local[(((i - i_sta) * slen + (s - s_sta)) * 1) * 2 + 0]) >> 16) & 0x7ff;

				if(nOffset<0)
					hue_Cur = ((buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h_sta - h_sta)) * 2 + 0]) >> 16) & 0x7ff;
				else
					hue_Cur = ((buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h_end - h_sta)) * 2 + 0]) >> 16) & 0x7ff;

				hue_Fmt = drvif_color_icm_FormatValue(hue_Cur+nOffset, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);

				if(nOffset<0)
				{
					if(hue_Cur < hue_Bnd)
					{
						if(hue_Cur+nOffset<0 && hue_Fmt<hue_Bnd)
							offset_tmp = hue_Bnd - hue_Fmt + nOffset;
					}
					else if(hue_Cur > hue_Bnd)
					{
						if(hue_Cur+nOffset<hue_Bnd)
							offset_tmp = hue_Bnd - hue_Cur;
					}
					else{
						if (buf_local) 
							vip_free(buf_local); // Klocwork analysis
						return 0;
					}
					if(offset_final < offset_tmp) //. find max
						offset_final = offset_tmp;
				}
				else //. nOffset>0
				{
					if(hue_Cur > hue_Bnd)
					{
						if(hue_Cur+nOffset>1535 && hue_Fmt>hue_Bnd)
							offset_tmp = hue_Bnd - hue_Fmt + nOffset;
					}
					else if(hue_Cur < hue_Bnd)
					{
						if(hue_Cur+nOffset>hue_Bnd)
							offset_tmp = hue_Bnd - hue_Cur;
					}
					else{
						if (buf_local) 
							vip_free(buf_local); // Klocwork analysis
						return 0;
					}

					if(offset_final > offset_tmp) //. find min
						offset_final = offset_tmp;
				}
		}
	}
	if (buf_local)
		vip_free(buf_local);
	return offset_final;
}

int drvif_color_icm_Adjust_sat(unsigned int *buf, int nOffset, int adjust_len , int h_sta, int h_end, unsigned char h_total,
						int s_sta, int s_end, unsigned char s_total, int i_sta, int i_end, unsigned char i_total)
{
	int h, s, i;
	int hlen, slen, ilen, hmax;
	int sat_Cur = 0, sat_Bnd = 0;
	int offset_tmp = nOffset;
	int offset_final;

	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	ilen = i_end - i_sta + 1;
	hmax = (h_total-1);
	if (hmax > (HUESEGMAX-1) || hmax < 47)
		return 0;

	/* get boundary */
	if(nOffset<0 )
		offset_final = -1024;
	else
		offset_final = 1024;

	/* find max/min offset */
	for (s = s_sta; s <= s_end; s++) {
		for (i = i_sta; i <= i_end; i++) {
			for (h = h_sta; h <= h_end; h++) {

				if(nOffset<0)
				{
					if(s == s_end) continue;
					sat_Bnd = (buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0]) & 0x1fff;
					sat_Cur = (buf[(((i - i_sta) * slen + (s + 1 - s_sta)) * hlen + (h - h_sta)) * 2 + 0]) & 0x1fff;

					if(sat_Cur+nOffset<= sat_Bnd)
						offset_tmp = sat_Bnd - sat_Cur+1;
					//rtd_pr_vpq_info("check3, (h,s,i) = %d,%d,%d, sat_Bnd = %d, sat_Cur = %d, nOffset = %d, offset_tmp = %d", h, s, i, sat_Bnd, sat_Cur, nOffset, offset_tmp);
					//if(offset_final < offset_tmp) //. find max
						//offset_final = offset_tmp;
					if(h != h_sta && h != h_end)
					{
						if(offset_final < offset_tmp ) //. find max
							offset_final = offset_tmp;
					}
					else
					{
						if(offset_final < offset_tmp*2 ) //. find max
							offset_final = offset_tmp*2; //due to boundary will only get 1/2 offset
					}
				}
				else
				{
					if(s==s_end)
						sat_Bnd = 6410;
					else
						sat_Bnd = (buf[(((i - i_sta) * slen + (s+1 - s_sta)) * hlen + (h - h_sta)) * 2 + 0]) & 0x1fff;
					sat_Cur = (buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0]) & 0x1fff;

					if(sat_Cur+nOffset>=sat_Bnd)
						offset_tmp = sat_Bnd - sat_Cur-1;

					//if(offset_final > offset_tmp) //. find min
						//offset_final = offset_tmp;

					if(h != h_sta && h != h_end)
					{
						if(offset_final > offset_tmp) //. find min
							offset_final = offset_tmp;
					}
					else
					{
						if(offset_final > offset_tmp*2 ) //. find min
							offset_final = offset_tmp*2; //due to boundary will only get 1/2 offset
					}
				}
			}
		}
	}

	return offset_final;
}

int drvif_color_icm_Adjust_itn(unsigned int *buf, int nOffset, int adjust_len , int h_sta, int h_end, unsigned char h_total,
						int s_sta, int s_end, unsigned char s_total, int i_sta, int i_end, unsigned char i_total)
{
	int h, s, i;
	int hlen, slen, ilen, hmax;
	int itn_Cur = 0, itn_Bnd = 0;
	int offset_tmp = nOffset;
	int offset_final;

	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	ilen = i_end - i_sta + 1;
	hmax = (h_total-1);
	if (hmax > (HUESEGMAX-1) || hmax < 47)
		return 0;

	/* get boundary */
	if(nOffset<0 )
		offset_final = -1024;
	else
		offset_final = 1024;
	rtd_pr_vpq_info("check2, nOffset = %d", nOffset);
	/* find max/min offset */
	for (i = i_sta; i <= i_end; i++) {
		for (s = s_sta; s <= s_end; s++) {
			for (h = h_sta; h <= h_end; h++) {

				if(nOffset<0)
				{
					if(i == i_end) continue;
					itn_Bnd = (buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1]) & 0x1fff;
					itn_Cur = (buf[(((i + 1 - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1]) & 0x1fff;

					if(itn_Cur+nOffset<= itn_Bnd)
						offset_tmp = itn_Bnd - itn_Cur + 1;
					//rtd_pr_vpq_info("check3, (h,s,i) = %d,%d,%d, itn_Bnd = %d, itn_Cur = %d, nOffset = %d, offset_tmp = %d", h, s, i,itn_Bnd, itn_Cur, nOffset, offset_tmp);
					if(h != h_sta && h != h_end)
					{
						if(offset_final < offset_tmp ) //. find max
							offset_final = offset_tmp;
					}
					else
					{
						if(offset_final < offset_tmp*2 ) //. find max
							offset_final = offset_tmp*2; //due to boundary will only get 1/2 offset
					}
					
				}
				else
				{
					if(i == i_sta  || s == s_sta) continue;
					if(i==i_end)
						itn_Bnd = 7135;
					else
						itn_Bnd = (buf[(((i+1 - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1]) & 0x1fff;
					itn_Cur = (buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1]) & 0x1fff;

					if(itn_Cur+nOffset>=itn_Bnd)
						offset_tmp = itn_Bnd - itn_Cur - 1;

					if(h != h_sta && h != h_end)
					{
						if(offset_final > offset_tmp) //. find min
							offset_final = offset_tmp;
					}
					else
					{
						if(offset_final > offset_tmp*2 ) //. find min
							offset_final = offset_tmp*2; //due to boundary will only get 1/2 offset
					}
				}
			}
		}
	}

	return offset_final;
}

int drvif_color_icm_Adjust_Block(unsigned char HSI, int prvOffset, int nOffset, int adjust_len, int h_sta, int h_end, unsigned char h_total, int s_sta, int s_end, unsigned char s_total, int i_sta, int i_end, unsigned char i_total)
{

	if(nOffset==0 && prvOffset==0)
		return 0;

// hue: 		| R | S Y G C B M	| R |
// h_part:	| 2 | 	0 		| 1 |
// pillar:		0...	         		   ...47

	if(h_sta>h_end) //. hue R
	{
		unsigned int *buf1;
		unsigned int *buf2;
		bool readok1, readok2;
		int tmp1, tmp2;
		int offset_out=0;

		h_sta1 = h_sta;
		h_end1 = h_total-1;
		h_sta2 = 0;
		h_end2 = h_end;
		
		//. read
		buf1 = (unsigned int*)vip_malloc(((i_end - i_sta + 1) * (s_end - s_sta + 1) * (h_end1 - h_sta1 + 1) * 2)*sizeof(unsigned int)); // two registers
		if(buf1 == NULL)
		{
			rtd_pr_vpq_info("drvif_color_icm_Adjust_Block, memory allocate fail\n");
			return -1;
		}
		readok1 = drvif_color_icm_SramBlockReadSpeedup(buf1, h_sta1,  h_end1,  s_sta,  s_end,  i_sta,  i_end,0, 0);
		
		buf2 = (unsigned int*)vip_malloc(((i_end - i_sta + 1) * (s_end - s_sta + 1) * (h_end2 - h_sta2 + 1) * 2)*sizeof(unsigned int)); // two registers
		if(buf2 == NULL)
		{
			rtd_pr_vpq_info("drvif_color_icm_Adjust_Block, memory allocate fail\n");
			vip_free(buf1);
			return -1;
		}
		
		readok2 = drvif_color_icm_SramBlockReadSpeedup(buf2, h_sta2,  h_end2,  s_sta,  s_end,  i_sta,  i_end,0, 0);
		if (!readok1 || !readok2){
			 if(buf1)
				vip_free(buf1);
			 
			 if(buf2)
				vip_free(buf2);
			 
			 return 0;
		}

		//. rollback previous adjustment
		drvif_color_icm_Adjust_setting( 1, buf1, -prvOffset, adjust_len, h_sta1,  h_end1, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total, HSI, 1);
		drvif_color_icm_Adjust_setting( 1, buf2, -prvOffset, adjust_len, h_sta2,  h_end2, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total, HSI, 2);
		//if(buf1 == NULL)
		//	rtd_pr_vpq_info("buf1 == NULL");
		//if(buf2 == NULL)
		//	rtd_pr_vpq_info("buf2 == NULL");
		//. find protected offset
		switch(HSI)
		{
			case _HUE_ELEM:
				if(nOffset)
				{
					if(nOffset<0)
					{
						offset_out = drvif_color_icm_Adjust_hue( buf1, nOffset, adjust_len, h_sta1,  h_end1, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);
					}
					else
					{
						offset_out = drvif_color_icm_Adjust_hue( buf2, nOffset, adjust_len, h_sta2,  h_end2, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);
					}
				}
				break;

			case _SAT_ELEM:
				if(nOffset)
				{
					tmp1 = drvif_color_icm_Adjust_sat( buf1, nOffset, adjust_len, h_sta1,  h_end1, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);
					tmp2 = drvif_color_icm_Adjust_sat( buf2, nOffset, adjust_len, h_sta2,  h_end2, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);

					if(nOffset<0)
						offset_out = (tmp1>tmp2)?tmp1:tmp2;
					else
						offset_out = (tmp1<tmp2)?tmp1:tmp2;
				}
				break;

			case _ITN_ELEM:
				if(nOffset)
				{
					tmp1 = drvif_color_icm_Adjust_itn( buf1, nOffset, adjust_len, h_sta1,  h_end1, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);
					tmp2 = drvif_color_icm_Adjust_itn( buf2, nOffset, adjust_len, h_sta2,  h_end2, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);

					if(nOffset<0)
						offset_out = (tmp1>tmp2)?tmp1:tmp2;
					else
						offset_out = (tmp1<tmp2)?tmp1:tmp2;
				}
			 	break;

			default:
			 	break;
		}

		//. set current adjustment
		if (offset_out)
		{
			drvif_color_icm_Adjust_setting( 0, buf1, offset_out, adjust_len, h_sta1,  h_end1, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total, HSI, 1);
			drvif_color_icm_Adjust_setting( 0, buf2, offset_out, adjust_len, h_sta2,  h_end2, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total, HSI, 2);
		}

		//. write
		drvif_color_icm_SramBlockAccessSpeedup(buf1,1,  h_sta1,  h_end1,  s_sta,  s_end,  i_sta,  i_end);
	 	drvif_color_icm_SramBlockAccessSpeedup(buf2,1,  h_sta2,  h_end2,  s_sta,  s_end,  i_sta,  i_end);

		if(buf1)
			vip_free(buf1);
		if(buf2)
			vip_free(buf2);
		return offset_out;

	}
	else //. others hue
	{
		unsigned int *buf;
		bool readok=0;
		int offset_out=0;

		buf = (unsigned int*)vip_malloc(((i_end - i_sta + 1) * (s_end - s_sta + 1) * (h_end - h_sta + 1) * 2)*sizeof(unsigned int)); // two registers
		if(buf == NULL)
		{
			rtd_pr_vpq_info("drvif_color_icm_Adjust_Block, memory allocate fail\n");
			return -1;
		}
		//. read
		readok = drvif_color_icm_SramBlockReadSpeedup(buf, h_sta,  h_end,  s_sta,  s_end,  i_sta,  i_end,0, 0);

		if (!readok) {
			if(buf)
				vip_free(buf);
			return 0;
		}

		//. rollback previous adjustment
		drvif_color_icm_Adjust_setting( 1, buf, -prvOffset, adjust_len, h_sta,  h_end, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total, HSI, 0);

		//. find protected offset
		switch(HSI)
		{
			case _HUE_ELEM:
				if(nOffset)
					offset_out = drvif_color_icm_Adjust_hue( buf, nOffset, adjust_len, h_sta,  h_end, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);
				break;

			case _SAT_ELEM:
				
				rtd_pr_vpq_info("(nOffset = %d)", nOffset);
				if(nOffset)
					offset_out = drvif_color_icm_Adjust_sat( buf, nOffset, adjust_len, h_sta,  h_end, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);
				rtd_pr_vpq_info("(offset_out = %d)", offset_out);

				break;

			case _ITN_ELEM:
				rtd_pr_vpq_info("(nOffset = %d)", nOffset);
				if(nOffset)
					offset_out = drvif_color_icm_Adjust_itn( buf, nOffset, adjust_len, h_sta,  h_end, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);
				rtd_pr_vpq_info("(offset_out = %d)", offset_out);
				break;

			default:
			 	break;
		}

		//. set current adjustment
		if (offset_out)
			drvif_color_icm_Adjust_setting( 0, buf, offset_out, adjust_len, h_sta,  h_end, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total, HSI, 0);

		//. write
		drvif_color_icm_SramBlockAccessSpeedup(buf,1,  h_sta,  h_end,  s_sta,  s_end,  i_sta,  i_end);

		if(buf)
			vip_free(buf);
		return offset_out;
	}
}

unsigned char drvif_color_icm_SramBlockAccessSpeedup(unsigned int *buf, unsigned char if_write_ic, int h_sta, int h_end, int s_sta, int s_end, int i_sta, int i_end)
{
	unsigned char result;
	if (if_write_ic) {
		/*drvif_color_icm_GetBufFromICMTab(buf, ICM_TAB_WRITE, h_sta, h_end, s_sta, s_end, i_sta, i_end);*/ /*have modify global 3d table*/
		result = drvif_color_icm_SramBlockWriteSpeedup(buf, h_sta, h_end, s_sta, s_end, i_sta, i_end);
	} else {
		result = drvif_color_icm_SramBlockReadSpeedup(buf, h_sta, h_end, s_sta, s_end, i_sta, i_end, 0, 0);
		/* drvif_color_icm_SetBufToICMTab(buf, ICM_TAB, h_sta, h_end, s_sta, s_end, i_sta, i_end);  */ /* only local 3d table*/
	}
	return result;
}
unsigned char drvif_color_icm_GetBufFromICMTab(unsigned int *buf, COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX], int h_sta, int h_end, int s_sta, int s_end, int i_sta, int i_end)
{
	int h, s, i;
	int hlen, slen, ilen;
	unsigned char if_write_ic = 1;
	color_icm_dm_icm_3d_tab_write_hsi_00_RBUS     dm_icm_3d_tab_write_hsi_00_reg;
	color_icm_dm_icm_3d_tab_write_hsi_01_RBUS     dm_icm_3d_tab_write_hsi_01_reg;
	color_icm_dm_icm_ctrl_RBUS                dm_icm_ctrl_reg;
	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	ilen = i_end - i_sta + 1;

	for (i = i_sta; i <= i_end; i++) {
		for (s = s_sta; s <= s_end; s++) {
			for (h = h_sta; h <= h_end; h++) {
				dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = drvif_color_icm_FormatValue(ICM_TAB_ACCESS[i][s][h].H, _HUE_ELEM,  dm_icm_ctrl_reg.table_mode, if_write_ic);
				dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = drvif_color_icm_FormatValue(ICM_TAB_ACCESS[i][s][h].S, _SAT_ELEM,  dm_icm_ctrl_reg.table_mode, if_write_ic);
				dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = drvif_color_icm_FormatValue(ICM_TAB_ACCESS[i][s][h].I, _ITN_ELEM,  dm_icm_ctrl_reg.table_mode, if_write_ic);

				buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = dm_icm_3d_tab_write_hsi_00_reg.regValue;
				buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1] = dm_icm_3d_tab_write_hsi_01_reg.regValue;
			}
		}
	}
	return true;
}

unsigned char drvif_color_icm_SramBlockWriteSpeedup(unsigned int *buf, int h_sta, int h_end, int s_sta, int s_end, int i_sta, int i_end)
{
	int timeout = 20;
	int h, s, i;
	int hlen, slen, ilen;
	int hidx, hmax, h_pillar_num;

	color_icm_dm_icm_3d_tab_write_hsi_00_RBUS     dm_icm_3d_tab_write_hsi_00_reg;
	color_icm_dm_icm_3d_tab_write_hsi_01_RBUS     dm_icm_3d_tab_write_hsi_01_reg;
	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS        dm_icm_3d_tab_sram_ctrl_reg;

	color_icm_dm_icm_hue_segment_11_RBUS	  dm_icm_hue_segment_11_reg;
	dm_icm_hue_segment_11_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_11_reg);	
	h_pillar_num=dm_icm_hue_segment_11_reg.h_pillar_48+1;


	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	ilen = i_end - i_sta + 1;

	dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);

	dm_icm_3d_tab_sram_ctrl_reg.write_random_access_en = 1; /*random access the word*/
	dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt = 0;
	for (i = i_sta; i <= i_end; i++) {
		for (s = s_sta; s <= s_end; s++) {
			for (h = h_sta; h <= h_end; h++) {

				//constrain: if h segment num is odd, e.x. 2n+1(h pillar num=2n+2), need to copy h_idx=2n layer to h_idx=2n+1 layer, for hw need even h layer sram table 
				if((h_pillar_num % 2 == 0) && (h == h_pillar_num - 2))
				{
					hmax = h + 1;
				}
				else
				{
					hmax = h;
				}
	
				for(hidx = h; hidx <= hmax; hidx++)
				{
					/*ctrl*/
					dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_h = hidx;
					dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_s = s;
					dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_i = i;
					/*data*/
					dm_icm_3d_tab_write_hsi_00_reg.regValue = buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0];
					dm_icm_3d_tab_write_hsi_01_reg.regValue = buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1];

					IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_00_reg, dm_icm_3d_tab_write_hsi_00_reg.regValue);
					IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_01_reg, dm_icm_3d_tab_write_hsi_01_reg.regValue);

					/*write enable*/
					dm_icm_3d_tab_sram_ctrl_reg.write_en = 1;
					IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);

					timeout = 20;
					do {
						dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
					} while (dm_icm_3d_tab_sram_ctrl_reg.write_en && (timeout-- > 0));

					if (timeout <= 0)
						return false;
				}
			}
		}
	}
	return true;
}

unsigned char drvif_color_icm_SramBlockReadSpeedup(unsigned int *buf, int h_sta, int h_end, int s_sta, int s_end, int i_sta, int i_end, unsigned char nAxis, int nOffset)
{
	int k, index, flag;
	int bound[3];
	int timeout = 20;
	int h, s, i;
	int hlen, slen, ilen;
	int x, x_sta, x_end;
	int y, y_sta, y_end;
	int sramtype;
	int org_hue, org_sat, org_int;
	bool if_write_ic = 1;

	color_icm_dm_icm_ctrl_RBUS             		  dm_icm_ctrl_reg;

	color_icm_dm_icm_3d_tab_read_hsi_00_RBUS dm_icm_3d_tab_read_hsi_00_reg;
	color_icm_dm_icm_3d_tab_read_hsi_01_RBUS dm_icm_3d_tab_read_hsi_01_reg;
	color_icm_dm_icm_3d_tab_read_hsi_10_RBUS dm_icm_3d_tab_read_hsi_10_reg;
	color_icm_dm_icm_3d_tab_read_hsi_11_RBUS dm_icm_3d_tab_read_hsi_11_reg;
	color_icm_dm_icm_3d_tab_read_hsi_20_RBUS dm_icm_3d_tab_read_hsi_20_reg;
	color_icm_dm_icm_3d_tab_read_hsi_21_RBUS dm_icm_3d_tab_read_hsi_21_reg;
	color_icm_dm_icm_3d_tab_read_hsi_30_RBUS dm_icm_3d_tab_read_hsi_30_reg;
	color_icm_dm_icm_3d_tab_read_hsi_31_RBUS dm_icm_3d_tab_read_hsi_31_reg;
	color_icm_dm_icm_3d_tab_read_hsi_40_RBUS dm_icm_3d_tab_read_hsi_40_reg;
	color_icm_dm_icm_3d_tab_read_hsi_41_RBUS dm_icm_3d_tab_read_hsi_41_reg;
	color_icm_dm_icm_3d_tab_read_hsi_50_RBUS dm_icm_3d_tab_read_hsi_50_reg;
	color_icm_dm_icm_3d_tab_read_hsi_51_RBUS dm_icm_3d_tab_read_hsi_51_reg;

	color_icm_dm_icm_3d_tab_write_hsi_00_RBUS dm_icm_3d_tab_write_hsi_00_reg = {0};
	color_icm_dm_icm_3d_tab_write_hsi_01_RBUS dm_icm_3d_tab_write_hsi_01_reg = {0};

	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS        dm_icm_3d_tab_sram_ctrl_reg;

	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	ilen = i_end - i_sta + 1;

	/*actively, need to read back one line , then get one point*/
	if (slen > ilen) {
		sramtype = 1; /* base on HI*/
		y_sta = i_sta;
		y_end = i_end;
		x_sta = s_sta;
		x_end = s_end;
	} else {
		sramtype = 0; /* base on HS*/
		y_sta = s_sta;
		y_end = s_end;
		x_sta = i_sta;
		x_end = i_end;
	}
	bound[0] = x_sta;
	bound[2] = x_end + 1;


	dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
	dm_icm_3d_tab_sram_ctrl_reg.write_random_access_en = 0; /*clear*/
	dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt = 0;
	dm_icm_3d_tab_sram_ctrl_reg.sram_type = sramtype;


	for (h = h_sta; h <= h_end; h++) {
		for (y = y_sta; y <= y_end; y++) {
			dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_h = h;
			if (sramtype == 0)
				dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_s = y;
			else
				dm_icm_3d_tab_sram_ctrl_reg.sram_addrres_i = y;

			if (x_end < 6) {
				flag = 0x1;
				bound[1] = x_end + 1;
			} else if (x_sta > 5) {
				flag = 0x2;
				bound[1] = x_sta;
			} else {
				flag = 0x3;
				bound[1] = 6;
			}

			for (k = 0; k < 2; k++) {
				if ((flag >> k) & 0x1) {
					dm_icm_3d_tab_sram_ctrl_reg.read_sel = k;
					dm_icm_3d_tab_sram_ctrl_reg.read_en = 1;
					IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg, dm_icm_3d_tab_sram_ctrl_reg.regValue);
					timeout = 20;
					do {
						dm_icm_3d_tab_sram_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
					} while (dm_icm_3d_tab_sram_ctrl_reg.read_en && (timeout-- > 0));

					if (timeout <= 0)
						return false;

					for (x = bound[k]; x < bound[k + 1]; x++) {
						if (sramtype == 0) { /* based on HS */
							i = x;
							s = y;
						} else { /* based on HI */
							s = x;
							i = y;
						}
						index = x % 6;
						if (index == 0) {
							dm_icm_3d_tab_read_hsi_00_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_00_reg);
							dm_icm_3d_tab_read_hsi_01_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_01_reg);
							dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = dm_icm_3d_tab_read_hsi_00_reg.read_table_h0;
							dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = dm_icm_3d_tab_read_hsi_00_reg.read_table_s0;
							dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = dm_icm_3d_tab_read_hsi_01_reg.read_table_i0;
						} else if (index == 1) {
							dm_icm_3d_tab_read_hsi_10_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_10_reg);
							dm_icm_3d_tab_read_hsi_11_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_11_reg);
							dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = dm_icm_3d_tab_read_hsi_10_reg.read_table_h1;
							dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = dm_icm_3d_tab_read_hsi_10_reg.read_table_s1;
							dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = dm_icm_3d_tab_read_hsi_11_reg.read_table_i1;
						} else if (index == 2) {
							dm_icm_3d_tab_read_hsi_20_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_20_reg);
							dm_icm_3d_tab_read_hsi_21_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_21_reg);
							dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = dm_icm_3d_tab_read_hsi_20_reg.read_table_h2;
							dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = dm_icm_3d_tab_read_hsi_20_reg.read_table_s2;
							dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = dm_icm_3d_tab_read_hsi_21_reg.read_table_i2;
						} else if (index == 3) {
							dm_icm_3d_tab_read_hsi_30_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_30_reg);
							dm_icm_3d_tab_read_hsi_31_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_31_reg);
							dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = dm_icm_3d_tab_read_hsi_30_reg.read_table_h3;
							dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = dm_icm_3d_tab_read_hsi_30_reg.read_table_s3;
							dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = dm_icm_3d_tab_read_hsi_31_reg.read_table_i3;
						} else if (index == 4) {
							dm_icm_3d_tab_read_hsi_40_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_40_reg);
							dm_icm_3d_tab_read_hsi_41_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_41_reg);
							dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = dm_icm_3d_tab_read_hsi_40_reg.read_table_h4;
							dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = dm_icm_3d_tab_read_hsi_40_reg.read_table_s4;
							dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = dm_icm_3d_tab_read_hsi_41_reg.read_table_i4;
						} else if (index == 5) {
							dm_icm_3d_tab_read_hsi_50_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_50_reg);
							dm_icm_3d_tab_read_hsi_51_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_51_reg);
							dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = dm_icm_3d_tab_read_hsi_50_reg.read_table_h5;
							dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = dm_icm_3d_tab_read_hsi_50_reg.read_table_s5;
							dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = dm_icm_3d_tab_read_hsi_51_reg.read_table_i5;
						}

						if (nOffset != 0) {
							org_hue = dm_icm_3d_tab_write_hsi_00_reg.write_table_h0;
							org_sat = dm_icm_3d_tab_write_hsi_00_reg.write_table_s0;
							org_int = dm_icm_3d_tab_write_hsi_01_reg.write_table_i0;

							switch (nAxis) {
							case _HUE_ELEM:
									org_hue = org_hue + nOffset;
									org_hue = drvif_color_icm_FormatValue(org_hue, _HUE_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
									dm_icm_3d_tab_write_hsi_00_reg.write_table_h0 = org_hue;
							break;
							case _SAT_ELEM:
									org_sat = org_sat + nOffset;
									org_sat = drvif_color_icm_FormatValue(org_sat, _SAT_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
									dm_icm_3d_tab_write_hsi_00_reg.write_table_s0 = org_sat;
							break;
							case _ITN_ELEM:
									org_int = org_int + nOffset;
									org_int = drvif_color_icm_FormatValue(org_int, _ITN_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
									dm_icm_3d_tab_write_hsi_01_reg.write_table_i0 = org_int;
							break;
							}
						}

						buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0] = dm_icm_3d_tab_write_hsi_00_reg.regValue;
						buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1] = dm_icm_3d_tab_write_hsi_01_reg.regValue;
					}
				}
			}
		}
	}
	return true;
}

unsigned char drvif_color_icm_SetBufToICMTab(unsigned int *buf, COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX], int h_sta, int h_end, int s_sta, int s_end, int i_sta, int i_end)
{
	int h, s, i;
	int hlen, slen, ilen;
	unsigned char if_write_ic = 1;

	color_icm_dm_icm_3d_tab_read_hsi_00_RBUS      dm_icm_3d_tab_read_hsi_00_reg;
	color_icm_dm_icm_3d_tab_read_hsi_01_RBUS      dm_icm_3d_tab_read_hsi_01_reg;
	color_icm_dm_icm_ctrl_RBUS                dm_icm_ctrl_reg;

	dm_icm_3d_tab_read_hsi_00_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_00_reg);
	dm_icm_3d_tab_read_hsi_01_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_01_reg);

	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);
	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	ilen = i_end - i_sta + 1;

	for (i = i_sta; i <= i_end; i++) {
		for (s = s_sta; s <= s_end; s++) {
			for (h = h_sta; h <= h_end; h++) {
				dm_icm_3d_tab_read_hsi_00_reg.regValue = buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 0];
				dm_icm_3d_tab_read_hsi_01_reg.regValue = buf[(((i - i_sta) * slen + (s - s_sta)) * hlen + (h - h_sta)) * 2 + 1];

				ICM_TAB_ACCESS[i][s][h].H = drvif_color_icm_ReadHueValue(dm_icm_3d_tab_read_hsi_00_reg.read_table_h0, i, s, h);
				ICM_TAB_ACCESS[i][s][h].S = drvif_color_icm_FormatValue(dm_icm_3d_tab_read_hsi_00_reg.read_table_s0, _SAT_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);
				ICM_TAB_ACCESS[i][s][h].I = drvif_color_icm_FormatValue(dm_icm_3d_tab_read_hsi_01_reg.read_table_i0, _ITN_ELEM, dm_icm_ctrl_reg.table_mode, if_write_ic);

				ICM_TAB_ACCESS[i][s][h].H -= g_dhue;
				ICM_TAB_ACCESS[i][s][h].S -= g_dsatbysat[s];
				ICM_TAB_ACCESS[i][s][h].I -= g_ditnbyitn[i];
			}
		}
	}
	return true;
}

int drvif_color_icm_ReadHueValue(int hue, int ii, int is, int ih)
{
	int result;
	color_icm_dm_icm_hue_segment_11_RBUS      dm_icm_hue_segment_11_reg;
	dm_icm_hue_segment_11_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_11_reg);
	if (ih < 8 && hue > (HUERANGE / 2)) /* protect for first  bin */
		result = hue - HUERANGE;
	else if ((ih > dm_icm_hue_segment_11_reg.h_pillar_48 - 8) && (ih < dm_icm_hue_segment_11_reg.h_pillar_48)
		&& hue < (HUERANGE / 2))
		result = HUERANGE + hue;
	else
		result = hue;

	return result;
}

void drvif_color_Icm_ctrl_YUV2RGB(unsigned char data_format_sel, unsigned char data_range_sel)
{
	color_icm_dm_icm_ctrl_RBUS				  dm_icm_ctrl_reg;
	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	dm_icm_ctrl_reg.data_format_sel = data_format_sel;
	dm_icm_ctrl_reg.data_range_sel = data_range_sel;

	IoReg_Write32(COLOR_ICM_DM_ICM_CTRL_reg, dm_icm_ctrl_reg.regValue);
}

void drvif_color_set_Icm_RGB_mode(unsigned char value)
{
    color_icm_dm_icm_ctrl_RBUS                  dm_icm_ctrl_reg;
    dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

    dm_icm_ctrl_reg.bypass_yuv2rgb_enable = value; //0:RGB mode

    IoReg_Write32(COLOR_ICM_DM_ICM_CTRL_reg, dm_icm_ctrl_reg.regValue);
}