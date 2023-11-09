#ifndef __ATSC3_DEMOD_BASE_H
#define __ATSC3_DEMOD_BASE_H



#include "foundation.h"


typedef struct {
	unsigned char PLPvalid[4];
	unsigned int   ErrNum[4];
	unsigned int   ErrDen[4];
} TV_ATSC3_ErrRate;


typedef struct {
	unsigned char PLPId[64];
	unsigned char PLPCnt;
} TV_ATSC3_PLP_ID;


typedef enum {
	NONE_PLP_ID    = 0x00,
	FULL_PLP_ID,
	LLS_ONLY_PLP_ID
} ATSC3_MPLP_ID_MODE_SEL;


// Definitions

/***********************I2C Read/Write Return Status ****************************************/
#define RTK_R_ATSC3_SUCCESSFUL                0     /* No error */
#define	RTK_R_ATSC3_FAILED                    1     /* General error, usually RTK_R_ATSC3 APIs not correctly used or DTV system error */
#define	RTK_R_ATSC3_I2C_ERROR                 2     /*General I2C error,usually from I2C interface */
#define	RTK_R_ATSC3_NO_ACKNOWLEDGE            4     /* No acknowledge from I2C bus */
#define	RTK_R_ATSC3_NO_SLAVE_ACK              8     /* No acknowledge from devices on the GPIO I2C bus */




/*Internal used 64bit data structure*/
typedef struct atsc3_st64Data {
	unsigned int ui32Low, ui32High;
} RTK_R_ATSC3_64Data;


#define RTK_R_ATSC3_ADAPTIVE_TS_ENABLE  1   /*Set 1 to enable adaptive TS rate(Variable TS clock)*/

#define RTK_R_ATSC3_TS_CLOCK_VARIABLE      1
#define RTK_R_ATSC3_TS_CLOCK_FREQ_FIXED    0

// Bandwidth modes
#define RTK_R_ATSC3_BANDWIDTH_NONE   -1
enum RTK_R_ATSC3_BANDWIDTH_MODE {
	RTK_R_ATSC3_BANDWIDTH_6MHZ,
	RTK_R_ATSC3_BANDWIDTH_7MHZ,
	RTK_R_ATSC3_BANDWIDTH_8MHZ,
};



/* DVB-T FFT mode*/
typedef enum {
	RTK_R_ATSC3_DVBT_MODE_2K = 0,
	RTK_R_ATSC3_DVBT_MODE_8K,
	RTK_R_ATSC3_DVBT_MODE_RESERVED3,
	RTK_R_ATSC3_DVBT_MODE_UNKNOWN
} RTK_R_ATSC3_DVBT_FFT_MODE_TYPE;

/* DVB-T guard interval*/
typedef enum {
	RTK_R_ATSC3_DVBT_GI_1_32 = 0,
	RTK_R_ATSC3_DVBT_GI_1_16,
	RTK_R_ATSC3_DVBT_GI_1_8,
	RTK_R_ATSC3_DVBT_GI_1_4,
	RTK_R_ATSC3_DVBT_GI_UNKNOWN
} RTK_R_ATSC3_DVBT_GUARD_INTERVAL_TYPE;

/*DVB-T constellation*/
typedef enum {
	RTK_R_ATSC3_DVBT_CONSTELLATION_QPSK = 0,
	RTK_R_ATSC3_DVBT_CONSTELLATION_16QAM,
	RTK_R_ATSC3_DVBT_CONSTELLATION_64QAM,
	RTK_R_ATSC3_DVBT_CONSTELLATION_UNKNOWN
} RTK_R_ATSC3_DVBT_CONSTELLATION_TYPE;

/*DVB-T code rate*/
typedef enum {
	RTK_R_ATSC3_DVBT_CODE_RATE_1_2 = 0,
	RTK_R_ATSC3_DVBT_CODE_RATE_2_3,
	RTK_R_ATSC3_DVBT_CODE_RATE_3_4,
	RTK_R_ATSC3_DVBT_CODE_RATE_5_6,
	RTK_R_ATSC3_DVBT_CODE_RATE_7_8,
	RTK_R_ATSC3_DVBT_CODE_RATE_RESERVED5,
	RTK_R_ATSC3_DVBT_CODE_RATE_RESERVED6,
	RTK_R_ATSC3_DVBT_CODERATE_UNKNOWN
} RTK_R_ATSC3_DVBT_CODE_RATE_TYPE;

/*DVB-T hierarchy*/
typedef enum {
	RTK_R_ATSC3_DVBT_HIERARCHY_NON = 0,   /*Non-hierarchical*/
	RTK_R_ATSC3_DVBT_HIERARCHY_1,
	RTK_R_ATSC3_DVBT_HIERARCHY_2,
	RTK_R_ATSC3_DVBT_HIERARCHY_4,
	RTK_R_ATSC3_DVBT_HIERARCHY_UNKNOWN
} RTK_R_ATSC3_DVBT_HIERARCHY_TYPE;

/* DVB-T HP, LP profile*/
typedef enum {
	RTK_R_ATSC3_DVBT_HP_PROFILE = 0,
	RTK_R_ATSC3_DVBT_LP_PROFILE,
	RTK_R_ATSC3_DVBT_PROFILE_UNKNOWN
} RTK_R_ATSC3_DVBT_PROFILE_TYPE;



/***********************************************************/
/*                 DVB-T2 parameters                                                          */
/************************************************************/


/*DVB-T2 FFT mode*/
/*
typedef enum {
	RTK_R_ATSC3_MODE_8K = 0,
	RTK_R_ATSC3_MODE_16K,
	RTK_R_ATSC3_MODE_32K,
	RTK_R_ATSC3_MODE_RESERVED,
	RTK_R_ATSC3_MODE_UNKNOWN
} RTK_R_ATSC3_FFT_MODE_TYPE;
*/

/*DVB-T2 Guard interval*/
/*
typedef enum {
	RTK_R_ATSC3_GI0_RESERVED = 0,
	RTK_R_ATSC3_GI1_192,
	RTK_R_ATSC3_GI2_384,
	RTK_R_ATSC3_GI3_512,
	RTK_R_ATSC3_GI4_768,
	RTK_R_ATSC3_GI5_1024,
	RTK_R_ATSC3_GI6_1536,
	RTK_R_ATSC3_GI7_2048,
	RTK_R_ATSC3_GI8_2432,
	RTK_R_ATSC3_GI9_3072,
	RTK_R_ATSC3_GI10_3648,
	RTK_R_ATSC3_GI11_4096,
	RTK_R_ATSC3_GI12_4864,
	RTK_R_ATSC3_GI13_RESERVED,
	RTK_R_ATSC3_GI14_RESERVED,
	RTK_R_ATSC3_GI15_RESERVED
} RTK_R_ATSC3_GUARD_INTERVAL_TYPE;
*/

/*DVB-T2 constellation type of data PLP*/
/*
typedef enum {
	RTK_R_ATSC3_QPSK = 0,
	RTK_R_ATSC3_QAM_16,
	RTK_R_ATSC3_QAM_64,
	RTK_R_ATSC3_QAM_256,
	RTK_R_ATSC3_QAM_1024,
	RTK_R_ATSC3_QAM_4096,
	RTK_R_ATSC3_QAM_RESERVED6,
	RTK_R_ATSC3_QAM_RESERVED7,
	RTK_R_ATSC3_QAM_RESERVED8,
	RTK_R_ATSC3_QAM_RESERVED9,
	RTK_R_ATSC3_QAM_RESERVED10,
	RTK_R_ATSC3_QAM_RESERVED11,
	RTK_R_ATSC3_QAM_RESERVED12,
	RTK_R_ATSC3_QAM_RESERVED13,
	RTK_R_ATSC3_QAM_RESERVED14,
	RTK_R_ATSC3_QAM_RESERVED15
} RTK_R_ATSC3_CONSTELLATION_TYPE;
*/

/*ATSC3 code rate type of data PLP*/
/*
typedef enum {
	RTK_R_ATSC3_CODE_RATE_2_15 = 0,
	RTK_R_ATSC3_CODE_RATE_3_15,
	RTK_R_ATSC3_CODE_RATE_4_15,
	RTK_R_ATSC3_CODE_RATE_5_15,
	RTK_R_ATSC3_CODE_RATE_6_15,
	RTK_R_ATSC3_CODE_RATE_7_15,
	RTK_R_ATSC3_CODE_RATE_8_15,
	RTK_R_ATSC3_CODE_RATE_9_15,
	RTK_R_ATSC3_CODE_RATE_10_15,
	RTK_R_ATSC3_CODE_RATE_11_15,
	RTK_R_ATSC3_CODE_RATE_12_15,
	RTK_R_ATSC3_CODE_RATE_13_15,
	RTK_R_ATSC3_CODE_RATE_RESERVED12,
	RTK_R_ATSC3_CODE_RATE_RESERVED13,
	RTK_R_ATSC3_CODE_RATE_RESERVED14,
	RTK_R_ATSC3_CODE_RATE_RESERVED15
} RTK_R_ATSC3_CODE_RATE_TYPE;

*/
/*ATSC3 signal params structure */
typedef struct RTK_R_ATSC3_PARAMS_STRUCT {
	/*ATSC3 Transmit params */
	unsigned char bSpectrumInv ;
	unsigned char PLPSelectedCnt;
	unsigned char PLP[64];
	ATSC3_FFT_MODE_TYPE            FFTMode[4];
	ATSC3_GUARD_INTERVAL_TYPE      GuardInterval[4];
	ATSC3_CODE_RATE_TYPE           CodeRate[4];
	ATSC3_CONSTELLATION_TYPE       Constellation[4];
} RTK_R_ATSC3_PARAMS;



// DVB-T demod module pre-definition
typedef struct ATSC3_DEMOD_MODULE_TAG ATSC3_DEMOD_MODULE;

/**********************************************************************************************************************************
struct RTK_R_ATSC3_MPEGTSMode
@ui8TSTransferType: TS stream transfer type, can be set to parallel(8 bit data bus) or serial(1 bit data bus) mode
@ui8OutputEdge: demodulator will output TS data on this edge of TS stream clock
@ui8TSSPIMSBSelection: TS MSB on bit 0 or bit selection
@ui8TSSSIOutputSelection: TS output on bit 0 or bit 7 selection
@ui8SPIClockConstantOutput: TS stream clock can be set outputting all the time or only during TS data valid (188 bytes)
**********************************************************************************************************************************/
/*****************ui8TSTransferType Option Value***************************/
#define RTK_R_ATSC3_TS_PARALLEL_MODE             1
#define RTK_R_ATSC3_TS_SERIAL_MODE               0
/*****************ui8TSOnOff Option Value***************************/
#define RTK_R_ATSC3_TS_ON                        1
#define RTK_R_ATSC3_TS_OFF                       0
/**********************ui8OutputEdge Option Value***************************/
#define RTK_R_ATSC3_TS_OUTPUT_FALLING_EDGE       1
#define RTK_R_ATSC3_TS_OUTPUT_RISING_EDGE        0
/**********************ui8TSSPIMSBSelection Option Value******************/
#define RTK_R_ATSC3_TS_SPI_MSB_ON_DATA_BIT7      1
#define RTK_R_ATSC3_TS_SPI_MSB_ON_DATA_BIT0      0
/**********************ui8TSSSIOutputSelection Option Value***************/
#define RTK_R_ATSC3_TS_SSI_OUTPUT_ON_DATA_BIT7   1
#define RTK_R_ATSC3_TS_SSI_OUTPUT_ON_DATA_BIT0   0
/**********************ui8SPIClockConstantOutput Option Value*************/
#define RTK_R_ATSC3_TS_CLOCK_CONST_OUTPUT        1
#define RTK_R_ATSC3_TS_CLOCK_VALID_OUTPUT        0


typedef struct RTK_R_ATSC3_MPEG_TS_MODE_STRUCT {
	unsigned char ui8TSTransferType;
	unsigned char ui8OutputEdge;
	unsigned char ui8TSSPIMSBSelection;
	unsigned char ui8TSSSIOutputSelection;
	unsigned char ui8SPIClockConstantOutput;
	unsigned char ui8TSClockVariableMode;
} RTK_R_ATSC3_MPEGTSMode;

typedef struct RTK_R_ATSC3_TUNER_CONFIG_STRUCT {
	unsigned char   ui8IQSwapMode;	   /*IQ mode, DVB-T_T2 IQ auto detection*/
	unsigned char   ui8BandwidthType;  /*Bandwidth Type,eg.RTK_R_ATSC3_DVBT_BANDWIDTH_8MHZ, RTK_R_ATSC3_DVBT_BANDWIDTH_7MHZ etc*/
	unsigned int u32IFFrequencyKHz;  /*Tuner IF frequency for DVB-T,DVB-T2, Unit is KHz*/
	unsigned char  ui8TxIFAgcPolarity;
} RTK_R_ATSC3_TUNER_INPUT_T;

/**********************SWAP IQ or not, related with tuner's IF interface*************/
#define   RTK_R_ATSC3_INPUT_IQ_SWAP           1
#define   RTK_R_ATSC3_INPUT_IQ_NORMAL        0

/*******************IQ auto detection option, By default enable this function********/
#define  RTK_R_ATSC3_IQ_AUTO_ENABLE 1
#define  RTK_R_ATSC3_IQ_AUTO_DISABLE 0

/*******************Tuner's AGC polarity inverted or normal**************/
#define   RTK_R_ATSC3_IF_AGC_POLARITY_INVERTED      1
#define   RTK_R_ATSC3_IF_AGC_POLARITY_NORMAL        0



typedef struct RTK_R_ATSC3_CONFIG_STRUCT {
	/**************Common setting*******************************************/
	RTK_R_ATSC3_MPEGTSMode stMPEGTSMode;      /*TS interface configuration	*/
	unsigned int ui32DemodClkKHz;                /*Demodulator clock Select Unit is KHz, 24000 KHz*/

	unsigned char   gUseI2CRepeaterEnable;

	/************************mode specific  setting *****************/
	/*Set DVB-T(T2),  DVB-C, DVB-S, DVB-S2 mode*/
	unsigned char  ui8ConfigDVBType;    /*eg. RTK_R_ATSC3_DVBT_MODE(contains auto detection of DVB-T2), or  RTK_R_ATSC3_DVBC_MODE*/

	/*DVB-T,T2 setting parameters*/
	RTK_R_ATSC3_TUNER_INPUT_T Atsc3TunerInput; /*DVB-T and DVB-T2 only*/
} stRtkaAtsc3Config;



/**

@brief   DVB-T demod type getting function pointer

One can use ATSC3_DEMOD_FP_GET_DEMOD_TYPE() to get DVB-T demod type.


@param [in]    pDemod       The demod module pointer
@param [out]   pDemodType   Pointer to an allocated memory for storing demod type


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_DEMOD_TYPE() with the corresponding function.


@see   MODULE_TYPE

*/


typedef void
(*ATSC3_DEMOD_FP_GET_DEVICE_ADDR)(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char *pDeviceAddr
);


typedef void
(*ATSC3_DEMOD_FP_GET_DEMOD_TYPE)(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pDemodType
);

/**

@brief   DVB-TX demod crystal frequency getting function pointer

One can use DVBT_DEMOD_FP_GET_CRYSTAL_FREQ_HZ() to get DVB-TX demod crystal frequency in Hz.


@param [in]    pDemod           The demod module pointer
@param [out]   pCrystalFreqHz   Pointer to an allocated memory for storing demod crystal frequency in Hz


@retval   FUNCTION_SUCCESS   Get demod crystal frequency successfully.
@retval   FUNCTION_ERROR     Get demod crystal frequency unsuccessfully.


@note
	-# Demod building function will set DVBT_DEMOD_FP_GET_CRYSTAL_FREQ_HZ() with the corresponding function.

*/
typedef void
(*ATSC3_DEMOD_FP_GET_CRYSTAL_FREQ_HZ)(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pCrystalFreqHz
);


typedef void
(*ATSC3_DEMOD_FP_IS_CONNECTED_TO_I2C)(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
);

/**

@brief   DVB-T demod initializing function pointer

One can use ATSC3_DEMOD_FP_INITIALIZE() to initialie DVB-T demod.


@param [in]   pDemod   The demod module pointer


@retval   FUNCTION_SUCCESS   Initialize demod successfully.
@retval   FUNCTION_ERROR     Initialize demod unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_INITIALIZE() with the corresponding function.

*/
typedef int
(*ATSC3_DEMOD_FP_INITIALIZE)(
	ATSC3_DEMOD_MODULE *pDemod
);





/**

@brief   DVB-T demod bandwidth mode setting function pointer

One can use ATSC3_DEMOD_FP_SET_BANDWIDTH_MODE() to set DVB-T demod bandwidth mode.


@param [in]   pDemod	      The demod module pointer
@param [in]   BandwidthMode   Bandwidth mode for setting


@retval   FUNCTION_SUCCESS   Set demod bandwidth mode successfully.
@retval   FUNCTION_ERROR     Set demod bandwidth mode unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_SET_ATSC3_MODE() with the corresponding function.


@see   ATSC3_BANDWIDTH_MODE

*/
typedef int
(*ATSC3_DEMOD_FP_SET_BANDWIDTH_MODE)(
	ATSC3_DEMOD_MODULE *pDemod,
	int BandwidthMode
);



/**

@brief   DVB-T demod TS interface mode setting function pointer

One can use ATSC3_DEMOD_FP_SET_TS_INTERFACE_MODE() to set DVB-T demod TS interface mode.


@param [in]   pDemod            The demod module pointer
@param [in]   TsInterfaceMode   TS interface mode for setting


@retval   FUNCTION_SUCCESS   Set demod TS interface mode successfully.
@retval   FUNCTION_ERROR     Set demod TS interface mode unsuccessfully.


@note
	-# ATSC3_DEMOD_FP_SET_TS_INTERFACE_MODE() corresponding function is private.
	-# ATSC3_DEMOD_FP_SET_TS_INTERFACE_MODE() does not appear in demod module structure.


@see   TS_INTERFACE_MODE

*/
typedef int
(*ATSC3_DEMOD_FP_SET_TS_INTERFACE_MODE)(
	ATSC3_DEMOD_MODULE *pDemod,
	RTK_R_ATSC3_MPEGTSMode stTSMode
);



/**

@brief   DVB-T demod IF frequency setting function pointer

One can use ATSC3_DEMOD_FP_SET_IF_FREQ_HZ() to set DVB-T demod IF frequency in Hz.


@param [in]   pDemod     The demod module pointer
@param [in]   IfFreqHz   IF frequency in Hz for setting


@retval   FUNCTION_SUCCESS   Set demod IF frequency successfully.
@retval   FUNCTION_ERROR     Set demod IF frequency unsuccessfully.


@note
	-# ATSC3_DEMOD_FP_SET_IF_FREQ_HZ() corresponding function is private.
	-# ATSC3_DEMOD_FP_SET_IF_FREQ_HZ() does not appear in demod module structure.


@see   IF_FREQ_HZ

*/
typedef int
(*ATSC3_DEMOD_FP_SET_IF_FREQ_HZ)(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int IfFreqHz
);



/**

@brief   DVB-T demod spectrum mode setting function pointer

One can use ATSC3_DEMOD_FP_SET_SPECTRUM_MODE() to set DVB-T demod spectrum mode.


@param [in]   pDemod         The demod module pointer
@param [in]   SpectrumMode   Spectrum mode for setting


@retval   FUNCTION_SUCCESS   Set demod spectrum mode successfully.
@retval   FUNCTION_ERROR     Set demod spectrum mode unsuccessfully.


@note
	-# ATSC3_DEMOD_FP_SET_SPECTRUM_MODE() corresponding function is private.
	-# ATSC3_DEMOD_FP_SET_SPECTRUM_MODE() does not appear in demod module structure.


@see   SPECTRUM_MODE

*/
typedef int
(*ATSC3_DEMOD_FP_SET_SPECTRUM_MODE)(
	ATSC3_DEMOD_MODULE *pDemod,
	int SpectrumMode
);



/**

@brief   DVB-T demod bandwidth mode getting function pointer

One can use ATSC3_DEMOD_FP_GET_BANDWIDTH_MODE() to get DVB-T demod bandwidth mode.


@param [in]    pDemod           The demod module pointer
@param [out]   pBandwidthMode   Pointer to an allocated memory for storing demod bandwidth mode


@retval   FUNCTION_SUCCESS   Get demod bandwidth mode successfully.
@retval   FUNCTION_ERROR     Get demod bandwidth mode unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_BANDWIDTH_MODE() with the corresponding function.


@see   ATSC3_ATSC3_MODE

*/
typedef int
(*ATSC3_DEMOD_FP_GET_BANDWIDTH_MODE)(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pBandwidthMode
);



typedef int
(*ATSC3_DEMOD_FP_GET_IF_FREQ_HZ)(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pIfFreqHz
);


typedef int
(*ATSC3_DEMOD_FP_GET_SPECTRUM_MODE)(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pSpectrumMode
);


/**

@brief   DVB-T demod signal lock asking function pointer

One can use ATSC3_DEMOD_FP_IS_SIGNAL_LOCKED() to ask DVB-T demod if it is signal-locked.


@param [in]    pDemod    The demod module pointer
@param [out]   pAnswer   Pointer to an allocated memory for storing answer


@retval   FUNCTION_SUCCESS   Perform signal lock asking to demod successfully.
@retval   FUNCTION_ERROR     Perform signal lock asking to demod unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_IS_SIGNAL_LOCKED() with the corresponding function.

*/
typedef int
(*ATSC3_DEMOD_FP_IS_SIGNAL_LOCKED)(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
);

typedef int
(*ATSC3_DEMOD_FP_IS_SIGNAL_SYNCLOCKED)(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
);



typedef int
(*ATSC3_DEMOD_FP_SOFT_RESET)(
	ATSC3_DEMOD_MODULE *pDemod
);


/**

@brief   DVB-T demod signal strength getting function pointer

One can use ATSC3_DEMOD_FP_GET_SIGNAL_STRENGTH() to get signal strength.


@param [in]    pDemod            The demod module pointer
@param [out]   pSignalStrength   Pointer to an allocated memory for storing signal strength (0 ~ 100)


@retval   FUNCTION_SUCCESS   Get demod signal strength successfully.
@retval   FUNCTION_ERROR     Get demod signal strength unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_SIGNAL_STRENGTH() with the corresponding function.

*/
typedef int
(*ATSC3_DEMOD_FP_GET_SIGNAL_STRENGTH)(
	ATSC3_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	unsigned int *pSignalStrength
);





/**

@brief   DVB-T demod layer signal quality getting function pointer

One can use ATSC3_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY() to get layer signal quality.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerSignalQuality      Pointer to an allocated memory for storing layer signal quality (0 ~ 100)


@retval   FUNCTION_SUCCESS   Get demod layer signal quality successfully.
@retval   FUNCTION_ERROR     Get demod layer signal quality unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY() with the corresponding function.


@see   ATSC3_HIERARCHICAL_LAYER_INDEX

*/
typedef int
(*ATSC3_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY)(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pSignalQuality
);





/**

@brief   DVB-T demod layer BER getting function pointer

One can use ATSC3_DEMOD_FP_GET_LAYER_BER() to get layer BER.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerBerNum             Pointer to an allocated memory for storing layer BER numerator
@param [out]   pLayerBerDen             Pointer to an allocated memory for storing layer BER denominator


@retval   FUNCTION_SUCCESS   Get demod layer BER successfully.
@retval   FUNCTION_ERROR     Get demod layer BER unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_LAYER_BER() with the corresponding function.

*/
typedef int
(*ATSC3_DEMOD_FP_GET_BER)(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_ErrRate *PacketErr
);





/**

@brief   DVB-T demod layer PER getting function pointer

One can use ATSC3_DEMOD_FP_GET_LAYER_PER() to get layer PER.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerPerNum             Pointer to an allocated memory for storing layer PER numerator
@param [out]   pLayerPerDen             Pointer to an allocated memory for storing layer PER denominator


@retval   FUNCTION_SUCCESS   Get demod layer PER successfully.
@retval   FUNCTION_ERROR     Get demod layer PER unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_LAYER_PER() with the corresponding function.

*/
typedef int
(*ATSC3_DEMOD_FP_GET_PER)(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_ErrRate *PacketErr
);





/**

@brief   DVB-T demod SNR getting function pointer

One can use ATSC3_DEMOD_FP_GET_SNR_DB() to get SNR in dB.


@param [in]    pDemod      The demod module pointer
@param [out]   pSnrDbNum   Pointer to an allocated memory for storing SNR dB numerator
@param [out]   pSnrDbDen   Pointer to an allocated memory for storing SNR dB denominator


@retval   FUNCTION_SUCCESS   Get demod SNR successfully.
@retval   FUNCTION_ERROR     Get demod SNR unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_SNR_DB() with the corresponding function.

*/
typedef int
(*ATSC3_DEMOD_FP_GET_SNR_DB)(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *ui32SNRValuex10
);



typedef int
(*ATSC3_DEMOD_FP_SET_AGC_POL)(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char agcpola
);


/**

@brief   DVB-T demod IF AGC getting function pointer

One can use ATSC3_DEMOD_FP_GET_IF_AGC() to get DVB-T demod IF AGC value.


@param [in]    pDemod   The demod module pointer
@param [out]   pIfAgc   Pointer to an allocated memory for storing IF AGC value


@retval   FUNCTION_SUCCESS   Get demod IF AGC value successfully.
@retval   FUNCTION_ERROR     Get demod IF AGC value unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_IF_AGC() with the corresponding function.

*/
typedef int
(*ATSC3_DEMOD_FP_GET_IF_AGC)(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pIfAgc
);


/**

@brief   DVB-T demod TR offset getting function pointer

One can use ATSC3_DEMOD_FP_GET_TR_OFFSET_PPM() to get TR offset in ppm.


@param [in]    pDemod         The demod module pointer
@param [out]   pTrOffsetPpm   Pointer to an allocated memory for storing TR offset in ppm


@retval   FUNCTION_SUCCESS   Get demod TR offset successfully.
@retval   FUNCTION_ERROR     Get demod TR offset unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_TR_OFFSET_PPM() with the corresponding function.

*/
typedef int
(*ATSC3_DEMOD_FP_GET_TR_OFFSET_PPM)(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pTrOffsetPpm
);





/**

@brief   DVB-T demod CR offset getting function pointer

One can use ATSC3_DEMOD_FP_GET_CR_OFFSET_HZ() to get CR offset in Hz.


@param [in]    pDemod        The demod module pointer
@param [out]   pCrOffsetHz   Pointer to an allocated memory for storing CR offset in Hz


@retval   FUNCTION_SUCCESS   Get demod CR offset successfully.
@retval   FUNCTION_ERROR     Get demod CR offset unsuccessfully.


@note
	-# Demod building function will set ATSC3_DEMOD_FP_GET_CR_OFFSET_HZ() with the corresponding function.

*/
typedef int
(*ATSC3_DEMOD_FP_GET_CR_OFFSET_HZ)(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pCrOffsetHz
);


typedef int
(*ATSC3_DEMOD_FP_SET_PLPID)(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_PLP_ID Atsc3Plp
);


typedef int
(*ATSC3_DEMOD_FP_GET_PLPID)(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_PLP_ID* Atsc3Plp,
	ATSC3_MPLP_ID_MODE_SEL Atsc3MPlpMode
);

typedef int
(*ATSC3_DEMOD_FP_GET_SIGANLPARAMS)(
	ATSC3_DEMOD_MODULE *pDemod,
	RTK_R_ATSC3_PARAMS *pAtsc3Params
);


typedef int
(*ATSC3_DEMOD_FP_GET_SIGANLPARAMST)(
	ATSC3_DEMOD_MODULE *pDemod,
	RTK_R_ATSC3_PARAMS *pAtsc3Params
);


typedef int
(*ATSC3_DEMOD_FP_GET_TS_THROUGHPUT)(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *TSThroughputBps
);


/// RTD299S DVB-T extra module
typedef struct RTK_R_ATSC3_EXTRA_MODULE_TAG RTK_R_ATSC3_EXTRA_MODULE;

struct RTK_R_ATSC3_EXTRA_MODULE_TAG {
	// RTD299S DVB-T extra variables
	unsigned int ShareMemoryOfst;          // Start address of share memory
	unsigned char NoUse;					// No use, for extra module architecture completion only.
};





/// DVB-T demod module structure
struct ATSC3_DEMOD_MODULE_TAG {
	// Private variables
	int           DemodType;
       unsigned char DeviceAddr;
	unsigned int CrystalFreqHz;
	int           TsInterfaceMode;
	unsigned int IfFreqHz;
	int           SpectrumMode;

	int IsIfFreqHzSet;
	int IsSpectrumModeSet;

       unsigned char m_ldpcIterMode; 
       unsigned char m_ldpcIterNum;

	union {										///<   Demod extra module used by driving module
		RTK_R_ATSC3_EXTRA_MODULE RtkaAtsc3;
	}
	Extra;

	BASE_INTERFACE_MODULE *pBaseInterface;
	I2C_BRIDGE_MODULE *pI2cBridge;


	// Demod manipulating function pointers
	ATSC3_DEMOD_FP_GET_DEMOD_TYPE               GetDemodType;
    	ATSC3_DEMOD_FP_GET_DEVICE_ADDR       GetDeviceAddr;
	ATSC3_DEMOD_FP_GET_CRYSTAL_FREQ_HZ          GetCrystalFreqHz;

       ATSC3_DEMOD_FP_IS_CONNECTED_TO_I2C   IsConnectedToI2c;

	ATSC3_DEMOD_FP_INITIALIZE                   Initialize;
	ATSC3_DEMOD_FP_SET_BANDWIDTH_MODE           SetBandwidthMode;
	ATSC3_DEMOD_FP_GET_BANDWIDTH_MODE           GetBandwidthMode;
    	ATSC3_DEMOD_FP_GET_IF_FREQ_HZ        GetIfFreqHz;
	ATSC3_DEMOD_FP_GET_SPECTRUM_MODE     GetSpectrumMode;

	ATSC3_DEMOD_FP_IS_SIGNAL_LOCKED             IsSignalLocked;
	ATSC3_DEMOD_FP_IS_SIGNAL_SYNCLOCKED    IsSignalSyncLocked;
	ATSC3_DEMOD_FP_IS_SIGNAL_SYNCLOCKED    IsL1Locked;

	ATSC3_DEMOD_FP_SOFT_RESET SoftwareReset;

	ATSC3_DEMOD_FP_GET_SIGNAL_STRENGTH          GetSignalStrength;
	ATSC3_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY     GetLayerSignalQuality;

	ATSC3_DEMOD_FP_GET_BER                GetBer;
	ATSC3_DEMOD_FP_GET_PER                GetPer;
	ATSC3_DEMOD_FP_GET_SNR_DB                   GetSnrDb;

	ATSC3_DEMOD_FP_SET_AGC_POL    SetAgcPol;

	ATSC3_DEMOD_FP_GET_IF_AGC                   GetIfAgc;

	ATSC3_DEMOD_FP_GET_TR_OFFSET_PPM            GetTrOffsetPpm;
	ATSC3_DEMOD_FP_GET_CR_OFFSET_HZ             GetCrOffsetHz;


	ATSC3_DEMOD_FP_SET_TS_INTERFACE_MODE        SetTsInterfaceMode;

	ATSC3_DEMOD_FP_SET_IF_FREQ_HZ               SetIfFreqHz;
	ATSC3_DEMOD_FP_SET_SPECTRUM_MODE            SetSpectrumMode;
	ATSC3_DEMOD_FP_SET_PLPID    SetPlpID;
	ATSC3_DEMOD_FP_GET_PLPID    GetPlpID;

	ATSC3_DEMOD_FP_GET_SIGANLPARAMS  GetSignalParams;
	ATSC3_DEMOD_FP_GET_TS_THROUGHPUT  GetTsThroughput;

};






void RTK_R_ATSC3_64Mult(RTK_R_ATSC3_64Data *pstRst, unsigned int m1, unsigned int m2);
unsigned int RTK_R_ATSC3_64Div(RTK_R_ATSC3_64Data stDivisor, RTK_R_ATSC3_64Data stDividend);
RTK_R_ATSC3_64Data RTK_R_ATSC3_64DivReturn64(RTK_R_ATSC3_64Data stDivisor, RTK_R_ATSC3_64Data stDividend);
unsigned int RTK_R_ATSC3_Log10ConvertdBx10(unsigned int i32InValue);
unsigned int RTK_R_ATSC3_Log10ConvertdB(unsigned int i32InValue);
unsigned int  RTK_R_ATSC3_FixPow(unsigned int x, unsigned int y);
unsigned char  RTK_R_ATSC3_64ComPare(RTK_R_ATSC3_64Data stPara, RTK_R_ATSC3_64Data stPara1);
void  RTK_R_ATSC3_MiddleValueu8(unsigned char u8ValueA, unsigned char u8ValueB, unsigned char u8ValueC, unsigned char *MidValue);



int rtd_4bytes_i2c_wr(ATSC3_DEMOD_MODULE *pDemod, unsigned int addr, unsigned int data);
int rtd_4bytes_i2c_rd(ATSC3_DEMOD_MODULE *pDemod, unsigned int addr, unsigned int* data);
int rtd_4bytes_i2c_mask(ATSC3_DEMOD_MODULE *pDemod, unsigned int addr, unsigned int mask, unsigned int data);

// ATSC demod default manipulaing functions
void atsc3_demod_default_GetDemodType(	ATSC3_DEMOD_MODULE *pDemod, int *pDemodType);
void atsc3_demod_default_GetDeviceAddr(ATSC3_DEMOD_MODULE *pDemod, unsigned char *pDeviceAddr);
void atsc3_demod_default_GetCrystalFreqHz(ATSC3_DEMOD_MODULE *pDemod, unsigned int *pCrystalFreqHz);
int atsc3_demod_default_GetIfFreqHz(ATSC3_DEMOD_MODULE *pDemod, unsigned int *pIfFreqHz);
int atsc3_demod_default_GetSpectrumMode(ATSC3_DEMOD_MODULE *pDemod, int *pSpectrumMode);


#define RTK_DEMOD_ATSC3_CALLER(fmt, args...)     rtd_demod_print(KERN_DEBUG,"CALLER, " fmt, ##args);
#define RTK_DEMOD_ATSC3_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"Debug, " fmt, ##args);
#define RTK_DEMOD_ATSC3_INFO(fmt, args...)       rtd_demod_print(KERN_INFO,"Info, " fmt, ##args);
#define RTK_DEMOD_ATSC3_WARNING(fmt, args...)    rtd_demod_print(KERN_WARNING,"Warning, " fmt, ##args);




#endif
