/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2014
  * All rights reserved.
  * ========================================================================*/
  /*================= File Description =======================================*/
  /**
   * @file
   *  This file is for Local Contrast related functions.
   *
   * @author  $Author$
   * @date	  $Date$
   * @version	  $Revision$
   */

  /*============================ Module dependency	===========================*/
#include <linux/string.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <linux/kernel.h>
#include <rtk_kdriver/io.h>
#include <linux/delay.h>

#include "tvscalercontrol/vip/localcontrast.h"
#include <tvscalercontrol/scaler/vipinclude.h> // for typedef that used in VIP_reg_def.h
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"


#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_LC_DEBUG,fmt,##args)

#ifndef ABS
#define ABS(x) ( ((x)>0)? (x):(-(x)) )
#endif
#ifndef CLIP
#define CLIP(min, max, val) (((val) < (min)) ? (min) : ((val) > (max) ? (max) : (val)))
#endif

#if 0
/* PARAMETERS */
#endif

//unsigned int SLC_Curve[256][33] = {{0}};
static unsigned char u1_SizeValid = 0; // 0:size invalid, 1:size valid

#if 0
/* LOCAL CONTRAST SET FUNCTIONS */
#endif
// Function Description : This function is for LC enable
void drvif_color_set_LC_Enable(unsigned char enable)
{
	lc_lc_global_ctrl1_RBUS lc_lc_global_ctrl1_reg;
	color_d_vc_global_ctrl_RBUS color_d_vc_global_ctrl;

	printk("drvif_color_set_LC_Enable(%d) \n",enable);
	color_d_vc_global_ctrl.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	color_d_vc_global_ctrl.m_lc_comp_en = enable;
	color_d_vc_global_ctrl.write_enable_12 = 1;
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, color_d_vc_global_ctrl.regValue);

	lc_lc_global_ctrl1_reg.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	lc_lc_global_ctrl1_reg.lc_db_apply = 1;
	lc_lc_global_ctrl1_reg.lc_db_en = 1;
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, lc_lc_global_ctrl1_reg.regValue);
}

void drvif_color_set_LC_Global_Ctrl(DRV_LC_Global_Ctrl *ptr)
{
	lc_lc_global_ctrl0_RBUS blu_lc_global_ctrl0;
	lc_lc_global_ctrl1_RBUS blu_lc_global_ctrl1;
	lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
	lc_lc_global_ctrl3_RBUS blu_lc_global_ctrl3;
	blu_lc_global_ctrl0.regValue = IoReg_Read32(LC_LC_Global_Ctrl0_reg);
	blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);
	blu_lc_global_ctrl3.regValue = IoReg_Read32(LC_LC_Global_Ctrl3_reg);

	blu_lc_global_ctrl0.lc_local_sharp_en = ptr->lc_local_sharp_en;
	blu_lc_global_ctrl0.lc_tone_mapping_en = ptr->lc_tone_mapping_en;
	blu_lc_global_ctrl2.lc_hist_mode = ptr->lc_hist_mode;
	blu_lc_global_ctrl2.lc_tenable = ptr->lc_tenable;
	blu_lc_global_ctrl2.lc_valid = ptr->lc_valid;

	IoReg_Write32(LC_LC_Global_Ctrl0_reg, blu_lc_global_ctrl0.regValue);
	IoReg_Write32(LC_LC_Global_Ctrl2_reg, blu_lc_global_ctrl2.regValue);
	IoReg_Write32(LC_LC_Global_Ctrl3_reg, blu_lc_global_ctrl3.regValue);

	// double buffer apply
	blu_lc_global_ctrl1.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	blu_lc_global_ctrl1.lc_db_apply = 1;
	blu_lc_global_ctrl1.lc_db_en = 0;
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, blu_lc_global_ctrl1.regValue);

}

void drvif_color_set_LC_Temporal_Filter(DRV_LC_Temporal_Filter *ptr)
{
	lc_lc_temporal_ctrl1_RBUS blu_lc_temporal_ctrl1;
	lc_lc_temporal_ctrl2_RBUS blu_lc_temporal_ctrl2;
	lc_lc_temporal_ctrl3_RBUS blu_lc_temporal_ctrl3;

	blu_lc_temporal_ctrl1.regValue = IoReg_Read32(LC_LC_Temporal_Ctrl1_reg);
	blu_lc_temporal_ctrl2.regValue = IoReg_Read32(LC_LC_Temporal_Ctrl2_reg);
	blu_lc_temporal_ctrl3.regValue = IoReg_Read32(LC_LC_Temporal_Ctrl3_reg);

	blu_lc_temporal_ctrl1.lc_tmp_pos0thd = ptr->lc_tmp_pos0thd;
	blu_lc_temporal_ctrl1.lc_tmp_pos1thd = ptr->lc_tmp_pos1thd;
	blu_lc_temporal_ctrl1.lc_tmp_posmingain = ptr->lc_tmp_posmingain;
	blu_lc_temporal_ctrl1.lc_tmp_posmaxgain = ptr->lc_tmp_posmaxgain;
	blu_lc_temporal_ctrl2.lc_tmp_neg0thd = ptr->lc_tmp_neg0thd;
	blu_lc_temporal_ctrl2.lc_tmp_neg1thd = ptr->lc_tmp_neg1thd;
	blu_lc_temporal_ctrl2.lc_tmp_negmingain = ptr->lc_tmp_negmingain;
	blu_lc_temporal_ctrl2.lc_tmp_negmaxgain = ptr->lc_tmp_negmaxgain;
	blu_lc_temporal_ctrl3.lc_tmp_maxdiff = ptr->lc_tmp_maxdiff;
	blu_lc_temporal_ctrl3.lc_tmp_scenechangegain1 = ptr->lc_tmp_scenechangegain1;

	IoReg_Write32(LC_LC_Temporal_Ctrl1_reg, blu_lc_temporal_ctrl1.regValue);
	IoReg_Write32(LC_LC_Temporal_Ctrl2_reg, blu_lc_temporal_ctrl2.regValue);
	IoReg_Write32(LC_LC_Temporal_Ctrl3_reg, blu_lc_temporal_ctrl3.regValue);
}


void drvif_color_set_LC_Backlight_Profile_Interpolation(DRV_LC_Backlight_Profile_Interpolation *ptr)
{
	lc_lc_inter_hctrl1_RBUS blu_lc_inter_hctrl1;
	lc_lc_inter_hctrl2_RBUS blu_lc_inter_hctrl2;
	lc_lc_inter_vctrl1_RBUS blu_lc_inter_vctrl1;
	lc_lc_inter_vctrl2_RBUS blu_lc_inter_vctrl2;

	lc_lc_sram_ctrl_RBUS blu_lc_sram_ctrl;
	//* by table
	lc_lc_blight_ap_RBUS blu_lc_blight_ap;
	lc_lc_blight_dp_RBUS blu_lc_blight_dp;
	//*/

	blu_lc_inter_hctrl1.regValue = IoReg_Read32(LC_LC_Inter_HCtrl1_reg);
	blu_lc_inter_hctrl2.regValue = IoReg_Read32(LC_LC_Inter_HCtrl2_reg);
	blu_lc_inter_vctrl1.regValue = IoReg_Read32(LC_LC_Inter_VCtrl1_reg);
	blu_lc_inter_vctrl2.regValue = IoReg_Read32(LC_LC_Inter_VCtrl2_reg);

	blu_lc_sram_ctrl.regValue = IoReg_Read32(LC_LC_SRAM_CTRL_reg);
	//*
	blu_lc_blight_ap.regValue = IoReg_Read32(LC_LC_BLIGHT_AP_reg);
	blu_lc_blight_dp.regValue = IoReg_Read32(LC_LC_BLIGHT_DP_reg);
	//*/

	blu_lc_inter_hctrl1.lc_tab_hsize = ptr->lc_tab_hsize;
	blu_lc_inter_hctrl2.lc_hboundary = ptr->lc_hboundary;
	blu_lc_inter_hctrl2.lc_hinitphase1 = ptr->lc_hinitphase1;
	blu_lc_inter_hctrl2.lc_hinitphase2 = ptr->lc_hinitphase2;
	blu_lc_inter_hctrl2.lc_hinitphase3_left = ptr->lc_hinitphase3_left;
	blu_lc_inter_vctrl1.lc_tab_vsize = ptr->lc_tab_vsize;
	blu_lc_inter_vctrl2.lc_vboundary = ptr->lc_vboundary;
	blu_lc_inter_vctrl2.lc_vinitphase1 = ptr->lc_vinitphase1;
	blu_lc_inter_vctrl2.lc_vinitphase2 = ptr->lc_vinitphase2;
	blu_lc_inter_vctrl2.lc_vinitphase3_left = ptr->lc_vinitphase3_left;

	blu_lc_sram_ctrl.lc_blight_update_en = ptr->lc_blight_update_en;
	blu_lc_sram_ctrl.lc_blight_sw_mode = ptr->lc_blight_sw_mode;

	IoReg_Write32(LC_LC_SRAM_CTRL_reg, blu_lc_sram_ctrl.regValue);

	IoReg_Write32(LC_LC_Inter_HCtrl1_reg, blu_lc_inter_hctrl1.regValue);
	IoReg_Write32(LC_LC_Inter_HCtrl2_reg, blu_lc_inter_hctrl2.regValue);
	IoReg_Write32(LC_LC_Inter_VCtrl1_reg, blu_lc_inter_vctrl1.regValue);
	IoReg_Write32(LC_LC_Inter_VCtrl2_reg, blu_lc_inter_vctrl2.regValue);

	IoReg_Write32(LC_LC_BLIGHT_AP_reg, blu_lc_blight_ap.regValue);
	IoReg_Write32(LC_LC_BLIGHT_DP_reg, blu_lc_blight_dp.regValue);

}

void drvif_color_set_LC_BL_Profile_Interpolation_Table(unsigned short *ptr, unsigned char mode ,unsigned char tab_size)
{
	int i = 0;
	//unsigned char tab_hsize = 0, tab_vsize = 0;

	lc_lc_inter_hctrl1_RBUS blu_lc_inter_hctrl1;
	lc_lc_inter_vctrl1_RBUS blu_lc_inter_vctrl1;

	lc_lc_sram_ctrl_RBUS blu_lc_sram_ctrl;

	//* by table
	lc_lc_vtab_ap_RBUS blu_lc_vtab_ap;
	lc_lc_vtab_dp_RBUS blu_lc_vtab_dp;
	lc_lc_htab_ap_RBUS blu_lc_htab_ap;
	lc_lc_htab_dp_RBUS blu_lc_htab_dp;
	//*/
	blu_lc_inter_hctrl1.regValue= IoReg_Read32(LC_LC_Inter_HCtrl1_reg);
	blu_lc_inter_vctrl1.regValue = IoReg_Read32(LC_LC_Inter_VCtrl1_reg);

	//tab_hsize = blu_lc_inter_hctrl1.lc_tab_hsize;
	//tab_vsize = blu_lc_inter_vctrl1.lc_tab_vsize;

	if(mode == 0)
	{
		//* by table
		blu_lc_htab_ap.regValue = IoReg_Read32(LC_LC_HTAB_AP_reg);
		blu_lc_htab_dp.regValue = IoReg_Read32(LC_LC_HTAB_DP_reg);
		//*/
		blu_lc_sram_ctrl.regValue = IoReg_Read32(LC_LC_SRAM_CTRL_reg);

		blu_lc_sram_ctrl.lc_table_sw_mode = 1; // software mode enable
		IoReg_Write32(LC_LC_SRAM_CTRL_reg, blu_lc_sram_ctrl.regValue);

		blu_lc_htab_ap.lc_htab_adr = 0;
		IoReg_Write32(LC_LC_HTAB_AP_reg, blu_lc_htab_ap.regValue);

		for(i=0;i<(tab_size/2+tab_size%2);i++)
		{
			blu_lc_htab_dp.lc_htab_dat_msb= *(ptr+i*2+1);//ptr->ld_htab_dat_msb;
			blu_lc_htab_dp.lc_htab_dat_lsb = *(ptr+i*2);//ptr->ld_htab_dat_lsb;
			IoReg_Write32(LC_LC_HTAB_DP_reg, blu_lc_htab_dp.regValue);
		}
	}
	else
	{
		//* by table
		blu_lc_vtab_ap.regValue = IoReg_Read32(LC_LC_VTAB_AP_reg);
		blu_lc_vtab_dp.regValue = IoReg_Read32(LC_LC_VTAB_DP_reg);
		//*/
		blu_lc_sram_ctrl.regValue = IoReg_Read32(LC_LC_SRAM_CTRL_reg);

		blu_lc_sram_ctrl.lc_table_sw_mode = 1; // software mode enable
		IoReg_Write32(LC_LC_SRAM_CTRL_reg, blu_lc_sram_ctrl.regValue);

		blu_lc_vtab_ap.lc_vtab_adr = 0;
		IoReg_Write32(LC_LC_VTAB_AP_reg, blu_lc_vtab_ap.regValue);

		for(i=0;i<(tab_size/2+tab_size%2);i++)
		{
			blu_lc_vtab_dp.lc_vtab_dat_msb = *(ptr+i*2+1);//ptr->ld_vtab_dat_msb;
			blu_lc_vtab_dp.lc_vtab_dat_lsb = *(ptr+i*2);//ptr->ld_vtab_dat_lsb;
			IoReg_Write32(LC_LC_VTAB_DP_reg, blu_lc_vtab_dp.regValue);
		}

	}
	blu_lc_sram_ctrl.lc_table_sw_mode = 0; // software mode disable
	IoReg_Write32(LC_LC_SRAM_CTRL_reg, blu_lc_sram_ctrl.regValue);

}



void drvif_color_set_LC_Demo_Window(DRV_LC_Demo_Window *ptr)
{
	lc_lc_demo_window_ctrl0_RBUS blu_lc_demo_window_ctrl0;
	lc_lc_demo_window_ctrl1_RBUS blu_lc_demo_window_ctrl1;

	blu_lc_demo_window_ctrl0.regValue = IoReg_Read32(LC_LC_Demo_window_ctrl0_reg);
	blu_lc_demo_window_ctrl1.regValue = IoReg_Read32(LC_LC_Demo_window_ctrl1_reg);

	blu_lc_demo_window_ctrl0.lc_demo_en = ptr->lc_demo_en;
	blu_lc_demo_window_ctrl0.lc_demo_mode = ptr->lc_demo_mode;
	blu_lc_demo_window_ctrl0.lc_demo_top = ptr->lc_demo_top;
	blu_lc_demo_window_ctrl0.lc_demo_bottom = ptr->lc_demo_bottom;
	blu_lc_demo_window_ctrl1.lc_demo_left = ptr->lc_demo_left;
	blu_lc_demo_window_ctrl1.lc_demo_right = ptr->lc_demo_right;

	IoReg_Write32(LC_LC_Demo_window_ctrl0_reg, blu_lc_demo_window_ctrl0.regValue);
	IoReg_Write32(LC_LC_Demo_window_ctrl1_reg, blu_lc_demo_window_ctrl1.regValue);

}

void drvif_color_set_LC_ToneMapping_Grid0(DRV_LC_ToneMapping_Grid0 *ptr)
{
	lc_lc_tonemapping_grid0_0_RBUS lc_tonemapping_grid0_0;
	lc_lc_tonemapping_grid0_1_RBUS lc_tonemapping_grid0_1;
	lc_lc_tonemapping_grid0_2_RBUS lc_tonemapping_grid0_2;
	lc_lc_tonemapping_grid0_3_RBUS lc_tonemapping_grid0_3;
	lc_lc_tonemapping_grid0_4_RBUS lc_tonemapping_grid0_4;
	lc_lc_tonemapping_grid0_5_RBUS lc_tonemapping_grid0_5;
	lc_lc_tonemapping_grid0_6_RBUS lc_tonemapping_grid0_6;
	lc_lc_tonemapping_grid0_7_RBUS lc_tonemapping_grid0_7;
	lc_lc_tonemapping_grid0_8_RBUS lc_tonemapping_grid0_8;
	lc_lc_tonemapping_grid0_9_RBUS lc_tonemapping_grid0_9;
	lc_lc_tonemapping_grid0_10_RBUS lc_tonemapping_grid0_10;
	lc_lc_tonemapping_grid0_11_RBUS lc_tonemapping_grid0_11;
	lc_lc_tonemapping_grid0_12_RBUS lc_tonemapping_grid0_12;
	lc_lc_tonemapping_grid0_13_RBUS lc_tonemapping_grid0_13;
	lc_lc_tonemapping_grid0_14_RBUS lc_tonemapping_grid0_14;
	lc_lc_tonemapping_grid0_15_RBUS lc_tonemapping_grid0_15;

	lc_tonemapping_grid0_0.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_0_reg);
	lc_tonemapping_grid0_1.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_1_reg);
	lc_tonemapping_grid0_2.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_2_reg);
	lc_tonemapping_grid0_3.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_3_reg);
	lc_tonemapping_grid0_4.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_4_reg);
	lc_tonemapping_grid0_5.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_5_reg);
	lc_tonemapping_grid0_6.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_6_reg);
	lc_tonemapping_grid0_7.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_7_reg);
	lc_tonemapping_grid0_8.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_8_reg);
	lc_tonemapping_grid0_9.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_9_reg);
	lc_tonemapping_grid0_10.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_10_reg);
	lc_tonemapping_grid0_11.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_11_reg);
	lc_tonemapping_grid0_12.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_12_reg);
	lc_tonemapping_grid0_13.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_13_reg);
	lc_tonemapping_grid0_14.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_14_reg);
	lc_tonemapping_grid0_15.regValue = IoReg_Read32(LC_LC_ToneMapping_grid0_15_reg);

	lc_tonemapping_grid0_0.lc_tmap_g0_th1 = ptr->lc_tmap_g0_th1;
	lc_tonemapping_grid0_1.lc_tmap_g0_th2 = ptr->lc_tmap_g0_th2;
	lc_tonemapping_grid0_1.lc_tmap_g0_th3 = ptr->lc_tmap_g0_th3;
	lc_tonemapping_grid0_2.lc_tmap_g0_th4 = ptr->lc_tmap_g0_th4;
	lc_tonemapping_grid0_2.lc_tmap_g0_th5 = ptr->lc_tmap_g0_th5;
	lc_tonemapping_grid0_3.lc_tmap_g0_th6 = ptr->lc_tmap_g0_th6;
	lc_tonemapping_grid0_3.lc_tmap_g0_th7 = ptr->lc_tmap_g0_th7;
	lc_tonemapping_grid0_4.lc_tmap_g0_th8 = ptr->lc_tmap_g0_th8;
	lc_tonemapping_grid0_4.lc_tmap_g0_th9 = ptr->lc_tmap_g0_th9;
	lc_tonemapping_grid0_5.lc_tmap_g0_th10 = ptr->lc_tmap_g0_th10;
	lc_tonemapping_grid0_5.lc_tmap_g0_th11 = ptr->lc_tmap_g0_th11;
	lc_tonemapping_grid0_6.lc_tmap_g0_th12 = ptr->lc_tmap_g0_th12;
	lc_tonemapping_grid0_6.lc_tmap_g0_th13 = ptr->lc_tmap_g0_th13;
	lc_tonemapping_grid0_7.lc_tmap_g0_th14 = ptr->lc_tmap_g0_th14;
	lc_tonemapping_grid0_7.lc_tmap_g0_th15 = ptr->lc_tmap_g0_th15;
	lc_tonemapping_grid0_8.lc_tmap_g0_th16 = ptr->lc_tmap_g0_th16;
	lc_tonemapping_grid0_8.lc_tmap_g0_th17 = ptr->lc_tmap_g0_th17;
	lc_tonemapping_grid0_9.lc_tmap_g0_th18 = ptr->lc_tmap_g0_th18;
	lc_tonemapping_grid0_9.lc_tmap_g0_th19 = ptr->lc_tmap_g0_th19;
	lc_tonemapping_grid0_10.lc_tmap_g0_th20 = ptr->lc_tmap_g0_th20;
	lc_tonemapping_grid0_10.lc_tmap_g0_th21 = ptr->lc_tmap_g0_th21;
	lc_tonemapping_grid0_11.lc_tmap_g0_th22 = ptr->lc_tmap_g0_th22;
	lc_tonemapping_grid0_11.lc_tmap_g0_th23 = ptr->lc_tmap_g0_th23;
	lc_tonemapping_grid0_12.lc_tmap_g0_th24 = ptr->lc_tmap_g0_th24;
	lc_tonemapping_grid0_12.lc_tmap_g0_th25 = ptr->lc_tmap_g0_th25;
	lc_tonemapping_grid0_13.lc_tmap_g0_th26 = ptr->lc_tmap_g0_th26;
	lc_tonemapping_grid0_13.lc_tmap_g0_th27 = ptr->lc_tmap_g0_th27;
	lc_tonemapping_grid0_14.lc_tmap_g0_th28 = ptr->lc_tmap_g0_th28;
	lc_tonemapping_grid0_14.lc_tmap_g0_th29 = ptr->lc_tmap_g0_th29;
	lc_tonemapping_grid0_15.lc_tmap_g0_th30 = ptr->lc_tmap_g0_th30;
	lc_tonemapping_grid0_15.lc_tmap_g0_th31 = ptr->lc_tmap_g0_th31;

	IoReg_Write32(LC_LC_ToneMapping_grid0_0_reg, lc_tonemapping_grid0_0.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_1_reg, lc_tonemapping_grid0_1.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_2_reg, lc_tonemapping_grid0_2.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_3_reg, lc_tonemapping_grid0_3.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_4_reg, lc_tonemapping_grid0_4.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_5_reg, lc_tonemapping_grid0_5.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_6_reg, lc_tonemapping_grid0_6.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_7_reg, lc_tonemapping_grid0_7.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_8_reg, lc_tonemapping_grid0_8.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_9_reg, lc_tonemapping_grid0_9.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_10_reg, lc_tonemapping_grid0_10.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_11_reg, lc_tonemapping_grid0_11.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_12_reg, lc_tonemapping_grid0_12.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_13_reg, lc_tonemapping_grid0_13.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_14_reg, lc_tonemapping_grid0_14.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid0_15_reg, lc_tonemapping_grid0_15.regValue);


}

void drvif_color_set_LC_ToneMapping_Grid2(DRV_LC_ToneMapping_Grid2 *ptr)
{
	lc_lc_tonemapping_grid2_0_RBUS lc_tonemapping_grid2_0;
	lc_lc_tonemapping_grid2_1_RBUS lc_tonemapping_grid2_1;
	lc_lc_tonemapping_grid2_2_RBUS lc_tonemapping_grid2_2;
	lc_lc_tonemapping_grid2_3_RBUS lc_tonemapping_grid2_3;
	lc_lc_tonemapping_grid2_4_RBUS lc_tonemapping_grid2_4;
	lc_lc_tonemapping_grid2_5_RBUS lc_tonemapping_grid2_5;
	lc_lc_tonemapping_grid2_6_RBUS lc_tonemapping_grid2_6;
	lc_lc_tonemapping_grid2_7_RBUS lc_tonemapping_grid2_7;
	lc_lc_tonemapping_grid2_8_RBUS lc_tonemapping_grid2_8;
	lc_lc_tonemapping_grid2_9_RBUS lc_tonemapping_grid2_9;
	lc_lc_tonemapping_grid2_10_RBUS lc_tonemapping_grid2_10;
	lc_lc_tonemapping_grid2_11_RBUS lc_tonemapping_grid2_11;
	lc_lc_tonemapping_grid2_12_RBUS lc_tonemapping_grid2_12;
	lc_lc_tonemapping_grid2_13_RBUS lc_tonemapping_grid2_13;
	lc_lc_tonemapping_grid2_14_RBUS lc_tonemapping_grid2_14;
	lc_lc_tonemapping_grid2_15_RBUS lc_tonemapping_grid2_15;

	lc_tonemapping_grid2_0.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_0_reg);
	lc_tonemapping_grid2_1.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_1_reg);
	lc_tonemapping_grid2_2.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_2_reg);
	lc_tonemapping_grid2_3.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_3_reg);
	lc_tonemapping_grid2_4.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_4_reg);
	lc_tonemapping_grid2_5.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_5_reg);
	lc_tonemapping_grid2_6.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_6_reg);
	lc_tonemapping_grid2_7.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_7_reg);
	lc_tonemapping_grid2_8.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_8_reg);
	lc_tonemapping_grid2_9.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_9_reg);
	lc_tonemapping_grid2_10.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_10_reg);
	lc_tonemapping_grid2_11.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_11_reg);
	lc_tonemapping_grid2_12.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_12_reg);
	lc_tonemapping_grid2_13.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_13_reg);
	lc_tonemapping_grid2_14.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_14_reg);
	lc_tonemapping_grid2_15.regValue = IoReg_Read32(LC_LC_ToneMapping_grid2_15_reg);

	lc_tonemapping_grid2_0.lc_tmap_g2_th1 = ptr->lc_tmap_g2_th1;
	lc_tonemapping_grid2_1.lc_tmap_g2_th2 = ptr->lc_tmap_g2_th2;
	lc_tonemapping_grid2_1.lc_tmap_g2_th3 = ptr->lc_tmap_g2_th3;
	lc_tonemapping_grid2_2.lc_tmap_g2_th4 = ptr->lc_tmap_g2_th4;
	lc_tonemapping_grid2_2.lc_tmap_g2_th5 = ptr->lc_tmap_g2_th5;
	lc_tonemapping_grid2_3.lc_tmap_g2_th6 = ptr->lc_tmap_g2_th6;
	lc_tonemapping_grid2_3.lc_tmap_g2_th7 = ptr->lc_tmap_g2_th7;
	lc_tonemapping_grid2_4.lc_tmap_g2_th8 = ptr->lc_tmap_g2_th8;
	lc_tonemapping_grid2_4.lc_tmap_g2_th9 = ptr->lc_tmap_g2_th9;
	lc_tonemapping_grid2_5.lc_tmap_g2_th10 = ptr->lc_tmap_g2_th10;
	lc_tonemapping_grid2_5.lc_tmap_g2_th11 = ptr->lc_tmap_g2_th11;
	lc_tonemapping_grid2_6.lc_tmap_g2_th12 = ptr->lc_tmap_g2_th12;
	lc_tonemapping_grid2_6.lc_tmap_g2_th13 = ptr->lc_tmap_g2_th13;
	lc_tonemapping_grid2_7.lc_tmap_g2_th14 = ptr->lc_tmap_g2_th14;
	lc_tonemapping_grid2_7.lc_tmap_g2_th15 = ptr->lc_tmap_g2_th15;
	lc_tonemapping_grid2_8.lc_tmap_g2_th16 = ptr->lc_tmap_g2_th16;
	lc_tonemapping_grid2_8.lc_tmap_g2_th17 = ptr->lc_tmap_g2_th17;
	lc_tonemapping_grid2_9.lc_tmap_g2_th18 = ptr->lc_tmap_g2_th18;
	lc_tonemapping_grid2_9.lc_tmap_g2_th19 = ptr->lc_tmap_g2_th19;
	lc_tonemapping_grid2_10.lc_tmap_g2_th20 = ptr->lc_tmap_g2_th20;
	lc_tonemapping_grid2_10.lc_tmap_g2_th21 = ptr->lc_tmap_g2_th21;
	lc_tonemapping_grid2_11.lc_tmap_g2_th22 = ptr->lc_tmap_g2_th22;
	lc_tonemapping_grid2_11.lc_tmap_g2_th23 = ptr->lc_tmap_g2_th23;
	lc_tonemapping_grid2_12.lc_tmap_g2_th24 = ptr->lc_tmap_g2_th24;
	lc_tonemapping_grid2_12.lc_tmap_g2_th25 = ptr->lc_tmap_g2_th25;
	lc_tonemapping_grid2_13.lc_tmap_g2_th26 = ptr->lc_tmap_g2_th26;
	lc_tonemapping_grid2_13.lc_tmap_g2_th27 = ptr->lc_tmap_g2_th27;
	lc_tonemapping_grid2_14.lc_tmap_g2_th28 = ptr->lc_tmap_g2_th28;
	lc_tonemapping_grid2_14.lc_tmap_g2_th29 = ptr->lc_tmap_g2_th29;
	lc_tonemapping_grid2_15.lc_tmap_g2_th30 = ptr->lc_tmap_g2_th30;
	lc_tonemapping_grid2_15.lc_tmap_g2_th31 = ptr->lc_tmap_g2_th31;

	IoReg_Write32(LC_LC_ToneMapping_grid2_0_reg, lc_tonemapping_grid2_0.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_1_reg, lc_tonemapping_grid2_1.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_2_reg, lc_tonemapping_grid2_2.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_3_reg, lc_tonemapping_grid2_3.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_4_reg, lc_tonemapping_grid2_4.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_5_reg, lc_tonemapping_grid2_5.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_6_reg, lc_tonemapping_grid2_6.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_7_reg, lc_tonemapping_grid2_7.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_8_reg, lc_tonemapping_grid2_8.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_9_reg, lc_tonemapping_grid2_9.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_10_reg, lc_tonemapping_grid2_10.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_11_reg, lc_tonemapping_grid2_11.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_12_reg, lc_tonemapping_grid2_12.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_13_reg, lc_tonemapping_grid2_13.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_14_reg, lc_tonemapping_grid2_14.regValue);
	IoReg_Write32(LC_LC_ToneMapping_grid2_15_reg, lc_tonemapping_grid2_15.regValue);


}

void drvif_color_set_LC_ToneMapping_CurveSelect(DRV_LC_ToneMapping_CurveSelect *ptr)
{
	lc_lc_tonemapping_curve_sel_0_RBUS curve_sel_0;
	lc_lc_tonemapping_curve_sel_1_RBUS curve_sel_1;
	lc_lc_tonemapping_curve_sel_2_RBUS curve_sel_2;
	lc_lc_tonemapping_curve_sel_3_RBUS curve_sel_3;

	curve_sel_0.regValue = IoReg_Read32(LC_LC_ToneMapping_Curve_sel_0_reg);
	curve_sel_1.regValue = IoReg_Read32(LC_LC_ToneMapping_Curve_sel_1_reg);
	curve_sel_2.regValue = IoReg_Read32(LC_LC_ToneMapping_Curve_sel_2_reg);
	curve_sel_3.regValue = IoReg_Read32(LC_LC_ToneMapping_Curve_sel_3_reg);

	curve_sel_0.lc_tmap_curve0_grid = ptr->lc_tmap_curve0_grid;
	curve_sel_0.lc_tmap_curve0_tone = ptr->lc_tmap_curve0_tone;
	curve_sel_0.lc_tmap_curve1_grid = ptr->lc_tmap_curve1_grid;
	curve_sel_0.lc_tmap_curve1_tone = ptr->lc_tmap_curve1_tone;
	curve_sel_1.lc_tmap_curve2_grid = ptr->lc_tmap_curve2_grid;
	curve_sel_1.lc_tmap_curve2_tone = ptr->lc_tmap_curve2_tone;
	curve_sel_1.lc_tmap_curve3_grid = ptr->lc_tmap_curve3_grid;
	curve_sel_1.lc_tmap_curve3_tone = ptr->lc_tmap_curve3_tone;
	curve_sel_2.lc_tmap_curve4_grid = ptr->lc_tmap_curve4_grid;
	curve_sel_2.lc_tmap_curve4_tone = ptr->lc_tmap_curve4_tone;
	curve_sel_2.lc_tmap_curve5_grid = ptr->lc_tmap_curve5_grid;
	curve_sel_2.lc_tmap_curve5_tone = ptr->lc_tmap_curve5_tone;
	curve_sel_3.lc_tmap_curve6_grid = ptr->lc_tmap_curve6_grid;
	curve_sel_3.lc_tmap_curve6_tone = ptr->lc_tmap_curve6_tone;
	curve_sel_3.lc_tmap_curve7_grid = ptr->lc_tmap_curve7_grid;
	curve_sel_3.lc_tmap_curve7_tone = ptr->lc_tmap_curve7_tone;

	IoReg_Write32(LC_LC_ToneMapping_Curve_sel_0_reg, curve_sel_0.regValue);
	IoReg_Write32(LC_LC_ToneMapping_Curve_sel_1_reg, curve_sel_1.regValue);
	IoReg_Write32(LC_LC_ToneMapping_Curve_sel_2_reg, curve_sel_2.regValue);
	IoReg_Write32(LC_LC_ToneMapping_Curve_sel_3_reg, curve_sel_3.regValue);


}

void drvif_color_set_LC_ToneMapping_Blend(DRV_LC_ToneMapping_Blend *ptr)
{
	lc_lc_tonemapping_blending_RBUS blending;

	blending.regValue = IoReg_Read32(LC_LC_ToneMapping_blending_reg);

	blending.lc_tmap_blend_factor = ptr->lc_tmap_blend_factor;
	blending.lc_tmap_slope_unit = ptr->lc_tmap_slope_unit;

	IoReg_Write32(LC_LC_ToneMapping_blending_reg, blending.regValue);
}

void drvif_color_get_LC_ToneMapping_Blend(DRV_LC_ToneMapping_Blend *ptr)
{
	lc_lc_tonemapping_blending_RBUS blending;

	blending.regValue = IoReg_Read32(LC_LC_ToneMapping_blending_reg);

	ptr->lc_tmap_blend_factor = blending.lc_tmap_blend_factor;
	ptr->lc_tmap_slope_unit = blending.lc_tmap_slope_unit;

	IoReg_Write32(LC_LC_ToneMapping_blending_reg, blending.regValue);
}

void drvif_color_set_LC_Shpnr_Gain1st(DRV_LC_Shpnr_Gain1st *ptr)
{
	lc_lc_local_shpnr_1st_gain_0_RBUS shpnr_1st_gain_0;
	lc_lc_local_shpnr_1st_gain_1_RBUS shpnr_1st_gain_1;
	lc_lc_local_shpnr_1st_gain_2_RBUS shpnr_1st_gain_2;
	lc_lc_local_shpnr_1st_gain_3_RBUS shpnr_1st_gain_3;
	lc_lc_local_shpnr_1st_gain_4_RBUS shpnr_1st_gain_4;
	lc_lc_local_shpnr_1st_gain_5_RBUS shpnr_1st_gain_5;
	lc_lc_local_shpnr_1st_gain_6_RBUS shpnr_1st_gain_6;
	lc_lc_local_shpnr_1st_gain_7_RBUS shpnr_1st_gain_7;
	lc_lc_local_shpnr_1st_gain_8_RBUS shpnr_1st_gain_8;

	shpnr_1st_gain_0.regValue = IoReg_Read32(LC_LC_Local_Shpnr_1st_gain_0_reg);
	shpnr_1st_gain_1.regValue = IoReg_Read32(LC_LC_Local_Shpnr_1st_gain_1_reg);
	shpnr_1st_gain_2.regValue = IoReg_Read32(LC_LC_Local_Shpnr_1st_gain_2_reg);
	shpnr_1st_gain_3.regValue = IoReg_Read32(LC_LC_Local_Shpnr_1st_gain_3_reg);
	shpnr_1st_gain_4.regValue = IoReg_Read32(LC_LC_Local_Shpnr_1st_gain_4_reg);
	shpnr_1st_gain_5.regValue = IoReg_Read32(LC_LC_Local_Shpnr_1st_gain_5_reg);
	shpnr_1st_gain_6.regValue = IoReg_Read32(LC_LC_Local_Shpnr_1st_gain_6_reg);
	shpnr_1st_gain_7.regValue = IoReg_Read32(LC_LC_Local_Shpnr_1st_gain_7_reg);
	shpnr_1st_gain_8.regValue = IoReg_Read32(LC_LC_Local_Shpnr_1st_gain_8_reg);

	shpnr_1st_gain_0.lc_gain_by_yin_0 = ptr->lc_gain_by_yin_0;
	shpnr_1st_gain_0.lc_gain_by_yin_1 = ptr->lc_gain_by_yin_1;
	shpnr_1st_gain_0.lc_gain_by_yin_2 = ptr->lc_gain_by_yin_2;
	shpnr_1st_gain_0.lc_gain_by_yin_3 = ptr->lc_gain_by_yin_3;
	shpnr_1st_gain_1.lc_gain_by_yin_4 = ptr->lc_gain_by_yin_4;
	shpnr_1st_gain_1.lc_gain_by_yin_5 = ptr->lc_gain_by_yin_5;
	shpnr_1st_gain_1.lc_gain_by_yin_6 = ptr->lc_gain_by_yin_6;
	shpnr_1st_gain_1.lc_gain_by_yin_7 = ptr->lc_gain_by_yin_7;
	shpnr_1st_gain_2.lc_gain_by_yin_8 = ptr->lc_gain_by_yin_8;
	shpnr_1st_gain_2.lc_gain_by_yin_9 = ptr->lc_gain_by_yin_9;
	shpnr_1st_gain_2.lc_gain_by_yin_10 = ptr->lc_gain_by_yin_10;
	shpnr_1st_gain_2.lc_gain_by_yin_11 = ptr->lc_gain_by_yin_11;
	shpnr_1st_gain_3.lc_gain_by_yin_12 = ptr->lc_gain_by_yin_12;
	shpnr_1st_gain_3.lc_gain_by_yin_13 = ptr->lc_gain_by_yin_13;
	shpnr_1st_gain_3.lc_gain_by_yin_14 = ptr->lc_gain_by_yin_14;
	shpnr_1st_gain_3.lc_gain_by_yin_15 = ptr->lc_gain_by_yin_15;
	shpnr_1st_gain_4.lc_gain_by_yin_16 = ptr->lc_gain_by_yin_16;
	shpnr_1st_gain_4.lc_gain_by_yin_17 = ptr->lc_gain_by_yin_17;
	shpnr_1st_gain_4.lc_gain_by_yin_18 = ptr->lc_gain_by_yin_18;
	shpnr_1st_gain_4.lc_gain_by_yin_19 = ptr->lc_gain_by_yin_19;
	shpnr_1st_gain_5.lc_gain_by_yin_20 = ptr->lc_gain_by_yin_20;
	shpnr_1st_gain_5.lc_gain_by_yin_21 = ptr->lc_gain_by_yin_21;
	shpnr_1st_gain_5.lc_gain_by_yin_22 = ptr->lc_gain_by_yin_22;
	shpnr_1st_gain_5.lc_gain_by_yin_23 = ptr->lc_gain_by_yin_23;
	shpnr_1st_gain_6.lc_gain_by_yin_24 = ptr->lc_gain_by_yin_24;
	shpnr_1st_gain_6.lc_gain_by_yin_25 = ptr->lc_gain_by_yin_25;
	shpnr_1st_gain_6.lc_gain_by_yin_26 = ptr->lc_gain_by_yin_26;
	shpnr_1st_gain_6.lc_gain_by_yin_27 = ptr->lc_gain_by_yin_27;
	shpnr_1st_gain_7.lc_gain_by_yin_28 = ptr->lc_gain_by_yin_28;
	shpnr_1st_gain_7.lc_gain_by_yin_29 = ptr->lc_gain_by_yin_29;
	shpnr_1st_gain_7.lc_gain_by_yin_30 = ptr->lc_gain_by_yin_30;
	shpnr_1st_gain_7.lc_gain_by_yin_31 = ptr->lc_gain_by_yin_31;
	shpnr_1st_gain_8.lc_gain_by_yin_32 = ptr->lc_gain_by_yin_32;
	shpnr_1st_gain_8.lc_gain_by_yin_divisor = ptr->lc_gain_by_yin_divisor;

	IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_0_reg, shpnr_1st_gain_0.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_1_reg, shpnr_1st_gain_1.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_2_reg, shpnr_1st_gain_2.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_3_reg, shpnr_1st_gain_3.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_4_reg, shpnr_1st_gain_4.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_5_reg, shpnr_1st_gain_5.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_6_reg, shpnr_1st_gain_6.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_7_reg, shpnr_1st_gain_7.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_8_reg, shpnr_1st_gain_8.regValue);

}

void drvif_color_set_LC_Shpnr_Gain2nd(DRV_LC_Shpnr_Gain2nd *ptr)
{
	lc_lc_local_shpnr_2nd_gain_0_RBUS shpnr_2nd_gain_0;
	lc_lc_local_shpnr_2nd_gain_1_RBUS shpnr_2nd_gain_1;
	lc_lc_local_shpnr_2nd_gain_2_RBUS shpnr_2nd_gain_2;
	lc_lc_local_shpnr_2nd_gain_3_RBUS shpnr_2nd_gain_3;
	lc_lc_local_shpnr_2nd_gain_4_RBUS shpnr_2nd_gain_4;
	lc_lc_local_shpnr_2nd_gain_5_RBUS shpnr_2nd_gain_5;
	lc_lc_local_shpnr_2nd_gain_6_RBUS shpnr_2nd_gain_6;
	lc_lc_local_shpnr_2nd_gain_7_RBUS shpnr_2nd_gain_7;
	lc_lc_local_shpnr_2nd_gain_8_RBUS shpnr_2nd_gain_8;

	shpnr_2nd_gain_0.regValue = IoReg_Read32(LC_LC_Local_Shpnr_2nd_gain_0_reg);
	shpnr_2nd_gain_1.regValue = IoReg_Read32(LC_LC_Local_Shpnr_2nd_gain_1_reg);
	shpnr_2nd_gain_2.regValue = IoReg_Read32(LC_LC_Local_Shpnr_2nd_gain_2_reg);
	shpnr_2nd_gain_3.regValue = IoReg_Read32(LC_LC_Local_Shpnr_2nd_gain_3_reg);
	shpnr_2nd_gain_4.regValue = IoReg_Read32(LC_LC_Local_Shpnr_2nd_gain_4_reg);
	shpnr_2nd_gain_5.regValue = IoReg_Read32(LC_LC_Local_Shpnr_2nd_gain_5_reg);
	shpnr_2nd_gain_6.regValue = IoReg_Read32(LC_LC_Local_Shpnr_2nd_gain_6_reg);
	shpnr_2nd_gain_7.regValue = IoReg_Read32(LC_LC_Local_Shpnr_2nd_gain_7_reg);
	shpnr_2nd_gain_8.regValue = IoReg_Read32(LC_LC_Local_Shpnr_2nd_gain_8_reg);

	shpnr_2nd_gain_0.lc_gain_by_ydiff_0 = ptr->lc_gain_by_ydiff_0;
	shpnr_2nd_gain_0.lc_gain_by_ydiff_1 = ptr->lc_gain_by_ydiff_1;
	shpnr_2nd_gain_0.lc_gain_by_ydiff_2 = ptr->lc_gain_by_ydiff_2;
	shpnr_2nd_gain_0.lc_gain_by_ydiff_3 = ptr->lc_gain_by_ydiff_3;
	shpnr_2nd_gain_1.lc_gain_by_ydiff_4 = ptr->lc_gain_by_ydiff_4;
	shpnr_2nd_gain_1.lc_gain_by_ydiff_5 = ptr->lc_gain_by_ydiff_5;
	shpnr_2nd_gain_1.lc_gain_by_ydiff_6 = ptr->lc_gain_by_ydiff_6;
	shpnr_2nd_gain_1.lc_gain_by_ydiff_7 = ptr->lc_gain_by_ydiff_7;
	shpnr_2nd_gain_2.lc_gain_by_ydiff_8 = ptr->lc_gain_by_ydiff_8;
	shpnr_2nd_gain_2.lc_gain_by_ydiff_9 = ptr->lc_gain_by_ydiff_9;
	shpnr_2nd_gain_2.lc_gain_by_ydiff_10 = ptr->lc_gain_by_ydiff_10;
	shpnr_2nd_gain_2.lc_gain_by_ydiff_11 = ptr->lc_gain_by_ydiff_11;
	shpnr_2nd_gain_3.lc_gain_by_ydiff_12 = ptr->lc_gain_by_ydiff_12;
	shpnr_2nd_gain_3.lc_gain_by_ydiff_13 = ptr->lc_gain_by_ydiff_13;
	shpnr_2nd_gain_3.lc_gain_by_ydiff_14 = ptr->lc_gain_by_ydiff_14;
	shpnr_2nd_gain_3.lc_gain_by_ydiff_15 = ptr->lc_gain_by_ydiff_15;
	shpnr_2nd_gain_4.lc_gain_by_ydiff_16 = ptr->lc_gain_by_ydiff_16;
	shpnr_2nd_gain_4.lc_gain_by_ydiff_17 = ptr->lc_gain_by_ydiff_17;
	shpnr_2nd_gain_4.lc_gain_by_ydiff_18 = ptr->lc_gain_by_ydiff_18;
	shpnr_2nd_gain_4.lc_gain_by_ydiff_19 = ptr->lc_gain_by_ydiff_19;
	shpnr_2nd_gain_5.lc_gain_by_ydiff_20 = ptr->lc_gain_by_ydiff_20;
	shpnr_2nd_gain_5.lc_gain_by_ydiff_21 = ptr->lc_gain_by_ydiff_21;
	shpnr_2nd_gain_5.lc_gain_by_ydiff_22 = ptr->lc_gain_by_ydiff_22;
	shpnr_2nd_gain_5.lc_gain_by_ydiff_23 = ptr->lc_gain_by_ydiff_23;
	shpnr_2nd_gain_6.lc_gain_by_ydiff_24 = ptr->lc_gain_by_ydiff_24;
	shpnr_2nd_gain_6.lc_gain_by_ydiff_25 = ptr->lc_gain_by_ydiff_25;
	shpnr_2nd_gain_6.lc_gain_by_ydiff_26 = ptr->lc_gain_by_ydiff_26;
	shpnr_2nd_gain_6.lc_gain_by_ydiff_27 = ptr->lc_gain_by_ydiff_27;
	shpnr_2nd_gain_7.lc_gain_by_ydiff_28 = ptr->lc_gain_by_ydiff_28;
	shpnr_2nd_gain_7.lc_gain_by_ydiff_29 = ptr->lc_gain_by_ydiff_29;
	shpnr_2nd_gain_7.lc_gain_by_ydiff_30 = ptr->lc_gain_by_ydiff_30;
	shpnr_2nd_gain_7.lc_gain_by_ydiff_31 = ptr->lc_gain_by_ydiff_31;
	shpnr_2nd_gain_8.lc_gain_by_ydiff_32 = ptr->lc_gain_by_ydiff_32;
	shpnr_2nd_gain_8.lc_gain_by_ydiff_divisor = ptr->lc_gain_by_ydiff_divisor;

	IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_0_reg, shpnr_2nd_gain_0.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_1_reg, shpnr_2nd_gain_1.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_2_reg, shpnr_2nd_gain_2.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_3_reg, shpnr_2nd_gain_3.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_4_reg, shpnr_2nd_gain_4.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_5_reg, shpnr_2nd_gain_5.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_6_reg, shpnr_2nd_gain_6.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_7_reg, shpnr_2nd_gain_7.regValue);
	IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_8_reg, shpnr_2nd_gain_8.regValue);

}


void drvif_color_set_LC_Diff_Ctrl0(DRV_LC_Diff_Ctrl0 *ptr)
{
	lc_lc_diff_ctrl0_RBUS diff_ctrl0;

	diff_ctrl0.regValue = IoReg_Read32(LC_LC_diff_ctrl0_reg);

	diff_ctrl0.lc_ydiff_measure_en = ptr->lc_ydiff_measure_en;
	diff_ctrl0.lc_ydiff_abs_th = ptr->lc_ydiff_abs_th;

	IoReg_Write32(LC_LC_diff_ctrl0_reg, diff_ctrl0.regValue);
}

//juwen, add LC : tone mapping curve
void drvif_color_set_LC_ToneMappingSlopePoint(unsigned int *ptr)
{
	int i = 0;
	lc_lc_tonemapping_sram_addr_RBUS lc_tmap_curve_address;//0xb802E130
	lc_lc_tonemapping_sram_data_RBUS lc_tmap_slope_point;//0xb802E134
	lc_lc_global_ctrl1_RBUS blu_lc_global_ctrl1;

	lc_tmap_curve_address.regValue = IoReg_Read32(LC_LC_ToneMapping_SRAM_Addr_reg);//
	lc_tmap_slope_point.regValue = IoReg_Read32(LC_LC_ToneMapping_SRAM_Data_reg);//

	lc_tmap_curve_address.regValue = 0; //
	IoReg_Write32(LC_LC_ToneMapping_SRAM_Addr_reg, lc_tmap_curve_address.regValue);

	for(i=0;i<LC_Curve_ToneM_PointSlope;i++)
	{
		lc_tmap_slope_point.regValue = *(ptr+i);
		IoReg_Write32(LC_LC_ToneMapping_SRAM_Data_reg, lc_tmap_slope_point.regValue);
	}

	blu_lc_global_ctrl1.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	blu_lc_global_ctrl1.lc_db_apply = 1;
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, blu_lc_global_ctrl1.regValue);

}

void drvif_color_set_LC_DebugMode(unsigned char DebugMode)
{
	lc_lc_global_ctrl0_RBUS blu_lc_global_ctrl0;
	blu_lc_global_ctrl0.regValue = IoReg_Read32(LC_LC_Global_Ctrl0_reg);
	blu_lc_global_ctrl0.lc_debugmode = DebugMode;
	IoReg_Write32(LC_LC_Global_Ctrl0_reg, blu_lc_global_ctrl0.regValue);
}

void drvif_color_set_HV_Num_Fac_Size(VIP_DRV_Local_Contrast_Region_Num_Size *LC_Region_Num_Size)
{
	lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;
	lc_lc_global_ctrl3_RBUS blu_lc_global_ctrl3;
	lc_lc_inter_hctrl1_RBUS blu_lc_inter_hctrl1;
	lc_lc_inter_vctrl1_RBUS blu_lc_inter_vctrl1;
	lc_lc_global_ctrl1_RBUS blu_lc_global_ctrl1;

	blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);
	blu_lc_global_ctrl3.regValue = IoReg_Read32(LC_LC_Global_Ctrl3_reg);
	blu_lc_inter_hctrl1.regValue = IoReg_Read32(LC_LC_Inter_HCtrl1_reg);
	blu_lc_inter_vctrl1.regValue = IoReg_Read32(LC_LC_Inter_VCtrl1_reg);

	blu_lc_global_ctrl2.lc_valid = 1;
	blu_lc_global_ctrl2.lc_blk_hnum = LC_Region_Num_Size->nBlk_Hnum;
	blu_lc_global_ctrl2.lc_blk_vnum = LC_Region_Num_Size->nBlk_Vnum;
	blu_lc_global_ctrl3.lc_blk_hsize = LC_Region_Num_Size->nBlk_Hsize;
	blu_lc_global_ctrl3.lc_blk_vsize = LC_Region_Num_Size->nBlk_Vsize;
	blu_lc_inter_hctrl1.lc_hfactor = LC_Region_Num_Size->lc_hfactor;
	blu_lc_inter_vctrl1.lc_vfactor = LC_Region_Num_Size->lc_vfactor;


	IoReg_Write32(LC_LC_Global_Ctrl2_reg, blu_lc_global_ctrl2.regValue);
	IoReg_Write32(LC_LC_Global_Ctrl3_reg, blu_lc_global_ctrl3.regValue);
	IoReg_Write32(LC_LC_Inter_HCtrl1_reg, blu_lc_inter_hctrl1.regValue);
	IoReg_Write32(LC_LC_Inter_VCtrl1_reg, blu_lc_inter_vctrl1.regValue);

	/*merlin5, num, size, factor apply double buffer*/
	blu_lc_global_ctrl1.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	blu_lc_global_ctrl1.lc_db_apply = 1;
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, blu_lc_global_ctrl1.regValue);

}

void drvif_color_get_LC_APL_GSR(unsigned short *ptr_out)
{

    int i=0, num_hblk=0, num_vblk=0, num_allBlk=0, read_times=0;

    lc_lc_global_ctrl0_RBUS lc_global_ctrl0;
    lc_lc_apl_read_data_RBUS lc_apl_read_data;
    lc_lc_apl_write_addr_RBUS lc_apl_write_addr;
    lc_lc_global_ctrl1_RBUS blu_lc_global_ctrl1;
    lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;

    lc_global_ctrl0.regValue = IoReg_Read32(LC_LC_Global_Ctrl0_reg);
    lc_apl_read_data.regValue = IoReg_Read32(LC_LC_APL_READ_DATA_reg);
    lc_apl_write_addr.regValue = IoReg_Read32(LC_LC_APL_write_addr_reg);
    blu_lc_global_ctrl1.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
    blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);

	num_hblk = blu_lc_global_ctrl2.lc_blk_hnum+1;
	num_vblk = blu_lc_global_ctrl2.lc_blk_vnum+1;
	num_allBlk = num_hblk * num_vblk;
	read_times = ((num_allBlk%2)==0)? num_allBlk/2 : (num_allBlk/2)+1;

	//set register value
	lc_global_ctrl0.lc_apl_en = 1;
	lc_apl_write_addr.lc_apl_addr = 0; /*merlin5, set lc_apl_addr*/
	blu_lc_global_ctrl1.lc_db_apply = 1;

	IoReg_Write32(LC_LC_Global_Ctrl0_reg, lc_global_ctrl0.regValue );
	IoReg_Write32(LC_LC_APL_write_addr_reg, lc_apl_read_data.regValue );
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, blu_lc_global_ctrl1.regValue);

	for(i=0;i<read_times;i++)
	{
		lc_apl_read_data.regValue = IoReg_Read32(LC_LC_APL_READ_DATA_reg);
		ptr_out[(i*2)]   = lc_apl_read_data.lc_apl_dat_lsb;/*merlin5, apl 11 bit*/
		ptr_out[(i*2)+1] = lc_apl_read_data.lc_apl_dat_msb;/*merlin5, apl 11 bit*/
	}

}

void drvif_color_set_LC_saturation(DRV_LC_saturation *ptr)
{
	lc_lc_saturation_ctrl_RBUS saturation_ctrl;
	lc_lc_saturation_gain_RBUS saturation_gain;
	lc_lc_saturation_rgb2yuv_0_RBUS saturation_rgb2yuv_0;
	lc_lc_saturation_rgb2yuv_1_RBUS saturation_rgb2yuv_1;
	lc_lc_saturation_rgb2yuv_2_RBUS saturation_rgb2yuv_2;
	lc_lc_saturation_rgb2yuv_3_RBUS saturation_rgb2yuv_3;
	lc_lc_saturation_rgb2yuv_4_RBUS saturation_rgb2yuv_4;
	lc_lc_saturation_yuv2rgb_0_RBUS saturation_yuv2rgb_0;
	lc_lc_saturation_yuv2rgb_1_RBUS saturation_yuv2rgb_1;
	lc_lc_saturation_yuv2rgb_2_RBUS saturation_yuv2rgb_2;
	lc_lc_saturation_yuv2rgb_3_RBUS saturation_yuv2rgb_3;
	DRV_LC_saturation_r2y LC_saturation_r2y_tmp;
	DRV_LC_saturation_y2r LC_saturation_y2r_tmp;

	saturation_ctrl.regValue = IoReg_Read32(LC_LC_Saturation_ctrl_reg);
	saturation_gain.regValue = IoReg_Read32(LC_LC_Saturation_Gain_reg);
	saturation_rgb2yuv_0.regValue = IoReg_Read32(LC_LC_Saturation_rgb2yuv_0_reg);
	saturation_rgb2yuv_1.regValue = IoReg_Read32(LC_LC_Saturation_rgb2yuv_1_reg);
	saturation_rgb2yuv_2.regValue = IoReg_Read32(LC_LC_Saturation_rgb2yuv_2_reg);
	saturation_rgb2yuv_3.regValue = IoReg_Read32(LC_LC_Saturation_rgb2yuv_3_reg);
	saturation_rgb2yuv_4.regValue = IoReg_Read32(LC_LC_Saturation_rgb2yuv_4_reg);
	saturation_yuv2rgb_0.regValue = IoReg_Read32(LC_LC_Saturation_yuv2rgb_0_reg);
	saturation_yuv2rgb_1.regValue = IoReg_Read32(LC_LC_Saturation_yuv2rgb_1_reg);
	saturation_yuv2rgb_2.regValue = IoReg_Read32(LC_LC_Saturation_yuv2rgb_2_reg);
	saturation_yuv2rgb_3.regValue = IoReg_Read32(LC_LC_Saturation_yuv2rgb_3_reg);

	saturation_ctrl.lc_saturation_en = ptr->LC_saturation_en;
	saturation_ctrl.lc_no_saturation_enhance_thl = ptr->LC_no_saturation_enhance_thl;
	saturation_ctrl.lc_saturation_blend_factor = ptr->LC_saturation_blend_factor;
	saturation_gain.lc_saturation_gain = ptr->LC_saturation_Gain;

	LC_saturation_r2y_tmp = ptr->LC_saturation_r2y;//wrong ??????
	saturation_rgb2yuv_0.lc_saturation_rgb2yuv_m11 = LC_saturation_r2y_tmp.LC_saturation_rgb2yuv_m11;
	saturation_rgb2yuv_0.lc_saturation_rgb2yuv_m12 = LC_saturation_r2y_tmp.LC_saturation_rgb2yuv_m12;
	saturation_rgb2yuv_1.lc_saturation_rgb2yuv_m13 = LC_saturation_r2y_tmp.LC_saturation_rgb2yuv_m13;
	saturation_rgb2yuv_1.lc_saturation_rgb2yuv_m21 = LC_saturation_r2y_tmp.LC_saturation_rgb2yuv_m21;
	saturation_rgb2yuv_2.lc_saturation_rgb2yuv_m22 = LC_saturation_r2y_tmp.LC_saturation_rgb2yuv_m22;
	saturation_rgb2yuv_2.lc_saturation_rgb2yuv_m23 = LC_saturation_r2y_tmp.LC_saturation_rgb2yuv_m23;
	saturation_rgb2yuv_3.lc_saturation_rgb2yuv_m31 = LC_saturation_r2y_tmp.LC_saturation_rgb2yuv_m31;
	saturation_rgb2yuv_3.lc_saturation_rgb2yuv_m32 = LC_saturation_r2y_tmp.LC_saturation_rgb2yuv_m32;
	saturation_rgb2yuv_4.lc_saturation_rgb2yuv_m33 = LC_saturation_r2y_tmp.LC_saturation_rgb2yuv_m33;

	LC_saturation_y2r_tmp = ptr->LC_saturation_y2r;//wrong ??????
	saturation_yuv2rgb_0.lc_saturation_yuv2rgb_k11 = LC_saturation_y2r_tmp.LC_saturation_yuv2rgb_k11;
	saturation_yuv2rgb_0.lc_saturation_yuv2rgb_k12 = LC_saturation_y2r_tmp.LC_saturation_yuv2rgb_k12;
	saturation_yuv2rgb_1.lc_saturation_yuv2rgb_k13 = LC_saturation_y2r_tmp.LC_saturation_yuv2rgb_k13;
	saturation_yuv2rgb_1.lc_saturation_yuv2rgb_k22 = LC_saturation_y2r_tmp.LC_saturation_yuv2rgb_k22;
	saturation_yuv2rgb_2.lc_saturation_yuv2rgb_k23 = LC_saturation_y2r_tmp.LC_saturation_yuv2rgb_k23;
	saturation_yuv2rgb_2.lc_saturation_yuv2rgb_k32 = LC_saturation_y2r_tmp.LC_saturation_yuv2rgb_k32;
	saturation_yuv2rgb_3.lc_saturation_yuv2rgb_k33 = LC_saturation_y2r_tmp.LC_saturation_yuv2rgb_k33;

	IoReg_Write32(LC_LC_Saturation_ctrl_reg, saturation_ctrl.regValue );
	IoReg_Write32(LC_LC_Saturation_Gain_reg, saturation_gain.regValue );
	IoReg_Write32(LC_LC_Saturation_rgb2yuv_0_reg, saturation_rgb2yuv_0.regValue );
	IoReg_Write32(LC_LC_Saturation_rgb2yuv_1_reg, saturation_rgb2yuv_1.regValue );
	IoReg_Write32(LC_LC_Saturation_rgb2yuv_2_reg, saturation_rgb2yuv_2.regValue );
	IoReg_Write32(LC_LC_Saturation_rgb2yuv_3_reg, saturation_rgb2yuv_3.regValue );
	IoReg_Write32(LC_LC_Saturation_rgb2yuv_4_reg, saturation_rgb2yuv_4.regValue );
	IoReg_Write32(LC_LC_Saturation_yuv2rgb_0_reg, saturation_yuv2rgb_0.regValue );
	IoReg_Write32(LC_LC_Saturation_yuv2rgb_1_reg, saturation_yuv2rgb_1.regValue );
	IoReg_Write32(LC_LC_Saturation_yuv2rgb_2_reg, saturation_yuv2rgb_2.regValue );
	IoReg_Write32(LC_LC_Saturation_yuv2rgb_3_reg, saturation_yuv2rgb_3.regValue );


}

/*juwen, k5lp, add*/
void drvif_color_set_LC_avoid_flicker(DRV_LC_avoid_flicker *ptr)
{
#if 0 // removed at K7Lp
	DRV_LC_avoid_flicker_diff_thl LC_avoid_flicker_diff_thl_tmp;
	DRV_LC_avoid_flicker_ratio_thl LC_avoid_flicker_ratio_thl_tmp;

	lc_lc_tmap_flicker_diff_counter_RBUS tmap_flicker_diff_counter;
	lc_lc_tmap_flicker_slow_add_thl_RBUS tmap_flicker_slow_add_thl;

	lc_lc_tmap_flicker_blend_factor_0_RBUS tmap_flicker_blend_factor_0;
	lc_lc_tmap_flicker_blend_factor_1_RBUS tmap_flicker_blend_factor_1;
	lc_lc_tmap_flicker_blend_factor_2_RBUS tmap_flicker_blend_factor_2;
	lc_lc_tmap_flicker_blend_factor_3_RBUS tmap_flicker_blend_factor_3;
	lc_lc_tmap_flicker_blend_factor_4_RBUS tmap_flicker_blend_factor_4;
	lc_lc_tmap_flicker_blend_factor_5_RBUS tmap_flicker_blend_factor_5;
	lc_lc_tmap_flicker_blend_factor_6_RBUS tmap_flicker_blend_factor_6;
	lc_lc_tmap_flicker_blend_factor_7_RBUS tmap_flicker_blend_factor_7;

	lc_lc_tmap_flicker_diff_thl_0_RBUS tmap_flicker_diff_thl_0;
	lc_lc_tmap_flicker_diff_thl_1_RBUS tmap_flicker_diff_thl_1;
	lc_lc_tmap_flicker_diff_thl_2_RBUS tmap_flicker_diff_thl_2;
	lc_lc_tmap_flicker_diff_thl_3_RBUS tmap_flicker_diff_thl_3;
	lc_lc_tmap_flicker_diff_thl_4_RBUS tmap_flicker_diff_thl_4;
	lc_lc_tmap_flicker_diff_thl_5_RBUS tmap_flicker_diff_thl_5;

	tmap_flicker_diff_counter.regValue   = IoReg_Read32(LC_LC_Tmap_flicker_diff_counter_reg);
	tmap_flicker_slow_add_thl.regValue   = IoReg_Read32(LC_LC_Tmap_flicker_slow_add_thl_reg);
	tmap_flicker_blend_factor_0.regValue = IoReg_Read32(LC_LC_Tmap_flicker_blend_factor_0_reg);
	tmap_flicker_blend_factor_1.regValue = IoReg_Read32(LC_LC_Tmap_flicker_blend_factor_1_reg);
	tmap_flicker_blend_factor_2.regValue = IoReg_Read32(LC_LC_Tmap_flicker_blend_factor_2_reg);
	tmap_flicker_blend_factor_3.regValue = IoReg_Read32(LC_LC_Tmap_flicker_blend_factor_3_reg);
	tmap_flicker_blend_factor_4.regValue = IoReg_Read32(LC_LC_Tmap_flicker_blend_factor_4_reg);
	tmap_flicker_blend_factor_5.regValue = IoReg_Read32(LC_LC_Tmap_flicker_blend_factor_5_reg);
	tmap_flicker_blend_factor_6.regValue = IoReg_Read32(LC_LC_Tmap_flicker_blend_factor_6_reg);
	tmap_flicker_blend_factor_7.regValue = IoReg_Read32(LC_LC_Tmap_flicker_blend_factor_7_reg);

	tmap_flicker_diff_thl_0.regValue = IoReg_Read32(LC_LC_Tmap_flicker_diff_thl_0_reg);
	tmap_flicker_diff_thl_1.regValue = IoReg_Read32(LC_LC_Tmap_flicker_diff_thl_1_reg);
	tmap_flicker_diff_thl_2.regValue = IoReg_Read32(LC_LC_Tmap_flicker_diff_thl_2_reg);
	tmap_flicker_diff_thl_3.regValue = IoReg_Read32(LC_LC_Tmap_flicker_diff_thl_3_reg);
	tmap_flicker_diff_thl_4.regValue = IoReg_Read32(LC_LC_Tmap_flicker_diff_thl_4_reg);
	tmap_flicker_diff_thl_5.regValue = IoReg_Read32(LC_LC_Tmap_flicker_diff_thl_5_reg);

	tmap_flicker_diff_counter.lc_tmap_avoid_flicker_en= ptr->lc_tmap_avoid_flicker_en;
	//tmap_flicker_diff_counter.lc_tmap_diff_counter   = ptr->lc_tmap_diff_counter;//read only
	tmap_flicker_slow_add_thl.lc_tmap_slow_add_thl = ptr->lc_tmap_slow_add_thl;

	LC_avoid_flicker_diff_thl_tmp = ptr->LC_avoid_flicker_diff_thl;
	LC_avoid_flicker_ratio_thl_tmp = ptr->LC_avoid_flicker_ratio_thl;

	tmap_flicker_diff_thl_0.lc_tmap_diff_thl_0 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_0;
	tmap_flicker_diff_thl_0.lc_tmap_diff_thl_1 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_1;
	tmap_flicker_diff_thl_0.lc_tmap_diff_thl_2 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_2;

	tmap_flicker_diff_thl_1.lc_tmap_diff_thl_3 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_3;
	tmap_flicker_diff_thl_1.lc_tmap_diff_thl_4 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_4;
	tmap_flicker_diff_thl_1.lc_tmap_diff_thl_5 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_5;

	tmap_flicker_diff_thl_2.lc_tmap_diff_thl_6 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_6;
	tmap_flicker_diff_thl_2.lc_tmap_diff_thl_7 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_7;
	tmap_flicker_diff_thl_2.lc_tmap_diff_thl_8 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_8;

	tmap_flicker_diff_thl_3.lc_tmap_diff_thl_9 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_9;
	tmap_flicker_diff_thl_3.lc_tmap_diff_thl_10 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_10;
	tmap_flicker_diff_thl_3.lc_tmap_diff_thl_11 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_11;

	tmap_flicker_diff_thl_4.lc_tmap_diff_thl_12 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_12;
	tmap_flicker_diff_thl_4.lc_tmap_diff_thl_13 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_13;
	tmap_flicker_diff_thl_4.lc_tmap_diff_thl_14 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_14;

	tmap_flicker_diff_thl_5.lc_tmap_diff_thl_15 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_15;
	tmap_flicker_diff_thl_5.lc_tmap_diff_thl_16 = LC_avoid_flicker_diff_thl_tmp.lc_tmap_diff_thl_16;

	tmap_flicker_blend_factor_0.lc_tmap_blend_factor_ratio_0 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_ratio_0;
	tmap_flicker_blend_factor_0.lc_tmap_blend_factor_thl_0 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_thl_0;

	tmap_flicker_blend_factor_1.lc_tmap_blend_factor_ratio_1 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_ratio_1;
	tmap_flicker_blend_factor_1.lc_tmap_blend_factor_thl_1 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_thl_1;

	tmap_flicker_blend_factor_2.lc_tmap_blend_factor_ratio_2 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_ratio_2;
	tmap_flicker_blend_factor_2.lc_tmap_blend_factor_thl_2 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_thl_2;

	tmap_flicker_blend_factor_3.lc_tmap_blend_factor_ratio_3 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_ratio_3;
	tmap_flicker_blend_factor_3.lc_tmap_blend_factor_thl_3 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_thl_3;

	tmap_flicker_blend_factor_4.lc_tmap_blend_factor_ratio_4 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_ratio_4;
	tmap_flicker_blend_factor_4.lc_tmap_blend_factor_thl_4 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_thl_4;

	tmap_flicker_blend_factor_5.lc_tmap_blend_factor_ratio_5 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_ratio_5;
	tmap_flicker_blend_factor_5.lc_tmap_blend_factor_thl_5 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_thl_5;

	tmap_flicker_blend_factor_6.lc_tmap_blend_factor_ratio_6 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_ratio_6;
	tmap_flicker_blend_factor_6.lc_tmap_blend_factor_thl_6 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_thl_6;

	tmap_flicker_blend_factor_7.lc_tmap_blend_factor_ratio_7 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_ratio_7;
	tmap_flicker_blend_factor_7.lc_tmap_blend_factor_thl_7 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_thl_7;
	tmap_flicker_blend_factor_7.lc_tmap_blend_factor_ratio_8 = LC_avoid_flicker_ratio_thl_tmp.lc_tmap_blend_factor_ratio_8;

	//IoReg_Write32(LC_LC_Tmap_flicker_diff_counter_reg, tmap_flicker_diff_counter.regValue);//readonly
	IoReg_Write32(LC_LC_Tmap_flicker_slow_add_thl_reg, tmap_flicker_slow_add_thl.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_blend_factor_0_reg, tmap_flicker_blend_factor_0.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_blend_factor_1_reg, tmap_flicker_blend_factor_1.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_blend_factor_2_reg, tmap_flicker_blend_factor_2.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_blend_factor_3_reg, tmap_flicker_blend_factor_3.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_blend_factor_4_reg, tmap_flicker_blend_factor_4.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_blend_factor_5_reg, tmap_flicker_blend_factor_5.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_blend_factor_6_reg, tmap_flicker_blend_factor_6.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_blend_factor_7_reg, tmap_flicker_blend_factor_7.regValue);

	IoReg_Write32(LC_LC_Tmap_flicker_diff_thl_0_reg, tmap_flicker_diff_thl_0.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_diff_thl_1_reg, tmap_flicker_diff_thl_1.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_diff_thl_2_reg, tmap_flicker_diff_thl_2.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_diff_thl_3_reg, tmap_flicker_diff_thl_3.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_diff_thl_4_reg, tmap_flicker_diff_thl_4.regValue);
	IoReg_Write32(LC_LC_Tmap_flicker_diff_thl_5_reg, tmap_flicker_diff_thl_5.regValue);
#endif
}

// jimmy, LC decontour, k5lp
void drvif_color_set_LC_decontour(DRV_LC_decontour *ptr)
{
    lc_lc_decontour_alpha_0_RBUS lc_decontour_alpha_0;
    lc_lc_decontour_alpha_1_RBUS lc_decontour_alpha_1;
    lc_lc_decontour_alpha_2_RBUS lc_decontour_alpha_2;
    lc_lc_decontour_alpha_3_RBUS lc_decontour_alpha_3;

    lc_decontour_alpha_0.regValue = IoReg_Read32(LC_LC_Decontour_alpha_0_reg);
    lc_decontour_alpha_1.regValue = IoReg_Read32(LC_LC_Decontour_alpha_1_reg);
    lc_decontour_alpha_2.regValue = IoReg_Read32(LC_LC_Decontour_alpha_2_reg);
    lc_decontour_alpha_3.regValue = IoReg_Read32(LC_LC_Decontour_alpha_3_reg);

    lc_decontour_alpha_0.lc_decontour_en				= ptr->LC_decont_en;
    lc_decontour_alpha_0.lc_decont_ydiff_shif			= ptr->LC_decont_shift;
    lc_decontour_alpha_0.lc_decont_alpha_by_ydiff_0		= ptr->LC_decont_alpha_0;
    lc_decontour_alpha_0.lc_decont_alpha_by_ydiff_1		= ptr->LC_decont_alpha_1;
    lc_decontour_alpha_0.lc_decont_alpha_by_ydiff_2		= ptr->LC_decont_alpha_2;
    lc_decontour_alpha_0.lc_decont_alpha_by_ydiff_3		= ptr->LC_decont_alpha_3;
    lc_decontour_alpha_1.lc_decont_alpha_by_ydiff_4		= ptr->LC_decont_alpha_4;
    lc_decontour_alpha_1.lc_decont_alpha_by_ydiff_5		= ptr->LC_decont_alpha_5;
    lc_decontour_alpha_1.lc_decont_alpha_by_ydiff_6		= ptr->LC_decont_alpha_6;
    lc_decontour_alpha_1.lc_decont_alpha_by_ydiff_7		= ptr->LC_decont_alpha_7;
    lc_decontour_alpha_2.lc_decont_alpha_by_ydiff_8		= ptr->LC_decont_alpha_8;
    lc_decontour_alpha_2.lc_decont_alpha_by_ydiff_9		= ptr->LC_decont_alpha_9;
    lc_decontour_alpha_2.lc_decont_alpha_by_ydiff_10	= ptr->LC_decont_alpha_10;
    lc_decontour_alpha_2.lc_decont_alpha_by_ydiff_11	= ptr->LC_decont_alpha_11;
    lc_decontour_alpha_3.lc_decont_alpha_by_ydiff_12	= ptr->LC_decont_alpha_12;
    lc_decontour_alpha_3.lc_decont_alpha_by_ydiff_13	= ptr->LC_decont_alpha_13;
    lc_decontour_alpha_3.lc_decont_alpha_by_ydiff_14	= ptr->LC_decont_alpha_14;
    lc_decontour_alpha_3.lc_decont_alpha_by_ydiff_15	= ptr->LC_decont_alpha_15;

    IoReg_Write32(LC_LC_Decontour_alpha_0_reg, lc_decontour_alpha_0.regValue);
    IoReg_Write32(LC_LC_Decontour_alpha_1_reg, lc_decontour_alpha_1.regValue);
    IoReg_Write32(LC_LC_Decontour_alpha_2_reg, lc_decontour_alpha_2.regValue);
    IoReg_Write32(LC_LC_Decontour_alpha_3_reg, lc_decontour_alpha_3.regValue);
}


#if 0
/* LOCAL CONTRAST GET FUNCTIONS*/
#endif

void drvif_color_get_LC_ToneMappingSlopePoint(unsigned int *ptr_out)
{
	int i = 0;

	lc_lc_tonemapping_sram_addr_RBUS lc_tmap_curve_address;//0xb802E130
	lc_lc_tonemapping_sram_data_RBUS lc_tmap_slope_point;//0xb802E134

	lc_tmap_curve_address.regValue = IoReg_Read32(LC_LC_ToneMapping_SRAM_Addr_reg);//
	lc_tmap_slope_point.regValue     = IoReg_Read32(LC_LC_ToneMapping_SRAM_Data_reg);//

	lc_tmap_curve_address.regValue = 0; //
	IoReg_Write32(LC_LC_ToneMapping_SRAM_Addr_reg, lc_tmap_curve_address.regValue);

	for(i=0;i<LC_Curve_ToneM_PointSlope;i++)
	{
		ptr_out[i]=IoReg_Read32(LC_LC_ToneMapping_SRAM_Data_reg);
	}

}

char drvif_color_get_LC_Enable(void)
{
	color_d_vc_global_ctrl_RBUS color_d_vc_global_ctrl;
	color_d_vc_global_ctrl.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	return color_d_vc_global_ctrl.m_lc_comp_en;
}

char drvif_color_get_LC_DebugMode(void)
{
	lc_lc_global_ctrl0_RBUS blu_lc_global_ctrl0;
	blu_lc_global_ctrl0.regValue = IoReg_Read32(LC_LC_Global_Ctrl0_reg);
	return blu_lc_global_ctrl0.lc_debugmode;
}

void drvif_color_get_LC_APL(unsigned int *APL)
{
	int i = 0;
	int num_hblk = 0, num_vblk = 0, num_allBlk = 0, read_times = 0;

	lc_lc_global_ctrl0_RBUS lc_global_ctrl0;
	lc_lc_apl_read_data_RBUS lc_apl_read_data;
	lc_lc_apl_write_addr_RBUS lc_apl_write_addr;
	lc_lc_global_ctrl1_RBUS blu_lc_global_ctrl1;
	lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;

	lc_global_ctrl0.regValue = IoReg_Read32(LC_LC_Global_Ctrl0_reg);
	lc_apl_read_data.regValue = IoReg_Read32(LC_LC_APL_READ_DATA_reg);
	lc_apl_write_addr.regValue = IoReg_Read32(LC_LC_APL_write_addr_reg);
	blu_lc_global_ctrl1.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	blu_lc_global_ctrl2.regValue = IoReg_Read32(LC_LC_Global_Ctrl2_reg);

	num_hblk = blu_lc_global_ctrl2.lc_blk_hnum + 1 ;
	num_vblk = blu_lc_global_ctrl2.lc_blk_vnum + 1;
	num_allBlk = num_hblk * num_vblk;
	read_times = (num_allBlk%2) ? (num_allBlk/2) + 1 : (num_allBlk/2);

	//set register value
	lc_global_ctrl0.lc_apl_en = 1;
	lc_apl_write_addr.lc_apl_addr = 0;
	blu_lc_global_ctrl1.lc_db_apply = 1;

	IoReg_Write32(LC_LC_Global_Ctrl0_reg, lc_global_ctrl0.regValue);
	IoReg_Write32(LC_LC_APL_write_addr_reg, lc_apl_write_addr.regValue);
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, blu_lc_global_ctrl1.regValue);

	for(i=0;i<read_times;i++)
	{
		lc_apl_read_data.regValue = IoReg_Read32(LC_LC_APL_READ_DATA_reg);
		APL[(i*2)]   = lc_apl_read_data.lc_apl_dat_lsb; /*merlin5, apl 11 bit*/
		APL[(i*2)+1] = lc_apl_read_data.lc_apl_dat_msb; /*merlin5, apl 11 bit*/
	}
}

void drvif_color_get_LC_decontour(DRV_LC_decontour *ptr)
{
    lc_lc_decontour_alpha_0_RBUS lc_decontour_alpha_0;
    lc_lc_decontour_alpha_1_RBUS lc_decontour_alpha_1;
    lc_lc_decontour_alpha_2_RBUS lc_decontour_alpha_2;
    lc_lc_decontour_alpha_3_RBUS lc_decontour_alpha_3;

    lc_decontour_alpha_0.regValue = IoReg_Read32(LC_LC_Decontour_alpha_0_reg);
    lc_decontour_alpha_1.regValue = IoReg_Read32(LC_LC_Decontour_alpha_1_reg);
    lc_decontour_alpha_2.regValue = IoReg_Read32(LC_LC_Decontour_alpha_2_reg);
    lc_decontour_alpha_3.regValue = IoReg_Read32(LC_LC_Decontour_alpha_3_reg);

    ptr->LC_decont_en       = lc_decontour_alpha_0.lc_decontour_en;
    ptr->LC_decont_shift    = lc_decontour_alpha_0.lc_decont_ydiff_shif;
    ptr->LC_decont_alpha_0  = lc_decontour_alpha_0.lc_decont_alpha_by_ydiff_0;
    ptr->LC_decont_alpha_1  = lc_decontour_alpha_0.lc_decont_alpha_by_ydiff_1;
    ptr->LC_decont_alpha_2  = lc_decontour_alpha_0.lc_decont_alpha_by_ydiff_2;
    ptr->LC_decont_alpha_3  = lc_decontour_alpha_0.lc_decont_alpha_by_ydiff_3;
    ptr->LC_decont_alpha_4  = lc_decontour_alpha_1.lc_decont_alpha_by_ydiff_4;
    ptr->LC_decont_alpha_5  = lc_decontour_alpha_1.lc_decont_alpha_by_ydiff_5;
    ptr->LC_decont_alpha_6  = lc_decontour_alpha_1.lc_decont_alpha_by_ydiff_6;
    ptr->LC_decont_alpha_7  = lc_decontour_alpha_1.lc_decont_alpha_by_ydiff_7;
    ptr->LC_decont_alpha_8  = lc_decontour_alpha_2.lc_decont_alpha_by_ydiff_8;
    ptr->LC_decont_alpha_9  = lc_decontour_alpha_2.lc_decont_alpha_by_ydiff_9;
    ptr->LC_decont_alpha_10 = lc_decontour_alpha_2.lc_decont_alpha_by_ydiff_10;
    ptr->LC_decont_alpha_11 = lc_decontour_alpha_2.lc_decont_alpha_by_ydiff_11;
    ptr->LC_decont_alpha_12 = lc_decontour_alpha_3.lc_decont_alpha_by_ydiff_12;
    ptr->LC_decont_alpha_13 = lc_decontour_alpha_3.lc_decont_alpha_by_ydiff_13;
    ptr->LC_decont_alpha_14 = lc_decontour_alpha_3.lc_decont_alpha_by_ydiff_14;
    ptr->LC_decont_alpha_15 = lc_decontour_alpha_3.lc_decont_alpha_by_ydiff_15;
}

unsigned char drvif_color_get_LC_size_support()
{
	return u1_SizeValid;
}

void drvif_color_set_LC_size_support(unsigned char u1_support)
{
	u1_SizeValid = u1_support;
}

void drvif_color_get_LC_Histogram(unsigned int *plc_out)
{

//declare
	lc_lc_global_hist1_RBUS  lc_global_hist1;
	lc_lc_global_hist2_RBUS  lc_global_hist2;
	lc_lc_global_hist3_RBUS  lc_global_hist3;
	lc_lc_global_hist4_RBUS  lc_global_hist4;
	lc_lc_global_hist5_RBUS  lc_global_hist5;
	lc_lc_global_hist6_RBUS  lc_global_hist6;
	lc_lc_global_hist7_RBUS  lc_global_hist7;
	lc_lc_global_hist8_RBUS  lc_global_hist8;

//IoReg_Read32
	lc_global_hist1.regValue = IoReg_Read32(LC_LC_Global_Hist1_reg);
	lc_global_hist2.regValue = IoReg_Read32(LC_LC_Global_Hist2_reg);
	lc_global_hist3.regValue = IoReg_Read32(LC_LC_Global_Hist3_reg);
	lc_global_hist4.regValue = IoReg_Read32(LC_LC_Global_Hist4_reg);
	lc_global_hist5.regValue = IoReg_Read32(LC_LC_Global_Hist5_reg);
	lc_global_hist6.regValue = IoReg_Read32(LC_LC_Global_Hist6_reg);
	lc_global_hist7.regValue = IoReg_Read32(LC_LC_Global_Hist7_reg);
	lc_global_hist8.regValue = IoReg_Read32(LC_LC_Global_Hist8_reg);

	plc_out[0]= lc_global_hist1.lc_global_hist0;
	plc_out[1]= lc_global_hist1.lc_global_hist1;

	plc_out[2]= lc_global_hist2.lc_global_hist2;
	plc_out[3]= lc_global_hist2.lc_global_hist3;

	plc_out[4]= lc_global_hist3.lc_global_hist4;
	plc_out[5]= lc_global_hist3.lc_global_hist5;

	plc_out[6]= lc_global_hist4.lc_global_hist6;
	plc_out[7]= lc_global_hist4.lc_global_hist7;

	plc_out[8]= lc_global_hist5.lc_global_hist8;
	plc_out[9]= lc_global_hist5.lc_global_hist9;

	plc_out[10]= lc_global_hist6.lc_global_hist10;
	plc_out[11]= lc_global_hist6.lc_global_hist11;

	plc_out[12]= lc_global_hist7.lc_global_hist12;
	plc_out[13]= lc_global_hist7.lc_global_hist13;

	plc_out[14]= lc_global_hist8.lc_global_hist14;
	plc_out[15]= lc_global_hist8.lc_global_hist15;
}

#if 0
/* SOFTWARE LOCAL CONTRAST SET FUNCTIONS*/
#endif

void drvif_color_set_SLC_Hist_Enable(unsigned char bEnable)
{
#ifdef VIP_SUPPORT_SLC
	lc_slc_global_ctrl0_RBUS lc_slc_global_ctrl0_reg;
	lc_slc_global_db_ctrl_RBUS lc_slc_global_db_ctrl_reg;

	lc_slc_global_ctrl0_reg.regValue = IoReg_Read32(LC_SLC_Global_Ctrl0_reg);
	lc_slc_global_ctrl0_reg.slc_histenable = bEnable;
	IoReg_Write32(LC_SLC_Global_Ctrl0_reg, lc_slc_global_ctrl0_reg.regValue);

	// double buffer apply
	lc_slc_global_db_ctrl_reg.regValue = IoReg_Read32(LC_SLC_Global_Db_Ctrl_reg);
	lc_slc_global_db_ctrl_reg.slc_db_en = 1;
	lc_slc_global_db_ctrl_reg.slc_db_read_level = 1;
	lc_slc_global_db_ctrl_reg.slc_db_apply = 1;
	IoReg_Write32(LC_SLC_Global_Db_Ctrl_reg, lc_slc_global_db_ctrl_reg.regValue);
#endif	
}

void drvif_color_set_SLC_Hist_Info(DRV_SLC_Histogram_Info* ptr)
{
#ifdef VIP_SUPPORT_SLC
	lc_slc_global_ctrl0_RBUS lc_slc_global_ctrl0_reg;
	lc_slc_global_ctrl2_RBUS lc_slc_global_ctrl2_reg;
	lc_slc_global_db_ctrl_RBUS lc_slc_global_db_ctrl_reg;

	lc_slc_global_ctrl0_reg.regValue = IoReg_Read32(LC_SLC_Global_Ctrl0_reg);
	lc_slc_global_ctrl0_reg.slc_histformatsel = ptr->FormatSel;
	lc_slc_global_ctrl0_reg.slc_histpixelsel = ptr->PxlSel;
	lc_slc_global_ctrl0_reg.slc_histrangesel = ptr->RangeSel;
	lc_slc_global_ctrl0_reg.slc_histrightshiftbits = ptr->RightSftBits;
	IoReg_Write32(LC_SLC_Global_Ctrl0_reg, lc_slc_global_ctrl0_reg.regValue);

	lc_slc_global_ctrl2_reg.regValue = IoReg_Read32(LC_SLC_Global_Ctrl2_reg);
	lc_slc_global_ctrl2_reg.slc_huehistoffset = ptr->HueOffset;
	lc_slc_global_ctrl2_reg.slc_huehist_satlowbound = ptr->SatLowBnd;
	IoReg_Write32(LC_SLC_Global_Ctrl2_reg, lc_slc_global_ctrl2_reg.regValue);

	// double buffer apply
	lc_slc_global_db_ctrl_reg.regValue = IoReg_Read32(LC_SLC_Global_Db_Ctrl_reg);
	lc_slc_global_db_ctrl_reg.slc_db_en = 1;
	lc_slc_global_db_ctrl_reg.slc_db_read_level = 1;
	lc_slc_global_db_ctrl_reg.slc_db_apply = 1;
	IoReg_Write32(LC_SLC_Global_Db_Ctrl_reg, lc_slc_global_db_ctrl_reg.regValue);
#endif
}

void drvif_color_set_SLC_Hist_Size(DRV_SLC_Histogram_Size* ptr)
{
#ifdef VIP_SUPPORT_SLC
	lc_slc_global_ctrl0_RBUS lc_slc_global_ctrl0_reg;
	lc_slc_global_ctrl1_RBUS lc_slc_global_ctrl1_reg;
	lc_slc_global_db_ctrl_RBUS lc_slc_global_db_ctrl_reg;

	lc_slc_global_ctrl0_reg.regValue = IoReg_Read32(LC_SLC_Global_Ctrl0_reg);
	lc_slc_global_ctrl0_reg.slc_horblknum = ptr->HorBlkNum;
	lc_slc_global_ctrl0_reg.slc_verblknum = ptr->VerBlkNum;
	IoReg_Write32(LC_SLC_Global_Ctrl0_reg, lc_slc_global_ctrl0_reg.regValue);

	lc_slc_global_ctrl1_reg.regValue = IoReg_Read32(LC_SLC_Global_Ctrl1_reg);
	lc_slc_global_ctrl1_reg.slc_horblksize = ptr->HorBlkSize;
	lc_slc_global_ctrl1_reg.slc_verblksize = ptr->VerBlkSize;
	IoReg_Write32(LC_SLC_Global_Ctrl1_reg, lc_slc_global_ctrl1_reg.regValue);

	// double buffer apply
	lc_slc_global_db_ctrl_reg.regValue = IoReg_Read32(LC_SLC_Global_Db_Ctrl_reg);
	lc_slc_global_db_ctrl_reg.slc_db_en = 1;
	lc_slc_global_db_ctrl_reg.slc_db_read_level = 1;
	lc_slc_global_db_ctrl_reg.slc_db_apply = 1;
	IoReg_Write32(LC_SLC_Global_Db_Ctrl_reg, lc_slc_global_db_ctrl_reg.regValue);
#endif	
}

void drvif_color_set_SLC_Curve_Enable(unsigned char bEnable)
{
#ifdef VIP_SUPPORT_SLC
	lc_slc_global_ctrl0_RBUS lc_slc_global_ctrl0_reg;
	lc_slc_global_db_ctrl_RBUS lc_slc_global_db_ctrl_reg;

	lc_slc_global_ctrl0_reg.regValue = IoReg_Read32(LC_SLC_Global_Ctrl0_reg);
	lc_slc_global_ctrl0_reg.slc_curveenable= bEnable;
	IoReg_Write32(LC_SLC_Global_Ctrl0_reg, lc_slc_global_ctrl0_reg.regValue);

	// double buffer apply
	lc_slc_global_db_ctrl_reg.regValue = IoReg_Read32(LC_SLC_Global_Db_Ctrl_reg);
	lc_slc_global_db_ctrl_reg.slc_db_en = 1;
	lc_slc_global_db_ctrl_reg.slc_db_read_level = 1;
	lc_slc_global_db_ctrl_reg.slc_db_apply = 1;
	IoReg_Write32(LC_SLC_Global_Db_Ctrl_reg, lc_slc_global_db_ctrl_reg.regValue);
#endif	
}

void drvif_color_set_SLC_Curve_Size(DRV_SLC_Curve_Size *ptr)
{
#ifdef VIP_SUPPORT_SLC
	lc_slc_global_db_ctrl_RBUS lc_slc_global_db_ctrl_reg;
	lc_slc_inter_ctrl0_RBUS lc_slc_inter_ctrl0_reg;
	lc_slc_inter_ctrl1_RBUS lc_slc_inter_ctrl1_reg;
	lc_slc_inter_ctrl2_RBUS lc_slc_inter_ctrl2_reg;
	lc_slc_inter_ctrl3_RBUS lc_slc_inter_ctrl3_reg;

	lc_slc_inter_ctrl0_reg.regValue = IoReg_Read32(LC_SLC_Inter_Ctrl0_reg);
	lc_slc_inter_ctrl0_reg.slc_hfactor = ptr->H_Factor;
	IoReg_Write32(LC_SLC_Inter_Ctrl0_reg, lc_slc_inter_ctrl0_reg.regValue);

	lc_slc_inter_ctrl1_reg.regValue = IoReg_Read32(LC_SLC_Inter_Ctrl1_reg);
	lc_slc_inter_ctrl1_reg.slc_hinitphase1 = ptr->H_InitPhase1;
	lc_slc_inter_ctrl1_reg.slc_hinitphase2 = ptr->H_InitPhase2;
	IoReg_Write32(LC_SLC_Inter_Ctrl1_reg, lc_slc_inter_ctrl1_reg.regValue);

	lc_slc_inter_ctrl2_reg.regValue = IoReg_Read32(LC_SLC_Inter_Ctrl2_reg);
	lc_slc_inter_ctrl2_reg.slc_vfactor = ptr->V_Factor;
	IoReg_Write32(LC_SLC_Inter_Ctrl2_reg, lc_slc_inter_ctrl2_reg.regValue);

	lc_slc_inter_ctrl3_reg.regValue = IoReg_Read32(LC_SLC_Inter_Ctrl3_reg);
	lc_slc_inter_ctrl3_reg.slc_vinitphase1 = ptr->V_InitPhase1;
	lc_slc_inter_ctrl3_reg.slc_vinitphase2 = ptr->V_InitPhase2;
	IoReg_Write32(LC_SLC_Inter_Ctrl3_reg, lc_slc_inter_ctrl3_reg.regValue);

	// double buffer apply
	lc_slc_global_db_ctrl_reg.regValue = IoReg_Read32(LC_SLC_Global_Db_Ctrl_reg);
	lc_slc_global_db_ctrl_reg.slc_db_en = 1;
	lc_slc_global_db_ctrl_reg.slc_db_read_level = 1;
	lc_slc_global_db_ctrl_reg.slc_db_apply = 1;
	IoReg_Write32(LC_SLC_Global_Db_Ctrl_reg, lc_slc_global_db_ctrl_reg.regValue);
#endif	
}


unsigned char drvif_color_set_SLC_Curve(unsigned int *pCurve, unsigned char Blk)
{
#ifdef VIP_SUPPORT_SLC
	int i=0, j=0;
	unsigned int WriteValue = 0;
	unsigned int TimeOut = 100;
	lc_d_slc_rw_tab_ctrl_RBUS lc_d_slc_rw_tab_ctrl_reg;

	for(i=0; i<3; i++)
	{
		// Set block and addr information
		TimeOut = 100;
		lc_d_slc_rw_tab_ctrl_reg.regValue = IoReg_Read32(LC_D_SLC_rw_tab_ctrl_reg);
		lc_d_slc_rw_tab_ctrl_reg.rw_tab_addr = i;
		lc_d_slc_rw_tab_ctrl_reg.rw_blk_num = Blk;
		IoReg_Write32(LC_D_SLC_rw_tab_ctrl_reg, lc_d_slc_rw_tab_ctrl_reg.regValue);

		// Put data on register
		for(j=0; j<5; j++)
		{
			WriteValue = ((pCurve[i*11+j*2+1] & 0xfff) << 16) + (pCurve[i*11+j*2] & 0xfff);
			IoReg_Write32(LC_D_SLC_tab_wr_data0_reg+j*4, WriteValue);
		}

		IoReg_Write32(LC_D_SLC_tab_wr_data10_reg, pCurve[i*11+10]);

		// Trigger write
		lc_d_slc_rw_tab_ctrl_reg.regValue = IoReg_Read32(LC_D_SLC_rw_tab_ctrl_reg);
		lc_d_slc_rw_tab_ctrl_reg.rd_tab_en = 0;
		lc_d_slc_rw_tab_ctrl_reg.wr_tab_en = 1;
		IoReg_Write32(LC_D_SLC_rw_tab_ctrl_reg, lc_d_slc_rw_tab_ctrl_reg.regValue);

		// wait for wr_tab_en clear by hw and check
		while( TimeOut > 0 && (IoReg_Read32(LC_D_SLC_rw_tab_ctrl_reg)&LC_D_SLC_rw_tab_ctrl_wr_tab_en_mask)!=0 )
		{
			TimeOut--;
			//usleep_range(1, 5); //can't sleep in interrup context and busy wait 100 count is enough
		}
		if( TimeOut == 0 )
		{
			return  0;
		}

	}
#endif	
	return 1;
}

#if 0
/* SOFTWARE LOCAL CONTRAST GET FUNCTIONS*/
#endif

unsigned char drvif_color_get_SLC_Hist_HorBlkNum(void)
{
#ifdef VIP_SUPPORT_SLC
	lc_slc_global_ctrl0_RBUS lc_slc_global_ctrl0_reg;
	lc_slc_global_ctrl0_reg.regValue = IoReg_Read32(LC_SLC_Global_Ctrl0_reg);
	return (lc_slc_global_ctrl0_reg.slc_horblknum + 1);
#else
	return 0;
#endif
}

unsigned char drvif_color_get_SLC_Hist_VerBlkNum(void)
{
#ifdef VIP_SUPPORT_SLC
	lc_slc_global_ctrl0_RBUS lc_slc_global_ctrl0_reg;
	lc_slc_global_ctrl0_reg.regValue = IoReg_Read32(LC_SLC_Global_Ctrl0_reg);
	return (lc_slc_global_ctrl0_reg.slc_verblknum + 1);
#else
	return 0;
#endif
}

unsigned char drvif_color_get_SLC_Hist(unsigned int *pHist, unsigned char Channel, unsigned char Blk)
{
#ifdef VIP_SUPPORT_SLC
	int i=0;
	unsigned int ReadValue = 0;
	unsigned int TimeOut = 100;
	lc_slc_hist_ctrl_RBUS lc_slc_hist_ctrl_reg;

	lc_slc_hist_ctrl_reg.regValue = IoReg_Read32(LC_SLC_Hist_Ctrl_reg);
	lc_slc_hist_ctrl_reg.hist_read_adr = Blk;
	lc_slc_hist_ctrl_reg.hist_read_channel = Channel;
	lc_slc_hist_ctrl_reg.hist_read_valid = 1;

	IoReg_Write32(LC_SLC_Hist_Ctrl_reg, lc_slc_hist_ctrl_reg.regValue);

	// wait for hist_read_valid clear by hw
	while( TimeOut > 0 && (IoReg_Read32(LC_SLC_Hist_Ctrl_reg)&LC_SLC_Hist_Ctrl_hist_read_valid_mask)!=0 )
	{
		TimeOut--;
		//usleep_range(1, 5); //can't sleep in interrup context and busy wait 100 count is enough
	}
	if( TimeOut == 0 )
	{
		return  0;
	}

	// start to read histogram data from register
	for( i=0; i<4; i++ )
	{
		ReadValue = IoReg_Read32(LC_SLC_Hist_Data_0_reg+i*4);
		pHist[i*4+0] = ((ReadValue >> 24) & 0xff);
		pHist[i*4+1] = ((ReadValue >> 16) & 0xff);
		pHist[i*4+2] = ((ReadValue >>  8) & 0xff);
		pHist[i*4+3] = ((ReadValue >>  0) & 0xff);
	}
#endif
	return 1;
}

unsigned char drvif_color_get_SLC_Curve(unsigned int *pCurve, unsigned char Blk)
{
#ifdef VIP_SUPPORT_SLC
	int i=0, j=0;
	unsigned int ReadValue = 0;
	unsigned int TimeOut = 100;
	lc_d_slc_rw_tab_ctrl_RBUS lc_d_slc_rw_tab_ctrl_reg;

	for(i=0; i<3; i++)
	{
		// Set block and addr information
		TimeOut = 100;
		lc_d_slc_rw_tab_ctrl_reg.regValue = IoReg_Read32(LC_D_SLC_rw_tab_ctrl_reg);
		lc_d_slc_rw_tab_ctrl_reg.rw_tab_addr = i;
		lc_d_slc_rw_tab_ctrl_reg.rw_blk_num = Blk;
		IoReg_Write32(LC_D_SLC_rw_tab_ctrl_reg, lc_d_slc_rw_tab_ctrl_reg.regValue);

		// Trigger read
		lc_d_slc_rw_tab_ctrl_reg.regValue = IoReg_Read32(LC_D_SLC_rw_tab_ctrl_reg);
		lc_d_slc_rw_tab_ctrl_reg.rd_tab_en = 1;
		lc_d_slc_rw_tab_ctrl_reg.wr_tab_en = 0;
		IoReg_Write32(LC_D_SLC_rw_tab_ctrl_reg, lc_d_slc_rw_tab_ctrl_reg.regValue);

		// wait for rb_tab_en clear by hw
		while( TimeOut > 0 && (IoReg_Read32(LC_D_SLC_rw_tab_ctrl_reg)&LC_D_SLC_rw_tab_ctrl_rd_tab_en_mask)!=0 )
		{
			TimeOut--;
			usleep_range(1, 5);
		}
		if( TimeOut == 0 )
		{
			return  0;
		}

		for(j=0; j<5; j++)
		{
			ReadValue = IoReg_Read32(LC_D_SLC_tab_rd_data0_reg+j*4);
			pCurve[i*11+j*2  ] = ((ReadValue >>  0) & 0xfff );
			pCurve[i*11+j*2+1] = ((ReadValue >> 16) & 0xfff );
		}

	}
#endif	
	return 1;
}

#if 0
/* SOFTWARE LOCAL CONTRAST AUTO FUNCTIONS*/
#endif


/*
	generate the y remapping curve according to the input histogram
	input  : 16 bit histogram
	output : 33 pts curve
*/

void SoftwareBaseLocalContrast_SimpleDCC(
	/*IN */ DYNAMIC_SLC_CTRL_ST *pCtrl,
	/*IN */ unsigned int *pHistogram,
	/*OUT*/ unsigned int *pCurve
)
{
	// Firmware DCC adjust Parameters
	int EQLevel = 11;
	int LPFWeight = 31;
	//int SCurveWeight = 16;
	//double SLowPow = 1.08, SHighPow = 0.92;

	// Histogram arrays
	int EQHist[16] = {0};
	int EQHistSlope[16] = {0};
	int Bin32Hist[32] = {0};
	//int u12_SCurve[33] = {0};

	// Equalization calculation process parameters
	int HistMean = 0, HistTotal = 0;
	int HistDev = 0;
	int EQHistDiff = 0;
	int LBin = 0;
	int RBin = 0;

	// Curve generation process parameters
	//int SCenter = 0;//, HistAcc = 0;
	//double NorVal = 0.0;
	//double PowSel = 0.0;

	// Local parameters
	int HistBin = 0;
	int Pts = 0;

	EQLevel = pCtrl->u8Target_Y_EQ_Lv;

	// ====================[ Histogram equalization ] ====================
	// Mean and deviation calculation

	for(HistBin=0; HistBin<16; HistBin++)
	{
		HistTotal = HistTotal + pHistogram[HistBin];
	}
	HistMean = (HistTotal+8)/16;

	for(HistBin=0; HistBin<16; HistBin++)
	{
		HistDev = HistDev + ABS( (int)pHistogram[HistBin] - HistMean );
	}
	HistDev = ( ( (HistDev * EQLevel) << 10 ) / (HistTotal+1) ) + (1 << 10);
	HistDev = ( HistDev + 512 ) >> 10;

	// Equalization
	for(HistBin=0; HistBin<16; HistBin++)
	{
		EQHist[HistBin] = pHistogram[HistBin]/(HistDev+1);
		EQHistDiff = EQHistDiff + ( pHistogram[HistBin] - EQHist[HistBin] );
	}

	for(HistBin=0; HistBin<16; HistBin++)
	{
		EQHist[HistBin] = EQHist[HistBin] + (EQHistDiff/16);
	}

	// EQ histogram slope limit by [121] LPF and blending with original EQ histogram
	for(HistBin=0; HistBin<16; HistBin++)
	{
		EQHistSlope[HistBin] = 0;
	}

	for(HistBin=0; HistBin<16; HistBin++)
	{
		if( (EQHist[HistBin] << 10) > HistTotal*205 ) // ratio = x/1024
		{
			LBin = CLIP(0, 15, HistBin-1);
			RBin = CLIP(0, 15, HistBin+1);
			EQHistSlope[LBin]    = EQHistSlope[LBin]    + ( ( EQHist[HistBin] * 1 ) >> 2 );
			EQHistSlope[HistBin] = EQHistSlope[HistBin] + ( ( EQHist[HistBin] * 2 ) >> 2 );
			EQHistSlope[RBin]    = EQHistSlope[RBin]    + ( ( EQHist[HistBin] * 1 ) >> 2 );
		}
		else
		{
			EQHistSlope[HistBin] = EQHistSlope[HistBin] + EQHist[HistBin];
		}

	}
	for(HistBin=0; HistBin<16; HistBin++)
	{
		EQHistSlope[HistBin] = (EQHistSlope[HistBin]*LPFWeight + EQHist[HistBin]*(32-LPFWeight)) >> 5;
	}

	// Histogram Adjust
	for (HistBin=0; HistBin<pCtrl->u8TGT_YTH[0]; HistBin++)
		EQHistSlope[HistBin] = ((EQHistSlope[HistBin]*pCtrl->u16Target_Y_EQ_GAIN[0])>>7)+pCtrl->s16Target_Y_EQ_OFF[0];
	for (HistBin=pCtrl->u8TGT_YTH[0]; HistBin<pCtrl->u8TGT_YTH[1]; HistBin++)
		EQHistSlope[HistBin] = ((EQHistSlope[HistBin]*pCtrl->u16Target_Y_EQ_GAIN[1])>>7)+pCtrl->s16Target_Y_EQ_OFF[1];
	for (HistBin=pCtrl->u8TGT_YTH[1]; HistBin<pCtrl->u8TGT_YTH[2]; HistBin++)
		EQHistSlope[HistBin] = ((EQHistSlope[HistBin]*pCtrl->u16Target_Y_EQ_GAIN[2])>>7)+pCtrl->s16Target_Y_EQ_OFF[2];
	for (HistBin=pCtrl->u8TGT_YTH[2]; HistBin<pCtrl->u8TGT_YTH[3]; HistBin++)
		EQHistSlope[HistBin] = ((EQHistSlope[HistBin]*pCtrl->u16Target_Y_EQ_GAIN[3])>>7)+pCtrl->s16Target_Y_EQ_OFF[3];
	for (HistBin=pCtrl->u8TGT_YTH[3]; HistBin<16; HistBin++)
		EQHistSlope[HistBin] = ((EQHistSlope[HistBin]*pCtrl->u16Target_Y_EQ_GAIN[4])>>7)+pCtrl->s16Target_Y_EQ_OFF[4];

	// Expand the 16 bin histogram to 32 bin by duplicate and [121] LPF
	for(HistBin=0; HistBin<32; HistBin++)
	{
		Bin32Hist[HistBin] = EQHistSlope[HistBin/2];
	}
	for(HistBin=1; HistBin<31; HistBin++)
	{
		LBin = CLIP(0, 31, HistBin-1);
		RBin = CLIP(0, 31, HistBin+1);
		Bin32Hist[HistBin] = ( Bin32Hist[LBin] + Bin32Hist[HistBin]*2 + Bin32Hist[RBin] ) >> 2;
	}

	// ====================[ Curve generation equalization ] ====================
	// Generate the curve by histogram
	HistTotal = 0;
	for(HistBin=0; HistBin<32; HistBin++)
	{
		if (Bin32Hist[HistBin] < 0) //protect Histogram Adjust
			Bin32Hist[HistBin] = 0;
		HistTotal = HistTotal + Bin32Hist[HistBin];
	}

	pCurve[0]  = 0;
	pCurve[32] = 4095;
	for(Pts=1; Pts<32; Pts++)
	{
		pCurve[Pts] = pCurve[Pts-1]+((Bin32Hist[Pts-1]*4095)/HistTotal);
	}
#if 0
	// Gernerate the S curve according to the MaxBin index
	for(HistBin=0; HistBin<32; HistBin++)
	{
		HistAcc = HistAcc + Bin32Hist[HistBin];
		if( HistAcc > (HistTotal/2) )
		{
			SCenter = HistBin;
			break;
		}
	}

	u12_SCurve[0]  = 0;
	u12_SCurve[32] = 4095;
	for(Pts=1; Pts<32; Pts++)
	{
		NorVal = ((double)Pts*128)/4096;
		PowSel = (Pts>SCenter)? SHighPow : ( (Pts<SCenter)? SLowPow : 1 );
		//u12_SCurve[Pts] = (int)(pow(NorVal, PowSel) * 4096); // need to correct
		pCurve[Pts] = (u12_SCurve[Pts]*SCurveWeight + pCurve[Pts]*(32-SCurveWeight)) >> 5;
	}
#endif
}

void SoftwareBaseLocalContrast_FaceProtection(void)
{
#if 0 // not finish yet
	// Necessary parameters from Local Contrast
	extern unsigned int reg_LC_Blk_Hnum;
	extern unsigned int reg_LC_Blk_Vnum;
	extern int reg_LC_Blk_Hsize;
	extern int reg_LC_Blk_Vsize;
	extern int reg_LC_tmap_curve_grid[8];
	extern int LC_tmap_g0_th[32];
	extern int LC_tmap_g1_th[32];
	extern int LC_tmap_g2_th[32]; // 12 bit
	extern int reg_LC_tmap_curve_tone[8]; // 10 bit
	extern int localcon_block_backlight[2040]; // 10 bit
	extern int reg_localcon_block_avg[60][34]; // 11 bit
	extern int LC_tmap_sram_dp[8][32];
	extern int LC_tmap_sram_ds[8][32];
	extern int reg_LC_tmap_slope_unit;

	// initial all the curve to identity
	for(int BlkIdx=0; BlkIdx<256; BlkIdx++)
	{
		for(int Pts=0; Pts<33; Pts++)
		{
			u12_BlockCurve[BlkIdx][Pts] = CLIP(0, 4095, Pts*128);
		}
	}

	// skip the no face image
	if(FaceInfo[FrameCount].Enable != 0)
	{

		// calculate the slc block start and end
		int HBlkSize = (reg_SLC_HorBlkSize+1)*2;
		int VBlkSize = reg_SLC_VerBlkSize+1;

		FaceInfo[FrameCount].SLC_BlkStaX = FaceInfo[FrameCount].StaX/HBlkSize;
		FaceInfo[FrameCount].SLC_BlkStaY = FaceInfo[FrameCount].StaY/VBlkSize;
		FaceInfo[FrameCount].SLC_BlkEndX = (FaceInfo[FrameCount].EndX+HBlkSize-1)/HBlkSize;
		FaceInfo[FrameCount].SLC_BlkEndY = (FaceInfo[FrameCount].EndY+VBlkSize-1)/VBlkSize;


		// Calculate the tone mapping curve for each APL
		int ToneMappingCurve[8][4096] = {{0}};
		int Divisor = (reg_LC_tmap_slope_unit==1)? 32 : 16;
		for(int CurveIdx=0; CurveIdx<8; CurveIdx++)
		{
			int GridSel = reg_LC_tmap_curve_grid[CurveIdx];
			int *GridPtr = (GridSel==0)? LC_tmap_g0_th : ( (GridSel==1)? LC_tmap_g1_th : LC_tmap_g2_th);
			for(int Pts=0; Pts<4096; Pts++)
			{
				// find the point corresponding grid index
				int GridIdx = 0;
				for(int Iter=0; Iter<32; Iter++)
				{
					if( Pts>=GridPtr[Iter] )
					{
						GridIdx=Iter;
					}
				}
				int SRAMBase = LC_tmap_sram_dp[CurveIdx][GridIdx];
				int GridBase = GridPtr[GridIdx];
				int Slp = LC_tmap_sram_ds[CurveIdx][GridIdx];
				ToneMappingCurve[CurveIdx][Pts] = ((Pts-GridBase)*Slp+(Divisor/2))/Divisor + SRAMBase;
			}
		}

		// generate the slc inverse curve
		for(int blk_y=FaceInfo[FrameCount].SLC_BlkStaY; blk_y<FaceInfo[FrameCount].SLC_BlkEndY; blk_y++)
		{
			for(int blk_x=FaceInfo[FrameCount].SLC_BlkStaX; blk_x<FaceInfo[FrameCount].SLC_BlkEndX; blk_x++)
			{
				int BlkIdx = blk_y*(reg_SLC_HorBlkNum+1)+blk_x;

				int CorrLCBlk_x = (blk_x*HBlkSize + (HBlkSize/2) + reg_LC_Blk_Hsize/2)/reg_LC_Blk_Hsize;
				int CorrLCBlk_y = (blk_y*VBlkSize + (VBlkSize/2) + reg_LC_Blk_Vsize/2)/reg_LC_Blk_Vsize;
				//int BlockAPL = reg_localcon_block_avg[CorrLCBlk_x][CorrLCBlk_y];
				int BlockAPL = localcon_block_backlight[CorrLCBlk_y*(reg_LC_Blk_Hnum+1)+CorrLCBlk_x];
				int CurveL = 0;
				int CurveH = 0;

				for(int ToneIdx=0; ToneIdx<7; ToneIdx++)
				{
					if( BlockAPL >= reg_LC_tmap_curve_tone[ToneIdx] )
					{
						CurveL = ToneIdx;
						CurveH = ToneIdx+1;
					}
				}

				for(int Pts=0; Pts<33; Pts++)
				{
					int InvPtsL = 0;
					int InvPtsH = 0;
					for(int TonePts=0; TonePts<4096; TonePts++)
					{
						if(Pts*128>=ToneMappingCurve[CurveL][TonePts])
							InvPtsL = TonePts;
						if(Pts*128>=ToneMappingCurve[CurveH][TonePts])
							InvPtsH = TonePts;
					}

					int APLDiffL = BlockAPL-reg_LC_tmap_curve_tone[CurveL];
					int APLDiffH = reg_LC_tmap_curve_tone[CurveH]-BlockAPL;
					int InvPtsOut = (InvPtsL*APLDiffH + InvPtsH*APLDiffL)/(APLDiffH+APLDiffL);
					u12_BlockCurve[BlkIdx][Pts] = InvPtsOut;

				}
			}
		}
	}
#endif
}

void SoftwareBaseLocalContrast_APL_Gain(
	/*IN */ DYNAMIC_SLC_CTRL_ST *pCtrl,
	/*IN */ unsigned int *pHistogram,
	/*OUT*/ unsigned int *pCurve
)
{
	int APL = 0, HistTotal = 0;
	int HistBin = 0;
	int Tone[8] = {128, 256, 384, 512, 640, 768, 896, 1024};
	int LinearCurve[33] = {0, 128, 256, 384, 512, 640, 768, 896, 1024, 1152, 1280, 1408, 1536, 1664, 1792, 1920, 2048, 2176, 2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200, 3328, 3456, 3584, 3712, 3840, 3968, 4095};

	if (pCtrl->u8APL_gain_en == 0) //do noting
		return;

	// Calc APL form Histogram
	for (HistBin = 0; HistBin < 16; HistBin++) {
		HistTotal = HistTotal + pHistogram[HistBin];
		APL = APL + pHistogram[HistBin]*(HistBin<<6);
	}

	if (HistTotal)
		APL = APL / HistTotal;

	if (pCtrl->u8APL_gain_en == 1) {
		int APLGain = 0;
		if (APL <= pCtrl->u16APL_TH[0]) {
			APLGain = pCtrl->u16APL_GAIN_LC[0];
		} else if (APL <= pCtrl->u16APL_TH[1]) {
			APLGain = ((APL-pCtrl->u16APL_TH[0])*pCtrl->u16APL_GAIN_LC[1]+(pCtrl->u16APL_TH[1]-APL)*pCtrl->u16APL_GAIN_LC[0])/(pCtrl->u16APL_TH[1]-pCtrl->u16APL_TH[0]);
		} else if (APL <= pCtrl->u16APL_TH[2]) {
			APLGain = pCtrl->u16APL_GAIN_LC[1];
		} else if (APL <= pCtrl->u16APL_TH[3]) {
			APLGain = ((APL-pCtrl->u16APL_TH[2])*pCtrl->u16APL_GAIN_LC[2]+(pCtrl->u16APL_TH[3]-APL)*pCtrl->u16APL_GAIN_LC[1])/(pCtrl->u16APL_TH[3]-pCtrl->u16APL_TH[2]);
		} else
			APLGain = pCtrl->u16APL_GAIN_LC[2];

		for (HistBin = 0; HistBin < 33; HistBin++) {
			pCurve[HistBin] = (pCurve[HistBin]*APLGain)>>7;
			if (pCurve[HistBin] > 4095)
				pCurve[HistBin] = 4095;
		}
	} else if (pCtrl->u8APL_gain_en == 2) {
		int i, LB, CurveGain;
		for (i = 1; i < 7; i++) {
			if (APL < Tone[i])
				break;
		}

		LB = i-1;

		for (HistBin = 0; HistBin < 33; HistBin++) {
			if (i > 1)
				CurveGain = ((APL-Tone[LB])*pCtrl->u16APL_Curve[i][HistBin]+(Tone[i]-APL)*pCtrl->u16APL_Curve[LB][HistBin])>>7;
			else
				CurveGain = pCtrl->u16APL_Curve[i][HistBin];
			pCurve[HistBin] = (pCurve[HistBin]*CurveGain)/LinearCurve[HistBin];
			if (pCurve[HistBin] > 4095)
				pCurve[HistBin] = 4095;
		}
	} else if (pCtrl->u8APL_gain_en == 3) {
		int i, LB, CurveGain;
		int APLGain = 0;
		if (APL <= pCtrl->u16APL_TH[0]) {
			APLGain = pCtrl->u16APL_GAIN_LC[0];
		} else if (APL <= pCtrl->u16APL_TH[1]) {
			APLGain = ((APL-pCtrl->u16APL_TH[0])*pCtrl->u16APL_GAIN_LC[1]+(pCtrl->u16APL_TH[1]-APL)*pCtrl->u16APL_GAIN_LC[0])/(pCtrl->u16APL_TH[1]-pCtrl->u16APL_TH[0]);
		} else if (APL <= pCtrl->u16APL_TH[2]) {
			APLGain = pCtrl->u16APL_GAIN_LC[1];
		} else if (APL <= pCtrl->u16APL_TH[3]) {
			APLGain = ((APL-pCtrl->u16APL_TH[2])*pCtrl->u16APL_GAIN_LC[2]+(pCtrl->u16APL_TH[3]-APL)*pCtrl->u16APL_GAIN_LC[1])/(pCtrl->u16APL_TH[3]-pCtrl->u16APL_TH[2]);
		} else
			APLGain = pCtrl->u16APL_GAIN_LC[2];

		for (i = 1; i < 7; i++) {
			if (APL < Tone[i]) {
				break;
			}
		}

		LB = i-1;

		APLGain = APLGain << 3;

		for (HistBin = 0; HistBin < 33; HistBin++) {
			if (i > 1)
				CurveGain = ((APL-Tone[LB])*pCtrl->u16APL_Curve[i][HistBin]+(Tone[i]-APL)*pCtrl->u16APL_Curve[LB][HistBin])>>7;
			else
				CurveGain = pCtrl->u16APL_Curve[i][HistBin];
			pCurve[HistBin] = (pCurve[HistBin]*((CurveGain*APLGain)/LinearCurve[HistBin]))>>10;
			if (pCurve[HistBin] > 4095)
				pCurve[HistBin] = 4095;
		}
	}
}

void SoftwareBaseLocalContrast_Black_Level_Saturation_Protect(
	/*IN */ DYNAMIC_SLC_CTRL_ST *pCtrl,
	/*OUT*/ unsigned int *pCurve
)
{
	int HistBin = 0;
	int LinearCurve[33] = {0, 128, 256, 384, 512, 640, 768, 896, 1024, 1152, 1280, 1408, 1536, 1664, 1792, 1920, 2048, 2176, 2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200, 3328, 3456, 3584, 3712, 3840, 3968, 4095};
	int Smooth[3] = {3, 2, 1};

	for (HistBin = 0; HistBin <= pCtrl->u8Sat_BK_CTRL; HistBin++) {
		pCurve[HistBin] = (int)pCurve[HistBin]-((((int)pCurve[HistBin]-LinearCurve[HistBin])*pCtrl->u8Sat_BK_Gain)>>8);
	}
	for (HistBin = pCtrl->u8Sat_BK_CTRL+1; HistBin <= (pCtrl->u8Sat_BK_CTRL+3); HistBin++) {
		int index = 0;
		if (HistBin >= 33)
			break;
		pCurve[HistBin] = (int)pCurve[HistBin]-((((int)pCurve[HistBin]-LinearCurve[HistBin])*pCtrl->u8Sat_BK_Gain*Smooth[index])>>10);
		index++;
	}
}

void SoftwareBaseLocalContrast_Final_BlendWeight(
	/*IN */ DYNAMIC_SLC_CTRL_ST *pCtrl,
	/*OUT*/ unsigned int *pCurve
)
{
	int HistBin = 0;
	int LinearCurve[33] = {0, 128, 256, 384, 512, 640, 768, 896, 1024, 1152, 1280, 1408, 1536, 1664, 1792, 1920, 2048, 2176, 2304, 2432, 2560, 2688, 2816, 2944, 3072, 3200, 3328, 3456, 3584, 3712, 3840, 3968, 4095};

	for (HistBin = 0; HistBin < 33; HistBin++) {
		pCurve[HistBin] = (LinearCurve[HistBin]+(((pCurve[HistBin]-LinearCurve[HistBin])*pCtrl->u8FinalBlendWeight)>>7));
	}
}


void drvif_convert_DSLC_st(unsigned char direct, DYNAMIC_SLC_CTRL_ST *drv, DYNAMIC_SLC_CTRL_RTICE_ST *rtice)
{
	int i, j;
	if (direct) {// rtice to drv
		drv->u8LC_Target_Y_en = rtice->u8LC_Target_Y_en;
		drv->u8LC_Hist_H = rtice->u8LC_Hist_H;
		drv->u8LC_Hist_V = rtice->u8LC_Hist_V;
		drv->u8Target_Y_EQ_Lv = rtice->u8Target_Y_EQ_Lv;
		drv->u8LC_Target_Y_face_en = rtice->u8LC_Target_Y_face_en;
		drv->u8LC_Target_Y_face_weight = rtice->u8LC_Target_Y_face_weight;

		for (i = 0; i < 4; i++)
			drv->u8TGT_YTH[i] = rtice->u8TGT_YTH[i]; //0~31 bin
		for (i = 0; i < 5; i++) {
			drv->u16Target_Y_EQ_GAIN[i] = rtice->u16Target_Y_EQ_GAIN[i]; //128=1
			drv->s16Target_Y_EQ_OFF[i] = rtice->s16Target_Y_EQ_OFF[i]; //-128~128
		}

		drv->u8APL_gain_en = rtice->u8APL_gain_en; //0:disable, 1:APL GlobalGain, 2:APL CurveGain, 3: APL GlobalGain+CurveGain

		for (i = 0; i < 4; i++)
			drv->u16APL_TH[i] = rtice->u16APL_TH[i];//0~1023
		for (i = 0; i < 3; i++)
			drv->u16APL_GAIN_LC[i] = rtice->u16APL_GAIN_LC[i];//128=1
		for (i = 0; i < 8; i++) {
			for (j = 0; j < 33; j++)
				drv->u16APL_Curve[i][j] = rtice->u16APL_Curve[i][j];//0~4095
		}

		drv->u8Sat_BK_CTRL = rtice->u8Sat_BK_CTRL; //0~31 bin
		drv->u8Sat_BK_Gain = rtice->u8Sat_BK_Gain; //0~256

		drv->u8FinalBlendWeight = rtice->u8FinalBlendWeight; //0~128
	} else { //drv to rtice
		rtice->u8LC_Target_Y_en = drv->u8LC_Target_Y_en;
		rtice->u8LC_Hist_H = drv->u8LC_Hist_H;
		rtice->u8LC_Hist_V = drv->u8LC_Hist_V;
		rtice->u8Target_Y_EQ_Lv = drv->u8Target_Y_EQ_Lv;
		rtice->u8LC_Target_Y_face_en = drv->u8LC_Target_Y_face_en;
		rtice->u8LC_Target_Y_face_weight = drv->u8LC_Target_Y_face_weight;

		for (i = 0; i < 4; i++)
			rtice->u8TGT_YTH[i] = drv->u8TGT_YTH[i]; //0~31 bin
		for (i = 0; i < 5; i++) {
			rtice->u16Target_Y_EQ_GAIN[i] = drv->u16Target_Y_EQ_GAIN[i]; //128=1
			rtice->s16Target_Y_EQ_OFF[i] = drv->s16Target_Y_EQ_OFF[i]; //-128~128
		}

		rtice->u8APL_gain_en = drv->u8APL_gain_en; //0:disable, 1:APL GlobalGain, 2:APL CurveGain, 3: APL GlobalGain+CurveGain

		for (i = 0; i < 4; i++)
			rtice->u16APL_TH[i] = drv->u16APL_TH[i];//0~1023
		for (i = 0; i < 3; i++)
			rtice->u16APL_GAIN_LC[i] = drv->u16APL_GAIN_LC[i];//128=1
		for (i = 0; i < 8; i++) {
			for (j = 0; j < 33; j++)
				rtice->u16APL_Curve[i][j] = drv->u16APL_Curve[i][j];//0~4095
		}

		rtice->u8Sat_BK_CTRL = drv->u8Sat_BK_CTRL; //0~31 bin
		rtice->u8Sat_BK_Gain = drv->u8Sat_BK_Gain; //0~256

		rtice->u8FinalBlendWeight = drv->u8FinalBlendWeight; //0~128
	}
}

#if 0
/* COLOR DEPENDENCY LOCAL CONTRAST SET FUNCTIONS*/
#endif

void drvif_color_set_CDLC_Enable(unsigned char bYEnable, unsigned char bUEnable, unsigned char bVEnable)
{
	lc_cdlc_global_ctrl0_RBUS lc_cdlc_global_ctrl0_reg;

	lc_cdlc_global_ctrl0_reg.regValue = IoReg_Read32(LC_CDLC_Global_Ctrl0_reg);
	lc_cdlc_global_ctrl0_reg.cdlc_y_bld_enable = bYEnable;
	lc_cdlc_global_ctrl0_reg.cdlc_u_bld_enable = bUEnable;
	lc_cdlc_global_ctrl0_reg.cdlc_v_bld_enable = bVEnable;
	IoReg_Write32(LC_CDLC_Global_Ctrl0_reg, lc_cdlc_global_ctrl0_reg.regValue);
}

void drvif_color_set_CDLC_Offset(DRV_CDLC_Offset *pOffset)
{
	lc_cdlc_global_ctrl0_RBUS lc_cdlc_global_ctrl0_reg;
	lc_cdlc_global_ctrl1_RBUS lc_cdlc_global_ctrl1_reg;
	lc_cdlc_u_offset_data_0_RBUS lc_cdlc_u_offset_data_0_reg;
	lc_cdlc_u_offset_data_1_RBUS lc_cdlc_u_offset_data_1_reg;
	lc_cdlc_u_offset_data_2_RBUS lc_cdlc_u_offset_data_2_reg;
	lc_cdlc_v_offset_data_0_RBUS lc_cdlc_v_offset_data_0_reg;
	lc_cdlc_v_offset_data_1_RBUS lc_cdlc_v_offset_data_1_reg;
	lc_cdlc_v_offset_data_2_RBUS lc_cdlc_v_offset_data_2_reg;

	lc_cdlc_global_ctrl1_reg.regValue = IoReg_Read32(LC_CDLC_Global_Ctrl1_reg);
	lc_cdlc_global_ctrl1_reg.cdlc_u_center = pOffset->UCenter;
	lc_cdlc_global_ctrl1_reg.cdlc_v_center = pOffset->VCenter;
	IoReg_Write32(LC_CDLC_Global_Ctrl1_reg, lc_cdlc_global_ctrl1_reg.regValue);

	lc_cdlc_u_offset_data_0_reg.regValue = IoReg_Read32(LC_CDLC_U_Offset_Data_0_reg);
	lc_cdlc_u_offset_data_0_reg.cdlc_uoffset_curve_0 = pOffset->U_Curve[0];
	lc_cdlc_u_offset_data_0_reg.cdlc_uoffset_curve_1 = pOffset->U_Curve[1];
	lc_cdlc_u_offset_data_0_reg.cdlc_uoffset_curve_2 = pOffset->U_Curve[2];
	lc_cdlc_u_offset_data_0_reg.cdlc_uoffset_curve_3 = pOffset->U_Curve[3];
	IoReg_Write32(LC_CDLC_U_Offset_Data_0_reg, lc_cdlc_u_offset_data_0_reg.regValue);

	lc_cdlc_u_offset_data_1_reg.regValue = IoReg_Read32(LC_CDLC_U_Offset_Data_1_reg);
	lc_cdlc_u_offset_data_1_reg.cdlc_uoffset_curve_4 = pOffset->U_Curve[4];
	lc_cdlc_u_offset_data_1_reg.cdlc_uoffset_curve_5 = pOffset->U_Curve[5];
	lc_cdlc_u_offset_data_1_reg.cdlc_uoffset_curve_6 = pOffset->U_Curve[6];
	lc_cdlc_u_offset_data_1_reg.cdlc_uoffset_curve_7 = pOffset->U_Curve[7];
	IoReg_Write32(LC_CDLC_U_Offset_Data_1_reg, lc_cdlc_u_offset_data_1_reg.regValue);

	lc_cdlc_u_offset_data_2_reg.regValue = IoReg_Read32(LC_CDLC_U_Offset_Data_2_reg);
	lc_cdlc_u_offset_data_2_reg.cdlc_uoffset_curve_8 = pOffset->U_Curve[8];
	IoReg_Write32(LC_CDLC_U_Offset_Data_2_reg, lc_cdlc_u_offset_data_2_reg.regValue);

	lc_cdlc_v_offset_data_0_reg.regValue = IoReg_Read32(LC_CDLC_V_Offset_Data_0_reg);
	lc_cdlc_v_offset_data_0_reg.cdlc_voffset_curve_0 = pOffset->V_Curve[0];
	lc_cdlc_v_offset_data_0_reg.cdlc_voffset_curve_1 = pOffset->V_Curve[1];
	lc_cdlc_v_offset_data_0_reg.cdlc_voffset_curve_2 = pOffset->V_Curve[2];
	lc_cdlc_v_offset_data_0_reg.cdlc_voffset_curve_3 = pOffset->V_Curve[3];
	IoReg_Write32(LC_CDLC_V_Offset_Data_0_reg, lc_cdlc_v_offset_data_0_reg.regValue);

	lc_cdlc_v_offset_data_1_reg.regValue = IoReg_Read32(LC_CDLC_V_Offset_Data_1_reg);
	lc_cdlc_v_offset_data_1_reg.cdlc_voffset_curve_4 = pOffset->V_Curve[4];
	lc_cdlc_v_offset_data_1_reg.cdlc_voffset_curve_5 = pOffset->V_Curve[5];
	lc_cdlc_v_offset_data_1_reg.cdlc_voffset_curve_6 = pOffset->V_Curve[6];
	lc_cdlc_v_offset_data_1_reg.cdlc_voffset_curve_7 = pOffset->V_Curve[7];
	IoReg_Write32(LC_CDLC_V_Offset_Data_1_reg, lc_cdlc_v_offset_data_1_reg.regValue);

	lc_cdlc_v_offset_data_2_reg.regValue = IoReg_Read32(LC_CDLC_V_Offset_Data_2_reg);
	lc_cdlc_v_offset_data_2_reg.cdlc_voffset_curve_8 = pOffset->V_Curve[8];
	IoReg_Write32(LC_CDLC_V_Offset_Data_2_reg, lc_cdlc_v_offset_data_2_reg.regValue);

	lc_cdlc_global_ctrl0_reg.regValue = IoReg_Read32(LC_CDLC_Global_Ctrl0_reg);
	lc_cdlc_global_ctrl0_reg.cdlc_uvoffset_enable = pOffset->Enable;
	lc_cdlc_global_ctrl0_reg.cdlc_uvoffset_gain = pOffset->Gain;
	IoReg_Write32(LC_CDLC_Global_Ctrl0_reg, lc_cdlc_global_ctrl0_reg.regValue);
}

void drvif_color_set_CDLC_Shape(DRV_CDLC_Shape *pShape)
{
	lc_cdlc_global_ctrl0_RBUS lc_cdlc_global_ctrl0_reg;
	lc_cdlc_u_shape_data_0_RBUS lc_cdlc_u_shape_data_0_reg;
	lc_cdlc_u_shape_data_1_RBUS lc_cdlc_u_shape_data_1_reg;
	lc_cdlc_u_shape_data_2_RBUS lc_cdlc_u_shape_data_2_reg;
	lc_cdlc_v_shape_data_0_RBUS lc_cdlc_v_shape_data_0_reg;
	lc_cdlc_v_shape_data_1_RBUS lc_cdlc_v_shape_data_1_reg;
	lc_cdlc_v_shape_data_2_RBUS lc_cdlc_v_shape_data_2_reg;

	lc_cdlc_u_shape_data_0_reg.regValue = IoReg_Read32(LC_CDLC_U_shape_Data_0_reg);
	lc_cdlc_u_shape_data_0_reg.cdlc_ushape_curve_0 = pShape->U_Curve[0];
	lc_cdlc_u_shape_data_0_reg.cdlc_ushape_curve_1 = pShape->U_Curve[1];
	lc_cdlc_u_shape_data_0_reg.cdlc_ushape_curve_2 = pShape->U_Curve[2];
	lc_cdlc_u_shape_data_0_reg.cdlc_ushape_curve_3 = pShape->U_Curve[3];
	IoReg_Write32(LC_CDLC_U_shape_Data_0_reg, lc_cdlc_u_shape_data_0_reg.regValue);

	lc_cdlc_u_shape_data_1_reg.regValue = IoReg_Read32(LC_CDLC_U_shape_Data_1_reg);
	lc_cdlc_u_shape_data_1_reg.cdlc_ushape_curve_4 = pShape->U_Curve[4];
	lc_cdlc_u_shape_data_1_reg.cdlc_ushape_curve_5 = pShape->U_Curve[5];
	lc_cdlc_u_shape_data_1_reg.cdlc_ushape_curve_6 = pShape->U_Curve[6];
	lc_cdlc_u_shape_data_1_reg.cdlc_ushape_curve_7 = pShape->U_Curve[7];
	IoReg_Write32(LC_CDLC_U_shape_Data_1_reg, lc_cdlc_u_shape_data_1_reg.regValue);

	lc_cdlc_u_shape_data_2_reg.regValue = IoReg_Read32(LC_CDLC_U_shape_Data_2_reg);
	lc_cdlc_u_shape_data_2_reg.cdlc_ushape_curve_8 = pShape->U_Curve[8];
	IoReg_Write32(LC_CDLC_U_shape_Data_2_reg, lc_cdlc_u_shape_data_2_reg.regValue);

	lc_cdlc_v_shape_data_0_reg.regValue = IoReg_Read32(LC_CDLC_V_shape_Data_0_reg);
	lc_cdlc_v_shape_data_0_reg.cdlc_vshape_curve_0 = pShape->V_Curve[0];
	lc_cdlc_v_shape_data_0_reg.cdlc_vshape_curve_1 = pShape->V_Curve[1];
	lc_cdlc_v_shape_data_0_reg.cdlc_vshape_curve_2 = pShape->V_Curve[2];
	lc_cdlc_v_shape_data_0_reg.cdlc_vshape_curve_3 = pShape->V_Curve[3];
	IoReg_Write32(LC_CDLC_V_shape_Data_0_reg, lc_cdlc_v_shape_data_0_reg.regValue);

	lc_cdlc_v_shape_data_1_reg.regValue = IoReg_Read32(LC_CDLC_V_shape_Data_1_reg);
	lc_cdlc_v_shape_data_1_reg.cdlc_vshape_curve_4 = pShape->V_Curve[4];
	lc_cdlc_v_shape_data_1_reg.cdlc_vshape_curve_5 = pShape->V_Curve[5];
	lc_cdlc_v_shape_data_1_reg.cdlc_vshape_curve_6 = pShape->V_Curve[6];
	lc_cdlc_v_shape_data_1_reg.cdlc_vshape_curve_7 = pShape->V_Curve[7];
	IoReg_Write32(LC_CDLC_V_shape_Data_1_reg, lc_cdlc_v_shape_data_1_reg.regValue);

	lc_cdlc_v_shape_data_2_reg.regValue = IoReg_Read32(LC_CDLC_V_shape_Data_2_reg);
	lc_cdlc_v_shape_data_2_reg.cdlc_vshape_curve_8 = pShape->V_Curve[8];
	IoReg_Write32(LC_CDLC_V_shape_Data_2_reg, lc_cdlc_v_shape_data_2_reg.regValue);

	lc_cdlc_global_ctrl0_reg.regValue = IoReg_Read32(LC_CDLC_Global_Ctrl0_reg);
	lc_cdlc_global_ctrl0_reg.cdlc_uvshape_enable = pShape->Enable;
	IoReg_Write32(LC_CDLC_Global_Ctrl0_reg, lc_cdlc_global_ctrl0_reg.regValue);
}

void drvif_color_set_CDLC_Distance(DRV_CDLC_Distance *pDistance)
{
	lc_cdlc_global_ctrl0_RBUS lc_cdlc_global_ctrl0_reg;

	lc_cdlc_distance_gain_data_0_RBUS lc_cdlc_distance_gain_data_0_reg;
	lc_cdlc_distance_gain_data_1_RBUS lc_cdlc_distance_gain_data_1_reg;
	lc_cdlc_distance_gain_data_2_RBUS lc_cdlc_distance_gain_data_2_reg;

	lc_cdlc_distance_gain_data_0_reg.regValue = IoReg_Read32(LC_CDLC_Distance_Gain_Data_0_reg);
	lc_cdlc_distance_gain_data_0_reg.cdlc_distance_curve_0 = pDistance->Curve[0];
	lc_cdlc_distance_gain_data_0_reg.cdlc_distance_curve_1 = pDistance->Curve[1];
	lc_cdlc_distance_gain_data_0_reg.cdlc_distance_curve_2 = pDistance->Curve[2];
	lc_cdlc_distance_gain_data_0_reg.cdlc_distance_curve_3 = pDistance->Curve[3];
	IoReg_Write32(LC_CDLC_Distance_Gain_Data_0_reg, lc_cdlc_distance_gain_data_0_reg.regValue);

	lc_cdlc_distance_gain_data_1_reg.regValue = IoReg_Read32(LC_CDLC_Distance_Gain_Data_1_reg);
	lc_cdlc_distance_gain_data_1_reg.cdlc_distance_curve_4 = pDistance->Curve[4];
	lc_cdlc_distance_gain_data_1_reg.cdlc_distance_curve_5 = pDistance->Curve[5];
	lc_cdlc_distance_gain_data_1_reg.cdlc_distance_curve_6 = pDistance->Curve[6];
	lc_cdlc_distance_gain_data_1_reg.cdlc_distance_curve_7 = pDistance->Curve[7];
	IoReg_Write32(LC_CDLC_Distance_Gain_Data_1_reg, lc_cdlc_distance_gain_data_1_reg.regValue);

	lc_cdlc_distance_gain_data_2_reg.regValue = IoReg_Read32(LC_CDLC_Distance_Gain_Data_2_reg);
	lc_cdlc_distance_gain_data_2_reg.cdlc_distance_curve_8 = pDistance->Curve[8];
	IoReg_Write32(LC_CDLC_Distance_Gain_Data_2_reg, lc_cdlc_distance_gain_data_2_reg.regValue);

	lc_cdlc_global_ctrl0_reg.regValue = IoReg_Read32(LC_CDLC_Global_Ctrl0_reg);
	lc_cdlc_global_ctrl0_reg.cdlc_distgain_enable = pDistance->Enable;
	lc_cdlc_global_ctrl0_reg.cdlc_dist_sft = pDistance->Shift;
	IoReg_Write32(LC_CDLC_Global_Ctrl0_reg, lc_cdlc_global_ctrl0_reg.regValue);
}

#if 0
/* COLOR DEPENDENCY LOCAL CONTRAST GET FUNCTIONS*/
#endif

#if 0
/* LOCAL CONTRAST PIP OVERLAY APL SET FUNCTIONS*/
#endif

void drvif_color_set_LC_overlayAPL(DRV_LC_overlay_APL *overlay)
{
	lc_lc_pip_overlay_apl_ctrl0_RBUS lc_pip_overlay_apl_ctrl0_reg;
	lc_lc_pip_overlay_apl_ctrl1_RBUS lc_pip_overlay_apl_ctrl1_reg;
	lc_lc_global_ctrl1_RBUS blu_lc_global_ctrl1;
	
	lc_pip_overlay_apl_ctrl0_reg.regValue = IoReg_Read32(LC_LC_PIP_Overlay_APL_Ctrl0_reg);
	lc_pip_overlay_apl_ctrl0_reg.lc_overlay_reg = overlay->lc_overlay_value;
	lc_pip_overlay_apl_ctrl0_reg.lc_overlaymode = overlay->lc_overlay_mode;
	IoReg_Write32(LC_LC_PIP_Overlay_APL_Ctrl0_reg, lc_pip_overlay_apl_ctrl0_reg.regValue);
	
	lc_pip_overlay_apl_ctrl1_reg.regValue = IoReg_Read32(LC_LC_PIP_Overlay_APL_Ctrl1_reg);
	lc_pip_overlay_apl_ctrl1_reg.lc_overlay_yblk1=overlay->lc_overlay_yblk1;
	lc_pip_overlay_apl_ctrl1_reg.lc_overlay_yblk2=overlay->lc_overlay_yblk2;
	lc_pip_overlay_apl_ctrl1_reg.lc_overlay_xblk1=overlay->lc_overlay_xblk1;
	lc_pip_overlay_apl_ctrl1_reg.lc_overlay_xblk2=overlay->lc_overlay_xblk2;
	IoReg_Write32(LC_LC_PIP_Overlay_APL_Ctrl1_reg, lc_pip_overlay_apl_ctrl1_reg.regValue);

	//double buffer apply
	blu_lc_global_ctrl1.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	blu_lc_global_ctrl1.lc_db_apply = 1;
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, blu_lc_global_ctrl1.regValue);

}

#if 0
/* OVERLAY CONTROL */
#endif
void drvif_color_set_overlay_control(unsigned char bOverlayEn)
{
	color_d_vc_global_ctrl_0_RBUS color_d_vc_global_ctrl_0_reg;

	color_d_vc_global_ctrl_0_reg.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_0_reg);
	color_d_vc_global_ctrl_0_reg.write_enable_19 = 1;
	if( bOverlayEn != 0 )
	{
		color_d_vc_global_ctrl_0_reg.m_ctrl_lc = 1;
	}
	else
	{
		color_d_vc_global_ctrl_0_reg.m_ctrl_lc = 0;
	}
	IoReg_SetBits(COLOR_D_VC_Global_CTRL_DB_reg, COLOR_D_VC_Global_CTRL_DB_db_load_mask);
}

#if 0
/* LC_TONEMAPPING_YAVG_YIN_BLEND */
#endif
void drvif_color_set_LC_Tonemapping_Yavg_Yin_Blend(DRV_LC_ToneMapping_Yavg_Yin_Blend *blend)
{
	lc_lc_tonemapping_yavg_yin_blend_RBUS lc_lc_tonemapping_yavg_yin_blend_reg;
	lc_lc_tonemapping_yavg_yin_blend_factor_0_RBUS lc_lc_tonemapping_yavg_yin_blend_factor_0_reg;
	lc_lc_tonemapping_yavg_yin_blend_factor_1_RBUS lc_lc_tonemapping_yavg_yin_blend_factor_1_reg;
	lc_lc_global_ctrl1_RBUS blu_lc_global_ctrl1;
	lc_lc_tonemapping_yavg_yin_blend_reg.regValue = IoReg_Read32(LC_LC_ToneMapping_Yavg_Yin_blend_reg);
	lc_lc_tonemapping_yavg_yin_blend_reg.lc_yavg_yin_blend_en=blend->lc_yavg_yin_blend_en;
	IoReg_Write32(LC_LC_ToneMapping_Yavg_Yin_blend_reg, lc_lc_tonemapping_yavg_yin_blend_reg.regValue);

	lc_lc_tonemapping_yavg_yin_blend_factor_0_reg.regValue = IoReg_Read32(LC_LC_ToneMapping_Yavg_Yin_blend_factor_0_reg);
	lc_lc_tonemapping_yavg_yin_blend_factor_0_reg.lc_yavg_yin_blend_factor1=blend->blend_factor[0];
	lc_lc_tonemapping_yavg_yin_blend_factor_0_reg.lc_yavg_yin_blend_factor2=blend->blend_factor[1];
	lc_lc_tonemapping_yavg_yin_blend_factor_0_reg.lc_yavg_yin_blend_factor3=blend->blend_factor[2];
	lc_lc_tonemapping_yavg_yin_blend_factor_0_reg.lc_yavg_yin_blend_factor4=blend->blend_factor[3];
	IoReg_Write32(LC_LC_ToneMapping_Yavg_Yin_blend_factor_0_reg, lc_lc_tonemapping_yavg_yin_blend_factor_0_reg.regValue);

	lc_lc_tonemapping_yavg_yin_blend_factor_1_reg.regValue = IoReg_Read32(LC_LC_ToneMapping_Yavg_Yin_blend_factor_1_reg);
	lc_lc_tonemapping_yavg_yin_blend_factor_1_reg.lc_yavg_yin_blend_factor5=blend->blend_factor[4];
	lc_lc_tonemapping_yavg_yin_blend_factor_1_reg.lc_yavg_yin_blend_factor6=blend->blend_factor[5];
	lc_lc_tonemapping_yavg_yin_blend_factor_1_reg.lc_yavg_yin_blend_factor7=blend->blend_factor[6];
	lc_lc_tonemapping_yavg_yin_blend_factor_1_reg.lc_yavg_yin_blend_factor8=blend->blend_factor[7];
	IoReg_Write32(LC_LC_ToneMapping_Yavg_Yin_blend_factor_1_reg, lc_lc_tonemapping_yavg_yin_blend_factor_1_reg.regValue);	

	//double buffer apply
	blu_lc_global_ctrl1.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	blu_lc_global_ctrl1.lc_db_apply = 1;
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, blu_lc_global_ctrl1.regValue);
}

#if 0
/* LC_TONEMAPPING_PQMASK */
#endif
void drvif_color_set_LC_Tonemapping_PQmask(DRV_LC_ToneMapping_PQmask *tmap_pqmask)
{
	lc_lc_tonemapping_yavg_yin_blend_RBUS lc_lc_tonemapping_yavg_yin_blend_reg;
	lc_lc_tonemapping_pqmask_offset_RBUS lc_lc_tonemapping_pqmask_offset_reg;
	lc_lc_global_ctrl1_RBUS blu_lc_global_ctrl1;
	
	lc_lc_tonemapping_yavg_yin_blend_reg.regValue = IoReg_Read32(LC_LC_ToneMapping_Yavg_Yin_blend_reg);
	lc_lc_tonemapping_yavg_yin_blend_reg.lc_pqmask_tmap_en=tmap_pqmask->lc_PQmask_tmap_En;
	IoReg_Write32(LC_LC_ToneMapping_Yavg_Yin_blend_reg, lc_lc_tonemapping_yavg_yin_blend_reg.regValue);

	lc_lc_tonemapping_pqmask_offset_reg.regValue = IoReg_Read32(LC_LC_ToneMapping_PQmask_Offset_reg);
	lc_lc_tonemapping_pqmask_offset_reg.lc_pqmask_offset=tmap_pqmask->lc_PQmask_offset;
	IoReg_Write32(LC_LC_ToneMapping_PQmask_Offset_reg, lc_lc_tonemapping_pqmask_offset_reg.regValue);

	//double buffer apply
	blu_lc_global_ctrl1.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	blu_lc_global_ctrl1.lc_db_apply = 1;
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, blu_lc_global_ctrl1.regValue);
}

#if 0
/* LC_SHARPNESS_PQMASK */
#endif
void drvif_color_set_LC_Sharpness_PQmask(DRV_LC_Sharpness_PQmask *shp_pqmask)
{
	lc_lc_pqmask_shp_gain_RBUS lc_lc_pqmask_shp_gain_reg;
	lc_lc_global_ctrl1_RBUS blu_lc_global_ctrl1;
	
	lc_lc_pqmask_shp_gain_reg.regValue = IoReg_Read32(LC_LC_PQmask_Shp_Gain_reg);
	lc_lc_pqmask_shp_gain_reg.lc_pqmask_shp_en=shp_pqmask->lc_PQmask_shp_En;
	lc_lc_pqmask_shp_gain_reg.lc_pqmask_shp_debug_shift=shp_pqmask->lc_PQmask_shp_debug_shift;
	lc_lc_pqmask_shp_gain_reg.lc_pqmask_shp_gain=shp_pqmask->lc_PQmask_shp_gain;
	IoReg_Write32(LC_LC_PQmask_Shp_Gain_reg, lc_lc_pqmask_shp_gain_reg.regValue);

	//double buffer apply
	blu_lc_global_ctrl1.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	blu_lc_global_ctrl1.lc_db_apply = 1;
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, blu_lc_global_ctrl1.regValue);
}