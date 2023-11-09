#ifndef __HDMI_DSC_H__
#define __HDMI_DSC_H__

typedef enum {
    DSCD_FSM_WAIT_LT_PASS = 0,
    DSCD_FSM_INIT,
    DSCD_FSM_WAIT_CVTEM,
    DSCD_FSM_SET_FW_PPS,
    DSCD_FSM_WAIT_PPS_READY,
    DSCD_FSM_PPS_MEASURE,
    DSCD_FSM_RUN,
} HDMI_DSCD_FSM_T;

#ifdef CONFIG_RTK_KDRV_HDMI_ENABLE_HDMI21

#else
#define lib_hdmi_dsc_crt_reset()
#define lib_hdmi_dsc_double_buffer_enable(...)
#define lib_hdmi_dsc_fw_mode_enable(enable)
#define lib_hdmi_dsc_set_fw_pps(...)
#define lib_hdmi_dsc_get_cur_pps(...)
#define lib_hdmi_dsc_enable(enable)
#define lib_hdmi_dsc_set_crc_en(enable)
#define lib_hdmi_dsc_get_dscd_out_crc()    (0)
#define lib_hdmi_dsc_get_dscd_in_crc()    (0)
#define lib_hdmi_dsc_get_dscd_pps_crc()    (0)
#define lib_hdmi_dsc_get_crt_on()   (0)
#define lib_hdmi_dscd_timing_monitor_en(enable)
#define lib_hdmi_dscd_is_timing_monitor_en()    (0)
#define lib_hdmi_dsc_set_timing_monitor_sta0(...)
#define lib_hdmi_dsc_set_timing_monitor_end0(...)
#define lib_hdmi_dsc_set_timing_monitor_sta1(...)
#define lib_hdmi_dsc_set_timing_monitor_end1(...)
#define lib_hdmi_dsc_get_timing_monitor_result(...)    (0)

#define newbase_hdmi_hd21_dsc_enanble(...)  (0)
#define newbase_hdmi_dscd_tm_vbackporch(...)  (0)
#define newbase_hdmi_hd21_dsc_handler(port)
#define newbase_hdmi_check_dscd_output_valid()
#define newbase_hdmi_hd21_dsc_get_current_pps_timing() (NULL)
#define newbase_hdmi_hd21_dsc_get_colordepth() (0)
#define _hdmi_hd21_dsc_fsm_str(fsm)    "DSC NOT SUPPORT"

#define GET_DSCD_FSM() (0)
#define SET_DSCD_FSM(fsm)	(0)

#define GET_DSCD_RUN_PORT() (0xF)

#endif

#endif //__HDMI_DSC_H__

