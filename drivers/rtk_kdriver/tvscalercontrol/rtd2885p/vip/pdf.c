/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2020
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author
 * @date 		2020/03/13
 * @version 	1
 */

/*============================ Module dependency  ===========================*/


#include "rtk_vip_logger.h"
#include <rtd_log/rtd_module_log.h>

#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/pdf.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/color.h>
#include <rbus/siw_pdf_reg.h>


/*================================ Global Variables ==============================*/
/*================================ Definitions ==============================*/
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_PCID_DEBUG,fmt,##args)
/*================================== Function ===============================*/
void drvif_color_set_pdf_enable(unsigned char bEnable)
{
	siw_pdf_pdf_fg_range_0_RBUS   siw_pdf_pdf_fg_range_0;
	siw_pdf_pdf_fg_range_0.regValue = IoReg_Read32(SIW_PDF_PDF_FG_RANGE_0_reg);
	siw_pdf_pdf_fg_range_0.pdf_en = ((bEnable!=0)? 1 : 0);
	IoReg_Write32(SIW_PDF_PDF_FG_RANGE_0_reg, siw_pdf_pdf_fg_range_0.regValue);
}

void drvif_color_set_pdf_fg_range(DRV_PDF_fg_range_t *pFG)
{
	siw_pdf_pdf_fg_range_0_RBUS   siw_pdf_pdf_fg_range_0;
	siw_pdf_pdf_fg_range_1_RBUS   siw_pdf_pdf_fg_range_1;

	siw_pdf_pdf_fg_range_0.regValue = IoReg_Read32(SIW_PDF_PDF_FG_RANGE_0_reg);
	siw_pdf_pdf_fg_range_1.regValue = IoReg_Read32(SIW_PDF_PDF_FG_RANGE_1_reg);

	siw_pdf_pdf_fg_range_0.pdf_range_top = pFG->range_top;
	siw_pdf_pdf_fg_range_0.pdf_range_bot = pFG->range_bot;
	siw_pdf_pdf_fg_range_1.pdf_range_left = pFG->range_left;
	siw_pdf_pdf_fg_range_1.pdf_range_right = pFG->range_right;
	
	IoReg_Write32(SIW_PDF_PDF_FG_RANGE_0_reg, siw_pdf_pdf_fg_range_0.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_RANGE_1_reg, siw_pdf_pdf_fg_range_1.regValue);
}

void drvif_color_set_pdf_fg_gl(DRV_PDF_fg_t *fg)
{
	siw_pdf_pdf_fg_gl_pt0_RBUS siw_pdf_pdf_fg_gl_pt0;
	siw_pdf_pdf_fg_gl_pt1_RBUS siw_pdf_pdf_fg_gl_pt1;
	siw_pdf_pdf_fg_gl_pt2_RBUS siw_pdf_pdf_fg_gl_pt2;
	siw_pdf_pdf_fg_gl_pt3_RBUS siw_pdf_pdf_fg_gl_pt3;
	siw_pdf_pdf_fg_gl_pt4_RBUS siw_pdf_pdf_fg_gl_pt4;
	siw_pdf_pdf_fg_gl_pt5_RBUS siw_pdf_pdf_fg_gl_pt5;
	siw_pdf_pdf_fg_gl_pt6_RBUS siw_pdf_pdf_fg_gl_pt6;
	siw_pdf_pdf_fg_gl_pt7_RBUS siw_pdf_pdf_fg_gl_pt7;

	siw_pdf_pdf_fg_gl_pt0.regValue = IoReg_Read32(SIW_PDF_PDF_FG_GL_PT0_reg);
	siw_pdf_pdf_fg_gl_pt1.regValue = IoReg_Read32(SIW_PDF_PDF_FG_GL_PT1_reg);
	siw_pdf_pdf_fg_gl_pt2.regValue = IoReg_Read32(SIW_PDF_PDF_FG_GL_PT2_reg);
	siw_pdf_pdf_fg_gl_pt3.regValue = IoReg_Read32(SIW_PDF_PDF_FG_GL_PT3_reg);
	siw_pdf_pdf_fg_gl_pt4.regValue = IoReg_Read32(SIW_PDF_PDF_FG_GL_PT4_reg);
	siw_pdf_pdf_fg_gl_pt5.regValue = IoReg_Read32(SIW_PDF_PDF_FG_GL_PT5_reg);
	siw_pdf_pdf_fg_gl_pt6.regValue = IoReg_Read32(SIW_PDF_PDF_FG_GL_PT6_reg);
	siw_pdf_pdf_fg_gl_pt7.regValue = IoReg_Read32(SIW_PDF_PDF_FG_GL_PT7_reg);

	siw_pdf_pdf_fg_gl_pt0.pdf_fg_gl_0_pt0 = fg->fg_gl[0].xtalk_fg_gl_0;
	siw_pdf_pdf_fg_gl_pt0.pdf_fg_gl_1_pt0 = fg->fg_gl[0].xtalk_fg_gl_1;
	siw_pdf_pdf_fg_gl_pt0.pdf_fg_gl_2_pt0 = fg->fg_gl[0].xtalk_fg_gl_2;
	siw_pdf_pdf_fg_gl_pt1.pdf_fg_gl_0_pt1 = fg->fg_gl[1].xtalk_fg_gl_0;
	siw_pdf_pdf_fg_gl_pt1.pdf_fg_gl_1_pt1 = fg->fg_gl[1].xtalk_fg_gl_1;
	siw_pdf_pdf_fg_gl_pt1.pdf_fg_gl_2_pt1 = fg->fg_gl[1].xtalk_fg_gl_2;
	siw_pdf_pdf_fg_gl_pt2.pdf_fg_gl_0_pt2 = fg->fg_gl[2].xtalk_fg_gl_0;
	siw_pdf_pdf_fg_gl_pt2.pdf_fg_gl_1_pt2 = fg->fg_gl[2].xtalk_fg_gl_1;
	siw_pdf_pdf_fg_gl_pt2.pdf_fg_gl_2_pt2 = fg->fg_gl[2].xtalk_fg_gl_2;
	siw_pdf_pdf_fg_gl_pt3.pdf_fg_gl_0_pt3 = fg->fg_gl[3].xtalk_fg_gl_0;
	siw_pdf_pdf_fg_gl_pt3.pdf_fg_gl_1_pt3 = fg->fg_gl[3].xtalk_fg_gl_1;
	siw_pdf_pdf_fg_gl_pt3.pdf_fg_gl_2_pt3 = fg->fg_gl[3].xtalk_fg_gl_2;
	siw_pdf_pdf_fg_gl_pt4.pdf_fg_gl_0_pt4 = fg->fg_gl[4].xtalk_fg_gl_0;
	siw_pdf_pdf_fg_gl_pt4.pdf_fg_gl_1_pt4 = fg->fg_gl[4].xtalk_fg_gl_1;
	siw_pdf_pdf_fg_gl_pt4.pdf_fg_gl_2_pt4 = fg->fg_gl[4].xtalk_fg_gl_2;
	siw_pdf_pdf_fg_gl_pt5.pdf_fg_gl_0_pt5 = fg->fg_gl[5].xtalk_fg_gl_0;
	siw_pdf_pdf_fg_gl_pt5.pdf_fg_gl_1_pt5 = fg->fg_gl[5].xtalk_fg_gl_1;
	siw_pdf_pdf_fg_gl_pt5.pdf_fg_gl_2_pt5 = fg->fg_gl[5].xtalk_fg_gl_2;
	siw_pdf_pdf_fg_gl_pt6.pdf_fg_gl_0_pt6 = fg->fg_gl[6].xtalk_fg_gl_0;
	siw_pdf_pdf_fg_gl_pt6.pdf_fg_gl_1_pt6 = fg->fg_gl[6].xtalk_fg_gl_1;
	siw_pdf_pdf_fg_gl_pt6.pdf_fg_gl_2_pt6 = fg->fg_gl[6].xtalk_fg_gl_2;
	siw_pdf_pdf_fg_gl_pt7.pdf_fg_gl_0_pt7 = fg->fg_gl[7].xtalk_fg_gl_0;
	siw_pdf_pdf_fg_gl_pt7.pdf_fg_gl_1_pt7 = fg->fg_gl[7].xtalk_fg_gl_1;
	siw_pdf_pdf_fg_gl_pt7.pdf_fg_gl_2_pt7 = fg->fg_gl[7].xtalk_fg_gl_2;

	IoReg_Write32(SIW_PDF_PDF_FG_GL_PT0_reg, siw_pdf_pdf_fg_gl_pt0.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_GL_PT1_reg, siw_pdf_pdf_fg_gl_pt1.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_GL_PT2_reg, siw_pdf_pdf_fg_gl_pt2.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_GL_PT3_reg, siw_pdf_pdf_fg_gl_pt3.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_GL_PT4_reg, siw_pdf_pdf_fg_gl_pt4.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_GL_PT5_reg, siw_pdf_pdf_fg_gl_pt5.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_GL_PT6_reg, siw_pdf_pdf_fg_gl_pt6.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_GL_PT7_reg, siw_pdf_pdf_fg_gl_pt7.regValue);

}

void drvif_color_set_pdf_fg_temporal_det(DRV_PDF_fg_temporal_det_t *pfg_temporal_det)
{
	siw_pdf_pdf_fg_thl_0_RBUS   siw_pdf_pdf_fg_thl_0;
	siw_pdf_pdf_fg_thl_1_RBUS   siw_pdf_pdf_fg_thl_1;
	siw_pdf_pdf_fg_thl_2_RBUS   siw_pdf_pdf_fg_thl_2;
	siw_pdf_pdf_fg_thl_3_RBUS   siw_pdf_pdf_fg_thl_3;
	siw_pdf_pdf_fg_thl_4_RBUS   siw_pdf_pdf_fg_thl_4;
	siw_pdf_pdf_fg_thl_5_RBUS   siw_pdf_pdf_fg_thl_5;
	siw_pdf_pdf_fg_thl_6_RBUS   siw_pdf_pdf_fg_thl_6;
	siw_pdf_pdf_fg_thl_7_RBUS   siw_pdf_pdf_fg_thl_7;

	siw_pdf_pdf_fg_thl_0.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_0_reg);
	siw_pdf_pdf_fg_thl_1.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_1_reg);
	siw_pdf_pdf_fg_thl_2.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_2_reg);
	siw_pdf_pdf_fg_thl_3.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_3_reg);
	siw_pdf_pdf_fg_thl_4.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_4_reg);
	siw_pdf_pdf_fg_thl_5.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_5_reg);
	siw_pdf_pdf_fg_thl_6.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_6_reg);
	siw_pdf_pdf_fg_thl_7.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_7_reg);

	siw_pdf_pdf_fg_thl_0.pdf_fg_frame_det_threshold_0 = pfg_temporal_det->xtalk_fg_det_threshold_0;
	siw_pdf_pdf_fg_thl_1.pdf_fg_frame_det_threshold_1 = pfg_temporal_det->xtalk_fg_det_threshold_1;
	siw_pdf_pdf_fg_thl_2.pdf_fg_frame_det_threshold_2 = pfg_temporal_det->xtalk_fg_det_threshold_2;
	siw_pdf_pdf_fg_thl_3.pdf_fg_frame_det_threshold_3 = pfg_temporal_det->xtalk_fg_det_threshold_3;
	siw_pdf_pdf_fg_thl_4.pdf_fg_frame_det_threshold_4 = pfg_temporal_det->xtalk_fg_det_threshold_4;
	siw_pdf_pdf_fg_thl_5.pdf_fg_frame_det_threshold_5 = pfg_temporal_det->xtalk_fg_det_threshold_5;
	siw_pdf_pdf_fg_thl_6.pdf_fg_frame_det_threshold_6 = pfg_temporal_det->xtalk_fg_det_threshold_6;
	siw_pdf_pdf_fg_thl_7.pdf_fg_frame_det_threshold_7 = pfg_temporal_det->xtalk_fg_det_threshold_7;

	IoReg_Write32(SIW_PDF_PDF_FG_THL_0_reg, siw_pdf_pdf_fg_thl_0.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_1_reg, siw_pdf_pdf_fg_thl_1.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_2_reg, siw_pdf_pdf_fg_thl_2.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_3_reg, siw_pdf_pdf_fg_thl_3.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_4_reg, siw_pdf_pdf_fg_thl_4.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_5_reg, siw_pdf_pdf_fg_thl_5.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_6_reg, siw_pdf_pdf_fg_thl_6.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_7_reg, siw_pdf_pdf_fg_thl_7.regValue);
}


void drvif_color_set_pdf_fg_temporal_undet(DRV_PDF_fg_temporal_undet_t *pfg_temporal_undet)
{
	siw_pdf_pdf_fg_thl_8_RBUS   siw_pdf_pdf_fg_thl_8;
	siw_pdf_pdf_fg_thl_9_RBUS   siw_pdf_pdf_fg_thl_9;

	siw_pdf_pdf_fg_thl_8.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_8_reg);
	siw_pdf_pdf_fg_thl_9.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_9_reg);

	siw_pdf_pdf_fg_thl_8.pdf_fg_frame_undet_threshold_0 = pfg_temporal_undet->xtalk_fg_undet_threshold_0;
	siw_pdf_pdf_fg_thl_8.pdf_fg_frame_undet_threshold_1 = pfg_temporal_undet->xtalk_fg_undet_threshold_1;
	siw_pdf_pdf_fg_thl_8.pdf_fg_frame_undet_threshold_2 = pfg_temporal_undet->xtalk_fg_undet_threshold_2;
	siw_pdf_pdf_fg_thl_8.pdf_fg_frame_undet_threshold_3 = pfg_temporal_undet->xtalk_fg_undet_threshold_3;
	siw_pdf_pdf_fg_thl_9.pdf_fg_frame_undet_threshold_4 = pfg_temporal_undet->xtalk_fg_undet_threshold_4;
	siw_pdf_pdf_fg_thl_9.pdf_fg_frame_undet_threshold_5 = pfg_temporal_undet->xtalk_fg_undet_threshold_5;
	siw_pdf_pdf_fg_thl_9.pdf_fg_frame_undet_threshold_6 = pfg_temporal_undet->xtalk_fg_undet_threshold_6;
	siw_pdf_pdf_fg_thl_9.pdf_fg_frame_undet_threshold_7 = pfg_temporal_undet->xtalk_fg_undet_threshold_7;

	IoReg_Write32(SIW_PDF_PDF_FG_THL_8_reg, siw_pdf_pdf_fg_thl_8.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_9_reg, siw_pdf_pdf_fg_thl_9.regValue);
}

void drvif_color_set_pdf_fg_area_hit(DRV_PDF_fg_area_hit_t *pfg_area_hit)
{
	siw_pdf_pdf_fg_thl_0_RBUS   siw_pdf_pdf_fg_thl_0;
	siw_pdf_pdf_fg_thl_1_RBUS   siw_pdf_pdf_fg_thl_1;
	siw_pdf_pdf_fg_thl_2_RBUS   siw_pdf_pdf_fg_thl_2;
	siw_pdf_pdf_fg_thl_3_RBUS   siw_pdf_pdf_fg_thl_3;
	siw_pdf_pdf_fg_thl_4_RBUS   siw_pdf_pdf_fg_thl_4;
	siw_pdf_pdf_fg_thl_5_RBUS   siw_pdf_pdf_fg_thl_5;
	siw_pdf_pdf_fg_thl_6_RBUS   siw_pdf_pdf_fg_thl_6;
	siw_pdf_pdf_fg_thl_7_RBUS   siw_pdf_pdf_fg_thl_7;

	siw_pdf_pdf_fg_thl_0.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_0_reg);
	siw_pdf_pdf_fg_thl_1.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_1_reg);
	siw_pdf_pdf_fg_thl_2.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_2_reg);
	siw_pdf_pdf_fg_thl_3.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_3_reg);
	siw_pdf_pdf_fg_thl_4.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_4_reg);
	siw_pdf_pdf_fg_thl_5.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_5_reg);
	siw_pdf_pdf_fg_thl_6.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_6_reg);
	siw_pdf_pdf_fg_thl_7.regValue = IoReg_Read32(SIW_PDF_PDF_FG_THL_7_reg);

	siw_pdf_pdf_fg_thl_0.pdf_fg_hit_threshold_0 = pfg_area_hit->xtalk_fg_hit_threshold_0;
	siw_pdf_pdf_fg_thl_1.pdf_fg_hit_threshold_1 = pfg_area_hit->xtalk_fg_hit_threshold_1;
	siw_pdf_pdf_fg_thl_2.pdf_fg_hit_threshold_2 = pfg_area_hit->xtalk_fg_hit_threshold_2;
	siw_pdf_pdf_fg_thl_3.pdf_fg_hit_threshold_3 = pfg_area_hit->xtalk_fg_hit_threshold_3;
	siw_pdf_pdf_fg_thl_4.pdf_fg_hit_threshold_4 = pfg_area_hit->xtalk_fg_hit_threshold_4;
	siw_pdf_pdf_fg_thl_5.pdf_fg_hit_threshold_5 = pfg_area_hit->xtalk_fg_hit_threshold_5;
	siw_pdf_pdf_fg_thl_6.pdf_fg_hit_threshold_6 = pfg_area_hit->xtalk_fg_hit_threshold_6;
	siw_pdf_pdf_fg_thl_7.pdf_fg_hit_threshold_7 = pfg_area_hit->xtalk_fg_hit_threshold_7;

	IoReg_Write32(SIW_PDF_PDF_FG_THL_0_reg, siw_pdf_pdf_fg_thl_0.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_1_reg, siw_pdf_pdf_fg_thl_1.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_2_reg, siw_pdf_pdf_fg_thl_2.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_3_reg, siw_pdf_pdf_fg_thl_3.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_4_reg, siw_pdf_pdf_fg_thl_4.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_5_reg, siw_pdf_pdf_fg_thl_5.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_6_reg, siw_pdf_pdf_fg_thl_6.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_THL_7_reg, siw_pdf_pdf_fg_thl_7.regValue);
}

void drvif_color_set_pdf_fg_gl_inUnit(DRV_PDF_fg_gl_inUnit_t *pfg_gl_inUnit)
{
	siw_pdf_pdf_fg_sel_gl_0_RBUS    siw_pdf_pdf_fg_sel_gl_0;
	siw_pdf_pdf_fg_sel_gl_1_RBUS    siw_pdf_pdf_fg_sel_gl_1;
	siw_pdf_pdf_fg_sel_gl_2_RBUS    siw_pdf_pdf_fg_sel_gl_2;
	siw_pdf_pdf_fg_sel_gl_3_RBUS    siw_pdf_pdf_fg_sel_gl_3;
	siw_pdf_pdf_fg_sel_gl_4_RBUS    siw_pdf_pdf_fg_sel_gl_4;
	siw_pdf_pdf_fg_sel_gl_5_RBUS    siw_pdf_pdf_fg_sel_gl_5;
	siw_pdf_pdf_fg_sel_gl_6_RBUS    siw_pdf_pdf_fg_sel_gl_6;
	siw_pdf_pdf_fg_sel_gl_7_RBUS    siw_pdf_pdf_fg_sel_gl_7;
	siw_pdf_pdf_fg_sel_gl_8_RBUS    siw_pdf_pdf_fg_sel_gl_8;
	siw_pdf_pdf_fg_sel_gl_9_RBUS    siw_pdf_pdf_fg_sel_gl_9;
	siw_pdf_pdf_fg_sel_gl_10_RBUS   siw_pdf_pdf_fg_sel_gl_10;
	siw_pdf_pdf_fg_sel_gl_11_RBUS   siw_pdf_pdf_fg_sel_gl_11;
	siw_pdf_pdf_fg_sel_gl_12_RBUS   siw_pdf_pdf_fg_sel_gl_12;
	siw_pdf_pdf_fg_sel_gl_13_RBUS   siw_pdf_pdf_fg_sel_gl_13;
	siw_pdf_pdf_fg_sel_gl_14_RBUS   siw_pdf_pdf_fg_sel_gl_14;
	siw_pdf_pdf_fg_sel_gl_15_RBUS   siw_pdf_pdf_fg_sel_gl_15;
	siw_pdf_pdf_fg_sel_gl_16_RBUS   siw_pdf_pdf_fg_sel_gl_16;
	siw_pdf_pdf_fg_sel_gl_17_RBUS   siw_pdf_pdf_fg_sel_gl_17;
	siw_pdf_pdf_fg_sel_gl_18_RBUS   siw_pdf_pdf_fg_sel_gl_18;
	siw_pdf_pdf_fg_sel_gl_19_RBUS   siw_pdf_pdf_fg_sel_gl_19;
	siw_pdf_pdf_fg_sel_gl_20_RBUS   siw_pdf_pdf_fg_sel_gl_20;
	siw_pdf_pdf_fg_sel_gl_21_RBUS   siw_pdf_pdf_fg_sel_gl_21;
	siw_pdf_pdf_fg_sel_gl_22_RBUS   siw_pdf_pdf_fg_sel_gl_22;
	siw_pdf_pdf_fg_sel_gl_23_RBUS   siw_pdf_pdf_fg_sel_gl_23;

	siw_pdf_pdf_fg_sel_gl_0.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_0_reg);
	siw_pdf_pdf_fg_sel_gl_1.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_1_reg);
	siw_pdf_pdf_fg_sel_gl_2.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_2_reg);
	siw_pdf_pdf_fg_sel_gl_3.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_3_reg);
	siw_pdf_pdf_fg_sel_gl_4.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_4_reg);
	siw_pdf_pdf_fg_sel_gl_5.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_5_reg);
	siw_pdf_pdf_fg_sel_gl_6.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_6_reg);
	siw_pdf_pdf_fg_sel_gl_7.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_7_reg);
	siw_pdf_pdf_fg_sel_gl_8.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_8_reg);
	siw_pdf_pdf_fg_sel_gl_9.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_9_reg);
	siw_pdf_pdf_fg_sel_gl_10.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_10_reg);
	siw_pdf_pdf_fg_sel_gl_11.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_11_reg);
	siw_pdf_pdf_fg_sel_gl_12.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_12_reg);
	siw_pdf_pdf_fg_sel_gl_13.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_13_reg);
	siw_pdf_pdf_fg_sel_gl_14.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_14_reg);
	siw_pdf_pdf_fg_sel_gl_15.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_15_reg);
	siw_pdf_pdf_fg_sel_gl_16.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_16_reg);
	siw_pdf_pdf_fg_sel_gl_17.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_17_reg);
	siw_pdf_pdf_fg_sel_gl_18.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_18_reg);
	siw_pdf_pdf_fg_sel_gl_19.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_19_reg);
	siw_pdf_pdf_fg_sel_gl_20.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_20_reg);
	siw_pdf_pdf_fg_sel_gl_21.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_21_reg);
	siw_pdf_pdf_fg_sel_gl_22.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_22_reg);
	siw_pdf_pdf_fg_sel_gl_23.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_GL_23_reg);

	siw_pdf_pdf_fg_sel_gl_0.pdf_fg_pt_00g = pfg_gl_inUnit->fg_gl_g[0][0];
	siw_pdf_pdf_fg_sel_gl_0.pdf_fg_pt_00b = pfg_gl_inUnit->fg_gl_b[0][0];
	siw_pdf_pdf_fg_sel_gl_1.pdf_fg_pt_01b = pfg_gl_inUnit->fg_gl_b[0][1];
	siw_pdf_pdf_fg_sel_gl_1.pdf_fg_pt_00r = pfg_gl_inUnit->fg_gl_r[0][0];
	siw_pdf_pdf_fg_sel_gl_2.pdf_fg_pt_01r = pfg_gl_inUnit->fg_gl_r[0][1];
	siw_pdf_pdf_fg_sel_gl_2.pdf_fg_pt_01g = pfg_gl_inUnit->fg_gl_g[0][1];
	siw_pdf_pdf_fg_sel_gl_3.pdf_fg_pt_02g = pfg_gl_inUnit->fg_gl_g[0][2];
	siw_pdf_pdf_fg_sel_gl_3.pdf_fg_pt_02b = pfg_gl_inUnit->fg_gl_b[0][2];
	siw_pdf_pdf_fg_sel_gl_4.pdf_fg_pt_03b = pfg_gl_inUnit->fg_gl_b[0][3];
	siw_pdf_pdf_fg_sel_gl_4.pdf_fg_pt_02r = pfg_gl_inUnit->fg_gl_r[0][2];
	siw_pdf_pdf_fg_sel_gl_5.pdf_fg_pt_03r = pfg_gl_inUnit->fg_gl_r[0][3];
	siw_pdf_pdf_fg_sel_gl_5.pdf_fg_pt_03g = pfg_gl_inUnit->fg_gl_g[0][3];
	siw_pdf_pdf_fg_sel_gl_6.pdf_fg_pt_10g = pfg_gl_inUnit->fg_gl_g[1][0];
	siw_pdf_pdf_fg_sel_gl_6.pdf_fg_pt_10b = pfg_gl_inUnit->fg_gl_b[1][0];
	siw_pdf_pdf_fg_sel_gl_7.pdf_fg_pt_11b = pfg_gl_inUnit->fg_gl_b[1][1];
	siw_pdf_pdf_fg_sel_gl_7.pdf_fg_pt_10r = pfg_gl_inUnit->fg_gl_r[1][0];
	siw_pdf_pdf_fg_sel_gl_8.pdf_fg_pt_11r = pfg_gl_inUnit->fg_gl_r[1][1];
	siw_pdf_pdf_fg_sel_gl_8.pdf_fg_pt_11g = pfg_gl_inUnit->fg_gl_g[1][1];
	siw_pdf_pdf_fg_sel_gl_9.pdf_fg_pt_12g = pfg_gl_inUnit->fg_gl_g[1][2];
	siw_pdf_pdf_fg_sel_gl_9.pdf_fg_pt_12b = pfg_gl_inUnit->fg_gl_b[1][2];
	siw_pdf_pdf_fg_sel_gl_10.pdf_fg_pt_13b = pfg_gl_inUnit->fg_gl_b[1][3];
	siw_pdf_pdf_fg_sel_gl_10.pdf_fg_pt_12r = pfg_gl_inUnit->fg_gl_r[1][2];
	siw_pdf_pdf_fg_sel_gl_11.pdf_fg_pt_13r = pfg_gl_inUnit->fg_gl_r[1][3];
	siw_pdf_pdf_fg_sel_gl_11.pdf_fg_pt_13g = pfg_gl_inUnit->fg_gl_g[1][3];
	siw_pdf_pdf_fg_sel_gl_12.pdf_fg_pt_20g = pfg_gl_inUnit->fg_gl_g[2][0];
	siw_pdf_pdf_fg_sel_gl_12.pdf_fg_pt_20b = pfg_gl_inUnit->fg_gl_b[2][0];
	siw_pdf_pdf_fg_sel_gl_13.pdf_fg_pt_21b = pfg_gl_inUnit->fg_gl_b[2][1];
	siw_pdf_pdf_fg_sel_gl_13.pdf_fg_pt_20r = pfg_gl_inUnit->fg_gl_r[2][0];
	siw_pdf_pdf_fg_sel_gl_14.pdf_fg_pt_21r = pfg_gl_inUnit->fg_gl_r[2][1];
	siw_pdf_pdf_fg_sel_gl_14.pdf_fg_pt_21g = pfg_gl_inUnit->fg_gl_g[2][1];
	siw_pdf_pdf_fg_sel_gl_15.pdf_fg_pt_22g = pfg_gl_inUnit->fg_gl_g[2][2];
	siw_pdf_pdf_fg_sel_gl_15.pdf_fg_pt_22b = pfg_gl_inUnit->fg_gl_b[2][2];
	siw_pdf_pdf_fg_sel_gl_16.pdf_fg_pt_23b = pfg_gl_inUnit->fg_gl_b[2][3];
	siw_pdf_pdf_fg_sel_gl_16.pdf_fg_pt_22r = pfg_gl_inUnit->fg_gl_r[2][2];
	siw_pdf_pdf_fg_sel_gl_17.pdf_fg_pt_23r = pfg_gl_inUnit->fg_gl_r[2][3];
	siw_pdf_pdf_fg_sel_gl_17.pdf_fg_pt_23g = pfg_gl_inUnit->fg_gl_g[2][3];
	siw_pdf_pdf_fg_sel_gl_18.pdf_fg_pt_30g = pfg_gl_inUnit->fg_gl_g[3][0];
	siw_pdf_pdf_fg_sel_gl_18.pdf_fg_pt_30b = pfg_gl_inUnit->fg_gl_b[3][0];
	siw_pdf_pdf_fg_sel_gl_19.pdf_fg_pt_31b = pfg_gl_inUnit->fg_gl_b[3][1];
	siw_pdf_pdf_fg_sel_gl_19.pdf_fg_pt_30r = pfg_gl_inUnit->fg_gl_r[3][0];
	siw_pdf_pdf_fg_sel_gl_20.pdf_fg_pt_31r = pfg_gl_inUnit->fg_gl_r[3][1];
	siw_pdf_pdf_fg_sel_gl_20.pdf_fg_pt_31g = pfg_gl_inUnit->fg_gl_g[3][1];
	siw_pdf_pdf_fg_sel_gl_21.pdf_fg_pt_32g = pfg_gl_inUnit->fg_gl_g[3][2];
	siw_pdf_pdf_fg_sel_gl_21.pdf_fg_pt_32b = pfg_gl_inUnit->fg_gl_b[3][2];
	siw_pdf_pdf_fg_sel_gl_22.pdf_fg_pt_33b = pfg_gl_inUnit->fg_gl_b[3][3];
	siw_pdf_pdf_fg_sel_gl_22.pdf_fg_pt_32r = pfg_gl_inUnit->fg_gl_r[3][2];
	siw_pdf_pdf_fg_sel_gl_23.pdf_fg_pt_33r = pfg_gl_inUnit->fg_gl_r[3][3];
	siw_pdf_pdf_fg_sel_gl_23.pdf_fg_pt_33g = pfg_gl_inUnit->fg_gl_g[3][3];

	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_0_reg, siw_pdf_pdf_fg_sel_gl_0.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_1_reg, siw_pdf_pdf_fg_sel_gl_1.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_2_reg, siw_pdf_pdf_fg_sel_gl_2.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_3_reg, siw_pdf_pdf_fg_sel_gl_3.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_4_reg, siw_pdf_pdf_fg_sel_gl_4.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_5_reg, siw_pdf_pdf_fg_sel_gl_5.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_6_reg, siw_pdf_pdf_fg_sel_gl_6.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_7_reg, siw_pdf_pdf_fg_sel_gl_7.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_8_reg, siw_pdf_pdf_fg_sel_gl_8.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_9_reg, siw_pdf_pdf_fg_sel_gl_9.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_10_reg, siw_pdf_pdf_fg_sel_gl_10.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_11_reg, siw_pdf_pdf_fg_sel_gl_11.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_12_reg, siw_pdf_pdf_fg_sel_gl_12.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_13_reg, siw_pdf_pdf_fg_sel_gl_13.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_14_reg, siw_pdf_pdf_fg_sel_gl_14.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_15_reg, siw_pdf_pdf_fg_sel_gl_15.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_16_reg, siw_pdf_pdf_fg_sel_gl_16.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_17_reg, siw_pdf_pdf_fg_sel_gl_17.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_18_reg, siw_pdf_pdf_fg_sel_gl_18.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_19_reg, siw_pdf_pdf_fg_sel_gl_19.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_20_reg, siw_pdf_pdf_fg_sel_gl_20.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_21_reg, siw_pdf_pdf_fg_sel_gl_21.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_22_reg, siw_pdf_pdf_fg_sel_gl_22.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_GL_23_reg, siw_pdf_pdf_fg_sel_gl_23.regValue);
}

void drvif_color_set_pdf_fg_sel_opr(DRV_PDF_fg_sel_opr_t *pfg_sel_opr)
{
	siw_pdf_pdf_fg_sel_opr_0_RBUS siw_pdf_pdf_fg_sel_opr_0;	
	siw_pdf_pdf_fg_sel_opr_1_RBUS siw_pdf_pdf_fg_sel_opr_1;	
	siw_pdf_pdf_fg_sel_opr_2_RBUS siw_pdf_pdf_fg_sel_opr_2;	
	siw_pdf_pdf_fg_sel_opr_3_RBUS siw_pdf_pdf_fg_sel_opr_3;	
	siw_pdf_pdf_fg_sel_opr_4_RBUS siw_pdf_pdf_fg_sel_opr_4;	
	siw_pdf_pdf_fg_sel_opr_5_RBUS siw_pdf_pdf_fg_sel_opr_5;	
	siw_pdf_pdf_fg_sel_opr_6_RBUS siw_pdf_pdf_fg_sel_opr_6;	
	siw_pdf_pdf_fg_sel_opr_7_RBUS siw_pdf_pdf_fg_sel_opr_7;	
	siw_pdf_pdf_fg_sel_opr_8_RBUS siw_pdf_pdf_fg_sel_opr_8;	
	siw_pdf_pdf_fg_sel_opr_9_RBUS siw_pdf_pdf_fg_sel_opr_9;	
	siw_pdf_pdf_fg_sel_opr_10_RBUS siw_pdf_pdf_fg_sel_opr_10;	
	siw_pdf_pdf_fg_sel_opr_11_RBUS siw_pdf_pdf_fg_sel_opr_11;	
	siw_pdf_pdf_fg_sel_opr_12_RBUS siw_pdf_pdf_fg_sel_opr_12;	
	siw_pdf_pdf_fg_sel_opr_13_RBUS siw_pdf_pdf_fg_sel_opr_13;	
	siw_pdf_pdf_fg_sel_opr_14_RBUS siw_pdf_pdf_fg_sel_opr_14;	
	siw_pdf_pdf_fg_sel_opr_15_RBUS siw_pdf_pdf_fg_sel_opr_15;	

	siw_pdf_pdf_fg_sel_opr_0.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_0_reg);
	siw_pdf_pdf_fg_sel_opr_1.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_1_reg);
	siw_pdf_pdf_fg_sel_opr_2.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_2_reg);
	siw_pdf_pdf_fg_sel_opr_3.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_3_reg);
	siw_pdf_pdf_fg_sel_opr_4.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_4_reg);
	siw_pdf_pdf_fg_sel_opr_5.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_5_reg);
	siw_pdf_pdf_fg_sel_opr_6.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_6_reg);
	siw_pdf_pdf_fg_sel_opr_7.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_7_reg);
	siw_pdf_pdf_fg_sel_opr_8.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_8_reg);
	siw_pdf_pdf_fg_sel_opr_9.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_9_reg);
	siw_pdf_pdf_fg_sel_opr_10.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_10_reg);
	siw_pdf_pdf_fg_sel_opr_11.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_11_reg);
	siw_pdf_pdf_fg_sel_opr_12.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_12_reg);
	siw_pdf_pdf_fg_sel_opr_13.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_13_reg);
	siw_pdf_pdf_fg_sel_opr_14.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_14_reg);
	siw_pdf_pdf_fg_sel_opr_15.regValue = IoReg_Read32(SIW_PDF_PDF_FG_SEL_OPR_15_reg);

	siw_pdf_pdf_fg_sel_opr_0.pdf_fg_gl_opr_00r = pfg_sel_opr->fg_opr_r[0][0];
	siw_pdf_pdf_fg_sel_opr_0.pdf_fg_gl_opr_00g = pfg_sel_opr->fg_opr_g[0][0];
	siw_pdf_pdf_fg_sel_opr_0.pdf_fg_gl_opr_00b = pfg_sel_opr->fg_opr_b[0][0];
	siw_pdf_pdf_fg_sel_opr_1.pdf_fg_gl_opr_01r = pfg_sel_opr->fg_opr_r[0][1];
	siw_pdf_pdf_fg_sel_opr_1.pdf_fg_gl_opr_01g = pfg_sel_opr->fg_opr_g[0][1];
	siw_pdf_pdf_fg_sel_opr_1.pdf_fg_gl_opr_01b = pfg_sel_opr->fg_opr_b[0][1];
	siw_pdf_pdf_fg_sel_opr_2.pdf_fg_gl_opr_02r = pfg_sel_opr->fg_opr_r[0][2];
	siw_pdf_pdf_fg_sel_opr_2.pdf_fg_gl_opr_02g = pfg_sel_opr->fg_opr_g[0][2];
	siw_pdf_pdf_fg_sel_opr_2.pdf_fg_gl_opr_02b = pfg_sel_opr->fg_opr_b[0][2];
	siw_pdf_pdf_fg_sel_opr_3.pdf_fg_gl_opr_03r = pfg_sel_opr->fg_opr_r[0][3];
	siw_pdf_pdf_fg_sel_opr_3.pdf_fg_gl_opr_03g = pfg_sel_opr->fg_opr_g[0][3];
	siw_pdf_pdf_fg_sel_opr_3.pdf_fg_gl_opr_03b = pfg_sel_opr->fg_opr_b[0][3];
	siw_pdf_pdf_fg_sel_opr_4.pdf_fg_gl_opr_10r = pfg_sel_opr->fg_opr_r[1][0];
	siw_pdf_pdf_fg_sel_opr_4.pdf_fg_gl_opr_10g = pfg_sel_opr->fg_opr_g[1][0];
	siw_pdf_pdf_fg_sel_opr_4.pdf_fg_gl_opr_10b = pfg_sel_opr->fg_opr_b[1][0];
	siw_pdf_pdf_fg_sel_opr_5.pdf_fg_gl_opr_11r = pfg_sel_opr->fg_opr_r[1][1];
	siw_pdf_pdf_fg_sel_opr_5.pdf_fg_gl_opr_11g = pfg_sel_opr->fg_opr_g[1][1];
	siw_pdf_pdf_fg_sel_opr_5.pdf_fg_gl_opr_11b = pfg_sel_opr->fg_opr_b[1][1];
	siw_pdf_pdf_fg_sel_opr_6.pdf_fg_gl_opr_12r = pfg_sel_opr->fg_opr_r[1][2];
	siw_pdf_pdf_fg_sel_opr_6.pdf_fg_gl_opr_12g = pfg_sel_opr->fg_opr_g[1][2];
	siw_pdf_pdf_fg_sel_opr_6.pdf_fg_gl_opr_12b = pfg_sel_opr->fg_opr_b[1][2];
	siw_pdf_pdf_fg_sel_opr_7.pdf_fg_gl_opr_13r = pfg_sel_opr->fg_opr_r[1][3];
	siw_pdf_pdf_fg_sel_opr_7.pdf_fg_gl_opr_13g = pfg_sel_opr->fg_opr_g[1][3];
	siw_pdf_pdf_fg_sel_opr_7.pdf_fg_gl_opr_13b = pfg_sel_opr->fg_opr_b[1][3];
	siw_pdf_pdf_fg_sel_opr_8.pdf_fg_gl_opr_20r = pfg_sel_opr->fg_opr_r[2][0];
	siw_pdf_pdf_fg_sel_opr_8.pdf_fg_gl_opr_20g = pfg_sel_opr->fg_opr_g[2][0];
	siw_pdf_pdf_fg_sel_opr_8.pdf_fg_gl_opr_20b = pfg_sel_opr->fg_opr_b[2][0];
	siw_pdf_pdf_fg_sel_opr_9.pdf_fg_gl_opr_21r = pfg_sel_opr->fg_opr_r[2][1];
	siw_pdf_pdf_fg_sel_opr_9.pdf_fg_gl_opr_21g = pfg_sel_opr->fg_opr_g[2][1];
	siw_pdf_pdf_fg_sel_opr_9.pdf_fg_gl_opr_21b = pfg_sel_opr->fg_opr_b[2][1];
	siw_pdf_pdf_fg_sel_opr_10.pdf_fg_gl_opr_22r = pfg_sel_opr->fg_opr_r[2][2];
	siw_pdf_pdf_fg_sel_opr_10.pdf_fg_gl_opr_22g = pfg_sel_opr->fg_opr_g[2][2];
	siw_pdf_pdf_fg_sel_opr_10.pdf_fg_gl_opr_22b = pfg_sel_opr->fg_opr_b[2][2];
	siw_pdf_pdf_fg_sel_opr_11.pdf_fg_gl_opr_23r = pfg_sel_opr->fg_opr_r[2][3];
	siw_pdf_pdf_fg_sel_opr_11.pdf_fg_gl_opr_23g = pfg_sel_opr->fg_opr_g[2][3];
	siw_pdf_pdf_fg_sel_opr_11.pdf_fg_gl_opr_23b = pfg_sel_opr->fg_opr_b[2][3];
	siw_pdf_pdf_fg_sel_opr_12.pdf_fg_gl_opr_30r = pfg_sel_opr->fg_opr_r[3][0];
	siw_pdf_pdf_fg_sel_opr_12.pdf_fg_gl_opr_30g = pfg_sel_opr->fg_opr_g[3][0];
	siw_pdf_pdf_fg_sel_opr_12.pdf_fg_gl_opr_30b = pfg_sel_opr->fg_opr_b[3][0];
	siw_pdf_pdf_fg_sel_opr_13.pdf_fg_gl_opr_31r = pfg_sel_opr->fg_opr_r[3][1];
	siw_pdf_pdf_fg_sel_opr_13.pdf_fg_gl_opr_31g = pfg_sel_opr->fg_opr_g[3][1];
	siw_pdf_pdf_fg_sel_opr_13.pdf_fg_gl_opr_31b = pfg_sel_opr->fg_opr_b[3][1];
	siw_pdf_pdf_fg_sel_opr_14.pdf_fg_gl_opr_32r = pfg_sel_opr->fg_opr_r[3][2];
	siw_pdf_pdf_fg_sel_opr_14.pdf_fg_gl_opr_32g = pfg_sel_opr->fg_opr_g[3][2];
	siw_pdf_pdf_fg_sel_opr_14.pdf_fg_gl_opr_32b = pfg_sel_opr->fg_opr_b[3][2];
	siw_pdf_pdf_fg_sel_opr_15.pdf_fg_gl_opr_33r = pfg_sel_opr->fg_opr_r[3][3];
	siw_pdf_pdf_fg_sel_opr_15.pdf_fg_gl_opr_33g = pfg_sel_opr->fg_opr_g[3][3];
	siw_pdf_pdf_fg_sel_opr_15.pdf_fg_gl_opr_33b = pfg_sel_opr->fg_opr_b[3][3];

	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_0_reg, siw_pdf_pdf_fg_sel_opr_0.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_1_reg, siw_pdf_pdf_fg_sel_opr_1.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_2_reg, siw_pdf_pdf_fg_sel_opr_2.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_3_reg, siw_pdf_pdf_fg_sel_opr_3.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_4_reg, siw_pdf_pdf_fg_sel_opr_4.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_5_reg, siw_pdf_pdf_fg_sel_opr_5.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_6_reg, siw_pdf_pdf_fg_sel_opr_6.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_7_reg, siw_pdf_pdf_fg_sel_opr_7.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_8_reg, siw_pdf_pdf_fg_sel_opr_8.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_9_reg, siw_pdf_pdf_fg_sel_opr_9.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_10_reg, siw_pdf_pdf_fg_sel_opr_10.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_11_reg, siw_pdf_pdf_fg_sel_opr_11.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_12_reg, siw_pdf_pdf_fg_sel_opr_12.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_13_reg, siw_pdf_pdf_fg_sel_opr_13.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_14_reg, siw_pdf_pdf_fg_sel_opr_14.regValue);
	IoReg_Write32(SIW_PDF_PDF_FG_SEL_OPR_15_reg, siw_pdf_pdf_fg_sel_opr_15.regValue);
	
}


void drvif_color_set_pdf_bg_gl(DRV_PDF_bg_t *bg)
{
	siw_pdf_pdf_bg_gl_pt0_RBUS siw_pdf_pdf_bg_gl_pt0;
	siw_pdf_pdf_bg_gl_pt1_RBUS siw_pdf_pdf_bg_gl_pt1;
	siw_pdf_pdf_bg_gl_pt2_RBUS siw_pdf_pdf_bg_gl_pt2;
	siw_pdf_pdf_bg_gl_pt3_RBUS siw_pdf_pdf_bg_gl_pt3;
	siw_pdf_pdf_bg_gl_pt4_RBUS siw_pdf_pdf_bg_gl_pt4;
	siw_pdf_pdf_bg_gl_pt5_RBUS siw_pdf_pdf_bg_gl_pt5;
	siw_pdf_pdf_bg_gl_pt6_RBUS siw_pdf_pdf_bg_gl_pt6;
	siw_pdf_pdf_bg_gl_pt7_RBUS siw_pdf_pdf_bg_gl_pt7;

	siw_pdf_pdf_bg_gl_pt0.regValue = IoReg_Read32(SIW_PDF_PDF_BG_GL_PT0_reg);
	siw_pdf_pdf_bg_gl_pt1.regValue = IoReg_Read32(SIW_PDF_PDF_BG_GL_PT1_reg);
	siw_pdf_pdf_bg_gl_pt2.regValue = IoReg_Read32(SIW_PDF_PDF_BG_GL_PT2_reg);
	siw_pdf_pdf_bg_gl_pt3.regValue = IoReg_Read32(SIW_PDF_PDF_BG_GL_PT3_reg);
	siw_pdf_pdf_bg_gl_pt4.regValue = IoReg_Read32(SIW_PDF_PDF_BG_GL_PT4_reg);
	siw_pdf_pdf_bg_gl_pt5.regValue = IoReg_Read32(SIW_PDF_PDF_BG_GL_PT5_reg);
	siw_pdf_pdf_bg_gl_pt6.regValue = IoReg_Read32(SIW_PDF_PDF_BG_GL_PT6_reg);
	siw_pdf_pdf_bg_gl_pt7.regValue = IoReg_Read32(SIW_PDF_PDF_BG_GL_PT7_reg);

	siw_pdf_pdf_bg_gl_pt0.pdf_bg_gl_0_pt0 = bg->bg_gl[0].xtalk_bg_gl_0;
	siw_pdf_pdf_bg_gl_pt0.pdf_bg_gl_1_pt0 = bg->bg_gl[0].xtalk_bg_gl_1;
	siw_pdf_pdf_bg_gl_pt0.pdf_bg_gl_2_pt0 = bg->bg_gl[0].xtalk_bg_gl_2;
	siw_pdf_pdf_bg_gl_pt1.pdf_bg_gl_0_pt1 = bg->bg_gl[1].xtalk_bg_gl_0;
	siw_pdf_pdf_bg_gl_pt1.pdf_bg_gl_1_pt1 = bg->bg_gl[1].xtalk_bg_gl_1;
	siw_pdf_pdf_bg_gl_pt1.pdf_bg_gl_2_pt1 = bg->bg_gl[1].xtalk_bg_gl_2;
	siw_pdf_pdf_bg_gl_pt2.pdf_bg_gl_0_pt2 = bg->bg_gl[2].xtalk_bg_gl_0;
	siw_pdf_pdf_bg_gl_pt2.pdf_bg_gl_1_pt2 = bg->bg_gl[2].xtalk_bg_gl_1;
	siw_pdf_pdf_bg_gl_pt2.pdf_bg_gl_2_pt2 = bg->bg_gl[2].xtalk_bg_gl_2;
	siw_pdf_pdf_bg_gl_pt3.pdf_bg_gl_0_pt3 = bg->bg_gl[3].xtalk_bg_gl_0;
	siw_pdf_pdf_bg_gl_pt3.pdf_bg_gl_1_pt3 = bg->bg_gl[3].xtalk_bg_gl_1;
	siw_pdf_pdf_bg_gl_pt3.pdf_bg_gl_2_pt3 = bg->bg_gl[3].xtalk_bg_gl_2;
	siw_pdf_pdf_bg_gl_pt4.pdf_bg_gl_0_pt4 = bg->bg_gl[4].xtalk_bg_gl_0;
	siw_pdf_pdf_bg_gl_pt4.pdf_bg_gl_1_pt4 = bg->bg_gl[4].xtalk_bg_gl_1;
	siw_pdf_pdf_bg_gl_pt4.pdf_bg_gl_2_pt4 = bg->bg_gl[4].xtalk_bg_gl_2;
	siw_pdf_pdf_bg_gl_pt5.pdf_bg_gl_0_pt5 = bg->bg_gl[5].xtalk_bg_gl_0;
	siw_pdf_pdf_bg_gl_pt5.pdf_bg_gl_1_pt5 = bg->bg_gl[5].xtalk_bg_gl_1;
	siw_pdf_pdf_bg_gl_pt5.pdf_bg_gl_2_pt5 = bg->bg_gl[5].xtalk_bg_gl_2;
	siw_pdf_pdf_bg_gl_pt6.pdf_bg_gl_0_pt6 = bg->bg_gl[6].xtalk_bg_gl_0;
	siw_pdf_pdf_bg_gl_pt6.pdf_bg_gl_1_pt6 = bg->bg_gl[6].xtalk_bg_gl_1;
	siw_pdf_pdf_bg_gl_pt6.pdf_bg_gl_2_pt6 = bg->bg_gl[6].xtalk_bg_gl_2;
	siw_pdf_pdf_bg_gl_pt7.pdf_bg_gl_0_pt7 = bg->bg_gl[7].xtalk_bg_gl_0;
	siw_pdf_pdf_bg_gl_pt7.pdf_bg_gl_1_pt7 = bg->bg_gl[7].xtalk_bg_gl_1;
	siw_pdf_pdf_bg_gl_pt7.pdf_bg_gl_2_pt7 = bg->bg_gl[7].xtalk_bg_gl_2;

	IoReg_Write32(SIW_PDF_PDF_BG_GL_PT0_reg, siw_pdf_pdf_bg_gl_pt0.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_GL_PT1_reg, siw_pdf_pdf_bg_gl_pt1.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_GL_PT2_reg, siw_pdf_pdf_bg_gl_pt2.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_GL_PT3_reg, siw_pdf_pdf_bg_gl_pt3.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_GL_PT4_reg, siw_pdf_pdf_bg_gl_pt4.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_GL_PT5_reg, siw_pdf_pdf_bg_gl_pt5.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_GL_PT6_reg, siw_pdf_pdf_bg_gl_pt6.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_GL_PT7_reg, siw_pdf_pdf_bg_gl_pt7.regValue);

}


void drvif_color_set_pdf_bg_temporal_det(DRV_PDF_bg_temporal_det_t *pbg_temporal_det)
{
	siw_pdf_pdf_bg_thl_0_RBUS   siw_pdf_pdf_bg_thl_0;
	siw_pdf_pdf_bg_thl_1_RBUS   siw_pdf_pdf_bg_thl_1;
	siw_pdf_pdf_bg_thl_2_RBUS   siw_pdf_pdf_bg_thl_2;
	siw_pdf_pdf_bg_thl_3_RBUS   siw_pdf_pdf_bg_thl_3;
	siw_pdf_pdf_bg_thl_4_RBUS   siw_pdf_pdf_bg_thl_4;
	siw_pdf_pdf_bg_thl_5_RBUS   siw_pdf_pdf_bg_thl_5;
	siw_pdf_pdf_bg_thl_6_RBUS   siw_pdf_pdf_bg_thl_6;
	siw_pdf_pdf_bg_thl_7_RBUS   siw_pdf_pdf_bg_thl_7;

	siw_pdf_pdf_bg_thl_0.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_0_reg);
	siw_pdf_pdf_bg_thl_1.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_1_reg);
	siw_pdf_pdf_bg_thl_2.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_2_reg);
	siw_pdf_pdf_bg_thl_3.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_3_reg);
	siw_pdf_pdf_bg_thl_4.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_4_reg);
	siw_pdf_pdf_bg_thl_5.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_5_reg);
	siw_pdf_pdf_bg_thl_6.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_6_reg);
	siw_pdf_pdf_bg_thl_7.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_7_reg);

	siw_pdf_pdf_bg_thl_0.pdf_bg_frame_det_threshold_0 = pbg_temporal_det->xtalk_bg_det_threshold_0;
	siw_pdf_pdf_bg_thl_1.pdf_bg_frame_det_threshold_1 = pbg_temporal_det->xtalk_bg_det_threshold_1;
	siw_pdf_pdf_bg_thl_2.pdf_bg_frame_det_threshold_2 = pbg_temporal_det->xtalk_bg_det_threshold_2;
	siw_pdf_pdf_bg_thl_3.pdf_bg_frame_det_threshold_3 = pbg_temporal_det->xtalk_bg_det_threshold_3;
	siw_pdf_pdf_bg_thl_4.pdf_bg_frame_det_threshold_4 = pbg_temporal_det->xtalk_bg_det_threshold_4;
	siw_pdf_pdf_bg_thl_5.pdf_bg_frame_det_threshold_5 = pbg_temporal_det->xtalk_bg_det_threshold_5;
	siw_pdf_pdf_bg_thl_6.pdf_bg_frame_det_threshold_6 = pbg_temporal_det->xtalk_bg_det_threshold_6;
	siw_pdf_pdf_bg_thl_7.pdf_bg_frame_det_threshold_7 = pbg_temporal_det->xtalk_bg_det_threshold_7;

	IoReg_Write32(SIW_PDF_PDF_BG_THL_0_reg, siw_pdf_pdf_bg_thl_0.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_1_reg, siw_pdf_pdf_bg_thl_1.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_2_reg, siw_pdf_pdf_bg_thl_2.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_3_reg, siw_pdf_pdf_bg_thl_3.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_4_reg, siw_pdf_pdf_bg_thl_4.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_5_reg, siw_pdf_pdf_bg_thl_5.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_6_reg, siw_pdf_pdf_bg_thl_6.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_7_reg, siw_pdf_pdf_bg_thl_7.regValue);
}


void drvif_color_set_pdf_bg_temporal_undet(DRV_PDF_bg_temporal_undet_t *pbg_temporal_undet)
{
	siw_pdf_pdf_bg_thl_8_RBUS   siw_pdf_pdf_bg_thl_8;
	siw_pdf_pdf_bg_thl_9_RBUS   siw_pdf_pdf_bg_thl_9;

	siw_pdf_pdf_bg_thl_8.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_8_reg);
	siw_pdf_pdf_bg_thl_9.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_9_reg);

	siw_pdf_pdf_bg_thl_8.pdf_bg_frame_undet_threshold_0 = pbg_temporal_undet->xtalk_bg_undet_threshold_0;
	siw_pdf_pdf_bg_thl_8.pdf_bg_frame_undet_threshold_1 = pbg_temporal_undet->xtalk_bg_undet_threshold_1;
	siw_pdf_pdf_bg_thl_8.pdf_bg_frame_undet_threshold_2 = pbg_temporal_undet->xtalk_bg_undet_threshold_2;
	siw_pdf_pdf_bg_thl_8.pdf_bg_frame_undet_threshold_3 = pbg_temporal_undet->xtalk_bg_undet_threshold_3;
	siw_pdf_pdf_bg_thl_9.pdf_bg_frame_undet_threshold_4 = pbg_temporal_undet->xtalk_bg_undet_threshold_4;
	siw_pdf_pdf_bg_thl_9.pdf_bg_frame_undet_threshold_5 = pbg_temporal_undet->xtalk_bg_undet_threshold_5;
	siw_pdf_pdf_bg_thl_9.pdf_bg_frame_undet_threshold_6 = pbg_temporal_undet->xtalk_bg_undet_threshold_6;
	siw_pdf_pdf_bg_thl_9.pdf_bg_frame_undet_threshold_7 = pbg_temporal_undet->xtalk_bg_undet_threshold_7;

	IoReg_Write32(SIW_PDF_PDF_BG_THL_8_reg, siw_pdf_pdf_bg_thl_8.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_9_reg, siw_pdf_pdf_bg_thl_9.regValue);
}

void drvif_color_set_pdf_bg_area_hit(DRV_PDF_bg_area_hit_t *pbg_area_hit)
{
	siw_pdf_pdf_bg_thl_0_RBUS   siw_pdf_pdf_bg_thl_0;
	siw_pdf_pdf_bg_thl_1_RBUS   siw_pdf_pdf_bg_thl_1;
	siw_pdf_pdf_bg_thl_2_RBUS   siw_pdf_pdf_bg_thl_2;
	siw_pdf_pdf_bg_thl_3_RBUS   siw_pdf_pdf_bg_thl_3;
	siw_pdf_pdf_bg_thl_4_RBUS   siw_pdf_pdf_bg_thl_4;
	siw_pdf_pdf_bg_thl_5_RBUS   siw_pdf_pdf_bg_thl_5;
	siw_pdf_pdf_bg_thl_6_RBUS   siw_pdf_pdf_bg_thl_6;
	siw_pdf_pdf_bg_thl_7_RBUS   siw_pdf_pdf_bg_thl_7;

	siw_pdf_pdf_bg_thl_0.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_0_reg);
	siw_pdf_pdf_bg_thl_1.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_1_reg);
	siw_pdf_pdf_bg_thl_2.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_2_reg);
	siw_pdf_pdf_bg_thl_3.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_3_reg);
	siw_pdf_pdf_bg_thl_4.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_4_reg);
	siw_pdf_pdf_bg_thl_5.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_5_reg);
	siw_pdf_pdf_bg_thl_6.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_6_reg);
	siw_pdf_pdf_bg_thl_7.regValue = IoReg_Read32(SIW_PDF_PDF_BG_THL_7_reg);

	siw_pdf_pdf_bg_thl_0.pdf_bg_hit_threshold_0 = pbg_area_hit->xtalk_bg_hit_threshold_0;
	siw_pdf_pdf_bg_thl_1.pdf_bg_hit_threshold_1 = pbg_area_hit->xtalk_bg_hit_threshold_1;
	siw_pdf_pdf_bg_thl_2.pdf_bg_hit_threshold_2 = pbg_area_hit->xtalk_bg_hit_threshold_2;
	siw_pdf_pdf_bg_thl_3.pdf_bg_hit_threshold_3 = pbg_area_hit->xtalk_bg_hit_threshold_3;
	siw_pdf_pdf_bg_thl_4.pdf_bg_hit_threshold_4 = pbg_area_hit->xtalk_bg_hit_threshold_4;
	siw_pdf_pdf_bg_thl_5.pdf_bg_hit_threshold_5 = pbg_area_hit->xtalk_bg_hit_threshold_5;
	siw_pdf_pdf_bg_thl_6.pdf_bg_hit_threshold_6 = pbg_area_hit->xtalk_bg_hit_threshold_6;
	siw_pdf_pdf_bg_thl_7.pdf_bg_hit_threshold_7 = pbg_area_hit->xtalk_bg_hit_threshold_7;

	IoReg_Write32(SIW_PDF_PDF_BG_THL_0_reg, siw_pdf_pdf_bg_thl_0.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_1_reg, siw_pdf_pdf_bg_thl_1.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_2_reg, siw_pdf_pdf_bg_thl_2.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_3_reg, siw_pdf_pdf_bg_thl_3.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_4_reg, siw_pdf_pdf_bg_thl_4.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_5_reg, siw_pdf_pdf_bg_thl_5.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_6_reg, siw_pdf_pdf_bg_thl_6.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_THL_7_reg, siw_pdf_pdf_bg_thl_7.regValue);
}

void drvif_color_set_pdf_bg_gl_inUnit(DRV_PDF_bg_gl_inUnit_t *pbg_gl_inUnit)
{
	siw_pdf_pdf_bg_sel_gl_0_RBUS    siw_pdf_pdf_bg_sel_gl_0;
	siw_pdf_pdf_bg_sel_gl_1_RBUS    siw_pdf_pdf_bg_sel_gl_1;
	siw_pdf_pdf_bg_sel_gl_2_RBUS    siw_pdf_pdf_bg_sel_gl_2;
	siw_pdf_pdf_bg_sel_gl_3_RBUS    siw_pdf_pdf_bg_sel_gl_3;
	siw_pdf_pdf_bg_sel_gl_4_RBUS    siw_pdf_pdf_bg_sel_gl_4;
	siw_pdf_pdf_bg_sel_gl_5_RBUS    siw_pdf_pdf_bg_sel_gl_5;
	siw_pdf_pdf_bg_sel_gl_6_RBUS    siw_pdf_pdf_bg_sel_gl_6;
	siw_pdf_pdf_bg_sel_gl_7_RBUS    siw_pdf_pdf_bg_sel_gl_7;
	siw_pdf_pdf_bg_sel_gl_8_RBUS    siw_pdf_pdf_bg_sel_gl_8;
	siw_pdf_pdf_bg_sel_gl_9_RBUS    siw_pdf_pdf_bg_sel_gl_9;
	siw_pdf_pdf_bg_sel_gl_10_RBUS   siw_pdf_pdf_bg_sel_gl_10;
	siw_pdf_pdf_bg_sel_gl_11_RBUS   siw_pdf_pdf_bg_sel_gl_11;
	siw_pdf_pdf_bg_sel_gl_12_RBUS   siw_pdf_pdf_bg_sel_gl_12;
	siw_pdf_pdf_bg_sel_gl_13_RBUS   siw_pdf_pdf_bg_sel_gl_13;
	siw_pdf_pdf_bg_sel_gl_14_RBUS   siw_pdf_pdf_bg_sel_gl_14;
	siw_pdf_pdf_bg_sel_gl_15_RBUS   siw_pdf_pdf_bg_sel_gl_15;
	siw_pdf_pdf_bg_sel_gl_16_RBUS   siw_pdf_pdf_bg_sel_gl_16;
	siw_pdf_pdf_bg_sel_gl_17_RBUS   siw_pdf_pdf_bg_sel_gl_17;
	siw_pdf_pdf_bg_sel_gl_18_RBUS   siw_pdf_pdf_bg_sel_gl_18;
	siw_pdf_pdf_bg_sel_gl_19_RBUS   siw_pdf_pdf_bg_sel_gl_19;
	siw_pdf_pdf_bg_sel_gl_20_RBUS   siw_pdf_pdf_bg_sel_gl_20;
	siw_pdf_pdf_bg_sel_gl_21_RBUS   siw_pdf_pdf_bg_sel_gl_21;
	siw_pdf_pdf_bg_sel_gl_22_RBUS   siw_pdf_pdf_bg_sel_gl_22;
	siw_pdf_pdf_bg_sel_gl_23_RBUS   siw_pdf_pdf_bg_sel_gl_23;

	siw_pdf_pdf_bg_sel_gl_0.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_0_reg);
	siw_pdf_pdf_bg_sel_gl_1.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_1_reg);
	siw_pdf_pdf_bg_sel_gl_2.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_2_reg);
	siw_pdf_pdf_bg_sel_gl_3.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_3_reg);
	siw_pdf_pdf_bg_sel_gl_4.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_4_reg);
	siw_pdf_pdf_bg_sel_gl_5.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_5_reg);
	siw_pdf_pdf_bg_sel_gl_6.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_6_reg);
	siw_pdf_pdf_bg_sel_gl_7.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_7_reg);
	siw_pdf_pdf_bg_sel_gl_8.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_8_reg);
	siw_pdf_pdf_bg_sel_gl_9.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_9_reg);
	siw_pdf_pdf_bg_sel_gl_10.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_10_reg);
	siw_pdf_pdf_bg_sel_gl_11.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_11_reg);
	siw_pdf_pdf_bg_sel_gl_12.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_12_reg);
	siw_pdf_pdf_bg_sel_gl_13.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_13_reg);
	siw_pdf_pdf_bg_sel_gl_14.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_14_reg);
	siw_pdf_pdf_bg_sel_gl_15.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_15_reg);
	siw_pdf_pdf_bg_sel_gl_16.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_16_reg);
	siw_pdf_pdf_bg_sel_gl_17.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_17_reg);
	siw_pdf_pdf_bg_sel_gl_18.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_18_reg);
	siw_pdf_pdf_bg_sel_gl_19.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_19_reg);
	siw_pdf_pdf_bg_sel_gl_20.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_20_reg);
	siw_pdf_pdf_bg_sel_gl_21.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_21_reg);
	siw_pdf_pdf_bg_sel_gl_22.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_22_reg);
	siw_pdf_pdf_bg_sel_gl_23.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_GL_23_reg);

	siw_pdf_pdf_bg_sel_gl_0.pdf_bg_pt_00g = pbg_gl_inUnit->bg_gl_g[0][0];
	siw_pdf_pdf_bg_sel_gl_0.pdf_bg_pt_00b = pbg_gl_inUnit->bg_gl_b[0][0];
	siw_pdf_pdf_bg_sel_gl_1.pdf_bg_pt_01b = pbg_gl_inUnit->bg_gl_b[0][1];
	siw_pdf_pdf_bg_sel_gl_1.pdf_bg_pt_00r = pbg_gl_inUnit->bg_gl_r[0][0];
	siw_pdf_pdf_bg_sel_gl_2.pdf_bg_pt_01r = pbg_gl_inUnit->bg_gl_r[0][1];
	siw_pdf_pdf_bg_sel_gl_2.pdf_bg_pt_01g = pbg_gl_inUnit->bg_gl_g[0][1];
	siw_pdf_pdf_bg_sel_gl_3.pdf_bg_pt_02g = pbg_gl_inUnit->bg_gl_g[0][2];
	siw_pdf_pdf_bg_sel_gl_3.pdf_bg_pt_02b = pbg_gl_inUnit->bg_gl_b[0][2];
	siw_pdf_pdf_bg_sel_gl_4.pdf_bg_pt_03b = pbg_gl_inUnit->bg_gl_b[0][3];
	siw_pdf_pdf_bg_sel_gl_4.pdf_bg_pt_02r = pbg_gl_inUnit->bg_gl_r[0][2];
	siw_pdf_pdf_bg_sel_gl_5.pdf_bg_pt_03r = pbg_gl_inUnit->bg_gl_r[0][3];
	siw_pdf_pdf_bg_sel_gl_5.pdf_bg_pt_03g = pbg_gl_inUnit->bg_gl_g[0][3];
	siw_pdf_pdf_bg_sel_gl_6.pdf_bg_pt_10g = pbg_gl_inUnit->bg_gl_g[1][0];
	siw_pdf_pdf_bg_sel_gl_6.pdf_bg_pt_10b = pbg_gl_inUnit->bg_gl_b[1][0];
	siw_pdf_pdf_bg_sel_gl_7.pdf_bg_pt_11b = pbg_gl_inUnit->bg_gl_b[1][1];
	siw_pdf_pdf_bg_sel_gl_7.pdf_bg_pt_10r = pbg_gl_inUnit->bg_gl_r[1][0];
	siw_pdf_pdf_bg_sel_gl_8.pdf_bg_pt_11r = pbg_gl_inUnit->bg_gl_r[1][1];
	siw_pdf_pdf_bg_sel_gl_8.pdf_bg_pt_11g = pbg_gl_inUnit->bg_gl_g[1][1];
	siw_pdf_pdf_bg_sel_gl_9.pdf_bg_pt_12g = pbg_gl_inUnit->bg_gl_g[1][2];
	siw_pdf_pdf_bg_sel_gl_9.pdf_bg_pt_12b = pbg_gl_inUnit->bg_gl_b[1][2];
	siw_pdf_pdf_bg_sel_gl_10.pdf_bg_pt_13b = pbg_gl_inUnit->bg_gl_b[1][3];
	siw_pdf_pdf_bg_sel_gl_10.pdf_bg_pt_12r = pbg_gl_inUnit->bg_gl_r[1][2];
	siw_pdf_pdf_bg_sel_gl_11.pdf_bg_pt_13r = pbg_gl_inUnit->bg_gl_r[1][3];
	siw_pdf_pdf_bg_sel_gl_11.pdf_bg_pt_13g = pbg_gl_inUnit->bg_gl_g[1][3];
	siw_pdf_pdf_bg_sel_gl_12.pdf_bg_pt_20g = pbg_gl_inUnit->bg_gl_g[2][0];
	siw_pdf_pdf_bg_sel_gl_12.pdf_bg_pt_20b = pbg_gl_inUnit->bg_gl_b[2][0];
	siw_pdf_pdf_bg_sel_gl_13.pdf_bg_pt_21b = pbg_gl_inUnit->bg_gl_b[2][1];
	siw_pdf_pdf_bg_sel_gl_13.pdf_bg_pt_20r = pbg_gl_inUnit->bg_gl_r[2][0];
	siw_pdf_pdf_bg_sel_gl_14.pdf_bg_pt_21r = pbg_gl_inUnit->bg_gl_r[2][1];
	siw_pdf_pdf_bg_sel_gl_14.pdf_bg_pt_21g = pbg_gl_inUnit->bg_gl_g[2][1];
	siw_pdf_pdf_bg_sel_gl_15.pdf_bg_pt_22g = pbg_gl_inUnit->bg_gl_g[2][2];
	siw_pdf_pdf_bg_sel_gl_15.pdf_bg_pt_22b = pbg_gl_inUnit->bg_gl_b[2][2];
	siw_pdf_pdf_bg_sel_gl_16.pdf_bg_pt_23b = pbg_gl_inUnit->bg_gl_b[2][3];
	siw_pdf_pdf_bg_sel_gl_16.pdf_bg_pt_22r = pbg_gl_inUnit->bg_gl_r[2][2];
	siw_pdf_pdf_bg_sel_gl_17.pdf_bg_pt_23r = pbg_gl_inUnit->bg_gl_r[2][3];
	siw_pdf_pdf_bg_sel_gl_17.pdf_bg_pt_23g = pbg_gl_inUnit->bg_gl_g[2][3];
	siw_pdf_pdf_bg_sel_gl_18.pdf_bg_pt_30g = pbg_gl_inUnit->bg_gl_g[3][0];
	siw_pdf_pdf_bg_sel_gl_18.pdf_bg_pt_30b = pbg_gl_inUnit->bg_gl_b[3][0];
	siw_pdf_pdf_bg_sel_gl_19.pdf_bg_pt_31b = pbg_gl_inUnit->bg_gl_b[3][1];
	siw_pdf_pdf_bg_sel_gl_19.pdf_bg_pt_30r = pbg_gl_inUnit->bg_gl_r[3][0];
	siw_pdf_pdf_bg_sel_gl_20.pdf_bg_pt_31r = pbg_gl_inUnit->bg_gl_r[3][1];
	siw_pdf_pdf_bg_sel_gl_20.pdf_bg_pt_31g = pbg_gl_inUnit->bg_gl_g[3][1];
	siw_pdf_pdf_bg_sel_gl_21.pdf_bg_pt_32g = pbg_gl_inUnit->bg_gl_g[3][2];
	siw_pdf_pdf_bg_sel_gl_21.pdf_bg_pt_32b = pbg_gl_inUnit->bg_gl_b[3][2];
	siw_pdf_pdf_bg_sel_gl_22.pdf_bg_pt_33b = pbg_gl_inUnit->bg_gl_b[3][3];
	siw_pdf_pdf_bg_sel_gl_22.pdf_bg_pt_32r = pbg_gl_inUnit->bg_gl_r[3][2];
	siw_pdf_pdf_bg_sel_gl_23.pdf_bg_pt_33r = pbg_gl_inUnit->bg_gl_r[3][3];
	siw_pdf_pdf_bg_sel_gl_23.pdf_bg_pt_33g = pbg_gl_inUnit->bg_gl_g[3][3];

	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_0_reg, siw_pdf_pdf_bg_sel_gl_0.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_1_reg, siw_pdf_pdf_bg_sel_gl_1.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_2_reg, siw_pdf_pdf_bg_sel_gl_2.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_3_reg, siw_pdf_pdf_bg_sel_gl_3.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_4_reg, siw_pdf_pdf_bg_sel_gl_4.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_5_reg, siw_pdf_pdf_bg_sel_gl_5.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_6_reg, siw_pdf_pdf_bg_sel_gl_6.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_7_reg, siw_pdf_pdf_bg_sel_gl_7.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_8_reg, siw_pdf_pdf_bg_sel_gl_8.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_9_reg, siw_pdf_pdf_bg_sel_gl_9.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_10_reg, siw_pdf_pdf_bg_sel_gl_10.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_11_reg, siw_pdf_pdf_bg_sel_gl_11.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_12_reg, siw_pdf_pdf_bg_sel_gl_12.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_13_reg, siw_pdf_pdf_bg_sel_gl_13.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_14_reg, siw_pdf_pdf_bg_sel_gl_14.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_15_reg, siw_pdf_pdf_bg_sel_gl_15.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_16_reg, siw_pdf_pdf_bg_sel_gl_16.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_17_reg, siw_pdf_pdf_bg_sel_gl_17.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_18_reg, siw_pdf_pdf_bg_sel_gl_18.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_19_reg, siw_pdf_pdf_bg_sel_gl_19.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_20_reg, siw_pdf_pdf_bg_sel_gl_20.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_21_reg, siw_pdf_pdf_bg_sel_gl_21.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_22_reg, siw_pdf_pdf_bg_sel_gl_22.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_GL_23_reg, siw_pdf_pdf_bg_sel_gl_23.regValue);
}

void drvif_color_set_pdf_bg_sel_opr(DRV_PDF_bg_sel_opr_t *pbg_sel_opr)
{
	siw_pdf_pdf_bg_sel_opr_0_RBUS siw_pdf_pdf_bg_sel_opr_0;	
	siw_pdf_pdf_bg_sel_opr_1_RBUS siw_pdf_pdf_bg_sel_opr_1;	
	siw_pdf_pdf_bg_sel_opr_2_RBUS siw_pdf_pdf_bg_sel_opr_2;	
	siw_pdf_pdf_bg_sel_opr_3_RBUS siw_pdf_pdf_bg_sel_opr_3;	
	siw_pdf_pdf_bg_sel_opr_4_RBUS siw_pdf_pdf_bg_sel_opr_4;	
	siw_pdf_pdf_bg_sel_opr_5_RBUS siw_pdf_pdf_bg_sel_opr_5;	
	siw_pdf_pdf_bg_sel_opr_6_RBUS siw_pdf_pdf_bg_sel_opr_6;	
	siw_pdf_pdf_bg_sel_opr_7_RBUS siw_pdf_pdf_bg_sel_opr_7;	
	siw_pdf_pdf_bg_sel_opr_8_RBUS siw_pdf_pdf_bg_sel_opr_8;	
	siw_pdf_pdf_bg_sel_opr_9_RBUS siw_pdf_pdf_bg_sel_opr_9;	
	siw_pdf_pdf_bg_sel_opr_10_RBUS siw_pdf_pdf_bg_sel_opr_10;	
	siw_pdf_pdf_bg_sel_opr_11_RBUS siw_pdf_pdf_bg_sel_opr_11;	
	siw_pdf_pdf_bg_sel_opr_12_RBUS siw_pdf_pdf_bg_sel_opr_12;	
	siw_pdf_pdf_bg_sel_opr_13_RBUS siw_pdf_pdf_bg_sel_opr_13;	
	siw_pdf_pdf_bg_sel_opr_14_RBUS siw_pdf_pdf_bg_sel_opr_14;	
	siw_pdf_pdf_bg_sel_opr_15_RBUS siw_pdf_pdf_bg_sel_opr_15;	

	siw_pdf_pdf_bg_sel_opr_0.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_0_reg);
	siw_pdf_pdf_bg_sel_opr_1.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_1_reg);
	siw_pdf_pdf_bg_sel_opr_2.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_2_reg);
	siw_pdf_pdf_bg_sel_opr_3.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_3_reg);
	siw_pdf_pdf_bg_sel_opr_4.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_4_reg);
	siw_pdf_pdf_bg_sel_opr_5.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_5_reg);
	siw_pdf_pdf_bg_sel_opr_6.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_6_reg);
	siw_pdf_pdf_bg_sel_opr_7.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_7_reg);
	siw_pdf_pdf_bg_sel_opr_8.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_8_reg);
	siw_pdf_pdf_bg_sel_opr_9.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_9_reg);
	siw_pdf_pdf_bg_sel_opr_10.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_10_reg);
	siw_pdf_pdf_bg_sel_opr_11.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_11_reg);
	siw_pdf_pdf_bg_sel_opr_12.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_12_reg);
	siw_pdf_pdf_bg_sel_opr_13.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_13_reg);
	siw_pdf_pdf_bg_sel_opr_14.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_14_reg);
	siw_pdf_pdf_bg_sel_opr_15.regValue = IoReg_Read32(SIW_PDF_PDF_BG_SEL_OPR_15_reg);

	siw_pdf_pdf_bg_sel_opr_0.pdf_bg_gl_opr_00r = pbg_sel_opr->bg_opr_r[0][0];
	siw_pdf_pdf_bg_sel_opr_0.pdf_bg_gl_opr_00g = pbg_sel_opr->bg_opr_g[0][0];
	siw_pdf_pdf_bg_sel_opr_0.pdf_bg_gl_opr_00b = pbg_sel_opr->bg_opr_b[0][0];
	siw_pdf_pdf_bg_sel_opr_1.pdf_bg_gl_opr_01r = pbg_sel_opr->bg_opr_r[0][1];
	siw_pdf_pdf_bg_sel_opr_1.pdf_bg_gl_opr_01g = pbg_sel_opr->bg_opr_g[0][1];
	siw_pdf_pdf_bg_sel_opr_1.pdf_bg_gl_opr_01b = pbg_sel_opr->bg_opr_b[0][1];
	siw_pdf_pdf_bg_sel_opr_2.pdf_bg_gl_opr_02r = pbg_sel_opr->bg_opr_r[0][2];
	siw_pdf_pdf_bg_sel_opr_2.pdf_bg_gl_opr_02g = pbg_sel_opr->bg_opr_g[0][2];
	siw_pdf_pdf_bg_sel_opr_2.pdf_bg_gl_opr_02b = pbg_sel_opr->bg_opr_b[0][2];
	siw_pdf_pdf_bg_sel_opr_3.pdf_bg_gl_opr_03r = pbg_sel_opr->bg_opr_r[0][3];
	siw_pdf_pdf_bg_sel_opr_3.pdf_bg_gl_opr_03g = pbg_sel_opr->bg_opr_g[0][3];
	siw_pdf_pdf_bg_sel_opr_3.pdf_bg_gl_opr_03b = pbg_sel_opr->bg_opr_b[0][3];
	siw_pdf_pdf_bg_sel_opr_4.pdf_bg_gl_opr_10r = pbg_sel_opr->bg_opr_r[1][0];
	siw_pdf_pdf_bg_sel_opr_4.pdf_bg_gl_opr_10g = pbg_sel_opr->bg_opr_g[1][0];
	siw_pdf_pdf_bg_sel_opr_4.pdf_bg_gl_opr_10b = pbg_sel_opr->bg_opr_b[1][0];
	siw_pdf_pdf_bg_sel_opr_5.pdf_bg_gl_opr_11r = pbg_sel_opr->bg_opr_r[1][1];
	siw_pdf_pdf_bg_sel_opr_5.pdf_bg_gl_opr_11g = pbg_sel_opr->bg_opr_g[1][1];
	siw_pdf_pdf_bg_sel_opr_5.pdf_bg_gl_opr_11b = pbg_sel_opr->bg_opr_b[1][1];
	siw_pdf_pdf_bg_sel_opr_6.pdf_bg_gl_opr_12r = pbg_sel_opr->bg_opr_r[1][2];
	siw_pdf_pdf_bg_sel_opr_6.pdf_bg_gl_opr_12g = pbg_sel_opr->bg_opr_g[1][2];
	siw_pdf_pdf_bg_sel_opr_6.pdf_bg_gl_opr_12b = pbg_sel_opr->bg_opr_b[1][2];
	siw_pdf_pdf_bg_sel_opr_7.pdf_bg_gl_opr_13r = pbg_sel_opr->bg_opr_r[1][3];
	siw_pdf_pdf_bg_sel_opr_7.pdf_bg_gl_opr_13g = pbg_sel_opr->bg_opr_g[1][3];
	siw_pdf_pdf_bg_sel_opr_7.pdf_bg_gl_opr_13b = pbg_sel_opr->bg_opr_b[1][3];
	siw_pdf_pdf_bg_sel_opr_8.pdf_bg_gl_opr_20r = pbg_sel_opr->bg_opr_r[2][0];
	siw_pdf_pdf_bg_sel_opr_8.pdf_bg_gl_opr_20g = pbg_sel_opr->bg_opr_g[2][0];
	siw_pdf_pdf_bg_sel_opr_8.pdf_bg_gl_opr_20b = pbg_sel_opr->bg_opr_b[2][0];
	siw_pdf_pdf_bg_sel_opr_9.pdf_bg_gl_opr_21r = pbg_sel_opr->bg_opr_r[2][1];
	siw_pdf_pdf_bg_sel_opr_9.pdf_bg_gl_opr_21g = pbg_sel_opr->bg_opr_g[2][1];
	siw_pdf_pdf_bg_sel_opr_9.pdf_bg_gl_opr_21b = pbg_sel_opr->bg_opr_b[2][1];
	siw_pdf_pdf_bg_sel_opr_10.pdf_bg_gl_opr_22r = pbg_sel_opr->bg_opr_r[2][2];
	siw_pdf_pdf_bg_sel_opr_10.pdf_bg_gl_opr_22g = pbg_sel_opr->bg_opr_g[2][2];
	siw_pdf_pdf_bg_sel_opr_10.pdf_bg_gl_opr_22b = pbg_sel_opr->bg_opr_b[2][2];
	siw_pdf_pdf_bg_sel_opr_11.pdf_bg_gl_opr_23r = pbg_sel_opr->bg_opr_r[2][3];
	siw_pdf_pdf_bg_sel_opr_11.pdf_bg_gl_opr_23g = pbg_sel_opr->bg_opr_g[2][3];
	siw_pdf_pdf_bg_sel_opr_11.pdf_bg_gl_opr_23b = pbg_sel_opr->bg_opr_b[2][3];
	siw_pdf_pdf_bg_sel_opr_12.pdf_bg_gl_opr_30r = pbg_sel_opr->bg_opr_r[3][0];
	siw_pdf_pdf_bg_sel_opr_12.pdf_bg_gl_opr_30g = pbg_sel_opr->bg_opr_g[3][0];
	siw_pdf_pdf_bg_sel_opr_12.pdf_bg_gl_opr_30b = pbg_sel_opr->bg_opr_b[3][0];
	siw_pdf_pdf_bg_sel_opr_13.pdf_bg_gl_opr_31r = pbg_sel_opr->bg_opr_r[3][1];
	siw_pdf_pdf_bg_sel_opr_13.pdf_bg_gl_opr_31g = pbg_sel_opr->bg_opr_g[3][1];
	siw_pdf_pdf_bg_sel_opr_13.pdf_bg_gl_opr_31b = pbg_sel_opr->bg_opr_b[3][1];
	siw_pdf_pdf_bg_sel_opr_14.pdf_bg_gl_opr_32r = pbg_sel_opr->bg_opr_r[3][2];
	siw_pdf_pdf_bg_sel_opr_14.pdf_bg_gl_opr_32g = pbg_sel_opr->bg_opr_g[3][2];
	siw_pdf_pdf_bg_sel_opr_14.pdf_bg_gl_opr_32b = pbg_sel_opr->bg_opr_b[3][2];
	siw_pdf_pdf_bg_sel_opr_15.pdf_bg_gl_opr_33r = pbg_sel_opr->bg_opr_r[3][3];
	siw_pdf_pdf_bg_sel_opr_15.pdf_bg_gl_opr_33g = pbg_sel_opr->bg_opr_g[3][3];
	siw_pdf_pdf_bg_sel_opr_15.pdf_bg_gl_opr_33b = pbg_sel_opr->bg_opr_b[3][3];

	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_0_reg, siw_pdf_pdf_bg_sel_opr_0.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_1_reg, siw_pdf_pdf_bg_sel_opr_1.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_2_reg, siw_pdf_pdf_bg_sel_opr_2.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_3_reg, siw_pdf_pdf_bg_sel_opr_3.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_4_reg, siw_pdf_pdf_bg_sel_opr_4.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_5_reg, siw_pdf_pdf_bg_sel_opr_5.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_6_reg, siw_pdf_pdf_bg_sel_opr_6.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_7_reg, siw_pdf_pdf_bg_sel_opr_7.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_8_reg, siw_pdf_pdf_bg_sel_opr_8.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_9_reg, siw_pdf_pdf_bg_sel_opr_9.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_10_reg, siw_pdf_pdf_bg_sel_opr_10.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_11_reg, siw_pdf_pdf_bg_sel_opr_11.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_12_reg, siw_pdf_pdf_bg_sel_opr_12.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_13_reg, siw_pdf_pdf_bg_sel_opr_13.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_14_reg, siw_pdf_pdf_bg_sel_opr_14.regValue);
	IoReg_Write32(SIW_PDF_PDF_BG_SEL_OPR_15_reg, siw_pdf_pdf_bg_sel_opr_15.regValue);
	
}


