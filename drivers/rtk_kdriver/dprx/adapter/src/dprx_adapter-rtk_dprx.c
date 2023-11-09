/*=============================================================
 * File:    dprx_adapter-rtk_dprx.c
 *
 * Desc:    Pseudo DPRX adapter for RTK TV
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
#include <dprx_platform.h>
#include <dprx_adapter-rtk_dprx.h>
#include <dprx_adapter-rtk_dprx-priv.h>
#ifdef CHECK_DPRX_OTP_SETTING
#include <rtk_kdriver/rtk_otp_region_api.h>
#endif
#ifdef CONFIG_DPRX_DRV_ENABLE_VIRTUAL_PLATFORM
#include <dprx_hpd-dprx_virt_plat.h>
#endif

static const UINT8 sink_specific_field[] =
{
    0x00, 0xE0, 0x4C,  // IEEE OUI
    'R', 'T', 'K', '-', 'T', 'V',  // Device Identification String
    0x00,   // Sink Hardware Version
    0x00, 0x01   // Sink FW Version (Major, Minor
};

UINT8 g_dprx_default_edid[] =
{
    0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x4a,0x8b,0x01,0x00,0x01,0x01,0x01,0x01,
    0x01,0x1a,0x01,0x04,0xb5,0xa0,0x5a,0x78,0x3b,0xee,0x91,0xa3,0x54,0x4c,0x99,0x26,
    0x0f,0x50,0x54,0xa1,0x08,0x00,0xd1,0xc0,0x71,0x4f,0x81,0xc0,0x81,0x40,0x81,0x80,
    0x95,0x00,0xb3,0x00,0x01,0x01,0x56,0x5e,0x00,0xa0,0xa0,0xa0,0x29,0x50,0x30,0x20,
    0x35,0x00,0xb9,0x88,0x21,0x00,0x00,0x1e,0x59,0xe7,0x00,0x6a,0xa0,0xa0,0x67,0x50,
    0x15,0x20,0x35,0x00,0xb9,0x88,0x21,0x00,0x00,0x1e,0x00,0x00,0x00,0xfd,0x00,0x3a,
    0x3e,0x1e,0x88,0x3c,0x00,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0xfc,
    0x00,0x52,0x54,0x4b,0x20,0x54,0x56,0x0a,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0xef,
    0x02,0x03,0x58,0xf1,0x58,0x61,0x60,0x66,0x65,0x10,0x1f,0x04,0x13,0x05,0x14,0x03,
    0x02,0x20,0x21,0x22,0x01,0x5d,0x5e,0x5f,0x62,0x63,0x64,0x3f,0x40,0x2c,0x09,0x57,
    0x07,0x15,0x07,0x50,0x57,0x07,0x01,0x67,0x04,0x03,0x65,0x03,0x0c,0x00,0x20,0x00,
    0xe2,0x00,0xd5,0xe3,0x05,0xc0,0x00,0xe6,0x06,0x05,0x01,0x70,0x70,0x07,0xeb,0x01,
    0x46,0xd0,0x00,0x4d,0x4b,0x7a,0x72,0x60,0x6f,0x75,0x6d,0x1a,0x00,0x00,0x02,0x01,
    0x30,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x6f,0xc2,0x00,0xa0,0xa0,0xa0,0x55,0x50,
    0x30,0x20,0x35,0x00,0xb9,0x88,0x21,0x00,0x00,0x1a,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf9
};


#define GET_RTK_DPRX_PORT(p_adp)      ((p_adp && p_adp->p_private) ? (RTK_DPRX_ADP*) p_adp->p_private : NULL)

//----------------------------------------------------------------------------------------
// DPRX thread operations
//----------------------------------------------------------------------------------------
dprx_adp_t* g_p_adp_dprx_current_port = NULL;
#ifdef DPRX_ASYNCHRONOUS_DETECT_TASK
static struct task_struct *p_adapter_dprx_timing_detect_tsk = NULL;
int g_dprx_check_mode_status = -1;
static int adapter_dprx_timing_detect_tsk(void *p)//This task run HDMI source timing detecting after source connect
{
    current->flags &= ~PF_NOFREEZE;
    while (1)
    {
        usleep_range(10*1000, 10*1000);

        if(g_p_adp_dprx_current_port != NULL)
        {
            if((rtk_dprx_get_connect_status(GET_RTK_DPRX_PORT(g_p_adp_dprx_current_port))) < 0)
            {
                g_dprx_check_mode_status = -1;
            }
            else
            {
                g_dprx_check_mode_status = rtk_dprx_source_check(GET_RTK_DPRX_PORT(g_p_adp_dprx_current_port));
            }
        }

        if (freezing(current))
        {
            try_to_freeze();
        }
        if (kthread_should_stop())
        {
            break;
        }
    }
    do_exit(0);
    return 0;
}

static int adapter_dprx_thread_enable(unsigned char enable)
{
    DPRX_ADP_INFO("adapter_dprx_thread_enable(%d)\n", enable);
    if (enable)
    {
        if (p_adapter_dprx_timing_detect_tsk==NULL)
        {
            p_adapter_dprx_timing_detect_tsk = kthread_create(adapter_dprx_timing_detect_tsk, NULL, "adapter_dprx_timing_detect_tsk");

            if (p_adapter_dprx_timing_detect_tsk)
            {
                DPRX_ADP_INFO("create adapter_dprx_timing_detect_tsk thread success\n");
                wake_up_process(p_adapter_dprx_timing_detect_tsk);
            }
            else
            {
                DPRX_ADP_ERR("create adapter_dprx_timing_detect_tsk thread fail\n");
                return -1;
            }
        }
    }
    else
    {
        if (p_adapter_dprx_timing_detect_tsk)
        {
            kthread_stop(p_adapter_dprx_timing_detect_tsk);
            p_adapter_dprx_timing_detect_tsk = NULL;
            DPRX_ADP_ERR("create adapter_dprx_timing_detect_tsk thread stop\n");
        }
    }
    return 0;
}
#endif


//----------------------------------------------------------------------------------------
// DPRX operations
//----------------------------------------------------------------------------------------
#ifdef CONFIG_ENABLE_TYPEC_DPRX

/*------------------------------------------------
 * Func : _update_adp_port_config
 *
 * Desc : update port config
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
void _update_adp_port_config(DPRX_PORT_CONFIG* port_cfg, DP_PHY_CFG* platform_phy_cfg, DP_AUX_CH_CFG* platform_aux_phy_cfg)
{
    port_cfg->aux_sel = platform_aux_phy_cfg->idx;
    port_cfg->aux_pn_swap = platform_aux_phy_cfg->pn_swap;
    port_cfg->phy_sel = platform_phy_cfg->idx;
    port_cfg->lane0_pn_swap = platform_phy_cfg->lane0_pn_swap;
    port_cfg->lane1_pn_swap = platform_phy_cfg->lane1_pn_swap;
    port_cfg->lane2_pn_swap = platform_phy_cfg->lane2_pn_swap;
    port_cfg->lane3_pn_swap = platform_phy_cfg->lane3_pn_swap;
    port_cfg->lane0_mapping = platform_phy_cfg->lane0_phy_idx;
    port_cfg->lane1_mapping = platform_phy_cfg->lane1_phy_idx;
    port_cfg->lane2_mapping = platform_phy_cfg->lane2_phy_idx;
    port_cfg->lane3_mapping = platform_phy_cfg->lane3_phy_idx;

    DP_PLAT_DBG("lane0_phy_idx = %d, lane1_phy_idx = %d, lane2_phy_idx = %d, lane3_phy_idx = %d, \
                    lane0_pn_swap = %d,  lane1_pn_swap = %d, lane2_pn_swap = %d, lane3_pn_swap = %d\n" , \
                    port_cfg->lane0_mapping, port_cfg->lane1_mapping, port_cfg->lane2_mapping, \
                    port_cfg->lane3_mapping, port_cfg->lane0_pn_swap, port_cfg->lane1_pn_swap, \
                    port_cfg->lane2_pn_swap, port_cfg->lane3_pn_swap);
}

#endif

/*------------------------------------------------
 * Func : _ops_dprx_init
 *
 * Desc : init DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_init(dprx_adp_t* p_adp)
{
#ifdef DPRX_ASYNCHRONOUS_DETECT_TASK
    adapter_dprx_thread_enable(1);
#endif
    return rtk_dprx_init(GET_RTK_DPRX_PORT(p_adp));
}


/*------------------------------------------------
 * Func : _ops_dprx_uninit
 *
 * Desc : unint DPRX adapter. this function should
 *        be called when adapter to be destroyed
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_uninit(dprx_adp_t* p_adp)
{
    RTK_DPRX_ADP* p_dprx = GET_RTK_DPRX_PORT(p_adp);

#ifdef DPRX_ASYNCHRONOUS_DETECT_TASK
    adapter_dprx_thread_enable(0);
#endif

    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("uninit rtk dprx port failed, p_dprx==NULL!!!! \n");
        return -1;
    }

    rtk_dprx_uninit(p_dprx);   // uninit dprx
    dprx_osal_mfree(p_dprx);   // free memory...
    return 0;
}


/*------------------------------------------------
 * Func : _ops_dprx_open
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
static int _ops_dprx_open(dprx_adp_t* p_adp)
{
    return rtk_dprx_open(GET_RTK_DPRX_PORT(p_adp));
}


/*------------------------------------------------
 * Func : _ops_dprx_close
 *
 * Desc : close DPRX adapter. after close, the DPRX
 *        adapter will stop operation.
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_close(dprx_adp_t* p_adp)
{
    return rtk_dprx_close(GET_RTK_DPRX_PORT(p_adp));
}


/*------------------------------------------------
 * Func : _ops_dprx_connect
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
static int _ops_dprx_connect(dprx_adp_t* p_adp)
{
    g_p_adp_dprx_current_port = p_adp;
    return rtk_dprx_connect(GET_RTK_DPRX_PORT(p_adp));
}


/*------------------------------------------------
 * Func : _ops_dprx_disconnect
 *
 * Desc : disable DPRX adapter. after detect, the
 *        DPRX adapter detect/output audio/video data
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_disconnect(dprx_adp_t* p_adp)
{
    g_p_adp_dprx_current_port = NULL;
    return rtk_dprx_disconnect(GET_RTK_DPRX_PORT(p_adp));
}


/*------------------------------------------------
 * Func : _ops_dprx_get_capability
 *
 * Desc : get dprx port hardware capability
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_capability(
    dprx_adp_t*         p_adp,
    DP_PORT_CAPABILITY* p_cap
    )
{
    return rtk_dprx_get_capability(GET_RTK_DPRX_PORT(p_adp), p_cap);
}


/*------------------------------------------------
 * Func : _ops_dprx_get_link_status
 *
 * Desc : get dprx port link status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_link_status(
    dprx_adp_t*         p_adp,
    DP_LINK_STATUS_T*   p_status
    )
{
    return rtk_dprx_get_link_status(GET_RTK_DPRX_PORT(p_adp), p_status);
}


/*------------------------------------------------
 * Func : _ops_dprx_get_stream_type
 *
 * Desc : get dprx port stream type
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, others : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_stream_type(
    dprx_adp_t*         p_adp,
    DP_STREAM_TYPE_E*   p_type
    )
{
    return rtk_dprx_get_stream_type(GET_RTK_DPRX_PORT(p_adp), p_type);
}


// HPD/Detect

/*------------------------------------------------
 * Func : _ops_dprx_get_connect_status
 *
 * Desc : get cable connection status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
static int _ops_dprx_get_connect_status(
    dprx_adp_t*         p_adp
    )
{
    return rtk_dprx_get_connect_status(GET_RTK_DPRX_PORT(p_adp));
}


/*------------------------------------------------
 * Func : _ops_dprx_get_connect_status
 *
 * Desc : get cable connection status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
static int _ops_dprx_get_aux_status(
    dprx_adp_t*         p_adp
    )
{
    return rtk_dprx_get_aux_status(GET_RTK_DPRX_PORT(p_adp));
}


/*------------------------------------------------
 * Func : _ops_dprx_set_hpd
 *
 * Desc : set/unset HPD signal of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *        on    : 0 : HPD low, others : HPD high
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_set_hpd(
    dprx_adp_t*         p_adp,
    unsigned char       on
    )
{
    return rtk_dprx_set_hpd(GET_RTK_DPRX_PORT(p_adp), on);
}

/*------------------------------------------------
 * Func : _ops_dprx_set_hpd_toggle
 *
 * Desc : set HPD toggle signal of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *        ms_delay  : the Interval of HPD toggle
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_set_hpd_toggle(
    dprx_adp_t*         p_adp,
    unsigned short      delay_ms
    )
{
    return rtk_dprx_set_hpd_toggle(GET_RTK_DPRX_PORT(p_adp), delay_ms);
}

/*------------------------------------------------
 * Func : _ops_dprx_get_hpd
 *
 * Desc : get HPD status of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : HPD low, 1 : HPD high
 *-----------------------------------------------*/
static int _ops_dprx_get_hpd(dprx_adp_t* p_adp)
{
    return rtk_dprx_get_hpd(GET_RTK_DPRX_PORT(p_adp));
}

// EDID operations

/*------------------------------------------------
 * Func : _ops_dprx_set_edid
 *
 * Desc : set EDID of the DPRX adapter
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_set_edid(
    dprx_adp_t*             p_adp,
    unsigned char*          p_edid,
    unsigned short          edid_len
    )
{
    return rtk_dprx_set_edid(GET_RTK_DPRX_PORT(p_adp), p_edid, edid_len);
}


/*------------------------------------------------
 * Func : _ops_dprx_get_edid
 *
 * Desc : get EDID of the DPRX adapter
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_edid : edid data
 *        edid_len : size of edid data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_edid(
    dprx_adp_t*             p_adp,
    unsigned char*          p_edid,
    unsigned short          edid_len
    )
{
    return rtk_dprx_get_edid(GET_RTK_DPRX_PORT(p_adp), p_edid, edid_len);
}


/*------------------------------------------------
 * Func : _ops_dprx_get_avi_info
 *
 * Desc : get dprx port video avi
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_avi : pointer of avi info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_avi_info(
    dprx_adp_t*         p_adp,
    DPRX_AVI_INFO_T*    p_avi
    )
{
    return -1;
}


/*------------------------------------------------
 * Func : _ops_dprx_get_pixel_encoding_info
 *
 * Desc : get dprx port video pixel encoding format
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_pixel : pointer of pixel encoding info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_pixel_encoding_info(
    dprx_adp_t*         p_adp,
    DP_PIXEL_ENCODING_FORMAT_T*    p_pixel
    )
{
    return rtk_dprx_get_pixel_encoding_info(GET_RTK_DPRX_PORT(p_adp), p_pixel);
}


/*------------------------------------------------
 * Func : _ops_dprx_get_drm_info
 *
 * Desc : get dprx port video drm
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_drm : pointer of drm info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_drm_info(
    dprx_adp_t*         p_adp,
    DPRX_DRM_INFO_T*    p_drm
    )
{
    return rtk_dprx_get_drm_info(GET_RTK_DPRX_PORT(p_adp), p_drm);
}


/*------------------------------------------------
 * Func : _ops_dprx_get_vbid_data
 *
 * Desc : get vbid data from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_vbid : VBID
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_vbid_data(
    dprx_adp_t*         p_adp,
    DPRX_VBID_INFO_T*   p_vbid
    )
{
    return rtk_dprx_get_vbid_data(GET_RTK_DPRX_PORT(p_adp), p_vbid);
}


/*------------------------------------------------
 * Func : _ops_dprx_get_msa_data
 *
 * Desc : get msa data from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_msa : MSA
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_msa_data(
    dprx_adp_t*         p_adp,
    DPRX_MSA_INFO_T*    p_msa
    )
{
    return rtk_dprx_get_msa_data(GET_RTK_DPRX_PORT(p_adp), p_msa);
}


/*------------------------------------------------
 * Func : _ops_dprx_get_sdp_data
 *
 * Desc : get sdp data from a given DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        type   : SDP type
 *        p_data : spd data output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_sdp_data(
    dprx_adp_t*         p_adp,
    DP_SDP_TYPE         type,
    DP_SDP_DATA_T*      p_data
    )
{
    return rtk_dprx_get_sdp_data(GET_RTK_DPRX_PORT(p_adp), type, p_data);
}

/*------------------------------------------------
 * Func : _ops_dprx_get_video_timing
 *
 * Desc : get dprx port video timing
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_timing : pointer of timing info
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_video_timing(
    dprx_adp_t*         p_adp,
    DPRX_TIMING_INFO_T* p_timing
    )
{
#ifdef DPRX_ASYNCHRONOUS_DETECT_TASK
    rtk_dprx_get_video_timing(GET_RTK_DPRX_PORT(p_adp), p_timing);
    return g_dprx_check_mode_status;
#else
    return rtk_dprx_get_video_timing(GET_RTK_DPRX_PORT(p_adp), p_timing);
#endif

}


/*------------------------------------------------
 * Func : _ops_dprx_source_check
 *
 * Desc : Check input source
 *
 * Para : p_adp  : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_source_check(
    dprx_adp_t*         p_adp
    )
{
#ifdef DPRX_ASYNCHRONOUS_DETECT_TASK
    return g_dprx_check_mode_status;                            //Check input source with asynchronous source
#else
    return rtk_dprx_source_check(GET_RTK_DPRX_PORT(p_adp));     //Check input source with synchronize source
#endif
}


/*------------------------------------------------
 * Func : _ops_dprx_get_audio_status
 *
 * Desc : get dprx port audio status
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_audio_status : pointer of hdcp status
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_audio_status(
    dprx_adp_t*         p_adp,
    DP_AUDIO_STATUS_T*  p_audio_status
    )
{
    return rtk_dprx_get_audio_status(GET_RTK_DPRX_PORT(p_adp), p_audio_status);
}


/*------------------------------------------------
 * Func : _ops_dprx_get_hdcp_status
 *
 * Desc : get dprx port hdcp status
 *
 * Para : p_adp  : handle of DPRX adapter
 *        p_hdcp_status : pointer of hdcp status
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_get_hdcp_status(
    dprx_adp_t*         p_adp,
    DPRX_HDCP_STATUS_T* p_hdcp_status
    )
{
    return rtk_dprx_get_hdcp_status(GET_RTK_DPRX_PORT(p_adp), p_hdcp_status);
}


//===============================================================================
// DPCD related
//===============================================================================

/*------------------------------------------------
 * Func : _ops_set_dpcd
 *
 * Desc : set DPCD of the DPRX adapter
 *
 * Para : p_adp   : handle of DPRX adapter
 *        address : dpcd address
 *        p_dpcd  : dpcd data
 *        len     : size of dpcd data
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
static int _ops_dprx_set_dpcd(
    dprx_adp_t*     p_adp,
    unsigned long   address,
    unsigned char*  p_dpcd,
    unsigned short  len
    )
{
    return rtk_dprx_set_dpcd(GET_RTK_DPRX_PORT(p_adp), address, p_dpcd, len);
}

/*------------------------------------------------
 * Func : _ops_dprx_get_dpcd
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
static int _ops_dprx_get_dpcd(
    dprx_adp_t*     p_adp,
    unsigned long   address,
    unsigned char*  p_dpcd,
    unsigned short  len
    )
{
    return rtk_dprx_get_dpcd(GET_RTK_DPRX_PORT(p_adp), address, p_dpcd, len);
}

//----------------------------------------------------------------------------------------
// Power Management API
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : _ops_dprx_suspend
 *
 * Desc : suspend a dprx adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int _ops_dprx_suspend(
    dprx_adp_t*         p_adp,
    unsigned int        mode
    )
{
    return rtk_dprx_suspend(GET_RTK_DPRX_PORT(p_adp), mode);
}


/*------------------------------------------------
 * Func : _ops_dprx_resume
 *
 * Desc : resume a dprx adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int _ops_dprx_resume(
    dprx_adp_t*         p_adp
    )
{
    return rtk_dprx_resume(GET_RTK_DPRX_PORT(p_adp));
}


/*------------------------------------------------
 * Func : _ops_event_handler
 *
 * Desc : handle event
 *
 * Para : p_adp : handle of DPRX adapter
 *        event_type : external event
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int _ops_event_handler(
    dprx_adp_t*     p_adp,
    unsigned int    event_type
    )
{
    switch(event_type)
    {
    case DPRX_ADP_EVENT_ONLINE_MEASURE_FAIL:
        return rtk_dprx_handle_online_measure_error(GET_RTK_DPRX_PORT(p_adp));
        break;

    default:
        return 0;
    }
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
 *        p_ext_ctrl : ext control command
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int _ops_set_ext_ctrl(
    dprx_adp_t*     p_adp,
    ADP_EXCTL_ID    ctrl,
    unsigned int    val
    )
{
    return rtk_dprx_set_ext_ctrl(GET_RTK_DPRX_PORT(p_adp), ctrl, val);
}

/*------------------------------------------------
 * Func : dprx_adp_get_ext_ctrl
 *
 * Desc : get ext control of a DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_ext_ctrl : ext control command
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
int _ops_get_ext_ctrl(
    dprx_adp_t*     p_adp,
    ADP_EXCTL_ID    ctrl
    )
{
    return rtk_dprx_get_ext_ctrl(GET_RTK_DPRX_PORT(p_adp), ctrl);
}

//===============================================================================
// Attribute Interface
//===============================================================================

/*------------------------------------------------
 * Func : _ops_attr_show
 *
 * Desc : show adapter attr
 *
 * Para : p_adp : handle of DPRX adapter
 *        attr  : attribute name
 *        p_buf : data buffer
 *        cnt   : buffer size
 *
 * Retn : number if bytes output
 *-----------------------------------------------*/
int _ops_attr_show(
    dprx_adp_t*     p_adp,
    ADP_ATTR        attr,
    unsigned char*  p_buf,
    unsigned int    cnt)
{
    return rtk_dprx_attr_show(GET_RTK_DPRX_PORT(p_adp), attr, p_buf, cnt);
}

/*------------------------------------------------
 * Func : _ops_attr_store
 *
 * Desc : store adapter attr
 *
 * Para : p_adp : handle of DPRX adapter
 *        attr  : attribute name
 *        p_buf : data buffer
 *        cnt   : buffer size
 *
 * Retn : number if bytes handled
 *-----------------------------------------------*/
int _ops_attr_store(
    dprx_adp_t*     p_adp,
    ADP_ATTR        attr,
    unsigned char*  p_buf,
    unsigned int    cnt)
{
    return rtk_dprx_attr_store(GET_RTK_DPRX_PORT(p_adp), attr, p_buf, cnt);
}

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------

static dprx_adp_ops_t g_rtk_dprx_ops =
{
    .init       = _ops_dprx_init,
    .uninit     = _ops_dprx_uninit,
    .open       = _ops_dprx_open,
    .close      = _ops_dprx_close,
    .connect    = _ops_dprx_connect,
    .disconnect = _ops_dprx_disconnect,

    // VFE
    .get_capability = _ops_dprx_get_capability,
    .get_link_status = _ops_dprx_get_link_status,
    .get_stream_type = _ops_dprx_get_stream_type,

    // HPD/Detect
    .get_connect_status = _ops_dprx_get_connect_status,
    .get_aux_status = _ops_dprx_get_aux_status,
    .set_hpd    = _ops_dprx_set_hpd,
    .set_hpd_toggle = _ops_dprx_set_hpd_toggle,
    .get_hpd    = _ops_dprx_get_hpd,

    // EDID
    .set_edid   = _ops_dprx_set_edid,
    .get_edid   = _ops_dprx_get_edid,

    // DPCD
    .set_dpcd   = _ops_dprx_set_dpcd,
    .get_dpcd   = _ops_dprx_get_dpcd,

    // SDP
    .get_vbid_data = _ops_dprx_get_vbid_data,
    .get_msa_data = _ops_dprx_get_msa_data,
    .get_sdp_data = _ops_dprx_get_sdp_data,

    // Video
    .detect_task = _ops_dprx_source_check,
    .get_avi_info = _ops_dprx_get_avi_info,
    .get_pixel_encoding_info = _ops_dprx_get_pixel_encoding_info,
    .get_drm_info = _ops_dprx_get_drm_info,
    .get_video_timing = _ops_dprx_get_video_timing,

    // Audio
    .get_audio_status = _ops_dprx_get_audio_status,

    // HDCP
    .get_hdcp_status = _ops_dprx_get_hdcp_status,

    // Power
    .suspend = _ops_dprx_suspend,
    .resume = _ops_dprx_resume,

    // extension control
    .get_ext_ctrl = _ops_get_ext_ctrl,
    .set_ext_ctrl = _ops_set_ext_ctrl,

    // Event handler
    .event_handler = _ops_event_handler,

    // Attribute
    .attr_show  = _ops_attr_show,
    .attr_store = _ops_attr_store,
};

#ifdef DPRX_GET_DEFAULT_EDID_FROM_DDC_SRAM

/*------------------------------------------------
 * Func : _reload_edid_from_ddc_sram
 *
 * Desc : load default edid from ddc sram
 *
 * Para : ddc_ch   : ddc id
 *        p_edid   : edid output
 *        edid_len : edid len
 *
 * Retn : N/A
 *-----------------------------------------------*/
int _reload_edid_from_ddc_sram(
    unsigned char   ddc_ch,
    unsigned char*  p_edid,
    unsigned short  edid_len
    )
{
    edid_dev_t* p_ddc = create_rtk_ddc_edid_device_ex(ddc_ch, RTK_DDC_EDID_QS_INIT);  // in QS mode
#ifdef DEBUG_DEFAULT_EDID
    unsigned short i;
#endif

    DPRX_ADP_INFO("_reload_edid_from_ddc_sram, ddc_ch=%d!!!!\n", ddc_ch);

    if (p_ddc==NULL)
    {
        DPRX_ADP_WARN("_reload_edid_from_ddc_sram failed, open ddc_ch%d failed!!!!\n", ddc_ch);
        return -1;
    }

    // read edid
    dprx_edid_get_edid(p_ddc, p_edid, edid_len);  // current port, read edid to sram

    if (memcmp(p_edid, g_dprx_default_edid, 8)!=0)
    {
        DPRX_ADP_WARN("_reload_edid_from_ddc_sram failed, invalid edid!!!!\n");
        return -1;
    }

#ifdef DEBUG_DEFAULT_EDID
    for (i=0; i< edid_len; i+=16)
    {
        DPRX_ADP_INFO( "%03x | %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
            i,
            p_edid[0],  p_edid[1],  p_edid[2],  p_edid[3],
            p_edid[4],  p_edid[5],  p_edid[6],  p_edid[7],
            p_edid[8],  p_edid[9],  p_edid[10], p_edid[11],
            p_edid[12], p_edid[13], p_edid[14], p_edid[15]);
        p_edid+=16;
    }
#endif

    destroy_dprx_edid_device(p_ddc);
    return 0;
}

#endif

/*------------------------------------------------
 * Func : create_rtk_dprx_adapter
 *
 * Desc : create rtk dprx adapter
 *
 * Para : p_cfg  : dprx config
 *
 * Retn : handle of dprx dev
 *-----------------------------------------------*/
dprx_dev_t* create_rtk_dprx_adapter(
    DP_PORT_CFG*    p_cfg
    )
{
    dprx_dev_t* p_dev = NULL;
    RTK_DPRX_ADP* p_dprx = NULL;
#ifdef DPRX_FW_CAPABILITY
    DPRX_FW_CAPABILITY  fw_cap;
#endif

    if (p_cfg==NULL)
    {
        DPRX_ADP_WARN("create_rtk_dprx_adapter failed, !!!!, p_cfg should not be NULL\n");
        return NULL;
    }

#ifdef CHECK_DPRX_OTP_SETTING
    if(rtk_get_set_type() == SET_TYPE_TV)
    {
        DPRX_ADP_WARN("create_rtk_dprx_adapter failed, !!!!, SET_TYPE is TV, p_cfg should not be NULL\n");
        return NULL;
    }

    if(rtk_otp_field_read_int_by_name("chip_type_dp_rx_disable") == 1)
    {
        DPRX_ADP_WARN("create_rtk_dprx_adapter failed, !!!!, chip_type_dp_rx_disable, p_cfg should not be NULL\n");
        return NULL;
    }
#endif

    // create sink capability
    p_dprx = (RTK_DPRX_ADP*)  dprx_osal_malloc(sizeof(RTK_DPRX_ADP));

    if (p_dprx==NULL)
    {
        DPRX_ADP_WARN("create_rtk_dprx_adapter failed, !!!! alloc dprx handle failed\n");
        return NULL;
    }

    memset(p_dprx, 0, sizeof(RTK_DPRX_ADP));

    //----  store default edid----
    memcpy(p_dprx->sink_edid, g_dprx_default_edid, sizeof(g_dprx_default_edid));

    //----  store dprx type and catble type----
    p_dprx->type = p_cfg->type;

#ifdef CONFIG_ENABLE_TYPEC_DPRX
    p_dprx->cable_type = CABLE_CFG_DP;
#endif

    //----  store sink capability ----
    DPRX_ADP_INFO("!!!!!!create_rtk_dprx_adapter type=%d, p_cfg->config.dp_mdp.phy.max_lane_count=%d, qs_ctrl=%d!!!!\n",
        p_dprx->type, p_cfg->config.dp_mdp.phy.max_lane_count, p_cfg->qs_ctrl);

#ifdef CONFIG_DPRX_VFE_ENABLE_QUICK_SHOW
    if ((p_cfg->qs_ctrl & DP_QS_INIT_READY))
    {
        p_dprx->qs_init_ready = 1;

  #ifdef DPRX_GET_DEFAULT_EDID_FROM_DDC_SRAM
        _reload_edid_from_ddc_sram(p_cfg->config.typec_dp.edid.ddc_i2c_idx, p_dprx->sink_edid, sizeof(p_dprx->sink_edid));
        p_dprx->qs_edid_ready = 1;     // set up qs edid ready flag
  #endif

        if ((p_cfg->qs_ctrl & DP_QS_CONNECT_READY))
        {
            p_dprx->qs_connect_ready = 1;

  #ifdef DPRX_GET_DEFAULT_EDID_FROM_DDC_SRAM
            // if DP port is connected, and its DDC channel is not AUX DDC
            // set qs_edid_reload_request flag to reload its edid to DPRX AUX DDC
            // This flag should be cleared when EDID reload completed
            if (p_cfg->config.typec_dp.edid.ddc_i2c_idx != DPRX_AUX_DDC)
                p_dprx->qs_edid_reload_request = 1;
  #endif
        }
    }
#endif

    p_dprx->sink_cap.max_lane_count = p_cfg->config.dp_mdp.phy.max_lane_count;

    // load default config
#ifdef DPRX_FW_CAPABILITY
    dprx_drv_get_fw_capability(&fw_cap);
    p_dprx->sink_cap.max_link_rate  = fw_cap.max_link_rate;
    p_dprx->sink_cap.audio_support  = 1;
    p_dprx->sink_cap.fec_support    = (fw_cap.capability & DPRX_FW_CAP_FEC_SUPPORT) ? 1 : 0;
    p_dprx->sink_cap.dsc_support    = (p_dprx->sink_cap.fec_support && (fw_cap.capability & DPRX_FW_CAP_DSC_SUPPORT)) ? 1 : 0;
#else
    p_dprx->sink_cap.max_link_rate  = _DP_LINK_HBR3;
    p_dprx->sink_cap.audio_support  = 1;
    p_dprx->sink_cap.fec_support    = 0;
    p_dprx->sink_cap.dsc_support    = 0;
#endif

#ifdef CONFIG_DPRX_EANBLE_FREESYNC
    p_dprx->sink_cap.freesync_support = 1;
    p_dprx->sink_cap.adaptive_sync_support = 1;
#endif

    p_dprx->sink_cap.check_dec_err_irq_support = 1;
    memcpy(&p_dprx->sink_cap.sink_device_information, sink_specific_field, sizeof(sink_specific_field));

    //DPRX_ADP_INFO("!!!!!!create_rtk_dprx_adapter p_sink_cap(%p)->max_lane_count=%d!!!!\n",
        //p_dprx, p_dprx->sink_cap.max_lane_count);

#ifdef CONFIG_ENABLE_TYPEC_DPRX
    //----  port config capability ----
    switch (p_cfg->type)
    {
    case DP_TYPE_DP:
    case DP_TYPE_mDP:
        _update_adp_port_config(&p_dprx->port_cfg, &p_cfg->config.dp_mdp.phy, &p_cfg->config.dp_mdp.aux);
        break;
    case DP_TYPE_USB_TYPE_C:
        _update_adp_port_config(&p_dprx->port_cfg, &p_cfg->config.typec_dp.phy, &p_cfg->config.typec_dp.aux);
        break;
    default:
        _update_adp_port_config(&p_dprx->port_cfg, &p_cfg->config.dp_mdp.phy, &p_cfg->config.dp_mdp.aux);
        break;
    }
#endif

    //----  allocate HPD ----
#ifdef CONFIG_DPRX_DRV_ENABLE_VIRTUAL_PLATFORM
    p_dprx->p_hpd = create_dprx_virt_plat_hpd_device(0);
#else
    switch (p_dprx->type)
    {
    case DP_TYPE_DP:
    case DP_TYPE_mDP:
        if (p_dprx->qs_init_ready)
            p_cfg->config.dp_mdp.hpd_det.flags = DP_HPD_QS_INIT;
        p_dprx->p_hpd = create_rtk_tv_dprx_hpd_device_ex(&p_cfg->config.dp_mdp.hpd_det);
        break;
    case DP_TYPE_USB_TYPE_C:
#ifdef CONFIG_ENABLE_TYPEC_DPRX
        p_dprx->p_hpd = create_rtk_tv_typec_hpd_device(p_cfg->config.typec_dp.pd_port_idx);
        break;
#endif
    default:
        p_dprx->p_hpd = alloc_dprx_hpd_device();
        break;
    }
#endif

    //---- allocate device ----
    p_dev = alloc_dprx_device();

    if (p_dev)
    {
        p_dev->adp.name = "RTK_DPRX";
        p_dev->adp.p_private = (void*)p_dprx;
        memcpy(&p_dev->ops, &g_rtk_dprx_ops, sizeof(dprx_adp_ops_t));
    }
    else
    {
        DPRX_ADP_WARN("create_rtk_dprx_adapter failed, !!!! alloc dprx handle failed\n");
        dprx_osal_mfree(p_dprx);
        return NULL;
    }

    return p_dev;
}

