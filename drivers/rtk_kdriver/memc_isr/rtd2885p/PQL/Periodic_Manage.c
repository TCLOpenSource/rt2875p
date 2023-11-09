#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_isr/PQL/Periodic_Manage.h"
#include "memc_reg_def.h"
#include "memc_isr/PQL/CalibMEMCInfo.h"
//#include <scaler/scaler_types.h>
#define TAG_NAME "MEMC"

#define PERIODIC_MANAGE_DEBUG_PRINT 0
#define NEW_REPEAT_NEW_ALGO_REMOVE_M8 (0)

#if CONFIG_REPEAT_NEW_ALOG
static PeriodicStatus gMEMC_PeriodicParamTable[MEMC_PERIODIC_LEVEL_NUM] =
{
    /* DISABLE */
    {
        128, // maxfoot_minfoot_diff
        128, // maxpeak_minpeak_diff
        16,  // maxpeak_minfoot_diff
        0,   // me0_med3_en
        1,   // me1_med3_en
        1,   // max_min_en
        1,   // me0_post_lpf_en
        1,   // me1_post_lpf_en
        1,   // center_match_en
        1,   // me0_detect_en
        1,   // me1_detect_en

        6,   // me0_med3_th_v
        11,  // me0_med3_th_h
        6,   // me1_med3_th_v
        11,  // me1_med3_th_h
        8,   // me0_energy_gain_v
        5,   // me0_energy_gain_h
        13,  // me1_energy_gain_v
        5,   // me1_energy_gain_h

        5,   // pre_cur_diff
        200, // energy_th3 
        150, // energy_th2 
        100, // energy_th1 

        1,   // avgbv_erosion_en
        0,   // statis_freq_loop
        0,   // freq_debugmode
        0,   // freq_candidate_compare_random
        0,   // freq_candidate_compare_temporal
        0,   // freq_candidate_compare_spatial
        0,   // freq_force_bv_replace_mode
        0,   // freq_force_bv_replace_en
        1,   // freq_avgbv_bv_diff_penalty_gain
        1,   // freq_avgbv_bv_diff_penalty_offset
        0,   // freq_avgbv_bv_diff_penalty_en
        0,   // freq_random_mv_replace
        0,   // freq_random_mask
        0,   // freq_candidate_temporal_dis
        0,   // freq_candidate_spatial_dis
        0,   // freq_obme_mode
        1,   // freq_obme_mode_sel
        0,   // freq_obme_en

        30,  // freq_avgbv_mv_cnt_th
        1,   // freq_avgbv_hist_close_th
        0,   // freq_avgbv_hist_close_en
    },
    /* LOW */
    {
        128, // maxfoot_minfoot_diff
        128, // maxpeak_minpeak_diff
        16,  // maxpeak_minfoot_diff
        0,   // me0_med3_en
        1,   // me1_med3_en
        1,   // max_min_en
        1,   // me0_post_lpf_en
        1,   // me1_post_lpf_en
        1,   // center_match_en
        1,   // me0_detect_en
        1,   // me1_detect_en

        6,   // me0_med3_th_v
        11,  // me0_med3_th_h
        6,   // me1_med3_th_v
        11,  // me1_med3_th_h
        8,   // me0_energy_gain_v
        5,   // me0_energy_gain_h
        13,  // me1_energy_gain_v
        5,   // me1_energy_gain_h

        5,   // pre_cur_diff
        200, // energy_th3 
        150, // energy_th2 
        100, // energy_th1 

        1,   // avgbv_erosion_en
        0,   // statis_freq_loop
        0,   // freq_debugmode
        0,   // freq_candidate_compare_random
        1,   // freq_candidate_compare_temporal
        1,   // freq_candidate_compare_spatial
        0,   // freq_force_bv_replace_mode
        0,   // freq_force_bv_replace_en
        1,   // freq_avgbv_bv_diff_penalty_gain
        1,   // freq_avgbv_bv_diff_penalty_offset
        1,   // freq_avgbv_bv_diff_penalty_en
        0,   // freq_random_mv_replace
        1,   // freq_random_mask
        0,   // freq_candidate_temporal_dis
        0,   // freq_candidate_spatial_dis
        0,   // freq_obme_mode
        1,   // freq_obme_mode_sel
        0,   // freq_obme_en

        30,  // freq_avgbv_mv_cnt_th
        1,   // freq_avgbv_hist_close_th
        0,   // freq_avgbv_hist_close_en
    },
    /* MID */
    {
        128, // maxfoot_minfoot_diff
        128, // maxpeak_minpeak_diff
        16,  // maxpeak_minfoot_diff
        0,   // me0_med3_en
        1,   // me1_med3_en
        1,   // max_min_en
        1,   // me0_post_lpf_en
        1,   // me1_post_lpf_en
        1,   // center_match_en
        1,   // me0_detect_en
        1,   // me1_detect_en

        6,   // me0_med3_th_v
        11,  // me0_med3_th_h
        6,   // me1_med3_th_v
        11,  // me1_med3_th_h
        8,   // me0_energy_gain_v
        5,   // me0_energy_gain_h
        13,  // me1_energy_gain_v
        5,   // me1_energy_gain_h

        5,   // pre_cur_diff
        200, // energy_th3 
        150, // energy_th2 
        100, // energy_th1 

        1,   // avgbv_erosion_en
        0,   // statis_freq_loop
        0,   // freq_debugmode
        0,   // freq_candidate_compare_random
        1,   // freq_candidate_compare_temporal
        1,   // freq_candidate_compare_spatial
        0,   // freq_force_bv_replace_mode
        0,   // freq_force_bv_replace_en
        1,   // freq_avgbv_bv_diff_penalty_gain
        1,   // freq_avgbv_bv_diff_penalty_offset
        1,   // freq_avgbv_bv_diff_penalty_en
        1,   // freq_random_mv_replace
        1,   // freq_random_mask
        1,   // freq_candidate_temporal_dis
        0,   // freq_candidate_spatial_dis
        0,   // freq_obme_mode
        1,   // freq_obme_mode_sel
        0,   // freq_obme_en

        30,  // freq_avgbv_mv_cnt_th
        1,   // freq_avgbv_hist_close_th
        0,   // freq_avgbv_hist_close_en
    },
    /* HIGH */
    {
        128, // maxfoot_minfoot_diff
        128, // maxpeak_minpeak_diff
        16,  // maxpeak_minfoot_diff
        0,   // me0_med3_en
        1,   // me1_med3_en
        1,   // max_min_en
        1,   // me0_post_lpf_en
        1,   // me1_post_lpf_en
        1,   // center_match_en
        1,   // me0_detect_en
        1,   // me1_detect_en

        6,   // me0_med3_th_v
        11,  // me0_med3_th_h
        6,   // me1_med3_th_v
        11,  // me1_med3_th_h
        8,   // me0_energy_gain_v
        5,   // me0_energy_gain_h
        13,  // me1_energy_gain_v
        5,   // me1_energy_gain_h

        5,   // pre_cur_diff
        200, // energy_th3 
        150, // energy_th2 
        100, // energy_th1 

        1,   // avgbv_erosion_en
        0,   // statis_freq_loop
        0,   // freq_debugmode
        0,   // freq_candidate_compare_random
        1,   // freq_candidate_compare_temporal
        1,   // freq_candidate_compare_spatial
        0,   // freq_force_bv_replace_mode
        0,   // freq_force_bv_replace_en
        1,   // freq_avgbv_bv_diff_penalty_gain
        1,   // freq_avgbv_bv_diff_penalty_offset
        1,   // freq_avgbv_bv_diff_penalty_en
        1,   // freq_random_mv_replace
        1,   // freq_random_mask
        1,   // freq_candidate_temporal_dis
        0,   // freq_candidate_spatial_dis
        0,   // freq_obme_mode
        1,   // freq_obme_mode_sel
        0,   // freq_obme_en

        30,  // freq_avgbv_mv_cnt_th
        1,   // freq_avgbv_hist_close_th
        0,   // freq_avgbv_hist_close_en
    },
};
		
static PrdParam gMEMC_PrdParamTable[4] =
{
    {0,1,1,0,0,128,0,0,0,16,0,16,3,1,2,200,10,1,0,0,0,6,0,0,6,4,0,3,4,500,0,1,9,32,128,216,32,16,4,0,247}, // disable all func
	{1,1,1,1,1,128,1,0,0,16,0,16,3,1,2,200,10,1,0,0,1,6,0,0,6,4,0,1,4,500,1,1,9,32,128,216,32,16,4,0,247},  //low
	{1,1,1,1,1,128,1,0,1,16,0,16,3,1,2,200,10,1,0,0,1,6,0,0,6,4,0,1,4,500,1,1,9,32,128,216,32,16,4,0,247},  //mid , normal
	{1,1,1,1,1,128,1,0,1,16,0,16,3,1,2,200,10,1,0,0,1,6,0,0,6,4,0,1,4,500,1,1,9,32,128,216,32,16,4,0,247}   //high
};
#endif

VOID Periodic_Init(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
#if CONFIG_REPEAT_NEW_ALOG
    int y_ofst = 0, x_ofst = 0;
    
    pOutput->u1_detect = MEMC_PERIODIC_LEVEL_DISABLE;
    for (y_ofst = 0; y_ofst <= 3; y_ofst += 1)
    {
        for (x_ofst = 0; x_ofst <= 7; x_ofst += 1)
        {
            pOutput->u10_freq_statistic_cnt1[y_ofst][x_ofst] = 0;
            pOutput->u10_freq_statistic_cnt2[y_ofst][x_ofst] = 0;
            pOutput->u10_freq_statistic_cnt3[y_ofst][x_ofst] = 0;
        }
    }

    Periodic_StatusInit(pOutput);
#endif
}

VOID Periodic_Proc(const _PARAM_PERIODIC_MANAGE *pParam, _OUTPUT_PERIODIC_MANAGE *pOutput)
{
#if CONFIG_REPEAT_NEW_ALOG
    /* update regr value */
    kme_algo_me2_ref_vbuf_RBUS kme_algo_me2_ref_vbuf_reg;
    kme_algo_me2_ref_vbuf_reg.regValue = rtd_inl(KME_ALGO_me2_ref_vbuf_reg);
   
    Periodic_UpdateInfo(pOutput);
    #if 1
    Periodic_StatusUpdate(pParam, pOutput);

    if ((kme_algo_me2_ref_vbuf_reg.dummy1809ed1c_27_13 & 0x1) == 0) // use 13 bits to control firmware auto, 1=bypass
        Periodic_Apply(pOutput);
		Periodic_Apply_Global_AvgBv(pOutput);
		Periodic_Apply_Region_AvgBv(pOutput);
    #endif
#endif
}

VOID Periodic_Detect(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
#if CONFIG_REPEAT_NEW_ALOG
    // check mv stablilty

    // check rp count
    unsigned int log_en = 0;
	static unsigned char u1_pre_detect = 0;
	kme_me1_top11_me1_bv_stats_rg_idx_0_1_2_cnt_RBUS kme_me1_top11_me1_bv_stats_rg_idx_0_1_2_cnt_reg;
	kme_me1_top11_me1_bv_stats_rg_idx_6_7_8_cnt_RBUS kme_me1_top11_me1_bv_stats_rg_idx_6_7_8_cnt_reg;
    const _PQLCONTEXT *s_pContext = GetPQLContext();
    unsigned int lt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[0] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[1];
    unsigned int rt_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[6] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[7];
	unsigned int logo_all = 0;
	// unsigned int lb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[24];
    // unsigned int rb_logo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[31];
    unsigned int lt_zmv_cnt = 0, rt_zmv_cnt = 0;
    int cnt3_count = 0;
    int y_ofst = 0, x_ofst = 0;
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 2, 2, &log_en);

    kme_me1_top11_me1_bv_stats_rg_idx_0_1_2_cnt_reg.regValue = rtd_inl(KME_ME1_TOP11_me1_bv_stats_rg_idx_0_1_2_cnt_reg);
    kme_me1_top11_me1_bv_stats_rg_idx_6_7_8_cnt_reg.regValue = rtd_inl(KME_ME1_TOP11_me1_bv_stats_rg_idx_6_7_8_cnt_reg);

	lt_zmv_cnt = kme_me1_top11_me1_bv_stats_rg_idx_0_1_2_cnt_reg.me1_bv_stats_rg_idx0_cnt+kme_me1_top11_me1_bv_stats_rg_idx_0_1_2_cnt_reg.me1_bv_stats_rg_idx1_cnt;
	rt_zmv_cnt = kme_me1_top11_me1_bv_stats_rg_idx_6_7_8_cnt_reg.me1_bv_stats_rg_idx6_cnt+kme_me1_top11_me1_bv_stats_rg_idx_6_7_8_cnt_reg.me1_bv_stats_rg_idx7_cnt;
    
    for (y_ofst = 0; y_ofst <= 3; y_ofst += 1)
    {
        for (x_ofst = 0; x_ofst <= 7; x_ofst += 1)
        {
            if (pOutput->u10_freq_statistic_cnt3_IIR[y_ofst][x_ofst] > 210)
                cnt3_count++;
			logo_all += s_pContext->_output_read_comreg.u10_blklogo_rgcnt[y_ofst * 8 + x_ofst];
        }
    }
#if 1
    if (logo_all < 3000 && ((lt_logo_cnt > 150 && lt_zmv_cnt> 200) || (rt_logo_cnt > 150 && rt_zmv_cnt > 200)))
    {
        Periodic_ChangeLevel(pOutput, MEMC_PERIODIC_LEVEL_DISABLE);
    }
#if 0
	else if (cnt3_count == 0)//beauty and the beast
	{
        Periodic_ChangeLevel(pOutput, MEMC_PERIODIC_LEVEL_LOW);
	}
#endif
    else
    {
        Periodic_ChangeLevel(pOutput, MEMC_PERIODIC_LEVEL_MID);
    }
#else
	Periodic_ChangeLevel(pOutput, MEMC_PERIODIC_LEVEL_MID);
#endif
	if(pOutput->u1_detect != u1_pre_detect && log_en)
	{	
		rtd_pr_memc_emerg("[%s][%d] u1_detect=%d, %d, %d, %d, %d, %d, %d\n",__func__,__LINE__, pOutput->u1_detect, logo_all, lt_logo_cnt,lt_zmv_cnt,rt_logo_cnt,rt_zmv_cnt,cnt3_count); 
	}
	u1_pre_detect = pOutput->u1_detect;
#endif
}

VOID Periodic_Detect_SC(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u1_algo_SC_en, u8_sc_hold_cnt_max, log_en;
	//modify use calibrated data
	unsigned int u31_sad = s_pContext->_output_read_comreg.u30_me_aSAD_rb;
	unsigned int u31_sc_th = s_pContext->_output_frc_sceneAnalysis.u32_sad_sc_th_without_rp_gain;
	static unsigned char sc_hold_cnt = 0;

	ReadRegister(SOFTWARE3_SOFTWARE3_51_reg, 30, 30, &u1_algo_SC_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_51_reg, 22, 22, &log_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_51_reg,  8, 15, &u8_sc_hold_cnt_max);

	if(!u1_algo_SC_en){
		sc_hold_cnt = 0;
		pOutput->u1_SC_disable_algo = false;
		pOutput->u1_SC_cnt = sc_hold_cnt;
		return;
	}

	if(u31_sad > u31_sc_th){
		pOutput->u1_SC_disable_algo = true;
		sc_hold_cnt = u8_sc_hold_cnt_max;
	}else if(sc_hold_cnt>0){
		pOutput->u1_SC_disable_algo = true;
		sc_hold_cnt--;
	}else{
		pOutput->u1_SC_disable_algo = false;
	}
	pOutput->u1_SC_cnt = sc_hold_cnt;

	if(log_en){//d9cc[22]
		rtd_pr_memc_emerg("[%s]  detect_sc,%d,  sc_hold_cnt,%d,\n", __FUNCTION__, pOutput->u1_SC_disable_algo, sc_hold_cnt);
	}
}

VOID Periodic_Detect_Cnt3(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
	unsigned char u8_y = 0, u8_x = 0;
	unsigned int u1_algo_cnt_en, u8_cnt_th, log_en;
	bool u1_AvgBv_en = false;

	ReadRegister(SOFTWARE3_SOFTWARE3_51_reg, 31, 31, &u1_algo_cnt_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_51_reg, 23, 23, &log_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_51_reg,  0,  7, &u8_cnt_th);
	
	for(u8_y=0; u8_y<4 && u1_AvgBv_en==false; u8_y++){
		for(u8_x=0; u8_x<7 && u1_AvgBv_en==false; u8_x++){
			if(pOutput->u10_freq_statistic_cnt3_IIR[u8_y][u8_x]>u8_cnt_th &&
				pOutput->u10_freq_statistic_cnt3_IIR[u8_y][u8_x+1]>u8_cnt_th){
				u1_AvgBv_en = true;
			}
		}
	}

	for(u8_y=0; u8_y<3 && u1_AvgBv_en==false; u8_y++){
		for(u8_x=0; u8_x<8 && u1_AvgBv_en==false; u8_x++){
			if(pOutput->u10_freq_statistic_cnt3_IIR[u8_y][u8_x]>u8_cnt_th &&
				pOutput->u10_freq_statistic_cnt3_IIR[u8_y+1][u8_x]>u8_cnt_th){
				u1_AvgBv_en = true;
			}
		}
	}
	pOutput->u1_detec_repeat_cnt = u1_AvgBv_en;

	if(!u1_algo_cnt_en){
		pOutput->u1_detec_repeat_cnt = true;
	}

	if(log_en){//d9cc[23]
		rtd_pr_memc_emerg("[%s]  detect_cnt,%d,  u1_AvgBv_en,%d,  u8_cnt_th,%d,\n", __FUNCTION__, pOutput->u1_detec_repeat_cnt, u1_AvgBv_en, u8_cnt_th);
	}
}

VOID Periodic_Detect_Unstable_Rmv(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
	unsigned int u8_mv_th, regOffset, u1_algo_en, u8_unstable_cnt, u1_force_en, u1_force_dis, log_en, u4_compare_times;
	unsigned char u8_i = 0, u8_y = 0, u8_x = 0;
	int u8_log_id = 0;

	ReadRegister(SOFTWARE3_SOFTWARE3_52_reg,  0,  7, &u8_mv_th);
	ReadRegister(SOFTWARE3_SOFTWARE3_52_reg,  8, 15, &u8_unstable_cnt);
	ReadRegister(SOFTWARE3_SOFTWARE3_52_reg, 16, 19, &u4_compare_times);
	ReadRegister(SOFTWARE3_SOFTWARE3_52_reg, 29, 29, &u1_force_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_52_reg, 30, 30, &u1_force_dis);
	ReadRegister(SOFTWARE3_SOFTWARE3_52_reg, 31, 31, &u1_algo_en);

	u4_compare_times = _CLIP_(u4_compare_times, 1, 32);

	if(!u1_algo_en){//d9d0[31]
		return;
	}
	pOutput->u1_rgn_stable = true;

	if(u1_force_en || u1_force_dis){
		for(u8_y=0; u8_y<4; u8_y++){
			for(u8_x=0; u8_x<8; u8_x++){
				regOffset = 4 * (u8_y * 8 + u8_x);
				if(u1_force_en){
					pOutput->u8_rmv_unstable_cnt[u8_y][u8_x] = 0;
					pOutput->u1_rgn_stable = true;
				}else{
					pOutput->u8_rmv_unstable_cnt[u8_y][u8_x] = 1;
					pOutput->u1_rgn_stable = false;
				}
			}
		}
		return;
	}
	
	for(u8_y=0; u8_y<4; u8_y++){
		for(u8_x=0; u8_x<8; u8_x++){
			regOffset = 4 * (u8_y * 8 + u8_x);
			pOutput->u8_rmv_unstable_cnt[u8_y][u8_x] = (pOutput->u8_rmv_unstable_cnt[u8_y][u8_x] > 0) ? pOutput->u8_rmv_unstable_cnt[u8_y][u8_x]-1 : 0;
			for(u8_i=0; u8_i<u4_compare_times-1; u8_i++){
				if(_ABS_DIFF_(pOutput->s11_freg_statistic_avgbv_mvx[u8_i][u8_y][u8_x], pOutput->s11_freg_statistic_avgbv_mvx[u8_i+1][u8_y][u8_x]) >= u8_mv_th ||
				   _ABS_DIFF_(pOutput->s10_freg_statistic_avgbv_mvy[u8_i][u8_y][u8_x], pOutput->s10_freg_statistic_avgbv_mvy[u8_i+1][u8_y][u8_x]) >= u8_mv_th ){
					pOutput->u8_rmv_unstable_cnt[u8_y][u8_x] = u8_unstable_cnt;
					pOutput->u1_rgn_stable = false;
					break;
				}
			}
			
			u8_log_id = u8_x + u8_y * 8;
			ReadRegister(SOFTWARE3_SOFTWARE3_53_reg, u8_log_id, u8_log_id, &log_en);
			if(log_en){
				rtd_pr_memc_emerg("[%s]	rgn,%d, mv(0),%d,%d,	mv(1),%d,%d,	diff,%d,%d, result,%d,%d\n", __FUNCTION__, u8_log_id,
				pOutput->s11_freg_statistic_avgbv_mvx[0][u8_y][u8_x], pOutput->s11_freg_statistic_avgbv_mvx[1][u8_y][u8_x],
				pOutput->s10_freg_statistic_avgbv_mvy[0][u8_y][u8_x], pOutput->s10_freg_statistic_avgbv_mvy[1][u8_y][u8_x],
				_ABS_DIFF_(pOutput->s11_freg_statistic_avgbv_mvx[0][u8_y][u8_x], pOutput->s11_freg_statistic_avgbv_mvx[1][u8_y][u8_x]),
				_ABS_DIFF_(pOutput->s10_freg_statistic_avgbv_mvy[0][u8_y][u8_x], pOutput->s10_freg_statistic_avgbv_mvy[1][u8_y][u8_x]),
				_ABS_DIFF_(pOutput->s11_freg_statistic_avgbv_mvx[0][u8_y][u8_x], pOutput->s11_freg_statistic_avgbv_mvx[1][u8_y][u8_x]) >= u8_mv_th ? 1 : 0,
				_ABS_DIFF_(pOutput->s10_freg_statistic_avgbv_mvy[0][u8_y][u8_x], pOutput->s10_freg_statistic_avgbv_mvy[1][u8_y][u8_x]) >= u8_mv_th ? 1 : 0);
			}
		}
	}
}

VOID Periodic_StatusInit(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
#if CONFIG_REPEAT_NEW_ALOG
	kme_me1_top11_me1_bv_stats_mv_center_threshold_RBUS kme_me1_top11_me1_bv_stats_mv_center_threshold_reg;
	
    kme_me1_top11_me1_bv_stats_mv_center_threshold_reg.regValue = rtd_inl(KME_ME1_TOP11_me1_bv_stats_mv_center_threshold_reg);
	kme_me1_top11_me1_bv_stats_mv_center_threshold_reg.me1_bv_stats_mv_threshold = 2;
    rtd_outl(KME_ME1_TOP11_me1_bv_stats_mv_center_threshold_reg, kme_me1_top11_me1_bv_stats_mv_center_threshold_reg.regValue);
	
    pOutput->PERIODIC_STATUS.Frame_Couter = 0;

    Periodic_ChangeLevel(pOutput, MEMC_PERIODIC_LEVEL_MID);
#endif
}

VOID Periodic_StatusUpdate(const _PARAM_PERIODIC_MANAGE *pParam, _OUTPUT_PERIODIC_MANAGE *pOutput)
{
#if CONFIG_REPEAT_NEW_ALOG
    if (pOutput->PERIODIC_STATUS.Frame_Couter > 65535)
        pOutput->PERIODIC_STATUS.Frame_Couter = 0;
    else
        pOutput->PERIODIC_STATUS.Frame_Couter++;

    // function here
    Periodic_Detect(pOutput);
	Periodic_Detect_SC(pOutput);
	Periodic_Detect_Cnt3(pOutput);
	Periodic_Detect_Unstable_Rmv(pOutput);
#endif
}

VOID Periodic_Apply(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
#if CONFIG_REPEAT_NEW_ALOG
    kme_ipme2_kme_ipme2_00_RBUS kme_ipme2_kme_ipme2_00_reg;
    kme_ipme2_kme_ipme2_04_RBUS kme_ipme2_kme_ipme2_04_reg;
    kme_ipme2_kme_ipme2_08_RBUS kme_ipme2_kme_ipme2_08_reg;

    kme_algo_me1_freq_ctrl_RBUS kme_algo_me1_freq_ctrl_reg;
    kme_algo_me1_freq_ctrl2_RBUS kme_algo_me1_freq_ctrl2_reg;
    
    kme_ipme2_kme_ipme2_00_reg.regValue = rtd_inl(KME_IPME2_KME_IPME2_00_reg);
    kme_ipme2_kme_ipme2_04_reg.regValue = rtd_inl(KME_IPME2_KME_IPME2_04_reg);
    kme_ipme2_kme_ipme2_08_reg.regValue = rtd_inl(KME_IPME2_KME_IPME2_08_reg);
    
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_maxfoot_minfoot_diff  = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_maxfoot_minfoot_diff;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_maxpeak_minpeak_diff  = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_maxpeak_minpeak_diff;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_maxpeak_minfoot_diff  = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_maxpeak_minfoot_diff;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_me0_med3_en           = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_med3_en         ;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_me1_med3_en           = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_med3_en         ;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_max_min_en            = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_max_min_en          ;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_me0_post_lpf_en       = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_post_lpf_en     ;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_me1_post_lpf_en       = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_post_lpf_en     ;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_center_match_en       = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_center_match_en     ;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_me0_detect_en         = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_detect_en       ;
    kme_ipme2_kme_ipme2_00_reg.ipme2_freq_me1_detect_en         = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_detect_en       ;

    kme_ipme2_kme_ipme2_04_reg.ipme2_freq_me0_med3_th_v         = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_med3_th_v       ;
    kme_ipme2_kme_ipme2_04_reg.ipme2_freq_me0_med3_th_h         = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_med3_th_h       ;
    kme_ipme2_kme_ipme2_04_reg.ipme2_freq_me1_med3_th_v         = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_med3_th_v       ;
    kme_ipme2_kme_ipme2_04_reg.ipme2_freq_me1_med3_th_h         = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_med3_th_h       ;
    kme_ipme2_kme_ipme2_04_reg.ipme2_freq_me0_energy_gain_v     = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_energy_gain_v   ;
    kme_ipme2_kme_ipme2_04_reg.ipme2_freq_me0_energy_gain_h     = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_energy_gain_h   ;
    kme_ipme2_kme_ipme2_04_reg.ipme2_freq_me1_energy_gain_v     = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_energy_gain_v   ;
    kme_ipme2_kme_ipme2_04_reg.ipme2_freq_me1_energy_gain_h     = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_energy_gain_h   ;

    kme_ipme2_kme_ipme2_08_reg.ipme2_freq_pre_cur_diff          = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_pre_cur_diff        ;
    kme_ipme2_kme_ipme2_08_reg.ipme2_freq_energy_th3            = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_energy_th3          ;
    kme_ipme2_kme_ipme2_08_reg.ipme2_freq_energy_th2            = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_energy_th2          ;
    kme_ipme2_kme_ipme2_08_reg.ipme2_freq_energy_th1            = pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_energy_th1          ;

    rtd_outl(KME_IPME2_KME_IPME2_00_reg, kme_ipme2_kme_ipme2_00_reg.regValue);
    rtd_outl(KME_IPME2_KME_IPME2_04_reg, kme_ipme2_kme_ipme2_04_reg.regValue);
    rtd_outl(KME_IPME2_KME_IPME2_08_reg, kme_ipme2_kme_ipme2_08_reg.regValue);

    kme_algo_me1_freq_ctrl_reg.regValue = rtd_inl(KME_ALGO_me1_freq_ctrl_reg);
    kme_algo_me1_freq_ctrl2_reg.regValue = rtd_inl(KME_ALGO_me1_freq_ctrl2_reg);

    kme_algo_me1_freq_ctrl_reg.me1_avgbv_erosion_en                     = pOutput->PERIODIC_STATUS.ORIGIN.me1_avgbv_erosion_en                  ;
    kme_algo_me1_freq_ctrl_reg.me1_statis_freq_loop                     = pOutput->PERIODIC_STATUS.ORIGIN.me1_statis_freq_loop                  ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_debugmode                       = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_debugmode                    ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_candidate_compare_random        = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_compare_random     ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_candidate_compare_temporal      = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_compare_temporal   ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_candidate_compare_spatial       = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_compare_spatial    ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_force_bv_replace_mode           = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_force_bv_replace_mode        ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_force_bv_replace_en             = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_force_bv_replace_en          ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_avgbv_bv_diff_penalty_gain      = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_bv_diff_penalty_gain   ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_avgbv_bv_diff_penalty_offset    = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_bv_diff_penalty_offset ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_avgbv_bv_diff_penalty_en        = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_bv_diff_penalty_en     ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_random_mv_replace               = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_random_mv_replace            ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_random_mask                     = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_random_mask                  ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_candidate_temporal_dis          = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_temporal_dis       ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_candidate_spatial_dis           = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_spatial_dis        ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_obme_mode                       = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_obme_mode                    ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_obme_mode_sel                   = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_obme_mode_sel                ;
    kme_algo_me1_freq_ctrl_reg.me1_freq_obme_en                         = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_obme_en                      ;

    kme_algo_me1_freq_ctrl2_reg.me1_freq_avgbv_mv_cnt_th                = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_mv_cnt_th              ;
#if NEW_REPEAT_NEW_ALGO_REMOVE_M8
    kme_algo_me1_freq_ctrl2_reg.me1_freq_avgbv_hist_close_th            = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_hist_close_th          ;
    kme_algo_me1_freq_ctrl2_reg.me1_freq_avgbv_hist_close_en            = pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_hist_close_en          ;
#endif    
    rtd_outl(KME_ALGO_me1_freq_ctrl_reg, kme_algo_me1_freq_ctrl_reg.regValue);
    rtd_outl(KME_ALGO_me1_freq_ctrl2_reg, kme_algo_me1_freq_ctrl2_reg.regValue);
#endif
}

VOID Periodic_Apply_Global_AvgBv(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
	kme_algo_me1_freq_ctrl_RBUS kme_algo_me1_freq_ctrl_reg;
	unsigned int u1_algo_en, log_en;

	ReadRegister(SOFTWARE3_SOFTWARE3_49_reg,  0, 0, &u1_algo_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_49_reg, 16,16, &log_en);

	if(!u1_algo_en){
		return;
	}

	kme_algo_me1_freq_ctrl_reg.regValue = rtd_inl(KME_ALGO_me1_freq_ctrl_reg);
	kme_algo_me1_freq_ctrl_reg.me1_freq_avgbv_bv_diff_penalty_en = (pOutput->u1_detec_repeat_cnt==true) ? true : false;
	rtd_outl(KME_ALGO_me1_freq_ctrl_reg, kme_algo_me1_freq_ctrl_reg.regValue);

	if(log_en){//d9c4[16]
		rtd_pr_memc_emerg("[%s]  detect_cnt,%d, detect_sc,%d, set(ED20[11]),%d\n", __FUNCTION__, 
			pOutput->u1_detec_repeat_cnt, pOutput->u1_SC_disable_algo, kme_algo_me1_freq_ctrl_reg.me1_freq_avgbv_bv_diff_penalty_en);
	}
}

VOID Periodic_Apply_Region_AvgBv(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
	unsigned int u1_algo_en, regOffset;
	unsigned char u8_y = 0, u8_x = 0;

	ReadRegister(SOFTWARE3_SOFTWARE3_49_reg, 1, 1, &u1_algo_en);

	if(!u1_algo_en){
		return;
	}
	
	for(u8_y=0; u8_y<4; u8_y++){
		for(u8_x=0; u8_x<8; u8_x++){
			regOffset = 4 * (u8_y * 8 + u8_x);
			// if rmv is stable, turn on the avg BV
			if(pOutput->u8_rmv_unstable_cnt[u8_y][u8_x]==0 && pOutput->u1_SC_disable_algo==false){
				WriteRegister(KME_ALGO2_me1_freq_avgbv_00_reg + regOffset, 21, 21, 1);
			}else{
				WriteRegister(KME_ALGO2_me1_freq_avgbv_00_reg + regOffset, 21, 21, 0);
			}
		}
	}
}

VOID Periodic_UpdateInfo(_OUTPUT_PERIODIC_MANAGE *pOutput)
{
#if CONFIG_REPEAT_NEW_ALOG
    int y_ofst = 0, x_ofst = 0;
	unsigned char u8_i = 0;
    unsigned int u32_freq_statistic_value = 0, log_en = 0;
	static unsigned char u8_cnt = 0;
	ReadRegister(SOFTWARE3_SOFTWARE3_54_reg, 29, 29, &log_en);

	pOutput->u15_max_freq_statistic_cnt1 = 0;
	pOutput->u15_max_freq_statistic_cnt2 = 0;
	pOutput->u15_max_freq_statistic_cnt3 = 0;

	pOutput->u15_sum_freq_statistic_cnt1 = 0;
	pOutput->u15_sum_freq_statistic_cnt2 = 0;
	pOutput->u15_sum_freq_statistic_cnt3 = 0;

    for (y_ofst = 0; y_ofst <= 3; y_ofst += 1)
    {
        for (x_ofst = 0; x_ofst <= 7; x_ofst += 1)
        {
            int regOffset = 4 * (y_ofst * 8 + x_ofst);
            ReadRegister(KME_ALGO_me1_freq_statistic_00_reg + regOffset, 0, 29, &u32_freq_statistic_value);
            pOutput->u10_freq_statistic_cnt1[y_ofst][x_ofst] = (u32_freq_statistic_value >> 0) & 0x3FF;
            pOutput->u10_freq_statistic_cnt2[y_ofst][x_ofst] = (u32_freq_statistic_value >> 10) & 0x3FF;
            pOutput->u10_freq_statistic_cnt3[y_ofst][x_ofst] = (u32_freq_statistic_value >> 20) & 0x3FF;

			pOutput->u15_max_freq_statistic_cnt1 = _MAX_(pOutput->u15_max_freq_statistic_cnt1, pOutput->u10_freq_statistic_cnt1[y_ofst][x_ofst]);
			pOutput->u15_max_freq_statistic_cnt2 = _MAX_(pOutput->u15_max_freq_statistic_cnt2, pOutput->u10_freq_statistic_cnt2[y_ofst][x_ofst]);
			pOutput->u15_max_freq_statistic_cnt3 = _MAX_(pOutput->u15_max_freq_statistic_cnt3, pOutput->u10_freq_statistic_cnt3[y_ofst][x_ofst]);

			pOutput->u15_sum_freq_statistic_cnt1 += pOutput->u10_freq_statistic_cnt1[y_ofst][x_ofst];
			pOutput->u15_sum_freq_statistic_cnt2 += pOutput->u10_freq_statistic_cnt2[y_ofst][x_ofst];
			pOutput->u15_sum_freq_statistic_cnt3 += pOutput->u10_freq_statistic_cnt3[y_ofst][x_ofst];

            // IIR
            pOutput->u10_freq_statistic_cnt1_IIR[y_ofst][x_ofst] = (3 * pOutput->u10_freq_statistic_cnt1_IIR[y_ofst][x_ofst] + pOutput->u10_freq_statistic_cnt1[y_ofst][x_ofst] + 2)>>2;
            pOutput->u10_freq_statistic_cnt2_IIR[y_ofst][x_ofst] = (3 * pOutput->u10_freq_statistic_cnt2_IIR[y_ofst][x_ofst] + pOutput->u10_freq_statistic_cnt2[y_ofst][x_ofst] + 2)>>2;
            pOutput->u10_freq_statistic_cnt3_IIR[y_ofst][x_ofst] = (3 * pOutput->u10_freq_statistic_cnt3_IIR[y_ofst][x_ofst] + pOutput->u10_freq_statistic_cnt3[y_ofst][x_ofst] + 2)>>2;

			for(u8_i=1; u8_i<PRD_STATIC_RMV_NUM; u8_i++){
				pOutput->s11_freg_statistic_avgbv_mvx[PRD_STATIC_RMV_NUM-u8_i][y_ofst][x_ofst] = pOutput->s11_freg_statistic_avgbv_mvx[PRD_STATIC_RMV_NUM-u8_i-1][y_ofst][x_ofst];
				pOutput->s10_freg_statistic_avgbv_mvy[PRD_STATIC_RMV_NUM-u8_i][y_ofst][x_ofst] = pOutput->s10_freg_statistic_avgbv_mvy[PRD_STATIC_RMV_NUM-u8_i-1][y_ofst][x_ofst];
			}
			ReadRegister(KME_ALGO3_me1_freq_statistic_avgbv_00_reg + regOffset, 0, 20, &u32_freq_statistic_value);			
			pOutput->s11_freg_statistic_avgbv_mvx[0][y_ofst][x_ofst] = (((u32_freq_statistic_value>>10) & 1)==0) ? (u32_freq_statistic_value & 0x7ff) : ((u32_freq_statistic_value & 0x7ff)	- (1<<11));
			pOutput->s10_freg_statistic_avgbv_mvy[0][y_ofst][x_ofst] = (((u32_freq_statistic_value>>20) & 1)==0)? ((u32_freq_statistic_value>>11) & 0x3ff) : (((u32_freq_statistic_value>>11) & 0x3ff)	- (1<<10));
			pOutput->s11_freg_statistic_avgbv_mvx[0][y_ofst][x_ofst] = Calib_MV(pOutput->s11_freg_statistic_avgbv_mvx[0][y_ofst][x_ofst]);
			pOutput->s10_freg_statistic_avgbv_mvy[0][y_ofst][x_ofst] = Calib_MV(pOutput->s10_freg_statistic_avgbv_mvy[0][y_ofst][x_ofst]);
		}
    }

	u8_cnt++;
	if(log_en && u8_cnt>10){ //d9d8[29]
		u8_cnt = 0;
		rtd_pr_memc_emerg("[%d] ,%d,%d,%d,%d,%d,%d,%d,%d	,%d,%d,%d,%d,%d,%d,%d,%d	,%d,%d,%d,%d,%d,%d,%d,%d	,%d,%d,%d,%d,%d,%d,%d,%d\n",
			pOutput->u15_max_freq_statistic_cnt3,
			pOutput->u10_freq_statistic_cnt3[0][0],pOutput->u10_freq_statistic_cnt3[0][1],
			pOutput->u10_freq_statistic_cnt3[0][2],pOutput->u10_freq_statistic_cnt3[0][3],
			pOutput->u10_freq_statistic_cnt3[0][4],pOutput->u10_freq_statistic_cnt3[0][5],
			pOutput->u10_freq_statistic_cnt3[0][6],pOutput->u10_freq_statistic_cnt3[0][7],
			pOutput->u10_freq_statistic_cnt3[1][0],pOutput->u10_freq_statistic_cnt3[1][1],
			pOutput->u10_freq_statistic_cnt3[1][2],pOutput->u10_freq_statistic_cnt3[1][3],
			pOutput->u10_freq_statistic_cnt3[1][4],pOutput->u10_freq_statistic_cnt3[1][5],
			pOutput->u10_freq_statistic_cnt3[1][6],pOutput->u10_freq_statistic_cnt3[1][7],
			pOutput->u10_freq_statistic_cnt3[2][0],pOutput->u10_freq_statistic_cnt3[2][1],
			pOutput->u10_freq_statistic_cnt3[2][2],pOutput->u10_freq_statistic_cnt3[2][3],
			pOutput->u10_freq_statistic_cnt3[2][4],pOutput->u10_freq_statistic_cnt3[2][5],
			pOutput->u10_freq_statistic_cnt3[2][6],pOutput->u10_freq_statistic_cnt3[2][7],
			pOutput->u10_freq_statistic_cnt3[3][0],pOutput->u10_freq_statistic_cnt3[3][1],
			pOutput->u10_freq_statistic_cnt3[3][2],pOutput->u10_freq_statistic_cnt3[3][3],
			pOutput->u10_freq_statistic_cnt3[3][4],pOutput->u10_freq_statistic_cnt3[3][5],
			pOutput->u10_freq_statistic_cnt3[3][6],pOutput->u10_freq_statistic_cnt3[3][7]);
		rtd_pr_memc_emerg("[mvx] ,%d,%d,%d,%d,%d,%d,%d,%d	,%d,%d,%d,%d,%d,%d,%d,%d	,%d,%d,%d,%d,%d,%d,%d,%d	,%d,%d,%d,%d,%d,%d,%d,%d\n",
			pOutput->s11_freg_statistic_avgbv_mvx[0][0][0],pOutput->s11_freg_statistic_avgbv_mvx[0][0][1],
			pOutput->s11_freg_statistic_avgbv_mvx[0][0][2],pOutput->s11_freg_statistic_avgbv_mvx[0][0][3],
			pOutput->s11_freg_statistic_avgbv_mvx[0][0][4],pOutput->s11_freg_statistic_avgbv_mvx[0][0][5],
			pOutput->s11_freg_statistic_avgbv_mvx[0][0][6],pOutput->s11_freg_statistic_avgbv_mvx[0][0][7],
			pOutput->s11_freg_statistic_avgbv_mvx[0][1][0],pOutput->s11_freg_statistic_avgbv_mvx[0][1][1],
			pOutput->s11_freg_statistic_avgbv_mvx[0][1][2],pOutput->s11_freg_statistic_avgbv_mvx[0][1][3],
			pOutput->s11_freg_statistic_avgbv_mvx[0][1][4],pOutput->s11_freg_statistic_avgbv_mvx[0][1][5],
			pOutput->s11_freg_statistic_avgbv_mvx[0][1][6],pOutput->s11_freg_statistic_avgbv_mvx[0][1][7],
			pOutput->s11_freg_statistic_avgbv_mvx[0][2][0],pOutput->s11_freg_statistic_avgbv_mvx[0][2][1],
			pOutput->s11_freg_statistic_avgbv_mvx[0][2][2],pOutput->s11_freg_statistic_avgbv_mvx[0][2][3],
			pOutput->s11_freg_statistic_avgbv_mvx[0][2][4],pOutput->s11_freg_statistic_avgbv_mvx[0][2][5],
			pOutput->s11_freg_statistic_avgbv_mvx[0][2][6],pOutput->s11_freg_statistic_avgbv_mvx[0][2][7],
			pOutput->s11_freg_statistic_avgbv_mvx[0][3][0],pOutput->s11_freg_statistic_avgbv_mvx[0][3][1],
			pOutput->s11_freg_statistic_avgbv_mvx[0][3][2],pOutput->s11_freg_statistic_avgbv_mvx[0][3][3],
			pOutput->s11_freg_statistic_avgbv_mvx[0][3][4],pOutput->s11_freg_statistic_avgbv_mvx[0][3][5],
			pOutput->s11_freg_statistic_avgbv_mvx[0][3][6],pOutput->s11_freg_statistic_avgbv_mvx[0][3][7]);	
	
	}
#endif
}

/*
	MEMC_PERIODIC_LEVEL_LOW
	MEMC_PERIODIC_LEVEL_MID
	MEMC_PERIODIC_LEVEL_HIGH
*/
VOID Periodic_ChangeLevel(_OUTPUT_PERIODIC_MANAGE *pOutput, int level)
{
#if CONFIG_REPEAT_NEW_ALOG
    pOutput->u1_detect = level;
    // detect initial value
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_maxfoot_minfoot_diff         = gMEMC_PeriodicParamTable[level].ipme2_freq_maxfoot_minfoot_diff;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_maxpeak_minpeak_diff         = gMEMC_PeriodicParamTable[level].ipme2_freq_maxpeak_minpeak_diff;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_maxpeak_minfoot_diff         = gMEMC_PeriodicParamTable[level].ipme2_freq_maxpeak_minfoot_diff;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_med3_en                  = gMEMC_PeriodicParamTable[level].ipme2_freq_me0_med3_en;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_med3_en                  = gMEMC_PeriodicParamTable[level].ipme2_freq_me1_med3_en;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_max_min_en                   = gMEMC_PeriodicParamTable[level].ipme2_freq_max_min_en;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_post_lpf_en              = gMEMC_PeriodicParamTable[level].ipme2_freq_me0_post_lpf_en;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_post_lpf_en              = gMEMC_PeriodicParamTable[level].ipme2_freq_me1_post_lpf_en;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_center_match_en              = gMEMC_PeriodicParamTable[level].ipme2_freq_center_match_en;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_detect_en                = gMEMC_PeriodicParamTable[level].ipme2_freq_me0_detect_en;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_detect_en                = gMEMC_PeriodicParamTable[level].ipme2_freq_me1_detect_en;

    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_med3_th_v                = gMEMC_PeriodicParamTable[level].ipme2_freq_me0_med3_th_v;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_med3_th_h                = gMEMC_PeriodicParamTable[level].ipme2_freq_me0_med3_th_h;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_med3_th_v                = gMEMC_PeriodicParamTable[level].ipme2_freq_me1_med3_th_v;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_med3_th_h                = gMEMC_PeriodicParamTable[level].ipme2_freq_me1_med3_th_h;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_energy_gain_v            = gMEMC_PeriodicParamTable[level].ipme2_freq_me0_energy_gain_v;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me0_energy_gain_h            = gMEMC_PeriodicParamTable[level].ipme2_freq_me0_energy_gain_h;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_energy_gain_v            = gMEMC_PeriodicParamTable[level].ipme2_freq_me1_energy_gain_v;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_me1_energy_gain_h            = gMEMC_PeriodicParamTable[level].ipme2_freq_me1_energy_gain_h;

    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_pre_cur_diff                 = gMEMC_PeriodicParamTable[level].ipme2_freq_pre_cur_diff;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_energy_th3                   = gMEMC_PeriodicParamTable[level].ipme2_freq_energy_th3;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_energy_th2                   = gMEMC_PeriodicParamTable[level].ipme2_freq_energy_th2;
    pOutput->PERIODIC_STATUS.ORIGIN.ipme2_freq_energy_th1                   = gMEMC_PeriodicParamTable[level].ipme2_freq_energy_th1;

    // apply initial value
    pOutput->PERIODIC_STATUS.ORIGIN.me1_avgbv_erosion_en                    = gMEMC_PeriodicParamTable[level].me1_avgbv_erosion_en;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_statis_freq_loop                    = gMEMC_PeriodicParamTable[level].me1_statis_freq_loop;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_debugmode                      = gMEMC_PeriodicParamTable[level].me1_freq_debugmode;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_compare_random       = gMEMC_PeriodicParamTable[level].me1_freq_candidate_compare_random;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_compare_temporal     = gMEMC_PeriodicParamTable[level].me1_freq_candidate_compare_temporal;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_compare_spatial      = gMEMC_PeriodicParamTable[level].me1_freq_candidate_compare_spatial;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_force_bv_replace_mode          = gMEMC_PeriodicParamTable[level].me1_freq_force_bv_replace_mode;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_force_bv_replace_en            = gMEMC_PeriodicParamTable[level].me1_freq_force_bv_replace_en;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_bv_diff_penalty_gain     = gMEMC_PeriodicParamTable[level].me1_freq_avgbv_bv_diff_penalty_gain;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_bv_diff_penalty_offset   = gMEMC_PeriodicParamTable[level].me1_freq_avgbv_bv_diff_penalty_offset;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_bv_diff_penalty_en       = gMEMC_PeriodicParamTable[level].me1_freq_avgbv_bv_diff_penalty_en;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_random_mv_replace              = gMEMC_PeriodicParamTable[level].me1_freq_random_mv_replace;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_random_mask                    = gMEMC_PeriodicParamTable[level].me1_freq_random_mask;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_temporal_dis         = gMEMC_PeriodicParamTable[level].me1_freq_candidate_temporal_dis;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_candidate_spatial_dis          = gMEMC_PeriodicParamTable[level].me1_freq_candidate_spatial_dis;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_obme_mode                      = gMEMC_PeriodicParamTable[level].me1_freq_obme_mode;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_obme_mode_sel                  = gMEMC_PeriodicParamTable[level].me1_freq_obme_mode_sel;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_obme_en                        = gMEMC_PeriodicParamTable[level].me1_freq_obme_en;

    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_mv_cnt_th                = gMEMC_PeriodicParamTable[level].me1_freq_avgbv_mv_cnt_th;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_hist_close_th            = gMEMC_PeriodicParamTable[level].me1_freq_avgbv_hist_close_th;
    pOutput->PERIODIC_STATUS.ORIGIN.me1_freq_avgbv_hist_close_en            = gMEMC_PeriodicParamTable[level].me1_freq_avgbv_hist_close_en;
#endif
}




//o--------------------------------o
//     PRD improve procedure
//o--------------------------------o



VOID PRD_Init(_OUTPUT_PRD_MANAGE *pOutput)
{
#if CONFIG_REPEAT_NEW_ALOG
	pOutput->prd_level = 0;
	PRD_Apply(pOutput);
#endif
}

VOID PRD_Proc(const _PARAM_PRD_MANAGE *pParam, _OUTPUT_PRD_MANAGE *pOutput, _OUTPUT_PERIODIC_MANAGE *pOutput2)
{
#if CONFIG_REPEAT_NEW_ALOG
    software3_software3_00_RBUS software3_software3_00_reg;
    software3_software3_00_reg.regValue = rtd_inl(SOFTWARE3_SOFTWARE3_00_reg);
    
    if (pParam->b_prd_auto_en) // 1:firmware en //d900[2]
    {
    	PRD_Detect(pParam, pOutput, pOutput2);
		PRD_Apply(pOutput);		
    }
#endif
}

VOID PRD_Detect(const _PARAM_PRD_MANAGE *pParam, _OUTPUT_PRD_MANAGE *pOutput, _OUTPUT_PERIODIC_MANAGE *pOutput2)
{
#if CONFIG_REPEAT_NEW_ALOG
	const _PQLCONTEXT *s_pContext = GetPQLContext();

    if(pOutput2->u1_SC_disable_algo){
		pOutput->prd_level = 0;//0ff
	}
	else if(pParam->u8_prd_manual_mode == 0) //d900[3:5]
    {
	    // check mv stablilty
    	// check rp count
    	// pOutput.prd_level = 2;

        kme_algo_me2_ref_vbuf_RBUS kme_algo_me2_ref_vbuf_reg;
        int cnt1_count = 0, cnt3_count = 0;
        int cnt3_th_a = 0, cnt3_th_b = 0, cnt1_th = 0;
        int y_ofst = 0, x_ofst = 0;
		int r32_index = 0;
		bool rgn_cond = 0;
		bool en0,en1,en2,en3,en4,en5,en6,en7;
		bool c0,c1,c2,c3,c4;
		int rgnDtl  = 0;
		int rmv_x   = 0;
		int rmv_y	= 0;
		int rmv_cnt = 0;
		int rmv_unconf  = 0;
		//int rmv_2nd_cnt = 0;
		//int rmv_2nd_unconf = 0;  
		int max_mvdiff = 0;
		int prd_valid_cnt=0;
		int freq_det_cnt = 0;

		int gmv_x = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
		int gmv_y = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
		//int gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
		//int gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;	
		unsigned char max_mvdiff_th = pParam->u8_max_mvdiff_th;	
		
        kme_algo_me2_ref_vbuf_reg.regValue = rtd_inl(KME_ALGO_me2_ref_vbuf_reg);
        for (y_ofst = 0; y_ofst <= 3; y_ofst += 1)
        {
            for (x_ofst = 0; x_ofst <= 7; x_ofst += 1)
            {
                if (pOutput2->u10_freq_statistic_cnt1_IIR[y_ofst][x_ofst] > 100)
                    cnt1_count++;
                    
                if (pOutput2->u10_freq_statistic_cnt3_IIR[y_ofst][x_ofst] > 100)
                    cnt3_count++;

				r32_index = y_ofst*8 + x_ofst;
				
				//o------ check RMV & GMV ------o
				rgnDtl = s_pContext->_output_read_comreg.u20_me_rDTL_rb[r32_index];
				rmv_x  = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[r32_index];
				rmv_y  = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[r32_index];	
				
				rmv_cnt = s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[r32_index];
				rmv_unconf =s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[r32_index] ;
				//rmv_2nd_cnt = s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[r32_index];	 
				//rmv_2nd_unconf = s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[r32_index];  
				max_mvdiff = _MAX_(_ABS_(gmv_x - rmv_x), _ABS_(gmv_y - rmv_y)); 	

				en0 = pParam->u8_criteria_en & 0x1;
				en1 = (pParam->u8_criteria_en >> 1) & 0x1;
				en2 = (pParam->u8_criteria_en >> 2) & 0x1;
				en3 = (pParam->u8_criteria_en >> 3) & 0x1;
				en4 = (pParam->u8_criteria_en >> 4) & 0x1;
				en5 = (pParam->u8_criteria_en >> 5) & 0x1;
				en6 = (pParam->u8_criteria_en >> 6) & 0x1;
				en7 = (pParam->u8_criteria_en >> 7) & 0x1;
		
				c0 = (((y_ofst+x_ofst) & 0x1) == 0);
				c1 = en1 && (pOutput2->u10_freq_statistic_cnt1_IIR[y_ofst][x_ofst] >= pParam->u8_test1_th);
				c2 = en2 && (pOutput2->u10_freq_statistic_cnt2_IIR[y_ofst][x_ofst] >= pParam->u8_test2_th);
				c3 = en3 && (pOutput2->u10_freq_statistic_cnt3_IIR[y_ofst][x_ofst] >= pParam->u8_test3_th);	
				c4 = !en4 || (max_mvdiff <= max_mvdiff_th); 

				rgn_cond = (c1 || c2 || c3);
				
				if(rgn_cond)
					freq_det_cnt++;				

				rgn_cond = rgn_cond && c4;

				if(rgn_cond)
					prd_valid_cnt++;
				
				//o------ for debug show -------o 
				//pOutput->r32_level[r32_index] = rgn_cond ? 255 : 0;//original from h5c2
				pOutput->r32_level[r32_index] = rgn_cond ? 1 : 0;
				
			}
        }

        cnt3_th_a = (kme_algo_me2_ref_vbuf_reg.dummy1809ed1c_27_13>>1) & 0x7;
        cnt3_th_b = (kme_algo_me2_ref_vbuf_reg.dummy1809ed1c_27_13>>4) & 0x7;
        cnt1_th   = (kme_algo_me2_ref_vbuf_reg.dummy1809ed1c_27_13>>7) & 0xff;

		//o------ decide the output level ------o
        if (cnt3_count < cnt3_th_a || (cnt3_count >= cnt3_th_b && cnt1_count > cnt1_th))
            pOutput->prd_level = 0;
		else if(en7 && (freq_det_cnt - prd_valid_cnt) >= pParam->u5_prd_valid_cnt_th)
			pOutput->prd_level = 1;
        else
            pOutput->prd_level = 2;
	
    }
	else
	{
    	pOutput->prd_level = pParam->u8_prd_manual_mode - 1;
	}
#endif
}


VOID PRD_Apply(_OUTPUT_PRD_MANAGE *pOutput)
{
#if CONFIG_REPEAT_NEW_ALOG
	kme_algo_me1_cand_dediff_00_RBUS kme_algo_me1_cand_dediff_00_reg;
	kme_algo_me1_cand_dediff_01_RBUS kme_algo_me1_cand_dediff_01_reg;
	kme_algo_me1_cand_dediff_02_RBUS kme_algo_me1_cand_dediff_02_reg;
#if NEW_REPEAT_NEW_ALGO_REMOVE_M8
	kme_algo_me1_cand_pnt_00_RBUS kme_algo_me1_cand_pnt_00_reg;
#endif
	kme_algo_me1_cand_pnt_01_RBUS kme_algo_me1_cand_pnt_01_reg;
	kme_algo_me1_mvd_pnt_00_RBUS kme_algo_me1_mvd_pnt_00_reg;
	kme_algo_me1_mvd_pnt_01_RBUS kme_algo_me1_mvd_pnt_01_reg;
	kme_algo_me1_mvd_pnt_02_RBUS kme_algo_me1_mvd_pnt_02_reg;
	
	int level = pOutput->prd_level;

    kme_algo_me1_cand_dediff_00_reg.regValue = rtd_inl(KME_ALGO_me1_cand_dediff_00_reg);
    kme_algo_me1_cand_dediff_01_reg.regValue = rtd_inl(KME_ALGO_me1_cand_dediff_01_reg);
	kme_algo_me1_cand_dediff_02_reg.regValue = rtd_inl(KME_ALGO_me1_cand_dediff_02_reg);
#if NEW_REPEAT_NEW_ALGO_REMOVE_M8
	kme_algo_me1_cand_pnt_00_reg.regValue    = rtd_inl(KME_ALGO_me1_cand_pnt_00_reg);   
#endif
	kme_algo_me1_cand_pnt_01_reg.regValue    = rtd_inl(KME_ALGO_me1_cand_pnt_01_reg);    
	kme_algo_me1_mvd_pnt_00_reg.regValue     = rtd_inl(KME_ALGO_me1_mvd_pnt_00_reg);     
	kme_algo_me1_mvd_pnt_01_reg.regValue     = rtd_inl(KME_ALGO_me1_mvd_pnt_01_reg);     
	kme_algo_me1_mvd_pnt_02_reg.regValue     = rtd_inl(KME_ALGO_me1_mvd_pnt_02_reg);

    kme_algo_me1_cand_dediff_00_reg.me1_cand_shrink_mask_en       = gMEMC_PrdParamTable[level].me1_cand_shrink_mask_en;      
	kme_algo_me1_cand_dediff_00_reg.me1_cand_shrink_mask_x        = gMEMC_PrdParamTable[level].me1_cand_shrink_mask_x;      
	kme_algo_me1_cand_dediff_00_reg.me1_cand_shrink_mask_y        = gMEMC_PrdParamTable[level].me1_cand_shrink_mask_y;       
	kme_algo_me1_cand_dediff_00_reg.me1_rejudge_occl_en           = gMEMC_PrdParamTable[level].me1_rejudge_occl_en;          
	kme_algo_me1_cand_dediff_00_reg.me1_rejudge_neargmv_en        = gMEMC_PrdParamTable[level].me1_rejudge_nearGMV_en;       
	kme_algo_me1_cand_dediff_00_reg.me1_rejudge_neargmv_th        = gMEMC_PrdParamTable[level].me1_rejudge_nearGMV_th;       
	kme_algo_me1_cand_dediff_00_reg.me1_cand_chk_redd_en          = gMEMC_PrdParamTable[level].me1_cand_chk_redd_en;         
	kme_algo_me1_cand_dediff_00_reg.me1_cand_redd_invalid         = gMEMC_PrdParamTable[level].me1_cand_redd_invalid;        
	kme_algo_me1_cand_dediff_00_reg.me1_cand_chk_invalid_en       = gMEMC_PrdParamTable[level].me1_cand_chk_invalid_en;      
	kme_algo_me1_cand_dediff_00_reg.me1_cand_avgbv_diff0_th       = gMEMC_PrdParamTable[level].me1_cand_avgbv_diff0_th;      
	kme_algo_me1_cand_dediff_00_reg.me1_cand_dediff_rp_pnt_rnd_en = gMEMC_PrdParamTable[level].me1_cand_dediff_rp_pnt_rnd_en;
	// 0xb802edb4
	kme_algo_me1_cand_dediff_01_reg.me1_cand_avgbv_diff_th        = gMEMC_PrdParamTable[level].me1_cand_avgbv_diff_th;
	kme_algo_me1_cand_dediff_01_reg.me1_cand_dediff_bmv_sel       = gMEMC_PrdParamTable[level].me1_cand_dediff_bMV_sel;      
	kme_algo_me1_cand_dediff_01_reg.me1_cand_dediff_sim_step      = gMEMC_PrdParamTable[level].me1_cand_dediff_sim_step;     
	kme_algo_me1_cand_dediff_01_reg.me1_cand_dediff_sim_mvd_gain  = gMEMC_PrdParamTable[level].me1_cand_dediff_sim_mvd_gain; 
	kme_algo_me1_cand_dediff_01_reg.me1_cand_dediff_sim_th        = gMEMC_PrdParamTable[level].me1_cand_dediff_sim_th;       
	kme_algo_me1_cand_dediff_01_reg.me1_cand_dediff_avgbv_diff_th = gMEMC_PrdParamTable[level].me1_cand_dediff_avgbv_diff_th;
	// 0xb802edb8                                                 
	kme_algo_me1_cand_dediff_02_reg.me1_cand_dediff_lmv_step      = gMEMC_PrdParamTable[level].me1_cand_dediff_lmv_step;
	kme_algo_me1_cand_dediff_02_reg.me1_cand_dediff_bmv_x         = gMEMC_PrdParamTable[level].me1_cand_dediff_bMV_x;   
	kme_algo_me1_cand_dediff_02_reg.me1_cand_dediff_bmv_y         = gMEMC_PrdParamTable[level].me1_cand_dediff_bMV_y;   
	// 0xb802edbc  
#if NEW_REPEAT_NEW_ALGO_REMOVE_M8
	kme_algo_me1_cand_pnt_00_reg.me1_dam_pnt_en                   = gMEMC_PrdParamTable[level].me1_dam_pnt_en;        
	kme_algo_me1_cand_pnt_00_reg.me1_st_apld_pnt_coef             = gMEMC_PrdParamTable[level].me1_st_apld_pnt_coef;  
	kme_algo_me1_cand_pnt_00_reg.me1_st_dtld_pnt_coef             = gMEMC_PrdParamTable[level].me1_st_dtld_pnt_coef;  
	kme_algo_me1_cand_pnt_00_reg.me1_rand_apld_pnt_coef           = gMEMC_PrdParamTable[level].me1_rand_apld_pnt_coef;
	kme_algo_me1_cand_pnt_00_reg.me1_rand_dtld_pnt_coef           = gMEMC_PrdParamTable[level].me1_rand_dtld_pnt_coef;
	kme_algo_me1_cand_pnt_00_reg.me1_dam_pnt_shft                 = gMEMC_PrdParamTable[level].me1_dam_pnt_shft;   
#endif
	// 0xb802edc0  
#if NEW_REPEAT_NEW_ALGO_REMOVE_M8
	kme_algo_me1_cand_pnt_01_reg.me1_freq_pnt_en                  = gMEMC_PrdParamTable[level].me1_freq_pnt_en;   
#endif
	kme_algo_me1_cand_pnt_01_reg.me1_cand_freq_det_th             = gMEMC_PrdParamTable[level].me1_cand_freq_det_th;   
#if NEW_REPEAT_NEW_ALGO_REMOVE_M8
	kme_algo_me1_cand_pnt_01_reg.me1_freq_pnt_gain                = gMEMC_PrdParamTable[level].me1_freq_pnt_gain;      
#endif
	kme_algo_me1_cand_pnt_01_reg.me1_cand_dediff_bmv_pnt          = gMEMC_PrdParamTable[level].me1_cand_dediff_bMV_pnt;
	// 0xb802edc4                                                 
	kme_algo_me1_mvd_pnt_00_reg.me1_new_mvd_cost_en               = gMEMC_PrdParamTable[level].me1_new_mvd_cost_en;  
	kme_algo_me1_mvd_pnt_00_reg.me1_apl_var_step                  = gMEMC_PrdParamTable[level].me1_apl_var_step;     
	kme_algo_me1_mvd_pnt_00_reg.me1_new_mvd_cost_shft             = gMEMC_PrdParamTable[level].me1_new_mvd_cost_shft;
	// 0xb802edc8                                                 
	kme_algo_me1_mvd_pnt_01_reg.me1_mvd_again_x1                  = gMEMC_PrdParamTable[level].me1_mvd_again_x1; 
	kme_algo_me1_mvd_pnt_01_reg.me1_mvd_again_x2                  = gMEMC_PrdParamTable[level].me1_mvd_again_x2; 
	kme_algo_me1_mvd_pnt_01_reg.me1_mvd_again_x3                  = gMEMC_PrdParamTable[level].me1_mvd_again_x3; 
	kme_algo_me1_mvd_pnt_01_reg.me1_mvd_again_y1                  = gMEMC_PrdParamTable[level].me1_mvd_again_y1; 
	// 0xb802edcc
	kme_algo_me1_mvd_pnt_02_reg.me1_mvd_again_y2                  = gMEMC_PrdParamTable[level].me1_mvd_again_y2;    
	kme_algo_me1_mvd_pnt_02_reg.me1_mvd_again_y3                  = gMEMC_PrdParamTable[level].me1_mvd_again_y3;    
	kme_algo_me1_mvd_pnt_02_reg.me1_mvd_again_slope1              = gMEMC_PrdParamTable[level].me1_mvd_again_slope1;
	kme_algo_me1_mvd_pnt_02_reg.me1_mvd_again_slope2              = gMEMC_PrdParamTable[level].me1_mvd_again_slope2;
    
    rtd_outl(KME_ALGO_me1_cand_dediff_00_reg, kme_algo_me1_cand_dediff_00_reg.regValue);
    rtd_outl(KME_ALGO_me1_cand_dediff_01_reg, kme_algo_me1_cand_dediff_01_reg.regValue);
	rtd_outl(KME_ALGO_me1_cand_dediff_02_reg, kme_algo_me1_cand_dediff_02_reg.regValue);
#if NEW_REPEAT_NEW_ALGO_REMOVE_M8
	rtd_outl(KME_ALGO_me1_cand_pnt_00_reg, kme_algo_me1_cand_pnt_00_reg.regValue);
#endif
	rtd_outl(KME_ALGO_me1_cand_pnt_01_reg, kme_algo_me1_cand_pnt_01_reg.regValue);
	rtd_outl(KME_ALGO_me1_mvd_pnt_00_reg, kme_algo_me1_mvd_pnt_00_reg.regValue);
	rtd_outl(KME_ALGO_me1_mvd_pnt_01_reg, kme_algo_me1_mvd_pnt_01_reg.regValue);
	rtd_outl(KME_ALGO_me1_mvd_pnt_02_reg, kme_algo_me1_mvd_pnt_02_reg.regValue);
#endif	
}

VOID Region32_indicator (const _PARAM_PRD_MANAGE *pParam, _OUTPUT_PRD_MANAGE *pOutput, _OUTPUT_PERIODIC_MANAGE *pOutput2)
{
#if CONFIG_REPEAT_NEW_ALOG
	//const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i=0, j=0;
	unsigned int u32_WR_val, u8_val;

	if(pParam->u8_r32_indicator_en==0)//d900[6]
		return;

	//WriteRegister(KMC_BI_bi_top_01_reg, 2, 3, 0x2); //bi_fb_mode

	for(i=0; i<8; i++)
	{
		rtd_outl(KMC_BI_bi_gama_00_reg + (i<<2), 0xffffffff);	//bi_fb_gamma
	
		u32_WR_val = 0;
		for(j=3; j>=0; j--){
			u8_val = (pOutput2->u8_rmv_unstable_cnt[j][i] == 0) ? 0 : 0xff; 
			u32_WR_val = (u32_WR_val << 8) | u8_val;
			//u32_WR_val = (u32_WR_val << 8) | pOutput->r32_level[i*4+j];
		}

		rtd_outl(KMC_BI_bi_fb_00_reg + (i<<2), u32_WR_val);    //bi_fb_level
	
	}
#endif
}

