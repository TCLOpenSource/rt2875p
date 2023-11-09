#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_reg_def.h"
//////////////////////////////////////////////////////////////////////////
static VOID LbmcMode_Detect_onlyCnt(const _PARAM_MC_LBMC_SWITCH *pParam, _OUTPUT_MC_LBMC_SWITCH *pOutput, unsigned char band_Type)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned short u16_cnt_totalNum = (s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb);

	unsigned char  u8_param_Ovlap_s2n_cntTh, u8_param_Lmv_s2n_cntTh, u8_param_Ovlap_n2s_cntTh, u8_param_Lmv_n2s_cntTh, u8_lbmc_mode;
	unsigned short u16_rb_smv_posCnt, u16_rb_smv_negCnt, u16_rb_Lmv_posCnt, u16_rb_Lmv_negCnt;

	unsigned char    i=0,j=0,diff_th=120;
	unsigned int    u8_me1_statis_mvy_th1,u8_me1_statis_mvy_th2;
	static unsigned char  u8_reduce_8to60_flag=0, u8_reduce_under60_flag=0,u8_0to7_flag=0,u8_24to31_flag=0;
	signed short  u16_8to23_mean_rmv;
	signed short  u10_rmv_mvy[32],rmv_diff[32];
	//unsigned int u32_RB_val_smv_post,u32_RB_val_smv_neg,u32_RB_val_lmv_post,u32_RB_val_lmv_neg;
		
	#if 0 //for YE Test temp test
	static unsigned int u32_ID_Log_en=0,u32_ID_Log_en2=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 16,  16, &u32_ID_Log_en);//YE Test debug dummy register
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 17,  17, &u32_ID_Log_en2);//YE Test debug dummy register
	ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_08_reg,8,17,&u8_me1_statis_mvy_th1);  //8
	ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_08_reg,18,27,&u8_me1_statis_mvy_th2); //60
	#endif

	#if 0
	ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_48_reg, 0, 19, &u32_RB_val_smv_post);
	ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_58_reg, 0, 19, &u32_RB_val_smv_neg);
	ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_4C_reg, 0, 19, &u32_RB_val_lmv_post);
	ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_5C_reg, 0, 19, &u32_RB_val_lmv_neg);
	#endif



	for(i=0;i<32;i++)
		{
			u10_rmv_mvy[i]=s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];

		}
	u16_8to23_mean_rmv=(u10_rmv_mvy[8]+u10_rmv_mvy[9]+u10_rmv_mvy[10]+u10_rmv_mvy[11]+u10_rmv_mvy[12]+u10_rmv_mvy[13]+u10_rmv_mvy[14]+u10_rmv_mvy[15]\
				+u10_rmv_mvy[16]+u10_rmv_mvy[17]+u10_rmv_mvy[18]+u10_rmv_mvy[19]+u10_rmv_mvy[20]+u10_rmv_mvy[21]+u10_rmv_mvy[22]+u10_rmv_mvy[23])/16;

	#if 0
	if(u32_ID_Log_en2==1)
		{
			if(u8_lbmc_mode==1)
			rtd_pr_memc_info("[MEMC YE_RMV],8to23_mean_rmv=%d, rmv_00_mvy=%d,rmv_01_mvy=%d,rmv_02_mvy=%d,rmv_03_mvy=%d,rmv_04_mvy=%d ,rmv_05_mvy=%d,rmv_06_mvy=%d,rmv_07_mvy=%d \
					,rmv_08_mvy=%d,rmv_09_mvy=%d,rmv_10_mvy=%d,rmv_11_mvy=%d,rmv_12_mvy=%d ,rmv_13_mvy=%d,rmv_14_mvy=%d,rmv_15_mvy=%d \
					,rmv_16_mvy=%d,rmv_17_mvy=%d,rmv_18_mvy=%d,rmv_19_mvy=%d,rmv_20_mvy=%d ,rmv_21_mvy=%d,rmv_22_mvy=%d,rmv_23_mvy=%d \
				     	,rmv_24_mvy=%d,rmv_25_mvy=%d,rmv_26_mvy=%d,rmv_27_mvy=%d ,rmv_28_mvy=%d,rmv_29_mvy=%d,rmv_30_mvy=%d,rmv_31_mvy=%d,\n",
				     u16_8to23_mean_rmv,u10_rmv_mvy[0],u10_rmv_mvy[1],u10_rmv_mvy[2],u10_rmv_mvy[3],u10_rmv_mvy[4],u10_rmv_mvy[5],u10_rmv_mvy[6],u10_rmv_mvy[7],\
				     u10_rmv_mvy[8],u10_rmv_mvy[9],u10_rmv_mvy[10],u10_rmv_mvy[11],u10_rmv_mvy[12],u10_rmv_mvy[13],u10_rmv_mvy[14],u10_rmv_mvy[15],\
				     u10_rmv_mvy[16],u10_rmv_mvy[17],u10_rmv_mvy[18],u10_rmv_mvy[19],u10_rmv_mvy[20],u10_rmv_mvy[21],u10_rmv_mvy[22],u10_rmv_mvy[23],\
				     u10_rmv_mvy[24],u10_rmv_mvy[25],u10_rmv_mvy[26],u10_rmv_mvy[27],u10_rmv_mvy[28],u10_rmv_mvy[29],u10_rmv_mvy[30],u10_rmv_mvy[31]);
		}
	#endif


	if (band_Type == _LBMC_LF_) // LF
	{
		u16_rb_smv_posCnt        = s_pContext->_output_read_comreg.u17_me_posCnt_0_rb;
		u16_rb_smv_negCnt        = s_pContext->_output_read_comreg.u17_me_negCnt_0_rb;
		u16_rb_Lmv_posCnt        = s_pContext->_output_read_comreg.u17_me_posCnt_1_rb;
		u16_rb_Lmv_negCnt        = s_pContext->_output_read_comreg.u17_me_negCnt_1_rb;

		u8_param_Ovlap_s2n_cntTh = pParam->u8_lf_Ovlap_s2n_cntTh;
		u8_param_Lmv_s2n_cntTh   = pParam->u8_lf_Lmv_s2n_cntTh;
		u8_param_Ovlap_n2s_cntTh = pParam->u8_lf_Ovlap_n2s_cntTh;
		u8_param_Lmv_n2s_cntTh   = pParam->u8_lf_Lmv_n2s_cntTh;

		u8_lbmc_mode             = pOutput->u8_lf_lbmcMode;
		#if 0
		/*
		if(u32_ID_Log_en==1)
		{
			if(u8_lbmc_mode==1)
			rtd_pr_memc_info("[MEMC YE_Band_LF],band_Type=%d,smv_posCnt=%d,smv_negCnt=%d,Lmv_posCnt=%d,Lmv_negCnt=%d,Ovlap_s2n_cntTh=%d,Lmv_s2n_cntTh=%d,Ovlap_n2s_cntTh=%d,Lmv_n2s_cntTh=%d,mode=%d, \n ",
				band_Type,u16_rb_smv_posCnt,u16_rb_smv_negCnt,u16_rb_Lmv_posCnt,u16_rb_Lmv_negCnt,
				u8_param_Ovlap_s2n_cntTh,u8_param_Lmv_s2n_cntTh,u8_param_Ovlap_n2s_cntTh,u8_param_Lmv_n2s_cntTh,u8_lbmc_mode);

		}	
		*/
		
		if(u32_ID_Log_en==1)
			{
				//if(u8_lbmc_mode==1)
				rtd_pr_memc_info("[MEMC_YE_Mode_LF],band_Type=%d, cnt_totalNum=%d,rb_smv_posCnt=%d,rb_smv_negCnt=%d,rb_Lmv_posCnt=%d,rb_Lmv_negCnt =%d, \n",
					//band_Type,u16_cnt_totalNum,u32_RB_val_smv_post,u32_RB_val_smv_neg,u32_RB_val_lmv_post,u32_RB_val_lmv_neg);
					band_Type,u16_cnt_totalNum,u16_rb_smv_posCnt,u16_rb_smv_negCnt,u16_rb_Lmv_posCnt,u16_rb_Lmv_negCnt);


			}


		
		#endif

		
	}
	else // Hf
	{
		u16_rb_smv_posCnt        = s_pContext->_output_read_comreg.u17_me_posCnt_2_rb;
		u16_rb_smv_negCnt        = s_pContext->_output_read_comreg.u17_me_negCnt_2_rb;
		u16_rb_Lmv_posCnt        = s_pContext->_output_read_comreg.u17_me_posCnt_3_rb;
		u16_rb_Lmv_negCnt        = s_pContext->_output_read_comreg.u17_me_negCnt_3_rb;

		u8_param_Ovlap_s2n_cntTh = pParam->u8_hf_Ovlap_s2n_cntTh;
		u8_param_Lmv_s2n_cntTh   = pParam->u8_hf_Lmv_s2n_cntTh;
		u8_param_Ovlap_n2s_cntTh = pParam->u8_hf_Ovlap_n2s_cntTh;
		u8_param_Lmv_n2s_cntTh   = pParam->u8_hf_Lmv_n2s_cntTh;

		u8_lbmc_mode             = pOutput->u8_hf_lbmcMode;
		#if 0
		
		if(u32_ID_Log_en==1)
		{
			if(u8_lbmc_mode==1)
			rtd_pr_memc_info("[MEMC YE_Band_HF],lbmc_mode=%d,band_Type=%d,smv_posCnt=%d,smv_negCnt=%d,Lmv_posCnt=%d,Lmv_negCnt=%d,Ovlap_s2n_cntTh=%d,Lmv_s2n_cntTh=%d,Ovlap_n2s_cntTh=%d,Lmv_n2s_cntTh=%d,mode=%d, \n ",
				u8_lbmc_mode,band_Type,u16_rb_smv_posCnt,u16_rb_smv_negCnt,u16_rb_Lmv_posCnt,u16_rb_Lmv_negCnt,
				u8_param_Ovlap_s2n_cntTh,u8_param_Lmv_s2n_cntTh,u8_param_Ovlap_n2s_cntTh,u8_param_Lmv_n2s_cntTh,u8_lbmc_mode);

		}
		#endif
	}

#if 1  //YE Test to avoid rim mv impact correvt panning detection
	  //YE Test it's workaround temp solution, Hsync SMV info ready, remove it
if(0)
{
	for(i=0 ;i<8;i++)
	{
		j=i+24;
		rmv_diff[i]=_ABS_DIFF_(u16_8to23_mean_rmv,u10_rmv_mvy[i]);
		rmv_diff[j]=_ABS_DIFF_(u16_8to23_mean_rmv,u10_rmv_mvy[j]);
		
			if(rmv_diff[i] >diff_th) //diff_th=120;
			{
				u8_0to7_flag=1;
				if((u10_rmv_mvy[i]< -u8_me1_statis_mvy_th1)&&(u10_rmv_mvy[i]> -u8_me1_statis_mvy_th2))
					{
						if(u16_rb_smv_negCnt>500)
						u16_rb_smv_negCnt=u16_rb_smv_negCnt-500;
						if(u16_cnt_totalNum>500)
						u16_cnt_totalNum=u16_cnt_totalNum-500;
						u8_reduce_8to60_flag=1;
					}
				else if(u10_rmv_mvy[i]< -u8_me1_statis_mvy_th2)
					{
						if(u16_rb_smv_negCnt>500)
						u16_rb_smv_negCnt=u16_rb_smv_negCnt-500;
						if(u16_rb_Lmv_negCnt>500)
						u16_rb_Lmv_negCnt = u16_rb_Lmv_negCnt-500;
						if(u16_cnt_totalNum>500)
						u16_cnt_totalNum=u16_cnt_totalNum-500;
						u8_reduce_under60_flag=1;
					}
				if((u10_rmv_mvy[i]> u8_me1_statis_mvy_th1)&&(u10_rmv_mvy[i]<u8_me1_statis_mvy_th2))
					{
						if(u16_rb_smv_posCnt>500)
						u16_rb_smv_posCnt=u16_rb_smv_posCnt-500;
						if(u16_cnt_totalNum>500)
						u16_cnt_totalNum=u16_cnt_totalNum-500;
						u8_reduce_8to60_flag=1;
					}
				else if(u10_rmv_mvy[i]>u8_me1_statis_mvy_th2)
					{
						if(u16_rb_smv_posCnt>500)
						u16_rb_smv_posCnt=u16_rb_smv_posCnt-500;
						if(u16_rb_Lmv_posCnt>500)
						u16_rb_Lmv_posCnt = u16_rb_Lmv_posCnt-500;
						if(u16_cnt_totalNum>500)
						u16_cnt_totalNum=u16_cnt_totalNum-500;
						u8_reduce_under60_flag=1;
					}
			
			}
			
			if(rmv_diff[j] >diff_th)
			{
				u8_24to31_flag=1;
				if((u10_rmv_mvy[j]< -u8_me1_statis_mvy_th1)&&(u10_rmv_mvy[j]> -u8_me1_statis_mvy_th2))
					{
						if(u16_rb_smv_negCnt>500)
						u16_rb_smv_negCnt=u16_rb_smv_negCnt-500;
						if(u16_cnt_totalNum>500)
						u16_cnt_totalNum=u16_cnt_totalNum-500;
						u8_reduce_8to60_flag=1;
					}
				else if(u10_rmv_mvy[j]< -u8_me1_statis_mvy_th2)
					{
						if(u16_rb_smv_negCnt>500)
						u16_rb_smv_negCnt=u16_rb_smv_negCnt-500;
						if(u16_rb_Lmv_negCnt>500)
						u16_rb_Lmv_negCnt = u16_rb_Lmv_negCnt-500;
						if(u16_cnt_totalNum>500)
						u16_cnt_totalNum=u16_cnt_totalNum-500;
						u8_reduce_under60_flag=1;
					}
				if((u10_rmv_mvy[j]>u8_me1_statis_mvy_th1)&&(u10_rmv_mvy[j]<u8_me1_statis_mvy_th2))
					{
						if(u16_rb_smv_posCnt>500)
						u16_rb_smv_posCnt=u16_rb_smv_posCnt-500;
						if(u16_cnt_totalNum>500)
						u16_cnt_totalNum=u16_cnt_totalNum-500;
						u8_reduce_8to60_flag=1;
					}
				else if(u10_rmv_mvy[j]>u8_me1_statis_mvy_th2)
					{
						if(u16_rb_smv_posCnt>500)
						u16_rb_smv_posCnt=u16_rb_smv_posCnt-500;
						if(u16_rb_Lmv_posCnt>500)
						u16_rb_Lmv_posCnt = u16_rb_Lmv_posCnt-500;
						if(u16_cnt_totalNum>500)
						u16_cnt_totalNum=u16_cnt_totalNum-500;
						u8_reduce_under60_flag=1;
					}
			}

			#if 0
			if(u32_ID_Log_en2==1)
			{
				if(u8_lbmc_mode==1)
					rtd_pr_memc_info("[MEMC YE_RMV_DIFF],8to23_mean_rmv=%d,i=%d,j=%d,0to7_flag=%d,reduce_8to60_flag=%d,24to31_flag=%d,reduce_under60_flag=%d,rmv_diff_i= %d,rmv_diff_j= %d,8to23_mean_rmv=%d,rmv_mvy[i]=%d,rmv_mvy[j]=%d,diff_th=%d,rb_smv_negCnt=%d,rb_Lmv_negCnt=%d,u16_rb_smv_posCnt=%d,u16_rb_Lmv_posCnt=%d, \n",
						u16_8to23_mean_rmv,i,j,u8_0to7_flag,u8_reduce_8to60_flag,u8_24to31_flag,u8_reduce_under60_flag,rmv_diff[i],rmv_diff[j],u16_8to23_mean_rmv,u10_rmv_mvy[i],u10_rmv_mvy[j],diff_th,u16_rb_smv_negCnt,u16_rb_Lmv_negCnt,u16_rb_smv_posCnt,u16_rb_Lmv_posCnt);
							
			}			
			#endif
			
		}


		u8_reduce_8to60_flag=0;
		u8_reduce_under60_flag=0;
		u8_0to7_flag=0;
		u8_24to31_flag=0;

}		
#endif


	// mode switch
	{
		unsigned short u16_smv_near0_cnt, u16_ovlap_posCnt, u16_ovlap_negCnt;
		unsigned char  u1_n2su_en = 0, u1_n2sd_en = 0, u1_su2n_en = 0, u1_sd2n_en = 0;

		u16_smv_near0_cnt = (u16_cnt_totalNum < (u16_rb_smv_posCnt + u16_rb_smv_negCnt))? 0 : (u16_cnt_totalNum - (u16_rb_smv_posCnt + u16_rb_smv_negCnt));
		u16_ovlap_posCnt  = _MIN_(u16_rb_smv_posCnt + u16_smv_near0_cnt, 0xFFFF);
		u16_ovlap_negCnt  = _MIN_(u16_rb_smv_negCnt + u16_smv_near0_cnt, 0xFFFF);

		if (band_Type == _LBMC_LF_)
		{
			u1_n2su_en = ((u16_ovlap_negCnt  << 8) > u16_cnt_totalNum * pParam->u8_lf_Ovlap_n2s_cntTh ) && \
						 ((u16_rb_Lmv_negCnt << 8) > u16_cnt_totalNum * pParam->u8_lf_Lmv_n2s_cntTh);
			u1_su2n_en = ((u16_ovlap_negCnt  << 8) < u16_cnt_totalNum * pParam->u8_lf_Ovlap_s2n_cntTh) || \
						 ((u16_rb_Lmv_negCnt << 8) < u16_cnt_totalNum * pParam->u8_lf_Lmv_s2n_cntTh);

			u1_n2sd_en = ((u16_ovlap_posCnt  << 8) > u16_cnt_totalNum * pParam->u8_lf_Ovlap_n2s_cntTh) && \
						 ((u16_rb_Lmv_posCnt << 8) > u16_cnt_totalNum * pParam->u8_lf_Lmv_n2s_cntTh);
			u1_sd2n_en = ((u16_ovlap_posCnt  << 8) < u16_cnt_totalNum * pParam->u8_lf_Ovlap_s2n_cntTh) || \
						 ((u16_rb_Lmv_posCnt << 8) < u16_cnt_totalNum * pParam->u8_lf_Lmv_s2n_cntTh  );
			#if 0
			if(u32_ID_Log_en==1)
			{
				//if(u8_lbmc_mode==1)
				rtd_pr_memc_info("[MEMC YE_Mode_LF],band_Type=%d, n2su_en=%d, su2n_en=%d,n2sd_en=%d,sd2n_en=%d,smv_near0_cnt=%d,cnt_totalNum=%d,rb_smv_posCnt=%d,rb_smv_negCnt=%d,ovlap_posCnt=%d,rb_Lmv_posCnt=%d,ovlap_negCnt=%d,rb_Lmv_negCnt =%d, \n",
					band_Type,u1_n2su_en,u1_su2n_en,u1_n2sd_en,u1_sd2n_en,u16_smv_near0_cnt,u16_cnt_totalNum,u16_rb_smv_posCnt,u16_rb_smv_negCnt,u16_ovlap_posCnt,u16_rb_Lmv_posCnt,u16_ovlap_negCnt,u16_rb_Lmv_negCnt);

			}
			#endif
		}
		else
		{
			u1_n2su_en = ((u16_ovlap_negCnt  << 8) > u16_cnt_totalNum * pParam->u8_hf_Ovlap_n2s_cntTh ) && \
						 ((u16_rb_Lmv_negCnt << 8) > u16_cnt_totalNum * pParam->u8_hf_Lmv_n2s_cntTh);
			u1_su2n_en = ((u16_ovlap_negCnt  << 8) < u16_cnt_totalNum * pParam->u8_hf_Ovlap_s2n_cntTh) || \
						 ((u16_rb_Lmv_negCnt << 8) < u16_cnt_totalNum * pParam->u8_hf_Lmv_s2n_cntTh);

			u1_n2sd_en = ((u16_ovlap_posCnt  << 8) > u16_cnt_totalNum * pParam->u8_hf_Ovlap_n2s_cntTh) && \
						 ((u16_rb_Lmv_posCnt << 8) > u16_cnt_totalNum * pParam->u8_hf_Lmv_n2s_cntTh);
			u1_sd2n_en = ((u16_ovlap_posCnt  << 8) < u16_cnt_totalNum * pParam->u8_hf_Ovlap_s2n_cntTh) || \
						 ((u16_rb_Lmv_posCnt << 8) < u16_cnt_totalNum * pParam->u8_hf_Lmv_s2n_cntTh  );
			#if 0
			if(u32_ID_Log_en==1)
			{
				if(u8_lbmc_mode==1)
				rtd_pr_memc_info("[MEMC YE_Mode_HF],band_Type=%d, n2su_en=%d, su2n_en=%d,n2sd_en=%d,sd2n_en=%d,smv_near0_cnt=%d,cnt_totalNum=%d,rb_smv_posCnt=%d,rb_smv_negCnt=%d,ovlap_posCnt=%d,rb_Lmv_posCnt=%d,ovlap_negCnt=%d,rb_Lmv_negCnt =%d, \n",
					band_Type,u1_n2su_en,u1_su2n_en,u1_n2sd_en,u1_sd2n_en,u16_smv_near0_cnt,u16_cnt_totalNum,u16_rb_smv_posCnt,u16_rb_smv_negCnt,u16_ovlap_posCnt,u16_rb_Lmv_posCnt,u16_ovlap_negCnt,u16_rb_Lmv_negCnt);

			}
			#endif
		}

		if (u8_lbmc_mode == _MC_NORMAL_)
		{
			u8_lbmc_mode = (u1_n2su_en == 1)? _MC_SINGLE_UP_ : ((u1_n2sd_en == 1)? _MC_SINGLE_DOWN_ : _MC_NORMAL_);

		}
		else if (u8_lbmc_mode == _MC_SINGLE_UP_)
		{
			u8_lbmc_mode = (u1_su2n_en == 1)? _MC_NORMAL_ : _MC_SINGLE_UP_;
		}
		else
		{
			u8_lbmc_mode = (u1_sd2n_en == 1)? _MC_NORMAL_ : _MC_SINGLE_DOWN_;
		}

		#if 0
		if(u32_ID_Log_en2==1)
			{
				if(u8_lbmc_mode==1)
					rtd_pr_memc_info("[MEMC YE_lbmc_mode] , u8_lbmc_mode=%d , \n", u8_lbmc_mode  );
							
			}			
		#endif



		if(0)
		{
			rtd_pr_memc_notice("ovp_posCnt = %d, ovp_negCnt = %d, Lmv_posCnt = %d, Lmv_negCnt = %d, total_cnt = %d \r\n", \
				u16_ovlap_posCnt, u16_ovlap_negCnt, u16_rb_Lmv_posCnt, u16_rb_Lmv_negCnt, u16_cnt_totalNum);
		}
	}


	if (band_Type == _LBMC_LF_)
	{
		pOutput->u8_lf_lbmcMode = u8_lbmc_mode;
	}
	else
	{
		pOutput->u8_hf_lbmcMode = u8_lbmc_mode;
	}

	// debug printf
	if(0)
	{
		if(band_Type == _LBMC_LF_)
		{
			rtd_pr_memc_notice("lf_lbmc_mode = %d \r\n", pOutput->u8_lf_lbmcMode);
		}
		else
		{
			rtd_pr_memc_notice("Hf_lbmc_mode = %d \r\n", pOutput->u8_hf_lbmcMode);
		}
	}

}
//////////////////////////////////////////////////////////////////////////

VOID Mc_lbmcSwitch_Init( _OUTPUT_MC_LBMC_SWITCH *pOutput)
{
	pOutput->u8_lf_lbmcMode = _MC_NORMAL_;
	pOutput->u8_hf_lbmcMode = _MC_NORMAL_;
}

VOID Mc_lbmcSwitch_Proc(const _PARAM_MC_LBMC_SWITCH *pParam, _OUTPUT_MC_LBMC_SWITCH *pOutput)
{
/*	const _PQLCONTEXT *s_pContext = GetPQLContext();*/

	if (pParam->u8_lbmcSwitch_bypass == 0)
	{
		LbmcMode_Detect_onlyCnt(pParam, pOutput, _LBMC_LF_);
		LbmcMode_Detect_onlyCnt(pParam, pOutput, _LBMC_HF_);
	}
	else
	{
		pOutput->u8_lf_lbmcMode = pParam->u8_lbmcSwitch_bypass - 1;
		pOutput->u8_hf_lbmcMode = pParam->u8_lbmcSwitch_bypass - 1;
	}
}


