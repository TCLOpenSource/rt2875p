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


static KHAL_DEMOD_DVBT_SET_PARAM_T tDvbtParam;
static KHAL_DEMOD_DVBT2_SET_PARAM_T tDvbt2Param;
static KHAL_DEMOD_DVBC_SET_PARAM_T tDvbcParam;
static KHAL_DEMOD_DVBS_SET_PARAM_T tDvbsParam;
static TV_SIG_INFO SigInfo;
static TV_SIG_INFO *pMyInfo = NULL;
static TS_PARAM   tsParam;


UINT32 KHAL_DEMOD_DVB_Probe(UINT8 portI2C)
{

	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_Probe);

int KHAL_DEMOD_DVB_Initialize(void)
{
       RTK_DEMOD_CONSTRUCTORS demod_init_data;
    
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_Initialize @@@\n");
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

	memset(&tDvbtParam, 0, sizeof(tDvbtParam));
	memset(&tDvbt2Param, 0, sizeof(tDvbt2Param));
	memset(&tDvbcParam, 0, sizeof(tDvbcParam));
	memset(&tDvbsParam, 0, sizeof(tDvbsParam));
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_Initialize);

int KHAL_DEMOD_DVB_ChangeTransMedia(KHAL_DEMOD_TRANS_SYSTEM_T transSystem)
{
	TV_SYSTEM_TYPE  systype = TV_SYS_TYPE_UNKNOWN;
	int ret;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_ChangeTransMedia @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	KHAL_DEMOD_INFO("transSystem: %d \n", transSystem);

	if ((transSystem == KHAL_DEMOD_TRANS_SYS_NTSC) || (transSystem == KHAL_DEMOD_TRANS_SYS_PAL) || (transSystem == KHAL_DEMOD_TRANS_SYS_ALWAYSREADY)) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_DVB_ChangeTransMedia but the standard is analog(standard = %d)! DTV demod is in sleep mode.\n" "\033[m", transSystem);
		pDemod_globe->m_pDemod->SetTvMode(pDemod_globe->m_pDemod, TV_SYS_TYPE_UNKNOWN);
		pDemod_globe->set_tv_mode_data.mode = TV_SYS_TYPE_UNKNOWN;
		goto SwitchRetOk;
	}


	switch (transSystem) {
	case KHAL_DEMOD_TRANS_SYS_DVBT:
		systype = TV_SYS_TYPE_DVBT;
		break;
	case KHAL_DEMOD_TRANS_SYS_DVBT2:
		systype = TV_SYS_TYPE_DVBT2;
		break;
	case KHAL_DEMOD_TRANS_SYS_DVBC:
		systype = TV_SYS_TYPE_DVBC;
		break;
	case KHAL_DEMOD_TRANS_SYS_DVBC2:
		systype = TV_SYS_TYPE_DVBC;
		break;
	case KHAL_DEMOD_TRANS_SYS_DVBS:
		systype = TV_SYS_TYPE_DVBS;
		break;
	case KHAL_DEMOD_TRANS_SYS_DVBS2:
		systype = TV_SYS_TYPE_DVBS2;
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_ChangeTransMedia);

int KHAL_DEMOD_DVB_DVBT_SetDemod(KHAL_DEMOD_DVBT_SET_PARAM_T paramStruct)
{
	TV_SYSTEM sys;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT_SetDemod @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	if ((paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_DVBT) && (paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_DVBT2)) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_DVB_DVBT_SetDemod but the standard is not DVBT!!!! standard=%d\n" "\033[m", paramStruct.transSystem);
		goto SetDvbtDemodRet;
	}

	KHAL_DEMOD_INFO("tuneMode:%d transSystem:%d, eChannelBW:%d, bSpectrumInv:%d, bProfileHP:%d, hierarchy:%d, carrierMode:%d, guardInterval:%d, codeRate:%d, constellation:%d\n",
					paramStruct.tuneMode, paramStruct.transSystem, paramStruct.eChannelBW, paramStruct.bSpectrumInv, paramStruct.bProfileHP,	paramStruct.hierarchy, paramStruct.carrierMode, paramStruct.guardInterval,	paramStruct.codeRate, paramStruct.constellation);


	SigInfo.mod = TV_MODULATION_OFDM;
	SigInfo.dvbt2_ofdm.scan_mode = ((paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN) || (paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN_START)) ? 1 : 0;
	SigInfo.dvbt2_ofdm.bProfileHP = paramStruct.bProfileHP;
	SigInfo.dvbt2_ofdm.bSpectrumInv = paramStruct.bSpectrumInv;
	//SigInfo.dvbt2_ofdm.constellation = trans_ConstelCode_Translate(paramStruct.constellation, true);
	//SigInfo.dvbt2_ofdm.alpha = trans_Hierarchy_Translate(paramStruct.hierarchy, true);
	//SigInfo.dvbt2_ofdm.guard_interval = trans_Hierarchy_Translate(paramStruct.guardInterval, true);
	//SigInfo.dvbt2_ofdm.fft_mode = trans_Hierarchy_Translate(paramStruct.carrierMode, true);
	//SigInfo.dvbt2_ofdm.code_rate_hp = trans_Hierarchy_Translate(paramStruct.codeRate, true);
	//SigInfo.dvbt2_ofdm.code_rate_lp = trans_Hierarchy_Translate(paramStruct.codeRate, true);
	SigInfo.dvbt2_ofdm.plp_select = 0;
	SigInfo.dvbt2_ofdm.bSpectrumInv = paramStruct.bSpectrumInv;

	switch (paramStruct.eChannelBW) {

	case KHAL_DEMOD_CH_BW_6M:
		sys = ((SigInfo.dvbt2_ofdm.scan_mode == 1) && (SigInfo.dvbt2_ofdm.bProfileHP == 1)) ? TV_SYS_AUTOSCAN_6M : TV_SYS_DVBT_6M;
		break;
	case KHAL_DEMOD_CH_BW_7M:
		sys = ((SigInfo.dvbt2_ofdm.scan_mode == 1) && (SigInfo.dvbt2_ofdm.bProfileHP == 1)) ? TV_SYS_AUTOSCAN_7M : TV_SYS_DVBT_7M;
		break;
	case KHAL_DEMOD_CH_BW_8M:
		sys = ((SigInfo.dvbt2_ofdm.scan_mode == 1) && (SigInfo.dvbt2_ofdm.bProfileHP == 1)) ? TV_SYS_AUTOSCAN_8M : TV_SYS_DVBT_8M;
		break;

	default:
		sys = TV_SYS_UNKNOWN;
	}

	if (sys == TV_SYS_UNKNOWN) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_DVB_DVBT_SetDemod but the BW is out fo range!!!! BW=%d\n" "\033[m", paramStruct.eChannelBW);
		goto  SetDvbtDemodRet;
	}


	memcpy(&tDvbtParam, &paramStruct, sizeof(KHAL_DEMOD_DVBT_SET_PARAM_T));

	if (pDemod_globe->m_pDemod->SetTVSysEx(pDemod_globe->m_pDemod, sys, &SigInfo) != TUNER_CTRL_OK)
		goto SetDvbtDemodRet;

	//pDemod_globe->set_tv_sys_ex_data.sys = sys;
	//memcpy(&pDemod_globe->set_tv_sys_ex_data.pSigInfo, &SigInfo, sizeof(TV_SIG_INFO));

	up(&rtkdemod_Semaphore);
	return API_OK;

SetDvbtDemodRet:

	KHAL_DEMOD_WARNING(" %s (%d)--------------------FAIL \n", __func__, __LINE__);
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT_SetDemod);

int KHAL_DEMOD_DVB_DVBC_SetDemod(KHAL_DEMOD_DVBC_SET_PARAM_T paramStruct)
{
	TV_SYSTEM sys;
	int ret;

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT_SetDemod @@@\n");

	if (paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_DVBC) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_DVB_DVBC_SetDemod but the standard is not DVBC!!!! standard=%d\n" "\033[m", paramStruct.transSystem);
		goto SetDvbcDemodRet;
	}


	SigInfo.mod = TV_MODULATION_DVBC_QAM;
	SigInfo.dvbc_qam.constellation = trans_ConstelCode_Translate(paramStruct.constellation, true);
	SigInfo.dvbc_qam.symbol_rate = paramStruct.symbolRate * 1000;
	SigInfo.dvbc_qam.freq_khz = paramStruct.frequency;
	//SigInfo.dvbc_qam.alpha = QAM_ALPHA_20;
	switch (paramStruct.tuneMode) {
	case KHAL_DEMOD_TUNE_SCAN:
	case KHAL_DEMOD_TUNE_SCAN_START:
		SigInfo.dvbc_qam.scan_mode = 1;
		break;
	case KHAL_DEMOD_TUNE_SPEC_DVBC_FIXED_DATA:
		SigInfo.dvbc_qam.scan_mode = 2;
		break;
	default:
		SigInfo.dvbc_qam.scan_mode = 0;
		break;
	}

	SigInfo.dvbc_qam.spectrum_inverse = paramStruct.bSpectrumInv;

	switch (paramStruct.eChannelBW) {

	case KHAL_DEMOD_CH_BW_6M:
		sys = TV_SYS_DVBC_6M;
		break;
	case KHAL_DEMOD_CH_BW_7M:
		sys = TV_SYS_DVBC_7M;
		break;
	case KHAL_DEMOD_CH_BW_8M:
		sys = TV_SYS_DVBC_8M;
		break;

	default:
		sys = TV_SYS_UNKNOWN;
	}

	if (sys == TV_SYS_UNKNOWN) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_DVB_DVBC_SetDemod but the BW is out fo range!!!! BW=%d\n" "\033[m", paramStruct.eChannelBW);
		goto  SetDvbcDemodRet;
	}

	memcpy(&tDvbcParam, &paramStruct, sizeof(KHAL_DEMOD_DVBC_SET_PARAM_T));

	ret = pDemod_globe->m_pDemod->SetTVSysEx(pDemod_globe->m_pDemod, sys, &SigInfo);

	if (ret != TUNER_CTRL_OK)
		goto SetDvbcDemodRet;

	//pDemod_globe->set_tv_sys_ex_data.sys = sys;
	//memcpy(&pDemod_globe->set_tv_sys_ex_data.pSigInfo, &SigInfo, sizeof(TV_SIG_INFO));

	up(&rtkdemod_Semaphore);
	return API_OK;

SetDvbcDemodRet:
	KHAL_DEMOD_WARNING(" %s (%d)--------------------FAIL \n", __func__, __LINE__);
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBC_SetDemod);

int KHAL_DEMOD_DVB_DVBC_TunePostJob(BOOLEAN *pFinished)
{
	unsigned char WaitSignalLock = 1;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBC_TunePostJob @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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

	KHAL_DEMOD_INFO("DVBC_TunePostJob ret = %d, Finished = %d\n", ret, *pFinished);

	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBC_TunePostJob);

int KHAL_DEMOD_DVB_DVBT_TunePostJob(BOOLEAN *pFinished)
{
	unsigned char WaitSignalLock = 1;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT_TunePostJob @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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

	KHAL_DEMOD_INFO("DVBT_TunePostJob ret = %d, Finished = %d\n", ret, *pFinished);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT_TunePostJob);

/*
int KHAL_DEMOD_DVB_DVBT2_TunePostJob(BOOLEAN *pFinished)
{
	unsigned char WaitSignalLock = 1;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT2_TunePostJob @@@\n");
	if (pFinished == NULL)
		return API_INVALID_PARAMS;

	ret = pDemod_globe->m_pDemod->AcquireSignal(pDemod_globe->m_pDemod, WaitSignalLock);

	if (ret == TUNER_CTRL_OK)
		*pFinished = TRUE;
	else
		*pFinished = FALSE;

	KHAL_DEMOD_INFO("DVBT2_TunePostJob ret = %d, Finished = %d\n", ret, *pFinished);
	return API_OK;
}
*/
int KHAL_DEMOD_DVB_DVBT_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState)
{
	DTV_STATUS ret = API_NOT_OK;
	unsigned char bSignalLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT_CheckLock @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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

	KHAL_DEMOD_INFO("DVBT LOCK status (0:unlock, 1:lock):---------(%d)\n", bSignalLock);
	up(&rtkdemod_Semaphore);
	return ret;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT_CheckLock);

int KHAL_DEMOD_DVB_DVBC_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState)
{
	DTV_STATUS ret = API_NOT_OK;
	unsigned char bSignalLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBC_CheckLock @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
	KHAL_DEMOD_INFO("DVBC LOCK status (0:unlock, 1:lock):---------(%d)\n", bSignalLock);
	up(&rtkdemod_Semaphore);
	return ret;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBC_CheckLock);

int KHAL_DEMOD_DVB_DVBT_GetSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState)
{

	TV_SIG_QUAL Quality;
	UINT8 bValue = 0;
//	UINT32	packetError; , unknown how to get packageError
	//bSignalValid

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT_GetSignalState @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT_GetSignalState);

int KHAL_DEMOD_DVB_DVBC_GetSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState)
{

	UINT8 bValue = 0;
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBC_GetSignalState @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBC_GetSignalState);

int KHAL_DEMOD_DVB_DVBT_CheckSpecialData(KHAL_DEMOD_SPECDATA_DVBT_T *pSpecDVBT)
{
	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSpecDVBT == NULL)
		return API_INVALID_PARAMS;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT_CheckSpecialData @@@\n");

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK)
		goto DvbtGetSpecialRet;

	if (pMyInfo->mod == TV_MODULATION_DVBT2_OFDM) {
		pSpecDVBT->bSpectrumInv = (BOOLEAN)pMyInfo->dvbt2_ofdm.bSpectrumInv;
		pSpecDVBT->hierarchy = (KHAL_DEMOD_TPS_HIERARCHY_T)trans_Hierarchy_Translate(pMyInfo->dvbt2_ofdm.alpha, false);
		pSpecDVBT->carrierMode = (KHAL_DEMOD_TPS_CARRIER_MODE_T)trans_CarrierMode_Translate(pMyInfo->dvbt2_ofdm.fft_mode, false);
		pSpecDVBT->guardInterval = (KHAL_DEMOD_TPS_GUARD_INTERVAL_T)trans_GuardCode_Translate(pMyInfo->dvbt2_ofdm.guard_interval, false);
		pSpecDVBT->codeRate = (KHAL_DEMOD_TPS_CODERATE_T)trans_CodeRate_Translate(pMyInfo->dvbt2_ofdm.code_rate_hp, false);
		pSpecDVBT->constellation = (KHAL_DEMOD_TPS_CONSTELLATION_T)trans_ConstelCode_Translate(pMyInfo->dvbt2_ofdm.constellation, false);
	} else {
		pSpecDVBT->bSpectrumInv = (BOOLEAN)pMyInfo->dvbt2_ofdm.bSpectrumInv;
		pSpecDVBT->bProfileHP = pMyInfo->dvbt2_ofdm.bProfileHP;
		pSpecDVBT->hierarchy = (KHAL_DEMOD_TPS_HIERARCHY_T)trans_Hierarchy_Translate(pMyInfo->dvbt2_ofdm.hierarchy, false);
		pSpecDVBT->carrierMode = (KHAL_DEMOD_TPS_CARRIER_MODE_T)trans_CarrierMode_Translate(pMyInfo->dvbt2_ofdm.fft_mode, false);
		pSpecDVBT->guardInterval = (KHAL_DEMOD_TPS_GUARD_INTERVAL_T)trans_GuardCode_Translate(pMyInfo->dvbt2_ofdm.guard_interval, false);
		pSpecDVBT->codeRate = (KHAL_DEMOD_TPS_CODERATE_T)trans_CodeRate_Translate(pMyInfo->dvbt2_ofdm.code_rate_hp, false);
		pSpecDVBT->constellation = (KHAL_DEMOD_TPS_CONSTELLATION_T)trans_ConstelCode_Translate(pMyInfo->dvbt2_ofdm.constellation, false);
	}
	//Others unknown now
	//	BOOLEAN 							bSpectrumInv;		/* 1 bit */
	//	BOOLEAN 							bProfileHP; 	/* 1 bit */
	KHAL_DEMOD_INFO("bSpectrumInv:%d constellation:%d\n", pSpecDVBT->bSpectrumInv, pSpecDVBT->constellation);
	up(&rtkdemod_Semaphore);
	return API_OK;

DvbtGetSpecialRet:
	up(&rtkdemod_Semaphore);
	return API_NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT_CheckSpecialData);

int KHAL_DEMOD_DVB_DVBC_CheckSpecialData(KHAL_DEMOD_SPECDATA_DVBC_T *pSpecDVBC)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBC_CheckSpecialData @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSpecDVBC == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK)
		goto DvbcGetSpecialRet;

	pSpecDVBC->bSpectrumInv = pMyInfo->dvbc_qam.spectrum_inverse;
	pSpecDVBC->constellation = (KHAL_DEMOD_TPS_CONSTELLATION_T)trans_ConstelCode_Translate(pMyInfo->dvbc_qam.constellation, false);
	pSpecDVBC->symbolRate = (int)pMyInfo->dvbc_qam.symbol_rate / 1000;

	up(&rtkdemod_Semaphore);
	return API_OK;
DvbcGetSpecialRet:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBC_CheckSpecialData);

int KHAL_DEMOD_DVB_CheckFrequencyOffset(SINT32 *pFreqOffset)
{
	S32BITS offset;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_CheckFrequencyOffset @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_CheckFrequencyOffset);

int KHAL_DEMOD_DVB_GetCellID(UINT16 *pCellID)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetCellID @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK)
		goto DvbtGetCellIDFail;

	*pCellID = pMyInfo->dvbt2_ofdm.cell_ID;
	up(&rtkdemod_Semaphore);
	return API_OK;

DvbtGetCellIDFail:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_GetCellID);

int KHAL_DEMOD_DVB_ControlTSMode(BOOLEAN bIsSerial)
{

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_ControlTSMode @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_ControlTSMode);

int KHAL_DEMOD_DVB_DebugMenu(void)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DebugMenu @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DebugMenu);

int KHAL_DEMOD_DVB_ControlOutput(BOOLEAN bEnableOutput)
{

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_ControlOutput @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_ControlOutput);

int KHAL_DEMOD_SetI2C(void *i2c_cli)
{
	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	return 0;
}
EXPORT_SYMBOL(KHAL_DEMOD_SetI2C);

int KHAL_DEMOD_DVB_GetTsClkRate(UINT32 *pTsClkRate)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetTsClkRate @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pTsClkRate == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);
	if (pDemod_globe->m_pDemod->GetTsClkRate(pDemod_globe->m_pDemod, pTsClkRate) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	KHAL_DEMOD_INFO("TsClkRate=%u KHz\n", *pTsClkRate);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_GetTsClkRate);

/***New function for K3L***/
int KHAL_DEMOD_DVB_DVBT2_ChangePLP(UINT8 unPLP)
{

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT2_ChangePLP @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	//if (unPLP == 0)
	//	return API_INVALID_PARAMS;

	KHAL_DEMOD_INFO("change PLP=%d\n", unPLP);

	pMyInfo->dvbt2_ofdm.plp_select = unPLP;

	if (pDemod_globe->m_pDemod->ChangeT2PLP(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK)
		goto SetDvbt2PLPRet;

	return API_OK;
SetDvbt2PLPRet:
	return API_NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT2_ChangePLP);

int KHAL_DEMOD_DVB_DVBT2_GetMultiPLPInfo(KHAL_MULTI_TS_INFO_T *pPlpInfo)
{

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT2_GetMultiPLPInfo @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	if (pPlpInfo == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);
	if (pDemod_globe->m_pDemod->GetT2PLPInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	pPlpInfo->PLPCount = pMyInfo->dvbt2_ofdm.plp_cnt;
	memcpy(pPlpInfo->paPLPID, &pMyInfo->dvbt2_ofdm.plp, sizeof(pMyInfo->dvbt2_ofdm.plp));


	KHAL_DEMOD_INFO("PLPCount=%d\n", pPlpInfo->PLPCount);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT2_GetMultiPLPInfo);

int KHAL_DEMOD_DVB_DVBT2_CheckSpecialData(KHAL_DEMOD_SPECDATA_DVBT2_T *pSpecDVBT2)
{

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT2_CheckSpecialData @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	if (pSpecDVBT2 == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}
	pSpecDVBT2->carrierMode = (KHAL_DEMOD_TPS_CARRIER_MODE_T)trans_CarrierMode_Translate(pMyInfo->dvbt2_ofdm.fft_mode, false);
	pSpecDVBT2->guardInterval = (KHAL_DEMOD_TPS_GUARD_INTERVAL_T)trans_GuardCode_Translate(pMyInfo->dvbt2_ofdm.guard_interval, false);
	pSpecDVBT2->codeRate = (KHAL_DEMOD_TPS_CODERATE_T)trans_CodeRate_Translate(pMyInfo->dvbt2_ofdm.code_rate_hp, false);
	pSpecDVBT2->constellation = (KHAL_DEMOD_TPS_CONSTELLATION_T)trans_ConstelCode_Translate(pMyInfo->dvbt2_ofdm.constellation, false);
	pSpecDVBT2->unPLP = pMyInfo->dvbt2_ofdm.plp_select;
	pSpecDVBT2->bSpectrumInv = (BOOLEAN)pMyInfo->dvbt2_ofdm.bSpectrumInv;

	KHAL_DEMOD_INFO("bSpectrumInv:%d constellation:%d, PLP_ID = %u\n", pSpecDVBT2->bSpectrumInv, pSpecDVBT2->constellation, pSpecDVBT2->unPLP);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT2_CheckSpecialData);

int KHAL_DEMOD_DVB_DVBT2_GetSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState)
{
	TV_SIG_QUAL Quality;
	UINT8 bValue = 0;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT2_GetSignalState @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT2_GetSignalState);

int KHAL_DEMOD_DVB_OperMode(KHAL_DEMOD_TRANS_SYSTEM_T *pOperMode)
{
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_OperMode @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	if (pOperMode == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);


	if (pDemod_globe->m_pDemod->GetSignalInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK) {
		up(&rtkdemod_Semaphore);
		return API_NOT_OK;
	}

	switch (pMyInfo->mod) {
	default:
		*pOperMode = KHAL_DEMOD_TRANS_SYS_UNKNOWN;
		break;
	case TV_MODULATION_OFDM:
		*pOperMode = KHAL_DEMOD_TRANS_SYS_DVBT;
		break;
	case TV_MODULATION_DVBT2_OFDM:
		*pOperMode = KHAL_DEMOD_TRANS_SYS_DVBT2;
		break;
	case TV_MODULATION_DVBC_QAM:
		*pOperMode = KHAL_DEMOD_TRANS_SYS_DVBC;
		break;
	case TV_MODULATION_DVBS:
		*pOperMode = KHAL_DEMOD_TRANS_SYS_DVBS;
		break;
	case TV_MODULATION_DVBS2:
		*pOperMode = KHAL_DEMOD_TRANS_SYS_DVBS2;
		break;
	}

	KHAL_DEMOD_INFO("current operMode: %d \n", *pOperMode);
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_OperMode);

int KHAL_DEMOD_DVB_DVBT2_SetDemod(KHAL_DEMOD_DVBT2_SET_PARAM_T paramStruct)
{
	TV_SYSTEM sys;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBT2_SetDemod @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	if ((paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_DVBT) && (paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_DVBT2)) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_DVB_DVBT_SetDemod but the standard is not DVBT!!!! standard=%d\n" "\033[m", paramStruct.transSystem);
		goto SetDvbtDemodRet;
	}

	KHAL_DEMOD_INFO("tuneMode:%d transSystem:%d, eChannelBW:%d, bSpectrumInv:%d, carrierMode:%d, guardInterval:%d, codeRate:%d, constellation:%d\n",
					paramStruct.tuneMode, paramStruct.transSystem, paramStruct.eChannelBW, paramStruct.bSpectrumInv, paramStruct.carrierMode, paramStruct.guardInterval, paramStruct.codeRate, paramStruct.constellation);

	SigInfo.mod = TV_MODULATION_DVBT2_OFDM;
	SigInfo.dvbt2_ofdm.scan_mode = ((paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN) || (paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN_START)) ? 1 : 0;
	SigInfo.dvbt2_ofdm.mod = TV_MODULATION_DVBT2_OFDM;
	//SigInfo.dvbt2_ofdm.constellation = trans_ConstelCode_Translate(paramStruct.constellation, true);
	//SigInfo.dvbt2_ofdm.guard_interval = trans_Hierarchy_Translate(paramStruct.guardInterval, true);
	//SigInfo.dvbt2_ofdm.fft_mode = trans_Hierarchy_Translate(paramStruct.carrierMode, true);
	//SigInfo.dvbt2_ofdm.code_rate_hp = trans_Hierarchy_Translate(paramStruct.codeRate, true);
	//SigInfo.dvbt2_ofdm.code_rate_lp = trans_Hierarchy_Translate(paramStruct.codeRate, true);
	SigInfo.dvbt2_ofdm.plp_select = paramStruct.unPLP;
	SigInfo.dvbt2_ofdm.bSpectrumInv = paramStruct.bSpectrumInv;


	switch (paramStruct.eChannelBW) {

	case KHAL_DEMOD_CH_BW_6M:
		sys = TV_SYS_DVBT2_6M;
		break;
	case KHAL_DEMOD_CH_BW_7M:
		sys = TV_SYS_DVBT2_7M;
		break;
	case KHAL_DEMOD_CH_BW_8M:
		sys = TV_SYS_DVBT2_8M;
		break;

	default:
		sys = TV_SYS_UNKNOWN;
	}

	if (sys == TV_SYS_UNKNOWN) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_DVB_DVBT2_SetDemod but the BW is out fo range!!!! BW=%d\n" "\033[m", paramStruct.eChannelBW);
		goto  SetDvbtDemodRet;
	}


	memcpy(&tDvbt2Param, &paramStruct, sizeof(KHAL_DEMOD_DVBT2_SET_PARAM_T));
	if (pDemod_globe->m_pDemod->SetTVSysEx(pDemod_globe->m_pDemod, sys, &SigInfo) != TUNER_CTRL_OK)
		goto SetDvbtDemodRet;

	//pDemod_globe->set_tv_sys_ex_data.sys = sys;
	//memcpy(&pDemod_globe->set_tv_sys_ex_data.pSigInfo, &SigInfo, sizeof(TV_SIG_INFO));

	up(&rtkdemod_Semaphore);
	return API_OK;
SetDvbtDemodRet:
	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBT2_SetDemod);


int KHAL_DEMOD_DVB_DVBS_SetDemod(KHAL_DEMOD_DVBS_SET_PARAM_T paramStruct)
{
	TV_SYSTEM sys;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_SetDemod @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);

	if ((paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_DVBS) && (paramStruct.transSystem != KHAL_DEMOD_TRANS_SYS_DVBS2)) {
		KHAL_DEMOD_WARNING("\033[1;32;32m""KHAL_DEMOD_DVB_DVBS_SetDemod but the standard is not DVBS!!!! standard=%d\n" "\033[m", paramStruct.transSystem);
		KHAL_DEMOD_INFO("KHAL_DEMOD_DVB_DVBS_SetDemod but the standard is not DVBS!!!! standard=%d\n", paramStruct.transSystem);
		goto SetDvbsDemodRet;
	}

	paramStruct.symbolRate = paramStruct.symbolRate * 1000;
	KHAL_DEMOD_INFO("tuneMode:%d, transSystem:%d, bSpectrumInv:%d,  symbolRate:%d, codeRate:%d, constellation:%d, freqKHz:%d, bIsBlind_search:%d \n",
					paramStruct.tuneMode,    paramStruct.transSystem,    paramStruct.bSpectrumInv,    paramStruct.symbolRate,    paramStruct.codeRate,    paramStruct.constellation,    paramStruct.freqKHz,    paramStruct.bIsBlind_search);

	SigInfo.mod = TV_MODULATION_OFDM;
	SigInfo.dvbs.scan_mode = ((paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN) || (paramStruct.tuneMode == KHAL_DEMOD_TUNE_SCAN_START)) ? 1 : 0;
	switch (paramStruct.transSystem) {
	default:
	case KHAL_DEMOD_TRANS_SYS_DVBS:
		SigInfo.mod = TV_MODULATION_DVBS;
		SigInfo.dvbs.mod = TV_MODULATION_DVBS;
		SigInfo.dvbs.freq_khz = paramStruct.freqKHz;
		SigInfo.dvbs.symbol_rate = paramStruct.symbolRate;
		SigInfo.dvbs.IsBlindsearch = paramStruct.bIsBlind_search;
		break;
	case KHAL_DEMOD_TRANS_SYS_DVBS2:
		SigInfo.mod = TV_MODULATION_DVBS2;
		SigInfo.dvbs.mod = TV_MODULATION_DVBS2;
		SigInfo.dvbs.freq_khz = paramStruct.freqKHz;
		SigInfo.dvbs.symbol_rate = paramStruct.symbolRate;
		SigInfo.dvbs.IsBlindsearch = paramStruct.bIsBlind_search;
		break;
	}

	sys = trans_TV_SYSTEM(paramStruct.transSystem, KHAL_DEMOD_CH_BW_8M);
	memcpy(&tDvbsParam, &paramStruct, sizeof(KHAL_DEMOD_DVBS_SET_PARAM_T));
	if (pDemod_globe->m_pDemod->SetTVSysEx(pDemod_globe->m_pDemod, sys, &SigInfo) != TUNER_CTRL_OK)
		goto SetDvbsDemodRet;

	//pDemod_globe->set_tv_sys_ex_data.sys = sys;
	//memcpy(&pDemod_globe->set_tv_sys_ex_data.pSigInfo, &SigInfo, sizeof(TV_SIG_INFO));
	up(&rtkdemod_Semaphore);

	return API_OK;

SetDvbsDemodRet:
	KHAL_DEMOD_WARNING(" %s (%d)--------------------FAIL \n", __func__, __LINE__);
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_SetDemod);

int KHAL_DEMOD_DVB_DVBS_TunePostJob(BOOLEAN *pFinished)
{
	unsigned char WaitSignalLock = 1;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_TunePostJob @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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

	KHAL_DEMOD_INFO("DVBS_TunePostJob ret = %d, Finished = %d\n", ret, *pFinished);

	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_TunePostJob);

int KHAL_DEMOD_DVB_DVBS2_TunePostJob(BOOLEAN *pFinished)
{
	unsigned char WaitSignalLock = 1;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS2_TunePostJob @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	if (pFinished == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	ret = pDemod_globe->m_pDemod->AcquireSignal(pDemod_globe->m_pDemod, WaitSignalLock);
	*pFinished = TRUE;

	KHAL_DEMOD_INFO("DVBS2_TunePostJob ret = %d, Finished = %d\n", ret, *pFinished);

	up(&rtkdemod_Semaphore);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS2_TunePostJob);

int KHAL_DEMOD_DVB_DVBS_CheckLock(KHAL_DEMOD_LOCK_STATE_T *pLockState)
{
	DTV_STATUS ret = API_NOT_OK;
	unsigned char bSignalLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_CheckLock @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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

	KHAL_DEMOD_INFO("DVBS LOCK status (0:unlock, 1:lock):---------(%d)\n", bSignalLock);
	up(&rtkdemod_Semaphore);
	return ret;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_CheckLock);

int KHAL_DEMOD_DVB_DVBS_GetSignalState(KHAL_DEMOD_SIGNAL_STATE_T *pSignalState)
{
	TV_SIG_QUAL Quality;
	UINT8 bValue = 0;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_GetSignalState @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSignalState == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetLockStatus(pDemod_globe->m_pDemod, &bValue) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;

	pSignalState->bSignalValid = bValue ? TRUE : FALSE;

	//strength
	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_SIGNAL_STRENGTH, &Quality) != TUNER_CTRL_OK) {
		goto Demod_GetSignalStateRet;
	}
	pSignalState->strength = Quality.strength;
	//quality
	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_SIGNAL_QUALITY, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->quality = Quality.quality;
	//unAGC
	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_AGC, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->unAGC = Quality.agc;
	//unSNR
	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_SNR, &Quality) != TUNER_CTRL_OK)
		goto Demod_GetSignalStateRet;
	pSignalState->unSNR = Quality.snr / 10000;
	//unBER
	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_BEC, &Quality) != TUNER_CTRL_OK) {
		goto Demod_GetSignalStateRet;
	}
	pSignalState->unBER = Quality.layer_ber.ber_num;
	//packetError
	if (pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_PEC, &Quality) != TUNER_CTRL_OK) {
		goto Demod_GetSignalStateRet;
	}
	pSignalState->packetError = Quality.layer_per.per_num;

	KHAL_DEMOD_INFO("SNR: %d, BER:(%d), SQI:(%d), AGC:(%d) , packageError:(%d),  SigValid:(%d) \n", pSignalState->unSNR, pSignalState->unBER,
					pSignalState->quality,  pSignalState->unAGC, pSignalState->packetError , pSignalState->bSignalValid);

	up(&rtkdemod_Semaphore);
	return API_OK;
Demod_GetSignalStateRet:
	up(&rtkdemod_Semaphore);
	return NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_GetSignalState);

int KHAL_DEMOD_DVB_DVBS_CheckSpecialData(KHAL_DEMOD_SPECDATA_DVBS_T *pSpecDVBS)
{
	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	if (pSpecDVBS == NULL)
		return API_INVALID_PARAMS;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_CheckSpecialData @@@\n");

	down(&rtkdemod_Semaphore);

	if (pDemod_globe->m_pDemod->GetSignalInfo(pDemod_globe->m_pDemod, pMyInfo) != TUNER_CTRL_OK)
		goto DvbsGetSpecialRet;

	if (pMyInfo->dvbs.mod == TV_MODULATION_DVBS2) {
		pSpecDVBS->bIsDVBS2 = TRUE;
	} else {
		pSpecDVBS->bIsDVBS2 = FALSE;
	}
	pSpecDVBS->bSpectrumInv = (BOOLEAN)pMyInfo->dvbs.spec_invert;
	pSpecDVBS->symbolRate = (UINT32)pMyInfo->dvbs.symbol_rate;
	pSpecDVBS->codeRate = (KHAL_DEMOD_TPS_CODERATE_T)DVBS_trans_codeRate_Translate(pMyInfo->dvbs.code_rate, false);
	pSpecDVBS->constellation = (KHAL_DEMOD_TPS_CONSTELLATION_T)DVBS_trans_constellation_Translate(pMyInfo->dvbs.modulation, false);

	KHAL_DEMOD_INFO("dvbs SpecialData : isDVBS2:%d, bSpectrumInv:%d, symbolRate:%d, codeRate:%d, constellation:%d \n",
					pSpecDVBS->bIsDVBS2, pSpecDVBS->bSpectrumInv, pSpecDVBS->symbolRate, pSpecDVBS->codeRate, pSpecDVBS->constellation);
	up(&rtkdemod_Semaphore);
	return API_OK;

DvbsGetSpecialRet:
	up(&rtkdemod_Semaphore);
	return API_NOT_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_CheckSpecialData);

int KHAL_DEMOD_DVB_PKT_ERR(UINT32 *pPKT_Err_Acu, BOOLEAN bReset)
{
	TV_SIG_QUAL Quality;
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_PKT_ERR @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	if (pPKT_Err_Acu == NULL)
		return API_INVALID_PARAMS;

	down(&rtkdemod_Semaphore);

	if (bReset)
		ret = pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_CUSTOMER_LG, &Quality);
	else
		ret = pDemod_globe->m_pDemod->GetSignalQuality(pDemod_globe->m_pDemod, TV_QUAL_PEC, &Quality);

	up(&rtkdemod_Semaphore);

	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	*pPKT_Err_Acu = (UINT32)Quality.layer_per.per_num;
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_PKT_ERR);

int KHAL_DEMOD_DVB_DVBS_22Khz_Tone(BOOLEAN b22kOn)
{
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_22Khz_Tone @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	ret = pDemod_globe->m_pDemod->SetDiseqcContinue22kOnOff(pDemod_globe->m_pDemod, b22kOn);
	//ret = pDemod_globe->m_pDemod->SetDiseqcContinue22kOnOff(b22kOn);
	up(&rtkdemod_Semaphore);

	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_22Khz_Tone);

int KHAL_DEMOD_DVB_DVBS_Send_Diseqc(UINT8* pCmd, UINT8 u8CmdSize)
{
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_Send_Diseqc @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);


	ret = pDemod_globe->m_pDemod->SetDiseqcCmd(pDemod_globe->m_pDemod, u8CmdSize, pCmd);
	up(&rtkdemod_Semaphore);

	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_Send_Diseqc);

int KHAL_DEMOD_DVB_DVBS_Send_Tone_Burst(BOOLEAN bTone1)
{
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_Send_Tone_Burst @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);

	ret = pDemod_globe->m_pDemod->SetDiseqcToneBurst(pDemod_globe->m_pDemod, bTone1);
	up(&rtkdemod_Semaphore);


	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_Send_Tone_Burst);

int KHAL_DEMOD_DVB_DVBS_BlindScan_Init(UINT32 StartFreqMhz, UINT32 EndFreqMhz)
{
	int ret;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_BlindScan_Init @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	KHAL_DEMOD_INFO("StartFreqMhz %u, EndFreqMhz %u\n", StartFreqMhz, EndFreqMhz);
	down(&rtkdemod_Semaphore);
	ret = pDemod_globe->m_pDemod->BlindScan_Init(pDemod_globe->m_pDemod, StartFreqMhz, EndFreqMhz);
	up(&rtkdemod_Semaphore);

	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_BlindScan_Init);

int KHAL_DEMOD_DVB_DVBS_BlindScan_GetTunerFreq(UINT32 *TunerCenterFreqMhz, UINT32 *TunerCutOffFreqKhz)
{
	int ret;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_BlindScan_GetTunerFreq @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);
	ret = pDemod_globe->m_pDemod->BlindScan_GetTunerFreq(pDemod_globe->m_pDemod, TunerCenterFreqMhz, TunerCutOffFreqKhz);
	up(&rtkdemod_Semaphore);

	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	KHAL_DEMOD_INFO("TunerCenterFreqMhz %u, TunerCutOffFreqKhz %u\n", *TunerCenterFreqMhz, *TunerCutOffFreqKhz);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_BlindScan_GetTunerFreq);

int KHAL_DEMOD_DVB_DVBS_BlindScan_NextFreq(BOOLEAN *bBlindScanEnd)
{
	int ret;
	unsigned int BlindScanEnd = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_BlindScan_NextFreq @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);
	ret = pDemod_globe->m_pDemod->BlindScan_NextFreq(pDemod_globe->m_pDemod, &BlindScanEnd);
	up(&rtkdemod_Semaphore);

	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	*bBlindScanEnd = BlindScanEnd;
	KHAL_DEMOD_INFO("bBlindScanEnd %u\n", *bBlindScanEnd);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_BlindScan_NextFreq);

int KHAL_DEMOD_DVB_DVBS_BlindScan_GetStatus(UINT8 *Status, BOOLEAN *bBlindScanLock)
{
	int ret;
	unsigned int BlindScanLock = 0;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_BlindScan_GetStatus @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	ret = pDemod_globe->m_pDemod->BlindScan_GetStatus(pDemod_globe->m_pDemod, Status, &BlindScanLock);
	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	*bBlindScanLock = BlindScanLock;
	KHAL_DEMOD_INFO("Status %u, bBlindScanLock %u\n", *Status, *bBlindScanLock);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_BlindScan_GetStatus);

int KHAL_DEMOD_DVB_DVBS_BlindScan_End(void)
{
	int ret;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_BlindScan_End @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}
	
	down(&rtkdemod_Semaphore);
	ret = pDemod_globe->m_pDemod->BlindScan_End(pDemod_globe->m_pDemod);
	up(&rtkdemod_Semaphore);

	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_BlindScan_End);

int KHAL_DEMOD_DVB_DVBS_BlindScan_GetFoundTP(UINT32 *FreqMhz, UINT32 *SymbolRateKhz)
{
	int ret;
	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_BlindScan_GetFoundTP @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
		return NOT_OK;
	}

	down(&rtkdemod_Semaphore);
	ret = pDemod_globe->m_pDemod->BlindScan_GetFoundTP(pDemod_globe->m_pDemod, FreqMhz, SymbolRateKhz);
	up(&rtkdemod_Semaphore);
	if (ret != TUNER_CTRL_OK)
		return NOT_OK;

	KHAL_DEMOD_INFO("FreqMhz %u, SymbolRateKhz %u\n", *FreqMhz, *SymbolRateKhz);
	return API_OK;
}
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_BlindScan_GetFoundTP);

int KHAL_DEMOD_DVB_GetFWVersion(UINT32 *pFWVersion)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetFWVersion @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_GetFWVersion);

int KHAL_DEMOD_DVB_GetSQI(UINT8 *pSQI)
{

	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetSQI @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_GetSQI);

int KHAL_DEMOD_DVB_DVBS_GetSQI(UINT8 *pSQI)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_GetSQI @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_GetSQI);


int KHAL_DEMOD_DVB_GetPacketError(UINT32 *pPacketError)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetPacketError @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_GetPacketError);

int KHAL_DEMOD_DVB_DVBS_GetPacketError(UINT32 *pPacketError)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_GetPacketError @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_GetPacketError);

int KHAL_DEMOD_DVB_GetBER(UINT32 *pBER)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetPacketError @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_GetBER);

int KHAL_DEMOD_DVB_DVBS_GetBER(UINT32 *pBER)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_GetBER @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_GetBER);

int KHAL_DEMOD_DVB_GetAGC(UINT32 *pAGC)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetPacketError @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_GetAGC);

int KHAL_DEMOD_DVB_DVBS_GetAGC(UINT32 *pAGC)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_GetAGC @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_GetAGC);

int KHAL_DEMOD_DVB_GetSNR(UINT32 *pSNR)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_GetPacketError @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_GetSNR);

int KHAL_DEMOD_DVB_DVBS_GetSNR(UINT32 *pSNR)
{
	TV_SIG_QUAL Quality;

	KHAL_DEMOD_CALLER("@@@ KHAL_DEMOD_DVB_DVBS_GetSNR @@@\n");

	if (isDemodConstructors == 0) {
		KHAL_DEMOD_WARNING("Need to Call KHAL_DEMOD_DVB_Initialize Firstly\n");
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
EXPORT_SYMBOL(KHAL_DEMOD_DVB_DVBS_GetSNR);

