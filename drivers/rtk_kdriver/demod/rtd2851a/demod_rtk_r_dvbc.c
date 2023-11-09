/***************************************************************************************************
  File        : demod_realtek_r_dvbc.cpp
  Description : REALTEK_R_ DVB-T demod
  Author      : Kevin Wang
****************************************************************************************************
    Update List :
----------------------------------------------------------------------------------------------------
    20120207    | create phase
***************************************************************************************************/
#include <linux/slab.h>
#include <linux/pageremap.h>
//#include <OSAL.h>
#include "foundation.h"
#include "demod_rtk_r_dvbc.h"
#include "qam_demod_rtk_r.h"
#include "demod_rtk_r_dvbc_priv.h"
#include <io.h>
#include "dvbc_reg.h"

unsigned char OutErrorCnt = 0;
/*----------------------------------------------------------------------
 * Func : REALTEK_R_DVBC
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
void REALTEK_R_DVBC_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm)
{
	REALTEK_R_DVBC* pRealtekRDVBC;
	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::REALTEK_R_DVBC\n");

	pDemodKernel->private_data = dvr_malloc(sizeof(REALTEK_R_DVBC));
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);

	Demod_Constructors(pDemodKernel);

	pDemodKernel->m_id                 = DTV_DEMOD_REALTEK_R_DVBC;
	pDemodKernel->m_addr               = addr;
	pDemodKernel->m_pTuner             = NULL;
	pDemodKernel->m_pComm              = pComm;
	pDemodKernel->m_ScanRange          = SCAN_RANGE_3_6;
	pDemodKernel->m_Capability         = TV_SYS_TYPE_DVBC;
	pDemodKernel->m_update_interval_ms = 300;
	pDemodKernel->m_if.freq            = 5000000;
	pDemodKernel->m_if.inversion       = 0;
	pDemodKernel->m_if.agc_pola        = 0;

	//if (DtvGetICVersion() == 1) //MERLIN4_VER_A
		//pDemodKernel->m_clock              = CRYSTAL_FREQ_31500000HZ;//CRYSTAL_FREQ_28800000HZ;//CRYSTAL_FREQ_27000000HZ;
	//else
		pDemodKernel->m_clock              = CRYSTAL_FREQ_36000000HZ;

	pRealtekRDVBC->m_output_freq        = output_freq;

	pRealtekRDVBC->m_private            = (void*) AllocRealtekRDvbcDriver(
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
	pDemodKernel->m_ts.tsOutEn = 1;
	pDemodKernel->m_ScanMode           = CH_UNKNOW;
	pRealtekRDVBC->m_TunerOptRegCnt     = 0;
	pRealtekRDVBC->m_auto_tune_en     = 0;
	pRealtekRDVBC->m_pTunerOptReg       = NULL;
	pRealtekRDVBC->m_acquire_stime = 0;
	pRealtekRDVBC->m_acquire_sig_en = 0;
	pRealtekRDVBC->m_ts_clk_KHz = 0;

	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "REALTEK_R_DVBC");

	pDemodKernel->Init = REALTEK_R_DVBC_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = REALTEK_R_DVBC_AcquireSignal;
	pDemodKernel->ScanSignal = REALTEK_R_DVBC_ScanSignal;
	pDemodKernel->SetIF = REALTEK_R_DVBC_SetIF;
	pDemodKernel->SetTSMode = REALTEK_R_DVBC_SetTSMode;
	pDemodKernel->SetTVSysEx = REALTEK_R_DVBC_SetTVSysEx;
	pDemodKernel->SetMode = REALTEK_R_DVBC_SetMode;
	pDemodKernel->SetTVSys = REALTEK_R_DVBC_SetTVSys;
	pDemodKernel->GetLockStatus = REALTEK_R_DVBC_GetLockStatus;
	pDemodKernel->GetSegmentNum = Demod_GetSegmentNum;
	pDemodKernel->GetSignalInfo = REALTEK_R_DVBC_GetSignalInfo;
	pDemodKernel->GetSignalQuality = REALTEK_R_DVBC_GetSignalQuality;
	pDemodKernel->ChangeT2PLP = Demod_ChangeT2PLP;
	pDemodKernel->GetT2PLPInfo = Demod_GetT2PLPInfo;
	pDemodKernel->GetT2L1PreInfo = Demod_GetT2L1PreInfo;
	pDemodKernel->GetT2L1PostInfo = Demod_GetT2L1PostInfo;
	pDemodKernel->GetDemodStatus = Demod_GetDemodStatus;
	pDemodKernel->GetCarrierOffset = REALTEK_R_DVBC_GetCarrierOffset;
	pDemodKernel->AutoTune = REALTEK_R_DVBC_AutoTune;
	pDemodKernel->KernelAutoTune = REALTEK_R_DVBC_KernelAutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = Demod_Suspend;
	pDemodKernel->Resume = Demod_Resume;
	pDemodKernel->SetTvMode = REALTEK_R_DVBC_SetTvMode;
	pDemodKernel->InspectSignal = REALTEK_R_DVBC_InspectSignal; // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
//	pDemodKernel->Lock = Demod_Lock;
//	pDemodKernel->Unlock = Demod_Unlock;
	pDemodKernel->Lock = REALTEK_R_DVBC_Lock;
	pDemodKernel->Unlock = REALTEK_R_DVBC_Unlock;

	//pDemodKernel->Activate = REALTEK_R_DVBC_Activate;
	pDemodKernel->Destory = REALTEK_R_DVBC_Destructors;

}

/*----------------------------------------------------------------------
 * Func : ~REALTEK_R_DVBC
 *
 * Desc : Destructor of REALTEK_R_DVBC
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void REALTEK_R_DVBC_Destructors(DEMOD* pDemodKernel)
{
	REALTEK_R_DVBC* pRealtekRDVBC;
	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::~REALTEK_R_DVBC\n");
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);

	if (pRealtekRDVBC->m_private != NULL)
		ReleaseRealtekRDvbcDriver((REALTEK_R_DVBC_DRIVER_DATA*)pRealtekRDVBC->m_private);

	dvr_free(pRealtekRDVBC);
	//pDemodKernel->Destory(pDemodKernel);
	REALTEK_R_DVBC_INFO("REALTEK_R_DVBC_Destructors Complete\n");
}



/*----------------------------------------------------------------------
 * Func : Init
 *
 * Desc : Initial realtek_i
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_DVBC_Init(DEMOD* pDemodKernel)
{
	int i;
	int IF_AGC_MAX = 0;
	int IF_AGC_MIN = 0;
	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;


	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);

	if (pDemodKernel->m_pComm == NULL || pRealtekRDVBC->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	pRealtekRDVBC->m_auto_tune_en = 0;
	pRealtekRDVBC->m_ts_clk_KHz = 0;

	DtvDemodInitialization(RTK_DEMOD_MODE_DVBC, 1);

	pDemod->Symbolratedelay = 40;

	// check tuner information
	if (pDemodKernel->m_pTuner) {
		TUNER_PARAM Param;

		if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, TV_SYS_DVBC_8M, &Param) == TUNER_CTRL_OK) {

			switch (Param.id) {
			case TV_TUNER_NXP_TDA18275A:
				REALTEK_R_DVBC_INFO("Tuner id %d, TV_TUNER_NXP_TDA1827X\n", Param.id);
				pRealtekRDVBC->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) TDA18273_OPT_VALUE_DVBC;
				pRealtekRDVBC->m_TunerOptRegCnt = sizeof(TDA18273_OPT_VALUE_DVBC) / sizeof(REG_VALUE_ENTRY_RBUS);

				break;
			case TV_TUNER_SILICONLABS_SI2151:
				REALTEK_R_DVBC_INFO("Tuner id %d, TV_TUNER_SILICONLABS_SI2151\n", Param.id);
				pRealtekRDVBC->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SI2151_OPT_VALUE_DVBC;
				pRealtekRDVBC->m_TunerOptRegCnt = sizeof(SI2151_OPT_VALUE_DVBC) / sizeof(REG_VALUE_ENTRY_RBUS);

				break;
			case TV_TUNER_SILICONLABS_SI2178B:
			case TV_TUNER_NO_TUNER:
				REALTEK_R_DVBC_INFO("Tuner id %d, TV_TUNER_SILICONLABS_SI215x\n", Param.id);
				pRealtekRDVBC->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SI2151_OPT_VALUE_DVBC;
				pRealtekRDVBC->m_TunerOptRegCnt = sizeof(SI2151_OPT_VALUE_DVBC) / sizeof(REG_VALUE_ENTRY_RBUS);

				break;
			case TV_TUNER_MAXLINEAR_MXL661:
				REALTEK_R_DVBC_INFO("Tuner id %d, TV_TUNER_MAXLINEAR_MXL6x1\n", Param.id);
				pDemod->Symbolratedelay = 80;
				rtd_maskl(DVBC_OPT_IF_AAGC_MAX_reg, ~DVBC_OPT_IF_AAGC_MAX_opt_if_aagc_max_mask, DVBC_OPT_IF_AAGC_MAX_opt_if_aagc_max(IF_AGC_MAX));
				rtd_maskl(DVBC_OPT_IF_AAGC_MIN_reg, ~DVBC_OPT_IF_AAGC_MIN_opt_if_aagc_min_mask, DVBC_OPT_IF_AAGC_MIN_opt_if_aagc_min(IF_AGC_MIN));
				pRealtekRDVBC->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) MXL661_OPT_VALUE_DVBC;
				pRealtekRDVBC->m_TunerOptRegCnt = sizeof(MXL661_OPT_VALUE_DVBC) / sizeof(REG_VALUE_ENTRY_RBUS);

				break;
			case TV_TUNER_RAFAEL_R840:
				REALTEK_R_DVBC_INFO("Tuner id %d, TV_TUNER_RAFAEL_R840\n", Param.id);
				pRealtekRDVBC->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) R840_OPT_VALUE_DVBC;
				pRealtekRDVBC->m_TunerOptRegCnt = sizeof(R840_OPT_VALUE_DVBC) / sizeof(REG_VALUE_ENTRY_RBUS);

				break;
			case TV_TUNER_SONY_SUTRX243:
			case TV_TUNER_SONY_SUTRX243_RA:
				REALTEK_R_DVBC_INFO("Tuner id %d, TV_TUNER_SONY_SUTRX243\n", Param.id);
				pRealtekRDVBC->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SUTRX243_OPT_VALUE_DVBC;
				pRealtekRDVBC->m_TunerOptRegCnt = sizeof(SUTRX243_OPT_VALUE_DVBC) / sizeof(REG_VALUE_ENTRY_RBUS);

				break;
			default:
				REALTEK_R_DVBC_WARNING("unkonw tuner id %d, using default setting\n", Param.id);
				break;
			}
			pDemodKernel->m_if.freq      = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola  = Param.if_agc_pola;
		}
	}

	// Initial Driver
	if (pDemod->Initialize(pDemod) != FUNCTION_SUCCESS) {
		pDemodKernel->Unlock(pDemodKernel);
		goto init_fail;
	}

	//pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OPT_MPEG_OUT_SEL, 1); // set MPEG output select
	rtd_maskl(DVBC_OPT_MPEG_OUT_SEL_reg, ~DVBC_OPT_MPEG_OUT_SEL_opt_mpeg_out_sel_mask, DVBC_OPT_MPEG_OUT_SEL_opt_mpeg_out_sel(0x1));

	// Run Optimize Configuration for specified tuner
	for (i = 0; i < pRealtekRDVBC->m_TunerOptRegCnt; i++) {
		U32BITS Reg = pRealtekRDVBC->m_pTunerOptRegRbus[i].RegBitNameRbus;
		U32BITS Val = pRealtekRDVBC->m_pTunerOptRegRbus[i].ValueRbus;

		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, Reg, Val) != FUNCTION_SUCCESS)
		//	goto init_fail;
		rtd_maskl(Reg, 0xffffff00, Val);
	}

	pDemodKernel->Unlock(pDemodKernel);

	if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
		goto init_fail;

	if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK)
		goto init_fail;

	REALTEK_R_DVBC_INFO("REALTEK_R_DVBC Init Complete\n");
	return TUNER_CTRL_OK;

init_fail:
	REALTEK_R_DVBC_WARNING("REALTEK_R_DVBC Init Failed\n");
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
int REALTEK_R_DVBC_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	int ret;
	TV_SIG_INFO *pSigInfo;
	REALTEK_R_DVBC* pRealtekRDVBC;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);

	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::SetTVSys\n");
	pSigInfo = dvr_malloc(sizeof(TV_SIG_INFO));

	pSigInfo->mod = TV_MODULATION_DVBC_QAM;
	pSigInfo->dvbc_qam.constellation = QAM_CONST_64;
	pSigInfo->dvbc_qam.symbol_rate = 6875000;

	ret = pDemodKernel->SetTVSysEx(pDemodKernel, sys, pSigInfo);
	dvr_free(pSigInfo);

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
int REALTEK_R_DVBC_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo)
{
	int ret = TUNER_CTRL_OK;
	TUNER_PARAM Param;
	CH_MODE DvbcScanMode = CH_NORMAL_MODE;
	unsigned int TSThroughputKBps = 96000, TsClkKHz = 12000;
	unsigned char q_val[] = {0, 4, 5, 6, 7, 8}; // *QPSK suit for qam mode defined, 16QAM, 32QAM, 64QAM, 128QAM, 256QAM
	int QamModeInfo = QAM_QAM_256; //Default Set to 256QAM

	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;
	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::SetTVSysEx\n");

	if (!IsDvbcSys(sys)) {
		REALTEK_R_DVBC_WARNING("Set TV system failed, unsupported TV system\n");
		return TUNER_CTRL_FAIL;
	}

	pRealtekRDVBC->m_acquire_stime = tv_osal_time();

	pRealtekRDVBC->m_auto_tune_en = 0;
	pRealtekRDVBC->m_ts_clk_KHz = 0;
	DtvDemodTPOutEn(0); //disable TS output

	//TUNER_PARAM Param;
	if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, pDemodKernel->m_tv_sys, &Param) == TUNER_CTRL_OK) {
		if ((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola != Param.if_agc_pola)) {
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;
			REALTEK_R_DVBC_WARNING("RF Param Changed, Re-Set\n");
			ret |= pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if);
		}
	}

	pDemodKernel->Lock(pDemodKernel);

	if (!pSigInfo) {
		REALTEK_R_DVBC_WARNING("Siginfo doesn't specified, SetTVSysEx Fail\n");
		goto SetTVSysEx_fail;
	}

	DvbcScanMode = pSigInfo->dvbc_qam.scan_mode;
	pDemodKernel->m_ScanMode = DvbcScanMode;
	pRealtekRDVBC->m_auto_fsr = pSigInfo->dvbc_qam.symbol_rate;
	pRealtekRDVBC->m_info.dvbc_qam.freq_khz = pSigInfo->dvbc_qam.freq_khz/1000;
	pRealtekRDVBC->m_info.dvbc_qam.symbol_rate = pSigInfo->dvbc_qam.symbol_rate;
	pDemodKernel->m_tv_sys = sys;
	pRealtekRDVBC->m_auto_qamd = 0xf;

	if (DvbcScanMode == CH_MANUAL_MODE)
	{
		REALTEK_R_DVBC_INFO("\033[1;32;33m" "[DVBC MANUAL MODE]\n" "\033[m");
		switch (pSigInfo->dvbc_qam.constellation) {
		case QAM_CONST_4:
			REALTEK_R_DVBC_INFO("\033[1;32;33m" "SR=%u, QAM=QAM_CONST_4\n" "\033[m", pRealtekRDVBC->m_auto_fsr);
			QamModeInfo = QAM_QAM_4;
			break;
		case QAM_CONST_16:
			REALTEK_R_DVBC_INFO("\033[1;32;33m" "SR=%u, QAM=QAM_CONST_16\n" "\033[m", pRealtekRDVBC->m_auto_fsr);
			QamModeInfo = QAM_QAM_16;
			pRealtekRDVBC->m_auto_qamd = 0x4;
			break;
		case QAM_CONST_32:
			REALTEK_R_DVBC_INFO("\033[1;32;33m" "SR=%u, QAM=QAM_CONST_32\n" "\033[m", pRealtekRDVBC->m_auto_fsr);
			QamModeInfo = QAM_QAM_32;
			pRealtekRDVBC->m_auto_qamd = 0x5;
			break;
		case QAM_CONST_64:
			REALTEK_R_DVBC_INFO("\033[1;32;33m" "SR=%u, QAM=QAM_CONST_64\n" "\033[m", pRealtekRDVBC->m_auto_fsr);
			QamModeInfo = QAM_QAM_64;
			pRealtekRDVBC->m_auto_qamd = 0x6;
			break;
		case QAM_CONST_128:
			REALTEK_R_DVBC_INFO("\033[1;32;33m" "SR=%u, QAM=QAM_CONST_128\n" "\033[m", pRealtekRDVBC->m_auto_fsr);
			QamModeInfo = QAM_QAM_128;
			pRealtekRDVBC->m_auto_qamd = 0x7;
			break;
		case QAM_CONST_256:
			REALTEK_R_DVBC_INFO("\033[1;32;33m" "SR=%u, QAM=QAM_CONST_256\n" "\033[m", pRealtekRDVBC->m_auto_fsr);
			QamModeInfo = QAM_QAM_256;
			pRealtekRDVBC->m_auto_qamd = 0x8;
			break;
		case QAM_CONST_512:
			REALTEK_R_DVBC_INFO("\033[1;32;33m" "SR=%u, QAM=QAM_CONST_512\n" "\033[m", pRealtekRDVBC->m_auto_fsr);
			QamModeInfo = QAM_QAM_512;
			break;
		case QAM_CONST_1024:
			REALTEK_R_DVBC_INFO("\033[1;32;33m" "SR=%u, QAM=QAM_CONST_1024\n" "\033[m", pRealtekRDVBC->m_auto_fsr);
			QamModeInfo = QAM_QAM_1024;
			break;
		case QAM_CONST_AUTO:
			REALTEK_R_DVBC_INFO("\033[1;32;33m" "SR=%u, QAM=QAM_CONST_AUTO\n" "\033[m", pRealtekRDVBC->m_auto_fsr);
			QamModeInfo = QAM_QAM_AUTO;
			break;
		default:
			REALTEK_R_DVBC_WARNING("set TV system failed, invalid constellation num %d\n", pSigInfo->dvbc_qam.constellation);
			return TUNER_CTRL_FAIL;
		}
	}
	else
	{
		REALTEK_R_DVBC_INFO("\033[1;32;33m" "[DVBC AUTO MODE]\n" "\033[m");
	}
	
	if (QamModeInfo > 5) {QamModeInfo = 5;}
	
	TSThroughputKBps = (6875 * q_val[QamModeInfo] * 188 / 204); //Default Set to 256QAM
	TsClkKHz = TSThroughputKBps * 107 / 100 / 8;
	DemodTsOutClkCtrl(TsClkKHz);
	pRealtekRDVBC->m_ts_clk_KHz = TsClkKHz;
	REALTEK_R_DVBC_INFO("[SetTvSysEx] throughput_Kbps = %u, TsClkKHz = %u\n", TSThroughputKBps, TsClkKHz);
	
	pRealtekRDVBC->m_acquire_sig_en = 1;
	pDemodKernel->Unlock(pDemodKernel);
	return ret;

SetTVSysEx_fail:
	pDemodKernel->Unlock(pDemodKernel);
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
int REALTEK_R_DVBC_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::SetIF\n");

	pDemodKernel->Lock(pDemodKernel);

	REALTEK_R_DVBC_INFO("SetIF, IF_freq="PT_U32BITS" spectrumn inv=%d, agc_pola=%d\n", pParam->freq, pParam->inversion, pParam->agc_pola);

	ret = DECODE_RET(pDemod->SetIfFreqHz(pDemod, pParam->freq));
	ret |= DECODE_RET(pDemod->SetSpectrumMode(pDemod, (pParam->inversion) ? 1 : 0));
	//rtd_maskl(DVBC_OPT_RF_AAGC_DRIVE_reg, ~DVBC_OPT_RF_AAGC_DRIVE_opt_if_aagc_drive_mask, DVBC_OPT_RF_AAGC_DRIVE_opt_if_aagc_drive(0x1));
	//rtd_maskl(DVBC_OPT_RF_AAGC_DRIVE_reg, ~DVBC_OPT_RF_AAGC_DRIVE_opt_par_if_sd_ib_mask, DVBC_OPT_RF_AAGC_DRIVE_opt_par_if_sd_ib(0x1));

	//set ifagc pola
	//ret |= DECODE_RET(pDemod->SetAagcLoopPol(pDemod, pParam->agc_pola));
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
int REALTEK_R_DVBC_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	REALTEK_R_DVBC* pRealtekRDVBC;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::SetMode\n");

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
int REALTEK_R_DVBC_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	switch (pParam->mode) {
	case PARALLEL_MODE:
		REALTEK_R_DVBC_DBG("TS Mode = PARALLEL\n");
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_SERIAL, 0));
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_CDIV_PH0, 7));
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_CDIV_PH1, 7));
		rtd_maskl(DVBC_OPT_CKOUTPAR_reg, ~DVBC_OPT_CKOUTPAR_opt_serial_mask, DVBC_OPT_CKOUTPAR_opt_serial(0x0));
		rtd_maskl(DVBC_OPT_CDIV_PH0_reg, ~DVBC_OPT_CDIV_PH0_opt_cdiv_ph0_mask, DVBC_OPT_CDIV_PH0_opt_cdiv_ph0(0x7));
		rtd_maskl(DVBC_OPT_CDIV_PH0_reg, ~DVBC_OPT_CDIV_PH0_opt_cdiv_ph1_mask, DVBC_OPT_CDIV_PH0_opt_cdiv_ph1(0x7));
		break;

	case SERIAL_MODE:
		REALTEK_R_DVBC_DBG("TS Mode = SERIAL\n");
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_SERIAL, 1));
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_CDIV_PH0, 0));
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_CDIV_PH1, 0));
		rtd_maskl(DVBC_OPT_CKOUTPAR_reg, ~DVBC_OPT_CKOUTPAR_opt_serial_mask, DVBC_OPT_CKOUTPAR_opt_serial(0x1));
		rtd_maskl(DVBC_OPT_CDIV_PH0_reg, ~DVBC_OPT_CDIV_PH0_opt_cdiv_ph0_mask, DVBC_OPT_CDIV_PH0_opt_cdiv_ph0(0x0));
		rtd_maskl(DVBC_OPT_CDIV_PH0_reg, ~DVBC_OPT_CDIV_PH0_opt_cdiv_ph1_mask, DVBC_OPT_CDIV_PH0_opt_cdiv_ph1(0x0));
		break;

	default:
		ret = TUNER_CTRL_FAIL;
	}

	switch (pParam->clk_pol) {
	case RISING_EDGE:
		REALTEK_R_DVBC_DBG("MPEG CLK POL = RISING_EDGE\n");
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_CKOUTPAR, 1));
		rtd_maskl(DVBC_OPT_CKOUTPAR_reg, ~DVBC_OPT_CKOUTPAR_opt_ckoutpar_mask, DVBC_OPT_CKOUTPAR_opt_ckoutpar(0x1));
		break;

	case FALLING_EDGE:
		REALTEK_R_DVBC_DBG("MPEG CLK POL = FALLING_EDGE\n");
		//ret = DECODE_RET(pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_CKOUTPAR, 0));
		rtd_maskl(DVBC_OPT_CKOUTPAR_reg, ~DVBC_OPT_CKOUTPAR_opt_ckoutpar_mask, DVBC_OPT_CKOUTPAR_opt_ckoutpar(0x0));
		break;

	default:
		ret = TUNER_CTRL_FAIL;
	}

	DtvDemodTPOutEn(pParam->tsOutEn);

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
int REALTEK_R_DVBC_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{
	int Val = 0, Val1 = 0, Val2 = 0, Val3 = 0, cr_rst = 0;
	unsigned int kp_cr = 0, fsm_state_o = 0;
	static unsigned char cnt = 0;
	int ret = TUNER_CTRL_OK;

	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*)pRealtekRDVBC->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	ret = DECODE_RET(pDemod->IsFrameLocked(pDemod, &Val1));
	tv_osal_msleep(5);
	ret = DECODE_RET(pDemod->IsFrameLocked(pDemod, &Val2));
	tv_osal_msleep(5);
	ret = DECODE_RET(pDemod->IsFrameLocked(pDemod, &Val3));

	if(cnt == 5)
	{
		cnt = 0;
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
		fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;

		kp_cr = DVBC_KP_CR_EXP_INC_get_kp_cr_exp_inc(rtd_inl(DVBC_KP_CR_EXP_INC_reg));
		if ((kp_cr & 0x8) == 0x8)
			cr_rst = -(0x10-kp_cr);
		else
			cr_rst = kp_cr;
		
		REALTEK_R_DVBC_INFO("\033[1;32;36m" "fsm_state_o=%u, kp_cr=%d\n" "\033[m", fsm_state_o, cr_rst);
	}
	cnt++;
	
	if (Val1 == 1 || Val2 == 1 || Val3 == 1)
		Val = 1;
	
	if (ret == TUNER_CTRL_OK)
		*pLock = DECODE_LOCK(Val);

	REALTEK_R_DVBC_INFO("\033[1;32;36m" "DVBC GetLock = %u (freq="PT_U32BITS", %d, %d, %d)\n" "\033[m", *pLock, pRealtekRDVBC->m_info.dvbc_qam.freq_khz, Val1, Val2, Val3);

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
int REALTEK_R_DVBC_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	int qam_mode = 0;
	U32BITS spec_mode;
	U32BITS pSymbolRateHz = 0;
	int pAlphaMode = 0;

	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*)pRealtekRDVBC->m_private)->pDemod;

	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::GetSignalInfo\n");

	pDemodKernel->Lock(pDemodKernel);

	pDemod->GetQamMode(pDemod, &qam_mode);

	switch (qam_mode) {
	case QAM_QAM_4:
		pInfo->dvbc_qam.constellation = QAM_CONST_4;
		break;
	case QAM_QAM_16:
		pInfo->dvbc_qam.constellation = QAM_CONST_16;
		break;
	case QAM_QAM_32:
		pInfo->dvbc_qam.constellation = QAM_CONST_32;
		break;
	default:
	case QAM_QAM_64:
		pInfo->dvbc_qam.constellation = QAM_CONST_64;
		break;
	case QAM_QAM_128:
		pInfo->dvbc_qam.constellation = QAM_CONST_128;
		break;
	case QAM_QAM_256:
		pInfo->dvbc_qam.constellation = QAM_CONST_256;
		break;
	case QAM_QAM_512:
		pInfo->dvbc_qam.constellation = QAM_CONST_512;
		break;
	case QAM_QAM_1024:
		pInfo->dvbc_qam.constellation = QAM_CONST_1024;
		break;
	case QAM_QAM_AUTO:
		pInfo->dvbc_qam.constellation = QAM_CONST_AUTO;
		break;
	}

	pDemod->GetSymbolRateHz(pDemod, &pSymbolRateHz);
	pInfo->dvbc_qam.symbol_rate = pSymbolRateHz;

	pDemod->GetAlphaMode(pDemod, &pAlphaMode);
	pInfo->dvbc_qam.alpha = pAlphaMode;

	//pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_SPEC_MODE, &spec_mode);
	spec_mode = DVBC_OPT_DDC_LPF_SCALING_get_specmode(rtd_inl(DVBC_OPT_DDC_LPF_SCALING_reg));
	if (spec_mode) {
		pInfo->qam.spectrum_inverse = QAM_SPEC_INVERSE;
		pInfo->dvbc_qam.spectrum_inverse = QAM_SPEC_INVERSE;
	} else {
		pInfo->qam.spectrum_inverse = QAM_SPEC_NORMAL;
		pInfo->dvbc_qam.spectrum_inverse = QAM_SPEC_NORMAL;
	}

	pInfo->mod = TV_MODULATION_DVBC_QAM;

	REALTEK_R_DVBC_INFO("GetSignalInfo QAM_MODE=%d, SPEC_INV=%d, symbol_rate="PT_U32BITS", alpha=%d\n", pInfo->dvbc_qam.constellation, pInfo->dvbc_qam.spectrum_inverse, pInfo->dvbc_qam.symbol_rate, pInfo->dvbc_qam.alpha);
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
int REALTEK_R_DVBC_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality)
{
	int ret = TUNER_CTRL_OK;
	int FrameLock = 0;
	int LockStatus = 0, TxRSSIdBm = 0;
	U32BITS BerNum = 1, BerDen = 1;
	U32BITS PerNum = 1, PerDen = 1;
	S32BITS IfAgc = -1;
	S32BITS SnrNum, SnrDen;
	U32BITS SignalStrength = 0, SignalQuality = 0;
	int qam_mode = 0;

	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	ret = DECODE_RET(pDemod->IsFrameLocked(pDemod, &FrameLock));

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
			pQuality->snr = (SnrNum * 10000) / SnrDen;
		break;

	case TV_QUAL_BER:
	case TV_QUAL_PER:
	case TV_QUAL_BEC:
	case TV_QUAL_PEC:
	case TV_QUAL_UCBLOCKS:
		ret = (FrameLock) ? DECODE_RET(pDemod->GetErrorRate(pDemod, 0, 800, &BerNum, &BerDen, &PerNum, &PerDen)) : TUNER_CTRL_OK;
		//printf("ber=%f, per=%f\n", (float) BerNum /(float)BerDen, (float) PerNum /(float)PerDen);
		if (id == TV_QUAL_BER)
			pQuality->ber = (FrameLock) ? (BerNum * 10000) / BerDen : 10000;
		else if (id == TV_QUAL_PER)
			pQuality->per = (FrameLock) ? (PerNum * 10000) / PerDen : 10000;
		else if (id == TV_QUAL_BEC) {
			pQuality->layer_ber.ber_num = (FrameLock) ? BerNum : BerDen;
			pQuality->layer_ber.ber_den = BerDen;
		} else if (id == TV_QUAL_PEC) {
			pQuality->layer_per.per_num = (FrameLock) ? PerNum : PerDen;
			pQuality->layer_per.per_den = PerDen;
			REALTEK_R_DVBC_INFO("per="PT_U32BITS" ( ("PT_U32BITS" /"PT_U32BITS") *10000)\n", pQuality->per, pQuality->layer_per.per_num, pQuality->layer_per.per_den);
		}
		pQuality->ucblocks = PerNum;

		break;
	case TV_QUAL_SIGNAL_STRENGTH:
		TxRSSIdBm = (int)pQuality->rflevel;

#ifdef GET_SIGNAL_STRENGTH_FROM_SNR
		ret = DECODE_RET(pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen));

		if (ret == TUNER_CTRL_OK)
			pQuality->snr = (SnrNum * 10000) / SnrDen;

		if (ret == TUNER_CTRL_OK)
			pQuality->strength = Snr2Str(pQuality->snr / 10000); // convert snr to signal strength
#else
		ret = DECODE_RET(pDemod->GetSignalStrength(pDemod, TxRSSIdBm, &SignalStrength));

		if (ret == TUNER_CTRL_OK)
			pQuality->strength = (unsigned char) SignalStrength;
#endif

		break;

	case TV_QUAL_SIGNAL_QUALITY:

		//ret = (FrameLock) ? DECODE_RET(pDemod->GetErrorRate(pDemod,  1, 200, &BerNum, &BerDen, &PerNum, &PerDen)) : TUNER_CTRL_OK;
		//pQuality->ber = (BerNum * 10000) / BerDen;

		//if (ret == TUNER_CTRL_OK)
		//	pQuality->quality = Ber2Quality(pQuality->ber);  // convert ber to signal quality

		ret = DECODE_RET(pDemod->GetSignalQuality(pDemod, &SignalQuality));

		if (ret == TUNER_CTRL_OK)
			pQuality->quality = (unsigned char) SignalQuality;

		break;
	case TV_QUAL_AGC:

		ret = DECODE_RET(pDemod->GetIfAgc(pDemod, &IfAgc));

		if (ret == TUNER_CTRL_OK)
			pQuality->agc = ((IfAgc + 1023) * 100) / 2048;

		break;
	case TV_QUAL_DEMODFWVERSION:
		pQuality->DemodFwVersion = 0x000001;
		break;

	case TV_QUAL_CUSTOMER_LG:
		TxRSSIdBm = (int)pQuality->rflevel;
		ret |= DECODE_RET(pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen));
		ret |= DECODE_RET(pDemod->GetIfAgc(pDemod, &IfAgc));
		ret |= DECODE_RET(pDemod->GetSignalStrength(pDemod, TxRSSIdBm, &SignalStrength));
		ret |= DECODE_RET(pDemod->GetSignalQuality(pDemod, &SignalQuality));
		ret |= (FrameLock) ? DECODE_RET(pDemod->GetErrorRate(pDemod,  0, 800, &BerNum, &BerDen, &PerNum, &PerDen)) : TUNER_CTRL_OK;


		if (ret == TUNER_CTRL_OK) {
			pQuality->snr = (SnrNum * 10000) / SnrDen;
			pQuality->agc = ((IfAgc + 1023) * 100) / 2048;
#ifdef GET_SIGNAL_STRENGTH_FROM_SNR
			pQuality->strength = Snr2Str(pQuality->snr / 10000); // convert snr to signal strength
#else
			pQuality->strength = (unsigned char) SignalStrength;
#endif

			pQuality->ber = (FrameLock) ? (BerNum * 10000) / BerDen : 10000;
			pQuality->per = (FrameLock) ? (PerNum * 10000) / PerDen : 10000;
			pQuality->layer_ber.ber_num = (FrameLock) ? BerNum : BerDen;
			pQuality->layer_ber.ber_den = BerDen;
			pQuality->layer_per.per_num = (FrameLock) ? PerNum : PerDen;
			pQuality->layer_per.per_den = PerDen;
			//pQuality->quality = Ber2Quality(pQuality->ber);  // convert ber to signal quality
			pQuality->quality = (unsigned char) SignalQuality;

			pDemod->GetQamMode(pDemod, &qam_mode);
		}

		REALTEK_R_DVBC_INFO("[GetSignalQuality] snr = "PT_U32BITS", agc = "PT_U32BITS", SSI = %u, SQI = %u, ber = "PT_U32BITS" (("PT_U32BITS" /"PT_U32BITS")*10000), per="PT_U32BITS" ( ("PT_U32BITS" /"PT_U32BITS") *10000)\n", pQuality->snr, pQuality->agc, pQuality->strength, pQuality->quality, pQuality->ber, pQuality->layer_ber.ber_num, pQuality->layer_ber.ber_den, pQuality->per, pQuality->layer_per.per_num, pQuality->layer_per.per_den);

		break;

	default:
		break;
	}

	pDemodKernel->Unlock(pDemodKernel);

	return TUNER_CTRL_OK;
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
int REALTEK_R_DVBC_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	int ret = TUNER_CTRL_OK;
	unsigned char LockStatus = 0;
	unsigned int CcfoInitFreq = 0;
	int est_ccfo = 0;

	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	pDemodKernel->GetLockStatus(pDemodKernel, &LockStatus);

	if (LockStatus == 0) {
		REALTEK_R_DVBC_WARNING("GetCarrierOffset fail beacause demod is unlock\n");
		return TUNER_CTRL_FAIL;
	}

	pDemodKernel->Lock(pDemodKernel);

	ret = DECODE_RET(pDemod->GetCrOffsetHz(pDemod, pOffset));

	if (DVBC_OPT_START_SRS_NBD_get_opt_ccfo_fft_method_en(rtd_inl(DVBC_OPT_START_SRS_NBD_reg)))
		pDemod->GetCcfoInitFreq(pDemod, pRealtekRDVBC->m_symbol_rate, &CcfoInitFreq, &est_ccfo);

	REALTEK_R_DVBC_INFO("CrOffset="PT_S32BITS"Hz est_ccfo=%dHz CarrierOffset="PT_S32BITS"Hz\n", *pOffset, est_ccfo, *pOffset + est_ccfo);
	if (ret == TUNER_CTRL_OK) {
		/*----------------------------------------------------
		  CrOffset = Current Frequency - Target Frequency
		  returned offset = Target Freq - Current Offset = -CrOffset
		  Note: RTD299S QAM Demod will handle IF polarity automatically
		-----------------------------------------------------*/
		//*pOffset *= -1;
		*pOffset = *pOffset + est_ccfo; //Hz
	} else {
		*pOffset = 0;
	}

	if (pDemodKernel->m_if.inversion == 1) *pOffset *= -1;

	pDemodKernel->Unlock(pDemodKernel);
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
int REALTEK_R_DVBC_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

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
int REALTEK_R_DVBC_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned long cur_time;
#endif

	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	//if (WaitSignalLock == 0)
	//	return TUNER_CTRL_OK;

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	cur_time = tv_osal_time();
	while ((tv_osal_time() - cur_time) <= 15) {

		if (pRealtekRDVBC->m_acquire_sig_en == 0)
			return TUNER_CTRL_OK;

		tv_osal_msleep(5);
	}
	return TUNER_CTRL_FAIL;

#else
	REALTEK_R_DVBC_AcquireSignalThread(pDemodKernel);
	REALTEK_R_DVBC_TSBitRateCheck(pDemodKernel);
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
#define DVBC_ACQ_TR_CNT 4
int REALTEK_R_DVBC_AcquireSignalThread(DEMOD* pDemodKernel)
{
	int lock = 0, lock_chk1 = 0, lock_chk2 = 0, lock_chk3 = 0;
	unsigned long stime = 0, auto_mode_etime = 0;
	unsigned int fsm_state_o = 0, tr_ld_o = 0;
	S32BITS snr = 0, SnrNum = 0, SnrDen = 0;
	unsigned char ccfo_acq_try = 0;
	unsigned char IsNoSignal = 0, Qamd = 0;
	U32BITS Coarse_SymbolRate = 0, Fine_SymbolRate = 0;
	S32BITS est_ccfo = 0;
	int QamModeInfo = QAM_QAM_256; //Default Set to 256QAM
	unsigned int TSThroughputKBps = 96000, TsClkKHz = 12000;
	unsigned char q_val[] = {0, 4, 5, 6, 7, 8}; // *QPSK suit for qam mode defined, 16QAM, 32QAM, 64QAM, 128QAM, 256QAM
	int ret = TUNER_CTRL_OK;

	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;

	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	REALTEK_R_DVBC_DBG("\033[1;32;32m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);

	if (pRealtekRDVBC->m_acquire_sig_en == 0)
		return TUNER_CTRL_OK;

	pDemodKernel->Lock(pDemodKernel);
	
	pRealtekRDVBC->m_acquire_sig_en = 0;
	
	tv_osal_msleep(pDemod->Symbolratedelay);//AGC stable issue
	
	ret = DECODE_RET(pDemod->AutoMode(pDemod, &IsNoSignal, &Coarse_SymbolRate, &Fine_SymbolRate, &est_ccfo, &Qamd));
	auto_mode_etime = tv_osal_time();
	pRealtekRDVBC->m_IsNoSignal = IsNoSignal;
	
	if(pDemodKernel->m_ScanMode == CH_MANUAL_MODE)
	{
		if(pRealtekRDVBC->m_auto_qamd != Qamd || ABS(pRealtekRDVBC->m_auto_fsr, Fine_SymbolRate) > 30000)
		{
			REALTEK_R_DVBC_INFO("CH_MANUAL_MODE (m_auto_qamd != Qamd || SR diff > 30000)\n");
			ret = DECODE_RET(pDemod->AutoModeSetSrHz(pDemod, 1000));//force sr to 1K
			IsNoSignal = 1;
		}
	}

	if(IsNoSignal == 1)
	{
		REALTEK_R_DVBC_INFO("\033[1;32;31m" "DVBC NO SIGNAL!!! Freq="PT_U32BITS", Total Acq Time="PT_U32BITS", AutoMode Time="PT_U32BITS", IsNoSignal=%u, CSR="PT_S32BITS" KHz, FSR="PT_S32BITS" Hz, CCFO="PT_S32BITS" Hz, QamMode=0x%x\n" "\033[m",
			pRealtekRDVBC->m_info.dvbc_qam.freq_khz, tv_osal_time() - pRealtekRDVBC->m_acquire_stime, auto_mode_etime - pRealtekRDVBC->m_acquire_stime, IsNoSignal, Coarse_SymbolRate, Fine_SymbolRate, est_ccfo, Qamd);
		goto AcquireSignal_end;
	}

	pRealtekRDVBC->m_auto_fsr = Fine_SymbolRate;
	pRealtekRDVBC->m_auto_ccfo = est_ccfo;
	pRealtekRDVBC->m_auto_qamd = Qamd;

	switch(Qamd)
	{
		case 4://16QAM
			ret = DECODE_RET(pDemod->SetQamMode(pDemod, QAM_QAM_16));
			QamModeInfo = QAM_QAM_16;
			break;
		case 5://32QAM
			ret = DECODE_RET(pDemod->SetQamMode(pDemod, QAM_QAM_32));
			QamModeInfo = QAM_QAM_32;
			break;
		case 6://64QAM
			ret = DECODE_RET(pDemod->SetQamMode(pDemod, QAM_QAM_64));
			QamModeInfo = QAM_QAM_64;
			break;
		case 7://128QAM
			ret = DECODE_RET(pDemod->SetQamMode(pDemod, QAM_QAM_128));
			QamModeInfo = QAM_QAM_128;
			break;
		case 8://256QAM
		default:
			ret = DECODE_RET(pDemod->SetQamMode(pDemod, QAM_QAM_256));
			QamModeInfo = QAM_QAM_256;
			break;
	}
	ret = DECODE_RET(pDemod->AutoModeSetSrHz(pDemod, Fine_SymbolRate));
	
	TSThroughputKBps = ((pRealtekRDVBC->m_auto_fsr / 1000) * q_val[QamModeInfo] * 188 / 204);
	TsClkKHz = TSThroughputKBps * 107 / 100 / 8;
	DemodTsOutClkCtrl(TsClkKHz);
	pRealtekRDVBC->m_ts_clk_KHz = TsClkKHz;
	REALTEK_R_DVBC_INFO("[AcquireSignal] throughput_Kbps = %u, TsClkKHz = %u\n", TSThroughputKBps, TsClkKHz);
	
	ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));
	
	stime = tv_osal_time();
	while ((tv_osal_time() - stime) <= 1500) {
		//ccfo patch
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));//stb INRX_MO
		fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;
		tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));

		if((tv_osal_time() - stime) > 60 && (fsm_state_o >= 6) && (tr_ld_o >= 32) && (pRealtekRDVBC->m_auto_qamd == 0x5 || pRealtekRDVBC->m_auto_qamd == 0x6 || pRealtekRDVBC->m_auto_qamd == 0x7))//128&32&64 QAM retry
		{
			ccfo_acq_try++;
			if(ccfo_acq_try <= DVBC_ACQ_TR_CNT)
			{
				REALTEK_R_DVBC_INFO("\033[1;32;31m" "32QAM/64QAM/128QAM TR not correct in Acq, retry %u times, SetQamAcq fsm_state=%u, tr=%u\n" "\033[m", ccfo_acq_try, fsm_state_o, tr_ld_o);
				ret |= DECODE_RET(pDemod->SoftwareReset(pDemod));
				tv_osal_msleep(100);
				stime = tv_osal_time();
			}
			else
			{
				REALTEK_R_DVBC_INFO("32QAM/64QAM/128QAM TR not correct in Acq retry over %d times\n", DVBC_ACQ_TR_CNT);
				REALTEK_R_DVBC_INFO("\033[1;32;31m" "DVBC UNLOCK!!! Freq="PT_U32BITS", Total Acq Time="PT_U32BITS", Local Time="PT_U32BITS", fsm_state=%u, tr=%u\n" "\033[m", 
					pRealtekRDVBC->m_info.dvbc_qam.freq_khz, tv_osal_time() - pRealtekRDVBC->m_acquire_stime, tv_osal_time() - stime, fsm_state_o, tr_ld_o);
				goto AcquireSignal_fail;
			}
		}

		//pDemod->IsNoSig(pDemod, &IsTrLocked);
		
		pDemod->IsFrameLocked(pDemod, &lock_chk1);
		if(lock_chk1 == 1)
		{
			tv_osal_msleep(5);
			pDemod->IsFrameLocked(pDemod, &lock_chk2);
			tv_osal_msleep(5);
			pDemod->IsFrameLocked(pDemod, &lock_chk3);
			REALTEK_R_DVBC_INFO("\033[1;32;34m" "lock_chk %u, %u, %u\n" "\033[m", lock_chk1, lock_chk2, lock_chk3);
		}
		else
		{
			REALTEK_R_DVBC_INFO("\033[1;32;34m" "first lock_chk %u\n" "\033[m", lock_chk1);
		}
		
		if(lock_chk1 == 1 && lock_chk2 == 1 && lock_chk3 == 1)
		{
			lock = 1;
		}

		if(lock == 1)
		{
			pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen);
			snr = (SnrNum * 10000) / SnrDen;
			
			if(pDemodKernel->m_ScanMode != CH_MANUAL_MODE)
			{
				REALTEK_R_DVBC_INFO("\033[1;32;36m" "DVBC LOCK!!! Freq="PT_U32BITS", Total Acq Time="PT_U32BITS", AutoMode Time="PT_U32BITS", Local Time="PT_U32BITS", IsNoSignal=%u, CSR="PT_S32BITS" KHz, FSR="PT_S32BITS" Hz, CCFO="PT_S32BITS" Hz, QamMode=0x%x, Snr="PT_S32BITS"/10000, fsm_state=%u, tr=%u\n" "\033[m", 
					pRealtekRDVBC->m_info.dvbc_qam.freq_khz, tv_osal_time() - pRealtekRDVBC->m_acquire_stime, auto_mode_etime - pRealtekRDVBC->m_acquire_stime, tv_osal_time() - stime, IsNoSignal, Coarse_SymbolRate, Fine_SymbolRate, est_ccfo, Qamd, snr, fsm_state_o, tr_ld_o);
			}
			else
			{
				REALTEK_R_DVBC_INFO("\033[1;32;36m" "DVBC LOCK!!! Freq="PT_U32BITS", Total Acq Time="PT_U32BITS", Local Time="PT_U32BITS", Snr="PT_S32BITS"/10000, fsm_state=%u, tr=%u\n" "\033[m", 
					pRealtekRDVBC->m_info.dvbc_qam.freq_khz, tv_osal_time() - pRealtekRDVBC->m_acquire_stime, tv_osal_time() - stime, snr, fsm_state_o, tr_ld_o);
			}
			//pRealtekRDVBC->m_auto_tune_en = 1;
			OutErrorCnt = 0;
			break;
		}
		tv_osal_msleep(5);
	}
	if(lock == 0)
	{
		if(pDemodKernel->m_ScanMode != CH_MANUAL_MODE)
		{
			REALTEK_R_DVBC_INFO("\033[1;32;31m" "DVBC UNLOCK!!! Freq="PT_U32BITS", Total Acq Time="PT_U32BITS", AutoMode Time="PT_U32BITS", Local Time="PT_U32BITS", IsNoSignal=%u, CSR="PT_S32BITS" KHz, FSR="PT_S32BITS" Hz, CCFO="PT_S32BITS" Hz, QamMode=0x%x, Snr="PT_S32BITS"/10000, fsm_state=%u, tr=%u\n" "\033[m", 
				pRealtekRDVBC->m_info.dvbc_qam.freq_khz, tv_osal_time() - pRealtekRDVBC->m_acquire_stime, auto_mode_etime - pRealtekRDVBC->m_acquire_stime, tv_osal_time() - stime, IsNoSignal, Coarse_SymbolRate, Fine_SymbolRate, est_ccfo, Qamd, snr, fsm_state_o, tr_ld_o);
		}
		else
		{
			REALTEK_R_DVBC_INFO("\033[1;32;31m" "DVBC UNLOCK!!! Freq="PT_U32BITS", Total Acq Time="PT_U32BITS", Local Time="PT_U32BITS", Snr="PT_S32BITS"/10000, fsm_state=%u, tr=%u\n" "\033[m", 
				pRealtekRDVBC->m_info.dvbc_qam.freq_khz, tv_osal_time() - pRealtekRDVBC->m_acquire_stime, tv_osal_time() - stime, snr, fsm_state_o, tr_ld_o);
		}
		//pRealtekRDVBC->m_auto_tune_en = 1;
	}

AcquireSignal_end:
	DtvDemodTPOutEn(1); //enable TS output
	pRealtekRDVBC->m_autotune_stime = tv_osal_time();
	pRealtekRDVBC->m_autotune_GetLockTime = tv_osal_time();
	pRealtekRDVBC->m_auto_tune_en = 1;
	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_OK;

AcquireSignal_fail:
	//REALTEK_R_DVBC_INFO("\033[1;32;31m" "DVBC UNLOCK!!! Freq="PT_U32BITS", Total Acq Time="PT_U32BITS", Local Time="PT_U32BITS", fsm_state=%u, tr=%u\n" "\033[m", 
	//	pRealtekRDVBC->m_info.dvbc_qam.freq_khz, tv_osal_time() - pRealtekRDVBC->m_acquire_stime, tv_osal_time() - stime, fsm_state_o, tr_ld_o);
	DtvDemodTPOutEn(1); //enable TS output
	pRealtekRDVBC->m_autotune_stime = tv_osal_time();
	pRealtekRDVBC->m_autotune_GetLockTime = tv_osal_time();
	pRealtekRDVBC->m_auto_tune_en = 1;
	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_FAIL;
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
int REALTEK_R_DVBC_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range)
{
	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

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
int REALTEK_R_DVBC_AutoTune(DEMOD* pDemodKernel)
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
int REALTEK_R_DVBC_KernelAutoTune(DEMOD* pDemodKernel)
{
	unsigned char Lock = 0;
	U32BITS BerNum = 1, BerDen = 1;
	U32BITS PerNum = 1, PerDen = 1;
	static unsigned char rst_cnt = 0;
	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	if (pRealtekRDVBC->m_acquire_sig_en) {
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
		REALTEK_R_DVBC_AcquireSignalThread(pDemodKernel);
		REALTEK_R_DVBC_TSBitRateCheck(pDemodKernel);
#endif
	}

	if(pRealtekRDVBC->m_auto_tune_en)
	{
		if((tv_osal_time() - pRealtekRDVBC->m_autotune_stime) > AUTODETECT_DVBC_SIG_UPDATE_INTERVAL_MS)
		{
			REALTEK_R_DVBC_GetLockStatus(pDemodKernel, &Lock);
			if(Lock == 0)
			{
				REALTEK_R_DVBC_INFO("[DVBC] KernelAutoTune AUTO DETECT FLOW !!!\n");
				pRealtekRDVBC->m_acquire_sig_en = 1;
				pRealtekRDVBC->m_acquire_stime = tv_osal_time();
				REALTEK_R_DVBC_AcquireSignalThread(pDemodKernel);
				REALTEK_R_DVBC_TSBitRateCheck(pDemodKernel);
			}
			pRealtekRDVBC->m_autotune_stime = tv_osal_time();
		}
		else if((tv_osal_time() - pRealtekRDVBC->m_autotune_GetLockTime) > DVBC_STATUS_CHECK_INTERVAL_MS)
		{
			REALTEK_R_DVBC_GetLockStatus(pDemodKernel, &Lock);
			if(Lock == 1)
			{
				pDemod->GetErrorRate(pDemod,  0, 800, &BerNum, &BerDen, &PerNum, &PerDen);
				if((PerNum == PerDen) && (PerNum != 0))
					OutErrorCnt++;

				if(PerNum != 0)
				{
					REALTEK_R_DVBC_INFO("\033[1;32;33m" "DVBC Packet Err = "PT_S32BITS" / "PT_S32BITS"\n" "\033[m", PerNum, PerDen);
				}
				else
				{
					REALTEK_R_DVBC_INFO("\033[1;32;36m" "DVBC Packet Err = "PT_S32BITS" / "PT_S32BITS"\n" "\033[m", PerNum, PerDen);
				}
			}
			pRealtekRDVBC->m_autotune_GetLockTime = tv_osal_time();
		}

		if(rst_cnt == 0)
		{
			rst_cnt = 12;
			if(OutErrorCnt > 2)
			{
				REALTEK_R_DVBC_INFO("\033[1;32;36m" "[DVBC] Re-acq !!! OutErrorCnt = %u\n" "\033[m", OutErrorCnt);
				OutErrorCnt = 0;
				pDemodKernel->Lock(pDemodKernel);
				pDemod->SoftwareReset(pDemod);
				pDemodKernel->Unlock(pDemodKernel);
				tv_osal_msleep(30);
			}
		}
		rst_cnt--;
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
int REALTEK_R_DVBC_InspectSignal(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	S32BITS IfAgc = -1;
	S32BITS RfAgc = -1;
	S32BITS TrOffsetPpm = -1;
	S32BITS CrOffsetHz  = -1;
	U32BITS DiAgc = 0;
	int AgcLock   = 0;
	int EqLock    = 0;
	int FrameLock = 0;
	U32BITS BerNum = 1;
	U32BITS BerDen = 1;
	U32BITS PerNum = 1;
	U32BITS PerDen = 1;
	S32BITS SnrDbNum = 1;
	S32BITS SnrDbDen = 1;

	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::InspectSignal\n");
	if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DVBC))
		return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);

	pDemod->GetRfAgc(pDemod, &RfAgc);
	pDemod->GetIfAgc(pDemod, &IfAgc);
	pDemod->GetDiAgc(pDemod, &DiAgc);

	pDemod->GetTrOffsetPpm(pDemod, &TrOffsetPpm);
	pDemod->GetCrOffsetHz(pDemod, &CrOffsetHz);
	pDemod->IsAagcLocked(pDemod, &AgcLock);
	pDemod->IsEqLocked(pDemod, &EqLock);
	pDemod->IsFrameLocked(pDemod, &FrameLock);
	pDemod->GetErrorRate(pDemod, 0, 800, &BerNum, &BerDen, &PerNum, &PerDen);
	pDemod->GetSnrDb(pDemod, &SnrDbNum, &SnrDbDen);

	pDemodKernel->Unlock(pDemodKernel);

	IfAgc = ((IfAgc + 1023) * 100) / 2048;
	RfAgc = ((RfAgc + 1023) * 100) / 2048;
	DiAgc = (DiAgc * 100) / 65536;

	REALTEK_R_DVBC_INFO("[InspectSignal] lock(agc/eq/frame)=%d/%d/%d, agc(rf/if/di)="PT_S32BITS"/"PT_S32BITS"/"PT_U32BITS", ofst(tr/cr)="PT_S32BITS"/"PT_S32BITS", error_rate(ber/per)="PT_S32BITS"/"PT_S32BITS", snr="PT_S32BITS"\n",
						AgcLock, EqLock, FrameLock,
						RfAgc, IfAgc, DiAgc,
						TrOffsetPpm, CrOffsetHz,
						(BerNum * 10000) / BerDen,
						(PerNum * 10000) / PerDen,
						(SnrDbNum * 10000) / SnrDbDen);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : Activate
 *
 * Desc : Activate RTK_DEMOD_DVBC Demod
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_DVBC_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{
	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::Activate\n");
	pDemodKernel->Init(pDemodKernel);

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetTvMode
 *
 * Desc : Set Mode of RTK_DEMOD_DVBC
 *
 * Parm : mode :    RTK_DEMOD_MODE_DVBC : DVBC mode
 *                  RTK_DEMOD_MODE_DVBC  : DVBC mode
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_R_DVBC_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;
	REALTEK_R_DVBC* pRealtekRDVBC;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);

	//FUNCTION_NAME_PRINT("REALTEK_R_DVBC::SetTvMode\n");
	pDemodKernel->Lock(pDemodKernel);

	REALTEK_R_DVBC_INFO("REALTEK_R_DVBC::SetTvMode(%d)\n", mode);

	switch (mode) {

	case TV_SYS_TYPE_DVBC:

		REALTEK_R_DVBC_INFO("Set to DVBC\n");
		ret = pDemodKernel->Init(pDemodKernel);
		ret |= pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts);

		if (ret != TUNER_CTRL_OK) {
			REALTEK_R_DVBC_WARNING("Set REALTEK_R_DVBC to DVBC mode failed\n");
			goto set_demod_mode_failed;
		}

		break;

	default:
		goto set_demod_mode_failed;
	}

	ret = TUNER_CTRL_OK;
	REALTEK_R_DVBC_INFO("Set REALTEK_R_DVBC to appointed DTV mode successed\n");

	pDemodKernel->Unlock(pDemodKernel);
	return ret;

set_demod_mode_failed:
	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_FAIL;
}


int REALTEK_R_DVBC_TSBitRateCheck(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	int LockStatus = 0;
	unsigned int TSThroughputKBps = 96000, TsClkKHz = 12000;

	REALTEK_R_DVBC* pRealtekRDVBC;
	QAM_DEMOD_MODULE* pDemod;
	pRealtekRDVBC = (REALTEK_R_DVBC*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_R_DVBC_DRIVER_DATA*) pRealtekRDVBC->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	ret |= DECODE_RET(pDemod->IsFrameLocked(pDemod, &LockStatus));

	if (LockStatus != 1)
		goto ts_clk_check_done;

	ret |= DECODE_RET(pDemod->GetTsThroughput(pDemod, &TSThroughputKBps));

	if (ret != TUNER_CTRL_OK)
		goto ts_clk_check_done;

	TsClkKHz = TSThroughputKBps * 107 / 100 / 8;

	if (ABS(TsClkKHz, pRealtekRDVBC->m_ts_clk_KHz) > 50) {
		DemodTsOutClkCtrl(TsClkKHz);
		pRealtekRDVBC->m_ts_clk_KHz = TsClkKHz;
		REALTEK_R_DVBC_INFO("[TSBitRateCheck] throughput_Kbps = %u, TsClkKHz = %u\n", TSThroughputKBps, TsClkKHz);
	}

ts_clk_check_done:
	pDemodKernel->Unlock(pDemodKernel);
	return ret;
}


void REALTEK_R_DVBC_Lock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("LOCK 0x%x\n",&pDemodKernel->m_lock);
	//mutex_lock(&pDemodKernel->m_lock);
}


void REALTEK_R_DVBC_Unlock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("UN_LOCK 0x%x\n",&pDemodKernel->m_lock);
	//mutex_unlock(&pDemodKernel->m_lock);
}


/*-----------------------------------------------------------------
 *     BASE_INTERFACE for REALTEK_R_DVBC API
 *----------------------------------------------------------------*/
void ReleaseRealtekRDvbcDriver(REALTEK_R_DVBC_DRIVER_DATA *pDriver)
{
	dvr_free(pDriver);
}


REALTEK_R_DVBC_DRIVER_DATA* AllocRealtekRDvbcDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS             CrystalFreq
)
{
	REALTEK_R_DVBC_DRIVER_DATA* pDriver = (REALTEK_R_DVBC_DRIVER_DATA*) dvr_malloc(
			sizeof(REALTEK_R_DVBC_DRIVER_DATA) +
			sizeof(QAM_DEMOD_MODULE)   +
			sizeof(I2C_BRIDGE_MODULE)   +
			sizeof(BASE_INTERFACE_MODULE));

	//FUNCTION_NAME_PRINT("AllocRealtekRDvbcDriver\n");
	if (pDriver) {
		BASE_INTERFACE_MODULE* pBIF;
		QAM_DEMOD_MODULE* pDemod;
		memset(pDriver, 0, sizeof(REALTEK_R_DVBC_DRIVER_DATA));

		pDriver->pDemod                = (QAM_DEMOD_MODULE*)(((unsigned char*)pDriver) + sizeof(REALTEK_R_DVBC_DRIVER_DATA));
		pDriver->pBaseInterface        = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(QAM_DEMOD_MODULE));
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
		BuildRealtekRQamModule(&pDemod,
							   pDriver->pDemod,
							   pDriver->pBaseInterface,
							   pDriver->pI2CBridge,
							   Addr,
							   CrystalFreq,
							   REALTEK_R_QAM_CONFIG_DVBC); // Spectrum mode is inverse.

	}

	return pDriver;
}

