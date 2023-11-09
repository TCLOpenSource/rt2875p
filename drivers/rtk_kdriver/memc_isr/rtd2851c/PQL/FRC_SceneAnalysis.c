#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/FRC_SceneAnalysis.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_isr/PQL/Read_ComReg.h"
#include "memc_reg_def.h"
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

VOID FRC_SceneAnalysis_Init(_OUTPUT_FRC_SceneAnalysis *pOutput)
{
	//////////////////////////////////////////////////////////////////////////
	//Fade In/Out
	//////////////////////////////////////////////////////////////////////////
	pOutput->u4_fadeInOut_conf				= 0;
	pOutput->u4_fadeInOut_conf_iir          = 0;
	pOutput->u4_fadeInOut_cnt_alpha			= 0;
	pOutput->u4_fadeInOut_bv_sad_alpha		= 0;
	pOutput->u4_fadeInOut_acdc_sad_alpha	= 0;
	pOutput->u4_fadeInOut_apl_diff_alpha    = 0;
	pOutput->u8_ME1_searchRange_x           = 0;
	pOutput->u8_ME1_searchRange_y           = 0;
	pOutput->u2_searchRange_x_status        = 0;
	pOutput->u2_searchRange_y_status        = 0;
	pOutput->u1_fadeInOut_dh_close          = 0;
	pOutput->u4_fadeInOut_logo_conf_iir	= 0;
	pOutput->u1_fadeInOut_logo_clr		= 0;
	//pOutput->u1_fadeInOut_acdc_bld          = 0;
	//pOutput->u1_fadeInOut_acdc_bld_pre      = 0;
	pOutput->u5_dc_ac_bld_alpha             = 0;
	pOutput->u5_acdc_bld_holdtime           = 0;
	pOutput->u8_zoom_force_fb_cnt		= 0;
	pOutput->u1_still_frame	        	= 0;

	//
	pOutput->u8_HDR_PicChg_prt_apply = 0;
	pOutput->u16_OSDBrightnessVal = 0;
	pOutput->u16_OSDContrastVal = 0;
	pOutput->u8_OSDBrightnessBigChg_prt_apply = 0;
	pOutput->u8_OSDContrastBigChg_prt_apply = 0;
	pOutput->u8_VOSeek_prt_apply = 0;

	pOutput->u8_dynME_ac_pix_sad_limt_act   = 0;
	pOutput->u4_acdc_bld_psad_shift_act     = 0;

	pOutput->u32_glb_sad_pre                = 0;
	pOutput->u32_glb_sad_cur                = 0;

	pOutput->u26_Apli[0]                    = 0;
	pOutput->u26_Apli[1]                    = 0;
	pOutput->u26_Apli[2]                    = 0;
	pOutput->u26_Apli[3]                    = 0;
	pOutput->u26_Apli[4]                    = 0;
	pOutput->u32_frm_cnt                    = 0;
	pOutput->u1_Apli_same                   = 0;
	pOutput->u8_is_insidePan                = 0;
	pOutput->u8_is_insidePanX               = 0;

	pOutput->u32_insidePan_avgAbsMVX        = 0;
	pOutput->u32_insidePan_avgAbsMVY        = 0;
	pOutput->s2m_12_cnt_iir                 = 0;

	pOutput->u1_fastMotion_det = 0;

    pOutput->u8_dh_divide_hold_cnt = 0;   
    pOutput->u8_dh_virtical_hold_cnt = 0;
    pOutput->u8_dh_large_mv_th_y2_hold_cnt = 0;
    pOutput->u8_dh_small_mv_th_y0_hold_cnt = 0;
	pOutput->u8_dh_slow_motion_hold_cnt = 0;
    pOutput->u8_dh_black_protect_off_hold_cnt = 0;
    pOutput->u8_dh_white_protect_on_hold_cnt = 0;
    pOutput->u8_dh_correct_cut_th_hold_cnt = 0;
    pOutput->u8_dh_aplfix_en_hold_cnt = 0;
    pOutput->u8_dh_pred_en0_hold_cnt = 0;
    pOutput->u8_dh_extend_more_hold_cnt = 0;
    pOutput->u8_dh_noslow_easy_extend_hold_cnt = 0;
	pOutput->u8_dh_predflag_dis_hold_cnt = 0;
    pOutput->u8_dh_meader_fix_hold_cnt = 0;
    pOutput->u8_dh_divide_in_cnt = 0;   
    pOutput->u8_dh_virtical_in_cnt = 0;
    pOutput->u8_dh_large_mv_th_y2_in_cnt = 0;
    pOutput->u8_dh_small_mv_th_y0_in_cnt = 0;
	pOutput->u8_dh_slow_motion_in_cnt = 0;
    pOutput->u8_dh_black_protect_off_in_cnt = 0;
    pOutput->u8_dh_white_protect_on_in_cnt = 0;
    pOutput->u8_dh_correct_cut_th_in_cnt = 0;
    pOutput->u8_dh_aplfix_en_in_cnt = 0;
    pOutput->u8_dh_pred_en0_in_cnt = 0;
    pOutput->u8_dh_extend_more_in_cnt = 0;
    pOutput->u8_dh_noslow_easy_extend_in_cnt = 0;
	pOutput->u8_dh_predflag_dis_in_cnt = 0;
    pOutput->u8_dh_meader_fix_in_cnt = 0;
}

#if 0
int MVDiff_Calc(int mvx1, int mvy1, int mvx2, int mvy2)
{
	int mvxd = _ABS_DIFF_(mvx1 , mvx2) + _ABS_DIFF_(mvy1 , mvy2);
	return mvxd;
}


//
VOID FRC_Scene2Motion_Hist(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i,j;
	unsigned char good_cnt		 = 0;
	unsigned char mv_type_cnt	 = 0;
	int mvx;
	int mvy;

	const int valid_num    = 6;
	const int valid_num1   = 950;
	const int bin_x_length = 32;
	const int bin_y_length = 16;

	unsigned char  mv_cnt[32*16];
	int max_mv_x = bin_x_length/2 * pParam->s2m_diffmv_thr;
	int max_mv_y = bin_y_length/2 * pParam->s2m_diffmv_thr;
	int mvx_bin,mvy_bin;
	int first_idx = 0, find_first = 0;

	unsigned char max_cnt     = 0;
	unsigned char sec_cnt     = 0;
	unsigned char max_cnt_i = 0,max_cnt_j = 0;
	unsigned char sec_cnt_i = 0,sec_cnt_j = 0;

	for (i=0;i<bin_x_length*bin_y_length;i++)
	{
		mv_cnt[i] = 0;
	}

	for (i = 0; i< 32; i++)
	{
		if((_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]) >= pParam->s2m_goodmv_thr ||
			 _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]) >= pParam->s2m_goodmv_thr) &&
			(s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > valid_num) &&
			(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] < pParam->s2m_goodmv_unconf)&&
			((s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i]> valid_num &&
			  s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[i] < pParam->s2m_goodmv_unconf) ||
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i]<= valid_num))
		{
			good_cnt++;
		}

		find_first = 0;
		if (s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > valid_num &&
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] < pParam->s2m_goodmv_unconf)
		{
			mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
			mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];
			mvx = _CLIP_(mvx, -max_mv_x, max_mv_x-1) + pParam->s2m_diffmv_thr/2;
			mvy = _CLIP_(mvy, -max_mv_y, max_mv_y-1) + pParam->s2m_diffmv_thr/2;
			mvx_bin = _CLIP_((mvx + max_mv_x)/pParam->s2m_diffmv_thr,0,bin_x_length-1);
			mvy_bin = _CLIP_((mvy + max_mv_y)/pParam->s2m_diffmv_thr,0,bin_y_length-1);

			if (mv_cnt[mvy_bin*bin_x_length + mvx_bin] == 0)
			{
				mv_type_cnt = mv_type_cnt + 1;
			}

			mv_cnt[mvy_bin*bin_x_length + mvx_bin] = mv_cnt[mvy_bin*bin_x_length + mvx_bin] + 1;
			find_first = 1;
			first_idx  = mvy_bin*bin_x_length + mvx_bin;
		}

		if (s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i] > valid_num &&
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[i] < pParam->s2m_goodmv_unconf)
		{
			mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
			mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];
			mvx = _CLIP_(mvx, -max_mv_x, max_mv_x-1) + pParam->s2m_diffmv_thr/2;
			mvy = _CLIP_(mvy, -max_mv_y, max_mv_y-1) + pParam->s2m_diffmv_thr/2;
			mvx_bin = _CLIP_((mvx + max_mv_x)/pParam->s2m_diffmv_thr,0,bin_x_length-1);
			mvy_bin = _CLIP_((mvy + max_mv_y)/pParam->s2m_diffmv_thr,0,bin_y_length-1);
			if (mv_cnt[mvy_bin*bin_x_length + mvx_bin] == 0)
			{
				mv_type_cnt = mv_type_cnt + 1;
			}
			mv_cnt[mvy_bin*bin_x_length + mvx_bin] = mv_cnt[mvy_bin*bin_x_length + mvx_bin] + 1;
		}
		else if(s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i] <= valid_num &&
			    s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > valid_num1 &&
			    find_first == 1)
		{
			mv_cnt[first_idx] = mv_cnt[first_idx] + 1;
		}
	}

	for (j=0; j<bin_y_length; j++)
	{
		for (i=0; i<bin_x_length; i++)
		{
			if (mv_cnt[j*bin_x_length + i] > max_cnt)
			{
				max_cnt   = mv_cnt[j*bin_x_length + i];
				max_cnt_i = i;
				max_cnt_j = j;
			}
		}
	}

	for (j=0; j<bin_y_length; j++)
	{
		for (i=0; i<bin_x_length; i++)
		{
			if (mv_cnt[j*bin_x_length + i] > max_cnt && (i!=max_cnt_i && j!=max_cnt_j))
			{
				sec_cnt   = mv_cnt[j*bin_x_length + i];
				sec_cnt_i = i;
				sec_cnt_j = j;
			}
		}
	}

	pOutput->s2m_1st_cnt    = max_cnt;
	pOutput->s2m_1st_mvx    = (max_cnt_i * pParam->s2m_diffmv_thr) - pParam->s2m_diffmv_thr/2 - max_mv_x;
	pOutput->s2m_1st_mvy    = (max_cnt_j * pParam->s2m_diffmv_thr) - pParam->s2m_diffmv_thr/2 - max_mv_y;
	pOutput->s2m_2nd_cnt    = sec_cnt;
	pOutput->s2m_2nd_mvx    = (sec_cnt_i * pParam->s2m_diffmv_thr) - pParam->s2m_diffmv_thr/2 - max_mv_x;
	pOutput->s2m_2nd_mvy    = (sec_cnt_j * pParam->s2m_diffmv_thr) - pParam->s2m_diffmv_thr/2 - max_mv_y;
	pOutput->s2m_12_cnt_iir = _2Val_RndBlending_(pOutput->s2m_12_cnt_iir,(max_cnt + sec_cnt),pParam->s2m_cnt_iir_alpha,4);
	pOutput->u8_goodGmv_cnt = good_cnt;
	pOutput->u8_diffGmv_cnt = mv_type_cnt;
}



VOID FRC_Scene2Motion(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i,j;
	unsigned char good_cnt		 = 0;
	unsigned char mv_type_cnt	 = 0;
	int mvdiff		 = 0;
	int min_classdif	 = 1000000;
	int min_class_idx = 0;

	int find_class  = 0;
	int mvx[65];
	int mvy[65];
	unsigned char mv_cnt[65];

	unsigned char max_cnt     = 0;
	unsigned char sec_cnt     = 0;
	unsigned char max_cnt_idx = 0;
	unsigned char sec_cnt_idx = 0;

	const int valid_num   = 6;

	for (i=0;i<65;i++)
	{
		mv_cnt[i] = 0;
	}

	for (i = 0; i< 32; i++)
	{
		if((_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]) >= pParam->s2m_goodmv_thr || _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]) >= pParam->s2m_goodmv_thr)
			&& s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > valid_num && s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] < pParam->s2m_goodmv_unconf
			&& ((s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i]> valid_num && s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[i] < pParam->s2m_goodmv_unconf) ||
				s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i]<= valid_num))

		{
			good_cnt++;
		}

		if (i == 0)
		{
			if (s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > valid_num)
			{
				mvx[mv_type_cnt] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
				mvy[mv_type_cnt] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];
				mv_cnt[mv_type_cnt]++;
				mv_type_cnt++;
			}

			if (mv_type_cnt > 0)
			{
				mvdiff = MVDiff_Calc(mvx[0],mvy[0],s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[i],s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[i]);
				if (s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i] > valid_num && mvdiff > pParam->s2m_diffmv_thr)
				{
					mvx[mv_type_cnt] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[i];
					mvy[mv_type_cnt] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[i];
					mv_cnt[mv_type_cnt]++;
					mv_type_cnt++;
				}
				else if (s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i] > valid_num)
				{
					mvx[0] = (mvx[0] + s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[i])/2 ;
					mvy[0] = (mvy[0] + s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[i])/2;
					mv_cnt[0]++;
				}
			}
			else if (s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i] > valid_num)
			{
				mvx[mv_type_cnt] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[i];
				mvy[mv_type_cnt] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[i];
				mv_cnt[mv_type_cnt]++;
				mv_type_cnt++;
			}
		}

		if (i > 0)
		{
			mv_type_cnt = _CLIP_UBOUND_(mv_type_cnt, 64);
			if (s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > valid_num)
			{
				find_class    = 0;
				min_classdif  = 1000000000;
				min_class_idx = 0;
				for (j=0; j< mv_type_cnt; j++)
				{
					mvdiff = MVDiff_Calc(mvx[j],mvy[j],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]);
					if (mvdiff <= pParam->s2m_diffmv_thr)
					{
						find_class    = 1;
						if (mvdiff < min_classdif)
						{
							min_classdif  = mvdiff;
							min_class_idx = j;
						}
					}
				}

				if (find_class == 1)
				{
					mvx[min_class_idx]    = (mvx[min_class_idx] + s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i])/2;
					mvy[min_class_idx]    = (mvy[min_class_idx] + s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i])/2;
					mv_cnt[min_class_idx]++;
				}
				else
				{
					mvx[mv_type_cnt] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
					mvy[mv_type_cnt] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];
					mv_cnt[mv_type_cnt] = 1;
					mv_type_cnt++;
				}
			}

			mv_type_cnt = _CLIP_(mv_type_cnt,0, 64);
			if (s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i] > valid_num)
			{
				find_class    = 0;
				min_classdif  = 1000000000;
				min_class_idx = 0;
				for (j=0; j< mv_type_cnt; j++)
				{
					mvdiff = MVDiff_Calc(mvx[j],mvy[j],s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[i],s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[i]);
					if (mvdiff <= pParam->s2m_diffmv_thr)
					{
						find_class    = 1;
						if (mvdiff < min_classdif)
						{
							min_classdif  = mvdiff;
							min_class_idx = j;
						}
					}
				}

				if (find_class == 1)
				{
					mvx[min_class_idx]    = (mvx[min_class_idx] + s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[i])/2;
					mvy[min_class_idx]    = (mvy[min_class_idx] + s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[i])/2;
					mv_cnt[min_class_idx]++;
				}
				else
				{
					mvx[mv_type_cnt] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[i];
					mvy[mv_type_cnt] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[i];
					mv_cnt[mv_type_cnt] = 1;
					mv_type_cnt++;
				}
			}
		}
	}


	//find max
	mv_type_cnt = _CLIP_(mv_type_cnt,0, 64);
	for (i=0; i<mv_type_cnt; i++)
	{
		if (mv_cnt[i] > max_cnt)
		{
			max_cnt = mv_cnt[i];
			max_cnt_idx = i;
		}
	}

	for (i=0; i<mv_type_cnt; i++)
	{
		if (mv_cnt[i] > sec_cnt && i != max_cnt_idx)
		{
			sec_cnt = mv_cnt[i];
			sec_cnt_idx = i;
		}
	}


	pOutput->s2m_1st_cnt    = max_cnt;
	pOutput->s2m_1st_mvx    = mvx[max_cnt_idx];
	pOutput->s2m_1st_mvy    = mvy[max_cnt_idx];
	pOutput->s2m_2nd_cnt    = sec_cnt;
	pOutput->s2m_2nd_mvx    = mvx[sec_cnt_idx];
	pOutput->s2m_2nd_mvy    = mvy[sec_cnt_idx];
	pOutput->s2m_12_cnt_iir = _2Val_RndBlending_(pOutput->s2m_12_cnt_iir,(max_cnt + sec_cnt),pParam->s2m_cnt_iir_alpha,4);
	pOutput->u8_goodGmv_cnt = good_cnt;
	pOutput->u8_diffGmv_cnt = mv_type_cnt;
}
#endif
VOID FRC_FadeInOut_Detect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	if (pParam->u1_fadeInOut_en == 1)
	{
		unsigned int cnt_alpha_use, cnt_total_use, apl_diff, i;
		//unsigned char p_cnt = 0;
		//unsigned char n_cnt = 0;
		//condier whole block cnt is accurate.
		unsigned int u16_cnt_totalNum = (s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb);

		//scene change/menu is both small, cnt/sad-sum
		unsigned int acGreateDc_cnt  = s_pContext->_output_read_comreg.u20_me_ZMV_AcGreatDc_cnt_rb;
		unsigned int dcGreateAc_cnt  = s_pContext->_output_read_comreg.u20_me_ZMV_DcGreatAc_cnt_rb;

		//unsigned int	apl_diff       = u16_cnt_totalNum > 0 ? _ABS_(((int)s_pContext->_output_read_comreg.u26_me_aAPLi_rb - (int)s_pContext->_output_read_comreg.u26_me_aAPLp_rb))/u16_cnt_totalNum :
		//						 _ABS_(((int)s_pContext->_output_read_comreg.u26_me_aAPLi_rb - (int)s_pContext->_output_read_comreg.u26_me_aAPLp_rb));

		//n.5 prec. max=64x32=2048
		unsigned int acdc_cnt_ratio      = acGreateDc_cnt == 0 ? dcGreateAc_cnt*32 : (dcGreateAc_cnt * 32 / acGreateDc_cnt);
		unsigned int cnt_alpha_diff_prm  = pParam->u8_fdIO_acdc_cnt_alpha_hig - pParam->u8_fdIO_acdc_cnt_alpha_low;
		unsigned int acdc_cnt_slope      = cnt_alpha_diff_prm <= 0 ? 0 : 16 * 256 / cnt_alpha_diff_prm;


		unsigned int acdc_cnt_total      = acGreateDc_cnt + dcGreateAc_cnt;
		unsigned int cnt_thr_diff_prm    = pParam->u20_fdIO_acdc_cnt_hig - pParam->u20_fdIO_acdc_cnt_low;
		unsigned int acdc_thr_slope      = cnt_thr_diff_prm <= 0 ? 0 : 16 * 256 / cnt_thr_diff_prm;

		unsigned char  u3_apl_diff_cnt     = _CLIP_((5 - pParam->u3_fdIO_apl_diff_frm_cnt),0,4);

		unsigned char u4_fadeInOut_logo_conf = 0;
		//unsigned char rgn_cnt = 0;
		//unsigned int rapl_diff[32] = {0};
		//unsigned int rapl_diff_avg = 0;
		//unsigned int u16_cnt_rgnNum = 30*33;
		#if 0
		unsigned int ip31=0,ip30=0;
		#endif

		acdc_cnt_ratio = (acdc_cnt_ratio>>12);
		acdc_cnt_total = (acdc_cnt_total>>6);

		for(i=0; i<4; i++)
		{
			pOutput->u26_Apli[i] = pOutput->u26_Apli[i+1];
		}
		pOutput->u26_Apli[4] = (int)s_pContext->_output_read_comreg.u26_me_aAPLi_rb;

		pOutput->u1_Apli_same = (pOutput->u26_Apli[u3_apl_diff_cnt] == (int)s_pContext->_output_read_comreg.u26_me_aAPLi_rb)? 1:0;

		if(pOutput->u32_frm_cnt > 5)
		{
			apl_diff       = u16_cnt_totalNum > 0 ? _ABS_DIFF_(((int)pOutput->u26_Apli[u3_apl_diff_cnt]),  ((int)s_pContext->_output_read_comreg.u26_me_aAPLp_rb))/u16_cnt_totalNum :
							_ABS_DIFF_(((int)pOutput->u26_Apli[u3_apl_diff_cnt]) , ((int)s_pContext->_output_read_comreg.u26_me_aAPLp_rb));
		}
		else
		{
			apl_diff       = u16_cnt_totalNum > 0 ? _ABS_DIFF_(((int)s_pContext->_output_read_comreg.u26_me_aAPLi_rb) , ((int)s_pContext->_output_read_comreg.u26_me_aAPLp_rb))/u16_cnt_totalNum :
							_ABS_DIFF_(((int)s_pContext->_output_read_comreg.u26_me_aAPLi_rb) , ((int)s_pContext->_output_read_comreg.u26_me_aAPLp_rb));
		}

		//for(i=0; i<32; i++)
		//{
		//	rapl_diff[i] = _ABS_DIFF_(((int)s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i])- ((int)s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i]))/u16_cnt_rgnNum;
		//	rapl_diff_avg += rapl_diff[i];
		//}
		//rapl_diff_avg = (rapl_diff_avg>>5);

		//for(i=0; i<32; i++)
		//{
		//	if(rapl_diff_avg>3 && _ABS_DIFF_(((int)rapl_diff_avg)-((int)rapl_diff[i]))<3)
		//		rgn_cnt++;
		//}

		pOutput->u4_fadeInOut_cnt_alpha      = _IncreaseCurveMapping(acdc_cnt_ratio, pParam->u8_fdIO_acdc_cnt_alpha_low, 0, 16, acdc_cnt_slope, 8);
		pOutput->u4_fadeInOut_cnt_Thr_alpha  = _IncreaseCurveMapping(acdc_cnt_total, pParam->u20_fdIO_acdc_cnt_low, 0, 16, acdc_thr_slope, 8);
		pOutput->u4_fadeInOut_apl_diff_alpha = (apl_diff < pParam->u10_fdIO_apl_diff_thr_low) ? 0 : (apl_diff > pParam->u10_fdIO_apl_diff_thr_hig ? 16 : 8);

		cnt_alpha_use   = pParam->u1_fdIO_acdc_cnt_en   == 1  ? pOutput->u4_fadeInOut_cnt_alpha      : 16;
		cnt_total_use   = pParam->u1_fdIO_acdc_ratio_en == 1  ? pOutput->u4_fadeInOut_cnt_Thr_alpha  : 16;


		pOutput->u4_fadeInOut_conf = cnt_alpha_use * cnt_total_use / 16;
		pOutput->u4_fadeInOut_conf = (apl_diff < pParam->u10_fdIO_apl_diff_thr_low) ?  0 : pOutput->u4_fadeInOut_conf;
		pOutput->u4_fadeInOut_conf = (apl_diff > pParam->u10_fdIO_apl_diff_thr_hig) ? 16 : pOutput->u4_fadeInOut_conf;

		u4_fadeInOut_logo_conf =  cnt_total_use;
		u4_fadeInOut_logo_conf = (apl_diff < pParam->u10_fdIO_apl_diff_thr_low) ?  0 : u4_fadeInOut_logo_conf;
		u4_fadeInOut_logo_conf = (apl_diff > pParam->u10_fdIO_apl_diff_thr_hig) ? 16 : u4_fadeInOut_logo_conf;
		pOutput->u4_fadeInOut_logo_conf_iir = pParam->u1_fdIO_conf_iir_en == 0 ? u4_fadeInOut_logo_conf :
			(pOutput->u4_fadeInOut_logo_conf_iir * pParam->u4_fdIO_conf_iir_alpha + u4_fadeInOut_logo_conf * (16 - pParam->u4_fdIO_conf_iir_alpha) + (1<<3))/16;
		pOutput->u1_fadeInOut_logo_clr = (pOutput->u4_fadeInOut_logo_conf_iir >= (pParam->u5_fadeInOut_logo_close_thr)) ? 1 : 0;

		pOutput->u4_fadeInOut_conf_iir = pParam->u1_fdIO_conf_iir_en == 0 ? pOutput->u4_fadeInOut_conf :
			(pOutput->u4_fadeInOut_conf_iir * pParam->u4_fdIO_conf_iir_alpha + pOutput->u4_fadeInOut_conf * (16 - pParam->u4_fdIO_conf_iir_alpha) + (1<<3))/16;

		pOutput->u1_fadeInOut_dh_close = (pOutput->u4_fadeInOut_conf_iir >= pParam->u5_fadeInOut_dh_close_thr) ? 1 : 0;
		
		//rtd_pr_memc_emerg("[johning_dai]u4_fadeInOut_conf_iir:%d,u4_fadeInOut_conf:%d,u5_fadeInOut_dh_close_thr=%d\n",pOutput->u4_fadeInOut_conf_iir,pOutput->u4_fadeInOut_conf,pParam->u5_fadeInOut_dh_close_thr);    	

		//except sc
		if(pParam->u1_fdIO_sc_clear_en == 1)
		{
			pOutput->u1_fadeInOut_dh_close = (s_pContext->_output_read_comreg.u1_sc_status_dh_rb==1) ? 0 : pOutput->u1_fadeInOut_dh_close;
		}

		pOutput->u32_frm_cnt = pOutput->u32_frm_cnt + 1;
		pOutput->u32_frm_cnt = _CLIP_(pOutput->u32_frm_cnt,0,14);


#if 0


	ReadRegister(KMC_TOP_kmc_top_24_reg,31,31,&ip31);
	ReadRegister(KMC_TOP_kmc_top_24_reg,30,30,&ip30);

	if(ip31)
		rtd_pr_memc_info("[fdio] %x %x %x\t2) %x %x %x\t3) %x %x %x\t4) %x %x\t5) %x %x %x\n",
			pOutput->u1_fadeInOut_dh_close, pOutput->u1_fadeInOut_logo_clr, pOutput->u4_fadeInOut_logo_conf_iir,
			pOutput->u4_fadeInOut_cnt_alpha,pOutput->u4_fadeInOut_cnt_Thr_alpha, pOutput->u4_fadeInOut_apl_diff_alpha,
			cnt_alpha_use*cnt_total_use/16, pOutput->u4_fadeInOut_conf, pOutput->u4_fadeInOut_conf_iir,
			acGreateDc_cnt,dcGreateAc_cnt,acdc_cnt_ratio, acdc_cnt_total, apl_diff);
/*
	if(ip30)
		rtd_pr_memc_info("[fdio] %x %x\t %d %x \t%x %x %x %x %x %x %x %x...%x %x %x %x %x %x %x %x...%x %x %x %x %x %x %x %x...%x %x %x %x %x %x %x %x\n",
			pOutput->u1_fadeInOut_dh_close, pOutput->u1_fadeInOut_logo_clr,
			rgn_cnt, rapl_diff_avg,
			rapl_diff[0],rapl_diff[1],rapl_diff[2],rapl_diff[3],rapl_diff[4],rapl_diff[5],rapl_diff[6],rapl_diff[7],
			rapl_diff[8],rapl_diff[9],rapl_diff[10],rapl_diff[11],rapl_diff[12],rapl_diff[13],rapl_diff[14],rapl_diff[15],
			rapl_diff[16],rapl_diff[17],rapl_diff[18],rapl_diff[19],rapl_diff[20],rapl_diff[21],rapl_diff[22],rapl_diff[23],
			rapl_diff[24],rapl_diff[25],rapl_diff[26],rapl_diff[27],rapl_diff[28],rapl_diff[29],rapl_diff[30],rapl_diff[31]);
*/
#endif
	}
	else
	{
		pOutput->u1_fadeInOut_dh_close = 0;
		pOutput->u4_fadeInOut_conf     = 0;
		pOutput->u4_fadeInOut_conf_iir = 0;
		pOutput->u4_fadeInOut_logo_conf_iir = 0;
		pOutput->u1_fadeInOut_logo_clr = 0;
	}
}
#if 0
//Inside Win Panning Detection(inside good rmv > thr, and no tc, no sc)
//need close dh.
//stream: vertical_monster
//Lin Zha
VOID FRC_InsidePanning_Detect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//unsigned int u16_cnt_totalNum = (s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb);
	int i;

	unsigned char u8_largeMVY = 0;
	unsigned char u8_largeMVX = 0;

	//16=1
	unsigned int tc_rgn_weight[32] = {0,  0,   0,   0,   0,   0,    0,  0,
		                              0,  8,  16,  16,  16,   16,   8,  0,
									  0,  8,  16,  16,  16,   16,   8,  0,
									  0,  0,   0,   0,   0,   0,    0,  0,
		                              };

	unsigned int sc_rgn_weight[32] = {0,  0,   0,   0,   0,   0,    0,  0,
									  0,  8,  16,  16,  16,   16,   8,  0,
									  0,  8,  16,  16,  16,   16,   8,  0,
									  0,  0,   0,   0,   0,   0,    0,  0,
								      };

	unsigned int mv_rgn_weight[32] = {0,  0,   0,   0,   0,   0,    0,  0,
								      0,  8,  16,  16,  16,   16,   8,  0,
									  0,  8,  16,  16,  16,   16,   8,  0,
									  0,  0,   0,   0,   0,   0,    0,  0,
									  };

	 unsigned int unconf_rgn_weight_center[32] = {0,  0,   0,   0,   0,   0,    0,  0,
									      0,  8,  16,  16,  16,   16,   8,  0,
									      0,  8,  16,  16,  16,   16,   8,  0,
										  0,  0,   0,   0,   0,   0,    0,  0,
	                                       };

	 unsigned int unconf_rgn_weight_all[32] = {16,  16,  16,  16,  16,   16,  16, 16,
									      16,  16,  16,  16,  16,   16,  16, 16,
									      16,  16,  16,  16,  16,   16,  16, 16,
									      16,  16,  16,  16,  16,   16,  16, 16, };

	unsigned int tc_avg = 0;
	unsigned int sc_avg = 0;
	int mvx_avg		= 0;
	int mvy_avg		= 0;
	int mv_w_sum		= 0;
	unsigned int unconf_avg   = 0;
	unsigned int unconf_w_sum = 0;
	unsigned int *unconf_w_use;

	for(i=0;i<32;i++) //32 region
	{
		//large mvy, small mvx
		if (  (_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]) < pParam->u10_PanRMV_SmallMV) &&
			  (_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]) > pParam->u10_PanRMV_LargeMV) &&
			  (s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > pParam->u12_PanRMV_Cnt)           &&
			  (s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] < pParam->u12_PanRMV_Unconf)
		    )
		{
			u8_largeMVY++;
		}

		if (  (_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]) < pParam->u10_PanRMV_SmallMV) &&
			(_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]) > pParam->u10_PanRMV_LargeMV) &&
			(s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > pParam->u12_PanRMV_Cnt)           &&
			(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] < pParam->u12_PanRMV_Unconf)
			)
		{
			u8_largeMVX++;
		}

		unconf_w_use = (pParam->inUnconf_sel == 0) ? unconf_rgn_weight_all : unconf_rgn_weight_center;
		if ( s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > 5)
		{
			unconf_w_sum += unconf_w_use[i];
			unconf_avg    = unconf_avg + unconf_w_use[i]*(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i]);
		}

		if ( s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i] > 5)
		{
			unconf_w_sum += unconf_w_use[i];
			unconf_avg    = unconf_avg + unconf_w_use[i]*(s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[i]);
	}

		unconf_avg = (unconf_w_sum == 0) ? 15 : unconf_avg/unconf_w_sum;
	}

	pOutput->u32_unconf_avg         = unconf_avg;
	pOutput->u8_insidePan_LargeXnum = u8_largeMVX;
	pOutput->u8_insidePan_LargeYnum = u8_largeMVY;

	for(i=0;i<32;i++) //32 region
	{
		tc_avg += s_pContext->_output_read_comreg.u22_me_rTC_rb[i] * tc_rgn_weight[i];
		sc_avg += s_pContext->_output_read_comreg.u22_me_rSC_rb[i] * sc_rgn_weight[i];
		mvx_avg   += s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i] * mv_rgn_weight[i];
		mvy_avg   += s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i] * mv_rgn_weight[i];
		mv_w_sum  += mv_rgn_weight[i];
	}

	pOutput->u32_insidePan_avgAbsMVX = _ABS_(mvx_avg)/mv_w_sum;
	pOutput->u32_insidePan_avgAbsMVY = _ABS_(mvy_avg)/mv_w_sum;
	pOutput->u32_insidePan_avgSc = sc_avg/16/32;  //each region.
	pOutput->u32_insidePan_avgTc = tc_avg/16/32;  //each region.

	//current only Y
	if (pOutput->u32_insidePan_avgSc < pParam->u32_avgRngSC_Thr && pOutput->u32_insidePan_avgTc < pParam->u32_avgRngTC_Thr &&
		(pOutput->u8_insidePan_LargeXnum > pParam->u15_PanRMV_FitNumThr || pOutput->u8_insidePan_LargeYnum > pParam->u15_PanRMV_FitNumThr))
	{
		pOutput->u8_is_insidePan = 1;
		if (pOutput->u8_insidePan_LargeXnum > pParam->u15_PanRMV_FitNumThr)
		{
			pOutput->u8_is_insidePanX = 1;
		}
	}
	else
	{
		pOutput->u8_is_insidePan = 0;
		pOutput->u8_is_insidePanX  = 0;
	}


	//temp, just try, need write in write_comReg.c
	//if (pOutput->u8_is_insidePan == 1)
	//{
	//	WriteRegister(FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_ADDR no mat,FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_BITSTART no mat,FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_BITEND no mat, 1);  //gmv
	//}
	//else
	//{
	//	WriteRegister(FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_ADDR no mat,FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_BITSTART no mat,FRC_TOP__KME_ME2_TOP_REG__reg_me2_ph_invalid_sel_BITEND no mat, 3);  //pfv1
	//}
}

//Lin: boundary sad is not bad, boundary mv is small, first test left/right boundary
VOID FRC_BdySmallMV_Detect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i, lft_i, rht_i;

	int lft_idx_lut[4] = {0, 8, 16, 24};
	int rht_idx_lut[4] = {7, 15, 23, 31};

	int lft_smallMV_cnt = 0;
	int rht_smallMV_cnt = 0;
	for (i = 0; i < 4; i++)
	{
		lft_i = lft_idx_lut[i];
		rht_i = rht_idx_lut[i];

		if(_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[lft_i]) < pParam->u8_BdySmall_mvx_thr &&
		   _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[lft_i]) < pParam->u8_BdySmall_mvy_thr &&
		   s_pContext->_output_read_comreg.u25_me_rSAD_rb[lft_i] < pParam->u15_BdySmall_sad_thr)
		{
			lft_smallMV_cnt++;
		}

		if(_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[rht_i]) < pParam->u8_BdySmall_mvx_thr &&
			_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[rht_i]) < pParam->u8_BdySmall_mvy_thr &&
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[rht_i] < pParam->u15_BdySmall_sad_thr)
		{
			rht_smallMV_cnt++;
		}
	}

	pOutput->u8_BdySmall_lft_cnt = lft_smallMV_cnt;
	pOutput->u8_BdySmall_rht_cnt = rht_smallMV_cnt;

	if (lft_smallMV_cnt >= pParam->u4_BdySmall_cnt_thr || rht_smallMV_cnt >= pParam->u4_BdySmall_cnt_thr)
	{
		pOutput->u1_is_BdySmall_MV = 1;
	}
	else
	{
		pOutput->u1_is_BdySmall_MV = 0;
	}
}
#endif
VOID FRC_Dynamic_ACDCBld(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//fadeIn/Out for acdc.
	if ((pParam->u1_fadeInOut_en == 1))
	{
		unsigned char u5_ac_dc_bld_apl_sig;
		int i;
		unsigned int acdc_bld_alpha_slope;
		if(pOutput->u4_fadeInOut_conf_iir >= pParam->u4_dynME_acdc_bld_alpha_x2)
		{
			u5_ac_dc_bld_apl_sig = 16;
		}
		else
		{
            acdc_bld_alpha_slope = (pParam->u4_dynME_acdc_bld_alpha_x2-pParam->u4_dynME_acdc_bld_alpha_x1)==0? 0 : 256 * 16/(pParam->u4_dynME_acdc_bld_alpha_x2-pParam->u4_dynME_acdc_bld_alpha_x1);
		 	u5_ac_dc_bld_apl_sig = _IncreaseCurveMapping2(pOutput->u4_fadeInOut_conf_iir, pParam->u4_dynME_acdc_bld_alpha_x1, 16, acdc_bld_alpha_slope, 8);//_IncreaseCurveMapping(pOutput->u4_fadeInOut_conf_iir, pParam->u4_dynME_acdc_bld_alpha_x1, 0, 16, acdc_bld_alpha_slope, 8)
		}

		u5_ac_dc_bld_apl_sig = _CLIP_UBOUND_(u5_ac_dc_bld_apl_sig,16);

		u5_ac_dc_bld_apl_sig = ((pParam->u1_acdc_bld_sc_clear_en == 1)&&(s_pContext->_output_read_comreg.u1_sc_status_rb == 1)) ? 0 : u5_ac_dc_bld_apl_sig;

		if(u5_ac_dc_bld_apl_sig == 16)
		{
			pOutput->u5_dc_ac_bld_alpha = 16;
			pOutput->u5_acdc_bld_holdtime = pParam->u6_dynME_ac_dc_bld_apl_holdfrm;
		}
		else if(pOutput->u5_acdc_bld_holdtime>0)
		{
			pOutput->u5_dc_ac_bld_alpha = 16;
			pOutput->u5_acdc_bld_holdtime = pOutput->u5_acdc_bld_holdtime - 1;
		}
		else
		{
			pOutput->u5_dc_ac_bld_alpha = u5_ac_dc_bld_apl_sig;
			pOutput->u5_acdc_bld_holdtime = 0;
		}

		for (i=0; i<3; i++)
		{
			pOutput->u8_dynME_acdc_bld_x_act[i] = _2Val_RndBlending_(pParam->u8_dynME_acdc_bld_x_fdIO[i],
				pParam->u8_dynME_acdc_bld_x_com[i],pOutput->u5_dc_ac_bld_alpha, 4);
			pOutput->u6_dynME_acdc_bld_y_act[i] = _2Val_RndBlending_(pParam->u6_dynME_acdc_bld_y_fdIO[i],
				pParam->u6_dynME_acdc_bld_y_com[i],pOutput->u5_dc_ac_bld_alpha, 4);
		}

		for (i=0; i<2; i++)
		{
			pOutput->s9_dynME_acdc_bld_s_act[i] = ((int)pOutput->u8_dynME_acdc_bld_x_act[i+1]-(int)pOutput->u8_dynME_acdc_bld_x_act[i]) == 0 ? 255 :
				((int)pOutput->u6_dynME_acdc_bld_y_act[i+1]-(int)pOutput->u6_dynME_acdc_bld_y_act[i])*16 / ((int)pOutput->u8_dynME_acdc_bld_x_act[i+1]-(int)pOutput->u8_dynME_acdc_bld_x_act[i]);

			pOutput->s9_dynME_acdc_bld_s_act[i] = _CLIP_(pOutput->s9_dynME_acdc_bld_s_act[i], -256, 255);
			pOutput->s9_dynME_acdc_bld_s_act[i] = pOutput->s9_dynME_acdc_bld_s_act[i] < 0 ? pOutput->s9_dynME_acdc_bld_s_act[i] + 512 :
												  pOutput->s9_dynME_acdc_bld_s_act[i];
		}

		pOutput->u8_dynME_ac_pix_sad_limt_act = _2Val_RndBlending_(pParam->u8_dynME_ac_pix_sad_limt_fdIO,
				pParam->u8_dynME_ac_pix_sad_limt_com,pOutput->u5_dc_ac_bld_alpha, 4);

		pOutput->u4_acdc_bld_psad_shift_act = _2Val_RndBlending_(pParam->u4_acdc_bld_psad_shift_fdIO,
				pParam->u8_dynME_ac_pix_sad_limt_com,pOutput->u5_dc_ac_bld_alpha, 4);

		#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		for (i=0; i<3; i++)
		{
			pOutput->u8_dynME2_acdc_bld_x_act[i] = pOutput->u8_dynME_acdc_bld_x_act[i] ;
			pOutput->u6_dynME2_acdc_bld_y_act[i] = pOutput->u6_dynME_acdc_bld_y_act[i] ;
		}

		for (i=0; i<2; i++)
		{
			pOutput->s9_dynME2_acdc_bld_s_act[i] = ((int)pOutput->u8_dynME2_acdc_bld_x_act[i+1]-(int)pOutput->u8_dynME2_acdc_bld_x_act[i]) == 0 ? 255 :
				((int)pOutput->u6_dynME2_acdc_bld_y_act[i+1]-(int)pOutput->u6_dynME2_acdc_bld_y_act[i])*16 / ((int)pOutput->u8_dynME2_acdc_bld_x_act[i+1]-(int)pOutput->u8_dynME2_acdc_bld_x_act[i]);

			pOutput->s9_dynME2_acdc_bld_s_act[i] = _CLIP_(pOutput->s9_dynME2_acdc_bld_s_act[i], -256, 255);
			pOutput->s9_dynME2_acdc_bld_s_act[i] = pOutput->s9_dynME2_acdc_bld_s_act[i] < 0 ? pOutput->s9_dynME2_acdc_bld_s_act[i] + 512 :
												  pOutput->s9_dynME2_acdc_bld_s_act[i];
		}

		#endif

	}
	else
	{
		int i;
		for (i=0; i<3; i++)
		{
			pOutput->u8_dynME_acdc_bld_x_act[i] = pParam->u8_dynME_acdc_bld_x_com[i];
			pOutput->u6_dynME_acdc_bld_y_act[i] = pParam->u6_dynME_acdc_bld_y_com[i];
		}

		for (i=0; i<2; i++)
		{
			pOutput->s9_dynME_acdc_bld_s_act[i] = ((int)pOutput->u8_dynME_acdc_bld_x_act[i+1]-(int)pOutput->u8_dynME_acdc_bld_x_act[i]) == 0 ? 255 :
				((int)pOutput->u6_dynME_acdc_bld_y_act[i+1]-(int)pOutput->u6_dynME_acdc_bld_y_act[i])*16 / ((int)pOutput->u8_dynME_acdc_bld_x_act[i+1]-(int)pOutput->u8_dynME_acdc_bld_x_act[i]);

			pOutput->s9_dynME_acdc_bld_s_act[i] = _CLIP_(pOutput->s9_dynME_acdc_bld_s_act[i], -256, 255);
			pOutput->s9_dynME_acdc_bld_s_act[i] = pOutput->s9_dynME_acdc_bld_s_act[i] < 0 ? pOutput->s9_dynME_acdc_bld_s_act[i] + 512 :
												  pOutput->s9_dynME_acdc_bld_s_act[i];
		}
		pOutput->u8_dynME_ac_pix_sad_limt_act = pParam->u8_dynME_ac_pix_sad_limt_com;
		pOutput->u4_acdc_bld_psad_shift_act = pParam->u4_acdc_bld_psad_shift_com;

		#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
		for (i=0; i<3; i++)
		{
			pOutput->u8_dynME2_acdc_bld_x_act[i] = pOutput->u8_dynME_acdc_bld_x_act[i] ;
			pOutput->u6_dynME2_acdc_bld_y_act[i] = pOutput->u6_dynME_acdc_bld_y_act[i] ;
		}

		for (i=0; i<2; i++)
		{
			pOutput->s9_dynME2_acdc_bld_s_act[i] = ((int)pOutput->u8_dynME2_acdc_bld_x_act[i+1]-(int)pOutput->u8_dynME2_acdc_bld_x_act[i]) == 0 ? 255 :
				((int)pOutput->u6_dynME2_acdc_bld_y_act[i+1]-(int)pOutput->u6_dynME2_acdc_bld_y_act[i])*16 / ((int)pOutput->u8_dynME2_acdc_bld_x_act[i+1]-(int)pOutput->u8_dynME2_acdc_bld_x_act[i]);

			pOutput->s9_dynME2_acdc_bld_s_act[i] = _CLIP_(pOutput->s9_dynME2_acdc_bld_s_act[i], -256, 255);
			pOutput->s9_dynME2_acdc_bld_s_act[i] = pOutput->s9_dynME2_acdc_bld_s_act[i] < 0 ? pOutput->s9_dynME2_acdc_bld_s_act[i] + 512 :
												  pOutput->s9_dynME2_acdc_bld_s_act[i];
		}

		#endif
	}
}


VOID FRC_Dynamic_SearchRange(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if (pParam->u1_dynSR_en == 1)
	{
		//for mvx
		int i;
		unsigned int large_mv_num_x   = 0;
		unsigned int middle_mv_num_x  = 0;
		unsigned int small_mv_num_x   = 0;
		unsigned int middle_mvx_avg   = 0;

		unsigned int large_mv_num_y   = 0;
		unsigned int middle_mv_num_y  = 0;
		unsigned int small_mv_num_y   = 0;
		unsigned int middle_mvy_avg   = 0;
//		unsigned char  dynSR_mvy_range0 = 0;

		for (i=0; i<32; i++)  //32 region
		{
			unsigned int mvx       = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]);
			unsigned int mv_cnt    = s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i];
			unsigned int mv_unconf = s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i];

			if (mvx >= pParam->u11_dynSR_rmv_x_thr1 && mv_cnt >= pParam->u12_dynSR_rmv_cnt_thr1 && mv_unconf <= pParam->u12_dynSR_rmv_unconf_thr1)
			{
				large_mv_num_x++;
			}
			else if (mvx >= pParam->u11_dynSR_rmv_x_thr0 && mvx < pParam->u11_dynSR_rmv_x_thr1 && mv_cnt >= pParam->u12_dynSR_rmv_cnt_thr0 && mv_unconf <= pParam->u12_dynSR_rmv_unconf_thr0)
			{
				middle_mvx_avg += mvx;
				middle_mv_num_x++;
			}
			else
			{
				small_mv_num_x++;
			}
		}

		if (large_mv_num_x > pParam->u6_dynSR_rmv_x_num_thr1)
		{
			pOutput->u8_ME1_searchRange_x = pParam->u9_dynSR_mvx_range1;
			pOutput->u2_searchRange_x_status = 3;
		}
		else if (middle_mv_num_x > pParam->u6_dynSR_rmv_x_num_thr0)
		{
			int delta_mv_sr;
			middle_mvx_avg = middle_mv_num_x > 0 ?  middle_mvx_avg/middle_mv_num_x : 0;
			delta_mv_sr    = (middle_mvx_avg - pParam->u11_dynSR_rmv_x_thr0) * (pParam->u9_dynSR_mvx_range1 - pParam->u9_dynSR_mvx_range0) / (pParam->u11_dynSR_rmv_x_thr1 - pParam->u11_dynSR_rmv_x_thr0);
			pOutput->u8_ME1_searchRange_x = pParam->u9_dynSR_mvx_range0 + delta_mv_sr;
			pOutput->u2_searchRange_x_status = 2;
}
		else
		{
			pOutput->u8_ME1_searchRange_x = pParam->u9_dynSR_mvx_range0;
			pOutput->u2_searchRange_x_status = 1;
		}

		pOutput->u8_ME1_searchRange_x = pOutput->u8_ME1_searchRange_x >> 2; // for pix perc.

		//for mvy
		for (i=0; i<32; i++)  //32 region
		{
			unsigned int mvy       = _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]);
			unsigned int mv_cnt    = s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i];
			unsigned int mv_unconf = s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i];

			if (mvy >= pParam->u10_dynSR_rmv_y_thr1 && mv_cnt >= pParam->u12_dynSR_rmv_cnt_thr1 && mv_unconf <= pParam->u12_dynSR_rmv_unconf_thr1)
			{
				large_mv_num_y++;
			}
			else if (mvy >= pParam->u10_dynSR_rmv_y_thr0 && mvy < pParam->u10_dynSR_rmv_y_thr1 && mv_cnt >= pParam->u12_dynSR_rmv_cnt_thr0 && mv_unconf <= pParam->u12_dynSR_rmv_unconf_thr0)
			{
				middle_mvy_avg += mvy;
				middle_mv_num_y++;
			}
			else
			{
				small_mv_num_y++;
			}
		}

//		if((s_pContext->_output_Patch_Manage.u1_BluceMic_Detected == 1) && (s_pParam->_param_Patch_Manage.u1_BluceMic_En == 1))
//		{
//			dynSR_mvy_range0 = 0xA0;
//		}
//		else
//		{
//			dynSR_mvy_range0 = pParam->u8_dynSR_mvy_range0;
//		}

		if (large_mv_num_y > pParam->u6_dynSR_rmv_y_num_thr1)
		{
			pOutput->u8_ME1_searchRange_y = pParam->u8_dynSR_mvy_range1;
			pOutput->u2_searchRange_y_status = 3;
		}
		else if (middle_mv_num_y > pParam->u6_dynSR_rmv_y_num_thr0)
		{
			int delta_mv_sr;
			middle_mvy_avg = middle_mv_num_y > 0 ?  middle_mvy_avg/middle_mv_num_y : 0;
			delta_mv_sr    = (middle_mvy_avg - pParam->u10_dynSR_rmv_y_thr0) * (pParam->u8_dynSR_mvy_range1 - pParam->u8_dynSR_mvy_range0) / (pParam->u10_dynSR_rmv_y_thr1 - pParam->u10_dynSR_rmv_y_thr0);
			pOutput->u8_ME1_searchRange_y = pParam->u8_dynSR_mvy_range0 + delta_mv_sr;
			pOutput->u2_searchRange_y_status = 2;
		}
		else
		{
			pOutput->u8_ME1_searchRange_y = pParam->u8_dynSR_mvy_range0;
			pOutput->u2_searchRange_y_status = 1;
		}

		pOutput->u8_ME1_searchRange_y = pOutput->u8_ME1_searchRange_y >> 2; // for pix perc.
	}
}


VOID FRC_Dynamic_SAD_Diff(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u27_tc_rb     = s_pContext->_output_read_comreg.u27_me_aTC_rb;
	unsigned int sad_diff;
	unsigned int tc_slp, sad_slp;
	//all rightshift 4bit
	tc_slp     = (pParam->u8_dyn_SAD_Diff_TC_gain_max - pParam->u8_dyn_SAD_Diff_TC_gain_min) *  65536 / ((pParam->u27_dyn_SAD_Diff_TC_th_h>>4) - (pParam->u27_dyn_SAD_Diff_TC_th_l>>4));
	pOutput->u8_tc_gain = _IncreaseCurveMapping((u27_tc_rb>>4), (pParam->u27_dyn_SAD_Diff_TC_th_l>>4), pParam->u8_dyn_SAD_Diff_TC_gain_min, pParam->u8_dyn_SAD_Diff_TC_gain_max,tc_slp, 16);

	//becare, Lin, This need use sad diff prev-cur, not absolute sad; shift6bit.
	pOutput->u32_glb_sad_cur = s_pContext->_output_read_comreg.u30_me_aSAD_rb;
	sad_diff                 = (_ABS_DIFF_(pOutput->u32_glb_sad_cur , pOutput->u32_glb_sad_pre))>>6;
	sad_slp				     = (pParam->u8_dyn_SAD_Diff_sadAll_gain_max - pParam->u8_dyn_SAD_Diff_sadAll_gain_min) *  65536 / ((pParam->u28_dyn_SAD_Diff_sadAll_th_h>>6) - (pParam->u28_dyn_SAD_Diff_sadAll_th_l>>6));
	pOutput->u8_saddiff_gain = _IncreaseCurveMapping(sad_diff,(pParam->u28_dyn_SAD_Diff_sadAll_th_l>>6), pParam->u8_dyn_SAD_Diff_sadAll_gain_min, pParam->u8_dyn_SAD_Diff_sadAll_gain_max,sad_slp, 16);

	pOutput->u8_dynSAD_Diff_Gain = pOutput->u8_tc_gain * pOutput->u8_saddiff_gain / 255;

	if (pOutput->u8_dynSAD_Diff_Gain >= pParam->u8_dyn_SADGain_thr)
	{
		pOutput->u1_is_saddiff_large = 1;
	}
	else
	{
		pOutput->u1_is_saddiff_large = 0;
	}

	pOutput->u32_glb_sad_pre = pOutput->u32_glb_sad_cur;
}

VOID FRC_RgPanning_Detect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
#if 0
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if (pParam->u1_RgPan_Detect_en == 1)
	{
		int idx;
		unsigned char  rg_pann_alp;
		unsigned int rg_mv_diff_prm  = pParam->u16_RgPan_rMV_th_h - pParam->u16_RgPan_rMV_th_l;
		unsigned int rg_mv_slope      = rg_mv_diff_prm <= 0 ? 0 : 16 * 255 / rg_mv_diff_prm;

		unsigned int rg_sc_diff_prm  = pParam->u22_RgPan_rSC_th_h - pParam->u22_RgPan_rSC_th_l;
		unsigned int rg_sc_slope      = rg_sc_diff_prm <= 0 ? 0 : 16 * 255 / rg_sc_diff_prm;

		unsigned int rg_tc_diff_prm  = pParam->u22_RgPan_rTC_th_h - pParam->u22_RgPan_rTC_th_l;
		unsigned int rg_tc_slope      = rg_tc_diff_prm <= 0 ? 0 : 16 * 255 / rg_tc_diff_prm;

		pOutput->u32_RgPan_idx  = 0;

		for (idx=0; idx<32; idx++)
		{
			unsigned int rg_mv = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[idx]) + _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[idx]);
			unsigned char  rg_mv_alp = _IncreaseCurveMapping2(rg_mv, pParam->u16_RgPan_rMV_th_l, 16, rg_mv_slope, 8);//_IncreaseCurveMapping(rg_mv, pParam->u16_RgPan_rMV_th_l, 0, 16, rg_mv_slope, 8)

			unsigned char  rg_sc_alp = _IncreaseCurveMapping2(s_pContext->_output_read_comreg.u22_me_rSC_rb[idx], pParam->u22_RgPan_rSC_th_l, 16, rg_sc_slope, 8);//_IncreaseCurveMapping(s_pContext->_output_read_comreg.u22_me_rSC_rb[idx], pParam->u22_RgPan_rSC_th_l, 0, 16, rg_sc_slope, 8)

			unsigned char  rg_tc_alp = _IncreaseCurveMapping2(s_pContext->_output_read_comreg.u22_me_rTC_rb[idx], pParam->u22_RgPan_rTC_th_l, 16, rg_tc_slope, 8);//_IncreaseCurveMapping(s_pContext->_output_read_comreg.u22_me_rTC_rb[idx], pParam->u22_RgPan_rTC_th_l, 0, 16, rg_tc_slope, 8)

			rg_sc_alp = _CLIP_((16- rg_sc_alp),0,16);
			rg_tc_alp = _CLIP_((16- rg_tc_alp),0,16);

			rg_pann_alp = (rg_mv_alp * rg_sc_alp * rg_tc_alp)/(16*16);

			if (rg_pann_alp > pParam->u5_RgPan_alp_th)
			{
				pOutput->u32_RgPan_idx = pOutput->u32_RgPan_idx | (1<<idx);
			}

		}
	}
	else
	{
		pOutput->u32_RgPan_idx = 0;
	}
#endif
}

VOID FRC_Still_Pattern_Detect(const _PARAM_FilmDetectCtrl *pParam,  _OUTPUT_FilmDetectCtrl *pOutput_film,  _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	//const _PQLCONTEXT *s_pContext = GetPQLContext();
	static unsigned int still_cnt = 0;
	unsigned char u1_still = 0;
	static unsigned char u8_pre_cadence_Id = 0;
	unsigned int mot_diff_th = 0x8;
	unsigned int frmCnt_th = 4 * frc_cadTable[pOutput_film->u8_cur_cadence_out[_FILM_ALL]].cad_outLen / frc_cadTable[pOutput_film->u8_cur_cadence_out[_FILM_ALL]].cad_inLen;
	unsigned int mot_diff = pOutput_film->u27_ipme_motionPool[_FILM_ALL][0];

	if(u8_pre_cadence_Id != pOutput_film->u8_cur_cadence_out[_FILM_ALL])
	{
		u8_pre_cadence_Id = pOutput_film->u8_cur_cadence_out[_FILM_ALL];
		still_cnt = 0;
	}
	if(pOutput_film->u8_cur_cadence_out[_FILM_ALL] >= _FRC_CADENCE_NUM_)
	{
		rtd_pr_memc_notice("[%s]Invalid u8_cur_cadence_out[_FILM_ALL]=%d.Resume!\n",__FUNCTION__,pOutput_film->u8_cur_cadence_out[_FILM_ALL]);
		pOutput_film->u8_cur_cadence_out[_FILM_ALL] = _CAD_VIDEO;
	}

	if(mot_diff < mot_diff_th)
	{
		still_cnt = ((still_cnt < 255) ? (still_cnt + 1) : 255);
	}
	else
	{
		if(pOutput->u8_OSDPictureModeChg_prt_apply==0){ // keep still_cnt status when u8_OSDPictureModeChg_prt_apply for zomm control
			still_cnt = 0;
		}
	}

	if(pOutput_film->u8_cur_cadence_out[_FILM_ALL] == _CAD_22)
	{
		frmCnt_th = 16;
	}

	if(pParam->u1_StillDetect_En == 1 && still_cnt > frmCnt_th)
	{
		u1_still = 1;
	}

	pOutput->u1_still_frame = u1_still;
}

VOID FRC_Zoom_Detect(const _PARAM_FRC_SceneAnalysis *pParam,  _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	if(pParam->u1_zoom_detect_en)
	{
		if(s_pContext->_external_info.u1_ZoomAction == 1 || s_pContext->_external_info.u1_ShiftAction == 1)
		{
			pOutput->u8_zoom_force_fb_cnt = 20;
		}
		else if(pOutput->u8_zoom_force_fb_cnt != 0)
		{
			pOutput->u8_zoom_force_fb_cnt--;
		}
		else
		{
			pOutput->u8_zoom_force_fb_cnt = 0;
		}
	}
	else
	{
		pOutput->u8_zoom_force_fb_cnt = 0;
	}

}




VOID FRC_SpecialSCFB_Detect(const _PARAM_FRC_SceneAnalysis *pParam,  _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;

	if(pParam->u1_special_scfb_en && s_pContext->_output_wrt_comreg.u8_FB_lvl<0x20 &&
		(u8_cadence_Id == _CAD_22 || u8_cadence_Id == _CAD_32 || ((in_fmRate == _PQL_IN_24HZ || in_fmRate == _PQL_IN_25HZ) && (u8_cadence_Id == _CAD_VIDEO))))
	{
		pOutput->u1_specialscfb_true = 1;
	}
	else
	{
		pOutput->u1_specialscfb_true = 0;
	}
}

VOID FRC_FastMotion_Detect(const _PARAM_FRC_SceneAnalysis *pParam,  _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned int u32_rimRatio = (s_pContext->_output_rimctrl.u32_rimRatio == 0) ? 128 : s_pContext->_output_rimctrl.u32_rimRatio;
	unsigned int hpan_th_rb, vpan_th_rb, cnt2_th_rb, u25_glb_dtl_th, u20_hpan_cnt, u20_vpan_cnt, u25_me_glb_dtl;
	unsigned int hpan_th, vpan_th, cnt2_th;
	unsigned char u8_fastMotion_entercnt_th = 0x0, u8_fastMotion_holdCnt_max = 0x30, u1_fastMotion_cond = 0;
	static unsigned char u8_fastMotion_holdcnt = 0;
	static unsigned char u8_fastMotion_entercnt = 0;

	u32_rimRatio = (u32_rimRatio > 128) ? 128:u32_rimRatio;

	hpan_th_rb = pParam->u8_hpan_th;
	vpan_th_rb = pParam->u8_vpan_th;
	cnt2_th_rb = pParam->u8_cnt2_th;

	hpan_th = (hpan_th_rb << 8) * u32_rimRatio / 128;
	vpan_th = (vpan_th_rb << 8) * u32_rimRatio / 128;
	cnt2_th = cnt2_th_rb << 4;

	u25_glb_dtl_th = (pParam->u25_glb_dtl_th << 8) * u32_rimRatio / 128;

	u20_hpan_cnt = s_pContext->_output_read_comreg.u20_me_Hpan_cnt_rb;
	u20_vpan_cnt = s_pContext->_output_read_comreg.u20_me_Vpan_cnt_rb;
	u25_me_glb_dtl = s_pContext->_output_read_comreg.u25_me_aDTL_rb;

	if( u20_hpan_cnt < hpan_th && u20_vpan_cnt < vpan_th
		&& (s_pContext->_output_read_comreg.u17_me_posCnt_2_rb+s_pContext->_output_read_comreg.u17_me_negCnt_2_rb) > cnt2_th
		&& u25_me_glb_dtl > u25_glb_dtl_th)
	{
		u1_fastMotion_cond = 1;
	}
	else
	{
		u1_fastMotion_cond = 0;
	}

	u8_fastMotion_entercnt = (u1_fastMotion_cond == 1) ? (u8_fastMotion_entercnt + 1) : 0;
	u8_fastMotion_entercnt = _CLIP_UBOUND_(u8_fastMotion_entercnt, 254);

	u8_fastMotion_holdcnt = (u8_fastMotion_entercnt > u8_fastMotion_entercnt_th) ? u8_fastMotion_holdCnt_max :
						((u8_fastMotion_holdcnt > 0) ? (u8_fastMotion_holdcnt - 1) : 0);

	pOutput->u1_fastMotion_det = (u8_fastMotion_holdcnt > 0 && pParam->u1_fastMotion_det_en) ? 1 : 0;
/*
	if(pOutput->u1_fastMotion_det)
	{
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,9,12, 0x0F);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,13,16, 0x0F);
	}
	else
	{
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,9,12, 0x07);
		WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_40_reg,13,16, 0x07);
	}
*/
}

VOID FRC_ZMV_AdaptPenality(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u11_GMV_mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	unsigned int u10_GMV_mvy = _ABS_(s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);

	if(pParam->u1_zmv_adapt_pnt_en)
	{
		if((u11_GMV_mvx < pParam->u11_zmv_adapt_pnt_GMV_mvx_thr) &&
		   (u10_GMV_mvy < pParam->u10_zmv_adapt_pnt_GMV_mvy_thr))
		{
			pOutput->u1_zmv_adapt_pnt_GMV_Small = TRUE;
		}
		else
		{
			pOutput->u1_zmv_adapt_pnt_GMV_Small = FALSE;
		}
	}
	else
	{
		pOutput->u1_zmv_adapt_pnt_GMV_Small = FALSE;
	}
}

VOID FRC_ME1_GMVD_Select(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u11_GMV_1st_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	unsigned int u10_GMV_1st_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	unsigned int u17_GMV_1st_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;

	unsigned int u11_GMV_2nd_mvx = s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb;
	unsigned int u10_GMV_2nd_mvy = s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb;
	unsigned int u17_GMV_2nd_cnt = s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb;

	unsigned int u16_cnt_totalNum = s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb;
	unsigned int mvx_diff = 0, mvy_diff = 0;

	if(u11_GMV_1st_mvx >= u11_GMV_2nd_mvx){
		mvx_diff = u11_GMV_1st_mvx - u11_GMV_2nd_mvx;
	}
	else{
		mvx_diff = u11_GMV_2nd_mvx - u11_GMV_1st_mvx;
	}
	if(u10_GMV_1st_mvy >= u10_GMV_2nd_mvy){
		mvy_diff = u10_GMV_1st_mvy - u10_GMV_2nd_mvy;
	}
	else{
		mvy_diff = u10_GMV_2nd_mvy - u10_GMV_1st_mvy;
	}
	
	if(pParam->u1_me1_gmvd_sel_en)
	{
		if(((mvx_diff > pParam->u11_me1_gmvd_sel_mvx_diff_thr) ||
		     (mvx_diff > pParam->u10_me1_gmvd_sel_mvy_diff_thr)) &&
		     (u17_GMV_1st_cnt > ((pParam->u4_me1_gmvd_sel_1st_gmv_ratio_thr * u16_cnt_totalNum) >> 4)) &&
		     (u17_GMV_2nd_cnt > ((pParam->u4_me1_gmvd_sel_2nd_gmv_ratio_thr * u16_cnt_totalNum) >> 4)))
		{
			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,1,1, 0);	// rmv
		}
		else
		{
			WriteRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg,1,1, 1); 	//gmv
		}
	}
}

VOID FRC_Good_Scene_Detect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i;
	unsigned char u8_GoodRegion_cnt = 0, u8_BadRegion_cnt = 0;
	unsigned int ui_Good_Rgn_SAD_th = 0, ui_Good_gmv_unconf_th = 0, ui_Good_Rgn_TC_th;
	unsigned int ui_Bad_Rgn_SAD_th = 0, ui_Bad_gmv_unconf_th = 0, ui_Bad_Rgn_TC_th;

	if(pParam->u1_SA_en)
	{
		for(i = 0; i < 32; i++)
		{
			if(s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] >= 16)
			{
				if((i >= 0 && i < 8) || (i >= 24 && i < 32))
				{
					ui_Good_Rgn_SAD_th = (540 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u20_SA_good_rgn_SAD_th / 540;
					ui_Good_gmv_unconf_th = (540 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u12_SA_good_rgn_unconf_th / 540;
					ui_Good_Rgn_TC_th = (540 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u12_SA_good_rgn_TC_th / 540;
					ui_Bad_Rgn_SAD_th = (540 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u20_SA_bad_rgn_SAD_th / 540;
					ui_Bad_gmv_unconf_th = (540 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u12_SA_bad_rgn_unconf_th / 540;
					ui_Bad_Rgn_TC_th = (540 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u12_SA_bad_rgn_TC_th / 540;
				}
				else
				{
					ui_Good_Rgn_SAD_th = pParam->u20_SA_good_rgn_SAD_th;
					ui_Good_gmv_unconf_th = pParam->u12_SA_good_rgn_unconf_th;
					ui_Good_Rgn_TC_th = pParam->u12_SA_good_rgn_TC_th;
					ui_Bad_Rgn_SAD_th = pParam->u20_SA_bad_rgn_SAD_th;
					ui_Bad_gmv_unconf_th = pParam->u12_SA_bad_rgn_unconf_th;
					ui_Bad_Rgn_TC_th = pParam->u12_SA_bad_rgn_TC_th;
				}
			}
			else if(s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT] >= 24)
			{
				if((i == 0) || (i == 8) || (i == 16) || (i == 24) || (i == 7) || (i == 15) || (i == 23) || (i == 31))
				{
					ui_Good_Rgn_SAD_th = (480 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u20_SA_good_rgn_SAD_th / 480;
					ui_Good_gmv_unconf_th = (480 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u12_SA_good_rgn_unconf_th / 480;
					ui_Good_Rgn_TC_th = (480 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u12_SA_good_rgn_TC_th / 480;
					ui_Bad_Rgn_SAD_th = (480 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u20_SA_bad_rgn_SAD_th / 480;
					ui_Bad_gmv_unconf_th = (480 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u12_SA_bad_rgn_unconf_th / 480;
					ui_Bad_Rgn_TC_th = (480 - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u12_SA_bad_rgn_TC_th / 480;
				}
				else
				{
					ui_Good_Rgn_SAD_th = pParam->u20_SA_good_rgn_SAD_th;
					ui_Good_gmv_unconf_th = pParam->u12_SA_good_rgn_unconf_th;
					ui_Good_Rgn_TC_th = pParam->u12_SA_good_rgn_TC_th;
					ui_Bad_Rgn_SAD_th = pParam->u20_SA_bad_rgn_SAD_th;
					ui_Bad_gmv_unconf_th = pParam->u12_SA_bad_rgn_unconf_th;
					ui_Bad_Rgn_TC_th = pParam->u12_SA_bad_rgn_TC_th;
				}
			}
			else
			{
				ui_Good_Rgn_SAD_th = pParam->u20_SA_good_rgn_SAD_th;
				ui_Good_gmv_unconf_th = pParam->u12_SA_good_rgn_unconf_th;
				ui_Good_Rgn_TC_th = pParam->u12_SA_good_rgn_TC_th;
				ui_Bad_Rgn_SAD_th = pParam->u20_SA_bad_rgn_SAD_th;
				ui_Bad_gmv_unconf_th = pParam->u12_SA_bad_rgn_unconf_th;
				ui_Bad_Rgn_TC_th = pParam->u12_SA_bad_rgn_TC_th;
			}

			ui_Good_Rgn_TC_th = ui_Good_Rgn_TC_th << 10;
			ui_Bad_Rgn_TC_th = ui_Bad_Rgn_TC_th << 10;

			if((s_pContext->_output_read_comreg.u25_me_rSAD_rb[i] < ui_Good_Rgn_SAD_th) &&
			   (s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] < ui_Good_gmv_unconf_th) &&
			   (s_pContext->_output_read_comreg.u22_me_rTC_rb[i] < ui_Good_Rgn_TC_th))
			{
				u8_GoodRegion_cnt++;
#if 0
				LogPrintf(DBG_MSG,"%08x  %08x  %08x  %08x  %08x  %08x\r\n",
									s_pContext->_output_read_comreg.u25_me_rSAD_rb[i], ui_Good_Rgn_SAD_th,
									s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i], ui_Good_gmv_unconf_th,
									s_pContext->_output_read_comreg.u22_me_rTC_rb[i], ui_Good_Rgn_TC_th);
#endif
			}

			if((s_pContext->_output_read_comreg.u25_me_rSAD_rb[i] >= ui_Bad_Rgn_SAD_th) ||
			   (s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] >= ui_Bad_gmv_unconf_th) ||
			   (s_pContext->_output_read_comreg.u22_me_rTC_rb[i] >= ui_Bad_Rgn_TC_th))
			{
				u8_BadRegion_cnt++;
#if 0
				LogPrintf(DBG_MSG,"%08x  %08x  %08x  %08x  %08x  %08x\r\n",
									s_pContext->_output_read_comreg.u25_me_rSAD_rb[i], ui_Bad_Rgn_SAD_th,
									s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i], ui_Bad_gmv_unconf_th,
									s_pContext->_output_read_comreg.u22_me_rTC_rb[i], ui_Bad_Rgn_TC_th);
#endif
			}
		}

		if(u8_GoodRegion_cnt >= pParam->u5_SA_good_rgn_num_th)
		{
			pOutput->u8_SA_good_scene_cnt = (pOutput->u8_SA_good_scene_cnt < 16) ? pOutput->u8_SA_good_scene_cnt + 1 : 16;
		}
		else
		{
			pOutput->u8_SA_good_scene_cnt = 0;
		}

		if(pOutput->u8_SA_good_scene_cnt > pParam->u4_SA_good_scene_enter_cnt)
		{
			pOutput->u1_SA_good_scene = 1;
		}
		else
		{
			pOutput->u1_SA_good_scene = 0;
		}

		if(u8_BadRegion_cnt >= pParam->u5_SA_bad_rgn_num_th)
		{
			pOutput->u8_SA_bad_scene_cnt = (pOutput->u8_SA_bad_scene_cnt < 16) ? pOutput->u8_SA_bad_scene_cnt + 1 : 16;
		}
		else
		{
			pOutput->u8_SA_bad_scene_cnt = 0;
		}

		if(pOutput->u8_SA_bad_scene_cnt > pParam->u4_SA_bad_scene_enter_cnt)
		{
			pOutput->u1_SA_bad_scene = 1;
			pOutput->u3_SA_bad_scene_keep_cnt = (pParam->u3_SA_bad_scene_keep_cnt << 3);
		}
		else if(pOutput->u3_SA_bad_scene_keep_cnt > 0)
		{
			pOutput->u3_SA_bad_scene_keep_cnt = pOutput->u3_SA_bad_scene_keep_cnt - 1;
			pOutput->u1_SA_bad_scene = 1;
		}
		else
		{
			pOutput->u3_SA_bad_scene_keep_cnt = 0;
			pOutput->u1_SA_bad_scene = 0;
		}

		if(pOutput->u1_SA_bad_scene == 1)
		{
			pOutput->u1_SA_good_scene = 0;
		}
#if 0
		LogPrintf(DBG_MSG,"BadRegion_cnt = %d, SA_bad_scene_cnt = %d, SA_bad_scene = %d\r\n",
													u8_BadRegion_cnt,
												   	pOutput->u8_SA_bad_scene_cnt,
												   	pOutput->u1_SA_bad_scene);
#endif
	}
}

VOID FRC_SC_PFV_CDD_Number(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
    const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u1_hw_sc_signal = s_pContext->_output_read_comreg.u1_sc_status_rb;

	if (u1_hw_sc_signal == 1)
	{
	    pOutput->u8_SC_PFV_CDD_hold_cnt = pParam->u8_SC_PFV_CDD_hold_cnt;
	}
	else if (pOutput->u8_SC_PFV_CDD_hold_cnt > 0)
	{
	    pOutput->u8_SC_PFV_CDD_hold_cnt = pOutput->u8_SC_PFV_CDD_hold_cnt - 1;
	}
	else
	{
	    pOutput->u8_SC_PFV_CDD_hold_cnt = 0;
	}
}

VOID FRC_Dynamic_SADDiffTh(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	unsigned int u32_saddiff_th_step = 0;
	unsigned int u32_saddiff_th_tmp = 0;
	unsigned int u32_i = 0, u32_rmv_cnt_sum = 0, u32_cnt = 0, rmv_cnt_avg = 0;

	const _PQLCONTEXT *s_pContext = GetPQLContext();

	int u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb) >> 2;
	int u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb) >> 2;
	unsigned int u16_cnt_totalNum = s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb;
	unsigned char  u6_gmv_ratio = (s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb << 6) / u16_cnt_totalNum;
	//unsigned char  u16_rmv_th_l_offset = 40,u16_rmv_th_h_offset = 50;
	unsigned int  u16_rmv_th_l = (pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l * u16_cnt_totalNum >> 9) ;
	unsigned int  u16_rmv_th_h = (pParam->u4_dyn_SAD_Diff_rmv_ratio_th_h * u16_cnt_totalNum >> 9) ;
	//reference dtl
	unsigned int u25_meAllDtl    = s_pContext->_output_read_comreg.u25_me_aDTL_rb;

	// dynamic rmv_th_l  more dtl mv messy
	if(u25_meAllDtl>=2500000 && pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l >6)
		u16_rmv_th_l=((pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l-6) * u16_cnt_totalNum >> 9) ;
	else if(u25_meAllDtl>=1700000 && pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l >3)
		u16_rmv_th_l=((pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l-3) * u16_cnt_totalNum >> 9) ;
	//=============================

	pOutput->u32_new_saddiff_th = pParam->u28_dyn_SAD_Diff_sadAll_th_l;

	if(pParam->u1_dyn_SAD_Diff_en)
	{


		u32_saddiff_th_tmp = pParam->u28_dyn_SAD_Diff_sadAll_th_h - _MIN_(pParam->u28_dyn_SAD_Diff_sadAll_th_l , pParam->u28_dyn_SAD_Diff_sadAll_th_h);
		for (u32_i = 0; u32_i < 32; u32_i ++)
		{
			if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[u32_i] < 2 &&
				_ABS_DIFF_(u11_gmv_mvx,(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u32_i] >> 2)) < 8 &&
				_ABS_DIFF_(u10_gmv_mvy,(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u32_i] >> 2)) < 6)
			{
				u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[u32_i];
				u32_cnt++;
			}
		}
#if 0
		rtd_pr_memc_notice("[%s]u32_rmv_cnt_sum=%d, u32_cnt=%d, saddiff_th_tmp = %d\n",__FUNCTION__,u32_rmv_cnt_sum, u32_cnt, u32_saddiff_th_tmp);
		rtd_pr_memc_notice("[%s]mv_info = %d,%d,%d \n",__FUNCTION__,u10_gmv_mvy,u11_gmv_mvx, u6_gmv_ratio);
		rtd_pr_memc_notice("[%s]rmv_th_h l=%d h=%d \n",__FUNCTION__,u16_rmv_th_l,u16_rmv_th_h);
#endif
		if(pOutput->u1_Zoom_true  && u6_gmv_ratio>(pParam->u6_dyn_SAD_Diff_gmv_ratio_th/2))    //  zoom in/out
		{
			u32_saddiff_th_step = ((u32_saddiff_th_tmp << 2) / (u16_rmv_th_h - (u16_rmv_th_l>>2)));
			if(u32_cnt)
			{
				rmv_cnt_avg = u32_rmv_cnt_sum/u32_cnt;

				if(rmv_cnt_avg >= u16_rmv_th_h)
				{
					pOutput->u32_new_saddiff_th = pParam->u28_dyn_SAD_Diff_sadAll_th_h;
				}
				else if(rmv_cnt_avg > (u16_rmv_th_l>>2))
				{
					pOutput->u32_new_saddiff_th = (((rmv_cnt_avg -  (u16_rmv_th_l>>2)) * u32_saddiff_th_step) >> 2) + \
							s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_sc_saddiff_th;
				}
			}
		}
		else if((u6_gmv_ratio > pParam->u6_dyn_SAD_Diff_gmv_ratio_th) && (_ABS_(u11_gmv_mvx) >= 2 || _ABS_(u10_gmv_mvy) >= 1))  // normal
		{
			u32_saddiff_th_step = ((u32_saddiff_th_tmp << 2) / (u16_rmv_th_h - u16_rmv_th_l));
			if(u32_cnt)
			{
				rmv_cnt_avg = u32_rmv_cnt_sum/u32_cnt;
				if(rmv_cnt_avg >= u16_rmv_th_h)
				{
					pOutput->u32_new_saddiff_th = pParam->u28_dyn_SAD_Diff_sadAll_th_h;
				}
				else if(rmv_cnt_avg > u16_rmv_th_l)
				{
					pOutput->u32_new_saddiff_th = (((rmv_cnt_avg - u16_rmv_th_l) * u32_saddiff_th_step) >> 2) + \
							s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_sc_saddiff_th;
				}
			}
		}
		else if((u6_gmv_ratio > pParam->u6_dyn_SAD_Diff_gmv_ratio_th) && (_ABS_(u11_gmv_mvx) <= 1 && _ABS_(u10_gmv_mvy) <= 1))
		{
			u32_saddiff_th_step = ((u32_saddiff_th_tmp << 2) / (u16_rmv_th_h - u16_rmv_th_l));
			if(u32_cnt)
			{
				rmv_cnt_avg = u32_rmv_cnt_sum/u32_cnt;
				if(rmv_cnt_avg >= u16_rmv_th_h)
				{
					pOutput->u32_new_saddiff_th = pParam->u28_dyn_SAD_Diff_sadAll_th_h*3;
				}
				else if(rmv_cnt_avg > u16_rmv_th_l)
				{
					pOutput->u32_new_saddiff_th = (((rmv_cnt_avg - u16_rmv_th_l) * u32_saddiff_th_step) >> 2) + \
							s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_sc_saddiff_th;
				}
			}
		}

#if 0
		rtd_pr_memc_notice("[%s]new_saddiff_th = %d, %d, %d \n",__FUNCTION__,pOutput->u32_new_saddiff_th, rmv_cnt_avg, u32_saddiff_th_step );
#endif
		pOutput->u32_new_saddiff_th = (pOutput->u32_new_saddiff_th * 1+pOutput->u32_new_saddiff_th_pre * 7) >> 3;
		pOutput->u32_new_saddiff_th_pre= pOutput->u32_new_saddiff_th;
	}
}

VOID FRC_Dynamic_MVAccord(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u11_GMV_1st_mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	unsigned int u10_GMV_1st_mvy = _ABS_(s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	unsigned int u16_cnt_totalNum = s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb;
	unsigned char  u6_gmv_ratio = (s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb << 6) / u16_cnt_totalNum;

	if(pParam->u1_mv_accord_en)
	{
		if((u11_GMV_1st_mvx < pParam->u6_mv_accord_mvx_thr) &&
			(u10_GMV_1st_mvy < pParam->u5_mv_accord_mvy_thr) &&
			(u6_gmv_ratio > pParam->u6_mv_accord_gmv_ratio_thr)
		  )
		{
			pOutput->u1_mv_accord_on = 1;
		}
		else
		{
			pOutput->u1_mv_accord_on = 0;
		}
		if(pOutput->u1_mv_accord_on)
		{
			pOutput->u6_mv_accord_hold_cnt = pParam->u6_mv_accord_hold_cnt;
		}
		else if(pOutput->u6_mv_accord_hold_cnt > 0)
		{
			pOutput->u6_mv_accord_hold_cnt = pOutput->u6_mv_accord_hold_cnt - 1;
		}
	}
	else
	{
		pOutput->u1_mv_accord_on = 0;
		pOutput->u6_mv_accord_hold_cnt = 0;
	}
}
VOID FRC_RgRotat_Detect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned char  nIdxX,nIdxY,nMap0,nMap1,nMap2,nMap3;
	unsigned char  nRecordMap0,nRecordMap1,nRecordMap2,nRecordMap3;
	unsigned char rg_cnt = 0;
	int nRMVx_1st[4]={0};
	int nRMVy_1st[4]={0};
	int nRMVx_2nd[4]={0};
	int nRMVy_2nd[4]={0};
	static unsigned char  u8_RgRotat_holdFrm=0;
	unsigned int rg_cnt_th=8,u8_RgRotat_holdFrmNum=8;
	static unsigned int u17_avgZMV_cnt_pre;
	unsigned int u17_avgZMV_cnt =0;
	unsigned int u32_rmv_cnt_sum = 0,u32_rmv_cnt_avg =0,u32_rmv_tc_sum = 0,u32_rmv_tc_avg =0,u32_dh_pfvconf_sum = 0,u32_dh_pfvconf_avg =0,u32_dh_pfvconf_diff =0,u32_rng_DTL_sum = 0,u32_rng_DTL_avg =0;
	unsigned int u26_aAPL_th=0x2a0000,u26_aDTL_th=0xc0000,u32_rMV_cnt_th1=0x2bc,u32_rMV_cnt_th2=0x352,u17_aZMV_cnt_th=0x0,u27_tc_th1 = 18000,u27_tc_th2 = 30000,u20_dh_pfvconf_diff_th1=24000,u20_dh_pfvconf_diff_th2=40000,u20_rngDTL_th=11000;
	unsigned int u26_aAPLi_rb=s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned int u26_aAPLp_rb=s_pContext->_output_read_comreg.u26_me_aAPLp_rb;
	unsigned int u26_aDTL_rb=s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	unsigned int u17_aZMV_cnt_rb=s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb;
	unsigned int u32_rimRatio = (s_pContext->_output_rimctrl.u32_rimRatio == 0) ? 128 : s_pContext->_output_rimctrl.u32_rimRatio;
	#if 0
	ReadRegister(SOFTWARE1_SOFTWARE1_41_reg, 0, 31, &u32_rMV_cnt_th1);
	ReadRegister(SOFTWARE1_SOFTWARE1_42_reg, 0, 31, &u32_rMV_cnt_th2);
	ReadRegister(SOFTWARE1_SOFTWARE1_43_reg, 0, 31, &u27_tc_th1);
	ReadRegister(SOFTWARE1_SOFTWARE1_44_reg, 0, 31, &u27_tc_th2);
	ReadRegister(SOFTWARE1_SOFTWARE1_45_reg, 0, 31, &u20_rngDTL_th);
	ReadRegister(SOFTWARE1_SOFTWARE1_46_reg, 0, 31, &u8_RgRotat_holdFrmNum);
	ReadRegister(SOFTWARE1_SOFTWARE1_47_reg, 0,31, &u20_dh_pfvconf_diff_th1);
	ReadRegister(SOFTWARE1_SOFTWARE1_48_reg, 0,31, &u20_dh_pfvconf_diff_th2);
	#endif
	u26_aAPL_th = (u26_aAPL_th * u32_rimRatio) >> 7;
	u17_avgZMV_cnt = (u17_avgZMV_cnt_pre*7 + u17_aZMV_cnt_rb)>>3;
	pOutput->u1_RgRotat_true  = 0;
	pOutput->u4_RgRotat_cnt = 0;
	for (nIdxX=0; nIdxX<=5; nIdxX++)
	{
		for (nIdxY=0; nIdxY<=1; nIdxY++)
		{
			//slow rotat
			// init
			rg_cnt = 0;
			u32_rmv_cnt_sum=0;
			nMap0 = (nIdxY+1)*8+nIdxX;
			nMap1 = nIdxY*8+nIdxX+1;
			nMap2 = (nIdxY+2)*8+nIdxX+1;
			nMap3 = (nIdxY+1)*8+nIdxX+2;
			// left
			nRMVx_1st[0] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap0];
			nRMVy_1st[0] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap0];
			nRMVx_2nd[0] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[nMap0];
			nRMVy_2nd[0] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[nMap0];
			// top
			nRMVx_1st[1] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap1];
			nRMVy_1st[1] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap1];
			nRMVx_2nd[1] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[nMap1];
			nRMVy_2nd[1] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[nMap1];
			// bot
			nRMVx_1st[2] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap2];
			nRMVy_1st[2] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap2];
			nRMVx_2nd[2] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[nMap2];
			nRMVy_2nd[2] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[nMap2];
			// right
			nRMVx_1st[3] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap3];
			nRMVy_1st[3] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap3];
			nRMVx_2nd[3] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[nMap3];
			nRMVy_2nd[3] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[nMap3];
			// left top
			if(((nRMVx_1st[0] * nRMVx_1st[1] >0) && (nRMVy_1st[0] *nRMVy_1st[1]<0)) || ((nRMVx_2nd[0] * nRMVx_2nd[1] >0) && (nRMVy_2nd[0] *nRMVy_2nd[1]<0)))
			{
				if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap0] > 2 && s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap1] > 2)
				{
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap0];
					rg_cnt++;
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap1];
					rg_cnt++;
				}
			}
			// top right
			if(((nRMVx_1st[1] * nRMVx_1st[3] <0) && (nRMVy_1st[1] *nRMVy_1st[3]>0)) || ((nRMVx_2nd[1] * nRMVx_2nd[3] <0) && (nRMVy_2nd[1] *nRMVy_2nd[3]>0)) )
			{
				if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap1] > 2 && s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap3] > 2)
				{
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap1];
					rg_cnt++;
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap3];
					rg_cnt++;
				}
			}
			// left right
			if(((nRMVx_1st[0] * nRMVx_1st[3] <0) && (nRMVy_1st[0] *nRMVy_1st[3]<0)) || ((nRMVx_2nd[0] * nRMVx_2nd[3] <0) && (nRMVy_2nd[0] *nRMVy_2nd[3]<0)) )
			{
				if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap0] > 2 && s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap3] > 2)
				{
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap0];
					rg_cnt++;
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap3];
					rg_cnt++;
				}
			}
			// left bot
			if(((nRMVx_1st[0] * nRMVx_1st[2] <0) && (nRMVy_1st[0] *nRMVy_1st[2]>0)) || ((nRMVx_2nd[0] * nRMVx_2nd[2] <0) && (nRMVy_2nd[0] *nRMVy_2nd[2]>0)))
			{
				if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap0] > 2 && s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap2] > 2)
				{
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap0];
					rg_cnt++;
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap2];
					rg_cnt++;
				}
			}
			// top bot
			if(((nRMVx_1st[1] * nRMVx_1st[2] <0) && (nRMVy_1st[1] *nRMVy_1st[2]<0)) ||((nRMVx_2nd[1] * nRMVx_2nd[2] <0) && (nRMVy_2nd[1] *nRMVy_2nd[2]<0)))
			{
				if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap1] > 2 && s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap2] > 2)
				{
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap1];
					rg_cnt++;
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap2];
					rg_cnt++;
				}
			}
			// bot right
			if(((nRMVx_1st[2] * nRMVx_1st[3] >0) && (nRMVy_1st[2] *nRMVy_1st[3]<0)) ||((nRMVx_2nd[2] * nRMVx_2nd[3] >0) && (nRMVy_2nd[2] *nRMVy_2nd[3]<0)) )
			{
				if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap2] > 2 && s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap3] > 2)
				{
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap2];
					rg_cnt++;
					u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap3];
					rg_cnt++;
				}
			}

			// rotat
			if(rg_cnt <rg_cnt_th )
			{
				// init
				rg_cnt = 0;
				u32_rmv_cnt_sum=0;
				nMap0 = nIdxY*8+nIdxX;
				nMap1 = nIdxY*8+nIdxX+2;
				nMap2 = (nIdxY+2)*8+nIdxX;
				nMap3 = (nIdxY+2)*8+nIdxX+2;
			  	// left_top
				nRMVx_1st[0] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap0]>>1;
				nRMVy_1st[0] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap0]>>1;
				nRMVx_2nd[0] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[nMap0]>>1;
				nRMVy_2nd[0] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[nMap0]>>1;
				// right_top
				nRMVx_1st[1] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap1]>>1;
				nRMVy_1st[1] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap1]>>1;
				nRMVx_2nd[1] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[nMap1]>>1;
				nRMVy_2nd[1] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[nMap1]>>1;
				// left_bot
				nRMVx_1st[2] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap2]>>1;
				nRMVy_1st[2] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap2]>>1;
				nRMVx_2nd[2] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[nMap2]>>1;
				nRMVy_2nd[2] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[nMap2]>>1;
				// right_bot
				nRMVx_1st[3] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap3]>>1;
				nRMVy_1st[3] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap3]>>1;
				nRMVx_2nd[3] = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[nMap3]>>1;
				nRMVy_2nd[3] = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[nMap3]>>1;

				// left_top right_top
				if(((nRMVx_1st[0] * nRMVx_1st[1] >0) && (nRMVy_1st[0] *nRMVy_1st[1]<0)) || ((nRMVx_2nd[0] * nRMVx_2nd[1] >0) && (nRMVy_2nd[0] *nRMVy_2nd[1]<0)))
				{
					if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap0] > 2&& s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap1] > 2)
					{
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap0];
						rg_cnt++;
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap1];
						rg_cnt++;
					}
				}
				// right_top right_bot
				if(((nRMVx_1st[1] * nRMVx_1st[3] <0) && (nRMVy_1st[1] *nRMVy_1st[3]>0)) || ((nRMVx_2nd[1] * nRMVx_2nd[3] <0) && (nRMVy_2nd[1] *nRMVy_2nd[3]>0)) )
				{
					if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap1] > 2&&s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap3] > 2)
					{
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap1];
						rg_cnt++;
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap3];
						rg_cnt++;
					}
				}
				// left_top right_bot
				if(((nRMVx_1st[0] * nRMVx_1st[3] <0) && (nRMVy_1st[0] *nRMVy_1st[3]<0)) || ((nRMVx_2nd[0] * nRMVx_2nd[3] <0) && (nRMVy_2nd[0] *nRMVy_2nd[3]<0)) /*|| ((nRMVx_2nd[0] * nRMVx_1st[3] <0) && (nRMVy_2nd[0] *nRMVy_1st[3]<0))*/)
				{
					if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap0] > 2&&s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap3] > 2)
					{
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap0];
						rg_cnt++;
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap3];
						rg_cnt++;
					}
				}
				// left_top left_bot
				if(((nRMVx_1st[0] * nRMVx_1st[2] <0) && (nRMVy_1st[0] *nRMVy_1st[2]>0)) || ((nRMVx_2nd[0] * nRMVx_2nd[2] <0) && (nRMVy_2nd[0] *nRMVy_2nd[2]>0)))
				{
					if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap0] > 2&&s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap2] > 2)
					{
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap0];
						rg_cnt++;
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap2];
						rg_cnt++;
					}
				}
				// right_top left_bot
				if(((nRMVx_1st[1] * nRMVx_1st[2] <0) && (nRMVy_1st[1] *nRMVy_1st[2]<0)) ||((nRMVx_2nd[1] * nRMVx_2nd[2] <0) && (nRMVy_2nd[1] *nRMVy_2nd[2]<0)))
				{
					if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap1] > 2&&s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap2] > 2)
					{
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap1];
						rg_cnt++;
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap2];
						rg_cnt++;
					}
				}
				// left_bot right_bot
				if(((nRMVx_1st[2] * nRMVx_1st[3] >0) && (nRMVy_1st[2] *nRMVy_1st[3]<0)) ||((nRMVx_2nd[2] * nRMVx_2nd[3] >0) && (nRMVy_2nd[2] *nRMVy_2nd[3]<0)) /*||((nRMVx_2nd[2] * nRMVx_1st[3] >=0) && (nRMVy_2nd[2] *nRMVy_1st[3]<0))*/)
				{
					if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap2] > 2&&s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[nMap3] > 2)
					{
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap2];
						rg_cnt++;
						u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[nMap3];
						rg_cnt++;
					}
				}

			}

			if(rg_cnt >pOutput->u4_RgRotat_cnt )
			{
				pOutput->u4_RgRotat_cnt = rg_cnt;
				u32_rmv_cnt_avg = u32_rmv_cnt_sum/(rg_cnt*1);
				nRecordMap0=nMap0;
				nRecordMap1=nMap1;
				nRecordMap2=nMap2;
				nRecordMap3=nMap3;
			}
		}
	}

	if(pOutput->u4_RgRotat_cnt <rg_cnt_th)
	{
		//reset
		nRecordMap0=0;
		nRecordMap1=0;
		nRecordMap2=0;
		nRecordMap3=0;
		u32_rmv_cnt_avg=0;
		pOutput->u4_RgRotat_cnt  =0;
	}
	#if 1  // for debug
	WriteRegister(SOFTWARE1_SOFTWARE1_39_reg, 0, 7, nRecordMap0);
	WriteRegister(SOFTWARE1_SOFTWARE1_39_reg, 8, 15, nRecordMap1);
	WriteRegister(SOFTWARE1_SOFTWARE1_39_reg, 16, 23, nRecordMap2);
	WriteRegister(SOFTWARE1_SOFTWARE1_39_reg, 24, 31, nRecordMap3);
	WriteRegister(SOFTWARE1_SOFTWARE1_40_reg, 0, 31, u32_rmv_cnt_avg);
	#endif
	// Jiugong
	if(nRecordMap0 != nRecordMap1)
	{
		if(nRecordMap0 > nRecordMap1)//slow rotat
		{
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap0];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap1];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap2];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap3];

			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap1-1];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap1+1];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap2-1];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap2+1];

			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap0];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap1];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap2];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap3];

			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap1-1];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap1+1];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap2-1];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap2+1];

			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap0];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap1];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap2];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap3];

			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap1-1];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap1+1];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap2-1];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap2+1];

			u32_dh_pfvconf_avg= u32_dh_pfvconf_sum>>3;
			u32_dh_pfvconf_sum=_MAX_(_MAX_(_ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap0])), _ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap1]))), _MAX_(_ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap2])), _ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap3]))));
			u32_dh_pfvconf_diff=_MAX_(_MAX_(_ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap1-1])), _ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap1+1]))), _MAX_(_ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap2-1])), _ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap2+1]))));
			u32_dh_pfvconf_diff =_MAX_(u32_dh_pfvconf_diff, u32_dh_pfvconf_sum);
		}
		else if(nRecordMap0 < nRecordMap1)  // rotat
		{
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap0];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap1];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap2];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[nRecordMap3];

			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[(nRecordMap0+nRecordMap1)/2];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[(nRecordMap0+nRecordMap2)/2];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[(nRecordMap1+nRecordMap3)/2];
			u32_rmv_tc_sum +=s_pContext->_output_read_comreg.u22_me_rTC_rb[(nRecordMap2+nRecordMap3)/2];

			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap0];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap1];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap2];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[nRecordMap3];

			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[(nRecordMap0+nRecordMap1)/2];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[(nRecordMap0+nRecordMap2)/2];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[(nRecordMap1+nRecordMap3)/2];
			u32_rng_DTL_sum +=s_pContext->_output_read_comreg.u20_me_rDTL_rb[(nRecordMap2+nRecordMap3)/2];

			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap0];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap1];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap2];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap3];

			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[(nRecordMap0+nRecordMap1)/2];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[(nRecordMap0+nRecordMap2)/2];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[(nRecordMap1+nRecordMap3)/2];
			u32_dh_pfvconf_sum +=s_pContext->_output_read_comreg.u20_dh_pfvconf[(nRecordMap2+nRecordMap3)/2];

			u32_dh_pfvconf_avg= u32_dh_pfvconf_sum>>3;
			u32_dh_pfvconf_sum=_MAX_(_MAX_(_ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap0])), _ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap1]))), _MAX_(_ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap2])), _ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[nRecordMap3]))));
			u32_dh_pfvconf_diff=_MAX_(_MAX_(_ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[(nRecordMap0+nRecordMap1)/2])), _ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[(nRecordMap0+nRecordMap2)/2]))), _MAX_(_ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[(nRecordMap1+nRecordMap3)/2])), _ABS_((int)(u32_dh_pfvconf_avg - s_pContext->_output_read_comreg.u20_dh_pfvconf[(nRecordMap2+nRecordMap3)/2]))));
			u32_dh_pfvconf_diff =_MAX_(u32_dh_pfvconf_diff, u32_dh_pfvconf_sum);
		}
		u32_rmv_tc_avg= u32_rmv_tc_sum>>3;
		u32_rng_DTL_avg = u32_rng_DTL_sum>>3;
	}
	//record
	u17_avgZMV_cnt_pre = u17_avgZMV_cnt;
	//=================
	if(pOutput->u4_RgRotat_cnt >=rg_cnt_th &&( (u26_aAPLi_rb+ u26_aAPLp_rb)/2  > u26_aAPL_th) && (u26_aDTL_rb <u26_aDTL_th) && (u32_rng_DTL_avg >u20_rngDTL_th)&& (u32_rmv_cnt_avg>u32_rMV_cnt_th1) &&(u32_rmv_cnt_avg<u32_rMV_cnt_th2) &&(u17_avgZMV_cnt >u17_aZMV_cnt_th)&&(u32_rmv_tc_avg > u27_tc_th1)&&(u32_rmv_tc_avg < u27_tc_th2)&&(u32_dh_pfvconf_diff > u20_dh_pfvconf_diff_th1)&&(u32_dh_pfvconf_diff <u20_dh_pfvconf_diff_th2) )
	{
		u8_RgRotat_holdFrm += u8_RgRotat_holdFrmNum;
		u8_RgRotat_holdFrm=_CLIP_(u8_RgRotat_holdFrm, 0, 16);
		pOutput->u1_RgRotat_true = 1;
	}
	else if(u8_RgRotat_holdFrm >0)
	{
		u8_RgRotat_holdFrm--;
		pOutput->u1_RgRotat_true = 1;
	}
	else
	{
		u8_RgRotat_holdFrm = 0;
		pOutput->u1_RgRotat_true = 0;
	}
}
VOID FRC_VideoZoom_Detect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned char  nIdxX,nIdxY,nMap0,nMap1,nMap2,nMap3;
	unsigned char ZoomIn_cnt = 0,ZoomOut_cnt = 0;
	int nRMVx_1st[4]={0};
	int nRMVy_1st[4]={0};
	int nRMVx_Region[4]={0};
	int nRMVy_Region[4]={0};
	static unsigned char  u8_Zoom_holdFrm=0;

	pOutput->u1_Zoom_true  = 0;
	for (nIdxX=0; nIdxX<=3; nIdxX++)
	{
		for (nIdxY=0; nIdxY<=1; nIdxY++)
		{
			// init
			nMap0 = nIdxY*8+nIdxX;
			nMap1 = nIdxY*8+nIdxX+4;
			nMap2 = (nIdxY+2)*8+nIdxX;
			nMap3 = (nIdxY+2)*8+nIdxX+4;
		  	// left_top
			nRMVx_1st[0] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap0]>>1;
			nRMVy_1st[0] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap0]>>1;

			// right_top
			nRMVx_1st[1] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap1]>>1;
			nRMVy_1st[1] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap1]>>1;

			// left_bot
			nRMVx_1st[2] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap2]>>1;
			nRMVy_1st[2] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap2]>>1;

			// right_bot
			nRMVx_1st[3] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap3]>>1;
			nRMVy_1st[3] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap3]>>1;

			nRMVx_Region[0]+=nRMVx_1st[0];
			nRMVy_Region[0]+=nRMVy_1st[0];

			nRMVx_Region[1]+=nRMVx_1st[1];
			nRMVy_Region[1]+=nRMVy_1st[1];

			nRMVx_Region[2]+=nRMVx_1st[2];
			nRMVy_Region[2]+=nRMVy_1st[2];

			nRMVx_Region[3]+=nRMVx_1st[3];
			nRMVy_Region[3]+=nRMVy_1st[3];

			// left_top
			if((nRMVx_1st[0] <0) && (nRMVy_1st[0] <0))
			{
				ZoomIn_cnt++;
			}
			else if((nRMVx_1st[0] >0) && (nRMVy_1st[0] >0))
			{
				ZoomOut_cnt++;
			}

			// right_top
			if((nRMVx_1st[1] >0) && (nRMVy_1st[1] <0))
			{
				ZoomIn_cnt++;
			}
			else if((nRMVx_1st[1] <0) && (nRMVy_1st[1] >0))
			{
				ZoomOut_cnt++;
			}

			// left_bot
			if((nRMVx_1st[2] <0) && (nRMVy_1st[2] >0))
			{
				ZoomIn_cnt++;
			}
			else if((nRMVx_1st[2] >0) && (nRMVy_1st[2] <0))
			{
				ZoomOut_cnt++;
			}

			// right_bot
			if((nRMVx_1st[3] >0) && (nRMVy_1st[3] >0))
			{
				ZoomIn_cnt++;
			}
			else if((nRMVx_1st[3] <0) && (nRMVy_1st[3] <0))
			{
				ZoomOut_cnt++;
			}
		}
	}
	pOutput->u4_ZoomIn_cnt = ZoomIn_cnt;
	pOutput->u4_ZoomOut_cnt = ZoomOut_cnt;

	if((nRMVx_Region[0] <0) && (nRMVy_Region[0] <0) &&(nRMVx_Region[1] >0) && (nRMVy_Region[1] <0)&&(nRMVx_Region[2] <0) && (nRMVy_Region[2] >0)&&(nRMVx_Region[3] >0) && (nRMVy_Region[3] >0))
	{
		if(pOutput->u4_ZoomIn_cnt >=22)
		{
			u8_Zoom_holdFrm = 8;
			pOutput->u1_Zoom_true = 1;
		}
	}
	else if((nRMVx_Region[0] >0) && (nRMVy_Region[0] >0) &&(nRMVx_Region[1] <0) && (nRMVy_Region[1] >0)&&(nRMVx_Region[2] >0) && (nRMVy_Region[2] <0)&&(nRMVx_Region[3] <0) && (nRMVy_Region[3] <0))
	{
		if(pOutput->u4_ZoomOut_cnt >=22)
		{
			u8_Zoom_holdFrm = 8;
			pOutput->u1_Zoom_true = 1;
		}
	}
	else if(u8_Zoom_holdFrm >0)
	{
		u8_Zoom_holdFrm--;
		pOutput->u1_Zoom_true = 1;
	}
	else
	{
		u8_Zoom_holdFrm = 0;
		pOutput->u1_Zoom_true = 0;
	}
}

VOID updateGMVArray(int *array,int arrayCnt, int newData)     // gmv de shuzu
{
    signed short tmp=0;
    unsigned int i=0;
    for(i=0;i<arrayCnt;i++){
        if(i==arrayCnt-1)
        {
            tmp = newData;
        }
		else if(i<(arrayCnt-1))
		{
            tmp = array[i+1];
		}
        array[i] = tmp;
    }
}

VOID updateGMVArray2(int array[16][6],int arrayCnt, int newData,int j)
{
    signed short tmp=0;
    unsigned int i=0;
    for(i=0;i<arrayCnt;i++){
        if(i==arrayCnt-1)
            tmp = newData;
        else if(i<(arrayCnt-1))
            tmp = array[j][i+1];

        array[j][i] = tmp;
    }
}

VOID MEMC_me1_bgmask_newalgo_Proc(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
    const _PQLCONTEXT *s_pContext = GetPQLContext();
    signed short gmv_fgx,gmv_fgy,gmv_bgx,gmv_bgy,u11_gmv_mvx,u10_gmv_mvy ,u11_mv_max_rx ,u10_mv_max_ry,u11_mv_min_rx,u10_mv_min_ry;
    bool aplcond1 ,aplcond2,aplcond3,aplcond4 ,aplcond5 ,aplcntcond;
    signed short gmv_bgcnt,gmv_fgcnt;    

    signed short bg_apl_seg0_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg0_cnt;
    signed short bg_apl_seg1_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg1_cnt;
    signed short bg_apl_seg2_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg2_cnt;
    signed short bg_apl_seg3_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg3_cnt;
    signed short bg_apl_seg4_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg4_cnt;
    signed short bg_apl_seg5_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg5_cnt;
    signed short bg_apl_seg6_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg6_cnt;
    signed short bg_apl_seg7_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg7_cnt;
    signed short fg_apl_seg0_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg0_cnt;
    signed short fg_apl_seg1_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg1_cnt;
    signed short fg_apl_seg2_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg2_cnt;
    signed short fg_apl_seg3_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg3_cnt;
    signed short fg_apl_seg4_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg4_cnt;
    signed short fg_apl_seg5_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg5_cnt;
    signed short fg_apl_seg6_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg6_cnt;
    signed short fg_apl_seg7_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg7_cnt;

    static signed int gmv_fg_x_array[6] = {0}; 
    unsigned int fgx_length = 6;

    int fg_apl_sum=fg_apl_seg0_cnt+fg_apl_seg1_cnt+fg_apl_seg2_cnt+fg_apl_seg3_cnt+fg_apl_seg4_cnt+fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt;
    int bg_apl_sum=bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt+bg_apl_seg3_cnt+bg_apl_seg4_cnt+bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt;

    static int frame_cnt;
    int rgnIdx  ;
    unsigned  int bgcnt_array[32]={0};
    int u32_RB_val ;
    //int me1_white;
    int  bgrgncnt  ,row ,col ;
    int rgnmvy_bg[32]={0};
    int rgnmvx_bg[32]={0};
    int rgn_bgcnt[32]={0};
    int rgn_apli[32] = {0};
    int rgn_aplp[32] = {0};
    int rmvx[32] ={0}   , rmvy[32] ={0}  ;
    bool cond1,cond2,cond3;
    int rgn_bgcnt_num = 0;
    int Dtl_array[32]={0};
    int sum_apl =0;
    int logo_rgn_cnt[32]={0};
    int sum_logo_cnt=0,sum_logo_cnt_bottom=0;


    int  rimblkcnt  ,  highaplfgcnt ,  highaplbgcnt , lowaplcnt   ,aplsum32rgn , lowaplrgncnt,  lowaplrgncnt_for120hz  ;
    static int horiz_slowmotion_hold_cnt;
    int   i , aplcnt ,  bgcntleftsum  ,bgcntleftrgn ;  //for 192 
    int oppositeDirction , minmax_x_cond ,minmax_y_cond  ,oldcond;   //for 315     
    int bgseg0ratio   , fgseg0ratio   ,   seg0cntsum; //for 171
    int highaplcnt , aplsum8rgn ; 
    int fgbgdiff ,fggmvdiff ,rfgbgdiff ,rfggmvdiff , gmvbgdiff  ,  rmv_gmv_near_cnt ,   rmv_gmv_diff  ,cond1_163 ,cond2_163 ,condcnt ; //163 dog
    static int frame_cnt_013,hold_cnt_185;


    int logo_array[32]={0};
    int logo_sum=0;
    bool cond_013_1,cond_013_2,cond_185_1,cond_185_2,cond1_185_1,cond2_185_1,cond3_185_1,cond4_185_1;
    bool cond317_apl0 ,cond317_apl1,cond317_apl2,cond317_apl3,cond317_apl7 ,cond317_mv ,cond_123,ver_fast_cond,slant_mv_376;
    bool cond1_185,cond2_185,cond3_185,cond4_185,cond_013_rmv,cod304,cond129_1,cond129_2,cond129_3,cond129_4;
    int j ,fgratio0 ,fgratio01;
    int fgcnt_num_left=0,fgcnt_num_right=0;
    int ver_mv_fast_num=0,hor_mv_slow_num=0;
    int numy_40=0,numx_20=0;
    int gmv_value,rmv_value;
    static int hold_cnt_140;
    int mv_diff_large = 0,mv_diff_small = 0;
    int fgcnt_num = 0;
    static int bgcnt_pre_array[32];
    int cnt_bgcnt_num = 0 ;
    int aplsum32rgn_i;
    int aplsum32rgn_p;
    int rgn8_aplpsum;  
    bool cond140_1,cond140_2,cond140_3;
    int allbgcnt_length = 6;
    static int allbgcnt[6]={0};

    int small_rgnmvx,big_rgnmvy20,big_rgnmvy15;
    int fast_hori_rgnnum,fast_ver_rgnnum;
    int fast_hori_rgnnum2,fast_ver_rgnnum2;
    //static int holdframe355;
    //static int holdframe100;

    //static int gmvx_pre,gmvy_pre;
    //int zeromvxy_cnt;
    //static int holdframe_sc;
    bool cond_030_1,cond_030_2;
    static int bgcnt_pre24;
    static int gmv_fgy_pre,gmv_fgx_pre;

    int sim_mvx_num,sim_mvy_num,sim_mvx_num_mid,sim_mvy_num_mid;   
    int rgnmvx_range,rgnmvy_range,rgnmv_range_mid;
    int sum_bgcnt_lefthalf,sum_bgcnt_righthalf;
    int cond1841,cond1842,cond1843,cond384_1,cond384_2,cond384_3;
    int cond043_1,cond043_2,cond043_3,cond26_1,cond26_2,cond26_apl;
    int apl_changenum_3,apl_changenum_5,apl_changenum_7,apl_changenum_9,apl_changenum_outside;
    int bgcnt_lefthalf,bgcnt_righthalf;
    int topdtlsum = 0;
    int bottom_aplcnt = 0;

    int cond333_2,cond333_3,cond333_4,cond333_5;
    int cond333_rgnapl1,cond333_rgnapl2,cond333_rgnapl3;
    int cond128_mv,cond128_dtl;
    static int holdframe128;

    int bottom_rim_pre,arv_mv_y_bottom;
    static int rim_hold_cnt_bottom,rim_hold_cnt_bottom_logo,video_logo_bottom;
    int fgx_diff,fgy_diff;
    int scflag_num; 
    int scflag_cnt;
    int scflag_length = 10;
    static int scflag_arrayx[10]={0};
    static int scflag_arrayy[10]={0};
    int bgcnt_full_sum;
    int cond412_1,cond412_2,cond412_3;
    static int holdframe412;
    int cond35_1, cond35_2, cond35_3;
    static int holdframe374,video374;
    int rgn_aplcnt = 0,rgn_aplcnt2 = 0,rgn_aplcnt3 = 0;
    bool cond374_1,cond374_2,cond374_3,cond374_4,cond374_5,cond374_6,cond374_7,cond374_8,cond374_9,cond374_10,cond374_11,cond374_12,cond374_13,cond374_14,cond374_15,cond374_sp;
    int range_rmv_cnt_374;

    int rgn_big_num_1 = 0;
    int rgn_big_sum_1 = 0;
    int rgn_small_num_1 = 0;
    int rgn_small_sum_1 = 0;
    int rgn_big_num_2 = 0;
    int rgn_big_sum_2 = 0;
    int rgn_small_num_2 = 0;
    int rgn_small_sum_2 = 0;
    int rgn_mean_x_1 = 0;
    int rgn_mean_x_2 = 0;
    int rgn_mean_y_1 = 0;
    int rgn_mean_y_2 = 0;
    int region_fg_x ,region_fg_y ; 
        
    int dh_bypass, gmv_y0;

    int bigcnt_has_fg_region = 0;
    int smallcnt_has_fg_region = 0;

    bool dh_bad_fg_1, dh_bad_fg_2, dh_bad_fg_3, dh_bad_fg_4;
    int aplcnt1_183 ,aplcnt2_183 ;
    int hold_173=0;
    int fgmv_val ,bgmv_val ,aplcond ,aplsum ,mvth ;
    bool apl_cond_for432;
    int rmvx_num_for432 = 0, rmvx_num_for432_2 = 0, rmvy_num_for432 = 0, top_dtl_num_for432 = 0, bot_dtl_num_for432 = 0, apl_num_for432 = 0;
    int bgleftcnt1 ,bgrightcnt  ;


    int readval8,readval9,readval10,readval11,readval12,readval13,readval14,readval15,readval16;
    static int holdframe96,holdframe421,holdframe219,holdframe96_3,holdframe215,holdframe227_0,holdframe227_1,holdframe227_2,holdframe227_3,holdframe227_4;
    bool cond215_1,cond215_2,cond215_3,cond963_1,cond963_2,cond963_3;
    bool cond96_1,cond96_2,cond96_3,cond421_1,cond421_2,cond421_3,cond421_4,cond219_1,cond219_2,cond219_3;
    int bgcnt_sum_fisrtline ;
    bool cond227_1,cond227_2,cond227_11,cond227_12,cond227_13,cond227_21,cond227_22,cond227_23,cond227_31,cond227_32,cond227_33,cond227_41,cond227_42,cond227_43;
    int cond96_aplcnt1,cond96_aplcnt2,cond96_mvcnt;
    int cnt_dtlarray_96,cnt_aplarray_96;

    bool cond1562_mv,cond1562_dtl,cond1562_apl;
    bool cond049_1,cond049_2,cond049_3;
    bool cond163_1,cond163_2,cond163_3;
    int cond432final_1,cond432final_2,cond432final_3,con432_aplcnt = 0;
    bool cond38_15s1,cond38_15s2,cond38_15s3,cond38_45s1,cond38_45s2,cond38_45s3;
    static int holdframe38_15s,holdframe38_45s,video214;
    int cond38_aplcnt1=0,cond38_aplcnt2=0,cond38_aplcnt3=0,cond38_aplcnt4=0,cond38_aplcnt5=0,cond38_aplcnt6=0,cond38_aplcnt7=0;
    bool cond214_mv,cond214_dtl,cond214_apl;
    bool cond317face2_1,cond317face2_2,cond317face2_3;
    bool cond182_mv,cond182_dtl,cond182_apl;
    bool cond410SC_apl,cond393SC_apl;
    bool cond412_mv,cond412_dtl,cond412_apl,cond412_aplcnt;
    int path;
	int fwcontrol_339;    
    bool PQIssue_mv,PQIssue_dtl,PQIssue_apl;
	bool cond_car_mv,cond_car_dtl,cond_car_apl,cond_eagle_mv,cond_eagle_dtl,cond_eagle_apl;
	int dtlcnt1=0,dtlcnt2=0,dtlcnt3=0,dtlcnt_plane=0;
	int rmvyfastcnt = 0,dtlsmallcnt = 0,aplbigcnt = 0,aplbigcntplane = 0;
	static int holdframe_car,holdframe_eagle,eaglepattern,holdframe_C005,video_C005,holdframe_0002,video_0002,holdframe_smallplane,video_smallplane,holdframe_60016,video_60016,holdframe_60016_2,video_60016_2,holdframe_soccer,video_soccer,holdframe_rotterdam,video_rotterdam,video_rotterdam2,holdframe_rotterdam2;
	bool C005plane_1,C005plane_2,C005plane_3,blkoff_1,blkoff_2,blkoff_3;
	bool cond_smallplane1,cond_smallplane2,cond_smallplane3,cond_60016dance_1,cond_60016dance_2,cond_60016dance_3,cond_60016dance_21,cond_60016dance_22,cond_60016dance_23;
	int smallplane_mvcnt = 0,smallplane_aplcnt = 0;
    static int holdframePQ,videoPQ;
    int rgnmv_cnt = 0,right_rim_pre = 0;
    int als32;
    static int rim_hold_cnt_right;
    int path2 = 0;
    static int rightrim;
    int avr_mv_x,min_array1,min_array2,min_array,first_rim;
    static int rim_hold_cnt_right_02,rim_hold_cnt_right_034,video002,video034;
    int occl_mid_chaos1,occl_mid_chaos2,occl_mid_chaos3;
    //int occl_hori_chaos1,occl_hori_chaos2,occl_hori_chaos3;
//static int holdframe_occl_hori,holdframe_occl_mid;
	static int holdframe_occl_mid;
	static int Dtl_array0_pre,Dtl_array14_pre,Dtl_array22_pre,Dtl_array31_pre;
	//static int holdframe215215;
	static int glb_bgcnt[3]={0};
	int glb_bgcnt_length = 3;
	static int holdframe_equalbgcnt,video_fastmvx_equalbgcnt;
	bool cond_soccer1,cond_soccer2,cond_soccer3,cond_rotterdam1,cond_rotterdam2,cond_rotterdam3;
	int cond_soccer_aplcnt1 = 0,cond_soccer_aplcnt2 = 0;
	int cond_rotterdam_dtlcnt = 0,cond_rotterdam_aplcnt = 0,cond_skating_mvcnt = 0,cond_C001soccer_mvcnt = 0;
	bool skating_1,skating_2,skating_3,cond_C001soccer_1,cond_C001soccer_2,cond_C001soccer_3;
	static int holdframe_skating,video_skating,holdframe_C001soccer,video_C001soccer,holdframe_30006,video30006;  
	bool cond30006_mv,cond30006_dtl,cond30006_apl;	
	bool ridebike_mv,ridebike_dtl,ridebike_apl;
	static int ridebikegirl,holdframe_ridebikegirl;
	bool toleft_bike_mv,toleft_bike_dtl,toleft_bike_apl;
	static int holdmframe_toleft_bike,video_toleft_bike,holdframe_3003man,video_3003man;
	bool cond30003_mv,cond30003_dtl,cond30003_apl;
	bool cond0026_mv,cond0026_apl,cond0026_dtl;
	static int holdframe_0026_1,video_0026_1;
	int insect_mv,insect_apl,insect_dtl;
	static int holdframe_insect,video_insect;
	int readval0,readval31;
	int cond_huapi_mv,cond_huapi_dtl,cond_huapi_apl;
	static int holdframe_huapi,video_huapi;
	int insect_1,insect_2,insect_3,insect_dtl2,insect_apl2,insect_dtl3,insect_apl3;
	int transporter_1,transporter_2,blkoff_4,blkoff_5,blkoff_6;
	unsigned int show7segment;
	int IDT_cond_1,IDT_aplcond,IDT_dtlcond, IDT_cond_2,IDT_aplcond2,IDT_dtlcond2,right_rim_pre_IDT,left_rim_pre_IDT;
    static int holdframe_IDT,video_IDT;

	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 30, 30, &show7segment);
    
    ReadRegister(HARDWARE_HARDWARE_25_reg, 7,7 , &fwcontrol_339);
    
    
    gmv_fgx=s_pContext->_output_read_comreg.dh_gmv_fgx;
    gmv_fgy=s_pContext->_output_read_comreg.dh_gmv_fgy;
    gmv_bgx=s_pContext->_output_read_comreg.dh_gmv_bgx;
    gmv_bgy=s_pContext->_output_read_comreg.dh_gmv_bgy;

    u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
    u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
    region_fg_x=pOutput->region_fg_x;
    region_fg_y=pOutput->region_fg_y;

    gmv_bgcnt=s_pContext->_output_read_comreg.me1_gmv_bgcnt; 
    u11_mv_max_rx  = s_pContext->_output_read_comreg.s11_me_mv_max_vx_rb; 
    u10_mv_max_ry  = s_pContext->_output_read_comreg.s10_me_mv_max_vy_rb; 
    u11_mv_min_rx   = s_pContext->_output_read_comreg.s11_me_mv_min_vx_rb; 
    u10_mv_min_ry   = s_pContext->_output_read_comreg.s10_me_mv_min_vy_rb; 
    
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
    	rgnmvx_bg[rgnIdx]=s_pContext->_output_read_comreg.s11_bg_rMV_x_rb[rgnIdx];
        rgnmvy_bg[rgnIdx]=s_pContext->_output_read_comreg.s10_bg_rMV_y_rb[rgnIdx] ;

    }

    //32 rgn mv
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        
        rmvx[rgnIdx] =    s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[rgnIdx]  ;
        rmvy[rgnIdx] =    s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[rgnIdx]  ;        
    }

    //32 rgn bgcnt

    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN00_BG_reg+4*rgnIdx ,0,9,&u32_RB_val);   
        bgcnt_array[rgnIdx] = u32_RB_val ;
    }
    
    //32 dtl
    for(rgnIdx=0;rgnIdx<32;rgnIdx++)
    {
        ReadRegister(KME_ME1_TOP7_ME1_STATIS_DTL_01_reg+4*rgnIdx ,0,19,&u32_RB_val);  
        Dtl_array[rgnIdx]=u32_RB_val;    
    }
    
    //32 rgn apl   
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_00_reg+4*rgnIdx ,0,19,&u32_RB_val);   
        rgn_apli[rgnIdx]=u32_RB_val;
    } 
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_80_reg+4*rgnIdx ,0,19,&u32_RB_val);   
        rgn_aplp[rgnIdx]=u32_RB_val;
    }

    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        sum_apl += rgn_apli[rgnIdx];
    }
	
     //logo_rgn_cnt

    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_LOGO2_KME_LOGO2_80_reg+4*rgnIdx ,20,29,&u32_RB_val);   
        logo_rgn_cnt[rgnIdx]=u32_RB_val; 
    }    
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        sum_logo_cnt+=logo_rgn_cnt[rgnIdx];
    }

        ReadRegister(KME_DEHALO5_PHMV_FIX_35_reg,23,23, &dh_bypass);
        ReadRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,&gmv_y0);

    //--------------------------------dehalo bad fg condition ----------------------------------------------//
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        bigcnt_has_fg_region += (bgcnt_array[rgnIdx] < 990) ? 1 : 0;
        smallcnt_has_fg_region += (bgcnt_array[rgnIdx] < 900) ? 1 : 0;
    }
    dh_bad_fg_1 = ((_ABS_(gmv_bgx) < 8) && (_ABS_(gmv_bgy) < 8) && (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) < 10) && (((gmv_bgcnt > 30000) && (bigcnt_has_fg_region > 17)) || ((gmv_bgcnt > 25000) && (bigcnt_has_fg_region > 26)) || ((gmv_bgcnt < 13000) && (smallcnt_has_fg_region > 30))));
    dh_bad_fg_2 = (_ABS_(gmv_bgx) < 5) && (_ABS_(gmv_bgy) < 5) && ((_ABS_(gmv_fgx) > 14) || (_ABS_(gmv_fgy) > 14)) && (bigcnt_has_fg_region > 4) && (gmv_bgcnt > 31500) && (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) < 100) && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*1000/(bg_apl_sum)) > 250);
    dh_bad_fg_3 = (_ABS_(gmv_bgx) < 5) && (_ABS_(gmv_bgy) < 5) && (_ABS_(gmv_fgx) > 20) && (_ABS_(gmv_fgy) > 20) && (gmv_bgcnt > 28000) && (bigcnt_has_fg_region > 8) && (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) < 10) && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*1000/(bg_apl_sum)) > 300);
    dh_bad_fg_4 = (_ABS_(gmv_bgx) < 5) && (_ABS_(gmv_bgy) < 5) && ((_ABS_(gmv_fgx) > 12) || (_ABS_(gmv_fgy) > 12)) && (bigcnt_has_fg_region > 14) && (gmv_bgcnt > 27000)  && (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) < 10) && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*1000/(bg_apl_sum)) < 100);

    if(dh_bad_fg_1 || dh_bad_fg_2 || dh_bad_fg_3 || dh_bad_fg_4)
    {
        pOutput->u8_dh_condition_bad_fg = 1;
    }
    else
    {
        pOutput->u8_dh_condition_bad_fg = 0;
    }
        //rtd_pr_memc_info("by(%d) y0(%d) %d c(%d) h(%d) cnt(%d) (%d %d) bg(%d %d) fg(%d %d) apl(%d %d) sumapl(%d)", dh_bypass, gmv_y0, pOutput->u8_dh_condition_bad_fg, pOutput->u8_dh_condition_bypass_check, pOutput->u8_dh_condition_dhbypass_hold_cnt, gmv_bgcnt, bigcnt_has_fg_region, smallcnt_has_fg_region, gmv_bgx, gmv_bgy, gmv_fgx, gmv_fgy,
             //          ((bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*1000/(bg_apl_sum)), ((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)), sum_apl/*, bgcnt_array[16], bgcnt_array[17], bgcnt_array[18], bgcnt_array[19], bgcnt_array[20], bgcnt_array[21], bgcnt_array[22], bgcnt_array[23]*/);
 
        //--------------------------------------432 flag ---------------------------------------------//
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        rmvx_num_for432 += (rmvx[rgnIdx] < -110 && rmvx[rgnIdx] > -160) ? 1 : 0;
        rmvx_num_for432_2 += (rmvx[rgnIdx] < -90 && rmvx[rgnIdx] > -120) ? 1 : 0;
        rmvy_num_for432 += (rmvy[rgnIdx] < 20 && rmvy[rgnIdx] > -10) ? 1 : 0;
        top_dtl_num_for432 += (Dtl_array[rgnIdx] < 3000 && rgnIdx < 16) ? 1 : 0;
        bot_dtl_num_for432 += (Dtl_array[rgnIdx] > 3000 && rgnIdx >= 16) ? 1 : 0;
        apl_num_for432 += (rgn_aplp[rgnIdx] > 120000 && rgn_aplp[rgnIdx] < 190000) ? 1 : 0;
    }

    apl_cond_for432 = ((((rgn_aplp[18]+rgn_aplp[19])/2 < rgn_aplp[17]) && ((rgn_aplp[18]+rgn_aplp[19])/2 < rgn_aplp[20])) ||
                                    (((rgn_aplp[19]+rgn_aplp[20])/2 < rgn_aplp[18]) && ((rgn_aplp[19]+rgn_aplp[20])/2 < rgn_aplp[21])) ||
                                    (((rgn_aplp[20]+rgn_aplp[21])/2 < rgn_aplp[19]) && ((rgn_aplp[20]+rgn_aplp[21])/2 < rgn_aplp[22])) ||
                                    (((rgn_aplp[21]+rgn_aplp[22])/2 < rgn_aplp[20]) && ((rgn_aplp[21]+rgn_aplp[22])/2 < rgn_aplp[23])));

    pOutput->u8_me1_condition_432_flag = (apl_cond_for432 && (apl_num_for432 >= 12) && (rmvx_num_for432 >= 14) && (rmvy_num_for432 >= 28) && (top_dtl_num_for432 >= 7) && (bot_dtl_num_for432 >= 12)) ? 1 : 
                                                                     (apl_cond_for432 && (apl_num_for432 >= 12) && (rmvx_num_for432_2 >= 13) && (rmvy_num_for432 >= 32) && (top_dtl_num_for432 >= 12) && (bot_dtl_num_for432 >= 14)) ? 2 : 
                                                                     (apl_cond_for432 && (apl_num_for432 >= 9) && (rmvx_num_for432_2 >= 20) && (rmvy_num_for432 >= 20) && (top_dtl_num_for432 >= 3) && (bot_dtl_num_for432 >= 15)) ? 3 : 0   ;
        //rtd_pr_memc_info("flag(%d) numx(%d %d) y(%d) dtl(%d %d) apl(%d) %d \n", pOutput->u8_me1_condition_432_flag, rmvx_num_for432, rmvx_num_for432_2, rmvy_num_for432, top_dtl_num_for432, bot_dtl_num_for432, apl_cond_for432, apl_num_for432);   
     //--------------------------------200 face cond ----------------------------------------------//
    if((gmv_bgcnt > 28000) && (gmv_bgcnt < 30000) && (gmv_bgx > 60) && (gmv_bgx < 80) && (_ABS_(gmv_bgy) < 3) && (gmv_fgx > 50) && (gmv_fgx < 75) && (_ABS_(gmv_fgy) < 5) && 
            (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) < 100) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) > 750) && 
            (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 400) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) > 200) && (sum_apl > 4000000)) 
        {
            pOutput->u8_me1_condition_200_face_flag = 1;
        }
        else
        {
            pOutput->u8_me1_condition_200_face_flag = 0;
        }

        //--------------------------------366 cond ----------------------------------------------//
        if((gmv_bgcnt > 28000) && (gmv_bgcnt < 31000) && (gmv_bgx > -85) && (gmv_bgx < -70) && (_ABS_(gmv_bgy) < 3) && (gmv_fgx > -17) && (gmv_fgx < -7) && (gmv_fgy > -35) && (gmv_fgy < -15) && 
            (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) < 180) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) > 300) && 
            (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 300) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) > 250) && (sum_apl > 2500000) && (sum_apl < 3300000)) 
        {
            pOutput->u8_me1_condition_366_flag = 1;
        }
        else
        {
            pOutput->u8_me1_condition_366_flag = 0;
        }

        //--------------------------------173 begin small black fg----------------------------------------------//
        if((gmv_bgcnt > 28500) && (gmv_bgcnt < 32000) && (gmv_bgx > -25) && (gmv_bgx < -12) && (_ABS_(gmv_bgy) < 5) && (gmv_fgx > -30) && (gmv_fgx < -7) && (_ABS_(gmv_fgy) < 7) && 
            (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) > 250) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) < 250) && 
            (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 400) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) < 400) && (sum_apl > 2500000) && (sum_apl < 3300000)) 
        {
            pOutput->u8_me1_condition_173_begin_flag = 1;
            hold_173 = 10;
        }
        else if(hold_173 > 0)
        {
            pOutput->u8_me1_condition_173_begin_flag = 1;
            hold_173 = hold_173 - 1;
        }
        else
        {
            pOutput->u8_me1_condition_173_begin_flag = 0;
            hold_173 = 0;
        }
        //rtd_pr_memc_info("by(%d) y0(%d) (%d %d) (%d %d %d %d) %d c(%d) h(%d) cnt(%d) (%d %d) bg(%d %d) fg(%d %d) bgapl(%d %d) fgapl(%d %d) sumapl(%d)", dh_bypass, gmv_y0, pOutput->u8_dh_condition_small_mv_th_y0, pOutput->u8_dh_condition_slow_motion, 
          //              pOutput->u8_dh_condition_vertical_broken, pOutput->u8_dh_condition_346_bypass, pOutput->u8_dh_condition_184_bypass, pOutput->u8_dh_condition_110_bypass, pOutput->u8_me1_condition_173_begin_flag, pOutput->u8_dh_condition_bypass_check, pOutput->u8_dh_condition_dhbypass_hold_cnt, gmv_bgcnt, bigcnt_has_fg_region, smallcnt_has_fg_region, gmv_bgx, gmv_bgy, gmv_fgx, gmv_fgy,
          //            ((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)), ((bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)), ((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)), ((fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)), sum_apl);

        //--------------------------------130 wrong gmv flag----------------------------------------------//
        if((gmv_bgcnt > 20000) && (gmv_bgcnt < 23000) && (gmv_bgx > -60) && (gmv_bgx < -45) && (_ABS_(gmv_bgy) < 5) && (gmv_fgx > -25) && (gmv_fgx < -5) && (_ABS_(gmv_fgy) < 10) && 
            (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) > 380) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) < 100) && 
            (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 700) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) < 100) && (sum_apl > 1100000) && (sum_apl < 1600000)) 
        {
            pOutput->u8_me1_condition_130_flag = 2;
        }
        else
        {
            pOutput->u8_me1_condition_130_flag = 0;
        }
        //rtd_pr_memc_info("by(%d) y0(%d) (%d %d) %d c(%d) h(%d) cnt(%d) (%d %d) bg(%d %d) fg(%d %d) bgapl(%d %d) fgapl(%d %d) sumapl(%d)", dh_bypass, gmv_y0, pOutput->u8_dh_condition_small_mv_th_y0, pOutput->u8_dh_condition_slow_motion, pOutput->u8_me1_condition_130_flag, pOutput->u8_dh_condition_bypass_check, pOutput->u8_dh_condition_dhbypass_hold_cnt, gmv_bgcnt, bigcnt_has_fg_region, smallcnt_has_fg_region, gmv_bgx, gmv_bgy, gmv_fgx, gmv_fgy,
         //             ((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)), ((bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)), ((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)), ((fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)), sum_apl);
 
        //--------------------------------317 face2 ----------------------------------------------//
        //1011
        //off545
        //if((gmv_bgcnt > 28000) && (gmv_bgcnt < 30000) && (gmv_bgx > 75) && (gmv_bgx < 87) && (_ABS_(gmv_bgy) < 3) && (gmv_fgx > 5) && (gmv_fgx < 20) && (_ABS_(gmv_fgy) > 15) && (_ABS_(gmv_fgy) < 28) && (smallcnt_has_fg_region <= 3) &&
        //    (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) < 200) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) > 350) && 
        //
        cond317face2_1 = gmv_bgx>35 && gmv_bgx<100 && _ABS_(gmv_bgy)<3 && gmv_fgx>40 && _ABS_(gmv_fgy)<15;       
        cond317face2_2 = Dtl_array[0]<25000 && Dtl_array[1]<25000 && Dtl_array[31]>15000 && Dtl_array[31]<35000 && Dtl_array[19]<35000 && Dtl_array[22]<40000 && Dtl_array[22]>10000;
        cond317face2_3 = rgn_apli[0]>60000 && rgn_apli[0]<100000 && rgn_apli[2]>70000 && rgn_apli[2]<100000 && rgn_apli[24]>40000 && rgn_apli[24]<90000 && rgn_apli[23]>110000 && rgn_apli[14]>100000;
        if(cond317face2_1 && cond317face2_2 && cond317face2_3)
        {
            pOutput->u8_me1_condition_317_face_flag2 = 1;
            path = 7;
        }
        else if((gmv_bgcnt > 18000) && (gmv_bgcnt < 30000) && (gmv_bgx > 25) && (gmv_bgx < 65) && (_ABS_(gmv_bgy) < 3) && (gmv_fgx > 15) && (gmv_fgx < 60) && (_ABS_(gmv_fgy) > 5) && (_ABS_(gmv_fgy) < 20) && (smallcnt_has_fg_region <= 3) &&
            (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) < 200) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) > 350) && 
            (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 500) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) < 250) && (sum_apl > 2600000) && (sum_apl < 3300000)) 

        {
            pOutput->u8_me1_condition_317_face_flag2 = 1;
            path = 7;
        }
        else if((gmv_bgcnt > 27000) && (gmv_bgcnt < 31000) && (gmv_bgx > 32) && (gmv_bgx < 49) && (_ABS_(gmv_bgy) < 3) && (gmv_fgx > 200) && (gmv_fgx < 250) && 
            (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) > 250) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) > 400) && 
            (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) < 400) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) < 600) && (sum_apl > 2600000) && (sum_apl < 3500000)) 
        {
            pOutput->u8_me1_condition_317_face_flag2 = 1;
            path = 7;
        }
        else
        {
            pOutput->u8_me1_condition_317_face_flag2 = 0;
            path = 0;
        }
        //rtd_pr_memc_info("%d %d %d\n",cond317face2_1 , cond317face2_2 , cond317face2_3);
        //rtd_pr_memc_info("mv:%d %d %d %d dtl:%d %d %d %d %d\n",gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy,Dtl_array[0],Dtl_array[1],Dtl_array[31],Dtl_array[19],Dtl_array[22]);
        //rtd_pr_memc_info("apl:%d %d %d %d %d\n",rgn_apli[0],rgn_apli[2],rgn_apli[24],rgn_apli[23],rgn_apli[14]);

        

        //--------------------------------317 face3 ----------------------------------------------//
        //off545
        //if((gmv_bgcnt > 28000) && (gmv_bgcnt < 30000) && (gmv_bgx > 110) && (gmv_bgx < 130) && (_ABS_(gmv_bgy) < 5) && (_ABS_(gmv_fgx) < 15) && (_ABS_(gmv_fgy) < 15) && 
        //    (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) > 320) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) > 220) && 
        //    (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 240) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) > 300) && (sum_apl > 2200000) && (sum_apl < 2700000)) 

        if((gmv_bgcnt > 22000) && (gmv_bgcnt < 30000) && (gmv_bgx > 25) && (gmv_bgx < 130) && (_ABS_(gmv_bgy) < 5) && (_ABS_(gmv_fgx) < 45) && (_ABS_(gmv_fgy) < 20) && 
            (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) >220) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) > 300) && 
            (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 400) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) > 200) && (sum_apl > 2200000) && (sum_apl < 2700000)) 
        {
            pOutput->u8_me1_condition_317_face_flag3 = 1;
        }
        else
        {
            pOutput->u8_me1_condition_317_face_flag3 = 0;
        }
        //rtd_pr_memc_info("flag:%d %d\n",pOutput->u8_me1_condition_317_face_flag2,pOutput->u8_me1_condition_317_face_flag3);
        //rtd_pr_memc_info("317face2:%d %d %d %d %d %d %d %d %d %d %d\n",gmv_bgcnt,gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy,((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)),
        //((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)),((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)),((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)),
        //sum_apl,smallcnt_has_fg_region);
     //--------------------------------419 ----------------------------------------------//
        if((gmv_bgcnt > 26000) && (gmv_bgcnt < 31000) && (gmv_bgx > -240) && (gmv_bgx < -210) && (gmv_bgy > 10)  && (gmv_bgy < 30) && (_ABS_(gmv_fgx) < 15) && (_ABS_(gmv_fgy) < 13) && 
            (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) < 150) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) > 450) && 
            (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 600) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) < 220) && (sum_apl > 4000000) && (sum_apl < 5500000)) 
        {
            pOutput->u8_me1_condition_419_flag = 1;
        }
        else
        {
            pOutput->u8_me1_condition_419_flag = 0;
        }
        
	//--------------------------------mixed mv cond ----------------------------------------------//
	for(rgnIdx=8;rgnIdx<15;rgnIdx++)
	{
		rgn_big_num_1 += (rmvx[rgnIdx] < rmvx[rgnIdx+1]) ? 1 : 0;
		rgn_big_sum_1 += (rmvx[rgnIdx] < rmvx[rgnIdx+1]) ? _ABS_DIFF_(rmvx[rgnIdx], rmvx[rgnIdx+1]) : 0;
		rgn_small_num_1 += (rmvx[rgnIdx] > rmvx[rgnIdx+1]) ? 1 : 0;
		rgn_small_sum_1 += (rmvx[rgnIdx] > rmvx[rgnIdx+1]) ? _ABS_DIFF_(rmvx[rgnIdx], rmvx[rgnIdx+1]) : 0;
	}
	for(rgnIdx=16;rgnIdx<23;rgnIdx++)
	{
		rgn_big_num_2 += (rmvx[rgnIdx] < rmvx[rgnIdx+1]) ? 1 : 0;
		rgn_big_sum_2 += (rmvx[rgnIdx] < rmvx[rgnIdx+1]) ? _ABS_DIFF_(rmvx[rgnIdx], rmvx[rgnIdx+1]) : 0;
		rgn_small_num_2 += (rmvx[rgnIdx] > rmvx[rgnIdx+1]) ? 1 : 0;
		rgn_small_sum_2 += (rmvx[rgnIdx] > rmvx[rgnIdx+1]) ? _ABS_DIFF_(rmvx[rgnIdx], rmvx[rgnIdx+1]) : 0;
	}
	rgn_big_sum_1 = (rgn_big_num_1 == 0) ? 0 : rgn_big_sum_1/rgn_big_num_1 ;
    rgn_small_sum_1 = (rgn_small_num_1 == 0) ? 0 : rgn_small_sum_1/rgn_small_num_1 ;
    rgn_big_sum_2 = (rgn_big_num_2 == 0) ? 0 : rgn_big_sum_2/rgn_big_num_2 ;
    rgn_small_sum_2 = (rgn_small_num_2 == 0) ? 0 : rgn_small_sum_2/rgn_small_num_2 ;
    rgn_mean_x_1 = (_ABS_(rmvx[8]) + _ABS_(rmvx[9]) + _ABS_(rmvx[10]) + _ABS_(rmvx[11]) + _ABS_(rmvx[12]) + _ABS_(rmvx[13]) + _ABS_(rmvx[14]) + _ABS_(rmvx[15]))/8;
    rgn_mean_x_2 = (_ABS_(rmvx[16]) + _ABS_(rmvx[17]) + _ABS_(rmvx[18]) + _ABS_(rmvx[19]) + _ABS_(rmvx[20]) + _ABS_(rmvx[21]) + _ABS_(rmvx[22]) + _ABS_(rmvx[23]))/8;
    rgn_mean_y_1 = (_ABS_(rmvy[8]) + _ABS_(rmvy[9]) + _ABS_(rmvy[10]) + _ABS_(rmvy[11]) + _ABS_(rmvy[12]) + _ABS_(rmvy[13]) + _ABS_(rmvy[14]) + _ABS_(rmvy[15]))/8;
    rgn_mean_y_2 = (_ABS_(rmvy[16]) + _ABS_(rmvy[17]) + _ABS_(rmvy[18]) + _ABS_(rmvy[19]) + _ABS_(rmvy[20]) + _ABS_(rmvy[21]) + _ABS_(rmvy[22]) + _ABS_(rmvy[23]))/8;

    if(((rgn_big_num_1 >= 6) && (rgn_big_sum_1 >= 12) && (rgn_big_sum_1 >= rgn_mean_x_1/3) && (rgn_big_num_2 >= 5) && (rgn_big_sum_2 >= 12) && (rgn_big_sum_2 >= rgn_mean_x_2/3) && (rgn_mean_y_2 - rgn_mean_y_1 > 20)) || 
           ((rgn_small_num_1 >= 6) && (rgn_small_sum_1 >= 12) && (rgn_small_sum_1 >= rgn_mean_x_1/3) && (rgn_small_num_2 >= 5) && (rgn_small_sum_2 >= 12) && (rgn_small_sum_2 >= rgn_mean_x_2/3) && (rgn_mean_y_2 - rgn_mean_y_1 > 20) ))
    {
        pOutput->u8_dh_condition_mixed_mv = 1;
    }
    else
    {
        pOutput->u8_dh_condition_mixed_mv = 0;
    }
        //rtd_pr_memc_info("cond(%d)  big1(%d %d) small1(%d %d) mean(%d %d) big2(%d %d) small2(%d %d) mean(%d %d) ",  pOutput->u8_dh_condition_mixed_mv , rgn_big_num_1, rgn_big_sum_1, rgn_small_num_1, rgn_small_sum_1, rgn_mean_x_1, rgn_mean_y_1, rgn_big_num_2, rgn_big_sum_2, rgn_small_num_2, rgn_small_sum_2, rgn_mean_x_2, rgn_mean_y_2);
        //---------------------------------------------------------------------------------------//
    if((u11_gmv_mvx!=0)||(u10_gmv_mvy!=0))
    {
        frame_cnt++;
    } 

        for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN00_BG_reg+4*rgnIdx ,0,9,&u32_RB_val);   
        rgn_bgcnt[rgnIdx] = u32_RB_val ;
    }

    updateGMVArray(gmv_fg_x_array,fgx_length,gmv_fgx);
    bgrgncnt = 0 ;
    for(row =0 ; row<4; row++)
    {    
        for(col =0 ; col<3; col++)
        {
           if(bgcnt_array[col+row*8]>=980)
               bgrgncnt++ ;
        }
    }

    if((_ABS_DIFF_(gmv_bgx,gmv_fgx)>=65)&&(gmv_bgcnt<=27000)&&(bgrgncnt>9))
    {
        pOutput->u8_me1_condition_large_fg_cnt=1;
    } 
    else
    {
        pOutput->u8_me1_condition_large_fg_cnt=0;
    }	

    if(frame_cnt>=6)
    {
        if((_ABS_DIFF_(gmv_fgx,gmv_fg_x_array[0])>=14)&&(gmv_bgcnt>=30000))
        {       
            pOutput->u8_me1_condition_small_fg_cnt=1;
        } 
        else
        {       
            pOutput->u8_me1_condition_small_fg_cnt=0;
        } 
    }

    if(_ABS_(gmv_bgx)>250)
    {  
        pOutput->u8_me1_condition_large_fg_mvx=1;
    } 
    else
    {  
        pOutput->u8_me1_condition_large_fg_mvx=0;
    } 
    /*
    if(fg_apl_sum>0)   //317
    {
        if(((fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/fg_apl_sum)>=400)
        {
            pOutput->u8_me1_condition_s2_apl_protect_en=1;
        }
        else
        {
            pOutput->u8_me1_condition_s2_apl_protect_en=0;
        }
    }
    else
    {
        pOutput->u8_me1_condition_s2_apl_protect_en=0;
    }

   */
    if((fg_apl_sum>0)&&(bg_apl_sum>0))  //299
    {
        aplcond1 =((fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum))>400 ; 
        aplcond2 = (((bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/bg_apl_sum)<=250) ; 
        aplcond3 = (((fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum))>180) ;
        aplcond4 = (((bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/bg_apl_sum)<=250) ;
        aplcond5 = ((fg_apl_seg0_cnt*1000/fg_apl_seg7_cnt )<  600) ;
        aplcntcond = (fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)<17000 ;

        if(((aplcond1&&aplcond2)||((gmv_bgcnt<17000)&&(aplcond3&&aplcond4)))&&aplcond5)
        {
            pOutput->u8_me1_conditions2_s3_apl_protect_en=1;
        }
        else
        {
            pOutput->u8_me1_conditions2_s3_apl_protect_en=0;
        }
    }
    else
    {
        pOutput->u8_me1_conditions2_s3_apl_protect_en=0;
    }

    if((gmv_bgcnt>29800)&&(_ABS_(gmv_fgy) >12))   //for vertical 181 wheel
    {
        pOutput->u8_me1_condition_virtical=1;
    }
    else
    {
        pOutput->u8_me1_condition_virtical=0;
    }

    // static int bgcnt_hold_val;

    if((gmv_bgcnt<10800))   //few bgcnt
    {
        pOutput->u8_me1_condition_fewbgcnt=1;
    }
    else
    {
        pOutput->u8_me1_condition_fewbgcnt=0;
    }


    if(fg_apl_seg0_cnt>5000)
    {
        pOutput->u8_me1_condition_lowfgapl=1;
    }
    else
    {
        pOutput->u8_me1_condition_lowfgapl=0;
    }
    
    
    for(rgnIdx =16 ; rgnIdx< 24; rgnIdx++)
    {
       if(rgn_bgcnt[rgnIdx]<510)
        rgn_bgcnt_num++;   
    }


    if((fg_apl_sum>0)&&(bg_apl_sum>0))  //339
    {  
        cond1=(fg_apl_seg0_cnt*100/fg_apl_sum)>45;
        cond2=(rgn_bgcnt_num>=5);
        cond3=(bg_apl_seg0_cnt*100/bg_apl_sum)<10;
        if(cond1 && cond2 && cond3)
        {
            pOutput->u8_me1_condition_blackth=1;
        }
        else
        {
            pOutput->u8_me1_condition_blackth=0;
        }
    }
    else
    {
        pOutput->u8_me1_condition_blackth=0;
    }
    //++++++++++++++192 horn+++++++++++++//
    aplcnt = 0 ;
    bgcntleftsum =0 ;
    for(i =0 ; i< 4; i++)
    {      
        if(rgn_apli[12+i]>160000)
        aplcnt ++ ;
        
        if(rgn_apli[20+i]>160000)
        aplcnt ++ ;  
    }

    for(col=0 ; col< 4; col++)
    {      
        bgcntleftrgn = (bgcnt_array[5+col*8]+ bgcnt_array[6+col*8]+ bgcnt_array[7+col*8] );
        bgcntleftsum+=bgcntleftrgn ; 
    }
    if(aplcnt>=5&&bgcntleftsum>12140&&gmv_bgcnt>27000&&_ABS_(u10_gmv_mvy)<=5&&_ABS_(u11_gmv_mvx)<=50&&_ABS_(u11_gmv_mvx)>=20&&bg_apl_seg0_cnt<2000&&fg_apl_seg0_cnt<800) 
    {       
        pOutput->u8_me1_condition_horn_skin_off=1;
    }
    else
    {
        pOutput->u8_me1_condition_horn_skin_off=0;
    }
   //+++++++++++++++315 horse vertical moving case++++++++++++++//
    oppositeDirction = (((u11_mv_max_rx>>31)&0x1)!=((u11_mv_min_rx>>31)&0x1))&&(((u10_mv_max_ry>>31)&0x1)!=((u10_mv_min_ry>>31)&0x1)) ; 
   
    minmax_x_cond = (_ABS_DIFF_(u11_mv_max_rx , u11_mv_min_rx)> 40)&&(_ABS_DIFF_(u11_mv_max_rx , u11_mv_min_rx)< 60) ; 
    minmax_y_cond = (_ABS_DIFF_(u10_mv_max_ry , u10_mv_min_ry)>20)&& (_ABS_DIFF_(u10_mv_max_ry , u10_mv_min_ry)<70) ;
    oldcond = oppositeDirction&&minmax_x_cond&&minmax_y_cond&&_ABS_(u10_gmv_mvy)>=5&&_ABS_(u11_gmv_mvx)<=30 ;
    if(oldcond&&gmv_bgcnt>30000&&fg_apl_seg0_cnt<480&&fg_apl_seg7_cnt<120&&(fg_apl_seg0_cnt+bg_apl_seg0_cnt)<1100&&(fg_apl_seg7_cnt+bg_apl_seg7_cnt)<1100)
    {
        pOutput->u8_me1_condition_ver_case1  =1 ; 
    }
    else
    {
        pOutput->u8_me1_condition_ver_case1  =0 ; 
    }

    //---------------------315 horse vertical moving case------------------// 
  
    ////////////////////+++++++++301+++++++++-/////////////
    if(fg_apl_sum>0)
    {
        if((_ABS_(gmv_bgx)<=2) && (_ABS_(gmv_bgy)==0) && (_ABS_(gmv_fgx)<=50) && (_ABS_(gmv_fgy)<=10) && ((fg_apl_seg0_cnt*100/fg_apl_sum)>15) && gmv_bgcnt>31500)
        {  
            pOutput->u8_me1_condition_blackoff=1;
            //WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x0);  //me1 black protect off 
            //WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x0);     
            horiz_slowmotion_hold_cnt=15;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x1);
        }
        else if(horiz_slowmotion_hold_cnt>0)
        {
            pOutput->u8_me1_condition_blackoff=1;
            //WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x0);
            //WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x0);
            horiz_slowmotion_hold_cnt--;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x1);
        }
        else
        {
            pOutput->u8_me1_condition_blackoff=0;
            //WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1);
            //WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x1);
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x0);
        }
    }
    else
    {
        pOutput->u8_me1_condition_blackoff=0;
        //WriteRegister(KME_ME1_BG0_ME1_BG_3_reg, 0, 0, 0x1);
        //WriteRegister(KME_ME1_BG0_ME1_BG_11_reg, 0, 0, 0x1);
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x0);
    }
    
    //++++++++add the black fg/bg and need to close the black apl protect   for example  #226 

     aplsum32rgn = 0;
     lowaplrgncnt = 0 ;
     lowaplrgncnt_for120hz = 0;
     for(rgnIdx=0 ; rgnIdx<32 ;rgnIdx++)
     {    
        aplsum32rgn+=rgn_aplp[rgnIdx] ; 
        if(rgn_aplp[rgnIdx]< 25000)
            lowaplrgncnt++ ;
        if(rgn_aplp[rgnIdx]< 60000)
            lowaplrgncnt_for120hz++ ;
     }
    rimblkcnt =  32400 - fg_apl_sum - bg_apl_sum  ;
    highaplfgcnt = fg_apl_seg5_cnt +fg_apl_seg6_cnt+fg_apl_seg7_cnt ; 
    highaplbgcnt =  bg_apl_seg5_cnt +bg_apl_seg6_cnt+bg_apl_seg7_cnt ; 
    lowaplcnt = fg_apl_seg0_cnt + fg_apl_seg1_cnt + bg_apl_seg0_cnt + bg_apl_seg1_cnt  ; 

    if(((lowaplcnt+rimblkcnt)>27000)&&(highaplfgcnt<100)&&(fg_apl_seg7_cnt<10) && (u11_gmv_mvx>=25) && (u11_gmv_mvx<=90)&&(_ABS_(u10_gmv_mvy)<4) &&(((lowaplrgncnt>=25) && (aplsum32rgn<900000)) || ((lowaplrgncnt_for120hz>=28) && (aplsum32rgn<1650000))))
    {
        pOutput->u8_me1_condition_black_apl=1;
    }
    else
    {
        pOutput->u8_me1_condition_black_apl=0;
    }

     //++++++++add the black fg/bg and need to close the black apl protect   for example  #171
     
    bgseg0ratio = bg_apl_seg0_cnt*100/bg_apl_sum ; 
    fgseg0ratio = fg_apl_seg0_cnt*100/fg_apl_sum ;    
    seg0cntsum =  bg_apl_seg0_cnt+fg_apl_seg0_cnt+32400- bg_apl_sum - fg_apl_sum ; 
   
    
    if((bg_apl_seg0_cnt>16000||seg0cntsum>22000)&&bgseg0ratio>50&&(fgseg0ratio<40||(fgseg0ratio>=95))&&gmv_bgcnt>25000&&(_ABS_(gmv_fgy)<3)&&(_ABS_(u10_gmv_mvy)<3)&&(_ABS_(u11_gmv_mvx)<=90)&&(_ABS_(u11_gmv_mvx)>=40))
    {

        pOutput->u8_me1_condition_black_off_case1 = 1 ;
    }
    else
    {
        pOutput->u8_me1_condition_black_off_case1 = 0 ;
    }

    if(seg0cntsum>20000&&gmv_bgcnt<16200&&fgseg0ratio>=86&&(_ABS_(u10_gmv_mvy)<3)&&(_ABS_(u11_gmv_mvx)<=80)&&fg_apl_seg0_cnt>14800)
    {

        pOutput->u8_me1_condition_black_off_case2= 1 ;
    }
    else
    {
        pOutput->u8_me1_condition_black_off_case2= 0 ;
    }

     //++++++++add vertical moving case 2   for example  #306+++++++//

     aplsum8rgn = 0;
     highaplcnt = 0 ;
     for(rgnIdx=0 ; rgnIdx<8 ;rgnIdx++)
     {    
        aplsum8rgn+=rgn_aplp[rgnIdx] ; 
        if(rgn_aplp[rgnIdx]> 195000)
            highaplcnt++ ;
     }


  //  if((((bg_apl_seg7_cnt+bg_apl_seg6_cnt)>12000&&gmv_bgcnt>26000)||(gmv_bgcnt<18000))&&(_ABS_(u10_gmv_mvy)>=4)&&(_ABS_(u11_gmv_mvx)<=60)&&(_ABS_(u11_gmv_mvx)>=20)&&highaplcnt>=7&&aplsum8rgn>=1600000 && Dtl_array[23]<35000)
    if((((bg_apl_seg7_cnt+bg_apl_seg6_cnt)>12000&&gmv_bgcnt>26000)||(gmv_bgcnt<18000))&&(_ABS_(u10_gmv_mvy)>=4)&&(_ABS_(u11_gmv_mvx)<=60)&&(_ABS_(u11_gmv_mvx)>30)&&highaplcnt>=7&&aplsum8rgn>=1600000 && Dtl_array[23]<35000)
	{
        pOutput->u8_me1_condition_virtical_case2 = 1 ;
    }
    else
    {
        pOutput->u8_me1_condition_virtical_case2 = 0 ;
    }

   //+++++++++++++++++++163 dog  vertical  moving fast+++++++//

  
    fgbgdiff =   _ABS_DIFF_(gmv_fgy , gmv_bgy) ;  
    fggmvdiff =   _ABS_DIFF_(gmv_fgy , u10_gmv_mvy)  + _ABS_DIFF_(gmv_fgx , u11_gmv_mvx);  
    gmvbgdiff =   _ABS_DIFF_(u10_gmv_mvy , gmv_bgy) ;  

    rfgbgdiff =   _ABS_DIFF_(region_fg_y , gmv_bgy) ;  
    rfggmvdiff =   _ABS_DIFF_(region_fg_y , u10_gmv_mvy);  
    gmvbgdiff =   _ABS_DIFF_(u10_gmv_mvy , gmv_bgy) ;  
    if(gmv_bgcnt>=32398||gmv_bgcnt<=2)
    {
        fgbgdiff =0 ; 
        fggmvdiff =0 ;
        gmvbgdiff =0 ;
    }
    rmv_gmv_near_cnt =0 ;
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        rmv_gmv_diff = (_ABS_DIFF_(rmvx[rgnIdx]  ,u11_gmv_mvx ) > _ABS_DIFF_(rmvy[rgnIdx]  ,u10_gmv_mvy ))?  _ABS_DIFF_(rmvx[rgnIdx]  ,u11_gmv_mvx ) : _ABS_DIFF_(rmvy[rgnIdx]  ,u10_gmv_mvy )  ; 
        if(rmv_gmv_diff<38)
             rmv_gmv_near_cnt ++ ;        
    }
    cond1_163 = (u10_gmv_mvy<-90)&&(u11_gmv_mvx<=35&&u11_gmv_mvx>=15)&&(_ABS_(gmv_fgx)<=30)&&(gmv_bgy<-95)&&gmv_bgcnt<16000 ;
    cond2_163 = (fggmvdiff<25||rfggmvdiff<25)&& (u10_gmv_mvy<-85)&&(_ABS_(u11_gmv_mvx)<=40)&&gmv_bgcnt<5000 ;

    condcnt = rmv_gmv_near_cnt>=30&&((fg_apl_seg0_cnt+bg_apl_seg0_cnt)<=200) ;
     
    if((cond1_163||cond2_163)&&condcnt)
    {
        pOutput->u8_me1_condition_virtical_case3 = 1 ;
    }
    else
    {
        pOutput->u8_me1_condition_virtical_case3 = 0 ;
    }
    
    for(rgnIdx=0;rgnIdx<32;rgnIdx++)
    {
        ReadRegister(KME_LOGO2_KME_LOGO2_80_reg+4*rgnIdx ,20,29,&u32_RB_val);  
        logo_array[rgnIdx]=u32_RB_val;    
    }
    
    for(rgnIdx=0;rgnIdx<32;rgnIdx++)
    {
        logo_sum+=logo_array[rgnIdx];
    }

    cond_013_1=(gmv_bgcnt>30500 &&  gmv_bgx==0 && _ABS_(gmv_bgy)<=1 && 
        ((Dtl_array[0]>8000 && Dtl_array[0]<12000)||(Dtl_array[0]<1500)) && ((Dtl_array[7]>8000 && Dtl_array[7]<12000)||( Dtl_array[7]<1500) )&& Dtl_array[15]<80 && Dtl_array[23]<3500
        && rgn_apli[1]>20000 && rgn_apli[1]<35000 &&  rgn_apli[7]>2000);  //zidoo 60hz
    
    cond_013_rmv = ((rmvx[0]<=1 && rmvy[0]<=1)&& (rmvx[1]<=1 && rmvy[1]<=1)&& (rmvx[2]<=5 && rmvy[2]<=5)&& (rmvx[3]<=5 && rmvy[3]<=5)&& (rmvx[5]<=1 && rmvy[5]<=1)&& (rmvx[6]<=1 && rmvy[6]<=1)&& (rmvx[7]<=1 && rmvy[7]<=1)); 
    cond_013_2=(gmv_bgcnt>27000 &&  gmv_bgx==0 && _ABS_(gmv_bgy)<=1 && 
        ((Dtl_array[0]>8000 && Dtl_array[0]<12000)||(Dtl_array[0]<1500)) && ((Dtl_array[7]>9000 && Dtl_array[7]<16000)||( Dtl_array[7]<1500) )&& (Dtl_array[15]<7000 && Dtl_array[15]>1000)&& (Dtl_array[23]<10000 && Dtl_array[23]>2000)
        && rgn_apli[1]>30000 && rgn_apli[1]<50000 &&  (rgn_apli[7]>20000 &&rgn_apli[7]<40000) && cond_013_rmv); //LG 120hz


    if(cond_013_1 || cond_013_2)
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0xa);
        frame_cnt_013=20;
        pOutput->u8_me1_condition_13m2v_flag =1;
        WriteRegister(HARDWARE_HARDWARE_25_reg, 11,11 , 1); 
        
    }
    else if(frame_cnt_013>0)
    {
        frame_cnt_013--;
         //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0xa);
         pOutput->u8_me1_condition_13m2v_flag =1;
         WriteRegister(HARDWARE_HARDWARE_25_reg, 11,11 , 1); 
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x1);
        pOutput->u8_me1_condition_13m2v_flag =0;
        WriteRegister(HARDWARE_HARDWARE_25_reg, 11,11 , 0); 

    }


    ///////////////////-------------------------185------------------------///////////////
    //1011
      //60Hz zidoo condition
    cond1_185_1 =  (Dtl_array[0]<65000 && Dtl_array[0]>15000 && Dtl_array[2]<60000 && Dtl_array[2]>30000 && Dtl_array[3]<65000 && Dtl_array[3]>30000 && Dtl_array[4]<70000 && Dtl_array[4]>40000);
    cond2_185_1 =  Dtl_array[5]<80000 && Dtl_array[5]>40000 && Dtl_array[6]<70000 && Dtl_array[6]>40000 && Dtl_array[7]<70000 && Dtl_array[7]>35000 && Dtl_array[14]<120000 && Dtl_array[14]>70000;
    cond3_185_1 =  Dtl_array[25]<50000 && Dtl_array[25]>15000 && Dtl_array[27]<55000 && Dtl_array[27]>15000 && Dtl_array[29]<60000 && Dtl_array[29]>25000 && Dtl_array[30]<65000 && Dtl_array[30]>25000&& Dtl_array[31]<55000 && Dtl_array[31]>15000;
    cond4_185_1 =  gmv_bgcnt < 32400 && gmv_bgcnt>24000 && gmv_bgx<=0 && (gmv_bgx+30)>0 && _ABS_(gmv_bgy)<=3 && rgn_apli[0]<100000;
    

    cond1_185 =  (Dtl_array[0]<100000 && Dtl_array[0]>30000 && Dtl_array[2]<100000 && Dtl_array[2]>30000 && Dtl_array[3]<100000 && Dtl_array[3]>40000 && Dtl_array[4]<110000 && Dtl_array[4]>50000);
    cond2_185 =  Dtl_array[5]<120000 && Dtl_array[5]>50000 && Dtl_array[7]<120000 && Dtl_array[7]>40000 && Dtl_array[14]<120000 && Dtl_array[14]>70000;
    cond3_185 =  Dtl_array[25]<70000 && Dtl_array[25]>10000 && Dtl_array[27]<80000 && Dtl_array[27]>10000 && Dtl_array[29]<110000 && Dtl_array[29]>30000 && Dtl_array[31]<80000 && Dtl_array[31]>30000;
    cond4_185 =  gmv_bgcnt < 32400 && gmv_bgcnt>23000 && gmv_bgx<=0 && (gmv_bgx+30)>0 && _ABS_(gmv_bgy)<=3 && rgn_apli[0]<150000;

    cond_185_1 = cond1_185_1 && cond2_185_1 && cond3_185_1 && cond4_185_1;
    cond_185_2 = cond1_185 && cond2_185 && cond3_185 && cond4_185;

    //off545
    //if((cond_185_1 || cond_185_2) && gmv_fgx>10 && aplsum32rgn>2500000 && aplsum32rgn<3300000 && rgn_apli[5]>110000)

    if((cond_185_1 || cond_185_2) && gmv_fgx>5 && aplsum32rgn>2500000 && aplsum32rgn<3300000 && rgn_apli[5]>100000)
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x1);
        hold_cnt_185=70;
        WriteRegister(HARDWARE_HARDWARE_25_reg, 10,10 , 1); 
    }
    else if(hold_cnt_185>0)
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x1);
        hold_cnt_185--;
        WriteRegister(HARDWARE_HARDWARE_25_reg, 10,10 , 1); 
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x0);
        WriteRegister(HARDWARE_HARDWARE_25_reg, 10,10 , 0); 
    }

    //rtd_pr_memc_info("185:%d %d %d %d %d\n",cond_185_1,cond_185_2, gmv_fgx , aplsum32rgn,rgn_apli[5]);
    //rtd_pr_memc_info("1851:%d %d %d %d  1852:%d %d %d %d \n",cond1_185_1 , cond2_185_1 ,cond3_185_1 , cond4_185_1,cond1_185 , cond2_185 , cond3_185 , cond4_185);
    //rtd_pr_memc_info("cond3_185 :%d %d %d %d\n",Dtl_array[25],Dtl_array[27],Dtl_array[29],Dtl_array[31]);
    //-----------------123-------------------------//
       
    for(j=0;j<4;j++)
    {
        for(rgnIdx=(8*j);rgnIdx<(8*j+4);rgnIdx++)
        {
            if(rgn_bgcnt[rgnIdx]<300)
            fgcnt_num_left++;
        }
    }

    for(j=0;j<4;j++)
    {
        if(rgn_bgcnt[7+8*j]>900)//<300  //>600   
        fgcnt_num_right++;         
    }
    
    cond_123 = ((fgcnt_num_left>=15 && fgcnt_num_right>=3)&&( _ABS_(gmv_bgx)<70 && _ABS_(gmv_bgx)>30 && _ABS_(gmv_fgx)>30));
         
    if((cond_123 == 1)&&(u11_gmv_mvx<0))
    {
        pOutput->u8_dh_condition_123_broken = 1;
    }
    else 
    {
        pOutput->u8_dh_condition_123_broken = 0;
    }

     //---------------------317 virtical-------------------------//
        
    for(rgnIdx=0;rgnIdx<32;rgnIdx++)
    {
        if(_ABS_(rmvy[rgnIdx])>50)
        {
            ver_mv_fast_num++;
        }
        if(_ABS_(rmvx[rgnIdx])<10)
        {
            hor_mv_slow_num++;
        }
    }
    ver_fast_cond = (ver_mv_fast_num>=30)&&(hor_mv_slow_num>=30);

		if(ver_fast_cond == 1 && aplsum32rgn<3300000)   //0924djn
		{
			pOutput->u8_dh_condition_vertical_broken = 1;
		}
		else 
		{
			pOutput->u8_dh_condition_vertical_broken = 0;
		}

		
		  //rtd_pr_memc_info("num:%d %d rmv5:%d %d\n",ver_mv_fast_num,hor_mv_slow_num,rmv_mvx[5],rmv_mvy[5]);

      //---------------------376 virtical-------------------------//
    for(rgnIdx=0;rgnIdx<32;rgnIdx++)
    {
        if(_ABS_(rmvy[rgnIdx])>40)
        {
            numy_40++;
        }
        if(_ABS_(rmvx[rgnIdx])>20 && rmvx[rgnIdx]<0)
        {
            numx_20++;
        }
    }
    slant_mv_376 = (numy_40>=28)&&(numx_20>=24) && (gmv_bgy>40)&& (_ABS_DIFF_(gmv_bgy,gmv_fgy)>20); 

    if(slant_mv_376 == 1)
    {
        pOutput->u8_dh_condition_slant_mv = 1;
    }
    else 
    {
        pOutput->u8_dh_condition_slant_mv = 0;
    }     


    /////////////////////////-------------------------140--------------------------------/////////////////////
    gmv_value = u11_gmv_mvx+u10_gmv_mvy;

    
    for(rgnIdx = 0;rgnIdx<32;rgnIdx++)
    {
        rmv_value = rmvx[rgnIdx]+rmvy[rgnIdx];
        if(_ABS_DIFF_(rmv_value,gmv_value)>30)
        {
            mv_diff_large++;
        }
        if(_ABS_DIFF_(rmv_value,gmv_value)>20)
        {
            mv_diff_small++;
        }
          

    }
    if(mv_diff_large>25)
    {
         //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x1);
         hold_cnt_140=60;
        // WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);
    }
    else if(hold_cnt_140>0)
    {
         //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x1);
         hold_cnt_140--;
        // WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);
    }
    else
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x0);
       // WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x0);
    }
    for(rgnIdx = 0; rgnIdx < 32; rgnIdx++)
    {
        if(_ABS_DIFF_(rgn_bgcnt[rgnIdx],bgcnt_pre_array[rgnIdx])>200)
        cnt_bgcnt_num++;
    }
    
    for(rgnIdx = 0; rgnIdx < 32; rgnIdx++)
    {
        if(rgn_bgcnt[rgnIdx] < 500)
        {
            fgcnt_num++;            
        }
    }

    if(gmv_bgx >=30  &&  gmv_bgy<=0 && ((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/bg_apl_sum)>=450 && (bg_apl_seg7_cnt*1000/bg_apl_sum)>=250 && rgn_apli[15]>160000 && rgn_apli[23]>100000 && rgn_apli[8]>160000 &&rgn_apli[17]>150000)
    {
         hold_cnt_140=100;
    }
    else if(hold_cnt_140>0)
    {
         hold_cnt_140--;
    }
    else
    {

    }
    cond140_1 =  gmv_bgx >=50 && gmv_bgy<0;
    cond140_2 = ((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/bg_apl_sum)>=500 && (bg_apl_seg7_cnt*1000/bg_apl_sum)>=250;
    cond140_3 = rgn_apli[15]>160000 && rgn_apli[23]>100000 && rgn_apli[8]>160000 && rgn_apli[17]>150000 && rgn_apli[11]>150000 && rgn_apli[12]>150000;

        //-----------------------------------304-------------------------///////
    updateGMVArray(allbgcnt,allbgcnt_length,gmv_bgcnt);
    aplsum32rgn_i = 0;
    aplsum32rgn_p = 0;
    for(rgnIdx=0 ; rgnIdx<32 ;rgnIdx++)
    {    
        aplsum32rgn_i+=rgn_apli[rgnIdx] ; 
        aplsum32rgn_p+=rgn_aplp[rgnIdx] ;
    }
    cod304 = allbgcnt[3]>29000&&allbgcnt[4]>29000&&allbgcnt[5]>29000; //right
    rgn8_aplpsum = rgn_aplp[24]+rgn_aplp[25]+rgn_aplp[26]+rgn_aplp[27]+rgn_aplp[28]+rgn_aplp[29]+rgn_aplp[30]+rgn_aplp[31];

    if(_ABS_DIFF_(aplsum32rgn_p,aplsum32rgn_i)>600000 && aplsum32rgn_i>500000 && aplsum32rgn_i<5000000 &&aplsum32rgn_p>2000000 && aplsum32rgn_p<5000000&& 
        gmv_bgcnt >29000  && _ABS_(u11_gmv_mvx)<15 &&_ABS_(u10_gmv_mvy)<5 && gmv_bgx<15 &&cod304 && ((rgn8_aplpsum>50000 && rgn8_aplpsum<300000) || (rgn8_aplpsum>500000 && rgn8_aplpsum<700000)))
    {
        pOutput->u8_dh_condition_flashing=1;
    }
    else
    {
        pOutput->u8_dh_condition_flashing=0;
    }        

    //////////////////-------------------------129---------------------------//////////

    small_rgnmvx = 0;
    big_rgnmvy20 = 0;
    big_rgnmvy15 = 0;

    for(rgnIdx=0;rgnIdx<32;rgnIdx++)
    {
        if(_ABS_(rmvx[rgnIdx])<10)
            small_rgnmvx++;
        if(_ABS_(rmvy[rgnIdx])>15)
            big_rgnmvy15++;
        if(_ABS_(rmvy[rgnIdx])>20)
            big_rgnmvy20++;
    }
    cond129_1 =  small_rgnmvx>=28 &&   big_rgnmvy15>=2;
    cond129_2 = gmv_fgx<10 && gmv_fgy>25  && gmv_bgx<=10 &&gmv_bgy <=25;
    cond129_3 = rmvy[8]>=30 ||  rmvy[9]>=30;
    cond129_4 = gmv_bgcnt<25000;

    if(cond129_1 && cond129_2 && cond129_3 && cond129_4)
    {
        pOutput->u8_dh_condition_ver_sea=1;
    }
    else
    {
        pOutput->u8_dh_condition_ver_sea=0;
    }

    //------------------------100----------------------------------//

    fast_hori_rgnnum = 0;
    fast_ver_rgnnum = 0;

    fast_hori_rgnnum2 = 0;
    fast_ver_rgnnum2 = 0;

    for(rgnIdx=0;rgnIdx<32;rgnIdx++)
    {
        if(_ABS_(rmvx[rgnIdx])>100 && _ABS_(rmvx[rgnIdx])<140 && rmvx[rgnIdx]<0)
            fast_hori_rgnnum++;
        if(_ABS_(rmvy[rgnIdx])>150 && _ABS_(rmvy[rgnIdx])<170 && rmvy[rgnIdx]<0)
            fast_ver_rgnnum++;
    }
    for(rgnIdx=0;rgnIdx<32;rgnIdx++)
    {
        if(_ABS_(rmvx[rgnIdx])>40 && _ABS_(rmvx[rgnIdx])<70 && rmvx[rgnIdx]<0)
            fast_hori_rgnnum2++;
        if(_ABS_(rmvy[rgnIdx])>130 && _ABS_(rmvy[rgnIdx])<180 && rmvy[rgnIdx]<0)
            fast_ver_rgnnum2++;
    }
    if((fast_hori_rgnnum>=30&& fast_ver_rgnnum>=20)||(fast_hori_rgnnum2==32&& fast_ver_rgnnum2==32 ))
    {
        pOutput->u8_dh_condition_ver_hor_fast = 1;
    }
    else
    {
        pOutput->u8_dh_condition_ver_hor_fast = 0;
    }

    //------------------335-----------------------------------//
    /*
    if(Dtl_array[0]<18000 && Dtl_array[0]>9000  && Dtl_array[3]<12000 && Dtl_array[3]>7000  && Dtl_array[6]<12000 && Dtl_array[6]>7000  && Dtl_array[7]<12000 && Dtl_array[7]>7000  && 
        Dtl_array[16]<70000 && Dtl_array[16]>40000  && Dtl_array[18]<70000 && Dtl_array[18]>50000  && Dtl_array[26]<35000 && Dtl_array[26]>15000
        && _ABS_(u10_gmv_mvy)<10 && u10_gmv_mvy<0 && gmv_bgcnt<29000 && gmv_bgcnt>22000)
    {
        holdframe355=15;
    }
    else if(holdframe100>0)
    {
        holdframe355--;
    }
    else
    {
    }
    */

    
    //--------------------------------------------- 317 broken ----------------------------------------//
    if(((gmv_bgx < -30) && (_ABS_(gmv_bgy) < 5) && (_ABS_(gmv_fgy) < 8) && (gmv_bgcnt > 20000) && (gmv_bgcnt < 25000) &&
        (((fg_apl_seg0_cnt + fg_apl_seg1_cnt)*1000/fg_apl_sum) > 500) && (((fg_apl_seg6_cnt + fg_apl_seg7_cnt)*1000/fg_apl_sum) < 100)  &&
        (((bg_apl_seg0_cnt + bg_apl_seg1_cnt)*1000/bg_apl_sum) < 250) && (((bg_apl_seg6_cnt + bg_apl_seg7_cnt)*1000/bg_apl_sum) > 200)) && (!fwcontrol_339))
    {
        pOutput->u8_dh_condition_317_broken_in_cnt  = pOutput->u8_dh_condition_317_broken_in_cnt + 1;
        pOutput->u8_dh_condition_317_broken_in_cnt = (pOutput->u8_dh_condition_317_broken_in_cnt > 3) ? 3 : pOutput->u8_dh_condition_317_broken_in_cnt;
    }
    else
    {
        pOutput->u8_dh_condition_317_broken_in_cnt = 0;
    }
    if(pOutput->u8_dh_condition_317_broken_in_cnt >= 3)
    {
        pOutput->u8_dh_condition_317_broken=1;
        pOutput->u8_dh_condition_317_broken_hold_cnt = 35;  
    }
    else if(pOutput->u8_dh_condition_317_broken_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_317_broken=1;
        pOutput->u8_dh_condition_317_broken_hold_cnt = pOutput->u8_dh_condition_317_broken_hold_cnt - 1;  
    }
    else
    {
        pOutput->u8_dh_condition_317_broken = 0;
        pOutput->u8_dh_condition_317_broken_hold_cnt = 0;
    }

    //+++++++++++++++++++++++317 broken face+++++++++++++++++++//
    fgratio0 = fg_apl_seg0_cnt*100/(fg_apl_seg0_cnt + bg_apl_seg0_cnt) ;
    cond317_apl0=  ((fg_apl_seg0_cnt + bg_apl_seg0_cnt)<8500)&&((fg_apl_seg0_cnt + bg_apl_seg0_cnt)>3400)&&(fgratio0<93)&&(fgratio0>65) ;
    cond317_apl1 =  ((fg_apl_seg1_cnt + bg_apl_seg1_cnt)<8000)&&((fg_apl_seg1_cnt + bg_apl_seg1_cnt)>4500) ;
    cond317_apl2 =  ((fg_apl_seg2_cnt + bg_apl_seg2_cnt)<7000)&&((fg_apl_seg2_cnt + bg_apl_seg2_cnt)>2500) ;
    cond317_apl3 =  ((fg_apl_seg3_cnt + bg_apl_seg3_cnt)<2500)&&((fg_apl_seg3_cnt + bg_apl_seg3_cnt)>1500) ;  
    cond317_apl7 = fg_apl_seg7_cnt<80&&((fg_apl_seg7_cnt + bg_apl_seg7_cnt)<350);
    cond317_mv = (_ABS_(gmv_fgx) >9) &&(_ABS_(gmv_fgx) <36)&&(u11_gmv_mvx <= -30) &&(u11_gmv_mvx > -85)&& (_ABS_(gmv_bgy) <=3) && (_ABS_(gmv_fgy) <= 3) && (gmv_bgcnt > 19000) && (gmv_bgcnt < 25000) ;

    //1011
    if(rgn_apli[26]>15000 && cond317_mv&&((cond317_apl0&&cond317_apl1&&cond317_apl2&&cond317_apl3&&cond317_apl7)||(fg_apl_sum>(bg_apl_sum+3000))))
    {
        pOutput->u8_me1_condition_317_face_flag = 1;
    }
    else
    {
        pOutput->u8_me1_condition_317_face_flag = 0;
    }
    //rtd_pr_memc_info("317face:%d %d %d\n",pOutput->u8_me1_condition_317_face_flag,pOutput->u8_me1_condition_317_face_flag2,pOutput->u8_me1_condition_317_face_flag3);

    
 //rtd_pr_memc_info("317info %d %d %d %d %d %d %d %d %d %d %d %d %d\n"  , gmv_fgx ,  u11_gmv_mvx ,   gmv_bgy ,  gmv_fgy ,gmv_bgx, gmv_bgcnt   ,cond317_apl0 ,cond317_apl1,cond317_apl2,cond317_apl3,cond317_apl7, cond317_mv,  pOutput->u8_me1_condition_317_face_flag) ;    
       //-------------------------------------171 begin ----------------------------------------//
    if(pOutput->u8_me1_condition_black_off_case1)
    {
        pOutput->u8_dh_condition_171_in_cnt  = pOutput->u8_dh_condition_171_in_cnt + 1;
        pOutput->u8_dh_condition_171_in_cnt = (pOutput->u8_dh_condition_171_in_cnt > 2) ? 2 : pOutput->u8_dh_condition_171_in_cnt;
    }
    else
    {
        pOutput->u8_dh_condition_171_in_cnt = 0;
    }
    if(pOutput->u8_dh_condition_171_in_cnt >= 2)
    {
        pOutput->u8_dh_condition_171=1;
        pOutput->u8_dh_condition_171_hold_cnt = 60;
    }
    else if(pOutput->u8_dh_condition_171_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_171=1;
        pOutput->u8_dh_condition_171_hold_cnt = pOutput->u8_dh_condition_171_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_171=0;
        pOutput->u8_dh_condition_171_hold_cnt = 0;
    }
	
	//++++++++++++++++172 broken ++++++++++++++//
    fgratio0 = (fg_apl_seg0_cnt)*100/fg_apl_sum;
    fgratio01 = (fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum;
        if(fgratio0>=75&&fgratio01>=85&&fg_apl_seg0_cnt>6000&&(fg_apl_seg0_cnt+fg_apl_seg1_cnt)>7000&&fg_apl_sum>8000&&(u11_gmv_mvx <= -5) &&(u11_gmv_mvx > -50)&&(_ABS_(u10_gmv_mvy) <2)&&bg_apl_sum>14500&&bg_apl_seg7_cnt<200)
    {
        pOutput->u8_me1_condition_172_black_th_flag =1 ;
    }
    else
    {
        pOutput->u8_me1_condition_172_black_th_flag =0 ;

    }

    //--------------------------------------------- 52 109 bottom rim----------------------------------//

    bottom_rim_pre=s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_BOT];
    arv_mv_y_bottom=(_ABS_(rmvy[24])+_ABS_(rmvy[25])+_ABS_(rmvy[26])+_ABS_(rmvy[27])+_ABS_(rmvy[28])+_ABS_(rmvy[29])+_ABS_(rmvy[30])+_ABS_(rmvy[31]))/8;
    if(arv_mv_y_bottom>40)   //52 rim bottom
    {
        WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,9,17,  bottom_rim_pre-15);
        rim_hold_cnt_bottom=20;
    }
    else if(rim_hold_cnt_bottom>0)
    {
        WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,9,17,  bottom_rim_pre-15);
        rim_hold_cnt_bottom--;
    }
    else 
    {            
        WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,9,17,  bottom_rim_pre);  
    }


	// --------------------------logo rim-------------------------------// //1228
	sum_logo_cnt_bottom = (logo_rgn_cnt[24]+logo_rgn_cnt[25]+logo_rgn_cnt[26]+logo_rgn_cnt[27]+logo_rgn_cnt[28]+logo_rgn_cnt[29]+logo_rgn_cnt[30]+logo_rgn_cnt[31]);
	//rtd_pr_memc_info("sum_logo_cnt_bottom=%d\n",sum_logo_cnt_bottom);
	if(sum_logo_cnt_bottom>2500)
	{
		rim_hold_cnt_bottom_logo = 30;	
		video_logo_bottom = 1;
		pOutput->u8_dh_condition_logo_rimoff = 1;
		//WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0);
		WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,9,17,  bottom_rim_pre-30);

	}
	else if(rim_hold_cnt_bottom_logo>0)
	{
	        rim_hold_cnt_bottom_logo--;
		pOutput->u8_dh_condition_logo_rimoff = 1;
		//WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x0);
       	 WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,9,17,  bottom_rim_pre-30);

	}
	else if(video_logo_bottom>0)
	{
		video_logo_bottom = 0;
		pOutput->u8_dh_condition_logo_rimoff = 0;
		//WriteRegister(KME_DEHALO5_PHMV_FIX_0_reg,12,12,0x1);
		 WriteRegister(KME_DEHALO_KME_DEHALO_E4_reg,9,17,  bottom_rim_pre);
	}



	
    
    //------------------------------------scene change pattern (051)------------------------------// 
    
    scflag_cnt=0;
    updateGMVArray(scflag_arrayx,scflag_length,gmv_fgx); 
    updateGMVArray(scflag_arrayy,scflag_length,gmv_fgy); 
    fgx_diff = scflag_arrayx[8]-scflag_arrayx[9];  //not ABS
    //fgy_diff = ABS(scflag_arrayy[8],scflag_arrayy[9]);
    fgy_diff = scflag_arrayy[8]-scflag_arrayy[9];
    for(scflag_num=1;scflag_num<9;scflag_num++)
    {
        if((_ABS_DIFF_(scflag_arrayx[scflag_num-1],scflag_arrayx[scflag_num])<4)&&(_ABS_DIFF_(scflag_arrayy[scflag_num-1],scflag_arrayy[scflag_num])<4))
        scflag_cnt++;           
    }
    if((((scflag_cnt == 8)&&((_ABS_(fgx_diff)+_ABS_(fgy_diff))>=24))||((scflag_cnt == 7)&&((_ABS_(fgx_diff)+_ABS_(fgy_diff))>=34)))&& gmv_bgcnt>30000 && scflag_arrayx[8]>=20 && Dtl_array[24]<40000 && Dtl_array[25]<30000 && Dtl_array[26]<30000)
    {
        pOutput->u8_dh_condition_051_bypass = 1;
    }
    else
    {
        pOutput->u8_dh_condition_051_bypass = 0;
    }

    //--------------------- scene change pattern (030) sc garbage lp ----------------------//
    cond_030_1 = (rgn_apli[8]-rgn_aplp[8])>20000 && (rgn_apli[9]-rgn_aplp[9])>50000 && (rgn_apli[10]-rgn_aplp[10])>100000 && (rgn_apli[11]-rgn_aplp[11])>35000 && (rgn_apli[12]-rgn_aplp[12])>1000 && (rgn_apli[13]-rgn_aplp[13])>35000 
        && (rgn_apli[14]-rgn_aplp[14])>80000 && (rgn_apli[15]-rgn_aplp[15])>80000;
    cond_030_2 = (rgn_aplp[16]-rgn_apli[16])> 30000 && (rgn_aplp[17]-rgn_apli[17])> 10000 && (rgn_apli[18]-rgn_aplp[18])> 30000  
        && (rgn_apli[20]-rgn_aplp[20])> 10000 && (rgn_aplp[23]-rgn_apli[23])> 20000;

    if(cond_030_1 && cond_030_2)
    {
        pOutput->u8_dh_condition_030_bypass = 1;
    }
    else
    {
        pOutput->u8_dh_condition_030_bypass = 0;
    }

    //--------------------- scene change pattern (024) sc garbage lp ----------------------//
    
    if(((_ABS_DIFF_(bgcnt_pre24,gmv_bgcnt)>7000 && _ABS_DIFF_(gmv_fgy_pre,gmv_fgy)>=45)||(_ABS_DIFF_(gmv_fgy_pre,gmv_fgy)>40 && (_ABS_DIFF_(gmv_fgx_pre,gmv_fgx)>15) && gmv_bgcnt>25000 &&gmv_bgcnt<28000))&& Dtl_array[24]<20000 && Dtl_array[25]<20000 && Dtl_array[26]<20000 )
    {
        pOutput->u8_dh_condition_024_bypass = 1;
    }
    else 
    {
        pOutput->u8_dh_condition_024_bypass = 0;
    }  
    bgcnt_pre24 = gmv_bgcnt;
    gmv_fgy_pre = gmv_fgy;
    gmv_fgx_pre = gmv_fgx;


    //-------------------------------------346  under sea ------------------------------------------//
    bgcnt_full_sum=0;               
    for(rgnIdx=0;rgnIdx<16;rgnIdx++)
    {
        if(bgcnt_array[rgnIdx]==1020)
            bgcnt_full_sum++;
    }
    for(rgnIdx=16;rgnIdx<19;rgnIdx++)
    {
        if(bgcnt_array[rgnIdx]<1020)
            bgcnt_full_sum++;
    }
    for(rgnIdx=19;rgnIdx<24;rgnIdx++)
    {
        if(bgcnt_array[rgnIdx]>=1010)
            bgcnt_full_sum++;
    }
    for(rgnIdx=24;rgnIdx<27;rgnIdx++)
    {
        if(bgcnt_array[rgnIdx]<990)
            bgcnt_full_sum++;
    }
    for(rgnIdx=27;rgnIdx<32;rgnIdx++)
    {
        if(bgcnt_array[rgnIdx]>=960)
            bgcnt_full_sum++;
    }


        if((gmv_bgcnt>30000 && gmv_bgcnt<32400 && bgcnt_full_sum>=29 &&((bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*100/bg_apl_sum)>=25 && aplsum32rgn<2900000 &&
            _ABS_(u11_gmv_mvx)<10 && _ABS_(u10_gmv_mvy)<5 && ((fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*100/fg_apl_sum)<=40) && (pOutput->u8_me1_condition_173_begin_flag == 0))
        {
            pOutput->u8_dh_condition_346_bypass = 1;
        }
        else
        {
            pOutput->u8_dh_condition_346_bypass = 0;
        }
        
        //------------------------------ 110 ----------------------------------//
        sim_mvx_num = 0;
        sim_mvy_num = 0;
        rgnmvx_range = 0;
        rgnmvy_range = 0;
        sim_mvx_num_mid = 0;
        sim_mvy_num_mid = 0;
        rgnmv_range_mid = 0;
        for(rgnIdx=0;rgnIdx<7;rgnIdx++)
        {
            if (_ABS_DIFF_(rmvx[rgnIdx],rmvx[rgnIdx+1])<=5)
                sim_mvx_num++;
            if (_ABS_DIFF_(rmvy[rgnIdx],rmvy[rgnIdx+1])<=5)
                sim_mvy_num++;
            if(rmvx[rgnIdx]<60 && rmvx[rgnIdx]>30)
                rgnmvx_range++;  
            if(rmvy[rgnIdx]<50 && rmvy[rgnIdx]>10)
                rgnmvy_range++;
        }
            if(rmvx[7]<60 && rmvx[7]>30)
                rgnmvx_range++;
            if(rmvy[7]<50 && rmvy[7]>10)
                rgnmvy_range++;
            
        for(rgnIdx=23;rgnIdx<30;rgnIdx++)
        {
            if (_ABS_DIFF_(rmvx[rgnIdx],rmvx[rgnIdx+1])<=5)
                sim_mvx_num++;
            if (_ABS_DIFF_(rmvy[rgnIdx],rmvy[rgnIdx+1])<=5)
                sim_mvy_num++;
            if(rmvx[rgnIdx]<60 && rmvx[rgnIdx]>30)
                rgnmvx_range++;
            if(rmvy[rgnIdx]<50 && rmvy[rgnIdx]>10)
                rgnmvy_range++;
        }
            if(rmvx[31]<60 && rmvx[31]>30)
                rgnmvx_range++;
            if(rmvy[31]<50 && rmvy[31]>10)
                rgnmvy_range++;
            
        for(rgnIdx=8;rgnIdx<23;rgnIdx++)
        {
            if (_ABS_DIFF_(rmvx[rgnIdx],rmvx[rgnIdx+1])<=10)
                sim_mvx_num_mid++;
            if (_ABS_DIFF_(rmvy[rgnIdx],rmvy[rgnIdx+1])<=10)
                sim_mvy_num_mid++;
            if(rmvx[rgnIdx]>=30 && rmvx[rgnIdx]<=70 &&rmvy[rgnIdx]<0 && _ABS_(rmvy[rgnIdx])>5 && _ABS_(rmvy[rgnIdx])<80)
                rgnmv_range_mid++;
        }
            if(rmvx[23]>=50 && rmvx[23]<=110 && rmvy[23]<0 && _ABS_(rmvy[23])>=25 && _ABS_(rmvy[23])<=80)
                rgnmv_range_mid++;
        //rtd_pr_memc_info("out:%d %d %d %d in:%d %d %d mv:(%d %d )\n",sim_mvx_num,sim_mvy_num,rgnmvx_range,rgnmvy_range,sim_mvx_num_mid,sim_mvy_num_mid,rgnmv_range_mid,u11_gmv_mvx,u10_gmv_mvy);
        if(sim_mvx_num>=13 && sim_mvy_num>=11 && rgnmvx_range>=14 && rgnmvy_range>=14 && sim_mvx_num_mid>=14 && sim_mvy_num_mid>=12 && rgnmv_range_mid>=15)
        {
              pOutput->u8_dh_condition_110_bypass = 1;
        }
        else
        {
            pOutput->u8_dh_condition_110_bypass = 0;
        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, pOutput->u8_dh_condition_110_bypass);


    //------------------------------184 ship broken ------------------------//
    sum_bgcnt_lefthalf = 0;
    sum_bgcnt_righthalf = 0;
    for(j=0;j<4;j++)
    {
        for(rgnIdx=0;rgnIdx<4;rgnIdx++)
        {
            sum_bgcnt_lefthalf+=bgcnt_array[rgnIdx+8*j];
            sum_bgcnt_righthalf+=bgcnt_array[rgnIdx+4+8*j];
        }
    }
    //rtd_pr_memc_info("%d %d %d\n",sum_bgcnt_lefthalf,sum_bgcnt_righthalf,gmv_bgcnt);  

    cond1841 = sum_bgcnt_lefthalf>16000 && sum_bgcnt_righthalf<11000 && sum_bgcnt_righthalf>5000;
    cond1842 = u11_gmv_mvx == 0 && u10_gmv_mvy == 0 && gmv_fgx <0 && _ABS_(gmv_fgx)>55 && _ABS_(gmv_fgx)<60;
    cond1843 = aplsum32rgn >4800000 && aplsum32rgn<5100000 && Dtl_array[25]>24000 && Dtl_array[25]<30000 && Dtl_array[29]>4000 && Dtl_array[29]<10000;
    //rtd_pr_memc_info("cond:%d %d %d\n",cond1841,cond1842,cond1843);
    //rtd_pr_memc_info("%d %d (%d %d) (%d %d) %d %d %d\n",sum_bgcnt_lefthalf,sum_bgcnt_righthalf,u11_gmv_mvx,u10_gmv_mvy,gmv_fgx,gmv_fgy,aplsum32rgn,Dtl_array[25],Dtl_array[29]);  
    if(sum_bgcnt_lefthalf>16000 && sum_bgcnt_righthalf<11000 && sum_bgcnt_righthalf>5000 && ((u11_gmv_mvx == 0) || (u11_gmv_mvx == -1))&& u10_gmv_mvy == 0
        && gmv_fgx <0 && _ABS_(gmv_fgx)>55 && _ABS_(gmv_fgx)<60 && aplsum32rgn >4800000 && aplsum32rgn<5500000)
    {
        pOutput->u8_dh_condition_184_bypass = 1;
    }
    else
    {
        pOutput->u8_dh_condition_184_bypass = 0;
    }
    //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, pOutput->u8_dh_condition_184_bypass);
    //rtd_pr_memc_info("%d %d gmv:%d %d %d apl:%d\n",sum_bgcnt_lefthalf,sum_bgcnt_righthalf,u11_gmv_mvx,u10_gmv_mvy,gmv_fgx,aplsum32rgn);

    
    //------------------------------- 342 begin flying bird -------------------------//
    cond384_1 = Dtl_array[5] == 0 &&  Dtl_array[6] == 0 &&  Dtl_array[7] == 0 &&  Dtl_array[13] == 0 &&  Dtl_array[14] == 0 &&  Dtl_array[15] == 0 && 
    Dtl_array[21] == 0 &&  Dtl_array[22] == 0 &&  Dtl_array[23] == 0 &&  Dtl_array[29] == 0 &&  Dtl_array[30] == 0 &&  Dtl_array[31] == 0;
    cond384_2 = aplsum32rgn>6000000 && aplsum32rgn<7000000;
    cond384_3 = ((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum)>90 && ((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum)<10;
    //rtd_pr_memc_info("cond:%d %d %d\n",cond384_1,cond384_2,cond384_3);
                                                        
    if(cond384_1 &&  cond384_2 &&  cond384_3)
    {
        pOutput->u8_dh_condition_342_bypass = 1;
    }
    else
    {
        pOutput->u8_dh_condition_342_bypass = 0;
    }

    
    //---------------------------- 026 star wars ---------------------------------//
    apl_changenum_3 = 0;
    apl_changenum_5 = 0;
    apl_changenum_7 = 0;
    apl_changenum_9 = 0;
    apl_changenum_outside = 0;
    for(j=0;j<4;j++)
    {
        for(rgnIdx=2;rgnIdx<6;rgnIdx++)
        {
            if(_ABS_DIFF_(rgn_apli[rgnIdx+8*j],rgn_aplp[rgnIdx+8*j])>30000)
               apl_changenum_3++;
            if(_ABS_DIFF_(rgn_apli[rgnIdx+8*j],rgn_aplp[rgnIdx+8*j])>50000)
               apl_changenum_5++;
            if(_ABS_DIFF_(rgn_apli[rgnIdx+8*j],rgn_aplp[rgnIdx+8*j])>70000)
               apl_changenum_7++;
            if(_ABS_DIFF_(rgn_apli[rgnIdx+8*j],rgn_aplp[rgnIdx+8*j])>90000)
               apl_changenum_9++;
        }
    }
    for(j=0;j<4;j++)
    {
        for(rgnIdx=0;rgnIdx<2;rgnIdx++)
        {
            if(_ABS_DIFF_(rgn_apli[rgnIdx+8*j],rgn_aplp[rgnIdx+8*j])<10000)
               apl_changenum_outside++;
        }
    }
    for(j=0;j<4;j++)
    {
        for(rgnIdx=6;rgnIdx<8;rgnIdx++)
        {
            if(_ABS_DIFF_(rgn_apli[rgnIdx+8*j],rgn_aplp[rgnIdx+8*j])<10000)
               apl_changenum_outside++;
        }
    }

    cond26_1 = apl_changenum_3>5 && apl_changenum_5>3 && apl_changenum_7>3 && apl_changenum_9>1 && apl_changenum_outside>6;
    cond26_2 = Dtl_array[5]>9000 && Dtl_array[5]<12000 && Dtl_array[6]>10000 && Dtl_array[6]<14000 && Dtl_array[7]>8000 && Dtl_array[7]<9000 && 
        Dtl_array[14]>2000 && Dtl_array[14]<4000  && Dtl_array[15]<5000 && Dtl_array[21]>20000 && Dtl_array[21]<27000 && 
        Dtl_array[22]>18000 && Dtl_array[22]<24000 && Dtl_array[23]>20000 && Dtl_array[23]<27000 && Dtl_array[29]>21000 && Dtl_array[29]<25000 && 
        Dtl_array[30]>18000 && Dtl_array[30]<22000 && Dtl_array[31]>24000 && Dtl_array[31]<30000 ;
        cond26_apl = (fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)< 4000 ;
       if((cond26_1 || cond26_2)&&cond26_apl&&_ABS_(u10_gmv_mvy)<3&&_ABS_(u11_gmv_mvx)<25)
// if((cond26_1 || cond26_2))
     {
            pOutput->u8_dh_condition_26_bypass = 1;

        }
        else
        {
            pOutput->u8_dh_condition_26_bypass = 0;
 
        }

    //---------------------------------  184 bridge cable blackline  ------------------------------------//
    if(Dtl_array[1]<5 &&  Dtl_array[2]<5 &&  Dtl_array[9]<5 &&  Dtl_array[10]<5 &&  Dtl_array[17]<5 &&  
        Dtl_array[18]<5 && Dtl_array[0]==4352 &&Dtl_array[8]==4352 &&Dtl_array[16]==4352 && ((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum)>45)
    {
        pOutput->u8_dh_condition_1841_bypass = 1;
    }
    else 
    {
        pOutput->u8_dh_condition_1841_bypass = 0;
    }


    //-------------------------------  207 black line  ----------------------------------------//
    bgcnt_lefthalf = 0;
    bgcnt_righthalf = 0;
    for(j=0;j<4;j++)
    {
        for(rgnIdx=0;rgnIdx<3;rgnIdx++)
        {
            bgcnt_lefthalf = bgcnt_lefthalf+bgcnt_array[rgnIdx+j*8];
        }
    }
    for(j=0;j<4;j++)
    {
        for(rgnIdx=4;rgnIdx<8;rgnIdx++)
        {               
            bgcnt_righthalf = bgcnt_righthalf+bgcnt_array[rgnIdx+j*8];
        }
    }        
    
    if((((bgcnt_lefthalf<2000 && bgcnt_righthalf>15500 && gmv_bgcnt<22000)||(gmv_bgcnt<7000)) && _ABS_(gmv_bgx)>70 && gmv_bgx<0 &&_ABS_(gmv_bgy)<10 && _ABS_(gmv_fgx)<10 && _ABS_(gmv_fgy)<10 && aplsum32rgn<6200000 && aplsum32rgn>5500000)||(gmv_bgcnt<1000 && gmv_fgx<0 && _ABS_(gmv_fgx)>25 && gmv_bgx==0 && gmv_bgy==0 && aplsum32rgn<6200000 && aplsum32rgn>5500000))
    {            
        pOutput->u8_dh_condition_207_bypass = 1;
    }
    else 
    {
        pOutput->u8_dh_condition_207_bypass = 0;
    }        

    //-------------------------------043 girl skating -------------------------//
    for(j=0;j<8;j++)
    {
        topdtlsum =  topdtlsum+Dtl_array[j];
        if(rgn_apli[24+j]>150000 && rgn_apli[24+j]<250000)
            bottom_aplcnt++;
    }
    cond043_1 = gmv_bgcnt>29000 && _ABS_(gmv_bgx)<10 && _ABS_(gmv_bgy)<5 && gmv_fgx>=90 && gmv_fgx<=120 && _ABS_(gmv_fgy)<5;
    //cond043_2 = Dtl_array[0]==4352 && Dtl_array[7]==4352 && topdtlsum<8754 && topdtlsum>8704;
    //cond043_3 = aplsum32rgn>5000000 && aplsum32rgn<5300000;

    //for120    
    cond043_2 = Dtl_array[0]<=4352 && Dtl_array[0]>3700 && Dtl_array[7]<4400 && Dtl_array[7]>4000 && topdtlsum<8754 && topdtlsum>8050;
    cond043_3 = aplsum32rgn>5000000 && aplsum32rgn<5500000;
    
    if(cond043_1 && cond043_2 && cond043_3)
    {
        pOutput->u8_me1_condition_43_flag = 1;
    }
    else
    {
        pOutput->u8_me1_condition_43_flag = 0;
    }
    //rtd_pr_memc_info("cond43:%d %d %d \n",cond043_1,cond043_2,cond043_3);

    //---------------------------333 cond ---------------------------//

    cond333_rgnapl1 = 0;
    cond333_rgnapl2 = 0;
    cond333_rgnapl3 = 0;                
    for(rgnIdx=8;rgnIdx<24;rgnIdx++)
    {
        if(rgn_apli[rgnIdx]<10000)
            cond333_rgnapl1++;
        if(rgn_apli[rgnIdx]>20000)
            cond333_rgnapl2++;
        if(rgn_apli[rgnIdx]>100000)
            cond333_rgnapl3++;
    }
    cond333_2 = (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum)) > 250) && (((bg_apl_seg5_cnt + bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum)) > 250);
    cond333_3 = (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 800) && (((fg_apl_seg5_cnt + fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum)) < 100) && (sum_apl > 1400000) && (sum_apl < 2400000) ;
    cond333_4 = cond333_rgnapl1>0 &&cond333_rgnapl1<=4 && cond333_rgnapl2>=10 && cond333_rgnapl3>=3 && Dtl_array[23]>5000;
    cond333_5 = ((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum)>80 && ((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum)<10 && aplsum32rgn<2400000 && aplsum32rgn>1400000 && _ABS_(u11_gmv_mvx)>45 && _ABS_(u10_gmv_mvy)<=10 && u11_gmv_mvx<0 && u10_gmv_mvy<0;
    
    if(cond333_2 && cond333_3 && cond333_4 && cond333_5)
    {
        pOutput->u8_dh_condition_333_bypass = 1;
    }
    else
    {
        pOutput->u8_dh_condition_333_bypass = 0;
    }

    
    //+++++++ for slow hortical   but  vertical moving  video  165
    if(_ABS_DIFF_(gmv_fgx,u11_gmv_mvx)<8&&_ABS_DIFF_(gmv_fgy,u10_gmv_mvy) >21&&gmv_bgcnt>30000&& fg_apl_seg0_cnt<200 ) 
    {
        pOutput->u8_me1_condition_165_ver_th =1 ; 
    }
    else
    {
        pOutput->u8_me1_condition_165_ver_th =0 ; 

    }

    aplcnt1_183 =0 ;  
    aplcnt2_183 =0 ;          
    for(i =0 ; i< 8; i++)
    {      
        if(rgn_apli[i]>165000)
        aplcnt1_183 ++ ;
        if(rgn_apli[i]>190000)
        aplcnt2_183 ++ ;
    }
    if((aplcnt1_183==8)&&aplcnt2_183>3&& gmv_bgcnt<25000&& gmv_bgcnt>16000&&_ABS_(u10_gmv_mvy)<2&&u11_gmv_mvx>16&&u11_gmv_mvx<50) 
    {
        pOutput->u8_me1_condition_183_face_broken =1  ;
    }
    else
    {
        pOutput->u8_me1_condition_183_face_broken =0  ;
    }

	//rtd_pr_memc_info("flag:%d cond:%d %d %d %d %d\n",pOutput->u8_me1_condition_183_face_broken,aplcnt1_183,aplcnt2_183,gmv_bgcnt,u10_gmv_mvy,u11_gmv_mvx);
	

    //+++++++++++++++++151 broken++++++++++++++++
    fgmv_val = _ABS_(gmv_fgx)+_ABS_(gmv_fgy) ;

    bgmv_val = _ABS_(u11_gmv_mvx)+_ABS_(u10_gmv_mvy) ;
    fgbgdiff = _ABS_DIFF_(gmv_fgx, u11_gmv_mvx) + _ABS_DIFF_(gmv_fgy, u10_gmv_mvy)  ;
    aplcond = rgn_apli[0]>160000&&rgn_apli[1]>140000&&rgn_apli[2]>140000&& rgn_apli[8]>160000&& rgn_apli[9]>160000&& rgn_apli[10]>160000 ;
    aplsum = rgn_apli[0]+ rgn_apli[1]+ rgn_apli[2]+ rgn_apli[8]+ rgn_apli[9]+ rgn_apli[10] ;
    ReadRegister(KME_ME1_BG1_ME_BG_TH0_reg,0,9,&mvth); 
    if(fgmv_val<40&&bgmv_val<40&&fgbgdiff<12&&_ABS_(gmv_fgy)>=4&&gmv_bgcnt<23000&&gmv_bgcnt>14000&&u11_gmv_mvx>10&&gmv_fgx>=10&&aplsum>900000&&mvth<15&&rgn_apli[4]<100000)
    {
        pOutput->u8_me1_condition_151_broken =1  ;
    }
    else
    {
        pOutput->u8_me1_condition_151_broken =0  ;
    }    

   // rtd_pr_memc_info("151 info : %d %d %d %d %d %d %d %d %d\n  " ,pOutput->u8_me1_condition_151_broken ,u11_gmv_mvx , rgn_apli[0],rgn_apli[1] ,rgn_apli[2], rgn_apli[3] ,rgn_apli[4] ,rgn_apli[5] ,rgn_apli[10]);
    //+++++++++++++++195 face broken++++++++++++++++++++//
    bgleftcnt1 =  rgn_bgcnt[0]+ rgn_bgcnt[1]+rgn_bgcnt[2]+ rgn_bgcnt[3] ;
    // bgleftcnt2 =  rgn_bgcnt[8]+ rgn_bgcnt[9]+rgn_bgcnt[10]+ rgn_bgcnt[11]+rgn_bgcnt[16]+ rgn_bgcnt[17]+rgn_bgcnt[18]+ rgn_bgcnt[19]+rgn_bgcnt[24]+ rgn_bgcnt[25]+rgn_bgcnt[26]+ rgn_bgcnt[27] ;
    // bgleftcnt3 =  rgn_bgcnt[3]+ rgn_bgcnt[11]+rgn_bgcnt[19]+ rgn_bgcnt[27] ;
    bgrightcnt =  rgn_bgcnt[4]+ rgn_bgcnt[5]+rgn_bgcnt[6]+ rgn_bgcnt[7]+rgn_bgcnt[12]+ rgn_bgcnt[13]+rgn_bgcnt[14]+ rgn_bgcnt[15]+
               rgn_bgcnt[20]+ rgn_bgcnt[21]+rgn_bgcnt[22]+ rgn_bgcnt[23]+rgn_bgcnt[28]+ rgn_bgcnt[29]+rgn_bgcnt[30]+ rgn_bgcnt[31] +rgn_bgcnt[3]+ rgn_bgcnt[11]+rgn_bgcnt[19]+ rgn_bgcnt[27];
    fgseg0ratio =fg_apl_seg0_cnt*100/fg_apl_sum ; 
    if(u11_gmv_mvx>55 &&u11_gmv_mvx<110&& _ABS_(u10_gmv_mvy)<3&&bgrightcnt>20000&&gmv_bgcnt>28000&&bgleftcnt1>3930&&fg_apl_sum>2500&&fgseg0ratio<=14&&_ABS_(gmv_fgx)<12&&_ABS_(gmv_fgy)>5&&rgn_apli[7] <95000&& rgn_apli[15]<65000)
    {
        pOutput->u8_me1_condition_195_face_broken =1  ;
    }
    else
    {

        pOutput->u8_me1_condition_195_face_broken =0  ;
    }


    //----------------------------- 128 --------------------------------//
    //1011
    //off545
    //cond128_mv = ((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum)>90 && ((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum)<=20 && aplsum32rgn>1200000 && aplsum32rgn<2700000 && _ABS_(u10_gmv_mvy)<20 && _ABS_(u11_gmv_mvx)>130 && u11_gmv_mvx<0;
    //cond128_dtl = Dtl_array[24]>9000 && Dtl_array[24]<14000 && Dtl_array[31]>10000 && Dtl_array[31]<18000 && Dtl_array[10]<10000 && Dtl_array[16]<10000 && Dtl_array[25]<12000 && Dtl_array[25]>7000;


    cond128_mv = ((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum)>90 && ((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum)<=25 && aplsum32rgn>1100000 && aplsum32rgn<1900000 && _ABS_(u10_gmv_mvy)<25 && _ABS_(u11_gmv_mvx)>100 && u11_gmv_mvx<0;
    cond128_dtl = Dtl_array[24]>9000 && Dtl_array[24]<22000 && Dtl_array[31]>9000 && Dtl_array[31]<20000 && Dtl_array[10]<20000 && Dtl_array[16]<18000 && Dtl_array[25]<20000 && Dtl_array[25]>7000;
    
    if(cond128_mv && cond128_dtl )
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x1);
        holdframe128=40;
        pOutput->u8_me1_condition_128_flag = 1;
        path = 6;
    }
    else
    {
		pOutput->u8_me1_condition_128_flag = 0;
        path = 0;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x0);
    }
    //rtd_pr_memc_info("128:%d %d \n",cond128_mv , cond128_dtl);
    //rtd_pr_memc_info("1281:%d %d %d %d %d\n",((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum),((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum),aplsum32rgn,u10_gmv_mvy,u11_gmv_mvx);
    //rtd_pr_memc_info("1282:%d %d %d %d %d\n",Dtl_array[24],Dtl_array[31],Dtl_array[10],Dtl_array[16],Dtl_array[25]);   



    //------------------------ 35 ------------------------//
    for(rgnIdx=24;rgnIdx<32;rgnIdx++)
    {
        if((rgn_apli[rgnIdx])>110000 && rgn_apli[rgnIdx]<160000)
            rgn_aplcnt++;
        if((rgn_apli[rgnIdx-8])>90000 && rgn_apli[rgnIdx-8]<170000)
            rgn_aplcnt2++;
        if((rgn_apli[rgnIdx])>130000 && rgn_apli[rgnIdx]<180000)
            rgn_aplcnt3++;
    }
    //off545
    //cond35_1 = ABS(gmv_bgx,gmv_fgx)>70  && (gmv_bgx>50 || gmv_fgx>50) && gmv_bgcnt>20000 && gmv_bgcnt<27000 && aplsum32rgn>4000000 && aplsum32rgn<4600000;
    //cond35_2 = rgn_aplcnt>=7; 
    //cond35_3 = Dtl_array[23]>4000 && Dtl_array[23]<13000 &&Dtl_array[22]<10000 && Dtl_array[21]>4000 && Dtl_array[21]<8000 &&Dtl_array[8]>12000 &&Dtl_array[8]<19000;

    cond35_1 = _ABS_DIFF_(gmv_bgx,gmv_fgx)>70  && (gmv_bgx>50 || gmv_fgx>50) && gmv_bgcnt>18000 && gmv_bgcnt<27000 && aplsum32rgn>3400000 && aplsum32rgn<4300000;
    cond35_2 = rgn_aplcnt>=7 ; 
    cond35_3 = Dtl_array[23]>4000 && Dtl_array[23]<20000 &&Dtl_array[22]<12000 && Dtl_array[21]>4000 && Dtl_array[21]<20000 &&Dtl_array[8]>12000 &&Dtl_array[8]<25000;
        
    if(cond35_1 && cond35_2 && cond35_3)
    {
        pOutput->u8_dh_condition_35_broken_in_cnt  = pOutput->u8_dh_condition_35_broken_in_cnt + 1;
        pOutput->u8_dh_condition_35_broken_in_cnt = (pOutput->u8_dh_condition_35_broken_in_cnt > 3) ? 3 : pOutput->u8_dh_condition_35_broken_in_cnt;
    }
    else
    {
        pOutput->u8_dh_condition_35_broken_in_cnt = 0;
    }
    if(pOutput->u8_dh_condition_35_broken_in_cnt >= 3)
    {
        pOutput->u8_dh_condition_35_bypass = 1;
    }
    //else if(holdframe35>0)
    //{
    //    holdframe35--;
    //    WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x2);
    //}
    else
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x0);
        pOutput->u8_dh_condition_35_bypass =0;
    }
    //rtd_pr_memc_info("%d %d %d\n",cond35_1, cond35_2, cond35_3);
    //rtd_pr_memc_info("1:%d %d %d %d 2:%d 3:%d %d %d %d\n",gmv_bgx, gmv_bgy, gmv_bgcnt,aplsum32rgn,rgn_aplcnt,Dtl_array[23],Dtl_array[22],Dtl_array[21],Dtl_array[8]);

    
   

    // --------------------- 412 dancing ---------------------//
    //off545
    //cond412_1 = gmv_bgcnt>22000 && gmv_bgcnt<30000 && ((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum)>75 && ((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum)<=25 ;
    //cond412_2 = gmv_bgx == 0 && gmv_bgy == 0  && aplsum32rgn<3400000 && aplsum32rgn>2900000;
    //cond412_3 = Dtl_array[7]<36000 && Dtl_array[7]>30000 && Dtl_array[6]<900 && rgn_apli[3]>13000 && (rgn_apli[4]>15000 || rgn_apli[4]<12000);

    cond412_1 = gmv_bgcnt>22000 && gmv_bgcnt<30000 && ((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum)>75 && ((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum)<=30;
    cond412_2 = gmv_bgx == 0 && gmv_bgy == 0  && aplsum32rgn<3600000 && aplsum32rgn>2900000;
    cond412_3 = Dtl_array[7]<39000 && Dtl_array[7]>30000 && Dtl_array[6]<2000 && Dtl_array[3]<20000 && Dtl_array[4]<20000 &&rgn_apli[3]>180000 && rgn_apli[3]<230000 && rgn_apli[4]>180000 && rgn_apli[4]<230000;

    if(cond412_1 && cond412_2 && cond412_3)
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x2);
        //if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
        //{
            holdframe412 = 100;
        //}
       // else
        //{            
       //     holdframe412 = 50;
       // }
        pOutput->u8_dh_condition_412_bypass = 1;
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);
    }
    //else if(holdframe412>0)
    //{
    //    holdframe412--;
    //    WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x2);
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);
    // }
    else
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
        pOutput->u8_dh_condition_412_bypass = 0;
         //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x0);
    }
    //rtd_pr_memc_info("%d %d %d \n",cond412_1,cond412_2,cond412_3);
    //rtd_pr_memc_info("bg:%d apl:%d %d mv:%d %d %d\n",gmv_bgcnt,((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum),((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum),gmv_bgx,gmv_bgy,aplsum32rgn);
    //rtd_pr_memc_info("%d %d %d %d %d %d\n",Dtl_array[3],Dtl_array[4],Dtl_array[6],Dtl_array[7],rgn_apli[3],rgn_apli[4]);
    //-------------------- 374 ---------------------------//
    range_rmv_cnt_374 = 0;
    for(rgnIdx=0;rgnIdx<24;rgnIdx++)
    {
        if(rmvx[rgnIdx]<20 && rmvx[rgnIdx]>10 && _ABS_(rmvy[rgnIdx])<5)
            range_rmv_cnt_374++;
    }

    cond374_1 = rgn_bgcnt[5]<600 && rgn_bgcnt[6]<500 && rgn_bgcnt[7]<500 && rgn_bgcnt[22]==1020 && rgn_bgcnt[23]==1020;
    cond374_2 = Dtl_array[8]<500 && Dtl_array[15]>60000 && Dtl_array[23]>50000 && Dtl_array[7]>25000 && Dtl_array[7]<40000;
    cond374_3 = aplsum32rgn>3000000 && aplsum32rgn<5000000 && gmv_bgcnt>24000 && _ABS_(gmv_fgy)<10 && _ABS_(gmv_bgy)<5 && u11_gmv_mvx>=8;

    cond374_4 = rgn_aplcnt>=6 && rgn_apli[0]>150000 && rgn_apli[1]>150000  && rgn_bgcnt[6]<600 && rgn_bgcnt[7]<600 && rgn_bgcnt[14]<600 && rgn_bgcnt[15]<600;
    cond374_5 = Dtl_array[5]>35000 && Dtl_array[5]<45000 && Dtl_array[7]>25000 && Dtl_array[7]<36000 && Dtl_array[0]>12000 && Dtl_array[0]<20000 && Dtl_array[31]>15000 && Dtl_array[31]<30000;
    cond374_6 = _ABS_(gmv_fgy)<2 && _ABS_(gmv_bgy)<2 && gmv_bgx<20 && gmv_bgx>0 && gmv_fgx<=20 && gmv_fgx>0 && aplsum32rgn>3000000 && aplsum32rgn<5000000;

    //cond374_7 = Dtl_array[3]>12000 &&Dtl_array[3]<20000 && Dtl_array[7]>32000 &&Dtl_array[7]<42000 && Dtl_array[13]>55000 &&Dtl_array[13]<65000 && Dtl_array[31]>15000 &&Dtl_array[31]<26000;
    //cond374_8 = rgn_aplcnt>=6 && rgn_aplcnt2>=6 && rgn_apli[3]>200000 && rgn_apli[3]<250000;
    //cond374_9 = rgn_bgcnt[12]<600 && rgn_bgcnt[14]<600 && rgn_bgcnt[15]<600 && gmv_bgcnt>23000 && gmv_bgx<20 && gmv_fgx<20 &&_ABS_(gmv_fgy)<2 && _ABS_(gmv_bgy)<2;
    //cond374_7 = ((Dtl_array[3]>12000 &&Dtl_array[3]<20000 && Dtl_array[7]>32000 &&Dtl_array[7]<42000)||(Dtl_array[3]>28000 && Dtl_array[3]<31000 && Dtl_array[7]>14000 &&Dtl_array[7]<16000 && Dtl_array[1]>71000 && Dtl_array[1]<73000)) && Dtl_array[13]>55000 &&Dtl_array[13]<70000 && Dtl_array[31]>15000 &&Dtl_array[31]<26000;
    cond374_7 = ((Dtl_array[3]>12000 &&Dtl_array[3]<20000 && Dtl_array[7]>32000 &&Dtl_array[7]<42000)) && Dtl_array[13]>55000 &&Dtl_array[13]<70000 && Dtl_array[31]>15000 &&Dtl_array[31]<26000;
    cond374_8 = rgn_aplcnt>=6 && rgn_aplcnt2>=6 && rgn_apli[3]>180000 && rgn_apli[3]<250000;
    cond374_9 = ((rgn_bgcnt[12]<800 && rgn_bgcnt[14]<800 && rgn_bgcnt[15]<600) || (rgn_bgcnt[12]==1020 && rgn_bgcnt[14]==1020 && rgn_bgcnt[15]==1020))&& gmv_bgcnt>23000 && gmv_bgx<20 && gmv_fgx<20 &&_ABS_(gmv_fgy)<2 && _ABS_(gmv_bgy)<2;

    //cond374_10 = rgn_aplcnt3>=5 && rgn_apli[2]<40000 && rgn_apli[6]>200000 && rgn_apli[31]>140000 && rgn_apli[31]<170000 && rgn_apli[30]>110000 && rgn_apli[30]<150000;
    //cond374_11 = rgn_bgcnt[11]==1020 && rgn_bgcnt[12]>700 && rgn_bgcnt[16]>900 && rgn_bgcnt[24]>900  && rgn_bgcnt[12]<1020 && rgn_bgcnt[16]<1020 && rgn_bgcnt[24]<1020;
    //cond374_12 = Dtl_array[24]>12000 && Dtl_array[24]<20000 && Dtl_array[26]>15000 && Dtl_array[26]<22000 && Dtl_array[27]>15000 && Dtl_array[27]<22000 ; 
    cond374_10 = rgn_aplcnt3>=5 && rgn_apli[2]<45000 && rgn_apli[6]>200000 && rgn_apli[31]>140000 && rgn_apli[31]<170000 && rgn_apli[30]>110000 && rgn_apli[30]<170000;
    cond374_11 = rgn_bgcnt[11]>700 && rgn_bgcnt[12]>600 && rgn_bgcnt[16]>900 && rgn_bgcnt[24]>900;
    cond374_12 = Dtl_array[24]>12000 && Dtl_array[24]<28000 && Dtl_array[26]>15000 && Dtl_array[26]<27000 && Dtl_array[27]>15000 && Dtl_array[27]<26000 ; 

    cond374_13 = rgn_aplcnt>=5 && rgn_aplcnt3>=5 && rgn_apli[0]>80000 && rgn_apli[5]>25000 && aplsum32rgn>3000000 && aplsum32rgn<5000000 && gmv_bgcnt>23000 && _ABS_(gmv_fgy)<15 && _ABS_(gmv_bgy)<5 && _ABS_(gmv_bgy)<=2 && u11_gmv_mvx>=8 && u11_gmv_mvx<25;
    cond374_14 = range_rmv_cnt_374>18 && rgn_bgcnt[11]==1020 && rgn_bgcnt[12]>700 && rgn_bgcnt[16]>900 && rgn_bgcnt[24]>900 ;
    cond374_15 = Dtl_array[1]>30000 && Dtl_array[1]<60000  && Dtl_array[10]>30000 && Dtl_array[11]>25000 && Dtl_array[13]>30000 && Dtl_array[16]<20000 && Dtl_array[21]>15000;

    //cond374_sp = cond374_6 && cond374_8 && cond374_9 && rgn_bgcnt[16]==1020 && rgn_bgcnt[15]<400 && rgn_apli[0]<14000 && rgn_apli[0]>13000 && Dtl_array[15]>28000 && Dtl_array[15]<31000 && Dtl_array[24]>25000 && Dtl_array[24]<27000;
    cond374_sp = cond374_3 && cond374_13 && rgn_bgcnt[15]<400  && rgn_bgcnt[16]>1000 && rgn_apli[0]>85000 && rgn_apli[0]<100000 && Dtl_array[15]<48000 && Dtl_array[15]>45000 && Dtl_array[24]>23000 && Dtl_array[24]<25000;

    //rtd_pr_memc_info("%d %d %d\n",range_rmv_cnt_374,rgn_aplcnt,rgn_aplcnt3);

    if((cond374_1 && cond374_2 && cond374_3)||(cond374_4 && cond374_5 && cond374_6)||(cond374_6 && cond374_7 && cond374_8 && cond374_9)||(cond374_6 && cond374_10 && cond374_11 && cond374_12)||(cond374_3 && cond374_13 && cond374_14 && cond374_15)|| cond374_sp)
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x2);
        
        if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
        {
            holdframe374 = 400;
        }
        else
        {            
            holdframe374 = 200;//150
            }
            
            video374 =1 ;
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,150);  //dh
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,120);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1);

           // WriteRegister(KME_ME1_BG1_ME_BG_TH1_reg,0,7,140); //me1
          //  WriteRegister(KME_ME1_BG1_ME_BG_TH1_reg,8,15,255);
          //  WriteRegister(KME_ME1_BG1_ME_BG_TH0_reg,30,30,1);

            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,140); //fg_apl_s3_protect_th0
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,255);   //fg_apl_s3_protect_th1

            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);  //cor
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
        }
        else if(holdframe374>0)
        {
            holdframe374--;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x2);
            
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,150);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,120);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x1);

            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,140); //fg_apl_s3_protect_th0
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,255);   //fg_apl_s3_protect_th1
            
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,8,15,0);  
            WriteRegister(KME_ME1_BG1_ME_BG_RANGE_1_reg,24,31,0);
        }
        else if(video374)
        {
            video374 =0  ;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x0);
            
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,8,15,0xff);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,0,7,0x0);
            WriteRegister(KME_DEHALO5_PHMV_FIX_23_reg,20,20,0x0);

            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,16,23,0); //fg_apl_s3_protect_th0
            WriteRegister(KME_ME1_BG0_ME1_BG_11_reg,8,15,30);   //fg_apl_s3_protect_th1
    }
    //rtd_pr_memc_info("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \n",cond374_1,cond374_2,cond374_3,cond374_4,cond374_5,cond374_6,cond374_7,cond374_8,cond374_9,cond374_10,cond374_11,cond374_12,cond374_13,cond374_14,cond374_15,cond374_sp);
    //rtd_pr_memc_info("%d %d %d %d %d %d %d %d\n",rgn_aplcnt,rgn_apli[0],rgn_apli[1],rgn_bgcnt[5],rgn_bgcnt[6],rgn_bgcnt[7],rgn_bgcnt[14],rgn_bgcnt[15]);
    //rtd_pr_memc_info("%d %d %d %d\n", rgn_bgcnt[11],rgn_bgcnt[12],rgn_bgcnt[16],rgn_bgcnt[24]);
    //rtd_pr_memc_info("374:%d %d %d %d %d %d\n",(cond374_1 && cond374_2 && cond374_3),(cond374_4 && cond374_5 && cond374_6),(cond374_6 && cond374_7 && cond374_8 && cond374_9),(cond374_6 && cond374_10 && cond374_11 && cond374_12),(cond374_3 && cond374_13 && cond374_14 && cond374_15), cond374_sp);

    //---------------------  96  part1 --------------------------------------//
    cond96_aplcnt1 = 0 ;
    cond96_aplcnt2 = 0 ;
    cond96_mvcnt = 0;
    for(rgnIdx=0;rgnIdx<8;rgnIdx++)
    {
            //off545
        //if(rgn_apli[rgnIdx]>70000 && rgn_apli[rgnIdx]<130000)
         //   cond96_aplcnt1++;
        //if(rgn_apli[rgnIdx+8]>130000 && rgn_apli[rgnIdx+8]<210000)
        //    cond96_aplcnt2++;
        //if(rmvx[rgnIdx] == 0 && _ABS_(rmvy[rgnIdx])<10 && rmvy[rgnIdx]<=0)
        //    cond96_mvcnt++;
        if(rgn_apli[rgnIdx]>60000 && rgn_apli[rgnIdx]<130000)
            cond96_aplcnt1++;
        if(rgn_apli[rgnIdx+8]>120000 && rgn_apli[rgnIdx+8]<210000)
            cond96_aplcnt2++;
        if(_ABS_(rmvx[rgnIdx]) < 10 && _ABS_(rmvy[rgnIdx])<20 && rmvy[rgnIdx]<=0)
            cond96_mvcnt++;
    }
    //rtd_pr_memc_info("apl:%d %d mv:%d \n",cond96_aplcnt1,cond96_aplcnt2,cond96_mvcnt); 
    
    //off545
    cond96_1 = aplsum32rgn>3300000 && aplsum32rgn<3800000 && cond96_aplcnt1>=7 && cond96_aplcnt2 >=7;
    cond96_2 = rgn_apli[8]>160000 && Dtl_array[0]<30000 &&  Dtl_array[7]<35000 && Dtl_array[14]<35000 &&Dtl_array[28]>55000 && Dtl_array[28]<65000 && Dtl_array[29]>50000 && Dtl_array[29]<65000;
    cond96_3 = cond96_mvcnt >=7;

    //cond96_1 = aplsum32rgn>3600000 && aplsum32rgn<4400000 && cond96_aplcnt1>=7 && cond96_aplcnt2 >=6;
    //cond96_2 = rgn_apli[8]>160000 && Dtl_array[0]<40000 &&  Dtl_array[7]<40000 && Dtl_array[14]<45000 &&Dtl_array[28]>50000 && Dtl_array[28]<65000 && Dtl_array[29]>50000 && Dtl_array[29]<65000;
    //cond96_3 = cond96_mvcnt >=7;

    
    //rtd_pr_memc_info("%d %d %d cnt:%d %d %d\n",cond96_1,cond96_2,cond96_3,cond96_aplcnt1,cond96_aplcnt2,cond96_mvcnt);
    //rtd_pr_memc_info("apl:%d %d dtl:%d %d %d %d %d %d\n",aplsum32rgn,rgn_apli[8],Dtl_array[0],Dtl_array[7],Dtl_array[14],Dtl_array[28],Dtl_array[29],cond96_mvcnt);
    
    if(cond96_1 && cond96_2)
    {

    
        if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
        {
            holdframe96 = 160;
        }
        else
        {
            //ff545
            //holdframe96 = 60;
            holdframe96 = 80;
        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x3);
        pOutput->u8_dh_gmv_valchange_96 = 1;
    }
    else if(holdframe96>0)
    {
        holdframe96--;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x3);
        pOutput->u8_dh_gmv_valchange_96 = 1;
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x0);
        pOutput->u8_dh_gmv_valchange_96 = 0;
    }


   //----------------------   421 ---------------------------//
   //cond421_1 = aplsum32rgn<3000000 && aplsum32rgn>1000000 && rgn_apli[13]>80000 && rgn_apli[14]>100000;
   //cond421_2 = Dtl_array[0]>7000 && Dtl_array[0]<14000 &&  Dtl_array[6]>1000 && Dtl_array[6]<20000 && Dtl_array[8]<5000 && Dtl_array[15]<6000 ;
   //cond421_3 = _ABS_(rmvx[15])>50 && _ABS_(rmvy[15])<10 && rmvx[15]<0 && rmvy[15]<0;

   cond421_1 = aplsum32rgn<3000000 && aplsum32rgn>1000000 && rgn_apli[8]>80000 &&rgn_apli[13]>80000 && rgn_apli[14]>80000 && rgn_apli[26]<20000 && (rgn_apli[31]>45000 || rgn_apli[31]<30000);
   cond421_2 = Dtl_array[0]>7000 && Dtl_array[0]<48000 &&  Dtl_array[1]>10000  &&  Dtl_array[5]>10000 && Dtl_array[5]<25000 && Dtl_array[6]>10000 && Dtl_array[6]<25000  && Dtl_array[15]<20000 && Dtl_array[25]<29000 && Dtl_array[26]<23000  && Dtl_array[10]>2000 && Dtl_array[18]>2000;
   cond421_3 = _ABS_(rmvy[8])<=5  &&  _ABS_(rmvx[15])>=30 && _ABS_(rmvy[15])<10 && rmvx[15]<0 && _ABS_(rmvx[23])>=30 ;
   cond421_4 = aplsum32rgn<2650000 && Dtl_array[13]>1000 && Dtl_array[21]>2000 && Dtl_array[18]>1000;

   if(cond421_1 && cond421_2 && cond421_3 && cond421_4)
   {
       holdframe421 = 135;
       //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
       pOutput->u8_dh_gmv_valchange_421 = 1;
   }
   else if(holdframe421>0)
   {
       holdframe421--;
       //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
       pOutput->u8_dh_gmv_valchange_421 = 1;
   }
   else 
   {
       //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
       pOutput->u8_dh_gmv_valchange_421 = 0;
   }
    //rtd_pr_memc_info("apl:%d %d %d dtl:%d %d %d %d mv:%d %d\n",aplsum32rgn,rgn_apli[13],rgn_apli[14],Dtl_array[0],Dtl_array[6],Dtl_array[8],Dtl_array[15],rmvx[15],rmvy[15]);
    //rtd_pr_memc_info("%d %d %d %d\n",cond421_1,cond421_2,cond421_3,cond421_4);
        //++++++++++++++431 halo+++++++++++++++//

        bgcnt_sum_fisrtline = bgcnt_array[0]+bgcnt_array[1]+bgcnt_array[2]+bgcnt_array[3]+bgcnt_array[4]+bgcnt_array[5]+bgcnt_array[6]+bgcnt_array[7] ;
        if(gmv_bgcnt>25000&& gmv_bgcnt<30000&&_ABS_(u10_gmv_mvy)<2&&u11_gmv_mvx<-16&&u11_gmv_mvx>-50&&fg_apl_seg7_cnt<10&&bg_apl_seg7_cnt<50
            &&(((fg_apl_seg0_cnt+bg_apl_seg0_cnt)>4000&&(fg_apl_seg0_cnt+bg_apl_seg0_cnt)<6000)||((fg_apl_seg0_cnt+bg_apl_seg0_cnt)<1400))&&_ABS_(gmv_fgy)<2&&_ABS_(gmv_fgx)<7&&bgcnt_sum_fisrtline>=8150) 
        {
            pOutput->u8_me1_condition_431_halo_flag = 1 ;
        }
        else
        {
            pOutput->u8_me1_condition_431_halo_flag = 0 ;
        }

           
    //---------------------------------  219 -------------------------------//
    cond219_1 = aplsum32rgn>2200000 && aplsum32rgn<2600000 && rgn_apli[3]>85000 && rgn_apli[3]<105000 && rgn_apli[14]>55000 && rgn_apli[14]<70000 && rgn_apli[24]>70000 && rgn_apli[24]<100000;
    cond219_2 = Dtl_array[3]>80000 && Dtl_array[3]<120000 && Dtl_array[7]>50000 && Dtl_array[7]<70000 && Dtl_array[16]>18000 && Dtl_array[16]<40000 && Dtl_array[24]>15000 && Dtl_array[24]<30000 && Dtl_array[26]>50000 && Dtl_array[26]<70000; 
    cond219_3 = 1;
    if(cond219_1 && cond219_2 && cond219_3)
    {
        holdframe219 = 50;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
        pOutput->u8_dh_gmv_valchange_219 = 1;
    }
    else if(holdframe219>0)
    {
        holdframe219--;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
        pOutput->u8_dh_gmv_valchange_219 = 1;
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
        pOutput->u8_dh_gmv_valchange_219 = 0;
    }
    //rtd_pr_memc_info("%d %d flag:%d\n",cond219_1,cond219_2,pOutput->u8_dh_gmv_valchange_219);
    //rtd_pr_memc_info("apl:%d %d %d dtl:%d %d %d %d %d \n",rgn_apli[3],rgn_apli[14],rgn_apli[24],Dtl_array[3],Dtl_array[7],Dtl_array[16],Dtl_array[24],Dtl_array[26]);

    //------------------------------- 215 sudden broken -------------------------------------//
    //old
    //cond215_1 = gmv_bgcnt<20000 && gmv_bgx<-20 && gmv_bgy>10 && gmv_fgx<-30 && gmv_fgy<20 && aplsum32rgn>2000000 && aplsum32rgn<2500000;
    //cond215_2 = rgn_apli[10]>80000 && rgn_apli[10]<120000 && rgn_apli[16]>80000 && rgn_apli[10]<120000 && rgn_apli[22]>90000 && rgn_apli[22]<110000;
    //cond215_3 = Dtl_array[4]>10000 && Dtl_array[4]<20000 &&Dtl_array[5]>10000 && Dtl_array[5]<30000 && Dtl_array[24]>10000 && Dtl_array[24]<20000 && Dtl_array[28]>10000 && Dtl_array[28]<20000;

    cond215_1 = gmv_bgcnt<20000 && gmv_bgx<-20 && gmv_bgy>10 && gmv_fgx<-30 && gmv_fgy<20 && aplsum32rgn>2000000 && aplsum32rgn<2500000;
    cond215_2 = rgn_apli[10]>80000 && rgn_apli[10]<120000 && rgn_apli[16]>80000 && rgn_apli[10]<120000 && rgn_apli[22]>90000 && rgn_apli[22]<110000;
    cond215_3 = Dtl_array[4]>10000 && Dtl_array[4]<20000 &&Dtl_array[5]>10000 && Dtl_array[5]<30000 && Dtl_array[24]>10000 && Dtl_array[24]<20000 && Dtl_array[28]>10000 && Dtl_array[28]<20000;

    //rtd_pr_memc_info("%d %d %d\n",cond215_1,cond215_2,cond215_3);
    //rtd_pr_memc_info("\n",gmv_bgcnt,gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy,aplsum32rgn);       
    if(cond215_1 && cond215_2 && cond215_3)
    {
        holdframe215 = 40;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
    	pOutput->u8_dh_condition_215_bypass = 1;
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
    	pOutput->u8_dh_condition_215_bypass = 0;
    } 
	//djntest	
	//rtd_pr_memc_info("flag:%d %d %d %d %d %d %d\n",pOutput->u8_dh_condition_215_bypass,gmv_bgcnt,gmv_bgx,gmv_bgy, gmv_fgx,gmv_fgy,aplsum32rgn);
	//rtd_pr_memc_info("%d %d %d %d %d %d %d\n",rgn_apli[10],rgn_apli[16],rgn_apli[22],Dtl_array[4],Dtl_array[5],Dtl_array[24],Dtl_array[28]);	





    //------------------------------- 227 gmv_change 1-------------------------------------//
    cond227_1 = Dtl_array[2]>20000 && Dtl_array[1]<40000  && Dtl_array[7]>15000 && Dtl_array[7]<35000 && Dtl_array[8]<25000 && Dtl_array[9]<20000 && Dtl_array[14]>30000 && Dtl_array[14]>40000 &&Dtl_array[16]<15000 &&  Dtl_array[23]>55000 && Dtl_array[24]<25000 ; 
    cond227_2 = rgn_apli[0]<20000 && rgn_apli[8]<15000 && rgn_apli[13]>80000 && rgn_apli[15]>90000 && rgn_apli[22]>80000 && rgn_apli[22]<140000 && rgn_apli[31]>35000 && rgn_apli[31]<60000;    
    if(cond227_1 && cond227_2)
    {
        holdframe227_0 = 0;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16,19, 0x1);
        pOutput->u8_dh_gmv_valchange_227_1=1;
        pOutput->u8_dh_condition_2271_bypass=1;
    }
    else if(holdframe227_0>0)
    {
        holdframe227_0--;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x1);
        pOutput->u8_dh_gmv_valchange_227_1=1;
        pOutput->u8_dh_condition_2271_bypass=1;
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x0);
        pOutput->u8_dh_gmv_valchange_227_1=0;
        pOutput->u8_dh_condition_2271_bypass=0;
    }        
    //rtd_pr_memc_info("1:%d %d\n",cond227_1,cond227_2);
//545
//    cond227_11 = Dtl_array[2]>45000 && Dtl_array[2]<65000 && Dtl_array[13]>75000 && Dtl_array[13]<100000 && Dtl_array[15]>80000 && Dtl_array[24]>10000 && Dtl_array[24]<20000 && Dtl_array[26]>15000 && Dtl_array[26]<25000 ; 
//    cond227_12 = rgn_apli[1]>40000 && rgn_apli[1]<55000 && rgn_apli[2]>50000 && rgn_apli[2]<60000 && rgn_apli[8]>100000 && rgn_apli[8]<130000 && rgn_apli[15]>9000 && rgn_apli[15]<120000 && rgn_apli[23]>50000 && rgn_apli[23]<70000 ;
//    cond227_13 = rmvx[10]>120 && rmvx[11]>100;

    
    cond227_11 = Dtl_array[2]>25000 && Dtl_array[2]<85000 && Dtl_array[13]>55000 && Dtl_array[13]<120000 && Dtl_array[15]>60000 && Dtl_array[24]>2000 && Dtl_array[24]<40000 && Dtl_array[26]>2000 && Dtl_array[26]<45000 ; 
    cond227_12 = rgn_apli[1]>40000 && rgn_apli[1]<77000 && rgn_apli[2]>50000 && rgn_apli[2]<80000 && rgn_apli[8]>120000 && rgn_apli[8]<160000 && rgn_apli[15]>100000 && rgn_apli[15]<160000 && rgn_apli[23]>60000 && rgn_apli[23]<110000 ;
    cond227_13 = rmvx[10]>130 && rmvx[11]>120;   
    if(cond227_11 && cond227_12 && cond227_13)
    {
        if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
        { 
            holdframe227_1 = 140;
        }
        else
        {
            holdframe227_1 = 50;
        }
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x2);
        pOutput->u8_dh_gmv_valchange_227_2=1;
    }
    else if(holdframe227_1>0)
    {
        holdframe227_1--;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x2);
        pOutput->u8_dh_gmv_valchange_227_2=1;
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x0);
        pOutput->u8_dh_gmv_valchange_227_2=0;
    }
    //rtd_pr_memc_info("2272:%d %d %d mv:%d %d\n",cond227_11,cond227_12,cond227_13,rmvx[10],rmvx[11]);
    //rtd_pr_memc_info("%d %d %d %d %d\n",rgn_apli[1],rgn_apli[2],rgn_apli[8],rgn_apli[15],rgn_apli[23]);

//off545
//    cond227_21 = Dtl_array[15]>100000 && Dtl_array[15]<120000 && Dtl_array[24]<20000 && Dtl_array[25]<25000 && Dtl_array[23]>90000;
//    cond227_22 = rgn_apli[6]>70000 && rgn_apli[7]>90000 && rgn_apli[24]<15000 && rgn_apli[25]<15000 && rgn_apli[31]<25000;
//    cond227_23 =  rmvx[16]>90 && rmvx[17]>80 && rmvx[18]>70 && rmvx[23]<25;    

    cond227_21 = Dtl_array[15]>90000 && Dtl_array[15]<130000 && Dtl_array[24]<30000 && Dtl_array[25]<35000 && Dtl_array[23]>80000;
    cond227_22 = rgn_apli[6]>70000 && rgn_apli[7]>90000 && rgn_apli[24]<25000 && rgn_apli[25]<25000 && rgn_apli[31]<35000;
    cond227_23 =  rmvx[16]>80 && rmvx[17]>70 && rmvx[18]>60 && rmvx[23]<35;    

    if(cond227_21 && cond227_22 && cond227_23)
    {
        holdframe227_2 = 20;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
        pOutput->u8_dh_gmv_valchange_227_3=1;
    }
    else if(holdframe227_2>0)
    {
        holdframe227_2--;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
        pOutput->u8_dh_gmv_valchange_227_3=1;
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
        pOutput->u8_dh_gmv_valchange_227_3=0;
    }
    //rtd_pr_memc_info("3:%d %d %d \n",cond227_21,cond227_22,cond227_23);


    cond227_31 = Dtl_array[0]>5000 && Dtl_array[0]<25000 && Dtl_array[3]>25000 && Dtl_array[3]<35000 && Dtl_array[7]>20000 && Dtl_array[7]<35000 && Dtl_array[27]>15000 && Dtl_array[27]<25000;
    cond227_32 = rgn_apli[7]>90000 && rgn_apli[7]<130000 && rgn_apli[15]>120000 && rgn_apli[23]>80000 && rgn_apli[24]<15000 && rgn_apli[25]<20000 ;
    cond227_33 = rmvx[16]>150 && rmvx[17]>130 ;


    
    if(cond227_31 && cond227_32 && cond227_33)
    {
        holdframe227_3 = 30;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x4);
        pOutput->u8_dh_gmv_valchange_227_4=1;
    }
    else if(holdframe227_3>0)
    {
        holdframe227_3--;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x4);
        pOutput->u8_dh_gmv_valchange_227_4=1;
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x0);
        pOutput->u8_dh_gmv_valchange_227_4=0;
    }
    

    cond227_41 = rgn_apli[0]>10000 && rgn_apli[0]<50000 && rgn_apli[2]>30000 && rgn_apli[2]<70000 &&rgn_apli[15]>200000 && rgn_apli[23]>55000 && rgn_apli[23]<90000 && rgn_apli[24]<15000 && rgn_apli[31]>10000 && rgn_apli[31]<35000;
    cond227_42 = rgn_apli[21]>50000;
    cond227_43 = cond227_22 && cond227_23;

    if(cond227_41 && cond227_42 && cond227_43)
    {
        holdframe227_4 = 20;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x5);
        pOutput->u8_dh_gmv_valchange_227_5=1;
    }
    else if(holdframe227_4>0)
    {
        holdframe227_4--;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x5);
        pOutput->u8_dh_gmv_valchange_227_5=1;
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x0);
        pOutput->u8_dh_gmv_valchange_227_5=0;
    }
    
    //rtd_pr_memc_info("5:%d %d %d mv:%d %d %d %d\n",cond227_41,cond227_42,cond227_43,rmvx[16],rmvx[17],rmvx[18],rmvx[23]);
    //rtd_pr_memc_info("51:%d %d %d %d %d %d %d %d %d\n",rgn_apli[0],rgn_apli[2],rgn_apli[15],rgn_apli[23],rgn_apli[24],rgn_apli[31], rgn_apli[21],cond227_22,cond227_23);

    //------------------------- 96_part3 --------------------------------------//
    cnt_dtlarray_96 = 0;
    cnt_aplarray_96 = 0;
    for(rgnIdx=0;rgnIdx<8;rgnIdx++)
    {
        if(Dtl_array[rgnIdx]>10000 && Dtl_array[rgnIdx]<20000)
            cnt_dtlarray_96++; 
        if(rgn_apli[rgnIdx+8]>120000 && rgn_apli[rgnIdx+8]<200000)
            cnt_aplarray_96++;
    }
    cond963_1 = gmv_bgx<-15 && gmv_bgy == 0 && _ABS_(gmv_fgy)<5 && aplsum32rgn>3500000 && aplsum32rgn<4500000;
    cond963_2 = cnt_dtlarray_96>=6 && Dtl_array[30]>15000; 
    cond963_3 = cnt_aplarray_96>=6 && rgn_apli[0]>90000 && rgn_apli[0]<110000;

        if(cond963_1 && cond963_2 && cond963_3)
        {
            holdframe96_3 = 30;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x5);
            pOutput->u8_me1_condition_96_2_flag = 1;
        }
        else if(holdframe96_3>0)
        {
            holdframe96_3--;
            pOutput->u8_me1_condition_96_2_flag = 1;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x5);
        }
        else 
        {
            pOutput->u8_me1_condition_96_2_flag = 0;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x0);
        }
        //rtd_pr_memc_info("963:%d %d %d\n",cond963_1,cond963_2,cond963_3);

        //---------------------  156_final ----------------------------//
        //cond1562_mv = u11_gmv_mvx>=35 && u11_gmv_mvx<40 && gmv_fgx>25 && gmv_fgx<30 && _ABS_(u10_gmv_mvy)<=2 && gmv_bgcnt<20000;
        //cond1562_apl = rgn_aplp[0]>34000 && rgn_aplp[0]<42000 && rgn_aplp[24]<12000 && rgn_aplp[3]>90000 && rgn_aplp[3]<125000;
        //cond1562_dtl = Dtl_array[3]>25000 && Dtl_array[3]<35000 && Dtl_array[6]>35000 && Dtl_array[6]<45000 && Dtl_array[31]<20000;
        
        cond1562_mv = u11_gmv_mvx>0 && gmv_bgx>=5 && gmv_bgy<=0 && gmv_fgy<10 && _ABS_(u10_gmv_mvy)<=2 && gmv_bgcnt<21000 && aplsum32rgn>2300000 && aplsum32rgn<2600000;
        cond1562_apl = rgn_aplp[0]>34000 && rgn_aplp[0]<52000 && rgn_aplp[24]<15000 && rgn_aplp[3]>90000 && rgn_aplp[3]<125000;
        cond1562_dtl = Dtl_array[3]>25000 && Dtl_array[3]<45000 && Dtl_array[6]>35000 && Dtl_array[6]<45000 && Dtl_array[31]<20000;
        if(cond1562_mv && cond1562_dtl && cond1562_apl)
        {
            pOutput->u8_me1_condition_156_final_flag = 1;
        }
        else 
        {
            pOutput->u8_me1_condition_156_final_flag = 0;
        }
        //rtd_pr_memc_info("156:%d %d %d\n",cond1562_mv,cond1562_dtl,cond1562_apl);

     //------------------------ 49 --------------------------------//
     //off545
     //   cond049_1 = gmv_bgcnt<26000 && aplsum32rgn>2500000 && aplsum32rgn<3100000 && _ABS_(u10_gmv_mvy)<15 && gmv_bgx>=15 && _ABS_(gmv_bgy)<=10;
     //  cond049_2 = rgn_apli[0]>50000 && rgn_apli[1]>60000 && rgn_apli[1]<130000 && rgn_apli[2]>70000 && rgn_apli[2]<130000 && rgn_apli[9]>65000 && rgn_apli[9]<150000 && rgn_apli[21]>60000 && rgn_apli[21]<150000 && rgn_apli[26]>70000 && rgn_apli[27]>90000 && rgn_apli[31]>65000;
     //  cond049_3 = Dtl_array[0]>20000 && Dtl_array[2]>15000 && Dtl_array[2]<50000 && Dtl_array[4]>20000 && Dtl_array[4]<40000 && Dtl_array[9]<25000 && Dtl_array[14]<25000 && Dtl_array[17]<25000; 

        cond049_1 = gmv_bgcnt<26000 && aplsum32rgn>2100000 && aplsum32rgn<2600000 && _ABS_(u10_gmv_mvy)<15 && gmv_bgx>=15 && _ABS_(gmv_bgy)<=10;
        cond049_2 = rgn_apli[0]>10000 && rgn_apli[1]>20000 && rgn_apli[1]<120000 && rgn_apli[2]>40000 && rgn_apli[2]<120000 && rgn_apli[9]>30000 && rgn_apli[9]<130000 && rgn_apli[21]>40000 && rgn_apli[21]<120000 && rgn_apli[26]>60000 && rgn_apli[27]>80000 && rgn_apli[31]>50000;
        cond049_3 = Dtl_array[0]>5000 && Dtl_array[2]>10000 && Dtl_array[2]<60000 && Dtl_array[4]>20000 && Dtl_array[4]<50000 && Dtl_array[9]<30000 && Dtl_array[14]<35000 && Dtl_array[17]<30000; 

        if(cond049_1 && cond049_2 && cond049_3)
        {
            //holdframe049 = 80;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x2);
            pOutput->u8_me1_condition_49_halo_flag = 1;
        }
        else 
        {
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x0);
            pOutput->u8_me1_condition_49_halo_flag = 0 ;
        }
        //rtd_pr_memc_info("%d %d %d\n",cond049_1,cond049_2,cond049_3);
        //rtd_pr_memc_info("mv:%d %d %d %d %d   apl:%d %d %d %d %d %d %d %d\n",gmv_bgcnt,aplsum32rgn,u10_gmv_mvy,gmv_bgx,gmv_bgy,rgn_apli[0],rgn_apli[1],rgn_apli[2],rgn_apli[9],rgn_apli[21],rgn_apli[26],rgn_apli[27],rgn_apli[31]);
        //rtd_pr_memc_info("dtl:%d %d %d %d %d %d\n",Dtl_array[0],Dtl_array[2],Dtl_array[4],Dtl_array[9],Dtl_array[14],Dtl_array[17]);
        //rtd_pr_memc_info("apl26271:%d %d %d\n",rgn_apli[26],rgn_apli[27],rgn_apli[31]);

        //-------------------- 163 -------------------------------//
        cond163_1 = rmvx[22]>70 && rmvx[23]>70 && rmvx[24]>70;
        cond163_2 = rgn_apli[0]>165000 && rgn_apli[0]<185000 && rgn_apli[1]>165000 && rgn_apli[1]<190000 && rgn_apli[2]>160000 && rgn_apli[16]>175000 && rgn_apli[16]<210000;
        cond163_3 = Dtl_array[0]>45000 && Dtl_array[0]<65000 && Dtl_array[17]>28000 && Dtl_array[17]<40000 && Dtl_array[24]>15000 && Dtl_array[24]<25000 && Dtl_array[28]<15000 && Dtl_array[30]<25000 && Dtl_array[31]<15000; 

        if(cond163_1 && cond163_2 && cond163_3)
        {
            pOutput->u8_dh_gmv_valchange_163 = 1;
        }
        else 
        {
            pOutput->u8_dh_gmv_valchange_163 = 0;
        }
        //rtd_pr_memc_info("mv:%d %d %d apl:%d %d %d %d dtl:%d %d %d %d %d %d\n",rmvx[22],rmvx[23],rmvx[24],rgn_apli[0],rgn_apli[1],rgn_apli[2],rgn_apli[16],Dtl_array[0],Dtl_array[17],Dtl_array[24],Dtl_array[28],Dtl_array[30],Dtl_array[31]);
        //rtd_pr_memc_info("163:%d %d %d\n",cond163_1,cond163_2,cond163_3);
        

    //-------------------------  432 final gmv -------------------------------//
    for(rgnIdx=0;rgnIdx<8;rgnIdx++)
    {
        if(rgn_apli[rgnIdx]>150000 && rgn_apli[rgnIdx]<230000)
            con432_aplcnt++;
    }

    cond432final_1 = ((rmvx[16]+50) < 0) && ((rmvx[23]+60) < 0) && ((rmvx[31]+60) < 0) ;
    cond432final_2 = Dtl_array[6]<200 && Dtl_array[7]<100 && Dtl_array[15]<25000 && Dtl_array[21]>5000 && Dtl_array[21]<30000 && Dtl_array[25]>25000 && Dtl_array[25]<45000;
    cond432final_3 = con432_aplcnt>6 && rgn_apli[31]>40000 && rgn_apli[31]<60000;
    if(cond432final_1 && cond432final_2 && cond432final_3)
    {
        pOutput->u8_dh_gmv_valchange_432final = 1;
    }
    else
    {
        pOutput->u8_dh_gmv_valchange_432final = 0;
    }



    //-------------------------  038 15s  -------------------------------//
    for(rgnIdx=0;rgnIdx<8;rgnIdx++)
    {
        if(rgn_apli[rgnIdx]<150000)
            cond38_aplcnt1++;        
        if(rgn_apli[rgnIdx+8]>100000)
            cond38_aplcnt2++;
        if(rgn_apli[rgnIdx+16]>140000)
            cond38_aplcnt3++;
        if(rgn_apli[rgnIdx+24]>100000)
            cond38_aplcnt4++;

        
        if(rgn_apli[rgnIdx]<100000)
            cond38_aplcnt5++;
        if(rgn_apli[rgnIdx+8]>120000)
            cond38_aplcnt6++;
        if(rgn_apli[rgnIdx+24]>180000 && rgn_apli[rgnIdx+24]<230000)
        //if(rgn_apli[rgnIdx+24]>180000)
            cond38_aplcnt7++;

        
        
    }
    
    //cond38_15s1 = rmvx[0]>15 && ((gmv_bgx>250)||(gmv_bgx==0 && gmv_bgy==0 && gmv_fgx>=35 && gmv_fgy==0)); 
    cond38_15s1 = (rmvx[0]>15 && ((gmv_bgx>250)||(gmv_bgx==0 && gmv_bgy==0 && gmv_fgx>=35 && gmv_fgy==0)))||(rmvx[0]>200 && _ABS_(gmv_bgx)<10 && gmv_bgx>15 && gmv_fgx >20);
    cond38_15s2 = Dtl_array[2]<70000 && Dtl_array[2]>50000 && Dtl_array[9]<35000 && Dtl_array[9]>10000 && Dtl_array[24]<15000 && Dtl_array[25]<15000 && Dtl_array[30]<15000 && Dtl_array[31]<15000;  
    cond38_15s3 = cond38_aplcnt1>=6 && cond38_aplcnt2>6 && cond38_aplcnt3>6 && cond38_aplcnt4>6;
    if(cond38_15s1 && cond38_15s2 && cond38_15s3)
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x2);
        holdframe38_15s = 120;
        pOutput->u8_dh_condition_38flag1=1;
    }
    else if(holdframe38_15s>0)
    {
        pOutput->u8_dh_condition_38flag1=1;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x2);
        holdframe38_15s--;
    }
    else
    {
        pOutput->u8_dh_condition_38flag1=0;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
    }
    //rtd_pr_memc_info("1:%d %d %d apl1:%d %d %d %d \n",cond38_15s1,cond38_15s2,cond38_15s3,cond38_aplcnt1,cond38_aplcnt2,cond38_aplcnt3,cond38_aplcnt4);
    //rtd_pr_memc_info("mv:%d %d %d %d %d\n",rmvx[0],gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy);
    //rtd_pr_memc_info("%d %d %d\n",cond38_15s1,cond38_15s2,cond38_15s3);

    
    cond38_45s1 = aplsum32rgn>4100000 && aplsum32rgn<5200000 && (gmv_bgx<=-45 || gmv_fgx<=-45) && _ABS_(gmv_bgy)<=5 &&  _ABS_(gmv_fgy)<=5 && rmvx[1]<=-45 && rmvx[1]>-180 && rmvx[31]<=-45 && rmvx[30]<=-40;
    cond38_45s2 = Dtl_array[6]>35000 && Dtl_array[6]<55000  && Dtl_array[16]>15000 && Dtl_array[16]<35000 && Dtl_array[24]<25000 && Dtl_array[30]<15000 && Dtl_array[31]<15000   && Dtl_array[23]>10000 && Dtl_array[23]<35000 && Dtl_array[27]>10000 && Dtl_array[27]<30000 && Dtl_array[25]<30000; 
    cond38_45s3 = cond38_aplcnt5>6 && cond38_aplcnt6>6 && cond38_aplcnt3>=6 && cond38_aplcnt7>=6 && rgn_apli[25]>190000 && rgn_apli[25]<220000 && rgn_apli[26]>190000 && rgn_apli[26]<220000;

    if(cond38_45s1 && cond38_45s2 && cond38_45s3)
    {        
	    //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x2);
        holdframe38_45s = 140; //150
        pOutput->u8_dh_condition_38flag2=1;

    }
    else if(holdframe38_45s>0)
    {
        pOutput->u8_dh_condition_38flag2=1;
	    //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x2);
        holdframe38_45s--;
    }
    else
    {
        pOutput->u8_dh_condition_38flag2=0;
	    //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x0);
    }
    //rtd_pr_memc_info("2:%d %d %d\n",cond38_45s1,cond38_45s2,cond38_45s3);
    //rtd_pr_memc_info("apl:%d mv:%d %d %d %d %d %d\n",aplsum32rgn,gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy,rmvx[1],rmvx[31]);
    //rtd_pr_memc_info("dtl:%d %d %d %d %d \n",Dtl_array[6],Dtl_array[16],Dtl_array[24],Dtl_array[30],Dtl_array[31]);    
    //rtd_pr_memc_info("cond1:%d %d %d %d %d %d %d\n",gmv_bgx,gmv_fgx,gmv_bgy,gmv_fgy,rmvx[1],rmvx[30],rmvx[31]);
    //rtd_pr_memc_info("cond2:%d %d %d %d %d %d %d %d\n",Dtl_array[6],Dtl_array[16],Dtl_array[24],Dtl_array[30],Dtl_array[31],Dtl_array[23],Dtl_array[27],Dtl_array[25]);
    //rtd_pr_memc_info("cond3:%d %d %d %d %d %d\n",cond38_aplcnt5,cond38_aplcnt6,cond38_aplcnt3,cond38_aplcnt7,rgn_apli[25],rgn_apli[26]);

    //10.14
    //cond214_mv = gmv_bgx>40 && gmv_bgx<70 && gmv_bgy<0 && gmv_bgy>-20 && _ABS_(gmv_fgx)<20 && _ABS_(gmv_fgy)<20 && rmvx[24]>45 && rmvx[31]>40;
    //cond214_dtl = Dtl_array[6]>45000 && Dtl_array[6]<65000 && Dtl_array[7]>40000 && Dtl_array[7]<60000 && Dtl_array[15]>40000 && Dtl_array[15]<65000 && Dtl_array[31]<30000 ; 
    //cond214_apl = aplsum32rgn>1400000 && aplsum32rgn<2800000 && ((rgn_apli[7]>70000 && rgn_apli[7]<100000)||(rgn_apli[7]>40000 && rgn_apli[7]<65000) )&& rgn_apli[24]<15000 && rgn_apli[25]<30000 && ((rgn_apli[31]>110000 && rgn_apli[31]<155000) || (rgn_apli[31]>60000 && rgn_apli[31]<80000)); 
    //new
    cond214_mv = gmv_bgx>40 && gmv_bgx<75 && gmv_bgy<0 && gmv_bgy>-20 && _ABS_(gmv_fgx)<20 && _ABS_(gmv_fgy)<20 && ((rmvx[24]>45 && rmvx[31]>40)||(rmvx[24]==0 && rmvx[31]==0));
    cond214_dtl = Dtl_array[6]>35000 && Dtl_array[6]<65000 && Dtl_array[7]>35000 && Dtl_array[7]<60000 && Dtl_array[15]>40000 && Dtl_array[15]<65000 && Dtl_array[31]<30000 ; 
    cond214_apl = aplsum32rgn>1400000 && aplsum32rgn<2800000 && ((rgn_apli[7]>70000 && rgn_apli[7]<100000)||(rgn_apli[7]>38000 && rgn_apli[7]<65000) )&& rgn_apli[24]<15000 && rgn_apli[25]<30000 && ((rgn_apli[31]>110000 && rgn_apli[31]<155000) || (rgn_apli[31]>50000 && rgn_apli[31]<80000)); 
    
    pOutput->u8_dh_condition_214=0;
    if(cond214_mv && cond214_dtl && cond214_apl)
    {        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
        pOutput->u8_dh_condition_214=1;
        video214 = 1;

    }
    else if(video214)
    {
        pOutput->u8_dh_condition_214=0;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
        video214 = 0;
    }
    //rtd_pr_memc_info("214:%d %d %d\n",cond214_mv , cond214_dtl , cond214_apl);
    //rtd_pr_memc_info("mv:%d %d %d %d %d %d dtl:%d %d %d %d apl:%d %d %d %d %d\n",gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy,rmvx[24],rmvx[31],Dtl_array[6],Dtl_array[7],Dtl_array[15],Dtl_array[31],aplsum32rgn,rgn_apli[7],rgn_apli[24],rgn_apli[25],rgn_apli[31]);



    //-------------------------  410 Sc garbage ----------------------------//
    cond410SC_apl = _ABS_(rgn_apli[0]-rgn_aplp[0])>120000 && _ABS_(rgn_apli[1]-rgn_aplp[1])>55000 && _ABS_(rgn_apli[8]-rgn_aplp[8])>60000 && _ABS_(rgn_apli[12]-rgn_aplp[12])>80000;
    
    if( cond410SC_apl )
    {        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);
        pOutput->u8_dh_condition_410_scgarbage = 1;
        path = 1;

    }
    else
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x0);
        pOutput->u8_dh_condition_410_scgarbage = 0;
        path = 0;
    }


    cond393SC_apl = _ABS_(rgn_apli[11]-rgn_aplp[11])>60000 && _ABS_(rgn_apli[12]-rgn_aplp[12])>35000 && _ABS_(rgn_apli[13]-rgn_aplp[13])>40000 
    && _ABS_(rgn_apli[18]-rgn_aplp[18])>50000 && _ABS_(rgn_apli[19]-rgn_aplp[19])>80000 && _ABS_(rgn_apli[20]-rgn_aplp[20])>45000;
    if( cond393SC_apl )
    {        
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);
        pOutput->u8_dh_condition_393_scgarbage = 1;
        path = 2;
    
    }
    else
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x0);
        pOutput->u8_dh_condition_393_scgarbage = 0;
        path = 0;
    }
    //rtd_pr_memc_info("flag:%d apl:%d %d %d %d %d %d\n",pOutput->u8_dh_condition_393_scgarbage,rgn_apli[11],rgn_apli[12],rgn_apli[13],rgn_apli[18],rgn_apli[19],rgn_apli[20]);

    cond182_mv = gmv_bgx>40 && gmv_bgx<75 && gmv_fgx>80 && aplsum32rgn>2300000 && aplsum32rgn<3300000;
    cond182_dtl = Dtl_array[0]>40000 && Dtl_array[6]<15000 && Dtl_array[7]<12000 && Dtl_array[24]<28000;
    cond182_apl = rgn_apli[7]>140000 && rgn_apli[7]<155000 && rgn_apli[15]>144000 && rgn_apli[15]<165000 && rgn_apli[21]>70000 && rgn_apli[21]<90000 && rgn_apli[24]<20000 && rgn_apli[28]<25000;
    if(cond182_mv && cond182_dtl && cond182_apl)
    {       
        pOutput->u8_dh_condition_182_bypass = 1;
        path = 3;
    }
    else
    {
        pOutput->u8_dh_condition_182_bypass = 0;
        path = 0;
    }

    //rtd_pr_memc_info("182:%d %d %d\n",cond182_mv , cond182_dtl , cond182_apl);
    //rtd_pr_memc_info("mv:%d %d %d dtl:%d %d %d %d apl:%d %d %d %d %d\n",gmv_bgx,gmv_fgx,aplsum32rgn,Dtl_array[0],Dtl_array[6],Dtl_array[7],Dtl_array[24], rgn_apli[7], rgn_apli[15], rgn_apli[21], rgn_apli[24], rgn_apli[28]);

    if(u11_gmv_mvx>-32&&u11_gmv_mvx<0&&u10_gmv_mvy>22&&gmv_bgcnt<16000&&fgseg0ratio<18&&_ABS_(gmv_fgy)>=10&&rgn_apli[8]>110000&&rgn_apli[9]>120000&&rgn_apli[9]>120000&&rgn_apli[11]<80000)
    {        
        pOutput->u8_dh_condition_343_bypass=1;
        path = 4;
    }
    else
    {
        pOutput->u8_dh_condition_343_bypass=0;	
        path = 0;
    }

    //------------------------------- 416 -------------------------------------//
    cond412_aplcnt = (bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/bg_apl_sum>600 && (fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/fg_apl_sum>600 && 
        (bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/bg_apl_sum<50 && (fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/fg_apl_sum<50;
    cond412_mv =  _ABS_(gmv_bgx)<15 && _ABS_(gmv_bgy)<5 && _ABS_(gmv_fgy)<10 && aplsum32rgn<1500000 && aplsum32rgn>500000;
    cond412_dtl = Dtl_array[7]>7000 && Dtl_array[7]<8500 && Dtl_array[15]<2000 && Dtl_array[22]<4000&& Dtl_array[23]<2000 && Dtl_array[15]<5000;
    cond412_apl = rgn_apli[24]<10000 && rgn_apli[15]>28000 && rgn_apli[11]>40000 && rgn_apli[28]>15000 && rgn_apli[28]<35000;


    if(cond412_mv && cond412_dtl && cond412_apl && cond412_aplcnt)
    {       
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);
        pOutput->u8_dh_condition_416_bypass =1;
        path = 5;
    }
    else 
    {
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
        //WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x0);
        pOutput->u8_dh_condition_416_bypass = 0;
        path = 0;
    }
    //rtd_pr_memc_info("182:%d %d %d %d\n",cond412_mv,cond412_dtl,cond412_apl,cond412_aplcnt);

    //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, path);


    //----------------------- PQ_Issue ------------------------------//
    PQIssue_mv = gmv_bgx<15 && gmv_bgx>=0 && gmv_bgy==0 && gmv_fgx<=15 && gmv_fgx>=0 && _ABS_(gmv_fgy)<=10 && aplsum32rgn>1100000 && aplsum32rgn<1550000;
    PQIssue_dtl = Dtl_array[0]>35000 && Dtl_array[0]<65000 && Dtl_array[23]<6000 && Dtl_array[29]>10000 && Dtl_array[29]<25000 &&Dtl_array[16]>50000 && Dtl_array[16]<85000 && Dtl_array[31]>15000 && Dtl_array[31]<30000;
    PQIssue_apl = rgn_apli[0]>10000 && rgn_apli[0]<25000 && rgn_apli[7]>34000 && rgn_apli[7]<60000 && rgn_apli[29]<10000 && rgn_apli[23]<8000 && rgn_apli[21]<10000 ;
 
    if(PQIssue_mv && PQIssue_dtl && PQIssue_apl)
    {
        holdframePQ = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 60: 30  ;  
        videoPQ = 1;
        pOutput->u8_dh_condition_PQ_Issue = 1;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);

    }
    //else if(holdframePQ>0)
    //{
    //    holdframePQ--;
    //    pOutput->u8_dh_condition_PQ_Issue = 1;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
    // }
    else if(videoPQ)
    {
        videoPQ = 0;
        pOutput->u8_dh_condition_PQ_Issue = 0;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
    }
    //rtd_pr_memc_info("PQ:%d %d %d\n",PQIssue_mv , PQIssue_dtl , PQIssue_apl);
    //rtd_pr_memc_info("dtl:%d %d %d %d %d\n",Dtl_array[0],Dtl_array[23],Dtl_array[29],Dtl_array[16],Dtl_array[31]);
    //rtd_pr_memc_info("PQ:%d %d %d mv:%d %d %d %d %d\n",PQIssue_mv , PQIssue_dtl , PQIssue_apl,gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy,aplsum32rgn);
    //rtd_pr_memc_info("dtl:%d %d %d %d %d\n",Dtl_array[0],Dtl_array[23],Dtl_array[29],Dtl_array[16],Dtl_array[31]);
    //rtd_pr_memc_info("apl:%d %d %d %d %d\n",rgn_apli[0],rgn_apli[7],rgn_apli[29],rgn_apli[23],rgn_apli[21]);


    

     /////////////////--------------------------rim 02-----------------////////////////////    
     //val =(u32_RB_val >> 24) & 0x3f;
     right_rim_pre=s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_RHT];
     avr_mv_x=_ABS_(rgnmvx_bg[7]+rgnmvx_bg[15]+rgnmvx_bg[23]+rgnmvx_bg[31])/4;
     if(avr_mv_x>80)   //02 rim
     {
         WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-14);//right  14 to 30
         rim_hold_cnt_right_02=6;
         video002 = 1;
         //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
     }
     else if(rim_hold_cnt_right_02>0)
     {
         WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-14);//right  
         rim_hold_cnt_right_02--;
         video002 = 1;
         //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
     }
     else if(video002>0) 
     {            
         WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre);//right 
         video002 = 0;
         //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
     }
     //rtd_pr_memc_info("mv:%d %d %d %d %d\n",rgnmvx_bg[7],rgnmvx_bg[15],rgnmvx_bg[23],rgnmvx_bg[31],avr_mv_x);
     
     
    min_array1=rgn_bgcnt[10];
    min_array2=rgn_bgcnt[18];
    
     for(rgnIdx=10;rgnIdx<=12;rgnIdx++)
     {
         if(rgn_bgcnt[rgnIdx]<min_array1)
         {
             min_array1=rgn_bgcnt[rgnIdx];
         }
     }  
     for(rgnIdx=18;rgnIdx<=20;rgnIdx++)
     {
         if(rgn_bgcnt[rgnIdx]<min_array2)
         {
             min_array2=rgn_bgcnt[rgnIdx];
         }
     }  
     min_array=min_array1>min_array2 ? min_array2:min_array1;
     
     ReadRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  &first_rim);//right
     
     if(avr_mv_x>60 && gmv_bgcnt<24000 && min_array<5)   //034 rim
     {
         WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-30);//right  14 to 30
         rim_hold_cnt_right_034=6;
         video034 = 1;
     }
     else if(rim_hold_cnt_right_034>0)
     {
         WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-30);//right  
         rim_hold_cnt_right_034--;
         video034 = 1;
     }
     else if(video034 > 0)
     {    
         if(first_rim==465)
         {
             WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  first_rim);
         }
         else
         {
             WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre);//right       
    
         }
         video034 = 0;
     }
     

     als32 = aplsum32rgn; //for right rim halo    
     for(rgnIdx=0;rgnIdx<32;rgnIdx++)        
     {
         if(rmvy[rgnIdx]<=-30) 
            rgnmv_cnt++; 
     }

    //-------------------------------------- right rim halo -------------------------------------------------//  
    
    right_rim_pre=s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_RHT];
    
    if((rgnmv_cnt==32) && (als32>2000000))
    {
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-35);//right  
        rim_hold_cnt_right=(s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 260: 130  ;  
        path2 = 0x1;
        rightrim = 1;
    }
    else if(rim_hold_cnt_right>0)
    {
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre-35);//right  
        rim_hold_cnt_right--;
        rightrim = 1;
        path2 = 0x1;
    }
    else if(rightrim>0)
    {            
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,  right_rim_pre);//right 
        rightrim = 0;
        path2 = 0x0;
    }




	//------------------------------ occl chaos 1---------------------------//
	occl_mid_chaos1 = _ABS_(gmv_bgx)<45 && _ABS_(gmv_bgx)>20 && _ABS_(u11_gmv_mvx)<45 && aplsum32rgn>3300000 && aplsum32rgn<4500000;
	occl_mid_chaos2 = Dtl_array[0]<10000  && Dtl_array[1]<10000  && Dtl_array[2]<10000  && Dtl_array[3]<10000  && Dtl_array[4]<10000  && Dtl_array[7]<50000  && Dtl_array[31]>50000 && Dtl_array[31]<130000 && Dtl_array[27]>70000 && Dtl_array[27]<140000 ;
	occl_mid_chaos3 = rgn_apli[0]>85000 && rgn_apli[16]>80000 && rgn_apli[16]<145000 && rgn_apli[9]>95000 && rgn_apli[9]<155000 && rgn_apli[18]>95000 && rgn_apli[18]<155000 && rgn_apli[10]>100000 && rgn_apli[10]<150000;

	
	pOutput->u8_dh_condition_occl_chaos1_bypass = 0;

	if(occl_mid_chaos1 && occl_mid_chaos2 && occl_mid_chaos3)
	{
	    	pOutput->u8_dh_condition_occl_chaos1_bypass = 1;
	   	//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0xa);
		holdframe_occl_mid = 60;
	}
	else if(holdframe_occl_mid>0)
	{
		holdframe_occl_mid--;
	       pOutput->u8_dh_condition_occl_chaos1_bypass = 1;
	  	//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0xa);
	 }
	else 
	{
		pOutput->u8_dh_condition_occl_chaos1_bypass = 0;
		 //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 28, 31, 0x0);
	}


	if((Dtl_array[0]>Dtl_array0_pre*3) &&(Dtl_array[14]>Dtl_array14_pre*3)&&(Dtl_array[22]>Dtl_array22_pre*3)&&(Dtl_array[31]>(Dtl_array31_pre*2)))
	{
		pOutput->u8_dh_condition_lager_dtlcomparison_bypass = 1;
	}
	else
	{
		pOutput->u8_dh_condition_lager_dtlcomparison_bypass = 0;
	}
	//rtd_pr_memc_info("124:dtl:%d %d %d %d flag:%d\n",Dtl_array[0],Dtl_array[14],Dtl_array[22],Dtl_array[31],pOutput->u8_dh_condition_lager_dtlcomparison_bypass);


	Dtl_array0_pre = Dtl_array[0];
	Dtl_array14_pre = Dtl_array[14];
	Dtl_array22_pre = Dtl_array[22];
	Dtl_array31_pre = Dtl_array[31];




	//-----------------215 shmmiering left side----------------------//


    if(gmv_bgcnt<13000)     
    {
        pOutput->u8_dh_smallbgcnt_slant_in_cnt=pOutput->u8_dh_smallbgcnt_slant_in_cnt+1;
        pOutput->u8_dh_smallbgcnt_slant_in_cnt=(pOutput->u8_dh_smallbgcnt_slant_in_cnt > 2) ? 2 : pOutput->u8_dh_smallbgcnt_slant_in_cnt;

    }
    else
    {
        pOutput->u8_dh_smallbgcnt_slant_in_cnt = 0;
    }  

	
    if((pOutput->u8_dh_smallbgcnt_slant_in_cnt >= 2)&&( u11_gmv_mvx<-40 && u10_gmv_mvy>=5 && gmv_bgx<-40 && gmv_bgy>10))
    {
		pOutput->u8_dh_condition_smallbgcnt_slant_bypass = 1;		
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
		//holdframe215215 = 100;
	}
	//else if(holdframe215215>0)
	//{
	//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
	//	holdframe215215--;
	//}
    else
    {
		pOutput->u8_dh_condition_smallbgcnt_slant_bypass = 0;	
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
    }
	//rtd_pr_memc_info("215flag:%d bgcnt:%d gmv(%d %d ) bg:(%d %d) fg(%d %d)\n",pOutput->u8_dh_condition_smallbgcnt_slant_bypass,gmv_bgcnt,u11_gmv_mvx,u10_gmv_mvy,gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy);



		//----------------- 109 ------------------------//
 	updateGMVArray(glb_bgcnt,glb_bgcnt_length,gmv_bgcnt);

	if((glb_bgcnt[0]<13000 && glb_bgcnt[1]<13000 &&glb_bgcnt[2]<13000) && gmv_bgy>110)
	{
		//WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);		
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
		pOutput->u8_dh_condition_smallbgcnt_ver_bypass = 1;	
	
	}
	//else if(holdframe_bigbgcnt>0)
	//{
		
		//WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x1);		
	//	holdframe_bigbgcnt--;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
	//}
	else
	{
		//WriteRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,0x0);	
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
		pOutput->u8_dh_condition_smallbgcnt_ver_bypass = 0;	

	}
	//rtd_pr_memc_info("109:%d %d %d %d \n",glb_bgcnt[0],glb_bgcnt[1],glb_bgcnt[2],gmv_bgy);
	
	gmv_fgcnt = 32400 - gmv_bgcnt;
	if( gmv_bgcnt>25000 &&  gmv_fgcnt>5000 &&  u11_gmv_mvx>100 && _ABS_(u10_gmv_mvy)<4 && aplsum32rgn<3000000)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
		holdframe_equalbgcnt = (s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) ? 30: 15  ;  
		video_fastmvx_equalbgcnt = 1;
		pOutput->u8_dh_condition_equalbgcnt_bigmvx = 1;
	}
	else if(holdframe_equalbgcnt>0)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
		holdframe_equalbgcnt--;
		pOutput->u8_dh_condition_equalbgcnt_bigmvx = 1;
	}
	else if(video_fastmvx_equalbgcnt>0)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
		video_fastmvx_equalbgcnt = 0;
		pOutput->u8_dh_condition_equalbgcnt_bigmvx = 0;

	}

	
	
	//------------------------------ occl chaos 2---------------------------//
	/*
	occl_hori_chaos1 = gmv_bgx<-70 && gmv_fgx<-60 && u11_gmv_mvx<-60 && aplsum32rgn>3800000 && aplsum32rgn<5300000;
	occl_hori_chaos2 = Dtl_array[31]<70000 && Dtl_array[24]<45000 && Dtl_array[8]<40000 && Dtl_array[16]<60000 && Dtl_array[22]<90000 && Dtl_array[22]>35000;
	occl_hori_chaos3 = rgn_apli[24]>100000 && rgn_apli[25]>90000 && rgn_apli[26]>70000 &&  rgn_apli[23]>130000 &&  rgn_apli[23]<200000 && rgn_apli[30]>80000 &&  rgn_apli[30]<180000 && rgn_apli[21]>150000 &&  rgn_apli[21]<225000;
	 
	pOutput->u8_dh_condition_occl_chaos2_bypass = 0;

	if(occl_hori_chaos1 && occl_hori_chaos2 && occl_hori_chaos3)
	{
		pOutput->u8_dh_condition_occl_chaos2_bypass = 1;
	 	holdframe_occl_hori = 30;
		//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0xa);

	}
	else if(holdframe_occl_hori>0)
	{
		holdframe_occl_hori--;
	       pOutput->u8_dh_condition_occl_chaos2_bypass = 1;
		//   	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0xa);

	 }
	else 
	{
		pOutput->u8_dh_condition_occl_chaos2_bypass = 0;
		//	WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0);

	}
	//rtd_pr_memc_info("1:%d %d %d 2:%d %d %d\n",occl_mid_chaos1,occl_mid_chaos2,occl_mid_chaos3,occl_hori_chaos1,occl_hori_chaos2,occl_hori_chaos3);
	*/

	//--------------------  car bypass--------------------------//
	for(rgnIdx=0;rgnIdx<8;rgnIdx++)
	{
		if(Dtl_array[rgnIdx]>10000 && Dtl_array[rgnIdx]<20000)
			dtlcnt_plane++;
		if(Dtl_array[rgnIdx+8]<15000)
			dtlcnt1++;
		if(Dtl_array[rgnIdx+16]<8000)
			dtlcnt2++;
		if(Dtl_array[rgnIdx+24]<20000 && Dtl_array[rgnIdx+24]>8000)
			dtlcnt3++;
	}
	
	cond_car_mv = aplsum32rgn>1000000 && aplsum32rgn<1500000 && ((gmv_bgx==0 && gmv_bgy==0)||( gmv_bgx<-40 && _ABS_(gmv_bgy)<10));
	cond_car_dtl = dtlcnt1>5 && dtlcnt2>7 &&  dtlcnt3>7 && Dtl_array[0]>10000 && Dtl_array[0]<35000;
	cond_car_apl = rgn_apli[7]>15000 && rgn_apli[7]<45000 && rgn_apli[11]>40000 && rgn_apli[11]<70000 && rgn_apli[12]>40000 && rgn_apli[12]<70000 && rgn_apli[24]<20000;
	if(cond_car_mv && cond_car_dtl  &&  cond_car_apl)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x1);
		holdframe_car =90;
		pOutput->u8_dh_condition_car_bypass = 1;
		path=0x1;
	}
	else
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
		pOutput->u8_dh_condition_car_bypass = 0;
		path = 0x0;
	}

	//rtd_pr_memc_info("car:%d %d %d cnt:%d %d %d\n",cond_car_mv,cond_car_dtl,cond_car_apl,dtlcnt1,dtlcnt2,dtlcnt3);


	//---------------------  C005 plane ---------------------------//

	for(rgnIdx=8;rgnIdx<24;rgnIdx++)
	{
		if(rgn_apli[rgnIdx]>100000 && rgn_apli[rgnIdx]<250000)
			aplbigcntplane++;
	}


	C005plane_1 = u11_gmv_mvx>15 && gmv_bgcnt>20000 && gmv_bgcnt<28000 && aplsum32rgn>3500000 && aplsum32rgn<4800000;
	C005plane_2 = dtlcnt2>=5 && dtlcnt_plane>=6 && Dtl_array[9]<5000 && Dtl_array[13]<5000 && Dtl_array[0]>12000 && Dtl_array[0]<20000 && Dtl_array[7]>12000 && Dtl_array[7]<21000; 
	C005plane_3 = aplbigcntplane>=13 && rgn_apli[0]<110000 && rgn_apli[6]<120000 &&  rgn_apli[6]>40000 && rgn_apli[7]<120000 &&  rgn_apli[7]>40000;
	if(C005plane_1 && C005plane_2 && C005plane_3)
	{
		holdframe_C005 = 60;
		video_C005 = 1;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x1);
		pOutput->u8_dh_condition_C005plane_status = 1;
		path = 0x3;
		
	}
	else if(holdframe_C005>0)
	{
		holdframe_C005--;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x1);
		pOutput->u8_dh_condition_C005plane_status = 1;
		path = 0x3;

	}
	else if(video_C005)
	{
		video_C005 = 0;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x0);
		pOutput->u8_dh_condition_C005plane_status = 0;
		path = 0x0;

	}

	//rtd_pr_memc_info("C005:%d %d %d cnt:%d %d\n",C005plane_1,C005plane_2,C005plane_3,dtlcnt2,aplbigcntplane);

	//---------------------- 0002  transporter black protect off--------------------------//
	//blkoff_1 = aplsum32rgn>1300000 && aplsum32rgn<2800000 &&  _ABS_(rmvx[9])>5 &&(( _ABS_(gmv_fgx)+_ABS_(gmv_fgy))>30 ||( _ABS_(gmv_bgx)+_ABS_(gmv_bgy))>45);
	//blkoff_2 = Dtl_array[1]<30000 && Dtl_array[8]<15000 && Dtl_array[24]>10000 && Dtl_array[24]<30000 && Dtl_array[25]>10000 && Dtl_array[25]<30000 && Dtl_array[29]>5000 && Dtl_array[29]<30000;
	//blkoff_3 = rgn_apli[11]>80000 && rgn_apli[11]<200000 && rgn_apli[14]>80000 && rgn_apli[14]<210000 && rgn_apli[22]>40000 && rgn_apli[22]<110000;

	//blkoff_1 = aplsum32rgn>1300000 && aplsum32rgn<2700000 && gmv_bgx<-30 && _ABS_(gmv_fgy)>5 && u11_gmv_mvx<-30 && _ABS_(u10_gmv_mvy)<=20;
	//blkoff_2 =Dtl_array[0]<50000 && Dtl_array[1]<50000 &&  Dtl_array[5]<50000 &&  Dtl_array[16]<30000 && Dtl_array[24]<30000 && Dtl_array[25]<28000 && Dtl_array[26]<30000 && Dtl_array[29]>1000 && Dtl_array[29]<35000 && Dtl_array[31]<42000 ;
	//blkoff_3 = rgn_apli[11]<200000 && rgn_apli[14]>40000 && rgn_apli[14]<210000 && rgn_apli[22]>20000 && rgn_apli[22]<160000;
	//blkoff_3 =Dtl_array[8]>0 &&  Dtl_array[15]>0 &&  Dtl_array[16]>0  && Dtl_array[23]>0 && rgn_apli[31]<100000 && rgn_apli[15]>50000 && rgn_apli[23]>40000  && Dtl_array[6]>5000 && Dtl_array[6]<40000;
	
	blkoff_1 = aplsum32rgn>1300000 && aplsum32rgn<2700000 && gmv_bgx<-25 && _ABS_(gmv_fgy)>5 && u11_gmv_mvx<-25 && _ABS_(u10_gmv_mvy)<=20;
	blkoff_2 =Dtl_array[0]<60000 && Dtl_array[1]<60000 &&  Dtl_array[5]<50000 &&  Dtl_array[16]<30000 && Dtl_array[24]<35000 && Dtl_array[25]<35000 && Dtl_array[26]<35000 && Dtl_array[29]>1000 && Dtl_array[29]<40000 && Dtl_array[31]<55000 ;
	//blkoff_3 = rgn_apli[11]<200000 && rgn_apli[14]>40000 && rgn_apli[14]<210000 && rgn_apli[22]>20000 && rgn_apli[22]<160000;
	blkoff_3 =Dtl_array[8]>0 &&  Dtl_array[15]>0 &&  Dtl_array[16]>0  && Dtl_array[23]>0 && rgn_apli[31]<100000  && rgn_apli[23]>40000  && Dtl_array[6]>5000 && Dtl_array[6]<46000;


	blkoff_4 = aplsum32rgn>1700000 && aplsum32rgn<2500000 && gmv_bgy>30 && gmv_fgy>20 && gmv_bgcnt<10000;
	blkoff_5 = Dtl_array[0]>8000 && Dtl_array[0]<35000 && Dtl_array[1]>15000 && Dtl_array[1]<30000 && Dtl_array[25]<20000 && Dtl_array[24]<30000;
	blkoff_6 = rgn_apli[0]>5000 && rgn_apli[0]<25000 &&  rgn_apli[3]>60000 && rgn_apli[3]<75000 && rgn_apli[7]>115000 && rgn_apli[7]<125000 && rgn_apli[29]<10000 &&rgn_apli[14]>100000 && rgn_apli[14]<120000;



	transporter_1 = blkoff_1 &&blkoff_2  && blkoff_3;
	transporter_2 =  (s_pContext->_external_data._output_frameRate == _PQL_OUT_100HZ)?(blkoff_4 &&blkoff_5  && blkoff_6):0;

	
	if(transporter_1 || transporter_2)		
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x2);
		holdframe_0002 = 250;
	 	video_0002 = 1;
		pOutput->u8_dh_condition_transporter_status = 1;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 18, 18, 0x1);
		path = 0x4;

	}
	else if(holdframe_0002>0)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x2);
		holdframe_0002--;
		pOutput->u8_dh_condition_transporter_status = 1;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 18, 18, 0x1);
		path = 0x4;

	}
	else if(video_0002)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
		video_0002 = 0;
		pOutput->u8_dh_condition_transporter_status = 0;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 18, 18, 0x0);
		path = 0x0;

	}
	//rtd_pr_memc_info("02:%d %d %d\n",blkoff_1,blkoff_2,blkoff_3);


		//---------------------------   small plane  ----------------------------//
	for(rgnIdx=0;rgnIdx<32;rgnIdx++)
	{
		if(rmvx[rgnIdx]>40 && rmvx[rgnIdx]<60)
			smallplane_mvcnt++;
		if(rgn_apli[rgnIdx]>80000 && rgn_apli[rgnIdx]<220000)
			smallplane_aplcnt++;
	}
	cond_smallplane1 = u11_gmv_mvx>40 && u11_gmv_mvx<60 && _ABS_(u10_gmv_mvy)<=10 && u10_gmv_mvy<=0 && smallplane_mvcnt>=27 && gmv_bgcnt>19000; 
	cond_smallplane2 = (Dtl_array[1]<50 || Dtl_array[2]<50 || Dtl_array[4]<50 || Dtl_array[5]<50 ||(Dtl_array[23]>80000 && Dtl_array[31]<130000 && Dtl_array[26]>40000 && Dtl_array[26]<70000 && Dtl_array[31]>80000 && Dtl_array[31]<120000)) ;
	cond_smallplane3 = smallplane_aplcnt >=20;

	if(cond_smallplane1 && cond_smallplane2 && cond_smallplane3)		
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x1);
		holdframe_smallplane = 80;
	 	video_smallplane = 1;
		pOutput->u8_dh_condition_smallplane_status = 1;
		//path = 0x5;

	}
	else if(holdframe_smallplane>0)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x1);
		holdframe_smallplane--;
		pOutput->u8_dh_condition_smallplane_status = 1;
		//path = 0x5;

	}
	else if(video_smallplane)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 16, 19, 0x0);
		video_smallplane = 0;
		pOutput->u8_dh_condition_smallplane_status = 0;
		//path = 0x0;

	}
	//rtd_pr_memc_info("cond:%d %d %d cnt:%d %d\n",cond_smallplane1,cond_smallplane2,cond_smallplane3,smallplane_mvcnt,smallplane_aplcnt);


	//---------------------------------- 60016 dance ----------------------------------------//
	//before
	//cond_60016dance_1 = u11_gmv_mvx>=15 && u11_gmv_mvx<=40 &&  _ABS_(u10_gmv_mvy)<10 && gmv_bgx>15 && gmv_bgx<45 && aplsum32rgn>2500000 && aplsum32rgn<3200000 && rmvx[31]>20 && rmvx[31]<45;
	//cond_60016dance_2 = Dtl_array[12]>40000 && Dtl_array[12]<80000 && Dtl_array[13]>60000 && Dtl_array[13]<110000 && Dtl_array[28]<40000 && Dtl_array[27]>30000 && Dtl_array[27]<70000;
	//cond_60016dance_3 = rgn_apli[1]>80000 && rgn_apli[1]<120000 && rgn_apli[2]>80000 && rgn_apli[2]<140000 && rgn_apli[5]>100000 && rgn_apli[5]<150000 && rgn_apli[30]>60000 && rgn_apli[30]<100000;


	cond_60016dance_1 = u11_gmv_mvx>=15 && u11_gmv_mvx<=50 &&  _ABS_(u10_gmv_mvy)<10 && gmv_bgx>15 && gmv_bgx<45 && aplsum32rgn>2500000 && aplsum32rgn<3200000 && rmvx[31]>20 && rmvx[31]<50;
	cond_60016dance_2 = Dtl_array[12]>35000 && Dtl_array[12]<80000 && Dtl_array[13]>60000 && Dtl_array[13]<110000 && Dtl_array[28]<40000 && Dtl_array[27]>30000 && Dtl_array[27]<70000;
	cond_60016dance_3 = rgn_apli[1]>80000 && rgn_apli[1]<120000 && rgn_apli[2]>80000 && rgn_apli[2]<150000 && rgn_apli[5]>100000 && rgn_apli[5]<150000 && rgn_apli[30]>55000 && rgn_apli[30]<100000;

	pOutput->u8_dh_condition_dance_status = 0;

	
	if(cond_60016dance_1 && cond_60016dance_2 && cond_60016dance_3)		
	{
		holdframe_60016 = 70;
	 	video_60016 = 1;
		pOutput->u8_dh_condition_dance_status = 1;
		//path = 0x6;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);

	}
	else if(holdframe_60016>0)
	{
		holdframe_60016--;
		pOutput->u8_dh_condition_dance_status = 1;
		//path = 0x6;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);

	}
	else if(video_60016)
	{
		video_60016 = 0;
		pOutput->u8_dh_condition_dance_status = 0;
		//path = 0x0;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);

	}
	//rtd_pr_memc_info("dance1:%d %d %d\n",cond_60016dance_1 , cond_60016dance_2 , cond_60016dance_3);

	//------------------------------ soccer ----------------------------------//
	for(rgnIdx=0;rgnIdx<8;rgnIdx++)
	{
		if(rgn_apli[rgnIdx]<100000)
			cond_soccer_aplcnt1++;
	}
	for(rgnIdx=16;rgnIdx<32;rgnIdx++)
	{
		if(rgn_apli[rgnIdx]>120000 && rgn_apli[rgnIdx]<210000)
			cond_soccer_aplcnt2++;
	}
	
	cond_soccer1 = gmv_bgcnt >22000  &&  gmv_bgx<-20 && gmv_bgy<=0 && _ABS_(gmv_bgy)<15 && u11_gmv_mvx<-20 && u10_gmv_mvy<=0 && rmvx[23]<-30 && rmvx[31]<-30 &&aplsum32rgn>3700000 && aplsum32rgn<4800000;
	cond_soccer2 = Dtl_array[16]<15000 && Dtl_array[18]<15000 && Dtl_array[22]<15000 && Dtl_array[23]<15000 && Dtl_array[30]<10000 && Dtl_array[6]>35000;
	cond_soccer3 = cond_soccer_aplcnt1>=7 && cond_soccer_aplcnt2>=15;
	
	if(cond_soccer1 && cond_soccer2 && cond_soccer3)		
	{
		holdframe_soccer = 70;
	 	video_soccer = 1;
		pOutput->u8_dh_condition_soccer_status = 1;
		path = 0x7;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);

	}
	else if(holdframe_soccer>0)
	{
		holdframe_soccer--;
		pOutput->u8_dh_condition_soccer_status = 1;
		path = 0x7;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);

	}
	else if(video_soccer)
	{
		video_soccer = 0;
		pOutput->u8_dh_condition_soccer_status = 0;
		path = 0x0;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);

	}
	//rtd_pr_memc_info("C002:cond:%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d cnt:%d %d\n",cond_soccer1,cond_soccer2,cond_soccer3,  gmv_bgcnt,  gmv_bgx,gmv_bgy, u11_gmv_mvx,  u10_gmv_mvy,   rmvx[23], rmvx[31], aplsum32rgn, 
	//	Dtl_array[16],Dtl_array[18],Dtl_array[22],Dtl_array[23],Dtl_array[30],Dtl_array[6],cond_soccer_aplcnt1,cond_soccer_aplcnt2);




	//------------------------- rotterdam ---------------------------//
	for(rgnIdx=0;rgnIdx<16;rgnIdx++)
	{
		if(Dtl_array[rgnIdx]<50)
			cond_rotterdam_dtlcnt++;
	}
	for(rgnIdx=0;rgnIdx<32;rgnIdx++)
	{
		if(rgn_apli[rgnIdx]<200000 && rgn_apli[rgnIdx]>70000)
			cond_rotterdam_aplcnt++;
	}
	pOutput->u8_dh_condition_rotterdam_status = 0;
	//old
	//cond_rotterdam1 = (gmv_bgx>100 || gmv_fgx>100)  &&  _ABS_(gmv_bgy)<5 &&  _ABS_(gmv_fgy)<5 && aplsum32rgn>3600000 && aplsum32rgn<5500000 && rmvx[2]>100;
	//cond_rotterdam2 = cond_rotterdam_dtlcnt>=3 && Dtl_array[0]>15000 && Dtl_array[1]>10000 ;
	//cond_rotterdam3= cond_rotterdam_aplcnt>30;
	cond_rotterdam1 = (gmv_bgx>70 || gmv_fgx>70)  &&  _ABS_(gmv_bgy)<20 &&  _ABS_(gmv_fgy)<10 && aplsum32rgn>3600000 && aplsum32rgn<5500000 && rmvx[2]>85;
	cond_rotterdam2 = cond_rotterdam_dtlcnt>=3 && Dtl_array[0]>14000 && Dtl_array[1]>8000 ;
	cond_rotterdam3= cond_rotterdam_aplcnt>30;

	WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 19, 19, 0x0);
	
	WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 19, 19, 0x0);
	
	WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 19, 19, 0x0);
	
	if(cond_rotterdam1 && cond_rotterdam2 && cond_rotterdam3)		
	{
		holdframe_rotterdam = 90;
	 	video_rotterdam = 1;
		pOutput->u8_dh_condition_rotterdam_status = 1;
		path = 0x8;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 19, 19, 0x1);


	}
	else if(holdframe_rotterdam>0)
	{
		holdframe_rotterdam--;
		pOutput->u8_dh_condition_rotterdam_status = 1;
		path = 0x8;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x3);
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,19, 19, 0x1);


	}
	else if(video_rotterdam)
	{
		video_rotterdam = 0;
		pOutput->u8_dh_condition_rotterdam_status = 0;
		path = 0x0;
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 19, 19, 0x0);


	}
	//rtd_pr_memc_info("flag:%d %d %d %d\n",pOutput->u8_dh_condition_rotterdam_status,cond_rotterdam1,cond_rotterdam2,cond_rotterdam3);



	if(pOutput->u8_dh_condition_rotterdam_status==1 &&  Dtl_array[25]<500 && Dtl_array[26]<500 && Dtl_array[27]<500)
	{
		path = 0xd;
		video_rotterdam2 = 1;
		pOutput->u8_dh_condition_rotterdam2_status = 1;
		holdframe_rotterdam2 = 90;

	}
	else if(holdframe_rotterdam2>0)
	{
		holdframe_rotterdam2 --;
		path = 0xd;
		video_rotterdam2 = 1;
		pOutput->u8_dh_condition_rotterdam2_status = 1;

	}	
	else if(video_rotterdam2>0)
	{
		path = 0x0;
		video_rotterdam2 = 0;
		pOutput->u8_dh_condition_rotterdam2_status = 0;

	}







	//---------------------------------- 60016 dance  2----------------------------------------//

	cond_60016dance_21 = u11_gmv_mvx>=50 && u11_gmv_mvx<75 && u10_gmv_mvy>=10 && u10_gmv_mvy<25 && gmv_fgx>15 &&  gmv_fgy>10 && rmvx[24]>70 && rmvx[31]>60 && aplsum32rgn>2400000 && aplsum32rgn<3400000;
	cond_60016dance_22 = Dtl_array[0]>25000 &&Dtl_array[0]<40000 && Dtl_array[4]>50000 && Dtl_array[15]<15000 &&  Dtl_array[23]<12000 && Dtl_array[28]<20000 && Dtl_array[24]>25000 && Dtl_array[24]<40000;
	cond_60016dance_23 = rgn_apli[28]<20000 && rgn_apli[3]>120000 && rgn_apli[3]<170000 && rgn_apli[26]>110000 && rgn_apli[26]<150000 && rgn_apli[16]>100000 && rgn_apli[16]<160000;
	pOutput->u8_dh_condition_dance_2_status = 0;
	if(cond_60016dance_21 && cond_60016dance_22 && cond_60016dance_23)		
	{
		holdframe_60016_2 = 40;
	 	video_60016_2 = 1;
		pOutput->u8_dh_condition_dance_2_status = 1;
		//path = 0x9;

	}
	else if(holdframe_60016_2>0)
	{
		holdframe_60016_2--;
		pOutput->u8_dh_condition_dance_2_status = 1;
		//path = 0x9;

	}
	else if(video_60016_2)
	{
		video_60016_2 = 0;
		pOutput->u8_dh_condition_dance_2_status = 0;
		//path = 0x0;
	}
	//rtd_pr_memc_info("dance2:%d %d %d\n",cond_60016dance_21 , cond_60016dance_22 , cond_60016dance_23);
	//rtd_pr_memc_info("mv:%d %d %d %d %d %d %d\n",u11_gmv_mvx,u10_gmv_mvy,gmv_fgx,gmv_fgy, rmvx[24], rmvx[31],aplsum32rgn);

	//--------------------------------- skating girl --------------------------------//
	
	for(rgnIdx=0;rgnIdx<32;rgnIdx++)
	{
		if(rmvx[rgnIdx]<-45 && rmvx[rgnIdx]>-80)
			cond_skating_mvcnt++;
	}

	//old
	//skating_1 = gmv_bgx<-50 && _ABS_(gmv_bgy)<5 && _ABS_(gmv_fgx)<8 && _ABS_(gmv_fgy)<8 && cond_skating_mvcnt>25 && aplsum32rgn>1500000 && aplsum32rgn<2800000;
	//skating_2 = Dtl_array[0]==7650 && Dtl_array[8]==0 && Dtl_array[16]==0 && Dtl_array[24]==7650 && Dtl_array[7]==0 && Dtl_array[15]==0 && Dtl_array[23]==0 && Dtl_array[31]==0 && Dtl_array[5]>40000 && Dtl_array[5]<60000;
	//skating_3 = rgn_apli[0]<10 && rgn_apli[8]==0 && rgn_apli[16]==0 && rgn_apli[24]<100 && rgn_apli[7]<50 && rgn_apli[15]==0 && rgn_apli[23]==0 && rgn_apli[31]<100; 

	skating_1 = gmv_bgx<-40 && _ABS_(gmv_bgy)<5 && ((_ABS_(gmv_fgx)>40) || (gmv_fgx<=0 ))&& gmv_fgy<=0 && aplsum32rgn>1200000 && aplsum32rgn<2800000;
	skating_2 = Dtl_array[0]==7650 && Dtl_array[8]==0 && Dtl_array[16]==0 && Dtl_array[24]==7650 && Dtl_array[7]==7650 && Dtl_array[15]==0 && Dtl_array[23]==0 && Dtl_array[31]==7650 && Dtl_array[5]>30000 && Dtl_array[5]<90000;
	skating_3 = rgn_apli[0]<10 && rgn_apli[8]<10 && rgn_apli[16]<10 && rgn_apli[24]<100 && rgn_apli[7]<50 && rgn_apli[15]<10 && rgn_apli[23]<10 && rgn_apli[31]<100; 

	
	pOutput->u8_dh_condition_skating_status = 0;

	if(skating_1 && skating_2 && skating_3)		
	{
		holdframe_skating = 120; //120
	 	video_skating = 1;
		pOutput->u8_dh_condition_skating_status = 1;
		path = 0xa;
	}
	else if(holdframe_skating>0)
	{
		holdframe_skating--;
		pOutput->u8_dh_condition_skating_status = 1;
		path = 0xa;

	}
	else if(video_skating)
	{
		video_skating = 0;
		pOutput->u8_dh_condition_skating_status = 0;
		path = 0x0;
	}	
	//rtd_pr_memc_info("skating:%d %d %d\n",skating_1 , skating_2 , skating_3);


	
	//---------------------------------- C001 soccer ---------------------------------------//
	
	for(rgnIdx=0;rgnIdx<32;rgnIdx++)
	{
		if(rmvx[rgnIdx]<=0 && rmvy[rgnIdx]<=0 && rmvy[rgnIdx]>=-30)
			cond_C001soccer_mvcnt++;
	}

	
	cond_C001soccer_1 = gmv_bgcnt>30000 && aplsum32rgn>3000000 && aplsum<5000000 && u10_gmv_mvy<=0 && u10_gmv_mvy>=-30 && u11_gmv_mvx<0 &&gmv_bgx<-5 && cond_C001soccer_mvcnt>30;
	cond_C001soccer_2 = Dtl_array[0]>80000 && Dtl_array[0]<125000 && Dtl_array[7]>90000 && Dtl_array[7]<125000 &&  Dtl_array[26]>60000 && Dtl_array[26]<115000 && Dtl_array[15]<55000;
	cond_C001soccer_3 = rgn_apli[23]>100000 && rgn_apli[23]<165000 &&  rgn_apli[30]>100000 && rgn_apli[30]<155000; 



	if(cond_C001soccer_1 && cond_C001soccer_2 && cond_C001soccer_3)		
	{
		holdframe_C001soccer = 150;
	 	video_C001soccer = 1;
		pOutput->u8_dh_condition_C001soccer_status = 1;
		path = 0xb;

	}
	else if(holdframe_C001soccer>0)
	{
		holdframe_C001soccer--;
		pOutput->u8_dh_condition_C001soccer_status = 1;
		path = 0xb;

	}
	else if(video_C001soccer)
	{
		video_C001soccer = 0;
		pOutput->u8_dh_condition_C001soccer_status = 0;
		path = 0x0;
	}	
	//rtd_pr_memc_info("C001:%d %d %d   1:%d %d %d  %d %d %d  3:%d %d\n",cond_C001soccer_1 , cond_C001soccer_2 ,cond_C001soccer_3,gmv_bgcnt,aplsum32rgn,u10_gmv_mvy,u11_gmv_mvx,gmv_bgx,cond_C001soccer_mvcnt, rgn_apli[23], rgn_apli[30]);
	//rtd_pr_memc_info("C001:2:%d %d %d %d\n",Dtl_array[0],Dtl_array[7],Dtl_array[26],Dtl_array[15]);


	//---------------------------- 30006 ----------------------------//
	cond30006_mv = u11_gmv_mvx<50 && u11_gmv_mvx>20 && u10_gmv_mvy<5 && u10_gmv_mvy>=0 ;
	cond30006_dtl = Dtl_array[24]>30000 && Dtl_array[24]<55000 &&  Dtl_array[31]>30000 && Dtl_array[31]<55000 &&  Dtl_array[16]>40000 && Dtl_array[16]<65000; 
	cond30006_apl = aplsum32rgn>2000000 && aplsum32rgn<3000000 && rgn_apli[24]>65000 && rgn_apli[24]<85000 && rgn_apli[3]>60000 && rgn_apli[3]<90000 && rgn_apli[30]>50000 && rgn_apli[30]<70000; 


	if(cond30006_mv && cond30006_dtl && cond30006_apl)		
	{
		holdframe_30006 = 50;
	 	video30006 = 1;
		pOutput->u8_dh_condition_30006_status = 1;
		path = 0xc;
	}
	else if(holdframe_30006>0)
	{
		holdframe_30006--;
		pOutput->u8_dh_condition_30006_status = 1;
		path = 0xc;
	}
	else if(video30006)
	{
		video30006 = 0;
		pOutput->u8_dh_condition_30006_status = 0;
		path = 0x0;
	}	
	//rtd_pr_memc_info("30006:%d %d %d\n",cond30006_mv , cond30006_dtl , cond30006_apl);



	//--------------------- girl ride bike -------------------------//
	//ridebike_mv = _ABS_(gmv_bgx)>10  &&  _ABS_(gmv_bgy)<5 && gmv_fgx>10 && gmv_fgx<40 && _ABS_(gmv_fgy)<5 && aplsum32rgn>3000000 && aplsum32rgn<4000000;
	//ridebike_dtl = Dtl_array[0]==7650 && Dtl_array[24]==7650 && Dtl_array[8]==0  && Dtl_array[16]==0 && Dtl_array[30]>20000 && Dtl_array[30]<40000 && Dtl_array[31]==0 ;
	//ridebike_apl = rgn_apli[0]==0 &&  rgn_apli[8]==0 &&  rgn_apli[16]==0 &&  rgn_apli[24]<100 && rgn_apli[31]<100 && rgn_apli[1]>50000 && rgn_apli[1]<160000 && rgn_apli[10]>170000;
	//new
	ridebike_mv = _ABS_(gmv_bgx)>5  &&  _ABS_(gmv_bgy)<5 && gmv_fgx>10 && gmv_fgx<60 && _ABS_(gmv_fgy)<5 && aplsum32rgn>3000000 && aplsum32rgn<4500000;
	ridebike_dtl = Dtl_array[0]==7650 && Dtl_array[24]==7650 && Dtl_array[8]==0  && Dtl_array[16]==0 && Dtl_array[30]>20000 && Dtl_array[30]<50000 && ((Dtl_array[31]==0)||(Dtl_array[31]==7650)) ;
	ridebike_apl = rgn_apli[0]<50 &&  rgn_apli[8]<50 &&  rgn_apli[16]<50&&  rgn_apli[24]<100 && rgn_apli[31]<200 && rgn_apli[1]>50000 && rgn_apli[1]<200000 && rgn_apli[10]>150000;


	if(ridebike_mv && ridebike_dtl && ridebike_apl) 	
	{
		holdframe_ridebikegirl = 70;
		ridebikegirl = 1;
		pOutput->u8_dh_condition_ridebikegirl_status = 1;
		path = 0xe;
	}
	else if(holdframe_ridebikegirl>0)
	{
		holdframe_ridebikegirl--;
		pOutput->u8_dh_condition_ridebikegirl_status = 1;
		path = 0xe;
	}
	else if(ridebikegirl)
	{
		ridebikegirl = 0;
		pOutput->u8_dh_condition_ridebikegirl_status = 0;
		path = 0x0;
	}	
	//rtd_pr_memc_info("e:%d %d %d\n",ridebike_mv,ridebike_dtl,ridebike_apl);	
	//rtd_pr_memc_info("1:%d %d %d %d %d, 2:%d %d %d %d %d %d\n",gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy,aplsum32rgn,Dtl_array[0],Dtl_array[24],Dtl_array[8],Dtl_array[16],Dtl_array[30],Dtl_array[31]);	
	//rtd_pr_memc_info("3:%d %d %d %d %d %d %d\n", rgn_apli[0],rgn_apli[8],rgn_apli[16],rgn_apli[24],rgn_apli[31],rgn_apli[1],rgn_apli[10]);	


	toleft_bike_mv = (gmv_bgx>30 ||gmv_fgx<-30) && u11_gmv_mvx>0 && _ABS_(u10_gmv_mvy)<5;
	toleft_bike_dtl = Dtl_array[16]<30000 && Dtl_array[0]<30000 && Dtl_array[7]<30000 && Dtl_array[22]<70000  && Dtl_array[22]>2000 && Dtl_array[25]<50000  && Dtl_array[25]>10000 && rgn_apli[27]>50000 && rgn_apli[27]<120000 && rgn_apli[25]>35000;
	toleft_bike_apl  = aplsum32rgn>1500000 && aplsum32rgn<2200000 && rgn_apli[0]<15000 && rgn_apli[7]<30000 && rgn_apli[31]<50000 &&rgn_apli[22]>70000 && rgn_apli[22]<180000 && rgn_apli[24]>20000 && rgn_apli[24]<45000 ;

	if(toleft_bike_mv && toleft_bike_dtl && toleft_bike_apl)
	{
		video_toleft_bike = 1;
		holdmframe_toleft_bike = 80;
		pOutput->u8_dh_conditio_toleft_bike_status = 1;
		//path = 0xf;

		
	}
	else if(holdmframe_toleft_bike>0)
	{
		holdmframe_toleft_bike--;
		pOutput->u8_dh_conditio_toleft_bike_status = 1;
		//path = 0xf;


	}
	else if(video_toleft_bike)
	{
		video_toleft_bike = 0;
		pOutput->u8_dh_conditio_toleft_bike_status = 0;
		//path = 0x0;

	}
	//rtd_pr_memc_info("cond:%d %d %d \n",toleft_bike_mv,toleft_bike_dtl,toleft_bike_apl);
	//rtd_pr_memc_info("1:%d %d %d %d 2:%d %d %d %d %d 3:%d %d %d %d %d %d %d\n",gmv_bgx,gmv_fgx,u11_gmv_mvx,u10_gmv_mvy,Dtl_array[16],Dtl_array[0],Dtl_array[7],Dtl_array[22],Dtl_array[25],aplsum32rgn,rgn_apli[0],rgn_apli[7],rgn_apli[31],rgn_apli[22],rgn_apli[24],rgn_apli[27]);


	cond30003_mv = gmv_bgx>55 && gmv_bgx<90 && gmv_fgx>10 &&gmv_fgx<50 && _ABS_(gmv_bgy)<5  && _ABS_(gmv_fgy)<5 && aplsum32rgn>1500000 && aplsum32rgn<3000000;
	cond30003_dtl = Dtl_array[7]<20000 && Dtl_array[16]<20000 && Dtl_array[29]<20000 && Dtl_array[30]<20000 && Dtl_array[31]<20000 && Dtl_array[23]<16000 && Dtl_array[6]>20000;
	cond30003_apl = rgn_apli[24]<70000 &&  rgn_apli[25]>50000 &&  rgn_apli[29]>65000 &&  rgn_apli[31]<40000 &&  rgn_apli[8]<30000; 

	
	if(cond30003_mv && cond30003_dtl && cond30003_apl)
	{
		video_3003man= 1;
		holdframe_3003man = 150;
		pOutput->u8_dh_conditio_3003man_status = 1;
		path = 0x1;		
	}
	else if(holdframe_3003man>0)
	{
		holdframe_3003man--;
		pOutput->u8_dh_conditio_3003man_status = 1;
		path = 0x1;
	}
	else if(video_3003man)
	{
		video_3003man = 0;
		pOutput->u8_dh_conditio_3003man_status = 0;
		path = 0x0;
	}
	//rtd_pr_memc_info("codnqiamian:%d %d %d\n",cond30003_mv , cond30003_dtl ,cond30003_apl);
	//rtd_pr_memc_info("1:%d %d %d %d %d   2:%d %d %d %d %d %d %d\n",gmv_bgx,gmv_fgx,gmv_bgy,gmv_fgy,aplsum32rgn,Dtl_array[7],Dtl_array[16],Dtl_array[29],Dtl_array[30],Dtl_array[31],Dtl_array[23],Dtl_array[6]);
	//rtd_pr_memc_info("3:%d %d %d %d %d\n",rgn_apli[24],rgn_apli[25],rgn_apli[29],rgn_apli[31],rgn_apli[8] );


	cond0026_mv =gmv_bgcnt<30000 && (((_ABS_(gmv_fgx)+_ABS_(gmv_fgy))>40)  ||  ((_ABS_(gmv_bgx)+_ABS_(gmv_bgy))>50)) && u11_gmv_mvx>25 && _ABS_(u10_gmv_mvy)<20 && aplsum32rgn>1500000 && aplsum32rgn<3000000;
	cond0026_dtl = Dtl_array[0]<20000 && Dtl_array[1]<20000 && Dtl_array[31]<10000 && Dtl_array[25]<20000 && Dtl_array[24]<30000  && Dtl_array[1]<30000 && Dtl_array[2]<30000 && Dtl_array[3]<30000;
	cond0026_apl = rgn_apli[0]>50000 &&  rgn_apli[0]<90000 && rgn_apli[8]>50000 && rgn_apli[27]<60000 && rgn_apli[7]>50000; 

	pOutput->u8_dh_conditio_0026_1_status = 0;

	if(cond0026_mv && cond0026_dtl && cond0026_apl)
	{
		holdframe_0026_1=45;
		video_0026_1 = 1;
		pOutput->u8_dh_conditio_0026_1_status = 1;
		path = 0x2;

	}
	else if(holdframe_0026_1>0)
	{
		holdframe_0026_1--;
		video_0026_1 = 1;
		pOutput->u8_dh_conditio_0026_1_status = 1;
		path = 0x2;

	}
	else if(video_0026_1>0)
	{
		video_0026_1 = 0;
		pOutput->u8_dh_conditio_0026_1_status = 0;
		path = 0x0;
	}


	//--------------------- insect 1080_24HZ_Pendulum ------------------------//
	insect_mv = (u11_gmv_mvx == 0) && (u10_gmv_mvy == 0)&&((_ABS_(gmv_fgx)>50)||(_ABS_(gmv_fgx)<10) )&& _ABS_(gmv_bgx)<10 && (gmv_bgy==0);
	insect_dtl = Dtl_array[1]<1000 && Dtl_array[2]<1000 && Dtl_array[4]<5000 && Dtl_array[7]>8000  && Dtl_array[7]<30000 && Dtl_array[25]<1000 && Dtl_array[26]<5000 && Dtl_array[24]>3000 && Dtl_array[24]<5000;
	insect_apl = aplsum32rgn>5000000 && aplsum32rgn<6200000 && rgn_apli[24]>150000 && rgn_apli[24]<190000 && rgn_apli[31]>180000 && rgn_apli[31]<220000;

	//biaozhun
	insect_dtl2 = Dtl_array[1]<5000 && Dtl_array[2]<5000 && Dtl_array[4]<8000 && Dtl_array[7]>10000  && Dtl_array[7]<30000 && Dtl_array[25]<1000 && Dtl_array[26]<5000 && Dtl_array[24]>3000 && Dtl_array[24]<6000;
	insect_apl2 = aplsum32rgn>4500000 && aplsum32rgn<5500000 && rgn_apli[24]>130000 && rgn_apli[24]<170000 && rgn_apli[31]>170000 && rgn_apli[31]<200000;

	//jieneng
	insect_dtl3 = Dtl_array[1]<5000 && Dtl_array[2]<5000 && Dtl_array[4]<8000 && Dtl_array[7]>10000  && Dtl_array[7]<30000 && Dtl_array[25]<1000 && Dtl_array[26]<5000 && Dtl_array[24]>3000 && Dtl_array[24]<6000;
	insect_apl3 = aplsum32rgn>4500000 && aplsum32rgn<5500000 && rgn_apli[24]>130000 && rgn_apli[24]<170000 && rgn_apli[31]>160000 && rgn_apli[31]<190000;

	insect_1 = insect_mv && insect_apl && insect_dtl;
	insect_2 = insect_mv && insect_apl2 && insect_dtl2;
	insect_3 = insect_mv && insect_apl3 && insect_dtl3;




	if(insect_1 || insect_2 || insect_3)
	{
		holdframe_insect = 140;
		video_insect = 1;
		pOutput->u8_dh_condition_insect_status = 1;
		path = 0x9;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 20, 20, 0x1);

	}
	else if(holdframe_insect>0)
	{
		holdframe_insect--;
		video_insect = 1;
		pOutput->u8_dh_condition_insect_status = 1;
		path = 0x9;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 20, 20, 0x1);
	}
	else if(video_insect>0)
	{
		video_insect = 0;
		pOutput->u8_dh_condition_insect_status = 0;
		path = 0x0;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 20, 20, 0x0);
	}


	//---------------------  eagle dehal on---------------------------//
	for(rgnIdx=0;rgnIdx<32;rgnIdx++)
	{
		if(rmvy[rgnIdx]<-60 &&  _ABS_(rmvx[rgnIdx])<15)
			rmvyfastcnt++;
		if(rgn_apli[rgnIdx]>100000 && rgn_apli[rgnIdx]<200000)
			aplbigcnt++;
	}
	for(rgnIdx=0;rgnIdx<16;rgnIdx++)
	{
		if(Dtl_array[rgnIdx]<5000)
			dtlsmallcnt++;
	}
	//cond_eagle_mv = gmv_bgy<-60 &&  gmv_bgx>-15 && gmv_bgx<=0 && _ABS_(gmv_fgx)<10  && rmvyfastcnt>=25 && aplsum32rgn>3500000 && aplsum32rgn<4800000;
	//cond_eagle_dtl = dtlsmallcnt>15 && Dtl_array[18]>10000 && Dtl_array[18]<25000 && Dtl_array[19]>15000 && Dtl_array[19]<35000 && Dtl_array[31]<1000;
	//cond_eagle_apl = aplbigcnt>=30;
	
	cond_eagle_mv = gmv_bgy<-60 &&  gmv_bgx>-15 && gmv_bgx<=0 && _ABS_(gmv_fgx)<10  &&  aplsum32rgn>4000000 && aplsum32rgn<5000000;
	cond_eagle_dtl = dtlsmallcnt>12 && Dtl_array[18]>10000 && Dtl_array[18]<30000 && Dtl_array[19]>15000 && Dtl_array[19]<35000 && Dtl_array[31]<5000;
	cond_eagle_apl = aplbigcnt>=25;

	//rtd_pr_memc_info("cond:%d %d %d  cnt:%d %d %d\n",cond_eagle_mv,cond_eagle_dtl,cond_eagle_apl,rmvyfastcnt,dtlsmallcnt,aplbigcnt);
	//rtd_pr_memc_info("mv:%d %d %d cnt:%d %d\n",gmv_bgy,gmv_bgx,gmv_fgx,rmvyfastcnt,aplsum32rgn);
	pOutput->u8_dh_condition_eagle_status = 0;
	
	if(cond_eagle_mv && cond_eagle_dtl && cond_eagle_apl)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x1);
		holdframe_eagle =100;
		eaglepattern = 1;
		pOutput->u8_dh_condition_eagle_status = 1;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 17, 17, 0x1);
		path = 0xf;
	}
	else if(holdframe_eagle>0)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x1);
		holdframe_eagle--;
		eaglepattern = 1;

		pOutput->u8_dh_condition_eagle_status = 1;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 17, 17, 0x1);
		path = 0xf;
	}
	else if(eaglepattern)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x0);
		eaglepattern = 0;
		
		pOutput->u8_dh_condition_eagle_status = 0;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 17, 17, 0x0);
		path = 0x0;

	}


	//cond_huapi_mv = gmv_bgx>100 && u11_gmv_mvx>110 && _ABS_(u10_gmv_mvy)<15 && _ABS_(gmv_fgx)<20 && _ABS_(gmv_fgy)<20;
	//cond_huapi_dtl = Dtl_array[15]<10000 && Dtl_array[0]>5000 && Dtl_array[0]<20000 && Dtl_array[7]<20000&&  Dtl_array[8]<10000 && Dtl_array[16]<10000; 
	//cond_huapi_apl = aplsum32rgn>1200000 && aplsum32rgn<1800000 && rgn_apli[31]>50000 && rgn_apli[31]<90000 && rgn_apli[29]<20000;
	
	cond_huapi_mv = gmv_bgx>60 && u11_gmv_mvx>60 && _ABS_(u10_gmv_mvy)<15 && _ABS_(gmv_fgx)<20 && _ABS_(gmv_fgy)<20;
	cond_huapi_dtl = Dtl_array[15]<30000 && Dtl_array[0]>5000 && Dtl_array[0]<20000 && Dtl_array[7]<25000&&  Dtl_array[8]<15000 && Dtl_array[16]<15000 && Dtl_array[31]>30000 && Dtl_array[31]<50000 && Dtl_array[1]<20000 && Dtl_array[2]<20000 ; 
	cond_huapi_apl =Dtl_array[3]<20000 && Dtl_array[4]<25000 && Dtl_array[9]<20000 && Dtl_array[10]<15000 &&  Dtl_array[23]<15000 && Dtl_array[22]<15000 &&   aplsum32rgn>1200000 && aplsum32rgn<2500000 && rgn_apli[31]>50000 && rgn_apli[31]<110000 && rgn_apli[29]<30000 ;


	pOutput->u8_dh_condition_huapi2_status = 0;
	
	if(cond_huapi_mv && cond_huapi_dtl && cond_huapi_apl)
	{
		pOutput->u8_dh_condition_huapi2_status = 1;
		WriteRegister(SOFTWARE3_SOFTWARE3_20_reg, 30, 30, 0x1);
		holdframe_huapi = 80;
		video_huapi = 1;
		path = 0x6;

		
	}
	else	 if(holdframe_huapi>0)
	{
		pOutput->u8_dh_condition_huapi2_status = 1;
		WriteRegister(SOFTWARE3_SOFTWARE3_20_reg, 30, 30, 0x1);
		holdframe_huapi--;
		path = 0x6;

	}
	else if(video_huapi>0)
	{
		pOutput->u8_dh_condition_huapi2_status = 0;
		WriteRegister(SOFTWARE3_SOFTWARE3_20_reg, 30, 30, 0x0);
		video_huapi  = 0;
		path = 0x6;

	}

	  IDT_aplcond = 0;
	  IDT_dtlcond = 0;
	  IDT_aplcond2 = 0;
	  IDT_dtlcond2 = 0;
	  
	  pOutput->u8_dh_condition_IDT_flag =0;
	
	  for(rgnIdx=0;rgnIdx<32;rgnIdx++)
	  {
		  if(rgn_apli[rgnIdx]>120000 && rgn_apli[rgnIdx]<200000)
			  IDT_aplcond++;
		  
		  if(Dtl_array[rgnIdx]>75000 && Dtl_array[rgnIdx]<110000)
			  IDT_dtlcond++;
	  }
	  
	IDT_cond_1 = (gmv_bgx ==0) && (gmv_bgy ==0) &&	_ABS_(gmv_fgy)<5 && (u11_gmv_mvx ==0) && (u10_gmv_mvy == 0) && aplsum32rgn>4650000 && aplsum32rgn<4850000;
	
	  for(rgnIdx=0;rgnIdx<32;rgnIdx++)
	  {
		  if(rgn_apli[rgnIdx]>100000 && rgn_apli[rgnIdx]<180000)
			  IDT_aplcond2++;
		  
		  if(Dtl_array[rgnIdx]>35000 && Dtl_array[rgnIdx]<78000)
			  IDT_dtlcond2++;
	  }
	  
	  IDT_cond_2 = (gmv_bgx ==0) && (gmv_bgy ==0) &&  _ABS_(gmv_fgy)<5 && (u11_gmv_mvx ==0) && (u10_gmv_mvy == 0) &&((aplsum32rgn>3800000 && aplsum32rgn<4200000)||( aplsum32rgn>4550000 && aplsum32rgn<4850000));
	
	  right_rim_pre_IDT=s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_RHT];
	  left_rim_pre_IDT=s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_LFT];
	
	
	ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,10 , 10 , &readval10);
	
	if(!readval10){
	  
	if((IDT_cond_1 && (IDT_dtlcond>30) && (IDT_aplcond>30))||(IDT_cond_2 && (IDT_dtlcond2>30) && (IDT_aplcond2>30)))
	{
		  pOutput->u8_dh_condition_IDT_flag =1;
		  holdframe_IDT = 80;
		  video_IDT = 1;
		  WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,	right_rim_pre_IDT-190);
		  WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8, left_rim_pre_IDT+30);
	
	}
	else if(holdframe_IDT>0)
	{
		  holdframe_IDT--;
		  pOutput->u8_dh_condition_IDT_flag =1;
		  WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,	right_rim_pre_IDT-190);
		  WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8, left_rim_pre_IDT+30);
	}
	else if(video_IDT>0)
	{
		  video_IDT = 0;
		  pOutput->u8_dh_condition_IDT_flag =0;
		  WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17,	right_rim_pre_IDT);
		  WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8, left_rim_pre_IDT);
	
	}
	  }
	
		  //rtd_pr_memc_info("rimbefore:%d %d after:%d %d \n",right_rim_pre_IDT,left_rim_pre_IDT,rightnow,leftnow);
	
	
	

	ReadRegister(SOFTWARE3_SOFTWARE3_20_reg,0, 0 , &readval0);

	if((
		//(pOutput->u8_dh_condition_eagle_status ==1) ||
		(pOutput->u8_dh_condition_C005plane_status == 1 )
		||(pOutput->u8_dh_condition_transporter_status == 1)
		||(pOutput->u8_dh_condition_smallplane_status == 1)
		||(pOutput->u8_dh_condition_dance_status == 1)
		||(pOutput->u8_dh_condition_soccer_status ==1)
		||(pOutput->u8_dh_condition_rotterdam_status == 1)
		||(pOutput->u8_dh_condition_rotterdam2_status ==1)
		||(pOutput->u8_dh_condition_dance_2_status == 1)
		||(pOutput->u8_dh_condition_skating_status == 1)
		||(pOutput->u8_dh_condition_C001soccer_status == 1)
		||(pOutput->u8_dh_condition_30006_status == 1)
		||(pOutput->u8_dh_condition_ridebikegirl_status == 1)
		//||(pOutput->u8_dh_conditio_toleft_bike_status == 1)
		)&& !readval0)
	{
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 11, 11, 0x1);
	}
	else
	{
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 11, 11, 0x0);
	}


		
	ReadRegister(SOFTWARE3_SOFTWARE3_20_reg,31, 31 , &readval31);
	if(readval31)
	{
		rtd_pr_memc_info("%d %d %d %d %d %d %d %d %d %d %d %d %d\n",pOutput->u8_dh_condition_eagle_status,pOutput->u8_dh_condition_C005plane_status,pOutput->u8_dh_condition_transporter_status,pOutput->u8_dh_condition_smallplane_status,pOutput->u8_dh_condition_dance_status,
		pOutput->u8_dh_condition_soccer_status,pOutput->u8_dh_condition_rotterdam_status,pOutput->u8_dh_condition_rotterdam2_status,pOutput->u8_dh_condition_dance_2_status,pOutput->u8_dh_condition_skating_status,pOutput->u8_dh_condition_C001soccer_status,pOutput->u8_dh_condition_30006_status,pOutput->u8_dh_condition_ridebikegirl_status);
	}

	



	if(show7segment) { //d8fc[30]
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, path); //path segment 
	}




    //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, path2);
    
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,8 , 8 , &readval8);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,9 , 9 , &readval9);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,10 , 10 , &readval10);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,11 ,11 , &readval11);

    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,12 , 12 , &readval12);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,13 , 13 , &readval13);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,14 , 14 , &readval14);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,15 , 15 , &readval15);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,16 , 16 , &readval16);

/*
    if(readval8)
    {                
        rtd_pr_memc_info("mv0(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[0],rmvy[0],rmvx[1],rmvy[1],rmvx[2],rmvy[2],rmvx[3],rmvy[3],rmvx[4],rmvy[4],
        rmvx[5],rmvy[5],rmvx[6],rmvy[6],rmvx[7],rmvy[7]);
    }
	else if(holdmframe_toleft_bike>0)
    {
        rtd_pr_memc_info("mv8(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[8],rmvy[8],rmvx[9],rmvy[9],rmvx[10],rmvy[10],rmvx[11],rmvy[11],rmvx[12],rmvy[12],
        rmvx[13],rmvy[13],rmvx[14],rmvy[14],rmvx[15],rmvy[15]);  
    }
	else if(video_toleft_bike)
	{
		video_toleft_bike = 0;
		pOutput->u8_dh_conditio_toleft_bike_status = 0;
		//path = 0x0;

	}
	//rtd_pr_memc_info("cond:%d %d %d \n",toleft_bike_mv,toleft_bike_dtl,toleft_bike_apl);
	//rtd_pr_memc_info("1:%d %d %d %d 2:%d %d %d %d %d 3:%d %d %d %d %d %d %d\n",gmv_bgx,gmv_fgx,u11_gmv_mvx,u10_gmv_mvy,Dtl_array[16],Dtl_array[0],Dtl_array[7],Dtl_array[22],Dtl_array[25],aplsum32rgn,rgn_apli[0],rgn_apli[7],rgn_apli[31],rgn_apli[22],rgn_apli[24],rgn_apli[27]);


	cond30003_mv = gmv_bgx>55 && gmv_bgx<90 && gmv_fgx>10 &&gmv_fgx<50 && _ABS_(gmv_bgy)<5  && _ABS_(gmv_fgy)<5 && aplsum32rgn>1500000 && aplsum32rgn<3000000;
	cond30003_dtl = Dtl_array[7]<20000 && Dtl_array[16]<20000 && Dtl_array[29]<20000 && Dtl_array[30]<20000 && Dtl_array[31]<20000 && Dtl_array[23]<16000 && Dtl_array[6]>20000;
	cond30003_apl = rgn_apli[24]<70000 &&  rgn_apli[25]>50000 &&  rgn_apli[29]>65000 &&  rgn_apli[31]<40000 &&  rgn_apli[8]<30000; 

	
	if(cond30003_mv && cond30003_dtl && cond30003_apl)
	{
		video_3003man= 1;
		holdframe_3003man = 150;
		pOutput->u8_dh_conditio_3003man_status = 1;
		path = 0x1;		
	}
	else if(holdframe_3003man>0)
	{
		holdframe_3003man--;
		pOutput->u8_dh_conditio_3003man_status = 1;
		path = 0x1;
	}
	else if(video_3003man)
	{
		video_3003man = 0;
		pOutput->u8_dh_conditio_3003man_status = 0;
		path = 0x0;
	}
	//rtd_pr_memc_info("codnqiamian:%d %d %d\n",cond30003_mv , cond30003_dtl ,cond30003_apl);
	//rtd_pr_memc_info("1:%d %d %d %d %d   2:%d %d %d %d %d %d %d\n",gmv_bgx,gmv_fgx,gmv_bgy,gmv_fgy,aplsum32rgn,Dtl_array[7],Dtl_array[16],Dtl_array[29],Dtl_array[30],Dtl_array[31],Dtl_array[23],Dtl_array[6]);
	//rtd_pr_memc_info("3:%d %d %d %d %d\n",rgn_apli[24],rgn_apli[25],rgn_apli[29],rgn_apli[31],rgn_apli[8] );


	cond0026_mv =gmv_bgcnt<30000 && (((_ABS_(gmv_fgx)+_ABS_(gmv_fgy))>40)  ||  ((_ABS_(gmv_bgx)+_ABS_(gmv_bgy))>50)) && u11_gmv_mvx>25 && _ABS_(u10_gmv_mvy)<20 && aplsum32rgn>1500000 && aplsum32rgn<3000000;
	cond0026_dtl = Dtl_array[0]<20000 && Dtl_array[1]<20000 && Dtl_array[31]<10000 && Dtl_array[25]<20000 && Dtl_array[24]<30000  && Dtl_array[1]<30000 && Dtl_array[2]<30000 && Dtl_array[3]<30000;
	cond0026_apl = rgn_apli[0]>50000 &&  rgn_apli[0]<90000 && rgn_apli[8]>50000 && rgn_apli[27]<60000 && rgn_apli[7]>50000; 

	pOutput->u8_dh_conditio_0026_1_status = 0;

	if(cond0026_mv && cond0026_dtl && cond0026_apl)
	{
		holdframe_0026_1=45;
		video_0026_1 = 1;
		pOutput->u8_dh_conditio_0026_1_status = 1;
		path = 0x2;

	}
	else if(holdframe_0026_1>0)
	{
		holdframe_0026_1--;
		video_0026_1 = 1;
		pOutput->u8_dh_conditio_0026_1_status = 1;
		path = 0x2;

	}
	else if(video_0026_1>0)
	{
		video_0026_1 = 0;
		pOutput->u8_dh_conditio_0026_1_status = 0;
		path = 0x0;
	}


	//--------------------- insect 1080_24HZ_Pendulum ------------------------//
	insect_mv = (u11_gmv_mvx == 0) && (u10_gmv_mvy == 0)&&((_ABS_(gmv_fgx)>50)||(_ABS_(gmv_fgx)<10) )&& _ABS_(gmv_bgx)<10 && (gmv_bgy==0);
	insect_dtl = Dtl_array[1]<1000 && Dtl_array[2]<1000 && Dtl_array[4]<5000 && Dtl_array[7]>8000  && Dtl_array[7]<30000 && Dtl_array[25]<1000 && Dtl_array[26]<5000 && Dtl_array[24]>3000 && Dtl_array[24]<5000;
	insect_apl = aplsum32rgn>5000000 && aplsum32rgn<6200000 && rgn_apli[24]>150000 && rgn_apli[24]<190000 && rgn_apli[31]>180000 && rgn_apli[31]<220000;

	//biaozhun
	insect_dtl2 = Dtl_array[1]<5000 && Dtl_array[2]<5000 && Dtl_array[4]<8000 && Dtl_array[7]>10000  && Dtl_array[7]<30000 && Dtl_array[25]<1000 && Dtl_array[26]<5000 && Dtl_array[24]>3000 && Dtl_array[24]<6000;
	insect_apl2 = aplsum32rgn>4500000 && aplsum32rgn<5500000 && rgn_apli[24]>130000 && rgn_apli[24]<170000 && rgn_apli[31]>170000 && rgn_apli[31]<200000;

	//jieneng
	insect_dtl3 = Dtl_array[1]<5000 && Dtl_array[2]<5000 && Dtl_array[4]<8000 && Dtl_array[7]>10000  && Dtl_array[7]<30000 && Dtl_array[25]<1000 && Dtl_array[26]<5000 && Dtl_array[24]>3000 && Dtl_array[24]<6000;
	insect_apl3 = aplsum32rgn>4500000 && aplsum32rgn<5500000 && rgn_apli[24]>130000 && rgn_apli[24]<170000 && rgn_apli[31]>160000 && rgn_apli[31]<190000;

	insect_1 = insect_mv && insect_apl && insect_dtl;
	insect_2 = insect_mv && insect_apl2 && insect_dtl2;
	insect_3 = insect_mv && insect_apl3 && insect_dtl3;




	if(insect_1 || insect_2 || insect_3)
	{
		holdframe_insect = 140;
		video_insect = 1;
		pOutput->u8_dh_condition_insect_status = 1;
		path = 0x9;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 20, 20, 0x1);


	}
	else if(holdframe_insect>0)
	{
		holdframe_insect--;
		video_insect = 1;
		pOutput->u8_dh_condition_insect_status = 1;
		path = 0x9;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 20, 20, 0x1);

	}
	else if(video_insect>0)
	{
		video_insect = 0;
		pOutput->u8_dh_condition_insect_status = 0;
		path = 0x0;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 20, 20, 0x0);
	}


	//---------------------  eagle dehal on---------------------------//
	for(rgnIdx=0;rgnIdx<32;rgnIdx++)
	{
		if(rmvy[rgnIdx]<-60 &&  _ABS_(rmvx[rgnIdx])<15)
			rmvyfastcnt++;
		if(rgn_apli[rgnIdx]>100000 && rgn_apli[rgnIdx]<200000)
			aplbigcnt++;
	}
	for(rgnIdx=0;rgnIdx<16;rgnIdx++)
	{
		if(Dtl_array[rgnIdx]<5000)
			dtlsmallcnt++;
	}
	//cond_eagle_mv = gmv_bgy<-60 &&  gmv_bgx>-15 && gmv_bgx<=0 && _ABS_(gmv_fgx)<10  && rmvyfastcnt>=25 && aplsum32rgn>3500000 && aplsum32rgn<4800000;
	//cond_eagle_dtl = dtlsmallcnt>15 && Dtl_array[18]>10000 && Dtl_array[18]<25000 && Dtl_array[19]>15000 && Dtl_array[19]<35000 && Dtl_array[31]<1000;
	//cond_eagle_apl = aplbigcnt>=30;
	
	cond_eagle_mv = gmv_bgy<-60 &&  gmv_bgx>-15 && gmv_bgx<=0 && _ABS_(gmv_fgx)<10  &&  aplsum32rgn>4000000 && aplsum32rgn<5000000;
	cond_eagle_dtl = dtlsmallcnt>12 && Dtl_array[18]>10000 && Dtl_array[18]<30000 && Dtl_array[19]>15000 && Dtl_array[19]<35000 && Dtl_array[31]<5000;
	cond_eagle_apl = aplbigcnt>=25;

	//rtd_pr_memc_info("cond:%d %d %d  cnt:%d %d %d\n",cond_eagle_mv,cond_eagle_dtl,cond_eagle_apl,rmvyfastcnt,dtlsmallcnt,aplbigcnt);
	//rtd_pr_memc_info("mv:%d %d %d cnt:%d %d\n",gmv_bgy,gmv_bgx,gmv_fgx,rmvyfastcnt,aplsum32rgn);
	pOutput->u8_dh_condition_eagle_status = 0;
	
	if(cond_eagle_mv && cond_eagle_dtl && cond_eagle_apl)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x1);
		holdframe_eagle =100;
		eaglepattern = 1;

		pOutput->u8_dh_condition_eagle_status = 1;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 17, 17, 0x1);
		path = 0xf;
	}
	else if(holdframe_eagle>0)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x1);
		holdframe_eagle--;
		eaglepattern = 1;

		pOutput->u8_dh_condition_eagle_status = 1;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 17, 17, 0x1);
		path = 0xf;
	}
	else if(eaglepattern)
	{
		//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 20, 23, 0x0);
		eaglepattern = 0;
		
		pOutput->u8_dh_condition_eagle_status = 0;
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 17, 17, 0x0);
		path = 0x0;

	}

	cond_huapi_mv = gmv_bgx>100 && u11_gmv_mvx>110 && _ABS_(u10_gmv_mvy)<15 && _ABS_(gmv_fgx)<20 && _ABS_(gmv_fgy)<20;
	cond_huapi_dtl = Dtl_array[15]<10000 && Dtl_array[0]>5000 && Dtl_array[0]<20000 && Dtl_array[7]<20000&&  Dtl_array[8]<10000 && Dtl_array[16]<10000; 
	cond_huapi_apl = aplsum32rgn>1200000 && aplsum32rgn<1800000 && rgn_apli[31]>50000 && rgn_apli[31]<90000 && rgn_apli[29]<20000;


	pOutput->u8_dh_condition_huapi2_status = 0;
	
	if(cond_huapi_mv && cond_huapi_dtl && cond_huapi_apl)
	{
		pOutput->u8_dh_condition_huapi2_status = 1;
		WriteRegister(SOFTWARE3_SOFTWARE3_20_reg, 30, 30, 0x1);
		holdframe_huapi = 80;
		video_huapi = 1;
		path = 0x6;

		
	}
	else	 if(holdframe_huapi>0)
	{
		pOutput->u8_dh_condition_huapi2_status = 1;
		WriteRegister(SOFTWARE3_SOFTWARE3_20_reg, 30, 30, 0x1);
		holdframe_huapi--;
		path = 0x6;

	}
	else if(video_huapi>0)
	{
		pOutput->u8_dh_condition_huapi2_status = 0;
		WriteRegister(SOFTWARE3_SOFTWARE3_20_reg, 30, 30, 0x0);
		video_huapi  = 0;
		path = 0x6;

	}







	ReadRegister(SOFTWARE3_SOFTWARE3_20_reg,0, 0 , &readval0);




	

	if((
		//(pOutput->u8_dh_condition_eagle_status ==1) ||
		(pOutput->u8_dh_condition_C005plane_status == 1 )
		||(pOutput->u8_dh_condition_transporter_status == 1)
		||(pOutput->u8_dh_condition_smallplane_status == 1)
		||(pOutput->u8_dh_condition_dance_status == 1)
		||(pOutput->u8_dh_condition_soccer_status ==1)
		||(pOutput->u8_dh_condition_rotterdam_status == 1)
		||(pOutput->u8_dh_condition_rotterdam2_status ==1)
		||(pOutput->u8_dh_condition_dance_2_status == 1)
		||(pOutput->u8_dh_condition_skating_status == 1)
		||(pOutput->u8_dh_condition_C001soccer_status == 1)
		||(pOutput->u8_dh_condition_30006_status == 1)
		||(pOutput->u8_dh_condition_ridebikegirl_status == 1)
		//||(pOutput->u8_dh_conditio_toleft_bike_status == 1)
		)&& !readval0)
	{
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 11, 11, 0x1);
	}
	else
    {
		WriteRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg, 11, 11, 0x0);
    }


		
	ReadRegister(SOFTWARE3_SOFTWARE3_20_reg,31, 31 , &readval31);
	if(readval31)
    {
		rtd_pr_memc_info("%d %d %d %d %d %d %d %d %d %d %d %d %d\n",pOutput->u8_dh_condition_eagle_status,pOutput->u8_dh_condition_C005plane_status,pOutput->u8_dh_condition_transporter_status,pOutput->u8_dh_condition_smallplane_status,pOutput->u8_dh_condition_dance_status,
		pOutput->u8_dh_condition_soccer_status,pOutput->u8_dh_condition_rotterdam_status,pOutput->u8_dh_condition_rotterdam2_status,pOutput->u8_dh_condition_dance_2_status,pOutput->u8_dh_condition_skating_status,pOutput->u8_dh_condition_C001soccer_status,pOutput->u8_dh_condition_30006_status,pOutput->u8_dh_condition_ridebikegirl_status);
    }  

    



























	if(show7segment) { //d8fc[30]
		WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 8, 11, path); //path segment 
	}
   
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,12 , 12 , &readval12);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,13 , 13 , &readval13);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,14 , 14 , &readval14);
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,10 , 10 , &readval10);


    if(readval10)
    {       // rtd_pr_memc_info("mv0(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[0],rmvy[0],rmvx[1],rmvy[1],rmvx[2],rmvy[2],rmvx[3],rmvy[3],rmvx[4],rmvy[4],
             //           rmvx[5],rmvy[5],rmvx[6],rmvy[6],rmvx[7],rmvy[7]);

         // rtd_pr_memc_info("mv8(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[8],rmvy[8],rmvx[9],rmvy[9],rmvx[10],rmvy[10],rmvx[11],rmvy[11],rmvx[12],rmvy[12],
         //    rmvx[13],rmvy[13],rmvx[14],rmvy[14],rmvx[15],rmvy[15]);  
	//	              rtd_pr_memc_info("mv16(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[16],rmvy[16],rmvx[17],rmvy[17],rmvx[18],rmvy[18],rmvx[19],rmvy[19],rmvx[20],rmvy[20],
        //   rmvx[21],rmvy[21],rmvx[22],rmvy[22],rmvx[23],rmvy[23]);
       // rtd_pr_memc_info("mv24(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[24],rmvy[24],rmvx[25],rmvy[25],rmvx[26],rmvy[26],rmvx[27],rmvy[27],rmvx[28],rmvy[28],
       //    rmvx[29],rmvy[29],rmvx[30],rmvy[30],rmvx[31],rmvy[31]);

	rtd_pr_memc_info("1:%d %d %d %d %d\n",aplsum32rgn,gmv_bgx,gmv_fgy,u11_gmv_mvx,u10_gmv_mvy);
	rtd_pr_memc_info("2:%d %d %d %d %d %d %d %d %d\n",Dtl_array[0],Dtl_array[1],Dtl_array[5], Dtl_array[16],Dtl_array[24],Dtl_array[25], Dtl_array[26],Dtl_array[29],Dtl_array[31]);
	//blkoff_3 = rgn_apli[11]<200000 && rgn_apli[14]>40000 && rgn_apli[14]<210000 && rgn_apli[22]>20000 && rgn_apli[22]<160000;
	rtd_pr_memc_info("3:%d %d %d\n",rgn_apli[31],rgn_apli[23],Dtl_array[6]);


    }
    if(readval12)
    {
        //rtd_pr_memc_info("bgcnt:%d mv:%d fgcnt:%d cnt:%d bgmv(%d %d)fgmv(%d %d) gmv(%d %d) \n",gmv_bgcnt,mv_diff_large,fgcnt_num,cnt_bgcnt_num,gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy,u11_gmv_mvx,u10_gmv_mvy);
        //rtd_pr_memc_info("gmv(%d %d)bgcnt%d apl:%d %d (%d %d %d)\n",u11_gmv_mvx,u10_gmv_mvy,gmv_bgcnt,(bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/bg_apl_sum,bg_apl_seg7_cnt*1000/bg_apl_sum,
        //mv_diff_large,fgcnt_num,cnt_bgcnt_num);
        rtd_pr_memc_info("bgcnt:%d bgmv(%d %d)fgmv(%d %d) gmv(%d %d)apl:%d \n",gmv_bgcnt,gmv_bgx,gmv_bgy,gmv_fgx,gmv_fgy,u11_gmv_mvx,u10_gmv_mvy,aplsum32rgn);
        //rtd_pr_memc_info("%d %d\n",gmv_bgcnt,rngbgcnt_change);
    }
            
    if(readval13)
    {
        rtd_pr_memc_info("-----start-----");
        rtd_pr_memc_info("0-7dtl:%d %d %d %d %d %d %d %d\n", Dtl_array[0] ,Dtl_array[1],Dtl_array[2],Dtl_array[3],Dtl_array[4],Dtl_array[5],Dtl_array[6],Dtl_array[7]);
        rtd_pr_memc_info("8-15dtl:%d %d %d %d %d %d %d %d\n", Dtl_array[8] ,Dtl_array[9],Dtl_array[10],Dtl_array[11],Dtl_array[12],Dtl_array[13],Dtl_array[14],Dtl_array[15]);
        rtd_pr_memc_info("16-23dtl:%d %d %d %d %d %d %d %d\n", Dtl_array[16] ,Dtl_array[17],Dtl_array[18],Dtl_array[19],Dtl_array[20],Dtl_array[21],Dtl_array[22],Dtl_array[23]);
        rtd_pr_memc_info("24-31dtl:%d %d %d %d %d %d %d %d\n", Dtl_array[24] ,Dtl_array[25],Dtl_array[26],Dtl_array[27],Dtl_array[28],Dtl_array[29],Dtl_array[30],Dtl_array[31]);
                      
        //rtd_pr_memc_info("%d %d \n",pOutput->u8_dh_condition_bypass_check,pOutput->u8_dh_condition_dhbypass_hold_cnt);
        //rtd_pr_memc_info("%d bg:%d fg:%d %d \n",gmv_bgcnt,(bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum,(fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum,aplsum32rgn);
        //rtd_pr_memc_info("mv0(%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[0],rmvy[0],rmvx[8],rmvy[8],rmvx[16],rmvy[16],rmvx[24],rmvy[24]);
        //rtd_pr_memc_info("%d %d\n",cond26_1,cond26_2);
    
        //rtd_pr_memc_info("mv0(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[0],rmvy[0],rmvx[1],rmvy[1],rmvx[2],rmvy[2],rmvx[3],rmvy[3],rmvx[4],rmvy[4],
        //                rmvx[5],rmvy[5],rmvx[6],rmvy[6],rmvx[7],rmvy[7]);
        //rtd_pr_memc_info("aplmid:%d %d %d %d\n",rgn_aplp[11],rgn_aplp[12],rgn_aplp[19],rgn_aplp[20]);
    } 
    if(readval14)
    {
        //  rtd_pr_memc_info("mv8(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[8],rmvy[8],rmvx[9],rmvy[9],rmvx[10],rmvy[10],rmvx[11],rmvy[11],rmvx[12],rmvy[12],
        //    rmvx[13],rmvy[13],rmvx[14],rmvy[14],rmvx[15],rmvy[15]);  
        //rtd_pr_memc_info("%d bg:%d fg:%d %d \n",gmv_bgcnt,(bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum,(fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum,aplsum32rgn);
                       
        // rtd_pr_memc_info("%d fgmv(%d %d)\n",gmv_bgcnt,gmv_fgx,gmv_fgy);
        //rtd_pr_memc_info("-----------start----------\n");
        //rtd_pr_memc_info("bg:%d %d %d %d\n",bgcnt_array[0],bgcnt_array[1],bgcnt_array[2],bgcnt_array[3]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",bgcnt_array[8],bgcnt_array[9],bgcnt_array[10],bgcnt_array[11]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",bgcnt_array[16],bgcnt_array[17],bgcnt_array[18],bgcnt_array[19]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",bgcnt_array[24],bgcnt_array[25],bgcnt_array[26],bgcnt_array[27]);
        //rtd_pr_memc_info("-----------start----------\n");
        //rtd_pr_memc_info("bg:%d %d %d %d\n",bgcnt_array[4],bgcnt_array[5],bgcnt_array[6],bgcnt_array[7]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",bgcnt_array[12],bgcnt_array[13],bgcnt_array[14],bgcnt_array[15]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",bgcnt_array[20],bgcnt_array[21],bgcnt_array[22],bgcnt_array[23]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",bgcnt_array[28],bgcnt_array[29],bgcnt_array[30],bgcnt_array[31]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",rgn_apli[2],rgn_apli[3],rgn_apli[4],rgn_apli[5]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",rgn_apli[10],rgn_apli[11],rgn_apli[12],rgn_apli[13]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",rgn_apli[18],rgn_apli[19],rgn_apli[20],rgn_apli[21]);
        //rtd_pr_memc_info("bg:%d %d %d %d\n",rgn_apli[26],rgn_apli[27],rgn_apli[28],rgn_apli[29]);
    
        rtd_pr_memc_info("-----------start----------\n");
        rtd_pr_memc_info("0-7apl:%d %d %d %d %d %d %d %d\n", rgn_apli[0] ,rgn_apli[1],rgn_apli[2],rgn_apli[3],rgn_apli[4],rgn_apli[5],rgn_apli[6],rgn_apli[7]);
        rtd_pr_memc_info("8-15apl:%d %d %d %d %d %d %d %d\n", rgn_apli[8] ,rgn_apli[9],rgn_apli[10],rgn_apli[11],rgn_apli[12],rgn_apli[13],rgn_apli[14],rgn_apli[15]);
        rtd_pr_memc_info("16-23apl:%d %d %d %d %d %d %d %d\n", rgn_apli[16] ,rgn_apli[17],rgn_apli[18],rgn_apli[19],rgn_apli[20],rgn_apli[21],rgn_apli[22],rgn_apli[23]);
        rtd_pr_memc_info("24-31apl:%d %d %d %d %d %d %d %d\n", rgn_apli[24] ,rgn_apli[25],rgn_apli[26],rgn_apli[27],rgn_apli[28],rgn_apli[29],rgn_apli[30],rgn_apli[31]);
        // rtd_pr_memc_info("%d %d %d \n",gmv_bgcnt,bgcnt_full_sum,(bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*100/bg_apl_sum);
        //rtd_pr_memc_info("mv16(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[16],rmvy[16],rmvx[17],rmvy[17],rmvx[18],rmvy[18],rmvx[19],rmvy[19],rmvx[20],rmvy[20],
        //  rmvx[21],rmvy[21],rmvx[22],rmvy[22],rmvx[23],rmvy[23]);
    }
                       
    if(readval16)
    {
        //rtd_pr_memc_info("bg:%d fg:%d\n",(bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*100/bg_apl_sum,(fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*100/fg_apl_sum);
        //  rtd_pr_memc_info("mv23(%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d) (%d %d)\n",rmvx[24],rmvy[24],rmvx[25],rmvy[25],rmvx[26],rmvy[26],rmvx[27],rmvy[27],rmvx[28],rmvy[28],
        //   rmvx[29],rmvy[29],rmvx[30],rmvy[30],rmvx[31],rmvy[31]);
        // rtd_pr_memc_info("cond:%d %d %d %d \n",cond3841,cond3842,cond3843,cond3844);
        //rtd_pr_memc_info("-----start-----");
        //rtd_pr_memc_info("0-8dtl:%d %d %d %d %d %d %d %d\n", Dtl_array[0],Dtl_array[1],Dtl_array[2],Dtl_array[3],Dtl_array[4],Dtl_array[5],Dtl_array[6],Dtl_array[7]);
        //rtd_pr_memc_info("9-15dtl:%d %d %d\n",Dtl_array[8],Dtl_array[9],Dtl_array[10]);
        //rtd_pr_memc_info("16-23dtl:%d %d %d\n",Dtl_array[16],Dtl_array[17],Dtl_array[18]);
        //rtd_pr_memc_info("24-31dtl:%d %d %d\n",Dtl_array[24],Dtl_array[25],Dtl_array[26]);
        //rtd_pr_memc_info("%d bg:%d %d fg:%d %d\n",gmv_bgcnt,(bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum,(bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt)*100/bg_apl_sum,(fg_apl_seg6_cnt+fg_apl_seg7_cnt)*100/fg_apl_sum,(fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum);
        rtd_pr_memc_info("bgcnt:5:%d 6:%d 7:%d 11:%d 12:%d 14:%d 15:%d 16:%d 24:%d\n",rgn_bgcnt[5],rgn_bgcnt[6],rgn_bgcnt[7],rgn_bgcnt[11],rgn_bgcnt[12],rgn_bgcnt[14],rgn_bgcnt[15],rgn_bgcnt[16],rgn_bgcnt[24]);
        rtd_pr_memc_info("dtl:0:%d 1:%d 3:%d 5:%d 7:%d 8:%d 10:%d 13:%d 15:%d 16:%d 21:%d 24:%d 26:%d 27:%d 31:%d\n",Dtl_array[0],Dtl_array[1],Dtl_array[3],Dtl_array[5],Dtl_array[7],Dtl_array[8],Dtl_array[10],Dtl_array[13],Dtl_array[15],Dtl_array[16],Dtl_array[21],Dtl_array[24],Dtl_array[26],Dtl_array[27],Dtl_array[31]);
        rtd_pr_memc_info("apl:0:%d 1:%d 2:%d 3:%d 5:%d 3:%d 30:%d 31:%d\n",rgn_apli[0],rgn_apli[1],rgn_apli[2],rgn_apli[3],rgn_apli[5],rgn_apli[6],rgn_apli[30],rgn_apli[31]);

    }  
*/
}
VOID MEMC_dehalo_newalgo_bgmaskBase_Proc(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
    const _PQLCONTEXT *s_pContext = GetPQLContext();
    static signed short gmv_bgx = 0;
    static signed short gmv_bgy = 0;
    static signed short gmv_fgx = 0;
    static signed short gmv_fgy = 0;
    static signed short pre_gmv_fgx = 0;
    static signed short pre_gmv_fgy = 0;
    signed short gmv_bgcnt;   
    signed short bg_apl_seg0_cnt,bg_apl_seg1_cnt,bg_apl_seg2_cnt,bg_apl_seg3_cnt,bg_apl_seg4_cnt,bg_apl_seg5_cnt,bg_apl_seg6_cnt,bg_apl_seg7_cnt;
    signed short fg_apl_seg0_cnt,fg_apl_seg1_cnt,fg_apl_seg2_cnt,fg_apl_seg3_cnt,fg_apl_seg4_cnt,fg_apl_seg5_cnt,fg_apl_seg6_cnt,fg_apl_seg7_cnt;
    int fg_apl_sum;
    int bg_apl_sum;

    int rgn_bgcnt[32]={0};
    static int rgn_bgcnt_pre[16]={0}; 
    int rgn_diff[16]={0};
    int temp;
    int m,n;
    int index_num1,index_num2,index_num3;
    int index_max,index_max2,index_max3;
    int readval16;  //for debug dehalo bypass 
    int bgcnt_array[32]={0};
    int bgcnt_array_duibi[32]={0};
    int rgnmvy_bg[32]={0};
    int rgnmvx_bg[32]={0};
    int rgnmvy_fg[32]={0};
    int rgnmvx_fg[32]={0}; 
    int rgn_has_bgfg_flag[32] = {0};
    int rgn_apli[32] = {0};
    int rgn_aplp[32] = {0};
    int rmv_mvx[32]={0};
    int rmv_mvy[32]={0}; 
    int rmv_cnt[32] = {0}; 
    int rmv_diff_sum = 0;
    
    int i;
    int rgnIdx ;
    int u32_RB_val ;
    int dh_is_bypass;

    int hold_in_cnt = 8;

    int sum_bgcnt=0, top_sum_bgcnt = 0, avr_bgcnt=0,fgmvcnt,bgmvcnt;
    
    static signed int mid_bgcnt_array[6] = {0}; 
    unsigned int mid_bgcnt_length = 6;
    int diff[5]={0};
    int avr_diff;
    int u8_dh_condition_dhbypass_en;
    int u8_dh_condition_fadeinout_dhbypass_en;
    
    int dh_bypass;
    int predflag_en;
    int gmv_y0;
    int pred_y0;
    bool SCFlag;
    int auto_extend_mvdiff_th; 

    int apl_mt_cnt =0;
    int apl_lt_cnt =0;
    static int apl_mt_array[5]={0};
    static int apl_lt_array[5]={0};
    int apl_mt_length=5;
    int apl_lt_length=5;    
    int apl_mt_rgncnt_th=24;
    int bypass2=0;

    static int rgn_bgcnt_diff[16][6]={0};
    int diff_rgn[16][5]={0};

    int sum[16]={0};
    int avr[16]={0};

    static int glb_bgcnt[3]={0};
    int glb_bgcnt_length=3;
    int glb_bgcnt_large;
    int dark_flash_big_fg = 0;
    int bad_light_off = 0; //for #115
    int avg_bg_diff_cond = 0;

    int logo_rgn_cnt[32]={0};
    int sum_logo_cnt=0;

    int fwcontrol_13; //for 13
    int fwcontrol_185; //for 185
    int fwcontrol_299; //for 299
    int fwcontrol_189; //for 189
    int fwcontrol_156;
     int fwcontrol_214; 
    int fwcontrol_306; 
    bool fwcontrol_227;
    int fwcontrol_183;
    int fwcontrol_49;
    int fwcontrol_163;
    int fwcontrol_432;

    bool apl_229_cond0, apl_229_cond1, apl_229_cond2, apl_229_cond3;

    bool white_pro1, white_pro2, white_pro3;

    bool flag_133, flag_136_1, flag_136_2, flag_136_3, flag_136_4;

    static int rgn_logo_cnt_cont[3]={0};
    int logo_cnt_length=3;

    int sum_apl =0;
    int avr_mv_x_left,left_rim_pre,val;
    static int rim_hold_cnt_left, rim_hold_cnt_left_for432;
	static int video_rimleft;
    int tinybgcnt_hold_in_cnt=1;
    int tiny_bgcnt_flag;
    int smallcnt_y_big_flag;

    static int video185_verhold;
    bool  bNotNear12 , bNotNear23 , bNotNear13  ;
    int Val_rgn1 , Val_rgn2 , Val_rgn3 , maxIdx , minIdx , maxVal , minVal ,gmvval ;
	int dh_bypass_forrotterdam_33s;
	
    gmv_bgcnt=s_pContext->_output_read_comreg.me1_gmv_bgcnt;
    
	bg_apl_seg0_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg0_cnt;
	bg_apl_seg1_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg1_cnt;
	bg_apl_seg2_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg2_cnt;
	bg_apl_seg3_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg3_cnt;
	bg_apl_seg4_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg4_cnt;
	bg_apl_seg5_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg5_cnt;
	bg_apl_seg6_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg6_cnt;
	bg_apl_seg7_cnt=s_pContext->_output_read_comreg.dh_bg_apl_seg7_cnt;
	fg_apl_seg0_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg0_cnt;
	fg_apl_seg1_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg1_cnt;
	fg_apl_seg2_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg2_cnt;
	fg_apl_seg3_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg3_cnt;
	fg_apl_seg4_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg4_cnt;
	fg_apl_seg5_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg5_cnt;
	fg_apl_seg6_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg6_cnt;
	fg_apl_seg7_cnt=s_pContext->_output_read_comreg.dh_fg_apl_seg7_cnt;
	
	fg_apl_sum=fg_apl_seg0_cnt+fg_apl_seg1_cnt+fg_apl_seg2_cnt+fg_apl_seg3_cnt+fg_apl_seg4_cnt+fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt;
	bg_apl_sum=bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt+bg_apl_seg3_cnt+bg_apl_seg4_cnt+bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt;

	//32 rgn bgcnt


    //rtd_pr_memc_info("-----------start----------\n");
    //rtd_pr_memc_info("0-7fb:%d %d %d %d %d %d %d %d\n", fb_cnt_array[0] ,fb_cnt_array[1],fb_cnt_array[2],fb_cnt_array[3],fb_cnt_array[4],fb_cnt_array[5],fb_cnt_array[6],fb_cnt_array[7]);
    //rtd_pr_memc_info("8-15dfb:%d %d %d %d %d %d %d %d\n", fb_cnt_array[8] ,fb_cnt_array[9],fb_cnt_array[10],fb_cnt_array[11],fb_cnt_array[12],fb_cnt_array[13],fb_cnt_array[14],fb_cnt_array[15]);
    //rtd_pr_memc_info("16-23fb:%d %d %d %d %d %d %d %d\n", fb_cnt_array[16] ,fb_cnt_array[17],fb_cnt_array[18],fb_cnt_array[19],fb_cnt_array[20],fb_cnt_array[21],fb_cnt_array[22],fb_cnt_array[23]);
    //rtd_pr_memc_info("24-31fb:%d %d %d %d %d %d %d %d\n", fb_cnt_array[24] ,fb_cnt_array[25],fb_cnt_array[26],fb_cnt_array[27],fb_cnt_array[28],fb_cnt_array[29],fb_cnt_array[30],fb_cnt_array[31]);

    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN00_BG_reg+4*rgnIdx ,0,9,&u32_RB_val);   
        rgn_bgcnt[rgnIdx] = u32_RB_val ;
        bgcnt_array[rgnIdx] = u32_RB_val ;
        bgcnt_array_duibi[rgnIdx] = u32_RB_val ;
    }
    
    
    for(rgnIdx =8 ; rgnIdx< 24; rgnIdx++)  //calc diff for 130
    {
          sum_bgcnt+=rgn_bgcnt[rgnIdx];
          
    }
    avr_bgcnt=sum_bgcnt/16;

    for(rgnIdx =0 ; rgnIdx< 16; rgnIdx++)  
    {
          top_sum_bgcnt+=rgn_bgcnt[rgnIdx]; 
    }
   
    ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV02_reg, 0, 14, &fgmvcnt);
    ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV02_reg, 16, 30, &bgmvcnt);
    
    updateGMVArray(mid_bgcnt_array,mid_bgcnt_length,avr_bgcnt);
    
    for(rgnIdx =0 ; rgnIdx< 5; rgnIdx++)
    {
          diff[rgnIdx]=_ABS_DIFF_(avr_bgcnt,mid_bgcnt_array[rgnIdx]);
    } 
    avr_diff=(diff[0]+diff[1]+diff[2]+diff[3]+diff[4])/5;

    // -----------------------16rgn bgcnt diff--------------------//
    for(rgnIdx =0 ; rgnIdx< 16; rgnIdx++)
    {
        rgn_diff[rgnIdx]=_ABS_DIFF_(rgn_bgcnt[rgnIdx+8],rgn_bgcnt_pre[rgnIdx]);   
    }


    for(rgnIdx =0 ; rgnIdx< 16; rgnIdx++)  
    {
        //updateGMVArray(rgn_bgcnt_diff[rgnIdx-8],mid_bgcnt_length,rgn_bgcnt[rgnIdx]);
         updateGMVArray2(rgn_bgcnt_diff,mid_bgcnt_length,rgn_bgcnt[rgnIdx+8],rgnIdx);
    }
    for(rgnIdx=8;rgnIdx< 24; rgnIdx++)
    {
        for(i =0 ; i< 5; i++)
        {
          diff_rgn[rgnIdx-8][i]=_ABS_DIFF_(rgn_bgcnt[rgnIdx],rgn_bgcnt_diff[rgnIdx-8][i]);
        } 
        
    }
    
    for(rgnIdx=0;rgnIdx< 16; rgnIdx++)
    {
        for(i =0 ; i< 5; i++)
        {
            sum[rgnIdx]+=diff_rgn[rgnIdx][i];
        }
    }
    
    for(rgnIdx=0;rgnIdx< 16; rgnIdx++)
    {
        avr[rgnIdx]=sum[rgnIdx]/5;  
    }


    //----------------------------------apl_diff---------------------//
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_00_reg+4*rgnIdx ,0,19,&u32_RB_val);   
        rgn_apli[rgnIdx]=u32_RB_val;
    } 
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_80_reg+4*rgnIdx ,0,19,&u32_RB_val);   
        rgn_aplp[rgnIdx]=u32_RB_val;
    }
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {   
        if((rgn_apli[rgnIdx] - rgn_aplp[rgnIdx]) > 1500)
        {
            apl_mt_cnt++;
        }
    }
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {   
        if((rgn_aplp[rgnIdx] - rgn_apli[rgnIdx]) > 1500)
        {
            apl_lt_cnt++;
        }
    }
    //rtd_pr_memc_info("apl_mt_cnt=%d  apl_lt_cnt=%d\n",apl_mt_cnt,apl_lt_cnt);  
    updateGMVArray(apl_mt_array,apl_mt_length,apl_mt_cnt);       //fadeinout  new
    updateGMVArray(apl_lt_array,apl_lt_length,apl_lt_cnt);
    ReadRegister(HARDWARE_HARDWARE_26_reg,30,30, &u8_dh_condition_fadeinout_dhbypass_en);
  
    ReadRegister(KME_DEHALO5_PHMV_FIX_35_reg,22,22,&bypass2); 
   

    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        rgn_has_bgfg_flag[rgnIdx] = ((bgcnt_array[rgnIdx] < 880) && (bgcnt_array[rgnIdx] > 150)) ? 1 : 0;
    }

     for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        sum_apl += rgn_apli[rgnIdx];
    }
    //rtd_pr_memc_info("sum_apl=%d\n",sum_apl);
        
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
    	rgnmvx_bg[rgnIdx]=s_pContext->_output_read_comreg.s11_bg_rMV_x_rb[rgnIdx];
        rgnmvy_bg[rgnIdx]=s_pContext->_output_read_comreg.s10_bg_rMV_y_rb[rgnIdx] ;

    }
    
     //rgn fg mv
    for(rgnIdx =0 ;  rgnIdx< 31; rgnIdx++)
    {
        rgnmvx_fg[rgnIdx] = s_pContext->_output_read_comreg.s11_fg_rMV_x_rb[rgnIdx] ; 
        rgnmvy_fg[rgnIdx] = s_pContext->_output_read_comreg.s10_fg_rMV_y_rb[rgnIdx] ; 
    }


    pre_gmv_fgx = gmv_fgx;
    pre_gmv_fgy = gmv_fgy;
    
    gmv_fgx=s_pContext->_output_read_comreg.dh_gmv_fgx;
    gmv_fgy=s_pContext->_output_read_comreg.dh_gmv_fgy;
    gmv_bgx=s_pContext->_output_read_comreg.dh_gmv_bgx;
    gmv_bgy=s_pContext->_output_read_comreg.dh_gmv_bgy;

    //--------------------------------logo_rgn_cnt--------------------------//

    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_LOGO2_KME_LOGO2_80_reg+4*rgnIdx ,20,29,&u32_RB_val);   
        logo_rgn_cnt[rgnIdx]=u32_RB_val; 
    }    
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        sum_logo_cnt+=logo_rgn_cnt[rgnIdx];
    }
    
	for (m = 0; m < 31; m++)
	{	
	    for (n = 0; n < 31 - m; n++)
	    {
	        if (bgcnt_array[n] >bgcnt_array[n+1])
	        {
	            temp = bgcnt_array[n];
	            bgcnt_array[n] = bgcnt_array[n+1];
	            bgcnt_array[n+1] = temp;
	        }
	    }
	}
		
	for(index_num1=0;index_num1<32;index_num1++)
	{
		if(bgcnt_array_duibi[index_num1]==bgcnt_array[0])
		{	
			index_max=index_num1;
		}
	}
			
	for(index_num2=0;index_num2<32;index_num2++)
	{
		if(bgcnt_array_duibi[index_num2]==bgcnt_array[1])
		{	
			index_max2=index_num2;
		}
	}
			
	for(index_num3=0;index_num3<32;index_num3++)
	{
		if(bgcnt_array_duibi[index_num3]==bgcnt_array[2])
		{	
			index_max3=index_num3;
		}			
	}		
    pOutput->region_fg_x=(rgnmvx_fg[index_max]+rgnmvx_fg[index_max2]+rgnmvx_fg[index_max3])/3;
    pOutput->region_fg_y=(rgnmvy_fg[index_max]+rgnmvy_fg[index_max2]+rgnmvy_fg[index_max3])/3;

     //+++++++add 72 13s broken ++++++++//
    bNotNear12 =  (_ABS_DIFF_(index_max, index_max2)!=1&&_ABS_DIFF_(index_max, index_max2)!=8) ;
    bNotNear23 =  (_ABS_DIFF_(index_max, index_max2)!=1&&_ABS_DIFF_(index_max, index_max2)!=8) ;
    bNotNear13=  (_ABS_DIFF_(index_max, index_max2)!=1&&_ABS_DIFF_(index_max, index_max2)!=8) ; 
    Val_rgn1 = _ABS_(rgnmvx_fg[index_max])+_ABS_(rgnmvy_fg[index_max]) ;
    Val_rgn2 = _ABS_(rgnmvx_fg[index_max2])+_ABS_(rgnmvy_fg[index_max2]) ;
    Val_rgn3 = _ABS_(rgnmvx_fg[index_max3])+_ABS_(rgnmvy_fg[index_max3]) ;     //fg region not near  calc max min fg mv value 

    gmvval =   _ABS_(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)+_ABS_( s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);

    if(Val_rgn1>Val_rgn2)
    {
          maxVal = Val_rgn1 ;
          maxIdx = index_max  ;
          minVal = Val_rgn2 ;
          minIdx = index_max2 ; 
    }
    else
    {
          maxVal = Val_rgn2 ;
          maxIdx = index_max2  ;
          minVal = Val_rgn1 ;
          minIdx = index_max ; 
    }
    if(maxVal>Val_rgn3)
    {
        if(minVal>Val_rgn3)
        {
            minVal = Val_rgn3 ;
            minIdx = index_max ; 
        }

    }
    else
    {
            maxVal = Val_rgn3 ;
            maxIdx = index_max3 ;
    }
  //  dirctionCond =(( rgnmvy_fg[maxIdx]>>31) != ( rgnmvy_fg[minIdx]>>31) )||((( rgnmvy_fg[maxIdx]>>31) == ( rgnmvy_fg[minIdx]>>31) ); 
    if(bNotNear12&&bNotNear23&&bNotNear13&&minVal<22&&gmvval<15 &&(maxVal-minIdx)>18&&gmv_bgcnt>29500&&(bg_apl_seg0_cnt+bg_apl_seg1_cnt)<200)
    {
        pOutput->u8_me1_condition_72_face_flag =1 ; 
    }
    else
    {
        pOutput->u8_me1_condition_72_face_flag =0 ; 
    }

	
    //32 rgn mv
    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        
        rmv_mvx[rgnIdx] =    s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[rgnIdx]  ;
        rmv_mvy[rgnIdx] =    s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[rgnIdx]  ;    
        rmv_cnt[rgnIdx] = s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[rgnIdx];
    }
    //---------------------------------------rgn mv diff ------------------------------------//
    for(rgnIdx =0 ; rgnIdx< 31; rgnIdx++)
    {
        rmv_diff_sum += _ABS_DIFF_(rmv_mvx[rgnIdx], rmv_mvx[rgnIdx+1]) + _ABS_DIFF_(rmv_mvy[rgnIdx], rmv_mvy[rgnIdx+1]);
    }
    
    ReadRegister(KME_DEHALO5_PHMV_FIX_35_reg ,23,23,&dh_is_bypass);
    //start get condition
    //----------------------------------- pred_en0 ----------------------------------------//
    if((_ABS_DIFF_(gmv_fgx,gmv_bgx)>150))
    {
        pOutput->u8_dh_pred_en0_in_cnt  = pOutput->u8_dh_pred_en0_in_cnt + 1;
        pOutput->u8_dh_pred_en0_in_cnt = (pOutput->u8_dh_pred_en0_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_pred_en0_in_cnt;
    }
    else
    {
        pOutput->u8_dh_pred_en0_in_cnt = 0;
    }
    if(pOutput->u8_dh_pred_en0_in_cnt  >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_pred_en0=1;
        pOutput->u8_dh_pred_en0_hold_cnt = 30;
    }
     else if(pOutput->u8_dh_pred_en0_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_pred_en0=1;
        pOutput->u8_dh_pred_en0_hold_cnt = pOutput->u8_dh_pred_en0_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_pred_en0=0;
        pOutput->u8_dh_pred_en0_hold_cnt = 0;
    }

    //--------------------------------- divide_en ----------------------------------------//
    if((_ABS_(gmv_fgx)<40)&&(_ABS_(gmv_fgy)<50))
    {
        pOutput->u8_dh_divide_in_cnt  = pOutput->u8_dh_divide_in_cnt + 1;
        pOutput->u8_dh_divide_in_cnt = (pOutput->u8_dh_divide_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_divide_in_cnt;
    }
    else
    {
        pOutput->u8_dh_divide_in_cnt = 0;
    }
    if(pOutput->u8_dh_divide_in_cnt >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_divide_en=1;
        pOutput->u8_dh_divide_hold_cnt = 30;   
    }
    else if(pOutput->u8_dh_divide_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_divide_en=1;
        pOutput->u8_dh_divide_hold_cnt = pOutput->u8_dh_divide_hold_cnt - 1;   
    }
    else
    {
        pOutput->u8_dh_condition_divide_en=0;
        pOutput->u8_dh_divide_hold_cnt = 0;
    }
    
    //----------------------------- extend_more ----------------------------------------//
    if((_ABS_(gmv_fgx)<40)&&(_ABS_(gmv_bgx)>= 85))
    {
        pOutput->u8_dh_extend_more_in_cnt  = pOutput->u8_dh_extend_more_in_cnt + 1;
        pOutput->u8_dh_extend_more_in_cnt = (pOutput->u8_dh_extend_more_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_extend_more_in_cnt;
    }
    else
    {
        pOutput->u8_dh_extend_more_in_cnt = 0;
    }
    if(pOutput->u8_dh_extend_more_in_cnt >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_extend_more=1;
        pOutput->u8_dh_extend_more_hold_cnt = 30;   
    }
    else if(pOutput->u8_dh_extend_more_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_extend_more=1;
        pOutput->u8_dh_extend_more_hold_cnt = pOutput->u8_dh_extend_more_hold_cnt - 1;   
    }
    else
    {
        pOutput->u8_dh_condition_extend_more=0;
        pOutput->u8_dh_extend_more_hold_cnt = 0;
    }

    //----------------------------- fast hor motion ----------------------------------------//
    if((_ABS_DIFF_(gmv_fgx,gmv_bgx) > 120) && (_ABS_DIFF_(gmv_fgy,gmv_bgy) < 30))
    {
        pOutput->u8_dh_fast_hor_in_cnt  = pOutput->u8_dh_fast_hor_in_cnt + 1;
        pOutput->u8_dh_fast_hor_in_cnt = (pOutput->u8_dh_fast_hor_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_fast_hor_in_cnt;
    }
    else
    {
        pOutput->u8_dh_fast_hor_in_cnt = 0;
    }
    if(pOutput->u8_dh_fast_hor_in_cnt >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_fast_hor=1;
        pOutput->u8_dh_fast_hor_hold_cnt = 10;   
    }
    else if(pOutput->u8_dh_fast_hor_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_fast_hor=1;
        pOutput->u8_dh_fast_hor_hold_cnt = pOutput->u8_dh_fast_hor_hold_cnt - 1;   
    }
    else
    {
        pOutput->u8_dh_condition_fast_hor=0;
        pOutput->u8_dh_fast_hor_hold_cnt = 0;
    }
    
    //------------------------ noslow_easy_extend ----------------------------------------//
    //broken_317_cond = ((gmv_bgx < -30) && (_ABS_(gmv_bgy) < 5) && (((fg_apl_seg0_cnt + fg_apl_seg1_cnt)*100/fg_apl_sum) > 500) && (((fg_apl_seg6_cnt + fg_apl_seg7_cnt)*100/fg_apl_sum) < 100));
    //if((_ABS_DIFF_(gmv_fgx,gmv_bgx) > 60) || ((_ABS_DIFF_(gmv_fgx,gmv_bgx) > 35) && (gmv_bgcnt > 27000))/*for 317*/) // fg bg mvx diff is big
   if(_ABS_DIFF_(gmv_fgx,gmv_bgx) > 35) // fg bg mvx diff is big
    {
        pOutput->u8_dh_noslow_easy_extend_in_cnt  = pOutput->u8_dh_noslow_easy_extend_in_cnt + 1;
        pOutput->u8_dh_noslow_easy_extend_in_cnt = (pOutput->u8_dh_noslow_easy_extend_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_noslow_easy_extend_in_cnt;
    }
    else if((_ABS_DIFF_(gmv_fgx,gmv_bgx) > 8) && ((_ABS_(gmv_fgx)+_ABS_(gmv_bgx)) > 35)) // bg and fg same direction, sum mvx is big
    {
        pOutput->u8_dh_noslow_easy_extend_in_cnt2  = pOutput->u8_dh_noslow_easy_extend_in_cnt2 + 1;
        pOutput->u8_dh_noslow_easy_extend_in_cnt2 = (pOutput->u8_dh_noslow_easy_extend_in_cnt2 > 3) ? 3 : pOutput->u8_dh_noslow_easy_extend_in_cnt2;
    }
    else
    {
        pOutput->u8_dh_noslow_easy_extend_in_cnt = 0;
        pOutput->u8_dh_noslow_easy_extend_in_cnt2 = 0;
    }
    if(pOutput->u8_dh_noslow_easy_extend_in_cnt >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_noslow_easy_extend=1;
        pOutput->u8_dh_noslow_easy_extend_hold_cnt = 10;   
    }
    else if(pOutput->u8_dh_noslow_easy_extend_in_cnt2 >= 3)
    {
        pOutput->u8_dh_condition_noslow_easy_extend=2;
        pOutput->u8_dh_noslow_easy_extend_hold_cnt2 = 10;   
    }
    else if(pOutput->u8_dh_noslow_easy_extend_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_noslow_easy_extend=1;
        pOutput->u8_dh_noslow_easy_extend_hold_cnt = pOutput->u8_dh_noslow_easy_extend_hold_cnt - 1;   
    }
    else if(pOutput->u8_dh_noslow_easy_extend_hold_cnt2 > 0)
    {
        pOutput->u8_dh_condition_noslow_easy_extend=2;
        pOutput->u8_dh_noslow_easy_extend_hold_cnt2 = pOutput->u8_dh_noslow_easy_extend_hold_cnt2 - 1;   
    }
    else
    {
        pOutput->u8_dh_condition_noslow_easy_extend=0;
        pOutput->u8_dh_noslow_easy_extend_hold_cnt = 0;
        pOutput->u8_dh_noslow_easy_extend_hold_cnt2 = 0;
    }
    //rtd_pr_memc_info("incnt(%d) en(%d) hold(%d)\n", pOutput->u8_dh_noslow_easy_extend_in_cnt, pOutput->u8_dh_condition_noslow_easy_extend,  pOutput->u8_dh_noslow_easy_extend_hold_cnt);

    //------------------------------ virtical ----------------------------------------//
    //if(((_ABS_DIFF_(gmv_fgy,gmv_bgy)-_ABS_DIFF_(gmv_fgx,gmv_bgx))>0)||((_ABS_DIFF_(_ABS_(gmv_fgy),_ABS_(gmv_fgx))<15)&&(_ABS_DIFF_(_ABS_(gmv_bgy),_ABS_(gmv_bgx))<10)))
    if(((_ABS_DIFF_(gmv_fgy,gmv_bgy)-_ABS_DIFF_(gmv_fgx,gmv_bgx))>0) || (((_ABS_DIFF_(gmv_fgx,gmv_bgx)-_ABS_DIFF_(gmv_fgy,gmv_bgy))< 8) && (_ABS_DIFF_(gmv_fgy,gmv_bgy) > 15)))
    {
        pOutput->u8_dh_virtical_in_cnt  = pOutput->u8_dh_virtical_in_cnt + 1;
        pOutput->u8_dh_virtical_in_cnt = (pOutput->u8_dh_virtical_in_cnt > 3) ? 3 : pOutput->u8_dh_virtical_in_cnt;
    }
    else
    {
        pOutput->u8_dh_virtical_in_cnt = 0;
    }
    if(pOutput->u8_dh_virtical_in_cnt >= 3)
    {
        pOutput->u8_dh_condition_virtical=1;
        pOutput->u8_dh_virtical_hold_cnt = 20;
    }
    else if(pOutput->u8_dh_virtical_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_virtical=1; 
        pOutput->u8_dh_virtical_hold_cnt = pOutput->u8_dh_virtical_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_virtical=0;
        pOutput->u8_dh_virtical_hold_cnt = 0;
    }

    //------------------------------ large_mv_th_y2 ----------------------------------------//
    if((_ABS_(gmv_fgx)>150)||(_ABS_(gmv_fgy)>150)||(_ABS_(gmv_bgx)>150)||(_ABS_(gmv_bgy)>150))
    {
        pOutput->u8_dh_large_mv_th_y2_in_cnt  = pOutput->u8_dh_large_mv_th_y2_in_cnt + 1;
        pOutput->u8_dh_large_mv_th_y2_in_cnt = (pOutput->u8_dh_large_mv_th_y2_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_large_mv_th_y2_in_cnt;
    }
    else
    {
        pOutput->u8_dh_large_mv_th_y2_in_cnt = 0;
    }
    if(pOutput->u8_dh_large_mv_th_y2_in_cnt >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_large_mv_th_y2=1;
        pOutput->u8_dh_large_mv_th_y2_hold_cnt = 30;
    }
    else if(pOutput->u8_dh_large_mv_th_y2_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_large_mv_th_y2=1;
        pOutput->u8_dh_large_mv_th_y2_hold_cnt = pOutput->u8_dh_large_mv_th_y2_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_large_mv_th_y2=0;
        pOutput->u8_dh_large_mv_th_y2_hold_cnt = 0;
    }

    //------------------------------ small_mv_th_y0 ----------------------------------------//
    if((_ABS_DIFF_(gmv_fgx,gmv_bgx)<=25)&&((_ABS_(gmv_fgx) + _ABS_(gmv_bgx)) < 40))
    {
        pOutput->u8_dh_small_mv_th_y0_in_cnt  = pOutput->u8_dh_small_mv_th_y0_in_cnt + 1;
        pOutput->u8_dh_small_mv_th_y0_in_cnt = (pOutput->u8_dh_small_mv_th_y0_in_cnt > 3) ? 3 : pOutput->u8_dh_small_mv_th_y0_in_cnt;
    }
    else
    {
        pOutput->u8_dh_small_mv_th_y0_in_cnt = 0;
    }
    if(pOutput->u8_dh_small_mv_th_y0_in_cnt >= 3)
    {
        pOutput->u8_dh_condition_small_mv_th_y0=1;
        pOutput->u8_dh_small_mv_th_y0_hold_cnt = 3;
    }
    else if(pOutput->u8_dh_small_mv_th_y0_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_small_mv_th_y0=1;
        pOutput->u8_dh_small_mv_th_y0_hold_cnt = pOutput->u8_dh_small_mv_th_y0_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_small_mv_th_y0=0;
        pOutput->u8_dh_small_mv_th_y0_hold_cnt = 0;
    }

    //------------------------------ slow motion close occl ----------------------------------------//
    if(((_ABS_DIFF_(gmv_fgx,gmv_bgx)<=10) && ((_ABS_(gmv_fgx) + _ABS_(gmv_bgx)) < 15) && ((_ABS_DIFF_(gmv_fgy,gmv_bgy)-_ABS_DIFF_(gmv_fgx,gmv_bgx))<4)) ||
        ((_ABS_DIFF_(gmv_fgy,gmv_bgy)<=13) && ((_ABS_(gmv_fgy) + _ABS_(gmv_bgy)) < 15) && ((_ABS_DIFF_(gmv_fgy,gmv_bgy)-_ABS_DIFF_(gmv_fgx,gmv_bgx))>=4)))
    {
        pOutput->u8_dh_slow_motion_in_cnt  = pOutput->u8_dh_slow_motion_in_cnt + 1;
        pOutput->u8_dh_slow_motion_in_cnt = (pOutput->u8_dh_slow_motion_in_cnt > 2) ? 2 : pOutput->u8_dh_slow_motion_in_cnt;
    }
    else
    {
        pOutput->u8_dh_slow_motion_in_cnt = 0;
    }
    if(pOutput->u8_dh_slow_motion_in_cnt >= 2)
    {
        pOutput->u8_dh_condition_slow_motion=1;
        pOutput->u8_dh_slow_motion_hold_cnt = 0;
    }
    else if(pOutput->u8_dh_slow_motion_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_slow_motion=1;
        pOutput->u8_dh_slow_motion_hold_cnt = pOutput->u8_dh_slow_motion_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_slow_motion=0;
        pOutput->u8_dh_slow_motion_hold_cnt = 0;
    }

     //------------------------------ small hor slow cond ----------------------------------------//
     if((gmv_bgcnt > 26000) && (_ABS_DIFF_(gmv_fgx,gmv_bgx)<=35)  && (_ABS_DIFF_(gmv_fgy,gmv_bgy)<=15) && ((_ABS_(gmv_fgx) + _ABS_(gmv_fgy)) < 15))
     {
        pOutput->u8_dh_small_hor_slow_in_cnt  = pOutput->u8_dh_small_hor_slow_in_cnt + 1;
        pOutput->u8_dh_small_hor_slow_in_cnt = (pOutput->u8_dh_small_hor_slow_in_cnt > 3) ? 3 : pOutput->u8_dh_small_hor_slow_in_cnt;
     }
     else
    {
        pOutput->u8_dh_small_hor_slow_in_cnt = 0;
    }

     if(pOutput->u8_dh_small_hor_slow_in_cnt >= 3)
    {
        pOutput->u8_dh_condition_small_hor_slow=1;
        pOutput->u8_dh_small_hor_slow_hold_cnt = 10;
    }
    else if(pOutput->u8_dh_small_hor_slow_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_small_hor_slow=1;
        pOutput->u8_dh_small_hor_slow_hold_cnt = pOutput->u8_dh_small_hor_slow_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_small_hor_slow=0;
        pOutput->u8_dh_small_hor_slow_hold_cnt = 0;
    }

    //------------------------------ black_protect_off ----------------------------------------//
    if((((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/fg_apl_sum)<250) && (fg_apl_sum>0))
    {
        pOutput->u8_dh_black_protect_off_in_cnt  = pOutput->u8_dh_black_protect_off_in_cnt + 1;
        pOutput->u8_dh_black_protect_off_in_cnt = (pOutput->u8_dh_black_protect_off_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_black_protect_off_in_cnt;
    }
    /*else if(((bgfg_apli_mean < 26000) && (bgfg_aplp_mean < 26000) && (_ABS_(gmv_bgx) > 70)) && (fg_apl_sum>0))
    {
        pOutput->u8_dh_black_protect_off_in_cnt  = pOutput->u8_dh_black_protect_off_in_cnt + 1;
        pOutput->u8_dh_black_protect_off_in_cnt = (pOutput->u8_dh_black_protect_off_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_black_protect_off_in_cnt;
    }*/
    else
    {
        pOutput->u8_dh_black_protect_off_in_cnt = 0;
    }
    
    if(pOutput->u8_dh_black_protect_off_in_cnt >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_black_protect_off=1;
        pOutput->u8_dh_black_protect_off_hold_cnt = 30;
        //rtd_pr_memc_info("rgn_black_off 111111\n");
        //black=0;
    }
    else if(pOutput->u8_dh_black_protect_off_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_black_protect_off=1;
        pOutput->u8_dh_black_protect_off_hold_cnt = pOutput->u8_dh_black_protect_off_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_black_protect_off=0;
        pOutput->u8_dh_black_protect_off_hold_cnt = 0;
        //black=1;
    }
    //rtd_pr_memc_info("rgn_black_off: en(%d) apl_mean(%d %d) bg(%d %d) fg(%d %d)\n", pOutput->u8_dh_condition_black_protect_off, bgfg_apli_mean, bgfg_aplp_mean, gmv_bgx, gmv_bgy, gmv_fgx, gmv_fgy);

    //------------------------------ white_protect_on ----------------------------------------//
    white_pro1 = ((((fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum))>400)&&(((bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/bg_apl_sum)<=250));
    white_pro2 = ((gmv_bgcnt<17000)&&((((fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum))>180)&&(((bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/bg_apl_sum)<=250)));
    white_pro3 = ((gmv_bgcnt>23000) &&(gmv_bgcnt<28000) && 
                        (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*1000/(bg_apl_sum))<10) && (((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum))>550)  && 
                        (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum))>300) && (((fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum))>160) && 
                        (gmv_bgx < -40) && (gmv_bgx > -65) && (_ABS_(gmv_bgy) < 3) && ( _ABS_(gmv_fgx) < 10) && ( _ABS_(gmv_fgy) < 10));
    if((white_pro1 ||white_pro2 || white_pro3) &&(fg_apl_sum>0)&&(bg_apl_sum>0))
    {
        pOutput->u8_dh_white_protect_on_in_cnt  = pOutput->u8_dh_white_protect_on_in_cnt + 1;
        pOutput->u8_dh_white_protect_on_in_cnt = (pOutput->u8_dh_white_protect_on_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_white_protect_on_in_cnt;
    }
    else
    {
        pOutput->u8_dh_white_protect_on_in_cnt = 0;
    }
    if(pOutput->u8_dh_white_protect_on_in_cnt >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_white_protect_on=1;
        pOutput->u8_dh_white_protect_on_hold_cnt = 80;
        //white=1;	
    }
    else if(pOutput->u8_dh_white_protect_on_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_white_protect_on=1;
        pOutput->u8_dh_white_protect_on_hold_cnt = pOutput->u8_dh_white_protect_on_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_white_protect_on=0;
        pOutput->u8_dh_white_protect_on_hold_cnt = 0;
        //white=0;
    }
    //rtd_pr_memc_info("fgapl0+fgapl1=%dfg567/7=%d bg567/7=%d black=%d white=%d\n ",(fg_apl_seg0_cnt+fg_apl_seg1_cnt),(fg_apl_seg5_cnt+fg_apl_seg6_cnt+fg_apl_seg7_cnt)*1000/(fg_apl_sum),((bg_apl_seg5_cnt+bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/bg_apl_sum),black,white);

    //------------------------------ correct_cut_th ----------------------------------------//
    if(((_MAX_(_ABS_(gmv_bgx), _ABS_(gmv_bgy))>40)&&(_ABS_(gmv_fgx)<40)&&(_ABS_(gmv_fgy)<50))||((_ABS_DIFF_(gmv_fgy,gmv_bgy)>_ABS_DIFF_(gmv_fgx,gmv_bgx))||((_ABS_DIFF_(_ABS_(gmv_fgy),_ABS_(gmv_fgx))<15)&&(_ABS_DIFF_(_ABS_(gmv_bgy),_ABS_(gmv_bgx))<10))))
    {
        pOutput->u8_dh_correct_cut_th_in_cnt  = pOutput->u8_dh_correct_cut_th_in_cnt + 1;
        pOutput->u8_dh_correct_cut_th_in_cnt = (pOutput->u8_dh_correct_cut_th_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_correct_cut_th_in_cnt;
    }
    else
    {
        pOutput->u8_dh_correct_cut_th_in_cnt = 0;
    }
    if(pOutput->u8_dh_correct_cut_th_in_cnt >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_correct_cut_th=1;
        pOutput->u8_dh_correct_cut_th_hold_cnt = 30;
    }
    else if(pOutput->u8_dh_correct_cut_th_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_correct_cut_th=1;
        pOutput->u8_dh_correct_cut_th_hold_cnt = pOutput->u8_dh_correct_cut_th_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_correct_cut_th=0;
    }

    //--------------- black fg, white bg, fg mv big, no refer predflag, for #113 ghost-------------------------//
    if((_ABS_(gmv_fgx) > 50) && (_ABS_(gmv_bgx) < 40) && (((fg_apl_seg0_cnt+fg_apl_seg1_cnt+fg_apl_seg2_cnt)*1000/(fg_apl_sum))>450) && (((bg_apl_seg6_cnt+bg_apl_seg7_cnt)*1000/(bg_apl_sum))>850) &&(fg_apl_sum>0)&&(bg_apl_sum>0))
    {
        pOutput->u8_dh_predflag_dis_in_cnt  = pOutput->u8_dh_predflag_dis_in_cnt + 1;
        pOutput->u8_dh_predflag_dis_in_cnt = (pOutput->u8_dh_predflag_dis_in_cnt > 2) ? 2 : pOutput->u8_dh_predflag_dis_in_cnt;
    }
    else
    {
        pOutput->u8_dh_predflag_dis_in_cnt = 0;
    }
    if(pOutput->u8_dh_predflag_dis_in_cnt >= 2)
    {
        pOutput->u8_dh_condition_predflag_dis=1;
        pOutput->u8_dh_predflag_dis_hold_cnt = 20;
    }
    else if(pOutput->u8_dh_predflag_dis_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_predflag_dis=1;
        pOutput->u8_dh_predflag_dis_hold_cnt = pOutput->u8_dh_predflag_dis_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_predflag_dis=0;
        pOutput->u8_dh_predflag_dis_hold_cnt = 0;
    }
    
    //------------------------------ aplfix_en ----------------------------------------//
   /* if((fb_cnt_1>=2)||(fb_cnt_2>=3))
    {
        pOutput->u8_dh_aplfix_en_in_cnt  = pOutput->u8_dh_aplfix_en_in_cnt + 1;
        pOutput->u8_dh_aplfix_en_in_cnt = (pOutput->u8_dh_aplfix_en_in_cnt > hold_in_cnt) ? hold_in_cnt : pOutput->u8_dh_aplfix_en_in_cnt;
    }
    else
    {
        pOutput->u8_dh_aplfix_en_in_cnt = 0;
    }
    if(pOutput->u8_dh_aplfix_en_in_cnt >= hold_in_cnt)
    {
        pOutput->u8_dh_condition_aplfix_en=1; 
        pOutput->u8_dh_aplfix_en_hold_cnt = 30;
    }
    else if(pOutput->u8_dh_aplfix_en_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_aplfix_en=1; 
        pOutput->u8_dh_aplfix_en_hold_cnt = pOutput->u8_dh_aplfix_en_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_aplfix_en=0; 
        pOutput->u8_dh_aplfix_en_hold_cnt = 0;
    }	*/	
    //rtd_pr_memc_info("fb_cnt_1=%d fb_cnt_2=%d\n",fb_cnt_1,fb_cnt_2);

    ReadRegister(HARDWARE_HARDWARE_26_reg,31,31, &u8_dh_condition_dhbypass_en);
    ReadRegister(HARDWARE_HARDWARE_25_reg, 10,10 , &fwcontrol_185); 
    ReadRegister(HARDWARE_HARDWARE_25_reg, 11,11 , &fwcontrol_13);
    ReadRegister(HARDWARE_HARDWARE_25_reg, 6,6 , &fwcontrol_214); 
    ReadRegister(HARDWARE_HARDWARE_25_reg, 8,8 , &fwcontrol_299); 
    ReadRegister(HARDWARE_HARDWARE_25_reg, 9,9 , &fwcontrol_189); 
    ReadRegister(HARDWARE_HARDWARE_25_reg, 13,13 , &fwcontrol_156); 
    ReadRegister(HARDWARE_HARDWARE_25_reg, 17,17 , &fwcontrol_306); 
    ReadRegister(HARDWARE_HARDWARE_25_reg, 23,23 , &fwcontrol_183); 
    ReadRegister(HARDWARE_HARDWARE_25_reg, 24,24 , &fwcontrol_49);
    ReadRegister(HARDWARE_HARDWARE_25_reg, 25,25 , &fwcontrol_163);
    ReadRegister(HARDWARE_HARDWARE_25_reg, 26,26 , &fwcontrol_432);

    //------------------------ for 133 flash, when black bg and very big bgcnt, bypass dh-----------------------//
    updateGMVArray(glb_bgcnt,glb_bgcnt_length,gmv_bgcnt);
    flag_133  = (glb_bgcnt[0]>31500)&&(glb_bgcnt[1]>31500)&&(glb_bgcnt[2]>31500)&& (sum_apl<1000000);
    if(flag_133)  
    {
        glb_bgcnt_large=1;
    }
    else
    {
        glb_bgcnt_large=0;
    }

    //------------------------ for 136 flash, when black bg and very big bgcnt, big fg bypass dh-----------------------//
    flag_136_1 = (gmv_bgcnt > 31000) && (_ABS_(gmv_bgx) < 6) && (_ABS_(gmv_bgy) < 6) && (gmv_fgx < -50) && (gmv_fgy < -20) && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*1000/(bg_apl_sum)) > 600) && (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 600) && (sum_apl<1600000);
    flag_136_2 = (gmv_bgcnt > 29000) && (_ABS_(gmv_bgx) < 10) && (_ABS_(gmv_bgy) < 3) && (gmv_fgx < -50) && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*1000/(bg_apl_sum)) > 700) && (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) < 450) && (sum_apl<1600000);
    flag_136_3 = (gmv_bgcnt > 32000) && (_ABS_(gmv_bgx) < 10) && (_ABS_(gmv_bgy) < 10) && (gmv_fgx < -15) && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*1000/(bg_apl_sum)) > 600) && (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 400) && (sum_apl<2000000);
    flag_136_4 = (gmv_bgcnt > 28000) && (_ABS_(gmv_fgx) > 50) && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt+bg_apl_seg2_cnt)*1000/(bg_apl_sum)) > 900) && (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*1000/(fg_apl_sum)) > 400) && (sum_apl<1000000);;

    if(flag_136_1 || flag_136_2 || flag_136_3 || flag_136_4)  
    {
        dark_flash_big_fg=1;
    }
    else
    {
        dark_flash_big_fg=0;
    }
     //------------------- for 115 light, center is bg, bg is white, around is fg, fg is dark  off dehalo----------------//
    if(((rgn_bgcnt[11]+rgn_bgcnt[12]+rgn_bgcnt[19]+rgn_bgcnt[20])/4 > 900) && 
        ((rgn_bgcnt[9]+rgn_bgcnt[10]+rgn_bgcnt[13]+rgn_bgcnt[14])/4 < 800)  && ((rgn_bgcnt[9]+rgn_bgcnt[10]+rgn_bgcnt[13]+rgn_bgcnt[14])/4 > 400) &&
        ((rgn_apli[11]+rgn_apli[12]+rgn_apli[19]+rgn_apli[20])/4 > 130000) &&
        ((rgn_apli[9]+rgn_apli[10]+rgn_apli[13]+rgn_apli[14])/4 < 80000) &&
        ((_ABS_(gmv_bgx) + _ABS_(gmv_bgy)) < 8) && 
        (((_ABS_(gmv_fgx)< 60) && (_ABS_(gmv_fgx) > 8) && (gmv_fgx < 0)) || ((_ABS_(gmv_fgx)< 10) && (_ABS_(gmv_fgx) > 4) && (gmv_fgx > 0))))
        {
            bad_light_off = 1;
        }
        else
        {
            bad_light_off = 0;
        }

    //---------------------------- for 231 flash, when very small bgcnt, bypass dh-----------------------------//
    if((gmv_bgcnt<9000))    //231 in_cnt 
    {
        pOutput->u8_dh_tinybgcnt_in_cnt=pOutput->u8_dh_tinybgcnt_in_cnt+1;
        pOutput->u8_dh_tinybgcnt_in_cnt=(pOutput->u8_dh_tinybgcnt_in_cnt > tinybgcnt_hold_in_cnt) ? tinybgcnt_hold_in_cnt : pOutput->u8_dh_tinybgcnt_in_cnt;

    }
    else
    {
        pOutput->u8_dh_tinybgcnt_in_cnt = 0;
    }  
    if(pOutput->u8_dh_tinybgcnt_in_cnt >= tinybgcnt_hold_in_cnt)
    {
        tiny_bgcnt_flag=1;
    }
    else
    {
        tiny_bgcnt_flag=0;
    }
	
     //-------------------------------------------- for 012---------------------------------------------------//
    if(((gmv_bgcnt<13000) && (gmv_bgy > gmv_bgx) && (_ABS_DIFF_(gmv_bgx, gmv_fgx) < 5) && (((gmv_bgx > 15) && (gmv_bgy > 20)) || ((gmv_bgx > 5) && (gmv_bgy > 28)))) || 
        ((gmv_bgy > gmv_bgx) && (gmv_bgx > 28) && (gmv_bgy > 28) && (gmv_fgx > 25))|| 
        ((gmv_bgcnt < 12000) && (gmv_bgx > 32) && (gmv_bgy > 27) && (gmv_fgx > 25) && (gmv_fgy > 2)))   
    {
        pOutput->u8_dh_smallcnt_y_big_in_cnt=pOutput->u8_dh_smallcnt_y_big_in_cnt+1;
        pOutput->u8_dh_smallcnt_y_big_in_cnt=(pOutput->u8_dh_smallcnt_y_big_in_cnt > 2) ? 2 : pOutput->u8_dh_smallcnt_y_big_in_cnt;
    }
    else
    {
        pOutput->u8_dh_smallcnt_y_big_in_cnt = 0;
    }  
    if(pOutput->u8_dh_smallcnt_y_big_in_cnt >= 2)
    {
        smallcnt_y_big_flag=1;
    }
    else
    {
        smallcnt_y_big_flag=0;
    }

    //---------------------------- for 130 and bgmask unstable pattern-----------------------------//
    if(avr_diff > 280)
    {
        avg_bg_diff_cond = 1;
        pOutput->u8_dh_avg_diff_hold_cnt = (fwcontrol_183 || fwcontrol_49) ? 2 : 22;
    }
    else if(pOutput->u8_dh_avg_diff_hold_cnt > 0)
    {
        avg_bg_diff_cond = 1;
        pOutput->u8_dh_avg_diff_hold_cnt = ((diff[4] < 10)/*center bgdiff*/ && (gmv_bgcnt > 28000)) ? 0 : (pOutput->u8_dh_avg_diff_hold_cnt - 1);;
    }
    else
    {
        avg_bg_diff_cond = 0;
        pOutput->u8_dh_avg_diff_hold_cnt = 0;
    }
    
    SCFlag = s_pContext->_output_read_comreg.u1_sc_status_rb;
    ReadRegister(KME_DEHALO5_PHMV_FIX_35_reg,23,23, &dh_bypass);
    fwcontrol_227 = pOutput->u8_dh_gmv_valchange_227_1 || pOutput->u8_dh_gmv_valchange_227_2 || pOutput->u8_dh_gmv_valchange_227_3 || pOutput->u8_dh_gmv_valchange_227_4 || pOutput->u8_dh_gmv_valchange_227_5;
	ReadRegister(HARDWARE_HARDWARE_43_reg,30,30, &dh_bypass_forrotterdam_33s);
    //rtd_pr_memc_info("pass(%d)  (%d) scene(%d) fg(%d %d) bg (%d %d)\n", dh_bypass, smallcnt_y_big_flag, SCFlag, gmv_fgx,gmv_fgy,gmv_bgx,gmv_bgy);
    //----------------------------------- new bypass dehalo------------------------------------//
    if(u8_dh_condition_dhbypass_en)
    {
        if(((avg_bg_diff_cond == 1)||(glb_bgcnt_large==1) || (dark_flash_big_fg == 1) || (bad_light_off == 1) || (tiny_bgcnt_flag == 1) || (smallcnt_y_big_flag == 1) || 
            (pOutput->u8_dh_condition_mixed_mv == 1)|| (pOutput->u8_dh_condition_123_broken == 1) || (pOutput->u8_dh_condition_vertical_broken == 1) || 
            (pOutput->u8_dh_condition_slant_mv == 1) || pOutput->u8_me1_condition_virtical_case3==1 || (pOutput->u8_dh_condition_flashing == 1) ||
            (pOutput->u8_dh_condition_ver_sea == 1)||(pOutput->u8_dh_condition_ver_hor_fast == 1) || pOutput->u8_dh_condition_bad_fg || (pOutput->u8_dh_condition_024_bypass == 1)||(pOutput->u8_dh_condition_030_bypass == 1 )||(pOutput->u8_dh_condition_051_bypass == 1 )
            ||(pOutput->u8_dh_condition_346_bypass == 1)||(pOutput->u8_dh_condition_184_bypass == 1)||(pOutput->u8_dh_condition_110_bypass == 1)||(pOutput->u8_dh_condition_342_bypass == 1)
            ||(pOutput->u8_dh_condition_26_bypass == 1)||(pOutput->u8_dh_condition_1841_bypass == 1)||(pOutput->u8_dh_condition_207_bypass == 1)||(pOutput->u8_dh_condition_333_bypass == 1)||(pOutput->u8_me1_condition_195_face_broken==1)||(pOutput->u8_dh_condition_car_bypass==1)
            ||(pOutput->u8_dh_condition_412_bypass == 1)||(pOutput->u8_dh_condition_35_bypass == 1)||(pOutput->u8_dh_condition_215_bypass==1)||(pOutput->u8_dh_condition_2271_bypass==1)||(pOutput->u8_dh_condition_410_scgarbage == 1)||(pOutput->u8_dh_condition_smallbgcnt_ver_bypass == 1)
            ||(pOutput->u8_dh_condition_393_scgarbage == 1)||(pOutput->u8_dh_condition_182_bypass == 1)||(pOutput->u8_dh_condition_343_bypass == 1)||(pOutput->u8_dh_condition_416_bypass == 1)||(	pOutput->u8_dh_condition_lager_dtlcomparison_bypass == 1)||(pOutput->u8_dh_condition_smallbgcnt_slant_bypass == 1)) && 
            (fwcontrol_13 == 0) && (fwcontrol_214 == 0) && (fwcontrol_185 == 0) && (fwcontrol_299 == 0) && (fwcontrol_189 == 0) && (pOutput->u8_dh_condition_229 == 0) && (fwcontrol_156 == 0) && (fwcontrol_306 == 0) && (fwcontrol_227 == 0)&&(pOutput->u8_dh_condition_214==0)&&(pOutput->u8_dh_condition_insect_status!= 1)&&(pOutput->u8_dh_condition_transporter_status != 1)
            &&(pOutput->u8_dh_condition_eagle_status!=1)&&(pOutput->u8_dh_condition_huapi2_status!=1)&&(pOutput->u8_dh_condition_dance_status!=1)&&(pOutput->u8_dh_condition_rotterdam_status!=1)&&(pOutput->u8_dh_condition_dance_2_status!=1)&&(pOutput->u8_dh_condition_skating_status!=1)&&(pOutput->u8_dh_conditio_0026_1_status!=1)&&(dh_bypass_forrotterdam_33s==0)&&(pOutput->u8_dh_condition_IDT_flag == 0))
        {
            pOutput->u8_dh_condition_dhbypass=1;
            if(bad_light_off == 1)
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 30) ? 1 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 30) ? 30 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if(fwcontrol_183 || fwcontrol_163 || fwcontrol_49)
            {
                pOutput->u8_dh_condition_dhbypass_hold_cnt = 0;
                pOutput->u8_dh_condition_bypass_check = 0;
                pOutput->u8_dh_condition_dhbypass=0;
            }
            else if((glb_bgcnt_large == 1)||(pOutput->u8_dh_condition_051_bypass == 1))
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 20) ? 2 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 20) ? 20 : pOutput->u8_dh_condition_dhbypass_hold_cnt;

            }
            else if(dark_flash_big_fg == 1)
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 3 : pOutput->u8_dh_condition_bypass_check;
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 100) ? 100 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 60 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                }

            }
            else if((tiny_bgcnt_flag == 1)||(pOutput->u8_dh_condition_030_bypass == 1 ))  //030 hold10
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 10) ? 4 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 10) ? 10 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if(smallcnt_y_big_flag == 1)
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 5 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 60 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if((pOutput->u8_dh_condition_mixed_mv == 1)||(pOutput->u8_dh_condition_ver_hor_fast == 1))
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 30) ? 6 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 30) ? 30 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if((pOutput->u8_dh_condition_123_broken == 1)||(pOutput->u8_dh_condition_1841_bypass == 1))
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 80) ? 7 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 80) ? 80 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if((pOutput->u8_dh_condition_vertical_broken == 1)||(pOutput->u8_dh_condition_346_bypass == 1)||(pOutput->u8_dh_condition_184_bypass == 1)||(pOutput->u8_dh_condition_110_bypass == 1))
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 8 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 60 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if(pOutput->u8_dh_condition_slant_mv == 1)
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 9 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 60 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if(pOutput->u8_dh_condition_flashing == 1)
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 35) ? 10 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 35) ? 35 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if(pOutput->u8_dh_condition_ver_sea == 1)
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 100) ? 11 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 100) ? 100 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if( (pOutput->u8_dh_condition_342_bypass == 1)||(pOutput->u8_dh_condition_26_bypass == 1))
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 180) ? 12 : pOutput->u8_dh_condition_bypass_check;
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 180) ? 180 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 90) ? 90 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                }
            }
            else if((pOutput->u8_me1_condition_virtical_case3 == 1)||(pOutput->u8_dh_condition_333_bypass == 1)||pOutput->u8_dh_condition_343_bypass)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 240) ? 240 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 120) ? 120 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                }
            }
            else if( avg_bg_diff_cond == 1)
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 2) ? 13 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 2) ? 2 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if(pOutput->u8_dh_condition_bad_fg == 1)
            {
                pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 14 : pOutput->u8_dh_condition_bypass_check;
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 60 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 30) ? 30 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                }
            }
             else if(pOutput->u8_dh_condition_024_bypass == 1)
            {
                 pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 40) ? 15 : pOutput->u8_dh_condition_bypass_check;
                 pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 40) ? 40 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if(pOutput->u8_dh_condition_207_bypass == 1)
            {
                 pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 16 : pOutput->u8_dh_condition_bypass_check;
                 pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 60 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
            }
            else if(pOutput->u8_me1_condition_195_face_broken==1)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 220) ? 220 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 220) ? 17 : pOutput->u8_dh_condition_bypass_check;
                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 110) ? 110 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 110) ? 17 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if(pOutput->u8_dh_condition_35_bypass==1)
            {
            
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 200) ? 200 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 200) ? 18 : pOutput->u8_dh_condition_bypass_check;

                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 70) ? 70 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 70) ? 18 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if(pOutput->u8_dh_condition_412_bypass==1)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 120) ? 120 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 120) ? 19 : pOutput->u8_dh_condition_bypass_check;

                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 60 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 60) ? 19 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if(pOutput->u8_dh_condition_215_bypass==1)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 80) ? 80 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 80) ? 20 : pOutput->u8_dh_condition_bypass_check;

                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 40) ? 40 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 40) ? 20 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if(pOutput->u8_dh_condition_2271_bypass==1)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 40) ? 40 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 40) ? 21 : pOutput->u8_dh_condition_bypass_check;

                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 40) ? 40 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 40) ? 21 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if((pOutput->u8_dh_condition_410_scgarbage == 1)||(pOutput->u8_dh_condition_393_scgarbage == 1))
            {
            
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 20) ? 20 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 20) ? 22 : pOutput->u8_dh_condition_bypass_check;
                
                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 10) ? 10 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 10) ? 22 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if(pOutput->u8_dh_condition_182_bypass == 1)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 200) ? 200 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 200) ? 23 : pOutput->u8_dh_condition_bypass_check;

                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 100) ? 100 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 100) ? 23 : pOutput->u8_dh_condition_bypass_check;
                }

            }
            else if(pOutput->u8_dh_condition_416_bypass == 1)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 160) ? 160 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 160) ? 24 : pOutput->u8_dh_condition_bypass_check;

                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 80) ? 80 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 80) ? 24 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if(pOutput->u8_dh_condition_lager_dtlcomparison_bypass == 1)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 20) ? 20 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 20) ? 25 : pOutput->u8_dh_condition_bypass_check;

                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 10) ? 10 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 10) ? 25 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if(pOutput->u8_dh_condition_smallbgcnt_slant_bypass== 1)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 180) ?180 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 180) ? 26 : pOutput->u8_dh_condition_bypass_check;

                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 90) ? 90 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 90) ? 26 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if(pOutput->u8_dh_condition_smallbgcnt_ver_bypass== 1)
            {
                if( s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ)
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 100) ?100 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 100) ? 27 : pOutput->u8_dh_condition_bypass_check;

                }
                else
                {
                    pOutput->u8_dh_condition_dhbypass_hold_cnt=(pOutput->u8_dh_condition_dhbypass_hold_cnt < 1) ? 1 : pOutput->u8_dh_condition_dhbypass_hold_cnt;
                    pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 1) ? 27 : pOutput->u8_dh_condition_bypass_check;
                }
            }
            else if(pOutput->u8_dh_condition_car_bypass == 1)
            {
            	  pOutput->u8_dh_condition_bypass_check = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 90) ? 18 : pOutput->u8_dh_condition_bypass_check;
                pOutput->u8_dh_condition_dhbypass_hold_cnt = (pOutput->u8_dh_condition_dhbypass_hold_cnt < 90) ? 90 : pOutput->u8_dh_condition_dhbypass_hold_cnt;

            }			
            else
            {
                //pOutput->u8_dh_condition_dhbypass_hold_cnt = 0;
            }
        }
        else if(pOutput->u8_dh_condition_dhbypass_hold_cnt > 0)
        {
            pOutput->u8_dh_condition_dhbypass=1;
            pOutput->u8_dh_condition_dhbypass_hold_cnt = pOutput->u8_dh_condition_dhbypass_hold_cnt - 1;
        }
        else
        {
            pOutput->u8_dh_condition_dhbypass=0;
            pOutput->u8_dh_condition_dhbypass_hold_cnt = 0;
            pOutput->u8_dh_condition_bypass_check = 0;
        }
    }
    else
    {
        pOutput->u8_dh_condition_dhbypass=0;
    }
		
    ReadRegister(KME_ME1_BG1_ME_BG_DUMMY_0_reg,16 , 16 , &readval16);
	if(readval16)
    {
	rtd_pr_memc_info("dypass:%d else8:%d %d %d %d else:%d %d 4:%d %d 6:%d %d 7:%d %d 12:%d %d\n",pOutput->u8_dh_condition_bypass_check,pOutput->u8_dh_condition_vertical_broken,pOutput->u8_dh_condition_346_bypass,pOutput->u8_dh_condition_184_bypass,pOutput->u8_dh_condition_110_bypass,pOutput->u8_me1_condition_virtical_case3,pOutput->u8_dh_condition_333_bypass,tiny_bgcnt_flag,pOutput->u8_dh_condition_030_bypass,
		pOutput->u8_dh_condition_mixed_mv ,pOutput->u8_dh_condition_ver_hor_fast,pOutput->u8_dh_condition_123_broken,pOutput->u8_dh_condition_1841_bypass,pOutput->u8_dh_condition_342_bypass,pOutput->u8_dh_condition_26_bypass);
	rtd_pr_memc_info("12:%d %d\n",pOutput->u8_dh_condition_342_bypass,pOutput->u8_dh_condition_26_bypass);
    }  

	//rtd_pr_memc_info("bypass:%d %d %d %d %d %d %d %d %d %d\n", fwcontrol_13,fwcontrol_214 ,fwcontrol_185 ,fwcontrol_299,fwcontrol_189,pOutput->u8_dh_condition_229,fwcontrol_156,fwcontrol_306 ,fwcontrol_227,pOutput->u8_dh_condition_214);

    //pOutput->u8_dh_condition_bypass_check = white_pro3 ? 1: 0;
        //------------------------------------fadeinout------------------------//

    if(u8_dh_condition_fadeinout_dhbypass_en)
    {
        if(((apl_mt_array[0]>=apl_mt_rgncnt_th)&&(apl_mt_array[1]>=apl_mt_rgncnt_th)&&(apl_mt_array[2]>=apl_mt_rgncnt_th)&&(apl_mt_array[3]>=apl_mt_rgncnt_th)&&(apl_mt_array[4]>=apl_mt_rgncnt_th))||((apl_lt_array[0]>=apl_mt_rgncnt_th)&&(apl_lt_array[1]>=apl_mt_rgncnt_th)&&(apl_lt_array[2]>=apl_mt_rgncnt_th)&&(apl_lt_array[3]>=apl_mt_rgncnt_th)&&(apl_lt_array[4]>=apl_mt_rgncnt_th)))
        {       
            pOutput->u8_dh_condition_fadeinout_dhbypass = 1;
            pOutput->u8_dh_condition_fadeinout_dhbypass_hold_cnt = 10;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0xF);
        }
        else if(pOutput->u8_dh_condition_fadeinout_dhbypass_hold_cnt>0)
        {
            pOutput->u8_dh_condition_fadeinout_dhbypass = 1;
            pOutput->u8_dh_condition_fadeinout_dhbypass_hold_cnt = pOutput->u8_dh_condition_fadeinout_dhbypass_hold_cnt - 1;
            //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0xF);
        }         
        else
        {
            pOutput->u8_dh_condition_fadeinout_dhbypass = 0;
            pOutput->u8_dh_condition_fadeinout_dhbypass_hold_cnt = 0;
           // WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
        }
    }
    else
    {
        pOutput->u8_dh_condition_fadeinout_dhbypass = 0;
        //WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 24, 27, 0x0);
    }

    updateGMVArray(rgn_logo_cnt_cont,logo_cnt_length,sum_logo_cnt);    
    if((rgn_logo_cnt_cont[0]>3000)&&(rgn_logo_cnt_cont[1]>3000)&&(rgn_logo_cnt_cont[2]>3000))
    {
        pOutput->u8_dh_condition_logo_off=1;
        pOutput->u8_dh_condition_logo_off_cnt=20;
    }
    else if(pOutput->u8_dh_condition_logo_off_cnt>0)
    {
        pOutput->u8_dh_condition_logo_off=1;
        pOutput->u8_dh_condition_logo_off_cnt=pOutput->u8_dh_condition_logo_off_cnt-1;
    }
    else
    {
        pOutput->u8_dh_condition_logo_off=0;
    }
    
    
    ////////////////---------rim 050----------------------/////////

    ReadRegister(HARDWARE_HARDWARE_25_reg , 0, 31, &u32_RB_val);
    val =(u32_RB_val >> 24) & 0x3f;  
    avr_mv_x_left=_ABS_(rgnmvx_bg[0]+rgnmvx_bg[8]+rgnmvx_bg[16]+rgnmvx_bg[24])/4;
    left_rim_pre=s_pContext->_output_rimctrl.u9_dh_blk_rim[_RIM_LFT];
    if((avr_mv_x_left>45))   //050 rim
    {
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8, left_rim_pre+16);//  val=16 ok
        rim_hold_cnt_left=6;
		video_rimleft = 1;
    }
    else if((avr_mv_x_left>80) && (fwcontrol_432 == 1))   //432 rim
    {
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8, left_rim_pre+20);
        rim_hold_cnt_left_for432 =6;
    }
    else if(rim_hold_cnt_left>0)
    {            
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8,  left_rim_pre+16);
        rim_hold_cnt_left--;       
    }
    else if(rim_hold_cnt_left_for432>0)
    {            
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8,  left_rim_pre+20);
        rim_hold_cnt_left_for432--;       
    }
	else if(video_rimleft>0)
    {
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8,  left_rim_pre);
		video_rimleft = 0;
    }
    else
    {
        WriteRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8,  left_rim_pre);
    }
    //rtd_pr_memc_info("mv:%d val:%d\n",avr_mv_x_left,val);


////////////////////-----------------301------------------------///////////// 
    if((_ABS_(gmv_bgx)<=2) && (_ABS_(gmv_bgy)==0) && (_ABS_(gmv_fgx)>=30) && (_ABS_(gmv_fgx)<=50) && (_ABS_(gmv_fgy)<=10) && ((fg_apl_seg0_cnt*100/fg_apl_sum)>10) && (fg_apl_sum>0) && (gmv_bgcnt > 31300) && (top_sum_bgcnt > 16300) && (sum_apl > 2800000))
    {
        pOutput->u8_dh_condition_301_in_cnt  = pOutput->u8_dh_condition_301_in_cnt + 1;
        pOutput->u8_dh_condition_301_in_cnt = (pOutput->u8_dh_condition_301_in_cnt > 3) ? 3 : pOutput->u8_dh_condition_301_in_cnt;
    }
    else
    {
        pOutput->u8_dh_condition_301_in_cnt = 0;
    }
    if(pOutput->u8_dh_condition_301_in_cnt >= 3)
    {
        pOutput->u8_dh_condition_301=1;
        pOutput->u8_dh_condition_301_hold_cnt = 50;
	 WriteRegister(HARDWARE_HARDWARE_25_reg, 19,19 , 1); 
    }
    else if(pOutput->u8_dh_condition_301_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_301=1;
        pOutput->u8_dh_condition_301_hold_cnt = pOutput->u8_dh_condition_301_hold_cnt - 1;
	 WriteRegister(HARDWARE_HARDWARE_25_reg, 19,19 , 1);
    }
    else
    {
        pOutput->u8_dh_condition_301=0;
        pOutput->u8_dh_condition_301_hold_cnt = 0;
        WriteRegister(HARDWARE_HARDWARE_25_reg, 19,19 , 0);
    }

    //-------------------------------------229-----------------------------------------//
    apl_229_cond0 = (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*100/bg_apl_sum)>85) && (bg_apl_sum>0);
    apl_229_cond1 = (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum)>75) && (fg_apl_sum>0)  && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*100/bg_apl_sum)>60) && (bg_apl_sum>0);
    apl_229_cond2 = (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum)>40) && (fg_apl_sum>0)  && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*100/bg_apl_sum)>60) && (bg_apl_sum>0);
    apl_229_cond3 = (((fg_apl_seg0_cnt+fg_apl_seg1_cnt)*100/fg_apl_sum)>90) && (fg_apl_sum>0)  && (((bg_apl_seg0_cnt+bg_apl_seg1_cnt)*100/bg_apl_sum)>40) && (bg_apl_sum>0);
    
    if((gmv_bgx>60) && (_ABS_(gmv_bgy)< 8) && /*(_ABS_(gmv_fgx)> 2) &&*/ (_ABS_(gmv_fgy)< 8) && (apl_229_cond0 || apl_229_cond1) && (gmv_bgcnt > 18000) && (sum_apl > 400000) && (sum_apl < 1000000))
    {
        pOutput->u8_dh_condition_229_in_cnt  = pOutput->u8_dh_condition_229_in_cnt + 1;
        pOutput->u8_dh_condition_229_in_cnt = (pOutput->u8_dh_condition_229_in_cnt > 2) ? 2 : pOutput->u8_dh_condition_229_in_cnt;
    }
    else if((gmv_bgx>15) && (_ABS_(gmv_bgy)< 8) && (gmv_fgx> 15) && (_ABS_(gmv_fgy)< 8) && (apl_229_cond0 || apl_229_cond2) && (gmv_bgcnt > 15000) && (sum_apl > 900000) && (sum_apl < 1600000))
    {
        pOutput->u8_dh_condition_229_in_cnt  = pOutput->u8_dh_condition_229_in_cnt + 1;
        pOutput->u8_dh_condition_229_in_cnt = (pOutput->u8_dh_condition_229_in_cnt > 2) ? 2 : pOutput->u8_dh_condition_229_in_cnt;
    }
    else if((gmv_bgx>10) && (_ABS_(gmv_bgy)< 8) && (_ABS_(gmv_fgx)< 40) && (_ABS_(gmv_fgy)< 8) && (apl_229_cond0 || apl_229_cond3) && (gmv_bgcnt > 16000) && (sum_apl > 900000) && (sum_apl < 1600000))
    {
        pOutput->u8_dh_condition_229_in_cnt  = pOutput->u8_dh_condition_229_in_cnt + 1;
        pOutput->u8_dh_condition_229_in_cnt = (pOutput->u8_dh_condition_229_in_cnt > 2) ? 2 : pOutput->u8_dh_condition_229_in_cnt;
    }
    else
    {
        pOutput->u8_dh_condition_229_in_cnt = 0;
    }
    if(pOutput->u8_dh_condition_229_in_cnt >= 2)
    {
        pOutput->u8_dh_condition_229=1;
        pOutput->u8_dh_condition_229_hold_cnt = 35;
    }
    else if(pOutput->u8_dh_condition_229_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_229=1;
        pOutput->u8_dh_condition_229_hold_cnt = pOutput->u8_dh_condition_229_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_229=0;
        pOutput->u8_dh_condition_229_hold_cnt = 0;
    }
    
    //-------------------------------------144 face-----------------------------------------//
    if((gmv_bgx>60) && (gmv_bgx<87) && (_ABS_(gmv_bgy)< 5) && ((_ABS_(gmv_fgx)+ _ABS_(gmv_fgy)) < 30) && (gmv_bgcnt > 22000) && (gmv_bgcnt < 26000) && (sum_apl > 3000000) && (sum_apl < 5000000) &&
        ((fg_apl_seg0_cnt + fg_apl_seg1_cnt)*100/fg_apl_sum >45) && ((fg_apl_seg7_cnt)*100/fg_apl_sum <10) && 
        ((bg_apl_seg0_cnt + bg_apl_seg1_cnt)*100/bg_apl_sum < 40) && ((bg_apl_seg7_cnt)*100/bg_apl_sum > 15))
    {
        pOutput->u8_dh_condition_144_face_in_cnt  = pOutput->u8_dh_condition_144_face_in_cnt + 1;
        pOutput->u8_dh_condition_144_face_in_cnt = (pOutput->u8_dh_condition_144_face_in_cnt > 2) ? 2 : pOutput->u8_dh_condition_144_face_in_cnt;
    }
    else
    {
        pOutput->u8_dh_condition_144_face_in_cnt = 0;
    }
    if(pOutput->u8_dh_condition_144_face_in_cnt >= 2)
    {
        pOutput->u8_dh_condition_144_face=1;
        pOutput->u8_dh_condition_144_face_hold_cnt = 10;
    }
    else if(pOutput->u8_dh_condition_144_face_hold_cnt > 0)
    {
        pOutput->u8_dh_condition_144_face=1;
        pOutput->u8_dh_condition_144_face_hold_cnt = pOutput->u8_dh_condition_144_face_hold_cnt - 1;
    }
    else
    {
        pOutput->u8_dh_condition_144_face=0;
        pOutput->u8_dh_condition_144_face_hold_cnt = 0;
    }
    
    ReadRegister(KME_DEHALO5_PHMV_FIX_6_reg,15,15, &predflag_en);
    ReadRegister(KME_DEHALO5_PHMV_FIX_8_reg,0,9,&gmv_y0);
    ReadRegister(KME_DEHALO5_PHMV_FIX_25_reg,16,25,&pred_y0);                                                                                                                                                                       
    ReadRegister(KME_DEHALO5_PHMV_FIX_1C_reg,16,23, &auto_extend_mvdiff_th);
     //---------------for 185 woman sit down broken vertical moving----------//

    if(rgnmvy_fg[12]>14||rgnmvy_fg[13]>14||rgnmvy_fg[20]>14||rgnmvy_fg[21]>14||rgnmvy_fg[29]>14)
    {

        pOutput->u8_me1_condition_virtical_case5= 1 ;
        video185_verhold = 20 ;
    }
    else if(video185_verhold>0)
    {
        video185_verhold -- ;
        pOutput->u8_me1_condition_virtical_case5= 1 ;
    }
    else
    {
        pOutput->u8_me1_condition_virtical_case5= 0 ;
    }   
}


VOID FRC_Swing_Detect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned char  nIdxX,nIdxY,nMap0,nMap1,nMap2,nMap3;
	unsigned char SwingRight_cnt = 0,SwingLeft_cnt = 0;
	int nRMVx_1st[4]={0};
	int nRMVy_1st[4]={0};
	int nRMVx_Region[4]={0};
	int nRMVy_Region[4]={0};
	static unsigned char  u8_Swing_holdFrm=0;


	pOutput->u1_Swing_true  = 0;
	for (nIdxX=0; nIdxX<=3; nIdxX++)
	{
		for (nIdxY=0; nIdxY<=1; nIdxY++)
		{
			// init
			nMap0 = nIdxY*8+nIdxX;
			nMap1 = nIdxY*8+nIdxX+4;
			nMap2 = (nIdxY+2)*8+nIdxX;
			nMap3 = (nIdxY+2)*8+nIdxX+4;
		  	// left_top
			nRMVx_1st[0] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap0]>>1;
			nRMVy_1st[0] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap0]>>1;

			// right_top
			nRMVx_1st[1] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap1]>>1;
			nRMVy_1st[1] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap1]>>1;

			// left_bot
			nRMVx_1st[2] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap2]>>1;
			nRMVy_1st[2] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap2]>>1;

			// right_bot
			nRMVx_1st[3] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap3]>>1;
			nRMVy_1st[3] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap3]>>1;

			nRMVx_Region[0]+=nRMVx_1st[0];
			nRMVy_Region[0]+=nRMVy_1st[0];

			nRMVx_Region[1]+=nRMVx_1st[1];
			nRMVy_Region[1]+=nRMVy_1st[1];

			nRMVx_Region[2]+=nRMVx_1st[2];
			nRMVy_Region[2]+=nRMVy_1st[2];

			nRMVx_Region[3]+=nRMVx_1st[3];
			nRMVy_Region[3]+=nRMVy_1st[3];

			// left_top
			if((nRMVx_1st[0] <0) && (nRMVy_1st[0] <0))
			{
				SwingLeft_cnt++;
			}
			else if((nRMVx_1st[0] >0) && (nRMVy_1st[0] >0))
			{
				SwingRight_cnt++;
			}

			// right_top
			if((nRMVx_1st[1] >0) && (nRMVy_1st[1] <0))
			{
				SwingRight_cnt++;
			}
			else if((nRMVx_1st[1] <0) && (nRMVy_1st[1] >0))
			{
				SwingLeft_cnt++;
			}

			// left_bot
			if((nRMVx_1st[2] <0) && (nRMVy_1st[2] <0))
			{
				SwingLeft_cnt++;
			}
			else if((nRMVx_1st[2] >0) && (nRMVy_1st[2] >0))
			{
				SwingRight_cnt++;
			}

			// right_bot
			if((nRMVx_1st[1] >0) && (nRMVy_1st[1] <0))
			{
				SwingRight_cnt++;
			}
			else if((nRMVx_1st[1] <0) && (nRMVy_1st[1] >0))
			{
				SwingLeft_cnt++;
			}
		}
	}
	pOutput->u4_SwingRight_cnt = SwingRight_cnt;
	pOutput->u4_SwingLeft_cnt = SwingLeft_cnt;

	if((nRMVx_Region[0] <0) && (nRMVy_Region[0] <0) &&(nRMVx_Region[1] <0) && (nRMVy_Region[1] >0)&&(nRMVx_Region[2] <0) && (nRMVy_Region[2] <0)&&(nRMVx_Region[3] <0) && (nRMVy_Region[3] >0))
	{
		if(pOutput->u4_SwingLeft_cnt >=14)
		{
			u8_Swing_holdFrm = 16;
			pOutput->u1_Swing_true = 1;
		}
	}
	else if((nRMVx_Region[0] >0) && (nRMVy_Region[0] >0) &&(nRMVx_Region[1] >0) && (nRMVy_Region[1] <0)&&(nRMVx_Region[2] >0) && (nRMVy_Region[2] >0)&&(nRMVx_Region[3] >0) && (nRMVy_Region[3] <0))
	{
		if(pOutput->u4_SwingRight_cnt >=14)
		{
			u8_Swing_holdFrm = 16;
			pOutput->u1_Swing_true = 1;
		}
	}
	else if(u8_Swing_holdFrm >0)
	{
		u8_Swing_holdFrm--;
		pOutput->u1_Swing_true = 1;
	}
	else
	{
		u8_Swing_holdFrm = 0;
		pOutput->u1_Swing_true = 0;
	}
}

VOID FRC_SceneAnalysis_Proc(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
//	_PQLPARAMETER *PatternParam = GetPQLParameter();//for golf pattern
	FRC_Dynamic_SearchRange(pParam, pOutput);
	FRC_ZMV_AdaptPenality(pParam, pOutput);
	FRC_ME1_GMVD_Select(pParam, pOutput);
	FRC_Good_Scene_Detect(pParam, pOutput);
	FRC_SC_PFV_CDD_Number(pParam, pOutput);
	FRC_Dynamic_MVAccord(pParam, pOutput);
        //FRC_Dynamic_SAD_Diff(pParam, 	pOutput);
	#if !PQL_DYNAMIC_ADJUST_FIX_ISSUE
		return ;
	#endif
	FRC_FadeInOut_Detect(pParam, pOutput);
//	FRC_InsidePanning_Detect(pParam, pOutput);
	FRC_Dynamic_ACDCBld(pParam,		pOutput);
//	FRC_BdySmallMV_Detect(pParam, pOutput);
//	if (pParam->s2m_method_sel == 0)
//	{
//		FRC_Scene2Motion_Hist(pParam, pOutput);
//	}
//	else
//	{
//		FRC_Scene2Motion(pParam,pOutput);
//	}

//	FRC_RgPanning_Detect(pParam, 	pOutput);

	// system zoom
	FRC_Zoom_Detect(pParam, pOutput);

	FRC_SpecialSCFB_Detect(pParam, pOutput);

	FRC_RgRotat_Detect(pParam, pOutput);
	FRC_VideoZoom_Detect(pParam, pOutput);
	//FRC_Swing_Detect(pParam, pOutput);  //S&M saech on a hammock

	//normal check send flag
	MEMC_me1_bgmask_newalgo_Proc(pParam, pOutput);

//	if( !(PatternParam->_param_read_comreg.pattern_golf_flag) ) {
		MEMC_dehalo_newalgo_bgmaskBase_Proc (pParam, pOutput);
//	}
	// rtd_pr_memc_info("afterMEMC_me1_bgmask_newalgo_Proc");
//	  rtd_pr_memc_info("golf_flag:%d\n",PatternParam->_param_read_comreg.pattern_golf_flag);
}


//---------------------
//INPUT INTERRUPT CALL:
//---------------------
VOID MEMC_HDR_PicChg_Protect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if(s_pContext->_external_info.u1_HDR_Chg)
		pOutput->u8_HDR_PicChg_prt_apply = 6;

	else if(s_pContext->_output_frc_sceneAnalysis.u8_HDR_PicChg_prt_apply > 0)
		pOutput->u8_HDR_PicChg_prt_apply = s_pContext->_output_frc_sceneAnalysis.u8_HDR_PicChg_prt_apply - 1;
	else
		pOutput->u8_HDR_PicChg_prt_apply = 0;

}


VOID MEMC_OSDBrightnessBigChg_Protect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned char u8_OSDBri_sub_pre = (pOutput->u16_OSDBrightnessVal >> 8);
	unsigned char u8_OSDBri_main_pre = (pOutput->u16_OSDBrightnessVal & 0xff);

	unsigned char u8_OSDBri_sub_cur = (s_pContext->_external_info.u16_Brightness >> 8);
	unsigned char u8_OSDBri_main_cur = (s_pContext->_external_info.u16_Brightness & 0xff);


	unsigned char u8_brightness_th_l = 6; // UI Brightness Range: [0,100] , th Range: [0,255]

	if((_ABS_DIFF_(u8_OSDBri_main_pre, u8_OSDBri_main_cur) > u8_brightness_th_l)
		|| (_ABS_DIFF_(u8_OSDBri_sub_pre, u8_OSDBri_sub_cur) > u8_brightness_th_l))
		pOutput->u8_OSDBrightnessBigChg_prt_apply = 5;
	else if(pOutput->u8_OSDBrightnessBigChg_prt_apply > 0)
		pOutput->u8_OSDBrightnessBigChg_prt_apply = pOutput->u8_OSDBrightnessBigChg_prt_apply - 1;
	else
		pOutput->u8_OSDBrightnessBigChg_prt_apply = 0;

	pOutput->u16_OSDBrightnessVal = s_pContext->_external_info.u16_Brightness;

	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 7, u8_OSDBri_main_cur);
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 15, u8_OSDBri_main_pre);
}


VOID MEMC_OSDContrastBigChg_Protect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned char u8_OSDCon_sub_pre = (pOutput->u16_OSDContrastVal >> 8);
	unsigned char u8_OSDCon_main_pre = (pOutput->u16_OSDContrastVal & 0xff);

	unsigned char u8_OSDCon_sub_cur = (s_pContext->_external_info.u16_Contrast >> 8);
	unsigned char u8_OSDCon_main_cur = (s_pContext->_external_info.u16_Contrast & 0xff);


	unsigned char u8_contrast_th_l = 6; // UI Contrast Range: [0,100] , th Range: [0,255]

	if((_ABS_DIFF_(u8_OSDCon_main_pre, u8_OSDCon_main_cur) > u8_contrast_th_l)
		|| (_ABS_DIFF_(u8_OSDCon_sub_pre, u8_OSDCon_sub_cur) > u8_contrast_th_l))
		pOutput->u8_OSDContrastBigChg_prt_apply = 5;
	else if(pOutput->u8_OSDContrastBigChg_prt_apply > 0)
		pOutput->u8_OSDContrastBigChg_prt_apply = pOutput->u8_OSDContrastBigChg_prt_apply - 1;
	else
		pOutput->u8_OSDContrastBigChg_prt_apply = 0;

	pOutput->u16_OSDContrastVal = s_pContext->_external_info.u16_Contrast;

	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 23, u8_OSDCon_main_cur);
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 24, 31, u8_OSDCon_main_pre);

}

VOID MEMC_VOSeek_Protect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();

	unsigned char u1_voseek_en = s_pParam->_param_frc_sceneAnalysis.u1_mc2_var_lpf_wrt_en; // 0912,lisa,temporal use, can be change if sync all dynamicPQ from K2

	if(s_pContext->_external_info.u1_mVOSeek == 1)
		pOutput->u8_VOSeek_prt_apply = 12;
	else if(pOutput->u8_VOSeek_prt_apply >= 1)
		pOutput->u8_VOSeek_prt_apply = pOutput->u8_VOSeek_prt_apply-1;
	else
		pOutput->u8_VOSeek_prt_apply = 0;

	if(s_pContext->_external_info.u1_mVOSeek == 1)
		pOutput->u8_VOSeek_prt_logo_apply = 48;
	else if(pOutput->u8_VOSeek_prt_logo_apply >= 1)
		pOutput->u8_VOSeek_prt_logo_apply = pOutput->u8_VOSeek_prt_logo_apply - 1;
	else
		pOutput->u8_VOSeek_prt_logo_apply = 0;

	if(u1_voseek_en == 0)
	{
		pOutput->u8_VOSeek_prt_apply = 0;
		pOutput->u8_VOSeek_prt_logo_apply = 0;
	}
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 16, 19, pOutput->u8_VOSeek_prt_apply);

}

VOID MEMC_OSDPictureModeChg_Protect(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if((s_pContext->_external_info.u1_PictureMode_Chg == 1) || (s_pContext->_external_info.u1_ZoomAction == 1)){
		pOutput->u8_OSDPictureModeChg_prt_apply = 9;
		//pOutput->u8_OSDPictureModeChg_prt_apply = 1;
	}
	else if(pOutput->u8_OSDPictureModeChg_prt_apply > 0)
		pOutput->u8_OSDPictureModeChg_prt_apply = pOutput->u8_OSDPictureModeChg_prt_apply - 1;
	else
		pOutput->u8_OSDPictureModeChg_prt_apply = 0;

	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 0, 7, u8_OSDBri_main_cur);
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 8, 15, u8_OSDBri_main_pre);
}

VOID FRC_SceneAnalysis_Proc_InputIntp(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	//hdr <-> normal
      	MEMC_HDR_PicChg_Protect(pParam, pOutput);

	//osd brightness big change, broken, need fallback
	MEMC_OSDBrightnessBigChg_Protect(pParam, pOutput);
	MEMC_OSDContrastBigChg_Protect(pParam, pOutput);

	// vo seek
	MEMC_VOSeek_Protect(pParam, pOutput);

	// Picture Mode & Zoom Action
	MEMC_OSDPictureModeChg_Protect(pParam, pOutput);

	#if 1
	SAD_Peak_Skip_GFB(pParam, pOutput);
	#endif
	
	// sc sad diff calc
	FRC_Dynamic_SADDiffTh(pParam, pOutput);
}

#if 1
VOID SAD_Peak_Skip_GFB(const _PARAM_FRC_SceneAnalysis *pParam, _OUTPUT_FRC_SceneAnalysis *pOutput)
{
	unsigned int u32_RB_val;
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	#if 1
	static unsigned int u32_SAD_pre=0,u32_SAD_pre_temp=0, u32_SAD_current=0,u32_SAD_diff=0;
	static unsigned char u8_SAD_overshoot_cnt=0;
	#endif	
	#if 0 //for YE Test temp test
	static unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 28,  28, &u32_ID_Log_en);//YE Test debug dummy register
	#endif

	ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_D4_reg, 0, 30, &u32_RB_val);
		//pOutput->u30_me_aSAD_rb = u32_RB_val;


		#if 1 //YE Test
		u32_SAD_current=u32_RB_val;
		
		//if(u8_SAD_overshot_cnt==0)
			//u32_SAD_diff= ABS(u32_SAD_current,u32_SAD_pre);
		if(u32_RB_val >=3800000) //currently get SAD data from 171
		{
			u32_SAD_diff=_ABS_DIFF_(u32_SAD_current,u32_SAD_pre);
			
			if(u32_SAD_diff>(u32_SAD_pre*3))//&&(s_pContext->_output_read_comreg.u1_sc_status_rb!=1))
			{
				pOutput->u8_sad_overshoot_flag=1;
				u8_SAD_overshoot_cnt=5;
			}
			if(u8_SAD_overshoot_cnt>0)
			{
				pOutput->u8_sad_overshoot_flag=1;
				u8_SAD_overshoot_cnt--;
			
			}	
			else
			{
				//u8_SAD_overshot_cnt=0;
				pOutput->u8_sad_overshoot_flag=0;
			}
		}
		
		#if 1
		if(0)//(u32_ID_Log_en==1)
		{
	        		rtd_pr_memc_info(" [MEMC SADP] , FBResult=%d, SAD =%d,SAD_current=%d,SAD_pre=%d,SAD_pre_temp=%d,SAD_diff=%d, SAD_overshot_cnt=%d,sad_overshot_flag=%d, sc_status=%d, HW_sc=%d, \n"
						,s_pContext->_output_fblevelctrl.u8_FBResult,u32_RB_val,u32_SAD_current,u32_SAD_pre,u32_SAD_pre_temp,u32_SAD_diff,u8_SAD_overshoot_cnt
						,pOutput->u8_sad_overshoot_flag,s_pContext->_output_read_comreg.u1_sc_status_rb,s_pContext->_output_fblevelctrl.u1_HW_sc_true);
		}
		#endif
		u32_SAD_pre=u32_RB_val; //YE Test


		#endif





	
}
#endif


