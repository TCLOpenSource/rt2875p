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
#include <rtk_kdriver/io.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"
#include <mach/rtk_log.h>

#define LocalDimming_Debug 0
#define TAG_NAME "VPQSMARTLD"
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_LD_DEBUG,fmt,##args)

//static unsigned char localDimmingFullSet = 0;
LED_BV_max_avg_table max_avg_table;
short AlgoSRAMTmp[4096] = {0};
void drvif_color_set_LD_interrupt_Enable(unsigned char enable)
{
	blu_ld_smartld_int_RBUS blu_ld_smartld_int_reg;

	blu_ld_smartld_int_reg.regValue = IoReg_Read32(BLU_LD_SMARTLD_int_reg);
	blu_ld_smartld_int_reg.hw_blight_end_ie2 = 1;

	IoReg_Write32(BLU_LD_SMARTLD_int_reg, blu_ld_smartld_int_reg.regValue);
}

/* Function Description : This function is for LD enable*/
void drvif_color_set_LD_Enable(unsigned char enable)
{
	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);

	blu_ld_global_ctrl2.ld_valid = enable;

	#ifdef LocalDimming_Debug
	VIPprintf("ld_valid = %d\n", blu_ld_global_ctrl2.ld_valid);
	#endif

	IoReg_Write32(BLU_LD_Global_Ctrl2_reg, blu_ld_global_ctrl2.regValue);

}

//For calculate repeat and large
void drvif_color_Set_LD_SimplifyVal(int *den_val, int *mole_val)
{
    bool simpfy_flag = false;
	int i = 0;
	
    while(!simpfy_flag)
    {
        for(i = *mole_val;i>0;i--)
        {
            if(i == 1)
            {
                simpfy_flag = true;
                break;
            }

            if(((*den_val % i) ==0)  && ((*mole_val % i) ==0) )
            {
                *den_val = *den_val/i;
                *mole_val = *mole_val/i;
                break;
            }
        }
    }
}

void drvif_color_set_LD_Global_Ctrl(DRV_LD_Global_Ctrl *ptr)
{
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
	blu_ld_global_ctrl1_RBUS blu_ld_global_ctrl1;
	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	blu_ld_global_ctrl3_RBUS blu_ld_global_ctrl3;
	blu_ld_global_ctrl5_RBUS blu_ld_global_ctrl5;
	unsigned short spinBox_Width = 3840;	
	unsigned short spinBox_Height = 1920;
	int in_hize, in_vize, hsample, h_mole_val, v_mole_val;
	int h_large_val, h_repeat_val, v_large_val, v_repeat_val;

	VIPprintf("ld_valid = %d\n", ptr->ld_valid);

	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
	blu_ld_global_ctrl1.regValue = IoReg_Read32(BLU_LD_Global_Ctrl1_reg);
	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	blu_ld_global_ctrl3.regValue = IoReg_Read32(BLU_LD_Global_Ctrl3_reg);


	blu_ld_global_ctrl1.ld_db_en = ptr->ld_db_en;
	blu_ld_global_ctrl1.ld_db_read_level = ptr->ld_db_read_level;
	IoReg_Write32(BLU_LD_Global_Ctrl1_reg, blu_ld_global_ctrl1.regValue);
	blu_ld_global_ctrl2.ld_blk_hnum = ptr->ld_blk_hnum;
	blu_ld_global_ctrl2.ld_blk_vnum = ptr->ld_blk_vnum;
	//blu_ld_global_ctrl2.ld_gdim_mode = ptr->ld_gdim_mode; // remove
	blu_ld_global_ctrl2.ld_blk_type = ptr->ld_blk_type;
	//blu_ld_global_ctrl2.ld_hist_mode = ptr->ld_hist_mode; // remove
	blu_ld_global_ctrl2.ld_tenable = ptr->ld_tenable;
	blu_ld_global_ctrl2.ld_valid = ptr->ld_valid;
	blu_ld_global_ctrl3.ld_blk_hsize = ptr->ld_blk_hsize;
	blu_ld_global_ctrl3.ld_blk_vsize = ptr->ld_blk_vsize;

	in_hize = spinBox_Width;
	in_vize = spinBox_Height;
	// calculate repeat len
	if(ptr->ld_blk_type == 0)
	{
		// set H value
		hsample = (in_hize / (ptr->ld_blk_hnum + 1)) / 2;
	    h_mole_val = hsample * (ptr->ld_blk_hnum + 1);

		drvif_color_Set_LD_SimplifyVal(&in_hize, &h_mole_val);
		
		h_large_val = in_hize % h_mole_val;
		h_repeat_val = h_mole_val;

	    if(h_repeat_val > 255)
	    {
	        h_large_val = h_large_val /(h_repeat_val/256);
	        h_repeat_val = 255;
	    }

		// set V value
	    
		v_mole_val = ptr->ld_blk_vnum + 1;
		drvif_color_Set_LD_SimplifyVal(&in_vize, &v_mole_val);

		v_large_val = in_vize % v_mole_val;
	    v_repeat_val = v_mole_val;

	    if(v_repeat_val > 255)
	    {
	        v_large_val = v_large_val/(v_repeat_val/256);
	        v_repeat_val = 255;
	    }
	}
	else if(ptr->ld_blk_type == 1)
	{
		hsample = (in_hize / (ptr->ld_blk_hnum + 1))/ 2;
	    h_mole_val = hsample * (ptr->ld_blk_hnum + 1);

		drvif_color_Set_LD_SimplifyVal(&in_hize, &h_mole_val);

	    h_large_val = in_hize % h_mole_val;
	    h_repeat_val = h_mole_val;
		

	    if(h_repeat_val > 255)
	    {
	        h_large_val = h_large_val /(h_repeat_val/256);
	        h_repeat_val = 255;
	    }
	    
	    v_mole_val = (ptr->ld_blk_vnum + 1)*4;// 4subblk
		drvif_color_Set_LD_SimplifyVal(&in_vize, &v_mole_val);

		v_large_val = in_vize % v_mole_val;
	    v_repeat_val = v_mole_val;

	    if(v_repeat_val > 255)
	    {
	        v_large_val = v_large_val/(v_repeat_val/256);
	        v_repeat_val = 255;
	    }
	}
	else
	{
		hsample = (in_hize / (ptr->ld_blk_hnum + 1))/4/2;// 4subblk
	    h_mole_val = hsample * (ptr->ld_blk_hnum + 1);

		drvif_color_Set_LD_SimplifyVal(&in_hize, &h_mole_val);

	    h_large_val = in_hize % h_mole_val;
	    h_repeat_val = h_mole_val;
		
	    if(h_repeat_val > 256)
	    {
	        h_large_val = h_large_val /(h_repeat_val/256);
	        h_repeat_val = 256;
	    }
	    
	    v_mole_val = (ptr->ld_blk_vnum + 1);
		drvif_color_Set_LD_SimplifyVal(&in_vize, &v_mole_val);

		v_large_val = in_vize % v_mole_val;
	    v_repeat_val = v_mole_val;
	    if(v_repeat_val > 256)
	    {
	        v_large_val = v_large_val/(v_repeat_val/256);
	        v_repeat_val = 256;
	    }
	}
	
	blu_ld_global_ctrl5.ld_h_large_len = h_large_val;
	blu_ld_global_ctrl5.ld_h_repeat_len =  h_repeat_val;
    blu_ld_global_ctrl5.ld_v_large_len = v_large_val;
    blu_ld_global_ctrl5.ld_v_repeat_len = v_repeat_val;

	IoReg_Write32(BLU_LD_Global_Ctrl2_reg, blu_ld_global_ctrl2.regValue);
	IoReg_Write32(BLU_LD_Global_Ctrl3_reg, blu_ld_global_ctrl3.regValue);
	IoReg_Write32(BLU_LD_Global_Ctrl5_reg, blu_ld_global_ctrl5.regValue);

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
	ptr->ld_blk_hnum = blu_ld_global_ctrl2.ld_blk_hnum;
	ptr->ld_blk_vnum = blu_ld_global_ctrl2.ld_blk_vnum;
	//ptr->ld_gdim_mode = blu_ld_global_ctrl2.ld_gdim_mode;
	ptr->ld_blk_type = blu_ld_global_ctrl2.ld_blk_type;
	//ptr->ld_hist_mode = blu_ld_global_ctrl2.ld_hist_mode;
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

void drvif_color_set_LD_Spatial_Filter(DRV_LD_Spatial_Filter *ptr)
{
	//blu_ld_spatial_ctrl1_RBUS blu_ld_spatial_ctrl1;
	//blu_ld_spatial_ctrl2_RBUS blu_ld_spatial_ctrl2;
	blu_ld_spatial_ctrl5_RBUS blu_ld_spatial_ctrl5;
	blu_ld_spatial_ctrl6_RBUS blu_ld_spatial_ctrl6;
	blu_ld_spatial_ctrl7_RBUS blu_ld_spatial_ctrl7;
	blu_ld_spatial_ctrl8_RBUS blu_ld_spatial_ctrl8;
	blu_ld_spatial_ctrl9_RBUS blu_ld_spatial_ctrl9;
	blu_ld_spatial_ctrla_RBUS blu_ld_spatial_ctrla;

	//blu_ld_spatial_ctrl1.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl1_reg);
	//blu_ld_spatial_ctrl2.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl2_reg);
	blu_ld_spatial_ctrl5.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl5_reg);
	blu_ld_spatial_ctrl6.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl6_reg);
	blu_ld_spatial_ctrl7.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl7_reg);
	blu_ld_spatial_ctrl8.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl8_reg);
	blu_ld_spatial_ctrl9.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl9_reg);
	blu_ld_spatial_ctrla.regValue = IoReg_Read32(BLU_LD_Spatial_CtrlA_reg);
	/*
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
	*/
	blu_ld_spatial_ctrl5.ld_spatialnewcoef00 = ptr->ld_spatialcoef_div;/*add SpatialCoef_div*/
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

	//IoReg_Write32(BLU_LD_Spatial_Ctrl1_reg, blu_ld_spatial_ctrl1.regValue);
	//IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl5_reg, blu_ld_spatial_ctrl5.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl6_reg, blu_ld_spatial_ctrl6.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl7_reg, blu_ld_spatial_ctrl7.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl8_reg, blu_ld_spatial_ctrl8.regValue);
	IoReg_Write32(BLU_LD_Spatial_Ctrl9_reg, blu_ld_spatial_ctrl9.regValue);
	IoReg_Write32(BLU_LD_Spatial_CtrlA_reg, blu_ld_spatial_ctrla.regValue);
}

void drvif_color_get_LD_Spatial_Filter(DRV_LD_Spatial_Filter *ptr)
{
	//blu_ld_spatial_ctrl1_RBUS blu_ld_spatial_ctrl1;
	//blu_ld_spatial_ctrl2_RBUS blu_ld_spatial_ctrl2;
	blu_ld_spatial_ctrl5_RBUS blu_ld_spatial_ctrl5;
	blu_ld_spatial_ctrl6_RBUS blu_ld_spatial_ctrl6;
	blu_ld_spatial_ctrl7_RBUS blu_ld_spatial_ctrl7;
	blu_ld_spatial_ctrl8_RBUS blu_ld_spatial_ctrl8;
	blu_ld_spatial_ctrl9_RBUS blu_ld_spatial_ctrl9;
	blu_ld_spatial_ctrla_RBUS blu_ld_spatial_ctrla;

	//blu_ld_spatial_ctrl1.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl1_reg);
	//blu_ld_spatial_ctrl2.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl2_reg);
	blu_ld_spatial_ctrl5.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl5_reg);
	blu_ld_spatial_ctrl6.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl6_reg);
	blu_ld_spatial_ctrl7.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl7_reg);
	blu_ld_spatial_ctrl8.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl8_reg);
	blu_ld_spatial_ctrl9.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl9_reg);
	blu_ld_spatial_ctrla.regValue = IoReg_Read32(BLU_LD_Spatial_CtrlA_reg);
	/*
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
	*/
	ptr->ld_spatialcoef_div = blu_ld_spatial_ctrl5.ld_spatialnewcoef00;/*add SpatialCoef_div*/
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

/*new feature*/
void drvif_color_get_LD_Avg_Bv(DRV_LD_Average_Bv *ptr)
{
	blu_ld_avg_bv0_RBUS blu_ld_avg_bv0;
	blu_ld_avg_bv1_RBUS blu_ld_avg_bv1;
	blu_ld_avg_bv2_RBUS blu_ld_avg_bv2;
	blu_ld_avg_bv3_RBUS blu_ld_avg_bv3;

	blu_ld_avg_bv0.regValue = IoReg_Read32(BLU_LD_AVG_BV0_reg);
	blu_ld_avg_bv1.regValue = IoReg_Read32(BLU_LD_AVG_BV1_reg);
	blu_ld_avg_bv2.regValue = IoReg_Read32(BLU_LD_AVG_BV2_reg);
	blu_ld_avg_bv3.regValue = IoReg_Read32(BLU_LD_AVG_BV3_reg);

	ptr->ld_global_gain_avg = blu_ld_avg_bv0.ld_global_gain_avg_bv;
	ptr->ld_bpl_first_avg = blu_ld_avg_bv0.ld_bpl_first_avg_bv;
	ptr->ld_ai_brightness_avg = blu_ld_avg_bv1.ld_ai_brightness_avg_bv;
	ptr->ld_local_bv_gain_avg = blu_ld_avg_bv1.ld_local_bv_gain_avg_bv;
	ptr->ld_abi_avg = blu_ld_avg_bv2.ld_abi_avg_bv;
	ptr->ld_third_bpl_avg = blu_ld_avg_bv2.ld_third_bpl_avg_bv;
	ptr->ld_backlightboost_avg = blu_ld_avg_bv3.ld_backlightboost_avg_bv;

}

void drvif_color_set_LD_3x3LPF(DRV_LD_Backlight_LPF *ptr)
{
	blu_ld_bv_lpf_RBUS blu_ld_bv_lpf;

	blu_ld_bv_lpf.regValue = IoReg_Read32(BLU_LD_BV_LPF_reg);

	blu_ld_bv_lpf.ld_3x3lpf_en = ptr->ld_3x3lpf_en;
	blu_ld_bv_lpf.ld_3x3lpfcoef3 = ptr->ld_3x3lpfcoef3;
	blu_ld_bv_lpf.ld_3x3lpfcoef2 = ptr->ld_3x3lpfcoef2;
	blu_ld_bv_lpf.ld_3x3lpfcoef1 = ptr->ld_3x3lpfcoef1;
	blu_ld_bv_lpf.ld_3x3lpfcoef0 = ptr->ld_3x3lpfcoef0;

	IoReg_Write32(BLU_LD_BV_LPF_reg, blu_ld_bv_lpf.regValue);

}

void drvif_color_get_LD_3x3LPF(DRV_LD_Backlight_LPF *ptr)
{
	blu_ld_bv_lpf_RBUS blu_ld_bv_lpf;

	blu_ld_bv_lpf.regValue = IoReg_Read32(BLU_LD_BV_LPF_reg);

	ptr->ld_3x3lpf_en = blu_ld_bv_lpf.ld_3x3lpf_en;
	ptr->ld_3x3lpfcoef3 = blu_ld_bv_lpf.ld_3x3lpfcoef3;
	ptr->ld_3x3lpfcoef2 = blu_ld_bv_lpf.ld_3x3lpfcoef2;
	ptr->ld_3x3lpfcoef1 = blu_ld_bv_lpf.ld_3x3lpfcoef1;
	ptr->ld_3x3lpfcoef0 = blu_ld_bv_lpf.ld_3x3lpfcoef0;

}

void drvif_color_set_LD_Spatial_Remap3(DRV_LD_Spatial_Remap3 *ptr)
{
	blu_ld_bv_remap3_by_bpl0_RBUS blu_ld_bv_remap3_by_bpl0;
	blu_ld_bv_remap3_by_bpl1_RBUS blu_ld_bv_remap3_by_bpl1;
	blu_ld_bv_remap3_by_bpl2_RBUS blu_ld_bv_remap3_by_bpl2;
	blu_ld_bv_remap3_by_bpl3_RBUS blu_ld_bv_remap3_by_bpl3;
	blu_ld_bv_remap3_by_bpl4_RBUS blu_ld_bv_remap3_by_bpl4;
	blu_ld_bv_remap3_by_bpl5_RBUS blu_ld_bv_remap3_by_bpl5;
	blu_ld_bv_remap3_by_bpl6_RBUS blu_ld_bv_remap3_by_bpl6;
	blu_ld_bv_remap3_by_bpl7_RBUS blu_ld_bv_remap3_by_bpl7;
	blu_ld_bv_remap3_by_bpl8_RBUS blu_ld_bv_remap3_by_bpl8;

	blu_ld_bv_remap3_by_bpl0.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL0_reg);
	blu_ld_bv_remap3_by_bpl1.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL1_reg);
	blu_ld_bv_remap3_by_bpl2.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL2_reg);
	blu_ld_bv_remap3_by_bpl3.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL3_reg);
	blu_ld_bv_remap3_by_bpl4.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL4_reg);
	blu_ld_bv_remap3_by_bpl5.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL5_reg);
	blu_ld_bv_remap3_by_bpl6.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL6_reg);
	blu_ld_bv_remap3_by_bpl7.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL7_reg);
	blu_ld_bv_remap3_by_bpl8.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL8_reg);

	blu_ld_bv_remap3_by_bpl0.ld_backlight_remap3_en = ptr->ld_bl_remap3_en;
	blu_ld_bv_remap3_by_bpl0.ld_backlight_remap3_bv0 = ptr->ld_bl_remap3_bv0;
	blu_ld_bv_remap3_by_bpl0.ld_backlight_remap3_bv1 = ptr->ld_bl_remap3_bv1;
	blu_ld_bv_remap3_by_bpl1.ld_backlight_remap3_bv2 = ptr->ld_bl_remap3_bv2;
	blu_ld_bv_remap3_by_bpl1.ld_backlight_remap3_bv3 = ptr->ld_bl_remap3_bv3;
	blu_ld_bv_remap3_by_bpl2.ld_backlight_remap3_bv4 = ptr->ld_bl_remap3_bv4;
	blu_ld_bv_remap3_by_bpl2.ld_backlight_remap3_bv5 = ptr->ld_bl_remap3_bv5;
	blu_ld_bv_remap3_by_bpl3.ld_backlight_remap3_bv6 = ptr->ld_bl_remap3_bv6;
	blu_ld_bv_remap3_by_bpl3.ld_backlight_remap3_bv7 = ptr->ld_bl_remap3_bv7;
	blu_ld_bv_remap3_by_bpl4.ld_backlight_remap3_bv8 = ptr->ld_bl_remap3_bv8;
	blu_ld_bv_remap3_by_bpl4.ld_backlight_remap3_bv9 = ptr->ld_bl_remap3_bv9;
	blu_ld_bv_remap3_by_bpl5.ld_backlight_remap3_bv10 = ptr->ld_bl_remap3_bv10;
	blu_ld_bv_remap3_by_bpl5.ld_backlight_remap3_bv11 = ptr->ld_bl_remap3_bv11;
	blu_ld_bv_remap3_by_bpl6.ld_backlight_remap3_bv12 = ptr->ld_bl_remap3_bv12;
	blu_ld_bv_remap3_by_bpl6.ld_backlight_remap3_bv13 = ptr->ld_bl_remap3_bv13;
	blu_ld_bv_remap3_by_bpl7.ld_backlight_remap3_bv14 = ptr->ld_bl_remap3_bv14;
	blu_ld_bv_remap3_by_bpl7.ld_backlight_remap3_bv15 = ptr->ld_bl_remap3_bv15;
	blu_ld_bv_remap3_by_bpl8.ld_backlight_remap3_bv16 = ptr->ld_bl_remap3_bv16;

	IoReg_Write32(BLU_LD_BV_Remap3_by_BPL0_reg, blu_ld_bv_remap3_by_bpl0.regValue);
	IoReg_Write32(BLU_LD_BV_Remap3_by_BPL1_reg, blu_ld_bv_remap3_by_bpl1.regValue);
	IoReg_Write32(BLU_LD_BV_Remap3_by_BPL2_reg, blu_ld_bv_remap3_by_bpl2.regValue);
	IoReg_Write32(BLU_LD_BV_Remap3_by_BPL3_reg, blu_ld_bv_remap3_by_bpl3.regValue);
	IoReg_Write32(BLU_LD_BV_Remap3_by_BPL4_reg, blu_ld_bv_remap3_by_bpl4.regValue);
	IoReg_Write32(BLU_LD_BV_Remap3_by_BPL5_reg, blu_ld_bv_remap3_by_bpl5.regValue);
	IoReg_Write32(BLU_LD_BV_Remap3_by_BPL6_reg, blu_ld_bv_remap3_by_bpl6.regValue);
	IoReg_Write32(BLU_LD_BV_Remap3_by_BPL7_reg, blu_ld_bv_remap3_by_bpl7.regValue);
	IoReg_Write32(BLU_LD_BV_Remap3_by_BPL8_reg, blu_ld_bv_remap3_by_bpl8.regValue);

}

void drvif_color_get_LD_Spatial_Remap3(DRV_LD_Spatial_Remap3 *ptr)
{
	blu_ld_bv_remap3_by_bpl0_RBUS blu_ld_bv_remap3_by_bpl0;
	blu_ld_bv_remap3_by_bpl1_RBUS blu_ld_bv_remap3_by_bpl1;
	blu_ld_bv_remap3_by_bpl2_RBUS blu_ld_bv_remap3_by_bpl2;
	blu_ld_bv_remap3_by_bpl3_RBUS blu_ld_bv_remap3_by_bpl3;
	blu_ld_bv_remap3_by_bpl4_RBUS blu_ld_bv_remap3_by_bpl4;
	blu_ld_bv_remap3_by_bpl5_RBUS blu_ld_bv_remap3_by_bpl5;
	blu_ld_bv_remap3_by_bpl6_RBUS blu_ld_bv_remap3_by_bpl6;
	blu_ld_bv_remap3_by_bpl7_RBUS blu_ld_bv_remap3_by_bpl7;
	blu_ld_bv_remap3_by_bpl8_RBUS blu_ld_bv_remap3_by_bpl8;

	blu_ld_bv_remap3_by_bpl0.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL0_reg);
	blu_ld_bv_remap3_by_bpl1.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL1_reg);
	blu_ld_bv_remap3_by_bpl2.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL2_reg);
	blu_ld_bv_remap3_by_bpl3.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL3_reg);
	blu_ld_bv_remap3_by_bpl4.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL4_reg);
	blu_ld_bv_remap3_by_bpl5.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL5_reg);
	blu_ld_bv_remap3_by_bpl6.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL6_reg);
	blu_ld_bv_remap3_by_bpl7.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL7_reg);
	blu_ld_bv_remap3_by_bpl8.regValue = IoReg_Read32(BLU_LD_BV_Remap3_by_BPL8_reg);

	ptr->ld_bl_remap3_en = blu_ld_bv_remap3_by_bpl0.ld_backlight_remap3_en;
	ptr->ld_bl_remap3_bv0 = blu_ld_bv_remap3_by_bpl0.ld_backlight_remap3_bv0;
	ptr->ld_bl_remap3_bv1 = blu_ld_bv_remap3_by_bpl0.ld_backlight_remap3_bv1;
	ptr->ld_bl_remap3_bv2 = blu_ld_bv_remap3_by_bpl1.ld_backlight_remap3_bv2;
	ptr->ld_bl_remap3_bv3 = blu_ld_bv_remap3_by_bpl1.ld_backlight_remap3_bv3;
	ptr->ld_bl_remap3_bv4 = blu_ld_bv_remap3_by_bpl2.ld_backlight_remap3_bv4;
	ptr->ld_bl_remap3_bv5 = blu_ld_bv_remap3_by_bpl2.ld_backlight_remap3_bv5;
	ptr->ld_bl_remap3_bv6 = blu_ld_bv_remap3_by_bpl3.ld_backlight_remap3_bv6;
	ptr->ld_bl_remap3_bv7 = blu_ld_bv_remap3_by_bpl3.ld_backlight_remap3_bv7;
	ptr->ld_bl_remap3_bv8 = blu_ld_bv_remap3_by_bpl4.ld_backlight_remap3_bv8;
	ptr->ld_bl_remap3_bv9 = blu_ld_bv_remap3_by_bpl4.ld_backlight_remap3_bv9;
	ptr->ld_bl_remap3_bv10 = blu_ld_bv_remap3_by_bpl5.ld_backlight_remap3_bv10;
	ptr->ld_bl_remap3_bv11 = blu_ld_bv_remap3_by_bpl5.ld_backlight_remap3_bv11;
	ptr->ld_bl_remap3_bv12 = blu_ld_bv_remap3_by_bpl6.ld_backlight_remap3_bv12;
	ptr->ld_bl_remap3_bv13 = blu_ld_bv_remap3_by_bpl6.ld_backlight_remap3_bv13;
	ptr->ld_bl_remap3_bv14 = blu_ld_bv_remap3_by_bpl7.ld_backlight_remap3_bv14;
	ptr->ld_bl_remap3_bv15 = blu_ld_bv_remap3_by_bpl7.ld_backlight_remap3_bv15;
	ptr->ld_bl_remap3_bv16 = blu_ld_bv_remap3_by_bpl8.ld_backlight_remap3_bv16;

}

void drvif_color_set_LD_Global_BV_Gain(DRV_LD_Global_Gain *ptr)
{
	blu_ld_global_gain_RBUS blu_ld_global_gain;

	blu_ld_global_gain.regValue = IoReg_Read32(BLU_LD_GLOBAL_GAIN_reg);

	blu_ld_global_gain.ld_global_bvgain_en = ptr->ld_global_bvgain_en;
	blu_ld_global_gain.ld_global_bvgain = ptr->ld_global_bvgain;

	IoReg_Write32(BLU_LD_GLOBAL_GAIN_reg, blu_ld_global_gain.regValue);
}

void drvif_color_get_LD_Global_BV_Gain(DRV_LD_Global_Gain *ptr)
{
	blu_ld_global_gain_RBUS blu_ld_global_gain;

	blu_ld_global_gain.regValue = IoReg_Read32(BLU_LD_GLOBAL_GAIN_reg);

	ptr->ld_global_bvgain_en = blu_ld_global_gain.ld_global_bvgain_en;
	ptr->ld_global_bvgain = blu_ld_global_gain.ld_global_bvgain;

}

void drvif_color_set_LD_AI_BV_Gain(DRV_LD_Global_Gain *ptr)
{
	blu_ld_global_gain_RBUS blu_ld_global_gain;

	blu_ld_global_gain.regValue = IoReg_Read32(BLU_LD_GLOBAL_GAIN_reg);

	blu_ld_global_gain.ld_ai_global_bvgain_en = ptr->ld_ai_global_bvgain_en;
	blu_ld_global_gain.ld_ai_global_bvgain = ptr->ld_ai_global_bvgain;

	IoReg_Write32(BLU_LD_GLOBAL_GAIN_reg, blu_ld_global_gain.regValue);
}

void drvif_color_get_LD_AI_BV_Gain(DRV_LD_Global_Gain *ptr)
{
	blu_ld_global_gain_RBUS blu_ld_global_gain;

	blu_ld_global_gain.regValue = IoReg_Read32(BLU_LD_GLOBAL_GAIN_reg);

	ptr->ld_ai_global_bvgain_en = blu_ld_global_gain.ld_ai_global_bvgain_en;
	ptr->ld_ai_global_bvgain = blu_ld_global_gain.ld_ai_global_bvgain;

}

void drvif_color_set_LD_ABI_BV_Gain(DRV_LD_Global_Gain *ptr)
{
	blu_ld_global_gain_RBUS blu_ld_global_gain;

	blu_ld_global_gain.regValue = IoReg_Read32(BLU_LD_GLOBAL_GAIN_reg);

	blu_ld_global_gain.ld_abi_global_bvgain_en = ptr->ld_abi_global_bvgain_en;
	blu_ld_global_gain.ld_abi_global_bvgain = ptr->ld_abi_global_bvgain;

	IoReg_Write32(BLU_LD_GLOBAL_GAIN_reg, blu_ld_global_gain.regValue);
}

void drvif_color_get_LD_ABI_BV_Gain(DRV_LD_Global_Gain *ptr)
{
	blu_ld_global_gain_RBUS blu_ld_global_gain;

	blu_ld_global_gain.regValue = IoReg_Read32(BLU_LD_GLOBAL_GAIN_reg);

	ptr->ld_abi_global_bvgain_en = blu_ld_global_gain.ld_abi_global_bvgain_en;
	ptr->ld_abi_global_bvgain = blu_ld_global_gain.ld_abi_global_bvgain;

}

void drvif_color_set_LD_Spatial_Remap2(DRV_LD_Spatial_Remap2 *ptr)
{

	blu_ld_spatial_ctrl2_RBUS blu_ld_spatial_ctrl2;
	blu_ld_spatial_ctrl3_RBUS blu_ld_spatial_ctrl3;
	blu_ld_spatial_ctrlb_RBUS blu_ld_spatial_ctrlb;
	blu_ld_spatial_ctrlc_RBUS blu_ld_spatial_ctrlc;
	unsigned char i=0;

	blu_ld_spatial_ctrl2.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl2_reg);
	blu_ld_spatial_ctrl3.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl3_reg);
	blu_ld_spatial_ctrlb.regValue = IoReg_Read32(BLU_LD_Spatial_CtrlB_reg);
	blu_ld_spatial_ctrlc.regValue = IoReg_Read32(BLU_LD_Spatial_CtrlC_reg);

	blu_ld_spatial_ctrl2.ld_spatialremap2en = 0;
	IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);
	blu_ld_spatial_ctrlc.ld_spatialremapadr = 0x60;
	blu_ld_spatial_ctrlb.ld_spatialremap2tab_initial = ptr->ld_spatialremaptab2[0];
	IoReg_Write32(BLU_LD_Spatial_CtrlB_reg, blu_ld_spatial_ctrlb.regValue);
	IoReg_Write32(BLU_LD_Spatial_CtrlC_reg, blu_ld_spatial_ctrlc.regValue);


	for (i = 0; i < 32; i++) {
		blu_ld_spatial_ctrl3.ld_spatialremaptab0 = (ptr->ld_spatialremaptab2[2*i+1] > 1023) ? 1023 : ptr->ld_spatialremaptab2[2*i+1];
		blu_ld_spatial_ctrl3.ld_spatialremaptab1 = (ptr->ld_spatialremaptab2[2*i+2] > 1023) ? 1023 : ptr->ld_spatialremaptab2[2*i+2];
		IoReg_Write32(BLU_LD_Spatial_Ctrl3_reg, blu_ld_spatial_ctrl3.regValue);
	}
	IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);

	blu_ld_spatial_ctrl2.ld_spatialremap2en = ptr->ld_spatialremap2en;
	IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);
	

}

void drvif_color_set_LD_Pixel_Bv_Gain_Offset(DRV_LD_Bl_Pixel_Bv_Gain_Offset *ptr)
{
	blu_ld_pixel_bv_gain_RBUS blu_ld_pixel_bv_gain;

	blu_ld_pixel_bv_gain.regValue = IoReg_Read32(BLU_LD_PIXEL_BV_GAIN_reg);

    blu_ld_pixel_bv_gain.ld_pixel_bv_gain = ptr->ld_pixel_bv_gain;
	blu_ld_pixel_bv_gain.ld_pixel_bv_offset = ptr->ld_pixel_bv_offset;

	IoReg_Write32(BLU_LD_PIXEL_BV_GAIN_reg, blu_ld_pixel_bv_gain.regValue);

}

void drvif_color_get_LD_Pixel_Bv_Gain_Offset(DRV_LD_Bl_Pixel_Bv_Gain_Offset *ptr)
{
	blu_ld_pixel_bv_gain_RBUS blu_ld_pixel_bv_gain;

	blu_ld_pixel_bv_gain.regValue = IoReg_Read32(BLU_LD_PIXEL_BV_GAIN_reg);

    ptr->ld_pixel_bv_gain = blu_ld_pixel_bv_gain.ld_pixel_bv_gain;
	ptr->ld_pixel_bv_offset = blu_ld_pixel_bv_gain.ld_pixel_bv_offset;

}
void drvif_color_set_LD_Spatial_Remap(DRV_LD_Spatial_Remap *ptr)
{
	blu_ld_spatial_ctrl2_RBUS blu_ld_spatial_ctrl2;
	blu_ld_spatial_ctrl3_RBUS blu_ld_spatial_ctrl3;
	blu_ld_spatial_ctrl4_RBUS blu_ld_spatial_ctrl4;
	blu_ld_spatial_ctrlc_RBUS blu_ld_spatial_ctrlc;
	unsigned char i=0;

	blu_ld_spatial_ctrl2.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl2_reg);
	blu_ld_spatial_ctrl3.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl3_reg);
	blu_ld_spatial_ctrl4.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl4_reg);
	blu_ld_spatial_ctrlc.regValue = IoReg_Read32(BLU_LD_Spatial_CtrlC_reg);

	blu_ld_spatial_ctrl2.ld_spatialremapen = 0;
	IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);
	blu_ld_spatial_ctrlc.ld_spatialremapadr = 0x40;
	
	IoReg_Write32(BLU_LD_Spatial_CtrlC_reg, blu_ld_spatial_ctrlc.regValue);
	blu_ld_spatial_ctrl4.ld_spatialremaptab_initial = ptr->ld_spatialremaptab[0];
	IoReg_Write32(BLU_LD_Spatial_Ctrl4_reg, blu_ld_spatial_ctrl4.regValue);


	for (i = 0; i < 32; i++) {
		//blu_ld_spatial_ctrl3.ld_spatialremapadr = i;
		blu_ld_spatial_ctrl3.ld_spatialremaptab0 = (ptr->ld_spatialremaptab[2*i+1] > 1023) ? 1023 : ptr->ld_spatialremaptab[2*i+1];
		blu_ld_spatial_ctrl3.ld_spatialremaptab1 = (ptr->ld_spatialremaptab[2*i+2] > 1023) ? 1023 : ptr->ld_spatialremaptab[2*i+2];
		IoReg_Write32(BLU_LD_Spatial_Ctrl3_reg, blu_ld_spatial_ctrl3.regValue);
	}
	blu_ld_spatial_ctrl2.ld_spatialremapen = ptr->ld_spatialremapen;
	IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);
}

void drvif_color_set_LD_Boost(DRV_LD_Boost *ptr)
{
	blu_ld_spatial_ctrl2_RBUS blu_ld_spatial_ctrl2;
	blu_ld_spatial_ctrl3_RBUS blu_ld_spatial_ctrl3;
	blu_ld_spatial_ctrl4_RBUS blu_ld_spatial_ctrl4;
	blu_ld_spatial_ctrlc_RBUS blu_ld_spatial_ctrlc;
	blu_ld_boost_RBUS blu_ld_boost;
	unsigned char i=0;

	blu_ld_spatial_ctrl2.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl2_reg);
	blu_ld_spatial_ctrl3.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl3_reg);
	blu_ld_spatial_ctrl4.regValue = IoReg_Read32(BLU_LD_Spatial_Ctrl4_reg);
	blu_ld_spatial_ctrlc.regValue = IoReg_Read32(BLU_LD_Spatial_CtrlC_reg);
	blu_ld_boost.regValue = IoReg_Read32(BLU_LD_Boost_reg);

	//blu_ld_spatial_ctrl2.ld_backlightboosten= ptr->ld_backlightboosten;
	blu_ld_spatial_ctrl2.ld_backlightboosten= 0;
	IoReg_Write32(BLU_LD_Spatial_Ctrl2_reg, blu_ld_spatial_ctrl2.regValue);
	
	blu_ld_spatial_ctrl4.ld_boostgain_initial= ptr->ld_boost_gain_lut[0];
	blu_ld_spatial_ctrl4.ld_spatialremaptab_initial= ptr->ld_boost_curve_lut[0];
	IoReg_Write32(BLU_LD_Spatial_Ctrl4_reg, blu_ld_spatial_ctrl4.regValue);

	/*k24 add*/
	blu_ld_boost.ld_boostgainfunc = ptr->ld_boostgainfunc;
	blu_ld_boost.ld_boost_gain_shift = ptr->ld_boost_gain_shift;
	blu_ld_boost.ld_boost_input_bv_shift = ptr->ld_boost_input_bv_shift;
	IoReg_Write32(BLU_LD_Boost_reg, blu_ld_boost.regValue);

	blu_ld_spatial_ctrlc.ld_spatialremapadr = 0;
	IoReg_Write32(BLU_LD_Spatial_CtrlC_reg, blu_ld_spatial_ctrlc.regValue);

	/* Gain LUT*/
	for (i = 0; i < 32; i++) {
		//blu_ld_spatial_ctrl3.ld_spatialremapadr = i+32;
		blu_ld_spatial_ctrl3.ld_spatialremaptab0 = (ptr->ld_boost_gain_lut[2*i+1] > 1023) ? 1023 : ptr->ld_boost_gain_lut[2*i+1];
		blu_ld_spatial_ctrl3.ld_spatialremaptab1 = (ptr->ld_boost_gain_lut[2*i+2] > 1023) ? 1023 : ptr->ld_boost_gain_lut[2*i+2];
		IoReg_Write32(BLU_LD_Spatial_Ctrl3_reg, blu_ld_spatial_ctrl3.regValue);
	}
	blu_ld_spatial_ctrlc.ld_spatialremapadr = 0x20;
	IoReg_Write32(BLU_LD_Spatial_CtrlC_reg, blu_ld_spatial_ctrlc.regValue);

	/* Boost LUT*/
	for (i = 0; i < 32; i++) {
		//blu_ld_spatial_ctrl3.ld_spatialremapadr = i+64;
		blu_ld_spatial_ctrl3.ld_spatialremaptab0 = (ptr->ld_boost_curve_lut[2*i+1] > 65535) ? 65535 : ptr->ld_boost_curve_lut[2*i+1];
		blu_ld_spatial_ctrl3.ld_spatialremaptab1 = (ptr->ld_boost_curve_lut[2*i+2] > 65535) ? 65535 : ptr->ld_boost_curve_lut[2*i+2];
		IoReg_Write32(BLU_LD_Spatial_Ctrl3_reg, blu_ld_spatial_ctrl3.regValue);
	}
	blu_ld_spatial_ctrl2.ld_backlightboosten= ptr->ld_backlightboosten;
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

void drvif_color_set_LD_Backlight_Final_Decision(DRV_LD_Backlight_Final_Decision *ptr)
{
	blu_ld_backlight_ctrl1_RBUS blu_ld_backlight_ctrl1;
	blu_ld_backlight_ctrl2_RBUS blu_ld_backlight_ctrl2;/*Merlin8 add*/
	blu_ld_global_ctrl1_RBUS blu_ld_global_ctrl1; /*for double buffer apply*/

	blu_ld_backlight_ctrl1.regValue = IoReg_Read32(BLU_LD_Backlight_Ctrl1_reg);
	blu_ld_global_ctrl1.regValue = IoReg_Read32(BLU_LD_Global_Ctrl1_reg);

	blu_ld_backlight_ctrl1.ld_blu_wr_num = ptr->ld_blu_wr_num;
	//blu_ld_backlight_ctrl1.ld_blu_spitotal = ptr->ld_blu_spitotal;
	blu_ld_backlight_ctrl1.ld_blu_spien = ptr->ld_blu_spien;
	blu_ld_backlight_ctrl1.ld_blu_nodim = ptr->ld_blu_nodim;
	blu_ld_backlight_ctrl1.ld_blu_usergain = ptr->ld_blu_usergain;
	blu_ld_backlight_ctrl2.ld_blu_spitotal = ptr->ld_blu_spitotal;
	blu_ld_backlight_ctrl2.ld_blu_gain_clamp = ptr->ld_blu_gain_clamp;
	blu_ld_backlight_ctrl2.ld_blu_gain_shift_right = ptr->ld_blu_gain_shift_right;
	blu_ld_backlight_ctrl2.ld_blu_gain_shift = ptr->ld_blu_gain_shift;

	IoReg_Write32(BLU_LD_Backlight_Ctrl1_reg, blu_ld_backlight_ctrl1.regValue);
	IoReg_Write32(BLU_LD_Backlight_Ctrl2_reg, blu_ld_backlight_ctrl2.regValue);/*Merlin8 add*/

	/*double buffer apply*/
	blu_ld_global_ctrl1.ld_db_apply = 1;
	IoReg_Write32(BLU_LD_Global_Ctrl1_reg, blu_ld_global_ctrl1.regValue);
}

void drvif_color_get_LD_Backlight_Final_Decision(DRV_LD_Backlight_Final_Decision *ptr)
{

	blu_ld_backlight_ctrl1_RBUS blu_ld_backlight_ctrl1;
	blu_ld_backlight_ctrl2_RBUS blu_ld_backlight_ctrl2;/*Merlin8 add*/

	blu_ld_backlight_ctrl1.regValue = IoReg_Read32(BLU_LD_Backlight_Ctrl1_reg);
	blu_ld_backlight_ctrl2.regValue = IoReg_Read32(BLU_LD_Backlight_Ctrl2_reg);

	ptr->ld_blu_wr_num = blu_ld_backlight_ctrl1.ld_blu_wr_num;
	//ptr->ld_blu_spitotal = blu_ld_backlight_ctrl1.ld_blu_spitotal;
	ptr->ld_blu_spitotal = blu_ld_backlight_ctrl2.ld_blu_spitotal;
	ptr->ld_blu_spien = blu_ld_backlight_ctrl1.ld_blu_spien;
	ptr->ld_blu_nodim = blu_ld_backlight_ctrl1.ld_blu_nodim;
	ptr->ld_blu_usergain = blu_ld_backlight_ctrl1.ld_blu_usergain;
	ptr->ld_blu_gain_clamp = blu_ld_backlight_ctrl2.ld_blu_gain_clamp;
	ptr->ld_blu_gain_shift_right = blu_ld_backlight_ctrl2.ld_blu_gain_shift_right;
	ptr->ld_blu_gain_shift = blu_ld_backlight_ctrl2.ld_blu_gain_shift;
	

}


void drvif_color_set_LD_Data_Compensation(DRV_LD_Data_Compensation *ptr)
{
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
	blu_ld_data_comp_ctrl_RBUS blu_ld_data_comp_ctrl;
	blu_ld_datacomp_ctrl1_RBUS blu_ld_datacomp_ctrl1;
	blu_ld_datacomp_ctrl2_RBUS blu_ld_datacomp_ctrl2;
	blu_ld_uv_compensation_RBUS blu_ld_uv_compensation;

	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
	blu_ld_data_comp_ctrl.regValue = IoReg_Read32(BLU_LD_DATA_COMP_CTRL_reg);
	blu_ld_datacomp_ctrl1.regValue = IoReg_Read32(BLU_LD_DataComp_Ctrl1_reg);
	blu_ld_datacomp_ctrl2.regValue = IoReg_Read32(BLU_LD_DataComp_Ctrl2_reg);
	blu_ld_uv_compensation.regValue = IoReg_Read32(BLU_LD_uv_compensation_reg);

	blu_ld_data_comp_ctrl.ld_data_comp_mode = ptr->ld_data_comp_mode;
	blu_ld_datacomp_ctrl1.ld_comp_satrange = ptr->ld_comp_satrange;
	blu_ld_datacomp_ctrl1.ld_comp_softmax = ptr->ld_comp_softmax;
	blu_ld_datacomp_ctrl1.ld_comp_minlimit = ptr->ld_comp_minlimit;
	blu_ld_datacomp_ctrl1.ld_softcomp_gain = ptr->ld_softcomp_gain;
	blu_ld_datacomp_ctrl1.ld_comp_maxmode = ptr->ld_comp_maxmode;
	blu_ld_datacomp_ctrl2.ld_comp_inv_shift = ptr->ld_comp_inv_shift;
	blu_ld_datacomp_ctrl2.ld_comp_inv_mode = ptr->ld_comp_inv_mode;
	blu_ld_datacomp_ctrl2.ld_srcgainsel = ptr->ld_srcgainsel;
	blu_ld_uv_compensation.uv_compensation_en = ptr->ld_uv_comp_en; // Merlin6 new
	blu_ld_uv_compensation.color_gain_coeff = ptr->ld_uv_comp_coeff; // Merlin6 new

	IoReg_Write32(BLU_LD_DATA_COMP_CTRL_reg, blu_ld_data_comp_ctrl.regValue);
	IoReg_Write32(BLU_LD_DataComp_Ctrl1_reg, blu_ld_datacomp_ctrl1.regValue);
	IoReg_Write32(BLU_LD_DataComp_Ctrl2_reg, blu_ld_datacomp_ctrl2.regValue);
	IoReg_Write32(BLU_LD_uv_compensation_reg, blu_ld_uv_compensation.regValue);

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
	blu_ld_data_comp_ctrl.ld_lut_ax_sel = 2;
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
#ifdef _VIP_Mer7_Compile_Error__
	blu_ld_inter_hctrl2.ld_hinitphase3_right = ptr->ld_hinitphase3_right;
#endif
	blu_ld_inter_hctrl2.ld_hboundary = ptr->ld_hboundary;
	blu_ld_inter_hctrl2.ld_hinitphase1 = ptr->ld_hinitphase1;
	blu_ld_inter_hctrl2.ld_hinitphase2 = ptr->ld_hinitphase2;
	blu_ld_inter_hctrl2.ld_hinitphase3_left = ptr->ld_hinitphase3_left;
	blu_ld_inter_vctrl1.ld_tab_vsize = ptr->ld_tab_vsize;
	blu_ld_inter_vctrl1.ld_vfactor = ptr->ld_vfactor;
#ifdef _VIP_Mer7_Compile_Error__
	blu_ld_inter_vctrl2.ld_vinitphase3_right = ptr->ld_vinitphase3_right;
#endif
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

unsigned char drvif_color_get_LD_APL_ave( unsigned int* APL_data )
{
    int i,j;
    unsigned short hnum, vnum;
    unsigned short block_addr;
    blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
    blu_ld_apl_read_ave_1_RBUS ld_apl_read_ave_1_reg;
    blu_ld_apl_read_ave_2_RBUS ld_apl_read_ave_2_reg;
    blu_ld_apl_read_ave_3_RBUS ld_apl_read_ave_3_reg;
	blu_ld_apl_read_ave_4_RBUS ld_apl_read_ave_4_reg;
    unsigned char LD_type, LD_Hnum, LD_Vnum;
    unsigned char valid;
    unsigned short access_ptr;
    unsigned short mdata;
    unsigned short ldata;
    unsigned short timeout;

    ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
    LD_type = ld_global_ctrl2_reg.ld_blk_type;
    LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum;
    LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum;

    if( LD_type == 0 ) // direct
    {
        hnum = LD_Hnum+1;
        vnum = LD_Vnum+1;
    }
    else if( LD_type == 1 ) // edge top/down
    {
        hnum = LD_Hnum+1;
        vnum = (LD_Vnum+1)*4;
    }
    else // edge left/right
    {
        hnum = (LD_Hnum+1)*4;
        vnum = LD_Vnum+1;
    }

    ld_apl_read_ave_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_1_reg);
    ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);

    ld_apl_read_ave_2_reg.ld_aplaveaccess_even_line = 1;
	ld_apl_read_ave_2_reg.ld_aplaveaccess_delta = 0;
	ld_apl_read_ave_2_reg.ld_aplaveaccess_offset = 0;
    ld_apl_read_ave_2_reg.ld_aplave_access_en = 1;

    ld_apl_read_ave_1_reg.ld_aplave_conti = 1;
    ld_apl_read_ave_1_reg.ld_aplave_start = 1;

    IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);
    IoReg_Write32(BLU_LD_APL_read_ave_1_reg, ld_apl_read_ave_1_reg.regValue);

    for( i=0; i<vnum; i++ )
    {
        for( j=0; j<((hnum+1)>>1); j++ )
        {
            timeout = 200;
            do {
                ld_apl_read_ave_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_3_reg);
                valid = (ld_apl_read_ave_3_reg.regValue & 0x80000000) >> 31;
                timeout--;
                if( timeout == 0 )
                    return 0;
            } while( !valid );
			ld_apl_read_ave_4_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_4_reg);
            access_ptr = (ld_apl_read_ave_4_reg.regValue & 0xFFF);
            mdata = (ld_apl_read_ave_3_reg.regValue & 0x000FFC00) >> 10;
            ldata = (ld_apl_read_ave_3_reg.regValue & 0x000003FF);

            if( i*((hnum+1)>>1)+j != access_ptr ) // access ptr wrong
                return 0;

            block_addr = i*hnum + j*2;
            APL_data[block_addr] = ldata;
            if( 2*j+1 < hnum )
                APL_data[block_addr+1] = mdata;
        }
    }

    ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);
    ld_apl_read_ave_2_reg.ld_aplave_access_en = 0;
    IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);

	return 1;
}

unsigned char drvif_color_get_LD_APL_max( unsigned int* APL_data )
{
    int i,j;
    unsigned short hnum, vnum;
    unsigned short block_addr;
    blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
    blu_ld_apl_read_max_1_RBUS ld_apl_read_max_1_reg;
    blu_ld_apl_read_max_2_RBUS ld_apl_read_max_2_reg;
    blu_ld_apl_read_max_3_RBUS ld_apl_read_max_3_reg;
	blu_ld_apl_read_max_4_RBUS ld_apl_read_max_4_reg;
    unsigned char LD_type, LD_Hnum, LD_Vnum;
    unsigned char valid;
    unsigned short access_ptr;
    unsigned short mdata;
    unsigned short ldata;
    unsigned short timeout;

    ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
    LD_type = ld_global_ctrl2_reg.ld_blk_type;
    LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum;
    LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum;

    if( LD_type == 0 ) // direct
    {
        hnum = LD_Hnum+1;
        vnum = LD_Vnum+1;
    }
    else if( LD_type == 1 ) // edge top/down
    {
        hnum = LD_Hnum+1;
        vnum = (LD_Vnum+1)*4;
    }
    else // edge left/right
    {
        hnum = (LD_Hnum+1)*4;
        vnum = LD_Vnum+1;
    }

    ld_apl_read_max_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_1_reg);
    ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);

    ld_apl_read_max_2_reg.ld_aplmaxaccess_even_line = 1;
	ld_apl_read_max_2_reg.ld_aplmaxaccess_delta = 0;
	ld_apl_read_max_2_reg.ld_aplmaxaccess_offset = 0;
    ld_apl_read_max_2_reg.ld_aplmax_access_en = 1;

    ld_apl_read_max_1_reg.ld_aplmax_conti = 1;
    ld_apl_read_max_1_reg.ld_aplmax_start = 1;

    IoReg_Write32(BLU_LD_APL_read_max_2_reg, ld_apl_read_max_2_reg.regValue);
    IoReg_Write32(BLU_LD_APL_read_max_1_reg, ld_apl_read_max_1_reg.regValue);

    for( i=0; i<vnum; i++ )
    {
        for( j=0; j<((hnum+1)>>1); j++ )
        {
            timeout = 20;
            do {
                ld_apl_read_max_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_3_reg);
                valid = (ld_apl_read_max_3_reg.regValue & 0x80000000) >> 31;
                timeout--;
                if( timeout == 0 )
                    return 0;
            } while( !valid );
			ld_apl_read_max_4_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_4_reg);
            access_ptr = (ld_apl_read_max_4_reg.regValue & 0xFFF);
            mdata = (ld_apl_read_max_3_reg.regValue & 0x000FFC00) >> 10;
            ldata = (ld_apl_read_max_3_reg.regValue & 0x000003FF);

            if( i*((hnum+1)>>1)+j != access_ptr ) // access ptr wrong
                return 0;

            block_addr = i*hnum + j*2;
            APL_data[block_addr] = ldata;
            if( 2*j+1 < hnum )
                APL_data[block_addr+1] = mdata;
        }
    }

    ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);
    ld_apl_read_max_2_reg.ld_aplmax_access_en = 0;
    IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_max_2_reg.regValue);

	return 1;
}

unsigned char drvif_color_get_LD_APL_max_avg()
{
    int i,j;
    unsigned short hnum, vnum;
    unsigned short block_addr;
	blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
	
    blu_ld_apl_read_max_1_RBUS ld_apl_read_max_1_reg;
    blu_ld_apl_read_max_2_RBUS ld_apl_read_max_2_reg;
    blu_ld_apl_read_max_3_RBUS ld_apl_read_max_3_reg;
	blu_ld_apl_read_max_4_RBUS ld_apl_read_max_4_reg;

	blu_ld_apl_read_ave_1_RBUS ld_apl_read_ave_1_reg;
    blu_ld_apl_read_ave_2_RBUS ld_apl_read_ave_2_reg;
    blu_ld_apl_read_ave_3_RBUS ld_apl_read_ave_3_reg;
	blu_ld_apl_read_ave_4_RBUS ld_apl_read_ave_4_reg;
	DRV_LD_Backlight_Decision bl_dec;
	
    unsigned char LD_type, LD_Hnum, LD_Vnum;
 //   unsigned char valid_max, valid_avg;
    unsigned short access_ptr_max, access_ptr_avg;
    unsigned short mdata_max, mdata_avg;
    unsigned short ldata_max, ldata_avg;
 //   unsigned short timeout;
	unsigned short bpl_value;
    unsigned short test1, test2, test3, test4 = 0; 
	static unsigned int LD_count1;
//	unsigned short tmp_bl_ave;
//	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];
//	extern unsigned char vpq_led_LD_lutTableIndex;

//	unsigned int t0, t_diff;

	drvif_color_get_LD_Backlight_Decision(&bl_dec);
    ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
    LD_type = ld_global_ctrl2_reg.ld_blk_type;
    LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum;
    LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum;

    if( LD_type == 0 ) // direct
    {
        hnum = LD_Hnum+1;
        vnum = LD_Vnum+1;
    }
    else if( LD_type == 1 ) // edge top/down
    {
        hnum = LD_Hnum+1;
        vnum = (LD_Vnum+1)*4;
    }
    else // edge left/right
    {
        hnum = (LD_Hnum+1)*4;
        vnum = LD_Vnum+1;
    }
	// calc. max
    ld_apl_read_max_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_1_reg);
    ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);

    //ld_apl_read_max_2_reg.ld_aplmax_addr = 0;
    ld_apl_read_max_2_reg.ld_aplmaxaccess_even_line = 1;
	ld_apl_read_max_2_reg.ld_aplmaxaccess_delta = 0;
	ld_apl_read_max_2_reg.ld_aplmaxaccess_offset = 0;
    ld_apl_read_max_2_reg.ld_aplmax_access_en = 1;

    ld_apl_read_max_1_reg.ld_aplmax_conti = 1;
    ld_apl_read_max_1_reg.ld_aplmax_start = 1;

    IoReg_Write32(BLU_LD_APL_read_max_2_reg, ld_apl_read_max_2_reg.regValue);
    IoReg_Write32(BLU_LD_APL_read_max_1_reg, ld_apl_read_max_1_reg.regValue);

	// calc. avg
    ld_apl_read_ave_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_1_reg);
    ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);

    //ld_apl_read_ave_2_reg.ld_aplave_addr = 0;
	ld_apl_read_ave_2_reg.ld_aplaveaccess_even_line = 1;
	ld_apl_read_ave_2_reg.ld_aplaveaccess_delta = 0;
	ld_apl_read_ave_2_reg.ld_aplaveaccess_offset = 0;
    ld_apl_read_ave_2_reg.ld_aplave_access_en = 1;

    ld_apl_read_ave_1_reg.ld_aplave_conti = 1;
    ld_apl_read_ave_1_reg.ld_aplave_start = 1;

    IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);
    IoReg_Write32(BLU_LD_APL_read_ave_1_reg, ld_apl_read_ave_1_reg.regValue);


    for( i=0; i<vnum; i++ )
    {
        for( j=0; j<((hnum+1)>>1); j++ )
        {
        	
			ld_apl_read_max_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_3_reg);
			ld_apl_read_max_4_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_4_reg);
	        ld_apl_read_ave_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_3_reg);
			ld_apl_read_ave_4_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_4_reg);
	                        
            access_ptr_max = (ld_apl_read_max_4_reg.regValue & 0xFFF);
            mdata_max = (ld_apl_read_max_3_reg.regValue & 0x000FFC00) >> 10;
            ldata_max = (ld_apl_read_max_3_reg.regValue & 0x000003FF);

			access_ptr_avg = (ld_apl_read_ave_4_reg.regValue & 0xFFF);
            mdata_avg = (ld_apl_read_ave_3_reg.regValue & 0x000FFC00) >> 10;
            ldata_avg = (ld_apl_read_ave_3_reg.regValue & 0x000003FF);

            block_addr = i*hnum + j*2;
            max_avg_table.max_table[block_addr] = ldata_max;
			max_avg_table.avg_table[block_addr] = ldata_avg;
            if( 2*j+1 < hnum ) {
                max_avg_table.max_table[block_addr+1] = mdata_max;
				max_avg_table.avg_table[block_addr+1] = mdata_avg;
            }
        }
    }

//	t0 = drvif_Get_90k_Lo_clk();
	max_avg_table.Global_APL = 0;
	max_avg_table.Leakage_Level = 0;
	test1 = 0;
	test2 = 0;
	test3 = 0;
	test4 = 0;

	for( i = 0; i < vnum; i++ ) {
		for( j = 0; j < hnum; j++ ) {
			bpl_value = (max_avg_table.avg_table[i * hnum + j] * bl_dec.ld_avegain / 32)
						+ (max_avg_table.max_table[i * hnum + j] * bl_dec.ld_maxgain / 32);
			//fwif_color_LD_SW_1DLUT(&bpl_value, &bpl_value, 1, Local_Dimming_Table[vpq_led_LD_lutTableIndex].LD_Spatial_Remap.ld_spatialremaptab, INPUT_DATA_10BITS, LUT_65POINTS);
			if(bpl_value > 1023)
				bpl_value = 1023;
			max_avg_table.WholeBPL = max_avg_table.WholeBPL + bpl_value;
			max_avg_table.Global_APL = max_avg_table.Global_APL + max_avg_table.avg_table[i * hnum + j];
			if (max_avg_table.avg_table[i * hnum + j]<10 && (max_avg_table.max_table[i * hnum + j]-max_avg_table.avg_table[i * hnum + j])>150)
				max_avg_table.Leakage_Level ++;
			if (max_avg_table.avg_table[i * hnum + j]>10)
				test1++;
			if ((max_avg_table.max_table[i * hnum + j]-max_avg_table.avg_table[i * hnum + j])>150)
				test2++;
			if ((max_avg_table.max_table[i * hnum + j]-max_avg_table.avg_table[i * hnum + j])>200)
				test3++;
			if ((max_avg_table.max_table[i * hnum + j]-max_avg_table.avg_table[i * hnum + j])>300)
				test4++;
				
		}
	}
	
//	t_diff = drvif_Get_90k_Lo_clk() - t0;
	max_avg_table.WholeBPL = (hnum * vnum == 0)? 0 : max_avg_table.WholeBPL/(hnum * vnum);
	max_avg_table.Global_APL = max_avg_table.Global_APL/(hnum*vnum);
	max_avg_table.Leakage_Level = 1023*max_avg_table.Leakage_Level/(hnum * vnum);

//	tmp_bl_ave = max_avg_table.WholeBPL;
	#if 0
	if(LD_count1 % 7200 == 0)
		rtd_printk(KERN_EMERG, "VPQLED", "t0 =%d, t1 =%d, t2 =%d, t3=%d\n",test1,test2,test3,test4);
	#endif
//		rtd_printk(KERN_INFO, TAG_NAME, "vpq_led_LD_lutTableIndex =%d\n", vpq_led_LD_lutTableIndex);
	//fwif_color_LD_SW_1DLUT(&tmp_bl_ave, &tmp_bl_ave, 1, Local_Dimming_Table[3].LD_Spatial_Remap.ld_spatialremaptab, INPUT_DATA_10BITS, LUT_65POINTS);
//	max_avg_table.WholeBPL = tmp_bl_ave;
	LD_count1++;

    ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);
    ld_apl_read_max_2_reg.ld_aplmax_access_en = 0;
    IoReg_Write32(BLU_LD_APL_read_max_2_reg, ld_apl_read_max_2_reg.regValue);

	ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);
    ld_apl_read_ave_2_reg.ld_aplave_access_en = 0;
    IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);

	return 1;
}


// Merlin6 add, only read 8bin histogram, jimmy 20200406
unsigned char drvif_color_get_LD_8bin( unsigned short blk_start, unsigned short read_num, unsigned int** blk_hist )
{

	int i,j;
	unsigned short hnum, vnum;
	int total_hist_num, read_hist_num;
	unsigned short block_addr;
	blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
	blu_ld_apl_read_8bin_1_RBUS ld_apl_read_8bin_1_reg;
	blu_ld_apl_read_8bin_2_RBUS ld_apl_read_8bin_2_reg;
	blu_ld_apl_read_8bin_3_RBUS ld_apl_read_8bin_3_reg;
	blu_ld_apl_read_8bin_4_RBUS ld_apl_read_8bin_4_reg;
	unsigned char LD_type, LD_Hnum, LD_Vnum;
	unsigned char valid;
	unsigned short access_ptr;
	unsigned int bin_data;
	unsigned short timeout;

	ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	LD_type = ld_global_ctrl2_reg.ld_blk_type;
	LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum;
	LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum;

	if( LD_type == 0 ) // direct
	{
		hnum = LD_Hnum+1;
		vnum = LD_Vnum+1;
	}
	else if( LD_type == 1 ) // edge top/down
	{
		hnum = LD_Hnum+1;
		vnum = (LD_Vnum+1)*4;
	}
	else // edge left/right
	{
		hnum = (LD_Hnum+1)*4;
		vnum = LD_Vnum+1;
	}

	// max 128 blocks
	total_hist_num = hnum * vnum;
	read_hist_num = (read_num > 128) ? 128 : read_num;
	if( blk_start + read_num > total_hist_num )
		read_hist_num = total_hist_num - blk_start;

	ld_apl_read_8bin_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_8bin_1_reg);
	ld_apl_read_8bin_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_8bin_2_reg);

	//ld_apl_read_8bin_2_reg.ld_apl8bin_addr = 0;
	ld_apl_read_8bin_2_reg.ld_apl8binaccess_even_line = 1;
	ld_apl_read_8bin_2_reg.ld_apl8binaccess_delta = 0;
	ld_apl_read_8bin_2_reg.ld_apl8binaccess_offset = 0;
	ld_apl_read_8bin_2_reg.ld_apl8bin_access_en = 1;

	ld_apl_read_8bin_1_reg.ld_apl8bin_conti = 1;
	ld_apl_read_8bin_1_reg.ld_apl8bin_begin_bl = blk_start;
	ld_apl_read_8bin_1_reg.ld_apl8bin_start = 1;

	IoReg_Write32(BLU_LD_APL_read_8bin_2_reg, ld_apl_read_8bin_2_reg.regValue);
	IoReg_Write32(BLU_LD_APL_read_8bin_1_reg, ld_apl_read_8bin_1_reg.regValue);

	for( i=0; i<read_hist_num; i++ )
	{
		for( j=0; j<8; j++ )
		{
			timeout = 20;
			do {
				ld_apl_read_8bin_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_8bin_3_reg);
				valid = (ld_apl_read_8bin_3_reg.regValue & _BIT31) >> 31;
				timeout--;
				if( timeout == 0 )
					return 0;
			} while( !valid );
		}

		//access_ptr = (ld_apl_read_8bin_3_reg.regValue & 0x3FF00000) >> 20; // bit 29:20
		access_ptr = (ld_apl_read_8bin_4_reg.regValue & 0x1FFF); // bit 12:00
		bin_data = (ld_apl_read_8bin_3_reg.regValue & 0x000FFFFF); // bit 19:0

		if( i*8+j != access_ptr ) // access ptr wrong
			return 0;

		block_addr = blk_start + i;
		blk_hist[block_addr][j] = bin_data;
	}

	ld_apl_read_8bin_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_8bin_2_reg);
	ld_apl_read_8bin_2_reg.ld_apl8bin_access_en = 0;
	IoReg_Write32(BLU_LD_APL_read_8bin_2_reg, ld_apl_read_8bin_2_reg.regValue);

	return 1;
}


// Merlin6 add, read 8bin histogram + max/ave, jimmy 20200406
unsigned char drvif_color_get_LD_8bin_max_ave( unsigned short blk_start, unsigned short read_num, unsigned int** blk_hist, unsigned int* blk_max, unsigned int* blk_ave )
{

	int i,j;
	unsigned short hnum, vnum;
	int total_hist_num, read_hist_num;
	unsigned short block_addr;
	blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
	blu_ld_apl_read_8bin_1_RBUS ld_apl_read_8bin_1_reg;
	blu_ld_apl_read_8bin_2_RBUS ld_apl_read_8bin_2_reg;
	blu_ld_apl_read_8bin_3_RBUS ld_apl_read_8bin_3_reg;
	blu_ld_apl_read_8bin_4_RBUS ld_apl_read_8bin_4_reg;
	unsigned char LD_type, LD_Hnum, LD_Vnum;
	unsigned char valid;
	unsigned short access_ptr;
	unsigned int bin_data, ave_data, max_data;
	unsigned short timeout;

	ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	LD_type = ld_global_ctrl2_reg.ld_blk_type;
	LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum;
	LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum;

	if( LD_type == 0 ) // direct
	{
		hnum = LD_Hnum+1;
		vnum = LD_Vnum+1;
	}
	else if( LD_type == 1 ) // edge top/down
	{
		hnum = LD_Hnum+1;
		vnum = (LD_Vnum+1)*4;
	}
	else // edge left/right
	{
		hnum = (LD_Hnum+1)*4;
		vnum = LD_Vnum+1;
	}

	// max 112 blocks
	total_hist_num = hnum * vnum;
	read_hist_num = (read_num > 112) ? 112 : read_num;
	if( blk_start + read_num > total_hist_num )
		read_hist_num = total_hist_num - blk_start;


	// 1st page: max 112 blocks
	ld_apl_read_8bin_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_8bin_1_reg);
	ld_apl_read_8bin_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_8bin_2_reg);

	//ld_apl_read_8bin_2_reg.ld_apl8bin_addr = 0;
	ld_apl_read_8bin_2_reg.ld_apl8binaccess_even_line = 1;
	ld_apl_read_8bin_2_reg.ld_apl8binaccess_delta = 0;
	ld_apl_read_8bin_2_reg.ld_apl8binaccess_even_line = 0;
	ld_apl_read_8bin_2_reg.ld_apl8bin_access_en = 1;

	ld_apl_read_8bin_1_reg.ld_apl8bin_conti = 1;
	ld_apl_read_8bin_1_reg.ld_apl8bin_begin_bl = blk_start;
	ld_apl_read_8bin_1_reg.ld_apl8bin_start = 1;

	IoReg_Write32(BLU_LD_APL_read_8bin_2_reg, ld_apl_read_8bin_2_reg.regValue);
	IoReg_Write32(BLU_LD_APL_read_8bin_1_reg, ld_apl_read_8bin_1_reg.regValue);

	for( i=0; i<read_hist_num; i++ )
	{
		for( j=0; j<8; j++ )
		{
			timeout = 20;
			do {
				ld_apl_read_8bin_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_8bin_3_reg);
				valid = (ld_apl_read_8bin_3_reg.regValue & _BIT31) >> 31;
				timeout--;
				if( timeout == 0 )
					return 0;
			} while( !valid );
		}

		access_ptr = (ld_apl_read_8bin_4_reg.regValue & 0x1FFF); // bit 12:00
		bin_data = (ld_apl_read_8bin_3_reg.regValue & 0x000FFFFF); // bit 19:0

		if( i*8+j != access_ptr ) // access ptr wrong
			return 0;

		block_addr = blk_start+i;
		blk_hist[block_addr][j] = bin_data;
	}

	ld_apl_read_8bin_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_8bin_2_reg);
	ld_apl_read_8bin_2_reg.ld_apl8bin_access_en = 0;
	IoReg_Write32(BLU_LD_APL_read_8bin_2_reg, ld_apl_read_8bin_2_reg.regValue);

	// read max/ave from addr 896
	//ld_apl_read_8bin_2_reg.ld_apl8bin_addr = 896;
	ld_apl_read_8bin_2_reg.ld_apl8binaccess_even_line = 1;
	ld_apl_read_8bin_2_reg.ld_apl8binaccess_delta = 0;
	ld_apl_read_8bin_2_reg.ld_apl8binaccess_even_line = 0;

	ld_apl_read_8bin_2_reg.ld_apl8bin_access_en = 1;

	ld_apl_read_8bin_1_reg.ld_apl8bin_conti = 1;
	ld_apl_read_8bin_1_reg.ld_apl8bin_begin_bl = 896;
	ld_apl_read_8bin_1_reg.ld_apl8bin_start = 1;

	IoReg_Write32(BLU_LD_APL_read_8bin_2_reg, ld_apl_read_8bin_2_reg.regValue);
	IoReg_Write32(BLU_LD_APL_read_8bin_1_reg, ld_apl_read_8bin_1_reg.regValue);

	for( i=0; i<read_hist_num; i++ )
	{
		timeout = 20;
		do {
			ld_apl_read_8bin_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_8bin_3_reg);
			valid = (ld_apl_read_8bin_3_reg.regValue & _BIT31) >> 31;
			timeout--;
			if( timeout == 0 )
				return 0;
		} while( !valid );

		access_ptr = (ld_apl_read_8bin_4_reg.regValue & 0x1FFF); // bit 12:00
		ave_data = (ld_apl_read_8bin_3_reg.regValue & 0x000FFC00); // bit 19:10
		max_data = (ld_apl_read_8bin_3_reg.regValue & 0x000003FF); // bit 9:0

		if( 896+i != access_ptr ) // access ptr wrong
			return 0;

		block_addr = blk_start+i;
		blk_ave[block_addr] = ave_data;
		blk_max[block_addr] = max_data;
	}


	return 1;

}


void drvif_HAL_VPQ_LED_LDEnable(unsigned char bCtrl)
{
	RBus_UInt32 staAddr = 0;
	RBus_UInt32 blvle = 0xff;
	int i;


	blu_ld_backlight_ctrl1_RBUS blu_ld_backlight_ctrl1;
	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	blu_ld_global_ctrl1_RBUS blu_ld_global_ctrl1;

	blu_ld_backlight_ctrl1.regValue = IoReg_Read32(BLU_LD_Backlight_Ctrl1_reg);
	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	blu_ld_global_ctrl1.regValue = IoReg_Read32(BLU_LD_Global_Ctrl1_reg);

	blu_ld_backlight_ctrl1.ld_blu_spien = bCtrl;

	fwif_color_WaitFor_DEN_STOP_MEMCDTG();
	IoReg_Write32(BLU_LD_Backlight_Ctrl1_reg, blu_ld_backlight_ctrl1.regValue);

	/*double buffer apply*/
	blu_ld_global_ctrl1.ld_db_apply = 1;
	IoReg_Write32(BLU_LD_Global_Ctrl1_reg, blu_ld_global_ctrl1.regValue);

	if (bCtrl == 0) {	/*reset LD interface sram*/

		IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, staAddr);
		for (i = 0; i < 64; i++) {		/*in Mac3 data sram size is 64 + header size = 128, clear data part.*/

			IoReg_Write32(LDSPI_Data_Rwport_reg, blvle);
		}

	}

	/*marked since 4x8 compensation is not ready*/
	//drvif_HAL_VPQ_LED_LDEnablePixelCompensation(bCtrl);


}

void drvif_HAL_VPQ_LED_LDEnablePixelCompensation(unsigned char bCtrl)
{
	//unsigned int buffer_status_count = 0;
	//unsigned int buffer_status_level;

	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
	blu_ld_data_comp_ctrl_RBUS blu_ld_data_comp_ctrl;

	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
	blu_ld_data_comp_ctrl.regValue = IoReg_Read32(BLU_LD_DATA_COMP_CTRL_reg);

	if (blu_ld_global_ctrl0.ld_comp_en == 1)
		blu_ld_data_comp_ctrl.ld_data_comp_mode = bCtrl;
	else
		blu_ld_data_comp_ctrl.ld_data_comp_mode = 0;

	IoReg_Write32(BLU_LD_DATA_COMP_CTRL_reg, blu_ld_data_comp_ctrl.regValue);
}

void drvif_HAL_VPQ_LED_LDCtrlDemoMode(DRV_HAL_VPQ_LED_LDCtrlDemoMode *ptr)
{
	blu_ld_demo_window_ctrl0_RBUS blu_ld_demo_window_ctrl0;
	blu_ld_demo_window_ctrl1_RBUS blu_ld_demo_window_ctrl1;
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;

	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	#ifdef _VIP_Mer7_Compile_Error__
	unsigned char ucld_blk_hnum, ucld_blk_vnum = 0;
	#endif
	bool bCtrl = ptr->bCtrl;/*LDCtrlDemoMode.bCtrl;*/
	UINT8 bType = ptr->ctrlMode;/*LDCtrlDemoMode.ctrlMode;*/

	blu_ld_demo_window_ctrl0.regValue = IoReg_Read32(BLU_LD_Demo_window_ctrl0_reg);
	blu_ld_demo_window_ctrl1.regValue = IoReg_Read32(BLU_LD_Demo_window_ctrl1_reg);
	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);

	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	
	#ifdef _VIP_Mer7_Compile_Error
	ucld_blk_hnum = blu_ld_global_ctrl2.ld_blk_hnum;
	ucld_blk_vnum = blu_ld_global_ctrl2.ld_blk_vnum;
	#endif
	
	if (bCtrl) {
		if (bType == LED_ONOFF) {	/*Each LED should blink sequentially*/
			/*autoMA do that?*/
			/*Use blu_ld_global_ctrl0.dummy1802c200_30_0 to save enable bit & number of backlight*/
			/*bit 0 -> enable bit*/
			/*bit 1~9 -> number of backlight*/
			/*bit 10~15 -> timer*/
#ifdef _VIP_Mer7_Compile_Error__
			unsigned int numOfbacklight = ucld_blk_hnum+1; /*max - need to be fixed by LD spec*/
			unsigned int blinkTimer = 60;
			blu_ld_global_ctrl0.dummy1802c200_30_0 = (blinkTimer<<10) + (numOfbacklight << 1) + 1;
#endif
		} else if (bType == SPLIT_SCREEN) {		/*Change half of screen as full white and show local dimming effect.*/
			blu_ld_demo_window_ctrl0.ld_demo_en = bCtrl;
			blu_ld_demo_window_ctrl0.ld_demo_mode = 0;
			blu_ld_demo_window_ctrl0.ld_demo_top = 0;
			blu_ld_demo_window_ctrl0.ld_demo_bottom = ptr->height;
			blu_ld_demo_window_ctrl1.ld_demo_left = 0;
			blu_ld_demo_window_ctrl1.ld_demo_right = ptr->width;
#ifdef _VIP_Mer7_Compile_Error__
			blu_ld_global_ctrl0.dummy1802c200_30_0 = 0;
#endif
		}
	} else {
		blu_ld_demo_window_ctrl0.ld_demo_en = bCtrl;
#ifdef _VIP_Mer7_Compile_Error__
		blu_ld_global_ctrl0.dummy1802c200_30_0 = 0;
#endif
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
	unsigned short max =0, min=255;//, dummy;//, tmp=0;

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
	//dummy = IoReg_Read32(LDSPI_Data_Rwport_reg);


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

	if (Wait_Tx_Done == 1) {/*Normal Usage Wait tx_done*/
	
		do {
			ldspi_wclr_ctrl.regValue = IoReg_Read32(LDSPI_WCLR_CTRL_reg);
			tx_done = ldspi_wclr_ctrl.tx_done;
			if(tx_done == 1) {
				ldspi_data_addr_ctrl.regValue = IoReg_Read32(LDSPI_Data_Addr_CTRL_reg);
				ldspi_data_addr_ctrl.data_rw_pointer_set = LDSPI_DataSRAM->SRAM_Position;
				IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
				for (i=0;i<LDSPI_DataSRAM->SRAM_Length;i++) {
					ldspi_data_rwport.data_rw_data_port = *(LDSPI_DataSRAM->SRAM_Value+i);
					IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
				}
				return;
			}
		} while(--timeoutcnt != 0);
		
		if(timeoutcnt == 0){
			rtd_pr_vpq_emerg("drvif_color_set_LDSPI_DataSRAM_Data_Continuous, timeoutcnt == 0\n");
		}
	} else {/*tx_done*/
		ldspi_data_addr_ctrl.regValue = IoReg_Read32(LDSPI_Data_Addr_CTRL_reg);
		ldspi_data_addr_ctrl.data_rw_pointer_set = LDSPI_DataSRAM->SRAM_Position;
		IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Select SRAM Position*/
		for (i=0;i<LDSPI_DataSRAM->SRAM_Length;i++) {
			ldspi_data_rwport.data_rw_data_port = *(LDSPI_DataSRAM->SRAM_Value+i);
			IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*SRAM Data*/
		}

	}
}

char drvif_color_Set_Local_Dimming_Object_Gain_DMA_Enable(unsigned char Enable, unsigned int phy_Addr, unsigned short* vir_Addr, unsigned short H_Num, unsigned short V_Num, unsigned char line_mode, unsigned char bit_gain)
{	
	blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
	blu_ld_rd_ctrl_RBUS blu_ld_rd_ctrl_reg;
	blu_ld_rd_num_bl_wrap_line_step_RBUS blu_ld_rd_num_bl_wrap_line_step_reg;
	blu_ld_rd_num_bl_wrap_word_RBUS blu_ld_rd_num_bl_wrap_word_reg;
	blu_ld_rd_num_bl_wrap_ctl_RBUS blu_ld_rd_num_bl_wrap_ctl_reg;
	blu_ld_ai_gain_RBUS blu_ld_ai_gain_reg;
	unsigned short line_128_num;
	unsigned short line_num;/*Merlin8 add*/
	unsigned char gain_8bit;/*Merlin8 add*/
	
	blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
	blu_ld_global_ctrl0.ld_ai_gain_en = 0;
	IoReg_Write32(BLU_LD_Global_Ctrl0_reg, blu_ld_global_ctrl0.regValue);

	blu_ld_rd_ctrl_reg.regValue = IoReg_Read32(BLU_LD_RD_Ctrl_reg);
	blu_ld_rd_ctrl_reg.ld_dma_en = Enable;
	blu_ld_rd_ctrl_reg.ld_128_msb_start = 1; // ?? or = 1
	IoReg_Write32(BLU_LD_RD_Ctrl_reg, blu_ld_rd_ctrl_reg.regValue);

	blu_ld_ai_gain_reg.regValue = IoReg_Read32(BLU_LD_AI_Gain_reg);/*Merlin8 add*/
	blu_ld_ai_gain_reg.ld_ai_gain_8bit = bit_gain;
	IoReg_Write32(BLU_LD_AI_Gain_reg, blu_ld_ai_gain_reg.regValue);

	IoReg_SetBits(BLU_LD_Global_Ctrl1_reg, BLU_LD_Global_Ctrl1_ld_db_apply_mask);
	
	if (Enable == 0)
		return 1;

	if (phy_Addr == 0 || vir_Addr == NULL) {
		rtd_printk(KERN_INFO, TAG_NAME, "drvif_color_Set_Local_Dimming_Object_Gain_DMA_Enable NUL\n");
		return -1;
	}

	// set address
	IoReg_Write32(BLU_LD_RD_num_bl_wrap_addr_0_reg, phy_Addr);
	gain_8bit = bit_gain;
	/*Merlin8 add*/
	if(line_mode == 0 && gain_8bit ==0){
		line_128_num =  ((H_Num+1)*(V_Num+1)*16 + 64 )>>7;
		line_num = 1;
	}
	else if(line_mode == 0 && gain_8bit ==1){
		line_128_num =  ((H_Num+1)*(V_Num+1)*8 + 64 )>>7;
		line_num = 1;
	}
	else if(line_mode == 1 && gain_8bit ==0){
		line_128_num =  ((H_Num+1)*16 + 64 )>>7;
		line_num = V_Num;
	}
	else{
		line_128_num =  ((H_Num+1)*8 + 64 )>>7;
		line_num = V_Num;
	}

	blu_ld_rd_num_bl_wrap_word_reg.regValue = IoReg_Read32(BLU_LD_RD_num_bl_wrap_word_reg);
	blu_ld_rd_num_bl_wrap_word_reg.ld_line_128_num = line_128_num;
	IoReg_Write32(BLU_LD_RD_num_bl_wrap_word_reg, blu_ld_rd_num_bl_wrap_word_reg.regValue);

	blu_ld_rd_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(BLU_LD_RD_num_bl_wrap_line_step_reg);
	blu_ld_rd_num_bl_wrap_line_step_reg.ld_line_step = line_128_num;
	IoReg_Write32(BLU_LD_RD_num_bl_wrap_line_step_reg, blu_ld_rd_num_bl_wrap_line_step_reg.regValue);

	blu_ld_rd_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(BLU_LD_RD_num_bl_wrap_ctl_reg);
	blu_ld_rd_num_bl_wrap_ctl_reg.ld_burst_len = line_128_num;
	blu_ld_rd_num_bl_wrap_ctl_reg.ld_line_num = line_num;	// + 1;
	IoReg_Write32(BLU_LD_RD_num_bl_wrap_ctl_reg, blu_ld_rd_num_bl_wrap_ctl_reg.regValue);

	IoReg_SetBits(BLU_LD_Global_Ctrl1_reg, BLU_LD_Global_Ctrl1_ld_db_apply_mask);

	return 0;
}

char drvif_color_Set_Local_Dimming_Object_Gain_Addr_Boundary(unsigned int addr_up_limit, unsigned int addr_low_limit)
{
	blu_ld_rd_rule_check_up_RBUS blu_ld_rd_rule_check_up_reg;
	blu_ld_rd_rule_check_low_RBUS blu_ld_rd_rule_check_low_reg;

	blu_ld_rd_rule_check_up_reg.regValue = IoReg_Read32(BLU_LD_RD_Rule_check_up_reg);
	blu_ld_rd_rule_check_low_reg.regValue = IoReg_Read32(BLU_LD_RD_Rule_check_low_reg);

	blu_ld_rd_rule_check_up_reg.ld_up_limit = addr_up_limit>>4;
	blu_ld_rd_rule_check_low_reg.ld_low_limit = addr_low_limit>>4;
	
	IoReg_Write32(BLU_LD_RD_Rule_check_up_reg, blu_ld_rd_rule_check_up_reg.regValue);
	IoReg_Write32(BLU_LD_RD_Rule_check_low_reg, blu_ld_rd_rule_check_low_reg.regValue);
	return 0;
}

void drvif_color_set_LDSPI_TxDoneIntEn(unsigned char enable)
{
	ldspi_rx_data_ctrl_RBUS ldspi_rx_data_ctrl;
	ldspi_rx_data_ctrl.regValue = IoReg_Read32(LDSPI_RX_data_CTRL_reg);

	ldspi_rx_data_ctrl.tx_done_int_en = enable;
	IoReg_Write32(LDSPI_RX_data_CTRL_reg, ldspi_rx_data_ctrl.regValue);
	rtd_pr_vpq_info("LDSPI_TxDoneIntEn = %d\n",enable);

}

void drvif_color_get_LD_Algo_sram(LED_AlgoSRAM* ptr)
{

	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	blu_ld_blight_ap_RBUS reg_blu_ld_blight_ap;
	blu_ld_blight_dp_RBUS reg_blu_ld_blight_dp;
	blu_ld_sram_ctrl_RBUS reg_blu_ld_sram_ctrl;

	unsigned short ld_blk_type, ld_blk_hnum, ld_blk_vnum;
	short i, j, count, temp;
	unsigned long sum = 0;

	/* get global set */
	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	ld_blk_type = blu_ld_global_ctrl2.ld_blk_type;
	ld_blk_hnum = blu_ld_global_ctrl2.ld_blk_hnum;
	ld_blk_vnum = blu_ld_global_ctrl2.ld_blk_vnum;

/*	if(((ld_blk_hnum + 1) * (ld_blk_vnum + 1)) > 120) {
		//rtd_printk(KERN_EMERG, TAG_NAME," [Warning] drvif_color_get_LD_Algo_sram is not correct, ld_blk_vnum or ld_blk_hnum out of bound.\n");
		return;
	}*/

	//rtd_printk(KERN_EMERG, TAG_NAME, "ld_blk_type = %d, ld_blk_hnum = %d, ld_blk_vnum = %d\n", ld_blk_type, ld_blk_hnum, ld_blk_vnum);
	//enable; setup sw mode
	reg_blu_ld_sram_ctrl.regValue = IoReg_Read32(BLU_LD_SRAM_CTRL_reg);
	reg_blu_ld_sram_ctrl.ld_blight_sw_mode = 1;
	IoReg_Write32(BLU_LD_SRAM_CTRL_reg,  reg_blu_ld_sram_ctrl.regValue);

	if(ld_blk_type == 1) {	// top/bottom mode
		ld_blk_vnum = 2;
		for(j = 0; j < ld_blk_hnum + 1; j++) {
			// addr
			reg_blu_ld_blight_ap.regValue = IoReg_Read32(BLU_LD_BLIGHT_AP_reg);
			//reg_blu_ld_blight_ap.ld_blight_adr = j * ((ld_blk_vnum + 1) >> 1);//
			if(j<((ld_blk_hnum + 1)>>1))
			{
				reg_blu_ld_blight_ap.ld_blight_even_line = 1;
				reg_blu_ld_blight_ap.ld_blight_delta = j;
				reg_blu_ld_blight_ap.ld_blight_offset = 0;
			}
			else
			{
				reg_blu_ld_blight_ap.ld_blight_even_line = 0;
				reg_blu_ld_blight_ap.ld_blight_delta = j;
				reg_blu_ld_blight_ap.ld_blight_offset = (ld_blk_hnum + 1)>>1;
			}
			
			IoReg_Write32(BLU_LD_BLIGHT_AP_reg, reg_blu_ld_blight_ap.regValue);
			for(i = 0; i < ld_blk_vnum;) {
				// data
				reg_blu_ld_blight_dp.regValue = IoReg_Read32(BLU_LD_BLIGHT_DP_reg);
				ptr->AlgoSRAM[i++][j] = reg_blu_ld_blight_dp.ld_blight_dat_lsb;
				ptr->AlgoSRAM[i++][j] = reg_blu_ld_blight_dp.ld_blight_dat_msb;
			}
		}
	}
	else if(ld_blk_type == 2) {	// left/right mode
		ld_blk_hnum = 2;
		i = 0;
		while(i < (ld_blk_vnum * ld_blk_hnum) + 1) {
			// addr
			reg_blu_ld_blight_ap.regValue = IoReg_Read32(BLU_LD_BLIGHT_AP_reg);
			//reg_blu_ld_blight_ap.ld_blight_adr = i >> 1;//

			temp = i/2;
			if(temp/2 == 0)
				count = 1;
			else
				count = 0;
			reg_blu_ld_blight_ap.ld_blight_even_line = count;
			reg_blu_ld_blight_ap.ld_blight_delta = 0;
			reg_blu_ld_blight_ap.ld_blight_offset = temp*2;


			IoReg_Write32(BLU_LD_BLIGHT_AP_reg, reg_blu_ld_blight_ap.regValue);
			// data
			reg_blu_ld_blight_dp.regValue = IoReg_Read32(BLU_LD_BLIGHT_DP_reg);
			AlgoSRAMTmp[i++] = reg_blu_ld_blight_dp.ld_blight_dat_lsb;
			AlgoSRAMTmp[i++] = reg_blu_ld_blight_dp.ld_blight_dat_msb;
		}
		for(i = 0; i < ld_blk_vnum + 1; i++) {
			for(j = 0; j < ld_blk_hnum; j++) {
				ptr->AlgoSRAM[i][j] =  AlgoSRAMTmp[i * ld_blk_hnum + j];
			}
		}
	}
	else {	// direct mode
		// addr
		reg_blu_ld_blight_ap.regValue = IoReg_Read32(BLU_LD_BLIGHT_AP_reg);
		//reg_blu_ld_blight_ap.ld_blight_adr = 0x00;//
		reg_blu_ld_blight_ap.ld_blight_even_line = 1;
		reg_blu_ld_blight_ap.ld_blight_delta = 0;
		reg_blu_ld_blight_ap.ld_blight_offset = 0;
		IoReg_Write32(BLU_LD_BLIGHT_AP_reg, reg_blu_ld_blight_ap.regValue);
		for(i = 0; i < ld_blk_hnum + 1; i++) {
			for(j = 0; j < ld_blk_vnum + 1;) {
				// data
				reg_blu_ld_blight_dp.regValue = IoReg_Read32(BLU_LD_BLIGHT_DP_reg);
				ptr->AlgoSRAM[i][j++] = reg_blu_ld_blight_dp.ld_blight_dat_lsb;
				ptr->AlgoSRAM[i][j++] = reg_blu_ld_blight_dp.ld_blight_dat_msb;
			}
		}
	}

	for(i = 0; i < ld_blk_hnum + 1; i++) {
		for(j = 0; j < ld_blk_vnum + 1; j++) {
				//rtd_printk(KERN_EMERG, TAG_NAME, "AlgoSRAM[%d][%d] = %d\n", i, j, AlgoSRAM[i][j]);
				sum += ptr->AlgoSRAM[i][j];
			}
	}
	ptr->AlgoSRAM_avg = sum / ((ld_blk_vnum + 1) * (ld_blk_hnum + 1));
	// sw mode disable
	reg_blu_ld_sram_ctrl.regValue = IoReg_Read32(BLU_LD_SRAM_CTRL_reg);
	reg_blu_ld_sram_ctrl.ld_blight_sw_mode = 0;
	IoReg_Write32(BLU_LD_SRAM_CTRL_reg,  reg_blu_ld_sram_ctrl.regValue);

}

void drvif_color_set_LD_Algo_sram(LED_AlgoSRAM* ptr)
{

	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
	blu_ld_blight_ap_RBUS reg_blu_ld_blight_ap;
	blu_ld_blight_dp_RBUS reg_blu_ld_blight_dp;
	blu_ld_sram_ctrl_RBUS reg_blu_ld_sram_ctrl;
	blu_ld_smartld_control_RBUS reg_blu_ldsram_ctrl;

	unsigned short ld_blk_type, ld_blk_hnum, ld_blk_vnum;
	short i, j, count, temp;

	// get global set
	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	ld_blk_type = blu_ld_global_ctrl2.ld_blk_type;
	ld_blk_hnum = blu_ld_global_ctrl2.ld_blk_hnum;
	ld_blk_vnum = blu_ld_global_ctrl2.ld_blk_vnum;

	for(i = 0; i < ld_blk_vnum + 1; i++) {
		for(j = 0; j < ld_blk_hnum + 1; j++) {
			if(ptr->AlgoSRAM[i][j] > 65535) {
				ptr->AlgoSRAM[i][j] = 65535;
			}

		}
	}

	// enable; setup sw mode
	reg_blu_ld_sram_ctrl.regValue = IoReg_Read32(BLU_LD_SRAM_CTRL_reg);
	reg_blu_ld_sram_ctrl.ld_blight_sw_mode = 1;
//	reg_blu_ld_sram_ctrl.ld_blight_update_en = 1;
	IoReg_Write32(BLU_LD_SRAM_CTRL_reg,  reg_blu_ld_sram_ctrl.regValue);

	if(ld_blk_type == 1) {	//top/bottom mode
		ld_blk_vnum = 2;
		for(j = 0; j < ld_blk_hnum + 1; j++) {
			//addr
			reg_blu_ld_blight_ap.regValue = IoReg_Read32(BLU_LD_BLIGHT_AP_reg);
			//reg_blu_ld_blight_ap.ld_blight_adr = j * ((ld_blk_vnum + 1) >> 1);//
			if(j<((ld_blk_hnum + 1)>>1))
			{
				reg_blu_ld_blight_ap.ld_blight_even_line = 1;
				reg_blu_ld_blight_ap.ld_blight_delta = j;
				reg_blu_ld_blight_ap.ld_blight_offset = 0;
			}
			else
			{
				reg_blu_ld_blight_ap.ld_blight_even_line = 0;
				reg_blu_ld_blight_ap.ld_blight_delta = j;
				reg_blu_ld_blight_ap.ld_blight_offset = (ld_blk_hnum + 1)>>1;
			}
			IoReg_Write32(BLU_LD_BLIGHT_AP_reg, reg_blu_ld_blight_ap.regValue);
			for(i = 0; i < ld_blk_vnum;) {
				//data
				reg_blu_ld_blight_dp.ld_blight_dat_lsb = ptr->AlgoSRAM[i++][j];
				reg_blu_ld_blight_dp.ld_blight_dat_msb = ptr->AlgoSRAM[i++][j];
				IoReg_Write32(BLU_LD_BLIGHT_DP_reg, reg_blu_ld_blight_dp.regValue);
			}
		}
	}
	else if(ld_blk_type == 2) {	//left/right mode
		ld_blk_hnum = 2;
		for(i = 0; i < ld_blk_vnum + 1; i++) {
			for(j = 0; j < ld_blk_hnum; j++) {
				AlgoSRAMTmp[i * ld_blk_hnum + j] = ptr->AlgoSRAM[i][j];
			}
		}
		i = 0;
		while(i < (ld_blk_vnum * ld_blk_hnum) + 1) {
			//addr
			reg_blu_ld_blight_ap.regValue = IoReg_Read32(BLU_LD_BLIGHT_AP_reg);
			//reg_blu_ld_blight_ap.ld_blight_adr = i >> 1;//
			temp = i/2;
			if(temp/2 == 0)
				count = 1;
			else
				count = 0;
			reg_blu_ld_blight_ap.ld_blight_even_line = count;
			reg_blu_ld_blight_ap.ld_blight_delta = 0;
			reg_blu_ld_blight_ap.ld_blight_offset = temp*2;
			IoReg_Write32(BLU_LD_BLIGHT_AP_reg, reg_blu_ld_blight_ap.regValue);
			//data
			reg_blu_ld_blight_dp.ld_blight_dat_lsb = AlgoSRAMTmp[i++];
			reg_blu_ld_blight_dp.ld_blight_dat_msb = AlgoSRAMTmp[i++];
			IoReg_Write32(BLU_LD_BLIGHT_DP_reg, reg_blu_ld_blight_dp.regValue);
		}
	}
	else {
		// addr
		reg_blu_ld_blight_ap.regValue = IoReg_Read32(BLU_LD_BLIGHT_AP_reg);
		//reg_blu_ld_blight_ap.ld_blight_adr = 0x00;//
		reg_blu_ld_blight_ap.ld_blight_even_line = 1;
		reg_blu_ld_blight_ap.ld_blight_delta = 0;
		reg_blu_ld_blight_ap.ld_blight_offset = 0;
		IoReg_Write32(BLU_LD_BLIGHT_AP_reg, reg_blu_ld_blight_ap.regValue);
		for(i = 0; i < ld_blk_hnum + 1; i++) {
			for(j = 0; j < ld_blk_vnum + 1;) {
				// data
				reg_blu_ld_blight_dp.ld_blight_dat_lsb = ptr->AlgoSRAM[i][j++];
				reg_blu_ld_blight_dp.ld_blight_dat_msb = ptr->AlgoSRAM[i][j++];
				IoReg_Write32(BLU_LD_BLIGHT_DP_reg, reg_blu_ld_blight_dp.regValue);
			}
		}
	}

	// sw mode disable
	reg_blu_ld_sram_ctrl.regValue = IoReg_Read32(BLU_LD_SRAM_CTRL_reg);
	reg_blu_ld_sram_ctrl.ld_blight_sw_mode = 0;
//	reg_blu_ld_sram_ctrl.ld_blight_update_en = 1;
	IoReg_Write32(BLU_LD_SRAM_CTRL_reg,  reg_blu_ld_sram_ctrl.regValue);

	// software go
	reg_blu_ldsram_ctrl.regValue = IoReg_Read32(BLU_LD_SMARTLD_control_reg);
	reg_blu_ldsram_ctrl.hw_blight_end_sw_go = 1;
	IoReg_Write32(BLU_LD_SMARTLD_control_reg,  reg_blu_ldsram_ctrl.regValue);

}


unsigned short drvif_color_get_LD_RGB_Max(void)
{
	return BLU_LD_Global_Max_Ctrl_get_ld_global_max(IoReg_Read32(BLU_LD_Global_Max_Ctrl_reg));
}

unsigned char drvif_color_get_LD_RGB_Max_Hist(unsigned short RGB_Max_Hist[16])
{
	unsigned int tmpValue;

	if (RGB_Max_Hist == NULL)
		return FALSE;

	tmpValue = IoReg_Read32(BLU_LD_Global_Hist1_reg);
	RGB_Max_Hist[0] = BLU_LD_Global_Hist1_get_ld_global_hist0(tmpValue);
	RGB_Max_Hist[1] = BLU_LD_Global_Hist1_get_ld_global_hist1(tmpValue);
	tmpValue = IoReg_Read32(BLU_LD_Global_Hist2_reg);
	RGB_Max_Hist[2] = BLU_LD_Global_Hist2_get_ld_global_hist2(tmpValue);
	RGB_Max_Hist[3] = BLU_LD_Global_Hist2_get_ld_global_hist3(tmpValue);
	tmpValue = IoReg_Read32(BLU_LD_Global_Hist3_reg);
	RGB_Max_Hist[4] = BLU_LD_Global_Hist3_get_ld_global_hist4(tmpValue);
	RGB_Max_Hist[5] = BLU_LD_Global_Hist3_get_ld_global_hist5(tmpValue);
	tmpValue = IoReg_Read32(BLU_LD_Global_Hist4_reg);
	RGB_Max_Hist[6] = BLU_LD_Global_Hist4_get_ld_global_hist6(tmpValue);
	RGB_Max_Hist[7] = BLU_LD_Global_Hist4_get_ld_global_hist7(tmpValue);
	tmpValue = IoReg_Read32(BLU_LD_Global_Hist5_reg);
	RGB_Max_Hist[8] = BLU_LD_Global_Hist5_get_ld_global_hist8(tmpValue);
	RGB_Max_Hist[9] = BLU_LD_Global_Hist5_get_ld_global_hist9(tmpValue);
	tmpValue = IoReg_Read32(BLU_LD_Global_Hist6_reg);
	RGB_Max_Hist[10] = BLU_LD_Global_Hist6_get_ld_global_hist10(tmpValue);
	RGB_Max_Hist[11] = BLU_LD_Global_Hist6_get_ld_global_hist11(tmpValue);
	tmpValue = IoReg_Read32(BLU_LD_Global_Hist7_reg);
	RGB_Max_Hist[12] = BLU_LD_Global_Hist7_get_ld_global_hist12(tmpValue);
	RGB_Max_Hist[13] = BLU_LD_Global_Hist7_get_ld_global_hist13(tmpValue);
	tmpValue = IoReg_Read32(BLU_LD_Global_Hist8_reg);
	RGB_Max_Hist[14] = BLU_LD_Global_Hist8_get_ld_global_hist14(tmpValue);
	RGB_Max_Hist[15] = BLU_LD_Global_Hist8_get_ld_global_hist15(tmpValue);

	return TRUE;
}

unsigned char drvif_color_get_LD_APL_ave_by_num( unsigned short* APL_data, unsigned short array_num )
{
#ifdef K24_PQ_COMPILE
	int i,j;
	unsigned short hnum, vnum;
	unsigned short block_addr;
	blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
	blu_ld_apl_read_ave_1_RBUS ld_apl_read_ave_1_reg;
	blu_ld_apl_read_ave_2_RBUS ld_apl_read_ave_2_reg;
	blu_ld_apl_read_ave_3_RBUS ld_apl_read_ave_3_reg;
	unsigned char LD_type, LD_Hnum, LD_Vnum;
	unsigned char valid;
	//unsigned short access_ptr;
	unsigned short mdata;
	unsigned short ldata;
	unsigned short timeout;

	ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	LD_type = ld_global_ctrl2_reg.ld_blk_type;
	LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum;
	LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum;

	if( LD_type == 0 ) // direct
	{
		hnum = LD_Hnum+1;
		vnum = LD_Vnum+1;
	}
	else if( LD_type == 1 ) // edge top/down
	{
		hnum = LD_Hnum+1;
		vnum = (LD_Vnum+1)*4;
	}
	else // edge left/right
	{
		hnum = (LD_Hnum+1)*4;
		vnum = LD_Vnum+1;
	}

	if (array_num < hnum*vnum)
		return 0;

	ld_apl_read_ave_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_1_reg);
	ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);

	ld_apl_read_ave_2_reg.ld_aplave_addr = 0;
	ld_apl_read_ave_2_reg.ld_aplave_access_en = 1;

	ld_apl_read_ave_1_reg.ld_aplave_conti = 1;
	ld_apl_read_ave_1_reg.ld_aplave_start = 1;

	IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);
	IoReg_Write32(BLU_LD_APL_read_ave_1_reg, ld_apl_read_ave_1_reg.regValue);

	for( i=0; i<vnum; i++ )
	{
		/* //remove constrain
		if( hnum < 32)
		{
			ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);
			ld_apl_read_ave_2_reg.ld_aplave_addr = i*16;
			ld_apl_read_ave_2_reg.ld_aplave_access_en = 0;
			IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);
			ld_apl_read_ave_2_reg.ld_aplave_access_en = 1;
			IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);
		}
		*/
	    for( j=0; j<((hnum+1)>>1); j++ )
	    {
			timeout = 20;
			do {
				ld_apl_read_ave_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_3_reg);
				valid = (ld_apl_read_ave_3_reg.regValue & 0x20000000) >> 29;
				timeout--;
				if( timeout == 0 ) {
					ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);
					ld_apl_read_ave_2_reg.ld_aplave_access_en = 0;
					IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);
					return 0;
				}
			} while( !valid );

			//access_ptr = (ld_apl_read_ave_3_reg.regValue & 0x1FF00000) >> 20;
			mdata = (ld_apl_read_ave_3_reg.regValue & 0x000FFC00) >> 10;
			ldata = (ld_apl_read_ave_3_reg.regValue & 0x000003FF);

			/* //remove constrain
			if( hnum < 32) {
			    if( i*16+j != access_ptr ) {// access ptr wrong
			    	ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);
					ld_apl_read_ave_2_reg.ld_aplave_access_en = 0;
					IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);
			    	return 0;
			    }
			}
			*/

			block_addr = i*hnum + j*2;
			APL_data[block_addr] = ldata;
			if( 2*j+1 < hnum )
				APL_data[block_addr+1] = mdata;
	    }
	}

	ld_apl_read_ave_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_ave_2_reg);
	ld_apl_read_ave_2_reg.ld_aplave_access_en = 0;
	IoReg_Write32(BLU_LD_APL_read_ave_2_reg, ld_apl_read_ave_2_reg.regValue);

	return 1;
#else
	return 0;
#endif
}

unsigned char drvif_color_get_LD_APL_max_by_num( unsigned short* APL_data, unsigned short array_num )
{
#ifdef K24_PQ_COMPILE
	int i,j;
	unsigned short hnum, vnum;
	unsigned short block_addr;
	blu_ld_global_ctrl2_RBUS ld_global_ctrl2_reg;
	blu_ld_apl_read_max_1_RBUS ld_apl_read_max_1_reg;
	blu_ld_apl_read_max_2_RBUS ld_apl_read_max_2_reg;
	blu_ld_apl_read_max_3_RBUS ld_apl_read_max_3_reg;
	unsigned char LD_type, LD_Hnum, LD_Vnum;
	unsigned char valid;
	//unsigned short access_ptr;
	unsigned short mdata;
	unsigned short ldata;
	unsigned short timeout;

	ld_global_ctrl2_reg.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
	LD_type = ld_global_ctrl2_reg.ld_blk_type;
	LD_Hnum = ld_global_ctrl2_reg.ld_blk_hnum;
	LD_Vnum = ld_global_ctrl2_reg.ld_blk_vnum;

	if( LD_type == 0 ) // direct
	{
		hnum = LD_Hnum+1;
		vnum = LD_Vnum+1;
	}
	else if( LD_type == 1 ) // edge top/down
	{
		hnum = LD_Hnum+1;
		vnum = (LD_Vnum+1)*4;
	}
	else // edge left/right
	{
		hnum = (LD_Hnum+1)*4;
		vnum = LD_Vnum+1;
	}

	if (array_num < hnum*vnum)
		return 0;

	ld_apl_read_max_1_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_1_reg);
	ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);

	ld_apl_read_max_2_reg.ld_aplmax_addr = 0;
	ld_apl_read_max_2_reg.ld_aplmax_access_en = 1;

	ld_apl_read_max_1_reg.ld_aplmax_conti = 1;
	ld_apl_read_max_1_reg.ld_aplmax_start = 1;

	IoReg_Write32(BLU_LD_APL_read_max_2_reg, ld_apl_read_max_2_reg.regValue);
	IoReg_Write32(BLU_LD_APL_read_max_1_reg, ld_apl_read_max_1_reg.regValue);

	for( i=0; i<vnum; i++ )
	{
		/* //remove constrain
		if( hnum < 32 )
		{
			ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);
			ld_apl_read_max_2_reg.ld_aplmax_addr = i*16;
			ld_apl_read_max_2_reg.ld_aplmax_access_en = 0;
			IoReg_Write32(BLU_LD_APL_read_max_2_reg, ld_apl_read_max_2_reg.regValue);
			ld_apl_read_max_2_reg.ld_aplmax_access_en = 1;
			IoReg_Write32(BLU_LD_APL_read_max_2_reg, ld_apl_read_max_2_reg.regValue);
		}
		*/
		for( j=0; j<((hnum+1)>>1); j++ )
		{
			timeout = 20;
			do {
				ld_apl_read_max_3_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_3_reg);
				valid = (ld_apl_read_max_3_reg.regValue & 0x20000000) >> 29;
				timeout--;
				if( timeout == 0 ) {
					ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);
					ld_apl_read_max_2_reg.ld_aplmax_access_en = 0;
					IoReg_Write32(BLU_LD_APL_read_max_2_reg, ld_apl_read_max_2_reg.regValue);
					return 0;
				}
			} while( !valid );

			//access_ptr = (ld_apl_read_max_3_reg.regValue & 0x1FF00000) >> 20;
			mdata = (ld_apl_read_max_3_reg.regValue & 0x000FFC00) >> 10;
			ldata = (ld_apl_read_max_3_reg.regValue & 0x000003FF);

			/* //remove constrain
			if( hnum < 32) {
				if( i*16+j != access_ptr ) {// access ptr wrong
					ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);
					ld_apl_read_max_2_reg.ld_aplmax_access_en = 0;
					IoReg_Write32(BLU_LD_APL_read_max_2_reg, ld_apl_read_max_2_reg.regValue);
					return 0;
				}
			}
			*/

			block_addr = i*hnum + j*2;
			APL_data[block_addr] = ldata;
			if( 2*j+1 < hnum )
				APL_data[block_addr+1] = mdata;
		}
	}

	ld_apl_read_max_2_reg.regValue = IoReg_Read32(BLU_LD_APL_read_max_2_reg);
	ld_apl_read_max_2_reg.ld_aplmax_access_en = 0;
	IoReg_Write32(BLU_LD_APL_read_max_2_reg, ld_apl_read_max_2_reg.regValue);

	return 1;
#else
	return 0;
#endif
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

