#ifndef _PATCH_MANAGE_H
#define _PATCH_MANAGE_H

typedef struct  
{
        //// HF1
	unsigned char u1_HF1_Detect_en;
	unsigned char u8_HF1_Hdtl_thr_gain;
	unsigned char u8_HF1_Vdtl_thr_gain;
	unsigned char u8_HF1_mvx_thr;
	unsigned char u7_HF1_mvy_thr;
	
	//// HF2
	unsigned char u1_HF2_Detect_en;
	unsigned char u8_HF2_Hdtl_thr_gain;
	unsigned char u8_HF2_Vdtl_thr_gain;
	unsigned char u8_HF2_mvx_thr;
	unsigned char u7_HF2_mvy_thr;	

	//// HF3
	unsigned char u1_HF3_Detect_en;
	unsigned char u8_HF3_Hdtl_thr_gain;
	unsigned char u8_HF3_Vdtl_thr_gain;
	unsigned char u8_HF3_mvx_thr;
	unsigned char u7_HF3_mvy_thr;	

	//// RidingHorse
	unsigned char u1_RidingHorse_Detect_en;
	unsigned char u8_RidingHorse_Uamount_thr_gain;
	unsigned char u8_RidingHorse_Vamount_thr_gain;
	unsigned char u8_RidingHorse_mvx_thr;
	unsigned char u7_RidingHorse_mvy_thr;	
        //====================

	unsigned char u1_Print_en;
	unsigned char u1_dbScan_en;

	// casino only.
	unsigned char  u1_patch_casino_en;

	// arm only
	unsigned char  u1_patch_arm_en;
	unsigned char  u8_patch_arm_holdCnt;
	unsigned char  u8_patch_arm_detCntTh;
	unsigned char  u8_patch_arm_detCnt_dStep;

	unsigned char  u1_arm_rgnSad_byp;
	unsigned char  u1_arm_gmv_byp;
	unsigned char  u1_arm_rim_byp;
	unsigned char  u1_arm_mcDtl_byp;

	unsigned int u23_arm_rgnSad_smallTh;
	unsigned char  u5_arm_rgnSad_Scnt_thL;
	unsigned char  u5_arm_rgnSad_Scnt_thH;
	unsigned int u23_arm_rgnSad_largeTh;
	unsigned char  u5_arm_rgnSad_Lcnt_thL;
	unsigned char  u5_arm_rgnSad_Lcnt_thH;

	signed short s11_arm_gmvX_thL;
	signed short s11_arm_gmvX_thH;
	signed short s10_arm_gmvY_thL;
	signed short s10_arm_gmvY_thH;
	unsigned char  u8_arm_gmvXratio_thL;
	unsigned char  u8_arm_gmvXratio_thH;
	unsigned char  u8_arm_gmvYratio_thL;
	unsigned char  u8_arm_gmvYratio_thH;

	unsigned short u16_arm_rimLft_th;
	unsigned short u16_arm_rimRgt_th;
	unsigned short u16_arm_rimUp_th;
	unsigned short u16_arm_rimDn_th;
	unsigned char  u8_arm_rim_delta;

	unsigned short u16_arm_mcHdtl_thL;
	unsigned short u16_arm_mcHdtl_thH;
	unsigned short u16_arm_mcVdtl_thL;
	unsigned short u16_arm_mcVdtl_thH;

	unsigned char  u8_patchCoring_sad0;
	unsigned char  u8_patchCoring_sad1;

	// golf ball
	unsigned char  u1_patch_golfBall_en;
	unsigned char  u1_golfBall_Hdtl_byp;
	unsigned char  u1_golfBall_Vdtl_byp;
	unsigned short u16_golfBall_bHsV_Vdtl_th0;
	unsigned short u16_golfBall_bHsV_Vdtl_th1;
	unsigned short u16_golfBall_bHsV_Hdtl_th0;
	unsigned short u16_golfBall_bHsV_Hdtl_th1;

	//ball jump
	unsigned char  u1_patch_ballJump_en;

	//// Bluce Mic
	unsigned char u1_BluceMic_En;	

	//soccer
	unsigned char u1_patch_soccer_en;	

	//sport scene(small object)
	unsigned char u1_sportscene_en;

	//high-freq player osd
	unsigned char u1_patch_highFreq_en;

	//Forbes
	unsigned char u1_ForceForbes_en;

	//patch_tri_ball_Issue_en
	unsigned char u1_tri_ball_Issue_en;

	//patch_M_en
	unsigned char u1_patch_M_en;
}_PARAM_PATCH_M;

typedef struct  
{
        //// HF1
	unsigned char u1_HF1_Detected;
	unsigned char u6_HF1_hold_cnt;

	//// HF2
	unsigned char u1_HF2_Detected;
	unsigned char u6_HF2_hold_cnt;

	//// HF3
	unsigned char u1_HF3_Detected;
	unsigned char u6_HF3_hold_cnt;

	//// HF3
	unsigned char u1_RidingHorse_Detected;
	unsigned char u6_RidingHorse_hold_cnt;
        //============================

//	unsigned int u27_TempConsis_pre;
	unsigned char  u1_Print_En;
	int			 s32_PrintCnt;

	unsigned char  u1_dbScan_en;

	// arm
	unsigned char  u1_patchArm_true;
	unsigned char  u1_patchArm_preTrue;
	unsigned char  u8_arm_hcnt;
	unsigned char  u8_arm_detCnt;

	unsigned char  u1_CoringThr_State;

	// golf ball
	unsigned char  u1_golfBall_HVdtl_FB_true;
	unsigned char  u1_golfBall_HVdtl_FB_preTrue;
	
	unsigned short u12_golfBall_bk_obmeMode;
	unsigned short u8_golfBall_bk_varLPF_y;

	//ball jump
	unsigned char   u1_BallJump_true;
	char   s8_BallJump_holdCnt;
	unsigned int  u32_foreground_tc_pre[4];
	unsigned char   u1_dh_close;
	
	//// Bluce Mic
	unsigned char u1_sad_matched;
	unsigned char u1_tc_matched;	
	unsigned char u1_dtl_matched;
	unsigned char u1_apl_matched;
	unsigned char u1_BluceMic_Detected;

	//high-freq player osd
	unsigned char u1_patch_highFreq_zmv_true;
	unsigned char u1_patch_highFreq_rp_true;	

	/////
	unsigned char  u1_Forbes_Patch;
	unsigned char  SAD_flg_h;
	unsigned char  rMV_flg_h;

	//patch_tri_ball_Issue
	unsigned char u1_tri_ball_Detected;
	unsigned char u6_tri_ball_hold_cnt;
	
}_OUTPUT_PATCH_M;

VOID PatchManage_Init(_OUTPUT_PATCH_M *pOutput);
VOID PatchManage_Proc(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput);

VOID Patch_detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput);
VOID Patch_Action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput);
//VOID Patch_CoringThr(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput);

// VOID DoubleScan_Action(unsigned char dbScan_en, unsigned char smallObj_en, unsigned char dh_close_en, _OUTPUT_PATCH_M *pOutput);

DEBUGMODULE_BEGIN(_PARAM_PATCH_M,_OUTPUT_PATCH_M)

#if 0
	ADD_PARAM_DES(unsigned char, u1_Print_en,             0,  "")
	ADD_PARAM_DES(unsigned char, u1_dbScan_en,            0,  "patch ctrl dbScan")

	/// casino
	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char, u1_patch_casino_en,                0,  "patch_casino")
	ADD_PARAM_DES(unsigned char, u8_patch_casino_holdCnt,           6,  "")
	ADD_PARAM_DES(unsigned char,  u8_patch_casino_detCntTh,         1,  " >= ")
	ADD_PARAM_DES(unsigned char,  u8_patch_casino_detCnt_dStep,     2,  " ")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u1_casino_meDtl_byp,              0,  "")
	ADD_PARAM_DES(unsigned char,  u1_casino_gmv_byp,                0,  "")
	ADD_PARAM_DES(unsigned char,  u1_casino_rim_byp,                0,  "")
	ADD_PARAM_DES(unsigned char,  u1_casino_mcDtl_byp,              0,  " ")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned int, u20_casino_meRgnDtl_diffTh[0],    0, "diff_10_17, <=")
	ADD_PARAM_DES(unsigned int, u20_casino_meRgnDtl_diffTh[1],    0, "diff_10_20")
	ADD_PARAM_DES(unsigned int, u20_casino_meRgnDtl_diffTh[2],    0, "diff_10_27")
	ADD_PARAM_DES(unsigned int, u20_casino_meRgnDtl_diffTh[3],    0, "diff_17_20")
	ADD_PARAM_DES(unsigned int, u20_casino_meRgnDtl_diffTh[4],    0, "diff_17_27")
	ADD_PARAM_DES(unsigned int, u20_casino_meRgnDtl_diffTh[5],    0, "diff_20_27")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(signed short, s11_casino_gmvX_thL,          0,  " for patch_casino")
	ADD_PARAM_DES(signed short, s11_casino_gmvX_thH,          0,  " <= H, s9.2")
	ADD_PARAM_DES(signed short, s10_casino_gmvY_thL,          0,  " >= L, s8.2")
	ADD_PARAM_DES(signed short, s10_casino_gmvY_thH,          0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_casino_gmvXratio_thL,      0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_casino_gmvXratio_thH,      0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_casino_gmvYratio_thL,      0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_casino_gmvYratio_thH,      0,  " ")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned short, u16_casino_rimLft_th,     0,  " ")
	ADD_PARAM_DES(unsigned short, u16_casino_rimRgt_th,     0,  " ")
	ADD_PARAM_DES(unsigned short, u16_casino_rimUp_th,      0,  " ")
	ADD_PARAM_DES(unsigned short, u16_casino_rimDn_th,      0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_casino_rim_delta,      2,  " lft-rht-up-dn delta")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned short, u16_casino_mcHdtl_thL,     0,  " u16.0 ")
	ADD_PARAM_DES(unsigned short, u16_casino_mcHdtl_thH,     0,  " ")
	ADD_PARAM_DES(unsigned short, u16_casino_mcVdtl_thL,     0,  " ")
	ADD_PARAM_DES(unsigned short, u16_casino_mcVdtl_thH,     0,  " ")

	//// arm
	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char, u1_patch_arm_en,                0,  "patch_arm")
	ADD_PARAM_DES(unsigned char, u8_patch_arm_holdCnt,           6,  "")
	ADD_PARAM_DES(unsigned char,  u8_patch_arm_detCntTh,         1,  " >= ")
	ADD_PARAM_DES(unsigned char,  u8_patch_arm_detCnt_dStep,     2,  "  ")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u1_arm_rgnSad_byp,             0,  "")
	ADD_PARAM_DES(unsigned char,  u1_arm_gmv_byp,                0,  "")
	ADD_PARAM_DES(unsigned char,  u1_arm_rim_byp,                0,  "")
	ADD_PARAM_DES(unsigned char,  u1_arm_mcDtl_byp,              0,  " ")


	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned int, u23_arm_rgnSad_smallTh,     0,  " <= ")
	ADD_PARAM_DES(unsigned char,  u5_arm_rgnSad_Scnt_thL,     0,  " ")
	ADD_PARAM_DES(unsigned char,  u5_arm_rgnSad_Scnt_thH,     0,  " ")
	ADD_PARAM_DES(unsigned int, u23_arm_rgnSad_largeTh,     0,  " >= ")
	ADD_PARAM_DES(unsigned char,  u5_arm_rgnSad_Lcnt_thL,     0,  " ")
	ADD_PARAM_DES(unsigned char,  u5_arm_rgnSad_Lcnt_thH,     0,  " ")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(signed short, s11_arm_gmvX_thL,          0,  "for patch_arm")
	ADD_PARAM_DES(signed short, s11_arm_gmvX_thH,          0,  " <= H , s9.2")
	ADD_PARAM_DES(signed short, s10_arm_gmvY_thL,          0,  " >= L,  s8.2")
	ADD_PARAM_DES(signed short, s10_arm_gmvY_thH,          0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_arm_gmvXratio_thL,      0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_arm_gmvXratio_thH,      0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_arm_gmvYratio_thL,      0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_arm_gmvYratio_thH,      0,  " ")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned short, u16_arm_rimLft_th,     0,  " ")
	ADD_PARAM_DES(unsigned short, u16_arm_rimRgt_th,     0,  " ")
	ADD_PARAM_DES(unsigned short, u16_arm_rimUp_th,      0,  " ")
	ADD_PARAM_DES(unsigned short, u16_arm_rimDn_th,      0,  " ")
	ADD_PARAM_DES(unsigned char,  u8_arm_rim_delta,      2,  " ")	

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned short, u16_arm_mcHdtl_thL,     0,  " u16.0 ")
	ADD_PARAM_DES(unsigned short, u16_arm_mcHdtl_thH,     0,  " ")
	ADD_PARAM_DES(unsigned short, u16_arm_mcVdtl_thL,     0,  " ")
	ADD_PARAM_DES(unsigned short, u16_arm_mcVdtl_thH,     0,  " ")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,  u8_patchCoring_sad0,     2, "")
	ADD_PARAM_DES(unsigned char,  u8_patchCoring_sad1,    4, "")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char,   u1_patch_golfBall_en,	    0, "")
	ADD_PARAM_DES(unsigned char,   u1_golfBall_Hdtl_byp, 	    0, "")
	ADD_PARAM_DES(unsigned char,   u1_golfBall_Vdtl_byp,	    0, "")
	ADD_PARAM_DES(unsigned short,  u16_golfBall_bHsV_Vdtl_th0,	0, "u16.0, >")
	ADD_PARAM_DES(unsigned short,  u16_golfBall_bHsV_Vdtl_th1, 	0, "u16.0, <")
	ADD_PARAM_DES(unsigned short,  u16_golfBall_bHsV_Hdtl_th0,	0, "u16.0, >")
	ADD_PARAM_DES(unsigned short,  u16_golfBall_bHsV_Hdtl_th1, 	0, "u16.0, <")

	////////  output  //////////////////////////////////////////////////////////////////
    
	ADD_OUTPUT(unsigned char,  u1_Print_En)
	ADD_OUTPUT(int			, s32_PrintCnt)
    
	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,  u1_dbScan_en)

	ADD_DUMMY_OUTPUT()
    ADD_OUTPUT(unsigned char,  u1_patchCasio_true)
	ADD_OUTPUT(unsigned char,  u1_patchCasio_preTrue)
	ADD_OUTPUT(unsigned char,  u8_casino_hcnt)
	ADD_OUTPUT(unsigned char,  u8_casino_detCnt)


	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,  u1_patchArm_true)
	ADD_OUTPUT(unsigned char,  u1_patchArm_preTrue)
	ADD_OUTPUT(unsigned char,  u8_arm_hcnt)
	ADD_OUTPUT(unsigned char,  u8_arm_detCnt)
#endif

	DEBUGMODULE_END

#endif
