#ifndef __HDMI_DEBUG_H__
#define __HDMI_DEBUG_H__

#ifndef BUILD_QUICK_SHOW
#include "generated/autoconf.h"
#endif

#include <rtd_log/rtd_module_log.h>

#define HDMI_RX_DBG_EN

#ifdef HDMI_RX_DBG_EN
    #define HDMI_EMG(fmt, args...)           rtd_pr_hdmi_emerg(  "[L0]" fmt, ##args)
    #define HDMI_ALT(fmt, args...)           rtd_pr_hdmi_alert(  "[L1]" fmt, ##args)
    #define HDMI_CRIT(fmt, args...)          rtd_pr_hdmi_crit(   "[L2]" fmt, ##args)
    #define HDMI_ERR(fmt, args...)           rtd_pr_hdmi_err(    "[L3]" fmt, ##args)
    #define HDMI_WARN(fmt, args...)          rtd_pr_hdmi_warn("[L4]" fmt, ##args)
    #define HDMI_NOTICE(fmt, args...)        rtd_pr_hdmi_notice( "[L5]" fmt, ##args)
    #define HDMI_INFO(fmt, args...)          rtd_pr_hdmi_info(   "[L6]" fmt, ##args)
    #define HDMI_DBG(fmt, args...)           rtd_pr_hdmi_debug(  "[L7]" fmt, ##args)
    #define HDMI_PRINTF(fmt, args...)        rtd_pr_hdmi_notice( "[L5]" fmt, ##args)
#else
    #define HDMI_EMG(fmt, args...)
    #define HDMI_ALT(fmt, args...)
    #define HDMI_CRIT(fmt, args...)
    #define HDMI_ERR(fmt, args...)
    #define HDMI_WARN(fmt, args...)
    #define HDMI_NOTICE(fmt, args...)
    #define HDMI_INFO(fmt, args...)
    #define HDMI_DBG(fmt, args...)
    #define HDMI_PRINTF(fmt, args...)
#endif

enum
{
    HDMI_FLOW_PRPINT_HDCP = (0x01<<0),
    HDMI_FLOW_PRPINT_AUDIO = (0x01<<1),
    HDMI_FLOW_PRPINT_INFOFRAME = (0x01<<2),
    HDMI_FLOW_PRPINT_CEDERR = (0x01<<3),
    HDMI_FLOW_PRPINT_SCDC = (0x01<<4),
    HDMI_FLOW_PRPINT_FRL = (0x01<<5),    // HDMI2.1 Link Training
    HDMI_FLOW_PRPINT_HD21 = (0x01<<6),    // HDMI2.1 flow
    HDMI_FLOW_PRPINT_VFE = (0x01<<7),    // for iocrtl debugging
    HDMI_FLOW_PRPINT_PWR = (0x01<<8),    // for power saving debugging
    HDMI_FLOW_PRPINT_AMDFREESYNC = (0x01<<9),    // for AMD freesync debugging
    HDMI_FLOW_PRPINT_DSCD = (0x01<<10),    // for DSCD debugging
    HDMI_FLOW_PRPINT_SOCTS = (0x01<<11),    // for enable or disable SoCTS print HDMI return message
};

enum
{
    HDMI_DEBUG_PACKET_USE_ORIGINAL = 0,
    HDMI_DEBUG_PACKET_FORCE_DISABLE = 1,
    HDMI_DEBUG_PACKET_FORCE_ENABLE = 2,
};



// HDMI debugging flow print control
extern unsigned int debugging_flow_print_flag;

#define HDCP_FLOW_PRINT_FLAG                 ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_HDCP) >> 0)
#define SET_HDCP_FLOW_PRINT_FLAG()           (debugging_flow_print_flag |= HDMI_FLOW_PRPINT_HDCP)
#define CLR_HDCP_FLOW_PRINT_FLAG()           (debugging_flow_print_flag &= ~HDMI_FLOW_PRPINT_HDCP)
#define HDCP_FLOW_PRINTF(fmt, args...)       if(HDCP_FLOW_PRINT_FLAG)  rtd_pr_hdmi_notice(  "[HDCP][FLOW]" fmt, ##args)   
#define HDCP_INFO(fmt, args...)              rtd_pr_hdmi_info(   "[HDCP][L6]" fmt, ##args)
#define HDCP_WARN(fmt, args...)              rtd_pr_hdmi_warn("[HDCP][L4]" fmt, ##args)

#define AUDIO_FLOW_PRINT_FLAG                ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_AUDIO) >> 1)
#define SET_AUDIO_FLOW_PRINT_FLAG()          (debugging_flow_print_flag |=  HDMI_FLOW_PRPINT_AUDIO)
#define CLR_AUDIO_FLOW_PRINT_FLAG()          (debugging_flow_print_flag &= ~HDMI_FLOW_PRPINT_AUDIO)
#define AUDIO_FLOW_PRINTF(fmt, args...)      if(AUDIO_FLOW_PRINT_FLAG)  rtd_pr_hdmi_notice(  "[AUDIOF]" fmt, ##args)   
extern unsigned short g_audio_print_cycle_thd;

#define INFOFRAME_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_INFOFRAME) >> 2)
#define SET_INFOFRAME_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_INFOFRAME)
#define CLR_INFOFRAME_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_INFOFRAME)
#define INFOFRAME_FLOW_PRINTF(fmt, args...)  if(INFOFRAME_FLOW_PRINT_FLAG) rtd_pr_hdmi_notice(  "[INFOF]" fmt, ##args)   

extern unsigned char g_infoframe_drm_debug_print;
extern unsigned char g_infoframe_avi_debug_print;
extern unsigned char g_infoframe_spd_debug_print;
extern unsigned char g_infoframe_audio_debug_print;
extern unsigned char g_infoframe_vsi_debug_print;
extern unsigned char g_infoframe_dvs_debug_print;
extern unsigned char g_infoframe_hdr10pvsi_debug_print;
extern unsigned char g_infoframe_rsv3_debug_print;
extern unsigned char g_infoframe_cvtem_debug_print;
extern unsigned char g_infoframe_vtem_debug_print;
extern unsigned char g_infoframe_hdremp_debug_print;
extern unsigned char g_infoframe_vsem_debug_print;
extern unsigned char g_qms_debug_print;

#define CEDERR_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_CEDERR) >> 3)
#define SET_CEDERR_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_CEDERR)
#define CLR_CEDERR_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_CEDERR)
#define CEDERR_FLOW_PRINTF(fmt, args...)  if(CEDERR_FLOW_PRINT_FLAG) rtd_pr_hdmi_notice(  "[CEDF]" fmt, ##args) 
extern unsigned short g_char_bit_print_cycle_cnt_thd;

#define SCDC_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_SCDC) >> 4)
#define SET_SCDC_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_SCDC)
#define CLR_SCDC_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_SCDC)
#define SCDC_FLOW_PRINTF(fmt, args...)  if(SCDC_FLOW_PRINT_FLAG) rtd_pr_hdmi_notice(  "[SCDCF]" fmt, ##args) 
extern unsigned short g_scdc_print_cycle_cnt_thd;

#define PWR_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_PWR) >> 8)
#define SET_PWR_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_PWR)
#define CLR_PWR_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_PWR)
#define PWR_FLOW_PRINTF(fmt, args...)  if(PWR_FLOW_PRINT_FLAG) rtd_pr_hdmi_notice(  "[PWR]" fmt, ##args) 
#define PWR_FLOW_INFO(fmt, args...)  if(PWR_FLOW_PRINT_FLAG) rtd_pr_hdmi_info(  "[PWR][L6]" fmt, ##args) 
extern unsigned short g_pwr_print_cycle_cnt_thd;

#define PWR_EMG(fmt, args...)  rtd_pr_hdmi_emerg(  "[PWR][L0]" fmt, ##args) 

#define AMDFREESYNC_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_AMDFREESYNC) >> 9)
#define SET_AMDFREESYNC_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_AMDFREESYNC)
#define CLR_AMDFREESYNC_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_AMDFREESYNC)
#define AMDFREESYNC_FLOW_PRINTF(fmt, args...)  if(AMDFREESYNC_FLOW_PRINT_FLAG) rtd_pr_hdmi_notice("[HDMI][AMD]" fmt, ##args) 
extern unsigned short g_amd_vsdb_debug_thd;

//------------------------------------------------------------------
// HDMI2.1
//------------------------------------------------------------------
#define FRL_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_FRL) >> 5)    //for data align issue debugging. for EQ/disaprity error debugging
#define SET_FRL_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_FRL)
#define CLR_FRL_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_FRL)
#define FRL_DBG(fmt, args...)  if(FRL_FLOW_PRINT_FLAG) rtd_pr_hdmi_debug(  "[HD21-FRL][L7]" fmt, ##args)
#define FRL_INFO(fmt, args...)  if(FRL_FLOW_PRINT_FLAG) rtd_pr_hdmi_info(  "[HD21-FRL][L6]" fmt, ##args)
#define FRL_WARN(fmt, args...)  if(FRL_FLOW_PRINT_FLAG) rtd_pr_hdmi_warn(  "[HD21-FRL][L4]" fmt, ##args)

#define HD21_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_HD21) >> 6)
#define SET_HD21_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_HD21)
#define CLR_HD21_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_HD21)
#define HD21_DBG(fmt, args...)  if(HD21_FLOW_PRINT_FLAG) rtd_pr_hdmi_debug(  "[HD21][L7]" fmt, ##args)
#define HD21_INFO(fmt, args...)  if(HD21_FLOW_PRINT_FLAG) rtd_pr_hdmi_info(  "[HD21][L6]" fmt, ##args)
#define HD21_WARN(fmt, args...)  if(HD21_FLOW_PRINT_FLAG) rtd_pr_hdmi_warn(  "[HD21][L4]" fmt, ##args)

#define VFE_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_VFE) >> 7)
#define SET_VFE_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_VFE)
#define CLR_VFE_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_VFE)
#define VFE_FLOW_PRINTF(fmt, args...)  if(VFE_FLOW_PRINT_FLAG) rtd_pr_hdmi_notice(  "[VFEF]" fmt, ##args) 

#define DSCD_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_DSCD) >> 10)
#define SET_DSCD_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_DSCD)
#define CLR_DSCD_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_DSCD)
#define DSCD_FLOW_PRINTF(fmt, args...)  if(DSCD_FLOW_PRINT_FLAG) rtd_pr_hdmi_notice("[HDMI][DSCDF]" fmt, ##args)
extern unsigned short g_dscd_print_cycle_cnt_thd;

#define SOCTS_FLOW_PRINT_FLAG            ((debugging_flow_print_flag &  HDMI_FLOW_PRPINT_SOCTS) >> 11)
#define SET_SOCTS_FLOW_PRINT_FLAG()      (debugging_flow_print_flag |=HDMI_FLOW_PRPINT_SOCTS)
#define CLR_SOCTS_FLOW_PRINT_FLAG()      (debugging_flow_print_flag &=~HDMI_FLOW_PRPINT_SOCTS)
#define SOCTS_INFO(fmt, args...)  if(SOCTS_FLOW_PRINT_FLAG) rtd_pr_hdmi_info("[HDMI][SOCTS][L6]" fmt, ##args)

#define HDMI_OFMS_INFO(fmt, args...)              rtd_pr_hdmi_info(   "[OFMS][L6]" fmt, ##args)
#define HDMI_OFMS_WARN(fmt, args...)              rtd_pr_hdmi_warn(  "[OFMS][L4]" fmt, ##args)
#define HDMI_OFMS_FSM_PRINT(fmt, args...) if((hdmi_rx[newbase_hdmi_get_current_display_port()].video_fsm != MAIN_FSM_HDMI_VIDEO_READY)) rtd_pr_hdmi_notice("[HDMI][OFMS]" fmt, ##args)

///////////////////////////////////////////////////////////////////
//#define HDMI_EDID_DBG_EN            // enable HDMI EDID debug message

#define HDMI_EDID_EMG(fmt, args...)     rtd_pr_hdmi_emerg(  "[EDID][L0]" fmt, ##args)
#define HDMI_EDID_ERR(fmt, args...)     rtd_pr_hdmi_err(  "[EDID][L3]" fmt, ##args)
#define HDMI_EDID_INFO(fmt, args...)    rtd_pr_hdmi_info(  "[EDID][L6]" fmt, ##args)

#ifdef HDMI_EDID_DBG_EN
#include <rtd_log/rtd_module_log.h>
#define HDMI_EDID_DBG(fmt, args...)     rtd_pr_hdmi_debug(  "[EDID][L7]" fmt, ##args)
#else
#define HDMI_EDID_DBG(fmt, args...)     do {} while(0)     
#endif

#define ERP_WARN(fmt, args...)    rtd_pr_hdmi_warn(  "[ERP][L4]" fmt, ##args)
#define ERP_INFO(fmt, args...)    rtd_pr_hdmi_info(  "[ERP][L6]" fmt, ##args)

#ifdef HDMI_ERP_DBG_EN
#define ERP_DBG(fmt, args...)     rtd_pr_hdmi_debug(  "[ERP][L7]" fmt, ##args)
#else
#define ERP_DBG(fmt, args...)     do {} while(0)
#endif

#define OVERFLOW_U32_MAX_THD    4000000000U
//------------------------------------------------------------------
//hdmi-sysfs debug_ctrl
//------------------------------------------------------------------

extern unsigned char g_hdmi_debug_force_avmute[4];

extern unsigned char g_hdmi_pixel_monitor_en;
extern unsigned int g_hdmi_pixel_monitor_x;
extern unsigned int g_hdmi_pixel_monitor_y;

extern unsigned int g_hdmi_err_det_message_interval;

extern unsigned char g_hdmi_debug_offms_continuous;
extern unsigned int g_hdmi_debug_add_onms_mask;
extern unsigned char g_hdmi_bypass_connect_disconnect;
extern unsigned int g_hdmi_check_commcrc;
extern unsigned char g_is_hdmi_crc_continue_mode;
extern unsigned char g_check_commcrc_num;
extern unsigned char g_picture_non_static_thd;
extern unsigned int g_hdmi_check_commcrc_over_ms;
extern unsigned int g_hdmi_check_commcrc_max_fail_times;
extern unsigned char g_yuv422_cd_pixel_cnt_thd;
extern unsigned char g_yun422_cd_frame_cnt_thd;
extern unsigned char g_hdmi_dsc_fw_mode_en;

extern unsigned char g_hdmi_force_5v_debug;

extern unsigned char g_hdmi_vrr_debug_disable;

#endif // __HDMI_DEBUG_H__

