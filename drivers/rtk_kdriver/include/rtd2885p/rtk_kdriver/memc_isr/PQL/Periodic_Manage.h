#ifndef _PERIODIC_MANAGE_DEF_H
#define _PERIODIC_MANAGE_DEF_H

#ifndef CONFIG_REPEAT_NEW_ALOG
#define CONFIG_REPEAT_NEW_ALOG (1)
#endif

#define PRD_STATIC_RMV_NUM	(32)
typedef struct
{
    unsigned char u1_update_hist_en;

} _PARAM_PERIODIC_MANAGE;

/*
	SMART INTEGRATION STATUS (SIS) for MEMC ver. by LearnRPG@20150911

	Collection and combination all those status by long time experience.
	We suggest in kernal API don't have below code
	(that's mean kernal API must independent operation)
	1. no global variable, must use function parameter be input.
	2. don't change global status in function, must use middle layer to update status.
*/

typedef struct _PeriodicStatus
{
	//============ detect ===
    unsigned int ipme2_freq_maxfoot_minfoot_diff;
    unsigned int ipme2_freq_maxpeak_minpeak_diff;
    unsigned int ipme2_freq_maxpeak_minfoot_diff;
    unsigned int ipme2_freq_me0_med3_en;
    unsigned int ipme2_freq_me1_med3_en;
    unsigned int ipme2_freq_max_min_en;
    unsigned int ipme2_freq_me0_post_lpf_en;
    unsigned int ipme2_freq_me1_post_lpf_en;
    unsigned int ipme2_freq_center_match_en;
    unsigned int ipme2_freq_me0_detect_en;
    unsigned int ipme2_freq_me1_detect_en;

    unsigned int ipme2_freq_me0_med3_th_v;
    unsigned int ipme2_freq_me0_med3_th_h;
    unsigned int ipme2_freq_me1_med3_th_v;
    unsigned int ipme2_freq_me1_med3_th_h;
    unsigned int ipme2_freq_me0_energy_gain_v;
    unsigned int ipme2_freq_me0_energy_gain_h;
    unsigned int ipme2_freq_me1_energy_gain_v;
    unsigned int ipme2_freq_me1_energy_gain_h;

    unsigned int ipme2_freq_pre_cur_diff;
    unsigned int ipme2_freq_energy_th3;
    unsigned int ipme2_freq_energy_th2;
    unsigned int ipme2_freq_energy_th1;
	//======================

	//============ apply ===
    unsigned int me1_avgbv_erosion_en;
    unsigned int me1_statis_freq_loop;
    unsigned int me1_freq_debugmode;
    unsigned int me1_freq_candidate_compare_random;
    unsigned int me1_freq_candidate_compare_temporal;
    unsigned int me1_freq_candidate_compare_spatial;
    unsigned int me1_freq_force_bv_replace_mode;
    unsigned int me1_freq_force_bv_replace_en;
    unsigned int me1_freq_avgbv_bv_diff_penalty_gain;
    unsigned int me1_freq_avgbv_bv_diff_penalty_offset;
    unsigned int me1_freq_avgbv_bv_diff_penalty_en;
    unsigned int me1_freq_random_mv_replace;
    unsigned int me1_freq_random_mask;
    unsigned int me1_freq_candidate_temporal_dis;
    unsigned int me1_freq_candidate_spatial_dis;
    unsigned int me1_freq_obme_mode;
    unsigned int me1_freq_obme_mode_sel;
    unsigned int me1_freq_obme_en;

    unsigned int me1_freq_avgbv_mv_cnt_th;
    unsigned int me1_freq_avgbv_hist_close_th;
    unsigned int me1_freq_avgbv_hist_close_en;
	//======================
} PeriodicStatus;

typedef struct _ScalerMEMCPeriodicStatus
{
	PeriodicStatus ORIGIN;

	unsigned int Frame_Couter;

} ScalerMEMCPeriodicStatus;

typedef struct
{
    ScalerMEMCPeriodicStatus PERIODIC_STATUS;

    unsigned char u1_detect;
	unsigned char u1_detec_repeat_cnt;
	unsigned char u1_SC_disable_algo;
	unsigned char u1_SC_cnt;
	unsigned char u1_rgn_stable;
	unsigned char u8_rmv_unstable_cnt[4][8];

    unsigned short u10_freq_statistic_cnt1[4][8];
    unsigned short u10_freq_statistic_cnt2[4][8];
    unsigned short u10_freq_statistic_cnt3[4][8];

	unsigned short u15_max_freq_statistic_cnt1;
	unsigned short u15_max_freq_statistic_cnt2;
	unsigned short u15_max_freq_statistic_cnt3;

	unsigned short u15_sum_freq_statistic_cnt1;
	unsigned short u15_sum_freq_statistic_cnt2;
	unsigned short u15_sum_freq_statistic_cnt3;

    unsigned short u10_freq_statistic_cnt1_IIR[4][8];
    unsigned short u10_freq_statistic_cnt2_IIR[4][8];
    unsigned short u10_freq_statistic_cnt3_IIR[4][8];

	short s11_freg_statistic_avgbv_mvx[PRD_STATIC_RMV_NUM][4][8];
	short s10_freg_statistic_avgbv_mvy[PRD_STATIC_RMV_NUM][4][8];
} _OUTPUT_PERIODIC_MANAGE;

enum
{
	MEMC_PERIODIC_LEVEL_DISABLE = 0,
	MEMC_PERIODIC_LEVEL_LOW,
	MEMC_PERIODIC_LEVEL_MID, // initial
	MEMC_PERIODIC_LEVEL_HIGH,
	MEMC_PERIODIC_LEVEL_NUM
};

// enum
// {
//     /* 0 */ SEC0 = 0,
//     /* 1 */ SEC1,
//     /* 2 */ SEC2,
//     /* 3 */ SEC3,
//     /* 4 */ SEC_NUM
// };

typedef struct
{
    bool  b_prd_auto_en;
	unsigned char u8_prd_manual_mode;
	unsigned char u8_max_mvdiff_th;
	bool u8_r32_indicator_en;
	unsigned char u8_criteria_en;
	unsigned char u8_test1_th;
	unsigned char u8_test2_th;
	unsigned char u8_test3_th;
	unsigned char u8_test4_th;
	unsigned int  u5_prd_valid_cnt_th;
	
} _PARAM_PRD_MANAGE;


typedef struct _PrdParam
{
	// 0xb802edb0
	unsigned int me1_cand_shrink_mask_en;
	unsigned int me1_cand_shrink_mask_x;
	unsigned int me1_cand_shrink_mask_y;
	unsigned int me1_rejudge_occl_en;
	unsigned int me1_rejudge_nearGMV_en;
	unsigned int me1_rejudge_nearGMV_th;
	unsigned int me1_cand_chk_redd_en;
	unsigned int me1_cand_redd_invalid;
	unsigned int me1_cand_chk_invalid_en;
	unsigned int me1_cand_avgbv_diff0_th;
	unsigned int me1_cand_dediff_rp_pnt_rnd_en;
	// 0xb802edb4
	unsigned int me1_cand_avgbv_diff_th;
	unsigned int me1_cand_dediff_bMV_sel;
	unsigned int me1_cand_dediff_sim_step;
	unsigned int me1_cand_dediff_sim_mvd_gain;
	unsigned int me1_cand_dediff_sim_th;
	unsigned int me1_cand_dediff_avgbv_diff_th;
	// 0xb802edb8
	unsigned int me1_cand_dediff_lmv_step;
	unsigned int me1_cand_dediff_bMV_x;
	unsigned int me1_cand_dediff_bMV_y;
	// 0xb802edbc
	unsigned int me1_dam_pnt_en;
	unsigned int me1_st_apld_pnt_coef;
	unsigned int me1_st_dtld_pnt_coef;
	unsigned int me1_rand_apld_pnt_coef;
	unsigned int me1_rand_dtld_pnt_coef;
	unsigned int me1_dam_pnt_shft;
	// 0xb802edc0
	unsigned int me1_freq_pnt_en;
	unsigned int me1_cand_freq_det_th;
	unsigned int me1_freq_pnt_gain;
	unsigned int me1_cand_dediff_bMV_pnt;
	// 0xb802edc4
	unsigned int me1_new_mvd_cost_en;
	unsigned int me1_apl_var_step;
	unsigned int me1_new_mvd_cost_shft;
	// 0xb802edc8
	unsigned int me1_mvd_again_x1;
	unsigned int me1_mvd_again_x2;
	unsigned int me1_mvd_again_x3;
	unsigned int me1_mvd_again_y1;
	// 0xb802edcc
	unsigned int me1_mvd_again_y2;
	unsigned int me1_mvd_again_y3;
	unsigned int me1_mvd_again_slope1;
	unsigned int me1_mvd_again_slope2;
} PrdParam;

typedef struct
{
    unsigned char prd_level;
	bool r32_level[32];

} _OUTPUT_PRD_MANAGE;


VOID Periodic_Init(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_Proc(const _PARAM_PERIODIC_MANAGE *pParam, _OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_Detect(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_Detect_SC(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_Detect_Cnt3(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_Detect_Unstable_Rmv(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_StatusInit(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_StatusUpdate(const _PARAM_PERIODIC_MANAGE *pParam, _OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_Apply(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_Apply_Global_AvgBv(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_Apply_Region_AvgBv(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_UpdateInfo(_OUTPUT_PERIODIC_MANAGE *pOutput);
VOID Periodic_ChangeLevel(_OUTPUT_PERIODIC_MANAGE *pOutput, int level);

VOID PRD_Init(_OUTPUT_PRD_MANAGE *pOutput);
VOID PRD_Proc(const _PARAM_PRD_MANAGE *pParam, _OUTPUT_PRD_MANAGE *pOutput, _OUTPUT_PERIODIC_MANAGE *pOutput2);
VOID PRD_Detect(const _PARAM_PRD_MANAGE *pParam, _OUTPUT_PRD_MANAGE *pOutput, _OUTPUT_PERIODIC_MANAGE *pOutput2);
VOID PRD_Apply(_OUTPUT_PRD_MANAGE *pOutput);
VOID Region32_indicator (const _PARAM_PRD_MANAGE *pParam, _OUTPUT_PRD_MANAGE *pOutput, _OUTPUT_PERIODIC_MANAGE *pOutput2);


#endif
