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

static KHAL_DEMOD_ISDBT_SET_PARAM_T tISDBTParam;
static TV_SIG_INFO SigInfo;
static TV_SIG_INFO *pMyInfo = NULL;
static TS_PARAM tsParam;

extern PS_RTK_DEMOD_DATA pDemod_globe;
extern struct semaphore rtkdemod_Semaphore;
extern unsigned char isDemodConstructors;
extern unsigned char isUseDDemod;

UINT32	KHAL_DEMOD_VQI_ISDBT_Probe(struct i2c_client *client)
{
	
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_Probe);

int KHAL_DEMOD_VQI_ISDBT_Initialize(void)
{

       RTK_DEMOD_CONSTRUCTORS demod_init_data; 
    
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_ISDBT_Initialize @@@\n");

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

	memset(&tISDBTParam, 0, sizeof(tISDBTParam));
	memset(&SigInfo, 0, sizeof(TV_SIG_INFO));
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_Initialize);


int KHAL_DEMOD_VQI_ISDBT_ChangeTransSystem(KHAL_DEMOD_TRANS_SYSTEM_T transSystem)
{
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_ISDBT_ChangeTransSystem @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);

	if ((transSystem == KHAL_DEMOD_TRANS_SYS_NTSC) || (transSystem == KHAL_DEMOD_TRANS_SYS_PAL) || (transSystem == KHAL_DEMOD_TRANS_SYS_ALWAYSREADY)) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_VQI_ISDBT_ChangeTransSystem but the standard is analog(standard = %d)! DTV demod is in sleep mode.\n" "\033[m", transSystem);
		pDemod_globe->m_pDemod->SetTvMode(pDemod_globe->m_pDemod, TV_SYS_TYPE_UNKNOWN);
		pDemod_globe->set_tv_mode_data.mode = TV_SYS_TYPE_UNKNOWN;
		goto SwitchRetOk;
	}

	if (transSystem != KHAL_DEMOD_TRANS_SYS_ISDBT) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_VQI_ISDBT_ChangeTransSystem but the standard is not ISDBT!!!! standard=%d\n" "\033[m", transSystem);
		goto SwitchRet;
	}

	isUseDDemod = 1;
	ret = pDemod_globe->m_pDemod->SetTvMode(pDemod_globe->m_pDemod, TV_SYS_TYPE_ISDBT);

	if (ret != TUNER_CTRL_OK)
		goto SwitchRet;

	pDemod_globe->set_tv_mode_data.mode = TV_SYS_TYPE_ISDBT;

SwitchRetOk:
	up(&rtkdemod_Semaphore);
	return API_OK;

SwitchRet:
	KHAL_DEMOD_WARNING("transSystem: %d ---------fail\n", transSystem);
	up(&rtkdemod_Semaphore);
	return NOT_OK;

}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_ChangeTransSystem);


int KHAL_DEMOD_VQI_ISDBT_SetDemod(KHAL_DEMOD_ISDBT_SET_PARAM_T paramStruct)
{
	TV_SYSTEM sys;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_ISDBT_SetDemod @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	if (paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_ISDBT) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_VQI_ISDBT_SetDemod but the standard is not ISDBT!!!! standard=%d\n" "\033[m", paramStruct.transSystem);
		goto SetISDBTDemodRet;
	}


	KHAL_DEMOD_INFO("tuneMode:%d transSystem:%d, eChannelBW:%d, bSpectrumInv:%d, carrierMode:%d, guardInterval:%d, codeRate:%d, constellation:%d\n",
					paramStruct.tuneMode, paramStruct.transSystem, paramStruct.eChannelBW, paramStruct.bSpectrumInv, paramStruct.carrierMode, paramStruct.guardInterval, paramStruct.codeRate, paramStruct.constellation);


	SigInfo.mod = TV_MODULATION_ISDBT;
	SigInfo.isdbt.scan_mode = ((paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN) || (paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN_START)) ? 1 : 0;
	SigInfo.isdbt.bSpectrumInv = paramStruct.bSpectrumInv;

	switch (paramStruct.eChannelBW) {

	case KHAL_DEMOD_CH_BW_6M:
		sys = TV_SYS_ISDBT_6M;
		break;
	//case KHAL_DEMOD_CH_BW_7M:
	//	sys = TV_SYS_ISDBT_7M;
	//	break;
	case KHAL_DEMOD_CH_BW_8M:
		sys = TV_SYS_ISDBT_8M;
		break;

	default:
		sys = TV_SYS_UNKNOWN;
	}

	if (sys == TV_SYS_UNKNOWN) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_VQI_ISDBT_SetDemod but the BW is out fo range!!!! BW=%d\n" "\033[m", paramStruct.eChannelBW);
		goto  SetISDBTDemodRet;
	}


	memcpy(&tISDBTParam, &paramStruct, sizeof(KHAL_DEMOD_ISDBT_SET_PARAM_T));

	if (pDemod_globe->m_pDemod->SetTVSysEx(pDemod_globe->m_pDemod, sys, &SigInfo) != TUNER_CTRL_OK)
		goto SetISDBTDemodRet;

	//pDemod_globe->set_tv_sys_ex_data.sys = sys;
	//memcpy(&pDemod_globe->set_tv_sys_ex_data.pSigInfo, &SigInfo, sizeof(TV_SIG_INFO));

	up(&rtkdemod_Semaphore);
	return API_OK;

SetISDBTDemodRet:

	KHAL_DEMOD_WARNING(" %s (%d)--------------------FAIL \n", __func__, __LINE__);
	up(&rtkdemod_Semaphore);

	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_SetDemod);


int KHAL_DEMOD_VQI_ISDBT_TunePostJob(BOOLEAN *pFinished)
{
	unsigned char WaitSignalLock = 1;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_ISDBT_TunePostJob @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
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

	KHAL_DEMOD_INFO("ISDBT_TunePostJob ret = %d, Finished = %d\n", ret, *pFinished);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_TunePostJob);



int KHAL_DEMOD_VQI_ISDBT_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState)
{
	DTV_STATUS ret = API_NOT_OK;
	unsigned char bSignalLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_ISDBT_CheckLock @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pLockState == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetLockStatus(pDemod_globe->m_pDemod, &bSignalLock) == TUNER_CTRL_OK) {
		ret = API_OK;
		*pLockState = bSignalLock ?  KHAL_DEMOD_LOCK_OK : KHAL_DEMOD_LOCK_FAIL;
	}
	KHAL_DEMOD_INFO("ISDBT LOCK status (0:unlock, 1:lock):---------(%d)\n", bSignalLock);
	up(&rtkdemod_Semaphore);
	return ret;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_CheckLock);



int KHAL_DEMOD_VQI_ISDBT_CheckSpecialData(KHAL_DEMOD_SPECDATA_ISDBT_T *pSpecISDB)
{
    unsigned char MaxSeglayer = 0;

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
		return NOT_OK;
	}
    
	if (pSpecISDB == NULL)
		return API_INVALID_PARAMS;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_ISDBT_CheckSpecialData @@@\n");

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK){
		goto DvbtGetSpecialRet;
	}   
        MaxSeglayer = 0;      //default MaxSeglayer set to layer_a and find bigger segmentNum to set MaxSeglayer
        if(pMyInfo->isdbt.layer[1].segment_num > pMyInfo->isdbt.layer[0].segment_num)
            MaxSeglayer = 1;

        if((pMyInfo->isdbt.layer[2].segment_num > pMyInfo->isdbt.layer[0].segment_num)&&(pMyInfo->isdbt.layer[2].segment_num > pMyInfo->isdbt.layer[1].segment_num))
            MaxSeglayer = 2;
        
        KHAL_DEMOD_DBG("MaxSeglayer = %d\n", MaxSeglayer);
   
		pSpecISDB->carrierMode = (KHAL_DEMOD_TPS_CARRIER_MODE_T)trans_CarrierMode_Translate(pMyInfo->isdbt.trans_mode, false);
		pSpecISDB->guardInterval = (KHAL_DEMOD_TPS_GUARD_INTERVAL_T)trans_GuardCode_Translate(pMyInfo->isdbt.guard_interval, false);    
		pSpecISDB->codeRate = (KHAL_DEMOD_TPS_CODERATE_T)trans_CodeRate_Translate(pMyInfo->isdbt.layer[MaxSeglayer].code_rate, false);
		pSpecISDB->constellation = (KHAL_DEMOD_TPS_CONSTELLATION_T)trans_ConstelCode_Translate(pMyInfo->isdbt.layer[MaxSeglayer].constellation, false);

        KHAL_DEMOD_INFO("KHAL_DEMOD_TPS_CARRIER_MODE_T =%d, KHAL_DEMOD_TPS_GUARD_INTERVAL_T =%d, KHAL_DEMOD_constellation =%d, KHAL_DEMOD_codeRate =%d\n", pSpecISDB->carrierMode, pSpecISDB->guardInterval, pSpecISDB->constellation, pSpecISDB->codeRate);
        
	up(&rtkdemod_Semaphore);
	return API_OK;

DvbtGetSpecialRet:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_CheckSpecialData);



int KHAL_DEMOD_VQI_ISDBT_CheckSignalStatus(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState)
{

	TV_SIG_QUAL Quality;
	UINT8 bValue = 0;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_ISDBT_CheckSignalStatus @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
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
					pSignalState->quality,  pSignalState->unAGC, pSignalState->packetError , pSignalState->bSignalValid);

	up(&rtkdemod_Semaphore);
	return API_OK;
Demod_GetSignalStateRet:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_CheckSignalStatus);



int KHAL_DEMOD_VQI_ISDBT_CheckFrequencyOffset(SINT32 *pFreqOffset)
{
	S32BITS offset;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_ISDBT_CheckFrequencyOffset @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_CheckFrequencyOffset);


int KHAL_DEMOD_VQI_ISDBT_ControlTSMode(BOOLEAN bIsSerial)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_ControlTSMode @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_ControlTSMode);



int KHAL_DEMOD_VQI_ISDBT_ControlOutput(BOOLEAN bEnableOutput)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_ControlOutput @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_ControlOutput);



int KHAL_DEMOD_VQI_ISDBT_GetFWVersion(UINT32 *pFWVersion)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetFWVersion @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_GetFWVersion);



int KHAL_DEMOD_ISDBT_DebugMenu(void)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_ISDBT_DebugMenu @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_ISDBT_DebugMenu);


int	KHAL_DEMOD_VQI_ISDBT_Monitor(KHAL_DEMOD_LOCK_STATE_T *pLockState)
{
	DTV_STATUS ret = API_NOT_OK;
	unsigned char bSignalLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_VQI_ISDBT_Monitor @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pLockState == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetLockStatus(pDemod_globe->m_pDemod, &bSignalLock) == TUNER_CTRL_OK) {
		ret = API_OK;
		*pLockState = bSignalLock ?  KHAL_DEMOD_LOCK_OK : KHAL_DEMOD_LOCK_FAIL;
	}
	KHAL_DEMOD_INFO("ISDBT LOCK status (0:unlock, 1:lock):---------(%d)\n", bSignalLock);
	up(&rtkdemod_Semaphore);
	return ret;
}
EXPORT_SYMBOL(KHAL_DEMOD_VQI_ISDBT_Monitor);

BOOLEAN KHAL_DEMOD_GetEmergencyAlertFlagStatus(void)
{

	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_GetEmergencyAlertFlagStatus @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_VQI_ISDBT_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_EMERGENCY_FLAG, &Quality) != TUNER_CTRL_OK)
		return API_NOT_OK;

	KHAL_DEMOD_INFO("EmergencyAlertFlagStatus: %d \n", Quality.emergency_flag);

	up(&rtkdemod_Semaphore);
	return Quality.emergency_flag ? API_OK : API_NOT_OK;

}
EXPORT_SYMBOL(KHAL_DEMOD_GetEmergencyAlertFlagStatus);

