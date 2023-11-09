#ifndef __ISDBT_DEMOD_BASE_H
#define __ISDBT_DEMOD_BASE_H

/**

@file

@brief   ISDB-T demod base module definition

ISDB-T demod base module definitions contains demod module structure, demod funciton pointers, and demod definitions.



@par Example:
@code


#include "demod_xxx.h"



int
CustomI2cRead(
	BASE_INTERFACE_MODULE *pBaseInterface,
	unsigned char DeviceAddr,
	unsigned char *pReadingBytes,
	U32BITS ByteNum
	)
{
	// I2C reading format:
	// start_bit + (DeviceAddr | reading_bit) + reading_byte * ByteNum + stop_bit

	...

	return FUNCTION_SUCCESS;

error_status:
	return FUNCTION_ERROR;
}



int
CustomI2cWrite(
	BASE_INTERFACE_MODULE *pBaseInterface,
	unsigned char DeviceAddr,
	const unsigned char *pWritingBytes,
	U32BITS ByteNum
	)
{
	// I2C writing format:
	// start_bit + (DeviceAddr | writing_bit) + writing_byte * ByteNum + stop_bit

	...

	return FUNCTION_SUCCESS;

error_status:
	return FUNCTION_ERROR;
}



void
CustomWaitMs(
	BASE_INTERFACE_MODULE *pBaseInterface,
	U32BITS WaitTimeMs
	)
{
	// Wait WaitTimeMs milliseconds.

	...

	return;
}



int main(void)
{
	BASE_INTERFACE_MODULE *pBaseInterface;
	BASE_INTERFACE_MODULE BaseInterfaceModuleMemory;

	ISDBT_DEMOD_MODULE *pDemod;
	ISDBT_DEMOD_MODULE IsdbtDemodModuleMemory;

	I2C_BRIDGE_MODULE I2cBridgeModuleMemory;

	int BandwidthMode;
	unsigned char SegmentNum;
	unsigned char SubChannelNum;

	int DemodType;
	unsigned char DeviceAddr;
	U32BITS CrystalFreqHz;

	S32BITS RfAgc, IfAgc, DiAgc;

	int Answer;

	S32BITS TrOffsetPpm, CrOffsetHz;

	U32BITS LayerBerNum, LayerBerDen;
	double LayerBer;
	U32BITS LayerPerNum, LayerPerDen;
	double LayerPer;

	S32BITS SnrDbNum, SnrDbDen;
	double SnrDb;

	U32BITS SignalStrength;
	U32BITS LayerSignalQuality;

	int LayerConstellation;
	int LayerCodeRate;
	int LayerInterleavingLen;
	unsigned char LayerSegmentNum;

	int GuardInterval;
	int FftMode;



	// Build base interface module.
	BuildBaseInterface(
		&pBaseInterface,
		&BaseInterfaceModuleMemory,
		9,								// Set maximum I2C reading byte number with 9.
		8,								// Set maximum I2C writing byte number with 8.
		CustomI2cRead,					// Employ CustomI2cRead() as basic I2C reading function.
		CustomI2cWrite,					// Employ CustomI2cWrite() as basic I2C writing function.
		CustomWaitMs					// Employ CustomWaitMs() as basic waiting function.
		);


	// Build ISDB-T demod XXX module.
	BuildXxxModule(
		&pDemod,
		&IsdbtDemodModuleMemory,
		&BaseInterfaceModuleMemory,
		&I2cBridgeModuleMemory,
		0x24,							// Demod I2C device address is 0x24 in 8-bit format.
		CRYSTAL_FREQ_27000000HZ,		// Demod crystal frequency is 27.0 MHz.
		TS_INTERFACE_SERIAL,			// Demod TS interface mode is serial.
		IF_FREQ_36000000HZ,				// IF frequency is 36.0 Mhz.
		SPECTRUM_INVERSE				// Spectrum mode is inverse.
		...								// Other arguments by each demod module
		);





	// ==== Initialize ISDB-T demod and set its parameters =====

	// Initialize demod.
	pDemod->Initialize(pDemod);


	// Set demod parameters. (bandwidth mode, segment number, sub-channel number)
	// Note: In the example:
	//       1. Bandwidth mode is 8 MHz.
	//       2. Segment number is 13.
	//       3. Sub-channel number is 22.
	BandwidthMode = ISDBT_BANDWIDTH_8MHZ;
	SegmentNum    = ISDBT_SEGMENT_NUM_13;
	SubChannelNum = 22;

	pDemod->SetParameters(pDemod, BandwidthMode, SegmentNum, SubChannelNum);


	// Need to set tuner before demod FSM restart.
	// The order to set demod and tuner is not important.
	// Note: 1. We can use "pDemod->SetRegBits(pDemod, ISDBT_I2CT_EN_CTRL, 0x1);"
	//          to enable tuner I2C command forwarding.
	//       2. We can use "pDemod->SetRegBits(pDemod, ISDBT_I2CT_EN_CTRL, 0x0);"
	//          to disable tuner I2C command forwarding.


	// Restart demod FSM.
	pDemod->RestartFsm(pDemod);


	// Wait maximum 1000 ms for demod converge.
	for(i = 0; i < 25; i++)
	{
		// Wait 40 ms.
		pBaseInterface->WaitMs(pBaseInterface, 40);

		// Check signal lock status.
		// Note: If Answer is YES, signal is locked.
		//       If Answer is NO, signal is not locked.
		pDemod->IsSignalLocked(pDemod, &Answer);

		if(Answer == YES)
		{
			// Signal is locked.
			break;
		}
	}





	// ==== Get ISDB-T demod information =====

	// Get demod type.
	// Note: One can find demod type in MODULE_TYPE enumeration.
	pDemod->GetDemodType(pDemod, &DemodType);

	// Get demod I2C device address.
	pDemod->GetDeviceAddr(pDemod, &DeviceAddr);

	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);


	// Ask demod if it is connected to I2C bus.
	// Note: If Answer is YES, demod is connected to I2C bus.
	//       If Answer is NO, demod is not connected to I2C bus.
	pDemod->IsConnectedToI2c(pDemod, &Answer);


	// Get demod parameters. (bandwidth mode, segment number, sub-channel number)
	pDemod->GetBandwidthMode(pDemod, &BandwidthMode);
	pDemod->GetSegmentNum(pDemod,    &SegmentNum);
	pDemod->GetSubChannelNum(pDemod, &SubChannelNum);


	// Get demod AGC value.
	// Note: The range of RF AGC and IF AGC value is -8192 ~ 8191.
	//       The range of digital AGC value is 0 ~ 4095.
	pDemod->GetRfAgc(pDemod, &RfAgc);
	pDemod->GetIfAgc(pDemod, &IfAgc);
	pDemod->GetDiAgc(pDemod, &DiAgc);


	// Get demod lock status.
	// Note: If Answer is YES, it is locked.
	//       If Answer is NO, it is not locked.
	pDemod->IsSignalLocked(pDemod, &Answer);


	// Get TR offset (symbol timing offset) in ppm.
	pDemod->GetTrOffsetPpm(pDemod, &TrOffsetPpm);

	// Get CR offset (RF frequency offset) in Hz.
	pDemod->GetCrOffsetHz(pDemod, &CrOffsetHz);


	// Get layer BER.
	// Note: Use ISDBT_HIERARCHICAL_LAYER_A, ISDBT_HIERARCHICAL_LAYER_B, ISDBT_HIERARCHICAL_LAYER_C
	//       for hierarchical layer index.
	pDemod->GetLayerBer(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &LayerBerNum, &LayerBerDen);
	LayerBer = (double)LayerBerNum / (double)LayerBerDen;

	// Get layer PER.
	// Note: Use ISDBT_HIERARCHICAL_LAYER_A, ISDBT_HIERARCHICAL_LAYER_B, ISDBT_HIERARCHICAL_LAYER_C
	//       for hierarchical layer index.
	pDemod->GetLayerPer(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &LayerPerNum, &LayerPerDen);
	LayerPer = (double)LayerPerNum / (double)LayerPerDen;

	// Get SNR in dB.
	pDemod->GetSnrDb(pDemod, &SnrDbNum, &SnrDbDen);
	SnrDb = (double)SnrDbNum / (double)SnrDbDen;


	// Get signal strength.
	// Note: 1. The range of SignalStrength is 0~100.
	//       2. Need to map SignalStrength value to UI signal strength bar manually.
	pDemod->GetSignalStrength(pDemod, &SignalStrength);

	// Get layer signal quality.
	// Note: 1. The range of SignalQuality is 0~100.
	//       2. Need to map SignalQuality value to UI signal quality bar manually.
	//       3. Use ISDBT_HIERARCHICAL_LAYER_A, ISDBT_HIERARCHICAL_LAYER_B, ISDBT_HIERARCHICAL_LAYER_C
	//          for hierarchical layer index.
	pDemod->GetLayerSignalQuality(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &LayerSignalQuality);


	// Get TMCC information.
	// Note: 1. One can find TMCC information definitions in the enumerations as follows:
	//          (1) ISDBT_CONSTELLATION_MODE
	//          (2) ISDBT_CODE_RATE_MODE
	//          (3) ISDBT_INTERLEAVING_LEN_MODE
	//          (4) ISDBT_SEGMENT_NUM
	//          (5) ISDBT_GUARD_INTERVAL_MODE
	//          (6) ISDBT_FFT_MODE_MODE
	//       2. Use ISDBT_HIERARCHICAL_LAYER_A, ISDBT_HIERARCHICAL_LAYER_B, ISDBT_HIERARCHICAL_LAYER_C
	//          for hierarchical layer index.

	pDemod->GetLayerConstellation(pDemod,   ISDBT_HIERARCHICAL_LAYER_A, &LayerConstellation);
	pDemod->GetLayerCodeRate(pDemod,        ISDBT_HIERARCHICAL_LAYER_A, &LayerCodeRate);
	pDemod->GetLayerInterleavingLen(pDemod, ISDBT_HIERARCHICAL_LAYER_A, &LayerInterleavingLen);
	pDemod->GetLayerSegmentNum(pDemod,      ISDBT_HIERARCHICAL_LAYER_A, &LayerSegmentNum);

	pDemod->GetGuardInterval(pDemod, &GuardInterval);
	pDemod->GetFftMode(pDemod,       &FftMode);



	return 0;
}


@endcode

*/


#include "foundation.h"





// Definitions

// Bandwidth modes
#define ISDBT_BANDWIDTH_NONE   -1
enum ISDBT_BANDWIDTH_MODE {
	ISDBT_BANDWIDTH_6MHZ,
	ISDBT_BANDWIDTH_7MHZ,
	ISDBT_BANDWIDTH_8MHZ,
};
#define ISDBT_BANDWIDTH_MODE_NUM   3


// Hierarchical layer
enum ISDBT_HIERARCHICAL_LAYER_INDEX {
	ISDBT_HIERARCHICAL_LAYER_A,
	ISDBT_HIERARCHICAL_LAYER_B,
	ISDBT_HIERARCHICAL_LAYER_C,
};
#define ISDBT_HIERARCHICAL_LAYER_INDEX_NUM   3


// Constellation
enum ISDBT_CONSTELLATION_MODE {
	ISDBT_CONSTELLATION_DQPSK,
	ISDBT_CONSTELLATION_QPSK,
	ISDBT_CONSTELLATION_16QAM,
	ISDBT_CONSTELLATION_64QAM,
};
#define ISDBT_CONSTELLATION_MODE_NUM   4


// Code rate
enum ISDBT_CODE_RATE_MODE {
	ISDBT_CODE_RATE_1_OVER_2,
	ISDBT_CODE_RATE_2_OVER_3,
	ISDBT_CODE_RATE_3_OVER_4,
	ISDBT_CODE_RATE_5_OVER_6,
	ISDBT_CODE_RATE_7_OVER_8,
};
#define ISDBT_CODE_RATE_MODE_NUM   5


// Interleaving length
enum ISDBT_INTERLEAVING_LEN_MODE {
	ISDBT_INTERLEAVING_LEN_0,
	ISDBT_INTERLEAVING_LEN_1,
	ISDBT_INTERLEAVING_LEN_2,
	ISDBT_INTERLEAVING_LEN_4,
	ISDBT_INTERLEAVING_LEN_8,
	ISDBT_INTERLEAVING_LEN_16,
};
#define ISDBT_INTERLEAVING_LEN_MODE_NUM   6


// Segment number
enum ISDBT_SEGMENT_NUM {
	ISDBT_SEGMENT_NUM_1  = 1,
	ISDBT_SEGMENT_NUM_3  = 3,
	ISDBT_SEGMENT_NUM_13 = 13,
};


// Guard interval
enum ISDBT_GUARD_INTERVAL_MODE {
	ISDBT_GUARD_INTERVAL_1_OVER_32,
	ISDBT_GUARD_INTERVAL_1_OVER_16,
	ISDBT_GUARD_INTERVAL_1_OVER_8,
	ISDBT_GUARD_INTERVAL_1_OVER_4,
};
#define ISDBT_GUARD_INTERVAL_MODE_NUM   4


// FFT mode
enum ISDBT_FFT_MODE_MODE {
	ISDBT_FFT_MODE_2K,
	ISDBT_FFT_MODE_4K,
	ISDBT_FFT_MODE_8K,
};
#define ISDBT_FFT_MODE_NUM   3





// Register entry definitions

// Register entry
typedef struct {
	int IsAvailable;
	unsigned short RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;
}
ISDBT_REG_ENTRY;



// Primary register entry
typedef struct {
	int RegBitName;
	unsigned short RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;
}
ISDBT_PRIMARY_REG_ENTRY;





// Register table dependence

// Demod register bit names
enum ISDBT_REG_BIT_NAME {
	// System control
	ISDBT_DEMOD_SEL,

	// I2C repeater control
	ISDBT_I2CT_EN_CTRL,

	// Auxiliary channel status
	ISDBT_AC_DECODE_OK,
	ISDBT_AC_CRC_OK,
	ISDBT_AC_MSG_INT_FLAG,
	ISDBT_AC_LOCK,
	ISDBT_AC_MSG_WD0,
	ISDBT_AC_MSG_DW1,
	ISDBT_AC_MSG_DW2,
	ISDBT_AC_MSG_DW3,
	ISDBT_AC_MSG_DW4,
	ISDBT_AC_MSG_DW5,
	ISDBT_AC_MSG_END,
	ISDBT_AC1_DATA_SEGA,
	ISDBT_AC2_DATA_SEGA,
	ISDBT_AC1_DATA_SEGB,
	ISDBT_AC2_DATA_SEGB,
	ISDBT_AC_OFDM_NUM,

	// AGC status
	ISDBT_RF_AGC_VAL,
	ISDBT_IF_AGC_VAL,
	ISDBT_AAGC_LOCK,
	ISDBT_GAIN_OUT_R,

	// Signal parameter status
	ISDBT_FFT_MODE_R,
	ISDBT_GI_MODE_R,
	ISDBT_RX_QAM0,
	ISDBT_RX_QAM1,
	ISDBT_RX_QAM2,
	ISDBT_SYSTEM_ID,
	ISDBT_EMERGENCY_FLAG,
	ISDBT_PARTIAL_FLAG,
	ISDBT_RX_CRATE0,
	ISDBT_RX_CRATE1,
	ISDBT_RX_CRATE2,
	ISDBT_RX_INTLV0,
	ISDBT_RX_INTLV1,
	ISDBT_RX_INTLV2,
	ISDBT_RX_NO_SEG0,
	ISDBT_RX_NO_SEG1,
	ISDBT_RX_NO_SEG2,

	// Demod status
	ISDBT_CFO_EST,
	ISDBT_SFO_OUT,
	ISDBT_FSM_STATE_R,
	ISDBT_EST_SNR,
	ISDBT_SIG_PRESENT,
	ISDBT_FRM_LOCK,
	ISDBT_VBER_0_VALID,
	ISDBT_VBER_0,
	ISDBT_VBER_1_VALID,
	ISDBT_VBER_1,
	ISDBT_VBER_2_VALID,
	ISDBT_VBER_2,
	ISDBT_RSBER_0_VALID,
	ISDBT_RSBER_0,
	ISDBT_RSBER_1_VALID,
	ISDBT_RSBER_1,
	ISDBT_RSBER_2_VALID,
	ISDBT_RSBER_2,
	ISDBT_PREFRM_LOCK_POST,
	ISDBT_PREFRM_EST_DBPSK_SNR,
	ISDBT_DPBSK_Z_AVG_DBG,
	ISDBT_DPBSK_N_AVG_DBG,
	// Demod control
	ISDBT_SOFT_RST_N,

	// AGC control
	ISDBT_RFAGC_MAX,
	ISDBT_RFAGC_MIN,
	ISDBT_IFAGC_MAX,
	ISDBT_IFAGC_MIN,
	ISDBT_AAGC_HOLD,
	ISDBT_PSET_IFAGC,
	ISDBT_PSET_RFAGC,
	ISDBT_PSET_IFAGC_VAL,
	ISDBT_PSET_RFAGC_VAL,
	ISDBT_POLAR_IFAGC,
	ISDBT_POLAR_RFAGC,
	ISDBT_EN_IFAGC,
	ISDBT_EN_RFAGC,
	ISDBT_LOOPGAIN1,
	ISDBT_Z_AGC,

    //Demod AAGC target VAL
    ISDBT_AAGC_TARGET_VAL,

	// Down conversion control
	ISDBT_EN_SP_INV,
	ISDBT_IFFREQ,

	// Auxiliary channel control
	ISDBT_SEG0_AC_DEC_SRC,
	ISDBT_SEGA_AC_DEC_SRC,
	ISDBT_SEGA_NUM,
	ISDBT_SEGB_AC_DEC_SRC,
	ISDBT_SEGB_NUM,
	ISDBT_IT_AC_OUT_FRZ,

	// Outer control
	ISDBT_LAYER_SEL,

	// MPEG interface control
	ISDBT_CKOUTPAR,
	ISDBT_SYNC_DUR,
	ISDBT_ERR_DUR,
	ISDBT_SERIAL,
	ISDBT_SER_LSB,
	ISDBT_CDIV_PH0,
	ISDBT_CDIV_PH1,
	ISDBT_FIX_TEI,

	// Private
	ISDBT_PRIVATE_0,
	ISDBT_DDR_ADDR_OFFSET,

    //rsper block average size
    ISDBT_RSPER0_BAVG, 
    ISDBT_RSPER1_BAVG, 
    ISDBT_RSPER2_BAVG,

	// PreFrm lock control
	ISDBT_IT_TMCC_PREFRM_POST_EN,
	ISDBT_IT_TMCC_PREFRM_SNR_POST,

        // Item terminator
	ISDBT_REG_BIT_NAME_ITEM_TERMINATOR,
};



// Register table length definitions
#define ISDBT_REG_TABLE_LEN_MAX			ISDBT_REG_BIT_NAME_ITEM_TERMINATOR





// ISDB-T demod module pre-definition
typedef struct ISDBT_DEMOD_MODULE_TAG ISDBT_DEMOD_MODULE;





/**

@brief   ISDB-T demod register byte setting function pointer

Demod upper level functions will use ISDBT_DEMOD_FP_SET_REG_BYTES() to set demod register bytes.


@param [in]   pDemod          The demod module pointer
@param [in]   RegStartAddr    Demod register start address
@param [in]   pWritingBytes   Pointer to writing bytes
@param [in]   ByteNum         Writing byte number


@retval   FUNCTION_SUCCESS   Set demod register bytes successfully with writing bytes.
@retval   FUNCTION_ERROR     Set demod register bytes unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_SET_REG_BYTES() with the corresponding function.
	-# Need to set register page by ISDBT_DEMOD_FP_SET_REG_PAGE() before using ISDBT_DEMOD_FP_SET_REG_BYTES().


@see   ISDBT_DEMOD_FP_SET_REG_PAGE, ISDBT_DEMOD_FP_GET_REG_BYTES



@par Example:
@code


#include "demod_pseudo.h"


int main(void)
{
	ISDBT_DEMOD_MODULE *pDemod;
	ISDBT_DEMOD_MODULE IsdbtDemodModuleMemory;
	unsigned char WritingBytes[10];


	// Build pseudo demod module.
	BuildPseudoDemodModule(&pDemod, &IsdbtDemodModuleMemory);

	...

	// Set demod register bytes (address 0x17 ~ 0x1b) with 5 writing bytes.
	pDemod->SetRegBytes(pDemod, 0x17, WritingBytes, 5);

	...

	return 0;
}


@endcode

*/
typedef int
(*ISDBT_DEMOD_FP_SET_REG_BYTES)(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	const unsigned char *pWritingBytes,
	unsigned char ByteNum
);





/**

@brief   ISDB-T demod register byte getting function pointer

Demod upper level functions will use ISDBT_DEMOD_FP_GET_REG_BYTES() to get demod register bytes.


@param [in]    pDemod          The demod module pointer
@param [in]    RegStartAddr    Demod register start address
@param [out]   pReadingBytes   Pointer to an allocated memory for storing reading bytes
@param [in]    ByteNum         Reading byte number


@retval   FUNCTION_SUCCESS   Get demod register bytes successfully with reading byte number.
@retval   FUNCTION_ERROR     Get demod register bytes unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_REG_BYTES() with the corresponding function.
	-# Need to set register page by ISDBT_DEMOD_FP_SET_REG_PAGE() before using ISDBT_DEMOD_FP_GET_REG_BYTES().


@see   ISDBT_DEMOD_FP_SET_REG_PAGE, ISDBT_DEMOD_FP_SET_REG_BYTES



@par Example:
@code


#include "demod_pseudo.h"


int main(void)
{
	ISDBT_DEMOD_MODULE *pDemod;
	ISDBT_DEMOD_MODULE IsdbtDemodModuleMemory;
	unsigned char ReadingBytes[10];


	// Build pseudo demod module.
	BuildPseudoDemodModule(&pDemod, &IsdbtDemodModuleMemory);

	...

	// Get demod register bytes (page 1, address 0x17 ~ 0x1b) with reading byte number 5.
	pDemod->SetRegPage(pDemod, 1);
	pDemod->GetRegBytes(pDemod, 0x17, ReadingBytes, 5);

	...

	return 0;
}


@endcode

*/
typedef int
(*ISDBT_DEMOD_FP_GET_REG_BYTES)(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char *pReadingBytes,
	unsigned char ByteNum
);





/**

@brief   ISDB-T demod register mask bits setting function pointer

Demod upper level functions will use ISDBT_DEMOD_FP_SET_REG_MASK_BITS() to set demod register mask bits.


@param [in]   pDemod         The demod module pointer
@param [in]   RegStartAddr   Demod register start address
@param [in]   Msb            Mask MSB with 0-based index
@param [in]   Lsb            Mask LSB with 0-based index
@param [in]   WritingValue   The mask bits writing value


@retval   FUNCTION_SUCCESS   Set demod register mask bits successfully with writing value.
@retval   FUNCTION_ERROR     Set demod register mask bits unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_SET_REG_MASK_BITS() with the corresponding function.
	-# Need to set register page by ISDBT_DEMOD_FP_SET_REG_PAGE() before using ISDBT_DEMOD_FP_SET_REG_MASK_BITS().
	-# The constraints of ISDBT_DEMOD_FP_SET_REG_MASK_BITS() function usage are described as follows:
		-# The mask MSB and LSB must be 0~31.
		-# The mask MSB must be greater than or equal to LSB.


@see   ISDBT_DEMOD_FP_SET_REG_PAGE, ISDBT_DEMOD_FP_GET_REG_MASK_BITS



@par Example:
@code


#include "demod_pseudo.h"


int main(void)
{
	ISDBT_DEMOD_MODULE *pDemod;
	ISDBT_DEMOD_MODULE IsdbtDemodModuleMemory;


	// Build pseudo demod module.
	BuildPseudoDemodModule(&pDemod, &IsdbtDemodModuleMemory);

	...

	// Set demod register bits (page 1, address {0x18, 0x17} [12:5]) with writing value 0x1d.
	pDemod->SetRegPage(pDemod, 1);
	pDemod->SetRegMaskBits(pDemod, 0x17, 12, 5, 0x1d);


	// Result:
	//
	// Writing value = 0x1d = 0001 1101 b
	//
	// Page 1
	// Register address   0x17          0x18
	// Register value     xxx0 0011 b   101x xxxx b

	...

	return 0;
}


@endcode

*/
typedef int
(*ISDBT_DEMOD_FP_SET_REG_MASK_BITS)(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	const U32BITS WritingValue
);





/**

@brief   ISDB-T demod register mask bits getting function pointer

Demod upper level functions will use ISDBT_DEMOD_FP_GET_REG_MASK_BITS() to get demod register mask bits.


@param [in]    pDemod          The demod module pointer
@param [in]    RegStartAddr    Demod register start address
@param [in]    Msb             Mask MSB with 0-based index
@param [in]    Lsb             Mask LSB with 0-based index
@param [out]   pReadingValue   Pointer to an allocated memory for storing reading value


@retval   FUNCTION_SUCCESS   Get demod register mask bits successfully.
@retval   FUNCTION_ERROR     Get demod register mask bits unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_REG_MASK_BITS() with the corresponding function.
	-# Need to set register page by ISDBT_DEMOD_FP_SET_REG_PAGE() before using ISDBT_DEMOD_FP_GET_REG_MASK_BITS().
	-# The constraints of ISDBT_DEMOD_FP_GET_REG_MASK_BITS() function usage are described as follows:
		-# The mask MSB and LSB must be 0~31.
		-# The mask MSB must be greater than or equal to LSB.


@see   ISDBT_DEMOD_FP_SET_REG_PAGE, ISDBT_DEMOD_FP_SET_REG_MASK_BITS



@par Example:
@code


#include "demod_pseudo.h"


int main(void)
{
	ISDBT_DEMOD_MODULE *pDemod;
	ISDBT_DEMOD_MODULE IsdbtDemodModuleMemory;
	U32BITS ReadingValue;


	// Build pseudo demod module.
	BuildPseudoDemodModule(&pDemod, &IsdbtDemodModuleMemory);

	...

	// Get demod register bits (page 1, address {0x18, 0x17} [12:5]).
	pDemod->SetRegPage(pDemod, 1);
	pDemod->GetRegMaskBits(pDemod, 0x17, 12, 5, &ReadingValue);


	// Result:
	//
	// Page 1
	// Register address   0x18          0x17
	// Register value     xxx0 0011 b   101x xxxx b
	//
	// Reading value = 0001 1101 b = 0x1d

	...

	return 0;
}


@endcode

*/
typedef int
(*ISDBT_DEMOD_FP_GET_REG_MASK_BITS)(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	U32BITS *pReadingValue
);





/**

@brief   ISDB-T demod register bits setting function pointer

Demod upper level functions will use ISDBT_DEMOD_FP_SET_REG_BITS() to set demod register bits with bit name.


@param [in]   pDemod         The demod module pointer
@param [in]   RegBitName     Pre-defined demod register bit name
@param [in]   WritingValue   The mask bits writing value


@retval   FUNCTION_SUCCESS   Set demod register bits successfully with bit name and writing value.
@retval   FUNCTION_ERROR     Set demod register bits unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_SET_REG_BITS() with the corresponding function.
	-# Need to set register page before using ISDBT_DEMOD_FP_SET_REG_BITS().
	-# Register bit names are pre-defined keys for bit access, and one can find these in demod header file.


@see   ISDBT_DEMOD_FP_SET_REG_PAGE, ISDBT_DEMOD_FP_GET_REG_BITS



@par Example:
@code


#include "demod_pseudo.h"


int main(void)
{
	ISDBT_DEMOD_MODULE *pDemod;
	ISDBT_DEMOD_MODULE IsdbtDemodModuleMemory;


	// Build pseudo demod module.
	BuildPseudoDemodModule(&pDemod, &IsdbtDemodModuleMemory);

	...

	// Set demod register bits with bit name PSEUDO_REG_BIT_NAME and writing value 0x1d.
	// The corresponding information of PSEUDO_REG_BIT_NAME is address {0x18, 0x17} [12:5] on page 1.
	pDemod->SetRegPage(pDemod, 1);
	pDemod->SetRegBits(pDemod, PSEUDO_REG_BIT_NAME, 0x1d);


	// Result:
	//
	// Writing value = 0x1d = 0001 1101 b
	//
	// Page 1
	// Register address   0x18          0x17
	// Register value     xxx0 0011 b   101x xxxx b

	...

	return 0;
}


@endcode

*/
typedef int
(*ISDBT_DEMOD_FP_SET_REG_BITS)(
	ISDBT_DEMOD_MODULE *pDemod,
	int RegBitName,
	const U32BITS WritingValue
);





/**

@brief   ISDB-T demod register bits getting function pointer

Demod upper level functions will use ISDBT_DEMOD_FP_GET_REG_BITS() to get demod register bits with bit name.


@param [in]    pDemod          The demod module pointer
@param [in]    RegBitName      Pre-defined demod register bit name
@param [out]   pReadingValue   Pointer to an allocated memory for storing reading value


@retval   FUNCTION_SUCCESS   Get demod register bits successfully with bit name.
@retval   FUNCTION_ERROR     Get demod register bits unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_REG_BITS() with the corresponding function.
	-# Need to set register page before using ISDBT_DEMOD_FP_GET_REG_BITS().
	-# Register bit names are pre-defined keys for bit access, and one can find these in demod header file.


@see   ISDBT_DEMOD_FP_SET_REG_PAGE, ISDBT_DEMOD_FP_SET_REG_BITS



@par Example:
@code


#include "demod_pseudo.h"


int main(void)
{
	ISDBT_DEMOD_MODULE *pDemod;
	ISDBT_DEMOD_MODULE IsdbtDemodModuleMemory;
	U32BITS ReadingValue;


	// Build pseudo demod module.
	BuildPseudoDemodModule(&pDemod, &IsdbtDemodModuleMemory);

	...

	// Get demod register bits with bit name PSEUDO_REG_BIT_NAME.
	// The corresponding information of PSEUDO_REG_BIT_NAME is address {0x18, 0x17} [12:5] on page 1.
	pDemod->SetRegPage(pDemod, 1);
	pDemod->GetRegBits(pDemod, PSEUDO_REG_BIT_NAME, &ReadingValue);


	// Result:
	//
	// Page 1
	// Register address   0x18          0x17
	// Register value     xxx0 0011 b   101x xxxx b
	//
	// Reading value = 0001 1101 b = 0x1d

	...

	return 0;
}


@endcode

*/
typedef int
(*ISDBT_DEMOD_FP_GET_REG_BITS)(
	ISDBT_DEMOD_MODULE *pDemod,
	int RegBitName,
	U32BITS *pReadingValue
);





/**

@brief   ISDB-T demod type getting function pointer

One can use ISDBT_DEMOD_FP_GET_DEMOD_TYPE() to get ISDB-T demod type.


@param [in]    pDemod       The demod module pointer
@param [out]   pDemodType   Pointer to an allocated memory for storing demod type


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_DEMOD_TYPE() with the corresponding function.


@see   MODULE_TYPE

*/
typedef void
(*ISDBT_DEMOD_FP_GET_DEMOD_TYPE)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pDemodType
);





/**

@brief   ISDB-T demod I2C device address getting function pointer

One can use ISDBT_DEMOD_FP_GET_DEVICE_ADDR() to get ISDB-T demod I2C device address.


@param [in]    pDemod        The demod module pointer
@param [out]   pDeviceAddr   Pointer to an allocated memory for storing demod I2C device address


@retval   FUNCTION_SUCCESS   Get demod device address successfully.
@retval   FUNCTION_ERROR     Get demod device address unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_DEVICE_ADDR() with the corresponding function.

*/
typedef void
(*ISDBT_DEMOD_FP_GET_DEVICE_ADDR)(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char *pDeviceAddr
);





/**

@brief   ISDB-T demod crystal frequency getting function pointer

One can use ISDBT_DEMOD_FP_GET_CRYSTAL_FREQ_HZ() to get ISDB-T demod crystal frequency in Hz.


@param [in]    pDemod           The demod module pointer
@param [out]   pCrystalFreqHz   Pointer to an allocated memory for storing demod crystal frequency in Hz


@retval   FUNCTION_SUCCESS   Get demod crystal frequency successfully.
@retval   FUNCTION_ERROR     Get demod crystal frequency unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_CRYSTAL_FREQ_HZ() with the corresponding function.

*/
typedef void
(*ISDBT_DEMOD_FP_GET_CRYSTAL_FREQ_HZ)(
	ISDBT_DEMOD_MODULE *pDemod,
	U32BITS *pCrystalFreqHz
);





/**

@brief   ISDB-T demod I2C bus connection asking function pointer

One can use ISDBT_DEMOD_FP_IS_CONNECTED_TO_I2C() to ask ISDB-T demod if it is connected to I2C bus.


@param [in]    pDemod    The demod module pointer
@param [out]   pAnswer   Pointer to an allocated memory for storing answer


@note
	-# Demod building function will set ISDBT_DEMOD_FP_IS_CONNECTED_TO_I2C() with the corresponding function.

*/
typedef void
(*ISDBT_DEMOD_FP_IS_CONNECTED_TO_I2C)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);





/**

@brief   ISDB-T demod FSM restarting function pointer

One can use ISDBT_DEMOD_FP_RESTART_FSM() to restart ISDB-T demod FSM.


@param [in]   pDemod   The demod module pointer


@retval   FUNCTION_SUCCESS   Restart ISDB-T demod FSM successfully.
@retval   FUNCTION_ERROR     Restart ISDB-T demod FSM unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_RESTART_FSM() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_RESTART_FSM)(
	ISDBT_DEMOD_MODULE *pDemod
);





/**

@brief   ISDB-T demod initializing function pointer

One can use ISDBT_DEMOD_FP_INITIALIZE() to initialie ISDB-T demod.


@param [in]   pDemod   The demod module pointer


@retval   FUNCTION_SUCCESS   Initialize demod successfully.
@retval   FUNCTION_ERROR     Initialize demod unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_INITIALIZE() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_INITIALIZE)(
	ISDBT_DEMOD_MODULE *pDemod
);





/**

@brief   ISDB-T demod TS interface mode setting function pointer

One can use ISDBT_DEMOD_FP_SET_TS_INTERFACE_MODE() to set ISDB-T demod TS interface mode.


@param [in]   pDemod            The demod module pointer
@param [in]   TsInterfaceMode   TS interface mode for setting


@retval   FUNCTION_SUCCESS   Set demod TS interface mode successfully.
@retval   FUNCTION_ERROR     Set demod TS interface mode unsuccessfully.


@note
	-# ISDBT_DEMOD_FP_SET_TS_INTERFACE_MODE() corresponding function is private.
	-# ISDBT_DEMOD_FP_SET_TS_INTERFACE_MODE() does not appear in demod module structure.


@see   TS_INTERFACE_MODE

*/
typedef int
(*ISDBT_DEMOD_FP_SET_TS_INTERFACE_MODE)(
	ISDBT_DEMOD_MODULE *pDemod,
	int TsInterfaceMode
);


typedef int
(*ISDBT_DEMOD_FP_SET_TS_INTERFACE_CLK_POL)(
	ISDBT_DEMOD_MODULE *pDemod,
	int TsInterfaceClkPol
);




/**

@brief   ISDB-T demod IF frequency setting function pointer

One can use ISDBT_DEMOD_FP_SET_IF_FREQ_HZ() to set ISDB-T demod IF frequency in Hz.


@param [in]   pDemod     The demod module pointer
@param [in]   IfFreqHz   IF frequency in Hz for setting


@retval   FUNCTION_SUCCESS   Set demod IF frequency successfully.
@retval   FUNCTION_ERROR     Set demod IF frequency unsuccessfully.


@note
	-# ISDBT_DEMOD_FP_SET_IF_FREQ_HZ() corresponding function is private.
	-# ISDBT_DEMOD_FP_SET_IF_FREQ_HZ() does not appear in demod module structure.


@see   IF_FREQ_HZ

*/
typedef int
(*ISDBT_DEMOD_FP_SET_IF_FREQ_HZ)(
	ISDBT_DEMOD_MODULE *pDemod,
	U32BITS IfFreqHz
);





/**

@brief   ISDB-T demod spectrum mode setting function pointer

One can use ISDBT_DEMOD_FP_SET_SPECTRUM_MODE() to set ISDB-T demod spectrum mode.


@param [in]   pDemod         The demod module pointer
@param [in]   SpectrumMode   Spectrum mode for setting


@retval   FUNCTION_SUCCESS   Set demod spectrum mode successfully.
@retval   FUNCTION_ERROR     Set demod spectrum mode unsuccessfully.


@note
	-# ISDBT_DEMOD_FP_SET_SPECTRUM_MODE() corresponding function is private.
	-# ISDBT_DEMOD_FP_SET_SPECTRUM_MODE() does not appear in demod module structure.


@see   SPECTRUM_MODE

*/
typedef int
(*ISDBT_DEMOD_FP_SET_SPECTRUM_MODE)(
	ISDBT_DEMOD_MODULE *pDemod,
	int SpectrumMode
);





/**

@brief   ISDB-T demod parameter setting function pointer

One can use ISDBT_DEMOD_FP_SET_PARAMETERS() to set ISDB-T demod parameters.


@param [in]   pDemod	      The demod module pointer
@param [in]   BandwidthMode   Bandwidth mode for setting
@param [in]   SegmentNum      Segment number for setting
@param [in]   SubChannelNum   Sub-channel number for setting


@retval   FUNCTION_SUCCESS   Set demod parameters successfully.
@retval   FUNCTION_ERROR     Set demod parameters unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_SET_PARAMETERS() with the corresponding function.


@see   ISDBT_BANDWIDTH_MODE

*/
typedef int
(*ISDBT_DEMOD_FP_SET_PARAMETERS)(
	ISDBT_DEMOD_MODULE *pDemod,
	int BandwidthMode,
	unsigned char SegmentNum,
	unsigned char SubChannelNum
);





/**

@brief   ISDB-T demod bandwidth mode getting function pointer

One can use ISDBT_DEMOD_FP_GET_BANDWIDTH_MODE() to get ISDB-T demod bandwidth mode.


@param [in]    pDemod           The demod module pointer
@param [out]   pBandwidthMode   Pointer to an allocated memory for storing demod bandwidth mode


@retval   FUNCTION_SUCCESS   Get demod bandwidth mode successfully.
@retval   FUNCTION_ERROR     Get demod bandwidth mode unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_BANDWIDTH_MODE() with the corresponding function.


@see   ISDBT_ISDBT_MODE

*/
typedef int
(*ISDBT_DEMOD_FP_GET_BANDWIDTH_MODE)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pBandwidthMode
);





/**

@brief   ISDB-T demod segment number getting function pointer

One can use ISDBT_DEMOD_FP_GET_SEGMENT_NUM() to get ISDB-T demod segment number.


@param [in]    pDemod        The demod module pointer
@param [out]   pSegmentNum   Pointer to an allocated memory for storing demod segment number


@retval   FUNCTION_SUCCESS   Get demod segment number successfully.
@retval   FUNCTION_ERROR     Get demod segment number unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_SEGMENT_NUM() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_SEGMENT_NUM)(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char *pSegmentNum
);





/**

@brief   ISDB-T demod sub-channel number getting function pointer

One can use ISDBT_DEMOD_FP_GET_SUB_CHANNEL_NUM() to get ISDB-T demod sub-channel number.


@param [in]    pDemod           The demod module pointer
@param [out]   pSubChannelNum   Pointer to an allocated memory for storing demod sub-channel number


@retval   FUNCTION_SUCCESS   Get demod sub-channel number successfully.
@retval   FUNCTION_ERROR     Get demod sub-channel number unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_SUB_CHANNEL_NUM() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_SUB_CHANNEL_NUM)(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char *pSubChannelNum
);





/**

@brief   ISDB-T demod signal lock asking function pointer

One can use ISDBT_DEMOD_FP_IS_SIGNAL_LOCKED() to ask ISDB-T demod if it is signal-locked.


@param [in]    pDemod    The demod module pointer
@param [out]   pAnswer   Pointer to an allocated memory for storing answer


@retval   FUNCTION_SUCCESS   Perform signal lock asking to demod successfully.
@retval   FUNCTION_ERROR     Perform signal lock asking to demod unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_IS_SIGNAL_LOCKED() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_IS_SIGNAL_LOCKED)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);


/**

@brief   ISDBT demod tr lock asking function pointer

One can use ISDBT_DEMOD_FP_IS_TR_LOCKED() to ask ISDBT demod if it is tr-locked.


@param [in]    pDemod    The demod module pointer
@param [out]   pAnswer   Pointer to an allocated memory for storing answer


@retval   FUNCTION_SUCCESS   Perform tr lock asking to demod successfully.
@retval   FUNCTION_ERROR     Perform tr lock asking to demod unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_IS_TR_LOCKED() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_IS_TR_LOCKED)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);


/**

@brief   ISDBT demod Cr lock asking function pointer

One can use ISDBT_DEMOD_FP_IS_CR_LOCKED() to ask ISDBT demod if it is cr-locked.


@param [in]    pDemod    The demod module pointer
@param [out]   pAnswer   Pointer to an allocated memory for storing answer


@retval   FUNCTION_SUCCESS   Perform cr lock asking to demod successfully.
@retval   FUNCTION_ERROR     Perform cr lock asking to demod unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_IS_CR_LOCKED() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_IS_CR_LOCKED)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);

/**

@brief   ISDBT demod inner lock asking function pointer

One can use ISDBT_DEMOD_FP_IS_INNER_LOCKED() to ask ISDBT demod if it is inner-locked.


@param [in]    pDemod    The demod module pointer
@param [out]   pAnswer   Pointer to an allocated memory for storing answer


@retval   FUNCTION_SUCCESS   Perform inner lock asking to demod successfully.
@retval   FUNCTION_ERROR     Perform inner lock asking to demod unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_IS_INNER_LOCKED() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_IS_INNER_LOCKED)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);

/**

@brief   ISDBT demod Nosig asking function pointer

One can use ISDBT_DEMOD_FP_IS_NO_SIG() to ask ISDBT demod if it is Nosig.


@param [in]    pDemod    The demod module pointer
@param [out]   pAnswer   Pointer to an allocated memory for storing answer


@retval   FUNCTION_SUCCESS   Perform no sig asking to demod successfully.
@retval   FUNCTION_ERROR     Perform no sig asking to demod unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_IS_INNER_LOCKED() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_IS_NO_SIG)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
);




/**

@brief   ISDB-T demod signal strength getting function pointer

One can use ISDBT_DEMOD_FP_GET_SIGNAL_STRENGTH() to get signal strength.


@param [in]    pDemod            The demod module pointer
@param [out]   pSignalStrength   Pointer to an allocated memory for storing signal strength (0 ~ 100)


@retval   FUNCTION_SUCCESS   Get demod signal strength successfully.
@retval   FUNCTION_ERROR     Get demod signal strength unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_SIGNAL_STRENGTH() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_SIGNAL_STRENGTH)(
	ISDBT_DEMOD_MODULE *pDemod,
	U32BITS *pSignalStrength
);





/**

@brief   ISDB-T demod layer signal quality getting function pointer

One can use ISDBT_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY() to get layer signal quality.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerSignalQuality      Pointer to an allocated memory for storing layer signal quality (0 ~ 100)


@retval   FUNCTION_SUCCESS   Get demod layer signal quality successfully.
@retval   FUNCTION_ERROR     Get demod layer signal quality unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY() with the corresponding function.


@see   ISDBT_HIERARCHICAL_LAYER_INDEX

*/
typedef int
(*ISDBT_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY)(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	U32BITS *pLayerSignalQuality
);





/**

@brief   ISDB-T demod layer BER getting function pointer

One can use ISDBT_DEMOD_FP_GET_LAYER_BER() to get layer BER.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerBerNum             Pointer to an allocated memory for storing layer BER numerator
@param [out]   pLayerBerDen             Pointer to an allocated memory for storing layer BER denominator


@retval   FUNCTION_SUCCESS   Get demod layer BER successfully.
@retval   FUNCTION_ERROR     Get demod layer BER unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_LAYER_BER() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_LAYER_BER)(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	U32BITS *pLayerBerNum,
	U32BITS *pLayerBerDen
);





/**

@brief   ISDB-T demod layer PER getting function pointer

One can use ISDBT_DEMOD_FP_GET_LAYER_PER() to get layer PER.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerPerNum             Pointer to an allocated memory for storing layer PER numerator
@param [out]   pLayerPerDen             Pointer to an allocated memory for storing layer PER denominator


@retval   FUNCTION_SUCCESS   Get demod layer PER successfully.
@retval   FUNCTION_ERROR     Get demod layer PER unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_LAYER_PER() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_LAYER_PER)(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	U32BITS *pLayerPerNum,
	U32BITS *pLayerPerDen
);





/**

@brief   ISDB-T demod SNR getting function pointer

One can use ISDBT_DEMOD_FP_GET_SNR_DB() to get SNR in dB.


@param [in]    pDemod      The demod module pointer
@param [out]   pSnrDbNum   Pointer to an allocated memory for storing SNR dB numerator
@param [out]   pSnrDbDen   Pointer to an allocated memory for storing SNR dB denominator


@retval   FUNCTION_SUCCESS   Get demod SNR successfully.
@retval   FUNCTION_ERROR     Get demod SNR unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_SNR_DB() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_SNR_DB)(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pSnrDbNum,
	S32BITS *pSnrDbDen
);





/**

@brief   ISDB-T demod RF AGC getting function pointer

One can use ISDBT_DEMOD_FP_GET_RF_AGC() to get ISDB-T demod RF AGC value.


@param [in]    pDemod   The demod module pointer
@param [out]   pRfAgc   Pointer to an allocated memory for storing RF AGC value


@retval   FUNCTION_SUCCESS   Get demod RF AGC value successfully.
@retval   FUNCTION_ERROR     Get demod RF AGC value unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_RF_AGC() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_RF_AGC)(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pRfAgc
);





/**

@brief   ISDB-T demod IF AGC getting function pointer

One can use ISDBT_DEMOD_FP_GET_IF_AGC() to get ISDB-T demod IF AGC value.


@param [in]    pDemod   The demod module pointer
@param [out]   pIfAgc   Pointer to an allocated memory for storing IF AGC value


@retval   FUNCTION_SUCCESS   Get demod IF AGC value successfully.
@retval   FUNCTION_ERROR     Get demod IF AGC value unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_IF_AGC() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_IF_AGC)(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pIfAgc
);





/**

@brief   ISDB-T demod digital AGC getting function pointer

One can use ISDBT_DEMOD_FP_GET_DI_AGC() to get ISDB-T demod digital AGC value.


@param [in]    pDemod   The demod module pointer
@param [out]   pDiAgc   Pointer to an allocated memory for storing digital AGC value


@retval   FUNCTION_SUCCESS   Get demod digital AGC value successfully.
@retval   FUNCTION_ERROR     Get demod digital AGC value unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_DI_AGC() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_DI_AGC)(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pDiAgc
);





/**

@brief   ISDB-T demod TR offset getting function pointer

One can use ISDBT_DEMOD_FP_GET_TR_OFFSET_PPM() to get TR offset in ppm.


@param [in]    pDemod         The demod module pointer
@param [out]   pTrOffsetPpm   Pointer to an allocated memory for storing TR offset in ppm


@retval   FUNCTION_SUCCESS   Get demod TR offset successfully.
@retval   FUNCTION_ERROR     Get demod TR offset unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_TR_OFFSET_PPM() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_TR_OFFSET_PPM)(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pTrOffsetPpm
);





/**

@brief   ISDB-T demod CR offset getting function pointer

One can use ISDBT_DEMOD_FP_GET_CR_OFFSET_HZ() to get CR offset in Hz.


@param [in]    pDemod        The demod module pointer
@param [out]   pCrOffsetHz   Pointer to an allocated memory for storing CR offset in Hz


@retval   FUNCTION_SUCCESS   Get demod CR offset successfully.
@retval   FUNCTION_ERROR     Get demod CR offset unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_CR_OFFSET_HZ() with the corresponding function.

*/
typedef int
(*ISDBT_DEMOD_FP_GET_CR_OFFSET_HZ)(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pCrOffsetHz
);





/**

@brief   ISDB-T demod layer constellation mode getting function pointer

One can use ISDBT_DEMOD_FP_GET_LAYER_CONSTELLATION() to get ISDB-T demod layer constellation mode according to
hierarchical layer index.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerConstellation      Pointer to an allocated memory for storing demod layer constellation mode


@retval   FUNCTION_SUCCESS   Get demod layer constellation mode successfully.
@retval   FUNCTION_ERROR     Get demod layer constellation mode unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_LAYER_CONSTELLATION() with the corresponding function.


@see   ISDBT_HIERARCHICAL_LAYER_INDEX, ISDBT_CONSTELLATION_MODE

*/
typedef int
(*ISDBT_DEMOD_FP_GET_LAYER_CONSTELLATION)(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	int *pLayerConstellation
);





/**

@brief   ISDB-T demod layer code rate mode getting function pointer

One can use ISDBT_DEMOD_FP_GET_LAYER_CODE_RATE() to get ISDB-T demod layer code rate mode according to
hierarchical layer index.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerCodeRate           Pointer to an allocated memory for storing demod layer code rate mode


@retval   FUNCTION_SUCCESS   Get demod layer code rate mode successfully.
@retval   FUNCTION_ERROR     Get demod layer code rate mode unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_LAYER_CODE_RATE() with the corresponding function.


@see   ISDBT_HIERARCHICAL_LAYER_INDEX, ISDBT_CODE_RATE_MODE

*/
typedef int
(*ISDBT_DEMOD_FP_GET_LAYER_CODE_RATE)(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	int *pLayerCodeRate
);





/**

@brief   ISDB-T demod layer interleaving length mode getting function pointer

One can use ISDBT_DEMOD_FP_GET_LAYER_INTERLEAVING_LEN() to get ISDB-T demod layer interleaving length mode
according to hierarchical layer index.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerInterleavingLen    Pointer to an allocated memory for storing demod layer interleaving len. mode


@retval   FUNCTION_SUCCESS   Get demod layer interleaving length mode successfully.
@retval   FUNCTION_ERROR     Get demod layer interleaving length mode unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_LAYER_INTERLEAVING_LEN() with the corresponding function.


@see   ISDBT_HIERARCHICAL_LAYER_INDEX, ISDBT_INTERLEAVING_LEN_MODE

*/
typedef int
(*ISDBT_DEMOD_FP_GET_LAYER_INTERLEAVING_LEN)(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	int *pLayerInterleavingLen
);





/**

@brief   ISDB-T demod layer segment number getting function pointer

One can use ISDBT_DEMOD_FP_GET_LAYER_SEGMENT_NUM() to get ISDB-T demod layer segment number according to
hierarchical layer index.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerSegmentNum         Pointer to an allocated memory for storing demod layer segment number


@retval   FUNCTION_SUCCESS   Get demod layer segment number successfully.
@retval   FUNCTION_ERROR     Get demod layer segment number unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_LAYER_SEGMENT_NUM() with the corresponding function.


@see   ISDBT_HIERARCHICAL_LAYER_INDEX, ISDBT_SEGMENT_NUM

*/
typedef int
(*ISDBT_DEMOD_FP_GET_LAYER_SEGMENT_NUM)(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	unsigned char *pLayerSegmentNum
);





/**

@brief   ISDB-T demod guard interval mode getting function pointer

One can use ISDBT_DEMOD_FP_GET_GUARD_INTERVAL() to get ISDB-T demod guard interval mode.


@param [in]    pDemod           The demod module pointer
@param [out]   pGuardInterval   Pointer to an allocated memory for storing demod guard interval mode


@retval   FUNCTION_SUCCESS   Get demod guard interval mode successfully.
@retval   FUNCTION_ERROR     Get demod guard interval mode unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_GUARD_INTERVAL() with the corresponding function.


@see   ISDBT_GUARD_INTERVAL_MODE

*/
typedef int
(*ISDBT_DEMOD_FP_GET_GUARD_INTERVAL)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pGuardInterval
);





/**

@brief   ISDB-T demod FFT mode getting function pointer

One can use ISDBT_DEMOD_FP_GET_FFT_MODE() to get ISDB-T demod FFT mode.


@param [in]    pDemod     The demod module pointer
@param [out]   pFftMode   Pointer to an allocated memory for storing demod FFT mode


@retval   FUNCTION_SUCCESS   Get demod FFT mode successfully.
@retval   FUNCTION_ERROR     Get demod FFT mode unsuccessfully.


@note
	-# Demod building function will set ISDBT_DEMOD_FP_GET_FFT_MODE() with the corresponding function.


@see   ISDBT_FFT_MODE_MODE

*/
typedef int
(*ISDBT_DEMOD_FP_GET_FFT_MODE)(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pFftMode
);





/// RTK_R ISDB-T extra module
typedef struct RTK_R_ISDBT_EXTRA_MODULE_TAG RTK_R_ISDBT_EXTRA_MODULE;

struct RTK_R_ISDBT_EXTRA_MODULE_TAG {
	// RTK_R ISDB-T extra variables
	U32BITS ShareMemoryOfst;          // Start address of share memory
	unsigned char ChipVersion;
	unsigned char NoUse;					// No use, for extra module architecture completion only.
};





/// ISDB-T demod module structure
struct ISDBT_DEMOD_MODULE_TAG {
	// Private variables
	int           DemodType;
	unsigned char DeviceAddr;
	U32BITS CrystalFreqHz;
	int           TsInterfaceMode;
	U32BITS IfFreqHz;
	int           SpectrumMode;

	int           BandwidthMode;
	unsigned char SegmentNum;
	unsigned char SubChannelNum;

	int IsBandwidthModeSet;
	int IsSegmentNumSet;
	int IsSubChannelNumSet;

	union {										///<   Demod extra module used by driving module
		RTK_R_ISDBT_EXTRA_MODULE RtkrIsdbt;
	}
	Extra;

	BASE_INTERFACE_MODULE *pBaseInterface;
	I2C_BRIDGE_MODULE *pI2cBridge;


	// Demod register table
	ISDBT_REG_ENTRY RegTable[ISDBT_REG_TABLE_LEN_MAX];


	// Demod I2C function pointers
	ISDBT_DEMOD_FP_SET_REG_BYTES       SetRegBytes;
	ISDBT_DEMOD_FP_GET_REG_BYTES       GetRegBytes;
	ISDBT_DEMOD_FP_SET_REG_MASK_BITS   SetRegMaskBits;
	ISDBT_DEMOD_FP_GET_REG_MASK_BITS   GetRegMaskBits;
	ISDBT_DEMOD_FP_SET_REG_BITS        SetRegBits;
	ISDBT_DEMOD_FP_GET_REG_BITS        GetRegBits;


	// Demod manipulating function pointers
	ISDBT_DEMOD_FP_GET_DEMOD_TYPE               GetDemodType;
	ISDBT_DEMOD_FP_GET_DEVICE_ADDR              GetDeviceAddr;
	ISDBT_DEMOD_FP_GET_CRYSTAL_FREQ_HZ          GetCrystalFreqHz;

	ISDBT_DEMOD_FP_IS_CONNECTED_TO_I2C          IsConnectedToI2c;

	ISDBT_DEMOD_FP_RESTART_FSM                  RestartFsm;

	ISDBT_DEMOD_FP_INITIALIZE                   Initialize;
	ISDBT_DEMOD_FP_SET_PARAMETERS               SetParameters;

	ISDBT_DEMOD_FP_GET_BANDWIDTH_MODE           GetBandwidthMode;
	ISDBT_DEMOD_FP_GET_SEGMENT_NUM              GetSegmentNum;
	ISDBT_DEMOD_FP_GET_SUB_CHANNEL_NUM          GetSubChannelNum;

	ISDBT_DEMOD_FP_IS_SIGNAL_LOCKED             IsSignalLocked;

	ISDBT_DEMOD_FP_IS_TR_LOCKED          IsTrLocked;
	ISDBT_DEMOD_FP_IS_CR_LOCKED          IsCrLocked;
	ISDBT_DEMOD_FP_IS_INNER_LOCKED       IsInnerLocked;
	ISDBT_DEMOD_FP_IS_NO_SIG             IsNoSig;

	ISDBT_DEMOD_FP_GET_SIGNAL_STRENGTH          GetSignalStrength;
	ISDBT_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY     GetLayerSignalQuality;

	ISDBT_DEMOD_FP_GET_LAYER_BER                GetLayerBer;
	ISDBT_DEMOD_FP_GET_LAYER_PER                GetLayerPer;
	ISDBT_DEMOD_FP_GET_SNR_DB                   GetSnrDb;

	ISDBT_DEMOD_FP_GET_RF_AGC                   GetRfAgc;
	ISDBT_DEMOD_FP_GET_IF_AGC                   GetIfAgc;
	ISDBT_DEMOD_FP_GET_DI_AGC                   GetDiAgc;

	ISDBT_DEMOD_FP_GET_TR_OFFSET_PPM            GetTrOffsetPpm;
	ISDBT_DEMOD_FP_GET_CR_OFFSET_HZ             GetCrOffsetHz;

	ISDBT_DEMOD_FP_GET_LAYER_CONSTELLATION      GetLayerConstellation;
	ISDBT_DEMOD_FP_GET_LAYER_CODE_RATE          GetLayerCodeRate;
	ISDBT_DEMOD_FP_GET_LAYER_INTERLEAVING_LEN   GetLayerInterleavingLen;
	ISDBT_DEMOD_FP_GET_LAYER_SEGMENT_NUM        GetLayerSegmentNum;

	ISDBT_DEMOD_FP_GET_GUARD_INTERVAL           GetGuardInterval;
	ISDBT_DEMOD_FP_GET_FFT_MODE                 GetFftMode;

	// 20120207 - kevin_wang add
	ISDBT_DEMOD_FP_SET_TS_INTERFACE_MODE        SetTsInterfaceMode;
	ISDBT_DEMOD_FP_SET_TS_INTERFACE_CLK_POL        SetTsInterfaceClkPol;
	ISDBT_DEMOD_FP_SET_IF_FREQ_HZ               SetIfFreqHz;
	ISDBT_DEMOD_FP_SET_SPECTRUM_MODE            SetSpectrumMode;
};







// ISDB-T demod default I2C functions
int
isdbt_demod_default_SetRegBytes(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	const unsigned char *pWritingBytes,
	unsigned char ByteNum
);

int
isdbt_demod_default_GetRegBytes(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char *pReadingBytes,
	unsigned char ByteNum
);

int
isdbt_demod_default_SetRegMaskBits(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	const U32BITS WritingValue
);

int
isdbt_demod_default_GetRegMaskBits(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	U32BITS *pReadingValue
);

int
isdbt_demod_default_SetRegBits(
	ISDBT_DEMOD_MODULE *pDemod,
	int RegBitName,
	const U32BITS WritingValue
);

int
isdbt_demod_default_GetRegBits(
	ISDBT_DEMOD_MODULE *pDemod,
	int RegBitName,
	U32BITS *pReadingValue
);





// ISDB-T demod default manipulating functions
void
isdbt_demod_default_GetDemodType(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pDemodType
);

void
isdbt_demod_default_GetDeviceAddr(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char *pDeviceAddr
);

void
isdbt_demod_default_GetCrystalFreqHz(
	ISDBT_DEMOD_MODULE *pDemod,
	U32BITS *pCrystalFreqHz
);

int
isdbt_demod_default_GetBandwidthMode(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pBandwidthMode
);

int
isdbt_demod_default_GetSegmentNum(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char *pSegmentNum
);

int
isdbt_demod_default_GetSubChannelNum(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char *pSubChannelNum
);



#define RTK_DEMOD_ISDBT_CALLER(fmt, args...)     rtd_demod_print(KERN_DEBUG,"CALLER, " fmt, ##args);
#define RTK_DEMOD_ISDBT_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"Debug, " fmt, ##args);
#define RTK_DEMOD_ISDBT_INFO(fmt, args...)       rtd_demod_print(KERN_INFO,"Info, " fmt, ##args);
#define RTK_DEMOD_ISDBT_WARNING(fmt, args...)    rtd_demod_print(KERN_WARNING,"Warning, " fmt, ##args);



#endif
