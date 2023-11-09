/*==========================================================================
    * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for AI related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version $Revision$
 */

/*============================ Module dependency  ===========================*/
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/vip/ai_pq.h>
//#include "vgip_isr/scalerVIP.h"
#include <scaler/vipRPCCommon.h>
//#include "gal/rtk_kadp_se.h"

#include <rbus/color_sharp_reg.h>
#include <rbus/color_dcc_reg.h>
#include <rbus/color_icm_reg.h>
#include <rbus/gamma_reg.h>
#include <rbus/color_reg.h>
#include <rbus/di_reg.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerCommon.h>
	#include <scalercommon/scalerDrvCommon.h>
#endif

#include <tvscalercontrol/io/ioregdrv.h>
//#include <linux/v4l2-ext/videodev2-ext.h> // V4L2_ERR
#include <tvscalercontrol/vip/scalerColor.h>

#if defined(CONFIG_RTK_AI_DRV)
/*================================== Variables ==============================*/
#define c_cds_region_smooth 16
DRV_AI_Ctrl_table ai_ctrl;
// for scene_detection
DRV_AI_SCENE_Ctrl_table ai_scene_ctrl;

unsigned int CDS_control[3][7] = {
// 0:edge, 1:text, 2:v, 3:v_edge, 4:color_conti, 5:conti_area, 6:smooth_area
{0,0,1,1,1,14,0}, // green rectangle for sport pq
{1,0,0,0,1,14,0}, // for animation pq
{0,0,0,0,0,0,0}, // not set
};
int CDS_uv_range[3][8] = {
// 0:U_c, 1:V_c, 2:U_lb, 3:U_ub, 4:V_lb, 5:V_ub, 6:UV_rad, 7:cm_mode
{275,275, 0,500, 0,480, 276, 0}, // green rectangle for sports pq
{512,512, -14,14, -14,14, 241, 2}, // black white for animation pq
{0,0, 0,0, 0,0, 0, 0}, // not set
};
/*================================== Functions ==============================*/
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
void demo_20CES_tv030(void)
{
	// mac7p AI TBD
	//color_d_vc_global_ctrl_RBUS d_vc_global_ctrl_Reg;
		
	/* disable gamma */
	IoReg_ClearBits(GAMMA_GAMMA_CTRL_2_reg, GAMMA_GAMMA_CTRL_2_gamma_m_tab_sel_mask);//_BIT6|_BIT7

	/* disable DCC */
	#if 0// mac7p AI TBD
	d_vc_global_ctrl_Reg.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
	d_vc_global_ctrl_Reg.dcc_en = 0;/*dcc curve off*/
	d_vc_global_ctrl_Reg.write_enable_1 =1;
	IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_Reg.regValue);
	#endif

	/* Setting local contrast */
	// hack in scalercolorlib.c
	//fwif_color_set_LC_ToneMappingSlopePoint(0, 6);
	//fwif_color_set_LC_saturation(0, 4);	

	/* sharpness double buffer for CDS */
	IoReg_ClearBits(COLOR_SHARP_DM_NR_SHP_CTRL_reg, COLOR_SHARP_DM_NR_SHP_CTRL_db_en_mask);
}

void demo_20CES_tv030_NR(int enable)
{
	if(enable)
	{
		IoReg_Write32(DI_IM_DI_RTNR_Y_16_K0_K7_reg, 0x76543210);
		IoReg_Write32(DI_IM_DI_RTNR_Y_16_K8_K15_reg, 0xfedcba98);
		IoReg_SetBits(0xb8024688, _BIT0); // masnr in rtnr
		
	}
	else
	{
		IoReg_Write32(DI_IM_DI_RTNR_Y_16_K0_K7_reg, 0xffffffff);
		IoReg_Write32(DI_IM_DI_RTNR_Y_16_K8_K15_reg, 0xffffffff);
		IoReg_ClearBits(0xb8024688, _BIT0);
	}
}

// for face effect to init dcc function
void drvif_color_AI_obj_dcc_init(unsigned char enable)
{
	#if 1 // old skin, for global dcc
	color_dcc_d_dcc_skin_tone_en_RBUS d_dcc_skin_tone_en_reg;
	color_dcc_d_dcc_cds_skin_0_RBUS   color_dcc_d_dcc_cds_skin_0_reg;

	d_dcc_skin_tone_en_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg);
	color_dcc_d_dcc_cds_skin_0_reg.regValue    = IoReg_Read32(COLOR_DCC_D_DCC_CDS_SKIN_0_reg);
	
	d_dcc_skin_tone_en_reg.y_blending_en = enable;
    //d_dcc_skin_tone_en_reg.colorspace_sel = 0; // mac7p no this
    d_dcc_skin_tone_en_reg.region_ratio_blending_en = 1;
	d_dcc_skin_tone_en_reg.y_blending_reg0_en = enable;
	//d_dcc_skin_tone_en_reg.y_blending_0_enhance_en = 1; 
	color_dcc_d_dcc_cds_skin_0_reg.cds_skin_en = 0;
	
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg, d_dcc_skin_tone_en_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_CDS_SKIN_0_reg, color_dcc_d_dcc_cds_skin_0_reg.regValue);

	#else // CDS skin
	color_dcc_d_dcc_cds_skin_0_RBUS color_dcc_d_dcc_cds_skin_0_reg;
	color_dcc_d_dcc_skin_tone_en_RBUS color_dcc_d_dcc_skin_tone_en_reg;
	color_dcc_d_dcc_ai_0_RBUS color_dcc_d_dcc_ai_0_reg;
	color_dcc_d_dcc_ai_6_RBUS color_dcc_d_dcc_ai_6_reg;
	color_dcc_d_dcc_cds_skin_1_RBUS color_dcc_d_dcc_cds_skin_1_reg;
	color_dcc_d_dcc_cds_skin_2_RBUS color_dcc_d_dcc_cds_skin_2_reg;
	color_dcc_d_dcc_cds_skin_3_RBUS color_dcc_d_dcc_cds_skin_3_reg;

	color_dcc_d_dcc_cds_skin_0_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_CDS_SKIN_0_reg);
	color_dcc_d_dcc_skin_tone_en_reg.regValue =IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg);
	color_dcc_d_dcc_ai_0_reg.regValue =IoReg_Read32(COLOR_DCC_D_DCC_AI_0_reg);
	color_dcc_d_dcc_ai_6_reg.regValue =IoReg_Read32(COLOR_DCC_D_DCC_AI_6_reg);
	color_dcc_d_dcc_cds_skin_1_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_CDS_SKIN_1_reg);
	color_dcc_d_dcc_cds_skin_2_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_CDS_SKIN_2_reg);
	color_dcc_d_dcc_cds_skin_3_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_CDS_SKIN_3_reg);

	color_dcc_d_dcc_cds_skin_0_reg.cds_skin_en = 1;
	color_dcc_d_dcc_skin_tone_en_reg.y_blending_en = 1; // this register is controled by handler
	color_dcc_d_dcc_ai_0_reg.ai_ros_en = 1;
	color_dcc_d_dcc_ai_6_reg.ai_adjust_en = 1;
	color_dcc_d_dcc_ai_6_reg.ai_control_value = 128;
	color_dcc_d_dcc_ai_6_reg.ai_detect_value = 255;
	color_dcc_d_dcc_cds_skin_1_reg.cds_uv_range_0 = 500;
	color_dcc_d_dcc_cds_skin_1_reg.cds_uv_range_1 = 500;
	color_dcc_d_dcc_cds_skin_2_reg.cds_uv_range_2 = 500;
	color_dcc_d_dcc_cds_skin_2_reg.cds_uv_range_3 = 500;
	color_dcc_d_dcc_cds_skin_3_reg.cds_uv_range_4 = 500;
	color_dcc_d_dcc_cds_skin_3_reg.cds_uv_range_5 = 500;

	IoReg_Write32(COLOR_DCC_D_DCC_CDS_SKIN_0_reg, color_dcc_d_dcc_cds_skin_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg, color_dcc_d_dcc_skin_tone_en_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_0_reg, color_dcc_d_dcc_ai_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_6_reg, color_dcc_d_dcc_ai_6_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_CDS_SKIN_1_reg, color_dcc_d_dcc_cds_skin_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_CDS_SKIN_2_reg, color_dcc_d_dcc_cds_skin_2_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_CDS_SKIN_3_reg, color_dcc_d_dcc_cds_skin_3_reg.regValue);
	#endif
}

// for face effect to init icm function
void drvif_color_AI_obj_icm_init(unsigned char enable)
{
	color_icm_d_icm_cds_skin_0_RBUS color_icm_d_icm_cds_skin_0_reg;
	color_icm_d_icm_ai_0_RBUS color_icm_d_icm_ai_0_reg;

	color_icm_d_icm_cds_skin_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_CDS_SKIN_0_reg);
	color_icm_d_icm_ai_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_0_reg);

	color_icm_d_icm_cds_skin_0_reg.cds_skin_en = enable;
	color_icm_d_icm_ai_0_reg.ai_ros_en = enable;

	IoReg_Write32(COLOR_ICM_D_ICM_CDS_SKIN_0_reg, color_icm_d_icm_cds_skin_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_0_reg, color_icm_d_icm_ai_0_reg.regValue);
}

// for face & scene effect to init shp cds function
void drvif_color_AI_cds_init(unsigned char enable)
{
	color_sharp_dm_cds_cm0_u_bound_0_RBUS color_sharp_dm_cds_cm0_u_bound_0_reg;

	color_sharp_dm_cds_cm0_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg);

	color_sharp_dm_cds_cm0_u_bound_0_reg.cds_enable = enable;

	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg, color_sharp_dm_cds_cm0_u_bound_0_reg.regValue);

	// mac7p no sub function register, always on

}

// Cmodel name: AI_ICM_reg_setting(face_icm_apply);
void drvif_color_AI_obj_icm_set(AI_ICM_apply face_icm_apply_f[6])
{
	color_icm_d_icm_ai_0_RBUS  	 icm_d_icm_ai_0_reg;
	color_icm_d_icm_ai_1_RBUS	 icm_d_icm_ai_1_reg;
	color_icm_d_icm_ai_2_RBUS	 icm_d_icm_ai_2_reg;
	color_icm_d_icm_ai_3_RBUS	 icm_d_icm_ai_3_reg;
	color_icm_d_icm_ai_4_RBUS	 icm_d_icm_ai_4_reg;
	color_icm_d_icm_ai_5_RBUS	 icm_d_icm_ai_5_reg;
	color_icm_d_icm_ai_7_RBUS	 icm_d_icm_ai_7_reg;
	color_icm_d_icm_ai_8_RBUS    icm_d_icm_ai_8_reg;
	color_icm_d_icm_ai_10_RBUS     icm_d_icm_ai_10_reg;
	color_icm_d_icm_ai_11_RBUS     icm_d_icm_ai_11_reg;
	color_icm_d_icm_ai_13_RBUS     icm_d_icm_ai_13_reg;
	color_icm_d_icm_ai_14_RBUS     icm_d_icm_ai_14_reg;
	color_icm_d_icm_ai_16_RBUS     icm_d_icm_ai_16_reg;
	color_icm_d_icm_ai_17_RBUS     icm_d_icm_ai_17_reg;
	color_icm_d_icm_ai_19_RBUS     icm_d_icm_ai_19_reg;
	color_icm_d_icm_ai_20_RBUS     icm_d_icm_ai_20_reg;
	
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

	color_icm_d_icm_skin_tone_segment_c0_0_RBUS	icm_d_icm_skin_tone_segment_c0_0_reg;
	color_icm_d_icm_skin_tone_segment_c0_1_RBUS	icm_d_icm_skin_tone_segment_c0_1_reg;
	color_icm_d_icm_skin_tone_segment_c1_0_RBUS	icm_d_icm_skin_tone_segment_c1_0_reg;
	color_icm_d_icm_skin_tone_segment_c1_1_RBUS	icm_d_icm_skin_tone_segment_c1_1_reg;
	color_icm_d_icm_skin_tone_segment_c2_0_RBUS	icm_d_icm_skin_tone_segment_c2_0_reg;
	color_icm_d_icm_skin_tone_segment_c2_1_RBUS	icm_d_icm_skin_tone_segment_c2_1_reg;
	color_icm_d_icm_skin_tone_segment_c3_0_RBUS	icm_d_icm_skin_tone_segment_c3_0_reg;
	color_icm_d_icm_skin_tone_segment_c3_1_RBUS	icm_d_icm_skin_tone_segment_c3_1_reg;
	color_icm_d_icm_skin_tone_segment_c4_0_RBUS	icm_d_icm_skin_tone_segment_c4_0_reg;
	color_icm_d_icm_skin_tone_segment_c4_1_RBUS	icm_d_icm_skin_tone_segment_c4_1_reg;
	color_icm_d_icm_skin_tone_segment_c5_0_RBUS	icm_d_icm_skin_tone_segment_c5_0_reg;
	color_icm_d_icm_skin_tone_segment_c5_1_RBUS	icm_d_icm_skin_tone_segment_c5_1_reg;

	// share with dcc
	color_dcc_d_dcc_ai_1_RBUS	dcc_d_dcc_ai_1_reg;
	color_dcc_d_dcc_ai_2_RBUS	dcc_d_dcc_ai_2_reg;
	color_dcc_d_dcc_ai_3_RBUS	dcc_d_dcc_ai_3_reg;
	color_dcc_d_dcc_ai_4_RBUS	dcc_d_dcc_ai_4_reg;
	color_dcc_d_dcc_ai_5_RBUS	dcc_d_dcc_ai_5_reg;
	color_dcc_d_dcc_ai_6_RBUS	dcc_d_dcc_ai_6_reg;
	color_dcc_d_dcc_ai_7_RBUS    dcc_d_dcc_ai_7_reg;
	color_dcc_d_dcc_ai_8_RBUS	 dcc_d_dcc_ai_8_reg;
	color_dcc_d_dcc_ai_9_RBUS	 dcc_d_dcc_ai_9_reg;
	color_dcc_d_dcc_ai_10_RBUS   dcc_d_dcc_ai_10_reg;
	color_dcc_d_dcc_ai_11_RBUS	 dcc_d_dcc_ai_11_reg;
	color_dcc_d_dcc_ai_12_RBUS	 dcc_d_dcc_ai_12_reg;
	color_dcc_d_dcc_ai_13_RBUS   dcc_d_dcc_ai_13_reg;
	color_dcc_d_dcc_ai_14_RBUS	 dcc_d_dcc_ai_14_reg;
	color_dcc_d_dcc_ai_15_RBUS	 dcc_d_dcc_ai_15_reg;
	color_dcc_d_dcc_ai_16_RBUS   dcc_d_dcc_ai_16_reg;
	color_dcc_d_dcc_ai_17_RBUS	 dcc_d_dcc_ai_17_reg;
	color_dcc_d_dcc_ai_18_RBUS	 dcc_d_dcc_ai_18_reg;

	icm_d_icm_ai_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_0_reg);
	icm_d_icm_ai_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_1_reg);
	icm_d_icm_ai_2_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_2_reg);
	icm_d_icm_ai_3_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_3_reg);
	icm_d_icm_ai_4_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_4_reg);
	icm_d_icm_ai_5_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_5_reg);
	icm_d_icm_ai_7_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_7_reg);
	icm_d_icm_ai_16_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_16_reg);
	icm_d_icm_ai_17_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_17_reg);
	icm_d_icm_ai_19_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_19_reg);
	icm_d_icm_ai_20_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_20_reg);

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

	icm_d_icm_skin_tone_segment_c0_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C0_0_reg);
	icm_d_icm_skin_tone_segment_c0_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C0_1_reg);
	icm_d_icm_skin_tone_segment_c1_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C1_0_reg);
	icm_d_icm_skin_tone_segment_c1_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C1_1_reg);
	icm_d_icm_skin_tone_segment_c2_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C2_0_reg);
	icm_d_icm_skin_tone_segment_c2_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C2_1_reg);
	icm_d_icm_skin_tone_segment_c3_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C3_0_reg);
	icm_d_icm_skin_tone_segment_c3_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C3_1_reg);
	icm_d_icm_skin_tone_segment_c4_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C4_0_reg);
	icm_d_icm_skin_tone_segment_c4_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C4_1_reg);
	icm_d_icm_skin_tone_segment_c5_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C5_0_reg);
	icm_d_icm_skin_tone_segment_c5_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C5_1_reg);

	dcc_d_dcc_ai_1_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_1_reg);
	dcc_d_dcc_ai_2_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_2_reg);
	dcc_d_dcc_ai_3_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_3_reg);
	dcc_d_dcc_ai_4_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_4_reg);
	dcc_d_dcc_ai_5_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_5_reg);
	dcc_d_dcc_ai_6_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_6_reg);
	dcc_d_dcc_ai_7_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_7_reg);
	dcc_d_dcc_ai_8_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_8_reg);
	dcc_d_dcc_ai_9_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_9_reg);
	dcc_d_dcc_ai_10_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_10_reg);
	dcc_d_dcc_ai_11_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_11_reg);
	dcc_d_dcc_ai_12_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_12_reg);
	dcc_d_dcc_ai_13_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_13_reg);
	dcc_d_dcc_ai_14_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_14_reg);
	dcc_d_dcc_ai_15_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_15_reg);
	dcc_d_dcc_ai_16_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_16_reg);
	dcc_d_dcc_ai_17_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_17_reg);
	dcc_d_dcc_ai_18_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_18_reg);

	//face0
	// win_pos	
	dcc_d_dcc_ai_1_reg.ai_center_u_0 = face_icm_apply_f[0].center_u_s;
	dcc_d_dcc_ai_1_reg.ai_center_v_0 = face_icm_apply_f[0].center_v_s;
	dcc_d_dcc_ai_7_reg.ai_region_start_i_0 = max(0,face_icm_apply_f[0].pos_x_s-(face_icm_apply_f[0].w12>>1));
	dcc_d_dcc_ai_7_reg.ai_region_end_i_0 = min(3839,face_icm_apply_f[0].pos_x_s+(face_icm_apply_f[0].w12>>1));
	dcc_d_dcc_ai_13_reg.ai_region_start_j_0 = max(0,face_icm_apply_f[0].pos_y_s-(face_icm_apply_f[0].h12>>1));
	dcc_d_dcc_ai_13_reg.ai_region_end_j_0 = min(2159,face_icm_apply_f[0].pos_y_s+(face_icm_apply_f[0].h12>>1));
	icm_d_icm_ai_4_reg.ai_region_ilu_0 = face_icm_apply_f[0].w12>>3;
	icm_d_icm_ai_4_reg.ai_region_ild_0 = face_icm_apply_f[0].w12>>3;
	icm_d_icm_ai_4_reg.ai_region_jlu_0 = face_icm_apply_f[0].h12>>3;
	icm_d_icm_ai_4_reg.ai_region_jld_0 = face_icm_apply_f[0].h12>>3;
	icm_d_icm_ai_5_reg.ai_region_iru_0 = face_icm_apply_f[0].w12>>3;
	icm_d_icm_ai_5_reg.ai_region_ird_0 = face_icm_apply_f[0].w12>>3;
	icm_d_icm_ai_5_reg.ai_region_jru_0 = face_icm_apply_f[0].h12>>3;
	icm_d_icm_ai_5_reg.ai_region_jrd_0 = face_icm_apply_f[0].h12>>3;

	// blending ratio
	icm_d_icm_skin_tone_segment_c0_0_reg.c_blending_0_ratio_0=face_icm_apply_f[0].uv_blending_0;
	icm_d_icm_skin_tone_segment_c0_0_reg.c_blending_0_ratio_1=face_icm_apply_f[0].uv_blending_1;
	icm_d_icm_skin_tone_segment_c0_0_reg.c_blending_0_ratio_2=face_icm_apply_f[0].uv_blending_2;
	icm_d_icm_skin_tone_segment_c0_0_reg.c_blending_0_ratio_3=face_icm_apply_f[0].uv_blending_3;
	icm_d_icm_skin_tone_segment_c0_1_reg.c_blending_0_ratio_4=face_icm_apply_f[0].uv_blending_4;
	icm_d_icm_skin_tone_segment_c0_1_reg.c_blending_0_ratio_5=face_icm_apply_f[0].uv_blending_5;
	icm_d_icm_skin_tone_segment_c0_1_reg.c_blending_0_ratio_6=face_icm_apply_f[0].uv_blending_6;
	icm_d_icm_skin_tone_segment_c0_1_reg.c_blending_0_ratio_7=face_icm_apply_f[0].uv_blending_7;

	//hue, sat adjust
	icm_d_icm_ai_offset_1_reg.ai_h_offset0=face_icm_apply_f[0].hue_adj_s;
	icm_d_icm_ai_offset_0_reg.ai_s_offset0=face_icm_apply_f[0].sat_adj_s;
	icm_d_icm_ai_offset_0_reg.ai_i_offset0=face_icm_apply_f[0].int_adj_s;

	//face1
	dcc_d_dcc_ai_2_reg.ai_center_u_1 = face_icm_apply_f[1].center_u_s;
	dcc_d_dcc_ai_2_reg.ai_center_v_1 = face_icm_apply_f[1].center_v_s;
	dcc_d_dcc_ai_8_reg.ai_region_start_i_1 = max(0,face_icm_apply_f[1].pos_x_s-(face_icm_apply_f[1].w12>>1));
	dcc_d_dcc_ai_8_reg.ai_region_end_i_1 = min(3839,face_icm_apply_f[1].pos_x_s+(face_icm_apply_f[1].w12>>1));
	dcc_d_dcc_ai_14_reg.ai_region_start_j_1 = max(0,face_icm_apply_f[1].pos_y_s-(face_icm_apply_f[1].h12>>1));
	dcc_d_dcc_ai_14_reg.ai_region_end_j_1 = min(2159,face_icm_apply_f[1].pos_y_s+(face_icm_apply_f[1].h12>>1));
	icm_d_icm_ai_7_reg.ai_region_ilu_1 = face_icm_apply_f[1].w12>>3;
	icm_d_icm_ai_7_reg.ai_region_ild_1 = face_icm_apply_f[1].w12>>3;
	icm_d_icm_ai_7_reg.ai_region_jlu_1 = face_icm_apply_f[1].h12>>3;
	icm_d_icm_ai_7_reg.ai_region_jld_1 = face_icm_apply_f[1].h12>>3;
	icm_d_icm_ai_8_reg.ai_region_iru_1 = face_icm_apply_f[1].w12>>3;
	icm_d_icm_ai_8_reg.ai_region_ird_1 = face_icm_apply_f[1].w12>>3;
	icm_d_icm_ai_8_reg.ai_region_jru_1 = face_icm_apply_f[1].h12>>3;
	icm_d_icm_ai_8_reg.ai_region_jrd_1 = face_icm_apply_f[1].h12>>3;

	icm_d_icm_skin_tone_segment_c1_0_reg.c_blending_1_ratio_0=face_icm_apply_f[1].uv_blending_0;
	icm_d_icm_skin_tone_segment_c1_0_reg.c_blending_1_ratio_1=face_icm_apply_f[1].uv_blending_1;
	icm_d_icm_skin_tone_segment_c1_0_reg.c_blending_1_ratio_2=face_icm_apply_f[1].uv_blending_2;
	icm_d_icm_skin_tone_segment_c1_0_reg.c_blending_1_ratio_3=face_icm_apply_f[1].uv_blending_3;
	icm_d_icm_skin_tone_segment_c1_1_reg.c_blending_1_ratio_4=face_icm_apply_f[1].uv_blending_4;
	icm_d_icm_skin_tone_segment_c1_1_reg.c_blending_1_ratio_5=face_icm_apply_f[1].uv_blending_5;
	icm_d_icm_skin_tone_segment_c1_1_reg.c_blending_1_ratio_6=face_icm_apply_f[1].uv_blending_6;
	icm_d_icm_skin_tone_segment_c1_1_reg.c_blending_1_ratio_7=face_icm_apply_f[1].uv_blending_7;

	//hue, sat adjust
	icm_d_icm_ai_offset_3_reg.ai_h_offset1=face_icm_apply_f[1].hue_adj_s;
	icm_d_icm_ai_offset_2_reg.ai_s_offset1=face_icm_apply_f[1].sat_adj_s;
	icm_d_icm_ai_offset_2_reg.ai_i_offset1=face_icm_apply_f[1].int_adj_s;


	//face2
	dcc_d_dcc_ai_3_reg.ai_center_u_2 = face_icm_apply_f[2].center_u_s;
	dcc_d_dcc_ai_3_reg.ai_center_v_2 = face_icm_apply_f[2].center_v_s;
	dcc_d_dcc_ai_9_reg.ai_region_start_i_2 = max(0,face_icm_apply_f[2].pos_x_s-(face_icm_apply_f[2].w12>>1));
	dcc_d_dcc_ai_9_reg.ai_region_end_i_2 = min(3839,face_icm_apply_f[2].pos_x_s+(face_icm_apply_f[2].w12>>1));
	dcc_d_dcc_ai_15_reg.ai_region_start_j_2 = max(0,face_icm_apply_f[2].pos_y_s-(face_icm_apply_f[2].h12>>1));
	dcc_d_dcc_ai_15_reg.ai_region_end_j_2 = min(2159,face_icm_apply_f[2].pos_y_s+(face_icm_apply_f[2].h12>>1));
	icm_d_icm_ai_10_reg.ai_region_ilu_2 = face_icm_apply_f[2].w12>>3;
	icm_d_icm_ai_10_reg.ai_region_ild_2 = face_icm_apply_f[2].w12>>3;
	icm_d_icm_ai_10_reg.ai_region_jlu_2 = face_icm_apply_f[2].h12>>3;
	icm_d_icm_ai_10_reg.ai_region_jld_2 = face_icm_apply_f[2].h12>>3;
	icm_d_icm_ai_11_reg.ai_region_iru_2 = face_icm_apply_f[2].w12>>3;
	icm_d_icm_ai_11_reg.ai_region_ird_2 = face_icm_apply_f[2].w12>>3;
	icm_d_icm_ai_11_reg.ai_region_jru_2 = face_icm_apply_f[2].h12>>3;
	icm_d_icm_ai_11_reg.ai_region_jrd_2 = face_icm_apply_f[2].h12>>3;

	icm_d_icm_skin_tone_segment_c2_0_reg.c_blending_2_ratio_0=face_icm_apply_f[2].uv_blending_0;
	icm_d_icm_skin_tone_segment_c2_0_reg.c_blending_2_ratio_1=face_icm_apply_f[2].uv_blending_1;
	icm_d_icm_skin_tone_segment_c2_0_reg.c_blending_2_ratio_2=face_icm_apply_f[2].uv_blending_2;
	icm_d_icm_skin_tone_segment_c2_0_reg.c_blending_2_ratio_3=face_icm_apply_f[2].uv_blending_3;
	icm_d_icm_skin_tone_segment_c2_1_reg.c_blending_2_ratio_4=face_icm_apply_f[2].uv_blending_4;
	icm_d_icm_skin_tone_segment_c2_1_reg.c_blending_2_ratio_5=face_icm_apply_f[2].uv_blending_5;
	icm_d_icm_skin_tone_segment_c2_1_reg.c_blending_2_ratio_6=face_icm_apply_f[2].uv_blending_6;
	icm_d_icm_skin_tone_segment_c2_1_reg.c_blending_2_ratio_7=face_icm_apply_f[2].uv_blending_7;

	//hue, sat adjust
	icm_d_icm_ai_offset_5_reg.ai_h_offset2=face_icm_apply_f[2].hue_adj_s;
	icm_d_icm_ai_offset_4_reg.ai_s_offset2=face_icm_apply_f[2].sat_adj_s;
	icm_d_icm_ai_offset_4_reg.ai_i_offset2=face_icm_apply_f[2].int_adj_s;


	//face3
	dcc_d_dcc_ai_4_reg.ai_center_u_3 = face_icm_apply_f[3].center_u_s;
	dcc_d_dcc_ai_4_reg.ai_center_v_3 = face_icm_apply_f[3].center_v_s;
	dcc_d_dcc_ai_10_reg.ai_region_start_i_3 = max(0,face_icm_apply_f[3].pos_x_s-(face_icm_apply_f[3].w12>>1));
	dcc_d_dcc_ai_10_reg.ai_region_end_i_3 = min(3839,face_icm_apply_f[3].pos_x_s+(face_icm_apply_f[3].w12>>1));
	dcc_d_dcc_ai_16_reg.ai_region_start_j_3 = max(0,face_icm_apply_f[3].pos_y_s-(face_icm_apply_f[3].h12>>1));
	dcc_d_dcc_ai_16_reg.ai_region_end_j_3 = min(2159,face_icm_apply_f[3].pos_y_s+(face_icm_apply_f[3].h12>>1));
	icm_d_icm_ai_13_reg.ai_region_ilu_3 = face_icm_apply_f[3].w12>>3;
	icm_d_icm_ai_13_reg.ai_region_ild_3 = face_icm_apply_f[3].w12>>3;
	icm_d_icm_ai_13_reg.ai_region_jlu_3 = face_icm_apply_f[3].h12>>3;
	icm_d_icm_ai_13_reg.ai_region_jld_3 = face_icm_apply_f[3].h12>>3;
	icm_d_icm_ai_14_reg.ai_region_iru_3 = face_icm_apply_f[3].w12>>3;
	icm_d_icm_ai_14_reg.ai_region_ird_3 = face_icm_apply_f[3].w12>>3;
	icm_d_icm_ai_14_reg.ai_region_jru_3 = face_icm_apply_f[3].h12>>3;
	icm_d_icm_ai_14_reg.ai_region_jrd_3 = face_icm_apply_f[3].h12>>3;

	icm_d_icm_skin_tone_segment_c3_0_reg.c_blending_3_ratio_0=face_icm_apply_f[3].uv_blending_0;
	icm_d_icm_skin_tone_segment_c3_0_reg.c_blending_3_ratio_1=face_icm_apply_f[3].uv_blending_1;
	icm_d_icm_skin_tone_segment_c3_0_reg.c_blending_3_ratio_2=face_icm_apply_f[3].uv_blending_2;
	icm_d_icm_skin_tone_segment_c3_0_reg.c_blending_3_ratio_3=face_icm_apply_f[3].uv_blending_3;
	icm_d_icm_skin_tone_segment_c3_1_reg.c_blending_3_ratio_4=face_icm_apply_f[3].uv_blending_4;
	icm_d_icm_skin_tone_segment_c3_1_reg.c_blending_3_ratio_5=face_icm_apply_f[3].uv_blending_5;
	icm_d_icm_skin_tone_segment_c3_1_reg.c_blending_3_ratio_6=face_icm_apply_f[3].uv_blending_6;
	icm_d_icm_skin_tone_segment_c3_1_reg.c_blending_3_ratio_7=face_icm_apply_f[3].uv_blending_7;

	//hue, sat adjust
	icm_d_icm_ai_offset_7_reg.ai_h_offset3=face_icm_apply_f[3].hue_adj_s;
	icm_d_icm_ai_offset_6_reg.ai_s_offset3=face_icm_apply_f[3].sat_adj_s;
	icm_d_icm_ai_offset_6_reg.ai_i_offset3=face_icm_apply_f[3].int_adj_s;


	//face4
	dcc_d_dcc_ai_5_reg.ai_center_u_4 = face_icm_apply_f[4].center_u_s;
	dcc_d_dcc_ai_5_reg.ai_center_v_4 = face_icm_apply_f[4].center_v_s;
	dcc_d_dcc_ai_11_reg.ai_region_start_i_4 = max(0,face_icm_apply_f[4].pos_x_s-(face_icm_apply_f[4].w12>>1));
	dcc_d_dcc_ai_11_reg.ai_region_end_i_4 = min(3839,face_icm_apply_f[4].pos_x_s+(face_icm_apply_f[4].w12>>1));
	dcc_d_dcc_ai_17_reg.ai_region_start_j_4 = max(0,face_icm_apply_f[4].pos_y_s-(face_icm_apply_f[4].h12>>1));
	dcc_d_dcc_ai_17_reg.ai_region_end_j_4 = min(2159,face_icm_apply_f[4].pos_y_s+(face_icm_apply_f[4].h12>>1));
	icm_d_icm_ai_16_reg.ai_region_ilu_4 = face_icm_apply_f[4].w12>>3;
	icm_d_icm_ai_16_reg.ai_region_ild_4 = face_icm_apply_f[4].w12>>3;
	icm_d_icm_ai_16_reg.ai_region_jlu_4 = face_icm_apply_f[4].h12>>3;
	icm_d_icm_ai_16_reg.ai_region_jld_4 = face_icm_apply_f[4].h12>>3;
	icm_d_icm_ai_17_reg.ai_region_iru_4 = face_icm_apply_f[4].w12>>3;
	icm_d_icm_ai_17_reg.ai_region_ird_4 = face_icm_apply_f[4].w12>>3;
	icm_d_icm_ai_17_reg.ai_region_jru_4 = face_icm_apply_f[4].h12>>3;
	icm_d_icm_ai_17_reg.ai_region_jrd_4 = face_icm_apply_f[4].h12>>3;

	icm_d_icm_skin_tone_segment_c4_0_reg.c_blending_4_ratio_0=face_icm_apply_f[4].uv_blending_0;
	icm_d_icm_skin_tone_segment_c4_0_reg.c_blending_4_ratio_1=face_icm_apply_f[4].uv_blending_1;
	icm_d_icm_skin_tone_segment_c4_0_reg.c_blending_4_ratio_2=face_icm_apply_f[4].uv_blending_2;
	icm_d_icm_skin_tone_segment_c4_0_reg.c_blending_4_ratio_3=face_icm_apply_f[4].uv_blending_3;
	icm_d_icm_skin_tone_segment_c4_1_reg.c_blending_4_ratio_4=face_icm_apply_f[4].uv_blending_4;
	icm_d_icm_skin_tone_segment_c4_1_reg.c_blending_4_ratio_5=face_icm_apply_f[4].uv_blending_5;
	icm_d_icm_skin_tone_segment_c4_1_reg.c_blending_4_ratio_6=face_icm_apply_f[4].uv_blending_6;
	icm_d_icm_skin_tone_segment_c4_1_reg.c_blending_4_ratio_7=face_icm_apply_f[4].uv_blending_7;

	//hue, sat adjust
	icm_d_icm_ai_offset_9_reg.ai_h_offset4=face_icm_apply_f[4].hue_adj_s;
	icm_d_icm_ai_offset_8_reg.ai_s_offset4=face_icm_apply_f[4].sat_adj_s;
	icm_d_icm_ai_offset_8_reg.ai_i_offset4=face_icm_apply_f[4].int_adj_s;


	//face5
	dcc_d_dcc_ai_6_reg.ai_center_u_5 = face_icm_apply_f[5].center_u_s;
	dcc_d_dcc_ai_6_reg.ai_center_v_5 = face_icm_apply_f[5].center_v_s;
	dcc_d_dcc_ai_12_reg.ai_region_start_i_5 = max(0,face_icm_apply_f[5].pos_x_s-(face_icm_apply_f[5].w12>>1));
	dcc_d_dcc_ai_12_reg.ai_region_end_i_5 = min(3839,face_icm_apply_f[5].pos_x_s+(face_icm_apply_f[5].w12>>1));
	dcc_d_dcc_ai_18_reg.ai_region_start_j_5 = max(0,face_icm_apply_f[5].pos_y_s-(face_icm_apply_f[5].h12>>1));
	dcc_d_dcc_ai_18_reg.ai_region_end_j_5 = min(2159,face_icm_apply_f[5].pos_y_s+(face_icm_apply_f[5].h12>>1));
	icm_d_icm_ai_19_reg.ai_region_ilu_5 = face_icm_apply_f[5].w12>>3;
	icm_d_icm_ai_19_reg.ai_region_ild_5 = face_icm_apply_f[5].w12>>3;
	icm_d_icm_ai_19_reg.ai_region_jlu_5 = face_icm_apply_f[5].h12>>3;
	icm_d_icm_ai_19_reg.ai_region_jld_5 = face_icm_apply_f[5].h12>>3;
	icm_d_icm_ai_20_reg.ai_region_iru_5 = face_icm_apply_f[5].w12>>3;
	icm_d_icm_ai_20_reg.ai_region_ird_5 = face_icm_apply_f[5].w12>>3;
	icm_d_icm_ai_20_reg.ai_region_jru_5 = face_icm_apply_f[5].h12>>3;
	icm_d_icm_ai_20_reg.ai_region_jrd_5 = face_icm_apply_f[5].h12>>3;
	icm_d_icm_skin_tone_segment_c5_0_reg.c_blending_5_ratio_0=face_icm_apply_f[5].uv_blending_0;
	icm_d_icm_skin_tone_segment_c5_0_reg.c_blending_5_ratio_1=face_icm_apply_f[5].uv_blending_1;
	icm_d_icm_skin_tone_segment_c5_0_reg.c_blending_5_ratio_2=face_icm_apply_f[5].uv_blending_2;
	icm_d_icm_skin_tone_segment_c5_0_reg.c_blending_5_ratio_3=face_icm_apply_f[5].uv_blending_3;
	icm_d_icm_skin_tone_segment_c5_1_reg.c_blending_5_ratio_4=face_icm_apply_f[5].uv_blending_4;
	icm_d_icm_skin_tone_segment_c5_1_reg.c_blending_5_ratio_5=face_icm_apply_f[5].uv_blending_5;
	icm_d_icm_skin_tone_segment_c5_1_reg.c_blending_5_ratio_6=face_icm_apply_f[5].uv_blending_6;
	icm_d_icm_skin_tone_segment_c5_1_reg.c_blending_5_ratio_7=face_icm_apply_f[5].uv_blending_7;

	//hue, sat adjust
	icm_d_icm_ai_offset_11_reg.ai_h_offset5=face_icm_apply_f[5].hue_adj_s;
	icm_d_icm_ai_offset_10_reg.ai_s_offset5=face_icm_apply_f[5].sat_adj_s;
	icm_d_icm_ai_offset_10_reg.ai_i_offset5=face_icm_apply_f[5].int_adj_s;

	IoReg_Write32(COLOR_ICM_D_ICM_AI_0_reg, icm_d_icm_ai_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_1_reg, icm_d_icm_ai_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_2_reg, icm_d_icm_ai_2_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_3_reg, icm_d_icm_ai_3_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_4_reg, icm_d_icm_ai_4_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_5_reg, icm_d_icm_ai_5_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_7_reg, icm_d_icm_ai_7_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_8_reg,icm_d_icm_ai_8_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_10_reg,icm_d_icm_ai_10_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_11_reg,icm_d_icm_ai_11_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_13_reg,icm_d_icm_ai_13_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_14_reg,icm_d_icm_ai_14_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_16_reg,icm_d_icm_ai_16_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_17_reg,icm_d_icm_ai_17_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_19_reg,icm_d_icm_ai_19_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_20_reg,icm_d_icm_ai_20_reg.regValue);

	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_0_reg, icm_d_icm_ai_offset_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_1_reg, icm_d_icm_ai_offset_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_2_reg, icm_d_icm_ai_offset_2_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_3_reg, icm_d_icm_ai_offset_3_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_4_reg, icm_d_icm_ai_offset_4_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_5_reg, icm_d_icm_ai_offset_5_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_6_reg, icm_d_icm_ai_offset_6_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_7_reg, icm_d_icm_ai_offset_7_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_8_reg, icm_d_icm_ai_offset_8_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_9_reg, icm_d_icm_ai_offset_9_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_10_reg, icm_d_icm_ai_offset_10_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_11_reg, icm_d_icm_ai_offset_11_reg.regValue);

	IoReg_Write32(COLOR_DCC_D_DCC_AI_1_reg, dcc_d_dcc_ai_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_2_reg, dcc_d_dcc_ai_2_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_3_reg, dcc_d_dcc_ai_3_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_4_reg, dcc_d_dcc_ai_4_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_5_reg, dcc_d_dcc_ai_5_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_6_reg, dcc_d_dcc_ai_6_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_7_reg, dcc_d_dcc_ai_7_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_8_reg, dcc_d_dcc_ai_8_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_9_reg, dcc_d_dcc_ai_9_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_10_reg, dcc_d_dcc_ai_10_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_11_reg, dcc_d_dcc_ai_11_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_12_reg, dcc_d_dcc_ai_12_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_13_reg, dcc_d_dcc_ai_13_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_14_reg, dcc_d_dcc_ai_14_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_15_reg, dcc_d_dcc_ai_15_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_16_reg, dcc_d_dcc_ai_16_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_17_reg, dcc_d_dcc_ai_17_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_18_reg, dcc_d_dcc_ai_18_reg.regValue);

	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C0_0_reg, icm_d_icm_skin_tone_segment_c0_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C0_1_reg, icm_d_icm_skin_tone_segment_c0_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C1_0_reg, icm_d_icm_skin_tone_segment_c1_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C1_1_reg, icm_d_icm_skin_tone_segment_c1_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C2_0_reg, icm_d_icm_skin_tone_segment_c2_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C2_1_reg, icm_d_icm_skin_tone_segment_c2_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C3_0_reg, icm_d_icm_skin_tone_segment_c3_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C3_1_reg, icm_d_icm_skin_tone_segment_c3_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C4_0_reg, icm_d_icm_skin_tone_segment_c4_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C4_1_reg, icm_d_icm_skin_tone_segment_c4_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C5_0_reg, icm_d_icm_skin_tone_segment_c5_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C5_1_reg, icm_d_icm_skin_tone_segment_c5_1_reg.regValue);

}

void drvif_color_AI_obj_dcc_set(AI_DCC_apply face_dcc_apply_f[6])
{
	// chen 0429
	color_dcc_d_dcc_ai_0_RBUS	dcc_d_dcc_ai_0_reg;
	color_dcc_d_dcc_ai_1_RBUS	dcc_d_dcc_ai_1_reg;
	color_dcc_d_dcc_ai_2_RBUS	dcc_d_dcc_ai_2_reg;
	color_dcc_d_dcc_ai_3_RBUS	dcc_d_dcc_ai_3_reg;
	color_dcc_d_dcc_ai_4_RBUS	dcc_d_dcc_ai_4_reg;
	color_dcc_d_dcc_ai_5_RBUS	dcc_d_dcc_ai_5_reg;
	color_dcc_d_dcc_ai_6_RBUS	dcc_d_dcc_ai_6_reg;


	// icm share with dcc
	color_dcc_d_dcc_ai_7_RBUS    dcc_d_dcc_ai_7_reg;
	color_dcc_d_dcc_ai_8_RBUS	 dcc_d_dcc_ai_8_reg;
	color_dcc_d_dcc_ai_9_RBUS	 dcc_d_dcc_ai_9_reg;
	color_dcc_d_dcc_ai_10_RBUS   dcc_d_dcc_ai_10_reg;
	color_dcc_d_dcc_ai_11_RBUS	 dcc_d_dcc_ai_11_reg;
	color_dcc_d_dcc_ai_12_RBUS	 dcc_d_dcc_ai_12_reg;
	color_dcc_d_dcc_ai_13_RBUS   dcc_d_dcc_ai_13_reg;
	color_dcc_d_dcc_ai_14_RBUS	 dcc_d_dcc_ai_14_reg;
	color_dcc_d_dcc_ai_15_RBUS	 dcc_d_dcc_ai_15_reg;
	color_dcc_d_dcc_ai_16_RBUS   dcc_d_dcc_ai_16_reg;
	color_dcc_d_dcc_ai_17_RBUS	 dcc_d_dcc_ai_17_reg;
	color_dcc_d_dcc_ai_18_RBUS	 dcc_d_dcc_ai_18_reg;
	color_dcc_d_dcc_ai_19_RBUS   dcc_d_dcc_ai_19_reg;
	color_dcc_d_dcc_ai_20_RBUS	 dcc_d_dcc_ai_20_reg;
	color_dcc_d_dcc_ai_21_RBUS	 dcc_d_dcc_ai_21_reg;
	color_dcc_d_dcc_ai_22_RBUS   dcc_d_dcc_ai_22_reg;
	color_dcc_d_dcc_ai_23_RBUS	 dcc_d_dcc_ai_23_reg;
	color_dcc_d_dcc_ai_24_RBUS	 dcc_d_dcc_ai_24_reg;
	color_dcc_d_dcc_ai_25_RBUS   dcc_d_dcc_ai_25_reg;
	color_dcc_d_dcc_ai_26_RBUS	 dcc_d_dcc_ai_26_reg;
	color_dcc_d_dcc_ai_27_RBUS	 dcc_d_dcc_ai_27_reg;
	color_dcc_d_dcc_ai_28_RBUS   dcc_d_dcc_ai_28_reg;
	color_dcc_d_dcc_ai_29_RBUS	 dcc_d_dcc_ai_29_reg;
	color_dcc_d_dcc_ai_30_RBUS	 dcc_d_dcc_ai_30_reg;
	color_dcc_d_dcc_ai_31_RBUS   dcc_d_dcc_ai_31_reg;
	color_dcc_d_dcc_ai_32_RBUS	 dcc_d_dcc_ai_32_reg;
	color_dcc_d_dcc_ai_33_RBUS	 dcc_d_dcc_ai_33_reg;
	color_dcc_d_dcc_ai_34_RBUS   dcc_d_dcc_ai_34_reg;
	color_dcc_d_dcc_ai_35_RBUS	 dcc_d_dcc_ai_35_reg;
	color_dcc_d_dcc_ai_36_RBUS	 dcc_d_dcc_ai_36_reg;	

	// chen 0524
	color_dcc_d_dcc_skin_tone_segment_c0_0_RBUS dcc_d_dcc_skin_tone_segment_c0_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c0_1_RBUS dcc_d_dcc_skin_tone_segment_c0_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c1_0_RBUS dcc_d_dcc_skin_tone_segment_c1_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c1_1_RBUS dcc_d_dcc_skin_tone_segment_c1_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c2_0_RBUS dcc_d_dcc_skin_tone_segment_c2_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c2_1_RBUS dcc_d_dcc_skin_tone_segment_c2_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c3_0_RBUS dcc_d_dcc_skin_tone_segment_c3_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c3_1_RBUS dcc_d_dcc_skin_tone_segment_c3_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c4_0_RBUS dcc_d_dcc_skin_tone_segment_c4_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c4_1_RBUS dcc_d_dcc_skin_tone_segment_c4_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c5_0_RBUS dcc_d_dcc_skin_tone_segment_c5_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c5_1_RBUS dcc_d_dcc_skin_tone_segment_c5_1_reg;

	//end chen 0524

	// lesley 0910
	color_dcc_d_dcc_skin_tone_en_RBUS d_dcc_skin_tone_en_reg;
	// end lesley 0910

	dcc_d_dcc_ai_0_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_0_reg);
	dcc_d_dcc_ai_1_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_1_reg);
	dcc_d_dcc_ai_2_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_2_reg);
	dcc_d_dcc_ai_3_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_3_reg);
	dcc_d_dcc_ai_4_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_4_reg);
	dcc_d_dcc_ai_5_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_5_reg);
	dcc_d_dcc_ai_6_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_6_reg);
	dcc_d_dcc_ai_7_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_7_reg);
	dcc_d_dcc_ai_8_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_8_reg);
	dcc_d_dcc_ai_9_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_9_reg);
	dcc_d_dcc_ai_10_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_10_reg);
	dcc_d_dcc_ai_11_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_11_reg);
	dcc_d_dcc_ai_12_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_12_reg);
	dcc_d_dcc_ai_13_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_13_reg);
	dcc_d_dcc_ai_14_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_14_reg);
	dcc_d_dcc_ai_15_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_15_reg);
	dcc_d_dcc_ai_16_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_16_reg);
	dcc_d_dcc_ai_17_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_17_reg);
	dcc_d_dcc_ai_18_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_18_reg);
	dcc_d_dcc_ai_19_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_19_reg);
	dcc_d_dcc_ai_20_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_20_reg);
	dcc_d_dcc_ai_21_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_21_reg);
	dcc_d_dcc_ai_22_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_22_reg);
	dcc_d_dcc_ai_23_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_23_reg);
	dcc_d_dcc_ai_24_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_24_reg);
	dcc_d_dcc_ai_25_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_25_reg);
	dcc_d_dcc_ai_26_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_26_reg);
	dcc_d_dcc_ai_27_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_27_reg);
	dcc_d_dcc_ai_28_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_28_reg);
	dcc_d_dcc_ai_29_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_29_reg);
	dcc_d_dcc_ai_30_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_30_reg);
	dcc_d_dcc_ai_31_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_31_reg);
	dcc_d_dcc_ai_32_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_32_reg);
	dcc_d_dcc_ai_33_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_33_reg);
	dcc_d_dcc_ai_34_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_34_reg);
	dcc_d_dcc_ai_35_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_35_reg);
	dcc_d_dcc_ai_36_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_36_reg);

	// chen 0524
	dcc_d_dcc_skin_tone_segment_c0_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_0_reg);
	dcc_d_dcc_skin_tone_segment_c0_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_1_reg);
	dcc_d_dcc_skin_tone_segment_c1_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_0_reg);
	dcc_d_dcc_skin_tone_segment_c1_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_1_reg);
	dcc_d_dcc_skin_tone_segment_c2_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_0_reg);
	dcc_d_dcc_skin_tone_segment_c2_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_1_reg);
	dcc_d_dcc_skin_tone_segment_c3_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_0_reg);
	dcc_d_dcc_skin_tone_segment_c3_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_1_reg);
	dcc_d_dcc_skin_tone_segment_c4_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C4_0_reg);
	dcc_d_dcc_skin_tone_segment_c4_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C4_1_reg);
	dcc_d_dcc_skin_tone_segment_c5_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C5_0_reg);
	dcc_d_dcc_skin_tone_segment_c5_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C5_1_reg);
	//end chen 0524

	// lesley 0910
	d_dcc_skin_tone_en_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg);
	// end lesley 0910

	//face0
	// win_pos
	dcc_d_dcc_ai_1_reg.ai_center_u_0 = face_dcc_apply_f[0].center_u_s;
	dcc_d_dcc_ai_1_reg.ai_center_v_0 = face_dcc_apply_f[0].center_v_s;
	dcc_d_dcc_ai_7_reg.ai_region_start_i_0 = max(0,face_dcc_apply_f[0].pos_x_s-(face_dcc_apply_f[0].w12>>1));
	dcc_d_dcc_ai_7_reg.ai_region_end_i_0 = min(3839,face_dcc_apply_f[0].pos_x_s+(face_dcc_apply_f[0].w12>>1));
	dcc_d_dcc_ai_13_reg.ai_region_start_j_0 = max(0,face_dcc_apply_f[0].pos_y_s-(face_dcc_apply_f[0].h12>>1));
	dcc_d_dcc_ai_13_reg.ai_region_end_j_0 = min(2159,face_dcc_apply_f[0].pos_y_s+(face_dcc_apply_f[0].h12>>1));
	dcc_d_dcc_ai_19_reg.ai_region_ilu_0 = face_dcc_apply_f[0].w12>>3;
	dcc_d_dcc_ai_19_reg.ai_region_ild_0 = face_dcc_apply_f[0].w12>>3;
	dcc_d_dcc_ai_19_reg.ai_region_jlu_0 = face_dcc_apply_f[0].h12>>3;
	dcc_d_dcc_ai_19_reg.ai_region_jld_0 = face_dcc_apply_f[0].h12>>3;
	dcc_d_dcc_ai_20_reg.ai_region_iru_0 = face_dcc_apply_f[0].w12>>3;
	dcc_d_dcc_ai_20_reg.ai_region_ird_0 = face_dcc_apply_f[0].w12>>3;
	dcc_d_dcc_ai_20_reg.ai_region_jru_0 = face_dcc_apply_f[0].h12>>3;
	dcc_d_dcc_ai_20_reg.ai_region_jrd_0 = face_dcc_apply_f[0].h12>>3;
	dcc_d_dcc_ai_21_reg.ai_region_ilum_0= 0;
	dcc_d_dcc_ai_21_reg.ai_region_ildm_0= 0;
	dcc_d_dcc_ai_21_reg.ai_region_jlum_0= 0;
	dcc_d_dcc_ai_21_reg.ai_region_jldm_0= 0;
	dcc_d_dcc_ai_21_reg.ai_region_irum_0= 0;
	dcc_d_dcc_ai_21_reg.ai_region_irdm_0= 0;
	dcc_d_dcc_ai_21_reg.ai_region_jrum_0= 0;
	dcc_d_dcc_ai_21_reg.ai_region_jrdm_0= 0;

	// blending ratio

// chen 0524
	dcc_d_dcc_skin_tone_segment_c0_0_reg.c_blending_0_ratio_0=face_dcc_apply_f[0].uv_blending_0;
	dcc_d_dcc_skin_tone_segment_c0_0_reg.c_blending_0_ratio_1=face_dcc_apply_f[0].uv_blending_1;
	dcc_d_dcc_skin_tone_segment_c0_0_reg.c_blending_0_ratio_2=face_dcc_apply_f[0].uv_blending_2;
	dcc_d_dcc_skin_tone_segment_c0_0_reg.c_blending_0_ratio_3=face_dcc_apply_f[0].uv_blending_3;
	dcc_d_dcc_skin_tone_segment_c0_1_reg.c_blending_0_ratio_4=face_dcc_apply_f[0].uv_blending_4;
	dcc_d_dcc_skin_tone_segment_c0_1_reg.c_blending_0_ratio_5=face_dcc_apply_f[0].uv_blending_5;
	dcc_d_dcc_skin_tone_segment_c0_1_reg.c_blending_0_ratio_6=face_dcc_apply_f[0].uv_blending_6;
	dcc_d_dcc_skin_tone_segment_c0_1_reg.c_blending_0_ratio_7=face_dcc_apply_f[0].uv_blending_7;
//end chen 0524


	//face1
	dcc_d_dcc_ai_2_reg.ai_center_u_1 = face_dcc_apply_f[1].center_u_s;
	dcc_d_dcc_ai_2_reg.ai_center_v_1 = face_dcc_apply_f[1].center_v_s;
	dcc_d_dcc_ai_8_reg.ai_region_start_i_1 = max(0,face_dcc_apply_f[1].pos_x_s-(face_dcc_apply_f[1].w12>>1));
	dcc_d_dcc_ai_8_reg.ai_region_end_i_1 = min(3839,face_dcc_apply_f[1].pos_x_s+(face_dcc_apply_f[1].w12>>1));
	dcc_d_dcc_ai_14_reg.ai_region_start_j_1 = max(0,face_dcc_apply_f[1].pos_y_s-(face_dcc_apply_f[1].h12>>1));
	dcc_d_dcc_ai_14_reg.ai_region_end_j_1 = min(2159,face_dcc_apply_f[1].pos_y_s+(face_dcc_apply_f[1].h12>>1));
	dcc_d_dcc_ai_22_reg.ai_region_ilu_1 = face_dcc_apply_f[1].w12>>3;
	dcc_d_dcc_ai_22_reg.ai_region_ild_1 = face_dcc_apply_f[1].w12>>3;
	dcc_d_dcc_ai_22_reg.ai_region_jlu_1 = face_dcc_apply_f[1].h12>>3;
	dcc_d_dcc_ai_22_reg.ai_region_jld_1 = face_dcc_apply_f[1].h12>>3;
	dcc_d_dcc_ai_23_reg.ai_region_iru_1 = face_dcc_apply_f[1].w12>>3;
	dcc_d_dcc_ai_23_reg.ai_region_ird_1 = face_dcc_apply_f[1].w12>>3;
	dcc_d_dcc_ai_23_reg.ai_region_jru_1 = face_dcc_apply_f[1].h12>>3;
	dcc_d_dcc_ai_23_reg.ai_region_jrd_1 = face_dcc_apply_f[1].h12>>3;
	dcc_d_dcc_ai_24_reg.ai_region_ilum_1= 0;
	dcc_d_dcc_ai_24_reg.ai_region_ildm_1= 0;
	dcc_d_dcc_ai_24_reg.ai_region_jlum_1= 0;
	dcc_d_dcc_ai_24_reg.ai_region_jldm_1= 0;
	dcc_d_dcc_ai_24_reg.ai_region_irum_1= 0;
	dcc_d_dcc_ai_24_reg.ai_region_irdm_1= 0;
	dcc_d_dcc_ai_24_reg.ai_region_jrum_1= 0;
	dcc_d_dcc_ai_24_reg.ai_region_jrdm_1= 0;

// chen 0524
	dcc_d_dcc_skin_tone_segment_c1_0_reg.c_blending_1_ratio_0=face_dcc_apply_f[1].uv_blending_0;
	dcc_d_dcc_skin_tone_segment_c1_0_reg.c_blending_1_ratio_1=face_dcc_apply_f[1].uv_blending_1;
	dcc_d_dcc_skin_tone_segment_c1_0_reg.c_blending_1_ratio_2=face_dcc_apply_f[1].uv_blending_2;
	dcc_d_dcc_skin_tone_segment_c1_0_reg.c_blending_1_ratio_3=face_dcc_apply_f[1].uv_blending_3;
	dcc_d_dcc_skin_tone_segment_c1_1_reg.c_blending_1_ratio_4=face_dcc_apply_f[1].uv_blending_4;
	dcc_d_dcc_skin_tone_segment_c1_1_reg.c_blending_1_ratio_5=face_dcc_apply_f[1].uv_blending_5;
	dcc_d_dcc_skin_tone_segment_c1_1_reg.c_blending_1_ratio_6=face_dcc_apply_f[1].uv_blending_6;
	dcc_d_dcc_skin_tone_segment_c1_1_reg.c_blending_1_ratio_7=face_dcc_apply_f[1].uv_blending_7;
//end chen 0524


	//face2
	dcc_d_dcc_ai_3_reg.ai_center_u_2 = face_dcc_apply_f[2].center_u_s;
	dcc_d_dcc_ai_3_reg.ai_center_v_2 = face_dcc_apply_f[2].center_v_s;
	dcc_d_dcc_ai_9_reg.ai_region_start_i_2 = max(0,face_dcc_apply_f[2].pos_x_s-(face_dcc_apply_f[2].w12>>1));
	dcc_d_dcc_ai_9_reg.ai_region_end_i_2 = min(3839,face_dcc_apply_f[2].pos_x_s+(face_dcc_apply_f[2].w12>>1));
	dcc_d_dcc_ai_15_reg.ai_region_start_j_2 = max(0,face_dcc_apply_f[2].pos_y_s-(face_dcc_apply_f[2].h12>>1));
	dcc_d_dcc_ai_15_reg.ai_region_end_j_2 = min(2159,face_dcc_apply_f[2].pos_y_s+(face_dcc_apply_f[2].h12>>1));
	dcc_d_dcc_ai_25_reg.ai_region_ilu_2 = face_dcc_apply_f[2].w12>>3;
	dcc_d_dcc_ai_25_reg.ai_region_ild_2 = face_dcc_apply_f[2].w12>>3;
	dcc_d_dcc_ai_25_reg.ai_region_jlu_2 = face_dcc_apply_f[2].h12>>3;
	dcc_d_dcc_ai_25_reg.ai_region_jld_2 = face_dcc_apply_f[2].h12>>3;
	dcc_d_dcc_ai_26_reg.ai_region_iru_2 = face_dcc_apply_f[2].w12>>3;
	dcc_d_dcc_ai_26_reg.ai_region_ird_2 = face_dcc_apply_f[2].w12>>3;
	dcc_d_dcc_ai_26_reg.ai_region_jru_2 = face_dcc_apply_f[2].h12>>3;
	dcc_d_dcc_ai_26_reg.ai_region_jrd_2 = face_dcc_apply_f[2].h12>>3;
	dcc_d_dcc_ai_27_reg.ai_region_ilum_2= 0;
	dcc_d_dcc_ai_27_reg.ai_region_ildm_2= 0;
	dcc_d_dcc_ai_27_reg.ai_region_jlum_2= 0;
	dcc_d_dcc_ai_27_reg.ai_region_jldm_2= 0;
	dcc_d_dcc_ai_27_reg.ai_region_irum_2= 0;
	dcc_d_dcc_ai_27_reg.ai_region_irdm_2= 0;
	dcc_d_dcc_ai_27_reg.ai_region_jrum_2= 0;
	dcc_d_dcc_ai_27_reg.ai_region_jrdm_2= 0;

// chen 0524
	dcc_d_dcc_skin_tone_segment_c2_0_reg.c_blending_2_ratio_0=face_dcc_apply_f[2].uv_blending_0;
	dcc_d_dcc_skin_tone_segment_c2_0_reg.c_blending_2_ratio_1=face_dcc_apply_f[2].uv_blending_1;
	dcc_d_dcc_skin_tone_segment_c2_0_reg.c_blending_2_ratio_2=face_dcc_apply_f[2].uv_blending_2;
	dcc_d_dcc_skin_tone_segment_c2_0_reg.c_blending_2_ratio_3=face_dcc_apply_f[2].uv_blending_3;
	dcc_d_dcc_skin_tone_segment_c2_1_reg.c_blending_2_ratio_4=face_dcc_apply_f[2].uv_blending_4;
	dcc_d_dcc_skin_tone_segment_c2_1_reg.c_blending_2_ratio_5=face_dcc_apply_f[2].uv_blending_5;
	dcc_d_dcc_skin_tone_segment_c2_1_reg.c_blending_2_ratio_6=face_dcc_apply_f[2].uv_blending_6;
	dcc_d_dcc_skin_tone_segment_c2_1_reg.c_blending_2_ratio_7=face_dcc_apply_f[2].uv_blending_7;
//end chen 0524


	//face3
	dcc_d_dcc_ai_4_reg.ai_center_u_3 = face_dcc_apply_f[3].center_u_s;
	dcc_d_dcc_ai_4_reg.ai_center_v_3 = face_dcc_apply_f[3].center_v_s;
	dcc_d_dcc_ai_10_reg.ai_region_start_i_3 = max(0,face_dcc_apply_f[3].pos_x_s-(face_dcc_apply_f[3].w12>>1));
	dcc_d_dcc_ai_10_reg.ai_region_end_i_3 = min(3839,face_dcc_apply_f[3].pos_x_s+(face_dcc_apply_f[3].w12>>1));
	dcc_d_dcc_ai_16_reg.ai_region_start_j_3 = max(0,face_dcc_apply_f[3].pos_y_s-(face_dcc_apply_f[3].h12>>1));
	dcc_d_dcc_ai_16_reg.ai_region_end_j_3 = min(2159,face_dcc_apply_f[3].pos_y_s+(face_dcc_apply_f[3].h12>>1));
	dcc_d_dcc_ai_28_reg.ai_region_ilu_3 = face_dcc_apply_f[3].w12>>3;
	dcc_d_dcc_ai_28_reg.ai_region_ild_3 = face_dcc_apply_f[3].w12>>3;
	dcc_d_dcc_ai_28_reg.ai_region_jlu_3 = face_dcc_apply_f[3].h12>>3;
	dcc_d_dcc_ai_28_reg.ai_region_jld_3 = face_dcc_apply_f[3].h12>>3;
	dcc_d_dcc_ai_29_reg.ai_region_iru_3 = face_dcc_apply_f[3].w12>>3;
	dcc_d_dcc_ai_29_reg.ai_region_ird_3 = face_dcc_apply_f[3].w12>>3;
	dcc_d_dcc_ai_29_reg.ai_region_jru_3 = face_dcc_apply_f[3].h12>>3;
	dcc_d_dcc_ai_29_reg.ai_region_jrd_3 = face_dcc_apply_f[3].h12>>3;
	dcc_d_dcc_ai_30_reg.ai_region_ilum_3= 0;
	dcc_d_dcc_ai_30_reg.ai_region_ildm_3= 0;
	dcc_d_dcc_ai_30_reg.ai_region_jlum_3= 0;
	dcc_d_dcc_ai_30_reg.ai_region_jldm_3= 0;
	dcc_d_dcc_ai_30_reg.ai_region_irum_3= 0;
	dcc_d_dcc_ai_30_reg.ai_region_irdm_3= 0;
	dcc_d_dcc_ai_30_reg.ai_region_jrum_3= 0;
	dcc_d_dcc_ai_30_reg.ai_region_jrdm_3= 0;

// chen 0524
	dcc_d_dcc_skin_tone_segment_c3_0_reg.c_blending_3_ratio_0=face_dcc_apply_f[3].uv_blending_0;
	dcc_d_dcc_skin_tone_segment_c3_0_reg.c_blending_3_ratio_1=face_dcc_apply_f[3].uv_blending_1;
	dcc_d_dcc_skin_tone_segment_c3_0_reg.c_blending_3_ratio_2=face_dcc_apply_f[3].uv_blending_2;
	dcc_d_dcc_skin_tone_segment_c3_0_reg.c_blending_3_ratio_3=face_dcc_apply_f[3].uv_blending_3;
	dcc_d_dcc_skin_tone_segment_c3_1_reg.c_blending_3_ratio_4=face_dcc_apply_f[3].uv_blending_4;
	dcc_d_dcc_skin_tone_segment_c3_1_reg.c_blending_3_ratio_5=face_dcc_apply_f[3].uv_blending_5;
	dcc_d_dcc_skin_tone_segment_c3_1_reg.c_blending_3_ratio_6=face_dcc_apply_f[3].uv_blending_6;
	dcc_d_dcc_skin_tone_segment_c3_1_reg.c_blending_3_ratio_7=face_dcc_apply_f[3].uv_blending_7;
//end chen 0524


	//face4
	dcc_d_dcc_ai_5_reg.ai_center_u_4 = face_dcc_apply_f[4].center_u_s;
	dcc_d_dcc_ai_5_reg.ai_center_v_4 = face_dcc_apply_f[4].center_v_s;
	dcc_d_dcc_ai_11_reg.ai_region_start_i_4 = max(0,face_dcc_apply_f[4].pos_x_s-(face_dcc_apply_f[4].w12>>1));
	dcc_d_dcc_ai_11_reg.ai_region_end_i_4 = min(3839,face_dcc_apply_f[4].pos_x_s+(face_dcc_apply_f[4].w12>>1));
	dcc_d_dcc_ai_17_reg.ai_region_start_j_4 = max(0,face_dcc_apply_f[4].pos_y_s-(face_dcc_apply_f[4].h12>>1));
	dcc_d_dcc_ai_17_reg.ai_region_end_j_4 = min(2159,face_dcc_apply_f[4].pos_y_s+(face_dcc_apply_f[4].h12>>1));
	dcc_d_dcc_ai_31_reg.ai_region_ilu_4 = face_dcc_apply_f[4].w12>>3;
	dcc_d_dcc_ai_31_reg.ai_region_ild_4 = face_dcc_apply_f[4].w12>>3;
	dcc_d_dcc_ai_31_reg.ai_region_jlu_4 = face_dcc_apply_f[4].h12>>3;
	dcc_d_dcc_ai_31_reg.ai_region_jld_4 = face_dcc_apply_f[4].h12>>3;
	dcc_d_dcc_ai_32_reg.ai_region_iru_4 = face_dcc_apply_f[4].w12>>3;
	dcc_d_dcc_ai_32_reg.ai_region_ird_4 = face_dcc_apply_f[4].w12>>3;
	dcc_d_dcc_ai_32_reg.ai_region_jru_4 = face_dcc_apply_f[4].h12>>3;
	dcc_d_dcc_ai_32_reg.ai_region_jrd_4 = face_dcc_apply_f[4].h12>>3;
	dcc_d_dcc_ai_33_reg.ai_region_ilum_4= 0;
	dcc_d_dcc_ai_33_reg.ai_region_ildm_4= 0;
	dcc_d_dcc_ai_33_reg.ai_region_jlum_4= 0;
	dcc_d_dcc_ai_33_reg.ai_region_jldm_4= 0;
	dcc_d_dcc_ai_33_reg.ai_region_irum_4= 0;
	dcc_d_dcc_ai_33_reg.ai_region_irdm_4= 0;
	dcc_d_dcc_ai_33_reg.ai_region_jrum_4= 0;
	dcc_d_dcc_ai_33_reg.ai_region_jrdm_4= 0;

// chen 0524
	dcc_d_dcc_skin_tone_segment_c4_0_reg.c_blending_4_ratio_0=face_dcc_apply_f[4].uv_blending_0;
	dcc_d_dcc_skin_tone_segment_c4_0_reg.c_blending_4_ratio_1=face_dcc_apply_f[4].uv_blending_1;
	dcc_d_dcc_skin_tone_segment_c4_0_reg.c_blending_4_ratio_2=face_dcc_apply_f[4].uv_blending_2;
	dcc_d_dcc_skin_tone_segment_c4_0_reg.c_blending_4_ratio_3=face_dcc_apply_f[4].uv_blending_3;
	dcc_d_dcc_skin_tone_segment_c4_1_reg.c_blending_4_ratio_4=face_dcc_apply_f[4].uv_blending_4;
	dcc_d_dcc_skin_tone_segment_c4_1_reg.c_blending_4_ratio_5=face_dcc_apply_f[4].uv_blending_5;
	dcc_d_dcc_skin_tone_segment_c4_1_reg.c_blending_4_ratio_6=face_dcc_apply_f[4].uv_blending_6;
	dcc_d_dcc_skin_tone_segment_c4_1_reg.c_blending_4_ratio_7=face_dcc_apply_f[4].uv_blending_7;
//end chen 0524



	//face5
	dcc_d_dcc_ai_6_reg.ai_center_u_5 = face_dcc_apply_f[5].center_u_s;
	dcc_d_dcc_ai_6_reg.ai_center_v_5 = face_dcc_apply_f[5].center_v_s;
	dcc_d_dcc_ai_12_reg.ai_region_start_i_5 = max(0,face_dcc_apply_f[5].pos_x_s-(face_dcc_apply_f[5].w12>>1));
	dcc_d_dcc_ai_12_reg.ai_region_end_i_5 = min(3839,face_dcc_apply_f[5].pos_x_s+(face_dcc_apply_f[5].w12>>1));
	dcc_d_dcc_ai_18_reg.ai_region_start_j_5 = max(0,face_dcc_apply_f[5].pos_y_s-(face_dcc_apply_f[5].h12>>1));
	dcc_d_dcc_ai_18_reg.ai_region_end_j_5 = min(2159,face_dcc_apply_f[5].pos_y_s+(face_dcc_apply_f[5].h12>>1));
	dcc_d_dcc_ai_34_reg.ai_region_ilu_5 = face_dcc_apply_f[5].w12>>3;
	dcc_d_dcc_ai_34_reg.ai_region_ild_5 = face_dcc_apply_f[5].w12>>3;
	dcc_d_dcc_ai_34_reg.ai_region_jlu_5 = face_dcc_apply_f[5].h12>>3;
	dcc_d_dcc_ai_34_reg.ai_region_jld_5 = face_dcc_apply_f[5].h12>>3;
	dcc_d_dcc_ai_35_reg.ai_region_iru_5 = face_dcc_apply_f[5].w12>>3;
	dcc_d_dcc_ai_35_reg.ai_region_ird_5 = face_dcc_apply_f[5].w12>>3;
	dcc_d_dcc_ai_35_reg.ai_region_jru_5 = face_dcc_apply_f[5].h12>>3;
	dcc_d_dcc_ai_35_reg.ai_region_jrd_5 = face_dcc_apply_f[5].h12>>3;
	dcc_d_dcc_ai_36_reg.ai_region_ilum_5= 0;
	dcc_d_dcc_ai_36_reg.ai_region_ildm_5= 0;
	dcc_d_dcc_ai_36_reg.ai_region_jlum_5= 0;
	dcc_d_dcc_ai_36_reg.ai_region_jldm_5= 0;
	dcc_d_dcc_ai_36_reg.ai_region_irum_5= 0;
	dcc_d_dcc_ai_36_reg.ai_region_irdm_5= 0;
	dcc_d_dcc_ai_36_reg.ai_region_jrum_5= 0;
	dcc_d_dcc_ai_36_reg.ai_region_jrdm_5= 0;

// chen 0524
	dcc_d_dcc_skin_tone_segment_c5_0_reg.c_blending_5_ratio_0=face_dcc_apply_f[5].uv_blending_0;
	dcc_d_dcc_skin_tone_segment_c5_0_reg.c_blending_5_ratio_1=face_dcc_apply_f[5].uv_blending_1;
	dcc_d_dcc_skin_tone_segment_c5_0_reg.c_blending_5_ratio_2=face_dcc_apply_f[5].uv_blending_2;
	dcc_d_dcc_skin_tone_segment_c5_0_reg.c_blending_5_ratio_3=face_dcc_apply_f[5].uv_blending_3;
	dcc_d_dcc_skin_tone_segment_c5_1_reg.c_blending_5_ratio_4=face_dcc_apply_f[5].uv_blending_4;
	dcc_d_dcc_skin_tone_segment_c5_1_reg.c_blending_5_ratio_5=face_dcc_apply_f[5].uv_blending_5;
	dcc_d_dcc_skin_tone_segment_c5_1_reg.c_blending_5_ratio_6=face_dcc_apply_f[5].uv_blending_6;
	dcc_d_dcc_skin_tone_segment_c5_1_reg.c_blending_5_ratio_7=face_dcc_apply_f[5].uv_blending_7;
//end chen 0524



//	dcc_d_dcc_ai_6_reg.ai_detect_value=255;

	// lesley 0910
		d_dcc_skin_tone_en_reg.y_blending_0_enhance_en = face_dcc_apply_f[0].enhance_en;
		d_dcc_skin_tone_en_reg.y_blending_1_enhance_en = face_dcc_apply_f[1].enhance_en;
		d_dcc_skin_tone_en_reg.y_blending_2_enhance_en = face_dcc_apply_f[2].enhance_en;
		d_dcc_skin_tone_en_reg.y_blending_3_enhance_en = face_dcc_apply_f[3].enhance_en;
		d_dcc_skin_tone_en_reg.y_blending_4_enhance_en = face_dcc_apply_f[4].enhance_en;
		d_dcc_skin_tone_en_reg.y_blending_5_enhance_en = face_dcc_apply_f[5].enhance_en;
	// end lesley 0910

	IoReg_Write32(COLOR_DCC_D_DCC_AI_0_reg, dcc_d_dcc_ai_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_1_reg, dcc_d_dcc_ai_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_2_reg, dcc_d_dcc_ai_2_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_3_reg, dcc_d_dcc_ai_3_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_4_reg, dcc_d_dcc_ai_4_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_5_reg, dcc_d_dcc_ai_5_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_6_reg, dcc_d_dcc_ai_6_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_7_reg, dcc_d_dcc_ai_7_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_8_reg, dcc_d_dcc_ai_8_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_9_reg, dcc_d_dcc_ai_9_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_10_reg, dcc_d_dcc_ai_10_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_11_reg, dcc_d_dcc_ai_11_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_12_reg, dcc_d_dcc_ai_12_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_13_reg, dcc_d_dcc_ai_13_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_14_reg, dcc_d_dcc_ai_14_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_15_reg, dcc_d_dcc_ai_15_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_16_reg, dcc_d_dcc_ai_16_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_17_reg, dcc_d_dcc_ai_17_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_18_reg, dcc_d_dcc_ai_18_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_19_reg, dcc_d_dcc_ai_19_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_20_reg, dcc_d_dcc_ai_20_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_21_reg, dcc_d_dcc_ai_21_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_22_reg, dcc_d_dcc_ai_22_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_23_reg, dcc_d_dcc_ai_23_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_24_reg, dcc_d_dcc_ai_24_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_25_reg, dcc_d_dcc_ai_25_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_26_reg, dcc_d_dcc_ai_26_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_27_reg, dcc_d_dcc_ai_27_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_28_reg, dcc_d_dcc_ai_28_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_29_reg, dcc_d_dcc_ai_29_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_30_reg, dcc_d_dcc_ai_30_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_31_reg, dcc_d_dcc_ai_31_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_32_reg, dcc_d_dcc_ai_32_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_33_reg, dcc_d_dcc_ai_33_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_34_reg, dcc_d_dcc_ai_34_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_35_reg, dcc_d_dcc_ai_35_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_36_reg, dcc_d_dcc_ai_36_reg.regValue);
	// end chen 0429

	// chen 0524
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_0_reg, dcc_d_dcc_skin_tone_segment_c0_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_1_reg, dcc_d_dcc_skin_tone_segment_c0_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_0_reg, dcc_d_dcc_skin_tone_segment_c1_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_1_reg, dcc_d_dcc_skin_tone_segment_c1_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_0_reg, dcc_d_dcc_skin_tone_segment_c2_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_1_reg, dcc_d_dcc_skin_tone_segment_c2_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_0_reg, dcc_d_dcc_skin_tone_segment_c3_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_1_reg, dcc_d_dcc_skin_tone_segment_c3_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C4_0_reg, dcc_d_dcc_skin_tone_segment_c4_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C4_1_reg, dcc_d_dcc_skin_tone_segment_c4_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C5_0_reg, dcc_d_dcc_skin_tone_segment_c5_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C5_1_reg, dcc_d_dcc_skin_tone_segment_c5_1_reg.regValue);

	//end chen 0524

	// lesley 0910
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg ,  d_dcc_skin_tone_en_reg.regValue );
	// end lesley 0910

}

// chen 0527
void drvif_color_AI_obj_sharp_set(AI_sharp_apply face_sharp_apply_f[6])
{
	AI_sharp_apply* z_cds_AIInfo;
// yush 190815
	AI_sharp_apply* z_cds_AIInso;// original face info
	AI_sharp_apply a_cds_AIInfo[1];// modified face info
	int shp_face_adjust_en=0;
// end yush 190815
	int y_cb=0;
	int y_cr=0;
	int y_face_count=0;
	int y_face_max=0;
	int z_face_weight=0;
	color_sharp_shp_cds_region_enable_RBUS reg_color_sharp_shp_cds_region_enable_reg;
	reg_color_sharp_shp_cds_region_enable_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg);
	z_cds_AIInfo=face_sharp_apply_f;

// yush 190815
	shp_face_adjust_en = ai_ctrl.ai_shp_tune.shp_face_adjust_en;
	
	if(shp_face_adjust_en)
	{
		z_cds_AIInso=face_sharp_apply_f;
		memcpy(a_cds_AIInfo,z_cds_AIInso,sizeof(a_cds_AIInfo));
		z_cds_AIInfo=a_cds_AIInfo;
	}
// end yush 190815
	z_face_weight=z_cds_AIInfo->pv8;

	if(z_face_weight>0)
	{
		color_sharp_shp_cds_region0_0_RBUS reg_color_sharp_shp_cds_region0_0_reg;
		color_sharp_shp_cds_region0_1_RBUS reg_color_sharp_shp_cds_region0_1_reg;
		color_sharp_shp_cds_region0_2_RBUS reg_color_sharp_shp_cds_region0_2_reg;
		color_sharp_shp_cds_region0_3_RBUS reg_color_sharp_shp_cds_region0_3_reg;
		color_sharp_shp_cds_region0_4_RBUS reg_color_sharp_shp_cds_region0_4_reg;
		if(y_face_max<z_face_weight){y_face_max=z_face_weight;}
// yush 190815
		if(shp_face_adjust_en)
		{
			if(z_face_weight<128)
			{
				// workaround because 6 faces share one confidence in the end
				z_cds_AIInfo->h12=(z_cds_AIInfo->h12*z_face_weight+64)>>7;
				z_cds_AIInfo->w12=(z_cds_AIInfo->w12*z_face_weight+64)>>7;
			}
		}
// end yush 190815
		reg_color_sharp_shp_cds_region0_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_0_reg);
		reg_color_sharp_shp_cds_region0_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_1_reg);
		reg_color_sharp_shp_cds_region0_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_2_reg);
		reg_color_sharp_shp_cds_region0_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_3_reg);
		reg_color_sharp_shp_cds_region0_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_4_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_0=1;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_smooth=c_cds_region_smooth;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_start_i=max(0,z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_end_i=min(2159,z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region0_1_reg.cds_region_0_start_j=max(0,z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region0_1_reg.cds_region_0_end_j=min(3839,z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region0_2_reg.cds_region_0_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region0_2_reg.cds_region_0_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region0_2_reg.cds_region_0_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region0_2_reg.cds_region_0_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region0_3_reg.cds_region_0_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region0_3_reg.cds_region_0_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region0_3_reg.cds_region_0_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region0_3_reg.cds_region_0_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_jrdm=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_jrum=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_irdm=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_irum=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_jldm=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_jlum=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_ildm=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_ilum=0;
		y_cb=y_cb+(1+z_cds_AIInfo->cb_med12*2)*z_face_weight;
		y_cr=y_cr+(1+z_cds_AIInfo->cr_med12*2)*z_face_weight;
		y_face_count=y_face_count+z_face_weight;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_0_reg,reg_color_sharp_shp_cds_region0_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_1_reg,reg_color_sharp_shp_cds_region0_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_2_reg,reg_color_sharp_shp_cds_region0_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_3_reg,reg_color_sharp_shp_cds_region0_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_4_reg,reg_color_sharp_shp_cds_region0_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region0_0_RBUS reg_color_sharp_shp_cds_region0_0_reg;
		color_sharp_shp_cds_region0_1_RBUS reg_color_sharp_shp_cds_region0_1_reg;
		reg_color_sharp_shp_cds_region0_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_0_reg);
		reg_color_sharp_shp_cds_region0_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_0=0;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_smooth=c_cds_region_smooth;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_start_i=2159;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_end_i=0;
		reg_color_sharp_shp_cds_region0_1_reg.cds_region_0_start_j=3839;
		reg_color_sharp_shp_cds_region0_1_reg.cds_region_0_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_0_reg,reg_color_sharp_shp_cds_region0_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_1_reg,reg_color_sharp_shp_cds_region0_1_reg.regValue);
	}
	z_cds_AIInfo=face_sharp_apply_f+1;
// yush 190815
	if(shp_face_adjust_en)
	{
		z_cds_AIInso=face_sharp_apply_f+1;
		memcpy(a_cds_AIInfo,z_cds_AIInso,sizeof(a_cds_AIInfo));
		z_cds_AIInfo=a_cds_AIInfo;
	}
// end yush 190815
		z_face_weight=z_cds_AIInfo->pv8;
	if(z_face_weight>0)
	{
		color_sharp_shp_cds_region1_0_RBUS reg_color_sharp_shp_cds_region1_0_reg;
		color_sharp_shp_cds_region1_1_RBUS reg_color_sharp_shp_cds_region1_1_reg;
		color_sharp_shp_cds_region1_2_RBUS reg_color_sharp_shp_cds_region1_2_reg;
		color_sharp_shp_cds_region1_3_RBUS reg_color_sharp_shp_cds_region1_3_reg;
		color_sharp_shp_cds_region1_4_RBUS reg_color_sharp_shp_cds_region1_4_reg;
		if(y_face_max<z_face_weight){y_face_max=z_face_weight;}
// yush 190815
		if(shp_face_adjust_en)
		{
			if(z_face_weight<128)
			{
				// workaround because 6 faces share one confidence in the end
				z_cds_AIInfo->h12=(z_cds_AIInfo->h12*z_face_weight+64)>>7;
				z_cds_AIInfo->w12=(z_cds_AIInfo->w12*z_face_weight+64)>>7;
			}
		}
// end yush 190815
		reg_color_sharp_shp_cds_region1_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_0_reg);
		reg_color_sharp_shp_cds_region1_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_1_reg);
		reg_color_sharp_shp_cds_region1_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_2_reg);
		reg_color_sharp_shp_cds_region1_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_3_reg);
		reg_color_sharp_shp_cds_region1_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_4_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_1=1;
		reg_color_sharp_shp_cds_region1_0_reg.cds_region_1_start_i=max(0,z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region1_0_reg.cds_region_1_end_i=min(2159,z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region1_1_reg.cds_region_1_start_j=max(0,z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region1_1_reg.cds_region_1_end_j=min(3839,z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region1_2_reg.cds_region_1_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region1_2_reg.cds_region_1_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region1_2_reg.cds_region_1_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region1_2_reg.cds_region_1_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region1_3_reg.cds_region_1_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region1_3_reg.cds_region_1_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region1_3_reg.cds_region_1_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region1_3_reg.cds_region_1_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_jrdm=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_jrum=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_irdm=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_irum=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_jldm=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_jlum=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_ildm=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_ilum=0;
		y_cb=y_cb+(1+z_cds_AIInfo->cb_med12*2)*z_face_weight;
		y_cr=y_cr+(1+z_cds_AIInfo->cr_med12*2)*z_face_weight;
		y_face_count=y_face_count+z_face_weight;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_0_reg,reg_color_sharp_shp_cds_region1_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_1_reg,reg_color_sharp_shp_cds_region1_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_2_reg,reg_color_sharp_shp_cds_region1_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_3_reg,reg_color_sharp_shp_cds_region1_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_4_reg,reg_color_sharp_shp_cds_region1_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region1_0_RBUS reg_color_sharp_shp_cds_region1_0_reg;
		color_sharp_shp_cds_region1_1_RBUS reg_color_sharp_shp_cds_region1_1_reg;
		reg_color_sharp_shp_cds_region1_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_0_reg);
		reg_color_sharp_shp_cds_region1_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_1=0;
		reg_color_sharp_shp_cds_region1_0_reg.cds_region_1_start_i=2159;
		reg_color_sharp_shp_cds_region1_0_reg.cds_region_1_end_i=0;
		reg_color_sharp_shp_cds_region1_1_reg.cds_region_1_start_j=3839;
		reg_color_sharp_shp_cds_region1_1_reg.cds_region_1_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_0_reg,reg_color_sharp_shp_cds_region1_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_1_reg,reg_color_sharp_shp_cds_region1_1_reg.regValue);
	}
		z_cds_AIInfo=face_sharp_apply_f+2;
// yush 190815
	if(shp_face_adjust_en)
	{
		z_cds_AIInso=face_sharp_apply_f+2;
		memcpy(a_cds_AIInfo,z_cds_AIInso,sizeof(a_cds_AIInfo));
		z_cds_AIInfo=a_cds_AIInfo;
	}
// end yush 190815
	z_face_weight=z_cds_AIInfo->pv8;
	if(z_face_weight>0)
	{
		color_sharp_shp_cds_region2_0_RBUS reg_color_sharp_shp_cds_region2_0_reg;
		color_sharp_shp_cds_region2_1_RBUS reg_color_sharp_shp_cds_region2_1_reg;
		color_sharp_shp_cds_region2_2_RBUS reg_color_sharp_shp_cds_region2_2_reg;
		color_sharp_shp_cds_region2_3_RBUS reg_color_sharp_shp_cds_region2_3_reg;
		color_sharp_shp_cds_region2_4_RBUS reg_color_sharp_shp_cds_region2_4_reg;
		if(y_face_max<z_face_weight){y_face_max=z_face_weight;}
// yush 190815
		if(shp_face_adjust_en)
		{
			if(z_face_weight<128)
			{
				// workaround because 6 faces share one confidence in the end
				z_cds_AIInfo->h12=(z_cds_AIInfo->h12*z_face_weight+64)>>7;
				z_cds_AIInfo->w12=(z_cds_AIInfo->w12*z_face_weight+64)>>7;
			}
		}
// end yush 190815
		reg_color_sharp_shp_cds_region2_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_0_reg);
		reg_color_sharp_shp_cds_region2_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_1_reg);
		reg_color_sharp_shp_cds_region2_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_2_reg);
		reg_color_sharp_shp_cds_region2_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_3_reg);
		reg_color_sharp_shp_cds_region2_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_4_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_2=1;
		reg_color_sharp_shp_cds_region2_0_reg.cds_region_2_start_i=max(0,z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region2_0_reg.cds_region_2_end_i=min(2159,z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region2_1_reg.cds_region_2_start_j=max(0,z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region2_1_reg.cds_region_2_end_j=min(3839,z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region2_2_reg.cds_region_2_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region2_2_reg.cds_region_2_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region2_2_reg.cds_region_2_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region2_2_reg.cds_region_2_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region2_3_reg.cds_region_2_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region2_3_reg.cds_region_2_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region2_3_reg.cds_region_2_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region2_3_reg.cds_region_2_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_jrdm=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_jrum=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_irdm=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_irum=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_jldm=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_jlum=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_ildm=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_ilum=0;
		y_cb=y_cb+(1+z_cds_AIInfo->cb_med12*2)*z_face_weight;
		y_cr=y_cr+(1+z_cds_AIInfo->cr_med12*2)*z_face_weight;
		y_face_count=y_face_count+z_face_weight;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_0_reg,reg_color_sharp_shp_cds_region2_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_1_reg,reg_color_sharp_shp_cds_region2_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_2_reg,reg_color_sharp_shp_cds_region2_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_3_reg,reg_color_sharp_shp_cds_region2_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_4_reg,reg_color_sharp_shp_cds_region2_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region2_0_RBUS reg_color_sharp_shp_cds_region2_0_reg;
		color_sharp_shp_cds_region2_1_RBUS reg_color_sharp_shp_cds_region2_1_reg;
		reg_color_sharp_shp_cds_region2_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_0_reg);
		reg_color_sharp_shp_cds_region2_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_2=0;
		reg_color_sharp_shp_cds_region2_0_reg.cds_region_2_start_i=2159;
		reg_color_sharp_shp_cds_region2_0_reg.cds_region_2_end_i=0;
		reg_color_sharp_shp_cds_region2_1_reg.cds_region_2_start_j=3839;
		reg_color_sharp_shp_cds_region2_1_reg.cds_region_2_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_0_reg,reg_color_sharp_shp_cds_region2_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_1_reg,reg_color_sharp_shp_cds_region2_1_reg.regValue);
	}
	z_cds_AIInfo=face_sharp_apply_f+3;
// yush 190815
	if(shp_face_adjust_en)
	{
		z_cds_AIInso=face_sharp_apply_f+3;
		memcpy(a_cds_AIInfo,z_cds_AIInso,sizeof(a_cds_AIInfo));
		z_cds_AIInfo=a_cds_AIInfo;
	}
// end yush 190815
	z_face_weight=z_cds_AIInfo->pv8;
	if(z_face_weight>0)
	{
		color_sharp_shp_cds_region3_0_RBUS reg_color_sharp_shp_cds_region3_0_reg;
		color_sharp_shp_cds_region3_1_RBUS reg_color_sharp_shp_cds_region3_1_reg;
		color_sharp_shp_cds_region3_2_RBUS reg_color_sharp_shp_cds_region3_2_reg;
		color_sharp_shp_cds_region3_3_RBUS reg_color_sharp_shp_cds_region3_3_reg;
		color_sharp_shp_cds_region3_4_RBUS reg_color_sharp_shp_cds_region3_4_reg;
		if(y_face_max<z_face_weight){y_face_max=z_face_weight;}
// yush 190815
		if(shp_face_adjust_en)
		{
			if(z_face_weight<128)
			{
				// workaround because 6 faces share one confidence in the end
				z_cds_AIInfo->h12=(z_cds_AIInfo->h12*z_face_weight+64)>>7;
				z_cds_AIInfo->w12=(z_cds_AIInfo->w12*z_face_weight+64)>>7;
			}
		}
// end yush 190815
		reg_color_sharp_shp_cds_region3_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_0_reg);
		reg_color_sharp_shp_cds_region3_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_1_reg);
		reg_color_sharp_shp_cds_region3_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_2_reg);
		reg_color_sharp_shp_cds_region3_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_3_reg);
		reg_color_sharp_shp_cds_region3_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_4_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_3=1;
		reg_color_sharp_shp_cds_region3_0_reg.cds_region_3_start_i=max(0,z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region3_0_reg.cds_region_3_end_i=min(2159,z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region3_1_reg.cds_region_3_start_j=max(0,z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region3_1_reg.cds_region_3_end_j=min(3839,z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region3_2_reg.cds_region_3_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region3_2_reg.cds_region_3_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region3_2_reg.cds_region_3_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region3_2_reg.cds_region_3_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region3_3_reg.cds_region_3_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region3_3_reg.cds_region_3_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region3_3_reg.cds_region_3_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region3_3_reg.cds_region_3_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_jrdm=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_jrum=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_irdm=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_irum=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_jldm=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_jlum=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_ildm=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_ilum=0;
		y_cb=y_cb+(1+z_cds_AIInfo->cb_med12*2)*z_face_weight;
		y_cr=y_cr+(1+z_cds_AIInfo->cr_med12*2)*z_face_weight;
		y_face_count=y_face_count+z_face_weight;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_0_reg,reg_color_sharp_shp_cds_region3_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_1_reg,reg_color_sharp_shp_cds_region3_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_2_reg,reg_color_sharp_shp_cds_region3_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_3_reg,reg_color_sharp_shp_cds_region3_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_4_reg,reg_color_sharp_shp_cds_region3_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region3_0_RBUS reg_color_sharp_shp_cds_region3_0_reg;
		color_sharp_shp_cds_region3_1_RBUS reg_color_sharp_shp_cds_region3_1_reg;
		reg_color_sharp_shp_cds_region3_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_0_reg);
		reg_color_sharp_shp_cds_region3_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_3=0;
		reg_color_sharp_shp_cds_region3_0_reg.cds_region_3_start_i=2159;
		reg_color_sharp_shp_cds_region3_0_reg.cds_region_3_end_i=0;
		reg_color_sharp_shp_cds_region3_1_reg.cds_region_3_start_j=3839;
		reg_color_sharp_shp_cds_region3_1_reg.cds_region_3_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_0_reg,reg_color_sharp_shp_cds_region3_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_1_reg,reg_color_sharp_shp_cds_region3_1_reg.regValue);
	}
	z_cds_AIInfo=face_sharp_apply_f+4;
// yush 190815
	if(shp_face_adjust_en)
	{
		z_cds_AIInso=face_sharp_apply_f+4;
		memcpy(a_cds_AIInfo,z_cds_AIInso,sizeof(a_cds_AIInfo));
		z_cds_AIInfo=a_cds_AIInfo;
	}
// end yush 190815
	z_face_weight=z_cds_AIInfo->pv8;
	if(z_face_weight>0)
	{
		color_sharp_shp_cds_region4_0_RBUS reg_color_sharp_shp_cds_region4_0_reg;
		color_sharp_shp_cds_region4_1_RBUS reg_color_sharp_shp_cds_region4_1_reg;
		color_sharp_shp_cds_region4_2_RBUS reg_color_sharp_shp_cds_region4_2_reg;
		color_sharp_shp_cds_region4_3_RBUS reg_color_sharp_shp_cds_region4_3_reg;
		color_sharp_shp_cds_region4_4_RBUS reg_color_sharp_shp_cds_region4_4_reg;
		if(y_face_max<z_face_weight){y_face_max=z_face_weight;}
// yush 190815
		if(shp_face_adjust_en)
		{
			if(z_face_weight<128)
			{
				// workaround because 6 faces share one confidence in the end
				z_cds_AIInfo->h12=(z_cds_AIInfo->h12*z_face_weight+64)>>7;
				z_cds_AIInfo->w12=(z_cds_AIInfo->w12*z_face_weight+64)>>7;
			}
		}
// end yush 190815
		reg_color_sharp_shp_cds_region4_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_0_reg);
		reg_color_sharp_shp_cds_region4_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_1_reg);
		reg_color_sharp_shp_cds_region4_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_2_reg);
		reg_color_sharp_shp_cds_region4_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_3_reg);
		reg_color_sharp_shp_cds_region4_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_4_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_4=1;
		reg_color_sharp_shp_cds_region4_0_reg.cds_region_4_start_i=max(0,z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region4_0_reg.cds_region_4_end_i=min(2159,z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region4_1_reg.cds_region_4_start_j=max(0,z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region4_1_reg.cds_region_4_end_j=min(3839,z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region4_2_reg.cds_region_4_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region4_2_reg.cds_region_4_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region4_2_reg.cds_region_4_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region4_2_reg.cds_region_4_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region4_3_reg.cds_region_4_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region4_3_reg.cds_region_4_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region4_3_reg.cds_region_4_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region4_3_reg.cds_region_4_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_jrdm=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_jrum=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_irdm=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_irum=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_jldm=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_jlum=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_ildm=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_ilum=0;
		y_cb=y_cb+(1+z_cds_AIInfo->cb_med12*2)*z_face_weight;
		y_cr=y_cr+(1+z_cds_AIInfo->cr_med12*2)*z_face_weight;
		y_face_count=y_face_count+z_face_weight;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_0_reg,reg_color_sharp_shp_cds_region4_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_1_reg,reg_color_sharp_shp_cds_region4_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_2_reg,reg_color_sharp_shp_cds_region4_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_3_reg,reg_color_sharp_shp_cds_region4_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_4_reg,reg_color_sharp_shp_cds_region4_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region4_0_RBUS reg_color_sharp_shp_cds_region4_0_reg;
		color_sharp_shp_cds_region4_1_RBUS reg_color_sharp_shp_cds_region4_1_reg;
		reg_color_sharp_shp_cds_region4_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_0_reg);
		reg_color_sharp_shp_cds_region4_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_4=0;
		reg_color_sharp_shp_cds_region4_0_reg.cds_region_4_start_i=2159;
		reg_color_sharp_shp_cds_region4_0_reg.cds_region_4_end_i=0;
		reg_color_sharp_shp_cds_region4_1_reg.cds_region_4_start_j=3839;
		reg_color_sharp_shp_cds_region4_1_reg.cds_region_4_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_0_reg,reg_color_sharp_shp_cds_region4_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_1_reg,reg_color_sharp_shp_cds_region4_1_reg.regValue);
	}
	z_cds_AIInfo=face_sharp_apply_f+5;
// yush 190815
	if(shp_face_adjust_en)
	{
		z_cds_AIInso=face_sharp_apply_f+5;
		memcpy(a_cds_AIInfo,z_cds_AIInso,sizeof(a_cds_AIInfo));
		z_cds_AIInfo=a_cds_AIInfo;
	}
// end yush 190815
	z_face_weight=z_cds_AIInfo->pv8;
	if(z_face_weight>0)
	{
		color_sharp_shp_cds_region5_0_RBUS reg_color_sharp_shp_cds_region5_0_reg;
		color_sharp_shp_cds_region5_1_RBUS reg_color_sharp_shp_cds_region5_1_reg;
		color_sharp_shp_cds_region5_2_RBUS reg_color_sharp_shp_cds_region5_2_reg;
		color_sharp_shp_cds_region5_3_RBUS reg_color_sharp_shp_cds_region5_3_reg;
		color_sharp_shp_cds_region5_4_RBUS reg_color_sharp_shp_cds_region5_4_reg;
		if(y_face_max<z_face_weight){y_face_max=z_face_weight;}
// yush 190815
		if(shp_face_adjust_en)
		{
			if(z_face_weight<128)
			{
				// workaround because 6 faces share one confidence in the end
				z_cds_AIInfo->h12=(z_cds_AIInfo->h12*z_face_weight+64)>>7;
				z_cds_AIInfo->w12=(z_cds_AIInfo->w12*z_face_weight+64)>>7;
			}
		}
// end yush 190815
		reg_color_sharp_shp_cds_region5_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_0_reg);
		reg_color_sharp_shp_cds_region5_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_1_reg);
		reg_color_sharp_shp_cds_region5_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_2_reg);
		reg_color_sharp_shp_cds_region5_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_3_reg);
		reg_color_sharp_shp_cds_region5_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_4_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_5=1;
		reg_color_sharp_shp_cds_region5_0_reg.cds_region_5_start_i=max(0,z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region5_0_reg.cds_region_5_end_i=min(2159,z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1));
		reg_color_sharp_shp_cds_region5_1_reg.cds_region_5_start_j=max(0,z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region5_1_reg.cds_region_5_end_j=min(3839,z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1));
		reg_color_sharp_shp_cds_region5_2_reg.cds_region_5_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region5_2_reg.cds_region_5_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region5_2_reg.cds_region_5_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region5_2_reg.cds_region_5_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region5_3_reg.cds_region_5_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region5_3_reg.cds_region_5_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region5_3_reg.cds_region_5_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region5_3_reg.cds_region_5_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_jrdm=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_jrum=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_irdm=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_irum=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_jldm=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_jlum=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_ildm=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_ilum=0;
		y_cb=y_cb+(1+z_cds_AIInfo->cb_med12*2)*z_face_weight;
		y_cr=y_cr+(1+z_cds_AIInfo->cr_med12*2)*z_face_weight;
		y_face_count=y_face_count+z_face_weight;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_0_reg,reg_color_sharp_shp_cds_region5_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_1_reg,reg_color_sharp_shp_cds_region5_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_2_reg,reg_color_sharp_shp_cds_region5_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_3_reg,reg_color_sharp_shp_cds_region5_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_4_reg,reg_color_sharp_shp_cds_region5_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region5_0_RBUS reg_color_sharp_shp_cds_region5_0_reg;
		color_sharp_shp_cds_region5_1_RBUS reg_color_sharp_shp_cds_region5_1_reg;
		reg_color_sharp_shp_cds_region5_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_0_reg);
		reg_color_sharp_shp_cds_region5_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_5=0;
		reg_color_sharp_shp_cds_region5_0_reg.cds_region_5_start_i=2159;
		reg_color_sharp_shp_cds_region5_0_reg.cds_region_5_end_i=0;
		reg_color_sharp_shp_cds_region5_1_reg.cds_region_5_start_j=3839;
		reg_color_sharp_shp_cds_region5_1_reg.cds_region_5_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_0_reg,reg_color_sharp_shp_cds_region5_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_1_reg,reg_color_sharp_shp_cds_region5_1_reg.regValue);
	}
	if(y_face_count>0)
	{
		color_sharp_dm_cds_cm0_u_bound_1_RBUS reg_color_sharp_dm_cds_cm0_u_bound_1_reg;
		color_sharp_shp_cds_predict_RBUS reg_color_sharp_shp_cds_predict_reg;
		reg_color_sharp_dm_cds_cm0_u_bound_1_reg.regValue=IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_1_reg);
		reg_color_sharp_shp_cds_predict_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_PREDICT_reg);
		y_cb=y_cb*4/(y_face_count<<3);
		y_cr=y_cr*4/(y_face_count<<3);
		y_cb=y_cb/4;
		y_cr=y_cr/4;
		reg_color_sharp_dm_cds_cm0_u_bound_1_reg.cu_0=y_cb;
		reg_color_sharp_dm_cds_cm0_u_bound_1_reg.cv_0=y_cr;
		reg_color_sharp_shp_cds_predict_reg.cds_region_predict_2=y_face_max;
		reg_color_sharp_shp_cds_predict_reg.cds_region_predict_1=y_face_max;

		IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_1_reg,reg_color_sharp_dm_cds_cm0_u_bound_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_PREDICT_reg,reg_color_sharp_shp_cds_predict_reg.regValue);
	}
	IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg,reg_color_sharp_shp_cds_region_enable_reg.regValue);
	{
		color_sharp_dm_nr_shp_ctrl_5_RBUS reg_color_sharp_dm_nr_shp_ctrl_5_reg;
		reg_color_sharp_dm_nr_shp_ctrl_5_reg.regValue=IoReg_Read32(COLOR_SHARP_DM_NR_SHP_CTRL_5_reg);
		reg_color_sharp_dm_nr_shp_ctrl_5_reg.db_load=1;
		IoReg_Write32(COLOR_SHARP_DM_NR_SHP_CTRL_5_reg,reg_color_sharp_dm_nr_shp_ctrl_5_reg.regValue);
	}
}
//end chen 0527

/*void drvif_color_AI_obj_cds_set(unsigned char enable)
{
	// for 2020 CES show 
	// force cds enable when face detected

#if 0// mac7p no these register, sub function always on
	color_sharp_dm_cds_peaking_gain_bound_ctrl_RBUS color_sharp_dm_cds_peaking_gain_bound_ctrl_reg;
	
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg);

	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_edg_enable = enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_tex_enable = enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_enable = enable;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_edg_enable = enable;

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg,color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue);
#endif
}*/

#if 0
void drvif_color_AI_obj_sharpness_set(void)
{
	AIInfo* todo_cds_AIInfo=(AIInfo*)0;
	AIInfo* z_cds_AIInfo;
	int y_cb=0;
	int y_cr=0;
	int y_face_count=0;
	color_sharp_shp_cds_region_enable_RBUS reg_color_sharp_shp_cds_region_enable_reg;
	reg_color_sharp_shp_cds_region_enable_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg);
	if(!todo_cds_AIInfo){return;}
	if(sizeof(todo_cds_AIInfo)>=sizeof(AIInfo))
	{
		color_sharp_shp_cds_region0_0_RBUS reg_color_sharp_shp_cds_region0_0_reg;
		color_sharp_shp_cds_region0_1_RBUS reg_color_sharp_shp_cds_region0_1_reg;
		color_sharp_shp_cds_region0_2_RBUS reg_color_sharp_shp_cds_region0_2_reg;
		color_sharp_shp_cds_region0_3_RBUS reg_color_sharp_shp_cds_region0_3_reg;
		color_sharp_shp_cds_region0_4_RBUS reg_color_sharp_shp_cds_region0_4_reg;
		reg_color_sharp_shp_cds_region0_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_0_reg);
		reg_color_sharp_shp_cds_region0_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_1_reg);
		reg_color_sharp_shp_cds_region0_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_2_reg);
		reg_color_sharp_shp_cds_region0_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_3_reg);
		reg_color_sharp_shp_cds_region0_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_4_reg);
		z_cds_AIInfo=todo_cds_AIInfo;
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_0=1;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_smooth=c_cds_region_smooth;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_start_i=z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_end_i=z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region0_1_reg.cds_region_0_start_j=z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region0_1_reg.cds_region_0_end_j=z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region0_2_reg.cds_region_0_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region0_2_reg.cds_region_0_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region0_2_reg.cds_region_0_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region0_2_reg.cds_region_0_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region0_3_reg.cds_region_0_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region0_3_reg.cds_region_0_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region0_3_reg.cds_region_0_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region0_3_reg.cds_region_0_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_jrdm=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_jrum=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_irdm=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_irum=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_jldm=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_jlum=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_ildm=0;
		reg_color_sharp_shp_cds_region0_4_reg.cds_region_0_ilum=0;
		y_cb=y_cb+1+z_cds_AIInfo->cb_med12*2;
		y_cr=y_cr+1+z_cds_AIInfo->cr_med12*2;
		y_face_count=y_face_count+1;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_0_reg,reg_color_sharp_shp_cds_region0_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_1_reg,reg_color_sharp_shp_cds_region0_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_2_reg,reg_color_sharp_shp_cds_region0_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_3_reg,reg_color_sharp_shp_cds_region0_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_4_reg,reg_color_sharp_shp_cds_region0_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region0_0_RBUS reg_color_sharp_shp_cds_region0_0_reg;
		color_sharp_shp_cds_region0_1_RBUS reg_color_sharp_shp_cds_region0_1_reg;
		reg_color_sharp_shp_cds_region0_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_0_reg);
		reg_color_sharp_shp_cds_region0_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION0_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_0=0;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_smooth=c_cds_region_smooth;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_start_i=2159;
		reg_color_sharp_shp_cds_region0_0_reg.cds_region_0_end_i=0;
		reg_color_sharp_shp_cds_region0_1_reg.cds_region_0_start_j=3839;
		reg_color_sharp_shp_cds_region0_1_reg.cds_region_0_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_0_reg,reg_color_sharp_shp_cds_region0_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION0_1_reg,reg_color_sharp_shp_cds_region0_1_reg.regValue);
	}
	if(sizeof(todo_cds_AIInfo)>=sizeof(AIInfo)*2)
	{
		color_sharp_shp_cds_region1_0_RBUS reg_color_sharp_shp_cds_region1_0_reg;
		color_sharp_shp_cds_region1_1_RBUS reg_color_sharp_shp_cds_region1_1_reg;
		color_sharp_shp_cds_region1_2_RBUS reg_color_sharp_shp_cds_region1_2_reg;
		color_sharp_shp_cds_region1_3_RBUS reg_color_sharp_shp_cds_region1_3_reg;
		color_sharp_shp_cds_region1_4_RBUS reg_color_sharp_shp_cds_region1_4_reg;
		reg_color_sharp_shp_cds_region1_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_0_reg);
		reg_color_sharp_shp_cds_region1_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_1_reg);
		reg_color_sharp_shp_cds_region1_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_2_reg);
		reg_color_sharp_shp_cds_region1_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_3_reg);
		reg_color_sharp_shp_cds_region1_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_4_reg);
		z_cds_AIInfo=todo_cds_AIInfo+1;
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_1=1;
		reg_color_sharp_shp_cds_region1_0_reg.cds_region_1_start_i=z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region1_0_reg.cds_region_1_end_i=z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region1_1_reg.cds_region_1_start_j=z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region1_1_reg.cds_region_1_end_j=z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region1_2_reg.cds_region_1_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region1_2_reg.cds_region_1_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region1_2_reg.cds_region_1_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region1_2_reg.cds_region_1_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region1_3_reg.cds_region_1_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region1_3_reg.cds_region_1_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region1_3_reg.cds_region_1_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region1_3_reg.cds_region_1_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_jrdm=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_jrum=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_irdm=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_irum=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_jldm=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_jlum=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_ildm=0;
		reg_color_sharp_shp_cds_region1_4_reg.cds_region_1_ilum=0;
		y_cb=y_cb+1+z_cds_AIInfo->cb_med12*2;
		y_cr=y_cr+1+z_cds_AIInfo->cr_med12*2;
		y_face_count=y_face_count+1;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_0_reg,reg_color_sharp_shp_cds_region1_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_1_reg,reg_color_sharp_shp_cds_region1_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_2_reg,reg_color_sharp_shp_cds_region1_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_3_reg,reg_color_sharp_shp_cds_region1_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_4_reg,reg_color_sharp_shp_cds_region1_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region1_0_RBUS reg_color_sharp_shp_cds_region1_0_reg;
		color_sharp_shp_cds_region1_1_RBUS reg_color_sharp_shp_cds_region1_1_reg;
		reg_color_sharp_shp_cds_region1_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_0_reg);
		reg_color_sharp_shp_cds_region1_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_1=0;
		reg_color_sharp_shp_cds_region1_0_reg.cds_region_1_start_i=2159;
		reg_color_sharp_shp_cds_region1_0_reg.cds_region_1_end_i=0;
		reg_color_sharp_shp_cds_region1_1_reg.cds_region_1_start_j=3839;
		reg_color_sharp_shp_cds_region1_1_reg.cds_region_1_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_0_reg,reg_color_sharp_shp_cds_region1_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_1_reg,reg_color_sharp_shp_cds_region1_1_reg.regValue);
	}
	if(sizeof(todo_cds_AIInfo)>=sizeof(AIInfo)*3)
	{
		color_sharp_shp_cds_region2_0_RBUS reg_color_sharp_shp_cds_region2_0_reg;
		color_sharp_shp_cds_region2_1_RBUS reg_color_sharp_shp_cds_region2_1_reg;
		color_sharp_shp_cds_region2_2_RBUS reg_color_sharp_shp_cds_region2_2_reg;
		color_sharp_shp_cds_region2_3_RBUS reg_color_sharp_shp_cds_region2_3_reg;
		color_sharp_shp_cds_region2_4_RBUS reg_color_sharp_shp_cds_region2_4_reg;
		reg_color_sharp_shp_cds_region2_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_0_reg);
		reg_color_sharp_shp_cds_region2_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_1_reg);
		reg_color_sharp_shp_cds_region2_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_2_reg);
		reg_color_sharp_shp_cds_region2_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_3_reg);
		reg_color_sharp_shp_cds_region2_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_4_reg);
		z_cds_AIInfo=todo_cds_AIInfo+2;
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_2=1;
		reg_color_sharp_shp_cds_region2_0_reg.cds_region_2_start_i=z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region2_0_reg.cds_region_2_end_i=z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region2_1_reg.cds_region_2_start_j=z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region2_1_reg.cds_region_2_end_j=z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region2_2_reg.cds_region_2_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region2_2_reg.cds_region_2_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region2_2_reg.cds_region_2_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region2_2_reg.cds_region_2_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region2_3_reg.cds_region_2_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region2_3_reg.cds_region_2_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region2_3_reg.cds_region_2_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region2_3_reg.cds_region_2_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_jrdm=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_jrum=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_irdm=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_irum=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_jldm=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_jlum=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_ildm=0;
		reg_color_sharp_shp_cds_region2_4_reg.cds_region_2_ilum=0;
		y_cb=y_cb+1+z_cds_AIInfo->cb_med12*2;
		y_cr=y_cr+1+z_cds_AIInfo->cr_med12*2;
		y_face_count=y_face_count+1;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_0_reg,reg_color_sharp_shp_cds_region2_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_1_reg,reg_color_sharp_shp_cds_region2_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_2_reg,reg_color_sharp_shp_cds_region2_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_3_reg,reg_color_sharp_shp_cds_region2_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_4_reg,reg_color_sharp_shp_cds_region2_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region2_0_RBUS reg_color_sharp_shp_cds_region2_0_reg;
		color_sharp_shp_cds_region2_1_RBUS reg_color_sharp_shp_cds_region2_1_reg;
		reg_color_sharp_shp_cds_region2_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_0_reg);
		reg_color_sharp_shp_cds_region2_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_2=0;
		reg_color_sharp_shp_cds_region2_0_reg.cds_region_2_start_i=2159;
		reg_color_sharp_shp_cds_region2_0_reg.cds_region_2_end_i=0;
		reg_color_sharp_shp_cds_region2_1_reg.cds_region_2_start_j=3839;
		reg_color_sharp_shp_cds_region2_1_reg.cds_region_2_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_0_reg,reg_color_sharp_shp_cds_region2_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_1_reg,reg_color_sharp_shp_cds_region2_1_reg.regValue);
	}
	if(sizeof(todo_cds_AIInfo)>=sizeof(AIInfo)*4)
	{
		color_sharp_shp_cds_region3_0_RBUS reg_color_sharp_shp_cds_region3_0_reg;
		color_sharp_shp_cds_region3_1_RBUS reg_color_sharp_shp_cds_region3_1_reg;
		color_sharp_shp_cds_region3_2_RBUS reg_color_sharp_shp_cds_region3_2_reg;
		color_sharp_shp_cds_region3_3_RBUS reg_color_sharp_shp_cds_region3_3_reg;
		color_sharp_shp_cds_region3_4_RBUS reg_color_sharp_shp_cds_region3_4_reg;
		reg_color_sharp_shp_cds_region3_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_0_reg);
		reg_color_sharp_shp_cds_region3_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_1_reg);
		reg_color_sharp_shp_cds_region3_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_2_reg);
		reg_color_sharp_shp_cds_region3_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_3_reg);
		reg_color_sharp_shp_cds_region3_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_4_reg);
		z_cds_AIInfo=todo_cds_AIInfo+3;
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_3=1;
		reg_color_sharp_shp_cds_region3_0_reg.cds_region_3_start_i=z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region3_0_reg.cds_region_3_end_i=z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region3_1_reg.cds_region_3_start_j=z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region3_1_reg.cds_region_3_end_j=z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region3_2_reg.cds_region_3_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region3_2_reg.cds_region_3_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region3_2_reg.cds_region_3_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region3_2_reg.cds_region_3_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region3_3_reg.cds_region_3_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region3_3_reg.cds_region_3_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region3_3_reg.cds_region_3_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region3_3_reg.cds_region_3_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_jrdm=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_jrum=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_irdm=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_irum=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_jldm=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_jlum=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_ildm=0;
		reg_color_sharp_shp_cds_region3_4_reg.cds_region_3_ilum=0;
		y_cb=y_cb+1+z_cds_AIInfo->cb_med12*2;
		y_cr=y_cr+1+z_cds_AIInfo->cr_med12*2;
		y_face_count=y_face_count+1;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_0_reg,reg_color_sharp_shp_cds_region3_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_1_reg,reg_color_sharp_shp_cds_region3_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_2_reg,reg_color_sharp_shp_cds_region3_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_3_reg,reg_color_sharp_shp_cds_region3_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_4_reg,reg_color_sharp_shp_cds_region3_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region3_0_RBUS reg_color_sharp_shp_cds_region3_0_reg;
		color_sharp_shp_cds_region3_1_RBUS reg_color_sharp_shp_cds_region3_1_reg;
		reg_color_sharp_shp_cds_region3_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_0_reg);
		reg_color_sharp_shp_cds_region3_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION3_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_3=0;
		reg_color_sharp_shp_cds_region3_0_reg.cds_region_3_start_i=2159;
		reg_color_sharp_shp_cds_region3_0_reg.cds_region_3_end_i=0;
		reg_color_sharp_shp_cds_region3_1_reg.cds_region_3_start_j=3839;
		reg_color_sharp_shp_cds_region3_1_reg.cds_region_3_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_0_reg,reg_color_sharp_shp_cds_region3_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION3_1_reg,reg_color_sharp_shp_cds_region3_1_reg.regValue);
	}
	if(sizeof(todo_cds_AIInfo)>=sizeof(AIInfo)*5)
	{
		color_sharp_shp_cds_region4_0_RBUS reg_color_sharp_shp_cds_region4_0_reg;
		color_sharp_shp_cds_region4_1_RBUS reg_color_sharp_shp_cds_region4_1_reg;
		color_sharp_shp_cds_region4_2_RBUS reg_color_sharp_shp_cds_region4_2_reg;
		color_sharp_shp_cds_region4_3_RBUS reg_color_sharp_shp_cds_region4_3_reg;
		color_sharp_shp_cds_region4_4_RBUS reg_color_sharp_shp_cds_region4_4_reg;
		reg_color_sharp_shp_cds_region4_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_0_reg);
		reg_color_sharp_shp_cds_region4_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_1_reg);
		reg_color_sharp_shp_cds_region4_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_2_reg);
		reg_color_sharp_shp_cds_region4_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_3_reg);
		reg_color_sharp_shp_cds_region4_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_4_reg);
		z_cds_AIInfo=todo_cds_AIInfo+4;
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_4=1;
		reg_color_sharp_shp_cds_region4_0_reg.cds_region_4_start_i=z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region4_0_reg.cds_region_4_end_i=z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region4_1_reg.cds_region_4_start_j=z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region4_1_reg.cds_region_4_end_j=z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region4_2_reg.cds_region_4_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region4_2_reg.cds_region_4_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region4_2_reg.cds_region_4_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region4_2_reg.cds_region_4_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region4_3_reg.cds_region_4_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region4_3_reg.cds_region_4_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region4_3_reg.cds_region_4_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region4_3_reg.cds_region_4_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_jrdm=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_jrum=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_irdm=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_irum=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_jldm=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_jlum=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_ildm=0;
		reg_color_sharp_shp_cds_region4_4_reg.cds_region_4_ilum=0;
		y_cb=y_cb+1+z_cds_AIInfo->cb_med12*2;
		y_cr=y_cr+1+z_cds_AIInfo->cr_med12*2;
		y_face_count=y_face_count+1;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_0_reg,reg_color_sharp_shp_cds_region4_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_1_reg,reg_color_sharp_shp_cds_region4_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_2_reg,reg_color_sharp_shp_cds_region4_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_3_reg,reg_color_sharp_shp_cds_region4_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_4_reg,reg_color_sharp_shp_cds_region4_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region4_0_RBUS reg_color_sharp_shp_cds_region4_0_reg;
		color_sharp_shp_cds_region4_1_RBUS reg_color_sharp_shp_cds_region4_1_reg;
		reg_color_sharp_shp_cds_region4_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_0_reg);
		reg_color_sharp_shp_cds_region4_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION4_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_4=0;
		reg_color_sharp_shp_cds_region4_0_reg.cds_region_4_start_i=2159;
		reg_color_sharp_shp_cds_region4_0_reg.cds_region_4_end_i=0;
		reg_color_sharp_shp_cds_region4_1_reg.cds_region_4_start_j=3839;
		reg_color_sharp_shp_cds_region4_1_reg.cds_region_4_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_0_reg,reg_color_sharp_shp_cds_region4_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION4_1_reg,reg_color_sharp_shp_cds_region4_1_reg.regValue);
	}
	if(sizeof(todo_cds_AIInfo)>=sizeof(AIInfo)*6)
	{
		color_sharp_shp_cds_region5_0_RBUS reg_color_sharp_shp_cds_region5_0_reg;
		color_sharp_shp_cds_region5_1_RBUS reg_color_sharp_shp_cds_region5_1_reg;
		color_sharp_shp_cds_region5_2_RBUS reg_color_sharp_shp_cds_region5_2_reg;
		color_sharp_shp_cds_region5_3_RBUS reg_color_sharp_shp_cds_region5_3_reg;
		color_sharp_shp_cds_region5_4_RBUS reg_color_sharp_shp_cds_region5_4_reg;
		reg_color_sharp_shp_cds_region5_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_0_reg);
		reg_color_sharp_shp_cds_region5_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_1_reg);
		reg_color_sharp_shp_cds_region5_2_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_2_reg);
		reg_color_sharp_shp_cds_region5_3_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_3_reg);
		reg_color_sharp_shp_cds_region5_4_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_4_reg);
		z_cds_AIInfo=todo_cds_AIInfo+5;
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_5=1;
		reg_color_sharp_shp_cds_region5_0_reg.cds_region_5_start_i=z_cds_AIInfo->cy12-(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region5_0_reg.cds_region_5_end_i=z_cds_AIInfo->cy12+(z_cds_AIInfo->h12>>1);
		reg_color_sharp_shp_cds_region5_1_reg.cds_region_5_start_j=z_cds_AIInfo->cx12-(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region5_1_reg.cds_region_5_end_j=z_cds_AIInfo->cx12+(z_cds_AIInfo->w12>>1);
		reg_color_sharp_shp_cds_region5_2_reg.cds_region_5_ilu=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region5_2_reg.cds_region_5_jlu=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region5_2_reg.cds_region_5_ild=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region5_2_reg.cds_region_5_jld=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region5_3_reg.cds_region_5_iru=z_cds_AIInfo->h12>>3;
		reg_color_sharp_shp_cds_region5_3_reg.cds_region_5_jru=z_cds_AIInfo->w12>>3;
		reg_color_sharp_shp_cds_region5_3_reg.cds_region_5_ird=z_cds_AIInfo->h12>>2;
		reg_color_sharp_shp_cds_region5_3_reg.cds_region_5_jrd=z_cds_AIInfo->w12>>2;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_jrdm=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_jrum=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_irdm=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_irum=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_jldm=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_jlum=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_ildm=0;
		reg_color_sharp_shp_cds_region5_4_reg.cds_region_5_ilum=0;
		y_cb=y_cb+1+z_cds_AIInfo->cb_med12*2;
		y_cr=y_cr+1+z_cds_AIInfo->cr_med12*2;
		y_face_count=y_face_count+1;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_0_reg,reg_color_sharp_shp_cds_region5_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_1_reg,reg_color_sharp_shp_cds_region5_1_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_2_reg,reg_color_sharp_shp_cds_region5_2_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_3_reg,reg_color_sharp_shp_cds_region5_3_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_4_reg,reg_color_sharp_shp_cds_region5_4_reg.regValue);
	}
	else
	{
		color_sharp_shp_cds_region5_0_RBUS reg_color_sharp_shp_cds_region5_0_reg;
		color_sharp_shp_cds_region5_1_RBUS reg_color_sharp_shp_cds_region5_1_reg;
		reg_color_sharp_shp_cds_region5_0_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_0_reg);
		reg_color_sharp_shp_cds_region5_1_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION5_1_reg);
		reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable_5=0;
		reg_color_sharp_shp_cds_region5_0_reg.cds_region_5_start_i=2159;
		reg_color_sharp_shp_cds_region5_0_reg.cds_region_5_end_i=0;
		reg_color_sharp_shp_cds_region5_1_reg.cds_region_5_start_j=3839;
		reg_color_sharp_shp_cds_region5_1_reg.cds_region_5_end_j=0;
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_0_reg,reg_color_sharp_shp_cds_region5_0_reg.regValue);
		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION5_1_reg,reg_color_sharp_shp_cds_region5_1_reg.regValue);
	}
	if(y_face_count>0)
	{
		color_sharp_dm_cds_cm0_u_bound_1_RBUS reg_color_sharp_dm_cds_cm0_u_bound_1_reg;
		reg_color_sharp_dm_cds_cm0_u_bound_1_reg.regValue=IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_1_reg);
		y_cb=y_cb*4/(y_face_count<<3);
		y_cr=y_cr*4/(y_face_count<<3);
		y_cb=y_cb/4;
		y_cr=y_cr/4;
		reg_color_sharp_dm_cds_cm0_u_bound_1_reg.cu_0=y_cb;
		reg_color_sharp_dm_cds_cm0_u_bound_1_reg.cv_0=y_cr;
		IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_1_reg,reg_color_sharp_dm_cds_cm0_u_bound_1_reg.regValue);
	}
	IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg,reg_color_sharp_shp_cds_region_enable_reg.regValue);
}
#endif
// chen 0429
// vip table 1260 set, for face detect
void scaler_AI_obj_Ctrl_Set(DRV_AI_Ctrl_table *ptr) // read from sharing memory
{
        if (ptr==NULL)
                return;

//(1) global setting ///////////////
        ai_ctrl.ai_global.frame_drop_num = ptr->ai_global.frame_drop_num;
        ai_ctrl.ai_global.frame_delay = ptr->ai_global.frame_delay;

        ai_ctrl.ai_global.AI_icm_en = ptr->ai_global.AI_icm_en;
        ai_ctrl.ai_global.AI_dcc_en= ptr->ai_global.AI_dcc_en;
        ai_ctrl.ai_global.AI_sharp_en= ptr->ai_global.AI_sharp_en;

        ai_ctrl.ai_global.sc_count_th= ptr->ai_global.sc_count_th;
        ai_ctrl.ai_global.ratio_max_th= ptr->ai_global.ratio_max_th;
        ai_ctrl.ai_global.range_gain= ptr->ai_global.range_gain;

        ai_ctrl.ai_global.iir_weight= ptr->ai_global.iir_weight;
        ai_ctrl.ai_global.iir_weight2= ptr->ai_global.iir_weight2;
        ai_ctrl.ai_global.iir_weight3= ptr->ai_global.iir_weight3;

        ai_ctrl.ai_global.icm_ai_blend_inside_ratio= ptr->ai_global.icm_ai_blend_inside_ratio;
        ai_ctrl.ai_global.icm_ai_blend_ratio0= ptr->ai_global.icm_ai_blend_ratio0;
        ai_ctrl.ai_global.icm_ai_blend_ratio1= ptr->ai_global.icm_ai_blend_ratio1;
        ai_ctrl.ai_global.icm_ai_blend_ratio2= ptr->ai_global.icm_ai_blend_ratio2;
        ai_ctrl.ai_global.icm_ai_blend_ratio3= ptr->ai_global.icm_ai_blend_ratio3;

        ai_ctrl.ai_global.dcc_ai_blend_inside_ratio= ptr->ai_global.dcc_ai_blend_inside_ratio;
        ai_ctrl.ai_global.dcc_ai_blend_ratio0= ptr->ai_global.dcc_ai_blend_ratio0;
        ai_ctrl.ai_global.dcc_ai_blend_ratio1= ptr->ai_global.dcc_ai_blend_ratio1;
        ai_ctrl.ai_global.dcc_ai_blend_ratio2= ptr->ai_global.dcc_ai_blend_ratio2;
        ai_ctrl.ai_global.dcc_ai_blend_ratio3= ptr->ai_global.dcc_ai_blend_ratio3;

        // chen 0524
        ai_ctrl.ai_global.icm_uv_blend_ratio0=ptr->ai_global.icm_uv_blend_ratio0;
        ai_ctrl.ai_global.icm_uv_blend_ratio1=ptr->ai_global.icm_uv_blend_ratio1;
        ai_ctrl.ai_global.icm_uv_blend_ratio2=ptr->ai_global.icm_uv_blend_ratio2;
        ai_ctrl.ai_global.icm_uv_blend_ratio3=ptr->ai_global.icm_uv_blend_ratio3;
        ai_ctrl.ai_global.icm_uv_blend_ratio4=ptr->ai_global.icm_uv_blend_ratio4;
        ai_ctrl.ai_global.icm_uv_blend_ratio5=ptr->ai_global.icm_uv_blend_ratio5;
        ai_ctrl.ai_global.icm_uv_blend_ratio6=ptr->ai_global.icm_uv_blend_ratio6;
        ai_ctrl.ai_global.icm_uv_blend_ratio7=ptr->ai_global.icm_uv_blend_ratio7;

        //end chen 0524

        // lesley 0718
        ai_ctrl.ai_global.demo_draw_en=ptr->ai_global.demo_draw_en;
        //end lesley 0718


//(2) icm_blending_setting ///////////

        ai_ctrl.ai_icm_blend.d_change_speed_default = ptr->ai_icm_blend.d_change_speed_default;
        ai_ctrl.ai_icm_blend.change_speed_default = ptr->ai_icm_blend.change_speed_default;

	// disappear //////
        ai_ctrl.ai_icm_blend.val_diff_loth = ptr->ai_icm_blend.val_diff_loth;
        ai_ctrl.ai_icm_blend.d_change_speed_val_loth = ptr->ai_icm_blend.d_change_speed_val_loth;
        ai_ctrl.ai_icm_blend.d_change_speed_val_hith = ptr->ai_icm_blend.d_change_speed_val_hith;
        ai_ctrl.ai_icm_blend.d_change_speed_val_slope = ptr->ai_icm_blend.d_change_speed_val_slope;
        ai_ctrl.ai_icm_blend.IOU_diff_loth = ptr->ai_icm_blend.IOU_diff_loth;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth = ptr->ai_icm_blend.d_change_speed_IOU_loth;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith = ptr->ai_icm_blend.d_change_speed_IOU_hith;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope = ptr->ai_icm_blend.d_change_speed_IOU_slope;
        ai_ctrl.ai_icm_blend.IOU_diff_loth2 = ptr->ai_icm_blend.IOU_diff_loth2;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth2 = ptr->ai_icm_blend.d_change_speed_IOU_loth2;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith2 = ptr->ai_icm_blend.d_change_speed_IOU_hith2;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope2 = ptr->ai_icm_blend.d_change_speed_IOU_slope2;
        ai_ctrl.ai_icm_blend.size_diff_loth = ptr->ai_icm_blend.size_diff_loth;
        ai_ctrl.ai_icm_blend.d_change_speed_size_loth = ptr->ai_icm_blend.d_change_speed_size_loth;
        ai_ctrl.ai_icm_blend.d_change_speed_size_hith = ptr->ai_icm_blend.d_change_speed_size_hith;
        ai_ctrl.ai_icm_blend.d_change_speed_size_slope = ptr->ai_icm_blend.d_change_speed_size_slope;



	// appear //////

        ai_ctrl.ai_icm_blend.val_diff_loth_a = ptr->ai_icm_blend.val_diff_loth_a;
        ai_ctrl.ai_icm_blend.d_change_speed_val_loth_a = ptr->ai_icm_blend.d_change_speed_val_loth_a;
        ai_ctrl.ai_icm_blend.d_change_speed_val_hith_a = ptr->ai_icm_blend.d_change_speed_val_hith_a;
        ai_ctrl.ai_icm_blend.d_change_speed_val_slope_a = ptr->ai_icm_blend.d_change_speed_val_slope_a;
        ai_ctrl.ai_icm_blend.IOU_diff_loth_a = ptr->ai_icm_blend.IOU_diff_loth_a;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth_a = ptr->ai_icm_blend.d_change_speed_IOU_loth_a;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith_a = ptr->ai_icm_blend.d_change_speed_IOU_hith_a;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope_a = ptr->ai_icm_blend.d_change_speed_IOU_slope_a;
        ai_ctrl.ai_icm_blend.IOU_diff_loth_a2 = ptr->ai_icm_blend.IOU_diff_loth_a2;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth_a2 = ptr->ai_icm_blend.d_change_speed_IOU_loth_a2;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith_a2 = ptr->ai_icm_blend.d_change_speed_IOU_hith_a2;
        ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope_a2 = ptr->ai_icm_blend.d_change_speed_IOU_slope_a2;
        ai_ctrl.ai_icm_blend.size_diff_loth_a = ptr->ai_icm_blend.size_diff_loth_a;
        ai_ctrl.ai_icm_blend.d_change_speed_size_loth_a = ptr->ai_icm_blend.d_change_speed_size_loth_a;
        ai_ctrl.ai_icm_blend.d_change_speed_size_hith_a = ptr->ai_icm_blend.d_change_speed_size_hith_a;
        ai_ctrl.ai_icm_blend.d_change_speed_size_slope_a = ptr->ai_icm_blend.d_change_speed_size_slope_a;
        ai_ctrl.ai_icm_blend.blend_hith = ptr->ai_icm_blend.blend_hith;


        //(3) dcc_blending_setting ///////////

        ai_ctrl.ai_dcc_blend.d_change_speed_default = ptr->ai_dcc_blend.d_change_speed_default;
        ai_ctrl.ai_dcc_blend.change_speed_default = ptr->ai_dcc_blend.change_speed_default;

	// disappear //////
        ai_ctrl.ai_dcc_blend.val_diff_loth = ptr->ai_dcc_blend.val_diff_loth;
        ai_ctrl.ai_dcc_blend.d_change_speed_val_loth = ptr->ai_dcc_blend.d_change_speed_val_loth;
        ai_ctrl.ai_dcc_blend.d_change_speed_val_hith = ptr->ai_dcc_blend.d_change_speed_val_hith;
        ai_ctrl.ai_dcc_blend.d_change_speed_val_slope = ptr->ai_dcc_blend.d_change_speed_val_slope;
        ai_ctrl.ai_dcc_blend.IOU_diff_loth = ptr->ai_dcc_blend.IOU_diff_loth;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth = ptr->ai_dcc_blend.d_change_speed_IOU_loth;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith = ptr->ai_dcc_blend.d_change_speed_IOU_hith;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope = ptr->ai_dcc_blend.d_change_speed_IOU_slope;
        ai_ctrl.ai_dcc_blend.IOU_diff_loth2 = ptr->ai_dcc_blend.IOU_diff_loth2;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth2 = ptr->ai_dcc_blend.d_change_speed_IOU_loth2;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith2 = ptr->ai_dcc_blend.d_change_speed_IOU_hith2;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope2 = ptr->ai_dcc_blend.d_change_speed_IOU_slope2;
        ai_ctrl.ai_dcc_blend.size_diff_loth = ptr->ai_dcc_blend.size_diff_loth;
        ai_ctrl.ai_dcc_blend.d_change_speed_size_loth = ptr->ai_dcc_blend.d_change_speed_size_loth;
        ai_ctrl.ai_dcc_blend.d_change_speed_size_hith = ptr->ai_dcc_blend.d_change_speed_size_hith;
        ai_ctrl.ai_dcc_blend.d_change_speed_size_slope = ptr->ai_dcc_blend.d_change_speed_size_slope;



        // appear //////
        ai_ctrl.ai_dcc_blend.val_diff_loth_a = ptr->ai_dcc_blend.val_diff_loth_a;
        ai_ctrl.ai_dcc_blend.d_change_speed_val_loth_a = ptr->ai_dcc_blend.d_change_speed_val_loth_a;
        ai_ctrl.ai_dcc_blend.d_change_speed_val_hith_a = ptr->ai_dcc_blend.d_change_speed_val_hith_a;
        ai_ctrl.ai_dcc_blend.d_change_speed_val_slope_a = ptr->ai_dcc_blend.d_change_speed_val_slope_a;
        ai_ctrl.ai_dcc_blend.IOU_diff_loth_a = ptr->ai_dcc_blend.IOU_diff_loth_a;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth_a = ptr->ai_dcc_blend.d_change_speed_IOU_loth_a;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith_a = ptr->ai_dcc_blend.d_change_speed_IOU_hith_a;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope_a = ptr->ai_dcc_blend.d_change_speed_IOU_slope_a;
        ai_ctrl.ai_dcc_blend.IOU_diff_loth_a2 = ptr->ai_dcc_blend.IOU_diff_loth_a2;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth_a2 = ptr->ai_dcc_blend.d_change_speed_IOU_loth_a2;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith_a2 = ptr->ai_dcc_blend.d_change_speed_IOU_hith_a2;
        ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope_a2 = ptr->ai_dcc_blend.d_change_speed_IOU_slope_a2;
        ai_ctrl.ai_dcc_blend.size_diff_loth_a = ptr->ai_dcc_blend.size_diff_loth_a;
        ai_ctrl.ai_dcc_blend.d_change_speed_size_loth_a = ptr->ai_dcc_blend.d_change_speed_size_loth_a;
        ai_ctrl.ai_dcc_blend.d_change_speed_size_hith_a = ptr->ai_dcc_blend.d_change_speed_size_hith_a;
        ai_ctrl.ai_dcc_blend.d_change_speed_size_slope_a = ptr->ai_dcc_blend.d_change_speed_size_slope_a;
        ai_ctrl.ai_dcc_blend.blend_hith = ptr->ai_dcc_blend.blend_hith;

        //(4) sharpness_blending_setting ///////////

        ai_ctrl.ai_sharp_blend.d_change_speed_default = ptr->ai_sharp_blend.d_change_speed_default;
        ai_ctrl.ai_sharp_blend.change_speed_default = ptr->ai_sharp_blend.change_speed_default;

	// disappear //////
        ai_ctrl.ai_sharp_blend.val_diff_loth = ptr->ai_sharp_blend.val_diff_loth;
        ai_ctrl.ai_sharp_blend.d_change_speed_val_loth = ptr->ai_sharp_blend.d_change_speed_val_loth;
        ai_ctrl.ai_sharp_blend.d_change_speed_val_hith = ptr->ai_sharp_blend.d_change_speed_val_hith;
        ai_ctrl.ai_sharp_blend.d_change_speed_val_slope = ptr->ai_sharp_blend.d_change_speed_val_slope;
        ai_ctrl.ai_sharp_blend.IOU_diff_loth = ptr->ai_sharp_blend.IOU_diff_loth;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth = ptr->ai_sharp_blend.d_change_speed_IOU_loth;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith = ptr->ai_sharp_blend.d_change_speed_IOU_hith;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope = ptr->ai_sharp_blend.d_change_speed_IOU_slope;
        ai_ctrl.ai_sharp_blend.IOU_diff_loth2 = ptr->ai_sharp_blend.IOU_diff_loth2;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth2 = ptr->ai_sharp_blend.d_change_speed_IOU_loth2;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith2 = ptr->ai_sharp_blend.d_change_speed_IOU_hith2;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope2 = ptr->ai_sharp_blend.d_change_speed_IOU_slope2;
        ai_ctrl.ai_sharp_blend.size_diff_loth = ptr->ai_sharp_blend.size_diff_loth;
        ai_ctrl.ai_sharp_blend.d_change_speed_size_loth = ptr->ai_sharp_blend.d_change_speed_size_loth;
        ai_ctrl.ai_sharp_blend.d_change_speed_size_hith = ptr->ai_sharp_blend.d_change_speed_size_hith;
        ai_ctrl.ai_sharp_blend.d_change_speed_size_slope = ptr->ai_sharp_blend.d_change_speed_size_slope;

        // appear //////
        ai_ctrl.ai_sharp_blend.val_diff_loth_a = ptr->ai_sharp_blend.val_diff_loth_a;
        ai_ctrl.ai_sharp_blend.d_change_speed_val_loth_a = ptr->ai_sharp_blend.d_change_speed_val_loth_a;
        ai_ctrl.ai_sharp_blend.d_change_speed_val_hith_a = ptr->ai_sharp_blend.d_change_speed_val_hith_a;
        ai_ctrl.ai_sharp_blend.d_change_speed_val_slope_a = ptr->ai_sharp_blend.d_change_speed_val_slope_a;
        ai_ctrl.ai_sharp_blend.IOU_diff_loth_a = ptr->ai_sharp_blend.IOU_diff_loth_a;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth_a = ptr->ai_sharp_blend.d_change_speed_IOU_loth_a;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith_a = ptr->ai_sharp_blend.d_change_speed_IOU_hith_a;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope_a = ptr->ai_sharp_blend.d_change_speed_IOU_slope_a;
        ai_ctrl.ai_sharp_blend.IOU_diff_loth_a2 = ptr->ai_sharp_blend.IOU_diff_loth_a2;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth_a2 = ptr->ai_sharp_blend.d_change_speed_IOU_loth_a2;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith_a2 = ptr->ai_sharp_blend.d_change_speed_IOU_hith_a2;
        ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope_a2 = ptr->ai_sharp_blend.d_change_speed_IOU_slope_a2;
        ai_ctrl.ai_sharp_blend.size_diff_loth_a = ptr->ai_sharp_blend.size_diff_loth_a;
        ai_ctrl.ai_sharp_blend.d_change_speed_size_loth_a = ptr->ai_sharp_blend.d_change_speed_size_loth_a;
        ai_ctrl.ai_sharp_blend.d_change_speed_size_hith_a = ptr->ai_sharp_blend.d_change_speed_size_hith_a;
        ai_ctrl.ai_sharp_blend.d_change_speed_size_slope_a = ptr->ai_sharp_blend.d_change_speed_size_slope_a;
        ai_ctrl.ai_sharp_blend.blend_hith = ptr->ai_sharp_blend.blend_hith;


//(4) icm_tune_setting	///////////
	//ICM dynamic tuning
        ai_ctrl.ai_icm_tune.hue_target_hi1 = ptr->ai_icm_tune.hue_target_hi1;
        ai_ctrl.ai_icm_tune.hue_target_hi2 = ptr->ai_icm_tune.hue_target_hi2;
        ai_ctrl.ai_icm_tune.hue_target_hi3 = ptr->ai_icm_tune.hue_target_hi3;
        ai_ctrl.ai_icm_tune.hue_target_lo1 = ptr->ai_icm_tune.hue_target_lo1;
        ai_ctrl.ai_icm_tune.hue_target_lo2 = ptr->ai_icm_tune.hue_target_lo2;
        ai_ctrl.ai_icm_tune.hue_target_lo3 = ptr->ai_icm_tune.hue_target_lo3;
        ai_ctrl.ai_icm_tune.sat_target_hi1 = ptr->ai_icm_tune.sat_target_hi1;
        ai_ctrl.ai_icm_tune.sat_target_hi2 = ptr->ai_icm_tune.sat_target_hi2;
        ai_ctrl.ai_icm_tune.sat_target_hi3 = ptr->ai_icm_tune.sat_target_hi3;
        ai_ctrl.ai_icm_tune.sat_target_lo1 = ptr->ai_icm_tune.sat_target_lo1;
        ai_ctrl.ai_icm_tune.sat_target_lo2 = ptr->ai_icm_tune.sat_target_lo2;
        ai_ctrl.ai_icm_tune.sat_target_lo3 = ptr->ai_icm_tune.sat_target_lo3;
        ai_ctrl.ai_icm_tune.s_adj_th_p = ptr->ai_icm_tune.s_adj_th_p;
        ai_ctrl.ai_icm_tune.s_adj_th_n = ptr->ai_icm_tune.s_adj_th_n;
        ai_ctrl.ai_icm_tune.h_adj_th_p = ptr->ai_icm_tune.h_adj_th_p;
        ai_ctrl.ai_icm_tune.h_adj_th_n = ptr->ai_icm_tune.h_adj_th_n;
        ai_ctrl.ai_icm_tune.sat3x3_gain = ptr->ai_icm_tune.sat3x3_gain;
        ai_ctrl.ai_icm_tune.bri_3x3_delta = ptr->ai_icm_tune.bri_3x3_delta;

	// chen 0528
	ai_ctrl.ai_icm_tune.icm_table_nouse = ptr->ai_icm_tune.icm_table_nouse;
	ai_ctrl.ai_icm_tune.icm_sat_hith_nomax = ptr->ai_icm_tune.icm_sat_hith_nomax;
  //end chen 0528

//(5) info_setting //////////////
	// face0
        ai_ctrl.ai_info_manul_0.AIPQ_dy_en = ptr->ai_info_manul_0.AIPQ_dy_en;
        ai_ctrl.ai_info_manul_0.info_manual_en = ptr->ai_info_manul_0.info_manual_en;
        ai_ctrl.ai_info_manul_0.scene_change = ptr->ai_info_manul_0.scene_change;
        ai_ctrl.ai_info_manul_0.pv8 = ptr->ai_info_manul_0.pv8;
        ai_ctrl.ai_info_manul_0.cx12 = ptr->ai_info_manul_0.cx12;
        ai_ctrl.ai_info_manul_0.cy12 = ptr->ai_info_manul_0.cy12;
        ai_ctrl.ai_info_manul_0.w12 = ptr->ai_info_manul_0.w12;
        ai_ctrl.ai_info_manul_0.h12 = ptr->ai_info_manul_0.h12;
        ai_ctrl.ai_info_manul_0.range12 = ptr->ai_info_manul_0.range12;
        ai_ctrl.ai_info_manul_0.cb_med12 = ptr->ai_info_manul_0.cb_med12;
        ai_ctrl.ai_info_manul_0.cr_med12 = ptr->ai_info_manul_0.cr_med12;
        ai_ctrl.ai_info_manul_0.hue_med12 = ptr->ai_info_manul_0.hue_med12;
        ai_ctrl.ai_info_manul_0.sat_med12 = ptr->ai_info_manul_0.sat_med12;
        ai_ctrl.ai_info_manul_0.val_med12 = ptr->ai_info_manul_0.val_med12;

	// face1
        ai_ctrl.ai_info_manul_1.AIPQ_dy_en = ptr->ai_info_manul_1.AIPQ_dy_en;
        ai_ctrl.ai_info_manul_1.info_manual_en = ptr->ai_info_manul_1.info_manual_en;
        ai_ctrl.ai_info_manul_1.scene_change = ptr->ai_info_manul_1.scene_change;
        ai_ctrl.ai_info_manul_1.pv8 = ptr->ai_info_manul_1.pv8;
        ai_ctrl.ai_info_manul_1.cx12 = ptr->ai_info_manul_1.cx12;
        ai_ctrl.ai_info_manul_1.cy12 = ptr->ai_info_manul_1.cy12;
        ai_ctrl.ai_info_manul_1.w12 = ptr->ai_info_manul_1.w12;
        ai_ctrl.ai_info_manul_1.h12 = ptr->ai_info_manul_1.h12;
        ai_ctrl.ai_info_manul_1.range12 = ptr->ai_info_manul_1.range12;
        ai_ctrl.ai_info_manul_1.cb_med12 = ptr->ai_info_manul_1.cb_med12;
        ai_ctrl.ai_info_manul_1.cr_med12 = ptr->ai_info_manul_1.cr_med12;
        ai_ctrl.ai_info_manul_1.hue_med12 = ptr->ai_info_manul_1.hue_med12;
        ai_ctrl.ai_info_manul_1.sat_med12 = ptr->ai_info_manul_1.sat_med12;
        ai_ctrl.ai_info_manul_1.val_med12 = ptr->ai_info_manul_1.val_med12;

	// face2
        ai_ctrl.ai_info_manul_2.AIPQ_dy_en = ptr->ai_info_manul_2.AIPQ_dy_en;
        ai_ctrl.ai_info_manul_2.info_manual_en = ptr->ai_info_manul_2.info_manual_en;
        ai_ctrl.ai_info_manul_2.scene_change = ptr->ai_info_manul_2.scene_change;
        ai_ctrl.ai_info_manul_2.pv8 = ptr->ai_info_manul_2.pv8;
        ai_ctrl.ai_info_manul_2.cx12 = ptr->ai_info_manul_2.cx12;
        ai_ctrl.ai_info_manul_2.cy12 = ptr->ai_info_manul_2.cy12;
        ai_ctrl.ai_info_manul_2.w12 = ptr->ai_info_manul_2.w12;
        ai_ctrl.ai_info_manul_2.h12 = ptr->ai_info_manul_2.h12;
        ai_ctrl.ai_info_manul_2.range12 = ptr->ai_info_manul_2.range12;
        ai_ctrl.ai_info_manul_2.cb_med12 = ptr->ai_info_manul_2.cb_med12;
        ai_ctrl.ai_info_manul_2.cr_med12 = ptr->ai_info_manul_2.cr_med12;
        ai_ctrl.ai_info_manul_2.hue_med12 = ptr->ai_info_manul_2.hue_med12;
        ai_ctrl.ai_info_manul_2.sat_med12 = ptr->ai_info_manul_2.sat_med12;
        ai_ctrl.ai_info_manul_2.val_med12 = ptr->ai_info_manul_2.val_med12;
	// face3
        ai_ctrl.ai_info_manul_3.AIPQ_dy_en = ptr->ai_info_manul_3.AIPQ_dy_en;
        ai_ctrl.ai_info_manul_3.info_manual_en = ptr->ai_info_manul_3.info_manual_en;
        ai_ctrl.ai_info_manul_3.scene_change = ptr->ai_info_manul_3.scene_change;
        ai_ctrl.ai_info_manul_3.pv8 = ptr->ai_info_manul_3.pv8;
        ai_ctrl.ai_info_manul_3.cx12 = ptr->ai_info_manul_3.cx12;
        ai_ctrl.ai_info_manul_3.cy12 = ptr->ai_info_manul_3.cy12;
        ai_ctrl.ai_info_manul_3.w12 = ptr->ai_info_manul_3.w12;
        ai_ctrl.ai_info_manul_3.h12 = ptr->ai_info_manul_3.h12;
        ai_ctrl.ai_info_manul_3.range12 = ptr->ai_info_manul_3.range12;
        ai_ctrl.ai_info_manul_3.cb_med12 = ptr->ai_info_manul_3.cb_med12;
        ai_ctrl.ai_info_manul_3.cr_med12 = ptr->ai_info_manul_3.cr_med12;
        ai_ctrl.ai_info_manul_3.hue_med12 = ptr->ai_info_manul_3.hue_med12;
        ai_ctrl.ai_info_manul_3.sat_med12 = ptr->ai_info_manul_3.sat_med12;
        ai_ctrl.ai_info_manul_3.val_med12 = ptr->ai_info_manul_3.val_med12;
	// face4
        ai_ctrl.ai_info_manul_4.AIPQ_dy_en = ptr->ai_info_manul_4.AIPQ_dy_en;
        ai_ctrl.ai_info_manul_4.info_manual_en = ptr->ai_info_manul_4.info_manual_en;
        ai_ctrl.ai_info_manul_4.scene_change = ptr->ai_info_manul_4.scene_change;
        ai_ctrl.ai_info_manul_4.pv8 = ptr->ai_info_manul_4.pv8;
        ai_ctrl.ai_info_manul_4.cx12 = ptr->ai_info_manul_4.cx12;
        ai_ctrl.ai_info_manul_4.cy12 = ptr->ai_info_manul_4.cy12;
        ai_ctrl.ai_info_manul_4.w12 = ptr->ai_info_manul_4.w12;
        ai_ctrl.ai_info_manul_4.h12 = ptr->ai_info_manul_4.h12;
        ai_ctrl.ai_info_manul_4.range12 = ptr->ai_info_manul_4.range12;
        ai_ctrl.ai_info_manul_4.cb_med12 = ptr->ai_info_manul_4.cb_med12;
        ai_ctrl.ai_info_manul_4.cr_med12 = ptr->ai_info_manul_4.cr_med12;
        ai_ctrl.ai_info_manul_4.hue_med12 = ptr->ai_info_manul_4.hue_med12;
        ai_ctrl.ai_info_manul_4.sat_med12 = ptr->ai_info_manul_4.sat_med12;
        ai_ctrl.ai_info_manul_4.val_med12 = ptr->ai_info_manul_4.val_med12;
	// face5
        ai_ctrl.ai_info_manul_5.AIPQ_dy_en = ptr->ai_info_manul_5.AIPQ_dy_en;
        ai_ctrl.ai_info_manul_5.info_manual_en = ptr->ai_info_manul_5.info_manual_en;
        ai_ctrl.ai_info_manul_5.scene_change = ptr->ai_info_manul_5.scene_change;
        ai_ctrl.ai_info_manul_5.pv8 = ptr->ai_info_manul_5.pv8;
        ai_ctrl.ai_info_manul_5.cx12 = ptr->ai_info_manul_5.cx12;
        ai_ctrl.ai_info_manul_5.cy12 = ptr->ai_info_manul_5.cy12;
        ai_ctrl.ai_info_manul_5.w12 = ptr->ai_info_manul_5.w12;
        ai_ctrl.ai_info_manul_5.h12 = ptr->ai_info_manul_5.h12;
        ai_ctrl.ai_info_manul_5.range12 = ptr->ai_info_manul_5.range12;
        ai_ctrl.ai_info_manul_5.cb_med12 = ptr->ai_info_manul_5.cb_med12;
        ai_ctrl.ai_info_manul_5.cr_med12 = ptr->ai_info_manul_5.cr_med12;
        ai_ctrl.ai_info_manul_5.hue_med12 = ptr->ai_info_manul_5.hue_med12;
        ai_ctrl.ai_info_manul_5.sat_med12 = ptr->ai_info_manul_5.sat_med12;
        ai_ctrl.ai_info_manul_5.val_med12 = ptr->ai_info_manul_5.val_med12;

// lesley 0808
//(11) ICM dynamic tuning2
        ai_ctrl.ai_icm_tune2.val_target_hi2_ratio = ptr->ai_icm_tune2.val_target_hi2_ratio;
        ai_ctrl.ai_icm_tune2.val_target_hi1 = ptr->ai_icm_tune2.val_target_hi1;
        ai_ctrl.ai_icm_tune2.val_target_lo1 = ptr->ai_icm_tune2.val_target_lo1;
        ai_ctrl.ai_icm_tune2.val_target_lo2_ratio = ptr->ai_icm_tune2.val_target_lo2_ratio;
        ai_ctrl.ai_icm_tune2.v_adj_th_max_p = ptr->ai_icm_tune2.v_adj_th_max_p;
 		ai_ctrl.ai_icm_tune2.v_adj_th_max_n = ptr->ai_icm_tune2.v_adj_th_max_n;

		// lesley 1007
		ai_ctrl.ai_icm_tune2.hue_target_hi2_ratio = ptr->ai_icm_tune2.hue_target_hi2_ratio;
		ai_ctrl.ai_icm_tune2.hue_target_lo2_ratio = ptr->ai_icm_tune2.hue_target_lo2_ratio;
		ai_ctrl.ai_icm_tune2.sat_target_hi2_ratio = ptr->ai_icm_tune2.sat_target_hi2_ratio;
		ai_ctrl.ai_icm_tune2.sat_target_lo2_ratio = ptr->ai_icm_tune2.sat_target_lo2_ratio;
		// end lesley 1007

        ai_ctrl.ai_icm_tune2.h_adj_th_p_norm = ptr->ai_icm_tune2.h_adj_th_p_norm;
 		ai_ctrl.ai_icm_tune2.h_adj_th_n_norm = ptr->ai_icm_tune2.h_adj_th_n_norm;
        ai_ctrl.ai_icm_tune2.s_adj_th_p_norm = ptr->ai_icm_tune2.s_adj_th_p_norm;
 		ai_ctrl.ai_icm_tune2.s_adj_th_n_norm = ptr->ai_icm_tune2.s_adj_th_n_norm;
        ai_ctrl.ai_icm_tune2.v_adj_th_p_norm = ptr->ai_icm_tune2.v_adj_th_p_norm;
 		ai_ctrl.ai_icm_tune2.v_adj_th_n_norm = ptr->ai_icm_tune2.v_adj_th_n_norm;


        ai_ctrl.ai_icm_tune2.h_adj_step = ptr->ai_icm_tune2.h_adj_step;
        ai_ctrl.ai_icm_tune2.s_adj_step = ptr->ai_icm_tune2.s_adj_step;
		ai_ctrl.ai_icm_tune2.v_adj_step = ptr->ai_icm_tune2.v_adj_step;

		// lesley 0821
        ai_ctrl.ai_icm_tune2.icm_global_en = ptr->ai_icm_tune2.icm_global_en;
		// end lesley 0821

		// lesley 0822
        ai_ctrl.ai_icm_tune2.center_uv_step = ptr->ai_icm_tune2.center_uv_step;
		// end lesley 0822


		// lesley 0826
		ai_ctrl.ai_icm_tune2.keep_gray_mode = ptr->ai_icm_tune2.keep_gray_mode;
		ai_ctrl.ai_icm_tune2.uv_range0_lo = ptr->ai_icm_tune2.uv_range0_lo;
		ai_ctrl.ai_icm_tune2.uv_range0_up = ptr->ai_icm_tune2.uv_range0_up;
		// end lesley 0826

		// lesley 0902
	    ai_ctrl.ai_icm_tune2.center_u_init = ptr->ai_icm_tune2.center_u_init;
	    ai_ctrl.ai_icm_tune2.center_v_init = ptr->ai_icm_tune2.center_v_init;
		ai_ctrl.ai_icm_tune2.center_u_lo = ptr->ai_icm_tune2.center_u_lo;
		ai_ctrl.ai_icm_tune2.center_u_up = ptr->ai_icm_tune2.center_u_up;
		ai_ctrl.ai_icm_tune2.center_v_lo = ptr->ai_icm_tune2.center_v_lo;
		ai_ctrl.ai_icm_tune2.center_v_up = ptr->ai_icm_tune2.center_v_up;
		// end lesley 0902



//(12) global2 setting (for dcc)
        ai_ctrl.ai_global2.dcc_uv_blend_ratio0=ptr->ai_global2.dcc_uv_blend_ratio0;
        ai_ctrl.ai_global2.dcc_uv_blend_ratio1=ptr->ai_global2.dcc_uv_blend_ratio1;
        ai_ctrl.ai_global2.dcc_uv_blend_ratio2=ptr->ai_global2.dcc_uv_blend_ratio2;
        ai_ctrl.ai_global2.dcc_uv_blend_ratio3=ptr->ai_global2.dcc_uv_blend_ratio3;
        ai_ctrl.ai_global2.dcc_uv_blend_ratio4=ptr->ai_global2.dcc_uv_blend_ratio4;
        ai_ctrl.ai_global2.dcc_uv_blend_ratio5=ptr->ai_global2.dcc_uv_blend_ratio5;
        ai_ctrl.ai_global2.dcc_uv_blend_ratio6=ptr->ai_global2.dcc_uv_blend_ratio6;
        ai_ctrl.ai_global2.dcc_uv_blend_ratio7=ptr->ai_global2.dcc_uv_blend_ratio7;

        ai_ctrl.ai_global2.dcc_global_en=ptr->ai_global2.dcc_global_en;
        ai_ctrl.ai_global2.dcc_old_skin_en = ptr->ai_global2.dcc_old_skin_en;

		// lesley 0904
        ai_ctrl.ai_global2.dcc_old_skin_y_range = ptr->ai_global2.dcc_old_skin_y_range;
        ai_ctrl.ai_global2.dcc_old_skin_u_range = ptr->ai_global2.dcc_old_skin_u_range;
        ai_ctrl.ai_global2.dcc_old_skin_v_range = ptr->ai_global2.dcc_old_skin_v_range;
        ai_ctrl.ai_global2.dcc_keep_gray_mode = ptr->ai_global2.dcc_keep_gray_mode;
		// end lesley 0904

		// lesley 0910
        ai_ctrl.ai_global2.dcc_enhance_en = ptr->ai_global2.dcc_enhance_en;
		// end lesley 0910

        ai_ctrl.ai_global2.center_y_step = ptr->ai_global2.center_y_step;
        ai_ctrl.ai_global2.center_uv_step = ptr->ai_global2.center_uv_step;
//(13) shp tuning
		// lesley 0814
        ai_ctrl.ai_shp_tune.edg_gain_level = ptr->ai_shp_tune.edg_gain_level;
        ai_ctrl.ai_shp_tune.tex_gain_level = ptr->ai_shp_tune.tex_gain_level;
        ai_ctrl.ai_shp_tune.vpk_gain_level = ptr->ai_shp_tune.vpk_gain_level;
        ai_ctrl.ai_shp_tune.vpk_edg_gain_level = ptr->ai_shp_tune.vpk_edg_gain_level;
		// end lesley 0814

		// chen 0815_2
        ai_ctrl.ai_shp_tune.AI_face_sharp_dynamic_en = ptr->ai_shp_tune.AI_face_sharp_dynamic_en;
		// end chen 0815_2

		// lesley 0815
        ai_ctrl.ai_shp_tune.AI_face_sharp_mode = ptr->ai_shp_tune.AI_face_sharp_mode;
		// end lesley 0815

		// yush 190815
        ai_ctrl.ai_shp_tune.shp_face_adjust_en = ptr->ai_shp_tune.shp_face_adjust_en;
		// end yush 190815

		// lesley 0819
        ai_ctrl.ai_shp_tune.edg_lv = ptr->ai_shp_tune.edg_lv;
        ai_ctrl.ai_shp_tune.tex_lv = ptr->ai_shp_tune.tex_lv;
        ai_ctrl.ai_shp_tune.vpk_lv = ptr->ai_shp_tune.vpk_lv;
        ai_ctrl.ai_shp_tune.vpk_edg_lv = ptr->ai_shp_tune.vpk_edg_lv;
        ai_ctrl.ai_shp_tune.shp_gain_en = ptr->ai_shp_tune.shp_gain_en;
        ai_ctrl.ai_shp_tune.shp_lv_en = ptr->ai_shp_tune.shp_lv_en;
		// end lesley 0819

		// lesley 0910
        ai_ctrl.ai_shp_tune.edg_gain_neg_level = ptr->ai_shp_tune.edg_gain_neg_level;
        ai_ctrl.ai_shp_tune.tex_gain_neg_level = ptr->ai_shp_tune.tex_gain_neg_level;
        ai_ctrl.ai_shp_tune.vpk_gain_neg_level = ptr->ai_shp_tune.vpk_gain_neg_level;
        ai_ctrl.ai_shp_tune.vpk_edg_gain_neg_level = ptr->ai_shp_tune.vpk_edg_gain_neg_level;
		// end lesley 0910

// (14) global3 setting
        ai_ctrl.ai_global3.ai_sc_y_diff_th = ptr->ai_global3.ai_sc_y_diff_th;
        ai_ctrl.ai_global3.ai_sc_count_th = ptr->ai_global3.ai_sc_count_th;
		// chen 0808
        ai_ctrl.ai_global3.IOU_select = ptr->ai_global3.IOU_select;
        ai_ctrl.ai_global3.sum_count_num = ptr->ai_global3.sum_count_num;
		// end chen 0808

		// chen 0812
        ai_ctrl.ai_global3.disappear_between_faces_new_en = ptr->ai_global3.disappear_between_faces_new_en;
		// end chen 0812

		// lesley 0813
        ai_ctrl.ai_global3.debug_face_info_mode = ptr->ai_global3.debug_face_info_mode;
		// end lesley 0813

		// chen 0815_2
        ai_ctrl.ai_global3.sc_y_diff_th = ptr->ai_global3.sc_y_diff_th;
		// end chen 0815_2

		// lesley 0820_2
        ai_ctrl.ai_global3.IOU_decay_en = ptr->ai_global3.IOU_decay_en;
        ai_ctrl.ai_global3.IOU_decay = ptr->ai_global3.IOU_decay;
		// end lesley 0820_2

		// lesley 0823
        ai_ctrl.ai_global3.blend_size_en = ptr->ai_global3.blend_size_en;
        ai_ctrl.ai_global3.blend_size_loth = ptr->ai_global3.blend_size_loth;
        ai_ctrl.ai_global3.blend_size_hith = ptr->ai_global3.blend_size_hith;
		// end lesley 0823

		// lesley 0829
        ai_ctrl.ai_global3.keep_still_mode = ptr->ai_global3.keep_still_mode;
        ai_ctrl.ai_global3.still_ratio_th = ptr->ai_global3.still_ratio_th;
        ai_ctrl.ai_global3.still_ratio_th1 = ptr->ai_global3.still_ratio_th1;
        ai_ctrl.ai_global3.still_ratio_th2 = ptr->ai_global3.still_ratio_th2;
        ai_ctrl.ai_global3.still_ratio_th3 = ptr->ai_global3.still_ratio_th3;
        ai_ctrl.ai_global3.still_ratio_clamp = ptr->ai_global3.still_ratio_clamp;
		// end lesley 0829

		// lesley 0904
        ai_ctrl.ai_global3.scene_change_en = ptr->ai_global3.scene_change_en;
		// end lesley 0904

		// lesley 0906_2
		ai_ctrl.ai_global3.ai_sc_y_diff_th1 = ptr->ai_global3.ai_sc_y_diff_th1;
		ai_ctrl.ai_global3.ai_sc_hue_ratio_th = ptr->ai_global3.ai_sc_hue_ratio_th;
		ai_ctrl.ai_global3.ai_sc_hue_ratio_th1 = ptr->ai_global3.ai_sc_hue_ratio_th1;
		// end lesley 0906_2

		// lesley 0920
		ai_ctrl.ai_global3.signal_cnt_th = ptr->ai_global3.signal_cnt_th;
		// end lesley 0920

		// lesley 0928
		ai_ctrl.ai_global3.IOU2_mode = ptr->ai_global3.IOU2_mode;
		ai_ctrl.ai_global3.IOU2_mode1_offset = ptr->ai_global3.IOU2_mode1_offset;
		ai_ctrl.ai_global3.IOU2_mode1_range_gain = ptr->ai_global3.IOU2_mode1_range_gain;
		ai_ctrl.ai_global3.IOU2_mode1_range_ratio = ptr->ai_global3.IOU2_mode1_range_ratio;
		// end lesley 0928

		ai_ctrl.ai_global3.draw_blend_en = ptr->ai_global3.draw_blend_en;
		ai_ctrl.ai_global3.apply_delay = ptr->ai_global3.apply_delay;

		ai_ctrl.ai_global3.ip_isr_ctrl = ptr->ai_global3.ip_isr_ctrl;
// end lesley 0808
}
// end chen 0429

// vip table 1260 get, for face detect
void scaler_AI_obj_Ctrl_Get(DRV_AI_Ctrl_table *ptr) // read from sharing memory
{
        if (ptr==NULL) return;

		memset(ptr, 0, sizeof(DRV_AI_Ctrl_table));

//(1) global setting ///////////////
        ptr->ai_global.frame_drop_num =ai_ctrl.ai_global.frame_drop_num;
        ptr->ai_global.frame_delay =ai_ctrl.ai_global.frame_delay;

        ptr->ai_global.AI_icm_en = ai_ctrl.ai_global.AI_icm_en;
        ptr->ai_global.AI_dcc_en = ai_ctrl.ai_global.AI_dcc_en;
        ptr->ai_global.AI_sharp_en = ai_ctrl.ai_global.AI_sharp_en;

        ptr->ai_global.sc_count_th = ai_ctrl.ai_global.sc_count_th;
        ptr->ai_global.ratio_max_th = ai_ctrl.ai_global.ratio_max_th;
        ptr->ai_global.range_gain = ai_ctrl.ai_global.range_gain;

        ptr->ai_global.iir_weight = ai_ctrl.ai_global.iir_weight;
        ptr->ai_global.iir_weight2 = ai_ctrl.ai_global.iir_weight2;
        ptr->ai_global.iir_weight3 = ai_ctrl.ai_global.iir_weight3;

        ptr->ai_global.icm_ai_blend_inside_ratio = ai_ctrl.ai_global.icm_ai_blend_inside_ratio;
        ptr->ai_global.icm_ai_blend_ratio0 = ai_ctrl.ai_global.icm_ai_blend_ratio0;
        ptr->ai_global.icm_ai_blend_ratio1 = ai_ctrl.ai_global.icm_ai_blend_ratio1;
        ptr->ai_global.icm_ai_blend_ratio2 = ai_ctrl.ai_global.icm_ai_blend_ratio2;
        ptr->ai_global.icm_ai_blend_ratio3 = ai_ctrl.ai_global.icm_ai_blend_ratio3;

        ptr->ai_global.dcc_ai_blend_inside_ratio = ai_ctrl.ai_global.dcc_ai_blend_inside_ratio;
        ptr->ai_global.dcc_ai_blend_ratio0 = ai_ctrl.ai_global.dcc_ai_blend_ratio0;
        ptr->ai_global.dcc_ai_blend_ratio1 = ai_ctrl.ai_global.dcc_ai_blend_ratio1;
        ptr->ai_global.dcc_ai_blend_ratio2 = ai_ctrl.ai_global.dcc_ai_blend_ratio2;
        ptr->ai_global.dcc_ai_blend_ratio3 = ai_ctrl.ai_global.dcc_ai_blend_ratio3;


        // chen 0524
        ptr->ai_global.icm_uv_blend_ratio0=ai_ctrl.ai_global.icm_uv_blend_ratio0;
        ptr->ai_global.icm_uv_blend_ratio1=ai_ctrl.ai_global.icm_uv_blend_ratio1;
        ptr->ai_global.icm_uv_blend_ratio2=ai_ctrl.ai_global.icm_uv_blend_ratio2;
        ptr->ai_global.icm_uv_blend_ratio3=ai_ctrl.ai_global.icm_uv_blend_ratio3;
        ptr->ai_global.icm_uv_blend_ratio4=ai_ctrl.ai_global.icm_uv_blend_ratio4;
        ptr->ai_global.icm_uv_blend_ratio5=ai_ctrl.ai_global.icm_uv_blend_ratio5;
        ptr->ai_global.icm_uv_blend_ratio6=ai_ctrl.ai_global.icm_uv_blend_ratio6;
        ptr->ai_global.icm_uv_blend_ratio7=ai_ctrl.ai_global.icm_uv_blend_ratio7;
        //end chen 0524

        // lesley 0718
        ptr->ai_global.demo_draw_en=ai_ctrl.ai_global.demo_draw_en;
        //end lesley 0718


//(2) icm_blending_setting ///////////

        ptr->ai_icm_blend.d_change_speed_default = ai_ctrl.ai_icm_blend.d_change_speed_default;
        ptr->ai_icm_blend.change_speed_default = ai_ctrl.ai_icm_blend.change_speed_default;

	// disappear //////
        ptr->ai_icm_blend.val_diff_loth = ai_ctrl.ai_icm_blend.val_diff_loth;
        ptr->ai_icm_blend.d_change_speed_val_loth = ai_ctrl.ai_icm_blend.d_change_speed_val_loth;
        ptr->ai_icm_blend.d_change_speed_val_hith = ai_ctrl.ai_icm_blend.d_change_speed_val_hith;
        ptr->ai_icm_blend.d_change_speed_val_slope = ai_ctrl.ai_icm_blend.d_change_speed_val_slope;
        ptr->ai_icm_blend.IOU_diff_loth = ai_ctrl.ai_icm_blend.IOU_diff_loth;
        ptr->ai_icm_blend.d_change_speed_IOU_loth = ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth;
        ptr->ai_icm_blend.d_change_speed_IOU_hith = ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith;
        ptr->ai_icm_blend.d_change_speed_IOU_slope = ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope;
        ptr->ai_icm_blend.IOU_diff_loth2 = ai_ctrl.ai_icm_blend.IOU_diff_loth2;
        ptr->ai_icm_blend.d_change_speed_IOU_loth2 = ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth2;
        ptr->ai_icm_blend.d_change_speed_IOU_hith2 = ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith2;
        ptr->ai_icm_blend.d_change_speed_IOU_slope2 = ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope2;
        ptr->ai_icm_blend.size_diff_loth = ai_ctrl.ai_icm_blend.size_diff_loth;
        ptr->ai_icm_blend.d_change_speed_size_loth = ai_ctrl.ai_icm_blend.d_change_speed_size_loth;
        ptr->ai_icm_blend.d_change_speed_size_hith = ai_ctrl.ai_icm_blend.d_change_speed_size_hith;
        ptr->ai_icm_blend.d_change_speed_size_slope = ai_ctrl.ai_icm_blend.d_change_speed_size_slope;



	// appear //////

        ptr->ai_icm_blend.val_diff_loth_a = ai_ctrl.ai_icm_blend.val_diff_loth_a;
        ptr->ai_icm_blend.d_change_speed_val_loth_a = ai_ctrl.ai_icm_blend.d_change_speed_val_loth_a;
        ptr->ai_icm_blend.d_change_speed_val_hith_a = ai_ctrl.ai_icm_blend.d_change_speed_val_hith_a;
        ptr->ai_icm_blend.d_change_speed_val_slope_a = ai_ctrl.ai_icm_blend.d_change_speed_val_slope_a;
        ptr->ai_icm_blend.IOU_diff_loth_a = ai_ctrl.ai_icm_blend.IOU_diff_loth_a;
        ptr->ai_icm_blend.d_change_speed_IOU_loth_a = ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth_a;
        ptr->ai_icm_blend.d_change_speed_IOU_hith_a = ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith_a;
        ptr->ai_icm_blend.d_change_speed_IOU_slope_a = ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope_a;
        ptr->ai_icm_blend.IOU_diff_loth_a2 = ai_ctrl.ai_icm_blend.IOU_diff_loth_a2;
        ptr->ai_icm_blend.d_change_speed_IOU_loth_a2 = ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth_a2;
        ptr->ai_icm_blend.d_change_speed_IOU_hith_a2 = ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith_a2;
        ptr->ai_icm_blend.d_change_speed_IOU_slope_a2 = ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope_a2;
        ptr->ai_icm_blend.size_diff_loth_a = ai_ctrl.ai_icm_blend.size_diff_loth_a;
        ptr->ai_icm_blend.d_change_speed_size_loth_a = ai_ctrl.ai_icm_blend.d_change_speed_size_loth_a;
        ptr->ai_icm_blend.d_change_speed_size_hith_a = ai_ctrl.ai_icm_blend.d_change_speed_size_hith_a;
        ptr->ai_icm_blend.d_change_speed_size_slope_a = ai_ctrl.ai_icm_blend.d_change_speed_size_slope_a;
        ptr->ai_icm_blend.blend_hith = ai_ctrl.ai_icm_blend.blend_hith;



        //(3) dcc_blending_setting ///////////

        ptr->ai_dcc_blend.d_change_speed_default = ai_ctrl.ai_dcc_blend.d_change_speed_default;
        ptr->ai_dcc_blend.change_speed_default = ai_ctrl.ai_dcc_blend.change_speed_default;

	// disappear //////
        ptr->ai_dcc_blend.val_diff_loth = ai_ctrl.ai_dcc_blend.val_diff_loth;
        ptr->ai_dcc_blend.d_change_speed_val_loth = ai_ctrl.ai_dcc_blend.d_change_speed_val_loth;
        ptr->ai_dcc_blend.d_change_speed_val_hith = ai_ctrl.ai_dcc_blend.d_change_speed_val_hith;
        ptr->ai_dcc_blend.d_change_speed_val_slope = ai_ctrl.ai_dcc_blend.d_change_speed_val_slope;
        ptr->ai_dcc_blend.IOU_diff_loth = ai_ctrl.ai_dcc_blend.IOU_diff_loth;
        ptr->ai_dcc_blend.d_change_speed_IOU_loth = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth;
        ptr->ai_dcc_blend.d_change_speed_IOU_hith = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith;
        ptr->ai_dcc_blend.d_change_speed_IOU_slope = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope;
        ptr->ai_dcc_blend.IOU_diff_loth2 = ai_ctrl.ai_dcc_blend.IOU_diff_loth2;
        ptr->ai_dcc_blend.d_change_speed_IOU_loth2 = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth2;
        ptr->ai_dcc_blend.d_change_speed_IOU_hith2 = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith2;
        ptr->ai_dcc_blend.d_change_speed_IOU_slope2 = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope2;
        ptr->ai_dcc_blend.size_diff_loth = ai_ctrl.ai_dcc_blend.size_diff_loth;
        ptr->ai_dcc_blend.d_change_speed_size_loth = ai_ctrl.ai_dcc_blend.d_change_speed_size_loth;
        ptr->ai_dcc_blend.d_change_speed_size_hith = ai_ctrl.ai_dcc_blend.d_change_speed_size_hith;
        ptr->ai_dcc_blend.d_change_speed_size_slope = ai_ctrl.ai_dcc_blend.d_change_speed_size_slope;



	// appear //////

        ptr->ai_dcc_blend.val_diff_loth_a = ai_ctrl.ai_dcc_blend.val_diff_loth_a;
        ptr->ai_dcc_blend.d_change_speed_val_loth_a = ai_ctrl.ai_dcc_blend.d_change_speed_val_loth_a;
        ptr->ai_dcc_blend.d_change_speed_val_hith_a = ai_ctrl.ai_dcc_blend.d_change_speed_val_hith_a;
        ptr->ai_dcc_blend.d_change_speed_val_slope_a = ai_ctrl.ai_dcc_blend.d_change_speed_val_slope_a;
        ptr->ai_dcc_blend.IOU_diff_loth_a = ai_ctrl.ai_dcc_blend.IOU_diff_loth_a;
        ptr->ai_dcc_blend.d_change_speed_IOU_loth_a = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth_a;
        ptr->ai_dcc_blend.d_change_speed_IOU_hith_a = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith_a;
        ptr->ai_dcc_blend.d_change_speed_IOU_slope_a = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope_a;
        ptr->ai_dcc_blend.IOU_diff_loth_a2 = ai_ctrl.ai_dcc_blend.IOU_diff_loth_a2;
        ptr->ai_dcc_blend.d_change_speed_IOU_loth_a2 = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth_a2;
        ptr->ai_dcc_blend.d_change_speed_IOU_hith_a2 = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith_a2;
        ptr->ai_dcc_blend.d_change_speed_IOU_slope_a2 = ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope_a2;
        ptr->ai_dcc_blend.size_diff_loth_a = ai_ctrl.ai_dcc_blend.size_diff_loth_a;
        ptr->ai_dcc_blend.d_change_speed_size_loth_a = ai_ctrl.ai_dcc_blend.d_change_speed_size_loth_a;
        ptr->ai_dcc_blend.d_change_speed_size_hith_a = ai_ctrl.ai_dcc_blend.d_change_speed_size_hith_a;
        ptr->ai_dcc_blend.d_change_speed_size_slope_a = ai_ctrl.ai_dcc_blend.d_change_speed_size_slope_a;
        ptr->ai_dcc_blend.blend_hith = ai_ctrl.ai_dcc_blend.blend_hith;


        //(4) sharpness_blending_setting ///////////

        ptr->ai_sharp_blend.d_change_speed_default = ai_ctrl.ai_sharp_blend.d_change_speed_default;
        ptr->ai_sharp_blend.change_speed_default = ai_ctrl.ai_sharp_blend.change_speed_default;

	// disappear //////
        ptr->ai_sharp_blend.val_diff_loth = ai_ctrl.ai_sharp_blend.val_diff_loth;
        ptr->ai_sharp_blend.d_change_speed_val_loth = ai_ctrl.ai_sharp_blend.d_change_speed_val_loth;
        ptr->ai_sharp_blend.d_change_speed_val_hith = ai_ctrl.ai_sharp_blend.d_change_speed_val_hith;
        ptr->ai_sharp_blend.d_change_speed_val_slope = ai_ctrl.ai_sharp_blend.d_change_speed_val_slope;
        ptr->ai_sharp_blend.IOU_diff_loth = ai_ctrl.ai_sharp_blend.IOU_diff_loth;
        ptr->ai_sharp_blend.d_change_speed_IOU_loth = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth;
        ptr->ai_sharp_blend.d_change_speed_IOU_hith = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith;
        ptr->ai_sharp_blend.d_change_speed_IOU_slope = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope;
        ptr->ai_sharp_blend.IOU_diff_loth2 = ai_ctrl.ai_sharp_blend.IOU_diff_loth2;
        ptr->ai_sharp_blend.d_change_speed_IOU_loth2 = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth2;
        ptr->ai_sharp_blend.d_change_speed_IOU_hith2 = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith2;
        ptr->ai_sharp_blend.d_change_speed_IOU_slope2 = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope2;
        ptr->ai_sharp_blend.size_diff_loth = ai_ctrl.ai_sharp_blend.size_diff_loth;
        ptr->ai_sharp_blend.d_change_speed_size_loth = ai_ctrl.ai_sharp_blend.d_change_speed_size_loth;
        ptr->ai_sharp_blend.d_change_speed_size_hith = ai_ctrl.ai_sharp_blend.d_change_speed_size_hith;
        ptr->ai_sharp_blend.d_change_speed_size_slope = ai_ctrl.ai_sharp_blend.d_change_speed_size_slope;


	// appear //////

        ptr->ai_sharp_blend.val_diff_loth_a = ai_ctrl.ai_sharp_blend.val_diff_loth_a;
        ptr->ai_sharp_blend.d_change_speed_val_loth_a = ai_ctrl.ai_sharp_blend.d_change_speed_val_loth_a;
        ptr->ai_sharp_blend.d_change_speed_val_hith_a = ai_ctrl.ai_sharp_blend.d_change_speed_val_hith_a;
        ptr->ai_sharp_blend.d_change_speed_val_slope_a = ai_ctrl.ai_sharp_blend.d_change_speed_val_slope_a;
        ptr->ai_sharp_blend.IOU_diff_loth_a = ai_ctrl.ai_sharp_blend.IOU_diff_loth_a;
        ptr->ai_sharp_blend.d_change_speed_IOU_loth_a = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth_a;
        ptr->ai_sharp_blend.d_change_speed_IOU_hith_a = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith_a;
        ptr->ai_sharp_blend.d_change_speed_IOU_slope_a = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope_a;
        ptr->ai_sharp_blend.IOU_diff_loth_a2 = ai_ctrl.ai_sharp_blend.IOU_diff_loth_a2;
        ptr->ai_sharp_blend.d_change_speed_IOU_loth_a2 = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth_a2;
        ptr->ai_sharp_blend.d_change_speed_IOU_hith_a2 = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith_a2;
        ptr->ai_sharp_blend.d_change_speed_IOU_slope_a2 = ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope_a2;
        ptr->ai_sharp_blend.size_diff_loth_a = ai_ctrl.ai_sharp_blend.size_diff_loth_a;
        ptr->ai_sharp_blend.d_change_speed_size_loth_a = ai_ctrl.ai_sharp_blend.d_change_speed_size_loth_a;
        ptr->ai_sharp_blend.d_change_speed_size_hith_a = ai_ctrl.ai_sharp_blend.d_change_speed_size_hith_a;
        ptr->ai_sharp_blend.d_change_speed_size_slope_a = ai_ctrl.ai_sharp_blend.d_change_speed_size_slope_a;
        ptr->ai_sharp_blend.blend_hith = ai_ctrl.ai_sharp_blend.blend_hith;



//(4) icm_tune_setting	///////////
	//ICM dynamic tuning
        ptr->ai_icm_tune.hue_target_hi1 = ai_ctrl.ai_icm_tune.hue_target_hi1;
        ptr->ai_icm_tune.hue_target_hi2 = ai_ctrl.ai_icm_tune.hue_target_hi2;
        ptr->ai_icm_tune.hue_target_hi3 = ai_ctrl.ai_icm_tune.hue_target_hi3;
        ptr->ai_icm_tune.hue_target_lo1 = ai_ctrl.ai_icm_tune.hue_target_lo1;
        ptr->ai_icm_tune.hue_target_lo2 = ai_ctrl.ai_icm_tune.hue_target_lo2;
        ptr->ai_icm_tune.hue_target_lo3 = ai_ctrl.ai_icm_tune.hue_target_lo3;
        ptr->ai_icm_tune.sat_target_hi1 = ai_ctrl.ai_icm_tune.sat_target_hi1;
        ptr->ai_icm_tune.sat_target_hi2 = ai_ctrl.ai_icm_tune.sat_target_hi2;
        ptr->ai_icm_tune.sat_target_hi3 = ai_ctrl.ai_icm_tune.sat_target_hi3;
        ptr->ai_icm_tune.sat_target_lo1 = ai_ctrl.ai_icm_tune.sat_target_lo1;
        ptr->ai_icm_tune.sat_target_lo2 = ai_ctrl.ai_icm_tune.sat_target_lo2;
        ptr->ai_icm_tune.sat_target_lo3 = ai_ctrl.ai_icm_tune.sat_target_lo3;

        ptr->ai_icm_tune.s_adj_th_p = ai_ctrl.ai_icm_tune.s_adj_th_p;
        ptr->ai_icm_tune.s_adj_th_n = ai_ctrl.ai_icm_tune.s_adj_th_n;
        ptr->ai_icm_tune.h_adj_th_p = ai_ctrl.ai_icm_tune.h_adj_th_p;
        ptr->ai_icm_tune.h_adj_th_n = ai_ctrl.ai_icm_tune.h_adj_th_n;
        ptr->ai_icm_tune.sat3x3_gain = ai_ctrl.ai_icm_tune.sat3x3_gain;
        ptr->ai_icm_tune.bri_3x3_delta = ai_ctrl.ai_icm_tune.bri_3x3_delta;

	  // chen 0528
	 ptr->ai_icm_tune.icm_table_nouse = ai_ctrl.ai_icm_tune.icm_table_nouse;
	 ptr->ai_icm_tune.icm_sat_hith_nomax = ai_ctrl.ai_icm_tune.icm_sat_hith_nomax;
 	//end chen 0528



//(5) info_setting //////////////
	// face0
        ptr->ai_info_manul_0.AIPQ_dy_en = ai_ctrl.ai_info_manul_0.AIPQ_dy_en ;
        ptr->ai_info_manul_0.info_manual_en = ai_ctrl.ai_info_manul_0.info_manual_en;
        ptr->ai_info_manul_0.scene_change = ai_ctrl.ai_info_manul_0.scene_change;
        ptr->ai_info_manul_0.pv8 = ai_ctrl.ai_info_manul_0.pv8;
        ptr->ai_info_manul_0.cx12 = ai_ctrl.ai_info_manul_0.cx12;
        ptr->ai_info_manul_0.cy12 = ai_ctrl.ai_info_manul_0.cy12;
        ptr->ai_info_manul_0.w12 = ai_ctrl.ai_info_manul_0.w12;
        ptr->ai_info_manul_0.h12 = ai_ctrl.ai_info_manul_0.h12;
        ptr->ai_info_manul_0.range12 = ai_ctrl.ai_info_manul_0.range12;
        ptr->ai_info_manul_0.cb_med12 = ai_ctrl.ai_info_manul_0.cb_med12;
        ptr->ai_info_manul_0.cr_med12 = ai_ctrl.ai_info_manul_0.cr_med12;
        ptr->ai_info_manul_0.hue_med12 = ai_ctrl.ai_info_manul_0.hue_med12;
        ptr->ai_info_manul_0.sat_med12 = ai_ctrl.ai_info_manul_0.sat_med12;
        ptr->ai_info_manul_0.val_med12 = ai_ctrl.ai_info_manul_0.val_med12;

	// face1
        ptr->ai_info_manul_1.AIPQ_dy_en = ai_ctrl.ai_info_manul_1.AIPQ_dy_en ;
        ptr->ai_info_manul_1.info_manual_en = ai_ctrl.ai_info_manul_1.info_manual_en;
        ptr->ai_info_manul_1.scene_change = ai_ctrl.ai_info_manul_1.scene_change;
        ptr->ai_info_manul_1.pv8 = ai_ctrl.ai_info_manul_1.pv8;
        ptr->ai_info_manul_1.cx12 = ai_ctrl.ai_info_manul_1.cx12;
        ptr->ai_info_manul_1.cy12 = ai_ctrl.ai_info_manul_1.cy12;
        ptr->ai_info_manul_1.w12 = ai_ctrl.ai_info_manul_1.w12;
        ptr->ai_info_manul_1.h12 = ai_ctrl.ai_info_manul_1.h12;
        ptr->ai_info_manul_1.range12 = ai_ctrl.ai_info_manul_1.range12;
        ptr->ai_info_manul_1.cb_med12 = ai_ctrl.ai_info_manul_1.cb_med12;
        ptr->ai_info_manul_1.cr_med12 = ai_ctrl.ai_info_manul_1.cr_med12;
        ptr->ai_info_manul_1.hue_med12 = ai_ctrl.ai_info_manul_1.hue_med12;
        ptr->ai_info_manul_1.sat_med12 = ai_ctrl.ai_info_manul_1.sat_med12;
        ptr->ai_info_manul_1.val_med12 = ai_ctrl.ai_info_manul_1.val_med12;


	// face2
        ptr->ai_info_manul_2.AIPQ_dy_en = ai_ctrl.ai_info_manul_2.AIPQ_dy_en ;
        ptr->ai_info_manul_2.info_manual_en = ai_ctrl.ai_info_manul_2.info_manual_en;
        ptr->ai_info_manul_2.scene_change = ai_ctrl.ai_info_manul_2.scene_change;
        ptr->ai_info_manul_2.pv8 = ai_ctrl.ai_info_manul_2.pv8;
        ptr->ai_info_manul_2.cx12 = ai_ctrl.ai_info_manul_2.cx12;
        ptr->ai_info_manul_2.cy12 = ai_ctrl.ai_info_manul_2.cy12;
        ptr->ai_info_manul_2.w12 = ai_ctrl.ai_info_manul_2.w12;
        ptr->ai_info_manul_2.h12 = ai_ctrl.ai_info_manul_2.h12;
        ptr->ai_info_manul_2.range12 = ai_ctrl.ai_info_manul_2.range12;
        ptr->ai_info_manul_2.cb_med12 = ai_ctrl.ai_info_manul_2.cb_med12;
        ptr->ai_info_manul_2.cr_med12 = ai_ctrl.ai_info_manul_2.cr_med12;
        ptr->ai_info_manul_2.hue_med12 = ai_ctrl.ai_info_manul_2.hue_med12;
        ptr->ai_info_manul_2.sat_med12 = ai_ctrl.ai_info_manul_2.sat_med12;
        ptr->ai_info_manul_2.val_med12 = ai_ctrl.ai_info_manul_2.val_med12;
	// face3
        ptr->ai_info_manul_3.AIPQ_dy_en = ai_ctrl.ai_info_manul_3.AIPQ_dy_en ;
        ptr->ai_info_manul_3.info_manual_en = ai_ctrl.ai_info_manul_3.info_manual_en;
        ptr->ai_info_manul_3.scene_change = ai_ctrl.ai_info_manul_3.scene_change;
        ptr->ai_info_manul_3.pv8 = ai_ctrl.ai_info_manul_3.pv8;
        ptr->ai_info_manul_3.cx12 = ai_ctrl.ai_info_manul_3.cx12;
        ptr->ai_info_manul_3.cy12 = ai_ctrl.ai_info_manul_3.cy12;
        ptr->ai_info_manul_3.w12 = ai_ctrl.ai_info_manul_3.w12;
        ptr->ai_info_manul_3.h12 = ai_ctrl.ai_info_manul_3.h12;
        ptr->ai_info_manul_3.range12 = ai_ctrl.ai_info_manul_3.range12;
        ptr->ai_info_manul_3.cb_med12 = ai_ctrl.ai_info_manul_3.cb_med12;
        ptr->ai_info_manul_3.cr_med12 = ai_ctrl.ai_info_manul_3.cr_med12;
        ptr->ai_info_manul_3.hue_med12 = ai_ctrl.ai_info_manul_3.hue_med12;
        ptr->ai_info_manul_3.sat_med12 = ai_ctrl.ai_info_manul_3.sat_med12;
        ptr->ai_info_manul_3.val_med12 = ai_ctrl.ai_info_manul_3.val_med12;
	// face4
        ptr->ai_info_manul_4.AIPQ_dy_en = ai_ctrl.ai_info_manul_4.AIPQ_dy_en ;
        ptr->ai_info_manul_4.info_manual_en = ai_ctrl.ai_info_manul_4.info_manual_en;
        ptr->ai_info_manul_4.scene_change = ai_ctrl.ai_info_manul_4.scene_change;
        ptr->ai_info_manul_4.pv8 = ai_ctrl.ai_info_manul_4.pv8;
        ptr->ai_info_manul_4.cx12 = ai_ctrl.ai_info_manul_4.cx12;
        ptr->ai_info_manul_4.cy12 = ai_ctrl.ai_info_manul_4.cy12;
        ptr->ai_info_manul_4.w12 = ai_ctrl.ai_info_manul_4.w12;
        ptr->ai_info_manul_4.h12 = ai_ctrl.ai_info_manul_4.h12;
        ptr->ai_info_manul_4.range12 = ai_ctrl.ai_info_manul_4.range12;
        ptr->ai_info_manul_4.cb_med12 = ai_ctrl.ai_info_manul_4.cb_med12;
        ptr->ai_info_manul_4.cr_med12 = ai_ctrl.ai_info_manul_4.cr_med12;
        ptr->ai_info_manul_4.hue_med12 = ai_ctrl.ai_info_manul_4.hue_med12;
        ptr->ai_info_manul_4.sat_med12 = ai_ctrl.ai_info_manul_4.sat_med12;
        ptr->ai_info_manul_4.val_med12 = ai_ctrl.ai_info_manul_4.val_med12;
	// face5
        ptr->ai_info_manul_5.AIPQ_dy_en = ai_ctrl.ai_info_manul_5.AIPQ_dy_en ;
        ptr->ai_info_manul_5.info_manual_en = ai_ctrl.ai_info_manul_5.info_manual_en;
        ptr->ai_info_manul_5.scene_change = ai_ctrl.ai_info_manul_5.scene_change;
        ptr->ai_info_manul_5.pv8 = ai_ctrl.ai_info_manul_5.pv8;
        ptr->ai_info_manul_5.cx12 = ai_ctrl.ai_info_manul_5.cx12;
        ptr->ai_info_manul_5.cy12 = ai_ctrl.ai_info_manul_5.cy12;
        ptr->ai_info_manul_5.w12 = ai_ctrl.ai_info_manul_5.w12;
        ptr->ai_info_manul_5.h12 = ai_ctrl.ai_info_manul_5.h12;
        ptr->ai_info_manul_5.range12 = ai_ctrl.ai_info_manul_5.range12;
        ptr->ai_info_manul_5.cb_med12 = ai_ctrl.ai_info_manul_5.cb_med12;
        ptr->ai_info_manul_5.cr_med12 = ai_ctrl.ai_info_manul_5.cr_med12;
        ptr->ai_info_manul_5.hue_med12 = ai_ctrl.ai_info_manul_5.hue_med12;
        ptr->ai_info_manul_5.sat_med12 = ai_ctrl.ai_info_manul_5.sat_med12;
        ptr->ai_info_manul_5.val_med12 = ai_ctrl.ai_info_manul_5.val_med12;

// lesley 0808
//(11) ICM dynamic tuning
        ptr->ai_icm_tune2.val_target_lo2_ratio = ai_ctrl.ai_icm_tune2.val_target_lo2_ratio;
        ptr->ai_icm_tune2.val_target_lo1 = ai_ctrl.ai_icm_tune2.val_target_lo1;
        ptr->ai_icm_tune2.val_target_hi1 = ai_ctrl.ai_icm_tune2.val_target_hi1;
        ptr->ai_icm_tune2.val_target_hi2_ratio = ai_ctrl.ai_icm_tune2.val_target_hi2_ratio;
        ptr->ai_icm_tune2.v_adj_th_max_p = ai_ctrl.ai_icm_tune2.v_adj_th_max_p;
        ptr->ai_icm_tune2.v_adj_th_max_n = ai_ctrl.ai_icm_tune2.v_adj_th_max_n;

		// lesley 1007
		ptr->ai_icm_tune2.hue_target_hi2_ratio = ai_ctrl.ai_icm_tune2.hue_target_hi2_ratio;
		ptr->ai_icm_tune2.hue_target_lo2_ratio = ai_ctrl.ai_icm_tune2.hue_target_lo2_ratio;
		ptr->ai_icm_tune2.sat_target_hi2_ratio = ai_ctrl.ai_icm_tune2.sat_target_hi2_ratio;
		ptr->ai_icm_tune2.sat_target_lo2_ratio = ai_ctrl.ai_icm_tune2.sat_target_lo2_ratio;
		// end lesley 1007

        ptr->ai_icm_tune2.h_adj_th_p_norm = ai_ctrl.ai_icm_tune2.h_adj_th_p_norm;
 		ptr->ai_icm_tune2.h_adj_th_n_norm = ai_ctrl.ai_icm_tune2.h_adj_th_n_norm;
        ptr->ai_icm_tune2.s_adj_th_p_norm = ai_ctrl.ai_icm_tune2.s_adj_th_p_norm;
 		ptr->ai_icm_tune2.s_adj_th_n_norm = ai_ctrl.ai_icm_tune2.s_adj_th_n_norm;
        ptr->ai_icm_tune2.v_adj_th_p_norm = ai_ctrl.ai_icm_tune2.v_adj_th_p_norm;
 		ptr->ai_icm_tune2.v_adj_th_n_norm = ai_ctrl.ai_icm_tune2.v_adj_th_n_norm;

        ptr->ai_icm_tune2.h_adj_step = ai_ctrl.ai_icm_tune2.h_adj_step;
        ptr->ai_icm_tune2.s_adj_step = ai_ctrl.ai_icm_tune2.s_adj_step;
		ptr->ai_icm_tune2.v_adj_step = ai_ctrl.ai_icm_tune2.v_adj_step;

		// lesley 0821
        ptr->ai_icm_tune2.icm_global_en = ai_ctrl.ai_icm_tune2.icm_global_en;
		// end lesley 0821

		// lesley 0822
        ptr->ai_icm_tune2.center_uv_step = ai_ctrl.ai_icm_tune2.center_uv_step;
		// end lesley 0822

		// lesley 0826
		ptr->ai_icm_tune2.keep_gray_mode = ai_ctrl.ai_icm_tune2.keep_gray_mode;
		ptr->ai_icm_tune2.uv_range0_lo = ai_ctrl.ai_icm_tune2.uv_range0_lo;
		ptr->ai_icm_tune2.uv_range0_up = ai_ctrl.ai_icm_tune2.uv_range0_up;
		// end lesley 0826

		// lesley 0902
	    ptr->ai_icm_tune2.center_u_init = ai_ctrl.ai_icm_tune2.center_u_init;
	    ptr->ai_icm_tune2.center_v_init = ai_ctrl.ai_icm_tune2.center_v_init;
		ptr->ai_icm_tune2.center_u_lo = ai_ctrl.ai_icm_tune2.center_u_lo;
		ptr->ai_icm_tune2.center_u_up = ai_ctrl.ai_icm_tune2.center_u_up;
		ptr->ai_icm_tune2.center_v_lo = ai_ctrl.ai_icm_tune2.center_v_lo;
		ptr->ai_icm_tune2.center_v_up = ai_ctrl.ai_icm_tune2.center_v_up;
		// end lesley 0902

//(12) dcc setting
        ptr->ai_global2.dcc_uv_blend_ratio0=ai_ctrl.ai_global2.dcc_uv_blend_ratio0;
        ptr->ai_global2.dcc_uv_blend_ratio1=ai_ctrl.ai_global2.dcc_uv_blend_ratio1;
        ptr->ai_global2.dcc_uv_blend_ratio2=ai_ctrl.ai_global2.dcc_uv_blend_ratio2;
        ptr->ai_global2.dcc_uv_blend_ratio3=ai_ctrl.ai_global2.dcc_uv_blend_ratio3;
        ptr->ai_global2.dcc_uv_blend_ratio4=ai_ctrl.ai_global2.dcc_uv_blend_ratio4;
        ptr->ai_global2.dcc_uv_blend_ratio5=ai_ctrl.ai_global2.dcc_uv_blend_ratio5;
        ptr->ai_global2.dcc_uv_blend_ratio6=ai_ctrl.ai_global2.dcc_uv_blend_ratio6;
        ptr->ai_global2.dcc_uv_blend_ratio7=ai_ctrl.ai_global2.dcc_uv_blend_ratio7;

        ptr->ai_global2.dcc_global_en = ai_ctrl.ai_global2.dcc_global_en;
        ptr->ai_global2.dcc_old_skin_en = ai_ctrl.ai_global2.dcc_old_skin_en;

		// lesley 0904
        ptr->ai_global2.dcc_old_skin_y_range = ai_ctrl.ai_global2.dcc_old_skin_y_range;
        ptr->ai_global2.dcc_old_skin_u_range = ai_ctrl.ai_global2.dcc_old_skin_u_range;
        ptr->ai_global2.dcc_old_skin_v_range = ai_ctrl.ai_global2.dcc_old_skin_v_range;
        ptr->ai_global2.dcc_keep_gray_mode = ai_ctrl.ai_global2.dcc_keep_gray_mode;
		// end lesley 0904

		// lesley 0910
        ptr->ai_global2.dcc_enhance_en = ai_ctrl.ai_global2.dcc_enhance_en;
		// end lesley 0910

        ptr->ai_global2.center_y_step = ai_ctrl.ai_global2.center_y_step;
        ptr->ai_global2.center_uv_step = ai_ctrl.ai_global2.center_uv_step;
//(13) shp tuning
		// lesley 0814
        ptr->ai_shp_tune.edg_gain_level = ai_ctrl.ai_shp_tune.edg_gain_level;
        ptr->ai_shp_tune.tex_gain_level = ai_ctrl.ai_shp_tune.tex_gain_level;
        ptr->ai_shp_tune.vpk_gain_level = ai_ctrl.ai_shp_tune.vpk_gain_level;
        ptr->ai_shp_tune.vpk_edg_gain_level = ai_ctrl.ai_shp_tune.vpk_edg_gain_level;
		// end lesley 0814

		// chen 0815_2
        ptr->ai_shp_tune.AI_face_sharp_dynamic_en = ai_ctrl.ai_shp_tune.AI_face_sharp_dynamic_en;
		// end chen 0815_2

		// lesley 0815
        ptr->ai_shp_tune.AI_face_sharp_mode = ai_ctrl.ai_shp_tune.AI_face_sharp_mode;
		// end lesley 0815

		// yush 190815
        ptr->ai_shp_tune.shp_face_adjust_en = ai_ctrl.ai_shp_tune.shp_face_adjust_en;
		// end yush 0815

		// lesley 0819
        ptr->ai_shp_tune.edg_lv = ai_ctrl.ai_shp_tune.edg_lv;
        ptr->ai_shp_tune.tex_lv = ai_ctrl.ai_shp_tune.tex_lv;
        ptr->ai_shp_tune.vpk_lv = ai_ctrl.ai_shp_tune.vpk_lv;
        ptr->ai_shp_tune.vpk_edg_lv = ai_ctrl.ai_shp_tune.vpk_edg_lv;
        ptr->ai_shp_tune.shp_gain_en = ai_ctrl.ai_shp_tune.shp_gain_en;
        ptr->ai_shp_tune.shp_lv_en = ai_ctrl.ai_shp_tune.shp_lv_en;
		// end lesley 0819

		// lesley 0910
        ptr->ai_shp_tune.edg_gain_neg_level = ai_ctrl.ai_shp_tune.edg_gain_neg_level;
        ptr->ai_shp_tune.tex_gain_neg_level = ai_ctrl.ai_shp_tune.tex_gain_neg_level;
        ptr->ai_shp_tune.vpk_gain_neg_level = ai_ctrl.ai_shp_tune.vpk_gain_neg_level;
        ptr->ai_shp_tune.vpk_edg_gain_neg_level = ai_ctrl.ai_shp_tune.vpk_edg_gain_neg_level;
		// end lesley 0910

// (14) global3 setting
        ptr->ai_global3.ai_sc_y_diff_th = ai_ctrl.ai_global3.ai_sc_y_diff_th;
        ptr->ai_global3.ai_sc_count_th = ai_ctrl.ai_global3.ai_sc_count_th;
		// chen 0808
        ptr->ai_global3.IOU_select = ai_ctrl.ai_global3.IOU_select;
        ptr->ai_global3.sum_count_num = ai_ctrl.ai_global3.sum_count_num;
		// end chen 0808

		// chen 0812
        ptr->ai_global3.disappear_between_faces_new_en = ai_ctrl.ai_global3.disappear_between_faces_new_en;
		// end chen 0812

		// lesley 0813
        ptr->ai_global3.debug_face_info_mode = ai_ctrl.ai_global3.debug_face_info_mode;
		// end lesley 0813

		// chen 0815_2
        ptr->ai_global3.sc_y_diff_th = ai_ctrl.ai_global3.sc_y_diff_th;
		// end chen 0815_2

		// lesley 0820_2
        ptr->ai_global3.IOU_decay_en = ai_ctrl.ai_global3.IOU_decay_en;
        ptr->ai_global3.IOU_decay = ai_ctrl.ai_global3.IOU_decay;
		// end lesley 0820_2

		// lesley 0823
        ptr->ai_global3.blend_size_en = ai_ctrl.ai_global3.blend_size_en;
        ptr->ai_global3.blend_size_loth = ai_ctrl.ai_global3.blend_size_loth;
        ptr->ai_global3.blend_size_hith = ai_ctrl.ai_global3.blend_size_hith;
		// end lesley 0823

		// lesley 0829
        ptr->ai_global3.keep_still_mode = ai_ctrl.ai_global3.keep_still_mode;
        ptr->ai_global3.still_ratio_th = ai_ctrl.ai_global3.still_ratio_th;
        ptr->ai_global3.still_ratio_th1 = ai_ctrl.ai_global3.still_ratio_th1;
        ptr->ai_global3.still_ratio_th2 = ai_ctrl.ai_global3.still_ratio_th2;
        ptr->ai_global3.still_ratio_th3 = ai_ctrl.ai_global3.still_ratio_th3;
        ptr->ai_global3.still_ratio_clamp = ai_ctrl.ai_global3.still_ratio_clamp;
		// end lesley 0829

		// lesley 0904
        ptr->ai_global3.scene_change_en = ai_ctrl.ai_global3.scene_change_en;
		// end lesley 0904

		// lesley 0906_2
		ptr->ai_global3.ai_sc_y_diff_th1 = ai_ctrl.ai_global3.ai_sc_y_diff_th1;
		ptr->ai_global3.ai_sc_hue_ratio_th = ai_ctrl.ai_global3.ai_sc_hue_ratio_th;
		ptr->ai_global3.ai_sc_hue_ratio_th1 = ai_ctrl.ai_global3.ai_sc_hue_ratio_th1;
		// end lesley 0906_2
		
		// lesley 0920
		ptr->ai_global3.signal_cnt_th = ai_ctrl.ai_global3.signal_cnt_th;
		// end lesley 0920

		// lesley 0928
		ptr->ai_global3.IOU2_mode = ai_ctrl.ai_global3.IOU2_mode;
		ptr->ai_global3.IOU2_mode1_offset = ai_ctrl.ai_global3.IOU2_mode1_offset;
		ptr->ai_global3.IOU2_mode1_range_gain = ai_ctrl.ai_global3.IOU2_mode1_range_gain;
		ptr->ai_global3.IOU2_mode1_range_ratio = ai_ctrl.ai_global3.IOU2_mode1_range_ratio;
		// end lesley 0928

		ptr->ai_global3.draw_blend_en = ai_ctrl.ai_global3.draw_blend_en;
		ptr->ai_global3.apply_delay = ai_ctrl.ai_global3.apply_delay;

		ptr->ai_global3.ip_isr_ctrl = ai_ctrl.ai_global3.ip_isr_ctrl;
// end lesley 0808
}

// lesley 0808
// for face PQ set shp cds cm0
void drvif_color_AI_Ctrl_shp(void)
{
	/*Set Edge Gain*/
	color_sharp_dm_segpk_edgpk3_RBUS color_sharp_dm_segpk_edgpk3;
	color_sharp_dm_segpk_edgpk4_RBUS color_sharp_dm_segpk_edgpk4;
	/*Set Texture Gain*/
	color_sharp_dm_peaking_gain_RBUS color_sharp_dm_peaking_gain_reg;
	color_sharp_dm_peaking_bound_1_RBUS color_sharp_dm_peaking_bound_1_reg;
	/* vertical gain*/
	color_sharp_dm_segpk_vpk2_RBUS color_sharp_dm_segpk_vpk2;
	color_sharp_dm_segpk_vpk3_RBUS color_sharp_dm_segpk_vpk3;
	
	/* vertical edge gain*/
	color_sharp_dm_segpk_vpk5_RBUS color_sharp_dm_segpk_vpk5;
	color_sharp_dm_segpk_vpk6_RBUS color_sharp_dm_segpk_vpk6;

	/* cm0 */
	color_sharp_dm_cds_peaking_gain_cm0_tex_RBUS	color_sharp_dm_cds_peaking_gain_cm0_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm0_edge_RBUS	color_sharp_dm_cds_peaking_gain_cm0_edge_reg;
	color_sharp_dm_cds_peaking_gain_cm0_v_RBUS		color_sharp_dm_cds_peaking_gain_cm0_v_reg;
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_RBUS	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg;	

	color_sharp_dm_cds_peaking_bound_cm0_tex_RBUS	color_sharp_dm_cds_peaking_bound_cm0_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm0_edge_RBUS	color_sharp_dm_cds_peaking_bound_cm0_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm0_v_RBUS		color_sharp_dm_cds_peaking_bound_cm0_v_reg;
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_RBUS	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg;

	color_sharp_shp_cds_predict_RBUS reg_color_sharp_shp_cds_predict_reg;

	/* */
	short edg_gain_pos, edg_gain_neg, edg_lv, edg_hv_pos, edg_hv_neg;
	short tex_gain_pos, tex_gain_neg, tex_lv, tex_hv_pos, tex_hv_neg;
	short vpk_gain_pos, vpk_gain_neg, vpk_lv, vpk_hv_pos, vpk_hv_neg;
	short vpk_edg_gain_pos, vpk_edg_gain_neg, vpk_edg_lv, vpk_edg_hv_pos, vpk_edg_hv_neg;
	unsigned char blendw;

	int edg_gain_level, tex_gain_level, vpk_gain_level, vpk_edg_gain_level;
	int edg_gain_neg_level, tex_gain_neg_level, vpk_gain_neg_level, vpk_edg_gain_neg_level;
	int edg_lv_cm0, tex_lv_cm0, v_lv_cm0, v_edg_lv_cm0, shp_gain_en, shp_lv_en;

	edg_gain_level = ai_ctrl.ai_shp_tune.edg_gain_level;
	tex_gain_level = ai_ctrl.ai_shp_tune.tex_gain_level;
	vpk_gain_level = ai_ctrl.ai_shp_tune.vpk_gain_level;
	vpk_edg_gain_level = ai_ctrl.ai_shp_tune.vpk_edg_gain_level;

	edg_gain_neg_level = ai_ctrl.ai_shp_tune.edg_gain_neg_level;
	tex_gain_neg_level = ai_ctrl.ai_shp_tune.tex_gain_neg_level;
	vpk_gain_neg_level = ai_ctrl.ai_shp_tune.vpk_gain_neg_level;
	vpk_edg_gain_neg_level = ai_ctrl.ai_shp_tune.vpk_edg_gain_neg_level;

	edg_lv_cm0 = ai_ctrl.ai_shp_tune.edg_lv;
	tex_lv_cm0 = ai_ctrl.ai_shp_tune.tex_lv;
	v_lv_cm0 = ai_ctrl.ai_shp_tune.vpk_lv;
	v_edg_lv_cm0 = ai_ctrl.ai_shp_tune.vpk_edg_lv;
	shp_gain_en = ai_ctrl.ai_shp_tune.shp_gain_en;
	shp_lv_en = ai_ctrl.ai_shp_tune.shp_lv_en;

	/*Set Edge Gain*/
	color_sharp_dm_segpk_edgpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);
	edg_gain_pos = color_sharp_dm_segpk_edgpk3.gain_pos;
	edg_gain_neg = color_sharp_dm_segpk_edgpk3.gain_neg;
	color_sharp_dm_segpk_edgpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg);
	edg_lv = color_sharp_dm_segpk_edgpk4.lv;
	edg_hv_pos = color_sharp_dm_segpk_edgpk4.hv_pos;
	edg_hv_neg = color_sharp_dm_segpk_edgpk4.hv_neg;

	/*Set Texture Gain*/
	color_sharp_dm_peaking_gain_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_GAIN_reg);
	tex_gain_pos = color_sharp_dm_peaking_gain_reg.gain_pos;
	tex_gain_neg = color_sharp_dm_peaking_gain_reg.gain_neg;
	color_sharp_dm_peaking_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_PEAKING_BOUND_1_reg);
	tex_lv = color_sharp_dm_peaking_bound_1_reg.lv;
	tex_hv_pos = color_sharp_dm_peaking_bound_1_reg.hv_pos;
	tex_hv_neg = color_sharp_dm_peaking_bound_1_reg.hv_neg;

	/* vertical gain*/
	color_sharp_dm_segpk_vpk2.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK2_reg);
	vpk_gain_pos = color_sharp_dm_segpk_vpk2.vpk_gain_pos;
	vpk_gain_neg = color_sharp_dm_segpk_vpk2.vpk_gain_neg;
	vpk_lv = color_sharp_dm_segpk_vpk2.vpk_lv;
	color_sharp_dm_segpk_vpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK3_reg);
	vpk_hv_pos = color_sharp_dm_segpk_vpk3.vpk_hv_pos;
	vpk_hv_neg = color_sharp_dm_segpk_vpk3.vpk_hv_neg;

	/* vertical edge gain*/
	color_sharp_dm_segpk_vpk5.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK5_reg);
	vpk_edg_gain_pos = color_sharp_dm_segpk_vpk5.vpk_edg_gain_pos;
	vpk_edg_gain_neg = color_sharp_dm_segpk_vpk5.vpk_edg_gain_neg;
	vpk_edg_lv = color_sharp_dm_segpk_vpk5.vpk_edg_lv;
	color_sharp_dm_segpk_vpk6.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK6_reg);
	vpk_edg_hv_pos = color_sharp_dm_segpk_vpk6.vpk_edg_hv_pos;
	vpk_edg_hv_neg = color_sharp_dm_segpk_vpk6.vpk_edg_hv_neg;
	
	/* cm0 */
	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue	= IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue	= IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg);
	color_sharp_dm_cds_peaking_gain_cm0_v_reg.regValue		= IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_reg);
	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.regValue	= IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_EDG_reg);
	
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue	= IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue	= IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm0_v_reg.regValue		= IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_reg);
	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.regValue	= IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_EDG_reg);

	reg_color_sharp_shp_cds_predict_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_PREDICT_reg);	
	blendw = (reg_color_sharp_shp_cds_predict_reg.cds_region_predict_sel)?(reg_color_sharp_shp_cds_predict_reg.cds_region_predict_1):(reg_color_sharp_shp_cds_predict_reg.cds_region_predict_2);

	if(shp_gain_en)
	{
		tex_gain_pos = tex_gain_pos*tex_gain_level/100;
		tex_gain_neg = tex_gain_neg*tex_gain_neg_level/100;
		edg_gain_pos = edg_gain_pos*edg_gain_level/100;
		edg_gain_neg = edg_gain_neg*edg_gain_neg_level/100;
		vpk_gain_pos = vpk_gain_pos*vpk_gain_level/100;
		vpk_gain_neg = vpk_gain_neg*vpk_gain_neg_level/100;
		vpk_edg_gain_pos = vpk_edg_gain_pos*vpk_edg_gain_level/100;
		vpk_edg_gain_neg = vpk_edg_gain_neg*vpk_edg_gain_neg_level/100;

		if(tex_gain_pos>255) tex_gain_pos = 255;
		if(tex_gain_neg>255) tex_gain_neg = 255;
		if(edg_gain_pos>255) edg_gain_pos = 255;
		if(edg_gain_neg>255) edg_gain_neg = 255;
		if(vpk_gain_pos>255) vpk_gain_pos = 255;
		if(vpk_gain_neg>255) vpk_gain_neg = 255;
		if(vpk_edg_gain_pos>255) vpk_edg_gain_pos = 255;
		if(vpk_edg_gain_neg>255) vpk_edg_gain_neg = 255;

		color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_pos = tex_gain_pos;
		color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_neg = tex_gain_neg;

		color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_pos = edg_gain_pos;
		color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_neg = edg_gain_neg;

		color_sharp_dm_cds_peaking_gain_cm0_v_reg.gain_pos = vpk_gain_pos;
		color_sharp_dm_cds_peaking_gain_cm0_v_reg.gain_neg = vpk_gain_neg;

		color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.gain_pos = vpk_edg_gain_pos;
		color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.gain_neg = vpk_edg_gain_neg;

		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg,		color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue);
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg,		color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue);
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_reg,		color_sharp_dm_cds_peaking_gain_cm0_v_reg.regValue);
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_V_EDG_reg,	color_sharp_dm_cds_peaking_gain_cm0_v_edg_reg.regValue);

		color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_pos	 = tex_hv_pos;
		color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_pos = edg_hv_pos;
		color_sharp_dm_cds_peaking_bound_cm0_v_reg.hv_pos 	 = vpk_hv_pos;
		color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.hv_pos= vpk_edg_hv_pos;
		color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_neg	 = tex_hv_neg;
		color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_neg = edg_hv_neg;
		color_sharp_dm_cds_peaking_bound_cm0_v_reg.hv_neg 	 = vpk_hv_neg;
		color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.hv_neg= vpk_edg_hv_neg;
		
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg, 	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue);
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg, 	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue);
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_reg, 		color_sharp_dm_cds_peaking_bound_cm0_v_reg.regValue);
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_EDG_reg,	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.regValue);		

	}

	if(shp_lv_en)
	{
		edg_lv 	   = (blendw * edg_lv_cm0   + (255-blendw) * edg_lv)/255;
		tex_lv 	   = (blendw * tex_lv_cm0   + (255-blendw) * tex_lv)/255;
		vpk_lv 	   = (blendw * v_lv_cm0     + (255-blendw) * vpk_lv)/255;
		vpk_edg_lv = (blendw * v_edg_lv_cm0 + (255-blendw) * vpk_edg_lv)/255;
		
		color_sharp_dm_cds_peaking_bound_cm0_tex_reg.lv	 = tex_lv;
		color_sharp_dm_cds_peaking_bound_cm0_edge_reg.lv = edg_lv;
		color_sharp_dm_cds_peaking_bound_cm0_v_reg.lv 	 = vpk_lv;
		color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.lv= vpk_edg_lv;
		
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg, 	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue);
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg, 	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue);
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_reg, 		color_sharp_dm_cds_peaking_bound_cm0_v_reg.regValue);
		IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_V_EDG_reg,	color_sharp_dm_cds_peaking_bound_cm0_v_edg_reg.regValue);		
	}

}

// for face PQ set dcc old skin
void drvif_color_old_skin_dcc_set(AI_OLD_DCC_apply old_dcc_apply)
{
	color_dcc_d_dcc_skin_tone_segment0_0_RBUS d_dcc_skin_tone_segment0_0_reg;
	color_dcc_d_dcc_skin_tone_segment0_1_RBUS d_dcc_skin_tone_segment0_1_reg;
	color_dcc_d_dcc_skin_tone_yuv_center_0_RBUS d_dcc_skin_tone_yuv_center_0_reg;
	color_dcc_d_dcc_skin_tone_yuv_range_0_RBUS d_dcc_skin_tone_yuv_range_0_reg;

	d_dcc_skin_tone_segment0_0_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_0_reg);
	d_dcc_skin_tone_segment0_1_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_1_reg);
	d_dcc_skin_tone_yuv_center_0_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_0_reg);
	d_dcc_skin_tone_yuv_range_0_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_0_reg);

	d_dcc_skin_tone_segment0_0_reg.y_blending_0_ratio_0 = old_dcc_apply.uv_blend_0;
	d_dcc_skin_tone_segment0_0_reg.y_blending_0_ratio_1 = old_dcc_apply.uv_blend_1;
	d_dcc_skin_tone_segment0_0_reg.y_blending_0_ratio_2 = old_dcc_apply.uv_blend_2;
	d_dcc_skin_tone_segment0_0_reg.y_blending_0_ratio_3 = old_dcc_apply.uv_blend_3;
	d_dcc_skin_tone_segment0_1_reg.y_blending_0_ratio_4 = old_dcc_apply.uv_blend_4;
	d_dcc_skin_tone_segment0_1_reg.y_blending_0_ratio_5 = old_dcc_apply.uv_blend_5;
	d_dcc_skin_tone_segment0_1_reg.y_blending_0_ratio_6 = old_dcc_apply.uv_blend_6;
	d_dcc_skin_tone_segment0_1_reg.y_blending_0_ratio_7 = old_dcc_apply.uv_blend_7;

	{
		d_dcc_skin_tone_yuv_center_0_reg.y_blending_0_y_center = old_dcc_apply.y_center;
		d_dcc_skin_tone_yuv_center_0_reg.y_blending_0_u_center = old_dcc_apply.u_center;
		d_dcc_skin_tone_yuv_center_0_reg.y_blending_0_v_center = old_dcc_apply.v_center;

		d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_y_range = old_dcc_apply.y_range;
		d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_u_range = old_dcc_apply.u_range;
		d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_v_range = old_dcc_apply.v_range;
	}

	// lesley 0910
	{
		color_dcc_d_dcc_skin_tone_en_RBUS d_dcc_skin_tone_en_reg;
		d_dcc_skin_tone_en_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg);
		d_dcc_skin_tone_en_reg.y_blending_0_enhance_en = old_dcc_apply.enhance_en;
		IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg ,  d_dcc_skin_tone_en_reg.regValue );
	}
	// end lesley 0910	

	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_0_reg, d_dcc_skin_tone_segment0_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_1_reg, d_dcc_skin_tone_segment0_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_0_reg, d_dcc_skin_tone_yuv_center_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_0_reg, d_dcc_skin_tone_yuv_range_0_reg.regValue);

}
// end lesley 0808

// same as drvif_color_AI_obj_icm_init()
/*void drvif_color_AI_ICM_mode(unsigned char mode)
{
	color_icm_d_icm_cds_skin_0_RBUS   icm_d_icm_cds_skin_0_reg;
	color_icm_d_icm_ai_0_RBUS     icm_d_icm_ai_0_reg;

	icm_d_icm_cds_skin_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_CDS_SKIN_0_reg);
	icm_d_icm_ai_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_0_reg);
	
	if(mode == 0)
	{
		icm_d_icm_cds_skin_0_reg.cds_skin_en = 0;
		icm_d_icm_ai_0_reg.ai_ros_en = 0;

	}
	else
	{
		icm_d_icm_cds_skin_0_reg.cds_skin_en = 1;
		icm_d_icm_ai_0_reg.ai_ros_en = 1;
	}

	IoReg_Write32(COLOR_ICM_D_ICM_CDS_SKIN_0_reg, icm_d_icm_cds_skin_0_reg.regValue);
}*/

// for face icm init default table
void drvif_color_AI_ICM_table_set(AI_ICM_Ctrl_table *ptr) // read from sharing memory
{
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
	color_icm_d_icm_cds_skin_0_RBUS   icm_d_icm_cds_skin_0_reg;
	color_icm_d_icm_cds_skin_1_RBUS   icm_d_icm_cds_skin_1_reg;
	color_icm_d_icm_cds_skin_2_RBUS   icm_d_icm_cds_skin_2_reg;
	color_icm_d_icm_skin_tone_segment_c0_0_RBUS    icm_d_icm_skin_tone_segment_c0_0_reg;
	color_icm_d_icm_skin_tone_segment_c0_1_RBUS    icm_d_icm_skin_tone_segment_c0_1_reg;
	color_icm_d_icm_skin_tone_segment_c1_0_RBUS    icm_d_icm_skin_tone_segment_c1_0_reg;
	color_icm_d_icm_skin_tone_segment_c1_1_RBUS    icm_d_icm_skin_tone_segment_c1_1_reg;
	color_icm_d_icm_skin_tone_segment_c2_0_RBUS    icm_d_icm_skin_tone_segment_c2_0_reg;
	color_icm_d_icm_skin_tone_segment_c2_1_RBUS    icm_d_icm_skin_tone_segment_c2_1_reg;
	color_icm_d_icm_skin_tone_segment_c3_0_RBUS    icm_d_icm_skin_tone_segment_c3_0_reg;
	color_icm_d_icm_skin_tone_segment_c3_1_RBUS    icm_d_icm_skin_tone_segment_c3_1_reg;
	color_icm_d_icm_skin_tone_segment_c4_0_RBUS    icm_d_icm_skin_tone_segment_c4_0_reg;
	color_icm_d_icm_skin_tone_segment_c4_1_RBUS    icm_d_icm_skin_tone_segment_c4_1_reg;
	color_icm_d_icm_skin_tone_segment_c5_0_RBUS    icm_d_icm_skin_tone_segment_c5_0_reg;
	color_icm_d_icm_skin_tone_segment_c5_1_RBUS    icm_d_icm_skin_tone_segment_c5_1_reg;
	color_icm_d_icm_ai_0_RBUS     icm_d_icm_ai_0_reg;
	color_icm_d_icm_ai_1_RBUS     icm_d_icm_ai_1_reg;
	color_icm_d_icm_ai_2_RBUS     icm_d_icm_ai_2_reg;
	color_icm_d_icm_ai_3_RBUS     icm_d_icm_ai_3_reg;
	color_icm_d_icm_ai_4_RBUS     icm_d_icm_ai_4_reg;
	color_icm_d_icm_ai_5_RBUS     icm_d_icm_ai_5_reg;
	color_icm_d_icm_ai_6_RBUS     icm_d_icm_ai_6_reg;
	color_icm_d_icm_ai_7_RBUS     icm_d_icm_ai_7_reg;
	color_icm_d_icm_ai_8_RBUS     icm_d_icm_ai_8_reg;
	color_icm_d_icm_ai_9_RBUS     icm_d_icm_ai_9_reg;
	color_icm_d_icm_ai_10_RBUS     icm_d_icm_ai_10_reg;
	color_icm_d_icm_ai_11_RBUS     icm_d_icm_ai_11_reg;
	color_icm_d_icm_ai_12_RBUS     icm_d_icm_ai_12_reg;
	color_icm_d_icm_ai_13_RBUS     icm_d_icm_ai_13_reg;
	color_icm_d_icm_ai_14_RBUS     icm_d_icm_ai_14_reg;
	color_icm_d_icm_ai_15_RBUS     icm_d_icm_ai_15_reg;
	color_icm_d_icm_ai_16_RBUS     icm_d_icm_ai_16_reg;
	color_icm_d_icm_ai_17_RBUS     icm_d_icm_ai_17_reg;
	color_icm_d_icm_ai_18_RBUS     icm_d_icm_ai_18_reg;
	color_icm_d_icm_ai_19_RBUS     icm_d_icm_ai_19_reg;
	color_icm_d_icm_ai_20_RBUS     icm_d_icm_ai_20_reg;
	color_icm_d_icm_ai_21_RBUS     icm_d_icm_ai_21_reg;
	color_icm_d_icm_ai_22_RBUS     icm_d_icm_ai_22_reg;

	color_icm_dm_icm_ctrl_RBUS color_icm_dm_icm_ctrl_reg;

	if (ptr==NULL)
		return;

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
	icm_d_icm_cds_skin_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_CDS_SKIN_0_reg);
	icm_d_icm_cds_skin_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_CDS_SKIN_1_reg);
	icm_d_icm_cds_skin_2_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_CDS_SKIN_2_reg);
	icm_d_icm_skin_tone_segment_c0_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C0_0_reg);
	icm_d_icm_skin_tone_segment_c0_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C0_1_reg);
	icm_d_icm_skin_tone_segment_c1_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C1_0_reg);
	icm_d_icm_skin_tone_segment_c1_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C1_1_reg);
	icm_d_icm_skin_tone_segment_c2_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C2_0_reg);
	icm_d_icm_skin_tone_segment_c2_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C2_1_reg);
	icm_d_icm_skin_tone_segment_c3_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C3_0_reg);
	icm_d_icm_skin_tone_segment_c3_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C3_1_reg);
	icm_d_icm_skin_tone_segment_c4_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C4_0_reg);
	icm_d_icm_skin_tone_segment_c4_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C4_1_reg);
	icm_d_icm_skin_tone_segment_c5_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C5_0_reg);
	icm_d_icm_skin_tone_segment_c5_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C5_1_reg);
	icm_d_icm_ai_0_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_0_reg);
	icm_d_icm_ai_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_1_reg);
	icm_d_icm_ai_2_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_2_reg);
	icm_d_icm_ai_3_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_3_reg);
	icm_d_icm_ai_4_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_4_reg);
	icm_d_icm_ai_5_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_5_reg);
	icm_d_icm_ai_6_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_6_reg);
	icm_d_icm_ai_7_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_7_reg);
	icm_d_icm_ai_8_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_8_reg);
	icm_d_icm_ai_9_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_9_reg);
	icm_d_icm_ai_10_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_10_reg);
	icm_d_icm_ai_11_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_11_reg);
	icm_d_icm_ai_12_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_12_reg);
	icm_d_icm_ai_13_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_13_reg);
	icm_d_icm_ai_14_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_14_reg);
	icm_d_icm_ai_15_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_15_reg);
	icm_d_icm_ai_16_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_16_reg);
	icm_d_icm_ai_17_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_17_reg);
	icm_d_icm_ai_18_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_18_reg);
	icm_d_icm_ai_19_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_19_reg);
	icm_d_icm_ai_20_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_20_reg);
	icm_d_icm_ai_21_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_21_reg);
	icm_d_icm_ai_22_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_AI_22_reg);

	color_icm_dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);

	//global settings
	icm_d_icm_cds_skin_0_reg.cds_skin_en	= 	ptr->ai_global.CDS_skin_en;
	icm_d_icm_ai_0_reg.ai_ros_en			= 	ptr->ai_global.AI_ros_en;
	icm_d_icm_ai_22_reg.ai_adjust_en		=	ptr->ai_global.AI_adjust_en;
	icm_d_icm_ai_22_reg.ai_detect_value		=	ptr->ai_global.AI_detect_value;
	icm_d_icm_ai_22_reg.ai_control_value	=	ptr->ai_global.AI_control_value;
	icm_d_icm_cds_skin_0_reg.icm_region_ratio_blending_en =	ptr->ai_global.region_ratio_blending_en;

	// ai icm
	color_icm_dm_icm_ctrl_reg.ai_inside_ratio0_bit5 = (ptr->ai_icm_face_0.ai_blending_inside_ratio)>>5;
	icm_d_icm_ai_0_reg.ai_blending_0_inside_ratio= (ptr->ai_icm_face_0.ai_blending_inside_ratio)&0x1f;
	color_icm_dm_icm_ctrl_reg.ai_inside_ratio1_bit5 = (ptr->ai_icm_face_1.ai_blending_inside_ratio)>>5;
	icm_d_icm_ai_0_reg.ai_blending_1_inside_ratio= (ptr->ai_icm_face_1.ai_blending_inside_ratio)&0x1f;
	color_icm_dm_icm_ctrl_reg.ai_inside_ratio2_bit5 = (ptr->ai_icm_face_2.ai_blending_inside_ratio)>>5;
	icm_d_icm_ai_0_reg.ai_blending_2_inside_ratio= (ptr->ai_icm_face_2.ai_blending_inside_ratio)&0x1f;
	color_icm_dm_icm_ctrl_reg.ai_inside_ratio3_bit5 = (ptr->ai_icm_face_3.ai_blending_inside_ratio)>>5;
	icm_d_icm_ai_0_reg.ai_blending_3_inside_ratio= (ptr->ai_icm_face_3.ai_blending_inside_ratio)&0x1f;
	color_icm_dm_icm_ctrl_reg.ai_inside_ratio4_bit5 = (ptr->ai_icm_face_4.ai_blending_inside_ratio)>>5;
	icm_d_icm_ai_0_reg.ai_blending_4_inside_ratio= (ptr->ai_icm_face_4.ai_blending_inside_ratio)&0x1f;
	color_icm_dm_icm_ctrl_reg.ai_inside_ratio5_bit5 = (ptr->ai_icm_face_5.ai_blending_inside_ratio)>>5;
	icm_d_icm_ai_0_reg.ai_blending_5_inside_ratio= (ptr->ai_icm_face_5.ai_blending_inside_ratio)&0x1f;

	icm_d_icm_ai_1_reg.ai_face0_region_en = ptr->ai_icm_face_0.region_en;
	icm_d_icm_ai_1_reg.ai_shrink_pix_num_0 = ptr->ai_icm_face_0.shrink_pix_num;
	icm_d_icm_ai_1_reg.ai_edge_pix_num_0 = ptr->ai_icm_face_0.edge_pix_num;	
	icm_d_icm_ai_1_reg.ai_face1_region_en = ptr->ai_icm_face_1.region_en;
	icm_d_icm_ai_1_reg.ai_shrink_pix_num_1 = ptr->ai_icm_face_1.shrink_pix_num;
	icm_d_icm_ai_1_reg.ai_edge_pix_num_1 = ptr->ai_icm_face_1.edge_pix_num;	
	icm_d_icm_ai_2_reg.ai_face2_region_en = ptr->ai_icm_face_2.region_en;
	icm_d_icm_ai_2_reg.ai_shrink_pix_num_2 = ptr->ai_icm_face_2.shrink_pix_num;
	icm_d_icm_ai_2_reg.ai_edge_pix_num_2 = ptr->ai_icm_face_2.edge_pix_num;	
	icm_d_icm_ai_2_reg.ai_face3_region_en = ptr->ai_icm_face_3.region_en;
	icm_d_icm_ai_2_reg.ai_shrink_pix_num_3 = ptr->ai_icm_face_3.shrink_pix_num;
	icm_d_icm_ai_2_reg.ai_edge_pix_num_3 = ptr->ai_icm_face_3.edge_pix_num;	
	icm_d_icm_ai_3_reg.ai_face4_region_en = ptr->ai_icm_face_4.region_en;
	icm_d_icm_ai_3_reg.ai_shrink_pix_num_4 = ptr->ai_icm_face_4.shrink_pix_num;
	icm_d_icm_ai_3_reg.ai_edge_pix_num_4 = ptr->ai_icm_face_4.edge_pix_num;	
	icm_d_icm_ai_3_reg.ai_face5_region_en = ptr->ai_icm_face_5.region_en;
	icm_d_icm_ai_3_reg.ai_shrink_pix_num_5 = ptr->ai_icm_face_5.shrink_pix_num;
	icm_d_icm_ai_3_reg.ai_edge_pix_num_5 = ptr->ai_icm_face_5.edge_pix_num;	

	icm_d_icm_ai_4_reg.ai_region_ilu_0 = ptr->ai_icm_face_0.region_iLU;
	icm_d_icm_ai_4_reg.ai_region_ild_0 = ptr->ai_icm_face_0.region_iLD;
	icm_d_icm_ai_4_reg.ai_region_jlu_0 = ptr->ai_icm_face_0.region_jLU;
	icm_d_icm_ai_4_reg.ai_region_jld_0 = ptr->ai_icm_face_0.region_jLD;
	icm_d_icm_ai_5_reg.ai_region_iru_0 = ptr->ai_icm_face_0.region_iRU;
	icm_d_icm_ai_5_reg.ai_region_ird_0 = ptr->ai_icm_face_0.region_iRD;
	icm_d_icm_ai_5_reg.ai_region_jru_0 = ptr->ai_icm_face_0.region_jRU;
	icm_d_icm_ai_5_reg.ai_region_jrd_0 = ptr->ai_icm_face_0.region_jRD;
	icm_d_icm_ai_6_reg.ai_region_ilum_0= ptr->ai_icm_face_0.region_iLUm;
	icm_d_icm_ai_6_reg.ai_region_ildm_0= ptr->ai_icm_face_0.region_iLDm;
	icm_d_icm_ai_6_reg.ai_region_jlum_0= ptr->ai_icm_face_0.region_jLUm;
	icm_d_icm_ai_6_reg.ai_region_jldm_0= ptr->ai_icm_face_0.region_jLDm;
	icm_d_icm_ai_6_reg.ai_region_irum_0= ptr->ai_icm_face_0.region_iRUm;
	icm_d_icm_ai_6_reg.ai_region_irdm_0= ptr->ai_icm_face_0.region_iRDm;
	icm_d_icm_ai_6_reg.ai_region_jrum_0= ptr->ai_icm_face_0.region_jRUm;
	icm_d_icm_ai_6_reg.ai_region_jrdm_0= ptr->ai_icm_face_0.region_jRDm;

	icm_d_icm_ai_7_reg.ai_region_ilu_1 = ptr->ai_icm_face_1.region_iLU;
	icm_d_icm_ai_7_reg.ai_region_ild_1 = ptr->ai_icm_face_1.region_iLD;
	icm_d_icm_ai_7_reg.ai_region_jlu_1 = ptr->ai_icm_face_1.region_jLU;
	icm_d_icm_ai_7_reg.ai_region_jld_1 = ptr->ai_icm_face_1.region_jLD;
	icm_d_icm_ai_8_reg.ai_region_iru_1 = ptr->ai_icm_face_1.region_iRU;
	icm_d_icm_ai_8_reg.ai_region_ird_1 = ptr->ai_icm_face_1.region_iRD;
	icm_d_icm_ai_8_reg.ai_region_jru_1 = ptr->ai_icm_face_1.region_jRU;
	icm_d_icm_ai_8_reg.ai_region_jrd_1 = ptr->ai_icm_face_1.region_jRD;
	icm_d_icm_ai_9_reg.ai_region_ilum_1= ptr->ai_icm_face_1.region_iLUm;
	icm_d_icm_ai_9_reg.ai_region_ildm_1= ptr->ai_icm_face_1.region_iLDm;
	icm_d_icm_ai_9_reg.ai_region_jlum_1= ptr->ai_icm_face_1.region_jLUm;
	icm_d_icm_ai_9_reg.ai_region_jldm_1= ptr->ai_icm_face_1.region_jLDm;
	icm_d_icm_ai_9_reg.ai_region_irum_1= ptr->ai_icm_face_1.region_iRUm;
	icm_d_icm_ai_9_reg.ai_region_irdm_1= ptr->ai_icm_face_1.region_iRDm;
	icm_d_icm_ai_9_reg.ai_region_jrum_1= ptr->ai_icm_face_1.region_jRUm;
	icm_d_icm_ai_9_reg.ai_region_jrdm_1= ptr->ai_icm_face_1.region_jRDm;

	icm_d_icm_ai_10_reg.ai_region_ilu_2 = ptr->ai_icm_face_2.region_iLU;
	icm_d_icm_ai_10_reg.ai_region_ild_2 = ptr->ai_icm_face_2.region_iLD;
	icm_d_icm_ai_10_reg.ai_region_jlu_2 = ptr->ai_icm_face_2.region_jLU;
	icm_d_icm_ai_10_reg.ai_region_jld_2 = ptr->ai_icm_face_2.region_jLD;
	icm_d_icm_ai_11_reg.ai_region_iru_2 = ptr->ai_icm_face_2.region_iRU;
	icm_d_icm_ai_11_reg.ai_region_ird_2 = ptr->ai_icm_face_2.region_iRD;
	icm_d_icm_ai_11_reg.ai_region_jru_2 = ptr->ai_icm_face_2.region_jRU;
	icm_d_icm_ai_11_reg.ai_region_jrd_2 = ptr->ai_icm_face_2.region_jRD;
	icm_d_icm_ai_12_reg.ai_region_ilum_2= ptr->ai_icm_face_2.region_iLUm;
	icm_d_icm_ai_12_reg.ai_region_ildm_2= ptr->ai_icm_face_2.region_iLDm;
	icm_d_icm_ai_12_reg.ai_region_jlum_2= ptr->ai_icm_face_2.region_jLUm;
	icm_d_icm_ai_12_reg.ai_region_jldm_2= ptr->ai_icm_face_2.region_jLDm;
	icm_d_icm_ai_12_reg.ai_region_irum_2= ptr->ai_icm_face_2.region_iRUm;
	icm_d_icm_ai_12_reg.ai_region_irdm_2= ptr->ai_icm_face_2.region_iRDm;
	icm_d_icm_ai_12_reg.ai_region_jrum_2= ptr->ai_icm_face_2.region_jRUm;
	icm_d_icm_ai_12_reg.ai_region_jrdm_2= ptr->ai_icm_face_2.region_jRDm;

	icm_d_icm_ai_13_reg.ai_region_ilu_3 = ptr->ai_icm_face_3.region_iLU;
	icm_d_icm_ai_13_reg.ai_region_ild_3 = ptr->ai_icm_face_3.region_iLD;
	icm_d_icm_ai_13_reg.ai_region_jlu_3 = ptr->ai_icm_face_3.region_jLU;
	icm_d_icm_ai_13_reg.ai_region_jld_3 = ptr->ai_icm_face_3.region_jLD;
	icm_d_icm_ai_14_reg.ai_region_iru_3 = ptr->ai_icm_face_3.region_iRU;
	icm_d_icm_ai_14_reg.ai_region_ird_3 = ptr->ai_icm_face_3.region_iRD;
	icm_d_icm_ai_14_reg.ai_region_jru_3 = ptr->ai_icm_face_3.region_jRU;
	icm_d_icm_ai_14_reg.ai_region_jrd_3 = ptr->ai_icm_face_3.region_jRD;
	icm_d_icm_ai_15_reg.ai_region_ilum_3= ptr->ai_icm_face_3.region_iLUm;
	icm_d_icm_ai_15_reg.ai_region_ildm_3= ptr->ai_icm_face_3.region_iLDm;
	icm_d_icm_ai_15_reg.ai_region_jlum_3= ptr->ai_icm_face_3.region_jLUm;
	icm_d_icm_ai_15_reg.ai_region_jldm_3= ptr->ai_icm_face_3.region_jLDm;
	icm_d_icm_ai_15_reg.ai_region_irum_3= ptr->ai_icm_face_3.region_iRUm;
	icm_d_icm_ai_15_reg.ai_region_irdm_3= ptr->ai_icm_face_3.region_iRDm;
	icm_d_icm_ai_15_reg.ai_region_jrum_3= ptr->ai_icm_face_3.region_jRUm;
	icm_d_icm_ai_15_reg.ai_region_jrdm_3= ptr->ai_icm_face_3.region_jRDm;

	icm_d_icm_ai_16_reg.ai_region_ilu_4 = ptr->ai_icm_face_4.region_iLU;
	icm_d_icm_ai_16_reg.ai_region_ild_4 = ptr->ai_icm_face_4.region_iLD;
	icm_d_icm_ai_16_reg.ai_region_jlu_4 = ptr->ai_icm_face_4.region_jLU;
	icm_d_icm_ai_16_reg.ai_region_jld_4 = ptr->ai_icm_face_4.region_jLD;
	icm_d_icm_ai_17_reg.ai_region_iru_4 = ptr->ai_icm_face_4.region_iRU;
	icm_d_icm_ai_17_reg.ai_region_ird_4 = ptr->ai_icm_face_4.region_iRD;
	icm_d_icm_ai_17_reg.ai_region_jru_4 = ptr->ai_icm_face_4.region_jRU;
	icm_d_icm_ai_17_reg.ai_region_jrd_4 = ptr->ai_icm_face_4.region_jRD;
	icm_d_icm_ai_18_reg.ai_region_ilum_4= ptr->ai_icm_face_4.region_iLUm;
	icm_d_icm_ai_18_reg.ai_region_ildm_4= ptr->ai_icm_face_4.region_iLDm;
	icm_d_icm_ai_18_reg.ai_region_jlum_4= ptr->ai_icm_face_4.region_jLUm;
	icm_d_icm_ai_18_reg.ai_region_jldm_4= ptr->ai_icm_face_4.region_jLDm;
	icm_d_icm_ai_18_reg.ai_region_irum_4= ptr->ai_icm_face_4.region_iRUm;
	icm_d_icm_ai_18_reg.ai_region_irdm_4= ptr->ai_icm_face_4.region_iRDm;
	icm_d_icm_ai_18_reg.ai_region_jrum_4= ptr->ai_icm_face_4.region_jRUm;
	icm_d_icm_ai_18_reg.ai_region_jrdm_4= ptr->ai_icm_face_4.region_jRDm;

	icm_d_icm_ai_19_reg.ai_region_ilu_5 = ptr->ai_icm_face_5.region_iLU;
	icm_d_icm_ai_19_reg.ai_region_ild_5 = ptr->ai_icm_face_5.region_iLD;
	icm_d_icm_ai_19_reg.ai_region_jlu_5 = ptr->ai_icm_face_5.region_jLU;
	icm_d_icm_ai_19_reg.ai_region_jld_5 = ptr->ai_icm_face_5.region_jLD;
	icm_d_icm_ai_20_reg.ai_region_iru_5 = ptr->ai_icm_face_5.region_iRU;
	icm_d_icm_ai_20_reg.ai_region_ird_5 = ptr->ai_icm_face_5.region_iRD;
	icm_d_icm_ai_20_reg.ai_region_jru_5 = ptr->ai_icm_face_5.region_jRU;
	icm_d_icm_ai_20_reg.ai_region_jrd_5 = ptr->ai_icm_face_5.region_jRD;
	icm_d_icm_ai_21_reg.ai_region_ilum_5= ptr->ai_icm_face_5.region_iLUm;
	icm_d_icm_ai_21_reg.ai_region_ildm_5= ptr->ai_icm_face_5.region_iLDm;
	icm_d_icm_ai_21_reg.ai_region_jlum_5= ptr->ai_icm_face_5.region_jLUm;
	icm_d_icm_ai_21_reg.ai_region_jldm_5= ptr->ai_icm_face_5.region_jLDm;
	icm_d_icm_ai_21_reg.ai_region_irum_5= ptr->ai_icm_face_5.region_iRUm;
	icm_d_icm_ai_21_reg.ai_region_irdm_5= ptr->ai_icm_face_5.region_iRDm;
	icm_d_icm_ai_21_reg.ai_region_jrum_5= ptr->ai_icm_face_5.region_jRUm;
	icm_d_icm_ai_21_reg.ai_region_jrdm_5= ptr->ai_icm_face_5.region_jRDm;

	// cds icm
	icm_d_icm_ai_offset_0_reg.ai_i_offset0	=	ptr->cds_icm_face_0.i_offset;
	icm_d_icm_ai_offset_0_reg.ai_s_offset0	=	ptr->cds_icm_face_0.s_offset;
	icm_d_icm_ai_offset_1_reg.ai_h_offset0	=	ptr->cds_icm_face_0.h_offset;
	icm_d_icm_ai_offset_2_reg.ai_i_offset1	=	ptr->cds_icm_face_1.i_offset;
	icm_d_icm_ai_offset_2_reg.ai_s_offset1	=	ptr->cds_icm_face_1.s_offset;
	icm_d_icm_ai_offset_3_reg.ai_h_offset1	=	ptr->cds_icm_face_1.h_offset;
	icm_d_icm_ai_offset_4_reg.ai_i_offset2	=	ptr->cds_icm_face_2.i_offset;
	icm_d_icm_ai_offset_4_reg.ai_s_offset2	=	ptr->cds_icm_face_2.s_offset;
	icm_d_icm_ai_offset_5_reg.ai_h_offset2	=	ptr->cds_icm_face_2.h_offset;
	icm_d_icm_ai_offset_6_reg.ai_i_offset3	=	ptr->cds_icm_face_3.i_offset;
	icm_d_icm_ai_offset_6_reg.ai_s_offset3	=	ptr->cds_icm_face_3.s_offset;
	icm_d_icm_ai_offset_7_reg.ai_h_offset3	=	ptr->cds_icm_face_3.h_offset;
	icm_d_icm_ai_offset_8_reg.ai_i_offset4	=	ptr->cds_icm_face_4.i_offset;;
	icm_d_icm_ai_offset_8_reg.ai_s_offset4	=	ptr->cds_icm_face_4.s_offset;;
	icm_d_icm_ai_offset_9_reg.ai_h_offset4	=	ptr->cds_icm_face_4.h_offset;;
	icm_d_icm_ai_offset_10_reg.ai_i_offset5	=	ptr->cds_icm_face_5.i_offset;;
	icm_d_icm_ai_offset_10_reg.ai_s_offset5	=	ptr->cds_icm_face_5.s_offset;;
	icm_d_icm_ai_offset_11_reg.ai_h_offset5	=	ptr->cds_icm_face_5.h_offset;;

	icm_d_icm_cds_skin_1_reg.cds_uv_range_0= ptr->cds_icm_face_0.uv_range;
	icm_d_icm_cds_skin_1_reg.cds_uv_range_1= ptr->cds_icm_face_1.uv_range;
	icm_d_icm_cds_skin_1_reg.cds_uv_range_2= ptr->cds_icm_face_2.uv_range;
	icm_d_icm_cds_skin_2_reg.cds_uv_range_3= ptr->cds_icm_face_3.uv_range;
	icm_d_icm_cds_skin_2_reg.cds_uv_range_4= ptr->cds_icm_face_4.uv_range;
	icm_d_icm_cds_skin_2_reg.cds_uv_range_5= ptr->cds_icm_face_5.uv_range;

	icm_d_icm_skin_tone_segment_c0_0_reg.c_blending_0_ratio_0= ptr->cds_icm_face_0.c_blending_ratio_0;
	icm_d_icm_skin_tone_segment_c0_0_reg.c_blending_0_ratio_1= ptr->cds_icm_face_0.c_blending_ratio_1;
	icm_d_icm_skin_tone_segment_c0_0_reg.c_blending_0_ratio_2= ptr->cds_icm_face_0.c_blending_ratio_2;
	icm_d_icm_skin_tone_segment_c0_0_reg.c_blending_0_ratio_3= ptr->cds_icm_face_0.c_blending_ratio_3;
	icm_d_icm_skin_tone_segment_c0_1_reg.c_blending_0_ratio_4= ptr->cds_icm_face_0.c_blending_ratio_4;
	icm_d_icm_skin_tone_segment_c0_1_reg.c_blending_0_ratio_5= ptr->cds_icm_face_0.c_blending_ratio_5;
	icm_d_icm_skin_tone_segment_c0_1_reg.c_blending_0_ratio_6= ptr->cds_icm_face_0.c_blending_ratio_6;
	icm_d_icm_skin_tone_segment_c0_1_reg.c_blending_0_ratio_7= ptr->cds_icm_face_0.c_blending_ratio_7;	
	icm_d_icm_skin_tone_segment_c1_0_reg.c_blending_1_ratio_0= ptr->cds_icm_face_1.c_blending_ratio_0;
	icm_d_icm_skin_tone_segment_c1_0_reg.c_blending_1_ratio_1= ptr->cds_icm_face_1.c_blending_ratio_1;
	icm_d_icm_skin_tone_segment_c1_0_reg.c_blending_1_ratio_2= ptr->cds_icm_face_1.c_blending_ratio_2;
	icm_d_icm_skin_tone_segment_c1_0_reg.c_blending_1_ratio_3= ptr->cds_icm_face_1.c_blending_ratio_3;
	icm_d_icm_skin_tone_segment_c1_1_reg.c_blending_1_ratio_4= ptr->cds_icm_face_1.c_blending_ratio_4;
	icm_d_icm_skin_tone_segment_c1_1_reg.c_blending_1_ratio_5= ptr->cds_icm_face_1.c_blending_ratio_5;
	icm_d_icm_skin_tone_segment_c1_1_reg.c_blending_1_ratio_6= ptr->cds_icm_face_1.c_blending_ratio_6;
	icm_d_icm_skin_tone_segment_c1_1_reg.c_blending_1_ratio_7= ptr->cds_icm_face_1.c_blending_ratio_7;
	icm_d_icm_skin_tone_segment_c2_0_reg.c_blending_2_ratio_0= ptr->cds_icm_face_2.c_blending_ratio_0;
	icm_d_icm_skin_tone_segment_c2_0_reg.c_blending_2_ratio_1= ptr->cds_icm_face_2.c_blending_ratio_1;
	icm_d_icm_skin_tone_segment_c2_0_reg.c_blending_2_ratio_2= ptr->cds_icm_face_2.c_blending_ratio_2;
	icm_d_icm_skin_tone_segment_c2_0_reg.c_blending_2_ratio_3= ptr->cds_icm_face_2.c_blending_ratio_3;
	icm_d_icm_skin_tone_segment_c2_1_reg.c_blending_2_ratio_4= ptr->cds_icm_face_2.c_blending_ratio_4;
	icm_d_icm_skin_tone_segment_c2_1_reg.c_blending_2_ratio_5= ptr->cds_icm_face_2.c_blending_ratio_5;
	icm_d_icm_skin_tone_segment_c2_1_reg.c_blending_2_ratio_6= ptr->cds_icm_face_2.c_blending_ratio_6;
	icm_d_icm_skin_tone_segment_c2_1_reg.c_blending_2_ratio_7= ptr->cds_icm_face_2.c_blending_ratio_7;
	icm_d_icm_skin_tone_segment_c3_0_reg.c_blending_3_ratio_0= ptr->cds_icm_face_3.c_blending_ratio_0;
	icm_d_icm_skin_tone_segment_c3_0_reg.c_blending_3_ratio_1= ptr->cds_icm_face_3.c_blending_ratio_1;
	icm_d_icm_skin_tone_segment_c3_0_reg.c_blending_3_ratio_2= ptr->cds_icm_face_3.c_blending_ratio_2;
	icm_d_icm_skin_tone_segment_c3_0_reg.c_blending_3_ratio_3= ptr->cds_icm_face_3.c_blending_ratio_3;
	icm_d_icm_skin_tone_segment_c3_1_reg.c_blending_3_ratio_4= ptr->cds_icm_face_3.c_blending_ratio_4;
	icm_d_icm_skin_tone_segment_c3_1_reg.c_blending_3_ratio_5= ptr->cds_icm_face_3.c_blending_ratio_5;
	icm_d_icm_skin_tone_segment_c3_1_reg.c_blending_3_ratio_6= ptr->cds_icm_face_3.c_blending_ratio_6;
	icm_d_icm_skin_tone_segment_c3_1_reg.c_blending_3_ratio_7= ptr->cds_icm_face_3.c_blending_ratio_7;
	icm_d_icm_skin_tone_segment_c4_0_reg.c_blending_4_ratio_0= ptr->cds_icm_face_4.c_blending_ratio_0;
	icm_d_icm_skin_tone_segment_c4_0_reg.c_blending_4_ratio_1= ptr->cds_icm_face_4.c_blending_ratio_1;
	icm_d_icm_skin_tone_segment_c4_0_reg.c_blending_4_ratio_2= ptr->cds_icm_face_4.c_blending_ratio_2;
	icm_d_icm_skin_tone_segment_c4_0_reg.c_blending_4_ratio_3= ptr->cds_icm_face_4.c_blending_ratio_3;
	icm_d_icm_skin_tone_segment_c4_1_reg.c_blending_4_ratio_4= ptr->cds_icm_face_4.c_blending_ratio_4;
	icm_d_icm_skin_tone_segment_c4_1_reg.c_blending_4_ratio_5= ptr->cds_icm_face_4.c_blending_ratio_5;
	icm_d_icm_skin_tone_segment_c4_1_reg.c_blending_4_ratio_6= ptr->cds_icm_face_4.c_blending_ratio_6;
	icm_d_icm_skin_tone_segment_c4_1_reg.c_blending_4_ratio_7= ptr->cds_icm_face_4.c_blending_ratio_7;
	icm_d_icm_skin_tone_segment_c5_0_reg.c_blending_5_ratio_0= ptr->cds_icm_face_5.c_blending_ratio_0;
	icm_d_icm_skin_tone_segment_c5_0_reg.c_blending_5_ratio_1= ptr->cds_icm_face_5.c_blending_ratio_1;
	icm_d_icm_skin_tone_segment_c5_0_reg.c_blending_5_ratio_2= ptr->cds_icm_face_5.c_blending_ratio_2;
	icm_d_icm_skin_tone_segment_c5_0_reg.c_blending_5_ratio_3= ptr->cds_icm_face_5.c_blending_ratio_3;
	icm_d_icm_skin_tone_segment_c5_1_reg.c_blending_5_ratio_4= ptr->cds_icm_face_5.c_blending_ratio_4;
	icm_d_icm_skin_tone_segment_c5_1_reg.c_blending_5_ratio_5= ptr->cds_icm_face_5.c_blending_ratio_5;
	icm_d_icm_skin_tone_segment_c5_1_reg.c_blending_5_ratio_6= ptr->cds_icm_face_5.c_blending_ratio_6;
	icm_d_icm_skin_tone_segment_c5_1_reg.c_blending_5_ratio_7= ptr->cds_icm_face_5.c_blending_ratio_7;
	
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_0_reg,icm_d_icm_ai_offset_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_1_reg,icm_d_icm_ai_offset_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_2_reg,icm_d_icm_ai_offset_2_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_3_reg,icm_d_icm_ai_offset_3_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_4_reg,icm_d_icm_ai_offset_4_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_5_reg,icm_d_icm_ai_offset_5_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_6_reg,icm_d_icm_ai_offset_6_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_7_reg,icm_d_icm_ai_offset_7_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_8_reg,icm_d_icm_ai_offset_8_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_9_reg,icm_d_icm_ai_offset_9_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_10_reg,icm_d_icm_ai_offset_10_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_OFFSET_11_reg,icm_d_icm_ai_offset_11_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_CDS_SKIN_0_reg,icm_d_icm_cds_skin_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_CDS_SKIN_1_reg,icm_d_icm_cds_skin_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_CDS_SKIN_2_reg,icm_d_icm_cds_skin_2_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C0_0_reg,icm_d_icm_skin_tone_segment_c0_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C0_1_reg,icm_d_icm_skin_tone_segment_c0_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C1_0_reg,icm_d_icm_skin_tone_segment_c1_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C1_1_reg,icm_d_icm_skin_tone_segment_c1_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C2_0_reg,icm_d_icm_skin_tone_segment_c2_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C2_1_reg,icm_d_icm_skin_tone_segment_c2_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C3_0_reg,icm_d_icm_skin_tone_segment_c3_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C3_1_reg,icm_d_icm_skin_tone_segment_c3_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C4_0_reg,icm_d_icm_skin_tone_segment_c4_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C4_1_reg,icm_d_icm_skin_tone_segment_c4_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C5_0_reg,icm_d_icm_skin_tone_segment_c5_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_SKIN_TONE_SEGMENT_C5_1_reg,icm_d_icm_skin_tone_segment_c5_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_0_reg,icm_d_icm_ai_0_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_1_reg,icm_d_icm_ai_1_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_2_reg,icm_d_icm_ai_2_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_3_reg,icm_d_icm_ai_3_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_4_reg,icm_d_icm_ai_4_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_5_reg,icm_d_icm_ai_5_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_6_reg,icm_d_icm_ai_6_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_7_reg,icm_d_icm_ai_7_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_8_reg,icm_d_icm_ai_8_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_9_reg,icm_d_icm_ai_9_reg.regValue);	
	IoReg_Write32(COLOR_ICM_D_ICM_AI_10_reg,icm_d_icm_ai_10_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_11_reg,icm_d_icm_ai_11_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_12_reg,icm_d_icm_ai_12_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_13_reg,icm_d_icm_ai_13_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_14_reg,icm_d_icm_ai_14_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_15_reg,icm_d_icm_ai_15_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_16_reg,icm_d_icm_ai_16_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_17_reg,icm_d_icm_ai_17_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_18_reg,icm_d_icm_ai_18_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_19_reg,icm_d_icm_ai_19_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_20_reg,icm_d_icm_ai_20_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_21_reg,icm_d_icm_ai_21_reg.regValue);
	IoReg_Write32(COLOR_ICM_D_ICM_AI_22_reg,icm_d_icm_ai_22_reg.regValue);

	IoReg_Write32(COLOR_ICM_DM_ICM_CTRL_reg,color_icm_dm_icm_ctrl_reg.regValue);

}

// for face dcc init default table
void drvif_color_AI_DCC_table_set(AI_DCC_Ctrl_table *ptr)
{
	color_dcc_d_dcc_ai_0_RBUS	dcc_d_dcc_ai_0_reg;
	color_dcc_d_dcc_ai_1_RBUS	dcc_d_dcc_ai_1_reg;
	color_dcc_d_dcc_ai_2_RBUS	dcc_d_dcc_ai_2_reg;
	color_dcc_d_dcc_ai_3_RBUS	dcc_d_dcc_ai_3_reg;
	color_dcc_d_dcc_ai_4_RBUS	dcc_d_dcc_ai_4_reg;
	color_dcc_d_dcc_ai_5_RBUS	dcc_d_dcc_ai_5_reg;
	color_dcc_d_dcc_ai_6_RBUS	dcc_d_dcc_ai_6_reg;
	color_dcc_d_dcc_ai_7_RBUS   dcc_d_dcc_ai_7_reg;
	color_dcc_d_dcc_ai_8_RBUS	dcc_d_dcc_ai_8_reg;
	color_dcc_d_dcc_ai_9_RBUS	dcc_d_dcc_ai_9_reg;
	color_dcc_d_dcc_ai_10_RBUS   dcc_d_dcc_ai_10_reg;
	color_dcc_d_dcc_ai_11_RBUS	 dcc_d_dcc_ai_11_reg;
	color_dcc_d_dcc_ai_12_RBUS	 dcc_d_dcc_ai_12_reg;
	color_dcc_d_dcc_ai_13_RBUS   dcc_d_dcc_ai_13_reg;
	color_dcc_d_dcc_ai_14_RBUS	 dcc_d_dcc_ai_14_reg;
	color_dcc_d_dcc_ai_15_RBUS	 dcc_d_dcc_ai_15_reg;
	color_dcc_d_dcc_ai_16_RBUS   dcc_d_dcc_ai_16_reg;
	color_dcc_d_dcc_ai_17_RBUS	 dcc_d_dcc_ai_17_reg;
	color_dcc_d_dcc_ai_18_RBUS	 dcc_d_dcc_ai_18_reg;
	color_dcc_d_dcc_ai_19_RBUS   dcc_d_dcc_ai_19_reg;
	color_dcc_d_dcc_ai_20_RBUS	 dcc_d_dcc_ai_20_reg;
	color_dcc_d_dcc_ai_21_RBUS	 dcc_d_dcc_ai_21_reg;
	color_dcc_d_dcc_ai_22_RBUS   dcc_d_dcc_ai_22_reg;
	color_dcc_d_dcc_ai_23_RBUS	 dcc_d_dcc_ai_23_reg;
	color_dcc_d_dcc_ai_24_RBUS	 dcc_d_dcc_ai_24_reg;
	color_dcc_d_dcc_ai_25_RBUS   dcc_d_dcc_ai_25_reg;
	color_dcc_d_dcc_ai_26_RBUS	 dcc_d_dcc_ai_26_reg;
	color_dcc_d_dcc_ai_27_RBUS	 dcc_d_dcc_ai_27_reg;
	color_dcc_d_dcc_ai_28_RBUS   dcc_d_dcc_ai_28_reg;
	color_dcc_d_dcc_ai_29_RBUS	 dcc_d_dcc_ai_29_reg;
	color_dcc_d_dcc_ai_30_RBUS	 dcc_d_dcc_ai_30_reg;
	color_dcc_d_dcc_ai_31_RBUS   dcc_d_dcc_ai_31_reg;
	color_dcc_d_dcc_ai_32_RBUS	 dcc_d_dcc_ai_32_reg;
	color_dcc_d_dcc_ai_33_RBUS	 dcc_d_dcc_ai_33_reg;
	color_dcc_d_dcc_ai_34_RBUS   dcc_d_dcc_ai_34_reg;
	color_dcc_d_dcc_ai_35_RBUS	 dcc_d_dcc_ai_35_reg;
	color_dcc_d_dcc_ai_36_RBUS	 dcc_d_dcc_ai_36_reg;
	color_dcc_d_dcc_ai_37_RBUS	 dcc_d_dcc_ai_37_reg;

	color_dcc_d_dcc_skin_tone_segment_c0_0_RBUS dcc_d_dcc_skin_tone_segment_c0_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c0_1_RBUS dcc_d_dcc_skin_tone_segment_c0_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c1_0_RBUS dcc_d_dcc_skin_tone_segment_c1_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c1_1_RBUS dcc_d_dcc_skin_tone_segment_c1_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c2_0_RBUS dcc_d_dcc_skin_tone_segment_c2_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c2_1_RBUS dcc_d_dcc_skin_tone_segment_c2_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c3_0_RBUS dcc_d_dcc_skin_tone_segment_c3_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c3_1_RBUS dcc_d_dcc_skin_tone_segment_c3_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c4_0_RBUS dcc_d_dcc_skin_tone_segment_c4_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c4_1_RBUS dcc_d_dcc_skin_tone_segment_c4_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c5_0_RBUS dcc_d_dcc_skin_tone_segment_c5_0_reg;
	color_dcc_d_dcc_skin_tone_segment_c5_1_RBUS dcc_d_dcc_skin_tone_segment_c5_1_reg;

	color_dcc_d_dcc_skin_tone_en_RBUS d_dcc_skin_tone_en_reg;
	color_dcc_d_dcc_cds_skin_1_RBUS color_dcc_d_dcc_cds_skin_1_reg;
	color_dcc_d_dcc_cds_skin_2_RBUS color_dcc_d_dcc_cds_skin_2_reg;
	color_dcc_d_dcc_ctrl_RBUS color_dcc_d_dcc_ctrl_reg;

	color_dcc_d_dcc_ctrl_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_CTRL_reg);
	color_dcc_d_dcc_cds_skin_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_CDS_SKIN_1_reg);
	color_dcc_d_dcc_cds_skin_2_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_CDS_SKIN_2_reg);
	d_dcc_skin_tone_en_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg);

	dcc_d_dcc_ai_0_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_0_reg);
	dcc_d_dcc_ai_1_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_1_reg);
	dcc_d_dcc_ai_2_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_2_reg);
	dcc_d_dcc_ai_3_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_3_reg);
	dcc_d_dcc_ai_4_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_4_reg);
	dcc_d_dcc_ai_5_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_5_reg);
	dcc_d_dcc_ai_6_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_6_reg);
	dcc_d_dcc_ai_7_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_7_reg);
	dcc_d_dcc_ai_8_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_8_reg);
	dcc_d_dcc_ai_9_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_9_reg);
	dcc_d_dcc_ai_10_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_10_reg);
	dcc_d_dcc_ai_11_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_11_reg);
	dcc_d_dcc_ai_12_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_12_reg);
	dcc_d_dcc_ai_13_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_13_reg);
	dcc_d_dcc_ai_14_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_14_reg);
	dcc_d_dcc_ai_15_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_15_reg);
	dcc_d_dcc_ai_16_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_16_reg);
	dcc_d_dcc_ai_17_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_17_reg);
	dcc_d_dcc_ai_18_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_18_reg);
	dcc_d_dcc_ai_19_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_19_reg);
	dcc_d_dcc_ai_20_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_20_reg);
	dcc_d_dcc_ai_21_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_21_reg);
	dcc_d_dcc_ai_22_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_22_reg);
	dcc_d_dcc_ai_23_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_23_reg);
	dcc_d_dcc_ai_24_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_24_reg);
	dcc_d_dcc_ai_25_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_25_reg);
	dcc_d_dcc_ai_26_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_26_reg);
	dcc_d_dcc_ai_27_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_27_reg);
	dcc_d_dcc_ai_28_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_28_reg);
	dcc_d_dcc_ai_29_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_29_reg);
	dcc_d_dcc_ai_30_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_30_reg);
	dcc_d_dcc_ai_31_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_31_reg);
	dcc_d_dcc_ai_32_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_32_reg);
	dcc_d_dcc_ai_33_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_33_reg);
	dcc_d_dcc_ai_34_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_34_reg);
	dcc_d_dcc_ai_35_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_35_reg);
	dcc_d_dcc_ai_36_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_36_reg);
	dcc_d_dcc_ai_37_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_AI_37_reg);
	
	dcc_d_dcc_skin_tone_segment_c0_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_0_reg);
	dcc_d_dcc_skin_tone_segment_c0_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_1_reg);
	dcc_d_dcc_skin_tone_segment_c1_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_0_reg);
	dcc_d_dcc_skin_tone_segment_c1_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_1_reg);
	dcc_d_dcc_skin_tone_segment_c2_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_0_reg);
	dcc_d_dcc_skin_tone_segment_c2_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_1_reg);
	dcc_d_dcc_skin_tone_segment_c3_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_0_reg);
	dcc_d_dcc_skin_tone_segment_c3_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_1_reg);
	dcc_d_dcc_skin_tone_segment_c4_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C4_0_reg);
	dcc_d_dcc_skin_tone_segment_c4_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C4_1_reg);
	dcc_d_dcc_skin_tone_segment_c5_0_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C5_0_reg);
	dcc_d_dcc_skin_tone_segment_c5_1_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C5_1_reg);

	if (ptr==NULL)
		return;

	//global  settings
	dcc_d_dcc_ai_0_reg.ai_ros_en = ptr->dcc_ai_global.AI_ros_en ;
	dcc_d_dcc_ai_37_reg.ai_adjust_en = ptr->dcc_ai_global.AI_adjust_en;
	dcc_d_dcc_ai_37_reg.ai_detect_value = 	ptr->dcc_ai_global.AI_detect_value;
	dcc_d_dcc_ai_37_reg.ai_control_value =  ptr->dcc_ai_global.AI_control_value;

	//face 0
	dcc_d_dcc_ai_1_reg.ai_face0_region_en = ptr->ai_dcc_face_0.region_en;
	color_dcc_d_dcc_ctrl_reg.ai_blending_0_inside_ratio_msb = (ptr->ai_dcc_face_0.ai_blending_inside_ratio)>>5;
	dcc_d_dcc_ai_0_reg.ai_blending_0_inside_ratio= (ptr->ai_dcc_face_0.ai_blending_inside_ratio)&0x1f;
	dcc_d_dcc_ai_1_reg.ai_shrink_pix_num_0 = ptr->ai_dcc_face_0.shrink_pix_num;
	dcc_d_dcc_ai_1_reg.ai_edge_pix_num_0 = ptr->ai_dcc_face_0.edge_pix_num;
	dcc_d_dcc_ai_19_reg.ai_region_ilu_0 = ptr->ai_dcc_face_0.region_iLU;
	dcc_d_dcc_ai_19_reg.ai_region_ild_0 = ptr->ai_dcc_face_0.region_iLD;
	dcc_d_dcc_ai_19_reg.ai_region_jlu_0 = ptr->ai_dcc_face_0.region_jLU;
	dcc_d_dcc_ai_19_reg.ai_region_jld_0 = ptr->ai_dcc_face_0.region_jLD;
	dcc_d_dcc_ai_20_reg.ai_region_iru_0 = ptr->ai_dcc_face_0.region_iRU;
	dcc_d_dcc_ai_20_reg.ai_region_ird_0 = ptr->ai_dcc_face_0.region_iRD;
	dcc_d_dcc_ai_20_reg.ai_region_jru_0 = ptr->ai_dcc_face_0.region_jRU;
	dcc_d_dcc_ai_20_reg.ai_region_jrd_0 = ptr->ai_dcc_face_0.region_jRD;
	dcc_d_dcc_ai_21_reg.ai_region_ilum_0= ptr->ai_dcc_face_0.region_iLUm;
	dcc_d_dcc_ai_21_reg.ai_region_ildm_0= ptr->ai_dcc_face_0.region_iLDm;
	dcc_d_dcc_ai_21_reg.ai_region_jlum_0= ptr->ai_dcc_face_0.region_jLUm;
	dcc_d_dcc_ai_21_reg.ai_region_jldm_0= ptr->ai_dcc_face_0.region_jLDm;
	dcc_d_dcc_ai_21_reg.ai_region_irum_0= ptr->ai_dcc_face_0.region_iRUm;
	dcc_d_dcc_ai_21_reg.ai_region_irdm_0= ptr->ai_dcc_face_0.region_iRDm;
	dcc_d_dcc_ai_21_reg.ai_region_jrum_0= ptr->ai_dcc_face_0.region_jRUm;
	dcc_d_dcc_ai_21_reg.ai_region_jrdm_0= ptr->ai_dcc_face_0.region_jRDm;
	dcc_d_dcc_ai_7_reg.ai_region_start_i_0 = ptr->ai_dcc_face_0.region_start_i;
	dcc_d_dcc_ai_7_reg.ai_region_end_i_0 = ptr->ai_dcc_face_0.region_end_i;
	dcc_d_dcc_ai_13_reg.ai_region_start_j_0 = ptr->ai_dcc_face_0.region_start_j;
	dcc_d_dcc_ai_13_reg.ai_region_end_j_0 = ptr->ai_dcc_face_0.region_end_j;

	dcc_d_dcc_skin_tone_segment_c0_0_reg.c_blending_0_ratio_0=ptr->cds_dcc_face_0.c_blending_ratio_0;
	dcc_d_dcc_skin_tone_segment_c0_0_reg.c_blending_0_ratio_1=ptr->cds_dcc_face_0.c_blending_ratio_1;
	dcc_d_dcc_skin_tone_segment_c0_0_reg.c_blending_0_ratio_2=ptr->cds_dcc_face_0.c_blending_ratio_2;
	dcc_d_dcc_skin_tone_segment_c0_0_reg.c_blending_0_ratio_3=ptr->cds_dcc_face_0.c_blending_ratio_3;
	dcc_d_dcc_skin_tone_segment_c0_1_reg.c_blending_0_ratio_4=ptr->cds_dcc_face_0.c_blending_ratio_4;
	dcc_d_dcc_skin_tone_segment_c0_1_reg.c_blending_0_ratio_5=ptr->cds_dcc_face_0.c_blending_ratio_5;
	dcc_d_dcc_skin_tone_segment_c0_1_reg.c_blending_0_ratio_6=ptr->cds_dcc_face_0.c_blending_ratio_6;
	dcc_d_dcc_skin_tone_segment_c0_1_reg.c_blending_0_ratio_7=ptr->cds_dcc_face_0.c_blending_ratio_7;
	color_dcc_d_dcc_cds_skin_1_reg.cds_uv_range_0 = ptr->cds_dcc_face_0.uv_range;
	dcc_d_dcc_ai_1_reg.ai_center_u_0 = ptr->cds_dcc_face_0.u_center;
	dcc_d_dcc_ai_1_reg.ai_center_v_0 = ptr->cds_dcc_face_0.v_center;
	d_dcc_skin_tone_en_reg.y_blending_0_enhance_en = ptr->cds_dcc_face_0.region_enhance_en;


	//face 1
	dcc_d_dcc_ai_2_reg.ai_face1_region_en = ptr->ai_dcc_face_1.region_en;
	color_dcc_d_dcc_ctrl_reg.ai_blending_1_inside_ratio_msb = (ptr->ai_dcc_face_1.ai_blending_inside_ratio)>>5;
	dcc_d_dcc_ai_0_reg.ai_blending_1_inside_ratio= (ptr->ai_dcc_face_1.ai_blending_inside_ratio)&0x1f;
	dcc_d_dcc_ai_2_reg.ai_shrink_pix_num_1 = ptr->ai_dcc_face_1.shrink_pix_num;
	dcc_d_dcc_ai_2_reg.ai_edge_pix_num_1 = ptr->ai_dcc_face_1.edge_pix_num;
	dcc_d_dcc_ai_22_reg.ai_region_ilu_1 = ptr->ai_dcc_face_1.region_iLU;
	dcc_d_dcc_ai_22_reg.ai_region_ild_1 = ptr->ai_dcc_face_1.region_iLD;
	dcc_d_dcc_ai_22_reg.ai_region_jlu_1 = ptr->ai_dcc_face_1.region_jLU;
	dcc_d_dcc_ai_22_reg.ai_region_jld_1 = ptr->ai_dcc_face_1.region_jLD;
	dcc_d_dcc_ai_23_reg.ai_region_iru_1 = ptr->ai_dcc_face_1.region_iRU;
	dcc_d_dcc_ai_23_reg.ai_region_ird_1 = ptr->ai_dcc_face_1.region_iRD;
	dcc_d_dcc_ai_23_reg.ai_region_jru_1 = ptr->ai_dcc_face_1.region_jRU;
	dcc_d_dcc_ai_23_reg.ai_region_jrd_1 = ptr->ai_dcc_face_1.region_jRD;
	dcc_d_dcc_ai_24_reg.ai_region_ilum_1= ptr->ai_dcc_face_1.region_iLUm;
	dcc_d_dcc_ai_24_reg.ai_region_ildm_1= ptr->ai_dcc_face_1.region_iLDm;
	dcc_d_dcc_ai_24_reg.ai_region_jlum_1= ptr->ai_dcc_face_1.region_jLUm;
	dcc_d_dcc_ai_24_reg.ai_region_jldm_1= ptr->ai_dcc_face_1.region_jLDm;
	dcc_d_dcc_ai_24_reg.ai_region_irum_1= ptr->ai_dcc_face_1.region_iRUm;
	dcc_d_dcc_ai_24_reg.ai_region_irdm_1= ptr->ai_dcc_face_1.region_iRDm;
	dcc_d_dcc_ai_24_reg.ai_region_jrum_1= ptr->ai_dcc_face_1.region_jRUm;
	dcc_d_dcc_ai_24_reg.ai_region_jrdm_1= ptr->ai_dcc_face_1.region_jRDm;
	dcc_d_dcc_ai_8_reg.ai_region_start_i_1 = ptr->ai_dcc_face_1.region_start_i;
	dcc_d_dcc_ai_8_reg.ai_region_end_i_1 = ptr->ai_dcc_face_1.region_end_i;
	dcc_d_dcc_ai_14_reg.ai_region_start_j_1 = ptr->ai_dcc_face_1.region_start_j;
	dcc_d_dcc_ai_14_reg.ai_region_end_j_1 = ptr->ai_dcc_face_1.region_end_j;

	dcc_d_dcc_skin_tone_segment_c1_0_reg.c_blending_1_ratio_0=ptr->cds_dcc_face_1.c_blending_ratio_0;
	dcc_d_dcc_skin_tone_segment_c1_0_reg.c_blending_1_ratio_1=ptr->cds_dcc_face_1.c_blending_ratio_1;
	dcc_d_dcc_skin_tone_segment_c1_0_reg.c_blending_1_ratio_2=ptr->cds_dcc_face_1.c_blending_ratio_2;
	dcc_d_dcc_skin_tone_segment_c1_0_reg.c_blending_1_ratio_3=ptr->cds_dcc_face_1.c_blending_ratio_3;
	dcc_d_dcc_skin_tone_segment_c1_1_reg.c_blending_1_ratio_4=ptr->cds_dcc_face_1.c_blending_ratio_4;
	dcc_d_dcc_skin_tone_segment_c1_1_reg.c_blending_1_ratio_5=ptr->cds_dcc_face_1.c_blending_ratio_5;
	dcc_d_dcc_skin_tone_segment_c1_1_reg.c_blending_1_ratio_6=ptr->cds_dcc_face_1.c_blending_ratio_6;
	dcc_d_dcc_skin_tone_segment_c1_1_reg.c_blending_1_ratio_7=ptr->cds_dcc_face_1.c_blending_ratio_7;
	color_dcc_d_dcc_cds_skin_1_reg.cds_uv_range_1 = ptr->cds_dcc_face_1.uv_range;
	dcc_d_dcc_ai_2_reg.ai_center_u_1 = ptr->cds_dcc_face_1.u_center;
	dcc_d_dcc_ai_2_reg.ai_center_v_1 = ptr->cds_dcc_face_1.v_center;
	d_dcc_skin_tone_en_reg.y_blending_1_enhance_en = ptr->cds_dcc_face_1.region_enhance_en;


	//face2
	dcc_d_dcc_ai_3_reg.ai_face2_region_en = ptr->ai_dcc_face_2.region_en;
	color_dcc_d_dcc_ctrl_reg.ai_blending_2_inside_ratio_msb = (ptr->ai_dcc_face_2.ai_blending_inside_ratio)>>5;
	dcc_d_dcc_ai_0_reg.ai_blending_2_inside_ratio= (ptr->ai_dcc_face_2.ai_blending_inside_ratio)&0x1f;
	dcc_d_dcc_ai_3_reg.ai_shrink_pix_num_2 = ptr->ai_dcc_face_2.shrink_pix_num;
	dcc_d_dcc_ai_3_reg.ai_edge_pix_num_2 = ptr->ai_dcc_face_2.edge_pix_num;
	dcc_d_dcc_ai_25_reg.ai_region_ilu_2 = ptr->ai_dcc_face_2.region_iLU;
	dcc_d_dcc_ai_25_reg.ai_region_ild_2 = ptr->ai_dcc_face_2.region_iLD;
	dcc_d_dcc_ai_25_reg.ai_region_jlu_2 = ptr->ai_dcc_face_2.region_jLU;
	dcc_d_dcc_ai_25_reg.ai_region_jld_2 = ptr->ai_dcc_face_2.region_jLD;
	dcc_d_dcc_ai_26_reg.ai_region_iru_2 = ptr->ai_dcc_face_2.region_iRU;
	dcc_d_dcc_ai_26_reg.ai_region_ird_2 = ptr->ai_dcc_face_2.region_iRD;
	dcc_d_dcc_ai_26_reg.ai_region_jru_2 = ptr->ai_dcc_face_2.region_jRU;
	dcc_d_dcc_ai_26_reg.ai_region_jrd_2 = ptr->ai_dcc_face_2.region_jRD;
	dcc_d_dcc_ai_27_reg.ai_region_ilum_2= ptr->ai_dcc_face_2.region_iLUm;
	dcc_d_dcc_ai_27_reg.ai_region_ildm_2= ptr->ai_dcc_face_2.region_iLDm;
	dcc_d_dcc_ai_27_reg.ai_region_jlum_2= ptr->ai_dcc_face_2.region_jLUm;
	dcc_d_dcc_ai_27_reg.ai_region_jldm_2= ptr->ai_dcc_face_2.region_jLDm;
	dcc_d_dcc_ai_27_reg.ai_region_irum_2= ptr->ai_dcc_face_2.region_iRUm;
	dcc_d_dcc_ai_27_reg.ai_region_irdm_2= ptr->ai_dcc_face_2.region_iRDm;
	dcc_d_dcc_ai_27_reg.ai_region_jrum_2= ptr->ai_dcc_face_2.region_jRUm;
	dcc_d_dcc_ai_27_reg.ai_region_jrdm_2= ptr->ai_dcc_face_2.region_jRDm;
	dcc_d_dcc_ai_9_reg.ai_region_start_i_2 = ptr->ai_dcc_face_2.region_start_i;
	dcc_d_dcc_ai_9_reg.ai_region_end_i_2 = ptr->ai_dcc_face_2.region_end_i;
	dcc_d_dcc_ai_15_reg.ai_region_start_j_2 = ptr->ai_dcc_face_2.region_start_j;
	dcc_d_dcc_ai_15_reg.ai_region_end_j_2 = ptr->ai_dcc_face_2.region_end_j;

	dcc_d_dcc_skin_tone_segment_c2_0_reg.c_blending_2_ratio_0=ptr->cds_dcc_face_2.c_blending_ratio_0;
	dcc_d_dcc_skin_tone_segment_c2_0_reg.c_blending_2_ratio_1=ptr->cds_dcc_face_2.c_blending_ratio_1;
	dcc_d_dcc_skin_tone_segment_c2_0_reg.c_blending_2_ratio_2=ptr->cds_dcc_face_2.c_blending_ratio_2;
	dcc_d_dcc_skin_tone_segment_c2_0_reg.c_blending_2_ratio_3=ptr->cds_dcc_face_2.c_blending_ratio_3;
	dcc_d_dcc_skin_tone_segment_c2_1_reg.c_blending_2_ratio_4=ptr->cds_dcc_face_2.c_blending_ratio_4;
	dcc_d_dcc_skin_tone_segment_c2_1_reg.c_blending_2_ratio_5=ptr->cds_dcc_face_2.c_blending_ratio_5;
	dcc_d_dcc_skin_tone_segment_c2_1_reg.c_blending_2_ratio_6=ptr->cds_dcc_face_2.c_blending_ratio_6;
	dcc_d_dcc_skin_tone_segment_c2_1_reg.c_blending_2_ratio_7=ptr->cds_dcc_face_2.c_blending_ratio_7;
	color_dcc_d_dcc_cds_skin_1_reg.cds_uv_range_2 = ptr->cds_dcc_face_2.uv_range;
	dcc_d_dcc_ai_3_reg.ai_center_u_2 = ptr->cds_dcc_face_2.u_center;
	dcc_d_dcc_ai_3_reg.ai_center_v_2 = ptr->cds_dcc_face_2.v_center;
	d_dcc_skin_tone_en_reg.y_blending_2_enhance_en = ptr->cds_dcc_face_2.region_enhance_en;
	

	//face3
	dcc_d_dcc_ai_4_reg.ai_face3_region_en = ptr->ai_dcc_face_3.region_en;
	color_dcc_d_dcc_ctrl_reg.ai_blending_3_inside_ratio_msb = (ptr->ai_dcc_face_3.ai_blending_inside_ratio)>>5;
	dcc_d_dcc_ai_0_reg.ai_blending_3_inside_ratio= (ptr->ai_dcc_face_3.ai_blending_inside_ratio)&0x1f;
	dcc_d_dcc_ai_4_reg.ai_shrink_pix_num_3 = ptr->ai_dcc_face_3.shrink_pix_num;
	dcc_d_dcc_ai_4_reg.ai_edge_pix_num_3 = ptr->ai_dcc_face_3.edge_pix_num;
	dcc_d_dcc_ai_28_reg.ai_region_ilu_3 = ptr->ai_dcc_face_3.region_iLU;
	dcc_d_dcc_ai_28_reg.ai_region_ild_3 = ptr->ai_dcc_face_3.region_iLD;
	dcc_d_dcc_ai_28_reg.ai_region_jlu_3 = ptr->ai_dcc_face_3.region_jLU;
	dcc_d_dcc_ai_28_reg.ai_region_jld_3 = ptr->ai_dcc_face_3.region_jLD;
	dcc_d_dcc_ai_29_reg.ai_region_iru_3 = ptr->ai_dcc_face_3.region_iRU;
	dcc_d_dcc_ai_29_reg.ai_region_ird_3 = ptr->ai_dcc_face_3.region_iRD;
	dcc_d_dcc_ai_29_reg.ai_region_jru_3 = ptr->ai_dcc_face_3.region_jRU;
	dcc_d_dcc_ai_29_reg.ai_region_jrd_3 = ptr->ai_dcc_face_3.region_jRD;
	dcc_d_dcc_ai_30_reg.ai_region_ilum_3= ptr->ai_dcc_face_3.region_iLUm;
	dcc_d_dcc_ai_30_reg.ai_region_ildm_3= ptr->ai_dcc_face_3.region_iLDm;
	dcc_d_dcc_ai_30_reg.ai_region_jlum_3= ptr->ai_dcc_face_3.region_jLUm;
	dcc_d_dcc_ai_30_reg.ai_region_jldm_3= ptr->ai_dcc_face_3.region_jLDm;
	dcc_d_dcc_ai_30_reg.ai_region_irum_3= ptr->ai_dcc_face_3.region_iRUm;
	dcc_d_dcc_ai_30_reg.ai_region_irdm_3= ptr->ai_dcc_face_3.region_iRDm;
	dcc_d_dcc_ai_30_reg.ai_region_jrum_3= ptr->ai_dcc_face_3.region_jRUm;
	dcc_d_dcc_ai_30_reg.ai_region_jrdm_3= ptr->ai_dcc_face_3.region_jRDm;
	dcc_d_dcc_ai_10_reg.ai_region_start_i_3 = ptr->ai_dcc_face_3.region_start_i;
	dcc_d_dcc_ai_10_reg.ai_region_end_i_3 = ptr->ai_dcc_face_3.region_end_i;
	dcc_d_dcc_ai_16_reg.ai_region_start_j_3 = ptr->ai_dcc_face_3.region_start_j;
	dcc_d_dcc_ai_16_reg.ai_region_end_j_3 = ptr->ai_dcc_face_3.region_end_j;

	dcc_d_dcc_skin_tone_segment_c3_0_reg.c_blending_3_ratio_0=ptr->cds_dcc_face_3.c_blending_ratio_0;
	dcc_d_dcc_skin_tone_segment_c3_0_reg.c_blending_3_ratio_1=ptr->cds_dcc_face_3.c_blending_ratio_1;
	dcc_d_dcc_skin_tone_segment_c3_0_reg.c_blending_3_ratio_2=ptr->cds_dcc_face_3.c_blending_ratio_2;
	dcc_d_dcc_skin_tone_segment_c3_0_reg.c_blending_3_ratio_3=ptr->cds_dcc_face_3.c_blending_ratio_3;
	dcc_d_dcc_skin_tone_segment_c3_1_reg.c_blending_3_ratio_4=ptr->cds_dcc_face_3.c_blending_ratio_4;
	dcc_d_dcc_skin_tone_segment_c3_1_reg.c_blending_3_ratio_5=ptr->cds_dcc_face_3.c_blending_ratio_5;
	dcc_d_dcc_skin_tone_segment_c3_1_reg.c_blending_3_ratio_6=ptr->cds_dcc_face_3.c_blending_ratio_6;
	dcc_d_dcc_skin_tone_segment_c3_1_reg.c_blending_3_ratio_7=ptr->cds_dcc_face_3.c_blending_ratio_7;
	color_dcc_d_dcc_cds_skin_2_reg.cds_uv_range_3 = ptr->cds_dcc_face_3.uv_range;
	dcc_d_dcc_ai_4_reg.ai_center_u_3 = ptr->cds_dcc_face_3.u_center;
	dcc_d_dcc_ai_4_reg.ai_center_v_3 = ptr->cds_dcc_face_3.v_center;
	d_dcc_skin_tone_en_reg.y_blending_3_enhance_en = ptr->cds_dcc_face_3.region_enhance_en;

	
	//face4
	dcc_d_dcc_ai_5_reg.ai_face4_region_en = ptr->ai_dcc_face_4.region_en;
	color_dcc_d_dcc_ctrl_reg.ai_blending_4_inside_ratio_msb = (ptr->ai_dcc_face_4.ai_blending_inside_ratio)>>5;
	dcc_d_dcc_ai_0_reg.ai_blending_4_inside_ratio= (ptr->ai_dcc_face_4.ai_blending_inside_ratio)&0x1f;
	dcc_d_dcc_ai_5_reg.ai_shrink_pix_num_4 = ptr->ai_dcc_face_4.shrink_pix_num;
	dcc_d_dcc_ai_5_reg.ai_edge_pix_num_4 = ptr->ai_dcc_face_4.edge_pix_num;	
	dcc_d_dcc_ai_31_reg.ai_region_ilu_4 = ptr->ai_dcc_face_4.region_iLU;
	dcc_d_dcc_ai_31_reg.ai_region_ild_4 = ptr->ai_dcc_face_4.region_iLD;
	dcc_d_dcc_ai_31_reg.ai_region_jlu_4 = ptr->ai_dcc_face_4.region_jLU;
	dcc_d_dcc_ai_31_reg.ai_region_jld_4 = ptr->ai_dcc_face_4.region_jLD;
	dcc_d_dcc_ai_32_reg.ai_region_iru_4 = ptr->ai_dcc_face_4.region_iRU;
	dcc_d_dcc_ai_32_reg.ai_region_ird_4 = ptr->ai_dcc_face_4.region_iRD;
	dcc_d_dcc_ai_32_reg.ai_region_jru_4 = ptr->ai_dcc_face_4.region_jRU;
	dcc_d_dcc_ai_32_reg.ai_region_jrd_4 = ptr->ai_dcc_face_4.region_jRD;
	dcc_d_dcc_ai_33_reg.ai_region_ilum_4= ptr->ai_dcc_face_4.region_iLUm;
	dcc_d_dcc_ai_33_reg.ai_region_ildm_4= ptr->ai_dcc_face_4.region_iLDm;
	dcc_d_dcc_ai_33_reg.ai_region_jlum_4= ptr->ai_dcc_face_4.region_jLUm;
	dcc_d_dcc_ai_33_reg.ai_region_jldm_4= ptr->ai_dcc_face_4.region_jLDm;
	dcc_d_dcc_ai_33_reg.ai_region_irum_4= ptr->ai_dcc_face_4.region_iRUm;
	dcc_d_dcc_ai_33_reg.ai_region_irdm_4= ptr->ai_dcc_face_4.region_iRDm;
	dcc_d_dcc_ai_33_reg.ai_region_jrum_4= ptr->ai_dcc_face_4.region_jRUm;
	dcc_d_dcc_ai_33_reg.ai_region_jrdm_4= ptr->ai_dcc_face_4.region_jRDm;
	dcc_d_dcc_ai_11_reg.ai_region_start_i_4 = ptr->ai_dcc_face_4.region_start_i;
	dcc_d_dcc_ai_11_reg.ai_region_end_i_4 = ptr->ai_dcc_face_4.region_end_i;
	dcc_d_dcc_ai_17_reg.ai_region_start_j_4 = ptr->ai_dcc_face_4.region_start_j;
	dcc_d_dcc_ai_17_reg.ai_region_end_j_4 = ptr->ai_dcc_face_4.region_end_j;
	
	dcc_d_dcc_skin_tone_segment_c4_0_reg.c_blending_4_ratio_0=ptr->cds_dcc_face_4.c_blending_ratio_0;
	dcc_d_dcc_skin_tone_segment_c4_0_reg.c_blending_4_ratio_1=ptr->cds_dcc_face_4.c_blending_ratio_1;
	dcc_d_dcc_skin_tone_segment_c4_0_reg.c_blending_4_ratio_2=ptr->cds_dcc_face_4.c_blending_ratio_2;
	dcc_d_dcc_skin_tone_segment_c4_0_reg.c_blending_4_ratio_3=ptr->cds_dcc_face_4.c_blending_ratio_3;
	dcc_d_dcc_skin_tone_segment_c4_1_reg.c_blending_4_ratio_4=ptr->cds_dcc_face_4.c_blending_ratio_4;
	dcc_d_dcc_skin_tone_segment_c4_1_reg.c_blending_4_ratio_5=ptr->cds_dcc_face_4.c_blending_ratio_5;
	dcc_d_dcc_skin_tone_segment_c4_1_reg.c_blending_4_ratio_6=ptr->cds_dcc_face_4.c_blending_ratio_6;
	dcc_d_dcc_skin_tone_segment_c4_1_reg.c_blending_4_ratio_7=ptr->cds_dcc_face_4.c_blending_ratio_7;
	color_dcc_d_dcc_cds_skin_2_reg.cds_uv_range_4 = ptr->cds_dcc_face_4.uv_range;
	dcc_d_dcc_ai_5_reg.ai_center_u_4 = ptr->cds_dcc_face_4.u_center;
	dcc_d_dcc_ai_5_reg.ai_center_v_4 = ptr->cds_dcc_face_4.v_center;
	d_dcc_skin_tone_en_reg.y_blending_4_enhance_en = ptr->cds_dcc_face_4.region_enhance_en;
	

	//face5
	dcc_d_dcc_ai_6_reg.ai_face5_region_en = ptr->ai_dcc_face_5.region_en;
	color_dcc_d_dcc_ctrl_reg.ai_blending_5_inside_ratio_msb = (ptr->ai_dcc_face_5.ai_blending_inside_ratio)>>5;
	dcc_d_dcc_ai_0_reg.ai_blending_5_inside_ratio= (ptr->ai_dcc_face_5.ai_blending_inside_ratio)&0x1f;
	dcc_d_dcc_ai_6_reg.ai_shrink_pix_num_5 = ptr->ai_dcc_face_5.shrink_pix_num;
	dcc_d_dcc_ai_6_reg.ai_edge_pix_num_5 = ptr->ai_dcc_face_5.edge_pix_num;
	dcc_d_dcc_ai_34_reg.ai_region_ilu_5 = ptr->ai_dcc_face_5.region_iLU;
	dcc_d_dcc_ai_34_reg.ai_region_ild_5 = ptr->ai_dcc_face_5.region_iLD;
	dcc_d_dcc_ai_34_reg.ai_region_jlu_5 = ptr->ai_dcc_face_5.region_jLU;
	dcc_d_dcc_ai_34_reg.ai_region_jld_5 = ptr->ai_dcc_face_5.region_jLD;
	dcc_d_dcc_ai_35_reg.ai_region_iru_5 = ptr->ai_dcc_face_5.region_iRU;
	dcc_d_dcc_ai_35_reg.ai_region_ird_5 = ptr->ai_dcc_face_5.region_iRD;
	dcc_d_dcc_ai_35_reg.ai_region_jru_5 = ptr->ai_dcc_face_5.region_jRU;
	dcc_d_dcc_ai_35_reg.ai_region_jrd_5 = ptr->ai_dcc_face_5.region_jRD;
	dcc_d_dcc_ai_36_reg.ai_region_ilum_5= ptr->ai_dcc_face_5.region_iLUm;
	dcc_d_dcc_ai_36_reg.ai_region_ildm_5= ptr->ai_dcc_face_5.region_iLDm;
	dcc_d_dcc_ai_36_reg.ai_region_jlum_5= ptr->ai_dcc_face_5.region_jLUm;
	dcc_d_dcc_ai_36_reg.ai_region_jldm_5= ptr->ai_dcc_face_5.region_jLDm;
	dcc_d_dcc_ai_36_reg.ai_region_irum_5= ptr->ai_dcc_face_5.region_iRUm;
	dcc_d_dcc_ai_36_reg.ai_region_irdm_5= ptr->ai_dcc_face_5.region_iRDm;
	dcc_d_dcc_ai_36_reg.ai_region_jrum_5= ptr->ai_dcc_face_5.region_jRUm;
	dcc_d_dcc_ai_36_reg.ai_region_jrdm_5= ptr->ai_dcc_face_5.region_jRDm;
	dcc_d_dcc_ai_12_reg.ai_region_start_i_5 = ptr->ai_dcc_face_5.region_start_i;
	dcc_d_dcc_ai_12_reg.ai_region_end_i_5 = ptr->ai_dcc_face_5.region_end_i;
	dcc_d_dcc_ai_18_reg.ai_region_start_j_5 = ptr->ai_dcc_face_5.region_start_j;
	dcc_d_dcc_ai_18_reg.ai_region_end_j_5 = ptr->ai_dcc_face_5.region_end_j;
	
	dcc_d_dcc_skin_tone_segment_c5_0_reg.c_blending_5_ratio_0=ptr->cds_dcc_face_5.c_blending_ratio_0;
	dcc_d_dcc_skin_tone_segment_c5_0_reg.c_blending_5_ratio_1=ptr->cds_dcc_face_5.c_blending_ratio_1;
	dcc_d_dcc_skin_tone_segment_c5_0_reg.c_blending_5_ratio_2=ptr->cds_dcc_face_5.c_blending_ratio_2;
	dcc_d_dcc_skin_tone_segment_c5_0_reg.c_blending_5_ratio_3=ptr->cds_dcc_face_5.c_blending_ratio_3;
	dcc_d_dcc_skin_tone_segment_c5_1_reg.c_blending_5_ratio_4=ptr->cds_dcc_face_5.c_blending_ratio_4;
	dcc_d_dcc_skin_tone_segment_c5_1_reg.c_blending_5_ratio_5=ptr->cds_dcc_face_5.c_blending_ratio_5;
	dcc_d_dcc_skin_tone_segment_c5_1_reg.c_blending_5_ratio_6=ptr->cds_dcc_face_5.c_blending_ratio_6;
	dcc_d_dcc_skin_tone_segment_c5_1_reg.c_blending_5_ratio_7=ptr->cds_dcc_face_5.c_blending_ratio_7;
	color_dcc_d_dcc_cds_skin_2_reg.cds_uv_range_5 = ptr->cds_dcc_face_5.uv_range;
	dcc_d_dcc_ai_6_reg.ai_center_u_5 = ptr->cds_dcc_face_5.u_center;
	dcc_d_dcc_ai_6_reg.ai_center_v_5 = ptr->cds_dcc_face_5.v_center;
	d_dcc_skin_tone_en_reg.y_blending_5_enhance_en = ptr->cds_dcc_face_5.region_enhance_en;


	IoReg_Write32(COLOR_DCC_D_DCC_AI_0_reg, dcc_d_dcc_ai_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_1_reg, dcc_d_dcc_ai_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_2_reg, dcc_d_dcc_ai_2_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_3_reg, dcc_d_dcc_ai_3_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_4_reg, dcc_d_dcc_ai_4_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_5_reg, dcc_d_dcc_ai_5_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_6_reg, dcc_d_dcc_ai_6_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_7_reg, dcc_d_dcc_ai_7_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_8_reg, dcc_d_dcc_ai_8_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_9_reg, dcc_d_dcc_ai_9_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_10_reg, dcc_d_dcc_ai_10_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_11_reg, dcc_d_dcc_ai_11_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_12_reg, dcc_d_dcc_ai_12_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_13_reg, dcc_d_dcc_ai_13_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_14_reg, dcc_d_dcc_ai_14_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_15_reg, dcc_d_dcc_ai_15_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_16_reg, dcc_d_dcc_ai_16_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_17_reg, dcc_d_dcc_ai_17_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_18_reg, dcc_d_dcc_ai_18_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_19_reg, dcc_d_dcc_ai_19_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_20_reg, dcc_d_dcc_ai_20_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_21_reg, dcc_d_dcc_ai_21_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_22_reg, dcc_d_dcc_ai_22_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_23_reg, dcc_d_dcc_ai_23_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_24_reg, dcc_d_dcc_ai_24_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_25_reg, dcc_d_dcc_ai_25_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_26_reg, dcc_d_dcc_ai_26_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_27_reg, dcc_d_dcc_ai_27_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_28_reg, dcc_d_dcc_ai_28_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_29_reg, dcc_d_dcc_ai_29_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_30_reg, dcc_d_dcc_ai_30_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_31_reg, dcc_d_dcc_ai_31_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_32_reg, dcc_d_dcc_ai_32_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_33_reg, dcc_d_dcc_ai_33_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_34_reg, dcc_d_dcc_ai_34_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_35_reg, dcc_d_dcc_ai_35_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_AI_36_reg, dcc_d_dcc_ai_36_reg.regValue);
	
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_0_reg, dcc_d_dcc_skin_tone_segment_c0_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_1_reg, dcc_d_dcc_skin_tone_segment_c0_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_0_reg, dcc_d_dcc_skin_tone_segment_c1_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_1_reg, dcc_d_dcc_skin_tone_segment_c1_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_0_reg, dcc_d_dcc_skin_tone_segment_c2_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_1_reg, dcc_d_dcc_skin_tone_segment_c2_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_0_reg, dcc_d_dcc_skin_tone_segment_c3_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_1_reg, dcc_d_dcc_skin_tone_segment_c3_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C4_0_reg, dcc_d_dcc_skin_tone_segment_c4_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C4_1_reg, dcc_d_dcc_skin_tone_segment_c4_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C5_0_reg, dcc_d_dcc_skin_tone_segment_c5_0_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C5_1_reg, dcc_d_dcc_skin_tone_segment_c5_1_reg.regValue);

	IoReg_Write32(COLOR_DCC_D_DCC_CDS_SKIN_1_reg ,  color_dcc_d_dcc_cds_skin_1_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_CDS_SKIN_2_reg ,  color_dcc_d_dcc_cds_skin_2_reg.regValue);
	IoReg_Write32(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg ,  d_dcc_skin_tone_en_reg.regValue );
	IoReg_Write32(COLOR_DCC_D_DCC_CTRL_reg ,  color_dcc_d_dcc_ctrl_reg.regValue );

}

// vip tabe 1270 set, for scene_detection
void scaler_AI_Scene_Ctrl_Set(DRV_AI_SCENE_Ctrl_table *ptr) // read from sharing memory
{
		int ip, i;
		
        if (ptr==NULL)
			return;

//(0) global setting
        ai_scene_ctrl.ai_scene_global.scene_en = ptr->ai_scene_global.scene_en;
        ai_scene_ctrl.ai_scene_global.scene_cnt_method = ptr->ai_scene_global.scene_cnt_method;

        ai_scene_ctrl.ai_scene_global.bdtop_th = ptr->ai_scene_global.bdtop_th;
        ai_scene_ctrl.ai_scene_global.bdbot_th = ptr->ai_scene_global.bdbot_th;

        ai_scene_ctrl.ai_scene_global.osd_pq_scene_cnt_th = ptr->ai_scene_global.osd_pq_scene_cnt_th;

		ai_scene_ctrl.ai_scene_global.crop_top = ptr->ai_scene_global.crop_top;
		ai_scene_ctrl.ai_scene_global.crop_bot = ptr->ai_scene_global.crop_bot;
		ai_scene_ctrl.ai_scene_global.crop_left = ptr->ai_scene_global.crop_left;
		ai_scene_ctrl.ai_scene_global.crop_right = ptr->ai_scene_global.crop_right;
		ai_scene_ctrl.ai_scene_global.adj_top_en = ptr->ai_scene_global.adj_top_en;
//(1) change speed
		for(ip=0; ip<PQ_IP_TYPE_NUM; ip++)
		{
			for(i=0; i<PQ_TUNING_NUM; i++)
			{
				ai_scene_ctrl.change_speed[ip][i] = ptr->change_speed[ip][i];
			}
		}
			
}

// vip tabe 1270 get, for scene_detection
void scaler_AI_Scene_Ctrl_Get(DRV_AI_SCENE_Ctrl_table *ptr) // read from sharing memory
{
		int ip, i;
		
        if (ptr==NULL)
			return;

		memset(ptr, 0, sizeof(DRV_AI_SCENE_Ctrl_table));

//(0) global setting
        ptr->ai_scene_global.scene_en = ai_scene_ctrl.ai_scene_global.scene_en;
        ptr->ai_scene_global.scene_cnt_method = ai_scene_ctrl.ai_scene_global.scene_cnt_method;

        ptr->ai_scene_global.bdtop_th = ai_scene_ctrl.ai_scene_global.bdtop_th;
        ptr->ai_scene_global.bdbot_th = ai_scene_ctrl.ai_scene_global.bdbot_th;

        ptr->ai_scene_global.osd_pq_scene_cnt_th = ai_scene_ctrl.ai_scene_global.osd_pq_scene_cnt_th;
		
		ptr->ai_scene_global.crop_top = ai_scene_ctrl.ai_scene_global.crop_top;
		ptr->ai_scene_global.crop_bot = ai_scene_ctrl.ai_scene_global.crop_bot;
		ptr->ai_scene_global.crop_left = ai_scene_ctrl.ai_scene_global.crop_left;
		ptr->ai_scene_global.crop_right = ai_scene_ctrl.ai_scene_global.crop_right;
		ptr->ai_scene_global.adj_top_en = ai_scene_ctrl.ai_scene_global.adj_top_en;
//(1) change speed
		for(ip=0; ip<PQ_IP_TYPE_NUM; ip++)
		{
			for(i=0; i<PQ_TUNING_NUM; i++)
			{
				ptr->change_speed[ip][i] = ai_scene_ctrl.change_speed[ip][i];
			}
		}
}

// for face effect to set shp cds cm0.
void drvif_color_AI_obj_srp_init(unsigned char enable_cm0)
{
	color_sharp_dm_cds_cm0_u_bound_0_RBUS color_sharp_dm_cds_cm0_u_bound_0_reg;
	//color_sharp_shp_cds_region_enable_RBUS color_sharp_shp_cds_region_enable_reg;

	color_sharp_dm_cds_cm0_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg);
	//color_sharp_shp_cds_region_enable_reg.regValue = IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg);

	color_sharp_dm_cds_cm0_u_bound_0_reg.cds_cm0_enable = enable_cm0;
	//color_sharp_shp_cds_region_enable_reg.cds_region_0_enable = enable_cm0; // [0] region condition, dynamic control(AI_Ctrl_TBL(34-4,34-5): (0,2):global, (0,0)(0,1):region, (1,x):dynamic)

	IoReg_Write32(COLOR_SHARP_DM_CDS_CM0_U_BOUND_0_reg, color_sharp_dm_cds_cm0_u_bound_0_reg.regValue);
	//IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg, color_sharp_shp_cds_region_enable_reg.regValue);
}

// for dynamic scene detect to set shp cds cm1, cm2.
void drvif_color_AI_scene_srp_init(unsigned char enable_cm1, unsigned char enable_cm2)
{
	/* scene detect PQ using cm1 */
	color_sharp_dm_cds_cm1_u_bound_0_RBUS color_sharp_dm_cds_cm1_u_bound_0_reg;
	color_sharp_shp_cds_region_enable_RBUS color_sharp_shp_cds_region_enable_reg;
	color_sharp_dm_cds_cm2_u_bound_0_RBUS color_sharp_dm_cds_cm2_u_bound_0_reg;
	
	color_sharp_dm_cds_cm1_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg);
	color_sharp_shp_cds_region_enable_reg.regValue = IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg);
	color_sharp_dm_cds_cm2_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_0_reg);

	color_sharp_dm_cds_cm1_u_bound_0_reg.cds_cm1_enable = enable_cm1;
	color_sharp_shp_cds_region_enable_reg.cds_region_1_enable = 0; // [1] cm1 region condition
	color_sharp_dm_cds_cm2_u_bound_0_reg.cds_cm2_enable = enable_cm2;
	color_sharp_shp_cds_region_enable_reg.cds_region_2_enable = 0; // [2] cm2 region condition

	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg, color_sharp_dm_cds_cm1_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg, color_sharp_shp_cds_region_enable_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_0_reg, color_sharp_dm_cds_cm2_u_bound_0_reg.regValue);
}
void drvif_color_AI_scene_cdsharp_set_cm1(unsigned char mode, unsigned int cds_gain[15])
{	
#if 0 // mac7p AI TBD
	/* for scene PQ use cm1, and doesn't need region function */
	
	color_sharp_dm_cds_cm1_u_bound_0_RBUS color_sharp_dm_cds_cm1_u_bound_0_reg;
	color_sharp_dm_cds_cm1_v_bound_0_RBUS color_sharp_dm_cds_cm1_v_bound_0_reg;
	color_sharp_dm_cds_cm1_u_bound_1_RBUS color_sharp_dm_cds_cm1_u_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_RBUS color_sharp_dm_cds_peaking_gain_bound_ctrl_reg;
	color_sharp_shp_cds_region1_0_RBUS color_sharp_shp_cds_region1_0_reg;
	color_sharp_dm_cds_peaking_gain_cm1_edge_RBUS color_sharp_dm_cds_peaking_gain_cm1_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm1_edge_RBUS color_sharp_dm_cds_peaking_bound_cm1_edge_reg;
	color_sharp_dm_cds_peaking_gain_cm1_tex_RBUS color_sharp_dm_cds_peaking_gain_cm1_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm1_tex_RBUS color_sharp_dm_cds_peaking_bound_cm1_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm1_v_RBUS color_sharp_dm_cds_peaking_gain_cm1_v_reg;
	color_sharp_dm_cds_peaking_bound_cm1_v_RBUS color_sharp_dm_cds_peaking_bound_cm1_v_reg;
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_RBUS color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_RBUS color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg;
	
	color_sharp_dm_cds_cm1_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg);
	color_sharp_dm_cds_cm1_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_0_reg);
	color_sharp_dm_cds_cm1_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg);
	color_sharp_shp_cds_region1_0_reg.regValue = IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION1_0_reg);
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg);
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm1_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_reg);
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_reg);
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_EDG_reg);
	
	color_sharp_dm_cds_cm1_u_bound_1_reg.cu_1 = CDS_uv_range[mode][0];
	color_sharp_dm_cds_cm1_u_bound_1_reg.cv_1 = CDS_uv_range[mode][1];
	color_sharp_dm_cds_cm1_u_bound_0_reg.u_lb_1 = CDS_uv_range[mode][2];
	color_sharp_dm_cds_cm1_u_bound_0_reg.u_ub_1 = CDS_uv_range[mode][3];
	color_sharp_dm_cds_cm1_v_bound_0_reg.v_lb_1 = CDS_uv_range[mode][4];
	color_sharp_dm_cds_cm1_v_bound_0_reg.v_ub_1 = CDS_uv_range[mode][5];
	color_sharp_dm_cds_cm1_u_bound_1_reg.uv_rad_1 = CDS_uv_range[mode][6];
	color_sharp_dm_cds_cm1_v_bound_0_reg.cm1_mode = CDS_uv_range[mode][7];

	//color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_edg_enable = CDS_control[mode][0];
	//color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_tex_enable = CDS_control[mode][1];
	//color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_enable = CDS_control[mode][2];
	//color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_edg_enable = CDS_control[mode][3];
	color_sharp_dm_cds_cm1_v_bound_0_reg.cm1_conti_enable = CDS_control[mode][4];
	color_sharp_dm_cds_cm1_v_bound_0_reg.cm1_conti_area = CDS_control[mode][5];
	color_sharp_shp_cds_region1_0_reg.cds_region_1_smooth = CDS_control[mode][6];

	// Edge
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_pos = cds_gain[0];
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_neg = cds_gain[1];
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_pos = cds_gain[2];
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_neg = cds_gain[3];
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.lv = cds_gain[4];
	// text
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_pos = 0;
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_neg = 0;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_pos = 0;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_neg = 0;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.lv = 0;
	// vertical
	color_sharp_dm_cds_peaking_gain_cm1_v_reg.gain_pos = cds_gain[5]+80;
	color_sharp_dm_cds_peaking_gain_cm1_v_reg.gain_neg = cds_gain[6]+30;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.hv_pos = 30;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.hv_neg = 50;
	color_sharp_dm_cds_peaking_bound_cm1_v_reg.lv = 0;
	// vertical edge
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.gain_pos = cds_gain[5];
	color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.gain_neg = cds_gain[6];
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.hv_pos = 5;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.hv_neg = 5;
	color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.lv = cds_gain[9];
	
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_0_reg,color_sharp_dm_cds_cm1_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_V_BOUND_0_reg,color_sharp_dm_cds_cm1_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM1_U_BOUND_1_reg,color_sharp_dm_cds_cm1_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg,color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION1_0_reg,color_sharp_shp_cds_region1_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg,color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg,color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg,color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg,color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_reg,color_sharp_dm_cds_peaking_gain_cm1_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_reg,color_sharp_dm_cds_peaking_bound_cm1_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_V_EDG_reg,color_sharp_dm_cds_peaking_gain_cm1_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_V_EDG_reg,color_sharp_dm_cds_peaking_bound_cm1_v_edg_reg.regValue);
#endif
}
void drvif_color_AI_scene_cdsharp_set_cm2(unsigned char mode, unsigned int cds_gain[15])
{	
#if 0 // mac7p AI TBD
	/* for scene PQ use cm1, and doesn't need region function */
	
	color_sharp_dm_cds_cm2_u_bound_0_RBUS color_sharp_dm_cds_cm2_u_bound_0_reg;
	color_sharp_dm_cds_cm2_v_bound_0_RBUS color_sharp_dm_cds_cm2_v_bound_0_reg;
	color_sharp_dm_cds_cm2_u_bound_1_RBUS color_sharp_dm_cds_cm2_u_bound_1_reg;
	color_sharp_dm_cds_peaking_gain_bound_ctrl_RBUS color_sharp_dm_cds_peaking_gain_bound_ctrl_reg;
	color_sharp_shp_cds_region2_0_RBUS color_sharp_shp_cds_region2_0_reg;
	color_sharp_dm_cds_peaking_gain_cm2_edge_RBUS color_sharp_dm_cds_peaking_gain_cm2_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm2_edge_RBUS color_sharp_dm_cds_peaking_bound_cm2_edge_reg;
	color_sharp_dm_cds_peaking_gain_cm2_tex_RBUS color_sharp_dm_cds_peaking_gain_cm2_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm2_tex_RBUS color_sharp_dm_cds_peaking_bound_cm2_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm2_v_RBUS color_sharp_dm_cds_peaking_gain_cm2_v_reg;
	color_sharp_dm_cds_peaking_bound_cm2_v_RBUS color_sharp_dm_cds_peaking_bound_cm2_v_reg;
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_RBUS color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_RBUS color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg;
	
	color_sharp_dm_cds_cm2_u_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_0_reg);
	color_sharp_dm_cds_cm2_v_bound_0_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_0_reg);
	color_sharp_dm_cds_cm2_u_bound_1_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_1_reg);
	color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg);
	color_sharp_shp_cds_region2_0_reg.regValue = IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION2_0_reg);
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg);
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm2_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_reg);
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_reg);
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_EDG_reg);
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_EDG_reg);
	
	color_sharp_dm_cds_cm2_u_bound_1_reg.cu_2 = CDS_uv_range[mode][0];
	color_sharp_dm_cds_cm2_u_bound_1_reg.cv_2 = CDS_uv_range[mode][1];
	color_sharp_dm_cds_cm2_u_bound_0_reg.u_lb_2 = CDS_uv_range[mode][2];
	color_sharp_dm_cds_cm2_u_bound_0_reg.u_ub_2 = CDS_uv_range[mode][3];
	color_sharp_dm_cds_cm2_v_bound_0_reg.v_lb_2 = CDS_uv_range[mode][4];
	color_sharp_dm_cds_cm2_v_bound_0_reg.v_ub_2 = CDS_uv_range[mode][5];
	color_sharp_dm_cds_cm2_u_bound_1_reg.uv_rad_2 = CDS_uv_range[mode][6];
	color_sharp_dm_cds_cm2_v_bound_0_reg.cm2_mode = CDS_uv_range[mode][7];

	//color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_edg_enable = CDS_control[mode][0];
	//color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_tex_enable = CDS_control[mode][1];
	//color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_enable = CDS_control[mode][2];
	//color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.cds_v_edg_enable = CDS_control[mode][3];
	color_sharp_dm_cds_cm2_v_bound_0_reg.cm2_conti_enable = CDS_control[mode][4];
	color_sharp_dm_cds_cm2_v_bound_0_reg.cm2_conti_area = CDS_control[mode][5];
	color_sharp_shp_cds_region2_0_reg.cds_region_2_smooth = CDS_control[mode][6];

	// Edge
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_pos = cds_gain[0];
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_neg = cds_gain[1];
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_pos = 100;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_neg = 1023;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.lv = cds_gain[4];
	// text
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_pos = 0;
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_neg = 0;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_pos = 0;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_neg = 0;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.lv = 0;	
	// vertical
	color_sharp_dm_cds_peaking_gain_cm2_v_reg.gain_pos = 0;
	color_sharp_dm_cds_peaking_gain_cm2_v_reg.gain_neg = 0;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.hv_pos = 0;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.hv_neg = 0;
	color_sharp_dm_cds_peaking_bound_cm2_v_reg.lv = 0;
	// vertical edge
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.gain_pos = cds_gain[5];
	color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.gain_neg = cds_gain[6];
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.hv_pos = 120;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.hv_neg = 120;
	color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.lv = cds_gain[9];
	
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_0_reg,color_sharp_dm_cds_cm2_u_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_V_BOUND_0_reg,color_sharp_dm_cds_cm2_v_bound_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_CM2_U_BOUND_1_reg,color_sharp_dm_cds_cm2_u_bound_1_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_BOUND_CTRL_reg,color_sharp_dm_cds_peaking_gain_bound_ctrl_reg.regValue);
	IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION2_0_reg,color_sharp_shp_cds_region2_0_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg,color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg,color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg,color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg,color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_reg,color_sharp_dm_cds_peaking_gain_cm2_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_reg,color_sharp_dm_cds_peaking_bound_cm2_v_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_V_EDG_reg,color_sharp_dm_cds_peaking_gain_cm2_v_edg_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_V_EDG_reg,color_sharp_dm_cds_peaking_bound_cm2_v_edg_reg.regValue);
#endif
}
#endif // end AI config
