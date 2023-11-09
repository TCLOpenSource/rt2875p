#ifndef __HDMI_POWER_SAVING_H__
#define __HDMI_POWER_SAVING_H__

#define HDMI_POWER_SAVING_MAX_STABLE_CNT    500
#define HDMI_POWER_SAVING_MIN_STABLE_CNT    300
#define HDMI_POWER_SAVING_MIN_PS_MEASURE_STABLE_CNT    20

// Power Saving
typedef enum {
    HDMI_14 = 0,
    HDMI_20,
    HDMI_21,
    HDMI_20_VS_WIN_OPP,
}HDMI_MODE;

typedef enum {
    PS_FSM_POWER_SAVING_OFF = 0,
    PS_FSM_MEASURE,
    PS_FSM_POWER_SAVING_ON,
} HDMI_POWER_SAVEING_FSM_T;

typedef enum {
    HDMI_POWER_SAVEING_MODE_OFF = 0,
    HDMI_POWER_SAVEING_MODE_MAC_HW,
    HDMI_POWER_SAVEING_MODE_FW_RONLY,
    HDMI_POWER_SAVEING_MODE_VS_WIN_OPP,
    HDMI_POWER_SAVEING_MODE_PHY_GET_STATUS_FAIL,
}HDMI_POWER_SAVEING_MODE_T;

typedef enum {
    HDMI_WIN_OPP_MODE_VS = 0,
    HDMI_WIN_OPP_MODE_DE,
}HDMI_WIN_OPP_MODE_T;

typedef enum {
    HDMI_PS_EVENT_SIGNAL_UNSTABLE = 1,
}HDMI_PS_EVENT;

typedef struct
{
    unsigned char ps_wakeup_request;                // for FW to request to wakeup from power saving state
    #define PS_WAKEUP_5V_DET_CHANGE     (0x1<<0)    // wakeup by 5V status change
    #define PS_WAKEUP_HPD_CHANGE        (0x1<<1)    // wakeup by HPD status change
    #define PS_WAKEUP_CLOCK_UNSTABLE    (0x1<<2)    // wakeup by Clock Stable
    #define PS_WAKEUP_HDCP_REQUEST      (0x1<<3)    // wakeup by HDCP authentication
    #define PS_WAKEUP_SCDC_TMDS_CFG     (0x1<<4)    // wakeup by SCDC
    #define PS_WAKEUP_SCDC_FRL_REQUEST  (0x1<<5)    // wakeup by SCDC FRL
    #define PS_WAKEUP_ACTIVE_SOURCE     (0x1<<6)    // wakeup by Source Selection (source has been selected as a active source)
    #define PS_WAKEUP_IMMEDIATELY       (0x1<<7)    // wakeup by Immediately

    HDMI_POWER_SAVEING_FSM_T ps_fsm;
    HDMI_POWER_SAVEING_MODE_T ps_mode;
    unsigned int  signal_stable_cnt;
    unsigned int  ps_fsm_sub_state;
    unsigned int  ps_round;
} HDMI_PS_ST;

//------------------------------------------------------------------
// Low Level Power Saving API - MAC
//------------------------------------------------------------------
extern void lib_hdmi_mac_reset_power_saving_mode(unsigned char nport);
extern void lib_hdmi_mac_enter_power_saving_mode_hw_mac(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, HDMI_WIN_OPP_MODE_T win_opp_mode, unsigned char hdcp_mode, unsigned int last_pixel2vs_tmds_cnt, unsigned int vs2vs_cnt);
extern void lib_hdmi_mac_exit_power_saving_mode_hw_mac(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, unsigned char hdcp_mode);
extern void lib_hdmi_mac_enter_power_saving_mode_fw_ronly(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, unsigned char hdcp_mode, unsigned int last_pixel2vs_tmds_cnt, unsigned int vs2vs_cnt);
extern void lib_hdmi_mac_exit_power_saving_mode_fw_ronly(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, unsigned char hdcp_mode);
extern void lib_hdmi_mac_enter_power_saving_mode_vs_win_opp(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, unsigned char hdcp_mode, unsigned int last_pixel2vs_tmds_cnt, unsigned int vs2vs_cnt);
extern void lib_hdmi_mac_exit_power_saving_mode_vs_win_opp(unsigned char nport, unsigned char frl_mode, unsigned char scrambled, unsigned char hdcp_mode);
extern void lib_hdmi_mac_dump_power_saving_reg(unsigned char nport);
extern void lib_hdmi_mac_sram_light_sleep_enable(unsigned char nport, unsigned char enable);
extern unsigned char lib_hdmi_mac_phy_power_saving_status(unsigned char nport);

//------------------------------------------------------------------
// Low Level Power Saving API - PHY
//------------------------------------------------------------------
extern void lib_hdmi_phy_reset_power_saving_state(unsigned char nport);
extern void lib_hdmi_phy_enter_power_saving_mode(unsigned char nport, HDMI_MODE mode, unsigned char br_swap);
extern void lib_hdmi_phy_exit_power_saving_mode(unsigned char nport, HDMI_MODE mode, unsigned char br_swap);
extern void lib_hdmi_phy_dump_pow_saving_reg(unsigned char nport);
extern void lib_hdmi_hysteresis_en(unsigned char en);

//------------------------------------------------------------------
// Common
//------------------------------------------------------------------
extern void newbase_hdmi_power_saving_init(void);
extern void newbase_set_hdmi_power_saving_enable(void);
extern void newbase_hdmi_power_saving_handler(unsigned char port);
extern HDMI_POWER_SAVEING_MODE_T newbase_hdmi_power_saving_mode_probe(unsigned char port);
extern int  newbase_hdmi_enter_power_saving(unsigned char port);
extern int  newbase_hdmi_exit_power_saving(unsigned char port);
extern void newbase_hdmi_power_saving_dump_status(unsigned char port);

//------------------------------------------------------------------
// extern power saving API
//------------------------------------------------------------------
extern void newbase_hdmi_set_active_source(unsigned char port);
extern void newbase_hdmi_clr_active_source(unsigned char port);
extern HDMI_PS_ST* newbase_hdmi_power_saving_get_status(unsigned char port);
extern void newbase_hdmi_reset_power_saving_state(unsigned char port);
extern void newbase_hdmi_power_saving_wakeup(unsigned char port, unsigned char wakup_condition);
extern HDMI_POWER_SAVEING_FSM_T newbase_hdmi_get_power_saving_state(unsigned char port);
extern void newbase_hdmi_power_saving_send_event(unsigned char port, HDMI_PS_EVENT event);

#endif //__HDMI_POWER_SAVING_H__
