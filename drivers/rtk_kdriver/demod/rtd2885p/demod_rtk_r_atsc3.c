/***************************************************************************************************
  File        : demod_rtk_dvbt2.cpp
  Description : REALTEK_I DVB-T/T2 demod
  Author      : Kevin Wang
****************************************************************************************************
    Update List :
----------------------------------------------------------------------------------------------------
    20120207    | create phase
***************************************************************************************************/
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <rtk_kdriver/io.h>
#include "foundation.h"
#include "demod_rtk_r_atsc3.h"
#include "atsc3_demod_rtk_r.h"
#include "demod_rtk_r_atsc3_priv.h"
#include "rbus/dtv_demod_misc_reg.h"

//#include "rbus/demod_sd_reg.h"
//#include "rbus/dtv_frontend_reg.h"

//#define SUPPORT_DEMOD_AUTOTEST


static unsigned long acquire_sig_stime;
unsigned char Atsc3Vv526Flag = 0, Atsc3KvvCrossSymbFlag = 0;
unsigned int atsc3L1CRCErrCnt = 0, atsc3DataErrCnt = 0;
unsigned char Atsc3mSubFrameFlag = 0xf;
unsigned char Atsc3PlayModePreLockFlag = 0;
unsigned char Atsc32plpHtiTiExtInterFlag = 0xf;
//unsigned int Atsc3CeDagcScaleErrCnt = 0;

#define ATSC3_6M_ONLY

//#define ATSC3_SYS_CLK_496P8M //for  VerA

#ifdef ATSC3_SYS_CLK_496P8M
const unsigned short Atsc3LowPassFilter[31] = {0xffa, 0xfff, 0x007, 0x00a, 0x003, 0xff5, 0xfed, 0xff7, 0x00f, 0x020, 0x015, 0xff0, 0xfcf, 0xfd6, 0x00b, 0x045, 0x04a, 0x005, 0xfa6, 0xf84, 0xfd7, 0x06f, 0x0c9, 0x074, 0xf81, 0xea5, 0xed4, 0x08b, 0x355, 0x5ee, 0x6fc};
#endif


TV_ATSC3_PLP_ID Atsc3PlpAll, Atsc3PlpLlsOnly;

//#define REALTEK_R_EXT_QFN

/*----------------------------------------------------------------------
 * Func : REALTEK_R_ATSC3
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
void REALTEK_R_ATSC3_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm)
{
	REALTEK_R_ATSC3* pRealtekRATSC3;

	pDemodKernel->private_data = kmalloc(sizeof(REALTEK_R_ATSC3), GFP_KERNEL);
	if (pDemodKernel->private_data == NULL) {
		goto kmalloc_fail;
	}

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);

	Demod_Constructors(pDemodKernel);

	pDemodKernel->m_id                 = DTV_DEMOD_REALTEK_R_ATSC3;
	pDemodKernel->m_addr               = addr;
	pDemodKernel->m_pTuner             = NULL;
	pDemodKernel->m_pComm              = pComm;
	pDemodKernel->m_ScanRange          = SCAN_RANGE_3_6;
	pDemodKernel->m_Capability         = TV_SYS_TYPE_ATSC3P0;
	pDemodKernel->m_update_interval_ms = 300;
	pDemodKernel->m_if.freq            = 5000000;
	pDemodKernel->m_if.inversion       = 0;
	pDemodKernel->m_if.agc_pola        = 0;

#ifdef ATSC3_SYS_CLK_496P8M
	pDemodKernel->m_clock              = CRYSTAL_FREQ_23000000HZ;
#else
	pDemodKernel->m_clock              = CRYSTAL_FREQ_24000000HZ;
#endif

	pRealtekRATSC3->m_output_freq        = output_freq;

	pRealtekRATSC3->m_private            = (void*) AllocRealtekRAtsc3Driver(pDemodKernel->m_pComm, pDemodKernel->m_addr, pDemodKernel->m_clock);

	pDemodKernel->m_ts.mode            = (output_mode == RTK_DEMOD_OUT_IF_PARALLEL) ? PARALLEL_MODE : SERIAL_MODE;
	pDemodKernel->m_ts.data_order      = MSB_D7;
	pDemodKernel->m_ts.datapin         = MSB_FIRST;
	pDemodKernel->m_ts.err_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.sync_pol        = ACTIVE_HIGH;
	pDemodKernel->m_ts.val_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.clk_pol         = FALLING_EDGE;
	pDemodKernel->m_ts.internal_demod_input = 1;
	pDemodKernel->m_ts.tsOutEn = 0;
	pDemodKernel->m_ScanMode           = CH_UNKNOW;
	pRealtekRATSC3->m_TunerOptRegCnt = 0;
	pRealtekRATSC3->m_acquire_sig_en = 0;
	pRealtekRATSC3->m_pTunerOptReg = NULL;

	pRealtekRATSC3->m_AP_mPlpSelectedCnt = 1;
	pRealtekRATSC3->m_AP_mPlpSelectedNo[0] = 0;

	pRealtekRATSC3->m_unlock_cnt = 0;
	pRealtekRATSC3->m_auto_tune_en = 0;


	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "REALTEK_R_ATSC3");

	pDemodKernel->Init = REALTEK_R_ATSC3_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = REALTEK_R_ATSC3_AcquireSignal;
	pDemodKernel->ScanSignal = REALTEK_R_ATSC3_ScanSignal;
	pDemodKernel->SetIF = REALTEK_R_ATSC3_SetIF;
	pDemodKernel->SetTSMode = REALTEK_R_ATSC3_SetTSMode;
	pDemodKernel->SetTVSysEx = REALTEK_R_ATSC3_SetTVSysEx;
	pDemodKernel->SetMode = REALTEK_R_ATSC3_SetMode;
	pDemodKernel->SetTVSys = REALTEK_R_ATSC3_SetTVSys;
	pDemodKernel->GetLockStatus = REALTEK_R_ATSC3_GetLockStatus;
	pDemodKernel->GetSegmentNum = Demod_GetSegmentNum;
	pDemodKernel->GetSignalInfo = REALTEK_R_ATSC3_GetSignalInfo;
	pDemodKernel->GetSignalQuality = REALTEK_R_ATSC3_GetSignalQuality;
	pDemodKernel->GetAtsc3PLPInfo = REALTEK_R_ATSC3_GetPLPInfo;
	pDemodKernel->ChangeAtsc3PLP = REALTEK_R_ATSC3_ChangePLP;
	pDemodKernel->GetCarrierOffset = REALTEK_R_ATSC3_GetCarrierOffset;
	pDemodKernel->GetDebugLogBuf = REALTEK_R_ATSC3_GetDebugLogBuf;
	pDemodKernel->ForwardI2C = REALTEK_R_ATSC3_ForwardI2C;
	pDemodKernel->AutoTune = REALTEK_R_ATSC3_AutoTune;
	pDemodKernel->KernelAutoTune = REALTEK_R_ATSC3_KernelAutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = REALTEK_R_ATSC3_Suspend;
	pDemodKernel->Resume = REALTEK_R_ATSC3_Resume;
	pDemodKernel->SetTvMode = REALTEK_R_ATSC3_SetTvMode;
	pDemodKernel->InspectSignal = REALTEK_R_ATSC3_InspectSignal;    // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
	//pDemodKernel->Lock = Demod_Lock;
	//pDemodKernel->Unlock = Demod_Unlock;
	pDemodKernel->Lock = REALTEK_R_ATSC3_Lock;
	pDemodKernel->Unlock = REALTEK_R_ATSC3_Unlock;

	//pDemodKernel->Activate = REALTEK_R_ATSC3_Activate;
	pDemodKernel->Destory = REALTEK_R_ATSC3_Destructors;

kmalloc_fail:
	REALTEK_R_ATSC3_WARNING("REALTEK_R_ATSC3_Constructors kmalloc Failed\n");
}


/*----------------------------------------------------------------------
 * Func : ~REALTEK_R_ATSC3
 *
 * Desc : Destructor of REALTEK_R_ATSC3
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void REALTEK_R_ATSC3_Destructors(DEMOD* pDemodKernel)
{
	REALTEK_R_ATSC3* pRealtekRATSC3;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);

	if (pRealtekRATSC3->m_private != NULL)
		ReleaseRealtekRAtsc3Driver((REALTEK_R_ATSC3_DRIVER_DATA*)pRealtekRATSC3->m_private);

	kfree(pRealtekRATSC3);
	//pDemodKernel->Destory(pDemodKernel);
	REALTEK_R_ATSC3_INFO("REALTEK_R_ATSC3_Destructors Complete\n");
}


/*----------------------------------------------------------------------
 * Func : Init
 *
 * Desc : Initial realtekI
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_ATSC3_Init(DEMOD* pDemodKernel)
{
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;


	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);


	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC3->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	//////////////////////////////////////////////////
	//ATSC3_DEMOD_MODULE* pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	//////////////////////////////////////////////////

	pRealtekRATSC3->m_auto_tune_en = 0;
	pRealtekRATSC3->m_unlock_cnt = 0;


	pDemodKernel->Lock(pDemodKernel);

	pDemodKernel->m_ts.tsOutEn = 0;

	ExtAtsc3DemodInitialization(RTK_DEMOD_MODE_ATSC3, 1);

	REALTEK_R_ATSC3_System_Init(pDemodKernel, 1);

	rtd_4bytes_i2c_mask(pDemod, 0xB816F004, 0xBFFFFFFF, 0x40000000);   //atb_wrapper_reset_flow_en
	rtd_4bytes_i2c_wr(pDemod, 0xB816F068, 0x00000010);  //DTV_DEMOD_SEL: atsc3_en

	REALTEK_R_ATSC3_BisrCheck(pDemodKernel);

	pDemodKernel->Unlock(pDemodKernel);


	// Initial Driver
	if (pDemod->Initialize(pDemod) != FUNCTION_SUCCESS)
		goto init_fail;


	if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
		goto init_fail;


	if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK) {
		goto init_fail;
	}

	REALTEK_R_ATSC3_DBG("REALTEK_ATB_ATSC3_Init Complete\n");


	return TUNER_CTRL_OK;

init_fail:
	REALTEK_R_ATSC3_WARNING("REALTEK_ATB_ATSC3_Init Fail\n");
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
int REALTEK_R_ATSC3_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	int ret;
	REALTEK_R_ATSC3* pRealtekRATSC3;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);

	REALTEK_R_ATSC3_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);
	ret = pDemodKernel->SetTVSysEx(pDemodKernel, sys, NULL);
	return ret;
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
int REALTEK_R_ATSC3_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo)
{
	int ret = TUNER_CTRL_OK;
	TUNER_PARAM Param;
	TV_ATSC3_PLP_ID Atsc3Plp;
	unsigned char SpectrumInvert = 0;
	unsigned int U32CntTmp = 0;
	unsigned char ldpcItrMode = 0,  ldpcIterNum = 50, ldpcLatencyCtrlEn = 0, ldpcLatencyVal = 0;

#ifdef ATSC3_SYS_CLK_496P8M
	unsigned char u8Tmp = 0;
	unsigned int AddrTmp = 0, DataTmp = 0;
#endif


	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;


	if (IsAtsc3p0Sys(sys) == 0) {

		REALTEK_R_ATSC3_WARNING("Set TV system failed, unsupported TV system\n");
		return TUNER_CTRL_FAIL;
	}

	/*
		if (!pSigInfo) {

			REALTEK_R_ATSC3_WARNING("Siginfo doesn't specified, SetTVSysEx Fail\n");
			return TUNER_CTRL_FAIL;
		}
	*/

	REALTEK_R_ATSC3_TPOutEn(pDemodKernel, 0);
	pRealtekRATSC3->m_auto_tune_en = 0;
	pDemodKernel->m_tv_sys = sys;

	if (pSigInfo == NULL)
		pDemodKernel->m_ScanMode = CH_SCAN_MODE;
	else
		pDemodKernel->m_ScanMode = pSigInfo->atsc3.scan_mode ? CH_SCAN_MODE : CH_NORMAL_MODE;


	//TUNER_PARAM Param;
	if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, sys, &Param) == TUNER_CTRL_OK) {
		if ((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola  != Param.if_agc_pola)) {
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;

			if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)  return  TUNER_CTRL_FAIL;
		}
	}


	if (pDemodKernel->m_ScanMode == CH_SCAN_MODE) {
		//Atsc3Plp.PLPCnt = 0;
		Atsc3Plp.PLPCnt = 1;
		Atsc3Plp.PLPId[0] = 0;
	} else {
		Atsc3Plp.PLPCnt = pSigInfo->atsc3.plp_cnt;
		pRealtekRATSC3->m_AP_mPlpSelectedCnt =  pSigInfo->atsc3.plp_cnt;
		for (U32CntTmp = 0; U32CntTmp < pSigInfo->atsc3.plp_cnt; U32CntTmp++) {
			Atsc3Plp.PLPId[U32CntTmp] = pSigInfo->atsc3.plp[U32CntTmp];
			pRealtekRATSC3->m_AP_mPlpSelectedNo[U32CntTmp] = pSigInfo->atsc3.plp[U32CntTmp];
		}
	}


#ifndef ATSC3_6M_ONLY
	switch (sys) {
	case TV_SYS_ATSC3P0_6M:
	default:
		pRealtekRATSC3->m_current_bw = RTK_R_ATSC3_BANDWIDTH_6MHZ;
		break;
	case TV_SYS_ATSC3P0_7M:
		pRealtekRATSC3->m_current_bw = RTK_R_ATSC3_BANDWIDTH_7MHZ;
		break;
	case TV_SYS_ATSC3P0_8M:
		pRealtekRATSC3->m_current_bw = RTK_R_ATSC3_BANDWIDTH_8MHZ;
		break;

	}
#else
	pRealtekRATSC3->m_current_bw = RTK_R_ATSC3_BANDWIDTH_6MHZ;
#endif



	REALTEK_R_ATSC3_INFO("System = %d, BW = %uKHz, scan mode = %d, bSpectrumInv = %u, plp_selected_cnt = %u, PLP_ID = [%u, %u, %u, %u]\n",
						 sys, pRealtekRATSC3->m_current_bw, pDemodKernel->m_ScanMode, SpectrumInvert, Atsc3Plp.PLPCnt, Atsc3Plp.PLPId[0], Atsc3Plp.PLPId[1], Atsc3Plp.PLPId[2], Atsc3Plp.PLPId[3]);

	pDemodKernel->Lock(pDemodKernel);

#ifdef ATSC3_SYS_CLK_496P8M
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8166430, 0x1DF2AAAB);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8166524, 0x1A9ED098);

	//load bpf
	for (u8Tmp = 0; u8Tmp < 31; u8Tmp++) {
		DataTmp = (unsigned int) Atsc3LowPassFilter[u8Tmp];
		AddrTmp = 0xb8162824 + (8 * u8Tmp);

		ret |= rtd_4bytes_i2c_wr(pDemod, AddrTmp, DataTmp & 0xff);
		ret |= rtd_4bytes_i2c_wr(pDemod, AddrTmp + 4, (DataTmp >> 8) & 0xf);
	}
#endif

#ifndef ATSC3_6M_ONLY
	ret |= DECODE_RET(pDemod->SetBandwidthMode(pDemod, pRealtekRATSC3->m_current_bw));
#endif

	ret |= DECODE_RET(pDemod->SetPlpID(pDemod, Atsc3Plp));

	realtek_r_atsc3_restore_default_setting(pDemod);
	realtek_r_atsc3_SetLdpcIterNum(pDemod, ldpcItrMode, ldpcIterNum);
	realtek_r_atsc3_SetLdpcLatency(pDemod, ldpcLatencyCtrlEn, ldpcLatencyVal);
	realtek_r_atsc3_LdpcAlphaFixModeEn(pDemod, 0);

	ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));

	pDemodKernel->Unlock(pDemodKernel);

	pRealtekRATSC3->m_status_checking_stime = tv_osal_time() + ATSC3_STATUS_CHECK_INTERVAL_MS;
	pRealtekRATSC3->m_autotune_VVPatchCheckTime = tv_osal_time() + AUTOTUNE_CHECK_VV_PATCH_INTERVAL_MS;
	pRealtekRATSC3->m_autotune_L1CrcCheckTime = tv_osal_time() + AUTOTUNE_CHECK_L1Crc_INTERVAL_MS;
	pRealtekRATSC3->m_status_LGchecking_stime = tv_osal_time();
	pRealtekRATSC3->m_acquire_sig_en = 1;
	pRealtekRATSC3->m_unlock_cnt = 0;
	pRealtekRATSC3->m_auto_tune_en = 1;

	Atsc3Vv526Flag = 0;
	Atsc3KvvCrossSymbFlag = 0;
	atsc3L1CRCErrCnt = 0;
	atsc3DataErrCnt = 0;
	Atsc3PlayModePreLockFlag = 0;
	Atsc32plpHtiTiExtInterFlag = 0xf;
	Atsc3mSubFrameFlag = 0xf;

	acquire_sig_stime = tv_osal_time();

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
int REALTEK_R_ATSC3_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	//REALTEK_R_ATSC3_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);
	pDemodKernel->Lock(pDemodKernel);

	ret |= DECODE_RET(pDemod->SetIfFreqHz(pDemod, (unsigned int) pParam->freq));
	ret |= DECODE_RET(pDemod->SetAgcPol(pDemod, pParam->agc_pola));


	REALTEK_R_ATSC3_DBG("SetIF, IF_freq = "PT_U32BITS", spectrumn inv = %u, agc_pola = %u\n", pParam->freq, pParam->inversion, pParam->agc_pola);
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
int REALTEK_R_ATSC3_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	REALTEK_R_ATSC3* pRealtekRATSC3;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);

	//REALTEK_R_ATSC3_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);

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
int REALTEK_R_ATSC3_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;

	RTK_R_ATSC3_MPEGTSMode stTSMode;

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	stTSMode.ui8TSTransferType = (pParam->mode) ? RTK_R_ATSC3_TS_SERIAL_MODE : RTK_R_ATSC3_TS_PARALLEL_MODE;
	stTSMode.ui8OutputEdge = (pParam->clk_pol) ? RTK_R_ATSC3_TS_OUTPUT_RISING_EDGE : RTK_R_ATSC3_TS_OUTPUT_FALLING_EDGE;
	stTSMode.ui8TSSPIMSBSelection = (pParam->data_order) ? RTK_R_ATSC3_TS_SPI_MSB_ON_DATA_BIT0 : RTK_R_ATSC3_TS_SPI_MSB_ON_DATA_BIT7;
	stTSMode.ui8TSSSIOutputSelection = RTK_R_ATSC3_TS_SSI_OUTPUT_ON_DATA_BIT0;
	stTSMode.ui8SPIClockConstantOutput = RTK_R_ATSC3_TS_CLOCK_CONST_OUTPUT;
	stTSMode.ui8TSClockVariableMode  = RTK_R_ATSC3_TS_CLOCK_VARIABLE;


	ret |= DECODE_RET(pDemod->SetTsInterfaceMode(pDemod, stTSMode));

	REALTEK_R_ATSC3_TPOutEn(pDemodKernel, pParam->tsOutEn);


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
int REALTEK_R_ATSC3_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{
	int ret = TUNER_CTRL_OK;
	int LockStatus = 0;
	int SyncLockFlag = 0, L1LockFlag = 0;
	//unsigned char ATSC3config2MplpCtiFlag = 0xf;

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*)pRealtekRATSC3->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	ret |= DECODE_RET(pDemod->IsSignalLocked(pDemod, &LockStatus));

	/*
		if (LockStatus == 0) {
			realtek_r_atsc3_mplp_cti_detect(pDemod, &ATSC3config2MplpCtiFlag);
			if (ATSC3config2MplpCtiFlag == 1)
				LockStatus = 1;
		}
	*/


	if (pDemodKernel->m_ScanMode == CH_NORMAL_MODE) {

		if (Atsc3PlayModePreLockFlag == 1) {
			if (LockStatus == 1) {
				Atsc3PlayModePreLockFlag = 0;
			} else {

				ret |= DECODE_RET(pDemod->IsSignalSyncLocked(pDemod, &SyncLockFlag));                //DVBT Sync locked
				ret |= DECODE_RET(pDemod->IsL1Locked(pDemod, &L1LockFlag));

				if ((SyncLockFlag == 1) && (L1LockFlag == 1))
					LockStatus = 1;
			}
		}
	}


	if (ret == TUNER_CTRL_OK) {
		*pLock = LockStatus ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK;
	} else {
		*pLock = DTV_SIGNAL_NOT_LOCK;
	}

	//REALTEK_R_ATSC3_DBG("[lock status] *pLock = %u\n", *pLock);

	pDemodKernel->Unlock(pDemodKernel);

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
int REALTEK_R_ATSC3_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	int ret = TUNER_CTRL_OK;

	RTK_R_ATSC3_PARAMS Atsc3Params;
	unsigned int u32CntTmp = 0;

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*)pRealtekRATSC3->m_private)->pDemod;

	pInfo->mod = TV_MODULATION_ATSC3;

	pDemodKernel->Lock(pDemodKernel);

	ret |= DECODE_RET(pDemod->GetSignalParams(pDemod, &Atsc3Params));

	pDemodKernel->Unlock(pDemodKernel);


	pInfo->atsc3.plp_cnt = Atsc3Params.PLPSelectedCnt;

	switch (pRealtekRATSC3->m_current_bw) {
	case RTK_R_ATSC3_BANDWIDTH_6MHZ:
	default:
		pInfo->atsc3.bandwidth = 6000000;
		break;
	case RTK_R_ATSC3_BANDWIDTH_7MHZ:
		pInfo->atsc3.bandwidth = 7000000;
		break;
	case RTK_R_ATSC3_BANDWIDTH_8MHZ:
		pInfo->atsc3.bandwidth = 8000000;
		break;
	}



	for (u32CntTmp = 0; u32CntTmp < Atsc3Params.PLPSelectedCnt; u32CntTmp++) {
		pInfo->atsc3.plp[u32CntTmp] = Atsc3Params.PLP[u32CntTmp];
		pInfo->atsc3.constellation[u32CntTmp] = (unsigned char)Atsc3Params.Constellation[u32CntTmp];
		pInfo->atsc3.fft_mode[u32CntTmp] = (unsigned char)Atsc3Params.FFTMode[u32CntTmp];
		pInfo->atsc3.guard_interval[u32CntTmp] = (unsigned char)Atsc3Params.GuardInterval[u32CntTmp];
		pInfo->atsc3.code_rate[u32CntTmp] = (unsigned char)Atsc3Params.CodeRate[u32CntTmp];

	}

	pInfo->atsc3.bSpectrumInv = Atsc3Params.bSpectrumInv;

	REALTEK_R_ATSC3_INFO("plp_cnt = %u, plp = [%u/%u/%u/%u], FFT = [%u/%u/%u/%u], GUARD_INT = [%u/%u/%u/%u]\n",
						 pInfo->atsc3.plp_cnt, pInfo->atsc3.plp[0], pInfo->atsc3.plp[1], pInfo->atsc3.plp[2], pInfo->atsc3.plp[3],  pInfo->atsc3.fft_mode[0], pInfo->atsc3.fft_mode[1], pInfo->atsc3.fft_mode[2], pInfo->atsc3.fft_mode[3],
						 pInfo->atsc3.guard_interval[0], pInfo->atsc3.guard_interval[1], pInfo->atsc3.guard_interval[2], pInfo->atsc3.guard_interval[3]);

	REALTEK_R_ATSC3_INFO("CODE_RATE = [%u/%u/%u/%u], CONSTELLATION = [%u/%u/%u/%u], SPEC_INV = %u, bandwidth = %uHz\n",
						 pInfo->atsc3.code_rate[0], pInfo->atsc3.code_rate[1], pInfo->atsc3.code_rate[2], pInfo->atsc3.code_rate[3],
						 pInfo->atsc3.constellation[0], pInfo->atsc3.constellation[1], pInfo->atsc3.constellation[2], pInfo->atsc3.constellation[3], pInfo->atsc3.bSpectrumInv, pInfo->atsc3.bandwidth);


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
int REALTEK_R_ATSC3_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality)
{
	int ret = TUNER_CTRL_OK;
	unsigned int Snr, PakN, BadPakN, BitN, BadBitN;
	unsigned int Agc;
	unsigned int Sqi, Ssi;
	int ConterTmp = 0;
	unsigned int u32Idx = 0;
	int TxRSSIdBm;
	unsigned long QualityStime = 0;

	TV_ATSC3_ErrRate BitErr, PacketErr;

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	//if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_ATSC3))
	//return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);

	switch (id) {
	case TV_QUAL_SNR:
		ret |= DECODE_RET(pDemod->GetSnrDb(pDemod, &Snr));
		pQuality->snr = Snr * 1000; //(Snr/10)*10000
		break;
	case TV_QUAL_BER:
	case TV_QUAL_BEC:
		ret |= DECODE_RET(pDemod->GetBer(pDemod, &BitErr));

		u32Idx = 0;

		for (ConterTmp = 0; ConterTmp < 4; ConterTmp++) {

			if (BitErr.PLPvalid[ConterTmp] == 1) {

				if ((BitErr.ErrDen[ConterTmp] == 0) | (BitErr.ErrNum[ConterTmp] != 0))
					u32Idx = ConterTmp;

			} else {
				break;
			}


		}

		BadBitN = BitErr.ErrNum[u32Idx];
		BitN = BitErr.ErrDen[u32Idx];


		if (BitN > 0) {
			pQuality->layer_ber.ber_num = BadBitN;
			pQuality->layer_ber.ber_den = BitN;
			pQuality->ber = (BadBitN * 10000) / BitN;
		} else {
			pQuality->layer_ber.ber_num = 0xffffffff;
			pQuality->layer_ber.ber_den = 0xffffffff;
			pQuality->ber = 0xffffffff;
		}

		break;
	case TV_QUAL_PER:
	case TV_QUAL_PEC:
	case TV_QUAL_UCBLOCKS:
		ret |= DECODE_RET(pDemod->GetPer(pDemod, &PacketErr));

		u32Idx = 0;

		for (ConterTmp = 0; ConterTmp < 4; ConterTmp++) {

			if (PacketErr.PLPvalid[ConterTmp] == 1) {

				if ((PacketErr.ErrDen[ConterTmp] == 0) | (PacketErr.ErrNum[ConterTmp] != 0))
					u32Idx = ConterTmp;

			} else {
				break;
			}


		}

		BadPakN = PacketErr.ErrNum[u32Idx];
		PakN = PacketErr.ErrDen[u32Idx];


		if (PakN > 0) {
			pQuality->layer_per.per_num = BadPakN;
			pQuality->layer_per.per_den = PakN;
			pQuality->per = (BadPakN * 10000) / PakN;
		} else {
			pQuality->layer_per.per_num = 0xffffffff;
			pQuality->layer_per.per_den = 0xffffffff;
			pQuality->per = 0xffffffff;
		}

		pQuality->ucblocks = BadPakN;
		break;
	case TV_QUAL_SIGNAL_STRENGTH:
		TxRSSIdBm = (int) pQuality-> rflevel;
		ret |= DECODE_RET(pDemod->GetSignalStrength(pDemod, TxRSSIdBm, &Ssi));
		pQuality->strength = (unsigned char) Ssi;
		break;
	case TV_QUAL_SIGNAL_QUALITY:
		ret |= DECODE_RET(pDemod->GetLayerSignalQuality(pDemod, &Sqi));
		pQuality->quality = (unsigned char) Sqi;
		break;
	case TV_QUAL_AGC:
		ret |= DECODE_RET(pDemod->GetIfAgc(pDemod, &Agc));
		pQuality->agc = (U32BITS)Agc;
		break;
	case TV_QUAL_DEMODFWVERSION:
		pQuality->DemodFwVersion = 0x000001;//V1.0.9
		break;
	case TV_QUAL_CUSTOMER_LG:

		QualityStime = tv_osal_time();
		if ((QualityStime > pRealtekRATSC3->m_status_LGchecking_stime) || ((pRealtekRATSC3->m_status_LGchecking_stime - QualityStime) > ATSC3_STATUS_LGCHECK_INTERVAL_MS)) {

			TxRSSIdBm = (int) pQuality-> rflevel;
			ret |= DECODE_RET(pDemod->GetSnrDb(pDemod, &Snr));
			ret |= DECODE_RET(pDemod->GetBer(pDemod, &BitErr));
			ret |= DECODE_RET(pDemod->GetPer(pDemod, &PacketErr));
			ret |= DECODE_RET(pDemod->GetSignalStrength(pDemod, TxRSSIdBm, &Ssi));
			ret |= DECODE_RET(pDemod->GetLayerSignalQuality(pDemod, &Sqi));
			pQuality->strength = (unsigned char) Ssi;
			pQuality->quality = (unsigned char) Sqi;

			pQuality->snr = Snr * 1000; //(Snr/10)*10000

			u32Idx = 0;

			for (ConterTmp = 0; ConterTmp < 4; ConterTmp++) {

				if (PacketErr.PLPvalid[ConterTmp] == 1) {
					if ((PacketErr.ErrDen[ConterTmp] == 0) | (PacketErr.ErrNum[ConterTmp] != 0))
						u32Idx = ConterTmp;
				} else {
					break;
				}
			}

			BadBitN = BitErr.ErrNum[u32Idx];
			BitN = BitErr.ErrDen[u32Idx];
			BadPakN = PacketErr.ErrNum[u32Idx];
			PakN = PacketErr.ErrDen[u32Idx];


			if (PakN > 0) {
				pQuality->layer_per.per_num = BadPakN;
				pQuality->layer_per.per_den = PakN;
				pQuality->layer_ber.ber_num = BadBitN;
				pQuality->layer_ber.ber_den = BitN;
			} else {
				pQuality->layer_per.per_num = 0x0;    //0xffffffff;
				pQuality->layer_per.per_den = 0xffffffff;
				pQuality->layer_ber.ber_num = 0x0;    //0xffffffff;
				pQuality->layer_ber.ber_den = 0xffffffff;
			}


			ret |= DECODE_RET(pDemod->GetIfAgc(pDemod, &Agc));
			pQuality->agc = (U32BITS)Agc;



			pRealtekRATSC3->mQuality.strength = pQuality->strength;
			pRealtekRATSC3->mQuality.quality = pQuality->quality;
			pRealtekRATSC3->mQuality.snr = pQuality->snr;

			pRealtekRATSC3->mQuality.layer_per.per_num = pQuality->layer_per.per_num;
			pRealtekRATSC3->mQuality.layer_per.per_den = pQuality->layer_per.per_den;
			pRealtekRATSC3->mQuality.layer_ber.ber_num = pQuality->layer_ber.ber_num;
			pRealtekRATSC3->mQuality.layer_ber.ber_den = pQuality->layer_ber.ber_den;
			pRealtekRATSC3->mQuality.agc = pQuality->agc;

			pRealtekRATSC3->m_status_LGchecking_stime = tv_osal_time() + ATSC3_STATUS_LGCHECK_INTERVAL_MS;

			REALTEK_R_ATSC3_INFO("\033[1;32;31m" "SNR = "PT_U32BITS", AGC = "PT_U32BITS", pkt_error = "PT_U32BITS", total_pkt = "PT_U32BITS"\n" "\033[m",
								 pQuality->snr, pQuality->agc, pQuality->layer_per.per_num, pQuality->layer_per.per_den);

			REALTEK_R_ATSC3_INFO("SSI = %u, SQI = %u\n", pQuality->strength, pQuality->quality);


			REALTEK_R_ATSC3_INFO("PLP_VLD= [%u/%u/%u/%u], ErrNum = [%u/%u/%u/%u], ErrDen = [%u/%u/%u/%u] \n", PacketErr.PLPvalid[0], PacketErr.PLPvalid[1], PacketErr.PLPvalid[2], PacketErr.PLPvalid[3],
								 PacketErr.ErrNum[0], PacketErr.ErrNum[1], PacketErr.ErrNum[2], PacketErr.ErrNum[3], PacketErr.ErrDen[0], PacketErr.ErrDen[1], PacketErr.ErrDen[2], PacketErr.ErrDen[3]);


		}  else {

			pQuality->strength = pRealtekRATSC3->mQuality.strength;
			pQuality->quality = pRealtekRATSC3->mQuality.quality;
			pQuality->snr = pRealtekRATSC3->mQuality.snr;

			pQuality->layer_per.per_num = pRealtekRATSC3->mQuality.layer_per.per_num;
			pQuality->layer_per.per_den = pRealtekRATSC3->mQuality.layer_per.per_den;
			pQuality->layer_ber.ber_num = pRealtekRATSC3->mQuality.layer_ber.ber_num;
			pQuality->layer_ber.ber_den = pRealtekRATSC3->mQuality.layer_ber.ber_den;
			pQuality->agc = pRealtekRATSC3->mQuality.agc;
		}

		break;
	default:
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
int REALTEK_R_ATSC3_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	int ret = TUNER_CTRL_OK;
	int Cfo = 0;

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	//if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_ATSC3))
	//return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);

	ret = DECODE_RET(pDemod->GetCrOffsetHz(pDemod, &Cfo));
	//*pOffset = (S32BITS)((-1) * Cfo);
	*pOffset = (S32BITS) Cfo;

	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_R_ATSC3_INFO("GetCarrierOffset = "PT_S32BITS"Hz\n", *pOffset);

	return ret;
}
/*----------------------------------------------------------------------
 * Func : GetDebugLogBuf
 *
 * Desc : Get the debug log Buf.
 *
 * Parm :
 *
 *
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_ATSC3_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize)
{
	int ret = TUNER_CTRL_OK;

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
int REALTEK_R_ATSC3_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned long cur_time;
#endif

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;


#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	cur_time = tv_osal_time();

	while ((tv_osal_time() - cur_time) <= 15) {

		if (pRealtekRATSC3->m_acquire_sig_en == 0)
			return TUNER_CTRL_OK;

		tv_osal_msleep(5);
	}

	return TUNER_CTRL_FAIL;

#else
	REALTEK_R_ATSC3_AcquireSignalThread(pDemodKernel);
	pRealtekRATSC3->m_autotune_stime = tv_osal_time() + ATSC3_AUTODETECT_SIG_UPDATE_INTERVAL_MS;
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
int REALTEK_R_ATSC3_AcquireSignalThread(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	unsigned long det_stime = 0;
	int SyncLockFlag = 0, FecLockFlag = 0, lock = 0, L1LockFlag = 0;
	unsigned int atsc3_timeout = 3000, NoSignalTimeOut = 400, atsc3_prelock_timeout = 1000;
	unsigned char ATSC3config2MplpCtiFlag = 0xf;
	unsigned char Atsc3_1PlpElOnly_multisub = 0xf;

	unsigned char Atsc3_L1BD_latency_modify_flag = 0xf;

	TV_ATSC3_PLP_ID Atsc3Plp;
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;



	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	if (pRealtekRATSC3->m_acquire_sig_en == 0)
		return TUNER_CTRL_OK;

	REALTEK_R_ATSC3_DBG("\033[1;32;32m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);
	pDemodKernel->Lock(pDemodKernel);


	det_stime = tv_osal_time();
	lock = NO;

	while ((tv_osal_time() - acquire_sig_stime) <= atsc3_timeout) {

		ret |= DECODE_RET(pDemod->IsSignalLocked(pDemod, &FecLockFlag));                    //DVBT FEC locked
		ret |= DECODE_RET(pDemod->IsSignalSyncLocked(pDemod, &SyncLockFlag));                //DVBT Sync locked
		ret |= DECODE_RET(pDemod->IsL1Locked(pDemod, &L1LockFlag));

		if (ATSC3config2MplpCtiFlag == 0xf) {
			realtek_r_atsc3_mplp_cti_detect(pDemod, &ATSC3config2MplpCtiFlag);
			if (ATSC3config2MplpCtiFlag == 0x1) {
				Atsc3Plp.PLPCnt = 1;
				Atsc3Plp.PLPId[0] = pRealtekRATSC3->m_AP_mPlpSelectedNo[0];
				ret |= DECODE_RET(pDemod->SetPlpID(pDemod, Atsc3Plp));
				ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));
				atsc3_timeout = 4000;
				REALTEK_R_ATSC3_INFO("[mPLP CTI case] set PLP ID = %u\n", Atsc3Plp.PLPId[0]);
				//lock = YES;
				//break;
			}
		}

		if (Atsc3_L1BD_latency_modify_flag == 0xf) {
			realtek_r_atsc3_L1BD_latency_modify(pDemod, &Atsc3_L1BD_latency_modify_flag);
			if (Atsc3_L1BD_latency_modify_flag == 0x1)
				atsc3_timeout = 4000;
		}

		if (Atsc3Vv526Flag == 0)
			realtek_r_atsc3_vv526_detect_flow(pDemod, &Atsc3Vv526Flag);

		if (Atsc3KvvCrossSymbFlag == 0)
			realtek_r_atsc3_kvv_fdm_check_flow(pDemod, &Atsc3KvvCrossSymbFlag);

		if (Atsc3_1PlpElOnly_multisub == 0xf)
			realtek_r_atsc3_config1PlpElOnly_multisubframe(pDemod, &Atsc3_1PlpElOnly_multisub);

		if (Atsc32plpHtiTiExtInterFlag == 0xf)
			realtek_r_atsc3_2plp_hti_TiExtInter_detect(pDemod, &Atsc32plpHtiTiExtInterFlag, &Atsc3PlpAll, &Atsc3PlpLlsOnly);


		if (Atsc3mSubFrameFlag == 0xf) {
			realtek_r_atsc3_multisubframe_fft8K_ce_fix(pDemod, &Atsc3mSubFrameFlag);
			if (Atsc3mSubFrameFlag == 0x1) {
				ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));
				atsc3_prelock_timeout = 1500;
				atsc3_timeout = 4000;
				REALTEK_R_ATSC3_INFO("Atsc3mSubFrameFlag = 1! demod reset~~\n");
			}
		}

		realtek_r_atsc3_1PLPLdpcOnlyDetect(pDemod);

		if (FecLockFlag == 1) {
			realtek_r_atsc3_LdpcParamCheck(pDemod);
			realtek_r_atsc3_L1DUpdateItemCheck(pDemod);
			lock = YES;
			break;
		}

		if ((SyncLockFlag == 0) && ((tv_osal_time() - acquire_sig_stime) > NoSignalTimeOut)) {
			lock = NO;
			break;
		}


		if ((SyncLockFlag == 1) && (L1LockFlag == 0) && ((tv_osal_time() - acquire_sig_stime) > atsc3_prelock_timeout)) {
			lock = NO;
			break;
		}

		if ((SyncLockFlag == 1) && (L1LockFlag == 1) && ((tv_osal_time() - acquire_sig_stime) > 700) && (pDemodKernel->m_ScanMode == CH_NORMAL_MODE)) {
			lock = YES;
			Atsc3PlayModePreLockFlag = 1;
			break;
		}

		tv_osal_msleep(5);
	}

	REALTEK_R_ATSC3_INFO("\033[1;32;32m" "Signal Lock=%u (PlayModePre=%u), 2MsubframeFlag=%u, API execution Time=%lu, total_acquire_time=%lu\n" "\033[m",
						 lock, Atsc3PlayModePreLockFlag, ATSC3config2MplpCtiFlag, tv_osal_time() - det_stime, tv_osal_time() - acquire_sig_stime);


	pDemodKernel->Unlock(pDemodKernel);
	pRealtekRATSC3->m_acquire_sig_en = 0;
	pRealtekRATSC3->m_status_checking_stime = tv_osal_time() + ATSC3_STATUS_CHECK_INTERVAL_MS;

	REALTEK_R_ATSC3_TPOutEn(pDemodKernel, 1);

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
int REALTEK_R_ATSC3_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range)
{
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	//REALTEK_R_ATSC3_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);

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
int REALTEK_R_ATSC3_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	//REALTEK_R_ATSC3_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);


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
int REALTEK_R_ATSC3_AutoTune(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;

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
int REALTEK_R_ATSC3_KernelAutoTune(DEMOD* pDemodKernel)
{
	unsigned long cur_time;
	unsigned char LockStatus = 0;
	unsigned char L1CrcErrStatus = 0;
	unsigned char ldpcItrMode = 0,  ldpcIterNum = 50, ldpcLatencyCtrlEn = 0, ldpcLatencyVal = 0;
	unsigned char Atsc3_L1BD_latency_modify_flag = 0xf;
	unsigned int BsdFsm = 0;


	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;



#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	TV_SIG_QUAL Quality = {0};
#endif


	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;


	if (pRealtekRATSC3->m_auto_tune_en == 0)
		return TUNER_CTRL_OK;


	cur_time = tv_osal_time();


#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	if ((cur_time > pRealtekRATSC3->m_status_checking_stime) || ((pRealtekRATSC3->m_status_checking_stime - cur_time) > ATSC3_STATUS_CHECK_INTERVAL_MS)) {
		REALTEK_R_ATSC3_GetLockStatus(pDemodKernel, &LockStatus);
		REALTEK_R_ATSC3_INFO("[KernelAutoTune] Lock = %d\n", LockStatus);
		if (LockStatus == 1)
			REALTEK_R_ATSC3_GetSignalQuality(pDemodKernel, TV_QUAL_CUSTOMER_LG, &Quality);
		pRealtekRATSC3->m_status_checking_stime = tv_osal_time() + ATSC3_STATUS_CHECK_INTERVAL_MS;

	}
#endif


	if (pRealtekRATSC3->m_acquire_sig_en) {

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
		REALTEK_R_ATSC3_AcquireSignalThread(pDemodKernel);
		pRealtekRATSC3->m_autotune_stime = tv_osal_time() + ATSC3_AUTODETECT_SIG_UPDATE_INTERVAL_MS;
#endif

	} else {

		REALTEK_R_ATSC3_GetLockStatus(pDemodKernel, &LockStatus);

		if (LockStatus == 0) {

			if ((cur_time > pRealtekRATSC3->m_autotune_VVPatchCheckTime) || ((pRealtekRATSC3->m_autotune_VVPatchCheckTime - cur_time) > AUTOTUNE_CHECK_VV_PATCH_INTERVAL_MS)) {

				if ((Atsc3Vv526Flag != 0) || (Atsc3KvvCrossSymbFlag != 0) || (Atsc3mSubFrameFlag == 0x1) || (Atsc32plpHtiTiExtInterFlag == 0x1)) {
					realtek_r_atsc3_restore_default_setting(pDemod);
					pDemod->SoftwareReset(pDemod);
					Atsc3Vv526Flag = 0;
					Atsc3KvvCrossSymbFlag = 0;
					Atsc3mSubFrameFlag = 0xf;
				}

				realtek_r_atsc3_SetLdpcIterNum(pDemod, ldpcItrMode, ldpcIterNum);
				realtek_r_atsc3_SetLdpcLatency(pDemod, ldpcLatencyCtrlEn, ldpcLatencyVal);
				realtek_r_atsc3_L1DUpdateItemCheck(pDemod);
				realtek_r_atsc3_1PLPLdpcOnlyDetect(pDemod);
				realtek_r_atsc3_L1BD_latency_modify(pDemod, &Atsc3_L1BD_latency_modify_flag);

				rtd_4bytes_i2c_rd(pDemod, 0xb8166450, &BsdFsm);
				BsdFsm &= 0x7;

				if (BsdFsm == 1) {
					pDemod->SoftwareReset(pDemod);
					REALTEK_R_ATSC3_INFO("[KernelAutoTune] SoftwareReset by BSD FSM= 1\n");
				}


				pRealtekRATSC3->m_autotune_VVPatchCheckTime = tv_osal_time() + AUTOTUNE_CHECK_VV_PATCH_INTERVAL_MS;
			}


			if (Atsc32plpHtiTiExtInterFlag == 1) {
				rtd_4bytes_i2c_mask(pDemod, 0xB816EE34, 0x7FFFFFFF, 0x00000000);  //restore default vaule for realtek_r_atsc3_2plp_hti_TiExtInter_detect  (dhti_mode)
				rtd_4bytes_i2c_mask(pDemod, 0xB8161C04, 0xFFFFFFEF, 0x00000000);  //restore default vaule for realtek_r_atsc3_2plp_hti_TiExtInter_detect (Bicm_plp_table_SW_mode)
				pDemod->SoftwareReset(pDemod);
			}
			Atsc32plpHtiTiExtInterFlag = 0xf;


			if (Atsc3Vv526Flag == 0)
				realtek_r_atsc3_vv526_detect_flow(pDemod, &Atsc3Vv526Flag);

			if (Atsc3KvvCrossSymbFlag == 0)
				realtek_r_atsc3_kvv_fdm_check_flow(pDemod, &Atsc3KvvCrossSymbFlag);

			if ((cur_time > pRealtekRATSC3->m_autotune_L1CrcCheckTime) || ((pRealtekRATSC3->m_autotune_L1CrcCheckTime - cur_time) > AUTOTUNE_CHECK_L1Crc_INTERVAL_MS)) {
				realtek_r_atsc3_multisubframe_fft8K_ce_fix(pDemod, &Atsc3mSubFrameFlag);
				pRealtekRATSC3->m_autotune_L1CrcCheckTime = tv_osal_time() + AUTOTUNE_CHECK_L1Crc_INTERVAL_MS;
			}


			atsc3L1CRCErrCnt = 0;
			atsc3DataErrCnt = 0;

		} else {

			if ((cur_time > pRealtekRATSC3->m_autotune_L1CrcCheckTime) || ((pRealtekRATSC3->m_autotune_L1CrcCheckTime - cur_time) > AUTOTUNE_CHECK_L1Crc_INTERVAL_MS)) {

				realtek_r_atsc3_L1CRCErr_check(pDemod, &L1CrcErrStatus);

				if ((L1CrcErrStatus & 0x3) == 0x3)
					atsc3L1CRCErrCnt ++;
				else
					atsc3L1CRCErrCnt = 0;

				if ((L1CrcErrStatus & 0x4) == 0x4)
					atsc3DataErrCnt ++;
				else
					atsc3DataErrCnt = 0;


				/*
								if ((L1CrcErrStatus & 0x8) == 0x8)
									Atsc3CeDagcScaleErrCnt ++;
								else
									Atsc3CeDagcScaleErrCnt = 0;
				*/


				//if ((atsc3L1CRCErrCnt > 2) || (Atsc3CeDagcScaleErrCnt > 2))  {
				if ((atsc3L1CRCErrCnt > 2) || (atsc3DataErrCnt > 6))  {
					pDemodKernel->Lock(pDemodKernel);

					realtek_r_atsc3_restore_default_setting(pDemod);
					pDemod->SoftwareReset(pDemod);
					REALTEK_R_ATSC3_INFO("\033[1;32;32m" "SoftWare Reset by L1 or Data error !!!  atsc3L1CRCErrCnt = %u, atsc3DataErrCnt = %u\n" "\033[m", atsc3L1CRCErrCnt, atsc3DataErrCnt);
					//REALTEK_R_ATSC3_INFO("\033[1;32;32m" "SoftWare Reset by L1B & L1D pkt error !!!  \n" "\033[m");

					pRealtekRATSC3->m_status_checking_stime = tv_osal_time() + ATSC3_STATUS_CHECK_INTERVAL_MS;
					pRealtekRATSC3->m_autotune_VVPatchCheckTime = tv_osal_time() + AUTOTUNE_CHECK_VV_PATCH_INTERVAL_MS;
					//pRealtekRATSC3->m_autotune_L1CrcCheckTime = tv_osal_time() + AUTOTUNE_CHECK_L1Crc_INTERVAL_MS;
					pRealtekRATSC3->m_unlock_cnt = 0;
					Atsc3Vv526Flag = 0;
					Atsc3KvvCrossSymbFlag = 0;
					atsc3L1CRCErrCnt = 0;
					atsc3DataErrCnt = 0;
					//Atsc3CeDagcScaleErrCnt = 0;
					pDemodKernel->Unlock(pDemodKernel);
				}


				if (Atsc32plpHtiTiExtInterFlag == 0xf)
					realtek_r_atsc3_2plp_hti_TiExtInter_detect(pDemod, &Atsc32plpHtiTiExtInterFlag, &Atsc3PlpAll, &Atsc3PlpLlsOnly);

				realtek_r_atsc3_LdpcParamCheck(pDemod);
				realtek_r_atsc3_L1DUpdateItemCheck(pDemod);
				realtek_r_atsc3_1PLPLdpcOnlyDetect(pDemod);
				realtek_r_atsc3_multisubframe_fft8K_ce_fix(pDemod, &Atsc3mSubFrameFlag);

				realtek_r_atsc3_L1BD_latency_modify(pDemod, &Atsc3_L1BD_latency_modify_flag);


				pRealtekRATSC3->m_autotune_L1CrcCheckTime = tv_osal_time() + AUTOTUNE_CHECK_L1Crc_INTERVAL_MS;
			}


		}


	}
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
int REALTEK_R_ATSC3_InspectSignal(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	return ret;
}


/*----------------------------------------------------------------------
 * Func : Activate
 *
 * Desc : Activate RTK_DEMOD_ATSC3 Demod
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_ATSC3_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetTvMode
 *
 * Desc : Set Mode of RTK_DEMOD_ATSC3
 *
 * Parm : mode :    RTK_DEMOD_MODE_ATSC3 : DVBT2 mode
 *                  RTK_DEMOD_MODE_DVBC  : DVBC mode
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_ATSC3_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;

	REALTEK_R_ATSC3* pRealtekRATSC3;
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);

	pDemodKernel->Lock(pDemodKernel);
	REALTEK_R_ATSC3_INFO("REALTEK_R_ATSC3::SetTvMode(%d)\n", mode);

	switch (mode) {
	case TV_SYS_TYPE_ATSC3P0:
		REALTEK_R_ATSC3_INFO("Set to ATSC3\n");
		ret = pDemodKernel->Init(pDemodKernel);
		if (ret != TUNER_CTRL_OK) {
			REALTEK_R_ATSC3_WARNING("Set ATSC3 moce failed\n");
			goto set_demod_mode_failed;
		}
		break;

	default:
		goto set_demod_mode_failed;
	}

	ret = TUNER_CTRL_OK;
	REALTEK_R_ATSC3_INFO("Set REALTEK_R_ATSC3 to appointed DTV mode successed\n");

	pDemodKernel->Unlock(pDemodKernel);
	return ret;

set_demod_mode_failed:
	pDemodKernel->Unlock(pDemodKernel);

	return TUNER_CTRL_FAIL;
}


int REALTEK_R_ATSC3_Suspend(DEMOD * pDemodKernel)
{
	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);


	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC3->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;


	//ret = REALTEK_R_ATSC3_PowerDown(pDemodKernel);


	return ret;
}


int REALTEK_R_ATSC3_Resume(DEMOD * pDemodKernel)
{
	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);


	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC3->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	//ret = REALTEK_R_ATSC3_Init(pDemodKernel);


	return ret;
}



void REALTEK_R_ATSC3_Lock(DEMOD* pDemodKernel)
{
	//mutex_lock(&pDemodKernel->m_lock);
}

void REALTEK_R_ATSC3_Unlock(DEMOD* pDemodKernel)
{
	//mutex_unlock(&pDemodKernel->m_lock);
}


/*-----------------------------------------------------------------
 *     BASE_INTERFACE for REALTEK_R_ATSC3 API
 *----------------------------------------------------------------*/
void ReleaseRealtekRAtsc3Driver(REALTEK_R_ATSC3_DRIVER_DATA *pDriver)
{
	kfree(pDriver);
}


REALTEK_R_ATSC3_DRIVER_DATA* AllocRealtekRAtsc3Driver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
)
{
	REALTEK_R_ATSC3_DRIVER_DATA* pDriver = (REALTEK_R_ATSC3_DRIVER_DATA*) kmalloc(
			sizeof(REALTEK_R_ATSC3_DRIVER_DATA) +
			sizeof(ATSC3_DEMOD_MODULE)   +
			sizeof(I2C_BRIDGE_MODULE)   +
			sizeof(BASE_INTERFACE_MODULE), GFP_KERNEL);

	if (pDriver) {
		BASE_INTERFACE_MODULE* pBIF;
		ATSC3_DEMOD_MODULE* pDemod;
		memset(pDriver, 0, sizeof(REALTEK_R_ATSC3_DRIVER_DATA));

		pDriver->pDemod                = (ATSC3_DEMOD_MODULE*)(((unsigned char*)pDriver) + sizeof(REALTEK_R_ATSC3_DRIVER_DATA));
		pDriver->pBaseInterface        = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(ATSC3_DEMOD_MODULE));
		pDriver->pI2CBridge            = (I2C_BRIDGE_MODULE*)(((unsigned char*)pDriver->pBaseInterface) + sizeof(BASE_INTERFACE_MODULE));
		pDriver->DeviceAddr            = Addr;
		pDriver->CrystalFreqHz         = CrystalFreq;

		// Init Base IF
		pBIF = pDriver->pBaseInterface;

		pBIF->I2cReadingByteNumMax      = RTK_DEMOD_BIF_TX_FIFO_DEPTH;
		pBIF->I2cWritingByteNumMax      = RTK_DEMOD_BIF_RX_FIFO_DEPTH;
		pBIF->I2cRead                   = __realtek_i2c_read_speedF;
		pBIF->I2cWrite                  = __realtek_i2c_write_speedF;
		pBIF->WaitMs                    = __realtek_wait_ms;
		pBIF->SetUserDefinedDataPointer = base_interface_SetUserDefinedDataPointer;
		pBIF->GetUserDefinedDataPointer = base_interface_GetUserDefinedDataPointer;
		pBIF->SetUserDefinedDataPointer(pBIF, (void*)pComm);

		// Init Demod Driver
		BuildRtkrAtsc3Module(&pDemod,
							 pDriver->pDemod,
							 pDriver->pBaseInterface,
							 pDriver->pI2CBridge,
							 Addr,
							 (unsigned int) CrystalFreq);            // Spectrum mode is inverse.
	}

	return pDriver;
}


/*----------------------------------------------------------------------
 * Func : GetT2PLPInfo
 *
 * Desc : Get T2 PLP Information
 *
 * Parm : pInfo : PLP Information
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_ATSC3_GetPLPInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo, unsigned char Atsc3MPlpMode)
{
	int ret = TUNER_CTRL_OK;
	unsigned char plp_index = 0;
	TV_ATSC3_PLP_ID Atsc3Plp = {0};

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;

	if (pDemodKernel == NULL) {
		ret = TUNER_CTRL_FAIL;
		goto GetPLPInfo_End;
	}

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	REALTEK_R_ATSC3_DBG("REALTEK_R_ATSC3_GetPLPInfo!! Mode = %u\n", Atsc3MPlpMode);

	pDemodKernel->Lock(pDemodKernel);
	/*Get PLP Data and PLP Type*/

	if (Atsc32plpHtiTiExtInterFlag == 1) {
		if (Atsc3MPlpMode ==  FULL_PLP_ID)
			memcpy(&Atsc3Plp, &Atsc3PlpAll, sizeof(TV_ATSC3_PLP_ID));
		else if (Atsc3MPlpMode ==  LLS_ONLY_PLP_ID)
			memcpy(&Atsc3Plp, &Atsc3PlpLlsOnly, sizeof(TV_ATSC3_PLP_ID));

	} else {
		ret |= DECODE_RET(pDemod->GetPlpID(pDemod, &Atsc3Plp, (ATSC3_MPLP_ID_MODE_SEL) Atsc3MPlpMode));
	}

	pDemodKernel->Unlock(pDemodKernel);

	if (ret != TUNER_CTRL_OK) {
		ret = TUNER_CTRL_FAIL;
		goto GetPLPInfo_End;
	}

	pInfo->atsc3.plp_cnt = Atsc3Plp.PLPCnt;
	REALTEK_R_ATSC3_DBG("plp_cnt = %u\n", pInfo->atsc3.plp_cnt);


	for (plp_index = 0; plp_index < pInfo->atsc3.plp_cnt; plp_index++) {
		pInfo->atsc3.plp[plp_index] = Atsc3Plp.PLPId[plp_index];
		REALTEK_R_ATSC3_DBG("PLP_ID[%u] = %u\n", plp_index, pInfo->atsc3.plp[plp_index]);
	}

GetPLPInfo_End:
	return ret;
}


/*----------------------------------------------------------------------
 * Func : ChangeT2PLP
 *
 * Desc : Get T2 PLP Information
 *
 * Parm : pInfo : PLP Information
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_ATSC3_ChangePLP(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	int ret = TUNER_CTRL_OK;
	int CounterTmp = 0;
	unsigned int atsc3_timeout = 3000;
	unsigned long stime = 0;
	unsigned int LockStatus = 0;
	unsigned char Atsc3_1PlpElOnly_multisub = 0xf;
	unsigned char ATSC3config2MplpCtiFlag = 0xf;
	unsigned char ldpcItrMode = 0,  ldpcIterNum = 50, ldpcLatencyCtrlEn = 0, ldpcLatencyVal = 0;
	unsigned char Atsc3_L1BD_latency_modify_flag = 0xf;
	unsigned int ReadingData = 0, swExtractPlp = 0;
	unsigned char CtiPlpNumCnt = 0;
	TV_ATSC3_PLP_ID Atsc3Plp = {0};

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;

	if (pDemodKernel == NULL)
		return TUNER_CTRL_FAIL;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	pRealtekRATSC3->m_auto_tune_en = 0;

	if (pInfo->atsc3.plp_cnt > 4) {
		REALTEK_R_ATSC3_WARNING("[ChangePLP]atsc3.plp_cnt  = %u!  It is illegal!!!!!! fix to 4 \n", pInfo->atsc3.plp_cnt);
		pInfo->atsc3.plp_cnt = 4;

	}

	Atsc3Plp.PLPCnt = pInfo->atsc3.plp_cnt;
	pRealtekRATSC3->m_AP_mPlpSelectedCnt =  pInfo->atsc3.plp_cnt;

	for (CounterTmp = 0; CounterTmp < pInfo->atsc3.plp_cnt; CounterTmp++) {
		Atsc3Plp.PLPId[CounterTmp] = pInfo->atsc3.plp[CounterTmp];
		pRealtekRATSC3->m_AP_mPlpSelectedNo[CounterTmp] = pInfo->atsc3.plp[CounterTmp];
		if ((pInfo->atsc3.plp[CounterTmp] & 0x80) == 0x80)
			CtiPlpNumCnt++;
	}

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ef40, &ReadingData);
	swExtractPlp =  ReadingData;

	if (CtiPlpNumCnt > 1) {
		if (((swExtractPlp & 0x40404040) == 0x40) && ((swExtractPlp & 0x3f) == (pRealtekRATSC3->m_AP_mPlpSelectedNo[0] & 0x3f))) { //single PLP

			REALTEK_R_ATSC3_INFO("[ChangePLP][mPlp Cti case] PLP ID is same as before! plp = %u \n", pRealtekRATSC3->m_AP_mPlpSelectedNo[0]);
			pRealtekRATSC3->m_auto_tune_en = 1;
			pDemodKernel->Unlock(pDemodKernel);
			return TUNER_CTRL_OK;
		}
	}

	ret |= DECODE_RET(pDemod->SetPlpID(pDemod, Atsc3Plp));

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ef40, &ReadingData);

	if (ReadingData == swExtractPlp) {
		REALTEK_R_ATSC3_INFO("[ChangePLP] PLP ID is same as before! PLPCnt=%u, plp = [%u/%u/%u/%u] \n", Atsc3Plp.PLPCnt, Atsc3Plp.PLPId[0], Atsc3Plp.PLPId[1], Atsc3Plp.PLPId[2], Atsc3Plp.PLPId[3]);
		pRealtekRATSC3->m_auto_tune_en = 1;
		pDemodKernel->Unlock(pDemodKernel);
		return TUNER_CTRL_OK;
	}

	realtek_r_atsc3_restore_default_setting(pDemod);
	realtek_r_atsc3_SetLdpcIterNum(pDemod, ldpcItrMode, ldpcIterNum);
	realtek_r_atsc3_SetLdpcLatency(pDemod, ldpcLatencyCtrlEn, ldpcLatencyVal);
	realtek_r_atsc3_LdpcAlphaFixModeEn(pDemod, 0);
	ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));


	pRealtekRATSC3->m_status_checking_stime = tv_osal_time() + ATSC3_STATUS_CHECK_INTERVAL_MS;
	pRealtekRATSC3->m_autotune_VVPatchCheckTime = tv_osal_time() + AUTOTUNE_CHECK_VV_PATCH_INTERVAL_MS;
	pRealtekRATSC3->m_autotune_L1CrcCheckTime = tv_osal_time() + AUTOTUNE_CHECK_L1Crc_INTERVAL_MS;
	pRealtekRATSC3->m_unlock_cnt = 0;
	pRealtekRATSC3->m_auto_tune_en = 1;

	Atsc3Vv526Flag = 0;
	Atsc3KvvCrossSymbFlag = 0;
	atsc3L1CRCErrCnt = 0;
	atsc3DataErrCnt = 0;
	Atsc32plpHtiTiExtInterFlag = 0xf;
	Atsc3mSubFrameFlag = 0xf;



	if (ret != TUNER_CTRL_OK)
		goto ChangePLP_Fail;

	stime = tv_osal_time();

	while ((tv_osal_time() - stime) <= atsc3_timeout) {
		tv_osal_msleep(5);
		ret |= DECODE_RET(pDemod->IsSignalLocked(pDemod, &LockStatus));


		if (ATSC3config2MplpCtiFlag == 0xf) {
			realtek_r_atsc3_mplp_cti_detect(pDemod, &ATSC3config2MplpCtiFlag);
			if (ATSC3config2MplpCtiFlag == 0x1) {
				Atsc3Plp.PLPCnt = 1;
				Atsc3Plp.PLPId[0] = pRealtekRATSC3->m_AP_mPlpSelectedNo[0];
				ret |= DECODE_RET(pDemod->SetPlpID(pDemod, Atsc3Plp));
				ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));
				atsc3_timeout = 4000;
				REALTEK_R_ATSC3_INFO("[mPLP CTI case] set PLP ID = %u\n", Atsc3Plp.PLPId[0]);
				//lock = YES;
				//break;
			}
		}

		if (Atsc3_L1BD_latency_modify_flag == 0xf) {
			realtek_r_atsc3_L1BD_latency_modify(pDemod, &Atsc3_L1BD_latency_modify_flag);
			if (Atsc3_L1BD_latency_modify_flag == 0x1)
				atsc3_timeout = 4000;
		}

		if (Atsc3Vv526Flag == 0)
			realtek_r_atsc3_vv526_detect_flow(pDemod, &Atsc3Vv526Flag);

		if (Atsc3KvvCrossSymbFlag == 0)
			realtek_r_atsc3_kvv_fdm_check_flow(pDemod, &Atsc3KvvCrossSymbFlag);

		if (Atsc3_1PlpElOnly_multisub == 0xf)
			realtek_r_atsc3_config1PlpElOnly_multisubframe(pDemod, &Atsc3_1PlpElOnly_multisub);

		if (Atsc32plpHtiTiExtInterFlag == 0xf)
			realtek_r_atsc3_2plp_hti_TiExtInter_detect(pDemod, &Atsc32plpHtiTiExtInterFlag, &Atsc3PlpAll, &Atsc3PlpLlsOnly);


		if (Atsc3mSubFrameFlag == 0xf) {
			realtek_r_atsc3_multisubframe_fft8K_ce_fix(pDemod, &Atsc3mSubFrameFlag);
			if (Atsc3mSubFrameFlag == 0x1) {
				ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));
				atsc3_timeout = 4000;
				REALTEK_R_ATSC3_INFO("Atsc3mSubFrameFlag = 1! demod reset~~\n");
			}
		}


		realtek_r_atsc3_1PLPLdpcOnlyDetect(pDemod);



		if (LockStatus == 1 && ret == TUNER_CTRL_OK) {
			realtek_r_atsc3_LdpcParamCheck(pDemod);
			realtek_r_atsc3_L1DUpdateItemCheck(pDemod);
			break;
		}


		if (stime > tv_osal_time())
			stime = 0;

	}

	REALTEK_R_ATSC3_INFO("[ChangePLP]PLPCnt=%u, plp = [%u/%u/%u/%u], Switch Time = %lu, LockStatus = %u\n", Atsc3Plp.PLPCnt, Atsc3Plp.PLPId[0], Atsc3Plp.PLPId[1], Atsc3Plp.PLPId[2], Atsc3Plp.PLPId[3], tv_osal_time() - stime, LockStatus);

	if (LockStatus != 1 || ret != TUNER_CTRL_OK)
		goto ChangePLP_Fail;

	pDemodKernel->Unlock(pDemodKernel);

	return TUNER_CTRL_OK;

ChangePLP_Fail:
	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_R_ATSC3_INFO("[ChangePLP] PLPCnt=%u, plp = [%u/%u/%u/ %u], Change PLP ID Fail!!!!!!!!\n", Atsc3Plp.PLPCnt, Atsc3Plp.PLPId[0], Atsc3Plp.PLPId[1], Atsc3Plp.PLPId[2], Atsc3Plp.PLPId[3]);

	return ret;
}

int REALTEK_R_ATSC3_TPOutEn(DEMOD* pDemodKernel, unsigned char enable)
{

	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	int ret = FUNCTION_SUCCESS;

	if (pDemodKernel == NULL)
		return TUNER_CTRL_FAIL;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;


	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB816F600, 0xFFFFFFFE, (enable & 0x1));
	pDemodKernel->m_ts.tsOutEn = enable;
	REALTEK_R_ATSC3_INFO("DtvRDemod TP Output enable = %u !!!\n", enable);
	return ret;
}

int REALTEK_R_ATSC3_PLL_Default(DEMOD* pDemodKernel, unsigned char On)
{

	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);


	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC3->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;
//PLL
//RL6681 register default setup
//rtd_outl(0xb80004b0,0x00000010);				//<bit4>MAIN_POR_EN = 0x1

	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004c4, 0x00000000);				//DTV PLLDIF PLLDIF_DIVDIG_SEL = 000(2), PLLDIF_DIVADC_SEL = 00(divide by 8)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004b8, 0x00003600);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004d4, 0x80086500);				//<bit31>Pll_xtal_pow=0x1, <bit20:19>PLL_LDO_XTAL_SEL=0x1(1.76V), <bit14>PLL_POW_LDO11V=0x1, <bit13:11>PLL_LDO11V_SEL=0x4,
	//<bit10>PLL_POW_LDO=0x1, <bit8:9>PLL_LDO_SEL=0x1(0.952V)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004d8, 0x00000000);				//Default setting, <bit15:8>PLL_RESERVE=0x0(0=Select ADC CLK From PLL27X_IFADC).
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004dc, 0x40000001);				//Default ATV mode, <bit30:29>PLL27X_IFADC_DUTY=0x2,<bit0>PLL_CK_IFADC_MUX=0x0(ATV mode)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8000500, 0x0000041c);				//<bit11:10>PLL27X_IFADC_CK108M_DIV=0x1(VCO=432M),<bit4>PLL27X_IFADC_DTV_CKO_EN=0x1(enable),<bit3:2>PLL27X_IFADC_ATV_CKO_SEL=0x3(135M or 162M (depend on VCO and REG_PLL27x_IFADC_DIV))
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8000504, 0x000e0224);				//PLL27X_IFADC VCO M,N, M<10>+2�P=14(VCO=432M)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8000508, 0x00000000);				//PLL27X_IFADC Poweroff and VCORSTB reset
	tv_osal_usleep(10);						//delay 10uS

//PLL27X
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb80004dc, 0xfffffffe, 0x00000000);	//Set MUX to DTV Mode<bit0>=0x0

//PLL27X_IFADC
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000500, 0xfffffff3, 0x0000000C);	//PPLL27X_IFADC_DTV_CKO_SEL = 00(/3), PLL27X_IFADC_ATV_CKO_SEL = 11 (135M or 162M (depend on VCO and REG_PLL27x_IFADC_DIV).
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000508, 0xfffffff8, 0x00000007);	//IFADC, RSTB and VCORSTB Power on
	return ret;
}


int REALTEK_R_ATSC3_ADC_Default(DEMOD* pDemodKernel, unsigned char On)
{
	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);


	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC3->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

//RL6681 ADC register default
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033100, 0x01110500);					//<bit25:24>REG_IF_BY_VCM_SEL=0x1, <bit21:20>REG_IPGA_MBIAS_ISEL=0x1, <bit16=1>IFD MBIAS power on,<bit11:10>REG_I_VCM_SEL=0x1(0.632V),<bit9:8>ADC2X_BG_SEL= 0x1(1.216V)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033104, 0x00000300);					//<bit24>IQADC input cm power off, <bit9:8>REG_IPGA_BW=0x3(37.5M)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033118, 0x00000000);					//<bit8>REG_I_CKOUT_EDGE=0x1=0(IADC CKOUT polarity=negative edge)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033124, 0x00000000);					//PGA/ADC Power Off
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033128, 0x00000002);					//Default setting(10=20uA), IQ PGA CURRENT  select
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803312C, 0x00000000);					//default setting, <bit3:0>REG_IPGA_D=0x0(IPGA gain=0dB)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033130, 0x08000000);					//default setting, <bit27:26>REG_IADC_I_VREF_SEL=0x2(1.5X)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033134, 0x11000091);					//default setting, <bit25:24>REG_IADC_VREF_SEL=0x1(1.4V), <bit8:7>REG_IADC_LDO0_SEL=0x1(1.3V), <bit2:0>REG_IADC_LDO_SEL=0x1(0.961V)

//Set IADC/IPGA power on and IPGA input
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8033124, 0xfffffefe, 0x00000101);		//I_PGA<bit8> power on, <bit0>I_ADC power on
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8033128, 0xfff8ffff, 0x00020000);		//<bit18:16>REG_IPGA_INPUT_SEL=0x2(IFD input)

//Set IPGA/QPGA GAIN = 0dB
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb803312c, 0xffffff00, 0x00000000);		//<bit3:0>reg_ipga_d=0x0(0dB)
	return ret;

}


int REALTEK_R_ATSC3_System_Init(DEMOD* pDemodKernel, unsigned char On)
{

	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);


	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC3->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

//--------CRT All turn on----------------//
	//ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x000001ff);
	//ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x000001ff);
//---------------------------------------//

//Turn on MISC 												//for GPIO
//<bit8>RSTN_MISC = 0x1(Release reset)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000101);
//<bit8>CLKEN_MISC = 0x1(Enable)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000101);


//DTV Demod Sel
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb80004b0, 0xffffffe8, 0x00000010); 				//<bit4>MAIN_POR_EN=0x1, <bit2>Dtv_demod_atb_clk_sel=0(Ldpc_Clk=370MHz), <bit1:0>dtv_demod_sel=00(atsc3.0) 3X

//IfdMbiasPowAndAdcPow
// 1. CRT_CLK_OnOff(TVADC, CLK_ON, NULL);					//Turn on CLKEN_IFADC
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000081);							//<bit7>RSTN_IFADC, set to 1(Release)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000081);							//<bit7>CLKEN_IFADC, set to 1
	tv_osal_usleep(50);									//delay 50uS

// 2. MBIAS and PLL_LDO Power turn on
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8033100, 0xfffeffff, 0x00010000);  			//<bit16>REG_POW_MBIAS turn on
	tv_osal_usleep(50);									//delay 50uS
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb80004d4, 0xffffbbff, 0x00004400);  			//<bit14>PLL_POW_LDO11V, <bit10>PLL_POW_LDO turn on
	tv_osal_usleep(50);									//delay 50uS

// 3. turn Off ADC/PGA power
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8033104, 0xfeffffff, 0x00000000);				//IQADC input cm power off <bit24> = 0
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8033124, 0xfffffefe, 0x00000000);				//IPGA/IADC Power Off
	tv_osal_usleep(50);									//delay 50uS

//----------------------------------------------------------//
//CRT_CLK_OnOff(TVALL, CLK_OFF, NULL);        				//Turn off TVALL
//DTV Demod IP reset
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x0000005a);							//Demod IP Reset<bit6, bit4, bit3, bit1> set to 0(active low)
	tv_osal_msleep(1);									//driver delay 1mS

//DTV Demod Wrapper reset
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000124);							//Demod IP Reset<bit8, bit5, bit2> set to 0(active low)
	tv_osal_msleep(1);									//driver delay 1mS

//DTV Demod turn off
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x000017e);							//Demod CLK<bit8, bit6, bit5, bit4, bit3, bit2, bit1> set to 0
	tv_osal_usleep(10);									//driver delay 10uS

//----------------------------------------------------------//
//#include "RL6681_TVFrontEnd_1_PLL_default.tbl"				//Load PLL default setting
	ret |= REALTEK_R_ATSC3_PLL_Default(pDemodKernel, On) ;

//--------PLL27X_IFADC-------------------
//RL6681_VCO=648MHz, ADC_CLK=144M, SYS_CLK=518.4MHz
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb80004b0, 0xffffffe8, 0x00000010);				//<bit4>MAIN_POR_EN = 0x1, <bit2>Dtv_demod_atb_clk_sel=0(Ldpc_Clk=370MHz), <bit1:0>dtv_demod_sel=00(atsc3.0)
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb80004dc, 0xfffffffe, 0x00000000);				//Set MUX to DTV Mode<bit0>=0x0
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000500, 0xffffffef, 0x00000010);				//PLL27x_IFADC DTV CLK<bit4>=0x1

#ifdef ATSC3_SYS_CLK_496P8M
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000504, 0xfc80fff8, 0x00150004);                        //VCO = M<6:0>+2 = 21+2, N:00=1, 27 /1 *23=621MHz, PLL27X_IFADC_M<bit22:16> = 0x15(21), PLL27X_IFADC_N<bit25:24>=0x0(1), PLL27X_IFADC PUMP current<bit2:0>=0x4(0x100:25uF)
#else
	//27M XTAL
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000504, 0xfc80fff8, 0x00160004);				//VCO = M<6:0>+2 = 22+2, N:00=1, 27 /1 *24=648MHz, PLL27X_IFADC_M<bit22:16> = 0x16(22), PLL27X_IFADC_N<bit25:24>=0x0(1), PLL27X_IFADC PUMP current<bit2:0>=0x4(0x100:25uF)
	//24M XTAL
	//ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000504, 0xfc80fff8, 0x00190004);				//VCO = M<6:0>+2 = 22+2, N:00=1, 27 /1 *24=648MHz, PLL27X_IFADC_M<bit22:16> = 0x16(22), PLL27X_IFADC_N<bit25:24>=0x0(1), PLL27X_IFADC PUMP current<bit2:0>=0x4(0x100:25uF)

#endif

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb80004c4, 0xffffff88, 0x00000037);				//System_CLK : PLLDIF_DIVDIG_SEL<bit6:4> = 0x3(/1.25), sys_clk = 648 / 1.25 = 518.4 MHz
	//ADC_CLK : PLLDIF_DIVADC_SEL<2:0> = 0x7(/4.5), Adc_clk = 648 / 4.5 =144MHz
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb80004b8, 0xFFFC03FB, 0x00021400);				//<bit17>ATVADCDIV_RSTB=0x1(normal), <bit13>DIF216M_EN=0x0, <bit12>DTVDIG_EN=0x1, <bit11>LDPC_DIV=0x0(div1.75), <bit10>LDPC_EN=0x1
	tv_osal_msleep(1);//delay 1ms

//*****RL6681_VCO=432MHz, ADC_CLK=144M, SYS_CLK=246.857MHz****//
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000508, 0xfffffff8, 0x00000000);  			//<2:1>PLL27X_IFADC_VCORSTB, PLL27X_IFADC_RSTB reset = 0(Reset); <0>PLL27X_IFADC_POW turn off
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000508, 0xfffffffe, 0x00000001);  			//<0>PLL27X_IFADC_POW turn On
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000508, 0xfffffff9, 0x00000006);  			//<2:1>PLL27X_IFADC_VCORSTB, PLL27X_IFADC_RSTB reset = 1(release)
	tv_osal_msleep(1);//delay 1ms

	tv_osal_usleep(50);									//driver delay 50uS
//#include "RL6681_TVFrontEnd_2_ADC_IFD.tbl"					//Load ADC default setting
	ret |= REALTEK_R_ATSC3_ADC_Default(pDemodKernel, On);

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB80004b8, 0xfffffdff, 0x00000000);  			//<bit9>IFADC ATV clk enable turn off
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8000500, 0xffffffef, 0x00000010);  			//<bit4>PLL27X_IFADC_DTV_CKO_EN setup
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB80004b8, 0xfffffdff, 0x00000200);  			//<bit9>IFADC ATV clk enable turn on
	tv_osal_usleep(50);									//driver delay 50uS

//----------------------------------------------------------//
//DtvDemodPowerControl(1)
//CRT_CLK_OnOff(TVFRONTEND, CLK_ON, NULL);					//Turn On TVFrontend(memory bank and dtv_forntend)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000061);							//<bit6>RSTN_dtv_frontend, <bit5>RSTN_dtv_demod_mb, <bit0>write bit = 0x1(release)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000061);							//<bit6>CLKEN_dtv_frontend, <bit5>Clken_dtv_demod_mb, <bit0>write bit = 0x1(CLKEN)
	tv_osal_usleep(10);									//driver delay 10uS

//----------- Set DTV Frontend path --------------//
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033800, 0x00010010);							//<bit18:16>Dtv_frontend_mode=0x1, <bit8>dtv_idata_sel=0x0(from dn flt), <bit4>Dtv_dnflt_en=0x1(DN6)
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8033804, 0xfffbffff, 0x00040000);				//input fifo clr, dtv_input_fifo_clr<bit18> FIFO waddr & raddr clear
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8033804, 0xfffbffff, 0x00000000);				//input fifo clr release, dtv_input_fifo_clr<bit18>, FIFO waddr & raddr clear

//Set if_agc
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb800081c, 0xFF0FFFFF, 0x00400000); 				//<bit23:20>GPIO_29, Set pinmux DEMOD_IF_AGC

//---------------Start set RL6681 down sample filter (dn6)-----------//
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033900, 0x1ffe1ffc);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033904, 0x00011fff);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033908, 0x00060003);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803390c, 0x00070007);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033910, 0x00020006);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033914, 0x1ff81ffd);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033918, 0x1ff31ff4);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803391c, 0x1ffb1ff5);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033920, 0x000d0004);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033924, 0x00170015);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033928, 0x000A0014);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803392c, 0x1fed1ffc);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033930, 0x1fda1fe0);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033934, 0x1fec1fde);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033938, 0x001a0002);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803393c, 0x003a002f);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033940, 0x00230037);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033944, 0x1fde0003);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033948, 0x1faa1fbd);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803394c, 0x1fc51fac);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033950, 0x002a1ff3);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033954, 0x007e005d);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033958, 0x005f0080);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803395c, 0x1fce001f);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033960, 0x1f4a1f80);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033964, 0x1f661f3e);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033968, 0x00381fbf);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803396c, 0x011200b4);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033970, 0x01040133);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033974, 0x1fc30082);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033978, 0x1e351eed);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803397c, 0x1dff1dd6);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033980, 0x00401ecd);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033984, 0x0480023b);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8033988, 0x08ad06c2);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb803398c, 0x0a6909f5);
//-----------------End of Set DownSample filter-------------//

//-----------------Enable DTV Demod IP-------------//
	tv_osal_usleep(50);									//driver delay 50uS
//DTV_DEMOD CLK / RESET flow : DTV Demod misc CRT turn on first then Demod IP CRT
//Enable MISC												//Turn On MISC
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000111);							//<bit8>RSTN_MISC, <bit4>RSTN_DTV_DEMOD, <bit0>write bit = 0x1(release)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000111);							//<bit8>CLKEN_MISC, <bit4>CLKEN_DTV_DEMOD <bit0>write bit = 0x1(CLKEN)
	tv_osal_usleep(10);									//delay 10uS

//DTV_ATB_Demod CLK/Reset Enable,  Demod wrapper frist then Demod IP
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000005);							//<bit2>RSTN_atb_demod_wrap, <bit0>write bit set to 1(release)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000005);							//<bit2>clken_atb_demod_wrap, <bit0>write bit set to 1
	tv_osal_usleep(10);									//delay 10uS
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000009);							//<bit3>RSTN_atb_demod, <bit0>write bit set to 1(release)
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000009);							//<bit3>clken_atb_demod, <bit0>write bit set to 1
	tv_osal_usleep(10);									//delay 10uS

//----Start RL6681 ATSC Demod TS output Pinmux setup----//
#ifndef REALTEK_R_EXT_QFN //MCM
	ret |= REALTEK_R_ATSC3_Ipga_Vcm_Setting(pDemodKernel);
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb800080c, 0x0f0fff0f, 0x10100010);	  //TP0 sync, val, clk
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000810, 0x0fff0f0f, 0x10001010);    //TP0 D7, D6, D5
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000814, 0xff0f0fff, 0x00101000);      //TP0 D4, D3
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000818, 0xff0f0fff, 0x00101000);	  //TP0 D2, D1
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb800081c, 0x0fffffff, 0x10000000);        //TP0 D0
#else //QFN
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb800082c, 0x0f0fffff, 0x20200000);	 //TPO_D0,D1
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000830, 0x0f0f0f0f, 0x20202020);  //TPO_D2,D3,D4,D5
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000838, 0xffffff0f, 0x00000020);  //TPO_Val
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb800083c, 0xff0f0f0f, 0x00202020);	 //TPO_D7,D6,Sync
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8000840, 0x0dffffff, 0x22000000);  //TPO_Clk, Driving
#endif
//----End of RL6681 ATSC Demod TS output Pinmux setup----//

//Bus CLK select
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb80004b0, 0xfffffff7, 0x00000008);	  //bus clk source:  ATB
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb80004ac, 0xfffffffe, 0x00000000);	  //BUS CLK sorce: PLLBUS


	tv_osal_usleep(50);									//delay 50uS


	return ret;

}


int REALTEK_R_ATSC3_PowerDown(DEMOD* pDemodKernel)
{
	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);


	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC3->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x000001fe);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x000001fe);

	return ret;


}

int REALTEK_R_ATSC3_BisrCheck(DEMOD * pDemodKernel)
{
	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;


	int loopcnt = 0;
	unsigned int Atsc3LdpcBisrDone = 0, BisrDone = 0, BicmBisrDone = 0;

	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);


	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC3->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;


	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB816F570, 0xFFFFFF80, 0x7f);  //bisr reset
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB816CD64, 0xFFFFFF7F, 0x80);  //mbisr_atsc3_ldpc_sw_reset
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161E08, 0xFFFFFFE0, 0x1f);  //BICM_BISR_RST_N

	while ((BisrDone != 0x7f) || (Atsc3LdpcBisrDone != 1) || (BicmBisrDone != 0x1f)) {
		loopcnt++;
		//tv_osal_usleep(100);
		tv_osal_msleep(1);
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816F584, &BisrDone);
		BisrDone &= 0x7f;
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816CD50, &Atsc3LdpcBisrDone);
		Atsc3LdpcBisrDone &= 0x1;
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8161E18, &BicmBisrDone);
		BicmBisrDone &= 0x1f;

		if (loopcnt > 120)
			break;
	}

	REALTEK_R_ATSC3_INFO("BisrCheck BisrDone= 0x%x, Atsc3LdpcBisrDone = 0x%x, BicmBisrDone = 0x%x,loopcnt = %d \n", BisrDone, Atsc3LdpcBisrDone, BicmBisrDone, loopcnt);

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB816F580, 0xFFFFFF80, 0x7f);  //bisr hold and remap
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB816CD60, 0xFFFFFFFE, 0x1);  //mbisr_atsc3_ldpc hold and remap
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161E20, 0xFFFFFFE0, 0x1f);  //BICM_BISR_HOLD_REMAP

	return ret;

}

int REALTEK_R_ATSC3_Ipga_Vcm_Setting(DEMOD * pDemodKernel)
{
	int ret = FUNCTION_SUCCESS;

/*	
	int OtpDone = 0;
	REALTEK_R_ATSC3* pRealtekRATSC3;
	ATSC3_DEMOD_MODULE* pDemod;
	
	pRealtekRATSC3 = (REALTEK_R_ATSC3*)(pDemodKernel->private_data);


	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC3->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC3_DRIVER_DATA*) pRealtekRATSC3->m_private)->pDemod;

	//0x18038188[26:24]: IFD_VCM_SEL(1 done bits + 2bits)
	OtpDone = (rtd_inl(EFUSE_DATAO_98_reg) >> 26) & 0x1;

	if(OtpDone) {
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8033100, 0xFFFFF3FF, ((rtd_inl(EFUSE_DATAO_98_reg) >> 24) & 0x03)<< 10);
	}

*/
	return ret;
}