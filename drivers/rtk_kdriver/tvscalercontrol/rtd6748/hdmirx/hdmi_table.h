#ifndef __HDMI_TABLE_H__
#define __HDMI_TABLE_H__

#include "hdmi_common.h"

extern const unsigned char* p_default_hdmi_edid_table[4];
extern const unsigned char* p_default_hdmi_edid_384_table[4];
 
//----------------------------------------------------------------------------------------------------//
//-----------------------------------Device Compatibility Region Start--------------------------------//
//----------------------------------------------------------------------------------------------------//
typedef enum _HDMI_SPECIFIC_DEVICE_{
	HDMI_SPECIFIC_DEVICE_NONE = 0,
//---------------------------With SPD info-----------------------------------------------
	HDMI_SPECIFIC_DEVICE_SONY_BDP_S1500_S570,
	HDMI_SPECIFIC_DEVICE_SCEI_PS4,
	HDMI_SPECIFIC_DEVICE_SAMSUNG_BD_F7500,    //WOSQRTK-11751, green transit issue
	HDMI_SPECIFIC_DEVICE_SAMSUNG_STB_OPENHD,  //SMT_H3021
	HDMI_SPECIFIC_DEVICE_SK_STB,
	HDMI_SPECIFIC_DEVICE_KT_STB_IC4000,  //[STB-46] KT UHD3 Air(IC4000)
	HDMI_SPECIFIC_DEVICE_KT_STB_IC1000,
	HDMI_SPECIFIC_DEVICE_KT_STB_IC1100,
	HDMI_SPECIFIC_DEVICE_KT_STB_KT600EJ, //[STB-29 GX-KT600EJ] , 3840x2160@60,YUV420, 10bit, HDCP22, VIC97
	HDMI_SPECIFIC_DEVICE_KT_STB_KT601EL, //[STB-40 GX-KT601EL]
	HDMI_SPECIFIC_DEVICE_LHB_755, // [HOME-05] LHB755 - LG
	HDMI_SPECIFIC_DEVICE_ROKU_ULTRA_4640R,
	HDMI_SPECIFIC_DEVICE_OPPO_UDP203,    // Device power up, logo tearing
	HDMI_SPECIFIC_DEVICE_THX_U300STB,    //WOSQRTK-12222
	HDMI_SPECIFIC_DEVICE_GX_CJ680CLSTB,  //WOSQRTK-12220
	HDMI_SPECIFIC_DEVICE_PANASONIC_DMP_BD90,
	HDMI_SPECIFIC_DEVICE_H5X,
	HDMI_SPECIFIC_DEVICE_XIAOMI_MI_BOX_S, //KTASKWBA-20144
//---------------------------Without SPD info-----------------------------------------------
	HDMI_SPECIFIC_DEVICE_KT_STB_KAO_CT1100,
	HDMI_SPECIFIC_DEVICE_PC40000,		// TBD, The device will be removed
	HDMI_SPECIFIC_DEVICE_APPLETV,		// TBD, The device will be removed
	HDMI_SPECIFIC_DEVICE_LG_BD550,
	HDMI_SPECIFIC_DEVICE_NUM
}HDMI_SPECIFIC_DEVICE_E;


typedef struct
{
     HDMI_SPECIFIC_DEVICE_E device_id;
     unsigned char check_condition_min_num;     //Check condition minimum num
     unsigned int check_characteristic_flag;
     unsigned int action_process_flag;     //Action special process
     unsigned int action_delay_Time;    //Action Delay timing on scaler IMD
     unsigned char action_phy_stable_cnt;    //Action phy stable count
     unsigned int action_str_hpd_low_ms;     //Action overwrite STR low duration
     unsigned int action_onms_det_mask;     //Action mask specified bit of onms watchdog/int
}HDMI_DEVICE_COMPABITILITY_CONFIG_T;

typedef struct
{
     HDMI_SPECIFIC_DEVICE_E device_id;
     unsigned char check_vn_name[8]; 
     unsigned char check_pd_desc[16];
     unsigned char check_src_dev_info;
}HDMI_DEVICE_CHECK_SPD_CONDITION;

typedef struct
{
     unsigned int check_h_act;
     unsigned int check_v_act;
}HDMI_CHECK_TIMING_H_V_ACTIVE;

typedef struct
{
     HDMI_SPECIFIC_DEVICE_E device_id;
     unsigned char check_no_spd;
     HDMI_CHECK_TIMING_H_V_ACTIVE check_hv_active;
     unsigned char check_avi_vic;
     unsigned char check_color_space;
     unsigned char check_color_depth;
}HDMI_DEVICE_CHECK_TIMING_CONDITION;

typedef struct
{
     HDMI_SPECIFIC_DEVICE_E device_id;
     HDMI_HDCP_E check_current_hdcp_mode;
     //unsigned char check_aksv[5];     //TBD
}HDMI_DEVICE_CHECK_HDCP_CONDITION;

#define CHECK_NONE                              0x00
#define CHECK_NO_NEED                           0xFF
#define CHECK_FLAG_OFF                          0
#define CHECK_FLAG_SPD                          _BIT0
#define CHECK_FLAG_TIMINGINFO                   _BIT1
#define CHECK_FLAG_HDCP                         _BIT2

#define ACTION_FLAG_OFF                         0
#define ACTION_FLAG_DELAY                       _BIT1    //Execute dalay at before display. Delay time equal different with DISPLAYOK and newbase_hdmi_check_stable_before_display
#define ACTION_FLAG_DELAY_ACDC_ONCE             _BIT2     //Execute dalay at once after AC/DC on
#define ACTION_FLAG_HPD                         _BIT3    //Execute HPD when STR.
#define ACTION_FLAG_Z0_HPD                      _BIT4    //Execute HPD when STR.
#define ACTION_FLAG_HDCP_ENC_DIS                _BIT5    //Execute ENC/DIS HDCP write
#define ACTION_FLAG_WAIT_HDCP14                 _BIT6    //Execute waiting HDCP14
#define ACTION_FLAG_OTHER                       _BIT7    //Execute custom behavior by device id. 
#define ACTION_FLAG_PHY_STABLE_CNT              _BIT8    // phy stable count for device compatibility
#define ACTION_FLAG_FORCE_PS_OFF                _BIT9    // do not enter PS for device compatibility
#define ACTION_FLAG_MASK_ONMS_DET                _BIT10    // fine tune onms watchdog&int for device compatibility

#define ACTION_DELAY_MAX_MS    3000

extern const HDMI_DEVICE_CHECK_SPD_CONDITION g_device_check_spd[];
extern const unsigned char g_device_check_spd_table_size;

extern const HDMI_DEVICE_CHECK_TIMING_CONDITION g_device_check_timing[];
extern const unsigned char g_device_check_timing_table_size;

extern const HDMI_DEVICE_CHECK_HDCP_CONDITION g_device_check_hdcp[];
extern const unsigned char g_device_check_hdcp_table_size;

extern const HDMI_DEVICE_COMPABITILITY_CONFIG_T g_device_compatibility_config[];
extern const unsigned char g_device_compatibility_config_table_size;

//-----------------------------------Device Compatibility Region End--------------------------------//

//--------------------------------------------------------------------------------------------------//
//------------------------------------Audio Region Start--------------------------------------------//
//--------------------------------------------------------------------------------------------------//


//---------------------------------------Audio Region End-------------------------------------------//
typedef struct{
    unsigned int freq;
    unsigned int freq_tol;
}HDMI_AUDIO_FREQ_TABLE_T;

extern const HDMI_AUDIO_FREQ_TABLE_T g_audio_freq_table[];
extern const unsigned char g_audio_freq_table_size;
//---------------------------------------Audio Region End-------------------------------------------//



//--------------------------------------------------------------------------------------------------//
//------------------------------------PCB Parameter Region Start------------------------------------//
//--------------------------------------------------------------------------------------------------//
typedef enum
{
    HDMI_PCB_USE_DEFAULT = 0,    // When read 0 from PCB 
    HDMI_PCB_FORCE_ENABLE,
    HDMI_PCB_FORCE_DISABLE,
    HDMI_PCB_RESERVED
}HDMI_PCB_CONFIG_SEL;

typedef enum
{
     HDMI_FLOW_CFG_GENERAL = 0,
     HDMI_FLOW_CFG_HPD,
     HDMI_FLOW_CFG_HDMI21_LT1,
     HDMI_FLOW_CFG_HDMI21_LT2,
     HDMI_FLOW_CFG_ERR_DET,
     HDMI_FLOW_CFG_DFE,
     HDMI_FLOW_CFG_DSC,
     HDMI_FLOW_CFG_TIMING_MEASURE,
     HDMI_FLOW_CFG_GROUP_NUM
}HDMI_FLOW_CFG_GROUP;

typedef enum
{
     HDMI_FLOW_CFG0_VALID = 0,
     HDMI_FLOW_CFG0_SMART_FIT_ENABLE,		// it can reduce 100ms when disable.but only measure by OFMS IP.
     HDMI_FLOW_CFG0_V4L2_ENABLE,                       // 0: Disable 1: ENABLE, 2: Disable.
     HDMI_FLOW_CFG0_FAST_DETECT_ENABLE,            // 0: normal mode, 1: Enable FAST DETECT MODE, bypass compability check and hdcp enc check
     HDMI_FLOW_CFG0_HDCP22_DISABLE,                 //0: HDCP22 workable; 1 (HDMI_PCB_FORCE_ENABLE): HDCP22 disable
     HDMI_FLOW_CFG0_WAITSYNC_STABLE_CNT_THD,	//[WOSQRTK-8394 -Green transient appears - SONY S570/S470]
     HDMI_FLOW_CFG0_MEASURE_ERR_CNT_THD,
     HDMI_FLOW_CFG0_TMDS_CLOCK_TOLERENCE_MIN,
     HDMI_FLOW_CFG0_PHY_STABLE_CNT_THD,
     HDMI_FLOW_CFG0_BEFORE_DISPLAY_BASE_DELAY_MS,
     HDMI_FLOW_CFG0_DISABLE_AKSV_INTERRUPT,  // 0: Interrupt mode, 1: Polling mode
     HDMI_FLOW_CFG0_MEASURE_HSYNC_EXTRA_TIMEOUT_CNT,
     HDMI_FLOW_CFG0_DEMODE_USE_VIC_CORRECTION,
     HDMI_FLOW_CFG0_TIMING_MONITOR_ENABLE,
     HDMI_FLOW_CFG0_FORCE_VIC_MEASURE,
     HDMI_FLOW_CFG0_MAX_FRL_MODE,     //HDMI2.1 FRL MODE, 0: TMDS MODE, 1: 3L3G, 2:3L6G, 3:4L6G, 4:4L8G, 5: 4L10G, 6: 4L12G
     HDMI_FLOW_CFG0_SUPPORT_DSC,    //HDMI2.1 DSC support, 0: default not support, 1: force support, 2: force not support
     HDMI_FLOW_CFG0_ENABLE_HDMI_AV_SYNC,// 1:  Audio output should check whether scaler video path is excuted  or not, for AUDIO only mode; 0: never check scaler video path
     HDMI_FLOW_CFG0_VIDEO_FSM_MONITOR_ENABLE,
     HDMI_FLOW_CFG0_NUM
}HDMI_FLOW_CFG0_INDEX;    //HDMI_FLOW_CFG_GENERAL

typedef enum
{
     HDMI_FLOW_CFG1_VALID = 0,
     HDMI_FLOW_CFG1_DISABLE_PORT_FAST_SWITCH,  // 0: SUPPORT FAST SWITCH, 1: DISABLE FAST SWITCH
     HDMI_FLOW_CFG1_ENABLE_YUN422_CD_DET_EN,
     HDMI_FLOW_CFG1_DEF_HPD_LOW_10MS,    //For EDID Change, 70*10 = 700ms ,default value (+ resume already had 500ms low)
     HDMI_FLOW_CFG1_DEF_STR0_HPD_10MS,	 //For DC ON/OFF. 31*10 = 310 ms; HDMI_STR_TOGGLE_HPD_MS
     HDMI_FLOW_CFG1_DEF_STR1_HPD_10MS,	 //For DC ON/OFF. 31*10 = 310 ms; HDMI_STR_TOGGLE_HPD_MS
     HDMI_FLOW_CFG1_DEF_STR2_HPD_10MS,	 //For DC ON/OFF. 31*10 = 310 ms; HDMI_STR_TOGGLE_HPD_MS
     HDMI_FLOW_CFG1_DEF_STR3_HPD_10MS,	 //For DC ON/OFF. 31*10 = 310 ms; HDMI_STR_TOGGLE_HPD_MS
     HDMI_FLOW_CFG1_DEF_PHY_THREAD_CYCLE_MS,
     HDMI_FLOW_CFG1_GOOD_TIMING_DEBOUNCE_MS,
     HDMI_FLOW_CFG1_NUM
}HDMI_FLOW_CFG1_INDEX;    //HDMI_FLOW_CFG_HPD

typedef enum
{
     HDMI_FLOW_CFG2_VALID = 0,
     HDMI_FLOW_CFG2_PT_TIMEOUT_METHOD,    // 0: USE FW TIMER, 1: USE IC TIMER, 2:  no timeout limited
     HDMI_FLOW_CFG2_PT_BYPASS,     // 0: normal flow with LT, 1: bypass pattern training, do not check "Lock" and "error count".
     HDMI_FLOW_CFG2_LINK_TRAINING_BYPASS,    // 0: normal flow with LT, 1: bypass LT and Force FRL mode=1, 2: frl mode=2, 3: frl mode=3......
     HDMI_FLOW_CFG2_CLR_FLTUPDATE_TIMEOUT_CNT,    // count for polling, wait tx to clear flt_update flag on normal mode
     HDMI_FLOW_CFG2_CTS_CLR_FLTUPDATE_TIMEOUT_CNT,    // count for polling, wait tx to clear flt_update flag on TX_NORETRAIN mode
     HDMI_FLOW_CFG2_PT_FW_TIMEOUT_MS_THD,
     HDMI_FLOW_CFG2_PT_IC_TIMEOUT_MS_THD,
     HDMI_FLOW_CFG2_PT_TRY_CNT_MAX,
     HDMI_FLOW_CFG2_PT_CHECKPOINT_MAX,
     HDMI_FLOW_CFG2_LINK_TRAINING_NO_BREAK,
     HDMI_FLOW_CFG2_CTS_EXTEND_MODE,
     HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE,
     HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE,     
     HDMI_FLOW_CFG2_LTP_PHY_ADAPTIVE_EN, // 0: disable for speed up LT, 1~3: cycle, enable for phy adaptive
     HDMI_FLOW_CFG2_NUM
}HDMI_FLOW_CFG2_INDEX;    //for HDMI_FLOW_CFG_HDMI21_LT1

typedef enum
{
     HDMI_FLOW_CFG3_VALID = 0,
     HDMI_FLOW_CFG3_PT_REQ_PATTERN_L0,		//
     HDMI_FLOW_CFG3_PT_REQ_PATTERN_L1,		//
     HDMI_FLOW_CFG3_PT_REQ_PATTERN_L2,
     HDMI_FLOW_CFG3_PT_REQ_PATTERN_L3,
     HDMI_FLOW_CFG3_PT_ERROR_THD,
     HDMI_FLOW_CFG3_HD21_SETUP_PLL_6G,
     HDMI_FLOW_CFG3_SCDC_RST_DEBOUNCE_MS,
     HDMI_FLOW_CFG3_DISPARITY_RST_DEBOUNCE_MS,
     HDMI_FLOW_CFG3_NUM
}HDMI_FLOW_CFG3_INDEX;    // for HDMI_FLOW_CFG_HDMI21_LT2

typedef enum
{
     HDMI_FLOW_CFG4_VALID = 0,
     HDMI_FLOW_CFG4_ERRDET_ENABLE,
     HDMI_FLOW_CFG4_LINK_STATUS_MONITOR_ENABLE,
     HDMI_FLOW_CFG4_LSM_V_SYNC_CHANGE_THD,
     HDMI_FLOW_CFG4_LSM_V_SYNC_POLLING_CNT,
     HDMI_FLOW_CFG4_ERR_HANDLER_POLLING_MS,
     HDMI_FLOW_CFG4_BCH_ERR_IRQ_MUTE_THD,    // BCH_ERR IRQ mute threshold. if err > thd, it will mute video, else no change.
     HDMI_FLOW_CFG4_BCH_ERR_IRQ_ENABLE,    // BCH_ERR IRQ enable
     HDMI_FLOW_CFG4_AVMUTE_IRQ_ENABLE,    // AVMUTE IRQ enable,  if scaler support avmute mute, it should disable avmute irq.
     HDMI_FLOW_CFG4_HDMI_DVI_MODE_IRQ_ENABLE,    // HDMI/DVI mode change IRQ enable
     HDMI_FLOW_CFG4_ERR_HANDLER_HDCP14_RECOVERY,
     HDMI_FLOW_CFG4_ERR_HANDLER_RESETPHY_EN,
     HDMI_FLOW_CFG4_ERR_HANDLER_HPD_EN,
     HDMI_FLOW_CFG4_DATA_OFF_SYNC_DET_EN, //0: disable, 1: enable, data off sync timeout detected, source may not support Reauth, do HPD toggle to recover
     HDMI_FLOW_CFG4_TMDS_OFF_SYNC_HPD_TOGGLE, //0: disable (reset scdc and phy), 1: enable (reset HPD)
     HDMI_FLOW_CFG4_CHRGB_CHECK_EN,
     HDMI_FLOW_CFG4_TMDS_CONFIG_CHECK_EN,
     HDMI_FLOW_CFG4_VIC_CONFIRM_CNT_THD,        //If use measure with VIC correction, it should confirm VIC many times related with the threshold.
     HDMI_FLOW_CFG4_BCH_ERR_IRQ_STOP_THD,    // BCH ERR STOP THD
     HDMI_FLOW_CFG4_NO_PKT_AND_VSYNC_UNSTABLE_TIMEOUT,
     HDMI_FLOW_CFG4_NUM
}HDMI_FLOW_CFG4_INDEX;    // for HDMI_FLOW_CFG_ERR_DET

typedef enum
{
     HDMI_FLOW_CFG5_VALID = 0,
     HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE,
     HDMI_FLOW_CFG5_FW_LE_SCAN_RANGE,
     HDMI_FLOW_CFG5_FW_LE_SCAN_CED_THRESHOLD,
     HDMI_FLOW_CFG5_VSYNC_FREQ_MIN,
     HDMI_FLOW_CFG5_VSYNC_FREQ_MAX,
     HDMI_FLOW_CFG5_TMDS_CLK_CHANGE_THD,
     HDMI_FLOW_CFG5_NUM
}HDMI_FLOW_CFG5_INDEX;    // for HDMI_FLOW_CFG_DFE

typedef enum
{
     HDMI_FLOW_CFG6_VALID = 0,
     HDMI_FLOW_CFG6_DSCD_CHECK_CRC_VAILD_EN,
     HDMI_FLOW_CFG6_DSCD_CHECK_OUTPUT_EN,
     HDMI_FLOW_CFG6_DSCD_CHECK_OUTPUT_CNT_THD,
     HDMI_FLOW_CFG6_DSCD_OUTPUT_ERROR_RECOVERY_EN,
     HDMI_FLOW_CFG6_DSCD_OUTPUT_ERROR_RECOVERY_THD,
     HDMI_FLOW_CFG6_NUM
}HDMI_FLOW_CFG6_INDEX;    // for HDMI_FLOW_CFG_DSC

typedef enum
{
     HDMI_FLOW_CFG7_VALID = 0,
     HDMI_FLOW_CFG7_OFMS_MAX_H_LEN,
     HDMI_FLOW_CFG7_V4L2_MAX_H_LEN_ON_2P0,
     HDMI_FLOW_CFG7_HDCP_IIC_STAY_ONE_TIMEOUT,
     HDMI_FLOW_CFG7_NUM
}HDMI_FLOW_CFG7_INDEX;    // for HDMI_FLOW_CFG_TIMING_MEASURE

#define STATUS_PCB_UPDATE_FLAG    _BIT0
#define STATUS_SW_UPDATE_FLAG    _BIT1

#define CTRL_OFF_FLAG    0
#define CTRL_PCB_WRITE_ENABLE_FLAG    _BIT0
#define CTRL_SW_WRITE_ENABLE_FLAG    _BIT1
#define PCBINFO_CTRL_CHECK(x, y)    (x == (x&y))

#define ENTRY_NAME_MAX_LENGTH 32
#define GROUP_NAME_MAX_LENGTH 32

typedef struct
{
     const unsigned char start_bit_index;
     const unsigned char end_bit_index;
     unsigned int default_value;	//max 32 bit
     unsigned char entry_status;
     unsigned char ctrl_config;
     const char entry_name[ENTRY_NAME_MAX_LENGTH];
}HDMI_CFG_ENTRY_T;


typedef struct
{
     const HDMI_FLOW_CFG_GROUP pcbinfo_group_id;
     HDMI_CFG_ENTRY_T*  p_cfg_table;
     const unsigned char cfg_table_length;
     const char pcb_enum_name[GROUP_NAME_MAX_LENGTH];
}HDMI_FLOW_CFG_GROUP_TABLE_T;

extern const HDMI_FLOW_CFG_GROUP_TABLE_T p_flow_cfg_group_table[HDMI_FLOW_CFG_GROUP_NUM];
//-----------------------------------PCB Parameter Region End-------------------------------------//

//-----------------------------------VIC correction check Region Start-------------------------------------//

extern const unsigned char g_vic_check_list[]; // always need to check differenc with OFMS and VIC table.
extern const unsigned char g_vic_check_list_num;

//-----------------------------------VIC correction check list Region End-------------------------------------//


//-----------------------------------Panel max vfeq vs HDMI2.1 Max PLL setting Start-------------------------------------//
typedef struct
{
    unsigned short panel_max_v_freq;
    unsigned short hd21_max_pll;
}VFREQ_VS_HD21_MAX_PLL_T;

extern const VFREQ_VS_HD21_MAX_PLL_T g_panel_vfreq_hd21_pll_mapping_table[]; // always need to check differenc with OFMS and VIC table.
extern const unsigned char g_panel_vfreq_hd21_pll_mapping_table_num;
//-----------------------------------Panel max vfeq vs HDMI2.1 Max PLL setting End-------------------------------------//


#endif // __HDMI_TABLE_H__

