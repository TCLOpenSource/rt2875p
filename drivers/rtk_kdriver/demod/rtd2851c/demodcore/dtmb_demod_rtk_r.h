#ifndef __DEMOD_REALTEK_R_DTMB_H
#define __DEMOD_REALTEK_R_DTMB_H

/**

@file

@brief   REALTEK_R DTMB demod module declaration

One can manipulate REALTEK_R DTMB demod through REALTEK_R DTMB module.
REALTEK_R DTMB module is derived from DTMB demod module.



@par Example:
@code

// The example is the same as the DTMB demod example in dtmb_demod_rtk_r_base.h except the listed lines.



#include "demod_rtk_dtmb.h"


...



int main(void)
{
	DTMB_DEMOD_MODULE *pDemod;

	DTMB_DEMOD_MODULE     DtmbDemodModuleMemory;
	BASE_INTERFACE_MODULE BaseInterfaceModuleMemory;
	I2C_BRIDGE_MODULE     I2cBridgeModuleMemory;


	...



	// Build REALTEK_R DTMB demod module.
	BuildRealtekRDtmbModule(
		&pDemod,
		&DtmbDemodModuleMemory,
		&BaseInterfaceModuleMemory,
		&I2cBridgeModuleMemory,
		0x24,								// I2C device address is 0x24 in 8-bit format.
		CRYSTAL_FREQ_27000000HZ,			// Crystal frequency is 27.0 MHz.
		TS_INTERFACE_SERIAL,				// TS interface mode is serial.
		);



	// See the example for other DTMB demod functions in dtmb_demod_rtk_r_base.h

	...


	return 0;
}


@endcode

*/


#include "dtmb_demod_rtk_r_base.h"




//#define DTMB_R_MEM_START_PHYS 0x02000000

// Definitions

// Firmware code
#define REALTEK_R_DTMB_FIRMWARE_CODE_TABLE_LEN		4096


// Initializing
#define REALTEK_R_DTMB_INIT_REG_TABLE_LEN				29
#define REALTEK_R_DTMB_TS_INTERFACE_INIT_TABLE_LEN		3
//#define REALTEK_R_DTMB_DEMUX_DDR_SIZE					0x800000	// 8MB


// Chip ID
#define REALTEK_R_DTMB_CHIP_ID_VALUE		0x4


// IF frequency setting
#define REALTEK_R_DTMB_ADC_FREQ_HZ		48000000
#define REALTEK_R_DTMB_IFFREQ_BIT_NUM	10


// BER
#define REALTEK_R_DTMB_BER_DEN_VALUE		99876864


// PER
#define REALTEK_R_DTMB_PER_DEN_VALUE		32768


// SNR
#define REALTEK_R_DTMB_EST_SNR_BIT_NUM		9
#define REALTEK_R_DTMB_SNR_DB_DEN_VALUE		4


// AGC
#define REALTEK_R_DTMB_RF_AGC_REG_BIT_NUM		14
#define REALTEK_R_DTMB_IF_AGC_REG_BIT_NUM		14


// TR offset and CR offset
#define REALTEK_R_DTMB_TR_OUT_R_BIT_NUM			17
#define REALTEK_R_DTMB_SFOAQ_OUT_R_BIT_NUM		14
#define REALTEK_R_DTMB_CFO_EST_R_BIT_NUM			23


// Register table length
//Rev.A
#define REALTEK_R_DTMB_REG_TABLE_LEN				54
//Rev.B
#define REALTEK_R_DTMB_REG_TABLE_LEN				54//49


// Demod module builder
void
BuildRealtekRDtmbModule(
	DTMB_DEMOD_MODULE **ppDemod,
	DTMB_DEMOD_MODULE *pDtmbDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	U32BITS CrystalFreqHz,
	U32BITS ShareMemoryOfst
);





// Manipulating functions
void
realtek_r_dtmb_IsConnectedToI2c(
	DTMB_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dtmb_SoftwareReset(
	DTMB_DEMOD_MODULE *pDemod
);

int
realtek_r_dtmb_Initialize(
	DTMB_DEMOD_MODULE *pDemod
);

int
realtek_r_dtmb_DeInitialize(
	DTMB_DEMOD_MODULE *pDemod
);

int
realtek_r_dtmb_Bisr(
	DTMB_DEMOD_MODULE *pDemod
);

int
realtek_r_dtmb_SetIfFreqHz(
	DTMB_DEMOD_MODULE *pDemod,
	U32BITS IfFreqHz
);

int
realtek_r_dtmb_SetSpectrumMode(
	DTMB_DEMOD_MODULE *pDemod,
	int SpectrumMode
);

int
realtek_r_dtmb_IsSignalLocked(
	DTMB_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dtmb_IsTpsLocked(
	DTMB_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dtmb_IsInnerLocked(
	DTMB_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dtmb_IsNoSig(
	DTMB_DEMOD_MODULE *pDemod,
	int *pAnswer
);

int
realtek_r_dtmb_GetSignalStrength(
	DTMB_DEMOD_MODULE *pDemod,
	U32BITS *pSignalStrength
);

int
realtek_r_dtmb_GetSignalQuality(
	DTMB_DEMOD_MODULE *pDemod,
	U32BITS *pSignalQuality
);

int
realtek_r_dtmb_GetDemodStatus(
	DTMB_DEMOD_MODULE *pDemod,
	U32BITS *pDemodStatus
);

int
realtek_r_dtmb_GetWrapperStatus(
	DTMB_DEMOD_MODULE *pDemod,
	U32BITS *pWrapperStatus
);

int
realtek_r_dtmb_GetBer(
	DTMB_DEMOD_MODULE *pDemod,
	U32BITS *pBerNum,
	U32BITS *pBerDen
);

int
realtek_r_dtmb_GetIter(
	DTMB_DEMOD_MODULE *pDemod,
	U32BITS *pIterNum
);

int
realtek_r_dtmb_GetPer(
	DTMB_DEMOD_MODULE *pDemod,
	U32BITS *pPerNum,
	U32BITS *pPerDen
);

int
realtek_r_dtmb_GetSnrDb(
	DTMB_DEMOD_MODULE *pDemod,
	S32BITS *pSnrDbNum,
	S32BITS *pSnrDbDen
);

int
realtek_r_dtmb_GetRfAgc(
	DTMB_DEMOD_MODULE *pDemod,
	S32BITS *pRfAgc
);

int
realtek_r_dtmb_GetIfAgc(
	DTMB_DEMOD_MODULE *pDemod,
	S32BITS *pIfAgc
);

int
realtek_r_dtmb_GetDiAgc(
	DTMB_DEMOD_MODULE *pDemod,
	U32BITS *pDiAgc
);

int
realtek_r_dtmb_GetTrOffsetPpm(
	DTMB_DEMOD_MODULE *pDemod,
	S32BITS *pTrOffsetPpm
);

int
realtek_r_dtmb_GetCrOffsetHz(
	DTMB_DEMOD_MODULE *pDemod,
	S32BITS *pCrOffsetHz
);

int
realtek_r_dtmb_GetCarrierMode(
	DTMB_DEMOD_MODULE *pDemod,
	int *pCarrierMode
);

int
realtek_r_dtmb_GetPnMode(
	DTMB_DEMOD_MODULE *pDemod,
	int *pPnMode
);

int
realtek_r_dtmb_GetQamMode(
	DTMB_DEMOD_MODULE *pDemod,
	int *pQamMode
);

int
realtek_r_dtmb_GetCodeRateMode(
	DTMB_DEMOD_MODULE *pDemod,
	int *pCodeRateMode
);

int
realtek_r_dtmb_GetTimeInterleaverMode(
	DTMB_DEMOD_MODULE *pDemod,
	int *pTimeInterleaverMode
);

int
realtek_r_dtmb_GetFsmStage(
	DTMB_DEMOD_MODULE *pDemod,
	int *fsm_ans
);

int
realtek_r_dtmb_UpdateFunction(
	DTMB_DEMOD_MODULE *pDemod
);

int
realtek_r_dtmb_ResetFunction(
	DTMB_DEMOD_MODULE *pDemod
);





// I2C command forwarding functions
int
realtek_r_dtmb_ForwardI2cReadingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	unsigned char *pReadingBytes,
	unsigned long ByteNum
);

int
realtek_r_dtmb_ForwardI2cWritingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	const unsigned char *pWritingBytes,
	unsigned long ByteNum
);


int
realtek_r_dtmb_SetBandwidthMode(
	DTMB_DEMOD_MODULE *pDemod,
	int BandwidthMode
);


// Register table initializing
void
realtek_r_dtmb_InitRegTable(
	DTMB_DEMOD_MODULE *pDemod
);





// I2C birdge module builder
void
realtek_r_dtmb_BuildI2cBridgeModule(
	DTMB_DEMOD_MODULE *pDemod
);





// Firmware code downloading
int
realtek_r_dtmb_DownloadFirmwareCode(
	DTMB_DEMOD_MODULE *pDemod
);

// Firmware code table
extern unsigned char realtek_r_dtmb_FirewareCodeTable[REALTEK_R_DTMB_FIRMWARE_CODE_TABLE_LEN];





#endif
