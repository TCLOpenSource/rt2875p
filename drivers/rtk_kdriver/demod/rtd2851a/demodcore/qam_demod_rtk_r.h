#ifndef __DEMOD_REALTEK_R_QAM_H
#define __DEMOD_REALTEK_R_QAM_H

/**

@file

@brief   REALTEK_R QAM demod module declaration

One can manipulate REALTEK_R QAM demod through REALTEK_R QAM module.
REALTEK_R QAM module is derived from QAM demod module.



@par Example:
@code

// The example is the same as the QAM demod example in qam_demod_base.h except the listed lines.



#include "demod_REALTEK_R_qam.h"


...



int main(void)
{
	QAM_DEMOD_MODULE *pDemod;

	QAM_DEMOD_MODULE      QamDemodModuleMemory;
	BASE_INTERFACE_MODULE BaseInterfaceModuleMemory;
	I2C_BRIDGE_MODULE     I2cBridgeModuleMemory;


	...



	// Build REALTEK_R QAM demod module.
	BuildREALTEK_RQamModule(
		&pDemod,
		&QamDemodModuleMemory,
		&BaseInterfaceModuleMemory,
		&I2cBridgeModuleMemory,
		0x24,								// I2C device address is 0x24 in 8-bit format.
		CRYSTAL_FREQ_25200000HZ,			// Crystal frequency is 25.2 MHz.
		REALTEK_R_QAM_CONFIG_OC				// Configuration mode is OpenCable.
		);



	// See the example for other QAM demod functions in qam_demod_base.h

	...


	return 0;
}


@endcode

*/

#include "qam_demod_rtk_r_base.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define OC_AutoDetectMode

// Definitions
#define REALTEK_R_QAM_BASE_REG_TABLE_LEN				89
#define REALTEK_R_QAM_MONITOR_REG_TABLE_LEN			6
#define REALTEK_R_QAM_SYS_VERSION_VALUE				0xa6
#define REALTEK_R_QAM_DDC_FREQ_FRAC_BIT_NUM			15
#define REALTEK_R_QAM_TR_DECI_RATIO_FRAC_BIT_NUM		18


// PLL lock register
#define REALTEK_R_QAM_BACKEND_PLL_LOCK			0x0

// RF_AGC_VALUE register
#define REALTEK_R_QAM_RF_AGC_VALUE_BIT_NUM		11

// IF_AGC_VALUE register
#define REALTEK_R_QAM_IF_AGC_VALUE_BIT_NUM		11

// CR_OFFSET register
#define REALTEK_R_QAM_CR_OFFSET_BIT_NUM			32

// TR_OFFSET register
#define REALTEK_R_QAM_TR_OFFSET_BIT_NUM			31


// BER and UPER
#define REALTEK_R_QAM_BER_WAIT_TIME_MS			10
#define REALTEK_R_OC_BER_REG2_MSB_SHIFT			16
#define REALTEK_R_DVBC_BER_REG2_MSB_SHIFT			16

// SNR
// Note: REALTEK_R_QAM_SNR_DB_DEN = round(log2(10) * pow(2, REALTEK_R_QAM_SNR_FRAC_BIT_NUM))
#define REALTEK_R_QAM_SNR_FRAC_BIT_NUM			10
#define REALTEK_R_QAM_SNR_DB_DEN					3402


// Singal strength and signal quality
#define REALTEK_R_QAM_SIGNAL_QUALITY_FRAC_BIT_NUM			7


// OpenCable Function 1
#define REALTEK_R_OC_FUNC1_TICK_NUM_MAX		3


// Table length
#define REALTEK_R_QAM_SPEC_REG_0_VALUE_TABLE_LEN						11
#define REALTEK_R_QAM_SYMBOL_RATE_VALUE_TABLE_LEN						11
#define REALTEK_R_QAM_ALPHA_VALUE_TABLE_LEN							16
#define REALTEK_R_QAM_SYMBOL_RATE_TABLE_LEN							3
#define REALTEK_R_OC_INIT_REG_TABLE_LEN								187
#define REALTEK_R_OC_QAM_MODE_REG_TABLE_LEN							37
#define REALTEK_R_OC_QAM_MODE_NUM										4
#define REALTEK_R_DVBC_INIT_REG_TABLE_LEN								190
#define REALTEK_R_DVBC_QAM_MODE_REG_TABLE_LEN							32
#define REALTEK_R_DVBC_QAM_MODE_NUM									10


// NBD and SRS retry time
#define REALTEK_R_DVBC_SRS_RETRY_TIME									5
#define REALTEK_R_DVBC_NBD_THD										0x20


#define QAMD_THD_16QAM   300
#define QAMD_THD_32QAM   240
#define QAMD_THD_64QAM   334
#define QAMD_THD_128QAM   28
#define QAMD_THD_256QAM   40


// Configuration modes
enum {
	REALTEK_R_QAM_CONFIG_OC,					// OpenCable
	REALTEK_R_QAM_CONFIG_OC_AM_HUM_EN,		// OpenCable with AM-hum enhancement
	REALTEK_R_QAM_CONFIG_DVBC,				// DVB-C
	REALTEK_R_QAM_CONFIG_DVBC_AM_HUM_EN,		// DVB-C with AM-hum enhancement
};

#ifndef ABS
#define ABS(x,y)				((x > y) ? (x-y) : (y-x))
#endif

// DVBC definitions
#define  FFT_SIZE		8192 
#define  SAMPLE_RATE_KHZ	36000 //ADC 36M
#define  SAMPLE_RATE_10KHZ	3600

#define  OC_SAMPLE_RATE_KHZ	25200 //ADC 25.2M
#define  OC_SAMPLE_RATE_10KHZ	2520

// Builder
void
BuildRealtekRQamModule(
	QAM_DEMOD_MODULE **ppDemod,
	QAM_DEMOD_MODULE *pQamDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	U32BITS CrystalFreqHz,
	int ConfigMode
);


// Manipulaing functions
void
realtek_r_qam_IsConnectedToI2c(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_qam_SoftwareReset(
	QAM_DEMOD_MODULE *pDemod
);

int
realtek_r_qam_RSReset(
	QAM_DEMOD_MODULE *pDemod
);

int
realtek_r_qam_SetSymbolRateHz(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
);

int
realtek_r_qam_SetIfFreqHz(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS IfFreqHz
);

int
realtek_r_qam_SetSpectrumMode(
	QAM_DEMOD_MODULE *pDemod,
	int SpectrumMode
);

int
realtek_r_qam_SetAagcLoopPol(
	QAM_DEMOD_MODULE *pDemod,
	int AagcLoopPol
);

int
realtek_r_qam_GetRfAgc(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pRfAgc
);

int
realtek_r_qam_GetIfAgc(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pIfAgc
);

int
realtek_r_qam_GetDiAgc(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pDiAgc
);

int
realtek_r_qam_GetTrOffsetPpm(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pTrOffsetPpm
);

int
realtek_r_qam_GetCrOffsetHz(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pCrOffsetHz
);

int
realtek_r_qam_IsAagcLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_qam_IsEqLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_qam_GetSnrDb(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pSnrDbNum,
	S32BITS *pSnrDbDen
);

int
realtek_r_qam_GetSignalStrength(
	QAM_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	U32BITS *pSignalStrength
);

int
realtek_r_qam_GetSignalQuality(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pSignalQuality
);


// Manipulaing functions for OpenCable
int
realtek_r_oc_Initialize(
	QAM_DEMOD_MODULE *pDemod
);

int
realtek_r_oc_SetQamMode(
	QAM_DEMOD_MODULE *pDemod,
	int QamMode
);

int
realtek_r_oc_am_hum_en_SetQamMode(
	QAM_DEMOD_MODULE *pDemod,
	int QamMode
);

int
realtek_r_oc_SetAlphaMode(
	QAM_DEMOD_MODULE *pDemod,
	int AlphaMode
);

int
realtek_r_oc_IsFrameLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);


int
realtek_r_oc_IsTrLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_oc_IsCrLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_oc_IsInnerLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_oc_IsNoSig(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_oc_GetErrorRate(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS TestVolume,
	unsigned int WaitTimeMsMax,
	U32BITS *pBerNum,
	U32BITS *pBerDen,
	U32BITS *pPerNum,
	U32BITS *pPerDen
);

int
realtek_r_oc_GetSignalStrength(
	QAM_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	U32BITS *pSignalStrength
);

int
realtek_r_oc_GetSignalQuality(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pSignalQuality
);

int
realtek_r_oc_UpdateFunction(
	QAM_DEMOD_MODULE *pDemod
);

int
realtek_r_oc_ResetFunction(
	QAM_DEMOD_MODULE *pDemod
);

int
realtek_r_oc_NbdAndSrs(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char *pNosig,
	U32BITS *pPowerDiff,
	U32BITS *pSymbolRate,
	unsigned char WaitAgcStableFlag
);

int
realtek_r_oc_Qamd(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pQammode,
	unsigned char MetricEn
);


int
realtek_r_oc_AutoMode(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char *pNosig,
	U32BITS *pCSR,
	U32BITS *pFSR,
	S32BITS *pCCFO,
	unsigned char *pQamMode
);


// Manipulaing functions for DVB-C
int
realtek_r_dvbc_Initialize(
	QAM_DEMOD_MODULE *pDemod
);

int
realtek_r_dvbc_SetQamMode(
	QAM_DEMOD_MODULE *pDemod,
	int QamMode
);

int
realtek_r_dvbc_GetQamMode(
	QAM_DEMOD_MODULE *pDemod,
	int *pQamMode
);


int
realtek_r_dvbc_am_hum_en_SetQamMode(
	QAM_DEMOD_MODULE *pDemod,
	int QamMode
);

int
realtek_r_dvbc_SetAlphaMode(
	QAM_DEMOD_MODULE *pDemod,
	int AlphaMode
);

int
realtek_r_dvbc_IsFrameLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dvbc_IsTrLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dvbc_IsCrLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dvbc_IsInnerLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dvbc_IsNoSig(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dvbc_GetErrorRate(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS TestVolume,
	unsigned int WaitTimeMsMax,
	U32BITS *pBerNum,
	U32BITS *pBerDen,
	U32BITS *pPerNum,
	U32BITS *pPerDen
);

int
realtek_r_dvbc_UpdateFunction(
	QAM_DEMOD_MODULE *pDemod
);

int
realtek_r_dvbc_ResetFunction(
	QAM_DEMOD_MODULE *pDemod
);


int
realtek_r_dvbc_NbdAndSrs(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char *pNosig,
	U32BITS *pPowerDiff,
	U32BITS *pSymbolRate,
	unsigned char WaitAgcStableFlag
);

int
realtek_r_dvbc_Qamd(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pQammode,
	unsigned char MetricEn
);

int
realtek_r_dvbc_QamRegPreset(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
);

int
realtek_r_dvbc_QamRegPostset(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
);

int
realtek_r_dvbc_QamPreCheck(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
);

int
realtek_r_dvbc_SetCcfoMethodEn(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char enable
);

int
realtek_r_dvbc_IsCcfoMethodEn(
	QAM_DEMOD_MODULE *pDemod,
	unsigned int* pccfo_en_flag
);

void
realtek_r_dvbc_GetCcfoInitFreq(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRate,
	unsigned int *pCfoValue,
	int *pEstCcfo
);


int
realtek_r_dvbc_SetCcfoInitFreq(
	QAM_DEMOD_MODULE *pDemod,
	unsigned int CfoValue
);

int
realtek_r_qam_GetSrsCcfoDoneFlag(
	QAM_DEMOD_MODULE *pDemod,
	unsigned int *SrsCcfoDoneFlag
);

int  realtek_r_dvbc_TSBitAccu(
	QAM_DEMOD_MODULE *pDemod,
	unsigned int *TSThroughputKBps
);

int
realtek_r_dvbc_SetAutoModeEn(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char enable
);

int
realtek_r_dvbc_AutoMode(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char *pNosig,
	U32BITS *pCSR,
	U32BITS *pFSR,
	S32BITS *pCCFO,
	unsigned char *pQamMode
);

int
realtek_r_dvbc_AutoModePreset(
	QAM_DEMOD_MODULE *pDemod
);

int
realtek_r_dvbc_AutoModePostset(
	QAM_DEMOD_MODULE *pDemod
);

int
realtek_r_dvbc_AutoModeSetSrHz(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
);

// Extra manipulaing functions
void
realtek_r_qam_GetConfigMode(
	QAM_DEMOD_MODULE *pDemod,
	int *pConfigMode
);

int
realtek_r_qam_SetFftMode(
	QAM_DEMOD_MODULE *pDemod,
	FFTMODE mode
);

// Backend register access functions
int
realtek_r_qam_backend_SetRegDword(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS RegAddr,
	const U32BITS WritingDword
);

int
realtek_r_qam_backend_GetRegDword(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS RegAddr,
	U32BITS *pReadingDword
);

int
realtek_r_qam_backend_SetRegMaskBits(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS RegAddr,
	unsigned char Msb,
	unsigned char Lsb,
	const U32BITS WritingValue
);

int
realtek_r_qam_backend_GetRegMaskBits(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS RegAddr,
	unsigned char Msb,
	unsigned char Lsb,
	U32BITS *pReadingValue
);

int
realtek_r_qam_backend_Initialize(
	QAM_DEMOD_MODULE *pDemod
);


// I2C command forwarding functions
int
realtek_r_qam_ForwardI2cReadingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	unsigned char *pReadingBytes,
	U32BITS ByteNum
);

int
realtek_r_qam_ForwardI2cWritingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	const unsigned char *pWritingBytes,
	U32BITS ByteNum
);


/*
// Register table initialization
void
realtek_r_qam_InitBaseRegTable(
	QAM_DEMOD_MODULE *pDemod
);
*/


/*
void
realtek_r_qam_InitMonitorRegTable(
	QAM_DEMOD_MODULE *pDemod
);
*/


// Register access methods
int
realtek_r_qam_acceleration_SetRegMaskBits(
	QAM_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	const U32BITS WritingValue
);

int
realtek_r_qam_GetInnerStrobeRegBits(
	QAM_DEMOD_MODULE *pDemod,
	int RegBitName,
	U32BITS *pReadingValue
);

int
realtek_r_qam_GetMonitorRegBits(
	QAM_DEMOD_MODULE *pDemod,
	int MonitorRegBitName,
	U32BITS *pReadingValue
);


// I2C birdge module builder
void
realtek_r_qam_BuildI2cBridgeModule(
	QAM_DEMOD_MODULE *pDemod
);


#ifdef __cplusplus
}
#endif
#endif
