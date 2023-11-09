/***************************************************************************************************
  File        : demod_rtk_s_atsc3p0.cpp
  Description : REALTEK_S_ATSC3P0 demod
  Author      : Kevin Wang
****************************************************************************************************
    Update List :
----------------------------------------------------------------------------------------------------
    20120207    | create phase
***************************************************************************************************/
#include <linux/slab.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <kadp_OSAL.h>
#include "demodcore/foundation.h"
#include "demod_rtk_common.h"
#include "demod_rtk_s_isdbt.h"
#include "demod_rtk_s_isdbt_priv.h"

#include "CXD2856Family_refcode/sony_demod.h"
#include "CXD2856Family_refcode/isdb_terr/sony_isdbt.h"
#include "CXD2856Family_refcode/isdb_terr/sony_demod_isdbt.h"
#include "CXD2856Family_refcode/isdb_terr/sony_demod_isdbt_monitor.h"
#include "CXD2856Family_refcode/isdb_terr/sony_integ_isdbt.h"

static int isCreated = 0;
BASE_INTERFACE_MODULE      BaseInterface;
const char *Common_Result[13] = {"OK", "Argument Error", "I2C Error", "SW State Error", "HW State Error", "Timeout", "Unlock", "Out of Range", "No Support", "Cancelled", "Other Error", "Overflow", "OK - Confirm"};

/*----------------------------------------------------------------------
 * Func : REALTEK_S_ISDBT
 *
 * Desc : constructor
 *
 * Parm : device_addr : device address
 *        output_mode : REALTEK_S_ISDBT_OUT_IF_SERIAL / REALTEK_S_ISDBT_OUT_IF_PARALLEL
 *        output_freq : REALTEK_S_ISDBT_OUT_FREQ_6M / REALTEK_S_ISDBT_OUT_FREQ_10M
 *        pComm       : hadle of COMM
 *
 * Retn : handle of demod
 *----------------------------------------------------------------------*/
void REALTEK_S_ISDBT_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm)
{
	REALTEK_S_ISDBT* pRealtekSISDBT;

	pDemodKernel->private_data = kmalloc(sizeof(REALTEK_S_ISDBT), GFP_KERNEL);
	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	Demod_Constructors(pDemodKernel);

	pDemodKernel->m_id                 = DTV_DEMOD_SONY_ISDBT;
	pDemodKernel->m_addr               = addr;
	pDemodKernel->m_pTuner             = NULL;
	pDemodKernel->m_pComm              = pComm;
	pDemodKernel->m_ScanRange          = SCAN_RANGE_3_6;
	pDemodKernel->m_ScanMode           = CH_UNKNOW;
	pDemodKernel->m_Capability         = TV_SYS_TYPE_ISDBT;
	pDemodKernel->m_update_interval_ms = 300;
	pDemodKernel->m_if.freq            = 36125000;
	pDemodKernel->m_if.inversion       = 1;
	pDemodKernel->m_if.agc_pola        = 0;
	pDemodKernel->m_clock              = CRYSTAL_FREQ_27000000HZ;
	pRealtekSISDBT->m_output_freq        = output_freq;
	pRealtekSISDBT->m_private            = (void*) AllocRealtekSIsdbtDriver(
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
	pDemodKernel->m_ts.tsOutEn 		   = 1;
	pRealtekSISDBT->m_low_snr_detect     = 0;
	pRealtekSISDBT->m_low_snr_recover    = 0;
	pRealtekSISDBT->m_auto_tune_enable   = 0;
	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "REALTEK_S_ATSC3P0");
	pRealtekSISDBT->m_pTunerOptReg       = NULL;
	pRealtekSISDBT->m_TunerOptRegCnt     = 0;
	pRealtekSISDBT->m_acquire_sig_en = 0;
	pDemodKernel->Init = REALTEK_S_ISDBT_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = REALTEK_S_ISDBT_AcquireSignal;
	pDemodKernel->ScanSignal = REALTEK_S_ISDBT_ScanSignal;
	pDemodKernel->SetIF = REALTEK_S_ISDBT_SetIF;
	pDemodKernel->SetTSMode = REALTEK_S_ISDBT_SetTSMode;
	pDemodKernel->SetTVSysEx = REALTEK_S_ISDBT_SetTVSysEx;
	pDemodKernel->SetMode = REALTEK_S_ISDBT_SetMode;
	pDemodKernel->SetTVSys = REALTEK_S_ISDBT_SetTVSys;
	pDemodKernel->GetLockStatus = REALTEK_S_ISDBT_GetLockStatus;
	pDemodKernel->GetSegmentNum = Demod_GetSegmentNum;
	pDemodKernel->GetSignalInfo = REALTEK_S_ISDBT_GetSignalInfo;
	pDemodKernel->GetSignalQuality = REALTEK_S_ISDBT_GetSignalQuality;
	pDemodKernel->ChangeT2PLP = Demod_ChangeT2PLP;
	pDemodKernel->GetT2PLPInfo = Demod_GetT2PLPInfo;
	pDemodKernel->GetT2L1PreInfo = Demod_GetT2L1PreInfo;
	pDemodKernel->GetT2L1PostInfo = Demod_GetT2L1PostInfo;
	pDemodKernel->GetDemodStatus = Demod_GetDemodStatus;
	pDemodKernel->GetCarrierOffset = REALTEK_S_ISDBT_GetCarrierOffset;
	pDemodKernel->ForwardI2C = REALTEK_S_ISDBT_ForwardI2C;
	pDemodKernel->AutoTune = REALTEK_S_ISDBT_AutoTune;
	pDemodKernel->KernelAutoTune = REALTEK_S_ISDBT_KernelAutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = Demod_Suspend;
	pDemodKernel->Resume = Demod_Resume;
	pDemodKernel->SetTvMode = REALTEK_S_ISDBT_SetTvMode;
	//pDemodKernel->DebugInfo = REALTEK_S_ISDBT_DebugInfo;	  	  // Add monitor function that used to monitor demod status
	pDemodKernel->InspectSignal = REALTEK_S_ISDBT_InspectSignal; // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
	pDemodKernel->Lock = REALTEK_S_ISDBT_Lock;
	pDemodKernel->Unlock = REALTEK_S_ISDBT_Unlock;
	//pDemodKernel->Activate = REALTEK_S_ATSC3P0_Activate;
	pDemodKernel->Destory = REALTEK_S_ISDBT_Destructors;
}


/*----------------------------------------------------------------------
 * Func : ~REALTEK_S_ISDBT
 *
 * Desc : Destructor of REALTEK_S_ISDBT
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void REALTEK_S_ISDBT_Destructors(DEMOD* pDemodKernel)
{
	REALTEK_S_ISDBT* pRealtekSISDBT;
	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);

	if (pRealtekSISDBT->m_private != NULL)
		ReleaseRealtekSIsdbtDriver((REALTEK_S_ISDBT_DRIVER_DATA*)pRealtekSISDBT->m_private);

	kfree(pRealtekSISDBT);
	//pDemodKernel->Destory(pDemodKernel);
	REALTEK_S_ISDBT_INFO("REALTEK_S_ISDBT_Destructors Complete\n");
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
int REALTEK_S_ISDBT_Init(DEMOD* pDemodKernel)
{
	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;
	sony_demod_create_param_t createParam;
	static sony_i2c_t DemodI2c;
	sony_result_t result = SONY_RESULT_OK;

	unsigned char dem_slv[5] = {0x00, 0xd8, 0xda, 0xc8, 0xca};

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	createParam.xtalFreq       = SONY_DEMOD_XTAL_24000KHz;/* 24MHz Xtal */
	createParam.tunerI2cConfig = SONY_DEMOD_TUNER_I2C_CONFIG_REPEATER;/* I2C repeater is used */

	pDemodKernel->Lock(pDemodKernel);
	pRealtekSISDBT->m_auto_tune_enable = 0;

	if (!isCreated) {
		DemodI2c.Read             = sony_i2c_dummyRead;
		DemodI2c.Write            = sony_i2c_dummyWrite;
		DemodI2c.ReadRegister     = sony_i2c_CommonReadRegister;
		DemodI2c.WriteRegister    = sony_i2c_CommonWriteRegister;
		DemodI2c.WriteOneRegister = sony_i2c_CommonWriteOneRegister;

		createParam.i2cAddressSLVT =  dem_slv[1];/* I2C slave address is 0xD8 */
		result = sony_demod_Create(pDemod, &createParam, &DemodI2c);
		if (result != SONY_RESULT_OK) {
			//printf("sony_demod_Create error (%d)\n", result);
			return TUNER_CTRL_FAIL;
		}

		isCreated = 1;
	}


	result = sony_demod_Initialize(pDemod);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Initialize error (%d)\n", result);
		goto init_fail;
	}

/*
	result = sony_demod_I2cRepeaterEnable(pDemod, 1);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_I2cRepeaterEnable error (%d)\n", result);
		goto init_fail;
	}

	result = sony_demod_Shutdown(pDemod);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Shutdown error (%d)\n", result);
		goto init_fail;
	}
*/
	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_S_ISDBT_INFO("REALTEK_S_ISDBT_Init Complete\n");
	return TUNER_CTRL_OK;

init_fail:
	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_S_ISDBT_WARNING("REALTEK_S_ISDBT_Init Fail\n");
	//pDemodKernel->Unlock(pDemodKernel);

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
int REALTEK_S_ISDBT_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	REALTEK_S_ISDBT* pRealtekSISDBT;
	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);

	REALTEK_S_ISDBT_INFO("\033[1;32;31m" "%s %s %d, system = %d\n" "\033[m", __FILE__, __func__, __LINE__, sys);
	return pDemodKernel->SetTVSysEx(pDemodKernel, sys, NULL);
}


/*----------------------------------------------------------------------
 * Func : SetTVSysEx
 *
 * Desc : Set TV System
 *
 * Parm : sys : TV System
 *        pSigInfo : signal information
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_S_ISDBT_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO*  pSigInfo)
{
	TUNER_PARAM Param;
	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;


	sony_result_t result = SONY_RESULT_OK;
	sony_isdbt_tune_param_t ISDBTparam;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	//sys = TV_SYS_ISDBT_6M;
	if (!IsIsdbtSys(sys)) {
		REALTEK_S_ISDBT_WARNING("Set TV system failed, unsupported TV system\n");
		return  TUNER_CTRL_FAIL;
	}

	pRealtekSISDBT->m_auto_tune_enable = 0;

	switch (sys) {
	case TV_SYS_ISDBT_6M :
		ISDBTparam.bandwidth = SONY_DTV_BW_6_MHZ;
		break;
	case TV_SYS_ISDBT_7M :
		ISDBTparam.bandwidth = SONY_DTV_BW_7_MHZ;
		break;
	case TV_SYS_ISDBT_8M :
		ISDBTparam.bandwidth = SONY_DTV_BW_8_MHZ;
		break;
	default :
		ISDBTparam.bandwidth = SONY_DTV_BW_6_MHZ;
		break;
	}

	REALTEK_S_ISDBT_INFO("\033[1;32;31m" "%s %s %d, system = %d\n" "\033[m", __FILE__, __func__, __LINE__, sys);

	//TUNER_PARAM Param;
	if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, sys, &Param) == TUNER_CTRL_OK) {
		if ((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola  != Param.if_agc_pola)) {
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;
			if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)  return  TUNER_CTRL_FAIL;

		}
	}
	REALTEK_S_ISDBT_INFO("m_if.freq = "PT_S32BITS", m_if.inversion = %d, m_if.agc_pola = %d\n", pDemodKernel->m_if.freq, pDemodKernel->m_if.inversion, pDemodKernel->m_if.agc_pola);
	
	pDemodKernel->Lock(pDemodKernel);

	result = sony_demod_Initialize(pDemod);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Shutdown error (%d)\n", result);
		goto SetDemod_fail;
	}


	result = sony_demod_I2cRepeaterEnable(pDemod, 1);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Shutdown error (%d)\n", result);
		goto SetDemod_fail;
	}

	/*
	* In default, The setting is optimized for Sony silicon tuners.
	* If non-Sony tuner is used, the user should call following to
	* disable Sony silicon tuner optimized setting.
	*/
	result = sony_demod_SetConfig (pDemod, SONY_DEMOD_CONFIG_TUNER_OPTIMIZE, SONY_DEMOD_TUNER_OPTIMIZE_NONSONY);
	if (result != SONY_RESULT_OK) {
		REALTEK_S_ISDBT_INFO ("sony_demod_SetConfig (SONY_DEMOD_CONFIG_TUNER_OPTIMIZE) failed. (%s)\n", Common_Result[result]);
		return result;
	}
/*
	result = sony_demod_SetTSClockModeAndFreq(pDemod, SONY_DTV_SYSTEM_ISDBT);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Shutdown error (%d)\n", result);
		goto SetDemod_fail;
	}

	result = sony_demod_SetConfig(pDemod, SONY_DEMOD_CONFIG_OUTPUT_ATSC3, SONY_DEMOD_OUTPUT_ATSC3_ALP);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Shutdown error (%d)\n", result);
		goto SetDemod_fail;
	}

	result = sony_demod_SetConfig(pDemod, SONY_DEMOD_CONFIG_PARALLEL_SEL, 0);


	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Shutdown error (%d)\n", result);
		goto SetDemod_fail;
	}

	result = sony_demod_SetConfig(pDemod, SONY_DEMOD_CONFIG_ALP_SER_DATA_ON_MSB, 0);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Shutdown error (%d)\n", result);
		goto SetDemod_fail;
	}
*/
	result = sony_demod_isdbt_Tune(pDemod, &ISDBTparam);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Shutdown error (%d)\n", result);
		goto SetDemod_fail;
	}

	result = sony_demod_TuneEnd(pDemod);
	if (result != SONY_RESULT_OK) {
		//printf("sony_demod_Shutdown error (%d)\n", result);
		goto SetDemod_fail;
	}


	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_S_ISDBT_WARNING("REALTEK_S_ISDBT_SetTVSysEx done\n");
	pRealtekSISDBT->m_acquire_sig_en = 1;
	pRealtekSISDBT->m_status_checking_stime = tv_osal_time() + ISDBT_STATUS_CHECK_INTERVAL_MS;
	pRealtekSISDBT->m_auto_tune_enable = 1;

	return TUNER_CTRL_OK;

SetDemod_fail:
	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_S_ISDBT_WARNING("REALTEK_S_ISDBT_SetTVSysEx Fail\n");
	//pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_FAIL;



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
int REALTEK_S_ISDBT_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;
	sony_demod_iffreq_config_t iffreqConfig;
	unsigned char if_freqMHz = 0;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);
	
	if_freqMHz = (unsigned char)(pParam->freq)/1000000;
	iffreqConfig.configISDBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(if_freqMHz);
	iffreqConfig.configISDBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(if_freqMHz);
	iffreqConfig.configISDBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG(if_freqMHz);
	ret = sony_demod_SetIFFreqConfig (pDemod, &iffreqConfig);
	if (ret != SONY_RESULT_OK) {
		REALTEK_S_ISDBT_WARNING("sony_demod_SetIFFreqConfig failed. (%s)\n", Common_Result[ret]);
		return ret;
	}
	
	/*     
	* Terrestrial side ¡V Forward AGC (Negative) is default
	* If non-Sony tuner is used, the user should do this setting depend on the IFAGC sense of the tuner.
	* Terrestrial side AGC sense setting 
	* Reverse AGC(Positive): 0
	* Forward AGC(Negative): 1
	*/   
	ret = sony_demod_SetConfig (pDemod, SONY_DEMOD_CONFIG_IFAGCNEG, pParam->agc_pola);
	if (ret != SONY_RESULT_OK) {
		REALTEK_S_ISDBT_WARNING ("sony_demod_SetConfig (SONY_DEMOD_CONFIG_IFAGCNEG) failed. (%s)\n", Common_Result[ret]);
		return ret;
	}
	/*     
	* generally the IF spectrum sense is inverted from the RF input signal
	* But some tuners output an IF without spectrum inversion
	* Note that even if this setting is not correct, the tune APIs will not be failed because
	* demodulator hardware automatically handles both inverted and not inverted spectrum.
	* However, the sign of the carrier offset monitor value will become opposite.
	* Spectrum sense setting. (IF spectrum sense is same as input signal.)=> 1
	* (IF spectrum sense is invert as input signal.)=> 0
	* However, the sign of the carrier offset monitor value will become opposite.
	*/  
	ret = sony_demod_SetConfig (pDemod, SONY_DEMOD_CONFIG_SPECTRUM_INV, pParam->inversion);
	if (ret != SONY_RESULT_OK) {
		REALTEK_S_ISDBT_WARNING ("sony_demod_SetConfig (SONY_DEMOD_CONFIG_SPECTRUM_INV) failed. (%s)\n", Common_Result[ret]);
		return ret;
	}
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
int REALTEK_S_ISDBT_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

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
		REALTEK_S_ISDBT_WARNING("unknown TV_OP_MODE\n");
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
int REALTEK_S_ISDBT_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	//need implement

	pDemodKernel->Lock(pDemodKernel);
	switch (pParam->mode) {
	case PARALLEL_MODE:
		REALTEK_S_ISDBT_DBG("TS Mode: PARALLEL\n");
		/**
		@brief Parallel or serial TS output selection.

			Value:
			- 0: Serial output.
			- 1: Parallel output (Default).
		*/
		ret |= sony_demod_SetConfig (pDemod, SONY_DEMOD_CONFIG_PARALLEL_SEL, 1);
		break;
	case SERIAL_MODE:
		REALTEK_S_ISDBT_DBG("TS Mode: SERIAL\n");
		ret |= sony_demod_SetConfig (pDemod, SONY_DEMOD_CONFIG_PARALLEL_SEL, 0);
		break;
	default:
		REALTEK_S_ISDBT_WARNING("unknown TS Mode\n");
		ret = TUNER_CTRL_FAIL;
		break;
	}

	switch (pParam->clk_pol) {
	case RISING_EDGE:
		REALTEK_S_ISDBT_DBG("MPEG CLK POL: RISING_EDGE\n");
		ret |= sony_demod_SetConfig (pDemod, SONY_DEMOD_CONFIG_LATCH_ON_POSEDGE, 1);
		break;
	case FALLING_EDGE:
		REALTEK_S_ISDBT_DBG("MPEG CLK POL: FALLING_EDGE\n");
		ret |= sony_demod_SetConfig (pDemod, SONY_DEMOD_CONFIG_LATCH_ON_POSEDGE, 0);
		break;
	default:
		REALTEK_S_ISDBT_WARNING("unknown MPEG CLK POL\n");
		ret = TUNER_CTRL_FAIL;
		break;
	}

	//if (pParam->tsOutEn == 0)
	//DtvRDemodTPFix(1);
	//else
	//DtvRDemodTPFix(0);

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
int REALTEK_S_ISDBT_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{

	sony_result_t result = SONY_RESULT_OK;
	unsigned char DemodLock = 0, TsLock = 0, EarlyUnlock = 0;

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;


	pDemodKernel->Lock(pDemodKernel);
	result = sony_demod_isdbt_monitor_SyncStat(pDemod, &DemodLock, &TsLock, &EarlyUnlock);
	pDemodKernel->Unlock(pDemodKernel);

	*pLock = DemodLock ;

	return (result == SONY_RESULT_OK) ? TUNER_CTRL_OK : TUNER_CTRL_FAIL;
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
int REALTEK_S_ISDBT_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	sony_result_t result = SONY_RESULT_OK;
	REALTEK_S_ISDBT*	pRealtekSISDBT;
	sony_demod_t*	pDemod;
	sony_isdbt_mode_t mode;
	sony_isdbt_guard_t guard;
	sony_isdbt_tmcc_info_t tmccInfo;
	
	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	//need implement
	pDemodKernel->Lock(pDemodKernel);
	result = sony_demod_isdbt_monitor_ModeGuard (pDemod, &mode, &guard);
	if (result == SONY_RESULT_OK) {
		switch ((unsigned char)mode) {
		case SONY_ISDBT_MODE_1 :
			pInfo->isdbt.trans_mode = OFDM_FFT_2K;
			break;
		case SONY_ISDBT_MODE_2 :
			pInfo->isdbt.trans_mode = OFDM_FFT_4K;
			break;
		case SONY_ISDBT_MODE_3 :
			pInfo->isdbt.trans_mode = OFDM_FFT_8K;
			break;
		}
		REALTEK_S_ISDBT_INFO("ISDBT_FFT_Mode = %d\n", pInfo->isdbt.trans_mode);

		switch ((unsigned char)guard) {
		case SONY_ISDBT_GUARD_1_32 :
			pInfo->isdbt.guard_interval = OFDM_GI_1_32;
			break;
		case SONY_ISDBT_GUARD_1_16 :
			pInfo->isdbt.guard_interval = OFDM_GI_1_16;
			break;
		case SONY_ISDBT_GUARD_1_8 :
			pInfo->isdbt.guard_interval = OFDM_GI_1_8;
			break;
		case SONY_ISDBT_GUARD_1_4 :
			pInfo->isdbt.guard_interval = OFDM_GI_1_4;
			break;
		}
		REALTEK_S_ISDBT_INFO("guard_interval = %d\n", pInfo->isdbt.guard_interval);
	}else {
		REALTEK_S_ISDBT_INFO (" Mode Guard              | Error           | %s\n", Common_Result[result]);
	}
	

	result |= sony_demod_isdbt_monitor_TMCCInfo (pDemod, &tmccInfo);
	if (result == SONY_RESULT_OK) {
		//Layer A
		switch ((unsigned short)tmccInfo.currentInfo.layerA.modulation) {
		case SONY_ISDBT_MODULATION_DQPSK :
			pInfo->isdbt.layer[0].constellation = OFDM_CONST_DQPSK;
			break;
		case SONY_ISDBT_MODULATION_QPSK :
			pInfo->isdbt.layer[0].constellation = OFDM_CONST_4;
			break;
		case SONY_ISDBT_MODULATION_16QAM :
			pInfo->isdbt.layer[0].constellation = OFDM_CONST_16;
			break;
		case SONY_ISDBT_MODULATION_64QAM :
			pInfo->isdbt.layer[0].constellation = OFDM_CONST_64;
			break;
		}
		switch ((unsigned char)tmccInfo.currentInfo.layerA.codingRate) {
		case SONY_ISDBT_CODING_RATE_1_2 :
			pInfo->isdbt.layer[0].code_rate = OFDM_CODE_RATE_1_2;
			break;
		case SONY_ISDBT_CODING_RATE_2_3 :
			pInfo->isdbt.layer[0].code_rate = OFDM_CODE_RATE_2_3;
			break;
		case SONY_ISDBT_CODING_RATE_3_4 :
			pInfo->isdbt.layer[0].code_rate = OFDM_CODE_RATE_3_4;
			break;
		case SONY_ISDBT_CODING_RATE_5_6 :
			pInfo->isdbt.layer[0].code_rate = OFDM_CODE_RATE_5_6;
			break;
		case SONY_ISDBT_CODING_RATE_7_8 :
			pInfo->isdbt.layer[0].code_rate = OFDM_CODE_RATE_7_8;
			break;
		}
		pInfo->isdbt.layer[0].segment_num = tmccInfo.currentInfo.layerA.segmentsNum;
		REALTEK_S_ISDBT_INFO("[Layer A]Segment %d, Constellation %d, Code Rate %d\n",
						pInfo->isdbt.layer[0].segment_num,
						pInfo->isdbt.layer[0].constellation,
						pInfo->isdbt.layer[0].code_rate);

		//Layer B
		switch ((unsigned short)tmccInfo.currentInfo.layerB.modulation) {
		case SONY_ISDBT_MODULATION_DQPSK :
			pInfo->isdbt.layer[1].constellation = OFDM_CONST_DQPSK;
			break;
		case SONY_ISDBT_MODULATION_QPSK :
			pInfo->isdbt.layer[1].constellation = OFDM_CONST_4;
			break;
		case SONY_ISDBT_MODULATION_16QAM :
			pInfo->isdbt.layer[1].constellation = OFDM_CONST_16;
			break;
		case SONY_ISDBT_MODULATION_64QAM :
			pInfo->isdbt.layer[1].constellation = OFDM_CONST_64;
			break;
		}
		switch ((unsigned char)tmccInfo.currentInfo.layerB.codingRate) {
		case SONY_ISDBT_CODING_RATE_1_2 :
			pInfo->isdbt.layer[1].code_rate = OFDM_CODE_RATE_1_2;
			break;
		case SONY_ISDBT_CODING_RATE_2_3 :
			pInfo->isdbt.layer[1].code_rate = OFDM_CODE_RATE_2_3;
			break;
		case SONY_ISDBT_CODING_RATE_3_4 :
			pInfo->isdbt.layer[1].code_rate = OFDM_CODE_RATE_3_4;
			break;
		case SONY_ISDBT_CODING_RATE_5_6 :
			pInfo->isdbt.layer[1].code_rate = OFDM_CODE_RATE_5_6;
			break;
		case SONY_ISDBT_CODING_RATE_7_8 :
			pInfo->isdbt.layer[1].code_rate = OFDM_CODE_RATE_7_8;
			break;
		}
		pInfo->isdbt.layer[1].segment_num = tmccInfo.currentInfo.layerB.segmentsNum;
		REALTEK_S_ISDBT_INFO("[Layer B]Segment %d, Constellation %d, Code Rate %d\n",
						pInfo->isdbt.layer[1].segment_num,
						pInfo->isdbt.layer[1].constellation,
						pInfo->isdbt.layer[1].code_rate);

		//Layer C
		switch ((unsigned short)tmccInfo.currentInfo.layerC.modulation) {
		case SONY_ISDBT_MODULATION_DQPSK :
			pInfo->isdbt.layer[2].constellation = OFDM_CONST_DQPSK;
			break;
		case SONY_ISDBT_MODULATION_QPSK :
			pInfo->isdbt.layer[2].constellation = OFDM_CONST_4;
			break;
		case SONY_ISDBT_MODULATION_16QAM :
			pInfo->isdbt.layer[2].constellation = OFDM_CONST_16;
			break;
		case SONY_ISDBT_MODULATION_64QAM :
			pInfo->isdbt.layer[2].constellation = OFDM_CONST_64;
			break;
		}
		switch ((unsigned char)tmccInfo.currentInfo.layerC.codingRate) {
		case SONY_ISDBT_CODING_RATE_1_2 :
			pInfo->isdbt.layer[2].code_rate = OFDM_CODE_RATE_1_2;
			break;
		case SONY_ISDBT_CODING_RATE_2_3 :
			pInfo->isdbt.layer[2].code_rate = OFDM_CODE_RATE_2_3;
			break;
		case SONY_ISDBT_CODING_RATE_3_4 :
			pInfo->isdbt.layer[2].code_rate = OFDM_CODE_RATE_3_4;
			break;
		case SONY_ISDBT_CODING_RATE_5_6 :
			pInfo->isdbt.layer[2].code_rate = OFDM_CODE_RATE_5_6;
			break;
		case SONY_ISDBT_CODING_RATE_7_8 :
			pInfo->isdbt.layer[2].code_rate = OFDM_CODE_RATE_7_8;
			break;
		}
		pInfo->isdbt.layer[2].segment_num = tmccInfo.currentInfo.layerC.segmentsNum;
		REALTEK_S_ISDBT_INFO("[Layer C]Segment %d, Constellation %d, Code Rate %d\n",
						pInfo->isdbt.layer[2].segment_num,
						pInfo->isdbt.layer[2].constellation,
						pInfo->isdbt.layer[2].code_rate);
	}else {
		REALTEK_S_ISDBT_INFO(" TMCC Information        | Error           |  %s\n", Common_Result[result]);
	}
	pDemodKernel->Unlock(pDemodKernel);

	return result;
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
int REALTEK_S_ISDBT_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality)
{
	sony_result_t result = SONY_RESULT_OK;

	unsigned char DemodLock = 0, TsLock = 0, EarlyUnlock = 0;
	int SNR = 0, TxRSSIdBm = 0;
	unsigned int BerA = 0, BerB = 0, BerC = 0;
	unsigned int PerA = 0, PerB = 0, PerC = 0;
	unsigned int ifAGCOut = 0;
	//unsigned int preBCH[4] = {0};
	//unsigned int postBCH[4] = {0};
	//unsigned int pen[4] = {0};
	//int i = 0;
	//unsigned char  FrameLock = 0;


	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;


	//pDemodKernel->GetLockStatus(pDemodKernel, &FrameLock);

	pDemodKernel->Lock(pDemodKernel);

	switch (id) {
	case TV_QUAL_NOSIG :
		result = sony_demod_isdbt_monitor_SyncStat(pDemod, &DemodLock, &TsLock, &EarlyUnlock);
		pQuality->nosig = (DemodLock) ? 0 : 1;
		break;

	case TV_QUAL_INNERLOCK :
		result = sony_demod_isdbt_monitor_SyncStat(pDemod, &DemodLock, &TsLock, &EarlyUnlock);
		pQuality->inner = (DemodLock) ? 1 : 0;
		break;

	case TV_QUAL_SNR:

		result = sony_demod_isdbt_monitor_SNR(pDemod, &SNR);//SNR(dB) * 1000 
		//REALTEK_S_ISDBT_INFO (" SNR                     | SNR             | %d x 10^-3 dB\n", snr);
		pQuality->snr = SNR * 10000;
		break;

	case TV_QUAL_BEC:
	case TV_QUAL_BER:
		//sony_demod_isdbt_monitor_PreRSBER
		
		result = sony_demod_isdbt_monitor_PreRSBER (pDemod, &BerA, &BerB, &BerC);
		pQuality->ber = BerA;
		pQuality->layer_ber.ber_num = BerA;
		pQuality->layer_ber.ber_den = 1000000000;
		//REALTEK_S_ISDBT_INFO (" Pre-RS BER              | Layer A         | %u x 10^-7\n", berA);
		break;

	case TV_QUAL_PEC:
		result = sony_demod_isdbt_monitor_PacketErrorNumber (pDemod, &PerA, &PerB, &PerC);
		pQuality->layer_per.per_num = PerA;
		pQuality->layer_per.per_den = 100000;
		break;
	case TV_QUAL_PER:
 		result = sony_demod_isdbt_monitor_PER (pDemod, &PerA, &PerB, &PerC);
		pQuality->per = PerA;
		//printf (" PER                     | Layer A         | %u x 10^-6\n", perA);
		break;

	case TV_QUAL_UCBLOCKS:

		pQuality->ucblocks = 0;
		result = SONY_RESULT_OK;

		break;
	case TV_QUAL_SIGNAL_STRENGTH:
		//- Signal strength = 100 when RSSI = +10 dBm			
		//- Signal strength decreases 1 unit for every 1dB decreases for RSSI
		TxRSSIdBm = (int)pQuality->rflevel;
		if(TxRSSIdBm >= 10)
			pQuality->strength = 100;
		else {
			pQuality->strength = TxRSSIdBm + 90;
		}
		break;
	case TV_QUAL_SIGNAL_QUALITY:
		result = sony_demod_isdbt_monitor_SNR(pDemod, &SNR);//SNR(dB) * 1000
		pQuality->quality = SNR * 2 / 1000;
		break;
	case TV_QUAL_AGC:
		result = sony_demod_isdbt_monitor_IFAGCOut (pDemod, &ifAGCOut);
		pQuality->agc = ifAGCOut;
		//REALTEK_S_ISDBT_INFO (" IF AGC                  | IF AGC          | %u\n", ifAGCOut);
		break;
	default:
		REALTEK_S_ISDBT_WARNING("unknown ENUM_TV_QUAL id = %u\n", (unsigned int) id);
		result = SONY_RESULT_OK;
		break;
	}

	pDemodKernel->Unlock(pDemodKernel);
	return (result == SONY_RESULT_OK) ? TUNER_CTRL_OK : TUNER_CTRL_FAIL;
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
int REALTEK_S_ISDBT_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	sony_result_t result = SONY_RESULT_OK;
	int offset;

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;


	pDemodKernel->Lock(pDemodKernel);


	result = sony_demod_isdbt_monitor_CarrierOffset(pDemod, &offset);


	if (result == SONY_RESULT_OK) {
		/*-----------------------------------------------------------------
		  CrOffset = Current Frequency - Target Frequency
		  returned offset = Target Freq - Current Offset = -CrOffset
		  Note: REALTEK_R ATSC Demod will handle IF polarity automatically
		------------------------------------------------------------------*/
		*pOffset = offset;
	} else
		*pOffset = 0;

	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_S_ISDBT_INFO("Carrier Offset = "PT_S32BITS" Hz\n", *pOffset);

	return (result == SONY_RESULT_OK) ? TUNER_CTRL_OK : TUNER_CTRL_FAIL;
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
int REALTEK_S_ISDBT_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	return ret;
}
/*----------------------------------------------------------------------
 * Func : AcquireSignal
 *
 * Desc : Reset Demodulator and Acquire Signal.
 *        This function should be called when channel changed.
 *
 * Parm : WaitSignalLock : wait until signal lock
 *        0 : don't wait
 *          others: wait for signal lock
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_S_ISDBT_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned long cur_time;
#endif

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;


#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	cur_time = tv_osal_time();

	while ((tv_osal_time() - cur_time) <= 15) {
		if (pRealtekSISDBT->m_acquire_sig_en == 0)
			return TUNER_CTRL_OK;

		tv_osal_msleep(5);
	}

	return TUNER_CTRL_FAIL;

#else
	REALTEK_S_ISDBT_AcquireSignalThread(pDemodKernel);
	//pRealtekSISDBT->m_autotune_stime = tv_osal_time() + AUTODETECT_SIG_UPDATE_INTERVAL_MS;
	return TUNER_CTRL_OK;
#endif
}

/*----------------------------------------------------------------------
 * Func : AcquireSignalThread
 *
 * Desc : Reset Demodulator and Acquire Signal.
 *        This function should be called when channel changed.
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_S_ISDBT_AcquireSignalThread(DEMOD* pDemodKernel)
{
	unsigned long   stime;
	unsigned char DemodLock = 0, TsLock = 0, EarlyUnlock = 0;

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	if (pRealtekSISDBT->m_acquire_sig_en == 0)
		return TUNER_CTRL_OK;


	REALTEK_S_ISDBT_INFO("\033[1;32;32m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);


	pDemodKernel->Lock(pDemodKernel);
	pRealtekSISDBT->m_auto_tune_enable = 0;
	stime = tv_osal_time();

	while ((tv_osal_time() - stime) <= 3500) {
		sony_demod_isdbt_monitor_SyncStat(pDemod, &DemodLock, &TsLock, &EarlyUnlock);
		if ((DemodLock) || (EarlyUnlock == 1)) break;
		tv_osal_msleep(5);
	}

	REALTEK_S_ISDBT_INFO("No Signal = %d, DemodLock = %d, Time = %lu\n", EarlyUnlock, DemodLock, tv_osal_time() - stime);

//acquire_signal_result:
	pDemodKernel->Unlock(pDemodKernel);
	pRealtekSISDBT->m_acquire_sig_en = 0;
	pRealtekSISDBT->m_status_checking_stime = tv_osal_time() + ISDBT_STATUS_CHECK_INTERVAL_MS;
	pRealtekSISDBT->m_auto_tune_enable = 1;
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
int REALTEK_S_ISDBT_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range)
{
	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

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
int REALTEK_S_ISDBT_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

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
int REALTEK_S_ISDBT_AutoTune(DEMOD* pDemodKernel)
{
	REALTEK_S_ISDBT* pRealtekSISDBT;
	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);

	pDemodKernel->Lock(pDemodKernel);
	pDemodKernel->Unlock(pDemodKernel);

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
int REALTEK_S_ISDBT_KernelAutoTune(DEMOD* pDemodKernel)
{
	int result = SONY_RESULT_OK;
	unsigned long cur_time;
	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned char ewsChange;
	unsigned char LockStatus = 0;
	TV_SIG_QUAL Quality;
#endif
	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	if(pRealtekSISDBT->m_auto_tune_enable == 0)
		return result;

	cur_time = tv_osal_time();

#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	if ((cur_time > pRealtekSISDBT->m_status_checking_stime) || ((pRealtekSISDBT->m_status_checking_stime - cur_time) > ISDBT_STATUS_CHECK_INTERVAL_MS)) {
		result = sony_demod_isdbt_monitor_EWSChange (pDemod, &ewsChange);
		if (result == SONY_RESULT_OK) {
			REALTEK_S_ISDBT_INFO (" EWSChange               | EWS Change      | %d\n", ewsChange);
			if (ewsChange == 1) {
				sony_demod_isdbt_ClearEWSChange(pDemod);
			}
		}
		REALTEK_S_ISDBT_GetLockStatus(pDemodKernel, &LockStatus);
		if (LockStatus == 1) {
			REALTEK_S_ISDBT_GetSignalQuality(pDemodKernel, TV_QUAL_SNR, &Quality);
			REALTEK_S_ISDBT_GetSignalQuality(pDemodKernel, TV_QUAL_BER, &Quality);
			REALTEK_S_ISDBT_GetSignalQuality(pDemodKernel, TV_QUAL_PER, &Quality);
			REALTEK_S_ISDBT_GetSignalQuality(pDemodKernel, TV_QUAL_SIGNAL_STRENGTH, &Quality);
			REALTEK_S_ISDBT_GetSignalQuality(pDemodKernel, TV_QUAL_SIGNAL_QUALITY, &Quality);
			REALTEK_S_ISDBT_GetSignalQuality(pDemodKernel, TV_QUAL_AGC, &Quality);
		}
	}
#endif

	if (pRealtekSISDBT->m_acquire_sig_en) {
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
		REALTEK_S_ISDBT_AcquireSignalThread(pDemodKernel);
		//pRealtekADVBTX->m_autotune_stime = tv_osal_time() + AUTODETECT_SIG_UPDATE_INTERVAL_MS;
#endif
	}

	return result;
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
int REALTEK_S_ISDBT_InspectSignal(DEMOD* pDemodKernel)
{

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : Activate REALTEK_R ISDBT Demod
 *
 * Desc : Run Auto Tuning for CCI
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_S_ISDBT_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{

	REALTEK_S_ISDBT* pRealtekSISDBT;
	sony_demod_t* pDemod;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_S_ISDBT_DRIVER_DATA*) pRealtekSISDBT->m_private)->pDemod;

	return TUNER_CTRL_OK;

}


int REALTEK_S_ISDBT_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;
	REALTEK_S_ISDBT* pRealtekSISDBT;

	pRealtekSISDBT = (REALTEK_S_ISDBT*)(pDemodKernel->private_data);
	pDemodKernel->Lock(pDemodKernel);

	REALTEK_S_ISDBT_INFO("REALTEK_S_ISDBT_SetTvMode(%d)\n", mode);

	switch (mode) {
	case TV_SYS_TYPE_ISDBT:
		REALTEK_S_ISDBT_INFO("Set to ISDBT \n");
		ret = pDemodKernel->Init(pDemodKernel);
		ret |= pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts);

		if (ret != TUNER_CTRL_OK) {
			REALTEK_S_ISDBT_WARNING("Set REALTEK_S_ISDBT to ISDBT mode failed\n");
			goto set_demod_mode_failed;
		}
		break;

	default:
		REALTEK_S_ISDBT_WARNING("unknown TV_SYSTEM_TYPE\n");
		goto set_demod_mode_failed;
	}

	ret = TUNER_CTRL_OK;
	REALTEK_S_ISDBT_INFO("Set REALTEK_S_ISDBT to appointed DTV mode successed\n");
	pDemodKernel->Unlock(pDemodKernel);
	return ret;

set_demod_mode_failed:
	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_FAIL;
}


/*-----------------------------------------------------------------
 *     BASE_INTERFACE for REALTEK_S_ISDBT API
 *----------------------------------------------------------------*/
void REALTEK_S_ISDBT_Lock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("LOCK 0x%x\n",&pDemodKernel->m_lock);
	//mutex_lock(&pDemodKernel->m_lock);
}


void REALTEK_S_ISDBT_Unlock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("UN_LOCK 0x%x\n",&pDemodKernel->m_lock);
	//mutex_unlock(&pDemodKernel->m_lock);
}


void ReleaseRealtekSIsdbtDriver(REALTEK_S_ISDBT_DRIVER_DATA *pDriver)
{
	kfree(pDriver);
}


REALTEK_S_ISDBT_DRIVER_DATA* AllocRealtekSIsdbtDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
	//,int TSMode
)
{
	REALTEK_S_ISDBT_DRIVER_DATA* pDriver = (REALTEK_S_ISDBT_DRIVER_DATA*) kmalloc(
				sizeof(REALTEK_S_ISDBT_DRIVER_DATA) +
				sizeof(sony_demod_t)   +
				sizeof(I2C_BRIDGE_MODULE)   +
				sizeof(BASE_INTERFACE_MODULE), GFP_KERNEL);

	if (pDriver) {
		BASE_INTERFACE_MODULE* pBIF;
		//sony_demod_t* pDemod;

		memset(pDriver, 0, sizeof(REALTEK_S_ISDBT_DRIVER_DATA));
		pDriver->pDemod                = (sony_demod_t*)(((unsigned char*)pDriver) + sizeof(REALTEK_S_ISDBT_DRIVER_DATA));
		pDriver->pBaseInterface        = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(sony_demod_t));
		pDriver->pI2CBridge            = (I2C_BRIDGE_MODULE*)(((unsigned char*)pDriver->pBaseInterface) + sizeof(BASE_INTERFACE_MODULE));
		pDriver->DeviceAddr            = Addr;
		pDriver->CrystalFreqHz         = CrystalFreq;

		// Init Base IF
		pBIF = &BaseInterface;
		pBIF->I2cReadingByteNumMax      = RTK_DEMOD_BIF_RX_FIFO_DEPTH;
		pBIF->I2cWritingByteNumMax      = RTK_DEMOD_BIF_TX_FIFO_DEPTH;
		pBIF->I2cRead                   = __realtek_i2c_read;
		pBIF->I2cWrite                  = __realtek_i2c_write;
		pBIF->WaitMs                    = __realtek_wait_ms;
		pBIF->SetUserDefinedDataPointer = base_interface_SetUserDefinedDataPointer;
		pBIF->GetUserDefinedDataPointer = base_interface_GetUserDefinedDataPointer;
		pBIF->SetUserDefinedDataPointer(pBIF, (void*)pComm);
		pBIF->useI2C = 0;


	}

	return pDriver;
}


sony_result_t sony_i2c_dummyRead(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t* pData, uint32_t size, uint8_t mode)
{
	/*
		Implement I2C functions
	*/

	int ret = FUNCTION_SUCCESS;

	ret = __realtek_i2c_read(&BaseInterface, deviceAddress, pData, (U32BITS) size);

	SONY_TRACE_I2C_RETURN((ret == FUNCTION_SUCCESS) ? SONY_RESULT_OK : SONY_RESULT_ERROR_I2C);

}

sony_result_t sony_i2c_dummyWrite(sony_i2c_t* pI2c, uint8_t deviceAddress, const uint8_t *pData, uint32_t size, uint8_t mode)
{
	/*
		Implement I2C functions
	*/

	//int i;
	//printf("Write : 0x%02X ", deviceAddress);
	//printf("0x%02X ", pData[0]);
	//for(i = 1; i < size; i++) printf("0x%02X ", pData[i]);
	//printf("\n");

	int ret = FUNCTION_SUCCESS;
	ret = __realtek_i2c_write(&BaseInterface, deviceAddress, pData, (U32BITS) size);

	SONY_TRACE_I2C_RETURN((ret == FUNCTION_SUCCESS) ? SONY_RESULT_OK : SONY_RESULT_ERROR_I2C);
}

