#ifndef __HDMI_POWER_SAVING_H__
#define __HDMI_POWER_SAVING_H__

#define HDMI_POWER_SAVING_MAX_STABLE_CNT    500
#define HDMI_POWER_SAVING_MIN_STABLE_CNT    300
#define HDMI_POWER_SAVING_MIN_PS_MEASURE_STABLE_CNT    20


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
}HDMI_POWER_SAVEING_MODE_T;

//------------------------------------------------------------------
// Common
//------------------------------------------------------------------
extern void newbase_hdmi_power_saving_init(void);
extern void newbase_hdmi_power_saving_handler(unsigned char port);
extern HDMI_POWER_SAVEING_MODE_T newbase_hdmi_power_saving_mode_probe(unsigned char port);
extern void newbase_hdmi_power_saving_dump_status(unsigned char port);

//------------------------------------------------------------------
// extern power saving API
//------------------------------------------------------------------
extern void newbase_hdmi_set_active_source(unsigned char port);
extern void newbase_hdmi_clr_active_source(unsigned char port);

extern void newbase_hdmi_reset_power_saving_state(unsigned char port);
extern void newbase_hdmi_power_saving_wakeup(unsigned char port, unsigned char wakup_condition);
extern HDMI_POWER_SAVEING_FSM_T newbase_hdmi_get_power_saving_state(unsigned char port);

#endif //__HDMI_POWER_SAVING_H__
