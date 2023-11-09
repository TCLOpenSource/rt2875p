#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_isr/PQL/Patch_Manage.h"
#include "memc_isr/MID/mid_mode.h"
#include "memc_reg_def.h"

VOID patch_HF_Detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned short u16_mcHdtl 		= s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt;
	unsigned short u16_mcVdtl 		= s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt;
	unsigned short u11_GMV_1st_mvx 	= _ABS_(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	unsigned short u10_GMV_1st_mvy 	= _ABS_(s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	unsigned int u26_me_aAPLi		= s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned int u25_me_aDTL_rb	= s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	
	unsigned char  u8_hdtl_gain1 = pParam->u8_HF1_Hdtl_thr_gain;
	unsigned char  u8_vdtl_gain1 = pParam->u8_HF1_Vdtl_thr_gain;
	unsigned char  u8_hdtl_gain2 = pParam->u8_HF2_Hdtl_thr_gain;
	unsigned char  u8_vdtl_gain2 = pParam->u8_HF2_Vdtl_thr_gain;
	unsigned char  u8_hdtl_gain3 = pParam->u8_HF3_Hdtl_thr_gain;
	unsigned char  u8_vdtl_gain3 = pParam->u8_HF3_Vdtl_thr_gain;

	if(pParam->u1_HF1_Detect_en)	//LCD_broken
	{
		if((u16_mcHdtl < ((0x2700 * (128 + u8_hdtl_gain1)) >> 8)) && ((u16_mcHdtl > (0x600 * (128 + u8_hdtl_gain1)) >> 8)) &&
		   (u16_mcVdtl < ((0x80 * (128 + u8_vdtl_gain1)) >> 8)) && ((u16_mcVdtl > (0x20 * (128 + u8_vdtl_gain1)) >> 8)) &&
		   (u11_GMV_1st_mvx < pParam->u8_HF1_mvx_thr) && (u10_GMV_1st_mvy < pParam->u7_HF1_mvy_thr))
		{
			pOutput->u1_HF1_Detected = 1;
		}
		else
		{
			pOutput->u1_HF1_Detected = 0;
		}
	}
	else
	{
		pOutput->u1_HF1_Detected = 0;
		pOutput->u6_HF1_hold_cnt = 0;
	}

	if(pParam->u1_HF2_Detect_en)	//repeat_pat3
	{
		if((u16_mcHdtl < ((0xB00 * (128 + u8_hdtl_gain2) >> 8))) && ((u16_mcHdtl > (0x600 * (128 + u8_hdtl_gain2) >> 8))) &&
		   (u16_mcVdtl < ((0xB00 * (128 + u8_vdtl_gain2) >> 8))) && ((u16_mcVdtl > (0x600 * (128 + u8_vdtl_gain2) >> 8))) &&
		   (u11_GMV_1st_mvx < pParam->u8_HF2_mvx_thr) && (u10_GMV_1st_mvy < pParam->u7_HF2_mvy_thr))
		{
			pOutput->u1_HF2_Detected = 1;
		}
		else
		{
			pOutput->u1_HF2_Detected = 0;
		}
	}
	else
	{
		pOutput->u1_HF2_Detected = 0;		
		pOutput->u6_HF2_hold_cnt = 0;
	}	

	if(pParam->u1_HF3_Detect_en)	//Female_College_Student
	{
		if((u16_mcHdtl < ((0xC0 * (128 + u8_hdtl_gain3) >> 8))) && ((u16_mcHdtl > (0x10 * (128 + u8_hdtl_gain3) >> 8))) &&
		   (u16_mcVdtl < ((0x800 * (128 + u8_vdtl_gain3) >> 8))) && ((u16_mcVdtl > (0x1 * (128 + u8_vdtl_gain3) >> 8))) &&
		   (u11_GMV_1st_mvx < pParam->u8_HF3_mvx_thr) && (u10_GMV_1st_mvy < pParam->u7_HF3_mvy_thr) &&
		   (u26_me_aAPLi < 0x4C0000) && (u26_me_aAPLi > 0x380000) &&
		   (u25_me_aDTL_rb > 0x100000))
		{
			pOutput->u1_HF3_Detected = 1;
		}
		else
		{
			pOutput->u1_HF3_Detected = 0;
		}
	}
	else
	{
		pOutput->u1_HF3_Detected = 0;
		pOutput->u6_HF3_hold_cnt = 0;
	}
#if 0
	if(pParam->u1_HF1_Detect_en || pParam->u1_HF2_Detect_en || pParam->u1_HF3_Detect_en)
	{
		rtd_pr_memc_emerg("[%s][%d][%x][%x][%x][%x][%x]\n", __FUNCTION__, __LINE__, 
				u16_mcHdtl, u16_mcVdtl, u11_GMV_1st_mvx, u10_GMV_1st_mvy,
				pOutput->u1_HF3_Detected << 2 | pOutput->u1_HF2_Detected << 1 | pOutput->u1_HF1_Detected); 
	}
#endif
}

VOID patch_HF_Action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	if(pParam->u1_HF1_Detect_en)
	{
		if(pOutput->u1_HF1_Detected)
		{
			pOutput->u6_HF1_hold_cnt = 0x20;
		}
		else if(pOutput->u6_HF1_hold_cnt > 0)
		{
			pOutput->u6_HF1_hold_cnt = pOutput->u6_HF1_hold_cnt - 1;
		}
	}
	if(pParam->u1_HF2_Detect_en)
	{
		if(pOutput->u1_HF2_Detected)
		{			
			pOutput->u6_HF2_hold_cnt = 0x20;
		}
		else if(pOutput->u6_HF2_hold_cnt > 0)
		{
			pOutput->u6_HF2_hold_cnt = pOutput->u6_HF2_hold_cnt - 1;
		}
	}
	if(pParam->u1_HF3_Detect_en)
	{
		if(pOutput->u1_HF3_Detected)
		{			
			pOutput->u6_HF3_hold_cnt = 0x20;
		}
		else if(pOutput->u6_HF3_hold_cnt > 0)
		{
			pOutput->u6_HF3_hold_cnt = pOutput->u6_HF3_hold_cnt - 1;
		}
	}
	
	if((pParam->u1_HF1_Detect_en) && (pOutput->u6_HF1_hold_cnt > 0))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 31, 0x33333333);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 31, 0x33333333);		
	}
	else if((pParam->u1_HF2_Detect_en) && (pOutput->u6_HF2_hold_cnt > 0))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 31, 0x33333333);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 31, 0x33333333);		
	}
	else if((pParam->u1_HF3_Detect_en) && (pOutput->u6_HF3_hold_cnt > 0))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 31, 0x33333333);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 31, 0x33333333);		
	}
	else if((pParam->u1_HF1_Detect_en) || (pParam->u1_HF2_Detect_en) || (pParam->u1_HF3_Detect_en))
	{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 31, 0x77777777);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 31, 0x77777777); 					
	}
}

VOID patch_RidingHorse_Detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char  u8_Uamount_gain	= pParam->u8_RidingHorse_Uamount_thr_gain;
	unsigned char  u8_Vamount_gain	= pParam->u8_RidingHorse_Vamount_thr_gain;
	unsigned int u25_me_aDTL_rb	= s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	
	unsigned short u11_GMV_1st_mvx 	= _ABS_(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	unsigned short u10_GMV_1st_mvy 	= _ABS_(s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	unsigned int u32_bottom_u_amount = (s_pContext->_output_read_comreg.u16_color_amount_u[4] + 
								  s_pContext->_output_read_comreg.u16_color_amount_u[5] + 
								  s_pContext->_output_read_comreg.u16_color_amount_u[6] +
								  s_pContext->_output_read_comreg.u16_color_amount_u[7]) / 4;
	unsigned int u32_bottom_v_amount = (s_pContext->_output_read_comreg.u16_color_amount_v[4] + 
								  s_pContext->_output_read_comreg.u16_color_amount_v[5] + 
								  s_pContext->_output_read_comreg.u16_color_amount_v[6] +
								  s_pContext->_output_read_comreg.u16_color_amount_v[7]) / 4;
	
	if(pParam->u1_RidingHorse_Detect_en)
	{
		if((u32_bottom_u_amount < ((0xC000 * (128 + u8_Uamount_gain) >> 8))) && 
		    (u32_bottom_u_amount > ((0x9000 * (128 + u8_Uamount_gain) >> 8))) &&
		    (u32_bottom_v_amount < ((0xF800 * (128 + u8_Vamount_gain) >> 8))) &&
		    (u32_bottom_v_amount > ((0xC000 * (128 + u8_Vamount_gain) >> 8))) &&
		   (u11_GMV_1st_mvx < pParam->u8_RidingHorse_mvx_thr) && (u10_GMV_1st_mvy < pParam->u7_RidingHorse_mvy_thr) &&
		   (u25_me_aDTL_rb < 0xF0000) && (u25_me_aDTL_rb > 0x90000))
		{
			pOutput->u1_RidingHorse_Detected = 1;
		}
		else
		{
			pOutput->u1_RidingHorse_Detected = 0;
		}
	}
	else
	{
		pOutput->u1_RidingHorse_Detected = 0;
		pOutput->u6_RidingHorse_hold_cnt = 0;
	}
#if 0	
	if(pParam->u1_RidingHorse_Detect_en)
	{
		rtd_pr_memc_emerg("[%s][%d][%x][%x][%x][%x][%x][%x]\n", __FUNCTION__, __LINE__, 
				u32_bottom_u_amount, u32_bottom_v_amount, u11_GMV_1st_mvx, u10_GMV_1st_mvy, u25_me_aDTL_rb,
				pOutput->u1_RidingHorse_Detected); 
	}
#endif
}

VOID patch_RidingHorse_Action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	if(pParam->u1_RidingHorse_Detect_en)
	{
		if(pOutput->u1_RidingHorse_Detected)
		{
			pOutput->u6_RidingHorse_hold_cnt = 0x20;
		}
		else if(pOutput->u6_RidingHorse_hold_cnt > 0)
		{
			pOutput->u6_RidingHorse_hold_cnt = pOutput->u6_RidingHorse_hold_cnt - 1;
		}

		if(pOutput->u6_RidingHorse_hold_cnt > 0)
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_A8_reg,12,18, 0);
		}
		else
		{
			WriteRegister(KME_DEHALO_KME_DEHALO_A8_reg,12,18, 0x10);
		}
	}
}

VOID  patch_BluceMic_detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char i = 0, u8_sad_match_cnt = 0, u8_tc_match_cnt = 0, u8_dtl_match_cnt = 0, u8_apl_match_cnt = 0;
		
	for(i = 0; i < 32; i++)
	{
		// region sad match
		if(s_pContext->_output_read_comreg.u25_me_rSAD_rb[i] < 0x7000) 
		{
			u8_sad_match_cnt++;
		}
		if(u8_sad_match_cnt >= 26)
		{
			pOutput->u1_sad_matched = 1;
		}
		else
		{
			pOutput->u1_sad_matched = 0;
		}
		// region tc match		
		if(s_pContext->_output_read_comreg.u22_me_rTC_rb[i] < 0x1C00)
		{
			u8_tc_match_cnt++;
		}
		if(u8_tc_match_cnt >= 26)
		{
			pOutput->u1_tc_matched = 1;
		}
		else
		{
			pOutput->u1_tc_matched = 0;
		}
		// region detail match
		if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[i] < 0x13000)
		{
			u8_dtl_match_cnt++;
		}
		if(u8_dtl_match_cnt >= 28)
		{
			pOutput->u1_dtl_matched = 1;
		}
		else
		{
			pOutput->u1_dtl_matched = 0;
		}	
		// region apl match
		if((s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i] > 0xc000) &&
		   (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i] < 0x30000) &&
		   (s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i] > 0xc000) &&
		   (s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i] < 0x30000)
		  )
		{
			u8_apl_match_cnt++;
		}
		if(u8_apl_match_cnt >= 30)
		{
			pOutput->u1_apl_matched = 1;
		}
		else
		{
			pOutput->u1_apl_matched = 0;
		}		
	}
	
	if((s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] == 0) &&
		(s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT] == 2159) &&
		(s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT] == 0) &&
		(s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_RHT] == 3839) &&	// no rim
		(pOutput->u1_sad_matched && pOutput->u1_tc_matched && pOutput->u1_dtl_matched && pOutput->u1_apl_matched) &&
		(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb >= 0) &&
		(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb < 0x10) &&
		(s_pContext->_output_read_comreg.u25_me_aDTL_rb > 0xF0000) &&
		(s_pContext->_output_read_comreg.u25_me_aDTL_rb < 0x190000) &&
		(s_pContext->_output_read_comreg.u26_me_aAPLi_rb > 0x390000) &&
		(s_pContext->_output_read_comreg.u26_me_aAPLi_rb < 0x450000) &&
		(s_pContext->_output_read_comreg.u26_me_aAPLp_rb > 0x390000) &&
		(s_pContext->_output_read_comreg.u26_me_aAPLp_rb < 0x450000)
	   )		
	{
		pOutput->u1_BluceMic_Detected = 1;
	}
	else
	{
		pOutput->u1_BluceMic_Detected = 0;
	}
}

VOID  patch_BluceMic_action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	if(pParam->u1_BluceMic_En == 1)
	{
		if(pOutput->u1_BluceMic_Detected == 1)
		{
	        WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 31, 0xFFFFFFFF);
	        WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 31, 0xFFFFFFFF);
		}
		else
		{
	        WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 31, 0x11131111);
	        WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 31, 0x11111111);
		}
	}
}

VOID  patch_casino_action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
#if 1/*need resolve:compiler error*/
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if (pParam->u1_patch_casino_en == 1){
		if (s_pContext->_output_me_sceneAnalysis.u1_casino_RP_detect_true == 1)
		{
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_4C_reg, 0, 12, 0x20);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_0C_reg, 0, 12, 0x20);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_17_reg, 16, 23, 0x10);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_18_reg, 8, 18, 0x7F9);
		}
		else // if (pOutput->u1_patchCasio_true == 0 && pOutput->u1_patchCasio_preTrue == 1)
		{
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_4C_reg, 0, 12, 0x6);
			WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_0C_reg, 0, 12, 0x6);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_17_reg, 16, 23, 0xC);
			WriteRegister(KME_ME1_TOP1_ME1_COMMON1_18_reg, 8, 18, 0x7FB);
		}
	}
#endif
}

VOID  patch_HighFreqPlayerOSD_detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned short u16_mc_Hdtl_rb         = s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt;
	unsigned short u16_mc_Vdtl_rb         = s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt;
	static unsigned char u8_vipStill_cnt = 0;
	static unsigned char u8_mcdtl_cnt = 0;
	unsigned char u1_mc_rdtl_true = 0, mc_rdtl_cnt = 0, u1_me_sad_true = 0, me_sad_cnt = 0, i=0;

	for(i=0; i<32; i++)
	{
		if(s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[i]<0x3)
			mc_rdtl_cnt++;
		
		if(s_pContext->_output_read_comreg.u25_me_rSAD_rb[i]<0x30)
			me_sad_cnt++;
			
	}

	if(mc_rdtl_cnt>22)
		u1_mc_rdtl_true = 1;

	if(me_sad_cnt>12)
		u1_me_sad_true = 1;
		
	if(u1_me_sad_true && u1_mc_rdtl_true && (s_pContext->_external_info.u1_VIP_Still || u8_vipStill_cnt>0) && u16_mc_Hdtl_rb>0x1f00 && u16_mc_Vdtl_rb>0x500)
	{
		pOutput->u1_patch_highFreq_zmv_true = 1;

		if(s_pContext->_external_info.u1_VIP_Still)
			u8_vipStill_cnt = 20;
		else
			u8_vipStill_cnt--;
	}
	else
	{
		pOutput->u1_patch_highFreq_zmv_true = 0;
		u8_vipStill_cnt = 0;
	}
	
	if(u1_me_sad_true && u16_mc_Hdtl_rb>0x4000 && u16_mc_Vdtl_rb>0x2000)
	{
		pOutput->u1_patch_highFreq_rp_true = 1;
		u8_mcdtl_cnt = 20;
	}
	else if(u1_me_sad_true && u8_mcdtl_cnt>0)
	{
		u8_mcdtl_cnt--;
	}
	else
	{
		pOutput->u1_patch_highFreq_rp_true = 0;
		u8_mcdtl_cnt = 0;
	}

	
#if 0
{
	unsigned int ip30=0, ip29=0, ip28=0, ip27=0, ip26=0;
	ReadRegister(KMC_TOP_kmc_top_24_reg,30,30,&ip30);
	ReadRegister(KMC_TOP_kmc_top_24_reg,29,29,&ip29);
	ReadRegister(KMC_TOP_kmc_top_24_reg,28,28,&ip28);
	ReadRegister(KMC_TOP_kmc_top_24_reg,27,27,&ip27);
	ReadRegister(KMC_TOP_kmc_top_24_reg,26,26,&ip26);

	if(ip30)
	rtd_pr_memc_info("%d %d..%d %d.. %d %d.. %x %x\n", pOutput->u1_patch_highFreq_zmv_true, pOutput->u1_patch_highFreq_rp_true,
	mc_rdtl_cnt, me_sad_cnt, 
	s_pContext->_external_info.u1_VIP_Still, u8_vipStill_cnt, 
	u16_mc_Hdtl_rb, u16_mc_Vdtl_rb);


	if(ip29)
	rtd_pr_memc_info("sad %d %d.. %x %x %x %x %x %x %x %x .. %x %x %x %x %x %x %x %x.. %x %x %x %x %x %x %x %x..%x %x %x %x %x %x %x %x \n", mc_rdtl_cnt, me_sad_cnt, 
	s_pContext->_output_read_comreg.u25_me_rSAD_rb[0], s_pContext->_output_read_comreg.u25_me_rSAD_rb[1], s_pContext->_output_read_comreg.u25_me_rSAD_rb[2], s_pContext->_output_read_comreg.u25_me_rSAD_rb[3], s_pContext->_output_read_comreg.u25_me_rSAD_rb[4], s_pContext->_output_read_comreg.u25_me_rSAD_rb[5], s_pContext->_output_read_comreg.u25_me_rSAD_rb[6], s_pContext->_output_read_comreg.u25_me_rSAD_rb[7], 
	s_pContext->_output_read_comreg.u25_me_rSAD_rb[8], s_pContext->_output_read_comreg.u25_me_rSAD_rb[9], s_pContext->_output_read_comreg.u25_me_rSAD_rb[10], s_pContext->_output_read_comreg.u25_me_rSAD_rb[11], s_pContext->_output_read_comreg.u25_me_rSAD_rb[12], s_pContext->_output_read_comreg.u25_me_rSAD_rb[13], s_pContext->_output_read_comreg.u25_me_rSAD_rb[14], s_pContext->_output_read_comreg.u25_me_rSAD_rb[15], 
	s_pContext->_output_read_comreg.u25_me_rSAD_rb[16], s_pContext->_output_read_comreg.u25_me_rSAD_rb[17], s_pContext->_output_read_comreg.u25_me_rSAD_rb[18], s_pContext->_output_read_comreg.u25_me_rSAD_rb[19], s_pContext->_output_read_comreg.u25_me_rSAD_rb[20], s_pContext->_output_read_comreg.u25_me_rSAD_rb[21], s_pContext->_output_read_comreg.u25_me_rSAD_rb[22], s_pContext->_output_read_comreg.u25_me_rSAD_rb[23], 
	s_pContext->_output_read_comreg.u25_me_rSAD_rb[24], s_pContext->_output_read_comreg.u25_me_rSAD_rb[25], s_pContext->_output_read_comreg.u25_me_rSAD_rb[26], s_pContext->_output_read_comreg.u25_me_rSAD_rb[27], s_pContext->_output_read_comreg.u25_me_rSAD_rb[28], s_pContext->_output_read_comreg.u25_me_rSAD_rb[29], s_pContext->_output_read_comreg.u25_me_rSAD_rb[30], s_pContext->_output_read_comreg.u25_me_rSAD_rb[31]);

	if(ip28)
	rtd_pr_memc_info("dtl %d %d.. %x %x %x %x %x %x %x %x .. %x %x %x %x %x %x %x %x.. %x %x %x %x %x %x %x %x..%x %x %x %x %x %x %x %x \n", mc_rdtl_cnt, me_sad_cnt, 
	s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[0], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[1], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[2], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[3], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[4], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[5], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[6], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[7], 
	s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[8], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[9], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[10], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[11], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[12], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[13], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[14], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[15], 
	s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[16], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[17], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[18], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[19], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[20], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[21], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[22], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[23], 
	s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[24], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[25], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[26], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[27], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[28], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[29], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[30], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[31]);

	if(ip27)
	rtd_pr_memc_info("apl %d %d.. %x %x %x %x %x %x %x %x .. %x %x %x %x %x %x %x %x.. %x %x %x %x %x %x %x %x..%x %x %x %x %x %x %x %x \n", mc_rdtl_cnt, me_sad_cnt, 
	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[0], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[1], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[2], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[3], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[4], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[5], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[6], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[7], 
	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[8], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[9], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[11], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[13], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[15], 
	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[17], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[19], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[23], 
	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[27], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[29], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[31]);

	if(ip26)
	rtd_pr_memc_info("apl %d %d.. %x %x %x %x %x %x %x %x .. %x %x %x %x %x %x %x %x.. %x %x %x %x %x %x %x %x..%x %x %x %x %x %x %x %x \n", mc_rdtl_cnt, me_sad_cnt, 
	s_pContext->_output_read_comreg.u20_me_rAPLp_rb[0], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[1], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[2], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[3], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[4], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[5], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[6], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[7], 
	s_pContext->_output_read_comreg.u20_me_rAPLp_rb[8], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[9], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[11], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[13], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[15], 
	s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[19], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[23], 
	s_pContext->_output_read_comreg.u20_me_rAPLp_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[27], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[29], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[31]);

	
}
#endif

}

VOID  patch_golfBall_detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
#if 0/*need resolve:compiler error*/

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned short u16_mc_Hdtl_rb         = s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt;
	unsigned short u16_mc_Vdtl_rb         = s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt;

	unsigned char  u1_Vdtl_true           = u16_mc_Vdtl_rb >= pParam->u16_golfBall_bHsV_Vdtl_th0 && u16_mc_Vdtl_rb <= pParam->u16_golfBall_bHsV_Vdtl_th1;
	unsigned char  u1_Hdtl_true           = u16_mc_Hdtl_rb >= pParam->u16_golfBall_bHsV_Hdtl_th0 && u16_mc_Hdtl_rb <= pParam->u16_golfBall_bHsV_Hdtl_th1;

	u1_Vdtl_true                       = u1_Vdtl_true || pParam->u1_golfBall_Vdtl_byp;
	u1_Hdtl_true                       = u1_Hdtl_true || pParam->u1_golfBall_Hdtl_byp;
	pOutput->u1_golfBall_HVdtl_FB_true = u1_Vdtl_true && u1_Hdtl_true;
#endif
}
VOID  patch_golfBall_action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
#if 0/*need resolve:compiler error*/

	if(pOutput->u1_golfBall_HVdtl_FB_true == 1 && pOutput->u1_golfBall_HVdtl_FB_preTrue == 0)
	{
		unsigned int u32_rb_data;

		ReadRegister(FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_obme_mode_ADDR, 8, 19, &u32_rb_data);
		pOutput->u12_golfBall_bk_obmeMode = u32_rb_data;

		ReadRegister(MC2_MC2_58_reg, 12, 19, &u32_rb_data);
		pOutput->u8_golfBall_bk_varLPF_y  = u32_rb_data;


		WriteRegister(FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_obme_mode_ADDR, 8, 19, 0);
		WriteRegister(MC2_MC2_58_reg, 12, 19,            0);

		pOutput->u1_golfBall_HVdtl_FB_preTrue = 1;
	}
	else if(pOutput->u1_golfBall_HVdtl_FB_true == 0 && pOutput->u1_golfBall_HVdtl_FB_preTrue == 1)
	{
		WriteRegister(FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_obme_mode_ADDR, 8, 19, pOutput->u12_golfBall_bk_obmeMode);
		WriteRegister(MC2_MC2_58_reg, 12, 19,            pOutput->u8_golfBall_bk_varLPF_y);

		pOutput->u1_golfBall_HVdtl_FB_preTrue = 0;
	}
#endif

}


VOID  patch_BallJump_detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
#if 0
	//const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u32_background_tc = 0;
	unsigned int u32_background_sad = 0;

	unsigned int u32_foreground_sad = 0;
	unsigned int u32_foreground_tc[4] = {0};
	unsigned int u32_foreground_tc_diff[4] = {0};

	unsigned int u32_background_tc_th = 0xFF;
	unsigned int u32_background_sad_th = 0xF00;
	unsigned int u32_foreground_tc_th = 0x4F00;
	unsigned int u32_foreground_sad_th_h = 0x18000;
	unsigned int u32_foreground_sad_th_l = 0x3000;

	unsigned char  u1_tc_diff_true = 0;
	unsigned char  u1_apl_true = 1;

 	unsigned int u32_RB_val = 0;
// 	unsigned int u32_RB_param;
#if 0/*need resolve:compiler error*/

 	ReadRegister(FRC_TOP__PQL_0__pql_patch_dummy_F4_ADDR, 0, 31, &u32_RB_val);
#endif

// 	ReadRegister(FRC_TOP__PQL_0__pql_patch_dummy_F8_ADDR, 0, 31, &u32_RB_param);
// 	u32_background_sad_th = u32_RB_param & 0xFFFFFFFF;
// 	ReadRegister(FRC_TOP__PQL_0__pql_patch_dummy_FC_ADDR, 0, 31, &u32_RB_param);
// 	u32_foreground_tc_th = (u32_RB_param>>8) & 0xFFFFFF;
// 	ReadRegister(FRC_TOP__PQL_0__pql_patch_dummy_D0_ADDR, 0, 31, &u32_RB_param);
// 	u32_foreground_sad_th_h  = u32_RB_param & 0xFFFFFFFF;
// 	ReadRegister(FRC_TOP__PQL_0__pql_patch_dummy_D4_ADDR, 0, 31, &u32_RB_param);
// 	u32_foreground_sad_th_l  = u32_RB_param & 0xFFFFFFFF;


	unsigned char  i = 0;
#if 0/*need resolve:compiler error*/
	for (i = 0; i < 32; i ++)
	{
		unsigned int u23_rgnSad_rb = s_pContext->_output_read_comreg.u23_subSAD[i];
		unsigned int u23_rgnTC_rb = s_pContext->_output_read_comreg.u23_subTC[i];

		if (((i%8)>=2) && ((i%8)<=5))
		{
			u32_foreground_sad += u23_rgnSad_rb;
			u32_foreground_tc[i/8] += u23_rgnTC_rb;
		}
		else
		{
			if (((i%8)>5) || (i<16))
			{
				u32_background_sad += u23_rgnSad_rb;
			}

			u32_background_tc  += u23_rgnTC_rb;
		}
	}

#endif
	for (i = 0; i < 4; i ++)
	{
		u32_foreground_tc_diff[i] = _ABS_DIFF_(u32_foreground_tc[i], pOutput->u32_foreground_tc_pre[i]);
		pOutput->u32_foreground_tc_pre[i] = u32_foreground_tc[i];

		if (u32_foreground_tc_diff[i] > u32_foreground_tc_th)
		{
			u1_tc_diff_true = 1;
		}
	}
#if 0/*need resolve:compiler error*/

	for (i = 0; i < 6; i ++)
	{
		if ((s_pContext->_output_read_comreg.u8_lbme_apl_rb[26+i] < 140) || (s_pContext->_output_read_comreg.u8_lbme_apl_rb[24+i] > 160))
		{
			u1_apl_true = 0;
		}
	}
#endif
	if (((u32_background_tc < u32_background_tc_th) || ((u32_RB_val>>25)&0x1))
		&& ((u32_background_sad < u32_background_sad_th)  || ((u32_RB_val>>26)&0x1))
		&& ((u32_foreground_sad > u32_foreground_sad_th_l)  || ((u32_RB_val>>27)&0x1))
		&& ((u32_foreground_sad < u32_foreground_sad_th_h) || ((u32_RB_val>>28)&0x1))
		&& ((u1_tc_diff_true == 1) || ((u32_RB_val>>29)&0x1))
		&& ((u1_apl_true == 1) || ((u32_RB_val>>30)&0x1)))
	{
		pOutput->u1_BallJump_true = 1;
		pOutput->s8_BallJump_holdCnt = 28;
	}
	else
	{
		pOutput->u1_BallJump_true = 0;
		pOutput->s8_BallJump_holdCnt = (pOutput->s8_BallJump_holdCnt <= 0) ? 0 : (pOutput->s8_BallJump_holdCnt-1);
	}
#else
	pOutput->u1_BallJump_true = 0;
	pOutput->s8_BallJump_holdCnt = 0;
#endif
}
VOID  patch_BallJump_action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	if (pOutput->s8_BallJump_holdCnt > 0)
	{
		pOutput->u1_dh_close = 1;
	}
	else
	{
		pOutput->u1_dh_close = 0;
	}
}

VOID  patch_Soccer_detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{

}
VOID  patch_Soccer_action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if(pParam->u1_patch_soccer_en == 0)
		return;

	if (pParam->u1_patch_soccer_en && s_pContext->_external_info.u8_Soccer_holdfrm > 0)
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, 0xA0); // 0x90
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0x80);
		
		WriteRegister(MC_MC_B4_reg, 16, 23, 0xFF);
		WriteRegister(MC_MC_B4_reg, 24, 31, 0xFF);
		
//		WriteRegister(FRC_TOP__KME_DEHALO2__reg_dh_pfv_check_dif_gain_i_ADDR, 1, 7, 0x0);
//		WriteRegister(FRC_TOP__KME_DEHALO2__reg_dh_pfv_check_dif_gain_p_ADDR, 8, 14, 0x0);
	}
	else
	{
		WriteRegister(IPPRE_IPPRE_04_reg, 4, 11, 0xA0); // 0xFF
		WriteRegister(IPPRE_IPPRE_04_reg, 16, 23, 0x60); // 0x20
		
		WriteRegister(MC_MC_B4_reg, 16, 23, 0x68); // 0xC0
		WriteRegister(MC_MC_B4_reg, 24, 31, 0xC0); // 0xFF
		
//		WriteRegister(FRC_TOP__KME_DEHALO2__reg_dh_pfv_check_dif_gain_i_ADDR, 1, 7, 0x10);
//		WriteRegister(FRC_TOP__KME_DEHALO2__reg_dh_pfv_check_dif_gain_p_ADDR, 8, 14, 0xC);
	}
}

VOID  patch_tri_ball_Issue_detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char  u8_cadence_Id = (s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL]&0xF);
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	PQL_OUTPUT_FRAME_RATE out_fmRate = s_pContext->_external_data._output_frameRate;
	
	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	unsigned int u32_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	
	if(u8_cadence_Id == _CAD_VIDEO && (in_fmRate == _PQL_IN_50HZ || in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_100HZ || out_fmRate == _PQL_OUT_120HZ)){
		pOutput->u1_tri_ball_Detected = 1;
		pOutput->u6_tri_ball_hold_cnt = 8;
	}else if(((u8_cadence_Id == _CAD_22 && (in_fmRate == _PQL_IN_50HZ || in_fmRate == _PQL_IN_60HZ))||( u8_cadence_Id == _CAD_VIDEO && (in_fmRate == _PQL_IN_30HZ || in_fmRate == _PQL_IN_25HZ))) && 
				( _ABS_(u11_gmv_mvx)>25 || _ABS_(u10_gmv_mvy)>25) && (u32_gmv_cnt>=20000) ){
		if(s_pContext->_external_info.u8_triball_holdfrm > 0){
			pOutput->u1_tri_ball_Detected = 4;			
		}else{
			pOutput->u1_tri_ball_Detected = 3;
		}
		pOutput->u6_tri_ball_hold_cnt = 8;
	}else if(((u8_cadence_Id == _CAD_22 && (in_fmRate == _PQL_IN_50HZ || in_fmRate == _PQL_IN_60HZ))||( u8_cadence_Id == _CAD_VIDEO && (in_fmRate == _PQL_IN_30HZ || in_fmRate == _PQL_IN_25HZ))) && 
				((_ABS_(u11_gmv_mvx)<=25 && _ABS_(u11_gmv_mvx)>=1)&&(_ABS_(u10_gmv_mvy)<=25) && _ABS_(u10_gmv_mvy)>=1)&&(u32_gmv_cnt>=15000)){
		pOutput->u1_tri_ball_Detected = 2;
		pOutput->u6_tri_ball_hold_cnt = 8;
	}else{
		
		if(pOutput->u6_tri_ball_hold_cnt>0){
			pOutput->u6_tri_ball_hold_cnt--;
		}else{			
			pOutput->u1_tri_ball_Detected = 0;
			pOutput->u6_tri_ball_hold_cnt = 0;
		}	
	}	
	
}

VOID  patch_tri_ball_Issue_action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	mc_mc_b4_RBUS mc_mc_b4_reg;	
	unsigned int mvd_y_max = 224;
	unsigned int mvd_y_min = 128;

	mc_mc_b4_reg.regValue = rtd_inl(MC_MC_B4_reg);

	//const _PQLCONTEXT *s_pContext = GetPQLContext();

	if(pParam->u1_tri_ball_Issue_en == 0)
		return;

	switch(pOutput->u1_tri_ball_Detected){
		case 1:
			mvd_y_max = 224;
			mvd_y_min = 192;		
			break;
		case 2:
			mvd_y_max = 224;
			mvd_y_min = 128;		
			break;
		case 3:
			mvd_y_max = 224;
			mvd_y_min = 192;	
			break;
		case 4:
			mvd_y_max = 240;
			mvd_y_min = 240;
			break;
		default:
			mvd_y_max = 224;
			mvd_y_min = 128;
			break;
	}

	mc_mc_b4_reg.mc_bld_mvd_y_max = mvd_y_max;//0xc0;
	mc_mc_b4_reg.mc_bld_mvd_y_min = mvd_y_min;//0x68;
	rtd_outl(MC_MC_B4_reg, mc_mc_b4_reg.regValue);

}

static unsigned int region1 = 0, region2 = 0;
VOID patch_Forbes_action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char memc_mode = s_pContext->_external_info.u16_MEMCMode;
	unsigned char SAD_1 = 0, SAD_2 = 0, SAD_3 = 0, SAD_4 = 0, SAD_5 = 0;
	unsigned char rMV_1 = 0, rMV_2 = 0, rMV_3 = 0, rMV_4 = 0;
	unsigned char region_cnt;
	static unsigned char SAD_cnt = 0, rMV_cnt = 0;
	static unsigned char Forbes_update = 0;
	
	for(region_cnt = 0; region_cnt < 8; region_cnt++){
		if(s_pContext->_output_read_comreg.u25_me_rSAD_rb[region_cnt] == 0){
			SAD_1++;
		}
		if(s_pContext->_output_read_comreg.u25_me_rSAD_rb[region_cnt+8] != 0){
			SAD_2++;
		}
		if(s_pContext->_output_read_comreg.u25_me_rSAD_rb[region_cnt+16] != 0){
			SAD_3++;
		}
		if(s_pContext->_output_read_comreg.u25_me_rSAD_rb[region_cnt+24] == 0){
			SAD_4++;
		}

		if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[region_cnt] == 0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[region_cnt] == 0)){
			rMV_1++;
		}
		if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[region_cnt+8] != 0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[region_cnt+8] == 0)){
			rMV_2++;
		}
		if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[region_cnt+16] != 0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[region_cnt+16] == 0)){
			rMV_3++;
		}
		if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[region_cnt+24] == 0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[region_cnt+24] == 0)){
			rMV_4++;
		}
	}

	if((s_pContext->_output_read_comreg.u25_me_rSAD_rb[8] == 0) && (s_pContext->_output_read_comreg.u25_me_rSAD_rb[15] == 0)
		&& (s_pContext->_output_read_comreg.u25_me_rSAD_rb[16] == 0) && (s_pContext->_output_read_comreg.u25_me_rSAD_rb[23] == 0)
		&& (s_pContext->_output_read_comreg.u25_me_rSAD_rb[24] == 0) && (s_pContext->_output_read_comreg.u25_me_rSAD_rb[31] == 0)){
		SAD_5 = 1;
	}

	if(SAD_1 == 8 && SAD_2 <= 3 && SAD_3 <= 3 && (SAD_4 >= 6 && SAD_4 <= 8) && (SAD_5 == 1)){
		SAD_cnt++;
		if(SAD_cnt >= 80)
			SAD_cnt = 80;
	}
	else{
		if(SAD_cnt -1 < 1)
			SAD_cnt = 1;
		else
			SAD_cnt--;
	}
	if(SAD_cnt > 15)
		pOutput->SAD_flg_h = 1;
	else
		pOutput->SAD_flg_h = 0;

	if((rMV_1 >= 5 && rMV_1 <= 8) && rMV_2 <= 4 && rMV_3 <= 4 && (rMV_4 >= 5 && rMV_4 <= 8)){
		rMV_cnt++;
		if(rMV_cnt >= 90)
			rMV_cnt = 90;
	}
	else{
		if(rMV_cnt -1 < 1)
			rMV_cnt = 1;
		else
			rMV_cnt--;
	}
	if(rMV_cnt > 10)
		pOutput->rMV_flg_h = 1;
	else
		pOutput->rMV_flg_h = 0;

	if((s_pContext->_external_info.u1_Forbes_flg) && (memc_mode != 3 && memc_mode != 5) && (pOutput->SAD_flg_h && pOutput->rMV_flg_h))
		pOutput->u1_Forbes_Patch = 1;
	else
		pOutput->u1_Forbes_Patch = 0;

	if(((s_pContext->_external_info.u1_Forbes_flg) && (memc_mode != 3 && memc_mode != 5) && (pOutput->SAD_flg_h && pOutput->rMV_flg_h)) && pParam->u1_ForceForbes_en){
		if(Forbes_update==0){
			ReadRegister(KME_IPME_KME_IPME_40_reg, 0, 31, &region1);
			ReadRegister(KME_IPME_KME_IPME_44_reg, 0, 31, &region2);
			Forbes_update = 1;
		}
		WriteRegister(KME_IPME_KME_IPME_40_reg, 0, 31, 0x00039096);
		WriteRegister(KME_IPME_KME_IPME_44_reg, 0, 31, 0x0006E54D);
	}
	else{
		if(Forbes_update==1){
			WriteRegister(KME_IPME_KME_IPME_40_reg, 0, 31, region1);
			WriteRegister(KME_IPME_KME_IPME_44_reg, 0, 31, region2);
			Forbes_update = 0;
		}
	}
}

int g_memc_switch_state = 0;
unsigned char g_power_saving_flag = 0;
unsigned int DTV_PowerSaving_stage = 0;
extern unsigned int frm_cnt_test1;
extern unsigned char VR360_en;
extern unsigned char u8_MEMCMode;
extern unsigned int dejudder;
extern VOID MEMC_Lib_set_memc_bypass_to_enable_flag(unsigned char u1_enable);
extern unsigned char MEMC_Lib_get_memc_bypass_to_enable_flag(VOID);
extern VOID MEMC_Lib_set_memc_enable_to_bypass_flag(unsigned char u1_enable);
extern unsigned char MEMC_Lib_get_memc_enable_to_bypass_flag(VOID);
extern VOID MEMC_Lib_set_memc_freeze_status_flag(unsigned char u1_enable);
extern unsigned char MEMC_Lib_get_memc_freeze_status_flag(VOID);
extern BOOL MEMC_LibGetMEMC_PCModeEnable(VOID);
extern VOID MEMC_LibDisableMEDMA(VOID);
extern VOID MEMC_LibSet_LBMC_DB_en(unsigned int u1_enable);
extern VOID Mid_Mode_SetMEMCFrameRepeatEnable(BOOL bEnable);
VOID patch_MEMC_bypass_switch(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
#if 1
	unsigned int memc_switch_en;
	static unsigned int memc_switch_frm_cnt = 1;
	static unsigned int memc_bypass_state = 0;
	unsigned int frm_num = 14;
//	unsigned int test_val, test_val2, test_val3, test_val4;
	unsigned int out_phase;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char me_clk_state = 0;

#ifdef MEMC_BRING_UP
	return;
#endif

	ReadRegister(KPHASE_kphase_9C_reg, 0, 5, &out_phase);
	ReadRegister(SOFTWARE1_SOFTWARE1_63_reg, 30, 30, &memc_switch_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 28, 31, &frm_num);
	me_clk_state = (rtd_inl(SYS_REG_SYS_CLKEN3_reg)>>30)&0x1;

	if(((MEMC_Lib_get_memc_bypass_to_enable_flag() == 1) || (memc_switch_frm_cnt >= 2)) && (DTV_PowerSaving_stage == 0)){
		g_memc_switch_state = 1;
	}else if((MEMC_Lib_get_memc_enable_to_bypass_flag() == 1) || (DTV_PowerSaving_stage != 0)){
		g_memc_switch_state = 2;
	}else{
		g_memc_switch_state = 0;
	}

	if(memc_switch_en == 1 && (g_memc_switch_state == 1)){
		if(MEMC_Lib_get_memc_bypass_to_enable_flag() == 1){
			memc_switch_frm_cnt = frm_num;
			MEMC_Lib_set_memc_bypass_to_enable_flag(0);
#if IC_K5LP // merlin4_blend_mode
			WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x0); //SOFTWARE1_SOFTWARE1_00_reg[1]
			WriteRegister(MC_MC_28_reg, 14, 14, 0x1); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
			WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
			WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x0);//u1_mc2_var_lpf_wrt_en
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
			WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x0);// u1_logoCtrl_wrt_en;
			WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);
			if(VR360_en == 1)
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
			else
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
#elif (IC_K6LP || IC_K8LP)
			//set Repeat Mode
			Mid_Mode_SetMEMCFrameRepeatEnable(1);
#endif
			rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][AA][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
		}
		else if(memc_switch_frm_cnt == frm_num-1){
			if(VR360_en == 1){
#if IC_K5LP // merlin4_blend_mode
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
#elif (IC_K6LP || IC_K8LP)
				//set Repeat Mode
				Mid_Mode_SetMEMCFrameRepeatEnable(1);
				rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][Repeat mode][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
#endif
				rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][BB][%d,%d,%d,%d][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}else{
#if IC_K5LP // merlin4_blend_mode
				if((u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0) || (me_clk_state == 0)){
					WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x0); //SOFTWARE1_SOFTWARE1_00_reg[1]
					WriteRegister(MC_MC_28_reg, 14, 14, 0x1); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
					WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
					WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
					WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x0);//u1_mc2_var_lpf_wrt_en
					WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
					WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x0);// u1_logoCtrl_wrt_en;
					WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
					WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);
					WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
					rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][Repeat mode][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
				}else{
					WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x1); //SOFTWARE1_SOFTWARE1_00_reg[1]
					WriteRegister(MC_MC_28_reg, 14, 14, 0x0); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
					WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;
					WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x1f);
					WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x1);//u1_mc2_var_lpf_wrt_en
					WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
					WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x1);// u1_logoCtrl_wrt_en;
					WriteRegister(MC2_MC2_98_reg, 8, 8, 0x1);
					WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x1);
					WriteRegister(MC_MC_B0_reg, 0, 2, 0x0);
					rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][Blending mode][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
				}
#elif (IC_K6LP || IC_K8LP)
				if((u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0) || (me_clk_state == 0)){
					//set Repeat Mode
					Mid_Mode_SetMEMCFrameRepeatEnable(1);
					g_power_saving_flag = 1;
					rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][Repeat mode][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
				}else{
					//set Blending
					Mid_Mode_SetMEMCFrameRepeatEnable(0);
					g_power_saving_flag = 1;
					rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][Blending mode][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
				}
#endif
			}
		}
		else if(memc_switch_frm_cnt == frm_num-2){
			memc_switch_frm_cnt = 1;
		}

		if((memc_switch_frm_cnt > 2) && (memc_switch_frm_cnt <= frm_num)){			
#if IC_K5LP // merlin4_blend_mode
			WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
#elif (IC_K6LP || IC_K8LP)
#endif
			//rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_test_02][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][CC][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			memc_switch_frm_cnt--;
		}

		memc_bypass_state = 1;
	}
	else if(memc_switch_en == 1 && (g_memc_switch_state == 2)){
		if(DTV_PowerSaving_stage == 1){
			//set Repeat Mode
			Mid_Mode_SetMEMCFrameRepeatEnable(1);
			rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][ME_CLK_off][BB][,%x,%x,%x,%x,%x,%x,%x,%x,]\n", rtd_inl(0xb809aea8), rtd_inl(0xb80996b0)&0x7, rtd_inl(0xb8099750)&0x3, (rtd_inl(0xb8099798)>>8)&0x1, rtd_inl(MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg), rtd_inl(MC_DMA_MC_HF_P_DMA_RD_Ctrl_reg), (rtd_inl(0xb809c5c0)>>9)&0x1, (rtd_inl(0xb8099628)>>14)&0x1);
			DTV_PowerSaving_stage = 2;
		}else if(DTV_PowerSaving_stage == 2){
			//ME CLK off
			rtd_outl(SYS_REG_SYS_CLKEN3_reg, 0x40000000);//IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, 0x40000000);
			rtd_outl(SYS_REG_SYS_SRST3_reg, 0x40000000);//IoReg_Write32(SYS_REG_SYS_SRST3_reg, 0x40000000);
			//MC func CLK off
			rtd_outl(SYS_REG_SYS_CLKEN4_reg, 0x00000004);
			rtd_outl(SYS_REG_SYS_SRST4_reg, 0x00000004);

			rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][ME_CLK_off][CC][,%x,%x,%x,%x,%x,%x,%x,%x,]\n", rtd_inl(0xb809aea8), rtd_inl(0xb80996b0)&0x7, rtd_inl(0xb8099750)&0x3, (rtd_inl(0xb8099798)>>8)&0x1, rtd_inl(MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg), rtd_inl(MC_DMA_MC_HF_P_DMA_RD_Ctrl_reg), (rtd_inl(0xb809c5c0)>>9)&0x1, (rtd_inl(0xb8099628)>>14)&0x1);
			DTV_PowerSaving_stage = 3;
		}else if(DTV_PowerSaving_stage == 3){
			MEMC_Lib_set_memc_freeze_status_flag(0);
			rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][ME_CLK_off][DD][,%x,%x,%x,%x,%x,%x,%x,%x,]\n", rtd_inl(0xb809aea8), rtd_inl(0xb80996b0)&0x7, rtd_inl(0xb8099750)&0x3, (rtd_inl(0xb8099798)>>8)&0x1, rtd_inl(MC_DMA_MC_LF_P_DMA_RD_Ctrl_reg), rtd_inl(MC_DMA_MC_HF_P_DMA_RD_Ctrl_reg), (rtd_inl(0xb809c5c0)>>9)&0x1, (rtd_inl(0xb8099628)>>14)&0x1);
			DTV_PowerSaving_stage = 0;
		}
		memc_switch_frm_cnt = 1;
		MEMC_Lib_set_memc_enable_to_bypass_flag(0);
		rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][DD][%d,%d,%d,%d][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
	}
	else if(memc_switch_en == 0){
		if(memc_bypass_state == 1){
#if IC_K5LP // merlin4_blend_mode
			if((u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0) || (me_clk_state == 0)){
				WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x0); //SOFTWARE1_SOFTWARE1_00_reg[1]
				WriteRegister(MC_MC_28_reg, 14, 14, 0x1); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
				WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x0);//u1_mc2_var_lpf_wrt_en
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x0);// u1_logoCtrl_wrt_en;
				WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
				WriteRegister(SOFTWARE_SOFTWARE_00_reg, 0, 0, 0x1);
			}else{
				WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x1); //SOFTWARE1_SOFTWARE1_00_reg[1]
				WriteRegister(MC_MC_28_reg, 14, 14, 0x0); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
				WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x1f);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x1);//u1_mc2_var_lpf_wrt_en
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x1);// u1_logoCtrl_wrt_en;
				WriteRegister(MC2_MC2_98_reg, 8, 8, 0x1);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x1);
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x0);
				WriteRegister(SOFTWARE_SOFTWARE_00_reg, 0, 0, 0x0);
			}
#elif (IC_K6LP || IC_K8LP)
			if((u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0) || (me_clk_state == 0)){
				//set Repeat Mode
				Mid_Mode_SetMEMCFrameRepeatEnable(1);
				g_power_saving_flag = 1;
				rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][Repeat mode][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}else{
				//set Blending
				Mid_Mode_SetMEMCFrameRepeatEnable(0);
				g_power_saving_flag = 1;
				rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch][Blending mode][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}
#endif
			memc_switch_frm_cnt = 1;
			memc_bypass_state = 0;
		}
	}
#else
	unsigned int memc_switch_en;
	static unsigned int memc_switch_frm_cnt = 1;
	static unsigned int memc_bypass_state = 0;
	static unsigned int LockReadBuf_state = 0;
	unsigned int frm_num = 14;
	unsigned int test_val, test_val2, test_val3, test_val4;
	unsigned int out_phase;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	static unsigned char wptr = 0, rptr_p = 0, rptr_i = 0, freeze_cnt =0;

	ReadRegister(KPHASE_kphase_9C_reg, 0, 5, &out_phase);
	ReadRegister(SOFTWARE1_SOFTWARE1_63_reg, 30, 30, &memc_switch_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 24, 31, &frm_num);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 20, 23, &test_val);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 16, 19, &test_val2);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 8, 15, &test_val3);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 7, &test_val4);
	//read back wptr
	wptr = rtd_inl(KPHASE_kphase_90_reg)&0xf;

	if(memc_switch_en == 1){
		if(MEMC_Lib_get_memc_bypass_to_enable_flag() == 1){
			memc_switch_frm_cnt = frm_num;
			freeze_cnt = 0;	
			MEMC_Lib_set_memc_bypass_to_enable_flag(0);			
			//MEMC_LibEnableDMA();
			//memec_bypass_switch_flg = 0;
			WriteRegister(MC_MC_B0_reg, 0, 2, 2);
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
			WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);
			WriteRegister(SOFTWARE_SOFTWARE_00_reg, 0, 0, 0x1);

			if(VR360_en == 1){
				//b-size = 4
				rtd_clearbits(0xb809d410, _BIT13);
			}else{
				//b-size = 6
				rtd_setbits(0xb809d410, _BIT13);
			}
			rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_test_01][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
		}

		if(memc_switch_frm_cnt == (frm_num-test_val)){// 1
			rptr_p = wptr;
			rptr_i = wptr;
			LockReadBuf_state = 10;
			freeze_cnt = 0;			
			rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_test_08][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_test_08][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
		}

		if(VR360_en == 1){
			if((wptr == ((rptr_p + test_val3)%4)) && (LockReadBuf_state == 10)){
				//unlock R
				WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0);
				LockReadBuf_state = 100;
				freeze_cnt = 0;
				rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_test_05][%d,%d,%d,%d][%x][%x]    $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg),freeze_cnt);
				//rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_test_05][VR360][%d,%d,%d,%d][%x][%d]    $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg),freeze_cnt);
			}else if(LockReadBuf_state == 10){
				//lock R
				WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0x1);
				WriteRegister(KPHASE_kphase_00_reg, 4, 6, rptr_p);
				WriteRegister(KPHASE_kphase_00_reg, 0, 2, rptr_i);
				freeze_cnt ++;
				rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_test_04][%d,%d,%d,%d][%x][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg),freeze_cnt);
				//rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_test_04][VR360][%d,%d,%d,%d][%x][%d]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg),freeze_cnt);
			}
			//b-size = 4
//			rtd_clearbits(0xb809d410, _BIT13);
		}else{
			if((wptr == ((rptr_p + test_val2)%6)) && (LockReadBuf_state == 10)){
				//unlock R
				WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0);
				LockReadBuf_state = 100;
				freeze_cnt = 0;
				//rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_test_05][%d,%d,%d,%d][%x][%d]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg),freeze_cnt);
			}else if(LockReadBuf_state == 10){
				//lock R
				WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0x1);
				WriteRegister(KPHASE_kphase_00_reg, 4, 6, rptr_p);
				WriteRegister(KPHASE_kphase_00_reg, 0, 2, rptr_i);								
				freeze_cnt ++;	
				//rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_test_04][%d,%d,%d,%d][%x][%d]    $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg),freeze_cnt);
			}
			//b-size = 6
//			rtd_setbits(0xb809d410, _BIT13);
		}

		if(freeze_cnt == 2){
			MEMC_Lib_set_memc_freeze_status_flag(0);
			rtd_pr_memc_notice("$$$   [MEMC_Lib_set_memc_freeze_status_flag][%d]    $$$\n\r",freeze_cnt);
		}

		if(memc_switch_frm_cnt == 2){
			if(VR360_en == 1){
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x2);
				WriteRegister(SOFTWARE_SOFTWARE_00_reg, 0, 0, 0x1);
				memc_switch_frm_cnt = 1;
				rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_test_06_01][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
				//rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_test_06_01][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}else{
				if((u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0)){
					WriteRegister(MC_MC_B0_reg, 0, 2, 0x2);
				}else{
					WriteRegister(MC_MC_B0_reg, 0, 2, 0x0);
				}
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
				WriteRegister(MC2_MC2_98_reg, 8, 8, 0x1);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x1);
				WriteRegister(SOFTWARE_SOFTWARE_00_reg, 0, 0, 0x0);
				memc_switch_frm_cnt = 1;
				rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_test_06_02][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
				//rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_test_06_02][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}
		}

		if((memc_switch_frm_cnt > 2) && (memc_switch_frm_cnt <= frm_num)){
			WriteRegister(MC_MC_B0_reg, 0, 2, 0x2);
			rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_test_02][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			//rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_test_02][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			memc_switch_frm_cnt--;
		}

		if(memc_switch_frm_cnt >= 2)
			g_memc_switch_state = 1;
		else
			g_memc_switch_state = 0;

		memc_bypass_state = 1;
	}
	else{
		if(memc_bypass_state == 1){
			WriteRegister(KPHASE_kphase_04_reg, 11, 11, 0x0);
			WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0x0);
			WriteRegister(MC_MC_B0_reg, 0, 2, 0x0);
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
			WriteRegister(MC2_MC2_98_reg, 8, 8, 0x1);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x1);
			WriteRegister(SOFTWARE_SOFTWARE_00_reg, 0, 0, 0x0);
			memc_bypass_state = 0;
		}
	}
#endif

}

int g_memc_hdmi_switch_state = 0;
extern VOID MEMC_Lib_HDMI_set_memc_bypass_to_enable_flag(unsigned char u1_enable);
extern unsigned char MEMC_Lib_HDMI_get_memc_bypass_to_enable_flag(VOID);
VOID patch_MEMC_bypass_switch_HDMI(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
#if 1
	unsigned int memc_switch_en;
	static unsigned int memc_switch_frm_cnt = 1;
	static unsigned int memc_bypass_state = 0;
	unsigned int frm_num = 14;
	unsigned int out_phase;
	unsigned int cur_cadence = 0;
	unsigned int buffer_size = 0;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;

	cur_cadence = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[0];
	ReadRegister(KPHASE_kphase_9C_reg, 0, 5, &out_phase);
	ReadRegister(SOFTWARE1_SOFTWARE1_63_reg, 30, 30, &memc_switch_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 24, 27, &frm_num);
	ReadRegister(KPHASE_kphase_10_reg, 12, 15, &buffer_size);
	//read back wptr

#ifdef MEMC_BRING_UP
	return;
#endif

	if(memc_switch_en == 1){
		if(MEMC_Lib_HDMI_get_memc_bypass_to_enable_flag() == 1){
			memc_switch_frm_cnt = frm_num;
			MEMC_Lib_HDMI_set_memc_bypass_to_enable_flag(0);
#if IC_K5LP // merlin4_blend_mode
			WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x0); //SOFTWARE1_SOFTWARE1_00_reg[1]
			WriteRegister(MC_MC_28_reg, 14, 14, 0x1); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
			WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
			WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
			WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x0);//u1_mc2_var_lpf_wrt_en
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
			WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x0);// u1_logoCtrl_wrt_en;
			WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);
			WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
#elif (IC_K6LP || IC_K8LP)
			//set Repeat Mode
			Mid_Mode_SetMEMCFrameRepeatEnable(1);
#endif
			rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch_HDMI][AA][%d,%d,%d,%d][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
		}
		else if(memc_switch_frm_cnt == frm_num-1){
#if IC_K5LP // merlin4_blend_mode
			if(MEMC_LibGetMEMC_PCModeEnable()){
				WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x0); //SOFTWARE1_SOFTWARE1_00_reg[1]
				WriteRegister(MC_MC_28_reg, 14, 14, 0x1); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
				WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x0);//u1_mc2_var_lpf_wrt_en
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x0);// u1_logoCtrl_wrt_en;
				WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
			}else if((u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0)){
				WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x0); //SOFTWARE1_SOFTWARE1_00_reg[1]
				WriteRegister(MC_MC_28_reg, 14, 14, 0x1); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
				WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x0);//u1_mc2_var_lpf_wrt_en
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x0);// u1_logoCtrl_wrt_en;
				WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
			}else{
				WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x1); //SOFTWARE1_SOFTWARE1_00_reg[1]
				WriteRegister(MC_MC_28_reg, 14, 14, 0x0); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
				WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x1f);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x1);//u1_mc2_var_lpf_wrt_en
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x1);// u1_logoCtrl_wrt_en;
				WriteRegister(MC2_MC2_98_reg, 8, 8, 0x1);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x1);
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x0);
			}
#elif (IC_K6LP || IC_K8LP)
			if((MEMC_LibGetMEMC_PCModeEnable()) || (u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0)){
				//set Repeat Mode
				Mid_Mode_SetMEMCFrameRepeatEnable(1);
				g_power_saving_flag = 1;
				rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch_HDMI][Repeat mode][,%d,%d,%d,%x,][,%x,][,%d,%d,%d,]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg), MEMC_LibGetMEMC_PCModeEnable(), u8_MEMCMode, dejudder);
			}else{
				//set Blending
				Mid_Mode_SetMEMCFrameRepeatEnable(0);
				g_power_saving_flag = 1;
				rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch_HDMI][Blending mode][,%d,%d,%d,%x,][,%x,][,%d,%d,%d,]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg), MEMC_LibGetMEMC_PCModeEnable(), u8_MEMCMode, dejudder);
			}
#endif
		}
		else if(memc_switch_frm_cnt == frm_num-2){
			memc_switch_frm_cnt = 1;
		}

		if((memc_switch_frm_cnt > 2) && (memc_switch_frm_cnt <= frm_num)){
#if IC_K5LP // merlin4_blend_mode
			WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
#elif (IC_K6LP || IC_K8LP)
#endif
			rtd_pr_memc_debug("[PowerSaving_MEMC_bypass_switch_HDMI][BB][%d,%d,%d,%d][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			memc_switch_frm_cnt--;
		}

		if(memc_switch_frm_cnt >= 2)
			g_memc_hdmi_switch_state = 1;
		else
			g_memc_hdmi_switch_state = 0;

		memc_bypass_state = 1;
	}
	else{
		if(memc_bypass_state == 1){
#if IC_K5LP // merlin4_blend_mode
			if(MEMC_LibGetMEMC_PCModeEnable()){
				WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x0); //SOFTWARE1_SOFTWARE1_00_reg[1]
				WriteRegister(MC_MC_28_reg, 14, 14, 0x1); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
				WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x0);//u1_mc2_var_lpf_wrt_en
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x0);// u1_logoCtrl_wrt_en;
				WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
			}else if((u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0)){
				WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x0); //SOFTWARE1_SOFTWARE1_00_reg[1]
				WriteRegister(MC_MC_28_reg, 14, 14, 0x1); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
				WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x0); // u1_scCtrl_wrt_en;
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x0);//u1_mc2_var_lpf_wrt_en
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x0);// u1_logoCtrl_wrt_en;
				WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
			}else{
				WriteRegister(SOFTWARE1_SOFTWARE1_00_reg, 1, 1, 0x1); //SOFTWARE1_SOFTWARE1_00_reg[1]
				WriteRegister(MC_MC_28_reg, 14, 14, 0x0); //rtd_setbits(MC__MC_28_ADDR, _BIT14); //MC_MC_28_reg
				WriteRegister(HARDWARE_HARDWARE_57_reg, 14, 14, 0x1); // u1_scCtrl_wrt_en;
				WriteRegister(KME_ME1_TOP1_ME1_SCENE1_00_reg, 9, 13, 0x1f);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 17, 17, 0x1);//u1_mc2_var_lpf_wrt_en
				WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
				WriteRegister(HARDWARE_HARDWARE_57_reg, 29, 29, 0x1);// u1_logoCtrl_wrt_en;
				WriteRegister(MC2_MC2_98_reg, 8, 8, 0x1);
				WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x1);
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x0);
			}
#elif (IC_K6LP || IC_K8LP)
			if((MEMC_LibGetMEMC_PCModeEnable()) || (u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0)){
				//set Repeat Mode
				Mid_Mode_SetMEMCFrameRepeatEnable(1);
				g_power_saving_flag = 1;
				rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch_HDMI][Repeat mode][IZ][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}else{
				//set Blending
				Mid_Mode_SetMEMCFrameRepeatEnable(0);
				g_power_saving_flag = 1;
				rtd_pr_memc_notice("[PowerSaving_MEMC_bypass_switch_HDMI][Blending mode][%d,%d,%d,%x][%x]\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}
#endif
			memc_bypass_state = 0;
		}
	}
#else
	unsigned int memc_switch_en;
	static unsigned int memc_switch_frm_cnt = 1;
	static unsigned int memc_bypass_state = 0;
	static unsigned int LockReadBuf_state = 0;
	unsigned int frm_num = 14;
	unsigned int test_val, test_val2, test_val3, test_val4;
	unsigned int out_phase;
	unsigned int cur_cadence = 0;
	unsigned int buffer_size = 0;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	static unsigned char wptr = 0, rptr_p = 0, rptr_i = 0;

	cur_cadence = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[0];
	if(cur_cadence == 2){
		WriteRegister(SOFTWARE1_SOFTWARE1_60_reg, 20, 23, 0x4);
	}else{
		WriteRegister(SOFTWARE1_SOFTWARE1_60_reg, 20, 23, 0x3);
	}
	ReadRegister(KPHASE_kphase_9C_reg, 0, 5, &out_phase);
	ReadRegister(SOFTWARE1_SOFTWARE1_63_reg, 30, 30, &memc_switch_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 24, 31, &frm_num);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 20, 23, &test_val);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 16, 19, &test_val2);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 8, 15, &test_val3);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 0, 7, &test_val4);
	ReadRegister(KPHASE_kphase_10_reg, 12, 15, &buffer_size);
	//read back wptr
	wptr = rtd_inl(KPHASE_kphase_90_reg)&0xf;

	if(memc_switch_en == 1){
		if(MEMC_Lib_HDMI_get_memc_bypass_to_enable_flag() == 1){
			memc_switch_frm_cnt = frm_num;
			MEMC_Lib_HDMI_set_memc_bypass_to_enable_flag(0);
			if(MEMC_LibGetMEMC_PCModeEnable()){
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
			}else{
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x2);
			}
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x0);
			WriteRegister(MC2_MC2_98_reg, 8, 8, 0x0);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x0);

			rptr_p = wptr;
			rptr_i = wptr;
			LockReadBuf_state = 10;

			rtd_pr_memc_notice("$$$   [jerry_MEMC_bypass_switch_HDMI_01][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
		}

		if(memc_switch_frm_cnt == frm_num-test_val){// 1
			rtd_clearbits(PPOVERLAY_Double_Buffer_CTRL2_reg, _BIT18);
			rtd_setbits(PPOVERLAY_MEMC_MUX_CTRL_reg, _BIT13);
			rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_HDMI_08][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
		}

		if((buffer_size < 6)&&(buffer_size != 0)){
			if((wptr == ((rptr_p + test_val3)%buffer_size)) && (LockReadBuf_state == 10)){
				//unlock R
				WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0);
				LockReadBuf_state = 100;
				rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_HDMI_05_1][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}else if(LockReadBuf_state == 10){
				//lock R
				WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0x1);
				WriteRegister(KPHASE_kphase_00_reg, 4, 6, rptr_p);
				WriteRegister(KPHASE_kphase_00_reg, 0, 2, rptr_i);
				rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_HDMI_04_1][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}
		}else{
			if((wptr == ((rptr_p + test_val2)%6)) && (LockReadBuf_state == 10)){
				//unlock R
				WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0);
				LockReadBuf_state = 100;
				rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_HDMI_05_2][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}else if(LockReadBuf_state == 10){
				//lock R
				WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0x1);
				WriteRegister(KPHASE_kphase_00_reg, 4, 6, rptr_p);
				WriteRegister(KPHASE_kphase_00_reg, 0, 2, rptr_i);
				rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_HDMI_04_2][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			}
		}

		if(memc_switch_frm_cnt == 2){
			if(MEMC_LibGetMEMC_PCModeEnable()){
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
			}else if((u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0)){
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x2);
			}else{
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x0);
			}
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
			WriteRegister(MC2_MC2_98_reg, 8, 8, 0x1);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x1);
			memc_switch_frm_cnt = 1;
			rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_HDMI_06][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
		}

		if((memc_switch_frm_cnt > 2) && (memc_switch_frm_cnt <= frm_num)){
			if(MEMC_LibGetMEMC_PCModeEnable()){
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
			}else{
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x2);
			}
			rtd_pr_memc_debug("$$$   [jerry_MEMC_bypass_switch_HDMI_02][%d,%d,%d,%d][%x]   $$$\n\r", memc_switch_frm_cnt, in_fmRate, rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg), rtd_inl(KPHASE_kphase_90_reg));
			memc_switch_frm_cnt--;
		}

		if(memc_switch_frm_cnt >= 2)
			g_memc_switch_state = 1;
		else
			g_memc_switch_state = 0;

		memc_bypass_state = 1;
	}
	else{
		if(memc_bypass_state == 1){
			WriteRegister(KPHASE_kphase_04_reg, 11, 11, 0x0);
			WriteRegister(KPHASE_kphase_00_reg, 3, 3, 0x0);
			if(MEMC_LibGetMEMC_PCModeEnable()){
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x1);
			}else if((u8_MEMCMode == 0) || (u8_MEMCMode == 4 && dejudder == 0)){
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x2);
			}else{
				WriteRegister(MC_MC_B0_reg, 0, 2, 0x0);
			}
			WriteRegister(MC2_MC2_50_reg, 0, 1, 0x3);
			WriteRegister(MC2_MC2_98_reg, 8, 8, 0x1);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0x1);
			memc_bypass_state = 0;
		}
	}
#endif
}

////////////////////////////////////////////////////////////////////////
VOID PatchManage_Init(_OUTPUT_PATCH_M *pOutput)
{
        pOutput->u1_HF1_Detected = 0;
	pOutput->u6_HF1_hold_cnt = 0;
	pOutput->u1_HF2_Detected = 0;
	pOutput->u6_HF2_hold_cnt = 0;
	pOutput->u1_HF3_Detected = 0;
	pOutput->u6_HF3_hold_cnt = 0;
	pOutput->u1_RidingHorse_Detected = 0;
	pOutput->u6_RidingHorse_hold_cnt = 0;

	pOutput->u1_patchArm_preTrue       = 0;
	pOutput->u8_arm_hcnt               = 0;
	pOutput->u8_arm_detCnt             = 0;

	pOutput->u1_Print_En               = 0;
	pOutput->s32_PrintCnt              = 0;

	pOutput->u1_CoringThr_State        = 0;

	pOutput->u1_golfBall_HVdtl_FB_preTrue = 0;
	pOutput->u1_golfBall_HVdtl_FB_true    = 0;

	pOutput->u1_BallJump_true          = 0;
	pOutput->s8_BallJump_holdCnt       = 0;
	pOutput->u32_foreground_tc_pre[0]  = 0;
	pOutput->u32_foreground_tc_pre[1]  = 0;
	pOutput->u32_foreground_tc_pre[2]  = 0;
	pOutput->u32_foreground_tc_pre[3]  = 0;
	pOutput->u1_dh_close               = 0;

	pOutput->u1_sad_matched = 0;
	pOutput->u1_tc_matched = 0;	
	pOutput->u1_dtl_matched = 0;
	pOutput->u1_apl_matched = 0;
	pOutput->u1_BluceMic_Detected = 0;	
}

VOID Patch_CoringThr(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
#if 0/*need resolve:compiler error*/

   unsigned int u32GlbDtl;
   ReadRegister(FRC_TOP__KME_ME_TOP5__regr_metop_rdbk_dtl_all_ADDR no mat,FRC_TOP__KME_ME_TOP5__regr_metop_rdbk_dtl_all_BITSTART no mat,FRC_TOP__KME_ME_TOP5__regr_metop_rdbk_dtl_all_BITEND no mat,&u32GlbDtl);

   if(pOutput->u1_CoringThr_State == 1 && u32GlbDtl < 0x500000)
   	{
   	    pOutput->u1_CoringThr_State= 0;
   	}

    if(pOutput->u1_CoringThr_State == 0 && u32GlbDtl > 0x550000)
    {
	    pOutput->u1_CoringThr_State= 1;
    }


    if(pOutput->u1_CoringThr_State)
    {
        WriteRegister(FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y1_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y1_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y1_BITEND no mat,pParam->u8_patchCoring_sad1);
		//WriteRegister(FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y2_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y2_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y2_BITEND no mat,0x4);
    }else
    {
        WriteRegister(FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y1_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y1_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y1_BITEND no mat,pParam->u8_patchCoring_sad0);
		//WriteRegister(FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y2_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y2_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_sad_th_curve_y2_BITEND no mat,0x2);
    }
#endif
}



VOID Patch_detect(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
        patch_HF_Detect(pParam, pOutput);
	patch_RidingHorse_Detect(pParam, pOutput);

	// golf ball
	if(pParam->u1_patch_golfBall_en == 1)
	{
		patch_golfBall_detect(pParam, pOutput);
	}
	else
	{
		pOutput->u1_golfBall_HVdtl_FB_true = 0;
	}

	if(pParam->u1_patch_ballJump_en == 1)
	{
		patch_BallJump_detect(pParam, pOutput);
	}
	else
	{
		pOutput->s8_BallJump_holdCnt = 0;
	}

	if(pParam->u1_BluceMic_En == 1)
	{
		patch_BluceMic_detect(pParam, pOutput);
	}
	else
	{
		pOutput->u1_BluceMic_Detected = 0;
	}

	if(pParam->u1_patch_highFreq_en == 1)
	{
		patch_HighFreqPlayerOSD_detect(pParam, pOutput);
	}
	else
	{
		pOutput->u1_patch_highFreq_zmv_true = 0;
		pOutput->u1_patch_highFreq_rp_true = 0;
	}
	
	if(pParam->u1_tri_ball_Issue_en == 1)
	{
		patch_tri_ball_Issue_detect(pParam, pOutput);
	}
	else
	{
		pOutput->u1_tri_ball_Detected = 0;
	}	
}
VOID Patch_Action(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
        patch_HF_Action(pParam, pOutput);
	patch_RidingHorse_Action(pParam, pOutput);

	// const _PQLCONTEXT *s_pContext = GetPQLContext();

	patch_casino_action(pParam, pOutput);     // casino action

	patch_golfBall_action(pParam, pOutput);   // golf ball

	patch_BallJump_action(pParam, pOutput);   // hisense ball jump

	patch_BluceMic_action(pParam, pOutput);

	patch_Soccer_action(pParam, pOutput);     // soccer

	patch_Forbes_action(pParam, pOutput);

	patch_MEMC_bypass_switch(pParam, pOutput);

	patch_MEMC_bypass_switch_HDMI(pParam, pOutput);

	patch_tri_ball_Issue_action(pParam, pOutput);
	//// double scan action
	{
		pOutput->u1_dbScan_en = pParam->u1_dbScan_en;
	}
}

VOID PatchManage_Proc(const _PARAM_PATCH_M *pParam, _OUTPUT_PATCH_M *pOutput)
{
	if(pParam->u1_patch_M_en == 0)
		return;
		
	Patch_detect(pParam, pOutput);
	Patch_Action(pParam, pOutput);

	Patch_CoringThr(pParam, pOutput);

	if (pOutput->u1_Print_En == 1 && pOutput->s32_PrintCnt < 500)
	{
		pOutput->s32_PrintCnt++;
	}

	if (pOutput->s32_PrintCnt >= 500)
    {
		if (pParam->u1_Print_en && pOutput->u1_Print_En == 0)
		{
			pOutput->s32_PrintCnt = 0;
			pOutput->u1_Print_En = 1;
		}
	}
	pOutput->u1_Print_En  = 0;
	pOutput->s32_PrintCnt = 100000;
}
