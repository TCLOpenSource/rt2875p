/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for de-interlace IP control functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version 	$Revision$
 * @ingroup 	ip
 */

 /**
 * @addtogroup ip
 * @{
 */

/*================ Module dependency  =============== */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/semaphore.h>

#include <rtd_log/rtd_module_log.h>
//#include "rtd_regs.h"
//#include "rtd_macro.h"
#include <rbus/di_reg.h> //no this file 20160603
//#include <rbus/scaler/dc_sys_reg.h>
//#include <rbusScaledownReg.h>
#include <rbus/vgip_reg.h>
#include <rbus/sub_vgip_reg.h>
//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/ppoverlay_reg.h>
//#include <rbus/scaler/rbustv-sb2-dcu1reg.h>
//#include <rbus/scaler/rbusCRTReg.h>
//#include <rbus/crt_reg.h>	 //no this file 20160603
#include <rbus/pll27x_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/vodma_reg.h>
#include <rbus/timer_reg.h>//for timer counter

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <scalercommon/scalerCommon.h>
#endif

#include <rbus/hsd_dither_reg.h>	 //no this file 20160603
#include <tvscalercontrol/io/ioregdrv.h>
//#include "libs/utils/rtSystem.h"
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scaler/scalervideo.h>
#include <tvscalercontrol/scalerdrv/scalerclock.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/pipmp.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#if 1  //fixme:vip
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/film.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/intra.h>
#include <tvscalercontrol/vip/xc.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/vip/ultrazoom.h>

#endif
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/scalerdrv/scaler_2dcvt3d_ctrl.h>
#include <tvscalercontrol/scalerdrv/scaler_i2rnd.h>

//#include <tvscalercontrol/scaler/scalerLib.h>

//#include <tvscalercontrol/scaler/modeState.h"
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
#include "tvscalercontrol/scaler/scalervideo.h" //for fw_video_get_27mhz_mode()
#endif //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
#include <scaler_vscdev.h>
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle_lib.h>

/*======================== PRIVATE CONSTANTS =================================*/

/*======================== PRIVATE MACROS ====================================*/

/*======================== PRIVATE TYPES =====================================*/

/*==================== Variables ================= */
//static unsigned char force_reset_di_flag = FALSE;//Force to reset DI flag
static unsigned char DI_chroma_10bits_in = FALSE;
static unsigned char DI_Gamemode_EN = FALSE;

static unsigned char DI_wait_frame_num = 0;//we need to wait DI frame and let DI ready
//static unsigned int record_timer_counter = 0;// record TIMER_SCPU_CLK90K_LO_reg when set DI

#ifdef CONFIG_I2RND_ENABLE
extern unsigned char vsc_i2rnd_sub_stage;
extern unsigned char vsc_i2rnd_dualvo_sub_stage;
extern unsigned char check_Sub_I2rnd_done(void);
#endif
#define VPQ_COMPILER_ERROR_ENABLE_MERLIN6


#define VO_FIXED_2K1K_HTOTAL 2480
#define VO_FIXED_4K2K_HTOTAL 4400
/*======================== PRIVATE FUNCTIONS =================================*/
/*============================================================================*/
/**
 * scalerip_bypass
 * Set whether channel enter IP or not
 *
 * @param <info> { display-info struecture }
 * @param <byPass> { enter/no enter }
 * @return { none }
 *
 */
static void scalerip_bypass(unsigned char byPass)
{
	SCALER_DISP_CHANNEL display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	unsigned char isAdaptivestream_flag = 0;
	unsigned int height;
	unsigned int width;
	unsigned char input_src_type;
	unsigned char isInterlace;
	vgip_data_path_select_RBUS data_path_select_reg;
	data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);

	height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);

	input_src_type = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);

	if (input_src_type == VSC_INPUTSRC_VDEC)
		isAdaptivestream_flag = get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY);

	isInterlace = Scaler_DispGetStatus(display, SLR_DISP_INTERLACE);

	rtd_pr_vbe_info("IP_bypass,byPass=%d,, input_src_type=%d,, isAdaptivestream_flag=%d,, height=%d, width=%d, isInterlace=%d,\n",
		byPass, input_src_type, isAdaptivestream_flag, height, width, isInterlace);

	if(byPass){ // bypass
		if(display){
			if ((Get_sub_from_DI_mux_path()) && (data_path_select_reg.uzd1_in_sel ==1)) {
				data_path_select_reg.uzd2_in_sel = 1;
				//data_path_select_reg.xcdi_in_sel = 1;
			} else if ((Get_sub_from_DI_mux_path()) && (data_path_select_reg.uzd1_in_sel ==0)) {
				data_path_select_reg.uzd2_in_sel = 3;
				//data_path_select_reg.xcdi_in_sel = 1;
			} else
				data_path_select_reg.uzd2_in_sel = 0;

			IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
		}else{
			/* always on for snr hw issue, elieli*/
			/* ====20180610 for power issue, USB 4K60 disable RTNR CLK==== */
			/* ====20180610 for power issue, USB 4K60 disable RTNR CLK==== */
			if (drvif_color_Get_DRV_SNR_Clock(display, 0) == 1) {
#ifdef CONFIG_I2RND_ENABLE
				//Eric@20171019 Dual vo i2rnd do not change snr clock setting when rerun main scaler path
				if(!(Scaler_I2rnd_get_timing_enable() && ((vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1) || (vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC))))
#endif
				{
					drvif_color_Set_DRV_SNR_Clock(display, 0);
					data_path_select_reg.uzd1_in_sel = 0;
					data_path_select_reg.di_i_clk_en = 0;
					/* ====20180610 for power issue, USB 4K60 disable RTNR CLK==== */
					if (isAdaptivestream_flag == 0)
						data_path_select_reg.xcdi_clk_en = 0;
					else
						data_path_select_reg.xcdi_clk_en = 1;
					/* ====20180610 for power issue, USB 4K60 disable RTNR CLK==== */
					IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
					drvif_color_Set_DRV_SNR_Clock(display, 1);
				}
			} else {
				data_path_select_reg.uzd1_in_sel = 0;
				data_path_select_reg.di_i_clk_en = 0;

				/* ====20180610 for power issue, USB 4K60 disable RTNR CLK==== */
					if (isAdaptivestream_flag == 0)
						data_path_select_reg.xcdi_clk_en = 0;
					else
						data_path_select_reg.xcdi_clk_en = 1;
				/* ====20180610 for power issue, USB 4K60 disable RTNR CLK==== */
				IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
			}
		}
	}else{
		if(display){
			data_path_select_reg.xcdi_in_sel = 1;
			data_path_select_reg.uzd2_in_sel = 1;
			IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
		}else{
			/* always on for snr hw issue, elieli*/
			if (drvif_color_Get_DRV_SNR_Clock(display, 0) == 1) {
#ifdef CONFIG_I2RND_ENABLE
				//Eric@20171019 Dual vo i2rnd do not change snr clock setting when rerun main scaler path
				if(!(Scaler_I2rnd_get_timing_enable() && ((vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1) || (vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC))))
#endif
				{
					drvif_color_Set_DRV_SNR_Clock(display, 0);
					data_path_select_reg.uzd1_in_sel = _ENABLE;
					data_path_select_reg.xcdi_in_sel = _CHANNEL1;
					data_path_select_reg.xcdi_clk_en = _ENABLE;
					if (isInterlace == 1 || isAdaptivestream_flag == 1)
						data_path_select_reg.di_i_clk_en = _ENABLE;
					else
						data_path_select_reg.di_i_clk_en = _DISABLE;

					IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
					drvif_color_Set_DRV_SNR_Clock(display, 1);
				}
			} else {
					data_path_select_reg.uzd1_in_sel = _ENABLE;
					data_path_select_reg.xcdi_in_sel = _CHANNEL1;
		            		data_path_select_reg.di_i_clk_en = _ENABLE;
					data_path_select_reg.xcdi_clk_en = _ENABLE;
					if (isInterlace == 1 || isAdaptivestream_flag == 1)
						data_path_select_reg.di_i_clk_en = _ENABLE;
					else
						data_path_select_reg.di_i_clk_en = _DISABLE;

				IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
			}
		}
	}

}

#ifndef UT_flag

void scalerip_updatesubmux(void)
{
	vgip_data_path_select_RBUS data_path_select_reg;
	if (!Get_sub_from_DI_mux_path())
		return;

	data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
	if ((Get_sub_from_DI_mux_path()) && (data_path_select_reg.uzd1_in_sel ==1))
		data_path_select_reg.uzd2_in_sel = 1;
	else if (Get_sub_from_DI_mux_path() && (data_path_select_reg.uzd1_in_sel ==0))
		data_path_select_reg.uzd2_in_sel = 3;
	else
		data_path_select_reg.uzd2_in_sel = 0;
	IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
}

#endif// end of UT_flag

/*============================================================================*/
/**
 * Modify_DI_RdStart
 * Set DI-ReadStart
 *
 * @param <channel> { i domain- channel }
 * @param <di_f>	{ input source: interlace or progressive}
 * @param <rtnr_f>		{ project: RTNR on or off}
  * @param <hscaledown_f>		{ Horizontal Scale Down on or off }
 * @return { none }
 *
 */
static void scalerip_modify_rdstart(unsigned char channel, unsigned char di_f, unsigned char rtnr_f, unsigned char hscaledown_f)
{
	unsigned int hsd_act_large = 0;
	unsigned int hsd_act_small = 0;
	di_im_di_debug_mode_RBUS reg_input_di_ctrl_REG;
	di_im_di_active_window_control_RBUS reg_active_window_REG;
	hsd_dither_di_hsd_scale_hor_factor_RBUS		di_hsd_scale_hor_factor_REG;
	hsd_dither_di_hsd_initial_value_RBUS			di_hsd_initial_value_REG;
	hsd_dither_di_hsd_hor_segment_RBUS			di_hsd_hor_segment_REG;
	//hsd_dither_di_hsd_hor_delta1_RBUS			di_hsd_hor_delta1_REG;
	rtd_pr_vbe_debug("Into Modify_DI_RdStart()\n");
	rtd_pr_vbe_debug("channel=%d, di_f=%d, rtnr_f=%d, hscaledown_f=%d\n",channel,di_f,rtnr_f,hscaledown_f);
	if(rtnr_f)
	{
		if(!hscaledown_f)
			return;
	}
	else if( (!di_f) || (!hscaledown_f))
		return;


#ifdef CONFIG_SDNR_CROP
	if (FALSE == Scaler_Get_di_hsd_flag())
		return;
#endif

	//Elsie 20130308: Using hsd dither instead of UZD in Magellan
	if (channel == _CHANNEL1)
	{
		di_hsd_scale_hor_factor_REG.regValue 	= IoReg_Read32(HSD_DITHER_DI_HSD_Scale_Hor_Factor_reg);
		di_hsd_initial_value_REG.regValue		= IoReg_Read32(HSD_DITHER_DI_HSD_Initial_Value_reg);
		di_hsd_hor_segment_REG.regValue 		= IoReg_Read32(HSD_DITHER_DI_HSD_Hor_Segment_reg);
		//di_hsd_hor_delta1_REG.regValue		= IoReg_Read32(HSD_DITHER_DI_HSD_Hor_Delta1_reg);
#if 0 //frank@0704 mark below code to solve some size will display fail problem to solve mantis#40619,43672
	if(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > 3840){//frank@0626 change below code to solve mantis#40619
		hsd_act_large = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
	}else{
		hsd_act_large = 1+(((di_hsd_initial_value_REG.hor_ini << 12) + di_hsd_scale_hor_factor_REG.hor_fac *
		  (di_hsd_hor_segment_REG.nl_seg1 *2 + di_hsd_hor_segment_REG.nl_seg2) +
		  (di_hsd_hor_delta1_REG.nl_d1 * di_hsd_hor_segment_REG.nl_seg1) *
		  (di_hsd_hor_segment_REG.nl_seg1 + di_hsd_hor_segment_REG.nl_seg2) -
		  di_hsd_scale_hor_factor_REG.hor_fac)>>20);
	}
#else
		hsd_act_large = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
#endif
		hsd_act_small = /*di_hsd_hor_segment_REG.nl_seg1 *2*/ + di_hsd_hor_segment_REG.nl_seg2;

		//rtd_pr_vbe_debug("h_ini=%d, h_fac=%x, Seg1=%d, Seg2=%d, NL_D1=%d\n",di_hsd_initial_value_REG.hor_ini,
		//di_hsd_scale_hor_factor_REG.hor_fac, di_hsd_hor_segment_REG.nl_seg1,
		//di_hsd_hor_segment_REG.nl_seg2,	di_hsd_hor_delta1_REG.nl_d1);
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else
	{
		di_hsd_hor_segment_REG.regValue 		= IoReg_Read32(HSD_DITHER_DI_HSD_Hor_Segment_reg);
		hsd_act_large = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);//Thur added, recommended by Fishtail 20080423
		hsd_act_small = /*di_hsd_hor_segment_REG.nl_seg1 *2*/ + di_hsd_hor_segment_REG.nl_seg2;
	}
#endif


#if 0	//Old way
	if (channel == _CHANNEL1)
	{
		scaledown_ich1_uzd_Scale_Hor_Factor_RBUS		ich1_uzd_Scale_Hor_Factor_REG;
		scaledown_ich1_uzd_Initial_Value_RBUS			ich1_uzd_Initial_Value_REG;
		scaledown_ich1_uzd_Hor_Segment_RBUS		ich1_uzd_Hor_Segment_REG;
		scaledown_ich1_uzd_Hor_Delta1_RBUS			ich1_uzd_Hor_Delta1_REG;

		ich1_uzd_Scale_Hor_Factor_REG.regValue 	= IoReg_Read32(SCALEDOWN_ICH1_UZD_SCALE_HOR_FACTOR_VADDR);
		ich1_uzd_Initial_Value_REG.regValue		= IoReg_Read32(SCALEDOWN_ICH1_UZD_INITIAL_VALUE_VADDR);
		ich1_uzd_Hor_Segment_REG.regValue 		= IoReg_Read32(SCALEDOWN_ICH1_UZD_HOR_SEGMENT_VADDR);
		ich1_uzd_Hor_Delta1_REG.regValue		= IoReg_Read32(SCALEDOWN_ICH1_UZD_HOR_DELTA1_VADDR);

		hsd_act_large = 1+(((ich1_uzd_Initial_Value_REG.hor_ini << 12) + ich1_uzd_Scale_Hor_Factor_REG.hor_fac *
		  (ich1_uzd_Hor_Segment_REG.nl_seg1 *2 + ich1_uzd_Hor_Segment_REG.nl_seg2) +
		  (ich1_uzd_Hor_Delta1_REG.nl_d1 * ich1_uzd_Hor_Segment_REG.nl_seg1) *
		  (ich1_uzd_Hor_Segment_REG.nl_seg1 + ich1_uzd_Hor_Segment_REG.nl_seg2) -
		  ich1_uzd_Scale_Hor_Factor_REG.hor_fac)>>20);
		hsd_act_small = ich1_uzd_Hor_Segment_REG.nl_seg1 *2 + ich1_uzd_Hor_Segment_REG.nl_seg2;

	rtd_pr_vbe_debug("h_ini=%d, h_fac=%x, Seg1=%d, Seg2=%d, NL_D1=%d\n",ich1_uzd_Initial_Value_REG.hor_ini,
		ich1_uzd_Scale_Hor_Factor_REG.hor_fac, ich1_uzd_Hor_Segment_REG.nl_seg1,
		ich1_uzd_Hor_Segment_REG.nl_seg2,	ich1_uzd_Hor_Delta1_REG.nl_d1);
	}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	else
	{
		scaledown_ich2_uzd_H_Output_Size_RBUS		ich2_uzd_H_Output_Size_REG;
		scaledown_ich2_uzd_Scale_Hor_Factor_RBUS		ich2_uzd_Scale_Hor_Factor_REG;
		scaledown_ich2_uzd_Initial_Value_RBUS			ich2_uzd_Initial_Value_REG;

		ich2_uzd_H_Output_Size_REG.regValue	= IoReg_Read32(SCALEDOWN_ICH2_UZD_H_OUTPUT_SIZE_VADDR);
		ich2_uzd_Scale_Hor_Factor_REG.regValue	= IoReg_Read32(SCALEDOWN_ICH2_UZD_SCALE_HOR_FACTOR_VADDR);
		ich2_uzd_Initial_Value_REG.regValue		= IoReg_Read32(SCALEDOWN_ICH2_UZD_INITIAL_VALUE_VADDR);

		/*
		hsd_act_large = 1 + (((ich2_uzd_Initial_Value_REG.hor_ini << 12) +
		(ich2_uzd_Scale_Hor_Factor_REG.hor_fac * ich2_uzd_H_Output_Size_REG.h_output_size) -
			(ich2_uzd_Scale_Hor_Factor_REG.hor_fac)) >> 20);
		*/
		hsd_act_large = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);//Thur added, recommended by Fishtail 20080423
		hsd_act_small = ich2_uzd_H_Output_Size_REG.h_output_size;

	}
#endif
#endif

	reg_input_di_ctrl_REG.regValue = IoReg_Read32(DI_IM_DI_DEBUG_MODE_reg);
	reg_active_window_REG.regValue = IoReg_Read32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg);

	// In 2D convert to 3D mode, need force to enable the scaling down IP even in/out picture horizontal size is 1:1
	// [WORKAROUND] set readstart to 2 in (hsd_act_large == hsd_act_small) case to avoid the picture offset error
	if(Get_PANEL_SCALER_2D_3D_CVT_HWSHIFT_ENABLE() && (hsd_act_large == hsd_act_small))
		reg_input_di_ctrl_REG.readstart = 2;
	else{
		unsigned int m_readstart = 0;
		//m_readstart = hsd_act_large - hsd_act_small;
		m_readstart = hsd_act_large - hsd_act_small + 1;        //      Readstart = Line size before HSD - line size after HSD (reg_hsize)+1
		#if 0
		if(m_readstart > 2047)
			m_readstart = 2047;
		#endif
		reg_input_di_ctrl_REG.readstart = m_readstart;
	}
	reg_active_window_REG.hsize = hsd_act_small;

//	reg_active_window_REG.hblksize = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN) - hsd_act_small; // Blank;
	//for freeze framesync underflow case
	reg_active_window_REG.hblksize = 64;
//      reg_active_window_REG.hblksize = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN) - hsd_act_small; // Blank;
	IoReg_Write32(DI_IM_DI_DEBUG_MODE_reg, reg_input_di_ctrl_REG.regValue);
	IoReg_Write32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg, reg_active_window_REG.regValue);

	rtd_pr_vbe_debug("Modify_DI_readstart\n");
//	rtd_pr_vbe_debug("ch=%d, readstart=%d, large=%d, small=%d\n",
//		Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), reg18061010_REG.readstart, hsd_act_large, hsd_act_small);
}

/*============================================================================*/
extern unsigned char get_sub_OutputVencMode(void);
#if 0
void set_di_doublebuf_write(unsigned char diflag,unsigned char _2d_flag)
{
	unsigned char timeout = 0, frame_count = 0, frame_num=0;
	//_2d_flag = 0; // 20140930 disable Force2D function in Mac3

	if(get_sub_OutputVencMode() == TRUE)
		IoReg_ClearBits(VGIP_VGIP_CHN1_CTRL_reg, _BIT24|_BIT25);
	else
		IoReg_ClearBits(VGIP_INT_CTL_reg, _BIT3); //avoid double buffer set fail, if i-domain isr usable
	if(diflag){
		//disable force 2d,enable ip with double buffer register
		//IoReg_SetBits(DI_db_reg_ctl_reg, _BIT0);
		IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, (diflag << 28));
		while(((IoReg_Read32(DI_IM_DI_CONTROL_reg) & _BIT28) >> 28) != diflag)
		{
			IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, (diflag << 28));
			msleep(1);//frank@0430 change busy delay to speed up boot time
			if(++timeout>60)	//30)
			{
				break;
			}
		}

		if (timeout > 60)	//30)
		{
			rtd_pr_vbe_debug("DI Write Double Buffer Write Error !!!!!!!!!!!!!!!!!!\n");
		}
		//IoReg_ClearBits(BUS_TV_SB2_DCU1_ARB_CR1_VADDR, _BIT0);
		// avoid DC_SYS2 client separate enable fail issue due to DI send zero write command to bus
		drv_memory_disable_di_write_req(TRUE);

		//msleep(40);

		IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~(_BIT19|_BIT20), (_2d_flag <<19)); //noicte: IoReg_Mask32 can't set _BIT19 & _BIT20 seperately

		timeout = 0;
		while((((IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg) & _BIT19) >> 19) != _2d_flag) ||
		   (((IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg) & _BIT20) >> 20) != 0))
		{
			IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~(_BIT19|_BIT20), (_2d_flag <<19));
			msleep(1);//frank@0430 change busy delay to speed up boot time
			if(++timeout>60)	//30)
			{
				break;
			}
		}

		//WaitFor_IVS1();
		//WaitFor_IVS1();

		//IoReg_SetBits(BUS_TV_SB2_DCU1_ARB_CR1_VADDR, _BIT0);
		// avoid DC_SYS2 client separate enable fail issue due to DI send zero write command to bus
		drv_memory_disable_di_write_req(FALSE);

		if(Scaler_DispGetStatus(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),SLR_DISP_THRIP)) {
			if(((IoReg_Read32(DI_IM_DI_CONTROL_reg) & _BIT4) >> 4) == 1) {		// 3A
				frame_num = 3;
			} else {	// 5A
				frame_num = 5;
			}
		} else { //p
			frame_num = 2;
		}
		frame_count = 0;
		timeout = 0;
		while(frame_count < frame_num && timeout < 30) {
			WaitFor_IVS1();
			if(((IoReg_Read32(DI_IM_DI_DEBUG_MODE_reg) & _BIT8) >> 8) == 0){
				frame_count++;
			} else {
				frame_count = 0;
			}
			timeout++;
		}

	}else{
		//force 2D
		//disable  double buffer register
		//IoReg_ClearBits(DI_db_reg_ctl_reg, _BIT1|_BIT0);
		IoReg_ClearBits(DI_db_reg_ctl_reg, _BIT2);
			IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~(_BIT19|_BIT20), (_2d_flag <<19)); //noicte: IoReg_Mask32 can't set _BIT19 & _BIT20 seperately
 		while((((IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg) & _BIT19) >> 19) != _2d_flag) ||
		   (((IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg) & _BIT20) >> 20) != 0))
		{
			IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~(_BIT19|_BIT20), (_2d_flag <<19));
			msleep(1);//frank@0430 change busy delay to speed up boot time
			if(++timeout>60)	//30)
			{
				break;
			}
		}
		if (timeout > 60)	//30)
		{
			rtd_pr_vbe_debug("DI Write 2 Double Buffer Write Error !!!!!!!!!!!!!!!!!!\n");
		}
		//enable  double buffer register
		//IoReg_SetBits(DI_db_reg_ctl_reg, _BIT1);
		IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, (diflag << 28));
		//IoReg_SetBits(DI_db_reg_ctl_reg, _BIT0);

		//di disable
		timeout = 0;
		while(((IoReg_Read32(DI_IM_DI_CONTROL_reg) & _BIT28) >> 28) != diflag)
		{
			IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, (diflag << 28));
			msleep(1);//frank@0430 change busy delay to speed up boot time
			if(++timeout>60)	//30)
			{
				break;
			}
		}
		IoReg_SetBits(DI_db_reg_ctl_reg, _BIT2);
	}

	if(get_sub_OutputVencMode() == TRUE)
		IoReg_SetBits(VGIP_VGIP_CHN1_CTRL_reg, _BIT24|_BIT25);
	else
		IoReg_SetBits(VGIP_INT_CTL_reg, _BIT3);

//	rtd_pr_vbe_debug("timeout count = %d\n",timeout);
	if (timeout > 60)	//30)
	{
		rtd_pr_vbe_debug("DI Double Buffer Write Error !!!!!!!!!!!!!!!!!!\n");
	}

}
#else
unsigned int set_di_sta_90k;
unsigned char set_di_doublebuf_write_TV006(unsigned char diflag,unsigned char _2d_flag)
{
//#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6
#if 0	/* since DI use rolling line mode, can't disable DI */
	di_im_di_control_RBUS di_im_di_control_reg;
	di_db_reg_ctl_RBUS di_db_reg_ctl_reg;
	di_im_di_weave_window_control_RBUS di_im_di_weave_window_control_reg;
	di_di_dma_multi_req_num_RBUS di_di_dma_multi_req_num_reg;

	char timeout_1 = 0;
	char timeout_2 = 0;
	unsigned char DI_IP_En_Lv2=0;
	unsigned int db_apply_st = 0, db_apply_end = 0, dela_frame_end = 0;

	// check DI En in level 2
	//IoReg_ClearBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_read_mask);
	// check DI status
	DI_IP_En_Lv2 = DI_IM_DI_CONTROL_get_ip_enable(IoReg_Read32(DI_IM_DI_CONTROL_reg));
	// resort db read level
	IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_read_mask);

	// must enable double buffer in DI rolling line mode, elieli
	IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_en_mask);

	rtd_pr_vbe_info("DI_Enable in lv2=%d, diflag=%d,\n", DI_IP_En_Lv2, diflag);

	if (DI_IP_En_Lv2 == 0) {
		di_im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
		di_im_di_control_reg.ip_enable = 1;
		di_im_di_control_reg.write_enable_8 = 1;
		IoReg_Write32(DI_IM_DI_CONTROL_reg, di_im_di_control_reg.regValue);
#if 1
		// set force 2D enable if flow need to disable DI for saving BW
		di_im_di_weave_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
		// set rolling mode
		di_di_dma_multi_req_num_reg.regValue = IoReg_Read32(DI_DI_DMA_Multi_Req_Num_reg);
		if (diflag == 0) {
			di_im_di_weave_window_control_reg.force2d = 1;
			di_im_di_weave_window_control_reg.force2d_i2rnd = 1;
			//di_di_dma_multi_req_num_reg.rdma_addr_rolling = 0;
			//di_di_dma_multi_req_num_reg.wdma_addr_rolling = 0;
		} else {
			di_im_di_weave_window_control_reg.force2d = 0;
			di_im_di_weave_window_control_reg.force2d_i2rnd = 0;
			if (drv_memory_di_Get_Compression_Mode(0) == PQC_Line_Mode_Rolling) {
			//	di_di_dma_multi_req_num_reg.rdma_addr_rolling = 1;
			//	di_di_dma_multi_req_num_reg.wdma_addr_rolling = 1;
			} else {
			//	di_di_dma_multi_req_num_reg.rdma_addr_rolling = 0;
			//	di_di_dma_multi_req_num_reg.wdma_addr_rolling = 0;
			}
		}
		IoReg_Write32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, di_im_di_weave_window_control_reg.regValue);
		IoReg_Write32(DI_DI_DMA_Multi_Req_Num_reg, di_di_dma_multi_req_num_reg.regValue);
#endif
		IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);	// db apply

		// wait for apply done
		timeout_1 = 30;
		db_apply_st = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		do {
			di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
			timeout_1--;
			//msleep(1);
			usleep_range(10000, 10000);
		} while (di_db_reg_ctl_reg.db_apply == 1 && timeout_1 >= 0);

		db_apply_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		if (timeout_1 < 0) {
			rtd_pr_vbe_emerg("DI_Enable time out0, timeout1=%d, db_st=%d, db_end=%d, dela_end=%d,\n", timeout_1, db_apply_st, db_apply_end, dela_frame_end);
		}
#if 0
		// wait for more one frame
		msleep(1);
		timeout_2 = 30;
		IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);
		do {
			di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
			timeout_2--;
			msleep(1);
		} while (di_db_reg_ctl_reg.db_apply == 1 && timeout_2 >= 0);

		dela_frame_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		if (timeout_2 < 0) {
			rtd_pr_vbe_emerg("DI Enable time out, timeout2=%d, db_st=%d, db_end=%d, dela_end=%d,\n", timeout_2, db_apply_st, db_apply_end, dela_frame_end);
		}
#endif
		rtd_pr_vbe_info("DI_Enable done,t_0=%d, t1=%d,db_st=%d, db_end=%d, dela_end=%d,PQCMode=%d,\n",
			timeout_1, timeout_2, db_apply_st, db_apply_end, dela_frame_end, drv_memory_di_Get_Compression_Mode(0));
	} else {
#if 1
		// set force 2D enable if flow need to disable DI for saving BW
		di_im_di_weave_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
		// set rolling mode
		di_di_dma_multi_req_num_reg.regValue = IoReg_Read32(DI_DI_DMA_Multi_Req_Num_reg);
		if (diflag == 0) {
			di_im_di_weave_window_control_reg.force2d = 1;
			di_im_di_weave_window_control_reg.force2d_i2rnd = 1;
			//di_di_dma_multi_req_num_reg.rdma_addr_rolling = 0;
			//di_di_dma_multi_req_num_reg.wdma_addr_rolling = 0;

			IoReg_Write32(DI_DI_DMA_Multi_Req_Num_reg, di_di_dma_multi_req_num_reg.regValue);
			IoReg_Write32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, di_im_di_weave_window_control_reg.regValue);

			IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);	// db apply

			// wait for apply done
			timeout_1 = 30;
			db_apply_st = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			do {
				di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
				timeout_1--;
				//msleep(1);
				usleep_range(10000, 10000);
			} while (di_db_reg_ctl_reg.db_apply == 1 && timeout_1 >= 0);

			db_apply_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			if (timeout_1 < 0) {
				rtd_pr_vbe_emerg("DI_Enable time out, timeout2=%d, db_st=%d, db_end=%d, dela_end=%d,\n", timeout_1, db_apply_st, db_apply_end, dela_frame_end);
			}

		} else {
			if (drv_memory_di_Get_Compression_Mode(0) == PQC_Line_Mode_Rolling) {
#if 1
				// wait for one frame to enable rolling,  WOSQRTK-11927
				IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);	// db apply
				timeout_1 = 30;
				dela_frame_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
				do {
					di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
					timeout_1--;
					//msleep(1);
					usleep_range(10000, 10000);
				} while (di_db_reg_ctl_reg.db_apply == 1 && timeout_1 >= 0);

				if (timeout_1 < 0) {
					rtd_pr_vbe_emerg("DI_Enable time out1, timeout2=%d, db_st=%d, db_end=%d, dela_end=%d,\n", timeout_1, db_apply_st, db_apply_end, dela_frame_end);
				}
#endif
				//di_di_dma_multi_req_num_reg.rdma_addr_rolling = 1;
				//di_di_dma_multi_req_num_reg.wdma_addr_rolling = 1;

			} else {
				//di_di_dma_multi_req_num_reg.rdma_addr_rolling = 0;
				//di_di_dma_multi_req_num_reg.wdma_addr_rolling = 0;
			}

			IoReg_Write32(DI_DI_DMA_Multi_Req_Num_reg, di_di_dma_multi_req_num_reg.regValue);

			IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);	// db apply

			// wait for apply done
			timeout_1 = 30;
			db_apply_st = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			do {
				di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
				timeout_1--;
				//msleep(1);
				usleep_range(10000, 10000);
			} while (di_db_reg_ctl_reg.db_apply == 1 && timeout_1 >= 0);

			db_apply_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			if (timeout_1 < 0) {
				rtd_pr_vbe_emerg("DI_Enable time out, timeout2=%d, db_st=%d, db_end=%d, dela_end=%d,\n", timeout_1, db_apply_st, db_apply_end, dela_frame_end);
			}

			di_im_di_weave_window_control_reg.force2d = 0;
			di_im_di_weave_window_control_reg.force2d_i2rnd = 0;

			IoReg_Write32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, di_im_di_weave_window_control_reg.regValue);

			IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);	// db apply
		}

#endif
		rtd_pr_vbe_info("DI_Enable f2D ,t_0=%d, t1=%d,db_st=%d, db_end=%d, dela_end=%d,PQCMode=%d,\n",
			timeout_1, timeout_2, db_apply_st, db_apply_end, dela_frame_end, drv_memory_di_Get_Compression_Mode(0));
	}
	drv_memory_disable_di_write_req(FALSE);
	return 0;
//#endif //VPQ_COMPILER_ERROR_ENABLE_MERLIN6 end
//return 0; //please remove it

#else
	di_im_di_control_RBUS di_im_di_control_reg;
	di_db_reg_ctl_RBUS di_db_reg_ctl_reg;
	di_im_di_weave_window_control_RBUS di_im_di_weave_window_control_reg;

	unsigned int db_apply_st = 0;

	unsigned int db_apply_end = 0;
	char timeout_1 = -1;

	// must enable double buffer in DI rolling line mode, elieli
	IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_en_mask);

	//rtd_pr_vbe_info("DI_Enable , diflag=%d, h_flip=%d,\n", diflag, Scaler_color_access_RTNR_H_Rotate_Flag(0, 0));

	di_im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
	di_im_di_control_reg.ip_enable = 1;
	di_im_di_control_reg.write_enable_8 = 1;
	IoReg_Write32(DI_IM_DI_CONTROL_reg, di_im_di_control_reg.regValue);

	// set force 2D enable if flow need to disable DI for saving BW
	if (diflag == 0) {
		di_im_di_weave_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
		di_im_di_weave_window_control_reg.force2d = 1;
		di_im_di_weave_window_control_reg.force2d_i2rnd = 1;
		IoReg_Write32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, di_im_di_weave_window_control_reg.regValue);

		IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);	// db apply

		// wait for apply done
		db_apply_st = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

		timeout_1 = 30;
		do {
			di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
			timeout_1--;
			//msleep(1);
			usleep_range(10000, 10000);
		} while (di_db_reg_ctl_reg.db_apply == 1 && timeout_1 >= 0);

		db_apply_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		if (timeout_1 < 0) {
			rtd_pr_vbe_emerg("DI_Enable time out, timeout2=%d, db_st=%d, db_end=%d, diflag=%d, 2dflag=%d, h_rotate=%d,\n", 
				timeout_1, db_apply_st, db_apply_end, diflag, _2d_flag, Scaler_color_access_RTNR_H_Rotate_Flag(0, 0));
		}
		rtd_pr_vbe_info("DI_Enable f2D ,t_0=%d, set_di_sta_90k=%d, db_st=%d, db_end=%d, diflag=%d, 2dflag=%d, h_rotate=%d,\n", 
			timeout_1, set_di_sta_90k, db_apply_st, db_apply_end, diflag, _2d_flag, Scaler_color_access_RTNR_H_Rotate_Flag(0, 0));	
		
	} else {
		db_apply_st = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		
		di_im_di_weave_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
		di_im_di_weave_window_control_reg.force2d = 0;
		di_im_di_weave_window_control_reg.force2d_i2rnd = 0;
		IoReg_Write32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, di_im_di_weave_window_control_reg.regValue);
		IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);	// db apply

		timeout_1 = 30;
		do {
			di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
			timeout_1--;
			//msleep(1);
			usleep_range(10000, 10000);
		} while (di_db_reg_ctl_reg.db_apply == 1 && timeout_1 >= 0);

		db_apply_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		if (timeout_1 < 0) {
			rtd_pr_vbe_emerg("DI_Enable time out nw, timeout2=%d, db_st=%d, db_end=%d, diflag=%d, 2dflag=%d, h_rotate=%d,\n", 
				timeout_1, db_apply_st, db_apply_end, diflag, _2d_flag, Scaler_color_access_RTNR_H_Rotate_Flag(0, 0));
		}

		rtd_pr_vbe_info("DI_Enable unf2d ,t_0=%d, set_di_sta_90k=%d, db_st=%d, db_end=%d, diflag=%d, 2dflag=%d, h_rotate=%d,\n", 
			timeout_1, set_di_sta_90k, db_apply_st, db_apply_end, diflag, _2d_flag, Scaler_color_access_RTNR_H_Rotate_Flag(0, 0));	

	}
	drv_memory_disable_di_write_req(FALSE);
	return 0;
#endif
}
#endif

unsigned int DI_SW_Detection_in_VPQ_TSK_Stable_CNT;
unsigned int DI_SW_Detection_90k;
unsigned char Get_DI_RTNR_DMA_force2D_Status(void)
{
	unsigned char ret_val;
	unsigned int t_now, t_diff, t_swrest;
	unsigned short frame_time = 20;	// i mode 50Hz
	t_now = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

	if (t_now < DI_SW_Detection_90k)
		t_swrest = 0;
	else	
		t_swrest = DI_SW_Detection_90k;

	t_diff = t_now - t_swrest;

	if (t_diff >= (frame_time*90*DI_SW_Detection_in_VPQ_TSK_Stable_CNT)) {
		ret_val = 0;
		rtd_pr_vbe_info("DI_Enable, status done ,t_now=%d, t_diff=%d, t_swrest=%d, CNT=%d,\n",
					t_now, t_diff, t_swrest, DI_SW_Detection_in_VPQ_TSK_Stable_CNT);
	} else { 
		ret_val = 1;
	}
	return ret_val;
		
}

extern unsigned char get_vsc_run_adaptive_stream(unsigned char display);
/*============================================================================*/
/**
 * fw_scalerip_set_di
 * Set IP (video process) registers, includes its memory setting
 *
 * @param <info> { display-info struecture }
 * @return { none }
 *
 */
void fw_scalerip_set_di(void)
{
//	unsigned short temp;
	extern char DI_PQC_CRC_Check_reset_cnt;
	unsigned char m_3DDIFlag = 0;
	unsigned char diflag = 0;
	unsigned char input_src_type;
	unsigned char input_src_form;
	unsigned short display ;
	unsigned int htotal = 0;
	unsigned int h_porch_num = 0;
	vgip_data_path_select_RBUS data_path_select_reg;
	di_im_di_active_window_control_RBUS	 reg_Active_Win_di_Reg;
	di_im_di_weave_window_control_RBUS im_di_weave_window_control_reg;
	di_color_recovery_option_RBUS Color_Recovery_Option_Reg;
	vgip_vgip_chn1_ctrl_RBUS vgip_chn1_ctrl_reg;
	vgip_vgip_chn1_misc_RBUS vgip_chn1_misc_reg;
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
	sub_vgip_vgip_chn2_misc_RBUS vgip_chn2_misc_reg;
	sub_vgip_vgip_chn2_ctrl_RBUS vgip_chn2_ctrl_reg;
#endif
	di_im_di_control_RBUS im_di_control_reg;
	di_im_di_rtnr_new_control_RBUS rtnr_new_Ctrl;

	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;


//	vp9_ctrl0_RBUS vodma_vp9_ctrl0_reg;
	//frank@1008 add below code to enable DI apply default
	di_db_reg_ctl_RBUS di_db_reg_ctl_reg;
	di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
#ifdef CONFIG_I2RND_ENABLE
	if(Scaler_I2rnd_get_timing_enable())
		di_db_reg_ctl_reg.db_en = 0;	//i2rnd do not enable double buffer
#endif
	di_db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, di_db_reg_ctl_reg.regValue);

	display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (display == SLR_MAIN_DISPLAY) {
		set_di_sta_90k = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		rtd_pr_vbe_info("DI_Enable , DI start, set_di_sta_90k=%d,\n", set_di_sta_90k);
		DI_PQC_CRC_Check_reset_cnt = 10;
		DI_SW_Detection_in_VPQ_TSK_Stable_CNT = 0;
	}
	if (fwif_color_get_force_run_i3ddma_enable(display)) {
#ifdef CONFIG_I2RND_ENABLE
		//[ANDROIDTV-3562]i2r pip case, if main is AVD, need to check sub source @Crixus 20171025
		if((Scaler_I2rnd_get_timing_enable() == _ENABLE) && (display == SLR_MAIN_DISPLAY) && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC) || (vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1)))
			fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_SUB_DISPLAY), &input_src_type, &input_src_form);
		else
#endif
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &input_src_type, &input_src_form);
	}else{
		input_src_type = Scaler_InputSrcGetMainChType();
	}

#ifdef CONFIG_I2RND_ENABLE
	//check s0 still run idomain stage before refine vo1 clock, vgip hporch and DI h_blk size @Crixus 20180119
	if((display == SLR_MAIN_DISPLAY) && (Scaler_I2rnd_get_timing_enable() == _ENABLE) && !((vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1) || (vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC))){
		htotal = Scaler_I2rnd_get_speedup_htotal(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE));
	}
	else
#endif
	{
		htotal = Scaler_DispGetInputInfo(SLR_INPUT_H_LEN);
	}
	di_rtnr_demode_ctrl();//control de mode
	if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_THRIP) || Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_RTNR)) {	// will added in future
	//	di_rtnr_demode_ctrl();//control de mode
		// disable exSRAM access mux (Let DI run correctly)
 		//IoReg_ClearBits(EXSRAM_CTRL_reg, _BIT0);
		// DI 8 or 10 bits by LearnRPG

		Color_Recovery_Option_Reg.regValue = IoReg_Read32(DI_Color_Recovery_Option_reg);
		if (fw_scalerip_get_DI_chroma_10bits()) {
			Color_Recovery_Option_Reg.y10bitsin = 1;
			Color_Recovery_Option_Reg.c10bitsin= 1;
		} else {
			Color_Recovery_Option_Reg.y10bitsin = 0;
			Color_Recovery_Option_Reg.c10bitsin= 0;
		}
		IoReg_Write32(DI_Color_Recovery_Option_reg, Color_Recovery_Option_Reg.regValue);
		// END

		drvif_scaler_vactive_end_irq(_DISABLE, display);//disable VGIP ISR (for automa)
		drvif_scaler_vactive_sta_irq(_DISABLE, display);//disable VGIP ISR (for automa)

#ifdef CONFIG_SUPPORT_SRC_ADC
	        //forster modified 071122
		if (Scaler_InputSrcGetFrom((SCALER_DISP_CHANNEL)display) == _SRC_FROM_ADC) {
			IoReg_Mask32(DDS_PLL_PLL_CTRL_reg, ~(_BIT9 | _BIT10), _BIT9 | _BIT10);
#if 1 //def CONFIG_VGA_JUMP_PHASE_SOLVED
			IoReg_SetBits(ADC_ADC_CLOCK_reg, _BIT21); // 1x ADC clock invert
#endif
			rtd_pr_vbe_debug("\n Set dual ADC clock%c",'!');
		}
#endif
		//frank@0514 move this position let force 2d and disable DI double buffer register work
		IoReg_SetBits(VGIP_Data_Path_Select_reg, _BIT3);
#ifdef CONFIG_I2RND_ENABLE
		if((display == SLR_MAIN_DISPLAY)&&(check_Sub_I2rnd_done() == TRUE)&&(Scaler_I2rnd_get_timing_enable() == _ENABLE))
			;
		else
#endif
		{
		// Disable IP temp
		//frank@0312 mark below code to change doublebuf_write to confirm DI disable to solve mantis#40113
		// set_di_doublebuf_write_TV006(0, 1);

		}
		// [Code Sync][LewisLee][0990226][1]
		if(_CHANNEL2 == Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL))
		{
			data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
			data_path_select_reg.uzd2_in_sel = _ENABLE;
			data_path_select_reg.xcdi_in_sel = _CHANNEL2;
			IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
		}
		// [Code Sync][LewisLee][0990226][1][End]

		if(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_THRIP)) {
			//if(Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID) >= 960) {	 //motion adaptive 3A
			// VO source in SG@120Hz 3D platform
			// [3D FRC] (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 288)  // bandwidth issue when video timing > 576i FRC mode (+5 for overscan margin)
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
				if(VD_27M_HSD960_DI5A == fw_video_get_27mhz_mode((SCALER_DISP_CHANNEL)display))
				{
//					if(Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID) >= 960)
					if((Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID) > 960))
					{	//sync from pacific
						drv_memory_set_ip_fifo(0);
						DI_SW_Detection_in_VPQ_TSK_Stable_CNT = 3;
						IoReg_ClearBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
						im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
						im_di_control_reg.ma_3aenable = 1;
						im_di_control_reg.write_enable_1 = 1;
						IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);



						Scaler_DispSetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_MA3A,TRUE);
						//rtd_pr_vbe_debug("\nDI 3A\n");
					}
					else //motion adaptive 5A or 3A
					{	// 5A
						drv_memory_set_ip_fifo(1);
						DI_SW_Detection_in_VPQ_TSK_Stable_CNT = 4;
						IoReg_SetBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
						im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
						im_di_control_reg.ma_3aenable = 0;
						im_di_control_reg.write_enable_1 = 1;
						IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
						Scaler_DispSetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_MA3A,FALSE);
						//rtd_pr_vbe_debug("\nDI 5A\n");
					}
				}
				else// if(VD_27M_HSD960_DI5A != fw_video_get_27mhz_mode((SCALER_DISP_CHANNEL)display))
				{
					if(((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())  && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D)&&(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 288))
						|| (Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE() && modestate_I3DDMA_get_In3dMode() && (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == FALSE) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 288))
						/*((Scaler_InputSrcGetMainChType() == _SRC_VO) && (GET_IS3DFMT() == 0 && GET_HDMI_3DTYPE() == HDMI3D_FRAMESEQUENCE) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > (288+5)))*/
						||(((input_src_type == _SRC_VO)&&((get_vsc_run_adaptive_stream(display))||(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>960)))
#ifdef CONFIG_I2RND_ENABLE
						//Update I2rnd Dual VO condition @Crixus 20170828
						&& (!((Scaler_I2rnd_get_timing_enable() == _ENABLE) && ((vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC) || (vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1))))
#endif
						)||((input_src_type == _SRC_HDMI)&&(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>960))
						||((input_src_type == _SRC_YPBPR)&&(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>960))
						||((input_src_type == _SRC_VGA)&&(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>960)))
					{	//sync from pacific
						drv_memory_set_ip_fifo(0);
						DI_SW_Detection_in_VPQ_TSK_Stable_CNT = 3;
						IoReg_ClearBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
						im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
						im_di_control_reg.ma_3aenable = 1;
						im_di_control_reg.write_enable_1 = 1;
						IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
						Scaler_DispSetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_MA3A,TRUE);
						//rtd_pr_vbe_debug("\nDI 3A\n");
					}
					else //motion adaptive 5A or 3A
					{	// 5A
						drv_memory_set_ip_fifo(1);
						DI_SW_Detection_in_VPQ_TSK_Stable_CNT = 4;
						IoReg_SetBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
						im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
						im_di_control_reg.ma_3aenable = 0;
						im_di_control_reg.write_enable_1 = 1;
						IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
						Scaler_DispSetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_MA3A,FALSE);
						//When enter 5A mode, HW need more stable time @Crixus 20151204
						if(input_src_type == _SRC_HDMI)
							msleep(100);
						//rtd_pr_vbe_debug("\nDI 5A\n");
					}
				}
#else //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
				if(((Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE())  && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)==SLR_3D_SIDE_BY_SIDE_HALF_CVT_2D)&&(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 288))
					/*((Scaler_InputSrcGetMainChType() == _SRC_VO) && (GET_IS3DFMT() == 0 && GET_HDMI_3DTYPE() == HDMI3D_FRAMESEQUENCE) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > (288+5)))*/
					|| (Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE() && modestate_I3DDMA_get_In3dMode() && (Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC) == FALSE) && (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN) > 288))
					||((input_src_type == _SRC_VO)&&((get_vsc_run_adaptive_stream(display))||(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>960)))
					||((input_src_type == _SRC_HDMI)&&(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>960))
					||((input_src_type == _SRC_YPBPR)&&(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>960))
					||((input_src_type == _SRC_VGA)&&(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>960)))
			{   //sync from pacific
				drv_memory_set_ip_fifo(0);
				DI_SW_Detection_in_VPQ_TSK_Stable_CNT = 3;
				IoReg_ClearBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
				im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
				im_di_control_reg.ma_3aenable = 1;
				im_di_control_reg.write_enable_1 = 1;
				IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
				Scaler_DispSetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_MA3A,TRUE);
				//rtd_pr_vbe_debug("\nSet DI Memory Main 3A\n");
			}
				else //motion adaptive 5A or 3A
				{	// 5A
				drv_memory_set_ip_fifo(1);
				DI_SW_Detection_in_VPQ_TSK_Stable_CNT = 4;
				IoReg_SetBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
				im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
				im_di_control_reg.ma_3aenable = 0;
				im_di_control_reg.write_enable_1 = 1;
				IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
				Scaler_DispSetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_MA3A,FALSE);
				//rtd_pr_vbe_debug("\nSet DI Memory Main 5A\n");
			}
#endif //#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
		}
		else {
			drv_memory_set_ip_fifo(2);
			DI_SW_Detection_in_VPQ_TSK_Stable_CNT = 0;
			IoReg_ClearBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
			im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
			im_di_control_reg.ma_3aenable = 1;
			im_di_control_reg.write_enable_1 = 1;
			IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
			//rtd_pr_vbe_debug("\nSet RTNR Memory\n");
		}

		// update ma_3aenable info to VCPU
		if(VIP_RPC_system_info_structure_table == NULL){
			rtd_pr_vbe_debug("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		}else{
			VIP_RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.DI_info.DI_ma_3aenable = im_di_control_reg.ma_3aenable;
		}


		//frank@0311 Open vodma film mode control
		if (Scaler_InputSrcGetFrom((SCALER_DISP_CHANNEL)display) == _SRC_FROM_VO)
			IoReg_SetBits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT7);
		else
			IoReg_ClearBits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT7);

		//enable 3ddi//Frank@0514 move to front od set di to set 2D DI and disable DI
		//IoReg_SetBits(VGIP_Data_Path_Select_reg, _BIT3);

		rtd_pr_vbe_debug("3A,5A reg=%x\n",IoReg_Read32(DI_IM_DI_CONTROL_reg));

		//frank@04262010 add below code avoid 3ddi write data to the address 0 let code haneg ++
		//frank@0817 sync LV to speed up switch channel
		//fw_scaler_double_buffer_maskl((SCALER_DISP_CHANNEL)display,DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);
		// IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);
		m_3DDIFlag = 0;
		//frank@04262010 add below code avoid 3ddi write data to the address 0 let code haneg --
		if(/*Scaler_PipGetInfo(SLR_PIP_ENABLE) && */((display == SLR_MAIN_DISPLAY) && ((Scaler_InputSrcGetType(SLR_MAIN_DISPLAY) == _SRC_VGA) &&  ((Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR) == _MODE_1080I25) || (Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR) == _MODE_1080I30))))) {
#ifdef CONFIG_1080I_THROUGH_FRAMESYNC
			if ((Scaler_PipGetInfo(SLR_PIP_TYPE)>=SUBTYPE_PIPMLSR)&&(Scaler_PipGetInfo(SLR_PIP_TYPE)<=SUBTYPE_PIPMLSR_4x3))
			{
				//frank@0817 sync LV to speed up switch channel
				//fw_scaler_double_buffer_maskl((SCALER_DISP_CHANNEL)display,DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);  // 2D DI
				// IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);  // 2D DI
				//frank@0414 free DI memory to avoid memory overlay of main and sub channel
				drvif_memory_free_block(MEMIDX_DI);
				Scaler_DispSetStatus(SLR_MAIN_DISPLAY,SLR_DISP_RTNR,FALSE);
			}
			else
				m_3DDIFlag = 1;
#else
			// IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);  // 2D DI
			//frank@0414 free DI memory to avoid memory overlay of main and sub channel
			drvif_memory_free_block(MEMIDX_DI);
			Scaler_DispSetStatus(SLR_MAIN_DISPLAY,SLR_DISP_RTNR,FALSE);
#endif
		}
	#if 0
 		else if(Scaler_TVGetScanMode()) {
			// IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);
			Scaler_DispSetStatus(SLR_MAIN_DISPLAY,SLR_DISP_RTNR,FALSE);
		}
	#endif
		else if(display == SLR_SUB_DISPLAY) {
			if((((input_src_type == _SRC_HDMI || input_src_type == _SRC_DISPLAYPORT)||(Scaler_InputSrcGetMainChFrom() == _SRC_FROM_ADC)||(Scaler_InputSrcGetMainChFrom() == _SRC_FROM_VO))&&
				((Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR) >= _MODE_1080I25) && (Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR) <= _MODE_1080I30)) ) ||
			    ((input_src_type == _SRC_HDMI || input_src_type == _SRC_DISPLAYPORT) && drvif_Hdmi_GetInterlace() && (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) >=0x06F0))){
				// fw_scaler_double_buffer_maskl((SCALER_DISP_CHANNEL)display,DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);
				//frank@0414 free DI memory to avoid memory overlay of main and sub channel
				drvif_memory_free_block(MEMIDX_DI);
			}else{
				m_3DDIFlag = 1;
			}
		} else {
			if(Scaler_InputSrcGetType(display) == _SRC_VO) {
				if(vbe_disp_get_adaptivestream_fs_mode()==1)  {
					if( (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>3000) &&
					    (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ)>310))
						m_3DDIFlag = 0;//4k2k50,4k2k60
					else
						m_3DDIFlag = 1; // 3D DI
				}
				else
					m_3DDIFlag = 1; // 3D DI
			} else {
				m_3DDIFlag = 1; // 3D DI
			}
		}


		drvif_scaler_vactive_end_irq(_ENABLE, display);//disable VGIP ISR (for automa)
		drvif_scaler_vactive_sta_irq(_ENABLE, display);//disable VGIP ISR (for automa)



				// Decide whether enable delete 4 line (Up 2 line and down 2 line) and calculate line
		if(fwif_scaler_decide_display_cut4line(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)))
		{
			im_di_weave_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
			im_di_weave_window_control_reg.dellineen = _ENABLE;
			IoReg_Write32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, im_di_weave_window_control_reg.regValue);
		}
		else
		{
			im_di_weave_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
			im_di_weave_window_control_reg.dellineen = _DISABLE;
			IoReg_Write32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, im_di_weave_window_control_reg.regValue);
		}

		// Set active size to normalizing size in order to avoid to conflict
		reg_Active_Win_di_Reg.regValue = IoReg_Read32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg);
		reg_Active_Win_di_Reg.hsize = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID);// Width;
		reg_Active_Win_di_Reg.vsize = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN); // Height;
		reg_Active_Win_di_Reg.hblksize = htotal - Scaler_DispGetInputInfo(SLR_INPUT_DI_WID); // Blank;
		IoReg_Write32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg, reg_Active_Win_di_Reg.regValue);

		//frank@0418 add below code for 4k2k input size ++//frank@0503 reset hsize_msb/vsize_msb/hblksize_msb avoid image noise
		im_di_weave_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
		if(Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)>0x7FF){
			im_di_weave_window_control_reg.hsize_msb= (Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)>>11);
		}else{
			im_di_weave_window_control_reg.hsize_msb= 0;
		}

		if(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)>0x3FF){
			im_di_weave_window_control_reg.vsize_msb= (Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN)>>10);
		}else{
			im_di_weave_window_control_reg.vsize_msb= 0;
		}

		if((htotal - Scaler_DispGetInputInfo(SLR_INPUT_DI_WID))>0x3FF){
			im_di_weave_window_control_reg.hblksize_msb = ((htotal - Scaler_DispGetInputInfo(SLR_INPUT_DI_WID))>>10);
		}else{
			im_di_weave_window_control_reg.hblksize_msb = 0;
		}
		IoReg_Write32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, im_di_weave_window_control_reg.regValue);
		//frank@0418 add below code for 4k2k input size --

	//	if (!Scaler_Get_DisplayRatio_Change_Flag())
		{
		//fix me:vip
		#if 1
			#if 0
			drvif_module_film_init(display);
			#else
			Scaler_SetFilm_Setting();
			#endif
			drvif_color_intra_init();
			drvif_color_ma_init();
			drvif_color_auto_bal_init(display,Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_THRIP));//20090907 jason9.ccc add
		#endif
		}


		if(display == _CHANNEL1)
		{
			vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
			if(vgip_chn1_ctrl_reg.ch1_digital_mode) //digital mode need to set porch
			{
				vgip_chn1_misc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_MISC_reg);
				if(Scaler_InputSrcGetType(display) == _SRC_VO){
					if ((get_vsc_run_adaptive_stream(display))&& ((htotal == VO_FIXED_4K2K_HTOTAL) ||(Scaler_DispGetInputInfo(SLR_INPUT_H_LEN) == VO_FIXED_2K1K_HTOTAL)))
					{
						vgip_chn1_misc_reg.ch1_hporch_num = 0;
					}
					else if(htotal > (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) + VGIP_H_PORCH_MARGIN)){
						//Eric@20180604 h_porch_num constrain:
						//1. hback_porch(hstart) <= hporch_num < hback_porch(hstart) + hactive
						//2. hporch_num > hback_porch(hstart) + hactive + 10(DIC Suggest)
						//=>if (hstart + hactive <= h_porch_num <= hstart + hactive) then set h_porch_num = hstart + 50
						h_porch_num = htotal - Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) - VGIP_H_PORCH_MARGIN;
						if((h_porch_num >= (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE) + Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)))&& (h_porch_num <= (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE) + Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)) + 10))
							h_porch_num = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE) + 50;
						vgip_chn1_misc_reg.ch1_hporch_num = 0;
					}
					else{
						vgip_chn1_misc_reg.ch1_hporch_num = 0;
						rtd_pr_vbe_debug("The H total porch is not enough!!!!\n");
					}
				}
				else//hdmi case
					vgip_chn1_misc_reg.ch1_hporch_num = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_MISC_reg, vgip_chn1_misc_reg.regValue);
			}
			else//analog mode
			{
				vgip_chn1_misc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_MISC_reg);
				vgip_chn1_misc_reg.ch1_hporch_num = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_MISC_reg, vgip_chn1_misc_reg.regValue);
			}
		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else
		{

			vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			if(vgip_chn2_ctrl_reg.ch2_digital_mode)
			{
				vgip_chn2_misc_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_MISC_reg);
				if(Scaler_InputSrcGetType(display) == _SRC_VO)
					vgip_chn2_misc_reg.ch2_hporch_num = 0;
				else//hdmi case
					vgip_chn2_misc_reg.ch2_hporch_num = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_MISC_reg, vgip_chn2_misc_reg.regValue);

			}
			else
			{
				vgip_chn2_misc_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_MISC_reg);
				vgip_chn2_misc_reg.ch2_hporch_num = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_MISC_reg, vgip_chn2_misc_reg.regValue);
			}
		}
#endif


		// Di read start must be set if turn on horizontal UZD
//		scalerip_modify_rdstart();
		scalerip_modify_rdstart(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL),Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_THRIP),Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_RTNR),Scaler_Get_di_hsd_flag());

		// no matter RTNR flag, must check progressive mode again. by LearnRPG
		drvif_color_rtnr_progressive_check(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_THRIP),Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL));

		if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY) {
			Scaler_color_set_RTNR_H_Rotate(display, Scaler_color_access_RTNR_H_Rotate_Flag(0, 0));

			rtnr_new_Ctrl.regValue = IoReg_Read32(DI_IM_DI_RTNR_NEW_CONTROL_reg);
			if(Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)>1920 && (Scaler_color_access_RTNR_H_Rotate_Flag(0, 0) == 0)){
				rtnr_new_Ctrl.prtnr_4k2k = 0x1; 	//4K2K support
			}else{
				rtnr_new_Ctrl.prtnr_4k2k = 0x0;
			}
			IoReg_Write32(DI_IM_DI_RTNR_NEW_CONTROL_reg, rtnr_new_Ctrl.regValue);
		}
		
		if (Scaler_color_access_RTNR_H_Rotate_Flag(0, 0) == 1)
			diflag = 0; /* for NTNR special case, need to disable DMA for BW issue. */
		else
			diflag = 1;  // set di

		set_di_doublebuf_write_TV006(diflag, !m_3DDIFlag);

		if(_CHANNEL1 == display)
		{
			/* always on for snr hw issue, elieli*/
			if (drvif_color_Get_DRV_SNR_Clock(display, 0) == 1) {
#ifdef CONFIG_I2RND_ENABLE
				//Eric@20171019 Dual vo i2rnd do not change snr clock setting when rerun main scaler path
				if(!((Scaler_I2rnd_get_timing_enable() == _ENABLE) && ((vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1) || (vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC))))
#endif
				{
					drvif_color_Set_DRV_SNR_Clock(display, 0);
					data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
					data_path_select_reg.uzd1_in_sel = _ENABLE;
					data_path_select_reg.xcdi_in_sel = _CHANNEL1;
		            		data_path_select_reg.di_i_clk_en = _ENABLE;
					data_path_select_reg.xcdi_clk_en = _ENABLE;//Enable clock. Will Add 20151218
					IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
					drvif_color_Set_DRV_SNR_Clock(display, 1);
				}
			} else {
				data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
				data_path_select_reg.uzd1_in_sel = _ENABLE;
				data_path_select_reg.xcdi_in_sel = _CHANNEL1;
	            		data_path_select_reg.di_i_clk_en = _ENABLE;
				data_path_select_reg.xcdi_clk_en = _ENABLE;//Enable clock. Will Add 20151218
				IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);
			}
		}
		scalerip_bypass(FALSE);
	}
	else {
#ifdef CONFIG_VGIP_DIGITAL_MODE
		if(display == _CHANNEL1)
		{
			if(get_vsc_run_adaptive_stream(display) || Get_vo_smoothtoggle_timingchange_flag(SLR_MAIN_DISPLAY)){
				if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE)) {
					drv_memory_set_ip_fifo(SLR_DI_3A_mode);
				} else {
					drv_memory_set_ip_fifo(SLR_DI_progress_mode);
				}
			}
				//drv_memory_set_ip_memory_size(SLR_DI_progress_mode);//for progressive 4k2k framerate>310
			vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
			if(vgip_chn1_ctrl_reg.ch1_digital_mode)
			{
				vgip_chn1_misc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_MISC_reg);

				if(Scaler_InputSrcGetType(display) == _SRC_VO){
					if ((get_vsc_run_adaptive_stream(display))&& ((htotal == VO_FIXED_4K2K_HTOTAL) ||(Scaler_DispGetInputInfo(SLR_INPUT_H_LEN) == VO_FIXED_2K1K_HTOTAL)))
					{
						vgip_chn1_misc_reg.ch1_hporch_num = 0;
					}
 					else if(htotal > (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) + VGIP_H_PORCH_MARGIN)){
						//Eric@20180604 h_porch_num constrain:
						//1. hback_porch(hstart) <= hporch_num < hback_porch(hstart) + hactive
						//2. hporch_num > hback_porch(hstart) + hactive + 10(DIC Suggest)
						//=>if (hstart + hactive <= h_porch_num <= hstart + hactive) then set h_porch_num = hstart + 50
						h_porch_num = htotal - Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE) - VGIP_H_PORCH_MARGIN;
						if((h_porch_num >= (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE) + Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)))&& (h_porch_num <= (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE) + Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)) + 10))
							h_porch_num = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA_PRE) + 50;
						vgip_chn1_misc_reg.ch1_hporch_num = 0;
					}
					else{
						vgip_chn1_misc_reg.ch1_hporch_num = 0;
						rtd_pr_vbe_debug("The H total porch is not enough!!!!\n");
					}
				}
				else//hdmi case
					vgip_chn1_misc_reg.ch1_hporch_num = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_MISC_reg, vgip_chn1_misc_reg.regValue);
			}
			else
			{
				vgip_chn1_misc_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_MISC_reg);
				vgip_chn1_misc_reg.ch1_hporch_num = 0;
				IoReg_Write32(VGIP_VGIP_CHN1_MISC_reg, vgip_chn1_misc_reg.regValue);
			}
		}
#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		else
		{
			vgip_chn2_ctrl_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			if(vgip_chn2_ctrl_reg.ch2_digital_mode)
			{
				vgip_chn2_misc_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_MISC_reg);
				vgip_chn2_misc_reg.ch2_hporch_num = 0;	/* need to check HDMI and VO case, using the same value now */
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_MISC_reg, vgip_chn2_misc_reg.regValue);
			}
			else
			{
				vgip_chn2_misc_reg.regValue = IoReg_Read32(SUB_VGIP_VGIP_CHN2_MISC_reg);
				vgip_chn2_misc_reg.ch2_hporch_num = 0;
				IoReg_Write32(SUB_VGIP_VGIP_CHN2_MISC_reg, vgip_chn2_misc_reg.regValue);
			}
		}
#endif
#else
if(display == _CHANNEL1)
{
	if(get_vsc_run_adaptive_stream(display) || Get_vo_smoothtoggle_timingchange_flag(SLR_MAIN_DISPLAY)){
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_INTERLACE)) {
			drv_memory_set_ip_fifo(SLR_DI_3A_mode);
		} else {
			drv_memory_set_ip_fifo(SLR_DI_progress_mode);
		}
	}
}
#endif//CONFIG_VGIP_DIGITAL_MODE

		if(display == SLR_MAIN_DISPLAY) {
			//  if sub enters DI/RTNR, do not close DI
				if(!(/*Scaler_PipGetInfo(SLR_PIP_ENABLE)&&*/(Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_THRIP)||Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_RTNR)))) {
					//IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, 0); //chengying mark set by double buffer
					diflag = 0;
					//frank@0514 move this position let force 2d and disable DI double buffer register work
					IoReg_SetBits(VGIP_Data_Path_Select_reg, _BIT3);
					set_di_doublebuf_write_TV006(diflag, !m_3DDIFlag);
				}
        }
		scalerip_bypass(TRUE);
		rtd_pr_vbe_debug("display: %x, Not Enter DI \n", display);
	}


	di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
	di_db_reg_ctl_reg.db_apply = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, di_db_reg_ctl_reg.regValue);

	if(VIP_RPC_system_info_structure_table == NULL){
		rtd_pr_vbe_debug("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
	}else{
		VIP_RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.DI_info.DI_info_reverse_2 = 255;
}


}

#ifndef UT_flag
void fw_scalerip_set_deXC_XL(void)
{
	unsigned int MemStartAddr = 0, mem_status_check = 0;
	//unsigned char vip_src = fwif_vip_source_check(3, 0);
	//unsigned char i2r_index = 0;
	unsigned short display;
	display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

       if (display == SLR_MAIN_DISPLAY) {
		mem_status_check = drv_memory_DeXCXL_Memory_Alloc_Condition_Check();
		//mem_status_check = DeXCXL_Disable;	/* temporarily patch to disable de-xc for Demo */
		MemStartAddr = drv_memory_I_De_XC_DMA_Config(MemStartAddr, mem_status_check);
	}

	rtd_pr_vbe_info("fw_scalerip_set_deXC_XL, display=%d,\n", display);
	return;
}

void fw_scalerip_set_IEGSM(void)
{
	unsigned short display;
	display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	drvif_IESM_format_convert(display); //drvif_IESM_format_convert() have to before drvif_IESM_frontporch_set()

	if (display == SLR_MAIN_DISPLAY) {
		drvif_color_iEdgeSmooth_init();
		drvif_IESM_frontporch_set(0);
	}
}

//not close rtnr
void fw_scalerip_disable_onlyip(UINT8 display)
{
//	di_im_di_control_RBUS im_di_control_reg;

	drvif_scaler_vactive_end_irq(_DISABLE, display);//disable VGIP interrupt (for auto ma)
	drvif_scaler_vactive_sta_irq(_DISABLE, display);//disable VGIP interrupt (for auto ma)
	#if 0 // Use table base
	drvif_color_ma_auto_init(); // reset auto ma
	#else
	//fix me:vip
	//Scaler_SetMADI_INIT();
	#endif

	IoReg_BusyCheckRegBit(PPOVERLAY_DTG_pending_status_2_reg, _BIT0,0x1FFFF);

	if(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)== _CHANNEL1)
		IoReg_ClearBits(VGIP_Data_Path_Select_reg, _BIT10);
	else
		IoReg_ClearBits(VGIP_Data_Path_Select_reg, _BIT8);
#if 0	/* DI use rolling mode, can't disable DI*/
	// Close IP enable
	im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
	im_di_control_reg.ip_enable= 0;
	im_di_control_reg.write_enable_8 = 1;
	IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
#endif
	//IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, 0);
	//frank@0518 add for disable RTNR

	drvif_memory_free_block(MEMIDX_DI);
}



void fw_scalerip_disable_ip(unsigned char display)
{
	 //USER:LewisLee DATE:2014/02/07
        //When Disable DI, need reset Di chroma error
        //to prevent Vsync not correvt
     //fix me:vip
	//Scaler_SetMA_Chroma_Error(0);
	di_im_new_mcnr_control_RBUS	di_im_new_mcnr_control;
	di_db_reg_ctl_RBUS	di_db_reg_ctl_reg;
//	di_im_di_control_RBUS im_di_control_reg;

	drvif_scaler_vactive_end_irq(_DISABLE, display);//disable VGIP interrupt (for auto ma)
	drvif_scaler_vactive_sta_irq(_DISABLE, display);//disable VGIP interrupt (for auto ma)
	#if 0 // Use table base
	drvif_color_ma_auto_init(); // reset auto ma
	#else
	//fix me:vip
	//Scaler_SetMADI_INIT();
	#endif

//frank@1103 mark to reduce time
	//IoReg_BusyCheckRegBit(PPOVERLAY_DTG_pending_status_2_reg, _BIT0,0x1FFFF);
#ifdef CONFIG_I2RND_ENABLE
	if((display == _CHANNEL1)&&(check_Sub_I2rnd_done() == FALSE)&&(Scaler_I2rnd_get_timing_enable() == _ENABLE))
		IoReg_ClearBits(VGIP_Data_Path_Select_reg, _BIT10);
	else if (display == _CHANNEL2)
		IoReg_ClearBits(VGIP_Data_Path_Select_reg, _BIT8);
#else

	if(Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_CHANNEL)== _CHANNEL1)
		IoReg_ClearBits(VGIP_Data_Path_Select_reg, _BIT10);
	else
		IoReg_ClearBits(VGIP_Data_Path_Select_reg, _BIT8);
#endif
	di_im_new_mcnr_control.regValue	= IoReg_Read32(DI_IM_DI_RTNR_HMCNR_reg);
	di_im_new_mcnr_control.n_mcnr_new_en = 0;
	IoReg_Write32(DI_IM_DI_RTNR_HMCNR_reg, di_im_new_mcnr_control.regValue);

        //frank@1016[KTASKWBS-1860]When not enter smooth toggle flow, need set below control bit 1
	di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
	di_db_reg_ctl_reg.sm_tog_apply_disable = 1;
	IoReg_Write32(DI_db_reg_ctl_reg, di_db_reg_ctl_reg.regValue);
#ifdef CONFIG_I2RND_ENABLE
	if((display == _CHANNEL1)&&(check_Sub_I2rnd_done() == TRUE)&&(Scaler_I2rnd_get_timing_enable() == _ENABLE))
		;
	else
#endif
	{
#if 0 	/* DI use rolling mode, can't disable DI*/
		// Close IP enable
		im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
		im_di_control_reg.ip_enable= 0;
		im_di_control_reg.write_enable_8 = 1;
		IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
#endif
	}
	//IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, 0);
//fix me:vip
#if 0
	//frank@0518 add for disable RTNR
	drvif_color_rtnr_off(display);
#endif

	drvif_memory_free_block(MEMIDX_DI);
}


#if 0
/*============================================================================*/
/**
* fw_scalerip_set_di_for_MaincvtSub
* Set IP (video process) registers, includes its memory setting for MaincvtSub conversion mode
*
* @param <info> { display-info struecture }
* @return { none }
*
*/

void fw_scalerip_set_di_for_MaincvtSub(unsigned char diEnable, unsigned char rtnrEnable)
{
	unsigned short temp;
	unsigned char m_3DDIFlag = 0;
	unsigned char diflag = 0;
	vgip_data_path_select_RBUS data_path_select_reg;
	di_im_di_active_window_control_RBUS	 reg_Active_Win_di_Reg;
	di_im_di_weave_window_control_RBUS im_di_weave_window_control_reg;
	unsigned short vgip2Display = SLR_MAIN_DISPLAY;
	unsigned short vgip2Channel = _CHANNEL2;
	unsigned char srcFrom = scaler_MaincvtSub_get_vgip2SrcFrom();
	unsigned int vgip2Height = scaler_MaincvtSub_get_vgip2IvHeight();
	unsigned int dispIHCount = scaler_MaincvtSub_get_vgip2IhCount();
	unsigned int vgip2Width =scaler_MaincvtSub_get_vgip2IhWidth();

	if (diEnable) {	// will added in future
		// DI 8 or 10 bits by LearnRPG
		di_color_recovery_option_RBUS Color_Recovery_Option_Reg;
		Color_Recovery_Option_Reg.regValue = IoReg_Read32(DI_Color_Recovery_Option_reg);
		if(fw_scalerip_get_DI_chroma_10bits())
		{
			Color_Recovery_Option_Reg.y10bitsin = 1;
			if(fw_scalerip_get_DI_chroma_10bits())
				Color_Recovery_Option_Reg.c10bitsin= 1;
			else
				Color_Recovery_Option_Reg.c10bitsin= 0;
		}
		else
		{
			Color_Recovery_Option_Reg.y10bitsin = 0;
			Color_Recovery_Option_Reg.c10bitsin= 0;
		}
		IoReg_Write32(DI_Color_Recovery_Option_reg, Color_Recovery_Option_Reg.regValue);
		// END

		// move to modestate_set_display_on()
		drvif_scaler_vactive_end_irq(_DISABLE, vgip2Channel);//disable VGIP ISR (for automa)
		drvif_scaler_vactive_sta_irq(_DISABLE, vgip2Channel);//disable VGIP ISR (for automa)
	        //forster modified 071122
		if (srcFrom == _SRC_FROM_ADC) {
			IoReg_Mask32(DDS_PLL_PLL_CTRL_VADDR, ~(_BIT9 | _BIT10), _BIT9 | _BIT10);

  			IoReg_SetBits(ADC_ADC_CLOCK_VADDR, _BIT21); // 1x ADC clock invert

			rtd_pr_vbe_debug("\n [2D3D] Set dual ADC clock%c",'!');
		}

		// Disable IP temp
		//frank@0817 sync LV to speed up switch channel
		//fw_scaler_double_buffer_maskl((SCALER_DISP_CHANNEL)display, DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);
		//fw_scaler_double_buffer_maskl((SCALER_DISP_CHANNEL)display,DI_IM_DI_CONTROL_reg, ~_BIT28, 0);
		//IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);
		//IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT28, 0);
		IoReg_SetBits(VGIP_Data_Path_Select_reg, _BIT3);

		set_di_doublebuf_write(0, 1);


		data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
		data_path_select_reg.uzd2_in_sel = _ENABLE;
		data_path_select_reg.xcdi_in_sel = _CHANNEL2;
		IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);


		if(diEnable) {
			//if(Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID) >= 960) {	 //motion adaptive 3A
			if(vgip2Width >= 960) {   //sync from pacific
				drv_memory_set_ip_fifo(0);
				IoReg_ClearBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
				IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT4, _BIT4);
				Scaler_DispSetStatus((SCALER_DISP_CHANNEL)vgip2Display,SLR_DISP_MA3A,TRUE);
				rtd_pr_vbe_debug("\n[2D3D] Set DI Memory Main 3A(w=%d)\n", vgip2Width);
			}
			else {   //motion adaptive 5A or 3A
				// 5A
				drv_memory_set_ip_fifo(1);
				IoReg_SetBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
				IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT4, 0);
				Scaler_DispSetStatus((SCALER_DISP_CHANNEL)vgip2Display,SLR_DISP_MA3A,FALSE);
				rtd_pr_vbe_debug("\n[2D3D] Set DI Memory Main 5A(w=%d)\n", vgip2Width);
			}
		}
		else {
			drv_memory_set_ip_fifo(2);
			IoReg_ClearBits(DI_IM_DI_BTR_CONTROL_reg, _BIT9);
			IoReg_Mask32(DI_IM_DI_CONTROL_reg, ~_BIT4, 0);//20080610 weihauo for progressive, jaosn9.ccc
			rtd_pr_vbe_debug("\n[2D3D] Set RTNR Memory\n");
		}

		IoReg_ClearBits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT7);

		//enable 3ddi
		//IoReg_SetBits(VGIP_Data_Path_Select_reg, _BIT3);

		rtd_pr_vbe_debug("[2D3D] 1...3A,5A condition :force 3A=%x\n",IoReg_Read32(DI_IM_DI_CONTROL_reg));

		//frank@04262010 add below code avoid 3ddi write data to the address 0 let code haneg ++
		//frank@0817 sync LV to speed up switch channel
		//fw_scaler_double_buffer_maskl((SCALER_DISP_CHANNEL)display,DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);
		// IoReg_Mask32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, ~_BIT19, _BIT19);

		m_3DDIFlag = 1; // 3D DI
		//frank@0924 Diable DI arbitrator avoid enable 3D DI let system hang
		//if(m_3DDIFlag)
			//IoReg_ClearBits(BUS_TV_SB2_DCU2_ARB_CR1_VADDR, _BIT0);

		drvif_scaler_vactive_end_irq(_ENABLE, vgip2Channel);//disable VGIP ISR (for automa)
		drvif_scaler_vactive_sta_irq(_ENABLE, vgip2Channel);//disable VGIP ISR (for automa)

		// disable delete 4 line (Up 2 line and down 2 line) and calculate line

		im_di_weave_window_control_reg.regValue = IoReg_Read32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
		im_di_weave_window_control_reg.dellineen = _DISABLE;
		IoReg_Write32(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, im_di_weave_window_control_reg.regValue);

		// Set active size to normalizing size in order to avoid to conflict
		reg_Active_Win_di_Reg.regValue = IoReg_Read32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg);
		reg_Active_Win_di_Reg.hsize = vgip2Width;// Width;
		reg_Active_Win_di_Reg.vsize = vgip2Height; // Height;
		reg_Active_Win_di_Reg.hblksize = dispIHCount - reg_Active_Win_di_Reg.hsize; // Blank;
		IoReg_Write32(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg, reg_Active_Win_di_Reg.regValue);

		#if 0
		drvif_module_film_init(vgip2Display);
		#else
		Scaler_SetFilm_Setting();
		#endif

		drvif_color_intra_init();
		drvif_color_ma_init();
		drvif_color_auto_bal_init(vgip2Display, diEnable);//20090907 jason9.ccc add
		temp = (dispIHCount -vgip2Width)/8;

#ifdef CONFIG_DUAL_CHANNEL  // There are two channels
		IoReg_Mask32(SUB_VGIP_VGIP_CHN2_MISC_reg , 0xfff000ff, (temp & 0x00000fff) << 8);
#endif

		// Di read start must be set if turn on horizontal UZD

		scalerip_modify_rdstart(vgip2Channel, diEnable, rtnrEnable, FALSE);//scaler_2Dcvt3D_get_vgip2InHoriDecMode());

		// no matter RTNR flag, must check progressive mode again. by LearnRPG
		drvif_color_rtnr_progressive_check(diEnable, vgip2Channel);

		if(rtnrEnable) {//20080609 jason9.ccc add for RTNR
			drvif_color_rtnr_init(vgip2Channel);
		}
		else
			drvif_color_rtnr_off(vgip2Channel);

		diflag = 1;  // set di

		set_di_doublebuf_write(diflag, !m_3DDIFlag);

		//frank@0924 Enable DI arbitrator avoid enable 3D DI let system hang
		//if(m_3DDIFlag)
			//IoReg_ClearBits(BUS_TV_SB2_DCU2_ARB_CR1_VADDR, _BIT0);
	}
	else {
		set_di_doublebuf_write(FALSE, TRUE);
		IoReg_ClearBits(VGIP_Data_Path_Select_reg, _BIT8);
		//rtd_pr_vbe_debug("[2D3D] display: %x, Not Enter DI \n", vgip2Display);
	}
}
#endif
//Elsie 20140117
/*============================================================================*/
/**
* fw_scalerip_set_DI_chroma_10bits
* Turn on/off DI chroma 10-bit input
*
* @param <isEnable> { TRUE: enable DI chroma 10-bit input, FALSE: disable }
* @return { none }
*
*/
extern unsigned char get_AVD_Input_Source(void);

#endif// end of UT_flag

void fw_scalerip_set_DI_chroma_10bits(unsigned char isEnable)
{
	DI_chroma_10bits_in = isEnable;
}

/*============================================================================*/
/**
* fw_scalerip_set_DI_chroma_10bits
* Get whether DI chroma 10-bit input is turned on
*
* @param <none>
* @return { DI_chroma_10bits_in } { TRUE: DI chroma 10-bit input is enabled, FALSE: DI chroma 10-bit input is disabled }
*
*/
unsigned char fw_scalerip_get_DI_chroma_10bits(void)
{
	return DI_chroma_10bits_in;
}

#ifndef UT_flag

/*
void drvif_color_rtnr_progressive_check(unsigned char interlace_flag, unsigned char channel)
{

//Set RTNR Progressive mode:
	di_im_di_rtnr_control_RBUS RecursiveTemporalNRCtrl_reg;
	RecursiveTemporalNRCtrl_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_VADDR);

	//frank@0518 mark this to avoid sub channel into DI fail
	//if( channel==_CHANNEL1)
	{
		if(!interlace_flag)
			RecursiveTemporalNRCtrl_reg.cp_rtnr_progressive = 0x1;
		else
			RecursiveTemporalNRCtrl_reg.cp_rtnr_progressive = 0x0;

		IoReg_Write32( DI_IM_DI_RTNR_CONTROL_VADDR,RecursiveTemporalNRCtrl_reg.regValue);
	}

}
*/
void fw_scalerip_set_di_gamemode_setting(unsigned char isEnable)
{
	di_color_recovery_option_RBUS Color_Recovery_Option_Reg;
	di_im_di_control_RBUS im_di_control_reg;
	unsigned short display ;

	unsigned char source=255;
	source = fwif_vip_source_check(3, 0);
	display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
	Color_Recovery_Option_Reg.regValue = IoReg_Read32(DI_Color_Recovery_Option_reg);
	if((DI_i2r_db_reg_ctl_get_reg_sm_size_change_hw_detect_en(IoReg_Read32(DI_i2r_db_reg_ctl_reg)) == 1)&&(Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_THRIP))){
		if((isEnable == 1)&&(source != VIP_QUALITY_CVBS_SECAN)) {
				im_di_control_reg.ma_3aenable = 1;
				im_di_control_reg.write_enable_1 = 1;
				IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
				IoReg_SetBits(DI_db_reg_ctl_reg, _BIT0);
		       Color_Recovery_Option_Reg.game_mode_3a = 1;
		} else {
				if((get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY) == 0) &&	/*sync adaptivestream condition ==> the same with soomth toggle, for internet i content(itv, Q-12309 case)*/
					(Scaler_DispGetInputInfo(SLR_INPUT_DI_WID)<=960)){ // 5A
					im_di_control_reg.ma_3aenable = 0;
					im_di_control_reg.write_enable_1 = 1;
					IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
					IoReg_SetBits(DI_db_reg_ctl_reg, _BIT0);
				}else{
					im_di_control_reg.ma_3aenable = 1;
					im_di_control_reg.write_enable_1 = 1;
					IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
					IoReg_SetBits(DI_db_reg_ctl_reg, _BIT0);
				}

		       Color_Recovery_Option_Reg.game_mode_3a = 0;
		}
		IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);
		IoReg_Write32(DI_Color_Recovery_Option_reg, Color_Recovery_Option_Reg.regValue);

	}else{
		Color_Recovery_Option_Reg.game_mode_3a = 0;
		IoReg_Write32(DI_Color_Recovery_Option_reg, Color_Recovery_Option_Reg.regValue);
	}
	

}

void fw_scalerip_set_di_gamemode(unsigned char isEnable)
{
	DI_Gamemode_EN = isEnable;
}

unsigned char fw_scalerip_get_di_gamemode(void)
{
	unsigned char source=255;
	source = fwif_vip_source_check(3, 0);

	/*factory self diagnosis grab pixel from third buffer,so not support single buffer mode*/
	if(Get_Factory_SelfDiagnosis_Mode()) {
		return FALSE;
	}
	/*Only support HDMI and Component*/
//	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_AVD) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_ADC) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_HDMI))
	if(!((Get_DisplayMode_Src(SLR_MAIN_DISPLAY)==VSC_INPUTSRC_ADC)||(Get_DisplayMode_Src(SLR_MAIN_DISPLAY)==VSC_INPUTSRC_HDMI)||((Get_DisplayMode_Src(SLR_MAIN_DISPLAY)==VSC_INPUTSRC_AVD)&&(get_AVD_Input_Source()!=_SRC_TV)&&(source != VIP_QUALITY_CVBS_SECAN))))

		return FALSE;

	return DI_Gamemode_EN;
}

/*======================== End of File =======================================*/

void check_di_buffer_mode(void)
{//This is for whether DI mode setting is worng. If wrong, recover the right setting. Recover API
	unsigned char DI_3a_flag ;
	di_im_di_control_RBUS im_di_control_reg;
	di_db_reg_ctl_RBUS di_db_reg_ctl_reg;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);

	DI_3a_flag = im_di_control_reg.ma_3aenable;

	if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP))
	{

		IoReg_ClearBits(DI_db_reg_ctl_reg, _BIT2);//Let read value be HW setting
		im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);

		if(im_di_control_reg.ma_3aenable != DI_3a_flag)
		{
			rtd_pr_vbe_notice("\r\n#####DI buffer mode need recover(%d)####\r\n", DI_3a_flag);
			down(get_DI_semaphore());
			if(DI_3a_flag) {
				im_di_control_reg.ma_3aenable = 1;
				im_di_control_reg.write_enable_1 = 1;
				IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);

			} else {
				im_di_control_reg.ma_3aenable = 0;
				im_di_control_reg.write_enable_1 = 1;
				IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);

			}



			up(get_DI_semaphore());
		}
		IoReg_SetBits(DI_db_reg_ctl_reg, _BIT2);//Let read value be rbus setting
		/* double buffer*/

		di_db_reg_ctl_reg.regValue = IoReg_Read32(DI_db_reg_ctl_reg);
		di_db_reg_ctl_reg.db_apply = 1;
		IoReg_Write32(DI_db_reg_ctl_reg, di_db_reg_ctl_reg.regValue);

		if(VIP_RPC_system_info_structure_table == NULL){
			rtd_pr_vbe_debug("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		}else{
			VIP_RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.DI_info.DI_ma_3aenable = im_di_control_reg.ma_3aenable;
		}

	}
}

void fw_scalerip_set_di_size_change_hw_detect(unsigned char enable)
{
	//DI default size setting for sub start address sample issue @Crixus 20160725
	di_i2r_db_reg_ctl_RBUS di_i2r_db_reg_ctl_reg;
	di_i2r_db_reg_ctl_reg.regValue = IoReg_Read32(DI_i2r_db_reg_ctl_reg);
	di_i2r_db_reg_ctl_reg.reg_sm_size_change_hw_detect_en = enable;
	IoReg_Write32(DI_i2r_db_reg_ctl_reg, di_i2r_db_reg_ctl_reg.regValue);
}

#endif //UT_flag

void set_di_wait_frame_num(unsigned char num)
{
	DI_wait_frame_num = num;
}

#ifndef UT_flag
void wait_DI_ready(void)
{//this api is for wait DI buffer ready
#if 1
	long counter = 0;
	const long timeout = 100;

	unsigned char di_status = Get_DI_RTNR_DMA_force2D_Status();

	while ((di_status == 1) && (counter < timeout)) {
		usleep_range(500, 1000);
		di_status = Get_DI_RTNR_DMA_force2D_Status();
		++counter;
	}
	rtd_pr_vbe_info("[vsc_profile][%s] wait di reset done %d(%ld)\n", __func__, di_status, counter);
#else
	unsigned int cur_counter;
	unsigned int need_time, vfreq;
	unsigned char timeout = 10;

	if(DI_wait_frame_num != 0)
	{
		vfreq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);

		if(vfreq)
			need_time = 900000 * DI_wait_frame_num / vfreq;
		else
		{
			rtd_pr_vbe_notice("\r\n#####func:%s err frame rate zero####\r\n", __FUNCTION__);
			return;
		}

		rtd_pr_vbe_notice("\r\n###func:%s ori:0x%x needwait num:%d time:0x%x#####\r\n", __FUNCTION__, record_timer_counter, DI_wait_frame_num, need_time);

		while(timeout)
		{
			cur_counter = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			if(cur_counter < record_timer_counter)
			{
				if((cur_counter + (0xFFFFFFFF - record_timer_counter)) >= need_time)
					break;
			}
			else if ((cur_counter - record_timer_counter) >= need_time)
					break;
			msleep(10);
			timeout --;
		}
		if(!timeout)
			rtd_pr_vbe_notice("\r\n#####func:%s err wait timeout#####\r\n", __FUNCTION__);
	}
#endif

	if (DI_i2r_db_reg_ctl_get_reg_sm_size_change_sw_detect_en(IoReg_Read32(DI_i2r_db_reg_ctl_reg)) == 1)
		IoReg_ClearBits(DI_i2r_db_reg_ctl_reg, DI_i2r_db_reg_ctl_reg_sm_size_change_sw_detect_en_mask);
}

#endif// end of UT_flag

void di_rtnr_demode_ctrl(void)
{//current digital mode need to set this
	di_im_di_no_hs_mode_RBUS di_im_di_no_hs_mode_reg;
	di_im_di_no_hs_mode_reg.regValue = IoReg_Read32(DI_IM_DI_NO_HS_MODE_reg);
	di_im_di_no_hs_mode_reg.hsd_in_hsize = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE);
	di_im_di_no_hs_mode_reg.hsd_in_vsize = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN_PRE);
	//if((Scaler_InputSrcGetMainChType() == _SRC_VO) || (Scaler_InputSrcGetMainChType() == _SRC_HDMI))
	{//digital mode
		di_im_di_no_hs_mode_reg.no_hs_mode = 1;
	}
	//else
	//{
	//	di_im_di_no_hs_mode_reg.no_hs_mode = 0;
	//}
	IoReg_Write32(DI_IM_DI_NO_HS_MODE_reg, di_im_di_no_hs_mode_reg.regValue);			

}
/**
  * @}
  */

void fw_scalerip_set_SPNR_size(void)
{
	nr_spnr_act_length_RBUS spnr_act_length_reg;
	nr_spnr_act_width_RBUS spnr_act_width_reg;
	unsigned int height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	unsigned int width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);

	spnr_act_length_reg.regValue = IoReg_Read32(NR_SPNR_ACT_Length_reg);
	spnr_act_width_reg.regValue = IoReg_Read32(NR_SPNR_ACT_Width_reg);

	spnr_act_length_reg.spnr_image_height = height;
	spnr_act_width_reg.spnr_image_width = width;

	IoReg_Write32(NR_SPNR_ACT_Length_reg, spnr_act_length_reg.regValue);
	IoReg_Write32(NR_SPNR_ACT_Width_reg, spnr_act_width_reg.regValue);
}


