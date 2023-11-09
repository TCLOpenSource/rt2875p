
#include <memc_isr/include/memc_lib.h>
#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_reg_def.h"
#include <rbus/ppoverlay_reg.h>
#include "memc_isr/MID/mid_mode.h"
#include <tvscalercontrol/panel/panelapi.h>

//dehalo me1 normal
#ifdef BUILD_QUICK_SHOW
#include "include/rbus/kme_dehalo5_reg.h"
#include "include/rbus/kme_me1_bg0_reg.h"
#include "include/rbus/kme_me1_bg1_reg.h"
#else
#if RTK_MEMC_Performance_tunging_from_tv001
#include "common/include/rbus/kme_dehalo5_reg.h"
#include "common/include/rbus/kme_me1_bg0_reg.h"
#include "common/include/rbus/kme_me1_bg1_reg.h"
#else
//dehalo me1 normal
#include "../../../common/include/rbus/kme_dehalo5_reg.h"
#include "../../../common/include/rbus/kme_me1_bg0_reg.h"
#include "../../../common/include/rbus/kme_me1_bg1_reg.h"
#endif
#endif

#include "memc_isr/PQL/MEMC_ParamTable.h"
#include "memc_isr/PQL/CalibMEMCInfo.h"
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
extern _MEMC_PARAM_TABLE MEMC_PARAM_TABLE;

unsigned int g_ME1_VBUF_GAIN = 1024;
#define g_ME1_VBUF_SHIFT (10)
unsigned int g_ME2_VBUF_GAIN = 1024;
#define g_ME2_VBUF_SHIFT (10)
unsigned int g_ME2_PFV_GAIN = 1024;
#define g_ME2_PFV_SHIFT (10)
unsigned int g_PLOGO_VBUF_GAIN = 1024;
#define g_PLOGO_VBUF_SHIFT (10)
unsigned int g_BLOGO_VBUF_GAIN = 1024;
#define g_BLOGO_VBUF_SHIFT (10)
unsigned int g_HEIGHT_GAIN = 1024;
#define g_HEIGHT_SHIFT (10)
unsigned int g_WIDTH_GAIN = 1024;
#define g_WIDTH_SHIFT (10)
unsigned int g_MOTION_REGION_GAIN = 2048;

#if RTK_MEMC_Performance_tunging_from_tv001
unsigned int g_MOTION_REGION_SHIFT = 20;
#else
#define g_MOTION_REGION_SHIFT (g_HEIGHT_SHIFT + g_WIDTH_SHIFT)
#endif

#define g_ME1_VBUF_GOLDEN	(32400)	 //240*135
#define g_ME2_VBUF_GOLDEN	(129600) //480*270
#define g_ME2_PFV_GOLDEN	(129600) //480*270
#define g_PLOGO_VBUF_GOLDEN	(518400) //960*540
#define g_BLOGO_VBUF_GOLDEN	(32400)  //240*135
#define g_HEIGHT_GOLDEN		(2160)
#define g_WIDTH_GOLDEN		(3840)

VOID ReadComReg_Init(_OUTPUT_ReadComReg *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_OUTPUT_RESOLUTION output_Resolution = s_pContext->_external_data._output_resolution;
	const _str_MEMC_buffer_size_table *tmp_MEMC_buffer_size_table = fwif_MEMC_get_MEMC_buffer_size_table((OUTPUT_RESOLUTION)output_Resolution);

#if RTK_MEMC_Performance_tunging_from_tv001
	unsigned int u14_Motion_Region_H_Gain = 0, u14_Motion_Region_V_Gain = 0;
#endif

	g_ME1_VBUF_GAIN = (((unsigned long long)g_ME1_VBUF_GOLDEN)<<g_ME1_VBUF_SHIFT) / (tmp_MEMC_buffer_size_table->ME1_BlockNum*tmp_MEMC_buffer_size_table->ME1_RowNum);
	g_ME2_VBUF_GAIN = (((unsigned long long)g_ME2_VBUF_GOLDEN)<<g_ME2_VBUF_SHIFT) / (tmp_MEMC_buffer_size_table->ME2_BlockNum*tmp_MEMC_buffer_size_table->ME2_RowNum);
	g_PLOGO_VBUF_GAIN = (((unsigned long long)g_PLOGO_VBUF_GOLDEN)<<g_PLOGO_VBUF_SHIFT) / (tmp_MEMC_buffer_size_table->PLogo_HSize*tmp_MEMC_buffer_size_table->PLogo_VSize);
	g_BLOGO_VBUF_GAIN = (((unsigned long long)g_BLOGO_VBUF_GOLDEN)<<g_BLOGO_VBUF_SHIFT) / (tmp_MEMC_buffer_size_table->BLogo_BlkNum*tmp_MEMC_buffer_size_table->BLogo_RowNum);
	g_ME2_PFV_GAIN = (((unsigned long long)g_ME2_PFV_GOLDEN)<<g_ME2_PFV_SHIFT) / (tmp_MEMC_buffer_size_table->ME2_PFV_BlockNum*tmp_MEMC_buffer_size_table->ME2_PFV_RowNum);
	g_HEIGHT_GAIN = (((unsigned long long)g_HEIGHT_GOLDEN)<<g_HEIGHT_SHIFT) / tmp_MEMC_buffer_size_table->MC_Vact;
	g_WIDTH_GAIN = (((unsigned long long)g_WIDTH_GOLDEN)<<g_WIDTH_SHIFT) / tmp_MEMC_buffer_size_table->MC_Hact;
	g_MOTION_REGION_GAIN = g_HEIGHT_GAIN*g_WIDTH_GAIN;

#if RTK_MEMC_Performance_tunging_from_tv001
	u14_Motion_Region_H_Gain = (u32IPME1_Golden_HSize<<(g_MOTION_REGION_SHIFT/2))/tmp_MEMC_buffer_size_table->IPME1_HSize;
	u14_Motion_Region_V_Gain = (u32IPME1_Golden_VSize<<(g_MOTION_REGION_SHIFT/2))/tmp_MEMC_buffer_size_table->IPME1_VSize;
	g_MOTION_REGION_GAIN = u14_Motion_Region_H_Gain*u14_Motion_Region_V_Gain;
	//g_MOTION_REGION_GAIN = 1;
	//g_MOTION_REGION_SHIFT = 0;

	rtd_pr_memc_info("[%s] %d, %d, %d, %d, %d, %d, %d, resolution,%d\n", __FUNCTION__, g_ME1_VBUF_GAIN, g_ME2_VBUF_GAIN, g_PLOGO_VBUF_GAIN, g_BLOGO_VBUF_GAIN, g_ME2_PFV_GAIN, g_HEIGHT_GAIN, g_WIDTH_GAIN, output_Resolution);
#endif

	rtd_pr_memc_notice("[ReadComReg_Init] g_ME1_VBUF_GAIN,%d, g_ME2_VBUF_GAIN,%d, g_ME2_PFV_GAIN,%d\n", g_ME1_VBUF_GAIN, g_ME2_VBUF_GAIN, g_ME2_PFV_GAIN);
	rtd_pr_memc_notice("[ReadComReg_Init] g_PLOGO_VBUF_GAIN,%d, g_BLOGO_VBUF_GAIN,%d\n", g_PLOGO_VBUF_GAIN, g_BLOGO_VBUF_GAIN);
	rtd_pr_memc_notice("[ReadComReg_Init] g_HEIGHT_GAIN,%d, g_WIDTH_GAIN,%d, g_MOTION_REGION_GAIN,%d\n", g_HEIGHT_GAIN, g_WIDTH_GAIN, g_MOTION_REGION_GAIN);
	rtd_pr_memc_notice("[ReadComReg_Init] ME1,%d,%d, ME2,%d,%d, PLogo,%d,%d, BLogo,%d,%d, ME2_PFV,%d,%d, IPME1,%d,%d\n",
		tmp_MEMC_buffer_size_table->ME1_BlockNum, tmp_MEMC_buffer_size_table->ME1_RowNum,
		tmp_MEMC_buffer_size_table->ME2_BlockNum, tmp_MEMC_buffer_size_table->ME2_RowNum,
		tmp_MEMC_buffer_size_table->PLogo_HSize,  tmp_MEMC_buffer_size_table->PLogo_VSize,
		tmp_MEMC_buffer_size_table->BLogo_BlkNum, tmp_MEMC_buffer_size_table->BLogo_RowNum,
		tmp_MEMC_buffer_size_table->ME2_PFV_BlockNum,tmp_MEMC_buffer_size_table->ME2_PFV_RowNum,
		tmp_MEMC_buffer_size_table->IPME1_HSize,  tmp_MEMC_buffer_size_table->IPME1_VSize);
}

//Input signal and parameters are connected locally

VOID ReadComReg_Proc_oneFifth_OutInterrupt(const _PARAM_ReadComReg *pParam, _OUTPUT_ReadComReg *pOutput)
{
	unsigned int u32_rb_val;

	if (pParam->u1_me_VbufSize_rb_en == 1)
	{
		ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_00_reg, 0, 31, &u32_rb_val);
		pOutput->u8_me1_Vbuf_Hsize_rb =  u32_rb_val       & 0x1FF;
		pOutput->u8_me1_Vbuf_Vsize_rb = (u32_rb_val >> 9) & 0x1FF;
	}

	if (pParam->u1_BBD_rb_en == 1)
	{
		unsigned char u8_k = 0;
		for (u8_k = 0; u8_k < _RIM_NUM; u8_k ++)
		{
			ReadRegister(BBD_BBD_OUTPUT_TOP_ACTIVE_LINE_reg + u8_k * 4, 0, 31, &u32_rb_val);
			pOutput->u12_BBD_roughRim_rb[u8_k]  =  u32_rb_val        & 0x1fff;
			pOutput->u1_BBD_roughValid_rb[u8_k] = (u32_rb_val >> 13) & 0x01;
			pOutput->u12_BBD_fineRim_rb[u8_k]   = (u32_rb_val >> 16) & 0x1fff;
			pOutput->u1_BBD_fineValid_rb[u8_k]  = (u32_rb_val >> 29) & 0x01;
		}

        ReadRegister(KME_DM_TOP2_MV05_RESOLUTION_reg, 0, 31, &u32_rb_val);
		pOutput->u12_ME1_blk_size[0] =   u32_rb_val        & 0xfff;
		pOutput->u12_ME1_blk_size[1] =  (u32_rb_val >> 12) & 0xfff;

		ReadRegister(KME_DM_TOP0_KME_ME_RESOLUTION_reg, 0, 31, &u32_rb_val);
		pOutput->u12_ME1_pix_size[0] =   u32_rb_val        & 0xfff;
		pOutput->u12_ME1_pix_size[1] =  (u32_rb_val >> 12) & 0xfff;

		ReadRegister(KME_DM_TOP2_MV01_RESOLUTION_reg, 0, 31, &u32_rb_val);
		pOutput->u12_ME2_blk_size[0] =   u32_rb_val        & 0xfff;
		pOutput->u12_ME2_blk_size[1] =  (u32_rb_val >> 12) & 0xfff;

//		ReadRegister(FRC_TOP__KME_DM_TOP2__mv12_hactive_ADDR, 0, 31, &u32_rb_val);
		pOutput->u12_ME2_pix_size[0] =   u32_rb_val        & 0xfff;
		pOutput->u12_ME2_pix_size[1] =  (u32_rb_val >> 12) & 0xfff;

	}
}

VOID ReadComReg_Proc_inputInterrupt_Hsync(const _PARAM_ReadComReg *pParam, _OUTPUT_ReadComReg *pOutput)
{
	unsigned int u32_RB_val;

	#if 1
	unsigned int log_en = 0;
	unsigned int me_write = 0, me1_read_i = 0, me1_read_p = 0, me2_read_i = 0, me2_read_p = 0;
	unsigned int mc_write = 0, mc_read_i = 0, mc_read_p = 0;
	unsigned int me2_phase = 0, mc_phase = 0;
	unsigned int u32_post_out_vtotal_int,Uzudtg_line_count_read_out,Memcdtg_line_count_read_out;
	static unsigned int u32_ID_Log_en=0;

	ReadRegister(KPHASE_kphase_8C_reg, 0, 2, &me_write);
	ReadRegister(KPHASE_kphase_8C_reg, 4, 6, &me1_read_i);
	ReadRegister(KPHASE_kphase_8C_reg, 8, 10, &me1_read_p);
	ReadRegister(KPHASE_kphase_8C_reg, 20, 22, &me2_read_i);
	ReadRegister(KPHASE_kphase_8C_reg, 24, 26, &me2_read_p);
	ReadRegister(KPHASE_kphase_90_reg, 0, 2, &mc_write);
	ReadRegister(KPHASE_kphase_90_reg, 4, 6, &mc_read_i);
	ReadRegister(KPHASE_kphase_90_reg, 8, 10, &mc_read_p);
	ReadRegister(MC_MC_F8_reg, 0, 6, &me2_phase);
	ReadRegister(MC_MC_F8_reg, 0, 6, &mc_phase);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_26_reg, 16,  16, &u32_ID_Log_en);//YE Test debug dummy register //ORI

	ReadRegister(KPOST_TOP_KPOST_TOP_04_reg,20,31,&u32_post_out_vtotal_int);
	ReadRegister(PPOVERLAY_new_meas1_linecnt_real_reg,16,28,&Uzudtg_line_count_read_out);
	ReadRegister(PPOVERLAY_new_meas1_linecnt_real_reg,0,12,&Memcdtg_line_count_read_out);
	
	#endif

	//////////////////////////////////////////////////////////////////////////
	// for LBMC mode switch.
	//////////////////////////////////////////////////////////////////////////
	if (pParam->u1_me_pnMV_cnt_rb_en == 1)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_48_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_posCnt_0_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_4C_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_posCnt_1_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_50_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_posCnt_2_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_54_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_posCnt_3_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);

		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_58_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_negCnt_0_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_5C_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_negCnt_1_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_60_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_negCnt_2_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_64_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_negCnt_3_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
	}

	if(u32_ID_Log_en==1) //part two
	{
        //if(u32_RB_val_smv_post>25000)
		rtd_pr_memc_notice("[MEMC_Mode_LF_Z][%s],ZZZZZ ,post_out_vtotal_int=%d,Uzudtg_line_count_read_out=%d,Memcdtg_line_count_read_out=%d\n", __FUNCTION__,
		u32_post_out_vtotal_int,Uzudtg_line_count_read_out,Memcdtg_line_count_read_out);
		rtd_pr_memc_notice("[MEMC_Mode_LF_Z][%s],ZZZZZ ,rb_smv_posCnt=%d,rb_smv_negCnt=%d,rb_Lmv_posCnt=%d,rb_Lmv_negCnt =%d,(,%d,%d,%d,%d,) \n", __FUNCTION__,
		pOutput->u17_me_posCnt_0_rb,pOutput->u17_me_negCnt_0_rb ,pOutput->u17_me_posCnt_1_rb,pOutput->u17_me_negCnt_1_rb,
		pOutput->u17_me_posCnt_2_rb,pOutput->u17_me_negCnt_2_rb ,pOutput->u17_me_posCnt_3_rb,pOutput->u17_me_negCnt_3_rb);			
	}

	if(log_en){
		rtd_pr_memc_notice("[phasetable][%s][ME1_idx,%d,%d,%d,ME2_idx,%d,%d,][MC_idx,%d,%d,%d,][phase,%d,%d,][Time,%d,]\n\r", __FUNCTION__,
			me_write, me1_read_i, me1_read_p, me2_read_i, me2_read_p,
			mc_write, mc_read_i, mc_read_p, me2_phase, mc_phase,
			rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
		#if 0
		for(u8_i = 0; u8_i < _CAD_CHANGE_QUE_BUF_FRM; u8_i++){
			rtd_pr_memc_notice("[%s][g_cadence_change_que][%d][u8_i,%d,][mc_wrt_index,%d,][state,%d,][type,%d,]\n\r",
				__FUNCTION__, __LINE__, u8_i, g_cadence_change_que->mc_wrt_index[u8_i], g_cadence_change_que->state[u8_i], g_cadence_change_que->type[u8_i]);
		}
		#endif
	}


}

VOID ReadComReg_Proc_inputInterrupt(const _PARAM_ReadComReg *pParam, _OUTPUT_ReadComReg *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_OUTPUT_RESOLUTION output_Resolution = s_pContext->_external_data._output_resolution;
	const _str_MEMC_buffer_size_table *tmp_MEMC_buffer_size_table = fwif_MEMC_get_MEMC_buffer_size_table((OUTPUT_RESOLUTION)output_Resolution);
	unsigned int u32_RB_val;
	_PQLPARAMETER *s_pParam       = GetPQLParameter();

	unsigned int u27_ipme_h0Mot_rb = 0;
	unsigned int u27_ipme_h1Mot_rb = 0;
	unsigned int u27_ipme_v0Mot_rb = 0;
	unsigned int u27_ipme_v1Mot_rb = 0;
	static short pre_maxgmv_mvx = 0;
	static short pre_maxgmv_mvy = 0;
	short diff_maxgmv_mvx = 0;
	short diff_maxgmv_mvy = 0;

	#if 0
	unsigned int u32_post_out_vtotal_int,Uzudtg_line_count_read_out,Memcdtg_line_count_read_out;
	static unsigned int u32_ID_Log_en=0;
	
	
	ReadRegister(SOFTWARE1_SOFTWARE1_26_reg, 16,  16, &u32_ID_Log_en);//YE Test debug dummy register

	ReadRegister(KPOST_TOP_KPOST_TOP_04_reg,20,31,&u32_post_out_vtotal_int);
	ReadRegister(PPOVERLAY_new_meas1_linecnt_real_reg,16,28,&Uzudtg_line_count_read_out);
	ReadRegister(PPOVERLAY_new_meas1_linecnt_real_reg,0,12,&Memcdtg_line_count_read_out);
	
	

		if(u32_ID_Log_en==1) //part two
			{
                                //if(u32_RB_val_smv_post>25000)
							rtd_pr_memc_debug("[MEMC_Mode_LF_T],TTTTT ,u32_post_out_vtotal_int=%d,Uzudtg_line_count_read_out=%d,Memcdtg_line_count_read_out=%d, \n",
					u32_post_out_vtotal_int,Uzudtg_line_count_read_out,Memcdtg_line_count_read_out);

			}
	#endif
      ///////////////////////////me max min mv information//////////
		{
		signed short s11_me_mv_max_vx_rb, s10_me_mv_max_vy_rb, s11_me_mv_min_vx_rb, s10_me_mv_min_vy_rb;
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN21_FG_reg ,0,31,&u32_RB_val);    //fg rgn30 and fg rgn31  
        s11_me_mv_max_vx_rb = ((u32_RB_val >> 10) &1) == 0? ((u32_RB_val>>0)& 0x7ff):(((u32_RB_val>>0)& 0x7ff)  - (1<<11)); 
        s10_me_mv_max_vy_rb =((u32_RB_val >> 20) &1) == 0? ((u32_RB_val >> 11) & 0x3ff) : (((u32_RB_val >> 11) & 0x3ff)  - (1<<10));
		pOutput->s11_me_mv_max_vx_rb = Calib_MV(s11_me_mv_max_vx_rb);
		pOutput->s10_me_mv_max_vy_rb = Calib_MV(s10_me_mv_max_vy_rb);
	
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_MIN_MV_reg ,0,31,&u32_RB_val);    //fg rgn30 and fg rgn31  
       	s11_me_mv_min_vx_rb = ((u32_RB_val >> 10) &1) == 0? ((u32_RB_val>>0)& 0x7ff):(((u32_RB_val>>0)& 0x7ff)  - (1<<11)); 
        s10_me_mv_min_vy_rb =((u32_RB_val >> 20) &1) == 0? ((u32_RB_val >> 11) & 0x3ff) : (((u32_RB_val >> 11) & 0x3ff)  - (1<<10));
		pOutput->s11_me_mv_min_vx_rb = Calib_MV(s11_me_mv_min_vx_rb);
		pOutput->s10_me_mv_min_vy_rb = Calib_MV(s10_me_mv_min_vy_rb);
      	}


	//////////////////////////////////////////////////////////////////////////
	// gmv all information
	//////////////////////////////////////////////////////////////////////////
	if (pParam->u1_me_GMV_1st_rb_en == 1)
	{
		ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_B0_reg, 0, 31, &u32_RB_val);
		pOutput->s11_me_GMV_1st_vx_rb = ((u32_RB_val >> 10) &1) == 0? (u32_RB_val         & 0x7ff) : ((u32_RB_val         & 0x7ff)  - (1<<11));
		pOutput->s10_me_GMV_1st_vy_rb = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val >> 11) & 0x3ff) : (((u32_RB_val >> 11) & 0x3ff)  - (1<<10));
		pOutput->s11_me_GMV_1st_vx_rb = Calib_MV(pOutput->s11_me_GMV_1st_vx_rb);
		pOutput->s10_me_GMV_1st_vy_rb = Calib_MV(pOutput->s10_me_GMV_1st_vy_rb);

		ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_C0_reg, 0, 16, &u32_RB_val);
		//pOutput->u17_me_GMV_1st_cnt_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		pOutput->u17_me_GMV_1st_cnt_rb = ReadComReg_Calib_Data(u32_RB_val, CALIB_PARAM_GMV_CNT, 0, 32400, false);

		ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_BC_reg, 0, 11, &u32_RB_val);
		pOutput->u12_me_GMV_1st_unconf_rb = u32_RB_val;

		diff_maxgmv_mvx = _ABS_DIFF_(pre_maxgmv_mvx , pOutput->s11_me_GMV_1st_vx_rb);
		diff_maxgmv_mvy = _ABS_DIFF_(pre_maxgmv_mvy , pOutput->s10_me_GMV_1st_vy_rb);

		pOutput->s11_gmv_diff_maxgmv_mvx = diff_maxgmv_mvx;
		pOutput->s11_gmv_diff_maxgmv_mvy = diff_maxgmv_mvy;
		
		pre_maxgmv_mvx = pOutput->s11_me_GMV_1st_vx_rb;
		pre_maxgmv_mvy = pOutput->s10_me_GMV_1st_vy_rb;
	}

	if (pParam->u1_me_GMV_2nd_rb_en == 1)
	{
		ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_B4_reg, 0, 31, &u32_RB_val);
		pOutput->s11_me_GMV_2nd_vx_rb = ((u32_RB_val >> 10) &1) == 0? (u32_RB_val         & 0x7ff) : ((u32_RB_val         & 0x7ff) - (1<<11));
		pOutput->s10_me_GMV_2nd_vy_rb = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val >> 11) & 0x3ff) : (((u32_RB_val >> 11) & 0x3ff) - (1<<10));
		pOutput->s11_me_GMV_2nd_vx_rb = Calib_MV(pOutput->s11_me_GMV_2nd_vx_rb);
		pOutput->s10_me_GMV_2nd_vy_rb = Calib_MV(pOutput->s10_me_GMV_2nd_vy_rb);
		ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_C4_reg, 0, 16, &u32_RB_val);
		pOutput->u17_me_GMV_2nd_cnt_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_BC_reg, 12, 23, &u32_RB_val);
		pOutput->u12_me_GMV_2nd_unconf_rb = u32_RB_val;
	}

	//////////////////////////////////////////////////////////////////////////
	// panning cnt
	//////////////////////////////////////////////////////////////////////////
	if(pParam->u1_me_Hpan_cnt_rb_en)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_70_reg, 0, 19, &u32_RB_val);
		pOutput->u20_me_Hpan_cnt_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
	}

	if(pParam->u1_me_Vpan_cnt_rb_en)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_74_reg, 0, 19, &u32_RB_val);
		pOutput->u20_me_Vpan_cnt_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
	}

	#if 1 //YE Test move it to Hsync
	{
		kme_me1_top10_status_sel_RBUS kme_me1_top10_status_sel_reg;
		UINT8 u8_scan_sel = 0;		
	#if 1
		unsigned int log_en = 0;
		unsigned int me_write = 0, me1_read_i = 0, me1_read_p = 0, me2_read_i = 0, me2_read_p = 0;
		unsigned int mc_write = 0, mc_read_i = 0, mc_read_p = 0;
		unsigned int me2_phase = 0, mc_phase = 0;
		unsigned int u32_post_out_vtotal_int,Uzudtg_line_count_read_out,Memcdtg_line_count_read_out;
		static unsigned int u32_ID_Log_en=0;
		
		ReadRegister(KPHASE_kphase_8C_reg, 0, 2, &me_write);
		ReadRegister(KPHASE_kphase_8C_reg, 4, 6, &me1_read_i);
		ReadRegister(KPHASE_kphase_8C_reg, 8, 10, &me1_read_p);
		ReadRegister(KPHASE_kphase_8C_reg, 20, 22, &me2_read_i);
		ReadRegister(KPHASE_kphase_8C_reg, 24, 26, &me2_read_p);
		ReadRegister(KPHASE_kphase_90_reg, 0, 2, &mc_write);
		ReadRegister(KPHASE_kphase_90_reg, 4, 6, &mc_read_i);
		ReadRegister(KPHASE_kphase_90_reg, 8, 10, &mc_read_p);
		ReadRegister(MC_MC_F8_reg, 0, 6, &me2_phase);
		ReadRegister(MC_MC_F8_reg, 0, 6, &mc_phase);
		ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
		ReadRegister(SOFTWARE1_SOFTWARE1_26_reg, 16,  16, &u32_ID_Log_en);//YE Test debug dummy register //ORI
		
		ReadRegister(KPOST_TOP_KPOST_TOP_04_reg,20,31,&u32_post_out_vtotal_int);
		ReadRegister(PPOVERLAY_new_meas1_linecnt_real_reg,16,28,&Uzudtg_line_count_read_out);
		ReadRegister(PPOVERLAY_new_meas1_linecnt_real_reg,0,12,&Memcdtg_line_count_read_out);
		
	#endif
		
		kme_me1_top10_status_sel_reg.regValue = rtd_inl(KME_ME1_TOP10_status_sel_reg);
		u8_scan_sel = (kme_me1_top10_status_sel_reg.status_scan_sel<5)?kme_me1_top10_status_sel_reg.status_scan_sel:0;

		
	//////////////////////////////////////////////////////////////////////////
	// for LBMC mode switch.
	//////////////////////////////////////////////////////////////////////////
	
	if ((pParam->u1_me_pnMV_cnt_rb_en == 1) && (u8_scan_sel!=0))
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_48_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_posCnt_0_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_4C_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_posCnt_1_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_50_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_posCnt_2_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_54_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_posCnt_3_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);

		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_58_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_negCnt_0_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_5C_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_negCnt_1_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_60_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_negCnt_2_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_64_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_negCnt_3_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
	

	if(u32_ID_Log_en==1) //part two
	{
                    //if(u32_RB_val_smv_post>25000)
		rtd_pr_memc_notice("[MEMC_Mode_LF_Z][%s],ZZZZZ ,post_out_vtotal_int=%d,Uzudtg_line_count_read_out=%d,Memcdtg_line_count_read_out=%d\n", __FUNCTION__,
		u32_post_out_vtotal_int,Uzudtg_line_count_read_out,Memcdtg_line_count_read_out);
		rtd_pr_memc_notice("[MEMC_Mode_LF_Z][%s],ZZZZZ ,rb_smv_posCnt=%d,rb_smv_negCnt=%d,rb_Lmv_posCnt=%d,rb_Lmv_negCnt =%d,(,%d,%d,%d,%d,) \n", __FUNCTION__,
		pOutput->u17_me_posCnt_0_rb,pOutput->u17_me_negCnt_0_rb ,pOutput->u17_me_posCnt_1_rb,pOutput->u17_me_negCnt_1_rb,
		pOutput->u17_me_posCnt_2_rb,pOutput->u17_me_negCnt_2_rb ,pOutput->u17_me_posCnt_3_rb,pOutput->u17_me_negCnt_3_rb);	
	}

	if(log_en){
		rtd_pr_memc_notice("[phasetable][%s][ME1_idx,%d,%d,%d,ME2_idx,%d,%d,][MC_idx,%d,%d,%d,][phase,%d,%d,][Time,%d,]\n\r", __FUNCTION__,
			me_write, me1_read_i, me1_read_p, me2_read_i, me2_read_p,
			mc_write, mc_read_i, mc_read_p, me2_phase, mc_phase,
			rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	}

	}
	
	}
	#endif

	//////////////////////////////////////////////////////////////////////////
	// ipme, film detect information
	//////////////////////////////////////////////////////////////////////////
	{
		ReadRegister(KME_IPME_KME_IPME_A4_reg, 0, 26, &u32_RB_val);
		u27_ipme_h0Mot_rb = Calib_Data(u32_RB_val, g_HEIGHT_GAIN, g_HEIGHT_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_AC_reg, 0, 26, &u32_RB_val);
		u27_ipme_h1Mot_rb = Calib_Data(u32_RB_val, g_HEIGHT_GAIN, g_HEIGHT_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_F8_reg, 0, 26, &u32_RB_val);
		u27_ipme_v0Mot_rb = Calib_Data(u32_RB_val, g_WIDTH_GAIN, g_WIDTH_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_FC_reg, 0, 26, &u32_RB_val);
		u27_ipme_v1Mot_rb = Calib_Data(u32_RB_val, g_WIDTH_GAIN, g_WIDTH_SHIFT);
	}

	if (pParam->u1_ipme_aMot_rb_en == 1)
	{
		unsigned int u27_ipme_rimMot_rb = 0;
		ReadRegister(KME_IPME_KME_IPME_A8_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_aMot_rb    =  Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);

		//minus rim line motion from bad-DI, use 12rgn motion to avoid bad-DI affect, but keep this idea here
		u27_ipme_rimMot_rb = (u27_ipme_h0Mot_rb + u27_ipme_h1Mot_rb + u27_ipme_v0Mot_rb + u27_ipme_v1Mot_rb);
		u27_ipme_rimMot_rb = 0;
		pOutput->u27_ipme_aMot_rb = (pOutput->u27_ipme_aMot_rb < u27_ipme_rimMot_rb) ? 0 : (pOutput->u27_ipme_aMot_rb - u27_ipme_rimMot_rb);
	}
	if(pParam->u1_ipme_5rMot_rb_en == 1)
	{
		unsigned short u16_top_v0 = 0;
		unsigned short u16_top_v1 = 0;
		unsigned short u16_bot_v0 = 0;
		unsigned short u16_bot_v1 = 0;
		unsigned short u16_lft_h0 = 0;
		unsigned short u16_lft_h1 = 0;
		unsigned short u16_rgt_h0 = 0;
		unsigned short u16_rgt_h1 = 0;
		unsigned short u16_ipme_res_x = (tmp_MEMC_buffer_size_table->IPME1_HSize)>>1;
		unsigned short u16_ipme_res_y = tmp_MEMC_buffer_size_table->IPME1_VSize;
		unsigned short u16_LR_rimMot_t = 0;
		unsigned short u16_LR_rimMot_b = 0;
		unsigned short u16_TB_rimMot_l = 0;
		unsigned short u16_TB_rimMot_r = 0;

		ReadRegister((KME_IPME_KME_IPME_40_reg), 0, 31, &u32_RB_val);//top
		u16_top_v0 = (u32_RB_val) & 0x3ff;
		u16_top_v1 = (u32_RB_val>>10) & 0x3ff;
		ReadRegister((KME_IPME_KME_IPME_44_reg), 0, 31, &u32_RB_val);//bottom
		u16_bot_v0 = (u32_RB_val) & 0x3ff;
		u16_bot_v1 = (u32_RB_val>>10) & 0x3ff;
		ReadRegister((KME_IPME_KME_IPME_48_reg), 0, 31, &u32_RB_val);//left
		u16_lft_h0 = (u32_RB_val) & 0x3ff;
		u16_lft_h1 = (u32_RB_val>>10) & 0x3ff;
		ReadRegister((KME_IPME_KME_IPME_4C_reg), 0, 31, &u32_RB_val);//bottom
		u16_rgt_h0 = (u32_RB_val) & 0x3ff;
		u16_rgt_h1 = (u32_RB_val>>10) & 0x3ff;

		ReadRegister(KME_IPME_KME_IPME_B0_reg, 0, 26, &u32_RB_val);//top
		pOutput->u27_ipme_5Mot_rb[0] =  u32_RB_val;
		u16_LR_rimMot_t = (u27_ipme_h0Mot_rb + u27_ipme_h1Mot_rb)*((u16_top_v1<u16_top_v0)?0:(u16_top_v1-u16_top_v0))/u16_ipme_res_y;
		#if !MIX_MODE_REGION_17
		u16_LR_rimMot_t = 0;
		#endif
		pOutput->u27_ipme_5Mot_rb[0]= (pOutput->u27_ipme_5Mot_rb[0] < u16_LR_rimMot_t) ? 0 : (pOutput->u27_ipme_5Mot_rb[0] - u16_LR_rimMot_t);
		pOutput->u27_ipme_5Mot_rb[0] = Calib_Data(pOutput->u27_ipme_5Mot_rb[0], g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);

		ReadRegister(KME_IPME_KME_IPME_B4_reg, 0, 26, &u32_RB_val);//bot
		pOutput->u27_ipme_5Mot_rb[1] =  u32_RB_val;
		u16_LR_rimMot_b = (u27_ipme_h0Mot_rb + u27_ipme_h1Mot_rb)*((u16_bot_v1<u16_bot_v0)?0:(u16_bot_v1- u16_bot_v0))/u16_ipme_res_y;
		#if !MIX_MODE_REGION_17
		u16_LR_rimMot_b = 0;
		#endif
		pOutput->u27_ipme_5Mot_rb[1] = (pOutput->u27_ipme_5Mot_rb[1] < u16_LR_rimMot_b) ? 0 : (pOutput->u27_ipme_5Mot_rb[1] - u16_LR_rimMot_b);
		pOutput->u27_ipme_5Mot_rb[1] = Calib_Data(pOutput->u27_ipme_5Mot_rb[1], g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);

		ReadRegister(KME_IPME_KME_IPME_B8_reg, 0, 26, &u32_RB_val);//left
		pOutput->u27_ipme_5Mot_rb[2] =  u32_RB_val;
		u16_TB_rimMot_l = (u27_ipme_v0Mot_rb + u27_ipme_v1Mot_rb)*((u16_lft_h1< u16_lft_h0)?0:(u16_lft_h1- u16_lft_h0))/u16_ipme_res_x;
		#if !MIX_MODE_REGION_17
		u16_TB_rimMot_l = 0;
		#endif
		pOutput->u27_ipme_5Mot_rb[2] = (pOutput->u27_ipme_5Mot_rb[2] < u16_TB_rimMot_l) ? 0 : (pOutput->u27_ipme_5Mot_rb[2] - u16_TB_rimMot_l);
		pOutput->u27_ipme_5Mot_rb[2] = Calib_Data(pOutput->u27_ipme_5Mot_rb[2], g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);

		ReadRegister(KME_IPME_KME_IPME_BC_reg, 0, 26, &u32_RB_val);//right
		pOutput->u27_ipme_5Mot_rb[3] =  u32_RB_val;
		u16_TB_rimMot_r = (u27_ipme_v0Mot_rb + u27_ipme_v1Mot_rb)*((u16_rgt_h1< u16_rgt_h0)?0:(u16_rgt_h1- u16_rgt_h0))/u16_ipme_res_x;
		#if !MIX_MODE_REGION_17
		u16_TB_rimMot_r = 0;
		#endif
		pOutput->u27_ipme_5Mot_rb[3] = (pOutput->u27_ipme_5Mot_rb[3] < u16_TB_rimMot_r) ? 0 : (pOutput->u27_ipme_5Mot_rb[3] - u16_TB_rimMot_r);
		pOutput->u27_ipme_5Mot_rb[3] = Calib_Data(pOutput->u27_ipme_5Mot_rb[3], g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);

		ReadRegister(KME_IPME_KME_IPME_C0_reg, 0, 26, &u32_RB_val);//cen
		pOutput->u27_ipme_5Mot_rb[4] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
	}
	if(pParam->u1_ipme_12rMot_rb_en == 1)
	{
		ReadRegister(KME_IPME_KME_IPME_C4_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[0] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_C8_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[1] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_CC_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[2] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_D0_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[3] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_D4_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[4] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);

		ReadRegister(KME_IPME_KME_IPME_D8_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[5] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_DC_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[6] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_E0_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[7] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_E4_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[8] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_E8_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[9] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);

		ReadRegister(KME_IPME_KME_IPME_EC_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[10] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
		ReadRegister(KME_IPME_KME_IPME_F0_reg, 0, 26, &u32_RB_val);
		pOutput->u27_ipme_12Mot_rb[11] = Calib_Data(u32_RB_val, g_MOTION_REGION_GAIN, g_MOTION_REGION_SHIFT);
	}
	#if !MIX_MODE_REGION_17
	{
		//use 12 rgn motion for mixmode to avoid outside of rim
		//top
		unsigned int u27_12rgn_top = 	pOutput->u27_ipme_12Mot_rb[0] +
								pOutput->u27_ipme_12Mot_rb[1] +
								pOutput->u27_ipme_12Mot_rb[2] +
								pOutput->u27_ipme_12Mot_rb[3];
		//bot
		unsigned int u27_12rgn_bot = 	pOutput->u27_ipme_12Mot_rb[8] +
								pOutput->u27_ipme_12Mot_rb[9] +
								pOutput->u27_ipme_12Mot_rb[10] +
								pOutput->u27_ipme_12Mot_rb[11];
		//lft
		unsigned int u27_12rgn_lft = 	pOutput->u27_ipme_12Mot_rb[0] +
								pOutput->u27_ipme_12Mot_rb[4] +
								pOutput->u27_ipme_12Mot_rb[8];
		//rgt
		unsigned int u27_12rgn_rgt = 	pOutput->u27_ipme_12Mot_rb[3] +
								pOutput->u27_ipme_12Mot_rb[7] +
								pOutput->u27_ipme_12Mot_rb[11];

		pOutput->u27_ipme_5Mot_rb[0]= u27_12rgn_top;
		pOutput->u27_ipme_5Mot_rb[1]= u27_12rgn_bot;
		pOutput->u27_ipme_5Mot_rb[2]= u27_12rgn_lft;
		pOutput->u27_ipme_5Mot_rb[3]= u27_12rgn_rgt;

	}
	#endif
        if(s_pParam->_param_filmDetectctrl.u1_mixMode_en == 0)
	{
                #if MIX_MODE_REGION_17
		pOutput->u27_ipme_aMot_rb	=	pOutput->u27_ipme_5Mot_rb[0]+pOutput->u27_ipme_5Mot_rb[4]+
                                        pOutput->u27_ipme_12Mot_rb[0] +
										pOutput->u27_ipme_12Mot_rb[1] +
										pOutput->u27_ipme_12Mot_rb[2] +
										pOutput->u27_ipme_12Mot_rb[3] +
										pOutput->u27_ipme_12Mot_rb[4] +
										pOutput->u27_ipme_12Mot_rb[5] +
										pOutput->u27_ipme_12Mot_rb[6] +
										pOutput->u27_ipme_12Mot_rb[7] +
                                        pOutput->u27_ipme_12Mot_rb[8] +
										pOutput->u27_ipme_12Mot_rb[9] +
                                        pOutput->u27_ipme_12Mot_rb[10]+
										pOutput->u27_ipme_12Mot_rb[11];
                 #else

                 pOutput->u27_ipme_aMot_rb	=	pOutput->u27_ipme_12Mot_rb[0] +
										pOutput->u27_ipme_12Mot_rb[1] +
										pOutput->u27_ipme_12Mot_rb[2] +
										pOutput->u27_ipme_12Mot_rb[3] +
										pOutput->u27_ipme_12Mot_rb[4] +
										pOutput->u27_ipme_12Mot_rb[5] +
										pOutput->u27_ipme_12Mot_rb[6] +
										pOutput->u27_ipme_12Mot_rb[7];
                #endif
	}

	if (pParam->u1_ipme_filmMode_rb_en == 1)
	{
		ReadRegister(KME_IPME_KME_IPME_F4_reg, 3, 5, &u32_RB_val);
		pOutput->u3_ipme_filmMode_rb = u32_RB_val;
	}

	//////////////////////////////////////////////////////////////////////////
	// kphase information
	//////////////////////////////////////////////////////////////////////////
	if (pParam->u1_kphase_rb_en == 1)
	{
		ReadRegister(KPHASE_kphase_9C_reg, 12, 15, &u32_RB_val);
		pOutput->u4_kphase_inPhase   = u32_RB_val;

		ReadRegister(KPHASE_kphase_9C_reg, 23, 23, &u32_RB_val);
		pOutput->u1_kphase_inLR      = u32_RB_val;

		ReadRegister(CRTC1_CRTC1_60_reg, 8, 15, &u32_RB_val);
		pOutput->u8_sys_N_rb      =  u32_RB_val;
		ReadRegister(CRTC1_CRTC1_60_reg, 16, 23, &u32_RB_val);
		pOutput->u8_sys_M_rb      = u32_RB_val;

		ReadRegister(CRTC1_CRTC1_1C_reg, 0, 12, &u32_RB_val);
		pOutput->u13_ip_vtrig_dly = u32_RB_val;

		ReadRegister(CRTC1_CRTC1_1C_reg, 13, 25, &u32_RB_val);
		pOutput->u13_me2_org_vtrig_dly = u32_RB_val;

		ReadRegister(CRTC1_CRTC1_24_reg, 0, 5, &u32_RB_val);
		pOutput->u13_me2_vtrig_dly = u32_RB_val;

		ReadRegister(CRTC1_CRTC1_24_reg, 6, 18, &u32_RB_val);
		pOutput->u13_dec_vtrig_dly = u32_RB_val;

		ReadRegister(CRTC1_CRTC1_F8_reg, 0, 12, &u32_RB_val);
		pOutput->u13_vtotal = u32_RB_val;

		ReadRegister(IPPRE_IPPRE_14_reg, 0, 11, &u32_RB_val);
		pOutput->u16_inHTotal = u32_RB_val;

		ReadRegister(IPPRE_IPPRE_F0_reg, 20, 31, &u32_RB_val);
		pOutput->u16_inVAct = u32_RB_val;

		ReadRegister(IPPRE_IPPRE_B4_reg, 0, 11, &u32_RB_val);
		pOutput->u16_inFrm_Pos = u32_RB_val;
	}

	//////////////////////////////////////////////////////////////////////////
	//Logo cnt readback
	//////////////////////////////////////////////////////////////////////////
	//no enable currently
	if(pParam->u1_lg_blk_cnt_rb_en == 1)
	{
		unsigned int idx;
		for (idx=0; idx < LOGO_RG_32; idx++)
		{
			ReadRegister(KME_LOGO2_KME_LOGO2_80_reg + idx * 4,
						20,
						29, pOutput->u10_blklogo_rgcnt + idx);

			pOutput->u10_blklogo_rgcnt[idx] = Calib_Data(pOutput->u10_blklogo_rgcnt[idx], g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);
		}
		ReadRegister(KME_DEHALO3_KME_DEHALO3_B0_reg, 0, 15, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt[0] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);
		ReadRegister(KME_DEHALO3_KME_DEHALO3_B8_reg, 0, 15, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt[1] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);
		ReadRegister(KME_DEHALO3_KME_DEHALO3_C4_reg, 0, 15, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt[2] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);
		ReadRegister(KME_DEHALO3_KME_DEHALO3_CC_reg, 0, 15, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt[3] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);

		ReadRegister(KME_DEHALO3_KME_DEHALO3_C4_reg, 16, 31, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt_bot[0] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);

		ReadRegister(KME_DEHALO3_KME_DEHALO3_C8_reg, 0, 15, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt_bot[1] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);

		ReadRegister(KME_DEHALO3_KME_DEHALO3_C8_reg, 16, 31, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt_bot[2] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);

		ReadRegister(KME_DEHALO3_KME_DEHALO3_B0_reg, 16, 31, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt_top[0] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);

		ReadRegister(KME_DEHALO3_KME_DEHALO3_B4_reg, 0, 15, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt_top[1] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);

		ReadRegister(KME_DEHALO3_KME_DEHALO3_B4_reg, 16, 31, &u32_RB_val);
		pOutput->u16_blklogo_dh_clrcnt_top[2] = Calib_Data((u32_RB_val & 0xFF), g_BLOGO_VBUF_GAIN, g_BLOGO_VBUF_SHIFT);

		for (idx=0; idx<16; idx++)
		{
			ReadRegister(KME_LOGO2_KME_LOGO2_40_reg + idx * 4,
						 0,
						 13, &u32_RB_val);
			pOutput->u14_pxllogo_rgcnt[idx*2+0] = Calib_Data(u32_RB_val, g_PLOGO_VBUF_GAIN, g_PLOGO_VBUF_SHIFT);

			ReadRegister(KME_LOGO2_KME_LOGO2_40_reg + idx * 4,
						 14,
						 27, &u32_RB_val);
			pOutput->u14_pxllogo_rgcnt[idx*2+1] = Calib_Data(u32_RB_val, g_PLOGO_VBUF_GAIN, g_PLOGO_VBUF_SHIFT);
		}


		for (idx=0; idx<8; idx++)
		{
			ReadRegister(KMC_METER_TOP_KMC_METER_TOP_DC_reg + idx * 4,
						 0,
						 24, &u32_RB_val);
			pOutput->u23_sub_histo_meter[idx] = Calib_Data(u32_RB_val, g_HEIGHT_GAIN, g_HEIGHT_SHIFT);
		}

		for (idx=0; idx<32; idx++)
		{
			ReadRegister(KMC_METER_TOP_KMC_METER_TOP_80_reg + idx * 4,
						 0,
						 24, &u32_RB_val);
			pOutput->u32_yuv_meter[idx] = Calib_Data(u32_RB_val, g_HEIGHT_GAIN, g_HEIGHT_SHIFT);
		}
	}
}
VOID ReadComReg_Proc_outputInterrupt(const _PARAM_ReadComReg *pParam, _OUTPUT_ReadComReg *pOutput)
{
	unsigned int u32_RB_val;
    unsigned int u32_local_flicker_val1, u32_local_flicker_val2;
    unsigned int u32_dh_bypass_cov_sum, u32_dh_bypass_ucov_sum;
	unsigned int u32_i;
	int RB_val;
       int  rgnIdx , rmvx_fg , rmvy_fg ,rmvx_bg , rmvy_bg ;
	unsigned int mvx_max = 0;
	unsigned short mvx_max_cnt = 0;
	unsigned int mvx_max_index = 0;
	unsigned int mvy_max = 0;
	unsigned short mvy_max_cnt = 0;
	unsigned int mvy_max_index = 0;
	unsigned short max_unconf = 0;
	signed short max_unconf_mvx = 0;
	signed short max_unconf_mvy = 0;
	unsigned short max_unconf_cnt = 0;
	unsigned int max_unconf_index = 0;
       unsigned short bgrgn_cnt =0 ;

	//unsigned int u32_new_saddiff_th;
	
	#if (IC_K5LP || IC_K6LP || IC_K8LP || IC_K24)
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_OUTPUT_RESOLUTION output_Resolution = s_pContext->_external_data._output_resolution;

#if 0//#if RTK_MEMC_Performance_tunging_from_tv001//xj //effect Check FB bad region
	const _Param_By_Resolution_RimCtrl *s_pRimParam = GetRimParam(output_Resolution);
	const _str_MEMC_buffer_size_table *tmp_MEMC_buffer_size_table = fwif_MEMC_get_MEMC_buffer_size_table((OUTPUT_RESOLUTION)output_Resolution);
	unsigned char u8_TopRim_th = (5*s_pRimParam->u8_scale_V)>>1; // 4k: u8_TopRim_th=10
	unsigned int SAD_offset = 0;
#else
	//const _Param_By_Resolution_RimCtrl *s_pRimParam = GetRimParam(output_Resolution);
	//const _str_MEMC_buffer_size_table *tmp_MEMC_buffer_size_table = fwif_MEMC_get_MEMC_buffer_size_table((OUTPUT_RESOLUTION)output_Resolution);
	//get the SAD_SHIFT_INFO method: https://wiki.realtek.com/x/9Rs2Eg
	unsigned char u8_TopRim_th = MEMC_PARAM_TABLE.SAD_SHIFT_Param[output_Resolution].u8_top_rim; // 4k: u8_TopRim_th=10
	unsigned int SAD_offset = MEMC_PARAM_TABLE.SAD_SHIFT_Param[output_Resolution].u32_sad_offset;
#endif
	#endif

	#if 0
	static unsigned int u32_SAD_pre=0,u32_SAD_pre_temp=0, u32_SAD_current=0,u32_SAD_diff=0;
	static unsigned char u8_SAD_overshot_cnt=0;
	#endif	
	#if 0 //for YE Test temp test
	static unsigned int u32_ID_Log_en=0;//,ES_Test_Value=0;
	ReadRegister(SOFTWARE1_SOFTWARE1_26_reg, 28,  28, &u32_ID_Log_en);//YE Test debug dummy register
	#endif

	//ReadRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, &u32_new_saddiff_th);

#if 0//#if RTK_MEMC_Performance_tunging_from_tv001//xj //effect Check FB bad region
	ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 0,  31, &SAD_offset);//YE Test debug dummy register
	SAD_offset = (SAD_offset/540)*tmp_MEMC_buffer_size_table->IPME1_VSize;
#else
	//ReadRegister(SOFTWARE1_SOFTWARE1_25_reg, 0,  31, &SAD_offset);//YE Test debug dummy register
	//SAD_offset = (SAD_offset/540)*tmp_MEMC_buffer_size_table->IPME1_VSize;
#endif
	

	if(0)//if(Get_DISPLAY_PANEL_TYPE() == P_VBY1_3K1K)//xj//3k
	{
		//u8_TopRim_th = 17;
		//SAD_offset = 0x00013ff6;
	}
	
	// scene change
	if (pParam->u1_sc_status_rb_en == 1)
	{
		WriteRegister(KME_ME1_TOP1_ME1_WLC1_00_reg, 0, 0, 0x01);
		ReadRegister(KME_ME1_TOP1_ME1_WLC1_00_reg, 0, 0, &u32_RB_val);
		pOutput->u1_sc_status_rb = u32_RB_val;

		WriteRegister(KME_ME1_TOP1_ME1_WLC1_00_reg, 4, 4, 0x01);
		ReadRegister(KME_ME1_TOP1_ME1_WLC1_00_reg, 4, 4, &u32_RB_val);
		pOutput->u1_sc_status_logo_rb = u32_RB_val;

		WriteRegister(KME_ME1_TOP1_ME1_WLC1_00_reg, 3, 3, 0x01);
		ReadRegister(KME_ME1_TOP1_ME1_WLC1_00_reg, 3, 3, &u32_RB_val);
		pOutput->u1_sc_status_dh_rb = u32_RB_val;
	}

	if (pParam->u1_me_aTC_rb_en == 1)
	{
		unsigned int u32_RB_val1 = 0, u32_RB_val2 = 0, u32_RB_val3 = 0;
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_E0_reg, 0, 27, &u32_RB_val);

		if (pParam->u2_rb_tc_mode == 0)
		{
			pOutput->u27_me_aTC_rb = u32_RB_val;
		}
		else if (pParam->u2_rb_tc_mode == 1)
		{
			ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_90_reg, 0, 19, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_94_reg, 0, 19, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_98_reg, 0, 19, &u32_RB_val3);

			pOutput->u27_me_aTC_rb = u32_RB_val1*pParam->u4_rb_tc1_coef + u32_RB_val2*pParam->u4_rb_tc2_coef + u32_RB_val3*pParam->u4_rb_tc3_coef;
		}
		else //if (pParam->u2_rb_tc_mode == 2)
		{
			ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_9C_reg, 0, 30, &u32_RB_val1);
			ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_A0_reg, 0, 30, &u32_RB_val2);
			ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_A4_reg, 0, 30, &u32_RB_val3);

			pOutput->u27_me_aTC_rb = u32_RB_val1*pParam->u4_rb_tc1_coef + u32_RB_val2*pParam->u4_rb_tc2_coef + u32_RB_val3*pParam->u4_rb_tc3_coef;
		}
        pOutput->u27_me_aTC_rb = Calib_Data(pOutput->u27_me_aTC_rb, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
	}
	if (pParam->u1_me_aSC_rb_en == 1)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_DC_reg, 0, 27, &u32_RB_val);
		pOutput->u27_me_aSC_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
	}

	
	if (pParam->u1_me_aSAD_rb_en == 1)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_D4_reg, 0, 30, &u32_RB_val);
		pOutput->u30_me_aSAD_rb = u32_RB_val;
		
		#if (IC_K5LP || IC_K6LP || IC_K8LP || IC_K24)
		if(s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] >= u8_TopRim_th){
			#if 0
			if(u32_ID_Log_en==1)
			{
				rtd_pr_memc_debug(" [MEMC SADG] , FBResult=%d, SAD =%d, sc_status=%d, HW_sc=%d, new_saddiff_th=%d, \n"
					,s_pContext->_output_fblevelctrl.u8_FBResult,pOutput->u30_me_aSAD_rb,s_pContext->_output_read_comreg.u1_sc_status_rb
					,s_pContext->_output_fblevelctrl.u1_HW_sc_true,u32_new_saddiff_th);
			}
			#endif
			if(pOutput->u30_me_aSAD_rb > (SAD_offset*16)){ // compare with UHD ME1 Vsize
				pOutput->u30_me_aSAD_rb = pOutput->u30_me_aSAD_rb - (SAD_offset*16);  // (0x3bfe2 *0x10)
			}

			}
		#endif
		pOutput->u30_me_aSAD_rb = ReadComReg_Calib_Data(pOutput->u30_me_aSAD_rb, CALIB_PARAM_SAD, 0, 0x7fffffff, false);
	}



	
	if (pParam->u1_me_aDTL_rb_en == 1)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_D8_reg, 0, 25, &u32_RB_val);
		pOutput->u25_me_aDTL_rb = ReadComReg_Calib_Data(u32_RB_val, CALIB_PARAM_DTL, 0, 0x3F4800, false);//4147200
	}

	if (pParam->u1_me_aAPLi_rb_en == 1)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_E4_reg, 0, 25, &u32_RB_val);
		pOutput->u26_me_aAPLi_rb = ReadComReg_Calib_Data(u32_RB_val, CALIB_PARAM_APL, 0, 0x3ffffff, false);
	}
	if (pParam->u1_me_aAPLp_rb_en == 1)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_E8_reg, 0, 25, &u32_RB_val);
		pOutput->u26_me_aAPLp_rb = ReadComReg_Calib_Data(u32_RB_val, CALIB_PARAM_APL, 0, 0x3ffffff, false);
	}

	// 32 rgn sad, dtl, apl, tc, sc, rmv
	for (u32_i = 0; u32_i < 32; u32_i ++)
	{
		// sad
		if (((pParam->u32_me_rSAD_rb_en >> u32_i)&0x1) == 1)
		{
			ReadRegister(KME_ME1_TOP7_ME1_STATIS_SAD_01_reg + 4 * u32_i, 0, 24, &u32_RB_val);
			pOutput->u25_me_rSAD_rb[u32_i] = u32_RB_val;
			#if (IC_K5LP || IC_K6LP || IC_K8LP || IC_K24)
			if(s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] >= u8_TopRim_th)
			{
				if((u32_i < 8) || (u32_i >= 24 && u32_i < 32)){
					#if 0
					if(u32_ID_Log_en==1)
					{
						//rtd_pr_memc_debug(" [MEMC SADR] ,u8_FBResult=%d,u32_i=%d Global SAD =%d, \n ",s_pContext->_output_fblevelctrl.u8_FBResult,u32_i,pOutput->u25_me_rSAD_rb[u32_i]);
					}
					#endif
					#if 1 // compare with UHD ME1 Vsize
						//pOutput->u25_me_rSAD_rb[u32_i] = pOutput->u25_me_rSAD_rb[u32_i] - ((0x3bfe2/540)*tmp_MEMC_buffer_size_table->IPME1_VSize);
					if(pOutput->u25_me_rSAD_rb[u32_i] > SAD_offset){ // compare with UHD ME1 Vsize
						pOutput->u25_me_rSAD_rb[u32_i] = pOutput->u25_me_rSAD_rb[u32_i] - SAD_offset;
					}
					#else
					pOutput->u25_me_rSAD_rb[u32_i] = pOutput->u25_me_rSAD_rb[u32_i] - 0x3bfe2;
					#endif
					}
			}
			#endif
			pOutput->u25_me_rSAD_rb[u32_i] = ReadComReg_Calib_Data(pOutput->u25_me_rSAD_rb[u32_i], CALIB_PARAM_SAD, 0, 0x1ffffff, true);
		}
		else
		{
			pOutput->u25_me_rSAD_rb[u32_i] = 0;
		}

		// detail
		if (((pParam->u32_me_rDTL_rb_en >> u32_i)&0x1) == 1)
		{
			ReadRegister(KME_ME1_TOP7_ME1_STATIS_DTL_01_reg + 4 * u32_i, 0, 19, &u32_RB_val);
			pOutput->u20_me_rDTL_rb[u32_i] = ReadComReg_Calib_Data(u32_RB_val, CALIB_PARAM_DTL, 0, 0x1FE00, true);//130560
		}
		else
		{
			pOutput->u20_me_rDTL_rb[u32_i] = 0;
		}

		// apl_i
		if (((pParam->u32_me_rAPLi_rb_en >> u32_i)&0x1) == 1)
		{
			ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_00_reg + 4 * u32_i, 0, 19, &u32_RB_val);
			pOutput->u20_me_rAPLi_rb[u32_i] = ReadComReg_Calib_Data(u32_RB_val, CALIB_PARAM_APL, 0, 0xfffff, true);
		}
		else
		{
			pOutput->u20_me_rAPLi_rb[u32_i] = 0;
		}

		// apl_p
		if (((pParam->u32_me_rAPLp_rb_en >> u32_i)&0x1) == 1)
		{
			ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_80_reg + 4 * u32_i, 0, 19, &u32_RB_val);
			pOutput->u20_me_rAPLp_rb[u32_i] = ReadComReg_Calib_Data(u32_RB_val, CALIB_PARAM_APL, 0, 0xfffff, true);
		}
		else
		{
			pOutput->u20_me_rAPLp_rb[u32_i] = 0;
		}

		// prd
		if (((pParam->u32_me_rPRD_rb_en >> u32_i)&0x1) == 1)
		{
			if(u32_i == 0)
			{
				ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_3C_reg, 0, 25, &u32_RB_val);
				pOutput->u13_me_rPRD_rb[0] = Calib_Data((u32_RB_val & 0x1fff), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
				pOutput->u13_me_rPRD_rb[1] = Calib_Data(((u32_RB_val & 0x3ffe000) >> 13), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
				
			}
			else if((u32_i % 2 == 0) && u32_i < 22)
			{
				ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_A8_reg + 2 * (u32_i - 2), 0, 25, &u32_RB_val);
				pOutput->u13_me_rPRD_rb[u32_i] = Calib_Data((u32_RB_val & 0x1fff), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
				pOutput->u13_me_rPRD_rb[u32_i + 1] = Calib_Data(((u32_RB_val & 0x3ffe000) >> 13), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
			}
			else if(u32_i % 2 == 0)
			{
				ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_EC_reg + 2 * (u32_i - 22), 0, 25, &u32_RB_val);
				pOutput->u13_me_rPRD_rb[u32_i] = Calib_Data((u32_RB_val & 0x1fff), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
				pOutput->u13_me_rPRD_rb[u32_i + 1] = Calib_Data(((u32_RB_val & 0x3ffe000) >> 13), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
			}
		}
		else
		{
			pOutput->u13_me_rPRD_rb[u32_i] = 0;
		}

		// LFB
		if (((pParam->u32_me_rLFB_rb_en >> u32_i)&0x1) == 1)
		{
			if(u32_i < 16)
			{
				ReadRegister(KME_DEHALO3_KME_DEHALO3_10_reg + 4 * u32_i, 20, 31, &u32_RB_val);
				pOutput->u12_me_rLFB_rb[u32_i] = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
			}
			else if(u32_i % 2 == 0)
			{
				ReadRegister(KME_DEHALO3_KME_DEHALO3_D8_reg + 2 * (u32_i - 16), 0, 23, &u32_RB_val);
				pOutput->u12_me_rLFB_rb[u32_i] = Calib_Data((u32_RB_val & 0xfff), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
				pOutput->u12_me_rLFB_rb[u32_i + 1] = Calib_Data(((u32_RB_val & 0xfff000) >> 12), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
			}
		}
		else
		{
			pOutput->u12_me_rLFB_rb[u32_i] = 0;
		}

		// temporal consistency
		if (((pParam->u32_me_rTC_rb_en >> u32_i)&0x1) == 1)
		{
			ReadRegister(KME_ME1_TOP8_KME_ME1_TOP8_80_reg + 4 * u32_i, 0, 19, &u32_RB_val);
			pOutput->u22_me_rTC_rb[u32_i] = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		}
		else
		{
			pOutput->u22_me_rTC_rb[u32_i] = 0;
		}

		// spatial consistency
		if (((pParam->u32_me_rSC_rb_en >> u32_i)&0x1) == 1)
		{
			ReadRegister(KME_ME1_TOP8_KME_ME1_TOP8_00_reg + 4 * u32_i, 0, 19, &u32_RB_val);
			pOutput->u22_me_rSC_rb[u32_i] = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		}
		else
		{
			pOutput->u22_me_rSC_rb[u32_i] = 0;
		}

		// rmv_1st
		if (((pParam->u32_me_rMV_1st_rb_en >> u32_i)&0x1) == 1)
		{
			ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_00_reg + 8 * u32_i, 0, 20, &u32_RB_val);
			pOutput->s11_me_rMV_1st_vx_rb[u32_i] = ((u32_RB_val >> 10) &1) == 0? (u32_RB_val         & 0x7ff) : ((u32_RB_val & 0x7ff) - (1<<11));
			pOutput->s10_me_rMV_1st_vy_rb[u32_i] = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val >> 11) & 0x3ff) : (((u32_RB_val >> 11) & 0x3ff) - (1<<10));
			pOutput->s11_me_rMV_1st_vx_rb[u32_i] = Calib_MV(pOutput->s11_me_rMV_1st_vx_rb[u32_i]);
			pOutput->s10_me_rMV_1st_vy_rb[u32_i] = Calib_MV(pOutput->s10_me_rMV_1st_vy_rb[u32_i]);

			ReadRegister(KME_ME1_TOP5_KME_ME1_TOP5_00_reg + 4 * u32_i, 0, 31, &u32_RB_val);
			pOutput->u12_me_rMV_1st_cnt_rb[u32_i] = Calib_Data((u32_RB_val & 0xfff), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);

			ReadRegister(KME_ME1_TOP5_KME_ME1_TOP5_80_reg + 4 * u32_i, 0, 31, &u32_RB_val);
			pOutput->u12_me_rMV_1st_unconf_rb[u32_i] = u32_RB_val & 0xfff;

			if (_ABS_(pOutput->s11_me_rMV_1st_vx_rb[u32_i]) > mvx_max){
				mvx_max = _ABS_(pOutput->s11_me_rMV_1st_vx_rb[u32_i]);	
				mvx_max_index = u32_i;
				mvx_max_cnt = pOutput->u12_me_rMV_1st_cnt_rb[u32_i];
			}
			pOutput->s11_rgn_mvx_max = mvx_max;
			pOutput->u8_rgn_mvx_max_index = mvx_max_index;
			pOutput->u12_rgn_mvx_max_cnt = mvx_max_cnt;

			if (_ABS_(pOutput->s10_me_rMV_1st_vy_rb[u32_i]) > mvy_max){
				mvy_max = _ABS_(pOutput->s10_me_rMV_1st_vy_rb[u32_i]);
				mvy_max_index = u32_i;
				mvy_max_cnt = pOutput->u12_me_rMV_1st_cnt_rb[u32_i];
			}
			pOutput->s11_rgn_mvy_max = mvy_max;
			pOutput->u8_rgn_mvy_max_index = mvy_max_index;
			pOutput->u12_rgn_mvy_max_cnt = mvy_max_cnt;

			if (pOutput->u12_me_rMV_1st_unconf_rb[u32_i] > max_unconf){
				max_unconf = pOutput->u12_me_rMV_1st_unconf_rb[u32_i];
				max_unconf_index = u32_i;
				max_unconf_mvx = pOutput->s11_me_rMV_1st_vx_rb[u32_i];
				max_unconf_mvy = pOutput->s10_me_rMV_1st_vy_rb[u32_i];
				max_unconf_cnt = pOutput->u12_me_rMV_1st_cnt_rb[u32_i];
			}
			pOutput->u12_rgn_max_unconf = max_unconf;
			pOutput->u8_rgn_max_unconf_index = max_unconf_index;
			pOutput->s11_rgn_max_unconf_mvx = max_unconf_mvx;
			pOutput->s11_rgn_max_unconf_mvy = max_unconf_mvy;
			pOutput->u12_rgn_max_unconf_cnt = max_unconf_cnt;			
			
		}
		else
		{
			pOutput->s11_me_rMV_1st_vx_rb[u32_i]     = 0;
			pOutput->s10_me_rMV_1st_vy_rb[u32_i]     = 0;
			pOutput->u12_me_rMV_1st_cnt_rb[u32_i]    = 0;
			pOutput->u12_me_rMV_1st_unconf_rb[u32_i] = 0;
			pOutput->s11_rgn_mvx_max = 0;
			pOutput->u8_rgn_mvx_max_index = 0;
			pOutput->u12_rgn_mvx_max_cnt = 0;
			pOutput->s11_rgn_mvy_max = 0;
			pOutput->u8_rgn_mvy_max_index = 0;
			pOutput->u12_rgn_mvy_max_cnt = 0; 
			pOutput->u12_rgn_max_unconf = 0;
			pOutput->u8_rgn_max_unconf_index = 0;
			pOutput->s11_rgn_max_unconf_mvx = 0;
			pOutput->s11_rgn_max_unconf_mvy = 0;
			pOutput->u12_rgn_max_unconf_cnt = 0;
		}

		// rmv_2nd
		if (((pParam->u32_me_rMV_2nd_rb_en >> u32_i)&0x1) == 1)
		{
			ReadRegister(KME_ME1_TOP4_KME_ME1_TOP4_04_reg + 8 * u32_i, 0, 20, &u32_RB_val);
			pOutput->s11_me_rMV_2nd_vx_rb[u32_i] = ((u32_RB_val >> 10) &1) == 0? (u32_RB_val         & 0x7ff) : ((u32_RB_val & 0x7ff) - (1<<11));
			pOutput->s10_me_rMV_2nd_vy_rb[u32_i] = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val >> 11) & 0x3ff) : (((u32_RB_val >> 11) & 0x3ff) - (1<<10));
			pOutput->s11_me_rMV_2nd_vx_rb[u32_i] = Calib_MV(pOutput->s11_me_rMV_2nd_vx_rb[u32_i]);
			pOutput->s10_me_rMV_2nd_vy_rb[u32_i] = Calib_MV(pOutput->s10_me_rMV_2nd_vy_rb[u32_i]);

			ReadRegister(KME_ME1_TOP5_KME_ME1_TOP5_00_reg + 4 * u32_i, 0, 31, &u32_RB_val);
			pOutput->u12_me_rMV_2nd_cnt_rb[u32_i] = Calib_Data(((u32_RB_val >> 12) & 0xfff), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);

			ReadRegister(KME_ME1_TOP5_KME_ME1_TOP5_80_reg + 4 * u32_i, 0, 31, &u32_RB_val);
			pOutput->u12_me_rMV_2nd_unconf_rb[u32_i] = (u32_RB_val >> 12) & 0xfff;
		}
		else
		{
			pOutput->s11_me_rMV_2nd_vx_rb[u32_i]     = 0;
			pOutput->s10_me_rMV_2nd_vy_rb[u32_i]     = 0;
			pOutput->u12_me_rMV_2nd_cnt_rb[u32_i]    = 0;
			pOutput->u12_me_rMV_2nd_unconf_rb[u32_i] = 0;
		}

		//dh readback
		if (pParam->u1_dh_rb_en == 1)
		{
			ReadRegister(KME_DEHALO3_KME_DEHALO3_10_reg + 4 * u32_i, 0, 19, &u32_RB_val);
			pOutput->u20_dh_pfvconf[u32_i] = Calib_Data((u32_RB_val & 0xFFFFF), g_ME2_PFV_GAIN, g_ME2_PFV_SHIFT);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//ZMV dc/ac psad, and
	//////////////////////////////////////////////////////////////////////////
	if (pParam->u1_me_ZMV_psad_rb_en)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_88_reg, 0, 30, &u32_RB_val);
		pOutput->u31_me_ZMV_psad_dc_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_8C_reg, 0, 30, &u32_RB_val);
		pOutput->u31_me_ZMV_psad_ac_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);

		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_80_reg, 0, 19, &u32_RB_val);
		pOutput->u20_me_ZMV_DcGreatAc_cnt_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_84_reg, 0, 19, &u32_RB_val);
		pOutput->u20_me_ZMV_AcGreatDc_cnt_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);

	}

	if (pParam->u1_mc_HVdtl_rb_en == 1)
	{
		ReadRegister(KMC_TOP_kmc_top_BC_reg, 0, 31, &u32_RB_val);
		pOutput->u16_mc_Hdtl_cnt = Calib_Data((u32_RB_val & 0xFFFF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
		pOutput->u16_mc_Vdtl_cnt = Calib_Data((u32_RB_val >> 16), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);		
	}

	//mc rdtl
	for (u32_i = 0; u32_i < 4; u32_i ++)
	{
		if (pParam->u1_mc_HVdtl_rb_en == 1)
		{
			ReadRegister(KMC_TOP_kmc_top_C0_reg + 12 * u32_i, 0, 31, &u32_RB_val);
			pOutput->u10_mc_VHdtl_rb[8*u32_i] = Calib_Data((u32_RB_val & 0x3FF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u10_mc_VHdtl_rb[8*u32_i+1] = Calib_Data(((u32_RB_val>>10) & 0x3FF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u10_mc_VHdtl_rb[8*u32_i+2] = Calib_Data(((u32_RB_val>>20) & 0x3FF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			ReadRegister(KMC_TOP_kmc_top_C0_reg + 12 * u32_i + 4, 0, 31, &u32_RB_val);
			pOutput->u10_mc_VHdtl_rb[8*u32_i+3] = Calib_Data((u32_RB_val & 0x3FF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u10_mc_VHdtl_rb[8*u32_i+4] = Calib_Data(((u32_RB_val>>10) & 0x3FF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u10_mc_VHdtl_rb[8*u32_i+5] = Calib_Data(((u32_RB_val>>20) & 0x3FF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			ReadRegister(KMC_TOP_kmc_top_C0_reg + 12 * u32_i + 8, 0, 31, &u32_RB_val);
			pOutput->u10_mc_VHdtl_rb[8*u32_i+6] = Calib_Data((u32_RB_val & 0x3FF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u10_mc_VHdtl_rb[8*u32_i+7] = Calib_Data(((u32_RB_val>>10) & 0x3FF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
		}
		else
		{
			pOutput->u10_mc_VHdtl_rb[8*u32_i] = 0;
			pOutput->u10_mc_VHdtl_rb[8*u32_i+1] = 0;
			pOutput->u10_mc_VHdtl_rb[8*u32_i+2] = 0;
			pOutput->u10_mc_VHdtl_rb[8*u32_i+3] = 0;
			pOutput->u10_mc_VHdtl_rb[8*u32_i+4] = 0;
			pOutput->u10_mc_VHdtl_rb[8*u32_i+5] = 0;
			pOutput->u10_mc_VHdtl_rb[8*u32_i+6] = 0;
			pOutput->u10_mc_VHdtl_rb[8*u32_i+7] = 0;
		}
	}

	if (pParam->u1_me_ZMV_cnt_en == 1)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_40_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_ZMV_cnt_rb = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);

		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_44_reg, 0, 19, &u32_RB_val);
		pOutput->u17_me_ZMV_dtl_cnt_rb = Calib_Data((u32_RB_val & 0xFFFFF), g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
	}

	if(pParam->u1_color_amount_rb_en == 1)
	{
		for (u32_i = 0; u32_i < 8; u32_i ++)
		{
			ReadRegister(IPPRE1_IPPRE1_E0_reg + u32_i * 4,  0, 31, &u32_RB_val);
			pOutput->u16_color_amount_u[u32_i] = Calib_Data((u32_RB_val & 0xFFFF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u16_color_amount_v[u32_i] = Calib_Data(((u32_RB_val>>16) & 0xFFFF), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);					
		}
	}


	if (pParam->u1_me_statis_glb_prd_en == 1)
	{
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_38_reg, 0, 18, &u32_RB_val);
		pOutput->u19_me_statis_glb_prd = Calib_Data(u32_RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
	}

	//RMV_blending & regional_fallback
	{
		ReadRegister(KMC_BI_bi_top_00_reg, 0, 0, &u32_RB_val);
		pOutput->u8_gmv_blend_en = u32_RB_val;
		ReadRegister(KMC_BI_bi_top_01_reg, 0, 0, &u32_RB_val);
		pOutput->u8_regional_fb_en = u32_RB_val;

		for(u32_i=0; u32_i<8; u32_i++)
		{
			ReadRegister(KMC_BI_bi_max_00_reg + u32_i*4, 0, 31, &u32_RB_val);
			pOutput->u8_region_fb_max[u32_i*4] = Calib_Data((u32_RB_val & 0xff), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u8_region_fb_max[u32_i*4+1] = Calib_Data(((u32_RB_val>>8) & 0xff), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u8_region_fb_max[u32_i*4+2] = Calib_Data(((u32_RB_val>>16) & 0xff), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u8_region_fb_max[u32_i*4+3] = Calib_Data(((u32_RB_val>>24) & 0xff), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
		}
		for(u32_i=0; u32_i<16; u32_i++)
		{
			ReadRegister(KMC_BI_bifb_cnt_00_reg + u32_i*4, 0, 31, &u32_RB_val);
			pOutput->u12_region_fb_cnt[u32_i*2] = Calib_Data((u32_RB_val & 0xfff), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
			pOutput->u12_region_fb_cnt[u32_i*2+1] = Calib_Data(((u32_RB_val>>16) & 0xfff), g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
		}
		for(u32_i=0; u32_i<32; u32_i++)
		{
			ReadRegister(KMC_BI_bifb_sum_11_reg + u32_i*4, 0, 31, &u32_RB_val);
			pOutput->u32_region_fb_sum[u32_i] = Calib_Data(u32_RB_val, g_ME2_VBUF_GAIN, g_ME2_VBUF_SHIFT);
		}

    }

        ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV01_reg,0,31, &u32_RB_val);
        pOutput->dh_gmv_fgx=((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>10)& 0x7ff):(((u32_RB_val>>10)& 0x7ff)  - (1<<11));
        pOutput->dh_gmv_fgy=((u32_RB_val >> 9) &1) == 0? (u32_RB_val& 0x3ff):((u32_RB_val& 0x3ff)  - (1<<10));
        pOutput->dh_gmv_bgy=((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10));
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV00_reg,0,31, &u32_RB_val);
        pOutput->dh_gmv_bgx=((u32_RB_val >> 26) &1) == 0? ((u32_RB_val>>16)& 0x7ff):(((u32_RB_val>>16)& 0x7ff)  - (1<<11)); 
        //pOutput->dh_gmv_bgx=Calib_Data(pOutput->dh_gmv_bgx, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        //djntest
		pOutput->dh_gmv_fgx=Calib_MV(pOutput->dh_gmv_fgx);
		pOutput->dh_gmv_fgy=Calib_MV(pOutput->dh_gmv_fgy);
		pOutput->dh_gmv_bgx=Calib_MV(pOutput->dh_gmv_bgx);
		pOutput->dh_gmv_bgy=Calib_MV(pOutput->dh_gmv_bgy);

        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_BG_APL00_reg,16,31, &RB_val);
        pOutput->dh_bg_apl_seg0_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_BG_APL00_reg,0,15, &RB_val);
        pOutput->dh_bg_apl_seg1_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_BG_APL01_reg,16,31, &RB_val);
        pOutput->dh_bg_apl_seg2_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_BG_APL01_reg,0,15, &RB_val);
        pOutput->dh_bg_apl_seg3_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_BG_APL02_reg,16,31, &RB_val);
        pOutput->dh_bg_apl_seg4_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_BG_APL02_reg,0,15, &RB_val);
        pOutput->dh_bg_apl_seg5_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_BG_APL03_reg,16,31, &RB_val);
        pOutput->dh_bg_apl_seg6_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_BG_APL03_reg,0,15, &RB_val);
        pOutput->dh_bg_apl_seg7_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_FG_APL00_reg,16,31, &RB_val);
        pOutput->dh_fg_apl_seg0_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_FG_APL00_reg,0,15, &RB_val);
        pOutput->dh_fg_apl_seg1_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_FG_APL01_reg,16,31, &RB_val);
        pOutput->dh_fg_apl_seg2_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_FG_APL01_reg,0,15, &RB_val);
        pOutput->dh_fg_apl_seg3_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_FG_APL02_reg,16,31, &RB_val);
        pOutput->dh_fg_apl_seg4_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_FG_APL02_reg,0,15, &RB_val);
        pOutput->dh_fg_apl_seg5_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_FG_APL03_reg,16,31, &RB_val);
        pOutput->dh_fg_apl_seg6_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_FG_APL03_reg,0,15, &RB_val);
        pOutput->dh_fg_apl_seg7_cnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);
        
        ReadRegister(KME_ME1_BG1_ME_BG_INFO_GMV00_reg,0,14, &RB_val);
        pOutput->me1_gmv_bgcnt=Calib_Data(RB_val, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);     
        //    rtd_pr_memc_debug("inMEMC_me1_bgmask_read");

    for(rgnIdx =0 ; rgnIdx< 32; rgnIdx++)
    {
        ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN00_BG_reg+4*rgnIdx ,0,31,&u32_RB_val);   
        rmvx_bg = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>10)& 0x7ff):(((u32_RB_val>>10)& 0x7ff)  - (1<<11)); 
        rmvy_bg = ((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10));
        ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN00_BG_reg+4*rgnIdx ,0,9,&u32_RB_val);   
	
#if 0//#if RTK_MEMC_Performance_tunging_from_tv001 //xj //effect Check FB bad region
	 bgrgn_cnt = u32_RB_val& 0x3ff ; 
#else
	 bgrgn_cnt = u32_RB_val;
#endif

	 pOutput->s11_bg_rMV_x_rb[rgnIdx] = Calib_MV(rmvx_bg);
	 pOutput->s10_bg_rMV_y_rb[rgnIdx] = Calib_MV(rmvy_bg);
	 pOutput->u16_bg_rgn_cnt[rgnIdx] = Calib_Data(bgrgn_cnt, g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT); 
    }

#if 0//#if RTK_MEMC_Performance_tunging_from_tv001 //xj //effect Check FB bad region
	//rgn fg mv
   for(rgnIdx =0 ;	rgnIdx< 10; rgnIdx++)
   {
	   ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN00_FG_reg+8*rgnIdx ,0,31,&u32_RB_val);   
	   rmvx_fg = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>10)& 0x7ff):(((u32_RB_val>>10)& 0x7ff)  - (1<<11)); 
	   rmvy_fg = ((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10));	  
	   pOutput->s11_fg_rMV_x_rb[0+3*rgnIdx]=rmvx_fg;
	   pOutput->s10_fg_rMV_y_rb[0+3*rgnIdx]=rmvy_fg;
	   rmvy_fg = ((u32_RB_val >> 9) &1) == 0? ((u32_RB_val>>0)& 0x3ff):(((u32_RB_val>>0)& 0x3ff)  - (1<<10));	   
	   ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN01_FG_reg+8*rgnIdx ,0,31,&u32_RB_val);   
	   rmvx_fg = ((u32_RB_val >> 31) &1) == 0? ((u32_RB_val>>21)& 0x7ff):(((u32_RB_val>>21)& 0x7ff)  - (1<<11)); 
	   pOutput->s11_fg_rMV_x_rb[1+3*rgnIdx]=rmvx_fg;
	   pOutput->s10_fg_rMV_y_rb[1+3*rgnIdx]=rmvy_fg; 
	   rmvx_fg = ((u32_RB_val >> 10) &1) == 0? ((u32_RB_val>>0)& 0x7ff):(((u32_RB_val>>0)& 0x7ff)  - (1<<11)); 
	   rmvy_fg = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>11)& 0x3ff):(((u32_RB_val>>11)& 0x3ff)  - (1<<10));	
	   pOutput->s11_fg_rMV_x_rb[2+3*rgnIdx]=rmvx_fg;
	   pOutput->s10_fg_rMV_y_rb[2+3*rgnIdx]=rmvy_fg;  
   //  rtd_pr_memc_info("fgmv %d %d %d %d %d %d\n",	rgnmvx_fg[0+3*rgnIdx],rgnmvy_fg[0+3*rgnIdx],rgnmvx_fg[1+3*rgnIdx], rgnmvy_fg[1+3*rgnIdx],rgnmvx_fg[2+3*rgnIdx] ,rgnmvy_fg[2+3*rgnIdx]); 
   }
   ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_20_FG_reg ,0,31,&u32_RB_val);	//fg rgn30 and fg rgn31
   pOutput->s11_fg_rMV_x_rb[30] = ((u32_RB_val >> 10) &1) == 0? ((u32_RB_val>>0)& 0x7ff):(((u32_RB_val>>0)& 0x7ff)	- (1<<11));  
   pOutput->s10_fg_rMV_y_rb[30] = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>11)& 0x3ff):(((u32_RB_val>>11)& 0x3ff)  - (1<<10));	   
   pOutput->s11_fg_rMV_x_rb[31]= ((u32_RB_val >> 31) &1) == 0? ((u32_RB_val>>21)& 0x7ff):(((u32_RB_val>>21)& 0x7ff)  - (1<<11)); 
   ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN21_FG_reg ,0,31,&u32_RB_val);    //fg rgn30 and fg rgn31	
   pOutput->s10_fg_rMV_y_rb[31]= ((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10)); 
#endif



	
/*	
	//rgnfgmvx
    for(rgnIdx =0 ; rgnIdx< 10; rgnIdx++)
    {
        ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN00_FG_reg+8*rgnIdx ,0,31,&u32_RB_val);   
        rmvx_fg = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>10)& 0x7ff):(((u32_RB_val>>10)& 0x7ff)  - (1<<11)); 
        rmvy_fg = ((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10));      
        pOutput->s11_fg_rMV_x_rb[0+3*rgnIdx]=rmvx_fg;
        pOutput->s10_fg_rMV_y_rb[0+3*rgnIdx]=rmvy_fg;
        rmvy_fg = ((u32_RB_val >> 9) &1) == 0? ((u32_RB_val>>0)& 0x3ff):(((u32_RB_val>>0)& 0x3ff)  - (1<<10));      
        ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN01_FG_reg+8*rgnIdx ,0,31,&u32_RB_val);   
        rmvx_fg = ((u32_RB_val >> 31) &1) == 0? ((u32_RB_val>>21)& 0x7ff):(((u32_RB_val>>21)& 0x7ff)  - (1<<11)); 
        pOutput->s11_fg_rMV_x_rb[1+3*rgnIdx]=rmvx_fg;
        pOutput->s10_fg_rMV_y_rb[1+3*rgnIdx]=rmvy_fg;
        rmvx_fg = ((u32_RB_val >> 10) &1) == 0? ((u32_RB_val>>0)& 0x7ff):(((u32_RB_val>>0)& 0x7ff)  - (1<<11)); 
        rmvy_fg = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>11)& 0x3ff):(((u32_RB_val>>11)& 0x3ff)  - (1<<10));    
        pOutput->s11_fg_rMV_x_rb[2+3*rgnIdx]=rmvx_fg;
        pOutput->s10_fg_rMV_y_rb[2+3*rgnIdx]=rmvy_fg; 
     //  rtd_pr_memc_debug("fgmv %d %d %d %d %d %d\n",  rgnmvx_fg[0+3*rgnIdx],rgnmvy_fg[0+3*rgnIdx],rgnmvx_fg[1+3*rgnIdx], rgnmvy_fg[1+3*rgnIdx],rgnmvx_fg[2+3*rgnIdx] ,rgnmvy_fg[2+3*rgnIdx]);       
    }
    ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_20_FG_reg ,0,31,&u32_RB_val);    //fg rgn30 and fg rgn31
    pOutput->s11_fg_rMV_x_rb[30] = ((u32_RB_val >> 10) &1) == 0? ((u32_RB_val>>0)& 0x7ff):(((u32_RB_val>>0)& 0x7ff)  - (1<<11));  

    pOutput->s10_fg_rMV_y_rb[30] = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>11)& 0x3ff):(((u32_RB_val>>11)& 0x3ff)  - (1<<10));   
    pOutput->s11_fg_rMV_x_rb[31]= ((u32_RB_val >> 31) &1) == 0? ((u32_RB_val>>21)& 0x7ff):(((u32_RB_val>>21)& 0x7ff)  - (1<<11)); 
    //pOutput->s11_fg_rMV_x_rb[30] = Calib_Data( pOutput->s11_fg_rMV_x_rb[30], g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);  
    ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN21_FG_reg ,0,31,&u32_RB_val);    //fg rgn30 and fg rgn31  
    pOutput->s10_fg_rMV_y_rb[31]= ((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10)); 
    //pOutput->s10_fg_rMV_y_rb[30] = Calib_Data( pOutput->s10_fg_rMV_y_rb[30], g_ME1_VBUF_GAIN, g_ME1_VBUF_SHIFT);  

    for(rgnIdx = 0; rgnIdx<32; rgnIdx++){
		pOutput->s11_fg_rMV_x_rb[rgnIdx] = Calib_MV(pOutput->s11_fg_rMV_x_rb[rgnIdx]);
		pOutput->s10_fg_rMV_y_rb[rgnIdx] = Calib_MV(pOutput->s10_fg_rMV_y_rb[rgnIdx]);
	}

*/

    for(rgnIdx =0 ; rgnIdx< 10; rgnIdx++)
    {
        ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN00_FG_reg+8*rgnIdx ,0,31,&u32_RB_val);   
        rmvx_fg = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>10)& 0x7ff):(((u32_RB_val>>10)& 0x7ff)  - (1<<11)); 
        rmvy_fg = ((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10));   
        pOutput->s11_fg_rMV_x_rb[0+3*rgnIdx]=rmvx_fg;
        pOutput->s10_fg_rMV_y_rb[0+3*rgnIdx]=rmvy_fg;
        rmvy_fg = ((u32_RB_val >> 9) &1) == 0? ((u32_RB_val>>0)& 0x3ff):(((u32_RB_val>>0)& 0x3ff)  - (1<<10));      
        pOutput->s10_fg_rMV_y_rb[1+3*rgnIdx]=rmvy_fg;

        
        ReadRegister(KME_ME1_BG0_ME_BG_INFO_RGN01_FG_reg+8*rgnIdx ,0,31,&u32_RB_val);   
        rmvx_fg = ((u32_RB_val >> 31) &1) == 0? ((u32_RB_val>>21)& 0x7ff):(((u32_RB_val>>21)& 0x7ff)  - (1<<11)); 
        pOutput->s11_fg_rMV_x_rb[1+3*rgnIdx]=rmvx_fg;

#if 0//#if RTK_MEMC_Performance_tunging_from_tv001 //xj //effect Check FB bad region
		pOutput->s10_fg_rMV_y_rb[1+3*rgnIdx]=rmvy_fg; 
#endif
        rmvx_fg = ((u32_RB_val >> 10) &1) == 0? ((u32_RB_val>>0)& 0x7ff):(((u32_RB_val>>0)& 0x7ff)  - (1<<11)); 
        rmvy_fg = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>11)& 0x3ff):(((u32_RB_val>>11)& 0x3ff)  - (1<<10));    
        pOutput->s11_fg_rMV_x_rb[2+3*rgnIdx]=rmvx_fg;
        pOutput->s10_fg_rMV_y_rb[2+3*rgnIdx]=rmvy_fg; 
     //  rtd_pr_memc_debug("fgmv %d %d %d %d %d %d\n",  rgnmvx_fg[0+3*rgnIdx],rgnmvy_fg[0+3*rgnIdx],rgnmvx_fg[1+3*rgnIdx], rgnmvy_fg[1+3*rgnIdx],rgnmvx_fg[2+3*rgnIdx] ,rgnmvy_fg[2+3*rgnIdx]);       
    }
    
    ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN_20_FG_reg ,0,31,&u32_RB_val);    //fg rgn30 and fg rgn31
    pOutput->s11_fg_rMV_x_rb[30] = ((u32_RB_val >> 10) &1) == 0? ((u32_RB_val>>0)& 0x7ff):(((u32_RB_val>>0)& 0x7ff)  - (1<<11));  
    pOutput->s10_fg_rMV_y_rb[30] = ((u32_RB_val >> 20) &1) == 0? ((u32_RB_val>>11)& 0x3ff):(((u32_RB_val>>11)& 0x3ff)  - (1<<10));   
    pOutput->s11_fg_rMV_x_rb[31]= ((u32_RB_val >> 31) &1) == 0? ((u32_RB_val>>21)& 0x7ff):(((u32_RB_val>>21)& 0x7ff)  - (1<<11)); 
    ReadRegister(KME_ME1_BG1_ME_BG_INFO_RGN21_FG_reg ,0,31,&u32_RB_val);    //fg rgn30 and fg rgn31  
    pOutput->s10_fg_rMV_y_rb[31]= ((u32_RB_val >> 30) &1) == 0? ((u32_RB_val>>21)& 0x3ff):(((u32_RB_val>>21)& 0x3ff)  - (1<<10)); 


    for(rgnIdx = 0; rgnIdx<32; rgnIdx++){
		pOutput->s11_fg_rMV_x_rb[rgnIdx] = Calib_MV(pOutput->s11_fg_rMV_x_rb[rgnIdx]);
		pOutput->s10_fg_rMV_y_rb[rgnIdx] = Calib_MV(pOutput->s10_fg_rMV_y_rb[rgnIdx]);
	}

    

    for (u32_i = 0; u32_i < 16; u32_i++)
	{
		ReadRegister(MC3_mc_local_lpf_cnt0_reg + 4 * u32_i, 0, 14, &u32_local_flicker_val1);
        ReadRegister(MC3_mc_local_lpf_cnt0_reg + 4 * u32_i, 15, 29, &u32_local_flicker_val2);
        
		pOutput->u32_region_local_flicker_sum[_CLIP_(u32_i * 2, 0, 31)] = u32_local_flicker_val1;
        pOutput->u32_region_local_flicker_sum[_CLIP_((u32_i * 2 + 1), 0, 31)] = u32_local_flicker_val2;
    }    
    ReadRegister(KME_DEHALO5_PHMV_FIX_4D_reg, 0, 16, &u32_dh_bypass_cov_sum);
    ReadRegister(KME_DEHALO5_PHMV_FIX_4E_reg, 0, 16, &u32_dh_bypass_ucov_sum);
    pOutput->u32_dh_bypass_cov_sum = u32_dh_bypass_cov_sum;
    pOutput->u32_dh_bypass_ucov_sum = u32_dh_bypass_ucov_sum;

}



