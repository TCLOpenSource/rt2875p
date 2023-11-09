/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 * file:    hdmi_drvif.c
 * author:  kevin_wang
 * date:    2018/4/15
 * version: 4.0
 *
 *============================================================*/
#include "hdmi_common.h"
#include "hdmi_hdr.h"
#include "hdmi_audio.h"
#include "hdmi_info_packet.h"
#include "hdmi_vfe_config.h"
#include "cea_861.h"
#include <ioctrl/scaler/vfe_cmd_id.h> // for vfe_hdmi_vsi_t

/**********************************************************************************************
*
*   Generic API
*
**********************************************************************************************/


/*------------------------------------------------------
 * Func : drvif_Hdmi_GetHDMIMode
 *
 * Desc : get current HDMI mode of current HDMI port
 *
 * Parm : N/A
 *
 * Retn : MODE_DVI  : DVI mode
 *        MODE_HDMI : HDMI 1.4/2.0
 *        MODE_HDMI21 : HDMI 2.1
 *        MODE_UNKNOW : unknow mode
 *------------------------------------------------------*/
HDMI_DVI_MODE_T drvif_Hdmi_GetHDMIMode(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (HDMI_DVI_MODE_T)newbase_hdmi_get_hdmi_mode(port);
}
EXPORT_SYMBOL(drvif_Hdmi_GetHDMIMode);

/*------------------------------------------------------
 * Func : drvif_Hdmi_WatchDogApply
 *
 * Desc : Enable/Disable HDMI watch dog
 *
 * Parm : Enable : 0 : disabled, others : enabled
 *        Condition : watch dog flags
 *             HDMI_WD_VIDEO_COND :
 *             HDMI_WD_AUDIO_COND :
 *             HDMI_WD_VSC_COND :
 *
 * Retn : TRUE  : HDMI video is stable
 *        FALSE : HDMI video is not stable
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_WatchDogApply(unsigned char Enable, unsigned char Condition)
{
    // [RL6557-463] watch dog has been removed in K5L
    // Humm.. Should we remove it here ??
    return TRUE;
}



/*------------------------------------------------------
 * Func : drvif_Hdmi_CheckMode
 *
 * Desc : Check HDMI status of current HDMI port. this function is
 *        called by VSC periodically to check HDMI status
 *
 * Parm : N/A
 *
 * Retn : TRUE  : HDMI status check is OK
 *        FALSE : some error detected
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_CheckMode(void)
{
    return newbase_hdmi_check_mode();
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_OutputDisable
 *
 * Desc : Disable HDMI output for Power Saving
 *
 * Parm : on : 0 : not disable hdmi output
 *             others : disable hdmi output
 *
 * Retn : N/A
 *------------------------------------------------------*/
void drvif_Hdmi_OutputDisable(unsigned char on)
{
#if ENABLE_HDMI_VIDEO_OUTPUT_DISABLE
    newbase_hdmi_output_disable(on);
#endif    
}


extern int newbase_hdmi_get_common_port(unsigned char port, unsigned char *p_common_port);

/*------------------------------------------------------
 * Func : drvif_Hdmi_GetHDMIOutputPort
 *
 * Desc : get HDMI output of the specified HDMI channel
 *
 * Parm : vfe_call : called by VFE or not
 *
 * Retn : HDMI_OUTPUT_CHANNEL_NONE : no HDMI output,
 *        HDMI_OUTPUT_CHANNEL_1 : HDMI output 1
 *        HDMI_OUTPUT_CHANNEL_2 : HDMI output 2
 *------------------------------------------------------*/
HDMI_OUTPUT_CHANNEL_T drvif_Hdmi_GetHDMIOutputChannel(unsigned char ch)
{
    unsigned char port = 0;
    unsigned char common_port = 0;

    if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
        return HDMI_OUTPUT_CHANNEL_NONE;

    if (newbase_hdmi_get_common_port(port, &common_port)<0)
        return HDMI_OUTPUT_CHANNEL_NONE;

    switch(common_port)
    {
    case 0:
        return HDMI_OUTPUT_CHANNEL_1;
    case 1:
        return HDMI_OUTPUT_CHANNEL_2;
    }

    return HDMI_OUTPUT_CHANNEL_NONE;
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_SetNoPolarityInverse
 *
 * Desc : Enable/disable no polarity inverse funciton. this function
 *        will be called by VSC to control
 *
 * Parm : on_off : enable / disable
 *
 * Retn : TRUE  : HDMI status check is OK
 *        FALSE : some error detected
 *------------------------------------------------------*/
void drvif_Hdmi_SetNoPolarityInverse(unsigned char on_off)
{
    set_no_polarity_inverse(on_off);
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_IsHpdHigh
 *
 * Desc : Check if Hot plug is high
 *
 * Parm : port: speficed physical port
 *
 * Retn : TRUE  : HPD is high
 *        FALSE : HPD is low
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_IsHpdHigh(unsigned char port)
{
    return newbase_hdmi_get_hpd(port);
}
EXPORT_SYMBOL(drvif_Hdmi_IsHpdHigh);
/*------------------------------------------------------
 * Func : drvif_Hdmi_Is5VHigh
 *
 * Desc : Check if 5V is high
 *
 * Parm : port: speficed physical port
 *
 * Retn : TRUE  : 5V is high
 *        FALSE : 5V is low
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_Is5VHigh(unsigned char port)
{
    return newbase_hdmi_get_5v_state(port);
}
EXPORT_SYMBOL(drvif_Hdmi_Is5VHigh);
/*------------------------------------------------------
 * Func : drvif_Hdmi_get_pcb_earc_port_index
 *
 * Desc : Get eARC support port form pcb
 *
 * Parm : N/A
 *
 * Retn : PORT_ID  : eARC port id
 *        < 0 : invalid eARC port
 *------------------------------------------------------*/
HDMI_PCB_ARC_EARC_T drvif_Hdmi_get_pcb_earc_port_index(void)
{
    return newbase_hdmi_get_pcb_earc_port();
}
EXPORT_SYMBOL(drvif_Hdmi_get_pcb_earc_port_index);
//-------------------------------------------------------------------------
//  Video Related
//-------------------------------------------------------------------------

/*------------------------------------------------------
 * Func : drvif_Hdmi_GetFrlMode
 *
 * Desc : Get FRL Mode
 *
 * Parm : none
 *
 * Retn : 0  : TMDS mode, non HDMI2.1
 *         1~6: FRL mode, HDMI2.1 Frl rate
 *------------------------------------------------------*/
unsigned char drvif_Hdmi_GetFrlMode(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_rxphy_get_frl_mode(port);
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_GetInterlace
 *
 * Desc : Check video is Interlace / progressive
 *
 * Parm : N/A
 *
 * Retn : TRUE  : video is in interace mode
 *        FALSE : video is in progressive mode
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_GetInterlace(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_is_interlace(port);
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_GetColorSpace
 *
 * Desc : Check Color space of current HDMI channel
 *
 * Parm : N/A
 *
 * Retn : COLOR_RGB
 *        COLOR_YUV422
 *        COLOR_YUV444
 *        COLOR_YUV420 -> Force to YUV444 for back-end
 *------------------------------------------------------*/
HDMI_COLOR_SPACE_T drvif_Hdmi_GetColorSpace(void)
{// for back-end driver use
    unsigned char port = newbase_hdmi_get_current_display_port();
    unsigned char real_color_space = (HDMI_COLOR_SPACE_T)newbase_hdmi_get_colorspace_reg(port);
    
    if (real_color_space == COLOR_YUV420)
    {
        return COLOR_YUV444;
    }
    else
    {
        return real_color_space;
    }
    return real_color_space;
}
EXPORT_SYMBOL(drvif_Hdmi_GetColorSpace);

/*------------------------------------------------------
 * Func : drvif_Hdmi_GetRawColorSpace
 *
 * Desc : Check Real Color space of current HDMI channel
 *
 * Parm : N/A
 *
 * Retn : COLOR_RGB
 *        COLOR_YUV422
 *        COLOR_YUV444
 *        COLOR_YUV420
 *------------------------------------------------------*/
HDMI_COLOR_SPACE_T drvif_Hdmi_GetRawColorSpace(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (HDMI_COLOR_SPACE_T)newbase_hdmi_get_colorspace_reg(port);
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_EnableBypassYUV420Engine
 *
 * Desc : Enable bypass yuv 420 engine
 *
 * Parm : enable=1, bypass YUV420 engine, use for TMDS 4K120 YUV420; enable=0, reset to normal flow.
 *
 * Retn : TRUE  : Apply successed
 *        FALSE : Apply failed
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_EnableBypassYUV420Engine(unsigned char enable)
{
    unsigned char port = newbase_hdmi_get_current_display_port();

    lib_hdmi_force_bypass_yuv420_engine_en(port, enable);
    return true;
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_GetColorimetry
 *
 * Desc : Get Color Colorimetry
 *
 * Parm : N/A
 *
 * Retn : HDMI_COLORIMETRY_NOSPECIFIED = 0,
 *        HDMI_COLORIMETRY_601,
 *        HDMI_COLORIMETRY_709,
 *        HDMI_COLORIMETRY_XYYCC601,
 *        HDMI_COLORIMETRY_XYYCC709,
 *        HDMI_COLORIMETRY_SYCC601,
 *        HDMI_COLORIMETRY_ADOBE_YCC601,
 *        HDMI_COLORIMETRY_ADOBE_RGB,
 *------------------------------------------------------*/
HDMI_COLORIMETRY_T drvif_Hdmi_GetColorimetry(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (HDMI_COLORIMETRY_T)newbase_hdmi_get_colorimetry(port);
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_GetColorDepth
 *
 * Desc : Get Color Depth
 *
 * Parm : N/A
 *
 * Retn : HDMI_COLOR_DEPTH_8B =0,
 *        HDMI_COLOR_DEPTH_10B,
 *        HDMI_COLOR_DEPTH_12B,
 *        HDMI_COLOR_DEPTH_16B,
 *------------------------------------------------------*/
HDMI_COLOR_DEPTH_T drvif_Hdmi_GetColorDepth(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (HDMI_COLOR_DEPTH_T)newbase_hdmi_get_colordepth(port);
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_GetRgbYuvRangeMode
 *
 * Desc : Get Range of RGB/YUV of current HDMI port. this function will
 *        be called by
 *
 * Parm : N/A
 *
 * Retn : MODE_RAG_DEFAULT
 *        MODE_RAG_LIMIT
 *        MODE_RAG_LIMIT
 *------------------------------------------------------*/
HDMI_RGB_YUV_RANGE_MODE_T drvif_Hdmi_GetRgbYuvRangeMode(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (HDMI_RGB_YUV_RANGE_MODE_T)newbase_hdmi_get_color_range(port);
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_GetReal3DFomat
 *
 * Desc : Get Real 3D format of current HDMI port
 *
 * Parm : N/A
 *
 * Retn :  HDMI3D_FRAME_PACKING /
 *         HDMI3D_FIELD_ALTERNATIVE
 *         HDMI3D_LINE_ALTERNATIVE = 2,
 *         HDMI3D_SIDE_BY_SIDE_FULL = 3,
 *         HDMI3D_L_DEPTH = 4,
 *         HDMI3D_L_DEPTH_GPX = 5,
 *         HDMI3D_TOP_AND_BOTTOM =6,
 *         HDMI3D_FRAMESEQUENCE=7,
 *         HDMI3D_SIDE_BY_SIDE_HALF = 8,
 *         // --- new 3D format ---
 *         HDMI3D_LINE_BY_LINE = 9,
 *         HDMI3D_VERTICAL_STRIPE = 10,
 *         HDMI3D_CHECKER_BOARD = 11,
 *         HDMI3D_REALID = 12,
 *         HDMI3D_SENSIO = 13,
 *         // -------------------
 *         HDMI3D_RSV1,
 *         HDMI3D_2D_ONLY,
 *         ....
 *------------------------------------------------------*/
HDMI_3D_T drvif_Hdmi_GetReal3DFomat(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (HDMI_3D_T)newbase_hdmi_get_3d_structure(port);
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_CheckStableBeforeDisplay
 *
 * Desc : Check Video is stable before display. This function will be
 *        called by VSC driver before VSC start display video.
 *
 * Parm : N/A
 *
 * Retn : TRUE  : HDMI video is stable
 *        FALSE : HDMI video is not stable
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_CheckStableBeforeDisplay(void)
{
    return newbase_hdmi_check_stable_before_display();
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_GetCurrentPhysicalPort
 *
 * Desc : Get current physical display port from HDMI driver.
 *
 * Parm : N/A
 *
 * Retn : current physical display port 
 *------------------------------------------------------*/
unsigned char  drvif_Hdmi_GetCurrentPhysicalPort(void)
{
    return newbase_hdmi_get_current_display_port();
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_IsAvmute
 *
 * Desc : Check AVMUTE status on current port
 *
 * Parm : N/A
 *
 * Retn : TRUE  : AVMUTE on is from HDMI MAC 
 *        FALSE :  AVMUTE off is from HDMI MAC 
 *------------------------------------------------------*/
 HDMI_bool drvif_Hdmi_IsAvmute(void)
 {
     unsigned char port = newbase_hdmi_get_current_display_port();

     return (lib_hdmi_get_avmute(port) && !lib_hdmi_get_bch_2bit_error(port));
 }

/*------------------------------------------------------
 * Func : drvif_Hdmi_Set_Avmute_WatchDog
 *
 * Desc : Enable/Disable HW AVMUTE watchdog
 *
 * Parm : enable=1, Enable AVMUTE WD; enable=0, Clear AVMUTE WD
 *
 * Retn : TRUE  : set AVMUTE WD success
 *        FALSE :  set AVMUTE WD fail
 *------------------------------------------------------*/
 HDMI_bool drvif_Hdmi_Set_Avmute_WatchDog(unsigned char enable)
 {
     unsigned char port = newbase_hdmi_get_current_display_port();

     return newbase_hdmi_set_avmute_wd_en(port, enable);
 }

//-------------------------------------------------------------------------
//  Audio Related
//-------------------------------------------------------------------------


/*------------------------------------------------------
 * Func : drvif_Hdmi_IsAudioLock
 *
 * Desc : Check HDMI Audio is locked.
 *
 * Parm : N/A
 *
 * Retn : TRUE  : HDMI Audio is locked
 *        FALSE : HDMI Audio is not locked
 *------------------------------------------------------*/
char drvif_Hdmi_IsAudioLock(void)
{
    return (newbase_hdmi_audio_get_current_status() != AUDIO_FSM_AUDIO_START);
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_Audio_Get_N
 *
 * Desc : Get Audio Clock Regeneration N value.
 *
 * Parm : N/A
 *
 * Retn : ACR N value
 *------------------------------------------------------*/
unsigned int drvif_Hdmi_Audio_Get_N(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (lib_hdmi_audio_get_n(port));
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_Audio_Get_Cts
 *
 * Desc : Get Audio Clock Regeneration CTS value.
 *
 * Parm : N/A
 *
 * Retn : ACR CTS value
 *------------------------------------------------------*/
unsigned int drvif_Hdmi_Audio_Get_Cts(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (lib_hdmi_audio_get_cts(port));
}


//-------------------------------------------------------------------------
//  Info Pack Related
//-------------------------------------------------------------------------


/*------------------------------------------------------
 * Func : drvif_Hdmi_GetVsInfoFrame
 *
 * Desc : Get Vendor Specific Info Frame of currenct HDMI port
 *
 * Parm : pVsInfo : Vendor specific info frame output
 *
 * Retn : TRUE  : successed
 *        FALSE : failed
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_GetVsInfoFrame(HDMI_VSI_T *pVsInfo)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_vs_infoframe(port, pVsInfo);
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_GetDolbyVsInfoFrame
 *
 * Desc : Get DolbyVision Vendor Specific Info Frame of currenct HDMI port
 *
 * Parm : info_frame : DolbyVision Vendor specific info frame output
 *
 * Retn : TRUE  : successed
 *        FALSE : failed
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_GetDolbyVsInfoFrame(vfe_hdmi_vsi_t *info_frame)
{
    HDMI_VSI_T vsi_info;
    unsigned char port = newbase_hdmi_get_current_display_port();

    memset(info_frame, 0, sizeof(vfe_hdmi_vsi_t));

    if(0 == newbase_hdmi_get_dvs_infoframe(port, &vsi_info))
        return false;

    info_frame->header.type    = vsi_info.VSIF_TypeCode;
    info_frame->header.version = vsi_info.VSIF_Version;
    info_frame->header.length  = vsi_info.Length;
    info_frame->checksum       = vsi_info.Checksum;

    memcpy(info_frame->ieee_reg_id, vsi_info.Reg_ID, VFE_HDMI_VENDOR_SPECIFIC_REGID_LEN);
    memcpy(info_frame->payload, vsi_info.Payload, VFE_HDMI_VENDOR_SPECIFIC_PAYLOAD_LEN) ;

    info_frame->vidoe_fmt      = newbase_hdmi_get_video_format(port);
    info_frame->struct_3d      = newbase_hdmi_get_3d_structure(port);
    info_frame->extdata_3d     = lib_hdmi_get_3d_extdata(port);
    info_frame->vic            = lib_hdmi_get_vsi_vic(port);;
    info_frame->packetStatus   = VFE_HDMI_PACKET_STATUS_UPDATED;

    return true;
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_GetHDR10pVsInfoFrame
 *
 * Desc : Get HDR10+ Vendor Specific Info Frame of currenct HDMI port
 *
 * Parm : info_frame : HDR10+ Vendor specific info frame output
 *
 * Retn : TRUE  : successed
 *        FALSE : failed
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_GetHDR10pVsInfoFrame(vfe_hdmi_vsi_t *info_frame)
{
    HDMI_VSI_T vsi_info;
    unsigned char port = newbase_hdmi_get_current_display_port();

    memset(info_frame, 0, sizeof(vfe_hdmi_vsi_t));

    if(0 == newbase_hdmi_get_hdr10p_infoframe(port, &vsi_info))
        return false;

    info_frame->header.type    = vsi_info.VSIF_TypeCode;
    info_frame->header.version = vsi_info.VSIF_Version;
    info_frame->header.length  = vsi_info.Length;
    info_frame->checksum       = vsi_info.Checksum;

    memcpy(info_frame->ieee_reg_id, vsi_info.Reg_ID, VFE_HDMI_VENDOR_SPECIFIC_REGID_LEN);
    memcpy(info_frame->payload, vsi_info.Payload, VFE_HDMI_VENDOR_SPECIFIC_PAYLOAD_LEN) ;

    info_frame->vidoe_fmt      = newbase_hdmi_get_video_format(port);
    info_frame->struct_3d      = newbase_hdmi_get_3d_structure(port);
    info_frame->extdata_3d     = lib_hdmi_get_3d_extdata(port);
    info_frame->vic            = lib_hdmi_get_vsi_vic(port);;
    info_frame->packetStatus   = VFE_HDMI_PACKET_STATUS_UPDATED;

    return true;
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_GetAviInfoFrame
 *
 * Desc : Get AVI Info Frame of currenct HDMI port
 *
 * Parm : pAviInfo : AVI info frame output (16 bytes)
 *
 * Retn : TRUE  : successed
 *        FALSE : failed
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_GetAviInfoFrame(HDMI_AVI_T *pAviInfo)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_avi_infoframe(port, pAviInfo);
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_GetSpdInfoFrame
 *
 * Desc : Get SPD Info Frame of currenct HDMI port
 *
 * Parm : pSpdInfo : SPD info frame output (28 bytes)
 *
 * Retn : TRUE  : successed
 *        FALSE : failed
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_GetSpdInfoFrame(HDMI_SPD_T *pSpdInfo)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_spd_infoframe(port, pSpdInfo);
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_GetAudioInfoFrame
 *
 * Desc : Get Audio Info Frame of currenct HDMI port
 *
 * Parm : pAudioInfo : Audio info frame output (13 bytes)
 *
 * Retn : TRUE  : successed
 *        FALSE : failed
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_GetAudioInfoFrame(HDMI_AUDIO_T *pAudioInfo)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_audio_infoframe(port, pAudioInfo);
}


/*------------------------------------------------------
 * Func : drvif_Hdmi_IsDRMInfoReady
 *
 * Desc : Check if DRM info is received
 *
 * Parm : N/A
 *
 * Retn : TRUE  : DRM info is ready
 *        FALSE : DRM info is not ready
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_IsDRMInfoReady(void)
{
    return newbase_hdmi_is_drm_info_ready();
}
EXPORT_SYMBOL(drvif_Hdmi_IsDRMInfoReady);



/*------------------------------------------------------
 * Func : drvif_Hdmi_GetDrmInfoFrame
 *
 * Desc : Get DRM Info Frame of currenct HDMI port
 *
 * Parm : pAudioInfo : Audio info frame output (29 bytes)
 *
 * Retn : TRUE  : successed
 *        FALSE : failed
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_GetDrmInfoFrame(HDMI_DRM_T *pDrmInfo)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_drm_infoframe(port, pDrmInfo);
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_GetEmpPacket
 *
 * Desc : Get Specified EMP packet Info Frame of currenct HDMI port
 *
 * Parm : type: specified emp packet type, pkt_buf: array pointer, len: input array length
 *
 * Retn : >0  : real length of specified emp type packet
 *        =0 : get data fail
 *------------------------------------------------------*/
int drvif_Hdmi_GetEmpPacket(vfe_hdmi_em_type_e type, unsigned char* pkt_buf, unsigned short len)
{
	unsigned char port = newbase_hdmi_get_current_display_port();

	if (pkt_buf == NULL)
	{
		HDMI_EMG("[drvif_Hdmi_GetEmpPacket port:%d] type=%d, len=%d, pkt_buf is NULL \n", port, type, len);
		return 0;
	}

	switch(type)
	{
	case KADP_VFE_HDMI_EMP_VSEMDS:		
		return (len >= EM_VSEM_INFO_LEN) ? newbase_hdmi_get_vsem_emp(port, pkt_buf) : FALSE;

	case KADP_VFE_HDMI_EMP_HDRDM:		
		return (len >= MAX_EM_HDR_INFO_LEN) ? newbase_hdmi_get_hdr_emp(port, pkt_buf) : FALSE;

	case KADP_VFE_HDMI_EMP_VTEM:		
		return (len >= EM_VTEM_INFO_LEN) ? newbase_hdmi_get_vtem_emp(port, pkt_buf) : FALSE;

	case KADP_VFE_HDMI_EMP_CVTEM:
		return (len >= EM_CVTEM_INFO_LEN) ? newbase_hdmi_get_cvtem_emp(port, pkt_buf) : FALSE;

	default:
		return 0;
	}
}

HDMI_bool drvif_Hdmi_GetVRREnable(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_vrr_enable(port);
}
EXPORT_SYMBOL(drvif_Hdmi_GetVRREnable);

HDMI_bool drvif_Hdmi_GetQMSEnable(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_qms_enable(port);
}

HDMI_bool drvif_Hdmi_GetMConst(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_m_const(port);
}

HDMI_VTEM_NEXT_TFR drvif_Hdmi_GetNextTFR(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (HDMI_VTEM_NEXT_TFR)newbase_hdmi_get_next_tfr(port);
}

HDMI_QMS_STATUS_T* drvif_Hdmi_GetQmsStatus(unsigned short* current_vfreq)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_qms_status(port); 
}

HDMI_bool drvif_Hdmi_GetAMDFreeSyncStatus(HDMI_AMD_FREE_SYNC_INFO* p_info)
{
    unsigned char port = newbase_hdmi_get_current_display_port();

    if (p_info!=NULL && newbase_hdmi_get_freesync_info(port, p_info)==TRUE)
        return TRUE;

    return FALSE;
}

HDMI_bool drvif_Hdmi_GetAMDFreeSyncEnable(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return newbase_hdmi_get_freesync_enable(port);
}
EXPORT_SYMBOL(drvif_Hdmi_GetAMDFreeSyncEnable);

HDMI_bool drvif_Hdmi_GetALLMEnable(void)
{
    unsigned char allm_mode = 0;
    unsigned char port = newbase_hdmi_get_current_display_port();
    
    if (newbase_hdmi_get_fvs_allm(port, &allm_mode) && allm_mode )
        return TRUE;
    
    return FALSE;
}

HDMI_bool drvif_Hdmi_GetDolbyL11Content(unsigned char get_data[4])
{
    unsigned char port = newbase_hdmi_get_current_display_port();

    if(newbase_hdmi_get_dolby_L11_content(port, get_data))
        return TRUE;
    else
        return FALSE;
}

HDMI_bool drvif_Hdmi_SetDolbyVsifUpdateNoPktThreshold(unsigned char no_pkt_thd)
{
    INFO_PACKET_CONFIG* dvs_pkt_config = newbase_hdmi_get_infopacket_config(INFO_TYPE_DVS);

    if(dvs_pkt_config == NULL)
        return FALSE;

    dvs_pkt_config->no_pkt_thd = no_pkt_thd;

    return TRUE;
}


/**********************************************************************************************
*
*   HDCP Related
*
**********************************************************************************************/


/*------------------------------------------------------
 * Func : drvif_Hdmi_HdcpEnabled
 *
 * Desc : Check if HDCP enabled on current HDMI port
 *
 * Parm : N/A
 *
 * Retn : TRUE  : successed
 *        FALSE : failed
 *------------------------------------------------------*/
unsigned char drvif_Hdmi_HdcpEnabled(void)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    return (newbase_hdcp_get_auth_mode(port)!=HDCP_OFF);
}

/*------------------------------------------------------
 * Func : drvif_Hdmi_SetClkPixelMode
 *
 * Desc : Set clock pixel mode
 *
 * Parm : pixel_mode=0, 1 pixel mode; pixel_mode=1, 2 pixel mode;
 *
 * Retn : TRUE  : Apply successed
 *        FALSE : Apply failed
 *------------------------------------------------------*/
HDMI_bool drvif_Hdmi_SetClkPixelMode(unsigned char pixel_mode)
{
    unsigned char port = newbase_hdmi_get_current_display_port();
    unsigned char set_mode = 0;

    switch(pixel_mode)
    {
    case 0: // signal pixel mode
        set_mode = DEFAULT_HDMI_SINGLE_PIXEL_MODE; // 1 pixel mode
        break;

    case 1: // multi pixel mode
        set_mode = DEFAULT_HDMI_MULTI_PIXEL_MODE; // 2 pixel mode
        break;

    default:
        HDMI_EMG("[drvif_Hdmi_SetClkPixelMode port=%d] invalid pixel mode =%d\n", port, pixel_mode);
        return FALSE; // invalid value
    }

    newbase_hdmi_hd21_clk_gen(port, newbase_hdmi_is_run_dsc(port), set_mode);

    return TRUE;
}

HDMI_bool drvif_Hdmi_GetRawTimingInfo(HDMI_TIMING_T* timing_info)
{
    unsigned char port = newbase_hdmi_get_current_display_port();

    if (timing_info == NULL || port>=HDMI_PORT_TOTAL_NUM)
    {
        HDMI_EMG("[drvif_Hdmi_GetRawTimingInfo] timing_info is NULL \n");
        return FALSE;
    }

    memset(timing_info, 0, sizeof(HDMI_TIMING_T));

    newbase_hdmi_get_timing(port, timing_info);

    return TRUE;
}

void drvif_hdmi_wakeup_by_tmds_en(unsigned char en)
{
    newbase_hdmi_wakeup_by_tmds_en(en);
}


unsigned int drvif_Hdmi_CheckHdmiOnmsMask(void)
{// return >0: onms setting has been modified, return 0: onms setting has not been modified.
    unsigned char port = newbase_hdmi_get_current_display_port(); //if not support, onms_channel=0/2 always map to hdmi current port.
    return newbase_hdmi_check_onms_mask(port);
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void drvif_Hdmi_set_infoframe_thread_stop(unsigned char ucStop)
{
    hdmi_set_infoframe_thread_stop(ucStop);
}

void drvif_Hdmi_set_infoframe_thread_stop_nonseamphore_for_isr(unsigned char ucStop)
{// for ISR use
    hdmi_set_infoframe_thread_stop_nonseamphore(ucStop);
}

unsigned char drvif_Hdmi_get_infoframe_thread_stop(void)
{
    return hdmi_get_infoframe_thread_stop();
}

void drvif_Hdmi_infoframe_update_for_isr(void)
{//only work for current port
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
		
    if (TRUE == hdmi_get_infoframe_thread_stop())
    {
        newbase_hdmi_infoframe_update();
    }
    else
    {
        HDMI_WARN("[drvif_Hdmi_infoframe_update_for_isr] Invalid calling.\n");
    }
#else
    newbase_hdmi_infoframe_update();
#endif
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
