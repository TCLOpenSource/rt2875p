#include <sysdefs.h>
#include <printf.h>
#include <timer.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/scaleup_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/dm_reg.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <scaler/scalerCommon.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <scaler_vbedev.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/pcbsource/pcbSource.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>
#include <no_os/printk.h>
#include <rtd_log/rtd_module_log.h>
#include <timer.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <scaler_vscdev.h>


extern unsigned char scaler_borrowmemory_from_i3ddma_for_mdomain(void);
extern PQ_HDR_TYPE Scaler_QsGetHdrType(void);
extern void fwif_color_set_HDR10_runmode(void);
extern unsigned char scaler_force_run_idma_impl(unsigned char display, VSC_INPUT_TYPE_T inputSrctype);
extern void Scaler_SET_VSCDispinfo_WithVFEHMDI(unsigned char display, vfe_hdmi_timing_info_t vfehdmitiminginfo);
//extern void QS_VSCInit_SyncConnectInfo2(void);
extern  unsigned char modestate_set_vgip(StructDisplayInfo* info);
extern void modestate_set_imd_domain(StructDisplayInfo* info);
extern unsigned char modestate_set_framesync(StructDisplayInfo* info);
extern void check_ARC_status(unsigned char display, KADP_VSC_INPUT_TYPE_T source);
extern void scaler_set_online_measure(unsigned char display, KADP_VSC_INPUT_TYPE_T inputType);
extern StructSrcRect main_dispwin;//Temp record main output info
extern StructSrcRect main_input_size;
extern StructSrcRect main_input_timing;
extern void fw_scaler_set_vgip(unsigned char channel, unsigned char src, unsigned char mode);
unsigned char scaler_quickshow_get_displayinfo_proc(unsigned char eVgipSrcType, unsigned char inputSrctype)//This is for driver base implement open source
{
    if(inputSrctype == VSC_INPUTSRC_HDMI)
	{
             if(1/*Scaler_GetHdrVideoPath() != HDR_VIDEO_INPUT_I3DDMA_VO_HDR*/)
             {
                drvif_mode_enableonlinemeasure(SLR_MAIN_DISPLAY);
    	  }else{
                drvif_mode_enable_dma_onlinemeasure();
            }
	}
    else
    {
		return FALSE;
	}
  

	Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC, FALSE);
    if(Get_Val_vsc_run_pc_mode() == TRUE && inputSrctype == VSC_INPUTSRC_HDMI)
    {
        Scaler_DispSetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP, FALSE);
        dvrif_memory_set_compression_bits(SLR_MAIN_DISPLAY, COMPRESSION_16_BITS);//444 go 16 bits
    } else {
        Scaler_DispSetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP, TRUE);
        dvrif_memory_set_compression_bits(SLR_MAIN_DISPLAY, COMPRESSION_12_BITS);//422 go 12 bits
    }

	rtd_clearbits(SCALEUP_D_UZU_Globle_Ctrl_reg, _BIT0|_BIT1);//clear uzu pattern gen
	Scaler_DispSetStatus(SLR_MAIN_DISPLAY, SLR_DISP_10BIT, TRUE);//let all source go 10bit mode

	dvrif_memory_compression_set_enable(SLR_MAIN_DISPLAY, TRUE);
	if(Get_Val_qs_game_flag())
	{
		dvrif_memory_set_compression_mode(SLR_MAIN_DISPLAY, COMPRESSION_LINE_MODE);
	}
	else
	{
		dvrif_memory_set_compression_mode(SLR_MAIN_DISPLAY, COMPRESSION_FRAME_MODE);
	}

	if(inputSrctype == VSC_INPUTSRC_HDMI)
	{
		printf( " ###Finally fsync:%d 422:%d comp(%d %d) ###\r\n", Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC), Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_DISP_422CAP), dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY), dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY));
	}
	else
	{
		printf(" ###Finally fsync:%d 422:%d comp(%d %d)###\r\n", Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC), Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_DISP_422CAP), dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY), dvrif_memory_get_compression_bits(SLR_MAIN_DISPLAY));
	}
	//limit mode setting

	drv_memory_set_limit_boundary_mode(SLR_MAIN_DISPLAY, 1);

	return TRUE;
}
unsigned char scaler_quickshow_Setup_IMD(unsigned char inputType, StructDisplayInfo* info)
{
	Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FAILURECOUNT, 0);
	//set_display_forcebg_mask(display, SLR_FORCE_BG_TYPE_HDMI_AVMUTE_MASK, FALSE);//disable hdmi av mute mask

	//set_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_DTV_CHANNEL_CHANGE_HDR_SEAMLESS, 0);//diable DTV switch seamless mask

    //Scaler_HdrHandle();
	two_pixel_mode_ctrl(0);

	drvif_scaler_check_vgip_data_dclk(SLR_MAIN_DISPLAY);
	rtd_maskl(VGIP_VGIP_CHN1_MISC_reg, ~_BIT0, 0);

	Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_DLY_PRE,IoReg_ReadByte0(VGIP_VGIP_CHN1_DELAY_reg));
	
	if(!modestate_set_vgip(info)){//set VGIP fail for vo source return detect state
		//flow_message_push(Scaler_DispGetInputInfoByDisp(SLR_INPUT_DISPLAY) ? _FL2FL_SUB_STATE_PERIODIC_EVENT: _FL2FL_MAIN_STATE_PERIODIC_EVENT);
		return FALSE;
	}


	fwif_color_DI_IP_ini();

	modestate_set_imd_domain(info);

	scaler_disp_dtg_pre_i_domain_setting(SLR_MAIN_DISPLAY);

	if (!modestate_set_framesync(info))  // if fs fail, keep in fs state
	{
		return FALSE;
	}
	Scaler_disp_setting(SLR_MAIN_DISPLAY);
	//state_update_disp_info();//Update the active state
	if(1)
	{
		unsigned int before_time=0,after_time=0;
		before_time = get_boot_time_us();
		fwif_color_video_quality_handler();
		after_time = get_boot_time_us();
		
			printf("\n PQ total time =%d , before time =%d, after time =d  \n",after_time-before_time,before_time, after_time);

	}

	//add main double buffer apply @Crixus 20160126
    if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC) == FALSE){
        if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID) >=3840 && Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN) >=2160
            &&  Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ)<=310)
        {
            drv_memory_wait_cap_last_write_done(SLR_MAIN_DISPLAY, 2, FALSE);
        }
        else
        {
            drv_memory_wait_cap_last_write_done(SLR_MAIN_DISPLAY, 1, FALSE);
        }
    }

    wait_DI_ready();//wait DI ready
	return TRUE;
}
typedef enum _HDR_VIDEO_PATH
{
    HDR_SEL_VODMA1 = 0,
    HDR_SEL_HDMI,
    HDR_SEL_NONE
}HDR_VIDEO_PATH;

HDR_VIDEO_PATH Scaler_QsGetHdrVideoPath(void)
{
    vfe_hdmi_timing_info_t vfehdmitiminginfo;
    unsigned char interlace = 0;

   interlace = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);

    if(Scaler_QsGetHdrType() != HDR_DM_MODE_NONE)
    {
        if(interlace)
        {
            return HDR_SEL_VODMA1;
        }
        else
        {
            return HDR_SEL_HDMI;
        }
    }
    else
    {
        return HDR_SEL_NONE;
    }
}

unsigned char scaler_quickshow_run_scaler(unsigned char inputType)
{
     ppoverlay_main_display_control_rsv_RBUS ppoverlay_main_display_control_rsv_reg;
     ppoverlay_main_display_control_rsv_reg.regValue = IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);//rtd_inl(PPOVERLAY_Main_Display_Control_RSV_reg);
	 ppoverlay_main_display_control_rsv_reg.m_force_bg = 1;
     IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, ppoverlay_main_display_control_rsv_reg.regValue);
     //rtd_outl(PPOVERLAY_Main_Display_Control_RSV_reg, ppoverlay_main_display_control_rsv_reg.regValue);
	//unsigned char game_mode_wait_timeout = 40;	//400ms
    unsigned char vgipSrcType = VGIP_SRC_TMDS;
    
    if(inputType == VSC_INPUTSRC_HDMI)
    {
        vbe_disp_set_freesync_mode_flag(drvif_Hdmi_GetAMDFreeSyncEnable());//save current freesync mode
        Set_Val_VRR_timingMode_flag(drvif_Hdmi_GetVRREnable());//save current vrr mode
    }

#if 1
    if(Scaler_QsGetHdrVideoPath() == HDR_SEL_VODMA1)
    {
        pr_info("[%s %d]hdr sel vodma1\n", __FUNCTION__, __LINE__);
        vgipSrcType = VGIP_SRC_HDR1;
        fw_scaler_set_vgip(SLR_MAIN_DISPLAY, vgipSrcType, VGIP_MODE_ANALOG);
        fw_scaler_set_sample(SLR_MAIN_DISPLAY, 1);
        set_force_hdmi_hdr_flow_enable(FALSE);
        Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_THROUGH_I3DDMA, TRUE);
        scaler_force_run_idma_impl(SLR_MAIN_DISPLAY, inputType);
        #if 0
        scaler_vtop_dolby_mode_setting(Scaler_QsGetHdrType());
        #else
        scaler_vtop_dolby_mode_setting(Scaler_QsGetHdrType());
    	Scaler_color_HDRIP_AutoRun(Scaler_QsGetHdrType());
    	scaler_hdr_mode_setting(SLR_MAIN_DISPLAY, Scaler_QsGetHdrType());
        //printf2("PQ hdr done\n");
        #endif        
        fwif_color_set_HDR10_runmode();
        //if(Scaler_GetGameMode() && (!Scaler_I3ddmaTripleBufFlag()))
        //{
            //Scaler_I3ddmaRunSignleBuffer();
        //}
        
    }
    else if(Scaler_QsGetHdrVideoPath() == HDR_SEL_HDMI)
    {
        pr_info("[%s %d]hdr sel hdmi\n", __FUNCTION__, __LINE__);
        vgipSrcType = VGIP_SRC_HDR1;
        set_force_hdmi_hdr_flow_enable(TRUE); 
        fw_scaler_set_vgip(SLR_MAIN_DISPLAY, vgipSrcType, VGIP_MODE_ANALOG);
        fw_scaler_set_sample(SLR_MAIN_DISPLAY, 1);
        #if 0
        scaler_vtop_dolby_mode_setting(Scaler_QsGetHdrType());
        #else
        scaler_vtop_dolby_mode_setting(Scaler_QsGetHdrType());
    	Scaler_color_HDRIP_AutoRun(Scaler_QsGetHdrType());
    	scaler_hdr_mode_setting(SLR_MAIN_DISPLAY, Scaler_QsGetHdrType());
        //printf2("PQ hdr done\n");
        #endif        
        hdmi_vtop_hfr_control();//for two pixel mode setting
        fwif_color_set_HDR10_runmode();
        if (!is_2k_memory_buffer_source(SLR_MAIN_DISPLAY))
        {//m odmain using 4k size need to borrow the memory from i3ddma
            if (1==scaler_borrowmemory_from_i3ddma_for_mdomain()) {//borrow fail
                pr_emerg("mdomian borrow i3ddma memory fail\n");
            }
        }
    }
    else
    {
        pr_info("[%s %d]normal case\n", __FUNCTION__, __LINE__);
        vgipSrcType = VGIP_SRC_TMDS;
        fw_scaler_set_vgip(SLR_MAIN_DISPLAY, vgipSrcType, VGIP_MODE_ANALOG);
        fw_scaler_set_sample(SLR_MAIN_DISPLAY, 1);
        
        if (!is_2k_memory_buffer_source(SLR_MAIN_DISPLAY))
        {//m odmain using 4k size need to borrow the memory from i3ddma
            if (1==scaler_borrowmemory_from_i3ddma_for_mdomain()) {//borrow fail
                pr_emerg("mdomian borrow i3ddma memory fail\n");
            }
        }
    }

#endif
	if(scaler_quickshow_get_displayinfo_proc(vgipSrcType, inputType) == FALSE)
	{
	#if 0//to fix
		printf(KERN_NOTICE, TAG_NAME_VSC, "######get_displayinfo_proc fail fail fail!!!#######\n");

		run_scaler_displayinfo_fail_handle(display, inputType);

		if (display == SLR_MAIN_DISPLAY) {
			scaler_quickshow_drivf_scaler_reset_freerun();
			if(DbgSclrFlgTkr.Main_AutoRunScaler) {
				printf(KERN_DEBUG, TAG_NAME_VSC, "######Need to run scale again#######\n");
				DbgSclrFlgTkr.Main_Run_Scaler_flag = TRUE;
			}
		} 
        #endif
        printf( "qs init get_displayinfo_proc error\n");
		return FALSE;
	}
    else
        {
printf( "qs init get_displayinfo_proc success\n");
    }

	if(scaler_quickshow_Setup_IMD(inputType, Scaler_DispGetInfoPtr()) == FALSE)
	{
	printf( "qs init Setup_IMD error\n");
	#if 0//to fix
		//reset smoothtoggle state
		if(Get_vo_smoothtoggle_timingchange_flag(display)) {
			unsigned int plane = Scaler_DispGetInputInfoByDisp(SLR_SUB_DISPLAY, SLR_INPUT_VODMA_PLANE);
			Check_smooth_toggle_update_flag(display);
			set_vo_smoothtoggle_state(SLR_SMOOTH_TOGGLE_INIT, display,plane);
                       	smooth_toggle_state_info_rpc(display);
		}
		Scaler_DispSetInputInfo(SLR_INPUT_STATE, _MODE_STATE_SEARCH);
		state_update_disp_info();//Update the search state
		
		run_scaler_setupIMD_fail_handle(display, inputType);
		
		printf(KERN_NOTICE, TAG_NAME_VSC, "######Channel:%d src:%d Setup_IMD fail!!!#######\n", display, inputType);
		if (display == SLR_MAIN_DISPLAY) {
			scaler_quickshow_drivf_scaler_reset_freerun();
			if(DbgSclrFlgTkr.Main_AutoRunScaler) {
				printf(KERN_DEBUG, TAG_NAME_VSC, "######Need to run scale again#######\n");
				DbgSclrFlgTkr.Main_Run_Scaler_flag = TRUE;
			}
		}
        #endif
		return FALSE;
	}
	else
	{
		// todo bg
		scaler_set_online_measure(SLR_MAIN_DISPLAY, inputType);
		if(Get_Val_qs_game_flag())
		{
			fw_scalerip_set_di_gamemode_setting(fw_scalerip_get_di_gamemode());
#if 1
			if(!drvif_i3ddma_triplebuf_flag() && (Scaler_InputSrcGetType(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) == _SRC_VO))
			{
				I3DDMA_TIMING_T *pI3ddma_tx_timing;
				pI3ddma_tx_timing = (I3DDMA_TIMING_T*) drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_TIMING);
				if(!((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC))&& pI3ddma_tx_timing && (pI3ddma_tx_timing->v_freq < 490)))
					drv_I3ddmaVodma_SingleBuffer_GameMode(_ENABLE); //I3DDMA 1-buffer
			}
#endif
		}
		set_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_VSC, FALSE);
		scalerdisplay_force_bg_enable(SLR_MAIN_DISPLAY, FALSE);
		printf(" Setup_IMD success disable background func %s ,line %d\n",__func__,__LINE__);
		//set_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_SIG_UNSTABLE, FALSE);//disable SLR_FORCE_BG_TYPE_SIG_UNSTABLE mask
		//set_display_forcebg_mask(SLR_MAIN_DISPLAY, SLR_FORCE_BG_TYPE_ROTATE, FALSE);//disable roate mask
	}

	return TRUE;
}
void scaler_quickshow_setoutputandinputregion(void)
{
        main_input_size.srcx = 0;
        main_input_size.srcy = 0;
        main_input_size.src_wid = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE);
        if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE))
        {
            main_input_size.src_height = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE) * 2;
        }
        else
        {
            main_input_size.src_height = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE);
        }
	 main_input_timing.srcx = main_input_size.srcx;
	 main_input_timing.srcy = main_input_size.srcy;
	 main_input_timing.src_height = main_input_size.src_height;
	 main_input_timing.src_wid = main_input_size.src_wid;
        //Scaler_AdaptiveStream_InputWindowSet(main_input_size,KADP_VIDEO_WID_0);
        main_dispwin.srcx = 0;
        main_dispwin.srcy = 0;
        main_dispwin.src_wid = _DISP_WID;
        main_dispwin.src_height = _DISP_LEN;
	set_input_for_vgip(SLR_MAIN_DISPLAY, main_input_size.srcx, main_input_size.srcy, main_input_size.src_wid, main_input_size.src_height);
	set_input_for_crop(SLR_MAIN_DISPLAY, main_input_size);
	set_input_timing_for_crop(SLR_MAIN_DISPLAY, main_input_timing);
	Scaler_InputRegionType_Set(SLR_MAIN_DISPLAY,INPUT_BASE_ON_TIMING);

	Scaler_DispWindowSet(main_dispwin);
}
extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr;
extern void set_source_info_forPQ(unsigned char display);
extern void increase_source_connect_verify_num(unsigned char display);
extern void Set_vsc_input_src_info(unsigned char display,KADP_VSC_OUTPUT_MODE_T vsc_output_mode, KADP_VSC_INPUT_SRC_INFO_T source_info);
extern StructDisplayInfo 			disp_info[MAX_DISP_CHANNEL_NUM];
extern StructDisplayInfo* 			info;	// Erin 100310
extern StructSourceInput *	src_info;
void scaler_quickshow_prepareInfo(void)
{
    KADP_VSC_INPUT_SRC_INFO_T inputSrcInfo;
    info = &disp_info[SLR_MAIN_DISPLAY];/*need remove after bring up*/
    disp_info[SLR_MAIN_DISPLAY].display = SLR_MAIN_DISPLAY;
    disp_info[SLR_MAIN_DISPLAY].channel = _CHANNEL1;
    src_info = PcbSource_GetTable();
    vfe_hdmi_timing_info_t vfehdmitiminginfo;
 
    increase_source_connect_verify_num(SLR_MAIN_DISPLAY);//increase the main source num
    //if(is_QS_hdmi_enable())
    inputSrcInfo.type = KADP_VSC_INPUTSRC_HDMI;
    inputSrcInfo.attr = 0;
    inputSrcInfo.resourceIndex=0;
    Set_vsc_input_src_info(SLR_MAIN_DISPLAY, KADP_VSC_OUTPUT_DISPLAY_MODE, inputSrcInfo);
    DbgSclrFlgTkr.OuputDisplayMode = TRUE;
    if(0 != vfe_hdmi_drv_get_display_timing_info(&vfehdmitiminginfo, SLR_MAIN_DISPLAY))
    {
        printf( "qs init vfe_hdmi_drv_get_display_timing_info error\n");
        return;
    }
    Scaler_SET_VSCDispinfo_WithVFEHMDI(SLR_MAIN_DISPLAY, vfehdmitiminginfo);
    if(Scaler_QsGetHdrVideoPath() == HDR_SEL_VODMA1)
    {
        set_force_i3ddma_enable(TRUE);
    }
    if (DbgSclrFlgTkr.main_opensourceID == _UNKNOWN_INPUT) {
        set_source_info_forPQ(SLR_MAIN_DISPLAY);//Set source info for PQ request befor scaler
    }
}

void quickshow_set_pc_mode();
unsigned char scaler_quickshow_output_connect(void)
{
    unsigned char result;
    // info = &disp_info[SLR_MAIN_DISPLAY];
    scaler_quickshow_prepareInfo();
    scaler_quickshow_setoutputandinputregion();
    quickshow_set_pc_mode();
    result = scaler_quickshow_run_scaler(VSC_INPUTSRC_HDMI);
    return result;
}
