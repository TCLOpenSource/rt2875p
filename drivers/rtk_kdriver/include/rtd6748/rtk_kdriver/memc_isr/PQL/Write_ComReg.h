#ifndef _WRITE_COM_REG_H_
#define _WRITE_COM_REG_H_

#define _NOT_Ctrl  0
//#define BG_MV_GET
typedef struct
{
	unsigned char u1_mc_lbmcMode_en;

	unsigned char u1_FBLevelWr_en;

	unsigned char u1_RimCtrl_wrt_en;
	unsigned char u1_dhClose_wrt_en;

	unsigned char u1_LGClr_wrt_en;
	unsigned char u1_LGSC_ctrl_wrt_en;

	unsigned char u1_dynME_acdcBld_wrt_en;
	unsigned char u1_bigFBlvl_dis_SCdet_en;

	unsigned char u1_LGBlkHstyClr_wrt_en;
	unsigned char u1_LGPixHstyClr_wrt_en;

	unsigned char u1_LGHstyClr_ClrLG_wrt_en;

	unsigned char u1_LGRg_dhclr_ctrl_wrt_en;
	unsigned char u1_LGRg_detect_adp_wrt_en;

	unsigned char u1_LGRg_rim_unbalance_wrt_en;
	unsigned char u1_dynamic_obme_sel_wrt_en;

	unsigned char u1_LG_SC_PtClose_wrt_en;

	unsigned char   u8_logo_blksameth_l_bidx0_lg;
	unsigned char   u8_logo_blksameth_l_bidx0_com;

	unsigned char   u8_logo_blksameth_a_bidx0_lg;
	unsigned char   u8_logo_blksameth_a_bidx0_com;

	unsigned char   u8_logo_blksameth_a_bidx10_lg;
	unsigned char   u8_logo_blksameth_a_bidx10_com;

	unsigned char   u8_blksameth_a_bidx4;
	unsigned char   u8_blksameth_h_bidx4;

	unsigned char   u8_logo_blkhsty_pth_bidx10_lg;
	unsigned char   u8_logo_blkhsty_pth_bidx10_com;

	unsigned char   u8_logo_blkhsty_nth_bidx10_lg;
	unsigned char   u8_logo_blkhsty_nth_bidx10_com;

	unsigned char   u8_logo_mv_diff_diff_th_lg;
	unsigned char   u8_logo_mv_diff_sad_th_lg;
	unsigned char   u8_logo_tmv_sad_th_lg;
	unsigned char   u8_logo_tmv_num_th_lg;

	unsigned char   u1_LG_netflix_wrt_en;

	unsigned char   u1_LG_lg16s_patch_wrt_en;

	unsigned char   u1_pql_debug_info_Show_en;

	unsigned char   u1_localfb_wrt_en;//fast motion

	//
	unsigned char  u1_scCtrl_wrt_en;
	unsigned char  u1_MECand_wrt_en;
	unsigned char  u1_MECandOfst_wrt_en;
	unsigned char  u1_MECandPnt_wrt_en;
	unsigned char  u1_GMV_wrt_en;
	unsigned char  u1_ME_ipme_flp_alp_wrt_en;
	unsigned char  u1_ME_mvd_cuv_wrt_en;
	unsigned char  u1_ME_AdptPnt_rnd_wrt_en;
	unsigned char  u1_logoCtrl_wrt_en;
	unsigned char  u1_dh_logo_bypass_en;
	unsigned char  u1_MEacdcBld_wrt_en;
	//

	unsigned char  u1_dh_accord_ctrl_en;

	unsigned char u1_ME1_DRP_en;
	unsigned char u1_ME_vst_en;
	unsigned char u1_Identification_Pattern_en;

	unsigned char u1_ME_Cost_Ctrl;

	unsigned char u1_Wrt_ComReg_Proc_en;

	unsigned char u1_logo_sc_FastDet_wrt_en;
	unsigned char u1_MEUpdStgh_wrt_en;
	unsigned char u1_Unsharp_Mask_wrt_en;
	unsigned char u1_MCSobj_wrt_en;
	unsigned char u1_ME2Sobj_wrt_en;
	unsigned char u1_OcclExt_wrt_en;
	bool  u1_rFB_show_en;

	unsigned char u1_DW_fmdet_4region_En;

	unsigned char u1_mc_near_rim_blending_en;
	unsigned char u1_mc_hf_lpf_en;
	unsigned char u1_mc_localfb_en;
	unsigned char u1_logo_HaloEnahce_en;
	unsigned char u1_logo_DynOBME_en;

	// for sld
	unsigned char u1_logo_pxldfth_dyn_en;
	unsigned char u1_logo_pxlrgclr_en;
	
	unsigned char u1_logo_dhRgnProc_en;

	//	unsigned char u1_me1_dehalo_Newalgo_print;
    unsigned char u1_me1_Newalgo_Proc_en;  //normal
    unsigned char u1_dehalo_Newalgo_Proc_en;
        
    unsigned char u1_dehalo_condition1;
    unsigned char u1_dehalo_condition2;
    unsigned char u1_dehalo_condition3;
    unsigned char u1_dehalo_condition4;
    unsigned char u1_dehalo_condition5;
    unsigned char u1_dehalo_condition6;
    unsigned char u1_dehalo_condition7;
    unsigned char u1_dehalo_condition8;
    unsigned char u1_dehalo_condition9;
        
    unsigned char u1_me1_condition1;
    unsigned char u1_me1_condition2;
    unsigned char u1_me1_condition3;
    unsigned char u1_me1_condition4;
    unsigned char u1_me1_condition5;
    unsigned char u1_me1_condition6;
    unsigned char u1_me1_condition7;
    unsigned char u1_me1_condition8;
           
    unsigned char u1_me1_Newalgo_Proc_en_all;  //all enable
    unsigned char u1_dehalo_Newalgo_Proc_en_all;

	unsigned char u1_dynamicME_en;
	unsigned char u1_Dh_MV_Corr_en;
	unsigned char  u1_scholdCtrl_wrt_en;

	bool u1_SmallObject_en;
}_PARAM_WRT_COM_REG;

typedef struct
{
	unsigned char  u8_lf_lbmcMode_pre;
	unsigned char  u8_hf_lbmcMode_pre;

	unsigned char  u8_FB_lvl;
	unsigned char  u1_LG_mb_wrt_en;
	unsigned char  u1_casino_RP_detect_true;

	unsigned char  u1_memc_mute_state_pre;
	unsigned char  u1_IP_preProcess_true;
	unsigned char  u1_IP_wrtAction_true;
	unsigned char  u1_MC_Logo_Close_true;
	unsigned char  u1_ME1_rnd_mask_Slow_Convergence_true;
	unsigned char  u1_ME1_rnd_mask_Fast_Convergence_true;
	unsigned char  u1_ME1_rnd_Slow_Convergence_true;
	unsigned char  u1_ME1_rndCand_Slow_Convergence_type;
	unsigned char  u1_ME1_rndCanddpnt_Slow_Convergence_true;
	unsigned char  u1_ME1_rndCanddpnt_Fast_Convergence_type;
	unsigned char  u1_ME1_adptpnt_rnd_gain_true;
	unsigned char  u1_ME1_adptpnt_rnd_Slow_Convergence_true;
	unsigned char  u1_GFB_specialScene_true;
	unsigned char  u1_MC_SmallObject_type;
	unsigned char  u1_ME2_SmallObject_type;
	unsigned char  u1_LFB_Dtl_weak_type;
	unsigned char  u1_LFB_Dtl_increase_true;
	unsigned char  u1_LFB_force_zero_true;
	unsigned char  u1_GFB_force_weak_type;
	unsigned char  u1_GFB_force_zero_true;
	unsigned char  u1_GFB_force_MAX_true;
	unsigned char  u1_GFB_modify_true;
	unsigned char  u1_FILM_force_mix_true;
	unsigned char  u1_KME_SR_increaseX_true;
	unsigned char  u1_KME_SR_increaseY_true;
	unsigned char  u1_KME_gmvd_cost_type;
	unsigned char  u1_IPME_increase_flp_type;
	unsigned char  u1_ME1_DRP_type;
	unsigned char  u1_GFB_Special_apply;
	unsigned char  u8_01_Black_19YO1_flag ; //YE Test for tv006 01_Black_19YO1 issue
	unsigned char  u1_Q13722_flag ; //Q-13722 issue
	unsigned char  u1_GFB_force_Custom_true;
	unsigned char  u1_me1_bg_pfv_corr_cond ;
    unsigned char  u1_me1_sad_bg_cond  ;
    unsigned char  u1_me1_bg_ver_hor_candsel  ;
	unsigned char  u1_Pattern_Update_true;
	unsigned char  u1_BTS_Rp_flag ; //Q-13722 issue 
	unsigned char  u1_SquidGame_subTitle_flag ; //Q-13722 issue 
	unsigned char  u4_sc_table_lvl;
	unsigned char  u1_GFB_force_Custom_true2;
	unsigned char  u1_GFB_force_Custom_true3;
	unsigned char  u1_GFB_force_Custom_true4;
	unsigned char  u1_GFB_force_Custom_true5;
	unsigned char  u1_GFB_force_Custom_true6;
	unsigned char  u1_GFB_force_Custom_true7;
	unsigned char  u1_GFB_force_Custom_true8;
	unsigned char  u1_GFB_force_Custom_true9;
	unsigned char  u1_LFB_force_Custom_true1;
	unsigned char  u1_24P_judder_test;
	unsigned char  u1_is_test_video;
	unsigned char  u1_is_special_local_fb;
	
}_OUTPUT_WRT_COM_REG;

typedef enum{
	CTRL_N = 0,		// not control
	CTRL_Pluse,		// pluse
	CTRL_Minus,		// Minus
	CTRL_Replace,		// Replace
	CTRL_Default,		// Apply initial default
	CTRL_TYPE_NUM,
	CTRL_NONE = 255,
}VAL_CTRL_TYPE;

typedef enum{
	scCtrl_table_Default = 0,
	scCtrl_table_Seek,
	scCtrl_table_VideoCad,
	scCtrl_table_Panning,
	scCtrl_table_SpecialSCFB,
	scCtrl_table_Big_AplDiff,
	scCtrl_table_SpecifyCad,
	scCtrl_table_Other,
	scCtrl_table_Num
}scCtrl_table_TYPE;

typedef struct {
	VAL_CTRL_TYPE type;
	unsigned int	default_val;
	unsigned int 	ctrl_val;
}_str_wrtRegister_ctrl;

typedef struct {
	unsigned char		CTRL_TYPE;
	unsigned char		mc_scene_change_fb;
}_str_mc_scene_change_fb_ctrl;

typedef struct {
	unsigned char		CTRL_TYPE;
	unsigned int		me1_sc_saddiff_th;
}_str_me1_sc_saddiff_ctrl;

typedef struct {
	unsigned char		CTRL_TYPE;
	unsigned char		me1_sc_me1_hold_cnt;
}_str_sc_me1_hold_cnt_ctrl;

typedef struct {
	_str_mc_scene_change_fb_ctrl	mc_scene_change_fb_ctrl;
	_str_me1_sc_saddiff_ctrl		me1_sc_saddiff_ctrl;
	_str_sc_me1_hold_cnt_ctrl		me1_sc_me1_hold_ctrl;
}_str_scCtrl_table;

typedef struct {
	unsigned char		CTRL_TYPE;
	unsigned char		mc_var_lpf_en;
}_str_mc_var_lpf_en_ctrl;

typedef struct {
	unsigned char	 mc_fb_lvl_force_en;
	unsigned char	mc_fb_lvl_force_value;
	unsigned char	u1_mc2_var_lpf_wrt_en;//pql_patch_dummy_e4,[17]
	_str_mc_var_lpf_en_ctrl	mc_var_lpf_en_ctrl;
}_str_FRC_PureOffON_table;

typedef struct {
	unsigned char	vbuf_ip_dc_obme_mode_sel;
	unsigned char	vbuf_ip_dc_obme_mode;
	unsigned char	vbuf_ip_ac_obme_mode;

	unsigned char	vbuf_pi_dc_obme_mode_sel;
	unsigned char	vbuf_pi_dc_obme_mode;
	unsigned char	vbuf_pi_ac_obme_mode;

	unsigned char	kme_me2_vbuf_1st_dc_obme_mode_sel;
	unsigned char	kme_me2_vbuf_1st_dc_obme_mode;
	unsigned char	kme_me2_vbuf_1st_ac_obme_mode;

	unsigned char	kme_me2_vbuf_2nd_dc_obme_mode_sel;
	unsigned char	kme_me2_vbuf_2nd_dc_obme_mode;
	unsigned char	kme_me2_vbuf_2nd_ac_obme_mode;
}_str_ME_obmeSelection_table;


typedef struct {
	unsigned short vbuf_mvx_range;
	unsigned short kme_me2_vbuf_mvx_clip_range;
	unsigned short vbuf_mvy_range;
	unsigned short kme_me2_vbuf_mvy_clip_range;
}_str_ME_dynSR_table;

typedef struct {
	unsigned char	vbuf_ip_1st_cand0_en;
	unsigned char	vbuf_ip_1st_cand1_en;
	unsigned char	vbuf_ip_1st_cand2_en;
	unsigned char	vbuf_ip_1st_cand3_en;
	unsigned char	vbuf_ip_1st_cand4_en;
	unsigned char	vbuf_ip_1st_cand5_en;
	unsigned char	vbuf_ip_1st_cand6_en;
	unsigned char	vbuf_ip_1st_cand7_en;
	unsigned char	vbuf_ip_1st_cand8_en;
	unsigned char	vbuf_pi_1st_cand0_en;
	unsigned char	vbuf_pi_1st_cand1_en;
	unsigned char	vbuf_pi_1st_cand2_en;
	unsigned char	vbuf_pi_1st_cand3_en;
	unsigned char	vbuf_pi_1st_cand4_en;
	unsigned char	vbuf_pi_1st_cand5_en;
	unsigned char	vbuf_pi_1st_cand6_en;
	unsigned char	vbuf_pi_1st_cand7_en;
	unsigned char	vbuf_pi_1st_cand8_en;
	unsigned char	vbuf_ip_1st_update0_en;
	unsigned char	vbuf_ip_1st_update1_en;
	unsigned char	vbuf_ip_1st_update2_en;
	unsigned char	vbuf_ip_1st_update3_en;
	unsigned char	vbuf_pi_1st_update0_en;
	unsigned char	vbuf_pi_1st_update1_en;
	unsigned char	vbuf_pi_1st_update2_en;
	unsigned char	vbuf_pi_1st_update3_en;
	unsigned char	vbuf_ip_1st_zmv_en;	
	unsigned char	vbuf_pi_1st_zmv_en;
	//unsigned char	vbuf_ip_1st_gmv_en;
	//unsigned char	vbuf_pi_1st_gmv_en;
}_str_ME_cand_table;

typedef struct {
	unsigned char	vbuf_ip_1st_cand0_offsetx;
	unsigned char	vbuf_ip_1st_cand0_offsety;
	unsigned char	vbuf_ip_1st_cand1_offsetx;
	unsigned char	vbuf_ip_1st_cand1_offsety;
	unsigned char	vbuf_ip_1st_cand2_offsetx;
	unsigned char	vbuf_ip_1st_cand2_offsety;
	unsigned char	vbuf_ip_1st_cand3_offsetx;
	unsigned char	vbuf_ip_1st_cand3_offsety;
	unsigned char	vbuf_ip_1st_cand4_offsetx;
	unsigned char	vbuf_ip_1st_cand4_offsety;
	unsigned char	vbuf_ip_1st_cand5_offsetx;
	unsigned char	vbuf_ip_1st_cand5_offsety;
	unsigned char	vbuf_ip_1st_cand6_offsetx;
	unsigned char	vbuf_ip_1st_cand6_offsety;
	unsigned char	vbuf_ip_1st_cand7_offsetx;
	unsigned char	vbuf_ip_1st_cand7_offsety;
	unsigned char	vbuf_pi_1st_cand0_offsetx;
	unsigned char	vbuf_pi_1st_cand0_offsety;
	unsigned char	vbuf_pi_1st_cand1_offsetx;
	unsigned char	vbuf_pi_1st_cand1_offsety;
	unsigned char	vbuf_pi_1st_cand2_offsetx;
	unsigned char	vbuf_pi_1st_cand2_offsety;
	unsigned char	vbuf_pi_1st_cand3_offsetx;
	unsigned char	vbuf_pi_1st_cand3_offsety;
	unsigned char	vbuf_pi_1st_cand4_offsetx;
	unsigned char	vbuf_pi_1st_cand4_offsety;
	unsigned char	vbuf_pi_1st_cand5_offsetx;
	unsigned char	vbuf_pi_1st_cand5_offsety;
	unsigned char	vbuf_pi_1st_cand6_offsetx;
	unsigned char	vbuf_pi_1st_cand6_offsety;
	unsigned char	vbuf_pi_1st_cand7_offsetx;
	unsigned char	vbuf_pi_1st_cand7_offsety;
	unsigned char	vbuf_ip_1st_update0_offsetx;
	unsigned char	vbuf_ip_1st_update0_offsety;
	unsigned char	vbuf_ip_1st_update1_offsetx;
	unsigned char	vbuf_ip_1st_update1_offsety;
	unsigned char	vbuf_ip_1st_update2_offsetx;
	unsigned char	vbuf_ip_1st_update2_offsety;
	unsigned char	vbuf_ip_1st_update3_offsetx;
	unsigned char	vbuf_ip_1st_update3_offsety;
	unsigned char	vbuf_pi_1st_update0_offsetx;
	unsigned char	vbuf_pi_1st_update0_offsety;
	unsigned char	vbuf_pi_1st_update1_offsetx;
	unsigned char	vbuf_pi_1st_update1_offsety;
	unsigned char	vbuf_pi_1st_update2_offsetx;
	unsigned char	vbuf_pi_1st_update2_offsety;
	unsigned char	vbuf_pi_1st_update3_offsetx;
	unsigned char	vbuf_pi_1st_update3_offsety;
}_str_ME_candOfst_table;

typedef struct {
	unsigned char	me1_ip_cddpnt_rnd0;
	unsigned char	me1_ip_cddpnt_rnd1;
	unsigned char	me1_ip_cddpnt_rnd2;
	unsigned char	me1_ip_cddpnt_rnd3;
	unsigned char	me1_pi_cddpnt_rnd0;
	unsigned char	me1_pi_cddpnt_rnd1;
	unsigned char	me1_pi_cddpnt_rnd2;
	unsigned char	me1_pi_cddpnt_rnd3;
	unsigned char	me1_ip_cddpnt_st0;
	unsigned char	me1_ip_cddpnt_st1;
	unsigned char	me1_ip_cddpnt_st2;
	unsigned char	me1_ip_cddpnt_st3;
	unsigned char	me1_ip_cddpnt_st4;
	unsigned char	me1_ip_cddpnt_st5;
	unsigned char	me1_ip_cddpnt_st6;
	unsigned char	me1_ip_cddpnt_st7;
	unsigned char	me1_pi_cddpnt_st0;
	unsigned char	me1_pi_cddpnt_st1;
	unsigned char	me1_pi_cddpnt_st2;
	unsigned char	me1_pi_cddpnt_st3;
	unsigned char	me1_pi_cddpnt_st4;
	unsigned char	me1_pi_cddpnt_st5;
	unsigned char	me1_pi_cddpnt_st6;
	unsigned char	me1_pi_cddpnt_st7;
	unsigned short	me1_ip_cddpnt_zmv;
	unsigned short	me1_pi_cddpnt_zmv;
	unsigned short	me1_ip_cddpnt_gmv;
	unsigned short	me1_pi_cddpnt_gmv;
}_str_ME_caddpnt_table;

typedef struct {
	unsigned char	vbuf_ip_1st_lfsr_mask0_x;
	unsigned char	vbuf_ip_1st_lfsr_mask0_y;
	unsigned char	vbuf_ip_1st_lfsr_mask1_x;
	unsigned char	vbuf_ip_1st_lfsr_mask1_y;
	unsigned char	vbuf_ip_1st_lfsr_mask2_x;
	unsigned char	vbuf_ip_1st_lfsr_mask2_y;
	unsigned char	vbuf_ip_1st_lfsr_mask3_x;
	unsigned char	vbuf_ip_1st_lfsr_mask3_y;
	unsigned char	vbuf_pi_1st_lfsr_mask0_x;
	unsigned char	vbuf_pi_1st_lfsr_mask0_y;
	unsigned char	vbuf_pi_1st_lfsr_mask1_x;
	unsigned char	vbuf_pi_1st_lfsr_mask1_y;
	unsigned char	vbuf_pi_1st_lfsr_mask2_x;
	unsigned char	vbuf_pi_1st_lfsr_mask2_y;
	unsigned char	vbuf_pi_1st_lfsr_mask3_x;
	unsigned char	vbuf_pi_1st_lfsr_mask3_y;
}_str_ME_Vbuf_LFSR_updStgh_table;

typedef struct {
	unsigned int	me1_gmvcost_sel;
	unsigned int	me1_gmvd_gain;
	unsigned int	me2_gmvcost_sel;
	unsigned int	me2_gmvd_gain;
	unsigned int	pql_me1_gmvd_sel_en;
	unsigned int	me1_pi_gmvd_cost_limt;
	unsigned int	me1_ip_gmvd_cost_limt;
	unsigned int	me2_ph_gmvd_cost_limt;
	unsigned int	me1_gmvd_ucf_x1;
	unsigned int	me1_gmvd_ucf_x2;
	unsigned int	me1_gmvd_ucf_x3;
	unsigned int	me1_gmvd_ucf_y1;
	unsigned int	me1_gmvd_ucf_y2;
	unsigned int	me1_gmvd_ucf_y3;
	unsigned int	me1_gmvd_ucf_slope1;
	unsigned int	me1_gmvd_ucf_slope2;
	unsigned int	me1_gmvd_cnt_x1;
	unsigned int	me1_gmvd_cnt_x2;
	unsigned int	me1_gmvd_cnt_x3;
	unsigned int	me1_gmvd_cnt_y1;
	unsigned int	me1_gmvd_cnt_y2;
	unsigned int	me1_gmvd_cnt_y3;
	unsigned int	me1_gmvd_cnt_slope1;
	unsigned int	me1_gmvd_cnt_slope2;
}_str_ME_cost_table;

typedef struct {
	unsigned int	ipme_h_flp_alp0;
	unsigned int	ipme_h_flp_alp1;
	unsigned int	ipme_h_flp_alp2;
	unsigned int	ipme_h_flp_alp3;
	unsigned int	ipme_h_flp_alp4;
	unsigned int	ipme_h_flp_alp5;
	unsigned int	ipme_h_flp_alp6;
	unsigned int	ipme_v_flp_alp0;
	unsigned int	ipme_v_flp_alp1;
	unsigned int	ipme_v_flp_alp2;
	unsigned int	ipme_v_flp_alp3;
	unsigned int	ipme_v_flp_alp4;
	unsigned int	ipme_v_flp_alp5;
	unsigned int	ipme_v_flp_alp6;
}_str_ME_ipme_flp_alp_table;


typedef struct {
	unsigned int	me1_mvd_cuv_x1;
	unsigned int	me1_mvd_cuv_x2;
	unsigned int	me1_mvd_cuv_x3;
	unsigned int	me1_mvd_cuv_y1;
	unsigned int	me1_mvd_cuv_y2;
	unsigned int	me1_mvd_cuv_y3;
	unsigned int	me1_mvd_cuv_slope1;
	unsigned int	me1_mvd_cuv_slope2;
}_str_ME_mvd_cuv_table;


typedef struct {
	unsigned char	me1_drp_en;
	unsigned int	me1_drp_cuv_y2;
	unsigned int	me1_drp_cuv_y1;
	unsigned int	me1_drp_cuv_y0;
}_str_ME_DRP_table;

typedef struct {
	unsigned int	me1_adptpnt_rnd_y1;
	unsigned int	me1_adptpnt_rnd_y2;
	unsigned int	me1_adptpnt_rnd_y3;
	unsigned int	me1_adptpnt_rnd_y4;
	unsigned int 	me1_adptpnt_rnd_slope2;
}_str_ME_adptpnt_rnd_cuv_table;

typedef struct {
	unsigned char	dh_dtl_curve_y1;
	unsigned char	dh_dtl_curve_y2;
	unsigned char	dh_dtl_curve_x1;
	unsigned char	dh_dtl_curve_x2;
}_str_Dh_Local_FB_table;

typedef struct {
	unsigned char	mc_sobj_ymin0;
	unsigned char	mc_sobj_ymin1;
	unsigned char	mc_sobj_slop1;
}_str_MC_SObj_table;

typedef struct {
	unsigned int	me2_sec_small_object_sad_th;
	unsigned int	me2_sec_small_object_sm_mv_th;
	unsigned int	me2_sec_small_object_ip_pi_mvdiff_gain;
	unsigned int 	me2_sec_small_object_ph_mvdiff_gain;
}_str_ME2_SObj_table;

typedef struct
{
	unsigned short R00;
	unsigned short R01;
	unsigned short R02;
	unsigned short R10;
	unsigned short R11;
	unsigned short R12;
	unsigned short R20;
	unsigned short R21;
	unsigned short R22;

	unsigned short T0;
	unsigned short T1;
	unsigned short T2;
} MEMC_ColorTable;

typedef struct
{
	MEMC_ColorTable RGB2YUV;
	MEMC_ColorTable YUV2RGB;
} MEMC_ColorTable_Pair;

#if 1
typedef enum _MEMC_PERFORMANCE_SETTING_TABLE {
	MEMC_PERFORMANCE_SETTING_reg = 0,
	MEMC_PERFORMANCE_SETTING_bitup,
	MEMC_PERFORMANCE_SETTING_bitlow,
	MEMC_PERFORMANCE_SETTING_value,
	MEMC_PERFORMANCE_SETTING_MAX,
} MEMC_PERFORMANCE_SETTING_TABLE;

#endif

#if 0
typedef struct {
	unsigned char	mc_fb_lvl_force_en;
	unsigned char	mc_logo_phase_fb_phase_en;
	unsigned char	mc_pre_phase_fb_phase_en;
	unsigned char 	mc_pt_phase_fb_phase_en;
	unsigned char 	mc_fb_lvl_force_value;
}_str_PanningDetect_table;
#endif

VOID Wrt_ComReg_Init(_OUTPUT_WRT_COM_REG *pOutput);
_str_scCtrl_table get_scCtrl_table(unsigned char u8_id);
VOID RimCtrl_wrtAction(const _PARAM_WRT_COM_REG *pParam);
VOID Wrt_ComReg_Proc_intputInterrupt(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput);
VOID Wrt_ComReg_Proc_outputInterrupt(const _PARAM_WRT_COM_REG *pParam, _OUTPUT_WRT_COM_REG *pOutput);
VOID MEMC_Lib_Manual_colormatrix(_OUTPUT_ME_SCENE_ANALYSIS *pSceneInfo);
BOOL MEMC_HSR_ON_OFF_ChangeStatus(void);
BOOL Check_MEMC_DMA_Error(void);


DEBUGMODULE_BEGIN(_PARAM_WRT_COM_REG, _OUTPUT_WRT_COM_REG)
	ADD_PARAM_DES(unsigned char, u1_mc_lbmcMode_en,            1,  "")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char, u1_FBLevelWr_en,              1,  "")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char, u1_RimCtrl_wrt_en,            1, "")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char, u1_dhClose_wrt_en,            1, "")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char, u1_LGClr_wrt_en,              1, "")
	ADD_PARAM_DES(unsigned char, u1_LGSC_ctrl_wrt_en,          0, "")

	ADD_DUMMY_PARAM()
	ADD_PARAM_DES(unsigned char, u1_dynME_acdcBld_wrt_en,         0, "dynamic ME, fadeInout write acdc bld Enable")

	////////  output  //////////////////////////////////////////////////////////////////

//	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char, u8_lf_lbmcMode_pre)
	ADD_OUTPUT(unsigned char, u8_hf_lbmcMode_pre)

	DEBUGMODULE_END

#endif
