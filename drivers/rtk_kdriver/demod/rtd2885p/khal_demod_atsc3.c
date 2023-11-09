/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author danielwei@realtek.com
 *
 *****************************************************************************/

#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <asm/cacheflush.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>

#include "comm.h"
#include "tuner.h"
#include "demod.h"
#include "demod_rtk_all.h"
#include "demod_rtk_common.h"
#include "DemodMgr.h"
#include "rtk_demod_khal_transform_common.h"

#include "khal_common.h"
#include "khal_demod_common.h"

extern PS_RTK_DEMOD_DATA pDemod_globe;
extern struct semaphore rtkdemod_Semaphore;
extern unsigned char isDemodConstructors;
extern unsigned char isUseDDemod;


static KHAL_DEMOD_ATSC3_SET_PARAM_T tATSC3Param;
static TV_SIG_INFO SigInfo;
static TV_SIG_INFO *pMyInfo = NULL;
static TS_PARAM   tsParam;


UINT32 KHAL_DEMOD_ATSC3_Probe(UINT8 portI2C)
{
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_Probe);

int KHAL_DEMOD_ATSC3_Initialize(void)
{
	RTK_DEMOD_CONSTRUCTORS demod_init_data;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_Initialize @@@\n");
	down(&rtkdemod_Semaphore);

	if (!isDemodConstructors) {
		pDemod_globe->m_comm = kmalloc(sizeof(COMM), GFP_KERNEL);
		pDemod_globe->m_pTuner = kmalloc(sizeof(TUNER), GFP_KERNEL);
		pDemod_globe->m_pDemod = kmalloc(sizeof(DEMOD), GFP_KERNEL);

		demod_init_data.m_addr = 0x70;
		demod_init_data.m_output_freq = 0x0;
		demod_init_data.m_output_mode = RTK_DEMOD_OUT_IF_PARALLEL;

		Comm_Constructors(pDemod_globe->m_comm, 2);
		Tuner_Constructors(pDemod_globe->m_pTuner);
		REALTEK_ALL_Constructors(pDemod_globe->m_pDemod, demod_init_data.m_addr, demod_init_data.m_output_mode, demod_init_data.m_output_freq, pDemod_globe->m_comm);
		pDemod_globe->m_pDemod->AttachTuner(pDemod_globe->m_pDemod, pDemod_globe->m_pTuner);
		isDemodConstructors = 1;

	}

	if (!pMyInfo)
		pMyInfo = kmalloc(sizeof(TV_SIG_INFO), GFP_KERNEL);

	memset(&tATSC3Param, 0, sizeof(tATSC3Param));
	memset(&SigInfo, 0, sizeof(SigInfo));
	memset(pMyInfo, 0, sizeof(TV_SIG_INFO));

	tsParam.mode = PARALLEL_MODE;
	tsParam.data_order = MSB_D7;
	tsParam.datapin  = MSB_FIRST;
	tsParam.err_pol  = ACTIVE_HIGH;
	tsParam.sync_pol = ACTIVE_HIGH;
	tsParam.val_pol  = ACTIVE_HIGH;
	tsParam.clk_pol  = RISING_EDGE;
	tsParam.internal_demod_input = 1;
	tsParam.tsOutEn = 1;

	up(&rtkdemod_Semaphore);

	return API_OK;

}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_Initialize);

int KHAL_DEMOD_ATSC3_ChangeTransSystem(KHAL_DEMOD_TRANS_SYSTEM_T transSystem)
{
	TV_SYSTEM_TYPE  systype = TV_SYS_TYPE_UNKNOWN;
	int ret;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_ChangeTransSystem @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	KHAL_DEMOD_INFO("transSystem: %d \n", transSystem);

	if ((transSystem == KHAL_DEMOD_TRANS_SYS_NTSC) || (transSystem == KHAL_DEMOD_TRANS_SYS_PAL) || (transSystem == KHAL_DEMOD_TRANS_SYS_ALWAYSREADY)) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_ATSC3_ChangeTransSystem but the standard is analog(standard = %d)! DTV demod is in sleep mode.\n" "\033[m", transSystem);
		pDemod_globe->m_pDemod->SetTvMode(pDemod_globe->m_pDemod, TV_SYS_TYPE_UNKNOWN);
		pDemod_globe->set_tv_mode_data.mode = TV_SYS_TYPE_UNKNOWN;
		goto SwitchRetOk;
	}


	switch (transSystem) {
	case KHAL_DEMOD_TRANS_SYS_ATSC3:
		systype = TV_SYS_TYPE_ATSC3P0;
		break;
	default:
		systype = TV_SYS_TYPE_UNKNOWN;
		break;
	}

	if (systype == TV_SYS_TYPE_UNKNOWN)
		goto SwitchRet;

	isUseDDemod = 1;

	ret = pDemod_globe->m_pDemod->SetTvMode(pDemod_globe->m_pDemod, systype);

	if (ret != TUNER_CTRL_OK)
		goto SwitchRet;

	pDemod_globe->set_tv_mode_data.mode = systype;

SwitchRetOk:

	up(&rtkdemod_Semaphore);
	return API_OK;

SwitchRet:

	KHAL_DEMOD_WARNING("transSystem: %d ---------fail\n", transSystem);
	up(&rtkdemod_Semaphore);
	return NOT_OK;;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_ChangeTransSystem);

int KHAL_DEMOD_ATSC3_TunePostJob(BOOLEAN *pFinished)
{
	unsigned char WaitSignalLock = 1;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_TunePostJob @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pFinished == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	ret = pDemod_globe->m_pDemod->AcquireSignal(pDemod_globe->m_pDemod, WaitSignalLock);

	if (ret == TUNER_CTRL_OK)
		*pFinished = TRUE;
	else
		*pFinished = FALSE;

	KHAL_DEMOD_INFO("[ATSC3]TunePostJob ret = %d, Finished = %d\n", ret, *pFinished);

	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_TunePostJob);

int KHAL_DEMOD_ATSC3_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState)
{
	DTV_STATUS ret = API_NOT_OK;
	unsigned char bSignalLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_CheckLock @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pLockState == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetLockStatus(pDemod_globe->m_pDemod, &bSignalLock) == TUNER_CTRL_OK) {
		ret = API_OK;
		if (bSignalLock) {
			*pLockState = KHAL_DEMOD_LOCK_OK;
		} else {
			*pLockState = KHAL_DEMOD_LOCK_FAIL;
		}
	}

	KHAL_DEMOD_INFO("[ATSC3]LOCK status (0:unlock, 1:lock):---------(%d)\n", bSignalLock);
	up(&rtkdemod_Semaphore);
	return ret;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_CheckLock);

int KHAL_DEMOD_ATSC3_CheckSpecialData(KHAL_DEMOD_SPECDATA_ATSC3_T *pSpecATSC3)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_CheckSpecialData @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSpecATSC3 == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK)
		goto Atsc3GetSpecialRet;


	pSpecATSC3-> bMPLP = (pMyInfo->atsc3.plp_cnt > 1) ? 1 : 0;


	switch (pMyInfo->atsc3.fft_mode[0]) {
	case ATSC3_MODE_8K:
		pSpecATSC3-> carrierMode = KHAL_DEMOD_TPS_CARR_8K;
		break;
	case ATSC3_MODE_16K:
		pSpecATSC3-> carrierMode = KHAL_DEMOD_TPS_CARR_16K;
		break;
	case ATSC3_MODE_32K:
		pSpecATSC3-> carrierMode = KHAL_DEMOD_TPS_CARR_32K;
		break;
	case ATSC3_MODE_UNKNOWN:
	default:
		pSpecATSC3-> carrierMode = KHAL_DEMOD_TPS_CARR_UNKNOWN;
		break;
	}


	switch (pMyInfo->atsc3.guard_interval[0]) {
	case ATSC3_GI1_192:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_1_192;
		break;
	case ATSC3_GI2_384:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_2_384;
		break;
	case ATSC3_GI3_512:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_3_512;
		break;
	case ATSC3_GI4_768:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_4_768;
		break;
	case ATSC3_GI5_1024:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_5_1024;
		break;
	case ATSC3_GI6_1536:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_6_1536;
		break;
	case ATSC3_GI7_2048:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_7_2048;
		break;
	case ATSC3_GI8_2432:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_8_2432;
		break;
	case ATSC3_GI9_3072:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_9_3072;
		break;
	case ATSC3_GI10_3648:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_10_3648;
		break;
	case ATSC3_GI11_4096:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_11_4096;
		break;
	case ATSC3_GI12_4864:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_12_4864;
		break;
	default:
		pSpecATSC3-> guardInterval = KHAL_DEMOD_TPS_GUARD_ATSC3_UNKNOWN;
		break;
	}


	switch (pMyInfo->atsc3.code_rate[0]) {
	case ATSC3_CODE_RATE_2_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_2_15;
		break;
	case ATSC3_CODE_RATE_3_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_3_15;
		break;
	case ATSC3_CODE_RATE_4_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_4_15;
		break;
	case ATSC3_CODE_RATE_5_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_5_15;
		break;
	case ATSC3_CODE_RATE_6_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_6_15;
		break;
	case ATSC3_CODE_RATE_7_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_7_15;
		break;
	case ATSC3_CODE_RATE_8_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_8_15;
		break;
	case ATSC3_CODE_RATE_9_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_9_15;
		break;
	case ATSC3_CODE_RATE_10_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_10_15;
		break;
	case ATSC3_CODE_RATE_11_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_11_15;
		break;
	case ATSC3_CODE_RATE_12_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_12_15;
		break;
	case ATSC3_CODE_RATE_13_15:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_13_15;
		break;
	default:
		pSpecATSC3-> codeRate = KHAL_DEMOD_TPS_CODE_ATSC3_UNKNOWN;
		break;
	}


	switch (pMyInfo->atsc3.constellation[0]) {
	case ATSC3_QPSK:
		pSpecATSC3-> constellation = KHAL_DEMOD_TPS_CONST_ATSC3_QPSK;
		break;
	case ATSC3_QAM_16:
		pSpecATSC3-> constellation = KHAL_DEMOD_TPS_CONST_ATSC3_QAM_16;
		break;
	case ATSC3_QAM_64:
		pSpecATSC3-> constellation = KHAL_DEMOD_TPS_CONST_ATSC3_QAM_64;
		break;
	case ATSC3_QAM_256:
		pSpecATSC3-> constellation = KHAL_DEMOD_TPS_CONST_ATSC3_QAM_256;
		break;
	case ATSC3_QAM_1024:
		pSpecATSC3-> constellation = KHAL_DEMOD_TPS_CONST_ATSC3_QAM_1024;
		break;
	case ATSC3_QAM_4096:
		pSpecATSC3-> constellation = KHAL_DEMOD_TPS_CONST_ATSC3_QAM_4096;
		break;
	default:
		pSpecATSC3-> constellation = KHAL_DEMOD_TPS_CONST_ATSC3_UNKNOWN;
		break;
	}


	up(&rtkdemod_Semaphore);
	return API_OK;
Atsc3GetSpecialRet:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_CheckSpecialData);

int KHAL_DEMOD_ATSC3_CheckFrequencyOffset(SINT32 * pFreqOffset)
{
	S32BITS offset;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_CheckFrequencyOffset @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pFreqOffset == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetCarrierOffset(pDemod_globe->m_pDemod, &offset) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return NOT_OK;
	} else {
		*pFreqOffset = offset / 1000;
		up(&rtkdemod_Semaphore);
		return API_OK;
	}
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_CheckFrequencyOffset);

int KHAL_DEMOD_ATSC3_GetFWVersion(UINT32 * pFWVersion)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_GetFWVersion @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);


	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_DEMODFWVERSION, &Quality) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	*pFWVersion = (UINT32)Quality.DemodFwVersion;
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_GetFWVersion);

int KHAL_DEMOD_ATSC3_GetSQI(UINT8 * pSQI)
{

	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_GetSQI @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSQI == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_SIGNAL_QUALITY, &Quality) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	*pSQI = (UINT8) Quality.quality;
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_GetSQI);

int KHAL_DEMOD_ATSC3_GetPacketError(UINT32 * pPacketError)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_GetPacketError @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pPacketError == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_PEC, &Quality) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return NOT_OK;
	}

	*pPacketError = (UINT32) Quality.layer_per.per_num;
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_GetPacketError);

int KHAL_DEMOD_ATSC3_GetBER(UINT32 * pBER)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_GetBER @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pBER == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_BEC, &Quality) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	*pBER = (UINT32) Quality.layer_ber.ber_num;
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_GetBER);

int KHAL_DEMOD_ATSC3_GetAGC(UINT32 * pAGC)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_GetAGC @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pAGC == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_AGC, &Quality) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	*pAGC = (UINT32)Quality.agc;
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_GetAGC);

int KHAL_DEMOD_ATSC3_GetSNR(UINT32 * pSNR)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_GetSNR @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSNR == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_SNR, &Quality) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	*pSNR = (UINT32)Quality.snr / 10000;
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_GetSNR);

int KHAL_DEMOD_ATSC3_ControlOutput(BOOLEAN bEnableOutput)
{

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_ControlOutput @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	KHAL_DEMOD_INFO("bEnableOutput:[%d]\n", bEnableOutput);

	tsParam.tsOutEn = bEnableOutput;

	if (pDemod_globe->m_pDemod->SetTSMode(pDemod_globe->m_pDemod, &tsParam) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_ControlOutput);

int KHAL_DEMOD_ATSC3_ControlTSMode(BOOLEAN bIsSerial)
{

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_ControlTSMode @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	tsParam.mode = bIsSerial ? SERIAL_MODE : PARALLEL_MODE;

	if (pDemod_globe->m_pDemod->SetTSMode(pDemod_globe->m_pDemod, &tsParam) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_ControlTSMode);

int KHAL_DEMOD_ATSC3_CheckSignalStatus(KHAL_DEMOD_SIGNAL_STATE_T * pSignalState)
{
	TV_SIG_QUAL Quality;
	UINT8 bValue = 0;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_CheckSignalStatus @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSignalState == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetLockStatus(pDemod_globe->m_pDemod, &bValue) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;

	pSignalState->bSignalValid = bValue ? TRUE : FALSE;


	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_CUSTOMER_LG, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;

	pSignalState->strength = Quality.strength;
	pSignalState->quality = Quality.quality;
	pSignalState->packetError = Quality.layer_per.per_num;
	pSignalState->unBER = Quality.layer_ber.ber_num;
	pSignalState->unAGC = Quality.agc;
	pSignalState->unSNR = Quality.snr / 10000;

	KHAL_DEMOD_INFO("SNR: %d, BER:(%d), SQI:(%d), AGC:(%d) , packageError:(%d),  SigValid:(%d) \n", pSignalState->unSNR, pSignalState->unBER,
					pSignalState->quality,  pSignalState->unAGC, pSignalState->packetError, pSignalState->bSignalValid);
	up(&rtkdemod_Semaphore);
	return API_OK;

Demod_GetSignalStateRet:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_CheckSignalStatus);

int KHAL_DEMOD_ATSC3_Monitor(KHAL_DEMOD_LOCK_STATE_T * pLockState)
{
	DTV_STATUS ret = API_NOT_OK;
	unsigned char bSignalLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_Monitor @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pLockState == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetLockStatus(pDemod_globe->m_pDemod, &bSignalLock) == TUNER_CTRL_OK) {
		ret = API_OK;
		if (bSignalLock) {
			*pLockState = KHAL_DEMOD_LOCK_OK;
		} else {
			*pLockState = KHAL_DEMOD_LOCK_FAIL;
		}
	}

	KHAL_DEMOD_INFO("[ATSC3]LOCK status (0:unlock, 1:lock):---------(%d)\n", bSignalLock);
	up(&rtkdemod_Semaphore);
	return ret;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_Monitor);

int KHAL_DEMOD_ATSC3_DebugMenu(void)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_DebugMenu @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_DebugMenu);

int KHAL_DEMOD_ATSC3_Get_MPLP_Info(KHAL_DEMOD_ATSC3_MULTI_PLP_ID_T * pPlpParams, KHAL_DEMOD_ATSC3_MULTI_PLP_ID_SEL_T plpSel)
{


	int i32CntTmp = 0;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_Get_MPLP_Info @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pPlpParams == NULL)
		return API_INVALID_PARAMS;

	if ((plpSel != FULL_PLP_ID) && (plpSel != LLS_ONLY_PLP_ID))
		return API_OK;


	down(&rtkdemod_Semaphore);
	if (pDemod_globe->m_pDemod->GetAtsc3PLPInfo(pDemod_globe->m_pDemod, pMyInfo, (unsigned char) FULL_PLP_ID) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}
	pPlpParams->total_plpCount = pMyInfo->atsc3.plp_cnt;

	if (plpSel == FULL_PLP_ID) {
		pPlpParams->selected_plpCount = pMyInfo->atsc3.plp_cnt;
		memcpy(pPlpParams->plpID, &pMyInfo->atsc3.plp, sizeof(pMyInfo->atsc3.plp));
	} else {

		if (pDemod_globe->m_pDemod->GetAtsc3PLPInfo(pDemod_globe->m_pDemod, pMyInfo, (unsigned char) LLS_ONLY_PLP_ID) != TUNER_CTRL_OK) {
			up(&rtkdemod_Semaphore);
			return API_NOT_OK;
		}
		pPlpParams->selected_plpCount = pMyInfo->atsc3.plp_cnt;
	memcpy(pPlpParams->plpID, &pMyInfo->atsc3.plp, sizeof(pMyInfo->atsc3.plp));
	}


	KHAL_DEMOD_INFO("Total PLPCount=%d, Selected PLPCount=%d\n", pPlpParams->total_plpCount, pPlpParams->selected_plpCount);

	for (i32CntTmp = 0; i32CntTmp < pPlpParams->selected_plpCount; i32CntTmp++) {
		KHAL_DEMOD_INFO("PLP[%d]=0x%x (%u)\n", i32CntTmp, pPlpParams->plpID[i32CntTmp], pPlpParams->plpID[i32CntTmp] & 0x3f);
	}


	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_Get_MPLP_Info);

int KHAL_DEMOD_ATSC3_PLP_Select(UINT8 plp0_id, UINT8 plp1_id, UINT8 plp2_id, UINT8 plp3_id)
{

	unsigned int counterTmp = 0;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_PLP_Select @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	//if (unPLP == 0)
	//	return API_INVALID_PARAMS;

	KHAL_DEMOD_INFO("PLP ID=%u, %u, %u, %u\n", plp0_id, plp1_id, plp2_id, plp3_id);

	SigInfo.atsc3.plp[0] = plp0_id;
	SigInfo.atsc3.plp[1] = plp1_id;
	SigInfo.atsc3.plp[2] = plp2_id;
	SigInfo.atsc3.plp[3] = plp3_id;

	SigInfo.atsc3.plp_cnt = 0;
	for (counterTmp = 0; counterTmp < 4; counterTmp++) {
		if (SigInfo.atsc3.plp[counterTmp] != 0x40)
			SigInfo.atsc3.plp_cnt++;
		else
			break;
	}


	if (pDemod_globe->m_pDemod->ChangeAtsc3PLP(pDemod_globe->m_pDemod, &SigInfo) != TUNER_CTRL_OK)
		goto SetAtsc3PLPRet;

	return API_OK;
SetAtsc3PLPRet:
	return API_NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_PLP_Select);

int KHAL_DEMOD_ATSC3_SetDemodExpand(KHAL_DEMOD_ATSC3_SET_PARAM_T paramStruct, UINT8 plp0_id, UINT8 plp1_id, UINT8 plp2_id, UINT8 plp3_id)
{
	TV_SYSTEM sys;
	unsigned int counterTmp = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATSC3_SetDemodExpand @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATSC3_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	if (paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_ATSC3) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_ATSC3_SetDemodExpand but the standard is not ATSC3!!!! standard=%d\n" "\033[m", paramStruct.transSystem);
		goto SetAtsc3DemodRet;
	}

	KHAL_DEMOD_INFO("tuneMode:%d, transSystem:%d, eChannelBW:%d, bSpectrumInv:%d, constellation:%d\n",
					paramStruct.tuneMode, paramStruct.transSystem, paramStruct.eChannelBW, paramStruct.bSpectrumInv, paramStruct.constellation);

	KHAL_DEMOD_INFO("plp0_id=%u, plp1_id=%u, plp2_id=%u, plp3_id=%u\n", plp0_id, plp1_id, plp2_id, plp3_id);


	SigInfo.mod = TV_MODULATION_ATSC3;
	//SigInfo.atsc3.scan_mode = ((paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN) || (paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN_START)) ? 1 : 0;
	SigInfo.atsc3.mod = TV_MODULATION_ATSC3;
	SigInfo.atsc3.bSpectrumInv = paramStruct.bSpectrumInv;


	SigInfo.atsc3.plp[0] = plp0_id;
	SigInfo.atsc3.plp[1] = plp1_id;
	SigInfo.atsc3.plp[2] = plp2_id;
	SigInfo.atsc3.plp[3] = plp3_id;

	SigInfo.atsc3.plp_cnt = 0;
	for (counterTmp = 0; counterTmp < 4; counterTmp++) {
		if (SigInfo.atsc3.plp[counterTmp] != 0x40)
			SigInfo.atsc3.plp_cnt++;
		else
			break;
	}

	if ((SigInfo.atsc3.plp_cnt == 1) && (SigInfo.atsc3.plp[0] == 0xff)) {  //PLP auto mode
		SigInfo.atsc3.plp[0] = 0;
		SigInfo.atsc3.plp[1] = 0;
		SigInfo.atsc3.plp[2] = 0;
		SigInfo.atsc3.plp[3] = 0;
		SigInfo.atsc3.scan_mode = 1;
	} else {
		SigInfo.atsc3.scan_mode = 0;
	}



	switch (paramStruct.eChannelBW) {

	case KHAL_DEMOD_CH_BW_6M:
		sys = TV_SYS_ATSC3P0_6M;
		break;
	case KHAL_DEMOD_CH_BW_7M:
		sys = TV_SYS_ATSC3P0_7M;
		break;
	case KHAL_DEMOD_CH_BW_8M:
		sys = TV_SYS_ATSC3P0_8M;
		break;

	default:
		sys = TV_SYS_UNKNOWN;

	}

	if (sys == TV_SYS_UNKNOWN) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_ATSC3_SetDemodExpand but the BW is out fo range!!!! BW=%d\n" "\033[m", paramStruct.eChannelBW);
		goto  SetAtsc3DemodRet;
	}


	memcpy(&tATSC3Param, &paramStruct, sizeof(KHAL_DEMOD_ATSC3_SET_PARAM_T));
	if (pDemod_globe->m_pDemod->SetTVSysEx(pDemod_globe->m_pDemod, sys, &SigInfo) != TUNER_CTRL_OK)
		goto SetAtsc3DemodRet;

	//pDemod_globe->set_tv_sys_ex_data.sys = sys;
	//memcpy(&pDemod_globe->set_tv_sys_ex_data.pSigInfo, &SigInfo, sizeof(TV_SIG_INFO));

	up(&rtkdemod_Semaphore);
	return API_OK;
SetAtsc3DemodRet:
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATSC3_SetDemodExpand);
