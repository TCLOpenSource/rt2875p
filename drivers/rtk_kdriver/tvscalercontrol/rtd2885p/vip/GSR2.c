/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2021
  * All rights reserved.
  * ========================================================================*/
  /*================= File Description =======================================*/
  /**
   * @file
   *  This file is for GSR2 related functions.
   *
   * @author     Bill Chou
   * @date	  2021/03/16
   * @version	  1
   */

  /*============================ Module dependency	===========================*/
#include <linux/string.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <linux/kernel.h>
//#include <mach/io.h>
#include <linux/delay.h>

#include "tvscalercontrol/vip/GSR2.h" 
#include <tvscalercontrol/scaler/vipinclude.h> // for typedef that used in VIP_reg_def.h
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"


#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_LC_DEBUG,fmt,##args) 

static VIP_SLD2GlobalSLD_CTRL SLD2GlobalSLD_CTRL = {
	// 0.mode_En; 1. Debug_Mode_Log; 2. Debug_Mode_Log_Delay;
	1, 0, 0,
	// 3. Logo_TH;
	512,
	// 4-12: SLD_CNT_Score_1-8(128-1024);	// 1024 = 1
	{0, 0, 0, 0, 0, 128, 256, 1024, 16384,},
	// reserved
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	// Debug_Blending_Score_by_th
	0xFFFF,
};

static VIP_SLD2GlobalSLD_INFO SLD2GlobalSLD_INFO = {0};

static DRV_GSR2_Table SLD2GlobalSLD_Table = {
	// unsigned short GSR2_global_gain;
	255,
	// DRV_GSR2_Tonemap_curve GSR2_tonemap_curve;
	{
		{0, 256, 512, 768, 1024,1280, 1536, 1792, 2048, 2304, 2520, 2710, 2885, 3050, 3165, 3290, 3355,}
	},
	// DRV_GSR2_gain_by_sat GSR2_gain_by_sat;
	{
		// unsigned short gsr2_sat_mode;
		1, 
		// unsigned short gsr2_gain_by_sat_curve[GSR2_IDX];
		{255, 255, 255, 255, 255, 255, 255, 255, 254, 251, 250, 248, 246, 243, 238, 231, 219,}
	},
};

// Function Description : This function is for GSR2 Global_Gain (Global_Gain=0 -> bypass GSR2)
void drvif_color_set_GSR2_Global_Gain(unsigned char global_gain)
{
	gsr2_gsr2_global_gain_RBUS gsr2_gsr2_global_gain_reg;
	gsr2_gsr2_db_ctrl_RBUS gsr2_gsr2_db_ctrl_reg;

	// double buffer enable
	gsr2_gsr2_db_ctrl_reg.regValue = IoReg_Read32(GSR2_GSR2_DB_CTRL_reg);
	gsr2_gsr2_db_ctrl_reg.db_en_0=1;
	IoReg_Write32(GSR2_GSR2_DB_CTRL_reg, gsr2_gsr2_db_ctrl_reg.regValue);

	gsr2_gsr2_global_gain_reg.regValue = IoReg_Read32(GSR2_GSR2_GLOBAL_GAIN_reg);
	gsr2_gsr2_global_gain_reg.gsr2_global_gain=global_gain;
	IoReg_Write32(GSR2_GSR2_GLOBAL_GAIN_reg, gsr2_gsr2_global_gain_reg.regValue);

	//double buffer apply
	gsr2_gsr2_db_ctrl_reg.regValue = IoReg_Read32(GSR2_GSR2_DB_CTRL_reg);
	gsr2_gsr2_db_ctrl_reg.db_apply_0=1;
	IoReg_Write32(GSR2_GSR2_DB_CTRL_reg, gsr2_gsr2_db_ctrl_reg.regValue);
}
/* GSR2 ToneMapping set function */
void drvif_color_set_GSR2_ToneMapping(DRV_GSR2_Tonemap_curve *curve)
{
	gsr2_gsr2_tonemap_0_RBUS gsr2_gsr2_tonemap_0_reg;
	gsr2_gsr2_tonemap_1_RBUS gsr2_gsr2_tonemap_1_reg;
	gsr2_gsr2_tonemap_2_RBUS gsr2_gsr2_tonemap_2_reg;
	gsr2_gsr2_tonemap_3_RBUS gsr2_gsr2_tonemap_3_reg;
	gsr2_gsr2_tonemap_4_RBUS gsr2_gsr2_tonemap_4_reg;
	gsr2_gsr2_tonemap_5_RBUS gsr2_gsr2_tonemap_5_reg;
	gsr2_gsr2_tonemap_6_RBUS gsr2_gsr2_tonemap_6_reg;
	gsr2_gsr2_tonemap_7_RBUS gsr2_gsr2_tonemap_7_reg;
	gsr2_gsr2_tonemap_8_RBUS gsr2_gsr2_tonemap_8_reg;
	gsr2_gsr2_db_ctrl_RBUS gsr2_gsr2_db_ctrl_reg;

	// double buffer enable
	gsr2_gsr2_db_ctrl_reg.regValue = IoReg_Read32(GSR2_GSR2_DB_CTRL_reg);
	gsr2_gsr2_db_ctrl_reg.db_en_0=1;
	IoReg_Write32(GSR2_GSR2_DB_CTRL_reg, gsr2_gsr2_db_ctrl_reg.regValue);

	gsr2_gsr2_tonemap_0_reg.regValue = IoReg_Read32(GSR2_GSR2_TONEMAP_0_reg);
	gsr2_gsr2_tonemap_1_reg.regValue = IoReg_Read32(GSR2_GSR2_TONEMAP_1_reg);
	gsr2_gsr2_tonemap_2_reg.regValue = IoReg_Read32(GSR2_GSR2_TONEMAP_2_reg);
	gsr2_gsr2_tonemap_3_reg.regValue = IoReg_Read32(GSR2_GSR2_TONEMAP_3_reg);
	gsr2_gsr2_tonemap_4_reg.regValue = IoReg_Read32(GSR2_GSR2_TONEMAP_4_reg);
	gsr2_gsr2_tonemap_5_reg.regValue = IoReg_Read32(GSR2_GSR2_TONEMAP_5_reg);
	gsr2_gsr2_tonemap_6_reg.regValue = IoReg_Read32(GSR2_GSR2_TONEMAP_6_reg);
	gsr2_gsr2_tonemap_7_reg.regValue = IoReg_Read32(GSR2_GSR2_TONEMAP_7_reg);
	gsr2_gsr2_tonemap_8_reg.regValue = IoReg_Read32(GSR2_GSR2_TONEMAP_8_reg);

	gsr2_gsr2_tonemap_0_reg.gsr2_tonemap_out_0=curve->gsr2_tonemap_curve[0];
	gsr2_gsr2_tonemap_0_reg.gsr2_tonemap_out_1=curve->gsr2_tonemap_curve[1];
	gsr2_gsr2_tonemap_1_reg.gsr2_tonemap_out_2=curve->gsr2_tonemap_curve[2];
	gsr2_gsr2_tonemap_1_reg.gsr2_tonemap_out_3=curve->gsr2_tonemap_curve[3];
	gsr2_gsr2_tonemap_2_reg.gsr2_tonemap_out_4=curve->gsr2_tonemap_curve[4];
	gsr2_gsr2_tonemap_2_reg.gsr2_tonemap_out_5=curve->gsr2_tonemap_curve[5];
	gsr2_gsr2_tonemap_3_reg.gsr2_tonemap_out_6=curve->gsr2_tonemap_curve[6];
	gsr2_gsr2_tonemap_3_reg.gsr2_tonemap_out_7=curve->gsr2_tonemap_curve[7];
	gsr2_gsr2_tonemap_4_reg.gsr2_tonemap_out_8=curve->gsr2_tonemap_curve[8];
	gsr2_gsr2_tonemap_4_reg.gsr2_tonemap_out_9=curve->gsr2_tonemap_curve[9];
	gsr2_gsr2_tonemap_5_reg.gsr2_tonemap_out_10=curve->gsr2_tonemap_curve[10];
	gsr2_gsr2_tonemap_5_reg.gsr2_tonemap_out_11=curve->gsr2_tonemap_curve[11];
	gsr2_gsr2_tonemap_6_reg.gsr2_tonemap_out_12=curve->gsr2_tonemap_curve[12];
	gsr2_gsr2_tonemap_6_reg.gsr2_tonemap_out_13=curve->gsr2_tonemap_curve[13];
	gsr2_gsr2_tonemap_7_reg.gsr2_tonemap_out_14=curve->gsr2_tonemap_curve[14];
	gsr2_gsr2_tonemap_7_reg.gsr2_tonemap_out_15=curve->gsr2_tonemap_curve[15];
	gsr2_gsr2_tonemap_8_reg.gsr2_tonemap_out_16=curve->gsr2_tonemap_curve[16];
	
	IoReg_Write32(GSR2_GSR2_TONEMAP_0_reg, gsr2_gsr2_tonemap_0_reg.regValue);
	IoReg_Write32(GSR2_GSR2_TONEMAP_1_reg, gsr2_gsr2_tonemap_1_reg.regValue);
	IoReg_Write32(GSR2_GSR2_TONEMAP_2_reg, gsr2_gsr2_tonemap_2_reg.regValue);
	IoReg_Write32(GSR2_GSR2_TONEMAP_3_reg, gsr2_gsr2_tonemap_3_reg.regValue);
	IoReg_Write32(GSR2_GSR2_TONEMAP_4_reg, gsr2_gsr2_tonemap_4_reg.regValue);
	IoReg_Write32(GSR2_GSR2_TONEMAP_5_reg, gsr2_gsr2_tonemap_5_reg.regValue);
	IoReg_Write32(GSR2_GSR2_TONEMAP_6_reg, gsr2_gsr2_tonemap_6_reg.regValue);
	IoReg_Write32(GSR2_GSR2_TONEMAP_7_reg, gsr2_gsr2_tonemap_7_reg.regValue);
	IoReg_Write32(GSR2_GSR2_TONEMAP_8_reg, gsr2_gsr2_tonemap_8_reg.regValue);

	//double buffer apply
	gsr2_gsr2_db_ctrl_reg.regValue = IoReg_Read32(GSR2_GSR2_DB_CTRL_reg);
	gsr2_gsr2_db_ctrl_reg.db_apply_0=1;
	IoReg_Write32(GSR2_GSR2_DB_CTRL_reg, gsr2_gsr2_db_ctrl_reg.regValue);
}

/* GSR2 Gain_by_sat set function */
void drvif_color_set_GSR2_Gain_by_Sat(DRV_GSR2_gain_by_sat *sat)
{

	gsr2_gsr2_gain_by_sat_0_RBUS gsr2_gsr2_gain_by_sat_0_reg;
	gsr2_gsr2_gain_by_sat_1_RBUS gsr2_gsr2_gain_by_sat_1_reg;
	gsr2_gsr2_gain_by_sat_2_RBUS gsr2_gsr2_gain_by_sat_2_reg;
	gsr2_gsr2_gain_by_sat_3_RBUS gsr2_gsr2_gain_by_sat_3_reg;
	gsr2_gsr2_gain_by_sat_4_RBUS gsr2_gsr2_gain_by_sat_4_reg;
	gsr2_gsr2_gain_by_sat_5_RBUS gsr2_gsr2_gain_by_sat_5_reg;
	gsr2_gsr2_gain_by_sat_6_RBUS gsr2_gsr2_gain_by_sat_6_reg;
	gsr2_gsr2_gain_by_sat_7_RBUS gsr2_gsr2_gain_by_sat_7_reg;
	gsr2_gsr2_gain_by_sat_8_RBUS gsr2_gsr2_gain_by_sat_8_reg;
	gsr2_gsr2_gain_by_sat_9_RBUS gsr2_gsr2_gain_by_sat_9_reg;
	gsr2_gsr2_db_ctrl_RBUS gsr2_gsr2_db_ctrl_reg;

	// double buffer enable
	gsr2_gsr2_db_ctrl_reg.regValue = IoReg_Read32(GSR2_GSR2_DB_CTRL_reg);
	gsr2_gsr2_db_ctrl_reg.db_en_0=1;
	IoReg_Write32(GSR2_GSR2_DB_CTRL_reg, gsr2_gsr2_db_ctrl_reg.regValue);

	gsr2_gsr2_gain_by_sat_0_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_0_reg);
	gsr2_gsr2_gain_by_sat_1_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_1_reg);
	gsr2_gsr2_gain_by_sat_2_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_2_reg);
	gsr2_gsr2_gain_by_sat_3_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_3_reg);
	gsr2_gsr2_gain_by_sat_4_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_4_reg);
	gsr2_gsr2_gain_by_sat_5_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_5_reg);
	gsr2_gsr2_gain_by_sat_6_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_6_reg);
	gsr2_gsr2_gain_by_sat_7_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_7_reg);
	gsr2_gsr2_gain_by_sat_8_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_8_reg);
	gsr2_gsr2_gain_by_sat_9_reg.regValue = IoReg_Read32(GSR2_GSR2_GAIN_BY_SAT_9_reg);

	gsr2_gsr2_gain_by_sat_0_reg.gsr2_sat_mode=sat->gsr2_sat_mode;
	gsr2_gsr2_gain_by_sat_1_reg.gsr2_gain_by_sat_0=sat->gsr2_gain_by_sat_curve[0];
	gsr2_gsr2_gain_by_sat_1_reg.gsr2_gain_by_sat_1=sat->gsr2_gain_by_sat_curve[1];
	gsr2_gsr2_gain_by_sat_2_reg.gsr2_gain_by_sat_2=sat->gsr2_gain_by_sat_curve[2];
	gsr2_gsr2_gain_by_sat_2_reg.gsr2_gain_by_sat_3=sat->gsr2_gain_by_sat_curve[3];
	gsr2_gsr2_gain_by_sat_3_reg.gsr2_gain_by_sat_4=sat->gsr2_gain_by_sat_curve[4];
	gsr2_gsr2_gain_by_sat_3_reg.gsr2_gain_by_sat_5=sat->gsr2_gain_by_sat_curve[5];
	gsr2_gsr2_gain_by_sat_4_reg.gsr2_gain_by_sat_6=sat->gsr2_gain_by_sat_curve[6];
	gsr2_gsr2_gain_by_sat_4_reg.gsr2_gain_by_sat_7=sat->gsr2_gain_by_sat_curve[7];
	gsr2_gsr2_gain_by_sat_5_reg.gsr2_gain_by_sat_8=sat->gsr2_gain_by_sat_curve[8];
	gsr2_gsr2_gain_by_sat_5_reg.gsr2_gain_by_sat_9=sat->gsr2_gain_by_sat_curve[9];
	gsr2_gsr2_gain_by_sat_6_reg.gsr2_gain_by_sat_10=sat->gsr2_gain_by_sat_curve[10];
	gsr2_gsr2_gain_by_sat_6_reg.gsr2_gain_by_sat_11=sat->gsr2_gain_by_sat_curve[11];
	gsr2_gsr2_gain_by_sat_7_reg.gsr2_gain_by_sat_12=sat->gsr2_gain_by_sat_curve[12];
	gsr2_gsr2_gain_by_sat_7_reg.gsr2_gain_by_sat_13=sat->gsr2_gain_by_sat_curve[13];
	gsr2_gsr2_gain_by_sat_8_reg.gsr2_gain_by_sat_14=sat->gsr2_gain_by_sat_curve[14];
	gsr2_gsr2_gain_by_sat_8_reg.gsr2_gain_by_sat_15=sat->gsr2_gain_by_sat_curve[15];
	gsr2_gsr2_gain_by_sat_9_reg.gsr2_gain_by_sat_16=sat->gsr2_gain_by_sat_curve[16];
	
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_0_reg, gsr2_gsr2_gain_by_sat_0_reg.regValue);
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_1_reg, gsr2_gsr2_gain_by_sat_1_reg.regValue);
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_2_reg, gsr2_gsr2_gain_by_sat_2_reg.regValue);
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_3_reg, gsr2_gsr2_gain_by_sat_3_reg.regValue);
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_4_reg, gsr2_gsr2_gain_by_sat_4_reg.regValue);
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_5_reg, gsr2_gsr2_gain_by_sat_5_reg.regValue);
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_6_reg, gsr2_gsr2_gain_by_sat_6_reg.regValue);
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_7_reg, gsr2_gsr2_gain_by_sat_7_reg.regValue);
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_8_reg, gsr2_gsr2_gain_by_sat_8_reg.regValue);
	IoReg_Write32(GSR2_GSR2_GAIN_BY_SAT_9_reg, gsr2_gsr2_gain_by_sat_9_reg.regValue);

	//double buffer apply
	gsr2_gsr2_db_ctrl_reg.regValue = IoReg_Read32(GSR2_GSR2_DB_CTRL_reg);
	gsr2_gsr2_db_ctrl_reg.db_apply_0=1;
	IoReg_Write32(GSR2_GSR2_DB_CTRL_reg, gsr2_gsr2_db_ctrl_reg.regValue);

}

/* GSR2 Gain_by_Y set function */
void drvif_color_set_GSR2_Gain_by_Y(DRV_GSR2_gain_by_y *gain_y)
{
	gsr2_gsr2_gain_by_y_RBUS gsr2_gsr2_gain_by_y_reg;
	gsr2_gsr2_db_ctrl_RBUS gsr2_gsr2_db_ctrl_reg;

	// double buffer enable
	gsr2_gsr2_db_ctrl_reg.regValue = IoReg_Read32(GSR2_GSR2_DB_CTRL_reg);
	gsr2_gsr2_db_ctrl_reg.db_en_0=1;
	IoReg_Write32(GSR2_GSR2_DB_CTRL_reg, gsr2_gsr2_db_ctrl_reg.regValue);

	gsr2_gsr2_gain_by_y_reg.regValue= IoReg_Read32(GSR2_GSR2_GAIN_BY_Y_reg);

	gsr2_gsr2_gain_by_y_reg.gsr2_gain_by_y_coef_b = gain_y->gsr_gain_by_Y_coef_B;
	gsr2_gsr2_gain_by_y_reg.gsr2_gain_by_y_coef_g= gain_y->gsr_gain_by_Y_coef_G;
	gsr2_gsr2_gain_by_y_reg.gsr2_gain_by_y_coef_r= gain_y->gsr_gain_by_Y_coef_R;
	
	gsr2_gsr2_gain_by_y_reg.gsr2_gain_by_y_en = gain_y->gsr_gain_by_Y_en;
	
	IoReg_Write32(GSR2_GSR2_GAIN_BY_Y_reg, gsr2_gsr2_gain_by_y_reg.regValue);

	//double buffer apply
	gsr2_gsr2_db_ctrl_reg.regValue = IoReg_Read32(GSR2_GSR2_DB_CTRL_reg);
	gsr2_gsr2_db_ctrl_reg.db_apply_0=1;
	IoReg_Write32(GSR2_GSR2_DB_CTRL_reg, gsr2_gsr2_db_ctrl_reg.regValue);
}

VIP_SLD2GlobalSLD_CTRL* drvif_fwif_color_get_SLD2GlobalSLD_CTRL(void)
{
	return &SLD2GlobalSLD_CTRL;
}

VIP_SLD2GlobalSLD_INFO* drvif_fwif_color_get_SLD2GlobalSLD_INFO(void)
{
	return &SLD2GlobalSLD_INFO;
}

DRV_GSR2_Table* drvif_fwif_color_get_SLD2GlobalSLD_TBL(void)
{
	return &SLD2GlobalSLD_Table;
}

