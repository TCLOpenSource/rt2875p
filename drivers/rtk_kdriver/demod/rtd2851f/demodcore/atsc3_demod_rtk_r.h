#ifndef __DEMOD_RTK_R_ATSC3_H
#define __DEMOD_RTK_R_ATSC3_H

#ifdef __cplusplus
extern "C" {
#endif



#include "atsc3_demod_rtk_r_base.h"


// Definitions

// Size of Shared Memory
#define RTK_R_ATSC3_DEMUX_DDR_SIZE            0x400000        // 4MB





// Demod module builder
void
BuildRtkrAtsc3Module(
	ATSC3_DEMOD_MODULE **ppDemod,
	ATSC3_DEMOD_MODULE *pDvbtDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	unsigned int CrystalFreqHz
);

/*
void
realtek_r_atsc3_GetCrystalFreqHz(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pCrystalFreqHz
);
*/

void
realtek_r_atsc3_IsConnectedToI2c(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
);

// Manipulating functions
int
realtek_r_atsc3_Initialize(
	ATSC3_DEMOD_MODULE *pDemod
);

int
realtek_r_atsc3_SetTsInterfaceMode(
	ATSC3_DEMOD_MODULE *pDemod,
	RTK_R_ATSC3_MPEGTSMode stTSMode
);


int
realtek_r_atsc3_SetTsInterfaceClkPol(
	ATSC3_DEMOD_MODULE *pDemod,
	int TsInterfaceClkPol
);

int
realtek_r_atsc3_SetIfFreqHz(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int IfFreqHz
);


int
realtek_r_atsc3_SetSpectrumMode(
	ATSC3_DEMOD_MODULE *pDemod,
	int SpectrumMode
);

int
realtek_r_atsc3_SetBandwidthMode(
	ATSC3_DEMOD_MODULE *pDemod,
	int BandwidthMode
);

int
realtek_r_atsc3_IsSignalLocked(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_atsc3_IsSignalSyncLock(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
);


int
realtek_r_atsc3_IsL1Lock(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_atsc3_SoftwareReset(
	ATSC3_DEMOD_MODULE *pDemod
);

int
realtek_r_atsc3_GetSignalStrength(
	ATSC3_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	unsigned int *pSignalStrength
);

int
realtek_r_atsc3_GetLayerSignalQuality(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pSignalQuality
);

int
realtek_r_atsc3_GetBer(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_ErrRate *PacketErr
);

int
realtek_r_atsc3_GetPer(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_ErrRate *PacketErr
);

int
realtek_r_atsc3_GetSnrDb(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *ui32SNRValuex10
);


int
realtek_r_atsc3_SetAgcPol(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char agcpola
);

int
realtek_r_atsc3_GetIfAgc(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pIfAgc
);


int
realtek_r_atsc3_GetTrOffsetPpm(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pTrOffsetPpm
);

int
realtek_r_atsc3_GetCrOffsetHz(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pCrOffsetHz
);


int
realtek_r_atsc3_SetPlpID(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_PLP_ID Atsc3Plp
);


int
realtek_r_atsc3_GetPlpID(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_PLP_ID* Atsc3Plp,
	ATSC3_MPLP_ID_MODE_SEL Atsc3MPlpMode
);

int
realtek_r_atsc3_GetSignalParams(
	ATSC3_DEMOD_MODULE *pDemod,
	RTK_R_ATSC3_PARAMS *pAtsc3Params
);

int
realtek_r_atsc3_TSBitAccu(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *TSThroughputBps
);


int
realtek_r_atsc3_vv526_detect_flow(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char* flag
);


int
realtek_r_atsc3_kvv_fdm_check_flow(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char* flag
);


int  realtek_r_atsc3_multisubframe_fft8K_ce_fix(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char* flag
);

int  realtek_r_atsc3_mplp_cti_detect(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char* flag
);

int  realtek_r_atsc3_config1PlpElOnly_multisubframe(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char* flag
);

int  realtek_r_atsc3_2plp_hti_TiExtInter_detect(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char* flag,
	TV_ATSC3_PLP_ID* pAtsc3PlpAll,
	TV_ATSC3_PLP_ID* pAtsc3PlpLlsOnly
);

int
realtek_r_atsc3_restore_default_setting(
	ATSC3_DEMOD_MODULE *pDemod
);


int
realtek_r_atsc3_L1CRCErr_check(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char* status
);


int  realtek_r_atsc3_SetLdpcIterNum(
	ATSC3_DEMOD_MODULE * pDemod,
	unsigned char mode,
	unsigned char num
);

int  realtek_r_atsc3_SetLdpcLatency(
	ATSC3_DEMOD_MODULE * pDemod,
	unsigned char ldpcLatencyCtrlEn,
	unsigned char ldpcLatencyVal
);

int  realtek_r_atsc3_LdpcParamCheck(
	ATSC3_DEMOD_MODULE * pDemod
);


int  realtek_r_atsc3_L1DUpdateItemCheck(
	ATSC3_DEMOD_MODULE * pDemod
);

int  realtek_r_atsc3_1PLPLdpcOnlyDetect(
	ATSC3_DEMOD_MODULE * pDemod
);

int  realtek_r_atsc3_LdpcAlphaFixModeEn(
	ATSC3_DEMOD_MODULE * pDemod,
	unsigned char enable
);

int  realtek_r_atsc3_L1DataDump(
	ATSC3_DEMOD_MODULE * pDemod,
	TV_ATSC3_PLP_ID* Atsc3Plp
);


int  realtek_r_atsc3_GetChipId(
	ATSC3_DEMOD_MODULE * pDemod,
	unsigned char* chipId
);


int  realtek_r_atsc3_L1BD_latency_modify(
	ATSC3_DEMOD_MODULE * pDemod,
	unsigned char* flag
);



#ifdef __cplusplus
}
#endif


#endif
