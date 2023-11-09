/***************************************************************************************************
  File        : demod_rtk_r_qam.cpp
  Description : REALTEK_R_QAM (OpenCable) demod
  Author      : Kevin Wang
****************************************************************************************************
    Update List :
----------------------------------------------------------------------------------------------------
    20120207    | create phase
***************************************************************************************************/
#include <linux/slab.h>
#include <rtk_kdriver/io.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "foundation.h"
#include "demod_rtk_r_qam.h"
#include "demod_rtk_r_qam_priv.h"
#include "rtk_dtv_demod_8051_reg.h"
#include "rtk_dtv_demod_sys_reg.h"
#include "dvbc_reg.h"

unsigned long Test_Time = 0;
extern unsigned int gSfi_cancel_wn_15_8;
extern unsigned int gSfi_cancel_wn_7_0;
extern unsigned char gCorse_sfi_found;
//#define REALTEK_R_QAM_SetIF_LGE
/*----------------------------------------------------------------------
 * Func : REALTEK_R_QAM
 *
 * Desc : constructor
 *
 * Parm : device_addr : device address
 *        output_mode : REALTEK_R_QAM_OUT_IF_SERIAL / REALTEK_R_QAM_OUT_IF_PARALLEL
 *        output_freq : REALTEK_R_QAM_OUT_FREQ_6M / REALTEK_R_QAM_OUT_FREQ_10M
 *        pComm       : hadle of COMM
 *
 * Retn : handle of demod
 *----------------------------------------------------------------------*/
void REALTEK_R_QAM_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm)
{
	REALTEK_R_QAM* pRealtekRQAM;

	pDemodKernel->private_data = kmalloc(sizeof(REALTEK_R_QAM), GFP_KERNEL);
	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);

	Demod_Constructors(pDemodKernel);
	pDemodKernel->m_id                 = DTV_DEMOD_REALTEK_R_QAM;
	pDemodKernel->m_addr               = addr;
	pDemodKernel->m_pTuner             = NULL;
	pDemodKernel->m_pComm              = pComm;
	pDemodKernel->m_ScanRange          = SCAN_RANGE_1_6;
	pDemodKernel->m_Capability         = TV_SYS_TYPE_OPENCABLE;
	pDemodKernel->m_update_interval_ms = 200;
	pDemodKernel->m_if.freq            = 36125000;
	pDemodKernel->m_if.inversion       = 1;
	pDemodKernel->m_if.agc_pola        = 0;
    if (DtvGetICVersion() == 1)      //MERLIN4_VER_A
	    pDemodKernel->m_clock              = CRYSTAL_FREQ_27000000HZ;
    else
	    pDemodKernel->m_clock              = CRYSTAL_FREQ_25200000HZ;

	pRealtekRQAM->m_output_freq         = output_freq;
	pRealtekRQAM->m_private             = (void*) AllocRealtekRQamDriver(
			pDemodKernel->m_pComm,
			pDemodKernel->m_addr,
			pDemodKernel->m_clock);
	pDemodKernel->m_ts.mode            = (output_mode == RTK_DEMOD_OUT_IF_PARALLEL) ? PARALLEL_MODE : SERIAL_MODE;
	pDemodKernel->m_ts.data_order      = MSB_D7;
	pDemodKernel->m_ts.datapin         = MSB_FIRST;
	pDemodKernel->m_ts.err_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.sync_pol        = ACTIVE_HIGH;
	pDemodKernel->m_ts.val_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.clk_pol         = RISING_EDGE;
	pDemodKernel->m_ts.internal_demod_input = 1;
	pDemodKernel->m_ts.tsOutEn = 1;
	pRealtekRQAM->m_low_snr_detect     = 0;
	pRealtekRQAM->m_low_snr_recover    = 0;
	pRealtekRQAM->m_TunerOptRegCnt     = 0;
	pRealtekRQAM->m_pTunerOptReg       = NULL;
	pRealtekRQAM->m_pTunerOptRegRbus   = NULL;
	pRealtekRQAM->m_acquire_sig_en = 0;
	pRealtekRQAM->m_auto_tune_en = 0;
	pRealtekRQAM->m_unlock_cnt = 0;
	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "REALTEK_R_QAM");
	pDemodKernel->Init = REALTEK_R_QAM_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = REALTEK_R_QAM_AcquireSignal;
	pDemodKernel->ScanSignal = REALTEK_R_QAM_ScanSignal;
	pDemodKernel->SetIF = REALTEK_R_QAM_SetIF;
	pDemodKernel->SetTSMode = REALTEK_R_QAM_SetTSMode;
	pDemodKernel->SetTVSysEx = REALTEK_R_QAM_SetTVSysEx;
	pDemodKernel->SetMode = REALTEK_R_QAM_SetMode;
	pDemodKernel->SetTVSys = REALTEK_R_QAM_SetTVSys;
	pDemodKernel->GetLockStatus = REALTEK_R_QAM_GetLockStatus;
	pDemodKernel->GetSegmentNum = Demod_GetSegmentNum;
	pDemodKernel->GetSignalInfo = REALTEK_R_QAM_GetSignalInfo;
	pDemodKernel->GetSignalQuality = REALTEK_R_QAM_GetSignalQuality;
	pDemodKernel->ChangeT2PLP = Demod_ChangeT2PLP;
	pDemodKernel->GetT2PLPInfo = Demod_GetT2PLPInfo;
	pDemodKernel->GetT2L1PreInfo = Demod_GetT2L1PreInfo;
	pDemodKernel->GetT2L1PostInfo = Demod_GetT2L1PostInfo;
	pDemodKernel->GetDemodStatus = Demod_GetDemodStatus;
	pDemodKernel->GetCarrierOffset = REALTEK_R_QAM_GetCarrierOffset;
	pDemodKernel->ForwardI2C = REALTEK_R_QAM_ForwardI2C;
	pDemodKernel->AutoTune = REALTEK_R_QAM_AutoTune;
	pDemodKernel->KernelAutoTune = REALTEK_R_QAM_KernelAutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = Demod_Suspend;
	pDemodKernel->Resume = Demod_Resume;
	pDemodKernel->SetTvMode = REALTEK_R_QAM_SetTvMode;
	pDemodKernel->InspectSignal = REALTEK_R_QAM_InspectSignal;	  // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
	pDemodKernel->Lock = REALTEK_R_QAM_Lock;
	pDemodKernel->Unlock = REALTEK_R_QAM_Unlock;
	//pDemodKernel->Activate = REALTEK_R_QAM_Activate;
	pDemodKernel->Destory = REALTEK_R_QAM_Destructors;
}


/*----------------------------------------------------------------------
 * Func : ~REALTEK_R_QAM
 *
 * Desc : Destructor of REALTEK_R_QAM
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void REALTEK_R_QAM_Destructors(DEMOD* pDemodKernel)
{
	REALTEK_R_QAM* pRealtekRQAM;
	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);

	if (pRealtekRQAM->m_private != NULL)
		ReleaseRealtekRQamDriver((REALTEK_R_QAM_DRIVER_DATA*)pRealtekRQAM->m_private);

	kfree(pRealtekRQAM);
	//pDemodKernel->Destory(pDemodKernel);
	REALTEK_R_QAM_INFO("REALTEK_R_QAM_Destructors Complete\n");
}


/*----------------------------------------------------------------------
 * Func : Init
 *
 * Desc : Initial realtek_r
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_Init(DEMOD* pDemodKernel)
{
	int i = 0;
	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekRQAM->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;
	pRealtekRQAM->m_auto_tune_en = 0;
	pRealtekRQAM->m_unlock_cnt = 0;

	//DtvDemodClockEnable(1);
	DtvDemodInitialization(RTK_DEMOD_MODE_QAM, 1);

	pDemod->Symbolratedelay = 40;

	// check tuner information
	if (pDemodKernel->m_pTuner) {
		TUNER_PARAM Param;
		if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, TV_SYS_OPENCABLE_64, &Param) == TUNER_CTRL_OK) {
			switch (Param.id) {
			case TV_TUNER_NXP_TDA18275A:
				REALTEK_R_QAM_INFO("Tuner id %u, TV_TUNER_NXP_TDA18275X\n", Param.id);
				break;
			case TV_TUNER_SILICONLABS_SI2151:
				REALTEK_R_QAM_INFO("Tuner id %u, TV_TUNER_SILICONLABS_SI2151\n", Param.id);
				pRealtekRQAM->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SI2151_OPT_VALUE_QAM;
				pRealtekRQAM->m_TunerOptRegCnt = sizeof(SI2151_OPT_VALUE_QAM) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			case TV_TUNER_SILICONLABS_SI2178B:
			case TV_TUNER_NO_TUNER:
				REALTEK_R_QAM_INFO("Tuner id %u, TV_TUNER_SILICONLABS_SI2178B\n", Param.id);
				pRealtekRQAM->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SI2178B_OPT_VALUE_QAM;
				pRealtekRQAM->m_TunerOptRegCnt = sizeof(SI2178B_OPT_VALUE_QAM) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			case TV_TUNER_MAXLINEAR_MXL661:
				REALTEK_R_QAM_INFO("Tuner id %u, TV_TUNER_MAXLINEAR_MXL661\n", Param.id);
				pRealtekRQAM->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) MXL661_OPT_VALUE_QAM;
				pRealtekRQAM->m_TunerOptRegCnt = sizeof(MXL661_OPT_VALUE_QAM) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			case TV_TUNER_RAFAEL_R840:
				REALTEK_R_QAM_INFO("Tuner id %u, TV_TUNER_RAFAEL_R840\n", Param.id);
				break;
			case TV_TUNER_SONY_SUTRX243:
			case TV_TUNER_SONY_SUTRX243_RA:
				REALTEK_R_QAM_INFO("Tuner id %u, TV_TUNER_SONY_SUTRX243\n", Param.id);
				pRealtekRQAM->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SUTRX243_OPT_VALUE_QAM;
				pRealtekRQAM->m_TunerOptRegCnt = sizeof(SUTRX243_OPT_VALUE_QAM) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			default:
				REALTEK_R_QAM_INFO("unkonw tuner id %u, using default setting\n", Param.id);
				pRealtekRQAM->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SI2178B_OPT_VALUE_QAM;
				pRealtekRQAM->m_TunerOptRegCnt = sizeof(SI2178B_OPT_VALUE_QAM) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			}
			pDemodKernel->m_if.freq      = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola  = Param.if_agc_pola;

			REALTEK_R_QAM_INFO("if_freq = "PT_U32BITS", Inverse = %u, if_agc_pola = %u\n", pDemodKernel->m_if.freq, pDemodKernel->m_if.inversion, pDemodKernel->m_if.agc_pola);
		}
	}

	// Initial Driver
	pDemodKernel->Lock(pDemodKernel);

	if (pDemod->Initialize(pDemod) != FUNCTION_SUCCESS)
		goto init_fail;

	ClearDemodReset(RTK_DEMOD_MODE_QAM);

	//pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OPT_MPEG_OUT_SEL, 1);      // set MPEG output select
	rtd_maskl(DVBC_OPT_MPEG_OUT_SEL_reg, ~DVBC_OPT_MPEG_OUT_SEL_opt_mpeg_out_sel_mask, DVBC_OPT_MPEG_OUT_SEL_opt_mpeg_out_sel(0x1));

	// Run Optimize Configuration for specified tuner
	for (i = 0; i < pRealtekRQAM->m_TunerOptRegCnt; i++) {
		U32BITS Reg = pRealtekRQAM->m_pTunerOptRegRbus[i].RegBitNameRbus;
		U32BITS Val = pRealtekRQAM->m_pTunerOptRegRbus[i].ValueRbus;

		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, Reg, Val) != FUNCTION_SUCCESS)
		//	goto init_fail;
		rtd_maskl(Reg, 0xffffff00, Val);
	}

	if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
		goto init_fail;

	if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK)
		goto init_fail;

	REALTEK_R_QAM_DBG("REALTEK_R_QAM Init Complete\n");
	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_OK;

init_fail:
	REALTEK_R_QAM_WARNING("REALTEK_R_QAM Init Fail\n");
	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_FAIL;
}


/*----------------------------------------------------------------------
 * Func : SetTVSys
 *
 * Desc : Set TV System
 *
 * Parm : sys : TV System
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	REALTEK_R_QAM* pRealtekRQAM;
	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);

	REALTEK_R_QAM_INFO("\033[1;32;31m" "%s %s %d, sys = %d\n" "\033[m", __FILE__, __func__, __LINE__, sys);
	pDemodKernel->SetTVSysEx(pDemodKernel, sys, NULL);

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetTVSys
 *
 * Desc : Set TV System
 *
 * Parm : sys : TV System
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo)
{
	int ret = TUNER_CTRL_OK;

	TUNER_PARAM Param;
	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	REALTEK_R_QAM_INFO("\033[1;32;31m" "%s %s %d, system = %d\n" "\033[m", __FILE__, __func__, __LINE__, sys);

	if (!IsOpenCableSys(sys)) {
		REALTEK_R_QAM_WARNING("Set TV system failed, unsupported TV system\n");

		return TUNER_CTRL_FAIL;
	}

	pRealtekRQAM->m_auto_tune_en = 0;
	pRealtekRQAM->m_acquire_stime = tv_osal_time();
	pDemodKernel->Lock(pDemodKernel);

	//TUNER_PARAM Param;
	if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, pDemodKernel->m_tv_sys, &Param) == TUNER_CTRL_OK) {
		if ((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola  != Param.if_agc_pola)) {
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;

			if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK) {
				pDemodKernel->Unlock(pDemodKernel);
				return  TUNER_CTRL_FAIL;
			}
		}
	}

	ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));
	pRealtekRQAM->m_acquire_sig_en = 1;
	pRealtekRQAM->m_auto_tune_en = 1;
	pRealtekRQAM->m_unlock_cnt = 0;
	pRealtekRQAM->m_low_snr_detect = 0;
	pRealtekRQAM->m_low_snr_recover = 0;
	pDemodKernel->Unlock(pDemodKernel);
	pDemodKernel->m_tv_sys = sys;

	return ret;
}


/*----------------------------------------------------------------------
 * Func : SetIF
 *
 * Desc : Set IF Parameter
 *
 * Parm : pParam : IF Param
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;
	pDemodKernel->Lock(pDemodKernel);

#ifdef REALTEK_R_QAM_SetIF_LGE
	pParam->freq = 6000000;
	REALTEK_R_QAM_INFO("REALTEK_R_QAM_SetIF_LGE\n");
#endif

	REALTEK_R_QAM_INFO("SetIF: IF_freq = "PT_U32BITS", spectrumn inv = %u, agc_pola = %u\n", pParam->freq, pParam->inversion, pParam->agc_pola);

	ret = DECODE_RET(pDemod->SetIfFreqHz(pDemod, pParam->freq));
	ret |= DECODE_RET(pDemod->SetSpectrumMode(pDemod, pParam->inversion));
	ret |= DECODE_RET(pDemod->SetAagcLoopPol(pDemod, pParam->agc_pola));// IFAGC polarity
	//ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_POLAR_IFAGC, pParam->agc_pola));
	//ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OPT_IF_AAGC_DRIVE, 1));  // push pull mode
	rtd_maskl(DVBC_OPT_RF_AAGC_DRIVE_reg, ~DVBC_OPT_RF_AAGC_DRIVE_opt_if_aagc_drive_mask, DVBC_OPT_RF_AAGC_DRIVE_opt_if_aagc_drive(0x1));
	//ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_PAR_IF_SD_IB, 1));   // agc if non-invert
	//rtd_maskl(DVBC_OPT_RF_AAGC_DRIVE_reg, ~DVBC_OPT_RF_AAGC_DRIVE_opt_par_if_sd_ib_mask, DVBC_OPT_RF_AAGC_DRIVE_opt_par_if_sd_ib(0x1));

	ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));
	pRealtekRQAM->m_low_snr_detect = 0;
	pRealtekRQAM->m_low_snr_recover = 0;
	pDemodKernel->m_if = *pParam;
	pDemodKernel->Unlock(pDemodKernel);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : SetMode
 *
 * Desc : Set operation mode of demod
 *
 * Parm : mode : Demod opreation mode
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	//unsigned char val;

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	switch (mode) {
	case TV_OP_MODE_NORMAL:
	case TV_OP_MODE_SCANING:
		REALTEK_R_QAM_INFO("demod enabled\n");
		//val = 1;
		//pDemod->RegAccess.Addr16Bit.SetRegBytes(pDemod, 0xe004, &val, 1);  // enable QAM demod & request share memory for ATSC&QAM demod
		rtd_maskl(DVBC_OPT_QAM_ON_reg, ~(DVBC_OPT_QAM_ON_opt_qam_on_mask), DVBC_OPT_QAM_ON_opt_qam_on(0x1));
		pDemodKernel->AcquireSignal(pDemodKernel, 0);              // reset demod....
		break;
	case TV_OP_MODE_STANDBY:
		REALTEK_R_QAM_INFO("demod disabled\n");
		//val = 0;
		//pDemod->RegAccess.Addr16Bit.SetRegBytes(pDemod, 0xe004, &val, 1);  // disable QAM demod & release share memory for ATSC&QAM demod)
		rtd_maskl(DVBC_OPT_QAM_ON_reg, ~(DVBC_OPT_QAM_ON_opt_qam_on_mask), DVBC_OPT_QAM_ON_opt_qam_on(0x0));
		break;
	case TV_OP_MODE_ATV_NORMAL:
	case TV_OP_MODE_TP_ONLY:
		break;
	default:
		REALTEK_R_QAM_INFO("unknown TV_OP_MODE\n");
		break;
	}

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetTSMode
 *
 * Desc : Set mode of transport stream output
 *
 * Parm : pParam : TS Param
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;
	pDemodKernel->Lock(pDemodKernel);

	switch (pParam->mode) {
	case PARALLEL_MODE:
		REALTEK_R_QAM_DBG("TS Mode: PARALLEL\n");
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_SERIAL,   0));
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_CDIV_PH0, 7));
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_CDIV_PH1, 7));
		rtd_maskl(DVBC_OC_CKOUT_PWR_reg, ~DVBC_OC_CKOUT_PWR_oc_serial_mask, DVBC_OC_CKOUT_PWR_oc_serial(0x0));
		rtd_maskl(DVBC_OC_CDIV_PH0_reg, ~DVBC_OC_CDIV_PH0_oc_cdiv_ph0_mask, DVBC_OC_CDIV_PH0_oc_cdiv_ph0(0x7));
		rtd_maskl(DVBC_OC_CDIV_PH0_reg, ~DVBC_OC_CDIV_PH0_oc_cdiv_ph1_mask, DVBC_OC_CDIV_PH0_oc_cdiv_ph1(0x7));
		break;
	case SERIAL_MODE:
		REALTEK_R_QAM_DBG("TS Mode: SERIAL\n");
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_SERIAL,   1));
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_CDIV_PH0, 0));
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_CDIV_PH1, 0));
		rtd_maskl(DVBC_OC_CKOUT_PWR_reg, ~DVBC_OC_CKOUT_PWR_oc_serial_mask, DVBC_OC_CKOUT_PWR_oc_serial(0x1));
		rtd_maskl(DVBC_OC_CDIV_PH0_reg, ~DVBC_OC_CDIV_PH0_oc_cdiv_ph0_mask, DVBC_OC_CDIV_PH0_oc_cdiv_ph0(0x0));
		rtd_maskl(DVBC_OC_CDIV_PH0_reg, ~DVBC_OC_CDIV_PH0_oc_cdiv_ph1_mask, DVBC_OC_CDIV_PH0_oc_cdiv_ph1(0x0));
		break;
	default:
		REALTEK_R_QAM_DBG("unknown TS Mode\n");
		ret = TUNER_CTRL_FAIL;
		break;
	}

	switch (pParam->clk_pol) {
	case RISING_EDGE:
		REALTEK_R_QAM_DBG("MPEG CLK POL: RISING_EDGE\n");
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_CKOUTPAR, 1));
		rtd_maskl(DVBC_OC_CKOUT_PWR_reg, ~DVBC_OC_CKOUT_PWR_oc_ckoutpar_mask, DVBC_OC_CKOUT_PWR_oc_ckoutpar(0x1));
		break;
	case FALLING_EDGE:
		REALTEK_R_QAM_DBG("MPEG CLK POL: FALLING_EDGE\n");
		//ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_CKOUTPAR, 0));
		rtd_maskl(DVBC_OC_CKOUT_PWR_reg, ~DVBC_OC_CKOUT_PWR_oc_ckoutpar_mask, DVBC_OC_CKOUT_PWR_oc_ckoutpar(0x0));
		break;
	default:
		REALTEK_R_QAM_DBG("unknown MPEG CLK POL\n");
		ret = TUNER_CTRL_FAIL;
		break;
	}

	DtvDemodTPOutEn(pParam->tsOutEn);

	if (ret == TUNER_CTRL_OK)
		pDemodKernel->m_ts = *pParam;

	pDemodKernel->Unlock(pDemodKernel);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : GetLockStatus
 *
 * Desc : Get Demod Lock Status
 *
 * Parm : pLock : Lock Status
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{
	int ret = TUNER_CTRL_OK;
	int Val;

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	if (DtvIsDemodClockEnable(RTK_DEMOD_MODE_QAM) == 0)
		return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);
	ret = DECODE_RET(pDemod->IsFrameLocked(pDemod, &Val));

	if (ret == TUNER_CTRL_OK)
		*pLock = DECODE_LOCK(Val);
	else
		*pLock = 0;

	if (*pLock == 1) {
		pRealtekRQAM->m_unlock_cnt = 0;
		pRealtekRQAM->m_autotune_stime = tv_osal_time() + AUTODETECT_OPENCABLE_SIG_UPDATE_INTERVAL_MS;
		pRealtekRQAM->m_autotune_GetLockTime = tv_osal_time() + AUTODETECT_OPENCABLE_GET_LOCK_INTERVAL_MS;
	} else {
		pRealtekRQAM->m_unlock_cnt = pRealtekRQAM->m_unlock_cnt + 1;
		pRealtekRQAM->m_autotune_GetLockTime = tv_osal_time() + AUTODETECT_OPENCABLE_GET_LOCK_INTERVAL_MS;
	}

	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_R_QAM_INFO("Frame Lock = %u, m_unlock_cnt = %u\n", *pLock, pRealtekRQAM->m_unlock_cnt);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : GetSignalInfo
 *
 * Desc : Get Signal Information
 *
 * Parm : pInfo : Siganl Information
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	int qam_mode = 0;
	U32BITS spec_mode = 0;
	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	pInfo->mod = TV_MODULATION_QAM;
	pDemod->GetQamMode(pDemod, &qam_mode);

	switch (qam_mode) {
	default:
	case QAM_QAM_64:
		pInfo->qam.constellation = QAM_CONST_64;
		break;
	case QAM_QAM_256:
		pInfo->qam.constellation = QAM_CONST_256;
		break;
	case QAM_QAM_AUTO:
		pInfo->qam.constellation  = QAM_CONST_AUTO;
		break;
	}

	//pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_SPEC_MODE, &spec_mode);
	spec_mode = DVBC_OPT_DDC_LPF_SCALING_get_specmode(rtd_inl(DVBC_OPT_DDC_LPF_SCALING_reg));
	REALTEK_R_QAM_INFO("spec_mode = "PT_U32BITS"\n", spec_mode);

	if (spec_mode)
		pInfo->qam.spectrum_inverse = QAM_SPEC_INVERSE;
	else
		pInfo->qam.spectrum_inverse = QAM_SPEC_NORMAL;

	REALTEK_R_QAM_INFO("GetSignalInfo: QAM_MODE = %u, SPEC_INV = %u\n", pInfo->qam.constellation, pInfo->qam.spectrum_inverse);
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : GetSignalQuality
 *
 * Desc : Get Signal Information
 *
 * Parm : id : Type of Quality Information
 *        pQuality : Siganl Quality
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality)
{
	int ret = TUNER_CTRL_OK;

	unsigned char FrameLock = 0;
	int LockStatus = 0, TxRSSIdBm = 0;
	S32BITS SnrNum, SnrDen;
	U32BITS BerNum = 1, BerDen = 1;
	U32BITS PerNum = 1, PerDen = 1;
	S32BITS IfAgc = -1;

#ifndef GET_SIGNAL_STRENGTH_FROM_SNR
	U32BITS SignalStrength = 0;
#endif

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	if (DtvIsDemodClockEnable(RTK_DEMOD_MODE_QAM) == 0)
		return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);
	pDemodKernel->GetLockStatus(pDemodKernel, &FrameLock);

	switch (id) {
	case TV_QUAL_NOSIG:
		ret = pDemod->IsNoSig(pDemod, &LockStatus);
		pQuality->nosig = (LockStatus) ? 1 : 0;
		break;

	case TV_QUAL_INNERLOCK:
		ret = pDemod->IsInnerLocked(pDemod, &LockStatus);
		pQuality->inner = (LockStatus) ? 1 : 0;
		break;

	case TV_QUAL_SNR:
		ret = DECODE_RET(pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen));

		if (ret == TUNER_CTRL_OK)
			pQuality->snr = SnrNum * 10000 / SnrDen;
		break;

	case TV_QUAL_BEC:
	case TV_QUAL_PEC:
	case TV_QUAL_BER:
	case TV_QUAL_PER:
	case TV_QUAL_UCBLOCKS:
		ret = (FrameLock) ? DECODE_RET(pDemod->GetErrorRate(pDemod, 0, 800, &BerNum, &BerDen, &PerNum, &PerDen)) : TUNER_CTRL_OK;
		REALTEK_R_QAM_INFO("FrameLock = %u, BerNum = "PT_U32BITS", BerDen = "PT_U32BITS", PerNum = "PT_U32BITS", PerDen = "PT_U32BITS"\n", FrameLock, BerNum, BerDen, PerNum, PerDen);

		if (id == TV_QUAL_BER)
			pQuality->ber = BerNum * 10000 / BerDen;
		else if (id == TV_QUAL_PER)
			pQuality->per = PerNum * 10000 / PerDen;
		else if (id == TV_QUAL_BEC) {
			pQuality->layer_ber.ber_num = BerNum;
			pQuality->layer_ber.ber_den = BerDen;
		} else if (id == TV_QUAL_PEC) {
			pQuality->layer_per.per_num = PerNum;
			pQuality->layer_per.per_den = PerDen;
		} else if (id == TV_QUAL_UCBLOCKS)
			pQuality->ucblocks = PerNum;

		break;

	case TV_QUAL_SIGNAL_STRENGTH:
		TxRSSIdBm = (int)pQuality->rflevel;

#ifdef GET_SIGNAL_STRENGTH_FROM_SNR //Default Formula
		ret = pDemodKernel->GetSignalQuality(pDemodKernel, TV_QUAL_SNR, pQuality);
		if (ret == TUNER_CTRL_OK)
			pQuality->strength = Snr2Str(pQuality->snr / 10000); //Convert SNR to Signal Strength

#else
		ret = DECODE_RET(pDemod->GetSignalStrength(pDemod, TxRSSIdBm, &SignalStrength));
		if (ret == TUNER_CTRL_OK)
			pQuality->strength = (unsigned char)SignalStrength;
#endif

		REALTEK_R_QAM_DBG("TxRSSIdBm = %d, strength = %u\n", TxRSSIdBm, pQuality->strength);
		break;

	case TV_QUAL_SIGNAL_QUALITY:
		ret = pDemodKernel->GetSignalQuality(pDemodKernel, TV_QUAL_BER, pQuality);

		if (ret == TUNER_CTRL_OK)
			pQuality->quality = Ber2Quality(pQuality->ber);  	//Convert BER to Signal Quality
		break;

	case TV_QUAL_AGC:
		ret = pDemod->GetIfAgc(pDemod, &IfAgc);

		if (ret == TUNER_CTRL_OK)
			pQuality->agc = ((IfAgc + 1023) * 100) / 2048;
		break;

	default:
		REALTEK_R_QAM_DBG("unknown ENUM_TV_QUAL id\n");
		ret = TUNER_CTRL_FAIL;
		break;
	}

	pDemodKernel->Unlock(pDemodKernel);
	return ret;
}


/*----------------------------------------------------------------------
 * Func : GetCarrierOffset
 *
 * Desc : Get the carrier offset.
 *
 * Parm : pOffset : carrier frequency offset output (unit : Hz)
 *
 *          Note: the offset value is only valid as demod is on lock stage.
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	int ret = TUNER_CTRL_OK;
	unsigned char LockStatus = 0;

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	if (DtvIsDemodClockEnable(RTK_DEMOD_MODE_QAM) == 0)
		return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);
	pDemodKernel->GetLockStatus(pDemodKernel, &LockStatus);
	pDemodKernel->Unlock(pDemodKernel);

	if (LockStatus == 0) {
		REALTEK_R_QAM_INFO("GetCarrierOffset fail beacause demod is unlock\n");
		return TUNER_CTRL_FAIL;
	}

	pDemodKernel->Lock(pDemodKernel);
	ret = DECODE_RET(pDemod->GetCrOffsetHz(pDemod, pOffset));

	if (ret == TUNER_CTRL_OK) {
		/*----------------------------------------------------------------
		  CrOffset = Current Frequency - Target Frequency
		  returned offset = Target Freq - Current Offset = -CrOffset
		  Note: REALTEK_R QAM Demod will handle IF polarity automatically
		-----------------------------------------------------------------*/
		*pOffset *= -1;
	} else
		*pOffset = 0;

	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_R_QAM_INFO("CarrierOffset = "PT_S32BITS" Hz\n", *pOffset);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : GetDebugLogBuf
 *
 * Desc : Get the debug log Buf.
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	return ret;
}


/*----------------------------------------------------------------------
 * Func : AcquireSignal
 *
 * Desc : Reset Demodulator and Acquire Signal.
 *        This function should be called when channel changed.
 *
 * Parm : WaitSignalLock : wait until signal lock
 *          0 : don't wait,
 *          others: wait for signal lock
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned long cur_time;
#endif

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	cur_time = tv_osal_time();

	while ((tv_osal_time() - cur_time) <= 15) {
		if (pRealtekRQAM->m_acquire_sig_en == 0)
			return TUNER_CTRL_OK;

		tv_osal_msleep(5);
	}
	return TUNER_CTRL_FAIL;
#else
	REALTEK_R_QAM_AcquireSignalThread(pDemodKernel, WaitSignalLock);
	pRealtekRQAM->m_autotune_stime = tv_osal_time() + AUTODETECT_OPENCABLE_SIG_UPDATE_INTERVAL_MS;
	pRealtekRQAM->m_autotune_GetLockTime = tv_osal_time() + AUTODETECT_OPENCABLE_GET_LOCK_INTERVAL_MS;
	pRealtekRQAM->m_autotune_SfiTrkTime = tv_osal_time() + AUTODETECT_OPENCABLE_SFI_TRK_INTERVAL_MS;
	return TUNER_CTRL_OK;
#endif
}

#define OC_ACQ_TR_CNT 2
int REALTEK_R_QAM_AcquireSignalThread(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{
	int ret = TUNER_CTRL_OK;

	int lock = 0;
	unsigned char IsNoSignal, aagc_ld = 0;
	unsigned long stime = 0;
	int IsNoSignalTr = 0;
	S32BITS snr = 0, SnrNum = 0, SnrDen = 0;
	unsigned int fsm_state_o = 0, tr_ld_o = 0;
	int PAPR_CCFO_CFO = 0;
	unsigned int ccfo_done = 0, papr_ccfo_max_value= 0, est_papr_ccfo_idx = 0;

#ifdef OC_AutoDetectMode
	unsigned long auto_mode_etime = 0;
	U32BITS CSR = 0 , FSR = 0;
	S32BITS CCFO = 0;
	unsigned char QamMode;//sfi_acq_try = 0;
	unsigned int sfi_cancel_wn = 0;//SfiWaitTime = 100;
	//unsigned char sfi_cancel_wn_offset = 0;
#else
	S32BITS SymbolRateDet = 0, Powdiff = 0;
#endif

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;
	REALTEK_R_QAM_INFO("\033[1;32;32m" "%s %s %d, WaitSignalLock = %u\n" "\033[m", __FILE__, __func__, __LINE__, WaitSignalLock);

	pDemodKernel->Lock(pDemodKernel);

	stime = tv_osal_time();

#ifdef OC_AutoDetectMode
	pDemod->SetIfFreqHz(pDemod, pDemodKernel->m_if.freq);
#ifndef CONFIG_CUSTOMER_TV002
	tv_osal_msleep(pDemod->Symbolratedelay);//AGC stable
#endif
	ret = DECODE_RET(pDemod->AutoMode(pDemod, &IsNoSignal, &CSR, &FSR, &CCFO, &QamMode));
	auto_mode_etime = tv_osal_time();

	if (IsNoSignal) {
		REALTEK_R_QAM_INFO("\033[1;32;31m" "QAM NO SIGNAL!!! Total Acq Time="PT_U32BITS", AutoMode Time="PT_U32BITS", IsNoSignal=%u, CSR="PT_U32BITS" KHz, FSR="PT_U32BITS" Hz, CCFO="PT_S32BITS" Hz SFI_En=%u\n",
		tv_osal_time() - pRealtekRQAM->m_acquire_stime, auto_mode_etime - pRealtekRQAM->m_acquire_stime, IsNoSignal, CSR, FSR, CCFO, gCorse_sfi_found);
		goto AcquireSignal_end;
	}

	if (QamMode == QAM_QAM_64) {
		REALTEK_R_QAM_INFO("Demod Set to 64QAM\n");
		pDemodKernel->m_tv_sys = TV_SYS_OPENCABLE_64;
		ret = DECODE_RET(pDemod->SetQamMode(pDemod, QAM_QAM_64));
		ret |= DECODE_RET(pDemod->SetSymbolRateHz(pDemod, 5056941));   //5.056941 MHz
		ret |= DECODE_RET(pDemod->SetAlphaMode(pDemod, QAM_ALPHA_0P15));
	} else if (QamMode == QAM_QAM_256) {
		REALTEK_R_QAM_INFO("Demod Set to 256QAM\n");
		pDemodKernel->m_tv_sys = TV_SYS_OPENCABLE_256;
		ret = DECODE_RET(pDemod->SetQamMode(pDemod, QAM_QAM_256));
		ret |= DECODE_RET(pDemod->SetSymbolRateHz(pDemod, 5360537));   //5.360537 MHz
		ret |= DECODE_RET(pDemod->SetAlphaMode(pDemod, QAM_ALPHA_0P12));
	} else {
		REALTEK_R_QAM_INFO("Double Check SymbolRateDet as No Signal\n");
		goto AcquireSignal_end;
	}
	if(pDemodKernel->m_if.inversion == 0)
		pDemod->SetIfFreqHz(pDemod, pDemodKernel->m_if.freq + CCFO);
	else
		pDemod->SetIfFreqHz(pDemod, pDemodKernel->m_if.freq - CCFO);
	//REALTEK_R_QAM_INFO("[ccfo compensate]IF freq = 0x%x\n", (rtd_inl(DVBC_OPT_DDC_FREQ_reg) | ((rtd_inl(DVBC_OPT_DDC_FREQ_1_reg)&0x7f) <<8)));
#else
	pDemod->SetFftMode(pDemod, REALTEK_R_OC_SRS);

	if (DECODE_RET(pDemod->NbdAndSrs(pDemod, &IsNoSignal, &Powdiff, &SymbolRateDet, 1)) != TUNER_CTRL_OK) {
		pDemodKernel->Unlock(pDemodKernel);
		pRealtekRQAM->m_acquire_sig_en = 0;
		
		return TUNER_CTRL_FAIL;
	}

	pDemod->SetFftMode(pDemod, REALTEK_R_CCFO);
	REALTEK_R_QAM_INFO("[SRS]IsNoSignal = %u, SymbolRateDet = "PT_S32BITS", SRS Time = %lu\n", IsNoSignal, SymbolRateDet, tv_osal_time() - stime);

	if (IsNoSignal) {
		goto AcquireSignal_end;
	}

	if (SymbolRateDet >= 5026941 && SymbolRateDet <= 5086941) {	//+-0.03MHz Margin for 64/256QAM SR
		REALTEK_R_QAM_INFO("Demod Set to 64QAM\n");
		pDemodKernel->m_tv_sys = TV_SYS_OPENCABLE_64;
		ret = DECODE_RET(pDemod->SetQamMode(pDemod, QAM_QAM_64));
		ret |= DECODE_RET(pDemod->SetSymbolRateHz(pDemod, 5056941));   //5.056941 MHz
		ret |= DECODE_RET(pDemod->SetAlphaMode(pDemod, QAM_ALPHA_0P15));
	} else if (SymbolRateDet >= 5330537 && SymbolRateDet <= 5390537) {
		REALTEK_R_QAM_INFO("Demod Set to 256QAM\n");
		pDemodKernel->m_tv_sys = TV_SYS_OPENCABLE_256;
		ret = DECODE_RET(pDemod->SetQamMode(pDemod, QAM_QAM_256));
		ret |= DECODE_RET(pDemod->SetSymbolRateHz(pDemod, 5360537));   //5.360537 MHz
		ret |= DECODE_RET(pDemod->SetAlphaMode(pDemod, QAM_ALPHA_0P12));
	} else {
		REALTEK_R_QAM_INFO("Double Check SymbolRateDet as No Signal\n");
		goto AcquireSignal_end;
	}
#endif

	pDemod->SoftwareReset(pDemod);

#ifdef OC_AutoDetectMode
	stime = tv_osal_time();
#endif

	while ((tv_osal_time() - stime) <= 1600) {

		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));//stb INRX_MO
		fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;
		aagc_ld = rtd_inl(DVBC_INRX_MO2_reg) & 0x1;
		tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));
		pDemod->IsNoSig(pDemod, &IsNoSignalTr);
		pDemod->IsFrameLocked(pDemod, (int*)&lock);
		ccfo_done = DVBC_SRS_NBD_DONE_get_ccfo_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));
		//sfi_cancel_wn_offset = DVBC_SFI_CANCEL_CTRL7_get_sfi_cancel_wn_offset(rtd_inl(DVBC_SFI_CANCEL_CTRL7_reg));
		//REALTEK_R_QAM_INFO("IsNoSignalTr = %d, Signal Lock = %u fsm_state_o=%u tr_ld_o=%u aagc_ld=%u sfi_cancel_wn_offset=%u\n", IsNoSignalTr, lock, fsm_state_o, tr_ld_o, aagc_ld, sfi_cancel_wn_offset);

		//Get CCFO
		if(ccfo_done == 1)
		{
			papr_ccfo_max_value = (rtd_inl(DVBC_CCFO_PAPR_MAX0_reg) | (rtd_inl(DVBC_CCFO_PAPR_MAX1_reg) << 8));
			est_papr_ccfo_idx = ((rtd_inl(DVBC_CCFO_PAPR_IDX1_reg) & 0x1f) << 8) + (rtd_inl(DVBC_CCFO_PAPR_IDX0_reg) & 0xff);
			PAPR_CCFO_CFO = (est_papr_ccfo_idx < 4096) ? ((int)est_papr_ccfo_idx*6300/8192) : (((int)est_papr_ccfo_idx - 8192)*6300/8192);
			//REALTEK_R_QAM_INFO("NEW PAPR_CCFO_CFO=%d KHz est_papr_ccfo_idx=%u papr_ccfo_max_value=%u\n", PAPR_CCFO_CFO, est_papr_ccfo_idx, papr_ccfo_max_value);
		}
#ifdef OC_AutoDetectMode
#if 0 //merlin5
		if(gCorse_sfi_found == 1)
		{
			sfi_cancel_wn = ((gSfi_cancel_wn_15_8 << 8) | gSfi_cancel_wn_7_0);
			if(aagc_ld == 1)
			{
				pDemod->SetSfiCancel(pDemod);//sfi cancel
			}

			if(((tv_osal_time() - stime) > SfiWaitTime) && (DVBC_SFI_CANCEL_CTRL2_get_opt_sfi_cancel_val_mu(rtd_inl(DVBC_SFI_CANCEL_CTRL2_reg)) == 0x3))
			{
				REALTEK_R_QAM_INFO("Go mid BW\n");
				rtd_maskl(DVBC_SFI_CANCEL_CTRL2_reg, ~(DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu_mask), DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu(0x5));
			}

			if(((tv_osal_time() - stime) > (SfiWaitTime + 30)) && (DVBC_SFI_CANCEL_CTRL2_get_opt_sfi_cancel_val_mu(rtd_inl(DVBC_SFI_CANCEL_CTRL2_reg)) == 0x5))
			{
				REALTEK_R_QAM_INFO("Go small BW\n");
				rtd_maskl(DVBC_SFI_CANCEL_CTRL2_reg, ~(DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu_mask), DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu(0x7));
			}

			if(((tv_osal_time() - stime) > (SfiWaitTime + 50)) && (DVBC_OPT_RST_STATE0_ON_get_opt_retry_on(rtd_inl(DVBC_OPT_RST_STATE0_ON_reg)) == 0))
			{
				REALTEK_R_QAM_INFO("Release Fix State & Retry on\n");
				rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(0) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));
				rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));//retry on
			}

			if(((tv_osal_time() - stime) > 1500) && (fsm_state_o <= 10))
			{
				if(sfi_acq_try == 0)
				{
					SfiWaitTime = 150;
					//restore mu BW
					rtd_maskl(DVBC_SFI_CANCEL_CTRL2_reg, ~(DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu_mask), DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu(0x3));
					sfi_cancel_wn = sfi_cancel_wn + 1;//right shift 0.5bin
					rtd_outl(DVBC_FSFI_CTRL5_reg, ((sfi_cancel_wn >> 8) & 0x7f));
					rtd_outl(DVBC_FSFI_CTRL4_reg, (sfi_cancel_wn & 0xff));
					rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(1) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));
					rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));//retry off
					REALTEK_R_QAM_INFO("\033[1;32;31m" "TR not correct in Acq, retry %u times, fsm_state=%u, tr=%u SfiWaitTime=%u cancel_wn_14_8=0x%x cancel_wn_7_0=0x%x\n" "\033[m", sfi_acq_try, fsm_state_o, tr_ld_o, SfiWaitTime, rtd_inl(DVBC_FSFI_CTRL5_reg), rtd_inl(DVBC_FSFI_CTRL4_reg));
					stime = tv_osal_time();
					pDemod->SoftwareReset(pDemod);
				}
				else if(sfi_acq_try == 1)
				{
					SfiWaitTime = 150;
					//restore mu BW
					rtd_maskl(DVBC_SFI_CANCEL_CTRL2_reg, ~(DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu_mask), DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu(0x3));
					sfi_cancel_wn = sfi_cancel_wn - 1;//left shift 0.5bin
					rtd_outl(DVBC_FSFI_CTRL5_reg, ((sfi_cancel_wn >> 8) & 0x7f));
					rtd_outl(DVBC_FSFI_CTRL4_reg, (sfi_cancel_wn & 0xff));
					rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(1) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));
					rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));//retry off
					REALTEK_R_QAM_INFO("\033[1;32;31m" "TR not correct in Acq, retry %u times, fsm_state=%u, tr=%u SfiWaitTime=%u cancel_wn_14_8=0x%x cancel_wn_7_0=0x%x\n" "\033[m", sfi_acq_try, fsm_state_o, tr_ld_o, SfiWaitTime, rtd_inl(DVBC_FSFI_CTRL5_reg), rtd_inl(DVBC_FSFI_CTRL4_reg));
					stime = tv_osal_time();
					pDemod->SoftwareReset(pDemod);
				}
				sfi_acq_try++;
			}
		}
#else //mark2, merlin7
		if(gCorse_sfi_found == 1)
		{
			sfi_cancel_wn = ((gSfi_cancel_wn_15_8 << 8) | gSfi_cancel_wn_7_0);
			pDemod->SetSfiCancel(pDemod);//sfi cancel
		}
#endif
#endif

		if (lock)
		{
			//Get SNR
			pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen);
			snr = (SnrNum * 10000) / SnrDen;
#ifdef OC_AutoDetectMode
			REALTEK_R_QAM_INFO("\033[1;32;36m" "[Auto]QAM LOCK!!! Total Acq Time="PT_U32BITS" AutoMode Time="PT_U32BITS", Local Time="PT_U32BITS", IsNoSignal=%u, CSR="PT_U32BITS" KHz, FSR="PT_U32BITS" Hz, CCFO="PT_S32BITS" Hz, Locked CCFO=%dKHz, SFI_En=%u, Snr="PT_S32BITS"/10000, fsm_state=%u, tr=%u\n" "\033[m", 
				tv_osal_time() - pRealtekRQAM->m_acquire_stime, auto_mode_etime - pRealtekRQAM->m_acquire_stime, tv_osal_time() - stime, IsNoSignal, CSR, FSR, CCFO, PAPR_CCFO_CFO, gCorse_sfi_found, snr, fsm_state_o, tr_ld_o);
#else
			REALTEK_R_QAM_INFO("\033[1;32;36m" "[DisAuto]QAM LOCK!!! Demod Lock Time="PT_U32BITS", IsNoSignal=%u, Locked CCFO=%dKHz Snr="PT_S32BITS"/10000, fsm_state=%u, tr=%u\n" "\033[m", 
				tv_osal_time() - stime, IsNoSignal, PAPR_CCFO_CFO, snr, fsm_state_o, tr_ld_o);
#endif
			break;
		}

		if (((tv_osal_time() - stime) > 800) && (DVBC_FSFI_CTRL3_get_opt_sfi_trk_comp_en_pset(rtd_inl(DVBC_FSFI_CTRL3_reg))== 0))
			break;

		tv_osal_msleep(10);
	}

	if (lock == 0)
	{
		pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen);
		snr = (SnrNum * 10000) / SnrDen;
#ifdef OC_AutoDetectMode 
		REALTEK_R_QAM_INFO("\033[1;32;36m" "[Auto]QAM Unlock!!! Total Acq Time="PT_U32BITS" AutoMode Time="PT_U32BITS", Local Time="PT_U32BITS", IsNoSignal=%u, CSR="PT_U32BITS" KHz, FSR="PT_U32BITS" Hz, CCFO="PT_S32BITS" Hz, SFI_En=%u, Snr="PT_S32BITS"/10000, fsm_state=%u, tr=%u\n" "\033[m", 
				tv_osal_time() - pRealtekRQAM->m_acquire_stime, auto_mode_etime - pRealtekRQAM->m_acquire_stime, tv_osal_time() - stime, IsNoSignal, CSR, FSR, CCFO, gCorse_sfi_found, snr, fsm_state_o, tr_ld_o);
#else
		REALTEK_R_QAM_INFO("\033[1;32;36m" "[DisAuto]QAM UnLOCK!!! Demod Lock Time="PT_U32BITS", IsNoSignal=%u, Locked CCFO=%dKHz Snr="PT_S32BITS"/10000, fsm_state=%u, tr=%u\n" "\033[m", 
				tv_osal_time() - stime, IsNoSignal, PAPR_CCFO_CFO, snr, fsm_state_o, tr_ld_o);
#endif
	}

AcquireSignal_end:
	pDemodKernel->Unlock(pDemodKernel);
	pRealtekRQAM->m_acquire_sig_en = 0;
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : ScanSignal
 *
 * Desc : Ask demodulator to Search signal within specified range
 *        This function should be called every time the channel changed.
 *
 * Parm : pOffset : Offset Output
 *        range   : Scan Range
 *          SCAN_RANGE_NONE : center only
 *          SCAN_RANGE_1_6  : (-)1/6 MHz
 *          SCAN_RANGE_2_6  : (-)2/6 MHz
 *          SCAN_RANGE_3_6  : (-)3/6 MHz
 *
 * Retn : TUNER_CTRL_OK        : signal found and locked
 *        TUNER_CTRL_FAIL      : error occured during scan channel
 *        TUNER_CTRL_NO_SIGNAL : no signal found in specified range
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset,	SCAN_RANGE range)
{
	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : ForwardI2C
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OPT_I2C_RELAY, (on_off) ? 1 : 0) != FUNCTION_SUCCESS)
	//	ret = TUNER_CTRL_FAIL;
	rtd_maskl(RTK_DTV_DEMOD_8051_GP_INT0_N_reg, ~RTK_DTV_DEMOD_8051_GP_INT0_N_bootcode_en_mask, ((on_off) ? 1 : 0) << 7);

	pDemodKernel->Unlock(pDemodKernel);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : AutoTune
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_AutoTune(DEMOD* pDemodKernel)
{
	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	//if (DtvIsDemodClockEnable(RTK_DEMOD_MODE_QAM) == 0)
	//	return TUNER_CTRL_OK;

	//pDemodKernel->Lock(pDemodKernel);
	//pDemod->UpdateFunction(pDemod);
	//pDemodKernel->Unlock(pDemodKernel);

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : AutoTune
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_KernelAutoTune(DEMOD* pDemodKernel)
{
#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned char Lock = 0;
#endif
	unsigned long cur_time = 0;
	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	if (pRealtekRQAM->m_auto_tune_en == 0)
		return TUNER_CTRL_OK;

	cur_time = tv_osal_time();

	if (pRealtekRQAM->m_acquire_sig_en) {
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
		REALTEK_R_QAM_AcquireSignalThread(pDemodKernel, 1);
		pRealtekRQAM->m_autotune_stime = tv_osal_time() + AUTODETECT_OPENCABLE_SIG_UPDATE_INTERVAL_MS;
		pRealtekRQAM->m_autotune_GetLockTime = tv_osal_time() + AUTODETECT_OPENCABLE_GET_LOCK_INTERVAL_MS;
#endif
	} else if (((cur_time > pRealtekRQAM->m_autotune_stime) || ((pRealtekRQAM->m_autotune_stime - cur_time) > AUTODETECT_OPENCABLE_SIG_UPDATE_INTERVAL_MS)) && pRealtekRQAM->m_unlock_cnt > 3) {
#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE //OpenCable QAM Mode Auto Detection by Thread or LGE WebOS
		REALTEK_R_QAM_AcquireSignalThread(pDemodKernel, 1);
		pRealtekRQAM->m_autotune_stime = tv_osal_time() + AUTODETECT_OPENCABLE_SIG_UPDATE_INTERVAL_MS;
		pRealtekRQAM->m_autotune_GetLockTime = tv_osal_time() + AUTODETECT_OPENCABLE_GET_LOCK_INTERVAL_MS;
		REALTEK_R_QAM_INFO("[KernelAutoTune] OpenCable QAM Mode Auto Detection !!!\n");
#endif
	} else if ((cur_time > pRealtekRQAM->m_autotune_GetLockTime) || ((pRealtekRQAM->m_autotune_GetLockTime - cur_time) > AUTODETECT_OPENCABLE_GET_LOCK_INTERVAL_MS)) {
#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
		REALTEK_R_QAM_GetLockStatus(pDemodKernel, &Lock);
		REALTEK_R_QAM_DBG("[KernelAutoTune] OpenCable LOCK status (0:unlock, 1:lock):---------(%u)\n", Lock);
#endif
	} else {
		pDemodKernel->Lock(pDemodKernel);
		pDemod->UpdateFunction(pDemod);
		pDemodKernel->Unlock(pDemodKernel);
	}
#ifdef OC_AutoDetectMode
	if ((cur_time > pRealtekRQAM->m_autotune_SfiTrkTime) || ((pRealtekRQAM->m_autotune_SfiTrkTime - cur_time) > AUTODETECT_OPENCABLE_SFI_TRK_INTERVAL_MS)) {
		pDemod->SfiFullTracking(pDemod);//sfi cancel
		pRealtekRQAM->m_autotune_SfiTrkTime = tv_osal_time() + AUTODETECT_OPENCABLE_SFI_TRK_INTERVAL_MS;
	}
#endif

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : InspectSignal
 *
 * Desc : Inspect Signal Info (for Debug Purpose)
 *
 * Parm : pBuff : message buffer
 *        BufferSize : size of message buffer
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_InspectSignal(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	S32BITS IfAgc = -1;
	S32BITS RfAgc = -1;
	S32BITS TrOffsetPpm = -1;
	S32BITS CrOffsetHz  = -1;
	U32BITS DiAgc = 0;
	int AgcLock = 0;
	int EqLock = 0;
	int FrameLock = 0;
	U32BITS BerNum = 1;
	U32BITS BerDen = 1;
	U32BITS PerNum = 1;
	U32BITS PerDen = 1;
	U32BITS MpegSyncMode = 0;
	S32BITS SnrDbNum = 1;
	S32BITS SnrDbDen = 1;

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	if (DtvIsDemodClockEnable(RTK_DEMOD_MODE_QAM) == 0)
		return TUNER_CTRL_OK;

	pDemodKernel->Lock(pDemodKernel);
	pDemod->GetRfAgc(pDemod, &RfAgc);
	pDemod->GetIfAgc(pDemod, &IfAgc);
	pDemod->GetDiAgc(pDemod, &DiAgc);
	pDemod->GetTrOffsetPpm(pDemod, &TrOffsetPpm);
	pDemod->GetCrOffsetHz(pDemod, &CrOffsetHz);
	pDemod->IsAagcLocked(pDemod, &AgcLock);
	pDemod->IsEqLocked(pDemod, &EqLock);
	pDemod->IsFrameLocked(pDemod, &FrameLock);
	pDemod->GetErrorRate(pDemod, 0, 800, &BerNum, &BerDen, &PerNum, &PerDen);
	pDemod->GetSnrDb(pDemod, &SnrDbNum, &SnrDbDen);
	//pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_OC_MPEG_SYNC_MODE, &MpegSyncMode);
	MpegSyncMode = DVBC_OC_MPEG_SYNC_MODE_get_oc_mpeg_sync_mode(rtd_inl(DVBC_OC_MPEG_SYNC_MODE_reg));
	pDemodKernel->Unlock(pDemodKernel);

	IfAgc = ((IfAgc + 1023) * 100) / 2048;
	RfAgc = ((RfAgc + 1023) * 100) / 2048;
	DiAgc = (DiAgc * 100) / 65536;

#if 0
	ret = printf("[REALTEK_R_QAM] lock(agc/eq/frame) = %d/%d/%d, agc(rf/if/di) = "PT_S32BITS"/"PT_S32BITS"/"PT_S32BITS", ofst(tr/cr) = "PT_S32BITS"/"PT_S32BITS", ber/per = %f/%f, snr = %f, mpeg_sync = %d\n",
				 AgcLock, EqLock, FrameLock,
				 RfAgc, IfAgc, DiAgc,
				 TrOffsetPpm, CrOffsetHz,
				 (double)BerNum / (double)BerDen,
				 (double)PerNum / (double)PerDen,
				 (double)SnrDbNum / (double) SnrDbDen,
				 MpegSyncMode);
#endif
	ret = 1;

#ifdef REALTEK_R_DUMP_SHARE_REG
	DtvDumpShareRegStatus();
#endif

	return ret;
}


/*----------------------------------------------------------------------
 * Func : Activate
 *
 * Desc : Activate REALTEK_R QAM Demod
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{
	unsigned char Val;
	int i;

	REALTEK_R_QAM* pRealtekRQAM;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_QAM_DRIVER_DATA*) pRealtekRQAM->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);
	if (DtvDemodInitialization(RTK_DEMOD_MODE_QAM, force_rst)) {
		REALTEK_R_QAM_WARNING("Demod PLL changed, reinit QAM Demod\n");
		// pll has been changed and demod has been reset
		// we need to re-init demod

		if (pDemod->Initialize(pDemod) != FUNCTION_SUCCESS)
			goto activate_demod_fail;

		ClearDemodReset(RTK_DEMOD_MODE_QAM);

		for (i = 0; i < pRealtekRQAM->m_TunerOptRegCnt && pRealtekRQAM->m_pTunerOptRegRbus; i++) {
			U32BITS Reg = pRealtekRQAM->m_pTunerOptRegRbus[i].RegBitNameRbus;
			U32BITS Val = pRealtekRQAM->m_pTunerOptRegRbus[i].ValueRbus;

			//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, Reg, Val) != FUNCTION_SUCCESS)
			//	goto activate_demod_fail;
			rtd_maskl(Reg, 0xffffff00, Val);
		}

		if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
			goto activate_demod_fail;

		if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK)
			goto activate_demod_fail;

		if (pDemodKernel->SetTVSys(pDemodKernel, pDemodKernel->m_tv_sys) != TUNER_CTRL_OK)
			goto activate_demod_fail;
	}

	//pDemod->RegAccess.Addr16Bit.GetRegBytes(pDemod, 0xe401, &Val, 1);
	Val = RTK_DTV_DEMOD_SYS_DEMOD_SEL_get_demod_sel(rtd_inl(RTK_DTV_DEMOD_SYS_DEMOD_SEL_reg));
	Val &= ~0x3;
	Val |= 0x1;
	//pDemod->RegAccess.Addr16Bit.SetRegBytes(pDemod, 0xe401, &Val, 1);
	rtd_maskl(RTK_DTV_DEMOD_SYS_DEMOD_SEL_reg, ~RTK_DTV_DEMOD_SYS_DEMOD_SEL_demod_sel_mask, RTK_DTV_DEMOD_SYS_DEMOD_SEL_demod_sel(Val));

	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_OK;

activate_demod_fail:
	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_FAIL;
}


/*----------------------------------------------------------------------
 * Func : SetTvMode
 *
 * Desc : Set Mode of REALTEK_R_ALL
 *
 * Parm : mode :    RTK_DEMOD_MODE_DVBT : DVBT mode
 *                  RTK_DEMOD_MODE_DVBC  : DVBC mode
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_QAM_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;

	REALTEK_R_QAM* pRealtekRQAM;
	pRealtekRQAM = (REALTEK_R_QAM*)(pDemodKernel->private_data);

	pDemodKernel->Lock(pDemodKernel);
	RTK_DEMOD_INFO("REALTEK_R_QAM_SetTvMode(%d)\n", mode);

	switch (mode) {
	case TV_SYS_TYPE_OPENCABLE:
		RTK_DEMOD_INFO("Set to OPENCABLE\n");
		ret = pDemodKernel->Init(pDemodKernel);
		ret |= pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts);

		if (ret != TUNER_CTRL_OK) {
			RTK_DEMOD_WARNING("Set REALTEK_R_QAM to QAM mode failed\n");
			goto set_demod_mode_failed;
		}
		break;
	default:
		REALTEK_R_QAM_INFO("unknown TV_SYSTEM_TYPE\n");
		goto set_demod_mode_failed;
	}

	ret = TUNER_CTRL_OK;
	RTK_DEMOD_INFO("Set REALTEK_R_QAM to appointed DTV mode successed\n");

	pDemodKernel->Unlock(pDemodKernel);
	return ret;

set_demod_mode_failed:
	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_FAIL;
}


void REALTEK_R_QAM_Lock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("LOCK 0x%x\n",&pDemodKernel->m_lock);
	//mutex_lock(&pDemodKernel->m_lock);
}


void REALTEK_R_QAM_Unlock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("UN_LOCK 0x%x\n",&pDemodKernel->m_lock);
	//mutex_unlock(&pDemodKernel->m_lock);
}


/*-----------------------------------------------------------------
 *     BASE_INTERFACE for REALTEK_R_QAM API
 *----------------------------------------------------------------*/
void ReleaseRealtekRQamDriver(REALTEK_R_QAM_DRIVER_DATA *pDriver)
{
	kfree(pDriver);
}


REALTEK_R_QAM_DRIVER_DATA* AllocRealtekRQamDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
)
{
	REALTEK_R_QAM_DRIVER_DATA* pDriver = (REALTEK_R_QAM_DRIVER_DATA*) kmalloc(
			sizeof(REALTEK_R_QAM_DRIVER_DATA) +
			sizeof(QAM_DEMOD_MODULE)   +
			sizeof(I2C_BRIDGE_MODULE)   +
			sizeof(BASE_INTERFACE_MODULE), GFP_KERNEL);

	if (pDriver) {
		BASE_INTERFACE_MODULE* pBIF;
		QAM_DEMOD_MODULE* pDemod;

		memset(pDriver, 0, sizeof(REALTEK_R_QAM_DRIVER_DATA));
		pDriver->pDemod                = (QAM_DEMOD_MODULE*)(((unsigned char*)pDriver) + sizeof(REALTEK_R_QAM_DRIVER_DATA));
		pDriver->pBaseInterface        = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(QAM_DEMOD_MODULE));
		pDriver->pI2CBridge            = (I2C_BRIDGE_MODULE*)(((unsigned char*)pDriver->pBaseInterface) + sizeof(BASE_INTERFACE_MODULE));
		pDriver->DeviceAddr            = Addr;
		pDriver->CrystalFreqHz         = CrystalFreq;

		// Init Base IF
		pBIF = pDriver->pBaseInterface;
		pBIF->I2cReadingByteNumMax      = RTK_DEMOD_BIF_RX_FIFO_DEPTH;
		pBIF->I2cWritingByteNumMax      = RTK_DEMOD_BIF_TX_FIFO_DEPTH;
		pBIF->I2cRead                   = __realtek_i2c_read;
		pBIF->I2cWrite                  = __realtek_i2c_write;
		pBIF->WaitMs                    = __realtek_wait_ms;
		pBIF->SetUserDefinedDataPointer = base_interface_SetUserDefinedDataPointer;
		pBIF->GetUserDefinedDataPointer = base_interface_GetUserDefinedDataPointer;
		pBIF->SetUserDefinedDataPointer(pBIF, (void*)pComm);

		// Init Demod Driver
		BuildRealtekRQamModule(&pDemod,
							   pDriver->pDemod,
							   pDriver->pBaseInterface,
							   pDriver->pI2CBridge,
							   Addr,
							   CrystalFreq,
							   REALTEK_R_QAM_CONFIG_OC);
	}
	return pDriver;
}



