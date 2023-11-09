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
#include <tvscalercontrol/vip/valc.h>
#include <tvscalercontrol/vip/pcid.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/vip/vip_reg_def.h>

#include <rbus/valc_reg.h>


/*================================ Global Variables ==============================*/
unsigned int valcTbl[3][2][PCID_VALC_TBL_LEN] = {{{0}}};

/*================================ Definitions ==============================*/
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_PCID_DEBUG,fmt,##args)

/*================================== Function ===============================*/

void drvif_color_set_valc_enable(unsigned char bEnable)
{
	// put the block at the front position
	unsigned long flags;//for spin_lock_irqsave
	valc_pcid_valc_ctrl_2_RBUS valc_pcid_valc_ctrl_reg;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;

	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	if( bEnable != 0 )
	{
#ifdef For_BringUp_Disable
		display_timing_ctrl2_reg.pcid_en = 0;
		display_timing_ctrl2_reg.pcid2_en = 0;
#else
		display_timing_ctrl2_reg.pcid_en = bEnable;
		display_timing_ctrl2_reg.pcid2_en = 0;
#endif

		IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

		down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
		spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
		IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, PPOVERLAY_Double_Buffer_CTRL_dreg_dbuf_set_mask); // Set double buffer apply bit
		spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
		up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	}

	valc_pcid_valc_ctrl_reg.regValue = IoReg_Read32(VALC_PCID_VALC_CTRL_reg);
	valc_pcid_valc_ctrl_reg.valc_en= bEnable;
	IoReg_Write32(VALC_PCID_VALC_CTRL_reg, valc_pcid_valc_ctrl_reg.regValue);

}

/*void drvif_color_set_valc_SpatialPattern(unsigned char PatternSel)// Mac8p no spatial pattern
{
	valc_pcid_valc_ctrl_RBUS valc_pcid_valc_ctrl_reg;
	valc_pcid_valc_ctrl_reg.regValue = IoReg_Read32(VALC_PCID_VALC_CTRL_reg);
	valc_pcid_valc_ctrl_reg.valc_spatial_sel = PatternSel;
	IoReg_Write32(VALC_PCID_VALC_CTRL_reg, valc_pcid_valc_ctrl_reg.regValue);
}*/

void drvif_color_set_pcid_valc_control_setting(DRV_pcid2_data_t_val* pPcid2Setting)
{
	valc_pcid_valc_ctrl_RBUS pcid_ctrl_reg;
	
	pcid_ctrl_reg.regValue = IoReg_Read32(VALC_PCID_VALC_CTRL_reg);

	pcid_ctrl_reg.boundary_mode = pPcid2Setting->boundary_mode;
	pcid_ctrl_reg.intp_bit_sel = pPcid2Setting->intp_bit_sel;
	pcid_ctrl_reg.first_line_tab = pPcid2Setting->first_line_tab;
		
	IoReg_Write32(VALC_PCID_VALC_CTRL_reg  , pcid_ctrl_reg.regValue);
	
}

void drvif_color_set_pcid_valc_pixel_setting(DRV_pcid2_data_t_val* pPcid2Setting)
{
	valc_pcid_valc_ctrl_RBUS pcid_ctrl_reg;
	valc_pcid_valc_line_1_RBUS pcid_line1_reg;
	valc_pcid_valc_line_2_RBUS pcid_line2_reg;
	valc_pcid_valc_line_3_RBUS pcid_line3_reg;
	valc_pcid_valc_line_4_RBUS pcid_line4_reg;

	pcid_ctrl_reg.regValue = IoReg_Read32(VALC_PCID_VALC_CTRL_reg);
	pcid_line1_reg.regValue = IoReg_Read32(VALC_PCID_VALC_LINE_1_reg);
	pcid_line2_reg.regValue = IoReg_Read32(VALC_PCID_VALC_LINE_2_reg);
	pcid_line3_reg.regValue = IoReg_Read32(VALC_PCID_VALC_LINE_3_reg);
	pcid_line4_reg.regValue = IoReg_Read32(VALC_PCID_VALC_LINE_4_reg);

	/* Enable control */
	pcid_ctrl_reg.boundary_mode = pPcid2Setting->boundary_mode;
	pcid_ctrl_reg.intp_bit_sel = pPcid2Setting->intp_bit_sel;
	pcid_ctrl_reg.first_line_tab = pPcid2Setting->first_line_tab;

	/* LINE 1 EVEN PIXEL */ 
	pcid_line1_reg.line1_even_r_line_select  = pPcid2Setting->pixel_ref.line1.even_r.line_sel;
	pcid_line1_reg.line1_even_r_point_select = pPcid2Setting->pixel_ref.line1.even_r.point_sel;
	pcid_line1_reg.line1_even_r_color_select = pPcid2Setting->pixel_ref.line1.even_r.color_sel;
	pcid_line1_reg.line1_even_g_line_select  = pPcid2Setting->pixel_ref.line1.even_g.line_sel;
	pcid_line1_reg.line1_even_g_point_select = pPcid2Setting->pixel_ref.line1.even_g.point_sel;
	pcid_line1_reg.line1_even_g_color_select = pPcid2Setting->pixel_ref.line1.even_g.color_sel;
	pcid_line1_reg.line1_even_b_line_select  = pPcid2Setting->pixel_ref.line1.even_b.line_sel;
	pcid_line1_reg.line1_even_b_point_select = pPcid2Setting->pixel_ref.line1.even_b.point_sel;
	pcid_line1_reg.line1_even_b_color_select = pPcid2Setting->pixel_ref.line1.even_b.color_sel;

	/* LINE 1 ODD PIXEL */
	pcid_line1_reg.line1_odd_r_line_select  = pPcid2Setting->pixel_ref.line1.odd_r.line_sel;
	pcid_line1_reg.line1_odd_r_point_select = pPcid2Setting->pixel_ref.line1.odd_r.point_sel;
	pcid_line1_reg.line1_odd_r_color_select = pPcid2Setting->pixel_ref.line1.odd_r.color_sel;
	pcid_line1_reg.line1_odd_g_line_select  = pPcid2Setting->pixel_ref.line1.odd_g.line_sel;
	pcid_line1_reg.line1_odd_g_point_select = pPcid2Setting->pixel_ref.line1.odd_g.point_sel;
	pcid_line1_reg.line1_odd_g_color_select = pPcid2Setting->pixel_ref.line1.odd_g.color_sel;
	pcid_line1_reg.line1_odd_b_line_select  = pPcid2Setting->pixel_ref.line1.odd_b.line_sel;
	pcid_line1_reg.line1_odd_b_point_select = pPcid2Setting->pixel_ref.line1.odd_b.point_sel;
	pcid_line1_reg.line1_odd_b_color_select = pPcid2Setting->pixel_ref.line1.odd_b.color_sel;

	/* LINE 2 EVEN PIXEL */
	pcid_line2_reg.line2_even_r_line_select  = pPcid2Setting->pixel_ref.line2.even_r.line_sel;
	pcid_line2_reg.line2_even_r_point_select = pPcid2Setting->pixel_ref.line2.even_r.point_sel;
	pcid_line2_reg.line2_even_r_color_select = pPcid2Setting->pixel_ref.line2.even_r.color_sel;
	pcid_line2_reg.line2_even_g_line_select  = pPcid2Setting->pixel_ref.line2.even_g.line_sel;
	pcid_line2_reg.line2_even_g_point_select = pPcid2Setting->pixel_ref.line2.even_g.point_sel;
	pcid_line2_reg.line2_even_g_color_select = pPcid2Setting->pixel_ref.line2.even_g.color_sel;
	pcid_line2_reg.line2_even_b_line_select  = pPcid2Setting->pixel_ref.line2.even_b.line_sel;
	pcid_line2_reg.line2_even_b_point_select = pPcid2Setting->pixel_ref.line2.even_b.point_sel;
	pcid_line2_reg.line2_even_b_color_select = pPcid2Setting->pixel_ref.line2.even_b.color_sel;

	/* LINE 2 ODD PIXEL */
	pcid_line2_reg.line2_odd_r_line_select  = pPcid2Setting->pixel_ref.line2.odd_r.line_sel;
	pcid_line2_reg.line2_odd_r_point_select = pPcid2Setting->pixel_ref.line2.odd_r.point_sel;
	pcid_line2_reg.line2_odd_r_color_select = pPcid2Setting->pixel_ref.line2.odd_r.color_sel;
	pcid_line2_reg.line2_odd_g_line_select  = pPcid2Setting->pixel_ref.line2.odd_g.line_sel;
	pcid_line2_reg.line2_odd_g_point_select = pPcid2Setting->pixel_ref.line2.odd_g.point_sel;
	pcid_line2_reg.line2_odd_g_color_select = pPcid2Setting->pixel_ref.line2.odd_g.color_sel;
	pcid_line2_reg.line2_odd_b_line_select  = pPcid2Setting->pixel_ref.line2.odd_b.line_sel;
	pcid_line2_reg.line2_odd_b_point_select = pPcid2Setting->pixel_ref.line2.odd_b.point_sel;
	pcid_line2_reg.line2_odd_b_color_select = pPcid2Setting->pixel_ref.line2.odd_b.color_sel;

	/* LINE 3 EVEN PIXEL */
	pcid_line3_reg.line3_even_r_line_select  = pPcid2Setting->pixel_ref.line3.even_r.line_sel;
	pcid_line3_reg.line3_even_r_point_select = pPcid2Setting->pixel_ref.line3.even_r.point_sel;
	pcid_line3_reg.line3_even_r_color_select = pPcid2Setting->pixel_ref.line3.even_r.color_sel;
	pcid_line3_reg.line3_even_g_line_select  = pPcid2Setting->pixel_ref.line3.even_g.line_sel;
	pcid_line3_reg.line3_even_g_point_select = pPcid2Setting->pixel_ref.line3.even_g.point_sel;
	pcid_line3_reg.line3_even_g_color_select = pPcid2Setting->pixel_ref.line3.even_g.color_sel;
	pcid_line3_reg.line3_even_b_line_select  = pPcid2Setting->pixel_ref.line3.even_b.line_sel;
	pcid_line3_reg.line3_even_b_point_select = pPcid2Setting->pixel_ref.line3.even_b.point_sel;
	pcid_line3_reg.line3_even_b_color_select = pPcid2Setting->pixel_ref.line3.even_b.color_sel;

	/* LINE 3 ODD PIXEL */
	pcid_line3_reg.line3_odd_r_line_select  = pPcid2Setting->pixel_ref.line3.odd_r.line_sel;
	pcid_line3_reg.line3_odd_r_point_select = pPcid2Setting->pixel_ref.line3.odd_r.point_sel;
	pcid_line3_reg.line3_odd_r_color_select = pPcid2Setting->pixel_ref.line3.odd_r.color_sel;
	pcid_line3_reg.line3_odd_g_line_select  = pPcid2Setting->pixel_ref.line3.odd_g.line_sel;
	pcid_line3_reg.line3_odd_g_point_select = pPcid2Setting->pixel_ref.line3.odd_g.point_sel;
	pcid_line3_reg.line3_odd_g_color_select = pPcid2Setting->pixel_ref.line3.odd_g.color_sel;
	pcid_line3_reg.line3_odd_b_line_select  = pPcid2Setting->pixel_ref.line3.odd_b.line_sel;
	pcid_line3_reg.line3_odd_b_point_select = pPcid2Setting->pixel_ref.line3.odd_b.point_sel;
	pcid_line3_reg.line3_odd_b_color_select = pPcid2Setting->pixel_ref.line3.odd_b.color_sel;

	/* LINE 4 EVEN PIXEL */
	pcid_line4_reg.line4_even_r_line_select  = pPcid2Setting->pixel_ref.line4.even_r.line_sel;
	pcid_line4_reg.line4_even_r_point_select = pPcid2Setting->pixel_ref.line4.even_r.point_sel;
	pcid_line4_reg.line4_even_r_color_select = pPcid2Setting->pixel_ref.line4.even_r.color_sel;
	pcid_line4_reg.line4_even_g_line_select  = pPcid2Setting->pixel_ref.line4.even_g.line_sel;
	pcid_line4_reg.line4_even_g_point_select = pPcid2Setting->pixel_ref.line4.even_g.point_sel;
	pcid_line4_reg.line4_even_g_color_select = pPcid2Setting->pixel_ref.line4.even_g.color_sel;
	pcid_line4_reg.line4_even_b_line_select  = pPcid2Setting->pixel_ref.line4.even_b.line_sel;
	pcid_line4_reg.line4_even_b_point_select = pPcid2Setting->pixel_ref.line4.even_b.point_sel;
	pcid_line4_reg.line4_even_b_color_select = pPcid2Setting->pixel_ref.line4.even_b.color_sel;

	/* LINE 4 ODD PIXEL */
	pcid_line4_reg.line4_odd_r_line_select  = pPcid2Setting->pixel_ref.line4.odd_r.line_sel;
	pcid_line4_reg.line4_odd_r_point_select = pPcid2Setting->pixel_ref.line4.odd_r.point_sel;
	pcid_line4_reg.line4_odd_r_color_select = pPcid2Setting->pixel_ref.line4.odd_r.color_sel;
	pcid_line4_reg.line4_odd_g_line_select  = pPcid2Setting->pixel_ref.line4.odd_g.line_sel;
	pcid_line4_reg.line4_odd_g_point_select = pPcid2Setting->pixel_ref.line4.odd_g.point_sel;
	pcid_line4_reg.line4_odd_g_color_select = pPcid2Setting->pixel_ref.line4.odd_g.color_sel;
	pcid_line4_reg.line4_odd_b_line_select  = pPcid2Setting->pixel_ref.line4.odd_b.line_sel;
	pcid_line4_reg.line4_odd_b_point_select = pPcid2Setting->pixel_ref.line4.odd_b.point_sel;
	pcid_line4_reg.line4_odd_b_color_select = pPcid2Setting->pixel_ref.line4.odd_b.color_sel;

	IoReg_Write32(VALC_PCID_VALC_CTRL_reg  , pcid_ctrl_reg.regValue);
	IoReg_Write32(VALC_PCID_VALC_LINE_1_reg, pcid_line1_reg.regValue);
	IoReg_Write32(VALC_PCID_VALC_LINE_2_reg, pcid_line2_reg.regValue);
	IoReg_Write32(VALC_PCID_VALC_LINE_3_reg, pcid_line3_reg.regValue);
	IoReg_Write32(VALC_PCID_VALC_LINE_4_reg, pcid_line4_reg.regValue);
}

void drvif_color_set_pcid_valc_poltable(unsigned int * pPcid2PolTable, DRV_pcid2_channel_t color)
{
	switch(color)
	{
		case _PCID2_COLOR_R:
		{
			valc_pcid2_valc_polarity_r_1_RBUS valc_pcid2_valc_polarity_r_1_reg;
			valc_pcid2_valc_polarity_r_2_RBUS valc_pcid2_valc_polarity_r_2_reg;
			valc_pcid2_valc_polarity_r_3_RBUS valc_pcid2_valc_polarity_r_3_reg;
			valc_pcid2_valc_polarity_r_4_RBUS valc_pcid2_valc_polarity_r_4_reg;

			valc_pcid2_valc_polarity_r_1_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_R_1_reg);
			valc_pcid2_valc_polarity_r_2_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_R_2_reg);
			valc_pcid2_valc_polarity_r_3_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_R_3_reg);
			valc_pcid2_valc_polarity_r_3_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_R_4_reg);

			valc_pcid2_valc_polarity_r_1_reg.r_po_line1 = pPcid2PolTable[0];
			valc_pcid2_valc_polarity_r_1_reg.r_po_line2 = pPcid2PolTable[1];
			valc_pcid2_valc_polarity_r_2_reg.r_po_line3 = pPcid2PolTable[2];
			valc_pcid2_valc_polarity_r_2_reg.r_po_line4 = pPcid2PolTable[3];
			valc_pcid2_valc_polarity_r_3_reg.r_po_line5 = pPcid2PolTable[4];
			valc_pcid2_valc_polarity_r_3_reg.r_po_line6 = pPcid2PolTable[5];
			valc_pcid2_valc_polarity_r_4_reg.r_po_line7 = pPcid2PolTable[6];
			valc_pcid2_valc_polarity_r_4_reg.r_po_line8 = pPcid2PolTable[7];

			IoReg_Write32(VALC_PCID2_VALC_POLARITY_R_1_reg, valc_pcid2_valc_polarity_r_1_reg.regValue);
			IoReg_Write32(VALC_PCID2_VALC_POLARITY_R_2_reg, valc_pcid2_valc_polarity_r_2_reg.regValue);
			IoReg_Write32(VALC_PCID2_VALC_POLARITY_R_3_reg, valc_pcid2_valc_polarity_r_3_reg.regValue);
			IoReg_Write32(VALC_PCID2_VALC_POLARITY_R_4_reg, valc_pcid2_valc_polarity_r_4_reg.regValue);
			break;
		}
		case _PCID2_COLOR_G:
		{
			valc_pcid2_valc_polarity_g_1_RBUS valc_pcid2_valc_polarity_g_1_reg;
			valc_pcid2_valc_polarity_g_2_RBUS valc_pcid2_valc_polarity_g_2_reg;
			valc_pcid2_valc_polarity_g_3_RBUS valc_pcid2_valc_polarity_g_3_reg;
			valc_pcid2_valc_polarity_g_4_RBUS valc_pcid2_valc_polarity_g_4_reg;

			valc_pcid2_valc_polarity_g_1_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_G_1_reg);
			valc_pcid2_valc_polarity_g_2_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_G_2_reg);
			valc_pcid2_valc_polarity_g_3_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_G_3_reg);
			valc_pcid2_valc_polarity_g_3_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_G_4_reg);

			valc_pcid2_valc_polarity_g_1_reg.g_po_line1 = pPcid2PolTable[0];
			valc_pcid2_valc_polarity_g_1_reg.g_po_line2 = pPcid2PolTable[1];
			valc_pcid2_valc_polarity_g_2_reg.g_po_line3 = pPcid2PolTable[2];
			valc_pcid2_valc_polarity_g_2_reg.g_po_line4 = pPcid2PolTable[3];
			valc_pcid2_valc_polarity_g_3_reg.g_po_line5 = pPcid2PolTable[4];
			valc_pcid2_valc_polarity_g_3_reg.g_po_line6 = pPcid2PolTable[5];
			valc_pcid2_valc_polarity_g_4_reg.g_po_line7 = pPcid2PolTable[6];
			valc_pcid2_valc_polarity_g_4_reg.g_po_line8 = pPcid2PolTable[7];

			IoReg_Write32(VALC_PCID2_VALC_POLARITY_G_1_reg, valc_pcid2_valc_polarity_g_1_reg.regValue);
			IoReg_Write32(VALC_PCID2_VALC_POLARITY_G_2_reg, valc_pcid2_valc_polarity_g_2_reg.regValue);
			IoReg_Write32(VALC_PCID2_VALC_POLARITY_G_3_reg, valc_pcid2_valc_polarity_g_3_reg.regValue);
			IoReg_Write32(VALC_PCID2_VALC_POLARITY_G_4_reg, valc_pcid2_valc_polarity_g_4_reg.regValue);
			break;
		}
		case _PCID2_COLOR_B:
		{
			valc_pcid2_valc_polarity_b_1_RBUS valc_pcid2_valc_polarity_b_1_reg;
			valc_pcid2_valc_polarity_b_2_RBUS valc_pcid2_valc_polarity_b_2_reg;
			valc_pcid2_valc_polarity_b_3_RBUS valc_pcid2_valc_polarity_b_3_reg;
			valc_pcid2_valc_polarity_b_4_RBUS valc_pcid2_valc_polarity_b_4_reg;

			valc_pcid2_valc_polarity_b_1_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_B_1_reg);
			valc_pcid2_valc_polarity_b_2_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_B_2_reg);
			valc_pcid2_valc_polarity_b_3_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_B_3_reg);
			valc_pcid2_valc_polarity_b_3_reg.regValue = IoReg_Read32(VALC_PCID2_VALC_POLARITY_B_4_reg);

			valc_pcid2_valc_polarity_b_1_reg.b_po_line1 = pPcid2PolTable[0];
			valc_pcid2_valc_polarity_b_1_reg.b_po_line2 = pPcid2PolTable[1];
			valc_pcid2_valc_polarity_b_2_reg.b_po_line3 = pPcid2PolTable[2];
			valc_pcid2_valc_polarity_b_2_reg.b_po_line4 = pPcid2PolTable[3];
			valc_pcid2_valc_polarity_b_3_reg.b_po_line5 = pPcid2PolTable[4];
			valc_pcid2_valc_polarity_b_3_reg.b_po_line6 = pPcid2PolTable[5];
			valc_pcid2_valc_polarity_b_4_reg.b_po_line7 = pPcid2PolTable[6];
			valc_pcid2_valc_polarity_b_4_reg.b_po_line8 = pPcid2PolTable[7];

			IoReg_Write32(VALC_PCID2_VALC_POLARITY_B_1_reg, valc_pcid2_valc_polarity_b_1_reg.regValue);
			IoReg_Write32(VALC_PCID2_VALC_POLARITY_B_2_reg, valc_pcid2_valc_polarity_b_2_reg.regValue);
			IoReg_Write32(VALC_PCID2_VALC_POLARITY_B_3_reg, valc_pcid2_valc_polarity_b_3_reg.regValue);
			IoReg_Write32(VALC_PCID2_VALC_POLARITY_B_4_reg, valc_pcid2_valc_polarity_b_4_reg.regValue);
			break;
		}
		default:
			break;
	}
}

void drvif_color_set_valc_HPF_protect(DRV_valc_hpf_ctrl_t* pHPFCtrl)
{
	valc_pcid_valc_ctrl_2_RBUS valc_pcid_valc_ctrl_reg;
	valc_pcid_valc_hpf_0_RBUS valc_pcid_valc_hpf_0_reg;
	valc_pcid_valc_hpf_1_RBUS valc_pcid_valc_hpf_1_reg;

	valc_pcid_valc_ctrl_reg.regValue = IoReg_Read32(VALC_PCID_VALC_CTRL_reg);
	valc_pcid_valc_hpf_0_reg.regValue = IoReg_Read32(VALC_PCID_VALC_HPF_0_reg);
	valc_pcid_valc_hpf_1_reg.regValue = IoReg_Read32(VALC_PCID_VALC_HPF_1_reg);

	valc_pcid_valc_ctrl_reg.valc_hpf_clip = pHPFCtrl->ClipSel;

	valc_pcid_valc_hpf_1_reg.valc_hpf_w0  = pHPFCtrl->Weight[0];
	valc_pcid_valc_hpf_1_reg.valc_hpf_w1  = pHPFCtrl->Weight[1];
	valc_pcid_valc_hpf_1_reg.valc_hpf_w2  = pHPFCtrl->Weight[2];
	valc_pcid_valc_hpf_1_reg.valc_hpf_w3  = pHPFCtrl->Weight[3];
	valc_pcid_valc_hpf_1_reg.valc_hpf_w4  = pHPFCtrl->Weight[4];
	valc_pcid_valc_hpf_1_reg.valc_hpf_w5  = pHPFCtrl->Weight[5];
	valc_pcid_valc_hpf_1_reg.valc_hpf_w6  = pHPFCtrl->Weight[6];
	valc_pcid_valc_hpf_1_reg.valc_hpf_w7  = pHPFCtrl->Weight[7];

	valc_pcid_valc_hpf_0_reg.valc_hpf_w8  = pHPFCtrl->Weight[8];
	valc_pcid_valc_hpf_0_reg.valc_hpf_w9  = pHPFCtrl->Weight[9];
	valc_pcid_valc_hpf_0_reg.valc_hpf_w10 = pHPFCtrl->Weight[10];
	valc_pcid_valc_hpf_0_reg.valc_hpf_w11 = pHPFCtrl->Weight[11];
	valc_pcid_valc_hpf_0_reg.valc_hpf_w12 = pHPFCtrl->Weight[12];
	valc_pcid_valc_hpf_0_reg.valc_hpf_w13 = pHPFCtrl->Weight[13];
	valc_pcid_valc_hpf_0_reg.valc_hpf_w14 = pHPFCtrl->Weight[14];
	valc_pcid_valc_hpf_0_reg.valc_hpf_w15 = pHPFCtrl->Weight[15];

	IoReg_Write32(VALC_PCID_VALC_CTRL_reg, valc_pcid_valc_ctrl_reg.regValue);
	IoReg_Write32(VALC_PCID_VALC_HPF_0_reg, valc_pcid_valc_hpf_0_reg.regValue);
	IoReg_Write32(VALC_PCID_VALC_HPF_1_reg, valc_pcid_valc_hpf_1_reg.regValue);
}

void drvif_color_set_valc_SAT_protect(DRV_valc_sat_ctrl_t* pSATCtrl)
{
	valc_pcid_valc_sat_0_RBUS valc_pcid_valc_sat_0_reg;
	valc_pcid_valc_sat_1_RBUS valc_pcid_valc_sat_1_reg;

	valc_pcid_valc_sat_0_reg.regValue = IoReg_Read32(VALC_PCID_VALC_SAT_0_reg);
	valc_pcid_valc_sat_1_reg.regValue = IoReg_Read32(VALC_PCID_VALC_SAT_1_reg);

	valc_pcid_valc_sat_1_reg.valc_sat_w0  = pSATCtrl->Weight[0];
	valc_pcid_valc_sat_1_reg.valc_sat_w1  = pSATCtrl->Weight[1];
	valc_pcid_valc_sat_1_reg.valc_sat_w2  = pSATCtrl->Weight[2];
	valc_pcid_valc_sat_1_reg.valc_sat_w3  = pSATCtrl->Weight[3];
	valc_pcid_valc_sat_1_reg.valc_sat_w4  = pSATCtrl->Weight[4];
	valc_pcid_valc_sat_1_reg.valc_sat_w5  = pSATCtrl->Weight[5];
	valc_pcid_valc_sat_1_reg.valc_sat_w6  = pSATCtrl->Weight[6];
	valc_pcid_valc_sat_1_reg.valc_sat_w7  = pSATCtrl->Weight[7];

	valc_pcid_valc_sat_0_reg.valc_sat_w8  = pSATCtrl->Weight[8];
	valc_pcid_valc_sat_0_reg.valc_sat_w9  = pSATCtrl->Weight[9];
	valc_pcid_valc_sat_0_reg.valc_sat_w10 = pSATCtrl->Weight[10];
	valc_pcid_valc_sat_0_reg.valc_sat_w11 = pSATCtrl->Weight[11];
	valc_pcid_valc_sat_0_reg.valc_sat_w12 = pSATCtrl->Weight[12];
	valc_pcid_valc_sat_0_reg.valc_sat_w13 = pSATCtrl->Weight[13];
	valc_pcid_valc_sat_0_reg.valc_sat_w14 = pSATCtrl->Weight[14];
	valc_pcid_valc_sat_0_reg.valc_sat_w15 = pSATCtrl->Weight[15];

	IoReg_Write32(VALC_PCID_VALC_SAT_0_reg, valc_pcid_valc_sat_0_reg.regValue);
	IoReg_Write32(VALC_PCID_VALC_SAT_1_reg, valc_pcid_valc_sat_1_reg.regValue);
}

void drvif_color_set_valc_table(unsigned int* pTbl, unsigned int TblSel,DRV_valc_channel_t ChannelSel)
{
	int i=0;
	valc_pcid_valc_lut_addr_RBUS valc_valc_lut_addr_reg;
	valc_pcid_valc_lut_data_RBUS valc_valc_lut_data_reg;

	if(!pTbl)
	{
		//VIPprintf("[Error][%s][%s] Table Ptr is null!\n", __FILE__, __func__);
		return;
	}

	// setup table access setting
	valc_valc_lut_addr_reg.regValue = IoReg_Read32(VALC_PCID_VALC_LUT_ADDR_reg);
	valc_valc_lut_addr_reg.pcid_lut_sel = ChannelSel;
	valc_valc_lut_addr_reg.pcid_lut_adr_mode = 0;
	valc_valc_lut_addr_reg.pcid_lut_ax_en = 1;
	valc_valc_lut_addr_reg.pcid_lut_row_addr = TblSel;
	valc_valc_lut_addr_reg.pcid_lut_column_addr = TblSel;

	IoReg_Write32(VALC_PCID_VALC_LUT_ADDR_reg, valc_valc_lut_addr_reg.regValue);

	for(i=0; i<VALC_TBL_LEN; i++)
	{
		valc_valc_lut_data_reg.pcid_lut_data = pTbl[i];
		IoReg_Write32(VALC_PCID_VALC_LUT_DATA_reg, valc_valc_lut_data_reg.regValue);
		//VIPprintf("[VPQ][VALC][W]Table[%d]=0x%x\n", i, pTbl[i]);
	}

	// Disable the table access
	valc_valc_lut_addr_reg.regValue = IoReg_Read32(VALC_PCID_VALC_LUT_ADDR_reg);
	valc_valc_lut_addr_reg.pcid_lut_ax_en = 0;
	IoReg_Write32(VALC_PCID_VALC_LUT_ADDR_reg, valc_valc_lut_addr_reg.regValue);

}

void drvif_color_get_valc_table(unsigned int* pTbl, unsigned int TblSel,DRV_valc_channel_t ChannelSel)
{
	int i=0;
	valc_pcid_valc_lut_addr_RBUS valc_valc_lut_addr_reg;
	valc_pcid_valc_lut_data_RBUS valc_valc_lut_data_reg;

	if(!pTbl)
	{
		//VIPprintf("[Error][%s][%s] Table Ptr is null!\n", __FILE__, __func__);
		return;
	}

	// setup table access setting
	valc_valc_lut_addr_reg.regValue = IoReg_Read32(VALC_PCID_VALC_LUT_ADDR_reg);
	valc_valc_lut_addr_reg.pcid_lut_sel = ChannelSel;
	valc_valc_lut_addr_reg.pcid_lut_adr_mode = 0;
	valc_valc_lut_addr_reg.pcid_lut_ax_en = 1;
	valc_valc_lut_addr_reg.pcid_lut_row_addr = 0;
	valc_valc_lut_addr_reg.pcid_lut_column_addr = TblSel;

	IoReg_Write32(VALC_PCID_VALC_LUT_ADDR_reg, valc_valc_lut_addr_reg.regValue);

	for(i=0; i<VALC_TBL_LEN; i++)
	{
		valc_valc_lut_data_reg.regValue = IoReg_Read32(VALC_PCID_VALC_LUT_DATA_reg);
		pTbl[i] = valc_valc_lut_data_reg.pcid_lut_data;
		//VIPprintf("[VPQ][VALC][R]Table[%d]=0x%x\n", i, pTbl[i]);
	}

	// Disable the table access
	valc_valc_lut_addr_reg.regValue = IoReg_Read32(VALC_PCID_VALC_LUT_ADDR_reg);
	valc_valc_lut_addr_reg.pcid_lut_ax_en = 0;
	IoReg_Write32(VALC_PCID_VALC_LUT_ADDR_reg, valc_valc_lut_addr_reg.regValue);
}

unsigned char driver_color_get_valc_enable(void)
{
	valc_pcid_valc_ctrl_2_RBUS valc_pcid_valc_ctrl_reg;
	valc_pcid_valc_ctrl_reg.regValue = IoReg_Read32(VALC_PCID_VALC_CTRL_2_reg);
	if( valc_pcid_valc_ctrl_reg.valc_en!= 0 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

