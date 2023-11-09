#ifndef _READ_COMREG_H
#define _READ_COMREG_H

#define LOGO_RG_32  (32)


typedef struct
{
	unsigned char  u1_me_VbufSize_rb_en;
	unsigned char  u1_sc_status_rb_en;

	unsigned char  u1_me_aTC_rb_en;
	unsigned char  u1_me_aSC_rb_en;
	unsigned char  u1_me_aSAD_rb_en;
	unsigned char  u1_me_aDTL_rb_en;
	unsigned char  u1_me_statis_glb_prd_en;
	unsigned char  u1_me_aAPLi_rb_en;
	unsigned char  u1_me_aAPLp_rb_en;

	unsigned char  u1_me_ZMV_cnt_rb_en;
	unsigned char  u1_me_ZMV_dtl_cnt_rb_en;
	unsigned char  u1_me_ZMV_psad_rb_en;
	unsigned char  u1_me_ZGMV_cnt_rb_en;

	unsigned char  u1_me_Hpan_cnt_rb_en;
	unsigned char  u1_me_Vpan_cnt_rb_en;
	unsigned char  u1_me_pnMV_cnt_rb_en;

	unsigned char  u1_me_GMV_1st_rb_en;
	unsigned char  u1_me_GMV_2nd_rb_en;

	unsigned int  u32_me_rTC_rb_en;
	unsigned int  u32_me_rSC_rb_en;
	unsigned int  u32_me_rSAD_rb_en;
	unsigned int  u32_me_rDTL_rb_en;
	unsigned int  u32_me_rAPLi_rb_en;
	unsigned int  u32_me_rAPLp_rb_en;
	unsigned int  u32_me_rPRD_rb_en;
	unsigned int  u32_me_rLFB_rb_en;

	unsigned int  u32_me_rMV_1st_rb_en;
	unsigned int  u32_me_rMV_2nd_rb_en;

	unsigned char  u1_ipme_aMot_rb_en;
	unsigned char  u1_ipme_5rMot_rb_en;
	unsigned char  u1_ipme_12rMot_rb_en;
	unsigned char  u1_ipme_filmMode_rb_en;

	unsigned char  u1_BBD_rb_en;

	unsigned char  u1_lg_blk_cnt_rb_en;

	unsigned char  u1_kphase_rb_en;

	unsigned char  u2_rb_tc_mode;
	unsigned char  u4_rb_tc1_coef;
	unsigned char  u4_rb_tc2_coef;
	unsigned char  u4_rb_tc3_coef;

	unsigned char  u1_dh_rb_en;

	unsigned char  u1_low_delay_test01;
	unsigned char  u1_low_delay_ctrl2;
	unsigned char  u1_low_delay_ctrl3;
	unsigned char  u1_Debug_OnlineMeasure;

	unsigned char  u1_mc_HVdtl_rb_en;
	unsigned char  u1_me_ZMV_cnt_en;

	unsigned char  u1_color_amount_rb_en;

	unsigned char  u1_powerSaving_en;
	//0xb809d700
	unsigned char  u1_HDMI_powerSaving_en;
	unsigned char  u1_jerry_PanAct_en;
	unsigned char  u1_forcePhTable_Regen;
	unsigned char  u1_ResetFlow_en;
	unsigned char  u1_force_cadence_en;
	unsigned char  u4_force_cadence_type;
	unsigned char  u1_SEFilm_fixCadence_flag;
	unsigned char  u1_SEFilm_powerSaving_flag;
	unsigned char  u4_SEFilm_force_cadence_type;

	bool pattern_golf_flag;
}_PARAM_ReadComReg;


typedef struct
{
	unsigned char  u8_me1_Vbuf_Hsize_rb;
	unsigned char  u8_me1_Vbuf_Vsize_rb;
	unsigned char  u1_sc_status_rb;			 //me1
	unsigned char  u1_sc_status_logo_rb;     //logo
	unsigned char  u1_sc_status_dh_rb;      //dh

	unsigned int u27_me_aTC_rb;
	unsigned int u27_me_aSC_rb;
	
	unsigned int u30_me_aSAD_rb;
	unsigned int u25_me_aDTL_rb;
	unsigned int u26_me_aAPLi_rb;
	unsigned int u26_me_aAPLp_rb;


	unsigned int u17_me_ZMV_cnt_rb;
	unsigned int u17_me_ZMV_dtl_cnt_rb;
	unsigned int u31_me_ZMV_psad_dc_rb;
	unsigned int u31_me_ZMV_psad_ac_rb;
	unsigned int u20_me_ZMV_DcGreatAc_cnt_rb;
	unsigned int u20_me_ZMV_AcGreatDc_cnt_rb;
	unsigned char  u6_me_ZGMV_cnt_rb;

	unsigned int u20_me_Hpan_cnt_rb;
	unsigned int u20_me_Vpan_cnt_rb;

	unsigned int u17_me_posCnt_0_rb;
	unsigned int u17_me_posCnt_1_rb;
	unsigned int u17_me_posCnt_2_rb;
	unsigned int u17_me_posCnt_3_rb;

	unsigned int u17_me_negCnt_0_rb;
	unsigned int u17_me_negCnt_1_rb;
	unsigned int u17_me_negCnt_2_rb;
	unsigned int u17_me_negCnt_3_rb;


	signed short s11_me_mv_max_vx_rb;
	signed short s10_me_mv_max_vy_rb;
	signed short s11_me_mv_min_vx_rb;
	signed short s10_me_mv_min_vy_rb;
	signed short s11_me_GMV_1st_vx_rb;
	signed short s10_me_GMV_1st_vy_rb;
	unsigned int u17_me_GMV_1st_cnt_rb;
	unsigned short u12_me_GMV_1st_unconf_rb;

	signed short s11_me_GMV_2nd_vx_rb;
	signed short s10_me_GMV_2nd_vy_rb;
	unsigned int u17_me_GMV_2nd_cnt_rb;
	unsigned short u12_me_GMV_2nd_unconf_rb;



	unsigned short  u16_mc_Hdtl_cnt;
	unsigned short  u16_mc_Vdtl_cnt;
	unsigned short  u10_mc_VHdtl_rb[32];

	unsigned int u22_me_rTC_rb[32];
	unsigned int u22_me_rSC_rb[32];
	unsigned int u25_me_rSAD_rb[32];
	unsigned int u20_me_rDTL_rb[32];
	unsigned int u20_me_rAPLi_rb[32];
	unsigned int u20_me_rAPLp_rb[32];
	unsigned int u13_me_rPRD_rb[32];
	unsigned int u12_me_rLFB_rb[32];

	signed short s11_me_rMV_1st_vx_rb[32];
	signed short s10_me_rMV_1st_vy_rb[32];
	unsigned short u12_me_rMV_1st_cnt_rb[32];
	unsigned short u12_me_rMV_1st_unconf_rb[32];

	signed short s11_me_rMV_2nd_vx_rb[32];
	signed short s10_me_rMV_2nd_vy_rb[32];
	unsigned short u12_me_rMV_2nd_cnt_rb[32];
	unsigned short u12_me_rMV_2nd_unconf_rb[32];

	//
	unsigned int u27_ipme_aMot_rb;
	unsigned char  u3_ipme_filmMode_rb;
	unsigned int u27_ipme_5Mot_rb[5];
	unsigned int u27_ipme_12Mot_rb[12];

	//
	unsigned short u12_BBD_roughRim_rb[4];
	unsigned char  u1_BBD_roughValid_rb[4];
	unsigned short u12_BBD_fineRim_rb[4];
	unsigned char  u1_BBD_fineValid_rb[4];

	unsigned int u12_ME1_blk_size[2];
	unsigned int u12_ME1_pix_size[2];
	unsigned int u12_ME2_blk_size[2];
	unsigned int u12_ME2_pix_size[2];


	//Logo cnt
	unsigned int u10_blklogo_rgcnt[LOGO_RG_32];

	unsigned int u16_blklogo_dh_clrcnt[4];

	unsigned int u16_blklogo_dh_clrcnt_bot[3];
	unsigned int u16_blklogo_dh_clrcnt_top[3];

	unsigned int u14_pxllogo_rgcnt[32];

	unsigned int u23_sub_histo_meter[8];
	unsigned int u32_yuv_meter[32];

	//
	unsigned char  u1_kphase_inLR;
	unsigned char  u4_kphase_inPhase;
	unsigned char  u8_sys_N_rb;
	unsigned char  u8_sys_M_rb;

	unsigned short u13_ip_vtrig_dly;
	unsigned short u13_me2_org_vtrig_dly;
	unsigned short u13_me2_vtrig_dly;
	unsigned short u13_dec_vtrig_dly;
	unsigned short u13_vtotal;

	unsigned short u16_inHTotal;
	unsigned short u16_inVAct;
	unsigned short u16_inFrm_Pos;

	//dh
	unsigned int u20_dh_pfvconf[32];

	//color amount
	unsigned short u16_color_amount_u[8];
	unsigned short u16_color_amount_v[8];
	// repeat pattern detection
	unsigned int u19_me_statis_glb_prd;

	// gmv_blend & regional_fallback
	unsigned char u8_gmv_blend_en;
	unsigned char u8_regional_fb_en;
	unsigned char u8_region_fb_max[32];
	unsigned short u12_region_fb_cnt[32];
	unsigned int u32_region_fb_sum[32];

	signed short s11_rgn_mvx_max;
	unsigned short u12_rgn_mvx_max_cnt;
	unsigned int u8_rgn_mvx_max_index;
	signed short s11_rgn_mvy_max;
	unsigned short u12_rgn_mvy_max_cnt;
	unsigned int u8_rgn_mvy_max_index;
	unsigned short u12_rgn_max_unconf;
	signed short s11_rgn_max_unconf_mvx;
	signed short s11_rgn_max_unconf_mvy;
	unsigned short u12_rgn_max_unconf_cnt;
	unsigned int u8_rgn_max_unconf_index;
	signed short s11_gmv_diff_maxgmv_mvx;
	signed short s11_gmv_diff_maxgmv_mvy;
	//unsigned char u8_sad_overshot_flag;
	signed short dh_gmv_fgx;  //dehalo me1 read
    signed short dh_gmv_fgy;
    signed short dh_gmv_bgx;
    signed short dh_gmv_bgy;
    signed short dh_bg_apl_seg0_cnt;
    signed short dh_bg_apl_seg1_cnt;
    signed short dh_bg_apl_seg2_cnt;
    signed short dh_bg_apl_seg3_cnt;
    signed short dh_bg_apl_seg4_cnt;
    signed short dh_bg_apl_seg5_cnt;
    signed short dh_bg_apl_seg6_cnt;
    signed short dh_bg_apl_seg7_cnt;
    signed short dh_fg_apl_seg0_cnt;
    signed short dh_fg_apl_seg1_cnt;
    signed short dh_fg_apl_seg2_cnt;
    signed short dh_fg_apl_seg3_cnt;
    signed short dh_fg_apl_seg4_cnt;
    signed short dh_fg_apl_seg5_cnt;
    signed short dh_fg_apl_seg6_cnt;
    signed short dh_fg_apl_seg7_cnt;
    signed short me1_gmv_bgcnt;    
    // int rgn_apli[32];
    //  int rgn_bgcnt[32];

	signed short s11_fg_rMV_x_rb[32];
    signed short s10_fg_rMV_y_rb[32];
    signed short s11_bg_rMV_x_rb[32];
    signed short s10_bg_rMV_y_rb[32];

	unsigned short u16_bg_rgn_cnt[32]; 
}_OUTPUT_ReadComReg;

VOID ReadComReg_Init( _OUTPUT_ReadComReg *pOutput);
VOID ReadComReg_Proc(const _PARAM_ReadComReg *pParam, _OUTPUT_ReadComReg *pOutput);

VOID ReadComReg_Proc_oneFifth_OutInterrupt(const _PARAM_ReadComReg *pParam, _OUTPUT_ReadComReg *pOutput);

VOID ReadComReg_Proc_inputInterrupt_Hsync(const _PARAM_ReadComReg *pParam, _OUTPUT_ReadComReg *pOutput);

VOID ReadComReg_Proc_inputInterrupt(const _PARAM_ReadComReg *pParam, _OUTPUT_ReadComReg *pOutput);
VOID ReadComReg_Proc_outputInterrupt(const _PARAM_ReadComReg *pParam, _OUTPUT_ReadComReg *pOutput);

DEBUGMODULE_BEGIN(_PARAM_ReadComReg,_OUTPUT_ReadComReg)

	ADD_PARAM(unsigned char,   u1_me_VbufSize_rb_en,     1)
	ADD_PARAM(unsigned char,   u1_sc_status_rb_en,       1)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,   u1_me_aTC_rb_en,          1)
	ADD_PARAM(unsigned char,   u1_me_aSC_rb_en,          0)
	ADD_PARAM(unsigned char,   u1_me_aSAD_rb_en,         1)
	ADD_PARAM(unsigned char,   u1_me_aDTL_rb_en,         0)
	ADD_PARAM(unsigned char,   u1_me_aAPLi_rb_en,        1)
	ADD_PARAM(unsigned char,   u1_me_aAPLp_rb_en,        1)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,   u1_me_ZMV_cnt_rb_en,      0)
	ADD_PARAM(unsigned char,   u1_me_ZMV_dtl_cnt_rb_en,  0)
	ADD_PARAM(unsigned char,   u1_me_ZMV_psad_rb_en,     1)
	ADD_PARAM(unsigned char,   u1_me_ZGMV_cnt_rb_en,     0)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,   u1_me_Hpan_cnt_rb_en,     0)
	ADD_PARAM(unsigned char,   u1_me_Vpan_cnt_rb_en,     0)
	ADD_PARAM(unsigned char,   u1_me_pnMV_cnt_rb_en,     1)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,   u1_me_GMV_1st_rb_en,      0)
	ADD_PARAM(unsigned char,   u1_me_GMV_2nd_rb_en,      0)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned int,  u32_me_rTC_rb_en,         0)
	ADD_PARAM(unsigned int,  u32_me_rSC_rb_en,         0)
	ADD_PARAM(unsigned int,  u32_me_rSAD_rb_en,        0)
	ADD_PARAM(unsigned int,  u32_me_rDTL_rb_en,        0)
	ADD_PARAM(unsigned int,  u32_me_rAPLi_rb_en,       0)
	ADD_PARAM(unsigned int,  u32_me_rAPLp_rb_en,       0)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned int,  u32_me_rMV_1st_rb_en,     0)
	ADD_PARAM(unsigned int,  u32_me_rMV_2nd_rb_en,     0)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,  u1_ipme_aMot_rb_en,        0)
	ADD_PARAM(unsigned char,  u1_ipme_5rMot_rb_en,       0)
	ADD_PARAM(unsigned char,  u1_ipme_12rMot_rb_en,      0)
	ADD_PARAM(unsigned char,  u1_ipme_filmMode_rb_en,    1)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,  u1_BBD_rb_en,              1)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,  u1_lg_blk_cnt_rb_en,       1)

	ADD_DUMMY_PARAM()
	ADD_PARAM(unsigned char,  u1_kphase_rb_en,           0)

	////////////  OUTPUT  //////////////////////////////////////////////////////////////
	ADD_OUTPUT(unsigned char,         u8_me1_Vbuf_Hsize_rb)
	ADD_OUTPUT(unsigned char,         u8_me1_Vbuf_Vsize_rb)
	ADD_OUTPUT(unsigned char,         u1_sc_status_rb)
	ADD_OUTPUT(unsigned char,         u1_sc_status_logo_rb)
	ADD_OUTPUT(unsigned char,         u1_sc_status_dh_rb)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned int,         u27_me_aTC_rb)
	ADD_OUTPUT(unsigned int,         u27_me_aSC_rb)
	ADD_OUTPUT(unsigned int,         u30_me_aSAD_rb)
	ADD_OUTPUT(unsigned int,         u25_me_aDTL_rb)
	ADD_OUTPUT(unsigned int,         u26_me_aAPLi_rb)
	ADD_OUTPUT(unsigned int,         u26_me_aAPLp_rb)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned int,        u17_me_ZMV_cnt_rb)
	ADD_OUTPUT(unsigned int,        u17_me_ZMV_dtl_cnt_rb)
	ADD_OUTPUT(unsigned int,        u31_me_ZMV_psad_dc_rb)
	ADD_OUTPUT(unsigned int,        u31_me_ZMV_psad_ac_rb)
	ADD_OUTPUT(unsigned int,        u20_me_ZMV_DcGreatAc_cnt_rb)
	ADD_OUTPUT(unsigned int,        u20_me_ZMV_AcGreatDc_cnt_rb)
	ADD_OUTPUT(unsigned char,         u6_me_ZGMV_cnt_rb)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned int,        u17_me_Hpan_cnt_rb)
	ADD_OUTPUT(unsigned int,        u17_me_Vpan_cnt_rb)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned int,        u17_me_posCnt_0_rb)
	ADD_OUTPUT(unsigned int,        u17_me_posCnt_1_rb)
	ADD_OUTPUT(unsigned int,        u17_me_posCnt_2_rb)
	ADD_OUTPUT(unsigned int,        u17_me_posCnt_3_rb)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned int,         u17_me_negCnt_0_rb)
	ADD_OUTPUT(unsigned int,         u17_me_negCnt_1_rb)
	ADD_OUTPUT(unsigned int,         u17_me_negCnt_2_rb)
	ADD_OUTPUT(unsigned int,         u17_me_negCnt_3_rb)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(signed short,        s11_me_GMV_1st_vx_rb)
	ADD_OUTPUT(signed short,        s10_me_GMV_1st_vy_rb)
	ADD_OUTPUT(unsigned char,         u17_me_GMV_1st_cnt_rb)
	ADD_OUTPUT(unsigned short,        u12_me_GMV_1st_unconf_rb)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(signed short,        s11_me_GMV_2nd_vx_rb)
	ADD_OUTPUT(signed short,        s10_me_GMV_2nd_vy_rb)
	ADD_OUTPUT(unsigned char,         u17_me_GMV_2nd_cnt_rb)
	ADD_OUTPUT(unsigned short,        u12_me_GMV_2nd_unconf_rb)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned int,         u27_ipme_aMot_rb)
	ADD_OUTPUT(unsigned char,          u3_ipme_filmMode_rb)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT_ARRAY_DES(unsigned int, u25_me_rSAD_rb, 32, "rgn sad")

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT_ARRAY_DES(unsigned int, u10_blklogo_rgcnt, LOGO_RG_32, "blk logo cnt")

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,          u1_kphase_inLR)
	ADD_OUTPUT(unsigned char,          u4_kphase_inPhase)
	ADD_OUTPUT(unsigned int,         u19_me_statis_glb_prd)

DEBUGMODULE_END

#endif

