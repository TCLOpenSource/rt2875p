/*=============================================================
 * File:    dprx_adapter-rtk_dprx_priv.h
 *
 * Desc:    DPRX Adapter
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
#ifndef __DPRX_ADAPTER_RTK_DPRX_PRIV_H__
#define __DPRX_ADAPTER_RTK_DPRX_PRIV_H__


#include <ScalerFunctionInclude.h>
#include <ScalerDpRxInclude.h>
#include <ScalerDpAuxRxInclude.h>
#include <ScalerDpMacRxInclude.h>   // for fec ???
#include <ScalerDpHdcpRxInclude.h>
#include <ScalerDpPhyRxInclude.h>

#include <dprx_edid-rtkddc.h>
#include <dprx_hpd-rtktv.h>
#include <dprx_hpd-typec.h>

#define DPRX_AUX_DDC                3
#define DPRX_EDID_SIZE              512

extern UINT32 g_dprx_sdp_debug_ctrl;
#define DEBUG_DOLBY_VIOSN_SDP       (1<<0)
#define DEBUG_AMD_FREESYNC_SPD_SDP  (1<<1)
#define DEBUG_VSC_EXT_VESA_SDP      (1<<2)

typedef struct
{
    UINT32       pkt_count;
    UINT32       time_stamp;   // time stamp
    UINT16       expire_time;  // expire time (in ms)
    UINT16       debug_en : 1;
    UINT16       reserved : 15;     // reserved control flag
    DPRX_SDP_PKT pkt;
}DPRX_SDP_PKT_BUFFER;

typedef struct
{
    DPRX_SDP_PKT_BUFFER vsc;
    DPRX_SDP_PKT_BUFFER spd;
    DPRX_SDP_PKT_BUFFER hdr;
    DPRX_SDP_PKT_BUFFER dvs;  // dolby vision vsif
    DPRX_SDP_PKT_BUFFER vsi;  // non DVS VSI
    DPRX_SDP_PKT_BUFFER adp_sync;  // adaptive sync
    DPRX_SDP_PKT_BUFFER vsc_ext_vesa;
    
    // for extra parser info
    UINT8 freesync_support : 1;
    UINT8 freesync_enable : 1;
    UINT8 freesync_activated : 1;
}DPRX_SDP_HANDLER;

extern DPRX_SDP_HANDLER g_dprx_sdp_handler;

typedef struct
{
    DP_TYPE type;                       // dp port type
    DPRX_SINK_CAPABILITY sink_cap;      // info for low level driver
#ifdef CONFIG_ENABLE_TYPEC_DPRX
    DPRX_PORT_CONFIG port_cfg;          // port config
    DPRX_CABLE_CFG cable_type;          // cable type
#endif
    UINT8 sink_edid[DPRX_EDID_SIZE];    // to store edid assigned from ap
    UINT8 suspend:1;
    UINT8 resume_connect:1;
    UINT8 qs_init_ready:1;
    UINT8 qs_connect_ready:1;
    UINT8 qs_open_ready:1;
    UINT8 qs_edid_ready:1;              // qs edid is ready (FW got real edid)
    UINT8 qs_edid_reload_request:1;     // need to reload edid after qs init
    UINT8 connected:1;
    UINT8 hpd_enable:1;                 // store hpd enable info
    UINT8 pre_cable_state:1;
    UINT8 wake_up_by_aux:1;
    UINT8 lpm_enable:1;

    UINT8 override_eotf_en:1;           // for RTKREQ-577
    UINT8 override_eotf:3;              // for RTKREQ-577

    UINT8 manul_eq_en:1;                // for RTKREQ-577 : when Enabled, DP PHY should use manual EQ setting
    UINT8 hdcp_1x_disable_en:1;         // for RTKREQ-577 : when Enabled, HDCP1x should be disabled
    UINT8 hdcp_2x_disable_en:1;         // for RTKREQ-577 : when Enabled, HDCP2x should be disabled
    UINT8 hdcp_2x_force_reauth:1;       // for RTKREQ-577 : when Enabled, need to reauth HDCP2 (auto clear after reauth)

#ifdef CONFIG_DPRX_FORCE_CHECK_MODE_FAIL_CABLE_OFF_ON
    UINT8 output_enable:1;
    UINT8 fake_cable_off;
#endif
    hpd_dev_t* p_hpd;   // for hpd / detect control

#ifdef CONFIG_DPRX_STATUS_MONITOR
    UINT32 status_monitor_time;
#endif
#ifdef CHECK_SOURCE_IN_GET_CONNECT_STATE
    UINT32 source_check_time;
#endif

    UINT8 link_round;
    UINT8 link_state;
    UINT8 link_sub_state;

}RTK_DPRX_ADP;

//================ MACRO ==================
#ifdef UNIT_TEST
#define static    // remove static declariation of local variable
extern RTK_DPRX_ADP* g_p_current_dprx_port;
extern edid_dev_t*   g_p_dprx_edid;
extern unsigned char g_dprx_edid_reference_cnt;
#endif

//================ API ==================

extern int  rtk_dprx_ddc_init(void);
extern int  rtk_dprx_ddc_uninit(void);
extern int  rtk_dprx_port_swap(RTK_DPRX_ADP* p_dprx);

extern int  rtk_dprx_init(RTK_DPRX_ADP* p_dprx);
extern int  rtk_dprx_uninit(RTK_DPRX_ADP* p_dprx);
extern int  rtk_dprx_open(RTK_DPRX_ADP* p_dprx);
extern int  rtk_dprx_close(RTK_DPRX_ADP* p_dprx);
extern int  rtk_dprx_connect(RTK_DPRX_ADP* p_dprx);
extern int  rtk_dprx_disconnect(RTK_DPRX_ADP* p_dprx);

extern int  rtk_dprx_source_check(RTK_DPRX_ADP* p_dprx);
extern int  rtk_dprx_get_capability(RTK_DPRX_ADP* p_dprx, DP_PORT_CAPABILITY* p_cap);
extern int  rtk_dprx_get_link_status(RTK_DPRX_ADP* p_dprx, DP_LINK_STATUS_T* p_status);

extern int  rtk_dprx_set_hpd(RTK_DPRX_ADP* p_dprx, UINT8 enable);
extern int  rtk_dprx_set_hpd_toggle(RTK_DPRX_ADP* p_dprx, UINT16 delay_ms);
extern int  rtk_dprx_set_irq_hpd(RTK_DPRX_ADP* p_dprx);
extern int  rtk_dprx_get_hpd(RTK_DPRX_ADP* p_dprx);
extern int  rtk_dprx_get_connect_status(RTK_DPRX_ADP* p_dprx);
extern int  rtk_dprx_get_aux_status(RTK_DPRX_ADP* p_dprx);

extern int  rtk_dprx_set_edid(RTK_DPRX_ADP* p_dprx, unsigned char* p_edid, unsigned short edid_len);
extern int  rtk_dprx_get_edid(RTK_DPRX_ADP* p_dprx, unsigned char* p_edid, unsigned short edid_len);

extern int  rtk_dprx_get_dpcd(RTK_DPRX_ADP* p_dprx, unsigned long address, unsigned char* p_dpcd, unsigned short len);
extern int  rtk_dprx_set_dpcd(RTK_DPRX_ADP* p_dprx, unsigned long address, unsigned char* p_dpcd, unsigned short len);

extern int  rtk_dprx_get_vbid_data(RTK_DPRX_ADP* p_dprx, DPRX_VBID_INFO_T* p_vbid);
extern int  rtk_dprx_get_msa_data(RTK_DPRX_ADP* p_dprx, DPRX_MSA_INFO_T* p_msa);
extern int  rtk_dprx_get_sdp_data(RTK_DPRX_ADP* p_dprx, DP_SDP_TYPE type, DP_SDP_DATA_T* p_data);

extern int  rtk_dprx_get_stream_type(RTK_DPRX_ADP* p_dprx, DP_STREAM_TYPE_E* p_type);
extern int  rtk_dprx_get_video_timing(RTK_DPRX_ADP* p_dprx, DPRX_TIMING_INFO_T* p_timing);
extern int  rtk_dprx_get_pixel_encoding_info(RTK_DPRX_ADP* p_dprx,DP_PIXEL_ENCODING_FORMAT_T* p_pixel);
extern int  rtk_dprx_get_drm_info(RTK_DPRX_ADP* p_dprx, DPRX_DRM_INFO_T* p_drm);

extern int  rtk_dprx_get_audio_status(RTK_DPRX_ADP* p_dprx, DP_AUDIO_STATUS_T* p_audio_status);
extern int  rtk_dprx_get_hdcp_status(RTK_DPRX_ADP* p_dprx, DPRX_HDCP_STATUS_T* p_hdcp_status);

extern int  rtk_dprx_suspend(RTK_DPRX_ADP* p_dprx, UINT32 mode);
extern int  rtk_dprx_resume(RTK_DPRX_ADP* p_dprx);

extern int  rtk_dprx_handle_online_measure_error(RTK_DPRX_ADP* p_dprx);
extern void rtk_dprx_handle_infoframe_sdp_update_event(RTK_DPRX_ADP* p_dprx, SDP_TYPE_E sdp_type);

#ifdef ENABLE_DPRX_LT_EVENT_MONITOR
extern void rtk_dprx_handle_lt_state_update_event(RTK_DPRX_ADP* p_dprx, UINT8 state);
#else
#define rtk_dprx_handle_lt_state_update_event(p_dprx, state)
#endif

extern int  rtk_dprx_set_ext_ctrl(RTK_DPRX_ADP* p_dprx, ADP_EXCTL_ID ctrl, unsigned int val);
extern int  rtk_dprx_get_ext_ctrl(RTK_DPRX_ADP* p_dprx, ADP_EXCTL_ID ctrl);

#ifdef CONFIG_RTK_DPRX_SYSFS
extern int  rtk_dprx_attr_show(RTK_DPRX_ADP* p_dprx, ADP_ATTR attr, UINT8* p_buf, UINT32 cnt);
extern int  rtk_dprx_attr_store(RTK_DPRX_ADP* p_dprx, ADP_ATTR attr, UINT8* p_buf, UINT32 cnt);
#else
#define rtk_dprx_attr_show(p_dprx, attr, p_buf, cnt)         (0)
#define rtk_dprx_attr_store(p_dprx, attr, p_buf, cnt)        (cnt)
#endif

#endif // __DPRX_ADAPTER_RTK_DPRX_PRIV_H__
