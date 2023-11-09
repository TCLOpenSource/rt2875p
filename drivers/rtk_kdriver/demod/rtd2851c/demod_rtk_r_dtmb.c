/***************************************************************************************************
  File        : demod_realtek_dtmb.cpp
  Description : REALTEK_R DVB-T demod
  Author      : Kevin Wang
****************************************************************************************************
    Update List :
----------------------------------------------------------------------------------------------------
    20120207    | create phase
***************************************************************************************************/
#include <linux/slab.h>
#include "demod_rtk_r_dtmb.h"
#include "demod_rtk_r_dtmb_priv.h"
#include "demod_rtk_common.h"
#include "tv_osal.h"
#include "dtmb_demod_rtk_r.h"

unsigned char m_AcquireWaitSignalLock;
extern unsigned long *pgShareMemory;

/*----------------------------------------------------------------------
 * Func : REALTEK_R_DTMB
 *
 * Desc : constructor
 *
 * Parm : device_addr : device address
 *        output_mode : RTK_DEMOD_OUT_IF_SERIAL / RTK_DEMOD_OUT_IF_PARALLEL
 *        output_freq : RTK_DEMOD_OUT_FREQ_6M / RTK_DEMOD_OUT_FREQ_10M
 *        pComm       : hadle of COMM
 *
 * Retn : handle of demod
 *----------------------------------------------------------------------*/
void REALTEK_R_DTMB_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM*	pComm)
{
	REALTEK_R_DTMB* pRealtekRDTMB;

	if ((pDemodKernel == NULL) || (pComm == NULL)) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return;
	}

	pDemodKernel->private_data = kmalloc(sizeof(REALTEK_R_DTMB), GFP_KERNEL);
	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);

	Demod_Constructors(pDemodKernel);

	pDemodKernel->m_id                 = DTV_DEMOD_REALTEK_R_DTMB;
	pDemodKernel->m_addr               = addr;
	pDemodKernel->m_pTuner             = NULL;
	pDemodKernel->m_pComm              = pComm;
	pDemodKernel->m_ScanRange          = SCAN_RANGE_3_6;
	pDemodKernel->m_Capability         = TV_SYS_TYPE_DTMB;
	pDemodKernel->m_update_interval_ms = 300;
	pDemodKernel->m_if.freq            = 5000000;
	pDemodKernel->m_if.inversion       = 0;
	pDemodKernel->m_if.agc_pola        = 0;
	pDemodKernel->m_clock              = CRYSTAL_FREQ_24000000HZ;
	pRealtekRDTMB->m_output_freq        = output_freq;
	pRealtekRDTMB->m_auto_tune_enable   = 0;
	pRealtekRDTMB->m_pTunerOptReg   = NULL;
	pRealtekRDTMB->m_TunerOptRegCnt   = 0;

	pRealtekRDTMB->m_private            = (void*) AllocRealtekRDtmbDriver(
			pDemodKernel->m_pComm,
			pDemodKernel->m_addr,
			pDemodKernel->m_clock);

	pDemodKernel->m_ts.mode            = (output_mode == RTK_DEMOD_OUT_IF_PARALLEL) ? PARALLEL_MODE : SERIAL_MODE;
	pDemodKernel->m_ts.data_order      = MSB_D7;
	pDemodKernel->m_ts.datapin         = MSB_FIRST;
	pDemodKernel->m_ts.err_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.sync_pol        = ACTIVE_HIGH;
	pDemodKernel->m_ts.val_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.clk_pol         = FALLING_EDGE;
	pDemodKernel->m_ts.internal_demod_input = 1;

	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "REALTEK_R_DTMB");

	pDemodKernel->Init = REALTEK_R_DTMB_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = REALTEK_R_DTMB_AcquireSignal;
	pDemodKernel->ScanSignal = REALTEK_R_DTMB_ScanSignal;
	pDemodKernel->SetIF = REALTEK_R_DTMB_SetIF;
	pDemodKernel->SetTSMode = REALTEK_R_DTMB_SetTSMode;
	pDemodKernel->SetTVSysEx = REALTEK_R_DTMB_SetTVSysEx;
	pDemodKernel->SetMode = REALTEK_R_DTMB_SetMode;
	pDemodKernel->SetTVSys = REALTEK_R_DTMB_SetTVSys;
	pDemodKernel->GetLockStatus = REALTEK_R_DTMB_GetLockStatus;
	pDemodKernel->GetSegmentNum = Demod_GetSegmentNum;
	pDemodKernel->GetSignalInfo = REALTEK_R_DTMB_GetSignalInfo;
	pDemodKernel->GetSignalQuality = REALTEK_R_DTMB_GetSignalQuality;
	pDemodKernel->ChangeT2PLP = Demod_ChangeT2PLP;
	pDemodKernel->GetT2PLPInfo = Demod_GetT2PLPInfo;
	pDemodKernel->GetT2L1PreInfo = Demod_GetT2L1PreInfo;
	pDemodKernel->GetT2L1PostInfo = Demod_GetT2L1PostInfo;
	pDemodKernel->GetDemodStatus = Demod_GetDemodStatus;
	pDemodKernel->GetCarrierOffset = REALTEK_R_DTMB_GetCarrierOffset;
	pDemodKernel->ForwardI2C = Demod_ForwardI2C;
	pDemodKernel->AutoTune = REALTEK_R_DTMB_AutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = Demod_Suspend;
	pDemodKernel->Resume = Demod_Resume;
	pDemodKernel->SetTvMode = REALTEK_R_DTMB_SetTvMode;
	pDemodKernel->InspectSignal = REALTEK_R_DTMB_InspectSignal;	  // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
//	pDemodKernel->Lock = Demod_Lock;
//	pDemodKernel->Unlock = Demod_Unlock;
	pDemodKernel->Lock = REALTEK_R_DTMB_Lock;
	pDemodKernel->Unlock = REALTEK_R_DTMB_Unlock;

	pRealtekRDTMB->Activate = REALTEK_R_DTMB_Activate;
	pRealtekRDTMB->Destory = REALTEK_R_DTMB_Destructors;

}

/*----------------------------------------------------------------------
 * Func : ~REALTEK_R_DTMB
 *
 * Desc : Destructor of REALTEK_R_DTMB
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void REALTEK_R_DTMB_Destructors(DEMOD* pDemodKernel)
{
	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;

	if (pDemodKernel == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	if (pRealtekRDTMB && pRealtekRDTMB->m_private != NULL) {
		pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;

		if (pRealtekRDTMB->m_private != NULL) {
			pDemod->DeInitialize(pDemod);
			ReleaseRealtekRDtmbDriver((REALTEK_R_DTMB_DRIVER_DATA*)pRealtekRDTMB->m_private);
		}
	}

	if (pRealtekRDTMB)
		kfree(pRealtekRDTMB);
	//pDemodKernel->Destory(pDemodKernel);
}



/*----------------------------------------------------------------------
 * Func : Init
 *
 * Desc : Initial
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_DTMB_Init(DEMOD* pDemodKernel)
{


	int Reg;
	U32BITS Val;
	int i = 0;


	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;

	if (pDemodKernel == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekRDTMB->m_private == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;


	pDemodKernel->Lock(pDemodKernel);
	pRealtekRDTMB->m_auto_tune_enable   = 0;

	DtvDemodInitialization(RTK_DEMOD_MODE_DTMB, 1);


	// check tuner information

	if (pDemodKernel->m_pTuner) {
		TUNER_PARAM Param;

		if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, TV_SYS_DTMB_8M, &Param) == TUNER_CTRL_OK) {
			switch (Param.id)  {
			case TV_TUNER_NXP_TDA18275A:
				REALTEK_R_DTMB_DBG("Tuner id %d, TV_TUNER_NXP_TDA182XX\n", Param.id);
				pRealtekRDTMB->m_pTunerOptReg = (REG_VALUE_ENTRY*) TDA18273_SIRIUS_OPT_VALUE_DTMB;
				pRealtekRDTMB->m_TunerOptRegCnt  = sizeof(TDA18273_SIRIUS_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY);
				break;

			case TV_TUNER_SILICONLABS_SI2151:
			case TV_TUNER_SILICONLABS_SI2178B:
				REALTEK_R_DTMB_DBG("Tuner id %d, TV_TUNER_SILICONLABS_SI21XX\n", Param.id);
				pRealtekRDTMB->m_pTunerOptReg = (REG_VALUE_ENTRY*) SI2158_OPT_VALUE_DTMB;
				pRealtekRDTMB->m_TunerOptRegCnt  = sizeof(SI2158_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY);
				break;

			case TV_TUNER_RAFAEL_R840:
				REALTEK_R_DTMB_DBG("Tuner id %d, TV_TUNER_RAFAEL_R840\n", Param.id);
				pRealtekRDTMB->m_pTunerOptReg = (REG_VALUE_ENTRY*) R830_OPT_VALUE_DTMB;
				pRealtekRDTMB->m_TunerOptRegCnt  = sizeof(R830_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY);
				break;

			case TV_TUNER_MAXLINEAR_MXL661:
				REALTEK_R_DTMB_DBG("Tuner id %d, TV_TUNER_MAXLINEAR_MXL6X1\n", Param.id);
				pRealtekRDTMB->m_pTunerOptReg = (REG_VALUE_ENTRY*) MXL601_OPT_VALUE_DTMB;
				pRealtekRDTMB->m_TunerOptRegCnt  = sizeof(MXL601_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY);
				break;

			case TV_TUNER_NO_TUNER:
				REALTEK_R_DTMB_DBG("Tuner id %d, TV_TUNER_NO_TUNER\n", Param.id);
				pRealtekRDTMB->m_pTunerOptReg = (REG_VALUE_ENTRY*) SI2158_OPT_VALUE_DTMB;
				pRealtekRDTMB->m_TunerOptRegCnt  = sizeof(SI2158_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY);
				break;

			default:
				REALTEK_R_DTMB_DBG("Unkonw tuner id %d, using default setting\n", Param.id);
				break;
			}

			pDemodKernel->m_if.freq      = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola  = Param.if_agc_pola;
		}
	}





	if (pDemod->Initialize(pDemod) != TUNER_CTRL_OK) {
		pDemodKernel->Unlock(pDemodKernel);
		goto init_fail;
	}

	// Run Optimize Configuration for specified tuner
	for (i = 0; i < pRealtekRDTMB->m_TunerOptRegCnt; i++) {
		Reg = pRealtekRDTMB->m_pTunerOptReg[i].RegBitName;
		Val = pRealtekRDTMB->m_pTunerOptReg[i].Value;
		if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, Reg, Val) != FUNCTION_SUCCESS) {
			pDemodKernel->Unlock(pDemodKernel);
			goto init_fail;
		}
	}

	pDemodKernel->Unlock(pDemodKernel);

	if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
		goto init_fail;

	if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK)
		goto init_fail;

	REALTEK_R_DTMB_DBG("REALTEK_R_DTMB Init Complete\n");

	return TUNER_CTRL_OK;

init_fail:

	DEMOD_WARNING("\033[1;32;32m" "REALTEK_R_DTMB Init Failed\n" "\033[m");
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
int REALTEK_R_DTMB_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	REALTEK_R_DTMB* pRealtekRDTMB;

	if (pDemodKernel == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);

	return pDemodKernel->SetTVSysEx(pDemodKernel, sys, NULL);

}



/*----------------------------------------------------------------------
 * Func : SetTVSys
 *
 * Desc : Set TV System
 *
 * Parm : sys : TV System
 *        pSigInfo : signal information
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_DTMB_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO*  pSigInfo)
{

	int ret;
	TUNER_PARAM Param;
	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;

	if (pDemodKernel == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;
	//FUNCTION_NAME_PRINT("REALTEK_R_DTMB::SetTVSysEx\n");

	if (!IsDtmbSys(sys)) {
		REALTEK_R_DTMB_WARNING("Set TV system failed, unsupported TV system\n");
		return TUNER_CTRL_FAIL;
	}


	pRealtekRDTMB->m_auto_tune_enable = 0;

	pDemodKernel->Lock(pDemodKernel);
	switch (sys) {
	case TV_SYS_DTMB_6M:
		ret  = DECODE_RET(pDemod->SetBandwidthMode(pDemod, DTMB_BANDWIDTH_6MHZ));
		break;

	case TV_SYS_DTMB:
	case TV_SYS_DTMB_8M:
		ret  = DECODE_RET(pDemod->SetBandwidthMode(pDemod, DTMB_BANDWIDTH_8MHZ));
		break;

	default:
		ret = TUNER_CTRL_FAIL;
	}
	pDemodKernel->m_tv_sys = sys;
	pDemodKernel->Unlock(pDemodKernel);


	if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, pDemodKernel->m_tv_sys, &Param) == TUNER_CTRL_OK) {
		if ((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola  != Param.if_agc_pola)) {
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;
			if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)  return  TUNER_CTRL_FAIL;
		}
	}

	pDemodKernel->Lock(pDemodKernel);
	pDemod->ResetFunction(pDemod);      // Reset demod particular registers.
	pDemod->SoftwareReset(pDemod);      // Reset demod
	pDemodKernel->Unlock(pDemodKernel);

	return  TUNER_CTRL_OK;
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
int REALTEK_R_DTMB_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;


	if ((pDemodKernel == NULL) || (pParam == NULL)) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;
	//REALTEK_R_DTMB_INFO("\033[1;32;32m""Demod REALTEK_R_DTMB_SetIF\n" "\033[m");

	REALTEK_R_DTMB_INFO("SetIF, IF_freq="PT_U32BITS", spectrumn inv=%d, agc_pola=%d\n", pParam->freq, pParam->inversion, pParam->agc_pola);

	pDemodKernel->Lock(pDemodKernel);

	ret  = DECODE_RET(pDemod->SetIfFreqHz(pDemod, pParam->freq));
	ret |= DECODE_RET(pDemod->SetSpectrumMode(pDemod, pParam->inversion));
	//ret |= DECODE_RET(pDemod->SetRegBitsWithPage(pDemod,DTMB_POLAR_IF_AGC, pParam->agc_pola));

	//ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));

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
int REALTEK_R_DTMB_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	REALTEK_R_DTMB* pRealtekRDTMB;

	if (pDemodKernel == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);

	//REALTEK_R_DTMB_INFO("\033[1;32;32m""Demod REALTEK_R_DTMB_SetMode\n" "\033[m");

	switch (mode) {
	case TV_OP_MODE_NORMAL:
	case TV_OP_MODE_SCANING:
		break;

	case TV_OP_MODE_STANDBY:
		break;

	case TV_OP_MODE_ATV_NORMAL:
	case TV_OP_MODE_TP_ONLY:
		break;
	default:
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
int REALTEK_R_DTMB_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;


	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;

	if ((pDemodKernel == NULL) || (pParam == NULL)) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;
	//REALTEK_R_DTMB_INFO("\033[1;32;32m""Demod REALTEK_R_DTMB_SetTSMode\n" "\033[m");

	pDemodKernel->Lock(pDemodKernel);


	switch (pParam->mode) {
	case PARALLEL_MODE:
		DEMOD_INFO("TS Mode = PARALLEL\n");
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DTMB_SERIAL,   0));
		ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DTMB_CDIV_PH0, 9));
		ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DTMB_CDIV_PH1, 9));
		break;

	case SERIAL_MODE:
		DEMOD_INFO("TS Mode = SERIAL\n");
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DTMB_SERIAL,   1));
		ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DTMB_CDIV_PH0, 1));
		ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DTMB_CDIV_PH1, 1));
		break;

	default:
		ret = TUNER_CTRL_FAIL;
	}

	switch (pParam->clk_pol) {
	case RISING_EDGE:
		DEMOD_INFO("MPEG CLK POL = RISING_EDGE\n");
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DTMB_CKOUTPAR, 1));
		break;

	case FALLING_EDGE:
		DEMOD_INFO("MPEG CLK POL = FALLING_EDGE\n");
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DTMB_CKOUTPAR, 0));
		break;

	default:
		ret = TUNER_CTRL_FAIL;
	}

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
int REALTEK_R_DTMB_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{
	int ret = TUNER_CTRL_OK;
	int Val;
	S32BITS SnrNum = 0, SnrDen = 0, DemodSnr = 0;

	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;

	if ((pDemodKernel == NULL) || (pLock == NULL)) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*)pRealtekRDTMB->m_private)->pDemod;
	//REALTEK_R_DTMB_INFO("\033[1;32;32m""Demod REALTEK_R_DTMB_GetLockStatus\n" "\033[m");

	pDemodKernel->Lock(pDemodKernel);

	ret = DECODE_RET(pDemod->IsSignalLocked(pDemod, &Val));

	if (Val == 0) {
		pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen);
		DemodSnr = (SnrDen!=0) ? (SnrNum / SnrDen) : 0;
		
		if (DemodSnr > 25) {
			pDemod->IsTpsLocked(pDemod, &Val);
			if (Val) {
				if (m_AcquireWaitSignalLock == 0)
					tv_osal_msleep(200);
			}
		}
	}

	if (ret == TUNER_CTRL_OK)
		*pLock = DECODE_LOCK(Val);

	pDemodKernel->Unlock(pDemodKernel);

	REALTEK_R_DTMB_INFO("DTMB_LockStatus =%d\n", *pLock);
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
int REALTEK_R_DTMB_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;
	int ret = TUNER_CTRL_OK;
	int CarrierMode = 0, PnMode = 0, QamMode = 0, CodeRateMode = 0, TimeInterleaverMode = 0;

	if ((pDemodKernel == NULL) || (pInfo == NULL)) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*)pRealtekRDTMB->m_private)->pDemod;


	pDemodKernel->Lock(pDemodKernel);
	ret = DECODE_RET(pDemod->GetCarrierMode(pDemod, &CarrierMode));
	ret |= DECODE_RET(pDemod->GetPnMode(pDemod, &PnMode));
	ret |= DECODE_RET(pDemod->GetQamMode(pDemod, &QamMode));
	ret |= DECODE_RET(pDemod->GetCodeRateMode(pDemod, &CodeRateMode));
	ret |= DECODE_RET(pDemod->GetTimeInterleaverMode(pDemod, &TimeInterleaverMode));
	pDemodKernel->Unlock(pDemodKernel);

	pInfo->dtmb_ofdm.mod  = TV_MODULATION_DTMB_OFDM;

	switch (TimeInterleaverMode) {
	case DTMB_TIME_INTERLEAVER_240:
		pInfo->dtmb_ofdm.tim  = DTMB_OFDM_TIM_240;
		break;
	case DTMB_TIME_INTERLEAVER_720:
		pInfo->dtmb_ofdm.tim  = DTMB_OFDM_TIM_720;
		break;
	default:
	case DTMB_TIME_INTERLEAVER_UNKNOWN:
		pInfo->dtmb_ofdm.tim  = DTMB_OFDM_TIM_INVALID;
		break;
	}

	switch (CarrierMode) {
	case DTMB_CARRIER_SINGLE:
		pInfo->dtmb_ofdm.carrier_mode  = DTMB_OFDM_CARRIER_SINGLE;
		break;
	case DTMB_CARRIER_MULTI:
		pInfo->dtmb_ofdm.carrier_mode  = DTMB_OFDM_CARRIER_MULTI;
		break;
	default:
		pInfo->dtmb_ofdm.carrier_mode  = DTMB_OFDM_CARRIER_INVALID;
		break;
	}

	switch (PnMode) {
	case DTMB_PN_420:
		pInfo->dtmb_ofdm.pn.mode  = DTMB_OFDM_PN_420;
		break;
	case DTMB_PN_595:
		pInfo->dtmb_ofdm.pn.mode  = DTMB_OFDM_PN_595;
		break;
	case DTMB_PN_945:
		pInfo->dtmb_ofdm.pn.mode  = DTMB_OFDM_PN_945;
		break;
	default:
		pInfo->dtmb_ofdm.pn.mode  = DTMB_OFDM_PN_INVALID;
		break;
	}

	switch (CodeRateMode) {
	case DTMB_CODE_RATE_0P4:
		pInfo->dtmb_ofdm.fec.rate  = DTMB_OFDM_FEC_4_OVER_10;
		break;
	case DTMB_CODE_RATE_0P6:
		pInfo->dtmb_ofdm.fec.rate  = DTMB_OFDM_FEC_6_OVER_10;
		break;
	case DTMB_CODE_RATE_0P8:
		pInfo->dtmb_ofdm.fec.rate  = DTMB_OFDM_FEC_8_OVER_10;
		break;
	default:
		pInfo->dtmb_ofdm.fec.rate  = DTMB_OFDM_FEC_INVALID;
		break;
	}

	switch (QamMode) {
	case DTMB_QAM_4QAM_NR:
		pInfo->dtmb_ofdm.fec.constellation  = DTMB_OFDM_QAM_4_NR;
		break;
	case DTMB_QAM_4QAM:
		pInfo->dtmb_ofdm.fec.constellation  = DTMB_OFDM_QAM_4;
		break;
	case DTMB_QAM_16QAM:
		pInfo->dtmb_ofdm.fec.constellation  = DTMB_OFDM_QAM_16;
		break;
	case DTMB_QAM_32QAM:
		pInfo->dtmb_ofdm.fec.constellation  = DTMB_OFDM_QAM_32;
		break;
	case DTMB_QAM_64QAM:
		pInfo->dtmb_ofdm.fec.constellation  = DTMB_OFDM_QAM_64;
		break;
	default:
	case DTMB_QAM_UNKNOWN:
		pInfo->dtmb_ofdm.fec.constellation  = DTMB_OFDM_QAM_INVALID;
		break;
	}

	REALTEK_R_DTMB_INFO("GetSignalInfo TimeInterleaverMode=%d, CarrierMode=%d, PnMode=%d, CodeRateMode=%d, QamMode=%d\n", pInfo->dtmb_ofdm.tim, pInfo->dtmb_ofdm.carrier_mode, pInfo->dtmb_ofdm.pn.mode, pInfo->dtmb_ofdm.fec.rate, pInfo->dtmb_ofdm.fec.constellation);
	return ret;
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
int REALTEK_R_DTMB_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id,	TV_SIG_QUAL* pQuality)
{
	int ret = TUNER_CTRL_OK;
	int LockStatus = 0;
	S32BITS SnrNum, SnrDen;
	U32BITS BerNum, BerDen;
	U32BITS PerNum, PerDen;
	U32BITS IterNum;
	U32BITS SignalStrength;


	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;


	if ((pDemodKernel == NULL) || (pQuality == NULL)) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}


	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;

	//REALTEK_R_DTMB_INFO("\033[1;32;32m""Demod REALTEK_R_DTMB_GetSignalQuality\n" "\033[m");

	pDemodKernel->Lock(pDemodKernel);

	switch (id) {
	case TV_QUAL_NOSIG :
		ret = DECODE_RET(pDemod->IsNoSig(pDemod, &LockStatus));
		pQuality->nosig = (LockStatus) ? 1 : 0;
		break;

	case TV_QUAL_INNERLOCK :
		ret = DECODE_RET(pDemod->IsInnerLocked(pDemod, &LockStatus));
		pQuality->inner = (LockStatus) ? 1 : 0;
		break;

	case TV_QUAL_SNR :

		ret = DECODE_RET(pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen));

		if (ret == TUNER_CTRL_OK)
			pQuality->snr = (SnrNum * 10000) / SnrDen;

		REALTEK_R_DTMB_DBG("[GetSignalQuality] ret = %d, SNR= "PT_U32BITS"\n", ret, pQuality->snr);

		break;

	case TV_QUAL_BER :
	case TV_QUAL_BEC :

		ret = DECODE_RET(pDemod->GetBer(pDemod, &BerNum, &BerDen));

		if (id == TV_QUAL_BER)
			pQuality->ber = (BerNum * 10000) / BerDen;
		else {
			pQuality->layer_ber.ber_num = BerNum;
			pQuality->layer_ber.ber_den = BerDen;
		}

		REALTEK_R_DTMB_DBG("[GetSignalQuality] ret = %d, id = %d, ber= "PT_U32BITS" ( "PT_U32BITS" / "PT_U32BITS") \n", ret, id, pQuality->ber, pQuality->layer_ber.ber_num, pQuality->layer_ber.ber_den);
		break;

	case TV_QUAL_PER :
	case TV_QUAL_PEC :

		ret = DECODE_RET(pDemod->GetPer(pDemod, &PerNum, &PerDen));

		if (id == TV_QUAL_PER)
			pQuality->per = (PerNum * 10000) / PerDen;
		else {
			pQuality->layer_per.per_num = PerNum;
			pQuality->layer_per.per_den = PerDen;
		}

		REALTEK_R_DTMB_DBG("[GetSignalQuality] ret = %d, id = %d, per= "PT_U32BITS" ( "PT_U32BITS" / "PT_U32BITS") \n", ret, id, pQuality->per, pQuality->layer_per.per_num, pQuality->layer_per.per_den);
		break;

	case TV_QUAL_ITER :

		ret = DECODE_RET(pDemod->GetIter(pDemod, &IterNum));

		if (ret == TUNER_CTRL_OK)
			pQuality->iter = (char) IterNum;

		break;

	case TV_QUAL_UCBLOCKS :

		pQuality->ucblocks = 0;
		ret = TUNER_CTRL_OK;
		break;

	case TV_QUAL_SIGNAL_STRENGTH:

		ret = DECODE_RET(pDemod->GetSignalStrength(pDemod, &SignalStrength));

		if (ret == TUNER_CTRL_OK)
			pQuality->strength = (unsigned char) SignalStrength;

		REALTEK_R_DTMB_DBG("[GetSignalQuality] ret = %d, SSI= %u\n", ret, pQuality->strength);

		break;

	case TV_QUAL_SIGNAL_QUALITY:

		ret = DECODE_RET(pDemod->GetBer(pDemod, &BerNum, &BerDen));
		pQuality->ber = (BerNum * 10000) / BerDen;

		if (ret == TUNER_CTRL_OK)
			pQuality->quality = Ber2Quality(pQuality->ber);  // convert ber to signal quality

		REALTEK_R_DTMB_DBG("[GetSignalQuality] ret = %d, SQI= %u\n", ret, pQuality->quality);
		break;

	case TV_QUAL_DEMODFWVERSION:
		pQuality->DemodFwVersion = 0x000001;
		break;


	case TV_QUAL_AGC: {
		S32BITS IfAgc = -1;
		ret = pDemod->GetIfAgc(pDemod, &IfAgc);

		if (ret == TUNER_CTRL_OK)
			pQuality->agc = ((IfAgc + 8191) * 100) / 16384;
	}
	break;

	default:
		ret = TUNER_CTRL_FAIL;
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
int REALTEK_R_DTMB_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;



	if ((pDemodKernel == NULL) || (pOffset == NULL)) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;
	//REALTEK_R_DTMB_INFO("\033[1;32;32m""Demod REALTEK_R_DTMB_GetCarrierOffset\n" "\033[m");

	pDemodKernel->Lock(pDemodKernel);
	ret = DECODE_RET(pDemod->GetCrOffsetHz(pDemod, pOffset));

	if (ret != TUNER_CTRL_OK)
		*pOffset = 0;
	REALTEK_R_DTMB_DBG("RealtekH DTMB offset = "PT_S32BITS" \n", *pOffset);
	pDemodKernel->Unlock(pDemodKernel);

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
int REALTEK_R_DTMB_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{
	U32BITS   stime;
	int lock, no_signal;
	S32BITS SnrNum = 0, SnrDen = 0, DemodSnr = 0;

	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;

	if (pDemodKernel == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}


	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;

	REALTEK_R_DTMB_DBG("\033[1;32;32m""Demod REALTEK_R_DTMB_AcquireSignal\n" "\033[m");

	pRealtekRDTMB->m_auto_tune_enable = 0;
	m_AcquireWaitSignalLock = WaitSignalLock;

	if (!WaitSignalLock) {
		REALTEK_R_DTMB_INFO("WaitSignalLock = %u \n", WaitSignalLock);
		return TUNER_CTRL_OK;
	}

	stime = tv_osal_time();
	no_signal = YES;
	lock = NO;

	pDemodKernel->Lock(pDemodKernel);

	while ((tv_osal_time() - stime) <= 250) {
		pDemod->IsNoSig(pDemod, &no_signal);
		tv_osal_msleep(5);
		if (no_signal == 0) break;
	}

	REALTEK_R_DTMB_INFO("no_signal = %d, time=%lu\n", no_signal, tv_osal_time() - stime);

	if (no_signal == 1) return TUNER_CTRL_OK;

	while ((tv_osal_time() - stime) <= 3000) {  //600
		tv_osal_msleep(5);

		pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen);
		//REALTEK_R_DTMB_INFO("*****SnrNum = %d, SnrDen = %d\n",SnrNum, SnrDen);
		DemodSnr = (SnrDen!=0) ? (SnrNum / SnrDen) : 0;

		if (DemodSnr > 25) {
			pDemod->IsTpsLocked(pDemod, &lock);
			//REALTEK_R_DTMB_INFO("*****TPS Lock Result = %d\n",lock);

			if (lock) {
				tv_osal_msleep(200);
				REALTEK_R_DTMB_DBG("SNR > TH && DemodTpsLock time = %lu\n", (tv_osal_time() - stime));
				break;
			}
		} else {
			//pDemod->IsInnerLocked(pDemod, &lock);
			pDemod->IsSignalLocked(pDemod, &lock);
			//REALTEK_R_DTMB_INFO("*****IsSignalLocked = %d\n",lock);
			if (lock)  {
				REALTEK_R_DTMB_DBG("SNR < TH && DemodPerLock time = %lu\n", (tv_osal_time() - stime));
				break;
			}
		}
	}

	//REALTEK_R_DTMB_INFO("signal inner lock = %d, time=%ld\n", lock, tv_osal_time() - stime);
	REALTEK_R_DTMB_INFO("signal lock = %d, time=%ld\n", lock, tv_osal_time() - stime);
	pDemodKernel->Unlock(pDemodKernel);

	pRealtekRDTMB->m_auto_tune_enable = 1;
	return TUNER_CTRL_OK;
	//return (lock) ? TUNER_CTRL_OK : TUNER_CTRL_NO_SIGNAL;

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
int REALTEK_R_DTMB_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range)
{

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
int REALTEK_R_DTMB_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off)
{

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
int REALTEK_R_DTMB_AutoTune(DEMOD* pDemodKernel)
{


	REALTEK_R_DTMB* pRealtekRDTMB;
	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	//DTMB_DEMOD_MODULE* pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;
	//REALTEK_R_DTMB_INFO("\033[1;32;32m""Demod REALTEK_R_DTMB_AutoTune\n" "\033[m");

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
int REALTEK_R_DTMB_InspectSignal(DEMOD* pDemodKernel)
{
	REALTEK_R_DTMB* pRealtekRDTMB;
	DTMB_DEMOD_MODULE* pDemod;
	S32BITS IfAgc = -1;
	S32BITS RfAgc = -1;
	S32BITS TrOffsetPpm = -1;
	S32BITS CrOffsetHz  = -1;
	U32BITS DiAgc = 0;
	int SignalLock = 0;
	U32BITS BerNum = 1;
	U32BITS BerDen = 1;
	U32BITS PerNum = 1;
	U32BITS PerDen = 1;
	S32BITS SnrDbNum = 1;
	S32BITS SnrDbDen = 1;

	if (pDemodKernel == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);
	if (pRealtekRDTMB == NULL || pDemodKernel->private_data == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pDemod = ((REALTEK_R_DTMB_DRIVER_DATA*) pRealtekRDTMB->m_private)->pDemod;
	if (pDemod == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}

	pDemodKernel->Lock(pDemodKernel);
	pDemod->GetRfAgc(pDemod, &RfAgc);
	pDemod->GetIfAgc(pDemod, &IfAgc);
	pDemod->GetDiAgc(pDemod, &DiAgc);

	pDemod->GetTrOffsetPpm(pDemod, &TrOffsetPpm);
	pDemod->GetCrOffsetHz(pDemod, &CrOffsetHz);
	pDemod->IsSignalLocked(pDemod, &SignalLock);
	pDemod->GetBer(pDemod, &BerNum, &BerDen);
	pDemod->GetPer(pDemod, &PerNum, &PerDen);

	pDemod->GetSnrDb(pDemod, &SnrDbNum, &SnrDbDen);
	pDemodKernel->Unlock(pDemodKernel);


	IfAgc = ((IfAgc + 8191) * 100) / 16384;
	RfAgc = ((RfAgc + 8191) * 100) / 16384;
	DiAgc = (DiAgc * 100) / 8192;

	REALTEK_R_DTMB_INFO("lock(signal)=%d, agc(rf/if/di)="PT_S32BITS"/"PT_S32BITS"/"PT_U32BITS", ofst(tr/cr)="PT_S32BITS"/"PT_S32BITS",  error_rate(ber/per)="PT_U32BITS"/"PT_U32BITS", snr="PT_S32BITS"\n",
						SignalLock,
						RfAgc, IfAgc, DiAgc,
						TrOffsetPpm, CrOffsetHz,
						(BerNum * 10000) / BerDen,
						(PerNum * 10000) / PerDen,
						(SnrDbNum * 10000) / SnrDbDen
					   );

	return TUNER_CTRL_OK;

}



/*----------------------------------------------------------------------
 * Func : Activate
 *
 * Desc : Activate REALTEK_R_ QAM Demod
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_DTMB_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{

	return TUNER_CTRL_OK;
}

/*----------------------------------------------------------------------
 * Func : SetTvMode
 *
 * Desc : Set Mode of REALTEK_R_ALL
 *
 * Parm : mode :    RTK_DEMOD_MODE_DTMB : DTMB mode
 *                  RTK_DEMOD_MODE_DVBC: DVBC mode
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_DTMB_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;
	REALTEK_R_DTMB* pRealtekRDTMB;

	if (pDemodKernel == NULL) {
		REALTEK_R_DTMB_WARNING("Please check for NULL pointer !!! file: %s, func: %s, line %d\n", __FILE__, __func__, __LINE__);
		return TUNER_CTRL_FAIL;
	}


	pRealtekRDTMB = (REALTEK_R_DTMB*)(pDemodKernel->private_data);

	//REALTEK_R_DTMB_INFO("\033[1;32;32m""Demod REALTEK_R_DTMB_SetTvMode\n" "\033[m");


	REALTEK_R_DTMB_DBG("REALTEK_R_DTMB::SetTvMode(%d)\n", mode);

	switch (mode) {

	case TV_SYS_TYPE_DTMB:

		REALTEK_R_DTMB_DBG("Set to DTMB\n");
		ret = pDemodKernel->Init(pDemodKernel);

		if (ret != TUNER_CTRL_OK) {
			DEMOD_WARNING("Set REALTEK_R_DTMB to DTMB mode failed\n");
			goto set_demod_mode_failed;
		}

		break;

	default:
		goto set_demod_mode_failed;
	}

	ret = TUNER_CTRL_OK;
	REALTEK_R_DTMB_DBG("Set REALTEK_R_DTMB to appointed DTV mode successed\n");


//end_proc:

	return ret;

//--------------
set_demod_mode_failed:

	return TUNER_CTRL_FAIL;
}

void REALTEK_R_DTMB_Lock(DEMOD* pDemodKernel)
{
	//mutex_lock(&pDemodKernel->m_lock);
}

void REALTEK_R_DTMB_Unlock(DEMOD* pDemodKernel)
{
	//mutex_unlock(&pDemodKernel->m_lock);
}



/*-----------------------------------------------------------------
 *     BASE_INTERFACE for REALTEK_R_DTMB API
 *----------------------------------------------------------------*/


void ReleaseRealtekRDtmbDriver(REALTEK_R_DTMB_DRIVER_DATA *pDriver)
{

	if (pDriver->pShareMemory) {
		DTMB_DEMOD_MODULE* pDemod = pDriver->pDemod;

		REALTEK_R_DTMB_INFO("ReleaseRtkrDriver : hold DTMB demod reset to disable share memory access\n");
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DTMB_SOFT_RST_N, 0x0);

		REALTEK_R_DTMB_INFO("ReleaseRtkrDriver : release share memory - addr=%p/%08lx, size=%08x\n",
							pDriver->pShareMemory,
							pDriver->ShareMemoryPhysicalAddress,
							RTK_R_DTMB_DDR_SIZE);

		DtvDemodFreeAllocMem(pDriver->pShareMemory);
		pDriver->pShareMemory = (unsigned long*)NULL;
	}



	if (pDriver != NULL)
		kfree(pDriver);
}


REALTEK_R_DTMB_DRIVER_DATA* AllocRealtekRDtmbDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
)
{
	unsigned int ret = TUNER_CTRL_OK;

	REALTEK_R_DTMB_DRIVER_DATA* pDriver = (REALTEK_R_DTMB_DRIVER_DATA*) kmalloc(
			sizeof(REALTEK_R_DTMB_DRIVER_DATA) +
			sizeof(DTMB_DEMOD_MODULE)   +
			sizeof(I2C_BRIDGE_MODULE)   +
			sizeof(BASE_INTERFACE_MODULE), GFP_KERNEL);

	if (pDriver) {
		BASE_INTERFACE_MODULE* pBIF;
		DTMB_DEMOD_MODULE* pDemod;
		memset(pDriver, 0, sizeof(REALTEK_R_DTMB_DRIVER_DATA));

		pDriver->pDemod                = (DTMB_DEMOD_MODULE*)(((unsigned char*)pDriver) + sizeof(REALTEK_R_DTMB_DRIVER_DATA));
		pDriver->pBaseInterface        = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(DTMB_DEMOD_MODULE));
		pDriver->pI2CBridge            = (I2C_BRIDGE_MODULE*)(((unsigned char*)pDriver->pBaseInterface) + sizeof(BASE_INTERFACE_MODULE));
		pDriver->DeviceAddr            = Addr;
		pDriver->CrystalFreqHz         = CrystalFreq;

		// Init Base IF
		pBIF = pDriver->pBaseInterface;

		pBIF->I2cReadingByteNumMax      = RTK_DEMOD_BIF_RX_FIFO_DEPTH;
		pBIF->I2cWritingByteNumMax      = RTK_DEMOD_BIF_TX_FIFO_DEPTH;
		pBIF->I2cRead                   = __realtek_rbus_i2c_read;
		pBIF->I2cWrite                  = __realtek_rbus_i2c_write;
		pBIF->WaitMs                    = __realtek_wait_ms;
		pBIF->SetUserDefinedDataPointer = base_interface_SetUserDefinedDataPointer;
		pBIF->GetUserDefinedDataPointer = base_interface_GetUserDefinedDataPointer;
		pBIF->SetUserDefinedDataPointer(pBIF, (void*)pComm);


		if (RTK_R_DTMB_DDR_SIZE) {

			unsigned int AllocateSize = 0;
			ret = DtvDemodMemRemappingInt(RTK_R_DTMB_DDR_SIZE, &pDriver->ShareMemoryPhysicalAddress, &AllocateSize);

			if (ret != TUNER_CTRL_OK) {
				REALTEK_R_DTMB_WARNING("AllocRtkrDriver_DTMB failed, allocate share memory failed\n");
				kfree(pDriver);
				return NULL;
			}
			pDriver->pShareMemory = pgShareMemory;
			REALTEK_R_DTMB_INFO("allocate share memory - addr=%p/%08lx", pDriver->pShareMemory, pDriver->ShareMemoryPhysicalAddress);
			if (pDriver->ShareMemoryPhysicalAddress & 0xfffff) {
				REALTEK_R_DTMB_WARNING("AllocRtkrDriver_ISDBT failed, share memory allocation is not 1MB align\n");
				//assert(0);
			}

		}







		// Init Demod Driver

		BuildRealtekRDtmbModule(&pDemod,
								pDriver->pDemod,
								pDriver->pBaseInterface,
								pDriver->pI2CBridge,
								Addr,
								CrystalFreq,
								pDriver->ShareMemoryPhysicalAddress); // Share memory address....
	}

	return pDriver;
}
