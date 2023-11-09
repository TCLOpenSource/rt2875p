#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/include/PQLAPI.h"
#include "memc_isr/PQL/PQLContext.h"

_PQLPARAMETER PQL_Glb_Param  =
{
	/*SystemInput:_param_external*/
	{
		0,             // u8Dummy
	},
	/*Read-Register:_param_read_comreg*/
	{
		1,             // u1_me_VbufSize_rb_en
		1,             // u1_sc_status_rb_en

		1,             // u1_me_aTC_rb_en
		1,             // u1_me_aSC_rb_en
		1,             // u1_me_aSAD_rb_en
		1,             // u1_me_aDTL_rb_en
		1,             // u1_me_statis_glb_prd_en
		1,             // u1_me_aAPLi_rb_en
		1,             // u1_me_aAPLp_rb_en

		0,             // u1_me_ZMV_cnt_rb_en
		0,             // u1_me_ZMV_dtl_cnt_rb_en
		1,             // u1_me_ZMV_psad_rb_en
		0,             // u1_me_ZGMV_cnt_rb_en

		1,             // u1_me_Hpan_cnt_rb_en
		1,             // u1_me_Vpan_cnt_rb_en
		1,             // u1_me_pnMV_cnt_rb_en

		1,             // u1_me_GMV_1st_rb_en
		1,             // u1_me_GMV_2nd_rb_en

		0xffffffff,    // u32_me_rTC_rb_en
		0xffffffff,    // u32_me_rSC_rb_en
		0xffffffff,    // u32_me_rSAD_rb_en
		0xffffffff,    // u32_me_rDTL_rb_en
		0xffffffff,    // u32_me_rAPLi_rb_en
		0xffffffff,    // u32_me_rAPLp_rb_en
		0xffffffff,    // u32_me_rPRD_rb_en
		0xffffffff,    // u32_me_rLFB_rb_en

		0xffffffff,    // u32_me_rMV_1st_rb_en
		0xffffffff,    // u32_me_rMV_2nd_rb_en

		1,             // u1_ipme_aMot_rb_en
		1,             // u1_ipme_5rMot_rb_en
		1,             // u1_ipme_12rMot_rb_en
		0,             // u1_ipme_filmMode_rb_en

		1,             // u1_BBD_rb_en

		1,             // u1_lg_blk_cnt_rb_en

		1,             // u1_kphase_rb_en

		0,             // u2_rb_tc_mode
		1,             // u4_rb_tc1_coef
		1,             // u4_rb_tc2_coef
		1,             // u4_rb_tc3_coef

		1,             // u1_dh_rb_en

		0,		  //u1_low_delay_test01
		0,		  //u1_low_delay_ctrl2
		0,		  //u1_low_delay_ctrl3
		0,		  //u1_Debug_OnlineMeasure

	 	1,             // u1_mc_HVdtl_rb_en
	 	1,		 // u1_me_ZMV_cnt_en

		1,			   //u1_color_amount_rb_en

	 	1,             // u1_powerSaving_en
	 	1,            // u1_HDMI_powerSaving_en
	 	0, 		// u1_jerry_PanAct_en
	 },
	/*Fall-back:_param_fblevelctrl*/
	{
        		1,             // u1_FBLevelCtrl_en
       		    0,             // u1_BadRegionCtrl_en
        		1,             // u1_FBLevelShow_en
        		0,             // u1_FbLvl_occlCtrl_en
        		0,             // u1_FbLvl_gmvCtrl_en
        		1,             // u1_FbLvl_disableSCclr_en

        		128,           // u8_FbLvl_dSCclr_d2en_th
        		64,            // u8_FbLvl_dSCclr_en2d_th
        		2,             // u8_FbLvl_dSCclr_holdCnt

        		0,             // u1_Fblvl_Dh_tcTh_mapEn
        		22,            // u8_FbLvl_Dh_tc_thLgain
        		38,            // u8_FbLvl_Dh_tc_thHgain
        		100,           // u8_DehaloFBlvl_iir

       		    0,             // u1_Fblvl_blur_ctrlEn
        		16,            // u8_FbLvl_blur_tc_thLgain
        		32,            // u8_FbLvl_blur_tc_thHgain
		        200,           // u8_FbLvl_blur_iir
        		110,           // u12_FbLvl_blur_gmvUnconf_th0
        		100,           // u12_FbLvl_blur_gmvUnconf_th1
        		8,             // u5_FbLvl_blur_badscene_dftLvl

        		0,             // u1_Fblvl_judder_ctrlEn
        		16,            // u8_FbLvl_judder_tc_thLgain
        		32,            // u8_FbLvl_judder_tc_thHgain
        		64,            // u8_FbLvl_judder_iir

        		750000,        // u27_Fb_tc_th_h
        		400000,        // u27_Fb_tc_th_l
        		64,            // u8_FBTempConsisIIR_up
        		128,           // u8_FBTempConsis_IIR_down
		        32,            // u8_FBTempConsis_limit_gain
		        800000,        // u32_FBTempConsis_limit_val
        		1000,          // u12_FBLevelIIRUp
        		1800,          // u12_FBLevelIIRDown

        		3,             // u8_SC_Gain
        		1,             // u8_SC_slope
        		2,             // u8_SC_HoldCnt
        		2,             // u8_SC_NormCnt

        		0,             // u8_badRgn_sad_iirA_up
        		0,             // u8_badRgn_sad_iirA_down
        		100000,        // u23_BadRegionSAD_th
        		180000,        // u23_BadRegionSAD_th_m
        		3000000,       // u23_BadRegionSAD_th_h
        		0,             // u8_badRgn_center_gain
        		1,             // u8_badRgn_center_oft
        		/*u5_BadRegionGain*/
		{
			0,             // u5_BadRegionGain[0]
			0,             // u5_BadRegionGain[1]
			0,             // u5_BadRegionGain[2]
			0,             // u5_BadRegionGain[3]
			0,             // u5_BadRegionGain[4]
			0,             // u5_BadRegionGain[5]
			0,             // u5_BadRegionGain[6]
			0,             // u5_BadRegionGain[7]
			0,             // u5_BadRegionGain[8]
			0,             // u5_BadRegionGain[9]
			0,             // u5_BadRegionGain[10]
			0,             // u5_BadRegionGain[11]
			0,             // u5_BadRegionGain[12]
			0,             // u5_BadRegionGain[13]
			0,             // u5_BadRegionGain[14]
			0,             // u5_BadRegionGain[15]
			0,             // u5_BadRegionGain[16]
			0,             // u5_BadRegionGain[17]
			0,             // u5_BadRegionGain[18]
			0,             // u5_BadRegionGain[19]
			0,             // u5_BadRegionGain[20]
			0,             // u5_BadRegionGain[21]
			0,             // u5_BadRegionGain[22]
			0,             // u5_BadRegionGain[23]
			0,             // u5_BadRegionGain[24]
			0,             // u5_BadRegionGain[25]
			0,             // u5_BadRegionGain[26]
			0,             // u5_BadRegionGain[27]
			0,             // u5_BadRegionGain[28]
			0,             // u5_BadRegionGain[29]
			0,             // u5_BadRegionGain[30]
			0,             // u5_BadRegionGain[31]
		},
		160,           // u8_BadRegionGainIIR

		0,             // u8_badRgn_dtl_thr1;
		0,             // u8_badRgn_dtl_thr2;
		0,             // u8_badRgn_dtl_gain1;
		0,             // u8_badRgn_dtl_gain2;

		0,             // u1_gmvratio_use
		100,           // u7_gmv_ratio_thH
		60,            // u7_gmv_ratio_thL
		32,            // u6_gmv_gain_max
		16,            // u6_gmv_gain_min
		//3,             // u6_gmv_similar_cnt
		0,             // u8_gmv_gain_iir
		0x384,		   // u12_gmv_rng_cnt_th
		8,			   // u12_gmv_unconf_cnt_th

		8,             // u8_FBLevelMapY1
		255,           // u8_FBLevelMapY2
		10,            // u8_FBLevelMapX1
		250,           // u8_FBLevelMapX2

		0,             // u1_FbLvl_disIIR_en
		0,             // u8_FbLvl_disIIR_holdCnt
		0,             // u1_scAnalysis_en
		3,             // u4_scAnalysis_frmCnt_th
		4,             // u4_scAnalysis_badFrmCnt_th

		1,             // u28_scAnalysis_sadAll_th0
		16,            // u28_scAnalysis_sadAll_th1
		1,             // u27_scAnalysis_TC_th0
		16,            // u27_scAnalysis_TC_th1
		1,             // u27_scAnalysis_static_motionTh
		16,            // u8_scAnalysis_st2mo_stPeriodTh

		1,             // u16_minusDiff_penalty

		1,             // u8_sc_bypass_cnt
		4,             // u8_static_cnt_th
		2000,          // u32_static_mot_th

		0,		// u20_good_rgn_sad_th;
		0,		// u12_good_rgn_gmv_unconf_th;
		0,		//  u8_fblvl_map_y2_good;
		0,		// u12_fblvl_iir_up_good;
		0,		//  u8_good_rgn_num_th1;
		0,		//  u8_good_rgn_num_th2;
		0,		//  u4_good_scene_hold_cnt;

		1,		// u1_FB_pan_apply_en;
		1,		// u1_FB_cross_apply_en;
	},

	/*Rim-control:_param_rimctrl*/
	{
		0,             // u1_RimCtrl_en
		20,            // u12_RimDiffTh
		1,             // u8_RimBiggerTh
		10,             // u8_RimSmallerTh
		3,		//u8_RimSmallerSCTh
		200,           // u8_RimIIRAlpha
		1,		//u5_RimLRSmallShift

		4,             // u8_mc_ptRim_shrink
		0,             // u8_me_pixRim_shrink
		0,			   // u8_lbme_pixRim_shrink
		2,				//u8_lbme_pixRim_comp

		2,             // u8_LogoBlkRimHOffset
		2,             // u8_LogoBlkRimVOffset
		8,             // u8_LogoPixRimHOffset
		8,             // u8_LogoPixRimVOffset
	},

	/*Dh-close:_param_dh_close*/
	{
		0,             // u2_dhClose_bypass
		1,             // u1_fblvl_en
		0,             // u1_meAllDtl_en
		0,             // u8_dhOff_holdFrm

		1,             // u8_fblvl_holdFrm
		160,             // u8_fbLvl_Th0
		80,             // u8_fbLvl_Th1

		100000,             // u25_meAllDtl_Th0
		10000000,             // u25_meAllDtl_Th1

		0,                // u1_dh_pfvconf_en;
		0,                // u20_dh_pfvconf_thrL;
		0,                // u6_dh_pfvconf_cnt_thr;
		0,                // u5_dh_pfvconf_holdfrm;
		0,                // u1_lbmc_switch_en
		0,                // u5_lbmc_switch_holdfrm
		0,				  // u1_panning_en
		0,				  // u5_panning_holdfrm

		0,                //u1_dh_fadeInOut_en;
		0,                //u5_dh_fadeInOut_holdfrm;
		0,                //u1_dh_sml_rgnAction_en;
		0,                //u8_dh_sml_rgnAction_gmv_x_thr;
		0,                //u8_dh_sml_rgnAction_gmv_y_thr;
		0,                //u12_dh_sml_rgnAction_gmv_cnt_thr;
		0,                //u12_dh_sml_rgnAction_gmv_ucof_thr;
		0,                //u10_dh_sml_rgnAction_badrgnsad_thr;
		0,                //u10_dh_sml_rgnAction_goodrgnsad_thr;
		0,                //u19_dh_sml_rgnAction_bad_pfvdiff_thr;
		0,                //u19_dh_sml_rgnAction_good_pfvdiff_thr;
		0,                //u5_dh_sml_rgnAction_holdfrm;
		0,                //u8_dh_sml_rgnAction_apl_thr;
		0,                //u5_dh_sml_rgnAction_goodrgn_cnt_thr;
		0,                //u5_dh_sml_rgnAction_badrgn_cnt_thr;

		0,                //u12_dh_sml_rgnAction_bad_gmv_cnt_thr;
		0,                //u12_dh_sml_rgnAction_bad_gmv_ucof_thr;

		0,				  //u1_simple_scene_en;

		0,				  //u1_pfv_cdd_num_en
		0,				  //u5_phfv0_candi_num;
		0,				  //u5_phfv1_candi_num;
		0,				  //u9_pfv_cdd_num_dtl_thr

		0,				  //unsigned char  u2_dh_me1_ip_dc_obme_mode_off;
		0,				  //unsigned char  u2_dh_me1_pi_dc_obme_mode_off;
		0,				  //unsigned char  u2_dh_me1_ip_ac_obme_mode_off;
		0,				  //unsigned char  u2_dh_me1_pi_ac_obme_mode_off;

		0,				  //unsigned char  u2_dh_me1_ip_dc_obme_mode_on;
		0,				  //unsigned char  u2_dh_me1_pi_dc_obme_mode_on;
		0,				  //unsigned char  u2_dh_me1_ip_ac_obme_mode_on;
		0,				  //unsigned char  u2_dh_me1_pi_ac_obme_mode_on;

		0,             	  // unsigned char  u1_dbg_print_en;
	},

	/*LBMC-switch:_param_mc_lbmcswitch*/
	{
		1,             // u8_lbmcSwitch_bypass

		248,           // u8_lf_Ovlap_s2n_cntTh
		10,            // u8_lf_Lmv_s2n_cntTh
		252,           // u8_lf_Ovlap_n2s_cntTh
		20,            // u8_lf_Lmv_n2s_cntTh

		248,           // u8_hf_Ovlap_s2n_cntTh
		10,            // u8_hf_Lmv_s2n_cntTh
		252,           // u8_hf_Ovlap_n2s_cntTh
		20,            // u8_hf_Lmv_n2s_cntTh
	},

	/*Phase-Table:_param_frc_phtable*/
	{
		1,             // u1_phT_autoGen_en

		0,             // u8_dejudder_lvl
		0,             // u8_deblur_lvl
		0,             // u2_candece_id_mode
		2,             // u4_cadence_id

		0,             // in_3dFormat
		0,             // out_3dFormat
		0,             // u2_outMode

		0,             // u1_outLR_LRsamePh
		1,             // u1_filmPh_sw_en
		0,             // u1_filmPh_pf_en

		0,             // u8_input_dly
		0,             // u1_Ph_delay_calc_en
		0,			   // u1_force_gen_table_en
	},

	/*Film-Detect:_param_filmDetectctrl*/
	{
		0,             // u1_filmDet_bypass
		8191,             // u32_filmDet_cadence_en

		8,             // u4_Mot_rgn_diffWgt
		8,             // u4_Mot_all_diffWgt
		4096,             // u27_Mot_all_min

		2,             // u4_enter_22_cntGain
		2,             // u4_enter_32_cntGain
		1,             // u4_enter_else_cntGain

		0,             // u1_quit_motWgt_auto_en
		5,             // u4_quit_bigMot_wgt
		3,             // u4_quit_smlMot_wgt
		4,             // u8_quit_cnt_th
		2,             // u8_quit_prd_th

		1,             //u1_mixMode_en;
		0,             // u1_mixMode_det_en
		/*u1_mixMode_rgnEn*/
		{
			0,
			0,
			0,
			0,
			0,
		},
		0,             // u8_mixMode_enter_cntTh
		0,             // u8_mixMode_cnt_max
		0,             // u27_mixMode_rgnMove_minMot
		0,             // u4_mixMode_rgnMove_motWgt
		1,	       // u1_FrcCadSupport_En;
		0,		// u1_QuickSwitch_En
		0,	       // u1_DbgPrt_FRChg_En
		0,		// u1_StillDetect_En
		0,		// u3_DbgPrt_Rgn
		0,		// u1_BadEditSlowOut_En
		0,             // u5_dbg_param1;
		0,             // u32_dbg_param2
		0,             // u32_dbg_cnt
	},

	/*Logo-clear:_param_FRC_LGDet*/
	{
		0,             // u1_logo_pql_en
		1,             // u1_logo_sw_clr_en
		0,             // u1_logo_rg_clr_en
		1,             // u1_logo_glb_clr_en
		1,             // u1_logo_left_half_clr_en
		1,             // u1_logo_right_half_clr_en
		1,             // u1_logo_top_half_clr_en
		1,             // u1_logo_bot_half_clr_en
		1,             // u1_logo_sc_alpha_ctrl_en
		1,             // u1_logo_sc_logo_clr_en

		215,           // u8_clr_rg_thr
		215,           // u8_clr_rg_thr_l
		80,            // u8_clr_glb_thr
		70,            // u8_clr_half_thr
		5,             // u6_rgClr_holdtime
		5,             // u6_glbClr_holdtime
		0,             // u4_blk_pix_merge_type
		31,            // u5_iir_alpha

		0,             //u1_RgDhClr_thr_en
		0,             //u16_RgDhClr_thr_l
		0,             //u16_RgDhClr_thr_h
		0,             //u5_RgDhClr_cnt_th

		0,             //u1_RgHstyClr_en                    ;
		0,             //u1_GlbHstyClr_en                   ;
		0,             //u1_RgHstyClr_Sc_en                 ;

		0,             //u16_RgHstyClr_mv_thr               ;
		0,             //u12_RgHstyClr_mv_cnt_thr          ;
		0,             //u12_RgHstyClr_mv_unconf_thr       ;
		0,             //u10_RgHstyClr_Rsad_thr             ;

		0,             //u16_RgHstyClr_Gmv_thr
		0,             //u8_RgHstyClr_Gmv_cnt_thr          ;
		0,             //u12_RgHstyClr_Gmv_unconf_thr       ;
		0,             //u10_RgHstyClr_Gsad_thr             ;
		0,             //u1_RgHstyClr_glbclr_close_en

		0,             //u1_RgDhClr_static_en;

		0,             // u8_rg_dh_clr_lgclr_hold_frm;
		0,             // u8_rg_dh_clr_static_hold_frm;

		0,            //u1_logo_sc_detect_en;
		0,            //u1_logo_sc_saddif_en;
		0,            //u1_logo_sc_Gmv_en;
		0,            //u1_logo_sc_dtldif_en;
		0,            //u31_logo_sc_saddiff_th;
		0,            //u6_logo_sc_Gmv_cnt_th;
		0,            //u12_logo_sc_Gmv_unconf_th;
		0,            //u20_logo_sc_dtldif_th;
		0,            //u8_logo_sc_hold_frm;

		0,            // u1_logo_static_bg_en;
		0,            // u6_logo_static_bg_th_l;
		0,            // u6_logo_static_bg_th_h;

		0,            // u1_param_reg_scdetct;

		0,           // u1_logo_netflix_detct_en;

		0,           // u20_logo_netflix_detect_lt_th_l;
		0,           // u20_logo_netflix_detect_lt_th_h;
		0,           // u20_logo_netflix_detect_rt_th_l;
		0,           // u20_logo_netflix_detect_rt_th_h;

		0,           // u20_logo_netflix_detect_lb_th_l;
		0,           // u20_logo_netflix_detect_lb_th_h;
		0,           // u20_logo_netflix_detect_rb_th_l;
		0,           // u20_logo_netflix_detect_rb_th_h;

		0,           // u8_logo_netflix_hold_frm;

		0,           // u1_param_reg_netflix_detect;

		0,           // u1_param_reg_lg16s_patch;
		0,           // u1_logo_lg16s_patch_en;
		0,           // u1_logo_lg16s_patch_Gapl_en;
		0,           // u1_logo_lg16s_patch_Gdtl_en;
		0,           // u1_logo_lg16s_patch_Gsad_en;
		0,           // u1_logo_lg16s_patch_Gmv_en;
		0,           // u1_logo_lg16s_patch_Rdtl_num_en;

		0,           // u10_logo_lg16s_patch_Gapl_th_l;
		0,           // u10_logo_lg16s_patch_Gapl_th_h;

		0,           // u25_logo_lg16s_patch_Gdtl_th_l;
		0,           // u25_logo_lg16s_patch_Gdtl_th_h;

		0,           // u28_logo_lg16s_patch_Gsad_th_l;
		0,           // u28_logo_lg16s_patch_Gsad_th_h;

		0,           // u11_logo_lg16s_patch_Gmvx_th_l;
		0,           // u11_logo_lg16s_patch_Gmvx_th_h;
		0,           // u11_logo_lg16s_patch_Gmvy_th;
		0,           // u6_logo_lg16s_patch_Gmv_cnt_th;
		0,           // u12_logo_lg16s_patch_Gmv_unconf_th;

		0,           // u20_logo_lg16s_patch_Rdtl_th_l;
		0,           // u20_logo_lg16s_patch_Rdtl_th_h;
		0,           // u6_logo_lg16s_patch_Rdtl_num_th;

		0,           // u8_logo_lg16s_patch_hold_frm;

		0,          // u16_RgDhadp_Rimunbalance_thr_l;
		0,          // u16_RgDhadp_Rimunbalance_thr_h ;
		0,          // u8_RgDhadp_Rimunbalance_hold_frm;

		0,          //u1_lg_UXN_patch_en;

		1,          // u1_logo_sc_FastDet_en
		60,         // u8_logo_sc_FastDet_rstholdfrm

		// for SLD
		0,	// u1_logo_sld_debug_print_en;
		1,	// u1_logo_sld_debug_print_period;

		60,		// u8_logo_rglt_pxldf_apl_thr_l;
		220,	// u8_logo_rglt_pxldf_apl_thr_h;
		190,	// u8_logo_rglt_pxldf_val_thr_l;
		230,	// u8_logo_rglt_pxldf_val_thr_h;

		0,	// u1_logo_rgclr_debug_print;
		2,	// u1_logo_rgclr_debug_print_period;

		1,		// u1_logo_rgclr_apl_en;
		241,	// u32_logo_rgclr_apl_thr;

		1,		// u1_logo_rgclr_dtl_en;
		5,		// u32_logo_rgclr_dtl_thr;
		200,	// u32_logo_rgclr_apl2dtl_thr;

		1,	// u1_logo_rgclr_cnt_en;
		60,	// u32_logo_rgclr_cnt_thr;

		1,	// u1_logo_rgclr_cmp_en;
		12,	// u8_logo_rgclr_logo_mv_thr;
		78,	// u32_logo_rgclr_cmp_gain_thr;
		20,	// u32_logo_rgclr_cmp_ofst_thr;

		//         0,             // u1_logo_dynY_en
		//         1,             // u1_logo_dynBlkTh_en
		//         1,             // u1_logo_dynPixTh_en
		//         1,             // u1_logo_dynBlkStep_en
		//         1,             // u1_logo_dynPixStep_en
		//         1,             // u1_logo_dynAlpha_en
		//         0,             // u1_logo_dynDelogo_en
		//         0,             // u1_logo_pixRgClr_en
		//         0,             // u1_logo_pixGlbClr_en
		//         0,             // u1_logo_blkGlbClrHsty_en
		//         0,             // u1_logo_pixGlbClrHsty_en
		//         0,             // u1_logo_blkRgClrHsty_en
		//         0,             // u1_logo_pixRgClrHsty_en
		//         0,             // u1_logo_delogoGlbClr_en
		//         0,             // u1_logo_delogoRgClr_en
		//         0,             // u1_logo_delogoGlbClrHsty_en
		//         0,             // u1_logo_delogoRgClrHsty_en
		//         1,             // u1_logo_SCalphaClr_en
		//         1,             // u1_logo_SCHstyClr_en
		//         1,             // u1_logo_SCLogoClr_en
		//         215,           // u8_clr_rg_thr
		//         60,            // u8_clr_glb_thr
		//         10,            // u8_dynBlkTh_cnt
		//         10,            // u8_dynPixTh_cnt
		//         10,            // u8_dynBlkStep_cnt
		//         10,            // u8_dynPixStep_cnt
		//         15,            // u16_dynDelogo_mv_th
		//         180,           // u8_dynDelogo_rat_th
		//         5,             // u6_rgClr_holdtime
		//         5,             // u6_GlbClr_holdtime
		//         1,             // u6_SCalpha_holdtime
		//         1,             // u6_SCclrHsty_holdtime
		//         1,             // u6_SCclrlogo_holdtime
		//         215,           // u8_pixRgClr_thr
		//         67,            // u8_pixGlbClr_thr
		//         10,            // u8_dynY_cnt
		//         215,           // u8_delogoRgclr_thr
		//         67,            // u8_delogoGlbClr_thr
		//         31,            // u5_reg_iir_alpha
		//         0,             // u32_pxRgYDf_bypass_0_31
		//         1,             // u4_blklogo_pStep
		//         1,             // u4_pixlogo_pStep
		//         2,             // u4_reg_merge_type
		//         55,            // u8_cmo_clear_thr
		//         240,           // u32_cmo_clear_idx
		//         20,            // u8_gg_mvd12_thr
		//         10,            // u8_gg_mvx_thr
		//         10,            // u8_gg_mvy_thr
		//         100,           // u8_gg_alpha_thr
		//         32000,         // u32_gg_sad_thr
		//         28,            // u8_gg_num_thr
		//         0,             // u8_reg_mvd
		//         0,             // u8_reg_tmv
		//         0,             // u1_printf_en
		//         1,             // u1_blkHsty_upmet
		//         15,            // u4_logo_half_clr_en
		//         28,            // u8_clr_half_thr
	},

	/*sceneAnalysis:_param_frc_sceneAnalysis*/
	{
		0,            //  u1_fadeInOut_en
		1,            //  u1_fdIO_acdc_cnt_en
		1,            //  u1_fdIO_acdc_ratio_en
		1,            //  u1_fdIO_bv_sad_en
		1,            //  u1_fdIO_apl_diff_en
		1,            //  u1_fdIO_conf_iir_en
		1,            //  u1_fdIO_sc_clear_en

		1,        //  u10_fdIO_apl_diff_thr_low
		3,       //  u10_fdIO_apl_diff_thr_hig
		15,      //  u8_fdIO_acdc_cnt_alpha_low
		128,          //  u8_fdIO_acdc_cnt_alpha_hig
		20,           //  u31_fdIO_acdc_dcsad_thr
		30,           //  u8_fdIO_acdc_sad_ratio_low
		150,          //  u8_fdIO_acdc_sad_ratio_hig
		40,           //  u16_fdIO_bv_sad_low
		80,           //  u16_fdIO_bv_sad_hig
		8,        //  u4_fdIO_conf_iir_alpha
        8,            // u4_fadeInOut_dh_close_thr
        8,            // u4_fadeInOut_dh_close_thr

		0,        //u4_acdc_bld_holdtime
		0,       //u20_fdIO_acdc_cnt_low
		0,       //u20_fdIO_acdc_cnt_hig
		0,            //u3_fdIO_apl_diff_frm_cnt

		0,            //u4_dynME_acdc_bld_alpha_x1
		0,        //u4_dynME_acdc_bld_alpha_x2

		{
			0,        //u8_dynME_acdc_bld_x_com
			0,
			0,
		},
		{
			0,        //u6_dynME_acdc_bld_y_com
			0,
			0,
		},
		{
			0,       //u8_dynME_acdc_bld_x_fdIO
			0,
			0,
		},
		{
			0,       //u6_dynME_acdc_bld_y_fdIO
			0,
			0,
		},

		0,	         //u8_dynME_ac_pix_sad_limt_com
		0,	         //u8_dynME_ac_pix_sad_limt_fdIO

		0,           //u6_dynME_ac_dc_bld_apl_holdfrm

		0,	         //u4_acdc_bld_psad_shift_com
		0,	         //u4_acdc_bld_psad_shift_fdIO

		0,           //u1_acdc_bld_sc_clear_en

		0,	         //u1_dynSR_en;
		0,	         //u11_dynSR_rmv_x_thr0;
		0,	         //u10_dynSR_rmv_y_thr0;
		0,	         //u11_dynSR_rmv_x_thr1;
		0,	         //u10_dynSR_rmv_y_thr1;
		0,	         //u12_dynSR_rmv_cnt_thr0;
		0,	         //u12_dynSR_rmv_cnt_thr1;
		0,	         //u12_dynSR_rmv_unconf_thr0;
		0,	         //u12_dynSR_rmv_unconf_thr1;
		0,	         //u9_dynSR_mvx_range0;
		0,	         //u9_dynSR_mvx_range1;
		0,	         //u8_dynSR_mvy_range0;
		0,	         //u8_dynSR_mvy_range1;
		0,	         //u6_dynSR_rmv_x_num_thr0;
		0,	         //u6_dynSR_rmv_x_num_thr1;
		0,	         //u6_dynSR_rmv_y_num_thr0;
		0,	         //u6_dynSR_rmv_y_num_thr1;

		0,           //unsigned char	 u1_dyn_SAD_Diff_en;
                //=========  Dynamic_SAD_Diff
		0,           //unsigned char     u8_dyn_SADGain_thr;
		0,           //unsigned int	 u27_dyn_SAD_Diff_TC_th_l;
		0,           //unsigned int	 u27_dyn_SAD_Diff_TC_th_h;
		0,           //unsigned int	 u8_dyn_SAD_Diff_TC_gain_min;
		0,           //unsigned int	 u8_dyn_SAD_Diff_TC_gain_max;
		0,           //unsigned int	 u8_dyn_SAD_Diff_sadAll_gain_min;
		0,           //unsigned int	 u8_dyn_SAD_Diff_sadAll_gain_max;
			//========================================
		0,           //unsigned int	 u28_dyn_SAD_Diff_sadAll_th_l;
		0,           //unsigned int	 u28_dyn_SAD_Diff_sadAll_th_h;
		0,	 //u4_dyn_SAD_Diff_rmv_ratio_th_l;
		0,	 //u4_dyn_SAD_Diff_rmv_ratio_th_h;
		0,	 //u6_dyn_SAD_Diff_gmv_ratio_th;

		0,           //u1_RgPan_Detect_en;
		0,           //u16_RgPan_rMV_th_h;
		0,           //u16_RgPan_rMV_th_l;

		0,           //u22_RgPan_rSC_th_h;
		0,           //u22_RgPan_rSC_th_l;

		0,           //u22_RgPan_rTC_th_h;
		0,           //u22_RgPan_rTC_th_l;

		0,           //u5_RgPan_alp_th;

		0,            //unsigned char s2m_goodmv_thr;
		0,            //unsigned char s2m_goodmv_unconf;
		0,            //unsigned char s2m_diffmv_thr;
		0,            //	unsigned char   s2m_cnt_iir_alpha;
		0,            //s2m_method_sel

		0,		//u1_zoom_detect_en
		0,		//u1_mc2_var_lpf_wrt_en

		0,		//unsigned short u8_hpan_th;
		0,		//unsigned short u8_vpan_th;
		0,		//unsigned short u8_cnt2_th;
		0,		//unsigned int u25_glb_dtl_th;
		0,		//unsigned char u1_fastMotion_gfb_en;
		0,		//unsigned char u8_fastMotion_glbfb_min;
		0,		//unsigned char u1_fastMotion_lfb_en;
		0,		//unsigned char u6_localfallback_lvl;
		0,		//unsigned char u1_fastMotion_det_en;

		0,		//unsigned char u1_special_scfb_en;

		0,		//u1_zmv_adapt_pnt_en;
		0,		//u11_zmv_adapt_pnt_GMV_mvx_thr;
		0,		//u10_zmv_adapt_pnt_GMV_mvy_thr;
		0,		//u13_zmv_adapt_pnt_GMV_Small_Y1;
		0,		//u13_zmv_adapt_pnt_GMV_Big_Y1;

		0,		//unsigned char u1_me1_gmvd_Sel_en;
		0,		//unsigned char u11_me1_gmvd_Sel_mvx_diff_thr;
		0,		//unsigned char u10_me1_gmvd_Sel_mvy_diff_thr;
		0,		//unsigned char u4_me1_gmvd_Sel_1st_gmv_ratio_thr;
		0,		//unsigned char u4_me1_gmvd_Sel_2nd_gmv_ratio_thr;

		0,		//	unsigned char  u1_SA_en;
		0,		//	unsigned int u20_SA_good_rgn_SAD_th;
		0,		//	unsigned int u12_SA_good_rgn_unconf_th;
		0,		//	unsigned int u12_SA_good_rgn_TC_th;
		0,		//	unsigned char  u5_SA_good_rgn_num_th;
		0,		//	unsigned char  u5_SA_bad_rgn_num_th;
		0,		//	unsigned char  u4_SA_good_scene_enter_cnt;
		0,		//	unsigned char  u4_SA_bad_scene_enter_cnt;
		0,		//	unsigned int u20_SA_bad_rgn_SAD_th;
		0,		//	unsigned int u12_SA_bad_rgn_unconf_th;
		0,		//	unsigned int u12_SA_bad_rgn_TC_th;
		0,		//	unsigned char  u3_SA_bad_scene_keep_cnt;
		0,		//  unsigned char  u8_SC_PFV_CDD_hold_cnt
		0,  //u1_mv_accord_en;
		0,  //u6_mv_accord_hold_cnt;
		0,  //u6_mv_accord_mvx_thr;
		0,  //u5_mv_accord_mvy_thr;
		0,  //u6_mv_accord_gmv_ratio_thr;
	},

	// _param_me_sceneAnalysis
   	{
		1,		//  u1_RepeatPattern_en;
		1,		//  u1_Natural_HighFreq_en;
		0,		//  u1_LightSword_en
		0,		//  u1_BigAPLDiff_en
 		0x04,          // u16_casino_rimLft_th
		0xEFB,         // u16_casino_rimRgt_th
		0x106,         // u16_casino_rimUp_th
		0x769,         // u16_casino_rimDn_th
		0x15,          // u8_casino_rim_delta
		1,		//  u1_EPG_en
		1,		// u1_DSR_en, DynamicSearchRange_detect
		0,	// u1_over_searchrange_en
		0,	// u1_force_panning
		3,	// u8_over_searchrange_thl
		// ===== occl extension
		1,	// u1_occl_type2_enable;
		11,	// u8_occl_ext_bg_mv_diff_thl;
		0,	// u1_occl_ext_gmv_weight_mode;
		9,	// u4_occl_ext_2nd_gmv_uconf_en_thl;
		11,	// u4_occl_ext_2nd_gmv_uconf_dis_thl;
		0,	// u1_occl_ext_force_protection_off;
		0,	// u1_occl_ext_force_lvl_en;
		0,	// u3_occl_ext_force_lvl;
		12,	// u4_occl_ext_rgn_thl;

		// ===== post correction
		0,		// u1_dh_postcorr_log_en;
		0,		// u3_dh_postcorr_force_lvl;
		0,		// u1_dh_postcorr_force_lvl_en;
		1,		// u1_dh_postcorrt_dyn_calc_en;
		25,		// u8_dh_postcorr_LogoCount_thr;
		20,		// u8_dh_postcorr_GmvMvy_thr;
		200,	// u8_dh_postcorr_y2x_ratio_x0;
		125,	// u8_dh_postcorr_y2x_ratio_x1;
		128,	// u8_dh_postcorr_y2x_ratio_y0;
		76,		// u8_dh_postcorr_y2x_ratio_y1;

		// ===== main object motion
		13,	// u4_MainObjectMove_bgMv_thl;
		4,	// u4_MainObjectMove_objBgMvDiff_thl;
		24,	// u8_MainObjectMove_dtl_thl;
		22,	// u8_MainObjectMove_thl_1;
		14,	// u8_MainObjectMove_thl_2;
		34,	// u8_MainObjectMove_1st_uconf_thr;
		38,	// u8_MainObjectMove_2nd_uconf_thr;
	},

	/*Write-Register:_param_wrt_comreg*/
	{
		0,             // u1_mc_lbmcMode_en

		0,             // u1_FBLevelWr_en

		1,             // u1_RimCtrl_wrt_en
		0,             // u1_dhClose_wrt_en

		0,             // u1_LGClr_wrt_en
		0,             // u1_LGSC_ctrl_wrt_en

		0,             // u1_dynME_acdcBld_wrt_en
		0,             // u1_bigFBlvl_dis_SCdet_en

		0,             // u1_LGBlkHstyClr_wrt_en
		0,             // u1_LGPixHstyClr_wrt_en

		0,             //u1_LGHstyClr_ClrLG_wrt_en

		0,              //unsigned char u1_LGRg_dhclr_ctrl_wrt_en;
		0,              //unsigned char u1_LGRg_detect_adp_wrt_en;

		0,              //u1_LGRg_rim_unbalance_wrt_en

		0,             //u1_LG_SC_PtClose_wrt_en

		0,              //unsigned char   u8_logo_blksameth_l_bidx0_lg;
		0,              //unsigned char   u8_logo_blksameth_l_bidx0_com;

		0,              //unsigned char   u8_logo_blksameth_a_bidx0_lg;
		0,              //unsigned char   u8_logo_blksameth_a_bidx0_com;

		0,              //unsigned char   u8_logo_blksameth_a_bidx10_lg;
		0,              //unsigned char   u8_logo_blksameth_a_bidx10_com;

		0,		// unsigned char   u8_blksameth_a_bidx4;
		0,		// unsigned char   u8_blksameth_h_bidx4;

		0,              //unsigned char   u8_logo_blkhsty_pth_bidx10_lg;
		0,              //unsigned char   u8_logo_blkhsty_pth_bidx10_com;

		0,              //unsigned char   u8_logo_blkhsty_nth_bidx10_lg;
		0,              //unsigned char   u8_logo_blkhsty_nth_bidx10_com;

		0,              // u8_logo_mv_diff_diff_th_lg;
		0,              // u8_logo_mv_diff_sad_th_lg;
		0,              // u8_logo_tmv_sad_th_lg;
		0,              // u8_logo_tmv_num_th_lg;

		0,              // u1_LG_netflix_wrt_en

		0,              //u1_LG_lg16s_patch_wrt_en;
		0,		//u1_pql_debug_info_Show_en,
		//1,		//u1_pql_onefifth_intp_en
		//1,		//u1_pql_input_intp_en
		//1,		//u1_pql_output_intp_en

		1,		//unsigned char   u1_localfb_wrt_en;
		1,              //unsigned char  u1_scCtrl_wrt_en;
		1,              //unsigned char  u1_MECand_wrt_en;
		1,              //unsigned char  u1_MECandPnt_wrt_en;
		1,              //unsigned char  u1_GMV_wrt_en;
		1,              //unsigned char  u1_ME_ipme_flp_alp_wrt_en;
		1,              //unsigned char  u1_ME_mvd_cuv_wrt_en;
		1,              //unsigned char  u1_ME_AdptPnt_rnd_wrt_en;
		1,              //unsigned char  u1_logoCtrl_wrt_en;
		1,              //unsigned char  u1_MEacdcBld_wrt_en;

		0,              //u1_dh_accord_ctrl_en
		1,              //u1_ME1_DRP_en
		1,              //u1_ME_vst_en
		0,              //u1_Identification_Pattern_en

		1,		//  u1_ME_Cost_Ctrl
		1, 		//u1_Wrt_ComReg_Proc_en;
		1,      // u1_logo_sc_FastDet_wrt_en
		1,		//  u1_MEUpdStgh_wrt_en
		1,		//  u1_Unsharp_Mask_wrt_en
		1,		// u1_MCSobj_wrt_en
		1,		// u1_ME2Sobj_wrt_en
		1,		// u1_OcclExt_wrt_en
		1,		// u1_rFB_show_en
		1,		// u1_DW_fmdet_4region_En
		1,		// u1_mc_near_rim_blending_en
		1,		// u1_mc_hf_lpf_en
		1,		// u1_mc_localfb_en
		1,		// u1_logo_HaloEnahce_en
		1,		// u1_logo_DynOBME_en
		1,		// u1_logo_pxldfth_dyn_en
		1,		// u1_logo_pxlrgclr_en
		1,		// u1_logo_dhRgnProc_en
	},
	/*Patch manage:_param_Patch_Manage*/
	{
	},
	/*_PARAM_ME_VST_CTRL*/
	{
		1, // u1_update_hist_en
	},
};

_PQLPARAMETER *GetPQLParameter()
{
    return (_PQLPARAMETER *)&PQL_Glb_Param;
}
