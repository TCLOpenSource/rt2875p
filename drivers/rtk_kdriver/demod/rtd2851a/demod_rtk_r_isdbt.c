/***************************************************************************************************
  File        : demod_rtk_r.cpp
  Description : RTK_R ISDB-T demod
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
//#include <assert.h>
#include <linux/pageremap.h>
#include "foundation.h"
#include "demod_rtk_r_isdbt.h"
#include "isdbt_demod_rtk_r.h"
#include "demod_rtk_r_isdbt_priv.h"

#define SNR_THD_FOR_FRM_LOCK 300000
#define SNR_THD_FOR_CH19 100000
#define BY_RSBER_VALID	0
#define BY_FRM_LOCK		1
#if USE_PREFRM_LOCK
#define	BY_PREFRM_LOCK	2
#define PREFRM_MISSING_MAX_CHANCE	2
#define DBPSK_Z_AVG_RATIO	1000
#define DBPSK_N_AVG_RATIO	31103
#endif

static unsigned long acquire_sig_stime;
static unsigned char IsCheckLockBy = BY_RSBER_VALID;
static unsigned char MaxSeglayer = 0;
#if USE_PREFRM_LOCK
static unsigned char gPrefrmMissingChance = 0;
#endif
#if USE_PREFRM_LOCK
typedef struct _s_IsdbtPrefrmPollingNumMapping {
	unsigned int trans_mode;
	unsigned int guard_interval;
	unsigned int PollingNum;
} s_IsdbtPrefrmPollingNumMapping;

static const s_IsdbtPrefrmPollingNumMapping _gsIsdbtPrefrmPollingNumMappingTable[] = {
	{ISDBT_FFT_MODE_2K,  ISDBT_GUARD_INTERVAL_1_OVER_4,    4},
	{ISDBT_FFT_MODE_2K,  ISDBT_GUARD_INTERVAL_1_OVER_8,    3},
	{ISDBT_FFT_MODE_2K,  ISDBT_GUARD_INTERVAL_1_OVER_16,   3},
	{ISDBT_FFT_MODE_2K,  ISDBT_GUARD_INTERVAL_1_OVER_32,   3},

	{ISDBT_FFT_MODE_4K,  ISDBT_GUARD_INTERVAL_1_OVER_4,    8},
	{ISDBT_FFT_MODE_4K,  ISDBT_GUARD_INTERVAL_1_OVER_8,    7},
	{ISDBT_FFT_MODE_4K,  ISDBT_GUARD_INTERVAL_1_OVER_16,   7},
	{ISDBT_FFT_MODE_4K,  ISDBT_GUARD_INTERVAL_1_OVER_32,   7},

	{ISDBT_FFT_MODE_8K,  ISDBT_GUARD_INTERVAL_1_OVER_4,    17},
	{ISDBT_FFT_MODE_8K,  ISDBT_GUARD_INTERVAL_1_OVER_8,    14},
	{ISDBT_FFT_MODE_8K,  ISDBT_GUARD_INTERVAL_1_OVER_16,   14},
	{ISDBT_FFT_MODE_8K,  ISDBT_GUARD_INTERVAL_1_OVER_32,   13},
};

#define ISDBT_PREFRM_POLLING_NUM_TABLE_SIZE sizeof(_gsIsdbtPrefrmPollingNumMappingTable)/sizeof(s_IsdbtPrefrmPollingNumMapping)
#endif
/*
//#define DEF_MEM_SIZE 0x10000000
#define DEF_MEM_SIZE 0x0

void*   pli_allocContinuousMemoryMesg(char *str, size_t size, unsigned char** nonCachedAddr, unsigned long *phyAddr)
{
// ^^^^^^^^^^^^^^^^^^^^^^^^^^  ERROR   need to modify
       char *ptr;
	//int oldPriority, iHw_alloc_size;
	unsigned char *nonCachedAddr_local = 0;
	unsigned long phyAddr_local = 0;

	if (size <= 0)
		return 0;

	size &= 0x0fffffff;

	ptr = (char *)pli_malloc(size, 0);

	if (!ptr) {
		RTK_R_ISDB_WARNING("[PLI] allocation failure... Allocate size = %d\n", size);
		return 0;
	}

	nonCachedAddr_local = (unsigned char *)(ptr + DEF_MEM_SIZE);
	phyAddr_local = (unsigned long *)(ptr + DEF_MEM_SIZE);
	RTK_R_ISDB_INFO("realtek_demod pli_allocContinuousMemoryMesg size=0x%x ptr=0x%p nonCachedAddr_local=0x%p phyAddr_local=0x%lx DEF_MEM_SIZE=0x%x\n", size, ptr, nonCachedAddr_local, phyAddr_local, DEF_MEM_SIZE);

	if (nonCachedAddr)
		*nonCachedAddr = nonCachedAddr_local;
	if (phyAddr)
		*phyAddr = phyAddr_local;

	return ptr;

}

void    pli_freeContinuousMemoryMesg(char *str, void *ptr)
{
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^  ERROR   need to modify
	kfree(ptr);
}

*/

/*----------------------------------------------------------------------
 * Func : RTK_R_ISDBT
 *
 * Desc : constructor
 *
 * Parm : device_addr : device address
 *        output_mode : RTK_R_OUT_IF_SERIAL / RTK_R_OUT_IF_PARALLEL
 *        output_freq : RTK_R_OUT_FREQ_6M / RTK_R_OUT_FREQ_10M
 *        pComm       : hadle of COMM
 *
 * Retn : handle of demod
 *----------------------------------------------------------------------*/
void RTK_R_ISDBT_Constructors(DEMOD* pDemodKernel, unsigned char addr, 	unsigned char output_mode, unsigned char output_freq, COMM* pComm)
{
	RTK_R_ISDB* pRtkrISDB;
	pDemodKernel->private_data = kmalloc(sizeof(RTK_R_ISDB), GFP_KERNEL);
	if(pDemodKernel->private_data == NULL){
		goto kmalloc_fail;
	}

	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	Demod_Constructors(pDemodKernel);

	pDemodKernel->m_id                 = DTV_DEMOD_REALTEK_R_ISDBT;
	pDemodKernel->m_addr               = addr;
	pDemodKernel->m_pTuner             = NULL;
	pDemodKernel->m_pComm              = pComm;
	pDemodKernel->m_ScanRange          = SCAN_RANGE_3_6;
	pDemodKernel->m_Capability         = TV_SYS_TYPE_ISDBT;
	pDemodKernel->m_update_interval_ms = 300;
	pDemodKernel->m_if.freq            = 36125000;
	pDemodKernel->m_if.inversion       = 1;
	pDemodKernel->m_if.agc_pola        = 0;
	pDemodKernel->m_clock              = CRYSTAL_FREQ_27000000HZ;
	pRtkrISDB->m_output_freq        = output_freq;

	pRtkrISDB->m_private            = (void*) AllocRtkrDriver_ISDBT(
										  pDemodKernel->m_pComm,
										  pDemodKernel->m_addr,
										  pDemodKernel->m_clock,
										  (output_mode == RTK_DEMOD_OUT_IF_PARALLEL) ? TS_INTERFACE_PARALLEL : TS_INTERFACE_SERIAL);


	pDemodKernel->m_ts.mode            = (output_mode == RTK_DEMOD_OUT_IF_PARALLEL) ? PARALLEL_MODE : SERIAL_MODE;
	pDemodKernel->m_ts.data_order      = MSB_D7;
	pDemodKernel->m_ts.datapin         = MSB_FIRST;
	pDemodKernel->m_ts.err_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.sync_pol        = ACTIVE_HIGH;
	pDemodKernel->m_ts.val_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.clk_pol         = RISING_EDGE;
	pDemodKernel->m_ts.internal_demod_input = 1;
	pRtkrISDB->m_low_snr_detect     = 0;
	pRtkrISDB->m_low_snr_recover    = 0;
	pRtkrISDB->m_auto_tune_enable   = 0;

	pRtkrISDB->m_TunerOptRegCnt     = 0;
	pRtkrISDB->m_pTunerOptReg       = NULL;

	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "RTK_R_ISDBT");

	pDemodKernel->Init = RTK_R_ISDBT_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = RTK_R_ISDBT_AcquireSignal;
	pDemodKernel->ScanSignal = RTK_R_ISDBT_ScanSignal;
	pDemodKernel->SetIF = RTK_R_ISDBT_SetIF;
	pDemodKernel->SetTSMode = RTK_R_ISDBT_SetTSMode;
	pDemodKernel->SetTVSysEx = RTK_R_ISDBT_SetTVSysEx;
	pDemodKernel->SetMode = RTK_R_ISDBT_SetMode;
	pDemodKernel->SetTVSys = RTK_R_ISDBT_SetTVSys;
	pDemodKernel->GetLockStatus = RTK_R_ISDBT_GetLockStatus;
	pDemodKernel->GetSegmentNum = RTK_R_ISDBT_GetSegmentNum;
	pDemodKernel->GetSignalInfo = RTK_R_ISDBT_GetSignalInfo;
	pDemodKernel->GetSignalQuality = RTK_R_ISDBT_GetSignalQuality;
	pDemodKernel->GetT2PLPInfo = Demod_GetT2PLPInfo;
	pDemodKernel->GetT2L1PreInfo = Demod_GetT2L1PreInfo;
	pDemodKernel->GetT2L1PostInfo = Demod_GetT2L1PostInfo;
	pDemodKernel->GetDemodStatus = Demod_GetDemodStatus;
	pDemodKernel->GetCarrierOffset = RTK_R_ISDBT_GetCarrierOffset;
	pDemodKernel->ForwardI2C = RTK_R_ISDBT_ForwardI2C;
	pDemodKernel->AutoTune = RTK_R_ISDBT_AutoTune;
	pDemodKernel->KernelAutoTune = RTK_R_ISDBT_KernelAutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = Demod_Suspend;
	pDemodKernel->Resume = Demod_Resume;
	pDemodKernel->SetTvMode = RTK_R_ISDBT_SetTvMode;
	pDemodKernel->InspectSignal = RTK_R_ISDBT_InspectSignal;	  // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
	pDemodKernel->Lock = RTK_R_ISDBT_Lock;
	pDemodKernel->Unlock = RTK_R_ISDBT_Unlock;

	//pDemodKernel->Activate = RTK_R_ISDBT_Activate;
	pDemodKernel->Destory = RTK_R_ISDBT_Destructors;

kmalloc_fail:
	RTK_R_ISDB_WARNING("RTK_R_ISDBT_Constructors kmalloc Failed\n");
}



/*----------------------------------------------------------------------
 * Func : ~RTK_R_ISDBT
 *
 * Desc : Destructor of RTK_R_ISDBT
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void RTK_R_ISDBT_Destructors(DEMOD* pDemodKernel)
{
	RTK_R_ISDB* pRtkrISDB;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);

	if (pRtkrISDB->m_private != NULL)
		ReleaseRtkrDriver((RTK_R_ISDBT_DRIVER_DATA*)pRtkrISDB->m_private);

	kfree(pRtkrISDB);
	//pDemodKernel->Destory(pDemodKernel);
	RTK_R_ISDB_INFO("RTK_R_ISDBT_Destructors Complete\n");
}



/*----------------------------------------------------------------------
 * Func : Init
 *
 * Desc : Initial rtk_r
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int RTK_R_ISDBT_Init(DEMOD* pDemodKernel)
{
	//int ret = TUNER_CTRL_OK;
	REG_VALUE_ENTRY*  OptRegTable = NULL;
	int               OptRegSize  = 0;
	int i;

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	if (pDemodKernel->m_pComm == NULL || pRtkrISDB->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	//////////////////////////////////////////////////
	//ISDBT_DEMOD_MODULE* pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	//////////////////////////////////////////////////
	//BASE_INTERFACE_MODULE*  pBaseInterface = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pBaseInterface;

	pDemodKernel->Lock(pDemodKernel);
	//DtvDemodClockEnable(1);
	DtvDemodInitialization(RTK_DEMOD_MODE_ISDB, 1);
	if (pDemod->SetRegBits(pDemod, ISDBT_SOFT_RST_N, 0x0) != FUNCTION_SUCCESS)
		goto init_fail;

	// check tuner information

	if (pDemodKernel->m_pTuner) {
		TUNER_PARAM Param;

		if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, TV_SYS_ISDBT_6M, &Param) == TUNER_CTRL_OK) {
			switch (Param.id) {
			case TV_TUNER_NXP_TDA18275A:
				RTK_R_ISDB_INFO("Tuner id %d, TV_TUNER_NXP_TDA18275A\n", Param.id);
				OptRegTable = (REG_VALUE_ENTRY*) TDA18275A_OPT_VALUE_ISDBT;
				OptRegSize  = sizeof(TDA18275A_OPT_VALUE_ISDBT) / sizeof(REG_VALUE_ENTRY);
				break;

			case TV_TUNER_SILICONLABS_SI2151:
			case TV_TUNER_SILICONLABS_SI2178B:
			case TV_TUNER_NO_TUNER:
				RTK_R_ISDB_INFO("Tuner id %d, TV_TUNER_SILICONLABS_SI21XX\n", Param.id);
				OptRegTable = (REG_VALUE_ENTRY*) SI2158_OPT_VALUE_ISDBT;
				OptRegSize  = sizeof(SI2158_OPT_VALUE_ISDBT) / sizeof(REG_VALUE_ENTRY);
				break;
			case TV_TUNER_MAXLINEAR_MXL661:
				RTK_R_ISDB_INFO("Tuner id %d, TV_TUNER_MAXLINEAR_MXL6x1\n", Param.id);
				OptRegTable = (REG_VALUE_ENTRY*) MXL601_OPT_VALUE_ISDBT;
				OptRegSize  = sizeof(MXL601_OPT_VALUE_ISDBT) / sizeof(REG_VALUE_ENTRY);
				break;
			case TV_TUNER_RAFAEL_R840:
				RTK_R_ISDB_INFO("Tuner id %d, TV_TUNER_RAFAEL_R840\n", Param.id);
				OptRegTable = (REG_VALUE_ENTRY*) R840_OPT_VALUE_ISDBT;
				OptRegSize  = sizeof(R840_OPT_VALUE_ISDBT) / sizeof(REG_VALUE_ENTRY);
				break;

                        case TV_TUNER_SONY_SUTRX243:
			case TV_TUNER_SONY_SUTRX243_RA:
				RTK_R_ISDB_INFO("Tuner id %d, TV_TUNER_SONY_SUTRC234\n", Param.id);
				OptRegTable = (REG_VALUE_ENTRY*) SUTRX243_OPT_VALUE_ISDBT;
				OptRegSize  = sizeof(SUTRX243_OPT_VALUE_ISDBT) / sizeof(REG_VALUE_ENTRY);
                                break;
			case TV_TUNER_ALTOBEAM_ATBM253:
				RTK_R_ISDB_INFO("Tuner id %d, TV_TUNER_ALTOBEAM_ATBM253\n", Param.id);
				OptRegTable = (REG_VALUE_ENTRY*) ATBM253_OPT_VALUE_ISDBT;
				OptRegSize = sizeof(ATBM253_OPT_VALUE_ISDBT) / sizeof(REG_VALUE_ENTRY);
				break;
			default:
				RTK_R_ISDB_WARNING("unkonw tuner id %d, using default setting\n", Param.id);
				OptRegTable = (REG_VALUE_ENTRY*) SI2158_OPT_VALUE_ISDBT;
				OptRegSize  = sizeof(SI2158_OPT_VALUE_ISDBT) / sizeof(REG_VALUE_ENTRY);
				break;
			}

			pDemodKernel->m_if.freq      = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola  = Param.if_agc_pola;

		}
	}


	// Initial Driver



	//pBaseInterface->I2cRead                   = __rtk_r_rbus_i2c_read;
	//pBaseInterface->I2cWrite                  = __rtk_r_rbus_i2c_write;

	if (pDemod->Initialize(pDemod) != FUNCTION_SUCCESS)
		goto init_fail;


	// Run Optimize Configuration for specified tuner
	for (i = 0; i < OptRegSize; i++) {
		int Reg = OptRegTable[i].RegBitName;
		U32BITS Val = OptRegTable[i].Value;
		if (pDemod->SetRegBits(pDemod, Reg, Val) != FUNCTION_SUCCESS)
			goto init_fail;
	}

	pDemodKernel->Unlock(pDemodKernel);


	if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK) {
		RTK_R_ISDB_WARNING("RTK_R_ISDBT Init Failed\n");
		return TUNER_CTRL_FAIL;
	}

	if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK) {
		RTK_R_ISDB_WARNING("RTK_R_ISDBT Init Failed\n");
		return TUNER_CTRL_FAIL;
	}


	RTK_R_ISDB_DBG("RTK_R_ISDBT Init Complete\n");
	return TUNER_CTRL_OK;

init_fail:

	RTK_R_ISDB_WARNING("RTK_R_ISDBT Init Failed\n");
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
int RTK_R_ISDBT_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	RTK_R_ISDB* pRtkrISDB;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);

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
int RTK_R_ISDBT_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo)
{
	unsigned char nSegment = ISDBT_SEGMENT_NUM_13;     // ISDB-T channel usually be divided into 13 segments with 22 sub channel
	unsigned char nSubChannel = 22;
	int Bandwidth = ISDBT_BANDWIDTH_6MHZ;
	TUNER_PARAM Param;
	int ret = TUNER_CTRL_OK;

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	pRtkrISDB->m_auto_tune_enable   = 0;


	switch (sys) {
	case TV_SYS_ISDBT_6M:
		Bandwidth = ISDBT_BANDWIDTH_6MHZ;
		break;
	case TV_SYS_ISDBT_7M:
		Bandwidth = ISDBT_BANDWIDTH_7MHZ;
		break;
	case TV_SYS_ISDBT_8M:
		Bandwidth = ISDBT_BANDWIDTH_8MHZ;
		break;
	default:
		RTK_R_ISDB_WARNING("Set TV Sys failed, TV system %s is not ISDB_T standard\n", tv_sys_name(sys));
		return TUNER_CTRL_FAIL;
	}


	//TUNER_PARAM Param;
	if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, sys, &Param) == TUNER_CTRL_OK) {
		if ((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola  != Param.if_agc_pola)) {
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;
			if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)  return  TUNER_CTRL_FAIL;

		}
	}

	/*
	//SetDemod fail
		if (pSigInfo && pSigInfo->mod == TV_MODULATION_ISDBT) {
			nSegment = pSigInfo->isdbt.segment_num;
			nSubChannel = pSigInfo->isdbt.subchannel_num;
		}
	*/
	RTK_R_ISDB_INFO("Set TV Sys to %s, Segment Count=%d, SubChannel Count=%d\n",
					tv_sys_name(sys), nSegment, nSubChannel);

	pDemodKernel->m_tv_sys = sys;
	pRtkrISDB->m_SigInfo.mod = TV_MODULATION_ISDBT;
	pRtkrISDB->m_SigInfo.isdbt.segment_num = nSegment;
	pRtkrISDB->m_SigInfo.isdbt.subchannel_num = nSubChannel;

	pDemodKernel->Lock(pDemodKernel);
	ret |= DECODE_RET(pDemod->SetParameters(pDemod, Bandwidth, nSegment, nSubChannel));
	ret |= DECODE_RET(pDemod->RestartFsm(pDemod));	 // Restart demod FSM.
	pDemodKernel->Unlock(pDemodKernel);

	pRtkrISDB->m_acquire_sig_en = 1;
	pRtkrISDB->m_auto_tune_enable = 1;
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
int RTK_R_ISDBT_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret;

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	RTK_R_ISDB_DBG("SetIF, IF_freq="PT_U32BITS", spectrumn inv=%d, agc_pola=%d\n",
				   pParam->freq, pParam->inversion, pParam->agc_pola);

	ret  = DECODE_RET(pDemod->SetIfFreqHz(pDemod, pParam->freq));
	ret |= DECODE_RET(pDemod->SetSpectrumMode(pDemod, pParam->inversion));

	// Kevin Add for AGC Ploarity
	ret |= DECODE_RET(pDemod->SetRegBits(pDemod, ISDBT_POLAR_IFAGC, pParam->agc_pola));
	ret |= DECODE_RET(pDemod->SetRegBits(pDemod, ISDBT_Z_AGC, 2));  // push pull mode
	ret |= DECODE_RET(pDemod->RestartFsm(pDemod));
	pRtkrISDB->m_low_snr_detect     = 0;
	pRtkrISDB->m_low_snr_recover    = 0;

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
int RTK_R_ISDBT_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	RTK_R_ISDB* pRtkrISDB;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);

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
int RTK_R_ISDBT_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret;
	//U32BITS Val;

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	switch (pParam->mode) {
	case PARALLEL_MODE:
		RTK_R_ISDB_DBG("TS Mode = PARALLEL\n");
		ret  = DECODE_RET(pDemod->SetTsInterfaceMode(pDemod, TS_INTERFACE_PARALLEL));
		break;

	case SERIAL_MODE:
		RTK_R_ISDB_DBG("TS Mode = SERIAL\n");
		ret  = DECODE_RET(pDemod->SetTsInterfaceMode(pDemod, TS_INTERFACE_SERIAL));
		break;

	default:
		ret = TUNER_CTRL_FAIL;
	}

	switch (pParam->clk_pol) {
	case RISING_EDGE:
		RTK_R_ISDB_DBG("MPEG CLK POL = RISING_EDGE\n");
		ret  = DECODE_RET(pDemod->SetTsInterfaceClkPol(pDemod, TS_INTERFACE_CLK_POL_RISING));
		break;

	case FALLING_EDGE:
		RTK_R_ISDB_DBG("MPEG CLK POL = FALLING_EDGE\n");
		ret  = DECODE_RET(pDemod->SetTsInterfaceClkPol(pDemod, TS_INTERFACE_CLK_POL_FALLING));
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
int RTK_R_ISDBT_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{
	int Val;
	int ret = TUNER_CTRL_OK;
	//add workaround for field CH19 in Brasil
	int lock = 0;
	U32BITS	RsberValid[3];
	S32BITS SnrNum = 0, SnrDen = 0, snr = 0;
	unsigned char RsberValidLayer = 0;
	int layer_constel = 0;
	int ret_a = TUNER_CTRL_OK;
	U32BITS PerNum = 1, PerDen = 1;
	//end of workaround
#if USE_PREFRM_LOCK
	unsigned char IsPiSnrOverThd = 0;
#endif
	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);
	if (IsCheckLockBy == BY_RSBER_VALID) { //Rsber
		ret = DECODE_RET(pDemod->IsSignalLocked(pDemod, &Val));
		RTK_R_ISDB_INFO("[GetLock]Check IsSignalLocked: %d\n", Val);
	} else if (IsCheckLockBy == BY_FRM_LOCK) { //Inner
		ret = DECODE_RET(pDemod->IsInnerLocked(pDemod, &Val));
		RTK_R_ISDB_INFO("[GetLock]Check IsInnerLocked: %d\n", Val);
	}
#if USE_PREFRM_LOCK
	else {//Prefrm
		ret = DECODE_RET(pDemod->IsInnerLocked(pDemod, &Val));

		RTK_R_ISDBT_IsPiSnrOverThd(pDemodKernel, &IsPiSnrOverThd);
		RTK_R_ISDB_INFO("[GetLock][PRE]IsPiSnrOverThd = %d, Frmlock = %d\n", IsPiSnrOverThd, Val);
		Val |= IsPiSnrOverThd;
		if ((Val == 0) && (gPrefrmMissingChance != 0)) {
			RTK_R_ISDB_INFO("\033[1;32;31m""[GetLock][PRE]Wait frame lock...\n""\033[m");
			Val = 1;
		}

		if (gPrefrmMissingChance > 0)
			gPrefrmMissingChance--;
	}
#endif

	if (ret == TUNER_CTRL_OK)
		*pLock = DECODE_LOCK(Val);
#if USE_PREFRM_LOCK
	if (IsCheckLockBy != BY_PREFRM_LOCK) { //checked by inner/outer lock
#endif
		//add workaround for field CH19 in Brasil
		if (*pLock == DTV_SIGNAL_LOCK) {
			pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen);
			snr = SnrNum * 10000 / SnrDen;
			if (snr <= SNR_THD_FOR_FRM_LOCK) {
				pDemod->GetRegBits(pDemod, ISDBT_RSBER_0_VALID, &RsberValid[0]);
				pDemod->GetRegBits(pDemod, ISDBT_RSBER_1_VALID, &RsberValid[1]);
				pDemod->GetRegBits(pDemod, ISDBT_RSBER_2_VALID, &RsberValid[2]);
				RTK_R_ISDB_DBG("Check Rsber valid\n");
				lock = (RsberValid[0] || RsberValid[1] || RsberValid[2]) ? 1 : 0;
				//Add workaround for field CH19
				if (RsberValid[2]) {
					RsberValidLayer = 2;
				} else if (RsberValid[1]) {
					RsberValidLayer = 1;
				} else {
					RsberValidLayer = 0;
				}

				if (snr <= SNR_THD_FOR_CH19) {
					ret_a |= DECODE_RET(pDemod->GetLayerConstellation(pDemod, RsberValidLayer, &layer_constel));
					ret_a |= (lock) ? DECODE_RET(pDemod->GetLayerPer(pDemod, RsberValidLayer, &PerNum, &PerDen)) : TUNER_CTRL_OK;

					if ((layer_constel == ISDBT_CONSTELLATION_64QAM) && (PerNum == 1)) {
						*pLock = 0;
						RTK_R_ISDB_DBG("[GetLockStatus] SNR < TH, 64QAM, Per = 1 => UNLOCK\n");
					} else if ((layer_constel == ISDBT_CONSTELLATION_64QAM) && (PerNum == 0x4000)) {
						*pLock = 0;
						RTK_R_ISDB_DBG("[GetLockStatus] SNR < TH, 64QAM, Per = 0x4000 => UNLOCK\n");
					} else if ((layer_constel == ISDBT_CONSTELLATION_64QAM) && (PerNum == 65536)) {
						*pLock = 0;
						RTK_R_ISDB_DBG("[GetLockStatus] SNR < TH, 64QAM, Per = 65536 => UNLOCK\n");
					} else {
						RTK_R_ISDB_DBG("[GetLockStatus] other case\n");
					}
				}
			}
		}
		//end of workaround for field CH19 in Brasil
#if USE_PREFRM_LOCK
	}
#endif
	pDemodKernel->Unlock(pDemodKernel);

	return ret;
}

int RTK_R_ISDBT_GetSegmentNum(DEMOD* pDemodKernel, int layerIndex, unsigned char* pNum)
{
	unsigned char Num;
	int ret;

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	ret = DECODE_RET(pDemod->GetLayerSegmentNum(pDemod, layerIndex, &Num));

	if (ret == TUNER_CTRL_OK)
		*pNum = DECODE_LOCK(Num);

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
int RTK_R_ISDBT_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	int ret = TUNER_CTRL_OK;
	int layer_constel = 0, layer_code_rate = 0, GuardInterval = 0, Fft = 0;
	int ret_a = TUNER_CTRL_OK, ret_b = TUNER_CTRL_OK, ret_c = TUNER_CTRL_OK, final_ret = TUNER_CTRL_OK;
	unsigned char SegmentNum = 0;

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	ret |= DECODE_RET(pDemod->GetFftMode(pDemod, &Fft));
	switch ((unsigned char)Fft) {
	case ISDBT_FFT_MODE_2K :
		pInfo->isdbt.trans_mode = OFDM_FFT_2K;
		break;
	case ISDBT_FFT_MODE_4K :
		pInfo->isdbt.trans_mode = OFDM_FFT_4K;
		break;
	case ISDBT_FFT_MODE_8K :
		pInfo->isdbt.trans_mode = OFDM_FFT_8K;
		break;
	}
	RTK_R_ISDB_DBG("ISDBT_FFT_Mode = %d\n", pInfo->isdbt.trans_mode);

	ret |= DECODE_RET(pDemod->GetGuardInterval(pDemod, &GuardInterval));
	//RTK_R_ISDB_INFO("ret_GetGuardInterval = %d\n", ret_a);
	switch ((unsigned char)GuardInterval) {
	case ISDBT_GUARD_INTERVAL_1_OVER_32 :
		pInfo->isdbt.guard_interval = OFDM_GI_1_32;
		break;
	case ISDBT_GUARD_INTERVAL_1_OVER_16 :
		pInfo->isdbt.guard_interval = OFDM_GI_1_16;
		break;
	case ISDBT_GUARD_INTERVAL_1_OVER_8 :
		pInfo->isdbt.guard_interval = OFDM_GI_1_8;
		break;
	case ISDBT_GUARD_INTERVAL_1_OVER_4 :
		pInfo->isdbt.guard_interval = OFDM_GI_1_4;
		break;
	}
	RTK_R_ISDB_DBG("guard_interval = %d\n", pInfo->isdbt.guard_interval);

	//Get SegmentNum and set MaxSeglayer
	MaxSeglayer = ISDBT_HIERARCHICAL_LAYER_A;           //default max segment_num = layer_a
	ret_a |= DECODE_RET(pDemod->GetLayerSegmentNum(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &SegmentNum));
	pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].segment_num = SegmentNum;

	ret_b |= DECODE_RET(pDemod->GetLayerSegmentNum(pDemod, ISDBT_HIERARCHICAL_LAYER_B, &SegmentNum));
	pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].segment_num = SegmentNum;

	if (SegmentNum > pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].segment_num)
		MaxSeglayer = ISDBT_HIERARCHICAL_LAYER_B;       //compare segment_num and set MaxSeglayer

	ret_c |= DECODE_RET(pDemod->GetLayerSegmentNum(pDemod, ISDBT_HIERARCHICAL_LAYER_C, &SegmentNum));
	pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].segment_num = SegmentNum;

	if (SegmentNum > pInfo->isdbt.layer[MaxSeglayer].segment_num)
		MaxSeglayer = ISDBT_HIERARCHICAL_LAYER_C;       //compare segment_num and set MaxSeglayer

	RTK_R_ISDB_DBG("ISDBT Signal Layer = %d, SegmentNum=%d\n", MaxSeglayer, pInfo->isdbt.layer[MaxSeglayer].segment_num);


	//Layer A
	ret_a |= DECODE_RET(pDemod->GetLayerConstellation(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &layer_constel));
	ret_a |= DECODE_RET(pDemod->GetLayerCodeRate(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &layer_code_rate));
	//RTK_R_ISDB_INFO("ret_layer_A_QAM = %d\n", ret_a);
	//RTK_R_ISDB_INFO("ret_layer_A_CR = %d\n", ret_a);

	switch ((unsigned short)layer_constel) {
	case ISDBT_CONSTELLATION_DQPSK :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].constellation = OFDM_CONST_DQPSK;
		break;
	case ISDBT_CONSTELLATION_QPSK :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].constellation = OFDM_CONST_4;
		break;
	case ISDBT_CONSTELLATION_16QAM :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].constellation = OFDM_CONST_16;
		break;
	case ISDBT_CONSTELLATION_64QAM :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].constellation = OFDM_CONST_64;
		break;
	}

	switch ((unsigned char)layer_code_rate) {
	case ISDBT_CODE_RATE_1_OVER_2 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].code_rate = OFDM_CODE_RATE_1_2;
		break;
	case ISDBT_CODE_RATE_2_OVER_3 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].code_rate = OFDM_CODE_RATE_2_3;
		break;
	case ISDBT_CODE_RATE_3_OVER_4 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].code_rate = OFDM_CODE_RATE_3_4;
		break;
	case ISDBT_CODE_RATE_5_OVER_6 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].code_rate = OFDM_CODE_RATE_5_6;
		break;
	case ISDBT_CODE_RATE_7_OVER_8 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].code_rate = OFDM_CODE_RATE_7_8;
		break;
	}

	RTK_R_ISDB_DBG("[Layer A]Constellation %d, Code Rate %d\n",
				   pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].constellation,
				   pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_A].code_rate);
	//Layer B
	ret_b |= DECODE_RET(pDemod->GetLayerConstellation(pDemod, ISDBT_HIERARCHICAL_LAYER_B, &layer_constel));
	ret_b |= DECODE_RET(pDemod->GetLayerCodeRate(pDemod, ISDBT_HIERARCHICAL_LAYER_B, &layer_code_rate));
	//RTK_R_ISDB_INFO("ret_layer_B_QAM = %d\n", ret_b);
	//RTK_R_ISDB_INFO("ret_layer_B_CR = %d\n", ret_b);
	switch ((unsigned short)layer_constel) {
	case ISDBT_CONSTELLATION_DQPSK :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].constellation = OFDM_CONST_DQPSK;
		break;
	case ISDBT_CONSTELLATION_QPSK :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].constellation = OFDM_CONST_4;
		break;
	case ISDBT_CONSTELLATION_16QAM :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].constellation = OFDM_CONST_16;
		break;
	case ISDBT_CONSTELLATION_64QAM :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].constellation = OFDM_CONST_64;
		break;
	}

	switch ((unsigned char)layer_code_rate) {
	case ISDBT_CODE_RATE_1_OVER_2 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].code_rate = OFDM_CODE_RATE_1_2;
		break;
	case ISDBT_CODE_RATE_2_OVER_3 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].code_rate = OFDM_CODE_RATE_2_3;
		break;
	case ISDBT_CODE_RATE_3_OVER_4 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].code_rate = OFDM_CODE_RATE_3_4;
		break;
	case ISDBT_CODE_RATE_5_OVER_6 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].code_rate = OFDM_CODE_RATE_5_6;
		break;
	case ISDBT_CODE_RATE_7_OVER_8 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].code_rate = OFDM_CODE_RATE_7_8;
		break;
	}

	RTK_R_ISDB_DBG("[Layer B]Constellation %d, Code Rate %d\n",
				   pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].constellation,
				   pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_B].code_rate);

	//Layer C
	ret_c |= DECODE_RET(pDemod->GetLayerConstellation(pDemod, ISDBT_HIERARCHICAL_LAYER_C, &layer_constel));
	ret_c |= DECODE_RET(pDemod->GetLayerCodeRate(pDemod, ISDBT_HIERARCHICAL_LAYER_C, &layer_code_rate));
	//RTK_R_ISDB_INFO("ret_layer_C_QAM = %d\n", ret_c);
	//RTK_R_ISDB_INFO("ret_layer_C_CR = %d\n", ret_c);
	switch ((unsigned short)layer_constel) {
	case ISDBT_CONSTELLATION_DQPSK :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].constellation = OFDM_CONST_DQPSK;
		break;
	case ISDBT_CONSTELLATION_QPSK :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].constellation = OFDM_CONST_4;
		break;
	case ISDBT_CONSTELLATION_16QAM :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].constellation = OFDM_CONST_16;
		break;
	case ISDBT_CONSTELLATION_64QAM :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].constellation = OFDM_CONST_64;
		break;
	}

	switch ((unsigned char)layer_code_rate) {
	case ISDBT_CODE_RATE_1_OVER_2 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].code_rate = OFDM_CODE_RATE_1_2;
		break;
	case ISDBT_CODE_RATE_2_OVER_3 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].code_rate = OFDM_CODE_RATE_2_3;
		break;
	case ISDBT_CODE_RATE_3_OVER_4 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].code_rate = OFDM_CODE_RATE_3_4;
		break;
	case ISDBT_CODE_RATE_5_OVER_6 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].code_rate = OFDM_CODE_RATE_5_6;
		break;
	case ISDBT_CODE_RATE_7_OVER_8 :
		pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].code_rate = OFDM_CODE_RATE_7_8;
		break;
	}
	RTK_R_ISDB_DBG("[Layer C]Constellation %d, Code Rate %d\n",
				   pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].constellation,
				   pInfo->isdbt.layer[ISDBT_HIERARCHICAL_LAYER_C].code_rate);

	if ((ret && ret_a && ret_b && ret_c) == 0) {
		final_ret = TUNER_CTRL_OK;
	} else {
		final_ret = TUNER_CTRL_FAIL;
	}
	RTK_R_ISDB_DBG("ret = %d\n", final_ret);

	pDemodKernel->Unlock(pDemodKernel);

	return final_ret;
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
int RTK_R_ISDBT_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality)
{
	int ret = TUNER_CTRL_OK;
	S32BITS SnrNum, SnrDen;

#ifndef GET_SIGNAL_STRENGTH_FROM_SNR
	U32BITS SignalStrength;
#endif

	unsigned char FrameLock = 0;
	int LockStatus = 0;
	S32BITS IfAgc = -1;
	U32BITS BerNum = 1, BerDen = 1;
	U32BITS PerNum = 1, PerDen = 1;
	U32BITS EmergencyFlag = 0;

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;


	pDemodKernel->GetLockStatus(pDemodKernel, &FrameLock);


	pDemodKernel->Lock(pDemodKernel);

	switch (id) {

	case TV_QUAL_NOSIG :
		ret = pDemod->IsNoSig(pDemod, &LockStatus);
		pQuality->nosig = (LockStatus) ? 1 : 0;
		break;

	case TV_QUAL_INNERLOCK :
		ret = pDemod->IsInnerLocked(pDemod, &LockStatus);
		pQuality->inner = (LockStatus) ? 1 : 0;
		break;

	case TV_QUAL_SNR :

		ret = DECODE_RET(pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen));

		if (ret == TUNER_CTRL_OK)
			//pQuality->snr = (float) SnrNum / (float)SnrDen;
			pQuality->snr = SnrNum * 10000 / SnrDen;

		//RTK_R_ISDB_INFO("[Quality]snr(%.4f, %.4f) = %.4f\n", SnrNum, SnrDen, pQuality->snr);
		break;

	case TV_QUAL_BEC :
	case TV_QUAL_BER : {

		ret = (FrameLock) ? DECODE_RET(pDemod->GetLayerBer(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &BerNum, &BerDen)) : TUNER_CTRL_OK;

		if (id == TV_QUAL_BER)
			//pQuality->ber = (float) BerNum / (float)BerDen;
			pQuality->ber = BerNum * 10000 / BerDen;
		else {
			pQuality->layer_ber.ber_num = BerNum;
			pQuality->layer_ber.ber_den = BerDen;
		}

		//RTK_R_ISDB_INFO("[Quality]ber(%.4f, %.4f) = %.4f\n", BerNum, BerDen, pQuality->ber);
		break;
	}

	case TV_QUAL_PEC :
	case TV_QUAL_PER : {

		ret = (FrameLock) ? DECODE_RET(pDemod->GetLayerPer(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &PerNum, &PerDen)) : TUNER_CTRL_OK;
		if (id == TV_QUAL_PER)
			//pQuality->per = (float) PerNum / (float)PerDen;
			pQuality->per = PerNum * 10000 / PerDen;
		else {
			pQuality->layer_per.per_num = PerNum;
			pQuality->layer_per.per_den = PerDen;
		}

		//RTK_R_ISDB_INFO("[Quality]per(%.4f, %.4f) = %.4f\n", PerNum, PerDen, pQuality->per);
		break;
	}
	case TV_QUAL_UCBLOCKS :

		pQuality->ucblocks = 0;
		ret = TUNER_CTRL_OK;
		break;

	case TV_QUAL_SIGNAL_STRENGTH:


#ifdef GET_SIGNAL_STRENGTH_FROM_SNR
		ret = pDemodKernel->GetSignalQuality(pDemodKernel, TV_QUAL_SNR, pQuality);

		if (ret == TUNER_CTRL_OK)
			pQuality->strength = Snr2Str(pQuality->snr / 10000);  // convert snr to signal strength
#else
		ret = DECODE_RET(pDemod->GetSignalStrength(pDemod, &SignalStrength));

		if (ret == TUNER_CTRL_OK)
			pQuality->strength = (unsigned char) SignalStrength;
#endif

		//RTK_R_ISDB_INFO("[Quality]strength : %d\n", pQuality->strength);
		break;

	case TV_QUAL_SIGNAL_QUALITY:

		ret = pDemodKernel->GetSignalQuality(pDemodKernel, TV_QUAL_BER, pQuality);

		if (ret == TUNER_CTRL_OK)
			pQuality->quality = Ber2Quality(pQuality->ber);  // convert ber to signal quality

		//RTK_R_ISDB_INFO("[Quality]quality : %d\n", pQuality->quality);
		break;

	case TV_QUAL_AGC: {
		ret = pDemod->GetIfAgc(pDemod, &IfAgc);

		if (ret == TUNER_CTRL_OK)
			pQuality->agc = ((IfAgc + 8191) * 100) / 16384;
	}
	break;

	case TV_QUAL_EMERGENCY_FLAG:
		ret = DECODE_RET(pDemod->GetRegBits(pDemod, ISDBT_EMERGENCY_FLAG, &EmergencyFlag));
		if (ret == TUNER_CTRL_OK)
			pQuality->emergency_flag = (unsigned char)EmergencyFlag;
		break;

	case TV_QUAL_CUSTOMER_LG:
		ret = DECODE_RET(pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen));
		ret |= DECODE_RET(pDemod->GetIfAgc(pDemod, &IfAgc));  //return Max SegmentNum Layer BER / PER
		ret |= (FrameLock) ? DECODE_RET(pDemod->GetLayerBer(pDemod, MaxSeglayer, &BerNum, &BerDen)) : TUNER_CTRL_OK;
		ret |= (FrameLock) ? DECODE_RET(pDemod->GetLayerPer(pDemod, MaxSeglayer, &PerNum, &PerDen)) : TUNER_CTRL_OK;
		RTK_R_ISDB_DBG("ISDBT_layer = %d, BerNum = "PT_U32BITS", PerNum_ = "PT_U32BITS"\n", MaxSeglayer , BerNum, PerNum);

#ifndef GET_SIGNAL_STRENGTH_FROM_SNR
		ret |= DECODE_RET(pDemod->GetSignalStrength(pDemod, &SignalStrength));
#endif
		pQuality->agc = ((IfAgc + 8191) * 100) / 16384;
		pQuality->layer_ber.ber_num = BerNum;
		pQuality->layer_ber.ber_den = BerDen;
		pQuality->ber = BerNum * 10000 / BerDen;
		pQuality->layer_per.per_num = PerNum;
		pQuality->layer_per.per_den = PerDen;
		pQuality->quality = Ber2Quality(pQuality->ber);
#ifdef GET_SIGNAL_STRENGTH_FROM_SNR
		if (ret == TUNER_CTRL_OK)
			pQuality->strength = Snr2Str(pQuality->snr);  // convert snr to signal strength
#else
		pQuality->strength = (unsigned char) SignalStrength;
#endif
		pQuality->snr = SnrNum * 10000 / SnrDen;
		break;

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
int RTK_R_ISDBT_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	int ret;
	unsigned char LockStatus = 0;

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	pDemodKernel->GetLockStatus(pDemodKernel, &LockStatus);

	if (LockStatus == 0) {
		RTK_R_ISDB_INFO("GetCarrierOffset fail beacause demod is unlock\n");
		return TUNER_CTRL_FAIL;
	}


	pDemodKernel->Lock(pDemodKernel);

	ret = DECODE_RET(pDemod->GetCrOffsetHz(pDemod, pOffset));

	/*----------------------------------------------------
	  ISDB-T Demod CrOffset = Target Frequency - Current Frequency
	  Note: RTK_R ISDB-T Demod will handle IF polarity automatically
	-----------------------------------------------------*/
	if (ret != TUNER_CTRL_OK)
		*pOffset = 0;

	pDemodKernel->Unlock(pDemodKernel);

	RTK_R_ISDB_INFO("GetCarrierOffset="PT_S32BITS"Hz\n", *pOffset);
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
int RTK_R_ISDBT_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned long cur_time;
#endif

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	if (!WaitSignalLock)
		return TUNER_CTRL_OK;

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	cur_time = tv_osal_time();

	while ((tv_osal_time() - cur_time) <= 15) {

		if (pRtkrISDB->m_acquire_sig_en == 0)
			return TUNER_CTRL_OK;

		tv_osal_msleep(5);
	}

	return TUNER_CTRL_FAIL;

#else
	RTK_R_ISDBT_AcquireSignalThread(pDemodKernel);
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
int RTK_R_ISDBT_AcquireSignalThread(DEMOD* pDemodKernel)
{
	int lock = 0, no_signal = 0;
	U32BITS	RsberValid[3]/*, Rsber = RTK_R_ISDBT_PER_DEN_VALUE*//*, Rsber_tmp = RTK_R_ISDBT_PER_DEN_VALUE*//*, number_seg[3]*/;
	S32BITS SnrNum = 0, SnrDen = 0, snr = 0;
	int FrmLock = 0;
	//add workaround for field CH19 in Brasil
	unsigned char RsberValidLayer = 0;
	int layer_constel = 0;
	int ret_a = TUNER_CTRL_OK;
	U32BITS PerNum = 1, PerDen = 1;
	//end of workaround
#if USE_PREFRM_LOCK
	int ret = TUNER_CTRL_OK;
	unsigned char u8i = 0, IsPiSnrOverThd = 0, u8PiSnrOverThdCnt = 0, u8PiSnrMaxPollingNum = 0;
	int Fft = 0, GuardInterval = 0, PrefrmLock = 0;
#endif
	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;
	IsCheckLockBy = BY_RSBER_VALID;

	if (pRtkrISDB->m_acquire_sig_en == 0)
		return TUNER_CTRL_OK;

	pRtkrISDB->m_auto_tune_enable = 0;

	RsberValid[0] = 0;
	RsberValid[1] = 0;
	RsberValid[2] = 0;

	//pDemodKernel->Lock(pDemodKernel);
	//pDemod->RestartFsm(pDemod);	 // Restart demod FSM.
	pRtkrISDB->m_low_snr_detect	  = 0;
	pRtkrISDB->m_low_snr_recover	  = 0;
	//pDemodKernel->Unlock(pDemodKernel);

	pDemodKernel->Lock(pDemodKernel);

	while ((tv_osal_time() - acquire_sig_stime) <= 200) {
		pDemod->IsNoSig(pDemod, &no_signal);
		tv_osal_msleep_interruptible(5);
		//RTK_R_ISDB_INFO("no_signal = %d, time=%lu\n", no_signal, tv_osal_time() - stime);
		if (no_signal == 0) break;
	}

	pDemodKernel->Unlock(pDemodKernel);

	RTK_R_ISDB_INFO("no_signal = %d, time=%lu\n", no_signal, tv_osal_time() - acquire_sig_stime);

	if (no_signal == 1) {
		pRtkrISDB->m_acquire_sig_en = 0;
		return TUNER_CTRL_OK;
	}
#if USE_PREFRM_LOCK
	ret |= DECODE_RET(pDemod->GetFftMode(pDemod, &Fft));
	ret |= DECODE_RET(pDemod->GetGuardInterval(pDemod, &GuardInterval));
	RTK_R_ISDB_DBG("[PRE]Fft = %d, GuardInterval =%d\n", Fft, GuardInterval);
	for (u8i = 0; u8i < ISDBT_PREFRM_POLLING_NUM_TABLE_SIZE; u8i++) {
		if (_gsIsdbtPrefrmPollingNumMappingTable[u8i].trans_mode == Fft &&
			_gsIsdbtPrefrmPollingNumMappingTable[u8i].guard_interval == GuardInterval) {
			u8PiSnrMaxPollingNum = _gsIsdbtPrefrmPollingNumMappingTable[u8i].PollingNum;
			RTK_R_ISDB_DBG("[PRE]u8PiSnrMaxPollingNum = %d\n", u8PiSnrMaxPollingNum);
			break;
		}
	}
#endif
	while ((tv_osal_time() - acquire_sig_stime) <= 1400) {
		tv_osal_msleep_interruptible(5);

		pDemod->IsInnerLocked(pDemod, &FrmLock);
#if USE_PREFRM_LOCK
		RTK_R_ISDBT_IsPiSnrOverThd(pDemodKernel, &IsPiSnrOverThd);
		RTK_R_ISDB_DBG("[PRE_API]IsPiSnrOverThd = %d\n", IsPiSnrOverThd);
		if (IsPiSnrOverThd) {
			u8PiSnrOverThdCnt++;
			if (u8PiSnrOverThdCnt >= u8PiSnrMaxPollingNum)
				PrefrmLock = 1;
		} else {
			u8PiSnrOverThdCnt = 0;
		}

		if (PrefrmLock) {
			lock = PrefrmLock;
		} else {
#endif
			if (FrmLock) {
				pDemod->GetSnrDb(pDemod, &SnrNum, &SnrDen);
				snr = SnrNum * 10000 / SnrDen;
				//RTK_R_ISDB_INFO("+++snr =%ld in Acquire\n", snr);
				if (snr <= SNR_THD_FOR_FRM_LOCK) {
					pDemod->GetRegBits(pDemod, ISDBT_RSBER_0_VALID, &RsberValid[0]);
					pDemod->GetRegBits(pDemod, ISDBT_RSBER_1_VALID, &RsberValid[1]);
					pDemod->GetRegBits(pDemod, ISDBT_RSBER_2_VALID, &RsberValid[2]);
					RTK_R_ISDB_INFO("Check Rsber valid\n");
					lock = (RsberValid[0] || RsberValid[1] || RsberValid[2]) ? 1 : 0;

					//Add workaround for field CH19
					if (RsberValid[2]) {
						RsberValidLayer = 2;
					} else if (RsberValid[1]) {
						RsberValidLayer = 1;
					} else {
						RsberValidLayer = 0;
					}

					if (snr <= SNR_THD_FOR_CH19) {
						ret_a = TUNER_CTRL_OK;
						ret_a |= DECODE_RET(pDemod->GetLayerConstellation(pDemod, RsberValidLayer, &layer_constel));
						ret_a |= (lock) ? DECODE_RET(pDemod->GetLayerPer(pDemod, RsberValidLayer, &PerNum, &PerDen)) : TUNER_CTRL_OK;

						if ((layer_constel == ISDBT_CONSTELLATION_64QAM) && (PerNum == 1)) {
							lock = 0;
							RTK_R_ISDB_DBG("[ACQ] SNR < TH, 64QAM, Per = 1 => UNLOCK\n");
						} else if ((layer_constel == ISDBT_CONSTELLATION_64QAM) && (PerNum == 0x4000)) {
							lock = 0;
							RTK_R_ISDB_DBG("[ACQ] SNR < TH, 64QAM, Per = 0x4000 => UNLOCK\n");
						} else if ((layer_constel == ISDBT_CONSTELLATION_64QAM) && (PerNum == 65536)) {
							lock = 0;
							RTK_R_ISDB_DBG("[ACQ] SNR < TH, 64QAM, Per = 65536 => UNLOCK\n");
						} else {
							RTK_R_ISDB_DBG("[ACQ]other case\n");
						}
					}
					//end of workaround
				} else {
					lock = FrmLock;

					if (FrmLock)
						RTK_R_ISDB_INFO("Check Frm_Lock\n");
				}
			} else {
				if ((tv_osal_time() - acquire_sig_stime) > 800)
					break;
			}
#if USE_PREFRM_LOCK
		}
#endif

		if (lock)
			break;

	}
#if USE_PREFRM_LOCK
	if (PrefrmLock) {
		RTK_R_ISDB_INFO("\033[1;32;32m""PrefrmLock = %d, time=%lu, u8PiSnrMaxPollingNum = %d\n""\033[m", PrefrmLock, tv_osal_time() - acquire_sig_stime, u8PiSnrMaxPollingNum);
		IsCheckLockBy = BY_PREFRM_LOCK;
		gPrefrmMissingChance  = PREFRM_MISSING_MAX_CHANCE;
	} else {
#endif
		if (snr <= SNR_THD_FOR_FRM_LOCK) {
			RTK_R_ISDB_INFO("\033[1;32;32m""signal lock = "PT_U32BITS"/"PT_U32BITS"/"PT_U32BITS", time=%lu\n""\033[m",
							RsberValid[0], RsberValid[1], RsberValid[2],
							tv_osal_time() - acquire_sig_stime);
			IsCheckLockBy = BY_RSBER_VALID;
		} else {
			RTK_R_ISDB_INFO("\033[1;32;32m""frm_lock = %d, time=%lu\n""\033[m", FrmLock, tv_osal_time() - acquire_sig_stime);
#if USE_PREFRM_LOCK
			RTK_R_ISDB_INFO("u8PiSnrOverThdCnt = %d\n", u8PiSnrOverThdCnt);
#endif
			IsCheckLockBy = BY_FRM_LOCK;
		}
#if USE_PREFRM_LOCK
	}
#endif

	/*
		if (lock) {
			// wait signal stable
			pDemod->GetRegBits(pDemod, ISDBT_RX_NO_SEG0, &number_seg[0]);
			pDemod->GetRegBits(pDemod, ISDBT_RX_NO_SEG1, &number_seg[1]);
			pDemod->GetRegBits(pDemod, ISDBT_RX_NO_SEG2, &number_seg[2]);

			pDemod->GetRegBits(pDemod, ISDBT_RSBER_0, &Rsber);

			while ((Rsber == RTK_R_ISDBT_PER_DEN_VALUE) && ((tv_osal_time() - acquire_sig_stime) <= 3000)) {
				tv_osal_msleep_interruptible(10);
				if (number_seg[0] != 0) {
					pDemod->GetRegBits(pDemod, ISDBT_RSBER_0, &Rsber_tmp);
					Rsber = (Rsber_tmp > Rsber) ? Rsber_tmp : Rsber;
				}
				if (number_seg[1] != 0) {
					pDemod->GetRegBits(pDemod, ISDBT_RSBER_1, &Rsber_tmp);
					Rsber = (Rsber_tmp > Rsber) ? Rsber_tmp : Rsber;
				}
				if (number_seg[2] != 0) {
					pDemod->GetRegBits(pDemod, ISDBT_RSBER_2, &Rsber_tmp);
					Rsber = (Rsber_tmp > Rsber) ? Rsber_tmp : Rsber;
				}
				//RTK_R_ISDB_INFO("per="PT_U32BITS" \n", Rsber);
				//pDemod->GetRegBits(pDemod, ISDBT_RSBER_0, &Rsber);
			}

			RTK_R_ISDB_INFO("per="PT_U32BITS", time=%lu\n\n", Rsber, tv_osal_time() - acquire_sig_stime);
		}
	*/
	pRtkrISDB->m_auto_tune_enable = 1;
	pRtkrISDB->m_acquire_sig_en = 0;
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
int RTK_R_ISDBT_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range)
{

	/*
		U32BITS   RsberValid[3];
		U32BITS   stime;
		unsigned char   low_signal_cnt;
		TV_SIG_QUAL     qual;

		RTK_R_ISDB* pRtkrISDB;
		ISDBT_DEMOD_MODULE* pDemod;
		pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
		pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

		pRtkrISDB->Activate(pDemodKernel, 0);

		pDemodKernel->Lock(pDemodKernel);

		// Restart demod FSM.
		pDemod->RestartFsm(pDemod);
		pRtkrISDB->m_low_snr_detect     = 0;
		pRtkrISDB->m_low_snr_recover    = 0;
		pDemodKernel->Unlock(pDemodKernel);

		stime = tv_osal_time();
		low_signal_cnt = 0;

		while ((tv_osal_time() - stime) <= 1400 && low_signal_cnt < 7) {
			tv_osal_msleep(120);

			pDemodKernel->Lock(pDemodKernel);

			pDemod->GetRegBits(pDemod, ISDBT_RSBER_0_VALID, &RsberValid[0]);
			pDemod->GetRegBits(pDemod, ISDBT_RSBER_1_VALID, &RsberValid[1]);
			pDemod->GetRegBits(pDemod, ISDBT_RSBER_2_VALID, &RsberValid[2]);

			if (RsberValid[0] || RsberValid[1] || RsberValid[2]) {
				pDemodKernel->GetCarrierOffset(pDemodKernel, pOffset);	    // Get Offset Value
				pDemodKernel->Unlock(pDemodKernel);
				RTK_R_ISDB_INFO("find Signal with offset = "PT_S32BITS" Hz\n", *pOffset);
				return TUNER_CTRL_OK;
			}
			pDemodKernel->Unlock(pDemodKernel);

			pDemodKernel->GetSignalQuality(pDemodKernel, TV_QUAL_SNR, &qual);

			RTK_R_ISDB_INFO("signal lock = 0, snr = "PT_U32BITS", low sig=%d, time=%lu\n", qual.snr, low_signal_cnt, tv_osal_time() - stime);

			if (qual.snr < 0)
				low_signal_cnt++;
		}

		RTK_R_ISDB_WARNING("can not find signal within specified %d/6MHz range Hz\n", range);

	*/
	return TUNER_CTRL_NO_SIGNAL;
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
int RTK_R_ISDBT_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off)
{
	int ret = TUNER_CTRL_OK;

	/*
		RTK_R_ISDB* pRtkrISDB;
		ISDBT_DEMOD_MODULE* pDemod;
		pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
		pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

		pDemodKernel->Lock(pDemodKernel);

		if (pDemod->SetRegBits(pDemod, ISDBT_I2CT_EN_CTRL, (on_off) ? 1 : 0) != FUNCTION_SUCCESS)
			ret = TUNER_CTRL_FAIL;

		pDemodKernel->Unlock(pDemodKernel);
	*/

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
int RTK_R_ISDBT_AutoTune(DEMOD* pDemodKernel)
{
#if 0
	ISDBT_DEMOD_MODULE* pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;
	TV_SIG_QUAL qual;
	unsigned char lock;
	U32BITS RsberValid[3];
	unsigned char low_snr_timeout = (pRtkrISDB->m_low_snr_recover <= 3) ? 15 : 50;

	if (DtvIsDemodClockEnable() == 0 || pRtkrISDB->m_auto_tune_enable == 0)
		return TUNER_CTRL_OK;

	pDemodKernel->Lock(pDemodKernel);
	pDemod->GetRegBits(pDemod, ISDBT_RSBER_0_VALID, &RsberValid[0]);
	pDemod->GetRegBits(pDemod, ISDBT_RSBER_1_VALID, &RsberValid[1]);
	pDemod->GetRegBits(pDemod, ISDBT_RSBER_2_VALID, &RsberValid[2]);
	pDemodKernel->GetSignalQuality(pDemodKernel, TV_QUAL_PER, &qual);

	lock = (RsberValid[0] | RsberValid[1] | RsberValid[2]) ? 1 : 0;

	if (lock && qual.per > 0.003) {
		pRtkrISDB->m_low_snr_detect++;

		RTK_R_ISDB_WARNING("bad per (%f) detected (%d/%d)\n", qual.per, pRtkrISDB->m_low_snr_detect, low_snr_timeout);

		if (pRtkrISDB->m_low_snr_detect >= low_snr_timeout) {
			RTK_R_ISDB_WARNING("poor signal timeout, reset demod to recover...\n");
			Activate(1);
			pRtkrISDB->m_low_snr_detect = 0;
			pRtkrISDB->m_low_snr_recover++;
		}
	} else {
		if (lock && qual.per >= 0.002)
			RTK_R_ISDB_INFO("AutoTune, lock=%d, per=%f (%d/%d)\n", lock, (lock == 0) ? -1.0 : qual.per, pRtkrISDB->m_low_snr_detect, low_snr_timeout);

		if (pRtkrISDB->m_low_snr_detect > 0)
			pRtkrISDB->m_low_snr_detect--;
	}

	pDemodKernel->Unlock(pDemodKernel);
#endif
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
int RTK_R_ISDBT_KernelAutoTune(DEMOD* pDemodKernel)
{
	/*
	    unsigned long cur_time ;
		unsigned char Lock;
	*/
	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	//cur_time = tv_osal_time();
	if (pRtkrISDB->m_acquire_sig_en) {

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
		RTK_R_ISDBT_AcquireSignalThread(pDemodKernel);
#endif

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
int RTK_R_ISDBT_InspectSignal(DEMOD* pDemodKernel)
{

	int ret = TUNER_CTRL_OK;

#if 0
	int ret;
	S32BITS IfAgc = -1;
	S32BITS RfAgc = -1;
	S32BITS TrOffsetPpm = -1;
	S32BITS CrOffsetHz  = -1;
	S32BITS DiAgc = 0;
	int SignalLock = 0;
	U32BITS BerNum[3];
	U32BITS BerDen[3];
	U32BITS PerNum[3];
	U32BITS PerDen[3];
	U32BITS RsberValid[3];
	S32BITS SnrDbNum = 1;
	S32BITS SnrDbDen = 1;
	unsigned char demod_err = 0;
	U32BITS demod_status;
	U32BITS ddr_offset = 0;
	static unsigned char display_msg_timeout = 0;

	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	if (DtvIsDemodClockEnable(RTK_DEMOD_MODE_ISDB) == 0)
		return 0;

	DtvDumpShareRegStatus();

	pDemodKernel->Lock(pDemodKernel);

	/*--------------------------------------------------
	reg 0xC858:
	D[0] : ddr_test_mode_error
	D[1] : inner_data_rw_error
	D[2] : wrap_read_num_error
	D[3] : wrap_write_num_error

	reg 0xC85C:
	D[31:0] : demod status
	*--------------------------------------------------*/
	pDemod->GetRegBytes(pDemod, 0xee58, &demod_err, 1);      // demod-ddr wrapper error indicator
	pDemod->GetRegBytes(pDemod, 0xee5C, (unsigned char*) &demod_status, sizeof(demod_status));      // wrap_demod_status_ro
	pDemod->GetRegBits(pDemod, ISDBT_DDR_ADDR_OFFSET, &ddr_offset);

	if (demod_err & 0xF) {
		RTK_R_ISDB_WARNING("DDR wrapper error detected, err=%x status=%08lx\n",
						   demod_err & 0xF, demod_status);
	} else if (display_msg_timeout++ > 6) {
		RTK_R_ISDB_INFO("DDR wrapper status, err=%x status=%08lx, ddr_ofst=%08lx\n",
						demod_err & 0xF, demod_status, ddr_offset);
		display_msg_timeout = 0;
	}


	pDemod->GetRfAgc(pDemod, &RfAgc);
	pDemod->GetIfAgc(pDemod, &IfAgc);
	pDemod->GetDiAgc(pDemod, &DiAgc);

	pDemod->GetTrOffsetPpm(pDemod, &TrOffsetPpm);
	pDemod->GetCrOffsetHz(pDemod, &CrOffsetHz);
	pDemod->IsSignalLocked(pDemod, &SignalLock);

	pDemod->GetRegBits(pDemod, ISDBT_RSBER_0_VALID, &RsberValid[0]);
	pDemod->GetRegBits(pDemod, ISDBT_RSBER_1_VALID, &RsberValid[1]);
	pDemod->GetRegBits(pDemod, ISDBT_RSBER_2_VALID, &RsberValid[2]);

	pDemod->GetLayerBer(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &BerNum[0], &BerDen[0]);
	pDemod->GetLayerPer(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &PerNum[0], &PerDen[1]);
	pDemod->GetLayerBer(pDemod, ISDBT_HIERARCHICAL_LAYER_B, &BerNum[1], &BerDen[1]);
	pDemod->GetLayerPer(pDemod, ISDBT_HIERARCHICAL_LAYER_B, &PerNum[1], &PerDen[1]);
	pDemod->GetLayerBer(pDemod, ISDBT_HIERARCHICAL_LAYER_C, &BerNum[2], &BerDen[2]);
	pDemod->GetLayerPer(pDemod, ISDBT_HIERARCHICAL_LAYER_C, &PerNum[2], &PerDen[2]);
	pDemod->GetSnrDb(pDemod, &SnrDbNum, &SnrDbDen);

	pDemodKernel->Unlock(pDemodKernel);

	IfAgc = ((IfAgc + 8191) * 100) / 16384;
	RfAgc = ((RfAgc + 8191) * 100) / 16384;
	DiAgc = (DiAgc * 100) / 4096;

	// printf("BER0=%d/%d, PER0=%d/%d, BER1=%d/%d, PER1=%d/%d, BER2=%d/%d, PER2=%d/%d\n",
	//     BerNum[0], BerDen[0], PerNum[0], PerDen[0],
	//     BerNum[1], BerDen[1], PerNum[1], PerDen[1],
	//     BerNum[2], BerDen[2], PerNum[2], PerDen[2]);

#if 0
	ret = printf("lock(signal)=%d, agc(rf/if/di)="PT_S32BITS"/"PT_S32BITS"/"PT_U32BITS", ofst(tr/cr)="PT_S32BITS"/"PT_S32BITS", err_a(valid/ber/per)="PT_U32BITS"/%f/%f, err_b(valid/ber/per)="PT_U32BITS"/%f/%f, err_c(valid/ber/per)="PT_U32BITS"/%f/%f,snr=%f\n",
				 SignalLock,
				 RfAgc, IfAgc, DiAgc,
				 TrOffsetPpm, CrOffsetHz,
				 RsberValid[0], (double)BerNum[0] / (double)BerDen[0], (double)PerNum[0] / (double)PerDen[0],
				 RsberValid[1], (double)BerNum[1] / (double)BerDen[1], (double)PerNum[1] / (double)PerDen[1],
				 RsberValid[2], (double)BerNum[2] / (double)BerDen[2], (double)PerNum[2] / (double)PerDen[2],
				 (double)SnrDbNum / (double) SnrDbDen);
#endif
	ret = 1;

#ifdef RTK_R_DUMP_SHARE_REG
	DtvDumpShareRegStatus();
#endif


#endif

	return ret;
}



/*----------------------------------------------------------------------
 * Func : Activate
 *
 * Desc : Activate RTK_R QAM Demod
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int RTK_R_ISDBT_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{
	/*
		int i;
		RTK_R_ISDB* pRtkrISDB;
		ISDBT_DEMOD_MODULE* pDemod;
		pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
		pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

		pDemodKernel->Lock(pDemodKernel);

		if (DtvDemodInitialization(RTK_DEMOD_MODE_ISDB, force_rst)) {
			RTK_R_ISDB_INFO("Demod PLL changed, reinit ISDBT Demod\n");
			// pll has been changed and demod has been reset
			// we need to re-init demod

			if (pDemod->Initialize(pDemod) != FUNCTION_SUCCESS)
				goto activate_demod_fail;

			ClearDemodReset(RTK_DEMOD_MODE_ISDB);

			for (i = 0; i < pRtkrISDB->m_TunerOptRegCnt && pRtkrISDB->m_pTunerOptReg; i++) {
				int Reg = pRtkrISDB->m_pTunerOptReg[i].RegBitName;
				U32BITS Val = pRtkrISDB->m_pTunerOptReg[i].Value;
				if (pDemod->SetRegBits(pDemod, Reg, Val) != FUNCTION_SUCCESS)
					goto activate_demod_fail;
			}

			if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
				goto activate_demod_fail;

			if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK)
				goto activate_demod_fail;

			if (pDemodKernel->SetTVSysEx(pDemodKernel, pDemodKernel->m_tv_sys, &pRtkrISDB->m_SigInfo) != TUNER_CTRL_OK)
				goto activate_demod_fail;
		}

		pDemodKernel->Unlock(pDemodKernel);

		return TUNER_CTRL_OK;

	//--------------------------
	activate_demod_fail:
		pDemodKernel->Unlock(pDemodKernel);
		return TUNER_CTRL_FAIL;

	*/
	return TUNER_CTRL_OK;

}

/*----------------------------------------------------------------------
 * Func : SetTvMode
 *
 * Desc : Set Mode of RTK_R_ALL
 *
 * Parm : mode :    RTK_R_MODE_DVBT : DVBT mode
 *                  RTK_R_MODE_DVBC  : DVBC mode
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int RTK_R_ISDBT_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;
	RTK_R_ISDB* pRtkrISDB;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);

	pDemodKernel->Lock(pDemodKernel);

	RTK_R_ISDB_INFO("RTK_R_ISDBT::SetTvMode(%d)\n", mode);

	switch (mode) {

	case TV_SYS_TYPE_ISDBT:
		RTK_R_ISDB_INFO("Set to ISDBT\n");
		ret  = pDemodKernel->Init(pDemodKernel);
		ret |= pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts);
		if (ret != TUNER_CTRL_OK) {
			RTK_R_ISDB_WARNING("Set RTK_R_ISDBT to ISDBT mode failed\n");
			goto set_demod_mode_failed;
		}
		break;

	default:
		goto set_demod_mode_failed;
	}

	ret = TUNER_CTRL_OK;

	RTK_R_ISDB_INFO("[SetTvMode]Set RTK_R_ISDBT to appointed DTV mode successed\n");

	pDemodKernel->Unlock(pDemodKernel);
//end_proc:

	return ret;

//--------------
set_demod_mode_failed:
	pDemodKernel->Unlock(pDemodKernel);
	return TUNER_CTRL_FAIL;
}

void RTK_R_ISDBT_Lock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("LOCK 0x%x\n",&pDemodKernel->m_lock);
	//mutex_lock(&pDemodKernel->m_lock);
}

void RTK_R_ISDBT_Unlock(DEMOD* pDemodKernel)
{
	//FUNCTION_NAME_PRINT("UN_LOCK 0x%x\n",&pDemodKernel->m_lock);
	//mutex_unlock(&pDemodKernel->m_lock);
}


/*-----------------------------------------------------------------
 *     BASE_INTERFACE for RTK_R_ISDBT API
 *----------------------------------------------------------------*/


void ReleaseRtkrDriver(RTK_R_ISDBT_DRIVER_DATA *pDriver)
{
#if 0    //RTK_R_ISDBT_DYNAMIC_ALLOCATE_SHARE_MEMORY
	if (pDriver->pShareMemory) {
		ISDBT_DEMOD_MODULE* pDemod = pDriver->pDemod;

		RTK_R_ISDB_INFO("ReleaseRtkrDriver : hold ISDB-T demod reset to disable share memory access\n");

		pDemod->SetRegBits(pDemod, ISDBT_SOFT_RST_N, 0x0);

		RTK_R_ISDB_INFO("ReleaseRtkrDriver : release share memory - addr=%p/%08lx, size=%08x\n",
						pDriver->pShareMemory,
						pDriver->ShareMemoryPhysicalAddress,
						RTK_R_ISDBTT_DEMUX_DDR_SIZE);

		pli_freeContinuousMemoryMesg("ISDB-T DEMOD", pDriver->pShareMemory);
	}
#endif

	kfree(pDriver);
}


RTK_R_ISDBT_DRIVER_DATA* AllocRtkrDriver_ISDBT(COMM* pComm,	unsigned char Addr, U32BITS CrystalFreq, int TSMode)
{
	RTK_R_ISDBT_DRIVER_DATA* pDriver = (RTK_R_ISDBT_DRIVER_DATA*) kmalloc(
										   sizeof(RTK_R_ISDBT_DRIVER_DATA) +
										   sizeof(ISDBT_DEMOD_MODULE)   +
										   sizeof(I2C_BRIDGE_MODULE)   +
										   sizeof(BASE_INTERFACE_MODULE), GFP_KERNEL);

	if (pDriver) {
		BASE_INTERFACE_MODULE* pBIF;
		ISDBT_DEMOD_MODULE* pDemod;
		memset(pDriver, 0, sizeof(RTK_R_ISDBT_DRIVER_DATA));

		pDriver->pDemod                = (ISDBT_DEMOD_MODULE*)(((unsigned char*)pDriver) + sizeof(RTK_R_ISDBT_DRIVER_DATA));
		pDriver->pBaseInterface        = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(ISDBT_DEMOD_MODULE));
		pDriver->pI2CBridge            = (I2C_BRIDGE_MODULE*)(((unsigned char*)pDriver->pBaseInterface) + sizeof(BASE_INTERFACE_MODULE));
		pDriver->DeviceAddr            = Addr;
		pDriver->CrystalFreqHz         = CrystalFreq;


		// Init Base IF
		pBIF = pDriver->pBaseInterface;

		pBIF->I2cReadingByteNumMax      = RTK_DEMOD_BIF_TX_FIFO_DEPTH;
		pBIF->I2cWritingByteNumMax      = RTK_DEMOD_BIF_RX_FIFO_DEPTH;
		pBIF->I2cRead                   = __realtek_rbus_i2c_read;
		pBIF->I2cWrite                  = __realtek_rbus_i2c_write;
		pBIF->WaitMs                    = __realtek_wait_ms;
		pBIF->SetUserDefinedDataPointer = base_interface_SetUserDefinedDataPointer;
		pBIF->GetUserDefinedDataPointer = base_interface_GetUserDefinedDataPointer;
		pBIF->SetUserDefinedDataPointer(pBIF, (void*)pComm);

#if 0   //RTK_R_ISDBT_DYNAMIC_ALLOCATE_SHARE_MEMORY
		// Dynamic allocate ddr address
		if (RTK_R_ISDBTT_DEMUX_DDR_SIZE) {
			unsigned char* nonCachedAddr;

			pDriver->pShareMemory = (unsigned char*)pli_allocContinuousMemoryMesg(
										"ISDB-T DEMOD",
										RTK_R_ISDBTT_DEMUX_DDR_SIZE,
										&nonCachedAddr,
										&pDriver->ShareMemoryPhysicalAddress);

			if (pDriver->pShareMemory == NULL) {
				RTK_R_ISDB_WARNING("AllocRtkrDriver_ISDBT failed, allocate share memory failed\n");
				kfree(pDriver);
				return NULL;
			}

			RTK_R_ISDB_INFO(" allocate share memory - addr=%p/%08lx, size=%08x\n",
							pDriver->pShareMemory,
							pDriver->ShareMemoryPhysicalAddress,
							RTK_R_ISDBTT_DEMUX_DDR_SIZE);

			if (pDriver->ShareMemoryPhysicalAddress & 0xfffff) {
				RTK_R_ISDB_WARNING("AllocRtkrDriver_ISDBT failed, dynamic share memory allocation is not  1MB align\n");
				//assert(0);
			}


		}
#else   // RTK_R_ISDBT_STATIC_ALLOCATE_SHARE_MEMORY

		if (RTK_R_ISDBTT_DEMUX_DDR_SIZE) {

			unsigned int AllocateSize = 0;

			AllocateSize = CravedoutMemoryQuery(&pDriver->ShareMemoryPhysicalAddress);

			if (AllocateSize < (RTK_R_ISDBTT_DEMUX_DDR_SIZE)) {
				RTK_R_ISDB_WARNING("AllocRtkrDriver_ISDBT failed, allocate share memory failed\n");
				kfree(pDriver);
				return NULL;
			}

			RTK_R_ISDB_INFO("allocate share memory(fix) - Physical_addr=%08lx, size=%08x\n", pDriver->ShareMemoryPhysicalAddress, AllocateSize);


			if (pDriver->ShareMemoryPhysicalAddress & 0xfff) {
				RTK_R_ISDB_WARNING("AllocRtkrDriver_ISDBT failed, dynamic share memory allocation is not  4KB align\n");
				//assert(0);
			}


		}



#endif
		// Init Demod Driver
		BuildRtkrIsdbtModule(&pDemod,
							 pDriver->pDemod,
							 pDriver->pBaseInterface,
							 pDriver->pI2CBridge,
							 Addr,
							 CrystalFreq,
							 TSMode,
							 IF_FREQ_36000000HZ,         // IF frequency is 36.0 Mhz.
							 SPECTRUM_NORMAL,            // Spectrum mode is inverse.
							 pDriver->ShareMemoryPhysicalAddress, // Share memory address....
							 RTK_R_ISDBT_CHIP_VERSION_B);
	}

	return pDriver;
}

#if USE_PREFRM_LOCK
void RTK_R_ISDBT_IsPiSnrOverThd(DEMOD* pDemodKernel, unsigned char *u8IsPiSnrOverThd)
{
	U32BITS Z_AVG = 0, N_AVG = 0;
	RTK_R_ISDB* pRtkrISDB;
	ISDBT_DEMOD_MODULE* pDemod;
	pRtkrISDB = (RTK_R_ISDB*)(pDemodKernel->private_data);
	pDemod = ((RTK_R_ISDBT_DRIVER_DATA*) pRtkrISDB->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);
	pDemod->GetRegBits(pDemod, ISDBT_DPBSK_Z_AVG_DBG, &Z_AVG);
	pDemod->GetRegBits(pDemod, ISDBT_DPBSK_N_AVG_DBG, &N_AVG);
	pDemodKernel->Unlock(pDemodKernel);
	RTK_R_ISDB_DBG("[PRE_API]Z_AVG = 0x"PT_H32BITS", N_AVG = 0x"PT_H32BITS"\n", Z_AVG, N_AVG);

	*u8IsPiSnrOverThd = 0;
	//dbpsk_z_avg_dbg * 2^7 > dbpsk_n_avg_dbg*3981.072
	//dbpsk_z_avg_dbg > dbpsk_n_avg_dbg * 31.102125
	if (Z_AVG * DBPSK_Z_AVG_RATIO > N_AVG * DBPSK_N_AVG_RATIO) {
		*u8IsPiSnrOverThd = 1;
	} else {
		*u8IsPiSnrOverThd = 0;
	}
}
#endif