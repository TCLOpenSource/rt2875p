/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author danielwei@realtek.com
 *
 *****************************************************************************/
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <asm/cacheflush.h>
#include <linux/proc_fs.h>

#include "comm.h"
#include "tuner.h"
#include "demod.h"
#include "demod_rtk_all.h"
#include "demod_rtk_common.h"
#include "DemodMgr.h"
#include "rtk_demod_khal_transform_common.h"

#include "khal_common.h"
#include "khal_demod_common.h"

static KHAL_DEMOD_ANALOG_CONFIG_T tATVParam;
static TV_SIG_INFO SigInfo;
static TV_SIG_INFO *pMyInfo = NULL;
static TS_PARAM tsParam;

extern PS_RTK_DEMOD_DATA pDemod_globe;
extern struct semaphore rtkdemod_Semaphore;
extern unsigned char isDemodConstructors;
extern unsigned char isUseDDemod;

int KHAL_DEMOD_ATV_Initialize(void)
{
	RTK_DEMOD_CONSTRUCTORS demod_init_data;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATV_Initialize @@@\n");
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

	memset(&tATVParam, 0, sizeof(tATVParam));
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
EXPORT_SYMBOL(KHAL_DEMOD_ATV_Initialize);

int KHAL_DEMOD_ATV_ChangeTransMedia(KHAL_DEMOD_TRANS_SYSTEM_T transSystem)
{
	TV_SYSTEM_TYPE systype = TV_SYS_TYPE_UNKNOWN;
	int ret;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATV_ChangeTransMedia @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATV_Initialize Firstly\n");
		return NOT_OK;
	}
	down(&rtkdemod_Semaphore);

	KHAL_DEMOD_INFO("transSystem: %d \n", transSystem);

	switch (transSystem) {
	case KHAL_DEMOD_TRANS_SYS_NTSC:
		systype = TV_SYS_TYPE_MN;
		break;
	case KHAL_DEMOD_TRANS_SYS_PAL:
		systype = TV_SYS_TYPE_BGDKIL;
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

	up(&rtkdemod_Semaphore);
	return API_OK;

SwitchRet:
	KHAL_DEMOD_WARNING("transSystem: %d ---------FAIL\n", transSystem);
	up(&rtkdemod_Semaphore);
	return NOT_OK;;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATV_ChangeTransMedia);

int KHAL_DEMOD_ATV_SetDemod(KHAL_DEMOD_ANALOG_CONFIG_T paramStruct)
{
	TV_SYSTEM sys = TV_SYS_UNKNOWN;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATV_SetDemod @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATV_Initialize Firstly\n");
		return NOT_OK;
	}
	down(&rtkdemod_Semaphore);

	if ((paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_NTSC) && (paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_PAL)) {
		KHAL_DEMOD_WARNING("\033[1;32;32m" "KHAL_DEMOD_ATV_SetDemod but the standard is not ATV!!!! standard=%d\n" "\033[m", paramStruct.transSystem);
		goto SetATVDemodRet;
	}

	KHAL_DEMOD_INFO("centerFreq:%u, tunedFreq:%u, bSpectrumInv:%d, transSystem:%d, tuneMode:%d, channelBW:%d, audioSystem:%d\n",
					paramStruct.centerFreq, paramStruct.tunedFreq, paramStruct.bSpectrumInv, paramStruct.transSystem, paramStruct.tuneMode,	paramStruct.channelBW, paramStruct.audioSystem);

	SigInfo.analog.scan_mode = ((paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN) || (paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN_START)) ? 1 : 0;
	
	switch (paramStruct.transSystem) {
	case KHAL_DEMOD_TRANS_SYS_NTSC:
	    if (paramStruct.audioSystem == KHAL_DEMOD_AUDIO_SIF_SYSTEM_MN)
			sys = TV_SYS_M_NTSC;
		else
			sys = TV_SYS_UNKNOWN;
		break;
		
	case KHAL_DEMOD_TRANS_SYS_PAL:
		if (paramStruct.audioSystem == KHAL_DEMOD_AUDIO_SIF_SYSTEM_BG)
			sys = TV_SYS_BG_PAL;
		else if (paramStruct.audioSystem == KHAL_DEMOD_AUDIO_SIF_SYSTEM_I)
			sys = TV_SYS_I_PAL;
		else if (paramStruct.audioSystem == KHAL_DEMOD_AUDIO_SIF_SYSTEM_DK)
			sys = TV_SYS_DK_PAL;
		else if (paramStruct.audioSystem == KHAL_DEMOD_AUDIO_SIF_SYSTEM_MN)
			sys = TV_SYS_M_PAL;
		else if (paramStruct.audioSystem == KHAL_DEMOD_AUDIO_SIF_SYSTEM_L)
			sys = TV_SYS_L_SECAM;
		else if (paramStruct.audioSystem == KHAL_DEMOD_AUDIO_SIF_SYSTEM_LP)
			sys = TV_SYS_LL_SECAM;
		else
			sys = TV_SYS_UNKNOWN;
		break;

	default:
		sys = TV_SYS_UNKNOWN;
		break;
	}

	if (sys == TV_SYS_UNKNOWN) 
		goto  SetATVDemodRet;

	memcpy(&tATVParam, &paramStruct, sizeof(KHAL_DEMOD_ANALOG_CONFIG_T));

	if (pDemod_globe->m_pDemod->SetTVSysEx(pDemod_globe->m_pDemod, sys, &SigInfo) != TUNER_CTRL_OK)
		goto SetATVDemodRet;

	//pDemod_globe->set_tv_sys_ex_data.sys = sys;
	//memcpy(&pDemod_globe->set_tv_sys_ex_data.pSigInfo, &SigInfo, sizeof(TV_SIG_INFO));
	up(&rtkdemod_Semaphore);
	return API_OK;

SetATVDemodRet:
	KHAL_DEMOD_WARNING(" %s (%d) --------------------FAIL \n", __func__, __LINE__);
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATV_SetDemod);

int KHAL_DEMOD_ATV_TunePostJob(BOOLEAN *pFinished)
{
	unsigned char WaitSignalLock = 1;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATV_TunePostJob @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATV_Initialize Firstly\n");
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

	KHAL_DEMOD_INFO("ATV_TunePostJob ret = %d, Finished = %d\n", ret, *pFinished);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATV_TunePostJob);

int KHAL_DEMOD_ATV_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState)
{
	DTV_STATUS ret = API_NOT_OK;
	unsigned char bSignalLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATV_CheckLock @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATV_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pLockState == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetLockStatus(pDemod_globe->m_pDemod, &bSignalLock) == TUNER_CTRL_OK) {
		ret = API_OK;
		*pLockState = bSignalLock ? KHAL_DEMOD_LOCK_OK : KHAL_DEMOD_LOCK_FAIL;
	}

	KHAL_DEMOD_INFO("ATV LOCK status (0:unlock, 1:lock):---------(%d)\n", bSignalLock);
	up(&rtkdemod_Semaphore);
	return ret;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATV_CheckLock);

int KHAL_DEMOD_ATV_CheckSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState)
{
	TV_SIG_QUAL Quality;
	UINT8 bValue = 0;
	
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATV_CheckSignalState @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATV_Initialize Firstly\n");
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

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_AGC, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->unAGC = Quality.agc;
	
	pSignalState->packetError = 0;
	pSignalState->unBER = 0;
	pSignalState->unSNR = 0;

	KHAL_DEMOD_INFO("SQI:(%d), AGC:(%d), SigValid:(%d)\n", pSignalState->quality, pSignalState->unAGC, pSignalState->bSignalValid);

	up(&rtkdemod_Semaphore);
	return API_OK;
	
Demod_GetSignalStateRet:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATV_CheckSignalState);

int KHAL_DEMOD_ATV_CheckFrequencyOffset(SINT32 *pFreqOffset)
{
	S32BITS offset;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATV_CheckFrequencyOffset @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATV_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_ATV_CheckFrequencyOffset);

int KHAL_DEMOD_ATV_GetFWVersion(UINT32 *pFWVersion)
{
	TV_SIG_QUAL Quality;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATV_GetFWVersion @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATV_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_ATV_GetFWVersion);

int KHAL_DEMOD_ATV_DebugMenu(void)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ATV_DebugMenu @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_ATV_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ATV_DebugMenu);


