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
	PHY_PROC_FW_LE_SCAN,

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
	unsigned char badcable_flag;	
	unsigned char le_re_adaptive;
	unsigned int bit_err_loop;
	unsigned int bit_err_occurred;
	unsigned int dfe_thr_chk_cnt;
	unsigned int ced_thr_chk_cnt;	
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

/////////////////////////////////////////////////////
//pure HDMI port
#if 1
typedef enum {
	TMDS_5p94G=0,
	TMDS_2p97G,
	TMDS_1p485G,
	TMDS_0p742G,
	TMDS_0p25G,
	TMDS_4p445G,
	TMDS_3p7125G,
	TMDS_2p2275G,
	TMDS_1p85625G,
	TMDS_1p11375G,
	TMDS_0p928125G,
	TMDS_0p54G,
	TMDS_0p405G,
	TMDS_0p3375G,
	TMDS_0p27G,
	TMDS_OLT_5p94G,
	TMDS_CP_5p94G,
}TMDS_VALUE_PARAM_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t CK_CKXTAL_SEL_24_1;
	u_int8_t CK_SEL_CKIN_25_1;
}APHY_Fix_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t CK_SEL_BAND_CAP_2_1;
	u_int8_t CK_POW_CP2_3_1;
	u_int8_t CK_SEL_ICP_4_1;
	u_int8_t CK_SEL_LPF_RES_5_1;
	u_int16_t CK_SEL_M_DIV_6_1;
	u_int8_t B_PI_ISEL_11_1;
	u_int8_t B_PI_ISEL_11_1_16;
	u_int8_t B_PI_ISEL_11_1_32;
	u_int8_t B_PI_SEL_DATARATE_12_1;
	u_int8_t B_PI_SEL_DATARATE_12_1_16;
	u_int8_t B_PI_SEL_DATARATE_12_1_32;
	u_int8_t B_DEMUX_DIV5_10_SEL_13_1;
	u_int8_t B_DEMUX_DIV5_10_SEL_13_1_16;
	u_int8_t B_DEMUX_DIV5_10_SEL_13_1_32;
	u_int8_t B_DEMUX_RATE_SEL_14_1;
	u_int8_t B_DEMUX_RATE_SEL_14_1_16;
	u_int8_t B_DEMUX_RATE_SEL_14_1_32;
	u_int8_t B_DFE_TAP_EN_15_1;
	u_int8_t B_DFE_TAP_EN_15_1_16;
	u_int8_t B_DFE_TAP_EN_15_1_32;
	u_int8_t B_DFE_TAP_EN_16_1;
	u_int8_t B_DFE_TAP_EN_16_1_16;
	u_int8_t B_DFE_TAP_EN_16_1_32;
	u_int8_t B_LE_IHALF_LE_18_1;
	u_int8_t B_LE_IHALF_LE_18_1_16;
	u_int8_t B_LE_IHALF_LE_18_1_32;
	u_int8_t B_LE_ISEL_LE1_21_1;
	u_int8_t B_LE_ISEL_LE1_21_1_16;
	u_int8_t B_LE_ISEL_LE1_21_1_32;
	u_int8_t B_LE_ISEL_LE2_22_1;
	u_int8_t B_LE_ISEL_LE2_22_1_16;
	u_int8_t B_LE_ISEL_LE2_22_1_32;
	u_int8_t B_LE_ISEL_NC_23_1;
	u_int8_t B_LE_ISEL_NC_23_1_16;
	u_int8_t B_LE_ISEL_NC_23_1_32;
	u_int8_t B_LE_ISEL_TAP0_24_1;
	u_int8_t B_LE_ISEL_TAP0_24_1_16;
	u_int8_t B_LE_ISEL_TAP0_24_1_32;
	u_int8_t B_LE_RLSEL_LE2_25_1;
	u_int8_t B_LE_RLSEL_LE2_25_1_16;
	u_int8_t B_LE_RLSEL_LE2_25_1_32;
	u_int8_t B_LE_RLSEL_LE11_26_1;
	u_int8_t B_LE_RLSEL_LE11_26_1_16;
	u_int8_t B_LE_RLSEL_LE11_26_1_32;
	u_int8_t B_LE_RLSEL_LE12_27_1;
	u_int8_t B_LE_RLSEL_LE12_27_1_16;
	u_int8_t B_LE_RLSEL_LE12_27_1_32;
	u_int8_t B_LE_RLSEL_NC1_28_1;
	u_int8_t B_LE_RLSEL_NC1_28_1_16;
	u_int8_t B_LE_RLSEL_NC1_28_1_32;
	u_int8_t B_LE_RLSEL_NC2_29_1;
	u_int8_t B_LE_RLSEL_NC2_29_1_16;
	u_int8_t B_LE_RLSEL_NC2_29_1_32;
	u_int8_t B_LE_RLSEL_TAP0_30_1;
	u_int8_t B_LE_RLSEL_TAP0_30_1_16;
	u_int8_t B_LE_RLSEL_TAP0_30_1_32;
	u_int8_t B_LE_RSSEL_LE2_31_1;
	u_int8_t B_LE_RSSEL_LE2_31_1_16;
	u_int8_t B_LE_RSSEL_LE2_31_1_32;
	u_int8_t B_LE_RSSEL_LE11_32_1;
	u_int8_t B_LE_RSSEL_LE11_32_1_16;
	u_int8_t B_LE_RSSEL_LE11_32_1_32;
	u_int8_t B_LE_RSSEL_LE12_33_1;
	u_int8_t B_LE_RSSEL_LE12_33_1_16;
	u_int8_t B_LE_RSSEL_LE12_33_1_32;
	u_int8_t B_LE_RSSEL_TAP0_34_1;
	u_int8_t B_LE_RSSEL_TAP0_34_1_16;
	u_int8_t B_LE_RSSEL_TAP0_34_1_32;
	u_int8_t B_POW_NC1_LEQ_38_1;
	u_int8_t B_POW_NC1_LEQ_38_1_16;
	u_int8_t B_POW_NC1_LEQ_38_1_32;
}APHY_Param_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t tap1_min_p2_6_1;
	u_int8_t tap0_init_lane0_p2_11_1;
	u_int8_t tap0_init_lane0_p2_11_1_16;
	u_int8_t tap0_init_lane0_p2_11_1_32;
	u_int8_t le_min_lane0_p2_12_1;
	u_int8_t le_min_lane0_p2_12_1_16;
	u_int8_t le_min_lane0_p2_12_1_32;
	u_int8_t le_init_lane0_p2_13_1;
	u_int8_t le_init_lane0_p2_13_1_16;
	u_int8_t le_init_lane0_p2_13_1_32;
	u_int8_t tap1_init_lane0_p2_14_1;
	u_int8_t tap1_init_lane0_p2_14_1_16;
	u_int8_t tap1_init_lane0_p2_14_1_32;
	u_int8_t adapt_mode_p2_20_1;
}DFE_ini_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t KP_2_1;
	u_int8_t KP_3_1;
	u_int8_t KI_4_1;
	u_int8_t RATE_SEL_5_1;
}DPHY_Param_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t B_DFE_ADAPT_EN_2_1;
	u_int8_t B_DFE_ADAPT_EN_2_1_16;
	u_int8_t B_DFE_ADAPT_EN_2_1_32;
	u_int8_t timer_ctrl_en_lane0_p2_3_1;
	u_int8_t timer_ctrl_en_lane0_p2_3_1_16;
	u_int8_t timer_ctrl_en_lane0_p2_3_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_4_1;
	u_int8_t dfe_adapt_en_lane0_p2_4_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_4_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_5_1;
	u_int8_t dfe_adapt_en_lane0_p2_5_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_5_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_6_1;
	u_int8_t dfe_adapt_en_lane0_p2_6_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_6_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_7_1;
	u_int8_t dfe_adapt_en_lane0_p2_7_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_7_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_8_1;
	u_int8_t dfe_adapt_en_lane0_p2_8_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_8_1_32;
}LEQ_TAP0_Adapt_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t tap1_min_p2_7_1;
	u_int8_t tap2_min_p2_9_1;
	u_int8_t vth_init_lane0_p2_10_1;
	u_int8_t vth_init_lane0_p2_10_1_16;
	u_int8_t vth_init_lane0_p2_10_1_32;
}Manual_DFE_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t B_DFE_ADAPT_EN_2_1;
	u_int8_t B_DFE_ADAPT_EN_2_1_16;
	u_int8_t B_DFE_ADAPT_EN_2_1_32;
	u_int8_t timer_ctrl_en_lane0_p2_3_1;
	u_int8_t timer_ctrl_en_lane0_p2_3_1_16;
	u_int8_t timer_ctrl_en_lane0_p2_3_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_4_1;
	u_int8_t dfe_adapt_en_lane0_p2_4_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_4_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_5_1;
	u_int8_t dfe_adapt_en_lane0_p2_5_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_5_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_6_1;
	u_int8_t dfe_adapt_en_lane0_p2_6_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_6_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_7_1;
	u_int8_t dfe_adapt_en_lane0_p2_7_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_7_1_32;
	u_int8_t dfe_adapt_en_lane0_p2_8_1;
	u_int8_t dfe_adapt_en_lane0_p2_8_1_16;
	u_int8_t dfe_adapt_en_lane0_p2_8_1_32;
}TAP0_2_Adapt_1_T;

void Mac8p_Pure_HDMI_TMDS_Main_Seq(unsigned char nport, TMDS_VALUE_PARAM_T value_param);

void DPHY_Fix_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DPHY_Param_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DPHY_Init_Flow_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void APHY_Fix_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void APHY_Param_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void APHY_Init_Flow_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DCDR_settings_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DFE_ini_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void Koffset_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DCDR_settings_2_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void LEQ_TAP0_Adapt_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void TAP0_2_Adapt_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void Manual_DFE_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);

#endif

///////////////////////////////////////////////////////

//HDMI_combo_DP_Port//////
#if 1

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t REG_TOP_IN_2_14;
	u_int8_t REG_TOP_IN_2_16;
	u_int8_t CK_10_42;
	u_int8_t CK_14_56;
	u_int8_t ACDR_CK_5_88;
	u_int8_t B_14_127;
	u_int8_t ACDR_B_10_173;
	u_int8_t G_14_200;
	u_int8_t G_15_201;
	u_int8_t ACDR_G_10_246;
	u_int8_t R_14_273;
	u_int8_t R_15_274;
	u_int8_t ACDR_R_10_319;
}APHY_Fix_tmds_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t CK_19_10;
	u_int8_t CK_19_11;
	u_int8_t ACDR_CK_3_19;
	u_int8_t ACDR_B_3_36;
	u_int8_t ACDR_G_3_56;
	u_int8_t ACDR_R_3_76;
	u_int8_t P0_CK_DEMUX_RSTB_96;
}APHY_Init_Flow_tmds_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t CK_14_2;
	u_int8_t CK_14_3;
	u_int8_t CK_18_5;
	u_int8_t CK_19_6;
	u_int8_t ACDR_B_3_8;
	u_int8_t ACDR_G_3_9;
	u_int8_t ACDR_R_3_10;
	u_int8_t ACDR_CK_3_11;
	u_int8_t ACDR_B_5_13;
	u_int8_t ACDR_G_5_14;
	u_int8_t ACDR_R_5_15;
	u_int8_t ACDR_B_6_17;
	u_int8_t ACDR_G_6_18;
	u_int8_t ACDR_R_6_19;
	u_int8_t ACDR_CK_9_20;
	u_int8_t ACDR_CK_10_23;
	u_int8_t ACDR_CK_11_24;
	u_int8_t B_2_25;
	u_int8_t B_2_26;
	u_int8_t B_2_27;
	u_int8_t B_2_28;
	u_int8_t B_3_29;
	u_int8_t B_3_30;
	u_int8_t B_4_31;
	u_int8_t B_4_32;
	u_int8_t B_4_33;
	u_int8_t B_5_34;
	u_int8_t B_5_35;
	u_int8_t B_6_36;
	u_int8_t B_7_38;
	u_int8_t B_7_39;
	u_int8_t B_7_40;
	u_int8_t B_7_42;
	u_int8_t B_9_45;
	u_int8_t B_14_51;
	u_int8_t B_14_53;
	u_int8_t B_16_56;
	u_int8_t B_16_57;
	u_int8_t B_18_58;
	u_int8_t B_19_60;
	u_int8_t ACDR_B_9_63;
	u_int8_t ACDR_B_10_65;
	u_int8_t ACDR_B_11_67;
	u_int8_t G_2_70;
	u_int8_t G_2_71;
	u_int8_t G_2_72;
	u_int8_t G_2_73;
	u_int8_t G_3_74;
	u_int8_t G_3_75;
	u_int8_t G_4_76;
	u_int8_t G_4_77;
	u_int8_t G_4_78;
	u_int8_t G_5_79;
	u_int8_t G_5_80;
	u_int8_t G_6_81;
	u_int8_t G_7_83;
	u_int8_t G_7_84;
	u_int8_t G_7_85;
	u_int8_t G_7_87;
	u_int8_t G_9_90;
	u_int8_t G_14_96;
	u_int8_t G_14_98;
	u_int8_t G_16_101;
	u_int8_t G_16_102;
	u_int8_t G_18_103;
	u_int8_t G_19_105;
	u_int8_t ACDR_G_9_108;
	u_int8_t ACDR_G_10_110;
	u_int8_t ACDR_G_11_112;
	u_int8_t R_2_115;
	u_int8_t R_2_116;
	u_int8_t R_2_117;
	u_int8_t R_2_118;
	u_int8_t R_3_119;
	u_int8_t R_3_120;
	u_int8_t R_4_121;
	u_int8_t R_4_122;
	u_int8_t R_4_123;
	u_int8_t R_5_124;
	u_int8_t R_5_125;
	u_int8_t R_6_126;
	u_int8_t R_7_128;
	u_int8_t R_7_129;
	u_int8_t R_7_130;
	u_int8_t R_7_132;
	u_int8_t R_9_135;
	u_int8_t R_14_141;
	u_int8_t R_14_143;
	u_int8_t R_16_146;
	u_int8_t R_16_147;
	u_int8_t R_18_148;
	u_int8_t R_19_150;
	u_int8_t ACDR_R_9_153;
	u_int8_t ACDR_R_10_155;
	u_int8_t ACDR_R_11_157;
}APHY_Para_tmds_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t LEQ_INIT_B_4_0_62;
	u_int8_t TAP0_INIT_B_7_0_63;
	u_int8_t LEQ_INIT_G_4_0_84;
	u_int8_t TAP0_INIT_G_7_0_85;
	u_int8_t LEQ_INIT_R_4_0_106;
	u_int8_t TAP0_INIT_R_7_0_107;
	u_int8_t LEQ_INIT_CK_4_0_128;
	u_int8_t TAP0_INIT_CK_7_0_129;
}DFE_ini_tmds_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t b_pi_m_mode_50;
	u_int8_t g_pi_m_mode_55;
	u_int8_t r_pi_m_mode_60;
	u_int8_t ck_pi_m_mode_65;
}DPHY_Fix_tmds_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t b_kp_2;
	u_int8_t g_kp_3;
	u_int8_t r_kp_4;
	u_int8_t ck_kp_5;
	u_int8_t b_bbw_kp_6;
	u_int8_t g_bbw_kp_7;
	u_int8_t r_bbw_kp_8;
	u_int8_t ck_bbw_kp_9;
	u_int8_t b_ki_10;
	u_int8_t g_ki_11;
	u_int8_t r_ki_12;
	u_int8_t ck_ki_13;
	u_int8_t b_rate_sel_26;
	u_int8_t g_rate_sel_27;
	u_int8_t r_rate_sel_28;
	u_int8_t b_clk_div2_en_29;
	u_int8_t g_clk_div2_en_30;
	u_int8_t r_clk_div2_en_31;
	u_int8_t b_acdr_pll_config_1_36;
	u_int8_t b_acdr_pll_config_2_37;
	u_int8_t b_acdr_pll_config_3_38;
	u_int8_t b_acdr_cdr_config_1_40;
	u_int8_t b_acdr_cdr_config_2_41;
	u_int8_t b_acdr_cdr_config_3_42;
	u_int8_t b_acdr_manual_config_1_44;
	u_int8_t b_acdr_manual_config_2_45;
	u_int8_t b_acdr_manual_config_3_46;
	u_int8_t g_acdr_pll_config_1_48;
	u_int8_t g_acdr_pll_config_2_49;
	u_int8_t g_acdr_pll_config_3_50;
	u_int8_t g_acdr_cdr_config_1_52;
	u_int8_t g_acdr_cdr_config_2_53;
	u_int8_t g_acdr_cdr_config_3_54;
	u_int8_t g_acdr_manual_config_1_56;
	u_int8_t g_acdr_manual_config_2_57;
	u_int8_t g_acdr_manual_config_3_58;
	u_int8_t r_acdr_pll_config_1_60;
	u_int8_t r_acdr_pll_config_2_61;
	u_int8_t r_acdr_pll_config_3_62;
	u_int8_t r_acdr_cdr_config_1_64;
	u_int8_t r_acdr_cdr_config_2_65;
	u_int8_t r_acdr_cdr_config_3_66;
	u_int8_t r_acdr_manual_config_1_68;
	u_int8_t r_acdr_manual_config_2_69;
	u_int8_t r_acdr_manual_config_3_70;
	u_int8_t ck_acdr_pll_config_1_72;
	u_int8_t ck_acdr_pll_config_2_73;
	u_int8_t ck_acdr_pll_config_3_74;
	u_int8_t ck_acdr_cdr_config_1_76;
	u_int8_t ck_acdr_cdr_config_2_77;
	u_int8_t ck_acdr_cdr_config_3_78;
	u_int8_t ck_acdr_manual_config_1_80;
	u_int8_t ck_acdr_manual_config_2_81;
	u_int8_t ck_acdr_manual_config_3_82;
}DPHY_Para_tmds_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t b_offset_rst_n_23;
	u_int8_t g_offset_rst_n_24;
	u_int8_t r_offset_rst_n_25;
	u_int8_t B_7_36;
	u_int8_t b_offset_en_de_54;
	u_int8_t b_offset_en_do_55;
	u_int8_t b_offset_en_ee_56;
	u_int8_t b_offset_en_eo_57;
	u_int8_t b_offset_en_opo_58;
	u_int8_t b_offset_en_ope_59;
	u_int8_t G_7_99;
	u_int8_t g_offset_en_de_117;
	u_int8_t g_offset_en_do_118;
	u_int8_t g_offset_en_ee_119;
	u_int8_t g_offset_en_eo_120;
	u_int8_t g_offset_en_opo_121;
	u_int8_t g_offset_en_ope_122;
	u_int8_t R_7_162;
	u_int8_t r_offset_en_de_180;
	u_int8_t r_offset_en_do_181;
	u_int8_t r_offset_en_ee_182;
	u_int8_t r_offset_en_eo_183;
	u_int8_t r_offset_en_opo_184;
	u_int8_t r_offset_en_ope_185;
	u_int8_t B_7_216;
	u_int8_t B_8_218;
	u_int8_t b_offset_en_eq_223;
	u_int8_t G_7_230;
	u_int8_t G_8_232;
	u_int8_t g_offset_en_eq_237;
	u_int8_t R_7_244;
	u_int8_t R_8_246;
	u_int8_t r_offset_en_eq_251;
	u_int8_t b_offset_en_de_259;
	u_int8_t b_offset_en_do_260;
	u_int8_t b_offset_en_ee_261;
	u_int8_t b_offset_en_eo_262;
	u_int8_t b_offset_en_opo_263;
	u_int8_t b_offset_en_ope_264;
	u_int8_t g_offset_en_de_282;
	u_int8_t g_offset_en_do_283;
	u_int8_t g_offset_en_ee_284;
	u_int8_t g_offset_en_eo_285;
	u_int8_t g_offset_en_opo_286;
	u_int8_t g_offset_en_ope_287;
	u_int8_t r_offset_en_de_305;
	u_int8_t r_offset_en_do_306;
	u_int8_t r_offset_en_ee_307;
	u_int8_t r_offset_en_eo_308;
	u_int8_t r_offset_en_opo_309;
	u_int8_t r_offset_en_ope_310;
	u_int8_t B_13_377;
	u_int8_t G_13_379;
	u_int8_t R_13_381;
}Koffset_tmds_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t EQFE_EN_CK_36;
	u_int8_t DFE_ADAPT_EN_CK_6_0_37;
	u_int8_t DFE_ADAPT_EN_CK_6_0_38;
	u_int8_t DFE_ADAPT_EN_CK_6_0_40;
	u_int8_t DFE_ADAPT_EN_CK_6_0_41;
	u_int8_t EQFE_EN_B_59;
	u_int8_t DFE_ADAPT_EN_B_6_0_60;
	u_int8_t DFE_ADAPT_EN_B_6_0_61;
	u_int8_t DFE_ADAPT_EN_B_6_0_63;
	u_int8_t DFE_ADAPT_EN_B_6_0_64;
	u_int8_t EQFE_EN_G_82;
	u_int8_t DFE_ADAPT_EN_G_6_0_83;
	u_int8_t DFE_ADAPT_EN_G_6_0_84;
	u_int8_t DFE_ADAPT_EN_G_6_0_86;
	u_int8_t DFE_ADAPT_EN_G_6_0_87;
	u_int8_t EQFE_EN_R_105;
	u_int8_t DFE_ADAPT_EN_R_6_0_106;
	u_int8_t DFE_ADAPT_EN_R_6_0_107;
	u_int8_t DFE_ADAPT_EN_R_6_0_109;
	u_int8_t DFE_ADAPT_EN_R_6_0_110;
}LEQ_VTH_Tap0_3_4_Adapt_tmds_1_T;

typedef struct {
	TMDS_VALUE_PARAM_T value_param;
	u_int8_t DFE_ADAPT_EN_CK_6_0_23;
	u_int8_t DFE_ADAPT_EN_CK_6_0_24;
	u_int8_t DFE_ADAPT_EN_CK_6_0_25;
	u_int8_t DFE_ADAPT_EN_CK_6_0_26;
	u_int8_t DFE_ADAPT_EN_CK_6_0_27;
	u_int8_t DFE_ADAPT_EN_B_6_0_32;
	u_int8_t DFE_ADAPT_EN_B_6_0_33;
	u_int8_t DFE_ADAPT_EN_B_6_0_34;
	u_int8_t DFE_ADAPT_EN_B_6_0_35;
	u_int8_t DFE_ADAPT_EN_B_6_0_36;
	u_int8_t DFE_ADAPT_EN_G_6_0_41;
	u_int8_t DFE_ADAPT_EN_G_6_0_42;
	u_int8_t DFE_ADAPT_EN_G_6_0_43;
	u_int8_t DFE_ADAPT_EN_G_6_0_44;
	u_int8_t DFE_ADAPT_EN_G_6_0_45;
	u_int8_t DFE_ADAPT_EN_R_6_0_50;
	u_int8_t DFE_ADAPT_EN_R_6_0_51;
	u_int8_t DFE_ADAPT_EN_R_6_0_52;
	u_int8_t DFE_ADAPT_EN_R_6_0_53;
	u_int8_t DFE_ADAPT_EN_R_6_0_54;
}Tap0_to_Tap4_Adapt_tmds_1_T;

void Mac8p_TMDS_setting_Main_Seq(unsigned char nport, TMDS_VALUE_PARAM_T value_param);

void DFE_ini_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DPHY_Fix_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DPHY_Para_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void APHY_Fix_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void APHY_Para_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DPHY_Init_Flow_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void APHY_Init_Flow_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DCDR_settings_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DCDR_settings_tmds_2_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void Koffset_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void LEQ_VTH_Tap0_3_4_Adapt_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void Tap0_to_Tap4_Adapt_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void DCDR_settings_tmds_3_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);
void Manual_DFE_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param);

#endif
///////////////////////////////////////////////////////////

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
extern unsigned char newbase_hdmi_get_badcable_flag(unsigned char port);
extern void newbase_hdmi_set_badcable_flag(unsigned char port, unsigned char enable);
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
extern void lib_hdmi_mac_release(unsigned char nport, unsigned char frl_mode);


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

#define MAX_LE                  24
#define IS_VALID_LE(x)          (x>=0 && x <= MAX_LE)


#define DFE_INFO(fmt, args...)              rtd_pr_hdmi_info("[DFE][INFO]"fmt, ##args)
#define DFE_WARN(fmt, args...)              rtd_pr_hdmi_warn("[DFE][WARN]"fmt, ##args)

//------------------------------------------------------------------
// HPD/DET
//------------------------------------------------------------------
extern void lib_hdmi_set_hdmi_hpd(unsigned char hpd_idx, unsigned char en);
extern unsigned char lib_hdmi_get_hdmi_hpd(unsigned char hpd_idx);
extern unsigned char lib_hdmi_get_hdmi_5v_det(unsigned char det_idx);

extern void newbase_set_qs_rxphy_status(unsigned char port);

extern int newbase_hdmi_dfe_fw_le_scan(unsigned char port);
#endif  // __HDMI_PHY_H_

