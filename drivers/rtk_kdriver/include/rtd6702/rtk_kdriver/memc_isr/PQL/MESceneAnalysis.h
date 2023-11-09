#ifndef _ME_SCENE_ANALYSIS_DEF_H
#define _ME_SCENE_ANALYSIS_DEF_H
//#define PRINT_GMV
//#define BG_MV_GET

typedef struct
{
	unsigned char  u1_RepeatPattern_en;
	unsigned char  u1_Natural_HighFreq_en;
	unsigned char  u1_LightSword_en;
	unsigned char  u1_BigAPLDiff_en;

	// casino
	unsigned int u16_casino_rimLft_th;
	unsigned int u16_casino_rimRht_th;
	unsigned int u16_casino_rimUp_th;
	unsigned int u16_casino_rimDn_th;
	unsigned char  u8_casino_rim_delta;

	unsigned char  u1_EPG_en;

	unsigned char  u1_DSR_en;//DynamicSearchRange_detect, u1_DSR_en

	unsigned char u1_over_searchrange_en;
	unsigned char u1_force_panning;
	unsigned char u8_over_searchrange_thl;

	// occl extension
	unsigned char u1_occl_type2_enable;
	unsigned char u8_occl_ext_bg_mv_diff_thl;
	unsigned char u1_occl_ext_gmv_weight_mode;
	unsigned char u4_occl_ext_2nd_gmv_uconf_en_thl;
	unsigned char u4_occl_ext_2nd_gmv_uconf_dis_thl;
	unsigned char u1_occl_ext_force_protection_off;
	#if 0  // LG TV006 493813 temp mark to sync
	unsigned char u1_occl_ext_force_lvl_enable;
	#else
	unsigned char u1_occl_ext_force_lvl_en;
	#endif
	unsigned char u3_occl_ext_force_lvl;
	unsigned char u4_occl_ext_rgn_thl;
	#if 1  // LG TV006 493813 temp mark to sync
	unsigned char u1_occl_ext_avoidLogo_en;
	unsigned char u8_occl_ext_LogoCount_thr;
	#endif
	// post correction
	unsigned char u1_dh_postcorr_log_en;
	unsigned char u3_dh_postcorr_force_lvl;
	unsigned char u1_dh_postcorr_force_lvl_en;
	unsigned char u1_dh_postcorrt_dyn_calc_en;
	unsigned char u8_dh_postcorr_LogoCount_thr;

	unsigned char u8_dh_postcorr_GmvMvy_thr;
	unsigned char u8_dh_postcorr_y2x_ratio_x0;
	unsigned char u8_dh_postcorr_y2x_ratio_x1;
	unsigned char u8_dh_postcorr_y2x_ratio_y0;
	unsigned char u8_dh_postcorr_y2x_ratio_y1;

	// main object motion
	unsigned char u4_MainObjectMove_bgMv_thl;
	unsigned char u4_MainObjectMove_objBgMvDiff_thl;
	unsigned char u8_MainObjectMove_dtl_thl;
	unsigned char u8_MainObjectMove_thl_1;
	unsigned char u8_MainObjectMove_thl_2;
	unsigned char u8_MainObjectMove_1st_uconf_thr;
	unsigned char u8_MainObjectMove_2nd_uconf_thr;

	// ME median filter
	bool  b_relMV_panning_en;
	bool  b_prd_en;
	bool  b_logo_en;
	bool  b_show_mode;
	bool  b_pan_noFB_en;
	bool  b_multi_GMV_en;
	unsigned char u2_medFLT_sel;
	unsigned char u3_show_mode2;
	unsigned char u8_mvDiff_thl;
	unsigned char u8_rFBss_thl;
	unsigned char u8_rSCss_thl;
	unsigned char u8_big_relMV_cnt_thl;
	unsigned char u8_rChaos_cnt_thl;
	unsigned char u8_panning_gmv_cnt_thl;
	unsigned char u4_panning_gmv_unconf_thl;
	unsigned char u8_rPrd_thl;
	unsigned char u8_rPrd_cnt_thl;
	// ME median filter
	bool  b_medFLT_adpt_en;
	bool  b_pfvd_en;
	
	// repeat periodic pattern
	bool b_FixRepeat_en;

	// detect sport scene
	bool u1_detect_SportScene_en;
}_PARAM_ME_SCENE_ANALYSIS;

/*
	SMART INTEGRATION STATUS (SIS) for MEMC ver. by LearnRPG@20150911

	Collection and combination all those status by long time experience.
	We suggest in kernal API don't have below code
	(that's mean kernal API must independent operation)
	1. no global variable, must use function parameter be input.
	2. don't change global status in function, must use middle layer to update status.

*/

typedef struct _MemcStatus
{
/*
	unsigned int reg_vbuf_ph_1st_lfsr_mask_x;
	unsigned int reg_vbuf_ph_1st_lfsr_mask_y;
	unsigned int reg_vbuf_ph_2nd_lfsr_mask_x;
	unsigned int reg_vbuf_ph_2nd_lfsr_mask_y;

	unsigned int reg_vbuf_ip_1st_lfsr_mask_x;
	unsigned int reg_vbuf_ip_1st_lfsr_mask_y;
	unsigned int reg_vbuf_ip_2nd_lfsr_mask_x;
	unsigned int reg_vbuf_ip_2nd_lfsr_mask_y;

	unsigned int reg_vbuf_pi_1st_lfsr_mask_x;
	unsigned int reg_vbuf_pi_1st_lfsr_mask_y;
	unsigned int reg_vbuf_pi_2nd_lfsr_mask_x;
	unsigned int reg_vbuf_pi_2nd_lfsr_mask_y;

	unsigned int reg_metop_ss_ip_update0_penalty;
	unsigned int reg_metop_ss_ip_update1_penalty;
	unsigned int reg_metop_ss_pi_update0_penalty;
	unsigned int reg_metop_ss_pi_update1_penalty;
	unsigned int reg_metop_ss_ph_update0_penalty;
	unsigned int reg_metop_ss_ph_update1_penalty;

	unsigned int reg_metop_ds_ip_update0_penalty;
	unsigned int reg_metop_ds_ip_update1_penalty;
	unsigned int reg_metop_ds_pi_update0_penalty;
	unsigned int reg_metop_ds_pi_update1_penalty;
	unsigned int reg_metop_ds_ph_update0_penalty;
	unsigned int reg_metop_ds_ph_update1_penalty;

	unsigned char  u3_ip_1st_cand1_offsetx;
	unsigned char  u3_ip_1st_cand5_offsetx;
	unsigned char  u3_ip_1st_update1_offsetx;
	unsigned char  u3_pi_1st_cand1_offsetx;
	unsigned char  u3_pi_1st_cand5_offsetx;
	unsigned char  u3_pi_1st_update1_offsetx;
	unsigned char  u3_ph_1st_cand1_offsetx;
	unsigned char  u3_ph_1st_cand2_offsetx;
	unsigned char  u3_ph_1st_cand2_offsety;
	unsigned char  u3_ph_1st_cand5_offsetx;
	unsigned char  u3_ph_1st_update1_offsetx;

	unsigned char  u3_ip_2nd_cand1_offsetx;
	unsigned char  u3_ip_2nd_cand5_offsetx;
	unsigned char  u3_ip_2nd_update1_offsetx;
	unsigned char  u3_pi_2nd_cand1_offsetx;
	unsigned char  u3_pi_2nd_cand5_offsetx;
	unsigned char  u3_pi_2nd_update1_offsetx;
	unsigned char  u3_ph_2nd_cand1_offsetx;
	unsigned char  u3_ph_2nd_cand2_offsetx;
	unsigned char  u3_ph_2nd_cand2_offsety;
	unsigned char  u3_ph_2nd_cand5_offsetx;
	unsigned char  u3_ph_2nd_update1_offsetx;

	unsigned int reg_mc_var_lpf_en;
	unsigned int reg_mc_var_lpf_mvd_x_th;

	unsigned char reg_metop_sad_th_mode_sel;
	unsigned char reg_metop_ss_ip_penalty_g;
	unsigned char reg_metop_ss_pi_penalty_g;
	unsigned char reg_metop_ss_ph_penalty_g;
	unsigned char reg_metop_ds_ip_penalty_g;
	unsigned char reg_metop_ds_pi_penalty_g;
	unsigned char reg_metop_ds_ph_penalty_g;

	unsigned char reg_metop_ss_ip_penalty_z;
	unsigned char reg_metop_ss_pi_penalty_z;
	unsigned char reg_metop_ss_ph_penalty_z;
	unsigned char reg_metop_ds_ip_penalty_z;
	unsigned char reg_metop_ds_pi_penalty_z;
	unsigned char reg_metop_ds_ph_penalty_z;

	unsigned char reg_dh_empty_proc_en;

	unsigned int reg_metop_sc_clr_saddiff_th;
	unsigned char reg_pql_fb_tc_iir;

        //RO reg
	unsigned int u25_rdbk_dtl_all;
	unsigned int u25_rdbk_dtl_all_pre;
	unsigned int u25_rdbk_dtl_all_intervel5;
	unsigned int u25_rdbk_dtl_all_intervel5_pre;
	unsigned char  u8_rgn_apl_p_rb[32];
	unsigned char  u8_rgn_apl_p_rb_pre[32];

	//analysis result
	unsigned char u1_apl_inc;
	unsigned char u1_apl_dec;
*/
	unsigned int reg_vbuf_ip_1st_cand0_en;
	unsigned int reg_vbuf_ip_1st_cand1_en;
	unsigned int reg_vbuf_ip_1st_cand2_en;
	unsigned int reg_vbuf_ip_1st_cand3_en;
	unsigned int reg_vbuf_ip_1st_cand4_en;
	unsigned int reg_vbuf_ip_1st_cand5_en;
	unsigned int reg_vbuf_ip_1st_cand6_en;
	unsigned int reg_vbuf_ip_1st_cand7_en;
	unsigned int reg_vbuf_ip_1st_cand8_en;
	unsigned int reg_vbuf_pi_1st_cand0_en;
	unsigned int reg_vbuf_pi_1st_cand1_en;
	unsigned int reg_vbuf_pi_1st_cand2_en;
	unsigned int reg_vbuf_pi_1st_cand3_en;
	unsigned int reg_vbuf_pi_1st_cand4_en;
	unsigned int reg_vbuf_pi_1st_cand5_en;
	unsigned int reg_vbuf_pi_1st_cand6_en;
	unsigned int reg_vbuf_pi_1st_cand7_en;
	unsigned int reg_vbuf_pi_1st_cand8_en;

	unsigned int reg_vbuf_ip_1st_update1_en;
	unsigned int reg_vbuf_ip_1st_update2_en;
	unsigned int reg_vbuf_ip_1st_update3_en;
	unsigned int reg_vbuf_ip_1st_update0_en;

	unsigned int reg_vbuf_pi_1st_update1_en;
	unsigned int reg_vbuf_pi_1st_update2_en;
	unsigned int reg_vbuf_pi_1st_update3_en;
	unsigned int reg_vbuf_pi_1st_update0_en;

	// candidate position
	unsigned int reg_vbuf_ip_1st_cand0_offsetx;
	unsigned int reg_vbuf_ip_1st_cand0_offsety;
	unsigned int reg_vbuf_ip_1st_cand1_offsetx;
	unsigned int reg_vbuf_ip_1st_cand1_offsety;
	unsigned int reg_vbuf_ip_1st_cand2_offsetx;
	unsigned int reg_vbuf_ip_1st_cand2_offsety;
	unsigned int reg_vbuf_ip_1st_cand3_offsetx;
	unsigned int reg_vbuf_ip_1st_cand3_offsety;
	unsigned int reg_vbuf_ip_1st_cand4_offsetx;
	unsigned int reg_vbuf_ip_1st_cand4_offsety;
	unsigned int reg_vbuf_ip_1st_cand5_offsetx;
	unsigned int reg_vbuf_ip_1st_cand5_offsety;
	unsigned int reg_vbuf_ip_1st_cand6_offsetx;
	unsigned int reg_vbuf_ip_1st_cand6_offsety;
	unsigned int reg_vbuf_ip_1st_cand7_offsetx;
	unsigned int reg_vbuf_ip_1st_cand7_offsety;
	
	unsigned int reg_vbuf_pi_1st_cand0_offsetx;
	unsigned int reg_vbuf_pi_1st_cand0_offsety;
	unsigned int reg_vbuf_pi_1st_cand1_offsetx;
	unsigned int reg_vbuf_pi_1st_cand1_offsety;
	unsigned int reg_vbuf_pi_1st_cand2_offsetx;
	unsigned int reg_vbuf_pi_1st_cand2_offsety;
	unsigned int reg_vbuf_pi_1st_cand3_offsetx;
	unsigned int reg_vbuf_pi_1st_cand3_offsety;
	unsigned int reg_vbuf_pi_1st_cand4_offsetx;
	unsigned int reg_vbuf_pi_1st_cand4_offsety;
	unsigned int reg_vbuf_pi_1st_cand5_offsetx;
	unsigned int reg_vbuf_pi_1st_cand5_offsety;
	unsigned int reg_vbuf_pi_1st_cand6_offsetx;
	unsigned int reg_vbuf_pi_1st_cand6_offsety;
	unsigned int reg_vbuf_pi_1st_cand7_offsetx;
	unsigned int reg_vbuf_pi_1st_cand7_offsety;

	unsigned int reg_vbuf_ip_1st_update0_offsetx;
	unsigned int reg_vbuf_ip_1st_update0_offsety;
	unsigned int reg_vbuf_ip_1st_update1_offsetx;
	unsigned int reg_vbuf_ip_1st_update1_offsety;
	unsigned int reg_vbuf_ip_1st_update2_offsetx;
	unsigned int reg_vbuf_ip_1st_update2_offsety;
	unsigned int reg_vbuf_ip_1st_update3_offsetx;
	unsigned int reg_vbuf_ip_1st_update3_offsety;

	unsigned int reg_vbuf_pi_1st_update0_offsetx;
	unsigned int reg_vbuf_pi_1st_update0_offsety;
	unsigned int reg_vbuf_pi_1st_update1_offsetx;
	unsigned int reg_vbuf_pi_1st_update1_offsety;
	unsigned int reg_vbuf_pi_1st_update2_offsetx;
	unsigned int reg_vbuf_pi_1st_update2_offsety;
	unsigned int reg_vbuf_pi_1st_update3_offsetx;
	unsigned int reg_vbuf_pi_1st_update3_offsety;

	// candidate penalty
	unsigned int reg_me1_ip_cddpnt_st0;
	unsigned int reg_me1_ip_cddpnt_st1;
	unsigned int reg_me1_ip_cddpnt_st2;
	unsigned int reg_me1_ip_cddpnt_st3;
	unsigned int reg_me1_ip_cddpnt_st4;
	unsigned int reg_me1_ip_cddpnt_st5;
	unsigned int reg_me1_ip_cddpnt_st6;
	unsigned int reg_me1_ip_cddpnt_st7;
	unsigned int reg_me1_pi_cddpnt_st0;
	unsigned int reg_me1_pi_cddpnt_st1;
	unsigned int reg_me1_pi_cddpnt_st2;
	unsigned int reg_me1_pi_cddpnt_st3;
	unsigned int reg_me1_pi_cddpnt_st4;
	unsigned int reg_me1_pi_cddpnt_st5;
	unsigned int reg_me1_pi_cddpnt_st6;
	unsigned int reg_me1_pi_cddpnt_st7;

	unsigned int reg_me1_ip_cddpnt_rnd0;
	unsigned int reg_me1_ip_cddpnt_rnd1;
	unsigned int reg_me1_ip_cddpnt_rnd2;
	unsigned int reg_me1_ip_cddpnt_rnd3;
	unsigned int reg_me1_pi_cddpnt_rnd0;
	unsigned int reg_me1_pi_cddpnt_rnd1;
	unsigned int reg_me1_pi_cddpnt_rnd2;
	unsigned int reg_me1_pi_cddpnt_rnd3;

	unsigned int reg_me1_ip_cddpnt_gmv;
	unsigned int reg_me1_pi_cddpnt_gmv;

       //  h/v  scalar down filter
	unsigned int reg_ipme_h_flp_alp0    ;
	unsigned int reg_ipme_h_flp_alp1     ;
	unsigned int reg_ipme_h_flp_alp2     ;
	unsigned int reg_ipme_h_flp_alp3     ;
	unsigned int reg_ipme_h_flp_alp4     ;
	unsigned int reg_ipme_h_flp_alp5     ;
	unsigned int reg_ipme_h_flp_alp6     ;
	unsigned int reg_ipme_v_flp_alp0     ;
	unsigned int reg_ipme_v_flp_alp1     ;
	unsigned int reg_ipme_v_flp_alp2     ;
	unsigned int reg_ipme_v_flp_alp3     ;
	unsigned int reg_ipme_v_flp_alp4     ;
	unsigned int reg_ipme_v_flp_alp5     ;
	unsigned int reg_ipme_v_flp_alp6     ;

       //me1 mvdiff curve
	unsigned int reg_me1_mvd_cuv_x1   ;
	unsigned int reg_me1_mvd_cuv_x2   ;
	unsigned int reg_me1_mvd_cuv_x3   ;
	unsigned int reg_me1_mvd_cuv_y1   ;
	unsigned int reg_me1_mvd_cuv_y2   ;
	unsigned int reg_me1_mvd_cuv_y3   ;
	unsigned int reg_me1_mvd_cuv_slope1    ;
	unsigned int reg_me1_mvd_cuv_slope2    ;
	 //me1 adptpnt rnd curve
	unsigned int reg_me1_adptpnt_rnd_y1   ;
	unsigned int reg_me1_adptpnt_rnd_y2   ;
	unsigned int reg_me1_adptpnt_rnd_y3   ;
	unsigned int reg_me1_adptpnt_rnd_y4    ;
	unsigned int reg_me1_adptpnt_rnd_slope2    ;

	unsigned int reg_me1_gmv_sel;
	unsigned int reg_me1_gmvd_sel;

	unsigned int reg_me1_gmvd_ucf_x1;
	unsigned int reg_me1_gmvd_ucf_x2;
	unsigned int reg_me1_gmvd_ucf_x3;
	unsigned int reg_me1_gmvd_ucf_y1;
	unsigned int reg_me1_gmvd_ucf_y2;
	unsigned int reg_me1_gmvd_ucf_y3;
	unsigned int reg_me1_gmvd_ucf_slope1;
	unsigned int reg_me1_gmvd_ucf_slope2;

	unsigned int reg_me1_gmvd_cnt_x1;
	unsigned int reg_me1_gmvd_cnt_x2;
	unsigned int reg_me1_gmvd_cnt_x3;
	unsigned int reg_me1_gmvd_cnt_y1;
	unsigned int reg_me1_gmvd_cnt_y2;
	unsigned int reg_me1_gmvd_cnt_y3;
	unsigned int reg_me1_gmvd_cnt_slope1;
	unsigned int reg_me1_gmvd_cnt_slope2;

	unsigned int reg_me1_pi_gmvd_cost_limt;
	unsigned int reg_me1_ip_gmvd_cost_limt;
	unsigned int reg_me2_ph_gmvd_cost_limt;

	unsigned int u25_rdbk_dtl_all;
	unsigned int u25_rdbk_dtl_all_pre;
	unsigned int u25_rdbk_dtl_all_intervel5;
	unsigned int u25_rdbk_dtl_all_intervel5_pre;

	unsigned int reg_mc_scene_change_fb;
	unsigned int reg_me1_sc_saddiff_th;

	unsigned int   u8_dynME_acdc_bld_x_act[3];
	unsigned int   u6_dynME_acdc_bld_y_act[3];
	int  s9_dynME_acdc_bld_s_act[2];
	unsigned int   u8_dynME_ac_pix_sad_limt_act;
	unsigned int   u4_acdc_bld_psad_shift_act;

	unsigned int   u8_dynME2_acdc_bld_x_act[3];
	unsigned int   u6_dynME2_acdc_bld_y_act[3];
	int  s9_dynME2_acdc_bld_s_act[2];

	unsigned int reg_kmc_blend_y_alpha;
	unsigned int reg_kmc_blend_uv_alpha;
	unsigned int reg_kmc_blend_logo_y_alpha;
	unsigned int reg_kmc_blend_logo_uv_alpha;
	//============  me1 drp  k5lp===
	unsigned int reg_me1_drp_cuv_y0   ;
	unsigned int reg_me1_drp_cuv_y1   ;
	unsigned int reg_me1_drp_cuv_y2   ;

	//============  local fb===
	unsigned char reg_dh_dtl_curve_x1;
	unsigned char reg_dh_dtl_curve_x2;
	unsigned char reg_dh_dtl_curve_y1;
	unsigned char reg_dh_dtl_curve_y2;
	//==============

	//============  sobj ===
	unsigned char reg_mc_sobj_ymin0;
	unsigned char reg_mc_sobj_ymin1;
	unsigned char reg_mc_sobj_slop1;
	//======================
	//============me2  sobj ===
	unsigned int reg_me2_sec_small_object_sad_th;
	unsigned int reg_me2_sec_small_object_sm_mv_th;
	unsigned int reg_me2_sec_small_object_ip_pi_mvdiff_gain;
	unsigned int reg_me2_sec_small_object_ph_mvdiff_gain;
	//======================

	//============  lfsr_mask ===
	unsigned char reg_vbuf_ip_1st_lfsr_mask0_x ;
	unsigned char reg_vbuf_ip_1st_lfsr_mask0_y ;
	unsigned char reg_vbuf_ip_1st_lfsr_mask1_x ;
	unsigned char reg_vbuf_ip_1st_lfsr_mask1_y ;
	unsigned char reg_vbuf_ip_1st_lfsr_mask2_x ;
	unsigned char reg_vbuf_ip_1st_lfsr_mask2_y ;
	unsigned char reg_vbuf_ip_1st_lfsr_mask3_x ;
	unsigned char reg_vbuf_ip_1st_lfsr_mask3_y ;

	unsigned char reg_vbuf_pi_1st_lfsr_mask0_x ;
	unsigned char reg_vbuf_pi_1st_lfsr_mask0_y ;
	unsigned char reg_vbuf_pi_1st_lfsr_mask1_x ;
	unsigned char reg_vbuf_pi_1st_lfsr_mask1_y ;
	unsigned char reg_vbuf_pi_1st_lfsr_mask2_x ;
	unsigned char reg_vbuf_pi_1st_lfsr_mask2_y ;
	unsigned char reg_vbuf_pi_1st_lfsr_mask3_x ;
	unsigned char reg_vbuf_pi_1st_lfsr_mask3_y ;
	//======================

	//============  zmv control ===
	unsigned char reg_vbuf_ip_1st_zmv_en;
	unsigned char reg_vbuf_pi_1st_zmv_en;
	unsigned int reg_me1_ip_cddpnt_zmv;
	unsigned int reg_me1_pi_cddpnt_zmv;
	//======================

	//=======================
	unsigned int reg_vbuf_invalid_cnt_th;
	//=======================

	//=========CRTC=========
	unsigned int reg_crtc1_frc_vtrig_dly;
	unsigned int reg_crtc1_mc_vtrig_dly_long;
	unsigned int reg_crtc1_lbmc_vtrig_dly;
	//======================

	unsigned int reg_vbuf_ip_dc_obme_mode_sel;
	unsigned int reg_vbuf_pi_dc_obme_mode_sel;
	unsigned int reg_vbuf_ip_dc_obme_mode;
	unsigned int reg_vbuf_pi_dc_obme_mode;
	unsigned int reg_dehalo2_90;
	unsigned int reg_dehalo2_A8;
	unsigned int reg_dehalo2_E8;
	unsigned int reg_dehalo2_14;
	unsigned int reg_dehalo2_18;
	unsigned int reg_dehalo2_1C;
	unsigned int reg_dehalo2_20;
	unsigned int reg_dehalo2_24;
	unsigned int reg_dehalo2_28;
}MemcStatus;

typedef struct _ScalerMEMCGlobalStatus
{
	MemcStatus MEMC_STATUS;

	unsigned int Frame_Couter;

} ScalerMEMCGlobalStatus;

typedef struct
{
	ScalerMEMCGlobalStatus SIS_MEMC;

	unsigned char u1_RP_detect_true;
	unsigned char u1_Region_Periodic_detect_true;
	unsigned char u1_casino_RP_detect_true;
	unsigned char u1_Avenger_RP_detect_true;
	unsigned char u1_Natural_HighFreq_detect_true;
	unsigned int u32_new_saddiff_th;
	unsigned int u32_new_saddiff_th_pre;
	int   s10_me_GMV_1st_vy_rb_iir;
	unsigned char  u6_me_GMV_1st_cnt_rb_iir;
	unsigned int u12_me_GMV_1st_unconf_rb_iir;
	unsigned char  u1_chaos_true;
	unsigned char  u8_chaos_apply;
	unsigned char   u8_big_apldiff_hold_cnt;
	unsigned char  u1_ToL10_RP_true;
	unsigned int  u32_glb_sad_pre;
	unsigned int	u32_big_apldiff_saddiff_th;
	unsigned char	u8_big_apldiff_sc_hold_cnt;
	unsigned int	u32_big_apldiff_sad_pre;
	unsigned char	u8_EPG_apply;
	unsigned char	u8_EPG_moveROI;
	unsigned int	u16_simple_scene_score_pre;

	//panning detect
	unsigned char	u2_panning_flag;
	unsigned char	u8_panning_cnt;
	unsigned char	u2_panning_direction;

	// general motion dir
	unsigned char	u2_GMV_single_move;
	// general detail dir
	unsigned char	u2_mcDtl_HV;

	// pure vertical panning
	unsigned char   u1_pure_TopDownPan_flag;
	unsigned char   u8_pure_TopDownPan_cnt;

	// boundary vertical panning
	unsigned char	u1_boundary_v_panning_flag;
	unsigned char	u8_boundary_v_panning_cnt;

	unsigned char u1_RP_DTLPRD_move_check;

	unsigned char u3_Dynamic_MVx_Step;
	unsigned char u3_Dynamic_MVy_Step;

	unsigned char u1_MainObject_Move;

	// over search range
	unsigned char u1_over_searchrange;

	// median filter
	bool  b_do_ME_medFLT;
	bool  b_big_relMV;
	bool  b_Chaos;
	bool  b_TCss;
	bool  b_Prd;
	bool  b_prd_scene;
	bool  b_logo_scene;
	bool  b_g_panning;
	bool  b_2grp_GMV;
	bool  b_DRP_special_scene_check;
	unsigned char u8_big_relMV_cnt;
	unsigned char u8_rChaos_cnt;
	unsigned char u8_rTCss_cnt;
	unsigned char u8_rPrd_cnt;
	unsigned char u8_panning_dbCnt;
	unsigned char u8_prd_dbCnt;
	unsigned char u8_big_relMV_dbCnt;
	unsigned char u8_rChaos_dbCnt;

	unsigned char u3_occl_ext_level;
	unsigned char u3_occl_post_corr_level;

	unsigned int u32_aFrmMot_4Max;
	unsigned int u32_cFrmMot_4Max;
	unsigned char u1_Q13835_flag;
	unsigned char u1_RP_035_flag;
	unsigned char u1_RP_053_flag;
	unsigned char u1_RP_062_1_flag;
	unsigned char u1_RP_062_2_flag;
	unsigned char u1_RP_095_flag;
	unsigned char u1_RP_ID_flag;
	unsigned char u1_zmv_on_flag;
	unsigned int u32_RP_ID_Show;
	unsigned char u1_BTS_DRP_detect_true;
	unsigned char u1_SAN_ANDREAS_DRP_detect_true;
	unsigned char u8_ES_rChaos_cnt_tmp[32];
	bool    b_ES_Chaos_tmp;
	unsigned char mvDiff_tmp[32];
	unsigned char big_relMV_cnt_tmp[32];
	// adaptive median filter
	bool b_do_adpt_med9flt;
	bool b_do_pfvd;
	//gmv ctrl
	int s1_gmv_calc_mvx;
	int s1_gmv_calc_mvy;

	unsigned char u1_vpan_for_nearrim;

	unsigned char u2_strong_pure_panning_flag;

	unsigned char u8_RepeatMode;
	bool u1_SportScene;

	// mean absolute deviation 
	bool u1_uniform_panning;
	unsigned char u8_MAD_x;
	unsigned char u8_MAD_y;
	signed short s11_avg_rmv_x;
	signed short s10_avg_rmv_y;
}_OUTPUT_ME_SCENE_ANALYSIS;

typedef enum
{
	Pan_Dir_None,
	Pan_Dir_X,
	Pan_Dir_Y,
}_MEMC_Panning_Direction;

typedef enum {
	RepeatMode_OFF = 0,
	RepeatMode_LOW,
	RepeatMode_MID,
	RepeatMode_HIGH,
	RepeatMode_SC,
} RepeatMode;

typedef enum {
	RepeatCase_None=0,
	RepeatCase_DetRpPanAndSlowMV,
	RepeatCase_DetRpAndSlowMV,
	RepeatCase_PanAndSlowMV,
	RepeatCase_PurePan,
	RepeatCase_VerySlowMV,
	RepeatCase_SlowMV,
	RepeatCase_Casino,
	RepeatCase_HoldOn,
	RepeatCase_HoldOn_SlowMV,
	RepeatCase_Sport=0xe,
	RepeatCase_SC=0xf,
} RepeatCase;

VOID Me_sceneAnalysis_Init(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID Me_sceneAnalysis_Proc_OutputIntp(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput, int iSliceIdx);

VOID MEMC_MiddleWare_StatusInit(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_MiddleWare_CRTC_StatusInit(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_MiddleWare_Reg_Status_Cad1(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_MiddleWare_StatusUpdate(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput, int iSliceIdx);

VOID MEMC_LightSwordDetect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_RepeatPattern_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Natural_HighFreq_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Casino_RP_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
unsigned char MEMC_RepeatBG_detect(VOID);
unsigned char MEMC_BlackSC_detect(VOID);
unsigned char MEMC_RMV_Consistent_detect(VOID);
unsigned char MEMC_GMV_detect(unsigned char * u11_GMV_x_max_rb,unsigned char *u10_GMV_y_max_rb);
unsigned char MEMC_RGNDtlPRD_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
unsigned char MEMC_RGNDtl_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
unsigned char MEMC_RGNPRD_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
unsigned char MEMC_HVDtl_Move_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Periodic_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Region_Periodic_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_MAD_detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Repeat_Periodic_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam,_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Panning_Detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_MotionInfo_SLD(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Big_APLDiff_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_EPG_Protect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Simple_Scene_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_PureTopDownPanning_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_BoundaryHPanning_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_DynamicSearchRange_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_MainObject_Move_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Over_SearchRange_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Relative_Motion_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);

VOID MEMC_Occl_Ext_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_ME_OFFSET_Proc(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_RMV_Detect_DTV_Ch2_1_tv006(_OUTPUT_ME_SCENE_ANALYSIS *pOutput); 
VOID MEMC_Motion_Info_Detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput); 
VOID MEMC_boundary_mv_convergence_Detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_special_DRP_Detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_adpt_med9flt_pfvd_condition(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_NearRim_Vpan_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
VOID MEMC_Sport_Scene_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);

VOID MEMC_info_Print(_OUTPUT_ME_SCENE_ANALYSIS *pOutput);
#ifdef BG_MV_GET
VOID MEMC_BG_MV_Get(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput);
#endif

/////////////////////////////////////////////////////////////////////////////

DEBUGMODULE_BEGIN(_PARAM_ME_SCENE_ANALYSIS,_OUTPUT_ME_SCENE_ANALYSIS)
 //   ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u1_RepeatPattern_en,         1, "")
	ADD_PARAM_DES(unsigned char,  u1_Natural_HighFreq_en,         1, "")
	ADD_PARAM_DES(unsigned char,  u1_LightSword_en,         1, "")
	ADD_PARAM_DES(unsigned char,  u1_BigAPLDiff_en,		 1, "")
DEBUGMODULE_END

#endif


