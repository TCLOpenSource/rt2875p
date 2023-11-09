#ifndef _FBLEVELCTRL_DEF_H_
#define _FBLEVELCTRL_DEF_H_


#ifdef __cplusplus
extern "C" {
#endif

#define _FB_LEVEL_MAX_   0xff

typedef struct
{
	unsigned char  u1_FBLevelCtrl_en;
	unsigned char  u1_BadRegionCtrl_en;
	unsigned char  u1_FBLevelShow_en;
	unsigned char  u1_FbLvl_occlCtrl_en;
	unsigned char  u1_FbLvl_gmvCtrl_en;
	unsigned char  u1_FbLvl_disableSCclr_en;

	unsigned char  u8_FbLvl_dSCclr_d2en_th;
	unsigned char  u8_FbLvl_dSCclr_en2d_th;
	unsigned char  u8_FbLvl_dSCclr_holdCnt;

	unsigned char  u1_Fblvl_Dh_tcTh_mapEn;
	unsigned char  u8_FbLvl_Dh_tc_thLgain;
	unsigned char  u8_FbLvl_Dh_tc_thHgain;
	unsigned char  u8_DehaloFBlvl_iir;

	unsigned char  u1_Fblvl_blur_ctrlEn;
	unsigned char  u8_FbLvl_blur_tc_thLgain;
	unsigned char  u8_FbLvl_blur_tc_thHgain;
	unsigned char  u8_FbLvl_blur_iir;
	unsigned short u12_FbLvl_blur_gmvUnconf_th0;
	unsigned short u12_FbLvl_blur_gmvUnconf_th1;
	unsigned char  u5_FbLvl_blur_badscene_dftLvl;

	unsigned char  u1_Fblvl_judder_ctrlEn;
	unsigned char  u8_FbLvl_judder_tc_thLgain;
	unsigned char  u8_FbLvl_judder_tc_thHgain;
	unsigned char  u8_FbLvl_judder_iir;


	unsigned int u27_Fb_tc_th_h;
	unsigned int u27_Fb_tc_th_l;
	unsigned char  u8_FBTempConsisIIR_up;
	unsigned char  u8_FBTempConsis_IIR_down;
	unsigned char  u8_FBTempConsis_limit_gain;
	unsigned int u32_FBTempConsis_limit_val;
	unsigned short u12_FBLevelIIRUp;
	unsigned short u12_FBLevelIIRDown;

	unsigned char  u8_SC_Gain;
	unsigned char  u8_SC_slope;
	unsigned char  u8_SC_HoldCnt;
	unsigned char  u8_SC_NormCnt;

	unsigned char  u8_badRgn_sad_iirA_up;
	unsigned char  u8_badRgn_sad_iirA_down;
	unsigned int u23_BadRegionSAD_th;
	unsigned int u23_BadRegionSAD_th_m;
	unsigned int u23_BadRegionSAD_th_h;
	unsigned char  u8_badRgn_center_gain;
	unsigned char  u8_badRgn_center_oft;
	unsigned char  u5_BadRegionGain[32];
	unsigned char  u8_BadRegionGainIIR;
	unsigned char  u8_badRgn_dtl_thr1;
	unsigned char  u8_badRgn_dtl_thr2;
	unsigned char  u8_badRgn_dtl_gain1;
	unsigned char  u8_badRgn_dtl_gain2;
	unsigned char  u1_gmvratio_use;
	unsigned char  u7_gmv_ratio_thH;
	unsigned char  u7_gmv_ratio_thL;
	unsigned char  u6_gmv_gain_max;
	unsigned char  u6_gmv_gain_min;
	//unsigned char  u6_gmv_similar_cnt;
	unsigned char  u8_gmv_gain_iir;
	unsigned short u12_gmv_rng_cnt_th;
	unsigned short u12_gmv_unconf_cnt_th;

	unsigned char  u8_FBLevelMapY1;
	unsigned char  u8_FBLevelMapY2;
	unsigned char  u8_FBLevelMapX1;
	unsigned char  u8_FBLevelMapX2;

	unsigned char  u1_FbLvl_disIIR_en;
	unsigned char  u8_FbLvl_disIIR_holdCnt;
	unsigned char  u1_scAnalysis_en;
	unsigned char  u4_scAnalysis_frmCnt_th;
	unsigned char  u4_scAnalysis_badFrmCnt_th;

	unsigned int u28_scAnalysis_sadAll_th0;
	unsigned int u28_scAnalysis_sadAll_th1;
	unsigned int u27_scAnalysis_TC_th0;
	unsigned int u27_scAnalysis_TC_th1;
	unsigned int u27_scAnalysis_static_motionTh;
	unsigned char  u8_scAnalysis_st2mo_stPeriodTh;

	unsigned short u16_minusDiff_penalty;

	unsigned char  u8_sc_bypass_cnt;
	unsigned char  u8_static_cnt_th;
	unsigned int u32_static_mot_th;

	unsigned int u20_good_rgn_sad_th;
	unsigned short u12_good_rgn_gmv_unconf_th;
	unsigned char  u8_fblvl_map_y2_good;
	unsigned short u12_fblvl_iir_up_good;
	unsigned char  u8_good_rgn_num_th1;
	unsigned char  u8_good_rgn_num_th2;
	unsigned char  u4_good_scene_hold_cnt;

	unsigned char  u1_FB_pan_apply_en;
	unsigned char  u1_FB_cross_apply_en;
}_PARAM_FBLevelCtrl;

typedef struct
{
	unsigned int u27_TempConsis_Th_h;
	unsigned int u27_TempConsis_Th_l;
	unsigned int u27_SC_TC_Th_h;
	unsigned int u27_SC_TC_Th_l;
	unsigned char  u8_SCHoldCnt;
	unsigned char  u8_SCNormCnt;
	unsigned int u27_FBTempConsis;
	unsigned int u27_FBTempConsisPre;
	unsigned int u27_FBTempConsisPre_raw;

	unsigned int  u25_badRgn_iirRgnSad[32];
	unsigned char  u6_BadRegionCnt;
	unsigned char  u5_BadRegionGain;
	unsigned short  u9_BadRegionGainPre;

	unsigned char  u4_gmv_gain;
	unsigned char  u4_gmv_gain_pre;

	unsigned char  u8_Dehalo_FBLevel;
	unsigned short u16_judder_lvl;
	unsigned short u16_blur_lvl;
	unsigned char  u1_deBlur_gmvU_isGood;


	unsigned char  u8_FBLevel;
	unsigned char  u8_FBSystem;
	unsigned int u8_FBResult;
	unsigned int u20_FBPreResult;

	unsigned char  u1_FbLvl_dSCclr;
	unsigned char  u8_FbLvl_dSCclr_holdCnt;


	unsigned char  u1_HW_sc_true;
	unsigned char  u1_SW_sc_true;
	unsigned char  u8_HW_sc_badFrm_cnt;
	unsigned char  u8_HW_sc_Frm_cnt;
	unsigned char  u1_preFrm_isBadScene;

	unsigned char  u1_swSC_static2move_true;
	unsigned char  u8_swSC_static_frmCnt;
	unsigned char  u8_swSC_disIIR_holdCnt;

	unsigned char  u1_badRgn_rgnSAD_noIIR;
	unsigned char  u1_fbLvl_tc_noIIR;
	unsigned char  u1_fbLvl_result_noIIR;

	unsigned char u3_FbLvl_dSCclr_wrt;

	unsigned char   u1_sc_signal;
	unsigned char   u8_sc_bypass_cnt;
	unsigned int  u8_mc_phase_pre;
	unsigned char   u8_static_cnt;

	unsigned char	u1_good_scene;
	unsigned char	u8_good_scene_cnt;
	unsigned char	u1_bad_scene;
	unsigned char	u8_fblvl_map_y2;
	unsigned short	u12_fblvl_iir_up;
	unsigned char	u8_good_rgn_cnt;

	unsigned char	u1_panx_scene;
	unsigned char	u1_pany_scene;
	unsigned char	u8_pan_scene_hold_cnt;
	
	unsigned char	u8_cross_scene;
	
}_OUTPUT_FBLevelCtrl;

typedef struct
{
	//o------ regional fallback ------o
	unsigned char u2_rFB_alpha_mode;
	unsigned char u8_rFB_alpha_curve_gain;
	unsigned char u8_rFB_alpha_curve_th0;
	unsigned char u8_rFB_alpha_curve_slp;
	unsigned char u8_rFB_alpha_curve_min;
	unsigned char u8_rFB_alpha_curve_max;
	bool  u1_rFB_alpha_auto;
	bool  u1_rFB_level_auto;
	unsigned char u8_rFB_alpha_force_value;
	unsigned char u8_rFB_level_force_value;
	unsigned char u8_rFB_level_curve_gain;
	unsigned char u8_rFB_level_curve_th0;
	unsigned char u8_rFB_level_curve_slp;
	unsigned char u8_rFB_level_curve_min;
	unsigned char u8_rFB_level_curve_max;	
	unsigned char u1_rFB_alpha_by_dtl_en;	
	unsigned char u1_rFB_alpha_by_rMV_cnt_en;
	unsigned char u8_big_rMV_thl;
	bool  u1_rFB_big_rMV_en;
	bool  u1_rFB_big_rTC_en;
	unsigned char u8_th0;
	unsigned char u8_th1;
	unsigned char u8_th2;
	unsigned char u8_th3;
	bool  u1_rFB_panning_en;
	unsigned char u8_rFB_panning_unconf_thl;
	unsigned char u8_rFB_panning_cnt_thl;

	//o------ rmv blending ------o
	unsigned char  u2_rmv_alpha_mode;
	unsigned char  u8_rmv_alpha_curve_gain;
	unsigned char  u8_rmv_alpha_curve_th0;
	unsigned char  u8_rmv_alpha_curve_slp;	
	unsigned char  u8_rmv_alpha_curve_min;
	unsigned char  u8_rmv_alpha_curve_max;
	bool   u1_gmv_alpha_auto;
	unsigned char  u8_gmv_alpha_force_value;
	unsigned short u12_regional_unconf_thl;
	unsigned short u12_regional_cnt_thl;
	unsigned char  u8_regioonal_unconf_cnt_th;
	unsigned char  u8_oppo_dir_cnt_thl;
	unsigned char  u8_rmv_var_thl;
	unsigned char  u8_gmv_cur_gain;
	unsigned char  u8_gmv_cur_slp;

}_PARAM_bRMV_rFB_CTRL;

typedef struct
{
	unsigned short u10_fb_rgn00_mvy[32];
	unsigned short u11_fb_rgn00_mvx[32];
	unsigned char  u8_fb_rmv_alpha[32];
	unsigned char  u8_rFB_level[32];
	unsigned char  u8_rFB_alpha[32];
	bool   u1_rFB_panning;
	unsigned char u8_ES_flag;
	unsigned char u8_ES_439_ID_flag;
	unsigned char u8_ES_21_ID_flag;
	unsigned char u8_ES_380_ID_flag;
	unsigned char u8_ES_413_ID_flag;
	unsigned char u8_ES_421_ID_flag;
	unsigned char u8_ES_Rassia_ID_flag;
	unsigned char u8_ES_DEHALO_flag;
	unsigned int  u32_ES_GFB;
	unsigned char u8_ES_040_ID_flag;
	unsigned char u8_rFB_LFB_ctrl_level;
	unsigned char u8_ES_Test_0_flag;
	unsigned int  u32_ES_RFB_ID_show;
	unsigned char u8_ES_flag2;
}_OUTPUT_bRMV_rFB_CTRL;


VOID FBLevelCtrl_Init(_OUTPUT_FBLevelCtrl *pOutput);
VOID FBLevelCtrl_Proc(const _PARAM_FBLevelCtrl *pParam, _OUTPUT_FBLevelCtrl *pOutput);

VOID MC_RMV_blending(const _PARAM_bRMV_rFB_CTRL *pParam, _OUTPUT_bRMV_rFB_CTRL *pOutput);
VOID MC_ReginaolFB(const _PARAM_bRMV_rFB_CTRL *pParam, _OUTPUT_bRMV_rFB_CTRL *pOutput);


DEBUGMODULE_BEGIN(_PARAM_FBLevelCtrl,_OUTPUT_FBLevelCtrl)
	ADD_PARAM(unsigned char,  u1_FBLevelCtrl_en,       1)
	ADD_PARAM(unsigned char,  u1_BadRegionCtrl_en,           0)
	ADD_PARAM(unsigned char,  u1_FBLevelShow_en,             0)
	ADD_PARAM(unsigned char,  u1_FbLvl_occlCtrl_en,    0)
	ADD_PARAM(unsigned char,  u1_FbLvl_gmvCtrl_en,           0)
	ADD_PARAM(unsigned char,  u1_FbLvl_disableSCclr_en,     1)

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_dSCclr_d2en_th,   128, ">, disable 2 enable Th")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_dSCclr_en2d_th,   64, "<")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_dSCclr_holdCnt,    2, "extra hold time")

	ADD_DUMMY_PARAM()
    ADD_PARAM_DES(unsigned char,  u1_Fblvl_Dh_tcTh_mapEn,    1,  "0:ori_propose, 1:A-B th")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_Dh_tc_thLgain,     20,  "u2.6")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_Dh_tc_thHgain,     30,  "")
	ADD_PARAM_DES(unsigned char,  u8_DehaloFBlvl_iir,       100, "u0.8")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u1_Fblvl_blur_ctrlEn,              0, "1-enable")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_blur_tc_thLgain,         16, "u2.6")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_blur_tc_thHgain,         32, "")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_blur_iir,               200, "u0.8")
	ADD_PARAM_DES(unsigned short, u12_FbLvl_blur_gmvUnconf_th0,    110, "<=,  bad-to-good")
	ADD_PARAM_DES(unsigned short, u12_FbLvl_blur_gmvUnconf_th1,    100, ">=,  good to bad")
	ADD_PARAM_DES(unsigned char,  u5_FbLvl_blur_badscene_dftLvl,     8, "u1.4, if bad scene, FRC result would not be better than default.")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u1_Fblvl_judder_ctrlEn,            0, "1-enable")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_judder_tc_thLgain,       16, "u2.6")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_judder_tc_thHgain,       32, "")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_judder_iir,              64, "u0.8")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned int, u27_Fb_tc_th_h,                 680000,  "u28.0")
	ADD_PARAM_DES(unsigned int, u27_Fb_tc_th_l,                 380000,  "u28.0")
	ADD_PARAM_DES(unsigned char,  u8_FBTempConsisIIR_up,              40,  "u0.8")
	ADD_PARAM_DES(unsigned char,  u8_FBTempConsis_IIR_down,          120,  "u0.8")
	ADD_PARAM_DES(unsigned char,  u8_FBTempConsis_limit_gain,         32,  "u6.2")
	ADD_PARAM_DES(unsigned int,  u32_FBTempConsis_limit_val,    800000,  "u32")
	ADD_PARAM_DES(unsigned short, u12_FBLevelIIRUp,                  600,  "u0.12")
	ADD_PARAM_DES(unsigned short, u12_FBLevelIIRDown,               1000,  "u0.12")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u8_SC_Gain,                  3,  "if sc, th = para_th*gain>>slope")
	ADD_PARAM_DES(unsigned char,  u8_SC_slope,                 1,  "suppose gain > (1<<slope)")
	ADD_PARAM_DES(unsigned char,  u8_SC_HoldCnt,               2,  "in hold time, sc_th keep ini")
	ADD_PARAM_DES(unsigned char,  u8_SC_NormCnt,               2,  "after hold, in Norm, from ini back to para_th")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u8_badRgn_sad_iirA_up,        0, "u1.8")
	ADD_PARAM_DES(unsigned char,  u8_badRgn_sad_iirA_down,      0, "u1.8")
	ADD_PARAM_DES(unsigned int, u23_BadRegionSAD_th,     100000, "")
	ADD_PARAM_DES(unsigned int, u23_BadRegionSAD_th_m,   180000, "")
	ADD_PARAM_DES(unsigned int, u23_BadRegionSAD_th_h,  3000000, "")
	ADD_PARAM_DES(unsigned char,  u8_badRgn_center_gain,        0, "u4.4, for center 4x2 rgn, add extra cnt")
	ADD_PARAM_DES(unsigned char,  u8_badRgn_center_oft,         1, "total bad rgn cnt <= 31")

	ADD_DUMMY_PARAM()
    ADD_PARAM_ARRAY_DES(unsigned char, u5_BadRegionGain, 32, 0, "uX.4")
	ADD_PARAM_DES(unsigned char,  u8_BadRegionGainIIR,  160, "u0.8")

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,  u7_gmv_ratio_thH,             100)
	ADD_PARAM(unsigned char,  u7_gmv_ratio_thL,             60)
	ADD_PARAM_DES(unsigned char,  u6_gmv_gain_max,          32, "uX.4")
	ADD_PARAM(unsigned char,  u6_gmv_gain_min,              16)
	//ADD_PARAM(unsigned char,  u6_gmv_similar_cnt,           3)
	ADD_PARAM(unsigned char,  u8_gmv_gain_iir,              0)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,  u8_FBLevelMapY1,      22)
	ADD_PARAM(unsigned char,  u8_FBLevelMapY2,     255)
	ADD_PARAM(unsigned char,  u8_FBLevelMapX1,      3)
	ADD_PARAM(unsigned char,  u8_FBLevelMapX2 ,    255)

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u1_FbLvl_disIIR_en,              0, "if big fblvl, no fb result iir")
	ADD_PARAM_DES(unsigned char,  u8_FbLvl_disIIR_holdCnt,         0, "")
	ADD_PARAM_DES(unsigned char,  u1_scAnalysis_en,                0, "1-scene change in FB need sw analysis, (in FPGA, set 0)")
	ADD_PARAM_DES(unsigned char,  u4_scAnalysis_frmCnt_th,         3, "u4.0")
	ADD_PARAM_DES(unsigned char,  u4_scAnalysis_badFrmCnt_th,      4, "u4.0, should < frmCnt")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned int, u28_scAnalysis_sadAll_th0,       1, "u28.0")
	ADD_PARAM_DES(unsigned int, u28_scAnalysis_sadAll_th1,      16, "")
	ADD_PARAM_DES(unsigned int, u27_scAnalysis_TC_th0,           1, "u27.0")
	ADD_PARAM_DES(unsigned int, u27_scAnalysis_TC_th1,          16, "")
	ADD_PARAM_DES(unsigned int, u27_scAnalysis_static_motionTh,  1, "u27.0, <")
	ADD_PARAM_DES(unsigned char,  u8_scAnalysis_st2mo_stPeriodTh, 16, "u8.0, >")


	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned short, u16_minusDiff_penalty,     1, "can't be 0")


	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u8_sc_bypass_cnt,      1, "")
	ADD_PARAM_DES(unsigned char,  u8_static_cnt_th,      4, "")
	ADD_PARAM_DES(unsigned int, u32_static_mot_th,     2000, "")


	//////////////////////////////////////////////////////////////////////////
	ADD_OUTPUT(unsigned int, u27_TempConsis_Th_h)
	ADD_OUTPUT(unsigned int, u27_TempConsis_Th_l)
    ADD_OUTPUT(unsigned int, u27_SC_TC_Th_h)
	ADD_OUTPUT(unsigned int, u27_SC_TC_Th_l)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,  u8_SCHoldCnt)
	ADD_OUTPUT(unsigned char,  u8_SCNormCnt)
	ADD_OUTPUT(unsigned int, u27_FBTempConsis)
	ADD_OUTPUT(unsigned int, u27_FBTempConsisPre)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,  u6_BadRegionCnt)
	ADD_OUTPUT(unsigned char,  u5_BadRegionGain)
	ADD_OUTPUT(unsigned short,  u9_BadRegionGainPre)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,   u4_gmv_gain)
	ADD_OUTPUT(unsigned char,   u4_gmv_gain_pre)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,   u8_Dehalo_FBLevel)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,   u1_deBlur_gmvU_isGood)
	ADD_OUTPUT(unsigned short,  u9_blur_lvl)
	ADD_OUTPUT(unsigned short,  u9_judder_lvl)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,   u8_FBLevel)
	ADD_OUTPUT(unsigned int,  u8_FBResult)
	ADD_OUTPUT(unsigned int,  u20_FBPreResult)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,  u1_FbLvl_dSCclr)
	ADD_OUTPUT(unsigned char,  u8_FbLvl_dSCclr_holdCnt)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,  u3_FbLvl_dSCclr_wrt)
DEBUGMODULE_END


#ifdef __cplusplus
}
#endif



#endif
