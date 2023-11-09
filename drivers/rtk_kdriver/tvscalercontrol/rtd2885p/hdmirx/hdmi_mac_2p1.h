#ifndef __HDMI_MAC_2P1_H__
#define __HDMI_MAC_2P1_H__


/////////////////////////HDMI 2.1/////////////////////////////////
typedef enum {
    HDMI_3LANE= 3,
    HDMI_4LANE,
}HDMI_LANE_COUNT;
#if 0
typedef enum {
    MODE_TMDS,
    MODE_FRL_3G_3_LANE,
    MODE_FRL_6G_3_LANE,
    MODE_FRL_6G_4_LANE,
    MODE_FRL_8G_4_LANE,
    MODE_FRL_10G_4_LANE,
    MODE_FRL_12G_4_LANE
}HDMI_FRL_MODE_T;
#endif
typedef enum {
    init_state,
    comma_seq_search_state,
    comma_seq_lock_state
} data_align_state;

typedef enum {
    LT_FSM_LTS2_FLT_READY = 0,
    LT_FSM_LTS3_PREPARE_START,
    LT_FSM_LTS3_WAIT_FLT_UPDATE_CLEAR,
    LT_FSM_LTS3_PATTERN_TRAINING,
    LT_FSM_LTSF_FAIL,
    LT_FSM_LTSL_FFE_CHANGE,
    LT_FSM_LTSF_FRL_RATE_CHANGE,
    LT_FSM_LTSP_PASS,
    LT_FSM_LTSP_CHECK
} HDMI_FRL_LT_FSM_T;

typedef enum {
    LT_ERROR_NONE = 0,    // No error
    LT_ERROR_UP_FFE = 0x01,    // LT fail, up ffe
    LT_ERROR_DOWN_FRL_RATE = (0x01<<1),    // LT fail, down frl rate
    LT_ERROR_PT_RX_TIMEOUT = (0x01 <<2),    // Include of lock fail and a lot of error count
    LT_ERROR_FRL_RATE_OVER_SPEC = (0x01 <<3),    // Sink can not support the FRL rate of TX requirement
    LT_ERROR_NO_RETRAIN_OVER_MAX = (0x01 <<4),    // Pattern Training fail until over max TRY count, 
} HDMI_FRL_LT_ERROR_FLAG;

typedef enum 
{
    HDMI21_MEASURE_SETTING_WRONG = 0, //timing change, restart detect mode
    HDMI21_MEASURE_WAIT_V_SYNC, //no vsync, do nothing, keep before state
    HDMI21_MEASURE_SUCCESS, //measure ok and do nothing, keep before state 
}HDMI21_MEASURE_RESULT;


typedef struct
{
    unsigned char state;
    unsigned int  debounce_cnt;
    unsigned int timeout;
}lane_align_state;

typedef struct
{
    HDMI_FRL_LT_FSM_T frl_lt_fsm;
    HDMI_FRL_MODE_T current_frl_mode;
    unsigned char max_tx_ffe_level;
    unsigned char current_ffe_level[4];    
    unsigned int wait_flt_update_timeout_cnt;
    unsigned long frl_lt_check_lock_timeout;
    unsigned char ltp_pass;
    unsigned char ltp[4];
    unsigned int err_cnt[4];
    unsigned char tx_flt_no_train;
    unsigned char rx_flt_no_timeout;    // for rx debugging
    unsigned int lt_error_status; // LT_FSM_LTSF_FAIL || LT_FSM_LTSL_FFE_CHANGE || LT_FSM_LTSF_FRL_RATE_CHANGE
    unsigned int lt_fail_cnt;
    unsigned int data_align_fail_cnt;   
}FRL_LT_FSM_STATUS_T;

#ifdef CONFIG_RTK_KDRV_HDMI_ENABLE_HDMI21
//------------------------------------------------------------------
// HDMI 2.1 Control
//------------------------------------------------------------------
#define HDMI21_DATA_ALIGN_FAIL_RETRY_DEBOUNCE   3
#define HDMI21_DATA_ALIGN_FAIL_DO_RETRAIN_THD   7 // For UCD-452 device,

//------------------------------------------------------------------
// CRT
//------------------------------------------------------------------

extern void lib_hdmi_hd21_crt_on(void);
extern void lib_hdmi_hd21_mac_crt_common_reset(unsigned char nport);
extern void lib_hdmi_hd21_mac_crt_reset(unsigned char nport);

//------------------------------------------------------------------
// Common
//------------------------------------------------------------------

extern unsigned char lib_hdmi_is_hdmi_21_available(unsigned char nport);
extern void lib_hdmi_hd21_enable(unsigned char nport, unsigned char on);
extern unsigned char lib_hdmi_is_hdmi_21_enabled(unsigned char nport);
extern void lib_hdmi_hd21_set_lane_mode_fw(unsigned char nport, HDMI_FRL_MODE_T frl_mode);

//------------------------------------------------------------------
// Phy FIFO
//------------------------------------------------------------------
extern void lib_hdmi_hd21_phy_afifo_enable(unsigned char nport, unsigned char enable);

//------------------------------------------------------------------
// CH FIFO
//------------------------------------------------------------------
extern void lib_hdmi_hd21_afifo_enable(unsigned char nport, unsigned char enable);

//------------------------------------------------------------------
// Data Align
//------------------------------------------------------------------
extern void lib_hdmi_hd21_data_align_status(unsigned char nport, unsigned char lane);
extern void lib_hdmi_hd21_force_realign(unsigned char nport);

//------------------------------------------------------------------
// Timing Gen
//------------------------------------------------------------------
extern void lib_hdmi_hd21_timing_gen_enable(unsigned char nport,unsigned char enable);
extern void lib_hdmi_hd21_timing_gen_check_error(unsigned char nport,unsigned char frl_mode);
extern void lib_hdmi_hd21_clk_div_en(unsigned char nport, unsigned char enable);
extern unsigned int lib_hdmi_hd21_is_clk_div_en(unsigned char nport);
extern void lib_hdmi_hd21_timing_gen_reset_toggle(unsigned char nport);
extern unsigned int lib_hdmi_hd21_get_timing_gen_status(unsigned char nport);

//------------------------------------------------------------------
// Link Training
//------------------------------------------------------------------
extern void lib_hdmi_hd21_link_training_disable(unsigned char nport);
extern void lib_hdmi_hd21_link_training_enable(unsigned char nport, HDMI_FRL_MODE_T frl_mode, unsigned int timeout);
extern void lib_hdmi_hd21_link_training_start(unsigned char nport, unsigned char ltp[4]);
extern void lib_hdmi_hd21_link_training_status(unsigned char nport, unsigned int lock[4], unsigned int err_cnt[4]);

//------------------------------------------------------------------
// Error Status
//------------------------------------------------------------------
extern void lib_hdmi_hd21_fw_disparity_error_reset(unsigned char nport);
extern void lib_hdmi_hd21_fw_symbol_error_reset(unsigned char nport);
extern void lib_hdmi_hd21_fw_disparity_symbol_error_enable(unsigned char nport);
extern void lib_hdmi_hd21_fw_disparity_symbol_error_disable(unsigned char nport);
extern void lib_hdmi_hd21_fw_disparity_error_status(unsigned char nport, unsigned int disparity_err[4]);
extern void lib_hdmi_hd21_fw_symbol_error_status(unsigned char nport, unsigned int symbol_err[4]);
extern unsigned int lib_hdmi_hd21_get_rs_err_cnt(unsigned char nport);
extern void newbase_hdmi_restart_scdc_extend(unsigned char port);
extern void newbase_hdmi_force_stop_scdc_extend(unsigned char port);


//------------------------------------------------------------------
// FEC RS Control
//------------------------------------------------------------------
extern void lib_hdmi_hd21_fec_rs_decode_enable(unsigned char nport, unsigned char enable);
extern void lib_hdmi_hd21_fec_scdc_rs_cnt_enable(unsigned char nport, unsigned char enable);
extern void lib_hdmi_hd21_fec_rs_cnt_keep_enable(unsigned char nport, unsigned char enable);
extern void lib_hdmi_hd21_fec_set_fw_vs_polarity(unsigned char nport, unsigned char fw_mode, unsigned char vs_inv);
extern void lib_hdmi_hd21_clr_fec_frl_vs(unsigned char nport);


//------------------------------------------------------------------
// Measure
//------------------------------------------------------------------
extern void lib_hdmi_hd21_meas_clear_interlace_reg(unsigned char nport);
extern HDMI_INT_PRO_CHECK_RESULT lib_hdmi_hd21_meas_get_interlace(unsigned char nport);
extern unsigned char lib_hdmi_hd21_meas_get_vs_polarity(unsigned char nport);
extern void lib_hdmi_hd21_fec_measure_enable(unsigned char nport, unsigned char is_dsc_mode, unsigned char enable);
extern void lib_hdmi_hd21_fec_measure_restart(unsigned char nport, unsigned char is_dsc_mode);
extern unsigned char lib_hdmi_hd21_vsync_checked(unsigned char nport);

//------------------------------------------------------------------
// Clock Detection
//------------------------------------------------------------------
extern void lib_hdmi_hd21_init_clkdet_counter(unsigned char nport, unsigned char lane_sel);
extern unsigned char lib_hdmi_hd21_clkdet_get_clk_counter(unsigned char nport, unsigned int* get_clk);
extern void lib_hdmi_hd21_clkdet_set_pop_up(unsigned char nport);
extern void lib_hdmi_hd21_clkdet_reset_counter(unsigned char nport);

//------------------------------------------------------------------
// High Level API
//------------------------------------------------------------------
extern void newbase_hdmi_hd21_init_fw_vs_polarity(unsigned char port);
extern unsigned char newbase_hdmi_hd21_check_data_align(unsigned char nport, HDMI_FRL_MODE_T frl_mode, HDMI_LANE_COUNT lane_mode);
extern void newbase_hdmi_hd21_mac_enable(unsigned char port);
extern void newbase_hdmi_hd21_mac_disable(unsigned char port);
extern void newbase_hdmi_hd21_frl_rate_update(unsigned char port, HDMI_FRL_MODE_T frl_rate);
extern HDMI_LANE_COUNT newbase_hdmi_hd21_get_frl_lane(HDMI_FRL_MODE_T frl_mode);
extern unsigned char newbase_hdmi_hd21_get_frl_rate(HDMI_FRL_MODE_T frl_mode);
extern void newbase_hdmi_hd21_lt_fsm_status_init(unsigned char port);
extern unsigned char newbase_hdmi_hd21_is_data_align_lock(unsigned char port);
extern void newbase_hdmi_hd21_link_training_fsm(unsigned char port);
extern const char* _hdmi_hd21_frl_mode_str(HDMI_FRL_MODE_T frl_mode);
extern const char* _hdmi_hd21_frl_lt_fsm_str(unsigned int fsm);
extern void newbase_hdmi_hd21_clk_gen(unsigned char nport, unsigned char dsc_enable, unsigned char pixel_mode);
extern unsigned char newbase_hdmi_hd21_check_rsed_upd_flag(unsigned char port);
extern unsigned char newbase_hdmi_hd21_is_timing_change(unsigned char port, unsigned char is_dsc_mode, MEASURE_TIMING_T* last_timing);
extern void newbase_hdmi_hd21_sink_request_retrain(unsigned char port);

extern FRL_LT_FSM_STATUS_T lt_fsm_status[HDMI_PORT_TOTAL_NUM];
#define SET_FRL_LT_FSM(port, fsm)	{ if (lt_fsm_status[port].frl_lt_fsm!=fsm) { rtd_pr_hdmi_warn("[FSM] Port [%d], FRL_LT_FSM=%d(%s) -> %d(%s)\n", port, lt_fsm_status[port].frl_lt_fsm, _hdmi_hd21_frl_lt_fsm_str(lt_fsm_status[port].frl_lt_fsm), fsm, _hdmi_hd21_frl_lt_fsm_str(fsm)); lt_fsm_status[port].frl_lt_fsm= fsm;  }  }
#define GET_FRL_LT_FSM(port)		(lt_fsm_status[port].frl_lt_fsm)

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
#define lib_hdmi_hd21_fec_measure_enable(...);
#define lib_hdmi_hd21_fec_measure_restart(...)
#define lib_hdmi_hd21_clr_fec_frl_vs(port)
#define lib_hdmi_hd21_mac_crt_common_reset(port)
#define lib_hdmi_hd21_vsync_checked(port)    0
#define lib_hdmi_hd21_fec_rs_cnt_keep_enable(...)


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
#define newbase_hdmi_hd21_get_frl_lane(frl_mode)     (HDMI_3LANE)
#define newbase_hdmi_hd21_get_frl_rate(frl_mode)    (3)
#define newbase_hdmi_hd21_link_training_fsm(port)
#define newbase_hdmi_hd21_lt_fsm_status_init(port)
#define _hdmi_hd21_frl_mode_str(frl_mode)    "MODE_TMDS"
#define _hdmi_hd21_frl_lt_fsm_str(fsm)    "HDMI2.1_NOT_SUPPORT"
#define newbase_hdmi_hd21_clk_gen(...)
#define newbase_hdmi_hd21_check_rsed_upd_flag(port)
#define newbase_hdmi_hd21_is_timing_change(...)    0
extern FRL_LT_FSM_STATUS_T lt_fsm_status[HDMI_PORT_TOTAL_NUM];

#define SET_FRL_LT_FSM(port, fsm)       (0)
#define GET_FRL_LT_FSM(port)            (0)
#endif

#endif  // __HDMI_MAC_2P1_H__

