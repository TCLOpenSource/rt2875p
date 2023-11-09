/***************************************************************************************************
  File        : demod_rtk_r_atsc.cpp
  Description : REALTEK_R_ATSC demod
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
#include <rtk_kdriver/io.h>
#include "demodcore/foundation.h"
#include "demod_rtk_r_atsc.h"
#include "demod_rtk_r_atsc_priv.h"
//#include "rbus/dtv_demod_misc_reg.h"


//#define REALTEK_R_EXT_QFN
#define REALTEK_R_ATSC_8051FW_PATCH
//#define REALTEK_R_ATSC_LGE_STREAM_PATCH_I37
//#define REALTEK_R_ATSC_SetIF_LGE
//#define REALTEK_R_ATSC_CCI_PATCH_LGE

unsigned char IsATSC3McmPkgFlag = 0;

/*----------------------------------------------------------------------
 * Func : REALTEK_R_ATSC
 *
 * Desc : constructor
 *
 * Parm : device_addr : device address
 *        output_mode : REALTEK_R_ATSC_OUT_IF_SERIAL / REALTEK_R_ATSC_OUT_IF_PARALLEL
 *        output_freq : REALTEK_R_ATSC_OUT_FREQ_6M / REALTEK_R_ATSC_OUT_FREQ_10M
 *        pComm       : hadle of COMM
 *
 * Retn : handle of demod
 *----------------------------------------------------------------------*/
void REALTEK_R_ATSC_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm)
{
	REALTEK_R_ATSC* pRealtekRATSC;

	pDemodKernel->private_data = kmalloc(sizeof(REALTEK_R_ATSC), GFP_KERNEL);
	if(pDemodKernel->private_data == NULL){
		goto kmalloc_fail;
	}

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	Demod_Constructors(pDemodKernel);

	pDemodKernel->m_id                 = DTV_DEMOD_REALTEK_R_ATSC;
	pDemodKernel->m_addr               = addr;
	pDemodKernel->m_pTuner             = NULL;
	pDemodKernel->m_pComm              = pComm;
	pDemodKernel->m_ScanRange          = SCAN_RANGE_2_6;
	pDemodKernel->m_ScanMode           = CH_UNKNOW;
	pDemodKernel->m_Capability         = TV_SYS_TYPE_ATSC;
	pDemodKernel->m_update_interval_ms = 1000;
	pDemodKernel->m_if.freq            = 36125000;
	pDemodKernel->m_if.inversion       = 1;
	pDemodKernel->m_if.agc_pola        = 0;
	pDemodKernel->m_clock              = CRYSTAL_FREQ_27000000HZ;
	pRealtekRATSC->m_output_freq        = output_freq;
	pRealtekRATSC->m_private            = (void*) AllocRealtekRAtscDriver(
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
	pRealtekRATSC->m_low_snr_detect     = 0;
	pRealtekRATSC->m_low_snr_recover    = 0;
	pRealtekRATSC->m_auto_tune_enable   = 0;
	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "REALTEK_R_ATSC");
	pRealtekRATSC->m_pTunerOptReg       = NULL;
	pRealtekRATSC->m_TunerOptRegCnt     = 0;
	pRealtekRATSC->m_patch_ind = 0;
	pRealtekRATSC->m_patch_cr2_ind = 0;
	pRealtekRATSC->m_acquire_sig_en = 0;
	pDemodKernel->Init = REALTEK_R_ATSC_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = REALTEK_R_ATSC_AcquireSignal;
	pDemodKernel->ScanSignal = REALTEK_R_ATSC_ScanSignal;
	pDemodKernel->SetIF = REALTEK_R_ATSC_SetIF;
	pDemodKernel->SetTSMode = REALTEK_R_ATSC_SetTSMode;
	pDemodKernel->SetTVSysEx = REALTEK_R_ATSC_SetTVSysEx;
	pDemodKernel->SetMode = REALTEK_R_ATSC_SetMode;
	pDemodKernel->SetTVSys = REALTEK_R_ATSC_SetTVSys;
	pDemodKernel->GetLockStatus = REALTEK_R_ATSC_GetLockStatus;
	pDemodKernel->GetSegmentNum = Demod_GetSegmentNum;
	pDemodKernel->GetSignalInfo = REALTEK_R_ATSC_GetSignalInfo;
	pDemodKernel->GetSignalQuality = REALTEK_R_ATSC_GetSignalQuality;
	pDemodKernel->ChangeT2PLP = Demod_ChangeT2PLP;
	pDemodKernel->GetT2PLPInfo = Demod_GetT2PLPInfo;
	pDemodKernel->GetT2L1PreInfo = Demod_GetT2L1PreInfo;
	pDemodKernel->GetT2L1PostInfo = Demod_GetT2L1PostInfo;
	pDemodKernel->GetDemodStatus = Demod_GetDemodStatus;
	pDemodKernel->GetCarrierOffset = REALTEK_R_ATSC_GetCarrierOffset;
	pDemodKernel->ForwardI2C = REALTEK_R_ATSC_ForwardI2C;
	pDemodKernel->AutoTune = REALTEK_R_ATSC_AutoTune;
	pDemodKernel->KernelAutoTune = REALTEK_R_ATSC_KernelAutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = Demod_Suspend;
	pDemodKernel->Resume = Demod_Resume;
	pDemodKernel->SetTvMode = REALTEK_R_ATSC_SetTvMode;
	pDemodKernel->DebugInfo = REALTEK_R_ATSC_DebugInfo;	  	  // Add monitor function that used to monitor demod status
	pDemodKernel->InspectSignal = REALTEK_R_ATSC_InspectSignal; // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
	//pDemodKernel->Lock = REALTEK_R_ATSC_Lock;
	//pDemodKernel->Unlock = REALTEK_R_ATSC_Unlock;
	pRealtekRATSC->Lock = REALTEK_R_ATSC_Lock;
	pRealtekRATSC->Unlock = REALTEK_R_ATSC_Unlock;
	pRealtekRATSC->Activate = REALTEK_R_ATSC_Activate;
	pRealtekRATSC->Destory = REALTEK_R_ATSC_Destructors;

	mutex_init(&pRealtekRATSC->m_lock);

kmalloc_fail:
	REALTEK_R_ATSC_WARNING("REALTEK_R_ATSC_Constructors kmalloc Failed\n");
}


/*----------------------------------------------------------------------
 * Func : ~REALTEK_R_ATSC
 *
 * Desc : Destructor of REALTEK_R_ATSC
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void REALTEK_R_ATSC_Destructors(DEMOD* pDemodKernel)
{
	REALTEK_R_ATSC* pRealtekRATSC;
	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	mutex_destroy(&pRealtekRATSC->m_lock);

	if (pRealtekRATSC->m_private != NULL)
		ReleaseRealtekRAtscDriver((REALTEK_R_ATSC_DRIVER_DATA*)pRealtekRATSC->m_private);

	kfree(pRealtekRATSC);
	//pDemodKernel->Destory(pDemodKernel);
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
int REALTEK_R_ATSC_Init(DEMOD* pDemodKernel)
{
	int i;
	//unsigned int AllocateSize = 0;
	//unsigned long ShareMemoryPhysicalAddress;
	
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;
	BASE_INTERFACE_MODULE *pBaseInterface;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;
	pBaseInterface = pDemod->pBaseInterface;

	pRealtekRATSC->Lock(pDemodKernel);
	pDemodKernel->m_ScanMode = CH_UNKNOW;

	if (IsAtsc3McmPackage(RTK_DEMOD_MODE_ATSC) == 0) {
		IsATSC3McmPkgFlag = 1;
		
		DtvDemodInitialization(RTK_DEMOD_MODE_ATSC, 1);
		tv_osal_msleep(10);                             //20190312 ATSC delay Added 10mS
		REALTEK_R_ATSC_INFO("Init ATSC 1.0 Demod by SOC Flow\n");

/*
		AllocateSize = CravedoutMemoryQuery(&ShareMemoryPhysicalAddress);

		if (AllocateSize < RTK_R_ATSC1_DDR_SIZE) {
			REALTEK_R_ATSC_WARNING("\033[0;32;31m" "Allocate Share Memory (Non-MCM ATSC 1.0) - Allocate DDR Size is Not Enough!!!\n" "\033[m");
			goto init_fail;
		}

		REALTEK_R_ATSC_INFO("Allocate Share Memory (Non-MCM ATSC 1.0) - Physical_Addr = 0x%08lx, Size = 0x%08x\n", ShareMemoryPhysicalAddress, AllocateSize);

		rtd_outl(DTV_DEMOD_MISC_ATSC_ITER_OUTER_DMA_OFFSET_reg, ShareMemoryPhysicalAddress);
*/

	} else {
		IsATSC3McmPkgFlag = 2;
		
		pBaseInterface->I2cRead  = __realtek_i2c_read_speedF;
		pBaseInterface->I2cWrite  = __realtek_i2c_write_speedF;

		ExtAtsc3DemodInitialization(RTK_DEMOD_MODE_ATSC, 1);
		//REALTEK_R_Ext_ATSC_I2C_Init(pDemodKernel);

		//pBaseInterface->I2cRead  = __realtek_i2c_read_speedF;
		//pBaseInterface->I2cWrite  = __realtek_i2c_write_speedF;

		REALTEK_R_Ext_ATSC_System_Init(pDemodKernel, 1);
		REALTEK_R_Ext_ATSC_BisrCheck(pDemodKernel);
		tv_osal_msleep(10);                             //20190312 ATSC delay Added 10mS

		pBaseInterface->I2cRead  = __realtek_rbus_i2c_read;
		pBaseInterface->I2cWrite  = __realtek_rbus_i2c_write;
		REALTEK_R_ATSC_INFO("Init ATSC 1.0 Demod by MCM or QFN Flow\n");
	}

	pRealtekRATSC->m_patch_ind = 0;
	pRealtekRATSC->m_patch_cr2_ind = 0;

	// check tuner information
	if (pDemodKernel->m_pTuner) {
		TUNER_PARAM Param;

		if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, TV_SYS_ATSC, &Param) == TUNER_CTRL_OK) {
			switch (Param.id) {
			case TV_TUNER_SILICONLABS_SI2151:
				REALTEK_R_ATSC_INFO("Tuner id %u, TV_TUNER_SILICONLABS_SI2151\n", Param.id);
				pRealtekRATSC->m_pTunerOptReg = (REG_VALUE_ENTRY*) SI2151_OPT_VALUE_ATSC;
				pRealtekRATSC->m_TunerOptRegCnt  = sizeof(SI2151_OPT_VALUE_ATSC) / sizeof(REG_VALUE_ENTRY);
				break;
			case TV_TUNER_SILICONLABS_SI2178B:
			case TV_TUNER_NO_TUNER:
				REALTEK_R_ATSC_INFO("Tuner id %u, TV_TUNER_SILICONLABS_SI2178B\n", Param.id);
				pRealtekRATSC->m_pTunerOptReg = (REG_VALUE_ENTRY*) SI2178B_OPT_VALUE_ATSC;
				pRealtekRATSC->m_TunerOptRegCnt  = sizeof(SI2178B_OPT_VALUE_ATSC) / sizeof(REG_VALUE_ENTRY);
				break;
			case TV_TUNER_MAXLINEAR_MXL661:
				REALTEK_R_ATSC_INFO("Tuner id %u, TV_TUNER_MAXLINEAR_MXL661\n", Param.id);
				pRealtekRATSC->m_pTunerOptReg = (REG_VALUE_ENTRY*) MXL661_OPT_VALUE_ATSC;
				pRealtekRATSC->m_TunerOptRegCnt  = sizeof(MXL661_OPT_VALUE_ATSC) / sizeof(REG_VALUE_ENTRY);
				break;
			case TV_TUNER_NXP_TDA18275A:
				REALTEK_R_ATSC_INFO("Tuner id %u, TV_TUNER_NXP_TDA18275A\n", Param.id);
				break;
			case TV_TUNER_RAFAEL_R840:
			case TV_TUNER_RAFAEL_R842:
				REALTEK_R_ATSC_INFO("Tuner id %u, TV_TUNER_RAFAEL_R84x\n", Param.id);
				pRealtekRATSC->m_pTunerOptReg = (REG_VALUE_ENTRY*) R84x_OPT_VALUE_ATSC;
				pRealtekRATSC->m_TunerOptRegCnt  = sizeof(R84x_OPT_VALUE_ATSC) / sizeof(REG_VALUE_ENTRY);
				break;
			case TV_TUNER_SONY_SUTRX243:
			case TV_TUNER_SONY_SUTRX243_RA:
				REALTEK_R_ATSC_INFO("Tuner id %u, TV_TUNER_SONY_SUTRX243\n", Param.id);
				pRealtekRATSC->m_pTunerOptReg = (REG_VALUE_ENTRY*) SUTRX243_OPT_VALUE_ATSC;
				pRealtekRATSC->m_TunerOptRegCnt  = sizeof(SUTRX243_OPT_VALUE_ATSC) / sizeof(REG_VALUE_ENTRY);
				break;
			default:
				REALTEK_R_ATSC_INFO("unknown tuner id %u, using default setting\n", Param.id);
				pRealtekRATSC->m_pTunerOptReg = (REG_VALUE_ENTRY*) DEFAULT_OPT_VALUE_ATSC;
				pRealtekRATSC->m_TunerOptRegCnt  = sizeof(DEFAULT_OPT_VALUE_ATSC) / sizeof(REG_VALUE_ENTRY);
				break;
			}
			pDemodKernel->m_if.freq      = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola  = Param.if_agc_pola;

			REALTEK_R_ATSC_INFO("if_freq = "PT_U32BITS", if_inversion = %u, if_agc_pola = %u\n",
								pDemodKernel->m_if.freq, pDemodKernel->m_if.inversion, pDemodKernel->m_if.agc_pola);
		}
	}

	// Initial Driver
	if (pDemod->Initialize(pDemod) != FUNCTION_SUCCESS) {
		pRealtekRATSC->Unlock(pDemodKernel);
		goto init_fail;
	}

	ClearDemodReset(RTK_DEMOD_MODE_ATSC);

	// Run Optimize Configuration for specified tuner
	for (i = 0; i < pRealtekRATSC->m_TunerOptRegCnt; i++) {
		int Reg = pRealtekRATSC->m_pTunerOptReg[i].RegBitName;
		U32BITS Val = pRealtekRATSC->m_pTunerOptReg[i].Value;

		if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, Reg, Val) != FUNCTION_SUCCESS) {
			pRealtekRATSC->Unlock(pDemodKernel);
			goto init_fail;
	}
	}

	pRealtekRATSC->Unlock(pDemodKernel);

	if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
		goto init_fail;

	if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK)
		goto init_fail;

	REALTEK_R_ATSC_INFO("REALTEK_R_ATSC Init Complete\n");
	return TUNER_CTRL_OK;

init_fail:
	REALTEK_R_ATSC_WARNING("REALTEK_R_ATSC Init Fail\n");
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
int REALTEK_R_ATSC_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	REALTEK_R_ATSC* pRealtekRATSC;
	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	REALTEK_R_ATSC_INFO("\033[1;32;31m" "%s %s %d, system = %d\n" "\033[m", __FILE__, __func__, __LINE__, sys);
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
int REALTEK_R_ATSC_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO*  pSigInfo)
{
	CH_MODE AtscScanMode;
	TUNER_PARAM Param;
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	if (pSigInfo != NULL)
		AtscScanMode = pSigInfo->vsb.scan_mode ? CH_SCAN_MODE : CH_NORMAL_MODE;
	else
		AtscScanMode = CH_NORMAL_MODE;

	REALTEK_R_ATSC_INFO("\033[1;32;31m" "%s %s %d, system = %d, AtscScanMode = %d\n" "\033[m", __FILE__, __func__, __LINE__, sys, AtscScanMode);

	if (!IsAtscSys(sys)) {
		REALTEK_R_ATSC_WARNING("Set TV system failed, unsupported TV system\n");

		return  TUNER_CTRL_FAIL;
	}
	pDemodKernel->m_tv_sys = sys;
	pRealtekRATSC->m_patch_ind = 0;
	pRealtekRATSC->m_patch_cr2_ind = 0;

	if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, pDemodKernel->m_tv_sys, &Param) == TUNER_CTRL_OK) {
		if ((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola != Param.if_agc_pola)) {
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;

			if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK) {
				return  TUNER_CTRL_FAIL;
			}
		}
	}

	pRealtekRATSC->Lock(pDemodKernel);

	REALTEK_R_ATSC_TPOutEn(pDemodKernel, 0); //Disable Demod TS Output

	if (IsATSC3McmPkgFlag == 2) {
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_CR2_CARB_WB, 2);
	}

	if (AtscScanMode == CH_SCAN_MODE) {
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DW_8051_RST, 0); //reset 8051
	#ifdef REALTEK_R_ATSC_CCI_PATCH_LGE
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CCI_REJ_EN, 0);
	#endif
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_FMU_UPD_CNT_TH, 1);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_LPF_SHIFT, 537);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_PMU_COMP1, 0);	
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_TR_FS_PLL_KP_EXP, 2);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_ROW, 32);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 14);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 207);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 15);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 218);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_ROW, 33);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 14);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 207);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 15);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 218);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_ROW, 34);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 14);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 207);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 15);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 217);
	} else {
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DW_8051_RST, 0); //reset 8051
	#ifdef REALTEK_R_ATSC_CCI_PATCH_LGE
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CCI_REJ_EN, 1);
	#endif
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_FMU_UPD_CNT_TH, 1);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_LPF_SHIFT, 537);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_PMU_COMP1, 0);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_TR_FS_PLL_KP_EXP, 5);	
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_ROW, 32);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 14);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 207);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 15);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 218);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_ROW, 33);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 14);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 207);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 15);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 218);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_ROW, 34);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 14);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 207);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_COL, 15);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FSM_RAM_DATA, 217);
		
#ifdef REALTEK_R_ATSC_8051FW_PATCH
		//Set dummy register for channel change indicator
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, Dummy_CHANGE_CHANNEL_IND, 1);
#endif

		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, DW_8051_RST, 1);
	}

#ifdef REALTEK_R_ATSC_LGE_STREAM_PATCH_I37
	pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_MSE_FIELD_TH_H, 0x0);
	pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_MSE_FIELD_TH_L, 0x2fa0000);
#endif

	pDemodKernel->m_ScanMode = AtscScanMode;

	if (pDemodKernel->m_ScanMode == CH_SCAN_MODE) {
		REALTEK_R_ATSC_TPOutEn(pDemodKernel, 1); //Enable Demod TS Output
	}

	pDemod->SoftwareReset(pDemod);   //Restart demod FSM
	pRealtekRATSC->m_acquire_sig_en = 1;
	pRealtekRATSC->Unlock(pDemodKernel);
	
	return TUNER_CTRL_OK;
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
int REALTEK_R_ATSC_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;
	pRealtekRATSC->Lock(pDemodKernel);
	
#ifdef REALTEK_R_ATSC_SetIF_LGE
	pParam->freq = 5945000;
	REALTEK_R_ATSC_INFO("REALTEK_R_ATSC_SetIF_LGE\n");
#endif

	REALTEK_R_ATSC_DBG("SetIF: IF_freq = "PT_U32BITS", spectrumn inv = %u, agc_pola = %u\n", pParam->freq, pParam->inversion, pParam->agc_pola);

	ret  = DECODE_RET(pDemod->SetIfFreqHz(pDemod, pParam->freq));
	ret |= DECODE_RET(pDemod->SetSpectrumMode(pDemod, (pParam->inversion) ? 0 : 1));
	ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_POLAR_IFAGC, pParam->agc_pola));
	ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_OPT_IF_AAGC_DRIVE, 1));   // push pull mode
	//ret |= DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_OPT_PAR_IF_SD_IB, 1));   // agc if non-invert

	ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));
	pRealtekRATSC->m_low_snr_detect     = 0;
	pRealtekRATSC->m_low_snr_recover    = 0;
	pDemodKernel->m_if = *pParam;
	pRealtekRATSC->Unlock(pDemodKernel);

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
int REALTEK_R_ATSC_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	unsigned char val;
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	switch (mode) {
	case TV_OP_MODE_NORMAL:
	case TV_OP_MODE_SCANING:
		REALTEK_R_ATSC_INFO("demod enabled\n");
		val = 1;
		pDemod->RegAccess.Addr16Bit.SetRegBytes(pDemod, 0xd004, &val, 1);  // enable ATSC demod
		pDemodKernel->AcquireSignal(pDemodKernel, 0);             		   // reset demod....
		break;
	case TV_OP_MODE_STANDBY:
		REALTEK_R_ATSC_INFO("demod disabled\n");
		val = 0;
		pDemod->RegAccess.Addr16Bit.SetRegBytes(pDemod, 0xd004, &val, 1);  // disable ATSC demod
		break;
	case TV_OP_MODE_ATV_NORMAL:
	case TV_OP_MODE_TP_ONLY:
		break;
	default:
		REALTEK_R_ATSC_WARNING("unknown TV_OP_MODE\n");
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
int REALTEK_R_ATSC_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	pRealtekRATSC->Lock(pDemodKernel);
	switch (pParam->mode) {
	case PARALLEL_MODE:
		REALTEK_R_ATSC_DBG("TS Mode: PARALLEL\n");
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_SERIAL,   0));
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CDIV_PH0, 5));
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CDIV_PH1, 5));
		break;
	case SERIAL_MODE:
		REALTEK_R_ATSC_DBG("TS Mode: SERIAL\n");
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_SERIAL,   1));
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CDIV_PH0, 0));
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CDIV_PH1, 0));
		break;
	default:
		REALTEK_R_ATSC_WARNING("unknown TS Mode\n");
		ret = TUNER_CTRL_FAIL;
		break;
	}

	switch (pParam->clk_pol) {
	case RISING_EDGE:
		REALTEK_R_ATSC_DBG("MPEG CLK POL: RISING_EDGE\n");
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CKOUTPAR, 1));
		break;
	case FALLING_EDGE:
		REALTEK_R_ATSC_DBG("MPEG CLK POL: FALLING_EDGE\n");
		ret  = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CKOUTPAR, 0));
		break;
	default:
		REALTEK_R_ATSC_WARNING("unknown MPEG CLK POL\n");
		ret = TUNER_CTRL_FAIL;
		break;
	}

	REALTEK_R_ATSC_TPOutEn(pDemodKernel, pParam->tsOutEn);

	if (ret == TUNER_CTRL_OK)
		pDemodKernel->m_ts = *pParam;

	pRealtekRATSC->Unlock(pDemodKernel);

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
int REALTEK_R_ATSC_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{
	int ret = TUNER_CTRL_OK;
	int FrameLock;

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	//if (DtvIsDemodClockEnable(RTK_DEMOD_MODE_ATSC) == 0)
	//	return TUNER_CTRL_FAIL;

	pRealtekRATSC->Lock(pDemodKernel);
	ret = DECODE_RET(pDemod->IsFrameLocked(pDemod, &FrameLock));

	if (ret == TUNER_CTRL_OK)
		*pLock = DECODE_LOCK(FrameLock);


	pRealtekRATSC->Unlock(pDemodKernel);

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
int REALTEK_R_ATSC_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	REALTEK_R_ATSC*	pRealtekRATSC;
	ATSC_DEMOD_MODULE*	pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	pInfo->mod = TV_MODULATION_VSB;
	pInfo->vsb.level = 8;
	REALTEK_R_ATSC_DBG("GetSignalInfo: level = %u\n", pInfo->vsb.level);

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
int REALTEK_R_ATSC_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality)
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
	
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	//if (DtvIsDemodClockEnable(RTK_DEMOD_MODE_ATSC) == 0)
	//	return TUNER_CTRL_FAIL;

	pDemodKernel->GetLockStatus(pDemodKernel, &FrameLock);

	pRealtekRATSC->Lock(pDemodKernel);
	
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
		ret = (FrameLock) ? DECODE_RET(pDemod->GetErrorRate(pDemod, 5, 1000, &BerNum, &BerDen, &PerNum, &PerDen)) : TUNER_CTRL_OK;
		REALTEK_R_ATSC_DBG("BerNum = "PT_U32BITS", BerDen = "PT_U32BITS", PerNum = "PT_U32BITS", PerDen = "PT_U32BITS"\n", BerNum, BerDen, PerNum, PerDen);

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
		pRealtekRATSC->Unlock(pDemodKernel);
		ret = pDemodKernel->GetSignalQuality(pDemodKernel, TV_QUAL_SNR, pQuality);
		pRealtekRATSC->Lock(pDemodKernel);

		if (ret == TUNER_CTRL_OK)
			pQuality->strength = Snr2Str(pQuality->snr / 10000); //Convert SNR to Signal Strength
			
		#else
		ret = DECODE_RET(pDemod->GetSignalStrength(pDemod, TxRSSIdBm, &SignalStrength));
		if (ret == TUNER_CTRL_OK)
			pQuality->strength = (unsigned char) SignalStrength;
		#endif
		
		REALTEK_R_ATSC_DBG("TxRSSIdBm = %d, strength = %u\n", TxRSSIdBm, pQuality->strength);
		break;

	case TV_QUAL_SIGNAL_QUALITY:
		pRealtekRATSC->Unlock(pDemodKernel);
		ret = pDemodKernel->GetSignalQuality(pDemodKernel, TV_QUAL_BER, pQuality);
		pRealtekRATSC->Lock(pDemodKernel);

		if (ret == TUNER_CTRL_OK)
			pQuality->quality = Ber2Quality(pQuality->ber);  	//Convert BER to Signal Quality
		break;

	case TV_QUAL_AGC:
		ret = pDemod->GetIfAgc(pDemod, &IfAgc);

		if (ret == TUNER_CTRL_OK)
			pQuality->agc = ((IfAgc + 8191) * 100) / 16384;
		break;

	default:
		REALTEK_R_ATSC_WARNING("unknown ENUM_TV_QUAL id\n");
		ret = TUNER_CTRL_FAIL;
		break;
	}

	pRealtekRATSC->Unlock(pDemodKernel);
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
int REALTEK_R_ATSC_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	int ret = TUNER_CTRL_OK;
	unsigned char LockStatus = 0;

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	pDemodKernel->GetLockStatus(pDemodKernel, &LockStatus);

	if (LockStatus == 0) {
		REALTEK_R_ATSC_INFO("GetCarrierOffset fail beacause demod is unlock\n");
		return TUNER_CTRL_FAIL;
	}

	pRealtekRATSC->Lock(pDemodKernel);
	ret = DECODE_RET(pDemod->GetCrOffsetHz(pDemod, pOffset));

	if (ret == TUNER_CTRL_OK) {
		/*-----------------------------------------------------------------
		  CrOffset = Current Frequency - Target Frequency
		  returned offset = Target Freq - Current Offset = -CrOffset
		  Note: REALTEK_R ATSC Demod will handle IF polarity automatically
		------------------------------------------------------------------*/
		*pOffset *= -1;
	} else
		*pOffset = 0;

	pRealtekRATSC->Unlock(pDemodKernel);
	REALTEK_R_ATSC_INFO("Carrier Offset = "PT_S32BITS" Hz\n", *pOffset);

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
int REALTEK_R_ATSC_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

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
int REALTEK_R_ATSC_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned long cur_time;
#endif

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	cur_time = tv_osal_time();

	while ((tv_osal_time() - cur_time) <= 15) {
		if (pRealtekRATSC->m_acquire_sig_en == 0)
			return TUNER_CTRL_OK;

		tv_osal_msleep(5);
	}

	return TUNER_CTRL_FAIL;

#else
	REALTEK_R_ATSC_AcquireSignalThread(pDemodKernel);
	//pRealtekRATSC->m_autotune_stime = tv_osal_time() + AUTODETECT_SIG_UPDATE_INTERVAL_MS;
	pRealtekRATSC->m_autotune_TsOutputEnTime = tv_osal_time() + AUTOTUNE_CHECK_TS_OUTPUT_EN_INTERVAL_MS;
	pRealtekRATSC->m_autotune_Cr2PatchTime = tv_osal_time() + AUTOTUNE_CHECK_CR2_PATCH_INTERVAL_MS;
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
int REALTEK_R_ATSC_AcquireSignalThread(DEMOD* pDemodKernel)
{
	unsigned long stime;
	int lock = 0, no_signal = 1;

#ifdef REALTEK_R_ATSC_LGE_STREAM_PATCH_I37
	U32BITS BerNum = 1, BerDen = 1;
	U32BITS PerNum = 1, PerDen = 1;
	U32BITS percnt = 0;
#endif

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	REALTEK_R_ATSC_INFO("\033[1;32;32m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);

	if (pRealtekRATSC->m_acquire_sig_en == 0)
		return TUNER_CTRL_OK;

	pRealtekRATSC->Lock(pDemodKernel);
	pRealtekRATSC->m_auto_tune_enable = 0;
	stime = tv_osal_time();

	while ((tv_osal_time() - stime) <= 200) {
		tv_osal_msleep(5);
		pDemod->IsNoSig(pDemod, &no_signal);

		if (no_signal == 0) break;
	}
	REALTEK_R_ATSC_INFO("No Signal = %d, Time = %lu\n", no_signal, tv_osal_time() - stime);

	if (no_signal == 1) goto acquire_signal_result;

	while ((tv_osal_time() - stime) <= 600) {
		tv_osal_msleep(5);
		pDemod->IsInnerLocked(pDemod, &lock);

		if (lock) break;
	}
	REALTEK_R_ATSC_INFO("Demod Inner Lock = %d, Time = %lu\n", lock, tv_osal_time() - stime);

#ifdef REALTEK_R_ATSC_LGE_STREAM_PATCH_I37
	if (lock == 0) goto acquire_signal_result;

	if (pDemodKernel->m_ScanMode == CH_NORMAL_MODE) {
		pRealtekRATSC->Unlock(pDemodKernel);
		pRealtekRATSC->m_acquire_sig_en = 0;
		return TUNER_CTRL_OK;
	}

	while ((tv_osal_time() - stime) <= 9550) {
		//pDemod->IsInnerLocked(pDemod, &lock2);
		pDemod->GetErrorRate(pDemod,  5, 1000, &BerNum, &BerDen, &PerNum, &PerDen);
		//REALTEK_R_ATSC_INFO("lock2 = %d, PerNum = "PT_U32BITS", PerDen = "PT_U32BITS" percnt = "PT_U32BITS", dtime = %lu\n", lock2, PerNum, PerDen, percnt, tv_osal_time() - stime);

		if (PerNum == 0)
			break;
		else if (PerNum * 5 < PerDen)
			percnt++;
		else percnt = 0;

		if (percnt == 1) {
			pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_AI_EN_5, 0);
			//pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_MSE_FIELD_TH_1, 0x37216ec);
			pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_MSE_FIELD_TH_H, 0x0);
			pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_MSE_FIELD_TH_L, 0x37216ec);
			pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_AI_EN_5, 1);
		}

		if (percnt > 2) break;
		tv_osal_msleep(15);
	}

	if (percnt < 1) {
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_AI_EN_5, 0);
		//pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_MSE_FIELD_TH_1, 0x37216ec);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_MSE_FIELD_TH_H, 0x0);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_MSE_FIELD_TH_L, 0x37216ec);
		pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_LD_AI_EN_5, 1);
	}
	REALTEK_R_ATSC_INFO("PerNum = "PT_U32BITS", PerDen = "PT_U32BITS", dtime = %lu\n", PerNum, PerDen, tv_osal_time() - stime);
#endif

acquire_signal_result:
	pRealtekRATSC->Unlock(pDemodKernel);
	pRealtekRATSC->m_acquire_sig_en = 0;
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
int REALTEK_R_ATSC_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range)
{
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

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
int REALTEK_R_ATSC_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	pRealtekRATSC->Lock(pDemodKernel);
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_OPT_I2C_RELAY, (on_off) ? 1 : 0) != FUNCTION_SUCCESS)
		ret = TUNER_CTRL_FAIL;

	pRealtekRATSC->Unlock(pDemodKernel);

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
int REALTEK_R_ATSC_AutoTune(DEMOD* pDemodKernel)
{
	REALTEK_R_ATSC* pRealtekRATSC;
	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	//pRealtekRATSC->Lock(pDemodKernel);
	//pRealtekRATSC->Unlock(pDemodKernel);

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
int REALTEK_R_ATSC_KernelAutoTune(DEMOD* pDemodKernel)
{
	unsigned long cur_time;
	unsigned char lock;
	U32BITS Ro8051PatchInd, RoCrCr2SwBw;
	U32BITS Ro8051CrSwModeInd, Ro8051StayCarbNbInd, Ro8051CrGlDiffInd;
	
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;
	
	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	cur_time = tv_osal_time();

	if (pRealtekRATSC->m_acquire_sig_en) {
		#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
		REALTEK_R_ATSC_AcquireSignalThread(pDemodKernel);
		//pRealtekADVBTX->m_autotune_stime = tv_osal_time() + AUTODETECT_SIG_UPDATE_INTERVAL_MS;
		pRealtekRATSC->m_autotune_TsOutputEnTime = tv_osal_time() + AUTOTUNE_CHECK_TS_OUTPUT_EN_INTERVAL_MS;
		pRealtekRATSC->m_autotune_Cr2PatchTime = tv_osal_time() + AUTOTUNE_CHECK_CR2_PATCH_INTERVAL_MS;
		#endif

	} else if ((pRealtekRATSC->m_patch_ind == 0) && (pDemodKernel->m_ScanMode == CH_NORMAL_MODE)) {
		REALTEK_R_ATSC_GetLockStatus(pDemodKernel, &lock);

		if (IsATSC3McmPkgFlag == 2) {
			pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, Dummy_8051_CR_SW_MODE_IND, &Ro8051CrSwModeInd);
			REALTEK_R_ATSC_DBG("MCM: pRealtekRATSC->m_patch_ind = %u, pDemodKernel->m_ScanMode = %d, lock = %u, Ro8051CrSwModeInd = "PT_U32BITS"\n",
				pRealtekRATSC->m_patch_ind, pDemodKernel->m_ScanMode, lock, Ro8051CrSwModeInd);
			
		} else if (IsATSC3McmPkgFlag == 1) {
			pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, Dummy_8051_PATCH_IND, &Ro8051PatchInd);
			REALTEK_R_ATSC_DBG("Non-MCM: pRealtekRATSC->m_patch_ind = %u, pDemodKernel->m_ScanMode = %d, lock = %u, Ro8051PatchInd = "PT_U32BITS"\n",
				pRealtekRATSC->m_patch_ind, pDemodKernel->m_ScanMode, lock, Ro8051PatchInd);
		}
			
		pRealtekRATSC->Lock(pDemodKernel);
		if (IsATSC3McmPkgFlag == 2 && lock == 1 && Ro8051CrSwModeInd == 3) {
			tv_osal_msleep(90);
			REALTEK_R_ATSC_TPOutEn(pDemodKernel, 1);
			pRealtekRATSC->m_patch_ind = 1;
			REALTEK_R_ATSC_INFO("\033[1;32;31m" "MCM: Enable Demod TS Output after 8051 FW Done\n" "\033[m");

		} else if (IsATSC3McmPkgFlag == 1 && lock == 1 && Ro8051PatchInd >= 2 && Ro8051PatchInd <= 5) {
			tv_osal_msleep(60);
			REALTEK_R_ATSC_TPOutEn(pDemodKernel, 1);
			pRealtekRATSC->m_patch_ind = 1;
			REALTEK_R_ATSC_INFO("\033[1;32;31m" "Non-MCM: Enable Demod TS Output after 8051 FW Done\n" "\033[m");
			
		} else if ((cur_time > pRealtekRATSC->m_autotune_TsOutputEnTime) || ((pRealtekRATSC->m_autotune_TsOutputEnTime - cur_time) > AUTOTUNE_CHECK_TS_OUTPUT_EN_INTERVAL_MS)) {
			REALTEK_R_ATSC_TPOutEn(pDemodKernel, 1);
			pRealtekRATSC->m_patch_ind = 1;
			pRealtekRATSC->m_autotune_TsOutputEnTime = tv_osal_time() + AUTOTUNE_CHECK_TS_OUTPUT_EN_INTERVAL_MS;
			REALTEK_R_ATSC_INFO("\033[1;32;31m" "Enable Demod TS Output after Time Out\n" "\033[m");
		}
		pRealtekRATSC->Unlock(pDemodKernel);
		
	} else if ((pRealtekRATSC->m_patch_ind == 1) && (pDemodKernel->m_ScanMode == CH_NORMAL_MODE)) {
	
		if (IsATSC3McmPkgFlag == 2) {
			REALTEK_R_ATSC_GetLockStatus(pDemodKernel, &lock);
			pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, Dummy_8051_CR_SW_MODE_IND, &Ro8051CrSwModeInd);
		
			pDemodKernel->Lock(pDemodKernel);
			if ((lock == 1) && (Ro8051CrSwModeInd == 3) && (pRealtekRATSC->m_patch_cr2_ind != 1)) {
				pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, Dummy_8051_STAY_CARB_NB_IND, &Ro8051StayCarbNbInd);
				pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, Dummy_8051_CR_GL_DIFF_IND, &Ro8051CrGlDiffInd);
		
				if (Ro8051StayCarbNbInd > 0 && Ro8051StayCarbNbInd < 3 && Ro8051CrGlDiffInd > 0 && Ro8051CrGlDiffInd < 3) {
					pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_CR_CR2_SWBW, &RoCrCr2SwBw);
				
					if (RoCrCr2SwBw == 1) {
						pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_CR2_CARB_WB, 3);
						pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_FMU_UPD_CNT_TH, 0);
						pRealtekRATSC->m_patch_cr2_ind = 1;
						pRealtekRATSC->m_autotune_Cr2PatchTime = tv_osal_time() + AUTOTUNE_CHECK_CR2_PATCH_INTERVAL_MS;
						REALTEK_R_ATSC_INFO("\033[1;32;31m" "Switch the Updated Speed of CR to Full\n" "\033[m");
					}
				}
			
			} else if ((pRealtekRATSC->m_patch_cr2_ind == 1) && ((lock == 0) || (cur_time > pRealtekRATSC->m_autotune_Cr2PatchTime) || ((pRealtekRATSC->m_autotune_Cr2PatchTime - cur_time) > AUTOTUNE_CHECK_CR2_PATCH_INTERVAL_MS))) {
				pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_CR_CR2_SWBW, &RoCrCr2SwBw);

				if (RoCrCr2SwBw == 0) {
					pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_CR2_CARB_WB, 2);
					pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_CR_FMU_UPD_CNT_TH, 1);
					pRealtekRATSC->m_patch_cr2_ind = 2;
					REALTEK_R_ATSC_INFO("\033[1;32;31m" "Switch the Updated Speed of CR to Lower\n" "\033[m");
				}
			}
			pDemodKernel->Unlock(pDemodKernel);
		}
	}

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : DebugInfo
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_ATSC_DebugInfo(DEMOD* pDemodKernel, unsigned char debug_mode)
{
	int ret = TUNER_CTRL_OK;

	BASE_INTERFACE_MODULE* pBaseInterface;
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	pRealtekRATSC->Lock(pDemodKernel);
	if (pDemod) {
		switch (debug_mode) {
		case 0: //ADC dump
			pBaseInterface = pDemod->pBaseInterface;
			DtvDemodDMAAllocateMemory();
			DtvDemodDMADump(pBaseInterface, pDemodKernel->m_addr, 0);
			break;
		case 1: //reg dump
			DtvDumpShareRegStatus();
			break;
		case 2: //sysinfo dump
			REALTEK_R_ATSC_DumpSysInfo();
			break;
		default:
			REALTEK_R_ATSC_WARNING("[DEMOD][DEBUGINFO]unknown debug mode\n");
			break;
		}
	}

	pRealtekRATSC->Unlock(pDemodKernel);

	return ret;
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
int REALTEK_R_ATSC_InspectSignal(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	S32BITS IfAgc = -1;
	S32BITS RfAgc = -1;
	S32BITS TrOffsetPpm = -1;
	S32BITS CrOffsetHz  = -1;
	U32BITS DiAgc = 0;
	int AgcLock   = 0;
	int FrameLock = 0;
	U32BITS BerNum = 1;
	U32BITS BerDen = 1;
	U32BITS PerNum = 1;
	U32BITS PerDen = 1;
	S32BITS SnrDbNum = 1;
	S32BITS SnrDbDen = 1;

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	//if (DtvIsDemodClockEnable(RTK_DEMOD_MODE_ATSC) == 0)
	//	return TUNER_CTRL_OK;

	pRealtekRATSC->Lock(pDemodKernel);
	pDemod->GetRfAgc(pDemod, &RfAgc);
	pDemod->GetIfAgc(pDemod, &IfAgc);
	pDemod->GetDiAgc(pDemod, &DiAgc);
	pDemod->GetTrOffsetPpm(pDemod, &TrOffsetPpm);
	pDemod->GetCrOffsetHz(pDemod, &CrOffsetHz);
	pDemod->IsAagcLocked(pDemod, &AgcLock);
	pDemod->IsFrameLocked(pDemod, &FrameLock);
	pDemod->GetErrorRate(pDemod, 3, 600, &BerNum, &BerDen, &PerNum, &PerDen);
	pDemod->GetSnrDb(pDemod, &SnrDbNum, &SnrDbDen);
	pRealtekRATSC->Unlock(pDemodKernel);

	IfAgc = ((IfAgc + 8191) * 100) / 16384;
	RfAgc = ((RfAgc + 8191) * 100) / 16384;
	DiAgc = (DiAgc * 100) / 8192;

#if 0
	REALTEK_R_ATSC_INFO("[REALTEK_R_ATSC] lock(agc/frame)=%d/%d, agc(rf/if/di)="PT_S32BITS"/"PT_S32BITS"/"PT_U32BITS", ofst(tr/cr)="PT_S32BITS"/"PT_S32BITS",  error_rate(ber/per)=%f/%f, snr=%f\n",
		   AgcLock, FrameLock,
		   RfAgc, IfAgc, DiAgc,
		   TrOffsetPpm, CrOffsetHz,
		   (double)BerNum / (double)BerDen,
		   (double)PerNum / (double)PerDen,
		   (double)SnrDbNum / (double) SnrDbDen);
#endif

	ret = 1;

#ifdef REALTEK_R_DUMP_SHARE_REG
	REALTEK_R_ATSC_INFO("dump dtv reg:\n");
	DtvDumpShareRegStatus();
#endif

	return ret;
}


/*----------------------------------------------------------------------
 * Func : Activate REALTEK_R ATSC Demod
 *
 * Desc : Run Auto Tuning for CCI
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_ATSC_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{
	int i;
	unsigned char Val;

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	pRealtekRATSC->Lock(pDemodKernel);
	if (DtvDemodInitialization(RTK_DEMOD_MODE_ATSC, force_rst)) {
		REALTEK_R_ATSC_INFO("Demod PLL changed, re-init ATSC Demod\n");
		// pll has been changed and demod has been reset
		// we need to re-init demod


		if (pDemod->Initialize(pDemod) != FUNCTION_SUCCESS) {
			pRealtekRATSC->Unlock(pDemodKernel);
			goto activate_demod_fail;
		}


		ClearDemodReset(RTK_DEMOD_MODE_ATSC);

		for (i = 0; i < pRealtekRATSC->m_TunerOptRegCnt && pRealtekRATSC->m_pTunerOptReg; i++) {
			int Reg = pRealtekRATSC->m_pTunerOptReg[i].RegBitName;
			U32BITS Val = pRealtekRATSC->m_pTunerOptReg[i].Value;

			if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, Reg, Val) != FUNCTION_SUCCESS) {
				pRealtekRATSC->Unlock(pDemodKernel);
				goto activate_demod_fail;
		}
		}

		pRealtekRATSC->Unlock(pDemodKernel);

		if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
			goto activate_demod_fail;

		if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK)
			goto activate_demod_fail;
	} else {
		pRealtekRATSC->Unlock(pDemodKernel);

	}

	pRealtekRATSC->Lock(pDemodKernel);
	pDemod->RegAccess.Addr16Bit.GetRegBytes(pDemod, 0xe401, &Val, 1);
	Val &= ~0x3;
	Val |= 0x2;
	pDemod->RegAccess.Addr16Bit.SetRegBytes(pDemod, 0xe401, &Val, 1);
	pRealtekRATSC->Unlock(pDemodKernel);
	return TUNER_CTRL_OK;

activate_demod_fail:
	return TUNER_CTRL_FAIL;
}


int REALTEK_R_ATSC_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;
	REALTEK_R_ATSC* pRealtekRATSC;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	REALTEK_R_ATSC_INFO("REALTEK_R_ATSC_SetTvMode(%d)\n", mode);

	switch (mode) {
	case TV_SYS_TYPE_ATSC:
		REALTEK_R_ATSC_INFO("Set to ATSC\n");
		ret = pDemodKernel->Init(pDemodKernel);
		ret |= pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts);

		if (ret != TUNER_CTRL_OK) {
			REALTEK_R_ATSC_WARNING("Set REALTEK_R_ATSC to ATSC mode failed\n");
			goto set_demod_mode_failed;
		}
		break;

	default:
		REALTEK_R_ATSC_WARNING("unknown TV_SYSTEM_TYPE\n");
		goto set_demod_mode_failed;
	}

	ret = TUNER_CTRL_OK;
	REALTEK_R_ATSC_INFO("Set REALTEK_R_ATSC to appointed DTV mode successed\n");
	return ret;

set_demod_mode_failed:
	return TUNER_CTRL_FAIL;
}


/*-----------------------------------------------------------------
 *     BASE_INTERFACE for REALTEK_R_ATSC API
 *----------------------------------------------------------------*/
void REALTEK_R_ATSC_Lock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("LOCK 0x%x\n",&pDemodKernel->m_lock);
	REALTEK_R_ATSC* pRealtekRATSC;
	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	mutex_lock(&pRealtekRATSC->m_lock);
}


void REALTEK_R_ATSC_Unlock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("UN_LOCK 0x%x\n",&pDemodKernel->m_lock);
	REALTEK_R_ATSC* pRealtekRATSC;
	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	mutex_unlock(&pRealtekRATSC->m_lock);
}


/*----------------------------------------------------------------------
 * Func : DumpSysInfo
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
void REALTEK_R_ATSC_DumpSysInfo()
{
	REALTEK_R_ATSC_INFO("[DEMOD][DUMPSYSINFO]tv_sys is ATSC\n");
}


void ReleaseRealtekRAtscDriver(REALTEK_R_ATSC_DRIVER_DATA *pDriver)
{
	kfree(pDriver);
}


REALTEK_R_ATSC_DRIVER_DATA* AllocRealtekRAtscDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
)
{
	REALTEK_R_ATSC_DRIVER_DATA* pDriver = (REALTEK_R_ATSC_DRIVER_DATA*) kmalloc(
			sizeof(REALTEK_R_ATSC_DRIVER_DATA) +
			sizeof(ATSC_DEMOD_MODULE)   +
			sizeof(I2C_BRIDGE_MODULE)   +
			sizeof(BASE_INTERFACE_MODULE), GFP_KERNEL);

	if (pDriver) {
		BASE_INTERFACE_MODULE* pBIF;
		ATSC_DEMOD_MODULE* pDemod;

		memset(pDriver, 0, sizeof(REALTEK_R_ATSC_DRIVER_DATA));
		pDriver->pDemod                = (ATSC_DEMOD_MODULE*)(((unsigned char*)pDriver) + sizeof(REALTEK_R_ATSC_DRIVER_DATA));
		pDriver->pBaseInterface        = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(ATSC_DEMOD_MODULE));
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
		pBIF->useI2C = 0;

		// Init Demod Driver
		BuildRealtekRAtscModule(&pDemod,
								pDriver->pDemod,
								pDriver->pBaseInterface,
								pDriver->pI2CBridge,
								Addr,
								CrystalFreq);
	}

	return pDriver;
}


int REALTEK_R_ATSC_TPOutEn(DEMOD* pDemodKernel, unsigned char enable)
{
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;
	BASE_INTERFACE_MODULE *pBaseInterface;
	int ret = FUNCTION_SUCCESS;

	if (pDemodKernel == NULL)
		return TUNER_CTRL_FAIL;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;
	pBaseInterface = pDemod->pBaseInterface;

	if (IsAtsc3McmPackage(RTK_DEMOD_MODE_ATSC) == 0) {
		DtvDemodTPOutEn(enable);
	} else {

		pBaseInterface->I2cRead  = __realtek_i2c_read_speedF;
		pBaseInterface->I2cWrite  = __realtek_i2c_write_speedF;

		ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xB816F600, 0xFFFFFFFE, (enable & 0x1));


		pBaseInterface->I2cRead  = __realtek_rbus_i2c_read;
		pBaseInterface->I2cWrite  = __realtek_rbus_i2c_write;
	}

	REALTEK_R_ATSC_INFO("DtvRDemod TP Output enable = %u !!!\n", enable);
	return ret;
}


int REALTEK_R_Ext_ATSC_PLL_Default(DEMOD* pDemodKernel, unsigned char On)
{
	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	//PLL
	//RL6681 register default setup
	//rtd_outl(0xb80004b0,0x00000010);				//<bit4>MAIN_POR_EN = 0x1

	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004c4, 0x00000000);				//DTV PLLDIF PLLDIF_DIVDIG_SEL = 000(2), PLLDIF_DIVADC_SEL = 00(divide by 8)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004b8, 0x00003600);
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004d4, 0x80086500);				//<bit31>Pll_xtal_pow=0x1, <bit20:19>PLL_LDO_XTAL_SEL=0x1(1.76V), <bit14>PLL_POW_LDO11V=0x1, <bit13:11>PLL_LDO11V_SEL=0x4,
	//<bit10>PLL_POW_LDO=0x1, <bit8:9>PLL_LDO_SEL=0x1(0.952V)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004d8, 0x00000000);				//Default setting, <bit15:8>PLL_RESERVE=0x0(0=Select ADC CLK From PLL27X_IFADC).
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004dc, 0x40000001);				//Default ATV mode, <bit30:29>PLL27X_IFADC_DUTY=0x2,<bit0>PLL_CK_IFADC_MUX=0x0(ATV mode)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8000500, 0x0000041c);				//<bit11:10>PLL27X_IFADC_CK108M_DIV=0x1(VCO=432M),<bit4>PLL27X_IFADC_DTV_CKO_EN=0x1(enable),<bit3:2>PLL27X_IFADC_ATV_CKO_SEL=0x3(135M or 162M (depend on VCO and REG_PLL27x_IFADC_DIV))
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8000504, 0x000e0224);				//PLL27X_IFADC VCO M,N, M<10>+2¦P=14(VCO=432M)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8000508, 0x00000000);				//PLL27X_IFADC Poweroff and VCORSTB reset
	tv_osal_usleep(10);						//delay 10uS

	//PLL27X
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb80004dc, 0xfffffffe, 0x00000000);	//Set MUX to DTV Mode<bit0>=0x0

	//PLL27X_IFADC
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000500, 0xfffffff3, 0x0000000C);	//PPLL27X_IFADC_DTV_CKO_SEL = 00(/3), PLL27X_IFADC_ATV_CKO_SEL = 11 (135M or 162M (depend on VCO and REG_PLL27x_IFADC_DIV).
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000508, 0xfffffff8, 0x00000007);	//IFADC, RSTB and VCORSTB Power on
	return ret;
}


int REALTEK_R_Ext_ATSC_ADC_Default(DEMOD* pDemodKernel, unsigned char On)
{
	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	//RL6681 ADC register default
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8033100, 0x01110500);					//<bit25:24>REG_IF_BY_VCM_SEL=0x1, <bit21:20>REG_IPGA_MBIAS_ISEL=0x1, <bit16=1>IFD MBIAS power on,<bit11:10>REG_I_VCM_SEL=0x1(0.632V),<bit9:8>ADC2X_BG_SEL= 0x1(1.216V)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8033104, 0x00000300);					//<bit24>IQADC input cm power off, <bit9:8>REG_IPGA_BW=0x3(37.5M)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8033118, 0x00000000);					//<bit8>REG_I_CKOUT_EDGE=0x1=0(IADC CKOUT polarity=negative edge)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8033124, 0x00000000);					//PGA/ADC Power Off
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8033128, 0x00000002);					//Default setting(10=20uA), IQ PGA CURRENT  select
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb803312C, 0x00000000);					//default setting, <bit3:0>REG_IPGA_D=0x0(IPGA gain=0dB)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8033130, 0x08000000);					//default setting, <bit27:26>REG_IADC_I_VREF_SEL=0x2(1.5X)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8033134, 0x11000091);					//default setting, <bit25:24>REG_IADC_VREF_SEL=0x1(1.4V), <bit8:7>REG_IADC_LDO0_SEL=0x1(1.3V), <bit2:0>REG_IADC_LDO_SEL=0x1(0.961V)

	//Set IADC/IPGA power on and IPGA input
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8033124, 0xfffffefe, 0x00000101);		//I_PGA<bit8> power on, <bit0>I_ADC power on
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8033128, 0xfff8ffff, 0x00020000);		//<bit18:16>REG_IPGA_INPUT_SEL=0x2(IFD input)

	//Set IPGA/QPGA GAIN = 0dB
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb803312c, 0xffffff00, 0x00000000);		//<bit3:0>reg_ipga_d=0x0(0dB)
	return ret;
}


int REALTEK_R_Ext_ATSC_I2C_Init(DEMOD* pDemodKernel)
{
	int ret = FUNCTION_SUCCESS;

	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	//unsigned int ReadData = 0;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb801A014, 0x01000101);
	//ret |= atsc_rtd_4bytes_i2c_rd(pDemod, 0xb801A014, &ReadData);
	//REALTEK_R_ATSC_INFO("0xb801A014 = 0x%x\n", ReadData);
	return ret;
}


int REALTEK_R_Ext_ATSC_System_Init(DEMOD* pDemodKernel, unsigned char On)
{
	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	//--------CRT All turn on----------------//
	//	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x000001ff);
	//	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x000001ff);
	//---------------------------------------//

	//Turn on MISC 												//for GPIO
	//<bit8>RSTN_MISC = 0x1(Release reset)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000101);
	//<bit8>CLKEN_MISC = 0x1(Enable)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000101);

	//DTV Demod Sel
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb80004b0, 0xffffffe8, 0x00000011); 				//<bit4>MAIN_POR_EN=0x1, <bit2>Dtv_demod_atb_clk_sel=0(Ldpc_Clk=370MHz), <bit1:0>dtv_demod_sel=00(atsc3.0)

	//IfdMbiasPowAndAdcPow
	// 1. CRT_CLK_OnOff(TVADC, CLK_ON, NULL);					//Turn on CLKEN_IFADC
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000081);	  //<bit7>RSTN_IFADC, set to 1(Release)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000081);	 //<bit7>CLKEN_IFADC, set to 1
	tv_osal_usleep(50);									//delay 50uS

	// 2. MBIAS and PLL_LDO Power turn on
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8033100, 0xfffeffff, 0x00010000);  			//<bit16>REG_POW_MBIAS turn on
	tv_osal_usleep(50);									//delay 50uS
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb80004d4, 0xffffbbff, 0x00004400);  			//<bit14>PLL_POW_LDO11V, <bit10>PLL_POW_LDO turn on
	tv_osal_usleep(50);									//delay 50uS

	// 3. turn Off ADC/PGA power
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8033104, 0xfeffffff, 0x00000000);				//IQADC input cm power off <bit24> = 0
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8033124, 0xfffffefe, 0x00000000);				//IPGA/IADC Power Off
	tv_osal_usleep(50);									//delay 50uS

	//----------------------------------------------------------//
	//CRT_CLK_OnOff(TVALL, CLK_OFF, NULL);        				//Turn off TVALL
	//DTV Demod IP reset
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x0000005a);							//Demod IP Reset<bit6, bit4, bit3, bit1> set to 0(active low)
	tv_osal_msleep(1);									//driver delay 1mS

	//DTV Demod Wrapper reset
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000124);							//Demod IP Reset<bit8, bit5, bit2> set to 0(active low)
	tv_osal_msleep(1);									//driver delay 1mS

	//DTV Demod turn off
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x000017e);							//Demod CLK<bit8, bit6, bit5, bit4, bit3, bit2, bit1> set to 0
	tv_osal_usleep(10);									//driver delay 10uS

	//----------------------------------------------------------//
	//#include "RL6681_TVFrontEnd_1_PLL_default.tbl"				//Load PLL default setting
	ret |= REALTEK_R_Ext_ATSC_PLL_Default(pDemodKernel, On) ;

	//--------PLL27X_IFADC-------------------
	//RL6681_VCO=648MHz, ADC_CLK=144M, SYS_CLK=518.4MHz
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb80004b0, 0xffffffe8, 0x00000011);				//<bit4>MAIN_POR_EN = 0x1, <bit2>Dtv_demod_atb_clk_sel=0(Ldpc_Clk=370MHz), <bit1:0>dtv_demod_sel=00(atsc3.0)
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb80004dc, 0xfffffffe, 0x00000000);				//Set MUX to DTV Mode<bit0>=0x0
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000500, 0xffffffef, 0x00000010);				//PLL27x_IFADC DTV CLK<bit4>=0x1

	//27M XTAL
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000504, 0xfc80fff8, 0x000e0004);				//VCO = M<6:0>+2 = 22+2, N:00=1, 27 /1 *24=648MHz, PLL27X_IFADC_M<bit22:16> = 0x16(22), PLL27X_IFADC_N<bit25:24>=0x0(1), PLL27X_IFADC PUMP current<bit2:0>=0x4(0x100:25uF)
	//24M XTAL
	//ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000504, 0xfc80fff8, 0x00100004);				//VCO = M<6:0>+2 = 22+2, N:00=1, 27 /1 *24=648MHz, PLL27X_IFADC_M<bit22:16> = 0x16(22), PLL27X_IFADC_N<bit25:24>=0x0(1), PLL27X_IFADC PUMP current<bit2:0>=0x4(0x100:25uF)


	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb80004c4, 0xffffff88, 0x00000000);				//System_CLK : PLLDIF_DIVDIG_SEL<bit6:4> = 0x3(/1.25), sys_clk = 648 / 1.25 = 518.4 MHz
	//ADC_CLK : PLLDIF_DIVADC_SEL<2:0> = 0x7(/4.5), Adc_clk = 648 / 4.5 =144MHz
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb80004b8, 0xFFFD81FB, 0x00021200);				//<bit17>ATVADCDIV_RSTB=0x1(normal), <bit13>DIF216M_EN=0x0, <bit12>DTVDIG_EN=0x1, <bit11>LDPC_DIV=0x0(div1.75), <bit10>LDPC_EN=0x1
	tv_osal_msleep(1);//delay 1ms

	//*****RL6681_VCO=432MHz, ADC_CLK=144M, SYS_CLK=246.857MHz****//
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000508, 0xfffffff8, 0x00000000);  			//<2:1>PLL27X_IFADC_VCORSTB, PLL27X_IFADC_RSTB reset = 0(Reset); <0>PLL27X_IFADC_POW turn off
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000508, 0xfffffffe, 0x00000001);  			//<0>PLL27X_IFADC_POW turn On
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000508, 0xfffffff9, 0x00000006);  			//<2:1>PLL27X_IFADC_VCORSTB, PLL27X_IFADC_RSTB reset = 1(release)
	tv_osal_msleep(1);//delay 1ms

	tv_osal_usleep(50);									//driver delay 50uS
	//#include "RL6681_TVFrontEnd_2_ADC_IFD.tbl"					//Load ADC default setting
	ret |= REALTEK_R_Ext_ATSC_ADC_Default(pDemodKernel, On);

	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xB80004b8, 0xfffffdff, 0x00000000);  			//<bit9>IFADC ATV clk enable turn off
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xB8000500, 0xffffffef, 0x00000010);  			//<bit4>PLL27X_IFADC_DTV_CKO_EN setup
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xB80004b8, 0xfffffdff, 0x00000200);  			//<bit9>IFADC ATV clk enable turn on
	tv_osal_usleep(50);									//driver delay 50uS

	//----------------------------------------------------------//
	//DtvDemodPowerControl(1)
	//CRT_CLK_OnOff(TVFRONTEND, CLK_ON, NULL);					//Turn On TVFrontend(memory bank and dtv_forntend)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000061);							//<bit6>RSTN_dtv_frontend, <bit5>RSTN_dtv_demod_mb, <bit0>write bit = 0x1(release)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000061);							//<bit6>CLKEN_dtv_frontend, <bit5>Clken_dtv_demod_mb, <bit0>write bit = 0x1(CLKEN)
	tv_osal_usleep(10);									//driver delay 10uS

	//----------- Set DTV Frontend path --------------//
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb8033800, 0x00040000);							//<bit18:16>Dtv_frontend_mode=0x1, <bit8>dtv_idata_sel=0x0(from dn flt), <bit4>Dtv_dnflt_en=0x1(DN6)
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8033804, 0xfffbffff, 0x00040000);				//input fifo clr, dtv_input_fifo_clr<bit18> FIFO waddr & raddr clear
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8033804, 0xfffbffff, 0x00000000);				//input fifo clr release, dtv_input_fifo_clr<bit18>, FIFO waddr & raddr clear

	//Set if_agc
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb800081c, 0xFF0FFFFF, 0x00400000); 				//<bit23:20>GPIO_29, Set pinmux DEMOD_IF_AGC

	//-----------------Enable DTV Demod IP-------------//
	tv_osal_usleep(50);									//driver delay 50uS
	//DTV_DEMOD CLK / RESET flow : DTV Demod misc CRT turn on first then Demod IP CRT
	//Enable MISC												//Turn On MISC
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x00000111);							//<bit8>RSTN_MISC, <bit4>RSTN_DTV_DEMOD, <bit0>write bit = 0x1(release)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x00000111);							//<bit8>CLKEN_MISC, <bit4>CLKEN_DTV_DEMOD <bit0>write bit = 0x1(CLKEN)
	tv_osal_usleep(10);									//delay 10uS

	//DTV_ATB_Demod CLK/Reset Enable,  Demod wrapper frist then Demod IP
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a0, 0x0000000b);							//<bit2>RSTN_atb_demod_wrap, <bit0>write bit set to 1(release)
	ret |= atsc_rtd_4bytes_i2c_wr(pDemod, 0xb80004a4, 0x0000000b);							//<bit2>clken_atb_demod_wrap, <bit0>write bit set to 1
	tv_osal_usleep(10);									//delay 10uS

	//----Start RL6681 ATSC Demod TS output Pinmux setup----//
#ifndef REALTEK_R_EXT_QFN //MCM
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb800080c, 0x0f0fff0f, 0x10100010);	  //TP0 sync, val, clk
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000810, 0x0fff0f0f, 0x10001010);    //TP0 D7, D6, D5
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000814, 0xff0f0fff, 0x00101000);      //TP0 D4, D3
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000818, 0xff0f0fff, 0x00101000);	  //TP0 D2, D1
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb800081c, 0x0fffffff, 0x10000000);        //TP0 D0
#else //QFN
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb800082c, 0x0f0fffff, 0x20200000);	//TPO_D0,D1
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000830, 0x0f0f0f0f, 0x20202020);    //TPO_D2,D3,D4,D5
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000838, 0xffffff0f, 0x00000020);    //TPO_Val
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb800083c, 0xff0f0f0f, 0x00202020);	//TPO_D7,D6,Sync
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb8000840, 0x0dffffff, 0x22000000);    //TPO_Clk, Driving
#endif
	//----End of RL6681 ATSC Demod TS output Pinmux setup----//

	//Bus CLK select
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb80004b0, 0xfffffff7, 0x00000000);	  //bus clk source:  ATSC
	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xb80004ac, 0xfffffffe, 0x00000000);	  //BUS CLK sorce: PLLBUS

	tv_osal_usleep(50);									//delay 50uS

	return ret;
}


int REALTEK_R_Ext_ATSC_BisrCheck(DEMOD* pDemodKernel)
{
	int ret = FUNCTION_SUCCESS;
	REALTEK_R_ATSC* pRealtekRATSC;
	ATSC_DEMOD_MODULE* pDemod;
	int loopcnt = 0;
	int BisrDone = 0;

	pRealtekRATSC = (REALTEK_R_ATSC*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekRATSC->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_ATSC_DRIVER_DATA*) pRealtekRATSC->m_private)->pDemod;

	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xB816F570, 0xFFFFFF80, 0x7f);  //bisr reset

	while (BisrDone != 0x7f) {
		loopcnt++;
		tv_osal_usleep(100);
		ret |= atsc_rtd_4bytes_i2c_rd(pDemod, 0xB816F584, &BisrDone);
		BisrDone &= 0x7f;

		if (loopcnt > 120)
			break;
	}

	REALTEK_R_ATSC_INFO("BisrCheck BisrDone= 0x%x, loopcnt = %d \n", BisrDone, loopcnt);

	ret |= atsc_rtd_4bytes_i2c_mask(pDemod, 0xB816F580, 0xFFFFFF80, 0x7f);  //bisr hold and remap

	return ret;
}


