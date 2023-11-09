#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/include/PQLAPI.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"

#include "memc_isr/PQL/FRC_glb_Context.h"
#include "memc_isr/PQL/FRC_decPhT_generate.h"
#include "memc_isr/PQL/PQLContext.h"
#include <memc_isr/scalerMEMC.h>

#include <rtd_log/rtd_module_log.h>

#include <memc_isr/include/memc_lib.h>
//#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include "memc_reg_def.h"
#include <tvscalercontrol/panel/panelapi.h>

int LowDelay_regen_chk = 0;
extern int LowDelayGen_cnt;
extern unsigned char u8_MEMCMode;
extern unsigned char VR360_en;
extern unsigned char lowdelay_state;
extern unsigned char lowdelay_state_pre;
extern unsigned char lowdelay_state_chg_flg;
extern unsigned char lowdelay_state_chg_cnt;

extern unsigned char Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(VOID);


extern VOID MEMC_LibBlueScreen(unsigned char is_BlueScreen);
//////////////////////////////////////////////////////////////////////////
static Ph_Dec_Pack_Info  phT_pack_bit_len =
{
	{1, 2, 6, 6, 2, 3, 3, 6, 1, 2},	//unsigned char u8_msb[MSB_NAMES_NUM];
	{4, 1, 3, 3, 1, 1, 3, 3, 7, 2, 3},	//unsigned char u8_lsb[LSB_NAMES_NUM];
	3								//unsigned char u8_inPhaseLen;
};
static Ph_Dec_Init_Info  phT_DecInitInfo[_FRC_CADENCE_NUM_] =
{
#if 1  // k4lp
	{4, 3, 4, 3, 0,  1, 3},	//_CAD_VIDEO,	//0
	{3, 2, 3, 2, 1, 12, 2},	//_CAD_22,		//1
	{3, 2, 3, 2, 1,  5, 2},	//_CAD_32,		//2
	{3, 2, 3, 2, 0, 24, 2},	//_CAD_32322,		//3
	{3, 2, 3, 2, 6, 20, 2},	//_CAD_334,		//4
	{3, 2, 3, 2, 8, 24, 2},	//_CAD_22224,		//5
	{3, 2, 3, 2, 5, 20, 2},	//_CAD_2224,		//6
	{3, 2, 3, 2, 4, 20, 2},	//_CAD_3223,		//7
	{3, 2, 3, 2, 4, 10, 2},	//_CAD_55,		//8
	{3, 2, 3, 2, 4, 12, 2},	//_CAD_66,		//9
	{3, 2, 3, 2, 2,  8, 2},	//_CAD_44,		//0xA
	{3, 2, 3, 2, 0, 10, 2},	//_CAD_1112,		//0xB
	{3, 2, 3, 2, 1, 12, 2},	//_CAD_11112,		//0xC
	{3, 2, 3, 2, 1, 10, 2},	//_CAD_122,		//0xD
	{3, 2, 3, 2, 5, 25, 2},	//_CAD_11i23,		//0xE
	{3, 2, 3, 2, 1, 12, 2},	//_CAD_321,		//0xF	
	{3, 2, 3, 2, 5, 25, 2},	//_CAD_1225_T1,		//0x10
	{3, 2, 3, 2, 5, 25, 2},	//_CAD_1225_T2,		//0x11	
	{3, 2, 3, 2, 1, 10, 2}  //_CAD_12214,       //0x12
#else
	{3, 2, 0,   1, 2},	//_CAD_VIDEO,	//0
	{3, 2, 1, 12, 2},	//_CAD_22,		//1
	{3, 2, 1,   5, 2},	//_CAD_32,		//2
	{3, 2, 0, 24, 2},	//_CAD_32322,		//3
	{3, 2, 6, 20, 2},	//_CAD_334,		//4
	{3, 2, 8, 24, 2},	//_CAD_22224,		//5
	{3, 2, 5, 20, 2},	//_CAD_2224,		//6
	{3, 2, 4, 20, 2},	//_CAD_3223,		//7
	{3, 2, 4, 10, 2},	//_CAD_55,		//8
	{3, 2, 4, 12, 2},	//_CAD_66,		//9
	{3, 2, 2,   8, 2},	//_CAD_44,		//0xA
	{3, 2, 0, 10, 2},	//_CAD_1112,		//0xB
	{3, 2, 1, 12,2},	//_CAD_11112,		//0xC
	{3, 2, 1, 10, 2},	//_CAD_122,		//0xD
	{3, 2, 5, 25, 2}	//_CAD_11i23,		//0xE
#endif
};

//static unsigned char             pht_record_addr[INI_CAD_END + 1];
//static unsigned int            phTable_WrtLSB_GLD[128];
//static unsigned int            phTable_WrtMSB_GLD[128];

Ph_Dec_Info       phTable_DecInfoGen[256] = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
unsigned int            phTable_WrtLSB[256] = {0};
unsigned int            phTable_WrtMSB[256] = {0};
Film_Wrt_Info     phTable_WrtInfoGen[64] = {{0, 0, 0, 0, 0}};
Film_Wrt_Info     phTable_WrtInfoGen_Amend[64] = {{0, 0, 0, 0, 0}};
Film_Wrt_Info     phTable_WrtInfoGen_vd_22_32[64] = {{0, 0, 0, 0, 0}};

///////   1. HW 64 bit & SW member pack & unpack function.    ///////////////////////
//////////////////////////////////////////////////////////////////////////////

VOID   FRC_phTable_Init_64bitConfig()
{
	phT_pack_bit_len.u8_msb[MSB_OUT_3D_ENABLE]           = 1;
	phT_pack_bit_len.u8_msb[MSB_FILM_MODE]               = 2;
	phT_pack_bit_len.u8_msb[MSB_OUT_PHASE]               = 6;
	phT_pack_bit_len.u8_msb[MSB_FILM_PHASE]              = 6;
	phT_pack_bit_len.u8_msb[MSB_ME2_PPFV_BUF_IDX]        = 2;
	phT_pack_bit_len.u8_msb[MSB_ME2_IBME_P_INDEX_OFFSET] = 3;
	phT_pack_bit_len.u8_msb[MSB_ME2_IBME_I_INDEX_OFFSET] = 3;
	phT_pack_bit_len.u8_msb[MSB_ME2_PHASE]               = 6;
	phT_pack_bit_len.u8_msb[MSB_ME2_FIRST_PHASE]         = 1;
	phT_pack_bit_len.u8_msb[MSB_CRTC_MC_HF_INDEX]        = 2;

	phT_pack_bit_len.u8_lsb[LSB_CRTC_MC_LF_INDEX]        = 4;
	phT_pack_bit_len.u8_lsb[LSB_ME_LAST_PHASE]           = 1;
	phT_pack_bit_len.u8_lsb[LSB_ME_DTS_P_INDEX]          = 3;
	phT_pack_bit_len.u8_lsb[LSB_ME_DTS_I_INDEX]          = 3;
	phT_pack_bit_len.u8_lsb[LSB_MC_MODE_HF_UPDATE]       = 1;
	phT_pack_bit_len.u8_lsb[LSB_MC_MODE_LF_UPDATE]       = 1;
	phT_pack_bit_len.u8_lsb[LSB_MC_CTS_P_INDEX]          = 3;
	phT_pack_bit_len.u8_lsb[LSB_MC_CTS_I_INDEX]          = 3;
	phT_pack_bit_len.u8_lsb[LSB_MC_PHASE]                = 7;
	phT_pack_bit_len.u8_lsb[LSB_MC_HF_INDEX]             = 2;
	phT_pack_bit_len.u8_lsb[LSB_MC_LF_INDEX]             = 4;
	phT_pack_bit_len.u8_inPhaseLen                       = 3;
}
VOID FRC_phTable_Init_1n2m_video_22_32()
{
	unsigned short u16_k;
	for(u16_k = 0; u16_k < 256; u16_k ++)
	{
		phTable_DecInfoGen[u16_k].out_phase  = 0;
		phTable_DecInfoGen[u16_k].film_mode  = 0;
		phTable_DecInfoGen[u16_k].film_phase = 0;
		phTable_DecInfoGen[u16_k].in_phase   = 0;

		phTable_DecInfoGen[u16_k].me2_phase        = 0;
		phTable_DecInfoGen[u16_k].mc_phase        = 0;
		phTable_DecInfoGen[u16_k].frac_phase_nume = 0;

		phTable_DecInfoGen[u16_k].me1_i_offset      = 0;
		phTable_DecInfoGen[u16_k].me1_p_offset      = 0;
		phTable_DecInfoGen[u16_k].me2_i_offset  = 0;
		phTable_DecInfoGen[u16_k].me2_p_offset  = 0;
#if 1  // k4lp
		phTable_DecInfoGen[u16_k].mc_i_offset	    = 0;
		phTable_DecInfoGen[u16_k].mc_p_offset	    = 0;
#endif
		phTable_DecInfoGen[u16_k].me1_last    = 0;
		phTable_DecInfoGen[u16_k].ppfv_offset   = 0;

		phTable_DecInfoGen[u16_k].outPhase_new   = 0;
		phTable_DecInfoGen[u16_k].inPhase_new   = 0;
		phTable_DecInfoGen[u16_k].cadSeq_new    = 0;

		phTable_DecInfoGen[u16_k].me1_wrt_idx_oft   = 0;
#if 1  // k4lp
 		phTable_DecInfoGen[u16_k].mc_wrt_idx_oft    = 0;
#else
		phTable_DecInfoGen[u16_k].me2_wrt_idx_oft   = 0;
#endif
		phTable_WrtLSB[u16_k]     = 0;
		phTable_WrtMSB[u16_k]     = 0;
	}

	for (u16_k = 0; u16_k < 64; u16_k ++)
	{
		phTable_WrtInfoGen[u16_k].me1_in_idx     = 0;
		phTable_WrtInfoGen[u16_k].me2_in_idx     = 0;
		phTable_WrtInfoGen[u16_k].mc_in_idx      = 0;
		phTable_WrtInfoGen[u16_k].logo_en        = 0;
		phTable_WrtInfoGen[u16_k].mc_finalPhase  = 0;
	}
}

VOID   phaseTable_unpack(unsigned int ui_pack_LSB, unsigned int ui_pack_MSB, Ph_Dec_Info *pPh_DecInfo)
{
	unsigned char  u8_k, u8_endBit;

	u8_endBit = 31;
	for (u8_k = 0; u8_k < MSB_FILM_MODE; u8_k ++)
	{
		u8_endBit = u8_endBit - phT_pack_bit_len.u8_msb[u8_k];
	}
	pPh_DecInfo->film_mode  = _32bit_unpack(&ui_pack_MSB, u8_endBit - phT_pack_bit_len.u8_msb[MSB_FILM_MODE]+1, u8_endBit);
	//
	u8_endBit = u8_endBit - phT_pack_bit_len.u8_msb[MSB_FILM_MODE];
	pPh_DecInfo->out_phase  = _32bit_unpack(&ui_pack_MSB, u8_endBit - phT_pack_bit_len.u8_msb[MSB_OUT_PHASE]+1, u8_endBit);
	//
	u8_endBit = u8_endBit - phT_pack_bit_len.u8_msb[MSB_OUT_PHASE];
	pPh_DecInfo->film_phase = _32bit_unpack(&ui_pack_MSB, u8_endBit - phT_pack_bit_len.u8_msb[MSB_FILM_PHASE]+1, u8_endBit);
	//
	u8_endBit = u8_endBit - phT_pack_bit_len.u8_msb[MSB_FILM_PHASE];
	pPh_DecInfo->ppfv_offset = _32bit_unpack(&ui_pack_MSB, u8_endBit - phT_pack_bit_len.u8_msb[MSB_ME2_PPFV_BUF_IDX]+1, u8_endBit);

#if 1  // k4lp
	//
	u8_endBit = u8_endBit - phT_pack_bit_len.u8_msb[MSB_ME2_PPFV_BUF_IDX];
	pPh_DecInfo->me2_p_offset   = _32bit_unpack(&ui_pack_MSB, u8_endBit - phT_pack_bit_len.u8_msb[MSB_ME2_IBME_P_INDEX_OFFSET]+1, u8_endBit);

	//
	u8_endBit = u8_endBit - phT_pack_bit_len.u8_msb[MSB_ME2_IBME_P_INDEX_OFFSET];
	pPh_DecInfo->me2_i_offset	= _32bit_unpack(&ui_pack_MSB, u8_endBit - phT_pack_bit_len.u8_msb[MSB_ME2_IBME_I_INDEX_OFFSET]+1, u8_endBit);
#endif

	//
	u8_endBit = 31;
	for (u8_k = 0; u8_k < MSB_ME2_PHASE; u8_k ++)
	{
		u8_endBit = u8_endBit - phT_pack_bit_len.u8_msb[u8_k];
	}
	pPh_DecInfo->me2_phase   = _32bit_unpack(&ui_pack_MSB, u8_endBit - phT_pack_bit_len.u8_msb[MSB_ME2_PHASE]+1, u8_endBit);

//
	u8_endBit = 31;
	for (u8_k = 0; u8_k < LSB_ME_LAST_PHASE; u8_k ++)
	{
		u8_endBit = u8_endBit - phT_pack_bit_len.u8_lsb[u8_k];
	}
	pPh_DecInfo->me1_last   = _32bit_unpack(&ui_pack_LSB, u8_endBit - phT_pack_bit_len.u8_lsb[LSB_ME_LAST_PHASE]+1, u8_endBit);
	u8_endBit = u8_endBit - phT_pack_bit_len.u8_lsb[LSB_ME_LAST_PHASE];
	pPh_DecInfo->me1_p_offset   = _32bit_unpack(&ui_pack_LSB, u8_endBit - phT_pack_bit_len.u8_lsb[LSB_ME_DTS_P_INDEX]+1, u8_endBit);
	u8_endBit = u8_endBit - phT_pack_bit_len.u8_lsb[LSB_ME_DTS_P_INDEX];
	pPh_DecInfo->me1_i_offset   = _32bit_unpack(&ui_pack_LSB, u8_endBit - phT_pack_bit_len.u8_lsb[LSB_ME_DTS_I_INDEX]+1, u8_endBit);

	u8_endBit = 31;
	for (u8_k = 0; u8_k < LSB_MC_PHASE; u8_k ++)
	{
		u8_endBit = u8_endBit - phT_pack_bit_len.u8_lsb[u8_k];
	}
	pPh_DecInfo->mc_phase   = _32bit_unpack(&ui_pack_LSB, u8_endBit - phT_pack_bit_len.u8_lsb[LSB_MC_PHASE]+1, u8_endBit);

	//
	u8_endBit = 31;
	for (u8_k = 0; u8_k < LSB_MC_CTS_P_INDEX; u8_k ++)
	{
		u8_endBit = u8_endBit - phT_pack_bit_len.u8_lsb[u8_k];
	}
#if 1  // k4lp
	pPh_DecInfo->mc_p_offset   = _32bit_unpack(&ui_pack_LSB, u8_endBit - phT_pack_bit_len.u8_lsb[LSB_MC_CTS_P_INDEX]+1, u8_endBit);
#else
	pPh_DecInfo->me2_p_offset   = _32bit_unpack(&ui_pack_LSB, u8_endBit - phT_pack_bit_len.u8_lsb[LSB_MC_CTS_P_INDEX]+1, u8_endBit);
#endif

	//
	u8_endBit = u8_endBit - phT_pack_bit_len.u8_lsb[LSB_MC_CTS_P_INDEX];
#if 1  // k4lp
	pPh_DecInfo->mc_i_offset   = _32bit_unpack(&ui_pack_LSB, u8_endBit - phT_pack_bit_len.u8_lsb[LSB_MC_CTS_I_INDEX]+1, u8_endBit);
#else
	pPh_DecInfo->me2_i_offset   = _32bit_unpack(&ui_pack_LSB, u8_endBit - phT_pack_bit_len.u8_lsb[LSB_MC_CTS_I_INDEX]+1, u8_endBit);
#endif
}

VOID   phaseTable_pack(Ph_Dec_Pack_Info *pPackInfo, unsigned int *pU32_pack_LSB, unsigned int *pU32_pack_MSB)
{
	unsigned char u8_k, u8_pack_stBit;

	u8_pack_stBit = 0;  	*pU32_pack_LSB = 0;
	for (u8_k = 0; u8_k < LSB_NAMES_NUM; u8_k ++)
	{
		unsigned char u8_lsb_idx = (LSB_NAMES_NUM - 1) - u8_k;
		_32bit_pack(pU32_pack_LSB, u8_pack_stBit, (u8_pack_stBit + phT_pack_bit_len.u8_lsb[u8_lsb_idx] - 1), (unsigned int)pPackInfo->u8_lsb[u8_lsb_idx]);
		u8_pack_stBit = u8_pack_stBit + phT_pack_bit_len.u8_lsb[u8_lsb_idx];
	}

	u8_pack_stBit = 0;  	*pU32_pack_MSB = 0;
	for (u8_k = 0; u8_k < MSB_NAMES_NUM; u8_k ++)
	{
		unsigned char u8_msb_idx = (MSB_NAMES_NUM - 1) - u8_k;
		_32bit_pack(pU32_pack_MSB, u8_pack_stBit, (u8_pack_stBit + phT_pack_bit_len.u8_msb[u8_msb_idx] - 1), (unsigned int)pPackInfo->u8_msb[u8_msb_idx]);
		u8_pack_stBit = u8_pack_stBit + phT_pack_bit_len.u8_msb[u8_msb_idx];
	}
}

VOID   _32bit_pack(unsigned int *pui_packWord, unsigned char u8_stBit, unsigned char u8_endBit, unsigned int uiVal)
{
	if (u8_endBit < u8_stBit)
	{
		rtd_pr_memc_notice("error pack order in phase-table-gen.\r\n");
		return;
	}
	else if(u8_endBit > 31){
		return;
	}
	else
	{
		unsigned int ui_MASK = 0;

		ui_MASK = (u8_endBit == 31)? 0xFFFFFFFF : ((1 << (u8_endBit+1)) - 1);
		ui_MASK = (ui_MASK >> u8_stBit) << u8_stBit;

		(*pui_packWord) = (*pui_packWord) & (0xFFFFFFFF - ui_MASK);
		(*pui_packWord) = (*pui_packWord) | ((uiVal << u8_stBit) & ui_MASK);
	}
}
unsigned int _32bit_unpack(unsigned int *pui_packWord, unsigned char u8_stBit, unsigned char u8_endBit)
{
	if (u8_endBit < u8_stBit)
	{
		rtd_pr_memc_notice("error unpack order in phase-table-gen\r\n");
		return 0;
	}
	else if(u8_endBit > 31){
		return 0;
	}
	else
	{
		unsigned int ui_MASK;
		ui_MASK = (u8_endBit == 31)? 0xFFFFFFFF : ((1 << (u8_endBit+1)) - 1);//for Coverity issue
		ui_MASK = (ui_MASK >> u8_stBit) << u8_stBit;
		return (((*pui_packWord) & ui_MASK) >> u8_stBit);
	}
}

////////     2. control in_wrt_table generate and pack.     ////////////////////////
VOID   FRC_phTable_WrtStep_PCmode_Gen()
{
	phTable_WrtInfoGen[0].mc_finalPhase  = 0;
	phTable_WrtInfoGen[0].logo_en        = 1;
	phTable_WrtInfoGen[0].me1_in_idx     = 0;
	phTable_WrtInfoGen[0].me2_in_idx     = 0;
	phTable_WrtInfoGen[0].mc_in_idx      = 0;
}
VOID FRC_phTable_WrtStep_VideoMode_Gen(FRC_CADENCE cadInfo)
{
	unsigned char sig_fmCnt = 0, u8_fmPh;
	unsigned char u1_me1_wrt_noMove;
#if 1
	unsigned char u1_mc_wrt_noMove;
#else
	unsigned char u1_me2_wrt_noMove;
#endif
	unsigned int MC_8_buffer_en = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 0, &MC_8_buffer_en);

	for (u8_fmPh = 0; u8_fmPh < cadInfo.cad_outLen; u8_fmPh++)
	{
		unsigned char cad_seqFlag   = cadence_seqFlag(cadInfo, u8_fmPh);

		// whether current film phase is firsts phase.
		phTable_WrtInfoGen[u8_fmPh].mc_finalPhase = 0;
		phTable_WrtInfoGen[u8_fmPh].logo_en       = (cad_seqFlag == 1 || cadInfo.cad_id == _CAD_22) ? 1 : 0;

		// whether current film is written in new buffer
		sig_fmCnt	  = (cad_seqFlag == 1) ? 1 : (cadence_usableFlag(cadInfo, u8_fmPh) == 1? (sig_fmCnt + 1) : sig_fmCnt);

		u1_me1_wrt_noMove = (sig_fmCnt > _PHASE_FRC_ME1FRM_BUF_SIZE_ || cadence_usableFlag(cadInfo, u8_fmPh) == 0)? 1 : 0;
#if 1  // k4lp
		u1_mc_wrt_noMove = (sig_fmCnt > _PHASE_FRC_MCFRM_BUF_SIZE_ || cadence_usableFlag(cadInfo, u8_fmPh) == 0)? 1 : 0;
#else
		u1_me2_wrt_noMove = (sig_fmCnt > _PHASE_FRC_MCFRM_BUF_SIZE_ || cadence_usableFlag(cadInfo, u8_fmPh) == 0)? 1 : 0;
#endif
		phTable_WrtInfoGen[u8_fmPh].mc_finalPhase = cadence_usableFlag(cadInfo, u8_fmPh) == 0 ? 1 : 0;

		phTable_WrtInfoGen[u8_fmPh].me1_in_idx = u1_me1_wrt_noMove ? (_PHASE_FRC_ME1_TOTAL_BUF_SIZE_ - 1) : 0;
#if 1  // k4lp
		phTable_WrtInfoGen[u8_fmPh].me2_in_idx = u1_me1_wrt_noMove ? (_PHASE_FRC_ME1_TOTAL_BUF_SIZE_ - 1) : 0;
		if(MC_8_buffer_en)
			phTable_WrtInfoGen[u8_fmPh].mc_in_idx  = u1_me1_wrt_noMove ? (_PHASE_FRC_MC_TOTAL_BUF_SIZE_ - 1) : 0;
		else
			phTable_WrtInfoGen[u8_fmPh].mc_in_idx  = u1_mc_wrt_noMove ? (_PHASE_FRC_MC_TOTAL_BUF_SIZE_ - 1) : 0;
#else
		phTable_WrtInfoGen[u8_fmPh].me2_in_idx = u1_me2_wrt_noMove ? (_PHASE_FRC_ME2_TOTAL_BUF_SIZE_ - 1) : 0;
		phTable_WrtInfoGen[u8_fmPh].mc_in_idx  = u1_me2_wrt_noMove ? (_PHASE_FRC_ME2_TOTAL_BUF_SIZE_ - 1) : 0;
#endif
#if 0
		rtd_pr_memc_notice("[%s][CAD_ID,%d,][,%d,][,%d,%d,][,%d,%d,%d,][,%d,%d,]\n\r", __FUNCTION__, cadInfo.cad_id,
			u8_fmPh, cadence_seqFlag(cadInfo, u8_fmPh), cadence_usableFlag(cadInfo, u8_fmPh),
			sig_fmCnt, u1_me1_wrt_noMove, u1_mc_wrt_noMove,
			phTable_WrtInfoGen[u8_fmPh].me1_in_idx, phTable_WrtInfoGen[u8_fmPh].mc_in_idx);
#endif
	}
}

VOID FRC_phTable_WrtStep_VideoMode_Gen_idx(FRC_CADENCE cadInfo,unsigned char st_idx)
{
	unsigned char sig_fmCnt = 0, u8_fmPh;
	unsigned char u1_me1_wrt_noMove;
#if 1  // k4lp
	unsigned char u1_mc_wrt_noMove;
#else
	unsigned char u1_me2_wrt_noMove;
#endif
	unsigned char idx;
	Film_Wrt_Info     phTable_WrtInfo_param;
	unsigned char u8_k;
	unsigned char end_idx;
	unsigned int MC_8_buffer_en = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 0, &MC_8_buffer_en);

	for (u8_fmPh = 0; u8_fmPh < cadInfo.cad_outLen; u8_fmPh++)
	{
		unsigned char cad_seqFlag   = cadence_seqFlag(cadInfo, u8_fmPh);
		idx = u8_fmPh + st_idx;
		// whether current film phase is firsts phase.
		//phTable_WrtInfoGen_vd_22_32[idx].mc_finalPhase = 0;
		phTable_WrtInfoGen_vd_22_32[idx].logo_en       = (cad_seqFlag == 1) ? 1 : 0;

		// whether current film is written in new buffer
		sig_fmCnt	  = (cad_seqFlag == 1) ? 1 : (cadence_usableFlag(cadInfo, u8_fmPh) == 1? (sig_fmCnt + 1) : sig_fmCnt);

#if 1  // k4lp
		u1_me1_wrt_noMove = (sig_fmCnt > _PHASE_FRC_ME1FRM_BUF_SIZE_ || cadence_usableFlag(cadInfo, u8_fmPh) == 0)? 1 : 0;
		u1_mc_wrt_noMove = (sig_fmCnt > _PHASE_FRC_MCFRM_BUF_SIZE_ || cadence_usableFlag(cadInfo, u8_fmPh) == 0)? 1 : 0;

		phTable_WrtInfoGen_vd_22_32[idx].mc_finalPhase = cadence_usableFlag(cadInfo, u8_fmPh) == 0 ? 1 : 0;

		phTable_WrtInfoGen_vd_22_32[idx].me1_in_idx = u1_me1_wrt_noMove ? (_PHASE_FRC_ME1_TOTAL_BUF_SIZE_ - 1) : 0;
		phTable_WrtInfoGen_vd_22_32[idx].me2_in_idx = u1_me1_wrt_noMove ? (_PHASE_FRC_ME1_TOTAL_BUF_SIZE_ - 1) : 0;
		if(MC_8_buffer_en)
			phTable_WrtInfoGen_vd_22_32[idx].mc_in_idx  = u1_me1_wrt_noMove ? (_PHASE_FRC_MC_TOTAL_BUF_SIZE_ - 1) : 0;
		else
			phTable_WrtInfoGen_vd_22_32[idx].mc_in_idx  = u1_mc_wrt_noMove ? (_PHASE_FRC_MC_TOTAL_BUF_SIZE_ - 1) : 0;
#else
		u1_me1_wrt_noMove = (sig_fmCnt > _PHASE_FRC_ME1FRM_BUF_SIZE_ || cadence_usableFlag(cadInfo, u8_fmPh) == 0)? 1 : 0;
		u1_me2_wrt_noMove = (sig_fmCnt > _PHASE_FRC_ME2FRM_BUF_SIZE_ || cadence_usableFlag(cadInfo, u8_fmPh) == 0)? 1 : 0;

		phTable_WrtInfoGen_vd_22_32[idx].mc_finalPhase = cadence_usableFlag(cadInfo, u8_fmPh) == 0 ? 1 : 0;

		phTable_WrtInfoGen_vd_22_32[idx].me1_in_idx = u1_me1_wrt_noMove ? (_PHASE_FRC_ME1_TOTAL_BUF_SIZE_ - 1) : 0;
		phTable_WrtInfoGen_vd_22_32[idx].me2_in_idx = u1_me2_wrt_noMove ? (_PHASE_FRC_ME2_TOTAL_BUF_SIZE_ - 1) : 0;
		phTable_WrtInfoGen_vd_22_32[idx].mc_in_idx  = u1_me2_wrt_noMove ? (_PHASE_FRC_ME2_TOTAL_BUF_SIZE_ - 1) : 0;
#endif
#if 0
		rtd_pr_memc_notice("[%s][CAD_ID,%d,][,%d,][,%d,%d,][,%d,%d,%d,][idx,%d,%d,%d,]\n\r", __FUNCTION__, cadInfo.cad_id,
			u8_fmPh, cadence_seqFlag(cadInfo, u8_fmPh), cadence_usableFlag(cadInfo, u8_fmPh),
			sig_fmCnt, u1_me1_wrt_noMove, u1_mc_wrt_noMove,
			idx, phTable_WrtInfoGen_vd_22_32[idx].me1_in_idx, phTable_WrtInfoGen_vd_22_32[idx].mc_in_idx);
#endif
	}
	end_idx = st_idx + cadInfo.cad_outLen -1;
	phTable_WrtInfo_param.me1_in_idx = phTable_WrtInfoGen_vd_22_32[st_idx].me1_in_idx ;
	phTable_WrtInfo_param.me2_in_idx = phTable_WrtInfoGen_vd_22_32[st_idx].me2_in_idx ;
	phTable_WrtInfo_param.mc_in_idx  = phTable_WrtInfoGen_vd_22_32[st_idx].mc_in_idx ;
	phTable_WrtInfo_param.logo_en    = phTable_WrtInfoGen_vd_22_32[st_idx].logo_en ;
	phTable_WrtInfo_param.mc_finalPhase  = phTable_WrtInfoGen_vd_22_32[st_idx].mc_finalPhase ;

	for(u8_k = st_idx; u8_k < end_idx; u8_k++)
	{
		phTable_WrtInfoGen_vd_22_32[u8_k].me1_in_idx = phTable_WrtInfoGen_vd_22_32[u8_k+1].me1_in_idx ;
		phTable_WrtInfoGen_vd_22_32[u8_k].me2_in_idx = phTable_WrtInfoGen_vd_22_32[u8_k+1].me2_in_idx ;
		phTable_WrtInfoGen_vd_22_32[u8_k].mc_in_idx  = phTable_WrtInfoGen_vd_22_32[u8_k+1].mc_in_idx ;
		phTable_WrtInfoGen_vd_22_32[u8_k].logo_en    = phTable_WrtInfoGen_vd_22_32[u8_k+1].logo_en ;
		phTable_WrtInfoGen_vd_22_32[u8_k].mc_finalPhase  = phTable_WrtInfoGen_vd_22_32[u8_k+1].mc_finalPhase ;
	}

	phTable_WrtInfoGen_vd_22_32[end_idx].me1_in_idx = phTable_WrtInfo_param.me1_in_idx ;
	phTable_WrtInfoGen_vd_22_32[end_idx].me2_in_idx = phTable_WrtInfo_param.me2_in_idx ;
	phTable_WrtInfoGen_vd_22_32[end_idx].mc_in_idx  = phTable_WrtInfo_param.mc_in_idx ;
	phTable_WrtInfoGen_vd_22_32[end_idx].logo_en    = phTable_WrtInfo_param.logo_en ;
	phTable_WrtInfoGen_vd_22_32[end_idx].mc_finalPhase  = phTable_WrtInfo_param.mc_finalPhase ;

}

unsigned char  FRC_phTable_WrtStep_Check_me1(unsigned char u8_filmPh, unsigned char in3d_format)
{
	unsigned char _2D_filmPh = (in3d_format == _PQL_IN_LR)? (u8_filmPh >> 1) : u8_filmPh;

	return ((phTable_WrtInfoGen[_2D_filmPh].me1_in_idx + 1)%_PHASE_FRC_ME1_TOTAL_BUF_SIZE_);
}

unsigned char  FRC_phTable_WrtStep_Check_me2(unsigned char u8_filmPh, unsigned char in3d_format)
{
	unsigned char _2D_filmPh = (in3d_format == _PQL_IN_LR)? (u8_filmPh >> 1) : u8_filmPh;

#if 1  // k4lp
	return ((phTable_WrtInfoGen[_2D_filmPh].me2_in_idx + 1)%_PHASE_FRC_ME1_TOTAL_BUF_SIZE_);
#else
	return ((phTable_WrtInfoGen[_2D_filmPh].me2_in_idx + 1)%_PHASE_FRC_ME2_TOTAL_BUF_SIZE_);
#endif
}

#if 1  // k4lp
unsigned char  FRC_phTable_WrtStep_Check_mc(unsigned char u8_filmPh, unsigned char in3d_format)
{
	unsigned char _2D_filmPh = (in3d_format == _PQL_IN_LR)? (u8_filmPh >> 1) : u8_filmPh;
	//rtd_pr_memc_notice("[%s][filmPh,%d,][%d]\n\r", __FUNCTION__, u8_filmPh, ((phTable_WrtInfoGen[_2D_filmPh].mc_in_idx + 1)%_PHASE_FRC_MC_TOTAL_BUF_SIZE_));
	return ((phTable_WrtInfoGen[_2D_filmPh].mc_in_idx + 1)%_PHASE_FRC_MC_TOTAL_BUF_SIZE_);
}

unsigned char FRC_phTable_WrtStep_Check_mc_vd_22_32(unsigned char cadence_id, unsigned char u8_filmPh, unsigned char in3d_format)
{
	unsigned char _2D_filmPh = (in3d_format == _PQL_IN_LR)? (u8_filmPh >> 1) : u8_filmPh;
	unsigned char _2D_filmPh_idx = (cadence_id==0) ? _2D_filmPh:((cadence_id==1)? (_2D_filmPh + 1):(_2D_filmPh + 1+2));
	//rtd_pr_memc_notice("[%s][CAD,%d,filmPh,%d,][%d]\n\r", __FUNCTION__, cadence_id, u8_filmPh, ((phTable_WrtInfoGen_vd_22_32[_2D_filmPh_idx].mc_in_idx + 1)%_PHASE_FRC_MC_TOTAL_BUF_SIZE_));
	return ((phTable_WrtInfoGen_vd_22_32[_2D_filmPh_idx].mc_in_idx + 1)%_PHASE_FRC_MC_TOTAL_BUF_SIZE_);
}

#else
unsigned char FRC_phTable_WrtStep_Check_me2_vd_22_32(unsigned char cadence_id, unsigned char u8_filmPh, unsigned char in3d_format)
{
	unsigned char _2D_filmPh = (in3d_format == _PQL_IN_LR)? (u8_filmPh >> 1) : u8_filmPh;
	unsigned char _2D_filmPh_idx = (cadence_id==0) ? _2D_filmPh:((cadence_id==1)? (_2D_filmPh + 1):(_2D_filmPh + 1+2));

	return ((phTable_WrtInfoGen_vd_22_32[_2D_filmPh_idx].me2_in_idx + 1)%_PHASE_FRC_ME2_TOTAL_BUF_SIZE_);
}
#endif

extern unsigned int GS_me_pqc_mode;
unsigned int phTable_WrtStep_pack(unsigned char u8_stIdx)
{
	unsigned char  u8_k    = 0;
	unsigned int u32_ret = 0;
	//rtd_pr_memc_notice("[%s][%d][GS_me_pqc_mode=%d]\n\r", __FUNCTION__, __LINE__, GS_me_pqc_mode);
	for (u8_k = 0; u8_k < 4; u8_k ++)
	{
		unsigned char u8_phT_idx = u8_k + u8_stIdx;
		unsigned char u8_rb_one  = 0;
		if(GS_me_pqc_mode == 3){
			phTable_WrtInfoGen_Amend[u8_phT_idx].logo_en        = 1; // phTable_WrtInfoGen[u8_k].logo_en ;
		}
		u8_rb_one  = (phTable_WrtInfoGen_Amend[u8_phT_idx].mc_in_idx     << 5) |\
					(phTable_WrtInfoGen_Amend[u8_phT_idx].mc_finalPhase << 4) | \
					(phTable_WrtInfoGen_Amend[u8_phT_idx].me1_in_idx     << 1) | \
					(phTable_WrtInfoGen_Amend[u8_phT_idx].logo_en);
		u32_ret          = u32_ret | (u8_rb_one << (u8_k * 8));
	}

	return u32_ret;
}

unsigned int phTable_WrtStep_pack_vd_22_32(unsigned char u8_stIdx)
{
	unsigned char  u8_k    = 0;
	unsigned int u32_ret = 0;
	//rtd_pr_memc_notice("[%s][%d][GS_me_pqc_mode=%d]\n\r", __FUNCTION__, __LINE__, GS_me_pqc_mode);
	for (u8_k = 0; u8_k < 4; u8_k ++)
	{
		unsigned char u8_phT_idx = u8_k + u8_stIdx;
		unsigned char u8_rb_one = 0;
		if(GS_me_pqc_mode == 3){
			phTable_WrtInfoGen_vd_22_32[u8_phT_idx].logo_en        = 1; // phTable_WrtInfoGen[u8_k].logo_en ;
		}
		u8_rb_one  = (phTable_WrtInfoGen_vd_22_32[u8_phT_idx].mc_in_idx     << 5) |\
					(phTable_WrtInfoGen_vd_22_32[u8_phT_idx].mc_finalPhase << 4) | \
					(phTable_WrtInfoGen_vd_22_32[u8_phT_idx].me1_in_idx     << 1) | \
					(phTable_WrtInfoGen_vd_22_32[u8_phT_idx].logo_en);
		u32_ret          = u32_ret | (u8_rb_one << (u8_k * 8));
	}

	return u32_ret;
}


unsigned int phTable_me2_WrtStep_pack(unsigned char u8_stIdx)
{
	unsigned char  u8_k    = 0;
	unsigned int u32_ret = 0;
	for (u8_k = 0; u8_k < 8; u8_k ++)
	{
		unsigned char u8_phT_idx = u8_k + u8_stIdx;
		unsigned char u8_rb_one  =  phTable_WrtInfoGen_Amend[u8_phT_idx].me2_in_idx ;
		u32_ret          = u32_ret | (u8_rb_one << (u8_k * 4));
	}

	return u32_ret;
}

unsigned int phTable_me2_WrtStep_pack_vd_22_32(unsigned char u8_stIdx)
{
	unsigned char  u8_k    = 0;
	unsigned int u32_ret = 0;
	for (u8_k = 0; u8_k < 8; u8_k ++)
	{
		unsigned char u8_phT_idx = u8_k + u8_stIdx;
		unsigned char u8_rb_one  =  phTable_WrtInfoGen_vd_22_32[u8_phT_idx].me2_in_idx ;
		u32_ret          = u32_ret | (u8_rb_one << (u8_k * 4));
	}

	return u32_ret;
}

/////////         3. generate phase table.           ////////////////////////////
VOID FRC_phTable_Init_decGen_config()
{
#if 1  // k4lp
	//  me1,me2 buff 2/8 ,mc buff 1/6
	// video
	phT_DecInitInfo[_CAD_VIDEO].ini_me2_i_offset     	= 4;
	phT_DecInitInfo[_CAD_VIDEO].ini_me2_p_offset     	= 3;
	phT_DecInitInfo[_CAD_VIDEO].ini_mc_i_offset      		= 4;
	phT_DecInitInfo[_CAD_VIDEO].ini_mc_p_offset      	= 3;
	phT_DecInitInfo[_CAD_VIDEO].ini_ph_frac_nume     	= 0;
	phT_DecInitInfo[_CAD_VIDEO].ini_ph_frac_denom       = 1;
	phT_DecInitInfo[_CAD_VIDEO].ini_me2_chgImg_cnt    	= 3;

	//22
	phT_DecInitInfo[_CAD_22].ini_me2_i_offset   	 		= 5;// 3;
	phT_DecInitInfo[_CAD_22].ini_me2_p_offset   	 	= 3;// 2;
	phT_DecInitInfo[_CAD_22].ini_mc_i_offset    			= 3;
	phT_DecInitInfo[_CAD_22].ini_mc_p_offset    			= 2;
	phT_DecInitInfo[_CAD_22].ini_ph_frac_nume       		= 1;
	phT_DecInitInfo[_CAD_22].ini_ph_frac_denom      		= 12;
	phT_DecInitInfo[_CAD_22].ini_me2_chgImg_cnt    	= 2;

	// 32
	phT_DecInitInfo[_CAD_32].ini_me2_i_offset   			= 5;
	phT_DecInitInfo[_CAD_32].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_32].ini_mc_i_offset   			= 3;
	phT_DecInitInfo[_CAD_32].ini_mc_p_offset   			= 2;
	phT_DecInitInfo[_CAD_32].ini_ph_frac_nume  		= 1;
	phT_DecInitInfo[_CAD_32].ini_ph_frac_denom		= 5;
	phT_DecInitInfo[_CAD_32].ini_me2_chgImg_cnt     	= 2;

	// 32322  3
	phT_DecInitInfo[_CAD_32322].ini_me2_i_offset   		= 5;
	phT_DecInitInfo[_CAD_32322].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_32322].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_32322].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_32322].ini_ph_frac_nume  		= 0;
	phT_DecInitInfo[_CAD_32322].ini_ph_frac_denom 		= 24;
	phT_DecInitInfo[_CAD_32322].ini_me2_chgImg_cnt      = 2;

	// 334  4
	phT_DecInitInfo[_CAD_334].ini_me2_i_offset  		= 5;
	phT_DecInitInfo[_CAD_334].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_334].ini_mc_i_offset   			= 3;
	phT_DecInitInfo[_CAD_334].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_334].ini_ph_frac_nume  		= 6;
	phT_DecInitInfo[_CAD_334].ini_ph_frac_denom 		= 20;
	phT_DecInitInfo[_CAD_334].ini_me2_chgImg_cnt    	= 2;

	//22224   5
	phT_DecInitInfo[_CAD_22224].ini_me2_i_offset   		= 5;
	phT_DecInitInfo[_CAD_22224].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_22224].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_22224].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_22224].ini_ph_frac_nume  		= 8;
	phT_DecInitInfo[_CAD_22224].ini_ph_frac_denom 		= 24;
	phT_DecInitInfo[_CAD_22224].ini_me2_chgImg_cnt    	= 2;

	//2224   6
	phT_DecInitInfo[_CAD_2224].ini_me2_i_offset   		= 5;
	phT_DecInitInfo[_CAD_2224].ini_me2_p_offset  	    	= 3;
	phT_DecInitInfo[_CAD_2224].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_2224].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_2224].ini_ph_frac_nume  		= 5;
	phT_DecInitInfo[_CAD_2224].ini_ph_frac_denom 		= 20;
	phT_DecInitInfo[_CAD_2224].ini_me2_chgImg_cnt    	= 2;

	//3223   7
	phT_DecInitInfo[_CAD_3223].ini_me2_i_offset   		= 5;
	phT_DecInitInfo[_CAD_3223].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_3223].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_3223].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_3223].ini_ph_frac_nume  		= 4;
	phT_DecInitInfo[_CAD_3223].ini_ph_frac_denom 		= 20;
	phT_DecInitInfo[_CAD_3223].ini_me2_chgImg_cnt    	= 2;

	// 55  8
	phT_DecInitInfo[_CAD_55].ini_me2_i_offset   			= 5;
	phT_DecInitInfo[_CAD_55].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_55].ini_mc_i_offset   			= 3;
	phT_DecInitInfo[_CAD_55].ini_mc_p_offset   			= 2;
	phT_DecInitInfo[_CAD_55].ini_ph_frac_nume  		= 4;
	phT_DecInitInfo[_CAD_55].ini_ph_frac_denom 		= 10;
	phT_DecInitInfo[_CAD_55].ini_me2_chgImg_cnt    	= 2;

	//66   9
	phT_DecInitInfo[_CAD_66].ini_me2_i_offset   			= 5;
	phT_DecInitInfo[_CAD_66].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_66].ini_mc_i_offset   			= 3;
	phT_DecInitInfo[_CAD_66].ini_mc_p_offset   			= 2;
	phT_DecInitInfo[_CAD_66].ini_ph_frac_nume  		= 4;
	phT_DecInitInfo[_CAD_66].ini_ph_frac_denom 		= 12;
	phT_DecInitInfo[_CAD_66].ini_me2_chgImg_cnt    	= 2;

	//44   10
	phT_DecInitInfo[_CAD_44].ini_me2_i_offset   			= 5;
	phT_DecInitInfo[_CAD_44].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_44].ini_mc_i_offset   			= 3;
	phT_DecInitInfo[_CAD_44].ini_mc_p_offset   			= 2;
	phT_DecInitInfo[_CAD_44].ini_ph_frac_nume  		= 2;
	phT_DecInitInfo[_CAD_44].ini_ph_frac_denom 		= 8;
	phT_DecInitInfo[_CAD_44].ini_me2_chgImg_cnt    	= 2;

	// 1112  11
	phT_DecInitInfo[_CAD_1112].ini_me2_i_offset   		= 4;
	phT_DecInitInfo[_CAD_1112].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_1112].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_1112].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_1112].ini_ph_frac_nume  		= 0;
	phT_DecInitInfo[_CAD_1112].ini_ph_frac_denom 		= 10;
	phT_DecInitInfo[_CAD_1112].ini_me2_chgImg_cnt    	= 2;

	//11112   12
	phT_DecInitInfo[_CAD_11112].ini_me2_i_offset   		= 4;
	phT_DecInitInfo[_CAD_11112].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_11112].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_11112].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_11112].ini_ph_frac_nume  		= 1;
	phT_DecInitInfo[_CAD_11112].ini_ph_frac_denom 		= 12;
	phT_DecInitInfo[_CAD_11112].ini_me2_chgImg_cnt    	= 2;

	//122   13
	phT_DecInitInfo[_CAD_122].ini_me2_i_offset   		= 5;
	phT_DecInitInfo[_CAD_122].ini_me2_p_offset   		= 3;
	phT_DecInitInfo[_CAD_122].ini_mc_i_offset   			= 3;
	phT_DecInitInfo[_CAD_122].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_122].ini_ph_frac_nume  		= 1;
	phT_DecInitInfo[_CAD_122].ini_ph_frac_denom 		= 10;
	phT_DecInitInfo[_CAD_122].ini_me2_chgImg_cnt    	= 2;

	//11'23   14
	phT_DecInitInfo[_CAD_11i23].ini_me2_i_offset   		= 5;
	phT_DecInitInfo[_CAD_11i23].ini_me2_p_offset  	 	= 3;
	phT_DecInitInfo[_CAD_11i23].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_11i23].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_11i23].ini_ph_frac_nume  		= 5;
	phT_DecInitInfo[_CAD_11i23].ini_ph_frac_denom 		= 25;
	phT_DecInitInfo[_CAD_11i23].ini_me2_chgImg_cnt    	= 2;

	//1225   16
	phT_DecInitInfo[_CAD_1225_T1].ini_me2_i_offset   		= 5;
	phT_DecInitInfo[_CAD_1225_T1].ini_me2_p_offset  	 	= 3;
	phT_DecInitInfo[_CAD_1225_T1].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_1225_T1].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_1225_T1].ini_ph_frac_nume  		= 5;
	phT_DecInitInfo[_CAD_1225_T1].ini_ph_frac_denom 		= 25;
	phT_DecInitInfo[_CAD_1225_T1].ini_me2_chgImg_cnt    	= 2;

	//1225_P2   17
	phT_DecInitInfo[_CAD_1225_T2].ini_me2_i_offset   		= 5;
	phT_DecInitInfo[_CAD_1225_T2].ini_me2_p_offset  	 	= 3;
	phT_DecInitInfo[_CAD_1225_T2].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_1225_T2].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_1225_T2].ini_ph_frac_nume  		= 5;
	phT_DecInitInfo[_CAD_1225_T2].ini_ph_frac_denom 		= 25;
	phT_DecInitInfo[_CAD_1225_T2].ini_me2_chgImg_cnt    	= 2;

	//12214   18
	phT_DecInitInfo[_CAD_12214].ini_me2_i_offset   		= 5;
	phT_DecInitInfo[_CAD_12214].ini_me2_p_offset  	 	= 3;
	phT_DecInitInfo[_CAD_12214].ini_mc_i_offset   		= 3;
	phT_DecInitInfo[_CAD_12214].ini_mc_p_offset   		= 2;
	phT_DecInitInfo[_CAD_12214].ini_ph_frac_nume  		= 1;
	phT_DecInitInfo[_CAD_12214].ini_ph_frac_denom 		= 10;
	phT_DecInitInfo[_CAD_12214].ini_me2_chgImg_cnt    	= 2;
#else
	//  me1 buff 2/6 me2,mc buff 1/6
		// video
	phT_DecInitInfo[_CAD_VIDEO].ini_me2_i_offset      = 3;
	phT_DecInitInfo[_CAD_VIDEO].ini_me2_p_offset      = 2;
		phT_DecInitInfo[_CAD_VIDEO].ini_ph_frac_nume      = 0;
		phT_DecInitInfo[_CAD_VIDEO].ini_ph_frac_denom     = 1;
	phT_DecInitInfo[_CAD_VIDEO].ini_me2_chgImg_cnt    = 2;

		//22
	phT_DecInitInfo[_CAD_22].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_22].ini_me2_p_offset   = 2;
		phT_DecInitInfo[_CAD_22].ini_ph_frac_nume  = 1;
	phT_DecInitInfo[_CAD_22].ini_ph_frac_denom = 12;
		phT_DecInitInfo[_CAD_22].ini_me2_chgImg_cnt    = 2;

		// 32
	phT_DecInitInfo[_CAD_32].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_32].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_32].ini_ph_frac_nume  = 1;
		phT_DecInitInfo[_CAD_32].ini_ph_frac_denom = 5;
		phT_DecInitInfo[_CAD_32].ini_me2_chgImg_cnt    = 2;

		// 32322  3
	phT_DecInitInfo[_CAD_32322].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_32322].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_32322].ini_ph_frac_nume  = 0;
		phT_DecInitInfo[_CAD_32322].ini_ph_frac_denom = 24;
		phT_DecInitInfo[_CAD_32322].ini_me2_chgImg_cnt    = 2;

		// 334  4

	phT_DecInitInfo[_CAD_334].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_334].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_334].ini_ph_frac_nume  = 6;
		phT_DecInitInfo[_CAD_334].ini_ph_frac_denom = 20;
		phT_DecInitInfo[_CAD_334].ini_me2_chgImg_cnt    = 2;

		//22224   5
	phT_DecInitInfo[_CAD_22224].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_22224].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_22224].ini_ph_frac_nume  = 8;
		phT_DecInitInfo[_CAD_22224].ini_ph_frac_denom = 24;
		phT_DecInitInfo[_CAD_22224].ini_me2_chgImg_cnt    = 2;

		//2224   6
	phT_DecInitInfo[_CAD_2224].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_2224].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_2224].ini_ph_frac_nume  = 5;
		phT_DecInitInfo[_CAD_2224].ini_ph_frac_denom = 20;
		phT_DecInitInfo[_CAD_2224].ini_me2_chgImg_cnt    = 2;

		//3223   7
	phT_DecInitInfo[_CAD_3223].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_3223].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_3223].ini_ph_frac_nume  = 4;
		phT_DecInitInfo[_CAD_3223].ini_ph_frac_denom = 20;
		phT_DecInitInfo[_CAD_3223].ini_me2_chgImg_cnt    = 2;

		// 55  8
	phT_DecInitInfo[_CAD_55].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_55].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_55].ini_ph_frac_nume  = 4;
		phT_DecInitInfo[_CAD_55].ini_ph_frac_denom = 10;
		phT_DecInitInfo[_CAD_55].ini_me2_chgImg_cnt    = 2;

		//66   9
	phT_DecInitInfo[_CAD_66].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_66].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_66].ini_ph_frac_nume  = 4;
		phT_DecInitInfo[_CAD_66].ini_ph_frac_denom = 12;
		phT_DecInitInfo[_CAD_66].ini_me2_chgImg_cnt    = 2;

		//44   10
	phT_DecInitInfo[_CAD_44].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_44].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_44].ini_ph_frac_nume  = 2;
		phT_DecInitInfo[_CAD_44].ini_ph_frac_denom = 8;
		phT_DecInitInfo[_CAD_44].ini_me2_chgImg_cnt    = 2;

		// 1112  11
	phT_DecInitInfo[_CAD_1112].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_1112].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_1112].ini_ph_frac_nume  = 0;
		phT_DecInitInfo[_CAD_1112].ini_ph_frac_denom = 10;
	phT_DecInitInfo[_CAD_1112].ini_me2_chgImg_cnt    = 2;

		//11112   12
	phT_DecInitInfo[_CAD_11112].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_11112].ini_me2_p_offset   = 2;
		phT_DecInitInfo[_CAD_11112].ini_ph_frac_nume  = 1;
		phT_DecInitInfo[_CAD_11112].ini_ph_frac_denom = 12;
		phT_DecInitInfo[_CAD_11112].ini_me2_chgImg_cnt    = 2;

		//122   13
	phT_DecInitInfo[_CAD_122].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_122].ini_me2_p_offset   = 2;
		phT_DecInitInfo[_CAD_122].ini_ph_frac_nume  = 1;
		phT_DecInitInfo[_CAD_122].ini_ph_frac_denom = 10;
		phT_DecInitInfo[_CAD_122].ini_me2_chgImg_cnt    = 2;

		//11'23   14
	phT_DecInitInfo[_CAD_11i23].ini_me2_i_offset   = 3;
	phT_DecInitInfo[_CAD_11i23].ini_me2_p_offset   = 2;
	phT_DecInitInfo[_CAD_11i23].ini_ph_frac_nume  = 5;
		phT_DecInitInfo[_CAD_11i23].ini_ph_frac_denom = 25;
		phT_DecInitInfo[_CAD_11i23].ini_me2_chgImg_cnt    = 2;
#endif
}

/////////
VOID FRC_phTable_VideoMode_Proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{
	unsigned short  u16_k;

	if(pOutput->u16_phT_endIdx_p1 > 255){
		rtd_pr_memc_notice("[%s]Invalid u16_phT_endIdx_p1:%d!!\n",__FUNCTION__,pOutput->u16_phT_endIdx_p1);
		pOutput->u16_phT_endIdx_p1 = 255;
	}
	if (pParam->u1_goldenPhT_gen_en == 0)
	{
		for(u16_k = pOutput->u16_phT_stIdx; u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
		{
			phaseTable_unpack(phTable_WrtLSB[u16_k], phTable_WrtMSB[u16_k], &(phTable_DecInfoGen[u16_k]));
		}
	}
	else
	{
		//LISA_160727
		//FRC_phTable_WrtStep_VideoMode_Gen(frc_cadTable[pParam->u8_cadence_id]);

		FRC_decPhT_goldenGen_Proc(pParam, pOutput);

		// 保存 golden table, 用于 fall back.
		for(u16_k = pOutput->u16_phT_stIdx; u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
		{
			unsigned int pack_MSB = 0, pack_LSB = 0;
			Ph_Dec_Pack_Info packInfo;
			FRC_phTable_IntpInfo_PackGen(pParam, pOutput, u16_k, &packInfo);
			phaseTable_pack(&packInfo, &pack_LSB, &pack_MSB);

			phTable_WrtLSB[u16_k] = pack_LSB;
			phTable_WrtMSB[u16_k] = pack_MSB;
		}
	}

	if (pParam->u1_phFlbk_en == 1)
	{
		FRC_decPhT_flbk_proc(pParam, pOutput);
	}
}

VOID FRC_decPhT_PCmode_LowDelay_Proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{

}

VOID FRC_decPhT_PCmode_Proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{
	unsigned char u8_k  = 0;
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	PQL_OUTPUT_FRAME_RATE  out_fmRate = s_pContext->_external_data._output_frameRate;
	PQL_INPUT_FRAME_RATE	in_fmRate = s_pContext->_external_data._input_frameRate;

	pOutput->u16_phT_stIdx     = 0;
	pOutput->u16_phT_endIdx_p1 = (unsigned short)pParam->u6_sys_M;
	pOutput->u16_phT_length    = (unsigned short)pParam->u6_sys_M;
	if(pOutput->u16_phT_endIdx_p1 > 255){
		rtd_pr_memc_notice("[%s]Invalid u16_phT_endIdx_p1:%d!!\n",__FUNCTION__,pOutput->u16_phT_endIdx_p1);
		pOutput->u16_phT_endIdx_p1 = 255;
	}
	for (u8_k = 0; u8_k < pParam->u6_sys_M; u8_k ++)
	{
		phTable_DecInfoGen[u8_k].film_mode  = 0; // force video
		phTable_DecInfoGen[u8_k].film_phase = 0;
		phTable_DecInfoGen[u8_k].out_phase  = u8_k + 1;
		phTable_DecInfoGen[u8_k].me2_phase   = 0; // (1 << ME_PHASE_PRECISION >> 1);
		phTable_DecInfoGen[u8_k].mc_phase   = 0; // (1 << MC_PHASE_PRECISION >> 1);

		#if 0
		phTable_DecInfoGen[u8_k].me1_i_offset   = 0;
		phTable_DecInfoGen[u8_k].me1_p_offset   = 0;
		phTable_DecInfoGen[u8_k].me2_i_offset   = 0;
		phTable_DecInfoGen[u8_k].me2_p_offset   = 0;
		phTable_DecInfoGen[u8_k].mc_i_offset   = 0;
		phTable_DecInfoGen[u8_k].mc_p_offset   = 0;
		#else
		if(Get_DISPLAY_PANEL_TYPE()== P_VBY1_TO_HDMI_MEMC_FREERUN){
			phTable_DecInfoGen[u8_k].me1_i_offset   = 1;
			phTable_DecInfoGen[u8_k].me1_p_offset   = 1;
			phTable_DecInfoGen[u8_k].me2_i_offset   = 1;
			phTable_DecInfoGen[u8_k].me2_p_offset   = 1;
			phTable_DecInfoGen[u8_k].mc_i_offset   = 1;
			phTable_DecInfoGen[u8_k].mc_p_offset   = 1;
		}
		else if(s_pContext->_output_frc_phtable.u1_lowdelaystate){
			if(VR360_en == 0){
				if((((in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_120HZ)) || ((in_fmRate == _PQL_IN_50HZ) && (out_fmRate == _PQL_OUT_100HZ)))){
					if(MEMC_Lib_get_DisplayMode_Src() == 2/*VSC_INPUTSRC_HDMI*/ && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1){
						unsigned char mc_i_offset[2] =	{1,1};
						unsigned char mc_p_offset[2] =	{1,1};
						unsigned char index_offset = u8_k - (pOutput->u16_phT_stIdx);
						phTable_DecInfoGen[u8_k].mc_i_offset   = mc_i_offset[index_offset];
						phTable_DecInfoGen[u8_k].mc_p_offset   = mc_p_offset[index_offset];
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
					else{
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
				}
				else if((((in_fmRate == _PQL_IN_30HZ) && (out_fmRate == _PQL_OUT_120HZ)) || ((in_fmRate == _PQL_IN_25HZ) && (out_fmRate == _PQL_OUT_100HZ)))){
					if(MEMC_Lib_get_DisplayMode_Src() == 2/*VSC_INPUTSRC_HDMI*/ && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1){
						unsigned char mc_i_offset[4] =	{0,0,0,1}; // {1,1,1,2};
						unsigned char mc_p_offset[4] =	{0,0,0,1}; // {1,1,1,2};
						unsigned char index_offset = u8_k - (pOutput->u16_phT_stIdx);
						phTable_DecInfoGen[u8_k].mc_i_offset   = mc_i_offset[index_offset];
						phTable_DecInfoGen[u8_k].mc_p_offset   = mc_p_offset[index_offset];
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
					else{
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
				}
				else if((in_fmRate == _PQL_IN_24HZ) && (out_fmRate == _PQL_OUT_120HZ)){
					if(MEMC_Lib_get_DisplayMode_Src() == 2/*VSC_INPUTSRC_HDMI*/ && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1){
						unsigned char mc_i_offset[10] = 	{0,0,0,0,1,0,0,0,0,1}; // {1,1,1,1,2,1,1,1,1,2};
						unsigned char mc_p_offset[10] = 	{0,0,0,0,1,0,0,0,0,1}; // {1,1,1,1,2,1,1,1,1,2};
						unsigned char index_offset = u8_k - (pOutput->u16_phT_stIdx);
						phTable_DecInfoGen[u8_k].mc_i_offset   = mc_i_offset[index_offset];
						phTable_DecInfoGen[u8_k].mc_p_offset   = mc_p_offset[index_offset];
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
					else{
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
				}
				else if((((in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_60HZ)) || ((in_fmRate == _PQL_IN_50HZ) && (out_fmRate == _PQL_OUT_50HZ)))){
					if(MEMC_Lib_get_DisplayMode_Src() == 2/*VSC_INPUTSRC_HDMI*/ && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1){
						unsigned char mc_i_offset[1] =	{0};
						unsigned char mc_p_offset[1] =	{0};
						unsigned char index_offset = u8_k - (pOutput->u16_phT_stIdx);
						phTable_DecInfoGen[u8_k].mc_i_offset   = mc_i_offset[index_offset];
						phTable_DecInfoGen[u8_k].mc_p_offset   = mc_p_offset[index_offset];
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
					else{
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
				}
				else if((((in_fmRate == _PQL_IN_30HZ) && (out_fmRate == _PQL_OUT_60HZ)) || ((in_fmRate == _PQL_IN_25HZ) && (out_fmRate == _PQL_OUT_50HZ)))){
					if(MEMC_Lib_get_DisplayMode_Src() == 2/*VSC_INPUTSRC_HDMI*/ && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1){
						unsigned char mc_i_offset[2] =	{0,1};
						unsigned char mc_p_offset[2] =	{0,1};
						unsigned char index_offset = u8_k - (pOutput->u16_phT_stIdx);
						phTable_DecInfoGen[u8_k].mc_i_offset   = mc_i_offset[index_offset];
						phTable_DecInfoGen[u8_k].mc_p_offset   = mc_p_offset[index_offset];
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
					else{
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
				}
				else if((in_fmRate == _PQL_IN_24HZ) && (out_fmRate == _PQL_OUT_60HZ)){
					if(MEMC_Lib_get_DisplayMode_Src() == 2/*VSC_INPUTSRC_HDMI*/ && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1){
						unsigned char mc_i_offset[5] =	{0,0,0,0,0}; // {1,1,0,1,1};
						unsigned char mc_p_offset[5] = {0,0,0,0,0}; // {1,1,0,1,1};
						unsigned char index_offset = u8_k - (pOutput->u16_phT_stIdx);
						phTable_DecInfoGen[u8_k].mc_i_offset   = mc_i_offset[index_offset];
						phTable_DecInfoGen[u8_k].mc_p_offset   = mc_p_offset[index_offset];
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
					else{
						printk(KERN_EMERG"[%s][%d][%d,%d]\n\r", __FUNCTION__, Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), in_fmRate, out_fmRate);
					}
				}
			}
//			FRC_decPhT_PCmode_LowDelay_Proc();
		}
		else{
			phTable_DecInfoGen[u8_k].me1_i_offset   = 0;
			phTable_DecInfoGen[u8_k].me1_p_offset   = 0;
			phTable_DecInfoGen[u8_k].me2_i_offset   = 0;
			phTable_DecInfoGen[u8_k].me2_p_offset   = 0;
			#if 1  // k4lp
			phTable_DecInfoGen[u8_k].mc_i_offset   = 0;
			phTable_DecInfoGen[u8_k].mc_p_offset   = 0;
			#endif
		}
		#endif
		phTable_DecInfoGen[u8_k].ppfv_offset   = 0;
		phTable_DecInfoGen[u8_k].me1_last   = 1;
	}
}
///////
VOID FRC_phTable_IntpInfo_PackGen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput, unsigned short phDecInfo_k, Ph_Dec_Pack_Info *pPackInfo)
{
	//// assign
	pPackInfo->u8_msb[MSB_FILM_MODE]                 = phTable_DecInfoGen[phDecInfo_k].film_mode;
	pPackInfo->u8_msb[MSB_OUT_PHASE]                 = phTable_DecInfoGen[phDecInfo_k].out_phase;
	pPackInfo->u8_msb[MSB_FILM_PHASE]                = phTable_DecInfoGen[phDecInfo_k].film_phase;
	pPackInfo->u8_msb[MSB_ME2_PPFV_BUF_IDX]          = phTable_DecInfoGen[phDecInfo_k].ppfv_offset;
	pPackInfo->u8_msb[MSB_ME2_IBME_P_INDEX_OFFSET]   = phTable_DecInfoGen[phDecInfo_k].me2_p_offset;
	pPackInfo->u8_msb[MSB_ME2_IBME_I_INDEX_OFFSET]   = phTable_DecInfoGen[phDecInfo_k].me2_i_offset;
	pPackInfo->u8_msb[MSB_ME2_PHASE]                 = phTable_DecInfoGen[phDecInfo_k].me2_phase;

	pPackInfo->u8_lsb[LSB_ME_LAST_PHASE]             = phTable_DecInfoGen[phDecInfo_k].me1_last;
	pPackInfo->u8_lsb[LSB_ME_DTS_P_INDEX]            = phTable_DecInfoGen[phDecInfo_k].me1_p_offset;
	pPackInfo->u8_lsb[LSB_ME_DTS_I_INDEX]            = phTable_DecInfoGen[phDecInfo_k].me1_i_offset;
#if 1  // k4lp
	pPackInfo->u8_lsb[LSB_MC_CTS_P_INDEX]            = phTable_DecInfoGen[phDecInfo_k].mc_p_offset;
	pPackInfo->u8_lsb[LSB_MC_CTS_I_INDEX]            = phTable_DecInfoGen[phDecInfo_k].mc_i_offset;
#else
	pPackInfo->u8_lsb[LSB_MC_CTS_P_INDEX]            = phTable_DecInfoGen[phDecInfo_k].me2_p_offset;
	pPackInfo->u8_lsb[LSB_MC_CTS_I_INDEX]            = phTable_DecInfoGen[phDecInfo_k].me2_i_offset;
#endif
	pPackInfo->u8_lsb[LSB_MC_PHASE]                  = phTable_DecInfoGen[phDecInfo_k].mc_phase;
}
////////

VOID FRC_decPhT_YT_ResizeGen_Proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{
	const _PQLCONTEXT     *s_pContext = GetPQLContext();
	PQL_OUTPUT_FRAME_RATE  out_fmRate = s_pContext->_external_data._output_frameRate;
	PQL_INPUT_FRAME_RATE	in_fmRate = s_pContext->_external_data._input_frameRate;
	unsigned int u16_k = 0;
//	unsigned char u1_YT_resize_chg_status = 0, u1_hdmi_gamemode_chg_status = 0;

//	u1_YT_resize_chg_status = s_pContext->_output_filmDetectctrl.u1_YT_resize_chg_status;
//	u1_hdmi_gamemode_chg_status = s_pContext->_output_filmDetectctrl.u1_hdmi_gamemode_chg_status;

	if(((in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_120HZ))
		|| ((in_fmRate == _PQL_IN_50HZ) && (out_fmRate == _PQL_OUT_100HZ)))
	{
		if(lowdelay_state_chg_cnt > 0){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char mc_offset = 0;
				#if CONFIG_MC_8_BUFFER
				if(lowdelay_state_chg_cnt > 6)
					mc_offset = 6;
				else
					mc_offset = lowdelay_state_chg_cnt;
				#else
				if(lowdelay_state_chg_cnt > 3)
					mc_offset = 3;
				else
					mc_offset = lowdelay_state_chg_cnt;
				#endif
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_offset; // mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_offset;
				rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
			}
		}
		//rtd_pr_memc_notice("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
	}
	else if(((in_fmRate == _PQL_IN_30HZ) && (out_fmRate == _PQL_OUT_120HZ))
		|| ((in_fmRate == _PQL_IN_25HZ) && (out_fmRate == _PQL_OUT_100HZ)))
	{
		if(lowdelay_state_chg_cnt > 1){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char mc_offset = 0;
				#if CONFIG_MC_8_BUFFER
				if(lowdelay_state_chg_cnt > 5)
					mc_offset = 5;
				else
					mc_offset = lowdelay_state_chg_cnt;
				#else
				if(lowdelay_state_chg_cnt > 3)
					mc_offset = 3;
				else
					mc_offset = lowdelay_state_chg_cnt;
				#endif
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_offset; // mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_offset;
				rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
			}
		}
		//rtd_pr_memc_notice("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
	}
	else if((in_fmRate == _PQL_IN_24HZ) && (out_fmRate == _PQL_OUT_120HZ))
	{
		if(lowdelay_state_chg_cnt > 1){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char mc_offset = 0;
				#if CONFIG_MC_8_BUFFER
				if(lowdelay_state_chg_cnt > 5)
					mc_offset = 5;
				else
					mc_offset = lowdelay_state_chg_cnt;
				#else
				if(lowdelay_state_chg_cnt > 3)
					mc_offset = 3;
				else
					mc_offset = lowdelay_state_chg_cnt;
				#endif
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_offset; // mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_offset;
			}
			rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
		}
		//rtd_pr_memc_notice("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
	}
	else if(((in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_60HZ))
		|| ((in_fmRate == _PQL_IN_50HZ) && (out_fmRate == _PQL_OUT_50HZ)))
	{
		if(lowdelay_state_chg_cnt == 6){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if CONFIG_MC_8_BUFFER
				unsigned char mc_i_offset[1] = 	{5}; // delay 2 frames
				unsigned char mc_p_offset[1] = 	{5};
				#else
				unsigned char mc_i_offset[1] = 	{3};
				unsigned char mc_p_offset[1] = 	{3};
				#endif
				unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
			}
			rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
		}
		else if(lowdelay_state_chg_cnt == 5){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if CONFIG_MC_8_BUFFER
				unsigned char mc_i_offset[1] = 	{4}; // delay 3 frames
				unsigned char mc_p_offset[1] = 	{4};
				#else
				unsigned char mc_i_offset[1] = 	{3};
				unsigned char mc_p_offset[1] = 	{3};
				#endif
				unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
			}
			rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
		}
		else if(lowdelay_state_chg_cnt == 4){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if CONFIG_MC_8_BUFFER
				unsigned char mc_i_offset[1] = 	{3}; // delay 4 frames
				unsigned char mc_p_offset[1] = 	{3};
				#else
				unsigned char mc_i_offset[1] = 	{3};
				unsigned char mc_p_offset[1] = 	{3};
				#endif
				unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
			}
			rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
		}
		else if(lowdelay_state_chg_cnt == 3){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if CONFIG_MC_8_BUFFER
				unsigned char mc_i_offset[1] = 	{2}; // delay 5 frames
				unsigned char mc_p_offset[1] = 	{2};
				#else
				unsigned char mc_i_offset[1] = 	{3};
				unsigned char mc_p_offset[1] = 	{3};
				#endif
				unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
			}
			rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
		}
		else if(lowdelay_state_chg_cnt == 2){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if CONFIG_MC_8_BUFFER
				unsigned char mc_i_offset[1] = 	{1}; // delay 6 frames
				unsigned char mc_p_offset[1] = 	{1};
				#else
				unsigned char mc_i_offset[1] = 	{3};
				unsigned char mc_p_offset[1] = 	{3};
				#endif
				unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
			}
			rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
		}

	}
	else if(((in_fmRate == _PQL_IN_30HZ) && (out_fmRate == _PQL_OUT_60HZ))
		|| ((in_fmRate == _PQL_IN_25HZ) && (out_fmRate == _PQL_OUT_50HZ)))
	{
		if(lowdelay_state_chg_cnt > 1){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char mc_offset = 0;
				#if CONFIG_MC_8_BUFFER
				if(lowdelay_state_chg_cnt > 5)
					mc_offset = 5;
				else
					mc_offset = lowdelay_state_chg_cnt;
				#else
				if(lowdelay_state_chg_cnt > 3)
					mc_offset = 3;
				else
					mc_offset = lowdelay_state_chg_cnt;
				#endif
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_offset; // mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_offset;
			}
		}
		rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
	}
	else if((in_fmRate == _PQL_IN_24HZ) && (out_fmRate == _PQL_OUT_60HZ))
	{
		if(lowdelay_state_chg_cnt >= 4){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if CONFIG_MC_8_BUFFER
				unsigned char mc_i_offset[5] = 	{5,5,4,5,5}; // delay 3 frames
				unsigned char mc_p_offset[5] = 	{5,5,4,5,5};
				#else
				unsigned char mc_i_offset[5] = 	{3,3,2,3,3};
				unsigned char mc_p_offset[5] = 	{3,3,2,3,3};
				#endif
				unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
			}
			rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
		}
		else if(lowdelay_state_chg_cnt == 3){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if CONFIG_MC_8_BUFFER
				unsigned char mc_i_offset[5] = 	{4,4,3,4,4}; // delay 4 frames
				unsigned char mc_p_offset[5] = 	{4,4,3,4,4};
				#else
				unsigned char mc_i_offset[5] = 	{3,3,2,3,3};
				unsigned char mc_p_offset[5] = 	{3,3,2,3,3};
				#endif
				unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
			}
			rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
		}
		else if(lowdelay_state_chg_cnt == 2){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if CONFIG_MC_8_BUFFER
				unsigned char mc_i_offset[5] = 	{3,3,2,3,3}; // delay 5 frames
				unsigned char mc_p_offset[5] = 	{3,3,2,3,3};
				#else
				unsigned char mc_i_offset[5] = 	{2,2,1,2,2};
				unsigned char mc_p_offset[5] = 	{2,2,1,2,2};
				#endif
				unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
				phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
			}
			rtd_pr_memc_notice("[%s][%d][%x,%x][cnt,%d,]\n\r", __FUNCTION__, __LINE__, u16_k, phTable_DecInfoGen[u16_k].mc_i_offset, lowdelay_state_chg_cnt);
		}
	}

}

VOID FRC_decPhT_goldenGen_Proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned short u16_k;
	int i = 0;
	unsigned char in_is_3dLR  = (pParam->in3d_format == _PQL_IN_LR)? 1 : 0;
	unsigned int test_32322_BadEdit_en = 0;
	unsigned int MC_8_buffer_en = 0, log_en = 0, log_en2 = 0, NewLowDelay_en = 0;
//	unsigned char u1_YT_resize_chg_flg = 0, u1_hdmi_gamemode_chg_flg = 0;
	PQL_OUTPUT_FRAME_RATE  out_fmRate = s_pContext->_external_data._output_frameRate;
	PQL_INPUT_FRAME_RATE	in_fmRate = s_pContext->_external_data._input_frameRate;

	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 29, 29, &test_32322_BadEdit_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_60_reg, 30, 30, &log_en);
	ReadRegister(SOFTWARE3_SOFTWARE3_09_reg, 30, 30, &log_en2);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 0, &MC_8_buffer_en);
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 2, 2, &NewLowDelay_en);

//	u1_YT_resize_chg_flg = s_pContext->_output_filmDetectctrl.u1_YT_resize_chg_flg;
//	u1_hdmi_gamemode_chg_flg = s_pContext->_output_filmDetectctrl.u1_hdmi_gamemode_chg_flg;

	cadence_copy_change((FRC_CADENCE_ID)(pParam->u8_cadence_id), in_is_3dLR, &(pOutput->local_FRC_cadence));	//baibaibai cad_id sequence  to pOutput->local_FRC_cadence

	pOutput->u16_phT_stIdx  = pParam->u16_phT_stIdx;  // phase tabel start idx

	FRC_phTable_basicInfo_Gen(pParam, pOutput);
	FRC_phTable_IP_oft_Gen(pParam, pOutput);
	if(pOutput->u16_phT_endIdx_p1 > 255){
		rtd_pr_memc_notice("[%s]Invalid u16_phT_endIdx_p1:%d!!\n",__FUNCTION__,pOutput->u16_phT_endIdx_p1);
		pOutput->u16_phT_endIdx_p1 = 255;
	}
	for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
	{
		unsigned char  next_wrt_me1 = 0, next_wrt_me2 = 0, next_wrt_mc = 0;
		unsigned char  u8_cadence_id                 = pOutput->local_FRC_cadence.cad_id;
		phTable_DecInfoGen[u16_k].out_phase  = phTable_DecInfoGen[u16_k].out_phase + 1;
		phTable_DecInfoGen[u16_k].film_mode  = u8_cadence_id == _CAD_VIDEO? 0 : (u8_cadence_id == _CAD_22? 1 : (u8_cadence_id == _CAD_32? 2 : 3));
		phTable_DecInfoGen[u16_k].film_phase = (pParam->in3d_format == _PQL_IN_LR)? (phTable_DecInfoGen[u16_k].film_phase >> 1) : phTable_DecInfoGen[u16_k].film_phase;

		if(pOutput->u8_fracPh_denomi == 0){
			rtd_pr_memc_notice("[%s]Invalid:pOutput->u8_fracPh_denomi==0!\n",__FUNCTION__);
			pOutput->u8_fracPh_denomi = 1;
		}
		phTable_DecInfoGen[u16_k].me2_phase   = ((phTable_DecInfoGen[u16_k].frac_phase_nume << ME_PHASE_PRECISION) + (pOutput->u8_fracPh_denomi >> 1)) / pOutput->u8_fracPh_denomi;
		phTable_DecInfoGen[u16_k].mc_phase   = ((phTable_DecInfoGen[u16_k].frac_phase_nume << MC_PHASE_PRECISION) + (pOutput->u8_fracPh_denomi >> 1)) / pOutput->u8_fracPh_denomi;

		// bai 下个in 是否写新buffer

		for(i = 0; i < (pOutput->u16_phT_length); i++)
		{
			int abs_idx = u16_k - (unsigned short)(pOutput->u16_phT_stIdx);
			int ner_in = (abs_idx + i + 1)% pOutput->u16_phT_length + (unsigned short)(pOutput->u16_phT_stIdx);
			if (phTable_DecInfoGen[ner_in].inPhase_new == 1)
			{
				next_wrt_me1 = phTable_DecInfoGen[ner_in].me1_wrt_idx_oft;
#if 1  // k4lp
				next_wrt_me2 = phTable_DecInfoGen[ner_in].me1_wrt_idx_oft;
				if(MC_8_buffer_en)
					next_wrt_mc = phTable_DecInfoGen[ner_in].me1_wrt_idx_oft;
				else
					next_wrt_mc = phTable_DecInfoGen[ner_in].mc_wrt_idx_oft;
#else
				next_wrt_me2 = phTable_DecInfoGen[ner_in].me2_wrt_idx_oft;
#endif
				break;
			}

		}
		phTable_DecInfoGen[u16_k].me1_i_offset   = (phTable_DecInfoGen[u16_k].me1_i_offset + next_wrt_me1)%_PHASE_FRC_ME1_TOTAL_BUF_SIZE_;
		phTable_DecInfoGen[u16_k].me1_p_offset   = (phTable_DecInfoGen[u16_k].me1_p_offset + next_wrt_me1)%_PHASE_FRC_ME1_TOTAL_BUF_SIZE_;

		phTable_DecInfoGen[u16_k].me1_i_offset   = _PHASE_FRC_ME1_TOTAL_BUF_SIZE_ - 1 - phTable_DecInfoGen[u16_k].me1_i_offset;
		phTable_DecInfoGen[u16_k].me1_p_offset   = _PHASE_FRC_ME1_TOTAL_BUF_SIZE_ - 1 - phTable_DecInfoGen[u16_k].me1_p_offset;

#if 1  // k4lp
		phTable_DecInfoGen[u16_k].me2_i_offset   = (phTable_DecInfoGen[u16_k].me2_i_offset + next_wrt_me2)%_PHASE_FRC_ME1_TOTAL_BUF_SIZE_;
		phTable_DecInfoGen[u16_k].me2_p_offset   = (phTable_DecInfoGen[u16_k].me2_p_offset + next_wrt_me2)%_PHASE_FRC_ME1_TOTAL_BUF_SIZE_;

		phTable_DecInfoGen[u16_k].me2_i_offset   = _PHASE_FRC_ME1_TOTAL_BUF_SIZE_ - 1 - phTable_DecInfoGen[u16_k].me2_i_offset;
		phTable_DecInfoGen[u16_k].me2_p_offset   = _PHASE_FRC_ME1_TOTAL_BUF_SIZE_ - 1 - phTable_DecInfoGen[u16_k].me2_p_offset;

		if(MC_8_buffer_en){
			phTable_DecInfoGen[u16_k].mc_i_offset	 = phTable_DecInfoGen[u16_k].me2_i_offset;
			phTable_DecInfoGen[u16_k].mc_p_offset	 = phTable_DecInfoGen[u16_k].me2_p_offset;
		}else{
			phTable_DecInfoGen[u16_k].mc_i_offset	 = (phTable_DecInfoGen[u16_k].mc_i_offset + next_wrt_mc)%_PHASE_FRC_MC_TOTAL_BUF_SIZE_;
			phTable_DecInfoGen[u16_k].mc_p_offset	 = (phTable_DecInfoGen[u16_k].mc_p_offset + next_wrt_mc)%_PHASE_FRC_MC_TOTAL_BUF_SIZE_;

			phTable_DecInfoGen[u16_k].mc_i_offset	 = _PHASE_FRC_MC_TOTAL_BUF_SIZE_ - 1 - phTable_DecInfoGen[u16_k].mc_i_offset;
			phTable_DecInfoGen[u16_k].mc_p_offset	 = _PHASE_FRC_MC_TOTAL_BUF_SIZE_ - 1 - phTable_DecInfoGen[u16_k].mc_p_offset;
		}
#else
		phTable_DecInfoGen[u16_k].me2_i_offset   = (phTable_DecInfoGen[u16_k].me2_i_offset + next_wrt_me2)%_PHASE_FRC_ME2_TOTAL_BUF_SIZE_;
		phTable_DecInfoGen[u16_k].me2_p_offset   = (phTable_DecInfoGen[u16_k].me2_p_offset + next_wrt_me2)%_PHASE_FRC_ME2_TOTAL_BUF_SIZE_;

		phTable_DecInfoGen[u16_k].me2_i_offset   = _PHASE_FRC_ME2_TOTAL_BUF_SIZE_ - 1 - phTable_DecInfoGen[u16_k].me2_i_offset;
		phTable_DecInfoGen[u16_k].me2_p_offset   = _PHASE_FRC_ME2_TOTAL_BUF_SIZE_ - 1 - phTable_DecInfoGen[u16_k].me2_p_offset;
#endif

		phTable_DecInfoGen[u16_k].ppfv_offset    = _PHASE_FRC_PPFV_BUF_SIZE_ - 1 - phTable_DecInfoGen[u16_k].ppfv_offset;

		#if 1
		if(log_en||log_en2){
		rtd_pr_memc_emerg("[%s][%d][%d][%d,%d][%d][me1_wrt_idx_oft,%d,%d][mc_wrt_idx_oft,%d,%d][,%d,%d,%d,%d,%d,%d,][,%d,%d,][,%d,%d,][,%d,%d,%d,]", __FUNCTION__, __LINE__, u16_k,
			s_pContext->_external_data._input_frameRate, s_pContext->_external_data._output_frameRate, s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL],
			phTable_DecInfoGen[u16_k].me1_wrt_idx_oft, next_wrt_me1,
			phTable_DecInfoGen[u16_k].mc_wrt_idx_oft, next_wrt_mc,
			phTable_DecInfoGen[u16_k].me1_i_offset, phTable_DecInfoGen[u16_k].me1_p_offset, phTable_DecInfoGen[u16_k].me2_i_offset, phTable_DecInfoGen[u16_k].me2_p_offset,
			phTable_DecInfoGen[u16_k].mc_i_offset, phTable_DecInfoGen[u16_k].mc_p_offset, phTable_DecInfoGen[u16_k].ppfv_offset, phTable_DecInfoGen[u16_k].me1_last,
			phTable_DecInfoGen[u16_k].me2_phase, phTable_DecInfoGen[u16_k].mc_phase, pParam->u4_sys_N, pParam->u6_sys_M, pParam->u8_ph_sys_dly);
		}
		#endif
	}
	// 1N1M
	if(pParam->u4_sys_N == 1 && pParam->u6_sys_M == 1 && pParam->u8_cadence_id == _CAD_VIDEO)
	{
		phTable_DecInfoGen[0].me1_i_offset   = 3; // 2; // 
		phTable_DecInfoGen[0].me1_p_offset   = 4; // 3; // 

		phTable_DecInfoGen[0].me2_i_offset   = 2; // 1; // 
		phTable_DecInfoGen[0].me2_p_offset   = 3; // 2; // 
		#if 1  // k4lp
		if(MC_8_buffer_en){
			phTable_DecInfoGen[0].mc_i_offset   = phTable_DecInfoGen[0].me2_i_offset;// 2;
			phTable_DecInfoGen[0].mc_p_offset   = phTable_DecInfoGen[0].me2_p_offset;// 3;
		}
		else{
			phTable_DecInfoGen[0].mc_i_offset   = 1;// 2;
			phTable_DecInfoGen[0].mc_p_offset   = 2;// 3;
		}
		#endif
	}

if(MC_8_buffer_en){
	printk(KERN_INFO"[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
	if(pParam->u8_cadence_id == _CAD_VIDEO
		&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
	{
		unsigned short u16_k_tmp = 0;
		if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
			phTable_DecInfoGen[0].me1_i_offset   = 1; // 2;
			phTable_DecInfoGen[0].me1_p_offset   = 2; // 3;

			phTable_DecInfoGen[0].me2_i_offset   = 0; // 1;
			phTable_DecInfoGen[0].me2_p_offset   = 1; // 2;

			phTable_DecInfoGen[0].mc_i_offset   = phTable_DecInfoGen[0].me2_i_offset;// 2;
			phTable_DecInfoGen[0].mc_p_offset   = phTable_DecInfoGen[0].me2_p_offset;// 3;
		}
		else if(pParam->u4_sys_N == 1 && pParam->u6_sys_M == 2){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[2] = {1,2}; //{2,3};
				unsigned char me1_p_offset[2] = {2,3}; //{3,4};
				unsigned char me2_i_offset[2] = {0,0}; //{1,1};
				unsigned char me2_p_offset[2] = {1,1}; //{2,2};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp]; // 3;
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp]; // 4;

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];// 2;
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];// 3;

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];// 2;
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];// 3;
				#endif
			}
		}
	}
	else if(pParam->u8_cadence_id == _CAD_22
		&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
	{
		unsigned short u16_k_tmp = 0;
		if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[2] = {3,4};
				unsigned char me1_p_offset[2] = {4,5};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];
				#endif
			}	
		}
		else{
			#if 0
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char me1_i_offset[4] = {3,3,2,4};
				unsigned char me1_p_offset[4] = {4,4,3,5};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];
			}	
			#endif
		}
	}
	else if(pParam->u8_cadence_id == _CAD_32
		&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
	{
		unsigned short u16_k_tmp = 0;
		if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[5] = {5,5,4,5,4}; //{2,2,2,2,2};
				unsigned char me1_p_offset[5] = {6,6,5,6,5}; //{3,3,3,3,3};
				unsigned char me2_i_offset[5] = {1,3,2,1,2}; //{2,2,2,2,2};
				unsigned char me2_p_offset[5] = {3,5,4,3,4}; //{3,3,3,3,3};
				unsigned char me2_phase[5] = {38,0,26,51,13}; //{2,2,2,2,2};
				unsigned char mc_phase[5] = {77,0,51,102,26}; //{3,3,3,3,3};
				unsigned char ppfv_offset[5] = {0,1,1,0,1};
				unsigned char me1_last[5] = {0,0,1,0,1};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

				phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
			}	
		}
		else{
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[10] = {3,5,5,5,4,4,5,5,4,4};
				unsigned char me1_p_offset[10] = {5,6,6,6,5,5,6,6,6,6}; // {5,6,6,6,6,6,6,6,6,6}; //
				unsigned char me2_i_offset[10] = {1,1,3,3,2,2,1,3,2,2}; //{2,2,2,2,2};
				unsigned char me2_p_offset[10] = {3,3,5,5,4,4,3,5,4,4}; //{3,3,3,3,3};
				unsigned char me2_phase[10] = {38,51,0,13,26,38,51,0,13,26}; //{2,2,2,2,2};
				unsigned char mc_phase[10] = {77,102,0,26,51,77,102,0,26,51}; //{3,3,3,3,3};
				unsigned char ppfv_offset[10] = {1,0,1,1,1,1,0,1,1,1};
				unsigned char me1_last[10] = {1,0,0,0,0,1,0,0,0,0};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

				phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#else
				unsigned char me1_i_offset[10] = {3,5,5,5,4,4,5,5,4,4}; //{3,5,5,5,4,4,5,5,4,4};
				unsigned char me1_p_offset[10] = {5,6,6,6,6,6,6,6,6,6}; //{4,6,6,6,5,5,6,6,5,5};
				unsigned char me2_i_offset[10] = {1,1,3,3,2,2,1,3,2,2}; //{2,2,2,2,2};
				unsigned char me2_p_offset[10] = {3,3,5,5,4,4,3,5,4,4}; //{3,3,3,3,3};
				unsigned char me2_phase[10] = {38,51,0,13,26,38,51,0,13,26}; //{2,2,2,2,2};
				unsigned char mc_phase[10] = {77,102,0,26,51,77,102,0,26,51}; //{3,3,3,3,3};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];
				#endif
			}	
		}
	}
	else if(pParam->u8_cadence_id == _CAD_32322 && test_32322_BadEdit_en == 1
		&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
	{
		unsigned short u16_k_tmp = 0;
		if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char me1_i_offset[12] = {3,3,4,3,4,3,3,4,3,4,3,4}; //{2,2,2,2,2};
				unsigned char me1_p_offset[12] = {5,5,6,5,6,5,5,6,5,6,5,6}; //{3,3,3,3,3};
				u16_k_tmp = u16_k -pOutput->u16_phT_stIdx;
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];
			}
		}
		else{
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char me1_i_offset[24] = {3,3,3,3,4,4,3,3,2,4,3,3,3,3,4,4,3,3,2,4,3,3,2,4}; //{2,2,2,2,2};
				unsigned char me1_p_offset[24] = {5,5,5,5,6,6,5,5,4,6,5,5,5,5,6,6,5,5,4,6,5,5,4,6}; //{3,3,3,3,3};
				u16_k_tmp = u16_k -pOutput->u16_phT_stIdx;
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];
			}
		}
 	}
	else if(pParam->u8_cadence_id == _CAD_2224
		&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
	{
		unsigned short u16_k_tmp = 0;
		if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[10] = {5, 4, 3, 4, 3, 4, 3, 3, 5, 4}; //{2,2,2,2,2};
				unsigned char me1_p_offset[10] = {6, 6, 5, 6, 5, 6, 5, 5, 6, 5}; //{3,3,3,3,3};
				unsigned char me2_i_offset[10] = {1, 2, 1, 0, 1, 0, 1, 1, 1, 2}; //{2,2,2,2,2};
				unsigned char me2_p_offset[10] = {3, 4, 3, 2, 3, 2, 3, 3, 3, 4}; //{3,3,3,3,3};
				unsigned char me2_phase[10] = {38,0,26,51,13,38,0,26,51,13}; //{2,2,2,2,2};
				unsigned char mc_phase[10] = {77,0,51,102,26,77,0,51,102,26}; //{3,3,3,3,3};
				unsigned char ppfv_offset[10] = {0,1,1,0,1,0,1,1,0,1};
				unsigned char me1_last[10] = {0,0,1,0,1,0,0,1,0,1};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

				phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
			}	
		}
		else{
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[20] = {3, 5, 4, 4, 3, 3, 4, 4, 3, 3, 2, 4, 3, 3, 3, 3, 5, 5, 4, 4}; //{2,2,2,2,2};
				unsigned char me1_p_offset[20] = {5, 6, 5, 6, 5, 5, 5, 5, 5, 5, 4, 5, 4, 5, 5, 5, 6, 6, 5, 5}; //{3,3,3,3,3};
				unsigned char me2_i_offset[20] = {1, 1, 2, 2, 1, 1, 0, 2, 1, 1, 0, 0, 1, 1, 1, 1, 1, 3, 2, 2}; //{2,2,2,2,2};
				unsigned char me2_p_offset[20] = {3, 3, 4, 4, 3, 3, 2, 4, 3, 3, 2, 2, 3, 3, 3, 3, 3, 5, 4, 4}; //{3,3,3,3,3};
				unsigned char me2_phase[20] = {38,51,0,13,26,38,51,0,13,26,38,51,0,13,26,38,51,0,13,26}; //{2,2,2,2,2};
				unsigned char mc_phase[20] = {77,102,0,26,51,77,102,0,26,51,77,102,0,26,51,77,102,0,26,51}; //{3,3,3,3,3};
				unsigned char ppfv_offset[20] = {1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1};
				unsigned char me1_last[20] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

				phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
			}	
		}
	}	
	else if(pParam->u8_cadence_id == _CAD_22224
		&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
	{
		unsigned short u16_k_tmp = 0;
		if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[12] = {5,4,5,4,3,4,3,4,3,3,5,4}; //{2,2,2,2,2};
				unsigned char me1_p_offset[12] = {6,6,6,6,5,5,5,5,5,5,6,5}; //{3,3,3,3,3};
				unsigned char me2_i_offset[12] = {1,2,1,2,1,0,1,0,1,1,1,2}; //{2,2,2,2,2};
				unsigned char me2_p_offset[12] = {3,4,3,4,3,2,3,2,3,3,3,4}; //{3,3,3,3,3};
				unsigned char me2_phase[12] = {48, 11, 37, 0, 27, 53, 16, 43, 5, 32, 59, 21}; //{2,2,2,2,2};
				unsigned char mc_phase[12] = {96, 21, 75, 0, 53, 107, 32, 85, 11, 64, 117, 43}; //{3,3,3,3,3};
				unsigned char ppfv_offset[12] = {0,1,0,1,1,0,1,0,1,1,0,1};
				unsigned char me1_last[12] = {0,1,0,0,1,0,1,0,0,1,0,1};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

				phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
			}	
		}
		else{
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[24] = {3, 5, 4, 4, 3, 5, 4, 4, 3, 3, 4, 4, 3, 3, 2, 4, 3, 3, 3, 3, 5, 5, 4, 4}; //{2,2,2,2,2};
				unsigned char me1_p_offset[24] = {5, 6, 5, 6, 5, 6, 5, 6, 5, 5, 5, 5, 5, 5, 4, 5, 4, 5, 5, 5, 6, 6, 6, 6}; //{3,3,3,3,3};
				unsigned char me2_i_offset[24] = {1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 0, 2, 1, 1, 0, 0, 1, 1, 1, 1, 1, 3, 2, 2}; //{2,2,2,2,2};
				unsigned char me2_p_offset[24] = {3, 3, 4, 4, 3, 3, 4, 4, 3, 3, 2, 4, 3, 3, 2, 2, 3, 3, 3, 3, 3, 5, 4, 4}; //{3,3,3,3,3};
				unsigned char me2_phase[24] = {48, 62, 11, 24, 38, 51, 0, 14, 27, 40, 54, 3, 16, 30, 43, 56, 6, 19, 32, 46, 59, 8, 22, 35}; //{2,2,2,2,2};
				unsigned char mc_phase[24] = {96, 123, 21, 48, 75, 101, 0, 27, 53, 80, 107, 5, 32, 59, 85, 112, 11, 37, 64, 91, 117, 16, 43, 69}; //{3,3,3,3,3};
				unsigned char ppfv_offset[24] = {1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1};
				unsigned char me1_last[24] = {1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

				phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
			}	
		}
	}
	else if(pParam->u8_cadence_id == _CAD_321
		&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
	{
		unsigned short u16_k_tmp = 0;
		if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[6] = {4,4,4,3,4,3}; //{2,2,2,2,2};
				unsigned char me1_p_offset[6] = {5,5,5,5,5,5}; //{3,3,3,3,3};
				unsigned char me2_i_offset[6] = {2,2,3,2,2,1}; //{2,2,2,2,2};
				unsigned char me2_p_offset[6] = {4,4,4,3,4,3}; //{3,3,3,3,3};
				unsigned char me2_phase[6] = {0,32,0,32,0,32}; //{2,2,2,2,2};
				unsigned char mc_phase[6] = {0,64,0,64,0,64}; //{3,3,3,3,3};
				unsigned char ppfv_offset[6] = {1,1,1,1,1,1};
				unsigned char me1_last[6] = {0,1,0,1,0,1};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

				phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
			}	
		}
		else{
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[12] = {4,4,4,5,4,4,3,5,4,4,3,5}; //{2,2,2,2,2};
				unsigned char me1_p_offset[12] = {5,5,5,6,5,5,5,6,5,5,5,6}; //{3,3,3,3,3};
				unsigned char me2_i_offset[12] = {2,2,2,2,3,3,2,2,2,2,1,1}; //{2,2,2,2,2};
				unsigned char me2_p_offset[12] = {4,4,4,4,4,4,3,3,4,4,3,3}; //{3,3,3,3,3};
				unsigned char me2_phase[12] = {0,16,32,48,0,16,32,48,0,16,32,48}; //{2,2,2,2,2};
				unsigned char mc_phase[12] = {0,32,64,96,0,32,64,96,0,32,64,96}; //{3,3,3,3,3};
				unsigned char ppfv_offset[12] = {1,1,1,0,1,1,1,0,1,1,1,0};
				unsigned char me1_last[12] = {0,0,1,0,0,0,1,0,0,0,1,0};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

				phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
			}	
		}
	}
	else if(pParam->u8_cadence_id == _CAD_11112
		&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
	{
		unsigned short u16_k_tmp = 0;
		if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[6] = {2,2,3,3,3,3}; //{2,2,2,2,2};
				unsigned char me1_p_offset[6] = {3,4,4,4,4,4}; //{3,3,3,3,3};
				unsigned char me2_i_offset[6] = {1,0,0,0,1,1}; //{2,2,2,2,2};
				unsigned char me2_p_offset[6] = {2,1,1,2,2,2}; //{3,3,3,3,3};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				#endif
			}	
		}
		else{
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				#if 1
				unsigned char me1_i_offset[12] = {2,2,2,2,3,3,3,3,2,3,2,3}; //{2,2,2,2,2};
				unsigned char me1_p_offset[12] = {3,3,3,4,4,4,4,4,3,4,3,4}; //{3,3,3,3,3};
				unsigned char me2_i_offset[12] = {1,1,0,1,0,1,0,2,1,1,1,1}; //{2,2,2,2,2};
				unsigned char me2_p_offset[12] = {2,2,1,2,1,3,2,3,2,2,2,2}; //{3,3,3,3,3};
				//unsigned char me2_phase[12] =  	 {5,32,59,21,48,11,37,0,27,53,16,43};
				//unsigned char mc_phase[12] = 	 {11,64,117,43,96,21,75,0,53,107,32,85};
				unsigned char me2_phase[12] =  	 {10,37,64,26,53,16,42,5,32,58,21,48};
				unsigned char mc_phase[12] = 	 {21,74,127,53,106,31,85,10,63,117,42,95};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];
				#endif
			}	
		}
	}
#if 1 //30->50
		else if(pParam->u8_cadence_id == _CAD_11i23
			&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
		{
			unsigned short u16_k_tmp = 0;
			if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					unsigned char me1_i_offset[25] = {5,4,5,4,5,4,5,4,5,4,3,4,3,4,3,4,3,4,3,4,3,4,3,5,4}; //{2,2,2,2,2};
					unsigned char me1_p_offset[25] = {6,5,6,5,6,5,6,5,6,5,4,5,4,5,4,5,4,5,4,5,4,5,4,6,5}; //{3,3,3,3,3};
					unsigned char me2_i_offset[25] = {1,2,1,2,1,2,1,2,1,2,1,0,1,0,1,0,1,0,1,0,1,0,1,1,2}; //{2,2,2,2,2};
					unsigned char me2_p_offset[25] = {3,4,3,4,3,4,3,4,3,4,3,2,3,2,3,2,3,2,3,2,3,2,3,3,4}; //{3,3,3,3,3};
					unsigned char me2_phase[25] =	 {44,10,41,8,38,5,36,3,33,0,31,61,28,59,26,56,23,54,20,51,18,49,15,46,13};
					unsigned char mc_phase[25] =	 {87,20,82,15,77,10,72,5,67,0,61,123,56,118,51,113,46,108,41,102,36,97,31,92,26};
					unsigned char ppfv_offset[25] =  {0,1,0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
					unsigned char me1_last[25] =	 {0,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1};

					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];
					
					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
				}	
			}
			else{
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					unsigned char me1_i_offset[50] = {3,3,4,4,3,3,4,4,3,3,4,4,3,3,4,4,3,3,4,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,3,3,4,4};
					unsigned char me1_p_offset[50] = {5,5,5,6,5,5,5,6,5,5,5,6,5,5,5,6,5,5,5,6,5,5,4,5,5,5,4,5,5,5,4,5,5,5,4,5,5,5,4,5,5,5,4,5,5,5,5,5,5,6};
					unsigned char me2_i_offset[50] = {1,1,0,2,1,1,0,2,1,1,0,2,1,1,0,2,1,1,0,2,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,2};	
					unsigned char me2_p_offset[50] = {3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,2,3,3,2,2,3,3,2,2,3,3,2,2,3,3,2,2,3,3,2,2,3,3,3,3,2,4};
					unsigned char me2_phase[50] =	 {28,44,59,10,26,41,56,8,23,38,54,5,20,36,51,3,18,33,49,0,15,31,46,61,13,28,44,59,10,26,41,56,8,23,38,54,5,20,36,51,3,18,33,49,0,15,31,46,61,13};
					unsigned char mc_phase[50] =	 {56,87,118,20,51,82,113,15,46,77,108,10,41,72,102,5,36,67,97,0,31,61,92,123,26,56,87,118,20,51,82,113,15,46,77,108,10,41,72,102,5,36,67,97,0,31,61,92,123,26};
					unsigned char ppfv_offset[50] =  {1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1};
					unsigned char me1_last[50] =	 {0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0};
	
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];
					
					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
				}	
			}
		}
#endif
#if 1 //24->50
		else if(pParam->u8_cadence_id == _CAD_1225_T2
			&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
		{
			unsigned short u16_k_tmp = 0;
			if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
			#if 0
					unsigned char me1_i_offset[6] = {2,2,3,3,3,3}; //{2,2,2,2,2};
					unsigned char me1_p_offset[6] = {3,4,4,4,4,4}; //{3,3,3,3,3};
					unsigned char me2_i_offset[6] = {1,0,0,0,1,1}; //{2,2,2,2,2};
					unsigned char me2_p_offset[6] = {2,1,1,2,2,2}; //{3,3,3,3,3};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
			#endif
				}	
			}
			else{
				
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
			#if 1
					unsigned char me1_i_offset[50] = {3,3,3,3,5,5,4,4,5,5,4,4,4,4,3,3,4,4,3,3,5,5,4,4,3,4,3,3,2,4,3,3,2,4,3,3,3,5,4,4,3,5,4,4,3,4,3,3,2,2};
					unsigned char me1_p_offset[50] = {5,5,5,5,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5,6,6,6,6,5,6,5,5,4,6,5,5,4,5,4,5,5,6,5,6,5,6,5,5,4,6,5,5,4,4};
					unsigned char me2_i_offset[50] = {0,1,1,1,1,3,2,2,1,3,2,2,1,3,2,2,1,2,1,1,1,3,2,2,1,1,2,2,1,1,1,1,0,0,1,1,1,1,2,2,1,1,2,2,1,1,2,2,1,1}; 
					unsigned char me2_p_offset[50] = {1,3,3,3,3,5,4,4,3,5,4,4,3,4,3,3,2,4,3,3,3,5,4,4,3,3,3,3,2,2,3,3,2,2,3,3,3,3,4,4,3,3,4,4,3,3,3,3,2,2};
					unsigned char me2_phase[50] =	 {61,13,28,44,59,10,26,41,56,8,23,38,54,5,20,36,51,3,18,33,49,0,15,31,46,61,13,28,44,59,10,26,41,56,8,23,38,54,5,20,36,51,3,18,33,49,0,15,31,46};
					unsigned char mc_phase[50] =	 {123,26,56,87,118,20,51,82,113,15,46,77,108,10,41,72,102,5,36,67,97,0,31,61,92,123,26,56,87,118,20,51,82,113,15,46,77,108,10,41,72,102,5,36,67,97,0,31,61,92};
					unsigned char ppfv_offset[50] =  {1,1,1,1,0,1,1,1,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2};
					unsigned char me1_last[50] =	 {0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1};
					//unsigned char me1_wrt_idx_offset[50] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0};
					//unsigned char mc_wrt_idx_offset[50] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0};

					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					//phTable_DecInfoGen[u16_k].me1_wrt_idx_oft = me1_wrt_idx_offset[u16_k_tmp];
					//phTable_DecInfoGen[u16_k].mc_wrt_idx_oft = mc_wrt_idx_offset[u16_k_tmp];
					
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];
					
					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];

				//WriteRegister(HARDWARE_HARDWARE_43_reg,19,19,1);//kphase detect length
				#endif
			}	
		}
	}
#endif

	/*
		else if(pParam->u8_cadence_id == _CAD_1112
			&& s_pContext->_external_data._input_frameRate == _PQL_IN_30HZ)
		{
			UINT16 u16_k_tmp = 0;
			if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					UBYTE me1_i_offset[10] = {3,3,3,3,4,4,3,4,3,4}; //{2,2,2,2,2};
					UBYTE me1_p_offset[10] = {4,4,5,5,5,5,4,5,4,5}; //{3,3,3,3,3};
					UBYTE me2_i_offset[10] = {2,2,1,2,1,2,1,1,2,2}; //{2,2,2,2,2};
					UBYTE me2_p_offset[10] = {3,3,2,3,2,4,3,3,3,3}; //{3,3,3,3,3};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
				#endif
				}	
			}
			else{
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					UBYTE me1_i_offset[12] = {2,2,2,2,3,3,3,3,2,3,2,3}; //{2,2,2,2,2};
					UBYTE me1_p_offset[12] = {3,3,3,4,4,4,4,4,3,4,3,4}; //{3,3,3,3,3};
					UBYTE me2_i_offset[12] = {1,1,0,1,0,1,0,2,1,1,1,1}; //{2,2,2,2,2};
					UBYTE me2_p_offset[12] = {2,2,1,2,1,3,2,3,2,2,2,2}; //{3,3,3,3,3};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
				#endif
				}
			}
		}
		else if(pParam->u8_cadence_id == _CAD_11112
			&& s_pContext->_external_data._input_frameRate == _PQL_IN_30HZ)
		{
			UINT16 u16_k_tmp = 0;
			if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					UBYTE me1_i_offset[12] = {3,3,3,3,4,4,4,4,3,4,3,4}; //{2,2,2,2,2};
					UBYTE me1_p_offset[12] = {4,4,5,5,5,5,5,5,4,5,4,5}; //{3,3,3,3,3};
					UBYTE me2_i_offset[12] = {2,2,1,2,1,2,1,3,2,2,2,2}; //{2,2,2,2,2};
					UBYTE me2_p_offset[12] = {3,3,2,3,2,4,3,4,3,3,3,3}; //{3,3,3,3,3};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
				#endif
				}	
			}
			else{
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					UBYTE me1_i_offset[12] = {2,2,2,2,3,3,3,3,2,3,2,3}; //{2,2,2,2,2};
					UBYTE me1_p_offset[12] = {3,3,3,4,4,4,4,4,3,4,3,4}; //{3,3,3,3,3};
					UBYTE me2_i_offset[12] = {1,1,0,1,0,1,0,2,1,1,1,1}; //{2,2,2,2,2};
					UBYTE me2_p_offset[12] = {2,2,1,2,1,3,2,3,2,2,2,2}; //{3,3,3,3,3};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
				#endif
				}	
			}
		}
	*/

#if 0
	else if(pParam->u8_cadence_id == _CAD_22 
		&& ((s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ && s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ) 
		||(s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ && s_pContext->_external_data._output_frameRate == _PQL_OUT_100HZ)))
	{
		for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
		{
			unsigned char me1_i_offset[4] = {4,5,4,5};
			unsigned char me1_p_offset[4] = {6,7,6,7};
			unsigned char me2_i_offset[4] = {2,2,2,2};
			unsigned char me2_p_offset[4] = {4,4,4,4};
			unsigned char mc_i_offset[4] = {0,0,0,0};
			unsigned char mc_p_offset[4] = {2,2,2,2};
			unsigned char u16_k_tmp = 0;
			u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
			phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp]; // 3;
			phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp]; // 4;

			phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];// 2;
			phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];// 3;

			phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[u16_k_tmp];// 2;
			phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[u16_k_tmp];// 3;

			phTable_DecInfoGen[u16_k].ppfv_offset    = 1;

			if(pParam->u8_cadence_id == _CAD_22){
//				phTable_DecInfoGen[u16_k].me2_phase   = (phTable_DecInfoGen[u16_k].me2_phase + 32)%64;
//				phTable_DecInfoGen[u16_k].mc_phase   = (phTable_DecInfoGen[u16_k].mc_phase + 64)%128;
			}
			#if 1
			rtd_pr_memc_notice("[%s][%d][%d][%d,%d][%d][%d,%d][,%d,%d,%d,%d,%d,%d,%d,%d,%d,]", __FUNCTION__, __LINE__, u16_k,
				s_pContext->_external_data._input_frameRate, s_pContext->_external_data._output_frameRate, s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL],
				phTable_DecInfoGen[u16_k].me1_wrt_idx_oft, phTable_DecInfoGen[u16_k].mc_wrt_idx_oft,
				phTable_DecInfoGen[u16_k].me1_i_offset, phTable_DecInfoGen[u16_k].me1_p_offset, phTable_DecInfoGen[u16_k].me2_i_offset, phTable_DecInfoGen[u16_k].me2_p_offset,
				phTable_DecInfoGen[u16_k].mc_i_offset, phTable_DecInfoGen[u16_k].mc_p_offset, phTable_DecInfoGen[u16_k].ppfv_offset,
				phTable_DecInfoGen[u16_k].me2_phase, phTable_DecInfoGen[u16_k].mc_phase);
			#endif
		}
	}
	else if(pParam->u8_cadence_id == _CAD_32 && (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ && s_pContext->_external_data._output_frameRate == _PQL_OUT_120HZ))
	{
		for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
		{
			unsigned char me1_i_offset[10] = {4,4,4,5,5,5,4,5,4,5};
			unsigned char me1_p_offset[10] = {6,6,6,6,7,7,6,6,6,7};
			unsigned char me2_i_offset[10] = {2,2,2,2,2,3,2,2,2,2};
			unsigned char me2_p_offset[10] = {4,4,4,4,4,5,4,4,4,4};
			unsigned char mc_i_offset[10] = {0,0,0,0,0,1,0,0,0,0};
			unsigned char mc_p_offset[10] = {2,2,2,2,2,3,2,2,2,2};
			unsigned char u16_k_tmp = 0;
			u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
			phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp]; // 3;
			phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp]; // 4;

			phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];// 2;
			phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];// 3;

			phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[u16_k_tmp];// 2;
			phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[u16_k_tmp];// 3;

			phTable_DecInfoGen[u16_k].ppfv_offset    = 1;

			phTable_DecInfoGen[u16_k].me2_phase   = (phTable_DecInfoGen[u16_k].me2_phase + 51)%64;
			phTable_DecInfoGen[u16_k].mc_phase   = (phTable_DecInfoGen[u16_k].mc_phase + 102)%128;
			#if 1
			rtd_pr_memc_notice("[%s][%d][%d][%d,%d][%d][%d,%d][,%d,%d,%d,%d,%d,%d,%d,%d,%d,]", __FUNCTION__, __LINE__, u16_k,
				s_pContext->_external_data._input_frameRate, s_pContext->_external_data._output_frameRate, s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL],
				phTable_DecInfoGen[u16_k].me1_wrt_idx_oft, phTable_DecInfoGen[u16_k].mc_wrt_idx_oft,
				phTable_DecInfoGen[u16_k].me1_i_offset, phTable_DecInfoGen[u16_k].me1_p_offset, phTable_DecInfoGen[u16_k].me2_i_offset, phTable_DecInfoGen[u16_k].me2_p_offset,
				phTable_DecInfoGen[u16_k].mc_i_offset, phTable_DecInfoGen[u16_k].mc_p_offset, phTable_DecInfoGen[u16_k].ppfv_offset,
				phTable_DecInfoGen[u16_k].me2_phase, phTable_DecInfoGen[u16_k].mc_phase);
			#endif
		}
	}
#endif
}
else{
	// 1N2M
	if(pParam->u4_sys_N == 1 && pParam->u6_sys_M == 2 && pParam->u8_cadence_id == _CAD_VIDEO
		&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
	{
		phTable_DecInfoGen[0].me1_i_offset   = 4; // 3;
		phTable_DecInfoGen[0].me1_p_offset   = 5; // 4;

		phTable_DecInfoGen[0].me2_i_offset   = 3;// 2;
		phTable_DecInfoGen[0].me2_p_offset   = 4;// 3;

		phTable_DecInfoGen[0].mc_i_offset   = 1;// 2;
		phTable_DecInfoGen[0].mc_p_offset   = 2;// 3;

		phTable_DecInfoGen[1].me1_i_offset   = 5; // 3;
		phTable_DecInfoGen[1].me1_p_offset   = 6; // 4;

		phTable_DecInfoGen[1].me2_i_offset   = 3;// 2;
		phTable_DecInfoGen[1].me2_p_offset   = 4;// 3;

		phTable_DecInfoGen[1].mc_i_offset   = 1;// 2;
		phTable_DecInfoGen[1].mc_p_offset   = 2;// 3;
	}
#if 1 //30->50
		else if(pParam->u8_cadence_id == _CAD_11i23
			&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
		{
			unsigned short u16_k_tmp = 0;
			if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 0
					unsigned char me1_i_offset[6] = {2,2,3,3,3,3}; //{2,2,2,2,2};
					unsigned char me1_p_offset[6] = {3,4,4,4,4,4}; //{3,3,3,3,3};
					unsigned char me2_i_offset[6] = {1,0,0,0,1,1}; //{2,2,2,2,2};
					unsigned char me2_p_offset[6] = {2,1,1,2,2,2}; //{3,3,3,3,3};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
				#endif
				}	
			}
			else{
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					unsigned char me1_i_offset[50] = {3,3,4,4,3,3,4,4,3,3,4,4,3,3,4,4,3,3,4,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,3,3,4,4};
					unsigned char me1_p_offset[50] = {5,5,5,6,5,5,5,6,5,5,5,6,5,5,5,6,5,5,5,6,5,5,4,5,5,5,4,5,5,5,4,5,5,5,4,5,5,5,4,5,5,5,4,5,5,5,5,5,5,6};
					unsigned char me2_i_offset[50] = {1,1,0,2,1,1,0,2,1,1,0,2,1,1,0,2,1,1,0,2,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,2};	
					unsigned char me2_p_offset[50] = {3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,4,3,3,2,2,3,3,2,2,3,3,2,2,3,3,2,2,3,3,2,2,3,3,2,2,3,3,3,3,2,4};
					unsigned char me2_phase[50] =	 {28,44,59,10,26,41,56,8,23,38,54,5,20,36,51,3,18,33,49,0,15,31,46,61,13,28,44,59,10,26,41,56,8,23,38,54,5,20,36,51,3,18,33,49,0,15,31,46,61,13};
					unsigned char mc_phase[50] =	 {56,87,118,20,51,82,113,15,46,77,108,10,41,72,102,5,36,67,97,0,31,61,92,123,26,56,87,118,20,51,82,113,15,46,77,108,10,41,72,102,5,36,67,97,0,31,61,92,123,26};
					unsigned char ppfv_offset[50] =  {1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1};
					unsigned char me1_last[50] =	 {0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0};
	
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];
					
					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
				}	
			}
		}
#endif
#if 1 //24->50
		else if(pParam->u8_cadence_id == _CAD_1225_T2
			&& (s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ || s_pContext->_external_data._input_frameRate == _PQL_IN_50HZ))
		{
			unsigned short u16_k_tmp = 0;
			if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
			#if 0
					unsigned char me1_i_offset[6] = {2,2,3,3,3,3}; //{2,2,2,2,2};
					unsigned char me1_p_offset[6] = {3,4,4,4,4,4}; //{3,3,3,3,3};
					unsigned char me2_i_offset[6] = {1,0,0,0,1,1}; //{2,2,2,2,2};
					unsigned char me2_p_offset[6] = {2,1,1,2,2,2}; //{3,3,3,3,3};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
			#endif
				}	
			}
			else{
				
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
			#if 1
					unsigned char me1_i_offset[50] = {3,3,3,3,5,5,4,4,5,5,4,4,4,4,3,3,4,4,3,3,5,5,4,4,3,4,3,3,2,4,3,3,2,4,3,3,3,5,4,4,3,5,4,4,3,4,3,3,2,2};
					unsigned char me1_p_offset[50] = {5,5,5,5,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5,6,6,6,6,5,6,5,5,4,6,5,5,4,5,4,5,5,6,5,6,5,6,5,5,4,6,5,5,4,4};
					unsigned char me2_i_offset[50] = {0,1,1,1,1,3,2,2,1,3,2,2,1,3,2,2,1,2,1,1,1,3,2,2,1,1,2,2,1,1,1,1,0,0,1,1,1,1,2,2,1,1,2,2,1,1,2,2,1,1}; 
					unsigned char me2_p_offset[50] = {1,3,3,3,3,5,4,4,3,5,4,4,3,4,3,3,2,4,3,3,3,5,4,4,3,3,3,3,2,2,3,3,2,2,3,3,3,3,4,4,3,3,4,4,3,3,3,3,2,2};
					unsigned char me2_phase[50] =	 {61,13,28,44,59,10,26,41,56,8,23,38,54,5,20,36,51,3,18,33,49,0,15,31,46,61,13,28,44,59,10,26,41,56,8,23,38,54,5,20,36,51,3,18,33,49,0,15,31,46};
					unsigned char mc_phase[50] =	 {123,26,56,87,118,20,51,82,113,15,46,77,108,10,41,72,102,5,36,67,97,0,31,61,92,123,26,56,87,118,20,51,82,113,15,46,77,108,10,41,72,102,5,36,67,97,0,31,61,92};
					unsigned char ppfv_offset[50] =  {1,1,1,1,0,1,1,1,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2};
					unsigned char me1_last[50] =	 {0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1};
					//unsigned char me1_wrt_idx_offset[50] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0};
					//unsigned char mc_wrt_idx_offset[50] = {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0};

					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					//phTable_DecInfoGen[u16_k].me1_wrt_idx_oft = me1_wrt_idx_offset[u16_k_tmp];
					//phTable_DecInfoGen[u16_k].mc_wrt_idx_oft = mc_wrt_idx_offset[u16_k_tmp];
					
					phTable_DecInfoGen[u16_k].me1_i_offset	 = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset	 = me1_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_i_offset	 = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset	 = me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].mc_i_offset	= me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset	= me2_p_offset[u16_k_tmp];
	
					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];
					
					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];

				//WriteRegister(HARDWARE_HARDWARE_43_reg,19,19,1);//kphase detect length
			#endif
				}	
			}
		}
#endif

	// 2N5M
	if((s_pContext->_external_data._input_frameRate == _PQL_IN_24HZ) && pParam->u8_cadence_id == _CAD_VIDEO  && (VR360_en == 1))
	//if((s_pContext->_external_data._input_frameRate == _PQL_IN_24HZ) && pParam->u8_cadence_id == _CAD_VIDEO)
	{
		unsigned short u16_k = 0, u16_k_tmp = 0;
		for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
		{
			unsigned char mc_i_offset[5] = {1,1,0,1,1}; //{2,2,2,2,2};
			unsigned char mc_p_offset[5] = {2,2,1,2,2}; //{3,3,3,3,3};

			u16_k_tmp = u16_k -pOutput->u16_phT_stIdx;

			phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[u16_k_tmp];
			phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[u16_k_tmp];
		}
 	}

	// 32322
	if(pParam->u8_cadence_id == _CAD_32322 && test_32322_BadEdit_en == 1)
	{
		unsigned short u16_k = 0, u16_k_tmp = 0;
		if(s_pContext->_external_data._output_frameRate <= _PQL_OUT_60HZ){
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char me1_i_offset[12] = {3,3,4,3,4,3,3,4,3,4,3,4}; //{2,2,2,2,2};
				unsigned char me1_p_offset[12] = {5,5,6,5,5,5,5,5,5,5,5,5}; //;

				u16_k_tmp = u16_k -pOutput->u16_phT_stIdx;

				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];
			}
		}
		else{
			for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char me1_i_offset[24] = {3,3,3,3,4,4,3,3,2,4,3,3,3,3,4,4,3,3,2,4,3,3,2,4}; //{2,2,2,2,2};
				unsigned char me1_p_offset[24] = {5,5,5,5,6,6,5,5,4,5,5,5,5,5,5,5,5,5,4,5,5,5,4,5}; //{5,5,5,5,6,6,5,5,4,5,5,5,5,5,5,5,5,5,4,5,5,5,4,5};

				u16_k_tmp = u16_k -pOutput->u16_phT_stIdx;

				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];
			}
		}
 	}

	// 1N2M
	if(pParam->u4_sys_N == 1 && pParam->u6_sys_M == 2 && pParam->u8_cadence_id == _CAD_VIDEO)
	{
		unsigned char me1_i_offset[2] = {3,4};
		unsigned char me1_p_offset[2] = {4,5};
		unsigned char me2_i_offset[2] = {2,2};
		unsigned char me2_p_offset[2] = {3,3};
		unsigned char mc_i_offset[2] = {2,2};
		unsigned char mc_p_offset[2] = {3,3};
		unsigned char ppfv_offset[2] = {1,0};
		unsigned char u8_i = 0;
		for(u8_i = 0; u8_i < 2; u8_i++){
			phTable_DecInfoGen[u8_i].me1_i_offset   = me1_i_offset[u8_i];
			phTable_DecInfoGen[u8_i].me1_p_offset   = me1_p_offset[u8_i];

			phTable_DecInfoGen[u8_i].me2_i_offset   = me2_i_offset[u8_i];
			phTable_DecInfoGen[u8_i].me2_p_offset   = me2_p_offset[u8_i];

			phTable_DecInfoGen[u8_i].mc_i_offset   = mc_i_offset[u8_i];
			phTable_DecInfoGen[u8_i].mc_p_offset   = mc_p_offset[u8_i];

			phTable_DecInfoGen[u8_i].ppfv_offset   = ppfv_offset[u8_i];
		}
	}
}

	for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
	{
		#if 1
		if(log_en){
		rtd_pr_memc_notice("[%s][%d][%d][%d,%d][%d][%d,%d][,%d,%d,%d,%d,%d,%d,][,%d,%d,][,%d,%d,][,%d,%d,%d,]", __FUNCTION__, __LINE__, u16_k,
			s_pContext->_external_data._input_frameRate, s_pContext->_external_data._output_frameRate, s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL],
			phTable_DecInfoGen[u16_k].me1_wrt_idx_oft, phTable_DecInfoGen[u16_k].mc_wrt_idx_oft,
			phTable_DecInfoGen[u16_k].me1_i_offset, phTable_DecInfoGen[u16_k].me1_p_offset, phTable_DecInfoGen[u16_k].me2_i_offset, phTable_DecInfoGen[u16_k].me2_p_offset,
			phTable_DecInfoGen[u16_k].mc_i_offset, phTable_DecInfoGen[u16_k].mc_p_offset, phTable_DecInfoGen[u16_k].ppfv_offset, phTable_DecInfoGen[u16_k].me1_last,
			phTable_DecInfoGen[u16_k].me2_phase, phTable_DecInfoGen[u16_k].mc_phase, pParam->u4_sys_N, pParam->u6_sys_M, pParam->u8_ph_sys_dly);
		}
		#endif
	}	
#if 0
	// 1N4M
	if(pParam->u4_sys_N == 1 && pParam->u6_sys_M == 4 && pParam->u8_cadence_id == _CAD_VIDEO)
	{
		unsigned char me1_i_offset[4] = {2, 2, 2, 3};
		unsigned char me1_p_offset[4] = {3, 3, 3, 4};
		unsigned char me2_i_offset[4] = {1, 1, 1, 1};
		unsigned char me2_p_offset[4] = {2, 2, 2, 2};
		unsigned char ppfv_offset[4] = {1, 1, 1, 0};
		unsigned char u8_i = 0;
		for(u8_i = 0; u8_i < 4; u8_i++){
			phTable_DecInfoGen[u8_i].me1_i_offset   = me1_i_offset[u8_i];
			phTable_DecInfoGen[u8_i].me1_p_offset   = me1_p_offset[u8_i];

			phTable_DecInfoGen[u8_i].me2_i_offset   = me2_i_offset[u8_i];
			phTable_DecInfoGen[u8_i].me2_p_offset   = me2_p_offset[u8_i];
			phTable_DecInfoGen[u8_i].ppfv_offset   = ppfv_offset[u8_i];
		}
	}
#endif

	//-----Jerry_0811_game mode lowdeay flow
	if(s_pContext->_output_frc_phtable.u1_lowdelay_regen == 1 /*&& (pParam->u8_cadence_id == _CAD_VIDEO)*/){
		rtd_pr_memc_emerg("$$$   no delay in forcebg hi_02_LowDelay_PhTable_Gen_(%d)   $$$\n\r$$$   (in, out) = (%d, %d)   $$$\n\r", LowDelayGen_cnt,  s_pContext->_external_data._input_frameRate, s_pContext->_external_data._output_frameRate);
		FRC_phTable_LowDelay_Gen_Video(pParam, pOutput, u16_k, s_pContext->_external_data._input_frameRate, s_pContext->_external_data._output_frameRate, s_pContext->_output_frc_phtable.u1_lowdelaystate);
		LowDelay_regen_chk = 1;
	}
	else{
		lowdelay_state = 0;
		rtd_pr_memc_emerg("$$$   [PhTable_AutoGen][%d,%d,%x]   $$$\n\r", 
		s_pContext->_output_frc_phtable.u1_generate_video_en, s_pContext->_output_frc_phtable.u1_generate_film_en, rtd_inl(SOFTWARE1_SOFTWARE1_08_reg));
	}
	//-----

	// YT_resize_gen
	if(lowdelay_state == 0 && lowdelay_state_pre != 0){
		lowdelay_state_chg_cnt = 7;
	}

	if(lowdelay_state_chg_cnt > 0){
		lowdelay_state_chg_flg = 1;
		lowdelay_state_chg_cnt--;
	}
	else{
		lowdelay_state_chg_flg = 0;
	}

	#if 1
	if(lowdelay_state_chg_flg && pParam->u8_cadence_id == _CAD_VIDEO){
		FRC_decPhT_YT_ResizeGen_Proc(pParam, pOutput);
	}
	#else
	if(u1_YT_resize_chg_flg){
		FRC_decPhT_YT_ResizeGen_Proc(pParam, pOutput);
	}
	
	// New_game_mode_gen
	if(NewLowDelay_en){
		FRC_phTable_NewLowDelay_Gen(pParam, pOutput, u16_k, in_fmRate, out_fmRate, pParam->u8_cadence_id);
	}
	#endif

	// New_game_mode_gen
	if(NewLowDelay_en){
		FRC_phTable_NewLowDelay_Gen(pParam, pOutput, u16_k, in_fmRate, out_fmRate, pParam->u8_cadence_id);
	}
	//

	lowdelay_state_pre = lowdelay_state;
}

extern unsigned char MEMC_Lib_get_Input_Src_Type(VOID);
extern unsigned char is_DTV_flag_get(void);
extern unsigned char MEMC_Lib_get_Adaptive_Stream_Flag(VOID);
extern unsigned char MEMC_Lib_get_DisplayMode_Src(VOID);

//extern unsigned char Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(VOID);
extern unsigned char drv_memory_get_vdec_direct_low_latency_mode(void);

VOID FRC_phTable_LowDelay_Gen_Video(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput, unsigned short u16_k , PQL_INPUT_FRAME_RATE in_fmRate, PQL_OUTPUT_FRAME_RATE out_fmRate, unsigned char u1_lowdelaystate)
{
	_PQLPARAMETER *s_LowDelay = GetPQLParameter();
	unsigned char u1_low_delay_ctrl1 = s_LowDelay->_param_read_comreg.u1_low_delay_test01;
	const _PQLCONTEXT     *s_pContext = GetPQLContext();
	unsigned short u16_MEMCMode = s_pContext->_external_info.u16_MEMCMode;
	unsigned char u1_adaptivestream_flag = MEMC_Lib_get_Adaptive_Stream_Flag();
	unsigned char u1_callback_subscibe = '\0';
	unsigned char u1_vdec_direct_lowdelay_flag = drv_memory_get_vdec_direct_low_latency_mode();

#ifdef CONFIG_MEDIA_SUPPORT
#ifdef IC_V4L2_Only  //Antv not support v4l2
	u1_callback_subscibe = vsc_get_main_win_apply_done_event_subscribe();
#else
	u1_callback_subscibe = 0;
#endif
#else
	u1_callback_subscibe = 0;

#endif
	rtd_pr_memc_emerg("[%s][%d][%d][FrmRate=,%d,%d,][,%d,%d,][,%d,%d,][vdec_direct_lowdelay,%d,]\n\r",
	__FUNCTION__, __LINE__, u1_lowdelaystate, in_fmRate, out_fmRate, u16_MEMCMode, u1_adaptivestream_flag,
	MEMC_Lib_get_DisplayMode_Src(), Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag(), u1_vdec_direct_lowdelay_flag);


	if(u1_lowdelaystate == 1 && (VR360_en == 0)){
		if((((in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_120HZ)) || ((in_fmRate == _PQL_IN_50HZ) && (out_fmRate == _PQL_OUT_100HZ))) && (pOutput->u16_phT_endIdx_p1 - pOutput->u16_phT_stIdx == 2)){
			if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 1)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[2] = 	{6,6};
					unsigned char mc_p_offset[2] = 	{6,6};
					#else
					unsigned char mc_i_offset[2] = 	{3,3};
					unsigned char mc_p_offset[2] = 	{3,3};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 0)){
				//MEMC_Lib_set_LineMode_flag(1);
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[2] = 	{5,6};
					unsigned char mc_p_offset[2] = 	{5,6};
					#else
					unsigned char mc_i_offset[2] = 	{2,2};
					unsigned char mc_p_offset[2] = 	{2,2};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(u1_low_delay_ctrl1 == 0 && MEMC_Lib_get_DisplayMode_Src() == 2/*VSC_INPUTSRC_HDMI*/ && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1){
				lowdelay_state = 1;

				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[2] = 	{3,4};
					unsigned char me1_p_offset[2] = 	{5,6};
					unsigned char me2_i_offset[2] = 	{3,4};
					unsigned char me2_p_offset[2] = 	{5,6};
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[2] = 	{6,6};
					unsigned char mc_p_offset[2] = 	{6,6};
					#else
					unsigned char mc_i_offset[2] = 	{4,4}; // {3,3};
					unsigned char mc_p_offset[2] = 	{4,4}; // {3,3}; // jerry_wang, for the case "low delay + real cinema on" (K-12267) //{3,4} will cause transient noise from game mode to HDR-effect mode (K-12340)
					#endif
					unsigned char ppfv_offset[2] = 	{1,1};
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[index_offset];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[index_offset];

					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];

					phTable_DecInfoGen[u16_k].ppfv_offset    = ppfv_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(u1_low_delay_ctrl1 == 0 && (Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1 || u1_vdec_direct_lowdelay_flag)){
				lowdelay_state = 1;

				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[2] = 	{5,6};
					unsigned char mc_p_offset[2] = 	{5,6};
					#else
					unsigned char mc_i_offset[2] = 	{3,3};
					unsigned char mc_p_offset[2] = 	{3,3}; // jerry_wang, for the case "low delay + real cinema on" (K-12267) //{3,4} will cause transient noise from game mode to HDR-effect mode (K-12340)
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else{
				lowdelay_state = 0;
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
		}
		else if((in_fmRate == _PQL_IN_24HZ) && (out_fmRate == _PQL_OUT_120HZ) && (pOutput->u16_phT_endIdx_p1 - pOutput->u16_phT_stIdx == 10)){
			if(u1_adaptivestream_flag == 1){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[10] = {5,5,5,5,6,5,5,5,5,6};
					unsigned char mc_p_offset[10] = {5,5,5,5,6,5,5,5,5,6};
					#else
					unsigned char mc_i_offset[10] = 	{3,3,3,3,3,3,3,3,3,3};
					unsigned char mc_p_offset[10] = 	{3,3,3,3,3,3,3,3,3,3};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(u1_adaptivestream_flag == 0 && Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1){
				lowdelay_state = 1;

				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[10] = {5,5,5,5,6,5,5,5,5,6};
					unsigned char mc_p_offset[10] = {5,5,5,5,6,5,5,5,5,6};
					#else
					unsigned char mc_i_offset[10] = 	{2,2,2,2,2,2,2,2,2,2};
					unsigned char mc_p_offset[10] = 	{2,2,2,2,2,2,2,2,2,2};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(u1_low_delay_ctrl1 == 0 && (Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1 || u1_vdec_direct_lowdelay_flag)){
				lowdelay_state = 1;

				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[10] = {5,5,5,5,6,5,5,5,5,6};
					unsigned char mc_p_offset[10] = {5,5,5,5,6,5,5,5,5,6};
					#else
					unsigned char mc_i_offset[10] = 	{3,3,3,3,3,3,3,3,3,3};//{3,3,3,3,3,3,3,3,3,3};
					unsigned char mc_p_offset[10] = 	{3,3,3,3,3,3,3,3,3,3};//{4,4,4,4,5,4,4,4,4,5}; // jerry_wang, K-12267, for the case "low delay + real cinema on"
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else{
				lowdelay_state = 0;
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
		}
		else if((((in_fmRate == _PQL_IN_30HZ) && (out_fmRate == _PQL_OUT_120HZ)) || (((in_fmRate == _PQL_IN_25HZ) && (out_fmRate == _PQL_OUT_100HZ)))) && (pOutput->u16_phT_endIdx_p1 - pOutput->u16_phT_stIdx == 4)){
			if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 1)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[4] = {5,5,5,6}; // {5,5,6,6};
					unsigned char mc_p_offset[4] = {5,5,5,6}; // {5,5,6,6};
					#else
					unsigned char mc_i_offset[4] = 	{3,3,3,3};
					unsigned char mc_p_offset[4] = 	{3,3,3,3};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 0)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[4] = {5,5,5,6}; // {5,5,6,6};
					unsigned char mc_p_offset[4] = {5,5,5,6}; // {5,5,6,6};
					#else
					unsigned char mc_i_offset[4] = 	{2,2,2,2};
					unsigned char mc_p_offset[4] = 	{2,2,2,2};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(u1_low_delay_ctrl1 == 0 && (Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1 || u1_vdec_direct_lowdelay_flag)){
				lowdelay_state = 1;

				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[4] = {5,5,5,6}; // {5,5,6,6};
					unsigned char mc_p_offset[4] = {5,5,5,6}; // {5,5,6,6};
					#else
					unsigned char mc_i_offset[4] = 	{3,3,3,3};//{3,3,3,3};
					unsigned char mc_p_offset[4] = 	{3,3,3,3};//{4,4,4,4};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else{
				lowdelay_state = 0;
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
		}
		else if((in_fmRate == _PQL_IN_24HZ) && (out_fmRate == _PQL_OUT_60HZ)){
			if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 1)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[5] = {5,5,4,5,5};
					unsigned char mc_p_offset[5] = {5,5,4,5,5};
					#else
					unsigned char mc_i_offset[5] = 	{3,3,3,3,3};
					unsigned char mc_p_offset[5] = 	{3,3,3,3,3};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 0)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[5] = {5,5,4,5,5};
					unsigned char mc_p_offset[5] = {5,5,4,5,5};
					#else
					unsigned char mc_i_offset[5] = 	{2,2,2,2,2};
					unsigned char mc_p_offset[5] = 	{2,2,2,2,2};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(u1_low_delay_ctrl1 == 0 && (Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1 || u1_vdec_direct_lowdelay_flag)){
				lowdelay_state = 1;

				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if 1
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[5] = {5,6,5,5,6};
					unsigned char mc_p_offset[5] = {5,6,5,5,6};
					#else
					unsigned char mc_i_offset[5] =  	{2,2,2,2,2};
					unsigned char mc_p_offset[5] =  	{3,3,3,3,3};
					#endif
					#else
					unsigned char mc_i_offset[5] = 		{3,3,3,3,3};
					unsigned char mc_p_offset[5] = 	{4,4,4,4,4};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else{
				lowdelay_state = 0;
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
		}
		else if(((in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_60HZ)) || ((in_fmRate == _PQL_IN_50HZ) && (out_fmRate == _PQL_OUT_50HZ))){
			if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 1 /*|| u1_direct_media_flag == 1*/)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[1] = {5};
					unsigned char mc_p_offset[1] = {5};
					#else
					unsigned char mc_i_offset[1] = 	{3};
					unsigned char mc_p_offset[1] = 	{3};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 0)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[1] = {5};
					unsigned char mc_p_offset[1] = {5};
					#else
					unsigned char mc_i_offset[1] = 	{2};
					unsigned char mc_p_offset[1] = 	{2};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(u1_low_delay_ctrl1 == 0 && (Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1 || u1_vdec_direct_lowdelay_flag)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[1] = {6};
					unsigned char mc_p_offset[1] = {6};
					#else
					unsigned char mc_i_offset[1] = 	{3};
					unsigned char mc_p_offset[1] = 	{4};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(u1_low_delay_ctrl1 == 0 && (MEMC_Lib_get_Input_Src_Type() == 0x02/*_SRC_VO*/) &&(is_DTV_flag_get() == FALSE) && (MEMC_Lib_get_DisplayMode_Src() == 3/*VSC_INPUTSRC_VDEC*/)){ // Q-12305
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER

					unsigned char mc_i_offset[1] = {5};
					unsigned char mc_p_offset[1] = {5};

					#else
					unsigned char mc_i_offset[1] = 	{3};
					unsigned char mc_p_offset[1] = 	{4};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else{
				lowdelay_state = 0;
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
		}
		else if(((in_fmRate == _PQL_IN_30HZ) && (out_fmRate == _PQL_OUT_60HZ)) || ((in_fmRate == _PQL_IN_25HZ) && (out_fmRate == _PQL_OUT_50HZ))){
			if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 1)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[2] = {5,6};
					unsigned char mc_p_offset[2] = {5,6};
					#else
					unsigned char mc_i_offset[2] = 	{3,3};
					unsigned char mc_p_offset[2] = 	{3,3};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if((u1_adaptivestream_flag == 1) && (u1_callback_subscibe == 0)){
				lowdelay_state = 1;
				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[2] = {5,6};
					unsigned char mc_p_offset[2] = {5,6};
					#else
					unsigned char mc_i_offset[2] = 	{2, 2};
					unsigned char mc_p_offset[2] = 	{2, 2};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(u1_low_delay_ctrl1 == 0 && (Scaler_MEMC_Get_VpqLowDelayToMEMC_Flag() == 1 || u1_vdec_direct_lowdelay_flag)){
				lowdelay_state = 1;

				for (u16_k = (unsigned short)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					#if 1
					#if CONFIG_MC_8_BUFFER
					unsigned char mc_i_offset[2] = {5,6};
					unsigned char mc_p_offset[2] = {5,6};
					#else
					unsigned char mc_i_offset[2] = 	{2, 2};
					unsigned char mc_p_offset[2] = 	{3, 3};
					#endif
					#else
					unsigned char mc_i_offset[2] = 	{3, 4};
					unsigned char mc_p_offset[2] = 	{4, 5};
					#endif
					unsigned char index_offset = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].mc_i_offset   = mc_i_offset[index_offset];
					phTable_DecInfoGen[u16_k].mc_p_offset   = mc_p_offset[index_offset];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else{
				lowdelay_state = 0;
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
		}
		else{
			lowdelay_state = 0;
		}
	}
#if 0
	if((in_fmRate == _PQL_IN_60HZ) && (out_fmRate == _PQL_OUT_60HZ) && (u1_lowdelaystate == 1)){
		rtdf_setBits(0xb8028640, _BIT24);
		rtd_pr_memc_info("$$$   hi_09_3_SetBit_24   $$$\n\r$$$   (%x)(%x)   $$$\n\r");
	}
#endif
}

VOID FRC_phTable_NewLowDelay_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput, UINT16 u16_k , PQL_INPUT_FRAME_RATE in_fmRate, PQL_OUTPUT_FRAME_RATE out_fmRate, FRC_CADENCE_ID cadence_id)
{
	//const _PQLCONTEXT *s_pContext = GetPQLContext();
	unsigned int NewLowDelay_en = 0;
	unsigned short u16_k_tmp = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 2, 2, &NewLowDelay_en);

	if(NewLowDelay_en == 1 && (VR360_en == 0)){
		if((in_fmRate == _PQL_IN_60HZ && out_fmRate == _PQL_OUT_120HZ) || (in_fmRate == _PQL_IN_50HZ && out_fmRate == _PQL_OUT_100HZ)){
			if(cadence_id == _CAD_VIDEO){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[2] = {5,6};
					unsigned char me1_p_offset[2] = {5,6};
					unsigned char me2_i_offset[2] = {4,5};
					unsigned char me2_p_offset[2] = {5,6};
					unsigned char me2_phase[2] = {32,0};
					unsigned char mc_phase[2] = {64,0};
					unsigned char ppfv_offset[2] = {0,1};
					unsigned char me1_last[2] = {0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_22){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[4] = {3,3,4,4};
					unsigned char me1_p_offset[4] = {5,5,6,6};
					unsigned char me2_i_offset[4] = {3,3,2,4};
					unsigned char me2_p_offset[4] = {5,5,4,6};
					unsigned char me2_phase[4] = {16,32,48,0};
					unsigned char mc_phase[4] = {32,64,96,0};
					unsigned char ppfv_offset[4] = {1,1,0,1};
					unsigned char me1_last[4] = {0,1,0,0};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);

			}
			else if(cadence_id == _CAD_32){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[10] = {2,4,4,4,3,3,4,4,3,3};
					unsigned char me1_p_offset[10] = {4,6,6,6,5,5,6,6,5,5};
					unsigned char me2_i_offset[10] = {2,2,4,4,3,3,2,4,3,3};
					unsigned char me2_p_offset[10] = {4,4,6,6,5,5,4,6,5,5};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_32322){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[24] = {3,3,3,3,4,4,3,3,3,4,3,3,3,3,4,4,3,3,2,4,3,3,2,4};
					unsigned char me1_p_offset[24] = {5,5,5,5,6,6,5,5,5,6,5,5,5,5,6,6,5,5,4,6,5,5,4,6};
					unsigned char me2_i_offset[24] = {3,3,3,3,2,4,3,3,2,2,3,3,3,3,2,4,3,3,2,2,3,3,2,2};
					unsigned char me2_p_offset[24] = {5,5,5,5,4,6,5,5,4,4,5,5,5,5,4,6,5,5,4,4,5,5,4,4};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_334){

			}
			else if(cadence_id == _CAD_22224){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[24] = {4,4,3,3,4,4,3,3,2,4,3,3,2,2,3,3,2,2,2,4,4,4,3,3};
					unsigned char me1_p_offset[24] = {6,6,5,5,6,6,5,5,4,6,5,5,4,4,5,5,4,4,4,6,6,6,5,5};
					unsigned char me2_i_offset[24] = {2,4,3,3,2,4,3,3,2,2,3,3,2,2,1,3,2,2,2,2,4,4,3,3};
					unsigned char me2_p_offset[24] = {4,6,5,5,4,6,5,5,4,4,5,5,4,4,3,5,4,4,4,4,6,6,5,5};
					unsigned char me2_phase[24] = {61,11,24,38,51,0,14,27,40,54,3,16,30,43,56,6,19,32,46,59,8,22,35,48};
					unsigned char mc_phase[24] = {123,21,48,75,101,0,27,53,80,107,5,32,59,85,112,11,37,64,91,117,16,43,69,96};
					unsigned char ppfv_offset[24] = {0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1};
					unsigned char me1_last[24] = {0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_2224){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[20] = {4,4,3,3,2,4,3,3,2,2,3,3,2,2,2,4,4,4,3,3};
					unsigned char me1_p_offset[20] = {6,6,5,5,4,6,5,5,4,4,5,5,4,4,4,6,6,6,5,5};
					unsigned char me2_i_offset[20] = {2,4,3,3,2,2,3,3,2,2,1,3,2,2,2,2,4,4,3,3};
					unsigned char me2_p_offset[20] = {4,6,5,5,4,4,5,5,4,4,3,5,4,4,4,4,6,6,5,5};
					unsigned char me2_phase[20] = {51,0,13,26,38,51,0,13,26,38,51,0,13,26,38,51,0,13,26,38};
					unsigned char mc_phase[20] = {102,0,26,51,77,102,0,26,51,77,102,0,26,51,77,102,0,26,51,77};
					unsigned char ppfv_offset[20] = {0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1};
					unsigned char me1_last[20] = {0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_3223){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[20] = {2,4,4,4,3,3,4,4,3,3,2,4,3,3,2,2,4,4,3,3};
					unsigned char me1_p_offset[20] = {4,6,6,6,5,5,6,6,5,5,4,6,5,5,4,4,6,6,5,5};
					unsigned char me2_i_offset[20] = {2,2,4,4,3,3,2,4,3,3,2,2,3,3,2,2,2,4,3,3};
					unsigned char me2_p_offset[20] = {4,4,6,6,5,5,4,6,5,5,4,4,5,5,4,4,4,6,5,5};
					unsigned char me2_phase[20] = {38,51,0,13,26,38,51,0,13,26,38,51,0,13,26,38,51,0,13,26};
					unsigned char mc_phase[20] = {77,102,0,26,51,77,102,0,26,51,77,102,0,26,51,77,102,0,26,51};
					unsigned char ppfv_offset[20] = {1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1};
					unsigned char me1_last[20] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_55){

			}
			else if(cadence_id == _CAD_66){

			}
			else if(cadence_id == _CAD_44){

			}
			else if(cadence_id == _CAD_1112){

			}
			else if(cadence_id == _CAD_11112){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[12] = {5,5,4,5,4,5,4,5,4,4,4,4};
					unsigned char me1_p_offset[12] = {6,6,5,6,5,6,5,6,5,5,6,6};
					unsigned char me2_i_offset[12] = {3,5,4,4,4,4,4,4,4,4,3,4};
					unsigned char me2_p_offset[12] = {5,6,5,5,5,5,5,5,5,5,4,6};
					unsigned char me2_phase[12] = {43,5,32,59,21,48,11,37,0,27,53,16}; //{2,2,2,2,2};
					unsigned char mc_phase[12] = {85,11,64,117,43,96,21,75,0,53,107,32}; //{3,3,3,3,3};
					unsigned char ppfv_offset[12] = {0,1,1,0,1,0,1,0,1,1,0,1};
					unsigned char me1_last[12] = {0,0,1,0,1,0,1,0,0,1,0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_122){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[10] = {4,4,3,3,4,4,3,5,4,4};
					unsigned char me1_p_offset[10] = {6,6,5,5,6,6,5,6,5,5};
					unsigned char me2_i_offset[10] = {3,4,3,3,2,4,3,3,4,4};
					unsigned char me2_p_offset[10] = {4,6,5,5,4,6,5,5,5,5};
					unsigned char me2_phase[10] = {45,0,19,38,58,13,32,51,6,26}; //{2,2,2,2,2};
					unsigned char mc_phase[10] = {90,0,38,77,115,26,64,102,13,51}; //{3,3,3,3,3};
					unsigned char ppfv_offset[10] = {0,1,1,1,0,1,1,0,1,1};
					unsigned char me1_last[10] = {0,0,0,1,0,0,1,0,0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
		}
		else if((in_fmRate == _PQL_IN_30HZ && out_fmRate == _PQL_OUT_120HZ) || (in_fmRate == _PQL_IN_25HZ && out_fmRate == _PQL_OUT_100HZ)){
			if(cadence_id == _CAD_VIDEO){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[4] = {5,5,5,5};
					unsigned char me1_p_offset[4] = {6,6,6,6};
					unsigned char me2_i_offset[4] = {4,5,5,5};
					unsigned char me2_p_offset[4] = {5,6,6,6};
					unsigned char me2_phase[4] = {48,0,16,32};
					unsigned char mc_phase[4] = {96,0,32,64};
					unsigned char ppfv_offset[4] = {1,1,0,1};
					unsigned char me1_last[4] = {0,1,0,0};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_22){
			}
			else if(cadence_id == _CAD_32){
			}
			else if(cadence_id == _CAD_32322){
			}
			else if(cadence_id == _CAD_334){
			}
			else if(cadence_id == _CAD_22224){
			}
			else if(cadence_id == _CAD_2224){
			}
			else if(cadence_id == _CAD_3223){
			}
			else if(cadence_id == _CAD_55){
			}
			else if(cadence_id == _CAD_66){
			}
			else if(cadence_id == _CAD_44){
			}
			else if(cadence_id == _CAD_1112){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					unsigned char me1_i_offset[20] = {4,4,4,4,3,5,5,5,4,4,5,5,4,4,4,5,4,4,4,4};
					unsigned char me1_p_offset[20] = {6,6,6,6,5,6,6,6,5,5,6,6,5,5,5,6,5,5,5,5};
					unsigned char me2_i_offset[20] = {3,4,4,4,3,3,5,5,4,4,4,5,4,4,4,4,4,4,4,4};
					unsigned char me2_p_offset[20] = {4,6,6,6,5,5,6,6,5,5,5,6,5,5,5,5,5,5,5,5};
					unsigned char me2_phase[20] = {51,0,13,26,38,51,0,13,26,38,51,0,13,26,38,51,0,13,26,38};
					unsigned char mc_phase[20] = {102,0,26,51,77,102,0,26,51,77,102,0,26,51,77,102,0,26,51,77};
					unsigned char ppfv_offset[20] = {0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1};
					unsigned char me1_last[20] = {0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#else
					unsigned char me1_i_offset[10] = {4,4,4,4,5,5,4,5,4,5}; //{2,2,2,2,2};
					unsigned char me1_p_offset[10] = {5,5,6,6,6,6,5,6,5,6}; //{3,3,3,3,3};
					unsigned char me2_i_offset[10] = {3,3,2,3,2,3,2,2,3,3}; //{4,4,3,4,3,5,4,4,4,4};
					unsigned char me2_p_offset[10] = {4,4,3,4,3,5,4,4,4,4}; //{5,5,4,6,5,6,5,5,5,5};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				#endif
				}	
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_11112){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					unsigned char me1_i_offset[24] = {4,4,4,4,3,5,5,5,4,5,5,5,4,4,5,5,4,4,4,5,4,4,4,4};
					unsigned char me1_p_offset[24] = {6,6,6,6,5,6,6,6,5,6,6,6,5,5,6,6,5,5,5,6,5,5,5,5};
					unsigned char me2_i_offset[24] = {3,3,4,4,3,3,3,5,4,4,4,5,4,4,4,4,4,4,4,4,3,4,4,4};
					unsigned char me2_p_offset[24] = {4,4,6,6,5,5,5,6,5,5,5,6,5,5,5,5,5,5,5,5,4,5,5,5};
					unsigned char me2_phase[24] = {45,59,8,21,35,48,61,11,24,37,51,0,13,27,40,53,3,16,29,43,56,5,19,32};
					unsigned char mc_phase[24] = {91,117,16,43,69,96,123,21,48,75,101,0,27,53,80,107,5,32,59,85,112,11,37,64};
					unsigned char ppfv_offset[24] = {1,0,1,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1};
					unsigned char me1_last[24] = {1,0,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#else
					unsigned char me1_i_offset[12] = {4,4,4,4,5,5,5,5,4,5,4,5}; //{2,2,2,2,2};
					unsigned char me1_p_offset[12] = {5,5,6,6,6,6,6,6,5,6,5,6}; //{3,3,3,3,3};
					unsigned char me2_i_offset[12] = {3,3,2,3,2,3,2,4,3,3,3,3}; //{4,4,3,4,3,5,4,5,4,4,4,4};
					unsigned char me2_p_offset[12] = {4,4,3,4,3,5,4,5,4,4,4,4}; //{5,5,4,6,5,6,5,6,5,5,5,5};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				#endif
				}	
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_122){
			}
		}
		else if(in_fmRate == _PQL_IN_24HZ && out_fmRate == _PQL_OUT_120HZ){
			if(cadence_id == _CAD_VIDEO){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					unsigned char me1_i_offset[10] = {4,5,5,5,5,4,5,5,5,5};
					unsigned char me1_p_offset[10] = {5,6,6,6,6,5,6,6,6,6};
					unsigned char me2_i_offset[10] = {4,4,5,5,5,4,4,5,5,5};
					unsigned char me2_p_offset[10] = {5,5,6,6,6,5,5,6,6,6};
					unsigned char me2_phase[10] = {38,51,0,13,26,38,51,0,13,26};
					unsigned char mc_phase[10] = {77,102,0,26,51,77,102,0,26,51};
					unsigned char ppfv_offset[10] = {1,0,1,1,1,1,0,1,1,1};
					unsigned char me1_last[10] = {1,0,0,0,0,1,0,0,0,0};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#endif
				}	
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
		}
		else if((in_fmRate == _PQL_IN_60HZ && out_fmRate == _PQL_OUT_60HZ) || (in_fmRate == _PQL_IN_50HZ && out_fmRate == _PQL_OUT_50HZ)){
			if(cadence_id == _CAD_VIDEO){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me2_i_offset = 6;
					unsigned char me2_p_offset = 6;
					phTable_DecInfoGen[u16_k].me1_i_offset   = me2_i_offset;
					phTable_DecInfoGen[u16_k].me1_p_offset   = me2_p_offset;

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset;
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset;

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset;
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset;
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_22){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 0
					unsigned char me1_i_offset[2] = {4,5};
					unsigned char me1_p_offset[2] = {6,6};
					unsigned char me2_i_offset[2] = {4,3};
					unsigned char me2_p_offset[2] = {6,5};
				#else
					unsigned char me1_i_offset[2] = {3,5};
					unsigned char me1_p_offset[2] = {4,6};
					unsigned char me2_i_offset[2] = {3,2};
					unsigned char me2_p_offset[2] = {5,4};
				#endif
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				#if 0
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];
				#else
				#endif
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);

			}
			else if(cadence_id == _CAD_32){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me2_i_offset[5] = {3,4,4,3,4}; //{2,2,2,2,2};
					unsigned char me2_p_offset[5] = {5,6,6,5,6}; //{3,3,3,3,3};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_32322){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me2_i_offset[12] = {3,3,2,3,2,3,3,2,3,2,3,2};
					unsigned char me2_p_offset[12] = {5,5,4,5,4,5,5,4,5,4,5,4};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_334){

			}
			else if(cadence_id == _CAD_22224){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me2_i_offset[12] = {3,4,3,4,3,2,3,2,3,3,3,4};
					unsigned char me2_p_offset[12] = {5,6,5,6,5,4,5,4,5,5,5,6};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_2224){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me2_i_offset[10] = {3,4,3,2,3,2,3,3,3,4};
					unsigned char me2_p_offset[10] = {5,6,5,4,5,4,5,5,5,6};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_3223){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me2_i_offset[10] = {3,3,4,3,2,3,2,3,3,2};
					unsigned char me2_p_offset[10] = {5,5,6,5,4,5,4,5,5,4};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_55){

			}
			else if(cadence_id == _CAD_66){

			}
			else if(cadence_id == _CAD_44){

			}
			else if(cadence_id == _CAD_1112){

			}
			else if(cadence_id == _CAD_11112){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[6] = {5,4,4,4,4,4};
					unsigned char me1_p_offset[6] = {6,5,5,5,5,6};
					unsigned char me2_i_offset[6] = {3,4,3,3,3,3};
					unsigned char me2_p_offset[6] = {5,5,4,4,4,4};
					unsigned char me2_phase[6] = {11,0,53,43,32,21}; //{2,2,2,2,2};
					unsigned char mc_phase[6] = {21,0,107,85,64,43}; //{3,3,3,3,3};
					unsigned char ppfv_offset[6] = {0,1,0,0,0,0};
					unsigned char me1_last[6] = {0,1,1,1,1,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_122){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[5] = {4,5,4,4,3};
					unsigned char me1_p_offset[5] = {6,6,5,6,5};
					unsigned char me2_i_offset[5] = {2,3,4,3,3};
					unsigned char me2_p_offset[5] = {4,5,5,4,5};
					unsigned char me2_phase[5] = {51,26,0,38,13}; //{2,2,2,2,2};
					unsigned char mc_phase[5] = {102,51,0,77,26}; //{3,3,3,3,3};
					unsigned char ppfv_offset[5] = {0,0,1,0,1};
					unsigned char me1_last[5] = {1,0,1,0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
		}
		else if((in_fmRate == _PQL_IN_30HZ && out_fmRate == _PQL_OUT_60HZ) || (in_fmRate == _PQL_IN_25HZ && out_fmRate == _PQL_OUT_50HZ)){
			if(cadence_id == _CAD_VIDEO){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
					unsigned char me1_i_offset[2] = {5,5};
					unsigned char me1_p_offset[2] = {6,6};
					unsigned char me2_i_offset[2] = {4,5};
					unsigned char me2_p_offset[2] = {5,6};
					unsigned char me2_phase[2] = {32,0};
					unsigned char mc_phase[2] = {64,0};
					unsigned char ppfv_offset[2] = {0,1};
					unsigned char me1_last[2] = {0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				}
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_22){
			}
			else if(cadence_id == _CAD_32){
			}
			else if(cadence_id == _CAD_32322){
			}
			else if(cadence_id == _CAD_334){
			}
			else if(cadence_id == _CAD_22224){
			}
			else if(cadence_id == _CAD_2224){
			}
			else if(cadence_id == _CAD_3223){
			}
			else if(cadence_id == _CAD_55){
			}
			else if(cadence_id == _CAD_66){
			}
			else if(cadence_id == _CAD_44){
			}
			else if(cadence_id == _CAD_1112){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					unsigned char me1_i_offset[10] = {5,5,4,5,4,5,4,4,4,4}; //{2,2,2,2,2};
					unsigned char me1_p_offset[10] = {6,6,5,6,5,6,5,5,6,6}; //{3,3,3,3,3};
					unsigned char me2_i_offset[10] = {3,5,4,4,4,4,4,4,3,4}; //{4,4,3,4,3,5,4,4,4,4};
					unsigned char me2_p_offset[10] = {5,6,5,5,5,5,5,5,4,6}; //{5,5,4,6,5,6,5,5,5,5};
					unsigned char me2_phase[10] = {38,0,26,51,13,38,0,26,51,13};
					unsigned char mc_phase[10] = {77,0,51,102,26,77,0,51,102,26};
					unsigned char ppfv_offset[10] = {0,1,1,0,1,0,1,1,0,1};
					unsigned char me1_last[10] = {0,0,1,0,1,0,0,1,0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#else
					unsigned char me1_i_offset[10] = {4,4,4,4,5,5,4,5,4,5}; //{2,2,2,2,2};
					unsigned char me1_p_offset[10] = {5,5,6,6,6,6,5,6,5,6}; //{3,3,3,3,3};
					unsigned char me2_i_offset[10] = {3,3,2,3,2,3,2,2,3,3}; //{4,4,3,4,3,5,4,4,4,4};
					unsigned char me2_p_offset[10] = {4,4,3,4,3,5,4,4,4,4}; //{5,5,4,6,5,6,5,5,5,5};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				#endif
				}	
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_11112){
				for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
				{
				#if 1
					unsigned char me1_i_offset[12] = {5,5,4,5,4,5,4,5,4,4,4,4}; //{2,2,2,2,2};
					unsigned char me1_p_offset[12] = {6,6,5,6,5,6,5,6,5,5,6,6}; //{3,3,3,3,3};
					unsigned char me2_i_offset[12] = {3,5,4,4,4,4,4,4,4,4,3,4}; //{4,4,3,4,3,5,4,4,4,4};
					unsigned char me2_p_offset[12] = {5,6,5,5,5,5,5,5,5,5,4,6}; //{5,5,4,6,5,6,5,5,5,5};
					unsigned char me2_phase[12] = {43,5,32,59,21,48,11,37,0,27,53,16};
					unsigned char mc_phase[12] = {85,11,64,117,43,96,21,75,0,53,107,32};
					unsigned char ppfv_offset[12] = {0,1,1,0,1,0,1,0,1,1,0,1};
					unsigned char me1_last[12] = {0,0,1,0,1,0,1,0,0,1,0,1};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

					phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
				#else
					unsigned char me1_i_offset[12] = {4,4,4,4,5,5,5,5,4,5,4,5}; //{2,2,2,2,2};
					unsigned char me1_p_offset[12] = {5,5,6,6,6,6,6,6,5,6,5,6}; //{3,3,3,3,3};
					unsigned char me2_i_offset[12] = {3,3,2,3,2,3,2,4,3,3,3,3}; //{4,4,3,4,3,5,4,5,4,4,4,4};
					unsigned char me2_p_offset[12] = {4,4,3,4,3,5,4,5,4,4,4,4}; //{5,5,4,6,5,6,5,6,5,5,5,5};
					u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
					phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

					phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
					phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];
				#endif
				}	
				rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
			}
			else if(cadence_id == _CAD_122){
			}
		}
		else if(in_fmRate == _PQL_IN_24HZ && out_fmRate == _PQL_OUT_60HZ){
			for (u16_k = (UINT16)(pOutput->u16_phT_stIdx); u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
			{
				unsigned char me1_i_offset[5] = {5,5,5,5,5};
				unsigned char me1_p_offset[5] = {6,6,6,6,6};
				unsigned char me2_i_offset[5] = {4,5,4,5,5};
				unsigned char me2_p_offset[5] = {5,6,5,6,6};
				unsigned char me2_phase[5] = {51,13,38,0,26};
				unsigned char mc_phase[5] = {102,26,77,0,51};
				unsigned char ppfv_offset[5] = {0,1,0,1,1};
				unsigned char me1_last[5] = {0,1,0,0,1};
				u16_k_tmp = u16_k - (pOutput->u16_phT_stIdx);
				phTable_DecInfoGen[u16_k].me1_i_offset   = me1_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_p_offset   = me1_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me2_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].mc_i_offset   = me2_i_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_p_offset   = me2_p_offset[u16_k_tmp];

				phTable_DecInfoGen[u16_k].me2_phase = me2_phase[u16_k_tmp];
				phTable_DecInfoGen[u16_k].mc_phase = mc_phase[u16_k_tmp];

				phTable_DecInfoGen[u16_k].ppfv_offset = ppfv_offset[u16_k_tmp];
				phTable_DecInfoGen[u16_k].me1_last = me1_last[u16_k_tmp];
			}
			rtd_pr_memc_emerg("[%s][%d][%x,%x]\n\r", __FUNCTION__, __LINE__, pOutput->u16_phT_stIdx, pOutput->u16_phT_endIdx_p1);
		}
	}

}

VOID FRC_phTable_basicInfo_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{
	// calc table len, and generate initial information
	unsigned char  fracPh_nume, fracPh_denomi;
	fracPh_nume                = pOutput->local_FRC_cadence.cad_inLen  * pParam->u4_sys_N;
	fracPh_denomi              = pOutput->local_FRC_cadence.cad_outLen * pParam->u6_sys_M;

	if(pParam->u4_sys_N == 0 || pOutput->local_FRC_cadence.cad_outLen == 0){

		rtd_pr_memc_notice("[%s]Invalid param:(u4_sys_N=%d,cad_outLen=%d!!\n",\
				__FUNCTION__,pParam->u4_sys_N,pOutput->local_FRC_cadence.cad_outLen);
		return;
	}
	if (pParam->u4_sys_N % pOutput->local_FRC_cadence.cad_outLen == 0) // 4n5m@22
	{
		pOutput->u16_phT_length    = _MAX_(pParam->u4_sys_N, pParam->u6_sys_M);
		pOutput->u16_phT_endIdx_p1 = pOutput->u16_phT_stIdx + pOutput->u16_phT_length;
	}
	else if (pOutput->local_FRC_cadence.cad_outLen % pParam->u4_sys_N == 0) // 5n12m@3223
	{
		pOutput->u16_phT_length    = (pParam->u6_sys_M >= pParam->u4_sys_N)? fracPh_denomi / pParam->u4_sys_N : pOutput->local_FRC_cadence.cad_outLen;
		pOutput->u16_phT_endIdx_p1 = pOutput->u16_phT_stIdx + pOutput->u16_phT_length;
	}
	else
	{
		pOutput->u16_phT_length    = fracPh_denomi;
		pOutput->u16_phT_endIdx_p1 = pOutput->u16_phT_stIdx + pOutput->u16_phT_length;
	}

#if 0
	rtd_pr_memc_notice("[%s][%d][n2m,%d,%d,][,%d,][,%d,%d,][,%d,%d,%d,]", __FUNCTION__, __LINE__, 
		pParam->u4_sys_N, pParam->u6_sys_M, pOutput->local_FRC_cadence.cad_outLen,
		fracPh_nume, fracPh_denomi,
		pOutput->u16_phT_stIdx, pOutput->u16_phT_length, pOutput->u16_phT_endIdx_p1);
#endif

	//// phase step simplified
	if (fracPh_nume != 0 && ((fracPh_denomi % fracPh_nume) == 0))
	{
		fracPh_denomi          = fracPh_denomi / fracPh_nume;
		fracPh_nume            = 1;
	}
	pOutput->u8_fracPh_nume    = fracPh_nume;
	pOutput->u8_fracPh_denomi  = fracPh_denomi;

	// inPhase and outPhase allocation
	FRC_phTable_inPh_outPh_Gen(pParam, pOutput);

	///// relationship between filmPh and outPh.
	FRC_phTable_filmPh_Gen(pParam, pOutput);
}
VOID FRC_phTable_inPh_outPh_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{
	int budget_tot    = -pParam->u8_ph_sys_dly;                    // to do::  -system_ini_dly_nume * sys_M;
	unsigned short budget_1film  = pParam->u6_sys_M*100;
	unsigned short cost_1frame   = pParam->u4_sys_N*100;
//	int budget_tot    = 0;                    // to do::  -system_ini_dly_nume * sys_M;
//	unsigned char  budget_1film  = pParam->u6_sys_M;
//	unsigned char  cost_1frame   = pParam->u4_sys_N;
	unsigned char  pre_inPhase   = pParam->u4_sys_N - 1;
	unsigned char  pre_outPhase  = pParam->u6_sys_M - 1;

	unsigned short u16_k;

	if(pParam->u6_sys_M == 0 || pParam->u4_sys_N == 0){
		rtd_pr_memc_notice("[%s]Invalid Param(u6_sys_M=%d,u4_sys_N=%d)!!\n",__FUNCTION__,pParam->u6_sys_M,pParam->u4_sys_N);
		return;
	}
	for (u16_k = pOutput->u16_phT_stIdx; u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
	{
		if (budget_tot > 0)
		{
			phTable_DecInfoGen[u16_k].inPhase_new  = 0;
			phTable_DecInfoGen[u16_k].in_phase     = pre_inPhase;

			phTable_DecInfoGen[u16_k].outPhase_new = 1;
			phTable_DecInfoGen[u16_k].out_phase    = (pre_outPhase + 1) % pParam->u6_sys_M;
			pre_outPhase                           = phTable_DecInfoGen[u16_k].out_phase;

			budget_tot                             = budget_tot - cost_1frame;
		}
		else // if (budget_tot <= 0)
		{
			phTable_DecInfoGen[u16_k].inPhase_new = 1;
			phTable_DecInfoGen[u16_k].in_phase    = (pre_inPhase + 1) % pParam->u4_sys_N;
			pre_inPhase                           = phTable_DecInfoGen[u16_k].in_phase;

			budget_tot = budget_tot + budget_1film;
			if (budget_tot > 0)
			{
				phTable_DecInfoGen[u16_k].outPhase_new = 1;
				phTable_DecInfoGen[u16_k].out_phase    = (pre_outPhase + 1) % pParam->u6_sys_M;
				pre_outPhase                           = phTable_DecInfoGen[u16_k].out_phase;
				budget_tot                             = budget_tot - cost_1frame;
			}
			else   // if sys_N > sys_M, this will happen.
			{
				phTable_DecInfoGen[u16_k].outPhase_new = 0;
				phTable_DecInfoGen[u16_k].out_phase    = pre_outPhase;
			}
		}
	}
}
VOID FRC_phTable_filmPh_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{
	unsigned char  pre_filmPh = pOutput->local_FRC_cadence.cad_outLen - 1;
	unsigned short u16_k;
	unsigned int MC_8_buffer_en = 0;
	ReadRegister(SOFTWARE1_SOFTWARE1_62_reg, 0, 0, &MC_8_buffer_en);

	// find where to start,
	for (u16_k = 0; u16_k < pOutput->u16_phT_length; u16_k ++)
	{
		if (phTable_DecInfoGen[u16_k + pOutput->u16_phT_stIdx].in_phase == pParam->u4_filmPh0_inPh)
		{
			pOutput->u16_filmPh0_tableIdx = u16_k;
			break;
		}
	}

	// filmPhase generate.
	for (u16_k = 0; u16_k < pOutput->u16_phT_length; u16_k ++)
	{
		unsigned short table_idx = (u16_k + pOutput->u16_filmPh0_tableIdx) % pOutput->u16_phT_length + pOutput->u16_phT_stIdx;
		if (phTable_DecInfoGen[table_idx].inPhase_new == 0)
		{
			phTable_DecInfoGen[table_idx].film_phase  = pre_filmPh;
			phTable_DecInfoGen[table_idx].cadSeq_new  = 0;

		#if 1  // k4lp
			phTable_DecInfoGen[table_idx].mc_wrt_idx_oft = 0;
		#else
			phTable_DecInfoGen[table_idx].me2_wrt_idx_oft = 0;
		#endif

			phTable_DecInfoGen[table_idx].me1_wrt_idx_oft = 0;
		}
		else
		{
			unsigned char cur_filmPhase = 0,cur_wrtStep_me1 = 0,cur_wrtStep_mc = 0; //cur_wrtStep_me2 = 0;

			if(pOutput->local_FRC_cadence.cad_outLen){
				cur_filmPhase = (pre_filmPh + 1) % pOutput->local_FRC_cadence.cad_outLen;
			}else{
				rtd_pr_memc_notice("[%s]Invaild local_FRC_cadence.cad_outLen is zero!!\n",__FUNCTION__);
				cur_filmPhase = pre_filmPh + 1;
			}
			cur_wrtStep_me1   = FRC_phTable_WrtStep_Check_me1(cur_filmPhase, pParam->in3d_format);
		#if 1  // k4lp
			cur_wrtStep_mc    = FRC_phTable_WrtStep_Check_mc(cur_filmPhase, pParam->in3d_format);
		#else
			cur_wrtStep_me2   = FRC_phTable_WrtStep_Check_me2(cur_filmPhase, pParam->in3d_format);
		#endif

			if (pParam->in3d_format == _PQL_IN_LR && (cur_filmPhase & 0x01) == 1)
			{
				cur_wrtStep_me1 = 0;
				cur_wrtStep_mc = 0;
				//cur_wrtStep_me2 = 0;
			}

			phTable_DecInfoGen[table_idx].cadSeq_new  = (cadence_seqFlag(pOutput->local_FRC_cadence, cur_filmPhase) == 1)? 1 : 0;
			phTable_DecInfoGen[table_idx].film_phase  = cur_filmPhase;
			pre_filmPh                                = cur_filmPhase;

			if(MC_8_buffer_en)
				phTable_DecInfoGen[table_idx].mc_wrt_idx_oft = cur_wrtStep_me1; // cur_wrtStep_mc;
			else
				phTable_DecInfoGen[table_idx].mc_wrt_idx_oft = cur_wrtStep_mc;
			//phTable_DecInfoGen[table_idx].me2_wrt_idx_oft = cur_wrtStep_me2;
			phTable_DecInfoGen[table_idx].me1_wrt_idx_oft = cur_wrtStep_me1;
		}
	}
}

VOID FRC_phTable_IP_oft_Gen(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{
	// initial the first one's state.
	unsigned char u8_k  = 0;
	unsigned char u8_fracPh_me1_nume_k_n1 = 0, u8_fracPh_me1_nume_k = 0, u8_me1_chgImg_cnt = 0;
	unsigned char u8_fracPh_me2_nume_k_n1 = 0, u8_fracPh_me2_nume_k = 0, u8_me2_chgImg_cnt = 0;

	unsigned char u8_cadId  = pParam->u8_cadence_id;
	unsigned char u8_iniIdx = pOutput->u16_phT_stIdx + pOutput->u16_filmPh0_tableIdx;   // idx of table

	unsigned char ini_ph_me2_frac_nume = 0, ini_ph_frac_denom = 0;

	if(u8_cadId >= _FRC_CADENCE_NUM_){

		rtd_pr_memc_notice("[%s]Invalid u8_cadId:%d.Resume!!\n",__FUNCTION__,u8_cadId);
		u8_cadId = _CAD_VIDEO;
	}
	ini_ph_me2_frac_nume = phT_DecInitInfo[u8_cadId].ini_ph_frac_nume;
	ini_ph_frac_denom = phT_DecInitInfo[u8_cadId].ini_ph_frac_denom == 0? 1 : phT_DecInitInfo[u8_cadId].ini_ph_frac_denom;

	// bai me2 same as previou me
	if(ini_ph_frac_denom == 0){
		rtd_pr_memc_notice("[%s]ini_ph_frac_denom is zero!!\n",__FUNCTION__);
		ini_ph_frac_denom = 1;
	}
	phTable_DecInfoGen[u8_iniIdx].frac_phase_nume = (ini_ph_me2_frac_nume * pOutput->u8_fracPh_denomi)/ini_ph_frac_denom;
	phTable_DecInfoGen[u8_iniIdx].me2_i_offset    =  phT_DecInitInfo[u8_cadId].ini_me2_i_offset;
	phTable_DecInfoGen[u8_iniIdx].me2_p_offset    =  phT_DecInitInfo[u8_cadId].ini_me2_p_offset;
	u8_me2_chgImg_cnt                             =  phT_DecInitInfo[u8_cadId].ini_me2_chgImg_cnt;

#if 1  // k4lp
	phTable_DecInfoGen[u8_iniIdx].mc_i_offset     =  phT_DecInitInfo[u8_cadId].ini_mc_i_offset;
	phTable_DecInfoGen[u8_iniIdx].mc_p_offset     =  phT_DecInitInfo[u8_cadId].ini_mc_p_offset;
#endif

	u8_me1_chgImg_cnt                             =  phT_DecInitInfo[u8_cadId].ini_me2_chgImg_cnt - 1;
	u8_me1_chgImg_cnt = ((phTable_DecInfoGen[u8_iniIdx].frac_phase_nume + pOutput->u8_fracPh_nume)>=pOutput->u8_fracPh_denomi)?(u8_me1_chgImg_cnt-1):u8_me1_chgImg_cnt;

	//congratulate me1 initial
	FRC_phTable_me1_deriva_me1(pParam, &(phTable_DecInfoGen[u8_iniIdx]), u8_me1_chgImg_cnt, &(pOutput->local_FRC_cadence));

	// based on Mathematical induction, generate (k+1) state from (k) state:
	u8_fracPh_me2_nume_k_n1 = phTable_DecInfoGen[u8_iniIdx].frac_phase_nume;
	if(pParam->out3d_format == _PQL_OUT_SG_LLRR)
	{
		u8_fracPh_me1_nume_k_n1 = u8_fracPh_me2_nume_k_n1;
	}
	else
	{
		u8_fracPh_me1_nume_k_n1 = u8_fracPh_me2_nume_k_n1 + pOutput->u8_fracPh_nume;
	}

	if (pParam->out3d_format != _PQL_OUT_2D)
		{
			if(pOutput->u8_fracPh_denomi == 0){
				rtd_pr_memc_notice("[%s]Invalid:u8_fracPh_denomi is zero!\n",__FUNCTION__);
				pOutput->u8_fracPh_denomi = 1;
			}
			if (((u8_fracPh_me1_nume_k_n1%pOutput->u8_fracPh_denomi) + 2*pOutput->u8_fracPh_nume) >= pOutput->u8_fracPh_denomi)
			{
				phTable_DecInfoGen[u8_iniIdx].me1_last = 1;
			}
			else
			{
				phTable_DecInfoGen[u8_iniIdx].me1_last = 0;
			}
		}
	else
		{
			if ((u8_fracPh_me1_nume_k_n1 + pOutput->u8_fracPh_nume) >= pOutput->u8_fracPh_denomi)
			{
				phTable_DecInfoGen[u8_iniIdx].me1_last = 1;
			}
			else
			{
				phTable_DecInfoGen[u8_iniIdx].me1_last = 0;
			}
		}
	// bai ppfv
	phTable_DecInfoGen[u8_iniIdx].ppfv_offset    =  u8_me2_chgImg_cnt - u8_me1_chgImg_cnt;

	if (u8_fracPh_me1_nume_k_n1 >= pOutput->u8_fracPh_denomi)
	{
		u8_fracPh_me1_nume_k_n1 = u8_fracPh_me1_nume_k_n1 - pOutput->u8_fracPh_denomi;
	}

	// bai  me2
	for (u8_k = 1; u8_k < pOutput->u16_phT_length; u8_k++)
	{
		unsigned char       u8_phT_idx_k, u8_phT_idx_k_n1;
		unsigned char       u1_me2_ipswitch;
		Ph_Dec_Info  *pDecInfo_k, decInfo_k_n1;

		if(pOutput->u16_phT_length == 0)
		{
			rtd_pr_memc_notice("[%s]Warning from phTable:: There must be a error in n2m or cad.outLen, please check!!!\n",__FUNCTION__);
			u8_phT_idx_k = (unsigned char) pOutput->u16_phT_stIdx;
		}
		else
		{
			u8_phT_idx_k = (u8_k+pOutput->u16_filmPh0_tableIdx) % pOutput->u16_phT_length + pOutput->u16_phT_stIdx; // bai 从filmph0 开始
		}

		pDecInfo_k      = &(phTable_DecInfoGen[u8_phT_idx_k]);
		u8_phT_idx_k_n1 = (u8_phT_idx_k == pOutput->u16_phT_stIdx)? (pOutput->u16_phT_endIdx_p1 - 1) : (u8_phT_idx_k - 1);
		decInfo_k_n1    = phTable_DecInfoGen[u8_phT_idx_k_n1];

		// changeImg_cnt update.
		u8_me2_chgImg_cnt += pDecInfo_k->cadSeq_new;
		u8_me1_chgImg_cnt += pDecInfo_k->cadSeq_new;

		// memc_ph update.
		if(pOutput->u8_fracPh_denomi == 0){
			rtd_pr_memc_notice("[%s]Invalid:u8_fracPh_denomi is zero!\n",__FUNCTION__);
			pOutput->u8_fracPh_denomi = 1;
		}
		if (pDecInfo_k->outPhase_new == 0) // if sys_N > sys_M, this will happen.
		{
			u8_fracPh_me2_nume_k = u8_fracPh_me2_nume_k_n1;
			u8_fracPh_me1_nume_k = u8_fracPh_me1_nume_k_n1;
		}
		else if (pParam->u1_LRout_samePh == 1 && (pParam->out3d_format == _PQL_OUT_SG || pParam->out3d_format == _PQL_OUT_SG_LLRR))
		{
			if (pParam->out3d_format == _PQL_OUT_SG)
			{
				u8_fracPh_me2_nume_k = (u8_k % 2 > 0) ? u8_fracPh_me2_nume_k_n1 : (u8_fracPh_me2_nume_k_n1 + pOutput->u8_fracPh_nume * 2);
				u8_fracPh_me1_nume_k = (u8_k % 2 > 0) ?((u8_fracPh_me2_nume_k%pOutput->u8_fracPh_denomi) + pOutput->u8_fracPh_nume):(u8_fracPh_me2_nume_k%pOutput->u8_fracPh_denomi);
			}
			else // (pParam->out3d_format == _PQL_OUT_SG_LLRR)
			{
				u8_fracPh_me2_nume_k = (u8_k % 4 > 0) ? u8_fracPh_me2_nume_k_n1 : (u8_fracPh_me2_nume_k_n1 + pOutput->u8_fracPh_nume * 4);
				u8_fracPh_me1_nume_k = (u8_k % 4 > 0) ?((u8_fracPh_me2_nume_k%pOutput->u8_fracPh_denomi) + pOutput->u8_fracPh_nume):(u8_fracPh_me2_nume_k%pOutput->u8_fracPh_denomi);
			}
		}
		else if(pParam->out3d_format == _PQL_OUT_SG_LLRR)
		{
			u8_fracPh_me2_nume_k = (u8_k % 2 > 0) ? u8_fracPh_me2_nume_k_n1 : (u8_fracPh_me2_nume_k_n1 + pOutput->u8_fracPh_nume * 2);
			u8_fracPh_me1_nume_k = (u8_k % 2 > 0) ?((u8_fracPh_me2_nume_k%pOutput->u8_fracPh_denomi) + 2*pOutput->u8_fracPh_nume):(u8_fracPh_me2_nume_k%pOutput->u8_fracPh_denomi);
		}
		else
		{
			u8_fracPh_me2_nume_k = u8_fracPh_me2_nume_k_n1 + pOutput->u8_fracPh_nume;
			u8_fracPh_me1_nume_k = (u8_fracPh_me2_nume_k%pOutput->u8_fracPh_denomi) + pOutput->u8_fracPh_nume;
		}

		// I/P_idx gen.  me2
		if (u8_fracPh_me2_nume_k < pOutput->u8_fracPh_denomi) // no IP switch
		{
		#if 1  // k4lp
			FRC_phTable_IP_oft_Gen_IPns_mc(&decInfo_k_n1, pDecInfo_k, u8_me2_chgImg_cnt, &(pOutput->local_FRC_cadence));
			FRC_phTable_IP_oft_Gen_IPns_me2(&decInfo_k_n1, pDecInfo_k, u8_me2_chgImg_cnt, &(pOutput->local_FRC_cadence));
		#endif

			u1_me2_ipswitch = 0;
		}
		else
		{
			u1_me2_ipswitch = 1;
			do
			{
			#if 1  // k4lp
				FRC_phTable_IP_oft_Gen_IPs_mc(pParam, &decInfo_k_n1, pDecInfo_k, u8_me2_chgImg_cnt, &(pOutput->local_FRC_cadence));
				decInfo_k_n1.mc_p_offset  = pDecInfo_k->mc_p_offset - pDecInfo_k->mc_wrt_idx_oft;
			#endif

				FRC_phTable_IP_oft_Gen_IPs_me2(pParam, &decInfo_k_n1, pDecInfo_k, u8_me2_chgImg_cnt, &(pOutput->local_FRC_cadence));
				u8_fracPh_me2_nume_k       = u8_fracPh_me2_nume_k - pOutput->u8_fracPh_denomi;
				decInfo_k_n1.me2_p_offset  = pDecInfo_k->me2_p_offset - pDecInfo_k->me1_wrt_idx_oft;
//				decInfo_k_n1.me2_p_offset  = pDecInfo_k->me2_p_offset - pDecInfo_k->me2_wrt_idx_oft;
				u8_me2_chgImg_cnt          = _MAX_(0, u8_me2_chgImg_cnt - 1);
			} while (u8_fracPh_me2_nume_k >= pOutput->u8_fracPh_denomi);
		}

		// I/P_idx gen.  me1
		if (u8_fracPh_me1_nume_k < pOutput->u8_fracPh_denomi) // no IP switch
		{
			FRC_phTable_IP_oft_Gen_IPns_me1(&decInfo_k_n1, pDecInfo_k, u8_me1_chgImg_cnt, &(pOutput->local_FRC_cadence));
		}
		else
		{
			do
			{
				FRC_phTable_IP_oft_Gen_IPs_me1(pParam, &decInfo_k_n1, pDecInfo_k, u8_me1_chgImg_cnt, &(pOutput->local_FRC_cadence));
				u8_fracPh_me1_nume_k       = u8_fracPh_me1_nume_k - pOutput->u8_fracPh_denomi;
				decInfo_k_n1.me1_p_offset  = pDecInfo_k->me1_p_offset - pDecInfo_k->me1_wrt_idx_oft;
				u8_me1_chgImg_cnt          = _MAX_(0, u8_me1_chgImg_cnt - 1);
			} while (u8_fracPh_me1_nume_k >= pOutput->u8_fracPh_denomi);
		}

		pDecInfo_k->frac_phase_nume = u8_fracPh_me2_nume_k;
		u8_fracPh_me2_nume_k_n1     = u8_fracPh_me2_nume_k;
		u8_fracPh_me1_nume_k_n1     = u8_fracPh_me1_nume_k;

		if (pParam->out3d_format != _PQL_OUT_2D)
		{
			if (((u8_fracPh_me1_nume_k_n1%pOutput->u8_fracPh_denomi) + 2*pOutput->u8_fracPh_nume) >= pOutput->u8_fracPh_denomi)
			{
				pDecInfo_k->me1_last = 1;
			}
			else
			{
				pDecInfo_k->me1_last = 0;
			}
		}
		else
		{
			if ((u8_fracPh_me1_nume_k + pOutput->u8_fracPh_nume) >= pOutput->u8_fracPh_denomi)
			{
				pDecInfo_k->me1_last = 1;
			}
			else
			{
				pDecInfo_k->me1_last = 0;
			}
		}
		pDecInfo_k->ppfv_offset    =  (u8_me2_chgImg_cnt - u8_me1_chgImg_cnt )%_PHASE_FRC_PPFV_BUF_SIZE_;
	}
}

VOID FRC_phTable_me1_deriva_me1(const Ph_decGen_Param *pParam, Ph_Dec_Info *pDecInfo_cur, unsigned char chgImg_cnt_me1, FRC_CADENCE *pLocalCadence)
{
	unsigned char p_offset = 0;
	unsigned char i_offset = 0;
	unsigned char chgImg_cnt_me1_p = chgImg_cnt_me1;
	unsigned char chgImg_cnt_me1_i = chgImg_cnt_me1 + 1;
	unsigned char chgImg_cnt = 1;
	unsigned char frame_cnt = 0;

 	unsigned char film_phase       = pDecInfo_cur->film_phase;
	if((pParam->in3d_format == _PQL_IN_LR) && ((film_phase & 0x01) == 1))
	{
		film_phase = film_phase - 1;
	}
	if( chgImg_cnt_me1_p == 1)
	{
		p_offset    = 1;
	}
	else
	{
		while (1)
		{
			p_offset    += FRC_phTable_WrtStep_Check_me1(film_phase, pParam->in3d_format);
			film_phase   = cadence_prePhase(*pLocalCadence, film_phase, 1);
			FRC_phTable_IP_oft_newUsableP_check_me1((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &p_offset);
			frame_cnt = frame_cnt + 1;
			if (cadence_seqFlag(*pLocalCadence, film_phase)==1)
			{
				if(frame_cnt > _PHASE_FRC_ME1FRM_BUF_SIZE_)
				{
					p_offset = p_offset - (frame_cnt - _PHASE_FRC_ME1FRM_BUF_SIZE_);
				}
				chgImg_cnt = chgImg_cnt + 1;
				frame_cnt = 0;
			}
			if (chgImg_cnt == chgImg_cnt_me1_p)
			{
				p_offset    += FRC_phTable_WrtStep_Check_me1(film_phase, pParam->in3d_format);
				film_phase   = cadence_prePhase(*pLocalCadence, film_phase, 1);
				FRC_phTable_IP_oft_newUsableP_check_me1((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &p_offset);
				break;
			}
		}
	}
	film_phase       = pDecInfo_cur->film_phase;
	chgImg_cnt       = 1;
	if((pParam->in3d_format == _PQL_IN_LR) && ((film_phase & 0x01) == 1))
	{
		film_phase = film_phase - 1;
	}
	while (1)
	{
		i_offset    += FRC_phTable_WrtStep_Check_me1(film_phase, pParam->in3d_format);
		film_phase   = cadence_prePhase(*pLocalCadence, film_phase, 1);
		FRC_phTable_IP_oft_newUsableP_check_me1((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &i_offset);
		frame_cnt = frame_cnt + 1;
		if (cadence_seqFlag(*pLocalCadence, film_phase)==1)
		{
			if(frame_cnt > _PHASE_FRC_ME1FRM_BUF_SIZE_)
			{
				i_offset = i_offset - (frame_cnt - _PHASE_FRC_ME1FRM_BUF_SIZE_);
			}
			chgImg_cnt = chgImg_cnt + 1;
			frame_cnt = 0;
		}
		if (chgImg_cnt == chgImg_cnt_me1_i)
		{
			i_offset    += FRC_phTable_WrtStep_Check_me1(film_phase, pParam->in3d_format);
			film_phase   = cadence_prePhase(*pLocalCadence, film_phase, 1);
		    FRC_phTable_IP_oft_newUsableP_check_me1((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &i_offset);
			break;
		}
	}
	pDecInfo_cur->me1_p_offset = p_offset;
	pDecInfo_cur->me1_i_offset = i_offset;
}

#if 1  // k4lp
VOID FRC_phTable_IP_oft_Gen_IPns_mc(Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur,unsigned char chgImg_cnt_me2, FRC_CADENCE *pLocalCadence)
{
	unsigned char cur_cadSeq       = cadence_seqFlag(*pLocalCadence, pDecInfo_cur->film_phase);
	unsigned char latest_is_P_me2      = ((chgImg_cnt_me2 == 0) || (chgImg_cnt_me2 == 1 && cur_cadSeq == 1))? 1 : 0;
	//unsigned char latest_is_Usable = cadence_usableFlag(*pLocalCadence, pDecInfo_pre->film_phase) == 1? 1 : 0;

	pDecInfo_cur->mc_i_offset = pDecInfo_pre->mc_i_offset + pDecInfo_cur->mc_wrt_idx_oft;
	//pDecInfo_cur->me2_p_offset = (latest_is_P_me2 && latest_is_Usable)? 1 : (pDecInfo_pre->me2_p_offset + pDecInfo_cur->me2_wrt_idx_oft);
	pDecInfo_cur->mc_p_offset = (latest_is_P_me2)? 1 : (pDecInfo_pre->mc_p_offset + pDecInfo_cur->mc_wrt_idx_oft);
}
#endif

VOID FRC_phTable_IP_oft_Gen_IPns_me2(Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur,unsigned char chgImg_cnt_me2, FRC_CADENCE *pLocalCadence)
{
	unsigned char cur_cadSeq       = cadence_seqFlag(*pLocalCadence, pDecInfo_cur->film_phase);
	unsigned char latest_is_P_me2      = ((chgImg_cnt_me2 == 0) || (chgImg_cnt_me2 == 1 && cur_cadSeq == 1))? 1 : 0;
	//unsigned char latest_is_Usable = cadence_usableFlag(*pLocalCadence, pDecInfo_pre->film_phase) == 1? 1 : 0;

	pDecInfo_cur->me2_i_offset = pDecInfo_pre->me2_i_offset + pDecInfo_cur->me1_wrt_idx_oft;
	//pDecInfo_cur->me2_i_offset = pDecInfo_pre->me2_i_offset + pDecInfo_cur->me2_wrt_idx_oft;
	//pDecInfo_cur->me2_p_offset = (latest_is_P_me2 && latest_is_Usable)? 1 : (pDecInfo_pre->me2_p_offset + pDecInfo_cur->me2_wrt_idx_oft);
	pDecInfo_cur->me2_p_offset = (latest_is_P_me2)? 1 : (pDecInfo_pre->me2_p_offset + pDecInfo_cur->me1_wrt_idx_oft);
	//pDecInfo_cur->me2_p_offset = (latest_is_P_me2)? 1 : (pDecInfo_pre->me2_p_offset + pDecInfo_cur->me2_wrt_idx_oft);
}

VOID FRC_phTable_IP_oft_Gen_IPns_me1(Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur,unsigned char chgImg_cnt_me1, FRC_CADENCE *pLocalCadence)
{
	unsigned char cur_cadSeq       = cadence_seqFlag(*pLocalCadence, pDecInfo_cur->film_phase);
	unsigned char latest_is_P_me1      = ((chgImg_cnt_me1 == 0) || (chgImg_cnt_me1 == 1 && cur_cadSeq == 1))? 1 : 0;
	//unsigned char latest_is_Usable = cadence_usableFlag(*pLocalCadence, pDecInfo_pre->film_phase) == 1? 1 : 0;

	pDecInfo_cur->me1_i_offset = pDecInfo_pre->me1_i_offset + pDecInfo_cur->me1_wrt_idx_oft;
	//pDecInfo_cur->me1_p_offset = (latest_is_P_me1 && latest_is_Usable)? 1 : (pDecInfo_pre->me1_p_offset + pDecInfo_cur->me1_wrt_idx_oft);
	pDecInfo_cur->me1_p_offset = (latest_is_P_me1)? 1 : (pDecInfo_pre->me1_p_offset + pDecInfo_cur->me1_wrt_idx_oft);
}

#if 1  // k4lp
VOID FRC_phTable_IP_oft_Gen_IPs_mc(const Ph_decGen_Param *pParam, Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur, unsigned char chgImg_cnt_me2, FRC_CADENCE *pLocalCadence)
{
	unsigned char film_phase       = pDecInfo_cur->film_phase;

	if((pParam->in3d_format == _PQL_IN_LR) && ((film_phase & 0x01) == 1))
	{
		film_phase = film_phase - 1;
	}
	pDecInfo_cur->mc_i_offset = pDecInfo_pre->mc_p_offset + pDecInfo_cur->mc_wrt_idx_oft;

	if (chgImg_cnt_me2 < 2)
	{
		rtd_pr_memc_notice("me1 and me2 IP cover when IP switch. outPh = %d, filmPh = %d \r\n", pDecInfo_cur->out_phase, film_phase);
	}
	else
	{
		unsigned char p_offset = 0;
		unsigned char chgImg_cnt = 1;
		chgImg_cnt_me2 = (cadence_seqFlag(*pLocalCadence, film_phase) == 1) ? (chgImg_cnt_me2 - 1) : (chgImg_cnt_me2);

		while (1)
		{
			p_offset    += FRC_phTable_WrtStep_Check_mc(film_phase, pParam->in3d_format);
			film_phase   = cadence_prePhase(*pLocalCadence, film_phase, 1);
			FRC_phTable_IP_oft_newUsableP_check_mc((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &p_offset);
			if (cadence_seqFlag(*pLocalCadence, film_phase)==1)
			{
				chgImg_cnt = chgImg_cnt + 1;
			}
			if (chgImg_cnt == chgImg_cnt_me2)
			{
				p_offset    += FRC_phTable_WrtStep_Check_mc(film_phase, pParam->in3d_format);
			    film_phase   = cadence_prePhase(*pLocalCadence, film_phase, 1);
		    	FRC_phTable_IP_oft_newUsableP_check_mc((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &p_offset);
				break;
			}
		}
		pDecInfo_cur->mc_p_offset = p_offset;
	}
}
#endif

VOID FRC_phTable_IP_oft_Gen_IPs_me2(const Ph_decGen_Param *pParam, Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur, unsigned char chgImg_cnt_me2, FRC_CADENCE *pLocalCadence)
{
	unsigned char film_phase       = pDecInfo_cur->film_phase;

	if((pParam->in3d_format == _PQL_IN_LR) && ((film_phase & 0x01) == 1))
	{
		film_phase = film_phase - 1;
	}
	pDecInfo_cur->me2_i_offset = pDecInfo_pre->me2_p_offset + pDecInfo_cur->me1_wrt_idx_oft;
	//pDecInfo_cur->me2_i_offset = pDecInfo_pre->me2_p_offset + pDecInfo_cur->me2_wrt_idx_oft;

	if (chgImg_cnt_me2 < 2)
	{
		rtd_pr_memc_notice("me1 and me2 IP cover when IP switch. outPh = %d, filmPh = %d \r\n", pDecInfo_cur->out_phase, film_phase);
	}
	else
	{
		unsigned char p_offset = 0;
		unsigned char chgImg_cnt = 1;
		chgImg_cnt_me2 = (cadence_seqFlag(*pLocalCadence, film_phase) == 1) ? (chgImg_cnt_me2 - 1) : (chgImg_cnt_me2);

		while (1)
		{
			p_offset    += FRC_phTable_WrtStep_Check_me2(film_phase, pParam->in3d_format);
			film_phase   = cadence_prePhase(*pLocalCadence, film_phase, 1);
			//FRC_phTable_IP_oft_newUsableP_check_me2((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &p_offset);

			if (cadence_seqFlag(*pLocalCadence, film_phase)==1)
			{
				chgImg_cnt = chgImg_cnt + 1;
			}
			if (chgImg_cnt == chgImg_cnt_me2)
			{
				p_offset    += FRC_phTable_WrtStep_Check_me2(film_phase, pParam->in3d_format);
			    film_phase   = cadence_prePhase(*pLocalCadence, film_phase, 1);
		    	FRC_phTable_IP_oft_newUsableP_check_me2((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &p_offset);
				break;
			}
		}
		pDecInfo_cur->me2_p_offset = p_offset;
	}
}

VOID FRC_phTable_IP_oft_Gen_IPs_me1(const Ph_decGen_Param *pParam, Ph_Dec_Info *pDecInfo_pre, Ph_Dec_Info *pDecInfo_cur, unsigned char chgImg_cnt_me1, FRC_CADENCE *pLocalCadence)
{
	unsigned char film_phase       = pDecInfo_cur->film_phase;
	pDecInfo_cur->me1_i_offset = pDecInfo_pre->me1_p_offset + pDecInfo_cur->me1_wrt_idx_oft;

	if (chgImg_cnt_me1 < 1)
	{
		pDecInfo_cur->me1_p_offset = 1;
		rtd_pr_memc_notice("no P to use when IP switch.\n");
	}
	else if (chgImg_cnt_me1 == 1)
	{
		unsigned char p_offset = 0;
		if (cadence_seqFlag(*pLocalCadence, film_phase)==0 && _PHASE_FRC_ME1FRM_BUF_SIZE_ > 1)
		{// if conditions permit, no use of the written one.
			p_offset    += FRC_phTable_WrtStep_Check_me1(film_phase, pParam->in3d_format);
			film_phase   = cadence_prePhase(*pLocalCadence, film_phase, 1);
			FRC_phTable_IP_oft_newUsableP_check_me1((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &p_offset);
		}
		pDecInfo_cur->me1_p_offset = p_offset;
	}
	else
	{
		unsigned char p_offset = 0;
		while (1)
		{
			if (pParam->in3d_format == _PQL_IN_LR && (film_phase & 0x01) == 1)
			{ // in inLR case, if changeImg happens on inR image, change to tracing changImg by inL image.
				film_phase = film_phase - 1;
			}

			chgImg_cnt_me1 = (cadence_seqFlag(*pLocalCadence, film_phase) == 1) ? (chgImg_cnt_me1 - 1) : (chgImg_cnt_me1);
			p_offset  +=  FRC_phTable_WrtStep_Check_me1(film_phase, pParam->in3d_format);
			film_phase =  cadence_prePhase(*pLocalCadence, film_phase, 1);
			if (chgImg_cnt_me1 == 1)
			{
				FRC_phTable_IP_oft_newUsableP_check_me1((unsigned char)pParam->in3d_format, pLocalCadence, film_phase, &p_offset);
				break;
			}
		}
		pDecInfo_cur->me1_p_offset = p_offset;
	}
}



VOID FRC_phTable_IP_oft_newUsableP_check_me1(unsigned char in3d_format, FRC_CADENCE *pLocalCadence, unsigned char u8_filmPh, unsigned char *pPoffset)
{
	unsigned char u8_k  = 0;
	while(1)
	{
		if (FRC_phTable_WrtStep_Check_me1(cadence_nxtPhase(*pLocalCadence, u8_filmPh, 1), in3d_format) == 1) // P is usable
		{
			return;
		}
		else
		{
			*pPoffset  += FRC_phTable_WrtStep_Check_me1(u8_filmPh, in3d_format);
			u8_filmPh   = cadence_prePhase(*pLocalCadence, u8_filmPh, 1);

			u8_k = u8_k +1;
			if(u8_k >= 50)
			{
			   rtd_pr_memc_notice("newUsableP_check wrong\r\n");
			   return;
			}
		}
	}

}

VOID FRC_phTable_IP_oft_newUsableP_check_me2(unsigned char in3d_format, FRC_CADENCE *pLocalCadence, unsigned char u8_filmPh, unsigned char *pPoffset)
{
	unsigned char u8_k  = 0;
	while(1)
	{
		if (FRC_phTable_WrtStep_Check_me2(cadence_nxtPhase(*pLocalCadence, u8_filmPh, 1), in3d_format) == 1) // P is usable
		{
			return;
		}
		else
		{
			if(cadence_seqFlag(*pLocalCadence, u8_filmPh)==0)
			{
			*pPoffset  += FRC_phTable_WrtStep_Check_me2(u8_filmPh, in3d_format);
			u8_filmPh   = cadence_prePhase(*pLocalCadence, u8_filmPh, 1);
			}
			else
			{
				return;
			}
			u8_k = u8_k +1;
			if(u8_k >= 50)
			{
			   rtd_pr_memc_notice("newUsableP_check wrong\r\n");
			   return;
			}
		}
	}
}

#if 1  // k4lp
VOID FRC_phTable_IP_oft_newUsableP_check_mc(unsigned char in3d_format, FRC_CADENCE *pLocalCadence, unsigned char u8_filmPh, unsigned char *pPoffset)
{
	unsigned char u8_k  = 0;
	while(1)
	{
		if (FRC_phTable_WrtStep_Check_mc(cadence_nxtPhase(*pLocalCadence, u8_filmPh, 1), in3d_format) == 1) // P is usable
		{
			return;
		}
		else
		{
			if(cadence_seqFlag(*pLocalCadence, u8_filmPh)==0)
			{
				*pPoffset  += FRC_phTable_WrtStep_Check_mc(u8_filmPh, in3d_format);
			    u8_filmPh   = cadence_prePhase(*pLocalCadence, u8_filmPh, 1);
			}
			else
			{
				return;
			}
			u8_k = u8_k +1;
			if(u8_k >= 50)
			{
			   rtd_pr_memc_notice("newUsableP_check wrong\r\n");
			   return;
			}
		}
	}
}
#endif

////////
unsigned char  u8_pulldown_mode;
VOID FRC_decPhT_flbk_proc(const Ph_decGen_Param *pParam, Ph_decGen_Out *pOutput)
{
	const _PQLCONTEXT   *s_pContext = GetPQLContext();
	FRC_CADENCE_ID u8_cadence_id = s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL];
	BOOL bSwitch = (s_pContext->_output_frc_phtable.u1_lowdelaystate == 0 && \
		(pParam->u8_pulldown_mode == _PQL_IN24HZ_TRUMOTOFF_CINEMAOFF || ((u8_cadence_id == _CAD_32 || u8_cadence_id == _CAD_2224) && pParam->u8_pulldown_mode == _PQL_IN60HZ_TRUMOTOFF_CINEMAOFF)));
	//static unsigned char state = 0;
	u8_pulldown_mode = pParam->u8_pulldown_mode;
	if(s_pContext->_external_data.u3_memcMode == 1)//DejudderByMotion would continusly change dejudder level, which would make log keep printing
	{
	}
	else{
		rtd_pr_memc_notice("[flbk_proc]:%x : %x : %x\n\r", bSwitch, u8_cadence_id, pParam->u8_pulldown_mode);
	}

	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  4, 7, pParam->u8_pulldown_mode);
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg,  8, 11, u8_cadence_id);
	//WriteRegister(KPOST_TOP_KPOST_TOP_A4_reg, 12, 15, bSwitch);

	//rtd_pr_memc_notice("[MEMC_===== %x \n",(int)pParam);
	if(bSwitch == TRUE) // 1st deblur -> 2nd dejudder
	{
		if (pParam->u8_deblur_lvl < (1<<_PHASE_FLBK_LVL_PRECISION))
		{
			FRC_deblur_phFlbk_ctrl(pParam->u8_deblur_lvl, pOutput);
		}
		rtd_pr_memc_notice("[%s][%d]:%x : %x : %x\n\r", __FUNCTION__, __LINE__, bSwitch, u8_cadence_id, pParam->u8_pulldown_mode);
		//if(state == 1){
			MEMC_LibSetMCBlend(2);
		//}
		//state = 0;
	}
	//rtd_pr_memc_notice("=====u8_dejudder_lvl= %d \n",pParam->u8_dejudder_lvl);

	//YE Test
	#if 0
	//YE_Test = 1<<_PHASE_FLBK_LVL_PRECISION;
	YE_u8_dejudder_lvl = pParam->u8_dejudder_lvl;
	//if (pParam->u8_dejudder_lvl < (1<<_PHASE_FLBK_LVL_PRECISION))
	if (YE_u8_dejudder_lvl < (1<<_PHASE_FLBK_LVL_PRECISION))
	{
		for ( YE_u8_dejudder_lvl=1;YE_u8_dejudder_lvl<100;YE_u8_dejudder_lvl++)
			{
  				rtd_pr_memc_notice("=====YE_u8_dejudder_lvl = %d FLBK_LVL=%d\n",YE_u8_dejudder_lvl,1<<_PHASE_FLBK_LVL_PRECISION);
				FRC_dejudder_phFlbk_ctrl(YE_u8_dejudder_lvl, pOutput);
			}
	}
	#endif
	#if 1	
	if (pParam->u8_dejudder_lvl < (1<<_PHASE_FLBK_LVL_PRECISION))
	{
		FRC_dejudder_phFlbk_ctrl(pParam->u8_dejudder_lvl, pOutput);
	}
	#endif
	//rtd_pr_memc_notice(" CCC =====u8_dejudder_lvl= %d \n",pParam->u8_dejudder_lvl);

	if(bSwitch == FALSE) // 1st dejudder -> 2nd deblur
	{
		if (pParam->u8_deblur_lvl < (1<<_PHASE_FLBK_LVL_PRECISION))
		{
			FRC_deblur_phFlbk_ctrl(pParam->u8_deblur_lvl, pOutput);
		}
		//if(state == 0){
			WriteRegister(LBMC_LBMC_24_reg, 13, 13, 0x0); //0xb8099924[13] //IZ mode
		//}
		//state = 1;
	}
	//rtd_pr_memc_notice(" DDD =====u8_dejudder_lvl= %d \n",pParam->u8_dejudder_lvl);

}
VOID FRC_dejudder_phFlbk_ctrl(unsigned char deJudder_lvl, Ph_decGen_Out *pOutput)
{
	unsigned short u16_k;
	#if Pro_tv030
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	#endif

	for(u16_k = pOutput->u16_phT_stIdx; u16_k < pOutput->u16_phT_endIdx_p1; u16_k ++)
	{
		phTable_DecInfoGen[u16_k].mc_phase = _2Val_RndBlending_(phTable_DecInfoGen[u16_k].mc_phase, 0, deJudder_lvl, _PHASE_FLBK_LVL_PRECISION);
		phTable_DecInfoGen[u16_k].me2_phase = _2Val_RndBlending_(phTable_DecInfoGen[u16_k].me2_phase, 0, deJudder_lvl, _PHASE_FLBK_LVL_PRECISION);
		#if Pro_tv030
		//60in_CAD32_cinema_test
		if( s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ && s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL] == _CAD_32 &&(u8_MEMCMode == MEMC_TYPE_55_PULLDOWN)){
			unsigned char u16_k_tmp = 0;
			unsigned char me2_phase[5] = { 0, 0, 0, 0, 32};
			unsigned char mc_phase[5] = {0, 0, 0, 0, 64};
			u16_k_tmp = u16_k - pOutput->u16_phT_stIdx;
			phTable_DecInfoGen[u16_k].me2_phase   = me2_phase[u16_k_tmp];
			phTable_DecInfoGen[u16_k].mc_phase   = mc_phase[u16_k_tmp];
			if(u8_MEMCMode == MEMC_TYPE_55_PULLDOWN){
				//rtd_pr_memc_notice("[MEMC_CinemaMuteCtrl][CC][k=%d][cad=%d][mode=%d][InFrameRate=%d][mc_phase=%d][time=%x]\n", u16_k, cadence, u8_MEMCMode, in_fmRate, phTable_DecInfoGen[u16_k].mc_phase, rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
			}
		}
		#endif
	}

}
VOID FRC_deblur_phFlbk_ctrl(unsigned char deBlur_lvl, Ph_decGen_Out *pOutput)
{
	unsigned short u16_k;
	#if Pro_tv030
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	#endif

	for(u16_k = 1; u16_k < pOutput->u16_phT_length; u16_k = (u16_k + 2))
	{
		unsigned char  idx_0 = (u16_k + pOutput->u16_filmPh0_tableIdx) % pOutput->u16_phT_length + pOutput->u16_phT_stIdx;
		unsigned char idx_n1 = (idx_0 == pOutput->u16_phT_stIdx)?  (pOutput->u16_phT_endIdx_p1 - 1) : (idx_0 - 1);
		unsigned char idx_n2 = (idx_0 == pOutput->u16_phT_stIdx)?  (pOutput->u16_phT_endIdx_p1 - 2) :((idx_0 == (pOutput->u16_phT_stIdx + 1))? (pOutput->u16_phT_endIdx_p1 - 1) : (idx_0 - 2));
		if( phTable_DecInfoGen[idx_0].mc_phase >= phTable_DecInfoGen[idx_n1].mc_phase)
		{
			phTable_DecInfoGen[idx_0].mc_phase = _2Val_RndBlending_(phTable_DecInfoGen[idx_0].mc_phase, phTable_DecInfoGen[idx_n1].mc_phase, deBlur_lvl, _PHASE_FLBK_LVL_PRECISION);
			phTable_DecInfoGen[idx_0].me2_phase = _2Val_RndBlending_(phTable_DecInfoGen[idx_0].me2_phase, phTable_DecInfoGen[idx_n1].me2_phase, deBlur_lvl, _PHASE_FLBK_LVL_PRECISION);
		}
		else if(phTable_DecInfoGen[idx_0].mc_phase < phTable_DecInfoGen[idx_n1].mc_phase)
		{
			FRC_deblur_phFlbk_IPs( &(phTable_DecInfoGen[idx_n2]),&(phTable_DecInfoGen[idx_n1]), &(phTable_DecInfoGen[idx_0]), deBlur_lvl);
		}

		#if Pro_tv030
		//60in_CAD32_cinema_test
		if( s_pContext->_external_data._input_frameRate == _PQL_IN_60HZ && s_pContext->_output_filmDetectctrl.u8_cur_cadence_out[_FILM_ALL] == _CAD_32 &&(u8_MEMCMode == MEMC_TYPE_55_PULLDOWN)){
			unsigned char u16_k_tmp = 0;
			unsigned char me2_phase[5] = { 0, 0, 0, 0, 32};
			unsigned char mc_phase[5] = {0, 0, 0, 0, 64};
			u16_k_tmp = u16_k - pOutput->u16_phT_stIdx;
			phTable_DecInfoGen[u16_k].me2_phase   = me2_phase[u16_k_tmp];
			phTable_DecInfoGen[u16_k].mc_phase   = mc_phase[u16_k_tmp];
			if(u8_MEMCMode == MEMC_TYPE_55_PULLDOWN){
				//rtd_pr_memc_notice("[MEMC_CinemaMuteCtrl][DD][k=%d][cad=%d][mode=%d][InFrameRate=%d][mc_phase=%d][time=%x]\n", u16_k, cadence, u8_MEMCMode, in_fmRate, phTable_DecInfoGen[u16_k].mc_phase, rtd_inl(TIMER_SCPU_CLK90K_LO_reg));
			}
		}
		#endif
	}
}
VOID FRC_deblur_phFlbk_IPs(Ph_Dec_Info *pDecInfo_n2,Ph_Dec_Info *pDecInfo_n1, Ph_Dec_Info *pDecInfo_0,  unsigned char flbk_lvl)
{
	unsigned char mc_phase_0 = pDecInfo_0->mc_phase + (1 << MC_PHASE_PRECISION);
	unsigned char me2_phase_0;
	mc_phase_0     = _2Val_RndBlending_(mc_phase_0, pDecInfo_n1->mc_phase, flbk_lvl, _PHASE_FLBK_LVL_PRECISION);
	if(mc_phase_0 > (1<<MC_PHASE_PRECISION))
	{
		mc_phase_0 = (mc_phase_0 - (1 << MC_PHASE_PRECISION));
	}

	pDecInfo_0->mc_phase = mc_phase_0;

	me2_phase_0 = pDecInfo_0->mc_phase >> (MC_PHASE_PRECISION - ME_PHASE_PRECISION);
	pDecInfo_0->me2_phase = (pDecInfo_0->mc_phase >= pDecInfo_n1->mc_phase)? _MAX_(me2_phase_0, pDecInfo_n1->me2_phase) : me2_phase_0;

	if(pDecInfo_0->mc_phase >= pDecInfo_n1->mc_phase) // need to change I/P
	{
		pDecInfo_0->mc_i_offset = pDecInfo_n1->mc_i_offset - pDecInfo_0->mc_wrt_idx_oft;
		pDecInfo_0->mc_p_offset = pDecInfo_n1->mc_p_offset - pDecInfo_0->mc_wrt_idx_oft;

		pDecInfo_0->me2_i_offset = pDecInfo_n1->me2_i_offset - pDecInfo_0->me1_wrt_idx_oft;
		pDecInfo_0->me2_p_offset = pDecInfo_n1->me2_p_offset - pDecInfo_0->me1_wrt_idx_oft;
		//pDecInfo_0->me2_i_offset = pDecInfo_n1->me2_i_offset - pDecInfo_0->me2_wrt_idx_oft;
		//pDecInfo_0->me2_p_offset = pDecInfo_n1->me2_p_offset - pDecInfo_0->me2_wrt_idx_oft;

		pDecInfo_n1->me1_i_offset = pDecInfo_n2->me1_i_offset - pDecInfo_n1->me1_wrt_idx_oft;
		pDecInfo_n1->me1_p_offset = pDecInfo_n2->me1_p_offset - pDecInfo_n1->me1_wrt_idx_oft;

		pDecInfo_n1->me1_last = 1;
		pDecInfo_n2->me1_last = 0;

		pDecInfo_n1->ppfv_offset = (pDecInfo_n1->ppfv_offset + 1)%_PHASE_FRC_PPFV_BUF_SIZE_;
		pDecInfo_0->ppfv_offset = (pDecInfo_0->ppfv_offset - 1)%_PHASE_FRC_PPFV_BUF_SIZE_;
	}
}


////after image
unsigned char after_image_proc(unsigned char u8_filmPhase_pre, unsigned char u8_filmPhase_dtc,unsigned char u8_outPhase, unsigned char u8_cadence_id_pre, unsigned char u8_cadence_id_dtc, unsigned char in_3dFormat, unsigned char u8_sys_M_pre, unsigned short u16_end_idx)
{
	unsigned short u16_k = 0;
	unsigned char film_mode_pre = 0;
	unsigned char film_mode_dtc = 0;
	unsigned char me_phase_pre = 0, me_phase_dtc = 0;
	unsigned char i_offset_pre = 0, i_offset_dtc = 0;
	unsigned char p_offset_pre = 0, p_offset_dtc = 0;
	unsigned char Wrt_in_idx_dtc = 0;
	unsigned char Wrt_in_idx_dtc_next = 0;
	unsigned char pre_idx_pre = 0, pre_idx_dtc = 0;
	unsigned char buff_idx_pre_i = 0, buff_idx_dtc_i = 0, buff_idx_pre_p = 0, buff_idx_dtc_p = 0;
	unsigned char u8_outPhase_next = 0;
	unsigned char outphase_last = 0;

	if(u8_cadence_id_pre >= _FRC_CADENCE_NUM_){
		rtd_pr_memc_notice("[%s]Invalid u8_cadence_id_pre:%d!!\n",__FUNCTION__,u8_cadence_id_pre);
		u8_cadence_id_pre = _CAD_VIDEO;
	}
	if(u8_cadence_id_dtc >= _FRC_CADENCE_NUM_){
		rtd_pr_memc_notice("[%s]Invalid u8_cadence_id_dtc:%d!!\n",__FUNCTION__,u8_cadence_id_dtc);
		u8_cadence_id_dtc = _CAD_VIDEO;
	}
	film_mode_pre = u8_cadence_id_pre == _CAD_VIDEO? 0 : (u8_cadence_id_pre == _CAD_22? 1 : (u8_cadence_id_pre == _CAD_32? 2 : 3));
	film_mode_dtc = u8_cadence_id_dtc == _CAD_VIDEO? 0 : (u8_cadence_id_dtc == _CAD_22? 1 : (u8_cadence_id_dtc == _CAD_32? 2 : 3));

	for(u16_k=0; u16_k<u16_end_idx; u16_k++)
	{
		if(phTable_DecInfoGen[u16_k].film_mode == film_mode_pre && phTable_DecInfoGen[u16_k].film_phase == u8_filmPhase_pre && phTable_DecInfoGen[u16_k].out_phase == u8_outPhase)
		{

			outphase_last = u8_outPhase;
			pre_idx_pre = u16_k;
			//LogPrintf(DBG_MSG,"bai print . film_mode_pre = %d, filmPhase_pre = %d, out_phase_pre = %d \r\n", phTable_DecInfoGen[u16_k].film_mode, phTable_DecInfoGen[u16_k].film_phase, phTable_DecInfoGen[u16_k].out_phase);
			//LogPrintf(DBG_MSG,"bai print . me_phase_pre = %d, i_offset_pre = %d, p_offset_pre = %d, idx_pre = %d \r\n", me_phase_pre, i_offset_pre, p_offset_pre, pre_idx_pre);

			break;
		}
	}
	for(u16_k=0; u16_k<u16_end_idx; u16_k++)
	{
		if(phTable_DecInfoGen[pre_idx_pre].film_phase == phTable_DecInfoGen[pre_idx_pre + u16_k].film_phase && phTable_DecInfoGen[pre_idx_pre].film_mode == phTable_DecInfoGen[pre_idx_pre + u16_k].film_mode)
		{
			outphase_last = phTable_DecInfoGen[pre_idx_pre + u16_k].out_phase;
			//me_phase_pre = phTable_DecInfoGen[pre_idx_pre + u16_k].me_phase;
			//i_offset_pre = phTable_DecInfoGen[pre_idx_pre + u16_k].i_offset;
			//p_offset_pre = phTable_DecInfoGen[pre_idx_pre + u16_k].p_offset;
		}
		else
		{
			rtd_pr_memc_notice("bai print . outphase_last_pre = %d \r\n", outphase_last);
			break;
		}
	}

	for(u16_k=0; u16_k<u16_end_idx; u16_k++)
	{
		if(phTable_DecInfoGen[u16_k].film_mode == film_mode_pre && phTable_DecInfoGen[u16_k].film_phase == u8_filmPhase_pre && phTable_DecInfoGen[u16_k].out_phase == outphase_last)
		{
			me_phase_pre = phTable_DecInfoGen[u16_k].me2_phase;
			i_offset_pre = phTable_DecInfoGen[u16_k].me2_i_offset;
			p_offset_pre = phTable_DecInfoGen[u16_k].me2_p_offset;
			//LogPrintf(DBG_MSG,"bai print . film_mode_pre = %d, filmPhase_pre = %d, out_phase_pre = %d \r\n", phTable_DecInfoGen[u16_k].film_mode, phTable_DecInfoGen[u16_k].film_phase, phTable_DecInfoGen[u16_k].out_phase);
			//LogPrintf(DBG_MSG,"bai print . me_phase_pre = %d, i_offset_pre = %d, p_offset_pre = %d, idx_pre = %d \r\n", me_phase_pre, i_offset_pre, p_offset_pre, pre_idx_pre); ;
			break;
		}
	}

	u8_outPhase_next = (outphase_last + 1)%(u8_sys_M_pre+1);
	if(u8_outPhase_next == 0)
	{
		u8_outPhase_next = 1;
	}

	for(u16_k=0; u16_k<u16_end_idx; u16_k++)
	{
		if(phTable_DecInfoGen[u16_k].film_mode == film_mode_dtc && phTable_DecInfoGen[u16_k].film_phase == u8_filmPhase_dtc && phTable_DecInfoGen[u16_k].out_phase == u8_outPhase_next)
		{
			me_phase_dtc = phTable_DecInfoGen[u16_k].me2_phase;
			i_offset_dtc = phTable_DecInfoGen[u16_k].me2_i_offset;
			p_offset_dtc = phTable_DecInfoGen[u16_k].me2_p_offset;
			pre_idx_dtc = u16_k;  // need change

			//LogPrintf(DBG_MSG,"bai print . film_mode_dtc = %d, filmPhase_dtc = %d, out_phase_dtc = %d \r\n", phTable_DecInfoGen[u16_k].film_mode, phTable_DecInfoGen[u16_k].film_phase, phTable_DecInfoGen[u16_k].out_phase);
			//LogPrintf(DBG_MSG,"bai print . me_phase_dtc = %d, i_offset_dtc = %d, p_offset_dtc = %d, idx_dtc = %d \r\n", me_phase_dtc, i_offset_dtc, p_offset_dtc, pre_idx_dtc);
			break;
		}
	}

	if(0)  // dtc control next in  wrt
	{
		if (film_mode_dtc == 0)
		{
			Wrt_in_idx_dtc = 1;
		}
		else
		{
			if(u8_cadence_id_dtc==0 || u8_cadence_id_dtc==1 || u8_cadence_id_dtc==2) // video  22 32
			{
				Wrt_in_idx_dtc = FRC_phTable_WrtStep_Check_mc_vd_22_32( u8_cadence_id_dtc, u8_filmPhase_dtc, in_3dFormat);
				//Wrt_in_idx_dtc = FRC_phTable_WrtStep_Check_me2_vd_22_32( u8_cadence_id_dtc, u8_filmPhase_dtc, in_3dFormat);
			}
			else //other film
			{
				Wrt_in_idx_dtc = FRC_phTable_WrtStep_Check_mc(u8_filmPhase_dtc, in_3dFormat);
				//Wrt_in_idx_dtc = FRC_phTable_WrtStep_Check_me2(u8_filmPhase_dtc, in_3dFormat);
			}

		}
	}

	if(1) // pre flme  control next in  wrt
	{
		if (film_mode_pre == 0)
		{
			Wrt_in_idx_dtc = 1;
		}
		else
		{
			unsigned char u8_filmPhase_pre_next = (u8_filmPhase_pre + 1)% frc_cadTable[u8_cadence_id_pre].cad_outLen;
			if(u8_cadence_id_pre==0 || u8_cadence_id_pre==1 || u8_cadence_id_pre==2) // video  22 32
			{
				Wrt_in_idx_dtc = FRC_phTable_WrtStep_Check_mc_vd_22_32( u8_cadence_id_pre, u8_filmPhase_pre_next, in_3dFormat);
				//Wrt_in_idx_dtc = FRC_phTable_WrtStep_Check_me2_vd_22_32( u8_cadence_id_pre, u8_filmPhase_pre_next, in_3dFormat);
			}
			else //other film
			{
				Wrt_in_idx_dtc = FRC_phTable_WrtStep_Check_mc(u8_filmPhase_pre_next, in_3dFormat);
				//Wrt_in_idx_dtc = FRC_phTable_WrtStep_Check_me2(u8_filmPhase_pre_next, in_3dFormat);
			}
		}
	}

	if (film_mode_dtc == 0)
	{
		Wrt_in_idx_dtc_next = 1;
	}
	else
	{
		unsigned char u8_filmPhase_dtc_next = (u8_filmPhase_dtc + 1)% frc_cadTable[u8_cadence_id_dtc].cad_outLen;
		if(u8_cadence_id_dtc==0 || u8_cadence_id_dtc==1 || u8_cadence_id_dtc==2) // video  22 32
		{
			Wrt_in_idx_dtc_next = FRC_phTable_WrtStep_Check_mc_vd_22_32( u8_cadence_id_dtc, u8_filmPhase_dtc_next, in_3dFormat);
			//Wrt_in_idx_dtc_next = FRC_phTable_WrtStep_Check_me2_vd_22_32( u8_cadence_id_dtc, u8_filmPhase_dtc_next, in_3dFormat);
		}
		else //other film
		{
			Wrt_in_idx_dtc_next = FRC_phTable_WrtStep_Check_mc(u8_filmPhase_dtc_next, in_3dFormat);
			//Wrt_in_idx_dtc_next = FRC_phTable_WrtStep_Check_me2(u8_filmPhase_dtc_next, in_3dFormat);
		}

	}

	rtd_pr_memc_notice("bai print . Wrt_in_idx_dtc = %d \r\n", Wrt_in_idx_dtc);
	/*buff_idx_pre_i = i_offset_pre + Wrt_in_idx_pre;
	buff_idx_pre_p = p_offset_pre + Wrt_in_idx_pre;*/

	buff_idx_pre_i = i_offset_pre;
	buff_idx_pre_p = p_offset_pre;

	buff_idx_dtc_i = i_offset_dtc + Wrt_in_idx_dtc + Wrt_in_idx_dtc_next;
	buff_idx_dtc_p = p_offset_dtc + Wrt_in_idx_dtc + Wrt_in_idx_dtc_next;

	if( buff_idx_dtc_i > buff_idx_pre_i && buff_idx_dtc_p > buff_idx_pre_p)
	{
		return  1;
	}
	else if(me_phase_dtc > me_phase_pre && (buff_idx_dtc_i >= buff_idx_pre_i && buff_idx_dtc_p >= buff_idx_pre_p))
	{
		return  1;
	}
	else
	{
		return 0;
	}
}

