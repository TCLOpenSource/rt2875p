/***************************************************************************************************
  File        : demod_realtek_h.cpp
  Description : REALTEK_H_ DVB-T demod
  Author      : Kevin Wang
****************************************************************************************************
    Update List :
----------------------------------------------------------------------------------------------------
    20120207    | create phase
***************************************************************************************************/
#include <linux/slab.h>
#include "demod_rtk_h_dtmb.h"
#include "demod_rtk_h_dtmb_priv.h"
#include "demod_rtk_common.h"
#include "tv_osal.h"
#include "dtmb_demod_rtk.h"
#include "rbus/hdic_register_bus_reg.h"

static unsigned long gPEstime = 0;
/*----------------------------------------------------------------------
 * Func : REALTEK_H_DTMB
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
void REALTEK_H_DTMB_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM*	pComm)
{
	REALTEK_H_DTMB* pRealtekHDTMB;

	pDemodKernel->private_data = kmalloc(sizeof(REALTEK_H_DTMB), GFP_KERNEL);
	if(pDemodKernel->private_data == NULL){
		goto kmalloc_fail;
	}
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);

	Demod_Constructors(pDemodKernel);

	pDemodKernel->m_id                 = DTV_DEMOD_REALTEK_H_DTMB;
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
	pRealtekHDTMB->m_output_freq        = output_freq;
	pRealtekHDTMB->m_auto_tune_enable   = 0;

	pRealtekHDTMB->m_private            = (void*) AllocRealtekHDtmbDriver(
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

	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "REALTEK_H_DTMB");

	pRealtekHDTMB->m_acquire_sig_en = 0;

	pDemodKernel->Init = REALTEK_H_DTMB_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = REALTEK_H_DTMB_AcquireSignal;
	pDemodKernel->ScanSignal = REALTEK_H_DTMB_ScanSignal;
	pDemodKernel->SetIF = REALTEK_H_DTMB_SetIF;
	pDemodKernel->SetTSMode = REALTEK_H_DTMB_SetTSMode;
	pDemodKernel->SetTVSysEx = REALTEK_H_DTMB_SetTVSysEx;
	pDemodKernel->SetMode = REALTEK_H_DTMB_SetMode;
	pDemodKernel->SetTVSys = REALTEK_H_DTMB_SetTVSys;
	pDemodKernel->GetLockStatus = REALTEK_H_DTMB_GetLockStatus;
	pDemodKernel->GetSegmentNum = Demod_GetSegmentNum;
	pDemodKernel->GetSignalInfo = REALTEK_H_DTMB_GetSignalInfo;
	pDemodKernel->GetSignalQuality = REALTEK_H_DTMB_GetSignalQuality;
	pDemodKernel->ChangeT2PLP = Demod_ChangeT2PLP;
	pDemodKernel->GetT2PLPInfo = Demod_GetT2PLPInfo;
	pDemodKernel->GetT2L1PreInfo = Demod_GetT2L1PreInfo;
	pDemodKernel->GetT2L1PostInfo = Demod_GetT2L1PostInfo;
	pDemodKernel->GetDemodStatus = Demod_GetDemodStatus;
	pDemodKernel->GetCarrierOffset = REALTEK_H_DTMB_GetCarrierOffset;
	pDemodKernel->ForwardI2C = REALTEK_H_DTMB_ForwardI2C;
	pDemodKernel->AutoTune = REALTEK_H_DTMB_AutoTune;
	pDemodKernel->KernelAutoTune = REALTEK_H_DTMB_KernelAutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = Demod_Suspend;
	pDemodKernel->Resume = Demod_Resume;
	pDemodKernel->SetTvMode = REALTEK_H_DTMB_SetTvMode;
	pDemodKernel->InspectSignal = REALTEK_H_DTMB_InspectSignal;	  // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
//	pDemodKernel->Lock = Demod_Lock;
//	pDemodKernel->Unlock = Demod_Unlock;
	pDemodKernel->Lock = REALTEK_H_DTMB_Lock;
	pDemodKernel->Unlock = REALTEK_H_DTMB_Unlock;

	//pDemodKernel->Activate = REALTEK_H_DTMB_Activate;
	pDemodKernel->Destory = REALTEK_H_DTMB_Destructors;

kmalloc_fail:
	REALTEK_H_DTMB_WARNING("REALTEK_H_DTMB_Constructors kmalloc Failed\n");
}

/*----------------------------------------------------------------------
 * Func : ~REALTEK_H_DTMB
 *
 * Desc : Destructor of REALTEK_H_DTMB
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void REALTEK_H_DTMB_Destructors(DEMOD* pDemodKernel)
{
	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);

	if (pRealtekHDTMB->m_private != NULL)
		ReleaseRealtekHDtmbDriver((REALTEK_H_DTMB_DRIVER_DATA*)pRealtekHDTMB->m_private);

	kfree(pRealtekHDTMB);
	//pDemodKernel->Destory(pDemodKernel);
	REALTEK_H_DTMB_INFO("REALTEK_H_DTMB_Destructors Complete\n");
}



/*----------------------------------------------------------------------
 * Func : Init
 *
 * Desc : Initial realtek_h
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_H_DTMB_Init(DEMOD* pDemodKernel)
{
	int i = 0;
	UINT8 DemodTarget = 0;

	unsigned int AllocateSize = 0;
	unsigned long ShareMemoryPhysicalAddress;

	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekHDTMB->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);
	pRealtekHDTMB->m_auto_tune_enable   = 0;

	//pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*) pRealtekHDTMB->m_private)->pDemod;
	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_Init\n" "\033[m");

	DtvDemodInitialization(RTK_DEMOD_MODE_DTMB, 1);

	AllocateSize = CravedoutMemoryQuery(&ShareMemoryPhysicalAddress);

	if (AllocateSize < RTK_H_DTMB_DDR_SIZE) {
		REALTEK_H_DTMB_WARNING("\033[0;32;31m""allocate share memory(dtmb) - allocate DDR size is not enough!!!\n""\033[m");
		goto init_fail;
	}


	REALTEK_H_DTMB_INFO("allocate share memory(dtmb) - Physical_addr=0x%08lx, size=0x%08x\n", ShareMemoryPhysicalAddress, AllocateSize);

	rtd_outl(HDIC_REGISTER_BUS_hdic_mem_bus_ctrl4_reg, ShareMemoryPhysicalAddress);

	DTMBIP_Init();
	DTMBIP_SetWorkMode();
	DTMBIP_SetAutoMode();

	// check tuner information
	if (pDemodKernel->m_pTuner) {
		TUNER_PARAM Param;

		if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, TV_SYS_DTMB_8M, &Param) == TUNER_CTRL_OK) {
			switch (Param.id)  {
			case TV_TUNER_NXP_TDA18275A:
				REALTEK_H_DTMB_INFO("Tuner id %d, TV_TUNER_NXP_TDA182XX\n", Param.id);
				pRealtekHDTMB->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) TDA18275A_OPT_VALUE_DTMB;
				pRealtekHDTMB->m_TunerOptRegCnt = sizeof(TDA18275A_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;

			case TV_TUNER_SILICONLABS_SI2151:
			case TV_TUNER_SILICONLABS_SI2178B:
			case TV_TUNER_NO_TUNER:
				REALTEK_H_DTMB_INFO("Tuner id %d, TV_TUNER_SILICONLABS_SI21XX\n", Param.id);
				pRealtekHDTMB->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SI2151_OPT_VALUE_DTMB;
				pRealtekHDTMB->m_TunerOptRegCnt = sizeof(SI2151_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;

			case TV_TUNER_RAFAEL_R840:
				REALTEK_H_DTMB_INFO("Tuner id %d, TV_TUNER_RAFAEL_R840\n", Param.id);
				pRealtekHDTMB->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) R840_OPT_VALUE_DTMB;
				pRealtekHDTMB->m_TunerOptRegCnt = sizeof(R840_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;

			case TV_TUNER_MAXLINEAR_MXL661:
				REALTEK_H_DTMB_INFO("Tuner id %d, TV_TUNER_MAXLINEAR_MXL6X1\n", Param.id);
				pRealtekHDTMB->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) MXL661_OPT_VALUE_DTMB;
				pRealtekHDTMB->m_TunerOptRegCnt = sizeof(MXL661_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;

			case TV_TUNER_SONY_SUTRX243:
			case TV_TUNER_SONY_SUTRX243_RA:
				DemodTarget = 0;
				REALTEK_H_DTMB_INFO("Tuner id %d, TV_TUNER_SONY_SUTRX243\n", Param.id);
				pRealtekHDTMB->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SUTRX243_OPT_VALUE_DTMB;
				pRealtekHDTMB->m_TunerOptRegCnt = sizeof(SUTRX243_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;

			case TV_TUNER_ALTOBEAM_ATBM253:
				REALTEK_H_DTMB_INFO("Tuner id %d, TV_TUNER_ALTOBEAM_ATBM253\n", Param.id);
				pRealtekHDTMB->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) ATBM253_OPT_VALUE_DTMB;
				pRealtekHDTMB->m_TunerOptRegCnt = sizeof(ATBM253_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			default:
				REALTEK_H_DTMB_INFO("Unkonw tuner id %d, using default setting\n", Param.id);
				pRealtekHDTMB->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SI2151_OPT_VALUE_DTMB;
				pRealtekHDTMB->m_TunerOptRegCnt = sizeof(SI2151_OPT_VALUE_DTMB) / sizeof(REG_VALUE_ENTRY_RBUS);

				break;
			}

			pDemodKernel->m_if.freq      = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola  = Param.if_agc_pola;
		}
	}

	// Run Optimize Configuration for specified tuner
	for (i = 0; i < pRealtekHDTMB->m_TunerOptRegCnt; i++) {
		U32BITS Reg = pRealtekHDTMB->m_pTunerOptRegRbus[i].RegBitNameRbus;
		U32BITS Val = pRealtekHDTMB->m_pTunerOptRegRbus[i].ValueRbus;
		rtd_maskl(Reg, 0xffffff00, Val);
	}

	pDemodKernel->Unlock(pDemodKernel);

	if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
		goto init_fail;

	if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK)
		goto init_fail;

	REALTEK_H_DTMB_INFO("REALTEK_H_DTMB Init Complete\n");

	return TUNER_CTRL_OK;

init_fail:

	REALTEK_H_DTMB_WARNING("\033[1;32;32m" "REALTEK_H_DTMB Init Failed\n" "\033[m");

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
int REALTEK_H_DTMB_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);

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
int REALTEK_H_DTMB_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO*  pSigInfo)
{
	TUNER_PARAM Param;
	REALTEK_H_DTMB* pRealtekHDTMB;
	DTMB_DEMOD_MODULE* pDemod;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*) pRealtekHDTMB->m_private)->pDemod;
	//FUNCTION_NAME_PRINT("REALTEK_H_DTMB::SetTVSysEx\n");

	if (!IsDtmbSys(sys)) {
		REALTEK_H_DTMB_WARNING("Set TV system failed, unsupported TV system\n");
		return TUNER_CTRL_FAIL;
	}

	pRealtekHDTMB->m_auto_tune_enable = 0;

	if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, pDemodKernel->m_tv_sys, &Param) == TUNER_CTRL_OK) {
		if ((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola  != Param.if_agc_pola)) {
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;
			if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)  return  TUNER_CTRL_FAIL;
		}
	}

	pDemodKernel->Lock(pDemodKernel);
	switch (sys) {
	case TV_SYS_DTMB_6M:
		//REALTEK_H_DTMB_INFO("\033[1;32;32m""REALTEK_H_DTMB_SetTVSysEx_6M Sys = %d\n""\033[m", sys);
		DTMBIP_SetSytemBandwidth6M();
		break;

	case TV_SYS_DTMB:
	case TV_SYS_DTMB_8M:
	default:
		DTMBIP_SetSytemBandwidth8M();
		//REALTEK_H_DTMB_INFO("\033[1;32;32m""REALTEK_H_DTMB_SetTVSysEx_8M Sys = %d\n""\033[m", sys);
		break;
	}

	DTMBIP_SoftReset();

	pDemodKernel->Unlock(pDemodKernel);


	pDemodKernel->m_tv_sys = sys;
	pRealtekHDTMB->m_acquire_sig_en = 1;

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
int REALTEK_H_DTMB_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_H_DTMB* pRealtekHDTMB;
	UINT8 demod_spec_invert = 0;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);
	//pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*) pRealtekHDTMB->m_private)->pDemod;
	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_SetIF\n" "\033[m");

	pDemodKernel->Lock(pDemodKernel);

	if (pParam->agc_pola == 1) {            //Register 0x01EE <bit_4> if_agc_pola : postive = 0(default); negative = 1;
		//postive agc gain, 1: postive, 0:negative
		DTMBIP_WriteRegister(0x01EE, 0x11);
		REALTEK_H_DTMB_INFO("Tuner IF_AGC Pola = negative\n");
	}

	//set Demod_spectrum_invert
	demod_spec_invert = !(pParam->inversion);
	DTMBIP_SetSpectrum(demod_spec_invert);         //set spectrum.invert

	REALTEK_H_DTMB_INFO("SetIF, IF_freq="PT_U32BITS", spectrumn inv=%d, agc_pola=%d\n",
						pParam->freq, demod_spec_invert, pParam->agc_pola);

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
int REALTEK_H_DTMB_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);

	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_SetMode\n" "\033[m");

	switch (mode) {
	case TV_OP_MODE_NORMAL:
	case TV_OP_MODE_SCANING:
		break;

	case TV_OP_MODE_STANDBY:
		break;

	case TV_OP_MODE_ATV_NORMAL:
	case TV_OP_MODE_TP_ONLY:
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
int REALTEK_H_DTMB_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;
	UINT8 TsSerial, ClkPhase;

	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);
	//pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*) pRealtekHDTMB->m_private)->pDemod;
	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_SetTSMode\n" "\033[m");

	pDemodKernel->Lock(pDemodKernel);


	TsSerial = (pParam->mode == SERIAL_MODE) ? DTMB_TS_OUTPUT_SERIAL : DTMB_TS_OUTPUT_PARALLEL;
	ClkPhase = (pParam->clk_pol == RISING_EDGE) ? DTMB_TS_DATA_SAMPLE_RISING : DTMB_TS_DATA_SAMPLE_FALLING;


	ret = DTMBIP_SetTsFormat(TsSerial, ClkPhase, DTMB_TS_CLK_WITH_TSVLD, DTMB_TS_NULLPACKET_DELETED, DTMB_TS_SERIAL_OUTPUT_D7, DTMB_TS_SERIAL_MSB_TO_LSB, DTMB_TS_SERIAL_SYNC_BYTE);

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
int REALTEK_H_DTMB_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{
	UINT8 LockResult = 0, locked = 0, status = 0;
	int ret = TUNER_CTRL_OK;
	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);
	//pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*)pRealtekHDTMB->m_private)->pDemod;
	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_GetLockStatus\n" "\033[m");

	//if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DTMB))
	//	return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);
	DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &status);
	REALTEK_H_DTMB_INFO("DTMB_LockResult =%x, DTMB_Status =%x\n", LockResult, status);

	if ((LockResult == 1) && (status >= 0x86)) {
		//REALTEK_H_DTMB_DBG(" RTD_2938_DTMB::GetLockStatus = true \n");
		*pLock = DTV_SIGNAL_LOCK;
	} else {
		//REALTEK_H_DTMB_DBG(" RTD_2938_DTMB::GetLockStatus = fail \n");
		*pLock = DTV_SIGNAL_NOT_LOCK;
	}
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
int REALTEK_H_DTMB_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	UINT8 LockResult = 0, locked = 0, status = 0;
	UINT8 CarrierMode = 0, pmode = 0, prate = 0, pinterleaverLength = 0, Pnmode = 0, PnVariable = 0;
	BOOL SignalDetect = FALSE;
	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);
	//pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*)pRealtekHDTMB->m_private)->pDemod;
	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_GetSignalInfo\n" "\033[m");

	if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DTMB))
		return TUNER_CTRL_FAIL;
	pDemodKernel->Lock(pDemodKernel);

	DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &status);

	if ((status & 0xf0) > 0x20) {
		SignalDetect = TRUE;
		DTMBIP_GetParameters(&CarrierMode, &pmode, &prate, &pinterleaverLength, &Pnmode, &PnVariable);

		pInfo->mod = TV_MODULATION_DTMB_OFDM;

		switch (CarrierMode) {
		case DTMB_OFDM_CARRIER_SINGLE:
			pInfo->dtmb_ofdm.carrier_mode = DTMB_CARRIER_SINGLE;
			break;
		case DTMB_OFDM_CARRIER_MULTI:
			pInfo->dtmb_ofdm.carrier_mode = DTMB_CARRIER_MULTI;
			break;
		}


		switch (pmode) {
		case DTMB_OFDM_QAM_4_NR:
			pInfo->dtmb_ofdm.fec.constellation = DTMB_DTMB_4QAM_NR;
			break;
		case DTMB_OFDM_QAM_4:
			pInfo->dtmb_ofdm.fec.constellation = DTMB_DTMB_4QAM;
			break;
		case DTMB_OFDM_QAM_16:
			pInfo->dtmb_ofdm.fec.constellation = DTMB_DTMB_16QAM;
			break;
		case DTMB_OFDM_QAM_32:
			pInfo->dtmb_ofdm.fec.constellation = DTMB_DTMB_32QAM;
			break;
		case DTMB_OFDM_QAM_64:
			pInfo->dtmb_ofdm.fec.constellation = DTMB_DTMB_64QAM;
			break;
		}


		switch (Pnmode) {
		case DTMB_OFDM_PN_945:
			pInfo->dtmb_ofdm.pn.mode = DTMB_PN_945;
			break;
		case DTMB_OFDM_PN_595:
			pInfo->dtmb_ofdm.pn.mode = DTMB_PN_595;
			break;
		case DTMB_OFDM_PN_420:
			pInfo->dtmb_ofdm.pn.mode = DTMB_PN_420;
			break;
		}



		switch (prate) {
		case DTMB_OFDM_FEC_4_OVER_10:
			pInfo->dtmb_ofdm.fec.rate = DTMB_RATE_04;
			break;
		case DTMB_OFDM_FEC_6_OVER_10:
			pInfo->dtmb_ofdm.fec.rate = DTMB_RATE_06;
			break;
		case DTMB_OFDM_FEC_8_OVER_10:
			pInfo->dtmb_ofdm.fec.rate = DTMB_RATE_08;
			break;
		}



		switch (PnVariable) {
		case DTMB_OFDM_PN_SEQ_VARIABLE:
			pInfo->dtmb_ofdm.pn.seq = DTMB_PN_VARIABLE;
			break;
		case DTMB_OFDM_PN_SEQ_CONSTANT:
			pInfo->dtmb_ofdm.pn.seq = DTMB_PN_FIXED;
			break;
		}

		switch (pinterleaverLength) {
		case DTMB_OFDM_TIM_240:
			pInfo->dtmb_ofdm.tim = DTMB_INTERLEAVER_240;
			break;
		case DTMB_OFDM_TIM_720:
			pInfo->dtmb_ofdm.tim = DTMB_INTERLEAVER_720;
			break;
		}


		REALTEK_H_DTMB_DBG("GetSignalInfo CARRIER = %d, QAM_MODE = %d, PN_MODE = %d, DTMB_RATE = %d, INTERLEAVER = %d\n", pInfo->dtmb_ofdm.carrier_mode, pInfo->dtmb_ofdm.fec.constellation, pInfo->dtmb_ofdm.pn.mode, pInfo->dtmb_ofdm.fec.rate, pInfo->dtmb_ofdm.tim);

	}
	pDemodKernel->Unlock(pDemodKernel);
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
int REALTEK_H_DTMB_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id,	TV_SIG_QUAL* pQuality)
{
	int ret = TUNER_CTRL_OK;
	int TxRSSIdBm;
	U32BITS val;
	U32BITS BadBerPakN, BadPerN, BadBecN;
	U32BITS IF_AGC;    //NCO
	UINT8 ReadData0, ReadData1;
	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);
	//pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*) pRealtekHDTMB->m_private)->pDemod;

	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_GetSignalQuality\n" "\033[m");

	if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DTMB))
		return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);

	switch (id) {
	case TV_QUAL_SNR :
		ret = DTMBIP_GetSignalSNR(&val);
		//ret = DTMBIP_GetSignalSNRMC(&val);
		if (ret != TUNER_CTRL_OK) ret = TUNER_CTRL_FAIL;
		pQuality->snr = val;
		REALTEK_H_DTMB_DBG("RealtekH SNR = "PT_U32BITS"\n", pQuality->snr);

		break;

	case TV_QUAL_PEC :
	case TV_QUAL_PER :
		ret = DTMBIP_GetPer(&BadPerN);
		if (ret != TUNER_CTRL_OK) ret = TUNER_CTRL_FAIL;
		pQuality->per = BadPerN;
		pQuality->layer_per.per_num = BadPerN;
		pQuality->layer_per.per_den = 32768;
		REALTEK_H_DTMB_DBG("RealtekH PacketError = "PT_U32BITS"\n", pQuality->per);
		break;

	case TV_QUAL_BEC :
		ret = DTMBIP_GetBec(&BadBecN);
		if (ret != TUNER_CTRL_OK) ret = TUNER_CTRL_FAIL;
		pQuality->layer_ber.ber_num = BadBecN;
		//pQuality->layer_ber.ber_num = 0;
		pQuality->layer_ber.ber_den = 32768 * 188 * 8;
		REALTEK_H_DTMB_DBG("RealtekH Bit_Error_Count = "PT_U32BITS"\n", BadBecN);
		break;

	case TV_QUAL_BER :
		ret = DTMBIP_GetBer(&BadBerPakN);
		if (ret != TUNER_CTRL_OK) ret = TUNER_CTRL_FAIL;
		//pQuality->ber = 0;
		pQuality->layer_ber.ber_num = BadBerPakN;
		pQuality->layer_ber.ber_den = 100000000;
		REALTEK_H_DTMB_INFO("RealtekH Bit_Error_Rate = "PT_U32BITS"\n", BadBerPakN);
		break;

	case TV_QUAL_ITER :
		pQuality->iter = 0;
		ret = TUNER_CTRL_OK;

		break;

	case TV_QUAL_UCBLOCKS :
		pQuality->ucblocks = 0;
		ret = TUNER_CTRL_OK;
		break;

	case TV_QUAL_SIGNAL_STRENGTH:
		TxRSSIdBm = (int) pQuality-> rflevel;
		ret = DTMBIP_GetSignalStrength(&pQuality->strength,&TxRSSIdBm);
	        REALTEK_H_DTMB_INFO("RF=%d\n",TxRSSIdBm );
		if (ret != TUNER_CTRL_OK) ret = TUNER_CTRL_FAIL;
		break;

	case TV_QUAL_SIGNAL_QUALITY:
		ret = DTMBIP_GetSignalQuality(&pQuality->quality);
		REALTEK_H_DTMB_DBG("RealtekH SignalQuality = %d\n", pQuality->quality);
		if (ret != TUNER_CTRL_OK) ret = TUNER_CTRL_FAIL;
		break;

	case TV_QUAL_DEMODFWVERSION:
		pQuality->DemodFwVersion = 0x000001;
		break;

	case TV_QUAL_AGC:
		DTMBIP_ReadRegister(0x0061, &ReadData0);    //(0x0061) high bit[11-8]
		DTMBIP_ReadRegister(0x0062, &ReadData1);    //(0x0062) low bit[7-0]
		IF_AGC = (((ReadData0 & 0x0f) << 8) + ReadData1);
		REALTEK_H_DTMB_DBG("RealtekH IF_AGC Value = "PT_U32BITS"\n", IF_AGC);
		pQuality->agc = ((IF_AGC * 100) / 4096);
		break;
	//**************Get carrier offset*************************//
	/*
	DTMBIP_GetNCOValue(&NCOValue);
	REALTEK_H_DTMB_DBG("RealtekH NCOValue = %d\n",NCOValue);
	*/
	default:
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
int REALTEK_H_DTMB_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_H_DTMB* pRealtekHDTMB;
	U32BITS NCOValue;
	int64_t offset = 0;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);
	//pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*) pRealtekHDTMB->m_private)->pDemod;
	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_GetCarrierOffset\n" "\033[m");

	if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DTMB))
		return TUNER_CTRL_FAIL;


	pDemodKernel->Lock(pDemodKernel);
	DTMBIP_GetNCOValue((UINT32 *)&NCOValue);
	REALTEK_H_DTMB_DBG("RealtekH NCOValue = "PT_U32BITS"\n", NCOValue);
	//offset = ((NCOValue*1000)/8388608)*Fsample - (Tuner_IF*1000);
	offset = (int64_t)NCOValue;
	REALTEK_H_DTMB_DBG("RealtekH offset2 = %lld\n", offset);
	offset = ((offset * 30375000) >> 23) - 5000000;
	if (offset > 0)
		offset += 500;
	else
		offset -= 500;
	* pOffset = offset;
	REALTEK_H_DTMB_DBG("RealtekH DTMB offset = "PT_S32BITS"\n", *pOffset);
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
int REALTEK_H_DTMB_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned long cur_time;
#endif

	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);

	if (!WaitSignalLock) {
		REALTEK_H_DTMB_INFO("WaitSignalLock = %u \n", WaitSignalLock);
		return TUNER_CTRL_OK;
	}

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	cur_time = tv_osal_time();
	gPEstime = tv_osal_time();

	while ((tv_osal_time() - cur_time) <= 15) {

		if (pRealtekHDTMB->m_acquire_sig_en == 0)
			return TUNER_CTRL_OK;

		tv_osal_msleep(5);
	}

	return TUNER_CTRL_FAIL;

#else
	REALTEK_H_DTMB_AcquireSignalThread(pDemodKernel);
	//pRealtekRATSC->m_autotune_stime = tv_osal_time() + AUTODETECT_SIG_UPDATE_INTERVAL_MS;
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
int REALTEK_H_DTMB_AcquireSignalThread(DEMOD* pDemodKernel)
{
	unsigned long   stime;
	unsigned char   lock;
	unsigned int lockcnt = 0;
	UINT8   Status, LockResult, locked, no_signal , Status_final;
	UINT8 CarrierMode = 0, pmode = 0, prate = 0, pinterleaverLength = 0, Pnmode = 0, PnVariable = 0;

	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);

	if (pRealtekHDTMB->m_acquire_sig_en == 0)
		return TUNER_CTRL_OK;

	REALTEK_H_DTMB_DBG("\033[1;32;32m""Demod REALTEK_H_DTMB_AcquireSignalThread\n" "\033[m");

	pRealtekHDTMB->m_auto_tune_enable = 0;

	stime = tv_osal_time();
	no_signal = YES;
	lock = NO;

	pDemodKernel->Lock(pDemodKernel);

	while ((tv_osal_time() - stime) <= 400) {   // Check demod status, if state <= 0x02 no signal
		tv_osal_msleep(5);
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &Status);
		Status_final = Status & 0xf0;
		if (Status_final > 0x20) {
			no_signal = NO;
			break;
		}
	}

	if (no_signal == 1) {
		REALTEK_H_DTMB_INFO("\033[1;32;32m""no_signal = %u, time = %lu\n""\033[m", no_signal, tv_osal_time() - stime);
		goto acquire_signal_result;
	}

	while ((tv_osal_time() - stime) <= 600) {   // No signal detect 900ms, default = 600ms
		tv_osal_msleep(5);
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &Status);
		Status_final = Status & 0xf0;
		if (Status_final > 0x30) {
			no_signal = NO;
			break;
		}
	}

	if (no_signal == 1) {
		REALTEK_H_DTMB_DBG("no_signal = %u, time = %lu\n", no_signal, tv_osal_time() - stime);
		goto acquire_signal_result;
	}

	while ((tv_osal_time() - stime) <= 2500) {  // acquiresignal timeout set 3000mS
		tv_osal_msleep(5);
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &Status);
		if (Status >= 0x86) {
			DTMBIP_GetParameters(&CarrierMode, &pmode, &prate, &pinterleaverLength, &Pnmode, &PnVariable);      //check Chengsha SC
			tv_osal_msleep(1);
			REALTEK_H_DTMB_DBG("[Pre_Acq]CarrierMode = %d, pmode = %d, Pnmode = %d, prate =%d\n", CarrierMode, pmode, Pnmode, prate);
			if ((CarrierMode == DTMB_CARRIER_SINGLE) && (pmode == DTMB_DTMB_16QAM) && (Pnmode == DTMB_PN_595) && (prate == DTMB_RATE_08)) {
				DTMBIP_WriteRegister(0x02E0, 0x00000035);//0x2e0
				DTMBIP_WriteRegister(0x02D3, 0x00000040);//0x2d3

				DTMBIP_WriteRegister(0x0250, 0x00000006);//0x250
				DTMBIP_WriteRegister(0x0316, 0x0000000A);//0x316
				tv_osal_msleep(1);
				REALTEK_H_DTMB_DBG("Change Register!\n");
			} else {
				DTMBIP_WriteRegister(0x02E0, 0x00000090);//0x2e0
				DTMBIP_WriteRegister(0x02D3, 0x00000010);//0x2d3

				DTMBIP_WriteRegister(0x0250, 0x0000000A);//0x250
				DTMBIP_WriteRegister(0x0316, 0x00000014);//0x316
				tv_osal_msleep(1);
			};
		}

		if (LockResult == 1) {
			lockcnt++;
			if (lockcnt > 2) {
				lock = YES;
				break;
			}
		} else
			lockcnt = 0;
	}

	if (lock == YES) {
		DTMBIP_GetParameters(&CarrierMode, &pmode, &prate, &pinterleaverLength, &Pnmode, &PnVariable);
		REALTEK_H_DTMB_INFO("[AcquireSignal]CarrierMode = %d, pmode = %d, Pnmode = %d, prate =%d, PnVariable =%d\n", CarrierMode, pmode, Pnmode, prate, PnVariable);
		if ((CarrierMode == DTMB_CARRIER_MULTI) && (pmode == DTMB_DTMB_32QAM) && (Pnmode == DTMB_PN_420) && (prate == DTMB_RATE_08) && (PnVariable == DTMB_PN_FIXED)) { //gz482
			RealtekHDtmbModeChange();
		} else {
			RealtekHDtmbModeDefault();
			if ((CarrierMode != DTMB_CARRIER_SINGLE) && (pmode != DTMB_DTMB_16QAM) && (Pnmode != DTMB_PN_595) && (prate != DTMB_RATE_08)) {
				DTMBIP_WriteRegister(0x0250, 0x0000000a);	//0x0250 CS642also
				DTMBIP_WriteRegister(0x0316, 0x00000014);	//0x0316 CS642also
				DTMBIP_WriteRegister(0x02E0, 0x00000090);	//0x02e0 CS642also
			}
		}
	}

	REALTEK_H_DTMB_INFO("\033[1;32;32m""signal lock = %u, DemodStatus = 0x%x, Time = %lu\n" "\033[m", LockResult, Status, (tv_osal_time() - stime));
	pDemodKernel->Unlock(pDemodKernel);

	pRealtekHDTMB->m_auto_tune_enable = 1;
	pRealtekHDTMB->m_auto_tune_reset = 1;
	pRealtekHDTMB->m_acquire_sig_en = 0;
	return TUNER_CTRL_OK;
	//return (lock) ? TUNER_CTRL_OK : TUNER_CTRL_NO_SIGNAL;

acquire_signal_result:
	pDemodKernel->Unlock(pDemodKernel);
	pRealtekHDTMB->m_auto_tune_enable = 1;
	pRealtekHDTMB->m_auto_tune_reset = 1;
	pRealtekHDTMB->m_acquire_sig_en = 0;
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
int REALTEK_H_DTMB_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range)
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
int REALTEK_H_DTMB_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off)
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
int REALTEK_H_DTMB_AutoTune(DEMOD* pDemodKernel)
{
#if 0
	UINT8 LockResult, locked,  Status;
	static unsigned long stime = 0;
	UINT8   CarrierMode = 0, pmode = 0, prate = 0, pinterleaverLength = 0, Pnmode = 0, PnVariable = 0;

	static unsigned long UnlockStime = 0;
	static unsigned char UnlockHappened = 0;
	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);
	//DTMB_DEMOD_MODULE* pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*) pRealtekHDTMB->m_private)->pDemod;
	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_AutoTune\n" "\033[m");
	if (pRealtekHDTMB->m_auto_tune_enable) {
		pDemodKernel->Lock(pDemodKernel);
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &Status);
		pDemodKernel->Unlock(pDemodKernel);

		if ((LockResult != 1) && (UnlockHappened == 0) && (pRealtekHDTMB->m_auto_tune_reset == 1)) {
			UnlockStime = tv_osal_time();
			UnlockHappened = 1;
			pRealtekHDTMB->m_auto_tune_reset = 0;
		} else if ((LockResult == 1) && ((Status == 0x86) || (Status == 0x87) || (Status == 0x8e) || (Status == 0x8f))) {
			//REALTEK_H_DTMB_INFO("\033[1;32;31m" "Autotune - Lock\n" "\033[m");

			if (LockResult == YES) {
				DTMBIP_GetParameters(&CarrierMode, &pmode, &prate, &pinterleaverLength, &Pnmode, &PnVariable);
				REALTEK_H_DTMB_INFO("[Auto_tune_1]CarrierMode = %d, pmode = %d, Pnmode = %d, prate =%d\n", CarrierMode, pmode, Pnmode, prate);
				if ((CarrierMode == DTMB_CARRIER_MULTI) && (pmode == DTMB_DTMB_32QAM) && (Pnmode == DTMB_PN_420) && (prate == DTMB_RATE_08))
					rtd_outl(0xB8170BA8, 0x00000004);
				else
					rtd_outl(0xB8170BA8, 0x00000014);
			}

			UnlockStime = tv_osal_time();
			UnlockHappened = 0;
		}
		if (((LockResult != 1) && (Status >= 0x86)) && (tv_osal_time() - UnlockStime >= 1000) && (tv_osal_time() - stime >= 2550)) {
			REALTEK_H_DTMB_INFO("\033[1;32;31m" "Autotune - Acquire\n" "\033[m");
			//REALTEK_H_DTMB_INFO("AUTOTUNE tv_osal_time() - UnlockStime = %d\n", tv_osal_time() - UnlockStime);
			//REALTEK_H_DTMB_INFO("AUTOTUNE tv_osal_time() - stime = %d\n", tv_osal_time() - stime);

			pDemodKernel->Lock(pDemodKernel);
			DTMBIP_SetWorkMode();
			DTMBIP_SetAutoMode();
			DTMBIP_SoftReset();

			DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &Status);
			if (LockResult == YES) {
				DTMBIP_GetParameters(&CarrierMode, &pmode, &prate, &pinterleaverLength, &Pnmode, &PnVariable);
				REALTEK_H_DTMB_INFO("[Auto_tune_2]CarrierMode = %d, pmode = %d, Pnmode = %d, prate =%d\n", CarrierMode, pmode, Pnmode, prate);
				if ((CarrierMode == DTMB_CARRIER_MULTI) && (pmode == DTMB_DTMB_32QAM) && (Pnmode == DTMB_PN_420) && (prate == DTMB_RATE_08))
					rtd_outl(0xB8170BA8, 0x00000004);
				else
					rtd_outl(0xB8170BA8, 0x00000014);
			}

			stime = tv_osal_time();
			pDemodKernel->Unlock(pDemodKernel);
		}
	}
#endif
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : KernelAutoTune
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_H_DTMB_KernelAutoTune(DEMOD* pDemodKernel)
{
	UINT8 LockResult = 0, locked = 0, Status = 0;
	UINT8 CarrierMode = 0, pmode = 0, prate = 0, pinterleaverLength = 0, Pnmode = 0, PnVariable = 0;
	static unsigned long stime = 0;
	static unsigned long UnlockStime = 0, Checktime = 0;
	static unsigned char UnlockHappened = 0;
	unsigned long cur_time;
	REALTEK_H_DTMB* pRealtekHDTMB;
	TV_SIG_QUAL Quality;

	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);

	cur_time = tv_osal_time();

	if (pRealtekHDTMB->m_acquire_sig_en) {

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
		REALTEK_H_DTMB_AcquireSignalThread(pDemodKernel);
		//pRealtekADVBTX->m_autotune_stime = tv_osal_time() + AUTODETECT_SIG_UPDATE_INTERVAL_MS;
#endif
	}

	if (pRealtekHDTMB->m_auto_tune_enable) {
		pDemodKernel->Lock(pDemodKernel);
		DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &Status);
		pDemodKernel->Unlock(pDemodKernel);

		if ((LockResult != 1) && (UnlockHappened == 0) && (pRealtekHDTMB->m_auto_tune_reset == 1)) {
			UnlockStime = tv_osal_time();
			UnlockHappened = 1;
			pRealtekHDTMB->m_auto_tune_reset = 0;
		} else if ((LockResult == 1) && ((Status == 0x86) || (Status == 0x87) || (Status == 0x8e) || (Status == 0x8f))) {
			//REALTEK_H_DTMB_INFO("\033[1;32;31m" "Autotune - Lock\n" "\033[m");
			if (LockResult == YES) {
				DTMBIP_GetParameters(&CarrierMode, &pmode, &prate, &pinterleaverLength, &Pnmode, &PnVariable);
				//REALTEK_H_DTMB_INFO("[Auto_tune_1]CarrierMode = %d, pmode = %d, Pnmode = %d, prate =%d, PnVariable=%d\n", CarrierMode, pmode, Pnmode, prate, PnVariable);
				if ((CarrierMode == DTMB_CARRIER_MULTI) && (pmode == DTMB_DTMB_32QAM) && (Pnmode == DTMB_PN_420) && (prate == DTMB_RATE_08) && (PnVariable == DTMB_PN_FIXED)) {
					REALTEK_H_DTMB_GetSignalQuality(pDemodKernel, TV_QUAL_PEC, &Quality);
					RealtekHDtmbModeChange();
					if ((Quality.layer_per.per_num >= 1000) && (tv_osal_time() - gPEstime >= 5000)) {
						DTMBIP_WriteRegister(0x0009, 0x10);
						tv_osal_msleep(10);
						DTMBIP_WriteRegister(0x0009, 0x00);
						REALTEK_H_DTMB_INFO("\033[1;32;31m" "GZ482 SW RESET\n" "\033[m");
						gPEstime = tv_osal_time();
					}
				} else {
					RealtekHDtmbModeDefault();
					if ((CarrierMode != DTMB_CARRIER_SINGLE) && (pmode != DTMB_DTMB_16QAM) && (Pnmode != DTMB_PN_595) && (prate != DTMB_RATE_08)) {
						DTMBIP_WriteRegister(0x0250, 0x0000000a);	//0x0250 CS642also
						DTMBIP_WriteRegister(0x0316, 0x00000014);	//0x0316 CS642also
						DTMBIP_WriteRegister(0x02E0, 0x00000090);	//0x02e0 CS642also
					}
				}
			}
			UnlockStime = tv_osal_time();
			UnlockHappened = 0;
		}

		if (((LockResult != 1) && (Status >= 0x86)) && (tv_osal_time() - UnlockStime >= 1000) && (tv_osal_time() - stime >= 2550)) {
			REALTEK_H_DTMB_DBG("\033[1;32;31m" "Autotune - Acquire\n" "\033[m");
			//REALTEK_H_DTMB_INFO("AUTOTUNE tv_osal_time() - UnlockStime = %d\n", tv_osal_time() - UnlockStime);
			//REALTEK_H_DTMB_INFO("AUTOTUNE tv_osal_time() - stime = %d\n", tv_osal_time() - stime);

			pDemodKernel->Lock(pDemodKernel);
			DTMBIP_SetWorkMode();
			DTMBIP_SetAutoMode();
			DTMBIP_SoftReset();

			Checktime = tv_osal_time();
			while (tv_osal_time() - Checktime < 250) {
				tv_osal_msleep(5);
				DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &Status);
				if (Status >= 0x86) {
					// REALTEK_H_DTMB_INFO("\033[1;32;32m" "auto Checktime = %lu\n" "\033[m",tv_osal_time() - Checktime);
					break;
				}
			}

			DTMBIP_GetParameters(&CarrierMode, &pmode, &prate, &pinterleaverLength, &Pnmode, &PnVariable);  //check Chengsha SC
			tv_osal_msleep(1);
			REALTEK_H_DTMB_DBG("[Kernel_Auto_1]CarrierMode = %d, pmode = %d, Pnmode = %d, prate =%d\n", CarrierMode, pmode, Pnmode, prate);
			if ((CarrierMode == DTMB_CARRIER_SINGLE) && (pmode == DTMB_DTMB_16QAM) && (Pnmode == DTMB_PN_595) && (prate == DTMB_RATE_08)) {
				DTMBIP_WriteRegister(0x02E0, 0x00000035);//0x2e0
				DTMBIP_WriteRegister(0x02D3, 0x00000040);//0x2d3

				DTMBIP_WriteRegister(0x0250, 0x00000006);//0x250
				DTMBIP_WriteRegister(0x0316, 0x0000000A);//0x316
				tv_osal_msleep(1);
				REALTEK_H_DTMB_DBG("Change Register!\n");
			} else {
				DTMBIP_WriteRegister(0x02E0, 0x00000090);//0x2e0
				DTMBIP_WriteRegister(0x02D3, 0x00000010);//0x2d3

				DTMBIP_WriteRegister(0x0250, 0x0000000A);//0x250
				DTMBIP_WriteRegister(0x0316, 0x00000014);//0x316
				tv_osal_msleep(1);
			}

			DTMBIP_IsDemodLocked(TRUE, &LockResult, &locked, &Status);
			if (LockResult == YES) {
				DTMBIP_GetParameters(&CarrierMode, &pmode, &prate, &pinterleaverLength, &Pnmode, &PnVariable);  //check Guangzhou MC
				//REALTEK_H_DTMB_INFO("[Auto_tune_2]CarrierMode = %d, pmode = %d, Pnmode = %d, prate =%d, PnVariable =%d\n", CarrierMode, pmode, Pnmode, prate, PnVariable);
				if ((CarrierMode == DTMB_CARRIER_MULTI) && (pmode == DTMB_DTMB_32QAM) && (Pnmode == DTMB_PN_420) && (prate == DTMB_RATE_08) && (PnVariable == DTMB_PN_FIXED)) { //gz482
					RealtekHDtmbModeChange();
				} else {
					RealtekHDtmbModeDefault();
					if ((CarrierMode != DTMB_CARRIER_SINGLE) && (pmode != DTMB_DTMB_16QAM) && (Pnmode != DTMB_PN_595) && (prate != DTMB_RATE_08)) {
						DTMBIP_WriteRegister(0x0250, 0x0000000a);	//0x0250 CS642also
						DTMBIP_WriteRegister(0x0316, 0x00000014);	//0x0316 CS642also
						DTMBIP_WriteRegister(0x02E0, 0x00000090);	//0x02e0 CS642also
					}
				}
			}

			stime = tv_osal_time();
			pDemodKernel->Unlock(pDemodKernel);
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
int REALTEK_H_DTMB_InspectSignal(DEMOD* pDemodKernel)
{

	return TUNER_CTRL_OK;

#if 0

	int ret = TUNER_CTRL_OK;

	unsigned CrOffsetHz = 0;
	unsigned char SignalLock = 0;
	unsigned Ber = 1;
	unsigned Per = 1;
	unsigned Snr = 1;

	REALTEK_H_DTMB* pRealtekHDTMB;
	DTMB_DEMOD_MODULE* pDemod;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);
	//pDemod = ((REALTEK_H_DTMB_DRIVER_DATA*) pRealtekHDTMB->m_private)->pDemod;
	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_InspectSignal\n" "\033[m");

	return TUNER_CTRL_OK;

	if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DTMB))
		return 0;

	pDemodKernel->Lock(pDemodKernel);

	CrOffsetHz = 0;
	pDemodKernel->GetLockStatus(pDemodKernel, &SignalLock);
	DTMBIP_GetSignalBER(&Ber);
	Per = 0;
	DTMBIP_GetSignalSNR(&Snr);
	pDemodKernel->Unlock(pDemodKernel);


	ret = snprintf(pBuff, BufferSize, "lock(signal)=%d, offset(cr)="PT_S32BITS"  error_rate(ber/per)=%f/%f, snr=%f\n",
				   SignalLock,
				   CrOffsetHz,
				   Ber,
				   Per,
				   Snr);


	return ret;
#endif

}



/*----------------------------------------------------------------------
 * Func : Activate
 *
 * Desc : Activate REALTEK_H_ QAM Demod
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_H_DTMB_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{

	return TUNER_CTRL_OK;
}

/*----------------------------------------------------------------------
 * Func : SetTvMode
 *
 * Desc : Set Mode of REALTEK_H_ALL
 *
 * Parm : mode :    RTK_DEMOD_MODE_DTMB : DTMB mode
 *                  RTK_DEMOD_MODE_DVBC: DVBC mode
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_H_DTMB_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;
	REALTEK_H_DTMB* pRealtekHDTMB;
	pRealtekHDTMB = (REALTEK_H_DTMB*)(pDemodKernel->private_data);

	//REALTEK_H_DTMB_INFO("\033[1;32;32m""Demod REALTEK_H_DTMB_SetTvMode\n" "\033[m");


	REALTEK_H_DTMB_DBG("REALTEK_H_DTMB::SetTvMode(%d)\n", mode);

	switch (mode) {

	case TV_SYS_TYPE_DTMB:

		REALTEK_H_DTMB_DBG("Set to DTMB\n");
		ret = pDemodKernel->Init(pDemodKernel);
		ret |= pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts);

		if (ret != TUNER_CTRL_OK) {
			DEMOD_WARNING("Set REALTEK_H_DTMB to DTMB mode failed\n");
			goto set_demod_mode_failed;
		}

		break;

	default:
		goto set_demod_mode_failed;
	}

	ret = TUNER_CTRL_OK;
	REALTEK_H_DTMB_DBG("Set REALTEK_H_DTMB to appointed DTV mode successed\n");


//end_proc:

	return ret;

//--------------
set_demod_mode_failed:

	return TUNER_CTRL_FAIL;
}

void REALTEK_H_DTMB_Lock(DEMOD* pDemodKernel)
{
	//mutex_lock(&pDemodKernel->m_lock);
}

void REALTEK_H_DTMB_Unlock(DEMOD* pDemodKernel)
{
	//mutex_unlock(&pDemodKernel->m_lock);
}



/*-----------------------------------------------------------------
 *     BASE_INTERFACE for REALTEK_H_DTMB API
 *----------------------------------------------------------------*/


void ReleaseRealtekHDtmbDriver(REALTEK_H_DTMB_DRIVER_DATA *pDriver)
{
	kfree(pDriver);
}

/*-----------------------------------------------------------------
 *   CHINA FIELD STREAM for REALTEK_H_DTMB API
 *----------------------------------------------------------------*/
int RealtekHDtmbModeChange(void)
{
	//for GZ482 1129_1
	DTMBIP_WriteRegister(0x02D5, 0x00000006);	//0x02d5
	DTMBIP_WriteRegister(0x02DA, 0x00000040);	//0x02da<=40
	DTMBIP_WriteRegister(0x028B, 0x000000ff);	//0x028b
	DTMBIP_WriteRegister(0x0250, 0x00000001);	//0x0250
	DTMBIP_WriteRegister(0x0316, 0x0000001c);	//0x0316
	DTMBIP_WriteRegister(0x02DD, 0x00000010);	//0x02dd
	DTMBIP_WriteRegister(0x0243, 0x00000001);	//0x0243
	DTMBIP_WriteRegister(0x0244, 0x00000009);	//0x0244
	DTMBIP_WriteRegister(0x0245, 0x00000060);	//0x0245
	DTMBIP_WriteRegister(0x0246, 0x000000f8);	//0x0246
	DTMBIP_WriteRegister(0x0247, 0x00000008);	//0x0247
	DTMBIP_WriteRegister(0x02EA, 0x0000000f);	//0x02ea

	DTMBIP_WriteRegister(0x0300, 0x00000001);	//0x0300<=01
	DTMBIP_WriteRegister(0x0227, 0x00000018);	//0x0227<=18
	DTMBIP_WriteRegister(0x02E0, 0x000000c0);	//0x02e0<=90

	//REALTEK_H_DTMB_INFO("SetModeChange!!\n");
	tv_osal_msleep(5);
	//DTMBIP_SoftReset();

	return 0;
}

int RealtekHDtmbModeDefault(void)
{
	DTMBIP_WriteRegister(0x02D5, 0x00000066);	//0x02d5    For default

	DTMBIP_WriteRegister(0x02DA, 0x00000010);	//0x02da

	DTMBIP_WriteRegister(0x028B, 0x00000050);	//0x028b
	//rtd_outl(0xB8170940, 0x0000000a);	//0x0250 CS642also
	//rtd_outl(0xB8170C58, 0x00000014);	//0x0316 CS642also
	DTMBIP_WriteRegister(0x02DD, 0x00000002);	//0x02dd
	DTMBIP_WriteRegister(0x0243, 0x00000002);	//0x0243
	DTMBIP_WriteRegister(0x0244, 0x00000007);	//0x0244
	DTMBIP_WriteRegister(0x0245, 0x00000040);	//0x0245
	DTMBIP_WriteRegister(0x0246, 0x00000080);	//0x0246

	DTMBIP_WriteRegister(0x0247, 0x0000000A);	//0x0247

	DTMBIP_WriteRegister(0x02EA, 0x00000014);	//0x02ea

	DTMBIP_WriteRegister(0x0300, 0x00000008);	//0x0300
	DTMBIP_WriteRegister(0x0227, 0x00000004);	//0x0227
	//rtd_outl(0xB8170B80, 0x00000090);	//0x02e0 CS642also

	//REALTEK_H_DTMB_INFO("SetModeDefault!\n");
	tv_osal_msleep(5);
	//DTMBIP_SoftReset();

	return 0;
}





REALTEK_H_DTMB_DRIVER_DATA* AllocRealtekHDtmbDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
)
{
	REALTEK_H_DTMB_DRIVER_DATA* pDriver = (REALTEK_H_DTMB_DRIVER_DATA*) kmalloc(
			sizeof(REALTEK_H_DTMB_DRIVER_DATA) +
			sizeof(DTMB_DEMOD_MODULE)   +
			sizeof(I2C_BRIDGE_MODULE)   +
			sizeof(BASE_INTERFACE_MODULE), GFP_KERNEL);

	if (pDriver) {
		BASE_INTERFACE_MODULE* pBIF;
		memset(pDriver, 0, sizeof(REALTEK_H_DTMB_DRIVER_DATA));

		pDriver->pDemod                = (DTMB_DEMOD_MODULE*)(((unsigned char*)pDriver) + sizeof(REALTEK_H_DTMB_DRIVER_DATA));
		pDriver->pBaseInterface        = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(DTMB_DEMOD_MODULE));
		pDriver->pI2CBridge            = (I2C_BRIDGE_MODULE*)(((unsigned char*)pDriver->pBaseInterface) + sizeof(BASE_INTERFACE_MODULE));
		pDriver->DeviceAddr            = Addr;
		pDriver->CrystalFreqHz         = CrystalFreq;

		// Init Base IF
		pBIF = pDriver->pBaseInterface;

		pBIF->I2cReadingByteNumMax      = RTK_DEMOD_BIF_TX_FIFO_DEPTH;
		pBIF->I2cWritingByteNumMax      = RTK_DEMOD_BIF_RX_FIFO_DEPTH;
		pBIF->I2cRead                   = __realtek_i2c_read;
		pBIF->I2cWrite                  = __realtek_i2c_write;
		pBIF->WaitMs                    = __realtek_wait_ms;
		pBIF->SetUserDefinedDataPointer = base_interface_SetUserDefinedDataPointer;
		pBIF->GetUserDefinedDataPointer = base_interface_GetUserDefinedDataPointer;
		pBIF->SetUserDefinedDataPointer(pBIF, (void*)pComm);

		// Init Demod Driver
	}

	return pDriver;
}
