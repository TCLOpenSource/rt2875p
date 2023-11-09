/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2014
  * All rights reserved.
  * ========================================================================*/
  /*================= File Description =======================================*/
  /**
   * @file
   *  This file is for FILM related functions.
   *
   * @author  $Author$
   * @date	  $Date$
   * @version	  $Revision$
   */

  /**
   * @addtogroup film
   * @{
   */

  /*============================ Module dependency	===========================*/
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/vip/localdimming.h>
#include <tvscalercontrol/scaler/vipinclude.h> /* for typedef that used in VIP_reg_def.h*/
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/scalerColor.h>
#ifndef BUILD_QUICK_SHOW
#include <rtd_log/rtd_module_log.h>
#include <rtk_kdriver/io.h>
#else
#include <no_os/printk.h>
#endif 
#include "rtk_vip_logger.h"
#include <mach/rtk_platform.h>


#define LocalDimming_Debug 0
#define ABS(x,y)((x > y) ? (x-y) : (y-x))

#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_LD_DEBUG,fmt,##args)

//static unsigned char localDimmingFullSet = 0;

/* Function Description : This function is for LD enable*/
void drvif_color_set_LD_Enable(unsigned char enable)
{
	blu_ld_global_ctrl1_RBUS blu_ld_global_ctrl1;
	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	blu_ld_global_ctrl1.regValue = IoReg_Read32(BLU_LD_Global_Ctrl1_reg);
	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);

	rtd_pr_vpq_info("ld_valid = %d\n", enable);
	blu_ld_global_ctrl2.ld_valid = enable;
	IoReg_Write32(BLU_LD_Global_Ctrl2_reg, blu_ld_global_ctrl2.regValue);

	/*double buffer apply*/
	blu_ld_global_ctrl1.ld_db_apply = 1;
	IoReg_Write32(BLU_LD_Global_Ctrl1_reg, blu_ld_global_ctrl1.regValue);

}

unsigned char drvif_color_get_LD_Enable(void)
{
	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	rtd_pr_vpq_info("get_LD_Enable=%d\n",blu_ld_global_ctrl2.ld_valid);

	return blu_ld_global_ctrl2.ld_valid;
}

void drvif_color_set_LD_Data_Compensation_Enable(unsigned char enable)
{
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
	blu_ld_global_ctrl1_RBUS blu_ld_global_ctrl1;

	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
	blu_ld_global_ctrl1.regValue = IoReg_Read32(BLU_LD_Global_Ctrl1_reg);


	blu_ld_global_ctrl0.ld_comp_en = enable;
	IoReg_Write32(BLU_LD_Global_Ctrl0_reg, blu_ld_global_ctrl0.regValue);
	blu_ld_global_ctrl1.ld_db_apply = 1;
	IoReg_Write32(BLU_LD_Global_Ctrl1_reg, blu_ld_global_ctrl1.regValue);

}

unsigned char drvif_color_get_LD_Data_Compensation_Enable(void)
{
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
	rtd_pr_vpq_info("get_LD_Data_Compensation_Enable=%d\n",blu_ld_global_ctrl0.ld_comp_en);

	return blu_ld_global_ctrl0.ld_comp_en;
}


void drvif_color_set_LD_Global_Ctrl(DRV_LD_Global_Ctrl *ptr)
{
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
	blu_ld_global_ctrl1_RBUS blu_ld_global_ctrl1;
	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	blu_ld_global_ctrl3_RBUS blu_ld_global_ctrl3;

	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
	blu_ld_global_ctrl1.regValue = IoReg_Read32(BLU_LD_Global_Ctrl1_reg);
	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	blu_ld_global_ctrl3.regValue = IoReg_Read32(BLU_LD_Global_Ctrl3_reg);


	blu_ld_global_ctrl1.ld_db_en = ptr->ld_db_en;
	blu_ld_global_ctrl1.ld_db_read_level = ptr->ld_db_read_level;
	IoReg_Write32(BLU_LD_Global_Ctrl1_reg, blu_ld_global_ctrl1.regValue);

	//blu_ld_global_ctrl2.ld_3d_mode = ptr->ld_3d_mode;
	blu_ld_global_ctrl2.ld_blk_hnum = ptr->ld_blk_hnum;
	blu_ld_global_ctrl2.ld_blk_vnum = ptr->ld_blk_vnum;
	blu_ld_global_ctrl2.ld_subblk_mode = ptr->ld_subblk_mode;
	blu_ld_global_ctrl2.ld_gdim_mode = ptr->ld_gdim_mode;
	blu_ld_global_ctrl2.ld_blk_type = ptr->ld_blk_type;
	blu_ld_global_ctrl2.ld_hist_mode = ptr->ld_hist_mode;
	blu_ld_global_ctrl2.ld_tenable = ptr->ld_tenable;
	blu_ld_global_ctrl2.ld_valid = ptr->ld_valid;
	blu_ld_global_ctrl3.ld_blk_hsize = ptr->ld_blk_hsize;
	blu_ld_global_ctrl3.ld_blk_vsize = ptr->ld_blk_vsize;

	IoReg_Write32(BLU_LD_Global_Ctrl2_reg, blu_ld_global_ctrl2.regValue);
	IoReg_Write32(BLU_LD_Global_Ctrl3_reg, blu_ld_global_ctrl3.regValue);

	blu_ld_global_ctrl0.ld_comp_en = ptr->ld_comp_en;
	IoReg_Write32(BLU_LD_Global_Ctrl0_reg, blu_ld_global_ctrl0.regValue);
	blu_ld_global_ctrl1.ld_db_apply = 1;
	IoReg_Write32(BLU_LD_Global_Ctrl1_reg, blu_ld_global_ctrl1.regValue);

}

void drvif_color_get_LD_Global_Ctrl(DRV_LD_Global_Ctrl *ptr)
{
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
	blu_ld_global_ctrl1_RBUS blu_ld_global_ctrl1;
	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	blu_ld_global_ctrl3_RBUS blu_ld_global_ctrl3;

	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
	blu_ld_global_ctrl1.regValue = IoReg_Read32(BLU_LD_Global_Ctrl1_reg);
	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	blu_ld_global_ctrl3.regValue = IoReg_Read32(BLU_LD_Global_Ctrl3_reg);


	ptr->ld_db_en = blu_ld_global_ctrl1.ld_db_en;
	ptr->ld_db_read_level = blu_ld_global_ctrl1.ld_db_read_level;
	//ptr->ld_3d_mode = blu_ld_global_ctrl2.ld_3d_mode;
	ptr->ld_blk_hnum = blu_ld_global_ctrl2.ld_blk_hnum;
	ptr->ld_blk_vnum = blu_ld_global_ctrl2.ld_blk_vnum;
	ptr->ld_subblk_mode = blu_ld_global_ctrl2.ld_subblk_mode;
	ptr->ld_gdim_mode = blu_ld_global_ctrl2.ld_gdim_mode;
	ptr->ld_blk_type = blu_ld_global_ctrl2.ld_blk_type;
	ptr->ld_hist_mode = blu_ld_global_ctrl2.ld_hist_mode;
	ptr->ld_tenable = blu_ld_global_ctrl2.ld_tenable;
	ptr->ld_valid = blu_ld_global_ctrl2.ld_valid;
	ptr->ld_blk_hsize = blu_ld_global_ctrl3.ld_blk_hsize;
	ptr->ld_blk_vsize = blu_ld_global_ctrl3.ld_blk_vsize;


}

void drvif_color_set_LD_Backlight_Decision(DRV_LD_Backlight_Decision *ptr)
{
	blu_ld_decision_ctrl1_RBUS blu_ld_decision_ctrl1;
	blu_ld_decision_ctrl2_RBUS blu_ld_decision_ctrl2;
	blu_ld_decision_ctrl3_RBUS blu_ld_decision_ctrl3;

	blu_ld_decision_ctrl1.regValue = IoReg_Read32(BLU_LD_Decision_Ctrl1_reg);
	blu_ld_decision_ctrl2.regValue = IoReg_Read32(BLU_LD_Decision_Ctrl2_reg);
	blu_ld_decision_ctrl3.regValue = IoReg_Read32(BLU_LD_Decision_Ctrl3_reg);

	blu_ld_decision_ctrl1.ld_maxgain = ptr->ld_maxgain;
	blu_ld_decision_ctrl1.ld_avegain = ptr->ld_avegain;
	blu_ld_decision_ctrl1.ld_histshiftbit = ptr->ld_histshiftbit;
	blu_ld_decision_ctrl2.ld_hist0gain = ptr->ld_hist0gain;
	blu_ld_decision_ctrl2.ld_hist1gain = ptr->ld_hist1gain;
	blu_ld_decision_ctrl2.ld_hist2gain = ptr->ld_hist2gain;
	blu_ld_decision_ctrl2.ld_hist3gain = ptr->ld_hist3gain;
	blu_ld_decision_ctrl3.ld_hist4gain = ptr->ld_hist4gain;
	blu_ld_decision_ctrl3.ld_hist5gain = ptr->ld_hist5gain;
	blu_ld_decision_ctrl3.ld_hist6gain = ptr->ld_hist6gain;
	blu_ld_decision_ctrl3.ld_hist7gain = ptr->ld_hist7gain;


	IoReg_Write32(BLU_LD_Decision_Ctrl1_reg, blu_ld_decision_ctrl1.regValue);
	IoReg_Write32(BLU_LD_Decision_Ctrl2_reg, blu_ld_decision_ctrl2.regValue);
	IoReg_Write32(BLU_LD_Decision_Ctrl3_reg, blu_ld_decision_ctrl3.regValue);

}

void drvif_color_get_LD_Backlight_Decision(DRV_LD_Backlight_Decision *ptr)
{
	blu_ld_decision_ctrl1_RBUS blu_ld_decision_ctrl1;
	blu_ld_decision_ctrl2_RBUS blu_ld_decision_ctrl2;
	blu_ld_decision_ctrl3_RBUS blu_ld_decision_ctrl3;

	blu_ld_decision_ctrl1.regValue = IoReg_Read32(BLU_LD_Decision_Ctrl1_reg);
	blu_ld_decision_ctrl2.regValue = IoReg_Read32(BLU_LD_Decision_Ctrl2_reg);
	blu_ld_decision_ctrl3.regValue = IoReg_Read32(BLU_LD_Decision_Ctrl3_reg);

	ptr->ld_maxgain = blu_ld_decision_ctrl1.ld_maxgain;
	ptr->ld_avegain = blu_ld_decision_ctrl1.ld_avegain;
	ptr->ld_histshiftbit = blu_ld_decision_ctrl1.ld_histshiftbit;
	ptr->ld_hist0gain = blu_ld_decision_ctrl2.ld_hist0gain;
	ptr->ld_hist1gain = blu_ld_decision_ctrl2.ld_hist1gain;
	ptr->ld_hist2gain = blu_ld_decision_ctrl2.ld_hist2gain;
	ptr->ld_hist3gain = blu_ld_decision_ctrl2.ld_hist3gain;
	ptr->ld_hist4gain = blu_ld_decision_ctrl3.ld_hist4gain;
	ptr->ld_hist5gain = blu_ld_decision_ctrl3.ld_hist5gain;
	ptr->ld_hist6gain = blu_ld_decision_ctrl3.ld_hist6gain;
	ptr->ld_hist7gain = blu_ld_decision_ctrl3.ld_hist7gain;

}


void drvif_color_set_LD_Spatial_Filter(DRV_LD_Spatial_Filter *ptr)
{
	blu_ld_spatial_ctrl1_RBUS blu_ld_spatial_ctrl1;
	blu_ld_spatial_ctrl2_RBUS blu_ld_spatial_ctrl2;
	blu_ld_spatial_ctrl5_RBUS blu_ld_spatial_ctrl5;
	blu_ld_spatial_ctrl6_RBUS blu_ld_spatial_ctrl6;
	blu_ld_spatial_ctrl7_RBUS blu_ld_spatial_ctrl7;
	blu_ld_spatial_ctrl8_RBUS blu_ld_spatial_ctrl8;
	blu_ld_spatial_ctrl9_RBUS blu_ld_spatial_ctrl9;
	blu_ld_spatial_ctrla_RBUS blu_ld_spatial_ctrla;

	blu_ld_spatial_ctrl1.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl1_reg);
	blu_ld_spatial_ctrl2.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl2_reg);
	blu_ld_spatial_ctrl5.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl5_reg);
	blu_ld_spatial_ctrl6.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl6_reg);
	blu_ld_spatial_ctrl7.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl7_reg);
	blu_ld_spatial_ctrl8.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl8_reg);
	blu_ld_spatial_ctrl9.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl9_reg);
	blu_ld_spatial_ctrla.regValue = IoReg_Read32(BLU_LD_Spatial_CtrlA_reg);

	blu_ld_spatial_ctrl1.ld_spatialcoef0 = ptr->ld_spatialcoef0;
	blu_ld_spatial_ctrl1.ld_spatialcoef1 = ptr->ld_spatialcoef1;
	blu_ld_spatial_ctrl1.ld_spatialcoef2 = ptr->ld_spatialcoef2;
	blu_ld_spatial_ctrl1.ld_spatialcoef3 = ptr->ld_spatialcoef3;
	blu_ld_spatial_ctrl1.ld_spatialcoef4 = ptr->ld_spatialcoef4;
	blu_ld_spatial_ctrl1.ld_spatialcoef5 = ptr->ld_spatialcoef5;
	blu_ld_spatial_ctrl1.ld_spatialcoef6 = ptr->ld_spatialcoef6;
	blu_ld_spatial_ctrl1.ld_spatialcoef7 = ptr->ld_spatialcoef7;
	blu_ld_spatial_ctrl2.ld_spatialcoef8 = ptr->ld_spatialcoef8;
	blu_ld_spatial_ctrl2.ld_spatialcoef9 = ptr->ld_spatialcoef9;
	blu_ld_spatial_ctrl2.ld_spatialcoef10 = ptr->ld_spatialcoef10;
	blu_ld_spatial_ctrl5.spatial_new_mode = ptr->spatial_new_mode;
	blu_ld_spatial_ctrl5.ld_spatialnewcoef00 = ptr->ld_spatialnewcoef00;
	blu_ld_spatial_ctrl5.ld_spatialnewcoef01 = ptr->ld_spatialnewcoef01;
	blu_ld_spatial_ctrl5.ld_spatialnewcoef02 = ptr->ld_spatialnewcoef02;
	blu_ld_spatial_ctrl5.ld_spatialnewcoef03 = ptr->ld_spatialnewcoef03;
	blu_ld_spatial_ctrl5.ld_spatialnewcoef04 = ptr->ld_spatialnewcoef04;
	blu_ld_spatial_ctrl6.ld_spatialnewcoef10 = ptr->ld_spatialnewcoef10;
	blu_ld_spatial_ctrl6.ld_spatialnewcoef11 = ptr->ld_spatialnewcoef11;
	blu_ld_spatial_ctrl6.ld_spatialnewcoef12 = ptr->ld_spatialnewcoef12;
	blu_ld_spatial_ctrl6.ld_spatialnewcoef13 = ptr->ld_spatialnewcoef13;
	blu_ld_spatial_ctrl6.ld_spatialnewcoef14 = ptr->ld_spatialnewcoef14;
	blu_ld_spatial_ctrl7.ld_spatialnewcoef20 = ptr->ld_spatialnewcoef20;
	blu_ld_spatial_ctrl7.ld_spatialnewcoef21 = ptr->ld_spatialnewcoef21;
	blu_ld_spatial_ctrl7.ld_spatialnewcoef22 = ptr->ld_spatialnewcoef22;
	blu_ld_spatial_ctrl7.ld_spatialnewcoef23 = ptr->ld_spatialnewcoef23;
	blu_ld_spatial_ctrl7.ld_spatialnewcoef24 = ptr->ld_spatialnewcoef24;
	blu_ld_spatial_ctrl8.ld_spatialnewcoef30 = ptr->ld_spatialnewcoef30;
	blu_ld_spatial_ctrl8.ld_spatialnewcoef31 = ptr->ld_spatialnewcoef31;
	blu_ld_spatial_ctrl8.ld_spatialnewcoef32 = ptr->ld_spatialnewcoef32;
	blu_ld_spatial_ctrl8.ld_spatialnewcoef33 = ptr->ld_spatialnewcoef33;
	blu_ld_spatial_ctrl8.ld_spatialnewcoef34 = ptr->ld_spatialnewcoef34;
	blu_ld_spatial_ctrl9.ld_spatialnewcoef05 = ptr->ld_spatialnewcoef05;
	blu_ld_spatial_ctrl9.ld_spatialnewcoef15 = ptr->ld_spatialnewcoef15;
	blu_ld_spatial_ctrl9.ld_spatialnewcoef25 = ptr->ld_spatialnewcoef25;
	blu_ld_spatial_ctrl9.ld_spatialnewcoef35 = ptr->ld_spatialnewcoef35;
	blu_ld_spatial_ctrla.ld_spatialnewcoef40 = ptr->ld_spatialnewcoef40;
	blu_ld_spatial_ctrla.ld_spatialnewcoef41 = ptr->ld_spatialnewcoef41;
	blu_ld_spatial_ctrla.ld_spatialnewcoef50 = ptr->ld_spatialnewcoef50;

	IoReg_Write32(BLU_LD_Spatial_Ctrl1_reg, blu_ld_spatial_ctrl1.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl5_reg, blu_ld_spatial_ctrl5.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl6_reg, blu_ld_spatial_ctrl6.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl7_reg, blu_ld_spatial_ctrl7.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl8_reg, blu_ld_spatial_ctrl8.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl9_reg, blu_ld_spatial_ctrl9.regValue);
	IoReg_Write32(BLU_LD_Spatial_CtrlA_reg, blu_ld_spatial_ctrla.regValue);

}

void drvif_color_get_LD_Spatial_Filter(DRV_LD_Spatial_Filter *ptr)
{
	blu_ld_spatial_ctrl1_RBUS blu_ld_spatial_ctrl1;
	blu_ld_spatial_ctrl2_RBUS blu_ld_spatial_ctrl2;
	blu_ld_spatial_ctrl5_RBUS blu_ld_spatial_ctrl5;
	blu_ld_spatial_ctrl6_RBUS blu_ld_spatial_ctrl6;
	blu_ld_spatial_ctrl7_RBUS blu_ld_spatial_ctrl7;
	blu_ld_spatial_ctrl8_RBUS blu_ld_spatial_ctrl8;
	blu_ld_spatial_ctrl9_RBUS blu_ld_spatial_ctrl9;
	blu_ld_spatial_ctrla_RBUS blu_ld_spatial_ctrla;

	blu_ld_spatial_ctrl1.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl1_reg);
	blu_ld_spatial_ctrl2.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl2_reg);
	blu_ld_spatial_ctrl5.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl5_reg);
	blu_ld_spatial_ctrl6.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl6_reg);
	blu_ld_spatial_ctrl7.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl7_reg);
	blu_ld_spatial_ctrl8.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl8_reg);
	blu_ld_spatial_ctrl9.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl9_reg);
	blu_ld_spatial_ctrla.regValue = IoReg_Read32(BLU_LD_Spatial_CtrlA_reg);

	ptr->ld_spatialcoef0 = blu_ld_spatial_ctrl1.ld_spatialcoef0;
	ptr->ld_spatialcoef1 = blu_ld_spatial_ctrl1.ld_spatialcoef1;
	ptr->ld_spatialcoef2 = blu_ld_spatial_ctrl1.ld_spatialcoef2;
	ptr->ld_spatialcoef3 = blu_ld_spatial_ctrl1.ld_spatialcoef3;
	ptr->ld_spatialcoef4 = blu_ld_spatial_ctrl1.ld_spatialcoef4;
	ptr->ld_spatialcoef5 = blu_ld_spatial_ctrl1.ld_spatialcoef5;
	ptr->ld_spatialcoef6 = blu_ld_spatial_ctrl1.ld_spatialcoef6;
	ptr->ld_spatialcoef7 = blu_ld_spatial_ctrl1.ld_spatialcoef7;
	ptr->ld_spatialcoef8 = blu_ld_spatial_ctrl2.ld_spatialcoef8;
	ptr->ld_spatialcoef9 = blu_ld_spatial_ctrl2.ld_spatialcoef9;
	ptr->ld_spatialcoef10 = blu_ld_spatial_ctrl2.ld_spatialcoef10;
	ptr->spatial_new_mode = blu_ld_spatial_ctrl5.spatial_new_mode;
	ptr->ld_spatialnewcoef00 = blu_ld_spatial_ctrl5.ld_spatialnewcoef00;
	ptr->ld_spatialnewcoef01 = blu_ld_spatial_ctrl5.ld_spatialnewcoef01;
	ptr->ld_spatialnewcoef02 = blu_ld_spatial_ctrl5.ld_spatialnewcoef02;
	ptr->ld_spatialnewcoef03 = blu_ld_spatial_ctrl5.ld_spatialnewcoef03;
	ptr->ld_spatialnewcoef04 = blu_ld_spatial_ctrl5.ld_spatialnewcoef04;
	ptr->ld_spatialnewcoef10 = blu_ld_spatial_ctrl6.ld_spatialnewcoef10;
	ptr->ld_spatialnewcoef11 = blu_ld_spatial_ctrl6.ld_spatialnewcoef11;
	ptr->ld_spatialnewcoef12 = blu_ld_spatial_ctrl6.ld_spatialnewcoef12;
	ptr->ld_spatialnewcoef13 = blu_ld_spatial_ctrl6.ld_spatialnewcoef13;
	ptr->ld_spatialnewcoef14 = blu_ld_spatial_ctrl6.ld_spatialnewcoef14;
	ptr->ld_spatialnewcoef20 = blu_ld_spatial_ctrl7.ld_spatialnewcoef20;
	ptr->ld_spatialnewcoef21 = blu_ld_spatial_ctrl7.ld_spatialnewcoef21;
	ptr->ld_spatialnewcoef22 = blu_ld_spatial_ctrl7.ld_spatialnewcoef22;
	ptr->ld_spatialnewcoef23 = blu_ld_spatial_ctrl7.ld_spatialnewcoef23;
	ptr->ld_spatialnewcoef24 = blu_ld_spatial_ctrl7.ld_spatialnewcoef24;
	ptr->ld_spatialnewcoef30 = blu_ld_spatial_ctrl8.ld_spatialnewcoef30;
	ptr->ld_spatialnewcoef31 = blu_ld_spatial_ctrl8.ld_spatialnewcoef31;
	ptr->ld_spatialnewcoef32 = blu_ld_spatial_ctrl8.ld_spatialnewcoef32;
	ptr->ld_spatialnewcoef33 = blu_ld_spatial_ctrl8.ld_spatialnewcoef33;
	ptr->ld_spatialnewcoef34 = blu_ld_spatial_ctrl8.ld_spatialnewcoef34;
	ptr->ld_spatialnewcoef05 = blu_ld_spatial_ctrl9.ld_spatialnewcoef05;
	ptr->ld_spatialnewcoef15 = blu_ld_spatial_ctrl9.ld_spatialnewcoef15;
	ptr->ld_spatialnewcoef25 = blu_ld_spatial_ctrl9.ld_spatialnewcoef25;
	ptr->ld_spatialnewcoef35 = blu_ld_spatial_ctrl9.ld_spatialnewcoef35;
	ptr->ld_spatialnewcoef40 = blu_ld_spatial_ctrla.ld_spatialnewcoef40;
	ptr->ld_spatialnewcoef41 = blu_ld_spatial_ctrla.ld_spatialnewcoef41;
	ptr->ld_spatialnewcoef50 = blu_ld_spatial_ctrla.ld_spatialnewcoef50;

}

void drvif_color_set_LD_Spatial_Remap(DRV_LD_Spatial_Remap *ptr)
{

	blu_ld_spatial_ctrl2_RBUS blu_ld_spatial_ctrl2;
	blu_ld_spatial_ctrl3_RBUS blu_ld_spatial_ctrl3;
	blu_ld_spatial_ctrl4_RBUS blu_ld_spatial_ctrl4;
	unsigned char i=0;

	blu_ld_spatial_ctrl2.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl2_reg);
	blu_ld_spatial_ctrl3.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl3_reg);
	blu_ld_spatial_ctrl4.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl4_reg);

	blu_ld_spatial_ctrl2.ld_spatialremapen = ptr->ld_spatialremapen;
	blu_ld_spatial_ctrl4.ld_spatialremaptab_initial = ptr->ld_spatialremaptab[0];
	IoReg_Write32(BLU_LD_Spatial_Ctrl4_reg, blu_ld_spatial_ctrl4.regValue);


	for (i = 0; i < 32; i++) {
		blu_ld_spatial_ctrl3.ld_spatialremapadr = i;
		blu_ld_spatial_ctrl3.ld_spatialremaptab0 = (ptr->ld_spatialremaptab[2*i+1] > 1023) ? 1023 : ptr->ld_spatialremaptab[2*i+1];
		blu_ld_spatial_ctrl3.ld_spatialremaptab1 = (ptr->ld_spatialremaptab[2*i+2] > 1023) ? 1023 : ptr->ld_spatialremaptab[2*i+2];
		IoReg_Write32(BLU_LD_Spatial_Ctrl3_reg, blu_ld_spatial_ctrl3.regValue);
	}

	IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);

}

void drvif_color_set_LD_Boost(DRV_LD_Boost *ptr)
{

	blu_ld_spatial_ctrl2_RBUS blu_ld_spatial_ctrl2;
	blu_ld_spatial_ctrl3_RBUS blu_ld_spatial_ctrl3;
	blu_ld_spatial_ctrl4_RBUS blu_ld_spatial_ctrl4;
	unsigned char i=0;

	blu_ld_spatial_ctrl2.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl2_reg);
	blu_ld_spatial_ctrl3.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl3_reg);
	blu_ld_spatial_ctrl4.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl4_reg);

	blu_ld_spatial_ctrl2.ld_backlightboosten= ptr->ld_backlightboosten;
	blu_ld_spatial_ctrl4.ld_spatialremaptab_initial2= ptr->ld_boost_gain_lut[0];
	blu_ld_spatial_ctrl4.ld_spatialremaptab_initial3= ptr->ld_boost_curve_lut[0];
	IoReg_Write32(BLU_LD_Spatial_Ctrl4_reg, blu_ld_spatial_ctrl4.regValue);


	/* Gain LUT*/
	for (i = 0; i < 32; i++) {
		blu_ld_spatial_ctrl3.ld_spatialremapadr = i+32;
		blu_ld_spatial_ctrl3.ld_spatialremaptab0 = (ptr->ld_boost_gain_lut[2*i+1] > 1023) ? 1023 : ptr->ld_boost_gain_lut[2*i+1];
		blu_ld_spatial_ctrl3.ld_spatialremaptab1 = (ptr->ld_boost_gain_lut[2*i+2] > 1023) ? 1023 : ptr->ld_boost_gain_lut[2*i+2];
		IoReg_Write32(BLU_LD_Spatial_Ctrl3_reg, blu_ld_spatial_ctrl3.regValue);
	}

	/* Boost LUT*/
	for (i = 0; i < 32; i++) {
		blu_ld_spatial_ctrl3.ld_spatialremapadr = i+64;
		blu_ld_spatial_ctrl3.ld_spatialremaptab0 = (ptr->ld_boost_curve_lut[2*i+1] > 1023) ? 1023 : ptr->ld_boost_curve_lut[2*i+1];
		blu_ld_spatial_ctrl3.ld_spatialremaptab1 = (ptr->ld_boost_curve_lut[2*i+2] > 1023) ? 1023 : ptr->ld_boost_curve_lut[2*i+2];
		IoReg_Write32(BLU_LD_Spatial_Ctrl3_reg, blu_ld_spatial_ctrl3.regValue);
	}

	IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);



}

void drvif_color_set_LD_Temporal_Filter(DRV_LD_Temporal_Filter *ptr)
{

	blu_ld_temporal_ctrl1_RBUS blu_ld_temporal_ctrl1;
	blu_ld_temporal_ctrl2_RBUS blu_ld_temporal_ctrl2;
	blu_ld_temporal_ctrl3_RBUS blu_ld_temporal_ctrl3;

	blu_ld_temporal_ctrl1.regValue = IoReg_Read32(BLU_LD_Temporal_Ctrl1_reg);
	blu_ld_temporal_ctrl2.regValue = IoReg_Read32(BLU_LD_Temporal_Ctrl2_reg);
	blu_ld_temporal_ctrl3.regValue = IoReg_Read32(BLU_LD_Temporal_Ctrl3_reg);

	blu_ld_temporal_ctrl1.ld_tmp_pos0thd = ptr->ld_tmp_pos0thd;
	blu_ld_temporal_ctrl1.ld_tmp_pos1thd = ptr->ld_tmp_pos1thd;
	blu_ld_temporal_ctrl1.ld_tmp_posmingain = ptr->ld_tmp_posmingain;
	blu_ld_temporal_ctrl1.ld_tmp_posmaxgain = ptr->ld_tmp_posmaxgain;
	blu_ld_temporal_ctrl2.ld_tmp_neg0thd = ptr->ld_tmp_neg0thd;
	blu_ld_temporal_ctrl2.ld_tmp_neg1thd = ptr->ld_tmp_neg1thd;
	blu_ld_temporal_ctrl2.ld_tmp_negmingain = ptr->ld_tmp_negmingain;
	blu_ld_temporal_ctrl2.ld_tmp_negmaxgain = ptr->ld_tmp_negmaxgain;
	blu_ld_temporal_ctrl3.ld_tmp_maxdiff = ptr->ld_tmp_maxdiff;
	blu_ld_temporal_ctrl3.ld_tmp_scenechangegain1 = ptr->ld_tmp_scenechangegain1;


	IoReg_Write32(BLU_LD_Temporal_Ctrl1_reg, blu_ld_temporal_ctrl1.regValue);
	IoReg_Write32(BLU_LD_Temporal_Ctrl2_reg, blu_ld_temporal_ctrl2.regValue);
	IoReg_Write32(BLU_LD_Temporal_Ctrl3_reg, blu_ld_temporal_ctrl3.regValue);



}


void drvif_color_get_LD_Temporal_Filter(DRV_LD_Temporal_Filter *ptr)
{

	blu_ld_temporal_ctrl1_RBUS blu_ld_temporal_ctrl1;
	blu_ld_temporal_ctrl2_RBUS blu_ld_temporal_ctrl2;
	blu_ld_temporal_ctrl3_RBUS blu_ld_temporal_ctrl3;

	blu_ld_temporal_ctrl1.regValue = IoReg_Read32(BLU_LD_Temporal_Ctrl1_reg);
	blu_ld_temporal_ctrl2.regValue = IoReg_Read32(BLU_LD_Temporal_Ctrl2_reg);
	blu_ld_temporal_ctrl3.regValue = IoReg_Read32(BLU_LD_Temporal_Ctrl3_reg);

	ptr->ld_tmp_pos0thd = blu_ld_temporal_ctrl1.ld_tmp_pos0thd;
	ptr->ld_tmp_pos1thd = blu_ld_temporal_ctrl1.ld_tmp_pos1thd;
	ptr->ld_tmp_posmingain = blu_ld_temporal_ctrl1.ld_tmp_posmingain;
	ptr->ld_tmp_posmaxgain = blu_ld_temporal_ctrl1.ld_tmp_posmaxgain;
	ptr->ld_tmp_neg0thd= blu_ld_temporal_ctrl2.ld_tmp_neg0thd;
	ptr->ld_tmp_neg1thd = blu_ld_temporal_ctrl2.ld_tmp_neg1thd;
	ptr->ld_tmp_negmingain = blu_ld_temporal_ctrl2.ld_tmp_negmingain;
	ptr->ld_tmp_negmaxgain = blu_ld_temporal_ctrl2.ld_tmp_negmaxgain;
	ptr->ld_tmp_maxdiff = blu_ld_temporal_ctrl3.ld_tmp_maxdiff;
	ptr->ld_tmp_scenechangegain1 = blu_ld_temporal_ctrl3.ld_tmp_scenechangegain1;

}

void drvif_color_set_LD_Backlight_Final_Decision(DRV_LD_Backlight_Final_Decision *ptr)
{
	blu_ld_backlight_ctrl1_RBUS blu_ld_backlight_ctrl1;
	blu_ld_global_ctrl1_RBUS blu_ld_global_ctrl1; /*for double buffer apply*/

	blu_ld_backlight_ctrl1.regValue = IoReg_Read32(BLU_LD_Backlight_Ctrl1_reg);
	blu_ld_global_ctrl1.regValue = IoReg_Read32(BLU_LD_Global_Ctrl1_reg);

	blu_ld_backlight_ctrl1.ld_blu_wr_num = ptr->ld_blu_wr_num;
	blu_ld_backlight_ctrl1.ld_blu_spitotal = ptr->ld_blu_spitotal;
	blu_ld_backlight_ctrl1.ld_blu_spien = ptr->ld_blu_spien;
	blu_ld_backlight_ctrl1.ld_blu_nodim = ptr->ld_blu_nodim;
	blu_ld_backlight_ctrl1.ld_blu_usergain = ptr->ld_blu_usergain;

	IoReg_Write32(BLU_LD_Backlight_Ctrl1_reg, blu_ld_backlight_ctrl1.regValue);

	/*double buffer apply*/
	blu_ld_global_ctrl1.ld_db_apply = 1;
	IoReg_Write32(BLU_LD_Global_Ctrl1_reg, blu_ld_global_ctrl1.regValue);


}

void drvif_color_get_LD_Backlight_Final_Decision(DRV_LD_Backlight_Final_Decision *ptr)
{
	blu_ld_backlight_ctrl1_RBUS blu_ld_backlight_ctrl1;

	blu_ld_backlight_ctrl1.regValue = IoReg_Read32(BLU_LD_Backlight_Ctrl1_reg);

	ptr->ld_blu_wr_num = blu_ld_backlight_ctrl1.ld_blu_wr_num;
	ptr->ld_blu_spitotal = blu_ld_backlight_ctrl1.ld_blu_spitotal;
	ptr->ld_blu_spien = blu_ld_backlight_ctrl1.ld_blu_spien;
	ptr->ld_blu_nodim = blu_ld_backlight_ctrl1.ld_blu_nodim;
	ptr->ld_blu_usergain = blu_ld_backlight_ctrl1.ld_blu_usergain;
}

void drvif_color_set_LD_Data_Compensation(DRV_LD_Data_Compensation *ptr)
{
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
	blu_ld_data_comp_ctrl_RBUS blu_ld_data_comp_ctrl;
	blu_ld_datacomp_ctrl1_RBUS blu_ld_datacomp_ctrl1;
	blu_ld_datacomp_ctrl2_RBUS blu_ld_datacomp_ctrl2;

	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
	blu_ld_data_comp_ctrl.regValue = IoReg_Read32(BLU_LD_DATA_COMP_CTRL_reg);
	blu_ld_datacomp_ctrl1.regValue = IoReg_Read32(BLU_LD_DataComp_Ctrl1_reg);
	blu_ld_datacomp_ctrl2.regValue = IoReg_Read32(BLU_LD_DataComp_Ctrl2_reg);

	blu_ld_datacomp_ctrl1.ld_comp_satrange = ptr->ld_comp_satrange;
	blu_ld_datacomp_ctrl1.ld_comp_softmax = ptr->ld_comp_softmax;
	blu_ld_datacomp_ctrl1.ld_comp_minlimit = ptr->ld_comp_minlimit;
	blu_ld_datacomp_ctrl1.ld_softcomp_gain = ptr->ld_softcomp_gain;
	blu_ld_datacomp_ctrl1.ld_comp_maxmode = ptr->ld_comp_maxmode;
	blu_ld_datacomp_ctrl2.ld_comp_inv_shift = ptr->ld_comp_inv_shift;
	blu_ld_datacomp_ctrl2.ld_comp_inv_mode = ptr->ld_comp_inv_mode;
	blu_ld_datacomp_ctrl2.ld_srcgainsel = ptr->ld_srcgainsel;

	IoReg_Write32(BLU_LD_DATA_COMP_CTRL_reg, blu_ld_data_comp_ctrl.regValue);
	IoReg_Write32(BLU_LD_DataComp_Ctrl1_reg, blu_ld_datacomp_ctrl1.regValue);
	IoReg_Write32(BLU_LD_DataComp_Ctrl2_reg, blu_ld_datacomp_ctrl2.regValue);

}

void drvif_color_set_LD_Data_Compensation_NewMode_2DTable(DRV_LD_Data_Compensation_NewMode_2DTable *ptr)
{
	int i,j;
	unsigned char ld_data_comp_mode_tmp;
	blu_ld_data_comp_ctrl_RBUS blu_ld_data_comp_ctrl;
	blu_ld_lut_data_RBUS blu_ld_lut_data;

	blu_ld_data_comp_ctrl.regValue = IoReg_Read32(BLU_LD_DATA_COMP_CTRL_reg);
	ld_data_comp_mode_tmp = blu_ld_data_comp_ctrl.ld_data_comp_mode;

	blu_ld_data_comp_ctrl.ld_data_comp_mode = 0;
	blu_ld_data_comp_ctrl.ld_lut_row_addr = 0;
	blu_ld_data_comp_ctrl.ld_lut_column_addr = 0;
	blu_ld_data_comp_ctrl.ld_lut_ax_en = 1;
	blu_ld_data_comp_ctrl.ld_lut_ax_mode = 0;
	blu_ld_data_comp_ctrl.ld_lut_adr_mode = 0;
	fwif_color_WaitFor_DEN_STOP_UZUDTG();
	IoReg_Write32(BLU_LD_DATA_COMP_CTRL_reg, blu_ld_data_comp_ctrl.regValue);

	for(i=0;i<17;i++) {
		for(j=0;j<17;j++) {
			blu_ld_lut_data.ld_lut_data = ptr->ld_comp_2Dtable[i][j];
			IoReg_Write32(BLU_LD_LUT_DATA_reg, blu_ld_lut_data.regValue);
		}
	}

	blu_ld_data_comp_ctrl.regValue = IoReg_Read32(BLU_LD_DATA_COMP_CTRL_reg);
	blu_ld_data_comp_ctrl.ld_lut_ax_en = 0;
	blu_ld_data_comp_ctrl.ld_data_comp_mode = ld_data_comp_mode_tmp;
	IoReg_Write32(BLU_LD_DATA_COMP_CTRL_reg, blu_ld_data_comp_ctrl.regValue);

}


void drvif_color_set_LD_Backlight_Profile_Interpolation(DRV_LD_Backlight_Profile_Interpolation *ptr)
{
	blu_ld_inter_hctrl1_RBUS blu_ld_inter_hctrl1;
	blu_ld_inter_hctrl2_RBUS blu_ld_inter_hctrl2;
	blu_ld_inter_vctrl1_RBUS blu_ld_inter_vctrl1;
	blu_ld_inter_vctrl2_RBUS blu_ld_inter_vctrl2;


	blu_ld_sram_ctrl_RBUS blu_ld_sram_ctrl;
	/*---- by table ----*/
	blu_ld_blight_ap_RBUS blu_ld_blight_ap;
	blu_ld_blight_dp_RBUS blu_ld_blight_dp;
	/*----------------*/
	blu_ld_inter_htabsel1_RBUS blu_ld_inter_htabsel1;
	blu_ld_inter_htabsel2_RBUS blu_ld_inter_htabsel2;
	blu_ld_inter_htabsel3_RBUS blu_ld_inter_htabsel3;
	blu_ld_inter_htabsel4_RBUS blu_ld_inter_htabsel4;
	blu_ld_inter_htabsel5_RBUS blu_ld_inter_htabsel5;
	blu_ld_inter_htabsel6_RBUS blu_ld_inter_htabsel6;
	blu_ld_inter_htabsel7_RBUS blu_ld_inter_htabsel7;
	blu_ld_inter_htabsel8_RBUS blu_ld_inter_htabsel8;
	blu_ld_inter_vtabsel1_RBUS blu_ld_inter_vtabsel1;
	blu_ld_inter_vtabsel2_RBUS blu_ld_inter_vtabsel2;
	blu_ld_inter_vtabsel3_RBUS blu_ld_inter_vtabsel3;
	blu_ld_inter_vtabsel4_RBUS blu_ld_inter_vtabsel4;
	blu_ld_inter_vtabsel5_RBUS blu_ld_inter_vtabsel5;
	blu_ld_inter_vtabsel6_RBUS blu_ld_inter_vtabsel6;
	blu_ld_inter_vtabsel7_RBUS blu_ld_inter_vtabsel7;
	blu_ld_inter_vtabsel8_RBUS blu_ld_inter_vtabsel8;

	blu_ld_inter_hctrl1.regValue = IoReg_Read32(BLU_LD_Inter_HCtrl1_reg);
	blu_ld_inter_hctrl2.regValue = IoReg_Read32(BLU_LD_Inter_HCtrl2_reg);
	blu_ld_inter_vctrl1.regValue = IoReg_Read32(BLU_LD_Inter_VCtrl1_reg);
	blu_ld_inter_vctrl2.regValue = IoReg_Read32(BLU_LD_Inter_VCtrl2_reg);

	blu_ld_sram_ctrl.regValue = IoReg_Read32(BLU_LD_SRAM_CTRL_reg);
	/*---- by table ----*/
	blu_ld_blight_ap.regValue = IoReg_Read32(BLU_LD_BLIGHT_AP_reg);
	blu_ld_blight_dp.regValue = IoReg_Read32(BLU_LD_BLIGHT_DP_reg);
	/*----------------*/
	blu_ld_inter_htabsel1.regValue = IoReg_Read32(BLU_LD_Inter_HTabSel1_reg);
	blu_ld_inter_htabsel2.regValue = IoReg_Read32(BLU_LD_Inter_HtabSel2_reg);
	blu_ld_inter_htabsel3.regValue = IoReg_Read32(BLU_LD_Inter_HtabSel3_reg);
	blu_ld_inter_htabsel4.regValue = IoReg_Read32(BLU_LD_Inter_HtabSel4_reg);
	blu_ld_inter_htabsel5.regValue = IoReg_Read32(BLU_LD_Inter_HtabSel5_reg);
	blu_ld_inter_htabsel6.regValue = IoReg_Read32(BLU_LD_Inter_HtabSel6_reg);
	blu_ld_inter_htabsel7.regValue = IoReg_Read32(BLU_LD_Inter_HtabSel7_reg);
	blu_ld_inter_htabsel8.regValue = IoReg_Read32(BLU_LD_Inter_HtabSel8_reg);
	blu_ld_inter_vtabsel1.regValue = IoReg_Read32(BLU_LD_Inter_VtabSel1_reg);
	blu_ld_inter_vtabsel2.regValue = IoReg_Read32(BLU_LD_Inter_VtabSel2_reg);
	blu_ld_inter_vtabsel3.regValue = IoReg_Read32(BLU_LD_Inter_VtabSel3_reg);
	blu_ld_inter_vtabsel4.regValue = IoReg_Read32(BLU_LD_Inter_VtabSel4_reg);
	blu_ld_inter_vtabsel5.regValue = IoReg_Read32(BLU_LD_Inter_VtabSel5_reg);
	blu_ld_inter_vtabsel6.regValue = IoReg_Read32(BLU_LD_Inter_VtabSel6_reg);
	blu_ld_inter_vtabsel7.regValue = IoReg_Read32(BLU_LD_Inter_VtabSel7_reg);
	blu_ld_inter_vtabsel8.regValue = IoReg_Read32(BLU_LD_Inter_VtabSel8_reg);

	blu_ld_inter_hctrl1.ld_tab_hsize = ptr->ld_tab_hsize;
	blu_ld_inter_hctrl1.ld_hfactor = ptr->ld_hfactor;
	blu_ld_inter_hctrl2.ld_hinitphase3_right = ptr->ld_hinitphase3_right;
	blu_ld_inter_hctrl2.ld_hboundary = ptr->ld_hboundary;
	blu_ld_inter_hctrl2.ld_hinitphase1 = ptr->ld_hinitphase1;
	blu_ld_inter_hctrl2.ld_hinitphase2 = ptr->ld_hinitphase2;
	blu_ld_inter_hctrl2.ld_hinitphase3_left = ptr->ld_hinitphase3_left;
	blu_ld_inter_vctrl1.ld_tab_vsize = ptr->ld_tab_vsize;
	blu_ld_inter_vctrl1.ld_vfactor = ptr->ld_vfactor;
	blu_ld_inter_vctrl2.ld_vinitphase3_right = ptr->ld_vinitphase3_right;
	blu_ld_inter_vctrl2.ld_vboundary = ptr->ld_vboundary;
	blu_ld_inter_vctrl2.ld_vinitphase1 = ptr->ld_vinitphase1;
	blu_ld_inter_vctrl2.ld_vinitphase2 = ptr->ld_vinitphase2;
	blu_ld_inter_vctrl2.ld_vinitphase3_left = ptr->ld_vinitphase3_left;

	blu_ld_sram_ctrl.ld_blight_update_en = ptr->ld_blight_update_en;
	blu_ld_sram_ctrl.ld_blight_sw_mode = ptr->ld_blight_sw_mode;


	blu_ld_inter_htabsel1.ld_htabsel_0 = ptr->ld_htabsel_0;
	blu_ld_inter_htabsel1.ld_htabsel_1 = ptr->ld_htabsel_1;
	blu_ld_inter_htabsel1.ld_htabsel_2 = ptr->ld_htabsel_2;
	blu_ld_inter_htabsel1.ld_htabsel_3 = ptr->ld_htabsel_3;
	blu_ld_inter_htabsel2.ld_htabsel_4 = ptr->ld_htabsel_4;
	blu_ld_inter_htabsel2.ld_htabsel_5 = ptr->ld_htabsel_5;
	blu_ld_inter_htabsel2.ld_htabsel_6 = ptr->ld_htabsel_6;
	blu_ld_inter_htabsel2.ld_htabsel_7 = ptr->ld_htabsel_7;
	blu_ld_inter_htabsel3.ld_htabsel_8 = ptr->ld_htabsel_8;
	blu_ld_inter_htabsel3.ld_htabsel_9 = ptr->ld_htabsel_9;
	blu_ld_inter_htabsel3.ld_htabsel_10 = ptr->ld_htabsel_10;
	blu_ld_inter_htabsel3.ld_htabsel_11 = ptr->ld_htabsel_11;
	blu_ld_inter_htabsel4.ld_htabsel_12 = ptr->ld_htabsel_12;
	blu_ld_inter_htabsel4.ld_htabsel_13 = ptr->ld_htabsel_13;
	blu_ld_inter_htabsel4.ld_htabsel_14 = ptr->ld_htabsel_14;
	blu_ld_inter_htabsel4.ld_htabsel_15 = ptr->ld_htabsel_15;
	blu_ld_inter_htabsel5.ld_htabsel_16 = ptr->ld_htabsel_16;
	blu_ld_inter_htabsel5.ld_htabsel_17 = ptr->ld_htabsel_17;
	blu_ld_inter_htabsel5.ld_htabsel_18 = ptr->ld_htabsel_18;
	blu_ld_inter_htabsel5.ld_htabsel_19 = ptr->ld_htabsel_19;
	blu_ld_inter_htabsel6.ld_htabsel_20 = ptr->ld_htabsel_20;
	blu_ld_inter_htabsel6.ld_htabsel_21 = ptr->ld_htabsel_21;
	blu_ld_inter_htabsel6.ld_htabsel_22 = ptr->ld_htabsel_22;
	blu_ld_inter_htabsel6.ld_htabsel_23 = ptr->ld_htabsel_23;
	blu_ld_inter_htabsel7.ld_htabsel_24 = ptr->ld_htabsel_24;
	blu_ld_inter_htabsel7.ld_htabsel_25 = ptr->ld_htabsel_25;
	blu_ld_inter_htabsel7.ld_htabsel_26 = ptr->ld_htabsel_26;
	blu_ld_inter_htabsel7.ld_htabsel_27 = ptr->ld_htabsel_27;
	blu_ld_inter_htabsel8.ld_htabsel_28 = ptr->ld_htabsel_28;
	blu_ld_inter_htabsel8.ld_htabsel_29 = ptr->ld_htabsel_29;
	blu_ld_inter_htabsel8.ld_htabsel_30 = ptr->ld_htabsel_30;
	blu_ld_inter_htabsel8.ld_htabsel_31 = ptr->ld_htabsel_31;
	blu_ld_inter_vtabsel1.ld_vtabsel_0 = ptr->ld_vtabsel_0;
	blu_ld_inter_vtabsel1.ld_vtabsel_1 = ptr->ld_vtabsel_1;
	blu_ld_inter_vtabsel1.ld_vtabsel_2 = ptr->ld_vtabsel_2;
	blu_ld_inter_vtabsel1.ld_vtabsel_3 = ptr->ld_vtabsel_3;
	blu_ld_inter_vtabsel2.ld_vtabsel_4 = ptr->ld_vtabsel_4;
	blu_ld_inter_vtabsel2.ld_vtabsel_5 = ptr->ld_vtabsel_5;
	blu_ld_inter_vtabsel2.ld_vtabsel_6 = ptr->ld_vtabsel_6;
	blu_ld_inter_vtabsel2.ld_vtabsel_7 = ptr->ld_vtabsel_7;
	blu_ld_inter_vtabsel3.ld_vtabsel_8 = ptr->ld_vtabsel_8;
	blu_ld_inter_vtabsel3.ld_vtabsel_9 = ptr->ld_vtabsel_9;
	blu_ld_inter_vtabsel3.ld_vtabsel_10 = ptr->ld_vtabsel_10;
	blu_ld_inter_vtabsel3.ld_vtabsel_11 = ptr->ld_vtabsel_11;
	blu_ld_inter_vtabsel4.ld_vtabsel_12 = ptr->ld_vtabsel_12;
	blu_ld_inter_vtabsel4.ld_vtabsel_13 = ptr->ld_vtabsel_13;
	blu_ld_inter_vtabsel4.ld_vtabsel_14 = ptr->ld_vtabsel_14;
	blu_ld_inter_vtabsel4.ld_vtabsel_15 = ptr->ld_vtabsel_15;
	blu_ld_inter_vtabsel5.ld_vtabsel_16 = ptr->ld_vtabsel_16;
	blu_ld_inter_vtabsel5.ld_vtabsel_17 = ptr->ld_vtabsel_17;
	blu_ld_inter_vtabsel5.ld_vtabsel_18 = ptr->ld_vtabsel_18;
	blu_ld_inter_vtabsel5.ld_vtabsel_19 = ptr->ld_vtabsel_19;
	blu_ld_inter_vtabsel6.ld_vtabsel_20 = ptr->ld_vtabsel_20;
	blu_ld_inter_vtabsel6.ld_vtabsel_21 = ptr->ld_vtabsel_21;
	blu_ld_inter_vtabsel6.ld_vtabsel_22 = ptr->ld_vtabsel_22;
	blu_ld_inter_vtabsel6.ld_vtabsel_23 = ptr->ld_vtabsel_23;
	blu_ld_inter_vtabsel7.ld_vtabsel_24 = ptr->ld_vtabsel_24;
	blu_ld_inter_vtabsel7.ld_vtabsel_25 = ptr->ld_vtabsel_25;
	blu_ld_inter_vtabsel7.ld_vtabsel_26 = ptr->ld_vtabsel_26;
	blu_ld_inter_vtabsel7.ld_vtabsel_27 = ptr->ld_vtabsel_27;
	blu_ld_inter_vtabsel8.ld_vtabsel_28 = ptr->ld_vtabsel_28;
	blu_ld_inter_vtabsel8.ld_vtabsel_29 = ptr->ld_vtabsel_29;
	blu_ld_inter_vtabsel8.ld_vtabsel_30 = ptr->ld_vtabsel_30;
	blu_ld_inter_vtabsel8.ld_vtabsel_31 = ptr->ld_vtabsel_31;


	IoReg_Write32(BLU_LD_SRAM_CTRL_reg, blu_ld_sram_ctrl.regValue);


	IoReg_Write32(BLU_LD_Inter_HCtrl1_reg, blu_ld_inter_hctrl1.regValue);
	IoReg_Write32(BLU_LD_Inter_HCtrl2_reg, blu_ld_inter_hctrl2.regValue);
	IoReg_Write32(BLU_LD_Inter_VCtrl1_reg, blu_ld_inter_vctrl1.regValue);
	IoReg_Write32(BLU_LD_Inter_VCtrl2_reg, blu_ld_inter_vctrl2.regValue);

	IoReg_Write32(BLU_LD_BLIGHT_AP_reg, blu_ld_blight_ap.regValue);
	IoReg_Write32(BLU_LD_BLIGHT_DP_reg, blu_ld_blight_dp.regValue);

	IoReg_Write32(BLU_LD_Inter_HTabSel1_reg, blu_ld_inter_htabsel1.regValue);
	IoReg_Write32(BLU_LD_Inter_HtabSel2_reg, blu_ld_inter_htabsel2.regValue);
	IoReg_Write32(BLU_LD_Inter_HtabSel3_reg, blu_ld_inter_htabsel3.regValue);
	IoReg_Write32(BLU_LD_Inter_HtabSel4_reg, blu_ld_inter_htabsel4.regValue);
	IoReg_Write32(BLU_LD_Inter_HtabSel5_reg, blu_ld_inter_htabsel5.regValue);
	IoReg_Write32(BLU_LD_Inter_HtabSel6_reg, blu_ld_inter_htabsel6.regValue);
	IoReg_Write32(BLU_LD_Inter_HtabSel7_reg, blu_ld_inter_htabsel7.regValue);
	IoReg_Write32(BLU_LD_Inter_HtabSel8_reg, blu_ld_inter_htabsel8.regValue);
	IoReg_Write32(BLU_LD_Inter_VtabSel1_reg, blu_ld_inter_vtabsel1.regValue);
	IoReg_Write32(BLU_LD_Inter_VtabSel2_reg, blu_ld_inter_vtabsel2.regValue);
	IoReg_Write32(BLU_LD_Inter_VtabSel3_reg, blu_ld_inter_vtabsel3.regValue);
	IoReg_Write32(BLU_LD_Inter_VtabSel4_reg, blu_ld_inter_vtabsel4.regValue);
	IoReg_Write32(BLU_LD_Inter_VtabSel5_reg, blu_ld_inter_vtabsel5.regValue);
	IoReg_Write32(BLU_LD_Inter_VtabSel6_reg, blu_ld_inter_vtabsel6.regValue);
	IoReg_Write32(BLU_LD_Inter_VtabSel7_reg, blu_ld_inter_vtabsel7.regValue);
	IoReg_Write32(BLU_LD_Inter_VtabSel8_reg, blu_ld_inter_vtabsel8.regValue);


}

void drvif_color_set_LD_BL_Profile_Interpolation_Table(unsigned int *ptr)
{
	int i, j, k;
	unsigned int *hlsb_ptr;
	unsigned int *hmsb_ptr;
	unsigned int *vlsb_ptr;
	unsigned int *vmsb_ptr;
	/*---- by table ----*/
	blu_ld_vtab_ap_RBUS blu_ld_vtab_ap;
	blu_ld_vtab_dp_RBUS blu_ld_vtab_dp;
	blu_ld_htab_ap_RBUS blu_ld_htab_ap;
	blu_ld_htab_dp_RBUS blu_ld_htab_dp;
	/*----------------*/

	blu_ld_sram_ctrl_RBUS blu_ld_sram_ctrl;

	/*---- by table ----*/
	blu_ld_vtab_ap.regValue = IoReg_Read32(BLU_LD_VTAB_AP_reg);
	blu_ld_vtab_dp.regValue = IoReg_Read32(BLU_LD_VTAB_DP_reg);
	blu_ld_htab_ap.regValue = IoReg_Read32(BLU_LD_HTAB_AP_reg);
	blu_ld_htab_dp.regValue = IoReg_Read32(BLU_LD_HTAB_DP_reg);
	/*----------------*/
	blu_ld_sram_ctrl.regValue = IoReg_Read32(BLU_LD_SRAM_CTRL_reg);

	blu_ld_sram_ctrl.ld_table_sw_mode = 1; /*software mode enable*/
	IoReg_Write32(BLU_LD_SRAM_CTRL_reg, blu_ld_sram_ctrl.regValue);

	blu_ld_vtab_ap.ld_vtab_adr = 0;
	blu_ld_htab_ap.ld_htab_adr = 0;
	IoReg_Write32(BLU_LD_VTAB_AP_reg, blu_ld_vtab_ap.regValue);
	IoReg_Write32(BLU_LD_HTAB_AP_reg, blu_ld_htab_ap.regValue);

	for (i = 0; i < BL_Profile_Table_NUM; i++) {
		for (k=0; k<BL_Profile_Table_ROW/4;k++) {

			hlsb_ptr = ptr+i*BL_Profile_Table_ROW*BL_Profile_Table_COLUMN + 2*k*BL_Profile_Table_COLUMN;
			hmsb_ptr = ptr+i*BL_Profile_Table_ROW*BL_Profile_Table_COLUMN + (BL_Profile_Table_ROW-2*k-1)*BL_Profile_Table_COLUMN;
			vlsb_ptr = ptr+i*BL_Profile_Table_ROW*BL_Profile_Table_COLUMN + (2*k+1)*BL_Profile_Table_COLUMN;
			vmsb_ptr = ptr+i*BL_Profile_Table_ROW*BL_Profile_Table_COLUMN + (BL_Profile_Table_ROW-2*k-2)*BL_Profile_Table_COLUMN;

			for (j = 0; j < BL_Profile_Table_COLUMN; j++) {

				blu_ld_vtab_dp.ld_vtab_dat_msb = *(vmsb_ptr+j);/*ptr->ld_vtab_dat_msb;*/
				blu_ld_vtab_dp.ld_vtab_dat_lsb = *(vlsb_ptr+j);/*ptr->ld_vtab_dat_lsb;*/

				blu_ld_htab_dp.ld_htab_dat_msb = *(hmsb_ptr+j);/*ptr->ld_htab_dat_msb;*/
				blu_ld_htab_dp.ld_htab_dat_lsb = *(hlsb_ptr+j);/*ptr->ld_htab_dat_lsb;*/

				//blu_ld_vtab_dp.ld_vtab_dat_msb = *(ptr+i*BL_Profile_Table_ROW*BL_Profile_Table_COLUMN+2*BL_Profile_Table_COLUMN+j);/*ptr->ld_vtab_dat_msb;*/
				//blu_ld_vtab_dp.ld_vtab_dat_lsb = *(ptr+i*BL_Profile_Table_ROW*BL_Profile_Table_COLUMN+1*BL_Profile_Table_COLUMN+j);/*ptr->ld_vtab_dat_lsb;*/

				//blu_ld_htab_dp.ld_htab_dat_msb = *(ptr+i*BL_Profile_Table_ROW*BL_Profile_Table_COLUMN+3*BL_Profile_Table_COLUMN+j);/*ptr->ld_htab_dat_msb;*/
				//blu_ld_htab_dp.ld_htab_dat_lsb = *(ptr+i*BL_Profile_Table_ROW*BL_Profile_Table_COLUMN+0*BL_Profile_Table_COLUMN+j);/*ptr->ld_htab_dat_lsb;*/

				IoReg_Write32(BLU_LD_VTAB_DP_reg, blu_ld_vtab_dp.regValue);
				IoReg_Write32(BLU_LD_HTAB_DP_reg, blu_ld_htab_dp.regValue);
			}
		}
	}

	blu_ld_sram_ctrl.ld_table_sw_mode = 0; /* software mode disable*/
	IoReg_Write32(BLU_LD_SRAM_CTRL_reg, blu_ld_sram_ctrl.regValue);


}

void drvif_color_set_LD_BL_Profile_Interpolation_Table_HV(unsigned int *Hptr, unsigned int *Vptr)
{
	int i, j;
	unsigned int *hlsb_ptr;
	unsigned int *vlsb_ptr;
	/*---- by table ----*/
	blu_ld_vtab_ap_RBUS blu_ld_vtab_ap;
	blu_ld_vtab_dp_RBUS blu_ld_vtab_dp;
	blu_ld_htab_ap_RBUS blu_ld_htab_ap;
	blu_ld_htab_dp_RBUS blu_ld_htab_dp;
	/*----------------*/

	blu_ld_sram_ctrl_RBUS blu_ld_sram_ctrl;

	/*---- by table ----*/
	blu_ld_vtab_ap.regValue = IoReg_Read32(BLU_LD_VTAB_AP_reg);
	blu_ld_vtab_dp.regValue = IoReg_Read32(BLU_LD_VTAB_DP_reg);
	blu_ld_htab_ap.regValue = IoReg_Read32(BLU_LD_HTAB_AP_reg);
	blu_ld_htab_dp.regValue = IoReg_Read32(BLU_LD_HTAB_DP_reg);
	/*----------------*/
	blu_ld_sram_ctrl.regValue = IoReg_Read32(BLU_LD_SRAM_CTRL_reg);

	blu_ld_sram_ctrl.ld_table_sw_mode = 1; /*software mode enable*/
	IoReg_Write32(BLU_LD_SRAM_CTRL_reg, blu_ld_sram_ctrl.regValue);

	blu_ld_vtab_ap.ld_vtab_adr = 0;
	blu_ld_htab_ap.ld_htab_adr = 0;
	IoReg_Write32(BLU_LD_VTAB_AP_reg, blu_ld_vtab_ap.regValue);
	IoReg_Write32(BLU_LD_HTAB_AP_reg, blu_ld_htab_ap.regValue);

	for (i = 0; i < BL_Profile_Table_NUM; i++) {
		hlsb_ptr = Hptr+i*BL_Profile_Table_COLUMN;
		vlsb_ptr = Vptr+i*BL_Profile_Table_COLUMN;
		
		for (j = 0; j < BL_Profile_Table_COLUMN/2; j++) {

			blu_ld_vtab_dp.ld_vtab_dat_msb = *(vlsb_ptr+2*j+1);/*ptr->ld_vtab_dat_msb;*/
			blu_ld_vtab_dp.ld_vtab_dat_lsb = *(vlsb_ptr+2*j);/*ptr->ld_vtab_dat_lsb;*/

			blu_ld_htab_dp.ld_htab_dat_msb = *(hlsb_ptr+2*j+1);/*ptr->ld_htab_dat_msb;*/
			blu_ld_htab_dp.ld_htab_dat_lsb = *(hlsb_ptr+2*j);/*ptr->ld_htab_dat_lsb;*/

			IoReg_Write32(BLU_LD_VTAB_DP_reg, blu_ld_vtab_dp.regValue);
			IoReg_Write32(BLU_LD_HTAB_DP_reg, blu_ld_htab_dp.regValue);
		}
	}

	blu_ld_sram_ctrl.ld_table_sw_mode = 0; /* software mode disable*/
	IoReg_Write32(BLU_LD_SRAM_CTRL_reg, blu_ld_sram_ctrl.regValue);


}


void drvif_color_set_LD_Demo_Window(DRV_LD_Demo_Window *ptr)
{
	blu_ld_demo_window_ctrl0_RBUS blu_ld_demo_window_ctrl0;
	blu_ld_demo_window_ctrl1_RBUS blu_ld_demo_window_ctrl1;

	blu_ld_demo_window_ctrl0.regValue = IoReg_Read32(BLU_LD_Demo_window_ctrl0_reg);
	blu_ld_demo_window_ctrl1.regValue = IoReg_Read32(BLU_LD_Demo_window_ctrl1_reg);

	blu_ld_demo_window_ctrl0.ld_demo_en = ptr->ld_demo_en;
	blu_ld_demo_window_ctrl0.ld_demo_mode = ptr->ld_demo_mode;
	blu_ld_demo_window_ctrl0.ld_demo_top = ptr->ld_demo_top;
	blu_ld_demo_window_ctrl0.ld_demo_bottom = ptr->ld_demo_bottom;
	blu_ld_demo_window_ctrl1.ld_demo_left = ptr->ld_demo_left;
	blu_ld_demo_window_ctrl1.ld_demo_right = ptr->ld_demo_right;

	IoReg_Write32(BLU_LD_Demo_window_ctrl0_reg, blu_ld_demo_window_ctrl0.regValue);
	IoReg_Write32(BLU_LD_Demo_window_ctrl1_reg, blu_ld_demo_window_ctrl1.regValue);

}

void drvif_color_set_LD_hw_blight_end_ie_en(unsigned char enable)
{
	blu_ld_int_RBUS blu_ld_int_reg;
	enum MACH_TYPE type = get_mach_type();
	if(type == MACH_ARCH_RTK2851C) {
		blu_ld_int_reg.regValue = IoReg_Read32(BLU_LD_int_reg);
		blu_ld_int_reg.hw_blight_end_ie= enable; /*ie routing to VCPU*/	
		IoReg_Write32(BLU_LD_int_reg, blu_ld_int_reg.regValue);
	}
}

void drvif_color_set_LD_hw_blight_end_ie2_en(unsigned char enable)
{
	blu_ld_int_RBUS blu_ld_int_reg;
	enum MACH_TYPE type = get_mach_type();
	if(type == MACH_ARCH_RTK2851C) {
		blu_ld_int_reg.regValue = IoReg_Read32(BLU_LD_int_reg);
		blu_ld_int_reg.hw_blight_end_ie2 = enable; /*ie2 routing to SCPU*/
		IoReg_Write32(BLU_LD_int_reg, blu_ld_int_reg.regValue);
	}
}

void drvif_color_set_LD_hist_done_ie_en(unsigned char enable)
{
	blu_ld_int_RBUS blu_ld_int_reg;
	enum MACH_TYPE type = get_mach_type();
	if(type == MACH_ARCH_RTK2851C) {
		blu_ld_int_reg.regValue = IoReg_Read32(BLU_LD_int_reg);
		blu_ld_int_reg.hist_done_ie= enable; /*ie routing to VCPU*/
		IoReg_Write32(BLU_LD_int_reg, blu_ld_int_reg.regValue);
	}
}

void drvif_color_set_LD_hist_done_ie2_en(unsigned char enable)
{
	blu_ld_int_RBUS blu_ld_int_reg;
	enum MACH_TYPE type = get_mach_type();
	if(type == MACH_ARCH_RTK2851C) {
		blu_ld_int_reg.regValue = IoReg_Read32(BLU_LD_int_reg);
		blu_ld_int_reg.hist_done_ie2 = enable; /*ie2 routing to SCPU*/
		IoReg_Write32(BLU_LD_int_reg, blu_ld_int_reg.regValue);
	}
}

void drvif_color_set_LD_AlgoSRAM_SWMode_En(unsigned char enable) 
{
	blu_ld_sram_ctrl_RBUS blu_ld_sram_ctrl;
	blu_ld_sram_ctrl.regValue = IoReg_Read32(BLU_LD_SRAM_CTRL_reg);
	blu_ld_sram_ctrl.ld_blight_sw_mode = enable; /*0:HW, 1:SW*/
	IoReg_Write32(BLU_LD_SRAM_CTRL_reg, blu_ld_sram_ctrl.regValue);

}

char drvif_color_get_LD_AlgoSRAM(unsigned int length, unsigned short *BLtable) 
{
	blu_ld_sram_ctrl_RBUS blu_ld_sram_ctrl;
	/*---- by table ----*/
	blu_ld_blight_ap_RBUS blu_ld_blight_ap;
	blu_ld_blight_dp_RBUS blu_ld_blight_dp;
	/*----------------*/
	unsigned char ret = 1;
	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	int i=0;
	unsigned char ld_blk_type, ld_blk_hnum, ld_blk_vnum;
	
	if(BLtable == NULL) {
		rtd_pr_vpq_emerg("BLtable is NULL\n");
		ret = 0;
		return ret;
	}

	//rtd_pr_vpq_emerg("drvif_color_set_LD_AlgoSRAM");
	blu_ld_sram_ctrl.regValue = IoReg_Read32(BLU_LD_SRAM_CTRL_reg);

	if (blu_ld_sram_ctrl.ld_blight_sw_mode == 1) {/*SW Mode enabled*/
			
		blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
		ld_blk_type = blu_ld_global_ctrl2.ld_blk_type;
		ld_blk_hnum = blu_ld_global_ctrl2.ld_blk_hnum;
		ld_blk_vnum = blu_ld_global_ctrl2.ld_blk_vnum;

		
		blu_ld_blight_ap.regValue = IoReg_Read32(BLU_LD_BLIGHT_AP_reg);
		blu_ld_blight_ap.ld_blight_adr= 0x00;
		IoReg_Write32(BLU_LD_BLIGHT_AP_reg, blu_ld_blight_ap.regValue);
		
		if (ld_blk_type == 1 || ld_blk_type==2) {/*edge type*/
			if (ld_blk_vnum == 0 || ld_blk_hnum == 0) {/*1 sided*/
				for(i=0;i<length;i++) {
					blu_ld_blight_dp.regValue = IoReg_Read32(BLU_LD_BLIGHT_DP_reg);
					//rtd_pr_vpq_emerg("A BLValue[%d] = %d\n", i, ldspi_data_rwport.data_rw_data_port);
					*(BLtable+i) = (unsigned short) blu_ld_blight_dp.ld_blight_dat_lsb;
				}
			} else {/*2 sided*/
				for(i=0;i<length;i++) {
					if (i%2 == 0){
						blu_ld_blight_dp.regValue = IoReg_Read32(BLU_LD_BLIGHT_DP_reg);
						//rtd_pr_vpq_emerg("A BLValue[%d] = %d\n", i, ldspi_data_rwport.data_rw_data_port);
						*(BLtable+i) = (unsigned short) blu_ld_blight_dp.ld_blight_dat_msb;
						*(BLtable+i+1) = (unsigned short) blu_ld_blight_dp.ld_blight_dat_lsb;
					}
				}
			}
		
		} else if(ld_blk_type == 0) { /*direct type*/
			for(i=0;i<length;i++) {
				if (i%2 == 0){
					blu_ld_blight_dp.regValue = IoReg_Read32(BLU_LD_BLIGHT_DP_reg);
					//rtd_pr_vpq_emerg("A BLValue[%d] = %d\n", i, ldspi_data_rwport.data_rw_data_port);
					*(BLtable+i) = (unsigned short) blu_ld_blight_dp.ld_blight_dat_lsb;
					*(BLtable+i+1) = (unsigned short) blu_ld_blight_dp.ld_blight_dat_msb;
				}
			}
		
		} else {
			rtd_pr_vpq_emerg("Get LD AlgoSRAM is not correct, id type = %d\n", ld_blk_type);
		
		}
	} else {
		rtd_pr_vpq_emerg("Please enable SWMode before writing the algo SRAM\n");
	}
	
	return ret;

}


void drvif_HAL_VPQ_LED_LDEnable(unsigned char bCtrl)
{
	drvif_color_set_LD_Data_Compensation_Enable(bCtrl);
	drvif_color_set_LD_Enable(bCtrl);
}

void drvif_HAL_VPQ_LED_LDCtrlDemoMode(DRV_HAL_VPQ_LED_LDCtrlDemoMode *ptr)
{
	blu_ld_demo_window_ctrl0_RBUS blu_ld_demo_window_ctrl0;
	blu_ld_demo_window_ctrl1_RBUS blu_ld_demo_window_ctrl1;
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;

	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	unsigned char ucld_blk_hnum, ucld_blk_vnum = 0;

	bool bCtrl = ptr->bCtrl;/*LDCtrlDemoMode.bCtrl;*/
	UINT8 bType = ptr->ctrlMode;/*LDCtrlDemoMode.ctrlMode;*/

	blu_ld_demo_window_ctrl0.regValue = IoReg_Read32(BLU_LD_Demo_window_ctrl0_reg);
	blu_ld_demo_window_ctrl1.regValue = IoReg_Read32(BLU_LD_Demo_window_ctrl1_reg);
	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);

	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);

	ucld_blk_hnum = blu_ld_global_ctrl2.ld_blk_hnum;
	ucld_blk_vnum = blu_ld_global_ctrl2.ld_blk_vnum;
	if (bCtrl) {
		if (bType == LED_ONOFF) {	/*Each LED should blink sequentially*/
			/*autoMA do that?*/
			/*Use blu_ld_global_ctrl0.dummy1802c200_30_0 to save enable bit & number of backlight*/
			/*bit 0 -> enable bit*/
			/*bit 1~9 -> number of backlight*/
			/*bit 10~15 -> timer*/
			unsigned int numOfbacklight = ucld_blk_hnum+1; /*max - need to be fixed by LD spec*/
			unsigned int blinkTimer = 60;
			blu_ld_global_ctrl0.dummy1802c200_30_0 = (blinkTimer<<10) + (numOfbacklight << 1) + 1;
		} else if (bType == SPLIT_SCREEN) {		/*Change half of screen as full white and show local dimming effect.*/
			blu_ld_demo_window_ctrl0.ld_demo_en = bCtrl;
			blu_ld_demo_window_ctrl0.ld_demo_mode = 0;
			blu_ld_demo_window_ctrl0.ld_demo_top = 0;
			blu_ld_demo_window_ctrl0.ld_demo_bottom = ptr->height;
			blu_ld_demo_window_ctrl1.ld_demo_left = 0;
			blu_ld_demo_window_ctrl1.ld_demo_right = ptr->width;
			blu_ld_global_ctrl0.dummy1802c200_30_0 = 0;

		}
	} else {
		blu_ld_demo_window_ctrl0.ld_demo_en = bCtrl;
		blu_ld_global_ctrl0.dummy1802c200_30_0 = 0;
	}
	IoReg_Write32(BLU_LD_Demo_window_ctrl0_reg, blu_ld_demo_window_ctrl0.regValue);
	IoReg_Write32(BLU_LD_Demo_window_ctrl1_reg, blu_ld_demo_window_ctrl1.regValue);
	IoReg_Write32(BLU_LD_Global_Ctrl0_reg, blu_ld_global_ctrl0.regValue);
}

extern unsigned char LD_CMD;
void drvif_HAL_VPQ_LED_LDCtrlSPI(unsigned char *LDCtrlSPI)
{

	unsigned char bitMask;
	unsigned char bCtrl;
	unsigned char cmd;

	ldspi_ld_ctrl_RBUS ld_ctrl;
	ldspi_data_addr_ctrl_RBUS ldspi_data_addr_ctrl;
	ldspi_data_rwport_RBUS ldspi_data_rwport;

	ld_ctrl.regValue = IoReg_Read32(LDSPI_LD_CTRL_reg);
	ld_ctrl.start_enable = 0;/*Ld_spi disable*/
	IoReg_Write32(LDSPI_LD_CTRL_reg, ld_ctrl.regValue);


	bitMask = *(LDCtrlSPI+0);
	bCtrl = *(LDCtrlSPI+1);

	cmd = LD_CMD;

	if (bCtrl == 1) {
		cmd = cmd | bitMask;
	} else if(bCtrl ==0) {
		cmd = cmd & ~bitMask;
	} else
		VIPprintf( "[VPQ] [Error] drvif_HAL_VPQ_LED_LDCtrlSPI, bCtrl=%d\n",bCtrl);

	ldspi_data_addr_ctrl.regValue = IoReg_Read32(LDSPI_Data_Addr_CTRL_reg);
	ldspi_data_addr_ctrl.data_rw_pointer_set = 0x7f;
	IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Modify data in data SRAM for cmd*/


	ldspi_data_rwport.data_rw_data_port = cmd;
	IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*Cmd*/
	LD_CMD = cmd;
	ld_ctrl.send_follow_vsync = 1; /*Send_follow_Vsync*/
	ld_ctrl.start_enable = 1;/*Ld_spi enable*/
	IoReg_Write32(LDSPI_LD_CTRL_reg, ld_ctrl.regValue);

}

unsigned char drvif_color_get_LD_GetAPL_TV006(unsigned short *BLValue)
{
	unsigned char ret = 1;

	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	ldspi_data_addr_ctrl_RBUS ldspi_data_addr_ctrl;
	ldspi_data_rwport_RBUS ldspi_data_rwport;
	unsigned char ld_blk_type, ld_blk_hnum, ld_blk_vnum;
	int i=0;
	unsigned short max =0, min=255, dummy;//, tmp=0;

	if(BLValue == NULL) {
		VIPprintf(" [Warning] drv GetAPL is not correct, BLValue is NULL\n");
		ret = 0;
		return ret;
	}

	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	ld_blk_type = blu_ld_global_ctrl2.ld_blk_type;
	ld_blk_hnum = blu_ld_global_ctrl2.ld_blk_hnum;
	ld_blk_vnum = blu_ld_global_ctrl2.ld_blk_vnum;

	ldspi_data_addr_ctrl.regValue = IoReg_Read32(LDSPI_Data_Addr_CTRL_reg);
	ldspi_data_addr_ctrl.data_rw_pointer_set = 0x00;
	IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue);
	dummy = IoReg_Read32(LDSPI_Data_Rwport_reg);


	max = 0;
	min = 255;
	if (ld_blk_type == 1 || ld_blk_type==2) {/*edge type*/
		if (ld_blk_vnum == 0) {/*1 sided*/
			for(i=0;i<2*(ld_blk_hnum+1);i+=2) {
				ldspi_data_rwport.regValue = IoReg_Read32(LDSPI_Data_Rwport_reg);

				if (ldspi_data_rwport.data_rw_data_port > max)
					max = ldspi_data_rwport.data_rw_data_port;
				if (ldspi_data_rwport.data_rw_data_port < min)
					min = ldspi_data_rwport.data_rw_data_port;
				ldspi_data_rwport.regValue = IoReg_Read32(LDSPI_Data_Rwport_reg);


			}
			VIPprintf("[VPQLED] 1. max=%d, min=%d\n", max,min);
		} else {/*2 sided*/
			for(i=0;i<2*(ld_blk_hnum+1);i++) {
				ldspi_data_rwport.regValue = IoReg_Read32(LDSPI_Data_Rwport_reg);
				if (ldspi_data_rwport.data_rw_data_port > max)
					max = ldspi_data_rwport.data_rw_data_port;
				if (ldspi_data_rwport.data_rw_data_port < min)
					min = ldspi_data_rwport.data_rw_data_port;
			}
			VIPprintf("[VPQLED] 2. max=%d, min=%d\n", max,min);

		}

	} else if(ld_blk_type == 0) { /*direct type*/
		for(i=0;i<(ld_blk_vnum+1)*(ld_blk_hnum+1);i++) {
			ldspi_data_rwport.regValue = IoReg_Read32(LDSPI_Data_Rwport_reg);
			if (ldspi_data_rwport.data_rw_data_port > max)
				max = ldspi_data_rwport.data_rw_data_port;
			if (ldspi_data_rwport.data_rw_data_port < min)
				min = ldspi_data_rwport.data_rw_data_port;

		}
		VIPprintf("[VPQLED] 3. max=%d, min=%d\n", max,min);

	} else {
		VIPprintf("[VPQLED][Warning] GetAPL is not correct, id type = %d\n", ld_blk_type);

	}

	*BLValue = max * 4;
	*(BLValue+1) = min * 4;

	return ret;
}

void drvif_color_set_LDSPI_DataSRAM_Data_Continuous(DRV_LD_LDSPI_DATASRAM_TYPE *LDSPI_DataSRAM, unsigned char Wait_Tx_Done)
{
	unsigned short i;
	
	ldspi_data_addr_ctrl_RBUS ldspi_data_addr_ctrl;
	ldspi_data_rwport_RBUS ldspi_data_rwport;
	ldspi_wclr_ctrl_RBUS ldspi_wclr_ctrl;

	unsigned int timeoutcnt = 0x32500;
	unsigned int dummy;
	unsigned char tx_done;

	/*Null Protection*/
	if (LDSPI_DataSRAM == NULL) {
		rtd_pr_vpq_emerg("drvif_color_set_LDSPI_DataSRAM_Data_Continuous, LDSPI_DataSRAM is NULL\n");
		return;
	}
	if (LDSPI_DataSRAM->SRAM_Value == NULL) {
		rtd_pr_vpq_emerg("drvif_color_set_LDSPI_DataSRAM_Data_Continuous, LDSPI_DataSRAM->SRAM_Value is NULL\n");
		return;
	}

	if ((LDSPI_DataSRAM->SRAM_Position+LDSPI_DataSRAM->SRAM_Length)>1072) {
		rtd_pr_vpq_emerg("drvif_color_set_LDSPI_DataSRAM_Data_Continuous, Data too much\n");
		return;
	}
		

	if (Wait_Tx_Done == 1) {/*Normal Usage Wait tx_done*/
	
		do {
			ldspi_wclr_ctrl.regValue = IoReg_Read32(LDSPI_WCLR_CTRL_reg);
			tx_done = ldspi_wclr_ctrl.tx_done;
			if(tx_done == 1) {
				ldspi_data_addr_ctrl.regValue = IoReg_Read32(LDSPI_Data_Addr_CTRL_reg);				
				if (LDSPI_DataSRAM->SRAM_Position%2==0) {/*SRAM_Position is even*/
					ldspi_data_addr_ctrl.data_rw_pointer_set = LDSPI_DataSRAM->SRAM_Position/2;
					IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
					for (i=0;i<LDSPI_DataSRAM->SRAM_Length/2;i++) { 		
						ldspi_data_rwport.data_rw_data_port = (*(LDSPI_DataSRAM->SRAM_Value+2*i))+(*(LDSPI_DataSRAM->SRAM_Value+2*i+1)<<12);
						IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
					}
					if (LDSPI_DataSRAM->SRAM_Length%2==1) {/*length is odd, read 24 bits, combine last data to MSB12bits*/
						ldspi_data_addr_ctrl.data_rw_pointer_set = (LDSPI_DataSRAM->SRAM_Position+LDSPI_DataSRAM->SRAM_Length)/2;
						IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
						dummy = IoReg_Read32(LDSPI_Data_Rwport_reg);
						ldspi_data_rwport.regValue= IoReg_Read32(LDSPI_Data_Rwport_reg);
						ldspi_data_rwport.data_rw_data_port = (*(LDSPI_DataSRAM->SRAM_Value+LDSPI_DataSRAM->SRAM_Length-1))+((ldspi_data_rwport.data_rw_data_port&0xfff)<<12);
						
						ldspi_data_addr_ctrl.data_rw_pointer_set = (LDSPI_DataSRAM->SRAM_Position+LDSPI_DataSRAM->SRAM_Length)/2;
						IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
						IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
					}
				} else { /* SRAM_Position is odd*/
						/*handel first data*/
						ldspi_data_addr_ctrl.data_rw_pointer_set = LDSPI_DataSRAM->SRAM_Position/2;
						IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
						dummy = IoReg_Read32(LDSPI_Data_Rwport_reg);
						ldspi_data_rwport.regValue = IoReg_Read32(LDSPI_Data_Rwport_reg);
						ldspi_data_rwport.data_rw_data_port = *(LDSPI_DataSRAM->SRAM_Value)+(ldspi_data_rwport.data_rw_data_port&0xfff000); 
						
						ldspi_data_addr_ctrl.data_rw_pointer_set = LDSPI_DataSRAM->SRAM_Position/2;
						IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
						IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
						
						/*handle following data*/
						ldspi_data_addr_ctrl.data_rw_pointer_set = (LDSPI_DataSRAM->SRAM_Position+1)/2;
						IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
						for (i=0;i<(LDSPI_DataSRAM->SRAM_Length-1)/2;i++) { 		
							ldspi_data_rwport.data_rw_data_port = (*(LDSPI_DataSRAM->SRAM_Value+2*i+1))+(*(LDSPI_DataSRAM->SRAM_Value+2*i+2)<<12);
							IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
						}
						if ((LDSPI_DataSRAM->SRAM_Length-1)%2==1) {/*length is even, read 24 bits, combine last data to MSB12bits*/
							ldspi_data_addr_ctrl.data_rw_pointer_set = ((LDSPI_DataSRAM->SRAM_Position+1)+(LDSPI_DataSRAM->SRAM_Length-1))/2;
							IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
							dummy = IoReg_Read32(LDSPI_Data_Rwport_reg);
							ldspi_data_rwport.regValue = IoReg_Read32(LDSPI_Data_Rwport_reg);
						
							ldspi_data_rwport.data_rw_data_port = (*(LDSPI_DataSRAM->SRAM_Value+(LDSPI_DataSRAM->SRAM_Length-1)))+((ldspi_data_rwport.data_rw_data_port&0xfff)<<12);
											
		
							ldspi_data_addr_ctrl.data_rw_pointer_set = ((LDSPI_DataSRAM->SRAM_Position+1)+(LDSPI_DataSRAM->SRAM_Length-1))/2;
							IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
							IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
					}				
		
				}

				return;
			}
		} while(--timeoutcnt != 0);
		
		if(timeoutcnt == 0){
			rtd_pr_vpq_emerg("drvif_color_set_LDSPI_DataSRAM_Data_Continuous, timeoutcnt == 0\n");
		}
	} else {/*tx_done*/
		ldspi_data_addr_ctrl.regValue = IoReg_Read32(LDSPI_Data_Addr_CTRL_reg);

		if (LDSPI_DataSRAM->SRAM_Position%2==0) {/*SRAM_Position is even*/
			ldspi_data_addr_ctrl.data_rw_pointer_set = LDSPI_DataSRAM->SRAM_Position/2;
			IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
			for (i=0;i<LDSPI_DataSRAM->SRAM_Length/2;i++) {			
				ldspi_data_rwport.data_rw_data_port = (*(LDSPI_DataSRAM->SRAM_Value+2*i))+(*(LDSPI_DataSRAM->SRAM_Value+2*i+1)<<12);
				IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
			}
			if (LDSPI_DataSRAM->SRAM_Length%2==1) {/*length is odd, read 24 bits, combine last data to MSB12bits*/
				ldspi_data_addr_ctrl.data_rw_pointer_set = (LDSPI_DataSRAM->SRAM_Position+LDSPI_DataSRAM->SRAM_Length)/2;
				IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
				dummy = IoReg_Read32(LDSPI_Data_Rwport_reg);
				ldspi_data_rwport.regValue= IoReg_Read32(LDSPI_Data_Rwport_reg);
				ldspi_data_rwport.data_rw_data_port = (*(LDSPI_DataSRAM->SRAM_Value+LDSPI_DataSRAM->SRAM_Length-1))+((ldspi_data_rwport.data_rw_data_port&0xfff)<<12);
				
				ldspi_data_addr_ctrl.data_rw_pointer_set = (LDSPI_DataSRAM->SRAM_Position+LDSPI_DataSRAM->SRAM_Length)/2;
				IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
				IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
			}
		} else { /* SRAM_Position is odd*/
				/*handel first data*/
				ldspi_data_addr_ctrl.data_rw_pointer_set = LDSPI_DataSRAM->SRAM_Position/2;
				IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
				dummy = IoReg_Read32(LDSPI_Data_Rwport_reg);
				ldspi_data_rwport.regValue = IoReg_Read32(LDSPI_Data_Rwport_reg);
				ldspi_data_rwport.data_rw_data_port = *(LDSPI_DataSRAM->SRAM_Value)+(ldspi_data_rwport.data_rw_data_port&0xfff000);	
				
				ldspi_data_addr_ctrl.data_rw_pointer_set = LDSPI_DataSRAM->SRAM_Position/2;
				IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
				IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
				
				/*handle following data*/
				ldspi_data_addr_ctrl.data_rw_pointer_set = (LDSPI_DataSRAM->SRAM_Position+1)/2;
				IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
				for (i=0;i<(LDSPI_DataSRAM->SRAM_Length-1)/2;i++) {			
					ldspi_data_rwport.data_rw_data_port = (*(LDSPI_DataSRAM->SRAM_Value+2*i+1))+(*(LDSPI_DataSRAM->SRAM_Value+2*i+2)<<12);
					IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
				}
				if ((LDSPI_DataSRAM->SRAM_Length-1)%2==1) {/*length is even, read 24 bits, combine last data to MSB12bits*/
					ldspi_data_addr_ctrl.data_rw_pointer_set = ((LDSPI_DataSRAM->SRAM_Position+1)+(LDSPI_DataSRAM->SRAM_Length-1))/2;
					IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
					dummy = IoReg_Read32(LDSPI_Data_Rwport_reg);
					ldspi_data_rwport.regValue = IoReg_Read32(LDSPI_Data_Rwport_reg);
				
					ldspi_data_rwport.data_rw_data_port = (*(LDSPI_DataSRAM->SRAM_Value+(LDSPI_DataSRAM->SRAM_Length-1)))+((ldspi_data_rwport.data_rw_data_port&0xfff)<<12);
									

					ldspi_data_addr_ctrl.data_rw_pointer_set = ((LDSPI_DataSRAM->SRAM_Position+1)+(LDSPI_DataSRAM->SRAM_Length-1))/2;
					IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
					IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
			}				

		}

	}
}



void drvif_color_get_LD_Block_Max_Info(unsigned short *LD_BlockMax, unsigned short start_addr, unsigned short length, unsigned short *global_max_ave, unsigned short *max_high_low_score)
{

	blu_ld_apl_read_max_1_RBUS blu_ld_apl_read_max_1_reg;
	blu_ld_apl_read_max_2_RBUS blu_ld_apl_read_max_2_reg;
	blu_ld_apl_read_max_3_RBUS blu_ld_apl_read_max_3_reg;

	unsigned int i = 0;
	unsigned int sum, high_low_sum = 0;
	unsigned short Hnum, Vnum = 0;
	Hnum = 60;
	Vnum = 32;
	sum = 0;
	high_low_sum = 0;
	
	if (LD_BlockMax==NULL || global_max_ave==NULL || max_high_low_score==NULL || length ==0 || length > LD_Max_Num)
		return;			
	
	blu_ld_apl_read_max_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_1_reg);
	blu_ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);
	blu_ld_apl_read_max_1_reg.ld_aplmax_conti = 1;
	blu_ld_apl_read_max_1_reg.ld_aplmax_start = 1;

	blu_ld_apl_read_max_2_reg.ld_aplmax_addr = start_addr;
	blu_ld_apl_read_max_2_reg.ld_aplmax_access_en = 1;

	IoReg_Write32(BLU_LD_APL_read_max_1_reg, blu_ld_apl_read_max_1_reg.regValue);
	IoReg_Write32(BLU_LD_APL_read_max_2_reg, blu_ld_apl_read_max_2_reg.regValue);
	
	for (i=0; i<(length+1)/2 ;i++) {
		blu_ld_apl_read_max_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_3_reg);
		*(LD_BlockMax+2*i) = (blu_ld_apl_read_max_3_reg.ld_aplmaxaccess_data) & 0x3ff;
		*(LD_BlockMax+2*i+1) = ((blu_ld_apl_read_max_3_reg.ld_aplmaxaccess_data)>>10) & 0x3ff;
		sum = sum + *(LD_BlockMax+2*i) + *(LD_BlockMax+2*i+1);
		if(i%(Hnum/2)==0) /*need to consider odd block number condition later*/
			high_low_sum = high_low_sum + ABS((int)*(LD_BlockMax+2*i+1),(int)*(LD_BlockMax+2*i));
		else {
			high_low_sum = high_low_sum + ABS((int)*(LD_BlockMax+2*i),(int)*(LD_BlockMax+2*i-1));
			high_low_sum = high_low_sum + ABS((int)*(LD_BlockMax+2*i+1),(int)*(LD_BlockMax+2*i));
		}
		if(i/(Hnum/2) != 0) {
			high_low_sum = high_low_sum + ABS((int)*(LD_BlockMax+2*i),(int)*(LD_BlockMax+2*i-Hnum));
			high_low_sum = high_low_sum + ABS((int)*(LD_BlockMax+2*i+1),(int)*(LD_BlockMax+2*i+1-Hnum));			
		}		
	}

	*global_max_ave = sum/length;
	*max_high_low_score = high_low_sum/((Hnum-1)*Vnum + Hnum*(Vnum-1));
		
	blu_ld_apl_read_max_2_reg.ld_aplmax_access_en = 0;
	IoReg_Write32(BLU_LD_APL_read_max_2_reg, blu_ld_apl_read_max_2_reg.regValue);

}

void drvif_color_get_LD_Block_Ave_Info(unsigned short *LD_BlockAve, unsigned short start_addr, unsigned short length, unsigned short *global_ave)
{

	blu_ld_apl_read_ave_1_RBUS blu_ld_apl_read_ave_1_reg;
	blu_ld_apl_read_ave_2_RBUS blu_ld_apl_read_ave_2_reg;
	blu_ld_apl_read_ave_3_RBUS blu_ld_apl_read_ave_3_reg;

	unsigned int i = 0;
	unsigned int sum = 0;
			
	if (LD_BlockAve==NULL || global_ave==NULL || length ==0 || length > LD_Max_Num)
		return;

	blu_ld_apl_read_ave_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_1_reg);
	blu_ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);
	blu_ld_apl_read_ave_1_reg.ld_aplave_conti = 1;
	blu_ld_apl_read_ave_1_reg.ld_aplave_start = 1;

	blu_ld_apl_read_ave_2_reg.ld_aplave_addr = start_addr;
	blu_ld_apl_read_ave_2_reg.ld_aplave_access_en = 1;

	IoReg_Write32(BLU_LD_APL_read_ave_1_reg, blu_ld_apl_read_ave_1_reg.regValue);
	IoReg_Write32(BLU_LD_APL_read_ave_2_reg, blu_ld_apl_read_ave_2_reg.regValue);

	for (i=0; i<(length+1)/2 ;i++) {
		blu_ld_apl_read_ave_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_3_reg);
		*(LD_BlockAve+2*i) = (blu_ld_apl_read_ave_3_reg.ld_aplaveaccess_data) & 0x3ff;
		*(LD_BlockAve+2*i+1) = ((blu_ld_apl_read_ave_3_reg.ld_aplaveaccess_data)>>10) & 0x3ff;
		sum = sum + *(LD_BlockAve+2*i) + *(LD_BlockAve+2*i+1);
	}

	*global_ave = sum/length;
	blu_ld_apl_read_ave_2_reg.ld_aplave_access_en = 0;
	IoReg_Write32(BLU_LD_APL_read_ave_2_reg, blu_ld_apl_read_ave_2_reg.regValue);

}

/*LDSPI Function*/
void drvif_color_set_LDSPI_TXDone_ISR_En(unsigned char en)
{
	ldspi_rx_data_ctrl_RBUS ldspi_rx_data_ctrl_reg;
	ldspi_rx_data_ctrl_reg.regValue = IoReg_Read32(LDSPI_RX_data_CTRL_reg);
	ldspi_rx_data_ctrl_reg.tx_done_int_en = en;
	IoReg_Write32(LDSPI_RX_data_CTRL_reg, ldspi_rx_data_ctrl_reg.regValue);
}




