#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_reg_def.h"
#include "memc_isr/PQL/CalibMEMCInfo.h"



/////////////////////////////////
VOID PQL_ContextProc_ReadParam(_PQLPARAMETER *pParam)
{
	unsigned int u32_RB_val;

	{
		//rim detect
		ReadRegister(HARDWARE_HARDWARE_00_reg, 0, 31, &u32_RB_val);
		pParam->_param_rimctrl.u1_RimCtrl_en      =  u32_RB_val        & 0x01;
		pParam->_param_rimctrl.u12_RimDiffTh      = (u32_RB_val >> 1)  & 0xFFF;
		pParam->_param_rimctrl.u8_RimBiggerTh     = (u32_RB_val >> 13) & 0x1F;
		pParam->_param_rimctrl.u8_RimSmallerTh    = (u32_RB_val >> 18) & 0x3F;
		pParam->_param_rimctrl.u8_RimIIRAlpha     = (u32_RB_val >> 24) & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_01_reg, 0, 31, &u32_RB_val);
		pParam->_param_rimctrl.u8_mc_ptRim_shrink   =  u32_RB_val        & 0xFF;
		pParam->_param_rimctrl.u8_me_pixRim_shrink  = (u32_RB_val >>  8) & 0xFF;
		pParam->_param_rimctrl.u8_LogoBlkRimHOffset = (u32_RB_val >> 16) & 0xFF;
		pParam->_param_rimctrl.u8_LogoBlkRimVOffset = (u32_RB_val >> 24) & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_02_reg, 0, 31, &u32_RB_val);
		pParam->_param_rimctrl.u8_LogoPixRimHOffset  =  u32_RB_val       & 0xFF;
		pParam->_param_rimctrl.u8_LogoPixRimVOffset  = (u32_RB_val >> 8) & 0xFF;
		pParam->_param_rimctrl.u8_lbme_pixRim_shrink = (u32_RB_val >> 16) & 0xFF;
		pParam->_param_rimctrl.u8_lbme_pixRim_comp 	 = (u32_RB_val >> 24) & 0xFF;
	}

	{
		// lbmc mode switch
		ReadRegister(HARDWARE_HARDWARE_03_reg, 0, 31, &u32_RB_val);
		pParam->_param_mc_lbmcswitch.u8_lbmcSwitch_bypass  =  u32_RB_val        & 0xFF;
		pParam->_param_mc_lbmcswitch.u8_lf_Ovlap_s2n_cntTh = (u32_RB_val >>  8) & 0xFF;
		pParam->_param_mc_lbmcswitch.u8_lf_Lmv_s2n_cntTh   = (u32_RB_val >> 16) & 0xFF;
		pParam->_param_mc_lbmcswitch.u8_lf_Ovlap_n2s_cntTh = (u32_RB_val >> 24) & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_04_reg, 0, 31, &u32_RB_val);
		pParam->_param_mc_lbmcswitch.u8_lf_Lmv_n2s_cntTh   =  u32_RB_val        & 0xFF;
		pParam->_param_mc_lbmcswitch.u8_hf_Ovlap_s2n_cntTh = (u32_RB_val >>  8) & 0xFF;
		pParam->_param_mc_lbmcswitch.u8_hf_Lmv_s2n_cntTh   = (u32_RB_val >> 16) & 0xFF;
		pParam->_param_mc_lbmcswitch.u8_hf_Ovlap_n2s_cntTh = (u32_RB_val >> 24) & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_05_reg, 0, 31, &u32_RB_val);
		pParam->_param_mc_lbmcswitch.u8_hf_Lmv_n2s_cntTh   =  u32_RB_val        & 0xFF;
//		pParam->_param_Patch_Manage.u1_BluceMic_En = 		 (u32_RB_val >>  8) & 0x01;
	}

	{
		//dh control
		ReadRegister(HARDWARE_HARDWARE_06_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u2_dhClose_bypass		=  u32_RB_val        & 0x03;
		pParam->_param_dh_close.u1_pfv_cdd_num_en		= (u32_RB_val >> 2)  & 0x01;
		//pParam->_param_dh_close.u1_dbg_print_en		= (u32_RB_val >> 2)  & 0x01;
		pParam->_param_dh_close.u1_fblvl_en				= (u32_RB_val >> 3)  & 0x01;
		pParam->_param_dh_close.u1_meAllDtl_en			= (u32_RB_val >> 4)  & 0x01;
		pParam->_param_dh_close.u5_phfv0_candi_num		= (u32_RB_val >> 5)  & 0x1F;
		pParam->_param_dh_close.u5_phfv1_candi_num		= (u32_RB_val >> 10) & 0x1F;
		pParam->_param_dh_close.u9_pfv_cdd_num_dtl_thr	= (u32_RB_val >> 15) & 0x1FF;
		pParam->_param_dh_close.u8_dhOff_holdFrm		= (u32_RB_val >> 24) & 0x1F;
		pParam->_param_dh_close.u1_dbg_print_en			= (u32_RB_val >> 29) & 0x01;

		ReadRegister(HARDWARE_HARDWARE_07_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u8_fblvl_holdFrm   =  u32_RB_val       & 0xFF;
		pParam->_param_dh_close.u8_fbLvl_Th0       = (u32_RB_val >> 8) & 0xFF;
		pParam->_param_dh_close.u8_fbLvl_Th1       = (u32_RB_val >>16) & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_SC_PFV_CDD_hold_cnt	= (u32_RB_val >> 24) & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_08_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u25_meAllDtl_Th0  = u32_RB_val   & 0x1FFFFFF;

		ReadRegister(HARDWARE_HARDWARE_09_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u25_meAllDtl_Th1  = u32_RB_val   & 0x1FFFFFF;

		ReadRegister(HARDWARE_HARDWARE_10_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u1_dh_pfvconf_en      =  u32_RB_val         & 0x01;
		pParam->_param_dh_close.u20_dh_pfvconf_thrL   = (u32_RB_val >>  1)  & 0xFFFFF;
		pParam->_param_dh_close.u6_dh_pfvconf_cnt_thr = (u32_RB_val >> 21)  & 0x3F;
		pParam->_param_dh_close.u5_dh_pfvconf_holdfrm = (u32_RB_val >> 27)  & 0x1F;

		//LBMC param read
		ReadRegister(HARDWARE_HARDWARE_11_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u1_lbmc_switch_en		= u32_RB_val & 1;
		pParam->_param_dh_close.u5_lbmc_switch_holdfrm	= (u32_RB_val >>  1) & 0xFF;
		pParam->_param_dh_close.u5_dh_fadeInOut_holdfrm	= (u32_RB_val >>  9) & 0x1F;
		pParam->_param_dh_close.u1_dh_fadeInOut_en		= (u32_RB_val >>  14) & 0x01;
		//sml rgnAction
		pParam->_param_dh_close.u1_dh_sml_rgnAction_en			= (u32_RB_val >>  15) & 0x01;
		pParam->_param_dh_close.u8_dh_sml_rgnAction_gmv_x_thr	= (u32_RB_val >>  16) & 0xFF;
		pParam->_param_dh_close.u8_dh_sml_rgnAction_gmv_y_thr	= (u32_RB_val >>  24) & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_12_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u19_dh_sml_rgnAction_good_pfvdiff_thr = u32_RB_val          & 0x7FFFF;
		pParam->_param_dh_close.u8_dh_sml_rgnAction_apl_thr = (u32_RB_val >>  19)           & 0xFF;
		pParam->_param_dh_close.u5_dh_sml_rgnAction_badrgn_cnt_thr = (u32_RB_val >>  27)    & 0x1F;

		ReadRegister(HARDWARE_HARDWARE_13_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u5_dh_sml_rgnAction_goodrgn_cnt_thr = u32_RB_val            & 0x1F;
		pParam->_param_dh_close.u10_dh_sml_rgnAction_goodrgnsad_thr = (u32_RB_val >>  5)    & 0x3FF;
		pParam->_param_dh_close.u5_dh_sml_rgnAction_holdfrm = (u32_RB_val >>  15)           & 0x1F;
		pParam->_param_dh_close.u12_dh_sml_rgnAction_gmv_cnt_thr = (u32_RB_val >>  20)      & 0xFFF;

		ReadRegister(HARDWARE_HARDWARE_14_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u12_dh_sml_rgnAction_gmv_ucof_thr = u32_RB_val              & 0xFFF;
		pParam->_param_dh_close.u19_dh_sml_rgnAction_bad_pfvdiff_thr = (u32_RB_val >>  12)  & 0x7FFFF;

		ReadRegister(HARDWARE_HARDWARE_15_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u10_dh_sml_rgnAction_badrgnsad_thr = u32_RB_val             & 0x3FF;
		pParam->_param_dh_close.u12_dh_sml_rgnAction_bad_gmv_cnt_thr = (u32_RB_val >>  10)  & 0xFFF;
		pParam->_param_frc_sceneAnalysis.u10_zmv_adapt_pnt_GMV_mvy_thr	= (u32_RB_val >>  22)  & 0x3FF;

		ReadRegister(HARDWARE_HARDWARE_16_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u12_dh_sml_rgnAction_bad_gmv_ucof_thr = u32_RB_val          & 0xFFF;
		//pParam->_param_dh_close.u1_dbg_print_en 					= (u32_RB_val >> 12) 		& 0x01;
		//pParam->_param_dh_close.u5_panning_holdfrm 				= (u32_RB_val >> 13) 		& 0x1F;
		//pParam->_param_frc_sceneAnalysis.u12_PanRMV_Cnt 	    = (u32_RB_val >> 18) 		& 0xFFF;

		//ReadRegister(FRC_TOP__PQL_0__pql_scAls_PanRMV_SmallMV_ADDR, 0, 31, &u32_RB_val);
		//pParam->_param_frc_sceneAnalysis.u10_PanRMV_SmallMV		= u32_RB_val		 		& 0x3FF;
		//pParam->_param_frc_sceneAnalysis.u10_PanRMV_LargeMV		= (u32_RB_val >> 10) 		& 0x3FF;
		//pParam->_param_frc_sceneAnalysis.u12_PanRMV_Unconf		= (u32_RB_val >> 20) 		& 0xFFF;
		pParam->_param_frc_sceneAnalysis.u11_zmv_adapt_pnt_GMV_mvx_thr	= (u32_RB_val >> 12) & 0x7FF;
		pParam->_param_frc_sceneAnalysis.u1_zmv_adapt_pnt_en			= (u32_RB_val >> 23) & 0x01;
		pParam->_param_dh_close.u2_dh_me1_ip_dc_obme_mode_off			= (u32_RB_val >> 24) & 0x03; // 3
		pParam->_param_dh_close.u2_dh_me1_pi_dc_obme_mode_off			= (u32_RB_val >> 26) & 0x03; // 3
		pParam->_param_dh_close.u2_dh_me1_ip_ac_obme_mode_off			= (u32_RB_val >> 28) & 0x03; // 0
		pParam->_param_dh_close.u2_dh_me1_pi_ac_obme_mode_off			= (u32_RB_val >> 30) & 0x03; // 0

		ReadRegister(HARDWARE_HARDWARE_17_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u13_zmv_adapt_pnt_GMV_Small_Y1 = u32_RB_val		 & 0x1FFF;
		pParam->_param_frc_sceneAnalysis.u13_zmv_adapt_pnt_GMV_Big_Y1	= (u32_RB_val >> 13) & 0x1FFF;
		pParam->_param_dh_close.u2_dh_me1_ip_dc_obme_mode_on			= (u32_RB_val >> 26) & 0x03; // 1
		pParam->_param_dh_close.u2_dh_me1_pi_dc_obme_mode_on			= (u32_RB_val >> 28) & 0x03; // 1
		pParam->_param_dh_close.u2_dh_me1_ip_ac_obme_mode_on			= (u32_RB_val >> 30) & 0x03; // 1

		ReadRegister(HARDWARE_HARDWARE_18_reg, 0, 31, &u32_RB_val);
		pParam->_param_dh_close.u2_dh_me1_pi_ac_obme_mode_on				= u32_RB_val		 & 0x03; // 1
		pParam->_param_frc_sceneAnalysis.u1_me1_gmvd_sel_en					= (u32_RB_val >>  2) & 0x01;
		pParam->_param_frc_sceneAnalysis.u11_me1_gmvd_sel_mvx_diff_thr		= (u32_RB_val >>  3) & 0x7FF;
		pParam->_param_frc_sceneAnalysis.u10_me1_gmvd_sel_mvy_diff_thr		= (u32_RB_val >> 14) & 0x3FF;
		pParam->_param_frc_sceneAnalysis.u4_me1_gmvd_sel_1st_gmv_ratio_thr 	= (u32_RB_val >> 24) & 0x0F;
		pParam->_param_frc_sceneAnalysis.u4_me1_gmvd_sel_2nd_gmv_ratio_thr 	= (u32_RB_val >> 28) & 0x0F;

		ReadRegister(HARDWARE_HARDWARE_19_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u20_SA_good_rgn_SAD_th 	= u32_RB_val  		 & 0xFFFFF;
		pParam->_param_frc_sceneAnalysis.u12_SA_good_rgn_unconf_th 	= (u32_RB_val >> 20) & 0xFFF;

		ReadRegister(HARDWARE_HARDWARE_20_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u20_SA_bad_rgn_SAD_th 		= u32_RB_val		 & 0xFFFFF;
		pParam->_param_frc_sceneAnalysis.u12_SA_bad_rgn_unconf_th	= (u32_RB_val >> 20) & 0xFFF;

		ReadRegister(HARDWARE_HARDWARE_21_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u12_SA_good_rgn_TC_th		= u32_RB_val		 & 0xFFF;
		pParam->_param_frc_sceneAnalysis.u12_SA_bad_rgn_TC_th		= (u32_RB_val >> 12) & 0xFFF;
		pParam->_param_frc_sceneAnalysis.u4_SA_good_scene_enter_cnt	= (u32_RB_val >> 24) & 0x00F;
		pParam->_param_frc_sceneAnalysis.u4_SA_bad_scene_enter_cnt	= (u32_RB_val >> 28) & 0x00F;

		ReadRegister(HARDWARE_HARDWARE_22_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u5_SA_good_rgn_num_th		= u32_RB_val		 & 0x1F;
		pParam->_param_frc_sceneAnalysis.u5_SA_bad_rgn_num_th		= (u32_RB_val >> 05) & 0x1F;
		pParam->_param_frc_sceneAnalysis.u3_SA_bad_scene_keep_cnt	= (u32_RB_val >> 10) & 0x07;
		pParam->_param_frc_sceneAnalysis.u1_SA_en					= (u32_RB_val >> 13) & 0x01;
		pParam->_param_frc_sceneAnalysis.u1_mv_accord_en			= (u32_RB_val >> 14) & 0x01;
		pParam->_param_frc_sceneAnalysis.u6_mv_accord_mvx_thr		= (u32_RB_val >> 15) & 0x3F;
		pParam->_param_frc_sceneAnalysis.u5_mv_accord_mvy_thr		= (u32_RB_val >> 21) & 0x1F;
		pParam->_param_frc_sceneAnalysis.u6_mv_accord_gmv_ratio_thr	= (u32_RB_val >> 26) & 0x3F;

		ReadRegister(HARDWARE_HARDWARE_23_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u6_mv_accord_hold_cnt		= u32_RB_val		 & 0x3F;
	}

	// me scene analysis
	{
#if 1
		ReadRegister(HARDWARE_HARDWARE_26_reg, 0, 31, &u32_RB_val);
		//pParam->_param_frc_sceneAnalysis.u20_fdIO_acdc_cnt_low        =  u32_RB_val        & 0xFFFFF;

		ReadRegister(HARDWARE_HARDWARE_27_reg, 0, 31, &u32_RB_val);
		//pParam->_param_frc_sceneAnalysis.u20_fdIO_acdc_cnt_hig        =  u32_RB_val			& 0xFFFFF;
		//pParam->_param_frc_sceneAnalysis.u3_fdIO_apl_diff_frm_cnt     =  (u32_RB_val >> 20)	& 0x0F;
		pParam->_param_frc_sceneAnalysis.u5_fadeInOut_logo_close_thr  =  (u32_RB_val >> 24) & 0x1F;
#endif
#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		ReadRegister(HARDWARE_HARDWARE_28_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u1_fadeInOut_en              =   u32_RB_val        & 0x01;
#endif
#if 1
		pParam->_param_frc_sceneAnalysis.u1_fdIO_acdc_cnt_en          =  (u32_RB_val >> 1)  & 0x01;
		pParam->_param_frc_sceneAnalysis.u1_fdIO_acdc_ratio_en        =  (u32_RB_val >> 2)  & 0x01;
		pParam->_param_frc_sceneAnalysis.u1_fdIO_bv_sad_en            =  (u32_RB_val >> 3)  & 0x01;
		pParam->_param_frc_sceneAnalysis.u1_fdIO_apl_diff_en          =  (u32_RB_val >> 4)  & 0x01;
		pParam->_param_frc_sceneAnalysis.u1_fdIO_conf_iir_en          =  (u32_RB_val >> 5)  & 0x01;
		pParam->_param_frc_sceneAnalysis.u1_fdIO_sc_clear_en          =  (u32_RB_val >> 6)  & 0x01;
		pParam->_param_frc_sceneAnalysis.u5_fadeInOut_dh_close_thr    =  (u32_RB_val >> 7)  & 0x1F;
		pParam->_param_frc_sceneAnalysis.u10_fdIO_apl_diff_thr_low    =  (u32_RB_val >>12)  & 0x3FF;
		pParam->_param_frc_sceneAnalysis.u10_fdIO_apl_diff_thr_hig    =  (u32_RB_val >>22)  & 0x3FF;

		ReadRegister(HARDWARE_HARDWARE_29_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u8_fdIO_acdc_cnt_alpha_low    =  u32_RB_val         & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_fdIO_acdc_cnt_alpha_hig    = (u32_RB_val >>  8)  & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_fdIO_acdc_sad_ratio_low    = (u32_RB_val >> 16)  & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_fdIO_acdc_sad_ratio_hig    = (u32_RB_val >> 24)  & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_30_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u31_fdIO_acdc_dcsad_thr    =  u32_RB_val   & 0xFFFFFFFF;

		ReadRegister(HARDWARE_HARDWARE_31_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u16_fdIO_bv_sad_low    =   u32_RB_val         & 0xFFFF;
		pParam->_param_frc_sceneAnalysis.u16_fdIO_bv_sad_hig    =  (u32_RB_val >> 16)  & 0xFFFF;
#endif
#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		ReadRegister(HARDWARE_HARDWARE_32_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u4_fdIO_conf_iir_alpha       =   u32_RB_val         & 0x0F;
		pParam->_param_frc_sceneAnalysis.u4_acdc_bld_holdtime         =  (u32_RB_val >>  4)  & 0x0F;
		pParam->_param_frc_sceneAnalysis.u8_dynME_acdc_bld_x_com[0]   =  (u32_RB_val >>  8)  & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_dynME_acdc_bld_x_com[1]   =  (u32_RB_val >> 16)  & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_dynME_acdc_bld_x_com[2]   =  (u32_RB_val >> 24)  & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_33_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u6_dynME_acdc_bld_y_com[0]   =   u32_RB_val         & 0xFF;
		pParam->_param_frc_sceneAnalysis.u6_dynME_acdc_bld_y_com[1]   =  (u32_RB_val >>  8)  & 0xFF;
		pParam->_param_frc_sceneAnalysis.u6_dynME_acdc_bld_y_com[2]   =  (u32_RB_val >> 16)  & 0xFF;          ;
		pParam->_param_frc_sceneAnalysis.u8_dynME_acdc_bld_x_fdIO[0]  =  (u32_RB_val >> 24)  & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_34_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u8_dynME_acdc_bld_x_fdIO[1]  =   u32_RB_val         & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_dynME_acdc_bld_x_fdIO[2]  =  (u32_RB_val >>  8)  & 0xFF;
		pParam->_param_frc_sceneAnalysis.u6_dynME_acdc_bld_y_fdIO[0]  =  (u32_RB_val >> 16)  & 0xFF;
		pParam->_param_frc_sceneAnalysis.u6_dynME_acdc_bld_y_fdIO[1]  =  (u32_RB_val >> 24)  & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_35_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u6_dynME_acdc_bld_y_fdIO[2]  =   u32_RB_val         & 0xFF;
#endif
		ReadRegister(HARDWARE_HARDWARE_36_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u1_dynSR_en              =    u32_RB_val           & 0x01;
		pParam->_param_frc_sceneAnalysis.u11_dynSR_rmv_x_thr0     =   (u32_RB_val >>   1)   & 0x7FF;
		pParam->_param_frc_sceneAnalysis.u10_dynSR_rmv_y_thr0     =   (u32_RB_val >>  12)   & 0x3FF;
		pParam->_param_frc_sceneAnalysis.u10_dynSR_rmv_y_thr1     =   (u32_RB_val >>  22)   & 0x3FF;

		ReadRegister(HARDWARE_HARDWARE_37_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u11_dynSR_rmv_x_thr1     =    u32_RB_val           & 0x7FF;
		pParam->_param_frc_sceneAnalysis.u12_dynSR_rmv_cnt_thr0   =   (u32_RB_val >>  11)   & 0xFFF;
		pParam->_param_frc_sceneAnalysis.u9_dynSR_mvx_range0      =   (u32_RB_val >>  23)   & 0x1FF;

		ReadRegister(HARDWARE_HARDWARE_38_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u12_dynSR_rmv_cnt_thr1     =    u32_RB_val           & 0xFFF;
		pParam->_param_frc_sceneAnalysis.u12_dynSR_rmv_unconf_thr0  =   (u32_RB_val >>  12)   & 0xFFF;
		pParam->_param_frc_sceneAnalysis.u8_dynSR_mvy_range0        =   (u32_RB_val >>  24)   & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_39_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u12_dynSR_rmv_unconf_thr1		=    u32_RB_val           & 0xFFF;
		pParam->_param_frc_sceneAnalysis.u9_dynSR_mvx_range1			=   (u32_RB_val >>  12)   & 0x1FF;
		pParam->_param_frc_sceneAnalysis.u8_dynSR_mvy_range1			=   (u32_RB_val >>  21)   & 0xFF;
		pParam->_param_frc_sceneAnalysis.u1_dyn_SAD_Diff_en				=	(u32_RB_val >>  29)   & 0x01;

		ReadRegister(HARDWARE_HARDWARE_40_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u6_dynSR_rmv_x_num_thr0     =    u32_RB_val           & 0x3F;
		pParam->_param_frc_sceneAnalysis.u6_dynSR_rmv_x_num_thr1     =   (u32_RB_val >>   6)   & 0x3F;
		pParam->_param_frc_sceneAnalysis.u6_dynSR_rmv_y_num_thr0     =   (u32_RB_val >>  12)   & 0x3F;
		pParam->_param_frc_sceneAnalysis.u6_dynSR_rmv_y_num_thr1     =   (u32_RB_val >>  18)   & 0x3F;
		pParam->_param_frc_sceneAnalysis.u6_dyn_SAD_Diff_gmv_ratio_th	=   (u32_RB_val >>  24)   & 0x3F;

		ReadRegister(HARDWARE_HARDWARE_41_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u28_dyn_SAD_Diff_sadAll_th_l	=	u32_RB_val & 0xFFFFFFF;
		pParam->_param_frc_sceneAnalysis.u4_dyn_SAD_Diff_rmv_ratio_th_l	=	(u32_RB_val >> 28) & 0x0F;

		ReadRegister(HARDWARE_HARDWARE_42_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u28_dyn_SAD_Diff_sadAll_th_h	=	u32_RB_val & 0xFFFFFFF;
		pParam->_param_frc_sceneAnalysis.u4_dyn_SAD_Diff_rmv_ratio_th_h	=	(u32_RB_val >> 28) & 0x0F;

		ReadRegister(HARDWARE_HARDWARE_46_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u4_dynME_acdc_bld_alpha_x1    = u32_RB_val				& 0x0F;
		pParam->_param_frc_sceneAnalysis.u4_dynME_acdc_bld_alpha_x2    = (u32_RB_val >>  4)		& 0x0F;
		pParam->_param_frc_sceneAnalysis.u8_dynME_ac_pix_sad_limt_com  = (u32_RB_val >>  8)		& 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_dynME_ac_pix_sad_limt_fdIO = (u32_RB_val >>  16)	& 0xFF;

		ReadRegister(HARDWARE_HARDWARE_47_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u6_dynME_ac_dc_bld_apl_holdfrm = u32_RB_val			& 0xFF;
		pParam->_param_frc_sceneAnalysis.u4_acdc_bld_psad_shift_com 	= (u32_RB_val >> 8)		& 0x0F;
		pParam->_param_frc_sceneAnalysis.u4_acdc_bld_psad_shift_fdIO 	= (u32_RB_val >> 12)	& 0x0F;
		pParam->_param_frc_sceneAnalysis.u1_acdc_bld_sc_clear_en 		= (u32_RB_val >> 16)	& 0x01;
//		pParam->_param_frc_sceneAnalysis.u15_PanRMV_FitNumThr 			= (u32_RB_val >> 17)	& 0x7FFF;

		ReadRegister(HARDWARE_HARDWARE_48_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u8_hpan_th 		  		= u32_RB_val  		 & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_vpan_th 		  		= (u32_RB_val >> 8)  & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_cnt2_th 		  		= (u32_RB_val >> 16) & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_fastMotion_glbfb_min	= (u32_RB_val >> 24) & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_49_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u1_fastMotion_det_en 		= u32_RB_val	    & 0x01;
		pParam->_param_frc_sceneAnalysis.u1_fastMotion_gfb_en 		= (u32_RB_val >> 1) & 0x01;
		pParam->_param_frc_sceneAnalysis.u25_glb_dtl_th 			= (u32_RB_val >> 2) & 0xFFFF;
		pParam->_param_rimctrl.u8_RimSmallerSCTh 			= (u32_RB_val >> 18) & 0xFF;
		pParam->_param_rimctrl.u5_RimLRSmallShift 			= (u32_RB_val >> 26) & 0x1F;

		ReadRegister(HARDWARE_HARDWARE_51_reg, 0, 31, &u32_RB_val);
		pParam->_param_Patch_Manage.u8_RidingHorse_Uamount_thr_gain =	u32_RB_val 		   & 0xFF;
		pParam->_param_Patch_Manage.u8_RidingHorse_Vamount_thr_gain =	(u32_RB_val >>  8) & 0xFF;
		pParam->_param_Patch_Manage.u8_RidingHorse_mvx_thr 		 	=	(u32_RB_val >> 16) & 0xFF;
		pParam->_param_Patch_Manage.u7_RidingHorse_mvy_thr 		 	=	(u32_RB_val >> 24) & 0x7F;
		pParam->_param_Patch_Manage.u1_RidingHorse_Detect_en 		=	(u32_RB_val >> 31) & 0x01;

		ReadRegister(HARDWARE_HARDWARE_52_reg, 0, 31, &u32_RB_val);
		pParam->_param_Patch_Manage.u8_HF1_Hdtl_thr_gain =	u32_RB_val 		   & 0xFF;
		pParam->_param_Patch_Manage.u8_HF1_Vdtl_thr_gain =	(u32_RB_val >>  8) & 0xFF;
		pParam->_param_Patch_Manage.u8_HF1_mvx_thr 		 =	(u32_RB_val >> 16) & 0xFF;
		pParam->_param_Patch_Manage.u7_HF1_mvy_thr 		 =	(u32_RB_val >> 24) & 0x7F;
		pParam->_param_Patch_Manage.u1_HF1_Detect_en 	 =	(u32_RB_val >> 31) & 0x01;

		ReadRegister(HARDWARE_HARDWARE_53_reg, 0, 31, &u32_RB_val);
		pParam->_param_Patch_Manage.u8_HF2_Hdtl_thr_gain =	u32_RB_val 		   & 0xFF;
		pParam->_param_Patch_Manage.u8_HF2_Vdtl_thr_gain =	(u32_RB_val >>  8) & 0xFF;
		pParam->_param_Patch_Manage.u8_HF2_mvx_thr 		 =	(u32_RB_val >> 16) & 0xFF;
		pParam->_param_Patch_Manage.u7_HF2_mvy_thr 		 =	(u32_RB_val >> 24) & 0x7F;
		pParam->_param_Patch_Manage.u1_HF2_Detect_en 	 =	(u32_RB_val >> 31) & 0x01;

		ReadRegister(HARDWARE_HARDWARE_54_reg, 0, 31, &u32_RB_val);
		pParam->_param_Patch_Manage.u8_HF3_Hdtl_thr_gain =	u32_RB_val 		   & 0xFF;
		pParam->_param_Patch_Manage.u8_HF3_Vdtl_thr_gain =	(u32_RB_val >>  8) & 0xFF;
		pParam->_param_Patch_Manage.u8_HF3_mvx_thr 		 =	(u32_RB_val >> 16) & 0xFF;
		pParam->_param_Patch_Manage.u7_HF3_mvy_thr 		 =	(u32_RB_val >> 24) & 0x7F;
		pParam->_param_Patch_Manage.u1_HF3_Detect_en 	 =	(u32_RB_val >> 31) & 0x01;


#if 0
		ReadRegister(FRC_TOP__PQL_0__pql_scAls_avgRngTC_Thr_ADDR, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u32_avgRngTC_Thr    = u32_RB_val;

		ReadRegister(FRC_TOP__PQL_0__pal_scAls_avgRngSC_Thr_ADDR, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u32_avgRngSC_Thr    = u32_RB_val;

		ReadRegister(FRC_TOP__PQL_0__pql_rim_ME2_blkRimsht_ADDR, 0, 31, &u32_RB_val);
		pParam->_param_rimctrl.u8_ME2_blkRimsht  = (u32_RB_val )	& 0xFF;
		pParam->_param_rimctrl.u8_ME2_ignRim  = (u32_RB_val >>8)	& 0xFF;
		pParam->_param_rimctrl.u8_ME2_RimMode  = (u32_RB_val >>  16) & 0xFF;

		ReadRegister(FRC_TOP__PQL_0__pql_scAls_BdySmall_cnt_thr_ADDR, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u4_BdySmall_cnt_thr  = (u32_RB_val    ) & 0xFF;	 //8bit for quick tuning current
		pParam->_param_frc_sceneAnalysis.u8_BdySmall_mvx_thr  = (u32_RB_val >>8) & 0xFF;
		pParam->_param_frc_sceneAnalysis.u8_BdySmall_mvy_thr  = (u32_RB_val >>16)& 0xFF;

		ReadRegister(FRC_TOP__PQL_0__pql_scAls_BdySmall_sad_thr_ADDR, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u15_BdySmall_sad_thr  = (u32_RB_val    ) & 0xFFFF;
		pParam->_param_rimctrl.dyn_pan_oor_cnt  = (u32_RB_val >>16) & 0x7;
		pParam->_param_rimctrl.dyn_scbdy_oor_cnt  = (u32_RB_val >>19) & 0x7;
		pParam->_param_rimctrl.dyn_smlbdy_oor_cnt  = (u32_RB_val >>22) & 0x7;
		pParam->_param_rimctrl.dyn_general_oor_cnt  = (u32_RB_val >>25) & 0x7;	  //3bit
		pParam->_param_rimctrl.u1_dynRim_insidePan_en  = (u32_RB_val >>28) & 0x1;
		pParam->_param_rimctrl.u1_dynRim_ScBdy_en  = (u32_RB_val >>29) & 0x1;
		pParam->_param_rimctrl.u1_dynRim_SmlBdy_en  = (u32_RB_val >>30) & 0x1;

		ReadRegister(FRC_TOP__PQL_0__pql_rim_dyn_pan_gmv_min_ADDR, 0, 31, &u32_RB_val);
		pParam->_param_rimctrl.dyn_pan_gmv_min      = (u32_RB_val >> 0)  & 0x7F;  //7bit
		pParam->_param_rimctrl.dyn_pan_gmv_max      = (u32_RB_val >> 7)  & 0x7F;  //7bit
		pParam->_param_rimctrl.dyn_pan_me2prim_min  = (u32_RB_val >> 14) & 0xF;  //4bit
		pParam->_param_rimctrl.dyn_pan_me2prim_max  = (u32_RB_val >> 18) & 0x1F;   //5bit
		pParam->_param_rimctrl.dyn_pan_mcprim_min  = (u32_RB_val >> 23) & 0xF;  //4bit
		pParam->_param_rimctrl.dyn_pan_mcprim_max  = (u32_RB_val >> 27) & 0x1F;  //5bit
#endif
		ReadRegister(HARDWARE_HARDWARE_55_reg, 0, 31, &u32_RB_val);
		pParam->_param_Patch_Manage.u1_tri_ball_Issue_en = (u32_RB_val >> 15) & 0x1;
		pParam->_param_frc_sceneAnalysis.u8_dyn_SADGain_thr	=	(u32_RB_val >> 16) & 0xFF;

		ReadRegister(HARDWARE_HARDWARE_56_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.s2m_goodmv_thr     = (u32_RB_val >> 0)   & 0xFF;
		pParam->_param_frc_sceneAnalysis.s2m_goodmv_unconf  = (u32_RB_val >> 8)   & 0xFF;
		pParam->_param_frc_sceneAnalysis.s2m_diffmv_thr     = (u32_RB_val >> 16)  & 0xFF;
		pParam->_param_frc_sceneAnalysis.s2m_cnt_iir_alpha  = (u32_RB_val >> 24)  & 0x0F;
		pParam->_param_fblevelctrl.u1_gmvratio_use          = (u32_RB_val >> 28)  & 0x01;
		pParam->_param_frc_sceneAnalysis.s2m_method_sel     = (u32_RB_val >> 29)  & 0x01;
		pParam->_param_wrt_comreg.u1_scholdCtrl_wrt_en		= (u32_RB_val >> 30)  & 0x01;


#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		ReadRegister(HARDWARE_HARDWARE_57_reg, 0, 31, &u32_RB_val);
		pParam->_param_wrt_comreg.u1_dynamicME_en				= (u32_RB_val) & 0x1;	// u1_dynamicME_en
		pParam->_param_wrt_comreg.u1_Dh_MV_Corr_en				= (u32_RB_val >> 1)  & 0x1;	// u1_Dh_MV_Corr_en
		pParam->_param_read_comreg.u1_low_delay_test01    		= (u32_RB_val >> 2)  & 0x1;
		pParam->_param_dh_close.u1_simple_scene_en				= (u32_RB_val >> 3)  & 0x1;	 // u1_simple_scene_en
		pParam->_param_me_sceneAnalysis.u1_DSR_en				= (u32_RB_val >> 4)  & 0x1;	 // u1_DSR_en
		pParam->_param_me_sceneAnalysis.u1_RepeatPattern_en		= (u32_RB_val >> 5)  & 0x1;	 // u1_RepeatPattern_en
		pParam->_param_me_sceneAnalysis.u1_Natural_HighFreq_en	= (u32_RB_val >> 6)  & 0x1;	 // u1_Natural_HighFreq_en
		pParam->_param_wrt_comreg.u1_localfb_wrt_en				= (u32_RB_val >> 7)  & 0x1;  // u1_localfb_wrt_en
		pParam->_param_wrt_comreg.u1_ME_Cost_Ctrl				= (u32_RB_val >> 8)  & 0x1;  // u1_ME_Cost_Ctrl
		pParam->_param_wrt_comreg.u1_MEUpdStgh_wrt_en			= (u32_RB_val >> 9)  & 0x1;  // u1_MEUpdStgh_wrt_en
		pParam->_param_wrt_comreg.u1_Unsharp_Mask_wrt_en		= (u32_RB_val >> 10) & 0x1;  // u1_Unsharp_Mask_wrt_en
		pParam->_param_wrt_comreg.u1_MCSobj_wrt_en				= (u32_RB_val >> 11) & 0x1;  // u1_MCSobj_wrt_en
		pParam->_param_wrt_comreg.u1_ME2Sobj_wrt_en				= (u32_RB_val >> 12) & 0x1;  // u1_ME2Sobj_wrt_en
		pParam->_param_wrt_comreg.u1_MECandOfst_wrt_en			= (u32_RB_val >> 13) & 0x1;  // u1_MECandOfst_wrt_en;
		pParam->_param_wrt_comreg.u1_scCtrl_wrt_en				= (u32_RB_val >> 14) & 0x1;  // u1_scCtrl_wrt_en;
		pParam->_param_wrt_comreg.u1_MECand_wrt_en				= (u32_RB_val >> 15) & 0x1;  // u1_MECand_wrt_en;
		pParam->_param_wrt_comreg.u1_MECandPnt_wrt_en			= (u32_RB_val >> 16) & 0x1;  // u1_MECandPnt_wrt_en;
		pParam->_param_frc_sceneAnalysis.u1_mc2_var_lpf_wrt_en	= (u32_RB_val >> 17) & 0x1;  // u1_mc2_var_lpf_wrt_en
		pParam->_param_wrt_comreg.u1_ME_ipme_flp_alp_wrt_en		= (u32_RB_val >> 18) & 0x1;  // u1_MEScan_wrt_en;
		pParam->_param_wrt_comreg.u1_ME_mvd_cuv_wrt_en			= (u32_RB_val >> 19) & 0x1;  // u1_ME_mvd_cuv_wrt_en;
		pParam->_param_wrt_comreg.u1_ME_AdptPnt_rnd_wrt_en		= (u32_RB_val >> 20) & 0x1;  // u1_ME_AdptPnt_rnd_wrt_en;
		pParam->_param_dh_close.u1_panning_en					= (u32_RB_val >> 21) & 0x1;  // u1_panning_en;
		pParam->_param_frc_phtable.u1_Ph_delay_calc_en			= (u32_RB_val >> 22) & 0x01; // Ph_delay_calc_en
		pParam->_param_Patch_Manage.u1_patch_highFreq_en		= (u32_RB_val >> 23) & 0x01; // u1_patch_highFreq_en
		pParam->_param_wrt_comreg.u1_ME1_DRP_en					= (u32_RB_val >> 24) & 0x1;  // u1_ME1_DRP_en;
		pParam->_param_wrt_comreg.u1_MEacdcBld_wrt_en			= (u32_RB_val >> 25) & 0x1;  // u1_MEacdcBld_wrt_en;
		pParam->_param_wrt_comreg.u1_ME_vst_en					= (u32_RB_val >> 26) & 0x1;  // u1_ME_vst_en;
		pParam->_param_frc_sceneAnalysis.u1_special_scfb_en		= (u32_RB_val >> 27) & 0x01; // u1_special_scfb_en
		pParam->_param_wrt_comreg.u1_Identification_Pattern_en	= (u32_RB_val >> 28) & 0x01; // u1_Identification_Pattern_en
		pParam->_param_wrt_comreg.u1_logoCtrl_wrt_en			= (u32_RB_val >> 29) & 0x1;  // u1_logoCtrl_wrt_en;
		pParam->_param_Patch_Manage.u1_patch_M_en				= (u32_RB_val >> 30) & 0x01; // u1_patch_M_en;
		pParam->_param_wrt_comreg.u1_Wrt_ComReg_Proc_en			= (u32_RB_val >> 31) & 0x1;  // u1_Wrt_ComReg_en;

#endif

#if 0
		ReadRegister(FRC_TOP__PQL_0__pql_rim_dyn_scbdy_me2prim_ADDR, 0, 31, &u32_RB_val);
		pParam->_param_rimctrl.dyn_scbdy_me2prim      = (u32_RB_val >> 0)  & 0xF;
		pParam->_param_rimctrl.dyn_scbdy_mcprim       = (u32_RB_val >> 4)  & 0xF;
		pParam->_param_rimctrl.dyn_smlbdy_me2prim     = (u32_RB_val >> 8)  & 0xF;
		pParam->_param_rimctrl.dyn_smlbdy_mcprim      = (u32_RB_val >> 12) & 0xF;
#endif

		ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);
		//for FRC_RgPanning_Detect(), it doesn't use now.
		//pParam->_param_frc_sceneAnalysis.u1_RgPan_Detect_en 		= u32_RB_val 			& 0x01;//bit 0
		//pParam->_param_frc_sceneAnalysis.u5_RgPan_alp_th			= (u32_RB_val >> 1)		& 0xFF;//bit1~8

		ReadRegister(HARDWARE_HARDWARE_59_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u16_RgPan_rMV_th_l               =  u32_RB_val			    & 0xFFFF;
		pParam->_param_frc_sceneAnalysis.u16_RgPan_rMV_th_h               =  (u32_RB_val>>16	)	& 0xFFFF;

		ReadRegister(HARDWARE_HARDWARE_60_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u22_RgPan_rSC_th_l             =  u32_RB_val			    & 0xFFFFFFFF;

		ReadRegister(HARDWARE_HARDWARE_61_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u22_RgPan_rSC_th_h             =  u32_RB_val			    & 0xFFFFFFFF;

		ReadRegister(HARDWARE_HARDWARE_62_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u22_RgPan_rTC_th_l             =  u32_RB_val			    & 0x0FFFFFFF;

		//=== no-used ====
		//ReadRegister(HARDWARE_HARDWARE_63_reg, 0, 31, &u32_RB_val);
		//pParam->_param_frc_sceneAnalysis.u22_RgPan_rTC_th_h             =  u32_RB_val			    & 0x0FFFFFFF;

		ReadRegister(HARDWARE_HARDWARE_63_reg, 0, 31, &u32_RB_val);
		pParam->_param_wrt_comreg.u1_pql_debug_info_Show_en 				=  (u32_RB_val >> 28)		& 0x01;
	}

	{
		// film detect
		ReadRegister(SOFTWARE_SOFTWARE_00_reg, 0, 31, &u32_RB_val);
		pParam->_param_filmDetectctrl.u1_filmDet_bypass			=  u32_RB_val        & 0x01;
		pParam->_param_filmDetectctrl.u32_filmDet_cadence_en      = (u32_RB_val >>  1) & 0x7FFF;
		pParam->_param_filmDetectctrl.u1_FrcCadSupport_En		= (u32_RB_val >> 16) & 0x1;
		pParam->_param_filmDetectctrl.u3_DbgPrt_Rgn         = (u32_RB_val >> 17) & 0x1FF;
		pParam->_param_read_comreg.u1_powerSaving_en 		= (u32_RB_val >> 26) & 0x1;

		pParam->_param_filmDetectctrl.u1_BadEditSlowOut_En		= (u32_RB_val >> 28) & 0x1;
		pParam->_param_filmDetectctrl.u1_QuickSwitch_En		= (u32_RB_val >> 29) & 0x1;
		pParam->_param_filmDetectctrl.u1_StillDetect_En			= (u32_RB_val >> 30) & 0x1;
		pParam->_param_filmDetectctrl.u1_DbgPrt_FRChg_En		= (u32_RB_val >> 31) & 0x1;

		ReadRegister(SOFTWARE_SOFTWARE_01_reg, 0, 31, &u32_RB_val);
		pParam->_param_filmDetectctrl.u4_Mot_rgn_diffWgt   =    u32_RB_val            & 0x0F;
		pParam->_param_filmDetectctrl.u4_Mot_all_diffWgt   =   (u32_RB_val  >>  4)    & 0x0F;
		pParam->_param_filmDetectctrl.u27_Mot_all_min      =   (u32_RB_val  >>  8)    & 0xFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_02_reg, 0, 31, &u32_RB_val);
		pParam->_param_filmDetectctrl.u4_enter_22_cntGain     =    u32_RB_val            & 0x0F;
		pParam->_param_filmDetectctrl.u4_enter_32_cntGain     =   (u32_RB_val  >>  4)    & 0x0F;
		pParam->_param_filmDetectctrl.u4_enter_else_cntGain   =   (u32_RB_val  >>  8)    & 0x0F;
		pParam->_param_filmDetectctrl.u4_quit_bigMot_wgt      =   (u32_RB_val  >> 12)    & 0x0F;
		pParam->_param_filmDetectctrl.u4_quit_smlMot_wgt      =   (u32_RB_val  >> 16)    & 0x0F;
		pParam->_param_filmDetectctrl.u1_quit_motWgt_auto_en  =   (u32_RB_val  >> 20)    & 0x01;
		pParam->_param_filmDetectctrl.u1_mixMode_det_en       =   (u32_RB_val  >> 26)    & 0x01;
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0]     =   (u32_RB_val  >> 27)    & 0x01;
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[1]     =   (u32_RB_val  >> 28)    & 0x01;
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[2]     =   (u32_RB_val  >> 29)    & 0x01;
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[3]     =   (u32_RB_val  >> 30)    & 0x01;
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[4]     =   (u32_RB_val  >> 31)    & 0x01;
		pParam->_param_filmDetectctrl.u5_dbg_param1           =   (u32_RB_val  >> 21)    & 0x1F;

		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[5]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[6]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[7]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[8]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[9]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[10]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[11]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[12]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[13]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[14]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[15]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];
		pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[16]     =   pParam->_param_filmDetectctrl.u1_mixMode_rgnEn[0];

		ReadRegister(SOFTWARE_SOFTWARE_03_reg, 0, 31, &u32_RB_val);
		pParam->_param_filmDetectctrl.u8_mixMode_enter_cntTh     =     u32_RB_val           & 0xFF;
		pParam->_param_filmDetectctrl.u8_mixMode_cnt_max         =    (u32_RB_val  >>  8)   & 0xFF;
		pParam->_param_filmDetectctrl.u8_quit_cnt_th             =    (u32_RB_val  >> 16)   & 0xFF;
		pParam->_param_filmDetectctrl.u8_quit_prd_th             =    (u32_RB_val  >> 24)   & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_04_reg, 0, 31, &u32_RB_val);
		pParam->_param_filmDetectctrl.u27_mixMode_rgnMove_minMot     =     u32_RB_val          & 0xFFFFFFF;
		pParam->_param_filmDetectctrl.u4_mixMode_rgnMove_motWgt      =    (u32_RB_val >> 28)   & 0x0F;

		ReadRegister(SOFTWARE_SOFTWARE_05_reg, 0, 31, &u32_RB_val);
		pParam->_param_filmDetectctrl.u32_dbg_param2      =     u32_RB_val          & 0xFFFFFFFF;
		ReadRegister(SOFTWARE_SOFTWARE_06_reg, 0, 31, &u32_RB_val);
		pParam->_param_filmDetectctrl.u32_dbg_cnt         =     u32_RB_val          & 0xFFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_07_reg, 0, 31, &u32_RB_val);
		pParam->_param_filmDetectctrl.u1_mixMode_en       =     u32_RB_val          & 0x1;
	}

	{
		//phase table
		ReadRegister(SOFTWARE_SOFTWARE_08_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_phtable.u1_phT_autoGen_en          =       u32_RB_val          & 0x01;
		pParam->_param_frc_phtable.u8_dejudder_lvl            =      (u32_RB_val >>  1)   & 0x1F;
		pParam->_param_frc_phtable.u8_deblur_lvl              =      (u32_RB_val >>  6)   & 0x1F;
		pParam->_param_frc_phtable.u2_candece_id_mode         =      (u32_RB_val >> 11)   & 0x03;
		pParam->_param_frc_phtable.u4_cadence_id              =      (u32_RB_val >> 13)   & 0x0F;
		pParam->_param_frc_phtable.in_3dFormat                =      (u32_RB_val >> 17)   & 0x07;
		pParam->_param_frc_phtable.out_3dFormat               =      (u32_RB_val >> 20)   & 0x03;
		pParam->_param_frc_phtable.u2_outMode                 =      (u32_RB_val >> 22)   & 0x03;
		pParam->_param_frc_phtable.u1_outLR_LRsamePh          =      (u32_RB_val >> 24)   & 0x01;
		pParam->_param_frc_phtable.u1_filmPh_sw_en            =      (u32_RB_val >> 25)   & 0x01;
		pParam->_param_frc_phtable.u1_filmPh_pf_en            =      (u32_RB_val >> 26)   & 0x01;
		pParam->_param_frc_phtable.u1_force_gen_table_en            =      (u32_RB_val >> 27)   & 0x01;


		ReadRegister(SOFTWARE_SOFTWARE_09_reg, 0, 31,  &u32_RB_val);
		pParam->_param_frc_phtable.u8_input_dly               =       u32_RB_val   & 0xFF;
		pParam->_param_fblevelctrl.u12_fblvl_iir_up_good	  =       (u32_RB_val >> 8)   & 0xFFF;
	}

	{
		// fall back control
		ReadRegister(SOFTWARE_SOFTWARE_10_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u1_FBLevelCtrl_en        =  u32_RB_val       & 0x01;
		pParam->_param_fblevelctrl.u1_BadRegionCtrl_en      = (u32_RB_val >> 1) & 0x01;
		pParam->_param_fblevelctrl.u1_FbLvl_gmvCtrl_en      = (u32_RB_val >> 2) & 0x01;
		pParam->_param_fblevelctrl.u1_FbLvl_occlCtrl_en     = (u32_RB_val >> 3) & 0x01;
		pParam->_param_fblevelctrl.u8_fblvl_map_y2_good     = (u32_RB_val >> 4) & 0xFF;
		pParam->_param_fblevelctrl.u8_good_rgn_num_th1     	= (u32_RB_val >> 12)& 0xFF;
		pParam->_param_fblevelctrl.u8_good_rgn_num_th2     	= (u32_RB_val >> 20)& 0xFF;
		pParam->_param_fblevelctrl.u4_good_scene_hold_cnt	= (u32_RB_val >> 28)& 0x0F;

		ReadRegister(SOFTWARE_SOFTWARE_11_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u27_Fb_tc_th_h = u32_RB_val & 0x7FFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_12_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u27_Fb_tc_th_l = u32_RB_val & 0x7FFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_13_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u8_FBTempConsisIIR_up       =  u32_RB_val        & 0xFF;
		pParam->_param_fblevelctrl.u8_FBTempConsis_IIR_down    = (u32_RB_val >>  8) & 0xFF;
		pParam->_param_fblevelctrl.u8_FBTempConsis_limit_gain  = (u32_RB_val >> 16) & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_14_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u32_FBTempConsis_limit_val       =  u32_RB_val     & 0xFFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_15_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u12_FBLevelIIRUp       =   u32_RB_val            & 0xFFF;
		pParam->_param_fblevelctrl.u12_FBLevelIIRDown     =  (u32_RB_val  >>  12)   & 0xFFF;
		pParam->_param_fblevelctrl.u8_SC_Gain             =  (u32_RB_val  >>  24)   & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_16_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u8_SC_slope            =  u32_RB_val        & 0xFF;
		pParam->_param_fblevelctrl.u8_SC_HoldCnt          = (u32_RB_val >>  8) & 0xFF;
		pParam->_param_fblevelctrl.u8_SC_NormCnt          = (u32_RB_val >> 16) & 0xFF;
		pParam->_param_fblevelctrl.u8_badRgn_sad_iirA_up  = (u32_RB_val >> 24) & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_17_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u8_badRgn_sad_iirA_down   =  u32_RB_val        & 0xFF;
		pParam->_param_fblevelctrl.u8_badRgn_center_gain     = (u32_RB_val >>  8) & 0xFF;
		pParam->_param_fblevelctrl.u8_badRgn_center_oft      = (u32_RB_val >> 16) & 0xFF;
		pParam->_param_fblevelctrl.u8_BadRegionGainIIR       = (u32_RB_val >> 24) & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_18_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u23_BadRegionSAD_th         =  u32_RB_val     & 0xFFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_19_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u23_BadRegionSAD_th_m       =  u32_RB_val     & 0xFFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_20_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u23_BadRegionSAD_th_h       =  u32_RB_val     & 0xFFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_21_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u5_BadRegionGain[0] =  u32_RB_val        & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[1] = (u32_RB_val >>  5) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[2] = (u32_RB_val >> 10) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[3] = (u32_RB_val >> 15) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[4] = (u32_RB_val >> 20) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[5] = (u32_RB_val >> 25) & 0x1F;

		ReadRegister(SOFTWARE_SOFTWARE_22_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u5_BadRegionGain[6]  =  u32_RB_val        & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[7]  = (u32_RB_val >>  5) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[8]  = (u32_RB_val >> 10) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[9]  = (u32_RB_val >> 15) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[10] = (u32_RB_val >> 20) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[11] = (u32_RB_val >> 25) & 0x1F;

		ReadRegister(SOFTWARE_SOFTWARE_23_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u5_BadRegionGain[12] =  u32_RB_val        & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[13] = (u32_RB_val >>  5) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[14] = (u32_RB_val >> 10) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[15] = (u32_RB_val >> 15) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[16] = (u32_RB_val >> 20) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[17] = (u32_RB_val >> 25) & 0x1F;

		ReadRegister(SOFTWARE_SOFTWARE_24_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u5_BadRegionGain[18] =  u32_RB_val        & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[19] = (u32_RB_val >>  5) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[20] = (u32_RB_val >> 10) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[21] = (u32_RB_val >> 15) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[22] = (u32_RB_val >> 20) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[23] = (u32_RB_val >> 25) & 0x1F;

		ReadRegister(SOFTWARE_SOFTWARE_25_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u5_BadRegionGain[24] =  u32_RB_val        & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[25] = (u32_RB_val >>  5) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[26] = (u32_RB_val >> 10) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[27] = (u32_RB_val >> 15) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[28] = (u32_RB_val >> 20) & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[29] = (u32_RB_val >> 25) & 0x1F;

		ReadRegister(SOFTWARE_SOFTWARE_26_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u5_BadRegionGain[30] =  u32_RB_val        & 0x1F;
		pParam->_param_fblevelctrl.u5_BadRegionGain[31] = (u32_RB_val >>  5) & 0x1F;
		pParam->_param_fblevelctrl.u7_gmv_ratio_thH     = (u32_RB_val >> 10) & 0x7F;
		pParam->_param_fblevelctrl.u7_gmv_ratio_thL     = (u32_RB_val >> 17) & 0x7F;
		pParam->_param_fblevelctrl.u6_gmv_gain_max      = (u32_RB_val >> 24) & 0x3F;

		ReadRegister(SOFTWARE_SOFTWARE_27_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u6_gmv_gain_min      =   u32_RB_val         & 0x3F;
		//pParam->_param_fblevelctrl.u6_gmv_similar_cnt   =  (u32_RB_val >>  6)  & 0x3F;
		pParam->_param_fblevelctrl.u8_gmv_gain_iir      =  (u32_RB_val  >> 12) & 0xFF;
		pParam->_param_fblevelctrl.u12_gmv_rng_cnt_th	=  (u32_RB_val  >> 20) & 0xFFF;

		ReadRegister(SOFTWARE_SOFTWARE_28_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u8_FBLevelMapY1 =  u32_RB_val        & 0xFF;
		pParam->_param_fblevelctrl.u8_FBLevelMapY2 = (u32_RB_val >>  8) & 0xFF;
		pParam->_param_fblevelctrl.u8_FBLevelMapX1 = (u32_RB_val >> 16) & 0xFF;
		pParam->_param_fblevelctrl.u8_FBLevelMapX2 = (u32_RB_val >> 24) & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_29_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u1_FbLvl_disIIR_en             =  u32_RB_val        & 0x01;
		pParam->_param_fblevelctrl.u8_FbLvl_disIIR_holdCnt        = (u32_RB_val >>  1) & 0xFF;
		pParam->_param_fblevelctrl.u1_scAnalysis_en               = (u32_RB_val >>  9) & 0x01;
		pParam->_param_fblevelctrl.u4_scAnalysis_frmCnt_th        = (u32_RB_val >> 10) & 0x0F;
		pParam->_param_fblevelctrl.u4_scAnalysis_badFrmCnt_th     = (u32_RB_val >> 14) & 0x0F;
		pParam->_param_fblevelctrl.u8_scAnalysis_st2mo_stPeriodTh = (u32_RB_val >> 18) & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_30_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u28_scAnalysis_sadAll_th0  =  u32_RB_val & 0xFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_31_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u28_scAnalysis_sadAll_th1  =  u32_RB_val & 0xFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_32_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u27_scAnalysis_TC_th0  =  u32_RB_val & 0xFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_33_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u27_scAnalysis_TC_th1  =  u32_RB_val & 0xFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_34_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u27_scAnalysis_static_motionTh  =  u32_RB_val & 0xFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_35_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u16_minusDiff_penalty  =  u32_RB_val & 0xFFFF;
		pParam->_param_fblevelctrl.u8_sc_bypass_cnt       = (u32_RB_val >> 16) & 0xFF;
		pParam->_param_fblevelctrl.u8_static_cnt_th       = (u32_RB_val >> 24) & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_36_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u32_static_mot_th  =  u32_RB_val & 0xFFFFFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_37_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u1_FbLvl_disableSCclr_en       =  u32_RB_val        & 0x01;
		pParam->_param_fblevelctrl.u1_Fblvl_Dh_tcTh_mapEn         = (u32_RB_val >>  1) & 0x01;
		pParam->_param_fblevelctrl.u1_Fblvl_blur_ctrlEn           = (u32_RB_val >>  2) & 0x01;
		pParam->_param_fblevelctrl.u1_Fblvl_judder_ctrlEn         = (u32_RB_val >>  3) & 0x01;
		pParam->_param_fblevelctrl.u8_FbLvl_dSCclr_d2en_th        = (u32_RB_val >>  4) & 0xFF;
		pParam->_param_fblevelctrl.u8_FbLvl_dSCclr_en2d_th        = (u32_RB_val >> 12) & 0xFF;
		pParam->_param_fblevelctrl.u8_FbLvl_dSCclr_holdCnt        = (u32_RB_val >> 20) & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_38_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u8_FbLvl_Dh_tc_thLgain     =     u32_RB_val           & 0xFF;
		pParam->_param_fblevelctrl.u8_FbLvl_Dh_tc_thHgain     =    (u32_RB_val  >>  8)   & 0xFF;
		pParam->_param_fblevelctrl.u8_DehaloFBlvl_iir         =    (u32_RB_val  >> 16)   & 0xFF;
		pParam->_param_fblevelctrl.u8_FbLvl_blur_tc_thLgain   =    (u32_RB_val  >> 24)   & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_39_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u8_FbLvl_blur_tc_thHgain        =     u32_RB_val           & 0xFF;
		pParam->_param_fblevelctrl.u8_FbLvl_blur_iir               =    (u32_RB_val  >>  8)   & 0xFF;
		pParam->_param_fblevelctrl.u12_FbLvl_blur_gmvUnconf_th0    =    (u32_RB_val  >> 16)   & 0xFFF;

		ReadRegister(SOFTWARE_SOFTWARE_40_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u12_FbLvl_blur_gmvUnconf_th1    =      u32_RB_val         & 0xFFF;
		pParam->_param_fblevelctrl.u5_FbLvl_blur_badscene_dftLvl   =     (u32_RB_val  >> 12) & 0x1F;
		pParam->_param_fblevelctrl.u8_FbLvl_judder_tc_thLgain      =     (u32_RB_val  >> 17) & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_41_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u8_FbLvl_judder_tc_thHgain      =      u32_RB_val         & 0xFF;
		pParam->_param_fblevelctrl.u8_FbLvl_judder_iir             =     (u32_RB_val >> 8)   & 0xFF;
		pParam->_param_fblevelctrl.u12_gmv_unconf_cnt_th		   =     (u32_RB_val >> 16)   & 0xFFF;

		ReadRegister(SOFTWARE_SOFTWARE_42_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u20_good_rgn_sad_th			   =	  u32_RB_val		 & 0xFFFFF;
		pParam->_param_fblevelctrl.u12_good_rgn_gmv_unconf_th	   =	 (u32_RB_val >> 20)  & 0xFFF;

		ReadRegister(SOFTWARE_SOFTWARE_43_reg, 0, 31, &u32_RB_val);
		pParam->_param_fblevelctrl.u8_badRgn_dtl_thr1			=	  u32_RB_val		 & 0xFF;
		pParam->_param_fblevelctrl.u8_badRgn_dtl_thr2	   		=	 (u32_RB_val >>  8)  & 0xFF;
		pParam->_param_fblevelctrl.u8_badRgn_dtl_gain2			=	 (u32_RB_val >> 16)  & 0xFF;
		pParam->_param_fblevelctrl.u8_badRgn_dtl_gain1			=	 (u32_RB_val >> 24)  & 0xFF;
	}

	{
		ReadRegister(SOFTWARE_SOFTWARE_44_reg, 14, 31, &u32_RB_val);
		pParam->_param_wrt_comreg.u8_blksameth_a_bidx4 = (u32_RB_val & 0xff);
		pParam->_param_wrt_comreg.u8_blksameth_h_bidx4 = (u32_RB_val >> 8)& 0xff;
	}

	{
               #if 0
		// fast motion
		ReadRegister(SOFTWARE_SOFTWARE_07_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u8_hpan_th			= (u32_RB_val & 0xff);
		pParam->_param_frc_sceneAnalysis.u8_vpan_th			= (u32_RB_val >> 8) & 0xff;
		pParam->_param_frc_sceneAnalysis.u8_cnt2_th			= (u32_RB_val >> 16) & 0xff;
		pParam->_param_frc_sceneAnalysis.u6_localfallback_lvl	= (u32_RB_val >> 24) & 0x3f;
		pParam->_param_frc_sceneAnalysis.u1_fastMotion_gfb_en	= (u32_RB_val >> 30) & 0x1;
		pParam->_param_frc_sceneAnalysis.u1_fastMotion_lfb_en	= (u32_RB_val >> 31) & 0x1;

		ReadRegister(SOFTWARE_SOFTWARE_45_reg, 0, 31, &u32_RB_val);
		pParam->_param_frc_sceneAnalysis.u25_glb_dtl_th 			= (u32_RB_val & 0xffff);
		pParam->_param_frc_sceneAnalysis.u8_fastMotion_glbfb_min 	= (u32_RB_val >> 24) & 0xff;
	}

	{
               #endif
		// write register
		ReadRegister(SOFTWARE_SOFTWARE_46_reg, 0, 31, &u32_RB_val);
		pParam->_param_wrt_comreg.u1_mc_lbmcMode_en				=  u32_RB_val         & 0x01;
		pParam->_param_wrt_comreg.u1_FBLevelWr_en				= (u32_RB_val >>  2 ) & 0x01;
		pParam->_param_wrt_comreg.u1_RimCtrl_wrt_en				= (u32_RB_val >>  3 ) & 0x01;
		pParam->_param_wrt_comreg.u1_dhClose_wrt_en				= (u32_RB_val >>  4 ) & 0x01;
		pParam->_param_wrt_comreg.u1_LGClr_wrt_en				= (u32_RB_val >>  5 ) & 0x01;
		pParam->_param_wrt_comreg.u1_LGSC_ctrl_wrt_en			= (u32_RB_val >>  6 ) & 0x01;
		pParam->_param_wrt_comreg.u1_dynME_acdcBld_wrt_en		= (u32_RB_val >>  7 ) & 0x01;
		pParam->_param_wrt_comreg.u1_bigFBlvl_dis_SCdet_en		= (u32_RB_val >>  8 ) & 0x01;
		pParam->_param_wrt_comreg.u1_LGBlkHstyClr_wrt_en		= (u32_RB_val >>  9 ) & 0x01;
		pParam->_param_wrt_comreg.u1_LGPixHstyClr_wrt_en		= (u32_RB_val >>  10) & 0x01;
		pParam->_param_wrt_comreg.u1_LGHstyClr_ClrLG_wrt_en		= (u32_RB_val >>  11) & 0x01;
		pParam->_param_wrt_comreg.u1_LGRg_dhclr_ctrl_wrt_en		= (u32_RB_val >>  12) & 0x01;
		pParam->_param_wrt_comreg.u1_LGRg_detect_adp_wrt_en		= (u32_RB_val >>  13) & 0x01;
		pParam->_param_wrt_comreg.u1_LG_SC_PtClose_wrt_en		= (u32_RB_val >>  14) & 0x01;
		pParam->_param_wrt_comreg.u1_LG_netflix_wrt_en			= (u32_RB_val >>  15) & 0x01;
		pParam->_param_wrt_comreg.u1_LG_lg16s_patch_wrt_en		= (u32_RB_val >>  17) & 0x01;
		pParam->_param_wrt_comreg.u1_LGRg_rim_unbalance_wrt_en	= (u32_RB_val >>  18) & 0x01;
		pParam->_param_wrt_comreg.u1_dynamic_obme_sel_wrt_en	= (u32_RB_val >>  19) & 0x01;
		pParam->_param_wrt_comreg.u1_dh_accord_ctrl_en			= (u32_RB_val >>  31) & 0x01;
	}

	{   // Logo dynamic params
		ReadRegister(SOFTWARE_SOFTWARE_47_reg, 0, 31, &u32_RB_val);
		pParam->_param_FRC_LGDet.u1_logo_pql_en                   =  u32_RB_val        & 0x01;
		pParam->_param_FRC_LGDet.u1_logo_sw_clr_en                = (u32_RB_val >>  1) & 0x01;
		pParam->_param_FRC_LGDet.u1_logo_rg_clr_en                = (u32_RB_val >>  2) & 0x01;
		pParam->_param_FRC_LGDet.u1_logo_glb_clr_en               = (u32_RB_val >>  3) & 0x01;
		pParam->_param_FRC_LGDet.u1_logo_left_half_clr_en         = (u32_RB_val >>  4) & 0x01;
		pParam->_param_FRC_LGDet.u1_logo_right_half_clr_en        = (u32_RB_val >>  5) & 0x01;
		pParam->_param_FRC_LGDet.u1_logo_top_half_clr_en          = (u32_RB_val >>  6) & 0x01;
		pParam->_param_FRC_LGDet.u1_logo_bot_half_clr_en          = (u32_RB_val >>  7) & 0x01;
		pParam->_param_FRC_LGDet.u1_logo_sc_alpha_ctrl_en         = (u32_RB_val >>  8) & 0x01;
		pParam->_param_FRC_LGDet.u1_logo_sc_logo_clr_en           = (u32_RB_val >>  9) & 0x01;
		pParam->_param_FRC_LGDet.u6_rgClr_holdtime                = (u32_RB_val >> 10) & 0x3F;
		pParam->_param_FRC_LGDet.u6_glbClr_holdtime               = (u32_RB_val >> 16) & 0x3F;
		pParam->_param_FRC_LGDet.u4_blk_pix_merge_type            = (u32_RB_val >> 22) & 0x0F;
		pParam->_param_FRC_LGDet.u5_iir_alpha                     = (u32_RB_val >> 26) & 0x1F;

		ReadRegister(SOFTWARE_SOFTWARE_48_reg, 0, 31, &u32_RB_val);
		pParam->_param_FRC_LGDet.u8_clr_rg_thr   =  u32_RB_val           & 0xFF;
		pParam->_param_FRC_LGDet.u8_clr_glb_thr  = (u32_RB_val >>  8)    & 0xFF;
		pParam->_param_FRC_LGDet.u8_clr_half_thr = (u32_RB_val >> 16)    & 0xFF;
		pParam->_param_FRC_LGDet.u8_clr_rg_thr_l = (u32_RB_val >> 24)    & 0xFF;

		ReadRegister(SOFTWARE_SOFTWARE_48_reg, 0, 31, &u32_RB_val);
		pParam->_param_FRC_LGDet.u8_logo_sc_FastDet_rstholdfrm = u32_RB_val         & 0xFF;
		pParam->_param_FRC_LGDet.u1_logo_sc_FastDet_en         = (u32_RB_val >> 30) & 0x01;
		pParam->_param_wrt_comreg.u1_logo_sc_FastDet_wrt_en    = (u32_RB_val >> 31) & 0x01;

		ReadRegister(SOFTWARE_SOFTWARE_49_reg, 0, 31, &u32_RB_val);
		pParam->_param_FRC_LGDet.u16_RgDhClr_thr_l = u32_RB_val           & 0xFFFF;
		pParam->_param_FRC_LGDet.u16_RgDhClr_thr_h = (u32_RB_val >> 16)   & 0xFFFF;

		ReadRegister(SOFTWARE_SOFTWARE_50_reg, 0, 31, &u32_RB_val);
		pParam->_param_FRC_LGDet.u1_RgDhClr_thr_en				= u32_RB_val		& 0x01;
		pParam->_param_FRC_LGDet.u1_GlbHstyClr_en				= (u32_RB_val >> 1)	& 0x01;
		pParam->_param_FRC_LGDet.u1_RgHstyClr_en				= (u32_RB_val >> 2)	& 0x01;
		pParam->_param_FRC_LGDet.u1_RgHstyClr_Sc_en				= (u32_RB_val >>3)	& 0x01;
		pParam->_param_FRC_LGDet.u1_RgHstyClr_glbclr_close_en	= (u32_RB_val >>4)	& 0x01;
		pParam->_param_FRC_LGDet.u1_logo_sc_detect_en			= (u32_RB_val >>5)	& 0x01;
		pParam->_param_FRC_LGDet.u1_logo_static_bg_en			= (u32_RB_val >>6)	& 0x01;
		pParam->_param_FRC_LGDet.u1_param_reg_scdetct			= (u32_RB_val >>7)	& 0x01;
		pParam->_param_FRC_LGDet.u1_logo_sc_saddif_en			= (u32_RB_val >>8)	& 0x01;
		pParam->_param_FRC_LGDet.u1_logo_sc_dtldif_en			= (u32_RB_val >>9)	& 0x01;
		pParam->_param_FRC_LGDet.u1_logo_sc_Gmv_en				= (u32_RB_val >>10)	& 0x01;
		pParam->_param_FRC_LGDet.u1_logo_netflix_detct_en		= (u32_RB_val >>11)	& 0x01;
		pParam->_param_FRC_LGDet.u1_param_reg_netflix_detect	= (u32_RB_val >>12)	& 0x01;
		pParam->_param_wrt_comreg.u1_dh_logo_bypass_en			= (u32_RB_val >>13)	& 0x01;
		pParam->_param_FRC_LGDet.u1_param_reg_lg16s_patch		= (u32_RB_val >>14)	& 0x01;
		pParam->_param_FRC_LGDet.u6_logo_static_bg_th_l			= (u32_RB_val >>20)	& 0x3F;
		pParam->_param_FRC_LGDet.u6_logo_static_bg_th_h			= (u32_RB_val >>26)	& 0x3F;

		if (pParam->_param_FRC_LGDet.u1_param_reg_lg16s_patch == 1)
		{
			// patch lg16s setting
			ReadRegister(SOFTWARE_SOFTWARE_51_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_en           = u32_RB_val        & 0x01;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Gapl_en      = (u32_RB_val >>1)  & 0x01;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Gdtl_en      = (u32_RB_val >>2)  & 0x01;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Gsad_en      = (u32_RB_val >>3)  & 0x01;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Gmv_en       = (u32_RB_val >>4)  & 0x01;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Rdtl_num_en  = (u32_RB_val >>5)  & 0x01;

			pParam->_param_FRC_LGDet.u8_logo_lg16s_patch_hold_frm     = (u32_RB_val >>24)    & 0xFF;

			ReadRegister(SOFTWARE_SOFTWARE_52_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u10_logo_lg16s_patch_Gapl_th_l   =  u32_RB_val      & 0xFFFF;
			pParam->_param_FRC_LGDet.u10_logo_lg16s_patch_Gapl_th_h   =  (u32_RB_val>>16) & 0xFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_53_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u25_logo_lg16s_patch_Gdtl_th_l   =  u32_RB_val      & 0xFFFFFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_54_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u25_logo_lg16s_patch_Gdtl_th_h   =  u32_RB_val      & 0xFFFFFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_55_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u28_logo_lg16s_patch_Gsad_th_l   =  u32_RB_val      & 0xFFFFFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_56_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u28_logo_lg16s_patch_Gsad_th_h   =  u32_RB_val      & 0xFFFFFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_57_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u11_logo_lg16s_patch_Gmvx_th_l   =  u32_RB_val      & 0xFFFF;
			pParam->_param_FRC_LGDet.u11_logo_lg16s_patch_Gmvx_th_h   =  (u32_RB_val>>16) & 0xFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_58_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u11_logo_lg16s_patch_Gmvy_th     =  u32_RB_val      & 0xFFFF;
			pParam->_param_FRC_LGDet.u6_logo_sc_Gmv_cnt_th            =  (u32_RB_val>>16) & 0xFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_59_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u12_logo_lg16s_patch_Gmv_unconf_th  = u32_RB_val      & 0xFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_60_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u20_logo_lg16s_patch_Rdtl_th_l      =   u32_RB_val      & 0xFFFFFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_61_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u20_logo_lg16s_patch_Rdtl_th_h      =   u32_RB_val      & 0xFFFFFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_62_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u6_logo_lg16s_patch_Rdtl_num_th     =    u32_RB_val      & 0xFFFF;


			// other setting const value

			// cc - d4
			pParam->_param_FRC_LGDet.u16_RgHstyClr_Gmv_thr  = 8;
			pParam->_param_FRC_LGDet.u16_RgHstyClr_mv_thr   =2;

			pParam->_param_FRC_LGDet.u10_RgHstyClr_Gsad_thr = 14;
			pParam->_param_FRC_LGDet.u10_RgHstyClr_Rsad_thr = 10;

			pParam->_param_FRC_LGDet.u12_RgHstyClr_Gmv_unconf_thr = 6;
			pParam->_param_FRC_LGDet.u12_RgHstyClr_mv_unconf_thr  = 6;
			// cc - d4
			pParam->_param_FRC_LGDet.u6_logo_sc_Gmv_cnt_th = 0;
			pParam->_param_FRC_LGDet.u12_logo_sc_Gmv_unconf_th = 0;
			pParam->_param_FRC_LGDet.u20_logo_sc_dtldif_th = 0xDF0;

			// d8
			pParam->_param_FRC_LGDet.u8_RgHstyClr_Gmv_cnt_thr     =  0x14;
			pParam->_param_FRC_LGDet.u12_RgHstyClr_mv_cnt_thr     =  0x3DD;

			// dc
			pParam->_param_FRC_LGDet.u31_logo_sc_saddiff_th       =  0x27000;
			pParam->_param_FRC_LGDet.u31_logo_sc_saddiff_th = ReadComReg_Calib_Data(pParam->_param_FRC_LGDet.u31_logo_sc_saddiff_th, CALIB_PARAM_SAD, 0, 0x7FFFFFFF, false);
			pParam->_param_FRC_LGDet.u1_RgDhClr_static_en         =  0;

			// e0
			pParam->_param_FRC_LGDet.u8_rg_dh_clr_lgclr_hold_frm		 = 8;

			// e4
			pParam->_param_FRC_LGDet.u8_rg_dh_clr_static_hold_frm         =  0;
			pParam->_param_FRC_LGDet.u8_logo_sc_hold_frm                  =  0xA;

			// e8
			pParam->_param_FRC_LGDet.u5_RgDhClr_cnt_th		   =  8;
			pParam->_param_FRC_LGDet.u8_logo_netflix_hold_frm  =  0xF;

			//ec
			pParam->_param_wrt_comreg.u8_logo_blksameth_l_bidx0_lg	   = 0x1A;
			pParam->_param_wrt_comreg.u8_logo_blksameth_l_bidx0_com 	= 0x20;

			//f0 - fc
			pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lt_th_l  = 0x58;
			pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lt_th_h  = 0x130;

			pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rt_th_l  = 0;
			pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rt_th_h  = 0x20;

			pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lb_th_l  = 0x75;
			pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lb_th_h  = 0xC0;

			pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rb_th_l  = 0x90;
			pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rb_th_h  = 0xF0;

			//f0  -  fc
			pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx0_lg	    = 0x4A;
			pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx0_com 	= 0x64;

			pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx10_lg 	 = 0x50;
			pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx10_com	 = 0x64;

			pParam->_param_wrt_comreg.u8_logo_blkhsty_pth_bidx10_lg 	 = 6;
			pParam->_param_wrt_comreg.u8_logo_blkhsty_pth_bidx10_com	 = 7;

			pParam->_param_wrt_comreg.u8_logo_blkhsty_nth_bidx10_lg 	 = 6;
			pParam->_param_wrt_comreg.u8_logo_blkhsty_nth_bidx10_com	 = 7;

		}
		else
		{
			// patch lg16s const setting
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_en           = 1;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Gapl_en      = 1;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Gdtl_en      = 1;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Gsad_en      = 0;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Gmv_en       = 1;
			pParam->_param_FRC_LGDet.u1_logo_lg16s_patch_Rdtl_num_en  = 0;

			pParam->_param_FRC_LGDet.u8_logo_lg16s_patch_hold_frm     = 0x20;

			pParam->_param_FRC_LGDet.u10_logo_lg16s_patch_Gapl_th_l   =  0x64;
			pParam->_param_FRC_LGDet.u10_logo_lg16s_patch_Gapl_th_h   =  0xB5;

			pParam->_param_FRC_LGDet.u25_logo_lg16s_patch_Gdtl_th_l   =  0x9EB10;

			pParam->_param_FRC_LGDet.u25_logo_lg16s_patch_Gdtl_th_h   =  0x13D620;

			pParam->_param_FRC_LGDet.u28_logo_lg16s_patch_Gsad_th_l   =  0x30D40;

			pParam->_param_FRC_LGDet.u28_logo_lg16s_patch_Gsad_th_h   =  0x61A80;

			pParam->_param_FRC_LGDet.u11_logo_lg16s_patch_Gmvx_th_l   =  0x40;
			pParam->_param_FRC_LGDet.u11_logo_lg16s_patch_Gmvx_th_h   =  0x12C;

			pParam->_param_FRC_LGDet.u11_logo_lg16s_patch_Gmvy_th     =  0xA;
			pParam->_param_FRC_LGDet.u6_logo_sc_Gmv_cnt_th            =  0x1D;

			pParam->_param_FRC_LGDet.u12_logo_lg16s_patch_Gmv_unconf_th  = 0x10;

			pParam->_param_FRC_LGDet.u20_logo_lg16s_patch_Rdtl_th_l      =   0;

			pParam->_param_FRC_LGDet.u20_logo_lg16s_patch_Rdtl_th_h      =   0;

			pParam->_param_FRC_LGDet.u6_logo_lg16s_patch_Rdtl_num_th     =    0;

			if (pParam->_param_FRC_LGDet.u1_param_reg_scdetct == 0)
			{
				ReadRegister(SOFTWARE_SOFTWARE_51_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u16_RgHstyClr_Gmv_thr  = u32_RB_val      & 0xFFFF;
				pParam->_param_FRC_LGDet.u16_RgHstyClr_mv_thr   =(u32_RB_val>>16) & 0xFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_52_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u10_RgHstyClr_Gsad_thr = u32_RB_val      &0xFFFF;
				pParam->_param_FRC_LGDet.u10_RgHstyClr_Rsad_thr = (u32_RB_val>>16)&0xFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_53_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u12_RgHstyClr_Gmv_unconf_thr = u32_RB_val       &0xFFFF;
				pParam->_param_FRC_LGDet.u12_RgHstyClr_mv_unconf_thr  = (u32_RB_val>>16) &0xFFFF;

				/////////
				pParam->_param_FRC_LGDet.u6_logo_sc_Gmv_cnt_th = 0;
				pParam->_param_FRC_LGDet.u12_logo_sc_Gmv_unconf_th = 0;
				pParam->_param_FRC_LGDet.u20_logo_sc_dtldif_th = 0xDF0;
			}
			else if (pParam->_param_FRC_LGDet.u1_param_reg_scdetct == 1)
			{

				pParam->_param_FRC_LGDet.u16_RgHstyClr_Gmv_thr  = 8;
				pParam->_param_FRC_LGDet.u16_RgHstyClr_mv_thr   =2;

				pParam->_param_FRC_LGDet.u10_RgHstyClr_Gsad_thr = 14;
				pParam->_param_FRC_LGDet.u10_RgHstyClr_Rsad_thr = 10;

				pParam->_param_FRC_LGDet.u12_RgHstyClr_Gmv_unconf_thr = 6;
				pParam->_param_FRC_LGDet.u12_RgHstyClr_mv_unconf_thr  = 6;
				///////////////////
				/*			ReadRegister(SOFTWARE_SOFTWARE_51_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u6_logo_sc_Gmv_cnt_th =  u32_RB_val      & 0xFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_52_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u12_logo_sc_Gmv_unconf_th = u32_RB_val    & 0xFFFFFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_53_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u20_logo_sc_dtldif_th = u32_RB_val    & 0xFFFFFFFF;*/

				// bai test
				pParam->_param_FRC_LGDet.u6_logo_sc_Gmv_cnt_th = 0;
				pParam->_param_FRC_LGDet.u12_logo_sc_Gmv_unconf_th = 0;
				//pParam->_param_FRC_LGDet.u20_logo_sc_dtldif_th = 0xDF0;

				ReadRegister(SOFTWARE_SOFTWARE_51_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u16_RgDhadp_Rimunbalance_thr_l = u32_RB_val       & 0xFFFF;
				pParam->_param_FRC_LGDet.u16_RgDhadp_Rimunbalance_thr_h = (u32_RB_val>>16) & 0xFFFF;
				ReadRegister(SOFTWARE_SOFTWARE_52_reg, 0, 31, &u32_RB_val);
				pParam->_param_wrt_comreg.u8_logo_mv_diff_diff_th_lg = u32_RB_val       & 0xFF;;
				pParam->_param_wrt_comreg.u8_logo_mv_diff_sad_th_lg  = (u32_RB_val >>  8)    & 0xFF;;
				pParam->_param_wrt_comreg.u8_logo_tmv_num_th_lg      = (u32_RB_val >> 16)    & 0xFF;
				pParam->_param_wrt_comreg.u8_logo_tmv_sad_th_lg      =  (u32_RB_val >> 24)   & 0xFF;

				ReadRegister(SOFTWARE_SOFTWARE_53_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u20_logo_sc_dtldif_th = u32_RB_val    & 0xFFFF;
				pParam->_param_FRC_LGDet.u8_RgDhadp_Rimunbalance_hold_frm = (u32_RB_val>>16) & 0xFFFF;

			}


			ReadRegister(SOFTWARE_SOFTWARE_54_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u8_RgHstyClr_Gmv_cnt_thr     = u32_RB_val       &0xFFFF;
			pParam->_param_FRC_LGDet.u12_RgHstyClr_mv_cnt_thr     = (u32_RB_val>>16) &0xFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_55_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u31_logo_sc_saddiff_th       = u32_RB_val       &0x7FFFFFFF;
			pParam->_param_FRC_LGDet.u1_RgDhClr_static_en         = (u32_RB_val>>31)      &0x01;		
			pParam->_param_FRC_LGDet.u31_logo_sc_saddiff_th = ReadComReg_Calib_Data(pParam->_param_FRC_LGDet.u31_logo_sc_saddiff_th, CALIB_PARAM_SAD, 0, 0x7FFFFFFF, false);

			ReadRegister(SOFTWARE_SOFTWARE_56_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u8_rg_dh_clr_lgclr_hold_frm         = u32_RB_val       &0xFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_57_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u8_rg_dh_clr_static_hold_frm         = u32_RB_val       &0xFFFF;
			pParam->_param_FRC_LGDet.u8_logo_sc_hold_frm                  = (u32_RB_val>>16) &0xFFFF;

			ReadRegister(SOFTWARE_SOFTWARE_58_reg, 0, 31, &u32_RB_val);
			pParam->_param_FRC_LGDet.u5_RgDhClr_cnt_th         = u32_RB_val       &0xFFFF;
			pParam->_param_FRC_LGDet.u8_logo_netflix_hold_frm  = (u32_RB_val>>16) &0xFF;

			ReadRegister(SOFTWARE_SOFTWARE_59_reg, 0, 31, &u32_RB_val);
			pParam->_param_wrt_comreg.u8_logo_blksameth_l_bidx0_lg     = u32_RB_val       &0xFFFF;
			pParam->_param_wrt_comreg.u8_logo_blksameth_l_bidx0_com     = (u32_RB_val>>16) &0xFFFF;


			if (pParam->_param_FRC_LGDet.u1_param_reg_netflix_detect == 1)
			{
				ReadRegister(SOFTWARE_SOFTWARE_60_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lt_th_l  = u32_RB_val 	  &0xFFFF;
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lt_th_h  = (u32_RB_val>>16) &0xFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_61_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rt_th_l  = u32_RB_val 	  &0xFFFF;
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rt_th_h  = (u32_RB_val>>16) &0xFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_62_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lb_th_l  = u32_RB_val 	  &0xFFFF;
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lb_th_h  = (u32_RB_val>>16) &0xFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_63_reg, 0, 31, &u32_RB_val);
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rb_th_l  = u32_RB_val 	  &0xFFFF;
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rb_th_h  = (u32_RB_val>>16) &0xFFFF;


				pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx0_lg	    = 0x4A;
				pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx0_com 	= 0x64;

				pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx10_lg 	 = 0x50;
				pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx10_com	 = 0x64;

				pParam->_param_wrt_comreg.u8_logo_blkhsty_pth_bidx10_lg 	 = 6;
				pParam->_param_wrt_comreg.u8_logo_blkhsty_pth_bidx10_com	 = 7;

				pParam->_param_wrt_comreg.u8_logo_blkhsty_nth_bidx10_lg 	 = 6;
				pParam->_param_wrt_comreg.u8_logo_blkhsty_nth_bidx10_com	 = 7;
			}
			else
			{
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lt_th_l  = 0x58;
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lt_th_h  = 0x130;

				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rt_th_l  = 0;
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rt_th_h  = 0x20;

				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lb_th_l  = 0x75;
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_lb_th_h  = 0xC0;

				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rb_th_l  = 0x90;
				pParam->_param_FRC_LGDet.u20_logo_netflix_detect_rb_th_h  = 0xF0;

				ReadRegister(SOFTWARE_SOFTWARE_60_reg, 0, 31, &u32_RB_val);
				pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx0_lg     = u32_RB_val       &0xFFFF;
				pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx0_com     = (u32_RB_val>>16) &0xFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_61_reg, 0, 31, &u32_RB_val);
				pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx10_lg     = u32_RB_val       &0xFFFF;
				pParam->_param_wrt_comreg.u8_logo_blksameth_a_bidx10_com     = (u32_RB_val>>16) &0xFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_62_reg, 0, 31, &u32_RB_val);
				pParam->_param_wrt_comreg.u8_logo_blkhsty_pth_bidx10_lg     = u32_RB_val       &0xFFFF;
				pParam->_param_wrt_comreg.u8_logo_blkhsty_pth_bidx10_com     = (u32_RB_val>>16) &0xFFFF;

				ReadRegister(SOFTWARE_SOFTWARE_63_reg, 0, 31, &u32_RB_val);
				pParam->_param_wrt_comreg.u8_logo_blkhsty_nth_bidx10_lg     = u32_RB_val       &0xFFFF;
				pParam->_param_wrt_comreg.u8_logo_blkhsty_nth_bidx10_com     = (u32_RB_val>>16) &0xFFFF;

			}

		}

	}

	{
		ReadRegister(SOFTWARE_SOFTWARE_44_reg, 0, 31, &u32_RB_val);
		pParam->_param_read_comreg.u2_rb_tc_mode  =   u32_RB_val           & 0x03;
		pParam->_param_read_comreg.u4_rb_tc1_coef =  (u32_RB_val >>  2)    & 0x0F;
		pParam->_param_read_comreg.u4_rb_tc2_coef =  (u32_RB_val >>  6)    & 0x0F;
		pParam->_param_read_comreg.u4_rb_tc3_coef =  (u32_RB_val >> 10)    & 0x0F;
	}



	{
		ReadRegister(SOFTWARE1_SOFTWARE1_00_reg, 0, 31, &u32_RB_val);
		pParam->_param_read_comreg.u1_HDMI_powerSaving_en = u32_RB_val & 0x1;  // u1_HDMI_powerSaving_en
		pParam->_param_read_comreg.u1_jerry_PanAct_en = (u32_RB_val >> 1) & 0x1;  // u1_jerry_PanAct_en
		pParam->_param_read_comreg.u1_forcePhTable_Regen = (u32_RB_val >> 2) & 0x1;  // u1_forcePhTable_Regen
		pParam->_param_read_comreg.u1_ResetFlow_en = (u32_RB_val >> 3) & 0x1;  // u1_ResetFlow_en
		pParam->_param_read_comreg.u1_force_cadence_en = (u32_RB_val >> 4) & 0x1;  // u1_force_cadence_en
		pParam->_param_read_comreg.u4_force_cadence_type = (u32_RB_val >> 5) & 0xf;  // u4_force_cadence_type
		pParam->_param_read_comreg.u1_SEFilm_fixCadence_flag = (u32_RB_val >> 9) & 0x1;  // u1_SEFilm_fixCadence_flag
		pParam->_param_read_comreg.u1_SEFilm_powerSaving_flag = (u32_RB_val >> 10) & 0x1;  // u1_SEFilm_powerSaving_flag
		pParam->_param_read_comreg.u4_SEFilm_force_cadence_type = (u32_RB_val >> 11) & 0xf;  // u4_SEFilm_force_cadence_type
	}

	// new algo enable control bit
	{
		ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 0, 31, &u32_RB_val);
		pParam->_param_wrt_comreg.u1_mc_near_rim_blending_en =			(u32_RB_val >> 0) & 0x1;
		pParam->_param_wrt_comreg.u1_mc_hf_lpf_en =						(u32_RB_val >> 1) & 0x1;
		pParam->_param_wrt_comreg.u1_mc_localfb_en =					(u32_RB_val >> 2) & 0x1;
		pParam->_param_wrt_comreg.u1_logo_dhRgnProc_en = 				(u32_RB_val >> 28) & 0x1;
		pParam->_param_wrt_comreg.u1_logo_HaloEnahce_en =				(u32_RB_val >> 30) & 0x1;
		pParam->_param_wrt_comreg.u1_logo_DynOBME_en =					(u32_RB_val >> 31) & 0x1;
	}

	// main object motion
	{
		ReadRegister(HARDWARE_HARDWARE_24_reg , 0, 31, &u32_RB_val);
		  
		pParam->_param_wrt_comreg.u1_me1_condition1 =(u32_RB_val >> 11) & 0x1;
		pParam->_param_wrt_comreg.u1_me1_condition2 =(u32_RB_val >> 12) & 0x1;
		pParam->_param_wrt_comreg.u1_me1_condition3 =(u32_RB_val >> 13) & 0x1;
		pParam->_param_wrt_comreg.u1_me1_condition4 =(u32_RB_val >> 14) & 0x1;
		pParam->_param_wrt_comreg.u1_me1_condition5 =(u32_RB_val >> 15) & 0x1;
		pParam->_param_wrt_comreg.u1_me1_condition6 =(u32_RB_val >> 16) & 0x1;
		pParam->_param_wrt_comreg.u1_me1_condition7 =(u32_RB_val >> 17) & 0x1;
		pParam->_param_wrt_comreg.u1_me1_condition8 =(u32_RB_val >> 29) & 0x1;		 
		pParam->_param_wrt_comreg.u1_me1_Newalgo_Proc_en_all =(u32_RB_val >> 18) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_condition9 =(u32_RB_val >> 19) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_condition1 =(u32_RB_val >> 20) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_condition2 =(u32_RB_val >> 21) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_condition3 =(u32_RB_val >> 22) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_condition4 =(u32_RB_val >> 23) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_condition5 =(u32_RB_val >> 24) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_condition6 =(u32_RB_val >> 25) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_condition7 =(u32_RB_val >> 26) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_condition8 =(u32_RB_val >> 27) & 0x1; 
		pParam->_param_wrt_comreg.u1_dehalo_Newalgo_Proc_en_all =(u32_RB_val >> 28) & 0x1;
		// pParam->_param_wrt_comreg.u1_me1_dehalo_Newalgo_print =(u32_RB_val >> 29) & 0x1;
		pParam->_param_wrt_comreg.u1_me1_Newalgo_Proc_en =(u32_RB_val >> 30) & 0x1;
		pParam->_param_wrt_comreg.u1_dehalo_Newalgo_Proc_en=(u32_RB_val >> 31) & 0x1;
		//rtd_pr_memc_info("dummy");
	}
		
	// main object motion
	{
		ReadRegister(SOFTWARE1_SOFTWARE1_47_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.u8_MainObjectMove_dtl_thl 			= (u32_RB_val >>  0) & 0xff; // u8_MainObjectMove_dtl_thl
		pParam->_param_me_sceneAnalysis.u4_MainObjectMove_bgMv_thl 			= (u32_RB_val >>  8) & 0xf;  // u4_MainObjectMove_bgMv_thl
		pParam->_param_me_sceneAnalysis.u4_MainObjectMove_objBgMvDiff_thl	= (u32_RB_val >> 12) & 0xf;  // u4_MainObjectMove_objBgMvDiff_thl
		#if 1 // LG TV006 493813 temp mark to sync
		pParam->_param_me_sceneAnalysis.u8_occl_ext_LogoCount_thr			= (u32_RB_val >> 16) & 0xff; // u8_occl_ext_LogoCount_thr
		pParam->_param_me_sceneAnalysis.u1_occl_ext_avoidLogo_en			= (u32_RB_val >> 31) & 0x1;  // u1_occl_ext_avoidLogo_en
		#endif

		ReadRegister(SOFTWARE1_SOFTWARE1_48_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.u8_MainObjectMove_thl_1			= (u32_RB_val >> 0 ) & 0xff; // u8_MainObjectMove_thl_1
		pParam->_param_me_sceneAnalysis.u8_MainObjectMove_thl_2			= (u32_RB_val >> 8 ) & 0xff; // u8_MainObjectMove_thl_2
		pParam->_param_me_sceneAnalysis.u8_MainObjectMove_1st_uconf_thr = (u32_RB_val >> 16) & 0xff; // u8_MainObjectMove_1st_uconf_thr
		pParam->_param_me_sceneAnalysis.u8_MainObjectMove_2nd_uconf_thr = (u32_RB_val >> 24) & 0xff; // u8_MainObjectMove_2nd_uconf_thr
	}

	// occl extension
	{
		ReadRegister(SOFTWARE1_SOFTWARE1_49_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.u8_occl_ext_bg_mv_diff_thl 			= (u32_RB_val >> 0 ) & 0xff; // u8_occl_ext_bg_mv_diff_thl
		pParam->_param_me_sceneAnalysis.u4_occl_ext_2nd_gmv_uconf_en_thl	= (u32_RB_val >> 8 ) & 0xf; // u4_occl_ext_2nd_gmv_uconf_en_thl
		pParam->_param_me_sceneAnalysis.u4_occl_ext_2nd_gmv_uconf_dis_thl	= (u32_RB_val >> 12) & 0xf; // u4_occl_ext_2nd_gmv_uconf_dis_thl
		pParam->_param_me_sceneAnalysis.u4_occl_ext_rgn_thl					= (u32_RB_val >> 16) & 0xf; // u4_occl_ext_rgn_thl
		pParam->_param_me_sceneAnalysis.u3_occl_ext_force_lvl				= (u32_RB_val >> 20) & 0x7; // u3_occl_ext_force_lvl
		#if 0 // LG TV006 493813 temp mark to sync
		pParam->_param_me_sceneAnalysis.u1_occl_ext_force_lvl_enable		= (u32_RB_val >> 23) & 0x1; // u1_occl_ext_force_lvl_enable
		#else
		pParam->_param_me_sceneAnalysis.u1_occl_ext_force_lvl_en			= (u32_RB_val >> 23) & 0x1; // u1_occl_ext_force_lvl_en
		#endif
		pParam->_param_me_sceneAnalysis.u1_occl_ext_force_protection_off	= (u32_RB_val >> 28) & 0x1; // u1_occl_ext_force_protection_off
		pParam->_param_me_sceneAnalysis.u1_occl_type2_enable				= (u32_RB_val >> 29) & 0x1; // u1_occl_type2_enable
		pParam->_param_me_sceneAnalysis.u1_occl_ext_gmv_weight_mode			= (u32_RB_val >> 30) & 0x1; // u1_occl_ext_gmv_weight_mode
		pParam->_param_wrt_comreg.u1_OcclExt_wrt_en							= (u32_RB_val >> 31) & 0x1; // u1_OcclExt_wrt_en
	}

	// dehalo post correction
	{
#if 0 // conflict with other func, need fine other software control register
		ReadRegister(HARDWARE_HARDWARE_50_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.u8_dh_postcorr_LogoCount_thr	= (u32_RB_val >> 0  ) & 0xff; // u8_dh_postcorr_LogoCount_thr
		pParam->_param_me_sceneAnalysis.u8_dh_postcorr_GmvMvy_thr		= (u32_RB_val >> 8  ) & 0xff; // u8_dh_postcorr_GmvMvy_thr
		pParam->_param_me_sceneAnalysis.u3_dh_postcorr_force_lvl		= (u32_RB_val >> 16 ) & 0x7;  // u3_dh_postcorr_force_lvl
		pParam->_param_me_sceneAnalysis.u1_dh_postcorr_force_lvl_en		= (u32_RB_val >> 19 ) & 0x1;  // u1_dh_postcorr_force_lvl_en
		pParam->_param_me_sceneAnalysis.u1_dh_postcorr_log_en			= (u32_RB_val >> 30 ) & 0x1;  // u1_dh_postcorr_log_en
		pParam->_param_me_sceneAnalysis.u1_dh_postcorrt_dyn_calc_en		= (u32_RB_val >> 31 ) & 0x1;  // u1_dh_postcorrt_dyn_calc_en
#endif
#if 0 // conflict with other func, need fine other software control register
		ReadRegister(SOFTWARE1_SOFTWARE1_38_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.u8_dh_postcorr_y2x_ratio_x0	= (u32_RB_val >> 0  ) & 0xff; // u8_dh_postcorr_y2x_ratio_x0
		pParam->_param_me_sceneAnalysis.u8_dh_postcorr_y2x_ratio_x1	= (u32_RB_val >> 8  ) & 0xff; // u8_dh_postcorr_y2x_ratio_x1
		pParam->_param_me_sceneAnalysis.u8_dh_postcorr_y2x_ratio_y0	= (u32_RB_val >> 16 ) & 0xff; // u8_dh_postcorr_y2x_ratio_y0
		pParam->_param_me_sceneAnalysis.u8_dh_postcorr_y2x_ratio_y1	= (u32_RB_val >> 24 ) & 0xff; // u8_dh_postcorr_y2x_ratio_y1
#endif
	}

	{
		//o------ regional fallback ------o
		ReadRegister(SOFTWARE1_SOFTWARE1_50_reg, 0, 31, &u32_RB_val);
		pParam->_param_bRMV_rFB_ctrl.u2_rFB_alpha_mode = u32_RB_val & 0x3;
		pParam->_param_bRMV_rFB_ctrl.u1_rFB_panning_en = (u32_RB_val >> 2) & 0x1;
		pParam->_param_wrt_comreg.u1_rFB_show_en = (u32_RB_val >> 3) & 0x1;
		pParam->_param_bRMV_rFB_ctrl.u8_rFB_alpha_curve_gain = (u32_RB_val >> 8) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rFB_alpha_curve_slp = (u32_RB_val >> 16) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rFB_alpha_curve_th0 = (u32_RB_val >> 24) & 0xff;


		ReadRegister(SOFTWARE1_SOFTWARE1_51_reg, 0, 31, &u32_RB_val);
//		pParam->_param_bRMV_rFB_ctrl.u8_rFB_alpha_curve_min = u32_RB_val & 0xff;
//		pParam->_param_bRMV_rFB_ctrl.u8_rFB_alpha_curve_max = (u32_RB_val >> 8) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_th0 = u32_RB_val & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_th1 = (u32_RB_val >> 8) & 0xff;

		pParam->_param_bRMV_rFB_ctrl.u8_rFB_alpha_force_value =  (u32_RB_val >> 16) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rFB_level_force_value =  (u32_RB_val >> 24) & 0xff;

		ReadRegister(SOFTWARE1_SOFTWARE1_52_reg, 0, 31, &u32_RB_val);
		pParam->_param_bRMV_rFB_ctrl.u1_rFB_alpha_auto = (u32_RB_val >> 0) & 0x1;
		pParam->_param_bRMV_rFB_ctrl.u1_rFB_level_auto = (u32_RB_val >> 1) & 0x1;
		pParam->_param_me_sceneAnalysis.u1_over_searchrange_en= (u32_RB_val >> 2) & 0x1;       // Over_SearchRange_en
		pParam->_param_me_sceneAnalysis.u1_force_panning= (u32_RB_val >> 3) & 0x1;             // Over_SearchRange_force_panning
		pParam->_param_bRMV_rFB_ctrl.u1_rFB_alpha_by_dtl_en = (u32_RB_val >> 4) & 0x1;         // enable for rFB alpha by DTL
		pParam->_param_bRMV_rFB_ctrl.u1_rFB_alpha_by_rMV_cnt_en = (u32_RB_val >> 5) & 0x1;     // enable for rFB alpha by rMV_cnt
		pParam->_param_bRMV_rFB_ctrl.u1_rFB_big_rMV_en = (u32_RB_val >> 6) & 0x1;         	   // enable for rFB alpha by big rMV
		pParam->_param_bRMV_rFB_ctrl.u1_rFB_big_rTC_en = (u32_RB_val >> 7) & 0x1;     		   // enable for rFB alpha by big rTC
		pParam->_param_bRMV_rFB_ctrl.u8_rFB_level_curve_gain = (u32_RB_val >> 8) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rFB_level_curve_slp = (u32_RB_val >> 16) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rFB_level_curve_th0 = (u32_RB_val >> 24) & 0xff;


		ReadRegister(SOFTWARE1_SOFTWARE1_53_reg, 0, 31, &u32_RB_val);
//		pParam->_param_bRMV_rFB_ctrl.u8_rFB_level_curve_min = u32_RB_val & 0xff;
//		pParam->_param_bRMV_rFB_ctrl.u8_rFB_level_curve_max = (u32_RB_val >> 8) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_th2 = u32_RB_val & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_th3 = (u32_RB_val >> 8) & 0xff;
		pParam->_param_me_sceneAnalysis.u8_over_searchrange_thl = (u32_RB_val >> 16) & 0xff;     // Over_SearchRange_threshold

		//o------ ME median filter ------	
		ReadRegister(SOFTWARE1_SOFTWARE1_21_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.u2_medFLT_sel = (u32_RB_val >> 3) & 0x3;		
		ReadRegister(SOFTWARE1_SOFTWARE1_54_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.b_multi_GMV_en = u32_RB_val & 0x1;

		pParam->_param_me_sceneAnalysis.b_relMV_panning_en = (u32_RB_val >> 1) & 0x1;
		//pParam->_param_me_sceneAnalysis.b_force_medFLT = (u32_RB_val >> 2) & 0x1;
		pParam->_param_me_sceneAnalysis.b_show_mode = (u32_RB_val >> 3) & 0x1;
		pParam->_param_me_sceneAnalysis.u4_panning_gmv_unconf_thl = (u32_RB_val >> 4) & 0xf;	
		pParam->_param_me_sceneAnalysis.u8_mvDiff_thl = (u32_RB_val >> 8) & 0xff;
		pParam->_param_me_sceneAnalysis.u8_rFBss_thl = (u32_RB_val >> 16) & 0xff;
		pParam->_param_me_sceneAnalysis.u8_rSCss_thl = (u32_RB_val >> 24) & 0xff;

		ReadRegister(SOFTWARE1_SOFTWARE1_55_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.u8_big_relMV_cnt_thl = u32_RB_val & 0x1f;
		pParam->_param_me_sceneAnalysis.u3_show_mode2 = (u32_RB_val >> 5) & 0x7;
		pParam->_param_me_sceneAnalysis.u8_rChaos_cnt_thl = (u32_RB_val >>  8) & 0x1f;
		pParam->_param_me_sceneAnalysis.u8_panning_gmv_cnt_thl = (u32_RB_val >> 16) & 0xff;		

		ReadRegister(SOFTWARE1_SOFTWARE1_56_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.u8_rPrd_thl     = u32_RB_val & 0xff;
		pParam->_param_me_sceneAnalysis.u8_rPrd_cnt_thl = (u32_RB_val >> 8) & 0x1f;
		pParam->_param_me_sceneAnalysis.b_prd_en = (u32_RB_val >> 13) & 0x1;
		pParam->_param_me_sceneAnalysis.b_logo_en = (u32_RB_val >> 14) & 0x1;
		pParam->_param_me_sceneAnalysis.b_pan_noFB_en = (u32_RB_val >> 15) & 0x1;

		//o------ ME median filter : adaptive condition ------o
		ReadRegister(SOFTWARE3_SOFTWARE3_00_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.b_medFLT_adpt_en = (u32_RB_val >> 0) & 0x1;
		pParam->_param_me_sceneAnalysis.b_pfvd_en = (u32_RB_val >> 1) & 0x1;

		//o------ Fix repeat broken ------o
		ReadRegister(SOFTWARE3_SOFTWARE3_63_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.b_FixRepeat_en = (u32_RB_val >> 31) & 0x1;

		//o------ Detect sport scene ------o
		ReadRegister(SOFTWARE3_SOFTWARE3_04_reg, 0, 31, &u32_RB_val);
		pParam->_param_me_sceneAnalysis.u1_detect_SportScene_en = (u32_RB_val >> 0) & 0x1;
		pParam->_param_Patch_Manage.u1_sportscene_en = (u32_RB_val >> 16) & 0x1;
		pParam->_param_wrt_comreg.u1_SmallObject_en = (u32_RB_val >> 20) & 0x1;

		//o------ RMV blending ------o
		ReadRegister(SOFTWARE1_SOFTWARE1_54_reg, 0, 31, &u32_RB_val);
		pParam->_param_bRMV_rFB_ctrl.u2_rmv_alpha_mode = u32_RB_val & 0x3;
		pParam->_param_bRMV_rFB_ctrl.u1_gmv_alpha_auto = (u32_RB_val >> 2) & 0x1;
		pParam->_param_bRMV_rFB_ctrl.u8_rmv_alpha_curve_gain = (u32_RB_val >> 8) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rmv_alpha_curve_slp = (u32_RB_val >> 16) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rmv_alpha_curve_th0 = (u32_RB_val >> 24) & 0xff;

		ReadRegister(SOFTWARE1_SOFTWARE1_55_reg, 0, 31, &u32_RB_val);
		pParam->_param_bRMV_rFB_ctrl.u8_rmv_alpha_curve_min = u32_RB_val & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rmv_alpha_curve_max = (u32_RB_val >> 8) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_gmv_alpha_force_value = (u32_RB_val >> 16) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_regioonal_unconf_cnt_th = (u32_RB_val >> 24) & 0xffff;

		ReadRegister(SOFTWARE1_SOFTWARE1_56_reg, 0, 31, &u32_RB_val);
		pParam->_param_bRMV_rFB_ctrl.u12_regional_cnt_thl = u32_RB_val & 0xffff;
		//pParam->_param_bRMV_rFB_ctrl.u12_regional_unconf_thl = (u32_RB_val >> 16) & 0xffff;
		pParam->_param_bRMV_rFB_ctrl.u8_rFB_panning_unconf_thl = (u32_RB_val >> 16) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rFB_panning_cnt_thl = (u32_RB_val >> 24) & 0xff;

		ReadRegister(SOFTWARE1_SOFTWARE1_57_reg, 0, 31, &u32_RB_val);
		pParam->_param_bRMV_rFB_ctrl.u8_oppo_dir_cnt_thl = u32_RB_val & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_rmv_var_thl = (u32_RB_val >> 8) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_gmv_cur_gain = (u32_RB_val >> 16) & 0xff;
		pParam->_param_bRMV_rFB_ctrl.u8_gmv_cur_slp = (u32_RB_val >> 24) & 0xff;

	}

	{
		ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 0, 31, &u32_RB_val);
		pParam->_param_wrt_comreg.u1_DW_fmdet_4region_En = (u32_RB_val >> 14);

#if 0 // conflict with other func, need fine other software control register
		// sld pixel logo dynamic adjust
		ReadRegister(SOFTWARE1_SOFTWARE1_58_reg, 0, 31, &u32_RB_val);
		// [0] : FRC_SLD_PxlTargetYVal
		// [1] : FRC_SLD_PxlRgnClr
		pParam->_param_FRC_LGDet.u1_logo_sld_debug_print_en		= (u32_RB_val >> 0 ) & 0xff;
		pParam->_param_FRC_LGDet.u1_logo_sld_debug_print_period = (u32_RB_val >> 12) & 0xf;
		pParam->_param_wrt_comreg.u1_logo_pxldfth_dyn_en		= (u32_RB_val >> 16) & 0x1;
		pParam->_param_wrt_comreg.u1_logo_pxlrgclr_en			= (u32_RB_val >> 17) & 0x1;
#endif
#if 0 // conflict with other func, need fine other software control register
		ReadRegister(SOFTWARE1_SOFTWARE1_59_reg, 0, 31, &u32_RB_val);
		pParam->_param_FRC_LGDet.u8_logo_rglt_pxldf_apl_thr_l = (u32_RB_val >> 0 ) & 0xff;
		pParam->_param_FRC_LGDet.u8_logo_rglt_pxldf_apl_thr_h = (u32_RB_val >> 8 ) & 0xff;
		pParam->_param_FRC_LGDet.u8_logo_rglt_pxldf_val_thr_l = (u32_RB_val >> 16) & 0xff;
		pParam->_param_FRC_LGDet.u8_logo_rglt_pxldf_val_thr_h = (u32_RB_val >> 24) & 0xff;
#endif
#if 0 // conflict with other func, need fine other software control register
		ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 0, 31, &u32_RB_val);
		pParam->_param_FRC_LGDet.u32_logo_rgclr_cmp_ofst_thr	= (u32_RB_val >> 0 ) & 0xff;
		pParam->_param_FRC_LGDet.u32_logo_rgclr_cmp_gain_thr	= (u32_RB_val >> 8 ) & 0xff;
		pParam->_param_FRC_LGDet.u8_logo_rgclr_logo_mv_thr		= (u32_RB_val >> 16) & 0xff;
#endif
#if 0 // conflict with other func, need fine other software control register
		ReadRegister(SOFTWARE1_SOFTWARE1_61_reg, 0, 31, &u32_RB_val);
		pParam->_param_FRC_LGDet.u32_logo_rgclr_apl_thr 	= (u32_RB_val >> 0 ) & 0xff;
		pParam->_param_FRC_LGDet.u32_logo_rgclr_dtl_thr 	= (u32_RB_val >> 8 ) & 0xf;
		pParam->_param_FRC_LGDet.u32_logo_rgclr_apl2dtl_thr	= (u32_RB_val >> 12) & 0xff;
		pParam->_param_FRC_LGDet.u32_logo_rgclr_cnt_thr 	= (u32_RB_val >> 20) & 0xff;
		pParam->_param_FRC_LGDet.u1_logo_rgclr_apl_en		= (u32_RB_val >> 28) & 0x1;
		pParam->_param_FRC_LGDet.u1_logo_rgclr_dtl_en		= (u32_RB_val >> 29) & 0x1;
		pParam->_param_FRC_LGDet.u1_logo_rgclr_cnt_en		= (u32_RB_val >> 30) & 0x1;
		pParam->_param_FRC_LGDet.u1_logo_rgclr_cmp_en		= (u32_RB_val >> 31) & 0x1;
#endif
	}
}

