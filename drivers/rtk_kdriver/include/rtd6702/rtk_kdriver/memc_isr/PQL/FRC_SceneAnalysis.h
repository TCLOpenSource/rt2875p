#ifndef _FRC_SCENEANALYSIS_H_
#define _FRC_SCENEANALYSIS_H_


#ifdef __cplusplus
extern "C" {
#endif


/* Param Definition*/
typedef struct
{
	unsigned char   u1_fadeInOut_en                    ;
	unsigned char	u1_fdIO_acdc_cnt_en			       ;
	unsigned char	u1_fdIO_acdc_ratio_en		       ;
	unsigned char	u1_fdIO_bv_sad_en                  ;
	unsigned char	u1_fdIO_apl_diff_en                ;
	unsigned char	u1_fdIO_conf_iir_en                ;
	unsigned char	u1_fdIO_sc_clear_en                ;

	unsigned int  u10_fdIO_apl_diff_thr_low          ;
	unsigned int  u10_fdIO_apl_diff_thr_hig          ;
	unsigned int  u8_fdIO_acdc_cnt_alpha_low         ;
	unsigned int	u8_fdIO_acdc_cnt_alpha_hig         ;
	unsigned int  u31_fdIO_acdc_dcsad_thr            ;
	unsigned char   u8_fdIO_acdc_sad_ratio_low         ;  //ac_sad/dc_sad -> 0~256(low, high)->0~16
	unsigned char	u8_fdIO_acdc_sad_ratio_hig         ;
	unsigned short  u16_fdIO_bv_sad_low                ;  //each block sad thr
	unsigned short	u16_fdIO_bv_sad_hig                ;
	unsigned char   u4_fdIO_conf_iir_alpha             ;
	unsigned char   u5_fadeInOut_dh_close_thr          ;
	unsigned char   u5_fadeInOut_logo_close_thr        ;
	unsigned char   u4_acdc_bld_holdtime               ;
	unsigned int  u20_fdIO_acdc_cnt_low              ;
	unsigned int	u20_fdIO_acdc_cnt_hig              ;

	unsigned char   u3_fdIO_apl_diff_frm_cnt           ;

	unsigned char   u4_dynME_acdc_bld_alpha_x1         ;
	unsigned char   u4_dynME_acdc_bld_alpha_x2         ;

	unsigned char   u8_dynME_acdc_bld_x_com[3]         ;
	unsigned char   u6_dynME_acdc_bld_y_com[3]         ;
	unsigned char   u8_dynME_acdc_bld_x_fdIO[3]        ;
	unsigned char   u6_dynME_acdc_bld_y_fdIO[3]        ;

	unsigned char   u8_dynME_ac_pix_sad_limt_com       ;
	unsigned char   u8_dynME_ac_pix_sad_limt_fdIO      ;

	unsigned char   u6_dynME_ac_dc_bld_apl_holdfrm     ;

    unsigned char   u4_acdc_bld_psad_shift_com         ;
	unsigned char   u4_acdc_bld_psad_shift_fdIO        ;

	unsigned char   u1_acdc_bld_sc_clear_en            ;

	unsigned char    u1_dynSR_en;
	unsigned int   u11_dynSR_rmv_x_thr0;
	unsigned int   u10_dynSR_rmv_y_thr0;
	unsigned int   u11_dynSR_rmv_x_thr1;
	unsigned int   u10_dynSR_rmv_y_thr1;
	unsigned int   u12_dynSR_rmv_cnt_thr0;
	unsigned int   u12_dynSR_rmv_cnt_thr1;
	unsigned int   u12_dynSR_rmv_unconf_thr0;
	unsigned int   u12_dynSR_rmv_unconf_thr1;
	unsigned int   u9_dynSR_mvx_range0;
	unsigned int   u9_dynSR_mvx_range1;
	unsigned int   u8_dynSR_mvy_range0;
	unsigned int   u8_dynSR_mvy_range1;
	unsigned char    u6_dynSR_rmv_x_num_thr0;
	unsigned char    u6_dynSR_rmv_x_num_thr1;
	unsigned char    u6_dynSR_rmv_y_num_thr0;
	unsigned char    u6_dynSR_rmv_y_num_thr1;
	unsigned char	 u1_dyn_SAD_Diff_en;
        //=========  Dynamic_SAD_Diff
	unsigned char    u8_dyn_SADGain_thr;
	unsigned int	 u27_dyn_SAD_Diff_TC_th_l;
	unsigned int	 u27_dyn_SAD_Diff_TC_th_h;
	unsigned int	 u8_dyn_SAD_Diff_TC_gain_min;
	unsigned int	 u8_dyn_SAD_Diff_TC_gain_max;
        unsigned int	 u8_dyn_SAD_Diff_sadAll_gain_min;
	unsigned int	 u8_dyn_SAD_Diff_sadAll_gain_max;
        //======
	unsigned int	 u28_dyn_SAD_Diff_sadAll_th_l;
	unsigned int	 u28_dyn_SAD_Diff_sadAll_th_h;
	unsigned char	 u4_dyn_SAD_Diff_rmv_ratio_th_l;
	unsigned char	 u4_dyn_SAD_Diff_rmv_ratio_th_h;
	unsigned char	 u6_dyn_SAD_Diff_gmv_ratio_th;

	//////////////////////////////////////////////////////////////////////////

	unsigned char  u1_RgPan_Detect_en;
	unsigned int u16_RgPan_rMV_th_h;
	unsigned int u16_RgPan_rMV_th_l;

	unsigned int u22_RgPan_rSC_th_h;
	unsigned int u22_RgPan_rSC_th_l;

	unsigned int u22_RgPan_rTC_th_h;
	unsigned int u22_RgPan_rTC_th_l;

	unsigned char  u5_RgPan_alp_th;


	//goodgmv, mvd cnt
	unsigned char	s2m_goodmv_thr;
	unsigned char	s2m_goodmv_unconf;
	unsigned char	s2m_diffmv_thr;
	unsigned char   s2m_cnt_iir_alpha;
	unsigned char   s2m_method_sel;

	unsigned char   u1_zoom_detect_en;
	unsigned char   u1_mc2_var_lpf_wrt_en;

	// fast motion
#if 1
	unsigned short u8_hpan_th;
	unsigned short u8_vpan_th;
	unsigned short u8_cnt2_th;
	//unsigned char u8_fastMotion_entercnt_th;
	//unsigned char u8_fastMotion_holdCnt_max;
	unsigned int u25_glb_dtl_th;
	unsigned char u1_fastMotion_gfb_en;
	unsigned char u8_fastMotion_glbfb_min;
	unsigned char u1_fastMotion_lfb_en;
	unsigned char u6_localfallback_lvl;
	unsigned char u1_fastMotion_det_en;

#endif

	unsigned char   u1_special_scfb_en;

	unsigned char  u1_zmv_adapt_pnt_en;
	unsigned char  u11_zmv_adapt_pnt_GMV_mvx_thr;
	unsigned char  u10_zmv_adapt_pnt_GMV_mvy_thr;
	unsigned int u13_zmv_adapt_pnt_GMV_Small_Y1;
	unsigned int u13_zmv_adapt_pnt_GMV_Big_Y1;

	unsigned char  u1_me1_gmvd_sel_en;
	unsigned char  u11_me1_gmvd_sel_mvx_diff_thr;
	unsigned char  u10_me1_gmvd_sel_mvy_diff_thr;
	unsigned char  u4_me1_gmvd_sel_1st_gmv_ratio_thr;
	unsigned char  u4_me1_gmvd_sel_2nd_gmv_ratio_thr;

	//Scene analysis
	unsigned char  u1_SA_en;
	unsigned int u20_SA_good_rgn_SAD_th;
	unsigned int u12_SA_good_rgn_unconf_th;
	unsigned int u12_SA_good_rgn_TC_th;
	unsigned char  u5_SA_good_rgn_num_th;
	unsigned char  u5_SA_bad_rgn_num_th;
	unsigned char  u4_SA_good_scene_enter_cnt;
	unsigned char  u4_SA_bad_scene_enter_cnt;
	unsigned int u20_SA_bad_rgn_SAD_th;
	unsigned int u12_SA_bad_rgn_unconf_th;
	unsigned int u12_SA_bad_rgn_TC_th;
	unsigned char  u3_SA_bad_scene_keep_cnt;

	//hold frame of pfv cdd after SC
	unsigned char  u8_SC_PFV_CDD_hold_cnt;

	// dynamic mv accord
	unsigned char  u1_mv_accord_en;
	unsigned char  u6_mv_accord_hold_cnt;
	unsigned char  u6_mv_accord_mvx_thr;
	unsigned char  u5_mv_accord_mvy_thr;
	unsigned char  u6_mv_accord_gmv_ratio_thr;
}_PARAM_FRC_SceneAnalysis;

typedef struct
{

	unsigned char   u4_fadeInOut_conf				      ; //fadeInOut Scene Confidence
	unsigned char   u4_fadeInOut_conf_iir			      ; //fadeInOut Scene Confidence

	unsigned char   u4_fadeInOut_cnt_alpha				  ;
	unsigned char   u4_fadeInOut_cnt_Thr_alpha			  ;
	unsigned char   u4_fadeInOut_acdc_sad_alpha  		  ;
	unsigned char   u4_fadeInOut_bv_sad_alpha			  ;
	unsigned char	u4_fadeInOut_apl_diff_alpha           ;
	unsigned int	u32_apldiff;

	unsigned char   u8_dynME_acdc_bld_x_act[3]            ;
	unsigned char   u6_dynME_acdc_bld_y_act[3]            ;
	signed short  s9_dynME_acdc_bld_s_act[2]            ;

	unsigned char   u8_dynME2_acdc_bld_x_act[3]            ;
	unsigned char   u6_dynME2_acdc_bld_y_act[3]            ;
	signed short  s9_dynME2_acdc_bld_s_act[2]            ;

	unsigned char   u8_dynME_ac_pix_sad_limt_act          ;

	unsigned char   u4_acdc_bld_psad_shift_act            ;

	signed short  u8_ME1_searchRange_x                  ;
	signed short  u8_ME1_searchRange_y                  ;
	unsigned char   u2_searchRange_x_status               ;
	unsigned char   u2_searchRange_y_status               ;

	unsigned char   u1_fadeInOut_dh_close                 ;
	unsigned char   u4_fadeInOut_logo_conf_iir;
	unsigned char   u1_fadeInOut_logo_clr;
	//unsigned char   u1_fadeInOut_acdc_bld                 ;
	//unsigned char   u1_fadeInOut_acdc_bld_pre             ;

	unsigned char   u5_dc_ac_bld_alpha                        ;
	unsigned char   u5_acdc_bld_holdtime                     ;

	unsigned int  u26_Apli[5];
	unsigned int  u32_frm_cnt;
	unsigned char   u1_Apli_same               ;
	unsigned char	u1_bad_scene;
	unsigned char	u8_dynSAD_Diff_Gain;
	unsigned int  u32_glb_sad_pre;
	unsigned int  u32_glb_sad_cur;
	unsigned char	u8_tc_gain;
	unsigned char   u8_saddiff_gain;
	unsigned char   u1_is_saddiff_large;

	unsigned char   u8_insidePan_LargeXnum;
	unsigned char   u8_insidePan_LargeYnum;
	unsigned int  u32_insidePan_avgTc;
	unsigned int  u32_insidePan_avgSc;
	unsigned char   u8_is_insidePan;
	unsigned char   u8_is_insidePanX;

	unsigned char   u8_goodGmv_cnt;
	unsigned char   u8_diffGmv_cnt;
	unsigned char	s2m_1st_cnt;
	unsigned char	s2m_2nd_cnt;
	unsigned char   s2m_12_cnt_iir;
	int			  s2m_1st_mvx;
	int			  s2m_1st_mvy;
	int			  s2m_2nd_mvx;
	int			  s2m_2nd_mvy;

	unsigned int  u32_insidePan_avgAbsMVX;
	unsigned int  u32_insidePan_avgAbsMVY;
	unsigned int  u32_unconf_avg;

	unsigned char   u8_BdySmall_lft_cnt;
	unsigned char	u8_BdySmall_rht_cnt;

	unsigned int u32_RgPan_idx;

	unsigned char   u4_dc_ac_alpha;
	unsigned char   u4_acdc_bld_holdtime;
	unsigned char   u8_zoom_force_fb_cnt;
	unsigned char   u1_still_frame;

	unsigned char  u1_fastMotion_det;
	unsigned char  u1_specialscfb_true;

	unsigned char  u1_zmv_adapt_pnt_GMV_Small;

	//Scene analysis
	unsigned char  u8_SA_good_scene_cnt;
	unsigned char  u1_SA_good_scene;
	unsigned char  u8_SA_bad_scene_cnt;
	unsigned char  u1_SA_bad_scene;
	unsigned char  u3_SA_bad_scene_keep_cnt;

	//hold frame of pfv cdd after SC
	unsigned char  u8_SC_PFV_CDD_hold_cnt;

	// dynamic sad diff thr
	unsigned int u32_new_saddiff_th;
	unsigned int u32_new_saddiff_th_pre;

	// dynamic mv accord
	unsigned char  u1_mv_accord_on;
	unsigned char  u6_mv_accord_hold_cnt;

	unsigned char  u1_RgRotat_true;
	unsigned char  u4_RgRotat_cnt;

	unsigned char  u1_Zoom_true;
	unsigned char  u4_ZoomIn_cnt;
	unsigned char  u4_ZoomOut_cnt;

	unsigned char  u1_Swing_true;
	unsigned char  u4_SwingRight_cnt;
	unsigned char  u4_SwingLeft_cnt;

	//in intp call
	unsigned char u8_HDR_PicChg_prt_apply;
	unsigned short u16_OSDBrightnessVal;
	unsigned short u16_OSDContrastVal;
	unsigned char u8_OSDBrightnessBigChg_prt_apply;
	unsigned char u8_OSDContrastBigChg_prt_apply;
	unsigned char u8_VOSeek_prt_apply;
	unsigned char u8_VOSeek_prt_logo_apply;
	unsigned char u8_OSDPictureModeChg_prt_apply;
	unsigned char u8_sad_overshoot_flag;
	//--end of "in intp call"

	unsigned char u6_gmv_ratio;
}_OUTPUT_FRC_SceneAnalysis;



/* MOdule UI Definition*/
DEBUGMODULE_BEGIN(_PARAM_FRC_SceneAnalysis, _OUTPUT_FRC_SceneAnalysis)

	////param
	ADD_PARAM(unsigned char,    u1_fadeInOut_en			     ,	  0 )
	ADD_PARAM(unsigned char,    u1_fdIO_acdc_cnt_en		     ,	  1 )
	ADD_PARAM(unsigned char,    u1_fdIO_acdc_ratio_en        ,	  1 )
	ADD_PARAM(unsigned char,    u1_fdIO_bv_sad_en		     ,	  1 )
	ADD_PARAM(unsigned char,    u1_fdIO_apl_diff_en		     ,	  1 )
	ADD_PARAM(unsigned char,    u1_fdIO_conf_iir_en		     ,	  1 )
	ADD_PARAM(unsigned char,    u1_fdIO_sc_clear_en		     ,	  0 )


	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned int,   u10_fdIO_apl_diff_thr_low    ,	  1,  "<low  ->0" )
	ADD_PARAM_DES(unsigned int,   u10_fdIO_apl_diff_thr_hig    ,	  3, ">high ->1" )
	ADD_PARAM(unsigned int,		u8_fdIO_acdc_cnt_alpha_low ,	  15  )
	ADD_PARAM(unsigned int,		u8_fdIO_acdc_cnt_alpha_hig ,	  128 )
	ADD_PARAM_DES(unsigned int,   u31_fdIO_acdc_dcsad_thr      ,	  20,  "need > dc-sad thr" )
	ADD_PARAM_DES(unsigned char,    u8_fdIO_acdc_sad_ratio_low   ,	  30, "ratio<low,  ->1" )
	ADD_PARAM_DES(unsigned char,    u8_fdIO_acdc_sad_ratio_hig   ,	  150, "ratio>high, ->0" )
	ADD_PARAM(unsigned short,		u16_fdIO_bv_sad_low			 ,	  40  )
	ADD_PARAM(unsigned short,		u16_fdIO_bv_sad_hig          ,    80 )
	ADD_PARAM(unsigned char,		u4_fdIO_conf_iir_alpha       ,    8)
	ADD_PARAM(unsigned char,		u4_fadeInOut_dh_close_thr    ,    8)
	ADD_PARAM(unsigned char,		u4_acdc_bld_holdtime     ,    0 )
	ADD_PARAM(unsigned int,		u20_fdIO_acdc_cnt_low    ,    0)
	ADD_PARAM(unsigned int,		u20_fdIO_acdc_cnt_hig    ,    8)


	ADD_DUMMY_PARAM()
	ADD_PARAM_ARRAY_DES(unsigned char, u8_dynME_acdc_bld_x_com,  3, 0, "u8.0")
	ADD_PARAM_ARRAY_DES(unsigned char, u6_dynME_acdc_bld_y_com,  3, 0, "u6.0")
	ADD_PARAM_ARRAY_DES(unsigned char, u8_dynME_acdc_bld_x_fdIO, 3, 0, "u8.0")
	ADD_PARAM_ARRAY_DES(unsigned char, u6_dynME_acdc_bld_y_fdIO, 3, 0, "u6.0")

	////output
	ADD_OUTPUT(unsigned char,   u4_fadeInOut_conf)
	ADD_OUTPUT(unsigned char,   u4_fadeInOut_conf_iir)
	ADD_OUTPUT(unsigned char,   u4_fadeInOut_cnt_alpha		)
	ADD_OUTPUT(unsigned char,   u4_fadeInOut_cnt_Thr_alpha		)
	ADD_OUTPUT(unsigned char,   u4_fadeInOut_acdc_sad_alpha )
	ADD_OUTPUT(unsigned char,   u4_fadeInOut_bv_sad_alpha	)
	ADD_OUTPUT(unsigned char,   u4_fadeInOut_apl_diff_alpha	)
	ADD_OUTPUT(unsigned int,   u32_apldiff	)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT_ARRAY_DES(unsigned char,  u8_dynME_acdc_bld_x_act, 3, "")
	ADD_OUTPUT_ARRAY_DES(unsigned char,  u6_dynME_acdc_bld_y_act, 3, "")
	ADD_OUTPUT_ARRAY_DES(signed short, s9_dynME_acdc_bld_s_act, 2, "")

	ADD_OUTPUT_ARRAY_DES(unsigned char,  u8_dynME2_acdc_bld_x_act, 3, "")
	ADD_OUTPUT_ARRAY_DES(unsigned char,  u6_dynME2_acdc_bld_y_act, 3, "")
	ADD_OUTPUT_ARRAY_DES(signed short, s9_dynME2_acdc_bld_s_act, 2, "")

	ADD_OUTPUT(unsigned char,   u1_fadeInOut_dh_close	)

DEBUGMODULE_END

	/* Function Declaration*/
	VOID FRC_SceneAnalysis_Init(_OUTPUT_FRC_SceneAnalysis *pOutput);
	VOID FRC_Still_Pattern_Detect(const _PARAM_FilmDetectCtrl *pParam,  _OUTPUT_FilmDetectCtrl *pOutput_film,  _OUTPUT_FRC_SceneAnalysis *pOutput);
	VOID FRC_FastMotion_Detect(const _PARAM_FRC_SceneAnalysis *pParam,  _OUTPUT_FRC_SceneAnalysis *pOutput);
	VOID FRC_SceneAnalysis_Proc(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput);
	VOID FRC_SceneAnalysis_Proc_InputIntp(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput);

	VOID SAD_Peak_Skip_GFB(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput);

#ifdef __cplusplus
}
#endif


#endif

