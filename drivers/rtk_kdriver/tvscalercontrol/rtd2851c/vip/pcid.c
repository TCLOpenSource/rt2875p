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


#include "rtk_vip_logger.h"
#include <rtd_log/rtd_module_log.h>
#include <linux/semaphore.h>

#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/pcid.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/vip/vip_reg_def.h>

#include <rbus/pcid_reg.h>

/*================================ Definitions ==============================*/
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_PCID_DEBUG,fmt,##args)

/*================================== Function ===============================*/
void drvif_color_get_pcid2_pixel_setting(DRV_pcid2_data_t* pPcid2Setting)
{
	pcid_pcid_ctrl_RBUS pcid_ctrl_reg;
	pcid_pcid_ctrl_2_RBUS pcid_ctrl2_reg;
	pcid_pcid_ctrl_3_RBUS pcid_ctrl3_reg;
	pcid_pcid_line_1_RBUS pcid_line1_reg;
	pcid_pcid_line_2_RBUS pcid_line2_reg;
	pcid_pcid_line_3_RBUS pcid_line3_reg;
	pcid_pcid_line_4_RBUS pcid_line4_reg;

	pcid_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_reg);
	pcid_ctrl2_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_2_reg);
	pcid_ctrl3_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_3_reg);
	pcid_line1_reg.regValue = IoReg_Read32(PCID_PCID_LINE_1_reg);
	pcid_line2_reg.regValue = IoReg_Read32(PCID_PCID_LINE_2_reg);
	pcid_line3_reg.regValue = IoReg_Read32(PCID_PCID_LINE_3_reg);
	pcid_line4_reg.regValue = IoReg_Read32(PCID_PCID_LINE_4_reg);

	/* Enable control */
	pPcid2Setting->boundary_mode = pcid_ctrl_reg.boundary_mode;
	/* //mac6p removed
	pcid_ctrl_reg.lsb_add_en             = pPcid2Setting->lsb_add_en;
	pcid_ctrl_reg.lsb_add_nr_en          = pPcid2Setting->lsb_add_nr_en;
	pcid_ctrl_reg.lsb_add_nr_thd         = pPcid2Setting->lsb_add_nr_thd;
	pcid_ctrl_reg.lsb_add_slope_check_en = pPcid2Setting->lsb_add_slope_check_en;
	*/

	pPcid2Setting->pcid_thd_en = pcid_ctrl2_reg.pcid_thd_en;
	pPcid2Setting->pcid_thd_mode = pcid_ctrl2_reg.pcid_thd_mode;
	pPcid2Setting->pcid_tbl1_r_th = pcid_ctrl2_reg.pcid_tab1_r_thd;
	pPcid2Setting->pcid_tbl1_g_th = pcid_ctrl2_reg.pcid_tab1_g_thd;
	pPcid2Setting->pcid_tbl1_b_th = pcid_ctrl2_reg.pcid_tab1_b_thd;
	pPcid2Setting->xtr_tbl1_en = pcid_ctrl2_reg.pcid_tab1_xtr_en;
	pPcid2Setting->xtr_tbl2_en = pcid_ctrl2_reg.pcid_tab2_xtr_en;

	pPcid2Setting->pcid_tbl2_r_th = pcid_ctrl3_reg.pcid_tab2_r_thd;
	pPcid2Setting->pcid_tbl2_g_th = pcid_ctrl3_reg.pcid_tab2_g_thd;
	pPcid2Setting->pcid_tbl2_b_th = pcid_ctrl3_reg.pcid_tab2_b_thd;

	/* LINE 1 EVEN PIXEL */
	pPcid2Setting->pixel_ref.line1_even_r.line_sel = pcid_line1_reg.line1_even_r_line_select;
	pPcid2Setting->pixel_ref.line1_even_r.point_sel = pcid_line1_reg.line1_even_r_point_select;
	pPcid2Setting->pixel_ref.line1_even_r.color_sel = pcid_line1_reg.line1_even_r_color_select;
	pPcid2Setting->pixel_ref.line1_even_g.line_sel = pcid_line1_reg.line1_even_g_line_select;
	pPcid2Setting->pixel_ref.line1_even_g.point_sel = pcid_line1_reg.line1_even_g_point_select;
	pPcid2Setting->pixel_ref.line1_even_g.color_sel = pcid_line1_reg.line1_even_g_color_select;
	pPcid2Setting->pixel_ref.line1_even_b.line_sel = pcid_line1_reg.line1_even_b_line_select;
	pPcid2Setting->pixel_ref.line1_even_b.point_sel = pcid_line1_reg.line1_even_b_point_select;
	pPcid2Setting->pixel_ref.line1_even_b.color_sel = pcid_line1_reg.line1_even_b_color_select;

	/* LINE 1 ODD PIXEL */
	pPcid2Setting->pixel_ref.line1_odd_r.line_sel = pcid_line1_reg.line1_odd_r_line_select;
	pPcid2Setting->pixel_ref.line1_odd_r.point_sel = pcid_line1_reg.line1_odd_r_point_select;
	pPcid2Setting->pixel_ref.line1_odd_r.color_sel = pcid_line1_reg.line1_odd_r_color_select;
	pPcid2Setting->pixel_ref.line1_odd_g.line_sel = pcid_line1_reg.line1_odd_g_line_select;
	pPcid2Setting->pixel_ref.line1_odd_g.point_sel = pcid_line1_reg.line1_odd_g_point_select;
	pPcid2Setting->pixel_ref.line1_odd_g.color_sel = pcid_line1_reg.line1_odd_g_color_select;
	pPcid2Setting->pixel_ref.line1_odd_b.line_sel = pcid_line1_reg.line1_odd_b_line_select;
	pPcid2Setting->pixel_ref.line1_odd_b.point_sel = pcid_line1_reg.line1_odd_b_point_select;
	pPcid2Setting->pixel_ref.line1_odd_b.color_sel = pcid_line1_reg.line1_odd_b_color_select;

	/* LINE 2 EVEN PIXEL */
	pPcid2Setting->pixel_ref.line2_even_r.line_sel = pcid_line2_reg.line2_even_r_line_select;
	pPcid2Setting->pixel_ref.line2_even_r.point_sel = pcid_line2_reg.line2_even_r_point_select;
	pPcid2Setting->pixel_ref.line2_even_r.color_sel = pcid_line2_reg.line2_even_r_color_select;
	pPcid2Setting->pixel_ref.line2_even_g.line_sel = pcid_line2_reg.line2_even_g_line_select;
	pPcid2Setting->pixel_ref.line2_even_g.point_sel = pcid_line2_reg.line2_even_g_point_select;
	pPcid2Setting->pixel_ref.line2_even_g.color_sel = pcid_line2_reg.line2_even_g_color_select;
	pPcid2Setting->pixel_ref.line2_even_b.line_sel = pcid_line2_reg.line2_even_b_line_select;
	pPcid2Setting->pixel_ref.line2_even_b.point_sel = pcid_line2_reg.line2_even_b_point_select;
	pPcid2Setting->pixel_ref.line2_even_b.color_sel = pcid_line2_reg.line2_even_b_color_select;

	/* LINE 2 ODD PIXEL */
	pPcid2Setting->pixel_ref.line2_odd_r.line_sel = pcid_line2_reg.line2_odd_r_line_select;
	pPcid2Setting->pixel_ref.line2_odd_r.point_sel = pcid_line2_reg.line2_odd_r_point_select;
	pPcid2Setting->pixel_ref.line2_odd_r.color_sel = pcid_line2_reg.line2_odd_r_color_select;
	pPcid2Setting->pixel_ref.line2_odd_g.line_sel = pcid_line2_reg.line2_odd_g_line_select;
	pPcid2Setting->pixel_ref.line2_odd_g.point_sel = pcid_line2_reg.line2_odd_g_point_select;
	pPcid2Setting->pixel_ref.line2_odd_g.color_sel = pcid_line2_reg.line2_odd_g_color_select;
	pPcid2Setting->pixel_ref.line2_odd_b.line_sel = pcid_line2_reg.line2_odd_b_line_select;
	pPcid2Setting->pixel_ref.line2_odd_b.point_sel = pcid_line2_reg.line2_odd_b_point_select;
	pPcid2Setting->pixel_ref.line2_odd_b.color_sel = pcid_line2_reg.line2_odd_b_color_select;

	/* LINE 3 EVEN PIXEL */
	pPcid2Setting->pixel_ref.line3_even_r.line_sel = pcid_line3_reg.line3_even_r_line_select;
	pPcid2Setting->pixel_ref.line3_even_r.point_sel = pcid_line3_reg.line3_even_r_point_select;
 	pPcid2Setting->pixel_ref.line3_even_r.color_sel = pcid_line3_reg.line3_even_r_color_select;
 	pPcid2Setting->pixel_ref.line3_even_g.line_sel = pcid_line3_reg.line3_even_g_line_select;
 	pPcid2Setting->pixel_ref.line3_even_g.point_sel = pcid_line3_reg.line3_even_g_point_select;
 	pPcid2Setting->pixel_ref.line3_even_g.color_sel = pcid_line3_reg.line3_even_g_color_select;
 	pPcid2Setting->pixel_ref.line3_even_b.line_sel = pcid_line3_reg.line3_even_b_line_select;
 	pPcid2Setting->pixel_ref.line3_even_b.point_sel = pcid_line3_reg.line3_even_b_point_select;
 	pPcid2Setting->pixel_ref.line3_even_b.color_sel = pcid_line3_reg.line3_even_b_color_select;

	/* LINE 3 ODD PIXEL */
	pPcid2Setting->pixel_ref.line3_odd_r.line_sel = pcid_line3_reg.line3_odd_r_line_select;
	pPcid2Setting->pixel_ref.line3_odd_r.point_sel = pcid_line3_reg.line3_odd_r_point_select;
	pPcid2Setting->pixel_ref.line3_odd_r.color_sel = pcid_line3_reg.line3_odd_r_color_select;
	pPcid2Setting->pixel_ref.line3_odd_g.line_sel = pcid_line3_reg.line3_odd_g_line_select;
	pPcid2Setting->pixel_ref.line3_odd_g.point_sel = pcid_line3_reg.line3_odd_g_point_select;
	pPcid2Setting->pixel_ref.line3_odd_g.color_sel = pcid_line3_reg.line3_odd_g_color_select;
	pPcid2Setting->pixel_ref.line3_odd_b.line_sel = pcid_line3_reg.line3_odd_b_line_select;
	pPcid2Setting->pixel_ref.line3_odd_b.point_sel = pcid_line3_reg.line3_odd_b_point_select;
	pPcid2Setting->pixel_ref.line3_odd_b.color_sel = pcid_line3_reg.line3_odd_b_color_select;

	/* LINE 4 EVEN PIXEL */
	pPcid2Setting->pixel_ref.line4_even_r.line_sel = pcid_line4_reg.line4_even_r_line_select;
	pPcid2Setting->pixel_ref.line4_even_r.point_sel =pcid_line4_reg.line4_even_r_point_select;
	pPcid2Setting->pixel_ref.line4_even_r.color_sel =pcid_line4_reg.line4_even_r_color_select;
	pPcid2Setting->pixel_ref.line4_even_g.line_sel = pcid_line4_reg.line4_even_g_line_select;
	pPcid2Setting->pixel_ref.line4_even_g.point_sel =pcid_line4_reg.line4_even_g_point_select;
	pPcid2Setting->pixel_ref.line4_even_g.color_sel =pcid_line4_reg.line4_even_g_color_select;
	pPcid2Setting->pixel_ref.line4_even_b.line_sel = pcid_line4_reg.line4_even_b_line_select;
	pPcid2Setting->pixel_ref.line4_even_b.point_sel =pcid_line4_reg.line4_even_b_point_select;
	pPcid2Setting->pixel_ref.line4_even_b.color_sel =pcid_line4_reg.line4_even_b_color_select;

	/* LINE 4 ODD PIXEL */
	pPcid2Setting->pixel_ref.line4_odd_r.line_sel = pcid_line4_reg.line4_odd_r_line_select;
	pPcid2Setting->pixel_ref.line4_odd_r.point_sel = pcid_line4_reg.line4_odd_r_point_select;
	pPcid2Setting->pixel_ref.line4_odd_r.color_sel = pcid_line4_reg.line4_odd_r_color_select;
	pPcid2Setting->pixel_ref.line4_odd_g.line_sel = pcid_line4_reg.line4_odd_g_line_select;
	pPcid2Setting->pixel_ref.line4_odd_g.point_sel = pcid_line4_reg.line4_odd_g_point_select;
	pPcid2Setting->pixel_ref.line4_odd_g.color_sel = pcid_line4_reg.line4_odd_g_color_select;
	pPcid2Setting->pixel_ref.line4_odd_b.line_sel = pcid_line4_reg.line4_odd_b_line_select;
	pPcid2Setting->pixel_ref.line4_odd_b.point_sel = pcid_line4_reg.line4_odd_b_point_select;
	pPcid2Setting->pixel_ref.line4_odd_b.color_sel = pcid_line4_reg.line4_odd_b_color_select;
}


void drvif_color_pcid2_pixel_setting(DRV_pcid2_data_t* pPcid2Setting)
{
	pcid_pcid_ctrl_RBUS pcid_ctrl_reg;
	pcid_pcid_ctrl_2_RBUS pcid_ctrl2_reg;
	pcid_pcid_ctrl_3_RBUS pcid_ctrl3_reg;
	pcid_pcid_line_1_RBUS pcid_line1_reg;
	pcid_pcid_line_2_RBUS pcid_line2_reg;
	pcid_pcid_line_3_RBUS pcid_line3_reg;
	pcid_pcid_line_4_RBUS pcid_line4_reg;

	pcid_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_reg);
	pcid_ctrl2_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_2_reg);
	pcid_ctrl3_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_3_reg);
	pcid_line1_reg.regValue = IoReg_Read32(PCID_PCID_LINE_1_reg);
	pcid_line2_reg.regValue = IoReg_Read32(PCID_PCID_LINE_2_reg);
	pcid_line3_reg.regValue = IoReg_Read32(PCID_PCID_LINE_3_reg);
	pcid_line4_reg.regValue = IoReg_Read32(PCID_PCID_LINE_4_reg);

	/* Enable control */
	pcid_ctrl_reg.boundary_mode          = pPcid2Setting->boundary_mode;
	pcid_ctrl_reg.intp_bit_sel           = 1; // avoid interpolation precision lost
	/* //mac6p removed
	pcid_ctrl_reg.lsb_add_en             = pPcid2Setting->lsb_add_en;
	pcid_ctrl_reg.lsb_add_nr_en          = pPcid2Setting->lsb_add_nr_en;
	pcid_ctrl_reg.lsb_add_nr_thd         = pPcid2Setting->lsb_add_nr_thd;
	pcid_ctrl_reg.lsb_add_slope_check_en = pPcid2Setting->lsb_add_slope_check_en;
	*/

	pcid_ctrl2_reg.pcid_thd_en      = pPcid2Setting->pcid_thd_en;
	pcid_ctrl2_reg.pcid_thd_mode    = pPcid2Setting->pcid_thd_mode;
	pcid_ctrl2_reg.pcid_tab1_r_thd  = pPcid2Setting->pcid_tbl1_r_th;
	pcid_ctrl2_reg.pcid_tab1_g_thd  = pPcid2Setting->pcid_tbl1_g_th;
	pcid_ctrl2_reg.pcid_tab1_b_thd  = pPcid2Setting->pcid_tbl1_b_th;
	pcid_ctrl2_reg.pcid_tab1_xtr_en = pPcid2Setting->xtr_tbl1_en;
	pcid_ctrl2_reg.pcid_tab2_xtr_en = pPcid2Setting->xtr_tbl2_en;

	pcid_ctrl3_reg.pcid_tab2_r_thd  = pPcid2Setting->pcid_tbl2_r_th;
	pcid_ctrl3_reg.pcid_tab2_g_thd  = pPcid2Setting->pcid_tbl2_g_th;
	pcid_ctrl3_reg.pcid_tab2_b_thd  = pPcid2Setting->pcid_tbl2_b_th;

	/* LINE 1 EVEN PIXEL */
	pcid_line1_reg.line1_even_r_line_select  = pPcid2Setting->pixel_ref.line1_even_r.line_sel;
	pcid_line1_reg.line1_even_r_point_select = pPcid2Setting->pixel_ref.line1_even_r.point_sel;
	pcid_line1_reg.line1_even_r_color_select = pPcid2Setting->pixel_ref.line1_even_r.color_sel;
	pcid_line1_reg.line1_even_g_line_select  = pPcid2Setting->pixel_ref.line1_even_g.line_sel;
	pcid_line1_reg.line1_even_g_point_select = pPcid2Setting->pixel_ref.line1_even_g.point_sel;
	pcid_line1_reg.line1_even_g_color_select = pPcid2Setting->pixel_ref.line1_even_g.color_sel;
	pcid_line1_reg.line1_even_b_line_select  = pPcid2Setting->pixel_ref.line1_even_b.line_sel;
	pcid_line1_reg.line1_even_b_point_select = pPcid2Setting->pixel_ref.line1_even_b.point_sel;
	pcid_line1_reg.line1_even_b_color_select = pPcid2Setting->pixel_ref.line1_even_b.color_sel;

	/* LINE 1 ODD PIXEL */
	pcid_line1_reg.line1_odd_r_line_select  = pPcid2Setting->pixel_ref.line1_odd_r.line_sel;
	pcid_line1_reg.line1_odd_r_point_select = pPcid2Setting->pixel_ref.line1_odd_r.point_sel;
	pcid_line1_reg.line1_odd_r_color_select = pPcid2Setting->pixel_ref.line1_odd_r.color_sel;
	pcid_line1_reg.line1_odd_g_line_select  = pPcid2Setting->pixel_ref.line1_odd_g.line_sel;
	pcid_line1_reg.line1_odd_g_point_select = pPcid2Setting->pixel_ref.line1_odd_g.point_sel;
	pcid_line1_reg.line1_odd_g_color_select = pPcid2Setting->pixel_ref.line1_odd_g.color_sel;
	pcid_line1_reg.line1_odd_b_line_select  = pPcid2Setting->pixel_ref.line1_odd_b.line_sel;
	pcid_line1_reg.line1_odd_b_point_select = pPcid2Setting->pixel_ref.line1_odd_b.point_sel;
	pcid_line1_reg.line1_odd_b_color_select = pPcid2Setting->pixel_ref.line1_odd_b.color_sel;

	/* LINE 2 EVEN PIXEL */
	pcid_line2_reg.line2_even_r_line_select  = pPcid2Setting->pixel_ref.line2_even_r.line_sel;
	pcid_line2_reg.line2_even_r_point_select = pPcid2Setting->pixel_ref.line2_even_r.point_sel;
	pcid_line2_reg.line2_even_r_color_select = pPcid2Setting->pixel_ref.line2_even_r.color_sel;
	pcid_line2_reg.line2_even_g_line_select  = pPcid2Setting->pixel_ref.line2_even_g.line_sel;
	pcid_line2_reg.line2_even_g_point_select = pPcid2Setting->pixel_ref.line2_even_g.point_sel;
	pcid_line2_reg.line2_even_g_color_select = pPcid2Setting->pixel_ref.line2_even_g.color_sel;
	pcid_line2_reg.line2_even_b_line_select  = pPcid2Setting->pixel_ref.line2_even_b.line_sel;
	pcid_line2_reg.line2_even_b_point_select = pPcid2Setting->pixel_ref.line2_even_b.point_sel;
	pcid_line2_reg.line2_even_b_color_select = pPcid2Setting->pixel_ref.line2_even_b.color_sel;

	/* LINE 2 ODD PIXEL */
	pcid_line2_reg.line2_odd_r_line_select  = pPcid2Setting->pixel_ref.line2_odd_r.line_sel;
	pcid_line2_reg.line2_odd_r_point_select = pPcid2Setting->pixel_ref.line2_odd_r.point_sel;
	pcid_line2_reg.line2_odd_r_color_select = pPcid2Setting->pixel_ref.line2_odd_r.color_sel;
	pcid_line2_reg.line2_odd_g_line_select  = pPcid2Setting->pixel_ref.line2_odd_g.line_sel;
	pcid_line2_reg.line2_odd_g_point_select = pPcid2Setting->pixel_ref.line2_odd_g.point_sel;
	pcid_line2_reg.line2_odd_g_color_select = pPcid2Setting->pixel_ref.line2_odd_g.color_sel;
	pcid_line2_reg.line2_odd_b_line_select  = pPcid2Setting->pixel_ref.line2_odd_b.line_sel;
	pcid_line2_reg.line2_odd_b_point_select = pPcid2Setting->pixel_ref.line2_odd_b.point_sel;
	pcid_line2_reg.line2_odd_b_color_select = pPcid2Setting->pixel_ref.line2_odd_b.color_sel;

	/* LINE 3 EVEN PIXEL */
	pcid_line3_reg.line3_even_r_line_select  = pPcid2Setting->pixel_ref.line3_even_r.line_sel;
	pcid_line3_reg.line3_even_r_point_select = pPcid2Setting->pixel_ref.line3_even_r.point_sel;
	pcid_line3_reg.line3_even_r_color_select = pPcid2Setting->pixel_ref.line3_even_r.color_sel;
	pcid_line3_reg.line3_even_g_line_select  = pPcid2Setting->pixel_ref.line3_even_g.line_sel;
	pcid_line3_reg.line3_even_g_point_select = pPcid2Setting->pixel_ref.line3_even_g.point_sel;
	pcid_line3_reg.line3_even_g_color_select = pPcid2Setting->pixel_ref.line3_even_g.color_sel;
	pcid_line3_reg.line3_even_b_line_select  = pPcid2Setting->pixel_ref.line3_even_b.line_sel;
	pcid_line3_reg.line3_even_b_point_select = pPcid2Setting->pixel_ref.line3_even_b.point_sel;
	pcid_line3_reg.line3_even_b_color_select = pPcid2Setting->pixel_ref.line3_even_b.color_sel;

	/* LINE 3 ODD PIXEL */
	pcid_line3_reg.line3_odd_r_line_select  = pPcid2Setting->pixel_ref.line3_odd_r.line_sel;
	pcid_line3_reg.line3_odd_r_point_select = pPcid2Setting->pixel_ref.line3_odd_r.point_sel;
	pcid_line3_reg.line3_odd_r_color_select = pPcid2Setting->pixel_ref.line3_odd_r.color_sel;
	pcid_line3_reg.line3_odd_g_line_select  = pPcid2Setting->pixel_ref.line3_odd_g.line_sel;
	pcid_line3_reg.line3_odd_g_point_select = pPcid2Setting->pixel_ref.line3_odd_g.point_sel;
	pcid_line3_reg.line3_odd_g_color_select = pPcid2Setting->pixel_ref.line3_odd_g.color_sel;
	pcid_line3_reg.line3_odd_b_line_select  = pPcid2Setting->pixel_ref.line3_odd_b.line_sel;
	pcid_line3_reg.line3_odd_b_point_select = pPcid2Setting->pixel_ref.line3_odd_b.point_sel;
	pcid_line3_reg.line3_odd_b_color_select = pPcid2Setting->pixel_ref.line3_odd_b.color_sel;

	/* LINE 4 EVEN PIXEL */
	pcid_line4_reg.line4_even_r_line_select  = pPcid2Setting->pixel_ref.line4_even_r.line_sel;
	pcid_line4_reg.line4_even_r_point_select = pPcid2Setting->pixel_ref.line4_even_r.point_sel;
	pcid_line4_reg.line4_even_r_color_select = pPcid2Setting->pixel_ref.line4_even_r.color_sel;
	pcid_line4_reg.line4_even_g_line_select  = pPcid2Setting->pixel_ref.line4_even_g.line_sel;
	pcid_line4_reg.line4_even_g_point_select = pPcid2Setting->pixel_ref.line4_even_g.point_sel;
	pcid_line4_reg.line4_even_g_color_select = pPcid2Setting->pixel_ref.line4_even_g.color_sel;
	pcid_line4_reg.line4_even_b_line_select  = pPcid2Setting->pixel_ref.line4_even_b.line_sel;
	pcid_line4_reg.line4_even_b_point_select = pPcid2Setting->pixel_ref.line4_even_b.point_sel;
	pcid_line4_reg.line4_even_b_color_select = pPcid2Setting->pixel_ref.line4_even_b.color_sel;

	/* LINE 4 ODD PIXEL */
	pcid_line4_reg.line4_odd_r_line_select  = pPcid2Setting->pixel_ref.line4_odd_r.line_sel;
	pcid_line4_reg.line4_odd_r_point_select = pPcid2Setting->pixel_ref.line4_odd_r.point_sel;
	pcid_line4_reg.line4_odd_r_color_select = pPcid2Setting->pixel_ref.line4_odd_r.color_sel;
	pcid_line4_reg.line4_odd_g_line_select  = pPcid2Setting->pixel_ref.line4_odd_g.line_sel;
	pcid_line4_reg.line4_odd_g_point_select = pPcid2Setting->pixel_ref.line4_odd_g.point_sel;
	pcid_line4_reg.line4_odd_g_color_select = pPcid2Setting->pixel_ref.line4_odd_g.color_sel;
	pcid_line4_reg.line4_odd_b_line_select  = pPcid2Setting->pixel_ref.line4_odd_b.line_sel;
	pcid_line4_reg.line4_odd_b_point_select = pPcid2Setting->pixel_ref.line4_odd_b.point_sel;
	pcid_line4_reg.line4_odd_b_color_select = pPcid2Setting->pixel_ref.line4_odd_b.color_sel;

	IoReg_Write32(PCID_PCID_CTRL_reg  , pcid_ctrl_reg.regValue);
	IoReg_Write32(PCID_PCID_CTRL_2_reg, pcid_ctrl2_reg.regValue);
	IoReg_Write32(PCID_PCID_CTRL_3_reg, pcid_ctrl3_reg.regValue);
	IoReg_Write32(PCID_PCID_LINE_1_reg, pcid_line1_reg.regValue);
	IoReg_Write32(PCID_PCID_LINE_2_reg, pcid_line2_reg.regValue);
	IoReg_Write32(PCID_PCID_LINE_3_reg, pcid_line3_reg.regValue);
	IoReg_Write32(PCID_PCID_LINE_4_reg, pcid_line4_reg.regValue);
}

void drvif_color_pcid2_valuetable(unsigned int * ValueTBL, unsigned int TblSel, DRV_pcid2_channel_t Channel)
{
	int i=0;
	pcid_pcid_lut_addr_RBUS pcid_lut_addr_reg;
	pcid_pcid_lut_data_RBUS pcid_pcid_lut_data_reg;

	if(!ValueTBL)
	{
		VIPprintf("[Error][%s][%s] Table Ptr is null!\n", __FILE__, __func__);
		return;
	}


	// Setup table access setting
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_sel = Channel;
	pcid_lut_addr_reg.pcid_lut_adr_mode = 0;	/* Increase Current (column) Index first */
	pcid_lut_addr_reg.pcid_lut_ax_en = 1;		/* access en */
	pcid_lut_addr_reg.pcid_lut_row_addr = TblSel;
	pcid_lut_addr_reg.pcid_lut_column_addr = TblSel;

	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	// Write the value into table
	for(i=0; i<81; i++)
	{
		pcid_pcid_lut_data_reg.pcid_lut_data = ValueTBL[i];
		IoReg_Write32(PCID_PCID_LUT_DATA_reg, pcid_pcid_lut_data_reg.regValue);
		VIPprintf("[VPQ][POD]Table[%d]=0x%x\n",i,ValueTBL[i]);
	}

	// Disable the table access
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_ax_en = 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

}

void drvif_color_pcid2_poltable(unsigned int * pPcid2PolTable, DRV_pcid2_channel_t color)
{
	switch(color)
	{
		case _PCID2_COLOR_R:
		{
			pcid_pcid2_polarity_r_1_RBUS pcid_pcid2_polarity_r_1_reg;
			pcid_pcid2_polarity_r_2_RBUS pcid_pcid2_polarity_r_2_reg;
			pcid_pcid2_polarity_r_3_RBUS pcid_pcid2_polarity_r_3_reg;
			pcid_pcid2_polarity_r_4_RBUS pcid_pcid2_polarity_r_4_reg;

			pcid_pcid2_polarity_r_1_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_R_1_reg);
			pcid_pcid2_polarity_r_2_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_R_2_reg);
			pcid_pcid2_polarity_r_3_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_R_3_reg);
			pcid_pcid2_polarity_r_3_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_R_4_reg);

			pcid_pcid2_polarity_r_1_reg.r_po_line1 = pPcid2PolTable[0];
			pcid_pcid2_polarity_r_1_reg.r_po_line2 = pPcid2PolTable[1];
			pcid_pcid2_polarity_r_2_reg.r_po_line3 = pPcid2PolTable[2];
			pcid_pcid2_polarity_r_2_reg.r_po_line4 = pPcid2PolTable[3];
			pcid_pcid2_polarity_r_3_reg.r_po_line5 = pPcid2PolTable[4];
			pcid_pcid2_polarity_r_3_reg.r_po_line6 = pPcid2PolTable[5];
			pcid_pcid2_polarity_r_4_reg.r_po_line7 = pPcid2PolTable[6];
			pcid_pcid2_polarity_r_4_reg.r_po_line8 = pPcid2PolTable[7];

			IoReg_Write32(PCID_PCID2_POLARITY_R_1_reg, pcid_pcid2_polarity_r_1_reg.regValue);
			IoReg_Write32(PCID_PCID2_POLARITY_R_2_reg, pcid_pcid2_polarity_r_2_reg.regValue);
			IoReg_Write32(PCID_PCID2_POLARITY_R_3_reg, pcid_pcid2_polarity_r_3_reg.regValue);
			IoReg_Write32(PCID_PCID2_POLARITY_R_4_reg, pcid_pcid2_polarity_r_4_reg.regValue);
			break;
		}
		case _PCID2_COLOR_G:
		{
			pcid_pcid2_polarity_g_1_RBUS pcid_pcid2_polarity_g_1_reg;
			pcid_pcid2_polarity_g_2_RBUS pcid_pcid2_polarity_g_2_reg;
			pcid_pcid2_polarity_g_3_RBUS pcid_pcid2_polarity_g_3_reg;
			pcid_pcid2_polarity_g_4_RBUS pcid_pcid2_polarity_g_4_reg;

			pcid_pcid2_polarity_g_1_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_G_1_reg);
			pcid_pcid2_polarity_g_2_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_G_2_reg);
			pcid_pcid2_polarity_g_3_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_G_3_reg);
			pcid_pcid2_polarity_g_3_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_G_4_reg);

			pcid_pcid2_polarity_g_1_reg.g_po_line1 = pPcid2PolTable[0];
			pcid_pcid2_polarity_g_1_reg.g_po_line2 = pPcid2PolTable[1];
			pcid_pcid2_polarity_g_2_reg.g_po_line3 = pPcid2PolTable[2];
			pcid_pcid2_polarity_g_2_reg.g_po_line4 = pPcid2PolTable[3];
			pcid_pcid2_polarity_g_3_reg.g_po_line5 = pPcid2PolTable[4];
			pcid_pcid2_polarity_g_3_reg.g_po_line6 = pPcid2PolTable[5];
			pcid_pcid2_polarity_g_4_reg.g_po_line7 = pPcid2PolTable[6];
			pcid_pcid2_polarity_g_4_reg.g_po_line8 = pPcid2PolTable[7];

			IoReg_Write32(PCID_PCID2_POLARITY_G_1_reg, pcid_pcid2_polarity_g_1_reg.regValue);
			IoReg_Write32(PCID_PCID2_POLARITY_G_2_reg, pcid_pcid2_polarity_g_2_reg.regValue);
			IoReg_Write32(PCID_PCID2_POLARITY_G_3_reg, pcid_pcid2_polarity_g_3_reg.regValue);
			IoReg_Write32(PCID_PCID2_POLARITY_G_4_reg, pcid_pcid2_polarity_g_4_reg.regValue);
			break;
		}
		case _PCID2_COLOR_B:
		{
			pcid_pcid2_polarity_b_1_RBUS pcid_pcid2_polarity_b_1_reg;
			pcid_pcid2_polarity_b_2_RBUS pcid_pcid2_polarity_b_2_reg;
			pcid_pcid2_polarity_b_3_RBUS pcid_pcid2_polarity_b_3_reg;
			pcid_pcid2_polarity_b_4_RBUS pcid_pcid2_polarity_b_4_reg;

			pcid_pcid2_polarity_b_1_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_B_1_reg);
			pcid_pcid2_polarity_b_2_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_B_2_reg);
			pcid_pcid2_polarity_b_3_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_B_3_reg);
			pcid_pcid2_polarity_b_3_reg.regValue = IoReg_Read32(PCID_PCID2_POLARITY_B_4_reg);

			pcid_pcid2_polarity_b_1_reg.b_po_line1 = pPcid2PolTable[0];
			pcid_pcid2_polarity_b_1_reg.b_po_line2 = pPcid2PolTable[1];
			pcid_pcid2_polarity_b_2_reg.b_po_line3 = pPcid2PolTable[2];
			pcid_pcid2_polarity_b_2_reg.b_po_line4 = pPcid2PolTable[3];
			pcid_pcid2_polarity_b_3_reg.b_po_line5 = pPcid2PolTable[4];
			pcid_pcid2_polarity_b_3_reg.b_po_line6 = pPcid2PolTable[5];
			pcid_pcid2_polarity_b_4_reg.b_po_line7 = pPcid2PolTable[6];
			pcid_pcid2_polarity_b_4_reg.b_po_line8 = pPcid2PolTable[7];

			IoReg_Write32(PCID_PCID2_POLARITY_B_1_reg, pcid_pcid2_polarity_b_1_reg.regValue);
			IoReg_Write32(PCID_PCID2_POLARITY_B_2_reg, pcid_pcid2_polarity_b_2_reg.regValue);
			IoReg_Write32(PCID_PCID2_POLARITY_B_3_reg, pcid_pcid2_polarity_b_3_reg.regValue);
			IoReg_Write32(PCID_PCID2_POLARITY_B_4_reg, pcid_pcid2_polarity_b_4_reg.regValue);
			break;
		}
		default:
			break;
	}
}

void drvif_color_set_pcid_data_access_read(unsigned short x, unsigned short y, unsigned char loc, unsigned char cross_bar_en)
{

	pcid_pcid_accessdata_ctrl_RBUS pcid_pcid_accessdata_ctrl_reg;
	pcid_pcid_accessdata_posctrl_RBUS pcid_pcid_accessdata_posctrl_reg;

	pcid_pcid_accessdata_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_CTRL_reg);
	pcid_pcid_accessdata_posctrl_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_PosCtrl_reg);
	
	pcid_pcid_accessdata_posctrl_reg.access_startx = x;
	pcid_pcid_accessdata_posctrl_reg.access_starty = y;

	pcid_pcid_accessdata_ctrl_reg.access_cross_bar_en = cross_bar_en;
	pcid_pcid_accessdata_ctrl_reg.access_location_sel = loc;	// after PCID
	pcid_pcid_accessdata_ctrl_reg.access_read_en = 1;
	pcid_pcid_accessdata_ctrl_reg.access_write_en = 1;
	
	IoReg_Write32(PCID_PCID_AccessData_CTRL_reg, pcid_pcid_accessdata_ctrl_reg.regValue);
	IoReg_Write32(PCID_PCID_AccessData_PosCtrl_reg, pcid_pcid_accessdata_posctrl_reg.regValue);

}

char drvif_color_get_pcid_data_access_read(unsigned short *R, unsigned short *G, unsigned short *B)
{
	pcid_pcid_accessdata_ctrl_RBUS pcid_pcid_accessdata_ctrl_reg;
	pcid_pcid_accessdata_r_RBUS	pcid_pcid_accessdata_r_reg;
	pcid_pcid_accessdata_g_RBUS pcid_pcid_accessdata_g_reg;
	pcid_pcid_accessdata_b_RBUS pcid_pcid_accessdata_b_reg;
	char ret_val = 0;
	int timeout = 20;
	
	if (R == NULL || G == NULL || B == NULL) {
		rtd_pr_vpq_emerg("drvif_color_get_pcid_data_access_read, NULL\n");
		ret_val = -1;
		return -1;
	}
	
	pcid_pcid_accessdata_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_CTRL_reg);

	while ((timeout>=0) && (pcid_pcid_accessdata_ctrl_reg.access_read_en == 1)) {
		//ROSTimeDly(ROS_TICKS_PER_SEC / 1000 * 5);
		usleep_range(5000, 10000);
		timeout--;
		pcid_pcid_accessdata_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_CTRL_reg);
	}

	if (timeout <= 0) {
		rtd_pr_vpq_emerg("drvif_color_get_pcid_data_access_read, wait time out = %d,\n",timeout);
		ret_val = -1;
	} else {
		pcid_pcid_accessdata_r_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_R_reg);
		pcid_pcid_accessdata_g_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_G_reg);
		pcid_pcid_accessdata_b_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_B_reg);

		*R = pcid_pcid_accessdata_r_reg.access_read_r;
		*G = pcid_pcid_accessdata_g_reg.access_read_g;
		*B = pcid_pcid_accessdata_b_reg.access_read_b;
		ret_val = 0;
	}
	
	return ret_val;
	
}

unsigned int drvif_color_get_pcid_data_access_read_R_mask(void)
{
	return PCID_PCID_AccessData_R_access_read_r_mask;
}

void drvif_color_get_pcid2_enable(unsigned char* bpcid2)
{
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	*bpcid2 = display_timing_ctrl2_reg.lineod_en;
}


void drvif_color_pcid2_enable(unsigned char bpcid2)
{
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
#ifdef For_BringUp_Disable
	display_timing_ctrl2_reg.lineod_en= 0;
#else
	display_timing_ctrl2_reg.lineod_en= bpcid2;
#endif
	
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT8); // Set double buffer apply bit
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}



/* new pcid by willy */
void drvif_color_pcid_valuetable(unsigned int *pPcidtable)
{
#if 0
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
#endif

	int i=0;
	pcid_pcid_lut_addr_RBUS pcid_lut_addr_reg;
	pcid_pcid_lut_data_RBUS pcid_pcid_lut_data_reg;

	if (!pPcidtable)
	{
		VIPprintf("[Error][%s][%s] Table Ptr is null!\n", __FILE__, __func__);
		return;
	}

#if 0
	// ------------------------------------------------------------------------------------------------------
	// Enable module
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT10); // disable DTG double buffer
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.pcid2_en = 1;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
	IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT10); // enable DTG double buffer
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	// ------------------------------------------------------------------------------------------------------
#endif

	// Setup table access setting
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_sel = 3;			/* all channels */
	pcid_lut_addr_reg.pcid_lut_adr_mode = 0;	/* Increase Current (column) Index first */
	pcid_lut_addr_reg.pcid_lut_ax_en = 1;		/* access en */
	pcid_lut_addr_reg.pcid_lut_row_addr = 0;	/* PCID only uses table 1 */
	pcid_lut_addr_reg.pcid_lut_column_addr = 0;	/* PCID only uses table 1 */

	// Write the value into table
	for(i=0; i<81; i++)
	{
		pcid_pcid_lut_data_reg.pcid_lut_data = pPcidtable[i];
		IoReg_Write32(PCID_PCID_LUT_DATA_reg, pcid_pcid_lut_data_reg.regValue);
		VIPprintf("[VPQ][PCID]Table[%d]=0x%x\n",i,pPcidtable[i]);
	}

	// Disable the table access
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_ax_en = 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

#if 0
	// Disable the PCID enable
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT10); // disable DTG double buffer
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.pcid2_en = 0;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
	IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT10); // enable DTG double buffer
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
#endif
}

void drvif_color_pcid_poltable(void)
{
	IoReg_Write32(PCID_PCID2_POLARITY_R_1_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_R_2_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_R_3_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_R_4_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_G_1_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_G_2_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_G_3_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_G_4_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_B_1_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_B_2_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_B_3_reg, 0x00000000);
	IoReg_Write32(PCID_PCID2_POLARITY_B_4_reg, 0x00000000);
}
void drvif_color_get_pcid_enable(unsigned char* bpcid_enable)
{
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	*bpcid_enable = display_timing_ctrl2_reg.lineod_en;

}

void drvif_color_pcid_enable(unsigned char bpcid_enable)
{
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
#ifdef For_BringUp_Disable
	display_timing_ctrl2_reg.lineod_en= 0;
#else
	display_timing_ctrl2_reg.lineod_en= bpcid_enable;
#endif
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, _BIT8); // Set double buffer apply bit
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}
// Mac8p devide PCID to lineod and valc
/*void drvif_color_get_pcid_VALC_enable(int* ucEnable) // Mac8p devide PCID to lineod and valc
{
	pcid_pcid_valc_ctrl_RBUS pcid_pcid_valc_ctrl_reg;
	pcid_pcid_valc_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_VALC_CTRL_reg);
	*ucEnable = pcid_pcid_valc_ctrl_reg.valc_en;
}*/

// Mac8p devide PCID to lineod and valc
/*void drvif_color_pcid_VALC_enable(unsigned char ucEnable)
{
	pcid_pcid_valc_ctrl_RBUS pcid_pcid_valc_ctrl_reg;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	if(ucEnable != 0 && (display_timing_ctrl2_reg.pcid2_en!= 0 || display_timing_ctrl2_reg.pcid_en != 0))
	{
		if(display_timing_ctrl2_reg.pcid2_en!= 0 || display_timing_ctrl2_reg.pcid_en != 0)
		{
			pcid_pcid_valc_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_VALC_CTRL_reg);
			pcid_pcid_valc_ctrl_reg.valc_en = 1;
			IoReg_Write32(PCID_PCID_VALC_CTRL_reg, pcid_pcid_valc_ctrl_reg.regValue);
		}
		else
			VIPprintf("[Error][%s][%s] VALC cannot be enabled when pcid/pcid2 are disable\n", __FILE__, __func__);
	}

}*/
// Mac8p devide PCID to lineod and valc
/*void drvif_color_get_pcid_VALC_protection(DRV_pcid_valc_t* pVALCprotectSetting)
{
	pcid_pcid_valc_ctrl_RBUS pcid_pcid_valc_ctrl_reg;
	pcid_pcid_valc_hpf_0_RBUS pcid_pcid_valc_hpf_0_reg;
	pcid_pcid_valc_hpf_1_RBUS pcid_pcid_valc_hpf_1_reg;
	pcid_pcid_valc_sat_0_RBUS pcid_pcid_valc_sat_0_reg;
	pcid_pcid_valc_sat_1_RBUS pcid_pcid_valc_sat_1_reg;

	pcid_pcid_valc_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_VALC_CTRL_reg);
	pcid_pcid_valc_hpf_0_reg.regValue = IoReg_Read32(PCID_PCID_VALC_HPF_0_reg);
	pcid_pcid_valc_hpf_1_reg.regValue = IoReg_Read32(PCID_PCID_VALC_HPF_1_reg);
	pcid_pcid_valc_sat_0_reg.regValue = IoReg_Read32(PCID_PCID_VALC_SAT_0_reg);
	pcid_pcid_valc_sat_1_reg.regValue = IoReg_Read32(PCID_PCID_VALC_SAT_1_reg);

	//pcid_pcid_valc_ctrl_reg.valc_en = pVALCprotectSetting->valc_en;
	pVALCprotectSetting->hpf_clip = pcid_pcid_valc_ctrl_reg.valc_hpf_clip;

	pVALCprotectSetting->hpf_weight[0] = pcid_pcid_valc_hpf_1_reg.valc_hpf_w0;
	pVALCprotectSetting->hpf_weight[1] = pcid_pcid_valc_hpf_1_reg.valc_hpf_w1;
	pVALCprotectSetting->hpf_weight[2] = pcid_pcid_valc_hpf_1_reg.valc_hpf_w2;
	pVALCprotectSetting->hpf_weight[3] = pcid_pcid_valc_hpf_1_reg.valc_hpf_w3;
	pVALCprotectSetting->hpf_weight[4] = pcid_pcid_valc_hpf_1_reg.valc_hpf_w4;
	pVALCprotectSetting->hpf_weight[5] = pcid_pcid_valc_hpf_1_reg.valc_hpf_w5;
	pVALCprotectSetting->hpf_weight[6] = pcid_pcid_valc_hpf_1_reg.valc_hpf_w6;
	pVALCprotectSetting->hpf_weight[7] = pcid_pcid_valc_hpf_1_reg.valc_hpf_w7;

	pVALCprotectSetting->hpf_weight[8] = pcid_pcid_valc_hpf_0_reg.valc_hpf_w8;
	pVALCprotectSetting->hpf_weight[9] = pcid_pcid_valc_hpf_0_reg.valc_hpf_w9;
	pVALCprotectSetting->hpf_weight[10] = pcid_pcid_valc_hpf_0_reg.valc_hpf_w10;
	pVALCprotectSetting->hpf_weight[11] = pcid_pcid_valc_hpf_0_reg.valc_hpf_w11;
	pVALCprotectSetting->hpf_weight[12] = pcid_pcid_valc_hpf_0_reg.valc_hpf_w12;
	pVALCprotectSetting->hpf_weight[13] = pcid_pcid_valc_hpf_0_reg.valc_hpf_w13;
	pVALCprotectSetting->hpf_weight[14] = pcid_pcid_valc_hpf_0_reg.valc_hpf_w14;
	pVALCprotectSetting->hpf_weight[15] = pcid_pcid_valc_hpf_0_reg.valc_hpf_w15;

	pVALCprotectSetting->sat_weight[0] = pcid_pcid_valc_sat_1_reg.valc_sat_w0;
	pVALCprotectSetting->sat_weight[1] = pcid_pcid_valc_sat_1_reg.valc_sat_w1;
	pVALCprotectSetting->sat_weight[2] = pcid_pcid_valc_sat_1_reg.valc_sat_w2;
	pVALCprotectSetting->sat_weight[3] = pcid_pcid_valc_sat_1_reg.valc_sat_w3;
	pVALCprotectSetting->sat_weight[4] = pcid_pcid_valc_sat_1_reg.valc_sat_w4;
	pVALCprotectSetting->sat_weight[5] = pcid_pcid_valc_sat_1_reg.valc_sat_w5;
	pVALCprotectSetting->sat_weight[6] = pcid_pcid_valc_sat_1_reg.valc_sat_w6;
	pVALCprotectSetting->sat_weight[7] = pcid_pcid_valc_sat_1_reg.valc_sat_w7;

	pVALCprotectSetting->sat_weight[8] = pcid_pcid_valc_sat_0_reg.valc_sat_w8;
	pVALCprotectSetting->sat_weight[9] = pcid_pcid_valc_sat_0_reg.valc_sat_w9;
	pVALCprotectSetting->sat_weight[10] = pcid_pcid_valc_sat_0_reg.valc_sat_w10;
	pVALCprotectSetting->sat_weight[11] = pcid_pcid_valc_sat_0_reg.valc_sat_w11;
	pVALCprotectSetting->sat_weight[12] = pcid_pcid_valc_sat_0_reg.valc_sat_w12;
	pVALCprotectSetting->sat_weight[13] = pcid_pcid_valc_sat_0_reg.valc_sat_w13;
	pVALCprotectSetting->sat_weight[14] = pcid_pcid_valc_sat_0_reg.valc_sat_w14;
	pVALCprotectSetting->sat_weight[15] = pcid_pcid_valc_sat_0_reg.valc_sat_w15;
	
	drvif_color_get_pcid_VALC_enable(&(pVALCprotectSetting->valc_en));
}*/

// Mac8p devide PCID to lineod and valc
/*void drvif_color_pcid_VALC_protection(DRV_pcid_valc_t* pVALCprotectSetting)
{
	pcid_pcid_valc_ctrl_RBUS pcid_pcid_valc_ctrl_reg;
	pcid_pcid_valc_hpf_0_RBUS pcid_pcid_valc_hpf_0_reg;
	pcid_pcid_valc_hpf_1_RBUS pcid_pcid_valc_hpf_1_reg;
	pcid_pcid_valc_sat_0_RBUS pcid_pcid_valc_sat_0_reg;
	pcid_pcid_valc_sat_1_RBUS pcid_pcid_valc_sat_1_reg;

	pcid_pcid_valc_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_VALC_CTRL_reg);
	pcid_pcid_valc_hpf_0_reg.regValue = IoReg_Read32(PCID_PCID_VALC_HPF_0_reg);
	pcid_pcid_valc_hpf_1_reg.regValue = IoReg_Read32(PCID_PCID_VALC_HPF_1_reg);
	pcid_pcid_valc_sat_0_reg.regValue = IoReg_Read32(PCID_PCID_VALC_SAT_0_reg);
	pcid_pcid_valc_sat_1_reg.regValue = IoReg_Read32(PCID_PCID_VALC_SAT_1_reg);

	//pcid_pcid_valc_ctrl_reg.valc_en = pVALCprotectSetting->valc_en;
	pcid_pcid_valc_ctrl_reg.valc_hpf_clip = pVALCprotectSetting->hpf_clip;

	pcid_pcid_valc_hpf_1_reg.valc_hpf_w0  = pVALCprotectSetting->hpf_weight[0];
	pcid_pcid_valc_hpf_1_reg.valc_hpf_w1  = pVALCprotectSetting->hpf_weight[1];
	pcid_pcid_valc_hpf_1_reg.valc_hpf_w2  = pVALCprotectSetting->hpf_weight[2];
	pcid_pcid_valc_hpf_1_reg.valc_hpf_w3  = pVALCprotectSetting->hpf_weight[3];
	pcid_pcid_valc_hpf_1_reg.valc_hpf_w4  = pVALCprotectSetting->hpf_weight[4];
	pcid_pcid_valc_hpf_1_reg.valc_hpf_w5  = pVALCprotectSetting->hpf_weight[5];
	pcid_pcid_valc_hpf_1_reg.valc_hpf_w6  = pVALCprotectSetting->hpf_weight[6];
	pcid_pcid_valc_hpf_1_reg.valc_hpf_w7  = pVALCprotectSetting->hpf_weight[7];

	pcid_pcid_valc_hpf_0_reg.valc_hpf_w8  = pVALCprotectSetting->hpf_weight[8];
	pcid_pcid_valc_hpf_0_reg.valc_hpf_w9  = pVALCprotectSetting->hpf_weight[9];
	pcid_pcid_valc_hpf_0_reg.valc_hpf_w10 = pVALCprotectSetting->hpf_weight[10];
	pcid_pcid_valc_hpf_0_reg.valc_hpf_w11 = pVALCprotectSetting->hpf_weight[11];
	pcid_pcid_valc_hpf_0_reg.valc_hpf_w12 = pVALCprotectSetting->hpf_weight[12];
	pcid_pcid_valc_hpf_0_reg.valc_hpf_w13 = pVALCprotectSetting->hpf_weight[13];
	pcid_pcid_valc_hpf_0_reg.valc_hpf_w14 = pVALCprotectSetting->hpf_weight[14];
	pcid_pcid_valc_hpf_0_reg.valc_hpf_w15 = pVALCprotectSetting->hpf_weight[15];

	pcid_pcid_valc_sat_1_reg.valc_sat_w0  = pVALCprotectSetting->sat_weight[0];
	pcid_pcid_valc_sat_1_reg.valc_sat_w1  = pVALCprotectSetting->sat_weight[1];
	pcid_pcid_valc_sat_1_reg.valc_sat_w2  = pVALCprotectSetting->sat_weight[2];
	pcid_pcid_valc_sat_1_reg.valc_sat_w3  = pVALCprotectSetting->sat_weight[3];
	pcid_pcid_valc_sat_1_reg.valc_sat_w4  = pVALCprotectSetting->sat_weight[4];
	pcid_pcid_valc_sat_1_reg.valc_sat_w5  = pVALCprotectSetting->sat_weight[5];
	pcid_pcid_valc_sat_1_reg.valc_sat_w6  = pVALCprotectSetting->sat_weight[6];
	pcid_pcid_valc_sat_1_reg.valc_sat_w7  = pVALCprotectSetting->sat_weight[7];

	pcid_pcid_valc_sat_0_reg.valc_sat_w8  = pVALCprotectSetting->sat_weight[8];
	pcid_pcid_valc_sat_0_reg.valc_sat_w9  = pVALCprotectSetting->sat_weight[9];
	pcid_pcid_valc_sat_0_reg.valc_sat_w10 = pVALCprotectSetting->sat_weight[10];
	pcid_pcid_valc_sat_0_reg.valc_sat_w11 = pVALCprotectSetting->sat_weight[11];
	pcid_pcid_valc_sat_0_reg.valc_sat_w12 = pVALCprotectSetting->sat_weight[12];
	pcid_pcid_valc_sat_0_reg.valc_sat_w13 = pVALCprotectSetting->sat_weight[13];
	pcid_pcid_valc_sat_0_reg.valc_sat_w14 = pVALCprotectSetting->sat_weight[14];
	pcid_pcid_valc_sat_0_reg.valc_sat_w15 = pVALCprotectSetting->sat_weight[15];

	IoReg_Write32(PCID_PCID_VALC_CTRL_reg, pcid_pcid_valc_ctrl_reg.regValue);
	IoReg_Write32(PCID_PCID_VALC_HPF_0_reg, pcid_pcid_valc_hpf_0_reg.regValue);
	IoReg_Write32(PCID_PCID_VALC_HPF_1_reg, pcid_pcid_valc_hpf_1_reg.regValue);
	IoReg_Write32(PCID_PCID_VALC_SAT_0_reg, pcid_pcid_valc_sat_0_reg.regValue);
	IoReg_Write32(PCID_PCID_VALC_SAT_1_reg, pcid_pcid_valc_sat_1_reg.regValue);

	if(pVALCprotectSetting->valc_en != 0)
		drvif_color_pcid_VALC_enable(1);
	else
		drvif_color_pcid_VALC_enable(0);
}*/


void drvif_color_set_pcid_regional_weight_setting(DRV_pcid_valc_t* pRgnWeight)
{
	pcid_pcid_regionalweight_ctrl_RBUS pcid_pcid_regionalweight_ctrl_reg;
	pcid_pcid_regionalweight_hor_0_RBUS pcid_pcid_regionalweight_hor_0_reg;
	pcid_pcid_regionalweight_hor_1_RBUS pcid_pcid_regionalweight_hor_1_reg;
	pcid_pcid_regionalweight_hor_2_RBUS pcid_pcid_regionalweight_hor_2_reg;
	pcid_pcid_regionalweight_hor_3_RBUS pcid_pcid_regionalweight_hor_3_reg;
	pcid_pcid_regionalweight_ver_0_RBUS pcid_pcid_regionalweight_ver_0_reg;
	pcid_pcid_regionalweight_ver_1_RBUS pcid_pcid_regionalweight_ver_1_reg;

	pcid_pcid_regionalweight_ctrl_reg.regValue  = IoReg_Read32(PCID_PCID_RegionalWeight_Ctrl_reg);
	pcid_pcid_regionalweight_hor_0_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Hor_0_reg);
	pcid_pcid_regionalweight_hor_1_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Hor_1_reg);
	pcid_pcid_regionalweight_hor_2_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Hor_2_reg);
	pcid_pcid_regionalweight_hor_3_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Hor_3_reg);
	pcid_pcid_regionalweight_ver_0_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Ver_0_reg);
	pcid_pcid_regionalweight_ver_1_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Ver_1_reg);

	pcid_pcid_regionalweight_ctrl_reg.regional_weight_en = pRgnWeight->RgnWeightEnable;
	pcid_pcid_regionalweight_ctrl_reg.rw_input_size = pRgnWeight->InputSz;
	pcid_pcid_regionalweight_ctrl_reg.rw_shift_sel = pRgnWeight->SftSel;
	pcid_pcid_regionalweight_ctrl_reg.rw_start_x = pRgnWeight->Sta_x;
	pcid_pcid_regionalweight_ctrl_reg.rw_start_y = pRgnWeight->Sta_y;

	pcid_pcid_regionalweight_hor_0_reg.rw_hor_w0 = pRgnWeight->HorWeight[0];
	pcid_pcid_regionalweight_hor_0_reg.rw_hor_w1 = pRgnWeight->HorWeight[1];
	pcid_pcid_regionalweight_hor_0_reg.rw_hor_w2 = pRgnWeight->HorWeight[2];
	pcid_pcid_regionalweight_hor_0_reg.rw_hor_w3 = pRgnWeight->HorWeight[3];
	pcid_pcid_regionalweight_hor_0_reg.rw_hor_w4 = pRgnWeight->HorWeight[4];
	pcid_pcid_regionalweight_hor_0_reg.rw_hor_w5 = pRgnWeight->HorWeight[5];
	pcid_pcid_regionalweight_hor_0_reg.rw_hor_w6 = pRgnWeight->HorWeight[6];
	pcid_pcid_regionalweight_hor_0_reg.rw_hor_w7 = pRgnWeight->HorWeight[7];

	pcid_pcid_regionalweight_hor_1_reg.rw_hor_w8 = pRgnWeight->HorWeight[8];
	pcid_pcid_regionalweight_hor_1_reg.rw_hor_w9 = pRgnWeight->HorWeight[9];
	pcid_pcid_regionalweight_hor_1_reg.rw_hor_w10 = pRgnWeight->HorWeight[10];
	pcid_pcid_regionalweight_hor_1_reg.rw_hor_w11 = pRgnWeight->HorWeight[11];
	pcid_pcid_regionalweight_hor_1_reg.rw_hor_w12 = pRgnWeight->HorWeight[12];
	pcid_pcid_regionalweight_hor_1_reg.rw_hor_w13 = pRgnWeight->HorWeight[13];
	pcid_pcid_regionalweight_hor_1_reg.rw_hor_w14 = pRgnWeight->HorWeight[14];
	pcid_pcid_regionalweight_hor_1_reg.rw_hor_w15 = pRgnWeight->HorWeight[15];

	pcid_pcid_regionalweight_hor_2_reg.rw_hor_w16 = pRgnWeight->HorWeight[16];
	pcid_pcid_regionalweight_hor_2_reg.rw_hor_w17 = pRgnWeight->HorWeight[17];
	pcid_pcid_regionalweight_hor_2_reg.rw_hor_w18 = pRgnWeight->HorWeight[18];
	pcid_pcid_regionalweight_hor_2_reg.rw_hor_w19 = pRgnWeight->HorWeight[19];
	pcid_pcid_regionalweight_hor_2_reg.rw_hor_w20 = pRgnWeight->HorWeight[20];
	pcid_pcid_regionalweight_hor_2_reg.rw_hor_w21 = pRgnWeight->HorWeight[21];
	pcid_pcid_regionalweight_hor_2_reg.rw_hor_w22 = pRgnWeight->HorWeight[22];
	pcid_pcid_regionalweight_hor_2_reg.rw_hor_w23 = pRgnWeight->HorWeight[23];

	pcid_pcid_regionalweight_hor_3_reg.rw_hor_w24 = pRgnWeight->HorWeight[24];

	pcid_pcid_regionalweight_ver_0_reg.rw_ver_w0 = pRgnWeight->VerWeight[0];
	pcid_pcid_regionalweight_ver_0_reg.rw_ver_w1 = pRgnWeight->VerWeight[1];
	pcid_pcid_regionalweight_ver_0_reg.rw_ver_w2 = pRgnWeight->VerWeight[2];
	pcid_pcid_regionalweight_ver_0_reg.rw_ver_w3 = pRgnWeight->VerWeight[3];
	pcid_pcid_regionalweight_ver_0_reg.rw_ver_w4 = pRgnWeight->VerWeight[4];
	pcid_pcid_regionalweight_ver_0_reg.rw_ver_w5 = pRgnWeight->VerWeight[5];
	pcid_pcid_regionalweight_ver_0_reg.rw_ver_w6 = pRgnWeight->VerWeight[6];
	pcid_pcid_regionalweight_ver_0_reg.rw_ver_w7 = pRgnWeight->VerWeight[7];

	pcid_pcid_regionalweight_ver_1_reg.rw_ver_w8 = pRgnWeight->VerWeight[8];
	pcid_pcid_regionalweight_ver_1_reg.rw_ver_w9 = pRgnWeight->VerWeight[9];

	IoReg_Write32(PCID_PCID_RegionalWeight_Ctrl_reg, pcid_pcid_regionalweight_ctrl_reg.regValue);
	IoReg_Write32(PCID_PCID_RegionalWeight_Hor_0_reg, pcid_pcid_regionalweight_hor_0_reg.regValue);
	IoReg_Write32(PCID_PCID_RegionalWeight_Hor_1_reg, pcid_pcid_regionalweight_hor_1_reg.regValue);
	IoReg_Write32(PCID_PCID_RegionalWeight_Hor_2_reg, pcid_pcid_regionalweight_hor_2_reg.regValue);
	IoReg_Write32(PCID_PCID_RegionalWeight_Hor_3_reg, pcid_pcid_regionalweight_hor_3_reg.regValue);
	IoReg_Write32(PCID_PCID_RegionalWeight_Ver_0_reg, pcid_pcid_regionalweight_ver_0_reg.regValue);
	IoReg_Write32(PCID_PCID_RegionalWeight_Ver_1_reg, pcid_pcid_regionalweight_ver_1_reg.regValue);
}

void drvif_color_get_pcid_regional_weight_setting(DRV_pcid_valc_t* pRgnWeight)
{
	pcid_pcid_regionalweight_ctrl_RBUS pcid_pcid_regionalweight_ctrl_reg;
	pcid_pcid_regionalweight_hor_0_RBUS pcid_pcid_regionalweight_hor_0_reg;
	pcid_pcid_regionalweight_hor_1_RBUS pcid_pcid_regionalweight_hor_1_reg;
	pcid_pcid_regionalweight_hor_2_RBUS pcid_pcid_regionalweight_hor_2_reg;
	pcid_pcid_regionalweight_hor_3_RBUS pcid_pcid_regionalweight_hor_3_reg;
	pcid_pcid_regionalweight_ver_0_RBUS pcid_pcid_regionalweight_ver_0_reg;
	pcid_pcid_regionalweight_ver_1_RBUS pcid_pcid_regionalweight_ver_1_reg;

	pcid_pcid_regionalweight_ctrl_reg.regValue  = IoReg_Read32(PCID_PCID_RegionalWeight_Ctrl_reg);
	pcid_pcid_regionalweight_hor_0_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Hor_0_reg);
	pcid_pcid_regionalweight_hor_1_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Hor_1_reg);
	pcid_pcid_regionalweight_hor_2_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Hor_2_reg);
	pcid_pcid_regionalweight_hor_3_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Hor_3_reg);
	pcid_pcid_regionalweight_ver_0_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Ver_0_reg);
	pcid_pcid_regionalweight_ver_1_reg.regValue = IoReg_Read32(PCID_PCID_RegionalWeight_Ver_1_reg);

	pRgnWeight->RgnWeightEnable = pcid_pcid_regionalweight_ctrl_reg.regional_weight_en;
	pRgnWeight->InputSz = pcid_pcid_regionalweight_ctrl_reg.rw_input_size;
	pRgnWeight->SftSel = pcid_pcid_regionalweight_ctrl_reg.rw_shift_sel;
	pRgnWeight->Sta_x = pcid_pcid_regionalweight_ctrl_reg.rw_start_x;
	pRgnWeight->Sta_y = pcid_pcid_regionalweight_ctrl_reg.rw_start_y;

	pRgnWeight->HorWeight[0] = pcid_pcid_regionalweight_hor_0_reg.rw_hor_w0;
	pRgnWeight->HorWeight[1] = pcid_pcid_regionalweight_hor_0_reg.rw_hor_w1;
	pRgnWeight->HorWeight[2] = pcid_pcid_regionalweight_hor_0_reg.rw_hor_w2;
	pRgnWeight->HorWeight[3] = pcid_pcid_regionalweight_hor_0_reg.rw_hor_w3;
	pRgnWeight->HorWeight[4] = pcid_pcid_regionalweight_hor_0_reg.rw_hor_w4;
	pRgnWeight->HorWeight[5] = pcid_pcid_regionalweight_hor_0_reg.rw_hor_w5;
	pRgnWeight->HorWeight[6] = pcid_pcid_regionalweight_hor_0_reg.rw_hor_w6;
	pRgnWeight->HorWeight[7] = pcid_pcid_regionalweight_hor_0_reg.rw_hor_w7;

	pRgnWeight->HorWeight[8] = pcid_pcid_regionalweight_hor_1_reg.rw_hor_w8;
	pRgnWeight->HorWeight[9] = pcid_pcid_regionalweight_hor_1_reg.rw_hor_w9;
	pRgnWeight->HorWeight[10] = pcid_pcid_regionalweight_hor_1_reg.rw_hor_w10;
	pRgnWeight->HorWeight[11] = pcid_pcid_regionalweight_hor_1_reg.rw_hor_w11;
	pRgnWeight->HorWeight[12] = pcid_pcid_regionalweight_hor_1_reg.rw_hor_w12;
	pRgnWeight->HorWeight[13] = pcid_pcid_regionalweight_hor_1_reg.rw_hor_w13;
	pRgnWeight->HorWeight[14] = pcid_pcid_regionalweight_hor_1_reg.rw_hor_w14;
	pRgnWeight->HorWeight[15] = pcid_pcid_regionalweight_hor_1_reg.rw_hor_w15;

	pRgnWeight->HorWeight[16] = pcid_pcid_regionalweight_hor_2_reg.rw_hor_w16;
	pRgnWeight->HorWeight[17] = pcid_pcid_regionalweight_hor_2_reg.rw_hor_w17;
	pRgnWeight->HorWeight[18] = pcid_pcid_regionalweight_hor_2_reg.rw_hor_w18;
	pRgnWeight->HorWeight[19] = pcid_pcid_regionalweight_hor_2_reg.rw_hor_w19;
	pRgnWeight->HorWeight[20] = pcid_pcid_regionalweight_hor_2_reg.rw_hor_w20;
	pRgnWeight->HorWeight[21] = pcid_pcid_regionalweight_hor_2_reg.rw_hor_w21;
	pRgnWeight->HorWeight[22] = pcid_pcid_regionalweight_hor_2_reg.rw_hor_w22;
	pRgnWeight->HorWeight[23] = pcid_pcid_regionalweight_hor_2_reg.rw_hor_w23;

	pRgnWeight->HorWeight[24] = pcid_pcid_regionalweight_hor_3_reg.rw_hor_w24;

	pRgnWeight->VerWeight[0] = pcid_pcid_regionalweight_ver_0_reg.rw_ver_w0;
	pRgnWeight->VerWeight[1] = pcid_pcid_regionalweight_ver_0_reg.rw_ver_w1;
	pRgnWeight->VerWeight[2] = pcid_pcid_regionalweight_ver_0_reg.rw_ver_w2;
	pRgnWeight->VerWeight[3] = pcid_pcid_regionalweight_ver_0_reg.rw_ver_w3;
	pRgnWeight->VerWeight[4] = pcid_pcid_regionalweight_ver_0_reg.rw_ver_w4;
	pRgnWeight->VerWeight[5] = pcid_pcid_regionalweight_ver_0_reg.rw_ver_w5;
	pRgnWeight->VerWeight[6] = pcid_pcid_regionalweight_ver_0_reg.rw_ver_w6;
	pRgnWeight->VerWeight[7] = pcid_pcid_regionalweight_ver_0_reg.rw_ver_w7;

	pRgnWeight->VerWeight[8] = pcid_pcid_regionalweight_ver_1_reg.rw_ver_w8;
	pRgnWeight->VerWeight[9] = pcid_pcid_regionalweight_ver_1_reg.rw_ver_w9;
}


void drvif_color_pcid_TblTrans_Target2Delta(unsigned char *TargetTbl)
{
	char DeltaTbl[PCID_TABLE_LEN] = {0};
	int idx = 0;

	// Trans the table from target value to delta value
	for(idx=0; idx<PCID_TABLE_LEN; idx++)
	{
		if( ( ((idx%17)*16) == TargetTbl[idx] ) ||
			( ((idx%17)==16) && (TargetTbl[idx] == 255) ) )
		{
			DeltaTbl[idx] = 0;
		}
		else
		{
			int InputVal = ((idx%17)*256)>=4095? 4095 : ((idx%17)*256); // 12 bit input value
			int DeltaVal = (TargetTbl[idx]*16) - InputVal; // 12 bit delta
			DeltaVal = (DeltaVal * 16) / (15*15);
			DeltaVal = DeltaVal>=127? 127 : ( DeltaVal<=-128? -128 : DeltaVal );
			DeltaTbl[idx] = DeltaVal;
		}
	}

	memcpy(TargetTbl, DeltaTbl, sizeof(char)*PCID_TABLE_LEN);
}

void drvif_color_get_pcid_valuetable(unsigned int *pRetTbl, unsigned int TblSel, DRV_pcid2_channel_t Channel)
{

	int i=0;
	pcid_pcid_lut_addr_RBUS pcid_lut_addr_reg;
	pcid_pcid_lut_data_RBUS pcid_pcid_lut_data_reg;

	// Setup table access setting
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_sel = Channel;
	pcid_lut_addr_reg.pcid_lut_adr_mode = 0;	/* Increase Current (column) Index first */
	pcid_lut_addr_reg.pcid_lut_ax_en = 1;		/* access en */
	pcid_lut_addr_reg.pcid_lut_row_addr = TblSel;
	pcid_lut_addr_reg.pcid_lut_column_addr = TblSel;

	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	// Write the value into table
	for(i=0; i<81; i++)
	{
		pcid_pcid_lut_data_reg.regValue = IoReg_Read32(PCID_PCID_LUT_DATA_reg);
		pRetTbl[i] = pcid_pcid_lut_data_reg.regValue;
		VIPprintf("[VPQ][PCID]Table[%d]=0x%x\n",i,pRetTbl[i]);
	}

	// Disable the table access
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_ax_en = 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

}


