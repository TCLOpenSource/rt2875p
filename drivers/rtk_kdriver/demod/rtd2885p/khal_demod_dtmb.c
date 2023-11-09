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

static KHAL_DEMOD_DTMB_SET_PARAM_T tDTMBParam;
static TV_SIG_INFO SigInfo;
static TV_SIG_INFO *pMyInfo = NULL;
static TS_PARAM   tsParam;


extern PS_RTK_DEMOD_DATA pDemod_globe;
extern struct semaphore rtkdemod_Semaphore;
extern unsigned char isDemodConstructors;
extern unsigned char isUseDDemod;


UINT32 KHAL_DEMOD_VQI_DTMB_Probe(struct i2c_client *client)
{
	return API_NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_Probe);

int KHAL_DEMOD_VQI_DTMB_Initialize(void)
{
       RTK_DEMOD_CONSTRUCTORS demod_init_data; 
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_Initialize @@@\n");

	down(&rtkdemod_Semaphore);

	if (!isDemodConstructors) {
		pDemod_globe->m_comm = kmalloc(sizeof(COMM), GFP_KERNEL);
		pDemod_globe->m_pTuner = kmalloc(sizeof(TUNER), GFP_KERNEL);
		pDemod_globe->m_pDemod = kmalloc(sizeof(DEMOD), GFP_KERNEL);

		demod_init_data.m_addr = 0x0;
		demod_init_data.m_output_freq = 0x0;
		demod_init_data.m_output_mode = RTK_DEMOD_OUT_IF_PARALLEL;

		Comm_Constructors(pDemod_globe->m_comm, 0);
		Tuner_Constructors(pDemod_globe->m_pTuner);
		REALTEK_ALL_Constructors(pDemod_globe->m_pDemod, demod_init_data.m_addr, demod_init_data.m_output_mode, demod_init_data.m_output_freq, pDemod_globe->m_comm);
		pDemod_globe->m_pDemod->AttachTuner(pDemod_globe->m_pDemod, pDemod_globe->m_pTuner);
		isDemodConstructors = 1;

	}

	if (!pMyInfo)
		pMyInfo = kmalloc(sizeof(TV_SIG_INFO), GFP_KERNEL);


	memset(&tDTMBParam, 0, sizeof(tDTMBParam));
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_Initialize);

int KHAL_DEMOD_VQI_DTMB_ChangeTransSystem(KHAL_DEMOD_TRANS_SYSTEM_T transSystem)
{
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_ChangeTransSystem @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);

	if ((transSystem == KHAL_DEMOD_TRANS_SYS_NTSC) || (transSystem == KHAL_DEMOD_TRANS_SYS_PAL) || (transSystem == KHAL_DEMOD_TRANS_SYS_ALWAYSREADY)) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_VQI_DTMB_ChangeTransSystem but the standard is analog(standard = %d)! DTV demod is in sleep mode.\n" "\033[m", transSystem);
		pDemod_globe->m_pDemod->SetTvMode(pDemod_globe->m_pDemod, TV_SYS_TYPE_UNKNOWN);
		pDemod_globe->set_tv_mode_data.mode = TV_SYS_TYPE_UNKNOWN;
		goto SwitchRetOk;
	}

	if (transSystem != KHAL_DEMOD_TRANS_SYS_DTMB) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_VQI_DTMB_ChangeTransSystem but the standard is not DTMB!!!! standard=%d\n" "\033[m", transSystem);
		goto SwitchRet;
	}

	isUseDDemod = 1;
	ret = pDemod_globe->m_pDemod->SetTvMode(pDemod_globe->m_pDemod, TV_SYS_TYPE_DTMB);

	if (ret != TUNER_CTRL_OK)
		goto SwitchRet;

	pDemod_globe->set_tv_mode_data.mode = TV_SYS_TYPE_DTMB;

SwitchRetOk:
	up(&rtkdemod_Semaphore);
	return API_OK;

SwitchRet:
	KHAL_DEMOD_WARNING("transSystem: %d ---------fail\n", transSystem);
	up(&rtkdemod_Semaphore);
	return NOT_OK;

}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_ChangeTransSystem);

int KHAL_DEMOD_VQI_DTMB_SetDemod(KHAL_DEMOD_DTMB_SET_PARAM_T paramStruct)
{
	TV_SYSTEM sys;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_SetDemod @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);

	if (paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_DTMB) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_VQI_DTMB_SetDemod but the standard is not DTMB!!!! standard=%d\n" "\033[m", paramStruct.transSystem);
		goto SetDTMBDemodRet;
	}

	KHAL_DEMOD_INFO("tuneMode:%d transSystem:%d, eChannelBW:%d, bM720:%d, carrierMode:%d, guardInterval:%d, codeRate:%d, constellation:%d\n",
					paramStruct.tuneMode, paramStruct.transSystem, paramStruct.eChannelBW, paramStruct.bM720, paramStruct.carrierMode, paramStruct.guardInterval, paramStruct.codeRate, paramStruct.constellation);

	SigInfo.dtmb_ofdm.scan_mode = ((paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN) || (paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN_START)) ? 1 : 0;

	switch (paramStruct.eChannelBW) {

	case KHAL_DEMOD_CH_BW_6M:
		sys = TV_SYS_DTMB_6M;
		break;
	case KHAL_DEMOD_CH_BW_8M:
		sys = TV_SYS_DTMB;
		break;

	default:
		sys = TV_SYS_UNKNOWN;
	}

	if (sys == TV_SYS_UNKNOWN) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_VQI_DTMB_SetDemod but the BW is out fo range!!!! BW=%d\n" "\033[m", paramStruct.eChannelBW);
		goto  SetDTMBDemodRet;
	}


	memcpy(&tDTMBParam, &paramStruct, sizeof(KHAL_DEMOD_DTMB_SET_PARAM_T));

	if (pDemod_globe->m_pDemod->SetTVSysEx(pDemod_globe->m_pDemod, sys, &SigInfo) != TUNER_CTRL_OK)
		goto SetDTMBDemodRet;

	//pDemod_globe->set_tv_sys_ex_data.sys = sys;
	//memcpy(&pDemod_globe->set_tv_sys_ex_data.pSigInfo, &SigInfo, sizeof(TV_SIG_INFO));

	up(&rtkdemod_Semaphore);
	return API_OK;

SetDTMBDemodRet:

	KHAL_DEMOD_WARNING(" %s (%d)--------------------FAIL \n", __func__, __LINE__);
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_SetDemod);

int KHAL_DEMOD_VQI_DTMB_TunePostJob(BOOLEAN *pFinished)
{
	unsigned char WaitSignalLock = 1;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_TunePostJob @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
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

	KHAL_DEMOD_INFO("DTMB_TunePostJob ret = %d, Finished = %d\n", ret, *pFinished);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_TunePostJob);

int KHAL_DEMOD_VQI_DTMB_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState)
{
	DTV_STATUS ret = API_NOT_OK;
	unsigned char bSignalLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_CheckLock @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pLockState == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetLockStatus(pDemod_globe->m_pDemod, &bSignalLock) == TUNER_CTRL_OK) {
		ret = API_OK;
		*pLockState = bSignalLock ?  KHAL_DEMOD_LOCK_OK : KHAL_DEMOD_LOCK_FAIL;
	}
	KHAL_DEMOD_INFO("DTMB LOCK status (0:unlock, 1:lock):---------(%d)\n", bSignalLock);
	up(&rtkdemod_Semaphore);
	return ret;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_CheckLock);

int KHAL_DEMOD_VQI_DTMB_CheckSpecialData(KHAL_DEMOD_SPECDATA_DTMB_T *pSpecDTMB)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_CheckSpecialData @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSpecDTMB == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK)
		goto DvbtGetSpecialRet;

	pSpecDTMB->bM720            = ((pMyInfo->dtmb_ofdm.tim == DTMB_OFDM_TIM_720) ? true : false);
	pSpecDTMB->carrierMode      = (KHAL_DEMOD_TPS_CARRIER_MODE_T)    DTMB_trans_carrierMode_Translate(pMyInfo->dtmb_ofdm.carrier_mode, false);
	pSpecDTMB->guardInterval    = (KHAL_DEMOD_TPS_GUARD_INTERVAL_T)  DTMB_trans_guardInterval_Translate(pMyInfo->dtmb_ofdm.pn.mode, pMyInfo->dtmb_ofdm.pn.seq, false);
	pSpecDTMB->codeRate         = (KHAL_DEMOD_TPS_CODERATE_T)        DTMB_trans_codeRate_Translate(pMyInfo->dtmb_ofdm.fec.rate, false);
	pSpecDTMB->constellation    = (KHAL_DEMOD_TPS_CONSTELLATION_T)   DTMB_trans_constellation_Translate(pMyInfo->dtmb_ofdm.fec.constellation, false);

	KHAL_DEMOD_INFO("DTMB_CheckSpecialData: bm720:%d, carrierMode:%d, guardInterval:%d, codeRate:%d, constellation:%d\n",
					pSpecDTMB->bM720, pSpecDTMB->carrierMode, pSpecDTMB->guardInterval, pSpecDTMB->codeRate, pSpecDTMB->constellation);
	up(&rtkdemod_Semaphore);
	return API_OK;

DvbtGetSpecialRet:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_CheckSpecialData);

int KHAL_DEMOD_VQI_DTMB_CheckSignalStatus(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState)
{

	TV_SIG_QUAL Quality;
	UINT8 bValue = 0;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_CheckSignalStatus @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSignalState == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetLockStatus(pDemod_globe->m_pDemod, &bValue) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;

	pSignalState->bSignalValid = bValue ? TRUE : FALSE;

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_SIGNAL_STRENGTH, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->strength = Quality.strength;

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_SIGNAL_QUALITY, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->quality = Quality.quality;

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_PEC, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->packetError = Quality.layer_per.per_num;

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_BEC, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->unBER = Quality.layer_ber.ber_num;

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_AGC, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->unAGC = Quality.agc;

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_SNR, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->unSNR = Quality.snr / 10000;

	KHAL_DEMOD_INFO("SNR: %d, BER:(%d), SQI:(%d), AGC:(%d) , packageError:(%d),  SigValid:(%d) \n", pSignalState->unSNR, pSignalState->unBER,
					pSignalState->quality,  pSignalState->unAGC, pSignalState->packetError , pSignalState->bSignalValid);

	up(&rtkdemod_Semaphore);
	return API_OK;
Demod_GetSignalStateRet:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_CheckSignalStatus);


int KHAL_DEMOD_VQI_DTMB_CheckFrequencyOffset(SINT32 *pFreqOffset)
{
	S32BITS offset;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_CheckFrequencyOffset @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_CheckFrequencyOffset);

int KHAL_DEMOD_VQI_DTMB_ControlTSMode(BOOLEAN bIsSerial)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_ControlTSMode @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_ControlTSMode);

int KHAL_DEMOD_VQI_DTMB_ControlOutput(BOOLEAN bEnableOutput)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_DTMB_ControlOutput @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_ControlOutput);

int KHAL_DEMOD_VQI_DTMB_GetFWVersion(UINT32 *pFWVersion)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetFWVersion @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_DTMB_GetFWVersion);

int KHAL_DEMOD_DTMB_DebugMenu(void)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DTMB_DebugMenu @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_DTMB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DTMB_DebugMenu);
