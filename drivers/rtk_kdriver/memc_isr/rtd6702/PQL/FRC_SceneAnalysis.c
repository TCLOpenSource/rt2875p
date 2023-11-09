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
	// dehalo switch
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

	if(pParam->u1_me1_gmvd_sel_en)
	{
		if(((_ABS_DIFF_(u11_GMV_1st_mvx, u11_GMV_2nd_mvx) > pParam->u11_me1_gmvd_sel_mvx_diff_thr) ||
		     (_ABS_DIFF_(u10_GMV_1st_mvy, u10_GMV_2nd_mvy) > pParam->u10_me1_gmvd_sel_mvy_diff_thr)) &&
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
	PQL_OUTPUT_RESOLUTION output_Resolution = s_pContext->_external_data._output_resolution;
	const _Param_By_Resolution_RimCtrl *s_pRimParam = GetRimParam(output_Resolution);
	int i;
	unsigned char u8_GoodRegion_cnt = 0, u8_BadRegion_cnt = 0;
	unsigned int ui_Good_Rgn_SAD_th = 0, ui_Good_gmv_unconf_th = 0, ui_Good_Rgn_TC_th;
	unsigned int ui_Bad_Rgn_SAD_th = 0, ui_Bad_gmv_unconf_th = 0, ui_Bad_Rgn_TC_th;
	unsigned char u8_TopRim_th = 4*s_pRimParam->u8_scale_V; //4k: u8_TopRim_th=16
	unsigned char u8_LftRim_th = 6*s_pRimParam->u8_scale_H; //4k: u8_LftRim_th=24
	unsigned short u12_tmp_v = s_pRimParam->s16_resolution_height>>2; //4k: u12_tmp_v=540
	unsigned short u12_tmp_h = s_pRimParam->s16_resolution_width>>3; //4k: u12_tmp_h=480

	if(pParam->u1_SA_en)
	{
		for(i = 0; i < 32; i++)
		{
			if(s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] >= u8_TopRim_th)
			{
				if((i >= 0 && i < 8) || (i >= 24 && i < 32))
				{
					ui_Good_Rgn_SAD_th = (u12_tmp_v - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u20_SA_good_rgn_SAD_th / u12_tmp_v;
					ui_Good_gmv_unconf_th = (u12_tmp_v - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u12_SA_good_rgn_unconf_th / u12_tmp_v;
					ui_Good_Rgn_TC_th = (u12_tmp_v - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u12_SA_good_rgn_TC_th / u12_tmp_v;
					ui_Bad_Rgn_SAD_th = (u12_tmp_v - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u20_SA_bad_rgn_SAD_th / u12_tmp_v;
					ui_Bad_gmv_unconf_th = (u12_tmp_v - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u12_SA_bad_rgn_unconf_th / u12_tmp_v;
					ui_Bad_Rgn_TC_th = (u12_tmp_v - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP]) *
											pParam->u12_SA_bad_rgn_TC_th / u12_tmp_v;
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
			else if(s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT] >= u8_LftRim_th)
			{
				if((i == 0) || (i == 8) || (i == 16) || (i == 24) || (i == 7) || (i == 15) || (i == 23) || (i == 31))
				{
					ui_Good_Rgn_SAD_th = (u12_tmp_h - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u20_SA_good_rgn_SAD_th / u12_tmp_h;
					ui_Good_gmv_unconf_th = (u12_tmp_h - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u12_SA_good_rgn_unconf_th / u12_tmp_h;
					ui_Good_Rgn_TC_th = (u12_tmp_h - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u12_SA_good_rgn_TC_th / u12_tmp_h;
					ui_Bad_Rgn_SAD_th = (u12_tmp_h - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u20_SA_bad_rgn_SAD_th / u12_tmp_h;
					ui_Bad_gmv_unconf_th = (u12_tmp_h - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u12_SA_bad_rgn_unconf_th / u12_tmp_h;
					ui_Bad_Rgn_TC_th = (u12_tmp_h - s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT]) *
											pParam->u12_SA_bad_rgn_TC_th / u12_tmp_h;
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
	unsigned int u32_i = 0, u32_rmv_cnt_sum = 0, u32_cnt = 0, rmv_cnt_avg = 0, log_en = 0;

	const _PQLCONTEXT *s_pContext = GetPQLContext();

	int u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb) >> 2;
	int u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb) >> 2;
	unsigned int u16_cnt_totalNum = s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb;
	unsigned char  u6_gmv_ratio = (s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb << 6) / u16_cnt_totalNum;
	//unsigned char  u16_rmv_th_l_offset = 40,u16_rmv_th_h_offset = 50;
	unsigned int  u16_rmv_th_l = (pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l * u16_cnt_totalNum >> 9);
	unsigned int  u16_rmv_th_h = (pParam->u4_dyn_SAD_Diff_rmv_ratio_th_h * u16_cnt_totalNum >> 9);//d5a8[28:31]
	//reference dtl
	unsigned int u25_meAllDtl    = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	unsigned char dubug_info = 0;

	// dynamic rmv_th_l  more dtl mv messy
	if(u25_meAllDtl>=2500000 && pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l >6)
		u16_rmv_th_l=((pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l-6) * u16_cnt_totalNum >> 9) ;
	else if(u25_meAllDtl>=1700000 && pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l >3)
		u16_rmv_th_l=((pParam->u4_dyn_SAD_Diff_rmv_ratio_th_l-3) * u16_cnt_totalNum >> 9) ;
	//=============================

	pOutput->u32_new_saddiff_th = pParam->u28_dyn_SAD_Diff_sadAll_th_l;//d5a4[0:27]
	pOutput->u6_gmv_ratio = u6_gmv_ratio;
	ReadRegister(SOFTWARE3_SOFTWARE3_01_reg, 24, 24, &log_en);

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
			dubug_info = 1;
			if(u32_cnt)
			{
				rmv_cnt_avg = u32_rmv_cnt_sum/u32_cnt;
				dubug_info = 2;

				if(rmv_cnt_avg >= u16_rmv_th_h)
				{
					pOutput->u32_new_saddiff_th = pParam->u28_dyn_SAD_Diff_sadAll_th_h;
					dubug_info = 3;
				}
				else if(rmv_cnt_avg > (u16_rmv_th_l>>2))
				{
					pOutput->u32_new_saddiff_th = (((rmv_cnt_avg -  (u16_rmv_th_l>>2)) * u32_saddiff_th_step) >> 2) + \
							s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_sc_saddiff_th;
					dubug_info = 4;
				}
			}
		}
		else if((u6_gmv_ratio > pParam->u6_dyn_SAD_Diff_gmv_ratio_th) && (_ABS_(u11_gmv_mvx) >= 2 || _ABS_(u10_gmv_mvy) >= 1))  // normal
		{
			u32_saddiff_th_step = ((u32_saddiff_th_tmp << 2) / (u16_rmv_th_h - u16_rmv_th_l));
			dubug_info = 11;
			if(u32_cnt)
			{
				rmv_cnt_avg = u32_rmv_cnt_sum/u32_cnt;
				dubug_info = 12;
				if(rmv_cnt_avg >= u16_rmv_th_h)
				{
					pOutput->u32_new_saddiff_th = pParam->u28_dyn_SAD_Diff_sadAll_th_h;
					dubug_info = 13;
				}
				else if(rmv_cnt_avg > u16_rmv_th_l)
				{
					pOutput->u32_new_saddiff_th = (((rmv_cnt_avg - u16_rmv_th_l) * u32_saddiff_th_step) >> 2) + \
							s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_sc_saddiff_th;
					dubug_info = 14;
				}
			}
		}
		else if((u6_gmv_ratio > pParam->u6_dyn_SAD_Diff_gmv_ratio_th) && (_ABS_(u11_gmv_mvx) <= 1 && _ABS_(u10_gmv_mvy) <= 1))
		{
			u32_saddiff_th_step = ((u32_saddiff_th_tmp << 2) / (u16_rmv_th_h - u16_rmv_th_l));
			dubug_info = 21;
			if(u32_cnt)
			{
				rmv_cnt_avg = u32_rmv_cnt_sum/u32_cnt;
				dubug_info = 22;
				if(rmv_cnt_avg >= u16_rmv_th_h)
				{
					pOutput->u32_new_saddiff_th = pParam->u28_dyn_SAD_Diff_sadAll_th_h*3;
					dubug_info = 23;
				}
				else if(rmv_cnt_avg > u16_rmv_th_l)
				{
					pOutput->u32_new_saddiff_th = (((rmv_cnt_avg - u16_rmv_th_l) * u32_saddiff_th_step) >> 2) + \
							s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_sc_saddiff_th;
					dubug_info = 24;
				}
			}
		}

		if(log_en){//d904[24]
			rtd_pr_memc_emerg("[%s] ,%d,info,%d,%d,%d,%d,rmv_cnt_avg,%d,%d,%d,totalNum,%d,sad_diff,%d,%d,\n", __FUNCTION__, dubug_info, 
				u6_gmv_ratio, pParam->u6_dyn_SAD_Diff_gmv_ratio_th, _ABS_(u11_gmv_mvx), _ABS_(u10_gmv_mvy),
				rmv_cnt_avg, u16_rmv_th_h, u16_rmv_th_l, u16_cnt_totalNum,
				pParam->u28_dyn_SAD_Diff_sadAll_th_h, u32_saddiff_th_step);
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
	// re-init when chang the dehalo mode
	FRC_SceneAnalysis_Init(pOutput);

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


