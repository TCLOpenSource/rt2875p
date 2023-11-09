#ifndef __HDMI_PHY_H_
#define __HDMI_PHY_H_

//TMDS clock define
#define TMDS_6G		5688
#define TMDS_5G		4740
#define TMDS_4G		3792
#define TMDS_3G		2844
#define TMDS_1p5G	1422

typedef enum {
	PHY_PROC_INIT,
	PHY_PROC_RECORD,
	PHY_PROC_CLOSE_ADP,


	PHY_PROC_RECOVER_6G_LONGCABLE,
	PHY_PROC_RECOVER_6G_LONGCABLE_RECORD,
	PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP,

	PHY_PROC_RECOVER_MI,
	PHY_PROC_RECOVER_HI,
	PHY_PROC_RECOVER_MID_BAND_LONGCABLE,

	PHY_PROC_DONE,
} HDMIRX_PHY_PROC_T;

typedef struct {
	unsigned char vth;
	unsigned char tap0;
	unsigned char tap0max;
	unsigned char tap0min;
	signed char tap1;
	signed char le;//Fix Coverity Issue:314901
	unsigned char lemax;
	unsigned char lemin;
	signed char tap2;
	signed char tap3;
	signed char tap4;
}HDMIRX_PHY_DFE_T;

typedef struct {
	unsigned int clk;
	unsigned int clk_pre;
	unsigned int clk_debounce_count;
	unsigned int clk_unstable_count;
	unsigned int noclk_debounce_count;
	unsigned int clk_tolerance;
	unsigned char clk_stable;
	unsigned int apply_clk;
	unsigned char rxphy_40x;
	unsigned char lane_num;
	unsigned char frl_mode;  //0: TMDS mode 1: FRL mode
	unsigned char pre_frl_mode;
	unsigned char do_koffset;
	HDMIRX_PHY_PROC_T phy_proc_state;

	unsigned int char_err_loop;
	unsigned int char_err_occurred;
	unsigned char detect_6g_longcable_enable;
	unsigned char longcable_flag;
	unsigned int bit_err_loop;
	unsigned int bit_err_occurred;
	unsigned int dfe_thr_chk_cnt;
	unsigned int error_detect_count;
	unsigned char recovery;
	HDMIRX_PHY_DFE_T dfe_t[4];
	unsigned short char_error_cnt[4];// character error detection value(accumulated)

	//hdmi2.1
	unsigned char ltp_state;  //0: ltp state 1: video state
	//disp err
	unsigned char disp_start;
	unsigned int disp_max_cnt;
	//unsigned int disp_err_pre[4];
	//unsigned int disp_err_after[4];
	unsigned int disp_err0[4];
	unsigned int disp_err1[4];
	unsigned int disp_err2[4];
	unsigned int disp_BER1[4];
	unsigned int disp_BER2[4];
	unsigned int disp_BER2_temp[4];	
	unsigned char disp_adp_stage[4];
	
	//unsigned int disp_err_diff1[4];
	//unsigned int disp_err_diff2[4];
	unsigned int disp_err_zero_cnt[4];
	unsigned int disp_BER_zero_cnt[4];

	//unsigned short disp_err_t[4];
	//unsigned short err_occur_t1[4];
	//unsigned short err_occur_t2[4];
	//unsigned short err_occur_t3[4];
	unsigned int disp_timer_cnt[4];

	//unsigned char wrong_direction_cnt[4];
	unsigned char adp_stage[4];
	int tap1_adp_step[4];
	int tap2_adp_step[4];	
	int le_adp_step[4];
	unsigned int ltp_err_diff1[4];
	unsigned int ltp_err_diff2[4];

#if PHY_ADAPTIVE_BY_LTP_PAT_AFTER_LTP_PASS
	//ltp err
	unsigned char ltp_err_start[4];
	unsigned char ltp_err_end[4];
	unsigned int ltp_err_pre[4];
	unsigned int ltp_err_zero_cnt[4];
#endif


} HDMIRX_PHY_STRUCT_T;

extern void newbase_rxphy_isr_set(unsigned char port, unsigned char en);
extern void newbase_rxphy_init_struct(unsigned char port);
extern int  newbase_rxphy_isr(void);
extern void newbase_rxphy_isr_enable(char enable);   // use to enable / disable hdmi phy thread

extern void newbase_rxphy_force_clear_clk_st(unsigned char port);
extern unsigned int newbase_rxphy_get_clk(unsigned char port);
extern unsigned int newbase_rxphy_get_clk_pre(unsigned char port);
extern int newbase_hdmi_get_frl_clock(unsigned char port, unsigned char frl_mode);
extern unsigned char newbase_rxphy_is_clk_stable(unsigned char port);
extern void newbase_rxphy_set_apply_clock(unsigned char port,unsigned int b);
extern unsigned int newbase_rxphy_get_apply_clock(unsigned char port);
extern unsigned char newbase_rxphy_get_setphy_done(unsigned char port);
extern unsigned char newbase_rxphy_is_tmds_mode(unsigned char port);
extern void newbase_rxphy_reset_setphy_proc(unsigned char port);
extern unsigned char newbase_rxphy_get_frl_mode(unsigned char port);
extern unsigned char newbase_rxphy_get_phy_proc_state(unsigned char port);

extern void newbase_hdmi_dfe_recovery_6g_long_cable(unsigned char port);
extern void newbase_hdmi_dfe_recovery_midband_long_cable(unsigned char port);
extern void newbase_hdmi_dfe_recovery_mi_speed(unsigned char port);
extern void newbase_hdmi_dfe_recovery_hi_speed(unsigned char port);



extern void newbase_hdmi_open_err_detect(unsigned char port);
extern void newbase_hdmi_reset_thr_cnt(unsigned char port);
extern unsigned char newbase_hdmi_err_detect_stop(unsigned char port);
extern unsigned char newbase_hdmi_get_err_recovery(unsigned char port);
extern void newbase_hdmi_err_detect_add(unsigned char port);

extern unsigned char newbase_hdmi_get_longcable_flag(unsigned char port);
extern void newbase_hdmi_set_longcable_flag(unsigned char port, unsigned char enable);
extern unsigned char newbase_hdmi_detect_6g_longcable(unsigned char port,unsigned int r_err,unsigned int g_err,unsigned int b_err);
extern unsigned char newbase_hdmi_char_err_detection(unsigned char port,unsigned int *r_err, unsigned int *g_err, unsigned int *b_err);
extern unsigned char newbase_hdmi_bit_err_detection(unsigned char port, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err);
extern unsigned char newbase_hdmi_set_6G_long_cable_enable(unsigned char port, unsigned char enable);

extern void newbase_hdmi_ced_error_cnt_reset(unsigned char port);
extern void newbase_hdmi_ced_error_cnt_accumulated(unsigned char port, unsigned int *b_err, unsigned int *g_err, unsigned int *r_err);
extern unsigned short newbase_hdmi_get_ced_error_cnt(unsigned char port , unsigned char ch);
extern unsigned char newbase_hdmi_is_bit_error_occured(unsigned char port);

extern void newbase_hdmi_set_eq_mode(unsigned char nport, unsigned char mode);
extern void newbase_hdmi_manual_eq(unsigned char nport, unsigned char eq_ch0, unsigned char eq_ch1, unsigned char eq_ch2);
extern  void newbase_hdmi_manual_eq_ch(unsigned char nport,unsigned char ch,unsigned char eq_ch);
extern HDMIRX_PHY_STRUCT_T* newbase_rxphy_get_status(unsigned char port);
extern void newbase_hdmi_rxphy_handler(unsigned char port);


#ifdef CONFIG_POWER_SAVING_MODE
extern void newbase_hdmi_phy_pw_saving(unsigned char port);
#endif

#if BIST_PHY_SCAN
extern int newbase_get_phy_scan_done(unsigned char nport);
#endif

extern unsigned int lib_hdmi_get_clock(unsigned char nport);
extern void lib_hdmi_phy_init(void);
extern void lib_hdmi_z0_set(unsigned char port, unsigned char lane, unsigned char enable);
extern void lib_hdmi_z300_sel(unsigned char port, unsigned char mode);
extern void lib_hdmi_ck_md_ref_sel(unsigned char port);
extern void lib_hdmi_mac_release(unsigned char nport, unsigned char frl_mode);

// Power Saving
typedef enum {
    HDMI_14 = 0,
    HDMI_20,
    HDMI_21,
}HDMI_MODE;

extern unsigned int lib_hdmi_get_hdmi_bypass_phy_ps_mode(void);
extern void lib_hdmi_set_hdmi_bypass_phy_ps_mode(unsigned int);
extern void lib_hdmi_phy_reset_power_saving_state(unsigned char nport);
extern void lib_hdmi_phy_enter_power_saving_mode(unsigned char nport, HDMI_MODE mode, unsigned char br_swap);
extern void lib_hdmi_phy_exit_power_saving_mode(unsigned char nport, HDMI_MODE mode, unsigned char br_swap);
extern void lib_hdmi_phy_dump_pow_saving_reg(unsigned char nport);
extern void lib_hdmi_hysteresis_en(unsigned char en);
extern void newbase_hdmi_dump_dfe_para(unsigned char nport, unsigned char lane_mode);

extern HDMIRX_PHY_STRUCT_T phy_st[HDMI_PORT_TOTAL_NUM];
//self test
#if BIST_PHY_SCAN
void debug_hdmi_phy_scan(unsigned char nport);
#endif
void debug_hdmi_dfe_scan(unsigned char nport,unsigned char lane_mode);
void debug_hdmi_dump_msg(unsigned char nport);
// Add


#define LN_CK 0x08
#define LN_B   0x01
#define LN_G   0x02
#define LN_R   0x04
#define LN_ALL (LN_CK|LN_B|LN_G|LN_R)



#define DFE_INFO(fmt, args...)              rtd_pr_hdmi_info("[DFE][INFO]"fmt, ##args)
#define DFE_WARN(fmt, args...)              rtd_pr_hdmi_warn("[DFE][WARN]"fmt, ##args)

//------------------------------------------------------------------
// HPD/DET
//------------------------------------------------------------------
extern void lib_hdmi_set_hdmi_hpd(unsigned char hpd_idx, unsigned char en);
extern unsigned char lib_hdmi_get_hdmi_hpd(unsigned char hpd_idx);
extern unsigned char lib_hdmi_get_hdmi_5v_det(unsigned char det_idx);

extern void newbase_set_qs_rxphy_status(unsigned char port);

#endif  // __HDMI_PHY_H_

