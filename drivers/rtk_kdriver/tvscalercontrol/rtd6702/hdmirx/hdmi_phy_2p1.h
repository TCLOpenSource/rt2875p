
#ifndef __HDMI_PHY_2P1_H_
#define __HDMI_PHY_2P1_H_

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//FROM Golden setting generator TMDS mode
typedef enum {
	TMDS_5_94G=0,
	TMDS_2_97G,
	TMDS_1_485G,
	TMDS_0_742G,
	TMDS_0_25G,
	TMDS_4_445G,
	TMDS_3_7125G,
	TMDS_2_2275G,
	TMDS_1_85625G,
	TMDS_1_11375G,
	TMDS_0_928125G,
	TMDS_0_54G,
	TMDS_0_405G,
	TMDS_0_3375G,
	TMDS_0_27G,
} TMDS_TIMING_PARAM_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t ACDR_CK_5_4;
	u_int8_t ACDR_CK_6_5;
	u_int8_t ACDR_CK_9_6;
	u_int8_t ACDR_CK_11_10;
	u_int8_t B_2_11;
	u_int8_t B_2_12;
	u_int8_t B_2_13;
	u_int8_t B_2_14;
	u_int8_t B_3_15;
	u_int8_t B_3_16;
	u_int8_t B_4_17;
	u_int8_t B_4_18;
	u_int8_t B_4_19;
	u_int8_t B_5_20;
	u_int8_t B_5_21;
	u_int8_t B_6_22;
	u_int8_t B_7_24;
	u_int8_t B_7_25;
	u_int8_t B_7_26;
	u_int8_t B_7_28;
	u_int8_t B_9_31;
	u_int8_t B_14_37;
	u_int8_t B_14_39;
	u_int8_t B_16_42;
	u_int8_t B_16_43;
	u_int8_t B_18_44;
	u_int8_t B_19_46;
	u_int8_t ACDR_B_11_49;
	u_int8_t G_2_52;
	u_int8_t G_2_53;
	u_int8_t G_2_54;
	u_int8_t G_2_55;
	u_int8_t G_3_56;
	u_int8_t G_3_57;
	u_int8_t G_4_58;
	u_int8_t G_4_59;
	u_int8_t G_4_60;
	u_int8_t G_5_61;
	u_int8_t G_5_62;
	u_int8_t G_6_63;
	u_int8_t G_7_65;
	u_int8_t G_7_66;
	u_int8_t G_7_67;
	u_int8_t G_7_69;
	u_int8_t G_9_72;
	u_int8_t G_9_73;
	u_int8_t G_9_74;
	u_int8_t G_14_78;
	u_int8_t G_14_80;
	u_int8_t G_16_83;
	u_int8_t G_16_84;
	u_int8_t G_18_85;
	u_int8_t G_19_87;
	u_int8_t ACDR_G_11_90;
	u_int8_t R_2_93;
	u_int8_t R_2_94;
	u_int8_t R_2_95;
	u_int8_t R_2_96;
	u_int8_t R_3_97;
	u_int8_t R_3_98;
	u_int8_t R_4_99;
	u_int8_t R_4_100;
	u_int8_t R_4_101;
	u_int8_t R_5_102;
	u_int8_t R_5_103;
	u_int8_t R_6_104;
	u_int8_t R_7_106;
	u_int8_t R_7_107;
	u_int8_t R_7_108;
	u_int8_t R_7_110;
	u_int8_t R_9_113;
	u_int8_t R_9_114;
	u_int8_t R_9_115;
	u_int8_t R_14_119;
	u_int8_t R_14_121;
	u_int8_t R_16_124;
	u_int8_t R_16_125;
	u_int8_t R_18_126;
	u_int8_t R_19_128;
	u_int8_t ACDR_R_11_131;
}APHY_Para_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t LEQ_INIT_B_4_0_62;
	u_int8_t TAP0_INIT_B_7_0_63;
	u_int8_t LEQ_INIT_G_4_0_83;
	u_int8_t TAP0_INIT_G_7_0_84;
	u_int8_t LEQ_INIT_R_4_0_104;
	u_int8_t TAP0_INIT_R_7_0_105;
	u_int8_t LEQ_INIT_CK_4_0_125;
	u_int8_t TAP0_INIT_CK_7_0_126;
}DFE_ini_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
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
	u_int8_t b_clk_div2_en_30;
	u_int8_t g_clk_div2_en_32;
	u_int8_t r_clk_div2_en_34;
	u_int8_t ck_acdr_pll_config_1_36;
	u_int8_t ck_acdr_pll_config_2_37;
	u_int8_t ck_acdr_pll_config_3_38;
	u_int8_t ck_acdr_cdr_config_1_40;
	u_int8_t ck_acdr_cdr_config_2_41;
	u_int8_t ck_acdr_cdr_config_3_42;
}DPHY_Para_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t B_7_163;
	u_int8_t G_7_177;
	u_int8_t R_7_191;
	u_int8_t B_13_297;
	u_int8_t G_13_299;
	u_int8_t R_13_301;
}Koffset_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t EQFE_EN_CK_26;
	u_int8_t DFE_ADAPT_EN_CK_6_0_30;
	u_int8_t DFE_ADAPT_EN_CK_6_0_31;
	u_int8_t EQFE_EN_B_46;
	u_int8_t DFE_ADAPT_EN_B_6_0_50;
	u_int8_t DFE_ADAPT_EN_B_6_0_51;
	u_int8_t EQFE_EN_G_66;
	u_int8_t DFE_ADAPT_EN_G_6_0_70;
	u_int8_t DFE_ADAPT_EN_G_6_0_71;
	u_int8_t EQFE_EN_R_86;
	u_int8_t DFE_ADAPT_EN_R_6_0_90;
	u_int8_t DFE_ADAPT_EN_R_6_0_91;
}LEQ_VTH_Tap0_3_4_Adapt_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t DFE_ADAPT_EN_CK_6_0_9;
	u_int8_t DFE_ADAPT_EN_CK_6_0_10;
	u_int8_t DFE_ADAPT_EN_CK_6_0_11;
	u_int8_t DFE_ADAPT_EN_B_6_0_17;
	u_int8_t DFE_ADAPT_EN_B_6_0_18;
	u_int8_t DFE_ADAPT_EN_B_6_0_19;
	u_int8_t DFE_ADAPT_EN_G_6_0_25;
	u_int8_t DFE_ADAPT_EN_G_6_0_26;
	u_int8_t DFE_ADAPT_EN_G_6_0_27;
	u_int8_t DFE_ADAPT_EN_R_6_0_33;
	u_int8_t DFE_ADAPT_EN_R_6_0_34;
	u_int8_t DFE_ADAPT_EN_R_6_0_35;
}Tap0_to_Tap4_Adapt_tmds_1_T;


void TMDS_Merged_V1p2_20220420_Main_Seq(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);


void DFE_ini_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void DPHY_Fix_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void DPHY_Para_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void APHY_Fix_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void APHY_Para_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void DPHY_Init_Flow_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void APHY_Init_Flow_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void DCDR_settings_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void DCDR_settings_tmds_2_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void Koffset_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void LEQ_VTH_Tap0_3_4_Adapt_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void Tap0_to_Tap4_Adapt_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void DCDR_settings_tmds_3_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
void Manual_DFE_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);
////////////////////////////////////////////////////////////////////////////////////////
//FROM Golden setting generator FRL mode

typedef enum {
	FRL_12G=0,
	FRL_10G,
	FRL_8G,
	FRL_6G,
	FRL_3G,
} FRL_TIMING_PARAM_T;


typedef struct {
	FRL_TIMING_PARAM_T frl_timing;
	u_int8_t ck_fine_tune_start_4;
}finetunestart_on_T;

typedef struct {
	FRL_TIMING_PARAM_T frl_timing;
	u_int8_t CK_2_2;
	u_int8_t CK_2_3;
	u_int8_t CK_2_4;
	u_int8_t CK_2_5;
	u_int8_t CK_3_6;
	u_int8_t CK_3_7;
	u_int8_t CK_4_8;
	u_int8_t CK_4_9;
	u_int8_t CK_4_10;
	u_int8_t CK_5_11;
	u_int8_t CK_5_12;
	u_int8_t CK_6_13;
	u_int8_t CK_7_16;
	u_int8_t CK_7_17;
	u_int8_t CK_7_19;
	u_int8_t CK_11_27;
	u_int8_t CK_14_28;
	u_int8_t CK_14_29;
	u_int8_t ACDR_CK_2_44;
	u_int8_t ACDR_CK_3_46;
	u_int8_t ACDR_CK_6_50;
	u_int8_t ACDR_CK_8_51;
	u_int8_t ACDR_CK_9_55;
	u_int8_t ACDR_CK_10_58;
	u_int8_t ACDR_CK_11_59;
	u_int8_t ACDR_CK_11_60;
	u_int8_t ACDR_CK_12_61;
	u_int8_t B_2_62;
	u_int8_t B_2_63;
	u_int8_t B_2_64;
	u_int8_t B_2_65;
	u_int8_t B_3_66;
	u_int8_t B_3_67;
	u_int8_t B_4_68;
	u_int8_t B_4_69;
	u_int8_t B_4_70;
	u_int8_t B_5_71;
	u_int8_t B_5_72;
	u_int8_t B_6_73;
	u_int8_t B_7_76;
	u_int8_t B_7_77;
	u_int8_t B_7_79;
	u_int8_t B_11_87;
	u_int8_t B_14_88;
	u_int8_t B_14_89;
	u_int8_t ACDR_B_2_104;
	u_int8_t ACDR_B_3_106;
	u_int8_t ACDR_B_6_110;
	u_int8_t ACDR_B_8_111;
	u_int8_t ACDR_B_9_115;
	u_int8_t ACDR_B_10_118;
	u_int8_t ACDR_B_11_119;
	u_int8_t ACDR_B_11_120;
	u_int8_t ACDR_B_12_121;
	u_int8_t G_2_122;
	u_int8_t G_2_123;
	u_int8_t G_2_124;
	u_int8_t G_2_125;
	u_int8_t G_3_126;
	u_int8_t G_3_127;
	u_int8_t G_4_128;
	u_int8_t G_4_129;
	u_int8_t G_4_130;
	u_int8_t G_5_131;
	u_int8_t G_5_132;
	u_int8_t G_6_133;
	u_int8_t G_7_136;
	u_int8_t G_7_137;
	u_int8_t G_7_139;
	u_int8_t G_11_147;
	u_int8_t G_14_148;
	u_int8_t G_14_149;
	u_int8_t ACDR_G_2_164;
	u_int8_t ACDR_G_3_166;
	u_int8_t ACDR_G_6_170;
	u_int8_t ACDR_G_8_171;
	u_int8_t ACDR_G_9_175;
	u_int8_t ACDR_G_10_178;
	u_int8_t ACDR_G_11_179;
	u_int8_t ACDR_G_11_180;
	u_int8_t ACDR_G_12_181;
	u_int8_t R_2_182;
	u_int8_t R_2_183;
	u_int8_t R_2_184;
	u_int8_t R_2_185;
	u_int8_t R_3_186;
	u_int8_t R_3_187;
	u_int8_t R_4_188;
	u_int8_t R_4_189;
	u_int8_t R_4_190;
	u_int8_t R_5_191;
	u_int8_t R_5_192;
	u_int8_t R_6_193;
	u_int8_t R_7_196;
	u_int8_t R_7_197;
	u_int8_t R_7_199;
	u_int8_t R_11_207;
	u_int8_t R_14_208;
	u_int8_t R_14_209;
	u_int8_t ACDR_R_2_224;
	u_int8_t ACDR_R_3_226;
	u_int8_t ACDR_R_6_230;
	u_int8_t ACDR_R_8_231;
	u_int8_t ACDR_R_9_235;
	u_int8_t ACDR_R_10_238;
	u_int8_t ACDR_R_11_239;
	u_int8_t ACDR_R_11_240;
	u_int8_t ACDR_R_12_241;
}APHY_Para_frl_1_T;

typedef struct {
	FRL_TIMING_PARAM_T frl_timing;
	u_int8_t VTHP_INIT_B_4_0_60;
	u_int8_t VTHN_INIT_B_4_0_61;
	u_int8_t VTHP_INIT_G_4_0_81;
	u_int8_t VTHN_INIT_G_4_0_82;
	u_int8_t VTHP_INIT_R_4_0_102;
	u_int8_t VTHN_INIT_R_4_0_103;
	u_int8_t VTHP_INIT_CK_4_0_123;
	u_int8_t VTHN_INIT_CK_4_0_124;
}DFE_ini_frl_1_T;

typedef struct {
	FRL_TIMING_PARAM_T frl_timing;
	u_int32_t b_cdr_cp_307;
	u_int32_t g_cdr_cp_350;
	u_int32_t r_cdr_cp_393;
	u_int32_t ck_cdr_cp_436;
}DPHY_Fix_frl_1_T;

typedef struct {
	FRL_TIMING_PARAM_T frl_timing;
	u_int16_t b_lock_dn_limit_10;
	u_int16_t b_lock_up_limit_11;
	u_int16_t b_coarse_lock_dn_limit_12;
	u_int16_t b_coarse_lock_up_limit_13;
	u_int16_t g_lock_dn_limit_16;
	u_int16_t g_lock_up_limit_17;
	u_int16_t g_coarse_lock_dn_limit_18;
	u_int16_t g_coarse_lock_up_limit_19;
	u_int16_t r_lock_dn_limit_22;
	u_int16_t r_lock_up_limit_23;
	u_int16_t r_coarse_lock_dn_limit_24;
	u_int16_t r_coarse_lock_up_limit_25;
	u_int16_t ck_lock_dn_limit_28;
	u_int16_t ck_lock_up_limit_29;
	u_int16_t ck_coarse_lock_dn_limit_30;
	u_int16_t ck_coarse_lock_up_limit_31;
}DPHY_Init_Flow_frl_1_T;

typedef struct {
	FRL_TIMING_PARAM_T frl_timing;
	u_int8_t b_rate_sel_2;
	u_int8_t g_rate_sel_3;
	u_int8_t r_rate_sel_4;
	u_int8_t ck_rate_sel_5;
}DPHY_Para_frl_1_T;

typedef struct {
	FRL_TIMING_PARAM_T frl_timing;
	u_int8_t B_7_212;
	u_int8_t G_7_225;
	u_int8_t R_7_238;
	u_int8_t CK_7_251;
	u_int8_t B_13_386;
	u_int8_t CK_13_388;
	u_int8_t G_13_390;
	u_int8_t R_13_392;
	u_int8_t ck_fine_tune_start_396;
}Koffset_frl_1_T;

typedef struct {
	FRL_TIMING_PARAM_T frl_timing;
	u_int8_t LEQ1_TRANS_MODE_CK_1_0_12;
	u_int8_t DFE_ADAPT_EN_CK_6_0_22;
	u_int8_t DFE_ADAPT_EN_CK_6_0_23;
	u_int8_t LEQ1_TRANS_MODE_B_1_0_32;
	u_int8_t DFE_ADAPT_EN_B_6_0_42;
	u_int8_t DFE_ADAPT_EN_B_6_0_43;
	u_int8_t LEQ1_TRANS_MODE_G_1_0_52;
	u_int8_t DFE_ADAPT_EN_G_6_0_62;
	u_int8_t DFE_ADAPT_EN_G_6_0_63;
	u_int8_t LEQ1_TRANS_MODE_R_1_0_72;
	u_int8_t DFE_ADAPT_EN_R_6_0_82;
	u_int8_t DFE_ADAPT_EN_R_6_0_83;
	u_int8_t ck_fine_tune_start_94;
}LEQ_VTH_Tap0_3_4_Adapt_frl_1_T;

typedef struct {
	FRL_TIMING_PARAM_T frl_timing;
	u_int8_t DFE_ADAPT_EN_CK_6_0_14;
	u_int8_t DFE_ADAPT_EN_CK_6_0_15;
	u_int8_t DFE_ADAPT_EN_B_6_0_22;
	u_int8_t DFE_ADAPT_EN_B_6_0_23;
	u_int8_t DFE_ADAPT_EN_G_6_0_30;
	u_int8_t DFE_ADAPT_EN_G_6_0_31;
	u_int8_t DFE_ADAPT_EN_R_6_0_38;
	u_int8_t DFE_ADAPT_EN_R_6_0_39;
	u_int8_t ck_fine_tune_start_44;
}Tap0_to_Tap4_Adapt_frl_1_T;



void FRL_Merged_V1_20220315_Main_Seq(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);



void DFE_ini_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void DPHY_Fix_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void DPHY_Para_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void APHY_Fix_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void APHY_Para_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void DPHY_Init_Flow_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void APHY_Init_Flow_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void ACDR_settings_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void ACDR_settings_frl_2_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void Koffset_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void LEQ_VTH_Tap0_3_4_Adapt_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void Tap0_to_Tap4_Adapt_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void ACDR_settings_frl_3_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);
void ACDR_settings_frl_4_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing);



/////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////
#define HD21_PORT_TOTAL_NUM	4

#define FRL_3G_B		3000
#define FRL_6G_B		6000
#define FRL_8G_B		8000
#define FRL_10G_B		10000
#define FRL_12G_B		12000

typedef enum {
	ACDR_MODE,
	DCDR_MODE,
}HDMIRX_CDR_MODE;


typedef enum{
	PS_NORMAL_MODE,
	PS_FAST_SWITCH_MODE,
}HDMIRX_PS_MODE;


extern void lib_hdmi21_phy_init(void);
extern void lib_hdmi21_z0_set(unsigned char port, unsigned char lane, unsigned char enable);
extern unsigned char newbase_rxphy_frl_measure(unsigned char port);
extern unsigned char newbase_rxphy_tmds_measure(unsigned char port);
extern void newbase_rxphy_frl_job(unsigned char p);
extern void newbase_rxphy_tmds_job(unsigned char p);
extern unsigned char newbase_hdmi_scdc_get_frl_mode(unsigned char port);
extern void newbase_rxphy_set_frl_mode(unsigned char port, unsigned char frl_mode);
extern unsigned char newbase_rxphy_get_frl_mode(unsigned char port);
extern void newbase_hdmi21_set_phy_frl_mode(unsigned port, unsigned char frl_mode);
extern void newbase_hdmi_frl_set_phy(unsigned char port, unsigned char frl_mode);
extern void newbase_hdmi21_phy_pw_saving(unsigned char port);
extern void lib_hdmi_hd21_ltp_phy_adaptive(unsigned char nport, unsigned char lane_num);
extern void lib_hdmi21_hysteresis_en(unsigned char en);
extern void lib_hdmi21_stb_tmds_clk_wakeup_en(unsigned char en);

#if HDMI_FRL_TRANS_DET
extern void lib_hdmi_lane_rate_detect_start(unsigned char nport);
extern void lib_hdmi_lane_rate_detect_stop(unsigned char nport);
extern int lib_hdmi_lane_rate_is_ready(unsigned char nport);
extern int lib_hdmi_get_lane_rate(unsigned char nport);
#endif
extern unsigned char PHY_DBG_MSG;

extern unsigned char factory_or_power_only_mode_en;
extern unsigned char factory_or_power_only_mode_bch_irq_en;
extern int factory_or_power_only_mode_hd21_eq_ofst;
extern unsigned int scdc_extend[4];
//extern  FRL_TIMING_PARAM_T_ORI FRL_Timing_last[4];
//extern  TMDS_TIMING_PARAM_T_ORI TMDS_Timing_last[4];


#endif  // __HDMI_PHY_H_

