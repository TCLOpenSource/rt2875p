#include "memc_isr/PQL/PQLPlatformDefs.h"
//#include "stdio.h"
//#include "stdlib.h"
#include "memc_isr/PQL/PQLContext.h"
//#include "MESceneAnalysis.h"
#include "rtk_vip_logger.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include "memc_reg_def.h"
#include <rtd_log/rtd_module_log.h>
#include "memc_isr/PQL/MEMC_ParamTable.h"

#define MEMC_EN_AI (0)
#if MEMC_EN_AI
#include <ioctrl/ai/ai_cmd_id.h>
#endif

#ifndef rtd_inl
#define rtd_inl(offset)		(*(volatile unsigned long *)(offset))
#endif
//#undef VIPprintf
//#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP30_DEBUG,fmt,##args)

extern unsigned int MA_print_count;
#define ROSPrintf_MEMC_IP30(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP30_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP30_interface1(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP30_interface1_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP30_interface2(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP30_interface2_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP30_interface3(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP30_interface3_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP30_interface4(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP30_interface4_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP30_interface5(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP30_interface5_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP30_interface6(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP30_interface6_DEBUG,MA_print_count,fmt,##args)
#define ROSPrintf_MEMC_IP30_interface7(fmt, args...) VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_MEMC_IP30_interface7_DEBUG,MA_print_count,fmt,##args)
#define MEMC_IP30_NULL_Print_Cnt (300)


//////////////////////////////////////////////////////////////////////////

#define FRAME_CHANGE_CNT	5
extern _MEMC_PARAM_TABLE MEMC_PARAM_TABLE;
extern unsigned char scalerVIP_Get_MEMCPatternFlag_Identification(unsigned short prj,unsigned short nNumber);

//////////////////////////////////////////////////////////////////////////

VOID Me_sceneAnalysis_Init(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	/* SIS MEMC Initial here */
	MEMC_MiddleWare_StatusInit(pOutput);

	pOutput->u1_RP_detect_true = 0;
	pOutput->u1_Avenger_RP_detect_true =0;

	pOutput->s10_me_GMV_1st_vy_rb_iir = 0;
	pOutput->u6_me_GMV_1st_cnt_rb_iir = 0;
	pOutput->u1_chaos_true = 0;
    pOutput->u8_chaos_apply = 0;
	pOutput->u8_big_apldiff_hold_cnt = 0;
	pOutput->u32_glb_sad_pre = 0;
	pOutput->u32_big_apldiff_saddiff_th = 0;
	pOutput->u8_big_apldiff_sc_hold_cnt = 1;
	pOutput->u32_big_apldiff_sad_pre = 0;
	pOutput->u8_pure_TopDownPan_cnt = 0;

	pOutput->u3_Dynamic_MVx_Step = 0;
	pOutput->u3_Dynamic_MVy_Step = 0;

	pOutput->u2_panning_flag = 0;

	pOutput->u8_panning_cnt = 0;

	// ME median filter: criteria debounce counter
	pOutput->u8_panning_dbCnt = 0;
	pOutput->u8_prd_dbCnt = 0;
	pOutput->u8_big_relMV_dbCnt = 0;
	pOutput->u8_rChaos_dbCnt = 0;	
	pOutput->u1_still_highfrequency_image_detect=0;
}

VOID Me_sceneAnalysis_Proc_OutputIntp(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput, int iSliceIdx)
{
	MEMC_MiddleWare_StatusUpdate(pParam, pOutput, iSliceIdx);
}

VOID MEMC_MiddleWare_CRTC_StatusInit(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	crtc1_crtc1_20_RBUS crtc1_crtc1_20;
	crtc1_crtc1_28_RBUS crtc1_crtc1_28;
	crtc1_crtc1_98_RBUS crtc1_crtc1_98;

	crtc1_crtc1_20.regValue = rtd_inl(CRTC1_CRTC1_20_reg);
	crtc1_crtc1_28.regValue = rtd_inl(CRTC1_CRTC1_28_reg);
	crtc1_crtc1_98.regValue = rtd_inl(CRTC1_CRTC1_98_reg);

	pOutput->SIS_MEMC.MEMC_STATUS.reg_crtc1_frc_vtrig_dly = crtc1_crtc1_20.crtc1_frc_vtrig_dly;
	pOutput->SIS_MEMC.MEMC_STATUS.reg_crtc1_mc_vtrig_dly_long = crtc1_crtc1_98.crtc1_mc_vtrig_dly_long;
	pOutput->SIS_MEMC.MEMC_STATUS.reg_crtc1_lbmc_vtrig_dly = crtc1_crtc1_28.crtc1_lbmc_vtrig_dly;
}

/* SIS MEMC function implement here */
VOID MEMC_MiddleWare_StatusInit(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	pOutput->SIS_MEMC.Frame_Couter = 0;

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 1, 1,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand0_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 2, 2,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand1_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 3, 3,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand2_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 4, 4,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 5, 5,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 6, 6,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand5_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 7, 7,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand6_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 8, 8,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand7_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 9, 9,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand8_en);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 0, 0,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand0_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 1, 1,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand1_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 2, 2,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand2_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 3, 3,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 4, 4,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 5, 5,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand5_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 6, 6,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand6_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 7, 7,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand7_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 8, 8,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand8_en);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 11, 11, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 12, 12, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 13, 13, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 14, 14, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update3_en);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 10, 10, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 11, 11, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 12, 12, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 13, 13, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update3_en);

	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 0, 7, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st0);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 8, 15, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st1);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 16, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st2);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_1C_reg, 24, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st3);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 0, 7, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st4);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 8, 15, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st5);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 16, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st6);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_20_reg, 24, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_st7);

	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 0, 7, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st0);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 8, 15, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st1);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 16, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st2);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_5C_reg, 24, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st3);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 0, 7, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st4);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 8, 15, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st5);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 16, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st6);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_60_reg, 24, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_st7);

	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 16, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd0);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_24_reg, 24, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd1);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 0, 7, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd2);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 8, 15, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_rnd3);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_2C_reg, 0, 12, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_gmv);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 16, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd0);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_64_reg, 24, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd1);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 0, 7, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd2);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 8, 15, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_rnd3);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_6C_reg, 0, 12, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_gmv);

	ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_00_reg, 0, 0, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmv_sel);
	ReadRegister(HARDWARE_HARDWARE_18_reg, 2, 2, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_sel);

	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_16_reg, 12, 21, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_x1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_16_reg, 22, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_x2);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_17_reg, 0, 9, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_x3);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_17_reg, 16, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_y1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_17_reg, 24, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_y2);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_18_reg, 0, 7, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_y3);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_18_reg, 8, 18, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_slope1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_18_reg, 20, 30, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_ucf_slope2);

	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_19_reg, 0, 11, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_x1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_19_reg, 12, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_x2);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_20_reg, 0, 11, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_x3);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_20_reg, 16, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_y1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_20_reg, 24, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_y2);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_21_reg, 0, 7, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_y3);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_21_reg, 8, 18, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_slope1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_21_reg, 20, 30, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_gmvd_cnt_slope2);

	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_4C_reg, 0, 12, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_gmvd_cost_limt);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_0C_reg, 0, 12, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_gmvd_cost_limt);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_78_reg, 15, 27, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me2_ph_gmvd_cost_limt);

	ReadRegister(KME_IPME_KME_IPME_8C_reg, 0, 8, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp0);
	ReadRegister(KME_IPME_KME_IPME_8C_reg, 9, 17, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp1);
	ReadRegister(KME_IPME_KME_IPME_8C_reg, 18, 26, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp2);
	ReadRegister(KME_IPME_KME_IPME_90_reg, 0, 8, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp3);
	ReadRegister(KME_IPME_KME_IPME_90_reg, 9, 17, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp4);
	ReadRegister(KME_IPME_KME_IPME_90_reg, 18, 26, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp5);
	ReadRegister(KME_IPME_KME_IPME_94_reg, 0, 8, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_h_flp_alp6);

	ReadRegister(KME_IPME_KME_IPME_94_reg, 9, 17, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp0);
	ReadRegister(KME_IPME_KME_IPME_94_reg, 18, 26, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp1);
	ReadRegister(KME_IPME_KME_IPME_98_reg, 0, 8, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp2);
	ReadRegister(KME_IPME_KME_IPME_98_reg, 9, 17, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp3);
	ReadRegister(KME_IPME_KME_IPME_98_reg, 18, 26, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp4);
	ReadRegister(KME_IPME_KME_IPME_04_reg, 0, 8, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp5);
	ReadRegister(KME_IPME_KME_IPME_04_reg, 9, 17, &pOutput->SIS_MEMC.MEMC_STATUS.reg_ipme_v_flp_alp6);

	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg, 0, 9, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_x1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg, 10, 19, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_x2);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_06_reg, 20, 29, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_x3);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg, 0, 9, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_y1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg, 10, 19,&pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_y2);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_07_reg, 20, 29, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_y3);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg, 0, 12,&pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_slope1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_08_reg, 13, 25, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_mvd_cuv_slope2);

	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 0, 5,  &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y1);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 8, 13,  &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y2);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 14, 19, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y3);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_23_reg, 20, 25, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_y4);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_24_reg, 9, 17, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_adptpnt_rnd_slope2);


	ReadRegister(MC_MC_30_reg, 10, 10, &pOutput->SIS_MEMC.MEMC_STATUS.reg_mc_scene_change_fb);
	ReadRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_sc_saddiff_th);
	ReadRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, &pOutput->u32_new_saddiff_th);
	ReadRegister(KME_ME1_TOP1_ME1_SCENE1_07_reg, 0, 30, &pOutput->u32_new_saddiff_th_pre);

	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,0,7, &pOutput->SIS_MEMC.MEMC_STATUS.u8_dynME_acdc_bld_x_act[0]);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,8,15, &pOutput->SIS_MEMC.MEMC_STATUS.u8_dynME_acdc_bld_x_act[1]);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,16,23, &pOutput->SIS_MEMC.MEMC_STATUS.u8_dynME_acdc_bld_x_act[2]);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_00_reg,24,29, &pOutput->SIS_MEMC.MEMC_STATUS.u6_dynME_acdc_bld_y_act[0]);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,0,5, &pOutput->SIS_MEMC.MEMC_STATUS.u6_dynME_acdc_bld_y_act[1]);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,6,11, &pOutput->SIS_MEMC.MEMC_STATUS.u6_dynME_acdc_bld_y_act[2]);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,12,20, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.s9_dynME_acdc_bld_s_act[0]);
	ReadRegister(KME_ME1_TOP1_ME1_COMMON1_01_reg,21,29, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.s9_dynME_acdc_bld_s_act[1]);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_04_reg,24,31, &pOutput->SIS_MEMC.MEMC_STATUS.u8_dynME_ac_pix_sad_limt_act);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_44_reg,24,31, &pOutput->SIS_MEMC.MEMC_STATUS.u8_dynME_ac_pix_sad_limt_act);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_88_reg,0,3, &pOutput->SIS_MEMC.MEMC_STATUS.u4_acdc_bld_psad_shift_act);

	#if PQL_DYNAMIC_ADJUST_FIX_ISSUE
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,0,7, &pOutput->SIS_MEMC.MEMC_STATUS.u8_dynME2_acdc_bld_x_act[0]);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,8,15, &pOutput->SIS_MEMC.MEMC_STATUS.u8_dynME2_acdc_bld_x_act[1]);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,16,23, &pOutput->SIS_MEMC.MEMC_STATUS.u8_dynME2_acdc_bld_x_act[2]);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_50_reg,24,29, &pOutput->SIS_MEMC.MEMC_STATUS.u6_dynME2_acdc_bld_y_act[0]);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,0,5, &pOutput->SIS_MEMC.MEMC_STATUS.u6_dynME2_acdc_bld_y_act[1]);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,6,11, &pOutput->SIS_MEMC.MEMC_STATUS.u6_dynME2_acdc_bld_y_act[2]);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,12,20, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.s9_dynME2_acdc_bld_s_act[0]);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_54_reg,21,29, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.s9_dynME2_acdc_bld_s_act[1]);
	#endif
	ReadRegister(IPPRE_IPPRE_04_reg, 4, 11, &pOutput->SIS_MEMC.MEMC_STATUS.reg_kmc_blend_y_alpha);
	ReadRegister(IPPRE_IPPRE_04_reg, 16, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_kmc_blend_uv_alpha);
	ReadRegister(IPPRE_IPPRE_0C_reg, 0, 7, &pOutput->SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_y_alpha);
	ReadRegister(IPPRE_IPPRE_0C_reg, 9, 16, &pOutput->SIS_MEMC.MEMC_STATUS.reg_kmc_blend_logo_uv_alpha);
	#if (IC_K5LP || IC_K6LP || IC_K8LP)
	ReadRegister(KME_ME1_TOP0_KME_ME1_DRP_2_reg, 20, 29, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y2);
	ReadRegister(KME_ME1_TOP0_KME_ME1_DRP_2_reg, 10, 19, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y1);
	ReadRegister(KME_ME1_TOP0_KME_ME1_DRP_2_reg, 0, 9, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_drp_cuv_y0);
	#endif
	
	ReadRegister(KME_DEHALO_KME_DEHALO_F0_reg,24, 31, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x1);
	ReadRegister(KME_DEHALO_KME_DEHALO_F4_reg,24, 31, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_x2);
	ReadRegister(KME_DEHALO_KME_DEHALO_F8_reg,24, 31, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y1);
	ReadRegister(KME_DEHALO_KME_DEHALO_FC_reg,0, 7, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_dh_dtl_curve_y2);

	ReadRegister(MC_MC_B8_reg, 12, 15, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_mc_sobj_ymin0);
	ReadRegister(MC_MC_BC_reg, 16, 19, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_mc_sobj_ymin1);
	ReadRegister(MC_MC_BC_reg, 8, 13, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_mc_sobj_slop1);

	//=====  me2  small_object
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_D4_reg, 17, 29, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_sad_th);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_D8_reg, 0, 9, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_sm_mv_th);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_D8_reg, 10, 15, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_ip_pi_mvdiff_gain);
	ReadRegister(KME_ME2_CALC0_KME_ME2_CALC0_D8_reg, 16, 21, &pOutput->SIS_MEMC.MEMC_STATUS.reg_me2_sec_small_object_ph_mvdiff_gain);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 0, 3, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_x);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 4, 7, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask0_y);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 8,11, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_x);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 12, 15, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask1_y);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 16, 19, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_x);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 20, 23, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask2_y);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 24, 27, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_x);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_0C_reg, 28, 31, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask3_y);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 0, 3, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_x);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 4, 7, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask0_y);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 8,11, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_x);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 12, 15, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask1_y);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 16, 19, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_x);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 20, 23, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask2_y);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 24, 27, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_x);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_5C_reg, 28, 31, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask3_y);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  0,  2, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand0_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  3,  5, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand0_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  6,  8, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand1_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,  9, 11, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand1_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 12, 14, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand2_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 15, 17, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand2_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 18, 20, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 21, 23, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand3_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 24, 26, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg, 27, 29, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand4_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  0,  2, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand5_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 3, 5, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand5_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,  6,  8, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand6_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 9, 11, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand6_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 12, 14, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand7_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg, 15, 17, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_cand7_offsety);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  0,  2, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand0_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  3,  5, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand0_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  6,  8, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand1_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,  9, 11, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand1_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 12, 14, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand2_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 15, 17, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand2_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 18, 20, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 21, 23, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand3_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 24, 26, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg, 27, 29, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand4_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  0,  2, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand5_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 3, 5, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand5_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,  6,  8, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand6_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 9, 11, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand6_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 12, 14, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand7_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg, 15, 17, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_cand7_offsety);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  0,  2, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  3,  5, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update0_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  6,  8, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,  9, 11, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update1_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 12, 14, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 15, 17, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update2_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 18, 20, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update3_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg, 21, 23, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_update3_offsety);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  0,  2, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  3,  5, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update0_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  6,  8, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,  9, 11, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update1_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 12, 14, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 15, 17, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update2_offsety);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 18, 20, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update3_offsetx);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg, 21, 23, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_update3_offsety);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_zmv_en);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_zmv_en);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_ip_cddpnt_zmv);
	ReadRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, (unsigned int *)&pOutput->SIS_MEMC.MEMC_STATUS.reg_me1_pi_cddpnt_zmv);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_invalid_cnt_th);

	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 22, 22, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_dc_obme_mode_sel);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 23, 23, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_dc_obme_mode_sel);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 18, 19, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_dc_obme_mode);
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_8C_reg, 20, 21, &pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_dc_obme_mode);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_90_reg, 0, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_dehalo2_90);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_A8_reg, 0, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_dehalo2_A8);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_E8_reg, 0, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_dehalo2_E8);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_14_reg, 0, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_dehalo2_14);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_18_reg, 0, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_dehalo2_18);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_1C_reg, 0, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_dehalo2_1C);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_20_reg, 0, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_dehalo2_20);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_24_reg, 0, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_dehalo2_24);
	ReadRegister(KME_DEHALO2_KME_DEHALO2_28_reg, 0, 31, &pOutput->SIS_MEMC.MEMC_STATUS.reg_dehalo2_28);
	
/*
	unsigned int u32_rb_data;

	pOutput->SIS_MEMC.MEMC_WIDTH = 3840;
	pOutput->SIS_MEMC.MEMC_HEIGHT = 2160;

	pOutput->SIS_MEMC.MEMC_BLOCK_X = 240;
	pOutput->SIS_MEMC.MEMC_BLOCK_Y = 135;

	pOutput->SIS_MEMC.Frame_Couter = 0;

	// SIS_MEMC initial
	pOutput->SIS_MEMC.MEMC_PAN_STATUS.wrt_idx = 0;

	PQL_ReadHDF(FRC_TOP__KME_ME_TOP1__reg_metop_smallobj_en_ADDR no mat,FRC_TOP__KME_ME_TOP1__reg_metop_smallobj_en_BITSTART no mat,FRC_TOP__KME_ME_TOP1__reg_metop_smallobj_en_BITEND no mat,&pOutput->SIS_MEMC.MEMC_PAN_STATUS.reg_metop_smallobj_en);

	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_lfsr_mask_x_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_lfsr_mask_x_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_lfsr_mask_x_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ph_1st_lfsr_mask_x);
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_lfsr_mask_y_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_lfsr_mask_y_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_lfsr_mask_y_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ph_1st_lfsr_mask_y);
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_lfsr_mask_x_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_lfsr_mask_x_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_lfsr_mask_x_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ph_2nd_lfsr_mask_x);
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_lfsr_mask_y_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_lfsr_mask_y_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_lfsr_mask_y_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ph_2nd_lfsr_mask_y);
	PQL_ReadHDF(MC2_MC2_50_reg,0,1,&pOutput->SIS_MEMC.MEMC_STATUS.reg_mc_var_lpf_en);
	PQL_ReadHDF(MC2_MC2_54_reg,0,7,&pOutput->SIS_MEMC.MEMC_STATUS.reg_mc_var_lpf_mvd_x_th);

	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ip_1st_lfsr_mask_x_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_1st_lfsr_mask_x_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_1st_lfsr_mask_x_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask_x);
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ip_1st_lfsr_mask_y_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_1st_lfsr_mask_y_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_1st_lfsr_mask_y_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_1st_lfsr_mask_y);
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_lfsr_mask_x_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_lfsr_mask_x_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_lfsr_mask_x_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_2nd_lfsr_mask_x);
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_lfsr_mask_y_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_lfsr_mask_y_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_lfsr_mask_y_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_ip_2nd_lfsr_mask_y);

	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_pi_1st_lfsr_mask_x_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_1st_lfsr_mask_x_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_1st_lfsr_mask_x_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask_x);
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_pi_1st_lfsr_mask_y_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_1st_lfsr_mask_y_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_1st_lfsr_mask_y_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_1st_lfsr_mask_y);
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_lfsr_mask_x_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_lfsr_mask_x_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_lfsr_mask_x_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_2nd_lfsr_mask_x);
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_lfsr_mask_y_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_lfsr_mask_y_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_lfsr_mask_y_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_vbuf_pi_2nd_lfsr_mask_y);

	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_update0_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_update0_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_update0_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_ip_update0_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_update1_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_update1_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_update1_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_ip_update1_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ss_pi_update0_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_pi_update0_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_pi_update0_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_pi_update0_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ss_pi_update1_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_pi_update1_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_pi_update1_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_pi_update1_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ss_ph_update0_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_ph_update0_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_ph_update0_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_ph_update0_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ss_ph_update1_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_ph_update1_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ss_ph_update1_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_ph_update1_penalty);

	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ds_ip_update0_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_ip_update0_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_ip_update0_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_ip_update0_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ds_ip_update1_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_ip_update1_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_ip_update1_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_ip_update1_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ds_pi_update0_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_pi_update0_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_pi_update0_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_pi_update0_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ds_pi_update1_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_pi_update1_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_pi_update1_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_pi_update1_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ds_ph_update0_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_ph_update0_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_ph_update0_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_ph_update0_penalty);
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ds_ph_update1_penalty_ADDR no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_ph_update1_penalty_BITSTART no mat,FRC_TOP__KME_ME_TOP__reg_metop_ds_ph_update1_penalty_BITEND no mat,&pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_ph_update1_penalty);

	PQL_ReadHDF(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,6,8, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ip_1st_cand1_offsetx = u32_rb_data;
	PQL_ReadHDF(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,0,2, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ip_1st_cand5_offsetx = u32_rb_data;
	PQL_ReadHDF(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,6,8, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ip_1st_update1_offsetx = u32_rb_data;

	PQL_ReadHDF(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,6,8, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_pi_1st_cand1_offsetx = u32_rb_data;
	PQL_ReadHDF(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,0,2, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_pi_1st_cand5_offsetx = u32_rb_data;
	PQL_ReadHDF(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,6,8, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_pi_1st_update1_offsetx = u32_rb_data;

	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand1_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand1_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand1_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_1st_cand1_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand2_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand2_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand2_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_1st_cand2_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand2_offsety_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand2_offsety_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand2_offsety_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_1st_cand2_offsety = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand5_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand5_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_cand5_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_1st_cand5_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_update1_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_update1_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_1st_update1_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_1st_update1_offsetx = u32_rb_data;

	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_cand1_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_cand1_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_cand1_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ip_2nd_cand1_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_cand5_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_cand5_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_cand5_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ip_2nd_cand5_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_update1_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_update1_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ip_2nd_update1_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ip_2nd_update1_offsetx = u32_rb_data;

	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_cand1_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_cand1_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_cand1_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_pi_2nd_cand1_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_cand5_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_cand5_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_cand5_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_pi_2nd_cand5_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_update1_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_update1_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_pi_2nd_update1_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_pi_2nd_update1_offsetx = u32_rb_data;

	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand1_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand1_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand1_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_2nd_cand1_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand2_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand2_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand2_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_2nd_cand2_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand2_offsety_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand2_offsety_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand2_offsety_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_2nd_cand2_offsety = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand5_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand5_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_cand5_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_2nd_cand5_offsetx = u32_rb_data;
	PQL_ReadHDF(FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_update1_offsetx_ADDR no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_update1_offsetx_BITSTART no mat,FRC_TOP__KME_VBUF_TOP__vbuf_ph_2nd_update1_offsetx_BITEND no mat, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.u3_ph_2nd_update1_offsetx = u32_rb_data;

	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_sad_th_mode_sel_ADDR, 0, 31, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_sad_th_mode_sel = u32_rb_data&0x3;

	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ss_ip_penalty_z_ADDR, 0, 31, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_ip_penalty_z = u32_rb_data&0xff;
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_ip_penalty_g = (u32_rb_data>>8)&0xff;
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ss_pi_penalty_z_ADDR, 0, 31, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_pi_penalty_z = u32_rb_data&0xff;
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_pi_penalty_g = (u32_rb_data>>8)&0xff;
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ss_ph_penalty_z_ADDR, 0, 31, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_ph_penalty_z = u32_rb_data&0xff;
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ss_ph_penalty_g = (u32_rb_data>>8)&0xff;

	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ds_ip_penalty_z_ADDR, 0, 31, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_ip_penalty_z = u32_rb_data&0xff;
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_ip_penalty_g = (u32_rb_data>>8)&0xff;
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ds_pi_penalty_z_ADDR, 0, 31, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_pi_penalty_z = u32_rb_data&0xff;
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_pi_penalty_g = (u32_rb_data>>8)&0xff;
	PQL_ReadHDF(FRC_TOP__KME_ME_TOP__reg_metop_ds_ph_penalty_z_ADDR, 0, 31, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_ph_penalty_z = u32_rb_data&0xff;
	pOutput->SIS_MEMC.MEMC_STATUS.reg_metop_ds_ph_penalty_g = (u32_rb_data>>8)&0xff;

	PQL_ReadHDF(FRC_TOP__DEHALO__reg_dh_empty_proc_en_ADDR, 0, 31, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.reg_dh_empty_proc_en = (u32_rb_data>>13)&0x1;

	PQL_ReadHDF(FRC_TOP__PQL_1__pql_fb_tc_iir_ADDR, 0, 7, &u32_rb_data);
	pOutput->SIS_MEMC.MEMC_STATUS.reg_pql_fb_tc_iir = (u32_rb_data)&0xff;
*/
}

VOID MEMC_MiddleWare_Reg_Status_Cad1(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	//register value get when candence is 1

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char  u8_cadence_phId  = s_pContext->_output_filmDetectctrl.u8_phT_phase_Idx_out[_FILM_ALL];
	unsigned char  u8_cadence_Id    = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	unsigned char  u1_cad_motionSeq    = 0;//cadence_seqFlag(frc_cadTable[u8_cadence_Id], cadence_prePhase(frc_cadTable[u8_cadence_Id],u8_cadence_phId, 1));

	unsigned int u32_RB_val;

	if(u8_cadence_Id == _CAD_32322){
		u1_cad_motionSeq  = cadence_seqFlag(frc_cadTable[u8_cadence_Id], ((cadence_prePhase(frc_cadTable[u8_cadence_Id],u8_cadence_phId, 1)+6)%12));
	}else{
		u1_cad_motionSeq = cadence_seqFlag(frc_cadTable[u8_cadence_Id], cadence_prePhase(frc_cadTable[u8_cadence_Id],u8_cadence_phId, 1));
	}

	if(u8_cadence_Id == _CAD_32322){
		u1_cad_motionSeq  = cadence_seqFlag(frc_cadTable[u8_cadence_Id], ((cadence_prePhase(frc_cadTable[u8_cadence_Id],u8_cadence_phId, 1)+6)%12));
	}else{
		u1_cad_motionSeq = cadence_seqFlag(frc_cadTable[u8_cadence_Id], cadence_prePhase(frc_cadTable[u8_cadence_Id],u8_cadence_phId, 1));
	}

	if(u1_cad_motionSeq==1)
	{
		pOutput->SIS_MEMC.MEMC_STATUS.u25_rdbk_dtl_all_pre = pOutput->SIS_MEMC.MEMC_STATUS.u25_rdbk_dtl_all;
		ReadRegister(KME_ME1_TOP6_KME_ME1_TOP6_D8_reg,0,25, &u32_RB_val);
		pOutput->SIS_MEMC.MEMC_STATUS.u25_rdbk_dtl_all = u32_RB_val&0x1ffffff;


		pOutput->SIS_MEMC.MEMC_STATUS.u25_rdbk_dtl_all_intervel5_pre = pOutput->SIS_MEMC.MEMC_STATUS.u25_rdbk_dtl_all_intervel5;
		pOutput->SIS_MEMC.MEMC_STATUS.u25_rdbk_dtl_all_intervel5 = pOutput->SIS_MEMC.MEMC_STATUS.u25_rdbk_dtl_all;




			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 0, 3, pOutput->SIS_MEMC.MEMC_STATUS.u1_apl_inc);
			//WriteRegister(KPOST_TOP_KPOST_TOP_A0_reg, 4, 7, pOutput->SIS_MEMC.MEMC_STATUS.u1_apl_dec);

	}
}

VOID MEMC_MiddleWare_StatusUpdate(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput, int iSliceIdx)
{
	if (pOutput->SIS_MEMC.Frame_Couter > 65535)
		pOutput->SIS_MEMC.Frame_Couter = 0;
	else
		pOutput->SIS_MEMC.Frame_Couter++;

	MEMC_HVDtl_Move_detect(pParam, pOutput);
	if (iSliceIdx == 0)
	{

	}
	else if (iSliceIdx == 1)
	{

	}
	else if (iSliceIdx == 2)
	{
	}
	else if (iSliceIdx == 3)
	{
		MEMC_LightSwordDetect(pParam, pOutput);
	}
	else if (iSliceIdx == 4)
	{
		MEMC_Natural_HighFreq_detect(pParam, pOutput);
		MEMC_Periodic_detect(pParam, pOutput);
	}
	else
	{
		// don't use function put here
	}
	MEMC_Still_Highfrequency_Image_Detect(pParam, pOutput);
	MEMC_Region_Periodic_detect(pParam, pOutput);
	MEMC_Big_APLDiff_Detect(pParam, pOutput);
	MEMC_EPG_Protect(pParam, pOutput);
	MEMC_Simple_Scene_Detect(pParam, pOutput);
	MEMC_PureTopDownPanning_Detect(pParam, pOutput);
	MEMC_BoundaryHPanning_Detect(pParam, pOutput);
	MEMC_DynamicSearchRange_detect(pParam, pOutput);
	MEMC_Over_SearchRange_detect(pParam, pOutput);
	MEMC_Relative_Motion_detect(pParam, pOutput);	
	MEMC_Occl_Ext_Detect(pParam, pOutput);
	MEMC_RMV_Detect_DTV_Ch2_1_tv006(pOutput);	
	MEMC_Motion_Info_Detect(pOutput);
	MEMC_special_DRP_Detect(pOutput);
	MEMC_MAD_detect(pOutput);
	MEMC_Repeat_Periodic_detect(pParam, pOutput);
	MEMC_boundary_mv_convergence_Detect(pOutput);
	MEMC_Sport_Scene_Detect(pParam, pOutput);
	// MEMC_ME_OFFSET_Proc(pParam, pOutput);
	MEMC_info_Print(pOutput);
	#ifdef BG_MV_GET
	MEMC_BG_MV_Get(pParam, pOutput);//ED
	#endif
	MEMC_NearRim_Vpan_detect(pParam, pOutput);
	MEMC_CMR_pattern_protect(pParam, pOutput);
	MEMC_Brightness_Chg_Detect(pParam, pOutput);
	MEMC_Small_Motion_Detect_K24987(pParam, pOutput);
	MEMC_Panning_Adjust_obme(pParam, pOutput);
}

static unsigned int frm_cnt = 0;
VOID MEMC_RepeatPattern_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	static unsigned char u6_RP_out_print = 0;
	static unsigned char u6_RP_frm_in_cnt = 0;
	static unsigned char u6_RP_frm_in_cnt_thl = 8;
	static unsigned char u8_RP_frm_hold_cnt = 0;
	static unsigned char u8_RP_frm_hold_cnt_max = 0;
	unsigned short u16_RP_frm_hold_thl = 120;
	unsigned char u1_det_true;
	unsigned char u1_basic_cond_true;
	static unsigned char u1_RP_detect_true_pre = 0;

	unsigned short u16_mcDTL_thmin = 0x180;
	unsigned short u16_mcDTL_th0 = 0x230;
	unsigned short u16_mcDTL_th1 = 0x340;
	unsigned short u16_mcDTL_th2 = 0x3d0;
	unsigned short u16_mcDTL_th3 = 0x400;
	unsigned char u8_mcDTL_offset = 0;
	unsigned char u8_me_aDTL_ShiftBit = 11 ;

	unsigned char u8_GMV_th0 = 0xA;
	unsigned char u8_GMV_th1 =0x18;
	unsigned char u8_GMV_th2 = 0x23;
	unsigned char u8_GMV_diff = 0x6;

	signed short s11_maxGMV_x = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>2;
	signed short s11_secGMV_x = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb)>>2;

	signed short s10_maxGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>2;
	signed short s10_secGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb)>>2;

	unsigned short u16_mcHdtl = s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt;
	unsigned short u16_mcVdtl = s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt;

	unsigned int u19_me_statis_glb_prd= s_pContext->_output_read_comreg.u19_me_statis_glb_prd;

//	unsigned char u1_gmv_true = 1, u1_mcDtl_move_true = 1, u1_tc_true = 1, u1_rim_true = 1, u1_dtl_all_cond_true = 1;
	unsigned char u1_gmv_true = 1, u1_mcDtl_move_true = 1, u1_tc_true = 1, u1_rDTL_move_true = 1,u1_rPRD_move_true = 1, u1_dtl_all_cond_true = 1;
	unsigned char u2_mcDtl_HV;	// 0: dtl small, 1: v dtl, 2: h dtl

	unsigned char u1_maxGMV_x_t;
	unsigned char u1_secGMV_x_t;
	unsigned char u1_maxGMV_y_t;
	unsigned char u1_secGMV_y_t;
	unsigned char u1_GMV32_t;

	unsigned char u1_black_sc=0;		//black scene or sc occur
	static unsigned char u8_black_sc_cnt = 0;
	unsigned char u1_det_start=0;

	unsigned char u11_GMV_x_max=0;	// 0: similar, 1:y, 2:x
	unsigned char u10_GMV_y_max=0;	// 0: similar, 1:y, 2:x
	unsigned char u11_GMV_x_sec=0;	// 0: similar, 1:y, 2:x
	unsigned char u10_GMV_y_sec=0;	// 0: similar, 1:y, 2:x
	unsigned char u2_GMV_single_move;	// 0: similar, 1:y, 2:x

//	unsigned short u16_rimUp  = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP];
	//rmv, rdtl
	unsigned int u32_rDTL_th_1 = 70000, u32_rDTL_th_2 = 80000,u32_rPRD_th_1 = 5, u32_rPRD_th_2 = 10;
	unsigned char  u8_rmove_1_cnt= 0, u8_rmove_2_cnt= 0, u8_rmove_3_cnt= 0, u8_rmove_4_cnt = 0,  u8_rmove_cnt_th = 16;
	unsigned char u8_rDTL_RMV_mark[32] = {0},u8_rPRD_RMV_mark[32] = {0}, u8_rmove_rgn1_cnt= 0, u8_rmove_rgn2_cnt= 0, u8_rmove_rgn3_cnt= 0, u8_rmove_rgn4_cnt = 0;
	static char u2_rDTL_RMV_cnt = 0,u2_rPRD_RMV_cnt = 0;

	int i;

	unsigned char u8_black_sc_cnt_th = 60;
	unsigned int u28_tc_th = 0xe0000;//0xb0000;//0x20000;
	unsigned char  u8_bad_region_cnt = 0;

	//Casino
	static unsigned char casino_frm_cnt = 0;
	static unsigned char casino_frm_hold_cnt = 0;
	unsigned char casino_RP_true;
	static unsigned char casino_start = 0;
	unsigned char casino_region_en[32] = {0};
	unsigned char casino_region_idx;
	unsigned char casino_gmv_cond;
	unsigned int casino_dtl_sum = 0, casino_dtl_avg_cond, casino_dtl_var, casino_dtl_var_sum = 0, casino_dtl_var_avg_cond;
	unsigned char casino_region_en_sum_cond = 10;
	unsigned char casino_cond = 0;
	signed short casino_global_GMV_x = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short casino_global_GMV_y = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;

	//Avenger
	unsigned char  avenger_rgn_mark_lft[4] = {0}, avenger_rgn_mark_rht[6] = {0};
	unsigned int avenger_th_lft = 12000, avenger_th_rht = 90000;
	unsigned char avenger_rgn_cnt_col1 = 0, avenger_rgn_cnt_col6 = 0, avenger_rgn_cnt_col7 = 0, avenger_RP_true;
	static unsigned char avenger_frm_cnt = 0;
	static unsigned char avenger_frm_hold_cnt = 0;
	static unsigned char avenger_start = 0;

	//ToL10
	unsigned char ToL10_rgn_mark[32] = {0};
	unsigned int ToL10_th = 90000;
	unsigned char ToL10_rgn_cnt_1 = 0, ToL10_rgn_cnt_2 = 0, ToL10_rgn_cnt_3 = 0, ToL10_rgn_cnt_4 = 0, ToL10_rgn_cnt_5 = 0, ToL10_rgn_cnt_6 = 0, ToL10_rgn_cnt_7 = 0, ToL10_RP_true;
	static unsigned char ToL10_frm_cnt = 0;
	static unsigned char ToL10_frm_hold_cnt = 0;
	static unsigned char ToL10_start = 0;
	//unsigned char ToL10_rgnDtl_diff_t = 1;
	unsigned int ToL10_RgnDtl_diff[6];
	unsigned int ToL10_RgnDtl_diff_mark[8] = {0};
	unsigned int ToL10_RgnDtl_diff_sum = 0;
	unsigned int ToL10_RgnDtl_avg[8] = {0};
	unsigned char ToL10_RgnDtl_avg_cnt = 0, ToL10_RgnDtl_avg_true = 0;

	//======= record pre-
	static unsigned char u6_RP_frm_in_cnt_pre,u6_RP_frm_in_cnt_thl_pre,u8_RP_frm_hold_cnt_pre;
	static unsigned char u1_det_true_pre,u1_det_start_pre,u1_basic_cond_true_pre,u1_mcDtl_move_true_pre,u1_rDTL_move_true_pre,u1_rPRD_move_true_pre,u1_dtl_all_cond_true_pre;
	static unsigned char u2_mcDtl_HV_pre,u2_GMV_single_move_pre;
	static char u2_rDTL_RMV_cnt_pre = 0,u2_rPRD_RMV_cnt_pre = 0;
	static unsigned int u27_me_aTC_rb_pre;
	static unsigned char   u1_maxGMV_x_t_pre,u1_maxGMV_y_t_pre,u1_secGMV_x_t_pre,u1_secGMV_y_t_pre,u1_GMV32_t_pre,u8_bad_region_cnt_pre;
	static unsigned int u19_me_statis_glb_prd_pre;
	static unsigned short u16_mcHdtl_pre,u16_mcVdtl_pre;
	static unsigned char u11_GMV_x_max_pre,u10_GMV_y_max_pre,u11_GMV_x_sec_pre,u10_GMV_y_sec_pre;
	static unsigned char  u8_rmove_1_cnt_pre= 0, u8_rmove_2_cnt_pre= 0, u8_rmove_3_cnt_pre= 0, u8_rmove_4_cnt_pre = 0,u8_rmove_rgn1_cnt_pre= 0, u8_rmove_rgn2_cnt_pre= 0, u8_rmove_rgn3_cnt_pre= 0, u8_rmove_rgn4_cnt_pre = 0;
	static unsigned int u17_me_ZMV_cnt_rb_pre,u25_me_aDTL_rb_ShiftBit_pre;

	//===================

	unsigned int u32_RB_val;
	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);   //  bit 9~15

	// gmv
	{
		u1_maxGMV_x_t = (_ABS_(s11_maxGMV_x) <= u8_GMV_th0)? 1 : 0;
		u1_maxGMV_y_t = (_ABS_(s10_maxGMV_y) <= u8_GMV_th0)? 1 : 0;

		u11_GMV_x_max = _ABS_(s11_maxGMV_x);
		u10_GMV_y_max = _ABS_(s10_maxGMV_y);

		if(s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb>0  && (u1_RP_detect_true_pre == 0) )
		{
			u1_secGMV_x_t = (_ABS_(s11_secGMV_x) <= u8_GMV_th1)? 1 : 0;
			u1_secGMV_y_t = (_ABS_(s10_secGMV_y) <= u8_GMV_th1)? 1 : 0;

			u11_GMV_x_sec = _ABS_(s11_secGMV_x);
			u10_GMV_y_sec = _ABS_(s10_secGMV_y);

			u11_GMV_x_max = _MAX_(u11_GMV_x_max, u11_GMV_x_sec);
			u10_GMV_y_max = _MAX_(u10_GMV_y_max, u10_GMV_y_sec);
		}
		else
		{
			u11_GMV_x_sec = _ABS_(s11_secGMV_x);
			u10_GMV_y_sec = _ABS_(s10_secGMV_y);
			u1_secGMV_x_t = 1;
			u1_secGMV_y_t = 1;
		}

		u1_GMV32_t = 1;

		for (i=0; i<32; i++)
		{
			if((_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i])>u8_GMV_th2)||(_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i])>u8_GMV_th2))
			{
				u8_bad_region_cnt++;
				//u1_GMV32_t = 0;
			}

			// rDtl move th1
			if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[i]>u32_rDTL_th_1 && (s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i] || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i])){
		#if 1
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=12 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=12 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0))
					u8_rmove_1_cnt++;
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-12) && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=12 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0))
					u8_rmove_2_cnt++;
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-12) && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-12) && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0))
					u8_rmove_3_cnt++;
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=12 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-12) && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0))
					u8_rmove_4_cnt++;
		#else
				if(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0)
					u8_rmove_1_cnt++;
				if(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0)
					u8_rmove_2_cnt++;
				if(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0)
					u8_rmove_3_cnt++;
				if(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0)
					u8_rmove_4_cnt++;
		#endif
			}

		#if 1
			// rDtl move th2
			if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[i]>u32_rDTL_th_2 && (s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]!=0 || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]!=0)){
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=12 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-12)) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=12 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-12)))
					u8_rDTL_RMV_mark[i] = 1;
			}
		#endif

		}
		if(u8_bad_region_cnt > 1)
			u1_GMV32_t = 0;

		u1_gmv_true = (u1_maxGMV_x_t && u1_maxGMV_y_t && u1_secGMV_x_t && u1_secGMV_y_t && u1_GMV32_t)? 1 : 0;
	}

	//gmv sigle move
	if(u11_GMV_x_max > u10_GMV_y_max + u8_GMV_diff)
	{
		u2_GMV_single_move = 2;
	}
	else if(u10_GMV_y_max>u11_GMV_x_max+u8_GMV_diff)
	{
		u2_GMV_single_move = 1;
	}
	else
	{
		u2_GMV_single_move = 0;
	}


	// mcDtl

	if(u19_me_statis_glb_prd >= 0x3d4)
		u8_mcDTL_offset = 3;
	else if(u19_me_statis_glb_prd >= 0x384)
		u8_mcDTL_offset = 2;
	else if(u19_me_statis_glb_prd >= 0x320)
		u8_mcDTL_offset = 1;

	if (((u16_mcHdtl > u16_mcDTL_thmin) && (u16_mcHdtl > u16_mcVdtl * (6-u8_mcDTL_offset))) ||\
		((u16_mcHdtl > u16_mcDTL_th0) && (u16_mcHdtl > u16_mcVdtl * (5-u8_mcDTL_offset))) ||\
		((u16_mcHdtl > u16_mcDTL_th1) && (u16_mcHdtl > u16_mcVdtl * (4-u8_mcDTL_offset))) ||\
		((u16_mcHdtl > u16_mcDTL_th2) && (u16_mcHdtl > u16_mcVdtl * (3-u8_mcDTL_offset))) ||\
		((u16_mcHdtl > u16_mcDTL_th3) && (u16_mcHdtl > u16_mcVdtl * (2-u8_mcDTL_offset))))
	{
		u2_mcDtl_HV = 2;	//vertical lines
	}
	else if(((u16_mcVdtl > u16_mcDTL_thmin) && (u16_mcVdtl > u16_mcHdtl * (6-u8_mcDTL_offset)))||\
		((u16_mcVdtl > u16_mcDTL_th0) && (u16_mcVdtl > u16_mcHdtl * (5-u8_mcDTL_offset)))||\
		((u16_mcVdtl > u16_mcDTL_th1) && (u16_mcVdtl > u16_mcHdtl * (4-u8_mcDTL_offset))) ||\
		((u16_mcVdtl > u16_mcDTL_th2) && (u16_mcVdtl > u16_mcHdtl * (3-u8_mcDTL_offset))) ||\
		((u16_mcVdtl > u16_mcDTL_th3) && (u16_mcVdtl > u16_mcHdtl * (2-u8_mcDTL_offset))))

	{
		u2_mcDtl_HV = 1;	//horizantal lines
	}
	else
	{
		u2_mcDtl_HV = 0;
	}

	//mcDtl move
	if(((u2_mcDtl_HV==2)&&(u2_GMV_single_move !=1))||((u2_mcDtl_HV==1)&&(u2_GMV_single_move !=2)))
	{
		u1_mcDtl_move_true = 1;
	}
	else
	{
		u1_mcDtl_move_true = 0;
	}

#if 1
	//rgn rDtl move
	u8_rmove_rgn1_cnt = u8_rDTL_RMV_mark[0] + u8_rDTL_RMV_mark[1] + u8_rDTL_RMV_mark[8] + u8_rDTL_RMV_mark[9] + u8_rDTL_RMV_mark[16] + u8_rDTL_RMV_mark[17] + u8_rDTL_RMV_mark[24] + u8_rDTL_RMV_mark[25];
	u8_rmove_rgn2_cnt = u8_rDTL_RMV_mark[2] + u8_rDTL_RMV_mark[3] + u8_rDTL_RMV_mark[10] + u8_rDTL_RMV_mark[11] + u8_rDTL_RMV_mark[18] + u8_rDTL_RMV_mark[19] + u8_rDTL_RMV_mark[26] + u8_rDTL_RMV_mark[27];
	u8_rmove_rgn3_cnt = u8_rDTL_RMV_mark[4] + u8_rDTL_RMV_mark[5] + u8_rDTL_RMV_mark[12] + u8_rDTL_RMV_mark[13] + u8_rDTL_RMV_mark[20] + u8_rDTL_RMV_mark[21] + u8_rDTL_RMV_mark[28] + u8_rDTL_RMV_mark[29];
	u8_rmove_rgn4_cnt = u8_rDTL_RMV_mark[6] + u8_rDTL_RMV_mark[7] + u8_rDTL_RMV_mark[14] + u8_rDTL_RMV_mark[15] + u8_rDTL_RMV_mark[22] + u8_rDTL_RMV_mark[23] + u8_rDTL_RMV_mark[30] + u8_rDTL_RMV_mark[31];

	if((u8_rmove_1_cnt+u8_rmove_2_cnt)>u8_rmove_cnt_th || (u8_rmove_2_cnt+u8_rmove_3_cnt)>u8_rmove_cnt_th || (u8_rmove_3_cnt+u8_rmove_4_cnt)>u8_rmove_cnt_th || (u8_rmove_4_cnt+u8_rmove_1_cnt)>u8_rmove_cnt_th){
		u2_rDTL_RMV_cnt++;
	}
	else if(u8_rmove_rgn1_cnt>1 || u8_rmove_rgn2_cnt>1 || u8_rmove_rgn3_cnt>1 || u8_rmove_rgn4_cnt>1){
		u2_rDTL_RMV_cnt++;
	}
	else{
		u2_rDTL_RMV_cnt--;
	}

	if(u2_rDTL_RMV_cnt>5) u2_rDTL_RMV_cnt = 5;
	if(u2_rDTL_RMV_cnt<0) u2_rDTL_RMV_cnt = 0;

	if(u2_rDTL_RMV_cnt>0){
		u1_rDTL_move_true = 1;
		//u1_gmv_true = 1; // ignore gmv
		u8_black_sc_cnt = u8_black_sc_cnt_th; //quick start
	}
	else{
		u1_rDTL_move_true = 0;
	}
#endif


#if 1    //rgn rPRD move
	u8_rmove_1_cnt= 0; u8_rmove_2_cnt= 0; u8_rmove_3_cnt= 0; u8_rmove_4_cnt = 0;
	for (i=0; i<32; i++)
	{
		// rDtl move th1
		if(s_pContext->_output_read_comreg.u13_me_rPRD_rb[i]>u32_rPRD_th_1 && (s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i] || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i])){
			if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=12 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=12 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0))
				u8_rmove_1_cnt++;
			if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-12) && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=12 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0))
				u8_rmove_2_cnt++;
			if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-12) && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-12) && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0))
				u8_rmove_3_cnt++;
			if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=12 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-12) && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0))
				u8_rmove_4_cnt++;
		}
	#if 1
		// rPRD move th2
		if(s_pContext->_output_read_comreg.u13_me_rPRD_rb[i]>u32_rPRD_th_2 && (s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]!=0 || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]!=0)){
			if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=12 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-12)) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=12 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-12)))
				u8_rPRD_RMV_mark[i] = 1;
		}
	#endif

	}

	//rgn rPRD move
	u8_rmove_rgn1_cnt = u8_rPRD_RMV_mark[0] + u8_rPRD_RMV_mark[1] + u8_rPRD_RMV_mark[8] + u8_rPRD_RMV_mark[9] + u8_rPRD_RMV_mark[16] + u8_rPRD_RMV_mark[17] + u8_rPRD_RMV_mark[24] + u8_rPRD_RMV_mark[25];
	u8_rmove_rgn2_cnt = u8_rPRD_RMV_mark[2] + u8_rPRD_RMV_mark[3] + u8_rPRD_RMV_mark[10] + u8_rPRD_RMV_mark[11] + u8_rPRD_RMV_mark[18] + u8_rPRD_RMV_mark[19] + u8_rPRD_RMV_mark[26] + u8_rPRD_RMV_mark[27];
	u8_rmove_rgn3_cnt = u8_rPRD_RMV_mark[4] + u8_rPRD_RMV_mark[5] + u8_rPRD_RMV_mark[12] + u8_rPRD_RMV_mark[13] + u8_rPRD_RMV_mark[20] + u8_rPRD_RMV_mark[21] + u8_rPRD_RMV_mark[28] + u8_rPRD_RMV_mark[29];
	u8_rmove_rgn4_cnt = u8_rPRD_RMV_mark[6] + u8_rPRD_RMV_mark[7] + u8_rPRD_RMV_mark[14] + u8_rPRD_RMV_mark[15] + u8_rPRD_RMV_mark[22] + u8_rPRD_RMV_mark[23] + u8_rPRD_RMV_mark[30] + u8_rPRD_RMV_mark[31];

	if((u8_rmove_1_cnt+u8_rmove_2_cnt)>u8_rmove_cnt_th || (u8_rmove_2_cnt+u8_rmove_3_cnt)>u8_rmove_cnt_th || (u8_rmove_3_cnt+u8_rmove_4_cnt)>u8_rmove_cnt_th || (u8_rmove_4_cnt+u8_rmove_1_cnt)>u8_rmove_cnt_th){
		u2_rPRD_RMV_cnt++;
	}
	else if(u8_rmove_rgn1_cnt>1 || u8_rmove_rgn2_cnt>1 || u8_rmove_rgn3_cnt>1 || u8_rmove_rgn4_cnt>1){
		u2_rPRD_RMV_cnt++;
	}
	else{
		u2_rPRD_RMV_cnt--;
	}

	if(u2_rPRD_RMV_cnt>5) u2_rPRD_RMV_cnt = 5;
	if(u2_rPRD_RMV_cnt<0) u2_rPRD_RMV_cnt = 0;

	if(u2_rPRD_RMV_cnt>0){
		u1_rPRD_move_true = 1;
		//u8_black_sc_cnt = u8_black_sc_cnt_th; //quick start
	}
	else{
		u1_rPRD_move_true = 0;
	}
#endif

	//black pattern&sc but not static
	u1_black_sc = ((u16_mcHdtl==0)&&(u16_mcVdtl==0))||((s_pContext->_output_read_comreg.u1_sc_status_rb==1)&&(s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb<0x6000));

	if(u1_black_sc)	// for after black pattern, img dtl not stable
	{
		u8_black_sc_cnt = 0;
	}
	else
	{
		u8_black_sc_cnt += FRAME_CHANGE_CNT;

		if(u8_black_sc_cnt>60)
		{
			u8_black_sc_cnt = 60;
		}
	}

	if(u8_black_sc_cnt>=60)
	{
		u1_det_start = 1;
	}
	else
	{
		u1_det_start = 0;
	}

	u1_tc_true = s_pContext->_output_read_comreg.u27_me_aTC_rb< u28_tc_th;//0x6000;

	//dtl all

//	if(u16_mcVdtl >u16_mcHdtl && u16_mcVdtl >=0x130 && u16_mcHdtl >=  0xd0 )
//		u8_me_aDTL_ShiftBit =12;
//	else
		u8_me_aDTL_ShiftBit =12;

	u1_dtl_all_cond_true = (u16_mcHdtl>(s_pContext->_output_read_comreg.u25_me_aDTL_rb>>u8_me_aDTL_ShiftBit))||(u16_mcVdtl>(s_pContext->_output_read_comreg.u25_me_aDTL_rb>>u8_me_aDTL_ShiftBit) || (u19_me_statis_glb_prd > 0x100));

	//rim
	//u1_rim_true = (u16_rimUp<0x80);

	// result

	u1_basic_cond_true = u1_gmv_true&&u1_tc_true;//&&u1_rim_true;

	u1_det_true = (u1_det_start&&u1_basic_cond_true && (u1_mcDtl_move_true || u1_rDTL_move_true || u1_rPRD_move_true) &&u1_dtl_all_cond_true) ? 1 : 0;

	if(u1_det_true)
	{
		if( u19_me_statis_glb_prd > 0x200)
			u6_RP_frm_in_cnt =u6_RP_frm_in_cnt+2;
		else
		u6_RP_frm_in_cnt++;

		u6_RP_frm_in_cnt = (u6_RP_frm_in_cnt>8)? 8:u6_RP_frm_in_cnt;
	}
	else
	{
		u6_RP_frm_in_cnt = (u6_RP_frm_in_cnt == 0)? 1:u6_RP_frm_in_cnt;
		u6_RP_frm_in_cnt --;
	}
	if( u19_me_statis_glb_prd > 0x300)
	{
		u6_RP_frm_in_cnt_thl =u6_RP_frm_in_cnt_thl-2;
		u6_RP_frm_in_cnt_thl = (u6_RP_frm_in_cnt_thl<2)? 2:u6_RP_frm_in_cnt_thl;
	}
	else if( u19_me_statis_glb_prd > 0x200)
	{
		u6_RP_frm_in_cnt_thl --;
		u6_RP_frm_in_cnt_thl = (u6_RP_frm_in_cnt_thl<3)? 3:u6_RP_frm_in_cnt_thl;
	}
       else if( u19_me_statis_glb_prd > 0x100)
	{
		u6_RP_frm_in_cnt_thl --;
		u6_RP_frm_in_cnt_thl = (u6_RP_frm_in_cnt_thl<4)? 4:u6_RP_frm_in_cnt_thl;
	}
	else
	{
		u6_RP_frm_in_cnt_thl ++ ;
		u6_RP_frm_in_cnt_thl = (u6_RP_frm_in_cnt_thl>8)? 8:u6_RP_frm_in_cnt_thl;
	}

	if((u1_det_start == 0)||(u1_basic_cond_true==0))		//black or sc, basic condition not match, quit
	{
		u8_RP_frm_hold_cnt = 0;
		u8_RP_frm_hold_cnt_max = 0;
	}
	else if( u19_me_statis_glb_prd >= 0x4e0)
	{
		if(u8_RP_frm_hold_cnt>0)
			u8_RP_frm_hold_cnt =  u16_RP_frm_hold_thl*5;

		u8_RP_frm_hold_cnt_max = u8_RP_frm_hold_cnt;
	}
	else if(u6_RP_frm_in_cnt>=u6_RP_frm_in_cnt_thl)
	{
		u8_RP_frm_hold_cnt += FRAME_CHANGE_CNT;

		u8_RP_frm_hold_cnt = (u8_RP_frm_hold_cnt>u16_RP_frm_hold_thl *3)? u16_RP_frm_hold_thl*3:u8_RP_frm_hold_cnt;			//video end not detect protect

		u8_RP_frm_hold_cnt_max = u8_RP_frm_hold_cnt;
	}
	else if((u8_RP_frm_hold_cnt_max>=u16_RP_frm_hold_thl*2)&&(u8_RP_frm_hold_cnt>FRAME_CHANGE_CNT)\
		&&(pOutput->SIS_MEMC.MEMC_STATUS.u25_rdbk_dtl_all_intervel5_pre>pOutput->SIS_MEMC.MEMC_STATUS.u25_rdbk_dtl_all_intervel5))	//hold only when repeat pattern move dispeared
	{
		u8_RP_frm_hold_cnt -= FRAME_CHANGE_CNT;
	}
	else if((s_pContext->_output_read_comreg.u1_sc_status_rb==1) && (u8_RP_frm_hold_cnt_max>=u16_RP_frm_hold_thl*2)&&(u8_RP_frm_hold_cnt>FRAME_CHANGE_CNT*2)&&u1_basic_cond_true)
	{
		u8_RP_frm_hold_cnt -= FRAME_CHANGE_CNT*2;
	}
	else if((u8_RP_frm_hold_cnt_max>=u16_RP_frm_hold_thl*2)&&(u8_RP_frm_hold_cnt>FRAME_CHANGE_CNT*1)&&u1_basic_cond_true)
	{
		u8_RP_frm_hold_cnt -= FRAME_CHANGE_CNT*1;
	}
	else
	{
		u8_RP_frm_hold_cnt = 0;
		u8_RP_frm_hold_cnt_max = 0;
		u6_RP_frm_in_cnt_thl =8;
	}
	if(u8_RP_frm_hold_cnt>0 ||  (u1_det_true && u6_RP_frm_in_cnt ==8))
	{
		pOutput->u1_RP_detect_true = 1;
		u6_RP_out_print = 1;
	}
	else
	{
		pOutput->u1_RP_detect_true = 0;
		if(u6_RP_out_print == 1)
			u6_RP_out_print =2;
	}


	if(((u32_RB_val >>  9) & 0x01) ==1)
		pOutput->u1_RP_detect_true = 1;
	if(((u32_RB_val >>  10) & 0x01) ==1)
		pOutput->u1_RP_detect_true = 0;


	// Casino_patch : write by fanny @20180619
	casino_gmv_cond = (_ABS_(casino_global_GMV_x) + _ABS_(casino_global_GMV_y) < 12) ;

	casino_region_en[8] = 1;
	casino_region_en[9] = 1;
	casino_region_en[10] = 1;
	casino_region_en[14] = 1;
	casino_region_en[15] = 1;
	casino_region_en[16] = 1;
	casino_region_en[17] = 1;
	casino_region_en[18] = 1;
       casino_region_en[22] = 1;
	casino_region_en[23] = 1;

	for(casino_region_idx = 0; casino_region_idx < 32; casino_region_idx++)
	{
		if(casino_region_en[casino_region_idx])
		{
			casino_dtl_sum += s_pContext->_output_read_comreg.u20_me_rDTL_rb[casino_region_idx];
		}
	}
	casino_dtl_avg_cond = casino_dtl_sum/casino_region_en_sum_cond;
	for(casino_region_idx = 0;casino_region_idx < 32;casino_region_idx++)
		{
		if(casino_region_en[casino_region_idx])
			{
			casino_dtl_var = _ABS_(casino_dtl_avg_cond - s_pContext->_output_read_comreg.u20_me_rDTL_rb[casino_region_idx]);
		       casino_dtl_var_sum += casino_dtl_var;
		}
	}
	casino_dtl_var_avg_cond =  casino_dtl_var_sum/casino_region_en_sum_cond;

	casino_cond = (casino_dtl_avg_cond > 34000 && casino_dtl_var_avg_cond < 12000 && casino_gmv_cond);
	//rtd_pr_memc_info("fanny_test : %d,%d,%d,%d,(%d,%d),\n",casino_dtl_avg_cond,casino_dtl_var_avg_cond,(abs(casino_global_GMV_x) + abs(casino_global_GMV_y)),casino_cond,casino_global_GMV_x,casino_global_GMV_y);
	if(casino_cond)
		casino_frm_cnt++;
	else
		casino_frm_cnt = 0;

	if(casino_frm_cnt >= 5)
		casino_frm_cnt = 5;
	if(casino_frm_cnt <= 0)
		casino_frm_cnt = 0;

	if(casino_frm_cnt >= 5)
		casino_start = 1;
	else
		casino_start = 0;

	if((casino_start == 1) && (casino_frm_hold_cnt == 0)){
		casino_frm_hold_cnt++;
		casino_RP_true = 1;
	}
	else if((casino_frm_hold_cnt > 0) && (casino_frm_hold_cnt <= 48)){
		casino_frm_hold_cnt++;
		casino_RP_true = 1;
	}
	else{
		casino_frm_hold_cnt = 0;
		casino_RP_true = 0;
		casino_start = 0;
	}

	if(casino_start == 0)
		casino_RP_true = 0;

	pOutput->u1_casino_RP_detect_true = casino_cond;

	if(((u32_RB_val >>  11) & 0x01) ==1)
		pOutput->u1_casino_RP_detect_true = 1;
	if(((u32_RB_val >>  12) & 0x01) ==1)
		pOutput->u1_casino_RP_detect_true = 0;

	// Avenger_patch : write by Jerry Wang @ 20161018
	//	6	7
	//	14	15
	//	22	23
	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[6] >= avenger_th_rht)
		avenger_rgn_mark_rht[0] = 1;
	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[7] >= avenger_th_rht)
		avenger_rgn_mark_rht[1] = 1;
	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[14] >= avenger_th_rht)
		avenger_rgn_mark_rht[2] = 1;
	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[15] >= avenger_th_rht)
		avenger_rgn_mark_rht[3] = 1;
	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[22] >= avenger_th_rht)
		avenger_rgn_mark_rht[4] = 1;
	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[23] >= avenger_th_rht)
		avenger_rgn_mark_rht[5] = 1;

	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[1] <= avenger_th_lft)
		avenger_rgn_mark_lft[0] = 1;
	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[9] <= avenger_th_lft)
		avenger_rgn_mark_lft[1] = 1;
	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[17] <= avenger_th_lft)
		avenger_rgn_mark_lft[2] = 1;
	if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[25] <= avenger_th_lft)
		avenger_rgn_mark_lft[3] = 1;


	// col1		col6 col7
	//	0		0	1
	//	1		2	3
	//	2		4	5
	//	3

	// 20161202 robin: for prevent sonycam to fall into this patch (Q-10035)
	avenger_rgn_cnt_col1 = avenger_rgn_mark_lft[0] + avenger_rgn_mark_lft[1] + avenger_rgn_mark_lft[2] + avenger_rgn_mark_lft[3];

	avenger_rgn_cnt_col6 = avenger_rgn_mark_rht[0] + avenger_rgn_mark_rht[2] + avenger_rgn_mark_rht[4];
	avenger_rgn_cnt_col7 = avenger_rgn_mark_rht[1] + avenger_rgn_mark_rht[3] + avenger_rgn_mark_rht[5];

	if((avenger_rgn_cnt_col1 >= 2) && ((avenger_rgn_cnt_col6 <= 2 && avenger_rgn_cnt_col7 >= 2) || (avenger_rgn_cnt_col6 >= 2 && avenger_rgn_cnt_col7 <= 2))){
		avenger_frm_cnt++;
	}
	else{
		avenger_frm_cnt = 0;
	}

	if(avenger_frm_cnt >= 5)
		avenger_frm_cnt = 5;
	if(avenger_frm_cnt <= 0)
		avenger_frm_cnt = 0;

	if(avenger_frm_cnt >= 5)
		avenger_start = 1;
	else
		avenger_start = 0;

	if((avenger_start == 1) && (avenger_frm_hold_cnt == 0)){
		avenger_frm_hold_cnt++;
		avenger_RP_true = 1;
	}
	else if((avenger_frm_hold_cnt > 0) && (avenger_frm_hold_cnt <= 48)){
		avenger_frm_hold_cnt++;
		avenger_RP_true = 1;
	}
	else{
		avenger_frm_hold_cnt = 0;
		avenger_RP_true = 0;
		avenger_start = 0;
	}


	// ToL_10_patch : write by Jerry Wang @ 20161108
	for (i=0; i<32; i++){
		if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[i] > ToL10_th){
			ToL10_rgn_mark[i] = 1;
		}
		else{
			ToL10_rgn_mark[i] = 0;
		}
	}

	if((ToL10_rgn_mark[0]+ToL10_rgn_mark[1]+ToL10_rgn_mark[8]+ToL10_rgn_mark[9]+ToL10_rgn_mark[16]+ToL10_rgn_mark[17]+ToL10_rgn_mark[24]+ToL10_rgn_mark[25]) >= 8){
		ToL10_rgn_cnt_1 = 1;
	}
	else{
		ToL10_rgn_cnt_1 = 0;
	}
	if((ToL10_rgn_mark[1]+ToL10_rgn_mark[2]+ToL10_rgn_mark[9]+ToL10_rgn_mark[10]+ToL10_rgn_mark[17]+ToL10_rgn_mark[18]+ToL10_rgn_mark[25]+ToL10_rgn_mark[26]) >= 8){
		ToL10_rgn_cnt_2 = 1;
	}
	else{
		ToL10_rgn_cnt_2 = 0;
	}
	if((ToL10_rgn_mark[2]+ToL10_rgn_mark[3]+ToL10_rgn_mark[10]+ToL10_rgn_mark[11]+ToL10_rgn_mark[18]+ToL10_rgn_mark[19]+ToL10_rgn_mark[26]+ToL10_rgn_mark[27]) >= 8){
		ToL10_rgn_cnt_3 = 1;
	}
	else{
		ToL10_rgn_cnt_3 = 0;
	}
	if((ToL10_rgn_mark[3]+ToL10_rgn_mark[4]+ToL10_rgn_mark[11]+ToL10_rgn_mark[12]+ToL10_rgn_mark[19]+ToL10_rgn_mark[20]+ToL10_rgn_mark[27]+ToL10_rgn_mark[28]) >= 8){
		ToL10_rgn_cnt_4 = 1;
	}
	else{
		ToL10_rgn_cnt_4 = 0;
	}
	if((ToL10_rgn_mark[4]+ToL10_rgn_mark[5]+ToL10_rgn_mark[12]+ToL10_rgn_mark[13]+ToL10_rgn_mark[20]+ToL10_rgn_mark[21]+ToL10_rgn_mark[28]+ToL10_rgn_mark[29]) >= 8){
		ToL10_rgn_cnt_5 = 1;
	}
	else{
		ToL10_rgn_cnt_5 = 0;
	}
	if((ToL10_rgn_mark[5]+ToL10_rgn_mark[6]+ToL10_rgn_mark[13]+ToL10_rgn_mark[14]+ToL10_rgn_mark[21]+ToL10_rgn_mark[22]+ToL10_rgn_mark[29]+ToL10_rgn_mark[30]) >= 8){
		ToL10_rgn_cnt_6 = 1;
	}
	else{
		ToL10_rgn_cnt_6 = 0;
	}
	if((ToL10_rgn_mark[6]+ToL10_rgn_mark[7]+ToL10_rgn_mark[14]+ToL10_rgn_mark[15]+ToL10_rgn_mark[22]+ToL10_rgn_mark[23]+ToL10_rgn_mark[30]+ToL10_rgn_mark[31]) >= 8){
		ToL10_rgn_cnt_7 = 1;
	}
	else{
		ToL10_rgn_cnt_7 = 0;
	}

	// me_rgnDtl_diff
	for (i = 0; i < 8; i ++){
		ToL10_RgnDtl_diff[0] = _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[0+i], s_pContext->_output_read_comreg.u20_me_rDTL_rb[8+i]);
		ToL10_RgnDtl_diff[1] = _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[8+i], s_pContext->_output_read_comreg.u20_me_rDTL_rb[16+i]);
		ToL10_RgnDtl_diff[2] = _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[16+i], s_pContext->_output_read_comreg.u20_me_rDTL_rb[24+i]);
		ToL10_RgnDtl_diff[3] = _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[8+i], s_pContext->_output_read_comreg.u20_me_rDTL_rb[24+i]);
		ToL10_RgnDtl_diff[4] = _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[0+i], s_pContext->_output_read_comreg.u20_me_rDTL_rb[16+i]);
		ToL10_RgnDtl_diff[5] = _ABS_DIFF_(s_pContext->_output_read_comreg.u20_me_rDTL_rb[0+i], s_pContext->_output_read_comreg.u20_me_rDTL_rb[24+i]);

		if((s_pContext->_output_read_comreg.u20_me_rDTL_rb[0+i] > ToL10_th) && (s_pContext->_output_read_comreg.u20_me_rDTL_rb[8+i] > ToL10_th)
			&& (s_pContext->_output_read_comreg.u20_me_rDTL_rb[16+i] > ToL10_th) && (s_pContext->_output_read_comreg.u20_me_rDTL_rb[24+i] > ToL10_th)
			&& (ToL10_RgnDtl_diff[0] < 9000) && (ToL10_RgnDtl_diff[1] < 9000) && (ToL10_RgnDtl_diff[2] < 9000)
			&& (ToL10_RgnDtl_diff[3] < 9000) && (ToL10_RgnDtl_diff[4] < 9000) && (ToL10_RgnDtl_diff[5] < 9000))
		{
			ToL10_RgnDtl_diff_mark[i] = 1;
		}
		ToL10_RgnDtl_diff_sum = ToL10_RgnDtl_diff_sum + ToL10_RgnDtl_diff_mark[i];

		// 20161202 robin: for prevent sonycam to fall into this patch (Q-10035)
		ToL10_RgnDtl_avg[i] = (s_pContext->_output_read_comreg.u20_me_rDTL_rb[0+i] + s_pContext->_output_read_comreg.u20_me_rDTL_rb[8+i]
							+ s_pContext->_output_read_comreg.u20_me_rDTL_rb[16+i] + s_pContext->_output_read_comreg.u20_me_rDTL_rb[24+i]) >> 2;
	}
#if 1

	// 20161202 robin: for prevent sonycam to fall into this patch (Q-10035)
	for (i = 0; i < 8; i ++){
		ToL10_RgnDtl_avg_cnt += (ToL10_RgnDtl_avg[i] > 60000 && ToL10_RgnDtl_avg[i] < 120000) ? 1 : 0;
	}

	ToL10_RgnDtl_avg_true = (ToL10_RgnDtl_avg_cnt > 5) ? 0 : 1;

	if(ToL10_RgnDtl_avg_true && (ToL10_rgn_cnt_1 || ToL10_rgn_cnt_2 || ToL10_rgn_cnt_3 || ToL10_rgn_cnt_4 || ToL10_rgn_cnt_5 || ToL10_rgn_cnt_6 || ToL10_rgn_cnt_7 || ((ToL10_RgnDtl_diff_sum > 0) && (ToL10_RgnDtl_diff_sum < 3)))){
		ToL10_frm_cnt++;
	}
	else{
		ToL10_frm_cnt = 0;
	}

	if(ToL10_frm_cnt >= 10){
		ToL10_frm_cnt = 10;
	}
	if(ToL10_frm_cnt <= 0){
		ToL10_frm_cnt = 0;
	}

	if((ToL10_frm_cnt >= 10) ){
		ToL10_start = 1;
	}
	else{
		ToL10_start = 0;
	}

	if((ToL10_start == 1) && (ToL10_frm_hold_cnt == 0)){
		ToL10_frm_hold_cnt++;
		ToL10_RP_true = 1;
	}
	else if((ToL10_frm_hold_cnt > 0) && (ToL10_frm_hold_cnt <= 60)){
		ToL10_frm_hold_cnt++;
		ToL10_RP_true = 1;
	}
	else{
		ToL10_frm_hold_cnt = 0;
		ToL10_RP_true = 0;
		ToL10_start = 0;
	}

//	if(ToL10_start == 0)
//		ToL10_RP_true = 0;
#else
	if(ToL10_rgn_cnt_1 || ToL10_rgn_cnt_2 || ToL10_rgn_cnt_3 || ToL10_rgn_cnt_4 || ToL10_rgn_cnt_5 || ToL10_rgn_cnt_6 || ToL10_rgn_cnt_7 || ((ToL10_RgnDtl_diff_sum > 0) && (ToL10_RgnDtl_diff_sum < 3))){
		ToL10_RP_true = 1;
	}
	else{
		ToL10_RP_true = 0;
	}
#endif

	pOutput->u1_ToL10_RP_true = ToL10_RP_true;


	//rtd_pr_memc_info("detect=%d, casino=%d, avenger=%d, ToL10=%d\n", pOutput->u1_RP_detect_true, pOutput->u1_casino_RP_detect_true, avenger_RP_true, ToL10_RP_true);

	if(pOutput->u1_RP_detect_true == 0 && avenger_RP_true == 1){
		pOutput->u1_RP_detect_true = 1;
	}
	else	if(pOutput->u1_RP_detect_true == 0 && ToL10_RP_true == 1){
		pOutput->u1_RP_detect_true = 1;
	}
	pOutput->u1_Avenger_RP_detect_true = pParam->u1_RepeatPattern_en && avenger_RP_true;
	pOutput->u1_RP_detect_true = pParam->u1_RepeatPattern_en && pOutput->u1_RP_detect_true;



		#if 1
	if ( (frm_cnt % 10 == 0  && ((u32_RB_val >>  13) & 0x01) ==1 ) || (frm_cnt % 1 == 0  && ((u32_RB_val >>  14) & 0x01) ==1  && (u6_RP_out_print ==2))){
		u6_RP_out_print =0;
		rtd_pr_memc_emerg("[%d  %d  %d ] [%d %d %d (%d %d %d) %d] [%d %d %d %d %d] [%d %d %d %d %d %d][%d %d %d %d %d %d %d, %d %d %d %d, %d %d %d %d][%d  %d]\n",
			u6_RP_frm_in_cnt,u6_RP_frm_in_cnt_thl,u8_RP_frm_hold_cnt,
			u1_det_true, u1_det_start, u1_basic_cond_true, u1_mcDtl_move_true, u1_rDTL_move_true,u1_rPRD_move_true, u1_dtl_all_cond_true,
			u2_mcDtl_HV, u2_GMV_single_move, u2_rDTL_RMV_cnt, u2_rPRD_RMV_cnt,s_pContext->_output_read_comreg.u27_me_aTC_rb,
			u1_maxGMV_x_t, u1_maxGMV_y_t, u1_secGMV_x_t, u1_secGMV_y_t,u1_GMV32_t, u8_bad_region_cnt,
			u19_me_statis_glb_prd,u16_mcHdtl ,u16_mcVdtl, u11_GMV_x_max, u10_GMV_y_max, u11_GMV_x_sec,u10_GMV_y_sec,u8_rmove_1_cnt, u8_rmove_2_cnt, u8_rmove_3_cnt, u8_rmove_4_cnt, u8_rmove_rgn1_cnt, u8_rmove_rgn2_cnt, u8_rmove_rgn3_cnt, u8_rmove_rgn4_cnt,
			s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb,(s_pContext->_output_read_comreg.u25_me_aDTL_rb>>u8_me_aDTL_ShiftBit));
			//s_pContext->_output_read_comreg.u22_me_rTC_rb[31], s_pContext->_output_read_comreg.u22_me_rSC_rb[31], s_pContext->_output_read_comreg.u25_me_rSAD_rb[31]);
		#if 1
		rtd_pr_memc_emerg("\n\r$$$   [(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d)]   $$$\n\r$$$   [(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d)]   $$$\n\r$$$   [(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d)]   $$$\n\r$$$   [(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d),\t(%d %d %d %d)]   $$$\n\r",
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[0], s_pContext->_output_read_comreg.u20_me_rDTL_rb[0],s_pContext->_output_read_comreg.u13_me_rPRD_rb[0],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[1], s_pContext->_output_read_comreg.u20_me_rDTL_rb[1],s_pContext->_output_read_comreg.u13_me_rPRD_rb[1],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[2], s_pContext->_output_read_comreg.u20_me_rDTL_rb[2],s_pContext->_output_read_comreg.u13_me_rPRD_rb[2],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[3], s_pContext->_output_read_comreg.u20_me_rDTL_rb[3],s_pContext->_output_read_comreg.u13_me_rPRD_rb[3],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[4], s_pContext->_output_read_comreg.u20_me_rDTL_rb[4],s_pContext->_output_read_comreg.u13_me_rPRD_rb[4],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[5], s_pContext->_output_read_comreg.u20_me_rDTL_rb[5],s_pContext->_output_read_comreg.u13_me_rPRD_rb[5],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[6], s_pContext->_output_read_comreg.u20_me_rDTL_rb[6],s_pContext->_output_read_comreg.u13_me_rPRD_rb[6],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[7], s_pContext->_output_read_comreg.u20_me_rDTL_rb[7],s_pContext->_output_read_comreg.u13_me_rPRD_rb[7],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[8], s_pContext->_output_read_comreg.u20_me_rDTL_rb[8],s_pContext->_output_read_comreg.u13_me_rPRD_rb[8],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9], s_pContext->_output_read_comreg.u20_me_rDTL_rb[9],s_pContext->_output_read_comreg.u13_me_rPRD_rb[9],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[10], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[10], s_pContext->_output_read_comreg.u20_me_rDTL_rb[10],s_pContext->_output_read_comreg.u13_me_rPRD_rb[10],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11], s_pContext->_output_read_comreg.u20_me_rDTL_rb[11],s_pContext->_output_read_comreg.u13_me_rPRD_rb[11],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12], s_pContext->_output_read_comreg.u20_me_rDTL_rb[12],s_pContext->_output_read_comreg.u13_me_rPRD_rb[12],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[13], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[13], s_pContext->_output_read_comreg.u20_me_rDTL_rb[13],s_pContext->_output_read_comreg.u13_me_rPRD_rb[13],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14], s_pContext->_output_read_comreg.u20_me_rDTL_rb[14],s_pContext->_output_read_comreg.u13_me_rPRD_rb[14],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[15], s_pContext->_output_read_comreg.u20_me_rDTL_rb[15],s_pContext->_output_read_comreg.u13_me_rPRD_rb[15],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16], s_pContext->_output_read_comreg.u20_me_rDTL_rb[16],s_pContext->_output_read_comreg.u13_me_rPRD_rb[16],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[17], s_pContext->_output_read_comreg.u20_me_rDTL_rb[17],s_pContext->_output_read_comreg.u13_me_rPRD_rb[17],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[18], s_pContext->_output_read_comreg.u20_me_rDTL_rb[18],s_pContext->_output_read_comreg.u13_me_rPRD_rb[18],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19], s_pContext->_output_read_comreg.u20_me_rDTL_rb[19],s_pContext->_output_read_comreg.u13_me_rPRD_rb[19],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[20], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[20], s_pContext->_output_read_comreg.u20_me_rDTL_rb[20],s_pContext->_output_read_comreg.u13_me_rPRD_rb[20],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[21], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[21], s_pContext->_output_read_comreg.u20_me_rDTL_rb[21],s_pContext->_output_read_comreg.u13_me_rPRD_rb[21],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[22], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[22], s_pContext->_output_read_comreg.u20_me_rDTL_rb[22],s_pContext->_output_read_comreg.u13_me_rPRD_rb[22],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[23], s_pContext->_output_read_comreg.u20_me_rDTL_rb[23],s_pContext->_output_read_comreg.u13_me_rPRD_rb[23],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[24], s_pContext->_output_read_comreg.u20_me_rDTL_rb[24],s_pContext->_output_read_comreg.u13_me_rPRD_rb[24],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[25], s_pContext->_output_read_comreg.u20_me_rDTL_rb[25],s_pContext->_output_read_comreg.u13_me_rPRD_rb[25],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[26], s_pContext->_output_read_comreg.u20_me_rDTL_rb[26],s_pContext->_output_read_comreg.u13_me_rPRD_rb[26],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[27], s_pContext->_output_read_comreg.u20_me_rDTL_rb[27],s_pContext->_output_read_comreg.u13_me_rPRD_rb[27],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[28], s_pContext->_output_read_comreg.u20_me_rDTL_rb[28],s_pContext->_output_read_comreg.u13_me_rPRD_rb[28],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[29], s_pContext->_output_read_comreg.u20_me_rDTL_rb[29],s_pContext->_output_read_comreg.u13_me_rPRD_rb[29],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[30], s_pContext->_output_read_comreg.u20_me_rDTL_rb[30],s_pContext->_output_read_comreg.u13_me_rPRD_rb[30],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[31], s_pContext->_output_read_comreg.u20_me_rDTL_rb[31],s_pContext->_output_read_comreg.u13_me_rPRD_rb[31]);
		#endif
	}
	else if( ((u32_RB_val >>  15) & 0x01) ==1 && u1_RP_detect_true_pre == 0 &&pOutput->u1_RP_detect_true ==1)
	{
		u6_RP_out_print =0;
		rtd_pr_memc_emerg("[%d  %d  %d ] [%d %d %d (%d %d %d) %d] [%d %d %d %d %d] [%d %d %d %d %d %d][%d %d %d %d %d %d %d, %d %d %d %d, %d %d %d %d][%d  %d]\n",
			u6_RP_frm_in_cnt_pre,u6_RP_frm_in_cnt_thl_pre,u8_RP_frm_hold_cnt_pre,
			u1_det_true_pre, u1_det_start_pre, u1_basic_cond_true_pre, u1_mcDtl_move_true_pre, u1_rDTL_move_true_pre,u1_rPRD_move_true_pre, u1_dtl_all_cond_true_pre,
			u2_mcDtl_HV_pre, u2_GMV_single_move_pre, u2_rDTL_RMV_cnt_pre, u2_rPRD_RMV_cnt_pre,u27_me_aTC_rb_pre,
			u1_maxGMV_x_t_pre, u1_maxGMV_y_t_pre, u1_secGMV_x_t_pre, u1_secGMV_y_t_pre,u1_GMV32_t_pre, u8_bad_region_cnt_pre,
			u19_me_statis_glb_prd_pre,u16_mcHdtl_pre,u16_mcVdtl_pre, u11_GMV_x_max_pre, u10_GMV_y_max_pre, u11_GMV_x_sec_pre,u10_GMV_y_sec_pre,u8_rmove_1_cnt_pre, u8_rmove_2_cnt_pre, u8_rmove_3_cnt_pre, u8_rmove_4_cnt_pre, u8_rmove_rgn1_cnt_pre, u8_rmove_rgn2_cnt_pre, u8_rmove_rgn3_cnt_pre, u8_rmove_rgn4_cnt_pre,
			u17_me_ZMV_cnt_rb_pre,u25_me_aDTL_rb_ShiftBit_pre);
	}
	#if 0
	// casino
 	if (frm_cnt % 10 == 0){
		rtd_pr_memc_emerg("$$$   [(%d),\t(%d),\t(%d),\t(%d)]   $$$\n\r", casino_RP_true, casino_rim_t, casino_rgnDtl_diff_t, casino_rgn_cnt);
	}
	// ToL_10
 	if (frm_cnt % 10 == 0){
		rtd_pr_memc_emerg("$$$   [(%d),\t(%d),\t(%d)]   $$$\n\r$$$   [(%d),\t(%d),\t(%d),\t(%d),\t(%d),\t(%d),\t(%d)]   $$$\n\r$$$   [(%d),\t(%d),\t(%d),\t(%d),\t(%d),\t(%d),\t(%d),\t(%d)]   $$$\n\r",
			ToL10_RP_true, ToL10_frm_hold_cnt, ToL10_RgnDtl_diff_sum,
			ToL10_rgn_cnt_1, ToL10_rgn_cnt_2, ToL10_rgn_cnt_3, ToL10_rgn_cnt_4, ToL10_rgn_cnt_5, ToL10_rgn_cnt_6, ToL10_rgn_cnt_7,
			ToL10_RgnDtl_diff_mark[0], ToL10_RgnDtl_diff_mark[1], ToL10_RgnDtl_diff_mark[2], ToL10_RgnDtl_diff_mark[3], ToL10_RgnDtl_diff_mark[4], ToL10_RgnDtl_diff_mark[5], ToL10_RgnDtl_diff_mark[6], ToL10_RgnDtl_diff_mark[7]);
 	}
	#endif
	#endif
	u1_RP_detect_true_pre = pOutput->u1_RP_detect_true;
	u6_RP_frm_in_cnt_pre = u6_RP_frm_in_cnt;
	u6_RP_frm_in_cnt_thl_pre = u6_RP_frm_in_cnt_thl;
	u8_RP_frm_hold_cnt_pre = u8_RP_frm_hold_cnt;
	u1_det_true_pre = u1_det_true;
	u1_det_start_pre=u1_det_start;
	u1_basic_cond_true_pre= u1_basic_cond_true;
	u1_mcDtl_move_true_pre=u1_mcDtl_move_true;
	u1_rDTL_move_true_pre=u1_rDTL_move_true;
	u1_rPRD_move_true_pre=u1_rPRD_move_true;
	u1_dtl_all_cond_true_pre=u1_dtl_all_cond_true;
	u2_mcDtl_HV_pre=u2_mcDtl_HV;
	u2_GMV_single_move_pre=u2_GMV_single_move;
	u2_rDTL_RMV_cnt_pre=u2_rDTL_RMV_cnt;
	u2_rPRD_RMV_cnt_pre=u2_rPRD_RMV_cnt;
	u27_me_aTC_rb_pre = s_pContext->_output_read_comreg.u27_me_aTC_rb;
	u1_maxGMV_x_t_pre=u1_maxGMV_x_t;
	u1_maxGMV_y_t_pre = u1_maxGMV_y_t;
	u1_secGMV_x_t_pre=u1_secGMV_x_t;
	u1_secGMV_y_t_pre=u1_secGMV_y_t;
	u1_GMV32_t_pre=u1_GMV32_t;
	u8_bad_region_cnt_pre=u8_bad_region_cnt;
	u19_me_statis_glb_prd_pre=u19_me_statis_glb_prd;
	u16_mcHdtl_pre=u16_mcHdtl;
	u16_mcVdtl_pre=u16_mcVdtl;
	u11_GMV_x_max_pre=u11_GMV_x_max;
	u10_GMV_y_max_pre=u10_GMV_y_max;
	u11_GMV_x_sec_pre=u11_GMV_x_sec;
	u10_GMV_y_sec_pre=u10_GMV_y_sec;
	u8_rmove_1_cnt_pre=u8_rmove_1_cnt;
	u8_rmove_2_cnt_pre=u8_rmove_2_cnt;
	u8_rmove_3_cnt_pre=u8_rmove_3_cnt;
	u8_rmove_4_cnt_pre=u8_rmove_4_cnt;
	u8_rmove_rgn1_cnt_pre=u8_rmove_rgn1_cnt;
	u8_rmove_rgn2_cnt_pre=u8_rmove_rgn2_cnt;
	u8_rmove_rgn3_cnt_pre=u8_rmove_rgn3_cnt;
	u8_rmove_rgn4_cnt_pre=u8_rmove_rgn4_cnt;
	u17_me_ZMV_cnt_rb_pre=s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb;
	u25_me_aDTL_rb_ShiftBit_pre=(s_pContext->_output_read_comreg.u25_me_aDTL_rb>>u8_me_aDTL_ShiftBit);



	frm_cnt ++;
	if (frm_cnt >= 100000) frm_cnt = 0;
}

VOID MEMC_Natural_HighFreq_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u26_aDTL_rb=s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	unsigned int u32_rDTL_th_1 = 100000,u32_rDTL_th_2 = 75000,u32_aDTL_th_1 = 1500000,u32_rDTL_Square_th_1 = 90000,u32_rDTL_Square_th_2 = 70000,u32_rDTL_Square_sum=0,u32_rDTL_Square_agv=0;
	unsigned char  nIdxX,nIdxY,nSquareIdx,nMap0,nMap1,nMap2,nMap3,nMap4,nMap5;
	unsigned int u32_rDTL[6]={0};
	unsigned char u1_rDTL_true = 0,u1_aDTL_true = 0,u1_rDTL_Square_true = 0,u1_rDTL_SixSquare_true,u1_rDTL_Separate_true;
	static unsigned char  u8_Natural_HighFreq_holdFrm=0;
	unsigned char u8_rDTL_cnt = 0,u8_rDTL_ThreeSquare_cnt= 0,u8_rDTL_ThreeSquare_cnt_thl=20;

	//DBG
	unsigned int u32_RB_u32_rDTL_th_1,u32_RB_u32_aDTL_th_1,u32_RB_u32_rDTL_Square_th_1,u32_RB_u32_rDTL_th_2,u32_RB_u32_rDTL_Square_th_2,u32_RB_u8_rDTL_ThreeSquare_cnt_thl;
	ReadRegister(SOFTWARE1_SOFTWARE1_09_reg, 0, 31, &u32_RB_u32_rDTL_th_1);
	ReadRegister(SOFTWARE1_SOFTWARE1_10_reg, 0, 31, &u32_RB_u32_aDTL_th_1);
	ReadRegister(SOFTWARE1_SOFTWARE1_11_reg, 0, 31, &u32_RB_u32_rDTL_Square_th_1);
	ReadRegister(SOFTWARE1_SOFTWARE1_17_reg, 0, 31, &u32_RB_u32_rDTL_th_2);//u32_rDTL_th_2
	ReadRegister(SOFTWARE1_SOFTWARE1_18_reg, 0, 31, &u32_RB_u32_rDTL_Square_th_2);//u32_rDTL_Square_th_2
	ReadRegister(SOFTWARE1_SOFTWARE1_19_reg, 0, 31, &u32_RB_u8_rDTL_ThreeSquare_cnt_thl);//u8_rDTL_ThreeSquare_cnt_thl
	// ===DBG APPLY
	u32_rDTL_th_1 = u32_RB_u32_rDTL_th_1;
	u32_aDTL_th_1 = u32_RB_u32_aDTL_th_1;
	u32_rDTL_Square_th_1 = u32_RB_u32_rDTL_Square_th_1;
	u32_rDTL_th_2=u32_RB_u32_rDTL_th_2;
	u32_rDTL_Square_th_2=u32_RB_u32_rDTL_Square_th_2;
	u8_rDTL_ThreeSquare_cnt_thl=u32_RB_u8_rDTL_ThreeSquare_cnt_thl;
	//============

	//glb
	u1_aDTL_true = (u26_aDTL_rb>=u32_aDTL_th_1) ? 1:0;

	//=== Square  dtl ====
	for (nIdxX=0; nIdxX<=5; nIdxX++)
	{
		for (nIdxY=0; nIdxY<=2; nIdxY++)
		{
			// init
			u32_rDTL_Square_sum = 0;
			nMap0 = nIdxY*8+nIdxX;
			nMap1 = nIdxY*8+nIdxX+1;
			nMap2 = nIdxY*8+nIdxX+2;
			nMap3 = (nIdxY+1)*8+nIdxX;
			nMap4 = (nIdxY+1)*8+nIdxX+1;
			nMap5 = (nIdxY+1)*8+nIdxX+2;

			u32_rDTL[0] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap0];
			u32_rDTL[1] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap1];
			u32_rDTL[2] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap2];
			u32_rDTL[3] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap3];
			u32_rDTL[4] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap4];
			u32_rDTL[5] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap5];

			//Square
			for (nSquareIdx=0; nSquareIdx<=5; nSquareIdx++)
			{
				if(u32_rDTL[nSquareIdx] >=u32_rDTL_th_1)
					u32_rDTL_Square_sum +=u32_rDTL[nSquareIdx];
			}

			u32_rDTL_Square_agv = u32_rDTL_Square_sum/6;

			//check  Six Square
			u1_rDTL_SixSquare_true=(u32_rDTL_Square_agv>=u32_rDTL_Square_th_1)  ?1 :0;
			u1_rDTL_Square_true = u1_rDTL_Square_true || u1_rDTL_SixSquare_true;
		}
	}

	//rgn dtl
	for (nIdxX=0; nIdxX<=31; nIdxX++)
	{
		if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[nIdxX] >=u32_rDTL_th_1)
			u8_rDTL_cnt++;
	}
	//check rDtl
	if(u8_rDTL_cnt >=12)
		u1_rDTL_true=1;

	//=== Separate rgn dtl ======
	for (nIdxX=0; nIdxX<=5; nIdxX++)
	{
		for (nIdxY=0; nIdxY<=3; nIdxY++)
		{
			// init
			u32_rDTL_Square_sum = 0;
			nMap0 = nIdxY*8+nIdxX;
			nMap1 = nIdxY*8+nIdxX+1;
			nMap2 = nIdxY*8+nIdxX+2;

			u32_rDTL[0] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap0];
			u32_rDTL[1] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap1];
			u32_rDTL[2] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap2];

			//Square
			for (nSquareIdx=0; nSquareIdx<=2; nSquareIdx++)
			{
				if(u32_rDTL[nSquareIdx] >=u32_rDTL_th_2)
					u32_rDTL_Square_sum +=u32_rDTL[nSquareIdx];
			}

			u32_rDTL_Square_agv = u32_rDTL_Square_sum/3;

			//check  three Square
			if(u32_rDTL_Square_agv>=u32_rDTL_Square_th_2)
				u8_rDTL_ThreeSquare_cnt ++;
		}
	}
	//check Separate rgn dtl
	u1_rDTL_Separate_true = (u8_rDTL_ThreeSquare_cnt>=u8_rDTL_ThreeSquare_cnt_thl)  ?1 :0;
	//all
	pOutput->u1_Natural_HighFreq_detect_true = pParam->u1_Natural_HighFreq_en && u1_aDTL_true && ((u1_rDTL_Square_true&&u1_rDTL_true) || u1_rDTL_Separate_true);

	if(pOutput->u1_Natural_HighFreq_detect_true)
	{
		pOutput->u1_Natural_HighFreq_detect_true =1;
		u8_Natural_HighFreq_holdFrm = 8;
	}
	else if(u8_Natural_HighFreq_holdFrm >0)
	{
		pOutput->u1_Natural_HighFreq_detect_true =1;
		u8_Natural_HighFreq_holdFrm--;
	}
	else
	{
		pOutput->u1_Natural_HighFreq_detect_true =0;
		u8_Natural_HighFreq_holdFrm=0;
	}
}
VOID MEMC_Casino_RP_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//Casino
	static unsigned char casino_frm_cnt = 0;
	static unsigned char casino_frm_hold_cnt = 0;
	unsigned char casino_RP_true;
	static unsigned char casino_start = 0;
	unsigned char casino_region_en[32] = {0};
	unsigned char casino_region_idx;
	unsigned char casino_gmv_cond;
	unsigned int casino_dtl_sum = 0, casino_dtl_avg_cond, casino_dtl_var, casino_dtl_var_sum = 0, casino_dtl_var_avg_cond;
	unsigned char casino_region_en_sum_cond = 10;
	unsigned char casino_cond = 0;
	signed short casino_global_GMV_x = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short casino_global_GMV_y = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;

	// Casino_patch : write by fanny @20180619
	casino_gmv_cond = (_ABS_(casino_global_GMV_x) + _ABS_(casino_global_GMV_y) < 12) ;

	casino_region_en[8] = 1;
	casino_region_en[9] = 1;
	casino_region_en[10] = 1;
	casino_region_en[14] = 1;
	casino_region_en[15] = 1;
	casino_region_en[16] = 1;
	casino_region_en[17] = 1;
	casino_region_en[18] = 1;
       casino_region_en[22] = 1;
	casino_region_en[23] = 1;

	for(casino_region_idx = 0; casino_region_idx < 32; casino_region_idx++)
	{
		if(casino_region_en[casino_region_idx])
		{
			casino_dtl_sum += s_pContext->_output_read_comreg.u20_me_rDTL_rb[casino_region_idx];
		}
	}
	casino_dtl_avg_cond = casino_dtl_sum/casino_region_en_sum_cond;
	for(casino_region_idx = 0;casino_region_idx < 32;casino_region_idx++)
		{
		if(casino_region_en[casino_region_idx])
			{
			casino_dtl_var = _ABS_(casino_dtl_avg_cond - s_pContext->_output_read_comreg.u20_me_rDTL_rb[casino_region_idx]);
		       casino_dtl_var_sum += casino_dtl_var;
		}
	}
	casino_dtl_var_avg_cond =  casino_dtl_var_sum/casino_region_en_sum_cond;

	casino_cond = (casino_dtl_avg_cond > 80000 && casino_dtl_var_avg_cond < 12000 && casino_gmv_cond);
	//rtd_pr_memc_info("fanny_test : %d,%d,%d,%d,(%d,%d),\n",casino_dtl_avg_cond,casino_dtl_var_avg_cond,(abs(casino_global_GMV_x) + abs(casino_global_GMV_y)),casino_cond,casino_global_GMV_x,casino_global_GMV_y);
	if(casino_cond)
		casino_frm_cnt++;
	else
		casino_frm_cnt = 0;

	if(casino_frm_cnt >= 5)
		casino_frm_cnt = 5;
	if(casino_frm_cnt <= 0)
		casino_frm_cnt = 0;

	if(casino_frm_cnt >= 5)
		casino_start = 1;
	else
		casino_start = 0;

	if((casino_start == 1) && (casino_frm_hold_cnt == 0)){
		casino_frm_hold_cnt++;
		casino_RP_true = 1;
	}
	else if((casino_frm_hold_cnt > 0) && (casino_frm_hold_cnt <= 48)){
		casino_frm_hold_cnt++;
		casino_RP_true = 1;
	}
	else{
		casino_frm_hold_cnt = 0;
		casino_RP_true = 0;
		casino_start = 0;
	}

	if(casino_start == 0)
		casino_RP_true = 0;

	pOutput->u1_casino_RP_detect_true = casino_cond;
}
unsigned char MEMC_RepeatBG_detect(VOID)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u32_rDTL_th_1 = 30000, u32_rDTL_th_2 = 45000,u32_rDTL_th_3 = 60000,u32_rPRD_th_1 = 150;

	unsigned char  nIdxX,nIdxY,nSquareIdx,nMap0,nMap1,nMap2,nMap3;
	unsigned int u32_rDTL[4]={0};
	unsigned int u32_rPRD[4]={0};
	unsigned int u32_mc_rVHdtl[4]={0};
	unsigned char u1_rDTL_true = 0,u1_rPRD_true = 0,u1_mc_rVHdtl_true = 0;
	unsigned char u1_RepeatBG_true = 0;
	unsigned char u1_SquareRepeat_true = 0;
	unsigned char u8_RepeatBG_cnt = 0;


	int s32_rMV_x[4]={0};
	int s32_rMV_y[4]={0};
	unsigned int rMV_diff_true=0, u32_RB_u32_rMV_diff_th=0, u32_rMV_diff_th=20, rMV_diff=0,u32_RB_u32_rMV_th_min=0;
	unsigned int rMV_true=0, u32_RB_u32_rMV_th=0, u32_rMV_th=16, rMV_max=0, rMV_min=0, u32_rMV_th_min=2;
	int rmv_x_max, rmv_x_min, rmv_y_max, rmv_y_min;

	//DBG
	unsigned int u32_RB_u32_rDTL_th_1,u32_RB_u32_rDTL_th_2,u32_RB_u32_rDTL_th_3,u32_RB_u32_rPRD_th_1;
	ReadRegister(SOFTWARE1_SOFTWARE1_04_reg, 0, 31, &u32_RB_u32_rDTL_th_1);
	ReadRegister(SOFTWARE1_SOFTWARE1_05_reg, 0, 31, &u32_RB_u32_rDTL_th_2);
	ReadRegister(SOFTWARE1_SOFTWARE1_06_reg, 0, 31, &u32_RB_u32_rDTL_th_3);
	ReadRegister(SOFTWARE1_SOFTWARE1_07_reg, 0, 31, &u32_RB_u32_rPRD_th_1);
	ReadRegister(SOFTWARE1_SOFTWARE1_13_reg, 0, 31, &u32_RB_u32_rMV_th_min);
	ReadRegister(SOFTWARE1_SOFTWARE1_14_reg, 0, 31, &u32_RB_u32_rMV_diff_th);
	ReadRegister(SOFTWARE1_SOFTWARE1_15_reg, 0, 31, &u32_RB_u32_rMV_th);
	// ===DBG APPLY
	u32_rDTL_th_1 = u32_RB_u32_rDTL_th_1;
	u32_rDTL_th_2 = u32_RB_u32_rDTL_th_2;
	u32_rDTL_th_3 = u32_RB_u32_rDTL_th_3;
	u32_rPRD_th_1 = u32_RB_u32_rPRD_th_1;
	u32_rMV_diff_th = u32_RB_u32_rMV_diff_th;
	u32_rMV_th=u32_RB_u32_rMV_th;
	u32_rMV_th_min = u32_RB_u32_rMV_th_min;

	//============
	for (nIdxX=0; nIdxX<=6; nIdxX++)
	{
		for (nIdxY=0; nIdxY<=2; nIdxY++)
		{
			// init
			rmv_x_max=-2000000; rmv_x_min=2000000; rmv_y_max=-2000000; rmv_y_min=2000000;

			u1_rDTL_true = 0;u1_rPRD_true = 0;u1_mc_rVHdtl_true = 0;rMV_diff_true=0;rMV_true=0;
			u8_RepeatBG_cnt = 0;
			nMap0 = nIdxY*8+nIdxX;
			nMap1 = nIdxY*8+nIdxX+1;
			nMap2 = (nIdxY+1)*8+nIdxX;
			nMap3 = (nIdxY+1)*8+nIdxX+1;

			u32_rDTL[0] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap0];
			u32_rDTL[1] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap1];
			u32_rDTL[2] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap2];
			u32_rDTL[3] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[nMap3];

			u32_rPRD[0] = s_pContext->_output_read_comreg.u13_me_rPRD_rb[nMap0];
			u32_rPRD[1] = s_pContext->_output_read_comreg.u13_me_rPRD_rb[nMap1];
			u32_rPRD[2] = s_pContext->_output_read_comreg.u13_me_rPRD_rb[nMap2];
			u32_rPRD[3] = s_pContext->_output_read_comreg.u13_me_rPRD_rb[nMap3];

			u32_mc_rVHdtl[0] = s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[nMap0];
			u32_mc_rVHdtl[1] = s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[nMap1];
			u32_mc_rVHdtl[2] = s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[nMap2];
			u32_mc_rVHdtl[3] = s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[nMap3];

			s32_rMV_x[0] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap0];
			s32_rMV_x[1] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap1];
			s32_rMV_x[2] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap2];
			s32_rMV_x[3] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nMap3];

			s32_rMV_y[0] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap0];
			s32_rMV_y[1] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap1];
			s32_rMV_y[2] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap2];
			s32_rMV_y[3] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nMap3];

			// rDtl
			for (nSquareIdx=0; nSquareIdx<=3; nSquareIdx++)
			{
				if(u32_rDTL[nSquareIdx] >=u32_rDTL_th_3)
					u8_RepeatBG_cnt +=3;
				else if(u32_rDTL[nSquareIdx] >=u32_rDTL_th_2)
					u8_RepeatBG_cnt +=2;
				else if(u32_rDTL[nSquareIdx] >=u32_rDTL_th_1)
					u8_RepeatBG_cnt +=1;
			}

			//check rDtl
			if(u8_RepeatBG_cnt >=8)
				u1_rDTL_true=1;

			//rPRD
			u8_RepeatBG_cnt = 0;
			for (nSquareIdx=0; nSquareIdx<=3; nSquareIdx++)
			{
				if(u32_rPRD[nSquareIdx] >=u32_rPRD_th_1)
					u8_RepeatBG_cnt +=1;
			}
			//check rPRD
			if(u8_RepeatBG_cnt ==4)
				u1_rPRD_true=1;

			// mc_rVHdtl
			u8_RepeatBG_cnt = 0;
			for (nSquareIdx=0; nSquareIdx<=3; nSquareIdx++)
			{
				if(u32_mc_rVHdtl[nSquareIdx] <0x3)
					u8_RepeatBG_cnt +=1;
			}

			//check mc_rVHdtl
			if(u8_RepeatBG_cnt ==4)
				u1_mc_rVHdtl_true=1;


			// rMV diff
			for(nSquareIdx=0; nSquareIdx<=3; nSquareIdx++)
			{
				if(rmv_x_max<s32_rMV_x[nSquareIdx])
					rmv_x_max=s32_rMV_x[nSquareIdx];
				if(rmv_x_min>s32_rMV_x[nSquareIdx])
					rmv_x_min=s32_rMV_x[nSquareIdx];
				if(rmv_y_max<s32_rMV_y[nSquareIdx])
					rmv_y_max=s32_rMV_y[nSquareIdx];
				if(rmv_y_min>s32_rMV_y[nSquareIdx])
					rmv_y_min=s32_rMV_y[nSquareIdx];
			}

			rMV_diff =_MAX_(rmv_x_max-rmv_x_min, rmv_y_max-rmv_y_min);
			rMV_max  =_MAX_(_MAX_(_ABS_(rmv_x_max), _ABS_(rmv_x_min)), _MAX_(_ABS_(rmv_y_max), _ABS_(rmv_y_min)));
			rMV_min	 =_MIN_(_MIN_(_ABS_(rmv_x_max), _ABS_(rmv_x_min)), _MAX_(_ABS_(rmv_y_max), _ABS_(rmv_y_min)));

			if(rMV_diff>u32_rMV_diff_th)
				rMV_diff_true=1;
			else if(rMV_diff && rMV_max<u32_rMV_th && rMV_min>u32_rMV_th_min )
				rMV_true=1;

			//check RepeatBG
			//check RepeatBG
			u1_SquareRepeat_true =  (u1_rDTL_true&&u1_rPRD_true &&u1_mc_rVHdtl_true&&(rMV_true || rMV_diff_true))  ? 1 : 0;
			u1_RepeatBG_true =  (u1_SquareRepeat_true||u1_RepeatBG_true) ? 1 : 0;
			WriteRegister(SOFTWARE1_SOFTWARE1_20_reg, nMap0, nMap0, u1_SquareRepeat_true);
		}
	}
	return u1_RepeatBG_true;
}
unsigned char MEMC_BlackSC_detect(VOID)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u1_black_sc=0;		//black scene or sc occur
	static unsigned char u8_black_sc_cnt = 0;
	
	unsigned short u16_mcHdtl = s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt;
	unsigned short u16_mcVdtl = s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt;

	//black pattern&sc but not static
	u1_black_sc = ((u16_mcHdtl==0)&&(u16_mcVdtl==0))||((s_pContext->_output_read_comreg.u1_sc_status_rb==1)&&(s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb<0x6000));

	if(u1_black_sc)	// for after black pattern, img dtl not stable
	{
		u8_black_sc_cnt = 0;
	}
	else
	{
		u8_black_sc_cnt += FRAME_CHANGE_CNT;

		if(u8_black_sc_cnt>60)
		{
			u8_black_sc_cnt = 60;
		}
	}
		
	return u8_black_sc_cnt;
}
unsigned char MEMC_RMV_Consistent_detect(VOID)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	signed short s11_maxGMV_x = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	signed short s10_maxGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	unsigned int u32_i = 0, u32_rmv_cnt_sum = 0, u32_cnt = 0, rmv_cnt_avg = 0;
	unsigned char u1_rmv_consistent_true = 0;
	unsigned int u32_rmv_cnt_avg_th=890;
	ReadRegister(SOFTWARE1_SOFTWARE1_16_reg, 0, 31, &u32_rmv_cnt_avg_th);
	for (u32_i = 0; u32_i < 32; u32_i ++)
	{
		if(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[u32_i]<2 && _ABS_DIFF_(s11_maxGMV_x,(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u32_i]>>1)) < 2 && _ABS_DIFF_(s10_maxGMV_y,(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u32_i]>>1)) < 2 &&( s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u32_i]!=0 || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u32_i] !=0))
		{
			u32_rmv_cnt_sum += s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[u32_i];
			u32_cnt++;
		}
	}

	if(u32_cnt)
	{
		rmv_cnt_avg = u32_rmv_cnt_sum/32;

		if(rmv_cnt_avg >= u32_rmv_cnt_avg_th)
			u1_rmv_consistent_true = 1;
		else
			u1_rmv_consistent_true = 0;
	}
	return u1_rmv_consistent_true;
}
unsigned char MEMC_GMV_detect(unsigned char * u11_GMV_x_max_rb,unsigned char *u10_GMV_y_max_rb)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u1_gmv_true = 1;
	unsigned char u1_maxGMV_x_t;
	unsigned char u1_secGMV_x_t;
	unsigned char u1_maxGMV_y_t;
	unsigned char u1_secGMV_y_t;
	unsigned char u11_GMV_x_max=0;
	unsigned char u10_GMV_y_max=0;
	unsigned char u11_GMV_x_sec=0;
	unsigned char u10_GMV_y_sec=0;
	unsigned char u8_GMV_th0 = 0xA;//0x96;
	unsigned char u8_GMV_th1 =0x12;// for K-17716, 0x18 -> 0x12; //0x96;
	signed short s11_maxGMV_x = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>2;
	signed short s11_secGMV_x = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb)>>2;
	signed short s10_maxGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>2;
	signed short s10_secGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb)>>2;

	u1_maxGMV_x_t = (_ABS_(s11_maxGMV_x) <= u8_GMV_th0)? 1 : 0;
	u1_maxGMV_y_t = (_ABS_(s10_maxGMV_y) <= u8_GMV_th0)? 1 : 0;

	u11_GMV_x_max = _ABS_(s11_maxGMV_x);
	u10_GMV_y_max = _ABS_(s10_maxGMV_y);

	if(s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb>0 )
	{
		u1_secGMV_x_t = (_ABS_(s11_secGMV_x) <= u8_GMV_th1)? 1 : 0;
		u1_secGMV_y_t = (_ABS_(s10_secGMV_y) <= u8_GMV_th1)? 1 : 0;

		u11_GMV_x_sec = _ABS_(s11_secGMV_x);
		u10_GMV_y_sec = _ABS_(s10_secGMV_y);

		u11_GMV_x_max = _MAX_(u11_GMV_x_max, u11_GMV_x_sec);
		u10_GMV_y_max = _MAX_(u10_GMV_y_max, u10_GMV_y_sec);
	}
	else
	{
		u11_GMV_x_sec = _ABS_(s11_secGMV_x);
		u10_GMV_y_sec = _ABS_(s10_secGMV_y);
		u1_secGMV_x_t = 1;
		u1_secGMV_y_t = 1;
	}
	*u11_GMV_x_max_rb=u11_GMV_x_max;
	*u10_GMV_y_max_rb=u10_GMV_y_max;


	u1_gmv_true = (u1_maxGMV_x_t && u1_maxGMV_y_t && u1_secGMV_x_t && u1_secGMV_y_t )? 1 : 0;

	return u1_gmv_true;
}

unsigned char MEMC_RGNDtlPRD_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	int i,j;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	//unsigned int u32_rPRD_th = 100, u32_rDTL_th = 50000;
	unsigned int u32_rPRD_th_2 = 500, u32_rDTL_th_2 = 95000, u32_gDTL_Step_th = 850;//u32_rDTL_th_2 = 80000, u32_gDTL_Step_th = 500;
	//static char u2_rDtlPRD_RMV_cnt = 0;
	//unsigned char u8_rmove_rgnDtlPRD_H_cnt[4]= {0}, u8_rmove_rgnDtlPRD_V_cnt[8]= {0};
	//unsigned char u8_rDtlPRD_RMV_mark[32] = {0};
	unsigned int u32_gDTL_Step_check = 0;
	unsigned char u8_rDtl_N2H_mark[28] = {0},u8_rPRD_N2H_mark[28] = {0};
	unsigned char u8_rgnDtlPRD_N4V_cnt[7]= {0},u8_rgnDtlPRD_N4by2_cnt[12]= {0};
	signed short  s16_rgnDtlPRD_N4V_mark_cnt= 0,s16_rgnDtlPRD_N4by2_mark_cnt= 0;
	static signed short s16_rDtlPRD_RMV_cnt = 0;
	unsigned int sum_me_rPRD_rb = 0;
	static signed short s16_rDtlPRD_RMV_th_cnt = 0;
	//unsigned char u1_RGNDtlPRD_level = 1;
	//signed short s11_maxGMV_x = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>2;
	//signed short s11_secGMV_x = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb)>>2;
	//signed short s10_maxGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>2;
	//signed short s10_secGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb)>>2;

	//if(((s11_maxGMV_x+s10_maxGMV_y)==0) && ((s11_secGMV_x+s10_secGMV_y)==0)){
	//u32_rPRD_th_2 = u32_rPRD_th_2+900;
		//s16_rDtlPRD_RMV_th_cnt = 3;
	//}
	//unsigned int u32_RB_val;
	//ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);

	if(s_pContext->_output_frc_sceneAnalysis.u1_still_frame || s_pContext->_external_info.u1_VIP_Still){// add th for PRD and DTL th of still frame when still image 
		//u32_rPRD_th_2 = u32_rPRD_th_2+900;
		u32_rPRD_th_2 = u32_rPRD_th_2+700;
		u32_rDTL_th_2 = u32_rDTL_th_2+60000;
		s16_rDtlPRD_RMV_th_cnt = 5;
	}

	if(s16_rDtlPRD_RMV_th_cnt>0){
		u32_rPRD_th_2 = u32_rPRD_th_2+900;
		u32_rDTL_th_2 = u32_rDTL_th_2+50000;
		//u32_rDTL_th_2 = u32_rDTL_th_2+60000;
		s16_rDtlPRD_RMV_th_cnt--;
	}

	if(s16_rDtlPRD_RMV_th_cnt<0) s16_rDtlPRD_RMV_th_cnt = 0;

#if 0
	for (i=0; i<32; i++){
		// rDtlPRD move check
		if( (s_pContext->_output_read_comreg.u20_me_rDTL_rb[i]>u32_rDTL_th)
			&& (s_pContext->_output_read_comreg.u13_me_rPRD_rb[i]>u32_rPRD_th)
			&& (s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]!=0 || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]!=0)
			/*&& ( _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i])<=12 || _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i])<=12)*/){
				u8_rDtlPRD_RMV_mark[i] = 1;
		}

		if(u8_rDtlPRD_RMV_mark[i] == 1 )
			u2_rDtlPRD_RMV_cnt++;
		else
			u2_rDtlPRD_RMV_cnt--;

	}

	for (i=0; i<4; i++){
		u8_rmove_rgnDtlPRD_H_cnt[i] = u8_rDtlPRD_RMV_mark[i*8] + u8_rDtlPRD_RMV_mark[i*8+1] + u8_rDtlPRD_RMV_mark[i*8+2] + u8_rDtlPRD_RMV_mark[i*8+3]
										+ u8_rDtlPRD_RMV_mark[i*8+4]+ u8_rDtlPRD_RMV_mark[i*8+5]+ u8_rDtlPRD_RMV_mark[i*8+6]+ u8_rDtlPRD_RMV_mark[i*8+7];

		if(u8_rmove_rgnDtlPRD_H_cnt[i] >=7 )
			u2_rDtlPRD_RMV_cnt+=15;
		else if(u8_rmove_rgnDtlPRD_H_cnt[i] >= 5 )
			u2_rDtlPRD_RMV_cnt+=10;
		else if(u8_rmove_rgnDtlPRD_H_cnt[i] >= 3)
			u2_rDtlPRD_RMV_cnt+=5;
		else if(u8_rmove_rgnDtlPRD_H_cnt[i] == 0)
			u2_rDtlPRD_RMV_cnt-=5;
	}

	for (i=0; i<8; i++){
		u8_rmove_rgnDtlPRD_V_cnt[i] = u8_rDtlPRD_RMV_mark[i] + u8_rDtlPRD_RMV_mark[i+8] + u8_rDtlPRD_RMV_mark[i+16] + u8_rDtlPRD_RMV_mark[i+24];

		if(u8_rmove_rgnDtlPRD_V_cnt[i] >= 4 )
			u2_rDtlPRD_RMV_cnt+=10;
		else if(u8_rmove_rgnDtlPRD_V_cnt[i] >= 2 )
			u2_rDtlPRD_RMV_cnt+=4;
		else if(u8_rmove_rgnDtlPRD_V_cnt[i] == 0 )
			u2_rDtlPRD_RMV_cnt-=4;
	}
#endif

	for (i=0; i<7; i++){
		for (j=0; j<4; j++){
			sum_me_rPRD_rb = 0;

			if((s_pContext->_output_read_comreg.u20_me_rDTL_rb[j*8+i] + s_pContext->_output_read_comreg.u20_me_rDTL_rb[j*8+i+1]) >= u32_rDTL_th_2){
				u8_rDtl_N2H_mark[j*7+i] = 1;
			}else{
				u8_rDtl_N2H_mark[j*7+i] = 0;
			}

			sum_me_rPRD_rb = (s_pContext->_output_read_comreg.u13_me_rPRD_rb[j*8+i])+(s_pContext->_output_read_comreg.u13_me_rPRD_rb[j*8+i+1]);

			//if(sum_me_rPRD_rb >=800)
		//		sum_me_rPRD_rb = sum_me_rPRD_rb -800;
		//	else
		//		sum_me_rPRD_rb = 0;

			if( sum_me_rPRD_rb >= u32_rPRD_th_2){
				u8_rPRD_N2H_mark[j*7+i] = 1;
			}else{
				u8_rPRD_N2H_mark[j*7+i] = 0;
			}

#if 0 // debug
			if(((u32_RB_val >>	31) & 0x01) == 1){
				if(u8_rPRD_N2H_mark[j*7+i]==1){
					rtd_pr_memc_emerg("[MEMC]u8_rPRD_N2H_mark[%d]=%d(%d)(%d) \n",j*7+i,u8_rPRD_N2H_mark[j*7+i],sum_me_rPRD_rb,s_pContext->_output_frc_sceneAnalysis.u1_still_frame);
				}

				if(u8_rDtl_N2H_mark[j*7+i] == 1){
					rtd_pr_memc_emerg("[MEMC]u8_rDtl_N2H_mark[%d]=%d(%d)(%d) \n",j*7+i,u8_rDtl_N2H_mark[j*7+i],(s_pContext->_output_read_comreg.u20_me_rDTL_rb[j*8+i] + s_pContext->_output_read_comreg.u20_me_rDTL_rb[j*8+i+1]),s_pContext->_output_frc_sceneAnalysis.u1_still_frame);
				}
			}
#endif
		}
	}

	for (i=0; i<7; i++){

		u8_rgnDtlPRD_N4V_cnt[i] = (u8_rDtl_N2H_mark[i]+u8_rPRD_N2H_mark[i])+(u8_rDtl_N2H_mark[i+7]+u8_rPRD_N2H_mark[i+7])
									+(u8_rDtl_N2H_mark[i+14]+u8_rPRD_N2H_mark[i+14])+(u8_rDtl_N2H_mark[i+21]+u8_rPRD_N2H_mark[i+21]);


		if(u8_rgnDtlPRD_N4V_cnt[i]>=8)
			s16_rgnDtlPRD_N4V_mark_cnt+=7;
		else if(u8_rgnDtlPRD_N4V_cnt[i]<=3)
			s16_rgnDtlPRD_N4V_mark_cnt--;

	}

#if 1
	for (i=0; i<4; i++){
		for(j=0; j<3; j++){
			u8_rgnDtlPRD_N4by2_cnt[j*4+i] = ((u8_rPRD_N2H_mark[j*7+i]+u8_rPRD_N2H_mark[j*7+i+1]+u8_rPRD_N2H_mark[j*7+i+2]+u8_rPRD_N2H_mark[j*7+i+3])
											+(u8_rPRD_N2H_mark[j*7+7+i]+u8_rPRD_N2H_mark[j*7+7+i+1]+u8_rPRD_N2H_mark[j*7+7+i+2]+u8_rPRD_N2H_mark[j*7+7+i+3]));

			if(u8_rgnDtlPRD_N4by2_cnt[j*4+i]>=7){
				s16_rgnDtlPRD_N4by2_mark_cnt+=7;
			}else if(u8_rgnDtlPRD_N4by2_cnt[j*4+i]<=4){
				s16_rgnDtlPRD_N4by2_mark_cnt--;
			}
		}
	}

	u32_gDTL_Step_check = s_pContext->_external_info.DTL_Step_Hist_sum_ratio[0];

	if(u32_gDTL_Step_check <= u32_gDTL_Step_th)
		s16_rgnDtlPRD_N4by2_mark_cnt = 0;


#else
	for (i=0; i<4; i++){
		for(j=0; j<3; j++){
			u8_rgnDtlPRD_N4by2_cnt[j*4+i] = ((u8_rDtl_N2H_mark[j*7+i]+u8_rDtl_N2H_mark[j*7+i+1]+u8_rDtl_N2H_mark[j*7+i+2]+u8_rDtl_N2H_mark[j*7+i+3])
											+(u8_rDtl_N2H_mark[j*7+7+i]+u8_rDtl_N2H_mark[j*7+7+i+1]+u8_rDtl_N2H_mark[j*7+7+i+2]+u8_rDtl_N2H_mark[j*7+7+i+3]))+
										    ((u8_rPRD_N2H_mark[j*7+i]+u8_rPRD_N2H_mark[j*7+i+1]+u8_rPRD_N2H_mark[j*7+i+2]+u8_rPRD_N2H_mark[j*7+i+3])
											+(u8_rPRD_N2H_mark[j*7+7+i]+u8_rPRD_N2H_mark[j*7+7+i+1]+u8_rPRD_N2H_mark[j*7+7+i+2]+u8_rPRD_N2H_mark[j*7+7+i+3]));

			if(u8_rgnDtlPRD_N4by2_cnt[j*4+i]>=16){
				s16_rgnDtlPRD_N4by2_mark_cnt+=16;
			}else if(u8_rgnDtlPRD_N4by2_cnt[j*4+i]<=14){
				s16_rgnDtlPRD_N4by2_mark_cnt--;
			}
		}
	}
#endif

	if((s16_rgnDtlPRD_N4V_mark_cnt>=1) || s16_rgnDtlPRD_N4by2_mark_cnt>=1)
		s16_rDtlPRD_RMV_cnt++;
	else
		s16_rDtlPRD_RMV_cnt--;

#if 0 // debug
	if(((u32_RB_val >>	31) & 0x01) == 1){

		if(s16_rDtlPRD_RMV_cnt>0){
			rtd_pr_memc_emerg("[MEMC]DtlPRD_N4V=%d,DtlPRD_N4by2=%d (%d)\n",s16_rgnDtlPRD_N4V_mark_cnt,s16_rgnDtlPRD_N4by2_mark_cnt,u32_gDTL_Step_check);
			rtd_pr_memc_emerg("[MEMC]s16_rDtlPRD_RMV_cnt=%d \n",s16_rDtlPRD_RMV_cnt);
		}
	}
#endif

	if(s16_rDtlPRD_RMV_cnt>10) s16_rDtlPRD_RMV_cnt = 10;
	if(s16_rDtlPRD_RMV_cnt<0) s16_rDtlPRD_RMV_cnt = 0;


	return s16_rDtlPRD_RMV_cnt;

}

unsigned char MEMC_RGNDtl_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	int i;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	static unsigned char u2_rDTL_cnt = 0;
	unsigned char case_index = 0;
	unsigned char u8_rDTL_CheckMask_case1[32] = {	0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,
													1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1};
	unsigned int u8_rDTL_MaskSum_case1[2] = {0,0};
	static unsigned char match_flag =0;
	unsigned char u1_rDTL_status_check = 1;
	unsigned int u32_RB_val;
		
	for (i=0; i<32; i++){		
		case_index = u8_rDTL_CheckMask_case1[i];
		if( i!=14 && i!=15){
			u8_rDTL_MaskSum_case1[case_index] = u8_rDTL_MaskSum_case1[case_index] + s_pContext->_output_read_comreg.u20_me_rDTL_rb[i];
		}
	}

	if(u2_rDTL_cnt>0){
		u2_rDTL_cnt--;
	}else{
		u2_rDTL_cnt = 0;
	}

	if(u8_rDTL_MaskSum_case1[0]<50000 && u8_rDTL_MaskSum_case1[1]>300000){

		if(u8_rDTL_MaskSum_case1[0]*10 < u8_rDTL_MaskSum_case1[1] ){
			u2_rDTL_cnt = 5;
			match_flag = 1;
		}
	}

	if(u2_rDTL_cnt==0){
		match_flag = 0;
	}
	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);//bit 25~31
	if((frm_cnt % 5 == 0  && ((u32_RB_val >>  13) & 0x01) ==1 )){
		rtd_pr_memc_emerg("[%s][%d][,%d,%d,][,%d,%d,]\n\r",__FUNCTION__, __LINE__,u8_rDTL_MaskSum_case1[0],u8_rDTL_MaskSum_case1[1],match_flag,u2_rDTL_cnt);
	}

	if(	match_flag > 0){
		u1_rDTL_status_check = match_flag;
	}
	else{
		u1_rDTL_status_check = 0;
	}

	return u1_rDTL_status_check;
	
}

unsigned char MEMC_RGNPRD_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	int i;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	static unsigned char u2_rPRD_cnt = 0;
	unsigned char case_index = 0;
	unsigned char u8_rPRD_CheckMask_case1[32] = {	0,0,0,0,0,0,0,0,
													0,0,0,0,0,0,0,0,
													1,1,1,1,1,1,1,1,
													1,1,1,1,1,1,1,1};
	unsigned int u8_rPRD_MaskSum_case1[2] = {0,0};
	static unsigned char match_flag =0;
	unsigned char u1_rPRD_status_check = 1;
	unsigned int u32_RB_val;
	
	for (i=0; i<32; i++){
		
		case_index = u8_rPRD_CheckMask_case1[i];
		if( i!=14 && i!=15){
			u8_rPRD_MaskSum_case1[case_index] = u8_rPRD_MaskSum_case1[case_index] + s_pContext->_output_read_comreg.u13_me_rPRD_rb[i];
		}	
	}

	if(u2_rPRD_cnt>0){
		u2_rPRD_cnt--;
	}else{
		u2_rPRD_cnt = 0;
	}

	if(u8_rPRD_MaskSum_case1[0]<100 && u8_rPRD_MaskSum_case1[1]>1500){

		if(u8_rPRD_MaskSum_case1[0]*40 < u8_rPRD_MaskSum_case1[1] ){
			u2_rPRD_cnt = 5;
			match_flag = 1;
		}
	}

	if(u2_rPRD_cnt==0){
		match_flag = 0;
	}

	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);//bit 25~31
	
	if((frm_cnt % 5 == 0  && ((u32_RB_val >>  13) & 0x01) ==1 )){
		rtd_pr_memc_emerg("[%s][%d][,%d,%d,][,%d,%d,]\n\r",__FUNCTION__, __LINE__,u8_rPRD_MaskSum_case1[0],u8_rPRD_MaskSum_case1[1],match_flag,u2_rPRD_cnt);
	}

	

	if(	match_flag > 0){
		u1_rPRD_status_check = match_flag;
	}
	else{
		u1_rPRD_status_check = 0;
	}

	return u1_rPRD_status_check;
	
}

unsigned char MEMC_RGNDtl_Move_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	int i;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u32_rDTL_th_1 = 66000, u32_rDTL_th_2 = 68000,u8_rmove_cnt_th = 1;
	unsigned char u8_GMV_th2 = 0x23;
	static char u2_rDTL_RMV_cnt = 0;
	unsigned char u8_rDTL_RMV_mark[32] = {0};
	unsigned char u8_rmove_rgn1_cnt= 0, u8_rmove_rgn2_cnt= 0, u8_rmove_rgn3_cnt= 0, u8_rmove_rgn4_cnt = 0;
	unsigned char  u8_rmove_1_cnt= 0, u8_rmove_2_cnt= 0, u8_rmove_3_cnt= 0, u8_rmove_4_cnt = 0,u8_bad_region_cnt = 0;
	unsigned char u1_rDTL_move_true = 1;
	
	for (i=0; i<32; i++)
		{
			if((_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i])>u8_GMV_th2)||(_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i])>u8_GMV_th2))
			{
				u8_bad_region_cnt++;
				//u1_GMV32_t = 0;
			}

			// rDtl move th1
			if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[i]>u32_rDTL_th_1 && (s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i] || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i])){

				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=8 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=8 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0))
					u8_rmove_1_cnt++;
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-8) && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=8 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0))
					u8_rmove_2_cnt++;
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-8) && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-8) && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0))
					u8_rmove_3_cnt++;
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=8 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-8) && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0))
					u8_rmove_4_cnt++;
			}

		#if 1
			// rDtl move th2
			if(s_pContext->_output_read_comreg.u20_me_rDTL_rb[i]>u32_rDTL_th_2 && (s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]!=0 || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]!=0)){
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=8 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-8)) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=8 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-8)))
					u8_rDTL_RMV_mark[i] = 1;
			}
		#endif

		}

	//rgn rDtl move
	u8_rmove_rgn1_cnt = u8_rDTL_RMV_mark[0] + u8_rDTL_RMV_mark[1] + u8_rDTL_RMV_mark[8] + u8_rDTL_RMV_mark[9] + u8_rDTL_RMV_mark[16] + u8_rDTL_RMV_mark[17] + u8_rDTL_RMV_mark[24] + u8_rDTL_RMV_mark[25];
	u8_rmove_rgn2_cnt = u8_rDTL_RMV_mark[2] + u8_rDTL_RMV_mark[3] + u8_rDTL_RMV_mark[10] + u8_rDTL_RMV_mark[11] + u8_rDTL_RMV_mark[18] + u8_rDTL_RMV_mark[19] + u8_rDTL_RMV_mark[26] + u8_rDTL_RMV_mark[27];
	u8_rmove_rgn3_cnt = u8_rDTL_RMV_mark[4] + u8_rDTL_RMV_mark[5] + u8_rDTL_RMV_mark[12] + u8_rDTL_RMV_mark[13] + u8_rDTL_RMV_mark[20] + u8_rDTL_RMV_mark[21] + u8_rDTL_RMV_mark[28] + u8_rDTL_RMV_mark[29];
	u8_rmove_rgn4_cnt = u8_rDTL_RMV_mark[6] + u8_rDTL_RMV_mark[7] + u8_rDTL_RMV_mark[14] + u8_rDTL_RMV_mark[15] + u8_rDTL_RMV_mark[22] + u8_rDTL_RMV_mark[23] + u8_rDTL_RMV_mark[30] + u8_rDTL_RMV_mark[31];

	if((u8_rmove_1_cnt+u8_rmove_2_cnt)>u8_rmove_cnt_th || (u8_rmove_2_cnt+u8_rmove_3_cnt)>u8_rmove_cnt_th || (u8_rmove_3_cnt+u8_rmove_4_cnt)>u8_rmove_cnt_th || (u8_rmove_4_cnt+u8_rmove_1_cnt)>u8_rmove_cnt_th){
		u2_rDTL_RMV_cnt++;
	}
	else if(u8_rmove_rgn1_cnt>=1 || u8_rmove_rgn2_cnt>=1 || u8_rmove_rgn3_cnt>=1 || u8_rmove_rgn4_cnt>=1){
		u2_rDTL_RMV_cnt++;
	}
	else{
		u2_rDTL_RMV_cnt--;
	}

	if(u2_rDTL_RMV_cnt>5) u2_rDTL_RMV_cnt = 5;
	if(u2_rDTL_RMV_cnt<0) u2_rDTL_RMV_cnt = 0;

	if(u2_rDTL_RMV_cnt>0){
		u1_rDTL_move_true = 1;
	}
	else{
		u1_rDTL_move_true = 0;
	}

	return u1_rDTL_move_true;
}

unsigned char MEMC_RGNPRD_Move_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	int i;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int u32_rPRD_th_1 = 220, u32_rPRD_th_2 = 280,  u8_rmove_cnt_th = 1;
	unsigned char u8_rmove_1_cnt= 0, u8_rmove_2_cnt= 0, u8_rmove_3_cnt= 0, u8_rmove_4_cnt = 0;
	unsigned char u8_rmove_rgn1_cnt= 0, u8_rmove_rgn2_cnt= 0, u8_rmove_rgn3_cnt= 0, u8_rmove_rgn4_cnt = 0;
	unsigned char u8_rPRD_RMV_mark[32] = {0};
	static char u2_rPRD_RMV_cnt = 0;
	unsigned char u1_rPRD_move_true = 1;
	
    //rgn rPRD move
		u8_rmove_1_cnt= 0; u8_rmove_2_cnt= 0; u8_rmove_3_cnt= 0; u8_rmove_4_cnt = 0;
		for (i=0; i<32; i++)
		{
			// rDtl move th1
			if(s_pContext->_output_read_comreg.u13_me_rPRD_rb[i]>u32_rPRD_th_1 && (s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i] || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i])){
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=8 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=8 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0))
					u8_rmove_1_cnt++;
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-8) && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=8 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=0))
					u8_rmove_2_cnt++;
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-8) && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-8) && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0))
					u8_rmove_3_cnt++;
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=8 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=0) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-8) && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=0))
					u8_rmove_4_cnt++;
			}
	#if 1
			// rPRD move th2
			if(s_pContext->_output_read_comreg.u13_me_rPRD_rb[i]>u32_rPRD_th_2 && (s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]!=0 || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]!=0)){
				if((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]<=8 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]>=(-8)) && (s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]<=8 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]>=(-8)))
					u8_rPRD_RMV_mark[i] = 1;
			}
	#endif

		}

		//rgn rPRD move
		u8_rmove_rgn1_cnt = u8_rPRD_RMV_mark[0] + u8_rPRD_RMV_mark[1] + u8_rPRD_RMV_mark[8] + u8_rPRD_RMV_mark[9] + u8_rPRD_RMV_mark[16] + u8_rPRD_RMV_mark[17] + u8_rPRD_RMV_mark[24] + u8_rPRD_RMV_mark[25];
		u8_rmove_rgn2_cnt = u8_rPRD_RMV_mark[2] + u8_rPRD_RMV_mark[3] + u8_rPRD_RMV_mark[10] + u8_rPRD_RMV_mark[11] + u8_rPRD_RMV_mark[18] + u8_rPRD_RMV_mark[19] + u8_rPRD_RMV_mark[26] + u8_rPRD_RMV_mark[27];
		u8_rmove_rgn3_cnt = u8_rPRD_RMV_mark[4] + u8_rPRD_RMV_mark[5] + u8_rPRD_RMV_mark[12] + u8_rPRD_RMV_mark[13] + u8_rPRD_RMV_mark[20] + u8_rPRD_RMV_mark[21] + u8_rPRD_RMV_mark[28] + u8_rPRD_RMV_mark[29];
		u8_rmove_rgn4_cnt = u8_rPRD_RMV_mark[6] + u8_rPRD_RMV_mark[7] + u8_rPRD_RMV_mark[14] + u8_rPRD_RMV_mark[15] + u8_rPRD_RMV_mark[22] + u8_rPRD_RMV_mark[23] + u8_rPRD_RMV_mark[30] + u8_rPRD_RMV_mark[31];

		if((u8_rmove_1_cnt+u8_rmove_2_cnt)>u8_rmove_cnt_th || (u8_rmove_2_cnt+u8_rmove_3_cnt)>u8_rmove_cnt_th || (u8_rmove_3_cnt+u8_rmove_4_cnt)>u8_rmove_cnt_th || (u8_rmove_4_cnt+u8_rmove_1_cnt)>u8_rmove_cnt_th){
			u2_rPRD_RMV_cnt++;
		}
		else if(u8_rmove_rgn1_cnt>=1 || u8_rmove_rgn2_cnt>=1 || u8_rmove_rgn3_cnt>=1 || u8_rmove_rgn4_cnt>=1){
			u2_rPRD_RMV_cnt++;
		}
		else{
			u2_rPRD_RMV_cnt--;
		}

		if(u2_rPRD_RMV_cnt>5) u2_rPRD_RMV_cnt = 5;
		if(u2_rPRD_RMV_cnt<0) u2_rPRD_RMV_cnt = 0;

		if(u2_rPRD_RMV_cnt>0){
			u1_rPRD_move_true = 1;
			//u8_black_sc_cnt = u8_black_sc_cnt_th; //quick start
		}
		else{
			u1_rPRD_move_true = 0;
		}

	return u1_rPRD_move_true;
}

unsigned char MEMC_HVDtl_Move_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u2_mcDtl_HV;	// 0: dtl small, 1: v dtl, 2: h dtl
	unsigned char u2_GMV_single_move;	// 0: similar, 1:y, 2:x
	unsigned char u1_mcDtl_move_true;
	unsigned char u8_GMV_diff = 0x6;
	unsigned char u11_GMV_x_max=0;
	unsigned char u10_GMV_y_max=0;

	unsigned short u16_mcDTL_thmin = 0x180;
	unsigned short u16_mcDTL_th0 = 0x230;
	unsigned short u16_mcDTL_th1 = 0x340;
	unsigned short u16_mcDTL_th2 = 0x3d0;
	unsigned short u16_mcDTL_th3 = 0x400;
	unsigned short u16_mcHdtl = s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt;
	unsigned short u16_mcVdtl = s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt;


	// GMV
	MEMC_GMV_detect(&u11_GMV_x_max,&u10_GMV_y_max);
	//======== mcDtl
	//gmv sigle move
	if(u11_GMV_x_max > u10_GMV_y_max + u8_GMV_diff)
	{
		u2_GMV_single_move = 2;
	}
	else if(u10_GMV_y_max>u11_GMV_x_max+u8_GMV_diff)
	{
		u2_GMV_single_move = 1;
	}
	else
	{
		u2_GMV_single_move = 0;
	}

	if (((u16_mcHdtl > u16_mcDTL_thmin) && (u16_mcHdtl > u16_mcVdtl * (6))) ||\
		((u16_mcHdtl > u16_mcDTL_th0) && (u16_mcHdtl > u16_mcVdtl * (5))) ||\
		((u16_mcHdtl > u16_mcDTL_th1) && (u16_mcHdtl > u16_mcVdtl * (4))) ||\
		((u16_mcHdtl > u16_mcDTL_th2) && (u16_mcHdtl > u16_mcVdtl * (3))) ||\
		((u16_mcHdtl > u16_mcDTL_th3) && (u16_mcHdtl > u16_mcVdtl * (2))))
	{
		u2_mcDtl_HV = 2;	//vertical lines
	}
	else if(((u16_mcVdtl > u16_mcDTL_thmin) && (u16_mcVdtl > u16_mcHdtl * (6)))||\
		((u16_mcVdtl > u16_mcDTL_th0) && (u16_mcVdtl > u16_mcHdtl * (5)))||\
		((u16_mcVdtl > u16_mcDTL_th1) && (u16_mcVdtl > u16_mcHdtl * (4))) ||\
		((u16_mcVdtl > u16_mcDTL_th2) && (u16_mcVdtl > u16_mcHdtl * (3))) ||\
		((u16_mcVdtl > u16_mcDTL_th3) && (u16_mcVdtl > u16_mcHdtl * (2))))
	{
		u2_mcDtl_HV = 1;	//horizantal lines
	}
	else
	{
		u2_mcDtl_HV = 0;
	}

	//mcDtl move
	if(((u2_mcDtl_HV==2)&&(u2_GMV_single_move !=1))||((u2_mcDtl_HV==1)&&(u2_GMV_single_move !=2)))
	{
		u1_mcDtl_move_true = 1;
	}
	else
	{
		u1_mcDtl_move_true = 0;
	}
	//=============

	pOutput->u2_GMV_single_move = u2_GMV_single_move;
	pOutput->u2_mcDtl_HV = u2_mcDtl_HV;

	return u1_mcDtl_move_true;
}
VOID MEMC_Periodic_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	static unsigned char u6_RP_out_print = 0;
	static unsigned char u6_RP_frm_in_cnt = 0;
	static unsigned char u6_RP_frm_in_cnt_thl = 8;
	static unsigned char u8_RP_frm_hold_cnt = 0;
	static unsigned char u8_RP_frm_hold_cnt_max = 0;
	static unsigned short RP_debounce_hold = 0;
	unsigned short u16_RP_frm_hold_thl = 120;
	static unsigned char u1_RP_detect_true_pre = 0;
	unsigned char u1_det_true;
	unsigned char u1_basic_cond_true;
	unsigned char u1_RepeatBG_true = 0;

	// 1.1 gmv
	unsigned char u1_gmv_true = 1;
	unsigned char u11_GMV_x_max=0;
	unsigned char u10_GMV_y_max=0;
	// 1.2 temporally consistency
	unsigned char u1_tc_true = 1;
	unsigned int u28_tc_th = 0x55730;
	unsigned int u27_mv_tempconsis=s_pContext->_output_read_comreg.u27_me_aTC_rb;
	// 1,3  rmv consistent
	unsigned char u1_rmv_consistent_true = 0;

	// 2.0 Dtl
	//unsigned int u26_aDTL_rb=s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	// 2.1 HV Dtl
	unsigned char u1_dtl_all_cond_true = 1;
	unsigned int u32_mc_HVDTL_thl = 0x200 ;
	unsigned short u16_mcHdtl = s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt;
	unsigned short u16_mcVdtl = s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt;

	// 3 glb PRD    // DRP affects the statistical value of PRD
	unsigned int u19_me_statis_glb_prd= s_pContext->_output_read_comreg.u19_me_statis_glb_prd;
	static unsigned int u19_avg_glb_prd=0;
	int diff_glb_prd =0;
	unsigned int u19_PRD_th0 = 0x76c;
	unsigned char u2_glb_PRD_status=0;

	//4. move check for Dtl, regDTL, regPRD
	unsigned char u1_mcDtl_move_true = 0,u1_rDTL_flag = 0,u1_rPRD_flag = 0,u1_rDTLPRD_move_cnt = 0, u1_det_DTLPRD_move_check =0;
	
	// 5.0 black_sc
	unsigned char u1_det_start=0;
	unsigned char u8_black_sc_cnt_th = 60;
	//======= record pre-
	static unsigned char u6_RP_frm_in_cnt_pre,u6_RP_frm_in_cnt_thl_pre,u8_RP_frm_hold_cnt_pre;
	static unsigned char u1_det_true_pre,u1_det_start_pre,u1_basic_cond_true_pre,u1_dtl_all_cond_true_pre,u2_glb_PRD_status_pre,u1_RepeatBG_true_pre;
	static unsigned int u27_me_aTC_rb_pre;
	static unsigned int u19_me_statis_glb_prd_pre;
	static unsigned short u16_mcHdtl_pre,u16_mcVdtl_pre;
	static unsigned char u11_GMV_x_max_pre,u10_GMV_y_max_pre;
	static unsigned int u17_me_ZMV_cnt_rb_pre;
	//===================
	//====rimRatio
	unsigned int u32_rimRatio = (s_pContext->_output_rimctrl.u32_rimRatio == 0) ? 128 : s_pContext->_output_rimctrl.u32_rimRatio;
	//==================
	//====DBG
	unsigned int u32_RB_val;
	unsigned int u32_RB_u28_tc_th,u32_RB_u32_mc_HVDTL_thl;
	unsigned int u32_RB_u19_PRD_th0;
	static signed short s16_rDtlPRD_RMV_th_cnt = 0;
	unsigned char u8_i = 0;
	static unsigned int u32_RP_detect_frame_cnt = 0;
	static unsigned int u32_RP_detect_chg_cnt = 0;
	static unsigned int u32_RP_detect_tmp[60] = {0};
	unsigned char u8_RP_detect_change_frequently_flag = 0;
	static unsigned char u8_RP_detect_change_frequently_frame_cnt = 0, K23424_flag = 0;
	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	unsigned int u32_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	unsigned int u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	unsigned int log_en = 0;

	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);//bit 25~31, 9~15
	ReadRegister(SOFTWARE1_SOFTWARE1_01_reg, 0, 31, &u32_RB_u28_tc_th);
	ReadRegister(SOFTWARE1_SOFTWARE1_02_reg, 0, 31, &u32_RB_u19_PRD_th0);
	ReadRegister(SOFTWARE1_SOFTWARE1_03_reg, 0, 31, &u32_RB_u32_mc_HVDTL_thl);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 0, 0, &log_en);
	
	// ===DBG APPLY
	u28_tc_th=u32_RB_u28_tc_th;
	u19_PRD_th0=u32_RB_u19_PRD_th0;
	u32_mc_HVDTL_thl=u32_RB_u32_mc_HVDTL_thl;
	//==========   Update thl
	//Update glb
	//u26_aDTL_rb = (u26_aDTL_rb * u32_rimRatio) >> 7;
	u28_tc_th = (u28_tc_th * u32_rimRatio) >> 7;
	u32_mc_HVDTL_thl = (u32_mc_HVDTL_thl * u32_rimRatio) >> 7;
	//Update rgn

	//===============   ipme flp   //  mc_HVdtl statis no change
	if(u1_RP_detect_true_pre  && u1_RP_detect_true_pre !=3)
	{
		u19_PRD_th0 = (u19_PRD_th0>>2)*3;
	}

	// gmv
	u1_gmv_true = MEMC_GMV_detect(&u11_GMV_x_max,&u10_GMV_y_max);

	//rmv
	u1_rmv_consistent_true = MEMC_RMV_Consistent_detect();

	//tc all
	u1_tc_true = u27_mv_tempconsis< u28_tc_th;
	//dtl all
	u1_dtl_all_cond_true = (u16_mcHdtl>u32_mc_HVDTL_thl)||(u16_mcVdtl>u32_mc_HVDTL_thl);
	//PRD all

	// for LG Internel NTSC ATSC DTV 33-1 test pattern condition (broken flicker) ---------- start
	diff_glb_prd =(u19_me_statis_glb_prd - u19_avg_glb_prd) ;
	if (_ABS_(diff_glb_prd) < u19_avg_glb_prd / 2)
		u19_avg_glb_prd = (u19_avg_glb_prd * 6 + u19_me_statis_glb_prd * 2 + 4) >>3;
	else
		u19_avg_glb_prd = (u19_avg_glb_prd * 62 + u19_me_statis_glb_prd * 2 + 32) >>6;
	u19_PRD_th0 = u19_PRD_th0 + 0x50;
	// for LG Internel NTSC ATSC DTV 33-1 test pattern condition (broken flicker) ---------- end

	// protection for still image 
	if(s_pContext->_output_frc_sceneAnalysis.u1_still_frame || s_pContext->_external_info.u1_VIP_Still){// add th for PRD and DTL th of still frame when still image 
		u19_PRD_th0 = u19_PRD_th0+2000;
		s16_rDtlPRD_RMV_th_cnt = 5;
	}

	if(s16_rDtlPRD_RMV_th_cnt>0){
		u19_PRD_th0 = u19_PRD_th0+2000;
		s16_rDtlPRD_RMV_th_cnt--;
	}

	if(s16_rDtlPRD_RMV_th_cnt<0) s16_rDtlPRD_RMV_th_cnt = 0;


	if(u19_avg_glb_prd >= u19_PRD_th0)
		u2_glb_PRD_status = 1;

	
	// move check
	u1_mcDtl_move_true = MEMC_HVDtl_Move_detect(pParam, pOutput);//current non-used , need to check more
	u1_rDTL_flag = MEMC_RGNDtl_detect(pParam, pOutput);//current non-used , need to check more
	u1_rPRD_flag = MEMC_RGNPRD_detect(pParam, pOutput);//current non-used , need to check more
	u1_rDTLPRD_move_cnt = MEMC_RGNDtlPRD_detect(pParam, pOutput);

	//   black_sc
	if(MEMC_BlackSC_detect()>=u8_black_sc_cnt_th || u2_glb_PRD_status) //quick start
		u1_det_start = 1;

	// BG HVdtl repeat
	u1_RepeatBG_true = MEMC_RepeatBG_detect();
	// Casino_RP  move to Identification
	//MEMC_Casino_RP_detect(pParam, pOutput);

	if(((u32_RB_val >>  31) & 0x01) ==1 && ((u32_RB_val >>  25) & 0x01) ==1)
		u1_gmv_true =0;
	else if(((u32_RB_val >>  25) & 0x01) ==1)
	{
		u1_gmv_true =1;
	}

	if(((u32_RB_val >>  31) & 0x01) ==1 && ((u32_RB_val >>  26) & 0x01) ==1)
		u1_tc_true =0;
	else if(((u32_RB_val >>  26) & 0x01) ==1)
	{
		u1_tc_true=1;
	}

	if(((u32_RB_val >>  31) & 0x01) ==1 && ((u32_RB_val >>  27) & 0x01) ==1)
		u1_rmv_consistent_true =1;
	else if(((u32_RB_val >>  27) & 0x01) ==1)
	{
		u1_rmv_consistent_true=0;
	}

	if(((u32_RB_val >>  31) & 0x01) ==1 && ((u32_RB_val >>  28) & 0x01) ==1)
		u1_dtl_all_cond_true =0;
	else if(((u32_RB_val >>  28) & 0x01) ==1)
	{
		u1_dtl_all_cond_true=1;
	}

	if(((u32_RB_val >>  31) & 0x01) ==1 && ((u32_RB_val >>  29) & 0x01) ==1)
		u2_glb_PRD_status =0;
	else if(((u32_RB_val >>  29) & 0x01) ==1)
	{
		u2_glb_PRD_status=1;
	}

	if(((u32_RB_val >>  31) & 0x01) ==1 && ((u32_RB_val >>  30) & 0x01) ==1){
		u1_RepeatBG_true =0;
	}else if(((u32_RB_val >>  30) & 0x01) ==1){
		u1_RepeatBG_true=1;
	}
#if 0
		if(((u32_RB_val >>	31) & 0x01) == 1){
			u1_mcDtl_move_true = 1;
			u1_rDTL_flag = 1;
			u1_rPRD_flag = 1;
			u1_rDTLPRD_move_cnt = 128;
		}
#endif
	// result
	u1_basic_cond_true = u1_gmv_true&&u1_tc_true&& (!u1_rmv_consistent_true);
	u1_det_true = (u1_det_start&&u1_basic_cond_true &&u1_dtl_all_cond_true&&u2_glb_PRD_status) ? 1 : 0;
	u1_det_true = u1_det_true|u1_RepeatBG_true;
	u1_det_DTLPRD_move_check = u1_rDTLPRD_move_cnt;

	ROSPrintf_MEMC_IP30_interface1("[%s,%d](%d,%d,%d,)(%d,%d,%d,%d,)(%d,%d,%d)",__FUNCTION__, __LINE__,
		u1_det_true,u1_RepeatBG_true,u1_det_DTLPRD_move_check,
		u1_det_start,u1_basic_cond_true,u1_dtl_all_cond_true,u2_glb_PRD_status,
		u1_gmv_true,u1_tc_true,(!u1_rmv_consistent_true));


	if(u1_det_true)
	{
		u6_RP_frm_in_cnt++;
		u6_RP_frm_in_cnt = (u6_RP_frm_in_cnt>8)? 8:u6_RP_frm_in_cnt;
	}
	else
	{
		u6_RP_frm_in_cnt = (u6_RP_frm_in_cnt == 0)? 1:u6_RP_frm_in_cnt;
		u6_RP_frm_in_cnt --;
	}

	if(u1_det_start == 0)		//black or sc, basic condition not match, quit
	{
		u8_RP_frm_hold_cnt = 0;
		u8_RP_frm_hold_cnt_max = 0;
	}
	else if(u6_RP_frm_in_cnt>=u6_RP_frm_in_cnt_thl)
	{
		u8_RP_frm_hold_cnt += FRAME_CHANGE_CNT;

		u8_RP_frm_hold_cnt = (u8_RP_frm_hold_cnt>u16_RP_frm_hold_thl *2)? u16_RP_frm_hold_thl*2:u8_RP_frm_hold_cnt;			//video end not detect protect

		u8_RP_frm_hold_cnt_max = u8_RP_frm_hold_cnt;
	}
	else if((u8_RP_frm_hold_cnt_max>=u16_RP_frm_hold_thl*1)&&(u8_RP_frm_hold_cnt>FRAME_CHANGE_CNT)&&u1_gmv_true&&u1_tc_true)
	{
		u8_RP_frm_hold_cnt -= FRAME_CHANGE_CNT;
	}
	else if((u8_RP_frm_hold_cnt_max>=u16_RP_frm_hold_thl*1)&&(u8_RP_frm_hold_cnt>FRAME_CHANGE_CNT*2))
	{
		u8_RP_frm_hold_cnt -= FRAME_CHANGE_CNT*2;
	}
	else
	{
		u8_RP_frm_hold_cnt = 0;
		u8_RP_frm_hold_cnt_max = 0;
	}

	if(u8_RP_frm_hold_cnt>0 && pOutput->u1_Natural_HighFreq_detect_true ==1)
	{
		pOutput->u1_RP_detect_true = 3;
		u6_RP_out_print = 1;
	}
	else if(u8_RP_frm_hold_cnt>0 && u1_RepeatBG_true )
	{
		pOutput->u1_RP_detect_true = 2;
		u6_RP_out_print = 1;
	}
	else if(u8_RP_frm_hold_cnt>0 )
	{
		if(u1_det_DTLPRD_move_check>=3){
			pOutput->u1_RP_detect_true = 1;
		}else if(u1_rDTL_flag == 1 && u1_rPRD_flag ==1){
			pOutput->u1_RP_detect_true = 0;
		}else{
			pOutput->u1_RP_detect_true = 4;
		}
		u6_RP_out_print = 1;
	}
	else
	{
		pOutput->u1_RP_detect_true = 0;
		if(u6_RP_out_print == 1)
			u6_RP_out_print =2;
	}

	//---K-23424
	if ((u32_gmv_cnt>28000) && ((_ABS_(u11_gmv_mvx) >= 0 && _ABS_(u11_gmv_mvx) <= 1)  || (_ABS_(u10_gmv_mvy) >= 0 && _ABS_(u10_gmv_mvy) <= 1))
		&& (u25_me_aDTL > 900000) && (u32_gmv_unconf<=1) && (scalerVIP_Get_MEMCPatternFlag_Identification(0,131) == TRUE))  // slow panning
	{
		K23424_flag = 60;
	}
	if(K23424_flag > 0){
		K23424_flag--;
		pOutput->u1_RP_detect_true = 4;
	}
	//---

	if(pOutput->u1_RP_detect_true >=1 ){
		pOutput->u1_RP_DTLPRD_move_check = u1_det_DTLPRD_move_check;
	}else{
		pOutput->u1_RP_DTLPRD_move_check = 0;
	}

	// protection for still image 
	if(u1_RP_detect_true_pre != pOutput->u1_RP_detect_true){
		u32_RP_detect_chg_cnt++;
	}

	for(u8_i = 0; u8_i < 59; u8_i++){
		u32_RP_detect_tmp[u8_i] = u32_RP_detect_tmp[u8_i+1];
	}
	u32_RP_detect_tmp[59] = u32_RP_detect_chg_cnt;

	if(u32_RP_detect_tmp[59] - u32_RP_detect_tmp[0] >= 26){
		u8_RP_detect_change_frequently_flag = 1;
		u32_RP_detect_chg_cnt = 0;
	}
	else if(u32_RP_detect_tmp[59] < u32_RP_detect_tmp[0]){
		for(u8_i = 0; u8_i < 60; u8_i++){
			u32_RP_detect_tmp[u8_i] = 0;
		}
	}

	if(u8_RP_detect_change_frequently_flag>0){
		u8_RP_detect_change_frequently_frame_cnt = 2;
	}

	if(u8_RP_detect_change_frequently_frame_cnt>0){
		if(u8_RP_detect_change_frequently_frame_cnt == 2){
			//WriteRegister(HARDWARE_HARDWARE_57_reg, 5, 5, 0);
		}
		else{
			//WriteRegister(HARDWARE_HARDWARE_57_reg, 5, 5, 1);
		}
		u8_RP_detect_change_frequently_frame_cnt--;
	}

	u32_RP_detect_frame_cnt++;
	if(u32_RP_detect_frame_cnt > 1000){
		u32_RP_detect_frame_cnt = 1;
	}
	if(u32_RP_detect_chg_cnt > 1000){
		u32_RP_detect_chg_cnt = 1;
	}

	//repeat debounce
	{
		static unsigned char u1_RP_detect_change_to = 0;
		if(((pOutput->u1_RP_detect_true != u1_RP_detect_true_pre) && (RP_debounce_hold == 0))||(pOutput->u1_RP_detect_true != u1_RP_detect_change_to))
		{
			u1_RP_detect_change_to = pOutput->u1_RP_detect_true;
			pOutput->u1_RP_detect_true = u1_RP_detect_true_pre;
			if(K23424_flag>0)
				RP_debounce_hold = 1;
			else
				RP_debounce_hold = 20;
		}
		else if(RP_debounce_hold > 1)
		{
			if(((u32_RB_val >>  31) & 0x01) == 1)
				rtd_pr_memc_emerg("RP_debounce_hold = %d, pOutput->u1_RP_detect_true = %d, u1_RP_detect_true_pre = %d\n",RP_debounce_hold,pOutput->u1_RP_detect_true,u1_RP_detect_true_pre);
			pOutput->u1_RP_detect_true = u1_RP_detect_true_pre;
			RP_debounce_hold --;
		}
		else if(RP_debounce_hold == 1)
		{
			if(((u32_RB_val >>  31) & 0x01) == 1)
				rtd_pr_memc_emerg("RP_debounce_hold = %d, pOutput->u1_RP_detect_true = %d, u1_RP_detect_true_pre = %d\n",RP_debounce_hold,pOutput->u1_RP_detect_true,u1_RP_detect_true_pre);
			RP_debounce_hold --;
		}
	}

	if(log_en){
		rtd_pr_memc_notice("[K23424_flag][%d][%d][u1_RP_detect_true,%d,]\n\r", K23424_flag, scalerVIP_Get_MEMCPatternFlag_Identification(0,131), pOutput->u1_RP_detect_true);
	}

	if(pParam->u1_RepeatPattern_en != 1)
		pOutput->u1_RP_detect_true = 0;


	if(((u32_RB_val >>  9) & 0x01) ==1)
		pOutput->u1_RP_detect_true = 0;
	if(((u32_RB_val >>  10) & 0x01) ==1)
		pOutput->u1_RP_detect_true = 1;
	if(((u32_RB_val >>  11) & 0x01) ==1)
		pOutput->u1_RP_detect_true = 2;
	if(((u32_RB_val >>  12) & 0x01) ==1)
		pOutput->u1_RP_detect_true = 3;
	if(((u32_RB_val >>  12) & 0x01) ==1 && ((u32_RB_val >>  9) & 0x01) ==1)
		pOutput->u1_RP_detect_true = 4;

	if ( (frm_cnt % 10 == 0  && ((u32_RB_val >>  13) & 0x01) ==1 ) || (frm_cnt % 1 == 0  && ((u32_RB_val >>  14) & 0x01) ==1  && (u6_RP_out_print ==2))){
		u6_RP_out_print =0;
		if(1){
		rtd_pr_memc_notice("[%s][%d][,%d,%d,][,%d,%d,%d,][,%d,%d,]\n\r",
			__FUNCTION__, __LINE__,
			pOutput->u1_RP_detect_true, u32_RP_detect_chg_cnt,
			u19_avg_glb_prd, u19_me_statis_glb_prd, u19_PRD_th0,
			u8_RP_frm_hold_cnt, u1_rDTLPRD_move_cnt);
		rtd_pr_memc_emerg("[%s][%d][%d  %d  %d ] [%d %d %d %d %d %d] [%d %d][%d %d %d %d %d][%d] [%d,%d,%d,%d][%d,%d,%d,%d,%d]\n",
			__FUNCTION__, __LINE__,
			u6_RP_frm_in_cnt,u6_RP_frm_in_cnt_thl,u8_RP_frm_hold_cnt,
			u1_det_true, u1_det_start, u1_basic_cond_true, u1_dtl_all_cond_true,u2_glb_PRD_status,u1_RepeatBG_true,
			u27_mv_tempconsis,u32_rimRatio,
			u19_me_statis_glb_prd,u16_mcHdtl ,u16_mcVdtl, u11_GMV_x_max, u10_GMV_y_max,
			s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb,
			u1_mcDtl_move_true,u1_rDTL_flag,u1_rPRD_flag,u1_rDTLPRD_move_cnt,
			s_pContext->_external_info.DTL_Step_Hist_sum_ratio[0],s_pContext->_external_info.DTL_Step_Hist_sum_ratio[1],s_pContext->_external_info.DTL_Step_Hist_sum_ratio[2],s_pContext->_external_info.DTL_Step_Hist_sum_ratio[3],s_pContext->_external_info.DTL_Step_Hist_sum_ratio[4]);
			//s_pContext->_output_read_comreg.u22_me_rTC_rb[31], s_pContext->_output_read_comreg.u22_me_rSC_rb[31], s_pContext->_output_read_comreg.u25_me_rSAD_rb[31]);
		}
		#if 1
		if(((u32_RB_val >>  31) & 0x01) == 1){
			rtd_pr_memc_emerg("\n\r$ [vx_rb][(,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,) $\n\r",
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1], 
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3], 
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5], 
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7], 
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9], 
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[10],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[13],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[20],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[21],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[22],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29],
				s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31] );
			rtd_pr_memc_emerg("\n\r$ [vy_rb][(,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,) $\n\r",
				s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[0],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[1],  
				s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[2],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[3],  
				s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[4],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[5], 
				s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[6],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[7],  
				s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[8],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9],  
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[10],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[13],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[15],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[17],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[18],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[20],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[21],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[22],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[23],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[24],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[25],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[26],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[27],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[28],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[29],
				 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[30],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[31] );
			rtd_pr_memc_emerg("\n\r$ [rDTL_rb][(,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,) $\n\r",
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[0],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[1],	
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[2],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[3],	
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[4],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[5],	
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[6],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[7],	
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[8],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[9],	
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[10],s_pContext->_output_read_comreg.u20_me_rDTL_rb[11],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[12],s_pContext->_output_read_comreg.u20_me_rDTL_rb[13],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[14],s_pContext->_output_read_comreg.u20_me_rDTL_rb[15],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[16],s_pContext->_output_read_comreg.u20_me_rDTL_rb[17],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[18],s_pContext->_output_read_comreg.u20_me_rDTL_rb[19],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[20],s_pContext->_output_read_comreg.u20_me_rDTL_rb[21],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[22],s_pContext->_output_read_comreg.u20_me_rDTL_rb[23],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[24],s_pContext->_output_read_comreg.u20_me_rDTL_rb[25],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[26],s_pContext->_output_read_comreg.u20_me_rDTL_rb[27],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[28],s_pContext->_output_read_comreg.u20_me_rDTL_rb[29],
				s_pContext->_output_read_comreg.u20_me_rDTL_rb[30],s_pContext->_output_read_comreg.u20_me_rDTL_rb[31] );
			rtd_pr_memc_emerg("\n\r$ [rPRD_rb][(,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,) $\n\r",
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[0],s_pContext->_output_read_comreg.u13_me_rPRD_rb[1],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[2],s_pContext->_output_read_comreg.u13_me_rPRD_rb[3],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[4],s_pContext->_output_read_comreg.u13_me_rPRD_rb[5],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[6],s_pContext->_output_read_comreg.u13_me_rPRD_rb[7],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[8],s_pContext->_output_read_comreg.u13_me_rPRD_rb[9],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[10],s_pContext->_output_read_comreg.u13_me_rPRD_rb[11],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[12],s_pContext->_output_read_comreg.u13_me_rPRD_rb[13],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[14],s_pContext->_output_read_comreg.u13_me_rPRD_rb[15],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[16],s_pContext->_output_read_comreg.u13_me_rPRD_rb[17],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[18],s_pContext->_output_read_comreg.u13_me_rPRD_rb[19],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[20],s_pContext->_output_read_comreg.u13_me_rPRD_rb[21],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[22],s_pContext->_output_read_comreg.u13_me_rPRD_rb[23],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[24],s_pContext->_output_read_comreg.u13_me_rPRD_rb[25],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[26],s_pContext->_output_read_comreg.u13_me_rPRD_rb[27],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[28],s_pContext->_output_read_comreg.u13_me_rPRD_rb[29],
				s_pContext->_output_read_comreg.u13_me_rPRD_rb[30],s_pContext->_output_read_comreg.u13_me_rPRD_rb[31]);
				
		}	

		#endif
	}
	else if( ((u32_RB_val >>  15) & 0x01) ==1 && u1_RP_detect_true_pre == 0 &&pOutput->u1_RP_detect_true ==1)
	{
		u6_RP_out_print =0;
		rtd_pr_memc_emerg("[%d  %d  %d ] [%d %d %d %d %d %d] [%d][%d %d %d %d %d][%d] [%d %d %d]\n",
			u6_RP_frm_in_cnt_pre,u6_RP_frm_in_cnt_thl_pre,u8_RP_frm_hold_cnt_pre,
			u1_det_true_pre, u1_det_start_pre, u1_basic_cond_true_pre, u1_dtl_all_cond_true_pre,u2_glb_PRD_status_pre,u1_RepeatBG_true_pre,
			u27_me_aTC_rb_pre,
			u19_me_statis_glb_prd_pre,u16_mcHdtl_pre,u16_mcVdtl_pre, u11_GMV_x_max_pre, u10_GMV_y_max_pre,
			u17_me_ZMV_cnt_rb_pre,
			u1_mcDtl_move_true,u1_rDTL_flag,u1_rPRD_flag);
	}
	//======= record pre-
	u1_RP_detect_true_pre = pOutput->u1_RP_detect_true;
	u6_RP_frm_in_cnt_pre = u6_RP_frm_in_cnt;
	u6_RP_frm_in_cnt_thl_pre = u6_RP_frm_in_cnt_thl;
	u8_RP_frm_hold_cnt_pre = u8_RP_frm_hold_cnt;
	u1_det_true_pre = u1_det_true;
	u1_det_start_pre=u1_det_start;
	u1_basic_cond_true_pre= u1_basic_cond_true;
	u1_dtl_all_cond_true_pre=u1_dtl_all_cond_true;
	u2_glb_PRD_status_pre=u2_glb_PRD_status;
	u1_RepeatBG_true_pre=u1_RepeatBG_true;
	u27_me_aTC_rb_pre = u27_mv_tempconsis;
	u19_me_statis_glb_prd_pre=u19_me_statis_glb_prd;
	u16_mcHdtl_pre=u16_mcHdtl;
	u16_mcVdtl_pre=u16_mcVdtl;
	u11_GMV_x_max_pre=u11_GMV_x_max;
	u10_GMV_y_max_pre=u10_GMV_y_max;
	u17_me_ZMV_cnt_rb_pre=s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb;
	//======= record pre-
	frm_cnt ++;
	if (frm_cnt >= 100000) frm_cnt = 0;
}
VOID MEMC_Region_Periodic_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	unsigned char u1_rDTL_move_true = 0,u1_rPRD_move_true = 0;
	static unsigned char u8_Region_Periodi_hold_cnt = 0;

	u1_rDTL_move_true = MEMC_RGNDtl_Move_detect(pParam, pOutput);
	u1_rPRD_move_true = MEMC_RGNPRD_Move_detect(pParam, pOutput);

	pOutput->u1_Region_Periodic_detect_true = u1_rPRD_move_true |(u1_rDTL_move_true<<1);


	if(pOutput->u1_Region_Periodic_detect_true == 3)
	{
		u8_Region_Periodi_hold_cnt =8;
	}
	else if(u8_Region_Periodi_hold_cnt >0)
	{
		pOutput->u1_Region_Periodic_detect_true=3;
		u8_Region_Periodi_hold_cnt--;
	}
}

extern unsigned int g_HisDiff;
extern unsigned int g_HueDiff;

VOID MEMC_Repeat_Periodic_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLCONTEXT *s_pContext_m = GetPQLContext_m();
	bool detect_repeat = pOutput->u1_RP_detect_true==0 ? false : true;
	bool detect_periodic = pOutput->u1_Region_Periodic_detect_true!=3 ? false : true;
	bool detect_panning = pOutput->u2_panning_flag!=1 ? false : true;
	bool detect_sc = (g_HisDiff > 100000) ? 1 : (g_HueDiff > 3200000) ? 1 : 0;
	bool detect_casino = pOutput->u1_casino_RP_detect_true == 0 ? false : true;		
	unsigned short max_mvx=0, max_mvy=0, max_rmv;
	unsigned int u8_i, sum_rmv_mvx=0, sum_rmv_mvy=0, avg_rmv, mv_th_1st, mv_th_2nd, mv_th_3rd, mv_th_4th, log_en, log_en2;
	static unsigned char chg_search_range_cnt = 0, sc_hold = 0,sad_hold=0;
	unsigned char Repeat_Mode = RepeatMode_MID, RepeatCase = RepeatCase_None;
	unsigned int ForceFlg, ForceLvl, cnt_th, sc_en, slowmv_en, sc_mv_th;
	unsigned char Repeat_Mode_Set = RepeatMode_MID;
	static unsigned char u8_cnt=0;
	static unsigned char Repeat_Mode_Pre=RepeatMode_MID;
	unsigned int u8_cnt_max1, u8_cnt_max2;
	bool detect_Still_Highfrequency_Image = pOutput->u1_still_highfrequency_image_detect;
	static unsigned int u30_sad_pre =0;
	bool sad_bigchange =0;
	
	if(!pParam->b_FixRepeat_en) { //d9fc[31]
		pOutput->u8_RepeatMode = RepeatMode_OFF;
		return;
	}

	ReadRegister(SOFTWARE3_SOFTWARE3_63_reg,0, 0, &ForceFlg);//D9FC
	ReadRegister(SOFTWARE3_SOFTWARE3_63_reg,1, 2, &ForceLvl);//D9FC
	if(ForceFlg) {//d9fc[2]
		ForceLvl = (ForceLvl>RepeatMode_HIGH) ? RepeatMode_HIGH : ForceLvl;
		pOutput->u8_RepeatMode = ForceLvl;
		s_pContext_m->_external_data.u8_repeatCase = RepeatCase;
		return;
	}
	
	ReadRegister(SOFTWARE3_SOFTWARE3_62_reg, 0,  7, &mv_th_1st); //D9F8
	ReadRegister(SOFTWARE3_SOFTWARE3_62_reg, 8, 15, &mv_th_2nd); //D9F8
	ReadRegister(SOFTWARE3_SOFTWARE3_62_reg,16, 23, &mv_th_3rd); //D9F8
	ReadRegister(SOFTWARE3_SOFTWARE3_62_reg,24, 31, &mv_th_4th); //D9F8
	ReadRegister(SOFTWARE3_SOFTWARE3_63_reg,30, 30, &log_en);	 //D9FC
	ReadRegister(SOFTWARE3_SOFTWARE3_63_reg, 8, 15, &cnt_th);	 //D9FC
	ReadRegister(SOFTWARE3_SOFTWARE3_63_reg,24, 24, &sc_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_63_reg,25, 25, &slowmv_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_63_reg,16, 23, &sc_mv_th);

	
	#if 1
		if(u30_sad_pre>100000 && s_pContext->_output_read_comreg.u30_me_aSAD_rb - u30_sad_pre >5000000){
			sad_bigchange=1;
		}else{
			sad_bigchange=0;
		}
		
	#endif 
#if 0
	//0 42 90 128
	if(s_pContext->_external_data.u8_dejudder_lvl<=30){ // MEMC OFF
		Repeat_lvl = RepeatMode_MID;//RepeatMode_HIGH
	}else if(s_pContext->_external_data.u8_dejudder_lvl<=60){ // MEMC LOW  //cinematic
		Repeat_lvl = RepeatMode_MID;//RepeatMode_HIGH
	}else if(s_pContext->_external_data.u8_dejudder_lvl<100){ // MEMC MID  //natrual
		Repeat_lvl = RepeatMode_MID;
	}else{	// MEMC HIGH  //vivid
		Repeat_lvl = RepeatMode_OFF;
	}
#endif
	for(u8_i=0;u8_i<32;u8_i++){
		if(_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i])>max_mvx) {
			max_mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i]);
		}
		if(_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i])>max_mvy) {
			max_mvy = _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i]);
		}

		sum_rmv_mvx += _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i]);
		sum_rmv_mvy += _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i]);
	}
	max_rmv = _MAX_(max_mvx, max_mvy);
	avg_rmv = (_MAX_(sum_rmv_mvx, sum_rmv_mvy))>>5;

	
	if(detect_casino && max_rmv<=60) {
		RepeatCase = RepeatCase_Casino;
		chg_search_range_cnt = chg_search_range_cnt >= 47 ? 50 : chg_search_range_cnt+3;
		sc_hold = 0;
	}
	else if(detect_sc && sc_en && max_rmv>sc_mv_th){
		RepeatCase = RepeatCase_SC;
		Repeat_Mode = RepeatMode_SC;
		chg_search_range_cnt = 0;
		sc_hold = 15;
	}
	else if(sc_hold > 0){
		RepeatCase = RepeatCase_SC;
		Repeat_Mode = RepeatMode_SC;
		chg_search_range_cnt = 0;
		sc_hold--;
	}
	else if(s_pContext->_output_me_sceneAnalysis.u1_SportScene) {
		RepeatCase = RepeatCase_Sport;
		Repeat_Mode = RepeatMode_OFF;
		chg_search_range_cnt = 0;
	}
	else if(max_mvx>80 || max_mvy>80) {
		RepeatCase=RepeatCase_None;
		Repeat_Mode=RepeatMode_OFF;
		chg_search_range_cnt = 0;
	}
	
	else if(sad_bigchange){
		RepeatCase = RepeatCase_BIGSAD;
		Repeat_Mode = RepeatMode_BIGSAD;
		chg_search_range_cnt = 0;
		sad_hold = 15;
	}
	else if(sad_hold > 0){
		RepeatCase = RepeatCase_BIGSAD;
		Repeat_Mode = RepeatMode_BIGSAD;
		chg_search_range_cnt = 0;
		sad_hold--;
	}
	else if(slowmv_en && max_rmv<=mv_th_4th){	
		RepeatCase = RepeatCase_VerySlowMV;
		chg_search_range_cnt = chg_search_range_cnt >= 47 ? 50 : chg_search_range_cnt+3;
	}
	else if((detect_repeat || detect_periodic || detect_panning) && detect_Still_Highfrequency_Image==0 && max_rmv<=mv_th_2nd){	
		RepeatCase = RepeatCase_DetRpAndSlowMV;
		chg_search_range_cnt = chg_search_range_cnt >= 47 ? 50 : chg_search_range_cnt+3;
	}
	else if((detect_repeat || detect_periodic) && detect_panning && max_rmv<=mv_th_1st){	
		RepeatCase = RepeatCase_DetRpPanAndSlowMV;
		chg_search_range_cnt = chg_search_range_cnt >= 47 ? 50 : chg_search_range_cnt+2;
	}
	else if(detect_panning && max_rmv<=mv_th_1st){	
		RepeatCase = RepeatCase_PanAndSlowMV;
		chg_search_range_cnt = chg_search_range_cnt >= 47 ? 50 : chg_search_range_cnt+1;
	}
	else if(slowmv_en && max_rmv<=mv_th_3rd){	
		RepeatCase = RepeatCase_SlowMV;
		chg_search_range_cnt = chg_search_range_cnt >= 47 ? 50 : chg_search_range_cnt+1;
	}
	else if(chg_search_range_cnt>2 && max_mvx<=20 && max_mvy<=20) {
		RepeatCase=RepeatCase_HoldOn_SlowMV;
		chg_search_range_cnt = chg_search_range_cnt <= 2 ? 0 : chg_search_range_cnt-2;
	}
	else if(chg_search_range_cnt>2 && max_mvx<=40 && max_mvy<=40 && slowmv_en) {
		RepeatCase=RepeatCase_HoldOn;
		Repeat_Mode=RepeatMode_LOW;
		chg_search_range_cnt = chg_search_range_cnt <= 2 ? 0 : chg_search_range_cnt-2;
	}
	else {
		RepeatCase=RepeatCase_None;
		Repeat_Mode = RepeatMode_OFF;

		if(max_mvx>80 || max_mvy>80) {
			chg_search_range_cnt = 0;
		}
		else if(detect_panning==false && (max_mvx>60 || max_mvx>60)){
			chg_search_range_cnt = 0;
		}
		else if(max_mvx>40 || max_mvx>40){
			chg_search_range_cnt = chg_search_range_cnt <= 5 ? 0 : chg_search_range_cnt-5;
		}
		else if(max_mvx>20 || max_mvx>20){
			chg_search_range_cnt = chg_search_range_cnt <= 3 ? 0 : chg_search_range_cnt-3;
		}
		else {
			chg_search_range_cnt = chg_search_range_cnt <= 2 ? 0 : chg_search_range_cnt-2;
		}
	}

	if(chg_search_range_cnt < cnt_th){
		Repeat_Mode = RepeatMode_OFF;
	}

// smoothly change the repeat mode  EX: set mid between off and mid, OFF->LOW->MID
#if 1
{
	ReadRegister(SOFTWARE3_SOFTWARE3_11_reg, 0,  5, &u8_cnt_max1);
	ReadRegister(SOFTWARE3_SOFTWARE3_11_reg, 8, 13, &u8_cnt_max2);
	ReadRegister(SOFTWARE3_SOFTWARE3_63_reg,29, 29, &log_en2);	 //D9FC
		
	Repeat_Mode_Set = Repeat_Mode;

	if(Repeat_Mode == RepeatMode_SC || Repeat_Mode == RepeatMode_BIGSAD) {
		u8_cnt=0;
		Repeat_Mode_Pre = RepeatMode_OFF;
		Repeat_Mode_Set = RepeatMode_OFF;
	}
	else {
		if(Repeat_Mode_Pre!=Repeat_Mode) {
			if(Repeat_Mode_Pre==RepeatMode_MID){
				u8_cnt = u8_cnt_max2;
				Repeat_Mode_Pre = Repeat_Mode;
			}
			else { //Repeat_Mode_Pre==RepeatMode_OFF
				u8_cnt = u8_cnt_max1;
				Repeat_Mode_Pre = Repeat_Mode;
			}
		}

		if(u8_cnt>0){
			u8_cnt--;
			Repeat_Mode_Set = RepeatMode_LOW;
		}
	}


	
	if(log_en2){//d9fc[29]
		rtd_pr_memc_emerg("[RepeatBroken Smooth] Pre:%d Det:%d Set:%d  cnt>>[%d,%d,%d] stillinmg[%d]\n",
			Repeat_Mode_Pre, Repeat_Mode, Repeat_Mode_Set,u8_cnt, u8_cnt_max1, u8_cnt_max2,detect_Still_Highfrequency_Image);
	}
}	
#endif

	pOutput->u8_RepeatMode = Repeat_Mode_Set;
	s_pContext_m->_external_data.u8_repeatCase = RepeatCase;
	u30_sad_pre = s_pContext->_output_read_comreg.u30_me_aSAD_rb;
	if(log_en) { //d9fc[30]
		rtd_pr_memc_emerg("[RepeatBroken] CASE>>[%d-%d] DET>>[%d,%d,%d,%d] RMV>>[%d,%d] HOLD>>[%d]	SC>>[%d,%d] CASINO>>[%d]\n", 
			RepeatCase, Repeat_Mode_Set, detect_periodic, detect_repeat, detect_panning, detect_sc, 
			avg_rmv, max_rmv, chg_search_range_cnt, 
			g_HisDiff, g_HueDiff, pOutput->u1_casino_RP_detect_true);
	}
}

VOID MEMC_MAD_detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned short max_mvx=0, max_mvy=0;
	int avg_rmv_x = 0, avg_rmv_y = 0;
	unsigned int u8_i=0, u8_MAD_x=0, u8_MAD_y=0, sum_abs_rmv_mvx=0, sum_abs_rmv_mvy=0, log_en;
	static unsigned char u8_cnt=0;
	
	for(u8_i=0;u8_i<32;u8_i++){
		if(_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i])>max_mvx) {
			max_mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i]);
		}
		if(_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i])>max_mvy) {
			max_mvy = _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i]);
		}

		sum_abs_rmv_mvx += _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i]);
		sum_abs_rmv_mvy += _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i]);
		avg_rmv_x += s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i];
		avg_rmv_y += s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i];
	}
	// calculate the average region mv
	//max_rmv = _MAX_(max_mvx, max_mvy);
	//avg_abs_rmv = (_MAX_(sum_abs_rmv_mvx, sum_abs_rmv_mvy))>>5;
	avg_rmv_x = avg_rmv_x>>5;
	avg_rmv_y = avg_rmv_y>>5;

	// calculate the mean absolute deviation
	for(u8_i=0;u8_i<32;u8_i++){
		u8_MAD_x += _ABS_DIFF_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i], avg_rmv_x);
		u8_MAD_y += _ABS_DIFF_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i], avg_rmv_y);
	}
	u8_MAD_x = u8_MAD_x>>5;
	u8_MAD_y = u8_MAD_y>>5;
	if(u8_MAD_x<=1+_ABS_(avg_rmv_x>>4) && u8_MAD_y<=1+_ABS_(avg_rmv_y>>4) && (sum_abs_rmv_mvx>0 || sum_abs_rmv_mvy>0)){
		u8_cnt=10;
		pOutput->u1_uniform_panning = 1;
	}
	else if(u8_cnt>0){
		u8_cnt--;
		pOutput->u1_uniform_panning = 1;
	}
	else{
		pOutput->u1_uniform_panning = 0;
	}

	pOutput->u8_MAD_x = _CLIP_(u8_MAD_x, 0, 255);
	pOutput->u8_MAD_y = _CLIP_(u8_MAD_y, 0, 255);
	pOutput->s11_avg_rmv_x = avg_rmv_x;
	pOutput->s10_avg_rmv_y = avg_rmv_y;

	ReadRegister(SOFTWARE3_SOFTWARE3_10_reg, 7, 7, &log_en);
	if(log_en){ //d928[7]
		rtd_pr_memc_emerg("[%s][%d] uniform_pan>>[%d] MAD>>[%d,%d]	sum>>[%d,%d]	avg>>[%d,%d]	det>>[%d,%d]\n",
				__FUNCTION__, __LINE__, pOutput->u1_uniform_panning,
				u8_MAD_x, u8_MAD_y, sum_abs_rmv_mvx, sum_abs_rmv_mvy, avg_rmv_x, avg_rmv_y,
				(u8_MAD_x<=1+_ABS_(avg_rmv_x>>4)) ? 1 : 0,
				(u8_MAD_y<=1+_ABS_(avg_rmv_y>>4)) ? 1 : 0);
	}
}

VOID MEMC_Panning_Detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	unsigned int u32_i = 0;
	unsigned int u32_cnt = 0;
	unsigned int u32_rb_val = 0;

	const _PQLCONTEXT *s_pContext = GetPQLContext();

	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	unsigned int u32_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	unsigned int u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;

	unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	static unsigned int u32_gmv_cnt_pre = 0;
	static unsigned int u25_me_aDTL_pre = 0;
	static unsigned int u26_avgAPLi_pre = 0;
	unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned char u1_gmv_mode = 0;
	static int panning_cnt = 0;
	unsigned char panning_mode = 0;
	static unsigned char panning_mode_pre = 0;

	//pOutput->u32_new_saddiff_th =  s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_me1_sc_saddiff_th;
	u32_gmv_cnt = (u32_gmv_cnt_pre*7 + u32_gmv_cnt)>>3;
	u25_me_aDTL = (u25_me_aDTL_pre*7 + u25_me_aDTL)>>3;
	u26_aAPLi_rb=(u26_avgAPLi_pre*7 + u26_aAPLi_rb)>>3;

	// gmv mode
	ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_1C_reg, 2, 2, &u32_rb_val);
	u1_gmv_mode = u32_rb_val;

	if(u1_gmv_mode == 0)
	{
		if ((u32_gmv_cnt>28) && (_ABS_(u11_gmv_mvx) >= 4  || _ABS_(u10_gmv_mvy) >= 4)&& (u25_me_aDTL > 1200000))
		{
			// pOutput->u2_panning_flag = 1;
			panning_mode = 1;
			if (panning_cnt < 32) panning_cnt++;
		}
		else
		{
			// pOutput->u2_panning_flag = 0;
			if (panning_cnt > 0) panning_cnt--;
		}
	}
	else
	{
		if ((u32_gmv_cnt>30000) && ((_ABS_(u11_gmv_mvx) >= 1 && _ABS_(u11_gmv_mvx) <= 3)  || (_ABS_(u10_gmv_mvy) >= 1 && _ABS_(u10_gmv_mvy) <= 3))&& (u25_me_aDTL > 2000000)&& (u32_gmv_unconf<2))  // slow panning
		{
			// pOutput->u2_panning_flag = 2;
			panning_mode = 2;
			if (panning_cnt < 32) panning_cnt++;
		}
		else if ((u32_gmv_cnt>15000) && (_ABS_(u11_gmv_mvx) >= 4 || _ABS_(u10_gmv_mvy) >= 4) && (u25_me_aDTL > 550000) && (u32_gmv_unconf<0x6))  // pure panning
		{
			// pOutput->u2_panning_flag = 1;
			panning_mode = 1;
			if (panning_cnt < 32) panning_cnt++;
		}
		else if ((u32_gmv_cnt>12000) && (_ABS_(u11_gmv_mvx) >= 4 || _ABS_(u10_gmv_mvy) >= 4) && (u25_me_aDTL > 550000) && (u32_gmv_unconf<0xc) && u26_aAPLi_rb>3000000)  // pure panning
		{
			// pOutput->u2_panning_flag = 1;
			panning_mode = 1;
			if (panning_cnt < 32) panning_cnt++;
		}
		else if ((u32_gmv_cnt>30000) && (_ABS_(u11_gmv_mvx) >= 2 || _ABS_(u10_gmv_mvy) >= 2) && (u25_me_aDTL > 800000) && (u32_gmv_unconf<0x6))  //  pure panning
		{
			// pOutput->u2_panning_flag = 1;
			panning_mode = 1;
			if (panning_cnt < 32) panning_cnt++;
		}
		else if ((u32_gmv_cnt>10000) &&   (_ABS_(u11_gmv_mvx) >= 4 || _ABS_(u10_gmv_mvy) >= 4) && (u25_me_aDTL > 2100000 && u25_me_aDTL < 3000000)&& (u32_gmv_unconf<0x7)&& ( u26_aAPLi_rb>3000000 && u26_aAPLi_rb<4000000)) //   pure panning
		{
			// pOutput->u2_panning_flag = 1;
			panning_mode = 1;
			if (panning_cnt < 32) panning_cnt++;
		}
		else if ((u32_gmv_cnt>10000) &&   (_ABS_(u11_gmv_mvx) >= 4 || _ABS_(u10_gmv_mvy) >= 4) && (u25_me_aDTL > 4100000))  //   pure panning
		{
			for (u32_i = 0; u32_i < 32; u32_i ++)
			{
				if( s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[u32_i] < 2 &&
					_ABS_DIFF_(u11_gmv_mvx, ( s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u32_i]>>2) ) < 2 &&
					_ABS_DIFF_(u10_gmv_mvy, ( s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u32_i]>>2) ) < 2 &&
					( s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u32_i]!=0 || s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u32_i] !=0))
				{
					u32_cnt++;
				}
			}
			if(u32_cnt == 32)
			{
				//  pq sharpness&rtnr&mcnr effect consistent
				// pOutput->u2_panning_flag = 1;
				panning_mode = 1;
				if (panning_cnt < 32) panning_cnt++;
			}
		}
		else if ((u32_gmv_cnt>20000) && (_ABS_(u11_gmv_mvx) >= 4  || _ABS_(u10_gmv_mvy) >= 4)&& (u25_me_aDTL > 550000)&& (u32_gmv_unconf < 0xa))  // revert to k5lp and retunning dtl for normal panning
		{
			// pOutput->u2_panning_flag = 3;
			panning_mode = 3;
			if (panning_cnt < 32) panning_cnt++;
		}
		else if ((u32_gmv_cnt>23000) && (_ABS_(u11_gmv_mvx) >= 4  || _ABS_(u10_gmv_mvy) >= 4)&& (u25_me_aDTL > 800000)&& (u32_gmv_unconf < 0x16) && u26_aAPLi_rb>6000000)  // revert to k5lp and retunning dtl for normal panning
		{
			// pOutput->u2_panning_flag = 3;
			panning_mode = 3;
			if (panning_cnt < 32) panning_cnt++;
		}
		else
		{
			// pOutput->u2_panning_flag = 0;
			if (panning_cnt > 0) panning_cnt--;
		}
	}

	if (panning_cnt > 15)
		pOutput->u2_panning_flag = panning_mode;
	else if (panning_cnt < 5)
		pOutput->u2_panning_flag = 0;

	if (panning_cnt > 20 && s_pContext->_output_filmDetectctrl.u1_match_flag==0 && 
		s_pContext->_output_filmDetectctrl.u8_cur_cadence_Id[_FILM_ALL]==_CAD_11i23 &&
		pOutput->u2_panning_flag == 0 && panning_mode_pre != 0){
		pOutput->u2_panning_flag = panning_mode_pre;
		if((u32_rb_val>>31)&0x1)//d5e8
			rtd_pr_memc_emerg("Hold panning flag in CAD_11i23 panning_cnt:%d\n", panning_cnt);
	}

	
	// force value
	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_rb_val);//bit 16, 31
	switch( (u32_rb_val >> 16) & 0x3 )
	{
		case 0x0:
			break;
		case 0x1:
			pOutput->u2_panning_flag = 1;
			break;
		case 0x2:
			pOutput->u2_panning_flag = 0;
			break;
		case 0x3:
			pOutput->u2_panning_flag = 3;
			break;
		default:
			break;
	}
	
	pOutput->u2_panning_direction = (pOutput->u2_panning_flag==0) ? Pan_Dir_None : 
									(_ABS_(u11_gmv_mvx)<4 && _ABS_(u10_gmv_mvy)<4) ? Pan_Dir_None : 
									(_ABS_(u11_gmv_mvx)>_ABS_(u10_gmv_mvy)) ? Pan_Dir_X : Pan_Dir_Y;
	if((u32_rb_val>>31)&0x1)
	{
		if(/*(pOutput->u2_panning_flag==0) && */(frm_cnt % 10 == 0)){
			rtd_pr_memc_emerg("[MEMC][panning](%d, %d),cnt(%d), unconf(%d), DTL(%d), APL(%d), pan_flag(%d), pan_dir(%d)]\n",u11_gmv_mvx,u10_gmv_mvy,u32_gmv_cnt,u32_gmv_unconf,u25_me_aDTL,u26_aAPLi_rb,pOutput->u2_panning_flag,pOutput->u2_panning_direction);
		}
	}
	// update record
	u32_gmv_cnt_pre = u32_gmv_cnt;
	u25_me_aDTL_pre = u25_me_aDTL;
	u26_avgAPLi_pre = u26_aAPLi_rb;

	panning_mode_pre = pOutput->u2_panning_flag;
}

VOID MEMC_LightSwordDetect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_OUTPUT_RESOLUTION output_Resolution = s_pContext->_external_data._output_resolution;
	//const _Param_By_Resolution_RimCtrl *s_pRimParam = GetRimParam(output_Resolution);
	unsigned char  chaos_true = 0, chaos_apply_num = 10;

	// gmv
	unsigned char  gmv_iir = 50;
	int       me_GMV_1st_vy_rb_iir = 0;
	unsigned char me_GMV_1st_cnt_rb_iir  = 0;
	unsigned int me_GMV_1st_unconf_rb_iir = 0;
	unsigned char gmv_true = 0;

	// reiong gmv
	unsigned char rg_idx, rg_cnt = 0, rmv_diff_th = 0x46, rg_cnt_th = 5, rmv_true = 0;

	// rim
	unsigned char rim_delta_H = 5*MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.u8_scale_H; // 4k:20
	unsigned char rim_delta_Up = 10*MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.u8_scale_V; // 4k:40
	unsigned char rim_delta_Dn = 15*MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.u8_scale_V; // 4k:60
	unsigned int rimLft_th = 0;
	unsigned int rimUp_th = 60*MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.u8_scale_V; // 4k:240
	unsigned int rimLft = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT];
	unsigned int rimRht = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_RHT];
	unsigned int rimUp = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP];
	unsigned int rimDn = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT];
	unsigned int rimRgt_th = MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.s16_resolution_width;
	unsigned int rimDn_th = MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.s16_resolution_height;
	unsigned char rim_true = 1;

	// apl
	unsigned char apl_true, apl_true_con0, apl_true_con1, apl_true_con2;
	unsigned int apl_th =  0xFF00, apl_cnt_th = 3;
	unsigned int max_rgn_apl, min_rgn_apl, rgn_apl,  apl_cnt;

	me_GMV_1st_vy_rb_iir = (gmv_iir * (pOutput->s10_me_GMV_1st_vy_rb_iir) + (256-gmv_iir) * s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb) >> 8;
	me_GMV_1st_cnt_rb_iir  = (gmv_iir * (pOutput->u6_me_GMV_1st_cnt_rb_iir) + (256-gmv_iir) * s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb) >> 8;
	me_GMV_1st_unconf_rb_iir = (gmv_iir * (pOutput->u12_me_GMV_1st_unconf_rb_iir) + (256-gmv_iir) * s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb) >> 8;
	if( _ABS_(me_GMV_1st_vy_rb_iir) < 6 && me_GMV_1st_cnt_rb_iir > 30 && me_GMV_1st_unconf_rb_iir < 50)
	{
		gmv_true = 1;
	}

	for(rg_idx = 8; rg_idx < 24; rg_idx++)
	{
		int vx_1st, vy_1st, cnt_1st;
		int vx_2nd, vy_2nd, cnt_2nd;
		vx_1st = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[rg_idx];
		vy_1st = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[rg_idx];
		cnt_1st = s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[rg_idx];
		vx_2nd = s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[rg_idx];
		vy_2nd = s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[rg_idx];
		cnt_2nd = s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[rg_idx];

		if((_ABS_DIFF_(vx_1st , vx_2nd) + _ABS_DIFF_(vy_1st , vy_2nd) > rmv_diff_th) && (cnt_1st != 0 && cnt_2nd != 0))
			rg_cnt ++;
	}

	rmv_true = (rg_cnt > rg_cnt_th) ? 1 : 0;

	// rim
	if ((rimLft > rimLft_th + rim_delta_H) || \
	    (rimRht < rimRgt_th -rim_delta_H) || \
	    (rimUp < rimUp_th - rim_delta_Up  || rimUp > rimUp_th + rim_delta_Up) || \
	    (rimDn < rimDn_th - rim_delta_Dn || rimDn > rimDn_th + rim_delta_Dn))
	{
		rim_true = 0;
	}

	// apl
	apl_true = apl_true_con0 = apl_true_con1 = apl_true_con2 = 0;

	apl_cnt= 0;
	if(s_pContext->_output_read_comreg.u20_me_rAPLp_rb[8] < apl_th)
	{
		apl_cnt++;
	}
	if(s_pContext->_output_read_comreg.u20_me_rAPLp_rb[15] < apl_th)
	{
		apl_cnt++;
	}
	if(s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16] < apl_th)
	{
		apl_cnt++;
	}
	if(s_pContext->_output_read_comreg.u20_me_rAPLp_rb[23] < apl_th)
	{
		apl_cnt++;
	}
	if(apl_cnt >= apl_cnt_th)
	{
		apl_true_con0 = 1;
	}

	max_rgn_apl = min_rgn_apl = s_pContext->_output_read_comreg.u20_me_rAPLp_rb[8];
	for(rg_idx = 9; rg_idx < 16; rg_idx++)
	{
		rgn_apl = s_pContext->_output_read_comreg.u20_me_rAPLp_rb[rg_idx];
		if(rgn_apl > max_rgn_apl)
		{
			max_rgn_apl = rgn_apl;
		}
		if(rgn_apl < min_rgn_apl)
		{
			min_rgn_apl = rgn_apl;
		}
	}
	if(max_rgn_apl > (min_rgn_apl * 3 / 2))
	{
		apl_true_con1 = 1;
	}

	max_rgn_apl = min_rgn_apl = s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16];
	for(rg_idx = 17; rg_idx < 24; rg_idx++)
	{
		rgn_apl = s_pContext->_output_read_comreg.u20_me_rAPLp_rb[rg_idx];
		if(rgn_apl > max_rgn_apl)
		{
			max_rgn_apl = rgn_apl;
		}
		if(rgn_apl < min_rgn_apl)
		{
			min_rgn_apl = rgn_apl;
		}
	}
	if(max_rgn_apl > (min_rgn_apl * 3 / 2))
	{
		apl_true_con2 = 1;
	}

	apl_true = ((apl_true_con0 == 1) && ((apl_true_con1 == 1) || (apl_true_con2 == 1))) ? 1 : 0;


	chaos_true = rim_true && rmv_true && gmv_true && apl_true && pParam->u1_LightSword_en;

	// detection
	if(chaos_true)
	{
		pOutput->u8_chaos_apply = chaos_apply_num;
	}
	else if(pOutput->u8_chaos_apply > 0)
	{
		pOutput->u8_chaos_apply = pOutput->u8_chaos_apply - 1;
	}
	else
	{
		pOutput->u8_chaos_apply = 0;
	}

	pOutput->u1_chaos_true = chaos_true;
	pOutput->s10_me_GMV_1st_vy_rb_iir = me_GMV_1st_vy_rb_iir;
	pOutput->u6_me_GMV_1st_cnt_rb_iir = me_GMV_1st_cnt_rb_iir;
	pOutput->u12_me_GMV_1st_unconf_rb_iir = me_GMV_1st_unconf_rb_iir;

/*
	// dump
	unsigned int u32_RB_val;
	ReadRegister(0x000040A8, 25, 25, &u32_RB_val);
	if(u32_RB_val)
	{
		rtd_pr_memc_info("i:(%d,%d,%d,%d,%d,%d,%d,%d); (%d,%d,%d,%d,%d,%d,%d,%d)\n",
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[9],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[10],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[11],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12],
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[14],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[15],
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[16],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[18],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[19],
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[20],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[21],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[22],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[23]);
		rtd_pr_memc_info("p:(%d,%d,%d,%d,%d,%d,%d,%d); (%d,%d,%d,%d,%d,%d,%d,%d)\n",
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[9],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[10],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[11],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12],
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[14],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[15],
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[18],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[19],
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[20],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[21],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[22],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[23]);


		//rtd_pr_memc_info("rim:(%d,%d,%d,%d)\n", s_pContext->_output_rimctrl.u12_Rim_Result[0], s_pContext->_output_rimctrl.u12_Rim_Result[1], s_pContext->_output_rimctrl.u12_Rim_Result[2], s_pContext->_output_rimctrl.u12_Rim_Result[3]);
		rtd_pr_memc_info("rim=%d:(%d,%d,%d,%d), (%d,%d,(%d,%d),(%d,%d))\n", rim_true, rimLft, rimRht, rimUp, rimDn, rimLft_th + rim_delta_H, rimRgt_th -rim_delta_H, rimUp_th - rim_delta_V, rimUp_th + rim_delta_V, rimDn_th - rim_delta_V, rimDn_th + rim_delta_V);
	}
*/
}

VOID MEMC_Big_APLDiff_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	_PQLPARAMETER *s_pParam       = GetPQLParameter();
	unsigned char  u8_cadence_Id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	PQL_INPUT_FRAME_RATE in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned char u8_cand_cond_hold_cnt = 0;

	unsigned int apl_diff, big_apldiff_th, big_apldiff_rgn_cnt;//, min_rgn_apl;  // unused
	unsigned int i;//, j;  // unused
	unsigned int ratio = 107;//96;//128;
	unsigned int big_apldiff_rgn_cnt_th = 18;

	//unsigned short rim_lft, rim_rht, rim_top, rim_bot, width, height;  // unused
	//unsigned short h_sta_rgn, h_end_rgn, v_sta_rgn, v_end_rgn;  // unused

	signed short gmv0_x, gmv0_y, gmv1_x, gmv1_y;
	unsigned char gmv0_cnt, gmv1_cnt;
	unsigned short gmv0_ucf, gmv1_ucf;
	unsigned int gmv_mv_th = 0x80;//0xA0;
	unsigned char gmv_cnt_th = 0x10;
	unsigned short gmv_ucf_th = 0x10;

	unsigned int glb_sad;
	unsigned int glb_sad_th_h = 0xE00000;
	unsigned int glb_sad_th_l = 0x20;

	unsigned char  u1_big_apldiff_true, u1_big_motion_true, u1_pan_motion_true, u1_sad_zero2big;

	if (u8_cadence_Id == _CAD_22 || (in_fmRate == _PQL_IN_25HZ && u8_cadence_Id == _CAD_VIDEO))
	{
		u8_cand_cond_hold_cnt = 1;
	}
	else
	{
		u8_cand_cond_hold_cnt = 2;
	}

	pOutput->u32_big_apldiff_saddiff_th = s_pParam->_param_frc_sceneAnalysis.u28_dyn_SAD_Diff_sadAll_th_l;

	if(pParam->u1_BigAPLDiff_en)
	{
		pOutput->u32_big_apldiff_saddiff_th = 0x4000;//0xF000;
#if 0
		// find min available rgn_apl
		rim_lft = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT];
	 	rim_rht = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_RHT];
		rim_top = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP];
		rim_bot = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT];
		width = s_pContext->_output_rimctrl.u12_out_resolution[_RIM_RHT] - s_pContext->_output_rimctrl.u12_out_resolution[_RIM_LFT];
		height =  s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT] - s_pContext->_output_rimctrl.u12_out_resolution[_RIM_TOP];
		min_rgn_apl = 0xFFFFF;
		width = (width == 0) ? 1 : width;
		height = (height == 0) ? 1 : height;
		h_sta_rgn = (2 * 8 * rim_lft / width + 1) >> 1;
		h_end_rgn = (2 * 8 * rim_rht / width - 1) >> 1;
		v_sta_rgn = (2 * 4 * rim_top / height + 1) >> 1;
		v_end_rgn = (2 * 4 * rim_bot / height - 1) >> 1;

		for(i = v_sta_rgn; i < v_end_rgn; i++)
		{
			for(j = h_sta_rgn; j < h_end_rgn; j++)
			{
				if(s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i * 8 + j] < min_rgn_apl)
				{
					min_rgn_apl = s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i * 8 + j];
				}
				if(s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i * 8 + j] < min_rgn_apl)
				{
					min_rgn_apl = s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i * 8 + j];
				}
			}
		}

		// calc rgn apl th
		//big_apldiff_th = 0x4000;
		big_apldiff_th = min_rgn_apl * ratio >> 8;

		// calc big apldiff rgn cnt
		big_apldiff_rgn_cnt = 0;
		for(i = 0; i < 32; i++)
		{
			if(s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i] > s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i])
			{
				apl_diff = s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i] - s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i];
			}
			else
			{
				apl_diff = s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i] - s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i];
			}
			if(apl_diff >= big_apldiff_th)
			{
				big_apldiff_rgn_cnt++;
			}
		}
#endif

#if 1
	ratio = 0x28;
	{
		// calc big apldiff rgn cnt
		big_apldiff_rgn_cnt = 0;
		for(i = 0; i < 32; i++)
		{
			if(s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i] > s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i])
			{
				apl_diff = s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i] - s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i];
				big_apldiff_th = (s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i] * ratio) >> 8;
			}
			else
			{
				apl_diff = s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i] - s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i];
				big_apldiff_th = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i] * ratio) >> 8;
			}
			if(apl_diff >= big_apldiff_th)
			{
				big_apldiff_rgn_cnt++;
			}
		}
	}
#endif

		if(big_apldiff_rgn_cnt >= big_apldiff_rgn_cnt_th)
		{
			u1_big_apldiff_true = 1;
		}
		else
		{
			u1_big_apldiff_true = 0;
		}

		// big motion
		gmv0_x = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
		gmv0_y = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
		gmv0_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
		gmv0_ucf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;

		gmv1_x = s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb;
		gmv1_y = s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb;
		gmv1_cnt = s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb;
		gmv1_ucf = s_pContext->_output_read_comreg.u12_me_GMV_2nd_unconf_rb;

		u1_big_motion_true = 0;
		if((_ABS_(gmv0_x) >= gmv_mv_th || _ABS_(gmv0_y) >= gmv_mv_th)
			&& (gmv0_cnt >= gmv_cnt_th) && (gmv0_ucf <= gmv_ucf_th))
		{
			u1_big_motion_true = 1;
		}

		if((_ABS_(gmv1_x) >= gmv_mv_th || _ABS_(gmv1_y) >= gmv_mv_th)
			&& (gmv1_cnt >= gmv_cnt_th) && (gmv1_ucf <= gmv_ucf_th))
		{
			u1_big_motion_true = 1;
		}

		// pan
		gmv_mv_th = 0x3C;
	 	gmv_cnt_th = 0x1E;
		gmv_ucf_th = 0x5;

		u1_pan_motion_true = 0;
		if((_ABS_(gmv0_x) >= gmv_mv_th || _ABS_(gmv0_y) >= gmv_mv_th)
			&& (gmv0_cnt >= gmv_cnt_th) && (gmv0_ucf <= gmv_ucf_th))
		{
			u1_pan_motion_true = 1;
		}

		if((_ABS_(gmv1_x) >= gmv_mv_th || _ABS_(gmv1_y) >= gmv_mv_th)
			&& (gmv1_cnt >= gmv_cnt_th) && (gmv1_ucf <= gmv_ucf_th))
		{
			u1_pan_motion_true = 1;
		}

		// change from still
		glb_sad = s_pContext->_output_read_comreg.u30_me_aSAD_rb;
		if((pOutput->u32_big_apldiff_sad_pre < glb_sad_th_l) && (glb_sad > glb_sad_th_h))
		{
			u1_sad_zero2big = 1;
		}
		else
		{
			u1_sad_zero2big = 0;
		}
		pOutput->u32_big_apldiff_sad_pre = glb_sad;

		if((u1_big_apldiff_true == 1) && (u1_big_motion_true == 0) && (u1_pan_motion_true == 0))
		{
			pOutput->u8_big_apldiff_hold_cnt = 5;
			if(u1_sad_zero2big == 1)
			{
				pOutput->u8_big_apldiff_sc_hold_cnt = 4;// 1;
			}
		}
		else if(pOutput->u8_big_apldiff_hold_cnt != 0)
		{
			pOutput->u8_big_apldiff_hold_cnt--;
			pOutput->u8_big_apldiff_sc_hold_cnt = u8_cand_cond_hold_cnt;
		}
		else
		{
			pOutput->u8_big_apldiff_hold_cnt = 0;
			pOutput->u8_big_apldiff_sc_hold_cnt = u8_cand_cond_hold_cnt;
		}

	}
	else
	{
		pOutput->u8_big_apldiff_hold_cnt = 0;
		pOutput->u8_big_apldiff_sc_hold_cnt = u8_cand_cond_hold_cnt;
	}
#if 0
	// dump
	unsigned int u32_RB_val;
	ReadRegister(HARDWARE_HARDWARE_57_reg, 12, 12, &u32_RB_val);
	if(u32_RB_val)
	{
	/*
		rtd_pr_memc_info("i:(%d,%d,%d,%d,%d,%d,%d,%d); (%d,%d,%d,%d,%d,%d,%d,%d)\n",
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[9],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[10],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[11],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12],
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[14],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[15],
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[16],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[18],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[19],
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[20],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[21],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[22],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[23]);
		rtd_pr_memc_info("p:(%d,%d,%d,%d,%d,%d,%d,%d); (%d,%d,%d,%d,%d,%d,%d,%d)\n",
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[9],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[10],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[11],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12],
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[14],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[15],
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[18],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[19],
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[20],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[21],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[22],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[23]);
	*/

		//rtd_pr_memc_info("rim:(%d,%d,%d,%d)\n", s_pContext->_output_rimctrl.u12_Rim_Result[0], s_pContext->_output_rimctrl.u12_Rim_Result[1], s_pContext->_output_rimctrl.u12_Rim_Result[2], s_pContext->_output_rimctrl.u12_Rim_Result[3]);
		rtd_pr_memc_info("GMV0:(%d, %d, %d, %d); GMV1:(%d, %d, %d, %d)\n", gmv0_x, gmv0_y, gmv0_cnt, gmv0_ucf,  gmv1_x, gmv1_y, gmv1_cnt, gmv1_ucf);
		rtd_pr_memc_info("rgn_cnt:%d, saddiff:%d\n", big_apldiff_rgn_cnt, s_pContext->_output_read_comreg.u30_me_aSAD_rb);
	}
#endif

}

VOID MEMC_EPG_Protect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	// for #161_CG_BSMI similar pattern
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char stillGMV_true = 0;
	unsigned char moveROI_true = 0;
	unsigned char stillNonROI_true = 0;
	unsigned int moveROI_sad = 0;
	unsigned char moveROI_cnt = 0;
	unsigned char dtl_hv_prt_true = 0;
	//unsigned char dtl_all_prt_true = 0;  // unused
	unsigned char dtl_rgn_prt_true = 0;
	unsigned char stillNonROI_cnt = 0;
	unsigned int stillNonROI_sad = 0;
	unsigned char u8_i = 0;
	unsigned char EPG_true = 0;
	static unsigned char EPG_detCnt = 0;

	unsigned int moveROI_aplDiff = 0;
	unsigned int stillNonROI_aplDiff = 0;
	unsigned int u32_rimRatio = (s_pContext->_output_rimctrl.u32_rimRatio == 0) ? 128 : s_pContext->_output_rimctrl.u32_rimRatio;
	unsigned int ip31=0;
	
	stillGMV_true = ((s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb == 0) &&
		          (_ABS_((s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>2) == 0) &&
		          (_ABS_((s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>2) == 0))? 1:0;

	for(u8_i = 0;u8_i < 32; u8_i++)
	{
		if(u8_i >= 9 && u8_i <= 14)
		{
			if(_ABS_((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i]>>2)) == 0 &&
				_ABS_((s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i]>>2)) == 0)
				moveROI_cnt++;

			moveROI_sad += s_pContext->_output_read_comreg.u25_me_rSAD_rb[u8_i] ;
			moveROI_aplDiff = (s_pContext->_output_read_comreg.u25_me_rSAD_rb[u8_i]>moveROI_aplDiff)?s_pContext->_output_read_comreg.u25_me_rSAD_rb[u8_i]:moveROI_aplDiff;
		}
		else
		{
			if(_ABS_((s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[u8_i]>>2)) == 0 &&
				_ABS_((s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[u8_i]>>2)) == 0)
			{
				stillNonROI_cnt++;
				stillNonROI_sad += s_pContext->_output_read_comreg.u25_me_rSAD_rb[u8_i] ;
				stillNonROI_aplDiff = (s_pContext->_output_read_comreg.u25_me_rSAD_rb[u8_i]>stillNonROI_aplDiff)?s_pContext->_output_read_comreg.u25_me_rSAD_rb[u8_i]:stillNonROI_aplDiff;
			}
		}
	}

	if((moveROI_cnt*u32_rimRatio>>7 >= 4) && (moveROI_sad*u32_rimRatio>>7 >= 0x20000))
		moveROI_true = 1;
	else if((moveROI_cnt*u32_rimRatio>>7 >= 4) && (moveROI_sad*u32_rimRatio>>7 <= 0x200))
		moveROI_true = 0;
	 else
	 	moveROI_true = s_pContext->_output_me_sceneAnalysis.u8_EPG_moveROI;

	 pOutput->u8_EPG_moveROI= moveROI_true;

	//stillNonROI_true = ((stillNonROI_cnt*u32_rimRatio>>7 >= 25) && (stillNonROI_sad*u32_rimRatio>>7 < 0x12000))? 1:0;
	stillNonROI_true = ((stillNonROI_cnt*u32_rimRatio>>7 >= 18) && (stillNonROI_sad*u32_rimRatio>>7 < 0x12000))? 1:0;

	//dtl_hv_prt_true = (s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb*u32_rimRatio>>7 > 0x7000) && (s_pContext->_output_read_comreg.u17_me_ZMV_dtl_cnt_rb*u32_rimRatio>>7 > 0x3000);
	dtl_hv_prt_true = (s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb*u32_rimRatio>>7 > 21850) && (s_pContext->_output_read_comreg.u17_me_ZMV_dtl_cnt_rb*u32_rimRatio>>7 > 8250);

	dtl_rgn_prt_true = ((s_pContext->_output_read_comreg.u20_me_rDTL_rb[17]<s_pContext->_output_read_comreg.u20_me_rDTL_rb[18])
					//&&(s_pContext->_output_read_comreg.u20_me_rDTL_rb[18]<s_pContext->_output_read_comreg.u20_me_rDTL_rb[19])
					//&&(s_pContext->_output_read_comreg.u20_me_rDTL_rb[17]*u32_rimRatio>>7 >0xb000));
					&&(s_pContext->_output_read_comreg.u20_me_rDTL_rb[17]*u32_rimRatio>>7 >40000));

	EPG_true = (stillGMV_true == 1 && moveROI_true == 1 && stillNonROI_true == 1 && dtl_hv_prt_true == 1 && dtl_rgn_prt_true==1 && scalerVIP_Get_MEMCPatternFlag_Identification(0,116)==TRUE);

	if(EPG_true)
		EPG_detCnt = (EPG_detCnt>=8)?8:(EPG_detCnt+1);
	else
		EPG_detCnt = 0;

	if(EPG_detCnt == 8 && EPG_true)
		pOutput->u8_EPG_apply = 15;
	else if(dtl_hv_prt_true == 0)
		pOutput->u8_EPG_apply = 0;
	else if(s_pContext->_output_me_sceneAnalysis.u8_EPG_apply > 0)
		pOutput->u8_EPG_apply = s_pContext->_output_me_sceneAnalysis.u8_EPG_apply - 1;
	else
		pOutput->u8_EPG_apply = 0;

	if(pParam->u1_EPG_en==0)
		pOutput->u8_EPG_apply = 0;
#if 1
	ReadRegister(SOFTWARE2_SOFTWARE2_62_reg, 31, 31, &ip31);

	if(ip31)
	rtd_pr_memc_notice("[%s][,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,][,%d,%d,][,%d,%d,][,%d,%d,%d,%d,][,%d,%d,]\n", __FUNCTION__, pOutput->u8_EPG_apply, s_pContext->_output_rimctrl.u32_rimRatio,
	EPG_detCnt, stillGMV_true, moveROI_true, stillNonROI_true, dtl_hv_prt_true, dtl_rgn_prt_true, scalerVIP_Get_MEMCPatternFlag_Identification(0,116),
	moveROI_cnt*u32_rimRatio>>7 , moveROI_sad*u32_rimRatio>>7 ,
	stillNonROI_cnt*u32_rimRatio>>7, stillNonROI_sad*u32_rimRatio>>7,
	s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt*u32_rimRatio>>7, s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt*u32_rimRatio>>7,
	s_pContext->_output_read_comreg.u20_me_rDTL_rb[17],s_pContext->_output_read_comreg.u20_me_rDTL_rb[18],s_pContext->_output_read_comreg.u20_me_rDTL_rb[19], s_pContext->_output_read_comreg.u20_me_rDTL_rb[17]*u32_rimRatio>>7,
	s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb*u32_rimRatio>>7, s_pContext->_output_read_comreg.u17_me_ZMV_dtl_cnt_rb*u32_rimRatio>>7);
#endif

}

VOID MEMC_Simple_Scene_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned int u32_i;
	signed short cond_sad_cnt = 0, cond_dtl_cnt = 0, cond_dhconf_cnt = 0;
	signed short cond_dtl_zero_cnt = 0, cond_dhconf_zero_cnt = 0;
	signed short cond_sad_score = 0, cond_dtl_score = 0, cond_dhconf_score = 0, cond_dhconf_zero_score = 0;

	unsigned int cur_simple_scene_score = 0;

	signed short s11_1st_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>2;
	signed short s10_1st_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>2;
	char  s6_1st_gmv_cnt  = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;

	//signed short s11_2nd_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb)>>2;  // unused
	//signed short s10_2nd_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb)>>2;
	//char  s6_2nd_gmv_cnt  = s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb;


	for (u32_i = 0; u32_i < 32; u32_i ++)
	{
		if (s_pContext->_output_read_comreg.u25_me_rSAD_rb[u32_i] < 0x4800)
		{
			cond_sad_cnt++;
		}

		if (s_pContext->_output_read_comreg.u20_me_rDTL_rb[u32_i] < 0x5000)
		{
			cond_dtl_cnt++;
		}

		if (s_pContext->_output_read_comreg.u20_me_rDTL_rb[u32_i] == 0)
		{
			cond_dtl_zero_cnt++;
		}

		if (s_pContext->_output_read_comreg.u20_dh_pfvconf[u32_i] < 0x8000)
		{
			cond_dhconf_cnt++;
		}

		if (s_pContext->_output_read_comreg.u20_dh_pfvconf[u32_i] == 0)
		{
			cond_dhconf_zero_cnt++;
		}
	}

/*
score
    |
64  |---- .  .  .  .
    |      .  |
    |   .16   |
0   |.____________
     24      28      32  cnt
*/
	cond_sad_score = _IncreaseCurveMapping(cond_sad_cnt, 24, 0, 64, 16, 0);
	cond_dtl_score = _IncreaseCurveMapping(cond_dtl_cnt, 24, 0, 64, 16, 0);
	cond_dhconf_score = _IncreaseCurveMapping(cond_dhconf_cnt, 24, 0, 64, 16, 0);

	cond_dhconf_zero_score = _IncreaseCurveMapping(cond_dhconf_zero_cnt, 16, 0, 64, 8, 0);

	if (_ABS_(s11_1st_gmv_mvx) <= 1 && _ABS_(s10_1st_gmv_mvy) <= 1 && s6_1st_gmv_cnt >= 30 && cond_dtl_zero_cnt >= 2)
	{
		cur_simple_scene_score = cond_sad_score + cond_dtl_score + cond_dhconf_score + cond_dhconf_zero_score;
	}
	cur_simple_scene_score = (_CLIP_(cur_simple_scene_score, 0, 256)<<8);

	pOutput->u16_simple_scene_score_pre = ((pOutput->u16_simple_scene_score_pre*248+cur_simple_scene_score*8+128)>>8);

#if 0
	rtd_pr_memc_info("[MEMC]:%2x:%2x(%2x) %2x(%2x):%2x(%2x)%2x:%2x(%2x):%2x(%2x) %2x,%3x\n\r",
		_ABS_(s11_1st_gmv_mvx), _ABS_(s10_1st_gmv_mvy), s6_1st_gmv_cnt,
		cond_sad_cnt, cond_sad_score,
		cond_dtl_cnt, cond_dtl_score, cond_dtl_zero_cnt,
		cond_dhconf_cnt, cond_dhconf_score,
		cond_dhconf_zero_cnt, cond_dhconf_zero_score,
		(cur_simple_scene_score>>8), ((pOutput->u16_simple_scene_score_pre+128)>>8));
#endif
}

VOID MEMC_PureTopDownPanning_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	signed short s11_1st_GMVy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	unsigned int u17_1st_GMVcnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	unsigned int u12_1st_GMVucf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;

	signed short s11_2nd_GMVx = s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb;
	signed short s10_2nd_GMVy = s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb;
	unsigned int u17_2nd_GMVcnt = s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb;
	unsigned int u12_2nd_GMVucf = s_pContext->_output_read_comreg.u12_me_GMV_2nd_unconf_rb;

	const unsigned int c_TotalBlks = 240*135;

	unsigned char u1_s1cond[4] = {0};
	unsigned char u1_s2cond[4] = {0};

	// 2 kind of GMV
	u1_s1cond[0] = ( u17_2nd_GMVcnt <= ( (c_TotalBlks*1 ) >> 4 ) );
	u1_s1cond[1] = ( u17_1st_GMVcnt >= ( (c_TotalBlks*11) >> 4 ) );
	u1_s1cond[2] = ( u12_1st_GMVucf < 15 );
	u1_s1cond[3] = ( s10_2nd_GMVy >= -5 );

	// 1 kind of GMV
	u1_s2cond[0] = ( u17_1st_GMVcnt >= ( (c_TotalBlks*4 ) >> 4 ) );
	u1_s2cond[1] = ( u12_1st_GMVucf < 3 );
	u1_s2cond[2] = ( u12_2nd_GMVucf >= 0xfff );
	u1_s2cond[3] = ( s11_2nd_GMVx == 0 && s10_2nd_GMVy == 0 );

	if( ( ( u1_s1cond[0] && u1_s1cond[1] && u1_s1cond[2] /*&& u1_s1cond[3]*/ ) || ( u1_s2cond[0] && u1_s2cond[1] && u1_s2cond[2] && u1_s2cond[3] ) ) && ( s11_1st_GMVy >= 5 ) )
	{
		if( pOutput->u8_pure_TopDownPan_cnt < 64 )
			pOutput->u8_pure_TopDownPan_cnt = pOutput->u8_pure_TopDownPan_cnt + 1;
	}
	else if(pOutput->u8_pure_TopDownPan_cnt > 0)
	{
		pOutput->u8_pure_TopDownPan_cnt = pOutput->u8_pure_TopDownPan_cnt >> 1;
	}
	else
	{
		pOutput->u8_pure_TopDownPan_cnt = 0;
	}

	if( pOutput->u8_pure_TopDownPan_cnt >= 18 )
	{
		pOutput->u1_pure_TopDownPan_flag = 1;
	}
	else
	{
		pOutput->u1_pure_TopDownPan_flag = 0;
	}

}

VOID MEMC_BoundaryHPanning_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char cond_pass_v = 0, cond_pass_h = 0, i = 0;
	unsigned int log_en2 = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 25, 25, &log_en2);

	//for (i = 0; i < sizeof(arrayTable)/sizeof(unsigned char); i += 1)
	for (i = 0; i < 32; i++)
	{
		// for 100 pattern , this condition is not hard to in (original is mv_y > 2* mv_x && mv_x < 10)
		// take care corner black side effect
		if (_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]) >= 35 && 
			_ABS_(s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb) <= 1)
		{
			cond_pass_v += 1;
		}
		if (_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]) >= 60 && 
			_ABS_(s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb) <= 1)
		{
			cond_pass_h += 1;
		}
	}
	pOutput->u8_boundary_v_panning_cnt = (4 * pOutput->u8_boundary_v_panning_cnt + 4 * cond_pass_v + 4) / 8;
	if (pOutput->u8_boundary_v_panning_cnt >= 24) // 80%
	{
		pOutput->u1_boundary_v_panning_flag = 1;
	}
	else if (pOutput->u8_boundary_v_panning_cnt < 21) // 70%
	{
		pOutput->u1_boundary_v_panning_flag = 0;
	}

	pOutput->u8_boundary_h_panning_cnt = (4 * pOutput->u8_boundary_h_panning_cnt + 4 * cond_pass_h + 4) / 8;
	if (pOutput->u8_boundary_h_panning_cnt >= 24)
	{
		pOutput->u1_boundary_h_panning_flag = 1;
	}
	else if (pOutput->u8_boundary_h_panning_cnt < 21)
	{
		pOutput->u1_boundary_h_panning_flag = 0;
	}

	if(log_en2){
		rtd_pr_memc_notice("[%s][v,%d,%d,%d,][h,%d,%d,%d,]\n\r", __FUNCTION__,
			pOutput->u1_boundary_v_panning_flag, cond_pass_v, pOutput->u8_boundary_v_panning_cnt,
			pOutput->u1_boundary_h_panning_flag, cond_pass_h, pOutput->u8_boundary_h_panning_cnt);
			
	}
	#if 0

	unsigned char arrayTable[] = {0,1,2,3,4,5,6,7,8,15,16,23,24,25,26,27,28,29,30,31};
	unsigned char cond_pass = 0, i = 0;

	for (i = 0; i < sizeof(arrayTable)/sizeof(unsigned char); i += 1)
	{
		// for 100 pattern , this condition is not hard to in (original is mv_y > 2* mv_x && mv_x < 10)
		// take care corner black side effect
		if (_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[ arrayTable[i] ]) > _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[ arrayTable[i] ]) && 
			_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[ arrayTable[i] ]) < 110)
		{
			cond_pass += 1;
		}
	}
	pOutput->u8_boundary_v_panning_cnt = (4 * pOutput->u8_boundary_v_panning_cnt + 4 * cond_pass + 4) / 8;

	if (pOutput->u8_boundary_v_panning_cnt >= 10)
	{
		pOutput->u1_boundary_v_panning_flag = 1;
	}
	else if (pOutput->u8_boundary_v_panning_cnt <=6)
	{
		pOutput->u1_boundary_v_panning_flag = 0;
	}

	rtd_pr_memc_notice("%d %d %d %d %d %d %d %d | %d %d | %d %d | %d %d %d %d %d %d %d %d => %d : %d\n", 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30]), 
		_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31]), pOutput->u8_boundary_v_panning_cnt, pOutput->u1_boundary_v_panning_flag);
	#endif
}

VOID MEMC_DynamicSearchRange_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u11_GMV_x_max=0;
	unsigned char u10_GMV_y_max=0;
	signed short s11_maxGMV_x = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	signed short s10_maxGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	unsigned int glb_sad = s_pContext->_output_read_comreg.u30_me_aSAD_rb;

	int i = 0;

	//======= record pre-
	static int u8_pass_x_cnt = 0,u8_pass_y_cnt = 0;
	static signed short pre_maxGMV_x = 0,pre_maxGMV_y = 0;
	//===================

	// threshold setting
	unsigned int _TH_MVX = 8;
	unsigned int _TH_MVY = 6;
	unsigned int _TH_CNT = 4;
	unsigned int _TH_MAX_X = 12;
	unsigned int _TH_MAX_Y = 12;
	unsigned int _TH_TOT = 150;
	unsigned int _TH_SAD = 1000000;//400000;
	// end

	signed short mvx_cnt = 0;
	signed short mvy_cnt = 0;
	signed short mvx_max = 0;
	signed short mvy_max = 0;
	signed short mvx_tot = 0;
	signed short mvy_tot = 0;
	signed short diff_maxGMV_x = 0;
	signed short diff_maxGMV_y = 0;

	static unsigned int print_cnt = 0 ;
	//unsigned int u32_RB_val = 0;
	print_cnt++;

	u11_GMV_x_max = _ABS_(s11_maxGMV_x);
	u10_GMV_y_max = _ABS_(s10_maxGMV_y);

	//ReadRegister(HARDWARE_HARDWARE_57_reg, 0, 31, &u32_RB_val);
	//if( (u32_RB_val>>2)&0x1){
	//	_TH_SAD = 1000000;
	//}

	for (i=0; i<32; i++)
	{
		//s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]
		if (_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]) >= _TH_MVX)
		mvx_cnt++;

		if (_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]) >= _TH_MVY)
		mvy_cnt++;

		if (_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]) > mvx_max)
		mvx_max = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]);

		if (_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]) > mvy_max)
		mvy_max = _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]);

		mvx_tot += _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]);
		mvy_tot += _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]);
	}

	diff_maxGMV_x = _ABS_DIFF_(pre_maxGMV_x , s11_maxGMV_x);
	diff_maxGMV_y = _ABS_DIFF_(pre_maxGMV_y , s10_maxGMV_y);


	if (mvx_cnt < _TH_CNT && mvx_max < _TH_MAX_X && mvx_tot < _TH_TOT && glb_sad < _TH_SAD)
	{
		if (u8_pass_x_cnt < 32) u8_pass_x_cnt++;
	}
	else
	{
		if (u8_pass_x_cnt > 0){
			if(u11_GMV_x_max>=5){
				u8_pass_x_cnt= u8_pass_x_cnt - (u11_GMV_x_max>>1);
			}else{
				u8_pass_x_cnt-= 1;//(1 + mvx_max/_TH_MAX_X);
			}

			u8_pass_x_cnt = u8_pass_x_cnt - diff_maxGMV_x;
		}
	}

	if (mvy_cnt < _TH_CNT && mvy_max < _TH_MAX_Y && mvy_tot < _TH_TOT && glb_sad < _TH_SAD)
	{
		if (u8_pass_y_cnt < 32) u8_pass_y_cnt++;
	}
	else
	{
		if (u8_pass_y_cnt > 0) {
			if(u10_GMV_y_max>=5){
				u8_pass_y_cnt = u8_pass_y_cnt- (u10_GMV_y_max>>1);
			}else{
				u8_pass_y_cnt-= 1;//(1 + mvy_max/_TH_MAX_Y);
			}

			u8_pass_y_cnt = u8_pass_y_cnt - diff_maxGMV_y;
		}
	}

	u8_pass_x_cnt = _CLIP_(u8_pass_x_cnt,0,30);
	u8_pass_y_cnt = _CLIP_(u8_pass_y_cnt,0,30);

	if (u8_pass_x_cnt > 20) pOutput->u3_Dynamic_MVx_Step = 3;// + (mvx_cnt < _TH_CNT && mvx_max < _TH_MAX && mvx_tot < _TH_TOT);
	if (u8_pass_x_cnt < 10) pOutput->u3_Dynamic_MVx_Step = 0; // mean condition fail don't do anything in writecom

	if (u8_pass_y_cnt > 20) pOutput->u3_Dynamic_MVy_Step = 3;// + (mvx_cnt < _TH_CNT && mvx_max < _TH_MAX && mvx_tot < _TH_TOT);
	if (u8_pass_y_cnt < 10) pOutput->u3_Dynamic_MVy_Step = 0; // mean condition fail don't do anything in writecom

	//pOutput->u3_Dynamic_MVx_Step = _CLIP_((3 + (30-u8_pass_x_cnt) / 5), 3, 7);
	//pOutput->u3_Dynamic_MVy_Step = _CLIP_((3 + (30-u8_pass_y_cnt) / 5), 3, 7);


	pre_maxGMV_x = s11_maxGMV_x;
	pre_maxGMV_y = s10_maxGMV_y;

	if (pParam->u1_DSR_en != 1) {
		pOutput->u3_Dynamic_MVy_Step = 0; // for debug check
		pOutput->u3_Dynamic_MVx_Step = 0; // for debug check
	}	

#if 0
	for (i=0; i<32; i++)
    {
		if ( (u32_RB_val>>2)&0x1){
			if(print_cnt%10==0){
				if(i%8 == 0){
					rtd_pr_memc_emerg("\n");
				}
				rtd_pr_memc_emerg("(%2d, %2d)", _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]), _ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]));
			}
		}
	}
#endif

#if 0
	if ( (u32_RB_val>>1)&0x1){
		if(print_cnt%2==0){

			 rtd_pr_memc_emerg("RPG_x: %8d, %2d:%2d:%2d => %2d(%d),%d,%d\n", glb_sad, mvx_cnt, mvx_max, mvx_tot, u8_pass_x_cnt, (mvx_cnt < _TH_CNT && mvx_max < _TH_MAX_X && mvx_tot < _TH_TOT && glb_sad < 300000),u11_GMV_x_max, diff_maxGMV_x);
			 rtd_pr_memc_emerg("RPG_y: %8d, %2d:%2d:%2d => %2d(%d),%d,%d\n", glb_sad, mvy_cnt, mvy_max, mvy_tot, u8_pass_y_cnt, (mvy_cnt < _TH_CNT && mvy_max < _TH_MAX_Y && mvy_tot < _TH_TOT && glb_sad < _TH_SAD),u10_GMV_y_max, diff_maxGMV_y);


		}
	}
#endif

}

VOID MEMC_MainObject_Move_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	const _PQLPARAMETER *s_pParam       = GetPQLParameter();

	signed short s11_1st_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	signed short s10_1st_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	unsigned int u32_1st_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	unsigned int u32_1st_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;

	signed short s11_2nd_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb)>>1;
	signed short s10_2nd_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb)>>1;
	unsigned int u32_2nd_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb;
	unsigned int u32_2nd_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_2nd_unconf_rb;

	unsigned short u16_meTotalBlkNum = (s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb);
	unsigned char u1_objInBG = 0;

	unsigned char u1_cond[6] = {0};

	unsigned int u25_me_aDTL = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	static signed short s5_MainObjectEnergy = 0;
	static unsigned int u25_me_aDTL_pre = 0;
	u25_me_aDTL = (u25_me_aDTL_pre*7 + u25_me_aDTL)>>3;

	// me total block number
	u16_meTotalBlkNum = (u16_meTotalBlkNum == 0)? 1 : u16_meTotalBlkNum;
	// object in background
#if 1
	// method 1
	// [A] 1stGmvCnt [B] 2ndGmvCnt [C]totalBlk
	// implied condition : [A]>[B]
	// cond0 : [A]+[B] >= [C]*Thl_1/32
	// cond1 : [A]-[B] >= [C]*Thl_2/32
	// what if [B] close to 0?
	if( ( ((u32_1st_gmv_cnt + u32_2nd_gmv_cnt) << 8)    >= (u16_meTotalBlkNum * (pParam->u8_MainObjectMove_thl_1 << 3) )) &&
		( (_ABS_(u32_1st_gmv_cnt-u32_2nd_gmv_cnt) << 8) >= (u16_meTotalBlkNum * (pParam->u8_MainObjectMove_thl_2 << 3) )) )
	{
		u1_objInBG = 1;
	}
	else
	{
		u1_objInBG = 0;
	}
#else
	// method 2
	// [A] 1stGmvCnt [B] 2ndGmvCnt [C]totalBlk
	// implied condition : [A]>[B]
	// cond0 : [A]+[B] >= [C]*Thl_1/32
	// cond1 : [B]     >= [C]*Thl_2/32
	if( ( (u32_1st_gmv_cnt + u32_2nd_gmv_cnt) >= ((u16_meTotalBlkNum * pParam->u8_MainObjectMove_thl_1) >> 5 )) &&
		( (u32_2nd_gmv_cnt)                   >= ((u16_meTotalBlkNum * pParam->u8_MainObjectMove_thl_2) >> 10 )) )
	{
		u1_objInBG = 1;
	}
	else
	{
		u1_objInBG = 0;
	}
#endif

	// final decision
	u1_cond[0] = (u1_objInBG == 1)? 1 : 0;
	// dtl
	u1_cond[1] = (u25_me_aDTL > (pParam->u8_MainObjectMove_dtl_thl*10000)/*360000*//*750000*/)? 1 : 0;
	// bg mv not small and correct
	u1_cond[2] = ( _ABS_(s11_1st_gmv_mvx) >= pParam->u4_MainObjectMove_bgMv_thl || _ABS_(s10_1st_gmv_mvy) >= pParam->u4_MainObjectMove_bgMv_thl /*13*/)? 1 : 0;
	u1_cond[3] = ( u32_1st_gmv_unconf < pParam->u8_MainObjectMove_1st_uconf_thr /*0x18*/ )? 1 : 0;
	// (bg, object mv) diff not small and object mv correct
#if 1
	u1_cond[4] = ( ( _ABS_DIFF_(s11_1st_gmv_mvx ,s11_2nd_gmv_mvx) + _ABS_DIFF_(s10_1st_gmv_mvy ,s10_2nd_gmv_mvy) ) >= (pParam->u4_MainObjectMove_objBgMvDiff_thl)/*4*/ )? 1 : 0; //???
	u1_cond[5] = ( u32_2nd_gmv_unconf < pParam->u8_MainObjectMove_2nd_uconf_thr /*0x18*/ )? 1: 0; //???
#else
	if( u32_2nd_gmv_unconf == 4095 )
	{
		u1_cond[4] = u1_cond[4];
		u1_cond[5] = u1_cond[5];
	}
	else if( s_pContext->_output_read_comreg.u1_sc_status_rb )
	{
		u1_cond[4] = 0;
		u1_cond[5] = 0;
	}
	else
	{
		u1_cond[4] = ( ( _ABS_DIFF_(s11_1st_gmv_mvx ,s11_2nd_gmv_mvx) + _ABS_DIFF_(s10_1st_gmv_mvy ,s10_2nd_gmv_mvy) ) >= (pParam->u4_MainObjectMove_objBgMvDiff_thl)/*4*/ )? 1 : 0;
		u1_cond[5] = ( u32_2nd_gmv_unconf < pParam->u8_MainObjectMove_2nd_uconf_thr /*0x18*/ )? 1: 0;
	}
#endif

	if( u1_cond[0]==1 && u1_cond[1]==1 && u1_cond[2]==1 && u1_cond[3]==1 && u1_cond[4]==1 && u1_cond[5]==1 )
	{
		s5_MainObjectEnergy = _MIN_(s5_MainObjectEnergy + 1, 32);
	}
	else
	{
		s5_MainObjectEnergy = _MAX_(s5_MainObjectEnergy - 1, 0);
	}

	if(s_pParam->_param_dh_close.u1_dbg_print_en)
	{
		rtd_pr_memc_notice("[%s]u1_MainObject_Move=%d,(%d,%d,%d,%d,%d,%d)\n",__FUNCTION__,s5_MainObjectEnergy,u1_cond[0],  u1_cond[1], u1_cond[2], u1_cond[3], u1_cond[4], u1_cond[5]);	
	}

	if( s5_MainObjectEnergy >= 18 )
	{
		pOutput->u1_MainObject_Move = 1;
	}
	else if( s5_MainObjectEnergy <= 14 )
	{
		pOutput->u1_MainObject_Move = 0;
	}
	u25_me_aDTL_pre = u25_me_aDTL;
}

VOID MEMC_Over_SearchRange_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	//signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	//unsigned int u32_gmv_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	//unsigned int u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;

	// pure or normal panning
	if((pOutput->u2_panning_flag == 1 || pOutput->u2_panning_flag == 3 || pParam->u1_force_panning == 1) &&
		(_ABS_(u10_gmv_mvy) >= pParam->u8_over_searchrange_thl))
		pOutput->u1_over_searchrange = 1;
	else
		pOutput->u1_over_searchrange = 0;
}

VOID MEMC_NearRim_Vpan_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	unsigned int u17GMV_1st_cnt = (s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb);
	static signed short u11_gmv_mvx_pre = 0, u10_gmv_mvy_pre = 0, cnt_v = 0, cnt_h = 0;

	if( ((_ABS_(u11_gmv_mvx) == 0) && (_ABS_(u10_gmv_mvy) >= 40) && u17GMV_1st_cnt > 20000) ){ // for V panning
		// || ((_ABS_(u11_gmv_mvx) > 10) && (_ABS_(u10_gmv_mvy) >= 2*_ABS_(u11_gmv_mvx))) // for #100
		// || ((_ABS_(u10_gmv_mvy) <= 20) && (_ABS_(u11_gmv_mvx) <= 15) && (_ABS_(u10_gmv_mvy) >= 5) && (_ABS_(u11_gmv_mvx) >= 5) && u17GMV_1st_cnt>18000 && (_ABS_(u10_gmv_mvy) > 2*_ABS_(u11_gmv_mvx))) // for #100
		// || (_ABS_(u10_gmv_mvy - u10_gmv_mvy_pre) >= 16) // for #100 sudden V move
		cnt_v = 3;
	}

	if((_ABS_(u10_gmv_mvy) == 0) && (_ABS_(u11_gmv_mvx) >= 70) && u17GMV_1st_cnt > 24000){ // for H panning
		// || ((_ABS_(u10_gmv_mvy) > 10) && (_ABS_(u11_gmv_mvx) >= 2*_ABS_(u10_gmv_mvy))) // for H panning
		// || (_ABS_(u11_gmv_mvx - u11_gmv_mvx_pre) >= 18)
		cnt_h = 3;
	}

	if(cnt_v>0){
		cnt_v--;
		pOutput->u1_vpan_for_nearrim = 1;
	}else{
		pOutput->u1_vpan_for_nearrim = 0;
	}

	if(cnt_h>0){
		cnt_h--;
		pOutput->u1_hpan_for_nearrim = 1;
	}else{
		pOutput->u1_hpan_for_nearrim = 0;
	}

	u11_gmv_mvx_pre = u11_gmv_mvx;
	u10_gmv_mvy_pre = u10_gmv_mvy;

}

VOID MEMC_CMR_pattern_protect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	unsigned int u17GMV_1st_cnt = (s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb);
	unsigned int log_en = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 25, 25, &log_en);

	if((_ABS_(u10_gmv_mvy) <= 4 && _ABS_(u11_gmv_mvx) <= 2 && u17GMV_1st_cnt >= 25000)
		&& s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11] == 0 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12] == 0 && s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19] == 0
		&& s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11] == 0 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12] == 0 && s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19] == 0){
		pOutput->u1_CMR_pattern = 1;
	}else{
		pOutput->u1_CMR_pattern = 0;
	}

	if(scalerVIP_Get_MEMCPatternFlag_Identification(2,4) == TRUE && pOutput->u1_CMR_pattern == 1){
		pOutput->u1_CMR_pattern_apply = 40;
	}

	if(pOutput->u1_CMR_pattern_apply > 0)
		pOutput->u1_CMR_pattern_apply--;

	if(0){
		rtd_pr_memc_emerg("[%s][,%d,%d,],[%d,][,%d,%d,%d,][,%d,%d,%d,%d,][,%d,%d,%d,%d,]\n", __FUNCTION__, scalerVIP_Get_MEMCPatternFlag_Identification(2,4), pOutput->u1_CMR_pattern, pOutput->u1_CMR_pattern_apply,
			_ABS_(u10_gmv_mvy), _ABS_(u11_gmv_mvx), u17GMV_1st_cnt, 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19],
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12],
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[18], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19]);
	}
}

extern unsigned int g_avg_K25219;
extern unsigned int g_var_K25219;
extern unsigned int g_hue_avg_K25219;
extern unsigned int g_hue_var_K25219;
unsigned char total_frm = 16;

VOID MEMC_Brightness_Chg_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	unsigned int log_en = 0, target_level = 0, step = 20;
	unsigned int threshold_1 = 0, threshold_2 = 0, threshold_3 = 0, threshold_4= 0;
	static unsigned int dark_score = 0;
	
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 25, 25, &log_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_52_reg, 0, 7, &threshold_1);
	ReadRegister(SOFTWARE2_SOFTWARE2_52_reg, 8, 31, &threshold_2);
	ReadRegister(SOFTWARE2_SOFTWARE2_53_reg, 0, 7, &threshold_3);
	ReadRegister(SOFTWARE2_SOFTWARE2_53_reg, 8, 31, &threshold_4);
	//up_bound = low_bound + total_frm;

	if(g_avg_K25219 <= threshold_1 && g_var_K25219 <= threshold_2 && g_hue_avg_K25219 <= threshold_3 && g_hue_var_K25219 <= threshold_4){
		if(dark_score < total_frm * step)
			dark_score = dark_score + 4;
	}
	else{
		if(dark_score > 0)
			dark_score--;
	}
	
	// set target level
	for(target_level = 0; target_level < total_frm; target_level++){
		if(dark_score <= (target_level * step)){
			break;
		}
	}
	
	pOutput->u1_Brightness_Chg_Cnt = target_level;

	if(log_en){
		pr_notice("[MEMC_Brightness_Chg_Detect][,%d,%d,][,%d,%d,][,%d,]\n", g_avg_K25219, g_var_K25219, dark_score, target_level, pOutput->u1_Brightness_Chg_Cnt);
	}
}

unsigned char u8_K24987_flag = 0;
VOID MEMC_Small_Motion_Detect_K24987(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	signed short s11_gmv1_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	signed short s10_gmv1_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	static unsigned char cnt = 0;
	unsigned int log_en = 0;
	ReadRegister(SOFTWARE3_SOFTWARE3_00_reg, 0, 0, &log_en);

	if(-8 <= s11_gmv1_mvx && s11_gmv1_mvx <= 8 && -5 <= s10_gmv1_mvy && s10_gmv1_mvy <= 5){
		cnt = 60;
	}
	else{
		if(cnt > 0)
			cnt--;
	}

	if(cnt > 0){
		u8_K24987_flag = 1;
	}
	else{
		u8_K24987_flag = 0;
	}

	if(log_en){
		pr_notice("[%s][%d][s11_gmv1_mvx,%d,][s10_gmv1_mvy,%d,][,%d,%d,]\n", __FUNCTION__, cnt, s11_gmv1_mvx, s10_gmv1_mvy, s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step, s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step);
	}
}

void MEMC_Relative_Motion_detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i = 0;
	signed short u11_gmv1_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	signed short u10_gmv1_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
	signed short u11_gmv2_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb);
	signed short u10_gmv2_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb);
	unsigned int u32_gmv1_cnt = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	unsigned int u32_gmv2_cnt = s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb;	
	unsigned int u32_gmv1_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	unsigned int u32_gmv2_unconf = s_pContext->_output_read_comreg.u12_me_GMV_2nd_unconf_rb;
	unsigned int rFBss = 0, rSCss = 0, rTCss = 0, rPrd = 0, rDTL = 0, rLogo = 0;
	unsigned char  mvDiff = 0, gmv_grp_mvDiff = 0;
	unsigned char  big_relMV_cnt = 0, rChaos_cnt = 0, rTCss_cnt = 0, rPrd_cnt = 0, rLogo_cnt = 0;
	unsigned char  medFLT_sel = pParam->u2_medFLT_sel;
	bool   relMV_panning_en = pParam->b_relMV_panning_en;
	bool   prd_en = pParam->b_prd_en;
	bool   logo_en = pParam->b_logo_en;
	bool   multi_GMV_en = pParam->b_multi_GMV_en;
	unsigned char  mvDiff_thl = pParam->u8_mvDiff_thl;               
	unsigned char  rFBss_thl = pParam->u8_rFBss_thl;           
	unsigned char  rSCss_thl = pParam->u8_rSCss_thl;      
	unsigned char  rTCss_thl = 0;
	unsigned char  rPrd_thl = pParam->u8_rPrd_thl;
	unsigned char  big_relMV_cnt_thl = pParam->u8_big_relMV_cnt_thl; 
	unsigned char  rChaos_cnt_thl = pParam->u8_rChaos_cnt_thl;     
	unsigned char  rTCss_cnt_thl = 32;
	unsigned char  rPrd_cnt_thl = pParam->u8_rPrd_cnt_thl;
	unsigned char  panning_gmv_cnt_thl = pParam->u8_panning_gmv_cnt_thl;        
	unsigned char  panning_gmv_unconf_thl = pParam->u4_panning_gmv_unconf_thl; 
	unsigned int relMV_x=0, relMV_y=0;
	signed short rMV_c_vx=0, rMV_t_vx=0, rMV_b_vx=0, rMV_l_vx=0, rMV_r_vx=0;
	signed short rMV_c_vy=0, rMV_t_vy=0, rMV_b_vy=0, rMV_l_vy=0, rMV_r_vy=0;
	static unsigned char u8_logo_dbCnt = 0;
	static unsigned char u8_hold_cnt = 0;
	static unsigned int rLogo_iir[32] ={0};
	unsigned int  nbr_mvx = 0, nbr_mvy = 0, nbr_mv = 0;
	static unsigned int fcnt =0;
	unsigned int u32_RB_val;

	unsigned char  t_idx[32] = {0,	1,	 2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
	unsigned char  b_idx[32] = {8,	9,	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 24, 25, 26, 27, 28, 29, 30, 31};
	unsigned char  l_idx[32] = {0,	0,	 1,  2,  3,  4,  5,  6,  8,  8,  9, 10, 11, 12, 13, 14, 16, 16, 17, 18, 19, 20, 21, 22, 24, 24, 25, 26, 27, 28, 29, 30};
	unsigned char  r_idx[32] = {1,	2,	 3,  4,  5,  6,  7,  7,  9, 10, 11, 12, 13, 14, 15, 15, 17, 18, 19, 20, 21, 22, 23, 23, 25, 26, 27, 28, 29, 30, 31, 31};

	unsigned char  rLogo_thl[32] = {0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0,
							0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0,
							0xa0, 0x30, 0x30, 0xa0, 0xa0, 0x30, 0x30, 0xa0,
							0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0};

	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);//bit 13
							
	//--------- multi group GMV ---------
	if( (u32_gmv2_cnt > 1200 ) && (u32_gmv2_unconf < 35) ) 
		gmv_grp_mvDiff = _MAX_(_ABS_DIFF_(u11_gmv1_mvx , u11_gmv2_mvx), _ABS_DIFF_(u10_gmv1_mvy , u10_gmv2_mvy));

	pOutput->b_2grp_GMV = (multi_GMV_en && gmv_grp_mvDiff > 10) ? 1 :0;	

	//-------- panning detect ---------	
	if( ((u32_gmv1_cnt >> 6) > panning_gmv_cnt_thl ) && (u32_gmv1_unconf < panning_gmv_unconf_thl) && ((_ABS_(u11_gmv1_mvx) > 12) || (_ABS_(u10_gmv1_mvy) > 12)) && !pOutput->b_2grp_GMV)
		pOutput->u8_panning_dbCnt = 4;
	else if(pOutput->u8_panning_dbCnt != 0)
		pOutput->u8_panning_dbCnt--;	
	else
		pOutput->u8_panning_dbCnt = 0;

	pOutput->b_g_panning = (relMV_panning_en && (pOutput->u8_panning_dbCnt > 0)) ? 1 : 0;

	//-------------------------------------
	for(i=0; i<32; i++)
	{
		// regional relative MV
		//t_idx = (i <=  7) ? i : (i - 8);
		//l_idx = ((i%8)==0)? i : (i - 1);
		//b_idx = (i >= 24) ? i : (i + 8);	
		//r_idx = ((i%8)==7)? i : (i + 1);
		
		rMV_c_vx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
		rMV_c_vy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];				
		rMV_t_vx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[t_idx[i]];
		rMV_t_vy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[t_idx[i]];			
		rMV_b_vx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[b_idx[i]];
		rMV_b_vy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[b_idx[i]];		
		rMV_l_vx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[l_idx[i]];
		rMV_l_vy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[l_idx[i]];
		rMV_r_vx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[r_idx[i]];
		rMV_r_vy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[r_idx[i]];

		relMV_x = (_ABS_DIFF_(rMV_t_vx , rMV_c_vx) + _ABS_DIFF_(rMV_b_vx , rMV_c_vx) + _ABS_DIFF_(rMV_l_vx , rMV_c_vx) + _ABS_DIFF_(rMV_r_vx , rMV_c_vx)) / 4;
		relMV_y = (_ABS_DIFF_(rMV_t_vy , rMV_c_vy) + _ABS_DIFF_(rMV_b_vy , rMV_c_vy) + _ABS_DIFF_(rMV_l_vy , rMV_c_vy) + _ABS_DIFF_(rMV_r_vy , rMV_c_vy)) / 4;
		mvDiff = _MAX_(relMV_x, relMV_y);

		pOutput->mvDiff_tmp[i]=mvDiff; //YE Test it's region info 

		
		if(mvDiff > mvDiff_thl)
			big_relMV_cnt++;

		pOutput->big_relMV_cnt_tmp[i]=big_relMV_cnt;  //YE Test it's region info 
		
		// regional spatial chaos
		rFBss = s_pContext->_output_read_comreg.u32_region_fb_sum[i] / (s_pContext->_output_read_comreg.u12_region_fb_cnt[i] + 1);
		rSCss = (s_pContext->_output_read_comreg.u22_me_rSC_rb[i] >> 9);
		if(rFBss > rFBss_thl || rSCss > rSCss_thl)
			rChaos_cnt++;

		pOutput->u8_ES_rChaos_cnt_tmp[i]=rChaos_cnt; //YE Test  it's region info 
		
		// regional temporal chaos
		rTCss = (s_pContext->_output_read_comreg.u22_me_rTC_rb[i] >> 12);		
		if(rTCss > rTCss_thl)
			rTCss_cnt++;
		// rerional preodic pattern	
		rDTL  =  s_pContext->_output_read_comreg.u20_me_rDTL_rb[i];
		rPrd  = (s_pContext->_output_read_comreg.u13_me_rPRD_rb[i] >> 2);
		if(((rDTL < 100000) && (rDTL > 60000)) && (rPrd > rPrd_thl))
			rPrd_cnt++;
		// rerional logo counter	
		rLogo = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[i];
		rLogo_iir[i] = (rLogo_iir[i] + rLogo) >> 1;

		nbr_mvx = (_ABS_(rMV_t_vx) + _ABS_(rMV_b_vx) + _ABS_(rMV_l_vx) + _ABS_(rMV_r_vx)) >> 2;
		nbr_mvy = (_ABS_(rMV_t_vy) + _ABS_(rMV_b_vy) + _ABS_(rMV_l_vy) + _ABS_(rMV_r_vy)) >> 2;
		nbr_mv = _MAX_(nbr_mvx, nbr_mvy);
		if((rLogo_iir[i] > (rLogo_thl[i] << 2)) && (rLogo_iir[i] < 600) && (nbr_mv > 5))
			rLogo_cnt++;
	}

	//-------- big relative MV decision ---------
	//pOutput->b_big_relMV = (big_relMV_cnt >= big_relMV_cnt_thl) ? 1 : 0;
	if(big_relMV_cnt >= big_relMV_cnt_thl)
		pOutput->u8_big_relMV_dbCnt = 4;
	else if(pOutput->u8_big_relMV_dbCnt != 0)
		pOutput->u8_big_relMV_dbCnt--;
	else
		pOutput->u8_big_relMV_dbCnt = 0;
	pOutput->b_big_relMV = (pOutput->u8_big_relMV_dbCnt > 0) ? 1 : 0;
	//-------- Chaos decision ---------
	//pOutput->b_Chaos = (rChaos_cnt >= rChaos_cnt_thl) ? 1 : 0;
	if(rChaos_cnt >= rChaos_cnt_thl)
		pOutput->u8_rChaos_dbCnt = 4;
	else if(pOutput->u8_rChaos_dbCnt != 0)
		pOutput->u8_rChaos_dbCnt--;
	else
		pOutput->u8_rChaos_dbCnt = 0;
	pOutput->b_Chaos = (pOutput->u8_rChaos_dbCnt > 0) ? 1 : 0;	

	pOutput->b_ES_Chaos_tmp=pOutput->b_Chaos; //YE Test  it's frame info not region



	pOutput->b_TCss = (rTCss_cnt >= rTCss_cnt_thl) ? 1 : 0;
	
	//-------- periodic pattern decision ---------
	//pOutput->b_Prd = (rPrd_cnt >= rPrd_cnt_thl) ? 1 : 0;
	if(((_ABS_(u11_gmv1_mvx) < 10) && (_ABS_(u10_gmv1_mvy) < 5)) && (rPrd_cnt >= rPrd_cnt_thl))
		pOutput->u8_prd_dbCnt = 4;
	else if(pOutput->u8_prd_dbCnt != 0)
		pOutput->u8_prd_dbCnt--;
	else
		pOutput->u8_prd_dbCnt = 0;	
	pOutput->b_prd_scene = (prd_en && pOutput->u8_prd_dbCnt > 0) ? 1 : 0; 

	//-------- logo scene decision ---------
	//pOutput->b_Prd = (rPrd_cnt >= rPrd_cnt_thl) ? 1 : 0;
	if(rLogo_cnt >= 2)
		u8_logo_dbCnt = 4;
	else if(u8_logo_dbCnt != 0)
		u8_logo_dbCnt--;
	else
		u8_logo_dbCnt = 0;	
	pOutput->b_logo_scene = (logo_en && u8_logo_dbCnt > 0) ? 1 : 0;



//>>>>>>>>> for debug tuning >>>>>>>>>>
	pOutput->u8_big_relMV_cnt = big_relMV_cnt;
	pOutput->u8_rChaos_cnt = rChaos_cnt;
	pOutput->u8_rTCss_cnt = rTCss_cnt;
	pOutput->u8_rPrd_cnt = rPrd_cnt;
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<	
	 	

	//-------- final behavior decision ---------

	if(medFLT_sel == 2 ){	
		
		if((!pOutput->b_g_panning && !pOutput->b_logo_scene && (pOutput->b_big_relMV || pOutput->b_Chaos || pOutput->b_TCss)) || pOutput->b_prd_scene)
			pOutput->b_do_ME_medFLT = 1;
		else
			pOutput->b_do_ME_medFLT = 0;
		
	}else if(medFLT_sel == 3 ){			
		if(((s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true ==1)||(s_pContext->_output_me_sceneAnalysis.u1_RP_detect_true ==4))&&
			((s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVx_Step == 0)&&(s_pContext->_output_me_sceneAnalysis.u3_Dynamic_MVy_Step != 0))&&
			((s_pContext->_output_read_comreg.s11_rgn_mvy_max < 30)))
		{
			if((!pOutput->b_g_panning && !pOutput->b_logo_scene && (pOutput->b_big_relMV || pOutput->b_Chaos || pOutput->b_TCss)) || pOutput->b_prd_scene)
				pOutput->b_DRP_special_scene_check = 1;
			else
				pOutput->b_DRP_special_scene_check = 0;
		}else{
			pOutput->b_DRP_special_scene_check = 0; 
		}

		
		if (pOutput->b_DRP_special_scene_check != 0)
		{
			pOutput->b_do_ME_medFLT = 1;		
			u8_hold_cnt = 25;
		}
		else if (u8_hold_cnt > 0)
		{
			pOutput->b_do_ME_medFLT = 1;
			u8_hold_cnt = u8_hold_cnt - 1;
		}
		else
		{
			pOutput->b_do_ME_medFLT = 0;
			u8_hold_cnt = 0;
		}
		
	}else{
			pOutput->b_do_ME_medFLT = 0;
	}

	if ( (fcnt % 10 == 0  && ((u32_RB_val >>  13) & 0x01) ==1 )){
		rtd_pr_memc_emerg("[%s][%d][,%d,][,%d,%d,%d,%d,%d,%d,]\n",__FUNCTION__, __LINE__,pOutput->b_do_ME_medFLT,
				pOutput->b_g_panning,pOutput->b_logo_scene,pOutput->b_big_relMV, pOutput->b_Chaos, pOutput->b_TCss,pOutput->b_prd_scene);
		rtd_pr_memc_emerg("[%s][%d][,%d,%d,%d,][,%d,%d,%d,][,%d,%d,%d,%d,%d,][,%d,%d,]\n",__FUNCTION__, __LINE__,
				s_pContext->_output_read_comreg.s11_rgn_mvx_max,s_pContext->_output_read_comreg.u12_rgn_mvx_max_cnt,s_pContext->_output_read_comreg.u8_rgn_mvx_max_index
				,s_pContext->_output_read_comreg.s11_rgn_mvy_max,s_pContext->_output_read_comreg.u12_rgn_mvy_max_cnt,s_pContext->_output_read_comreg.u8_rgn_mvy_max_index
				,s_pContext->_output_read_comreg.u12_rgn_max_unconf,s_pContext->_output_read_comreg.s11_rgn_max_unconf_mvx,s_pContext->_output_read_comreg.s11_rgn_max_unconf_mvy
				,s_pContext->_output_read_comreg.u12_rgn_max_unconf_cnt,s_pContext->_output_read_comreg.u8_rgn_max_unconf_index
				,s_pContext->_output_read_comreg.s11_gmv_diff_maxgmv_mvx,s_pContext->_output_read_comreg.s11_gmv_diff_maxgmv_mvy);
	}
	

	if(medFLT_sel == 0)
		pOutput->b_do_ME_medFLT = 0;
	if(medFLT_sel == 1)
		pOutput->b_do_ME_medFLT = 1;


	if(fcnt<=0xffff){
		fcnt++;	
	}
else{
		fcnt=0;
	}

		
}

void MEMC_adpt_med9flt_pfvd_condition(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	//bool   pfvd_en = pParam->b_prd_en;
	//bool   pfvd_en = pParam->b_prd_en;

	if(pOutput->b_g_panning && !pOutput->b_prd_scene)
		pOutput->b_do_adpt_med9flt = 0;
	else
		pOutput->b_do_adpt_med9flt = 1;		

	if(1)
		pOutput->b_do_pfvd = 0;

}

VOID MEMC_Occl_Ext_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_OUTPUT_RESOLUTION output_Resolution = s_pContext->_external_data._output_resolution;
	//const _Param_By_Resolution_RimCtrl *s_pRimParam = GetRimParam(output_Resolution);

	signed short s11_1st_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb) >> 1;
	signed short s10_1st_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb) >> 1;
	signed short s11_2nd_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb) >> 1;
	signed short s10_2nd_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb) >> 1;

	unsigned short u16_fb_bg_mvx_diff = 0;
	signed short u8_iir_weighting = 0;
	static signed short s11_1st_gmv_mvx_iir = 0, s11_2nd_gmv_mvx_iir = 0;
	static signed short s10_1st_gmv_mvy_iir = 0, s10_2nd_gmv_mvy_iir = 0;
	static signed short s16_bg_fb_dir_x = 0; // [-48~-16] : different dir [-16~16] : transition [16~48] : same dir

	const unsigned int RgnHSize = MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.s16_resolution_width>>3;
	const unsigned int RgnVSize = MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.s16_resolution_height>>2;
	unsigned int rimLft = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_LFT];
	unsigned int rimRht = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_RHT];
	unsigned int rimUp  = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP];
	unsigned int rimDn  = s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT];
	unsigned char Rim_Pattern[32] = {0};
	unsigned char RimRgnCnt = 0;

	const unsigned char OcclExtLevelMapping[6][2] =
	{		//  low, high
		/*0*/ {   0,   10 },
		/*1*/ {   5,   15 },
		/*2*/ {  10,   25 },
		/*3*/ {  20,   40 },
		/*4*/ {  35,   50 },
		/*5*/ {  45,   99 }
	};

	const unsigned char FG_ObjPos_T1[3][32] =
	{
		{
			/**/ 0, 0, 0, 0, 0, 0, 0, 0,
			/**/ 0, 1, 1, 1, 1, 0, 0, 0,
			/**/ 0, 1, 1, 1, 1, 0, 0, 0,
			/**/ 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			/**/ 0, 0, 0, 0, 0, 0, 0, 0,
			/**/ 0, 0, 1, 1, 1, 1, 0, 0,
			/**/ 0, 0, 1, 1, 1, 1, 0, 0,
			/**/ 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			/**/ 0, 0, 0, 0, 0, 0, 0, 0,
			/**/ 0, 0, 0, 1, 1, 1, 1, 0,
			/**/ 0, 0, 0, 1, 1, 1, 1, 0,
			/**/ 0, 0, 0, 0, 0, 0, 0, 0
		}
	};

	const unsigned char FG_ObjPos_T2[5][32] =
	{
		{
			/**/ 0, 1, 1, 0, 0, 0, 0, 0,
			/**/ 0, 1, 1, 0, 0, 0, 0, 0,
			/**/ 0, 1, 1, 0, 0, 0, 0, 0,
			/**/ 0, 1, 1, 0, 0, 0, 0, 0
		},
		{
			/**/ 0, 0, 1, 1, 0, 0, 0, 0,
			/**/ 0, 0, 1, 1, 0, 0, 0, 0,
			/**/ 0, 0, 1, 1, 0, 0, 0, 0,
			/**/ 0, 0, 1, 1, 0, 0, 0, 0
		},
		{
			/**/ 0, 0, 0, 1, 1, 0, 0, 0,
			/**/ 0, 0, 0, 1, 1, 0, 0, 0,
			/**/ 0, 0, 0, 1, 1, 0, 0, 0,
			/**/ 0, 0, 0, 1, 1, 0, 0, 0
		},
		{
			/**/ 0, 0, 0, 0, 1, 1, 0, 0,
			/**/ 0, 0, 0, 0, 1, 1, 0, 0,
			/**/ 0, 0, 0, 0, 1, 1, 0, 0,
			/**/ 0, 0, 0, 0, 1, 1, 0, 0
		},
		{
			/**/ 0, 0, 0, 0, 0, 1, 1, 0,
			/**/ 0, 0, 0, 0, 0, 1, 1, 0,
			/**/ 0, 0, 0, 0, 0, 1, 1, 0,
			/**/ 0, 0, 0, 0, 0, 1, 1, 0
		}
	};

	signed short i=0, j=0;

	unsigned int T1_bg_mvx_avg[3] = {0}; // 4x2 center with offset
	unsigned int T1_RgnCnt[3] = {0};
	unsigned int T1_Debounce[3] = {0};
	static unsigned int T1_Debounce_pre[3] = {0};

	unsigned int T2_bg_mvx_avg[5] = {0}; // 4x2 center with offset
	unsigned int T2_RgnCnt[5] = {0};
	unsigned int T2_Debounce[5] = {0};
	static unsigned int T2_Debounce_pre[5] = {0};

	unsigned int MaxRgn_T1 = 0, MaxRgn_T2 = 0, MaxRgn_All = 0;

	const unsigned int TotalCenLogoBlks = 60*34*8; // 16320
	unsigned int CenRgnLogoBlks = 0;
	unsigned int Ratio_Mvy2Mvx = 0;
	unsigned int Ratio_x0 = 0, Ratio_x1 = 0, Ratio_y0 = 0, Ratio_y1 = 0;

	// Generate the rim pattern
	for(i = 0; i < 4; i++) // veritcal
	{
		for(j = 0; j < 8; j++)
		{
			if( rimLft >= (RgnHSize/3)+RgnHSize*j || rimRht <= (2*RgnHSize/3)+RgnHSize*j || rimUp >= (RgnVSize/3)+RgnVSize*i || rimDn <= (2*RgnVSize/3)+RgnVSize*i )
			{
				Rim_Pattern[i*8+j] = 1;
				RimRgnCnt += 1;
			}
			else
			{
				Rim_Pattern[i*8+j] = 0;
			}
		}
	}

	// Caclulate the center logo count
	for(i = 8; i < 24; i++)
	{
		CenRgnLogoBlks = CenRgnLogoBlks + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[i];
	}

	// Caclulate the background average mv
	for(i = 0; i < 32; i++)
	{
		unsigned int mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]);
		for( j = 0; j < 3; j++ )
		{
			if( FG_ObjPos_T1[j][i] == 0 && Rim_Pattern[i] == 0)
			{
				T1_bg_mvx_avg[j] += mvx;
				T1_RgnCnt[j] += 1;
			}
		}
		for( j = 0; j < 5; j++ )
		{
			if( FG_ObjPos_T2[j][i] == 0 && Rim_Pattern[i] == 0)
			{
				T2_bg_mvx_avg[j] += mvx;
				T2_RgnCnt[j] += 1;
			}
		}
	}
	for( i = 0; i < 3; i++ )
		T1_bg_mvx_avg[i] /= T1_RgnCnt[i];
	for( i = 0; i < 5; i++ )
		T2_bg_mvx_avg[i] /= T2_RgnCnt[i];

	// Debounce
	for( i = 0; i < 3; i++ )
		T1_Debounce[i] = 0 ;
	for( i = 0; i < 5; i++ )
		T2_Debounce[i] = 0 ;

	for(i = 0; i < 32; i++)
	{
		unsigned int mvx = _ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]);
		for( j = 0; j < 3; j++ )
		{
			if( FG_ObjPos_T1[j][i] == 0 && Rim_Pattern[i] == 0)
			{
				int mv_diff = mvx - T1_bg_mvx_avg[j];
				if (_ABS_(mv_diff) < pParam->u8_occl_ext_bg_mv_diff_thl)
					T1_Debounce[j]++;
			}
		}
		for( j = 0; j < 5; j++ )
		{
			if( FG_ObjPos_T2[j][i] == 0 && Rim_Pattern[i] == 0)
			{
				int mv_diff = mvx - T2_bg_mvx_avg[j];
				if (_ABS_(mv_diff) < pParam->u8_occl_ext_bg_mv_diff_thl)
					T2_Debounce[j]++;
			}
		}
	}

	for(i = 0; i < 3; i++)
	{
		T1_Debounce[i] = (T1_Debounce[i]+RimRgnCnt) * _MIN_(16, T1_bg_mvx_avg[i]) / 16;
		T1_Debounce_pre[i] = (32*T1_Debounce_pre[i] + 32*T1_Debounce[i] + 32)/64;
	}
	for(i = 0; i < 5; i++)
	{
		T2_Debounce[i] = (T2_Debounce[i]+RimRgnCnt) * _MIN_(16, T2_bg_mvx_avg[i]) / 16;
		T2_Debounce_pre[i] = (32*T2_Debounce_pre[i] + 32*T2_Debounce[i] + 32)/64;
	}

	// mv iir and determin the extension level by fb/bg x mv diff
	/*y dir*/
	s10_1st_gmv_mvy_iir = (s10_1st_gmv_mvy + s10_1st_gmv_mvy_iir + 1) >> 1;
	s10_2nd_gmv_mvy_iir = (s10_2nd_gmv_mvy + s10_2nd_gmv_mvy_iir + 1) >> 1;

	/*x dir*/
	if( pParam->u1_occl_ext_gmv_weight_mode == 0 )
		u8_iir_weighting = 16;
	else
		u8_iir_weighting = _MIN_(_ABS_DIFF_( s11_1st_gmv_mvx_iir , s11_1st_gmv_mvx ), 31);

	s11_1st_gmv_mvx_iir = (s11_1st_gmv_mvx_iir * u8_iir_weighting + s11_1st_gmv_mvx * (32-u8_iir_weighting) + 16) / 32;

	if( pParam->u1_occl_ext_gmv_weight_mode == 0 )
		u8_iir_weighting = 16;
	else
		u8_iir_weighting = _MIN_(_ABS_DIFF_( s11_2nd_gmv_mvx_iir , s11_2nd_gmv_mvx ), 31);

	s11_2nd_gmv_mvx_iir = (s11_2nd_gmv_mvx_iir * u8_iir_weighting + s11_2nd_gmv_mvx * (32-u8_iir_weighting) + 16) / 32;

	u16_fb_bg_mvx_diff = _ABS_DIFF_(s11_1st_gmv_mvx_iir , s11_2nd_gmv_mvx_iir);

	// sc
	if( s_pContext->_output_read_comreg.u1_sc_status_rb )
	{
		s16_bg_fb_dir_x = 0;
		s11_1st_gmv_mvx_iir = 0;
		s10_1st_gmv_mvy_iir = 0;
		s11_2nd_gmv_mvx_iir = 0;
		s10_2nd_gmv_mvy_iir = 0;
	}
	// 1st and 2nd both zero, close to zero
	else if( (s11_1st_gmv_mvx_iir == 0 ) && ( s11_2nd_gmv_mvx_iir == 0 ) )
	{
		if( s16_bg_fb_dir_x > 0 )
			s16_bg_fb_dir_x = s16_bg_fb_dir_x - 1;
		else if( s16_bg_fb_dir_x < 0 )
			s16_bg_fb_dir_x = s16_bg_fb_dir_x + 1;
	}
	/*
	// special condition, currently don't change the value
	else if( (s11_1st_gmv_mvx_iir == 0 ) || ( s11_2nd_gmv_mvx_iir == 0 ) )
	{
		s16_bg_fb_dir_x = s16_bg_fb_dir_x;
	}
	*/
	else if( (s11_1st_gmv_mvx_iir > 0  && s11_2nd_gmv_mvx_iir > 0 ) || (s11_1st_gmv_mvx_iir < 0  && s11_2nd_gmv_mvx_iir < 0 ) )
	{
		s16_bg_fb_dir_x = _MIN_(s16_bg_fb_dir_x + 1, 48);
	}
	else if( (s11_1st_gmv_mvx_iir > 0  && s11_2nd_gmv_mvx_iir < 0 ) || (s11_1st_gmv_mvx_iir < 0  && s11_2nd_gmv_mvx_iir > 0 ) )
	{
		s16_bg_fb_dir_x = _MAX_(s16_bg_fb_dir_x - 1, -48);
	
}
	/*
	else
	{
		s16_bg_fb_dir_x = s16_bg_fb_dir_x;
	}
	*/

	MaxRgn_T1 = 0;
	for(j = 0; j < 3; j++)
	{
		if( T1_Debounce_pre[j] > MaxRgn_T1 )
			MaxRgn_T1 = T1_Debounce_pre[j];
	}
	MaxRgn_T2 = 0;
	for(j = 0; j < 5; j++)
	{
		if( T2_Debounce_pre[j] > MaxRgn_T2 )
			MaxRgn_T2 = T2_Debounce_pre[j];
	}
	// consider type 2 or not
	MaxRgn_All = ( pParam->u1_occl_type2_enable == 1 )? _MAX_(MaxRgn_T1, MaxRgn_T2) : MaxRgn_T1;

	if( MaxRgn_All >= (pParam->u4_occl_ext_rgn_thl + 8) /*20*/ &&
		s_pContext->_output_me_sceneAnalysis.u1_MainObject_Move == 1 &&
		s16_bg_fb_dir_x > -16 )
	{

		// mapping the level according to the mv diff
		if( pOutput->u3_occl_ext_level == 0 )
		{
			if( u16_fb_bg_mvx_diff >= OcclExtLevelMapping[pOutput->u3_occl_ext_level][1] )
				pOutput->u3_occl_ext_level++;
		}
		else if( pOutput->u3_occl_ext_level == 1 )
		{
			if( u16_fb_bg_mvx_diff <= OcclExtLevelMapping[pOutput->u3_occl_ext_level][0] )
				pOutput->u3_occl_ext_level--;
			else if( u16_fb_bg_mvx_diff >= OcclExtLevelMapping[pOutput->u3_occl_ext_level][1] )
				pOutput->u3_occl_ext_level++;
		}
		else if( pOutput->u3_occl_ext_level == 2 )
		{
			if( u16_fb_bg_mvx_diff <= OcclExtLevelMapping[pOutput->u3_occl_ext_level][0] )
				pOutput->u3_occl_ext_level--;
			else if( u16_fb_bg_mvx_diff >= OcclExtLevelMapping[pOutput->u3_occl_ext_level][1] )
				pOutput->u3_occl_ext_level++;
		}
		else if( pOutput->u3_occl_ext_level == 3 )
		{
			if( u16_fb_bg_mvx_diff <= OcclExtLevelMapping[pOutput->u3_occl_ext_level][0] )
				pOutput->u3_occl_ext_level--;
			else if( u16_fb_bg_mvx_diff >= OcclExtLevelMapping[pOutput->u3_occl_ext_level][1] )
				pOutput->u3_occl_ext_level++;
		}
		else if( pOutput->u3_occl_ext_level == 4 )
		{
			if( u16_fb_bg_mvx_diff <= OcclExtLevelMapping[pOutput->u3_occl_ext_level][0] )
				pOutput->u3_occl_ext_level--;
			else if( u16_fb_bg_mvx_diff >= OcclExtLevelMapping[pOutput->u3_occl_ext_level][1] )
				pOutput->u3_occl_ext_level++;
		}
		else if( pOutput->u3_occl_ext_level == 5 )
		{
			if( u16_fb_bg_mvx_diff <= OcclExtLevelMapping[pOutput->u3_occl_ext_level][0] )
				pOutput->u3_occl_ext_level--;
		}
		else
		{
			pOutput->u3_occl_ext_level = 0;
		}

	}
	else if( MaxRgn_All < (pParam->u4_occl_ext_rgn_thl + 3) )
	{
		pOutput->u3_occl_ext_level = 0;
	}

	pOutput->u3_occl_ext_level = _MIN_(pOutput->u3_occl_ext_level, 5);

	// decide whether to decrease post correction strength
	/*
	Ratio
	  |
	y0|--.
	  |  |  .
	y1|  |_____.___
	  |  |     |
	  |___________
	     x0    x1  CenRgnLogoCnt
	*/

	Ratio_x0 = pParam->u8_dh_postcorr_y2x_ratio_x0;
	Ratio_x1 = pParam->u8_dh_postcorr_y2x_ratio_x1*16;
	Ratio_x1 = _MAX_(Ratio_x0, Ratio_x1);
	Ratio_y0 = pParam->u8_dh_postcorr_y2x_ratio_y0;
	Ratio_y1 = pParam->u8_dh_postcorr_y2x_ratio_y1;
	Ratio_y0 = _MAX_(Ratio_y0, Ratio_y1);

	if( CenRgnLogoBlks < Ratio_x0 )
		Ratio_Mvy2Mvx = Ratio_y0;
	else if( CenRgnLogoBlks > Ratio_x1 )
		Ratio_Mvy2Mvx = Ratio_y1;
	else
		Ratio_Mvy2Mvx = ( (Ratio_x1-Ratio_x0)*Ratio_y0 - (CenRgnLogoBlks-Ratio_x0)*(Ratio_y0-Ratio_y1) + ((Ratio_x1-Ratio_x0)>>1) ) / (Ratio_x1-Ratio_x0);

	if( (pParam->u1_dh_postcorrt_dyn_calc_en != 0) &&
		( (CenRgnLogoBlks<<11) >= TotalCenLogoBlks*pParam->u8_dh_postcorr_LogoCount_thr) &&
		(
			( (_ABS_(s10_1st_gmv_mvy_iir) >= pParam->u8_dh_postcorr_GmvMvy_thr) && ( _ABS_(s10_1st_gmv_mvy_iir)*64 >= _ABS_(s11_1st_gmv_mvx_iir)*Ratio_Mvy2Mvx ) ) ||
			( (_ABS_(s10_2nd_gmv_mvy_iir) >= pParam->u8_dh_postcorr_GmvMvy_thr) && ( _ABS_(s10_2nd_gmv_mvy_iir)*64 >= _ABS_(s11_2nd_gmv_mvx_iir)*Ratio_Mvy2Mvx ) )
		)
	  )
		pOutput->u3_occl_post_corr_level = 0;
	else
		pOutput->u3_occl_post_corr_level = pOutput->u3_occl_ext_level;

	if( pParam->u1_dh_postcorr_log_en )
	{
		/*
		rtd_pr_memc_emerg("%d %d %d %d %d %d %d\n",
			pParam->u1_dh_postcorrt_dyn_calc_en,
			Ratio_x0, Ratio_x1, Ratio_y0, Ratio_y1,
			pParam->u8_dh_postcorr_GmvMvy_thr, pParam->u8_dh_postcorr_LogoCount_thr
		);
		*/
		rtd_pr_memc_emerg("Ratio = %3d, LogoCount = %5d, mvIIR = (%4d %4d) (%4d %4d) %5d, oLv = %1d %1d\n",
			Ratio_Mvy2Mvx, CenRgnLogoBlks,
			s11_1st_gmv_mvx_iir, s10_1st_gmv_mvy_iir,
			s11_2nd_gmv_mvx_iir, s10_2nd_gmv_mvy_iir,
			s16_bg_fb_dir_x,
			pOutput->u3_occl_ext_level, pOutput->u3_occl_post_corr_level);
	}

}

#if 1
#if MEMC_EN_AI
extern int ai_genre;
extern int drvif_color_get_Genre_info(void);
#endif
extern unsigned int sat_ratio_mean_value;
extern unsigned short scalerVIP_GetGreenHue(void);
VOID MEMC_Sport_Scene_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned short Sum_hue_Green=0, Sum_hue_th_1=410, Sum_hue_th_2=680, Sat_th_1=100, Sat_th_2=125;
	unsigned int log_en, caseID=0, forceSport, forceUnSport;
	bool u1_SportScene=false;
	static bool lastScene_sport = false;
	
	signed short s11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short s10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	static signed short s11_pre_gmv_mvx=0;
	static signed short s10_pre_gmv_mvy=0;
	bool is_zero_mv_image=false;
	
	log_en = (rtd_inl(SOFTWARE3_SOFTWARE3_04_reg)>>1)&0x1;
	forceSport = (rtd_inl(SOFTWARE3_SOFTWARE3_04_reg)>>2)&0x1;
	forceUnSport = (rtd_inl(SOFTWARE3_SOFTWARE3_04_reg)>>3)&0x1;
	Sum_hue_Green = scalerVIP_GetGreenHue();

	if(!pParam->u1_detect_SportScene_en){//d910[0]
		pOutput->u1_SportScene = false;
		return;
	}
	
	if((s11_gmv_mvx==s11_pre_gmv_mvx && s10_gmv_mvy==s10_pre_gmv_mvy && s11_gmv_mvx==0 && s10_gmv_mvy==0)){
		is_zero_mv_image=true;
	}
	else{
		is_zero_mv_image=false;
	}

	
	if(forceUnSport || s_pContext->_output_read_comreg.u30_me_aSAD_rb==0 || is_zero_mv_image)//d910[3]
	{
		caseID=9;
		u1_SportScene = 0;
	}
	else if(Sum_hue_Green>=Sum_hue_th_2 && sat_ratio_mean_value>=Sat_th_2)
	{
		caseID=1;
		u1_SportScene = 1;
	}
	else if(lastScene_sport==true && Sum_hue_Green>=Sum_hue_th_1 && sat_ratio_mean_value>=Sat_th_1)
	{
		caseID=2;
		u1_SportScene = 1;
	}
#if MEMC_EN_AI
	else if(ai_genre==AI_GENRE_SPORT)
	{
		caseID=3;
		u1_SportScene = 1;
	}
#endif
	else if(forceSport)//d910[2]
	{
		caseID=4;
		u1_SportScene = 1;
	}
	else
	{
		caseID=5;
		u1_SportScene = 0;
	}

#if MEMC_EN_AI
	if(log_en) {//d910[1]
		rtd_pr_memc_emerg("[%s][%d] Case:%d AI:(%d,%d) Sum_hue_green:%d Sat:%d\n", __FUNCTION__, __LINE__, 
					caseID,ai_genre, drvif_color_get_Genre_info(),Sum_hue_Green, sat_ratio_mean_value);
	}
#else
	if(log_en) {//d910[1]
		rtd_pr_memc_emerg("[%s][%d] Case:%d Sum_hue_green:%d Sat:%d\n", __FUNCTION__, __LINE__, caseID,Sum_hue_Green, sat_ratio_mean_value);
	}
#endif

	s11_pre_gmv_mvx=s11_gmv_mvx;
	s10_pre_gmv_mvy=s10_gmv_mvy;
	lastScene_sport = u1_SportScene;
	pOutput->u1_SportScene = u1_SportScene;
}
#endif

VOID MEMC_ME_OFFSET_Proc(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	static int count = 0;
	unsigned int posX[8], posY[8];
	unsigned int tmpX[8], tmpY[8];
	int strIdx;
	int i;

	count++;
	if (count > 10000) count = 0;

	posX[0] = 7;
	posY[0] = 7;
	posX[1] = 0;
	posY[1] = 7;
	posX[2] = 1;
	posY[2] = 7;
	posX[3] = 7;
	posY[3] = 0;
	posX[4] = 1;
	posY[4] = 0;
	posX[5] = 6;
	posY[5] = 2;
	posX[6] = 0;
	posY[6] = 2;
	posX[7] = 2;
	posY[7] = 2;

	strIdx = count % 8;
	for (i=0; i<8; i++)
	{
		int idx = strIdx + i;
		if (idx > 7) idx -= 8;
		tmpX[idx] = posX[i];
		tmpY[idx] = posY[i];
	}
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,3,5, tmpY[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,0,2, tmpX[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,9,11, tmpY[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,6,8, tmpX[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,15,17, tmpY[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,12,14, tmpX[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,21,23, tmpY[3]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,18,20, tmpX[3]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,27,29, tmpY[4]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_18_reg,24,26, tmpX[4]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,3,5, tmpY[5]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,0,2, tmpX[5]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,9,11, tmpY[6]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,6,8, tmpX[6]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,15,17, tmpY[7]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,12,14, tmpX[7]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,21,23, tmpY[9]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,18,20, tmpX[9]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,27,29, tmpY[0]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_1C_reg,24,26, tmpX[0]);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,3,5, tmpY[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,0,2, tmpX[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,9,11, tmpY[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,6,8, tmpX[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,15,17, tmpY[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,12,14, tmpX[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,21,23, tmpY[3]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,18,20, tmpX[3]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,27,29, tmpY[4]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_24_reg,24,26, tmpX[4]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,3,5, tmpY[5]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,0,2, tmpX[5]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,9,11, tmpY[6]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,6,8, tmpX[6]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,15,17, tmpY[7]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,12,14, tmpX[7]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,21,23, tmpY[9]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,18,20, tmpX[9]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,27,29, tmpY[0]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_28_reg,24,26, tmpX[0]);

	strIdx = count % 3;
	for (i=0; i<8; i++)
	{
		int idx = strIdx + i;
		if (idx > 2) idx -= 3;
		tmpX[idx] = posX[i];
		tmpY[idx] = posY[i];
	}

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,3,5, tmpY[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,0,2, tmpX[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,9,11, tmpY[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,6,8, tmpX[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,15,17, tmpY[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,12,14, tmpX[2]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,21,23, tmpY[0]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_20_reg,18,20, tmpX[0]);

	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,3,5, tmpY[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,0,2, tmpX[0]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,9,11, tmpY[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,6,8, tmpX[1]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,15,17, tmpY[2]);
	WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,12,14, tmpX[2]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,21,23, tmpY[0]);
	//WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_2C_reg,18,20, tmpX[0]);

	posX[0] = 7;
	posY[0] = 7;
	posX[1] = 0;
	posY[1] = 7;
	posX[2] = 1;
	posY[2] = 7;
	posX[3] = 6;
	posY[3] = 2;
	posX[4] = 0;
	posY[4] = 2;
	posX[5] = 2;
	posY[5] = 2;

	strIdx = count % 5;
	for (i=0; i<5; i++)
	{
		int idx = strIdx + i;
		if (idx > 4) idx -= 5;
		tmpX[idx] = posX[i];
		tmpY[idx] = posY[i];
	}

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,11,13, tmpY[0]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,8,10, tmpX[0]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,17,19, tmpY[1]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,14,16, tmpX[1]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,23,25, tmpY[2]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,20,22, tmpX[2]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,29,31, tmpY[3]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_4C_reg,26,28, tmpX[3]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,3,5, tmpY[4]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,0,2, tmpX[4]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,9,11, tmpY[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,6,8, tmpX[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,15,17, tmpY[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,12,14, tmpX[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,21,23, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,18,20, tmpX[0]);

	posX[0] = 1;
	posY[0] = 1;
	posX[1] = 0;
	posY[1] = 0;

	strIdx = count % 2;
	for (i=0; i<2; i++)
	{
		int idx = strIdx + i;
		if (idx > 1) idx -= 2;
		tmpX[idx] = posX[i];
		tmpY[idx] = posY[i];
	}

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,27,29, tmpY[0]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_50_reg,24,26, tmpX[0]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,3,5, tmpY[1]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,0,2, tmpX[1]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,9,11, tmpY[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,6,8, tmpX[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,15,17, tmpY[4]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,12,14, tmpX[4]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,21,23, tmpY[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,18,20, tmpX[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,27,29, tmpY[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_54_reg,24,26, tmpX[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,3,5, tmpY[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,0,2, tmpX[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,9,11, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,6,8, tmpX[0]);

	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,15,17, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,12,14, tmpX[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,21,23, tmpY[2]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,18,20, tmpX[2]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,27,29, tmpY[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_58_reg,24,26, tmpX[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,3,5, tmpY[4]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,0,2, tmpX[4]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,9,11, tmpY[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,6,8, tmpX[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,15,17, tmpY[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,12,14, tmpX[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,21,23, tmpY[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,18,20, tmpX[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,27,29, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_5C_reg,24,26, tmpX[0]);

	posX[0] = 7;
	posY[0] = 7;
	posX[1] = 0;
	posY[1] = 7;
	posX[2] = 1;
	posY[2] = 7;
	posX[3] = 6;
	posY[3] = 2;
	posX[4] = 0;
	posY[4] = 2;

	strIdx = count % 4;
	for (i=0; i<4; i++)
	{
		int idx = strIdx + i;
		if (idx > 3) idx -= 4;
		tmpX[idx] = posX[i];
		tmpY[idx] = posY[i];
	}

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,10,12, tmpY[0]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,7,9, tmpX[0]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,16,18, tmpY[1]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,13,15, tmpX[1]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,22,24, tmpY[2]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,19,21, tmpX[2]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,28,30, tmpY[3]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_84_reg,25,27, tmpX[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,3,5, tmpY[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,0,2, tmpX[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,9,11, tmpY[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,6,8, tmpX[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,15,17, tmpY[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,12,14, tmpX[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,21,23, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,18,20, tmpX[0]);

	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,27,29, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_88_reg,24,26, tmpX[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,3,5, tmpY[2]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,0,2, tmpX[2]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,9,11, tmpY[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,6,8, tmpX[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,15,17, tmpY[4]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,12,14, tmpX[4]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,21,23, tmpY[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,18,20, tmpX[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,27,29, tmpY[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_8C_reg,24,26, tmpX[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,3,5, tmpY[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,0,2, tmpX[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,9,11, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,6,8, tmpX[0]);

	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,15,17, tmpY[1]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,12,14, tmpX[1]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,21,23, tmpY[2]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,18,20, tmpX[2]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,27,29, tmpY[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_90_reg,24,26, tmpX[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,3,5, tmpY[4]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,0,2, tmpX[4]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,9,11, tmpY[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,6,8, tmpX[5]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,15,17, tmpY[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,12,14, tmpX[6]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,21,23, tmpY[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,18,20, tmpX[7]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,27,29, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_94_reg,24,26, tmpX[0]);

/*
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,3,5, &tmpY[0]);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,0,2, &tmpX[0]);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,9,11, &tmpY[1]);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,6,8, &tmpX[1]);
	//ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,15,17, &tmpY[2]);
	//ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,12,14, &tmpX[2]);
	//ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,21,23, &tmpY[3]);
	//ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,18,20, &tmpX[3]);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,3,5, tmpY[1]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,0,2, tmpX[1]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,9,11, tmpY[0]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,6,8, tmpX[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,15,17, tmpY[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,12,14, tmpX[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,21,23, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_60_reg,18,20, tmpX[0]);

	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,3,5, &tmpY[0]);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,0,2, &tmpX[0]);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,9,11, &tmpY[1]);
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,6,8, &tmpX[1]);
	//ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,15,17, &tmpY[2]);
	//ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,12,14, &tmpX[2]);
	//ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,21,23, &tmpY[3]);
	//ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,18,20, &tmpX[3]);

	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,3,5, tmpY[1]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,0,2, tmpX[1]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,9,11, tmpY[0]);
	WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,6,8, tmpX[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,15,17, tmpY[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,12,14, tmpX[3]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,21,23, tmpY[0]);
	//WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_98_reg,18,20, tmpX[0]);
*/
}

VOID MEMC_MotionInfo_SLD(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned char u8_Idx = 0;
	unsigned int cFrmMot = 0;
	unsigned int u32_maxAFrmMot = 0, u32_maxCFrmMot = 0;
	static unsigned int u32_aFrmMot[4] = {0};
	static unsigned int u32_cFrmMot[4] = {0};
	static unsigned char u8_FrmIdx = 0;

	// all motion
	u32_aFrmMot[u8_FrmIdx] = s_pContext->_output_read_comreg.u27_ipme_aMot_rb;

	// center motion
	for(u8_Idx=0; u8_Idx<12; u8_Idx++)
	{
		cFrmMot = cFrmMot + s_pContext->_output_read_comreg.u27_ipme_12Mot_rb[u8_Idx];
	}
	u32_cFrmMot[u8_FrmIdx] = cFrmMot;

	// store the max value
	for(u8_Idx=0; u8_Idx<4; u8_Idx++)
	{
		u32_maxAFrmMot = ( u32_aFrmMot[u8_Idx] >= u32_maxAFrmMot)? u32_aFrmMot[u8_Idx] : u32_maxAFrmMot;
		u32_maxCFrmMot = ( u32_cFrmMot[u8_Idx] >= u32_maxCFrmMot)? u32_cFrmMot[u8_Idx] : u32_maxCFrmMot;
	}

	u8_FrmIdx = ((u8_FrmIdx + 1)>=4)? 0 : u8_FrmIdx + 1;

	pOutput->u32_aFrmMot_4Max = u32_maxAFrmMot;
	pOutput->u32_cFrmMot_4Max = u32_maxCFrmMot;

}


VOID MEMC_RMV_Detect_DTV_Ch2_1_tv006(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned short u16_vip_source_type = 0;
	#if 0
	int u32_RMV_0x=0,u32_RMV_0y=0,u32_RMV_1x=0,u32_RMV_1y=0,u32_RMV_8x=0,u32_RMV_8y=0,u32_RMV_9x=0,u32_RMV_9y=0,
				u32_RMV_16x=0,u32_RMV_16y=0,u32_RMV_17x=0,u32_RMV_17y=0,
				u32_RMV_24x=0,u32_RMV_24y=0,u32_RMV_25x=0,u32_RMV_25y=0,u32_RMV_26x=0,u32_RMV_26y=0,u32_RMV_27x=0,u32_RMV_27y=0,
				u32_RMV_28x=0,u32_RMV_28y=0,u32_RMV_29x=0,u32_RMV_29y=0,u32_RMV_30x=0,u32_RMV_30y=0,u32_RMV_31x=0,u32_RMV_31y=0	;
	#endif
	
	signed short u11_rmv_00_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0],
		    u10_rmv_00_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[0],
		  //   u11_rmv_01_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1],
		  //   u10_rmv_01_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[1],
		  //  u11_rmv_08_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8],
		  //  u10_rmv_08_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[8],
		  // u11_rmv_09_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9],
		  // u10_rmv_09_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9],
		    u11_rmv_16_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16],
		    u10_rmv_16_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16],
		  // u11_rmv_17_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17],
		  //u10_rmv_17_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[17],
		  // u11_rmv_24_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24],
		  // u10_rmv_24_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[24],
		    u11_rmv_25_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25],
		    u10_rmv_25_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[25],
		    u11_rmv_26_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26],
		    u10_rmv_26_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[26],
		    u11_rmv_27_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27],
		    u10_rmv_27_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[27],
		    u11_rmv_28_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28],
		    u10_rmv_28_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[28],
		    u11_rmv_29_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29],
		    u10_rmv_29_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[29],
		    u11_rmv_30_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30],
		    u10_rmv_30_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[30],
		    u11_rmv_31_mvx = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31],
		    u10_rmv_31_mvy = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[31];

	#if 0
	static unsigned int log_en=0;
	static unsigned int log_en_2=0;
	static unsigned int log_en_3=0;
	static unsigned int log_en_4=0;
	static unsigned int log_en_6=0;
	#endif

	
	static unsigned int RMV_zero_cnt=0;
	static unsigned int RMV_Con_OK_cnt=0;

	
	#if 0
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 0, 0, &log_en);		// YE Test for test_bit0
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 2, 2, &log_en_2);		// YE Test for test_bit0
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 3, 3, &log_en_3);		// YE Test for test_bit0
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 4, 4, &log_en_4);		// YE Test for test_bit0
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 6, 6, &log_en_6);		// YE Test for test_bit0
	
	
	if(log_en==1){
	rtd_pr_memc_info("0=[%d,%d], 1=[%d,%d], 8=[%d,%d], 9=[%d,%d], 16=[%d,%d], 17=[%d,%d],  24=[%d,%d], 25=[%d,%d]   26=[%d,%d], 27=[%d,%d],   28=[%d,%d], 29=[%d,%d]   30=[%d,%d], 31=[%d,%d] \n", 
		u11_rmv_00_mvx, u10_rmv_00_mvy, u11_rmv_01_mvx, u10_rmv_01_mvy,u11_rmv_08_mvx, u10_rmv_08_mvy,u11_rmv_09_mvx, u10_rmv_09_mvy,
		u11_rmv_16_mvx, u10_rmv_16_mvy,u11_rmv_17_mvx, u10_rmv_17_mvy,
		u11_rmv_24_mvx, u10_rmv_24_mvy,u11_rmv_25_mvx, u10_rmv_25_mvy,u11_rmv_26_mvx, u10_rmv_26_mvy,u11_rmv_27_mvx, u10_rmv_27_mvy,
		u11_rmv_28_mvx, u10_rmv_28_mvy,u11_rmv_29_mvx, u10_rmv_29_mvy,u11_rmv_30_mvx, u10_rmv_30_mvy,u11_rmv_31_mvx, u10_rmv_31_mvy);
	}
	#endif



	u16_vip_source_type = s_pContext->_external_info.u16_source_type;
	
	//for DTV boundary gabage.
	//only apply in DTV

	#if 1
	if(((u16_vip_source_type >= 24) && (u16_vip_source_type<=33))
		   &&((((u11_rmv_00_mvx==0)&&(u10_rmv_00_mvy==0))
		   &&((u11_rmv_16_mvx==0)&&(u10_rmv_16_mvy==0)))
		   &&(((u11_rmv_25_mvx== -1)&&(u10_rmv_25_mvy==0))||((u11_rmv_26_mvx== -1)&&(u10_rmv_26_mvy==0))
		   ||((u11_rmv_27_mvx== -1)&&(u10_rmv_27_mvy==0))||(((u11_rmv_28_mvx== -1)&&(u10_rmv_28_mvy==0))
		   ||((u11_rmv_29_mvx== -1)&&(u10_rmv_29_mvy==0))||((u11_rmv_30_mvx== -1)&&(u10_rmv_30_mvy==0))
		   ||((u11_rmv_31_mvx== -1)&&(u10_rmv_31_mvy==0))))))  {


		RMV_Con_OK_cnt++;
		if(RMV_Con_OK_cnt>=2){
		
		pOutput->u1_Q13835_flag=1;
		RMV_Con_OK_cnt=0;
		
			}
		RMV_zero_cnt=0;
				#if 0
				if(log_en_2==1){
				rtd_pr_memc_info("AAA 0=[%d,%d], 1=[%d,%d], 8=[%d,%d], 9=[%d,%d], 16=[%d,%d], 17=[%d,%d],  24=[%d,%d], 25=[%d,%d]   26=[%d,%d], 27=[%d,%d],   28=[%d,%d], 29=[%d,%d]   30=[%d,%d], 31=[%d,%d] \n", 
					u11_rmv_00_mvx, u10_rmv_00_mvy, u11_rmv_01_mvx, u10_rmv_01_mvy,u11_rmv_08_mvx, u10_rmv_08_mvy,u11_rmv_09_mvx, u10_rmv_09_mvy,
					u11_rmv_16_mvx, u10_rmv_16_mvy,u11_rmv_17_mvx, u10_rmv_17_mvy,
					u11_rmv_24_mvx, u10_rmv_24_mvy,u11_rmv_25_mvx, u10_rmv_25_mvy,u11_rmv_26_mvx, u10_rmv_26_mvy,u11_rmv_27_mvx, u10_rmv_27_mvy,
					u11_rmv_28_mvx, u10_rmv_28_mvy,u11_rmv_29_mvx, u10_rmv_29_mvy,u11_rmv_30_mvx, u10_rmv_30_mvy,u11_rmv_31_mvx, u10_rmv_31_mvy);
					
				}
				#endif
			}
	else if(((u16_vip_source_type >= 24) && (u16_vip_source_type<=33))
		   &&((((u11_rmv_00_mvx==0)&&(u10_rmv_00_mvy==0))
		   &&((u11_rmv_16_mvx==0)&&(u10_rmv_16_mvy==0)))
		   &&(((u11_rmv_25_mvx==0)&&(u10_rmv_25_mvy==0))&&((u11_rmv_26_mvx==0)&&(u10_rmv_26_mvy==0))
		   &&((u11_rmv_27_mvx==0)&&(u10_rmv_27_mvy==0))&&(((u11_rmv_28_mvx==0)&&(u10_rmv_28_mvy==0))
		   &&((u11_rmv_29_mvx==0)&&(u10_rmv_29_mvy==0))&&((u11_rmv_30_mvx==0)&&(u10_rmv_30_mvy==0))
		   &&((u11_rmv_31_mvx==0)&&(u10_rmv_31_mvy==0))))))  {

                RMV_zero_cnt++;
		   
		   //RMV_Con_OK_cnt=0;
		   		#if 0
				if(log_en_3==1){
				rtd_pr_memc_info("BBB 0=[%d,%d], 1=[%d,%d], 8=[%d,%d], 9=[%d,%d], 16=[%d,%d], 17=[%d,%d],  24=[%d,%d], 25=[%d,%d]   26=[%d,%d], 27=[%d,%d],   28=[%d,%d], 29=[%d,%d]   30=[%d,%d], 31=[%d,%d] \n", 
					u11_rmv_00_mvx, u10_rmv_00_mvy, u11_rmv_01_mvx, u10_rmv_01_mvy,u11_rmv_08_mvx, u10_rmv_08_mvy,u11_rmv_09_mvx, u10_rmv_09_mvy,
					u11_rmv_16_mvx, u10_rmv_16_mvy,u11_rmv_17_mvx, u10_rmv_17_mvy,
					u11_rmv_24_mvx, u10_rmv_24_mvy,u11_rmv_25_mvx, u10_rmv_25_mvy,u11_rmv_26_mvx, u10_rmv_26_mvy,u11_rmv_27_mvx, u10_rmv_27_mvy,
					u11_rmv_28_mvx, u10_rmv_28_mvy,u11_rmv_29_mvx, u10_rmv_29_mvy,u11_rmv_30_mvx, u10_rmv_30_mvy,u11_rmv_31_mvx, u10_rmv_31_mvy);
					
				}		
				#endif

		}
	else if(RMV_zero_cnt>=7){

		pOutput->u1_Q13835_flag=0;
		// RMV_Con_OK_cnt=0;
		RMV_zero_cnt=0;
				#if 0
				if(log_en_4==1){
				rtd_pr_memc_info("CCC 0=[%d,%d], 1=[%d,%d], 8=[%d,%d], 9=[%d,%d], 16=[%d,%d], 17=[%d,%d],  24=[%d,%d], 25=[%d,%d]   26=[%d,%d], 27=[%d,%d],   28=[%d,%d], 29=[%d,%d]   30=[%d,%d], 31=[%d,%d] \n", 
					u11_rmv_00_mvx, u10_rmv_00_mvy, u11_rmv_01_mvx, u10_rmv_01_mvy,u11_rmv_08_mvx, u10_rmv_08_mvy,u11_rmv_09_mvx, u10_rmv_09_mvy,
					u11_rmv_16_mvx, u10_rmv_16_mvy,u11_rmv_17_mvx, u10_rmv_17_mvy,
					u11_rmv_24_mvx, u10_rmv_24_mvy,u11_rmv_25_mvx, u10_rmv_25_mvy,u11_rmv_26_mvx, u10_rmv_26_mvy,u11_rmv_27_mvx, u10_rmv_27_mvy,
					u11_rmv_28_mvx, u10_rmv_28_mvy,u11_rmv_29_mvx, u10_rmv_29_mvy,u11_rmv_30_mvx, u10_rmv_30_mvy,u11_rmv_31_mvx, u10_rmv_31_mvy);
				
				}
				#endif
		}
	 else{
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_4C_reg, 6, 10, s_pContext->_output_me_sceneAnalysis.SIS_MEMC.MEMC_STATUS.reg_vbuf_invalid_cnt_th);

		pOutput->u1_Q13835_flag=0;
		 RMV_Con_OK_cnt=0;
		RMV_zero_cnt=0;
		RMV_zero_cnt=0;		

		#if 0
		if(log_en_6==1){
				rtd_pr_memc_info("DDD 0=[%d,%d], 1=[%d,%d], 8=[%d,%d], 9=[%d,%d], 16=[%d,%d], 17=[%d,%d],  24=[%d,%d], 25=[%d,%d]   26=[%d,%d], 27=[%d,%d],   28=[%d,%d], 29=[%d,%d]   30=[%d,%d], 31=[%d,%d] \n", 
					u11_rmv_00_mvx, u10_rmv_00_mvy, u11_rmv_01_mvx, u10_rmv_01_mvy,u11_rmv_08_mvx, u10_rmv_08_mvy,u11_rmv_09_mvx, u10_rmv_09_mvy,
					u11_rmv_16_mvx, u10_rmv_16_mvy,u11_rmv_17_mvx, u10_rmv_17_mvy,
					u11_rmv_24_mvx, u10_rmv_24_mvy,u11_rmv_25_mvx, u10_rmv_25_mvy,u11_rmv_26_mvx, u10_rmv_26_mvy,u11_rmv_27_mvx, u10_rmv_27_mvy,
					u11_rmv_28_mvx, u10_rmv_28_mvy,u11_rmv_29_mvx, u10_rmv_29_mvy,u11_rmv_30_mvx, u10_rmv_30_mvy,u11_rmv_31_mvx, u10_rmv_31_mvy);
					
				}
		#endif
	 	}


	if(RMV_Con_OK_cnt>=10)
	RMV_Con_OK_cnt=0;

	if(RMV_zero_cnt>=10)
	RMV_zero_cnt=0;
	
	 #endif
}

VOID MEMC_boundary_mv_convergence_Detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)//KTASKWBS-22855: garbage beside "Progresive" at right side
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_OUTPUT_RESOLUTION output_Resolution = s_pContext->_external_data._output_resolution;
	//const _Param_By_Resolution_RimCtrl *s_pRimParam = GetRimParam(output_Resolution);
	static unsigned char u1_zmv_on_cnt = 0;
	unsigned char u26_surround_area_dark_cnt = 1;	
	unsigned char u26_central_area_bright_cnt = 0;
	unsigned short bot_blklogo_cnt = s_pContext->_output_read_comreg.u10_blklogo_rgcnt[27] + s_pContext->_output_read_comreg.u10_blklogo_rgcnt[28];
	unsigned int i = 0;	
	unsigned int u32_RB_val = 0;
	unsigned int BOT_ratio = 0;

	ReadRegister(HARDWARE_HARDWARE_58_reg, 0, 31, &u32_RB_val);//0xB809D5E8 bit 25~31
 	BOT_ratio = ((_ABS_DIFF_(s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT] , s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT]))<<10)>>(9+MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.u8_V_Divide);

	//surrounding area is dark and stable, center area is bright spin semi circle
	for(i = 0;i < 32; ++i)
	{
		if(i!=3 && i!=4 && i!=10 && i!=11 && i!=12 && i!=13 && i!=18 && i!=19 && i!=20 && i!=21 && i!=27 && i!=28)
		{
			if((s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i]>>10)<2 && s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] == 0 && s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i]==s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i])
			{
				u26_surround_area_dark_cnt++;
			}
		}
		else
		{
			if((s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i]>>10) > 39)
			{
				u26_central_area_bright_cnt++;
			}
		}
	}


	if(u26_surround_area_dark_cnt>14 && u26_central_area_bright_cnt>3 && BOT_ratio<90 && BOT_ratio>80 && bot_blklogo_cnt > 150)
		u1_zmv_on_cnt = 20;

	if(u1_zmv_on_cnt > 0)
		pOutput->u1_zmv_on_flag = 1;
	else
		pOutput->u1_zmv_on_flag = 0;

	u1_zmv_on_cnt>0?u1_zmv_on_cnt--:0;


	if ( (frm_cnt % 10 == 0  && ((u32_RB_val >>  13) & 0x01) ==1 )){//0xB809D5E8
		rtd_pr_memc_emerg("[%s][%d]v0908[%d][%d,%d,%d,%d,%d]\n",__FUNCTION__, __LINE__,pOutput->u1_zmv_on_flag, u26_surround_area_dark_cnt, u26_central_area_bright_cnt, BOT_ratio, bot_blklogo_cnt, u1_zmv_on_cnt);
		if(u26_surround_area_dark_cnt <= 14 || u26_central_area_bright_cnt <=4)
		{		
			rtd_pr_memc_err("[MEMC_info]rAPLi>>10 (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[0]>>10,  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[1]>>10,  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[2]>>10,	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[3]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[4]>>10,  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[5]>>10,  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[6]>>10,  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[7]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[8]>>10,	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[9]>>10,
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[10]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[11]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[13]>>10,s_pContext->_output_read_comreg.u20_me_rAPLi_rb[14]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[15]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[16]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[17]>>10,s_pContext->_output_read_comreg.u20_me_rAPLi_rb[18]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[19]>>10,
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[20]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[21]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[22]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[23]>>10,s_pContext->_output_read_comreg.u20_me_rAPLi_rb[24]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[25]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[26]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[27]>>10,s_pContext->_output_read_comreg.u20_me_rAPLi_rb[28]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[29]>>10,
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[30]>>10, s_pContext->_output_read_comreg.u20_me_rAPLi_rb[31]>>10);

			rtd_pr_memc_err("[MEMC_info]rAPLi (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[0],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[1],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[2],	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[3], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[4],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[5],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[6],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[7], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[8],	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[9],
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[11], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[15], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[19],
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[23],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[27],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[29],
				s_pContext->_output_read_comreg.u20_me_rAPLi_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[31]);
		
			rtd_pr_memc_err("[MEMC_info]rAPLp (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.u20_me_rAPLp_rb[0], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[1], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[2], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[3], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[4],  s_pContext->_output_read_comreg.u20_me_rAPLp_rb[5],  s_pContext->_output_read_comreg.u20_me_rAPLp_rb[6],  s_pContext->_output_read_comreg.u20_me_rAPLp_rb[7], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[8],	s_pContext->_output_read_comreg.u20_me_rAPLp_rb[9],
				s_pContext->_output_read_comreg.u20_me_rAPLp_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[11], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[15], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[19],
				s_pContext->_output_read_comreg.u20_me_rAPLp_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[23],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[27],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[29],
				s_pContext->_output_read_comreg.u20_me_rAPLp_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[31]);
			
			rtd_pr_memc_err("[MEMC_info]rmv_unconf (%d,%d,%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)(%d,%d)\n", 
				s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[0],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[1],	s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[2],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[3], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[4],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[5],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[6],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[7], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[8],	s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[9],
				s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[10], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[11], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[12], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[13],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[14], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[15], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[16], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[17],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[18], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[19],
				s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[20], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[21], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[22], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[23],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[24], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[25], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[26], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[27],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[28], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[29],
				s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[30], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[31]);
		}
	}
}

unsigned char k23755_flash_flg = 0;
VOID MEMC_special_DRP_Detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)//for BTS WOSQRTK-15393
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	signed short u11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb)>>1;
	signed short u10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>1;
	unsigned int u17_me_GMV_1st_cnt_rb2 = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	unsigned int u26_aAPLi_rb = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned int u32_gmv_unconf = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;
	unsigned int u13_me_rPRD_rb[32];
	//signed short s10_maxGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb)>>2;
	//signed short s10_secGMV_y = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb)>>2;
	signed short  u10_rmv_mvy[32];
	//signed short	u11_rmv_mvx[32];
	//unsigned int u25_rgnSad_rb_R[32],u20_rgnDtl_rb_R[32],u13_rPRD_rb_R[32];
	unsigned int i=0, cnt = 0, prd_cnt = 0, prd_cnt2 = 0;
	unsigned int rPRD_rb_sum=0, rPRD_rb_sum1 = 0,rPRD_rb_sum2=0, rPRD_rb_sum_total = 0, ratio_1 = 0, ratio = 0,ratio_2=0, log_en3 = 0, test_val_1 = 0, test_val_2 = 0;
	static unsigned int u26_aAPLi_rb_pre = 0, flash_cnt = 0;

	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 0, 31, &log_en3);
	ReadRegister(SOFTWARE2_SOFTWARE2_60_reg, 0, 31, &test_val_1);
	ReadRegister(SOFTWARE2_SOFTWARE2_61_reg, 0, 7, &test_val_2);
	
	for(i=0;i<32;i++)
	{
		//u11_rmv_mvx[i]=s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
		u10_rmv_mvy[i]=s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];
		u13_me_rPRD_rb[i]= s_pContext->_output_read_comreg.u13_me_rPRD_rb[i];
		if(_ABS_(u10_rmv_mvy[i])>5)
			cnt++;
		if(u13_me_rPRD_rb[i]<=200){
			u13_me_rPRD_rb[i] = 0;
		}
		//u25_rgnSad_rb_R[i] = s_pContext->_output_read_comreg.u25_me_rSAD_rb[i];
		//u20_rgnDtl_rb_R[i] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[i];
		//u13_rPRD_rb_R[i] = s_pContext->_output_read_comreg.u13_me_rPRD_rb[i];
		rPRD_rb_sum_total = rPRD_rb_sum_total + s_pContext->_output_read_comreg.u13_me_rPRD_rb[i];
	}
	for(i=8;i<=23;i++)
	{
		if(s_pContext->_output_read_comreg.u13_me_rPRD_rb[i] > 250)
			prd_cnt++;
	}
	if(rPRD_rb_sum_total < 1){
		rPRD_rb_sum_total = 1;
	}

	rPRD_rb_sum = (u13_me_rPRD_rb[2]+u13_me_rPRD_rb[3]+u13_me_rPRD_rb[4]+u13_me_rPRD_rb[5])+(u13_me_rPRD_rb[10]+u13_me_rPRD_rb[11]+u13_me_rPRD_rb[12]+u13_me_rPRD_rb[13])+
				  (u13_me_rPRD_rb[18]+u13_me_rPRD_rb[19]+u13_me_rPRD_rb[20]+u13_me_rPRD_rb[21])+(u13_me_rPRD_rb[8]+u13_me_rPRD_rb[16]);
	ratio = (rPRD_rb_sum*100)/rPRD_rb_sum_total;

	rPRD_rb_sum1 = (u13_me_rPRD_rb[4]+u13_me_rPRD_rb[5]+u13_me_rPRD_rb[6]+u13_me_rPRD_rb[7])+(u13_me_rPRD_rb[12]+u13_me_rPRD_rb[13]+u13_me_rPRD_rb[14]+u13_me_rPRD_rb[15])+
				  (u13_me_rPRD_rb[20]+u13_me_rPRD_rb[21]+u13_me_rPRD_rb[22]+u13_me_rPRD_rb[23])+(u13_me_rPRD_rb[8]+u13_me_rPRD_rb[9]+u13_me_rPRD_rb[16]+u13_me_rPRD_rb[17]);
	ratio_1 = (rPRD_rb_sum1*100)/rPRD_rb_sum_total;

	rPRD_rb_sum2 = (u13_me_rPRD_rb[16]+u13_me_rPRD_rb[17]);
	ratio_2 = (rPRD_rb_sum2*100)/rPRD_rb_sum_total;

	for(i=8;i<=23;i++)
	{
		if(s_pContext->_output_read_comreg.u13_me_rPRD_rb[i] > 250)
			prd_cnt++;

		if((s_pContext->_output_read_comreg.u13_me_rPRD_rb[16]+s_pContext->_output_read_comreg.u13_me_rPRD_rb[17])> 500 && (ratio_2>15 && ratio_2<=30))
			prd_cnt2++;
		
	}

	if(((log_en3>>0)&0x1) == 1){
		rtd_pr_memc_notice("[YE_MEMC], cnt=%d, u11_gmv_mvx=%d, u10_gmv_mvy=%d,u17_me_GMV_1st_cnt_rb2=%d, u26_aAPLi_rb=%d, u32_gmv_unconf=%d, rPRD_rb_sum=%d, \n\r"
			, cnt, u11_gmv_mvx, u10_gmv_mvy, u17_me_GMV_1st_cnt_rb2, u26_aAPLi_rb, u32_gmv_unconf, rPRD_rb_sum );
	}

	//K-23755
	if(u26_aAPLi_rb > u26_aAPLi_rb_pre+test_val_1 || u26_aAPLi_rb_pre > u26_aAPLi_rb+test_val_1){
		flash_cnt += 10;
	}
	if(flash_cnt > 150){
		flash_cnt = 150;
	}

	if(flash_cnt > test_val_2){
		k23755_flash_flg = 1;
	}
	else{
		k23755_flash_flg = 0;
	}

	if(flash_cnt > 0){
		flash_cnt--;
	}
	u26_aAPLi_rb_pre = u26_aAPLi_rb;

	if((cnt<=15) && (u11_gmv_mvx <=4 && u10_gmv_mvy <= 2) && (u17_me_GMV_1st_cnt_rb2>=22000 && u17_me_GMV_1st_cnt_rb2<=34000) && 
		//(u26_aAPLi_rb>=3000000 && u26_aAPLi_rb<=5000000) && (u32_gmv_unconf<=3) && (rPRD_rb_sum>2000 && rPRD_rb_sum<6500) )
		(u26_aAPLi_rb>=3000000 && u26_aAPLi_rb<=5300000) && (u32_gmv_unconf<=6) && ((ratio >= 65) ||(ratio_1 >= 65))) //k6
	{
		pOutput->u1_BTS_DRP_detect_true = 1;
	}else{
		pOutput->u1_BTS_DRP_detect_true = 0;
	}

	//# PQ_Issue_SAN_ANDREAS_01_repeat.mp4
	if((prd_cnt>6) && (_ABS_(u10_gmv_mvy) < 3) && (u17_me_GMV_1st_cnt_rb2>=16000 && u17_me_GMV_1st_cnt_rb2<=28000) && 
		(u26_aAPLi_rb>1000000 && u26_aAPLi_rb<2800000))
	{
		pOutput->u1_SAN_ANDREAS_DRP_detect_true = 30;
	}else{
		pOutput->u1_SAN_ANDREAS_DRP_detect_true>0?pOutput->u1_SAN_ANDREAS_DRP_detect_true--:0;
	}
	if(((log_en3>>5)&0x1) == 1){
		static unsigned char counter = 0;
		if(counter % 3 == 0)
		rtd_pr_memc_notice("[HUGH_MEMC](%d)%d, %d, %d, %d, %d, %d\n\r"
			,pOutput->u1_SAN_ANDREAS_DRP_detect_true,  prd_cnt, u11_gmv_mvx, u10_gmv_mvy, u17_me_GMV_1st_cnt_rb2, u26_aAPLi_rb, u32_gmv_unconf);
		counter++;
	}

	//# u1_Adele_DRP_detect_true
	if((cnt<4) && (_ABS_(u11_gmv_mvx) < 3 && _ABS_(u10_gmv_mvy) < 3) && (u17_me_GMV_1st_cnt_rb2>=27000 && u17_me_GMV_1st_cnt_rb2<=30000) && 
		(u26_aAPLi_rb>1200000 && u26_aAPLi_rb<3200000) && (u32_gmv_unconf <=3))
	{
		pOutput->u1_Adele_DRP_detect_true = 30;
	}else{
		pOutput->u1_Adele_DRP_detect_true>0?pOutput->u1_Adele_DRP_detect_true--:0;
	}

	if(((log_en3>>6)&0x1) == 1){
		static unsigned char counter2 = 0;
		if(counter2 % 3 == 0)
		rtd_pr_memc_notice("[HUGH_MEMC](%d)%d, %d, %d, %d, %d, %d, %d\n\r"
			,pOutput->u1_Adele_DRP_detect_true, prd_cnt2, cnt, u11_gmv_mvx, u10_gmv_mvy, u17_me_GMV_1st_cnt_rb2, u26_aAPLi_rb, u32_gmv_unconf);
		counter2++;
	}
	
	if(((log_en3>>0)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u25_me_aDTL_rb,%d,][u16_mc_Hdtl_cnt,%d,][u16_mc_Vdtl_cnt,%d,][GMV_1st,%d,%d,][GMV_2nd,%d,%d,][GMV_1st_cnt,%d,%d,][GMV_2nd_cnt,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u25_me_aDTL_rb, 
			s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt, s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt,
			s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb, s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb,
			s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb, s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb,
			s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb, s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb,
			s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb, s_pContext->_output_read_comreg.u12_me_GMV_2nd_unconf_rb);
	}
	if(((log_en3>>1)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u10_mc_VHdtl_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[0], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[1], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[2], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[3], 
			s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[4], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[5], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[6], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[7], 
			s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[8], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[9], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[10], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[11], 
			s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[12], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[13], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[14], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[15], 
			s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[16], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[17], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[18], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[19], 
			s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[20], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[21], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[22], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[23], 
			s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[24], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[25], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[26], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[27], 
			s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[28], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[29], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[30], s_pContext->_output_read_comreg.u10_mc_VHdtl_rb[31]);
	}
	if(((log_en3>>2)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u13_me_rPRD_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u13_me_rPRD_rb[0], s_pContext->_output_read_comreg.u13_me_rPRD_rb[1], s_pContext->_output_read_comreg.u13_me_rPRD_rb[2], s_pContext->_output_read_comreg.u13_me_rPRD_rb[3], 
			s_pContext->_output_read_comreg.u13_me_rPRD_rb[4], s_pContext->_output_read_comreg.u13_me_rPRD_rb[5], s_pContext->_output_read_comreg.u13_me_rPRD_rb[6], s_pContext->_output_read_comreg.u13_me_rPRD_rb[7], 
			s_pContext->_output_read_comreg.u13_me_rPRD_rb[8], s_pContext->_output_read_comreg.u13_me_rPRD_rb[9], s_pContext->_output_read_comreg.u13_me_rPRD_rb[10], s_pContext->_output_read_comreg.u13_me_rPRD_rb[11], 
			s_pContext->_output_read_comreg.u13_me_rPRD_rb[12], s_pContext->_output_read_comreg.u13_me_rPRD_rb[13], s_pContext->_output_read_comreg.u13_me_rPRD_rb[14], s_pContext->_output_read_comreg.u13_me_rPRD_rb[15], 
			s_pContext->_output_read_comreg.u13_me_rPRD_rb[16], s_pContext->_output_read_comreg.u13_me_rPRD_rb[17], s_pContext->_output_read_comreg.u13_me_rPRD_rb[18], s_pContext->_output_read_comreg.u13_me_rPRD_rb[19], 
			s_pContext->_output_read_comreg.u13_me_rPRD_rb[20], s_pContext->_output_read_comreg.u13_me_rPRD_rb[21], s_pContext->_output_read_comreg.u13_me_rPRD_rb[22], s_pContext->_output_read_comreg.u13_me_rPRD_rb[23], 
			s_pContext->_output_read_comreg.u13_me_rPRD_rb[24], s_pContext->_output_read_comreg.u13_me_rPRD_rb[25], s_pContext->_output_read_comreg.u13_me_rPRD_rb[26], s_pContext->_output_read_comreg.u13_me_rPRD_rb[27], 
			s_pContext->_output_read_comreg.u13_me_rPRD_rb[28], s_pContext->_output_read_comreg.u13_me_rPRD_rb[29], s_pContext->_output_read_comreg.u13_me_rPRD_rb[30], s_pContext->_output_read_comreg.u13_me_rPRD_rb[31]);
	}
	if(((log_en3>>3)&0x1) == 1){
		rtd_pr_memc_notice("[%s][s11_me_rMV_1st_vx_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[10], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[13], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[20], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[21], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[22], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31]);
	}
	if(((log_en3>>4)&0x1) == 1){
		rtd_pr_memc_notice("[%s][s10_me_rMV_1st_vy_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[0], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[1], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[2], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[3], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[4], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[5], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[6], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[7], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[8], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[10], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[13], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[15], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[17], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[18], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[20], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[21], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[22], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[23], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[24], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[25], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[26], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[27], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[28], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[29], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[30], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[31]);
	}	
	if(((log_en3>>5)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u25_me_rSAD_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[0], s_pContext->_output_read_comreg.u25_me_rSAD_rb[1], s_pContext->_output_read_comreg.u25_me_rSAD_rb[2], s_pContext->_output_read_comreg.u25_me_rSAD_rb[3], 
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[4], s_pContext->_output_read_comreg.u25_me_rSAD_rb[5], s_pContext->_output_read_comreg.u25_me_rSAD_rb[6], s_pContext->_output_read_comreg.u25_me_rSAD_rb[7], 
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[8], s_pContext->_output_read_comreg.u25_me_rSAD_rb[9], s_pContext->_output_read_comreg.u25_me_rSAD_rb[10], s_pContext->_output_read_comreg.u25_me_rSAD_rb[11], 
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[12], s_pContext->_output_read_comreg.u25_me_rSAD_rb[13], s_pContext->_output_read_comreg.u25_me_rSAD_rb[14], s_pContext->_output_read_comreg.u25_me_rSAD_rb[15], 
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[16], s_pContext->_output_read_comreg.u25_me_rSAD_rb[17], s_pContext->_output_read_comreg.u25_me_rSAD_rb[18], s_pContext->_output_read_comreg.u25_me_rSAD_rb[19], 
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[20], s_pContext->_output_read_comreg.u25_me_rSAD_rb[21], s_pContext->_output_read_comreg.u25_me_rSAD_rb[22], s_pContext->_output_read_comreg.u25_me_rSAD_rb[23], 
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[24], s_pContext->_output_read_comreg.u25_me_rSAD_rb[25], s_pContext->_output_read_comreg.u25_me_rSAD_rb[26], s_pContext->_output_read_comreg.u25_me_rSAD_rb[27], 
			s_pContext->_output_read_comreg.u25_me_rSAD_rb[28], s_pContext->_output_read_comreg.u25_me_rSAD_rb[29], s_pContext->_output_read_comreg.u25_me_rSAD_rb[30], s_pContext->_output_read_comreg.u25_me_rSAD_rb[31]);
	}
	if(((log_en3>>6)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u20_me_rDTL_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[0], s_pContext->_output_read_comreg.u20_me_rDTL_rb[1], s_pContext->_output_read_comreg.u20_me_rDTL_rb[2], s_pContext->_output_read_comreg.u20_me_rDTL_rb[3], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[4], s_pContext->_output_read_comreg.u20_me_rDTL_rb[5], s_pContext->_output_read_comreg.u20_me_rDTL_rb[6], s_pContext->_output_read_comreg.u20_me_rDTL_rb[7], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[8], s_pContext->_output_read_comreg.u20_me_rDTL_rb[9], s_pContext->_output_read_comreg.u20_me_rDTL_rb[10], s_pContext->_output_read_comreg.u20_me_rDTL_rb[11], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[12], s_pContext->_output_read_comreg.u20_me_rDTL_rb[13], s_pContext->_output_read_comreg.u20_me_rDTL_rb[14], s_pContext->_output_read_comreg.u20_me_rDTL_rb[15], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[16], s_pContext->_output_read_comreg.u20_me_rDTL_rb[17], s_pContext->_output_read_comreg.u20_me_rDTL_rb[18], s_pContext->_output_read_comreg.u20_me_rDTL_rb[19], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[20], s_pContext->_output_read_comreg.u20_me_rDTL_rb[21], s_pContext->_output_read_comreg.u20_me_rDTL_rb[22], s_pContext->_output_read_comreg.u20_me_rDTL_rb[23], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[24], s_pContext->_output_read_comreg.u20_me_rDTL_rb[25], s_pContext->_output_read_comreg.u20_me_rDTL_rb[26], s_pContext->_output_read_comreg.u20_me_rDTL_rb[27], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[28], s_pContext->_output_read_comreg.u20_me_rDTL_rb[29], s_pContext->_output_read_comreg.u20_me_rDTL_rb[30], s_pContext->_output_read_comreg.u20_me_rDTL_rb[31]);
	}
	if(((log_en3>>7)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u22_me_rTC_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u22_me_rTC_rb[0], s_pContext->_output_read_comreg.u22_me_rTC_rb[1], s_pContext->_output_read_comreg.u22_me_rTC_rb[2], s_pContext->_output_read_comreg.u22_me_rTC_rb[3], 
			s_pContext->_output_read_comreg.u22_me_rTC_rb[4], s_pContext->_output_read_comreg.u22_me_rTC_rb[5], s_pContext->_output_read_comreg.u22_me_rTC_rb[6], s_pContext->_output_read_comreg.u22_me_rTC_rb[7], 
			s_pContext->_output_read_comreg.u22_me_rTC_rb[8], s_pContext->_output_read_comreg.u22_me_rTC_rb[9], s_pContext->_output_read_comreg.u22_me_rTC_rb[10], s_pContext->_output_read_comreg.u22_me_rTC_rb[11], 
			s_pContext->_output_read_comreg.u22_me_rTC_rb[12], s_pContext->_output_read_comreg.u22_me_rTC_rb[13], s_pContext->_output_read_comreg.u22_me_rTC_rb[14], s_pContext->_output_read_comreg.u22_me_rTC_rb[15], 
			s_pContext->_output_read_comreg.u22_me_rTC_rb[16], s_pContext->_output_read_comreg.u22_me_rTC_rb[17], s_pContext->_output_read_comreg.u22_me_rTC_rb[18], s_pContext->_output_read_comreg.u22_me_rTC_rb[19], 
			s_pContext->_output_read_comreg.u22_me_rTC_rb[20], s_pContext->_output_read_comreg.u22_me_rTC_rb[21], s_pContext->_output_read_comreg.u22_me_rTC_rb[22], s_pContext->_output_read_comreg.u22_me_rTC_rb[23], 
			s_pContext->_output_read_comreg.u22_me_rTC_rb[24], s_pContext->_output_read_comreg.u22_me_rTC_rb[25], s_pContext->_output_read_comreg.u22_me_rTC_rb[26], s_pContext->_output_read_comreg.u22_me_rTC_rb[27], 
			s_pContext->_output_read_comreg.u22_me_rTC_rb[28], s_pContext->_output_read_comreg.u22_me_rTC_rb[29], s_pContext->_output_read_comreg.u22_me_rTC_rb[30], s_pContext->_output_read_comreg.u22_me_rTC_rb[31]);
	}
	if(((log_en3>>8)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u12_me_rMV_1st_cnt_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[0], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[1], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[2], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[3], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[4], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[5], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[6], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[7], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[8], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[9], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[10], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[11], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[12], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[13], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[14], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[15], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[16], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[17], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[18], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[19], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[20], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[21], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[22], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[23], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[24], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[25], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[26], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[27], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[28], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[29], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[30], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[31]);
		rtd_pr_memc_notice("[%s][u12_me_rMV_2nd_cnt_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[0], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[1], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[2], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[3], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[4], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[5], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[6], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[7], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[8], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[9], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[10], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[11], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[12], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[13], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[14], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[15], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[16], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[17], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[18], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[19], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[20], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[21], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[22], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[23], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[24], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[25], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[26], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[27], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[28], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[29], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[30], s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[31]);
	}
	if(((log_en3>>9)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u12_me_rMV_1st_unconf_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[0], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[1], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[2], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[3], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[4], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[5], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[6], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[7], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[8], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[9], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[10], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[11], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[12], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[13], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[14], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[15], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[16], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[17], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[18], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[19], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[20], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[21], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[22], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[23], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[24], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[25], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[26], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[27], 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[28], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[29], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[30], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[31]);
		rtd_pr_memc_notice("[%s][u12_me_rMV_2nd_unconf_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[0], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[1], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[2], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[3], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[4], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[5], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[6], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[7], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[8], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[9], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[10], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[11], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[12], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[13], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[14], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[15], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[16], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[17], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[18], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[19], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[20], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[21], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[22], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[23], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[24], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[25], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[26], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[27], 
			s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[28], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[29], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[30], s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[31]);
	}
	if(((log_en3>>11)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u12_region_fb_cnt][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u12_region_fb_cnt[0], s_pContext->_output_read_comreg.u12_region_fb_cnt[1], s_pContext->_output_read_comreg.u12_region_fb_cnt[2], s_pContext->_output_read_comreg.u12_region_fb_cnt[3], 
			s_pContext->_output_read_comreg.u12_region_fb_cnt[4], s_pContext->_output_read_comreg.u12_region_fb_cnt[5], s_pContext->_output_read_comreg.u12_region_fb_cnt[6], s_pContext->_output_read_comreg.u12_region_fb_cnt[7], 
			s_pContext->_output_read_comreg.u12_region_fb_cnt[8], s_pContext->_output_read_comreg.u12_region_fb_cnt[9], s_pContext->_output_read_comreg.u12_region_fb_cnt[10], s_pContext->_output_read_comreg.u12_region_fb_cnt[11], 
			s_pContext->_output_read_comreg.u12_region_fb_cnt[12], s_pContext->_output_read_comreg.u12_region_fb_cnt[13], s_pContext->_output_read_comreg.u12_region_fb_cnt[14], s_pContext->_output_read_comreg.u12_region_fb_cnt[15], 
			s_pContext->_output_read_comreg.u12_region_fb_cnt[16], s_pContext->_output_read_comreg.u12_region_fb_cnt[17], s_pContext->_output_read_comreg.u12_region_fb_cnt[18], s_pContext->_output_read_comreg.u12_region_fb_cnt[19], 
			s_pContext->_output_read_comreg.u12_region_fb_cnt[20], s_pContext->_output_read_comreg.u12_region_fb_cnt[21], s_pContext->_output_read_comreg.u12_region_fb_cnt[22], s_pContext->_output_read_comreg.u12_region_fb_cnt[23], 
			s_pContext->_output_read_comreg.u12_region_fb_cnt[24], s_pContext->_output_read_comreg.u12_region_fb_cnt[25], s_pContext->_output_read_comreg.u12_region_fb_cnt[26], s_pContext->_output_read_comreg.u12_region_fb_cnt[27], 
			s_pContext->_output_read_comreg.u12_region_fb_cnt[28], s_pContext->_output_read_comreg.u12_region_fb_cnt[29], s_pContext->_output_read_comreg.u12_region_fb_cnt[30], s_pContext->_output_read_comreg.u12_region_fb_cnt[31]);
	}
	if(((log_en3>>12)&0x1) == 1){
		rtd_pr_memc_notice("[%s][u32_region_fb_sum][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u32_region_fb_sum[0], s_pContext->_output_read_comreg.u32_region_fb_sum[1], s_pContext->_output_read_comreg.u32_region_fb_sum[2], s_pContext->_output_read_comreg.u32_region_fb_sum[3], 
			s_pContext->_output_read_comreg.u32_region_fb_sum[4], s_pContext->_output_read_comreg.u32_region_fb_sum[5], s_pContext->_output_read_comreg.u32_region_fb_sum[6], s_pContext->_output_read_comreg.u32_region_fb_sum[7], 
			s_pContext->_output_read_comreg.u32_region_fb_sum[8], s_pContext->_output_read_comreg.u32_region_fb_sum[9], s_pContext->_output_read_comreg.u32_region_fb_sum[10], s_pContext->_output_read_comreg.u32_region_fb_sum[11], 
			s_pContext->_output_read_comreg.u32_region_fb_sum[12], s_pContext->_output_read_comreg.u32_region_fb_sum[13], s_pContext->_output_read_comreg.u32_region_fb_sum[14], s_pContext->_output_read_comreg.u32_region_fb_sum[15], 
			s_pContext->_output_read_comreg.u32_region_fb_sum[16], s_pContext->_output_read_comreg.u32_region_fb_sum[17], s_pContext->_output_read_comreg.u32_region_fb_sum[18], s_pContext->_output_read_comreg.u32_region_fb_sum[19], 
			s_pContext->_output_read_comreg.u32_region_fb_sum[20], s_pContext->_output_read_comreg.u32_region_fb_sum[21], s_pContext->_output_read_comreg.u32_region_fb_sum[22], s_pContext->_output_read_comreg.u32_region_fb_sum[23], 
			s_pContext->_output_read_comreg.u32_region_fb_sum[24], s_pContext->_output_read_comreg.u32_region_fb_sum[25], s_pContext->_output_read_comreg.u32_region_fb_sum[26], s_pContext->_output_read_comreg.u32_region_fb_sum[27], 
			s_pContext->_output_read_comreg.u32_region_fb_sum[28], s_pContext->_output_read_comreg.u32_region_fb_sum[29], s_pContext->_output_read_comreg.u32_region_fb_sum[30], s_pContext->_output_read_comreg.u32_region_fb_sum[31]);
	}

}

VOID MEMC_info_Print(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)//for gmv analysis
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	signed short s11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short s10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	signed short s11_2nd_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb);
	signed short s10_2nd_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb);	
	static int frameCnt_tmp = 10;
	unsigned int print_en = 0;
	unsigned int u1_RB_val;
	ReadRegister(HARDWARE_HARDWARE_26_reg, 0, 0, &u1_RB_val);
	frameCnt_tmp++;	
	print_en = u1_RB_val;
	if(print_en)
	{
		rtd_pr_memc_emerg(",%d,%d,%d,%d,%d\n",s11_gmv_mvx,s10_gmv_mvy,s11_2nd_gmv_mvx,s10_2nd_gmv_mvy,frameCnt_tmp);		
	}
	{
		ROSPrintf_MEMC_IP30_interface2("[MEMC_info_1]gmv_x, gmv_y = %d,%d\n", s11_gmv_mvx, s10_gmv_mvy);
		ROSPrintf_MEMC_IP30_interface2("[MEMC_info_2]other (%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)(%d,%d,%d,%d)\n", 
			s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb, s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb, s_pContext->_output_read_comreg.u27_me_aTC_rb, s_pContext->_output_read_comreg.u27_me_aSC_rb,
			s_pContext->_output_read_comreg.u30_me_aSAD_rb, s_pContext->_output_read_comreg.u25_me_aDTL_rb, s_pContext->_output_read_comreg.u26_me_aAPLi_rb, s_pContext->_output_read_comreg.u26_me_aAPLp_rb,
			s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb, s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb, s_pContext->_output_read_comreg.u1_sc_status_rb, s_pContext->_output_read_comreg.u1_sc_status_logo_rb,
			s_pContext->_output_read_comreg.u1_sc_status_dh_rb, s_pContext->_output_read_comreg.u17_me_ZMV_cnt_rb, s_pContext->_output_read_comreg.u17_me_ZMV_dtl_cnt_rb, s_pContext->_output_read_comreg.u31_me_ZMV_psad_dc_rb,
			s_pContext->_output_read_comreg.u31_me_ZMV_psad_ac_rb, s_pContext->_output_read_comreg.u20_me_ZMV_DcGreatAc_cnt_rb, s_pContext->_output_read_comreg.u20_me_ZMV_AcGreatDc_cnt_rb, s_pContext->_output_read_comreg.u6_me_ZGMV_cnt_rb,
			s_pContext->_output_read_comreg.u20_me_Hpan_cnt_rb, s_pContext->_output_read_comreg.u20_me_Vpan_cnt_rb, s_pContext->_output_read_comreg.u16_mc_Hdtl_cnt, s_pContext->_output_read_comreg.u16_mc_Vdtl_cnt,
			s_pContext->_output_read_comreg.u17_me_posCnt_0_rb, s_pContext->_output_read_comreg.u17_me_posCnt_1_rb, s_pContext->_output_read_comreg.u17_me_posCnt_2_rb, s_pContext->_output_read_comreg.u17_me_posCnt_3_rb,
			s_pContext->_output_read_comreg.u17_me_negCnt_0_rb, s_pContext->_output_read_comreg.u17_me_negCnt_1_rb, s_pContext->_output_read_comreg.u17_me_negCnt_2_rb, s_pContext->_output_read_comreg.u17_me_negCnt_3_rb,
			s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb, s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb, s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb, s_pContext->_output_read_comreg.u12_me_GMV_2nd_unconf_rb,
			s_pContext->_output_read_comreg.u27_ipme_aMot_rb, s_pContext->_output_read_comreg.u3_ipme_filmMode_rb, s_pContext->_output_read_comreg.u1_kphase_inLR, s_pContext->_output_read_comreg.u4_kphase_inPhase,
			s_pContext->_output_read_comreg.u8_sys_N_rb, s_pContext->_output_read_comreg.u8_sys_M_rb, s_pContext->_output_read_comreg.u13_ip_vtrig_dly, s_pContext->_output_read_comreg.u13_me2_org_vtrig_dly,
			s_pContext->_output_read_comreg.u13_me2_vtrig_dly, s_pContext->_output_read_comreg.u13_dec_vtrig_dly, s_pContext->_output_read_comreg.u13_vtotal, s_pContext->_output_read_comreg.u16_inHTotal,
			s_pContext->_output_read_comreg.u16_inVAct, s_pContext->_output_read_comreg.u16_inFrm_Pos, s_pContext->_output_read_comreg.u19_me_statis_glb_prd, s_pContext->_output_read_comreg.u8_gmv_blend_en);
		
		ROSPrintf_MEMC_IP30_interface3("[MEMC_info_3]rmv_x (%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d)\n", 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0],  s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1],	s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2],   s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4],  s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5],	s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6],  s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[10], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[13],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[20], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[21], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[22], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29],
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31]);
		
		ROSPrintf_MEMC_IP30_interface3("[MEMC_info_4]rmv_y (%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d)\n", 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[0],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[1],	s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[2],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[3], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[4],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[5],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[6],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[7], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[8],	s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9],
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[10], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[13],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[15], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[17],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[18], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19],
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[20], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[21], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[22], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[23],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[24], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[25], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[26], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[27],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[28], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[29],
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[30], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[31]);
			
		ROSPrintf_MEMC_IP30_interface4("[MEMC_info_5]rmv_cnt (%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d)\n", 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[0],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[1],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[2],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[3], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[4],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[5],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[6],	s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[7], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[8],  s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[9],
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[10], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[11], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[12], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[13],s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[14], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[15], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[16], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[17],s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[18], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[19],
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[20], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[21], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[22], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[23],s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[24], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[25], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[26], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[27],s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[28], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[29],
			s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[30], s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[31]);
	
		ROSPrintf_MEMC_IP30_interface4("[MEMC_info_6]rmv_unconf (%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d)\n", 
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[0],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[1],	s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[2],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[3], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[4],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[5],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[6],  s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[7], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[8],	s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[9],
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[10], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[11], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[12], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[13],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[14], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[15], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[16], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[17],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[18], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[19],
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[20], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[21], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[22], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[23],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[24], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[25], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[26], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[27],s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[28], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[29],
			s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[30], s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[31]);
	
		ROSPrintf_MEMC_IP30_interface5("[MEMC_info_7]rDTL (%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d)\n", 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[0],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[1],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[2],	s_pContext->_output_read_comreg.u20_me_rDTL_rb[3], s_pContext->_output_read_comreg.u20_me_rDTL_rb[4],  s_pContext->_output_read_comreg.u20_me_rDTL_rb[5],  s_pContext->_output_read_comreg.u20_me_rDTL_rb[6],  s_pContext->_output_read_comreg.u20_me_rDTL_rb[7], s_pContext->_output_read_comreg.u20_me_rDTL_rb[8],  s_pContext->_output_read_comreg.u20_me_rDTL_rb[9],
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[10], s_pContext->_output_read_comreg.u20_me_rDTL_rb[11], s_pContext->_output_read_comreg.u20_me_rDTL_rb[12], s_pContext->_output_read_comreg.u20_me_rDTL_rb[13],s_pContext->_output_read_comreg.u20_me_rDTL_rb[14], s_pContext->_output_read_comreg.u20_me_rDTL_rb[15], s_pContext->_output_read_comreg.u20_me_rDTL_rb[16], s_pContext->_output_read_comreg.u20_me_rDTL_rb[17],s_pContext->_output_read_comreg.u20_me_rDTL_rb[18], s_pContext->_output_read_comreg.u20_me_rDTL_rb[19],
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[20], s_pContext->_output_read_comreg.u20_me_rDTL_rb[21], s_pContext->_output_read_comreg.u20_me_rDTL_rb[22], s_pContext->_output_read_comreg.u20_me_rDTL_rb[23],s_pContext->_output_read_comreg.u20_me_rDTL_rb[24], s_pContext->_output_read_comreg.u20_me_rDTL_rb[25], s_pContext->_output_read_comreg.u20_me_rDTL_rb[26], s_pContext->_output_read_comreg.u20_me_rDTL_rb[27],s_pContext->_output_read_comreg.u20_me_rDTL_rb[28], s_pContext->_output_read_comreg.u20_me_rDTL_rb[29],
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[30], s_pContext->_output_read_comreg.u20_me_rDTL_rb[31]);
		
		ROSPrintf_MEMC_IP30_interface6("[MEMC_info_8]rAPLi (%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d)\n", 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[0],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[1],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[2],	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[3], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[4],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[5],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[6],  s_pContext->_output_read_comreg.u20_me_rAPLi_rb[7], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[8],	s_pContext->_output_read_comreg.u20_me_rAPLi_rb[9],
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[11], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[15], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[19],
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[23],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[27],s_pContext->_output_read_comreg.u20_me_rAPLi_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[29],
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[31]);
	
		ROSPrintf_MEMC_IP30_interface6("[MEMC_info_9]rAPLp (%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d),(%d,%d,%d,%d,%d,%d,%d,%d)\n", 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[0], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[1], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[2], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[3], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[4],  s_pContext->_output_read_comreg.u20_me_rAPLp_rb[5],  s_pContext->_output_read_comreg.u20_me_rAPLp_rb[6],  s_pContext->_output_read_comreg.u20_me_rAPLp_rb[7], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[8],	s_pContext->_output_read_comreg.u20_me_rAPLp_rb[9],
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[11], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[13],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[15], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[19],
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[23],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[27],s_pContext->_output_read_comreg.u20_me_rAPLp_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[29],
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[31]);
	}
}	

#ifdef BG_MV_GET
VOID MEMC_BG_MV_Get(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)//for gmv adjustment.
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	signed short s11_gmv_mvx = (s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
	signed short s10_gmv_mvy = (s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);

	static UINT32 GMV_adjust_by_apl_Path_en = 1;
	static UINT32 GMV_adjust_by_apl_128_Path_en = 0;
	static UINT32 GMV_adjust_by_apl_421_Path_en = 0;
	static UINT32 GMV_adjust_by_apl_153_Path_en = 0;
	static UINT32 GMV_adjust_by_apl_robot_Path_en = 0;
	static UINT32 GMV_adjust_by_apl_299_Path_en = 0;
	static UINT32 GMV_adjust_by_apl_330_Path_en = 0;
	static UINT32 GMV_adjust_by_apl_130_Path_en = 0;
	static UINT32 GMV_adjust_by_sel_rmv_Path_en = 0;

	static UINT32 u32_RgnApl[32] = {0};
	int s11_rmv_mvx[32] = {0};
	int rmv_temp_x = 0;//for debug
	int s10_rmv_mvy[32] = {0};
	int gmv_calc_rapl_max_idx = 0;
	int gmv_calc_rapl_max_temp = 0;
	int gmv_calc_rapl_min_idx = 0;
	int gmv_calc_rapl_min_temp = 0;

	static int pre_calc_gmv_mvx = 0;
	static int pre_calc_gmv_rgn_idx = 0;
	static int frameCnt_tmp = 1;

	int reg_gmvd_th = 10;
	int cond0 = 0,cond1 = 0;
	int gmvd_calc_x = 0;
	int gmvd_calc_x_min = 255;
	int gmvd_calc_x_min_idx = 0;

	int gmv_calc_x = 0;
	int gmv_calc_y = 0;
	UINT32  nIdxX = 0;
	unsigned int swap_s1_flag = 0;
	unsigned int swap_s2_flag = 0;
	unsigned int print_final_en = 0;
	unsigned int print_detail_en = 0;

	unsigned int sel_rmv_cond1 = 0;
	unsigned int sel_rmv_cond1_1 = 0;
	unsigned int sel_rmv_cond1_2 = 0;
	unsigned int sel_rmv_cond1_3 = 0;
	unsigned int sel_rmv_cond2 = 0;
	unsigned int sel_rmv_cond2_1 = 0;
	unsigned int sel_rmv_cond2_2 = 0;
	unsigned int sel_rmv_cond2_3 = 0;
	
	unsigned int u1_RB_val;
	ReadRegister(HARDWARE_HARDWARE_26_reg, 0, 23, &u1_RB_val);
	ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_B0_reg, 0, 10, &gmv_calc_x);
	ReadRegister(KME_ME1_TOP2_KME_ME1_TOP2_B0_reg, 11, 20, &gmv_calc_x);

	print_final_en = (u1_RB_val>>23)& 0x1;
	print_detail_en = (u1_RB_val>>22)& 0x1;

	if(GMV_adjust_by_apl_Path_en)
	{
		GMV_adjust_by_apl_128_Path_en = ((u1_RB_val>>6)& 0x1);
		GMV_adjust_by_apl_421_Path_en =	((u1_RB_val>>6)& 0x1);
		GMV_adjust_by_apl_153_Path_en = ((u1_RB_val>>1)& 0x1);
		GMV_adjust_by_apl_robot_Path_en = ((u1_RB_val>>2)& 0x1);
		GMV_adjust_by_apl_299_Path_en = ((u1_RB_val>>3)& 0x1);
		GMV_adjust_by_apl_330_Path_en = ((u1_RB_val>>4)& 0x1);
		GMV_adjust_by_apl_130_Path_en = ((u1_RB_val>>5)& 0x1);
		GMV_adjust_by_sel_rmv_Path_en = ((u1_RB_val>>7)& 0x1);
		
		if(GMV_adjust_by_apl_128_Path_en || GMV_adjust_by_apl_421_Path_en)
		{
			gmv_calc_x = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16];
			gmv_calc_y = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16];
			swap_s1_flag = 1;
			if(print_detail_en)
			{
				rtd_pr_memc_emerg("func:128/421_path,apl_idx:16,gmv_idx:16,s1_flag:%d\n",swap_s1_flag);	
			}
			pOutput->s1_gmv_calc_mvx = gmv_calc_x;
			pOutput->s1_gmv_calc_mvy = gmv_calc_y;
		}

		if(GMV_adjust_by_apl_153_Path_en)
		{
			gmv_calc_x = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14];
			gmv_calc_y = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14];
			swap_s1_flag = 1;
			if(print_detail_en)
			{
				rtd_pr_memc_emerg("func:153_path,apl_idx:16,gmv_idx:14,s1_flag:%d\n",swap_s1_flag);	
			}
			pOutput->s1_gmv_calc_mvx = gmv_calc_x;
			pOutput->s1_gmv_calc_mvy = gmv_calc_y;
		}
		
		if(GMV_adjust_by_apl_robot_Path_en)
		{
			//if(frameCnt_tmp % delay_frame == 0)
			{
				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					s11_rmv_mvx[nIdxX] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nIdxX];
					s10_rmv_mvy[nIdxX] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX];
				}

				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					u32_RgnApl[nIdxX] = 0;
					u32_RgnApl[nIdxX] = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[nIdxX] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[nIdxX]+1012)/2024;
				}
				
				gmv_calc_rapl_max_temp = u32_RgnApl[8];
				gmv_calc_rapl_max_idx = 8;
							
				for (nIdxX = 8; nIdxX <= 23; nIdxX++)
				{
					if((nIdxX != 12) &&(nIdxX != 20) && (nIdxX != 21) && (nIdxX != 22))
					{
						if(u32_RgnApl[nIdxX] > gmv_calc_rapl_max_temp)
						{
							gmv_calc_rapl_max_temp = u32_RgnApl[nIdxX];
							gmv_calc_rapl_max_idx = nIdxX;
						}
					}					
				}
				
				gmv_calc_x = s11_rmv_mvx[gmv_calc_rapl_max_idx];
				gmv_calc_y = s10_rmv_mvy[gmv_calc_rapl_max_idx];
				rmv_temp_x = s11_rmv_mvx[gmv_calc_rapl_max_idx];
				swap_s1_flag = 1;
				if(frameCnt_tmp % 2 == 1)
				{
					cond1 = (abs(gmv_calc_x - pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;
					pre_calc_gmv_mvx = gmv_calc_x;
					pre_calc_gmv_rgn_idx = gmv_calc_rapl_max_idx;
							
				}
				else
				{
					cond0 = (abs(gmv_calc_x - pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;	
					pre_calc_gmv_mvx = gmv_calc_x;
					pre_calc_gmv_rgn_idx = gmv_calc_rapl_max_idx;
					
				}

				if(cond0 || cond1)
				{
					for(nIdxX=8; nIdxX<24; nIdxX++)
					{
						if((nIdxX != 12) &&(nIdxX != 18) &&(nIdxX != 20) && (nIdxX != 21) && (nIdxX != 23))
						{
							if( u32_RgnApl[nIdxX] > 90)
							{
								gmvd_calc_x = abs(pre_calc_gmv_mvx - s11_rmv_mvx[nIdxX]);
								//gmvd_calc_y = abs(pre_calc_gmv_mvy - s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX]);
								if(gmvd_calc_x_min > gmvd_calc_x)
								{
									gmvd_calc_x_min_idx = nIdxX;
								}
							}	
						}
					}
					gmv_calc_x = s11_rmv_mvx[gmvd_calc_x_min_idx];
					gmv_calc_y = s10_rmv_mvy[gmvd_calc_x_min_idx];
					swap_s2_flag = 1;
				}			
			}

			if(print_detail_en)
			{
				rtd_pr_memc_emerg("func:robot_path,apl_idx:%d,gmv_idx:%d,s1_flag:%d,s2_flag:%d\n",gmv_calc_rapl_max_idx,gmvd_calc_x_min_idx,swap_s1_flag,swap_s2_flag);	
			}
			pOutput->s1_gmv_calc_mvx = gmv_calc_x;
			pOutput->s1_gmv_calc_mvy = gmv_calc_y;

		}

		if(GMV_adjust_by_apl_299_Path_en)
		{
			//if(frameCnt_tmp % delay_frame == 0)
			{
				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					s11_rmv_mvx[nIdxX] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nIdxX];
					s10_rmv_mvy[nIdxX] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX];
				}

				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					u32_RgnApl[nIdxX] = 0;
					u32_RgnApl[nIdxX] = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[nIdxX] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[nIdxX]+1012)/2024;
				}

				
				gmv_calc_rapl_min_temp = u32_RgnApl[8];
				gmv_calc_rapl_min_idx = 8;
							
				for (nIdxX = 0; nIdxX <= 31; nIdxX++)
				{
					if((nIdxX != 1) &&(nIdxX != 2)&&(nIdxX != 11)&&(nIdxX != 22))
					{
						if(u32_RgnApl[nIdxX] < gmv_calc_rapl_min_temp)
						{
							gmv_calc_rapl_min_temp = u32_RgnApl[nIdxX];
							gmv_calc_rapl_min_idx = nIdxX;
						}
					}					
				}
				
				gmv_calc_x = s11_rmv_mvx[gmv_calc_rapl_min_idx];
				gmv_calc_y = s10_rmv_mvy[gmv_calc_rapl_max_idx];
				rmv_temp_x = s11_rmv_mvx[gmv_calc_rapl_min_idx];
				swap_s1_flag = 1;
				if(frameCnt_tmp % 2 == 1)
				{
					cond1 = (abs(gmv_calc_x - pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;
					pre_calc_gmv_mvx = gmv_calc_x;
					pre_calc_gmv_rgn_idx = gmv_calc_rapl_min_idx;		
				}
				else
				{
					cond0 = (abs(gmv_calc_x - pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;	
					pre_calc_gmv_mvx = gmv_calc_x;
					pre_calc_gmv_rgn_idx = gmv_calc_rapl_min_idx;
				}

				if(cond0 || cond1)
				{
					for(nIdxX=0; nIdxX<32; nIdxX++)
					{
						if((nIdxX != 1)&&(nIdxX != 2)&&(nIdxX != 11)&&(nIdxX != 22)&&(nIdxX != 31)&&(nIdxX != 25)&&(nIdxX != 7))
						{
							if(u32_RgnApl[nIdxX] < 80)
							{
								gmvd_calc_x = abs(pre_calc_gmv_mvx - s11_rmv_mvx[nIdxX]);
								//gmvd_calc_y = abs(pre_calc_gmv_mvy - s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX]);
								if(gmvd_calc_x_min > gmvd_calc_x)
								{
									gmvd_calc_x_min_idx = nIdxX;
								}
							}	
						}
					}
					gmv_calc_x = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[gmvd_calc_x_min_idx];
					gmv_calc_y = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[gmvd_calc_x_min_idx];	
					swap_s2_flag = 1;
				}			
			}
			if(print_detail_en)
			{
				rtd_pr_memc_emerg("func:299_path,apl_idx:%d,gmv_idx:%d,s1_flag:%d,s2_flag:%d\n",gmv_calc_rapl_min_idx,gmvd_calc_x_min_idx,swap_s1_flag,swap_s2_flag);	
			}
			pOutput->s1_gmv_calc_mvx = gmv_calc_x;
			pOutput->s1_gmv_calc_mvy = gmv_calc_y;
		}

		if(GMV_adjust_by_apl_330_Path_en)
		{
			//if(frameCnt_tmp % delay_frame == 0)
			{
				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					s11_rmv_mvx[nIdxX] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nIdxX];
					s10_rmv_mvy[nIdxX] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX];
				}

				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					u32_RgnApl[nIdxX] = 0;
					u32_RgnApl[nIdxX] = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[nIdxX] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[nIdxX]+1012)/2024;
				}

				
				gmv_calc_rapl_max_temp = u32_RgnApl[8];
				gmv_calc_rapl_max_idx = 8;
							
				for (nIdxX = 0; nIdxX <= 31; nIdxX++)
				{
					if((nIdxX != 14) &&(nIdxX != 0)&&(nIdxX != 22))
					{
						if(u32_RgnApl[nIdxX] > gmv_calc_rapl_max_temp)
						{
							gmv_calc_rapl_max_temp = u32_RgnApl[nIdxX];
							gmv_calc_rapl_max_idx = nIdxX;
						}
					}					
				}
				
				gmv_calc_x = s11_rmv_mvx[gmv_calc_rapl_max_idx];
				gmv_calc_y = s10_rmv_mvy[gmv_calc_rapl_max_idx];
				rmv_temp_x = s11_rmv_mvx[gmv_calc_rapl_max_idx];
				swap_s1_flag = 1;
				if(frameCnt_tmp % 2 == 1)
				{
					cond1 = (abs(gmv_calc_x - pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;
					pre_calc_gmv_mvx = gmv_calc_x;
					pre_calc_gmv_rgn_idx = gmv_calc_rapl_max_idx;		
				}
				else
				{
					cond0 = (abs(gmv_calc_x - pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;	
					pre_calc_gmv_mvx = gmv_calc_x;
					pre_calc_gmv_rgn_idx = gmv_calc_rapl_max_idx;
				}

				if(cond0 || cond1)
				{
					for(nIdxX=0; nIdxX<32; nIdxX++)
					{
						if((nIdxX != 14)&&(nIdxX != 0)&&(nIdxX != 22)&&(nIdxX != 10))
						{
							if(u32_RgnApl[nIdxX] > 50)
							{
								gmvd_calc_x = abs(pre_calc_gmv_mvx - s11_rmv_mvx[nIdxX]);
								//gmvd_calc_y = abs(pre_calc_gmv_mvy - s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX]);
								if(gmvd_calc_x_min > gmvd_calc_x)
								{
									gmvd_calc_x_min_idx = nIdxX;
								}
							}	
						}
					}
					gmv_calc_x = s11_rmv_mvx[gmvd_calc_x_min_idx];
					gmv_calc_y = s10_rmv_mvy[gmvd_calc_x_min_idx];
					swap_s2_flag = 1;
				}			
			}
			
			if(print_detail_en)
			{
				rtd_pr_memc_emerg("func:330_path,apl_idx:%d,gmv_idx:%d,s1_flag:%d,s2_flag:%d\n",gmv_calc_rapl_max_idx,gmvd_calc_x_min_idx,swap_s1_flag,swap_s2_flag);	
			}
			pOutput->s1_gmv_calc_mvx = gmv_calc_x;
			pOutput->s1_gmv_calc_mvy = gmv_calc_y;
		}

		if(GMV_adjust_by_apl_130_Path_en)
		{
			//if(frameCnt_tmp % delay_frame == 0)
			{
				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					s11_rmv_mvx[nIdxX] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nIdxX];
					s10_rmv_mvy[nIdxX] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX];
				}

				for(nIdxX=0; nIdxX<32; nIdxX++)
				{
					u32_RgnApl[nIdxX] = 0;
					u32_RgnApl[nIdxX] = (s_pContext->_output_read_comreg.u20_me_rAPLi_rb[nIdxX] + s_pContext->_output_read_comreg.u20_me_rAPLp_rb[nIdxX]+1012)/2024;
				}

				
				gmv_calc_rapl_min_temp = u32_RgnApl[8];
				gmv_calc_rapl_min_idx = 8;
							
				for (nIdxX = 0; nIdxX <= 31; nIdxX++)
				{
					if((nIdxX != 0) &&(nIdxX != 1) &&(nIdxX != 2)&&(nIdxX != 11)&&(nIdxX != 22)&&(nIdxX != 26)&&(nIdxX != 27)&&(nIdxX != 30))
					{
						if(u32_RgnApl[nIdxX] < gmv_calc_rapl_min_temp)
						{
							gmv_calc_rapl_min_temp = u32_RgnApl[nIdxX];
							gmv_calc_rapl_min_idx = nIdxX;
						}
					}					
				}
				
				gmv_calc_x = s11_rmv_mvx[gmv_calc_rapl_min_idx];
				gmv_calc_y = s10_rmv_mvy[gmv_calc_rapl_max_idx];
				rmv_temp_x = s11_rmv_mvx[gmv_calc_rapl_min_idx];
				swap_s1_flag = 1;
				if(frameCnt_tmp % 2 == 1)
				{
					cond1 = (abs(gmv_calc_x - pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;
					pre_calc_gmv_mvx = gmv_calc_x;
					pre_calc_gmv_rgn_idx = gmv_calc_rapl_min_idx;		
				}
				else
				{
					cond0 = (abs(gmv_calc_x - pre_calc_gmv_mvx) > reg_gmvd_th) ? 1 : 0;	
					pre_calc_gmv_mvx = gmv_calc_x;
					pre_calc_gmv_rgn_idx = gmv_calc_rapl_min_idx;
				}

				if(cond0 || cond1)
				{
					for(nIdxX=0; nIdxX<32; nIdxX++)
					{
						if((nIdxX != 0)&&(nIdxX != 1)&&(nIdxX != 2)&&(nIdxX != 11)&&(nIdxX != 22)&&(nIdxX != 31)&&(nIdxX != 25)&&(nIdxX != 7)&&(nIdxX != 26)&&(nIdxX != 27)&&(nIdxX != 30))
						{
							if( u32_RgnApl[nIdxX] > 80)
							{
								gmvd_calc_x = abs(pre_calc_gmv_mvx - s11_rmv_mvx[nIdxX]);
								//gmvd_calc_y = abs(pre_calc_gmv_mvy - s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX]);
								if(gmvd_calc_x_min > gmvd_calc_x)
								{
									gmvd_calc_x_min_idx = nIdxX;
								}
							}	
						}
					}
					gmv_calc_x = s11_rmv_mvx[gmvd_calc_x_min_idx];
					gmv_calc_y = s10_rmv_mvy[gmvd_calc_x_min_idx];
					swap_s2_flag = 1;
				}				
			}
			if(print_detail_en)
			{
				rtd_pr_memc_emerg("func:130_path,apl_idx:%d,gmv_idx:%d,s1_flag:%d,s2_flag:%d\n",gmv_calc_rapl_min_idx,gmvd_calc_x_min_idx,swap_s1_flag,swap_s2_flag);	
			}
			pOutput->s1_gmv_calc_mvx = gmv_calc_x;
			pOutput->s1_gmv_calc_mvy = gmv_calc_y;
		}		
	}
	
	if(GMV_adjust_by_sel_rmv_Path_en)
	{
		for(nIdxX=0; nIdxX<32; nIdxX++)
		{
			s11_rmv_mvx[nIdxX] = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[nIdxX];
			s10_rmv_mvy[nIdxX] = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[nIdxX];
		}
			
		sel_rmv_cond1_1 = abs(s11_gmv_mvx - s11_rmv_mvx[8])> reg_gmvd_th ? 1 : 0;
		sel_rmv_cond1_2 = abs(s11_gmv_mvx - s11_rmv_mvx[15])> reg_gmvd_th ? 1 : 0;
		sel_rmv_cond1_3 = abs(s11_rmv_mvx[8] - s11_rmv_mvx[15])< reg_gmvd_th ? 1 : 0;
		sel_rmv_cond1 = sel_rmv_cond1_1 && sel_rmv_cond1_2 && sel_rmv_cond1_3;

		sel_rmv_cond2_1 = abs(s11_gmv_mvx - s11_rmv_mvx[16])> reg_gmvd_th ? 1 : 0;
		sel_rmv_cond2_2 = abs(s11_gmv_mvx - s11_rmv_mvx[23])> reg_gmvd_th ? 1 : 0;
		sel_rmv_cond2_3 = abs(s11_rmv_mvx[16] - s11_rmv_mvx[23])< reg_gmvd_th ? 1 : 0;
		sel_rmv_cond2 = sel_rmv_cond2_1 && sel_rmv_cond2_2 && sel_rmv_cond2_3;

		if(sel_rmv_cond1)
		{
			gmv_calc_x = (s11_rmv_mvx[8] + s11_rmv_mvx[15]) / 2;
			gmv_calc_y = (s10_rmv_mvy[8] + s10_rmv_mvy[15]) / 2;		
		}
		else if(sel_rmv_cond2)
		{
			gmv_calc_x = (s11_rmv_mvx[16] + s11_rmv_mvx[23]) / 2;
			gmv_calc_y = (s10_rmv_mvy[16] + s10_rmv_mvy[23]) / 2;
		}
		else
		{
			gmv_calc_x = s11_gmv_mvx;
			gmv_calc_y = s10_gmv_mvy;
		}

		if(print_detail_en)
		{
			rtd_pr_memc_emerg("func:rmv_sel_path,sel_rmv_cond1:%d,sel_rmv_cond2:%d\n",sel_rmv_cond1,sel_rmv_cond2);	
		}
		
	}
	if(print_final_en)
	{
		rtd_pr_memc_emerg("Gmv0:(%d %d),calc:(%d %d)\n",s11_gmv_mvx,s10_gmv_mvy,gmv_calc_x,gmv_calc_y);	
	}	
}
#endif


#if 1
VOID MEMC_Motion_Info_Detect(_OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_OUTPUT_RESOLUTION output_Resolution = s_pContext->_external_data._output_resolution;
	//const _Param_By_Resolution_RimCtrl *s_pRimParam = GetRimParam(output_Resolution);

	int i = 0;

	unsigned int BOT_ratio = 0,  TOP_ratio = 0;
 	//_OUTPUT_RimCtrl
	
	unsigned int u25_rgnSad_rb = s_pContext->_output_read_comreg.u25_me_rSAD_rb[i];
	unsigned int u20_rgnDtl_rb = s_pContext->_output_read_comreg.u20_me_rDTL_rb[i];
	unsigned int u22_rTC_rb = s_pContext->_output_read_comreg.u22_me_rTC_rb[i];
	unsigned int u13_rPRD_rb = s_pContext->_output_read_comreg.u13_me_rPRD_rb[i];

	#if 1
	unsigned int u30_me_aSAD_rb2 = s_pContext->_output_read_comreg.u30_me_aSAD_rb;
	unsigned int u25_me_aDTL_rb2 = s_pContext->_output_read_comreg.u25_me_aDTL_rb;
	unsigned int u26_me_aAPLi_rb2 = s_pContext->_output_read_comreg.u26_me_aAPLi_rb;
	unsigned int u26_me_aAPLp_rb2 = s_pContext->_output_read_comreg.u26_me_aAPLp_rb;

	signed short s11_me_GMV_1st_vx_rb2 = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short s10_me_GMV_1st_vy_rb2 = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
	unsigned int u17_me_GMV_1st_cnt_rb2 = s_pContext->_output_read_comreg.u17_me_GMV_1st_cnt_rb;
	unsigned short u12_me_GMV_1st_unconf_rb2 = s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb;

	signed short s11_me_GMV_2nd_vx_rb2 = s_pContext->_output_read_comreg.s11_me_GMV_2nd_vx_rb;
	signed short s10_me_GMV_2nd_vy_rb2 = s_pContext->_output_read_comreg.s10_me_GMV_2nd_vy_rb;
	unsigned int u17_me_GMV_2nd_cnt_rb2 = s_pContext->_output_read_comreg.u17_me_GMV_2nd_cnt_rb;
	unsigned short u12_me_GMV_2nd_unconf_rb2 = s_pContext->_output_read_comreg.u12_me_GMV_2nd_unconf_rb;
	#endif
	//int rmv_x  =0;
	//int rmv_y   =0;
	//int rmv_x_delta =0;
	//nt rmv_cnt = 0;
	//int rmv_unconf = 0;
	//bool rmv_cond = 0;
	//unsigned char rFB_alpha_tmp = 0, rFB_level_tmp = 0;

	
	static unsigned char k=0;//j=0,L_To_R_Paning=0,R_To_L_Paning=0;//,ES_317_cnt=0;
	//static unsigned char j=0, k=0,L_To_R_Paning=0,R_To_L_Paning=0;//,ES_317_cnt=0;
	signed short  u10_rmv_mvy[32],u11_rmv_mvx[32];//,rmv_diff[32];
	signed short  u10_rmv_2_mvy[32],u11_rmv_2_mvx[32];

	unsigned short  rmv_1st_2nd_x_delta[32], rmv_1st_2nd_y_delta[32];
	
	signed short  u16_8to23_mean_rmv;	

	static unsigned int u32_ID_Log_en=0,u32_ID_Log_en2=0;//,ES_Test_Value=0;

	//static signed short  rmv_08_temp[120]={0};
	//static signed short  gmv_mvx_temp[120]={0};
	//static signed short  gmv_mvy_temp[120]={0};
	
	unsigned int u26_aAPLp_rb=(s_pContext->_output_read_comreg.u26_me_aAPLp_rb); //current
	//static unsigned char	ES_Flag_317_Panning_reverse=0;
	//static signed short rmv_08_x_delta=0,rmv_08_x_delta_L_to_R_cnt=0,rmv_08_x_delta_R_to_L_cnt=0; // rmv_08_x_current=0,rmv_08_x_pre=0,
	//static signed short gmv_x_delta_L_to_R_cnt=0,gmv_x_delta_R_to_L_cnt=0; // gmv_x_delta=0,rmv_08_x_current=0,rmv_08_x_pre=0,

	signed short u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	signed short u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;

	unsigned int u25_rgnSad_rb_R[32],u20_rgnDtl_rb_R[32],u22_rTC_rb_R[32],u13_rPRD_rb_R[32] ;
	static unsigned int u25_rgnSad_rb_R_avg=0,u20_rgnDtl_rb_R_avg=0,u22_rTC_rb_R_avg=0,u13_rPRD_rb_R_avg=0;
	static unsigned int u25_rgnSad_rb_R_avg_temp=0,u20_rgnDtl_rb_R_avg_temp=0,u22_rTC_rb_R_avg_temp=0,u13_rPRD_rb_R_avg_temp=0;

	static unsigned int u25_rgnSad_rb_R_32total=0,u20_rgnDtl_rb_R_32total=0,u22_rTC_rb_R_32total=0,u13_rPRD_rb_R_32total=0;
	static unsigned int  ES_test_24,u32_RP_ID=0;//,u32_RP_ID=0;
	
	//static signed short  rmv_08_temp[120]={0},rmv_20_y_temp[120]={0};
	//static signed short  gmv_mvx_temp[120]={0};
	//static signed short  gmv_mvy_temp[120]={0};
	
	//static unsigned char	ES_Flag_317_Panning_reverse=0;
	//static signed short rmv_08_x_delta=0,rmv_08_x_delta_L_to_R_cnt=0,rmv_08_x_delta_R_to_L_cnt=0; // rmv_08_x_current=0,rmv_08_x_pre=0,
	static signed short rmv_20_y_delta=0;



	
	//static signed short gmv_x_delta_L_to_R_cnt=0,gmv_x_delta_R_to_L_cnt=0; // gmv_x_delta=0,rmv_08_x_current=0,rmv_08_x_pre=0,

	//++ YE Test 20200731
	int rmv_x_delta =0;
	int rmv_cnt = 0;
	int rmv_unconf = 0;
	int rmv_2nd_cnt = 0;  
	int rmv_2nd_unconf = 0;	

	unsigned int rSCss=0, rTCss =0;


	
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 14,  14, &u32_ID_Log_en);//YE Test debug dummy register
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 24,  24, &ES_test_24);//YE Test debug dummy register
	ReadRegister(SOFTWARE1_SOFTWARE1_22_reg, 16,  16, &u32_ID_Log_en2);//YE Test debug dummy register 


	//-- YE Test 20200731




	for(i=0;i<32;i++)
		{
			u11_rmv_mvx[i]=s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
			u10_rmv_mvy[i]=s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];

			u11_rmv_2_mvx[i]=s_pContext->_output_read_comreg.s11_me_rMV_2nd_vx_rb[i];
			u10_rmv_2_mvy[i]=s_pContext->_output_read_comreg.s10_me_rMV_2nd_vy_rb[i];
			
			rmv_1st_2nd_x_delta[i]=_ABS_DIFF_(u11_rmv_2_mvx[i],u11_rmv_mvx[i]);
			rmv_1st_2nd_y_delta[i]=_ABS_DIFF_(u10_rmv_2_mvy[i],u10_rmv_mvy[i]);				
		}
	u16_8to23_mean_rmv=(u10_rmv_mvy[8]+u10_rmv_mvy[9]+u10_rmv_mvy[10]+u10_rmv_mvy[11]+u10_rmv_mvy[12]+u10_rmv_mvy[13]+u10_rmv_mvy[14]+u10_rmv_mvy[15]\
				+u10_rmv_mvy[16]+u10_rmv_mvy[17]+u10_rmv_mvy[18]+u10_rmv_mvy[19]+u10_rmv_mvy[20]+u10_rmv_mvy[21]+u10_rmv_mvy[22]+u10_rmv_mvy[23])/16;

	TOP_ratio = ((_ABS_DIFF_( s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_TOP] , s_pContext->_output_rimctrl.u12_out_resolution[_RIM_TOP]))<<10)>>(9+MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.u8_V_Divide);//h=2160:u8_V_Divide=2, h=1080:u8_V_Divide=1
	BOT_ratio = ((_ABS_DIFF_(s_pContext->_output_rimctrl.u12_out_resolution[_RIM_BOT] , s_pContext->_output_rimctrl.u12_Rim_Result[_RIM_BOT]))<<10)>>(9+MEMC_PARAM_TABLE.RIM_Param[output_Resolution].RimCtrl_Param.u8_V_Divide);

	
	for(i=0; i<32; i++)
	{

		u25_rgnSad_rb_R[i] = s_pContext->_output_read_comreg.u25_me_rSAD_rb[i];
		u20_rgnDtl_rb_R[i] = s_pContext->_output_read_comreg.u20_me_rDTL_rb[i];
 		u22_rTC_rb_R[i] = s_pContext->_output_read_comreg.u22_me_rTC_rb[i];
		u13_rPRD_rb_R[i] = s_pContext->_output_read_comreg.u13_me_rPRD_rb[i];	


		//rSCss = (s_pContext->_output_read_comreg.u22_me_rSC_rb[i] >> 9);
		//rTCss = (s_pContext->_output_read_comreg.u22_me_rTC_rb[i] >> 12);		

		rSCss = (s_pContext->_output_read_comreg.u22_me_rSC_rb[i]);
		rTCss = (s_pContext->_output_read_comreg.u22_me_rTC_rb[i]);
		if(i>0)
			rmv_x_delta = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]-s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i-1];
		else 
			rmv_x_delta=0;
		rmv_cnt 		= s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i];
		rmv_unconf		=s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] ;

		rmv_2nd_cnt        = s_pContext->_output_read_comreg.u12_me_rMV_2nd_cnt_rb[i];    //YE Test 20200731
		rmv_2nd_unconf   =  s_pContext->_output_read_comreg.u12_me_rMV_2nd_unconf_rb[i];  //YE Test 20200731

if(0) //YE Test 20200907
{
#if 0  // conflict with 18202
	      if((u25_rgnSad_rb_R[i]<200000)
		&&((u20_rgnDtl_rb_R[i]<130000)&&(u20_rgnDtl_rb_R[i]>8000))
		&&(u13_rPRD_rb_R[i]<1100)
		&&((u26_aAPLp_rb<4000000)&&(u26_aAPLp_rb>2000000))
		&&((u20_rgnDtl_rb<140000)&&(u20_rgnDtl_rb>11000))
		&&(u13_rPRD_rb<1100)
		&&((u11_gmv_mvx<70)&&(u11_gmv_mvx>10))
		&&((u10_gmv_mvy<8)&&(u10_gmv_mvy>-7))
		&&(BOT_ratio<2))
		{
		pOutput->u1_RP_035_flag=1;
		pOutput->u1_RP_ID_flag=1;
		u32_RP_ID=35;
		}
	else{
		pOutput->u1_RP_035_flag=0;
		pOutput->u1_RP_ID_flag=0;
		//u32_RP_ID=0;
		}
#endif

#if 1
	      if((u25_rgnSad_rb_R[i]<200000)
		&&((u20_rgnDtl_rb_R[i]<125000)&&(u20_rgnDtl_rb_R[i]>25000))
		&&(u13_rPRD_rb_R[i]<1000)
		&&((u26_aAPLp_rb<4060000)&&(u26_aAPLp_rb>3980000))
		&&((u20_rgnDtl_rb<88000)&&(u20_rgnDtl_rb>83000))
		&&(u13_rPRD_rb<650)
		&&((u11_gmv_mvx<2)&&(u11_gmv_mvx>-2))
		&&(u10_gmv_mvy<1)
		&&(BOT_ratio<4))
		{
		pOutput->u1_RP_053_flag=1;
		pOutput->u1_RP_ID_flag=1;
		u32_RP_ID=53;
		}
	else{
		pOutput->u1_RP_053_flag=0;
		pOutput->u1_RP_ID_flag=0;
		//u32_RP_ID=0;
		}
#endif

#if 0  // conflict with 18202
	      if((u25_rgnSad_rb_R[i]<500000)
		&&(u20_rgnDtl_rb_R[i]<2000000)
		&&(u13_rPRD_rb_R[i]<900)
		&&((u26_aAPLp_rb<4500000)&&(u26_aAPLp_rb>3000000))
		&&((u20_rgnDtl_rb<90000)&&(u20_rgnDtl_rb>8000))
		&&(u13_rPRD_rb<650)
		&&((u11_gmv_mvx<300)&&(u11_gmv_mvx>80))
		&&((u10_gmv_mvy<5)&&(u10_gmv_mvy)>-5)
		&&(BOT_ratio<2))
		{
		pOutput->u1_RP_062_1_flag=1;
		pOutput->u1_RP_ID_flag=1;
		u32_RP_ID=6201;
		}
	else{
		pOutput->u1_RP_062_1_flag=0;
		pOutput->u1_RP_ID_flag=0;
		//u32_RP_ID=0;
		}
#endif

#if 0  // conflict with 18202
	      if((u25_rgnSad_rb_R[i]<200000)
		&&((u20_rgnDtl_rb_R[i]<100000)&&(u20_rgnDtl_rb_R[i]>10000))
		&&(u13_rPRD_rb_R[i]<750)
		&&((u26_aAPLp_rb<3000000)&&(u26_aAPLp_rb>2400000))
		&&((u20_rgnDtl_rb<70000)&&(u20_rgnDtl_rb>25000))
		&&(u13_rPRD_rb<550)
		&&((u11_rmv_mvx[10]<-10)&&(u11_rmv_mvx[10]>-30))
		&&((u10_rmv_mvy[10]<8)&&(u10_rmv_mvy[10]>-10))
		&&((u11_gmv_mvx<-15)&&(u11_gmv_mvx>-30))
		&&((u10_gmv_mvy<7)&&(u10_gmv_mvy)>-6)
		&&(BOT_ratio<2))
		{
		pOutput->u1_RP_062_2_flag=1;
		pOutput->u1_RP_ID_flag=1;
		u32_RP_ID=6202;
		}
	else{
		pOutput->u1_RP_062_2_flag=0;
		pOutput->u1_RP_ID_flag=0;
		//u32_RP_ID=0;
		}
#endif

#if 1   //conflict with 364 video start
	      if((u25_rgnSad_rb_R[i]<1450000)  
		&&((u20_rgnDtl_rb_R[i]<100000)&&(u20_rgnDtl_rb_R[i]>4000))
		&&(u13_rPRD_rb_R[i]<950)
		&&((u26_aAPLp_rb<3500000)&&(u26_aAPLp_rb>2500000))
		&&((u20_rgnDtl_rb<40000)&&(u20_rgnDtl_rb>16000))
		&&(u13_rPRD_rb<250)
		&&((u11_rmv_mvx[26]<-10)&&(u11_rmv_mvx[26]>-200))
		&&((u10_rmv_mvy[26]<5)&&(u10_rmv_mvy[26]>-3))
		&&((u11_gmv_mvx<-5)&&(u11_gmv_mvx>-200))
		&&((u10_gmv_mvy<8)&&(u10_gmv_mvy)>-5)
		&&(BOT_ratio<143)&&(BOT_ratio>139))
		{
			if(s_pContext->_output_bRMV_rFB_ctrl.u8_ES_flag==0){
			pOutput->u1_RP_095_flag=1;
			pOutput->u1_RP_ID_flag=1;
			u32_RP_ID=95;
			}
		}
		else{
			pOutput->u1_RP_095_flag=0;
			pOutput->u1_RP_ID_flag=0;
			//u32_RP_ID=0;
		}
#endif

}		

		pOutput->u32_RP_ID_Show=(u32_RP_ID&0xFFF);

		
		//rtd_pr_memc_info("=========  RP_062_flag=%d ========= \n",pOutput->u1_RP_062_flag);



		#if 1
	
		if(ES_test_24==1)
			{
				if(u32_RP_ID>0){
				rtd_pr_memc_info("[YE_MEMC_RP_FLAG] , RP_ID=%d, \n",u32_RP_ID);
				}
				u32_RP_ID=0;
			}

		
		if(u32_ID_Log_en==1)
		{
		
			rtd_pr_memc_info("[YE_MEMC_RMVX],i=%d,8to23_mean_rmv=%d, rmv_00_mvx=%d,rmv_01_mvx=%d,rmv_02_mvx=%d,rmv_03_mvx=%d,rmv_04_mvx=%d ,rmv_05_mvx=%d,rmv_06_mvx=%d,rmv_07_mvx=%d,\
						rmv_08_mvx=%d,rmv_09_mvx=%d,rmv_10_mvx=%d,rmv_11_mvx=%d,rmv_12_mvx=%d ,rmv_13_mvx=%d,rmv_14_mvx=%d,rmv_15_mvx=%d,\
						rmv_16_mvx=%d,rmv_17_mvx=%d,rmv_18_mvx=%d,rmv_19_mvx=%d,rmv_20_mvx=%d ,rmv_21_mvx=%d,rmv_22_mvx=%d,rmv_23_mvx=%d,\
					     	rmv_24_mvx=%d,rmv_25_mvx=%d,rmv_26_mvx=%d,rmv_27_mvx=%d ,rmv_28_mvx=%d,rmv_29_mvx=%d,rmv_30_mvx=%d,rmv_31_mvx=%d,\n",
					     i,u16_8to23_mean_rmv,u11_rmv_mvx[0],u11_rmv_mvx[1],u11_rmv_mvx[2],u11_rmv_mvx[3],u11_rmv_mvx[4],u11_rmv_mvx[5],u11_rmv_mvx[6],u11_rmv_mvx[7],\
					     u11_rmv_mvx[8],u11_rmv_mvx[9],u11_rmv_mvx[10],u11_rmv_mvx[11],u11_rmv_mvx[12],u11_rmv_mvx[13],u11_rmv_mvx[14],u11_rmv_mvx[15],\
					     u11_rmv_mvx[16],u11_rmv_mvx[17],u11_rmv_mvx[18],u11_rmv_mvx[19],u11_rmv_mvx[20],u11_rmv_mvx[21],u11_rmv_mvx[22],u11_rmv_mvx[23],\
					     u11_rmv_mvx[24],u11_rmv_mvx[25],u11_rmv_mvx[26],u11_rmv_mvx[27],u11_rmv_mvx[28],u11_rmv_mvx[29],u11_rmv_mvx[30],u11_rmv_mvx[31]);


			rtd_pr_memc_info("[YE_MEMC_RMVY],i=%d,8to23_mean_rmv=%d, rmv_00_mvy=%d,rmv_01_mvy=%d,rmv_02_mvy=%d,rmv_03_mvy=%d,rmv_04_mvy=%d ,rmv_05_mvy=%d,rmv_06_mvy=%d,rmv_07_mvy=%d,\
						rmv_08_mvy=%d,rmv_09_mvy=%d,rmv_10_mvy=%d,rmv_11_mvy=%d,rmv_12_mvy=%d ,rmv_13_mvy=%d,rmv_14_mvy=%d,rmv_15_mvy=%d,\
						rmv_16_mvy=%d,rmv_17_mvy=%d,rmv_18_mvy=%d,rmv_19_mvy=%d,rmv_20_mvy=%d ,rmv_21_mvy=%d,rmv_22_mvy=%d,rmv_23_mvy=%d,\
					     	rmv_24_mvy=%d,rmv_25_mvy=%d,rmv_26_mvy=%d,rmv_27_mvy=%d ,rmv_28_mvy=%d,rmv_29_mvy=%d,rmv_30_mvy=%d,rmv_31_mvy=%d,rmv_20_y_delta=%d,\n",
					     i,u16_8to23_mean_rmv,u10_rmv_mvy[0],u10_rmv_mvy[1],u10_rmv_mvy[2],u10_rmv_mvy[3],u10_rmv_mvy[4],u10_rmv_mvy[5],u10_rmv_mvy[6],u10_rmv_mvy[7],\
					     u10_rmv_mvy[8],u10_rmv_mvy[9],u10_rmv_mvy[10],u10_rmv_mvy[11],u10_rmv_mvy[12],u10_rmv_mvy[13],u10_rmv_mvy[14],u10_rmv_mvy[15],\
					     u10_rmv_mvy[16],u10_rmv_mvy[17],u10_rmv_mvy[18],u10_rmv_mvy[19],u10_rmv_mvy[20],u10_rmv_mvy[21],u10_rmv_mvy[22],u10_rmv_mvy[23],\
					     u10_rmv_mvy[24],u10_rmv_mvy[25],u10_rmv_mvy[26],u10_rmv_mvy[27],u10_rmv_mvy[28],u10_rmv_mvy[29],u10_rmv_mvy[30],u10_rmv_mvy[31],rmv_20_y_delta);
			 
			 rtd_pr_memc_info("[YE_MEMC_INFO] ,i=%d,gmv_mvx=%d,gmv_mvy=%d ,rgnSad_rb_R_avg=%d, rgnDtl_rb_R_avg=%d,rTC_rb_R_avg=%d,rPRD_rb_R_avg=%d,BOT_ratio=%d,sc_true=%d,\
			 	     rgnSad_rb=%d, rgnDtl_rb=%d, rTC_rb=%d, rPRD_rb=%d, aAPLp_rb=%d,\
			 	     rmv_x_delta=%d,rmv_cnt=%d,rmv_unconf=%d,rmv_2nd_cnt=%d,rmv_2nd_unconf=%d ,rSCss=%d,rTCss=%d,rmv_1st_2nd_x_delta[i]=%d,rmv_1st_2nd_y_delta[i]=%d, b_ES_Chaos_tmp=%d,ES_rChaos_cnt_tmp[i]=%d,\
			 	     mvDiff_tmp[i] =%d ,big_relMV_cnt_tmp[i]=%d, \n"   
				    ,i,u11_gmv_mvx,u10_gmv_mvy,u25_rgnSad_rb_R_avg,u20_rgnDtl_rb_R_avg,u22_rTC_rb_R_avg,u13_rPRD_rb_R_avg,BOT_ratio, s_pContext->_output_fblevelctrl.u1_HW_sc_true, \
				     u25_rgnSad_rb,u20_rgnDtl_rb,u22_rTC_rb,u13_rPRD_rb,u26_aAPLp_rb,
				     rmv_x_delta,rmv_cnt,rmv_unconf,rmv_2nd_cnt,rmv_2nd_unconf,rSCss,rTCss,rmv_1st_2nd_x_delta[i],rmv_1st_2nd_y_delta[i],\
				     s_pContext->_output_me_sceneAnalysis.b_ES_Chaos_tmp, s_pContext->_output_me_sceneAnalysis.u8_ES_rChaos_cnt_tmp[i],\
				     s_pContext->_output_me_sceneAnalysis.mvDiff_tmp[i], s_pContext->_output_me_sceneAnalysis.big_relMV_cnt_tmp[i]);



		/*
		rtd_pr_memc_info("[YE_MEMC_REV] ,i=%d ,gmv_mvx=%d,gmv_mvy=%d,8_mvx =%d ,rmv_08_temp[119]=%d ,rmv_08_temp[0]=%d ,rmv_08_x_delta=%d ,ES_Flag_317_Panning_reverse=%d, ES_317_cnt=%d ,L_to_R_cnt=%d,R_to_L_cnt=%d ,L_To_R=%d ,R_To_L=%d, \n "
		,j,u11_gmv_mvx,u10_gmv_mvy,u11_rmv_08_mvx, rmv_08_temp[119], rmv_08_temp[0], rmv_08_x_delta, ES_Flag_317_Panning_reverse, ES_317_cnt 
		,rmv_08_x_delta_L_to_R_cnt,rmv_08_x_delta_R_to_L_cnt,L_To_R_Paning,R_To_L_Paning);
		*/
		}

		#endif


	//pre_rTcss=rTcss; //YE Test

		
	}

		if(u32_ID_Log_en2==1)
		{
		 rtd_pr_memc_info("[YE_SAD_GMV] ,aSAD_rb=%d,aDTL_rb=%d ,aAPLi_rb=%d, aAPLp_rb=%d,GMV_1st_vx=%d,GMV_1st_vy=%d ,GMV_1st_cnt=%d, GMV_1st_unconf=%d,GMV_2nd_vx=%d,GMV_2nd_vy=%d,GMV_2nd_cnt=%d,GMV_2nd_unconf=%d,\n"   
	 	,u30_me_aSAD_rb2,u25_me_aDTL_rb2,u26_me_aAPLi_rb2,u26_me_aAPLp_rb2
		,s11_me_GMV_1st_vx_rb2,s10_me_GMV_1st_vy_rb2,u17_me_GMV_1st_cnt_rb2,u12_me_GMV_1st_unconf_rb2
		,s11_me_GMV_2nd_vx_rb2,s10_me_GMV_2nd_vy_rb2,u17_me_GMV_2nd_cnt_rb2,u12_me_GMV_2nd_unconf_rb2);
		//rtd_pr_memc_info("[GMV] ,1st_vx_rb=%d,1st_vy_rb=%d \n",s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb,s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);
        }

	
	if(1) // temp mark for b8099104 bit 2 test
		{
		for(k=0;k<32;k++)
			{
				u25_rgnSad_rb_R_32total=u25_rgnSad_rb_R_32total+u25_rgnSad_rb_R[k];
				u20_rgnDtl_rb_R_32total=u20_rgnDtl_rb_R_32total+u20_rgnDtl_rb_R[k];
				u22_rTC_rb_R_32total=u22_rTC_rb_R_32total+u22_rTC_rb_R[k];
				u13_rPRD_rb_R_32total=u13_rPRD_rb_R_32total+u13_rPRD_rb_R[k];
				if(k==31)
				{
					u25_rgnSad_rb_R_avg_temp=u25_rgnSad_rb_R_32total/32;
					u20_rgnDtl_rb_R_avg_temp=u20_rgnDtl_rb_R_32total/32;
					u22_rTC_rb_R_avg_temp=u22_rTC_rb_R_32total/32;
					u13_rPRD_rb_R_avg_temp=u13_rPRD_rb_R_32total/32;
				}
				u25_rgnSad_rb_R_avg=u25_rgnSad_rb_R_avg_temp;
				u20_rgnDtl_rb_R_avg =u20_rgnDtl_rb_R_avg_temp;
				u22_rTC_rb_R_avg=u22_rTC_rb_R_avg_temp;
				u13_rPRD_rb_R_avg=u13_rPRD_rb_R_avg_temp;
				#if 1
				if(u32_ID_Log_en==1)
				{
					 rtd_pr_memc_info("[YE_MEMC_INFO_R] ,i=%d,k=%d,rgnSad_rb_R[k]=%d,rgnSad_rb_R_32total=%d,rgnSad_rb_R_avg=%d,\
					 	     rgnDtl_rb_R[k]=%d,rgnDtl_rb_R_32total=%d,rgnDtl_rb_R_avg=%d,\
					 	     rTC_rb_R[k]=%d,rTC_rb_R_32total=%d,rTC_rb_R_avg=%d,\
					 	     rPRD_rb_R[k]=%d,rPRD_rb_R_32total=%d,rPRD_rb_R_avg=%d,\n"
				 	,i,k,u25_rgnSad_rb_R[k],u25_rgnSad_rb_R_32total,u25_rgnSad_rb_R_avg
				 	,u20_rgnDtl_rb_R[k],u20_rgnDtl_rb_R_32total,u20_rgnDtl_rb_R_avg
				 	,u22_rTC_rb_R[k],u22_rTC_rb_R_32total,u22_rTC_rb_R_avg
				 	,u13_rPRD_rb_R[k],u13_rPRD_rb_R_32total,u13_rPRD_rb_R_avg);
				}
				#endif

				
			}
		}
			u25_rgnSad_rb_R_32total=0;
			u20_rgnDtl_rb_R_32total=0;
			u22_rTC_rb_R_32total=0;
			u13_rPRD_rb_R_32total=0;
			
			u25_rgnSad_rb_R_avg_temp=0;
			u20_rgnDtl_rb_R_avg_temp=0;
			u22_rTC_rb_R_avg_temp=0;
			u13_rPRD_rb_R_avg_temp=0;


	#if 0
		for(j = 0; j < 119; j++){
			rmv_08_temp[j]=rmv_08_temp[j+1];
			gmv_mvx_temp[j]=gmv_mvx_temp[j+1];
			gmv_mvy_temp[j]=gmv_mvy_temp[j+1];

		}
		rmv_08_temp[119]=u11_rmv_mvx[8];//u11_rmv_08_mvx;
		rmv_08_x_delta = rmv_08_temp[119] - rmv_08_temp[0];

		for(j = 0; j < 119; j++){
		rmv_08_temp[j]=rmv_08_temp[j+1];
		gmv_mvx_temp[j]=gmv_mvx_temp[j+1];
		gmv_mvy_temp[j]=gmv_mvy_temp[j+1];

		rmv_20_y_temp[j]=rmv_20_y_temp[j+1];
		}
		rmv_08_temp[119]=u11_rmv_mvx[8];
		rmv_08_x_delta = rmv_08_temp[119] - rmv_08_temp[0];

		rmv_20_y_temp[60]=u10_rmv_mvy[20];
		rmv_20_y_delta=rmv_20_y_temp[60]-rmv_20_y_temp[0];



		#if 1
		if(((gmv_mvx_temp[1]-gmv_mvx_temp[0])>0)&&(gmv_x_delta_L_to_R_cnt<=10)){
		gmv_x_delta_L_to_R_cnt++;
			if (gmv_x_delta_R_to_L_cnt>0)
				gmv_x_delta_R_to_L_cnt--;
		}

		if(((gmv_mvx_temp[1]-gmv_mvx_temp[0])<0)&&(gmv_x_delta_R_to_L_cnt<=10)){
		gmv_x_delta_R_to_L_cnt++;
			if(gmv_x_delta_L_to_R_cnt>0)
				gmv_x_delta_L_to_R_cnt--;
		}

			if((L_To_R_Paning==1)&&(gmv_x_delta_R_to_L_cnt>0))
				ES_Flag_317_Panning_reverse=1;
			else if((R_To_L_Paning==1)&&(gmv_x_delta_L_to_R_cnt>0))
				ES_Flag_317_Panning_reverse=1;
			else
				ES_Flag_317_Panning_reverse=0;

		
		if(gmv_x_delta_L_to_R_cnt>0)
			{
				L_To_R_Paning=1;
				gmv_x_delta_L_to_R_cnt=0;
			}
		else if(gmv_x_delta_R_to_L_cnt==1)
			{
				L_To_R_Paning=0;	
			}
		
		if(gmv_x_delta_R_to_L_cnt>0)
			{
				R_To_L_Paning=1;
				gmv_x_delta_R_to_L_cnt=0;
			}
		else if(gmv_x_delta_L_to_R_cnt==1)
			{
				R_To_L_Paning=0;	
			}
		#endif

				#if 1
					if(u32_ID_Log_en==1)
					rtd_pr_memc_info("[YE_MEMC_REV] ,j =%d ,8_mvx =%d ,rmv_08_temp[0]=%d ,rmv_08_temp[119],rmv_08_x_delta=%d ,ES_Flag_317_Panning_reverse=%d, ES_317_cnt=%d ,L_to_R_cnt=%d,R_to_L_cnt=%d ,L_To_R=%d ,R_To_L=%d, \n ",
					j, u11_rmv_mvx[8],rmv_08_temp[0],rmv_08_temp[119],  rmv_08_x_delta, ES_Flag_317_Panning_reverse 
					,rmv_08_x_delta_L_to_R_cnt,rmv_08_x_delta_R_to_L_cnt,L_To_R_Paning,R_To_L_Paning);
				#endif
	#endif
}

#endif

VOID MEMC_Still_Highfrequency_Image_Detect(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i =0;

	unsigned int dtlcnt=0,aplcnt=0,rmvcnt=0;
	unsigned int rmvmatch =32,dtlmatch=32,aplmatch=32;
	unsigned int rmvxsum=0,rmvysum=0,dtlsum=0,aplisum=0,aplpsum=0;
	unsigned int rmvxavg=0,rmvyavg=0,dtlavg=0,apliavg=0,aplpavg=0;

	unsigned int rmvxcomp=0,rmvycomp=0,dtlcomp=0,aplicomp=0,aplpcomp=0;

	unsigned int rmvxhigh=0,rmvxlow=0,rmvyhigh=0,rmvylow=0,dtlhigh=0,dtllow=0,aplihigh=0,aplilow=0,aplphigh=0,aplplow=0;
	
	unsigned int log_en=0,cnt_th=0,mv_speed_th=0,max_mv=0,log_en2=0,log_en3=0,log_en4=0,log_en5=0,log_en6=0;
	
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 14, 14, &log_en);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 15, 15, &log_en2);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 16, 16, &log_en3);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 17, 17, &log_en4);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 18, 18, &log_en5);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 19, 19, &log_en6);
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 20, 23, &cnt_th);
	
	ReadRegister(SOFTWARE2_SOFTWARE2_63_reg, 24, 27, &mv_speed_th);
	
	rmvmatch = rmvmatch - cnt_th;
	dtlmatch = dtlmatch - cnt_th;
	aplmatch = aplmatch - cnt_th;

	for (i=0; i<32; i++){
		rmvxsum += s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
		rmvysum += s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];

		dtlsum += s_pContext->_output_read_comreg.u20_me_rDTL_rb[i];
		aplisum += s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i];
		aplpsum += s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i];
	}
	rmvxavg=rmvxsum/32;
	rmvyavg=rmvysum/32;
	dtlavg=dtlsum/32;
	apliavg=aplisum/32;
	aplpavg=aplpsum/32;

	rmvxhigh=rmvxavg+(rmvxavg/20);
	rmvxlow=rmvxavg-(rmvxavg/20);

	rmvyhigh=rmvyavg+(rmvyavg/20);
	rmvylow=rmvyavg-(rmvyavg/20);

	dtlhigh=dtlavg+(dtlavg/20);
	dtllow=dtlavg-(dtlavg/20);

	aplihigh=apliavg+(apliavg/20);
	aplilow=apliavg-(apliavg/20);

	aplphigh=aplpavg+(aplpavg/20);
	aplplow=aplpavg-(aplpavg/20);

	
	for (i=0; i<32; i++){
		rmvxcomp = s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i];
		rmvycomp = s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i];
		dtlcomp = s_pContext->_output_read_comreg.u20_me_rDTL_rb[i];
		aplicomp = s_pContext->_output_read_comreg.u20_me_rAPLi_rb[i];
		aplpcomp = s_pContext->_output_read_comreg.u20_me_rAPLp_rb[i];
		max_mv=_MAX_(rmvxcomp, rmvycomp);
		
		if(rmvxhigh>=rmvxcomp && rmvxcomp >=rmvxlow && rmvyhigh>=rmvycomp && rmvycomp>=rmvylow && mv_speed_th>=max_mv)
			rmvcnt++;

		if(dtlhigh>=dtlcomp && dtlcomp >=dtllow && dtlavg>=10000)
			dtlcnt++;

		if(aplihigh>=aplicomp && aplicomp >=aplilow && aplphigh>=aplpcomp && aplpcomp>=aplplow)
			aplcnt++;
	}

	if(rmvcnt==rmvmatch && dtlcnt==dtlmatch && aplcnt==aplmatch)
		pOutput->u1_still_highfrequency_image_detect=true;
	else
		pOutput->u1_still_highfrequency_image_detect=false;


	if(log_en){//d8fc[9]
		rtd_pr_memc_emerg("MEMC_Still_Highfrequency_Image_Detect:[%d,%d,%d,%d,%d],mv_speed_th[%d],max_mv:[%d]"
		,rmvmatch,rmvcnt,dtlcnt,aplcnt,pOutput->u1_still_highfrequency_image_detect,mv_speed_th,max_mv);
	}	

	if(log_en2){
		rtd_pr_memc_emerg("[%s][s11_me_rMV_1st_vx_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[10], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[13], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[20], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[21], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[22], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27], 
			s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31]);
	}
	
	if(log_en3){
		rtd_pr_memc_emerg("[%s][s10_me_rMV_1st_vy_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[0], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[1], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[2], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[3], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[4], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[5], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[6], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[7], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[8], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[10], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[13], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[15], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[17], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[18], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[20], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[21], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[22], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[23], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[24], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[25], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[26], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[27], 
			s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[28], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[29], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[30], s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[31]);
	}

	if(log_en4){
		rtd_pr_memc_emerg("[%s][u20_me_rDTL_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[0], s_pContext->_output_read_comreg.u20_me_rDTL_rb[1], s_pContext->_output_read_comreg.u20_me_rDTL_rb[2], s_pContext->_output_read_comreg.u20_me_rDTL_rb[3], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[4], s_pContext->_output_read_comreg.u20_me_rDTL_rb[5], s_pContext->_output_read_comreg.u20_me_rDTL_rb[6], s_pContext->_output_read_comreg.u20_me_rDTL_rb[7], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[8], s_pContext->_output_read_comreg.u20_me_rDTL_rb[9], s_pContext->_output_read_comreg.u20_me_rDTL_rb[10], s_pContext->_output_read_comreg.u20_me_rDTL_rb[11], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[12], s_pContext->_output_read_comreg.u20_me_rDTL_rb[13], s_pContext->_output_read_comreg.u20_me_rDTL_rb[14], s_pContext->_output_read_comreg.u20_me_rDTL_rb[15], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[16], s_pContext->_output_read_comreg.u20_me_rDTL_rb[17], s_pContext->_output_read_comreg.u20_me_rDTL_rb[18], s_pContext->_output_read_comreg.u20_me_rDTL_rb[19], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[20], s_pContext->_output_read_comreg.u20_me_rDTL_rb[21], s_pContext->_output_read_comreg.u20_me_rDTL_rb[22], s_pContext->_output_read_comreg.u20_me_rDTL_rb[23], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[24], s_pContext->_output_read_comreg.u20_me_rDTL_rb[25], s_pContext->_output_read_comreg.u20_me_rDTL_rb[26], s_pContext->_output_read_comreg.u20_me_rDTL_rb[27], 
			s_pContext->_output_read_comreg.u20_me_rDTL_rb[28], s_pContext->_output_read_comreg.u20_me_rDTL_rb[29], s_pContext->_output_read_comreg.u20_me_rDTL_rb[30], s_pContext->_output_read_comreg.u20_me_rDTL_rb[31]);
	}

	if(log_en5){
		rtd_pr_memc_emerg("[%s][u20_me_rAPLi_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[0], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[1], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[2], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[3], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[4], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[5], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[6], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[7], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[8], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[9], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[11], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[13], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[15], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[17], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[19], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[23], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[27], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[29], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[31]);
	}

	if(log_en6){
		rtd_pr_memc_emerg("[%s][u20_me_rAPLp_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[0], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[1], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[2], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[3], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[4], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[5], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[6], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[7], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[8], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[9], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[11], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[13], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[15], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[19], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[23], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[27], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[29], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[31]);
	}

}



#if 1
VOID MEMC_Panning_Adjust_obme(const _PARAM_ME_SCENE_ANALYSIS *pParam, _OUTPUT_ME_SCENE_ANALYSIS *pOutput)
{

	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int i=0;
	int log1=0,log2=0;
	unsigned char u8_Panning_Adjust_obme_cnt=0;	
	ReadRegister(SOFTWARE3_SOFTWARE3_45_reg, 28, 28, &log1);//0xB809D9B4
	ReadRegister(SOFTWARE3_SOFTWARE3_45_reg, 29, 29, &log2);//0xB809D9B4


	for(i=0;i<32;i++)
	{
		if((i==16||i==17||i==18||i==19||i==21||i==22) && (20>=_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]))){
			u8_Panning_Adjust_obme_cnt++;
		}
		else if (_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i])>100){
			u8_Panning_Adjust_obme_cnt++;
		}
	}

	if(pOutput->u2_panning_flag!=0 && u8_Panning_Adjust_obme_cnt>=28){
		pOutput->u8_Panning_Adjust_obme=1;
	}
	else{
		pOutput->u8_Panning_Adjust_obme=0;
	}

	if(log1 && pOutput->u2_panning_flag!=0)
	{
	rtd_pr_memc_emerg("\n\r$ [vx_rb][(,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,) $\n\r",
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[0], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[1], 
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[2], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[3], 
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[4], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[5], 
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[6], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[7], 
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[8], s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[9], 
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[10],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[11],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[12],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[13],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[14],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[15],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[16],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[17],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[18],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[19],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[20],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[21],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[22],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[23],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[24],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[25],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[26],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[27],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[28],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[29],
		s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[30],s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[31] );
	rtd_pr_memc_emerg("\n\r$ [vy_rb][(,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,) $\n\r",
		s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[0],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[1],	
		s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[2],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[3],	
		s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[4],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[5], 
		s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[6],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[7],	
		s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[8],  s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[9],	
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[10],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[11],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[12],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[13],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[14],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[15],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[16],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[17],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[18],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[19],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[20],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[21],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[22],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[23],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[24],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[25],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[26],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[27],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[28],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[29],
		 s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[30],s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[31] );
	}

	if(log2 && pOutput->u2_panning_flag!=0){
		rtd_pr_memc_emerg("[%s][u20_me_rAPLi_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[0], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[1], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[2], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[3], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[4], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[5], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[6], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[7], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[8], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[9], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[11], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[13], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[15], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[17], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[19], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[23], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[27], 
			s_pContext->_output_read_comreg.u20_me_rAPLi_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[29], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLi_rb[31]);

		rtd_pr_memc_emerg("[%s][u20_me_rAPLp_rb][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,][,%d,%d,%d,%d,%d,%d,%d,%d,]\n\r", __FUNCTION__,
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[0], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[1], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[2], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[3], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[4], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[5], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[6], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[7], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[8], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[9], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[10], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[11], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[12], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[13], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[14], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[15], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[16], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[17], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[18], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[19], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[20], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[21], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[22], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[23], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[24], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[25], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[26], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[27], 
			s_pContext->_output_read_comreg.u20_me_rAPLp_rb[28], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[29], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[30], s_pContext->_output_read_comreg.u20_me_rAPLp_rb[31]);
	}
	
}
#endif




