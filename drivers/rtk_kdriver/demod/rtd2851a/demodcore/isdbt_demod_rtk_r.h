#ifndef __DEMOD_RTK_R_ISDBT_H
#define __DEMOD_RTK_R_ISDBT_H

#ifdef __cplusplus
extern "C" {
#endif
/**

@file

@brief   RTK_R ISDB-T demod module declaration

One can manipulate RTK_R ISDB-T demod through RTK_R ISDB-T module.
RTK_R ISDB-T module is derived from ISDB-T demod module.



@par Example:
@code

// The example is the same as the ISDB-T demod example in isdbt_demod_base.h except the listed lines.



#include "demod_rtk_r_isdbt.h"


...



int main(void)
{
	ISDBT_DEMOD_MODULE *pDemod;

	ISDBT_DEMOD_MODULE     DvbtDemodModuleMemory;
	BASE_INTERFACE_MODULE BaseInterfaceModuleMemory;
	I2C_BRIDGE_MODULE     I2cBridgeModuleMemory;


	...



	// Build RTK_R ISDB-T demod module.
	BuildRtkrIsdbtModule(
		&pDemod,
		&DvbtDemodModuleMemory,
		&BaseInterfaceModuleMemory,
		&I2cBridgeModuleMemory,
		0x24,								// I2C device address is 0x24 in 8-bit format.
		CRYSTAL_FREQ_27000000HZ,			// Crystal frequency is 27.0 MHz.
		TS_INTERFACE_PARALLEL,				// TS interface mode is parallel.
		IF_FREQ_36000000HZ,					// IF frequency is 36.0 Mhz.
		SPECTRUM_INVERSE					// Spectrum mode is inverse.
		);



	// See the example for other ISDB-T demod functions in isdbt_demod_base.h

	...


	return 0;
}


@endcode

*/


#include "isdbt_demod_rtk_r_base.h"





// Definitions

// Size of Shared Memory
#define RTK_R_ISDBT_DEMUX_DDR_SIZE            0x400000        // 4MB

// ADC to crystal frequency ratio
#define RTK_R_ISDBT_ADC_TO_CRYSTAL_FREQ_RATIO   2


// Parameter setting
#define RTK_R_ISDBT_GROUP_0_SELECTION_NUM   4
#define RTK_R_ISDBT_GROUP_0_BIT_NUM         12

#define RTK_R_ISDBT_GROUP_1_SELECTION_NUM   3
#define RTK_R_ISDBT_GROUP_1_BIT_NUM         12

#define RTK_R_ISDBT_GROUP_2_SELECTION_NUM   3

#define RTK_R_ISDBT_GROUP_3_SELECTION_NUM   2

#define RTK_R_ISDBT_GROUP_4_SELECTION_NUM   15


// Bandwidth setting
#define RTK_R_ISDBT_CFREQ_OFF_RATIO_BIT_NUM		20


// IF frequency setting
#define RTK_R_ISDBT_IFFREQ_BIT_NUM	10


// Signal quality
#define RTK_R_ISDBT_SIGNAL_QUALITY_FRAC_BIT_NUM   5


// BER
#define RTK_R_ISDBT_BER_DEN_VALUE         (0x1 << 17)
#define RTK_R_ISDBT_BER_NUM_VALUE_ERROR   RTK_R_ISDBT_BER_DEN_VALUE


// PER
#define RTK_R_ISDBT_PER_DEN_VALUE   (0x1 << 16)


// SNR
#define RTK_R_ISDBT_EST_SNR_BIT_NUM    9
#define RTK_R_ISDBT_SNR_DB_DEN_VALUE   4


// AGC
#define RTK_R_ISDBT_RF_AGC_REG_BIT_NUM   14
#define RTK_R_ISDBT_IF_AGC_REG_BIT_NUM   14


// TR offset and CR offset
#define RTK_R_ISDBT_SFO_OUT_BIT_NUM   15
#define RTK_R_ISDBT_CFO_EST_BIT_NUM   27

#define RTK_R_ISDBT_CHIP_VERSION_A    0
#define RTK_R_ISDBT_CHIP_VERSION_B    1

#define USE_PREFRM_LOCK	1
	// GROUP_0 setting entry only used in SetParameters()
	typedef struct {
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		S32BITS WritingValue[RTK_R_ISDBT_GROUP_0_SELECTION_NUM];
	}
	SETTING_ENTRY_GROUP_0;

	// GROUP_1 setting entry only used in SetParameters()
	typedef struct {
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		S32BITS WritingValue[RTK_R_ISDBT_GROUP_1_SELECTION_NUM];
	}
	SETTING_ENTRY_GROUP_1;

	// GROUP_2 setting entry only used in SetParameters()
	typedef struct {
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		U32BITS WritingValue[RTK_R_ISDBT_GROUP_2_SELECTION_NUM];
	}
	SETTING_ENTRY_GROUP_2;

	// GROUP_3 setting entry only used in SetParameters()
	typedef struct {
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		U32BITS WritingValue[RTK_R_ISDBT_GROUP_3_SELECTION_NUM];
	}
	SETTING_ENTRY_GROUP_3;

	// GROUP_4 setting entry only used in SetParameters()
	typedef struct {
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		U32BITS WritingValue[RTK_R_ISDBT_GROUP_4_SELECTION_NUM];
	}
	SETTING_ENTRY_GROUP_4;
// Demod module builder
void
BuildRtkrIsdbtModule(
	ISDBT_DEMOD_MODULE **ppDemod,
	ISDBT_DEMOD_MODULE *pDvbtDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	U32BITS CrystalFreqHz,
	int TsInterfaceMode,
	U32BITS IfFreqHz,
	int SpectrumMode,
	U32BITS ShareMemoryOfst,
	unsigned char ChipVersion
);





// Manipulating functions
void
realtek_r_isdbt_IsConnectedToI2c(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_isdbt_RestartFsm(
	ISDBT_DEMOD_MODULE *pDemod
);

int
realtek_r_isdbt_Initialize(
	ISDBT_DEMOD_MODULE *pDemod
);

int
realtek_r_isdbt_SetTsInterfaceMode(
	ISDBT_DEMOD_MODULE *pDemod,
	int TsInterfaceMode
);

int
realtek_r_isdbt_SetTsInterfaceClkPol(
	ISDBT_DEMOD_MODULE *pDemod,
	int TsInterfaceClkPol
);

int
realtek_r_isdbt_SetIfFreqHz(
	ISDBT_DEMOD_MODULE *pDemod,
	U32BITS IfFreqHz
);

int
realtek_r_isdbt_SetSpectrumMode(
	ISDBT_DEMOD_MODULE *pDemod,
	int SpectrumMode
);

int
realtek_r_isdbt_SetParameters(
	ISDBT_DEMOD_MODULE *pDemod,
	int BandwidthMode,
	unsigned char SegmentNum,
	unsigned char SubChannelNum
);

int
realtek_r_isdbt_IsSignalLocked(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_isdbt_IsTrLocked(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_isdbt_IsCrLocked(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_isdbt_IsInnerLocked(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_isdbt_IsNoSig(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_isdbt_GetSignalStrength(
	ISDBT_DEMOD_MODULE *pDemod,
	U32BITS *pSignalStrength
);

int
realtek_r_isdbt_GetLayerSignalQuality(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	U32BITS *pLayerSignalQuality
);

int
realtek_r_isdbt_GetLayerBer(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	U32BITS *pLayerBerNum,
	U32BITS *pLayerBerDen
);

int
realtek_r_isdbt_GetLayerPer(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	U32BITS *pLayerPerNum,
	U32BITS *pLayerPerDen
);

int
realtek_r_isdbt_GetSnrDb(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pSnrDbNum,
	S32BITS *pSnrDbDen
);

int
realtek_r_isdbt_GetRfAgc(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pRfAgc
);

int
realtek_r_isdbt_GetIfAgc(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pIfAgc
);

int
realtek_r_isdbt_GetDiAgc(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pDiAgc
);

int
realtek_r_isdbt_GetTrOffsetPpm(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pTrOffsetPpm
);

int
realtek_r_isdbt_GetCrOffsetHz(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pCrOffsetHz
);

int
realtek_r_isdbt_GetLayerConstellation(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	int *pLayerConstellation
);

int
realtek_r_isdbt_GetLayerCodeRate(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	int *pLayerCodeRate
);

int
realtek_r_isdbt_GetLayerInterleavingLen(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	int *pLayerInterleavingLen
);

int
realtek_r_isdbt_GetLayerSegmentNum(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	unsigned char *pLayerSegmentNum
);

int
realtek_r_isdbt_GetGuardInterval(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pGuardInterval
);

int
realtek_r_isdbt_GetFftMode(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pFftMode
);





// I2C command forwarding functions
int
realtek_r_isdbt_ForwardI2cReadingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	unsigned char *pReadingBytes,
	unsigned long ByteNum
);

int
realtek_r_isdbt_ForwardI2cWritingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	const unsigned char *pWritingBytes,
	unsigned long ByteNum
);





// Register table initializing
void
realtek_r_isdbt_InitRegTable(
	ISDBT_DEMOD_MODULE *pDemod
);





// I2C birdge module builder
void
realtek_r_isdbt_BuildI2cBridgeModule(
	ISDBT_DEMOD_MODULE *pDemod
);

// Download firmware code (for demo only)
int
realtek_r_isdbt_DownloadFirmwareCode(
	ISDBT_DEMOD_MODULE *pDemod
);


// accelerated Set Reg Mask Bits
int
realtek_r_isdb_acceleration_SetRegMaskBits(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	const U32BITS WritingValue
);













#ifdef __cplusplus
}
#endif


#endif
