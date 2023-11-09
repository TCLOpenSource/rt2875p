
#ifndef __HDMI_PHY_2P1_H_
#define __HDMI_PHY_2P1_H_
//TMDS from  Merlin8


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
} TMDS_TIMING_PARAM_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t ACDR_CK_5_93_1;
}APHY_Fix_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t ACDR_B_5_7_1;
	u_int8_t ACDR_G_5_8_1;
	u_int8_t ACDR_R_5_9_1;
	u_int8_t ACDR_CK_6_10_1;
	u_int8_t ACDR_B_6_11_1;
	u_int8_t ACDR_G_6_12_1;
	u_int8_t ACDR_R_6_13_1;
	u_int8_t ACDR_CK_9_14_1;
	u_int8_t ACDR_CK_10_17_1;
	u_int8_t ACDR_CK_11_18_1;
	u_int8_t B_2_19_1;
	u_int8_t B_2_19_1_32;
	u_int8_t B_2_19_1_64;
	u_int8_t B_2_20_1;
	u_int8_t B_2_20_1_32;
	u_int8_t B_2_20_1_64;
	u_int8_t B_2_21_1;
	u_int8_t B_2_21_1_32;
	u_int8_t B_2_21_1_64;
	u_int8_t B_2_22_1;
	u_int8_t B_2_22_1_32;
	u_int8_t B_2_22_1_64;
	u_int8_t B_3_23_1;
	u_int8_t B_3_23_1_32;
	u_int8_t B_3_23_1_64;
	u_int8_t B_3_24_1;
	u_int8_t B_3_24_1_32;
	u_int8_t B_3_24_1_64;
	u_int8_t B_4_25_1;
	u_int8_t B_4_25_1_32;
	u_int8_t B_4_25_1_64;
	u_int8_t B_4_26_1;
	u_int8_t B_4_26_1_32;
	u_int8_t B_4_26_1_64;
	u_int8_t B_4_27_1;
	u_int8_t B_4_27_1_32;
	u_int8_t B_4_27_1_64;
	u_int8_t B_5_28_1;
	u_int8_t B_5_28_1_32;
	u_int8_t B_5_28_1_64;
	u_int8_t B_5_29_1;
	u_int8_t B_5_29_1_32;
	u_int8_t B_5_29_1_64;
	u_int8_t B_6_30_1;
	u_int8_t B_6_30_1_32;
	u_int8_t B_6_30_1_64;
	u_int8_t B_7_32_1;
	u_int8_t B_7_32_1_32;
	u_int8_t B_7_32_1_64;
	u_int8_t B_7_33_1;
	u_int8_t B_7_33_1_32;
	u_int8_t B_7_33_1_64;
	u_int8_t B_7_34_1;
	u_int8_t B_7_34_1_32;
	u_int8_t B_7_34_1_64;
	u_int8_t B_7_36_1;
	u_int8_t B_7_36_1_32;
	u_int8_t B_7_36_1_64;
	u_int8_t B_9_39_1;
	u_int8_t B_9_39_1_32;
	u_int8_t B_9_39_1_64;
	u_int8_t B_14_45_1;
	u_int8_t B_14_45_1_32;
	u_int8_t B_14_45_1_64;
	u_int8_t B_14_47_1;
	u_int8_t B_14_47_1_32;
	u_int8_t B_14_47_1_64;
	u_int8_t B_16_50_1;
	u_int8_t B_16_50_1_32;
	u_int8_t B_16_50_1_64;
	u_int8_t B_16_51_1;
	u_int8_t B_16_51_1_32;
	u_int8_t B_16_51_1_64;
	u_int8_t B_18_52_1;
	u_int8_t B_18_52_1_32;
	u_int8_t B_18_52_1_64;
	u_int8_t B_19_54_1;
	u_int8_t B_19_54_1_32;
	u_int8_t B_19_54_1_64;
	u_int8_t ACDR_B_9_57_1;
	u_int8_t ACDR_B_9_57_1_32;
	u_int8_t ACDR_B_9_57_1_64;
	u_int8_t ACDR_B_10_59_1;
	u_int8_t ACDR_B_10_59_1_32;
	u_int8_t ACDR_B_10_59_1_64;
	u_int8_t ACDR_B_11_61_1;
	u_int8_t ACDR_B_11_61_1_32;
	u_int8_t ACDR_B_11_61_1_64;
}APHY_Para_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t LEQ_INIT_B_4_0_62_1;
	u_int8_t TAP0_INIT_B_7_0_63_1;
	u_int8_t LEQ_INIT_G_4_0_84_1;
	u_int8_t TAP0_INIT_G_7_0_85_1;
	u_int8_t LEQ_INIT_R_4_0_106_1;
	u_int8_t TAP0_INIT_R_7_0_107_1;
	u_int8_t LEQ_INIT_CK_4_0_128_1;
	u_int8_t TAP0_INIT_CK_7_0_129_1;
}DFE_ini_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t b_kp_2_1;
	u_int8_t g_kp_3_1;
	u_int8_t r_kp_4_1;
	u_int8_t ck_kp_5_1;
	u_int8_t b_bbw_kp_6_1;
	u_int8_t g_bbw_kp_7_1;
	u_int8_t r_bbw_kp_8_1;
	u_int8_t ck_bbw_kp_9_1;
	u_int8_t b_ki_10_1;
	u_int8_t g_ki_11_1;
	u_int8_t r_ki_12_1;
	u_int8_t ck_ki_13_1;
	u_int8_t b_rate_sel_26_1;
	u_int8_t g_rate_sel_27_1;
	u_int8_t r_rate_sel_28_1;
	u_int8_t b_clk_div2_en_29_1;
	u_int8_t g_clk_div2_en_30_1;
	u_int8_t r_clk_div2_en_31_1;
	u_int8_t b_acdr_pll_config_1_36_1;
	u_int8_t b_acdr_pll_config_2_37_1;
	u_int8_t b_acdr_pll_config_3_38_1;
	u_int8_t b_acdr_cdr_config_1_40_1;
	u_int8_t b_acdr_cdr_config_2_41_1;
	u_int8_t b_acdr_cdr_config_3_42_1;
	u_int8_t b_acdr_manual_config_1_44_1;
	u_int8_t b_acdr_manual_config_2_45_1;
	u_int8_t b_acdr_manual_config_3_46_1;
	u_int8_t g_acdr_pll_config_1_48_1;
	u_int8_t g_acdr_pll_config_2_49_1;
	u_int8_t g_acdr_pll_config_3_50_1;
	u_int8_t g_acdr_cdr_config_1_52_1;
	u_int8_t g_acdr_cdr_config_2_53_1;
	u_int8_t g_acdr_cdr_config_3_54_1;
	u_int8_t g_acdr_manual_config_1_56_1;
	u_int8_t g_acdr_manual_config_2_57_1;
	u_int8_t g_acdr_manual_config_3_58_1;
	u_int8_t r_acdr_pll_config_1_60_1;
	u_int8_t r_acdr_pll_config_2_61_1;
	u_int8_t r_acdr_pll_config_3_62_1;
	u_int8_t r_acdr_cdr_config_1_64_1;
	u_int8_t r_acdr_cdr_config_2_65_1;
	u_int8_t r_acdr_cdr_config_3_66_1;
	u_int8_t r_acdr_manual_config_1_68_1;
	u_int8_t r_acdr_manual_config_2_69_1;
	u_int8_t r_acdr_manual_config_3_70_1;
	u_int8_t ck_acdr_pll_config_1_72_1;
	u_int8_t ck_acdr_pll_config_2_73_1;
	u_int8_t ck_acdr_pll_config_3_74_1;
	u_int8_t ck_acdr_cdr_config_1_76_1;
	u_int8_t ck_acdr_cdr_config_2_77_1;
	u_int8_t ck_acdr_cdr_config_3_78_1;
	u_int8_t ck_acdr_manual_config_1_80_1;
	u_int8_t ck_acdr_manual_config_2_81_1;
	u_int8_t ck_acdr_manual_config_3_82_1;
}DPHY_Para_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t B_7_42_1;
	u_int8_t B_7_42_1_32;
	u_int8_t B_7_42_1_64;
	u_int8_t B_7_140_1;
	u_int8_t B_7_140_1_32;
	u_int8_t B_7_140_1_64;
	u_int8_t B_13_251_1;
	u_int8_t G_13_253_1;
	u_int8_t R_13_255_1;
}Koffset_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t EQFE_EN_R_78_1;
	u_int8_t EQFE_EN_R_78_1_64;
	u_int8_t EQFE_EN_R_78_1_128;
	u_int8_t EQFE_EN_R_78_1_192;
	u_int8_t DFE_ADAPT_EN_R_6_0_80_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_80_1_64;
	u_int8_t DFE_ADAPT_EN_R_6_0_80_1_128;
	u_int8_t DFE_ADAPT_EN_R_6_0_80_1_192;
	u_int8_t DFE_ADAPT_EN_R_6_0_82_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_82_1_64;
	u_int8_t DFE_ADAPT_EN_R_6_0_82_1_128;
	u_int8_t DFE_ADAPT_EN_R_6_0_82_1_192;
	u_int8_t DFE_ADAPT_EN_R_6_0_83_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_83_1_64;
	u_int8_t DFE_ADAPT_EN_R_6_0_83_1_128;
	u_int8_t DFE_ADAPT_EN_R_6_0_83_1_192;
}LEQ_VTH_Tap0_3_4_Adapt_tmds_1_T;

typedef struct {
	TMDS_TIMING_PARAM_T tmds_timing;
	u_int8_t DFE_ADAPT_EN_R_6_0_29_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_29_1_64;
	u_int8_t DFE_ADAPT_EN_R_6_0_29_1_128;
	u_int8_t DFE_ADAPT_EN_R_6_0_29_1_192;
	u_int8_t DFE_ADAPT_EN_R_6_0_30_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_30_1_64;
	u_int8_t DFE_ADAPT_EN_R_6_0_30_1_128;
	u_int8_t DFE_ADAPT_EN_R_6_0_30_1_192;
	u_int8_t DFE_ADAPT_EN_R_6_0_31_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_31_1_64;
	u_int8_t DFE_ADAPT_EN_R_6_0_31_1_128;
	u_int8_t DFE_ADAPT_EN_R_6_0_31_1_192;
}Tap0_to_Tap4_Adapt_tmds_1_T;

void TMDS_Main_Seq(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing);

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
#endif

///////////////////////////////////////////////////////////


//From FRL Merlin8
////////////////////////////////////////////////////////////////////////////////////////
//FROM Golden setting generator FRL mode

#if 1
typedef enum {
	FRL_12G=0,
	FRL_10G,
	FRL_8G,
	FRL_6G,
	FRL_3G,
} FRL_VALUE_PARAM_T;

typedef struct {
	FRL_VALUE_PARAM_T frl_timing;
	u_int8_t ck_fine_tune_start_4_3;
}finetunestart_on_T;

typedef struct {
	FRL_VALUE_PARAM_T frl_timing;
	u_int8_t CK_2_2_1;
	u_int8_t CK_2_3_1;
	u_int8_t CK_2_4_1;
	u_int8_t CK_2_5_1;
	u_int8_t CK_3_6_1;
	u_int8_t CK_3_7_1;
	u_int8_t CK_4_8_1;
	u_int8_t CK_4_9_1;
	u_int8_t CK_4_10_1;
	u_int8_t CK_5_11_1;
	u_int8_t CK_5_12_1;
	u_int8_t CK_6_13_1;
	u_int8_t CK_7_16_1;
	u_int8_t CK_7_17_1;
	u_int8_t CK_7_19_1;
	u_int8_t CK_11_27_1;
	u_int8_t CK_14_28_1;
	u_int8_t CK_14_29_1;
	u_int8_t ACDR_CK_2_44_1;
	u_int8_t ACDR_CK_3_46_1;
	u_int8_t ACDR_CK_6_50_1;
	u_int8_t ACDR_CK_8_51_1;
	u_int8_t ACDR_CK_9_55_1;
	u_int8_t ACDR_CK_10_58_1;
	u_int8_t ACDR_CK_11_59_1;
	u_int8_t ACDR_CK_11_60_1;
	u_int8_t ACDR_CK_12_61_1;
	u_int8_t B_2_62_1;
	u_int8_t B_2_63_1;
	u_int8_t B_2_64_1;
	u_int8_t B_2_65_1;
	u_int8_t B_3_66_1;
	u_int8_t B_3_67_1;
	u_int8_t B_4_68_1;
	u_int8_t B_4_69_1;
	u_int8_t B_4_70_1;
	u_int8_t B_5_71_1;
	u_int8_t B_5_72_1;
	u_int8_t B_6_73_1;
	u_int8_t B_7_76_1;
	u_int8_t B_7_77_1;
	u_int8_t B_7_79_1;
	u_int8_t B_11_87_1;
	u_int8_t B_14_88_1;
	u_int8_t B_14_89_1;
	u_int8_t ACDR_B_2_104_1;
	u_int8_t ACDR_B_3_106_1;
	u_int8_t ACDR_B_6_110_1;
	u_int8_t ACDR_B_8_111_1;
	u_int8_t ACDR_B_9_115_1;
	u_int8_t ACDR_B_10_118_1;
	u_int8_t ACDR_B_11_119_1;
	u_int8_t ACDR_B_11_120_1;
	u_int8_t ACDR_B_12_121_1;
	u_int8_t G_2_122_1;
	u_int8_t G_2_123_1;
	u_int8_t G_2_124_1;
	u_int8_t G_2_125_1;
	u_int8_t G_3_126_1;
	u_int8_t G_3_127_1;
	u_int8_t G_4_128_1;
	u_int8_t G_4_129_1;
	u_int8_t G_4_130_1;
	u_int8_t G_5_131_1;
	u_int8_t G_5_132_1;
	u_int8_t G_6_133_1;
	u_int8_t G_7_136_1;
	u_int8_t G_7_137_1;
	u_int8_t G_7_139_1;
	u_int8_t G_11_147_1;
	u_int8_t G_14_148_1;
	u_int8_t G_14_149_1;
	u_int8_t ACDR_G_2_164_1;
	u_int8_t ACDR_G_3_166_1;
	u_int8_t ACDR_G_6_170_1;
	u_int8_t ACDR_G_8_171_1;
	u_int8_t ACDR_G_9_175_1;
	u_int8_t ACDR_G_10_178_1;
	u_int8_t ACDR_G_11_179_1;
	u_int8_t ACDR_G_11_180_1;
	u_int8_t ACDR_G_12_181_1;
	u_int8_t R_2_182_1;
	u_int8_t R_2_183_1;
	u_int8_t R_2_184_1;
	u_int8_t R_2_185_1;
	u_int8_t R_3_186_1;
	u_int8_t R_3_187_1;
	u_int8_t R_4_188_1;
	u_int8_t R_4_189_1;
	u_int8_t R_4_190_1;
	u_int8_t R_5_191_1;
	u_int8_t R_5_192_1;
	u_int8_t R_6_193_1;
	u_int8_t R_7_196_1;
	u_int8_t R_7_197_1;
	u_int8_t R_7_199_1;
	u_int8_t R_11_207_1;
	u_int8_t R_14_208_1;
	u_int8_t R_14_209_1;
	u_int8_t ACDR_R_2_224_1;
	u_int8_t ACDR_R_3_226_1;
	u_int8_t ACDR_R_6_230_1;
	u_int8_t ACDR_R_8_231_1;
	u_int8_t ACDR_R_9_235_1;
	u_int8_t ACDR_R_10_238_1;
	u_int8_t ACDR_R_11_239_1;
	u_int8_t ACDR_R_11_240_1;
	u_int8_t ACDR_R_12_241_1;
}APHY_Para_frl_1_T;

typedef struct {
	FRL_VALUE_PARAM_T frl_timing;
	u_int16_t b_lock_dn_limit_10_1;
	u_int16_t b_lock_up_limit_11_1;
	u_int16_t b_coarse_lock_dn_limit_12_1;
	u_int16_t b_coarse_lock_up_limit_13_1;
	u_int16_t g_lock_dn_limit_16_1;
	u_int16_t g_lock_up_limit_17_1;
	u_int16_t g_coarse_lock_dn_limit_18_1;
	u_int16_t g_coarse_lock_up_limit_19_1;
	u_int16_t r_lock_dn_limit_22_1;
	u_int16_t r_lock_up_limit_23_1;
	u_int16_t r_coarse_lock_dn_limit_24_1;
	u_int16_t r_coarse_lock_up_limit_25_1;
	u_int16_t ck_lock_dn_limit_28_1;
	u_int16_t ck_lock_up_limit_29_1;
	u_int16_t ck_coarse_lock_dn_limit_30_1;
	u_int16_t ck_coarse_lock_up_limit_31_1;
}DPHY_Init_Flow_frl_1_T;

typedef struct {
	FRL_VALUE_PARAM_T frl_timing;
	u_int32_t b_cdr_cp_2_1;
	u_int32_t g_cdr_cp_3_1;
	u_int32_t r_cdr_cp_4_1;
	u_int32_t ck_cdr_cp_5_1;
	u_int8_t b_rate_sel_6_1;
	u_int8_t g_rate_sel_7_1;
	u_int8_t r_rate_sel_8_1;
	u_int8_t ck_rate_sel_9_1;
}DPHY_Para_frl_1_T;

typedef struct {
	FRL_VALUE_PARAM_T frl_timing;
	u_int8_t B_7_284_1;
	u_int8_t G_7_291_1;
	u_int8_t R_7_298_1;
	u_int8_t CK_7_305_1;
	u_int8_t B_13_494_1;
	u_int8_t CK_13_496_1;
	u_int8_t G_13_498_1;
	u_int8_t R_13_500_1;
}Koffset_frl_1_T;

typedef struct {
	FRL_VALUE_PARAM_T frl_timing;
	u_int8_t LEQ1_TRANS_MODE_CK_1_0_25_1;
	u_int8_t DFE_ADAPT_EN_CK_6_0_36_1;
	u_int8_t DFE_ADAPT_EN_CK_6_0_37_1;
	u_int8_t LEQ1_TRANS_MODE_B_1_0_52_1;
	u_int8_t DFE_ADAPT_EN_B_6_0_63_1;
	u_int8_t DFE_ADAPT_EN_B_6_0_64_1;
	u_int8_t LEQ1_TRANS_MODE_G_1_0_79_1;
	u_int8_t DFE_ADAPT_EN_G_6_0_90_1;
	u_int8_t DFE_ADAPT_EN_G_6_0_91_1;
	u_int8_t LEQ1_TRANS_MODE_R_1_0_106_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_117_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_118_1;
	u_int8_t ck_fine_tune_start_142_1;
}LEQ_VTH_Tap0_3_4_Adapt_frl_1_T;

typedef struct {
	FRL_VALUE_PARAM_T frl_timing;
	u_int8_t DFE_ADAPT_EN_CK_6_0_34_1;
	u_int8_t DFE_ADAPT_EN_CK_6_0_35_1;
	u_int8_t DFE_ADAPT_EN_B_6_0_43_1;
	u_int8_t DFE_ADAPT_EN_B_6_0_44_1;
	u_int8_t DFE_ADAPT_EN_G_6_0_52_1;
	u_int8_t DFE_ADAPT_EN_G_6_0_53_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_61_1;
	u_int8_t DFE_ADAPT_EN_R_6_0_62_1;
	u_int8_t ck_fine_tune_start_68_1;
}Tap0_to_Tap4_Adapt_frl_1_T;

void FRL_Main_Seq(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);

void DFE_ini_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void DPHY_Fix_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void DPHY_Para_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void APHY_Fix_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void APHY_Para_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void DPHY_Init_Flow_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void APHY_Init_Flow_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void ACDR_settings_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void ACDR_settings_frl_2_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void Koffset_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void LEQ_VTH_Tap0_3_4_Adapt_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void Tap0_to_Tap4_Adapt_frl_1_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void ACDR_settings_frl_3_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
void ACDR_settings_frl_4_func(unsigned char nport, FRL_VALUE_PARAM_T frl_timing);
#endif


/////////////////////////////////////////////////////////////////////////////////////////


#define HD21_PORT_TOTAL_NUM	2

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

extern void lib_hdmi21_z0_calibration(void);
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

extern unsigned char factory_or_power_only_mode_en;
extern unsigned char factory_or_power_only_mode_bch_irq_en;
extern int factory_or_power_only_mode_hd21_eq_ofst;
extern unsigned int scdc_extend[4];

#endif  // __HDMI_PHY_H_

