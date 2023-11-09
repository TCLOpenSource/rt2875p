/*
   comment
//20140930should fix rpc related part

*/
#include <linux/kernel.h>
#include <linux/semaphore.h>

#include <linux/string.h>

#ifdef CONFIG_ARM64
#include <asm/io.h>
#endif

#include <mach/platform.h>
//#include <mach/io.h>
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle_lib.h>
#include <tvscalercontrol/panel/panelapi.h>


#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/scalerDrvCommon.h>//rpc SCALERIOC_SENDPANELPARAMTERINFO
#else
#include <scalercommon/scalerCommon.h>
	#include <scalercommon/scalerDrvCommon.h>//rpc SCALERIOC_SENDPANELPARAMTERINFO
#endif


#include "tvscalercontrol/scalerdrv/scalerdrv.h"


#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle.h>

#include <rbus/vodma_reg.h>
//#include <rbus/pst_i2rnd_reg.h>
#include <tvscalercontrol/vo/rtk_vo.h>
#include "rbus/vodma2_reg.h"
#include <rbus/vgip_reg.h>
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
#include <tvscalercontrol/scalerdrv/scaler_i2rnd.h>
#include <tvscalercontrol/scalerdrv/scaler_pst.h>
#include <scaler_vscdev.h>

// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#include <rtd_log/rtd_module_log.h>



#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))

static unsigned char need_send_panel_para_flag = TRUE;

static unsigned char enable_smooth_toggle_vo_changeratio[MAX_DISP_CHANNEL_NUM];

//---vopart
//static unsigned char enable_smooth_toggle_vo=0;
//static unsigned char enable_smooth_toggle_zoom_changeratio=0;//rika 20140829 added for tcl api
static unsigned int zoom_smoothtoggle_vo_hwid=0;//rika 20140919 added
static unsigned int zoom_smoothtoggle_vo_vlen=0;
static unsigned char zoom_reset_double_state[2]={_DISABLE};//rika20141125; for vo flow to disable double buffer later

//---other part
static UINT8 Scaler_FRC_Open_Fix_Lastline_DHtotal_Adjust_Enable_Flag = _DISABLE;
static unsigned char MainSubonPRFuc = FALSE;


//----define
#define Rt_Sleep(mSec)					msleep(mSec)
#define Rt_Delay(mSec)					msleep(mSec)

#ifdef CONFIG_I2RND_ENABLE
extern unsigned char vsc_i2rnd_dualvo_sub_stage;
extern unsigned char vsc_i2rnd_sub_stage;
#endif



void Set_send_panel_para_flag(unsigned char flag)
{
	need_send_panel_para_flag = flag;
}
#if 1
//----
//----------begin of api
void Set_vo_smoothtoggle_changeratio_flag2(unsigned char enable, unsigned char display)
{
	/*
	   for fpp tcl api  : added by rika 20140714
	   */
	///enable_smooth_toggle_vo_changeratio=enable;//sirius version
	enable_smooth_toggle_vo_changeratio[display]=enable;

}
#endif
void set_zoom_smoothtoggle_vo_hwid(unsigned int hwid)
{//added for tcl api
	zoom_smoothtoggle_vo_hwid=hwid;

}
unsigned int get_zoom_smoothtoggle_vo_hwid(unsigned char display)
{//added for tcl api
	if(zoom_smoothtoggle_vo_hwid)
	{
		return zoom_smoothtoggle_vo_hwid;
	}
	else
	{
		return Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_IPH_ACT_WID_PRE);
	}

}
void set_zoom_smoothtoggle_vo_vlen(unsigned int vlen)
{//added for tcl api
	zoom_smoothtoggle_vo_vlen=vlen;

}
unsigned int get_zoom_smoothtoggle_vo_vlen(unsigned char display)
{//added for tcl api
	if(zoom_smoothtoggle_vo_vlen)
	{
		return zoom_smoothtoggle_vo_vlen;
	}
	else
	{
		return Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_IPV_ACT_LEN_PRE);
	}
}

void clear_imd_smooth_toggle_clk_time2switch_en(unsigned char display)
{
	if(display==SLR_MAIN_DISPLAY){
		vgip_smooth_tog_ctrl_RBUS smooth_tog_ctrl_reg;
		smooth_tog_ctrl_reg.regValue = rtd_inl(VGIP_Smooth_tog_ctrl_reg);
		if(smooth_tog_ctrl_reg.main_imd_smooth_toggle_clk_time2switch_en){
			smooth_tog_ctrl_reg.main_imd_smooth_toggle_clk_time2switch_en = 0; // need reset this register when run scaler
			rtd_outl(VGIP_Smooth_tog_ctrl_reg,smooth_tog_ctrl_reg.regValue);
		}
	}
}

extern struct semaphore* get_double_buffer_semaphore(void);
void set_zoom_reset_double_state(unsigned char enable,unsigned char display)
{
	unsigned int plane = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_VODMA_PLANE);
	zoom_reset_double_state[display]=enable;
	rtd_pr_smt_debug("_rk reset display:%d zoom_double_state later:%d\n",display,zoom_reset_double_state[display]);
	if (enable == _DISABLE) {
		Check_smooth_toggle_update_flag(display);
	}
	if(enable == _DISABLE)
	{
		down(get_double_buffer_semaphore());
		if(display==SLR_MAIN_DISPLAY)
			imd_smooth_enable_main_double_buffer(FALSE);
		else
			imd_smooth_enable_sub_double_buffer(FALSE);
		up(get_double_buffer_semaphore());

		if(display==SLR_MAIN_DISPLAY)
			clear_imd_smooth_toggle_clk_time2switch_en(display);
	}

	if((Scaler_InputSrcGetType(display) == _SRC_VO)&&(display==SLR_MAIN_DISPLAY))
	{
		if(VO_VIDEO_PLANE_V1 == plane)
		{
			vodma_vodma_smooth_toggle_RBUS vodma_smooth_toggle_reg;
			vodma_smooth_toggle_reg.regValue=rtd_inl(VODMA_VODMA_SMOOTH_TOGGLE_reg);
			if(enable==_DISABLE)
			{
				if(vodma_smooth_toggle_reg.smooth_en==1){
					vodma_smooth_toggle_reg.smooth_en=0;
					rtd_outl(VODMA_VODMA_SMOOTH_TOGGLE_reg,vodma_smooth_toggle_reg.regValue);
				}
			}
		}
		else if(VO_VIDEO_PLANE_V2 == plane)
		{
			vodma2_vodma2_smooth_toggle_RBUS vodma2_smooth_toggle_reg;
			vodma2_smooth_toggle_reg.regValue=rtd_inl(VODMA2_VODMA2_SMOOTH_TOGGLE_reg);
			if(enable==_DISABLE)
			{
				if(vodma2_smooth_toggle_reg.smooth_en==1){
					vodma2_smooth_toggle_reg.smooth_en=0;
					rtd_outl(VODMA2_VODMA2_SMOOTH_TOGGLE_reg,vodma2_smooth_toggle_reg.regValue);
				}
			}
		}
	}
}

unsigned char get_zoom_reset_double_state(unsigned char display)
{
	return zoom_reset_double_state[display];
 }

//other part

void set_mainsubonpr_fuc(unsigned char value)
{
	MainSubonPRFuc = value;
}

unsigned char get_mainsubonpr_fuc(void)
{
	return MainSubonPRFuc;
}
UINT8 Scaler_Get_EnterFRC_LastLine_DHtotal_Adjust_Enable_Flag(void)
{
	return Scaler_FRC_Open_Fix_Lastline_DHtotal_Adjust_Enable_Flag;
}

void Scaler_Set_EnterFRC_LastLine_DHtotal_Adjust_Enable_Flag(UINT8 ucEnable)
{
	Scaler_FRC_Open_Fix_Lastline_DHtotal_Adjust_Enable_Flag = ucEnable;
}


// Check which VO is for the 2D main video
ENUM_TVD_INPUT_FORMAT Scaler_Get_2D_VodmaIndex(void)
{//fix me rika 20141022
	ENUM_TVD_INPUT_FORMAT inPutFmt=TVD_INFMT_VODMA0;

	// [Magellan] VO output select
	//  - VODMA2: for 2D video output(DTV/Playback/Photo)
	//  - VODMA1: for 3D video or PIP application

	// change this VO index define need update CVideoOutFilter::SetVideoOutLayer() too.
#if defined(IS_MAGELLAN_ONLY) && defined(ENABLE_DRIVER_I3DDMA) // VO1 for scaler 3D
	if(Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE())
	{
		static unsigned char lastInPutFmt=0;
#ifdef ENABLE_USE_VO_UZU // [VO UZU] VO in VO1 for UZU funciton (bypass IDMA)
		if(Scaler_Get_forceEnable_VO_UZU())
			inPutFmt = TVD_INFMT_VODMA0;
		else
#endif
			inPutFmt = TVD_INFMT_VODMA1;

		if(inPutFmt != lastInPutFmt)
			rtd_pr_smt_debug("[VO] Force to VO Plane[%d]\n", (inPutFmt == TVD_INFMT_VODMA1? 1: 0));
		lastInPutFmt = inPutFmt;
	}
#endif

	return inPutFmt;
}

// Check which VO is for the 2D main video
VO_VIDEO_PLANE Scaler_Get_2D_VoPlane(void)
{//fix me rika 20141022
	if(Scaler_Get_2D_VodmaIndex() == TVD_INFMT_VODMA1)
		return VO_VIDEO_PLANE_V2;
	else
		return VO_VIDEO_PLANE_V1;
}


// Check which VO is for the 2D main video
VO_VIDEO_PLANE Scaler_Get_3D_VoPlane(void)
{ //fix me rika 20141022
	return VO_VIDEO_PLANE_V1;
}

SOURCE_INFO_SMOOTH_TOGGLE vo_source_state_smooth_toggle[MAX_DISP_CHANNEL_NUM];
static unsigned char enable_smooth_toggle_vo_timing_change[MAX_DISP_CHANNEL_NUM];
//DISPLAY_DATA_SMOOTH_TOGGLE vo_source_force_data_mode[MAX_DISP_CHANNEL_NUM];
extern void drvif_color_Set_DRV_SNR_Clock(SCALER_DISP_CHANNEL display, unsigned char En_flag);
//static unsigned char vo_smooth_toggle_start_scaler;
void Set_direct_vo_smoothtoggle_enable(unsigned char enable,/*DISPLAY_DATA_SMOOTH_TOGGLE data_pach,*/ unsigned char display,unsigned char plane)
{
#ifndef CONFIG_DUAL_CHANNEL
	display=SLR_MAIN_DISPLAY;
#endif
//	unsigned int plane = get_vo_plane_flag(display);

	if(enable)
	{
		//vo_force_data_mode_set_enable(data_pach, display);
	//	vo_smoothtoggle_memory_alloc(display);
		enable_smooth_toggle_vo_timing_change[display]=enable;
		rtd_pr_smt_debug("Set_direct_vo_smoothtoggle_start=true\n");
#ifdef NEW_FLL_METHOD
		//When video playback preview, it may cause fll unlock when change file;
		// so we disable fll check under this case.
		// It can enable again, when display on finish.
		framesync_new_fixlastline_threashold_check_enable(0);
#endif
		set_vo_smoothtoggle_state(SLR_SMOOTH_TOGGLE_INIT, display,plane);
//		panel_parameter_send_info();
	}
	else
	{
		enable_smooth_toggle_vo_timing_change[display]=enable;
		rtd_pr_smt_debug("close vo timing change smooth toggle \n");
		set_vo_open_smooth_toggle_enable(0,display);
		set_vo_run_smooth_toggle_enable(0,display);
		set_vo_smoothtoggle_state(SLR_SMOOTH_TOGGLE_NONE, display,plane);
		smooth_toggle_state_info_rpc(display);
        // Disable VO empty gate before change s1_clk_en status
        IoReg_Mask32(VODMA_VODMA_CLKGEN_reg, ~(_BIT3), 0);
#ifdef CONFIG_I2RND_ENABLE
		//Eric@20171019 Dual vo i2rnd do not change snr clock setting when rerun main scaler path
		if(!(Scaler_I2rnd_get_timing_enable() && ((vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1) || (vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC))))
#endif
		// [KTASKWBS-3853] disable SDNR clock before disable main_imd_smooth_toggle_clk_time2switch_en
		drvif_color_Set_DRV_SNR_Clock(display, 0);
		clear_imd_smooth_toggle_clk_time2switch_en(display);
		if(VO_VIDEO_PLANE_V1 == plane)
		{
			vodma_vodma_smooth_toggle_RBUS vodma_smooth_toggle_reg;
			vodma_smooth_toggle_reg.regValue=IoReg_Read32(VODMA_VODMA_SMOOTH_TOGGLE_reg);
			if(vodma_smooth_toggle_reg.smooth_en==1)
			{
				vodma_smooth_toggle_reg.smooth_en=0;
				IoReg_Write32(VODMA_VODMA_SMOOTH_TOGGLE_reg,vodma_smooth_toggle_reg.regValue);
			}
		}
		else if(VO_VIDEO_PLANE_V2 == plane)
		{
			vodma2_vodma2_smooth_toggle_RBUS vodma2_smooth_toggle_reg;
			vodma2_smooth_toggle_reg.regValue=IoReg_Read32(VODMA2_VODMA2_SMOOTH_TOGGLE_reg);
			if(vodma2_smooth_toggle_reg.smooth_en==1)
			{
				vodma2_smooth_toggle_reg.smooth_en=0;
				IoReg_Write32(VODMA2_VODMA2_SMOOTH_TOGGLE_reg,vodma2_smooth_toggle_reg.regValue);
			}
		}

		//vo_force_data_mode_set_enable(SLR_DISPLAY_DEFAULT, display);
	}
}

unsigned char Get_vo_smoothtoggle_timingchange_flag(unsigned char display)
{
	return enable_smooth_toggle_vo_timing_change[display];
}
#if 0
unsigned char Get_vo_smoothtoggle_changeratio_flag(unsigned char display)
{
	return enable_smooth_toggle_vo_changeratio[display];
}
#endif
void set_vo_smoothtoggle_state(SOURCE_INFO_SMOOTH_TOGGLE state, unsigned char display,unsigned char plane)
{
#ifndef CONFIG_DUAL_CHANNEL
	display=SLR_MAIN_DISPLAY;
#endif
//	unsigned int plane = get_vo_plane_flag(display);
	vo_source_state_smooth_toggle[display] = state;
	if(VO_VIDEO_PLANE_V1 == plane)
	{
		vodma_vodma_smooth_toggle_RBUS vodma_smooth_toggle_reg;
		vodma_vodma_pvs_free_RBUS vodma_pvs_free_reg;
		vodma_pvs_free_reg.regValue = rtd_inl(VODMA_VODMA_PVS_Free_reg);
		vodma_smooth_toggle_reg.regValue = rtd_inl(VODMA_VODMA_SMOOTH_TOGGLE_reg);
		if (vo_source_state_smooth_toggle[display] != SLR_SMOOTH_TOGGLE_ACTIVE) {
			down(get_double_buffer_semaphore());
		#ifdef CONFIG_DUAL_CHANNEL
			if (display) {
				imd_smooth_enable_sub_double_buffer(FALSE);
			} else
		#endif
			{
				imd_smooth_enable_main_double_buffer(FALSE);
#if 0 // [KTASKWBS-3853] disable SDNR clock before disable main_imd_smooth_toggle_clk_time2switch_en
				clear_imd_smooth_toggle_clk_time2switch_en(display);
#endif
			}
			up(get_double_buffer_semaphore());
			
			if (vodma_smooth_toggle_reg.smooth_en == 1) {
				vodma_smooth_toggle_reg.smooth_en = 0;
				rtd_outl(VODMA_VODMA_SMOOTH_TOGGLE_reg, vodma_smooth_toggle_reg.regValue);
				vodma_pvs_free_reg.pvs0_free_period_update = 0;
				rtd_outl(VODMA_VODMA_PVS_Free_reg, vodma_pvs_free_reg.regValue);
			}
		}
	}
	else if(VO_VIDEO_PLANE_V2 == plane)
	{
		vodma2_vodma2_smooth_toggle_RBUS vodma2_smooth_toggle_reg;
		vodma2_vodma2_pvs_free_RBUS vodma2_pvs_free_reg;
		vodma2_pvs_free_reg.regValue = rtd_inl(VODMA2_VODMA2_PVS_Free_reg);
		vodma2_smooth_toggle_reg.regValue = rtd_inl(VODMA2_VODMA2_SMOOTH_TOGGLE_reg);
		if (vo_source_state_smooth_toggle[display] != SLR_SMOOTH_TOGGLE_ACTIVE) {
			down(get_double_buffer_semaphore());
		#ifdef CONFIG_DUAL_CHANNEL
			if (display) {
				imd_smooth_enable_sub_double_buffer(FALSE);
			} else
		#endif
			{
				imd_smooth_enable_main_double_buffer(FALSE);
				clear_imd_smooth_toggle_clk_time2switch_en(display);
			}
			up(get_double_buffer_semaphore());
			if (vodma2_smooth_toggle_reg.smooth_en == 1) {
				vodma2_smooth_toggle_reg.smooth_en = 0;
				rtd_outl(VODMA2_VODMA2_SMOOTH_TOGGLE_reg, vodma2_smooth_toggle_reg.regValue);
				vodma2_pvs_free_reg.pvs0_free_period_update = 0;
				rtd_outl(VODMA2_VODMA2_PVS_Free_reg, vodma2_pvs_free_reg.regValue);
			}
		}
	}
}
SOURCE_INFO_SMOOTH_TOGGLE get_vo_smoothtoggle_state(unsigned char display)
{
	return vo_source_state_smooth_toggle[display];
}

/*void vo_force_data_mode_set_enable(DISPLAY_DATA_SMOOTH_TOGGLE enable, unsigned char display)
{
#ifndef CONFIG_DUAL_CHANNEL
	display = SLR_MAIN_DISPLAY;
#endif
	unsigned int *pulDataTemp = NULL;
	unsigned int ulCount = 0, i = 0;
	SMOOTH_TOGGLE_DATA_MODE data_mode_info={display,enable};
	int ret = 0;
	vo_source_force_data_mode[display] = enable;

	pulDataTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_SENDSCALERDATAMODE);
	ulCount = sizeof(SMOOTH_TOGGLE_DATA_MODE) / sizeof(unsigned int);
	// copy to RPC share memory
	#ifdef CONFIG_ARM64
	memcpy_toio(pulDataTemp, &data_mode_info, sizeof(SMOOTH_TOGGLE_DATA_MODE));
	#else
	memcpy(pulDataTemp, &data_mode_info, sizeof(SMOOTH_TOGGLE_DATA_MODE));
	#endif

	for (i = 0; i < ulCount; i++)
		pulDataTemp[i] = htonl(pulDataTemp[i]);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SENDSCALERDATAMODE,0,0)))
		rtd_pr_smt_debug("ret=%d, send SCALERIOC_SENDSCALERDATAMODE to driver fail !!!\n", ret );
	rtd_pr_smt_debug("vo_force_data_mode_set_enable[display:%d]=%d\n",display,enable);
}
DISPLAY_DATA_SMOOTH_TOGGLE vo_force_data_mode_get_enable(unsigned char display)
{
	return vo_source_force_data_mode[display];
}
*/

void panel_parameter_send_info(void)
{
	Panel_Parameter_INFO parameter_info;
	unsigned int *pulDataTemp = NULL;
	unsigned int ulCount = 0, i = 0;
	int ret = 0;
	if(!need_send_panel_para_flag) {//No need to send parameter again
		rtd_pr_smt_debug("No need to send panel parameter!!!\n");
		return;
	} else {
		need_send_panel_para_flag = FALSE;
	}
	parameter_info.disp_h_total=Get_DISP_HORIZONTAL_TOTAL();
	parameter_info.disp_v_total=Get_DISP_VERTICAL_TOTAL();
	parameter_info.disp_clock_max=Get_DISPLAY_CLOCK_MAX();
	parameter_info.disp_clock_min=Get_DISPLAY_CLOCK_MIN();
	parameter_info.disp_clock_typical=Get_DISPLAY_CLOCK_TYPICAL();
	parameter_info.disp_ratio_4x3=Get_DISPLAY_RATIO_4X3();
	parameter_info.disp_den_start_vpos=Get_DISP_DEN_STA_VPOS();
	parameter_info.disp_act_start_vpos=Get_DISP_ACT_STA_VPOS();
	parameter_info.disp_act_end_vpos=Get_DISP_ACT_END_VPOS();
	parameter_info.disp_act_start_hpos=Get_DISP_ACT_STA_HPOS();
	parameter_info.disp_act_end_hpos=Get_DISP_ACT_END_HPOS();
	parameter_info.panel_custom_index=Get_DISPLAY_PANEL_CUSTOM_INDEX();
	parameter_info.panel_type=Get_DISPLAY_PANEL_TYPE();
	parameter_info.DISPLAY_PORT =Get_DISPLAY_PORT();
	parameter_info.DISPLAY_REFRESH_RATE =Get_DISPLAY_REFRESH_RATE();
	parameter_info.panel_support_3d_display=Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE();
	parameter_info.panel_3d_line_alternative_support=Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE();
	parameter_info.panel_3d_PR_outout_LR_swap=Get_PANEL_3D_PR_OUTPUT_LR_SWAP();
	parameter_info.panel_3d_SG_output_120HZ=Get_PANEL_3D_SG_OUTPUT_120HZ_ENABLE();
	parameter_info.panel_3d_SG_24HZ_output_FHD=Get_PANEL_3D_SG_24HZ_OUTPUT_FHD_ENABLE();
	parameter_info.panel_3d_PR_to_SG_output_FRC=Get_PANEL_3D_PR_TO_SG_OUTPUT_FRC_ENABLE();
	parameter_info.lvds_converter_to_HDMI=0;
	parameter_info.ivflip = Get_PANEL_VFLIP_ENABLE();

	if (get_panel_res() == PANEL_RES_FHD) {
		rtd_pr_smt_notice("set specific panel index to video fw for k6lp fhd panel\n");
		parameter_info.specific_panel_index = SPECIFIC_PANEL_INDEX_LGD_FHD;
	}else if(get_panel_res() == PANEL_RES_HD){
		rtd_pr_smt_notice("set specific panel index to video fw for k6lp hd panel\n");
		parameter_info.specific_panel_index = SPECIFIC_PANEL_INDEX_LGD_HD;
	}
	else {
		parameter_info.specific_panel_index = 0;
	}

	pulDataTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_SENDPANELPARAMTERINFO);
	ulCount = sizeof(Panel_Parameter_INFO) / sizeof(unsigned int);
	// copy to RPC share memory
	#ifdef CONFIG_ARM64
	memcpy_toio(pulDataTemp, &parameter_info, sizeof(Panel_Parameter_INFO));
	#else
	memcpy(pulDataTemp, &parameter_info, sizeof(Panel_Parameter_INFO));
	#endif

	for (i = 0; i < ulCount; i++)
		pulDataTemp[i] = htonl(pulDataTemp[i]);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SENDPANELPARAMTERINFO,0,0)))
	{
		rtd_pr_smt_debug("ret=%d, SCALERIOC_SENDPANELPARAMTERINFO to driver fail !!!\n", ret);
	}
}

extern StructSrcRect Scaler_SubDispWindowGet(void);
void smooth_toggle_state_info_rpc(unsigned char display)
{
	int ret = 0;
	SLR_RATIO_TYPE ratio_type = Scaler_DispGetRatioMode2();
	//StructSrcRect dispwin=Scaler_DispWindowGet();
	//StructSrcRect overscan_size = Scaler_AdaptiveStream_InputWindowGet();
	VO_Scaler_State scaler_info;
	unsigned int *pulDataTemp = NULL;
	unsigned int ulCount = 0, i = 0;
	StructSrcRect dispwin;
	StructSrcRect overscan_size;
	StructSrcRect hal_intputTiming;
	/*if (display == SLR_SUB_DISPLAY){
			rtk_hal_vsc_GetInputRegion(VIDEO_WID_1, &overscan_size);
			rtk_hal_vsc_GetOutputRegion(VIDEO_WID_1, &dispwin);
	} else {
			dispwin = Scaler_DispWindowGet();
			overscan_size = Scaler_AdaptiveStream_InputWindowGet();
	}*/
	if(SLR_MAIN_DISPLAY == display){
		dispwin = Scaler_DispWindowGet();
	}else{
		dispwin = Scaler_SubDispWindowGet();
	}
	overscan_size = get_input_for_crop(display);
	hal_intputTiming = get_input_timing_for_crop(display);

	scaler_info.type=(unsigned int)ratio_type;
	scaler_info.state=(unsigned int)get_vo_smoothtoggle_state(display);

	scaler_info.overscan_x=(unsigned int)overscan_size.srcx;
	scaler_info.overscan_y=(unsigned int)overscan_size.srcy;
	scaler_info.overscan_wid=(unsigned int)overscan_size.src_wid;
	scaler_info.overscan_height=(unsigned int)overscan_size.src_height;

	scaler_info.inputTiming_x=(unsigned int)hal_intputTiming.srcx;
	scaler_info.inputTiming_y=(unsigned int)hal_intputTiming.srcy;
	scaler_info.inputTiming_wid=(unsigned int)hal_intputTiming.src_wid;
	scaler_info.inputTiming_height=(unsigned int)hal_intputTiming.src_height;

	scaler_info.disp_x=(unsigned int)dispwin.srcx;
	scaler_info.disp_y=(unsigned int)dispwin.srcy;
	scaler_info.disp_wid=(unsigned int)dispwin.src_wid;
	scaler_info.disp_height=(unsigned int)dispwin.src_height;

	scaler_info.vo_3d_mode=Scaler_Get3DMode((SCALER_DISP_CHANNEL)display);
	scaler_info.display = display;

	scaler_info.NewAdaptiveInputFlag = Scaler_InputRegionType_Get(display);
	
	Check_smooth_toggle_update_flag(display);

	pulDataTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_SENDSMOOTHTOGGLESTATE);
	ulCount = sizeof(VO_Scaler_State) / sizeof(unsigned int);

	// copy to RPC share memory
	memcpy(pulDataTemp, &scaler_info, sizeof(VO_Scaler_State));

	for (i = 0; i < ulCount; i++)
		pulDataTemp[i] = htonl(pulDataTemp[i]);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SENDSMOOTHTOGGLESTATE,0,0)))
	{
		rtd_pr_smt_debug("ret=%d, send VO's scaler state to driver fail !!!\n", ret);
	}
}
void Share_Memory_Set_Scaler_Run_Flag(unsigned char enable)
{
	unsigned int *sync_flag = NULL;
	unsigned int *pulTemp = NULL;
	unsigned int i = 0, ulItemCount = 0;

	sync_flag = (unsigned int *)Scaler_GetShareMemVirAddr(VO_RUN_SCALER_FLAG);
	*sync_flag = enable;
	
	ulItemCount = sizeof(unsigned int) /  sizeof(UINT32);
	pulTemp = sync_flag;
	for (i = 0; i < ulItemCount; i++)
		pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);
}

void wait_smoothtoggle_rpcfinish(void)
{
	unsigned int timeout=0x1fffff;//change timeout time = 200ms @Crixus 20151217 ;
	unsigned int *sync_flag;
	sync_flag = (unsigned int *)Scaler_GetShareMemVirAddr(SCALERIOC_SMOOTHTOGGLE_SHAREMEMORY_SYNC_FALG);
	while(timeout) {
		  if(*sync_flag == 0)
		  	break;
		  else
		  	timeout--;
		   sync_flag = (unsigned int *)Scaler_GetShareMemVirAddr(SCALERIOC_SMOOTHTOGGLE_SHAREMEMORY_SYNC_FALG);
	}
	if(timeout == 0)
		rtd_pr_smt_notice("\r\n####wait_smoothtoggle_rpcfinish timeout####\r\n");
}

void Check_smooth_toggle_update_flag(unsigned char display)
{
		unsigned int timeout=0x1fffff;//change timeout time = 200ms @Crixus 20151217 ;
		unsigned int frametimeout = 0x1fffff;
		unsigned char check_bit = display?(_BIT5):(_BIT0);

	if(1)//currently all smooth toggle flow is at video fw//((Scaler_InputSrcGetType(display) == _SRC_VO) && (Get_DisplayMode_Src(display) != VSC_INPUTSRC_JPEG))
	{//VO and Not JPEG case, jpag goes kernel smooth toggle
		unsigned char point_out=50;
#ifdef CONFIG_I2RND_ENABLE
		if (Scaler_I2rnd_get_timing_enable()) {
			if(display == SLR_MAIN_DISPLAY){
				point_out = 50;
				if (get_i2rnd_pst_write_pnt_counter()) {
					while(point_out) {
						if (!get_i2rnd_pst_write_pnt_counter()) {
							break;
						}
						msleep(10);
						point_out--;
					}
				}
				point_out= 50;
				if (((Scaler_i2rnd_get_read_point(I2RND_MAIN_S0_TABLE) +1) %(PST_CMD_NUMBER+1)) <= Scaler_i2rnd_get_write_point(I2RND_MAIN_S0_TABLE)) {
					while(point_out) {
						if (Scaler_i2rnd_get_read_point(I2RND_MAIN_S0_TABLE) == Scaler_i2rnd_get_write_point(I2RND_MAIN_S0_TABLE)) {
							break;
						}
						msleep(10);
						point_out--;
					}
				}
				rtd_pr_smt_emerg("i2rnd s0 write(%d) exteed read buffer(%d)\n",Scaler_i2rnd_get_write_point(I2RND_MAIN_S0_TABLE),Scaler_i2rnd_get_read_point(I2RND_MAIN_S0_TABLE));
			#ifdef CONFIG_PST_ENABLE
					if (!Scaler_main_md_pst_get_enable()) {
						return;
					}
			#else
					return;
			#endif
			}
			else{
				point_out = 50;
				if (get_i2rnd_s1_pst_write_pnt_counter()) {
					while(point_out) {
						if (!get_i2rnd_s1_pst_write_pnt_counter()) {
							break;
						}
						msleep(10);
						point_out--;
					}
				}
				point_out= 50;
				if (((Scaler_i2rnd_get_read_point(I2RND_SUB_S1_TABLE) +1) %(PST_CMD_NUMBER+1)) <= Scaler_i2rnd_get_write_point(I2RND_SUB_S1_TABLE)) {
					while(point_out) {
						if (Scaler_i2rnd_get_read_point(I2RND_SUB_S1_TABLE) == Scaler_i2rnd_get_write_point(I2RND_SUB_S1_TABLE)) {
							break;
						}
						msleep(10);
						point_out--;
					}
				}
				rtd_pr_smt_emerg("i2rnd s1 write(%d) exteed read buffer(%d)\n",Scaler_i2rnd_get_write_point(I2RND_SUB_S1_TABLE),Scaler_i2rnd_get_read_point(I2RND_SUB_S1_TABLE));
			#ifdef CONFIG_PST_ENABLE
					if (!Scaler_sub_md_pst_get_enable()) {
						return;
					}
			#else
					return;
			#endif
			}
		}
#endif
		point_out=50;
#ifdef CONFIG_PST_ENABLE
		if (Scaler_main_md_pst_get_enable()) {
			if(display == SLR_MAIN_DISPLAY){
				point_out=50;
				if (((Scaler_pst_get_read_point(_MAIN_MD) +1) %(PST_CMD_NUMBER+1)) <= Scaler_pst_get_write_point(_MAIN_MD)) {
					while(point_out) {
						if (Scaler_pst_get_write_point(_MAIN_MD) == Scaler_pst_get_read_point(_MAIN_MD)) {
							break;
						}
						msleep(10);
						point_out--;
					}
				}
				rtd_pr_smt_emerg("main md pst write(%d) exteed read buffer(%d)\n",Scaler_pst_get_write_point(_MAIN_MD),Scaler_pst_get_read_point(_MAIN_MD));
				return;
			}
			else{
				point_out=50;
				if (((Scaler_pst_get_read_point(_SUB_MD) +1) %(PST_CMD_NUMBER+1)) <= Scaler_pst_get_write_point(_SUB_MD)) {
					while(point_out) {
						if (Scaler_pst_get_write_point(_SUB_MD) == Scaler_pst_get_read_point(_SUB_MD)) {
							break;
						}
						msleep(10);
						point_out--;
					}
				}
				rtd_pr_smt_emerg("sub md pst write(%d) exteed read buffer(%d)\n",Scaler_pst_get_write_point(_SUB_MD),Scaler_pst_get_read_point(_SUB_MD));
				return;
			}
		}
#endif

		if  ((Get_sub_from_DI_mux_path())&&(display == SLR_SUB_DISPLAY))
			check_bit = _BIT0;

		wait_smoothtoggle_rpcfinish();
		while(frametimeout) {
			if ((rtd_inl(VGIP_Smooth_tog_ctrl_reg)&check_bit)==0) {
				unsigned int *pulTemp;
				unsigned int i = 0, ulItemCount = 0;
				SMOOTH_TOGGLE_UPDATE_FWINFO_PKG *sm_info_pkg = NULL;
				SMOOTH_TOGGLE_UPDATE_FWINFO_PKG update_info;
				while(timeout) {
					sm_info_pkg = (SMOOTH_TOGGLE_UPDATE_FWINFO_PKG *)Scaler_GetShareMemVirAddr(SCALERIOC_SMOOTHTOGGLE_SHARE_INFO);
					if (sm_info_pkg) {
#ifdef CONFIG_ARM64						
						memcpy_toio(&update_info, sm_info_pkg, sizeof(SMOOTH_TOGGLE_UPDATE_FWINFO_PKG));
#else
						memcpy(&update_info, sm_info_pkg, sizeof(SMOOTH_TOGGLE_UPDATE_FWINFO_PKG));
#endif
						ulItemCount = sizeof(SMOOTH_TOGGLE_UPDATE_FWINFO_PKG) /  sizeof(UINT32);
						pulTemp = (unsigned int *)&update_info;
						for (i = 0; i < ulItemCount; i++)
							pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);
						if (!update_info.sm_info[display].update_flag) {
							break;
						}
					}
					timeout--;
				}
				break;
			}
			frametimeout--;
		}
		rtd_pr_smt_debug("Check_smooth_toggle_update_flag:timeout=%x,%x\n",timeout,frametimeout);
		if(timeout==0){
			unsigned char info=0;
			unsigned char plane = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_VODMA_PLANE);
			if(display==SLR_MAIN_DISPLAY){
				info=plane;
			} else {
				info=plane+100;
			}
			Scaler_Set_VOsmoothenable(info);
		}
		if(frametimeout==0){
			rtd_clearbits(VGIP_Smooth_tog_ctrl_reg,check_bit);
		}

		if((timeout == 0) || (frametimeout == 0))
		{
			rtd_pr_smt_notice("Check_smooth_toggle_update_flag timeout=%x, frametimeout=%x\n", timeout, frametimeout);
		}
	}
}

void set_smooth_toggle_update_flag(unsigned char display, unsigned int flag)
{
	unsigned int *pulTemp;
	unsigned int i = 0, ulItemCount = 0;
	SMOOTH_TOGGLE_UPDATE_FWINFO_PKG *sm_info_pkg = NULL;
	SMOOTH_TOGGLE_UPDATE_FWINFO_PKG update_info;
	sm_info_pkg = (SMOOTH_TOGGLE_UPDATE_FWINFO_PKG *)Scaler_GetShareMemVirAddr(SCALERIOC_SMOOTHTOGGLE_SHARE_INFO);
	if (sm_info_pkg)
	{
		memcpy(&update_info, sm_info_pkg, sizeof(SMOOTH_TOGGLE_UPDATE_FWINFO_PKG));
		ulItemCount = sizeof(SMOOTH_TOGGLE_UPDATE_FWINFO_PKG) /  sizeof(UINT32);
		pulTemp = (unsigned int *)&update_info;
		for (i = 0; i < ulItemCount; i++)
			pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);
		update_info.sm_info[display].update_flag = flag;
		for (i = 0; i < ulItemCount; i++)
			pulTemp[i] = Scaler_ChangeUINT32Endian(pulTemp[i]);
		memcpy(sm_info_pkg,&update_info, sizeof(SMOOTH_TOGGLE_UPDATE_FWINFO_PKG));
	}

}

static SCALER_MEMORY_INFO scaler_memory_info;

#ifdef CONFIG_MDOMAIN_RESERVED_MEMORY
void send_memory_mapping_for_adaptive_streaming(void)
{
	unsigned int *pulDataTemp = NULL;
	unsigned int ulCount = 0, i = 0;
	int ret = 0;
	scaler_memory_info.M_main_first_addr = drvif_memory_get_memtag_startaddr(MEMIDX_MAIN);
	scaler_memory_info.M_main_sec_addr = drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_SEC);
	//scaler_memory_info.M_main_size=m_size;
	scaler_memory_info.M_main_third_addr = drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_THIRD);
#ifdef CONFIG_I2RND_ENABLE
	scaler_memory_info.M_main_i2r_first_addr = drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_2);
	scaler_memory_info.M_main_i2r_sec_addr = drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_2_SEC);
	scaler_memory_info.M_main_i2r_third_addr = drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_2_THIRD);
	scaler_memory_info.DI_i2r_addr = get_query_start_address(QUERY_IDX_DI_2);
#endif
	pulDataTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_SENDSCALERMEMORYINFO);
	ulCount = sizeof(scaler_memory_info) / sizeof(unsigned int);
	// copy to RPC share memory
#ifdef CONFIG_ARM64	
	memcpy_toio(pulDataTemp, &scaler_memory_info, sizeof(scaler_memory_info));
#else
	memcpy(pulDataTemp, &scaler_memory_info, sizeof(scaler_memory_info));
#endif

	/*rtd_pr_smt_notice("[memory info]main:%x, %x, %dx\n sub:%x, %x, %x\n di :%x\n",
	scaler_memory_info.M_main_first_addr,scaler_memory_info.M_main_sec_addr,scaler_memory_info.M_main_third_addr,
	scaler_memory_info.M_sub_first_addr,scaler_memory_info.M_sub_sec_addr,scaler_memory_info.M_sub_third_addr,
	scaler_memory_info.DI_addr);*/

	for (i = 0; i < ulCount; i++)
		pulDataTemp[i] = htonl(pulDataTemp[i]);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SENDSCALERMEMORYINFO,0,0)))
	{
		rtd_pr_smt_debug("ret=%d, send SCALERIOC_SENDSCALERMEMORYINFO to driver fail !!!\n", ret);
	}
}
#endif

void vo_smoothtoggle_memory_alloc(unsigned char display)
{
	unsigned int di_addr,m_addr_sec,m_addr_first,m_addr_third;
	unsigned int di_size;
	unsigned int m_sub_sec,m_sub_first,m_sub_third;

	if (display==SLR_MAIN_DISPLAY) {
		/*if(vo_force_data_mode_get_enable(SLR_MAIN_DISPLAY)==SLR_DISPLAY_DATA_FRAME_SYNC)
		{
			di_addr=drvif_memory_get_block_addr(MEMIDX_DI);
			m_addr_third=0xffffffff;
			m_addr_sec=0xffffffff;
			m_addr_first=0xffffffff;
		}
		else if(vo_force_data_mode_get_enable(SLR_MAIN_DISPLAY)==SLR_DISPLAY_DATA_FS_NO_MEMORY)
		{
			di_addr=0xffffffff;
			m_addr_third=0xffffffff;
			m_addr_sec=0xffffffff;
			m_addr_first=0xffffffff;
		}
		else*/ //if(vo_force_data_mode_get_enable()==SLR_DISPLAY_DATA_FRC)
		{
			di_addr=drvif_memory_get_block_addr(MEMIDX_DI);
#ifdef CONFIG_MDOMAIN_RESERVED_MEMORY
			m_addr_third=drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_THIRD);
			m_addr_sec=drvif_memory_get_memtag_startaddr(MEMIDX_MAIN_SEC);
			m_addr_first=drvif_memory_get_memtag_startaddr(MEMIDX_MAIN);
#else
			m_addr_third=drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD); // the max: 444 ,8 bit:1920*1080*3byte
			m_addr_sec=drvif_memory_get_block_addr(MEMIDX_MAIN_SEC);  // the max: 444 ,10 bit:2160*1440*30bit
			m_addr_first=drvif_memory_get_block_addr(MEMIDX_MAIN);
#endif
		}

		di_size=drv_memory_di_buffer_get();
		m_sub_third=drvif_memory_get_block_addr(MEMIDX_SUB_THIRD);
		m_sub_sec=drvif_memory_get_block_addr(MEMIDX_SUB_SEC);
		m_sub_first=drvif_memory_get_block_addr(MEMIDX_SUB);
	}
#ifdef CONFIG_DUAL_CHANNEL
	else {
		/*if(vo_force_data_mode_get_enable(SLR_SUB_DISPLAY)==SLR_DISPLAY_DATA_FRAME_SYNC)
		{
			m_sub_third=0xffffffff;
			m_sub_sec=0xffffffff;
			m_sub_first=0xffffffff;
		}
		else if(vo_force_data_mode_get_enable(SLR_SUB_DISPLAY)==SLR_DISPLAY_DATA_FS_NO_MEMORY)
		{
			m_sub_third=0xffffffff;
			m_sub_sec=0xffffffff;
			m_sub_first=0xffffffff;
		}
		else*/ //if(vo_force_data_mode_get_enable()==SLR_DISPLAY_DATA_FRC)
		{
			m_sub_third=drvif_memory_get_block_addr(MEMIDX_SUB_THIRD);//0xffffffff;//drvif_memory_alloc_block(MEMIDX_MAIN_THIRD,0x600000, MEMALIGN_ANY); // the max: 444 ,8 bit:1920*1080*3byte
			m_sub_sec=drvif_memory_get_block_addr(MEMIDX_SUB_SEC);  // the max: 444 ,10 bit:2160*1440*30bit
			m_sub_first=drvif_memory_get_block_addr(MEMIDX_SUB);
		}
		di_size=drv_memory_di_buffer_get();
		di_addr=drvif_memory_get_block_addr(MEMIDX_DI);
		m_addr_third=drvif_memory_get_block_addr(MEMIDX_MAIN_THIRD);
		m_addr_sec=drvif_memory_get_block_addr(MEMIDX_MAIN_SEC);
		m_addr_first=drvif_memory_get_block_addr(MEMIDX_MAIN);
	}
#endif
	scaler_memory_info.DI_addr=di_addr;
	scaler_memory_info.DI_size=di_size;
	scaler_memory_info.M_main_first_addr=m_addr_first;
	scaler_memory_info.M_main_sec_addr=m_addr_sec;
	//scaler_memory_info.M_main_size=m_size;
	scaler_memory_info.M_main_third_addr=m_addr_third;

	scaler_memory_info.M_sub_first_addr = m_sub_first;
	scaler_memory_info.M_sub_sec_addr = m_sub_sec;
	scaler_memory_info.M_sub_third_addr = m_sub_third;

#ifndef CONFIG_MDOMAIN_RESERVED_MEMORY
	int ret = 0;
	unsigned int *pulDataTemp = NULL;
	unsigned int ulCount = 0, i = 0;

	pulDataTemp = (unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_SENDSCALERMEMORYINFO);
	ulCount = sizeof(scaler_memory_info) / sizeof(unsigned int);

	// copy to RPC share memory
	memcpy(pulDataTemp, &scaler_memory_info, sizeof(scaler_memory_info));

	for (i = 0; i < ulCount; i++)
		pulDataTemp[i] = htonl(pulDataTemp[i]);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SENDSCALERMEMORYINFO,0,0)))
	{
		rtd_pr_smt_debug("ret=%d, send SCALERIOC_SENDSCALERMEMORYINFO to driver fail !!!\n", ret);
	}
#else
	send_memory_mapping_for_adaptive_streaming();
#endif
}

void vodma_smoothtoggle_reset(unsigned char display,unsigned char plane)
{
	if(SLR_MAIN_DISPLAY == display)
		clear_imd_smooth_toggle_clk_time2switch_en(display);
	
	if(VO_VIDEO_PLANE_V1 == plane) {
		vodma_vodma_smooth_toggle_RBUS vodma_smooth_toggle_reg;
		vodma_smooth_toggle_reg.regValue = rtd_inl(VODMA_VODMA_SMOOTH_TOGGLE_reg);
		if (vodma_smooth_toggle_reg.smooth_en == 1) {
			vodma_smooth_toggle_reg.smooth_en = 0;
			rtd_outl(VODMA_VODMA_SMOOTH_TOGGLE_reg, vodma_smooth_toggle_reg.regValue);
		}
	} else if(VO_VIDEO_PLANE_V2 == plane) {
		vodma2_vodma2_smooth_toggle_RBUS vodma2_smooth_toggle_reg;
		vodma2_smooth_toggle_reg.regValue = rtd_inl(VODMA2_VODMA2_SMOOTH_TOGGLE_reg);
		if (vodma2_smooth_toggle_reg.smooth_en == 1) {
			vodma2_smooth_toggle_reg.smooth_en = 0;
			rtd_outl(VODMA2_VODMA2_SMOOTH_TOGGLE_reg, vodma2_smooth_toggle_reg.regValue);
		}
	}
}

void smooth_toggle_disconnect(unsigned char display,unsigned char plane)
{
	Check_smooth_toggle_update_flag(display);
	if(Get_vo_smoothtoggle_timingchange_flag(display)) //direct_vo
	{
		Set_direct_vo_smoothtoggle_enable(0,/*SLR_DISPLAY_DEFAULT,*/ display,plane); //close smooth toggle
	} else {
		vodma_smoothtoggle_reset(display,plane);
		set_vo_open_smooth_toggle_enable(0,display);
		set_vo_run_smooth_toggle_enable(0,display);
	}
}

void smooth_toggle_preset_info(unsigned char display)
{
	if (!Get_vo_smoothtoggle_timingchange_flag(display)) {
		/*if(Scaler_DispGetInputInfoByDisp(display,SLR_INPUT_FRAMESYNC)) {
			vo_force_data_mode_set_enable(SLR_DISPLAY_DATA_FRAME_SYNC,display);
		} else {
			vo_force_data_mode_set_enable(SLR_DISPLAY_DATA_FRC,display);
		}*/
		vo_smoothtoggle_memory_alloc(display);
		panel_parameter_send_info();
		rtd_pr_smt_notice("smooth_toggle_preset_info\n");
	} else {
		if (get_vo_smoothtoggle_state(display) == SLR_SMOOTH_TOGGLE_INIT) {
			panel_parameter_send_info();
			vo_smoothtoggle_memory_alloc(display);
			set_vo_smoothtoggle_state(SLR_SMOOTH_TOGGLE_ACTIVE, display, Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_VODMA_PLANE));
			smooth_toggle_state_info_rpc(display);
			rtd_pr_smt_notice("smooth_toggle_preset_info\n");
		}
	}
}

unsigned char decide_run_sub_smoothtoggle_case(void)
{//return TRUE: can run sub smooth toggle
	return TRUE;
#if 0
	if((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) || 
		(Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_V_FREQ) <= Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) + 1))
	{
		return TRUE;
	}
	else
		return FALSE;
#endif						
}

//----------end of api
