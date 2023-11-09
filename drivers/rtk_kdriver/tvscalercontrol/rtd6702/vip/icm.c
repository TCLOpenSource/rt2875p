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
//#include <mach/RPCDriver.h>
#include <linux/pageremap.h>

#include <linux/mm.h>

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

#include <linux/pageremap.h>


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

#define CLIP(min, max, val) ((val < min) ? min : (val > max ? max : val))
typedef struct _ICM_GLOBAL_BIT_MAP {
	int value:12;
	int reservd:20;
} ICM_GLOBAL_BIT_MAP;

#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_ICM_DEBUG,fmt,##args)
#define	vip_malloc(x)	kmalloc(x, GFP_KERNEL)
#define	vip_free(x)	kfree(x)

/*================================== Function ===============================*/
void drvif_color_icm_set_bypass_onoff(unsigned char display, unsigned char bEable)
{
	color_icm_dm_icm_ctrl_1_RBUS		color_icm_dm_icm_ctrl_1_reg;
    	color_icm_dm_icm_ctrl_1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_1_reg);
    	if (display == SLR_MAIN_DISPLAY)
	{
		color_icm_dm_icm_ctrl_1_reg.m_icm_bypass_en = bEable;
	}
#ifdef VIP_CONFIG_DUAL_CHANNEL
	else
	{
		color_icm_dm_icm_ctrl_1_reg.s_icm_bypass_en = bEable;
	}
#endif

    IoReg_Write32(COLOR_ICM_DM_ICM_CTRL_1_reg, 	color_icm_dm_icm_ctrl_1_reg.regValue);
}

void drvif_color_icm_hue_histo_init()
{
	/*hue histogram*/
	{
	color_icm_icm_auto_ctrl_RBUS 		icm_auto_ctrl_reg;
	color_icm_hue_hist_settings0_RBUS 	hue_hist_settings0_reg;
	color_icm_hue_hist_settings1_RBUS 	hue_hist_settings1_reg;

	icm_auto_ctrl_reg.regValue		= IoReg_Read32(COLOR_ICM_ICM_AUTO_CTRL_reg);
	hue_hist_settings0_reg.regValue	= IoReg_Read32(COLOR_ICM_HUE_HIST_SETTINGS0_reg);
	hue_hist_settings1_reg.regValue	= IoReg_Read32(COLOR_ICM_HUE_HIST_SETTINGS1_reg);

	hue_hist_settings1_reg.hue_hist_itn_hb = 5846;
	hue_hist_settings1_reg.hue_hist_itn_lb = -1752;
	hue_hist_settings0_reg.hue_hist_sat_lb = 0;

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
	d_auto_sat_hist_ctrl_reg.sat_hist_mode = 1;
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
	d_auto_sat_hist0_int_thrd_reg.d_hist0_thrd_auto_sat_i_u_thre0 = 5846;
	d_auto_sat_hist0_int_thrd_reg.d_hist0_thrd_auto_sat_i_l_thre0 = -1752;
	d_auto_sat_hist01_sat_thrd_reg.d_hist0_thrd_auto_sat_s_thre0 = 0;
	/*hist1*/
	d_auto_sat_hist1_int_thrd_reg.d_hist1_thrd_auto_sat_i_u_thre1 = 5846;
	d_auto_sat_hist1_int_thrd_reg.d_hist1_thrd_auto_sat_i_l_thre1 = -1752;
	d_auto_sat_hist01_sat_thrd_reg.d_hist1_thrd_auto_sat_s_thre1 = 0;
	/*hist2*/
	d_auto_sat_hist2_int_thrd_reg.d_hist2_thrd_auto_sat_i_u_thre2 = 5846;
	d_auto_sat_hist2_int_thrd_reg.d_hist2_thrd_auto_sat_i_l_thre2 = -1752;
	d_auto_sat_hist23_sat_thrd_reg.d_hist2_thrd_auto_sat_s_thre2 = 0;
	/*hist3*/
	d_auto_sat_hist3_int_thrd_reg.d_hist3_thrd_auto_sat_i_u_thre3 = 5846;
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
	//unsigned char if_write_ic;
	int timeout;
	int LayerSize;

	unsigned char ICM_Num_Limite	= 255;/*30;*/ /*planck changed to 255 for the max of icm tables.20101111*/

	/* Global Setting*/
	unsigned int baseAddr			= which_table * VIP_ICM_TBL_Y * VIP_ICM_TBL_Z;
	unsigned int baseAddr_global	= baseAddr + 0 * VIP_ICM_TBL_Z;		/*global control*/
	unsigned int baseAddr_hue_pillar	= baseAddr + 1 * VIP_ICM_TBL_Z;		/*hue pillar*/

	unsigned int baseAddr_hue_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 0) * VIP_ICM_TBL_Z;		/*hue table*/
	unsigned int baseAddr_sat_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 1) * VIP_ICM_TBL_Z;		/*sat table*/
	unsigned int baseAddr_itn_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 2) * VIP_ICM_TBL_Z;		/*itn table*/

	VIPprintf("drvif_color_icm_table_select Initialize  which_table= %d\n", which_table);

	if (which_table >= ICM_Num_Limite) /* overflow */
		return;

	/*enable icm*/
	{
	color_d_vc_global_ctrl_RBUS				d_vc_global_ctrl_reg;
	d_vc_global_ctrl_reg.regValue 	= IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_reg.m_vc_en		= 1;
	d_vc_global_ctrl_reg.write_enable_11 =1;
	d_vc_global_ctrl_reg.m_icm_en		= 1;
	d_vc_global_ctrl_reg.write_enable_3 =1;
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
	}

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
		color_icm_dm_icm_hue_segment_12_RBUS       dm_icm_hue_segment_12_reg;
		color_icm_dm_icm_hue_segment_13_RBUS       dm_icm_hue_segment_13_reg;
		color_icm_dm_icm_hue_segment_14_RBUS       dm_icm_hue_segment_14_reg;
		color_icm_dm_icm_hue_segment_15_RBUS       dm_icm_hue_segment_15_reg;
		color_icm_dm_icm_hue_segment_16_RBUS       dm_icm_hue_segment_16_reg;
		color_icm_dm_icm_hue_segment_17_RBUS       dm_icm_hue_segment_17_reg;
		color_icm_dm_icm_hue_segment_18_RBUS       dm_icm_hue_segment_18_reg;
		color_icm_dm_icm_hue_segment_19_RBUS       dm_icm_hue_segment_19_reg;
		color_icm_dm_icm_hue_segment_20_RBUS      dm_icm_hue_segment_20_reg;
		color_icm_dm_icm_hue_segment_21_RBUS      dm_icm_hue_segment_21_reg;
		color_icm_dm_icm_hue_segment_22_RBUS       dm_icm_hue_segment_22_reg;
		color_icm_dm_icm_hue_segment_23_RBUS       dm_icm_hue_segment_23_reg;
		color_icm_dm_icm_hue_segment_24_RBUS       dm_icm_hue_segment_24_reg;
		color_icm_dm_icm_hue_segment_25_RBUS       dm_icm_hue_segment_25_reg;
		color_icm_dm_icm_hue_segment_26_RBUS       dm_icm_hue_segment_26_reg;
		color_icm_dm_icm_hue_segment_27_RBUS       dm_icm_hue_segment_27_reg;
		color_icm_dm_icm_hue_segment_28_RBUS       dm_icm_hue_segment_28_reg;
		color_icm_dm_icm_hue_segment_29_RBUS       dm_icm_hue_segment_29_reg;

		color_icm_dm_icm_sat_segment_0_RBUS       dm_icm_sat_segment_0_reg;
		color_icm_dm_icm_sat_segment_1_RBUS       dm_icm_sat_segment_1_reg;
		color_icm_dm_icm_sat_segment_2_RBUS       dm_icm_sat_segment_2_reg;
		color_icm_dm_icm_sat_segment_3_RBUS       dm_icm_sat_segment_3_reg;
		color_icm_dm_icm_sat_segment_4_RBUS       dm_icm_sat_segment_4_reg;

		color_icm_dm_icm_int_segment_0_RBUS       dm_icm_int_segment_0_reg;
		color_icm_dm_icm_int_segment_1_RBUS       dm_icm_int_segment_1_reg;
		color_icm_dm_icm_int_segment_2_RBUS       dm_icm_int_segment_2_reg;
		color_icm_dm_icm_pillar_num_ctrl_RBUS		color_icm_dm_icm_pillar_num_ctrl_reg;

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
		for (i = 0; i < SATSEGMAX; i++) {
			g_dsatbysat[i]	= tICM_ini[baseAddr_global + 7 + i];
			g_dsatbysat[i] = ((ICM_GLOBAL_BIT_MAP *)(g_dsatbysat+i))->value;
		}
		/*g_ditnbyitn*/
		for (i = 0; i < ITNSEGMAX; i++) {
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

		/*pillar num*/
		color_icm_dm_icm_pillar_num_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_PILLAR_NUM_CTRL_reg);

            	color_icm_dm_icm_pillar_num_ctrl_reg.h_pillar_num = tICM_ini[baseAddr_global + 31];
            	color_icm_dm_icm_pillar_num_ctrl_reg.s_pillar_num =tICM_ini[baseAddr_global + 32];
            	color_icm_dm_icm_pillar_num_ctrl_reg.i_pillar_num = tICM_ini[baseAddr_global + 33];

		IoReg_Write32(COLOR_ICM_DM_ICM_PILLAR_NUM_CTRL_reg, color_icm_dm_icm_pillar_num_ctrl_reg.regValue);

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
		dm_icm_hue_segment_12_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_12_reg);
		dm_icm_hue_segment_13_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_13_reg);
		dm_icm_hue_segment_14_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_14_reg);
		dm_icm_hue_segment_15_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_15_reg);
		dm_icm_hue_segment_16_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_16_reg);
		dm_icm_hue_segment_17_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_17_reg);
		dm_icm_hue_segment_18_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_18_reg);
		dm_icm_hue_segment_19_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_19_reg);
		dm_icm_hue_segment_20_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_20_reg);
		dm_icm_hue_segment_21_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_21_reg);
		dm_icm_hue_segment_22_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_22_reg);
		dm_icm_hue_segment_23_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_23_reg);
		dm_icm_hue_segment_24_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_24_reg);
		dm_icm_hue_segment_25_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_25_reg);
		dm_icm_hue_segment_26_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_26_reg);
		dm_icm_hue_segment_27_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_27_reg);
		dm_icm_hue_segment_28_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_28_reg);
		dm_icm_hue_segment_29_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_29_reg);


		dm_icm_sat_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg);
		dm_icm_sat_segment_1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_1_reg);
		dm_icm_sat_segment_2_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_2_reg);
		dm_icm_sat_segment_3_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_3_reg);
		dm_icm_sat_segment_4_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_4_reg);

		dm_icm_int_segment_0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg);
		dm_icm_int_segment_1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_1_reg);
		dm_icm_int_segment_2_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_2_reg);


		/*hue pillar*/
		dm_icm_hue_segment_0_reg.h_pillar_1 = tICM_ini[baseAddr_hue_pillar + 0];
		dm_icm_hue_segment_0_reg.h_pillar_2 = tICM_ini[baseAddr_hue_pillar + 1];
		dm_icm_hue_segment_1_reg.h_pillar_3 = tICM_ini[baseAddr_hue_pillar + 2];
		dm_icm_hue_segment_1_reg.h_pillar_4 = tICM_ini[baseAddr_hue_pillar + 3];

		dm_icm_hue_segment_2_reg.h_pillar_5 = tICM_ini[baseAddr_hue_pillar + 4];
		dm_icm_hue_segment_2_reg.h_pillar_6 = tICM_ini[baseAddr_hue_pillar + 5];
		dm_icm_hue_segment_3_reg.h_pillar_7 = tICM_ini[baseAddr_hue_pillar + 6];
		dm_icm_hue_segment_3_reg.h_pillar_8 = tICM_ini[baseAddr_hue_pillar + 7];

		dm_icm_hue_segment_4_reg.h_pillar_9  = tICM_ini[baseAddr_hue_pillar + 8];
		dm_icm_hue_segment_4_reg.h_pillar_10 = tICM_ini[baseAddr_hue_pillar + 9];
		dm_icm_hue_segment_5_reg.h_pillar_11 = tICM_ini[baseAddr_hue_pillar + 10];
		dm_icm_hue_segment_5_reg.h_pillar_12 = tICM_ini[baseAddr_hue_pillar + 11];

		dm_icm_hue_segment_6_reg.h_pillar_13 = tICM_ini[baseAddr_hue_pillar + 12];
		dm_icm_hue_segment_6_reg.h_pillar_14 = tICM_ini[baseAddr_hue_pillar + 13];
		dm_icm_hue_segment_7_reg.h_pillar_15 = tICM_ini[baseAddr_hue_pillar + 14];
		dm_icm_hue_segment_7_reg.h_pillar_16 = tICM_ini[baseAddr_hue_pillar + 15];

		dm_icm_hue_segment_8_reg.h_pillar_17 = tICM_ini[baseAddr_hue_pillar + 16];
		dm_icm_hue_segment_8_reg.h_pillar_18 = tICM_ini[baseAddr_hue_pillar + 17];
		dm_icm_hue_segment_9_reg.h_pillar_19 = tICM_ini[baseAddr_hue_pillar + 18];
		dm_icm_hue_segment_9_reg.h_pillar_20 = tICM_ini[baseAddr_hue_pillar + 19];

		dm_icm_hue_segment_10_reg.h_pillar_21 = tICM_ini[baseAddr_hue_pillar + 20];
		dm_icm_hue_segment_10_reg.h_pillar_22 = tICM_ini[baseAddr_hue_pillar + 21];
		dm_icm_hue_segment_11_reg.h_pillar_23 = tICM_ini[baseAddr_hue_pillar + 22];
		dm_icm_hue_segment_11_reg.h_pillar_24 = tICM_ini[baseAddr_hue_pillar + 23];

		dm_icm_hue_segment_12_reg.h_pillar_25 = tICM_ini[baseAddr_hue_pillar + 24];
		dm_icm_hue_segment_12_reg.h_pillar_26 = tICM_ini[baseAddr_hue_pillar + 25];
		dm_icm_hue_segment_13_reg.h_pillar_27 = tICM_ini[baseAddr_hue_pillar + 26];
		dm_icm_hue_segment_13_reg.h_pillar_28 = tICM_ini[baseAddr_hue_pillar + 27];

		dm_icm_hue_segment_14_reg.h_pillar_29 = tICM_ini[baseAddr_hue_pillar + 28];
		dm_icm_hue_segment_14_reg.h_pillar_30 = tICM_ini[baseAddr_hue_pillar + 29];
		dm_icm_hue_segment_15_reg.h_pillar_31 = tICM_ini[baseAddr_hue_pillar + 30];
		dm_icm_hue_segment_15_reg.h_pillar_32 = tICM_ini[baseAddr_hue_pillar + 31];

		dm_icm_hue_segment_16_reg.h_pillar_33 = tICM_ini[baseAddr_hue_pillar + 32];
		dm_icm_hue_segment_16_reg.h_pillar_34 = tICM_ini[baseAddr_hue_pillar + 33];
		dm_icm_hue_segment_17_reg.h_pillar_35 = tICM_ini[baseAddr_hue_pillar + 34];
		dm_icm_hue_segment_17_reg.h_pillar_36 = tICM_ini[baseAddr_hue_pillar + 35];

		dm_icm_hue_segment_18_reg.h_pillar_37 = tICM_ini[baseAddr_hue_pillar + 36];
		dm_icm_hue_segment_18_reg.h_pillar_38 = tICM_ini[baseAddr_hue_pillar + 37];
		dm_icm_hue_segment_19_reg.h_pillar_39 = tICM_ini[baseAddr_hue_pillar + 38];
		dm_icm_hue_segment_19_reg.h_pillar_40 = tICM_ini[baseAddr_hue_pillar + 39];

		dm_icm_hue_segment_20_reg.h_pillar_41 = tICM_ini[baseAddr_hue_pillar + 40];
		dm_icm_hue_segment_20_reg.h_pillar_42 = tICM_ini[baseAddr_hue_pillar + 41];
		dm_icm_hue_segment_21_reg.h_pillar_43 = tICM_ini[baseAddr_hue_pillar + 42];
		dm_icm_hue_segment_21_reg.h_pillar_44 = tICM_ini[baseAddr_hue_pillar + 43];

		dm_icm_hue_segment_22_reg.h_pillar_45 = tICM_ini[baseAddr_hue_pillar + 44];
		dm_icm_hue_segment_22_reg.h_pillar_46 = tICM_ini[baseAddr_hue_pillar + 45];
		dm_icm_hue_segment_23_reg.h_pillar_47 = tICM_ini[baseAddr_hue_pillar + 46];
		dm_icm_hue_segment_23_reg.h_pillar_48 = tICM_ini[baseAddr_hue_pillar + 47];

		dm_icm_hue_segment_24_reg.h_pillar_49 = tICM_ini[baseAddr_hue_pillar + 48];
		dm_icm_hue_segment_24_reg.h_pillar_50 = tICM_ini[baseAddr_hue_pillar + 49];
		dm_icm_hue_segment_25_reg.h_pillar_51 = tICM_ini[baseAddr_hue_pillar + 50];
		dm_icm_hue_segment_25_reg.h_pillar_52 = tICM_ini[baseAddr_hue_pillar + 51];

		dm_icm_hue_segment_26_reg.h_pillar_53 = tICM_ini[baseAddr_hue_pillar + 52];
		dm_icm_hue_segment_26_reg.h_pillar_54 = tICM_ini[baseAddr_hue_pillar + 53];
		dm_icm_hue_segment_27_reg.h_pillar_55 = tICM_ini[baseAddr_hue_pillar + 54];
		dm_icm_hue_segment_27_reg.h_pillar_56 = tICM_ini[baseAddr_hue_pillar + 55];

		dm_icm_hue_segment_28_reg.h_pillar_57 = tICM_ini[baseAddr_hue_pillar + 56];
		dm_icm_hue_segment_28_reg.h_pillar_58 = tICM_ini[baseAddr_hue_pillar + 57];
		dm_icm_hue_segment_29_reg.h_pillar_59 = tICM_ini[baseAddr_hue_pillar + 58];

		/*sat pillar*/
		dm_icm_sat_segment_0_reg.s_pillar_1 = tICM_ini[baseAddr_global + 34];
		dm_icm_sat_segment_0_reg.s_pillar_2 = tICM_ini[baseAddr_global + 35];
		dm_icm_sat_segment_1_reg.s_pillar_3 = tICM_ini[baseAddr_global + 36];
		dm_icm_sat_segment_1_reg.s_pillar_4 = tICM_ini[baseAddr_global + 37];
		dm_icm_sat_segment_2_reg.s_pillar_5 = tICM_ini[baseAddr_global + 38];
		dm_icm_sat_segment_2_reg.s_pillar_6 = tICM_ini[baseAddr_global + 39];
		dm_icm_sat_segment_3_reg.s_pillar_7 = tICM_ini[baseAddr_global + 40];
		dm_icm_sat_segment_3_reg.s_pillar_8 = tICM_ini[baseAddr_global + 41];
		dm_icm_sat_segment_4_reg.s_pillar_9 = tICM_ini[baseAddr_global + 42];
		dm_icm_sat_segment_4_reg.s_pillar_10 = tICM_ini[baseAddr_global + 43];
		/*itn pillar*/
		dm_icm_int_segment_0_reg.i_pillar_1 = tICM_ini[baseAddr_global + 44];
		dm_icm_int_segment_0_reg.i_pillar_2 = tICM_ini[baseAddr_global + 45];
		dm_icm_int_segment_1_reg.i_pillar_3 = tICM_ini[baseAddr_global + 46];
		dm_icm_int_segment_1_reg.i_pillar_4 = tICM_ini[baseAddr_global + 47];
		dm_icm_int_segment_2_reg.i_pillar_5 = tICM_ini[baseAddr_global + 48];
		dm_icm_int_segment_2_reg.i_pillar_6 = tICM_ini[baseAddr_global + 49];

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
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_12_reg, dm_icm_hue_segment_12_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_13_reg, dm_icm_hue_segment_13_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_14_reg, dm_icm_hue_segment_14_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_15_reg, dm_icm_hue_segment_15_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_16_reg, dm_icm_hue_segment_16_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_17_reg, dm_icm_hue_segment_17_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_18_reg, dm_icm_hue_segment_18_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_19_reg, dm_icm_hue_segment_19_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_20_reg, dm_icm_hue_segment_20_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_21_reg, dm_icm_hue_segment_21_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_22_reg, dm_icm_hue_segment_22_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_23_reg, dm_icm_hue_segment_23_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_24_reg, dm_icm_hue_segment_24_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_25_reg, dm_icm_hue_segment_25_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_26_reg, dm_icm_hue_segment_26_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_27_reg, dm_icm_hue_segment_27_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_28_reg, dm_icm_hue_segment_28_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_29_reg, dm_icm_hue_segment_29_reg.regValue);

		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg, dm_icm_sat_segment_0_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_1_reg, dm_icm_sat_segment_1_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_2_reg, dm_icm_sat_segment_2_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_3_reg, dm_icm_sat_segment_3_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_4_reg, dm_icm_sat_segment_4_reg.regValue);


		IoReg_Write32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg, dm_icm_int_segment_0_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_INT_SEGMENT_1_reg, dm_icm_int_segment_1_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_INT_SEGMENT_2_reg, dm_icm_int_segment_2_reg.regValue);

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

		h_sta = 0;
		h_end = HUESEGMAX - 1;
		si_sta = 0;
		si_end = SATSEGMAX - 1;
		//if_write_ic = 1;
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

unsigned char drvif_color_set_icm_Enable(unsigned char enable_Flag)
{
	color_d_vc_global_ctrl_RBUS				d_vc_global_ctrl_reg;
	d_vc_global_ctrl_reg.regValue 	= IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_reg.m_icm_en		= enable_Flag;
	d_vc_global_ctrl_reg.write_enable_3 =1;
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);

	return true;
}

unsigned char drvif_color_icm_set_ctrl(unsigned short table_mode, unsigned short hsi_grid_sel, unsigned short out_grid_process, unsigned short y_ctrl)
{
	color_icm_dm_icm_ctrl_RBUS				  dm_icm_ctrl_reg;
	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	dm_icm_ctrl_reg.table_mode			= table_mode;
	dm_icm_ctrl_reg.hsi_grid_sel		= hsi_grid_sel;
	dm_icm_ctrl_reg.out_grid_process	= out_grid_process;
	dm_icm_ctrl_reg.y_ctrl				= y_ctrl;

	IoReg_Write32(COLOR_ICM_DM_ICM_CTRL_reg, dm_icm_ctrl_reg.regValue);

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
	int i;
	/* 3. update current global hue/sat/itn*/
	/*g_dhue*/
	g_dhue = g_dhue_cur;
	/*g_dsatbysat*/
	for(i = 0; i < SATSEGMAX; i++)
	{
		g_dsatbysat[i]	= g_dsatbysat_cur[i];
	}
	/*g_ditnbyitn*/
	for(i = 0; i < ITNSEGMAX; i++)
	{
		g_ditnbyitn[i]	= g_ditnbyitn_cur[i];
	}

	return true;
}

unsigned char drvif_color_icm_save_global_of_viptable(unsigned short dhue_cur, unsigned short *dsatbysat_cur, unsigned short *ditnbyitn_cur)
{
	int i;
	/*record vip table value*/
	/*g_dhue*/
	g_dhue_tab			= dhue_cur;
	g_dhue_tab			= ((ICM_GLOBAL_BIT_MAP *)&g_dhue_tab)->value;

		/*g_dsatbysat*/
	for (i = 0; i < SATSEGMAX; i++)
	{
		g_dsatbysat_tab[i]	= dsatbysat_cur[i];
		g_dsatbysat_tab[i] = ((ICM_GLOBAL_BIT_MAP *)(g_dsatbysat_tab+i))->value;
	}

		/*g_ditnbyitn*/
	for (i = 0; i < ITNSEGMAX; i++)
	{
		g_ditnbyitn_tab[i]	= ditnbyitn_cur[i];
		g_ditnbyitn_tab[i] = ((ICM_GLOBAL_BIT_MAP *)(g_ditnbyitn_tab+i))->value;
	}

	return true;
}

unsigned char drvif_color_icm_set_pillar(unsigned short *h_pillar, unsigned short *s_pillar, unsigned short *i_pillar, unsigned char h_pillar_num, unsigned char s_pillar_num, unsigned char i_pillar_num)
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
	color_icm_dm_icm_hue_segment_12_RBUS	dm_icm_hue_segment_12_reg;
	color_icm_dm_icm_hue_segment_13_RBUS	dm_icm_hue_segment_13_reg;
	color_icm_dm_icm_hue_segment_14_RBUS	dm_icm_hue_segment_14_reg;
	color_icm_dm_icm_hue_segment_15_RBUS	dm_icm_hue_segment_15_reg;
	color_icm_dm_icm_hue_segment_16_RBUS	dm_icm_hue_segment_16_reg;
	color_icm_dm_icm_hue_segment_17_RBUS	dm_icm_hue_segment_17_reg;
	color_icm_dm_icm_hue_segment_18_RBUS	dm_icm_hue_segment_18_reg;
	color_icm_dm_icm_hue_segment_19_RBUS	dm_icm_hue_segment_19_reg;
	color_icm_dm_icm_hue_segment_20_RBUS	dm_icm_hue_segment_20_reg;
	color_icm_dm_icm_hue_segment_21_RBUS	dm_icm_hue_segment_21_reg;
	color_icm_dm_icm_hue_segment_22_RBUS	dm_icm_hue_segment_22_reg;
	color_icm_dm_icm_hue_segment_23_RBUS	dm_icm_hue_segment_23_reg;
	color_icm_dm_icm_hue_segment_24_RBUS	dm_icm_hue_segment_24_reg;
	color_icm_dm_icm_hue_segment_25_RBUS	dm_icm_hue_segment_25_reg;
	color_icm_dm_icm_hue_segment_26_RBUS	dm_icm_hue_segment_26_reg;
	color_icm_dm_icm_hue_segment_27_RBUS	dm_icm_hue_segment_27_reg;
	color_icm_dm_icm_hue_segment_28_RBUS	dm_icm_hue_segment_28_reg;
	color_icm_dm_icm_hue_segment_29_RBUS	dm_icm_hue_segment_29_reg;

	color_icm_dm_icm_sat_segment_0_RBUS	dm_icm_sat_segment_0_reg;
	color_icm_dm_icm_sat_segment_1_RBUS	dm_icm_sat_segment_1_reg;
	color_icm_dm_icm_sat_segment_2_RBUS	dm_icm_sat_segment_2_reg;
	color_icm_dm_icm_sat_segment_3_RBUS	dm_icm_sat_segment_3_reg;
	color_icm_dm_icm_sat_segment_4_RBUS	dm_icm_sat_segment_4_reg;

	color_icm_dm_icm_int_segment_0_RBUS	dm_icm_int_segment_0_reg;
	color_icm_dm_icm_int_segment_1_RBUS	dm_icm_int_segment_1_reg;
	color_icm_dm_icm_int_segment_2_RBUS	dm_icm_int_segment_2_reg;


	color_icm_dm_icm_pillar_num_ctrl_RBUS		color_icm_dm_icm_pillar_num_ctrl_reg;
	color_icm_dm_icm_pillar_num_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_PILLAR_NUM_CTRL_reg);

    	color_icm_dm_icm_pillar_num_ctrl_reg.h_pillar_num = h_pillar_num;
    	color_icm_dm_icm_pillar_num_ctrl_reg.s_pillar_num = s_pillar_num;
    	color_icm_dm_icm_pillar_num_ctrl_reg.i_pillar_num = i_pillar_num;

	IoReg_Write32(COLOR_ICM_DM_ICM_PILLAR_NUM_CTRL_reg, color_icm_dm_icm_pillar_num_ctrl_reg.regValue);

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
		dm_icm_hue_segment_1_reg.h_pillar_3 = h_pillar[2];
		dm_icm_hue_segment_1_reg.h_pillar_4 = h_pillar[3];
		dm_icm_hue_segment_2_reg.h_pillar_5 = h_pillar[4];
		dm_icm_hue_segment_2_reg.h_pillar_6 = h_pillar[5];
		dm_icm_hue_segment_3_reg.h_pillar_7 = h_pillar[6];
		dm_icm_hue_segment_3_reg.h_pillar_8 = h_pillar[7];
		dm_icm_hue_segment_4_reg.h_pillar_9  = h_pillar[8];
		dm_icm_hue_segment_4_reg.h_pillar_10 = h_pillar[9];
		dm_icm_hue_segment_5_reg.h_pillar_11 = h_pillar[10];
		dm_icm_hue_segment_5_reg.h_pillar_12 = h_pillar[11];
		dm_icm_hue_segment_6_reg.h_pillar_13 = h_pillar[12];
		dm_icm_hue_segment_6_reg.h_pillar_14 = h_pillar[13];
		dm_icm_hue_segment_7_reg.h_pillar_15 = h_pillar[14];
		dm_icm_hue_segment_7_reg.h_pillar_16 = h_pillar[15];
		dm_icm_hue_segment_8_reg.h_pillar_17 = h_pillar[16];
		dm_icm_hue_segment_8_reg.h_pillar_18 = h_pillar[17];
		dm_icm_hue_segment_9_reg.h_pillar_19 = h_pillar[18];
		dm_icm_hue_segment_9_reg.h_pillar_20 = h_pillar[19];
		dm_icm_hue_segment_10_reg.h_pillar_21 = h_pillar[20];
		dm_icm_hue_segment_10_reg.h_pillar_22 = h_pillar[21];
		dm_icm_hue_segment_11_reg.h_pillar_23 = h_pillar[22];
		dm_icm_hue_segment_11_reg.h_pillar_24 = h_pillar[23];
		dm_icm_hue_segment_12_reg.h_pillar_25 = h_pillar[24];
		dm_icm_hue_segment_12_reg.h_pillar_26 = h_pillar[25];
		dm_icm_hue_segment_13_reg.h_pillar_27 = h_pillar[26];
		dm_icm_hue_segment_13_reg.h_pillar_28 = h_pillar[27];
		dm_icm_hue_segment_14_reg.h_pillar_29 = h_pillar[28];
		dm_icm_hue_segment_14_reg.h_pillar_30 = h_pillar[29];
		dm_icm_hue_segment_15_reg.h_pillar_31 = h_pillar[30];
		dm_icm_hue_segment_15_reg.h_pillar_32 = h_pillar[31];
		dm_icm_hue_segment_16_reg.h_pillar_33 = h_pillar[32];
		dm_icm_hue_segment_16_reg.h_pillar_34 = h_pillar[33];
		dm_icm_hue_segment_17_reg.h_pillar_35 = h_pillar[34];
		dm_icm_hue_segment_17_reg.h_pillar_36 = h_pillar[35];
		dm_icm_hue_segment_18_reg.h_pillar_37 = h_pillar[36];
		dm_icm_hue_segment_18_reg.h_pillar_38 = h_pillar[37];
		dm_icm_hue_segment_19_reg.h_pillar_39 = h_pillar[38];
		dm_icm_hue_segment_19_reg.h_pillar_40 = h_pillar[39];
		dm_icm_hue_segment_20_reg.h_pillar_41 = h_pillar[40];
		dm_icm_hue_segment_20_reg.h_pillar_42 = h_pillar[41];
		dm_icm_hue_segment_21_reg.h_pillar_43 = h_pillar[42];
		dm_icm_hue_segment_21_reg.h_pillar_44 = h_pillar[43];
		dm_icm_hue_segment_22_reg.h_pillar_45 = h_pillar[44];
		dm_icm_hue_segment_22_reg.h_pillar_46 = h_pillar[45];
		dm_icm_hue_segment_23_reg.h_pillar_47 = h_pillar[46];
		dm_icm_hue_segment_23_reg.h_pillar_48 = h_pillar[47];
		dm_icm_hue_segment_24_reg.h_pillar_49 = h_pillar[48];
		dm_icm_hue_segment_24_reg.h_pillar_50 = h_pillar[49];
		dm_icm_hue_segment_25_reg.h_pillar_51 = h_pillar[50];
		dm_icm_hue_segment_25_reg.h_pillar_52 = h_pillar[51];
		dm_icm_hue_segment_26_reg.h_pillar_53 = h_pillar[52];
		dm_icm_hue_segment_26_reg.h_pillar_54 = h_pillar[53];
		dm_icm_hue_segment_27_reg.h_pillar_55 = h_pillar[54];
		dm_icm_hue_segment_27_reg.h_pillar_56 = h_pillar[55];
		dm_icm_hue_segment_28_reg.h_pillar_57 = h_pillar[56];
		dm_icm_hue_segment_28_reg.h_pillar_58 = h_pillar[57];
		dm_icm_hue_segment_29_reg.h_pillar_59 = h_pillar[58];

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
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_12_reg, dm_icm_hue_segment_12_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_13_reg, dm_icm_hue_segment_13_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_14_reg, dm_icm_hue_segment_14_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_15_reg, dm_icm_hue_segment_15_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_16_reg, dm_icm_hue_segment_16_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_17_reg, dm_icm_hue_segment_17_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_18_reg, dm_icm_hue_segment_18_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_19_reg, dm_icm_hue_segment_19_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_20_reg, dm_icm_hue_segment_20_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_21_reg, dm_icm_hue_segment_21_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_22_reg, dm_icm_hue_segment_22_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_23_reg, dm_icm_hue_segment_23_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_24_reg, dm_icm_hue_segment_24_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_25_reg, dm_icm_hue_segment_25_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_26_reg, dm_icm_hue_segment_26_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_27_reg, dm_icm_hue_segment_27_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_28_reg, dm_icm_hue_segment_28_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_HUE_SEGMENT_29_reg, dm_icm_hue_segment_29_reg.regValue);

	}

	/*sat pillar*/
	if (NULL != s_pillar) {
		dm_icm_sat_segment_0_reg.s_pillar_1 = s_pillar[0];
		dm_icm_sat_segment_0_reg.s_pillar_2 = s_pillar[1];
		dm_icm_sat_segment_1_reg.s_pillar_3 = s_pillar[2];
		dm_icm_sat_segment_1_reg.s_pillar_4 = s_pillar[3];
		dm_icm_sat_segment_2_reg.s_pillar_5 = s_pillar[4];
		dm_icm_sat_segment_2_reg.s_pillar_6 = s_pillar[5];
		dm_icm_sat_segment_3_reg.s_pillar_7 = s_pillar[6];
		dm_icm_sat_segment_3_reg.s_pillar_8 = s_pillar[7];
		dm_icm_sat_segment_4_reg.s_pillar_9 = s_pillar[8];
		dm_icm_sat_segment_4_reg.s_pillar_10 = s_pillar[9];

		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg, dm_icm_sat_segment_0_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_1_reg, dm_icm_sat_segment_1_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_2_reg, dm_icm_sat_segment_2_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_3_reg, dm_icm_sat_segment_3_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_SAT_SEGMENT_4_reg, dm_icm_sat_segment_4_reg.regValue);
	}

	/*itn pillar*/
	if (NULL != i_pillar) {
		dm_icm_int_segment_0_reg.i_pillar_1 = i_pillar[0];
		dm_icm_int_segment_0_reg.i_pillar_2 = i_pillar[1];
		dm_icm_int_segment_1_reg.i_pillar_3 = i_pillar[2];
		dm_icm_int_segment_1_reg.i_pillar_4 = i_pillar[3];
		dm_icm_int_segment_2_reg.i_pillar_5 = i_pillar[4];
		dm_icm_int_segment_2_reg.i_pillar_6 = i_pillar[5];

		IoReg_Write32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg, dm_icm_int_segment_0_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_INT_SEGMENT_1_reg, dm_icm_int_segment_1_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_INT_SEGMENT_2_reg, dm_icm_int_segment_2_reg.regValue);

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
	int i;

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

	color_icm_dm_icm_3d_tab_read_hsi_00_RBUS      dm_icm_3d_tab_read_hsi_00_reg;
	color_icm_dm_icm_3d_tab_read_hsi_01_RBUS      dm_icm_3d_tab_read_hsi_01_reg;
	color_icm_dm_icm_3d_tab_read_hsi_10_RBUS      dm_icm_3d_tab_read_hsi_10_reg;
	color_icm_dm_icm_3d_tab_read_hsi_11_RBUS      dm_icm_3d_tab_read_hsi_11_reg;
	color_icm_dm_icm_3d_tab_read_hsi_20_RBUS      dm_icm_3d_tab_read_hsi_20_reg;
	color_icm_dm_icm_3d_tab_read_hsi_21_RBUS      dm_icm_3d_tab_read_hsi_21_reg;
	color_icm_dm_icm_3d_tab_read_hsi_30_RBUS      dm_icm_3d_tab_read_hsi_30_reg;
	color_icm_dm_icm_3d_tab_read_hsi_31_RBUS      dm_icm_3d_tab_read_hsi_31_reg;

	/*enable icm*/
	{
	color_d_vc_global_ctrl_RBUS				d_vc_global_ctrl_reg;
	d_vc_global_ctrl_reg.regValue 	= IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_reg.m_vc_en		= 1;
	d_vc_global_ctrl_reg.write_enable_11 =1;
	d_vc_global_ctrl_reg.m_icm_en		= 1;
	d_vc_global_ctrl_reg.write_enable_3 =1;
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
	}

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

				ICM_TAB[k * 4 + 0][si][h].H = dm_icm_3d_tab_read_hsi_00_reg.read_table_h0;
				ICM_TAB[k * 4 + 0][si][h].S = dm_icm_3d_tab_read_hsi_00_reg.read_table_s0;
				ICM_TAB[k * 4 + 0][si][h].I = dm_icm_3d_tab_read_hsi_01_reg.read_table_i0;
				ICM_TAB[k * 4 + 1][si][h].H = dm_icm_3d_tab_read_hsi_10_reg.read_table_h1;
				ICM_TAB[k * 4 + 1][si][h].S = dm_icm_3d_tab_read_hsi_10_reg.read_table_s1;
				ICM_TAB[k * 4 + 1][si][h].I = dm_icm_3d_tab_read_hsi_11_reg.read_table_i1;
				ICM_TAB[k * 4 + 2][si][h].H = dm_icm_3d_tab_read_hsi_20_reg.read_table_h2;
				ICM_TAB[k * 4 + 2][si][h].S = dm_icm_3d_tab_read_hsi_20_reg.read_table_s2;
				ICM_TAB[k * 4 + 2][si][h].I = dm_icm_3d_tab_read_hsi_21_reg.read_table_i2;
				ICM_TAB[k * 4 + 3][si][h].H = dm_icm_3d_tab_read_hsi_30_reg.read_table_h3;
				ICM_TAB[k * 4 + 3][si][h].S = dm_icm_3d_tab_read_hsi_30_reg.read_table_s3;
				ICM_TAB[k * 4 + 3][si][h].I = dm_icm_3d_tab_read_hsi_31_reg.read_table_i3;

				/* (1)subtract previous global hue/sat/itn*/
				if (bSubtractPrevious)
				{
					for(i = 0; i < 4; i++)
					{
						ICM_TAB[k * 4 + i][si][h].H -= g_dhue;
						ICM_TAB[k * 4 + i][si][h].S -= g_dsatbysat[si];
						ICM_TAB[k * 4 + i][si][h].I -= g_ditnbyitn[k * 4 + i];
					}
				}

				/* (2)add current global hue/sat/itn*/
				for(i = 0; i < 4; i++)
				{
					ICM_TAB[k * 4 + i][si][h].H = (ICM_TAB[k * 4 + i][si][h].H + g_dhue_cur + HUERANGE) % HUERANGE;
					ICM_TAB[k * 4 + i][si][h].S = CLIP(0, SATRANGEEX, ICM_TAB[k * 4 + i][si][h].S + g_dsatbysat_cur[si]);
					ICM_TAB[k * 4 + i][si][h].I = CLIP(0, ITNRANGEEX, ICM_TAB[k * 4 + i][si][h].I + g_ditnbyitn_cur[k * 4 + i]);
				}

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
	for(i = 0; i < SATSEGMAX; i++)
	{
		g_dsatbysat[i]	= g_dsatbysat_cur[i];
	}
	/*g_ditnbyitn*/
	for(i = 0; i < ITNSEGMAX; i++)
	{
		g_ditnbyitn[i]	= g_ditnbyitn_cur[i];
	}

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
	color_d_vc_global_ctrl_RBUS			d_vc_global_ctrl_REG;
	d_vc_global_ctrl_REG.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_REG.m_icm_en = bEable;
	d_vc_global_ctrl_REG.write_enable_3 =1;

	if (bEable && !d_vc_global_ctrl_REG.m_vc_en){
		d_vc_global_ctrl_REG.m_vc_en = 1;
		d_vc_global_ctrl_REG.write_enable_11 =1;

	}

	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, 	d_vc_global_ctrl_REG.regValue);
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

				ICM_TAB_ACCESS[k * 4 + 0][si][h].H = dm_icm_3d_tab_read_hsi_00_reg.read_table_h0;
				ICM_TAB_ACCESS[k * 4 + 0][si][h].S = dm_icm_3d_tab_read_hsi_00_reg.read_table_s0;
				ICM_TAB_ACCESS[k * 4 + 0][si][h].I = dm_icm_3d_tab_read_hsi_01_reg.read_table_i0;
				ICM_TAB_ACCESS[k * 4 + 1][si][h].H = dm_icm_3d_tab_read_hsi_10_reg.read_table_h1;
				ICM_TAB_ACCESS[k * 4 + 1][si][h].S = dm_icm_3d_tab_read_hsi_10_reg.read_table_s1;
				ICM_TAB_ACCESS[k * 4 + 1][si][h].I = dm_icm_3d_tab_read_hsi_11_reg.read_table_i1;
				ICM_TAB_ACCESS[k * 4 + 2][si][h].H = dm_icm_3d_tab_read_hsi_20_reg.read_table_h2;
				ICM_TAB_ACCESS[k * 4 + 2][si][h].S = dm_icm_3d_tab_read_hsi_20_reg.read_table_s2;
				ICM_TAB_ACCESS[k * 4 + 2][si][h].I = dm_icm_3d_tab_read_hsi_21_reg.read_table_i2;
				ICM_TAB_ACCESS[k * 4 + 3][si][h].H = dm_icm_3d_tab_read_hsi_30_reg.read_table_h3;
				ICM_TAB_ACCESS[k * 4 + 3][si][h].S = dm_icm_3d_tab_read_hsi_30_reg.read_table_s3;
				ICM_TAB_ACCESS[k * 4 + 3][si][h].I = dm_icm_3d_tab_read_hsi_31_reg.read_table_i3;

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

	//constrain: if h segment num is odd, e.x. 2n+1(h pillar num=2n+2), need to copy h_idx=2n layer to h_idx=2n+1 layer, for hw need even h layer sram table
	int h_pillar_num, s_pillar_num, i_pillar_num, ii, is;
	color_icm_dm_icm_pillar_num_ctrl_RBUS		color_icm_dm_icm_pillar_num_ctrl_reg;
	color_icm_dm_icm_pillar_num_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_PILLAR_NUM_CTRL_reg);

    	h_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.h_pillar_num;
    	s_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.s_pillar_num;
    	i_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.i_pillar_num;

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

	/*enable icm*/
	{
	color_d_vc_global_ctrl_RBUS				d_vc_global_ctrl_reg;
	d_vc_global_ctrl_reg.regValue 	= IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_reg.m_vc_en		= 1;
	d_vc_global_ctrl_reg.write_enable_11 = 1;
	d_vc_global_ctrl_reg.m_icm_en		= 1;
	d_vc_global_ctrl_reg.write_enable_3 =1;
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
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

unsigned char drvif_color_icm_Write3DTable_By_DMA(COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX], unsigned int addr, unsigned int* vir_addr,  unsigned char h_pillar_num, unsigned char s_pillar_num, unsigned char i_pillar_num)
{
	color_d_vc_global_ctrl_RBUS 		d_vc_global_ctrl_REG;

	//color_icm_dm_icm_ctrl_RBUS		color_icm_dm_icm_ctrl_reg;
	//color_icm_dm_icm_db_ctrl_0_RBUS color_icm_dm_icm_db_ctrl_0_reg;
	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS color_icm_dm_icm_3d_tab_sram_ctrl_reg;
	color_icm_dm_icm_dmato3dlut_ctrl_RBUS color_icm_dm_icm_dmato3dlut_ctrl_reg;

	dmato3dlut_dmato3dlut_table0_format0_RBUS dmato3dlut_dmato3dlut_table0_format0_reg;
	//dmato3dlut_dmato3dlut_table1_format0_RBUS dmato3dlut_dmato3dlut_table1_format0_reg;
	//dmato3dlut_dmato3dlut_table0_addr_RBUS dmato3dlut_dmato3dlut_table0_addr_reg;
	//dmato3dlut_dmato3dlut_table1_addr_RBUS dmato3dlut_dmato3dlut_table1_addr_reg;
	dmato3dlut_dmato3dlut_table0_errstatus2irq_RBUS dmato3dlut_dmato3dlut_table0_errstatus2irq_reg;
	dmato3dlut_dmato3dlut_table1_errstatus2irq_RBUS dmato3dlut_dmato3dlut_table1_errstatus2irq_reg;
	dmato3dlut_dmato3dlut_table0_status_RBUS dmato3dlut_dmato3dlut_table0_status_reg;
	dmato3dlut_dmato3dlut_db_ctl_RBUS dmato3dlut_dmato3dlut_db_ctl_reg;

	UINT32 hue_bit = 13;
	UINT32 sat_bit = 13;
	UINT32 itn_bit = 13;
	UINT32 buf_bit = 32;
	UINT32 total_bit;
	UINT32 *buf = NULL;
	UINT32 buf_size;
	UINT32 buf_left_bit;
	UINT32 data;
	UINT32 ih, is, ii, mh, ms, mi, nh, ns, ni, idx;

	UINT32 *pVirtAddr=NULL;
	UINT32 i=0;
	UINT32 timeout_clear_err=0x32000;
	UINT8 buf_by_malloc=0;

	//constrain: if h segment num is odd, e.x. 2n+1(h pillar num=2n+2), need to copy h_idx=2n layer to h_idx=2n+1 layer, for hw need even h layer sram table
	//int h_pillar_num, s_pillar_num, i_pillar_num;
	//color_icm_dm_icm_pillar_num_ctrl_RBUS		color_icm_dm_icm_pillar_num_ctrl_reg;
	//color_icm_dm_icm_pillar_num_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_PILLAR_NUM_CTRL_reg);

	//h_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.h_pillar_num;
	//s_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.s_pillar_num;
	//i_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.i_pillar_num;

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
		return -1;
	}

	/* phy addr setting*/
	IoReg_Write32(DMATO3DLUT_DMAto3DLUT_Table0_Addr_reg, (addr/*-0x200*/));//Set ICM dma address

	// Calc DMA data format
	total_bit = (hue_bit + sat_bit + itn_bit) * (HUESEGMAX * SATSEGMAX * ITNSEGMAX);
	buf_size = (total_bit % buf_bit ) == 0 ? total_bit / buf_bit : total_bit / buf_bit + 1;

	buf = (unsigned int *)dvr_malloc(buf_size * sizeof(unsigned int));
//	buf = (unsigned int *)dvr_malloc_specific(buf_size * sizeof(unsigned int), GFP_DCU2_FIRST);
	if (buf == NULL) {
		#define ICM_TMPBUF_OFFSET 32*1024
		buf = (UINT32 *)((unsigned char *)vir_addr + ICM_TMPBUF_OFFSET);

		if ( (buf_size * sizeof(unsigned int)) > (VIP_DMAto3DTable_ICM_SIZE - ICM_TMPBUF_OFFSET) ) {
			buf = NULL;
		}
	} else {
		buf_by_malloc = 1;
	}

	if (buf == NULL)
	{
		rtd_pr_vpq_emerg("ICM :buf PQ warning ,  drvif_color_icm_Write3DTable_By_DMA size(%zx) memory allocate fail!\n", (buf_size * sizeof(unsigned int)));

		return false;
	}
	memset(buf, 0, buf_size * sizeof(unsigned int));

	buf_left_bit = 32;
	idx = 0;
	for(ni = 0; ni < ITNSEGMAX; ni += 2)
	{
		for(ns = 0; ns < SATSEGMAX; ns += 2)
		{
			for(nh = 0; nh < HUESEGMAX; nh += 2)
			{
				for(mi = 0; mi < 2; mi ++)
				{
					for(ms = 0; ms < 2; ms ++)
					{
					    for(mh = 0; mh < 2; mh ++)
					    {
							ii = ni + mi;
							is = ns + ms;
							ih = nh + mh;

							// hue
							data = ICM_TAB_ACCESS[ii][is][ih].H & GetMask(hue_bit);
							//data =0x789& GetMask(hue_bit);
							if(buf_left_bit >= hue_bit)
							{
							    buf[idx] = buf[idx] | (data << (buf_left_bit - hue_bit));
							    buf_left_bit -= hue_bit;
							}
							else
							{
							    buf[idx] = buf[idx] | (data >> (hue_bit - buf_left_bit));
							    idx += 1;
							    buf_left_bit = 32 - (hue_bit - buf_left_bit);
							    buf[idx] = buf[idx] | (data << buf_left_bit);
							}

							// sat
							data = ICM_TAB_ACCESS[ii][is][ih].S & GetMask(sat_bit);
							//data =0x1253& GetMask(sat_bit);
							if(buf_left_bit >= sat_bit)
							{
							    buf[idx] = buf[idx] | (data << (buf_left_bit - sat_bit));
							    buf_left_bit -= sat_bit;
							}
							else
							{
							    buf[idx] = buf[idx] | (data >> (sat_bit - buf_left_bit));
							    idx += 1;
							    buf_left_bit = 32 - (sat_bit - buf_left_bit);
							    buf[idx] = buf[idx] | (data << buf_left_bit);
							}

							// itn
							data = ICM_TAB_ACCESS[ii][is][ih].I & GetMask(itn_bit);
							//data = (ICM_TAB_ACCESS[ii][is][ih].I) & GetMask(itn_bit);

							//data =(0x1234) & GetMask(itn_bit);
							if(buf_left_bit >= itn_bit)
							{
							    buf[idx] = buf[idx] | (data << (buf_left_bit - itn_bit));
							    buf_left_bit -= itn_bit;
							}
							else
							{
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

	// set data to DMA
	//dmato3dlut_dmato3dlut_table0_addr_reg.regValue = IoReg_Read32(DMATO3DLUT_DMAto3DLUT_Table0_Addr_reg);
	//address = dmato3dlut_dmato3dlut_table0_addr_reg.regValue;
	//pVirtAddr = dvr_remap_uncached_memory(address, ICM_BUFFER_SIZE, __builtin_return_address(0));

	pVirtAddr=vir_addr;

	rtd_pr_vpq_debug("\n  ICM addr %x , virtaddr %p \n",addr,pVirtAddr );

	drvif_Wait_UZUDTG_start(10, -200);

	if (pVirtAddr != NULL) {
		unsigned char *temp;
		for(i = 0; i < buf_size; i++) {
			temp=(unsigned char *)&buf[i];
			pVirtAddr[i]=temp[3]+(temp[2]<<8)+(temp[1]<<16)+(temp[0]<<24);
		}
	} else
		rtd_pr_vpq_emerg("ICM :pVirtAddrPQ warning , pVirtAddr drvif_color_icm_Write3DTable_By_DMA map virt memory fail!\n");

	if (buf_by_malloc)
		dvr_free((void *)buf);

	// enable ICM
	d_vc_global_ctrl_REG.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_REG.m_icm_en = 1;
	d_vc_global_ctrl_REG.write_enable_3 = 1;
	d_vc_global_ctrl_REG.m_vc_en = 1;
	d_vc_global_ctrl_REG.write_enable_11 = 1;
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, 	d_vc_global_ctrl_REG.regValue);

	//set ICM DMA
	timeout_clear_err = 0x100;
	do {
		dmato3dlut_dmato3dlut_table0_status_reg.regValue = IoReg_Read32(DMATO3DLUT_DMAto3DLUT_Table0_Status_reg);
		dmato3dlut_dmato3dlut_table0_status_reg.table0_wdone = 1;
		IoReg_Write32(DMATO3DLUT_DMAto3DLUT_Table0_Status_reg,dmato3dlut_dmato3dlut_table0_status_reg.regValue);

		/* table 0 clear */
		dmato3dlut_dmato3dlut_table0_errstatus2irq_reg.regValue = IoReg_Read32(DMATO3DLUT_DMAto3DLUT_Table0_ErrStatus2Irq_reg);
		dmato3dlut_dmato3dlut_table0_errstatus2irq_reg.table0_err_status = 0x3f;
		IoReg_Write32(DMATO3DLUT_DMAto3DLUT_Table0_ErrStatus2Irq_reg,dmato3dlut_dmato3dlut_table0_errstatus2irq_reg.regValue);

		/* table 1 clear */
		dmato3dlut_dmato3dlut_table1_errstatus2irq_reg.regValue = IoReg_Read32(DMATO3DLUT_DMAto3DLUT_Table1_ErrStatus2Irq_reg);
		dmato3dlut_dmato3dlut_table1_errstatus2irq_reg.table1_err_status = 0x3f;
		IoReg_Write32(DMATO3DLUT_DMAto3DLUT_Table1_ErrStatus2Irq_reg,dmato3dlut_dmato3dlut_table1_errstatus2irq_reg.regValue);

		dmato3dlut_dmato3dlut_table0_errstatus2irq_reg.regValue = IoReg_Read32(DMATO3DLUT_DMAto3DLUT_Table0_ErrStatus2Irq_reg);
		dmato3dlut_dmato3dlut_table1_errstatus2irq_reg.regValue = IoReg_Read32(DMATO3DLUT_DMAto3DLUT_Table1_ErrStatus2Irq_reg);
	} while(((dmato3dlut_dmato3dlut_table0_errstatus2irq_reg.table0_err_status != 0x0) ||
		(dmato3dlut_dmato3dlut_table1_errstatus2irq_reg.table1_err_status != 0x0)) &&
		timeout_clear_err--);


	// Start ICM DMA flow
	//  =======================================================================

	// step3. db_en = 1, table0_dma_en = 1, table1_dma_en = 1 and others DMA register setting
	dmato3dlut_dmato3dlut_db_ctl_reg.regValue= IoReg_Read32(DMATO3DLUT_DMAto3DLUT_db_ctl_reg);
	dmato3dlut_dmato3dlut_db_ctl_reg.db_en = 1;
	IoReg_Write32(DMATO3DLUT_DMAto3DLUT_db_ctl_reg,dmato3dlut_dmato3dlut_db_ctl_reg.regValue);

	color_icm_dm_icm_dmato3dlut_ctrl_reg.regValue= IoReg_Read32(COLOR_ICM_DM_ICM_DMATO3DLUT_CTRL_reg);
	color_icm_dm_icm_dmato3dlut_ctrl_reg.dmato3dlut_en =1;
	IoReg_Write32(COLOR_ICM_DM_ICM_DMATO3DLUT_CTRL_reg,color_icm_dm_icm_dmato3dlut_ctrl_reg.regValue);

	color_icm_dm_icm_3d_tab_sram_ctrl_reg.regValue= IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg);
	color_icm_dm_icm_3d_tab_sram_ctrl_reg.hw_fw_priority_opt=1;
	IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_SRAM_CTRL_reg,color_icm_dm_icm_3d_tab_sram_ctrl_reg.regValue);

	dmato3dlut_dmato3dlut_table0_format0_reg.regValue = IoReg_Read32(DMATO3DLUT_DMAto3DLUT_Table0_Format0_reg);
	dmato3dlut_dmato3dlut_table0_format0_reg.table0_dma_en = 1;
	IoReg_Write32(DMATO3DLUT_DMAto3DLUT_Table0_Format0_reg,dmato3dlut_dmato3dlut_table0_format0_reg.regValue);

	dmato3dlut_dmato3dlut_db_ctl_reg.regValue= IoReg_Read32(DMATO3DLUT_DMAto3DLUT_db_ctl_reg);
	dmato3dlut_dmato3dlut_db_ctl_reg.wtable_apply =1;
	IoReg_Write32(DMATO3DLUT_DMAto3DLUT_db_ctl_reg,dmato3dlut_dmato3dlut_db_ctl_reg.regValue);

	//================================================================================================================

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
			d_vc_global_ctrl_REG.m_icm_en		= 0;
			d_vc_global_ctrl_REG.write_enable_3 =1;
			IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_REG.regValue);
			return;
		} else {
			d_vc_global_ctrl_REG.m_vc_en		= 1;
			d_vc_global_ctrl_REG.write_enable_11 =1;
			d_vc_global_ctrl_REG.m_icm_en		= 1;
			d_vc_global_ctrl_REG.write_enable_3 =1;
			IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_REG.regValue);
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
	//unsigned char if_write_ic;
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

	/*enable icm*/
	{
	color_d_vc_global_ctrl_RBUS				d_vc_global_ctrl_reg;
	d_vc_global_ctrl_reg.regValue 	= IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_reg.m_vc_en		= 1;
	d_vc_global_ctrl_reg.write_enable_11 =1;
	d_vc_global_ctrl_reg.m_icm_en		= 1;
	d_vc_global_ctrl_reg.write_enable_3 =1;
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
	}

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

		h_sta = 0;
		h_end = HUESEGMAX - 1;
		si_sta = 0;
		si_end = SATSEGMAX - 1;
		//if_write_ic = 1;
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

				/*write*/
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_00_reg, dm_icm_3d_tab_write_hsi_00_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_10_reg, dm_icm_3d_tab_write_hsi_10_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_20_reg, dm_icm_3d_tab_write_hsi_20_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_30_reg, dm_icm_3d_tab_write_hsi_30_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_40_reg, dm_icm_3d_tab_write_hsi_40_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_50_reg, dm_icm_3d_tab_write_hsi_50_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_60_reg, dm_icm_3d_tab_write_hsi_60_reg.regValue);
				IoReg_Write32(COLOR_ICM_DM_ICM_3D_TAB_WRITE_HSI_70_reg, dm_icm_3d_tab_write_hsi_70_reg.regValue);

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
	int hlen, slen/*, ilen*/, hmax;
	int hue_C = 0, Sat_C = 0, hue_LB = 0, hue_RB = 0;
	bool if_write_ic = 1;

	color_icm_dm_icm_ctrl_RBUS  dm_icm_ctrl_reg;
	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	//ilen = i_end - i_sta + 1;
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

unsigned char drvif_color_icm_Adjust_Block(unsigned char nAxis, int nOffset, int adjust_len, int h_sta, int h_end, unsigned char h_total, int s_sta, int s_end, unsigned char s_total, int i_sta, int i_end, unsigned char i_total)
{
	unsigned int *buf = NULL;
	unsigned char result;

	if(nOffset==0)
		return FALSE;

	VIPprintf("~ drvif_color_icm_Adjust_Block, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);

	buf = (unsigned int *)vip_malloc(((i_end - i_sta + 1) * (s_end - s_sta + 1) * (h_end - h_sta + 1) * 2) * sizeof(unsigned int));

	if (buf == NULL)
		return FALSE;

	/*read*/
	result = drvif_color_icm_SramBlockReadSpeedup(buf, h_sta,  h_end,  s_sta,  s_end,  i_sta,  i_end,nAxis, nOffset);

	/*Protection*/
	if (result) {
		switch(nAxis)
		{
			case _HUE_ELEM:
				result = drvif_color_icm_Adjust_Protection( buf, nOffset, adjust_len, h_sta,  h_end, h_total,  s_sta,  s_end, s_total,  i_sta,  i_end, i_total);
			break;
			case _SAT_ELEM:
			case _ITN_ELEM:
			 break;
			default:
			 break;
		}
	}

	/*write*/
	if (result)
		drvif_color_icm_SramBlockAccessSpeedup(buf,1,  h_sta,  h_end,  s_sta,  s_end,  i_sta,  i_end);
	vip_free((void *)buf);
	return result;
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
	int hlen, slen;//, ilen;
	unsigned char if_write_ic = 1;
	color_icm_dm_icm_3d_tab_write_hsi_00_RBUS     dm_icm_3d_tab_write_hsi_00_reg;
	color_icm_dm_icm_3d_tab_write_hsi_01_RBUS     dm_icm_3d_tab_write_hsi_01_reg;
	color_icm_dm_icm_ctrl_RBUS                dm_icm_ctrl_reg;
	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	//ilen = i_end - i_sta + 1;

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
	int hlen, slen;//, ilen;
	int hidx, hmax, h_pillar_num;

	color_icm_dm_icm_3d_tab_write_hsi_00_RBUS     dm_icm_3d_tab_write_hsi_00_reg;
	color_icm_dm_icm_3d_tab_write_hsi_01_RBUS     dm_icm_3d_tab_write_hsi_01_reg;
	color_icm_dm_icm_3d_tab_sram_ctrl_RBUS        dm_icm_3d_tab_sram_ctrl_reg;

	color_icm_dm_icm_pillar_num_ctrl_RBUS		color_icm_dm_icm_pillar_num_ctrl_reg;
	color_icm_dm_icm_pillar_num_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_PILLAR_NUM_CTRL_reg);
    	h_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.h_pillar_num;

	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	//ilen = i_end - i_sta + 1;

	/*enable icm*/
	{
	color_d_vc_global_ctrl_RBUS				d_vc_global_ctrl_reg;
	d_vc_global_ctrl_reg.regValue 	= IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_reg.m_vc_en		= 1;
	d_vc_global_ctrl_reg.write_enable_11 =1;
	d_vc_global_ctrl_reg.m_icm_en		= 1;
	d_vc_global_ctrl_reg.write_enable_3 =1;
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
	}

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
	int base;
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

	color_icm_dm_icm_3d_tab_write_hsi_00_RBUS dm_icm_3d_tab_write_hsi_00_reg;
	color_icm_dm_icm_3d_tab_write_hsi_01_RBUS dm_icm_3d_tab_write_hsi_01_reg;

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
		base = 6;
	} else {
		sramtype = 0; /* base on HS*/
		y_sta = s_sta;
		y_end = s_end;
		x_sta = i_sta;
		x_end = i_end;
		base = 4;
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

			if (x_end < base) {
				flag = 0x1;
				bound[1] = x_end + 1;
			} else if (x_sta > base - 1) {
				flag = 0x2;
				bound[1] = x_sta;
			} else {
				flag = 0x3;
				bound[1] = base;
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
						index = x % base;
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
	int hlen, slen;//, ilen;
	unsigned char if_write_ic = 1;

	color_icm_dm_icm_3d_tab_read_hsi_00_RBUS      dm_icm_3d_tab_read_hsi_00_reg;
	color_icm_dm_icm_3d_tab_read_hsi_01_RBUS      dm_icm_3d_tab_read_hsi_01_reg;
	color_icm_dm_icm_ctrl_RBUS                dm_icm_ctrl_reg;

	dm_icm_3d_tab_read_hsi_00_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_00_reg);
	dm_icm_3d_tab_read_hsi_01_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_3D_TAB_READ_HSI_01_reg);

	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);
	hlen = h_end - h_sta + 1;
	slen = s_end - s_sta + 1;
	//ilen = i_end - i_sta + 1;

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

	color_icm_dm_icm_hue_segment_23_RBUS      dm_icm_hue_segment_23_reg;
	dm_icm_hue_segment_23_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_11_reg);
	if (ih < 8 && hue > (HUERANGE / 2)) /* protect for first  bin */
		result = hue - HUERANGE;
	else if ((ih > dm_icm_hue_segment_23_reg.h_pillar_48 - 8) && (ih < dm_icm_hue_segment_23_reg.h_pillar_48)
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

unsigned char drvif_color_icm_Set_RGB_Offset_Enable(unsigned char enable_Flag)
{
	color_icm_dm_icm_rgb_offset_sram_ctrl_RBUS	color_icm_dm_icm_rgb_offset_sram_ctrl_reg;
	color_icm_dm_icm_rgb_offset_sram_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_CTRL_reg);
	color_icm_dm_icm_rgb_offset_sram_ctrl_reg.rgb_offset_en		= enable_Flag;
	IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_CTRL_reg, color_icm_dm_icm_rgb_offset_sram_ctrl_reg.regValue);

	return true;
}

unsigned char drvif_color_icm_Set_RGB_Offset_Table(int rgb_offset_curve[3][HUESEGMAX], unsigned char rgb_offset_s_gain[SATSEGMAX], unsigned char rgb_offset_i_gain[ITNSEGMAX])
{
	int timeout;
	int addr, i;

	color_icm_dm_icm_rgb_offset_sram_ctrl_RBUS		color_icm_dm_icm_rgb_offset_sram_ctrl_reg;
	color_icm_dm_icm_rgb_offset_sram_wr_data0_RBUS	color_icm_dm_icm_rgb_offset_sram_wr_data0_reg;
	color_icm_dm_icm_rgb_offset_sram_wr_data1_RBUS	color_icm_dm_icm_rgb_offset_sram_wr_data1_reg;
	color_icm_dm_icm_rgb_offset_sram_wr_data2_RBUS	color_icm_dm_icm_rgb_offset_sram_wr_data2_reg;
	color_icm_dm_icm_rgb_offset_sram_wr_data3_RBUS	color_icm_dm_icm_rgb_offset_sram_wr_data3_reg;
	color_icm_dm_icm_rgb_offset_s_ctrl0_RBUS		color_icm_dm_icm_rgb_offset_s_ctrl0_reg;
	color_icm_dm_icm_rgb_offset_s_ctrl1_RBUS		color_icm_dm_icm_rgb_offset_s_ctrl1_reg;
	color_icm_dm_icm_rgb_offset_s_ctrl2_RBUS		color_icm_dm_icm_rgb_offset_s_ctrl2_reg;
	color_icm_dm_icm_rgb_offset_i_ctrl0_RBUS		color_icm_dm_icm_rgb_offset_i_ctrl0_reg;
	color_icm_dm_icm_rgb_offset_i_ctrl1_RBUS		color_icm_dm_icm_rgb_offset_i_ctrl1_reg;
	//printk("[ICM]===============write rgb offset start!!! % d", rgb_offset_curve[0][60]);

	//constrain: if h segment num is odd, e.x. 2n+1(h pillar num=2n+2), need to copy h_idx=2n layer to h_idx=2n+1 layer, for hw need even h layer sram table
	int h_pillar_num;
	color_icm_dm_icm_pillar_num_ctrl_RBUS		color_icm_dm_icm_pillar_num_ctrl_reg;
	color_icm_dm_icm_pillar_num_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_PILLAR_NUM_CTRL_reg);
    	h_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.h_pillar_num;
	if(h_pillar_num % 2 == 0)
	{
		rgb_offset_curve[0][h_pillar_num - 1] = rgb_offset_curve[0][h_pillar_num - 2];
		rgb_offset_curve[1][h_pillar_num - 1] = rgb_offset_curve[1][h_pillar_num - 2];
		rgb_offset_curve[2][h_pillar_num - 1] = rgb_offset_curve[2][h_pillar_num - 2];
	}

	//rgb-offset curve
	for(addr = 0; addr < (HUESEGMAX / 2); addr++)
	{
		i = addr << 1;
		color_icm_dm_icm_rgb_offset_sram_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_CTRL_reg);
		color_icm_dm_icm_rgb_offset_sram_ctrl_reg.rgb_offset_addr = addr;
		IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_CTRL_reg, color_icm_dm_icm_rgb_offset_sram_ctrl_reg.regValue);

		color_icm_dm_icm_rgb_offset_sram_wr_data0_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_WR_DATA0_reg);
		color_icm_dm_icm_rgb_offset_sram_wr_data1_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_WR_DATA1_reg);
		color_icm_dm_icm_rgb_offset_sram_wr_data2_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_WR_DATA2_reg);
		color_icm_dm_icm_rgb_offset_sram_wr_data3_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_WR_DATA3_reg);
		color_icm_dm_icm_rgb_offset_sram_wr_data0_reg.wr_rgb_offset_curve_r0 = rgb_offset_curve[0][i];
		color_icm_dm_icm_rgb_offset_sram_wr_data0_reg.wr_rgb_offset_curve_g0 = rgb_offset_curve[1][i];
		color_icm_dm_icm_rgb_offset_sram_wr_data1_reg.wr_rgb_offset_curve_b0 = rgb_offset_curve[2][i];
		color_icm_dm_icm_rgb_offset_sram_wr_data2_reg.wr_rgb_offset_curve_r1 = rgb_offset_curve[0][i + 1];
		color_icm_dm_icm_rgb_offset_sram_wr_data2_reg.wr_rgb_offset_curve_g1 = rgb_offset_curve[1][i + 1];
		color_icm_dm_icm_rgb_offset_sram_wr_data3_reg.wr_rgb_offset_curve_b1 = rgb_offset_curve[2][i + 1];
		IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_WR_DATA0_reg, color_icm_dm_icm_rgb_offset_sram_wr_data0_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_WR_DATA1_reg, color_icm_dm_icm_rgb_offset_sram_wr_data1_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_WR_DATA2_reg, color_icm_dm_icm_rgb_offset_sram_wr_data2_reg.regValue);
		IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_WR_DATA3_reg, color_icm_dm_icm_rgb_offset_sram_wr_data3_reg.regValue);

		color_icm_dm_icm_rgb_offset_sram_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_CTRL_reg);
		color_icm_dm_icm_rgb_offset_sram_ctrl_reg.rgb_offset_write_en = 1;
		IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_CTRL_reg, color_icm_dm_icm_rgb_offset_sram_ctrl_reg.regValue);
		timeout = 0x062500;
		do
		{
		    //Sleep(50);
		    color_icm_dm_icm_rgb_offset_sram_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_SRAM_CTRL_reg);
		}
		while(color_icm_dm_icm_rgb_offset_sram_ctrl_reg.rgb_offset_write_en && (timeout-- > 0));

		if(timeout <= 0)
		{
		    rtd_pr_vpq_emerg("drvif_color_icm_Set_RGB_Offset_Table write rgb offset table error: address = %d\n", addr);
				return false;
		}
	}

    //s-gain
	color_icm_dm_icm_rgb_offset_s_ctrl0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_S_CTRL0_reg);
	color_icm_dm_icm_rgb_offset_s_ctrl1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_S_CTRL1_reg);
	color_icm_dm_icm_rgb_offset_s_ctrl2_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_S_CTRL2_reg);
	color_icm_dm_icm_rgb_offset_s_ctrl0_reg.rgb_offset_s_gain_0 = rgb_offset_s_gain[0];
	color_icm_dm_icm_rgb_offset_s_ctrl0_reg.rgb_offset_s_gain_1 = rgb_offset_s_gain[1];
	color_icm_dm_icm_rgb_offset_s_ctrl0_reg.rgb_offset_s_gain_2 = rgb_offset_s_gain[2];
	color_icm_dm_icm_rgb_offset_s_ctrl0_reg.rgb_offset_s_gain_3 = rgb_offset_s_gain[3];
	color_icm_dm_icm_rgb_offset_s_ctrl1_reg.rgb_offset_s_gain_4 = rgb_offset_s_gain[4];
	color_icm_dm_icm_rgb_offset_s_ctrl1_reg.rgb_offset_s_gain_5 = rgb_offset_s_gain[5];
	color_icm_dm_icm_rgb_offset_s_ctrl1_reg.rgb_offset_s_gain_6 = rgb_offset_s_gain[6];
	color_icm_dm_icm_rgb_offset_s_ctrl1_reg.rgb_offset_s_gain_7 = rgb_offset_s_gain[7];
	color_icm_dm_icm_rgb_offset_s_ctrl2_reg.rgb_offset_s_gain_8 = rgb_offset_s_gain[8];
	color_icm_dm_icm_rgb_offset_s_ctrl2_reg.rgb_offset_s_gain_9 = rgb_offset_s_gain[9];
	color_icm_dm_icm_rgb_offset_s_ctrl2_reg.rgb_offset_s_gain_10 = rgb_offset_s_gain[10];
	color_icm_dm_icm_rgb_offset_s_ctrl2_reg.rgb_offset_s_gain_11 = rgb_offset_s_gain[11];
	IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_S_CTRL0_reg, color_icm_dm_icm_rgb_offset_s_ctrl0_reg.regValue);
	IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_S_CTRL1_reg, color_icm_dm_icm_rgb_offset_s_ctrl1_reg.regValue);
	IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_S_CTRL2_reg, color_icm_dm_icm_rgb_offset_s_ctrl2_reg.regValue);

	//i-gain
	color_icm_dm_icm_rgb_offset_i_ctrl0_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_I_CTRL0_reg);
	color_icm_dm_icm_rgb_offset_i_ctrl1_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_RGB_OFFSET_I_CTRL1_reg);
	color_icm_dm_icm_rgb_offset_i_ctrl0_reg.rgb_offset_i_gain_0 = rgb_offset_i_gain[0];
	color_icm_dm_icm_rgb_offset_i_ctrl0_reg.rgb_offset_i_gain_1 = rgb_offset_i_gain[1];
	color_icm_dm_icm_rgb_offset_i_ctrl0_reg.rgb_offset_i_gain_2 = rgb_offset_i_gain[2];
	color_icm_dm_icm_rgb_offset_i_ctrl0_reg.rgb_offset_i_gain_3 = rgb_offset_i_gain[3];
	color_icm_dm_icm_rgb_offset_i_ctrl1_reg.rgb_offset_i_gain_4 = rgb_offset_i_gain[4];
	color_icm_dm_icm_rgb_offset_i_ctrl1_reg.rgb_offset_i_gain_5 = rgb_offset_i_gain[5];
	color_icm_dm_icm_rgb_offset_i_ctrl1_reg.rgb_offset_i_gain_6 = rgb_offset_i_gain[6];
	color_icm_dm_icm_rgb_offset_i_ctrl1_reg.rgb_offset_i_gain_7 = rgb_offset_i_gain[7];
	IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_I_CTRL0_reg, color_icm_dm_icm_rgb_offset_i_ctrl0_reg.regValue);
	IoReg_Write32(COLOR_ICM_DM_ICM_RGB_OFFSET_I_CTRL1_reg, color_icm_dm_icm_rgb_offset_i_ctrl1_reg.regValue);

	return true;
}


void drvif_color_icm_readAIReg()
{
#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6
	int h_offset_0,s_offset_0,i_offset_0;
	int h_offset_1,s_offset_1,i_offset_1;
	int h_offset_2,s_offset_2,i_offset_2;
	int h_offset_3,s_offset_3,i_offset_3;
	int h_offset_4,s_offset_4,i_offset_4;
	int h_offset_5,s_offset_5,i_offset_5;

	int ai_adjust_en_reg,ai_detect_value_reg,ai_control_value_reg;
	int ai_octa_tang_dir0_reg,ai_octa_tang_dir1_reg,ai_octa_tang_dir2_reg,ai_octa_tang_dir3_reg,ai_octa_tang_dir4_reg,ai_octa_tang_dir5_reg;
	int ai_octa_tang_mode0_reg,ai_octa_tang_mode1_reg,ai_octa_tang_mode2_reg,ai_octa_tang_mode3_reg,ai_octa_tang_mode4_reg,ai_octa_tang_mode5_reg;

	color_icm_d_icm_ai_6_RBUS  icm_d_icm_ai_6_reg;	
	color_icm_d_icm_ai_7_RBUS  icm_d_icm_ai_7_reg;
	
	color_icm_d_icm_ai_offset_0_RBUS  icm_d_icm_ai_offset_0_reg;
	color_icm_d_icm_ai_offset_1_RBUS  icm_d_icm_ai_offset_1_reg;
	color_icm_d_icm_ai_offset_2_RBUS  icm_d_icm_ai_offset_2_reg;
	color_icm_d_icm_ai_offset_3_RBUS  icm_d_icm_ai_offset_3_reg;
	color_icm_d_icm_ai_offset_4_RBUS  icm_d_icm_ai_offset_4_reg;
	color_icm_d_icm_ai_offset_5_RBUS  icm_d_icm_ai_offset_5_reg;
	color_icm_d_icm_ai_offset_6_RBUS  icm_d_icm_ai_offset_6_reg;
	color_icm_d_icm_ai_offset_7_RBUS  icm_d_icm_ai_offset_7_reg;
	color_icm_d_icm_ai_offset_8_RBUS  icm_d_icm_ai_offset_8_reg;
	color_icm_d_icm_ai_offset_9_RBUS  icm_d_icm_ai_offset_9_reg;
	color_icm_d_icm_ai_offset_10_RBUS  icm_d_icm_ai_offset_10_reg;
	color_icm_d_icm_ai_offset_11_RBUS  icm_d_icm_ai_offset_11_reg;
	
	icm_d_icm_ai_offset_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_0_reg);
	icm_d_icm_ai_offset_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_1_reg);
	icm_d_icm_ai_offset_2_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_2_reg);
	icm_d_icm_ai_offset_3_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_3_reg);
	icm_d_icm_ai_offset_4_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_4_reg);
	icm_d_icm_ai_offset_5_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_5_reg);
	icm_d_icm_ai_offset_6_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_6_reg);
	icm_d_icm_ai_offset_7_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_7_reg);
	icm_d_icm_ai_offset_8_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_8_reg);
	icm_d_icm_ai_offset_9_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_9_reg);
	icm_d_icm_ai_offset_10_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_10_reg);
	icm_d_icm_ai_offset_11_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_OFFSET_11_reg);
	
	i_offset_0 = icm_d_icm_ai_offset_0_reg.ai_i_offset0;
	s_offset_0 = icm_d_icm_ai_offset_0_reg.ai_s_offset0;
	h_offset_0 = icm_d_icm_ai_offset_1_reg.ai_h_offset0;

	i_offset_1 = icm_d_icm_ai_offset_2_reg.ai_i_offset1;
	s_offset_1 = icm_d_icm_ai_offset_2_reg.ai_s_offset1;	
	h_offset_1 = icm_d_icm_ai_offset_3_reg.ai_h_offset1;

	i_offset_2 = icm_d_icm_ai_offset_4_reg.ai_i_offset2;
	s_offset_2 = icm_d_icm_ai_offset_4_reg.ai_s_offset2;	
	h_offset_2 = icm_d_icm_ai_offset_5_reg.ai_h_offset2;	

	i_offset_3 = icm_d_icm_ai_offset_6_reg.ai_i_offset3;
	s_offset_3= icm_d_icm_ai_offset_6_reg.ai_s_offset3;	
	h_offset_3 = icm_d_icm_ai_offset_7_reg.ai_h_offset3;	
	
	i_offset_4 = icm_d_icm_ai_offset_8_reg.ai_i_offset4;
	s_offset_4= icm_d_icm_ai_offset_8_reg.ai_s_offset4;	
	h_offset_4= icm_d_icm_ai_offset_9_reg.ai_h_offset4;	

	i_offset_5 = icm_d_icm_ai_offset_10_reg.ai_i_offset5;
	s_offset_5= icm_d_icm_ai_offset_10_reg.ai_s_offset5;	
	h_offset_5 = icm_d_icm_ai_offset_11_reg.ai_h_offset5;		

	
	icm_d_icm_ai_6_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_6_reg);		
	icm_d_icm_ai_7_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_7_reg);	
	
	ai_adjust_en_reg = icm_d_icm_ai_6_reg.ai_adjust_en;
	ai_detect_value_reg = icm_d_icm_ai_6_reg.ai_detect_value;
	ai_control_value_reg = icm_d_icm_ai_6_reg.ai_control_value;

	ai_octa_tang_dir0_reg = icm_d_icm_ai_7_reg.ai_octa_tang_dir0;
	ai_octa_tang_dir1_reg = icm_d_icm_ai_7_reg.ai_octa_tang_dir1;
	ai_octa_tang_dir2_reg = icm_d_icm_ai_7_reg.ai_octa_tang_dir2;
	ai_octa_tang_dir3_reg = icm_d_icm_ai_7_reg.ai_octa_tang_dir3;
	ai_octa_tang_dir4_reg = icm_d_icm_ai_7_reg.ai_octa_tang_dir4;
	ai_octa_tang_dir5_reg = icm_d_icm_ai_7_reg.ai_octa_tang_dir5;

	ai_octa_tang_mode0_reg = icm_d_icm_ai_7_reg.ai_octa_tang_mode0;
	ai_octa_tang_mode1_reg = icm_d_icm_ai_7_reg.ai_octa_tang_mode1;
	ai_octa_tang_mode2_reg = icm_d_icm_ai_7_reg.ai_octa_tang_mode2;
	ai_octa_tang_mode3_reg = icm_d_icm_ai_7_reg.ai_octa_tang_mode3;
	ai_octa_tang_mode4_reg = icm_d_icm_ai_7_reg.ai_octa_tang_mode4;
	ai_octa_tang_mode5_reg = icm_d_icm_ai_7_reg.ai_octa_tang_mode5;

#endif
}

void drvif_color_icm_AI_debugbox_fadeInOut(int debugbox_en,unsigned int debugbox_width,unsigned int debugbox_color,int debugbox_blendingRatio)
{
	/*
		Parameter:
		debugbox_en:total enable.
		debugbox_width: 0~15,real width=2*reg+1.
		debugbox_color:
					0:white
					1:black
					2:yellow
					3:green
					4:sky blue
					5:pink
					6~7:reserved
		debugbox_blendingRatio: 0~64(0:no debugbox,64:full debugbox)
	*/

	color_icm_d_icm_ai_7_RBUS  icm_d_icm_ai_7_reg;
	color_icm_d_icm_ai_debug_box0_RBUS color_icm_d_icm_ai_debug_box0_reg;
	color_icm_d_icm_ai_debug_box1_RBUS color_icm_d_icm_ai_debug_box1_reg;
	color_icm_d_icm_ai_debug_box2_RBUS color_icm_d_icm_ai_debug_box2_reg;	
		
	icm_d_icm_ai_7_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_7_reg);	
	color_icm_d_icm_ai_debug_box0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_DEBUG_BOX0_reg);	
	color_icm_d_icm_ai_debug_box1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_DEBUG_BOX1_reg);	
	color_icm_d_icm_ai_debug_box2_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_DEBUG_BOX2_reg);		
	
	icm_d_icm_ai_7_reg.ai_debug_mode_box_en = debugbox_en;
	
	color_icm_d_icm_ai_debug_box0_reg.ai_debug_box_line_width0 = debugbox_width & 0xf;
	color_icm_d_icm_ai_debug_box0_reg.ai_debug_box_line_width1 = (debugbox_width >> 4) & 0xf;
	color_icm_d_icm_ai_debug_box1_reg.ai_debug_box_line_width2 = (debugbox_width >> 8) & 0xf;
	color_icm_d_icm_ai_debug_box1_reg.ai_debug_box_line_width3 = (debugbox_width >> 12) & 0xf;
	color_icm_d_icm_ai_debug_box2_reg.ai_debug_box_line_width4 = (debugbox_width >> 16) & 0xf;
	color_icm_d_icm_ai_debug_box2_reg.ai_debug_box_line_width5 = (debugbox_width >> 20) & 0xf;

	color_icm_d_icm_ai_debug_box0_reg.ai_debug_box_color0 = debugbox_color & 0x7;
	color_icm_d_icm_ai_debug_box0_reg.ai_debug_box_color1 = (debugbox_color >> 3) & 0x7;
	color_icm_d_icm_ai_debug_box1_reg.ai_debug_box_color2 = (debugbox_color >> 6) & 0x7;
	color_icm_d_icm_ai_debug_box1_reg.ai_debug_box_color3 = (debugbox_color >> 9) & 0x7;
	color_icm_d_icm_ai_debug_box2_reg.ai_debug_box_color4 = (debugbox_color >> 12) & 0x7;
	color_icm_d_icm_ai_debug_box2_reg.ai_debug_box_color5 = (debugbox_color >> 15) & 0x7;

	color_icm_d_icm_ai_debug_box0_reg.ai_debug_box_blending_ratio0 = debugbox_blendingRatio;
	color_icm_d_icm_ai_debug_box0_reg.ai_debug_box_blending_ratio1 = debugbox_blendingRatio;
	color_icm_d_icm_ai_debug_box1_reg.ai_debug_box_blending_ratio2 = debugbox_blendingRatio;
	color_icm_d_icm_ai_debug_box1_reg.ai_debug_box_blending_ratio3 = debugbox_blendingRatio;
	color_icm_d_icm_ai_debug_box2_reg.ai_debug_box_blending_ratio4 = debugbox_blendingRatio;
	color_icm_d_icm_ai_debug_box2_reg.ai_debug_box_blending_ratio5 = debugbox_blendingRatio;
	
	IoReg_Write32(COLOR_ICM_D_ICM_AI_7_reg, icm_d_icm_ai_7_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_DEBUG_BOX0_reg, color_icm_d_icm_ai_debug_box0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_DEBUG_BOX1_reg, color_icm_d_icm_ai_debug_box1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_DEBUG_BOX2_reg, color_icm_d_icm_ai_debug_box2_reg.regValue);
	
}

void drvif_color_icm_Set_pqmask_hue(unsigned int bEnable, unsigned int gain) {

	color_icm_d_icm_pq_mask_ctrl_RBUS color_icm_d_icm_pq_mask_ctrl_reg;

	color_icm_d_icm_pq_mask_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_PQ_MASK_CTRL_reg);

	color_icm_d_icm_pq_mask_ctrl_reg.pq_mask_h_en = bEnable;
	color_icm_d_icm_pq_mask_ctrl_reg.pq_mask_gain_h = gain;

	IoReg_Write32(COLOR_ICM_D_ICM_PQ_MASK_CTRL_reg, color_icm_d_icm_pq_mask_ctrl_reg.regValue);
}

void drvif_color_icm_Set_pqmask_sat(unsigned int bEnable, unsigned int gain) {

	color_icm_d_icm_pq_mask_ctrl_RBUS color_icm_d_icm_pq_mask_ctrl_reg;

	color_icm_d_icm_pq_mask_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_PQ_MASK_CTRL_reg);

	color_icm_d_icm_pq_mask_ctrl_reg.pq_mask_s_en = bEnable;
	color_icm_d_icm_pq_mask_ctrl_reg.pq_mask_gain_s = gain;

	IoReg_Write32(COLOR_ICM_D_ICM_PQ_MASK_CTRL_reg, color_icm_d_icm_pq_mask_ctrl_reg.regValue);
}

void drvif_color_icm_Set_pqmask_int(unsigned int bEnable, unsigned int gain) {

	color_icm_d_icm_pq_mask_ctrl_RBUS color_icm_d_icm_pq_mask_ctrl_reg;

	color_icm_d_icm_pq_mask_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_PQ_MASK_CTRL_reg);

	color_icm_d_icm_pq_mask_ctrl_reg.pq_mask_i_en = bEnable;
	color_icm_d_icm_pq_mask_ctrl_reg.pq_mask_gain_i = gain;

	IoReg_Write32(COLOR_ICM_D_ICM_PQ_MASK_CTRL_reg, color_icm_d_icm_pq_mask_ctrl_reg.regValue);
}

void drvif_color_icm_Set_pqmask_debug_gain(unsigned char Gain) {

	color_icm_d_icm_pq_mask_ctrl_RBUS color_icm_d_icm_pq_mask_ctrl_reg;

	color_icm_d_icm_pq_mask_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_PQ_MASK_CTRL_reg);

	color_icm_d_icm_pq_mask_ctrl_reg.pq_mask_debug_gain = Gain;

	IoReg_Write32(COLOR_ICM_D_ICM_PQ_MASK_CTRL_reg, color_icm_d_icm_pq_mask_ctrl_reg.regValue);
}
