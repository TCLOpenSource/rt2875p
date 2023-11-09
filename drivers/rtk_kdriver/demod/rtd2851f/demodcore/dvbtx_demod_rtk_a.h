#ifndef __DEMOD_RTK_A_DVBTX_H
#define __DEMOD_RTK_A_DVBTX_H

#ifdef __cplusplus
extern "C" {
#endif



#include "dvbtx_demod_rtk_a_base.h"





// Definitions

// Size of Shared Memory
#define RTK_A_DVBTX_DEMUX_DDR_SIZE            0x400000        // 4MB





// Demod module builder
void
BuildRtkaDvbtxModule(
	DVBTX_DEMOD_MODULE **ppDemod,
	DVBTX_DEMOD_MODULE *pDvbtDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	unsigned int CrystalFreqHz
);


void
realtek_a_dvbtx_GetCrystalFreqHz(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pCrystalFreqHz
);


// Manipulating functions
int
realtek_a_dvbtx_Initialize(
	DVBTX_DEMOD_MODULE *pDemod
);

int
realtek_a_dvbtx_SetTsInterfaceMode(
	DVBTX_DEMOD_MODULE *pDemod,
	RTK_A_DVBTX_MPEGTSMode stTSMode
);


int realtek_a_dvbtx_SetTSOutputOnOff(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char ui8TSOnOff
);

int
realtek_a_dvbtx_SetTsInterfaceClkPol(
	DVBTX_DEMOD_MODULE *pDemod,
	int TsInterfaceClkPol
);

int
realtek_a_dvbtx_SetIfFreqHz(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int IfFreqHz
);


int
realtek_a_dvbtx_SetSpectrumMode(
	DVBTX_DEMOD_MODULE *pDemod,
	int SpectrumMode
);

int
realtek_a_dvbtx_SetBandwidthMode(
	DVBTX_DEMOD_MODULE *pDemod,
	int BandwidthMode
);

int
realtek_a_dvbtx_IsSignalLocked(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_a_dvbtx_IsDVBTxSignalSyncLock(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_a_dvbtx_IsDVBTSignalSyncLock(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_a_dvbtx_IsDVBT2SignalSyncLock(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
);


int
realtek_a_dvbtx_GetDVBTxSignalParamsLockFlag(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
);


int
realtek_a_dvbtx_IsDVBT2L1Lock(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_a_dvbtx_GetSignalStrength(
	DVBTX_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	unsigned int *pSignalStrength
);

int
realtek_a_dvbtx_GetLayerSignalQuality(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pSignalQuality
);

int
realtek_a_dvbtx_GetBer(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *ui32BERNum,
	unsigned int *ui32BERDen
);

int
realtek_a_dvbtx_GetPer(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *ui32PEC_NUM,
	unsigned int *ui32PEC_DEN
);

int
realtek_a_dvbtx_GetSnrDb(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *ui32SNRValuex10
);


int
realtek_a_dvbtx_SetAgcPol(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char agcpola
);

int
realtek_a_dvbtx_GetIfAgc(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pIfAgc
);


int
realtek_a_dvbtx_GetTrOffsetPpm(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pTrOffsetPpm
);

int
realtek_a_dvbtx_GetCrOffsetHz(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pCrOffsetHz
);

int
realtek_a_dvbtx_GetCellId(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pCellId
);

int
realtek_a_dvbtx_SetTHierarchy(
	DVBTX_DEMOD_MODULE *pDemod,
	int Hierarchy
);

int
realtek_a_dvbtx_SetT2PlpID(
	DVBTX_DEMOD_MODULE *pDemod,
	int T2PlpID
);

int
realtek_a_dvbtx_GetDVBTxMode(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char * pu8DVBTxMode
);

int
realtek_a_dvbtx_GetSignalParamsT2(
	DVBTX_DEMOD_MODULE *pDemod,
	RTK_A_DVBTX_DVBT2_PARAMS *pDvbT2Params
);

int
realtek_a_dvbtx_GetSignalParamsT(
	DVBTX_DEMOD_MODULE *pDemod,
	RTK_A_DVBTX_DVBT_PARAMS *pDvbTParams
);

void
realtek_a_dvbtx_SetTSVarThresh(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char Nvco
);


int
realtek_a_dvbtx_TSBitAccu(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *TSThroughputBps
);

int
realtek_a_dvbtx_CellIDOnOff(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char ui8CellIDOnOff
);

int realtek_a_dvbtx_CheckT2TxIDErrSignal(
	DVBTX_DEMOD_MODULE *pDemod
);


int realtek_a_dvbtx_T2TxIDErrSignalRestore(
	DVBTX_DEMOD_MODULE *pDemod
);

int realtek_a_dvbtx_DyStaitcCh_InitSet(
	DVBTX_DEMOD_MODULE *pDemod
);

int realtek_a_dvbtx_CheckbyStaicChPwrChange(
    DVBTX_DEMOD_MODULE *pDemod
);

int realtek_a_dvbtx_CheckDyStaicChl(
	DVBTX_DEMOD_MODULE *pDemod
);


#ifdef __cplusplus
}
#endif


#endif
