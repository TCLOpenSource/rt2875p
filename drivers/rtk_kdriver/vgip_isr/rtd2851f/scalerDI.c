/***************************************************************************
                          scalerDI.c  -  description
                             -------------------
    begin                : Wed Dec 11 2013
    copyright           : (C) 2013 by LearnRPG
    email                : learnrpg@realtek.com
    history              : Original from auto_ma, then scalerVIP, finally scalerDI
 ***************************************************************************/

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <rtd_macro.h>

// driver header
#include <rtk_kdriver/io.h>
#include <scaler/scalerCommon.h>
//#include <kernel/scaler/scalerInfo.h>
#include "vgip_isr/scalerDI.h"
#include "vgip_isr/scalerVideo.h"
#include "vgip_isr/scalerVIP.h"
#include <rbus/vdtop_reg.h>
#include <rbus/histogram_reg.h>
#include <rbus/di_reg.h>

#include <tvscalercontrol/vip/vip_reg_def.h>
#include "tvscalercontrol/scaler/scalerstruct.h"
#include "rtk_vip_logger.h"

#ifdef VIP_DEBUG_ENABLE
  #define VIP_DEBUG_PRINTF(fmt,args...)	VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VGIP_DI_DEBUG,0,fmt,##args)
#else
  #define VIP_DEBUG_PRINTF(x,y...)
#endif


#define _DISABLE		0
#define _ENABLE		1

//extern UINT32	*reg_HistCnt;	// histogram
extern _clues *SmartPic_clue;

extern unsigned int MA_print_count;

extern Start_Print_info_t Start_Print;

static unsigned int BeerCount = 0;
static unsigned char BeerFlag = 0;
static unsigned char ZoomInCount = 0;
static unsigned char PanInCount = 0;

unsigned int pre_total_motion_sum;
unsigned int cur_total_motion_sum;
unsigned int CaveCount=0;
unsigned int soft_move=0;
unsigned int static_move=0;

static unsigned char btr_control = 2; // 0: disable, 1: enable, 2: initial value.
static unsigned char SIS_INIT = 0;
static ScalerDIGlobalStatus Smart_Integration_Status; // private variable, nobody can extern me
extern unsigned char DynamicOptimizeSystem[200];

extern _system_setting_info		 	*system_info_structure_table;
extern _RPC_system_setting_info		 	*RPC_system_info_structure_table;
//static unsigned char Mcnr_Pan =0; //0:pan disable 1:pan_enable
//static unsigned char Mcnr_V_Pan =0; //0:pan disable 1:pan_enable
int GMV_ratio=0;
static unsigned int New_Mcnr_Mv =0;

// fw film mode end ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
// R //
unsigned int R_film_StatusFrameStaticPattern=0,R_film_StatusFrameStaticPattern_H1=0,R_film_StatusFrameStaticPattern_H2=0,R_film_StatusFrameStaticPattern_H3=0;
unsigned int R_film_StatusFrameStaticPattern_V1=0,R_film_StatusFrameStaticPattern_V2=0,R_film_StatusFrameStaticPattern_V3=0,R_film_StatusFrameStaticPattern_C=0;
unsigned int R_film_StatusFramePairPattern=0,R_film_StatusFramePairPattern_H1=0,R_film_StatusFramePairPattern_H2=0,R_film_StatusFramePairPattern_H3=0;
unsigned int R_film_StatusFramePairPattern_V1=0,R_film_StatusFramePairPattern_V2=0,R_film_StatusFramePairPattern_V3=0,R_film_StatusFramePairPattern_C=0;
unsigned int R_film_StatusPrevStaticPattern=0,R_film_StatusPrevStaticPattern_H1=0,R_film_StatusPrevStaticPattern_H2=0,R_film_StatusPrevStaticPattern_H3=0;
unsigned int R_film_StatusPrevStaticPattern_V1=0,R_film_StatusPrevStaticPattern_V2=0,R_film_StatusPrevStaticPattern_V3=0,R_film_StatusPrevStaticPattern_C=0;
unsigned int R_film_StatusNextStaticPattern=0,R_film_StatusNextStaticPattern_H1=0,R_film_StatusNextStaticPattern_H2=0,R_film_StatusNextStaticPattern_H3=0;
unsigned int R_film_StatusNextStaticPattern_V1=0,R_film_StatusNextStaticPattern_V2=0,R_film_StatusNextStaticPattern_V3=0,R_film_StatusNextStaticPattern_C=0;
unsigned int R_film_StatusPrevStaticPattern2=0,R_film_StatusPrevStaticPattern2_H1=0,R_film_StatusPrevStaticPattern2_H2=0,R_film_StatusPrevStaticPattern2_H3=0;
unsigned int R_film_StatusPrevStaticPattern2_V1=0,R_film_StatusPrevStaticPattern2_V2=0,R_film_StatusPrevStaticPattern2_V3=0,R_film_StatusPrevStaticPattern2_C=0;
unsigned int R_film_StatusNextStaticPattern2=0,R_film_StatusNextStaticPattern2_H1=0,R_film_StatusNextStaticPattern2_H2=0,R_film_StatusNextStaticPattern2_H3=0;
unsigned int R_film_StatusNextStaticPattern2_V1=0,R_film_StatusNextStaticPattern2_V2=0,R_film_StatusNextStaticPattern2_V3=0,R_film_StatusNextStaticPattern2_C=0;
unsigned int R_film22_StatusNextStaticPattern=0,R_film22_StatusNextStaticPattern_H1=0,R_film22_StatusNextStaticPattern_H2=0,R_film22_StatusNextStaticPattern_H3=0;
unsigned int R_film22_StatusNextStaticPattern_V1=0,R_film22_StatusNextStaticPattern_V2=0,R_film22_StatusNextStaticPattern_V3=0,R_film22_StatusNextStaticPattern_C=0;

unsigned int R_film_index32=0,R_film_index32_H1=0,R_film_index32_H2=0,R_film_index32_H3=0,R_film_index32_V1=0,R_film_index32_V2=0,R_film_index32_V3=0,R_film_index32_C=0;
//unsigned int R_film_stillpix_count32=0,R_film_stillpix_count32_H1=0,R_film_stillpix_count32_H2=0,R_film_stillpix_count32_H3=0,R_film_stillpix_count32_V1=0,R_film_stillpix_count32_V2=0,R_film_stillpix_count32_V3=0,R_film_stillpix_count32_C=0;
unsigned char R_film_Status32Detected1=0,R_film_Status32Detected1_H1=0,R_film_Status32Detected1_H2=0,R_film_Status32Detected1_H3=0,R_film_Status32Detected1_V1=0,R_film_Status32Detected1_V2=0,R_film_Status32Detected1_V3=0,R_film_Status32Detected1_C=0;
unsigned int R_film_Status32Sequence1=0,R_film_Status32Sequence1_H1=0,R_film_Status32Sequence1_H2=0,R_film_Status32Sequence1_H3=0,R_film_Status32Sequence1_V1=0,R_film_Status32Sequence1_V2=0,R_film_Status32Sequence1_V3=0,R_film_Status32Sequence1_C=0;

unsigned int R_film_index322=0,R_film_index322_H1=0,R_film_index322_H2=0,R_film_index322_H3=0,R_film_index322_V1=0,R_film_index322_V2=0,R_film_index322_V3=0,R_film_index322_C=0;
unsigned char R_film_Status32Detected2=0,R_film_Status32Detected2_H1=0,R_film_Status32Detected2_H2=0,R_film_Status32Detected2_H3=0,R_film_Status32Detected2_V1=0,R_film_Status32Detected2_V2=0,R_film_Status32Detected2_V3=0,R_film_Status32Detected2_C=0;
unsigned int R_film_Status32Sequence2=0,R_film_Status32Sequence2_H1=0,R_film_Status32Sequence2_H2=0,R_film_Status32Sequence2_H3=0,R_film_Status32Sequence2_V1=0,R_film_Status32Sequence2_V2=0,R_film_Status32Sequence2_V3=0,R_film_Status32Sequence2_C=0;

unsigned int R_film_index22=0,R_film_index22_H1=0,R_film_index22_H2=0,R_film_index22_H3=0,R_film_index22_V1=0,R_film_index22_V2=0,R_film_index22_V3=0,R_film_index22_C=0;
//unsigned int R_film_stillpix_count22=0,R_film_stillpix_count22_H1=0,R_film_stillpix_count22_H2=0,R_film_stillpix_count22_H3=0,R_film_stillpix_count22_V1=0,R_film_stillpix_count22_V2=0,R_film_stillpix_count22_V3=0,R_film_stillpix_count22_C=0;
unsigned char R_film_Status22Detected=0,R_film_Status22Detected_H1=0,R_film_Status22Detected_H2=0,R_film_Status22Detected_H3=0,R_film_Status22Detected_V1=0,R_film_Status22Detected_V2=0,R_film_Status22Detected_V3=0,R_film_Status22Detected_C=0;
unsigned int R_film_Status22Sequence=0,R_film_Status22Sequence_H1=0,R_film_Status22Sequence_H2=0,R_film_Status22Sequence_H3=0,R_film_Status22Sequence_V1=0,R_film_Status22Sequence_V2=0,R_film_Status22Sequence_V3=0,R_film_Status22Sequence_C=0;

unsigned int R_Status32Detected1_cnt=0,R_Status32Detected1_H1_cnt=0,R_Status32Detected1_H2_cnt=0,R_Status32Detected1_H3_cnt=0;
unsigned int R_Status32Detected1_V1_cnt=0,R_Status32Detected1_V2_cnt=0,R_Status32Detected1_V3_cnt=0,R_Status32Detected1_C_cnt=0;
unsigned int R_Status32Detected2_cnt=0,R_Status32Detected2_H1_cnt=0,R_Status32Detected2_H2_cnt=0,R_Status32Detected2_H3_cnt=0;
unsigned int R_Status32Detected2_V1_cnt=0,R_Status32Detected2_V2_cnt=0,R_Status32Detected2_V3_cnt=0,R_Status32Detected2_C_cnt=0;
unsigned int R_Status22Detected_cnt=0,R_Status22Detected_H1_cnt=0,R_Status22Detected_H2_cnt=0,R_Status22Detected_H3_cnt=0;
unsigned int R_Status22Detected_V1_cnt=0,R_Status22Detected_V2_cnt=0,R_Status22Detected_V3_cnt=0,R_Status22Detected_C_cnt=0;

unsigned char R_film_StatusFilmDetected=0;
unsigned int R_film_StatusFilmSequence=0;
unsigned char R_StatusMixedFilmDetected=0;
unsigned int R_film_index=0;
unsigned char R_force_weave_flag=0;
unsigned char R_weave_prev=0;
unsigned char R_NextTopDetected=0;
unsigned char R_PrevTopDetected=0;
unsigned char R_PrevTopDetected_d=0;
unsigned char R_NextTopDetected_d=0;
unsigned int  R_TopLine=0, R_BotLine=0;
unsigned int  R_TopLine2=0, R_BotLine2=0;
unsigned int  R_TopLine3=0, R_BotLine3=0;

// L //
unsigned int L_film_StatusFrameStaticPattern=0,L_film_StatusFrameStaticPattern_H1=0,L_film_StatusFrameStaticPattern_H2=0,L_film_StatusFrameStaticPattern_H3=0;
unsigned int L_film_StatusFrameStaticPattern_V1=0,L_film_StatusFrameStaticPattern_V2=0,L_film_StatusFrameStaticPattern_V3=0,L_film_StatusFrameStaticPattern_C=0;
unsigned int L_film_StatusFramePairPattern=0,L_film_StatusFramePairPattern_H1=0,L_film_StatusFramePairPattern_H2=0,L_film_StatusFramePairPattern_H3=0;
unsigned int L_film_StatusFramePairPattern_V1=0,L_film_StatusFramePairPattern_V2=0,L_film_StatusFramePairPattern_V3=0,L_film_StatusFramePairPattern_C=0;
unsigned int L_film_StatusPrevStaticPattern=0,L_film_StatusPrevStaticPattern_H1=0,L_film_StatusPrevStaticPattern_H2=0,L_film_StatusPrevStaticPattern_H3=0;
unsigned int L_film_StatusPrevStaticPattern_V1=0,L_film_StatusPrevStaticPattern_V2=0,L_film_StatusPrevStaticPattern_V3=0,L_film_StatusPrevStaticPattern_C=0;
unsigned int L_film_StatusNextStaticPattern=0,L_film_StatusNextStaticPattern_H1=0,L_film_StatusNextStaticPattern_H2=0,L_film_StatusNextStaticPattern_H3=0;
unsigned int L_film_StatusNextStaticPattern_V1=0,L_film_StatusNextStaticPattern_V2=0,L_film_StatusNextStaticPattern_V3=0,L_film_StatusNextStaticPattern_C=0;
unsigned int L_film_StatusPrevStaticPattern2=0,L_film_StatusPrevStaticPattern2_H1=0,L_film_StatusPrevStaticPattern2_H2=0,L_film_StatusPrevStaticPattern2_H3=0;
unsigned int L_film_StatusPrevStaticPattern2_V1=0,L_film_StatusPrevStaticPattern2_V2=0,L_film_StatusPrevStaticPattern2_V3=0,L_film_StatusPrevStaticPattern2_C=0;
unsigned int L_film_StatusNextStaticPattern2=0,L_film_StatusNextStaticPattern2_H1=0,L_film_StatusNextStaticPattern2_H2=0,L_film_StatusNextStaticPattern2_H3=0;
unsigned int L_film_StatusNextStaticPattern2_V1=0,L_film_StatusNextStaticPattern2_V2=0,L_film_StatusNextStaticPattern2_V3=0,L_film_StatusNextStaticPattern2_C=0;
unsigned int L_film22_StatusNextStaticPattern=0,L_film22_StatusNextStaticPattern_H1=0,L_film22_StatusNextStaticPattern_H2=0,L_film22_StatusNextStaticPattern_H3=0;
unsigned int L_film22_StatusNextStaticPattern_V1=0,L_film22_StatusNextStaticPattern_V2=0,L_film22_StatusNextStaticPattern_V3=0,L_film22_StatusNextStaticPattern_C=0;

unsigned int L_film_index32=0,L_film_index32_H1=0,L_film_index32_H2=0,L_film_index32_H3=0,L_film_index32_V1=0,L_film_index32_V2=0,L_film_index32_V3=0,L_film_index32_C=0;
//unsigned int L_film_stillpix_count32=0,L_film_stillpix_count32_H1=0,L_film_stillpix_count32_H2=0,L_film_stillpix_count32_H3=0,L_film_stillpix_count32_V1=0,L_film_stillpix_count32_V2=0,L_film_stillpix_count32_V3=0,L_film_stillpix_count32_C=0;
unsigned char L_film_Status32Detected1=0,L_film_Status32Detected1_H1=0,L_film_Status32Detected1_H2=0,L_film_Status32Detected1_H3=0,L_film_Status32Detected1_V1=0,L_film_Status32Detected1_V2=0,L_film_Status32Detected1_V3=0,L_film_Status32Detected1_C=0;
unsigned int L_film_Status32Sequence1=0,L_film_Status32Sequence1_H1=0,L_film_Status32Sequence1_H2=0,L_film_Status32Sequence1_H3=0,L_film_Status32Sequence1_V1=0,L_film_Status32Sequence1_V2=0,L_film_Status32Sequence1_V3=0,L_film_Status32Sequence1_C=0;

unsigned int L_film_index322=0,L_film_index322_H1=0,L_film_index322_H2=0,L_film_index322_H3=0,L_film_index322_V1=0,L_film_index322_V2=0,L_film_index322_V3=0,L_film_index322_C=0;
unsigned char L_film_Status32Detected2=0,L_film_Status32Detected2_H1=0,L_film_Status32Detected2_H2=0,L_film_Status32Detected2_H3=0,L_film_Status32Detected2_V1=0,L_film_Status32Detected2_V2=0,L_film_Status32Detected2_V3=0,L_film_Status32Detected2_C=0;
unsigned int L_film_Status32Sequence2=0,L_film_Status32Sequence2_H1=0,L_film_Status32Sequence2_H2=0,L_film_Status32Sequence2_H3=0,L_film_Status32Sequence2_V1=0,L_film_Status32Sequence2_V2=0,L_film_Status32Sequence2_V3=0,L_film_Status32Sequence2_C=0;

unsigned int L_film_index22=0,L_film_index22_H1=0,L_film_index22_H2=0,L_film_index22_H3=0,L_film_index22_V1=0,L_film_index22_V2=0,L_film_index22_V3=0,L_film_index22_C=0;
//unsigned int L_film_stillpix_count22=0,L_film_stillpix_count22_H1=0,L_film_stillpix_count22_H2=0,L_film_stillpix_count22_H3=0,L_film_stillpix_count22_V1=0,L_film_stillpix_count22_V2=0,L_film_stillpix_count22_V3=0,L_film_stillpix_count22_C=0;
unsigned char L_film_Status22Detected=0,L_film_Status22Detected_H1=0,L_film_Status22Detected_H2=0,L_film_Status22Detected_H3=0,L_film_Status22Detected_V1=0,L_film_Status22Detected_V2=0,L_film_Status22Detected_V3=0,L_film_Status22Detected_C=0;
unsigned int L_film_Status22Sequence=0,L_film_Status22Sequence_H1=0,L_film_Status22Sequence_H2=0,L_film_Status22Sequence_H3=0,L_film_Status22Sequence_V1=0,L_film_Status22Sequence_V2=0,L_film_Status22Sequence_V3=0,L_film_Status22Sequence_C=0;

unsigned int L_Status32Detected1_cnt=0,L_Status32Detected1_H1_cnt=0,L_Status32Detected1_H2_cnt=0,L_Status32Detected1_H3_cnt=0;
unsigned int L_Status32Detected1_V1_cnt=0,L_Status32Detected1_V2_cnt=0,L_Status32Detected1_V3_cnt=0,L_Status32Detected1_C_cnt=0;
unsigned int L_Status32Detected2_cnt=0,L_Status32Detected2_H1_cnt=0,L_Status32Detected2_H2_cnt=0,L_Status32Detected2_H3_cnt=0;
unsigned int L_Status32Detected2_V1_cnt=0,L_Status32Detected2_V2_cnt=0,L_Status32Detected2_V3_cnt=0,L_Status32Detected2_C_cnt=0;
unsigned int L_Status22Detected_cnt=0,L_Status22Detected_H1_cnt=0,L_Status22Detected_H2_cnt=0,L_Status22Detected_H3_cnt=0;
unsigned int L_Status22Detected_V1_cnt=0,L_Status22Detected_V2_cnt=0,L_Status22Detected_V3_cnt=0,L_Status22Detected_C_cnt=0;

unsigned char L_film_StatusFilmDetected=0;
unsigned int L_film_StatusFilmSequence=0;
unsigned char L_StatusMixedFilmDetected=0;
unsigned int L_film_index=0;
unsigned char L_force_weave_flag=0;
unsigned char L_weave_prev=0;
unsigned char L_NextTopDetected=0;
unsigned char L_PrevTopDetected=0;
unsigned char L_PrevTopDetected_d=0;
unsigned char L_NextTopDetected_d=0;
unsigned int  L_TopLine=0, L_BotLine=0;
unsigned int  L_TopLine2=0, L_BotLine2=0;
unsigned int  L_TopLine3=0, L_BotLine3=0;
#endif
// fw film mode end ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char scalerVIP_DI_InArray(unsigned int array[], unsigned int length, unsigned int x)
{
	unsigned int i;

	for (i=0; i<length; i++)
	{
		if (array[i] == x)
			return 1;
	}

	return 0;
}

void scalerVIP_DI_MiddleWare_StatusInit(VIP_SOURCE_TIMING cur_vip_source)
{
	unsigned int i;

	di_im_di_active_window_control_RBUS di_active_window_control_reg;
	di_im_di_weave_window_control_RBUS di_weave_window_control_reg;
	di_im_di_rtnr_control_RBUS im_di_rtnr_control_reg;
	// hmcnr testing, this code is temporal, must remove when release.
	//di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;
	//di_di_pq_pq_cmp_pair_RBUS di_di_pq_pq_cmp_pair_reg; //henry mark
	//di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);

	im_di_rtnr_control_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);
	di_active_window_control_reg.regValue = rtd_inl(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg);
	di_weave_window_control_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);

	Smart_Integration_Status.DI_HEIGHT = ((di_weave_window_control_reg.vsize_msb<<10)+di_active_window_control_reg.vsize);
	Smart_Integration_Status.DI_WIDTH = ((di_weave_window_control_reg.hsize_msb<<11)+di_active_window_control_reg.hsize);
	Smart_Integration_Status.Mode_Progressive = im_di_rtnr_control_reg.cp_rtnr_progressive;

	if (Smart_Integration_Status.DI_HEIGHT == 0) {
		Smart_Integration_Status.DI_HEIGHT = 540;
		if (Smart_Integration_Status.SCREEN_HMCNR_STATUS.Show_Debug_MSG)
			VIP_DEBUG_PRINTF("[SIS HMCNR] Error : DI_HEIGHT is zero!!\n");
	}
	if (Smart_Integration_Status.DI_WIDTH == 0) {
		if (Smart_Integration_Status.SCREEN_HMCNR_STATUS.Show_Debug_MSG)
			VIP_DEBUG_PRINTF("[SIS HMCNR] Error : DI_WIDTH is zero!!\n");
		Smart_Integration_Status.DI_WIDTH= 1920;
	}

	if (Smart_Integration_Status.Vip_Source_Timing != cur_vip_source || SIS_INIT == 0) // only initial on vip change source
	{
		VIP_DEBUG_PRINTF("[SIS INIT] %dx%d VIP SOURCE UPDATE : %d -> %d\n", Smart_Integration_Status.DI_WIDTH, Smart_Integration_Status.DI_HEIGHT, Smart_Integration_Status.Vip_Source_Timing, cur_vip_source);

		// initial all status
		for (i=0; i<HMC_BIN_COUNT; i++)
		{
			Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[i] = 0;
			Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[i] = 0;
		}

		Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.pass_ratio = 64;
		Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.pass_couter = 6;

		Smart_Integration_Status.Vip_Source_Timing = cur_vip_source;
		SIS_INIT = 1;

/* henry mark
		di_di_pq_pq_cmp_pair_reg.regValue = rtd_inl(DI_DI_PQ_PQ_CMP_PAIR_reg);
		switch(Smart_Integration_Status.Vip_Source_Timing)
		{
			case VIP_QUALITY_CVBS_NTSC:
			case VIP_QUALITY_CVBS_PAL:
			case VIP_QUALITY_YPbPr_480I:
			case VIP_QUALITY_YPbPr_576I:
			case VIP_QUALITY_HDMI_480I:
			case VIP_QUALITY_HDMI_576I:
				//di_im_di_hmc_pan_control_reg.dummy18024660_31_24 = 0xcf; // SD disalbe all
				di_di_pq_pq_cmp_pair_reg.dummy18024c04_14_7 = 0xcf; // SD disalbe all
				break;

			case VIP_QUALITY_HDMI_1080I:
				//di_im_di_hmc_pan_control_reg.dummy18024660_31_24 = 0xcf; // (1100 1111) hmcnr_fw_en / mv_modify_en / mv_avg_count = 15
				di_di_pq_pq_cmp_pair_reg.dummy18024c04_14_7 = 0xcf;
				break;

			default:
				//di_im_di_hmc_pan_control_reg.dummy18024660_31_24 = 0x0; // SD disalbe all
				di_di_pq_pq_cmp_pair_reg.dummy18024c04_14_7 = 0; // SD disalbe all
				break;
		}
		//rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_reg, di_im_di_hmc_pan_control_reg.regValue);
		rtd_outl(DI_DI_PQ_PQ_CMP_PAIR_reg, di_di_pq_pq_cmp_pair_reg.regValue);
*/
	}
}

void scalerVIP_DI_MiddleWare_HistUpdate(unsigned int *HistCnt)
{
	unsigned char i=0;
	for (i=0; i<HISTOGRAM_SIZE; i++)
		Smart_Integration_Status.Y_Histogram[i] = HistCnt[i];
}

void scalerVIP_DI_MiddleWare_ProfileUpdate(unsigned short h_block[16], unsigned short v_block[16])
{
	unsigned char i=0;
	for (i=0; i<PROFILE_SIZE; i++)
	{
		Smart_Integration_Status.Y_H_Block[i] = h_block[i];
		Smart_Integration_Status.Y_V_Block[i] = v_block[i];
	}
}

void scalerVIP_DI_MiddleWare_StatusUpdate(void)
{
	if (Smart_Integration_Status.Frame_Couter > 10000)
		Smart_Integration_Status.Frame_Couter = 0;
	else
		Smart_Integration_Status.Frame_Couter++;

	scalerVIP_DI_MOTION_StatusUpdate();
	scalerVIP_DI_HMC_StatusUpdate();
	scalerVIP_DI_HMC_Continue();
	scalerVIP_DI_PAN_HistSwitch();
	scalerVIP_DI_PAN_StatusUpdate();
	scalerVIP_DI_VPAN_StatusUpdate();
	scalerVIP_DI_SPAN_StatusUpdate();
// chen 170522
//	scalerVIP_DI_ZOOMMOTION_StatusUpdate();
//	scalerVIP_NOISE_LEVEL_StatusUpdate();
// end chen 170522
	//scalerVIP_DI_SLOWMOTION_StatusUpdate(); //rord.tsao mark 2015/07/10
	scalerVIP_DI_SCENECHANGE_StatusUpdate();
	//scalerVIP_DI_FADING_StatusUpdate();
	scalerVIP_DI_HIGHFREQUENCY_StatusUpdate();
	scalerVIP_DI_FILM_StatusUpdate();
	scalerVIP_DI_SMD_StautsUpdate();
}

unsigned int scalerVIP_DI_MiddleWare_GetStatus(SIS_STATUS access)
{
	unsigned int status = 0;

	if (access == STATUS_MOTION)
	{
		status = Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL;
	}
	else if (access == STATUS_HMC)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.HMC_detect;
	}
	else if (access == STATUS_HMC_HIST)
	{
		status = Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag;
	}
	else if (access == STATUS_PAN)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect;
	}
	else if (access == STATUS_PAN_STR)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str;
	}
	else if (access == STATUS_PAN_LEVEL)
	{
		if (Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str == 1)
			status = Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[0];//Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[0]; // return 16 = mv zero
		else
			status = 16;
	}
	else if (access == STATUS_VPAN)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.VPAN_detect;
	}
	else if (access == STATUS_SPAN)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.SPAN_detect;
	}
	else if (access == STATUS_HMCNR)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.HMCNR_detect;
	}
	else if (access == STATUS_ZOOMMOTION)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.ZOOMMOTION_detect;
	}
	else if (access == STATUS_SLOWMOTION)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.SLOWMOTION_detect;
	}
	else if (access == STATUS_SCENECHANGE)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.SCENECHANGE_detect;
	}
	else if (access == STATUS_FADING)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.FADING_detect;
	}
	else if (access == STATUS_HIGHFREQUENCY)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.HIGHFREQUENCY_detect;
	}
	else if (access == STATUS_HIGHFREQUENCY_LEVEL)
	{
		status = Smart_Integration_Status.SCREEN_HIGHFREQUENCY_STATUS.Pass_Result_FIR;
	}
	else if (access == STATUS_FILM)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.FILM_detect;
	}
	else if (access == STATUS_BLACKWHITE)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.BLACKWHITE_detect;
	}
	else if (access == STATUS_SATURATION)
	{
		status = Smart_Integration_Status.SCREEN_ACTION_STATUS.SATURATION_detect;
	}
	else if (access == STATUS_NOISE)
	{
		status = MAX(Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.RESULT_NOISE_LEVEL, Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.RESULT_NOISE_LEVEL);
		status = MAX(Smart_Integration_Status.SCREEN_NOISE_STATUS.VD_SIGNAL_NOISE_LEVEL, status);
	}
	else if (access == STATUS_CONTENT_SPATIAL_NOISE_LEVEL)
	{
		status = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.RESULT_NOISE_LEVEL;
	}
	else if (access == STATUS_CONTENT_SPATIAL_NOISE_LEVEL_RELIABLE)
	{
		status = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.RESULT_NOISE_LEVEL_RELIABLE;
	}
	else if (access == STATUS_CONTENT_TEMPORAL_NOISE_LEVEL)
	{
		status = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.RESULT_NOISE_LEVEL;
	}
	else if (access == STATUS_CONTENT_TEMPORAL_NOISE_RELIABLE)
	{
		status = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.RESULT_NOISE_RELIABLE;
	}
	else if (access == STATUS_STILL_CONCENTRIC_CIRCLES)
	{
		status = Smart_Integration_Status.SCREEN_SPECIAL_STATUS.STILL_CONCENTRIC_CIRCLES_detect;
	}
	else if (access == STATUS_CUSTOMER_HMC_VMC)
	{
		status = Smart_Integration_Status.SCREEN_SPECIAL_STATUS.CUSTOMER_HMC_VMC_detect;
	}
	else if (access == STATUS_MOVING_CAN)
	{
		status = Smart_Integration_Status.SCREEN_SPECIAL_STATUS.MOVING_CAN_detect;
	}

	else if (access == STATUS_PAN_SPECIAL_CASE)
	{
		status = Smart_Integration_Status.SCREEN_HPAN_STATUS.Pen_special_case;
	}




	return status;
}

void scalerVIP_DI_MiddleWare_SetStatus(SIS_STATUS access, unsigned char status)
{
	if (access == STATUS_FADING)
	{
		Smart_Integration_Status.SCREEN_ACTION_STATUS.FADING_detect = status;
	}
	else if (access == STATUS_STILL_CONCENTRIC_CIRCLES)
	{
		Smart_Integration_Status.SCREEN_SPECIAL_STATUS.STILL_CONCENTRIC_CIRCLES_detect = status;
	}
	else if (access == STATUS_CUSTOMER_HMC_VMC)
	{
		Smart_Integration_Status.SCREEN_SPECIAL_STATUS.CUSTOMER_HMC_VMC_detect = status;
	}
	else if (access == STATUS_MOVING_CAN)
	{
		Smart_Integration_Status.SCREEN_SPECIAL_STATUS.MOVING_CAN_detect = status;
	}
}

void scalerVIP_DI_MiddleWare_DebugMode(SIS_STATUS access, unsigned char timer)
{
	if (access == STATUS_MOTION)
	{
		if (timer != 0 && Smart_Integration_Status.Frame_Couter % timer == 0)
		{
			Smart_Integration_Status.SCREEN_MOTION_STATUS.Show_Debug_MSG = 1;
		}
		else
		{
			Smart_Integration_Status.SCREEN_MOTION_STATUS.Show_Debug_MSG = 0;
		}
	}
	else if (access == STATUS_HMC)
	{
		if (timer != 0 && Smart_Integration_Status.Frame_Couter % timer == 0)
		{
			Smart_Integration_Status.SCREEN_HMC_STATUS.Show_Debug_MSG = 1;
		}
		else
		{
			Smart_Integration_Status.SCREEN_HMC_STATUS.Show_Debug_MSG = 0;
		}
	}
	else if (access == STATUS_PAN)
	{
		if (timer != 0 && Smart_Integration_Status.Frame_Couter % timer == 0)
		{
			Smart_Integration_Status.SCREEN_HPAN_STATUS.Show_Debug_MSG = 1;
		}
		else
		{
			Smart_Integration_Status.SCREEN_HPAN_STATUS.Show_Debug_MSG = 0;
		}
	}
	else if (access == STATUS_SPAN)
	{
		if (timer != 0 && Smart_Integration_Status.Frame_Couter % timer == 0)
		{
			Smart_Integration_Status.SCREEN_SPAN_STATUS.Show_Debug_MSG = 1;
		}
		else
		{
			Smart_Integration_Status.SCREEN_SPAN_STATUS.Show_Debug_MSG = 0;
		}
	}
	else if (access == STATUS_HMCNR)
	{
		if (timer != 0 && Smart_Integration_Status.Frame_Couter % timer == 0)
		{
			Smart_Integration_Status.SCREEN_HMCNR_STATUS.Show_Debug_MSG = 1;
		}
		else
		{
			Smart_Integration_Status.SCREEN_HMCNR_STATUS.Show_Debug_MSG = 0;
		}
	}
	else if (access == STATUS_NOISE)
	{
		if (timer != 0 && Smart_Integration_Status.Frame_Couter % timer == 0)
		{
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.Show_Debug_MSG = 1;
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Show_Debug_MSG = 1;
		}
		else
		{
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.Show_Debug_MSG = 0;
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Show_Debug_MSG = 0;
		}
	}
	else if (access == STATUS_SLOWMOTION)
	{
		if (timer != 0 && Smart_Integration_Status.Frame_Couter % timer == 0)
		{
			Smart_Integration_Status.SCREEN_SLOWMOTION_STATUS.Show_Debug_MSG = 1;
		}
		else
		{
			Smart_Integration_Status.SCREEN_SLOWMOTION_STATUS.Show_Debug_MSG = 0;
		}
	}
	else if (access == STATUS_SCENECHANGE)
	{
		if (timer != 0 && Smart_Integration_Status.Frame_Couter % timer == 0)
		{
			Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Show_Debug_MSG = 1;
		}
		else
		{
			Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Show_Debug_MSG = 0;
		}
	}
	else if (access == STATUS_SMD)
	{
		if (timer != 0 && Smart_Integration_Status.Frame_Couter % timer == 0)
		{
			Smart_Integration_Status.SCREEN_SMD_STATUS.Show_Debug_MSG = 1;
		}
		else
		{
			Smart_Integration_Status.SCREEN_SMD_STATUS.Show_Debug_MSG = 0;
		}
	}
}

void scalerVIP_DI_MiddleWare_HMC_Apply(void)
{
	di_im_di_hmc_statistic1_RBUS hmc_statistic1_reg;
	di_im_di_control_RBUS di_control_reg;
  di_im_di_hmc_me_refine_ctrl_2_RBUS di_hmc_me_refine_ctrl2_reg;
  unsigned char cur_source=0;

/*
	int th_start = 1500, th_remain = 0, th_step = 0, lineratio = 0;

	di_im_di_si_film_final_result_RBUS film_final_result_reg;
	film_final_result_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_FINAL_RESULT_reg);
	th_step = film_final_result_reg.dummy180240c4_31_9;

	hmc_statistic1_reg.regValue = rtd_inl(DI_IM_DI_HMC_STATISTIC1_reg);

	th_remain = Smart_Integration_Status.SCREEN_HMC_STATUS.Cur_Top5_Total-th_start;
	if (th_remain<0)
		th_remain = 0;
	lineratio = 12-(th_remain>>th_step);
	if (lineratio < 0)
		lineratio = 1;
	hmc_statistic1_reg.ma_hmc_lineratio = lineratio;
*/
	di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);
	hmc_statistic1_reg.regValue = rtd_inl(DI_IM_DI_HMC_STATISTIC1_reg);
	di_hmc_me_refine_ctrl2_reg.regValue = rtd_inl(DI_IM_DI_HMC_ME_REFINE_CTRL_2_reg);

	cur_source=(RPC_system_info_structure_table->VIP_source);

	if((cur_source == VIP_QUALITY_HDMI_576I )||(cur_source == VIP_QUALITY_YPbPr_576I)) // for 576i need more time to enter pan
	{

		if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 40 || Smart_Integration_Status.SCREEN_ACTION_STATUS.FILM_detect == 1)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 1;
			hmc_statistic1_reg.ma_hmc_lineratio = 0;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 40 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 50)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 1;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 50 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 60)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 2;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 60 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 70)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 3;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 70 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 80)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 4;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 80 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 90)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 5;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 90 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 100)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 6;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total > 100)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 7;

			if(Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str && Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag)
				hmc_statistic1_reg.ma_hmc_lineratio = 8;	//	for sony window pan	if pan teeth enable(avoid OSD teeth)->HMc must compensate all missing pixels

		}

	}
	else
	{

		if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 40 || Smart_Integration_Status.SCREEN_ACTION_STATUS.FILM_detect == 1)
		{
      			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 1;
			hmc_statistic1_reg.ma_hmc_lineratio = 0;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 40 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 50)
		{
      			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 1;//3;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 50 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 60)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 2;//6;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 60 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 70)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 4;//8;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 70 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 80)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 6;//12;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 80 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 90)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 8;//24;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total >= 90 && Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total < 100)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 10;//36;
		}
		else if (Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total > 100)
		{
			di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 0;
			hmc_statistic1_reg.ma_hmc_lineratio = 12;//48;

			if(Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str && Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag)
				hmc_statistic1_reg.ma_hmc_lineratio = 72;	//	for sony window pan	if pan teeth enable(avoid OSD teeth)->HMc must compensate all missing pixels

		}

	}
	if(DynamicOptimizeSystem[152] ==1) //for special case
	{
		di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 1;

	}
	if (Smart_Integration_Status.SCREEN_SPECIAL_STATUS.MOVING_CAN_detect != 1 &&
		Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str == 1 && (HMC_BIN_COUNT - (Smart_Integration_Status.SCREEN_HPAN_STATUS.Pan_mv))%2==0) //odd mv
		rtd_setbits(DI_IM_DI_HMC_ADJUSTABLE_reg, _BIT17);
	else
		rtd_clearbits(DI_IM_DI_HMC_ADJUSTABLE_reg, _BIT17);

	/*rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);*/ /* for WOSQRTK_10435, ip enable can't enable, cause by isr control, marked this*/
	rtd_outl(DI_IM_DI_HMC_STATISTIC1_reg, hmc_statistic1_reg.regValue);
	rtd_outl(DI_IM_DI_HMC_ME_REFINE_CTRL_2_reg, di_hmc_me_refine_ctrl2_reg.regValue);

	if (Smart_Integration_Status.SCREEN_HMC_STATUS.Show_Debug_MSG)
		VIP_DEBUG_PRINTF("[SIS HMC] hmc_pass_total: %d\n", Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total);
}

unsigned char fw_mv[32];
void scalerVIP_DI_MiddleWare_PAN_Apply(void)
{
	char hpan_pass_total = 0;
	unsigned char i = 0;
	unsigned char cur_source=0,sd_input=0;
	int pre_mv_avg = 0;
	unsigned char mv_avg_count;
	unsigned char mv_modify_en;

	di_im_di_pan_detection_control_1_RBUS di_pan_detection_control_1_reg;
	di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;
	di_im_di_pan_detection_control_2_RBUS di_im_di_pan_detection_control_2_reg;
	di_im_di_ma_frame_motion_th_a_RBUS di_ma_frame_motion_th_a_reg;
	di_im_di_ma_frame_motion_th_b_RBUS di_ma_frame_motion_th_b_reg;
	di_im_di_ma_frame_motion_th_c_RBUS di_ma_frame_motion_th_c_reg;
	// henry merlin3 tmp
	di_im_di_rtnr_refined_ctrl_RBUS di_im_di_rtnr_refined_ctrl_reg;
	di_tnrxc_mkii_ctrl_RBUS di_tnrxc_mkii_ctrl_reg;

	di_pan_detection_control_1_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_CONTROL_1_reg);
	di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);
	di_im_di_pan_detection_control_2_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_CONTROL_2_reg);
	di_ma_frame_motion_th_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	di_ma_frame_motion_th_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
	di_ma_frame_motion_th_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);

	di_im_di_rtnr_refined_ctrl_reg.regValue = rtd_inl(DI_IM_DI_RTNR_REFINED_CTRL_reg);
	di_tnrxc_mkii_ctrl_reg.regValue = rtd_inl(DI_TNRXC_MkII_CTRL_reg);

	cur_source=(RPC_system_info_structure_table->VIP_source);

	if((cur_source >= VIP_QUALITY_CVBS_NTSC && cur_source <=VIP_QUALITY_YPbPr_576P)||
		(cur_source >= VIP_QUALITY_HDMI_480I && cur_source <=VIP_QUALITY_HDMI_576P)||
		(cur_source >= VIP_QUALITY_DTV_480I && cur_source <=VIP_QUALITY_DTV_576P)||
		(cur_source >= VIP_QUALITY_PVR_480I && cur_source <=VIP_QUALITY_PVR_576P)||
		(cur_source >= VIP_QUALITY_CVBS_SECAN && cur_source <=VIP_QUALITY_TV_NTSC443))
	{
		sd_input = 1;

		if (Smart_Integration_Status.SCREEN_ACTION_STATUS.VPAN_detect == 0 && Smart_Integration_Status.SCREEN_ACTION_STATUS.SPAN_detect == 0)
			di_pan_detection_control_1_reg.pan_en = 1;
		else
			di_pan_detection_control_1_reg.pan_en = 0;



	}
	else
	{
		di_pan_detection_control_1_reg.pan_en = 0;
		sd_input = 0;

		//if (Smart_Integration_Status.SCREEN_ACTION_STATUS.VPAN_detect == 0 && Smart_Integration_Status.SCREEN_ACTION_STATUS.SPAN_detect == 0)
		//	di_pan_detection_control_1_reg.pan_en = 1;
		//else
		//	di_pan_detection_control_1_reg.pan_en = 0;
	}




	rtd_outl(DI_IM_DI_PAN_DETECTION_CONTROL_1_reg, di_pan_detection_control_1_reg.regValue);

	di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector_det = (Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str & Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag);
	if (Smart_Integration_Status.SCREEN_ACTION_STATUS.HMCNR_detect == 1)
		di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector_det = 1; // for HMCNR reason, always enable, disable function use below edge control.
	//di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector = Smart_Integration_Status.SCREEN_HPAN_STATUS.Pan_mv + 1;



	//if(sd_input==1)
	//{
	//	di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector_det = Mcnr_Pan;
	//}


	if (Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str & Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag)
	{
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_h = 1;
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_l = 1;
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_n = 1;
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_r = 1;
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_v = 1;
	}
	else
	{
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_h = 0;
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_l = 0;
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_n = 0;
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_r = 0;
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_v = 0;
	}
	//	reverse HPan motion vector
	di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector = HMC_BIN_COUNT -(Smart_Integration_Status.SCREEN_HPAN_STATUS.Pan_mv);

	for (i=31; i>0; i--)
	{
		fw_mv[i] = fw_mv[i-1];
	}
	fw_mv[0] = HMC_BIN_COUNT -(Smart_Integration_Status.SCREEN_HPAN_STATUS.Pan_mv);

#if	0
	unsigned char mv_avg_count = (di_im_di_hmc_pan_control_reg.dummy18024660_31_24 & 0x3F)+1; // bit29~24
#else // henry merlin3 tmp
	mv_avg_count = di_tnrxc_mkii_ctrl_reg.dummy180242a0_7_2+1;
#endif

	if (mv_avg_count > 32)
		mv_avg_count = 32;
	for (i=0; i<mv_avg_count; i++)
	{
		pre_mv_avg+=fw_mv[i];
	}
	pre_mv_avg/=mv_avg_count;
	di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector = pre_mv_avg;

	// if mv_avg is +-6, still use +-5 mv for hmcnr brand pan case
#if 0
	unsigned char mv_modify_en = ((di_im_di_hmc_pan_control_reg.dummy18024660_31_24 & 0x40)>>6); // bit30
#else // henry merlin3 tmp
	mv_modify_en = (di_im_di_rtnr_refined_ctrl_reg.dummy180242a8_7);
#endif
	if (mv_modify_en == 1)
	{
		if (di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector == 23)
			di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector = 22;
		if (di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector == 11)
			di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector = 12;
	}

	//if((HMC_BIN_COUNT -(Smart_Integration_Status.SCREEN_HPAN_STATUS.Pan_mv))%2==0)	//	odd mv
		di_im_di_pan_detection_control_2_reg.pan_mc_edge_v = 0;
	//else
	//	di_im_di_pan_detection_control_2_reg.pan_mc_edge_v = 0; //1 rord.tsao form yl

	for (i=0; i<HPAN_PASS_COUNT; i++)
	{
		if (Smart_Integration_Status.SCREEN_HPAN_STATUS.Pass_Result_Sequence[i] == 1)
			hpan_pass_total++;
	}

	// when pattern is SD case, need vertical lowpass filter for smooth
	if (Smart_Integration_Status.Vip_Source_Timing == VIP_QUALITY_CVBS_NTSC/*Smart_Integration_Status.DI_WIDTH <= 960*/)
	{
		scalerVIP_Set_hpan_vfir(hpan_pass_total/8);
        //apply value used void scalerVIP_Dynamic_Vertical_NR(void)
	}
	New_Mcnr_Mv = di_im_di_hmc_pan_control_reg.pan_me_fw_control_motion_vector;
	rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_reg, di_im_di_hmc_pan_control_reg.regValue);
	rtd_outl(DI_IM_DI_PAN_DETECTION_CONTROL_2_reg,di_im_di_pan_detection_control_2_reg.regValue);

	// henry merlin3 tmp
	rtd_outl(DI_IM_DI_RTNR_REFINED_CTRL_reg, di_im_di_rtnr_refined_ctrl_reg.regValue);
	rtd_outl(DI_TNRXC_MkII_CTRL_reg, di_tnrxc_mkii_ctrl_reg.regValue);


#if 0  //move to sclervip rord.tsao 2015/12/1
	if(Smart_Integration_Status.SCREEN_HPAN_STATUS.Pen_special_case < 15) {
		/*rock_rau 20150922 Q-tek monoscope pan flicker issue(original setting)*/
		di_ma_frame_motion_th_a_reg.ma_framehtha = 0x04;
		di_ma_frame_motion_th_b_reg.ma_framehthb = 0x0a;
		di_ma_frame_motion_th_c_reg.ma_framehthc = 0x14;
	} else if(Smart_Integration_Status.SCREEN_HPAN_STATUS.Pen_special_case < 30) {
		di_ma_frame_motion_th_a_reg.ma_framehtha = 0x0a;
		di_ma_frame_motion_th_b_reg.ma_framehthb = 0x12;
		di_ma_frame_motion_th_c_reg.ma_framehthc = 0x16;
	}else {
		/*rock_rau 20150922 Q-tek monoscope pan flicker issue(trend weave)*/
		di_ma_frame_motion_th_a_reg.ma_framehtha = 0x14;
		di_ma_frame_motion_th_b_reg.ma_framehthb = 0x16;
		di_ma_frame_motion_th_c_reg.ma_framehthc = 0x18;
	}
	rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, di_ma_frame_motion_th_a_reg.regValue);
	rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, di_ma_frame_motion_th_b_reg.regValue);
	rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, di_ma_frame_motion_th_c_reg.regValue);
#endif
}
unsigned char pre_hmcnr_detect_status = 0;

// chen 170522
#if 0
void scalerVIP_DI_MiddleWare_HMCNR_Apply(unsigned char weight)
{
	char mv = 0, hpan_pass_total = 0;
	unsigned char i = 0, mv_index = 0;
	float ratio;

	di_im_di_rtnr_hmcnr_RBUS di_rtnr_hmcnr_reg;
	di_im_new_mcnr_control2_RBUS new_mcnr_control2_reg;
	di_rtnr_hmcnr_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_reg);
	new_mcnr_control2_reg.regValue = rtd_inl(DI_IM_NEW_MCNR_CONTROL2_reg);

//	if (Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect == 1)
		mv = Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[0]; // return 16 = mv zero
//	else
	//	mv = -1;

	mv_index = mv;

	ratio = 1920*540;
	ratio = ratio/(Smart_Integration_Status.DI_HEIGHT*Smart_Integration_Status.DI_WIDTH);
	if(Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag == 0)
	{
		if ((ratio*Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[mv_index]) > 30000)
			mv = mv-16;
		else
			mv = 0;
	}
	else
	{
		if ((ratio*Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[mv_index]) < 12500)
			mv = mv-16;
		else
			mv = 0;
	}

	if (weight > 10)
		weight = 10;

	for (i=0; i<HPAN_PASS_COUNT; i++)
	{
		if (Smart_Integration_Status.SCREEN_HPAN_STATUS.Pass_Result_Sequence[i] == 1)
			hpan_pass_total++;
	}


	switch(RPC_system_info_structure_table->VIP_source)
	{
		case VIP_QUALITY_CVBS_NTSC:
		case VIP_QUALITY_CVBS_PAL:
		case VIP_QUALITY_YPbPr_480I:
		case VIP_QUALITY_YPbPr_576I:
		case VIP_QUALITY_HDMI_480I:
		case VIP_QUALITY_HDMI_576I:

	if (hpan_pass_total < 2)
	{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 1;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
	}
	else if (hpan_pass_total < 4)
	{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 2;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
	}
	else if (hpan_pass_total < 6)
	{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 3;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
	}
	else if (hpan_pass_total < 8)
	{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 4;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
	}
	else if (hpan_pass_total < 10)
	{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 5;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
	}
	else if (hpan_pass_total < 12)
	{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 5;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 1;
	}
	else if (hpan_pass_total < 14)
	{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 5;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 2;
	}
	else
	{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 5;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 3;
	}

			break;
		case VIP_QUALITY_HDMI_1080I:

			if (hpan_pass_total < 2)
			{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 1;
				di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
			}
			else if (hpan_pass_total < 4)
			{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 2;
				di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
			}
			else if (hpan_pass_total < 6)
			{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 3;
				di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
			}
			else if (hpan_pass_total < 8)
			{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 4;
				di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
			}
			else if (hpan_pass_total < 10)
			{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 5;
				di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 0;
			}
			else if (hpan_pass_total < 12)
			{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 5;
				di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 1;
			}
			else if (hpan_pass_total < 14)
			{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 5;
				di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 2;
			}
			else
			{
				new_mcnr_control2_reg.cp_temporal_hmcnr_search_range = 5;
				di_rtnr_hmcnr_reg.cp_temporal_hmcnr_weight = 2;
			}



			break;

		default:
			break;

	}

			int pre_mv_avg = 0;
	for (i=0; i<16; i++) // pre 16 frames mv
	{
		pre_mv_avg+=(Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[i]-16);
	}
	pre_mv_avg/=16;

	pre_hmcnr_detect_status = Smart_Integration_Status.SCREEN_ACTION_STATUS.HMCNR_detect;
	di_im_di_rtnr_k_force_modify_RBUS di_rtnr_k_force_modify_reg;
	di_rtnr_k_force_modify_reg.regValue = rtd_inl(DI_IM_DI_RTNR_K_FORCE_MODIFY_reg);
	if (weight != 0 && pre_mv_avg >= -7 && pre_mv_avg <= 7 && Smart_Integration_Status.Mode_Progressive==0 && hpan_pass_total > 30 && Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL > 10)
	{
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_y_en = 1;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_follow_pan_en = 1;

		Smart_Integration_Status.SCREEN_ACTION_STATUS.HMCNR_detect = 1;
	}
	else
	{
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_y_en = 0;
		di_rtnr_hmcnr_reg.cp_temporal_hmcnr_follow_pan_en = 0;

		Smart_Integration_Status.SCREEN_ACTION_STATUS.HMCNR_detect = 0;
	}

	if (Smart_Integration_Status.SCREEN_HMCNR_STATUS.Show_Debug_MSG && pre_hmcnr_detect_status != Smart_Integration_Status.SCREEN_ACTION_STATUS.HMCNR_detect)
		VIP_DEBUG_PRINTF("[SIS HMCNR] HMCNR status change!! - current is: %d\n", Smart_Integration_Status.SCREEN_ACTION_STATUS.HMCNR_detect);

	rtd_outl(DI_IM_DI_RTNR_HMCNR_reg, di_rtnr_hmcnr_reg.regValue);
	rtd_outl(DI_IM_NEW_MCNR_CONTROL2_reg, new_mcnr_control2_reg.regValue);

	if (Smart_Integration_Status.SCREEN_HMCNR_STATUS.Show_Debug_MSG)
		VIP_DEBUG_PRINTF("[SIS HMCNR] mv: %d, mv_seq: %d, pre_mv_avg: %d, hpan_total : %d, count: [%d]%d\n", mv, Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[0], pre_mv_avg, hpan_pass_total, Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag, Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[mv_index]);
}
#endif
// end chen 170522

unsigned int scalerVIP_Cal_Idx_Ave_Hpan(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_hpan 32
	#define windowSizeBit_I_hpan 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_hpan]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_hpan;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_hpan;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_hpan;

		idx++;
		if(idx==windowSize_I_hpan)
			idx=0;
	}
	return ret;
}
unsigned int scalerVIP_Cal_Idx_Ave_Vpan(unsigned int lv_index, unsigned char isAll_Flag)
{
	#define windowSize_I_vpan 32
	#define windowSizeBit_I_vpan 5

	static unsigned char idx=0;
	static unsigned int levelIdx_arr[windowSize_I_vpan]={0};

	unsigned char i=0;
	unsigned int sum= 0, ret =0 ;

	if(isAll_Flag==1)
	{
		for(i=0;i<windowSize_I_vpan;i++)
			levelIdx_arr[i] = lv_index;
		ret = lv_index;
	}
	else
	{
		levelIdx_arr[idx] = lv_index;

		for(i=0;i<windowSize_I_vpan;i++)
			sum = sum + levelIdx_arr[i];
		ret = sum>>windowSizeBit_I_vpan;

		idx++;
		if(idx==windowSize_I_vpan)
			idx=0;
	}
	return ret;
}
extern unsigned int RTNR_MAD_count_Y_avg;

//chen 170809
unsigned char pattern_gen_flag=0;
unsigned char tower_flag=0;
unsigned char pattern_gen_pan_flag=0;
void scalerVIP_DI_MiddleWare_MCNR_Ctrl2(unsigned char weight)
{
	int Hpan_value=5;
	unsigned long GMVx=0;
	unsigned long GMVy=3;
	int maxGMV1_ctr=0;
	int GMV_ctr=0;
	INT8 hpan_pass_total = 0;
	UINT8 i = 0;
	int max_hist_h=0;
	int max_hist_h_MV=5;
	int sum_hist_h=0;
	int il=0;
	int ir=0;
	int mvx_hist_adj=0;
	int mvx_hist_adj_add=0;
	int ration_TH_H=50;
	static int Hpan_flag_n=0;
	static int Vpan_flag_n=0;
	unsigned char interlace_mode=0;

	UINT8 GMV_ratio_h = 0;
	static int GMVx1_acc =0;
	static int GMVx1_ctr_acc =0;
	static int GMVx2_acc = 0;
	static int GMVx2_ctr_acc = 0;
	int diff_th_h=0;
	int GMV_ctr_th=10;
	UINT8 temp_x=0,temp_y=0,temp_xy;

	UINT8 Pan_flag=0;
	UINT32	mv_index=0;
	unsigned char sd_input=0,cur_source=255;
	unsigned int vc_nr_MCNR_mvx_hist[12],sum=0;
	static unsigned char ratio=0;
	INT32 pre_mv_avg = 0;
	int pass_total_th=30;
	_system_setting_info* system_info_structure_table = NULL;
	unsigned short IphActWid_H=0,IpvActLen_V=0;

	di_im_di_rtnr_hmcnr_RBUS di_rtnr_hmcnr_reg;
	di_rtnr_output_clamp_RBUS di_rtnr_output_clamp_reg;
	di_im_new_mcnr_weighting_condition_RBUS di_im_new_mcnr_weighting_condition;
	di_im_new_mcnr_control_RBUS di_im_new_mcnr_control;
	di_im_new_mcnr_pan_condition_RBUS di_im_new_mcnr_pan_condition;
	di_im_new_mcnr_clamping_value_RBUS di_im_new_mcnr_clamping_value;
	di_im_di_rtnr_hmcnr_statistic_1_RBUS di_im_di_rtnr_hmcnr_statistic_1;
	di_im_di_rtnr_hmcnr_statistic_2_RBUS di_im_di_rtnr_hmcnr_statistic_2;
	di_im_di_rtnr_hmcnr_statistic_3_RBUS di_im_di_rtnr_hmcnr_statistic_3;
	di_im_di_rtnr_hmcnr_statistic_4_RBUS di_im_di_rtnr_hmcnr_statistic_4;
	di_im_di_rtnr_hmcnr_statistic_5_RBUS di_im_di_rtnr_hmcnr_statistic_5;
	di_im_di_rtnr_hmcnr_statistic_6_RBUS di_im_di_rtnr_hmcnr_statistic_6;
	di_di_smd_gmvcoeff3_RBUS init_di_smd_gmvcoeff3_reg;
	di_di_smd_sampling_gmvc_RBUS di_di_smd_sampling_gmvc;
	di_di_smd_gmvcoeff1_RBUS smd_gmvcoeff1_reg;


	system_info_structure_table = scaler_GetShare_Memory_system_setting_info_Struct();

	if(system_info_structure_table == NULL) {
		if(MA_print_count%360==0)
			ROSPrintf("Share Memory PQ data ERROR,system_info_structure_table = %p \n",system_info_structure_table);
		return;
	}
	IpvActLen_V = system_info_structure_table->I_Height;
	IphActWid_H = system_info_structure_table->I_Width;


	interlace_mode = (Smart_Integration_Status.Mode_Progressive==1)?0:1;

	cur_source=(RPC_system_info_structure_table->VIP_source);

	if(IphActWid_H <=960)
		sd_input = 1;
	else
		sd_input = 0;

	di_im_di_rtnr_hmcnr_statistic_1.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_1_reg);
	di_im_di_rtnr_hmcnr_statistic_2.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_2_reg);
	di_im_di_rtnr_hmcnr_statistic_3.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_3_reg);
	di_im_di_rtnr_hmcnr_statistic_4.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_4_reg);
	di_im_di_rtnr_hmcnr_statistic_5.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_5_reg);
	di_im_di_rtnr_hmcnr_statistic_6.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_6_reg);

	vc_nr_MCNR_mvx_hist[0]=di_im_di_rtnr_hmcnr_statistic_6.hmcnr_l5_sum;//4858[31:16]
	vc_nr_MCNR_mvx_hist[1]=di_im_di_rtnr_hmcnr_statistic_5.hmcnr_l4_sum;//4854[15:0]
	vc_nr_MCNR_mvx_hist[2]=di_im_di_rtnr_hmcnr_statistic_5.hmcnr_l3_sum;//4854[31:16]
	vc_nr_MCNR_mvx_hist[3]=di_im_di_rtnr_hmcnr_statistic_4.hmcnr_l2_sum;//4850[15:0]
	vc_nr_MCNR_mvx_hist[4]=di_im_di_rtnr_hmcnr_statistic_4.hmcnr_l1_sum;//4850[31:16]
	vc_nr_MCNR_mvx_hist[5]=di_im_di_rtnr_hmcnr_statistic_3.hmcnr_c0_sum;//484c[15:0]
	vc_nr_MCNR_mvx_hist[6]=di_im_di_rtnr_hmcnr_statistic_3.hmcnr_r1_sum;//484c[31:16]
	vc_nr_MCNR_mvx_hist[7]=di_im_di_rtnr_hmcnr_statistic_2.hmcnr_r2_sum;//4848[15:0]
	vc_nr_MCNR_mvx_hist[8]=di_im_di_rtnr_hmcnr_statistic_2.hmcnr_r3_sum;//4848[31:16]
	vc_nr_MCNR_mvx_hist[9]=di_im_di_rtnr_hmcnr_statistic_1.hmcnr_r4_sum;//4844[15:0]
	vc_nr_MCNR_mvx_hist[10]=di_im_di_rtnr_hmcnr_statistic_1.hmcnr_r5_sum;//4844[31:16]

	for(i=0; i<11; i++)
	{
	      sum=sum+vc_nr_MCNR_mvx_hist[i];
	}




	if(interlace_mode ==0)
	{
		if(sum != 0)
		      ratio = (vc_nr_MCNR_mvx_hist[5])*100/sum;

	}
	else
	{
		ratio =SmartPic_clue->HMC_bin_hist_Ratio[16]/10;

	}
	scalerVIP_DI_MiddleWare_MCNR_Set_GMV_Ratio(ratio);


	di_rtnr_hmcnr_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_HMCNR_reg);
	di_rtnr_output_clamp_reg.regValue = IoReg_Read32(DI_RTNR_OUTPUT_CLAMP_reg);
	di_im_new_mcnr_weighting_condition.regValue = IoReg_Read32(DI_IM_NEW_MCNR_WEIGHTING_CONDITION_reg);
	di_im_new_mcnr_control.regValue = IoReg_Read32(DI_IM_NEW_MCNR_CONTROL_reg);
	di_im_new_mcnr_pan_condition.regValue = IoReg_Read32(DI_IM_NEW_MCNR_PAN_CONDITION_reg);
	di_im_new_mcnr_clamping_value.regValue  = IoReg_Read32(DI_IM_NEW_MCNR_CLAMPING_VALUE_reg);

	init_di_smd_gmvcoeff3_reg.regValue = IoReg_Read32(DI_DI_SMD_GMVCoeff3_reg);
	di_di_smd_sampling_gmvc.regValue = IoReg_Read32(DI_DI_SMD_Sampling_GMVc_reg);
	smd_gmvcoeff1_reg.regValue= IoReg_Read32(DI_DI_SMD_GMVCoeff1_reg);

	GMV_ctr = smd_gmvcoeff1_reg.smd_gmv_ctr;
	maxGMV1_ctr = smd_gmvcoeff1_reg.smd_gmv1_max_ctr;
	GMVx = init_di_smd_gmvcoeff3_reg.smd_gmv1_x;
	GMVy = di_di_smd_sampling_gmvc.smd_gmvc_y;


	// find GMVx //////////////////////////
	for(i=0; i<11; i++)
	{
	      il=i-1;
	      ir=i+1;
	      if (il<0) il=1;
	      if (ir>10) ir=9;
	      if(i==4) ir=3;
	      if(i==6) il=7;

	      mvx_hist_adj = max(vc_nr_MCNR_mvx_hist[il],vc_nr_MCNR_mvx_hist[ir]);
	      if(mvx_hist_adj > vc_nr_MCNR_mvx_hist[i])
		      mvx_hist_adj = vc_nr_MCNR_mvx_hist[i];

	      if(i==5)
		      mvx_hist_adj=0;

	      mvx_hist_adj_add = mvx_hist_adj+vc_nr_MCNR_mvx_hist[i];

	      if((mvx_hist_adj_add>max_hist_h) ||(mvx_hist_adj_add==max_hist_h && abs(i-5)<abs(max_hist_h_MV-5)))
	      {
		      max_hist_h=mvx_hist_adj_add;
		      max_hist_h_MV=i;
	      }

	      sum_hist_h=sum_hist_h+vc_nr_MCNR_mvx_hist[i];
	}

	if(sum_hist_h<=0)
	      sum_hist_h=1;

	Hpan_value=max_hist_h_MV;
	GMVx = max_hist_h_MV;


	if(interlace_mode==1)
	{
	      GMVx = init_di_smd_gmvcoeff3_reg.smd_gmv1_x;

	      if(sd_input==1)
	      {
		      Hpan_value = New_Mcnr_Mv -12;//7;
		      if(((Hpan_value<=6 && Hpan_value>=4) && GMVx==3)&& (RTNR_MAD_count_Y_avg>980))//for interlace
		      Hpan_value=5;
	      }
	      else
	      {
		      Hpan_value= -(Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[0]+1-17)+5;
		      if(((Hpan_value<=6 && Hpan_value>=4) && GMVx==3))
			      Hpan_value=5;
	      }
	}


	if(GMV_ctr==0)
	      GMV_ctr=1;

	if(interlace_mode==1)
	{
	      GMV_ratio = maxGMV1_ctr*100/GMV_ctr;
	      GMV_ratio_h = GMV_ratio;
	}
	else
	{
	      GMV_ratio_h=max_hist_h*100/sum_hist_h;
	      GMV_ratio = GMV_ratio_h;
	}

	if(GMV_ratio>=100)
	      GMV_ratio=100;

	if(GMV_ratio<0)
	      GMV_ratio=0;


	if(Hpan_flag_n==1 && abs(GMVx1_acc-5)>=2)
	       diff_th_h=1;
	else
	       diff_th_h=0;



	////////// calculate GMVx ////////////
	////////// calculate GMVx1 ////////////
	if(GMVx1_ctr_acc==0 && Hpan_value!=5  && GMV_ratio_h>=ration_TH_H && GMVy==3)
	{
	      GMVx1_acc=Hpan_value;
	      GMVx1_ctr_acc=GMVx1_ctr_acc+1;
	}
	else if(GMVx1_ctr_acc>0)
	{
	      if(abs(GMVx1_acc-Hpan_value)<=diff_th_h && GMVy==3)
	      {
		      //GMVx1_acc=GMVx1_acc;
		      if(GMV_ratio_h>=ration_TH_H)
		      {
			      if(GMVx1_acc==Hpan_value)
				      GMVx1_ctr_acc=GMVx1_ctr_acc+1;
			      //else
				      //GMVx1_ctr_acc=GMVx1_ctr_acc;
		      }
		      else
			      GMVx1_ctr_acc=GMVx1_ctr_acc-1;
	      }
	      else
	      {
		      //GMVx1_acc=GMVx1_acc;
		      GMVx1_ctr_acc=GMVx1_ctr_acc-2;
		      if(GMVx1_ctr_acc<0)
			      GMVx1_ctr_acc=0;
	      }
	}

	////// calculate GMVx2 /////////////
	if(GMVx1_ctr_acc>=0 && (GMVx1_acc!=Hpan_value))
	{
	      if(GMVx2_ctr_acc==0 && Hpan_value!=5 && GMV_ratio_h>=ration_TH_H)
	      {
		      GMVx2_acc=Hpan_value;
		      GMVx2_ctr_acc=GMVx2_ctr_acc+1;
	      }
	      else if(GMVx2_ctr_acc>0)
	      {
		      if(abs(GMVx2_acc-Hpan_value)==0 && GMV_ratio_h>=ration_TH_H)
		      {
			      //GMVx2_acc=GMVx2_acc;
			      GMVx2_ctr_acc=GMVx2_ctr_acc+1;
		      }
		      else
		      {
			      GMVx2_acc=Hpan_value;
			      GMVx2_ctr_acc=0;
		      }
	      }
	}
	else if(GMVx2_ctr_acc>0)
	      GMVx2_ctr_acc--;


	// compare GMVx1 and GMVx2 ////
	if(GMVx2_ctr_acc>GMVx1_ctr_acc)
	{
	      GMVx1_acc=GMVx2_acc;
	      GMVx1_ctr_acc=GMVx2_ctr_acc-1;
	}
	if(GMVx2_ctr_acc<0)
	      GMVx2_ctr_acc=0;

	if(GMVx1_ctr_acc<0)
	      GMVx1_ctr_acc=0;

	//int GMV_ctr_th=10;

	if(GMVx2_ctr_acc>GMV_ctr_th)
	      GMVx2_ctr_acc=GMV_ctr_th;
	if(GMVx1_ctr_acc>GMV_ctr_th)
	      GMVx1_ctr_acc=GMV_ctr_th;

	if(GMVx1_ctr_acc>=7 || (Hpan_flag_n==1 && GMVx1_ctr_acc>=5))
	      Hpan_flag_n=1;
	else
	      Hpan_flag_n=0;



	if((interlace_mode==1)&&(sd_input ==1)) // if interlace mode & sd input
	{
		for (i=0; i<80; i++)
		{
		      if (Smart_Integration_Status.SCREEN_HPAN_STATUS.Pass_Result_Sequence[i] == 1)
			      hpan_pass_total++;
		}
		for (i=0; i<16; i++) // pre 16 frames mv
		{
		      pre_mv_avg+=(Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[i]-16);
		}
		pre_mv_avg/=16;


		if(Pan_flag==1)
		{
		       pass_total_th=10;
		}


		if((Hpan_value ==3)&&(cur_source!=VIP_QUALITY_CVBS_NTSC)&&
		      (cur_source!=VIP_QUALITY_CVBS_PAL))
		{
		      di_im_new_mcnr_pan_condition.h_pan_mv_penalty =10;

		      Hpan_value=4;
		}
		else
		{
		      di_im_new_mcnr_pan_condition.h_pan_mv_penalty =3;
		}

		di_im_new_mcnr_pan_condition.h_pan_mv_value = Hpan_value;


		if ((((pre_mv_avg >= -7 && pre_mv_avg <= 7) && hpan_pass_total > pass_total_th))&& Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL > 10)
		{
		Pan_flag=1;

		      di_im_new_mcnr_weighting_condition.mv_diff_slope = 0; // set mv diff slope
		      di_im_new_mcnr_control.n_h_pan_flag =1;
		      di_im_new_mcnr_control.n_mcnr_offset = 0;

		      if(hpan_pass_total > 16)
		      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 12;
		      else if(hpan_pass_total > 14)
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 8;
		      else if(hpan_pass_total > 12)
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 4;
		      else if(hpan_pass_total > 10)
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 0;
		      else
		      {
			      Pan_flag=0;
		      }

		}
		else
		{
		Pan_flag=0;

		      di_im_new_mcnr_control.n_mcnr_offset = 2;
		      di_im_new_mcnr_control.n_h_pan_flag =0 ;
		      di_im_new_mcnr_weighting_condition.mv_diff_slope = 2;

		      if (abs(GMVx-3)==0 && abs(GMVy-3)==0)
		      {
			      if(GMV_ratio>=95)
				      di_im_new_mcnr_control.n_mcnr_offset = 6;
			      else if(GMV_ratio>=90)
				      di_im_new_mcnr_control.n_mcnr_offset = 5;
			      else if(GMV_ratio>=85)
				      di_im_new_mcnr_control.n_mcnr_offset = 4;
			      else if(GMV_ratio>=80)
				      di_im_new_mcnr_control.n_mcnr_offset = 3;
			      else
				      di_im_new_mcnr_control.n_mcnr_offset = 1;

			      if(GMV_ratio>=90)
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 0;
			      else
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 16;
		      }
		      else
		      {
			      di_im_new_mcnr_control.n_mcnr_offset = 2;
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 16;

		      }
		}

		temp_x=abs(Hpan_value-5);//(abs(GMVx-3));
		temp_y=(abs(GMVy-3));
		temp_xy = max(temp_x,temp_y);

		if(Pan_flag==0)
		{
		      if((temp_xy==0)&&(GMV_ratio>=80))
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 0;
		      }
		      else if (temp_xy <= 1)
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 10;
		      }
		      else if (temp_xy <= 2)
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 12;
		      }
		      else if (temp_xy <= 3)
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 14;
		      }
		      else
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 16;
		      }

		}
	}
	else if((interlace_mode==1)&&(sd_input ==0)) // if interlace mode HD
	{
		for (i=0; i<120; i++)
		{
		      if (Smart_Integration_Status.SCREEN_HPAN_STATUS.Pass_Result_Sequence[i] == 1)
			      hpan_pass_total++;
		}
		for (i=0; i<16; i++) // pre 16 frames mv
		{
		      pre_mv_avg+=(Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[i]-16);
		}
		pre_mv_avg/=16;


		if(Pan_flag==1)
		{
		      pass_total_th=10;
		}

		di_im_new_mcnr_pan_condition.h_pan_mv_penalty =3;

		di_im_new_mcnr_pan_condition.h_pan_mv_value = Hpan_value;
		//di_im_new_mcnr_control3.n_mcnr_v_pan_mv = Vpan_value;

		if (((pre_mv_avg >= -7 && pre_mv_avg <= 7 && hpan_pass_total > 20)))
		{
		Pan_flag=1;

		      di_im_new_mcnr_weighting_condition.mv_diff_slope = 0; // set mv diff slope

		      di_im_new_mcnr_control.n_h_pan_flag =1;
		      di_im_new_mcnr_control.n_mcnr_offset = 0;

		      if(hpan_pass_total > 16)
		      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 16;
		      else if(hpan_pass_total > 14)
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 8;
		      else if(hpan_pass_total > 12)
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 4;
		      else if(hpan_pass_total > 10)
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 0;
		      else
		      {
			      Pan_flag=0;
		      }

		}
		else
		{
		Pan_flag=0;

		      di_im_new_mcnr_control.n_mcnr_offset = 2;
		      di_im_new_mcnr_control.n_h_pan_flag =0 ;
		      di_im_new_mcnr_weighting_condition.mv_diff_slope = 2;

		      if (abs(GMVx-3)==0 && abs(GMVy-3)==0)
		      {
			      if(GMV_ratio>=95)
				      di_im_new_mcnr_control.n_mcnr_offset = 6;
			      else if(GMV_ratio>=90)
				      di_im_new_mcnr_control.n_mcnr_offset = 5;
			      else if(GMV_ratio>=85)
				      di_im_new_mcnr_control.n_mcnr_offset = 4;
			      else if(GMV_ratio>=80)
				      di_im_new_mcnr_control.n_mcnr_offset = 3;
			      else
				      di_im_new_mcnr_control.n_mcnr_offset = 1;

			      if(GMV_ratio>=90)
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 0;
			      else
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 16;
		      }
		      else //if (abs(GMVx-3)<=1 && abs(GMVy-3)<=1)
		      {
			      di_im_new_mcnr_control.n_mcnr_offset = 1;
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 16;

		      }
		}

		temp_x=abs(Hpan_value-5);//(abs(GMVx-3));
		temp_y=(abs(GMVy-3));
		temp_xy = max(temp_x,temp_y);
		mv_index =SmartPic_clue->HMC_bin_hist_Ratio[16];

		if(Pan_flag==0)
		{
		      if((temp_xy==0)&&(GMV_ratio>=80))
		{
		      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 0;
			      di_im_new_mcnr_weighting_condition.mv_diff_slope = 2;
		}
		else if (temp_xy <= 1)
		{
			      if(GMV_ratio>=60)  // this patch for lg test disc #46
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 10;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;
			      }
			      else if (GMV_ratio>=50)
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 11;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;

			      }
			      else if(GMV_ratio>=40)
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 12;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;

			      }
			      else
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 13;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;
			      }
		}
		else if (temp_xy <= 2)
		{
			      if(GMV_ratio>=60)
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 12;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;
			      }
			      else if (GMV_ratio>=50)
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 13;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;

			      }
			      else if(GMV_ratio>=40)
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 14;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;
			      }
			      else
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 15;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;
			      }
		}
		else if (temp_xy <= 3)
		{
			      if(GMV_ratio>=60)
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 14;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;
			      }
			      else if (GMV_ratio>=50)
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 15;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;

			      }
			      else
			      {
				      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 15;
				      di_im_new_mcnr_weighting_condition.mv_diff_slope = 1;
			      }
		      }
		      else
		      {
		      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 16;
			      di_im_new_mcnr_weighting_condition.mv_diff_slope = 0;
		}
		}
	}
      	else
	{

		Hpan_flag_n =0;//k2l progressive mode ,pan info easy mistake , disable it
		Vpan_flag_n =0;//k2l progressive mode ,pan info easy mistake , disable it

		Pan_flag=0;

		di_im_new_mcnr_control.n_mcnr_offset = 2;
		di_im_new_mcnr_control.n_h_pan_flag =0 ;
		di_im_new_mcnr_weighting_condition.mv_diff_slope = 2;

		if (abs(GMVx-10)==0 && abs(GMVy-3)==0)
		{
		      if(GMV_ratio>=95)
			      di_im_new_mcnr_control.n_mcnr_offset = 6;
		      else if(GMV_ratio>=90)
			      di_im_new_mcnr_control.n_mcnr_offset = 5;
		      else if(GMV_ratio>=85)
			      di_im_new_mcnr_control.n_mcnr_offset = 4;
		      else if(GMV_ratio>=80)
			      di_im_new_mcnr_control.n_mcnr_offset = 3;
		      else
			      di_im_new_mcnr_control.n_mcnr_offset = 1;
		}
		else
		{
		      di_im_new_mcnr_control.n_mcnr_offset = 2;
		}

		if(Pan_flag==0)
		{
		      temp_x=(abs(GMVx-5));
		//      temp_y=(abs(GMVy-3));
		      temp_xy = temp_x;//max(temp_x,temp_y);

		      if((temp_xy==0)&&(GMV_ratio>=90))
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 0;
		      }
		      else if (temp_xy <= 1)
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 10;
		      }
		      else if (temp_xy <= 2)
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 12;
		      }
		      else if (temp_xy <= 3)
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 14;
		      }
		      else
		      {
			      di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 16;
		      }
		}


		if((IphActWid_H > 2048)&& (IpvActLen_V > 1080)){ 	// 4k2k
			di_im_new_mcnr_clamping_value.mcnr_weight_clamp_upth = 0;
			di_im_new_mcnr_control.n_mcnr_offset = 6;
		}
	}


	rtd_outl(DI_IM_DI_RTNR_HMCNR_reg, di_rtnr_hmcnr_reg.regValue);
	rtd_outl(DI_IM_NEW_MCNR_WEIGHTING_CONDITION_reg, di_im_new_mcnr_weighting_condition.regValue);
	// rtd_outl(DI_IM_NEW_MCNR_CONTROL3_reg, di_im_new_mcnr_control3.regValue);
	rtd_outl(DI_IM_NEW_MCNR_CONTROL_reg, di_im_new_mcnr_control.regValue);
	rtd_outl(DI_IM_NEW_MCNR_PAN_CONDITION_reg, di_im_new_mcnr_pan_condition.regValue);
	rtd_outl(DI_IM_NEW_MCNR_CLAMPING_VALUE_reg,di_im_new_mcnr_clamping_value.regValue);







}


void scalerVIP_DI_MiddleWare_MCNR_Set_GMV_Ratio(int GMV_xy_ratio)
{
	GMV_ratio = GMV_xy_ratio;
}


unsigned char scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(void)
{
	return GMV_ratio;
}
#ifdef CONFIG_HW_SUPPORT_MCNR

unsigned char scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag(void)
{
	return Mcnr_Pan;
}
void scalerVIP_DI_MiddleWare_MCNR_Set_Pan_Flag(unsigned char pan_flag)
{
	Mcnr_Pan = pan_flag;
}
unsigned char scalerVIP_DI_MiddleWare_MCNR_Get_V_Pan_Flag(void)
{
	return Mcnr_V_Pan;
}
void scalerVIP_DI_MiddleWare_MCNR_Set_V_Pan_Flag(unsigned char pan_flag)
{
	Mcnr_V_Pan = pan_flag;
}
#endif
// end chen 170809

void scalerVIP_DI_MiddleWare_FILM_Apply(void)
{
	di_im_di_film_sawtooth_filmframe_th_RBUS film_sawtooth_filmframe_th_reg;
	di_im_di_si_film_final_result_RBUS im_di_si_film_final_result_reg;

	unsigned char cur_source=(RPC_system_info_structure_table->VIP_source);
	static unsigned char pre_source =255;
	static unsigned char film_SawtoothThl_ref = 22;
	static unsigned char film_FrMotionThl_ref = 18;
	unsigned char film_SawtoothThl = 22;
	unsigned char film_FrMotionThl = 18;
	unsigned char motionMax = 28;
	unsigned char motionMin = 8;
	unsigned char motionl_level;
	unsigned char motionl_level_tmp=0;
	unsigned char motion_ratio = 0;

	film_sawtooth_filmframe_th_reg.regValue = rtd_inl(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg);
	im_di_si_film_final_result_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_FINAL_RESULT_reg);

	motion_ratio = (im_di_si_film_final_result_reg.dummy180240c4_31_10>>6) & 0xf;
	motion_ratio = (motion_ratio==0)? 4:motion_ratio;

	if(pre_source !=cur_source)
	{
		film_SawtoothThl_ref = film_sawtooth_filmframe_th_reg.film_sawtooththl;
		film_FrMotionThl_ref = film_sawtooth_filmframe_th_reg.film_frmotionthl;
	}
	pre_source = cur_source;

	motionl_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION);
	//	STATUS_MOTION, //0~48, 0:condition error 1:no motion very still, 48: very motion

	//	motion		0	motionMin	motionMax	48
	//	film ratio		0	0			100			100
	motionl_level_tmp = motionl_level;
	motionl_level_tmp = (motionl_level_tmp<motionMin)? motionMin:motionl_level_tmp;
	motionl_level_tmp = (motionl_level_tmp>motionMax)? motionMax:motionl_level_tmp;

	film_SawtoothThl = film_SawtoothThl_ref*(motionl_level_tmp-motionMin)/(motionMax-motionMin);
	film_FrMotionThl = film_FrMotionThl_ref*(motionl_level_tmp-motionMin)/(motionMax-motionMin);

	film_SawtoothThl = (film_SawtoothThl<(film_SawtoothThl_ref/motion_ratio))? (film_SawtoothThl_ref/motion_ratio):film_SawtoothThl;
	film_FrMotionThl = (film_FrMotionThl<(film_FrMotionThl_ref/motion_ratio))? (film_FrMotionThl_ref/motion_ratio):film_FrMotionThl;

	film_sawtooth_filmframe_th_reg.film_sawtooththl =  film_SawtoothThl;
	film_sawtooth_filmframe_th_reg.film_frmotionthl = film_FrMotionThl;

	rtd_outl(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg, film_sawtooth_filmframe_th_reg.regValue);

}

// chen 170522
#if 0
void scalerVIP_DI_MiddleWare_ZOOMMOTION_Apply(unsigned char offset)
{
	di_im_di_zoommotion_detector_RBUS zoommotion_detector_reg;
	di_im_di_zoommotion_det_fm_th_a_RBUS zoommotion_det_fm_th_a_reg;
	di_im_di_zoommotion_det_fm_th_b_RBUS zoommotion_det_fm_th_b_reg;
	di_im_di_zoommotion_det_fm_th_c_RBUS zoommotion_det_fm_th_c_reg;
	di_im_di_ma_frame_motion_th_a_RBUS di_ma_frame_motion_th_a_reg;
	di_im_di_ma_frame_motion_th_b_RBUS di_ma_frame_motion_th_b_reg;
	di_im_di_ma_frame_motion_th_c_RBUS di_ma_frame_motion_th_c_reg;

	zoommotion_detector_reg.regValue = rtd_inl(DI_IM_DI_ZoomMotion_Detector_reg);
	zoommotion_det_fm_th_a_reg.regValue = rtd_inl(DI_IM_DI_ZoomMotion_Det_FM_TH_A_reg);
	zoommotion_det_fm_th_b_reg.regValue = rtd_inl(DI_IM_DI_ZoomMotion_Det_FM_TH_B_reg);
	zoommotion_det_fm_th_c_reg.regValue = rtd_inl(DI_IM_DI_ZoomMotion_Det_FM_TH_C_reg);
	di_ma_frame_motion_th_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	di_ma_frame_motion_th_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
	di_ma_frame_motion_th_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);

	if (Smart_Integration_Status.SCREEN_ACTION_STATUS.ZOOMMOTION_detect == 1)
	{
		zoommotion_det_fm_th_a_reg.smd_zoommotion_ma_framehtha	= di_ma_frame_motion_th_a_reg.ma_framehtha + offset;
		zoommotion_det_fm_th_a_reg.smd_zoommotion_ma_framemtha	= di_ma_frame_motion_th_a_reg.ma_framemtha + offset;
		zoommotion_det_fm_th_a_reg.smd_zoommotion_ma_framerlvtha	= di_ma_frame_motion_th_a_reg.ma_framerlvtha + offset;
		zoommotion_det_fm_th_a_reg.smd_zoommotion_ma_framestha	= di_ma_frame_motion_th_a_reg.ma_framestha + offset;

		zoommotion_det_fm_th_b_reg.smd_zoommotion_ma_framehthb	= di_ma_frame_motion_th_b_reg.ma_framehthb + offset;
		zoommotion_det_fm_th_b_reg.smd_zoommotion_ma_framemthb	= di_ma_frame_motion_th_b_reg.ma_framemthb + offset;
		zoommotion_det_fm_th_b_reg.smd_zoommotion_ma_framerlvthb	= di_ma_frame_motion_th_b_reg.ma_framerlvthb + offset;
		zoommotion_det_fm_th_b_reg.smd_zoommotion_ma_framesthb	= di_ma_frame_motion_th_b_reg.ma_framesthb + offset;

		zoommotion_det_fm_th_c_reg.smd_zoommotion_ma_framehthc	= di_ma_frame_motion_th_c_reg.ma_framehthc + offset;
		zoommotion_det_fm_th_c_reg.smd_zoommotion_ma_framemthc	= di_ma_frame_motion_th_c_reg.ma_framemthc + offset;
		zoommotion_det_fm_th_c_reg.smd_zoommotion_ma_framerlvthc	= di_ma_frame_motion_th_c_reg.ma_framerlvthc + offset;
		zoommotion_det_fm_th_c_reg.smd_zoommotion_ma_framesthc	= di_ma_frame_motion_th_c_reg.ma_framesthc + offset;

		zoommotion_detector_reg.smd_zoommotion_en = 1;
	}
	else
	{
		zoommotion_detector_reg.smd_zoommotion_en = 0;
	}

	rtd_outl(DI_IM_DI_ZoomMotion_Det_FM_TH_A_reg, zoommotion_det_fm_th_a_reg.regValue);
	rtd_outl(DI_IM_DI_ZoomMotion_Det_FM_TH_B_reg, zoommotion_det_fm_th_b_reg.regValue);
	rtd_outl(DI_IM_DI_ZoomMotion_Det_FM_TH_C_reg, zoommotion_det_fm_th_c_reg.regValue);
	rtd_outl(DI_IM_DI_ZoomMotion_Detector_reg, zoommotion_detector_reg.regValue);
}
#endif
// end chen 170522

void scalerVIP_DI_MiddleWare_SLOWMOTION_Apply(unsigned char offset)
{
	di_im_di_slowmotion_detector_1_RBUS slowmotion_detector_1_reg;
	di_im_di_slowmotion_det_fm_th_a_RBUS slowmotion_det_fm_th_a_reg;
	di_im_di_slowmotion_det_fm_th_b_RBUS slowmotion_det_fm_th_b_reg;
	di_im_di_slowmotion_det_fm_th_c_RBUS slowmotion_det_fm_th_c_reg;
	di_im_di_ma_frame_motion_th_a_RBUS di_ma_frame_motion_th_a_reg;
	di_im_di_ma_frame_motion_th_b_RBUS di_ma_frame_motion_th_b_reg;
	di_im_di_ma_frame_motion_th_c_RBUS di_ma_frame_motion_th_c_reg;

	slowmotion_detector_1_reg.regValue = rtd_inl(DI_IM_DI_SlowMotion_Detector_1_reg);
	slowmotion_det_fm_th_a_reg.regValue = rtd_inl(DI_IM_DI_SlowMotion_Det_FM_TH_A_reg);
	slowmotion_det_fm_th_b_reg.regValue = rtd_inl(DI_IM_DI_SlowMotion_Det_FM_TH_B_reg);
	slowmotion_det_fm_th_c_reg.regValue = rtd_inl(DI_IM_DI_SlowMotion_Det_FM_TH_C_reg);
	di_ma_frame_motion_th_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	di_ma_frame_motion_th_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
	di_ma_frame_motion_th_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);

	if (Smart_Integration_Status.SCREEN_ACTION_STATUS.SLOWMOTION_detect == 1 && Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL < 14)
	{
		slowmotion_det_fm_th_a_reg.smd_slowmotion_ma_framehtha		= di_ma_frame_motion_th_a_reg.ma_framehtha + offset;
		slowmotion_det_fm_th_a_reg.smd_slowmotion_ma_framemtha		= di_ma_frame_motion_th_a_reg.ma_framemtha + offset;
		slowmotion_det_fm_th_a_reg.smd_slowmotion_ma_framerlvtha	= di_ma_frame_motion_th_a_reg.ma_framerlvtha + offset;
		slowmotion_det_fm_th_a_reg.smd_slowmotion_ma_framestha		= di_ma_frame_motion_th_a_reg.ma_framestha + offset;

		slowmotion_det_fm_th_b_reg.smd_slowmotion_ma_framehthb		= di_ma_frame_motion_th_b_reg.ma_framehthb + offset;
		slowmotion_det_fm_th_b_reg.smd_slowmotion_ma_framemthb		= di_ma_frame_motion_th_b_reg.ma_framemthb + offset;
		slowmotion_det_fm_th_b_reg.smd_slowmotion_ma_framerlvthb	= di_ma_frame_motion_th_b_reg.ma_framerlvthb + offset;
		slowmotion_det_fm_th_b_reg.smd_slowmotion_ma_framesthb		= di_ma_frame_motion_th_b_reg.ma_framesthb + offset;

		slowmotion_det_fm_th_c_reg.smd_slowmotion_ma_framehthc		= di_ma_frame_motion_th_c_reg.ma_framehthc + offset;
		slowmotion_det_fm_th_c_reg.smd_slowmotion_ma_framemthc		= di_ma_frame_motion_th_c_reg.ma_framemthc + offset;
		slowmotion_det_fm_th_c_reg.smd_slowmotion_ma_framerlvthc	= di_ma_frame_motion_th_c_reg.ma_framerlvthc + offset;
		slowmotion_det_fm_th_c_reg.smd_slowmotion_ma_framesthc		= di_ma_frame_motion_th_c_reg.ma_framesthc + offset;

		slowmotion_detector_1_reg.smd_slowmotion_en = 1;
	}
	else if (Smart_Integration_Status.SCREEN_ACTION_STATUS.SLOWMOTION_detect == 0 && Smart_Integration_Status.SCREEN_ACTION_STATUS.FADING_detect == 0)
	{
		slowmotion_detector_1_reg.smd_slowmotion_en = 0;
	}

	rtd_outl(DI_IM_DI_SlowMotion_Det_FM_TH_A_reg, slowmotion_det_fm_th_a_reg.regValue);
	rtd_outl(DI_IM_DI_SlowMotion_Det_FM_TH_B_reg, slowmotion_det_fm_th_b_reg.regValue);
	rtd_outl(DI_IM_DI_SlowMotion_Det_FM_TH_C_reg, slowmotion_det_fm_th_c_reg.regValue);
	rtd_outl(DI_IM_DI_SlowMotion_Detector_1_reg, slowmotion_detector_1_reg.regValue);
}

void scalerVIP_DI_MiddleWare_FADING_Apply(unsigned char offset)
{
	di_im_di_slowmotion_detector_1_RBUS slowmotion_detector_1_reg;
	di_im_di_slowmotion_det_fm_th_a_RBUS slowmotion_det_fm_th_a_reg;
	di_im_di_slowmotion_det_fm_th_b_RBUS slowmotion_det_fm_th_b_reg;
	di_im_di_slowmotion_det_fm_th_c_RBUS slowmotion_det_fm_th_c_reg;
	di_im_di_ma_frame_motion_th_a_RBUS di_ma_frame_motion_th_a_reg;
	di_im_di_ma_frame_motion_th_b_RBUS di_ma_frame_motion_th_b_reg;
	di_im_di_ma_frame_motion_th_c_RBUS di_ma_frame_motion_th_c_reg;

	slowmotion_detector_1_reg.regValue = rtd_inl(DI_IM_DI_SlowMotion_Detector_1_reg);
	slowmotion_det_fm_th_a_reg.regValue = rtd_inl(DI_IM_DI_SlowMotion_Det_FM_TH_A_reg);
	slowmotion_det_fm_th_b_reg.regValue = rtd_inl(DI_IM_DI_SlowMotion_Det_FM_TH_B_reg);
	slowmotion_det_fm_th_c_reg.regValue = rtd_inl(DI_IM_DI_SlowMotion_Det_FM_TH_C_reg);
	di_ma_frame_motion_th_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	di_ma_frame_motion_th_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
	di_ma_frame_motion_th_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);

	if (Smart_Integration_Status.SCREEN_ACTION_STATUS.FADING_detect == 1)
	{
		slowmotion_det_fm_th_a_reg.smd_slowmotion_ma_framehtha		= di_ma_frame_motion_th_a_reg.ma_framehtha + offset;
		slowmotion_det_fm_th_a_reg.smd_slowmotion_ma_framemtha		= di_ma_frame_motion_th_a_reg.ma_framemtha + offset;
		slowmotion_det_fm_th_a_reg.smd_slowmotion_ma_framerlvtha	= di_ma_frame_motion_th_a_reg.ma_framerlvtha + offset;
		slowmotion_det_fm_th_a_reg.smd_slowmotion_ma_framestha		= di_ma_frame_motion_th_a_reg.ma_framestha + offset;

		slowmotion_det_fm_th_b_reg.smd_slowmotion_ma_framehthb		= di_ma_frame_motion_th_b_reg.ma_framehthb + offset;
		slowmotion_det_fm_th_b_reg.smd_slowmotion_ma_framemthb		= di_ma_frame_motion_th_b_reg.ma_framemthb + offset;
		slowmotion_det_fm_th_b_reg.smd_slowmotion_ma_framerlvthb	= di_ma_frame_motion_th_b_reg.ma_framerlvthb + offset;
		slowmotion_det_fm_th_b_reg.smd_slowmotion_ma_framesthb		= di_ma_frame_motion_th_b_reg.ma_framesthb + offset;

		slowmotion_det_fm_th_c_reg.smd_slowmotion_ma_framehthc		= di_ma_frame_motion_th_c_reg.ma_framehthc + offset;
		slowmotion_det_fm_th_c_reg.smd_slowmotion_ma_framemthc		= di_ma_frame_motion_th_c_reg.ma_framemthc + offset;
		slowmotion_det_fm_th_c_reg.smd_slowmotion_ma_framerlvthc	= di_ma_frame_motion_th_c_reg.ma_framerlvthc + offset;
		slowmotion_det_fm_th_c_reg.smd_slowmotion_ma_framesthc		= di_ma_frame_motion_th_c_reg.ma_framesthc + offset;

		slowmotion_detector_1_reg.smd_slowmotion_en = 1;
	}
	else if (Smart_Integration_Status.SCREEN_ACTION_STATUS.SLOWMOTION_detect == 0 && Smart_Integration_Status.SCREEN_ACTION_STATUS.FADING_detect == 0)
	{
		slowmotion_detector_1_reg.smd_slowmotion_en = 0;
	}

	rtd_outl(DI_IM_DI_SlowMotion_Det_FM_TH_A_reg, slowmotion_det_fm_th_a_reg.regValue);
	rtd_outl(DI_IM_DI_SlowMotion_Det_FM_TH_B_reg, slowmotion_det_fm_th_b_reg.regValue);
	rtd_outl(DI_IM_DI_SlowMotion_Det_FM_TH_C_reg, slowmotion_det_fm_th_c_reg.regValue);
	rtd_outl(DI_IM_DI_SlowMotion_Detector_1_reg, slowmotion_detector_1_reg.regValue);
}

void scalerVIP_DI_MiddleWare_SPECIAL_Apply(unsigned char source_timing_index)
{
#if 1
	di_di_smd_additionalcoeff2_RBUS smd_additionalcoeff2_reg;
	smd_additionalcoeff2_reg.regValue = rtd_inl(DI_DI_SMD_AdditionalCoeff2_reg);

	if (Smart_Integration_Status.SCREEN_SPECIAL_STATUS.STILL_CONCENTRIC_CIRCLES_detect == 1)
	{
		smd_additionalcoeff2_reg.smd_image_boundary_height = 0;
		smd_additionalcoeff2_reg.smd_image_boundary_width = 0;
	}
	else
	{
		smd_additionalcoeff2_reg.smd_image_boundary_height = 4;
		smd_additionalcoeff2_reg.smd_image_boundary_width = 10;
	}

	rtd_outl(DI_DI_SMD_AdditionalCoeff2_reg, smd_additionalcoeff2_reg.regValue);

#else // dirty solution
	//=========同心圓
	//+++++++++中間DI做intra
	//+++++++++外圍做weave
	unsigned int mcc_solution_table[6][6] =
	{
//		en,		xstart_12_0	ystart_12_0	xend_12_0	yend_12_0	DI fix weave
		{1,		0x0080,		0x0040,		0x01ff,		0x008f,		1		},	//NTSC
		{1,		0x0080,		0x0040,		0x01ff,		0x008f,		1		},	//PAL
		{1,		0x0080,		0x0040,		0x01ff,		0x008f,		1		},	//SD
		{0,		0x0080,		0x0040,		0x01ff,		0x008f,		0		},	//HD
		{0,		0x0080,		0x0040,		0x01ff,		0x008f,		0		},	//VGA
		{1,		0x0080,		0x0040,		0x01ff,		0x008f,		1		},	//HDMI PAL
	};
							// NTSC   PAL      SD        HD       VGA     HDMI PAL
	unsigned int pan_info_table[6] = {0xA000,0xf000,0xBD00,0xf000,0xf000,0xf000};


	static int temp_cnt;
	temp_cnt++;
	//if((motion_concentric_circles_detect(0, scaler_disp_info[_CHANNEL1].DispWid, scaler_disp_info[_CHANNEL1].DispLen) == 1)&&(mcc_solution_table[source_timing_index][0] == 1))
	if((Smart_Integration_Status.SCREEN_SPECIAL_STATUS.STILL_CONCENTRIC_CIRCLES_detect == 1)&&(mcc_solution_table[source_timing_index][0] == 1))
	{
		//VIP_DEBUG_PRINTF("\n    ifififif ifififif ..............= %d <<<<<\n",temp_cnt);
		di_im_di_weave_window_control_RBUS im_di_weave_window_control_reg;
		di_im_di_weave_window_0_start_RBUS im_di_weave_window_0_start_reg;
		di_im_di_weave_window_0_end_RBUS im_di_weave_window_0_end_reg;

		im_di_weave_window_control_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
		im_di_weave_window_0_start_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_0_START_reg);
		im_di_weave_window_0_end_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_0_END_reg);

		di_im_di_pan_detection_statistic_4_RBUS im_di_pan_detection_statistic_4_reg;
		im_di_pan_detection_statistic_4_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_4_reg);

		im_di_weave_window_control_reg.fixedwindowmode = 3;
		if (im_di_pan_detection_statistic_4_reg.hmc_c0_pixelsum > pan_info_table[source_timing_index])
			im_di_weave_window_control_reg.fixedwindow0 = 0;
		else
		{
			//VIP_DEBUG_PRINTF("\n    if if if if ..............= %x <<<<<\n",im_di_pan_detection_statistic_4_reg.hmc_c0_pixelsum);
			im_di_weave_window_control_reg.fixedwindow0 = 1;
		}
		im_di_weave_window_0_start_reg.xstart_12_0 = (mcc_solution_table[source_timing_index][1] + (((Smart_Integration_Status.Frame_Couter*7)%5)-2));		// +/- 2跳動
		im_di_weave_window_0_start_reg.ystart_12_0 = (mcc_solution_table[source_timing_index][2] + (((Smart_Integration_Status.Frame_Couter*7)%5)-2));		// +/- 2跳動
		im_di_weave_window_0_end_reg.xend_12_0 = (mcc_solution_table[source_timing_index][3] + (((Smart_Integration_Status.Frame_Couter*7)%5)-2));		// +/- 2跳動
		im_di_weave_window_0_end_reg.yend_12_0 = (mcc_solution_table[source_timing_index][4] + (((Smart_Integration_Status.Frame_Couter*7)%5)-2));		// +/- 2跳動


		if(mcc_solution_table[source_timing_index][5] == 1)//av pal
		{
			rtd_maskl(DI_IM_DI_CONTROL_reg, 0xfffffff3, 0x8);			//DI fix weave
		}

		//	VIP_DEBUG_PRINTF("\n    source_timing_index= %d <<<<<\n",source_timing_index);
		if( (Scaler_InputSrcGetMainChType()) == _SRC_HDMI&&(source_timing_index == 5) )
		{ // HDMI
			rtd_clearbits(VGIP_VGIP_CHN1_CTRL_reg, _BIT18);
		}
		else if( (Scaler_InputSrcGetMainChType()) == _SRC_CVBS&&(source_timing_index == 1) )
		{
			rtd_setbits(VGIP_VGIP_CHN1_CTRL_reg, _BIT18);
		}
		rtd_outl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg,im_di_weave_window_control_reg.regValue);
		rtd_outl(DI_IM_DI_WEAVE_WINDOW_0_START_reg,im_di_weave_window_0_start_reg.regValue);
		rtd_outl(DI_IM_DI_WEAVE_WINDOW_0_END_reg,im_di_weave_window_0_end_reg.regValue);
	}
	else
	{
		//VIP_DEBUG_PRINTF("\n    else else ..............= %d <<<<<\n",temp_cnt);
		di_im_di_weave_window_control_RBUS im_di_weave_window_control_reg;

		im_di_weave_window_control_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
		im_di_weave_window_control_reg.fixedwindowmode = 0;
		im_di_weave_window_control_reg.fixedwindow0 = 0;

		rtd_maskl(DI_IM_DI_CONTROL_reg, 0xfffffff3, 0x4);			//DI MA

		rtd_outl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg,im_di_weave_window_control_reg.regValue);

		//VIP_DEBUG_PRINTF("\n    out    source_timing_index= %d <<<<<\n",source_timing_index);
		if((Scaler_InputSrcGetMainChType()) == _SRC_HDMI&&(source_timing_index == 5))
		{ // HDMI
			rtd_setbits(VGIP_VGIP_CHN1_CTRL_reg, _BIT18);
		}
		else if((Scaler_InputSrcGetMainChType()) == _SRC_CVBS&&(source_timing_index == 1))
		{
			rtd_clearbits(VGIP_VGIP_CHN1_CTRL_reg, _BIT18);
		}
	}
#endif
}

void scalerVIP_DI_MiddleWare_SMD_Apply(void)
{
	di_di_smd_gmvcoeff1_RBUS smd_gmvcoeff1_reg;
	di_di_smd_pan1_RBUS smd_pan1_reg;

	//Smart_Integration_Status.Vip_Source_Timing
	if (Smart_Integration_Status.DI_WIDTH>1000) //1080i
	{
		smd_gmvcoeff1_reg.regValue= rtd_inl(DI_DI_SMD_GMVCoeff1_reg);

		smd_pan1_reg.regValue=rtd_inl(DI_DI_SMD_Pan1_reg);


		if (smd_gmvcoeff1_reg.smd_gmv_ctr>2048)
			smd_pan1_reg.smd_hardware_pan_en=1;
		else
			smd_pan1_reg.smd_hardware_pan_en=0;

		rtd_outl(DI_DI_SMD_Pan1_reg, smd_pan1_reg.regValue);

	}

}

// chen 170522
#if 0
void scalerVIP_NR_MiddleWare_RTNR_k128_Apply(unsigned char enable)
{
	unsigned char th_multiple = 2;

	di_im_di_rtnr_new_sad_edge_th_1_RBUS			rtnr_new_sad_edge_th_1_reg;
	di_im_di_rtnr_new_sad_edge_th_2_RBUS			rtnr_new_sad_edge_th_2_reg;
	di_im_di_rtnr_new_sad_nonedge_th_1_RBUS		rtnr_new_sad_nonedge_th_1_reg;
	di_im_di_rtnr_new_sad_nonedge_th_2_RBUS		rtnr_new_sad_nonedge_th_2_reg;
	di_im_di_rtnr_new_sad_count_th_RBUS			rtnr_new_sad_count_th_reg;
	di_im_di_rtnr_new_edgetypediff_th_1_RBUS		rtnr_new_edgetypediff_th_1_reg;
	di_im_di_rtnr_new_edgetypediff_th_2_RBUS		rtnr_new_edgetypediff_th_2_reg;

	di_im_di_rtnr_k128_mode_RBUS					rtnr_k128_mode_reg;
	di_im_di_rtnr_new_sad_th_3_RBUS				rtnr_new_sad_th_3_reg;
	di_im_di_rtnr_new_sad_count_th_2_RBUS			rtnr_new_sad_count_th_2_reg;
	di_im_di_rtnr_new_sta_motion_th_0_RBUS			rtnr_new_sta_motion_th_0_reg;
	di_im_di_rtnr_new_sta_motion_th_1_RBUS			rtnr_new_sta_motion_th_1_reg;
	di_im_di_rtnr_new_sta_motion_th_3_RBUS			rtnr_new_sta_motion_th_3_reg;
	di_im_di_rtnr_new_sad_edge_slope_0_4_RBUS		rtnr_new_sad_edge_slope_0_4_reg;
	di_im_di_rtnr_new_sad_edge_slope_4_8_RBUS		rtnr_new_sad_edge_slope_4_8_reg;
	di_im_di_rtnr_new_sad_nonedge_slope_0_4_RBUS	rtnr_new_sad_nonedge_slope_0_4_reg;
	di_im_di_rtnr_new_sad_nonedge_slope_4_8_RBUS	rtnr_new_sad_nonedge_slope_4_8_reg;
	di_im_di_rtnr_new_sta_motion_slope_0_4_RBUS	rtnr_new_sta_motion_slope_0_4_reg;
	di_im_di_rtnr_new_sta_motion_slope_4_8_RBUS	rtnr_new_sta_motion_slope_4_8_reg;

	rtnr_new_sad_edge_th_1_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_1_reg);
	rtnr_new_sad_edge_th_2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_EDGE_TH_2_reg);
	rtnr_new_sad_nonedge_th_1_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_1_reg);
	rtnr_new_sad_nonedge_th_2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_TH_2_reg);
	rtnr_new_sad_count_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_reg);
	rtnr_new_edgetypediff_th_1_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_1_reg);
	rtnr_new_edgetypediff_th_2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_EDGETYPEDIFF_TH_2_reg);

	rtnr_k128_mode_reg.regValue = rtd_inl(DI_IM_DI_RTNR_K128_MODE_reg);
	rtnr_new_sad_th_3_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_TH_3_reg);
	rtnr_new_sad_count_th_2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_2_reg);
	rtnr_new_sta_motion_th_0_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_STA_MOTION_TH_0_reg);
	rtnr_new_sta_motion_th_1_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_STA_MOTION_TH_1_reg);
	rtnr_new_sta_motion_th_3_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_STA_MOTION_TH_3_reg);
	rtnr_new_sad_edge_slope_0_4_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_EDGE_SLOPE_0_4_reg);
	rtnr_new_sad_edge_slope_4_8_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_EDGE_SLOPE_4_8_reg);
	rtnr_new_sad_nonedge_slope_0_4_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_SLOPE_0_4_reg);
	rtnr_new_sad_nonedge_slope_4_8_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_SLOPE_4_8_reg);
	rtnr_new_sta_motion_slope_0_4_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_STA_MOTION_SLOPE_0_4_reg);
	rtnr_new_sta_motion_slope_4_8_reg.regValue = rtd_inl(DI_IM_DI_RTNR_NEW_STA_MOTION_SLOPE_4_8_reg);

	if (enable)
	{
		rtnr_k128_mode_reg.cp_temporal_k128_mode = 1;

		rtnr_new_sad_edge_slope_0_4_reg.rtnr_calculate_motion_sad_edge_th0_1_slope = (((rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th1<<th_multiple)-(rtnr_new_sad_th_3_reg.rtnr_calculate_motion_sad_edge_th0<<th_multiple))>>2); // >>2 mean ((>>4)<<2) for msb 6 bits
		rtnr_new_sad_edge_slope_0_4_reg.rtnr_calculate_motion_sad_edge_th1_2_slope = (((rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th2<<th_multiple)-(rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th1<<th_multiple))>>2);
		rtnr_new_sad_edge_slope_0_4_reg.rtnr_calculate_motion_sad_edge_th2_3_slope = (((rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th3<<th_multiple)-(rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th2<<th_multiple))>>2);
		rtnr_new_sad_edge_slope_0_4_reg.rtnr_calculate_motion_sad_edge_th3_4_slope = (((rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th4<<th_multiple)-(rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th3<<th_multiple))>>2);
		rtnr_new_sad_edge_slope_4_8_reg.rtnr_calculate_motion_sad_edge_th4_5_slope = (((rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th5<<th_multiple)-(rtnr_new_sad_edge_th_1_reg.rtnr_calculate_motion_sad_edge_th4<<th_multiple))>>2);
		rtnr_new_sad_edge_slope_4_8_reg.rtnr_calculate_motion_sad_edge_th5_6_slope = (((rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th6<<th_multiple)-(rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th5<<th_multiple))>>2);
		rtnr_new_sad_edge_slope_4_8_reg.rtnr_calculate_motion_sad_edge_th6_7_slope = (((rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th7<<th_multiple)-(rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th6<<th_multiple))>>2);
		rtnr_new_sad_edge_slope_4_8_reg.rtnr_calculate_motion_sad_edge_th7_8_slope = (((rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th8<<th_multiple)-(rtnr_new_sad_edge_th_2_reg.rtnr_calculate_motion_sad_edge_th7<<th_multiple))>>2);

		rtnr_new_sad_nonedge_slope_0_4_reg.rtnr_calculate_motion_sad_nonedge_th0_1_slope = (((rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th1<<th_multiple)-(rtnr_new_sad_th_3_reg.rtnr_calculate_motion_sad_nonedge_th0<<th_multiple))>>2);
		rtnr_new_sad_nonedge_slope_0_4_reg.rtnr_calculate_motion_sad_nonedge_th1_2_slope = (((rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th2<<th_multiple)-(rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th1<<th_multiple))>>2);
		rtnr_new_sad_nonedge_slope_0_4_reg.rtnr_calculate_motion_sad_nonedge_th2_3_slope = (((rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th3<<th_multiple)-(rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th2<<th_multiple))>>2);
		rtnr_new_sad_nonedge_slope_0_4_reg.rtnr_calculate_motion_sad_nonedge_th3_4_slope = (((rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th4<<th_multiple)-(rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th3<<th_multiple))>>2);
		rtnr_new_sad_nonedge_slope_4_8_reg.rtnr_calculate_motion_sad_nonedge_th4_5_slope = (((rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th5<<th_multiple)-(rtnr_new_sad_nonedge_th_1_reg.rtnr_calculate_motion_sad_nonedge_th4<<th_multiple))>>2);
		rtnr_new_sad_nonedge_slope_4_8_reg.rtnr_calculate_motion_sad_nonedge_th5_6_slope = (((rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th6<<th_multiple)-(rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th5<<th_multiple))>>2);
		rtnr_new_sad_nonedge_slope_4_8_reg.rtnr_calculate_motion_sad_nonedge_th6_7_slope = (((rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th7<<th_multiple)-(rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th6<<th_multiple))>>2);
		rtnr_new_sad_nonedge_slope_4_8_reg.rtnr_calculate_motion_sad_nonedge_th7_8_slope = (((rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th8<<th_multiple)-(rtnr_new_sad_nonedge_th_2_reg.rtnr_calculate_motion_sad_nonedge_th7<<th_multiple))>>2);

		rtnr_new_sta_motion_slope_0_4_reg.rtnr_statistic_motion_count_th0_1_slope = (((rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th1<<th_multiple)-(rtnr_new_sad_count_th_2_reg.rtnr_statistic_motion_count_th0<<th_multiple))>>2);
		rtnr_new_sta_motion_slope_0_4_reg.rtnr_statistic_motion_count_th1_2_slope = (((rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th2<<th_multiple)-(rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th1<<th_multiple))>>2);
		rtnr_new_sta_motion_slope_0_4_reg.rtnr_statistic_motion_count_th2_3_slope = (((rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th3<<th_multiple)-(rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th2<<th_multiple))>>2);
		rtnr_new_sta_motion_slope_0_4_reg.rtnr_statistic_motion_count_th3_4_slope = (((rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th4<<th_multiple)-(rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th3<<th_multiple))>>2);
		rtnr_new_sta_motion_slope_4_8_reg.rtnr_statistic_motion_count_th4_5_slope = (((rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th5<<th_multiple)-(rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th4<<th_multiple))>>2);
		rtnr_new_sta_motion_slope_4_8_reg.rtnr_statistic_motion_count_th5_6_slope = (((rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th6<<th_multiple)-(rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th5<<th_multiple))>>2);
		rtnr_new_sta_motion_slope_4_8_reg.rtnr_statistic_motion_count_th6_7_slope = (((rtnr_new_sad_count_th_2_reg.rtnr_statistic_motion_count_th7<<th_multiple)-(rtnr_new_sad_count_th_reg.rtnr_statistic_motion_count_th6<<th_multiple))>>2);
		rtnr_new_sta_motion_slope_4_8_reg.rtnr_statistic_motion_count_th7_8_slope = (((rtnr_new_sad_count_th_2_reg.rtnr_statistic_motion_count_th8<<th_multiple)-(rtnr_new_sad_count_th_2_reg.rtnr_statistic_motion_count_th7<<th_multiple))>>2);

		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_TH_3_reg, rtnr_new_sad_th_3_reg.regValue);
		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_COUNT_TH_2_reg, rtnr_new_sad_count_th_2_reg.regValue);

		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_EDGE_SLOPE_0_4_reg, rtnr_new_sad_edge_slope_0_4_reg.regValue);
		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_EDGE_SLOPE_4_8_reg, rtnr_new_sad_edge_slope_4_8_reg.regValue);
		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_SLOPE_0_4_reg, rtnr_new_sad_nonedge_slope_0_4_reg.regValue);
		rtd_outl(DI_IM_DI_RTNR_NEW_SAD_NONEDGE_SLOPE_4_8_reg, rtnr_new_sad_nonedge_slope_4_8_reg.regValue);
		rtd_outl(DI_IM_DI_RTNR_NEW_STA_MOTION_SLOPE_0_4_reg, rtnr_new_sta_motion_slope_0_4_reg.regValue);
		rtd_outl(DI_IM_DI_RTNR_NEW_STA_MOTION_SLOPE_4_8_reg, rtnr_new_sta_motion_slope_4_8_reg.regValue);
	}
	else
	{
		rtnr_k128_mode_reg.cp_temporal_k128_mode = 0;
	}

	rtd_outl(DI_IM_DI_RTNR_K128_MODE_reg, rtnr_k128_mode_reg.regValue);
}
#endif
// end chen 170522

unsigned char scalerVIP_DI_HMC_Continue(void)
{
	short i = 0;
	unsigned char hmc_pass_total = 0, pass = 0, ratio = 0, no_pass = 0;
	di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;
	di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);

	Smart_Integration_Status.SCREEN_HMC_STATUS.Cur_Top5_Total = ((Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[0]])>>2)+
																((Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[1]])>>2)+
																((Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[2]])>>2)+
																((Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[3]])>>2)+
																((Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[4]])>>2);

	pass = scalerVIP_DI_HMC_Rank_Check(4, 5, 0, 1); // current top 4 rank must in previous 1 frame of top 5 rank, and top 1 rank can't change

	if (Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[0]] < 50)
		pass = 0;
	i = Smart_Integration_Status.SCREEN_HMC_STATUS.Cur_Top5_Total/20;
	if (i<100)
		i = 100;
	if (abs(Smart_Integration_Status.SCREEN_HMC_STATUS.Pre2_Top5_Total - Smart_Integration_Status.SCREEN_HMC_STATUS.Cur_Top5_Total) > i || abs(Smart_Integration_Status.SCREEN_HMC_STATUS.Pre3_Top5_Total - Smart_Integration_Status.SCREEN_HMC_STATUS.Pre_Top5_Total) > i)
		pass = 0;

	// check rank 10~20 is large then maybe problem
	ratio = (1920*540)/(Smart_Integration_Status.DI_HEIGHT*Smart_Integration_Status.DI_WIDTH);
	for (i=0; i<10; i++)
	{
		if ((Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[9+i]]*ratio) > 2500)
			no_pass++;
	}
	if (no_pass > 7)
		pass = 0;

	if (scalerVIP_DI_HMC_LR_Check() == 0)
		pass = 0;

	if (Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag == 1) // when use 2nd histogram, hmc don't check any condition
		pass = 1;

	// assign pass sequence
	for (i=HMC_PASS_COUNT-2; i>=0; i--)
		Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Sequence[i+1] = Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Sequence[i];
	Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Sequence[0] = pass;

	if (Smart_Integration_Status.SCREEN_HMC_STATUS.Show_Debug_MSG)
	{
		VIP_DEBUG_PRINTF("[SIS HMC] Top5 (%d, %d, %d, %d, %d)c: %d, p: %d, p2:%d, p3:%d, pass: %d\n",
			Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[0], Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[1],
			Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[2], Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[3],
			Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[4], Smart_Integration_Status.SCREEN_HMC_STATUS.Cur_Top5_Total,
			Smart_Integration_Status.SCREEN_HMC_STATUS.Pre_Top5_Total, Smart_Integration_Status.SCREEN_HMC_STATUS.Pre2_Top5_Total,
			Smart_Integration_Status.SCREEN_HMC_STATUS.Pre3_Top5_Total, pass);
	}

	Smart_Integration_Status.SCREEN_HMC_STATUS.Pre3_Top5_Total = Smart_Integration_Status.SCREEN_HMC_STATUS.Pre2_Top5_Total;
	Smart_Integration_Status.SCREEN_HMC_STATUS.Pre2_Top5_Total = Smart_Integration_Status.SCREEN_HMC_STATUS.Pre_Top5_Total;
	Smart_Integration_Status.SCREEN_HMC_STATUS.Pre_Top5_Total = Smart_Integration_Status.SCREEN_HMC_STATUS.Cur_Top5_Total;

	for (i=0;i <HMC_PASS_COUNT; i++)
		hmc_pass_total += Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Sequence[i];

	Smart_Integration_Status.SCREEN_HMC_STATUS.Pass_Result_Total = hmc_pass_total;

	if (hmc_pass_total > 80)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.HMC_detect = 1;
	else
		Smart_Integration_Status.SCREEN_ACTION_STATUS.HMC_detect = 0;

	return pass;
}

/*
fromRank : from Rank list of how many Top
inRank : in Rank list of how many Top
fixRank : how many top rank can't change
vsync_time : using how many time

fromRank must smaller/equal to inRank

ie. (3, 5, 1, 1) : Current Top 3 Rank have all in previous 1 frame of Top 5 Rank, and Top 1 rank can't change
*/
unsigned char scalerVIP_DI_HMC_Rank_Check(unsigned char fromRank, unsigned char inRank, unsigned char fixRank, unsigned char vsync_time)
{
	static unsigned char rank[30][HMC_BIN_COUNT]; //[T][Rank]
	unsigned char pass = 0, counter = 0, found = 0;
	int i, j, k, l;

	if (vsync_time == 0)
		return 0;

	if (vsync_time >= 29)
		vsync_time = 29;

	if (fromRank == 0)
		return 0;

	if (inRank == 0)
		return 0;

	if (fromRank >= 10)
		fromRank = 10;

	if (inRank >= 10)
		inRank = 10;

	if (fromRank > inRank)
		fromRank = inRank;

	for (i=0; i<HMC_BIN_COUNT; i++)
	{
		for (j=vsync_time-1; j>=0; j--)
		{
			rank[j+1][i] = rank[j][i];
		}
		rank[0][i] = Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[i];
	}

	pass = 1;
	for (k=0; k<vsync_time; k++)
	{
		counter = 0;
		for (i=0; i<fromRank; i++)
		{
			found = 0;
			for (j=0; j<inRank; j++)
			{
				if (rank[0][i] == rank[k+1][j])
					found = 1;
			}

			if (found)
				counter++;
		}

		if (counter != fromRank)
			pass = 0;

		for (l=0; l<fixRank; l++)
		{
			if (rank[0][l] != rank[k+1][l])
				pass = 0;
		}

		if (Smart_Integration_Status.SCREEN_HMC_STATUS.Show_Debug_MSG)
			VIP_DEBUG_PRINTF("[SIS HMC]HMC Rank Check (vsync:%d): inRank:%d, fromRank:%d, counter:%d, Rank pass:%d\n", k, inRank, fromRank, counter, pass);
	}

	return pass;
}

unsigned char scalerVIP_DI_HMC_LR_Check(void)
{
	enum LR_FLAG {NO_RL, L_Flag, R_Flag};
	static unsigned char LR_check[360];
	unsigned char pass = 0, R_count = 0, L_count = 0, R_bin_count = 0, L_bin_count = 0;
	int i;

	di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;
	di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);

	if(di_im_di_hmc_pan_control_reg.pan_me_select_mv_hist ==0)
	{
		for (i=0; i<HMC_BIN_COUNT; i++)
		{
			if (i<16 && Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[i] > 100)
				R_bin_count++;

			if (i>16 && Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[i] > 100)
				L_bin_count++;
		}
	}

	// assign LR sequence
	for (i=360-2; i>=0; i--)
		LR_check[i+1] = LR_check[i];

	if (R_bin_count >= 3 && L_bin_count == 0)
		LR_check[0] = L_Flag;
	else if (L_bin_count >= 3 && R_bin_count == 0)
		LR_check[0] = R_Flag;
	else
		LR_check[0] = NO_RL;

	for (i=0; i<360-1; i++)
	{
		if (LR_check[i] == L_Flag && LR_check[i+1] == L_Flag)
			L_count++;
		else if (LR_check[i] == R_Flag && LR_check[i+1] == R_Flag)
			R_count++;
	}

	if (L_count >= 20 && R_count >= 20)
		pass = 0;
	else
		pass = 1;

	if (Smart_Integration_Status.SCREEN_HMC_STATUS.Show_Debug_MSG)
		VIP_DEBUG_PRINTF("[SIS HMC]HMC LR Check: R_bin: %d, L_bin: %d, L_count: %d, R_count: %d, pass: %d\n", R_bin_count, L_bin_count, L_count, R_count, pass);
	return pass;
}

void scalerVIP_DI_MOTION_StatusUpdate(void)
{
	short i=0, tmp=0;
	unsigned int total_pixel = 0, total_score_ratio = 0;
	unsigned char motion_level = 0;
	di_im_di_si_ma_total_fm_sum_RBUS ma_total_fm_sum_reg;
	di_im_di_si_ma_large_fm_pixel_RBUS ma_large_fm_pxl_reg;
	di_im_di_si_ma_large_fm_sum_RBUS ma_large_fm_sum_reg;

	total_pixel = Smart_Integration_Status.DI_WIDTH*Smart_Integration_Status.DI_HEIGHT;

	// assign status sequence
	for (i=MOTION_COUNT-2; i>=0; i--)
	{
		Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL_Sequence[i+1] = Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL_Sequence[i];
		Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[i+1] = Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[i];
		Smart_Integration_Status.SCREEN_MOTION_STATUS.Total_FM_SUM_Sequence[i+1] = Smart_Integration_Status.SCREEN_MOTION_STATUS.Total_FM_SUM_Sequence[i];
	}

	ma_total_fm_sum_reg.regValue = rtd_inl(DI_IM_DI_SI_MA_TOTAL_FM_SUM_reg);
	ma_large_fm_pxl_reg.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_PIXEL_reg);
	ma_large_fm_sum_reg.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_SUM_reg);

	Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL_Sequence[0] = ma_large_fm_pxl_reg.large_frame_motion_pixel;
	Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[0] = ma_large_fm_sum_reg.large_frame_motion_sum;
	Smart_Integration_Status.SCREEN_MOTION_STATUS.Total_FM_SUM_Sequence[0] = ma_total_fm_sum_reg.total_frame_motion_sum;

	for (i=0; i<MOTION_COUNT; i++)
	{
		total_score_ratio += ((Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL_Sequence[i]*MOTION_SCALE)/(total_pixel==0?1:total_pixel)); // equalizer score to 0~1000
	}
	Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL = total_score_ratio/MOTION_COUNT;

	total_score_ratio = 0;
	for (i=0; i<MOTION_COUNT; i++)
	{
		tmp = (((Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[i]*MOTION_SCALE)/(Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL_Sequence[i]+1))-(2*MOTION_SCALE));  // equalizer score to (2000~3000)-2000 = 0~1000
		if (tmp >= 0)
			total_score_ratio += tmp;
	}
	Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY = total_score_ratio/MOTION_COUNT;

	total_score_ratio = 0;
	for (i=0; i<MOTION_COUNT; i++)
	{
		// blend mode 1 total pixels (total_fm_sum[0,1,2,3]-large_fm_sum[2,3] = [0,1])
		total_score_ratio += (((Smart_Integration_Status.SCREEN_MOTION_STATUS.Total_FM_SUM_Sequence[i]-Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[i])*MOTION_SCALE)/(total_pixel==0?1:total_pixel)); // equalizer score to 0~1000
	}
	Smart_Integration_Status.SCREEN_MOTION_STATUS.Small_FM_PXL = total_score_ratio/MOTION_COUNT;
	Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL = (MOTION_SCALE-Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL-Smart_Integration_Status.SCREEN_MOTION_STATUS.Small_FM_PXL);

	if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 300 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 490)
		motion_level = 48;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 300 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 480)
		motion_level = 47;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 300 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 470)
		motion_level = 46;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 300 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 460)
		motion_level = 45;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 300 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 440)
		motion_level = 44;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 290 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 420)
		motion_level = 43;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 280 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 400)
		motion_level = 42;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 270 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 380)
		motion_level = 41;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 260 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 360)
		motion_level = 40;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 250 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 340)
		motion_level = 39;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 240 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 320)
		motion_level = 38;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 230 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 300)
		motion_level = 37;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 220 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 280)
		motion_level = 36;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 210 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 260)
		motion_level = 35;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 200 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 240)
		motion_level = 34;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 190 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 220)
		motion_level = 33;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 180 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 200)
		motion_level = 32;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 170 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 180)
		motion_level = 31;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 160 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 160)
		motion_level = 30;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 150 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 140)
		motion_level = 29;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 140 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 120)
		motion_level = 28;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 130 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 105)
		motion_level = 27;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 120 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 90)
		motion_level = 26;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 110 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 75)
		motion_level = 25;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 100 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 60)
		motion_level = 24;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 90 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 45)
		motion_level = 23;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 80 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 30)
		motion_level = 22;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 70 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 15)
		motion_level = 21;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL >= 60 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY >= 0)
		motion_level = 20;

	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL == 1000)
		motion_level = 1;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 995)
		motion_level = 2;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 990)
		motion_level = 3;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 980)
		motion_level = 4;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 960)
		motion_level = 5;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 940)
		motion_level = 6;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 920)
		motion_level = 7;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 900)
		motion_level = 8;

	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 2 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 900)
		motion_level = 9;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 4 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 890)
		motion_level = 10;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 6 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 880)
		motion_level = 11;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 8 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 865)
		motion_level = 12;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 10 && Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 850)
		motion_level = 13;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 15 && (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 800 || Smart_Integration_Status.SCREEN_MOTION_STATUS.Small_FM_PXL >= 450))
		motion_level = 14;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 20 && (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 750 || Smart_Integration_Status.SCREEN_MOTION_STATUS.Small_FM_PXL >= 430))
		motion_level = 15;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 30 && (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 700 || Smart_Integration_Status.SCREEN_MOTION_STATUS.Small_FM_PXL >= 410))
		motion_level = 16;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 40 && (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 650 || Smart_Integration_Status.SCREEN_MOTION_STATUS.Small_FM_PXL >= 390))
		motion_level = 17;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 50 && (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 600 || Smart_Integration_Status.SCREEN_MOTION_STATUS.Small_FM_PXL >= 370))
		motion_level = 18;
	else if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL < 60 && (Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL >= 550 || Smart_Integration_Status.SCREEN_MOTION_STATUS.Small_FM_PXL >= 350))
		motion_level = 19;
	else	 // debug, check unknown info.
		motion_level = 0;

	for (i=MOTION_COUNT-2; i>=0; i--)
	{
		Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL_Sequence[i+1] = Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL_Sequence[i];
	}
	Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL_Sequence[0] = motion_level;

	total_score_ratio = 0;
	for (i=0; i<MOTION_COUNT; i++)
	{
		total_score_ratio += Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL_Sequence[i];
	}
	Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL = total_score_ratio/MOTION_COUNT;

	if (Smart_Integration_Status.SCREEN_MOTION_STATUS.Show_Debug_MSG)
	{
		VIP_DEBUG_PRINTF("[SIS MOTION] LV: %d/48 [large: %d[eg: %d], small: %d, zero: %d]/1000\n", Smart_Integration_Status.SCREEN_MOTION_STATUS.MOTION_LEVEL,
		Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_PXL, Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_ENERGY,
		Smart_Integration_Status.SCREEN_MOTION_STATUS.Small_FM_PXL, Smart_Integration_Status.SCREEN_MOTION_STATUS.Zero_FM_PXL);
	}
}

void scalerVIP_DI_HMC_StatusUpdate(void)
{
	unsigned int i, j, m;
	unsigned short sort_array[HMC_BIN_COUNT], tmp_value, tmp_index;

	di_im_di_pan_detection_statistic_1_RBUS im_di_pan_detection_statistic_1_reg;
	di_im_di_pan_detection_statistic_2_RBUS im_di_pan_detection_statistic_2_reg;
	di_im_di_pan_detection_statistic_3_RBUS im_di_pan_detection_statistic_3_reg;
	di_im_di_pan_detection_statistic_4_RBUS im_di_pan_detection_statistic_4_reg;
	di_im_di_pan_detection_statistic_5_RBUS im_di_pan_detection_statistic_5_reg;
	di_im_di_pan_detection_statistic_6_RBUS im_di_pan_detection_statistic_6_reg;
	di_im_di_pan_detection_statistic_7_RBUS im_di_pan_detection_statistic_7_reg;
	di_im_di_pan_detection_statistic_8_RBUS im_di_pan_detection_statistic_8_reg;
	di_im_di_pan_detection_statistic_9_RBUS im_di_pan_detection_statistic_9_reg;
	di_im_di_pan_detection_statistic_10_RBUS im_di_pan_detection_statistic_10_reg;
	di_im_di_pan_detection_statistic_11_RBUS im_di_pan_detection_statistic_11_reg;
	di_im_di_pan_detection_statistic_12_RBUS im_di_pan_detection_statistic_12_reg;
	di_im_di_pan_detection_statistic_13_RBUS im_di_pan_detection_statistic_13_reg;
	di_im_di_pan_detection_statistic_14_RBUS im_di_pan_detection_statistic_14_reg;
	di_im_di_pan_detection_statistic_15_RBUS im_di_pan_detection_statistic_15_reg;
	di_im_di_pan_detection_statistic_16_RBUS im_di_pan_detection_statistic_16_reg;
	di_im_di_pan_detection_statistic_17_RBUS im_di_pan_detection_statistic_17_reg;
	di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;

	im_di_pan_detection_statistic_1_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_1_reg);
	im_di_pan_detection_statistic_2_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_2_reg);
	im_di_pan_detection_statistic_3_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_3_reg);
	im_di_pan_detection_statistic_4_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_4_reg);
	im_di_pan_detection_statistic_5_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_5_reg);
	im_di_pan_detection_statistic_6_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_6_reg);
	im_di_pan_detection_statistic_7_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_7_reg);
	im_di_pan_detection_statistic_8_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_8_reg);
	im_di_pan_detection_statistic_9_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_9_reg);
	im_di_pan_detection_statistic_10_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_10_reg);
	im_di_pan_detection_statistic_11_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_11_reg);
	im_di_pan_detection_statistic_12_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_12_reg);
	im_di_pan_detection_statistic_13_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_13_reg);
	im_di_pan_detection_statistic_14_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_14_reg);
	im_di_pan_detection_statistic_15_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_15_reg);
	im_di_pan_detection_statistic_16_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_16_reg);
	im_di_pan_detection_statistic_17_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_17_reg);
	di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);

	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[0] = im_di_pan_detection_statistic_9_reg.hmc_r16_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[1] = im_di_pan_detection_statistic_9_reg.hmc_r15_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[2] = im_di_pan_detection_statistic_10_reg.hmc_r14_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[3] = im_di_pan_detection_statistic_10_reg.hmc_r13_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[4] = im_di_pan_detection_statistic_11_reg.hmc_r12_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[5] = im_di_pan_detection_statistic_11_reg.hmc_r11_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[6] = im_di_pan_detection_statistic_12_reg.hmc_r10_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[7] = im_di_pan_detection_statistic_12_reg.hmc_r9_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[8] = im_di_pan_detection_statistic_13_reg.hmc_r8_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[9] = im_di_pan_detection_statistic_1_reg.hmc_r7_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[10] = im_di_pan_detection_statistic_1_reg.hmc_r6_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[11] = im_di_pan_detection_statistic_2_reg.hmc_r5_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[12] = im_di_pan_detection_statistic_2_reg.hmc_r4_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[13] = im_di_pan_detection_statistic_3_reg.hmc_r3_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[14] = im_di_pan_detection_statistic_3_reg.hmc_r2_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[15] = im_di_pan_detection_statistic_4_reg.hmc_r1_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[16] = im_di_pan_detection_statistic_4_reg.hmc_c0_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[17] = im_di_pan_detection_statistic_5_reg.hmc_l1_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[18] = im_di_pan_detection_statistic_5_reg.hmc_l2_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[19] = im_di_pan_detection_statistic_6_reg.hmc_l3_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[20] = im_di_pan_detection_statistic_6_reg.hmc_l4_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[21] = im_di_pan_detection_statistic_7_reg.hmc_l5_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[22] = im_di_pan_detection_statistic_7_reg.hmc_l6_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[23] = im_di_pan_detection_statistic_8_reg.hmc_l7_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[24] = im_di_pan_detection_statistic_13_reg.hmc_l8_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[25] = im_di_pan_detection_statistic_14_reg.hmc_l9_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[26] = im_di_pan_detection_statistic_14_reg.hmc_l10_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[27] = im_di_pan_detection_statistic_15_reg.hmc_l11_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[28] = im_di_pan_detection_statistic_15_reg.hmc_l12_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[29] = im_di_pan_detection_statistic_16_reg.hmc_l13_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[30] = im_di_pan_detection_statistic_16_reg.hmc_l14_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[31] = im_di_pan_detection_statistic_17_reg.hmc_l15_pixelsum;
	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[32] = im_di_pan_detection_statistic_17_reg.hmc_l16_pixelsum;

	if(di_im_di_hmc_pan_control_reg.pan_me_select_mv_hist ==0)
	{
		for (i=0; i<HMC_BIN_COUNT; i++)
		{
			sort_array[i] = Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[i];
			if (i == 16)
				sort_array[i] = 0; // zero mv don't sort
		}

		// sort
		for (i=0; i<HMC_BIN_COUNT-1; i++)
		{
			m = i;
			for (j=i+1; j<HMC_BIN_COUNT; j++)
			{
				if (sort_array[j] > sort_array[m])
					m = j;
			}

			//swap
			if (i != m)
			{
				tmp_value = sort_array[i];
				sort_array[i] = sort_array[m];
				sort_array[m] = tmp_value;

				tmp_index = i;
				Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[i] = m;
				Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[m] = tmp_index;
			}
		}
	}
	else
	{
		for (i=0; i<HMC_BIN_COUNT; i++)
		{
			sort_array[i] = Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[i];
			if (i == 16)
				sort_array[i] = 0xffff; // zero mv don't sort
		}

		// sort
		for (i=0; i<HMC_BIN_COUNT-1; i++)
		{
			m = i;
			for (j=i+1; j<HMC_BIN_COUNT; j++)
			{
				if (sort_array[j] < sort_array[m])
					m = j;
			}

			//swap
			if (i != m)
			{
				tmp_value = sort_array[i];
				sort_array[i] = sort_array[m];
				sort_array[m] = tmp_value;

				tmp_index = i;
				Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[i] = m;
				Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Rank[m] = tmp_index;
			}
		}
	}
}

void scalerVIP_DI_PAN_HistSwitch(void)
{
	static unsigned int switchCnt=0;
	di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;
#if 0
	di_im_di_hmc_pan_dummy_2_RBUS di_im_di_hmc_pan_dummy_2_reg;
	di_im_di_hmc_pan_dummy_2_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_DUMMY_2_reg);
#else // henry merlin3 tmp
	unsigned char panConti = 0;
	di_di_smd_dummy_RBUS di_di_smd_dummy_reg;
	di_di_smd_dummy_reg.regValue = rtd_inl(DI_DI_SMD_dummy_reg);
	panConti = (di_di_smd_dummy_reg.dummy18024540_31_2 & 0x3FC00) >> 8; // [17:10]
#endif
	di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);

	Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag_pre = Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag;

	if (Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag == 0 && Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str == 1)
	{
		Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag = 1;
	}
#if	0
	else if (Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag == 1 && Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str == 0 && switchCnt>di_im_di_hmc_pan_dummy_2_reg.panConti+10)
#else // henry merlin3 tmp
	else if (Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag == 1 && Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str == 0 && switchCnt>panConti+10)
#endif
	{
		Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag = 0;
	}

	if(Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag == 0)
		switchCnt=0;
	else
#if 0
		switchCnt=(switchCnt>=(di_im_di_hmc_pan_dummy_2_reg.panConti+20))? (di_im_di_hmc_pan_dummy_2_reg.panConti+20):switchCnt+1;
#else
		switchCnt=(switchCnt>=(panConti+20))? (panConti+20):switchCnt+1;
#endif

	di_im_di_hmc_pan_control_reg.pan_me_select_mv_hist = Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag;

	rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_reg, di_im_di_hmc_pan_control_reg.regValue);

	if (Smart_Integration_Status.SCREEN_HMC_STATUS.Show_Debug_MSG)
	{
		VIP_DEBUG_PRINTF("[SIS HMC] Hist_flag: %d, switchCnt: %d\n", Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag, switchCnt);
	}
}

void scalerVIP_DI_PAN_StatusUpdate(void)
{

	unsigned int temp = 0, MaxValue_Pan= 0, SecondMaxValue_Pan = 0, Sum_Pan_Count =0, Pan_Count_Ratio = 0;
	unsigned char MAX_Pan_index = 0, SecondMax_Pan_index = 0, i=0;

	//condiction defined
	static unsigned int Pan_ratio_th = 50;
	unsigned char Pan_counter_th_str = 5;//,Pan_counter_th_upbound = 10;
	unsigned char Pan_counter_th = 5;//,Pan_counter_th_upbound = 10;

	//	chris added from Mac3 - 20140924
	static unsigned char pan_debounce_str=0;
	static unsigned char pan_debounce=0;

	unsigned char pan_mv_loc=0,restIdx=0,pass=0;
	unsigned int diffMinThd=0,restIdxThd=0,diffMaxMin=0;
	unsigned char reg_pan_debounce_str=10,reg_pan_debounce=20;

	int sort_array[HMC_BIN_COUNT];
	unsigned char sort_array_index[HMC_BIN_COUNT];
	// henry 161104
	int avg_Pan_count;
	int tmp_var = 0, var_Pan_count = 0;
	int j,m,tmp,tmp_loc;
	unsigned char hPanCnt_str=0;		//	for pan compensation
	unsigned char hPanCnt=0;		//	for HMCNR stable
	_RPC_system_setting_info* RPC_VIP_system_info_structure_table = NULL;

	di_im_di_hmc_pan_control_RBUS di_im_di_hmc_pan_control_reg;
#if	1
	vip_di_im_di_hmc_pan_dummy_1_RBUS di_im_di_hmc_pan_dummy_1_reg;
	vip_di_im_di_hmc_pan_dummy_2_RBUS di_im_di_hmc_pan_dummy_2_reg;
	vip_di_im_di_hmc_pan_dummy_3_RBUS di_im_di_hmc_pan_dummy_3_reg;
	di_im_di_hmc_pan_dummy_1_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_DUMMY_1_reg);
	di_im_di_hmc_pan_dummy_2_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_DUMMY_2_reg);
	di_im_di_hmc_pan_dummy_3_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_DUMMY_3_reg);

	diffMinThd = di_im_di_hmc_pan_dummy_1_reg.diffMinThd;
	restIdx = di_im_di_hmc_pan_dummy_1_reg.restIdx;
	restIdxThd = di_im_di_hmc_pan_dummy_1_reg.restIdxThd;
	diffMaxMin = di_im_di_hmc_pan_dummy_2_reg.diffMaxMin;
	Pan_counter_th_str = di_im_di_hmc_pan_dummy_2_reg.panConti_str;
	Pan_counter_th = di_im_di_hmc_pan_dummy_2_reg.panConti;
	reg_pan_debounce_str = di_im_di_hmc_pan_dummy_3_reg.panDeb_str;
	reg_pan_debounce = di_im_di_hmc_pan_dummy_3_reg.panDeb;
#else // henry merlin3 tmp
	di_im_di_rtnr_ediff_sobel_th_RBUS di_rtnr_ediff_sobel_th_reg;
	di_di_smd_dummy_RBUS di_di_smd_dummy_reg;
	di_im_di_film_motion_sum_th_RBUS di_im_di_film_motion_sum_th_reg;
//	di_im_di_si_film_final_result_RBUS di_im_di_si_film_final_result_reg;
	od_od_ctrl_RBUS od_ctrl_reg;
	di_im_di_rtnr_hmcnr_statistic_6_RBUS di_im_di_rtnr_hmcnr_statistic_6_reg;
	di_im_di_rtnr_refined_ctrl2_RBUS di_im_di_rtnr_refined_ctrl2_reg;
	di_rtnr_ediff_sobel_th_reg.regValue = rtd_inl(DI_IM_DI_RTNR_EDIFF_SOBEL_TH_reg);
	di_di_smd_dummy_reg.regValue = rtd_inl(DI_DI_SMD_dummy_reg);
	di_im_di_film_motion_sum_th_reg.regValue = rtd_inl(DI_IM_DI_FILM_MOTION_SUM_TH_reg);
//	di_im_di_si_film_final_result_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_FINAL_RESULT_reg);
	od_ctrl_reg.regValue = rtd_inl(OD_OD_CTRL_reg);
	di_im_di_rtnr_hmcnr_statistic_6_reg.regValue = rtd_inl(DI_IM_DI_RTNR_HMCNR_STATISTIC_6_reg);
	di_im_di_rtnr_refined_ctrl2_reg.regValue = rtd_inl(DI_IM_DI_RTNR_REFINED_CTRL2_reg);

	diffMinThd = di_rtnr_ediff_sobel_th_reg.dummy180241d0_31_22;
	restIdx = di_im_di_film_motion_sum_th_reg.dummy180240b4_23_16;
//	restIdxThd = (di_im_di_si_film_final_result_reg.dummy180240c4_31_9 & 0xFFF0) >> 4; // 16 bits [28:13]
	restIdxThd = (od_ctrl_reg.dummy1802ca00_31_2 & 0xFFFF0000) >> 16; // 16 bits [28:13]
	diffMaxMin = di_im_di_rtnr_hmcnr_statistic_6_reg.dummy18024858_15_0;
	Pan_counter_th_str = di_im_di_rtnr_refined_ctrl2_reg.dummy180242ac_31_17 & 0xFF; // 8 bits [24:17]
	Pan_counter_th = (di_di_smd_dummy_reg.dummy18024540_31_2 & 0xFF00) >> 8; // 8 bits [17:10];
	reg_pan_debounce_str = (di_di_smd_dummy_reg.dummy18024540_31_2 & 0xFF); // 8 bits [9:2];
	reg_pan_debounce = (di_di_smd_dummy_reg.dummy18024540_31_2 & 0xFF0000) >> 16; // 8 bits [25:18];
#endif

	di_im_di_hmc_pan_control_reg.regValue = rtd_inl(DI_IM_DI_HMC_PAN_CONTROL_reg);

	if(Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag_pre != Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag)
		return;

	/* not used
	reg_pan_debounce_str = di_im_di_hmc_pan_dummy_3_reg.dummy18024684_31_0 & 0x0000ff00;
	reg_pan_debounce = di_im_di_hmc_pan_dummy_3_reg.dummy18024684_31_0 & 0x000000ff;
	reg_pan_debounce_str = 20;
	reg_pan_debounce = 20;
	diffMinThd = di_im_di_hmc_pan_dummy_1_reg.dummy1802467c_31_0 & 0x3ff;
	restIdx = di_im_di_hmc_pan_dummy_1_reg.dummy1802467c_31_0 & 0xfc00;
	restIdxThd = di_im_di_hmc_pan_dummy_1_reg.dummy1802467c_31_0 & 0xffff0000;
	diffMaxMin = di_im_di_hmc_pan_dummy_2_reg.dummy18024680_31_0 & 0xffff;
	Pan_counter_th = di_im_di_hmc_pan_dummy_2_reg.dummy18024680_31_0 & 0xff0000;
	*/
	restIdx = (restIdx>=HMC_BIN_COUNT-1)? HMC_BIN_COUNT-1:restIdx;

	if(di_im_di_hmc_pan_control_reg.pan_me_select_mv_hist ==0)
	{
		//	original decision
		for(i=0; i<HMC_BIN_COUNT; i++)
		{
			temp = Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[i]; // must confirm scalerVIP_DI_HMC_StatusUpdate() processing correctly

			Sum_Pan_Count =Sum_Pan_Count + temp;

			if(MaxValue_Pan <= temp)
			{
				SecondMaxValue_Pan = MaxValue_Pan;
				SecondMax_Pan_index = MAX_Pan_index;
				MaxValue_Pan = temp;
				MAX_Pan_index = i;
			}
			else if(SecondMaxValue_Pan <= temp)
			{
				SecondMaxValue_Pan = temp;
				SecondMax_Pan_index = i;
			}
		}
		pan_mv_loc = MAX_Pan_index;

		if(Sum_Pan_Count!=0)
			Pan_Count_Ratio = (MaxValue_Pan*100)/Sum_Pan_Count ;

		if(MAX_Pan_index!=16)
		{
			if(Pan_Count_Ratio>=Pan_ratio_th)
			{
				pass = 1;
				//Pan_ratio_counter = (Pan_ratio_counter>=Pan_counter_th_upbound)?Pan_counter_th_upbound:(Pan_ratio_counter+1);
			}
			else
			{
				pass = 0;
				//Pan_ratio_counter = (Pan_ratio_counter<=0)?0:(Pan_ratio_counter-1);
			}
		}
		else
		{
			pass = 0;
			//Pan_ratio_counter = (Pan_ratio_counter<=0)?0:(Pan_ratio_counter-1);
		}

		/*
		if(Pan_ratio_counter > Pan_counter_th)
		{
			Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str = 1;
			Pan_ratio_th = 40;
		}
		else
		{
			Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str = 0;
			Pan_ratio_th =70;
		}
		*/

		/* rock_rau 20150923 [special case] Q-tek pan Horizontal DI need trend 3D setting*/
		/* for Q-tek monoscope pan pattern*/
		if(Sum_Pan_Count != 0)
		{
			if((MAX_Pan_index==0 && SecondMax_Pan_index==1) || (MAX_Pan_index==32 && SecondMax_Pan_index==31)) {
				Smart_Integration_Status.SCREEN_HPAN_STATUS.Pen_special_case = (((MaxValue_Pan+SecondMaxValue_Pan)*100)/Sum_Pan_Count);
			} else {
				Smart_Integration_Status.SCREEN_HPAN_STATUS.Pen_special_case = 0;
			}
		}
	}
	else
	{
		for(i=0;i<HMC_BIN_COUNT;i++)
		{
			sort_array[i]=Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram[i];
			sort_array_index[i] = i;
			// henry 161104
			Sum_Pan_Count =Sum_Pan_Count + sort_array[i];
		}

		// henry 161104
		avg_Pan_count = Sum_Pan_Count/HMC_BIN_COUNT;
		for(i=0;i<HMC_BIN_COUNT;i++)
		{
			tmp_var += abs(sort_array[i]-avg_Pan_count);
		}
		var_Pan_count = tmp_var/HMC_BIN_COUNT;
		diffMinThd = (2*avg_Pan_count - 7*var_Pan_count) >> 1;

		// DOS case setting
		RPC_VIP_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();
		if( RPC_VIP_system_info_structure_table == NULL )
		{
			if( MA_print_count%360 == 0 )
				ROSPrintf(" share Memory PQ data ERROR, RPC_system_info = %p\n", RPC_VIP_system_info_structure_table);
			return;
		}

		switch( RPC_VIP_system_info_structure_table->VIP_source )
		{
			case VIP_QUALITY_CVBS_NTSC:
			case VIP_QUALITY_CVBS_PAL:
			case VIP_QUALITY_HDMI_480I:
			case VIP_QUALITY_YPbPr_480I:
			case VIP_QUALITY_HDMI_576I:
			case VIP_QUALITY_YPbPr_576I:
				if( DynamicOptimizeSystem[7] == 1 || DynamicOptimizeSystem[75] == 1 )
					diffMinThd = 1023;
				break;
			default:
				break;
		}

		// Write information register value protection
#if 1
		di_im_di_hmc_pan_dummy_1_reg.diffMinThd = (diffMinThd < 0)? 0 : ((diffMinThd>1023)? 1023 : diffMinThd);
		rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_1_reg, di_im_di_hmc_pan_dummy_1_reg.regValue);
		di_im_di_hmc_pan_dummy_2_reg.diffMaxMin = diffMaxMin;
		rtd_outl(DI_IM_DI_HMC_PAN_DUMMY_2_reg, di_im_di_hmc_pan_dummy_2_reg.regValue);
#else // henry merlin3 temp
		di_rtnr_ediff_sobel_th_reg.dummy180241d0_31_22 = (diffMinThd < 0)? 0 : ((diffMinThd>1023)? 1023 : diffMinThd);
		rtd_outl(DI_IM_DI_RTNR_EDIFF_SOBEL_TH_reg, di_rtnr_ediff_sobel_th_reg.regValue);
		di_im_di_rtnr_hmcnr_statistic_6_reg.dummy18024858_15_0 = diffMaxMin;
		rtd_outl(DI_IM_DI_RTNR_HMCNR_STATISTIC_6_reg, di_im_di_rtnr_hmcnr_statistic_6_reg.regValue);
#endif
		diffMaxMin = avg_Pan_count;


		for (i=0; i<HMC_BIN_COUNT-1; i++)
		{
			m = i;
			for (j=i+1; j<HMC_BIN_COUNT; j++)
			{
				if (sort_array[j] < sort_array[m])
					m = j;
			}
			//swap
			if (i != m)
			{
				tmp = sort_array[i];
				sort_array[i] = sort_array[m];
				sort_array[m] = tmp;
				tmp_loc = sort_array_index[i];
				sort_array_index[i] = sort_array_index[m];
				sort_array_index[m] = tmp_loc;
			}
		}
		pan_mv_loc = sort_array_index[0];

		if(((sort_array[0] < diffMinThd && sort_array[restIdx] > restIdxThd) ||
		//	modified by chris for a moving windows on a still pic (pattern gen) - start
		(((sort_array[0] * 15) < sort_array[1]*10 && (sort_array[0] * 24) < sort_array[2]*10 && (sort_array[0] * 3) < sort_array[3])
		&& (sort_array[0] < diffMinThd+200))) // 960 condition
			/*(((sort_array[0] * 1.5) < sort_array[1] && (sort_array[0] * 2.4) < sort_array[2] && (sort_array[0] * 3.0) < sort_array[3]) // 960 condition
			) )*/
		//	&& sort_array_index[1] !=16
		//	&& sort_array_index[2] !=16
		//	modified by chris for a moving windows on a still pic (pattern gen) - end
			&& sort_array_index[0] !=16
			&& (abs(sort_array[0]-sort_array[32]) > diffMaxMin)
		)
			pass = 1;
		else
			pass = 0;

		if(pan_mv_loc==16)
			pass = 0;
	}

	di_im_di_hmc_pan_control_reg.pan_me_select_mv_hist = Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag;

	rtd_outl(DI_IM_DI_HMC_PAN_CONTROL_reg, di_im_di_hmc_pan_control_reg.regValue);

	if(pan_mv_loc<0)
		pan_mv_loc=0;
	if(pan_mv_loc>HMC_BIN_COUNT-1)
		pan_mv_loc = HMC_BIN_COUNT-1;

	//	Update to SCREEN_HPAN_STATUS
	Smart_Integration_Status.SCREEN_HPAN_STATUS.Pan_det = pass;
	Smart_Integration_Status.SCREEN_HPAN_STATUS.Pan_mv = pan_mv_loc;

	// assign pass sequence
	for (i=HPAN_PASS_COUNT-1; i>0; i--)
	{
		Smart_Integration_Status.SCREEN_HPAN_STATUS.Pass_Result_Sequence[i] = Smart_Integration_Status.SCREEN_HPAN_STATUS.Pass_Result_Sequence[i-1];
		Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[i] = Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[i-1];
	}
	Smart_Integration_Status.SCREEN_HPAN_STATUS.Pass_Result_Sequence[0] = pass;
	Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[0] = pan_mv_loc;

	//	Update to SCREEN_ACTION_STATUS
	Pan_counter_th_str = (Pan_counter_th_str>(HPAN_PASS_COUNT-2))? (HPAN_PASS_COUNT-2):Pan_counter_th_str;

	for (i=0; i<=Pan_counter_th_str; i++)
	{
		if((Smart_Integration_Status.SCREEN_HPAN_STATUS.Pass_Result_Sequence[i] == 1) &&
		(Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[i] == Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[i+1]))//][MIN(i+1,HPAN_PASS_COUNT-1)]))
		{
			hPanCnt_str++;
		}
	}

	/*unsigned char pan_exit_flg=0;
	if((abs(Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[0]-Smart_Integration_Status.SCREEN_HPAN_STATUS.Mv_Result_Sequence[1])>=8) ||
	())
		pan_exit_flg=1;

	if(pan_exit_flg)
	{
	}
	else */if(hPanCnt_str == (Pan_counter_th_str+1))
	{
		Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str = 1;
		Pan_ratio_th = 40;
		pan_debounce_str = reg_pan_debounce_str;
	}
	else
	{
		pan_debounce_str = (pan_debounce_str<=0)? 0: pan_debounce_str-1;
		if(pan_debounce_str==0)
		{
			Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str = 0;
			Pan_ratio_th = 50;
		}
	}


	Pan_counter_th = (Pan_counter_th>(HPAN_PASS_COUNT-1))? (HPAN_PASS_COUNT-1):Pan_counter_th;
	for (i=0; i<=Pan_counter_th; i++)
	{
		if(Smart_Integration_Status.SCREEN_HPAN_STATUS.Pass_Result_Sequence[i] == 1)
		{
			hPanCnt++;
		}
	}

	if(hPanCnt == (Pan_counter_th+1))
	{
		Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect = 1;
		pan_debounce = reg_pan_debounce;
	}
	else
	{
		pan_debounce = (pan_debounce<=0)? 0: pan_debounce-1;
		if(pan_debounce==0)
		{
			Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect = 0;
			Pan_ratio_th = 50;
		}
	}

	if (Smart_Integration_Status.SCREEN_HPAN_STATUS.Show_Debug_MSG)
	{
		if (Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag == 0)
		{
			VIP_DEBUG_PRINTF("[SIS PAN 1] max: %d:%d, second: %d:%d, ratio: %d, pass : %d\n", MaxValue_Pan, MAX_Pan_index, SecondMaxValue_Pan, SecondMax_Pan_index, Pan_Count_Ratio, pass);
		}
		else
		{
			VIP_DEBUG_PRINTF("[SIS PAN 2] min:%d %d,rest:%d %d, diff:%d %d, pass : %d\n", sort_array[0],diffMinThd,sort_array[restIdx],restIdxThd,abs(sort_array[0]-sort_array[32]),diffMaxMin, pass);
		}
		VIP_DEBUG_PRINTF("[SIS PAN] mv:%d HstFlg:%d, Pan det:%d %d Deb:%d %d\n",
			HMC_BIN_COUNT -(Smart_Integration_Status.SCREEN_HPAN_STATUS.Pan_mv),
			Smart_Integration_Status.SCREEN_HMC_STATUS.BIN_Histogram_flag,
			Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect_str,
			Smart_Integration_Status.SCREEN_ACTION_STATUS.PAN_detect,pan_debounce_str,pan_debounce);
	}
}

void scalerVIP_DI_VPAN_StatusUpdate(void)
{
	short i = 0;
	unsigned char vpan_up_pass_total = 0, vpan_down_pass_total = 0, pass = 0;
	di_di_smd_gmvcoeff3_RBUS smd_gmvcoeff3_reg;

	smd_gmvcoeff3_reg.regValue = rtd_inl(DI_DI_SMD_GMVCoeff3_reg);

	if (smd_gmvcoeff3_reg.smd_gmv1_x == 3 && smd_gmvcoeff3_reg.smd_gmv1_y < 3)
		pass = 2; // move up
	else if (smd_gmvcoeff3_reg.smd_gmv1_x == 3 && smd_gmvcoeff3_reg.smd_gmv1_y > 3)
		pass = 1; // move down
	else
		pass = 0;

	// assign pass sequence
	for (i=VPAN_PASS_COUNT-2; i>=0; i--){
		Smart_Integration_Status.SCREEN_VPAN_STATUS.Pass_Result_Sequence[i+1] = Smart_Integration_Status.SCREEN_VPAN_STATUS.Pass_Result_Sequence[i];
	}
	Smart_Integration_Status.SCREEN_VPAN_STATUS.Pass_Result_Sequence[0] = pass;

	for (i=0; i<VPAN_PASS_COUNT; i++)
	{
		if (Smart_Integration_Status.SCREEN_VPAN_STATUS.Pass_Result_Sequence[i] == 2)
			vpan_up_pass_total++;
		else if (Smart_Integration_Status.SCREEN_VPAN_STATUS.Pass_Result_Sequence[i] == 1)
			vpan_down_pass_total++;
	}

	Smart_Integration_Status.SCREEN_VPAN_STATUS.Pass_Result_Total_VPanUp = vpan_up_pass_total;
	Smart_Integration_Status.SCREEN_VPAN_STATUS.Pass_Result_Total_VPanDown = vpan_down_pass_total;

	if (vpan_up_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.VPAN_detect = 2;
	else if (vpan_down_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.VPAN_detect = 1;
	else
		Smart_Integration_Status.SCREEN_ACTION_STATUS.VPAN_detect = 0;

}

void scalerVIP_DI_SPAN_StatusUpdate(void)
{
	short i = 0;
	unsigned char span_RU_pass_total = 0, span_RD_pass_total = 0, span_LU_pass_total = 0, span_LD_pass_total = 0, pass = 0;
	unsigned int smd_sad_bin_total = 0, smd_sad_bin_first = 0;
	di_di_smd_gmvcoeff3_RBUS smd_gmvcoeff3_reg;

	di_di_smd_sadhisbina_RBUS di_smd_sadhistbina_reg;
	di_di_smd_sadhisbinb_RBUS di_smd_sadhistbinb_reg;
	di_di_smd_sadhisbinc_RBUS di_smd_sadhistbinc_reg;
	di_di_smd_sadhisbind_RBUS di_smd_sadhistbind_reg;
	di_di_smd_sadhisbine_RBUS di_smd_sadhistbine_reg;
	di_di_smd_sadhisbinf_RBUS di_smd_sadhistbinf_reg;
	di_di_smd_sadhisbing_RBUS di_smd_sadhistbing_reg;
	di_di_smd_sadhisbinh_RBUS di_smd_sadhistbinh_reg;

	di_smd_sadhistbina_reg.regValue = rtd_inl(DI_DI_SMD_SADHisBinA_reg);
	di_smd_sadhistbinb_reg.regValue = rtd_inl(DI_DI_SMD_SADHisBinB_reg);
	di_smd_sadhistbinc_reg.regValue = rtd_inl(DI_DI_SMD_SADHisBinC_reg);
	di_smd_sadhistbind_reg.regValue = rtd_inl(DI_DI_SMD_SADHisBinD_reg);
	di_smd_sadhistbine_reg.regValue = rtd_inl(DI_DI_SMD_SADHisBinE_reg);
	di_smd_sadhistbinf_reg.regValue = rtd_inl(DI_DI_SMD_SADHisBinF_reg);
	di_smd_sadhistbing_reg.regValue = rtd_inl(DI_DI_SMD_SADHisBinG_reg);
	di_smd_sadhistbinh_reg.regValue = rtd_inl(DI_DI_SMD_SADHisBinH_reg);

	smd_sad_bin_total = di_smd_sadhistbina_reg.smd_sad_his_bin0 + di_smd_sadhistbina_reg.smd_sad_his_bin1 +
						di_smd_sadhistbinb_reg.smd_sad_his_bin2 + di_smd_sadhistbinb_reg.smd_sad_his_bin3 +
						di_smd_sadhistbinc_reg.smd_sad_his_bin4 + di_smd_sadhistbinc_reg.smd_sad_his_bin5 +
						di_smd_sadhistbind_reg.smd_sad_his_bin6 + di_smd_sadhistbind_reg.smd_sad_his_bin7 +
						di_smd_sadhistbine_reg.smd_sad_his_bin8 + di_smd_sadhistbine_reg.smd_sad_his_bin9 +
						di_smd_sadhistbinf_reg.smd_sad_his_bin10 + di_smd_sadhistbinf_reg.smd_sad_his_bin11 +
						di_smd_sadhistbing_reg.smd_sad_his_bin12 + di_smd_sadhistbing_reg.smd_sad_his_bin13 +
						di_smd_sadhistbinh_reg.smd_sad_his_bin14 + di_smd_sadhistbinh_reg.smd_sad_his_bin15;

	smd_sad_bin_first = di_smd_sadhistbina_reg.smd_sad_his_bin0;

	smd_gmvcoeff3_reg.regValue = rtd_inl(DI_DI_SMD_GMVCoeff3_reg);

	if (smd_gmvcoeff3_reg.smd_gmv1_x > 3 && smd_gmvcoeff3_reg.smd_gmv1_y > 3)
		pass = 4; // right-down
	else if (smd_gmvcoeff3_reg.smd_gmv1_x > 3 && smd_gmvcoeff3_reg.smd_gmv1_y < 3)
		pass = 3; // right-up
	else if (smd_gmvcoeff3_reg.smd_gmv1_x < 3 && smd_gmvcoeff3_reg.smd_gmv1_y > 3)
		pass = 2; // left-down
	else if (smd_gmvcoeff3_reg.smd_gmv1_x < 3 && smd_gmvcoeff3_reg.smd_gmv1_y < 3)
		pass = 1; // left-up
	else
		pass = 0;

	if ((smd_sad_bin_first*100/(smd_sad_bin_total+1)) < 95)
		pass = 0;

	// assign pass sequence
	for (i=SPAN_PASS_COUNT-2; i>=0; i--)
		Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Sequence[i+1] = Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Sequence[i];
	Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Sequence[0] = pass;

	for (i=0; i<SPAN_PASS_COUNT; i++)
	{
		if (Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Sequence[i] == 4)
			span_RD_pass_total++;
		else if (Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Sequence[i] == 3)
			span_RU_pass_total++;
		else if (Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Sequence[i] == 2)
			span_LD_pass_total++;
		else if (Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Sequence[i] == 1)
			span_LU_pass_total++;
	}

	Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Total_SPanRU = span_RU_pass_total;
	Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Total_SPanRD = span_RD_pass_total;
	Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Total_SPanLU = span_LU_pass_total;
	Smart_Integration_Status.SCREEN_SPAN_STATUS.Pass_Result_Total_SPanLD = span_LD_pass_total;

	if (span_RD_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.SPAN_detect = 4;
	else if (span_RU_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.SPAN_detect = 3;
	else if (span_LD_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.SPAN_detect = 2;
	else if (span_LU_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.SPAN_detect = 1;
	else
		Smart_Integration_Status.SCREEN_ACTION_STATUS.SPAN_detect = 0;

	if (Smart_Integration_Status.SCREEN_SPAN_STATUS.Show_Debug_MSG)
	{
		VIP_DEBUG_PRINTF("[SIS SPAN] RD: %d, RU: %d, LD: %d, LU: %d, pass: %d\n", span_RD_pass_total, span_RU_pass_total, span_LD_pass_total, span_LU_pass_total, pass);
	}
}

// chen 170522
#if 0
void scalerVIP_DI_ZOOMMOTION_StatusUpdate()
{

	short i = 0;
	unsigned char zoomin_pass_total = 0, zoomout_pass_total = 0;
	di_im_di_zoommotion_detector_RBUS zoommotion_detector_reg;

	zoommotion_detector_reg.regValue = rtd_inl(DI_IM_DI_ZoomMotion_Detector_reg);

	// assign pass sequence
	for (i=ZOOMMOTION_PASS_COUNT-2; i>=0; i--)
		Smart_Integration_Status.SCREEN_ZOOMMOTION_STATUS.Pass_Result_Sequence[i+1] = Smart_Integration_Status.SCREEN_ZOOMMOTION_STATUS.Pass_Result_Sequence[i];
	Smart_Integration_Status.SCREEN_ZOOMMOTION_STATUS.Pass_Result_Sequence[0] = zoommotion_detector_reg.smd_zoommotion_flag;

	for (i=0; i<ZOOMMOTION_PASS_COUNT; i++)
	{
		if (Smart_Integration_Status.SCREEN_ZOOMMOTION_STATUS.Pass_Result_Sequence[i] == 1)
			zoomout_pass_total++;
		else if (Smart_Integration_Status.SCREEN_ZOOMMOTION_STATUS.Pass_Result_Sequence[i] == 2)
			zoomin_pass_total++;
	}

	Smart_Integration_Status.SCREEN_ZOOMMOTION_STATUS.Pass_Result_Total_ZoomIn = zoomin_pass_total;
	Smart_Integration_Status.SCREEN_ZOOMMOTION_STATUS.Pass_Result_Total_ZoomOut = zoomout_pass_total;

	if (zoomin_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.ZOOMMOTION_detect = 2;
	else if (zoomout_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.ZOOMMOTION_detect = 1;
	else
		Smart_Integration_Status.SCREEN_ACTION_STATUS.ZOOMMOTION_detect = 0;
}
#endif
// end chen 170522

void scalerVIP_DI_SLOWMOTION_StatusUpdate(void)
{
	short i = 0;
	unsigned char slow_pass_total = 0;
	di_im_di_slowmotion_detector_1_RBUS slowmotion_detector_1_reg;

	slowmotion_detector_1_reg.regValue = rtd_inl(DI_IM_DI_SlowMotion_Detector_1_reg);

	// assign pass sequence
	for (i=SLOWMOTION_PASS_COUNT-2; i>=0; i--)
		Smart_Integration_Status.SCREEN_SLOWMOTION_STATUS.Pass_Result_Sequence[i+1] = Smart_Integration_Status.SCREEN_SLOWMOTION_STATUS.Pass_Result_Sequence[i];
	Smart_Integration_Status.SCREEN_SLOWMOTION_STATUS.Pass_Result_Sequence[0] = slowmotion_detector_1_reg.smd_slowmotion_flag;

	for (i=0; i<SLOWMOTION_PASS_COUNT; i++)
	{
		slow_pass_total += Smart_Integration_Status.SCREEN_SLOWMOTION_STATUS.Pass_Result_Sequence[i];
	}

	Smart_Integration_Status.SCREEN_SLOWMOTION_STATUS.Pass_Result_Total_Slow = slow_pass_total;

	if (Smart_Integration_Status.SCREEN_SLOWMOTION_STATUS.Show_Debug_MSG)
	{
		VIP_DEBUG_PRINTF("[SIS SLOWMOTION] slow_pass_total: %d\n", slow_pass_total);
	}

	if (slow_pass_total > 5)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.SLOWMOTION_detect = 1;
	else
		Smart_Integration_Status.SCREEN_ACTION_STATUS.SLOWMOTION_detect = 0;
}

void scalerVIP_DI_SCENECHANGE_StatusUpdate(void)
{
	short i = 0;
	unsigned char scenechange_pass_total = 0;
	unsigned int near_compare_diff = 0, far_compare_diff = 0;
	unsigned int total_energy = 0;

	for (i=0; i<HISTOGRAM_SIZE; i++)
		total_energy += Smart_Integration_Status.Y_Histogram[i]*i;

	// because result is from Y Histogram, so MA info must delay 2 field, so pass sequence [0] and [1].
	near_compare_diff = abs(Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[2]-Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[3]);
	for (i=3; i<SCENECHANGE_COUNT-1; i++)
	{
		far_compare_diff = abs(Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[i]-Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[i+1]);
		if (near_compare_diff > (Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.pass_ratio*far_compare_diff) && near_compare_diff > 2000)
			scenechange_pass_total++;
	}

	for (i=SCENECHANGE_COUNT-2; i>=0; i--)
		Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Result_Sequence[i+1] = Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Result_Sequence[i];
	Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Result_Sequence[0] = total_energy;

	if (scenechange_pass_total >= Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.pass_couter &&
		abs(Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Result_Sequence[0]-Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Result_Sequence[4]) > 3500)
	{
		Smart_Integration_Status.SCREEN_ACTION_STATUS.SCENECHANGE_detect = 1;

		if (Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Show_Debug_MSG)
		{
			VIP_DEBUG_PRINTF("[SIS SCENECHANGE] near: %d, far: %d, %d, %d, %d, Y diff: %d\n", near_compare_diff, far_compare_diff,
				abs(Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[1]-Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[2]),
				abs(Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[2]-Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[3]),
				abs(Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[3]-Smart_Integration_Status.SCREEN_MOTION_STATUS.Large_FM_SUM_Sequence[4]),
				abs(Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Result_Sequence[0]-Smart_Integration_Status.SCREEN_SCENECHANGE_STATUS.Result_Sequence[4]));
		}
	}
	else
		Smart_Integration_Status.SCREEN_ACTION_STATUS.SCENECHANGE_detect = 0;
}

void scalerVIP_DI_FADING_StatusUpdate(void)
{
	short i = 0;
	unsigned char fadein_pass_total = 0, fadeout_pass_total = 0;
	unsigned int total_energy = 0;

	for (i=0; i<HISTOGRAM_SIZE; i++)
		total_energy += Smart_Integration_Status.Y_Histogram[i]*i;

	// assign pass sequence
	for (i=FADING_PASS_COUNT-2; i>=0; i--)
		Smart_Integration_Status.SCREEN_FADING_STATUS.Result_Sequence[i+1] = Smart_Integration_Status.SCREEN_FADING_STATUS.Result_Sequence[i];
	Smart_Integration_Status.SCREEN_FADING_STATUS.Result_Sequence[0] = total_energy;

	for (i=0; i<FADING_PASS_COUNT-4; i+=4)
	{
		if ((Smart_Integration_Status.SCREEN_FADING_STATUS.Result_Sequence[i] - Smart_Integration_Status.SCREEN_FADING_STATUS.Result_Sequence[i+4]) > 10000)
			fadein_pass_total++;
		else if ((Smart_Integration_Status.SCREEN_FADING_STATUS.Result_Sequence[i] - Smart_Integration_Status.SCREEN_FADING_STATUS.Result_Sequence[i+4]) < 10000)
			fadeout_pass_total++;
	}

//	VIP_DEBUG_PRINTF("[SIS System] fadein:%d, fadeout:%d: cur:%d, pre:%d\n", fadein_pass_total, fadeout_pass_total, Smart_Integration_Status.SCREEN_FADING_STATUS.Result_Sequence[0], Smart_Integration_Status.SCREEN_FADING_STATUS.Result_Sequence[4]);
/*	if (fadein_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.FADING_detect = 2;
	else if (fadeout_pass_total > 12)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.FADING_detect = 1;
	else
		Smart_Integration_Status.SCREEN_ACTION_STATUS.FADING_detect = 0;*/
}

void scalerVIP_DI_HIGHFREQUENCY_StatusUpdate(void)
{
	short i = 0;
	unsigned int total_pixel = 0, total_score_ratio = 0;
	di_im_di_active_window_control_RBUS di_active_window_control_reg;
	di_im_di_weave_window_control_RBUS di_weave_window_control_reg;
	di_im_di_hfd_statistic_RBUS di_hfd_statistic_reg;

	di_active_window_control_reg.regValue = rtd_inl(DI_IM_DI_ACTIVE_WINDOW_CONTROL_reg);
	di_weave_window_control_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);
	di_hfd_statistic_reg.regValue = rtd_inl(DI_IM_DI_HFD_STATISTIC_reg);

	total_pixel = Smart_Integration_Status.DI_WIDTH*Smart_Integration_Status.DI_HEIGHT;

	// assign status sequence
	for (i=HIGHFREQUENCY_PASS_COUNT-2; i>=0; i--)
		Smart_Integration_Status.SCREEN_HIGHFREQUENCY_STATUS.Pass_Result_Sequence[i+1] = Smart_Integration_Status.SCREEN_HIGHFREQUENCY_STATUS.Pass_Result_Sequence[i];
	Smart_Integration_Status.SCREEN_HIGHFREQUENCY_STATUS.Pass_Result_Sequence[0] = ((di_hfd_statistic_reg.hfd_statistic*HIGHFREQUENCY_SCALE)/(total_pixel==0?1:total_pixel));

	for (i=0; i<HIGHFREQUENCY_PASS_COUNT; i++)
	{
		total_score_ratio += Smart_Integration_Status.SCREEN_HIGHFREQUENCY_STATUS.Pass_Result_Sequence[i];
	}

	Smart_Integration_Status.SCREEN_HIGHFREQUENCY_STATUS.Pass_Result_FIR = total_score_ratio/HIGHFREQUENCY_PASS_COUNT;

	if (Smart_Integration_Status.SCREEN_HIGHFREQUENCY_STATUS.Pass_Result_FIR > 400)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.HIGHFREQUENCY_detect = 1;
	else
		Smart_Integration_Status.SCREEN_ACTION_STATUS.HIGHFREQUENCY_detect = 0;
}

// chen 170522
#if 0
void scalerVIP_NOISE_LEVEL_StatusUpdate()
{
	scalerVIP_SPATIAL_NOISE_LEVEL_StatusUpdate();
	scalerVIP_DI_TEMPORAL_NOISE_LEVEL_StatusUpdate();
}

void scalerVIP_SPATIAL_NOISE_LEVEL_StatusUpdate()
{
	short i = 0, cnt1 = 0, cnt2 = 0;
	unsigned char result1 = 0, result2 = 0;

	for (i=SPATIAL_NOISELEVEL_CNT-2; i>=0; i--)
	{
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL[i+1] = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL[i];
		cnt1 = cnt1 + Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL[i+1];
	}
	Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL[0] = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_CUR;
	cnt1 = cnt1 + Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL[0];


	for (i=SPATIAL_NOISELEVEL_RELIABLE_CNT-2; i>=0; i--)
	{
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_RELIABLE[i+1] = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_RELIABLE[i];
		cnt2 = cnt2 + Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_RELIABLE[i+1];
	}
	Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_RELIABLE[0] = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_RELIABLE_CUR;
	cnt2 = cnt2 + Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_RELIABLE[0];

	result1 = (unsigned char)(cnt1/SPATIAL_NOISELEVEL_CNT);
	if(result1 > 15)
		result1 = 15;
	else if(result1 < 0)
		result1 = 0;
	Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.RESULT_NOISE_LEVEL = result1;

	result2 = (unsigned char)(cnt2/SPATIAL_NOISELEVEL_RELIABLE_CNT);
	Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.RESULT_NOISE_LEVEL_RELIABLE = (result2 > 1)? 1:0;

	di_di_rtnr_nm_infor1_RBUS di_rtnr_nm_infor1_reg;
	di_rtnr_nm_infor1_reg.regValue = rtd_inl(DI_DI_RTNR_NM_INFOR1_reg);
	di_rtnr_nm_infor1_reg.nm_noiselevel = result1;

	//di_di_rtnr_nm_infor1_RBUS di_rtnr_nm_infor1_reg;
	//di_rtnr_nm_infor1_reg.regValue = rtd_inl(DI_DI_RTNR_NM_INFOR1_reg);
	unsigned int Sum;
	Sum = di_rtnr_nm_infor1_reg.nm_sumvarnum;
	if(Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.Show_Debug_MSG)
	{
		VIP_DEBUG_PRINTF("Cur NM NoiseLevel : %d , Reliable: %d \n",
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL[0],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_RELIABLE[0]);
		VIP_DEBUG_PRINTF("Result NM NoiseLevel : %d , Reliable: %d \n",
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.RESULT_NOISE_LEVEL,
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.RESULT_NOISE_LEVEL_RELIABLE);
		VIP_DEBUG_PRINTF("Cnt: %d, Height : %d, Width : %d\n", Sum,Smart_Integration_Status.DI_HEIGHT,Smart_Integration_Status.DI_WIDTH);
	}
	scaler_rtd_outl(DI_DI_RTNR_NM_INFOR1_reg, di_rtnr_nm_infor1_reg.regValue);
}

void scalerVIP_DI_TEMPORAL_NOISE_LEVEL_StatusUpdate()
{
	short i = 0;
	unsigned int total_pixel = 0;
	unsigned int et_y_count = 0, et_y_sum = 0;
	unsigned short result = 0, result2 = 0;

	di_im_di_rtnr_level_bound_RBUS di_rtnr_level_bound_reg;
	di_im_di_rtnr_et_count_y_RBUS di_rtnr_et_count_y_reg;
	di_im_di_rtnr_et_sum_y_RBUS di_rtnr_et_sum_y_reg;

	di_rtnr_level_bound_reg.regValue = rtd_inl(DI_IM_DI_RTNR_LEVEL_BOUND_reg);
	di_rtnr_level_bound_reg.cp_temporal_nl_low_bnd_y = 0;
	di_rtnr_level_bound_reg.cp_temporal_nl_up_bnd_y = 0xFF;
	rtd_outl(DI_IM_DI_RTNR_LEVEL_BOUND_reg, di_rtnr_level_bound_reg.regValue);

	di_rtnr_et_count_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_COUNT_Y_reg);
	et_y_count = di_rtnr_et_count_y_reg.cp_temporal_same_et_cnt_y;

	di_rtnr_et_sum_y_reg.regValue = rtd_inl(DI_IM_DI_RTNR_ET_SUM_Y_reg);
	et_y_sum = di_rtnr_et_sum_y_reg.cp_temporal_same_et_sad_sum_y;

	result = et_y_sum/(et_y_count+1);
	total_pixel = Smart_Integration_Status.DI_WIDTH*Smart_Integration_Status.DI_HEIGHT;

	// assign status sequence
	for (i=TEMPORAL_NOISELEVEL_COUNT-2; i>=0; i--)
	{
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[i+1] = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[i];
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence[i+1] = Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence[i];
	}
	if ((et_y_count*100)>(total_pixel*90))
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[0] = 4;
	else if ((et_y_count*100)>(total_pixel*85))
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[0] = 3;
	else if ((et_y_count*100)>(total_pixel*80))
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[0] = 2;
	else if ((et_y_count*100)>(total_pixel*75))
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[0] = 1;
	else
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[0] = 0;

	Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence[0] = result;

	result = 0, result2 = 0;
	for (i=0; i<TEMPORAL_NOISELEVEL_COUNT; i++)
	{
		result += Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[i];
		result2 += Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence[i];
	}
	result = result/TEMPORAL_NOISELEVEL_COUNT;
	result2 = ((result2/TEMPORAL_NOISELEVEL_COUNT)>>4);

	if (result >= 2)
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.RESULT_NOISE_RELIABLE = 1;
	else
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.RESULT_NOISE_RELIABLE = 0;

	Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.RESULT_NOISE_LEVEL = result2;

	if (Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Show_Debug_MSG)
	{
		VIP_DEBUG_PRINTF("[SIS NR_TMP_R: %d, %d, %d, %d, %d, NR_TMP_LEVEL: %d, %d, %d, %d, %d => %d\n",
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[0],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[1],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[2],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[3],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence_Reliable[4],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence[0],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence[1],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence[2],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence[3],
			Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_TEMPORAL_NOISE.Result_Sequence[4], (et_y_sum/(et_y_count+1)));
	}
}
#endif
// end chen 170522


void scalerVIP_DI_FILM_StatusUpdate(void)
{
	di_im_di_film_new_function_main_RBUS film_new_function_main_reg;
	di_im_di_si_film_final_result_RBUS film_final_result_reg;

	film_new_function_main_reg.regValue = rtd_inl(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg);
	film_final_result_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_FINAL_RESULT_reg);

	if (film_new_function_main_reg.fw_film_filmdetected == 1 || film_final_result_reg.film_detected == 1)
		Smart_Integration_Status.SCREEN_ACTION_STATUS.FILM_detect = 1;
	else
		Smart_Integration_Status.SCREEN_ACTION_STATUS.FILM_detect = 0;
}

void scalerVIP_DI_SMD_StautsUpdate(void)
{
	di_di_smd_gmvcoeff1_RBUS smd_gmvcoeff1_reg;
	smd_gmvcoeff1_reg.regValue= rtd_inl(DI_DI_SMD_GMVCoeff1_reg);

	if(Smart_Integration_Status.SCREEN_SMD_STATUS.Show_Debug_MSG == 1)
	{
		//VIP_DEBUG_PRINTF("GMV_global_ctr: %d \n",smd_gmvcoeff1_reg.smd_gmv_ctr);
	}
}

/**
 * Set auto MA threshold ??
 *
 * @param	Level: degree of auto MA level
 * @return	void
 */
void scalerVIP_colorMAQuickSetTh(unsigned char Level)
{
	di_im_di_control_RBUS di_control_reg;
	di_im_di_ma_frame_motion_th_a_RBUS motion_detect_th_a_reg;
	di_im_di_ma_frame_motion_th_b_RBUS motion_detect_th_b_reg;
	di_im_di_ma_frame_motion_th_c_RBUS motion_detect_th_c_reg;
	di_im_di_ma_field_teeth_th_a_RBUS teeth_detect_th_a_reg;
	di_im_di_ma_others_th_RBUS teeth_detect_th_b_reg;
	di_im_di_ma_sawtooth_th_RBUS sawtooth_th_reg;


 	switch (Level)
	{
		case 0: //weave only
			di_control_reg.regValue = scaler_rtd_inl(DI_IM_DI_CONTROL_reg);
			di_control_reg.ma_controlmode = 0;
			di_control_reg.write_enable_0 =1;
			scaler_rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);

			break;

		case 10: //inter only

			di_control_reg.regValue = scaler_rtd_inl(DI_IM_DI_CONTROL_reg);
			di_control_reg.ma_controlmode = 2;
			di_control_reg.write_enable_0 =1;
			scaler_rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);

			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			di_control_reg.regValue = scaler_rtd_inl(DI_IM_DI_CONTROL_reg);
			motion_detect_th_a_reg.regValue = scaler_rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
			motion_detect_th_b_reg.regValue = scaler_rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
			motion_detect_th_c_reg.regValue = scaler_rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);

			di_control_reg.ma_controlmode = 1;
			di_control_reg.write_enable_0 =1;

			/*
			if((Scaler_InputSrcGetMainChType() != _SRC_TV))
			{
				di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass = 1;
			}
			*/

			//VIP_DEBUG_PRINTF("[Young]scalerVIP_colorMAQuickSetTh88888888\n");

			/*
			motion_detect_th_a_reg.ma_framehtha = 2 + Level;
			motion_detect_th_a_reg.ma_framemtha = 3 + Level;
			motion_detect_th_a_reg.ma_framerlvtha =0x10+ Level;
			motion_detect_th_a_reg.ma_framestha =0x25+ Level;// Level;

			motion_detect_th_b_reg.ma_framehthb = 0x8 + Level;
			motion_detect_th_b_reg.ma_framemthb = 0x12 + Level;
			motion_detect_th_b_reg.ma_framerlvthb = 0x11 + Level;
			motion_detect_th_b_reg.ma_framesthb = 0x26+ Level;//0xa + Level;

			motion_detect_th_c_reg.ma_framehthc = 0x12 + Level;
			motion_detect_th_c_reg.ma_framemthc = 0x30 + Level;
			motion_detect_th_c_reg.ma_framerlvthc = 0x12 + Level;
			motion_detect_th_c_reg.ma_framesthc =0x27+ Level; //0x16 + Level;
  	  		*/

			motion_detect_th_a_reg.ma_framehtha =  (Level-1)<0?0:(Level-1);
			motion_detect_th_a_reg.ma_framemtha =   Level;
			motion_detect_th_a_reg.ma_framerlvtha = Level-3<0?0:(Level-3);
			motion_detect_th_a_reg.ma_framestha =( Level-3)<0?0:(Level-3);

			motion_detect_th_b_reg.ma_framehthb = 0x5 + Level;
			motion_detect_th_b_reg.ma_framemthb = 0x0f + Level;
			motion_detect_th_b_reg.ma_framerlvthb = 0x1 + Level;
			motion_detect_th_b_reg.ma_framesthb = 0x7 + Level;

			motion_detect_th_c_reg.ma_framehthc = 0x0f + Level;
			motion_detect_th_c_reg.ma_framemthc = 0x2d + Level;
			motion_detect_th_c_reg.ma_framerlvthc = 0x0b + Level;
			motion_detect_th_c_reg.ma_framesthc = 0x13 + Level;

			scaler_rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);
			scaler_rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, motion_detect_th_a_reg.regValue);
			scaler_rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, motion_detect_th_b_reg.regValue);
			scaler_rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, motion_detect_th_c_reg.regValue);

			break;

		default:


			di_control_reg.regValue = scaler_rtd_inl(DI_IM_DI_CONTROL_reg);
			motion_detect_th_a_reg.regValue = scaler_rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
			motion_detect_th_b_reg.regValue = scaler_rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
			motion_detect_th_c_reg.regValue = scaler_rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);
			teeth_detect_th_a_reg.regValue = scaler_rtd_inl(DI_IM_DI_MA_FIELD_TEETH_TH_A_reg);
			teeth_detect_th_b_reg.regValue = scaler_rtd_inl(DI_IM_DI_MA_OTHERS_TH_reg);
			sawtooth_th_reg.regValue = scaler_rtd_inl(DI_IM_DI_MA_SAWTOOTH_TH_reg);

			di_control_reg.ma_controlmode=1;
			di_control_reg.ma_3aenable=0;
			di_control_reg.ma_controlmode=1;
			di_control_reg.ma_messintra90=1;
			di_control_reg.ma_onedirectionweaveen=1;

			//if((Scaler_InputSrcGetMainChType() != _SRC_TV))
				//di_hmc_me_refine_ctrl2_reg.hmc_fw_ctrl_bypass= 1;

			di_control_reg.ma_smoothintra90=1;
			di_control_reg.ma_stilllogoena=1;

			di_control_reg.write_enable_0 = 1;
			di_control_reg.write_enable_1 = 1;
			di_control_reg.write_enable_2 = 1;
			di_control_reg.write_enable_3 = 1;


			motion_detect_th_a_reg.ma_framehtha=4;
			motion_detect_th_a_reg.ma_framemtha=5;
			motion_detect_th_a_reg.ma_framerlvtha=2;
			motion_detect_th_a_reg.ma_framestha=2;

			motion_detect_th_b_reg.ma_framehthb=0xa;
			motion_detect_th_b_reg.ma_framemthb=0x14;
			motion_detect_th_b_reg.ma_framerlvthb=0x6;
			motion_detect_th_b_reg.ma_framesthb=0xc;

			motion_detect_th_c_reg.ma_framehthc=0x14;
			motion_detect_th_c_reg.ma_framemthc=0x32;
			motion_detect_th_c_reg.ma_framerlvthc=0x10;
			motion_detect_th_c_reg.ma_framesthc=0x18;

			teeth_detect_th_a_reg.ma_fieldteethhtha=0xf;
			teeth_detect_th_a_reg.ma_fieldteethmtha=0xf;
			teeth_detect_th_a_reg.ma_fieldteethrlvtha=0x2;
			teeth_detect_th_a_reg.ma_fieldteethstha=0x2;

			teeth_detect_th_b_reg.ma_fieldteeththb=0x14;
			teeth_detect_th_b_reg.ma_horicomth=0x14;
			teeth_detect_th_b_reg.ma_stilllogovdth=0xf;
			teeth_detect_th_b_reg.ma_ydiffth=0x96;

			sawtooth_th_reg.ma_sawtoothhth=0x14;
			sawtooth_th_reg.ma_sawtoothmth=0x14;
			sawtooth_th_reg.ma_sawtoothrlvth=0xa;
			sawtooth_th_reg.ma_sawtoothsth=0xa;
			scaler_rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);
			scaler_rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, motion_detect_th_a_reg.regValue);
			scaler_rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, motion_detect_th_b_reg.regValue);
			scaler_rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, motion_detect_th_c_reg.regValue);
			scaler_rtd_outl(DI_IM_DI_MA_FIELD_TEETH_TH_A_reg, teeth_detect_th_a_reg.regValue);
			scaler_rtd_outl(DI_IM_DI_MA_OTHERS_TH_reg, teeth_detect_th_b_reg.regValue);
			scaler_rtd_outl(DI_IM_DI_MA_SAWTOOTH_TH_reg, sawtooth_th_reg.regValue);
			break;
	}
}


/**
 * AutoMA bean drop case
 *
 * @param	mode: display mode
 * @param	en_test: debug mode
 * @return	void
 */
void scalerVIP_colorMABeanDrop(unsigned char mode, unsigned int en_test)
{
	static unsigned char BeanCount = 0; //Thur 20070926

	unsigned char th_BeanCntMax = 90;

	unsigned int th_BeanTop = 500;
	unsigned int th_BeanMid = 500;
	unsigned int th_BeanBot = 100;
	unsigned char en_BeanTest = 0;


	di_im_di_control_RBUS di_control_reg;
	di_im_di_debug_mode_RBUS debug_mode_reg; //20070807 Thur test for debug mode
	histogram_ich1_hist_in_cselect_RBUS ich1_hist_in_cselect_reg;
	unsigned int bean_th = 0;
	di_im_di_si_film_motion_next_h_t_RBUS next_h_t;
	di_im_di_si_film_motion_next_h_m_RBUS next_h_m;
	di_im_di_si_film_motion_next_h_b_RBUS next_h_b;
	di_im_di_si_film_motion_pre_h_t_RBUS pre_h_t;
	di_im_di_si_film_motion_pre_h_m_RBUS pre_h_m;
	di_im_di_si_film_motion_pre_h_b_RBUS pre_h_b;

	//scalerDrvPrint( "enter scalerVIP_colorMABeanDrop\n" );

	en_BeanTest = (en_test<<31)>>31;

	di_control_reg.regValue = scaler_rtd_inl(DI_IM_DI_CONTROL_reg); //MA control mode
	debug_mode_reg.regValue = scaler_rtd_inl(DI_IM_DI_DEBUG_MODE_reg); //20070807 Thur Test

	switch (mode) {
		case _MODE_480I:
			break;

		case _MODE_576I:
			th_BeanTop = th_BeanTop*576/480;
			th_BeanMid = th_BeanMid*576/480;
			th_BeanBot = th_BeanBot*576/480;
			break;
		case _MODE_1080I25:
		case _MODE_1080I30:
			th_BeanTop = th_BeanTop*6;
			th_BeanMid = th_BeanMid*6;
			th_BeanBot = th_BeanBot*6;
			break;
	}

//========Bean drop problem========

	ich1_hist_in_cselect_reg.regValue = rtd_inl(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg);
	bean_th = 0x10<<(5-ich1_hist_in_cselect_reg.ch1_his_shift_sel);

	next_h_t.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg);
	next_h_m.regValue= rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_M_reg);
	next_h_b.regValue= rtd_inl(DI_IM_DI_SI_FILM_MOTION_NEXT_H_B_reg);
	pre_h_t.regValue= rtd_inl(DI_IM_DI_SI_FILM_MOTION_PRE_H_T_reg);
	pre_h_m.regValue= rtd_inl(DI_IM_DI_SI_FILM_MOTION_PRE_H_M_reg);
	pre_h_b.regValue= rtd_inl(DI_IM_DI_SI_FILM_MOTION_PRE_H_B_reg);

	if ( ((SmartPic_clue->Y_Main_Hist)[16]) > bean_th && ((SmartPic_clue->Y_Main_Hist)[17]) > bean_th &&
		(
			next_h_t.regValue>th_BeanTop &&
			next_h_m.regValue>th_BeanMid &&
			next_h_b.regValue<th_BeanBot &&
			pre_h_t.regValue>th_BeanTop &&
			pre_h_m.regValue>th_BeanMid &&
			pre_h_b.regValue<th_BeanBot ) )

	{
		BeanCount = BeanCount + 2;
		if ( BeanCount >= th_BeanCntMax ) {
			BeanCount = th_BeanCntMax;
		}

		if (1)
		{
				//di_control_reg.ma_preferintra = 1; //mac6p removed
				#if defined (VIP_TEST_MODE)
				if ( en_BeanTest == 1 ) {
					debug_mode_reg.ma_debugmode = 1; //Thur test 20070807
				}
				#endif
		}
	}
	else
	{
		if ( BeanCount > 0 )
			BeanCount--;

#if 0
		if (	rtd_t_inl(i_hist_in_data_17_16_RBUS,HISTOGRAM_I_HIST_IN_DATA_17_16_ADDR).his_hist16>0x10 &&
			rtd_t_inl(i_hist_in_data_17_16_RBUS ,HISTOGRAM_I_HIST_IN_DATA_17_16_ADDR).his_hist17>0x10 &&
#else
		if ( ((SmartPic_clue->Y_Main_Hist)[16])>bean_th && ((SmartPic_clue->Y_Main_Hist)[17])>bean_th &&
#endif


#if 0 //. can't use rtd_t_inl in kernel
			rtd_t_inl(di_im_di_si_film_motion_next_h_t_RBUS, DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg).regValue>0x500 &&
			rtd_t_inl(di_im_di_si_film_motion_next_h_m_RBUS, DI_IM_DI_SI_FILM_MOTION_NEXT_H_M_reg).regValue>0x500 &&
			rtd_t_inl(di_im_di_si_film_motion_next_h_b_RBUS, DI_IM_DI_SI_FILM_MOTION_NEXT_H_B_reg).regValue>0x500 )
#else
			next_h_t.regValue>0x500 &&
			next_h_m.regValue>0x500 &&
			next_h_b.regValue>0x500 )
#endif
		{
			BeanCount++;
		}
	}

	if ( BeanCount == 0 )
	{
		//di_control_reg.ma_preferintra = 0; //mac6p removed
		#if defined (VIP_TEST_MODE)
		if ( en_BeanTest == 1 ) {
			debug_mode_reg.ma_debugmode = 0; //Thur test 20070807
		}
		#endif
	}


	scaler_rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);

	#if defined (VIP_TEST_MODE)
         scaler_rtd_outl(DI_IM_DI_DEBUG_MODE_reg, debug_mode_reg.regValue); //Thur test 20070807
	#endif
}



/**
 * AutoMA beer problem case
 *
 * @param	en_test: debug mode
 * @return	void
 */
int scalerVIP_colorMABeerProblem(unsigned int en_test)
{
	static unsigned char flag_same = 0;
	unsigned char en_BeerTest = 0;
	unsigned char film_still = 0;

       unsigned int total_motion;
       unsigned int large_pixel;
       unsigned int large_motion;
	unsigned int film_motion_c;
	unsigned int film_motion;

	di_im_di_control_RBUS di_control_reg;
	di_im_di_debug_mode_RBUS debug_mode_reg;//20070807 Thur test for debug mode
	di_im_di_ma_frame_motion_th_a_RBUS frame_motion_a_reg;// frame motion a
	di_im_di_ma_frame_motion_th_b_RBUS frame_motion_b_reg;// frame motion b
	di_im_di_ma_frame_motion_th_c_RBUS frame_motion_c_reg;// frame motion c

	di_im_di_si_ma_total_fm_sum_RBUS	ma_total_fm_sum;
	di_im_di_si_ma_large_fm_pixel_RBUS	ma_large_fm_pixel;
	di_im_di_si_ma_large_fm_sum_RBUS	ma_large_fm_sum;
	di_im_di_si_film_motion_c_RBUS		di_si_film_motion_c;
	di_im_di_si_film_motion_RBUS		di_si_film_motion;

	frame_motion_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	frame_motion_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
	frame_motion_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);

	//di_im_di_rtnr_k_RBUS RecursiveTemporalK_RBUS_reg;//20090325 jason9.ccc// mark for keep rtnr k free March 20100716
//	di_im_di_rtnr_control_RBUS di_im_di_rtnr_control_RBUS_reg;

     //  RecursiveTemporalK_RBUS_reg.regValue = scaler_rtd_inl(DI_IM_DI_RTNR_K_reg);// mark for keep rtnr k free March 20100716
//	di_im_di_rtnr_control_RBUS_reg.regValue = rtd_inl(DI_IM_DI_RTNR_CONTROL_reg);

	//scalerDrvPrint( "enter scalerVIP_colorMABeerProblem\n" );

	di_control_reg.regValue = scaler_rtd_inl(DI_IM_DI_CONTROL_reg);//MA control mode
	debug_mode_reg.regValue = scaler_rtd_inl(DI_IM_DI_DEBUG_MODE_reg);//20070807 Thur Test

	en_BeerTest = (en_test<<30)>>31;

	ma_total_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_MA_TOTAL_FM_SUM_reg);
	ma_large_fm_pixel.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_PIXEL_reg);
	ma_large_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_MA_LARGE_FM_SUM_reg);
	di_si_film_motion_c.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_C_reg);
	di_si_film_motion.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);

	total_motion = ma_total_fm_sum.total_frame_motion_sum;
	large_pixel = ma_large_fm_pixel.large_frame_motion_pixel;
	large_motion = ma_large_fm_sum.large_frame_motion_sum;
	film_motion_c = di_si_film_motion_c.film_motionstatus_259_240;
	film_motion = di_si_film_motion.film_motionstatus_399_380;

	//scalerDrvPrint("============== BEER : %d (tm: %d, lp : %d, lm : %d, fmc : %d)=============\n", film_still, total_motion, large_pixel, large_motion, film_motion_c);
	if (total_motion<800 && large_pixel<70 && large_motion<150 && film_motion_c<5)
	{
		flag_same = 1;

		//frame_motion_a_reg.regValue = 0x18191616; //0x04050202;
		//frame_motion_b_reg.regValue = 0x281E1A20; //0x140a060c;
		//frame_motion_c_reg.regValue = 0x4E28242C; //0x3a141018;
		//rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, frame_motion_a_reg.regValue);
		//rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, frame_motion_b_reg.regValue);
		//rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, frame_motion_c_reg.regValue);

		//---enable crct2----
		/*
		#ifdef CONFIG_RTNR_Use_Correction_2
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 0;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en= 1;
		#else // add this condition to force original correction 2 into correction 1, 2010015 Jerry Wu
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 1;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en= 0;
		#endif
		*/
//		rtd_outl(DI_IM_DI_RTNR_CONTROL_reg, di_im_di_rtnr_control_RBUS_reg.regValue);//rtnr

		#if defined (VIP_TEST_MODE)
		if ( en_BeerTest == 1 )
		{
			debug_mode_reg.ma_debugmode = 1; //Thur test 20070807
		}
		#endif

		BeerCount++;
	}
	else if (film_motion<20 || (film_motion_c < 5 && total_motion < 25000))
	{
		flag_same = 1;
		film_still = 1;
		BeerCount++;
	}
	else
	{
		if ( flag_same == 1 )
		{
			flag_same = 0;

			scalerVIP_colorMAQuickSetTh(5);

			di_control_reg.ma_controlmode = 1;
			di_control_reg.write_enable_0 = 1;

			#if defined (VIP_TEST_MODE)
			if ( en_BeerTest == 1 )
			{
				debug_mode_reg.ma_debugmode = 0; //Thur test 20070807
			}
			#endif
			/*
			//---enable crct1----
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction = 1;
			di_im_di_rtnr_control_RBUS_reg.cp_rtnr_rounding_correction2_en= 0;
			*/
//			rtd_outl(DI_IM_DI_RTNR_CONTROL_reg, di_im_di_rtnr_control_RBUS_reg.regValue);//rtnr

		}

		if (BeerCount>0)
			BeerCount--;
		if (BeerFlag)
		{
			if (BeerCount > 20)
				BeerCount -=20;
			else
			BeerCount = 0;
		}
	}

	if (BeerCount > 50)
	{
		if (BeerFlag == 0)
		{
			CaveCount =0;
		}
		BeerCount  = 50;
		BeerFlag = 1;
	}
	if (BeerCount == 0)
	{
		if (BeerFlag == 1)
		{
			ZoomInCount = 0;
			PanInCount = 0;
		}
		BeerFlag = 0;
	}

	if (BeerFlag)
	{
		frame_motion_a_reg.regValue = 0x18190216;//0x18191616; //0x04050202;
		frame_motion_b_reg.regValue = 0x281E1A0C;//0x281E1A20; //0x140a060c;
		frame_motion_c_reg.regValue = 0x3E282418;//0x3E28242C; //0x3a141018;


		if (film_still)
		{
			frame_motion_a_reg.regValue = 0x38393636; //0x04050202;
			frame_motion_b_reg.regValue = 0x483E3A40; //0x140a060c;
			frame_motion_c_reg.regValue = 0x5E48444C; //0x3a141018;

		}

		rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, frame_motion_a_reg.regValue);
		rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, frame_motion_b_reg.regValue);
		rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, frame_motion_c_reg.regValue);

		//drvif_color_ma_TeethCheck(0);
/*		// mark for keep rtnr k free March 20100716
		if( SCALER_GET_RTNR_MODE(_CHANNEL1) )
		{
			RecursiveTemporalK_RBUS_reg.cp_temporal_y_k0=0;
			RecursiveTemporalK_RBUS_reg.cp_temporal_c_k0=0;
			scaler_rtd_outl(DI_IM_DI_RTNR_K_reg, RecursiveTemporalK_RBUS_reg.regValue);
		}
		*/
	}
	else
	{
		scalerVIP_colorMAQuickSetTh(5);

		di_control_reg.ma_controlmode = 1;
		di_control_reg.write_enable_0 = 1;

		//drvif_color_ma_TeethCheck(3);
/* 		// mark for keep rtnr k free March 20100716
		if( SCALER_GET_RTNR_MODE(_CHANNEL1) )
		{
			RecursiveTemporalK_RBUS_reg.cp_temporal_y_k0=1;
			RecursiveTemporalK_RBUS_reg.cp_temporal_c_k0=1;
			scaler_rtd_outl(DI_IM_DI_RTNR_K_reg, RecursiveTemporalK_RBUS_reg.regValue);
		}
		*/
	}

	scaler_rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);

	#if defined (VIP_TEST_MODE)
		scaler_rtd_outl(DI_IM_DI_DEBUG_MODE_reg, debug_mode_reg.regValue); //Thur test 20070807
	#endif

	return BeerFlag;
}


int drvif_color_Zoom(void)
{
	unsigned int cur_total_motion;
	unsigned int film_motion_c, film_motion_top, film_motion_bot, film_motion_left, film_motion_right;

	static unsigned char counter = 0;
	static unsigned char ZoomInFlag = 0;

	di_im_di_control_RBUS di_control_reg;
	di_im_di_ma_frame_motion_th_a_RBUS frame_motion_a_reg;// frame motion a
	di_im_di_ma_frame_motion_th_b_RBUS frame_motion_b_reg;// frame motion b
	di_im_di_ma_frame_motion_th_c_RBUS frame_motion_c_reg;// frame motion c

	di_im_di_si_ma_total_fm_sum_RBUS	ma_total_fm_sum;
	di_im_di_si_film_motion_c_RBUS		di_si_film_motion_c;
	di_im_di_si_film_motion_h_t_RBUS		di_si_film_motion_h_t;
	di_im_di_si_film_motion_h_b_RBUS	di_si_film_motion_h_b;
	di_im_di_si_film_motion_v_l_RBUS		di_si_film_motion_v_l;
	di_im_di_si_film_motion_v_r_RBUS		di_si_film_motion_v_r;

	frame_motion_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	frame_motion_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
	frame_motion_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);

	ma_total_fm_sum.regValue = rtd_inl(DI_IM_DI_SI_MA_TOTAL_FM_SUM_reg);
	di_si_film_motion_c.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_C_reg);
	di_si_film_motion_h_t.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_T_reg);
	di_si_film_motion_h_b.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_B_reg);
	di_si_film_motion_v_l.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_L_reg);
	di_si_film_motion_v_r.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_R_reg);

       cur_total_motion = ma_total_fm_sum.total_frame_motion_sum;
	film_motion_c = di_si_film_motion_c.film_motionstatus_259_240;
	film_motion_top = di_si_film_motion_h_t.film_motionstatus_379_360;
	film_motion_bot = di_si_film_motion_h_b.film_motionstatus_339_320;
	film_motion_left = di_si_film_motion_v_l.film_motionstatus_319_300;
	film_motion_right = di_si_film_motion_v_r.film_motionstatus_279_260;


	di_control_reg.regValue = rtd_inl(DI_IM_DI_CONTROL_reg);//MA control mode

	if (counter < 10)
	{
		counter++;
		cur_total_motion_sum += cur_total_motion;
	}
	else
	{
		counter = 0;

		if (cur_total_motion_sum > pre_total_motion_sum)
			ZoomInCount++;
		else
		{
			if (ZoomInCount>2)
				ZoomInCount-=2;
			else
				ZoomInCount=0;
		}

		pre_total_motion_sum = cur_total_motion_sum;
		cur_total_motion_sum = 0;
	}

	if (ZoomInCount > 15)
	{
		ZoomInCount = 15;
        	if (ZoomInFlag == 0)
			VIP_DEBUG_PRINTF(" ====== INTO Zoom Statement =======\n");
		ZoomInFlag = 1;
	}
	if (ZoomInCount == 0)
	{
        	if (ZoomInFlag == 1)
			VIP_DEBUG_PRINTF(" ====== LEAVE Zoom Statement =======\n");
		ZoomInFlag = 0;
	}
	//if((MA_print_count % 30) == 0)
	//VIP_DEBUG_PRINTF("[Zoom]=== %d, %d : %d : %d : %d : %d : %d ===\n", cur_total_motion_sum, pre_total_motion_sum, ZoomInFlag, film_motion_c, film_motion_top, film_motion_bot, film_motion_left, film_motion_right);

	if (ZoomInFlag && film_motion_c < 800)// && film_motion_top > 4 && film_motion_bot > 4 && film_motion_left > 4 && film_motion_right > 4)
	{
		di_control_reg.ma_onedirectionweaveen = 0;

		rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);

		frame_motion_a_reg.regValue = 0x38393636; //0x04050202;
		frame_motion_b_reg.regValue = 0x483E3A40; //0x140a060c;
		frame_motion_c_reg.regValue = 0x5E48444C; //0x3a141018;

		rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, frame_motion_a_reg.regValue);
		rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, frame_motion_b_reg.regValue);
		rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, frame_motion_c_reg.regValue);
	}
	else
	{
		di_control_reg.ma_onedirectionweaveen = 1;

		rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);

		scalerVIP_colorMAQuickSetTh(5);
	}

	return ZoomInFlag;
}

void scaleVIP_color_tnrxc(unsigned char display, unsigned char Level)
{
	//Level = 0~8
	di_im_di_tnr_xc_control_RBUS TNR_XC_reg;
	di_im_di_tnr_th_RBUS Temporal_Thl_reg;
	di_im_di_control_RBUS di_control_reg;

	TNR_XC_reg.regValue = scaler_rtd_inl(DI_IM_DI_TNR_XC_CONTROL_reg);
	Temporal_Thl_reg.regValue = scaler_rtd_inl(DI_IM_DI_TNR_TH_reg);

	if(display == 0) //main
	{
		if (Level==0)
		{
			TNR_XC_reg.cp_temporal_xc_en = 0;
		}
		else if (Level>=8)
		{
			TNR_XC_reg.cp_temporal_xc_en = 1;
			Temporal_Thl_reg.cp_temporalthly = 0xff;
		}
		else
		{
			TNR_XC_reg.cp_temporal_xc_en = 1;
			Temporal_Thl_reg.cp_temporalthly = 1<<Level;
		}
	}

	//20150805 roger, temporalenable must follow cp_temporal_xc_en
	di_control_reg.regValue = scaler_rtd_inl(DI_IM_DI_CONTROL_reg);
	di_control_reg.cp_temporalenable = TNR_XC_reg.cp_temporal_xc_en;
	di_control_reg.write_enable_6= 1;
	scaler_rtd_outl(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);

	scaler_rtd_outl(DI_IM_DI_TNR_XC_CONTROL_reg, TNR_XC_reg.regValue);
	scaler_rtd_outl(DI_IM_DI_TNR_TH_reg, Temporal_Thl_reg.regValue);

}

#if 0
unsigned char scalerVIP_Pan_Detector(void)
{
	unsigned int temp = 0, MaxValue_Pan= 0, SecondMaxValue_Pan = 0, Sum_Pan_Count =0, Pan_Count_Ratio = 0;
	unsigned char MAX_Pan_index = 0, SecondMax_Pan_index = 0, i=0;
	static unsigned char Pan_ratio_counter = 0;
	unsigned char Pan_flag=0;

	//condiction defined
	static unsigned int Pan_ratio_th = 70;
	unsigned char Pan_ratio_counter_th = 5,Pan_counter_th_upbound = 10 ;

//===========================================================
//========================Motion Pan Check =======================
//===========================================================
	di_im_di_pan_detection_statistic_1_RBUS im_di_pan_detection_statistic_1_reg;
	di_im_di_pan_detection_statistic_2_RBUS im_di_pan_detection_statistic_2_reg;
	di_im_di_pan_detection_statistic_3_RBUS im_di_pan_detection_statistic_3_reg;
	di_im_di_pan_detection_statistic_4_RBUS im_di_pan_detection_statistic_4_reg;
	di_im_di_pan_detection_statistic_5_RBUS im_di_pan_detection_statistic_5_reg;
	di_im_di_pan_detection_statistic_6_RBUS im_di_pan_detection_statistic_6_reg;
	di_im_di_pan_detection_statistic_7_RBUS im_di_pan_detection_statistic_7_reg;
	di_im_di_pan_detection_statistic_8_RBUS im_di_pan_detection_statistic_8_reg;
	di_im_di_pan_detection_statistic_9_RBUS im_di_pan_detection_statistic_9_reg;
	di_im_di_pan_detection_statistic_10_RBUS im_di_pan_detection_statistic_10_reg;
	di_im_di_pan_detection_statistic_11_RBUS im_di_pan_detection_statistic_11_reg;
	di_im_di_pan_detection_statistic_12_RBUS im_di_pan_detection_statistic_12_reg;
	di_im_di_pan_detection_statistic_13_RBUS im_di_pan_detection_statistic_13_reg;
	di_im_di_pan_detection_statistic_14_RBUS im_di_pan_detection_statistic_14_reg;
	di_im_di_pan_detection_statistic_15_RBUS im_di_pan_detection_statistic_15_reg;
	di_im_di_pan_detection_statistic_16_RBUS im_di_pan_detection_statistic_16_reg;
	di_im_di_pan_detection_statistic_17_RBUS im_di_pan_detection_statistic_17_reg;

	im_di_pan_detection_statistic_1_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_1_reg);
	im_di_pan_detection_statistic_2_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_2_reg);
	im_di_pan_detection_statistic_3_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_3_reg);
	im_di_pan_detection_statistic_4_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_4_reg);
	im_di_pan_detection_statistic_5_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_5_reg);
	im_di_pan_detection_statistic_6_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_6_reg);
	im_di_pan_detection_statistic_7_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_7_reg);
	im_di_pan_detection_statistic_8_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_8_reg);
	im_di_pan_detection_statistic_9_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_9_reg);
	im_di_pan_detection_statistic_10_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_10_reg);
	im_di_pan_detection_statistic_11_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_11_reg);
	im_di_pan_detection_statistic_12_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_12_reg);
	im_di_pan_detection_statistic_13_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_13_reg);
	im_di_pan_detection_statistic_14_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_14_reg);
	im_di_pan_detection_statistic_15_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_15_reg);
	im_di_pan_detection_statistic_16_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_16_reg);
	im_di_pan_detection_statistic_17_reg.regValue = rtd_inl(DI_IM_DI_PAN_DETECTION_STATISTIC_17_reg);

	for(i=0;i<33;i++)//read hor 8 counters
	{
		switch(i)
		{
			case 0:
				temp = im_di_pan_detection_statistic_9_reg.hmc_r16_pixelsum;
				break;
			case 1:
				temp = im_di_pan_detection_statistic_9_reg.hmc_r15_pixelsum;
				break;
			case 2:
				temp = im_di_pan_detection_statistic_10_reg.hmc_r14_pixelsum;
				break;
			case 3:
				temp = im_di_pan_detection_statistic_10_reg.hmc_r13_pixelsum;
				break;
			case 4:
				temp = im_di_pan_detection_statistic_11_reg.hmc_r12_pixelsum;
				break;
			case 5:
				temp = im_di_pan_detection_statistic_11_reg.hmc_r11_pixelsum;
				break;
			case 6:
				temp = im_di_pan_detection_statistic_12_reg.hmc_r10_pixelsum;
				break;
			case 7:
				temp = im_di_pan_detection_statistic_12_reg.hmc_r9_pixelsum;
				break;
			case 8:
				temp = im_di_pan_detection_statistic_13_reg.hmc_r8_pixelsum;
				break;
	    		case 9:
				temp = im_di_pan_detection_statistic_1_reg.hmc_r7_pixelsum;
				break;
			case 10:
				temp = im_di_pan_detection_statistic_1_reg.hmc_r6_pixelsum;
				break;
			case 11:
				temp = im_di_pan_detection_statistic_2_reg.hmc_r5_pixelsum;
				break;
			case 12:
				temp = im_di_pan_detection_statistic_2_reg.hmc_r4_pixelsum;
				break;
			case 13:
				temp = im_di_pan_detection_statistic_3_reg.hmc_r3_pixelsum;
				break;
			case 14:
				temp = im_di_pan_detection_statistic_3_reg.hmc_r2_pixelsum;
				break;
			case 15:
				temp = im_di_pan_detection_statistic_4_reg.hmc_r1_pixelsum;
				break;
			case 16:
				temp = im_di_pan_detection_statistic_4_reg.hmc_c0_pixelsum;
				break;
			case 17:
				temp = im_di_pan_detection_statistic_5_reg.hmc_l1_pixelsum;
				break;
			case 18:
				temp = im_di_pan_detection_statistic_5_reg.hmc_l2_pixelsum;
				break;
			case 19:
				temp = im_di_pan_detection_statistic_6_reg.hmc_l3_pixelsum;
				break;
			case 20:
				temp = im_di_pan_detection_statistic_6_reg.hmc_l4_pixelsum;
				break;
			case 21:
				temp = im_di_pan_detection_statistic_7_reg.hmc_l5_pixelsum;
				break;
			case 22:
				temp = im_di_pan_detection_statistic_7_reg.hmc_l6_pixelsum;
				break;
			case 23:
				temp = im_di_pan_detection_statistic_8_reg.hmc_l7_pixelsum;
				break;
			case 24:
				temp = im_di_pan_detection_statistic_13_reg.hmc_l8_pixelsum;
				break;
			case 25:
				temp = im_di_pan_detection_statistic_14_reg.hmc_l9_pixelsum;
				break;
			case 26:
				temp = im_di_pan_detection_statistic_14_reg.hmc_l10_pixelsum;
				break;
			case 27:
				temp = im_di_pan_detection_statistic_15_reg.hmc_l11_pixelsum;
				break;
			case 28:
				temp = im_di_pan_detection_statistic_15_reg.hmc_l12_pixelsum;
				break;
			case 29:
				temp = im_di_pan_detection_statistic_16_reg.hmc_l13_pixelsum;
				break;
			case 30:
				temp = im_di_pan_detection_statistic_16_reg.hmc_l14_pixelsum;
				break;
			case 31:
				temp = im_di_pan_detection_statistic_17_reg.hmc_l15_pixelsum;
				break;
			case 32:
				temp = im_di_pan_detection_statistic_17_reg.hmc_l16_pixelsum;
				break;
    		}

		Sum_Pan_Count =Sum_Pan_Count + temp;

		if(MaxValue_Pan <= temp)
		{
			SecondMaxValue_Pan = MaxValue_Pan;
			MaxValue_Pan = temp;
			MAX_Pan_index = i;
		}
		else if(SecondMaxValue_Pan <= temp)
		{
			SecondMaxValue_Pan = temp;
			SecondMax_Pan_index = i;
		}
	}

	if(Sum_Pan_Count!=0)
		Pan_Count_Ratio = (MaxValue_Pan*100)/Sum_Pan_Count ;

	if(MAX_Pan_index!=16)
	{
		if(Pan_Count_Ratio>=Pan_ratio_th)
		{
			Pan_ratio_counter = (Pan_ratio_counter>=Pan_counter_th_upbound)?Pan_counter_th_upbound:(Pan_ratio_counter+1);
		}
		else
		{
			Pan_ratio_counter = (Pan_ratio_counter<=0)?0:(Pan_ratio_counter-1);
		}
	}
	else
	{
		Pan_ratio_counter = (Pan_ratio_counter<=0)?0:(Pan_ratio_counter-1);
	}

	if(Pan_ratio_counter > Pan_ratio_counter_th)
	{
		Pan_flag = 1;
		Pan_ratio_th = 40;
	}
	else
	{
		Pan_flag = 0;
		Pan_ratio_th =70;
	}
	return Pan_flag;
}
#endif

void scalerVIP_DI_Dynamic_VDNoise_FrameMotion(void)
{
/*
	This function is consider VD Noise to dynamic adjust DI frame motion
	VD noise table
	noise_gen		noise value	frame motion
	0_0				0x32
	0_2				0x29
	0_4				0x20
	0_6				0x1a
	0_8				0x15
	0_10/10_0		0x10		+0x10
	10_2			0x0c				+0xc
	10_4			0x0a			+0x8
	10_6			0x07		+0x5
	10_8			0x06			+0x2
	10_10/20_0		0x04		default 0x04050202
	20_2			0x03
	20_4			0x03
	20_6			0x02
	20_8			0x01
	20_10/30_0		0x01

*/

	unsigned int FM_bias;
	vdtop_noise_status_RBUS noise_status_reg;

	di_im_di_ma_frame_motion_th_a_RBUS frame_motion_a_reg;// frame motion a
	di_im_di_ma_frame_motion_th_b_RBUS frame_motion_b_reg;// frame motion b
	di_im_di_ma_frame_motion_th_c_RBUS frame_motion_c_reg;// frame motion c

	frame_motion_a_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg);
	frame_motion_b_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg);
	frame_motion_c_reg.regValue = rtd_inl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg);

	noise_status_reg.regValue = VDPQ_rtd_inl(VDTOP_NOISE_STATUS_reg);

	if (noise_status_reg.status_noise > 0x04 && noise_status_reg.status_noise <= 0x06)
		FM_bias = 0x02020202;
	else if (noise_status_reg.status_noise > 0x06 && noise_status_reg.status_noise <= 0x07)
		FM_bias = 0x05050505;
	else if (noise_status_reg.status_noise > 0x07 && noise_status_reg.status_noise <= 0x0a)
		FM_bias = 0x08080808;
	else if (noise_status_reg.status_noise > 0x0a && noise_status_reg.status_noise <= 0x0c)
		FM_bias = 0x0c0c0c0c;
	else if (noise_status_reg.status_noise > 0x0c /*&& noise_status_reg.status_noise <= 0x10*/)
		FM_bias = 0x10101010;
	else
		FM_bias = 0x0;

//	VIP_DEBUG_PRINTF("======================= FM_bias = %d\n", FM_bias);
	frame_motion_a_reg.regValue = 0x04050202+FM_bias; //0x04050202;
	frame_motion_b_reg.regValue = 0x140a060c+FM_bias; //0x140a060c;
	frame_motion_c_reg.regValue = 0x3a141018+FM_bias; //0x3a141018;

	rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_A_reg, frame_motion_a_reg.regValue);
	rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_B_reg, frame_motion_b_reg.regValue);
	rtd_outl(DI_IM_DI_MA_FRAME_MOTION_TH_C_reg, frame_motion_c_reg.regValue);
}

void scalerVIP_DI_Dynamic_Status_Adjust_by_Global_Motion(MOTION_LEVEL motion_level_flag)
{
	di_im_di_btr_control_RBUS BTR_Control_Reg;

	BTR_Control_Reg.regValue = rtd_inl(DI_IM_DI_BTR_CONTROL_reg);
	if (btr_control == 2) // not assign
	{
		btr_control = BTR_Control_Reg.btr_blendtemporalrecoveryen;
	}

	rtd_clearbits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT3);
	rtd_clearbits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT4);
	rtd_clearbits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT5);
	rtd_clearbits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT6);

	switch (motion_level_flag)
	{
		case MOTION_LEVEL_VERY_STILL:
			rtd_setbits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT3);
			BTR_Control_Reg.btr_blendtemporalrecoveryen = 0;
			break;
		case MOTION_LEVEL_STILL:
			rtd_setbits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT4);
			BTR_Control_Reg.btr_blendtemporalrecoveryen = 0;
			break;
		case MOTION_LEVEL_SLOW_MOTION:
			rtd_setbits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT5);
			if (btr_control)
			{
				BTR_Control_Reg.btr_blendtemporalrecoveryen = 1;
			}
			break;
		case MOTION_LEVEL_MOTION:
			rtd_setbits(DI_IM_DI_MPEG_SEQUENCE_APP_reg, _BIT6);
			if (btr_control)
			{
				BTR_Control_Reg.btr_blendtemporalrecoveryen = 1;
			}
			break;
		default:
			break;
	}


	rtd_outl(DI_IM_DI_BTR_CONTROL_reg, BTR_Control_Reg.regValue);
}

//////////////////// FM Film ///////////////////////////////
void scalerVIP_film_FWmode_film_reset(void)
{
	static unsigned char film32_debounce_FieldNum = 0xf;
	static unsigned char film22_debounce_FieldNum = 0xf;

	di_im_di_si_film_final_result_RBUS im_di_si_film_final_result_reg;
	di_im_di_si_film_motion_RBUS im_di_si_film_motion_reg;
	di_im_di_si_film_motion_h_t_RBUS im_di_si_film_motion_h_t_reg;
	di_im_di_si_film_motion_h_m_RBUS im_di_si_film_motion_h_m_reg;
	di_im_di_si_film_motion_h_b_RBUS im_di_si_film_motion_h_b_reg;
	di_im_di_si_film_motion_v_l_RBUS im_di_si_film_motion_v_l_reg;
	di_im_di_si_film_motion_v_m_RBUS im_di_si_film_motion_v_m_reg;
	di_im_di_si_film_motion_v_r_RBUS im_di_si_film_motion_v_r_reg;
	di_im_di_si_film_motion_c_RBUS im_di_si_film_motion_c_reg;
	di_im_di_film_motion_sum_th_RBUS im_di_film_motion_sum_th_reg;
	di_im_di_film_new_function_main_RBUS di_im_di_film_new_function_main_reg;

	im_di_si_film_final_result_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_FINAL_RESULT_reg);
	im_di_si_film_motion_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_reg);
	im_di_si_film_motion_h_t_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_T_reg);
	im_di_si_film_motion_h_m_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_M_reg);
	im_di_si_film_motion_h_b_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_H_B_reg);
	im_di_si_film_motion_v_l_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_L_reg);
	im_di_si_film_motion_v_m_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_M_reg);
	im_di_si_film_motion_v_r_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_V_R_reg);
	im_di_si_film_motion_c_reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_MOTION_C_reg);
	im_di_film_motion_sum_th_reg.regValue = rtd_inl(DI_IM_DI_FILM_MOTION_SUM_TH_reg);
	di_im_di_film_new_function_main_reg.regValue = rtd_inl(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg);

	if(	im_di_si_film_motion_reg.film_motionstatus_399_380 == 0 &&
		im_di_si_film_motion_h_t_reg.film_motionstatus_379_360 == 0 &&
		im_di_si_film_motion_h_m_reg.film_motionstatus_359_340 == 0 &&
		im_di_si_film_motion_h_b_reg.film_motionstatus_339_320 == 0 &&
		im_di_si_film_motion_v_l_reg.film_motionstatus_319_300 == 0 &&
		im_di_si_film_motion_v_m_reg.film_motionstatus_299_280 == 0 &&
		im_di_si_film_motion_v_r_reg.film_motionstatus_279_260 == 0 &&
		im_di_si_film_motion_c_reg.film_motionstatus_259_240==0 &&
		im_di_si_film_final_result_reg.film_detected == 1 &&
		(di_im_di_film_new_function_main_reg.fw_film_en == 0 && di_im_di_film_new_function_main_reg.fw_film_filmdetected == 1)
	) {

		film32_debounce_FieldNum = im_di_film_motion_sum_th_reg.film32_debounce_fieldnum;
		film22_debounce_FieldNum = im_di_film_motion_sum_th_reg.film22_debounce_fieldnum;

		im_di_film_motion_sum_th_reg.film32_debounce_fieldnum = 0xf;
		im_di_film_motion_sum_th_reg.film22_debounce_fieldnum = 0xf;

	} else if(film32_debounce_FieldNum!=0xf && film22_debounce_FieldNum!=0xf) {
		im_di_film_motion_sum_th_reg.film32_debounce_fieldnum = film32_debounce_FieldNum;
		im_di_film_motion_sum_th_reg.film22_debounce_fieldnum = film22_debounce_FieldNum;
	}

	scaler_rtd_outl(DI_IM_DI_FILM_MOTION_SUM_TH_reg, im_di_film_motion_sum_th_reg.regValue);
}

#if 0
void scalerVIP_film_FWmode_3D_start()
{
	di_im_di_debug_mode_RBUS reg18024008_Reg;
	di_im_di_si_film_hb_cout_RBUS reg180240c0_Reg;
	reg18024008_Reg.regValue = rtd_inl(DI_IM_DI_DEBUG_MODE_reg);
	reg180240c0_Reg.regValue = rtd_inl(DI_IM_DI_SI_FILM_HB_COUT_reg);
	unsigned char hw_3D_l_flag = reg18024008_Reg.di_3d_l_flag;
	unsigned char hw_3D_mode = reg180240c0_Reg.film_3d_mode;

  	if( (hw_3D_l_flag==0 /*|| reg_vc_3D_Enable==0*/ || hw_3D_mode==2) && (hw_3D_mode!=1) )
	 {
		//Right=======================================
		StatusFrameStaticPattern    = R_film_StatusFrameStaticPattern   ;
		StatusFrameStaticPattern_H1 = R_film_StatusFrameStaticPattern_H1;
		StatusFrameStaticPattern_H2 = R_film_StatusFrameStaticPattern_H2;
		StatusFrameStaticPattern_H3 = R_film_StatusFrameStaticPattern_H3;
		StatusFrameStaticPattern_V1 = R_film_StatusFrameStaticPattern_V1;
		StatusFrameStaticPattern_V2 = R_film_StatusFrameStaticPattern_V2;
		StatusFrameStaticPattern_V3 = R_film_StatusFrameStaticPattern_V3;
		StatusFrameStaticPattern_C  = R_film_StatusFrameStaticPattern_C ;

		StatusFramePairPattern    = R_film_StatusFramePairPattern   ;
		StatusFramePairPattern_H1 = R_film_StatusFramePairPattern_H1;
		StatusFramePairPattern_H2 = R_film_StatusFramePairPattern_H2;
		StatusFramePairPattern_H3 = R_film_StatusFramePairPattern_H3;
		StatusFramePairPattern_V1 = R_film_StatusFramePairPattern_V1;
		StatusFramePairPattern_V2 = R_film_StatusFramePairPattern_V2;
		StatusFramePairPattern_V3 = R_film_StatusFramePairPattern_V3;
		StatusFramePairPattern_C  = R_film_StatusFramePairPattern_C ;

		StatusPrevStaticPattern2    = R_film_StatusPrevStaticPattern2   ;
		StatusPrevStaticPattern2_H1 = R_film_StatusPrevStaticPattern2_H1;
		StatusPrevStaticPattern2_H2 = R_film_StatusPrevStaticPattern2_H2;
		StatusPrevStaticPattern2_H3 = R_film_StatusPrevStaticPattern2_H3;
		StatusPrevStaticPattern2_V1 = R_film_StatusPrevStaticPattern2_V1;
		StatusPrevStaticPattern2_V2 = R_film_StatusPrevStaticPattern2_V2;
		StatusPrevStaticPattern2_V3 = R_film_StatusPrevStaticPattern2_V3;
		StatusPrevStaticPattern2_C  = R_film_StatusPrevStaticPattern2_C ;

		StatusNextStaticPattern    = R_film_StatusNextStaticPattern   ;
		StatusNextStaticPattern_H1 = R_film_StatusNextStaticPattern_H1;
		StatusNextStaticPattern_H2 = R_film_StatusNextStaticPattern_H2;
		StatusNextStaticPattern_H3 = R_film_StatusNextStaticPattern_H3;
		StatusNextStaticPattern_V1 = R_film_StatusNextStaticPattern_V1;
		StatusNextStaticPattern_V2 = R_film_StatusNextStaticPattern_V2;
		StatusNextStaticPattern_V3 = R_film_StatusNextStaticPattern_V3;
		StatusNextStaticPattern_C  = R_film_StatusNextStaticPattern_C ;

		StatusNextStaticPattern2    = R_film_StatusNextStaticPattern2   ;
		StatusNextStaticPattern2_H1 = R_film_StatusNextStaticPattern2_H1;
		StatusNextStaticPattern2_H2 = R_film_StatusNextStaticPattern2_H2;
		StatusNextStaticPattern2_H3 = R_film_StatusNextStaticPattern2_H3;
		StatusNextStaticPattern2_V1 = R_film_StatusNextStaticPattern2_V1;
		StatusNextStaticPattern2_V2 = R_film_StatusNextStaticPattern2_V2;
		StatusNextStaticPattern2_V3 = R_film_StatusNextStaticPattern2_V3;
		StatusNextStaticPattern2_C  = R_film_StatusNextStaticPattern2_C ;

		Status32Detected1    = R_film_Status32Detected1   ;
		Status32Detected2    = R_film_Status32Detected2   ;
		Status32Detected1_H1 = R_film_Status32Detected1_H1;
		Status32Detected1_H2 = R_film_Status32Detected1_H2;
		Status32Detected1_H3 = R_film_Status32Detected1_H3;
		Status32Detected1_V1 = R_film_Status32Detected1_V1;
		Status32Detected1_V2 = R_film_Status32Detected1_V2;
		Status32Detected1_V3 = R_film_Status32Detected1_V3;
		Status32Detected1_C  = R_film_Status32Detected1_C ;
		Status32Detected2_H1 = R_film_Status32Detected2_H1;
		Status32Detected2_H2 = R_film_Status32Detected2_H2;
		Status32Detected2_H3 = R_film_Status32Detected2_H3;
		Status32Detected2_V1 = R_film_Status32Detected2_V1;
		Status32Detected2_V2 = R_film_Status32Detected2_V2;
		Status32Detected2_V3 = R_film_Status32Detected2_V3;
		Status32Detected2_C  = R_film_Status32Detected2_C ;
		Status22Detected     = R_film_Status22Detected    ;
		Status22Detected_H1  = R_film_Status22Detected_H1 ;
		Status22Detected_H2  = R_film_Status22Detected_H2 ;
		Status22Detected_H3  = R_film_Status22Detected_H3 ;
		Status22Detected_V1  = R_film_Status22Detected_V1 ;
		Status22Detected_V2  = R_film_Status22Detected_V2 ;
		Status22Detected_V3  = R_film_Status22Detected_V3 ;
		Status22Detected_C   = R_film_Status22Detected_C  ;

	  	Status32Sequence1    = R_film_Status32Sequence1   ;
	  	Status32Sequence2    = R_film_Status32Sequence2   ;
	  	Status32Sequence1_H1 = R_film_Status32Sequence1_H1;
	  	Status32Sequence1_H2 = R_film_Status32Sequence1_H2;
	  	Status32Sequence1_H3 = R_film_Status32Sequence1_H3;
	  	Status32Sequence1_V1 = R_film_Status32Sequence1_V1;
	  	Status32Sequence1_V2 = R_film_Status32Sequence1_V2;
	  	Status32Sequence1_V3 = R_film_Status32Sequence1_V3;
	  	Status32Sequence1_C  = R_film_Status32Sequence1_C ;
	  	Status32Sequence2_H1 = R_film_Status32Sequence2_H1;
	  	Status32Sequence2_H2 = R_film_Status32Sequence2_H2;
	  	Status32Sequence2_H3 = R_film_Status32Sequence2_H3;
	  	Status32Sequence2_V1 = R_film_Status32Sequence2_V1;
	  	Status32Sequence2_V2 = R_film_Status32Sequence2_V2;
	  	Status32Sequence2_V3 = R_film_Status32Sequence2_V3;
	  	Status32Sequence2_C  = R_film_Status32Sequence2_C ;
	  	Status22Sequence     = R_film_Status22Sequence    ;
	  	Status22Sequence_H1  = R_film_Status22Sequence_H1 ;
	  	Status22Sequence_H2  = R_film_Status22Sequence_H2 ;
	  	Status22Sequence_H3  = R_film_Status22Sequence_H3 ;
	  	Status22Sequence_V1  = R_film_Status22Sequence_V1 ;
	  	Status22Sequence_V2  = R_film_Status22Sequence_V2 ;
	  	Status22Sequence_V3  = R_film_Status22Sequence_V3 ;
	  	Status22Sequence_C   = R_film_Status22Sequence_C  ;

	  	StatusNextStaticPattern_film22    = R_film22_StatusNextStaticPattern   ;
	  	StatusNextStaticPattern_film22_H1 = R_film22_StatusNextStaticPattern_H1;
	  	StatusNextStaticPattern_film22_H2 = R_film22_StatusNextStaticPattern_H2;
	  	StatusNextStaticPattern_film22_H3 = R_film22_StatusNextStaticPattern_H3;
	  	StatusNextStaticPattern_film22_V1 = R_film22_StatusNextStaticPattern_V1;
	  	StatusNextStaticPattern_film22_V2 = R_film22_StatusNextStaticPattern_V2;
	  	StatusNextStaticPattern_film22_V3 = R_film22_StatusNextStaticPattern_V3;
	  	StatusNextStaticPattern_film22_C  = R_film22_StatusNextStaticPattern_C ;

	 	Status32Detected1_cnt   =R_Status32Detected1_cnt   ;
	 	Status32Detected1_H1_cnt=R_Status32Detected1_H1_cnt;
	 	Status32Detected1_H2_cnt=R_Status32Detected1_H2_cnt;
	 	Status32Detected1_H3_cnt=R_Status32Detected1_H3_cnt;
	 	Status32Detected1_V1_cnt=R_Status32Detected1_V1_cnt;
	 	Status32Detected1_V2_cnt=R_Status32Detected1_V2_cnt;
	 	Status32Detected1_V3_cnt=R_Status32Detected1_V3_cnt;
	 	Status32Detected1_C_cnt =R_Status32Detected1_C_cnt ;

	 	Status32Detected2_cnt   =R_Status32Detected2_cnt   ;
	 	Status32Detected2_H1_cnt=R_Status32Detected2_H1_cnt;
	 	Status32Detected2_H2_cnt=R_Status32Detected2_H2_cnt;
	 	Status32Detected2_H3_cnt=R_Status32Detected2_H3_cnt;
	 	Status32Detected2_V1_cnt=R_Status32Detected2_V1_cnt;
	 	Status32Detected2_V2_cnt=R_Status32Detected2_V2_cnt;
	 	Status32Detected2_V3_cnt=R_Status32Detected2_V3_cnt;
	 	Status32Detected2_C_cnt =R_Status32Detected2_C_cnt ;

	 	Status22Detected_cnt   =R_Status22Detected_cnt   ;
	 	Status22Detected_H1_cnt=R_Status22Detected_H1_cnt;
	 	Status22Detected_H2_cnt=R_Status22Detected_H2_cnt;
	 	Status22Detected_H3_cnt=R_Status22Detected_H3_cnt;
	 	Status22Detected_V1_cnt=R_Status22Detected_V1_cnt;
	 	Status22Detected_V2_cnt=R_Status22Detected_V2_cnt;
	 	Status22Detected_V3_cnt=R_Status22Detected_V3_cnt;
	 	Status22Detected_C_cnt =R_Status22Detected_C_cnt ;

		StatusFilmDetected     =  R_film_StatusFilmDetected;
		StatusFilmSequence     =  R_film_StatusFilmSequence;
		StatusMixedFilmDetected=  R_StatusMixedFilmDetected;

		NextTopDetected_d                     =  R_NextTopDetected_d      ;
		NextTopDetected                       =  R_NextTopDetected        ;
		PrevTopDetected_d                     =  R_PrevTopDetected_d      ;
		PrevTopDetected                       =  R_PrevTopDetected        ;
		TopLine                               =  R_TopLine                ;
		BotLine                               =  R_BotLine                ;
		TopLine2                              =  R_TopLine2               ;
		BotLine2                              =  R_BotLine2               ;
		TopLine3                              =  R_TopLine3               ;
		BotLine3                              =  R_BotLine3               ;

		// fw add
		index32 = R_film_index32;
		index32_H1 = R_film_index32_H1;
		index32_H2 = R_film_index32_H2;
		index32_H3 = R_film_index32_H3;
		index32_V1 = R_film_index32_V1;
		index32_V2 = R_film_index32_V2;
		index32_V3 = R_film_index32_V3;
		index32_C = R_film_index32_C;

		/*
		stillpix_count32 = R_film_stillpix_count32;
		stillpix_count32_H1 = R_film_stillpix_count32_H1;
		stillpix_count32_H2 = R_film_stillpix_count32_H2;
		stillpix_count32_H3 = R_film_stillpix_count32_H3;
		stillpix_count32_V1 = R_film_stillpix_count32_V1;
		stillpix_count32_V2 = R_film_stillpix_count32_V2;
		stillpix_count32_V3 = R_film_stillpix_count32_V3;
		stillpix_count32_C = R_film_stillpix_count32_C;
		*/
		index322 = R_film_index322;
		index322_H1 = R_film_index322_H1;
		index322_H2 = R_film_index322_H2;
		index322_H3 = R_film_index322_H3;
		index322_V1 = R_film_index322_V1;
		index322_V2 = R_film_index322_V2;
		index322_V3 = R_film_index322_V3;
		index322_C = R_film_index322_C;

		index22 = R_film_index22;
		index22_H1 = R_film_index22_H1;
		index22_H2 = R_film_index22_H2;
		index22_H3 = R_film_index22_H3;
		index22_V1 = R_film_index22_V1;
		index22_V2 = R_film_index22_V2;
		index22_V3 = R_film_index22_V3;
		index22_C = R_film_index22_C;

		/*
		stillpix_count22 = R_film_stillpix_count22;
		stillpix_count22_H1 = R_film_stillpix_count22_H1;
		stillpix_count22_H2 = R_film_stillpix_count22_H2;
		stillpix_count22_H3 = R_film_stillpix_count22_H3;
		stillpix_count22_V1 = R_film_stillpix_count22_V1;
		stillpix_count22_V2 = R_film_stillpix_count22_V2;
		stillpix_count22_V3 = R_film_stillpix_count22_V3;
		stillpix_count22_C = R_film_stillpix_count22_C;
		*/

		film_index = R_film_index;
		force_weave_flag = R_force_weave_flag;
		weave_prev = R_weave_prev;
	  }
	  else
	  {
		//Left=======================================
		StatusFrameStaticPattern    = L_film_StatusFrameStaticPattern   ;
		StatusFrameStaticPattern_H1 = L_film_StatusFrameStaticPattern_H1;
		StatusFrameStaticPattern_H2 = L_film_StatusFrameStaticPattern_H2;
		StatusFrameStaticPattern_H3 = L_film_StatusFrameStaticPattern_H3;
		StatusFrameStaticPattern_V1 = L_film_StatusFrameStaticPattern_V1;
		StatusFrameStaticPattern_V2 = L_film_StatusFrameStaticPattern_V2;
		StatusFrameStaticPattern_V3 = L_film_StatusFrameStaticPattern_V3;
		StatusFrameStaticPattern_C  = L_film_StatusFrameStaticPattern_C ;

		StatusFramePairPattern    = L_film_StatusFramePairPattern   ;
		StatusFramePairPattern_H1 = L_film_StatusFramePairPattern_H1;
		StatusFramePairPattern_H2 = L_film_StatusFramePairPattern_H2;
		StatusFramePairPattern_H3 = L_film_StatusFramePairPattern_H3;
		StatusFramePairPattern_V1 = L_film_StatusFramePairPattern_V1;
		StatusFramePairPattern_V2 = L_film_StatusFramePairPattern_V2;
		StatusFramePairPattern_V3 = L_film_StatusFramePairPattern_V3;
		StatusFramePairPattern_C  = L_film_StatusFramePairPattern_C ;

		StatusPrevStaticPattern2    = L_film_StatusPrevStaticPattern2   ;
		StatusPrevStaticPattern2_H1 = L_film_StatusPrevStaticPattern2_H1;
		StatusPrevStaticPattern2_H2 = L_film_StatusPrevStaticPattern2_H2;
		StatusPrevStaticPattern2_H3 = L_film_StatusPrevStaticPattern2_H3;
		StatusPrevStaticPattern2_V1 = L_film_StatusPrevStaticPattern2_V1;
		StatusPrevStaticPattern2_V2 = L_film_StatusPrevStaticPattern2_V2;
		StatusPrevStaticPattern2_V3 = L_film_StatusPrevStaticPattern2_V3;
		StatusPrevStaticPattern2_C  = L_film_StatusPrevStaticPattern2_C ;

		StatusNextStaticPattern    = L_film_StatusNextStaticPattern   ;
		StatusNextStaticPattern_H1 = L_film_StatusNextStaticPattern_H1;
		StatusNextStaticPattern_H2 = L_film_StatusNextStaticPattern_H2;
		StatusNextStaticPattern_H3 = L_film_StatusNextStaticPattern_H3;
		StatusNextStaticPattern_V1 = L_film_StatusNextStaticPattern_V1;
		StatusNextStaticPattern_V2 = L_film_StatusNextStaticPattern_V2;
		StatusNextStaticPattern_V3 = L_film_StatusNextStaticPattern_V3;
		StatusNextStaticPattern_C  = L_film_StatusNextStaticPattern_C ;

		StatusNextStaticPattern2    = L_film_StatusNextStaticPattern2   ;
		StatusNextStaticPattern2_H1 = L_film_StatusNextStaticPattern2_H1;
		StatusNextStaticPattern2_H2 = L_film_StatusNextStaticPattern2_H2;
		StatusNextStaticPattern2_H3 = L_film_StatusNextStaticPattern2_H3;
		StatusNextStaticPattern2_V1 = L_film_StatusNextStaticPattern2_V1;
		StatusNextStaticPattern2_V2 = L_film_StatusNextStaticPattern2_V2;
		StatusNextStaticPattern2_V3 = L_film_StatusNextStaticPattern2_V3;
		StatusNextStaticPattern2_C  = L_film_StatusNextStaticPattern2_C ;

		Status32Detected1    = L_film_Status32Detected1   ;
		Status32Detected2    = L_film_Status32Detected2   ;
		Status32Detected1_H1 = L_film_Status32Detected1_H1;
		Status32Detected1_H2 = L_film_Status32Detected1_H2;
		Status32Detected1_H3 = L_film_Status32Detected1_H3;
		Status32Detected1_V1 = L_film_Status32Detected1_V1;
		Status32Detected1_V2 = L_film_Status32Detected1_V2;
		Status32Detected1_V3 = L_film_Status32Detected1_V3;
		Status32Detected1_C  = L_film_Status32Detected1_C ;
		Status32Detected2_H1 = L_film_Status32Detected2_H1;
		Status32Detected2_H2 = L_film_Status32Detected2_H2;
		Status32Detected2_H3 = L_film_Status32Detected2_H3;
		Status32Detected2_V1 = L_film_Status32Detected2_V1;
		Status32Detected2_V2 = L_film_Status32Detected2_V2;
		Status32Detected2_V3 = L_film_Status32Detected2_V3;
		Status32Detected2_C  = L_film_Status32Detected2_C ;
		Status22Detected     = L_film_Status22Detected    ;
		Status22Detected_H1  = L_film_Status22Detected_H1 ;
		Status22Detected_H2  = L_film_Status22Detected_H2 ;
		Status22Detected_H3  = L_film_Status22Detected_H3 ;
		Status22Detected_V1  = L_film_Status22Detected_V1 ;
		Status22Detected_V2  = L_film_Status22Detected_V2 ;
		Status22Detected_V3  = L_film_Status22Detected_V3 ;
		Status22Detected_C   = L_film_Status22Detected_C  ;

	  	Status32Sequence1    = L_film_Status32Sequence1   ;
	  	Status32Sequence2    = L_film_Status32Sequence2   ;
	  	Status32Sequence1_H1 = L_film_Status32Sequence1_H1;
	  	Status32Sequence1_H2 = L_film_Status32Sequence1_H2;
	  	Status32Sequence1_H3 = L_film_Status32Sequence1_H3;
	  	Status32Sequence1_V1 = L_film_Status32Sequence1_V1;
	  	Status32Sequence1_V2 = L_film_Status32Sequence1_V2;
	  	Status32Sequence1_V3 = L_film_Status32Sequence1_V3;
	  	Status32Sequence1_C  = L_film_Status32Sequence1_C ;
	  	Status32Sequence2_H1 = L_film_Status32Sequence2_H1;
	  	Status32Sequence2_H2 = L_film_Status32Sequence2_H2;
	  	Status32Sequence2_H3 = L_film_Status32Sequence2_H3;
	  	Status32Sequence2_V1 = L_film_Status32Sequence2_V1;
	  	Status32Sequence2_V2 = L_film_Status32Sequence2_V2;
	  	Status32Sequence2_V3 = L_film_Status32Sequence2_V3;
	  	Status32Sequence2_C  = L_film_Status32Sequence2_C ;
	  	Status22Sequence     = L_film_Status22Sequence    ;
	  	Status22Sequence_H1  = L_film_Status22Sequence_H1 ;
	  	Status22Sequence_H2  = L_film_Status22Sequence_H2 ;
	  	Status22Sequence_H3  = L_film_Status22Sequence_H3 ;
	  	Status22Sequence_V1  = L_film_Status22Sequence_V1 ;
	  	Status22Sequence_V2  = L_film_Status22Sequence_V2 ;
	  	Status22Sequence_V3  = L_film_Status22Sequence_V3 ;
	  	Status22Sequence_C   = L_film_Status22Sequence_C  ;

	  	StatusNextStaticPattern_film22    = L_film22_StatusNextStaticPattern   ;
	  	StatusNextStaticPattern_film22_H1 = L_film22_StatusNextStaticPattern_H1;
	  	StatusNextStaticPattern_film22_H2 = L_film22_StatusNextStaticPattern_H2;
	  	StatusNextStaticPattern_film22_H3 = L_film22_StatusNextStaticPattern_H3;
	  	StatusNextStaticPattern_film22_V1 = L_film22_StatusNextStaticPattern_V1;
	  	StatusNextStaticPattern_film22_V2 = L_film22_StatusNextStaticPattern_V2;
	  	StatusNextStaticPattern_film22_V3 = L_film22_StatusNextStaticPattern_V3;
	  	StatusNextStaticPattern_film22_C  = L_film22_StatusNextStaticPattern_C ;

	 	Status32Detected1_cnt   =L_Status32Detected1_cnt   ;
	 	Status32Detected1_H1_cnt=L_Status32Detected1_H1_cnt;
	 	Status32Detected1_H2_cnt=L_Status32Detected1_H2_cnt;
	 	Status32Detected1_H3_cnt=L_Status32Detected1_H3_cnt;
	 	Status32Detected1_V1_cnt=L_Status32Detected1_V1_cnt;
	 	Status32Detected1_V2_cnt=L_Status32Detected1_V2_cnt;
	 	Status32Detected1_V3_cnt=L_Status32Detected1_V3_cnt;
	 	Status32Detected1_C_cnt =L_Status32Detected1_C_cnt ;

	 	Status32Detected2_cnt   =L_Status32Detected2_cnt   ;
	 	Status32Detected2_H1_cnt=L_Status32Detected2_H1_cnt;
	 	Status32Detected2_H2_cnt=L_Status32Detected2_H2_cnt;
	 	Status32Detected2_H3_cnt=L_Status32Detected2_H3_cnt;
	 	Status32Detected2_V1_cnt=L_Status32Detected2_V1_cnt;
	 	Status32Detected2_V2_cnt=L_Status32Detected2_V2_cnt;
	 	Status32Detected2_V3_cnt=L_Status32Detected2_V3_cnt;
	 	Status32Detected2_C_cnt =L_Status32Detected2_C_cnt ;

	 	Status22Detected_cnt   =L_Status22Detected_cnt   ;
	 	Status22Detected_H1_cnt=L_Status22Detected_H1_cnt;
	 	Status22Detected_H2_cnt=L_Status22Detected_H2_cnt;
	 	Status22Detected_H3_cnt=L_Status22Detected_H3_cnt;
	 	Status22Detected_V1_cnt=L_Status22Detected_V1_cnt;
	 	Status22Detected_V2_cnt=L_Status22Detected_V2_cnt;
	 	Status22Detected_V3_cnt=L_Status22Detected_V3_cnt;
	 	Status22Detected_C_cnt =L_Status22Detected_C_cnt ;

		StatusFilmDetected	  =  L_film_StatusFilmDetected;
		StatusFilmSequence     =  L_film_StatusFilmSequence;
		StatusMixedFilmDetected=  L_StatusMixedFilmDetected;
		NextTopDetected_d                     =  L_NextTopDetected_d      ;
		NextTopDetected                       =  L_NextTopDetected        ;
		PrevTopDetected_d                     =  L_PrevTopDetected_d      ;
		PrevTopDetected                       =  L_PrevTopDetected        ;
		TopLine                               =  L_TopLine                ;
		BotLine                               =  L_BotLine                ;
		TopLine2                              =  L_TopLine2               ;
		BotLine2                              =  L_BotLine2               ;
		TopLine3                              =  L_TopLine3               ;
		BotLine3                              =  L_BotLine3               ;

		// fw add
		index32 = L_film_index32;
		index32_H1 = L_film_index32_H1;
		index32_H2 = L_film_index32_H2;
		index32_H3 = L_film_index32_H3;
		index32_V1 = L_film_index32_V1;
		index32_V2 = L_film_index32_V2;
		index32_V3 = L_film_index32_V3;
		index32_C = L_film_index32_C;

		/*
		stillpix_count32 = L_film_stillpix_count32;
		stillpix_count32_H1 = L_film_stillpix_count32_H1;
		stillpix_count32_H2 = L_film_stillpix_count32_H2;
		stillpix_count32_H3 = L_film_stillpix_count32_H3;
		stillpix_count32_V1 = L_film_stillpix_count32_V1;
		stillpix_count32_V2 = L_film_stillpix_count32_V2;
		stillpix_count32_V3 = L_film_stillpix_count32_V3;
		stillpix_count32_C = L_film_stillpix_count32_C;
		*/

		index322 = L_film_index322;
		index322_H1 = L_film_index322_H1;
		index322_H2 = L_film_index322_H2;
		index322_H3 = L_film_index322_H3;
		index322_V1 = L_film_index322_V1;
		index322_V2 = L_film_index322_V2;
		index322_V3 = L_film_index322_V3;
		index322_C = L_film_index322_C;

		index22 = L_film_index22;
		index22_H1 = L_film_index22_H1;
		index22_H2 = L_film_index22_H2;
		index22_H3 = L_film_index22_H3;
		index22_V1 = L_film_index22_V1;
		index22_V2 = L_film_index22_V2;
		index22_V3 = L_film_index22_V3;
		index22_C = L_film_index22_C;

		/*
		stillpix_count22 = L_film_stillpix_count22;
		stillpix_count22_H1 = L_film_stillpix_count22_H1;
		stillpix_count22_H2 = L_film_stillpix_count22_H2;
		stillpix_count22_H3 = L_film_stillpix_count22_H3;
		stillpix_count22_V1 = L_film_stillpix_count22_V1;
		stillpix_count22_V2 = L_film_stillpix_count22_V2;
		stillpix_count22_V3 = L_film_stillpix_count22_V3;
		stillpix_count22_C = L_film_stillpix_count22_C;
		*/

		film_index = L_film_index;
		force_weave_flag = L_force_weave_flag;
		weave_prev = L_weave_prev;
	  }
}
void scalerVIP_film_FWmode_3D_end()
{
	di_im_di_debug_mode_RBUS reg18024008_Reg;
	reg18024008_Reg.regValue = rtd_inl(DI_IM_DI_DEBUG_MODE_reg);
	unsigned char hw_3D_l_flag = reg18024008_Reg.di_3d_l_flag;

	  if(hw_3D_l_flag==0)// || reg_vc_3D_Enable==0)
	  {
		//Right=======================================
		R_film_StatusFrameStaticPattern    = StatusFrameStaticPattern   ;
		R_film_StatusFrameStaticPattern_H1 = StatusFrameStaticPattern_H1;
		R_film_StatusFrameStaticPattern_H2 = StatusFrameStaticPattern_H2;
		R_film_StatusFrameStaticPattern_H3 = StatusFrameStaticPattern_H3;
		R_film_StatusFrameStaticPattern_V1 = StatusFrameStaticPattern_V1;
		R_film_StatusFrameStaticPattern_V2 = StatusFrameStaticPattern_V2;
		R_film_StatusFrameStaticPattern_V3 = StatusFrameStaticPattern_V3;
		R_film_StatusFrameStaticPattern_C  = StatusFrameStaticPattern_C ;

		R_film_StatusFramePairPattern    = StatusFramePairPattern   ;
		R_film_StatusFramePairPattern_H1 = StatusFramePairPattern_H1;
		R_film_StatusFramePairPattern_H2 = StatusFramePairPattern_H2;
		R_film_StatusFramePairPattern_H3 = StatusFramePairPattern_H3;
		R_film_StatusFramePairPattern_V1 = StatusFramePairPattern_V1;
		R_film_StatusFramePairPattern_V2 = StatusFramePairPattern_V2;
		R_film_StatusFramePairPattern_V3 = StatusFramePairPattern_V3;
		R_film_StatusFramePairPattern_C  = StatusFramePairPattern_C ;

		R_film_StatusPrevStaticPattern2    = StatusPrevStaticPattern2   ;
		R_film_StatusPrevStaticPattern2_H1 = StatusPrevStaticPattern2_H1;
		R_film_StatusPrevStaticPattern2_H2 = StatusPrevStaticPattern2_H2;
		R_film_StatusPrevStaticPattern2_H3 = StatusPrevStaticPattern2_H3;
		R_film_StatusPrevStaticPattern2_V1 = StatusPrevStaticPattern2_V1;
		R_film_StatusPrevStaticPattern2_V2 = StatusPrevStaticPattern2_V2;
		R_film_StatusPrevStaticPattern2_V3 = StatusPrevStaticPattern2_V3;
		R_film_StatusPrevStaticPattern2_C  = StatusPrevStaticPattern2_C ;

		R_film_StatusNextStaticPattern    = StatusNextStaticPattern   ;
		R_film_StatusNextStaticPattern_H1 = StatusNextStaticPattern_H1;
		R_film_StatusNextStaticPattern_H2 = StatusNextStaticPattern_H2;
		R_film_StatusNextStaticPattern_H3 = StatusNextStaticPattern_H3;
		R_film_StatusNextStaticPattern_V1 = StatusNextStaticPattern_V1;
		R_film_StatusNextStaticPattern_V2 = StatusNextStaticPattern_V2;
		R_film_StatusNextStaticPattern_V3 = StatusNextStaticPattern_V3;
		R_film_StatusNextStaticPattern_C  = StatusNextStaticPattern_C ;

		R_film_StatusNextStaticPattern2    = StatusNextStaticPattern2   ;
		R_film_StatusNextStaticPattern2_H1 = StatusNextStaticPattern2_H1;
		R_film_StatusNextStaticPattern2_H2 = StatusNextStaticPattern2_H2;
		R_film_StatusNextStaticPattern2_H3 = StatusNextStaticPattern2_H3;
		R_film_StatusNextStaticPattern2_V1 = StatusNextStaticPattern2_V1;
		R_film_StatusNextStaticPattern2_V2 = StatusNextStaticPattern2_V2;
		R_film_StatusNextStaticPattern2_V3 = StatusNextStaticPattern2_V3;
		R_film_StatusNextStaticPattern2_C  = StatusNextStaticPattern2_C ;

		R_film_Status32Detected1    = Status32Detected1   ;
		R_film_Status32Detected2    = Status32Detected2   ;
		R_film_Status32Detected1_H1 = Status32Detected1_H1;
		R_film_Status32Detected1_H2 = Status32Detected1_H2;
		R_film_Status32Detected1_H3 = Status32Detected1_H3;
		R_film_Status32Detected1_V1 = Status32Detected1_V1;
		R_film_Status32Detected1_V2 = Status32Detected1_V2;
		R_film_Status32Detected1_V3 = Status32Detected1_V3;
		R_film_Status32Detected1_C  = Status32Detected1_C ;
		R_film_Status32Detected2_H1 = Status32Detected2_H1;
		R_film_Status32Detected2_H2 = Status32Detected2_H2;
		R_film_Status32Detected2_H3 = Status32Detected2_H3;
		R_film_Status32Detected2_V1 = Status32Detected2_V1;
		R_film_Status32Detected2_V2 = Status32Detected2_V2;
		R_film_Status32Detected2_V3 = Status32Detected2_V3;
		R_film_Status32Detected2_C  = Status32Detected2_C ;
		R_film_Status22Detected     = Status22Detected    ;
		R_film_Status22Detected_H1  = Status22Detected_H1 ;
		R_film_Status22Detected_H2  = Status22Detected_H2 ;
		R_film_Status22Detected_H3  = Status22Detected_H3 ;
		R_film_Status22Detected_V1  = Status22Detected_V1 ;
		R_film_Status22Detected_V2  = Status22Detected_V2 ;
		R_film_Status22Detected_V3  = Status22Detected_V3 ;
		R_film_Status22Detected_C   = Status22Detected_C  ;

	  	R_film_Status32Sequence1    = Status32Sequence1   ;
	  	R_film_Status32Sequence2    = Status32Sequence2   ;
	  	R_film_Status32Sequence1_H1 = Status32Sequence1_H1;
	  	R_film_Status32Sequence1_H2 = Status32Sequence1_H2;
	  	R_film_Status32Sequence1_H3 = Status32Sequence1_H3;
	  	R_film_Status32Sequence1_V1 = Status32Sequence1_V1;
	  	R_film_Status32Sequence1_V2 = Status32Sequence1_V2;
	  	R_film_Status32Sequence1_V3 = Status32Sequence1_V3;
	  	R_film_Status32Sequence1_C  = Status32Sequence1_C ;
	  	R_film_Status32Sequence2_H1 = Status32Sequence2_H1;
	  	R_film_Status32Sequence2_H2 = Status32Sequence2_H2;
	  	R_film_Status32Sequence2_H3 = Status32Sequence2_H3;
	  	R_film_Status32Sequence2_V1 = Status32Sequence2_V1;
	  	R_film_Status32Sequence2_V2 = Status32Sequence2_V2;
	  	R_film_Status32Sequence2_V3 = Status32Sequence2_V3;
	  	R_film_Status32Sequence2_C  = Status32Sequence2_C ;
	  	R_film_Status22Sequence     = Status22Sequence    ;
	  	R_film_Status22Sequence_H1  = Status22Sequence_H1 ;
	  	R_film_Status22Sequence_H2  = Status22Sequence_H2 ;
	  	R_film_Status22Sequence_H3  = Status22Sequence_H3 ;
	  	R_film_Status22Sequence_V1  = Status22Sequence_V1 ;
	  	R_film_Status22Sequence_V2  = Status22Sequence_V2 ;
	  	R_film_Status22Sequence_V3  = Status22Sequence_V3 ;
	  	R_film_Status22Sequence_C   = Status22Sequence_C  ;

	  	R_film22_StatusNextStaticPattern    = StatusNextStaticPattern_film22   ;
	  	R_film22_StatusNextStaticPattern_H1 = StatusNextStaticPattern_film22_H1;
	  	R_film22_StatusNextStaticPattern_H2 = StatusNextStaticPattern_film22_H2;
	  	R_film22_StatusNextStaticPattern_H3 = StatusNextStaticPattern_film22_H3;
	  	R_film22_StatusNextStaticPattern_V1 = StatusNextStaticPattern_film22_V1;
	  	R_film22_StatusNextStaticPattern_V2 = StatusNextStaticPattern_film22_V2;
	  	R_film22_StatusNextStaticPattern_V3 = StatusNextStaticPattern_film22_V3;
	  	R_film22_StatusNextStaticPattern_C  = StatusNextStaticPattern_film22_C ;

	 	R_Status32Detected1_cnt   =Status32Detected1_cnt   ;
	 	R_Status32Detected1_H1_cnt=Status32Detected1_H1_cnt;
	 	R_Status32Detected1_H2_cnt=Status32Detected1_H2_cnt;
	 	R_Status32Detected1_H3_cnt=Status32Detected1_H3_cnt;
	 	R_Status32Detected1_V1_cnt=Status32Detected1_V1_cnt;
	 	R_Status32Detected1_V2_cnt=Status32Detected1_V2_cnt;
	 	R_Status32Detected1_V3_cnt=Status32Detected1_V3_cnt;
	 	R_Status32Detected1_C_cnt =Status32Detected1_C_cnt ;

	 	R_Status32Detected2_cnt   =Status32Detected2_cnt   ;
	 	R_Status32Detected2_H1_cnt=Status32Detected2_H1_cnt;
	 	R_Status32Detected2_H2_cnt=Status32Detected2_H2_cnt;
	 	R_Status32Detected2_H3_cnt=Status32Detected2_H3_cnt;
	 	R_Status32Detected2_V1_cnt=Status32Detected2_V1_cnt;
	 	R_Status32Detected2_V2_cnt=Status32Detected2_V2_cnt;
	 	R_Status32Detected2_V3_cnt=Status32Detected2_V3_cnt;
	 	R_Status32Detected2_C_cnt =Status32Detected2_C_cnt ;

	 	R_Status22Detected_cnt   =Status22Detected_cnt   ;
	 	R_Status22Detected_H1_cnt=Status22Detected_H1_cnt;
	 	R_Status22Detected_H2_cnt=Status22Detected_H2_cnt;
	 	R_Status22Detected_H3_cnt=Status22Detected_H3_cnt;
	 	R_Status22Detected_V1_cnt=Status22Detected_V1_cnt;
	 	R_Status22Detected_V2_cnt=Status22Detected_V2_cnt;
	 	R_Status22Detected_V3_cnt=Status22Detected_V3_cnt;
	 	R_Status22Detected_C_cnt =Status22Detected_C_cnt ;

		R_film_StatusFilmDetected=StatusFilmDetected       ;
		R_film_StatusFilmSequence= StatusFilmSequence      ;
		R_StatusMixedFilmDetected= StatusMixedFilmDetected ;
		R_NextTopDetected_d      = NextTopDetected_d                      ;
		R_NextTopDetected        = NextTopDetected                        ;
		R_PrevTopDetected_d      = PrevTopDetected_d                      ;
		R_PrevTopDetected        = PrevTopDetected                        ;
		R_TopLine                = TopLine                                ;
		R_BotLine                = BotLine                                ;
		R_TopLine2               = TopLine2                               ;
		R_BotLine2               = BotLine2                               ;
		R_TopLine3               = TopLine3                               ;
		R_BotLine3               = BotLine3                               ;

		// fw add
		R_film_index32 = index32  ;
		R_film_index32_H1 = index32_H1 ;
		R_film_index32_H2 = index32_H2 ;
		R_film_index32_H3 = index32_H3 ;
		R_film_index32_V1 = index32_V1  ;
		R_film_index32_V2 = index32_V2  ;
		R_film_index32_V3 = index32_V3  ;
		R_film_index32_C = index32_C  ;

		/*
		R_film_stillpix_count32 = stillpix_count32 ;
		R_film_stillpix_count32_H1 = stillpix_count32_H1 ;
		R_film_stillpix_count32_H2 = stillpix_count32_H2 ;
		R_film_stillpix_count32_H3 = stillpix_count32_H3 ;
		R_film_stillpix_count32_V1 = stillpix_count32_V1 ;
		R_film_stillpix_count32_V2 = stillpix_count32_V2 ;
		R_film_stillpix_count32_V3 = stillpix_count32_V3 ;
		R_film_stillpix_count32_C = stillpix_count32_C ;
		*/

		R_film_index322 = index322 ;
		R_film_index322_H1 = index322_H1;
		R_film_index322_H2 = index322_H2;
		R_film_index322_H3 = index322_H3 ;
		R_film_index322_V1 = index322_V1 ;
		R_film_index322_V2 = index322_V2 ;
		R_film_index322_V3 = index322_V3 ;
		R_film_index322_C = index322_C ;

		R_film_index22 = index22 ;
		R_film_index22_H1 = index22_H1 ;
		R_film_index22_H2 = index22_H2 ;
		R_film_index22_H3 = index22_H3  ;
		R_film_index22_V1 = index22_V1  ;
		R_film_index22_V2 = index22_V2  ;
		R_film_index22_V3 = index22_V3  ;
		R_film_index22_C = index22_C ;

		/*
		R_film_stillpix_count22 = stillpix_count22 ;
		R_film_stillpix_count22_H1 = stillpix_count22_H1 ;
		R_film_stillpix_count22_H2 = stillpix_count22_H2 ;
		R_film_stillpix_count22_H3 = stillpix_count22_H3 ;
		R_film_stillpix_count22_V1 = stillpix_count22_V1 ;
		R_film_stillpix_count22_V2 = stillpix_count22_V2 ;
		R_film_stillpix_count22_V3 = stillpix_count22_V3 ;
		R_film_stillpix_count22_C = stillpix_count22_C ;
		*/

		R_film_index = film_index ;
		R_force_weave_flag = force_weave_flag ;
		R_weave_prev = weave_prev ;
	  }
	  else
	  {
		L_film_StatusFrameStaticPattern    = StatusFrameStaticPattern   ;
		L_film_StatusFrameStaticPattern_H1 = StatusFrameStaticPattern_H1;
		L_film_StatusFrameStaticPattern_H2 = StatusFrameStaticPattern_H2;
		L_film_StatusFrameStaticPattern_H3 = StatusFrameStaticPattern_H3;
		L_film_StatusFrameStaticPattern_V1 = StatusFrameStaticPattern_V1;
		L_film_StatusFrameStaticPattern_V2 = StatusFrameStaticPattern_V2;
		L_film_StatusFrameStaticPattern_V3 = StatusFrameStaticPattern_V3;
		L_film_StatusFrameStaticPattern_C  = StatusFrameStaticPattern_C ;

		L_film_StatusFramePairPattern    = StatusFramePairPattern   ;
		L_film_StatusFramePairPattern_H1 = StatusFramePairPattern_H1;
		L_film_StatusFramePairPattern_H2 = StatusFramePairPattern_H2;
		L_film_StatusFramePairPattern_H3 = StatusFramePairPattern_H3;
		L_film_StatusFramePairPattern_V1 = StatusFramePairPattern_V1;
		L_film_StatusFramePairPattern_V2 = StatusFramePairPattern_V2;
		L_film_StatusFramePairPattern_V3 = StatusFramePairPattern_V3;
		L_film_StatusFramePairPattern_C  = StatusFramePairPattern_C ;

		L_film_StatusPrevStaticPattern2    = StatusPrevStaticPattern2   ;
		L_film_StatusPrevStaticPattern2_H1 = StatusPrevStaticPattern2_H1;
		L_film_StatusPrevStaticPattern2_H2 = StatusPrevStaticPattern2_H2;
		L_film_StatusPrevStaticPattern2_H3 = StatusPrevStaticPattern2_H3;
		L_film_StatusPrevStaticPattern2_V1 = StatusPrevStaticPattern2_V1;
		L_film_StatusPrevStaticPattern2_V2 = StatusPrevStaticPattern2_V2;
		L_film_StatusPrevStaticPattern2_V3 = StatusPrevStaticPattern2_V3;
		L_film_StatusPrevStaticPattern2_C  = StatusPrevStaticPattern2_C ;

		L_film_StatusNextStaticPattern    = StatusNextStaticPattern   ;
		L_film_StatusNextStaticPattern_H1 = StatusNextStaticPattern_H1;
		L_film_StatusNextStaticPattern_H2 = StatusNextStaticPattern_H2;
		L_film_StatusNextStaticPattern_H3 = StatusNextStaticPattern_H3;
		L_film_StatusNextStaticPattern_V1 = StatusNextStaticPattern_V1;
		L_film_StatusNextStaticPattern_V2 = StatusNextStaticPattern_V2;
		L_film_StatusNextStaticPattern_V3 = StatusNextStaticPattern_V3;
		L_film_StatusNextStaticPattern_C  = StatusNextStaticPattern_C ;

		L_film_StatusNextStaticPattern2    = StatusNextStaticPattern2   ;
		L_film_StatusNextStaticPattern2_H1 = StatusNextStaticPattern2_H1;
		L_film_StatusNextStaticPattern2_H2 = StatusNextStaticPattern2_H2;
		L_film_StatusNextStaticPattern2_H3 = StatusNextStaticPattern2_H3;
		L_film_StatusNextStaticPattern2_V1 = StatusNextStaticPattern2_V1;
		L_film_StatusNextStaticPattern2_V2 = StatusNextStaticPattern2_V2;
		L_film_StatusNextStaticPattern2_V3 = StatusNextStaticPattern2_V3;
		L_film_StatusNextStaticPattern2_C  = StatusNextStaticPattern2_C ;

		L_film_Status32Detected1    = Status32Detected1   ;
		L_film_Status32Detected2    = Status32Detected2   ;
		L_film_Status32Detected1_H1 = Status32Detected1_H1;
		L_film_Status32Detected1_H2 = Status32Detected1_H2;
		L_film_Status32Detected1_H3 = Status32Detected1_H3;
		L_film_Status32Detected1_V1 = Status32Detected1_V1;
		L_film_Status32Detected1_V2 = Status32Detected1_V2;
		L_film_Status32Detected1_V3 = Status32Detected1_V3;
		L_film_Status32Detected1_C  = Status32Detected1_C ;
		L_film_Status32Detected2_H1 = Status32Detected2_H1;
		L_film_Status32Detected2_H2 = Status32Detected2_H2;
		L_film_Status32Detected2_H3 = Status32Detected2_H3;
		L_film_Status32Detected2_V1 = Status32Detected2_V1;
		L_film_Status32Detected2_V2 = Status32Detected2_V2;
		L_film_Status32Detected2_V3 = Status32Detected2_V3;
		L_film_Status32Detected2_C  = Status32Detected2_C ;
		L_film_Status22Detected     = Status22Detected    ;
		L_film_Status22Detected_H1  = Status22Detected_H1 ;
		L_film_Status22Detected_H2  = Status22Detected_H2 ;
		L_film_Status22Detected_H3  = Status22Detected_H3 ;
		L_film_Status22Detected_V1  = Status22Detected_V1 ;
		L_film_Status22Detected_V2  = Status22Detected_V2 ;
		L_film_Status22Detected_V3  = Status22Detected_V3 ;
		L_film_Status22Detected_C   = Status22Detected_C  ;

	  	L_film_Status32Sequence1    = Status32Sequence1   ;
	  	L_film_Status32Sequence2    = Status32Sequence2   ;
	  	L_film_Status32Sequence1_H1 = Status32Sequence1_H1;
	  	L_film_Status32Sequence1_H2 = Status32Sequence1_H2;
	  	L_film_Status32Sequence1_H3 = Status32Sequence1_H3;
	  	L_film_Status32Sequence1_V1 = Status32Sequence1_V1;
	  	L_film_Status32Sequence1_V2 = Status32Sequence1_V2;
	  	L_film_Status32Sequence1_V3 = Status32Sequence1_V3;
	  	L_film_Status32Sequence1_C  = Status32Sequence1_C ;
	  	L_film_Status32Sequence2_H1 = Status32Sequence2_H1;
	  	L_film_Status32Sequence2_H2 = Status32Sequence2_H2;
	  	L_film_Status32Sequence2_H3 = Status32Sequence2_H3;
	  	L_film_Status32Sequence2_V1 = Status32Sequence2_V1;
	  	L_film_Status32Sequence2_V2 = Status32Sequence2_V2;
	  	L_film_Status32Sequence2_V3 = Status32Sequence2_V3;
	  	L_film_Status32Sequence2_C  = Status32Sequence2_C ;
	  	L_film_Status22Sequence     = Status22Sequence    ;
	  	L_film_Status22Sequence_H1  = Status22Sequence_H1 ;
	  	L_film_Status22Sequence_H2  = Status22Sequence_H2 ;
	  	L_film_Status22Sequence_H3  = Status22Sequence_H3 ;
	  	L_film_Status22Sequence_V1  = Status22Sequence_V1 ;
	  	L_film_Status22Sequence_V2  = Status22Sequence_V2 ;
	  	L_film_Status22Sequence_V3  = Status22Sequence_V3 ;
	  	L_film_Status22Sequence_C   = Status22Sequence_C  ;

	  	L_film22_StatusNextStaticPattern    = StatusNextStaticPattern_film22   ;
	  	L_film22_StatusNextStaticPattern_H1 = StatusNextStaticPattern_film22_H1;
	  	L_film22_StatusNextStaticPattern_H2 = StatusNextStaticPattern_film22_H2;
	  	L_film22_StatusNextStaticPattern_H3 = StatusNextStaticPattern_film22_H3;
	  	L_film22_StatusNextStaticPattern_V1 = StatusNextStaticPattern_film22_V1;
	  	L_film22_StatusNextStaticPattern_V2 = StatusNextStaticPattern_film22_V2;
	  	L_film22_StatusNextStaticPattern_V3 = StatusNextStaticPattern_film22_V3;
	  	L_film22_StatusNextStaticPattern_C  = StatusNextStaticPattern_film22_C ;

	 	L_Status32Detected1_cnt   =Status32Detected1_cnt   ;
	 	L_Status32Detected1_H1_cnt=Status32Detected1_H1_cnt;
	 	L_Status32Detected1_H2_cnt=Status32Detected1_H2_cnt;
	 	L_Status32Detected1_H3_cnt=Status32Detected1_H3_cnt;
	 	L_Status32Detected1_V1_cnt=Status32Detected1_V1_cnt;
	 	L_Status32Detected1_V2_cnt=Status32Detected1_V2_cnt;
	 	L_Status32Detected1_V3_cnt=Status32Detected1_V3_cnt;
	 	L_Status32Detected1_C_cnt =Status32Detected1_C_cnt ;

	 	L_Status32Detected2_cnt   =Status32Detected2_cnt   ;
	 	L_Status32Detected2_H1_cnt=Status32Detected2_H1_cnt;
	 	L_Status32Detected2_H2_cnt=Status32Detected2_H2_cnt;
	 	L_Status32Detected2_H3_cnt=Status32Detected2_H3_cnt;
	 	L_Status32Detected2_V1_cnt=Status32Detected2_V1_cnt;
	 	L_Status32Detected2_V2_cnt=Status32Detected2_V2_cnt;
	 	L_Status32Detected2_V3_cnt=Status32Detected2_V3_cnt;
	 	L_Status32Detected2_C_cnt =Status32Detected2_C_cnt ;

	 	L_Status22Detected_cnt   =Status22Detected_cnt   ;
	 	L_Status22Detected_H1_cnt=Status22Detected_H1_cnt;
	 	L_Status22Detected_H2_cnt=Status22Detected_H2_cnt;
	 	L_Status22Detected_H3_cnt=Status22Detected_H3_cnt;
	 	L_Status22Detected_V1_cnt=Status22Detected_V1_cnt;
	 	L_Status22Detected_V2_cnt=Status22Detected_V2_cnt;
	 	L_Status22Detected_V3_cnt=Status22Detected_V3_cnt;
	 	L_Status22Detected_C_cnt =Status22Detected_C_cnt ;

		L_film_StatusFilmDetected=StatusFilmDetected	      ;
		L_film_StatusFilmSequence= StatusFilmSequence      ;
		L_StatusMixedFilmDetected= StatusMixedFilmDetected ;
		L_NextTopDetected_d      = NextTopDetected_d                      ;
		L_NextTopDetected        = NextTopDetected                        ;
		L_PrevTopDetected_d      = PrevTopDetected_d                      ;
		L_PrevTopDetected        = PrevTopDetected                        ;
		L_TopLine                = TopLine                                ;
		L_BotLine                = BotLine                                ;
		L_TopLine2               = TopLine2                               ;
		L_BotLine2               = BotLine2                               ;
		L_TopLine3               = TopLine3                               ;
		L_BotLine3               = BotLine3                               ;

		// fw add
		L_film_index32 = index32  ;
		L_film_index32_H1 = index32_H1 ;
		L_film_index32_H2 = index32_H2 ;
		L_film_index32_H3 = index32_H3 ;
		L_film_index32_V1 = index32_V1  ;
		L_film_index32_V2 = index32_V2  ;
		L_film_index32_V3 = index32_V3  ;
		L_film_index32_C = index32_C  ;

		/*
		L_film_stillpix_count32 = stillpix_count32 ;
		L_film_stillpix_count32_H1 = stillpix_count32_H1 ;
		L_film_stillpix_count32_H2 = stillpix_count32_H2 ;
		L_film_stillpix_count32_H3 = stillpix_count32_H3 ;
		L_film_stillpix_count32_V1 = stillpix_count32_V1 ;
		L_film_stillpix_count32_V2 = stillpix_count32_V2 ;
		L_film_stillpix_count32_V3 = stillpix_count32_V3 ;
		L_film_stillpix_count32_C = stillpix_count32_C ;
		*/

		L_film_index322 = index322 ;
		L_film_index322_H1 = index322_H1;
		L_film_index322_H2 = index322_H2;
		L_film_index322_H3 = index322_H3 ;
		L_film_index322_V1 = index322_V1 ;
		L_film_index322_V2 = index322_V2 ;
		L_film_index322_V3 = index322_V3 ;
		L_film_index322_C = index322_C ;

		L_film_index22 = index22 ;
		L_film_index22_H1 = index22_H1 ;
		L_film_index22_H2 = index22_H2 ;
		L_film_index22_H3 = index22_H3  ;
		L_film_index22_V1 = index22_V1  ;
		L_film_index22_V2 = index22_V2  ;
		L_film_index22_V3 = index22_V3  ;
		L_film_index22_C = index22_C ;

		/*
		L_film_stillpix_count22 = stillpix_count22 ;
		L_film_stillpix_count22_H1 = stillpix_count22_H1 ;
		L_film_stillpix_count22_H2 = stillpix_count22_H2 ;
		L_film_stillpix_count22_H3 = stillpix_count22_H3 ;
		L_film_stillpix_count22_V1 = stillpix_count22_V1 ;
		L_film_stillpix_count22_V2 = stillpix_count22_V2 ;
		L_film_stillpix_count22_V3 = stillpix_count22_V3 ;
		L_film_stillpix_count22_C = stillpix_count22_C ;
		*/

		L_film_index = film_index ;
		L_force_weave_flag = force_weave_flag ;
		L_weave_prev = weave_prev ;
	  }
}
#endif


// chen 170522
#if 0

// noise measure fw start
// global parameters
void scalerVIP_rtnr_noise_measure(void)
{
	di_di_rtnr_nm_control_RBUS di_rtnr_nm_control_reg;
	di_di_rtnr_nm_hist1_0_RBUS di_rtnr_nm_hist1_0_reg;
	di_di_rtnr_nm_hist1_1_RBUS di_rtnr_nm_hist1_1_reg;
	di_di_rtnr_nm_hist1_2_RBUS di_rtnr_nm_hist1_2_reg;
	di_di_rtnr_nm_hist1_3_RBUS di_rtnr_nm_hist1_3_reg;
	di_di_rtnr_nm_hist1_4_RBUS di_rtnr_nm_hist1_4_reg;
	di_di_rtnr_nm_hist1_5_RBUS di_rtnr_nm_hist1_5_reg;
	di_di_rtnr_nm_hist1_6_RBUS di_rtnr_nm_hist1_6_reg;
	di_di_rtnr_nm_hist1_7_RBUS di_rtnr_nm_hist1_7_reg;
	di_di_rtnr_nm_hist1_8_RBUS di_rtnr_nm_hist1_8_reg;
	di_di_rtnr_nm_hist1_9_RBUS di_rtnr_nm_hist1_9_reg;
	di_di_rtnr_nm_hist1_10_RBUS di_rtnr_nm_hist1_10_reg;
	di_di_rtnr_nm_hist1_11_RBUS di_rtnr_nm_hist1_11_reg;
	di_di_rtnr_nm_hist1_12_RBUS di_rtnr_nm_hist1_12_reg;
	di_di_rtnr_nm_hist1_13_RBUS di_rtnr_nm_hist1_13_reg;
	di_di_rtnr_nm_hist1_14_RBUS di_rtnr_nm_hist1_14_reg;
	di_di_rtnr_nm_hist1_15_RBUS di_rtnr_nm_hist1_15_reg;

	di_di_rtnr_nm_hist2_0_RBUS di_rtnr_nm_hist2_0_reg;
	di_di_rtnr_nm_hist2_1_RBUS di_rtnr_nm_hist2_1_reg;
	di_di_rtnr_nm_hist2_2_RBUS di_rtnr_nm_hist2_2_reg;
	di_di_rtnr_nm_hist2_3_RBUS di_rtnr_nm_hist2_3_reg;
	di_di_rtnr_nm_hist2_4_RBUS di_rtnr_nm_hist2_4_reg;
	di_di_rtnr_nm_hist2_5_RBUS di_rtnr_nm_hist2_5_reg;
	di_di_rtnr_nm_hist2_6_RBUS di_rtnr_nm_hist2_6_reg;
	di_di_rtnr_nm_hist2_7_RBUS di_rtnr_nm_hist2_7_reg;
	di_di_rtnr_nm_hist2_8_RBUS di_rtnr_nm_hist2_8_reg;
	di_di_rtnr_nm_hist2_9_RBUS di_rtnr_nm_hist2_9_reg;
	di_di_rtnr_nm_hist2_10_RBUS di_rtnr_nm_hist2_10_reg;
	di_di_rtnr_nm_hist2_11_RBUS di_rtnr_nm_hist2_11_reg;
	di_di_rtnr_nm_hist2_12_RBUS di_rtnr_nm_hist2_12_reg;
	di_di_rtnr_nm_hist2_13_RBUS di_rtnr_nm_hist2_13_reg;
	di_di_rtnr_nm_hist2_14_RBUS di_rtnr_nm_hist2_14_reg;
	di_di_rtnr_nm_hist2_15_RBUS di_rtnr_nm_hist2_15_reg;

	di_di_rtnr_nm_hist2var_0_RBUS di_rtnr_nm_hist2var_0_reg;
	di_di_rtnr_nm_hist2var_1_RBUS di_rtnr_nm_hist2var_1_reg;
	di_di_rtnr_nm_hist2var_2_RBUS di_rtnr_nm_hist2var_2_reg;
	di_di_rtnr_nm_hist2var_3_RBUS di_rtnr_nm_hist2var_3_reg;
	di_di_rtnr_nm_hist2var_4_RBUS di_rtnr_nm_hist2var_4_reg;
	di_di_rtnr_nm_hist2var_5_RBUS di_rtnr_nm_hist2var_5_reg;
	di_di_rtnr_nm_hist2var_6_RBUS di_rtnr_nm_hist2var_6_reg;
	di_di_rtnr_nm_hist2var_7_RBUS di_rtnr_nm_hist2var_7_reg;
	di_di_rtnr_nm_hist2var_8_RBUS di_rtnr_nm_hist2var_8_reg;
	di_di_rtnr_nm_hist2var_9_RBUS di_rtnr_nm_hist2var_9_reg;
	di_di_rtnr_nm_hist2var_10_RBUS di_rtnr_nm_hist2var_10_reg;
	di_di_rtnr_nm_hist2var_11_RBUS di_rtnr_nm_hist2var_11_reg;
	di_di_rtnr_nm_hist2var_12_RBUS di_rtnr_nm_hist2var_12_reg;
	di_di_rtnr_nm_hist2var_13_RBUS di_rtnr_nm_hist2var_13_reg;
	di_di_rtnr_nm_hist2var_14_RBUS di_rtnr_nm_hist2var_14_reg;
	di_di_rtnr_nm_hist2var_15_RBUS di_rtnr_nm_hist2var_15_reg;

	di_di_rtnr_nm_infor1_RBUS di_rtnr_nm_infor1_reg;
	di_di_rtnr_nm_infor2_RBUS di_rtnr_nm_infor2_reg;	// var sum

	di_im_di_film_motion_sum_th_RBUS im_di_film_motion_sum_th_reg;	// dummy control
	// parameters

	unsigned int VarSum,Sum;
	unsigned int NoiseLevel = 0xff;	// default value
	unsigned int hist1[16];
	unsigned int hist2[16];
	unsigned int hist2var[16];
	unsigned char ref_Y_en,Yref=128,Ydiff=128;
	unsigned char nm_start = 0;
	unsigned char NM_control = 1;
	unsigned char detReliable = 0;
	unsigned char smplgBit = 0;
	//float NoiseLevel_tmp = 0;

	di_rtnr_nm_infor1_reg.regValue = rtd_inl(DI_DI_RTNR_NM_INFOR1_reg);
	di_rtnr_nm_infor2_reg.regValue = rtd_inl(DI_DI_RTNR_NM_INFOR2_reg);

	di_rtnr_nm_control_reg.regValue = rtd_inl(DI_DI_RTNR_NM_CONTROL_reg);
	di_rtnr_nm_hist1_0_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_0_reg);
	di_rtnr_nm_hist1_1_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_1_reg);
	di_rtnr_nm_hist1_2_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_2_reg);
	di_rtnr_nm_hist1_3_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_3_reg);
	di_rtnr_nm_hist1_4_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_4_reg);
	di_rtnr_nm_hist1_5_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_5_reg);
	di_rtnr_nm_hist1_6_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_6_reg);
	di_rtnr_nm_hist1_7_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_7_reg);
	di_rtnr_nm_hist1_8_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_8_reg);
	di_rtnr_nm_hist1_9_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_9_reg);
	di_rtnr_nm_hist1_10_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_10_reg);
	di_rtnr_nm_hist1_11_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_11_reg);
	di_rtnr_nm_hist1_12_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_12_reg);
	di_rtnr_nm_hist1_13_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_13_reg);
	di_rtnr_nm_hist1_14_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_14_reg);
	di_rtnr_nm_hist1_15_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST1_15_reg);

	di_rtnr_nm_hist2_0_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_0_reg);
	di_rtnr_nm_hist2_1_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_1_reg);
	di_rtnr_nm_hist2_2_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_2_reg);
	di_rtnr_nm_hist2_3_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_3_reg);
	di_rtnr_nm_hist2_4_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_4_reg);
	di_rtnr_nm_hist2_5_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_5_reg);
	di_rtnr_nm_hist2_6_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_6_reg);
	di_rtnr_nm_hist2_7_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_7_reg);
	di_rtnr_nm_hist2_8_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_8_reg);
	di_rtnr_nm_hist2_9_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_9_reg);
	di_rtnr_nm_hist2_10_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_10_reg);
	di_rtnr_nm_hist2_11_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_11_reg);
	di_rtnr_nm_hist2_12_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_12_reg);
	di_rtnr_nm_hist2_13_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_13_reg);
	di_rtnr_nm_hist2_14_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_14_reg);
	di_rtnr_nm_hist2_15_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2_15_reg);

	di_rtnr_nm_hist2var_0_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_0_reg);
	di_rtnr_nm_hist2var_1_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_1_reg);
	di_rtnr_nm_hist2var_2_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_2_reg);
	di_rtnr_nm_hist2var_3_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_3_reg);
	di_rtnr_nm_hist2var_4_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_4_reg);
	di_rtnr_nm_hist2var_5_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_5_reg);
	di_rtnr_nm_hist2var_6_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_6_reg);
	di_rtnr_nm_hist2var_7_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_7_reg);
	di_rtnr_nm_hist2var_8_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_8_reg);
	di_rtnr_nm_hist2var_9_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_9_reg);
	di_rtnr_nm_hist2var_10_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_10_reg);
	di_rtnr_nm_hist2var_11_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_11_reg);
	di_rtnr_nm_hist2var_12_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_12_reg);
	di_rtnr_nm_hist2var_13_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_13_reg);
	di_rtnr_nm_hist2var_14_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_14_reg);
	di_rtnr_nm_hist2var_15_reg.regValue = rtd_inl(DI_DI_RTNR_NM_HIST2VAR_15_reg);
	im_di_film_motion_sum_th_reg.regValue = rtd_inl(DI_IM_DI_FILM_MOTION_SUM_TH_reg);

	Sum = di_rtnr_nm_infor1_reg.nm_sumvarnum;
	VarSum = di_rtnr_nm_infor2_reg.nm_sumvar;

	ref_Y_en = di_rtnr_nm_control_reg.nm_ref_y_en;
	Yref = di_rtnr_nm_control_reg.nm_fw_yref;
	Ydiff = di_rtnr_nm_control_reg.nm_fw_ydiff;
	nm_start = di_rtnr_nm_control_reg.nm_start;
	smplgBit = di_rtnr_nm_control_reg.nm_samplingbit;
	NM_control = im_di_film_motion_sum_th_reg.nm_main_enable;

	if(nm_start == 0 && NM_control)
	{
		hist1[0] = di_rtnr_nm_hist1_0_reg.hist1_0;
		hist1[1] = di_rtnr_nm_hist1_1_reg.hist1_1;
		hist1[2] = di_rtnr_nm_hist1_2_reg.hist1_2;
		hist1[3] = di_rtnr_nm_hist1_3_reg.hist1_3;
		hist1[4] = di_rtnr_nm_hist1_4_reg.hist1_4;
		hist1[5] = di_rtnr_nm_hist1_5_reg.hist1_5;
		hist1[6] = di_rtnr_nm_hist1_6_reg.hist1_6;
		hist1[7] = di_rtnr_nm_hist1_7_reg.hist1_7;
		hist1[8] = di_rtnr_nm_hist1_8_reg.hist1_8;
		hist1[9] = di_rtnr_nm_hist1_9_reg.hist1_9;
		hist1[10] = di_rtnr_nm_hist1_10_reg.hist1_10;
		hist1[11] = di_rtnr_nm_hist1_11_reg.hist1_11;
		hist1[12] = di_rtnr_nm_hist1_12_reg.hist1_12;
		hist1[13] = di_rtnr_nm_hist1_13_reg.hist1_13;
		hist1[14] = di_rtnr_nm_hist1_14_reg.hist1_14;
		hist1[15] = di_rtnr_nm_hist1_15_reg.hist1_15;

		hist2[0] = di_rtnr_nm_hist2_0_reg.hist2_0;
		hist2[1] = di_rtnr_nm_hist2_1_reg.hist2_1;
		hist2[2] = di_rtnr_nm_hist2_2_reg.hist2_2;
		hist2[3] = di_rtnr_nm_hist2_3_reg.hist2_3;
		hist2[4] = di_rtnr_nm_hist2_4_reg.hist2_4;
		hist2[5] = di_rtnr_nm_hist2_5_reg.hist2_5;
		hist2[6] = di_rtnr_nm_hist2_6_reg.hist2_6;
		hist2[7] = di_rtnr_nm_hist2_7_reg.hist2_7;
		hist2[8] = di_rtnr_nm_hist2_8_reg.hist2_8;
		hist2[9] = di_rtnr_nm_hist2_9_reg.hist2_9;
		hist2[10] = di_rtnr_nm_hist2_10_reg.hist2_10;
		hist2[11] = di_rtnr_nm_hist2_11_reg.hist2_11;
		hist2[12] = di_rtnr_nm_hist2_12_reg.hist2_12;
		hist2[13] = di_rtnr_nm_hist2_13_reg.hist2_13;
		hist2[14] = di_rtnr_nm_hist2_14_reg.hist2_14;
		hist2[15] = di_rtnr_nm_hist2_15_reg.hist2_15;

		hist2var[0] = di_rtnr_nm_hist2var_0_reg.hist2var_0;
		hist2var[1] = di_rtnr_nm_hist2var_1_reg.hist2var_1;
		hist2var[2] = di_rtnr_nm_hist2var_2_reg.hist2var_2;
		hist2var[3] = di_rtnr_nm_hist2var_3_reg.hist2var_3;
		hist2var[4] = di_rtnr_nm_hist2var_4_reg.hist2var_4;
		hist2var[5] = di_rtnr_nm_hist2var_5_reg.hist2var_5;
		hist2var[6] = di_rtnr_nm_hist2var_6_reg.hist2var_6;
		hist2var[7] = di_rtnr_nm_hist2var_7_reg.hist2var_7;
		hist2var[8] = di_rtnr_nm_hist2var_8_reg.hist2var_8;
		hist2var[9] = di_rtnr_nm_hist2var_9_reg.hist2var_9;
		hist2var[10] = di_rtnr_nm_hist2var_10_reg.hist2var_10;
		hist2var[11] = di_rtnr_nm_hist2var_11_reg.hist2var_11;
		hist2var[12] = di_rtnr_nm_hist2var_12_reg.hist2var_12;
		hist2var[13] = di_rtnr_nm_hist2var_13_reg.hist2var_13;
		hist2var[14] = di_rtnr_nm_hist2var_14_reg.hist2var_14;
		hist2var[15] = di_rtnr_nm_hist2var_15_reg.hist2var_15;

		#if 0
		unsigned char k;
		unsigned char vc_rtnr_NM_MaxBinRatio=16;	//if(Hist2[k]!=0 && Hist2[k]>=(maxbin2*reg_vc_rtnr_NM_MaxBinRatio)>>5)
		unsigned char vc_rtnr_NM_BinRatio =16; //if(((sumbin*reg_vc_rtnr_NM_BinRatio)>>3) > maxbin1) // leave criterion
		unsigned char vc_rtnr_NM_VarRatio=32;  // if(((min2_temp*reg_vc_rtnr_NM_VarRatio)>>5)>min_temp)
		unsigned char vc_rtnr_NM_FW_Yref=Yref,vc_rtnr_NM_FW_Ydiff=Ydiff;
		unsigned char index=0;
		unsigned int min_temp=0xffff;
		unsigned int min2_temp=0xffff;
		unsigned int maxbin2=0,maxbin1=0,sumbin=0;
		unsigned char shiftbit = 4;
		if(ref_Y_en)
		{
			for(k=0;k<16;k++)
			{
				sumbin += hist2[k];
				maxbin2 = (maxbin2>hist2[k])? maxbin2:hist2[k];//  max(maxbin2,hist2[k]);
				maxbin1 = (maxbin1>hist1[k])? maxbin1:hist1[k];// max(maxbin1,hist1[k]);
			}
			switch (Ydiff)
			{
				case 128:
					shiftbit = 4;
					break;
				case 64:
					shiftbit = 3;
					break;
				case 32:
					shiftbit = 2;
					break;
				case 16:
					shiftbit = 1;
					break;
				case 8:
					shiftbit = 0;
					break;
				default:
					shiftbit = 4;
					break;
			}

			//sumbin = sumbin>>(*shiftbit-1);
			for(k=0;k<16;k++)
			{
				if(hist2[k]!=0 && hist2[k]>=(maxbin2*vc_rtnr_NM_MaxBinRatio)>>5)
				{
					if((hist2var[k]/hist2[k]) <= min2_temp)
					{
						if((hist2var[k]/hist2[k]) <= min_temp)
						{
							min2_temp = min_temp;
							min_temp = hist2var[k]/hist2[k];
							index = k;
						}
						else
						{
							min2_temp = hist2var[k]/hist2[k];
						}
					}
				}
			}

			if(((sumbin*vc_rtnr_NM_BinRatio)>>3) > maxbin1) // leave criterion
			{
				if(((min2_temp*vc_rtnr_NM_VarRatio)>>5)>=min_temp)
				{
					if(shiftbit>=1)
					{
						vc_rtnr_NM_FW_Yref = vc_rtnr_NM_FW_Yref - vc_rtnr_NM_FW_Ydiff+ index*(1<<(shiftbit));
						vc_rtnr_NM_FW_Ydiff = vc_rtnr_NM_FW_Ydiff>>1;
					}
				}
			}
			else
			{
				vc_rtnr_NM_FW_Yref = 128;
				vc_rtnr_NM_FW_Ydiff = 128;
			}
			if(vc_rtnr_NM_FW_Yref - vc_rtnr_NM_FW_Ydiff <=0)
				vc_rtnr_NM_FW_Yref = vc_rtnr_NM_FW_Ydiff;
			else if(vc_rtnr_NM_FW_Yref + vc_rtnr_NM_FW_Ydiff>=255)
				vc_rtnr_NM_FW_Yref = 256 - vc_rtnr_NM_FW_Ydiff;

			// write back
			di_rtnr_nm_control_reg.nm_fw_yref = vc_rtnr_NM_FW_Yref;
			di_rtnr_nm_control_reg.nm_fw_ydiff = vc_rtnr_NM_FW_Ydiff;
		}
		#endif

		// detection reliable ->    0        1        2         3
		//						25% - 50% - 75%

		if(Sum>(((Smart_Integration_Status.DI_WIDTH*Smart_Integration_Status.DI_HEIGHT*5)>>3)/(13+smplgBit)))
			detReliable = 3;
		else if(Sum>(((Smart_Integration_Status.DI_WIDTH*Smart_Integration_Status.DI_HEIGHT*3)>>3)/(13+smplgBit)))
			detReliable = 2;
		else if(Sum>(((Smart_Integration_Status.DI_WIDTH*Smart_Integration_Status.DI_HEIGHT*1)>>3)/(13+smplgBit)))
			detReliable = 1;
		else
			detReliable = 0;

		//int tmp = (VarSum*4 / (Sum+1)) - 4;//NoiseLevel=uint32, always >0, but NoiseLevel is not>15, so...
		int tmp = VarSum / (Sum+1); // jimmy.lin 20140627: return to simple average, raise precision by adjusting thresholds
		if(tmp>15)
			NoiseLevel = 15;
		else if(tmp<0)
			NoiseLevel = 0;
		else
			NoiseLevel = tmp;

		//di_rtnr_nm_infor1_reg.nm_noiselevel = NoiseLevel;
		di_rtnr_nm_control_reg.nm_start = 1;

		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_CUR = (unsigned char)NoiseLevel;
		Smart_Integration_Status.SCREEN_NOISE_STATUS.CONTENT_SPATIAL_NOISE.TMP_NOISE_LEVEL_RELIABLE_CUR = detReliable;

		//////////////////////////////////// APPLY ////////////////////////////////////
		//scaler_rtd_outl(DI_DI_RTNR_NM_INFOR1_reg, di_rtnr_nm_infor1_reg.regValue);
		scaler_rtd_outl(DI_DI_RTNR_NM_CONTROL_reg, di_rtnr_nm_control_reg.regValue);
	}
	else // turn on NM_control
	{
		im_di_film_motion_sum_th_reg.nm_main_enable = 1;
		scaler_rtd_outl(DI_IM_DI_FILM_MOTION_SUM_TH_reg, im_di_film_motion_sum_th_reg.regValue);
	}
//#endif
}
// noise measure fw end

#endif
// end chen 170522

void scalerVIP_DI_MCNR_RTNR_bottom_fresh_line_patch(unsigned char enable)
{
	di_im_di_weave_window_control_RBUS im_di_weave_window_control_reg;
	di_im_di_weave_window_5_start_RBUS im_di_weave_window_5_start_reg;
	di_im_di_weave_window_5_end_RBUS im_di_weave_window_5_end_reg;

	unsigned char motionl_level = scalerVIP_DI_MiddleWare_GetStatus(STATUS_MOTION);
	static unsigned int pre_w = 0, pre_h = 0;
	static unsigned int s_count = 0;

	im_di_weave_window_control_reg.regValue = rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg);

	if (Smart_Integration_Status.DI_WIDTH != pre_w || Smart_Integration_Status.DI_HEIGHT != pre_h)
		s_count = 0;
	s_count++;

	pre_w = Smart_Integration_Status.DI_WIDTH;
	pre_h = Smart_Integration_Status.DI_HEIGHT;

	if (enable && Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE) && (SmartPic_clue->RTNR_MAD_count_Y_avg_ratio > 950|| motionl_level<=2)&& s_count > 5) {
		im_di_weave_window_control_reg.fixedwindow5 = 1;
		im_di_weave_window_5_start_reg.xstart_77_65 = 1;
		im_di_weave_window_5_start_reg.ystart_77_65 = Smart_Integration_Status.DI_HEIGHT;

		im_di_weave_window_5_end_reg.xend_77_65 = Smart_Integration_Status.DI_WIDTH;
		im_di_weave_window_5_end_reg.yend_77_65 = Smart_Integration_Status.DI_HEIGHT;

		rtd_outl(DI_IM_DI_WEAVE_WINDOW_5_START_reg, im_di_weave_window_5_start_reg.regValue);
		rtd_outl(DI_IM_DI_WEAVE_WINDOW_5_END_reg, im_di_weave_window_5_end_reg.regValue);
		s_count = 100;
	} else {
		im_di_weave_window_control_reg.fixedwindow5 = 0;
	}

	rtd_outl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg, im_di_weave_window_control_reg.regValue);

}

void scalerVIP_Get_DI_Width_Height(unsigned short *pWidth, unsigned short *pHeight)
{
	*pWidth = Smart_Integration_Status.DI_WIDTH;
	*pHeight = Smart_Integration_Status.DI_HEIGHT;
}

void scalerDI_Set_DI_SMD(unsigned char DI_SMD_enable)
{
	di_di_smd_control_candidatelist_RBUS di_smd_control_candiate_reg;
	di_smd_control_candiate_reg.regValue = rtd_inl(DI_DI_SMD_Control_CandidateList_reg);
	di_smd_control_candiate_reg.smd_en = DI_SMD_enable;
	rtd_outl(DI_DI_SMD_Control_CandidateList_reg, di_smd_control_candiate_reg.regValue);
}

