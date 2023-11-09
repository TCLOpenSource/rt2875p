#ifndef __DPRX_VFE_API_H__
#define __DPRX_VFE_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "dprx_types.h"


/*------------------------------------------------
 * Func : vfe_dprx_drv_init
 *
 * Desc : init DPRX driver. allocate all
 *        resources will be created.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_init(void);


/*------------------------------------------------
 * Func : vfe_dprx_drv_uninit
 *
 * Desc : uninit DPRX driver. all resources
 *        will be released.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_uninit(void);


/*------------------------------------------------
 * Func : vfe_dprx_drv_open
 *
 * Desc : open DPRX driver. should be called before
 *        any operations
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_open(void);


/*------------------------------------------------
 * Func : vfe_dprx_drv_close
 *
 * Desc : close DPRX driver.
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_close(void);


/*------------------------------------------------
 * Func : vfe_dprx_drv_connect
 *
 * Desc : close DPRX driver.
 *
 * Para : ch : DP channel 0~3
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_connect(unsigned char ch);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_connected_channel
 *
 * Desc : get current connected channel
 *
 * Para : p_ch : current channel output
 *          DP channel 0~3
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_connected_channel(unsigned char* p_ch);


/*------------------------------------------------
 * Func : vfe_dprx_drv_disconnect
 *
 * Desc : disconnect a given DPRX frontend.
 *        after disalbed, DPRX frontend will be decode.
 *
 * Para : ch : DP channel 0~3
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_disconnect(unsigned char ch);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_rx_port_cnt
 *
 * Desc : get number of rx port
 *
 * Para : port_cnt : rx port output
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_rx_port_cnt(unsigned char* p_port_cnt);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_rx_port_capability
 *
 * Desc : get capability of rx port
 *
 * Para : ch : DP channel 0~3
 *        p_cap : DP port capability output
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_rx_port_capability(unsigned char ch, DP_PORT_CAPABILITY* p_cap);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_link_status
 *
 * Desc : get link status of rx port
 *
 * Para : ch : DP channel 0~3
 *        p_status : link status output
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_link_status(unsigned char ch, DP_LINK_STATUS_T* p_status);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_stream_type
 *
 * Desc : get stream type of rx port
 *
 * Para : ch : DP channel 0~3
 *        p_type : stream type output
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_stream_type(unsigned char ch, DP_STREAM_TYPE_E* p_type);


//===============================================================================
// HPD / DET related
//===============================================================================


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_connection_state
 *
 * Desc : get cable connect state of a DPRX port
 *
 * Para : ch : DPRX channel
 *        p_connect_state : connect state output
 *           0 : disconnect, others : connect
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_connection_state(unsigned char ch, unsigned char *p_connect_state);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_aux_state
 *
 * Desc : get cable aux state.
 *        for DP, it means PD Aux P/N is at correct level
 *        for TypeC, it mens TypeC enter DP alt mode
 *
 * Para : p_dev : handle of DPRX adapter
 *        p_aux_state :
 *           0 : aux is not ready to use
 *           1 : aux is ready to use
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_aux_state(unsigned char ch, unsigned char *p_aux_state);

/*------------------------------------------------
 * Func : vfe_dprx_drv_enable_hpd
 *
 * Desc : enable HPD of all DPRX port
 *
 * Para : on : 0 : disable hpd, others : enable hpd
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_enable_hpd(unsigned char on);

/*------------------------------------------------
 * Func : vfe_dprx_drv_set_hpd
 *
 * Desc : enable / disable HPD of a DPRX port
 *
 * Para : ch : DPRX channel
 *        on : 0 : disable hpd, others : enable hpd
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_set_hpd(unsigned char ch, unsigned char on);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_hpd
 *
 * Desc : get HPD status of a DPRX port
 *
 * Para : ch : DPRX channel
 *        p_hpd_state : hpd state output
 *           0 : hpd disabled,
 *           others : hpd enabled
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_hpd(unsigned char ch, unsigned char* p_hpd_state);

/*------------------------------------------------
 * Func : vfe_dprx_drv_hpd_toggle
 *
 * Desc : do HPD toggle on a given DPRX port
 *
 * Para : ch : DPRX channel
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_hpd_toggle(unsigned char ch);

/*------------------------------------------------
 * Func : vfe_dprx_drv_hpd_toggle_ex
 *
 * Desc : do HPD toggle on a given DPRX port
 *
 * Para : ch : DPRX channel
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_hpd_toggle_ex(unsigned char ch, unsigned short delay_ms);

/*------------------------------------------------
 * Func : vfe_dprx_drv_set_hpd_low_delay
 *
 * Desc : set hpd low delay
 *
 * Para : delay_ms : hpd low delay ms
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_set_hpd_low_delay(unsigned int delay_ms);


//===============================================================================
// EDID related
//===============================================================================


/*------------------------------------------------
 * Func : vfe_dprx_drv_read_edid
 *
 * Desc : read edid from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_edid : edid data
 *        len : edid len
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_read_edid(unsigned char ch, unsigned char* p_edid, unsigned short len);


/*------------------------------------------------
 * Func : vfe_hdmi_drv_write_edid
 *
 * Desc : write edid to a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_edid : edid data
 *        len : edid len
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_write_edid(unsigned char ch, const unsigned char* p_edid, unsigned short len);


//===============================================================================
// DPCD related
//===============================================================================

/*------------------------------------------------
 * Func : vfe_dprx_drv_read_dpcd
 *
 * Desc : read dpcd from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        address : dpcd address
 *        p_dpcd : dpcd output
 *        len : dpcd len
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_read_dpcd(unsigned char ch, unsigned long address, unsigned char* p_dpcd, unsigned short len);

/*------------------------------------------------
 * Func : vfe_dprx_drv_write_dpcd
 *
 * Desc : write dpcd from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        address : dpcd address
 *        p_dpcd : dpcd to be written
 *        len : dpcd len
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_write_dpcd(unsigned char ch, unsigned long address, unsigned char* p_dpcd, unsigned short len);

//===============================================================================
// Video related
//===============================================================================

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_vbid_info
 *
 * Desc : get vbid information from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_vbid : vbid info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_port_vbid_info(unsigned char ch, DPRX_VBID_INFO_T *p_vbid);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_msa_info
 *
 * Desc : get msa information from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_msa : msa info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_port_msa_info(unsigned char ch, DPRX_MSA_INFO_T *p_msa);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_sdp_data
 *
 * Desc : get sdp data from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        type   : SDP type
 *        p_data : spd data output
 *
 * Retn : 0 : successed, 0< : failed
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_port_sdp_data(unsigned char ch, DP_SDP_TYPE type, DP_SDP_DATA_T* p_data);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_avi_info
 *
 * Desc : get avi information from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_avi_info : avi info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_port_avi_info(unsigned char ch, DPRX_AVI_INFO_T *p_avi_info);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_drm_info
 *
 * Desc : get drm information from a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_drm_info : drm info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_port_drm_info(unsigned char ch, DPRX_DRM_INFO_T *p_drm_info);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_port_timing_info
 *
 * Desc : write edid to a given DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_timing_info : timing info
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_port_timing_info(unsigned char ch, DPRX_TIMING_INFO_T *p_timing_info);


/*------------------------------------------------
 * Func : vfe_dprx_drv_source_check
 *
 * Desc : Check input source
 *
 * Para : ch : DP channel 0~3
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_source_check(unsigned char ch);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_avmute_status
 *
 * Desc : write edid to a given DPRX Port
 *
 * Para : p_avmute : AV mute status
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_avmute_status(unsigned char* p_avmute);



//===============================================================================
// Audio related
//===============================================================================

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_audio_status
 *
 * Desc : get audio status of current DPRX Port
 *
 * Para : p_audio_status : audio status
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_audio_status(DP_AUDIO_STATUS_T *p_audio_status);


//===============================================================================
// HDCP related
//===============================================================================


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_hdcp_capability
 *
 * Desc : get hdcp capability of DPRX VFE
 *
 * Para : p_hdcp_cap : bit mask of HDCP capability
 *          DP_HDCP_CAPBILITY_HDCP13
 *          DP_HDCP_CAPBILITY_HDCP22
 *	        DP_HDCP_CAPBILITY_HDCP23
 *          DP_HDCP_CAPBILITY_HDCP_REPEATER
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_hdcp_capability(unsigned int *p_hdcp_cap);


/*------------------------------------------------
 * Func : vfe_dprx_drv_write_hdcp_key
 *
 * Desc : write hdcp key
 *
 * Para : p_key : hdcp  1.x/2.x key
 *        apply : immediate apply
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_write_hdcp_key(DPRX_HDCP_KEY_T * p_key, unsigned char apply);


/*------------------------------------------------
 * Func : vfe_dprx_drv_get_hdcp_status
 *
 * Desc : get hdcp status of a DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_hdcp_status : hdcp status
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_hdcp_status(unsigned char ch, DPRX_HDCP_STATUS_T *p_hdcp_status);

//===============================================================================
// Extension Control
//===============================================================================
/*------------------------------------------------
 * Func : vfe_dprx_drv_set_ext_ctrl
 *
 * Desc : set ext control of a DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_ext_ctrl : ext_ctrl
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_set_ext_ctrl(unsigned char ch, DP_EXT_CTRL* p_ext_ctrl);
/*------------------------------------------------
 * Func : vfe_dprx_drv_get_ext_ctrl
 *
 * Desc : get ext control of a DPRX Port
 *
 * Para : ch : DPRX channel
 *        p_ext_ctrl : ext_ctrl
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_get_ext_ctrl(unsigned char ch, DP_EXT_CTRL* p_ext_ctrl);

//===============================================================================
// Power Management related
//===============================================================================

/*------------------------------------------------
 * Func : vfe_dprx_drv_suspend
 *
 * Desc : suspend
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_suspend(void);


/*------------------------------------------------
 * Func : vfe_dprx_drv_resume
 *
 * Desc : suspend
 *
 * Para : N/A
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_resume(void);

/*------------------------------------------------
 * Func : vfe_dprx_drv_set_wakeup_mode
 *
 * Desc : set dprx wakeup mode
 *
 * Para : mode :
 *  DP_WAKEUP_MODE_NO_WAKEUP
 *  DP_WAKEUP_MODE_BY_AUX
 *  DP_WAKEUP_MODE_BY_SIGNAL
 *  DP_WAKEUP_MODE_BY_AUX_OR_SIGNAL
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_ARG_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_set_wakeup_mode(DP_WAKEUP_MODE_E mode);

/*------------------------------------------------
 * Func : vfe_dprx_drv_get_wakeup_mode
 *
 * Desc : get dprx wakeup mode
 *
 * Para : p_mode : wakeup mode output
 *  DP_WAKEUP_MODE_NO_WAKEUP
 *  DP_WAKEUP_MODE_BY_AUX
 *  DP_WAKEUP_MODE_BY_SIGNAL
 *  DP_WAKEUP_MODE_BY_AUX_OR_SIGNAL
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_ARG_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
int vfe_dprx_drv_get_wakeup_mode(DP_WAKEUP_MODE_E* p_mode);

#ifdef CONFIG_DPRX_VFE_ENABLE_ONLINE_MEASURE_ERROR

/*------------------------------------------------
 * Func : vfe_dprx_drv_handle_online_measure_error
 *
 * Desc : handle online measure error
 *
 * Para : flags : ertra control flag
 *
 * Retn : DPRX_DRV_NO_ERR / DPRX_DRV_STATE_ERR
 *        DPRX_DRV_HW_ERR / DPRX_DRV_ARG_ERR
 *-----------------------------------------------*/
extern int vfe_dprx_drv_handle_online_measure_error(unsigned char flags);

#else

#define vfe_dprx_drv_handle_online_measure_error(x)

#endif

#ifdef __cplusplus
}
#endif


#endif // __DPRX_VFE_API_H__
