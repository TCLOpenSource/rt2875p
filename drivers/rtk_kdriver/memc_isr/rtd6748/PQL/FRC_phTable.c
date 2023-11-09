#include "memc_isr/PQL/PQLPlatformDefs.h"

#include "memc_isr/PQL/PQLGlobalDefs.h"

#include "memc_isr/PQL/FRC_glb_Context.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/Common/kw_debug.h"
#include "memc_isr/Driver/regio.h"
#include "memc_isr/scalerMEMC.h"
#include "tvscalercontrol/panel/panelapi.h"
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include "memc_reg_def.h"
#include <rtd_log/rtd_module_log.h>
//#include "scaler_vpqmemcdev.h"
#include <rbus/mc_dma_reg.h>
#include <rbus/me_share_dma_reg.h>
#include "memc_isr/PQL/MEMC_ParamTable.h"

#ifdef CONFIG_ARM64 //ARM32 compatible
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#endif

//////////////////////////////////////////////////////////////////////////

Ph_decGen_Out         phT_dec_PC =
{
	{ 0x01, 0x01,   1,  1, 0,  0x1,   1,  0,  _CAD_VIDEO, 0},	//FRC_CADENCE           local_FRC_cadence;
	0,		//unsigned char                 u8_fracPh_nume;
	1,		//unsigned char                 u8_fracPh_denomi;
	0,		//unsigned short                u16_phT_stIdx;
	1,		//unsigned short                u16_phT_endIdx_p1;
	1,		//unsigned short                u16_phT_length;
	0		//unsigned short                u16_filmPh0_tableIdx;
};
Ph_decGen_Out         phT_dec_Video =
{
	{ 0x01, 0x01,   1,  1, 0,  0x1,   1,  0,  _CAD_VIDEO, 0},	//FRC_CADENCE           local_FRC_cadence;
	0,		//unsigned char                 u8_fracPh_nume;
	1,		//unsigned char                 u8_fracPh_denomi;
	0,		//unsigned short                u16_phT_stIdx;
	1,		//unsigned short                u16_phT_endIdx_p1;
	1,		//unsigned short                u16_phT_length;
	0		//unsigned short                u16_filmPh0_tableIdx;
};
Ph_decGen_Out         phT_dec_Film =
{
	{ 0xA,  0x03,   1,  2, 0,  0xf,   4,  0,  _CAD_22,    0},	//FRC_CADENCE           local_FRC_cadence;
	0,		//unsigned char                 u8_fracPh_nume;
	1,		//unsigned char                 u8_fracPh_denomi;
	0,		//unsigned short                u16_phT_stIdx;
	1,		//unsigned short                u16_phT_endIdx_p1;
	1,		//unsigned short                u16_phT_length;
	0		//unsigned short                u16_filmPh0_tableIdx;
};

//////////////////////////////////////////////////////////////////////////

unsigned char phase_cnt = 0;
unsigned char lowdelay_state = 0;
unsigned char lowdelay_state_pre = 0;
unsigned char lowdelay_state_chg_flg = 0;
unsigned char lowdelay_state_chg_cnt = 0;
extern int DC_off_on_flg;
extern int LowDelay_mode;
extern int LowDelay_regen_chk;
extern unsigned char VR360_en;
extern unsigned char u8_MEMCMode;
extern unsigned int GS_image_vsize;
int LowDelayGen_cnt = 0;
#define  PPOVERLAY_MEMC_MUX_CTRL_reg	0xB8028100
#define  PPOVERLAY_Main_Display_Control_RSV_reg	0xB8028300
#define  PPOVERLAY_Main_Display_Control_RSV_get_m_force_bg(data)	((0x00000002&(data))>>1)
extern VOID MEMC_Lib_set_LineMode_flag(unsigned char u1_enable);
extern unsigned char MEMC_Lib_get_Adaptive_Stream_Flag(VOID);
extern unsigned char MEMC_Lib_get_DisplayMode_Src(VOID);
extern unsigned char Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(VOID);
extern unsigned char fwif_color_get_BFI_En(void);
#ifdef CONFIG_MEDIA_SUPPORT
extern unsigned char vsc_get_main_win_apply_done_event_subscribe(void);
#endif
extern unsigned char MEMC_Lib_UltraLowDelayMode_Judge(VOID);
extern VOID MEMC_Lib_Set_UltraLowDelayMode(unsigned char u1_enable);
extern unsigned char get_scaler_run_cloud_game(void);
extern unsigned char drv_memory_get_vdec_direct_low_latency_mode(void);
extern VOID MEMC_Lib_set_LbmcTrig_flag(unsigned char u1_enable);
extern unsigned int Get_DISPLAY_PANEL_OLED_TYPE(void);
extern unsigned char Scaler_check_orbit_store_mode(void);
extern VOID Mid_Mode_EnableMCDMA(VOID);
extern VOID Mid_Mode_DisableMCDMA(VOID);
extern VOID Mid_Mode_EnableMEDMA(VOID);
extern VOID Mid_Mode_DisableMEDMA(unsigned char MEinfoKeep);
extern VOID Mid_Mode_SetMEMCFrameRepeatEnable(BOOL bEnable);
extern void Scaler_MEMC_MC_CLK_Enable(void);
extern void Scaler_MEMC_MC_CLK_Disable(void);
extern void Scaler_MEMC_ME_CLK_Enable(void);
extern void Scaler_MEMC_ME_CLK_Disable(void);
extern int Scaler_MEMC_MEMC_CLK_OnOff(unsigned char ME_enable, unsigned char MC_enable, unsigned char bMute);
extern VOID Scaler_MEMC_Set_EnableToBypass_Flag(unsigned char u1_enable);
extern BOOL MEMC_LibSetMEMC_PCModeEnable(BOOL bEnable);
extern unsigned char get_scaler_qms_mode_flag(void);
extern unsigned int Get_DISPLAY_PANEL_OLED_TYPE(void);
extern unsigned char Scaler_check_orbit_store_mode(void);
extern unsigned char vbe_get_disp_timing_change_status(void);
extern VOID Mid_Mode_EnableMCDMA(VOID);
extern VOID Mid_Mode_DisableMCDMA(VOID);
extern VOID Mid_Mode_EnableMEDMA(VOID);
extern VOID Mid_Mode_DisableMEDMA(unsigned char MEinfoKeep);
extern VOID Mid_Mode_SetMEMCFrameRepeatEnable(BOOL bEnable);
extern void Scaler_MEMC_MC_CLK_Enable(void);
extern void Scaler_MEMC_MC_CLK_Disable(void);
extern void Scaler_MEMC_ME_CLK_Enable(void);
extern void Scaler_MEMC_ME_CLK_Disable(void);
extern int Scaler_MEMC_MEMC_CLK_OnOff(unsigned char ME_enable, unsigned char MC_enable, unsigned char bMute);
extern VOID MEMC_Lib_set_memc_enable_to_bypass_flag(unsigned char u1_enable);
extern BOOL MEMC_LibSetMEMC_PCModeEnable(BOOL bEnable);

//extern VPQ_MEMC_TYPE_T Scaler_MEMC_GetMotionType(void);
//extern void Scaler_MEMC_SetMotionComp(unsigned char blurLevel, unsigned char judderLevel, SCALER_MEMC_TYPE_T motion);

VOID FRC_phTable_Init(_OUTPUT_FRC_PH_TABLE *pOutput)
{
	pOutput->u8_system_start_lock      = 0;
	pOutput->u1_prt_trig               = 0;
	pOutput->u8_phT_print_en_pre       = 0;
	pOutput->u8_phT_print_frmCnt0      = 0;

	pOutput->u8_deblur_lvl_pre         = (1 << _PHASE_FLBK_LVL_PRECISION);
	pOutput->u8_dejudder_lvl_pre       = (1 << _PHASE_FLBK_LVL_PRECISION);
	pOutput->u8_pulldown_mode_pre      = _PQL_PULLDOWN_MODE_ELSE;

	pOutput->u8_filmPh                 = 0;
	pOutput->u8_filmPh0_inPh_pre       = 0;
	pOutput->u8_filmPh0_cnt            = 0;

	pOutput->u8_cadence_id_pre         = _FRC_CADENCE_NUM_;
	pOutput->u8_film_cadence_id_pre    = _CAD_22;
	pOutput->u8_sys_N_pre              = 1;
	pOutput->u8_sys_M_pre              = 2;

	pOutput->in3d_format_pre           = _PQL_IN_2D;
	pOutput->out3d_format_pre          = _PQL_OUT_2D;

	pOutput->u1_outMode_pre            = 0;

	pOutput->u16_phTable_wrt_stIdx     = 0;
	pOutput->u16_phTable_wrt_endIdx_p1 = 0;
	pOutput->u16_phTable_wrt_vd_endIdx_p1 = 1;

	//// after image
	pOutput->u1_after_img               = 0;
	pOutput->u1_after_img_type             = 0;
	pOutput->u8_after_img_cadence_id_pre   = _CAD_VIDEO;

	pOutput->u1_lowdelaystate               = 0;
	pOutput->u1_lowdelay_regen               = 0;
	pOutput->u1_inframe_pre               = 0;
	pOutput->u8_source_type_pre               = 0;
	pOutput->u8_memc_mode_pre               = 0;
	pOutput->u1_VR360_en_pre               = 0;
	pOutput->u1_adaptivestream_flag_pre	= 0;
	pOutput->u1_direct_media_flag_pre	= 0;
	pOutput->u1_timing_status_pre = 0;
	pOutput->QMS_24hz_state_pre = 0;
	pOutput->u1_qms_flag_pre = 0;
	///// phase table init setting
	FRC_phTable_Init_64bitConfig();
	FRC_phTable_Init_decGen_config();
	FRC_phTable_Init_1n2m_video_22_32();
	FRC_phTable_Init_1n2m_video_22_32_intable();
}

extern unsigned char MEMC_instanboot_resume_check[10];
VOID FRC_PhaseTable_Init(_OUTPUT_FRC_PH_TABLE *pOutput)
{
	unsigned int u32InINT_Clear = 0;
	MEMC_instanboot_resume_check[3]=1;
	rtd_pr_memc_notice("FRC_PhaseTable_Init Called!![9018,%x][d008,%x]\r\n", rtd_inl(KMC_TOP_kmc_top_18_reg), rtd_inl(KPOST_TOP_KPOST_TOP_08_reg));

	// reset wclr bit to sure interrupt can continue processing.
	ReadRegister(KMC_TOP_kmc_top_18_reg,20,23, &u32InINT_Clear);
	u32InINT_Clear = u32InINT_Clear & (~( 1 << (unsigned int)1)); // INT_TYPE_VSYNC
	WriteRegister(KMC_TOP_kmc_top_18_reg,20,23, u32InINT_Clear);

	//rtd_pr_memc_notice("%s %d\n",__func__,__LINE__);	

	ReadRegister(KPOST_TOP_KPOST_TOP_08_reg,1,4, &u32InINT_Clear);
	//rtd_pr_memc_notice("1.[%d]\r\n",u32InINT_Clear);
	u32InINT_Clear = u32InINT_Clear & (~( 1 << (unsigned int)1)); // INT_TYPE_VSYNC
	//rtd_pr_memc_notice("2.[%d]\r\n",u32InINT_Clear);
	WriteRegister(KPOST_TOP_KPOST_TOP_08_reg,1,4, u32InINT_Clear);

	pOutput->u8_system_start_lock = 0;
	//pOutput->u1_usecase_call = 1;//k2 method, k3 remove but keep code

	// work around patch
	//k2 method, k3 remove but keep code
	//WriteRegister(KME_IPME_KME_IPME_8C_reg, 0, 31, 0x24242424);
	//WriteRegister(KME_IPME_KME_IPME_94_reg, 0, 31, 0x24242424);
}


VOID FRC_phTable_Proc(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput)
{
	if (pParam->u1_phT_autoGen_en == 1)
	{
		FRC_phTable_StateMachine(pParam, pOutput);
		FRC_phTable_Generate(pParam, pOutput);
		FRC_phTable_Write(pParam, pOutput);  // bai  need think and add
		FRC_phTable_Delay_Calc(pParam, pOutput);//for delay information calc.
		FRC_phTable_Cadence_Change(pParam, pOutput);

		if(pOutput->u1_prt_trig == 1 && pOutput->u8_phT_print_frmCnt0 > 1)
		{
			rtd_pr_memc_notice("video_stIdx = %d, video_endIdx = %d, film_stIdx = %d, film_endIdx = %d, \r\n ", phT_dec_Video.u16_phT_stIdx, phT_dec_Video.u16_phT_endIdx_p1, phT_dec_Film.u16_phT_stIdx, phT_dec_Film.u16_phT_endIdx_p1);
			rtd_pr_memc_notice("outT_stIdx = %d, outT_endIdx = %d \r\n ", pOutput->u16_phTable_wrt_stIdx, pOutput->u16_phTable_wrt_endIdx_p1);
		}

		if(pOutput->u8_system_start_lock == 0)
		{
			pOutput->u8_system_start_lock = pOutput->u8_system_start_lock + 1;
		}
	}
}

extern unsigned int loadscript_cnt_2;
VOID FRC_phTable_Delay_Calc(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput)
{
#if 0 // move to MEMC_Lib_DelayTime_Calc()
	// MEMC delay time
	static unsigned char Ph_delay_calc_en_pre = 0;
	unsigned char Ph_delay_calc_en_cur = pParam->u1_Ph_delay_calc_en;
	static unsigned char calc_valid = 0;

	static unsigned int cnt = 1;
	static unsigned int cnt1 = 1;
	cnt1 ++ ;

	if(cnt1%480 ==0 || cnt%480 == 0 ){
		rtd_pr_memc_emerg("cnt1 = %d,cnt = %d,en_cur=%d\n",cnt1, cnt, Ph_delay_calc_en_cur);
		cnt1 = 0 ;
	}

	if(Ph_delay_calc_en_cur)
	{
		const _PQLCONTEXT *s_pContext = GetPQLContext();
		static unsigned char Ph_gen_en_pre = 0;
		static unsigned int delay_ms = 0;
		unsigned char Ph_gen_en_cur = pOutput->u1_inTable_wrt_en || pOutput->u1_outTable_wrt_en || pOutput->u1_filmPh_wrt_en;

		static unsigned char u8_delay_calc_cnt = 0;    // add by robin
		static unsigned char u8_interrupt_cnt = 0;
		unsigned char u8_sys_N = s_pContext->_output_read_comreg.u8_sys_N_rb;
		unsigned char u8_sys_M = s_pContext->_output_read_comreg.u8_sys_M_rb;


		if((Ph_gen_en_pre == 1 && Ph_gen_en_cur == 0) || (Ph_delay_calc_en_pre == 0 && Ph_delay_calc_en_cur == 1))
		{
			u8_interrupt_cnt = 1;
			u8_delay_calc_cnt = 5;
			delay_ms = 0;
		}

		Ph_gen_en_pre = Ph_gen_en_cur;

		// continuous 12 frames table content
		// toggle
		if(u8_interrupt_cnt == 1)
		{
			WriteRegister(KPHASE_kphase_10_reg, 27, 27, 0);
		}
		else if(u8_interrupt_cnt == 2)
		{
			WriteRegister(KPHASE_kphase_10_reg, 27, 27, 1);
		}
		else if(u8_interrupt_cnt > 12 * u8_sys_N /u8_sys_M + 4 + (u8_delay_calc_cnt))
		{
			// read me2 I/P index
			unsigned int in_me2_index[12] = {0}, me2_p_index[12] = {0}, me2_phase[12] = {0};
			unsigned char i = 0, j = 0, max_delay = 0;
			int delay[12];
			unsigned int out_framerate;

			for(i = 0; i < 12; i++)
			{
				//in me2 index
				ReadRegister(KPHASE_kphase_A4_reg + 8*i, 13, 15, &in_me2_index[i]);
				//me2 P index
				ReadRegister(KPHASE_kphase_A4_reg + 8*i, 19, 21, &me2_p_index[i]);
				//me2 phase
				ReadRegister(KPHASE_kphase_A0_reg + 8*i, 25, 30, &me2_phase[i]);
			}

			if(MEMC_LibGetMEMCFrameRepeatEnable())	  //memc off: repeat mode
			{
				for(i = 0; i < 12; i++)
				{
					delay[i] = -1;
					if(i > 0 && in_me2_index[i] == in_me2_index[i-1])
						continue;
					for(j = i; j <12; j++)
					{
						if( me2_p_index[j] == in_me2_index[i] )
						{
							delay[i] = j - i;	 //pz mode
							break;
						}
					}
				}
			}
			else
			{

 			if(pOutput->u1_is_inFormat_PureVideo)
 			{
 				for(i = 0; i < 12; i++)
				{
					delay[i] = -1;
					if(i > 0 && in_me2_index[i] == in_me2_index[i-1])
						continue;
					for(j = i; j <12; j++)
					{
						if(me2_p_index[j] == in_me2_index[i])
						{
							delay[i] = j - i + 1;
							break;
						}
					}
				}
 			}
			else
			{
				for(i = 0; i < 12; i++)
				{
					delay[i] = -1;
					if(i > 0 && in_me2_index[i] == in_me2_index[i-1])
						continue;
					for(j = i; j <12; j++)
					{
						if(me2_p_index[j] == in_me2_index[i] && me2_phase[j] > 0)
						{
							delay[i] = j - i;
							break;
						}
					}
				}
			}
			}

			for(i = 0; i < 12; i++)
			{
				if(delay[i] >= 0 && delay[i] > max_delay)
				{
					max_delay = delay[i];
				}
			}

			RTKReadRegister(0xB80282EC, &out_framerate);
			out_framerate = ((270000000 / (out_framerate + 1)) + 5) / 10;

			//delay_ms = (max_delay * 1000) / out_framerate;
			delay_ms = max(delay_ms, (max_delay * 1000) / out_framerate);

			calc_valid = 1;

#if 0
			cnt++;
			rtd_pr_memc_emerg("cnt = %d, u8_interrupt_cnt = %d, u8_delay_calc_cnt =%d\n",cnt,u8_interrupt_cnt, u8_delay_calc_cnt);
			rtd_pr_memc_emerg("PureVideo = %d, RepeatEnable = %d\n",pOutput->u1_is_inFormat_PureVideo, MEMC_LibGetMEMCFrameRepeatEnable());
			rtd_pr_memc_emerg("[ph] (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n[write] (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n[read_p] (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n[delay] (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)\n",
				me2_phase[0], me2_phase[1], me2_phase[2], me2_phase[3], me2_phase[4], me2_phase[5],
				me2_phase[6], me2_phase[7], me2_phase[8], me2_phase[9], me2_phase[10], me2_phase[11],
				in_me2_index[0], in_me2_index[1], in_me2_index[2], in_me2_index[3], in_me2_index[4], in_me2_index[5],
				in_me2_index[6], in_me2_index[7], in_me2_index[8], in_me2_index[9], in_me2_index[10], in_me2_index[11],
				me2_p_index[0], me2_p_index[1], me2_p_index[2], me2_p_index[3], me2_p_index[4], me2_p_index[5],
				me2_p_index[6], me2_p_index[7], me2_p_index[8], me2_p_index[9], me2_p_index[10], me2_p_index[11],
				delay[0], delay[1], delay[2], delay[3], delay[4], delay[5], delay[6], delay[7], delay[8], delay[9], delay[10], delay[11]);
#endif

			u8_interrupt_cnt = 0;
			u8_delay_calc_cnt --;

		}

		//if(u8_interrupt_cnt > 0)
		if(u8_interrupt_cnt > 0 || u8_delay_calc_cnt > 0)
			u8_interrupt_cnt ++;


		if(calc_valid)
		{
			if(loadscript_cnt_2 % 6000 == 1)			{
				rtd_pr_memc_notice("[MEMC][Delay] time = %dms\n", delay_ms);
			}
		}

		Scaler_MEMC_SetAVSyncDelay(delay_ms);

	}
	else
	{
		calc_valid = 0;
	}

	Ph_delay_calc_en_pre = Ph_delay_calc_en_cur;
#endif
}

// ++ YE Test API for Dejudder_By_Motion
//static unsigned int DBM_table[][]={{0,0,0,0,0}, 	//dejudder
//					 {0,0,0,0,0},};	//sad_motion

#if 1
unsigned int FRC_Dejudder_By_Motion_Mapping(unsigned int dejudder_lvl)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i = 0;
	unsigned int u32_GMV_ratio[5]={0,0,0,0,0};
	static int gmv_record[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
	unsigned int u32_GMV_Value = 0;//bit31 transfer to bit xx
	int u32_DBM_dejudder=0;
	static unsigned int u32_DBM_dejudder_last=0;
	int u32_dejudder_0=0,u32_dejudder_1=0,u32_dejudder_2=0,u32_dejudder_3=0,u32_dejudder_4=0;
	int u32_gmv_0=0,u32_gmv_1=0,u32_gmv_2=0,u32_gmv_3=0,u32_gmv_4=0;
	unsigned char u1_gmv_mode;
	unsigned int u32_RB_val;//, u32_gmv_cnt_thl = 25000;
	static unsigned int u32_DBM_table[2][5]={{0,0,0,0,0}, 	//dejudder
					 			     {0,0,0,0,0},};	//gmv_motion
////////////////////////////start
	static unsigned int u32_gmv_cnt_pre = 0;

	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>2;
	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>2;
	unsigned int u32_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;

	unsigned short GMV_avg = 0;
	unsigned short GMV = _ABS_(u11_gmv_mvx) > _ABS_(u10_gmv_mvy)? _ABS_(u11_gmv_mvx) : _ABS_(u10_gmv_mvy);
///////////////////////////end

	// gmv mode
	ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_1C_reg, 2, 2, &u32_RB_val);
	u1_gmv_mode = u32_RB_val;
#if 1
	for (i = 0; i < 31; i ++)
	{
		gmv_record[i] = gmv_record[i+1];
		GMV_avg += gmv_record[i];
	}
	gmv_record[31] = GMV;
	GMV_avg += gmv_record[31];
	GMV_avg = (GMV_avg>>5);
#endif
	//rtd_pr_memc_info("(%d)gmv_record = %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d..(%d,%d,%d)\n",
		//GMV_avg,gmv_record[0],gmv_record[1],gmv_record[2],gmv_record[3],gmv_record[4],gmv_record[5],gmv_record[6],gmv_record[7],gmv_record[8],gmv_record[9],
		//gmv_record[10],gmv_record[11],gmv_record[12],gmv_record[13],gmv_record[14],gmv_record[15],gmv_record[16],gmv_record[17],gmv_record[18],gmv_record[19],
		//gmv_record[20],gmv_record[21],gmv_record[22],gmv_record[23],gmv_record[24],gmv_record[25],gmv_record[26],gmv_record[27],gmv_record[28],gmv_record[29],
		//gmv_record[30],gmv_record[31],
		//u32_gmv_cnt,u11_gmv_mvx,u10_gmv_mvy);

//////////////

	u32_GMV_Value = GMV_avg;
	if(u32_GMV_Value>=0xFFFF)
		u32_GMV_Value=0xFFFF;

	//++ YE Test u1_DejudderByMotion_en
	//unsigned int u32_DBM_en =0; //DejudderByMotion
	//rtd_pr_memc_notice("memcMode=%d\n",s_pContext->_external_data.u3_memcMode);
	//rtd_pr_memc_notice("==[DBM] u32_SAD_Value_temp=%x  u32_SAD_Value=%x \n",u32_SAD_Value_temp,u32_SAD_Value);
	ReadRegister(SOFTWARE1_SOFTWARE1_45_reg, 24, 31,    &u32_dejudder_0);		// YE Test for dejudder_0
	ReadRegister(SOFTWARE1_SOFTWARE1_46_reg, 0, 7,    &u32_dejudder_1);		// YE Test for dejudder_1
	ReadRegister(SOFTWARE1_SOFTWARE1_46_reg, 8, 15,  &u32_dejudder_2);		// YE Test for dejudder_2
	ReadRegister(SOFTWARE1_SOFTWARE1_46_reg, 16, 23, &u32_dejudder_3);	// YE Test for dejudder_3
	ReadRegister(SOFTWARE1_SOFTWARE1_46_reg, 24, 31, &u32_dejudder_4);	// YE Test for dejudder_4

	ReadRegister(SOFTWARE1_SOFTWARE1_58_reg, 0, 15, &u32_gmv_0);
	ReadRegister(SOFTWARE1_SOFTWARE1_58_reg, 16, 31, &u32_gmv_1);
	ReadRegister(SOFTWARE1_SOFTWARE1_59_reg, 0, 10, &u32_gmv_2);
	ReadRegister(SOFTWARE1_SOFTWARE1_59_reg, 11, 20, &u32_gmv_3);
	ReadRegister(SOFTWARE1_SOFTWARE1_59_reg, 21, 30,&u32_gmv_4);


	//read from dummy register
	u32_DBM_table[0][0]  =(u32_dejudder_0)&0x000000FF; 		//dejuddefr for LG control point 1;
	u32_DBM_table[0][1] = (u32_dejudder_1)&0x000000FF; 		//dejuddefr for LG control point 1
	u32_DBM_table[0][2] = (u32_dejudder_2)&0x000000FF; 		//dejuddefr for LG control point 2
	u32_DBM_table[0][3] = (u32_dejudder_3)&0x000000FF; 		//dejuddefr for LG control point 3
	u32_DBM_table[0][4] = (u32_dejudder_4)&0x000000FF; 		//dejuddefr for LG control point 4
	//u32_DBM_table[0][5] = u32_dejudder_Value_Max;
	u32_DBM_table[1][0]  =(u32_gmv_0)&0x0000FFFF;			//gmv for for LG control point 1
	u32_DBM_table[1][1] = (u32_gmv_1)&0x0000FFFF;			//gmv for for LG control point 1
	u32_DBM_table[1][2] = (u32_gmv_2)&0x0000FFFF;			//gmv for for LG control point 2
	u32_DBM_table[1][3] = (u32_gmv_3)&0x0000FFFF;			//gmv for for LG control point 3
	u32_DBM_table[1][4] = (u32_gmv_4)&0x0000FFFF;			//gmv for for LG control point 4
	//u32_DBM_table[1][5] = u32_GMV_Value_Max;

	u32_dejudder_0  =u32_DBM_table[0][0];					//dejuddefr for LG control point 1;
	u32_dejudder_1  =u32_DBM_table[0][1];		//dejuddefr for LG control point 1
	u32_dejudder_2  =u32_DBM_table[0][2];		//dejuddefr for LG control point 2
	u32_dejudder_3  =u32_DBM_table[0][3];		//dejuddefr for LG control point 3
	u32_dejudder_4  =u32_DBM_table[0][4];		//dejuddefr for LG control point 4
	//u32_DBM_table[0][5] = u32_dejudder_Value_Max;
	u32_gmv_0  =  u32_DBM_table[1][0];			//gmv for for LG control point 1
	u32_gmv_1  =  u32_DBM_table[1][1];			//gmv for for LG control point 1
	u32_gmv_2  =  u32_DBM_table[1][2];			//gmv for for LG control point 2
	u32_gmv_3  =  u32_DBM_table[1][3];			//gmv for for LG control point 3
	u32_gmv_4  =  u32_DBM_table[1][4];			//gmv for for LG control point 4

	if(u32_GMV_Value<u32_gmv_0)
	{
		u32_DBM_dejudder = u32_dejudder_0;
	}
	else if(((u32_GMV_Value>=u32_gmv_0))&&(u32_GMV_Value<u32_gmv_1))
	{  					//gmv range
		if(((u32_gmv_1)-(u32_gmv_0))>0){
		   	if((u32_dejudder_1-u32_dejudder_0)>0)
			{  //positive slope
		   		u32_GMV_ratio[1]=((u32_GMV_Value-(u32_gmv_0))*100)/((u32_gmv_1)-(u32_gmv_0)); //GMV ratio
		   		u32_DBM_dejudder= u32_dejudder_0+((u32_dejudder_1-u32_dejudder_0)*u32_GMV_ratio[1])/100; 						//Real dejudder output
		   	}
			else if((u32_dejudder_1-u32_dejudder_0)<0)
			{ //negative slope
				u32_GMV_ratio[1]=((u32_GMV_Value-(u32_gmv_0))*100)/((u32_gmv_1)-(u32_gmv_0)); //GMV ratio
		   		u32_DBM_dejudder= u32_dejudder_0-((u32_dejudder_0-u32_dejudder_1)*u32_GMV_ratio[1])/100;					//Real dejudder output
			}
			else
			{
				u32_DBM_dejudder= u32_dejudder_1;
			}
		}
		else
		{ //it meas 2 poionts are the same
		     u32_DBM_dejudder= u32_dejudder_1;				  //Real dejudder output
		}
	}
	else if((u32_GMV_Value>=u32_gmv_1)&&(u32_GMV_Value<u32_gmv_2))
	{
		if(((u32_gmv_2)-(u32_gmv_1))>0){
		   	if((u32_dejudder_2-u32_dejudder_1)>0)
			{  //positive slope
		   		u32_GMV_ratio[2]=((u32_GMV_Value-(u32_gmv_1))*100)/((u32_gmv_2)-(u32_gmv_1)); //GMV ratio
		   		u32_DBM_dejudder= u32_dejudder_1+((u32_dejudder_2-u32_dejudder_1)*u32_GMV_ratio[2])/100; 						//Real dejudder output
		   	}
			else if((u32_dejudder_2-u32_dejudder_1)<0)
			{ //negative slope
				u32_GMV_ratio[2]=((u32_GMV_Value-(u32_gmv_1))*100)/((u32_gmv_2)-(u32_gmv_1)); //GMV ratio
		   		u32_DBM_dejudder= u32_dejudder_1-((u32_dejudder_1-u32_dejudder_2)*u32_GMV_ratio[2])/100; 						//Real dejudder output
			}
			else
			{
				u32_DBM_dejudder= u32_dejudder_2;
			}
		}
		else { //it meas 2 poionts are the same
		u32_DBM_dejudder= u32_dejudder_2;				  //Real dejudder output
		}
	}
	else if((u32_GMV_Value>=u32_gmv_2)&&(u32_GMV_Value<u32_gmv_3))
	{
		if(((u32_gmv_3)-(u32_gmv_2))>0)
		{
		   	if((u32_dejudder_3-u32_dejudder_2)>0)
			{  //positive slope
		   		u32_GMV_ratio[3]=((u32_GMV_Value-(u32_gmv_2))*100)/((u32_gmv_3)-(u32_gmv_2)); //GMV ratio
		   		u32_DBM_dejudder= u32_dejudder_2+((u32_dejudder_3-u32_dejudder_2)*u32_GMV_ratio[3])/100; 						//Real dejudder output
		   	}
			else if((u32_dejudder_3-u32_dejudder_2)<0)
			{ //negative slope
				u32_GMV_ratio[3]=((u32_GMV_Value-(u32_gmv_2))*100)/((u32_gmv_3)-(u32_gmv_2)); //GMV ratio
		   		u32_DBM_dejudder= u32_dejudder_2-((u32_dejudder_2-u32_dejudder_3)*u32_GMV_ratio[3])/100; 						//Real dejudder output
			}
			else
			{
				u32_DBM_dejudder= u32_dejudder_3;
			}
		}
		else { //it meas 2 poionts are the same
		u32_DBM_dejudder= u32_dejudder_3;				  //Real dejudder output
		}
	}
	else if((u32_GMV_Value>=u32_gmv_3)&&(u32_GMV_Value<u32_gmv_4))
	{
		if(((u32_gmv_4)-(u32_gmv_3))>0){
		   	if((u32_dejudder_4-u32_dejudder_3)>0)
			{  //positive slope
		   		u32_GMV_ratio[4]=((u32_GMV_Value-(u32_gmv_3))*100)/((u32_gmv_4)-(u32_gmv_3)); //GMV ratio
		   		u32_DBM_dejudder= u32_dejudder_3+((u32_dejudder_4-u32_dejudder_3)*u32_GMV_ratio[4])/100; 						//Real dejudder output
		   	}
			else if((u32_dejudder_4-u32_dejudder_3)<0)
			{ //negative slope
				u32_GMV_ratio[4]=((u32_GMV_Value-(u32_gmv_3))*100)/((u32_gmv_4)-(u32_gmv_3)); //GMV ratio
		   		u32_DBM_dejudder= u32_dejudder_3-((u32_dejudder_3-u32_dejudder_4)*u32_GMV_ratio[4])/100; 						//Real dejudder output
			}
			else
			{
				u32_DBM_dejudder= u32_dejudder_4;
			}
		}
		else { //it meas 2 poionts are the same
		u32_DBM_dejudder= u32_dejudder_4;				  //Real dejudder output
		}
	}
	else if(u32_GMV_Value>=u32_gmv_4)
	{
		u32_DBM_dejudder = u32_dejudder_4;
	}
	else
	{
		u32_DBM_dejudder=u32_DBM_dejudder_last;
	}

	if(u32_DBM_dejudder>=50)
		u32_DBM_dejudder=50;
	//recored last dejudder output
	u32_DBM_dejudder_last=u32_DBM_dejudder;
	u32_gmv_cnt_pre = u32_gmv_cnt;

	//adjust range
	dejudder_lvl = dejudder_lvl + ((u32_DBM_dejudder - 25)>0?(u32_DBM_dejudder - 25):0);
	dejudder_lvl = dejudder_lvl > 128? 128:dejudder_lvl;

	//rtd_pr_memc_notice("memcMode=%d\n",s_pContext->_external_data.u3_memcMode);


	//rtd_pr_memc_notice("==[DBM] Dejudder table  0=%x       1=%x       2=%x        3=%x        4=%x            \n",u32_dejudder_0,u32_dejudder_1,u32_dejudder_2,u32_dejudder_3,u32_dejudder_4);
	//rtd_pr_memc_notice("==[DBM] SAD table         0=%x       1=%x       2=%x        3=%x        4=%x           \n",u32_gmv_0,u32_gmv_1,u32_gmv_2,u32_gmv_3,u32_gmv_4);
	//rtd_pr_memc_notice("==[DBM] SAD_ratio         1=%d       2=%d      3=%d        4=%d      \n",u32_SAD_ratio[1],u32_SAD_ratio[2],u32_SAD_ratio[3],u32_SAD_ratio[4]);
	//rtd_pr_memc_notice("==[DBM] DBM_dejudder    =%d \n",u32_DBM_dejudder);
	WriteRegister(SOFTWARE1_SOFTWARE1_45_reg, 16, 23, dejudder_lvl);
	//rtd_pr_memc_info("%d, dejudder_lvl = %d\n",u32_DBM_dejudder,dejudder_lvl);
	//debug print

	return dejudder_lvl ;


}
#endif
// -- YE Test API for Dejudder_By_Motion

VOID FRC_phTable_set_MC_8_buffer(VOID)
{
	#if 0
	kphase_kphase_10_RBUS kphase_kphase_10_reg;
	kphase_kphase_10_reg.regValue = rtd_inl(KPHASE_kphase_10_reg);
	
	//MC bsize
	#if CONFIG_MC_8_BUFFER
	kphase_kphase_10_reg.kphase_in_mc_index_bsize = 8;
	#else
	kphase_kphase_10_reg.kphase_in_mc_index_bsize = 6;
	#endif

	rtd_outl(KPHASE_kphase_10_reg, kphase_kphase_10_reg.regValue);

	const _PQLCONTEXT     *s_pContext = GetPQLContext();
	PQL_OUTPUT_FRAME_RATE  out_fmRate = s_pContext->_external_data._output_frameRate;

	if(out_fmRate <= _PQL_OUT_60HZ){
		//06_m5_pqc_frame_mode_420_7bit
		//db off
		rtd_maskl(PQC_PQDC_MC_PQ_CMP_DB_CTRL_reg, 0x00000000, 0x00000000);
		rtd_maskl(PQC_PQDC_MC_PQ_DECMP_DB_CTRL_reg, 0x00000000, 0x00000000);

		//LFL : bitwidht: 9bit
		//MC_LFL_PQ_CMP_PAIR
		rtd_maskl(0xb8099204,0xffffff80, 0x01100055);//pqc
		rtd_maskl(0xb8099804,0xffffff80, 0x01100055);//pqdc
		//MC_LFL_PQ_CMP_BIT
		rtd_maskl(0xb8099208,0xffff0000, 0x00000c09);//d24

		//HF : bitwidht: 6bit
		//MC_LFH_PQ_CMP_PAIR
		rtd_maskl(0xb8099284,0xffffff80, 0x0098011b);//pqc
		rtd_maskl(0xb8099884,0xffffff80, 0x0098011b);//pqdc
		//MC_LFH_PQ_CMP_BIT
		rtd_maskl(0xb8099288,0xffff0000, 0x00000805);//d24

		//dma lfl
		rtd_maskl(0xb8099410, 0x00000000, 0x00093c04);
		rtd_maskl(0xb809ae10, 0x00000000, 0x00093c04);
		rtd_maskl(0xb809ae50, 0x00000000, 0x00093c04);
		//dma hf                                      
		rtd_maskl(0xb8099480, 0x00000000, 0x00053c04);
		rtd_maskl(0xb809ae30, 0x00000000, 0x00053c04);
		rtd_maskl(0xb809ae70, 0x00000000, 0x00053c04);
	}

	if(out_fmRate > _PQL_OUT_60HZ){
		//o------ ME start address ------o
		rtd_outl(0xb809C000, 0x46401060);
		rtd_outl(0xb809C004, 0x4647F960);
		rtd_outl(0xb809C008, 0x464FE260);
		rtd_outl(0xb809C00C, 0x4657CB60);
		rtd_outl(0xb809C010, 0x465FB460);
		rtd_outl(0xb809C014, 0x46679D60);
		rtd_outl(0xb809C27C, 0x466F8660);
		rtd_outl(0xb809C280, 0x46776F60);

		rtd_outl(0xb809C100, 0x467F5860);
		rtd_outl(0xb809C104, 0x467F5860);

		rtd_outl(0xb809C124, 0x46874160);
		rtd_outl(0xb809C128, 0x46874160);

		rtd_outl(0xb809C148, 0x468B35E0);
		rtd_outl(0xb809C14C, 0x468B35E0);

		rtd_outl(0xb809C16C, 0x468CCAE0);
		rtd_outl(0xb809C170, 0x468CCAE0);

		rtd_outl(0xb809C200, 0x468D3020);

		rtd_outl(0xb809C214, 0x4692B9A0);

		rtd_outl(0xb809C22C, 0x46954BC0);
		rtd_outl(0xb809C230, 0x469778A0);
		rtd_outl(0xb809C0D8, 0x4699A580);

		rtd_outl(0xb809C248, 0x469BD260);
		rtd_outl(0xb809C24C, 0x469FC6E0);
		//o------ MC start address ------o
		//o------ LF ------o
		rtd_outl(0xb8099414, 0x46A3BB60);
		rtd_outl(0xb8099418, 0x46EE1660);
		rtd_outl(0xb809941C, 0x47387160);
		rtd_outl(0xb8099420, 0x4782CC60);
		rtd_outl(0xb8099424, 0x47CD2760);
		rtd_outl(0xb8099428, 0x48178260);
		rtd_outl(0xb809942C, 0x4861DD60);
		rtd_outl(0xb8099430, 0x48AC3860);
		//o------ HF ------o
		rtd_outl(0xb8099484, 0x48F69360);
		rtd_outl(0xb8099488, 0x49214A60);
		rtd_outl(0xb809948C, 0x494C0160);
		rtd_outl(0xb8099490, 0x4976B860);
		rtd_outl(0xb8099494, 0x49A16F60);
		rtd_outl(0xb8099498, 0x49CC2660);
		rtd_outl(0xb809949C, 0x49F6DD60);
		rtd_outl(0xb80994A0, 0x4A219460);

		rtd_outl(0xb809B70C, 0x4A4C4B60);
		rtd_outl(0xb809B710, 0x4A4CB0A0);

		rtd_outl(0xb809B728, 0x4A4D15E0);
		rtd_outl(0xb809B72C, 0x4A4D7B20);
		rtd_outl(0xb809B730, 0x4A4DE060);

		rtd_outl(0xb809B74C, 0x4A4E45A0);
		rtd_outl(0xb809B750, 0x4A4E7840);
		rtd_outl(0xb809B754, 0x4A4EAAE0);
		rtd_outl(0xb809B758, 0x4A4EDD80);
		rtd_outl(0xb809B75C, 0x4A4F1020);
		rtd_outl(0xb809B760, 0x4A4F42C0);
		rtd_outl(0xb809B764, 0x4A4F7560);
		rtd_outl(0xb809B768, 0x4A4FA800);

		//o------ ME end address ------o
		rtd_outl(0xb809C018, 0x4647F960);
		rtd_outl(0xb809C01C, 0x464FE260);
		rtd_outl(0xb809C020, 0x4657CB60);
		rtd_outl(0xb809C024, 0x465FB460);
		rtd_outl(0xb809C028, 0x46679D60);
		rtd_outl(0xb809C02C, 0x466F8660);
		rtd_outl(0xb809C284, 0x46776F60);
		rtd_outl(0xb809C288, 0x467F5860);

		rtd_outl(0xb809C108, 0x46874160);
		rtd_outl(0xb809C10c, 0x46874160);

		rtd_outl(0xb809C12c, 0x468B35E0);
		rtd_outl(0xb809C130, 0x468B35E0);

		rtd_outl(0xb809C150, 0x468CCAE0);
		rtd_outl(0xb809C154, 0x468CCAE0);

		rtd_outl(0xb809C174, 0x468D3020);
		rtd_outl(0xb809C178, 0x468D3020);

		rtd_outl(0xb809C204, 0x4692B9A0);

		rtd_outl(0xb809C218, 0x46954BC0);

		rtd_outl(0xb809C234, 0x469778A0);
		rtd_outl(0xb809C238, 0x4699A580);
		rtd_outl(0xb809C0DC, 0x469BD260);

		rtd_outl(0xb809C250, 0x469FC6E0);
		rtd_outl(0xb809C254, 0x46A3BB60);
		//o------ MC end address ------o
		//o------ LF ------o
		rtd_outl(0xb8099434, 0x46EE1660);
		rtd_outl(0xb8099438, 0x47387160);
		rtd_outl(0xb809943c, 0x4782CC60);
		rtd_outl(0xb8099440, 0x47CD2760);
		rtd_outl(0xb8099444, 0x48178260);
		rtd_outl(0xb8099448, 0x4861DD60);
		rtd_outl(0xb809944c, 0x48AC3860);
		rtd_outl(0xb8099450, 0x48F69360);
		//o------ HF ------o
		rtd_outl(0xb80994A4, 0x49214A60);
		rtd_outl(0xb80994A8, 0x494C0160);
		rtd_outl(0xb80994Ac, 0x4976B860);
		rtd_outl(0xb80994b0, 0x49A16F60);
		rtd_outl(0xb80994b4, 0x49CC2660);
		rtd_outl(0xb80994b8, 0x49F6DD60);
		rtd_outl(0xb80994bc, 0x4A219460);
		rtd_outl(0xb80994c0, 0x4A4C4B60);

		rtd_outl(0xb809B714, 0x4A4CB0A0);
		rtd_outl(0xb809B718, 0x4A4D15E0);

		rtd_outl(0xb809B734, 0x4A4D7B20);
		rtd_outl(0xb809B738, 0x4A4DE060);
		rtd_outl(0xb809B73c, 0x4A4E45A0);

		rtd_outl(0xb809B76c, 0x4A4E7840);
		rtd_outl(0xb809B770, 0x4A4EAAE0);
		rtd_outl(0xb809B774, 0x4A4EDD80);
		rtd_outl(0xb809B778, 0x4A4F1020);
		rtd_outl(0xb809B77c, 0x4A4F42C0);
		rtd_outl(0xb809B780, 0x4A4F7560);
		rtd_outl(0xb809B784, 0x4A4FA800);
		rtd_outl(0xb809B788, 0x4A4FDAA0);
	}
	else{
		//o------ ME start address ------o
		rtd_outl(0xb809C000, 0x44400060);
		rtd_outl(0xb809C004, 0x4447E960);
		rtd_outl(0xb809C008, 0x444FD260);
		rtd_outl(0xb809C00C, 0x4457BB60);
		rtd_outl(0xb809C010, 0x445FA460);
		rtd_outl(0xb809C014, 0x44678D60);
		rtd_outl(0xb809C27C, 0x446F7660);
		rtd_outl(0xb809C280, 0x44775F60);

		rtd_outl(0xb809C100, 0x447F4860);
		rtd_outl(0xb809C104, 0x447F4860);

		rtd_outl(0xb809C124, 0x44873160);
		rtd_outl(0xb809C128, 0x44873160);

		rtd_outl(0xb809C148, 0x448B25E0);
		rtd_outl(0xb809C14C, 0x448B25E0);

		rtd_outl(0xb809C16C, 0x448CBAE0);
		rtd_outl(0xb809C170, 0x448CBAE0);

		rtd_outl(0xb809C200, 0x448D2020);

		rtd_outl(0xb809C214, 0x4492A9A0);

		rtd_outl(0xb809C22C, 0x44953BC0);
		rtd_outl(0xb809C230, 0x449768A0);
		rtd_outl(0xb809C0D8, 0x44999580);

		rtd_outl(0xb809C248, 0x449BC260);
		rtd_outl(0xb809C24C, 0x449FB6E0);
		//o------ MC start address ------o
		//o------ LF ------o
		rtd_outl(0xb8099414, 0x44A3AB60);
		rtd_outl(0xb8099418, 0x44EE0660);
		rtd_outl(0xb809941C, 0x45386160);
		rtd_outl(0xb8099420, 0x4582BC60);
		rtd_outl(0xb8099424, 0x45CD1760);
		rtd_outl(0xb8099428, 0x46177260);
		rtd_outl(0xb809942C, 0x4661CD60);
		rtd_outl(0xb8099430, 0x46AC2860);
		//o------ HF ------o
		rtd_outl(0xb8099484, 0x46F68360);
		rtd_outl(0xb8099488, 0x47213A60);
		rtd_outl(0xb809948C, 0x474BF160);
		rtd_outl(0xb8099490, 0x4776A860);
		rtd_outl(0xb8099494, 0x47A15F60);
		rtd_outl(0xb8099498, 0x47CC1660);
		rtd_outl(0xb809949C, 0x47F6CD60);
		rtd_outl(0xb80994A0, 0x48218460);

		rtd_outl(0xb809B70C, 0x484C3B60);
		rtd_outl(0xb809B710, 0x484CA0A0);

		rtd_outl(0xb809B728, 0x484D05E0);
		rtd_outl(0xb809B72C, 0x484D6B20);
		rtd_outl(0xb809B730, 0x484DD060);

		rtd_outl(0xb809B74C, 0x484E35A0);
		rtd_outl(0xb809B750, 0x484E6840);
		rtd_outl(0xb809B754, 0x484E9AE0);
		rtd_outl(0xb809B758, 0x484ECD80);
		rtd_outl(0xb809B75C, 0x484F0020);
		rtd_outl(0xb809B760, 0x484F32C0);
		rtd_outl(0xb809B764, 0x484F6560);
		rtd_outl(0xb809B768, 0x484F9800);

		//o------ ME end address ------o
		rtd_outl(0xb809C018, 0x4447E960);
		rtd_outl(0xb809C01C, 0x444FD260);
		rtd_outl(0xb809C020, 0x4457BB60);
		rtd_outl(0xb809C024, 0x445FA460);
		rtd_outl(0xb809C028, 0x44678D60);
		rtd_outl(0xb809C02C, 0x446F7660);
		rtd_outl(0xb809C284, 0x44775F60);
		rtd_outl(0xb809C288, 0x447F4860);

		rtd_outl(0xb809C108, 0x44873160);
		rtd_outl(0xb809C10c, 0x44873160);

		rtd_outl(0xb809C12c, 0x448B25E0);
		rtd_outl(0xb809C130, 0x448B25E0);

		rtd_outl(0xb809C150, 0x448CBAE0);
		rtd_outl(0xb809C154, 0x448CBAE0);

		rtd_outl(0xb809C174, 0x448D2020);
		rtd_outl(0xb809C178, 0x448D2020);

		rtd_outl(0xb809C204, 0x4492A9A0);

		rtd_outl(0xb809C218, 0x44953BC0);

		rtd_outl(0xb809C234, 0x449768A0);
		rtd_outl(0xb809C238, 0x44999580);
		rtd_outl(0xb809C0DC, 0x449BC260);

		rtd_outl(0xb809C250, 0x449FB6E0);
		rtd_outl(0xb809C254, 0x44A3AB60);
		//o------ MC end address ------o
		//o------ LF ------o
		rtd_outl(0xb8099434, 0x44EE0660);
		rtd_outl(0xb8099438, 0x45386160);
		rtd_outl(0xb809943c, 0x4582BC60);
		rtd_outl(0xb8099440, 0x45CD1760);
		rtd_outl(0xb8099444, 0x46177260);
		rtd_outl(0xb8099448, 0x4661CD60);
		rtd_outl(0xb809944c, 0x46AC2860);
		rtd_outl(0xb8099450, 0x46F68360);
		//o------ HF ------o
		rtd_outl(0xb80994A4, 0x47213A60);
		rtd_outl(0xb80994A8, 0x474BF160);
		rtd_outl(0xb80994Ac, 0x4776A860);
		rtd_outl(0xb80994b0, 0x47A15F60);
		rtd_outl(0xb80994b4, 0x47CC1660);
		rtd_outl(0xb80994b8, 0x47F6CD60);
		rtd_outl(0xb80994bc, 0x48218460);
		rtd_outl(0xb80994c0, 0x484C3B60);

		rtd_outl(0xb809B714, 0x484CA0A0);
		rtd_outl(0xb809B718, 0x484D05E0);

		rtd_outl(0xb809B734, 0x484D6B20);
		rtd_outl(0xb809B738, 0x484DD060);
		rtd_outl(0xb809B73c, 0x484E35A0);

		rtd_outl(0xb809B76c, 0x484E6840);
		rtd_outl(0xb809B770, 0x484E9AE0);
		rtd_outl(0xb809B774, 0x484ECD80);
		rtd_outl(0xb809B778, 0x484F0020);
		rtd_outl(0xb809B77c, 0x484F32C0);
		rtd_outl(0xb809B780, 0x484F6560);
		rtd_outl(0xb809B784, 0x484F9800);
		rtd_outl(0xb809B788, 0x484FCAA0);
	}
	#endif
}

VOID FRC_phTable_set_LineModeCtrl(VOID)
{
	const _PQLCONTEXT     *s_pContext = GetPQLContext();
	PQL_OUTPUT_FRAME_RATE  out_fmRate = s_pContext->_external_data._output_frameRate;
	PQL_INPUT_FRAME_RATE	in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char u1_isLowDelayMode = (LowDelay_mode == TRUE) ? 1 : 0;
	unsigned char u1_adaptivestream_flag = MEMC_Lib_get_Adaptive_Stream_Flag();
	static unsigned char status = 0, LbmcTrig_status = 0;
	//unsigned char u1_direct_media_flag = get_scaler_run_cloud_game();
	unsigned int log_en = 0;
	unsigned char u1_vdec_direct_lowdelay_flag = drv_memory_get_vdec_direct_low_latency_mode();
	unsigned int force_line_mode_en = 0, force_line_mode_type = 0;
	unsigned char u1_callback_subscibe = 0;//vsc_get_main_win_apply_done_event_subscribe();
	unsigned char u1_direct_media_flag = 0;//get_scaler_run_cloud_game();

	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 25, 25, &log_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 13, 13, &force_line_mode_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 14, 14, &force_line_mode_type);
			
	if(log_en){
		rtd_pr_memc_notice("[%s][u1_isLowDelayMode,%d,][u1_adaptivestream_flag,%d,][u1_vdec_direct_lowdelay_flag,%d,][MEMC_Lib_get_DisplayMode_Src,%d,][Vpq_LowDelay_to_MEMC,%d,]\n", 
		__FUNCTION__, u1_isLowDelayMode, u1_adaptivestream_flag,u1_vdec_direct_lowdelay_flag, MEMC_Lib_get_DisplayMode_Src(), Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag());
	}

	if(((in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_120HZ)) || ((in_fmRate == _PQL_IN_50HZ) && (out_fmRate == _PQL_OUT_100HZ))){
		//if((u1_adaptivestream_flag == 1 && (u1_callback_subscibe == 1 || u1_direct_media_flag == 1)) || (u1_isLowDelayMode == 1 && MEMC_Lib_get_DisplayMode_Src() == 2/*VSC_INPUTSRC_HDMI*/ && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1)){
		//if((u1_isLowDelayMode == 1 && (u1_adaptivestream_flag == 1 || (MEMC_Lib_get_DisplayMode_Src() == 2/*VSC_INPUTSRC_HDMI*/ && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1)))){
		//if(u1_isLowDelayMode == 1){
		if((u1_isLowDelayMode == 1 && ((u1_adaptivestream_flag == 1 && (u1_callback_subscibe == 1 || u1_direct_media_flag == 1)) || (MEMC_Lib_get_DisplayMode_Src() == 2 && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1)))){
			if(status != 1){
				MEMC_Lib_set_LineMode_flag(1);
				status = 1;
			}
		}
		else{
			if(status != 0){
				MEMC_Lib_set_LineMode_flag(0);
				status = 0;
			}
		}
		if(LbmcTrig_status != 0){
			MEMC_Lib_set_LbmcTrig_flag(0);
			LbmcTrig_status = 0;
		}
	}
	else if(((in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_60HZ)) || ((in_fmRate == _PQL_IN_50HZ) && (out_fmRate == _PQL_OUT_50HZ))){
		//if(u1_isLowDelayMode == 1){
		if(u1_isLowDelayMode == 1 && ((u1_adaptivestream_flag == 1 && (u1_callback_subscibe == 1 /*|| u1_direct_media_flag == 1*/)) || (u1_vdec_direct_lowdelay_flag)) ){
			if(status != 1){
				MEMC_Lib_set_LineMode_flag(1);
				status = 1;
			}
			if((in_fmRate == _PQL_IN_50HZ) && (out_fmRate == _PQL_OUT_50HZ)){
				if(LbmcTrig_status != 1){
					MEMC_Lib_set_LbmcTrig_flag(1);
					LbmcTrig_status = 1;
				}
			}
			else{
				if(LbmcTrig_status != 0){
					MEMC_Lib_set_LbmcTrig_flag(0);
					LbmcTrig_status = 0;
				}
			}
		}
		else{
			if(status != 0){
				MEMC_Lib_set_LineMode_flag(0);
				status = 0;
			}
			if(LbmcTrig_status != 0){
				MEMC_Lib_set_LbmcTrig_flag(0);
				LbmcTrig_status = 0;
			}
		}
	}
	else{
		if(status != 0){
			MEMC_Lib_set_LineMode_flag(0);
			status = 0;
		}
		if(LbmcTrig_status != 0){
			MEMC_Lib_set_LbmcTrig_flag(0);
			LbmcTrig_status = 0;
		}
	}

	//-----MEMC debug mode-----//
	if(force_line_mode_en){ // force lowdelay
		if(force_line_mode_type){
			MEMC_Lib_set_LineMode_flag(1);
		}
		else{
			MEMC_Lib_set_LineMode_flag(0);
		}
	}
	//-----MEMC debug mode-----//
}

VOID FRC_phTable_set_NewLowDelay(unsigned char u1_enable)
{
	static unsigned char status = 0;
	if(u1_enable){
		if(status == 0){
			WriteRegister(HARDWARE_HARDWARE_06_reg, 2, 2, 0);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_30_reg, 0, 4, 0);
			WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_34_reg, 3, 7, 0);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 0);
			status = 1;
		}
	}
	else{
		if(status == 1){
			WriteRegister(HARDWARE_HARDWARE_06_reg, 2, 2, 1);
			WriteRegister(KME_DEHALO_KME_DEHALO_10_reg, 0, 0, 1);
			status = 0;
		}
	}

}

unsigned int FRC_phTable_UltraLowDelayMode_Target(VOID)
{
	unsigned char ret = 0;
	unsigned int log_en = 0;

	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 25, 25, &log_en);

	if(log_en){
		rtd_pr_memc_notice("[%s][%d,%d][8644,%x,][d120,%x,][d198,%x,]\n", __FUNCTION__, Get_DISPLAY_PANEL_OLED_TYPE(), Scaler_check_orbit_store_mode(),
			rtd_inl(PPOVERLAY_MEMCDTG_CONTROL3_reg), rtd_inl(CRTC1_CRTC1_20_reg), rtd_inl(CRTC1_CRTC1_98_reg));
	}

	if(Get_DISPLAY_PANEL_OLED_TYPE()==TRUE /*&& Scaler_check_orbit_store_mode() == TRUE*/){
		ret = 0x2a;
	}
	else{
		ret = 0xa;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
extern unsigned char N2M_memcdtg_dv_total_check;
extern void Scaler_MEMC_Set_BypassToEnable_Flag(unsigned char u1_enable);
VOID FRC_phTable_StateMachine(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput)
{
	
	// current frame signal read back.
	const _PQLCONTEXT     *s_pContext = GetPQLContext();
	unsigned char                  u1_in3D_LR = s_pContext->_output_read_comreg.u1_kphase_inLR;
	unsigned char                    u8_sys_N = s_pContext->_output_read_comreg.u8_sys_N_rb; // 1;
	unsigned char                    u8_sys_M = s_pContext->_output_read_comreg.u8_sys_M_rb; // 2;
	unsigned char                 u4_in_phase = s_pContext->_output_read_comreg.u4_kphase_inPhase;

	unsigned char              u8_filmPh0_cnt = s_pContext->_output_frc_phtable.u8_filmPh0_cnt;
	unsigned char                   u8_filmPh = s_pContext->_output_filmDetectctrl.u8_phT_phase_Idx_out[_FILM_ALL];   // bai  need make sure

	// value of fb lvl
	unsigned char         u8_flbk_deblur_lvl  = (1 << _PHASE_FLBK_LVL_PRECISION) - (s_pContext->_output_fblevelctrl.u16_blur_lvl >> 4);
	unsigned char         u8_flbk_dejudder_lvl = (1 << _PHASE_FLBK_LVL_PRECISION) - (s_pContext->_output_fblevelctrl.u16_judder_lvl >> 4);
	PQL_OUTPUT_FRAME_RATE  out_fmRate = s_pContext->_external_data._output_frameRate;
	PQL_INPUT_FRAME_RATE	in_fmRate = s_pContext->_external_data._input_frameRate;

	// max of fb lvl
	//dejudder&deblur can only get 5 bits for hardware limitation, we should left shift 3 bit to get 128 value
	unsigned char         u8_dejudder_dft_lvl = (pParam->u8_dejudder_lvl == 0)? (s_pContext->_external_data.u8_dejudder_lvl) : ((pParam->u8_dejudder_lvl << 3) - 1);
	unsigned char         u8_deblur_dft_lvl   = (pParam->u8_deblur_lvl   == 0)? (s_pContext->_external_data.u8_deblur_lvl)   : ((pParam->u8_deblur_lvl << 3)  - 1);

	unsigned char                  u1_outMode = (pParam->u2_outMode      == 0)?  s_pContext->_external_data._output_mode     : (pParam->u2_outMode     - 1);
	PQL_INPUT_3D_FORMAT   in_3dFormat = (pParam->in_3dFormat     == 0)?  s_pContext->_external_data._input_3dFormat  : (PQL_INPUT_3D_FORMAT)(pParam->in_3dFormat - 1);
	PQL_OUTPUT_3D_FORMAT out_3dFormat = (pParam->out_3dFormat    == 0)?  s_pContext->_external_data._output_3dFormat : (PQL_OUTPUT_3D_FORMAT)(pParam->out_3dFormat - 1);
	FRC_CADENCE_ID      u8_cadence_id = (pParam->u2_candece_id_mode  == 0) ? (FRC_CADENCE_ID)(s_pContext->_output_read_comreg.u3_ipme_filmMode_rb>>1) : \
		                                ((pParam->u2_candece_id_mode == 1) ? (FRC_CADENCE_ID)s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL] : \
										                                     (FRC_CADENCE_ID)pParam->u4_cadence_id) ;

	unsigned char		u2_isPreCadSupport = PQL_FRCCaseSupportJudge(pOutput->u8_film_cadence_id_pre);
	unsigned char u8_film_cadence_id = (u8_cadence_id != _CAD_VIDEO)? u8_cadence_id : ((u2_isPreCadSupport==CAD_T)? pOutput->u8_film_cadence_id_pre : _CAD_22);

	unsigned char u8_cadence_outLen = 0,u8_dejudder_lvl = 0,u8_deblur_lvl = 0,u8_pulldown_mode = 0,u1_is_inFormat_PureVideo = 0, u1_adaptivestream_flag = 0, u1_direct_media_flag = 0, u1_vdec_direct_lowdelay_flag = 0;
	BOOL u1_osd_force_mode = 0;
	// game mode param
	unsigned short u8_source_type = s_pContext->_external_info.u16_source_type;
	//unsigned short u16_MEMCMode = s_pContext->_external_info.u16_MEMCMode; //. backup for miracast
	//unsigned char u1_isLowDelayMode = (s_pContext->_external_data.u1_lowDelayMode == TRUE) ? 1 : 0;
	unsigned char u1_isLowDelayMode = (LowDelay_mode == TRUE) ? 1 : 0;
	unsigned char u1_LowDelayMode_chg = (u1_isLowDelayMode == pOutput->u1_lowdelaystate) ? 0 : 1;
	unsigned char u1_UI_GameMode = (Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == TRUE) ? 1 : 0;
	unsigned char u1_UI_GameMode_chg = (u1_UI_GameMode == pOutput->u1_UI_GameMode_state) ? 0 : 1;
	unsigned char u1_inframe_chg = (in_fmRate == pOutput->u1_inframe_pre) ? 0 : 1;
	unsigned char u1_source_type_chg = (u8_source_type == pOutput->u8_source_type_pre) ? 0 : 1;
	unsigned char scaler_force_bg_status = PPOVERLAY_Main_Display_Control_RSV_get_m_force_bg(rtd_inl(PPOVERLAY_Main_Display_Control_RSV_reg));
	//unsigned char u1_MEMCMode_chg = (u16_MEMCMode == pOutput->u16_MEMCMode_pre) ? 0 : 1; //. backup for miracast
	unsigned char VR360_en_chg = (VR360_en == pOutput->u1_VR360_en_pre) ? 0 : 1;
	unsigned char timing_unstable_flag = (vbe_get_disp_timing_change_status() == pOutput->u1_timing_status_pre) ? 0 : 1;
	unsigned char qms_flag = (get_scaler_qms_mode_flag() == TRUE) ? 1 : 0;
	//

	// k6 BFI test
	unsigned int test_BFI_en = 0;
	//

	// ----default setting:close print log---
	unsigned int u32_value = 0;
	unsigned int u32_log_flag =0;

	static unsigned char UltraLowDelay_status = 0;
	static unsigned int NewLowDelay_en_pre = 0;
	unsigned int MC_8_buffer_en = 0, NewLowDelay_en = 0, frc_vtrig_dly = 0;
	unsigned char u1_lowdelay_state_chg_flg = 0, NewLowDelay_chg = 0;
	unsigned int log_en = 0;
	unsigned int force_lowdelay_en = 0, force_lowdelay_type = 0, force_repeat_en = 0, force_repeat_type = 0;
	unsigned int force_DMA_ctrl_en = 0, force_MC_DMA_type = 0, force_ME_DMA_type = 0;
	unsigned int force_clk_ctrl_en = 0, force_MC_clk_type = 0, force_ME_clk_type = 0;
	unsigned int force_MC_only_en = 0, force_MC_only_type = 0, force_PC_mode_en = 0, force_PC_mode_type = 0;
	static unsigned char debug_status_1 = 0, debug_status_2 = 0;
	//unsigned char memc_motion_type = Scaler_MEMC_GetMotionType();
	static unsigned char QMS_24hz_state = 0;
	unsigned int QMS_24hz_en = 0;

	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 31, 31, &log_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 0, 0, &force_lowdelay_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 1, 1, &force_lowdelay_type);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 2, 2, &force_repeat_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 3, 3, &force_repeat_type);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 4, 4, &force_DMA_ctrl_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 5, 5, &force_MC_DMA_type);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 6, 7, &force_ME_DMA_type);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 8, 8, &force_clk_ctrl_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 9, 9, &force_MC_clk_type);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 10, 10, &force_ME_clk_type);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 11, 11, &force_MC_only_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 12, 12, &force_MC_only_type);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 15, 15, &force_PC_mode_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_00_reg, 16, 16, &force_PC_mode_type);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 4, 4, &QMS_24hz_en);
	ReadRegister(SOFTWARE_SOFTWARE_45_reg, 0, 31, &u32_value);
	ReadRegister(CRTC1_CRTC1_20_reg, 20, 29, &frc_vtrig_dly);
	u32_log_flag = (u32_value ) & 0x1;

	test_BFI_en = fwif_color_get_BFI_En();
	
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 0, &MC_8_buffer_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 2, 2, &NewLowDelay_en);
	NewLowDelay_chg = (NewLowDelay_en == NewLowDelay_en_pre) ? 0 : 1;
	u1_lowdelay_state_chg_flg = lowdelay_state_chg_flg;

	if(u8_sys_N == 0 || u8_sys_M == 0){
		if(u32_log_flag==1)
		{
			rtd_pr_memc_notice("[%s]Fatal error:u8_sys_N=%d,u8_sys_M=%d!!\n",__FUNCTION__,u8_sys_N,u8_sys_M);
		}

		u8_sys_N = 1;
		u8_sys_M = 1;
	}
	if(u8_cadence_id >= _FRC_CADENCE_NUM_){
		if(u32_log_flag==1)
		{
			rtd_pr_memc_notice("[%s]Invalid u8_cadence_id=%d.Resume!\n",__FUNCTION__,u8_cadence_id);
		}

		u8_cadence_id = _CAD_VIDEO;
	}
	u8_cadence_outLen = frc_cadTable[u8_cadence_id].cad_outLen;

	//rtd_pr_memc_notice("=====1 << _PHASE_FLBK =%d  u16_judder_lvl >> 4=%d u8_flbk_dejudder_lvl=%d u \n",1 << _PHASE_FLBK_LVL_PRECISION,s_pContext->_output_fblevelctrl.u16_judder_lvl >> 4, u8_flbk_dejudder_lvl);
	//rtd_pr_memc_notice("=====u8_dejudder_lvl=%d  (s_pContext->_external_data.u8_dejudder_lvl)=%d ((pParam->u8_dejudder_lvl << 3) - 1)=%d \n",pParam->u8_dejudder_lvl ,(s_pContext->_external_data.u8_dejudder_lvl) ,((pParam->u8_dejudder_lvl << 3) - 1));

	// clip fb lvl
	u8_dejudder_lvl = _CLIP_UBOUND_(u8_flbk_dejudder_lvl, u8_dejudder_dft_lvl);
	//rtd_pr_memc_notice("=====u8_dejudder_lvl=%d u8_flbk_dejudder_lvl=%d u8_dejudder_dft_lvl=%d\n",u8_dejudder_lvl,u8_flbk_dejudder_lvl,u8_dejudder_dft_lvl);

	u8_deblur_lvl   = _CLIP_UBOUND_(u8_flbk_deblur_lvl, u8_deblur_dft_lvl);
	u8_pulldown_mode = s_pContext->_external_data.u8_pulldown_mode;
	u1_osd_force_mode = s_pContext->_external_data.u1_osd_force_mode;
	u1_is_inFormat_PureVideo = ((in_fmRate == _PQL_IN_60HZ||(in_fmRate == _PQL_IN_50HZ)) && (u8_cadence_id == _CAD_VIDEO)) ? 1 : 0;
	u1_adaptivestream_flag = MEMC_Lib_get_Adaptive_Stream_Flag();
	u1_vdec_direct_lowdelay_flag = drv_memory_get_vdec_direct_low_latency_mode();
	u1_direct_media_flag = 0;//get_scaler_run_cloud_game();

	if(u8_dejudder_lvl >  (1 << _PHASE_FLBK_LVL_PRECISION))
	{
		u8_dejudder_lvl =  (1 << _PHASE_FLBK_LVL_PRECISION);
	}

	if(u8_deblur_lvl >  (1 << _PHASE_FLBK_LVL_PRECISION))
	{
		u8_deblur_lvl =  (1 << _PHASE_FLBK_LVL_PRECISION);
	}

	if(u1_osd_force_mode == 1) // all picture mode // k6 project, Mr. Ann's requirement
	{
		if (u1_is_inFormat_PureVideo == 1)
		{
			//rtd_pr_memc_emerg("2 test_BFI_en=%d \n",test_BFI_en);
			if(test_BFI_en || u1_isLowDelayMode)
				u8_deblur_lvl = 0; // (1 << _PHASE_FLBK_LVL_PRECISION);
			else
				u8_dejudder_lvl = (1 << _PHASE_FLBK_LVL_PRECISION);
		}
		else
		{
			//rtd_pr_memc_emerg("3 test_BFI_en=%d \n",test_BFI_en);
			if(test_BFI_en || u1_isLowDelayMode)
				u8_deblur_lvl = 0; // (1 << _PHASE_FLBK_LVL_PRECISION);
			else
				u8_deblur_lvl = (1 << _PHASE_FLBK_LVL_PRECISION);
		}
	}

	if (out_fmRate <= _PQL_OUT_60HZ || out_3dFormat != _PQL_OUT_2D)
	{
		u8_deblur_lvl = (1 << _PHASE_FLBK_LVL_PRECISION);
	}


	//-----MEMC debug mode-----//
	if(force_lowdelay_en){ // force lowdelay
		if(force_lowdelay_type){
			LowDelay_mode = 1;
			Scaler_MEMC_SetMotionComp(0,0,0);//OFF
		}
		else{
			LowDelay_mode = 0;
			Scaler_MEMC_SetMotionComp(128,128,2);//SMOOTH
		}
		u1_LowDelayMode_chg = (LowDelay_mode == pOutput->u1_lowdelaystate) ? 0 : 1;
	}

	//	if(memc_motion_type == VPQ_MEMC_TYPE_MEDIUM && u1_inframe_chg == 1){
	//		Scaler_MEMC_SetMotionComp(3, 3, MEMC_TYPE_MEDIUM);
	//	}
	if(force_repeat_en){ // force repeat
		if(force_repeat_type){
			Mid_Mode_SetMEMCFrameRepeatEnable(1);
			u8_dejudder_lvl = 0;
			u8_deblur_lvl = 0;
		}
		else{
			Mid_Mode_SetMEMCFrameRepeatEnable(0);
			u8_dejudder_lvl = (1 << _PHASE_FLBK_LVL_PRECISION);
			u8_deblur_lvl = (1 << _PHASE_FLBK_LVL_PRECISION);
		}
	}
	if(force_DMA_ctrl_en){ // force ctrl MC DMA
		if(force_MC_DMA_type){
			Mid_Mode_EnableMCDMA();
		}
		else{
			Mid_Mode_DisableMCDMA();
		}
	}
	if(force_DMA_ctrl_en){ // force ctrl ME DMA
		if(force_ME_DMA_type == 2){
			Mid_Mode_EnableMEDMA();
		}
		else if(force_ME_DMA_type == 1){
			Mid_Mode_DisableMEDMA(1);
		}
		else{
			Mid_Mode_DisableMEDMA(0);
		}
	}
	if(force_clk_ctrl_en){ // force ctrl MC clk
		if(force_MC_clk_type){
			Scaler_MEMC_MC_CLK_Enable();
		}
		else{
			Scaler_MEMC_MC_CLK_Disable();
		}
	}
	if(force_clk_ctrl_en){ // force ctrl ME clk
		if(force_ME_clk_type){
			Scaler_MEMC_ME_CLK_Enable();
		}
		else{
			Scaler_MEMC_ME_CLK_Disable();
		}
	}
	if(force_MC_only_en){ // force ctrl MC only
		if(force_MC_only_type){
			if(debug_status_1 == 0){
				Scaler_MEMC_MEMC_CLK_OnOff(0, 1, 0);
				Scaler_MEMC_Set_EnableToBypass_Flag(1);
				debug_status_1 = 1;
			}
		}
		else{
			if(debug_status_1 == 1){
				Scaler_MEMC_MEMC_CLK_OnOff(1, 1, 0);
				Scaler_MEMC_Set_BypassToEnable_Flag(1);
				debug_status_1 = 0;
			}
		}
	}
	if(force_PC_mode_en){ // force ctrl PC mode
		MEMC_LibSetMEMC_PCModeEnable((BOOL)force_PC_mode_type);
		debug_status_2 = 1;
	}
	else{
		if(debug_status_2 == 1){
			MEMC_LibSetMEMC_PCModeEnable(FALSE);
			debug_status_2 = 0;
		}
	}
	//-----MEMC debug mode-----//

	//rtd_pr_memc_notice("=====u8_dejudder_lvl=%d.pOutput->u8_dejudder_lvl_pre=%d \n",u8_dejudder_lvl,pOutput->u8_dejudder_lvl_pre);

	// synthesize to generate change signal, and merged into phase table control signal
	{
		//unsigned char u1_phT_gen_nor_drive, u1_outTable_PC_wrt, u1_outTable_Video_wrt;
		unsigned char u1_n2m_switch          = (u8_sys_N == pOutput->u8_sys_N_pre && u8_sys_M == pOutput->u8_sys_M_pre)? 0 : 1;
		unsigned char u1_in3dFormat_chg      = (in_3dFormat     == pOutput->in3d_format_pre    )? 0 : 1;
		unsigned char u1_out3dFormat_chg     = (out_3dFormat    == pOutput->out3d_format_pre   )? 0 : 1;
		unsigned char u1_outMode_chg         = (u1_outMode      == pOutput->u1_outMode_pre     )? 0 : 1;
		unsigned char u1_cadence_chg         = (u8_cadence_id   == pOutput->u8_cadence_id_pre  )? 0 : 1;
		unsigned char u1_film_cadence_chg    = (u2_isPreCadSupport==CAD_T) ? ((u8_cadence_id   == pOutput->u8_film_cadence_id_pre || u8_cadence_id == _CAD_VIDEO) ? 0 : 1) :
														((pOutput->u8_film_cadence_id_pre==_CAD_22)? 0 : 1);
		unsigned char u1_dejudder_lvl_chg    = (u8_dejudder_lvl    == pOutput->u8_dejudder_lvl_pre)? 0 : 1;
		unsigned char u1_deblur_lvl_chg      = (u8_deblur_lvl      == pOutput->u8_deblur_lvl_pre  )? 0 : 1;
		unsigned char u1_pulldown_mode_chg	 = (u8_pulldown_mode == pOutput->u8_pulldown_mode_pre)? 0 : 1;
		unsigned char u1_adaptivestream_flag_chg	 = (u1_adaptivestream_flag == pOutput->u1_adaptivestream_flag_pre)? 0 : 1;
		unsigned char u1_direct_media_flag_chg	 = (u1_direct_media_flag == pOutput->u1_direct_media_flag_pre)? 0 : 1;
		unsigned char u1_vdec_direct_lowdelay_flag_chg	 = (u1_vdec_direct_lowdelay_flag == pOutput->u1_vdec_direct_lowdelay_flag_pre)? 0 : 1;
#if 0
		unsigned char u8_sysN_cadOut_comDiv  = (unsigned char)FRC_phTable_tool_commonDivisor(u8_sys_N, (in_3dFormat == _PQL_IN_LR? u8_cadence_outLen*2 : u8_cadence_outLen));
		unsigned char u1_is2D_is3DL = ((in_3dFormat != _PQL_IN_LR) ||((in_3dFormat == _PQL_IN_LR) && (u1_in3D_LR == _PQL_3D_FRAME_L)));
		unsigned char u8_filmPh0_cnt_cur     =  u1_cadence_chg? 0 : ((u1_is2D_is3DL && u8_filmPh == 0)? (u8_filmPh0_cnt + 1) % u8_sys_N : u8_filmPh0_cnt);
		unsigned char u4_in_phase_use = (u8_sys_N == 0) ? 0 : ((u4_in_phase+1)%u8_sys_N);
		unsigned char u1_filmPh0_inPh_chg    = (u1_is2D_is3DL && u8_filmPh == 0 && u8_filmPh0_cnt_cur == 0 && (u4_in_phase_use % u8_sysN_cadOut_comDiv) != pOutput->u8_filmPh0_inPh_pre)? 1 : 0;

		pOutput->u8_filmPh0_inPh_pre  = u1_filmPh0_inPh_chg? (u4_in_phase_use % u8_sysN_cadOut_comDiv) : pOutput->u8_filmPh0_inPh_pre;
		pOutput->u8_filmPh0_cnt       = u8_filmPh0_cnt_cur;

#else

		unsigned char u8_sysN_cadOut_comDiv  = (unsigned char)FRC_phTable_tool_commonDivisor(u8_sys_N, (in_3dFormat == _PQL_IN_LR? u8_cadence_outLen*2 : u8_cadence_outLen));
		unsigned char u4_in_phase_reverse    =  (u8_sysN_cadOut_comDiv != 2) ? u4_in_phase : (u4_in_phase == 0 ? 1 : 0);
		unsigned char u8_filmPh0_cnt_cur     =  u1_cadence_chg? 0 : ((in_3dFormat == _PQL_IN_LR? (u1_in3D_LR == _PQL_3D_FRAME_L? 1:0): 1) && u8_filmPh == 0? (u8_filmPh0_cnt + 1) % u8_sys_N : u8_filmPh0_cnt);
		unsigned char u1_filmPh0_inPh_chg    = ((in_3dFormat == _PQL_IN_LR?  (u1_in3D_LR == _PQL_3D_FRAME_L? 1:0): 1) && u8_filmPh == 0 && u8_filmPh0_cnt_cur == 0 && (u4_in_phase_reverse % u8_sysN_cadOut_comDiv) != pOutput->u8_filmPh0_inPh_pre)? 1 : 0;

		static unsigned int u8_LowDelay_cnt = 0;
		unsigned int u32_rb_val = 0;
		unsigned char u1_forcePhTable_Regen = 0, u1_inTable_wrt_PC_en, u1_inTable_wrt_Video_en, u1_outTable_wrt_PC_en, u1_outTable_wrt_Video_en, u1_filmPh_wrt_PC_en, u1_filmPh_wrt_Video_en;
		unsigned char QMS_24hz_state_chg = 0;
		unsigned char qms_chg = (qms_flag == pOutput->u1_qms_flag_pre) ? 0 : 1;
		u1_direct_media_flag_chg = (u1_direct_media_flag == pOutput->u1_direct_media_flag_pre)? 0 : 1;
		u1_film_cadence_chg = ((u8_film_cadence_id==pOutput->u8_film_cadence_id_pre)?0:1);

		pOutput->u8_filmPh0_inPh_pre  = u1_filmPh0_inPh_chg? (u4_in_phase_reverse % u8_sysN_cadOut_comDiv) : pOutput->u8_filmPh0_inPh_pre;
		pOutput->u8_filmPh0_cnt       = u8_filmPh0_cnt_cur;
#endif

	//*****  LowDelayMode  *****//
		// force apply LowDelay
//		unsigned int u32_low_delay;
//		ReadRegister(SOFTWARE_SOFTWARE_63_reg, 0, 31, &u32_low_delay);
//		if(((u32_low_delay >> 1) & 1) == 1){
//			pOutput->u1_lowdelay_regen = 1;
//			WriteRegister(SOFTWARE_SOFTWARE_63_reg, 1, 1, 0);
//		}
//		u32_low_delay = (u32_low_delay>>4) & 0x00000ff;

		// In_FrameRate change
		if(u1_inframe_chg == 1){
			if(u32_log_flag==1){
				rtd_pr_memc_emerg("[%s][inframe_chg : %d -> %d]\n\r", __FUNCTION__, pOutput->u1_inframe_pre, in_fmRate);
			}
		}
		// u1_source_type change
		if(u1_source_type_chg == 1){
			rtd_pr_memc_emerg("[%s][source_type_chg : %d -> %d]\n\r", __FUNCTION__, pOutput->u8_source_type_pre, u8_source_type);
		}
		if(qms_chg == 1){
			rtd_pr_memc_emerg("[%s][QMS_flag_chg : %d -> %d]\n\r", __FUNCTION__, pOutput->u1_qms_flag_pre, qms_flag);
		}
		if(in_fmRate == _PQL_IN_24HZ && out_fmRate == _PQL_OUT_120HZ && u8_sys_N == 1 && u8_sys_M == 5 && QMS_24hz_en && qms_flag){
			if(MEMC_LibGetMEMCMode() == MEMC_OFF && MEMC_LibGetCinemaMode() == FALSE){
				QMS_24hz_state++;
				if(QMS_24hz_state>100)
					QMS_24hz_state = 1;
			}else{
				QMS_24hz_state = 0;
			}
		}
		else{
			QMS_24hz_state = 0;
		}
		QMS_24hz_state_chg = (QMS_24hz_state != pOutput->QMS_24hz_state_pre) ? 1 : 0;


		#if 0 //. backup for miracast
		if(u1_MEMCMode_chg == 1){
			rtd_pr_memc_emerg("$$$   hi_MEMCMode_chg = (%d)(%d)   $$$\n\r", u16_MEMCMode, pOutput->u16_MEMCMode_pre);
		}
		#endif
#if 1
		if((u1_outMode_chg == 1) || (u1_n2m_switch == 1) || (u1_in3dFormat_chg == 1) || (u1_out3dFormat_chg == 1)
			|| (pOutput->u8_system_start_lock == 0) || (u1_LowDelayMode_chg == 1) || (u1_inframe_chg == 1)
			|| (DC_off_on_flg == 1) || (u1_source_type_chg == 1) || (VR360_en_chg == 1) || (u1_adaptivestream_flag_chg == 1)
			|| (u1_lowdelay_state_chg_flg) || (u1_direct_media_flag_chg == 1) || (NewLowDelay_chg == 1) || (u1_UI_GameMode_chg == 1)
			|| (u1_vdec_direct_lowdelay_flag_chg)
			|| (u1_direct_media_flag_chg == 1)|| (N2M_memcdtg_dv_total_check==1 && u1_n2m_switch==0)  || (timing_unstable_flag == 1) || (QMS_24hz_state_chg) || (qms_chg)
			/*|| ((s_pContext->_external_info.u1_mMiraCast == 1)&&(u1_MEMCMode_chg))*//*backup for miracast*/){
			pOutput->u1_generate_video_en = 1;
			N2M_memcdtg_dv_total_check = 0;

			u8_LowDelay_cnt = 1;
			if(u32_log_flag==1)
			{
				rtd_pr_memc_emerg("[%s][u1_generate_video_en](%d)(%d)(%d)(%d)(%d)(%d),(%d)(%d)(%d)(%d)(%d)(%d),(%d)(%d)(%d)(%d),(%d)(%d)(%d)(%d)\n\r", __FUNCTION__,
					u8_LowDelay_cnt, u1_outMode_chg, u1_n2m_switch, u1_in3dFormat_chg, u1_out3dFormat_chg, pOutput->u8_system_start_lock,
					u1_LowDelayMode_chg, DC_off_on_flg, u1_inframe_chg, u1_source_type_chg, VR360_en_chg, u1_isLowDelayMode,
					u1_adaptivestream_flag_chg, u1_lowdelay_state_chg_flg, u1_direct_media_flag_chg, u1_UI_GameMode_chg,
					u1_vdec_direct_lowdelay_flag_chg, timing_unstable_flag, QMS_24hz_state_chg, qms_chg);
			}
			DC_off_on_flg = 0;
		}
		else if((u8_LowDelay_cnt > 0) && (u8_LowDelay_cnt <= 120)){
			pOutput->u1_generate_video_en = 0;
			u8_LowDelay_cnt++;
			//rtd_pr_memc_emerg("$$$   hi_LowDelay_cnt_02 = (%d)   $$$\n\r", u8_LowDelay_cnt);
		}
		else{
			pOutput->u1_generate_video_en = 0;
			u8_LowDelay_cnt = 0;
		}

		if(scaler_force_bg_status == 1){
			if(u8_LowDelay_cnt == 1 && (u1_isLowDelayMode == 1)){
				pOutput->u1_lowdelay_regen = 1;
				LowDelayGen_cnt++;
				if(u32_log_flag==1)
				{
					rtd_pr_memc_emerg("$$$   hi_01_LowDelayRegen (@BG=1) = (%d)(%d)(%d)(%d)(%d)   $$$\n\r", pOutput->u1_lowdelay_regen, u1_LowDelayMode_chg, u1_inframe_chg, u8_LowDelay_cnt, LowDelayGen_cnt);
				}
				u8_LowDelay_cnt = 0;
			}
			else{
				pOutput->u1_lowdelay_regen = 0;
			}
		}
		else{
			if((u8_LowDelay_cnt == 1) && (u1_isLowDelayMode == 1)){
				pOutput->u1_lowdelay_regen = 1;
				LowDelayGen_cnt++;
				if(u32_log_flag==1)
				{
					rtd_pr_memc_notice("$$$   LowDelayRegen (@BG=0 @adaptivestream_flag=0) = (%d)(%d)(%d)(%d)(%d)   $$$\n\r", pOutput->u1_lowdelay_regen, u1_LowDelayMode_chg, u1_inframe_chg, u8_LowDelay_cnt, LowDelayGen_cnt);
				}
				u8_LowDelay_cnt = 0;
			}
			else{
				pOutput->u1_lowdelay_regen = 0;
			}
		}

		ReadRegister(SOFTWARE1_SOFTWARE1_00_reg, 2, 2, &u32_rb_val);
		u1_forcePhTable_Regen = (u32_rb_val >> 0) & 0x1;
		if(u1_forcePhTable_Regen){
			pOutput->u1_lowdelay_regen = 1;
		}

		if(pOutput->u1_lowdelay_regen == 1){
			pOutput->u1_generate_video_en = 1;
		}
#else
		if((u1_isLowDelayMode == 1) && (u1_outMode_chg || u1_n2m_switch || u1_in3dFormat_chg || u1_out3dFormat_chg||pOutput->u8_system_start_lock == 0 || (u1_LowDelayMode_chg == 1) || (u1_inframe_chg == 1) || (DC_off_on_flg == 1) || (u1_source_type_chg == 1))){
			u8_LowDelay_cnt = 1;
			rtd_pr_memc_emerg("$$$   hi_LowDelay_cnt_01 = (%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)(%d)   $$$\n\r", u8_LowDelay_cnt, u1_outMode_chg, u1_n2m_switch, u1_in3dFormat_chg, u1_out3dFormat_chg, pOutput->u8_system_start_lock, u1_LowDelayMode_chg, DC_off_on_flg, u1_inframe_chg, u1_source_type_chg, u1_isLowDelayMode);
			DC_off_on_flg = 0;
		}
		else if(u8_LowDelay_cnt > 0){
			u8_LowDelay_cnt++;
			rtd_pr_memc_emerg("$$$   hi_LowDelay_cnt_02 = (%d)   $$$\n\r", u8_LowDelay_cnt);
		}
		else if(u8_LowDelay_cnt > 120){
			u8_LowDelay_cnt = 0;
			rtd_pr_memc_emerg("$$$   hi_LowDelay_cnt_03 = (%d)   $$$\n\r", u8_LowDelay_cnt);
		}

		if(u8_LowDelay_cnt > 119)
		{
			pOutput->u1_lowdelay_regen = 1;
			LowDelayGen_cnt++;
			rtd_pr_memc_emerg("$$$   hi_01_LowDelayRegen = (%d)(%d)(%d)(%d)(%d)   $$$\n\r", pOutput->u1_lowdelay_regen, u1_LowDelayMode_chg, u1_inframe_chg, u8_LowDelay_cnt, LowDelayGen_cnt);
			u8_LowDelay_cnt = 0;
		}
		else
			pOutput->u1_lowdelay_regen = 0;
	#if 0
		if(u1_isLowDelayMode == 0){
			u8_LowDelay_cnt = 0;
		}
	#endif
#endif
	//*******************************//

		// synthesize
#if 0
		u1_phT_gen_nor_drive          = (u1_outMode_chg == 1 || u1_n2m_switch || u1_in3dFormat_chg || u1_out3dFormat_chg || pOutput->u8_system_start_lock == 0)? 1 : 0;

		pOutput->u1_generate_PC_en    = (u1_outMode == _PQL_OUT_PC_MODE    && (u1_outMode_chg == 1 || u1_n2m_switch))? 1 : 0;
		pOutput->u1_generate_video_en = (u1_outMode == _PQL_OUT_VIDEO_MODE &&  u1_phT_gen_nor_drive)? 1 : 0;
		//pOutput->u1_generate_film_en  = (u1_outMode == _PQL_OUT_VIDEO_MODE && (u1_phT_gen_nor_drive || u1_film_cadence_chg))? 1 : 0;
		pOutput->u1_generate_film_en  = (u1_outMode == _PQL_OUT_VIDEO_MODE && (u1_phT_gen_nor_drive || u1_filmPh0_inPh_chg|| u1_film_cadence_chg))? 1 : 0;

		pOutput->u1_phFlbk_video_en   = (u1_outMode == _PQL_OUT_VIDEO_MODE && (pOutput->u1_generate_video_en || u1_deblur_lvl_chg || u1_dejudder_lvl_chg))? 1 : 0;
		pOutput->u1_phFlbk_film_en    = (u1_outMode == _PQL_OUT_VIDEO_MODE && (pOutput->u1_generate_film_en || u1_deblur_lvl_chg || u1_dejudder_lvl_chg) )? 1 : 0;


		u1_outTable_PC_wrt            = (pOutput->u1_generate_PC_en || u1_out3dFormat_chg)? 1 : 0;
		u1_outTable_Video_wrt         = (pOutput->u1_phFlbk_film_en || pOutput->u1_phFlbk_video_en)? 1 : 0;
		pOutput->u1_inTable_wrt_en    = (u1_outMode_chg || u1_cadence_chg)? 1 : 0;
		pOutput->u1_outTable_wrt_en   = ((u1_outMode == _PQL_OUT_PC_MODE && u1_outTable_PC_wrt) || (u1_outMode == _PQL_OUT_VIDEO_MODE && u1_outTable_Video_wrt))? 1 : 0;
		pOutput->u1_filmPh_wrt_en     = (u1_outMode_chg || u1_cadence_chg)? 1 : 0;

#else
		pOutput->u1_generate_PC_en    =  (u1_outMode_chg || u1_n2m_switch || pOutput->u8_system_start_lock == 0 || (u1_LowDelayMode_chg == 1) || (u1_UI_GameMode_chg == 1))? 1 : 0;
		//pOutput->u1_generate_video_en = (u1_outMode_chg || u1_n2m_switch || u1_in3dFormat_chg || u1_out3dFormat_chg||pOutput->u8_system_start_lock == 0 )? 1 : 0;  ||(u1_LowDelayMode_chg == 1) || ((u1_isLowDelayMode == 1) && (DC_off_on_flg == 1))

		//*** merge into line:438 with game mode low delay ***//
		//pOutput->u1_generate_video_en = (u1_outMode_chg || u1_n2m_switch || u1_in3dFormat_chg || u1_out3dFormat_chg || pOutput->u8_system_start_lock == 0 || (u1_LowDelayMode_chg == 1) || (pOutput->u1_lowdelay_regen == 1) || (u1_inframe_chg == 1) || ((u1_isLowDelayMode == 1) && (DC_off_on_flg == 1)) || (u1_source_type_chg == 1))? 1 : 0;
		//********************************************//

		if(pOutput->u1_generate_video_en == 1){
			rtd_pr_memc_emerg("[u1_generate_video_en](%d)(%d)(%d)(%d),(%d)(%d)(%d)(%d),(%d)(%d)(%d)(%d),(%d)(%d)(%d)(%d),(%d)(%d)(%d)(%d)(%d),(%d)(%d)(%d)\n\r",
				pOutput->u1_lowdelay_regen, u1_outMode_chg, u1_n2m_switch, u1_in3dFormat_chg,
				u1_out3dFormat_chg, pOutput->u8_system_start_lock, u1_LowDelayMode_chg, u1_inframe_chg,
				u8_LowDelay_cnt, in_fmRate, out_fmRate, u1_isLowDelayMode,
				LowDelayGen_cnt, u1_source_type_chg, VR360_en_chg, u1_adaptivestream_flag_chg,
				u1_lowdelay_state_chg_flg, u1_direct_media_flag_chg, u1_UI_GameMode_chg, u1_vdec_direct_lowdelay_flag_chg, u1_direct_media_flag_chg,timing_unstable_flag, QMS_24hz_state_chg, qms_chg);
			if(u1_in3dFormat_chg){
				if(u32_log_flag==1)
				{
					rtd_pr_memc_emerg("$$$   hi_in3dFormat_chg   $$$\n\r$$$   (pre_in, cur_in) = (%d, %d)   $$$\n\r", pOutput->in3d_format_pre, in_3dFormat);
				}
                      }
			if(u1_out3dFormat_chg){
				if(u32_log_flag==1)
				{
					rtd_pr_memc_emerg("$$$   hi_out3dFormat_chg   $$$\n\r$$$   (pre_out, cur_out) = (%d, %d)   $$$\n\r", pOutput->out3d_format_pre, out_3dFormat);
				}
			}
		}
		pOutput->u1_generate_film_en  = (u1_outMode_chg || u1_n2m_switch || u1_in3dFormat_chg || u1_out3dFormat_chg || pOutput->u8_system_start_lock == 0 
			|| u1_filmPh0_inPh_chg || u1_film_cadence_chg || NewLowDelay_chg
				/*|| (u1_adaptivestream_flag_chg == 1)*/) ? 1 : 0;

		if (pOutput->u1_generate_film_en == 1)
		{
			if(u32_log_flag==1)
			{
				rtd_pr_memc_emerg("$$$   u1_generate_film_en = (%d)(%d)(%d)(%d),(%d)(%d)(%d)(%d)   $$$\n\r",
					u1_outMode_chg, u1_n2m_switch, u1_in3dFormat_chg, u1_out3dFormat_chg,
					pOutput->u8_system_start_lock, u1_filmPh0_inPh_chg, u1_film_cadence_chg, u1_adaptivestream_flag_chg);
				rtd_pr_memc_emerg("$$$   u1_film_cadence_chg = %d,  cur = %d, pre = %d $$$\n\r", u1_film_cadence_chg, u8_cadence_id, pOutput->u8_film_cadence_id_pre);
			}
		}

		if (pParam->u1_force_gen_table_en == 1)
		{
			if(u32_log_flag==1)
			{
				rtd_pr_memc_emerg("$$$   force gen phase table   $$$\n\r");
			}

			if(u1_outMode == _PQL_OUT_PC_MODE){
				pOutput->u1_generate_PC_en = 1;
			}else{
				pOutput->u1_generate_film_en = 1;
				pOutput->u1_generate_video_en = 1;
			}
			// clear
			WriteRegister(SOFTWARE_SOFTWARE_08_reg, 27, 27, 0);
		}


		pOutput->u1_phFlbk_video_en  = (pOutput->u1_generate_video_en || pOutput->u1_generate_film_en || u1_deblur_lvl_chg || u1_dejudder_lvl_chg || u1_pulldown_mode_chg)? 1 : 0;
		pOutput->u1_phFlbk_film_en  = (pOutput->u1_generate_video_en || pOutput->u1_generate_film_en || u1_deblur_lvl_chg || u1_dejudder_lvl_chg || u1_pulldown_mode_chg)? 1 : 0;


		u1_inTable_wrt_PC_en    = (u1_outMode == _PQL_OUT_PC_MODE && (u1_outMode_chg ||pOutput->u8_system_start_lock == 0))? 1 : 0;
		u1_inTable_wrt_Video_en = (u1_outMode == _PQL_OUT_VIDEO_MODE && (u1_outMode_chg || u1_cadence_chg||(pOutput->u8_system_start_lock == 0)))? 1 : 0;
		pOutput->u1_inTable_wrt_en    = (u1_inTable_wrt_PC_en || u1_inTable_wrt_Video_en);

		u1_outTable_wrt_PC_en   = (u1_outMode == _PQL_OUT_PC_MODE && (pOutput->u1_generate_PC_en))? 1 : 0;
		u1_outTable_wrt_Video_en   = (u1_outMode == _PQL_OUT_VIDEO_MODE && (pOutput->u1_phFlbk_film_en || pOutput->u1_phFlbk_video_en))? 1 : 0;
		pOutput->u1_outTable_wrt_en   = (u1_outTable_wrt_PC_en || u1_outTable_wrt_Video_en);
		if(pOutput->u1_outTable_wrt_en == 1)
		{
			if(u32_log_flag==1)
			{
			       rtd_pr_memc_emerg("$$$   outTable_wrt_en: generate_video_en = %d,  u1_generate_film_en = %d, deblur_lvl_chg = %d, dejudder_lvl_chg=%d, pulldown_mode_chg=%d   $$$\n\r", pOutput->u1_generate_video_en, pOutput->u1_generate_film_en, u1_deblur_lvl_chg, u1_dejudder_lvl_chg, u1_pulldown_mode_chg);
			}
		}

		u1_filmPh_wrt_PC_en     = (u1_outMode == _PQL_OUT_PC_MODE && (u1_outMode_chg ||pOutput->u8_system_start_lock == 0)) ? 1 : 0;
		u1_filmPh_wrt_Video_en     = (u1_outMode == _PQL_OUT_VIDEO_MODE && (u1_outMode_chg || u1_cadence_chg || u1_filmPh0_inPh_chg ||pOutput->u8_system_start_lock == 0)) ? 1 : 0;
		pOutput->u1_filmPh_wrt_en     = (u1_filmPh_wrt_PC_en || u1_filmPh_wrt_Video_en || NewLowDelay_en);

#endif
		//// after image
		if(0)
		{
			if(u1_outMode == _PQL_OUT_VIDEO_MODE && (u1_cadence_chg == 1 || s_pContext->_output_filmDetectctrl.u1_cad_resync_true[0] == 1))  // bai need make sure
			{
				pOutput->u1_after_img = 1;
				pOutput->u8_after_img_cadence_id_pre = pOutput->u8_cadence_id_pre;

				if(u1_cadence_chg == 1)
				{
					pOutput->u1_after_img_type = 0;
					//LogPrintf(DBG_MSG,"bai print .cadence_change = 1, pre_cad_id = %d, cur_cad_id = %d, filmph_cur = %d \r\n", pOutput->u8_cadence_id_pre, u8_cadence_id,s_pContext->_output_filmDetectctrl.u8_PhaseIdx);
				}
				if(s_pContext->_output_filmDetectctrl.u1_cad_resync_true[0] == 1)
				{
					pOutput->u1_after_img_type = 1;
					//LogPrintf(DBG_MSG,"bai print . resync_flag = %d \r\n", s_pContext->_output_filmDetectctrl.u1_cad_resync_true[0]);
				}
			}
		}

	}

	if(MC_8_buffer_en){
		FRC_phTable_set_MC_8_buffer();
	}

	FRC_phTable_set_LineModeCtrl();

	if(MEMC_Lib_UltraLowDelayMode_Judge() == TRUE && NewLowDelay_en == 0){
		#if 1
		if(!(UltraLowDelay_status == 1 && frc_vtrig_dly == FRC_phTable_UltraLowDelayMode_Target() )){
			MEMC_Lib_Set_UltraLowDelayMode(1);
			UltraLowDelay_status = 1;
		}
		#else
		if(!(UltraLowDelay_status == 1 && frc_vtrig_dly == 0xa)){
			MEMC_Lib_Set_UltraLowDelayMode(1);
			UltraLowDelay_status = 1;
		}
		#endif
	}
	else{
		if(!(UltraLowDelay_status == 0 && frc_vtrig_dly == s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_crtc1_frc_vtrig_dly)){
			MEMC_Lib_Set_UltraLowDelayMode(0);
			UltraLowDelay_status = 0;
		}
	}

	if(NewLowDelay_en){
		FRC_phTable_set_NewLowDelay(1);
		u8_dejudder_lvl = (1 << _PHASE_FLBK_LVL_PRECISION);
		u8_deblur_lvl = (1 << _PHASE_FLBK_LVL_PRECISION);
	}
	else{
		FRC_phTable_set_NewLowDelay(0);
	}

	// lbmc_vtrig_dly_ctrl
	#if 0
		{
			unsigned int tmp_val = 0, lbmc_vtrig_dly = 0, rb_val = 0, log_en = 0;
			lbmc_vtrig_dly = s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_crtc1_lbmc_vtrig_dly;
			ReadRegister(CRTC1_CRTC1_28_reg, 16, 28, &rb_val);
			ReadRegister(SOFTWARE1_00_ADDR_SOFTWARE1_60_ADDR_reg, 25, 25, &log_en);
			if(log_en == 1){
				rtd_pr_memc_notice("[%s][adaptive,%d,][callback,%d,]\n\r", __FUNCTION__, u1_adaptivestream_flag, u1_callback_subscibe);
			}
			if(u1_adaptivestream_flag && u1_callback_subscibe){
				if(in_fmRate == _PQL_IN_50HZ && out_fmRate == _PQL_OUT_50HZ){
					tmp_val = lbmc_vtrig_dly + 450;
					if(rb_val != tmp_val){
						HAL_CRTC_SetLbmcVtrigDelay(tmp_val);
					}
				}
				else{
					if(rb_val != lbmc_vtrig_dly){
						HAL_CRTC_SetLbmcVtrigDelay(lbmc_vtrig_dly);
					}
				}
			}
			else{
				if(rb_val != lbmc_vtrig_dly){
					HAL_CRTC_SetLbmcVtrigDelay(s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_crtc1_lbmc_vtrig_dly);
				}
			}
		}
	#endif

	//Q-16730
	if(log_en){
		rtd_pr_memc_notice("[MEMC][%s][,%d,%d,]\n", __FUNCTION__, u8_MEMCMode, MEMC_LibGetMEMCMode());
	}

	// update states
	{
		phase_cnt = 0;
		pOutput->in3d_format_pre        = in_3dFormat;
		pOutput->out3d_format_pre       = out_3dFormat;
		pOutput->u8_sys_N_pre           = u8_sys_N;
		pOutput->u8_sys_M_pre           = u8_sys_M;
		pOutput->u8_cadence_id_pre      = u8_cadence_id;
		pOutput->u8_film_cadence_id_pre = (u8_cadence_id != _CAD_VIDEO)? u8_cadence_id : ((u2_isPreCadSupport==CAD_T)? pOutput->u8_film_cadence_id_pre : _CAD_22);
		pOutput->u8_deblur_lvl_pre      = u8_deblur_lvl;
		//++YE Test
		if(s_pContext->_external_data.u3_memcMode == 1)//Natural Mode only
		{
			pOutput->u8_dejudder_lvl_pre = FRC_Dejudder_By_Motion_Mapping((unsigned int)u8_dejudder_lvl);
		}
		else{
			pOutput->u8_dejudder_lvl_pre    = u8_dejudder_lvl;
		}
		//pOutput->u8_dejudder_lvl_pre    = u8_dejudder_lvl;
		pOutput->u8_pulldown_mode_pre	= u8_pulldown_mode;
		pOutput->u8_filmPh              = u8_filmPh;  // bai
		pOutput->u1_outMode_pre         = u1_outMode;
		pOutput->u1_is_inFormat_PureVideo  = u1_is_inFormat_PureVideo;

		pOutput->u1_lowdelaystate = u1_isLowDelayMode;
		pOutput->u1_UI_GameMode_state = u1_UI_GameMode;
		pOutput->u1_inframe_pre = in_fmRate;
		pOutput->u8_source_type_pre = u8_source_type;
		//pOutput->u16_MEMCMode_pre = u16_MEMCMode; //. backup for miracast
		pOutput->u1_VR360_en_pre = VR360_en;
		if(LowDelay_regen_chk == 1){   // prevent force apply error
			pOutput->u1_lowdelay_regen = 0;
			LowDelay_regen_chk = 0;
		}
		pOutput->u1_adaptivestream_flag_pre = u1_adaptivestream_flag;
		pOutput->u1_direct_media_flag_pre = u1_direct_media_flag;
		pOutput->u1_vdec_direct_lowdelay_flag_pre = u1_vdec_direct_lowdelay_flag;
		NewLowDelay_en_pre = NewLowDelay_en;
		pOutput->u1_timing_status_pre = vbe_get_disp_timing_change_status();
		pOutput->QMS_24hz_state_pre = QMS_24hz_state;
		pOutput->u1_qms_flag_pre = qms_flag;
	}

	// debug print signal control.
	/*	if(pParam->u1_filmPh_pf_en == 1)
		{
			LogPrintf(DBG_MSG,"PC_gen = %d, video_gen = %d, video_flbk = %d, ", pOutput->u1_generate_PC_en, pOutput->u1_generate_video_en, pOutput->u1_phFlbk_video_en);
			LogPrintf(DBG_MSG,"film_gen = %d, film_flbk = %d, ", pOutput->u1_generate_film_en, pOutput->u1_phFlbk_film_en);
			LogPrintf(DBG_MSG,"in_wrt = %d, out_wrt = %d, film_wrt = %d, \r\n", pOutput->u1_inTable_wrt_en, pOutput->u1_outTable_wrt_en, pOutput->u1_filmPh_wrt_en);
			LogPrintf(DBG_MSG,"film_cadence = %d, ", pOutput->u8_cadence_id_pre);
		}*/

}

#if 0
static _dtg_vtotal_table  DTG_vtotal_table[3][8]={
	/* 2K */
	{	{1350,  1350,  1125,  1125,  1125,  1125,  1125,  1125},	},
	/* 4K */
	{	{2700,  2700,  2250,  2250,  2250,  2250,  2250,  2250},	},
	/* 8K */
	{	{5400,  5400,  4500,  4500,  4500,  4500,  4500,  4500},	},
};
#endif

#define	PPOVERLAY_memcdtg_DV_TOTAL_reg	0xB8028604
#define	PPOVERLAY_uzudtg_DV_TOTAL_reg	0xB8028504

extern unsigned int scalerdisplay_get_DTG_MASTER_vtotal_by_vfreq(void);
VOID FRC_phTable_Generate(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput)
{
	const _PQLCONTEXT     *s_pContext = GetPQLContext();
	Ph_decGen_Param  phT_decGen_param;
	PQL_INPUT_FRAME_RATE in_fmRate = 0;
	PQL_OUTPUT_FRAME_RATE  out_fmRate = 0;
	unsigned char warning_flag = 0;
	static unsigned int frm_cnt = 0;
	unsigned int log_en = 0;
	unsigned int VTotal = 0;

#ifdef new_delay
	unsigned int u32BP, u32SysDly = 0, inVtotal = 0;
#endif

	phT_decGen_param.u1_LRout_samePh  = pParam->u1_outLR_LRsamePh;
	phT_decGen_param.u4_sys_N         = pOutput->u8_sys_N_pre;
	phT_decGen_param.u6_sys_M         = pOutput->u8_sys_M_pre;
	phT_decGen_param.u8_deblur_lvl    = pOutput->u8_deblur_lvl_pre;
	phT_decGen_param.u8_dejudder_lvl  = pOutput->u8_dejudder_lvl_pre;
	phT_decGen_param.u8_pulldown_mode = pOutput->u8_pulldown_mode_pre;
	phT_decGen_param.in3d_format      = pOutput->in3d_format_pre;
	phT_decGen_param.out3d_format     = pOutput->out3d_format_pre;
	in_fmRate = s_pContext->_external_data._input_frameRate;
	out_fmRate = s_pContext->_external_data._output_frameRate;
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	frm_cnt++;

	if(pOutput->u1_generate_video_en == 1 || pOutput->u1_generate_film_en == 1)
	{
		unsigned int VSize = Get_DISP_DEN_END_VPOS() - Get_DISP_DEN_STA_VPOS();

		VTotal = scalerdisplay_get_DTG_MASTER_vtotal_by_vfreq();

#ifdef new_delay
		RTKReadRegister(PPOVERLAY_uzudtg_DV_TOTAL_reg, &inVtotal);
		inVtotal = inVtotal & 0x1FFF;
#endif

		if(VSize > 2100)
		{
			if(VTotal < 2000)
			{
				VTotal = 0x8ca;
				rtd_pr_memc_notice("4k VTotal is error, use default value 0x8ca \n");
			}
		}
		else if(VSize > 1000)
		{
			if(VTotal < 1000)
			{
				VTotal = 0x44c;
				rtd_pr_memc_notice("2k VTotal is error, use default value 0x44c \n");
			}
		}
		else
		{
			VTotal = 0x8ca;
			rtd_pr_memc_notice("VSize is error, use default VTotal value 0x8ca \n");
		}
//		phT_decGen_param.u8_ph_sys_dly = ((s_pContext->_output_read_comreg.u13_dec_vtrig_dly * phT_decGen_param.u4_sys_N * 100) / VTotal) + (pParam->u8_input_dly * phT_decGen_param.u6_sys_M);
#ifdef new_delay
		u32BP = inVtotal - s_pContext->_output_read_comreg.u16_inVAct -
					   (s_pContext->_output_read_comreg.u16_inFrm_Pos << 4) / s_pContext->_output_read_comreg.u16_inHTotal;

		u32SysDly = u32BP * phT_decGen_param.u6_sys_M * inVtotal / phT_decGen_param.u4_sys_N / VTotal +
					s_pContext->_output_read_comreg.u13_ip_vtrig_dly +
					s_pContext->_output_read_comreg.u13_me2_org_vtrig_dly +
					s_pContext->_output_read_comreg.u13_me2_vtrig_dly +
					s_pContext->_output_read_comreg.u13_dec_vtrig_dly -
					(pParam->u8_input_dly << 1);

		phT_decGen_param.u8_ph_sys_dly = u32SysDly * phT_decGen_param.u4_sys_N * 100 / VTotal;
#else
		phT_decGen_param.u8_ph_sys_dly = ((s_pContext->_output_read_comreg.u13_dec_vtrig_dly * phT_decGen_param.u4_sys_N * 100) / VTotal) + (pParam->u8_input_dly * phT_decGen_param.u6_sys_M);
#endif
	
	//rtd_pr_memc_notice("[%s]VTotal=%d[,%d,%d,][,%d,%d,]\n",__FUNCTION__, VTotal, (s_pContext->_output_read_comreg.u13_dec_vtrig_dly), (pParam->u8_input_dly), (phT_decGen_param.u4_sys_N), (phT_decGen_param.u6_sys_M));
	}

	if((out_fmRate == _PQL_OUT_50HZ || out_fmRate == _PQL_OUT_100HZ) && GS_image_vsize == 3840 && VTotal != 2700){
		warning_flag = 1;
	}
	else if((out_fmRate == _PQL_OUT_60HZ || out_fmRate == _PQL_OUT_120HZ) && GS_image_vsize == 3840 && VTotal != 2250){
		warning_flag = 1;
	}
	
	if((warning_flag && frm_cnt%36000 == 0)||(log_en == 1)){
		//rtd_pr_memc_notice("DTG Master VTotal = %d, DTG in/out frame rate = %x/%x\r\n", scalerdisplay_get_DTG_MASTER_vtotal_by_vfreq(), rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(PPOVERLAY_memcdtg_DVS_cnt_reg));
		rtd_pr_memc_notice("DTG in/out frame rate = %x/%x\r\n", rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(PPOVERLAY_memcdtg_DVS_cnt_reg));
		rtd_pr_memc_notice("MEMC get in/out frame rate = %d/%d\r\n", in_fmRate, out_fmRate);
	}



	///// generate of table
	if (pOutput->u1_outMode_pre == _PQL_OUT_PC_MODE)
	{
		// pc mode
		if (pOutput->u1_generate_PC_en == 1)
		{
			phT_decGen_param.u16_phT_stIdx    = 0;

			FRC_phTable_WrtStep_PCmode_Gen();
			FRC_decPhT_PCmode_Proc(&phT_decGen_param, &phT_dec_PC);

			pOutput->u16_phTable_wrt_stIdx     = phT_dec_PC.u16_phT_stIdx;
			pOutput->u16_phTable_wrt_endIdx_p1 = phT_dec_PC.u16_phT_endIdx_p1;
			pOutput->u16_phTable_wrt_vd_endIdx_p1 = phT_dec_PC.u16_phT_endIdx_p1;
		}
	}
	else
	{

		// in wrt Table //LISA_161009
		if(pOutput->u1_inTable_wrt_en == 1 || pOutput->u1_generate_video_en == 1)
		{
			FRC_phTable_WrtStep_VideoMode_Gen(frc_cadTable[_CAD_VIDEO]);
		}

  		// out Phase Table
	 	//video
		phT_decGen_param.u1_goldenPhT_gen_en = pOutput->u1_generate_video_en;
		phT_decGen_param.u1_phFlbk_en        = pOutput->u1_phFlbk_video_en;
		phT_decGen_param.u4_filmPh0_inPh    = 0;
		phT_decGen_param.u8_cadence_id       = _CAD_VIDEO;
		phT_decGen_param.u16_phT_stIdx       = 0;

		FRC_phTable_VideoMode_Proc(&phT_decGen_param, &phT_dec_Video);

		pOutput->u16_phTable_wrt_stIdx     = phT_dec_Video.u16_phT_stIdx;
		pOutput->u16_phTable_wrt_endIdx_p1 = phT_dec_Video.u16_phT_endIdx_p1;
		pOutput->u16_phTable_wrt_vd_endIdx_p1 = phT_dec_Video.u16_phT_endIdx_p1;

		// in wrt Table //LISA_161009
		if(pOutput->u1_inTable_wrt_en == 1 || pOutput->u1_generate_film_en == 1)
		{
			FRC_phTable_WrtStep_VideoMode_Gen(frc_cadTable[pOutput->u8_film_cadence_id_pre]);
		}

		//film
		phT_decGen_param.u1_goldenPhT_gen_en = pOutput->u1_generate_film_en;
		phT_decGen_param.u1_phFlbk_en        = pOutput->u1_phFlbk_film_en;
		phT_decGen_param.u4_filmPh0_inPh     = pOutput->u8_filmPh0_inPh_pre;
		phT_decGen_param.u8_cadence_id       = pOutput->u8_film_cadence_id_pre;
		phT_decGen_param.u16_phT_stIdx       = phT_dec_Video.u16_phT_endIdx_p1;

		FRC_phTable_VideoMode_Proc(&phT_decGen_param, &phT_dec_Film);

		pOutput->u16_phTable_wrt_endIdx_p1 = phT_dec_Film.u16_phT_endIdx_p1;

		// in wrt Table //LISA_161009
		if(pOutput->u1_inTable_wrt_en == 1 || pOutput->u1_generate_video_en == 1 || pOutput->u1_generate_film_en == 1)
		{
			FRC_phTable_WrtStep_VideoMode_Gen(frc_cadTable[pOutput->u8_cadence_id_pre]);
		}
	}

//		if(pParam->u1_filmPh_pf_en == 1)
//		{
//			LogPrintf(DBG_MSG,"PC_gen = %d, vd_22_32_gen = %d, ", pOutput->u1_generate_PC_en, pOutput->u1_generate_vd_22_32_en);
//			LogPrintf(DBG_MSG,"film_gen = %d, film_flbk = %d, ", pOutput->u1_generate_film_en, pOutput->u1_phFlbk_film_en);
//			LogPrintf(DBG_MSG,"in_wrt = %d, out_wrt = %d, film_wrt = %d, \r\n", pOutput->u1_inTable_wrt_en, pOutput->u1_outTable_wrt_en, pOutput->u1_filmPh_wrt_en);
//			LogPrintf(DBG_MSG,"vid2232_st = %d, vid2232_end = %d,wt_end = %d,", pOutput->u16_phTable_wrt_stIdx,pOutput->u16_phTable_wrt_vd_22_32_endIdx,pOutput->u16_phTable_wrt_endIdx_p1);
//		}
	if(frm_cnt > 36000)
			frm_cnt = 1;
}
VOID FRC_phTable_Write(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput)
{
	unsigned char u8_filmPhase = 0;

	if(pOutput->u8_cadence_id_pre >= _FRC_CADENCE_NUM_){

		rtd_pr_memc_notice("[%s]Invalid u8_cadence_id_pre:%d!\n",__FUNCTION__,pOutput->u8_cadence_id_pre);
		pOutput->u8_cadence_id_pre = _CAD_VIDEO;
	}
	if(frc_cadTable[pOutput->u8_cadence_id_pre].cad_outLen == 0){
		rtd_pr_memc_notice("[%s]Invalid cad_outLen=%d.Resume!\n",__FUNCTION__,frc_cadTable[pOutput->u8_cadence_id_pre].cad_outLen);
		frc_cadTable[pOutput->u8_cadence_id_pre].cad_outLen = 1;
	}
	u8_filmPhase = (pOutput->u1_outMode_pre == _PQL_OUT_PC_MODE)? 0 : (pOutput->u8_filmPh + 1)% frc_cadTable[pOutput->u8_cadence_id_pre].cad_outLen;
	WriteRegister(KPHASE_kphase_0C_reg,0,5, u8_filmPhase);

	// after image and write Kphase
	if(0)
	{
		unsigned char u8_filmPhase_det = (pOutput->u1_outMode_pre == _PQL_OUT_PC_MODE)? 0 : (pOutput->u8_filmPh + 1)% frc_cadTable[pOutput->u8_cadence_id_pre].cad_outLen;
		unsigned char after_img;

		if(pOutput->u1_after_img == 1)
		{
			unsigned int u8_filmPhase_pre;
			unsigned int u8_outPhase;
			unsigned char u8_cadence_id_pre = pOutput->u8_after_img_cadence_id_pre;
			unsigned char u8_cadence_id_dtc = pOutput->u8_cadence_id_pre;

			unsigned int filmPhase_result;

			ReadRegister(KPHASE_kphase_0C_reg,0,5, &u8_filmPhase_pre);
			ReadRegister(KPHASE_kphase_9C_reg,0,5, &u8_outPhase);

			//LogPrintf(DBG_MSG,"bai print . u8_cadence_id_pre = %d, u8_cadence_id_dtc = %d, u8_filmPhase_pre = %d, u8_filmPhase_det = %d, u8_outPhase_pre =%d \r\n", u8_cadence_id_pre, u8_cadence_id_dtc, u8_filmPhase_pre, u8_filmPhase_det, u8_outPhase);

			after_img= after_image_proc(u8_filmPhase_pre, u8_filmPhase_det, u8_outPhase, u8_cadence_id_pre, u8_cadence_id_dtc, pParam->in_3dFormat, pOutput->u8_sys_M_pre,pOutput->u16_phTable_wrt_endIdx_p1);

			//LogPrintf(DBG_MSG,"print bai. after_img = %d \r\n", after_img);

			if(after_img == 1)
			{
				filmPhase_result = u8_filmPhase_det;
				pOutput->u1_after_img = 0;

				FRC_phTable_Kphase_ctrl_Wrt(pParam);

				WriteRegister(KPHASE_kphase_0C_reg,0,5, u8_filmPhase_det);
			}
			else
			{
				filmPhase_result = (u8_filmPhase_pre+ 1)% frc_cadTable[u8_cadence_id_pre].cad_outLen;
				WriteRegister(KPHASE_kphase_0C_reg,0,5, filmPhase_result);
			}
			if(0)
			{
				unsigned int u8_filmPhase;
				unsigned int u8_outPhase;
				unsigned int cad_outLen;
				unsigned int u32_wrt_film_mode;

				ReadRegister(KPHASE_kphase_0C_reg,28,30,  &u32_wrt_film_mode);
				ReadRegister(KPHASE_kphase_0C_reg,18,23, &cad_outLen);
				ReadRegister(KPHASE_kphase_0C_reg,0,5, &u8_filmPhase);
				ReadRegister(KPHASE_kphase_9C_reg,0,5, &u8_outPhase);

				rtd_pr_memc_notice("bai print result read back. film_mode = %d, cad_outLen = %d, filmPhase = %d, outPhase = %d \r\n", u32_wrt_film_mode, cad_outLen, u8_filmPhase, u8_outPhase);
			}
		}
		else
		{
			WriteRegister(KPHASE_kphase_0C_reg,0,5, u8_filmPhase_det);
			if (pOutput->u1_filmPh_wrt_en == 1)
			{
				FRC_phTable_Kphase_ctrl_Wrt(pParam);
			}
		}

	}

	if (pOutput->u1_inTable_wrt_en == 1 && pOutput->u8_cadence_id_pre != _CAD_VIDEO && pOutput->u8_film_cadence_id_pre != _CAD_22 && pOutput->u8_film_cadence_id_pre != _CAD_32)
	{
		FRC_phTable_inDec_Wrt();
	}

	if (pOutput->u1_outTable_wrt_en == 1)
	{
		FRC_phTable_outDec_Wrt(pParam, pOutput);
	}

	if (pOutput->u1_filmPh_wrt_en == 1)
	{
		FRC_phTable_Kphase_ctrl_Wrt(pParam);
	}
}

VOID   FRC_phTable_inDec_Wrt()
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u8_cadence_id           = s_pContext->_output_frc_phtable.u8_cadence_id_pre;
	unsigned char u8_k = 0;
	unsigned char Len_vid_22_32;
	unsigned char Len_vid_22_32_cadence;
	unsigned char me2_wrt_reg_stId = 0;

	if(u8_cadence_id >= _FRC_CADENCE_NUM_){
		rtd_pr_memc_notice("[%s]Invalid u8_cadence_id:%d.Resume!\n",__FUNCTION__,u8_cadence_id);
		u8_cadence_id = _CAD_VIDEO;
	}
	FRC_phTable_inDec_amend(u8_cadence_id);

	Len_vid_22_32 = frc_cadTable[_CAD_VIDEO].cad_outLen + frc_cadTable[_CAD_22].cad_outLen + frc_cadTable[_CAD_32].cad_outLen;
	Len_vid_22_32_cadence = Len_vid_22_32 + frc_cadTable[u8_cadence_id].cad_outLen;
	me2_wrt_reg_stId = Len_vid_22_32/2 ;

	for (u8_k = Len_vid_22_32; u8_k < Len_vid_22_32_cadence; u8_k = u8_k + 4)
	{
		unsigned char pack_stIdx = u8_k - Len_vid_22_32;
		unsigned int u32_phT_wrtStep_pack = phTable_WrtStep_pack(pack_stIdx);
		WriteRegister(KPHASE_kphase_14_reg + u8_k, 0, 31, u32_phT_wrtStep_pack);
	}
	for (u8_k = 0; u8_k < frc_cadTable[u8_cadence_id].cad_outLen; u8_k = u8_k + 8)
	{
		unsigned int u32_phT_wrtStep_pack = phTable_me2_WrtStep_pack(u8_k);
		WriteRegister(KPHASE_kphase_58_reg + me2_wrt_reg_stId, 0, 31, u32_phT_wrtStep_pack);
		me2_wrt_reg_stId = me2_wrt_reg_stId + 4;
	}
}
VOID   FRC_phTable_outDec_Wrt(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput)
{

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned short phT_stIdx;
	unsigned short phT_endIdx_p1          = s_pContext->_output_frc_phtable.u16_phTable_wrt_endIdx_p1;
	unsigned short clear_st;

	unsigned short u16_k, phT_endIdx_p1_new;
	Ph_Dec_Pack_Info packInfo;
	//unsigned int ee_addr_base = 0x00000000;//0xC0000000
	unsigned int ee_addr_base = KMC_TOP_kmc_top_00_reg;//0xC0000000



	clear_st      = 0;
	phT_stIdx     = s_pContext->_output_frc_phtable.u16_phTable_wrt_stIdx;

	FRC_kphase_sram_clear(clear_st, 255);

	WriteRegister(ee_addr_base + 0x4F00, 0, 31, 0);     //Prepare write EEPROM
	WriteRegister(ee_addr_base + 0x4F00, 0, 31, 0x80000000 | phT_stIdx);
	//LogPrintf(DBG_MSG, "%s::phT_stIdx=%d,phT_endIdx_pl=%d \n",__FUNCTION__,phT_stIdx,phT_endIdx_p1);
	for (u16_k = phT_stIdx; u16_k < phT_endIdx_p1; u16_k ++)
	{
		unsigned int pack_MSB = 0, pack_LSB = 0;

		FRC_phTable_allInfo_Pack_Gen(pParam, pOutput, u16_k, &packInfo);
		phaseTable_pack(&packInfo, &pack_LSB, &pack_MSB);

		WriteRegister(ee_addr_base + 0x4F04, 0, 31, pack_LSB);
		WriteRegister(ee_addr_base + 0x4F08, 0, 31, pack_MSB);
	}

	//Protect for outPhase over
	phT_endIdx_p1_new = _CLIP_(phT_endIdx_p1+_ABS_DIFF_(phT_endIdx_p1,phT_stIdx), 0, 255);
	for (u16_k = phT_endIdx_p1; u16_k < phT_endIdx_p1_new; u16_k ++)
	{
		unsigned int pack_MSB = 0, pack_LSB = 0;

		unsigned short u16_k_phT_idx = u16_k-phT_endIdx_p1+phT_stIdx;
		FRC_phTable_allInfo_Pack_Gen(pParam, pOutput, u16_k_phT_idx, &packInfo);
		if(u16_k_phT_idx < pOutput->u16_phTable_wrt_vd_endIdx_p1)//VIDEO
		{
			packInfo.u8_msb[MSB_OUT_PHASE] = phTable_DecInfoGen[u16_k_phT_idx].out_phase
											+ phTable_DecInfoGen[pOutput->u16_phTable_wrt_vd_endIdx_p1-1].out_phase;
		}
		else
		{
			packInfo.u8_msb[MSB_OUT_PHASE] = phTable_DecInfoGen[u16_k_phT_idx].out_phase
											+ phTable_DecInfoGen[phT_endIdx_p1-1].out_phase;
		}
		phaseTable_pack(&packInfo, &pack_LSB, &pack_MSB);

		WriteRegister(ee_addr_base + 0x4F04, 0, 31, pack_LSB);
		WriteRegister(ee_addr_base + 0x4F08, 0, 31, pack_MSB);
	}

	// change lut selection.
	{
		unsigned int u32_Tmp;
		ReadRegister(KPHASE_kphase_10_reg,16,16, &u32_Tmp);
		u32_Tmp = ((u32_Tmp&0x01) == 1)? 0 : 1;
		WriteRegister(KPHASE_kphase_10_reg,16,16, u32_Tmp);
	}

}
VOID   FRC_phTable_Kphase_ctrl_Wrt(const _PARAM_FRC_PH_TABLE *pParam)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char  u1_outMode             = s_pContext->_output_frc_phtable.u1_outMode_pre;
	unsigned char  u8_cadence_id          = s_pContext->_output_frc_phtable.u8_cadence_id_pre;
	unsigned int u32_wrt_film_mode      = u8_cadence_id == _CAD_VIDEO? 0 : (u8_cadence_id == _CAD_22? 1 : (u8_cadence_id == _CAD_32? 2 : 3));

	if(u8_cadence_id >= _FRC_CADENCE_NUM_){
		rtd_pr_memc_notice("[%s]Invalid u8_cadence_id:%d.Resume!\n",__FUNCTION__,u8_cadence_id);
		u8_cadence_id = _CAD_VIDEO;
	}
	if (u1_outMode == _PQL_OUT_PC_MODE)
	{
		WriteRegister(KPHASE_kphase_0C_reg,24,24,             1);
		WriteRegister(KPHASE_kphase_0C_reg,0,5,          0);
		WriteRegister(KPHASE_kphase_0C_reg,28,30,              0);
		WriteRegister(KPHASE_kphase_0C_reg,18,23,        1);
	}
	else
	{
		if(pParam->u1_filmPh_sw_en == 1)
		{
			WriteRegister(KPHASE_kphase_0C_reg,24,24,                 1);
			WriteRegister(KPHASE_kphase_0C_reg,28,30,  u32_wrt_film_mode);
			WriteRegister(KPHASE_kphase_0C_reg,18,23, frc_cadTable[u8_cadence_id].cad_outLen);
		}
		else
		{
			WriteRegister(KPHASE_kphase_0C_reg,24,24,              0);
			WriteRegister(KPHASE_kphase_0C_reg,28,30,               0);
			WriteRegister(KPHASE_kphase_0C_reg,18,23, frc_cadTable[u8_cadence_id].cad_outLen);
		}
	}

}

///////////////////////    phase table  write into sram   //////////////////////////////////
VOID   FRC_phTable_allInfo_Pack_Gen(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput, unsigned short phDecInfo_k, Ph_Dec_Pack_Info *pPackInfo)  // bai  need  change
{
        #define DEC_INFO_WRT_STIDX 0
        #if DEC_INFO_WRT_STIDX
	const _PQLCONTEXT   *s_pContext = GetPQLContext();
        #endif
	//PQL_INPUT_3D_FORMAT in_3dFormat = s_pContext->_output_frc_phtable.in3d_format_pre;

	unsigned char u1_out3D_en = (pOutput->out3d_format_pre == _PQL_OUT_SG || pOutput->out3d_format_pre == _PQL_OUT_SG_LLRR || pOutput->out3d_format_pre == _PQL_OUT_PR)? 1 : 0;
	unsigned char mc_hf_idx   = 0;
	unsigned char mc_lf_idx   = 0;

	unsigned char u1_is_firstPhase = 0;
	//unsigned char org_out_3d_lr = 0;

	if(phDecInfo_k >= sizeof(phTable_DecInfoGen) / sizeof(Ph_Dec_Info)){
		rtd_pr_memc_notice("[%s]Invalid phDecInfo_k=%d.Resume!\n",__FUNCTION__,phDecInfo_k);
                #if DEC_INFO_WRT_STIDX
		phDecInfo_k = s_pContext->_output_frc_phtable.u16_phTable_wrt_stIdx;
                #else
                phDecInfo_k= 0;
                #endif
	}
	mc_hf_idx   = _CLIP_(((phTable_DecInfoGen[phDecInfo_k].mc_phase* 4 + 64) >> 7), 1, 3) - 1;
	mc_lf_idx   = _CLIP_(((phTable_DecInfoGen[phDecInfo_k].mc_phase*10 + 64) >> 7), 3, 7) - 1;
	// firstPhase, lastPhase judge
	{
		unsigned short u16_phT_stIdx, u16_phT_endIdx, u16_phDecInfo_k_n1, u16_phDecInfo_k_p1;
		if (pOutput->u1_outMode_pre == _PQL_OUT_PC_MODE)
		{
			u16_phT_stIdx    = phT_dec_PC.u16_phT_stIdx;
			u16_phT_endIdx   = phT_dec_PC.u16_phT_endIdx_p1 - 1;
		}
		else if (phDecInfo_k < phT_dec_Film.u16_phT_stIdx)
		{
			u16_phT_stIdx    = phT_dec_Video.u16_phT_stIdx;
			u16_phT_endIdx   = phT_dec_Video.u16_phT_endIdx_p1 - 1;

		}
		else
		{
			u16_phT_stIdx    = phT_dec_Film.u16_phT_stIdx;
			u16_phT_endIdx   = phT_dec_Film.u16_phT_endIdx_p1 - 1;
		}
		u16_phDecInfo_k_n1   = (phDecInfo_k == u16_phT_stIdx)?  u16_phT_endIdx : phDecInfo_k - 1;
		u16_phDecInfo_k_p1   = (phDecInfo_k == u16_phT_endIdx)? u16_phT_stIdx  : phDecInfo_k + 1;

		if(u16_phDecInfo_k_n1 >= sizeof(phTable_DecInfoGen) / sizeof(Ph_Dec_Info)){
			rtd_pr_memc_notice("[%s]Invalid u16_phDecInfo_k_n1=%d.Resume!\n",__FUNCTION__,u16_phDecInfo_k_n1);
			u16_phDecInfo_k_n1 = 0;
		}
                #if DEC_INFO_WRT_STIDX
		u16_phDecInfo_k_n1 = _CLIP_(u16_phDecInfo_k_n1, 0, 255);
                #endif
		u1_is_firstPhase     = (phDecInfo_k == u16_phDecInfo_k_n1)? 1 : ( (phTable_DecInfoGen[phDecInfo_k].me2_phase < phTable_DecInfoGen[u16_phDecInfo_k_n1].me2_phase)? 1 : 0 );
	}

	//// assign
	pPackInfo->u8_msb[MSB_OUT_3D_ENABLE]           = u1_out3D_en;
	pPackInfo->u8_msb[MSB_FILM_MODE]               = phTable_DecInfoGen[phDecInfo_k].film_mode;
	pPackInfo->u8_msb[MSB_OUT_PHASE]               = phTable_DecInfoGen[phDecInfo_k].out_phase;
	pPackInfo->u8_msb[MSB_FILM_PHASE]              = phTable_DecInfoGen[phDecInfo_k].film_phase;
	pPackInfo->u8_msb[MSB_ME2_PPFV_BUF_IDX]        = phTable_DecInfoGen[phDecInfo_k].ppfv_offset;
	pPackInfo->u8_msb[MSB_ME2_IBME_P_INDEX_OFFSET] = phTable_DecInfoGen[phDecInfo_k].me2_p_offset;
	pPackInfo->u8_msb[MSB_ME2_IBME_I_INDEX_OFFSET] = phTable_DecInfoGen[phDecInfo_k].me2_i_offset;
	pPackInfo->u8_msb[MSB_ME2_PHASE]               = phTable_DecInfoGen[phDecInfo_k].me2_phase;
	pPackInfo->u8_msb[MSB_ME2_FIRST_PHASE]         = u1_is_firstPhase;
	pPackInfo->u8_msb[MSB_CRTC_MC_HF_INDEX]        = mc_hf_idx;

	pPackInfo->u8_lsb[LSB_CRTC_MC_LF_INDEX]        = mc_lf_idx;
	pPackInfo->u8_lsb[LSB_ME_LAST_PHASE]           = phTable_DecInfoGen[phDecInfo_k].me1_last;
	pPackInfo->u8_lsb[LSB_ME_DTS_P_INDEX]          = phTable_DecInfoGen[phDecInfo_k].me1_p_offset;
	pPackInfo->u8_lsb[LSB_ME_DTS_I_INDEX]          = phTable_DecInfoGen[phDecInfo_k].me1_i_offset;
	pPackInfo->u8_lsb[LSB_MC_MODE_HF_UPDATE]       = 1;
	pPackInfo->u8_lsb[LSB_MC_MODE_LF_UPDATE]       = 1;
	pPackInfo->u8_lsb[LSB_MC_CTS_P_INDEX]          = phTable_DecInfoGen[phDecInfo_k].mc_p_offset;
	pPackInfo->u8_lsb[LSB_MC_CTS_I_INDEX]          = phTable_DecInfoGen[phDecInfo_k].mc_i_offset;
	//pPackInfo->u8_lsb[LSB_MC_CTS_P_INDEX]          = phTable_DecInfoGen[phDecInfo_k].me2_p_offset;
	//pPackInfo->u8_lsb[LSB_MC_CTS_I_INDEX]          = phTable_DecInfoGen[phDecInfo_k].me2_i_offset;
	pPackInfo->u8_lsb[LSB_MC_PHASE]                = phTable_DecInfoGen[phDecInfo_k].mc_phase;
	pPackInfo->u8_lsb[LSB_MC_HF_INDEX]             = mc_hf_idx;
	pPackInfo->u8_lsb[LSB_MC_LF_INDEX]             = mc_lf_idx;
}

extern unsigned int GS_me_pqc_mode;
VOID   FRC_phTable_inDec_amend(unsigned char  cadence_id)
{
	unsigned char Len_end;
	Film_Wrt_Info     phTable_WrtInfo_param;
	unsigned char u8_k;

	if(cadence_id >= _FRC_CADENCE_NUM_){
		rtd_pr_memc_notice("[%s]Invalid u8_cadence_id:%d.Resume!\n",__FUNCTION__,cadence_id);
		cadence_id = _CAD_VIDEO;
	}

	for (u8_k = 0; u8_k < 64; u8_k ++)
	{
		phTable_WrtInfoGen_Amend[u8_k].me1_in_idx     = phTable_WrtInfoGen[u8_k].me1_in_idx ;
		phTable_WrtInfoGen_Amend[u8_k].me2_in_idx     = phTable_WrtInfoGen[u8_k].me2_in_idx ;
		phTable_WrtInfoGen_Amend[u8_k].mc_in_idx      = phTable_WrtInfoGen[u8_k].mc_in_idx ;
		if(GS_me_pqc_mode == 3){
			phTable_WrtInfoGen_Amend[u8_k].logo_en        = 1; // phTable_WrtInfoGen[u8_k].logo_en ;
		}else{
			phTable_WrtInfoGen_Amend[u8_k].logo_en        = phTable_WrtInfoGen[u8_k].logo_en ;
		}
		phTable_WrtInfoGen_Amend[u8_k].mc_finalPhase  = phTable_WrtInfoGen[u8_k].mc_finalPhase ;
	}
	Len_end = frc_cadTable[cadence_id].cad_outLen - 1;

	phTable_WrtInfo_param.me1_in_idx = phTable_WrtInfoGen_Amend[0].me1_in_idx ;
	phTable_WrtInfo_param.me2_in_idx = phTable_WrtInfoGen_Amend[0].me2_in_idx ;
	phTable_WrtInfo_param.mc_in_idx  = phTable_WrtInfoGen_Amend[0].mc_in_idx ;
	phTable_WrtInfo_param.logo_en    = phTable_WrtInfoGen_Amend[0].logo_en ;
	phTable_WrtInfo_param.mc_finalPhase  = phTable_WrtInfoGen_Amend[0].mc_finalPhase ;

	for(u8_k = 0; u8_k < Len_end; u8_k++)
	{
		phTable_WrtInfoGen_Amend[u8_k].me1_in_idx = phTable_WrtInfoGen_Amend[u8_k+1].me1_in_idx ;
		phTable_WrtInfoGen_Amend[u8_k].me2_in_idx = phTable_WrtInfoGen_Amend[u8_k+1].me2_in_idx ;
		phTable_WrtInfoGen_Amend[u8_k].mc_in_idx  = phTable_WrtInfoGen_Amend[u8_k+1].mc_in_idx ;
		phTable_WrtInfoGen_Amend[u8_k].logo_en    = phTable_WrtInfoGen_Amend[u8_k+1].logo_en ;
		phTable_WrtInfoGen_Amend[u8_k].mc_finalPhase  = phTable_WrtInfoGen_Amend[u8_k+1].mc_finalPhase ;
	}

	phTable_WrtInfoGen_Amend[Len_end].me1_in_idx = phTable_WrtInfo_param.me1_in_idx ;
	phTable_WrtInfoGen_Amend[Len_end].me2_in_idx = phTable_WrtInfo_param.me2_in_idx ;
	phTable_WrtInfoGen_Amend[Len_end].mc_in_idx  = phTable_WrtInfo_param.mc_in_idx ;
	phTable_WrtInfoGen_Amend[Len_end].logo_en    = phTable_WrtInfo_param.logo_en ;
	phTable_WrtInfoGen_Amend[Len_end].mc_finalPhase  = phTable_WrtInfo_param.mc_finalPhase ;
}

unsigned int  FRC_phTable_tool_commonDivisor(unsigned int v0, unsigned int v1)
{
	// based on LiuWei <<The Nine Chapters on the Mathematical Art>>
	if(v0 == 0 || v1 == 0)
	{
		rtd_pr_memc_notice("phTable:: illegal sys_N or cadence outLength, please check.");
		v0 = 1;
		v1 = 1;
	}

	while(v0 != v1)
	{
		if(v0 > v1)
		{
			v0 = v0 - v1;
		}
		else
		{
			v1 = v1 - v0;
		}
	}

	return v0;
}

//////////////////////////////////////////////////////////////////////////
VOID FRC_kphase_sram_clear(unsigned short stIdx, unsigned short endIdx)
{
	unsigned short u16_k;
	//unsigned int ee_addr_base = 0x00000000;//0xC0000000

	unsigned int ee_addr_base = KMC_TOP_kmc_top_00_reg;
	WriteRegister(ee_addr_base + 0x4F00, 0, 31, 0);     //Prepare write EEPROM
	WriteRegister(ee_addr_base + 0x4F00, 0, 31, 0x80000000 | stIdx);
	for (u16_k = stIdx; u16_k <= endIdx; u16_k ++)
	{
		unsigned int pack_MSB = 0, pack_LSB = 0;

		WriteRegister(ee_addr_base + 0x4F04, 0, 31, pack_LSB);
		WriteRegister(ee_addr_base + 0x4F08, 0, 31, pack_MSB);
	}
}

VOID FRC_kphase_sram_init()
{
	FRC_kphase_sram_clear(0, 255);

	// change LUT selection.
	{
		unsigned int u32_Tmp;
		ReadRegister(KPHASE_kphase_10_reg,16,16, &u32_Tmp);
		u32_Tmp = ((u32_Tmp & 0x01) == 1)? 0 : 1;
		WriteRegister(KPHASE_kphase_10_reg,16,16, u32_Tmp);
	}
	FRC_kphase_sram_clear(0, 255);
}

VOID FRC_phTable_Init_1n2m_video_22_32_intable()
{
	FRC_phTable_WrtStep_VideoMode_Gen_idx(frc_cadTable[_CAD_VIDEO], 0);
	FRC_phTable_WrtStep_VideoMode_Gen_idx(frc_cadTable[_CAD_22], 1);
	FRC_phTable_WrtStep_VideoMode_Gen_idx(frc_cadTable[_CAD_32], 3);

	FRC_phTable_inDec_Wrt_init();
}

VOID   FRC_phTable_inDec_Wrt_init( )  // bai need change  need know 64 table
{
	//const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u8_k = 0;
	unsigned char Len_vid_22_32;

	//Film_Wrt_Info phTable_Wrt_param;

	Len_vid_22_32 = frc_cadTable[_CAD_VIDEO].cad_outLen + frc_cadTable[_CAD_22].cad_outLen + frc_cadTable[_CAD_32].cad_outLen;
	for (u8_k = 0; u8_k < Len_vid_22_32; u8_k = u8_k + 4)
	{
		unsigned int u32_phT_wrtStep_pack = phTable_WrtStep_pack_vd_22_32(u8_k);
		WriteRegister(KPHASE_kphase_14_reg + u8_k, 0, 31, u32_phT_wrtStep_pack);
	}
	for (u8_k = 0; u8_k < Len_vid_22_32; u8_k = u8_k + 8)
	{
		unsigned int u32_phT_wrtStep_pack = phTable_me2_WrtStep_pack_vd_22_32(u8_k);
		WriteRegister(KPHASE_kphase_58_reg + u8_k, 0, 31, u32_phT_wrtStep_pack);
	}

}

int g_memc_VR360_state = 0;
extern unsigned char g_cadence_change_flag;
VOID FRC_phTable_Cadence_Change(const _PARAM_FRC_PH_TABLE *pParam, _OUTPUT_FRC_PH_TABLE *pOutput)
{
 	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int adapt_stream_flag = 0;
	unsigned int cur_cadence = 0;
	unsigned int me_write = 0, me1_read_i = 0, me1_read_p = 0, me2_read_i = 0, me2_read_p = 0;
	unsigned int mc_write = 0, mc_read_i = 0, mc_read_p = 0;
	unsigned int me2_phase = 0, mc_phase = 0;
	static unsigned int cadence_pre = 0;
	unsigned int log_en = 0, gmv_rb = 0;
	PQL_OUTPUT_FRAME_RATE  out_fmRate = 0;

	adapt_stream_flag = MEMC_Lib_get_Adaptive_Stream_Flag();
	cur_cadence = s_pContext->_output_filmDetectctrl.u8_cur_cadence_Id[_FILM_ALL];
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
	gmv_rb = (signed short)s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	out_fmRate = s_pContext->_external_data._output_frameRate;

	cadence_pre = cur_cadence;

	if(log_en){
		rtd_pr_memc_notice("[phasetable][in interrupt][,%d,%d,][,%d,][GMV,%d,][ME1_idx,%d,%d,%d,ME2_idx,%d,%d,][MC_idx,%d,%d,%d,][phase,%d,%d,][Motion,%d,][Seq,%d,%x][FrameRate,%d,][Time,%d,]\n\r",
			s_pContext->_output_filmDetectctrl.u8_det_cadence_Id[_FILM_ALL], cur_cadence, s_pContext->_output_filmDetectctrl.u8_phT_phase_Idx_out[_FILM_ALL], adapt_stream_flag,
			me_write, me1_read_i, me1_read_p, me2_read_i, me2_read_p,
			mc_write, mc_read_i, mc_read_p, me2_phase, mc_phase,
			s_pContext->_output_read_comreg.u27_ipme_aMot_rb, 
			s_pContext->_output_filmDetectctrl.u32_Mot_sequence[_FILM_ALL]&0x1, s_pContext->_output_filmDetectctrl.u32_Mot_sequence[_FILM_ALL]&0xFFFF, 
			rtd_inl(PPOVERLAY_uzudtg_DVS_cnt_reg), rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
	}

}

