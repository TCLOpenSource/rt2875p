/*=============================================================
 * File:    dprx_adapter.h
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
#ifndef __DPRX_ADAPTER_H__
#define __DPRX_ADAPTER_H__

#include <dprx_osal.h>
#include <dprx_types.h>
#include <dprx_platform.h>

typedef struct
{
    const char* name;
    void* p_private;    // dprx private data
}dprx_adp_t;

typedef enum
{
    ADP_ATTR_STATUS = 0,
    ADP_ATTR_DPCD,
    ADP_ATTR_SDP,
    ADP_ATTR_DEBUG_CTRL,
    ADP_ATTR_MAX,
}ADP_ATTR;

typedef enum
{
    // EOTF
    ADP_EXCTL_OVERRIDE_EOTF_EN = 0,
    ADP_EXCTL_OVERRIDE_EOTF,

    // PHY
    ADP_EXCTL_MANUAL_EQ_EN,

    // HDCP
    ADP_EXCTL_DISABLE_HDCP14,
    ADP_EXCTL_DISABLE_HDCP22,
    ADP_EXCTL_FRORCE_REAUTH_HDCP22,
    ADP_EXCTL_MAX,

    //WatchDag
    ADP_EXCTL_WATCH_DOG_ENABLE,
}ADP_EXCTL_ID;

typedef struct
{
    int (*init)(dprx_adp_t* p_adp);
    int (*uninit)(dprx_adp_t* p_adp);   // this function only be called automtaically when a adapter to be released

    // Basic control
    int (*open)(dprx_adp_t* p_adp);     // device should become operational
    int (*close)(dprx_adp_t* p_adp);    // device should stop from operation
    int (*connect)(dprx_adp_t* p_adp);      // device should start detect audio/video, and output audio/video
    int (*disconnect)(dprx_adp_t* p_adp);   // device should stop output audio/video

    // VFE
    int (*get_capability)(dprx_adp_t* p_adp, DP_PORT_CAPABILITY* p_cap);
    int (*get_link_status)(dprx_adp_t* p_adp, DP_LINK_STATUS_T *p_status);
    int (*get_stream_type)(dprx_adp_t* p_adp, DP_STREAM_TYPE_E *p_type);

    // HPD/Detect
    int (*get_connect_status)(dprx_adp_t* p_adp);           // get connect status (0: cable disconnected, 1 : cable connected)
    int (*get_aux_status)(dprx_adp_t* p_adp);               // get aux status (0: aux disconnected, 1 : aux connected)
    int (*set_hpd)(dprx_adp_t* p_adp, unsigned char on);    // set HPD (0: HPD low, 1 : HPD High)
    int (*set_hpd_toggle)(dprx_adp_t* p_adp, unsigned short ms_delay);    // set HPD Toggle(ms_delay)
    int (*get_hpd)(dprx_adp_t* p_adp);                      // get HPD (0: HPD low, 1 : HPD High)

    // EDID
    int (*set_edid)(dprx_adp_t* p_adp, unsigned char* p_edid, unsigned short edid_len);
    int (*get_edid)(dprx_adp_t* p_adp, unsigned char* p_edid, unsigned short edid_len);

    // DPCD
    int (*set_dpcd)(dprx_adp_t* p_adp, unsigned long address, unsigned char* p_dpcd, unsigned short len);
    int (*get_dpcd)(dprx_adp_t* p_adp, unsigned long address, unsigned char* p_dpcd, unsigned short len);

    // SDP
    int (*get_vbid_data)(dprx_adp_t* p_adp, DPRX_VBID_INFO_T* p_data);
    int (*get_msa_data)(dprx_adp_t* p_adp, DPRX_MSA_INFO_T* p_data);
    int (*get_sdp_data)(dprx_adp_t* p_adp, DP_SDP_TYPE type, DP_SDP_DATA_T* p_data);

    // Video
    int (*detect_task)(dprx_adp_t* p_adp);   // check input source
    int (*get_avi_info)(dprx_adp_t* p_adp, DPRX_AVI_INFO_T* p_avi);
    int (*get_pixel_encoding_info)(dprx_adp_t* p_adp, DP_PIXEL_ENCODING_FORMAT_T* p_pixel);
    int (*get_drm_info)(dprx_adp_t* p_adp, DPRX_DRM_INFO_T* p_drm);
    int (*get_video_timing)(dprx_adp_t* p_adp, DPRX_TIMING_INFO_T* p_timing);
    int (*get_avmute)(dprx_adp_t* p_adp);

    // Audio
    int (*get_audio_status)(dprx_adp_t* p_adp, DP_AUDIO_STATUS_T *p_status);

    // HDCP
    int (*get_hdcp_capability)(dprx_adp_t* p_adp, unsigned int *p_hdcp_cap);
    int (*get_hdcp_status)(dprx_adp_t* p_adp, DPRX_HDCP_STATUS_T *p_hdcp_status);

    // extension control
    int (*set_ext_ctrl)(dprx_adp_t* p_adp, ADP_EXCTL_ID id, unsigned int val);
    int (*get_ext_ctrl)(dprx_adp_t* p_adp, ADP_EXCTL_ID id);

    // power management
    int (*suspend)(dprx_adp_t* p_adp, unsigned int mode);
    int (*resume)(dprx_adp_t* p_adp);

    // event handler
    int (*event_handler)(dprx_adp_t* p_adp, unsigned int event_type);
    #define DPRX_ADP_EVENT_ONLINE_MEASURE_FAIL      (1<<31)

    // debug attr
    int (*attr_show)(dprx_adp_t* p_adp, ADP_ATTR attr, unsigned char* p_buf, unsigned int cnt);
    int (*attr_store)(dprx_adp_t* p_adp, ADP_ATTR attr, unsigned char* p_buf, unsigned int cnt);

}dprx_adp_ops_t;

typedef struct
{
    unsigned char   ch;   // channel idx
    dprx_adp_t      adp;
    dprx_adp_ops_t  ops;
}dprx_dev_t;

//================= API of DPRX Adapter ==============

extern dprx_dev_t* alloc_dprx_device(void);
extern void destroy_dprx_device(dprx_dev_t* dev);

extern int dprx_adp_init(dprx_dev_t* p_dev);
extern int dprx_adp_open(dprx_dev_t* p_dev);
extern int dprx_adp_close(dprx_dev_t* p_dev);
extern int dprx_adp_connect(dprx_dev_t* p_dev);
extern int dprx_adp_disconnect(dprx_dev_t* p_dev);

//===============================================================================
// VFE related
//===============================================================================

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
extern int dprx_adp_get_capability(dprx_dev_t* p_dev, DP_PORT_CAPABILITY* p_cap);


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
extern int dprx_adp_get_link_status(dprx_dev_t* p_dev, DP_LINK_STATUS_T* p_status);


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
extern int dprx_adp_get_stream_type(dprx_dev_t* p_dev, DP_STREAM_TYPE_E* p_type);


//===============================================================================
// HPD / DET related
//===============================================================================

/*------------------------------------------------
 * Func : dprx_adp_get_connect_status
 *
 * Desc : get cable connection status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : disconnect, 1 : connected
 *-----------------------------------------------*/
extern int dprx_adp_get_connect_status(dprx_dev_t* p_dev);

/*------------------------------------------------
 * Func : dprx_adp_get_aux_status
 *
 * Desc : get aux status
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : aux disconnect, 1 : aux connected
 *-----------------------------------------------*/
extern int dprx_adp_get_aux_status(dprx_dev_t* p_dev);

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
extern int dprx_adp_set_hpd(dprx_dev_t* p_dev, unsigned char on);

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
extern int dprx_adp_set_hpd_toggle(dprx_dev_t* p_dev, unsigned short delay_ms);

/*------------------------------------------------
 * Func : dprx_adp_get_hpd
 *
 * Desc : get HPD status of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : HPD low, 1 : HPD high
 *-----------------------------------------------*/
extern int dprx_adp_get_hpd(dprx_dev_t* p_dev);


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
extern int dprx_adp_set_edid(dprx_dev_t* p_dev, unsigned char* p_edid, unsigned short edid_len);

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
extern int dprx_adp_get_edid(dprx_dev_t* p_dev, unsigned char* p_edid, unsigned short edid_len);


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
extern int dprx_adp_set_dpcd(dprx_dev_t* p_dev, unsigned long address, unsigned char* p_dpcd, unsigned short len);

/*------------------------------------------------
 * Func : dprx_adp_get_dpcd
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
extern int dprx_adp_get_dpcd(dprx_dev_t* p_dev, unsigned long address, unsigned char* p_dpcd, unsigned short len);

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
extern int dprx_adp_get_vbid_info(dprx_dev_t* p_dev, DPRX_VBID_INFO_T* p_vbid);

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
extern int dprx_adp_get_msa_info(dprx_dev_t* p_dev, DPRX_MSA_INFO_T* p_msa);

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
extern int dprx_adp_get_sdp_data(dprx_dev_t* p_dev, DP_SDP_TYPE type, DP_SDP_DATA_T* p_data);


//===============================================================================
// Video related
//===============================================================================

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
extern int dprx_adp_get_avi_info(dprx_dev_t* p_dev, DPRX_AVI_INFO_T *p_avi_info);

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
extern int dprx_adp_get_pixel_encoding_info(dprx_dev_t* p_dev, DP_PIXEL_ENCODING_FORMAT_T *p_pixel_info);

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
extern int dprx_adp_get_drm_info(dprx_dev_t* p_dev, DPRX_DRM_INFO_T *p_drm_info);

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
extern int dprx_adp_get_video_timing(dprx_dev_t* p_dev, DPRX_TIMING_INFO_T *p_timing_info);

/*------------------------------------------------
 * Func : dprx_adp_source_check
 *
 * Desc : Check input source
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
extern int dprx_adp_source_check(dprx_dev_t* p_dev);

/*------------------------------------------------
 * Func : dprx_adp_get_avmute
 *
 * Desc : get AVMUTE status of the DPRX adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 :No Mute, 1 : AV Mute
 *-----------------------------------------------*/
extern int dprx_adp_get_avmute(dprx_dev_t* p_dev);


//===============================================================================
// Audio related
//===============================================================================


/*------------------------------------------------
 * Func : dprx_adp_get_audio_coding_type
 *
 * Desc : get audio coding type of current DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_coding_type : audio coding type
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
extern int dprx_adp_get_audio_status(dprx_dev_t* p_dev, DP_AUDIO_STATUS_T *p_status);


//===============================================================================
// HDCP related
//===============================================================================

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_hdcp_capability
 *
 * Desc : get hdcp capability pf DPRX VFE
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_hdcp_cap : bit mask of HDCP capability
 *          DP_HDCP_CAPBILITY_HDCP13
 *          DP_HDCP_CAPBILITY_HDCP22
 *	        DP_HDCP_CAPBILITY_HDCP23
 *          DP_HDCP_CAPBILITY_HDCP_REPEATER
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
extern int dprx_adp_get_hdcp_capability(dprx_dev_t* p_dev, unsigned int *p_hdcp_cap);


/*------------------------------------------------
 * Func : dprx_adp_get_hdcp_status
 *
 * Desc : get hdcp status of a DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *        p_hdcp_status : hdcp status
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
extern int dprx_adp_get_hdcp_status(dprx_dev_t* p_dev, DPRX_HDCP_STATUS_T *p_hdcp_status);


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
 * Retn : >= 0: successed, 0< : failed
 *-----------------------------------------------*/
extern int dprx_adp_set_ext_ctrl(dprx_dev_t* p_dev, ADP_EXCTL_ID id, unsigned int val);

#define dprx_adp_ext_set_eotf_override_en(p_dev, en)    dprx_adp_set_ext_ctrl(p_dev, ADP_EXCTL_OVERRIDE_EOTF_EN, en)
#define dprx_adp_ext_set_eotf_override(p_dev, eotf)     dprx_adp_set_ext_ctrl(p_dev, ADP_EXCTL_OVERRIDE_EOTF, eotf)
#define dprx_adp_ext_set_manual_eq_en(p_dev, en)        dprx_adp_set_ext_ctrl(p_dev, ADP_EXCTL_MANUAL_EQ_EN, en)
#define dprx_adp_ext_set_hdcp1x_disable_en(p_dev, en)   dprx_adp_set_ext_ctrl(p_dev, ADP_EXCTL_DISABLE_HDCP14, en)
#define dprx_adp_ext_set_hdcp2x_disable_en(p_dev, en)   dprx_adp_set_ext_ctrl(p_dev, ADP_EXCTL_DISABLE_HDCP22, en)
#define dprx_adp_ext_set_hdcp2x_reauth_en(p_dev, en)    dprx_adp_set_ext_ctrl(p_dev, ADP_EXCTL_FRORCE_REAUTH_HDCP22, en)

/*------------------------------------------------
 * Func : dprx_adp_get_ext_ctrl
 *
 * Desc : get ext control of a DPRX Port
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : >= 0 : successed, 0< : failed
 *-----------------------------------------------*/
extern int dprx_adp_get_ext_ctrl(dprx_dev_t* p_dev, ADP_EXCTL_ID id);

#define dprx_adp_ext_get_eotf_override_en(p_dev)    dprx_adp_get_ext_ctrl(p_dev, ADP_EXCTL_OVERRIDE_EOTF_EN)
#define dprx_adp_ext_get_eotf_override(p_dev)       dprx_adp_get_ext_ctrl(p_dev, ADP_EXCTL_OVERRIDE_EOTF)
#define dprx_adp_ext_get_manual_eq_en(p_dev)        dprx_adp_get_ext_ctrl(p_dev, ADP_EXCTL_MANUAL_EQ_EN)
#define dprx_adp_ext_get_hdcp1x_disable_en(p_dev)   dprx_adp_get_ext_ctrl(p_dev, ADP_EXCTL_DISABLE_HDCP14)
#define dprx_adp_ext_get_hdcp2x_disable_en(p_dev)   dprx_adp_get_ext_ctrl(p_dev, ADP_EXCTL_DISABLE_HDCP22)
#define dprx_adp_ext_get_hdcp2x_reauth_en(p_dev)    dprx_adp_get_ext_ctrl(p_dev, ADP_EXCTL_FRORCE_REAUTH_HDCP22)

//===============================================================================
// Power Management related
//===============================================================================

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
extern int dprx_adp_suspend(dprx_dev_t* p_dev, unsigned int mode);
#define WAKEUP_BY_AUX               0x1
#define WAKEUP_BY_SIGNAL            (0x1<<1)

/*------------------------------------------------
 * Func : dprx_adp_resume
 *
 * Desc : resume a dprx adapter
 *
 * Para : p_adp : handle of DPRX adapter
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
extern int dprx_adp_resume(dprx_dev_t* p_dev);


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
extern int dprx_adp_event_handler(dprx_dev_t* p_dev, unsigned int event_type);


//===============================================================================
// Attribute Interface
//===============================================================================

/*------------------------------------------------
 * Func : dprx_adp_attr_show
 *
 * Desc : show adapter attr
 *
 * Para : p_adp : handle of DPRX adapter
 *        attr : attribute name
 *        p_buf : data buffer
 *        cnt : buffer size
 *
 * Retn : number if bytes output
 *-----------------------------------------------*/
extern int dprx_adp_attr_show(dprx_dev_t* p_dev, ADP_ATTR attr, unsigned char* p_buf, unsigned int cnt);

/*------------------------------------------------
 * Func : dprx_adp_attr_store
 *
 * Desc : store adapter attr
 *
 * Para : p_adp : handle of DPRX adapter
 *        attr : attribute name
 *        p_buf : data buffer
 *        cnt : buffer size
 *
 * Retn : number if bytes handled
 *-----------------------------------------------*/
extern int dprx_adp_attr_store(dprx_dev_t* p_dev, ADP_ATTR attr, unsigned char* p_buf, unsigned int cnt);

//======== DPRX Adapter debug message control ========
#define DPRX_ADP_ERR(fmt, args...)         dprx_err("[DP][ADP][ERR] " fmt, ## args)
#define DPRX_ADP_WARN(fmt, args...)        dprx_warn("[DP][ADP][WARN] " fmt, ## args)
#define DPRX_ADP_INFO(fmt, args...)        dprx_info("[DP][ADP][INFO] " fmt, ## args)
#define DPRX_ADP_DBG(fmt, args...)         dprx_dbg("[DP][ADP][DEBUG] " fmt, ## args)

#endif // __DPRX_ADAPTER_H__
