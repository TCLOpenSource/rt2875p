#ifndef __HDMI_PHY_API_H__
#define __HDMI_PHY_API_H__

#ifdef CONFIG_RTK_KDRV_HDMI_ENABLE_PHY
//For external use
#include "hdmi_phy.h"
#else
//For bring-up use
#define  PHY_PROC_DONE    0
#define  HDMI_14    0
#define  HDMI_20    0
#define  HDMI_21    0

#define LN_CK 0x08
#define LN_B   0x01
#define LN_G   0x02
#define LN_R   0x04
#define LN_ALL (LN_CK|LN_B|LN_G|LN_R)

#define newbase_rxphy_isr_enable(enable)
#define lib_hdmi_phy_init()
#define newbase_hdmi_ced_error_cnt_reset(port)
#define newbase_rxphy_isr_set(port, en)
#define newbase_rxphy_init_struct(port)
#define newbase_rxphy_is_tmds_mode(port)    0
#define newbase_hdmi_set_6G_long_cable_enable(port, enable)
#define newbase_rxphy_get_phy_proc_state(port)    0
#define newbase_rxphy_is_clk_stable(port)	 0
#define newbase_rxphy_get_clk(port) 	0
#define newbase_rxphy_get_setphy_done(port)    0
#define newbase_hdmi_open_err_detect(port)    0
#define newbase_hdmi_reset_thr_cnt(port)    0
#define newbase_rxphy_reset_setphy_proc(port)
#define newbase_hdmi_err_detect_stop(port)    0
#define newbase_hdmi_get_longcable_flag(port)    0
#define newbase_hdmi_get_err_recovery(port)    0
#define newbase_hdmi_dfe_recovery_hi_speed(port)
#define newbase_hdmi_is_bit_error_occured(port)    0
#define newbase_hdmi_dfe_recovery_mi_speed(port)
#define newbase_hdmi_dfe_recovery_midband_long_cable(port)
#define newbase_get_phy_scan_done(port)    0
#define newbase_rxphy_get_clk_pre(port)    0
#define lib_hdmi_z0_set(port, lane, enable)
#define newbase_hdmi_set_longcable_flag(port, enable)
#define newbase_hdmi_phy_pw_saving(port)

#define lib_hdmi_phy_enter_power_saving_mode(nport, mode,br_swap)
#define lib_hdmi_phy_exit_power_saving_mode(nport, mode,br_swap)

#define newbase_hdmi_bit_err_detection(port,r_err,g_err,b_err)    0
#define newbase_hdmi_char_err_detection(port,r_err,g_err,b_err)

#define lib_hdmi_phy_reset_power_saving_state(port)
#define lib_hdmi_phy_dump_pow_saving_reg(port)

#define newbase_set_qs_rxphy_status(port)
#define newbase_rxphy_force_clear_clk_st(port)
#define newbase_hdmi_set_eq_mode(nport, mode)
#define newbase_hdmi_manual_eq(nport,eq_ch0,eq_ch1,eq_ch2)
#define newbase_hdmi_manual_eq_ch(nport,ch,eq_ch)
#define newbase_hdmi_get_ced_error_cnt(port,ch)    0

#define newbase_hdmi_dfe_recovery_6g_long_cable(port)

#define lib_hdmi_get_hdmi_bypass_phy_ps_mode()    0
#define lib_hdmi_set_hdmi_bypass_phy_ps_mode(value)

#define newbase_rxphy_get_frl_mode(port)    0
#endif

#endif  //__HDMI_PHY_API_H__

