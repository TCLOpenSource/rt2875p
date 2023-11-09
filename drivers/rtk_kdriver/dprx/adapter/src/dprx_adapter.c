/*=============================================================
 * File:    dprx_adapter.c
 *
 * Desc:    DPRX adapter for RTK TV
 *
 * AUTHOR:  kevin_wang@realtek.com
 *
 * Vresion: 0.0.1
 *
 *------------------------------------------------------------
 * Copyright (c) Realtek Semiconductor Corporation, 2021
 *
 * All rights reserved.
 *
 *============================================================*/
#include <dprx_adapter.h>


//----------------------------------------------------------------------------------------
// DPRX Operations
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : dprx_adp_init
 *
 * Desc : init DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int dprx_adp_init(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.init)
        return p_dev->ops.init(&p_dev->adp);
    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_open
 *
 * Desc : open DPRX adapter. after open, the DPRX
 *        adapter will become operational. caller
 *        should call open before any other operations
 *        excepts init/uninit
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int dprx_adp_open(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.open)
        return p_dev->ops.open(&p_dev->adp);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_close
 *
 * Desc : close DPRX adapter. after close, the DPRX
 *        adapter will stop operation.
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int dprx_adp_close(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.close)
        return p_dev->ops.close(&p_dev->adp);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_uninit
 *
 * Desc : unint DPRX adapter. this function should
 *        be called when adapter to be destroyed
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int dprx_adp_uninit(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.uninit)
        return p_dev->ops.uninit(&p_dev->adp);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_adp_connect
 *
 * Desc : connect DPRX adapter. after connect, the
 *        DPRX adapter will begin audio/video format
 *        detection and able to output audio/video
 *        after detect.
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int dprx_adp_connect(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.connect)
        return p_dev->ops.connect(&p_dev->adp);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_disconnect
 *
 * Desc : disable DPRX adapter. after detect, the
 *        DPRX adapter detect/output audio/video data
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
int dprx_adp_disconnect(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.disconnect)
        return p_dev->ops.disconnect(&p_dev->adp);

    return 0;
}

// VFE related

/*------------------------------------------------
 * Func : dprx_adp_get_capability
 *
 * Desc : get capability of rx port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_cap : DP port capability output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_capability(dprx_dev_t* p_dev, DP_PORT_CAPABILITY* p_cap)
{
    if (p_dev==NULL || p_cap==NULL)
        return -1;

    if (p_dev->ops.get_capability)
        return p_dev->ops.get_capability(&p_dev->adp, p_cap);

    // report default capability
    memset(p_cap, 0, sizeof(DP_PORT_CAPABILITY));
    p_cap->type = DP_TYPE_mDP;
    p_cap->lane_cnt = DP_LANE_MODE_4_LANE;
    p_cap->link_rate_mask = DP_LINK_RATE_RBR_MASK;
    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_get_link_status
 *
 * Desc : get link status of rx port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_status : link status output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_link_status(dprx_dev_t* p_dev, DP_LINK_STATUS_T* p_status)
{
    if (p_dev==NULL || p_status==NULL)
        return -1;

    if (p_dev->ops.get_link_status)
        return p_dev->ops.get_link_status(&p_dev->adp, p_status);

    // report unlocked link status
    memset(p_status, 0, sizeof(DP_LINK_STATUS_T));
    p_status->lane_mode = DP_LANE_MODE_4_LANE;
    p_status->link_rate = DP_LINK_RATE_HBR3_8p1G;
    p_status->channel_coding = DP_CHANNEL_CODING_8b10b;
    p_status->fec_enable = 0;
    p_status->scarmbling_enable = 0;
    p_status->channel_align = 0;
    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_get_stream_type
 *
 * Desc : get stream type of rx port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_type : stream type output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_stream_type(dprx_dev_t* p_dev, DP_STREAM_TYPE_E* p_type)
{
    if (p_dev==NULL || p_type==NULL)
        return -1;

    if (p_dev->ops.get_stream_type)
        return p_dev->ops.get_stream_type(&p_dev->adp, p_type);

    *p_type = DPRX_STREAM_TYPE_AUDIO_VIDEO;
    return 0;
}

// HPD/Detect
/*------------------------------------------------
 * Func : dprx_adp_get_connect_status
 *
 * Desc : get cable connection status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
int dprx_adp_get_connect_status(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.get_connect_status)
        return p_dev->ops.get_connect_status(&p_dev->adp);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_get_aux_status
 *
 * Desc : get aux connection status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : aux disconnect, 1 : aux connected
 *-----------------------------------------------*/
int dprx_adp_get_aux_status(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.get_aux_status)
        return p_dev->ops.get_aux_status(&p_dev->adp);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_set_hpd
 *
 * Desc : set/unset HPD signal of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *        on    : 0 : HPD low, others : HPD high
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_set_hpd(dprx_dev_t* p_dev, unsigned char on)
{
    if (p_dev && p_dev->ops.set_hpd)
        return p_dev->ops.set_hpd(&p_dev->adp, on);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_adp_set_hpd_toggle
 *
 * Desc : set HPD toggle signal of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *        delay_ms  : the Interval of HPD toggle
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_set_hpd_toggle(dprx_dev_t* p_dev, unsigned short delay_ms)
{
    if (p_dev && p_dev->ops.set_hpd)
        return p_dev->ops.set_hpd_toggle(&p_dev->adp, delay_ms);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_get_hpd
 *
 * Desc : get HPD status of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : HPD low, 1 : HPD high
 *-----------------------------------------------*/
int dprx_adp_get_hpd(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.get_hpd)
        return p_dev->ops.get_hpd(&p_dev->adp);

    return 0;
}

//===============================================================================
// EDID related
//===============================================================================


/*------------------------------------------------
 * Func : dprx_adp_set_edid
 *
 * Desc : set EDID of the DPRX adapter
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_set_edid(dprx_dev_t* p_dev, unsigned char* p_edid, unsigned short edid_len)
{
    if (p_dev && p_dev->ops.set_edid)
        return p_dev->ops.set_edid(&p_dev->adp, p_edid, edid_len);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_get_edid
 *
 * Desc : set EDID of the DPRX adapter
 *
 * Para : p_dev  : handle of DPRX adapter
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_edid(dprx_dev_t* p_dev, unsigned char* p_edid, unsigned short edid_len)
{
    if (p_dev && p_dev->ops.get_edid)
        return p_dev->ops.get_edid(&p_dev->adp, p_edid, edid_len);

    return 0;
}

//===============================================================================
// DPCD related
//===============================================================================

/*------------------------------------------------
 * Func : dprx_adp_set_dpcd
 *
 * Desc : set DPCD of the DPRX adapter
 *
 * Para : p_dev   : handle of DPRX adapter
 *        address : dpcd address
 *        p_dpcd  : dpcd data
 *        len     : size of dpcd data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_set_dpcd(
    dprx_dev_t*     p_dev,
    unsigned long   address,
    unsigned char*  p_dpcd,
    unsigned short  len
    )
{
    if (p_dev && p_dev->ops.set_dpcd)
        return p_dev->ops.set_dpcd(&p_dev->adp, address, p_dpcd, len);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_adp_get_dpcd
 *
 * Desc : get DPCD of the DPRX adapter
 *
 * Para : p_dev   : handle of DPRX adapter
 *        address : dpcd address
 *        p_dpcd  : dpcd data
 *        len     : size of dpcd data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_dpcd(
    dprx_dev_t*     p_dev,
    unsigned long   address,
    unsigned char*  p_dpcd,
    unsigned short  len
    )
{
    if (p_dev && p_dev->ops.get_dpcd)
        return p_dev->ops.get_dpcd(&p_dev->adp, address, p_dpcd, len);

    return 0;
}

//===============================================================================
// SDP related
//===============================================================================


/*------------------------------------------------
 * Func : dprx_adp_get_vbid_info
 *
 * Desc : get vbid information from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_vbid : vbid data output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_vbid_info(
    dprx_dev_t*         p_dev,
    DPRX_VBID_INFO_T*   p_vbid
    )
{
    if (p_dev==NULL || p_vbid==NULL)
        return -1;

    memset(p_vbid, 0, sizeof(DPRX_VBID_INFO_T));

    if (p_dev && p_dev->ops.get_vbid_data)
        return p_dev->ops.get_vbid_data(&p_dev->adp, p_vbid);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_get_msa_info
 *
 * Desc : get msa information from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_msa : msa data output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_msa_info(dprx_dev_t* p_dev, DPRX_MSA_INFO_T* p_msa)
{
    if (p_dev==NULL || p_msa==NULL)
        return -1;

    memset(p_msa, 0, sizeof(DPRX_MSA_INFO_T));

    if (p_dev && p_dev->ops.get_msa_data)
        return p_dev->ops.get_msa_data(&p_dev->adp, p_msa);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_adp_get_sdp_data
 *
 * Desc : get sdp data from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        type   : SDP type
 *        p_data : spd data output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_sdp_data(
    dprx_dev_t*         p_dev,
    DP_SDP_TYPE         type,
    DP_SDP_DATA_T*      p_data
    )
{
    if (p_data==NULL)
        return -1;

    memset(p_data, 0, sizeof(DP_SDP_DATA_T));

    if (p_dev && p_dev->ops.get_sdp_data)
        return p_dev->ops.get_sdp_data(&p_dev->adp, type, p_data);

    return -1;
}


//===============================================================================
// Video related
//===============================================================================

/*------------------------------------------------
 * Func : dprx_adp_get_drm_info
 *
 * Desc : get drm information from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_drm_info : drm info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_drm_info(dprx_dev_t* p_dev, DPRX_DRM_INFO_T *p_drm_info)
{
    if (p_drm_info==NULL)
        return -1;

    memset(p_drm_info, 0, sizeof(DPRX_DRM_INFO_T));

    if (p_dev && p_dev->ops.get_drm_info)
        return p_dev->ops.get_drm_info(&p_dev->adp, p_drm_info);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_get_avi_info
 *
 * Desc : get avi information from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_avi_info : avi info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_avi_info(dprx_dev_t* p_dev, DPRX_AVI_INFO_T *p_avi_info)
{
    if (p_avi_info==NULL)
        return -1;

    memset(p_avi_info, 0, sizeof(DPRX_AVI_INFO_T));

    if (p_dev && p_dev->ops.get_avi_info)
        return p_dev->ops.get_avi_info(&p_dev->adp, p_avi_info);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_get_pixel_encoding_info
 *
 * Desc : get pixel encoding format from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_pixel_info : pixel encoding info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_pixel_encoding_info(dprx_dev_t* p_dev, DP_PIXEL_ENCODING_FORMAT_T *p_pixel_info)
{
    if (p_pixel_info==NULL)
        return -1;

    memset(p_pixel_info, 0, sizeof(DP_PIXEL_ENCODING_FORMAT_T));

    if (p_dev && p_dev->ops.get_pixel_encoding_info)
        return p_dev->ops.get_pixel_encoding_info(&p_dev->adp, p_pixel_info);

    return 0;
}


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_timing_info
 *
 * Desc : write edid to a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_timing_info : timing info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_video_timing(dprx_dev_t* p_dev, DPRX_TIMING_INFO_T *p_timing_info)
{
    if (p_timing_info==NULL)
        return -1;

    memset(p_timing_info, 0, sizeof(DPRX_TIMING_INFO_T));

    if (p_dev && p_dev->ops.get_video_timing)
        return p_dev->ops.get_video_timing(&p_dev->adp, p_timing_info);

    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_source_check
 *
 * Desc : Check input source
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_source_check(dprx_dev_t* p_dev)
{
    if (p_dev==NULL)
        return -1;

    if (p_dev->ops.detect_task)
        return p_dev->ops.detect_task(&p_dev->adp);
    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_get_avmute
 *
 * Desc : get AVMUTE status of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 :No Mute, 1 : AV Mute
 *-----------------------------------------------*/
int dprx_adp_get_avmute(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.get_avmute)
        return p_dev->ops.get_avmute(&p_dev->adp);

    return 0;
}


//===============================================================================
// Audio related
//===============================================================================

/*------------------------------------------------
 * Func : dprx_adp_get_audio_status
 *
 * Desc : get audio status of current DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_status : audio status
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_audio_status(dprx_dev_t* p_dev, DP_AUDIO_STATUS_T *p_status)
{
    if (p_status==NULL)
        return -1;

    if (p_dev && p_dev->ops.get_audio_status)
        return p_dev->ops.get_audio_status(&p_dev->adp, p_status);

    p_status->coding_type = DP_AUDIO_CODING_NO_AUDIO;
    p_status->ch_num = 0;
    p_status->sampling_frequency_khz_x100 = 0;
    return 0;
}


//===============================================================================
// HDCP related
//===============================================================================


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_hdcp_capability
 *
 * Desc : get hdcp capability pf DPRX VFE
 *
 * Para : p_hdcp_cap : bit mask of HDCP capability
 *          DP_HDCP_CAPBILITY_HDCP13
 *          DP_HDCP_CAPBILITY_HDCP22
 *          DP_HDCP_CAPBILITY_HDCP23
 *          DP_HDCP_CAPBILITY_HDCP_REPEATER
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_hdcp_capability(dprx_dev_t* p_dev, unsigned int *p_hdcp_cap)
{
    if (p_hdcp_cap==NULL)
        return -1;

    if (p_dev && p_dev->ops.get_hdcp_capability)
        return p_dev->ops.get_hdcp_capability(&p_dev->adp, p_hdcp_cap);

    return 0;
}


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_hdcp_status
 *
 * Desc : get hdcp status of a DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_hdcp_status : hdcp status
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_hdcp_status(dprx_dev_t* p_dev, DPRX_HDCP_STATUS_T *p_hdcp_status)
{
    if (p_hdcp_status==NULL)
        return -1;

    if (p_dev && p_dev->ops.get_hdcp_status)
        return p_dev->ops.get_hdcp_status(&p_dev->adp, p_hdcp_status);

    memset(p_hdcp_status, 0, sizeof(DPRX_HDCP_STATUS_T));
    return 0;
}

//----------------------------------------------------------------------------------------
// Power Management API
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : dprx_adp_suspend
 *
 * Desc : suspend a dprx adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *        mode  : suspend mode
 *            WAKEUP_BY_AUX (enable wakeup AUX)
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_suspend(dprx_dev_t* p_dev, unsigned int mode)
{
    if (p_dev && p_dev->ops.suspend)
        return p_dev->ops.suspend(&p_dev->adp, mode);

    DPRX_ADP_INFO("dprx_adp_suspend without suspend ops, do nothing\n");
    return 0;
}


/*------------------------------------------------
 * Func : dprx_adp_resume
 *
 * Desc : resume a dprx adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_resume(dprx_dev_t* p_dev)
{
    if (p_dev && p_dev->ops.resume)
        return p_dev->ops.resume(&p_dev->adp);

    DPRX_ADP_INFO("dprx_adp_resume without resume ops, do nothing\n");
    return 0;
}

//===============================================================================
// Event Handler
//===============================================================================

/*------------------------------------------------
 * Func : dprx_adp_event_handler
 *
 * Desc : handle event
 *
 * Para : p_adp : handle of DPRX adapter
 *        event_type : external event
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_event_handler(
    dprx_dev_t*     p_dev,
    unsigned int    event_type
    )
{
    if (p_dev && p_dev->ops.event_handler)
        return p_dev->ops.event_handler(&p_dev->adp, event_type);

    DPRX_ADP_INFO("dprx_adp_event_handler without event_handler ops, do nothing\n");
    return 0;
}


//===============================================================================
// Extension Control
//===============================================================================

/*------------------------------------------------
 * Func : dprx_adp_set_ext_ctrl
 *
 * Desc : set ext control of a DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        ctrl : ext control command
 *        val : param val
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_set_ext_ctrl(
    dprx_dev_t*     p_dev,
    ADP_EXCTL_ID    ctrl,
    unsigned int    val
    )
{
    if (p_dev && p_dev->ops.set_ext_ctrl)
        return p_dev->ops.set_ext_ctrl(&p_dev->adp, ctrl, val);

    return -1;
}


/*------------------------------------------------
 * Func : dprx_adp_get_ext_ctrl
 *
 * Desc : get ext control of a DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        ctrl : ext control command
 *
 * Retn : >=0 : successed, 0< : failed
 *-----------------------------------------------*/
int dprx_adp_get_ext_ctrl(
    dprx_dev_t*     p_dev,
    ADP_EXCTL_ID    ctrl
    )
{
    if (p_dev && p_dev->ops.get_ext_ctrl)
        return p_dev->ops.get_ext_ctrl(&p_dev->adp, ctrl);

    return -1;
}

//===============================================================================
// Attribute Interface
//===============================================================================

/*------------------------------------------------
 * Func : dprx_adp_attr_show
 *
 * Desc : show adapter attr
 *
 * Para : p_adp : handle of DPRX adapter
 *        attr  : attribute
 *        p_buf : data buffer
 *        cnt   : buffer size
 *
 * Retn : number if bytes output
 *-----------------------------------------------*/
int dprx_adp_attr_show(
    dprx_dev_t*     p_dev,
    ADP_ATTR        attr,
    unsigned char*  p_buf,
    unsigned int    cnt)
{
    if (p_dev && p_dev->ops.attr_show)
        return p_dev->ops.attr_show(&p_dev->adp, attr, p_buf, cnt);

    return 0;
}

/*------------------------------------------------
 * Func : dprx_adp_attr_store
 *
 * Desc : store adapter attr
 *
 * Para : p_adp : handle of DPRX adapter
 *        attr  : attribute
 *        p_buf : data buffer
 *        cnt   : buffer size
 *
 * Retn : number if bytes handled
 *-----------------------------------------------*/
int dprx_adp_attr_store(
    dprx_dev_t*     p_dev,
    ADP_ATTR        attr,
    unsigned char*  p_buf,
    unsigned int    cnt)
{
    if (p_dev && p_dev->ops.attr_store)
        return p_dev->ops.attr_store(&p_dev->adp, attr, p_buf, cnt);

    return cnt;
}

//----------------------------------------------------------------------------------------
// Low Level API
//----------------------------------------------------------------------------------------


/*------------------------------------------------
 * Func : alloc_dprx_device
 *
 * Desc : alocate a dprx_device
 *
 * Para : N/A
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
dprx_dev_t* alloc_dprx_device(void)
{
    dprx_dev_t* p_dev = dprx_osal_malloc(sizeof(dprx_dev_t));

    if (p_dev)
        memset(p_dev, 0, sizeof(dprx_dev_t));

    return p_dev;
}


/*------------------------------------------------
 * Func : destroy_dprx_device
 *
 * Desc : dsatroy a dprx_device
 *
 * Para : N/A
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
void destroy_dprx_device(dprx_dev_t* p_dev)
{
    if (p_dev)
    {
        dprx_adp_close(p_dev);   // force stop operation
        dprx_adp_uninit(p_dev);  // unit all resources of DPRX
        dprx_osal_mfree(p_dev);  // free memeory of DPRX
    }
}
