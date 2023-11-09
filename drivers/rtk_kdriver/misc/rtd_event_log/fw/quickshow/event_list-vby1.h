#ifndef __RTK_MODULE_EVENT_VBY1_H__
#define __RTK_MODULE_EVENT_VBY1_H__
/*
######################################################################################
# VBY1 EVENT DEFINE
######################################################################################
*/
typedef enum
{
    QUICKSHOW_VBY1_FUN_RTK_AMP_INIT_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_RTK_AMP_INIT_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_UNMUTE_AUDIO_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_UNMUTE_AUDIO_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_VBY1_TX_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_VBY1_TX_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_HDMI_RX_INIT_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_HDMI_RX_INIT_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_NEWBASE_HDMI_RX_DETECT_TASK_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_NEWBASE_HDMI_RX_DETECT_TASK_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_FWIF_COLOR_SET_STRUCTCOLORDATAFACMODETYPE_QUICK_SHOW_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_FWIF_COLOR_SET_STRUCTCOLORDATAFACMODETYPE_QUICK_SHOW_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_SCALER_INITRPCSHAREMEM_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_SCALER_INITRPCSHAREMEM_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_VIPTABLEINIT_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_VIPTABLEINIT_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_SETUP_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_SETUP_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_GPIO_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_GPIO_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_PWM_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_PWM_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_I2C_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_I2C_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_SOURCE_SELECT_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_SOURCE_SELECT_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_PANEL_SEQ_1_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_PANEL_SEQ_1_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_PANEL_SEQ_2_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_PANEL_SEQ_2_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_PANEL_LAST_HANDLER_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_PANEL_LAST_HANDLER_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_HDMI_PORT_SCAN_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_HDMI_PORT_SCAN_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_SCALER_START_EVENT_ID,
    QUICKSHOW_VBY1_FUN_SCALER_END_EVENT_ID,
    QUICKSHOW_VBY1_FUN_EVENT_MAX
}QUICKSHOW_VBY1_FUN_EVENT_ID;

/*
######################################################################################
# VBY1 EVENT TYPE DEFINE
######################################################################################
*/
typedef enum
{
    QUICKSHOW_VBY1_FUN_TYPE_ID,
    QUICKSHOW_VBY1_TYPE_MAX
}QUICKSHOW_VBY1_TYPE_ID;

/*
######################################################################################
# VBY1 EVENT MERGER DW1 DEFINE
######################################################################################
*/
#define   VBY1_DW1_fun_scaler_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_SCALER_END_EVENT_ID)
#define   VBY1_DW1_fun_scaler_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_SCALER_START_EVENT_ID)
#define   VBY1_DW1_fun_scaler    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_SCALER_EVENT_ID)
#define   VBY1_DW1_fun_hdmi_port_scan_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_HDMI_PORT_SCAN_END_EVENT_ID)
#define   VBY1_DW1_fun_hdmi_port_scan_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_HDMI_PORT_SCAN_START_EVENT_ID)
#define   VBY1_DW1_fun_panel_last_handler_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_PANEL_LAST_HANDLER_END_EVENT_ID)
#define   VBY1_DW1_fun_panel_last_handler_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_PANEL_LAST_HANDLER_START_EVENT_ID)
#define   VBY1_DW1_fun_panel_seq_2_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_PANEL_SEQ_2_END_EVENT_ID)
#define   VBY1_DW1_fun_panel_seq_2_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_PANEL_SEQ_2_START_EVENT_ID)
#define   VBY1_DW1_fun_panel_seq_1_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_PANEL_SEQ_1_END_EVENT_ID)
#define   VBY1_DW1_fun_panel_seq_1_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_PANEL_SEQ_1_START_EVENT_ID)
#define   VBY1_DW1_fun_source_select_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_SOURCE_SELECT_END_EVENT_ID)
#define   VBY1_DW1_fun_source_select_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_SOURCE_SELECT_START_EVENT_ID)
#define   VBY1_DW1_fun_i2c_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_I2C_END_EVENT_ID)
#define   VBY1_DW1_fun_i2c_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_I2C_START_EVENT_ID)
#define   VBY1_DW1_fun_pwm_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_PWM_END_EVENT_ID)
#define   VBY1_DW1_fun_pwm_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_PWM_START_EVENT_ID)
#define   VBY1_DW1_fun_gpio_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_GPIO_END_EVENT_ID)
#define   VBY1_DW1_fun_gpio_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_GPIO_START_EVENT_ID)
#define   VBY1_DW1_fun_setup_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_SETUP_END_EVENT_ID)
#define   VBY1_DW1_fun_setup_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_SETUP_START_EVENT_ID)
#define   VBY1_DW1_fun_VipTableInit_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_VIPTABLEINIT_END_EVENT_ID)
#define   VBY1_DW1_fun_VipTableInit_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_VIPTABLEINIT_START_EVENT_ID)
#define   VBY1_DW1_fun_Scaler_InitRPCShareMem_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_SCALER_INITRPCSHAREMEM_END_EVENT_ID)
#define   VBY1_DW1_fun_Scaler_InitRPCShareMem_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_SCALER_INITRPCSHAREMEM_START_EVENT_ID)
#define   VBY1_DW1_fun_fwif_color_SET_StructColorDataFacModeType_quick_show_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_FWIF_COLOR_SET_STRUCTCOLORDATAFACMODETYPE_QUICK_SHOW_END_EVENT_ID)
#define   VBY1_DW1_fun_fwif_color_SET_StructColorDataFacModeType_quick_show_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_FWIF_COLOR_SET_STRUCTCOLORDATAFACMODETYPE_QUICK_SHOW_START_EVENT_ID)
#define   VBY1_DW1_fun_newbase_hdmi_rx_detect_task_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_NEWBASE_HDMI_RX_DETECT_TASK_END_EVENT_ID)
#define   VBY1_DW1_fun_newbase_hdmi_rx_detect_task_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_NEWBASE_HDMI_RX_DETECT_TASK_START_EVENT_ID)
#define   VBY1_DW1_fun_hdmi_rx_init_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_HDMI_RX_INIT_END_EVENT_ID)
#define   VBY1_DW1_fun_hdmi_rx_init_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_HDMI_RX_INIT_START_EVENT_ID)
#define   VBY1_DW1_fun_vby1_tx_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_VBY1_TX_END_EVENT_ID)
#define   VBY1_DW1_fun_vby1_tx_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_VBY1_TX_START_EVENT_ID)
#define   VBY1_DW1_fun_unmute_audio_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_UNMUTE_AUDIO_END_EVENT_ID)
#define   VBY1_DW1_fun_unmute_audio_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_UNMUTE_AUDIO_START_EVENT_ID)
#define   VBY1_DW1_fun_rtk_amp_init_end    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_RTK_AMP_INIT_END_EVENT_ID)
#define   VBY1_DW1_fun_rtk_amp_init_start    MERGER_EVENT_DW1(EVENT_FW_QUICKSHOW, QUICKSHOW_VBY1_MODULE_ID, QUICKSHOW_VBY1_FUN_TYPE_ID, QUICKSHOW_VBY1_FUN_RTK_AMP_INIT_START_EVENT_ID)

#define rtd_vby1_event_log(type, event, event_val, module_reserved)     rtd_quickshow_event_log(type, QUICKSHOW_VBY1_MODULE_ID, event, event_val, module_reserved)
#endif /* __RTK_MODULE_EVENT_VBY1_H__ */
