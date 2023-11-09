#ifndef __HDMI_MAC_2P1_H__
#define __HDMI_MAC_2P1_H__


/////////////////////////HDMI 2.1/////////////////////////////////

#ifdef CONFIG_RTK_KDRV_HDMI_ENABLE_HDMI21

#else
//------------------------------------------------------------------
// HDMI 2.1 Control
//------------------------------------------------------------------
#define lib_hdmi_is_hdmi_21_available(x)         (0)
#define lib_hdmi_hd21_clk_div_en(nport, enable)
#define lib_hdmi_hd21_is_clk_div_en(nport)    (0)
#define lib_hdmi_hd21_timing_gen_reset_toggle(nport)
#define lib_hdmi_hd21_get_timing_gen_status(nport)    (0xFFFFFFFF)
#define lib_hdmi_hd21_meas_clear_interlace_reg(nport)
#define lib_hdmi_hd21_meas_get_interlace(...)    0
#define lib_hdmi_hd21_fec_measure_enable(...)
#define lib_hdmi_hd21_fec_measure_restart(...)
#define lib_hdmi_hd21_clr_fec_frl_vs(port)
#define lib_hdmi_hd21_mac_crt_common_reset(port)
#define lib_hdmi_hd21_vsync_checked(port)    0


//------------------------------------------------------------------
// Error Status
//------------------------------------------------------------------
#define lib_hdmi_hd21_fw_disparity_error_reset(nport)
#define lib_hdmi_hd21_fw_symbol_error_reset(nport)
#define lib_hdmi_hd21_fw_disparity_symbol_error_enable(nport)
#define lib_hdmi_hd21_fw_disparity_symbol_error_disable(nport)
#define lib_hdmi_hd21_fw_disparity_error_status(...)
#define lib_hdmi_hd21_fw_symbol_error_status(...)
#define lib_hdmi_hd21_get_rs_err_cnt(nport)    (0)

//------------------------------------------------------------------
// High Level API
//------------------------------------------------------------------
#define newbase_hdmi_hd21_init_fw_vs_polarity(port)
#define newbase_hdmi_hd21_check_data_align(...)  (0)
#define newbase_hdmi_hd21_mac_enable(port)
#define newbase_hdmi_hd21_mac_disable(port)
#define newbase_hdmi_hd21_is_data_align_lock(port)   (FALSE)
#define newbase_hdmi_hd21_get_frl_lane(frl_mode)     (3)  // 3LANE
#define newbase_hdmi_hd21_get_frl_rate(frl_mode)    (0)  // TMDS_MODE
#define newbase_hdmi_hd21_link_training_fsm(port)
#define newbase_hdmi_hd21_lt_fsm_status_init(port)
#define _hdmi_hd21_frl_lt_fsm_str(fsm)    "HDMI2.1_NOT_SUPPORT"
#define newbase_hdmi_hd21_clk_gen(...)
#define newbase_hdmi_hd21_check_rsed_upd_flag(port)
#define newbase_hdmi_hd21_is_timing_change(...)    0


#define SET_FRL_LT_FSM(port, fsm)       (0)
#define GET_FRL_LT_FSM(port)            (0)
#endif

#endif  // __HDMI_MAC_2P1_H__

