#ifndef __DVBTX_DEMOD_BASE_H
#define __DVBTX_DEMOD_BASE_H



#include "foundation.h"


typedef unsigned char RTK_A_DVBTX_STATUS;

// Definitions

/**********************Chip Support receiving Mode Macro*******************************************************/
//#define RTK_A_DVBTX_DVBC_MODE                 0
//#define RTK_A_DVBTX_RSV_MODE                  1     /*Reserved*/
#define RTK_A_DVBTX_DVBT_MODE                 2     /*DVB-T and DVB-T2, The register setting is not different*/
#define RTK_A_DVBTX_DVBT2_MODE                3
//#define RTK_A_DVBTX_DVBS_MODE                 4
//#define RTK_A_DVBTX_DVBS2_MODE                5

//#define RTK_A_DVBTX_DVBC2_MODE				   6
#define RTK_A_DVBTX_DVB_UNKNOWN               255


#define ATBM783X_SUPPORT_DVBS2X 0  /*DVB-S2x option, if DVB-S2x not required,just disable it*/



/***********************I2C Read/Write Return Status ****************************************/
#define RTK_A_DVBTX_SUCCESSFUL                0     /* No error */
#define	RTK_A_DVBTX_FAILED                    1     /* General error, usually RTK_A_DVBTX APIs not correctly used or DTV system error */
#define	RTK_A_DVBTX_I2C_ERROR                 2     /*General I2C error,usually from I2C interface */
#define	RTK_A_DVBTX_NO_ACKNOWLEDGE            4     /* No acknowledge from I2C bus */
#define	RTK_A_DVBTX_NO_SLAVE_ACK              8     /* No acknowledge from devices on the GPIO I2C bus */




/*Internal used 64bit data structure*/
typedef struct st64Data {
	unsigned int ui32Low, ui32High;
} RTK_A_DVBTX_64Data;


#define RTK_A_DVBTX_ADAPTIVE_TS_ENABLE  1   /*Set 1 to enable adaptive TS rate(Variable TS clock)*/

#define RTK_A_DVBTX_TS_CLOCK_VARIABLE      1
#define RTK_A_DVBTX_TS_CLOCK_FREQ_FIXED    0

// Bandwidth modes
#define RTK_A_DVBTX_BANDWIDTH_NONE   -1
enum RTK_A_DVBTX_BANDWIDTH_MODE {
	RTK_A_DVBTX_BANDWIDTH_1D7MHZ,   /*only support and used for T2 */
	RTK_A_DVBTX_BANDWIDTH_5MHZ,
	RTK_A_DVBTX_BANDWIDTH_6MHZ,
	RTK_A_DVBTX_BANDWIDTH_7MHZ,
	RTK_A_DVBTX_BANDWIDTH_8MHZ,
	RTK_A_DVBTX_BANDWIDTH_10MHZ,
};



/* DVB-T FFT mode*/
typedef enum {
	RTK_A_DVBTX_DVBT_MODE_2K = 0,
	RTK_A_DVBTX_DVBT_MODE_8K,
	RTK_A_DVBTX_DVBT_MODE_RESERVED3,
	RTK_A_DVBTX_DVBT_MODE_UNKNOWN
} RTK_A_DVBTX_DVBT_FFT_MODE_TYPE;

/* DVB-T guard interval*/
typedef enum {
	RTK_A_DVBTX_DVBT_GI_1_32 = 0,
	RTK_A_DVBTX_DVBT_GI_1_16,
	RTK_A_DVBTX_DVBT_GI_1_8,
	RTK_A_DVBTX_DVBT_GI_1_4,
	RTK_A_DVBTX_DVBT_GI_UNKNOWN
} RTK_A_DVBTX_DVBT_GUARD_INTERVAL_TYPE;

/*DVB-T constellation*/
typedef enum {
	RTK_A_DVBTX_DVBT_CONSTELLATION_QPSK = 0,
	RTK_A_DVBTX_DVBT_CONSTELLATION_16QAM,
	RTK_A_DVBTX_DVBT_CONSTELLATION_64QAM,
	RTK_A_DVBTX_DVBT_CONSTELLATION_UNKNOWN
} RTK_A_DVBTX_DVBT_CONSTELLATION_TYPE;

/*DVB-T code rate*/
typedef enum {
	RTK_A_DVBTX_DVBT_CODE_RATE_1_2 = 0,
	RTK_A_DVBTX_DVBT_CODE_RATE_2_3,
	RTK_A_DVBTX_DVBT_CODE_RATE_3_4,
	RTK_A_DVBTX_DVBT_CODE_RATE_5_6,
	RTK_A_DVBTX_DVBT_CODE_RATE_7_8,
	RTK_A_DVBTX_DVBT_CODE_RATE_RESERVED5,
	RTK_A_DVBTX_DVBT_CODE_RATE_RESERVED6,
	RTK_A_DVBTX_DVBT_CODERATE_UNKNOWN
} RTK_A_DVBTX_DVBT_CODE_RATE_TYPE;

/*DVB-T hierarchy*/
typedef enum {
	RTK_A_DVBTX_DVBT_HIERARCHY_NON = 0,   /*Non-hierarchical*/
	RTK_A_DVBTX_DVBT_HIERARCHY_1,
	RTK_A_DVBTX_DVBT_HIERARCHY_2,
	RTK_A_DVBTX_DVBT_HIERARCHY_4,
	RTK_A_DVBTX_DVBT_HIERARCHY_UNKNOWN
} RTK_A_DVBTX_DVBT_HIERARCHY_TYPE;

/* DVB-T HP, LP profile*/
typedef enum {
	RTK_A_DVBTX_DVBT_HP_PROFILE = 0,
	RTK_A_DVBTX_DVBT_LP_PROFILE,
	RTK_A_DVBTX_DVBT_PROFILE_UNKNOWN
} RTK_A_DVBTX_DVBT_PROFILE_TYPE;



/***********************************************************/
/*                 DVB-T2 parameters                                                          */
/************************************************************/
/*DVB-T2 FFT mode*/
typedef enum {
	RTK_A_DVBTX_DVBT2_MODE_1K = 0,
	RTK_A_DVBTX_DVBT2_MODE_2K,
	RTK_A_DVBTX_DVBT2_MODE_4K,
	RTK_A_DVBTX_DVBT2_MODE_8K,
	RTK_A_DVBTX_DVBT2_MODE_16K,
	RTK_A_DVBTX_DVBT2_MODE_32K,
	RTK_A_DVBTX_DVBT2_MODE_RESERVED,
	RTK_A_DVBTX_DVBT2_MODE_UNKNOWN
} RTK_A_DVBTX_DVBT2_FFT_MODE_TYPE;

/*DVB-T2 Guard interval*/
typedef enum {
	RTK_A_DVBTX_DVBT2_GI_1_32 = 0,
	RTK_A_DVBTX_DVBT2_GI_1_16,
	RTK_A_DVBTX_DVBT2_GI_1_8,
	RTK_A_DVBTX_DVBT2_GI_1_4,
	RTK_A_DVBTX_DVBT2_GI_1_128,
	RTK_A_DVBTX_DVBT2_GI_19_128,
	RTK_A_DVBTX_DVBT2_GI_19_256,
	RTK_A_DVBTX_DVBT2_GI_UNKNOWN
} RTK_A_DVBTX_DVBT2_GUARD_INTERVAL_TYPE;

/*DVB-T2 Base profile or Lite profile*/
typedef enum {
	RTK_A_DVBTX_DVBT2_BASE_PROFILE = 0,
	RTK_A_DVBTX_DVBT2_LITE_PROFILE,
	RTK_A_DVBTX_DVBT2_BASE_LITE_UNKNOWN
} RTK_A_DVBTX_DVBT2_BASE_OR_LITE_TYPE;

/*DVB-T2 standard version number*/
typedef enum {
	RTK_A_DVBTX_DVBT2_V111 = 0,
	RTK_A_DVBTX_DVBT2_V121,
	RTK_A_DVBTX_DVBT2_V131,
	RTK_A_DVBTX_DVBT2_VERSION_UNKNOWN
} RTK_A_DVBTX_DVBT2_VERSION_TYPE;


/*DVBT2 S1 signaling definition */
typedef enum {
	RTK_A_DVBTX_DVBT2_SISO = 0,
	RTK_A_DVBTX_DVBT2_MISO,
	RTK_A_DVBTX_DVBT2_NON_DVBT2,                     /* Non DVB-T2 */
	RTK_A_DVBTX_DVBT2_LITE_SISO,
	RTK_A_DVBTX_DVBT2_LITE_MISO,
	RTK_A_DVBTX_DVBT2_S1_RESERVED5,                  /* Reserved */
	RTK_A_DVBTX_DVBT2_S1_RESERVED6,
	RTK_A_DVBTX_DVBT2_SISO_MISO_UNKNOWN
} RTK_A_DVBTX_DVBT2_SISO_MISO_TYPE;


/*DVB-T2 bandwidth extended or not extended*/
typedef enum {
	RTK_A_DVBTX_DVBT2_BANDWIDTH_NO_EXT = 0,
	RTK_A_DVBTX_DVBT2_BANDWIDTH_EXTENDED,
	RTK_A_DVBTX_DVBT2_BANDWIDTH_EXT_UNKNOWN
} RTK_A_DVBTX_DVBT2_BANDWIDTH_EXT_TYPE;

/*DVB-T2 constellation type of data PLP*/
typedef enum {
	RTK_A_DVBTX_DVBT2_QPSK = 0,
	RTK_A_DVBTX_DVBT2_QAM_16,
	RTK_A_DVBTX_DVBT2_QAM_64,
	RTK_A_DVBTX_DVBT2_QAM_256,
	RTK_A_DVBTX_DVBT2_QAM_RESERVED4,
	RTK_A_DVBTX_DVBT2_QAM_RESERVED5,
	RTK_A_DVBTX_DVBT2_QAM_RESERVED6,
	RTK_A_DVBTX_DVBT2_QAM_RESERVED7,
	RTK_A_DVBTX_DVBT2_QAM_UNKNOWN
} RTK_A_DVBTX_DVBT2_CONSTELLATION_TYPE;


/*DVB-T2 code rate type of data PLP*/
typedef enum {
	RTK_A_DVBTX_DVBT2_CODE_RATE_1_2 = 0,
	RTK_A_DVBTX_DVBT2_CODE_RATE_3_5,
	RTK_A_DVBTX_DVBT2_CODE_RATE_2_3,
	RTK_A_DVBTX_DVBT2_CODE_RATE_3_4,
	RTK_A_DVBTX_DVBT2_CODE_RATE_4_5,
	RTK_A_DVBTX_DVBT2_CODE_RATE_5_6,
	RTK_A_DVBTX_DVBT2_CODE_RATE_1_3, /*for t2 lite*/
	RTK_A_DVBTX_DVBT2_CODE_RATE_2_5, /*for t2 lite*/
	RTK_A_DVBTX_DVBT2_CODE_RATE_UNKNOWN
} RTK_A_DVBTX_DVBT2_CODE_RATE_TYPE;

/*DVB-T2 pilot pattern*/
typedef enum {
	RTK_A_DVBTX_DVBT2_PP1 = 0,
	RTK_A_DVBTX_DVBT2_PP2,
	RTK_A_DVBTX_DVBT2_PP3,
	RTK_A_DVBTX_DVBT2_PP4,
	RTK_A_DVBTX_DVBT2_PP5,
	RTK_A_DVBTX_DVBT2_PP6,
	RTK_A_DVBTX_DVBT2_PP7,
	RTK_A_DVBTX_DVBT2_PP8,
	RTK_A_DVBTX_DVBT2_PP_RESERVED8,       /* Reserved */
	RTK_A_DVBTX_DVBT2_PP_RESERVED9,
	RTK_A_DVBTX_DVBT2_PP_RESERVED10,
	RTK_A_DVBTX_DVBT2_PP_RESERVED11,
	RTK_A_DVBTX_DVBT2_PP_RESERVED12,
	RTK_A_DVBTX_DVBT2_PP_RESERVED13,
	RTK_A_DVBTX_DVBT2_PP_RESERVED14,
	RTK_A_DVBTX_DVBT2_PP_RESERVED15,      /* Reserved */
	RTK_A_DVBTX_DVBT2_PP_UNKNOWN
} RTK_A_DVBTX_DVBT2_PILOT_PATTERN_TYPE;

/*DVB-T2 data payload type*/
typedef enum {
	RTK_A_DVBTX_DVBT2_PAYLOAD_GFPS = 0,
	RTK_A_DVBTX_DVBT2_PAYLOAD_GCS,
	RTK_A_DVBTX_DVBT2_PAYLOAD_GSE,
	RTK_A_DVBTX_DVBT2_PAYLOAD_TS,
	RTK_A_DVBTX_DVBT2_PAYLOAD_UNKNOWN
} RTK_A_DVBTX_DVBT2_PLP_PAYLOAD_TYPE;


/*DVB-T2 data type of PLP*/
typedef enum {
	RTK_A_DVBTX_DVBT2_COMMON_PLP = 0,                       /* DVBT2 PLP Common Type */
	RTK_A_DVBTX_DVBT2_DATA_PLP_TYPE1,                       /* DVBT2 PLP Data Type 1 */
	RTK_A_DVBTX_DVBT2_DATA_PLP_TYPE2,                       /* DVBT2 PLP Data Type 2 */
	RTK_A_DVBTX_DVBT2_PLP_TYPE_RSVD3,                       /*Reserved */
	RTK_A_DVBTX_DVBT2_PLP_TYPE_RSVD4,
	RTK_A_DVBTX_DVBT2_PLP_TYPE_RSVD5,
	RTK_A_DVBTX_DVBT2_PLP_TYPE_RSVD6,
	RTK_A_DVBTX_DVBT2_PLP_TYPE_RSVD7,                       /* Reserved */
	RTK_A_DVBTX_DVBT2_PLP_TYPE_UNKNOWN
} RTK_A_DVBTX_DVBT2_PLP_TYPE;

/*DVB-T2 data PLP's ISSY type*/
typedef enum {
	RTK_A_DVBTX_DVBT2_ISSY_NO = 0,
	RTK_A_DVBTX_DVBT2_ISSY_SHORT,
	RTK_A_DVBTX_DVBT2_ISSY_LONG,
	RTK_A_DVBTX_DVBT2_ISSY_UNKNOWN
} RTK_A_DVBTX_DVBT2_ISSY_TYPE;


/*DVB-T2 data PLP's LDPC length type*/
typedef enum {
	RTK_A_DVBTX_DVBT2_LDPC_SHORT = 0,
	RTK_A_DVBTX_DVBT2_LDPC_NORMAL,
	RTK_A_DVBTX_DVBT2_LDPC_RESERVED,
	RTK_A_DVBTX_DVBT2_LDPC_LENGTH_UNKNOWN
} RTK_A_DVBTX_DVBT2_FEC_LENGTH_TYPE;

/*DVB-T2 data PLP's stream mode*/
typedef enum {
	RTK_A_DVBTX_DVBT2_NORMAL_MODE = 0,
	RTK_A_DVBTX_DVBT2_HEM_MODE,
	RTK_A_DVBTX_DVBT2_STREAM_MODE_UNKNOWN
} RTK_A_DVBTX_DVBT2_STREAM_MODE_TYPE;



/*DVB-T signal params structure */
typedef struct RTK_A_DVBTX_DVBT_TPS_STRUCT {
	RTK_A_DVBTX_DVBT_FFT_MODE_TYPE       DvbtFFTMode;
	RTK_A_DVBTX_DVBT_GUARD_INTERVAL_TYPE DvbtGuardInterval;
	RTK_A_DVBTX_DVBT_CONSTELLATION_TYPE  DvbtConstellation;
	RTK_A_DVBTX_DVBT_CODE_RATE_TYPE      DvbtHpCodeRate;
	RTK_A_DVBTX_DVBT_CODE_RATE_TYPE      DvbtLpCodeRate;
	RTK_A_DVBTX_DVBT_HIERARCHY_TYPE      DvbtHierarchy;
	RTK_A_DVBTX_DVBT_PROFILE_TYPE        DvbtProfile;
	unsigned int                       DvbtCellId;
	unsigned char                             DvbtIQSwap;  //LGE
} RTK_A_DVBTX_DVBT_PARAMS;

/*DVB-T2 signal params structure */
typedef struct RTK_A_DVBTX_DVBT2_PARAMS_STRUCT {
	/*DVB-T2 Transmit params */
	RTK_A_DVBTX_DVBT2_FFT_MODE_TYPE            Dvbt2FFTMode;
	RTK_A_DVBTX_DVBT2_BANDWIDTH_EXT_TYPE       Dvbt2BandwidthExt;
	RTK_A_DVBTX_DVBT2_GUARD_INTERVAL_TYPE      Dvbt2GuardInterval;
	RTK_A_DVBTX_DVBT2_PILOT_PATTERN_TYPE       Dvbt2PilotPattern;
	RTK_A_DVBTX_DVBT2_SISO_MISO_TYPE           Dvbt2SisoMiso;
	RTK_A_DVBTX_DVBT2_VERSION_TYPE             Dvbt2Version;
	RTK_A_DVBTX_DVBT2_BASE_OR_LITE_TYPE        Dvbt2BaseOrLiteType;
	unsigned int                       Dvbt2CellId;
	unsigned int                       Dvbt2NetworkId;
	unsigned int                       Dvbt2T2SystemId;

	unsigned char  u8FefType;
	unsigned int u32FefLength;
	unsigned char  u8FefInterval;

	unsigned char Dvbt2IQSwap; //LGE

	/*
	DVB-T2 PLP Parameters

	For single PLP T2 mode:
	u8DataPlpNumber == 1, PLP Index =0.
	the PLP_ID is default set by demodulator, usually used like DVB-T.

	For multi-PLP application:
	u8DataPlpNumber is the data PLP number. the maximum value is 255.
	PLP index is the index of PLP ID list start from 0.

	u8DataPlpIdArray contains all Data PLP ID.  common PLP is not listed in, as common PLP's data
	is processed by demodulator internally
	*/
	unsigned char u8DataPlpNumber;
	unsigned char u8DataPlpIdArray[255];

	/*
	When Single PLP used, DVB-T2  demodulator output  single PLP.
	When Multi-PLP used, DVB-T2 demodulator output only one active data PLP
	u8ActivePlpIdis the current received Data PLP's PLP ID
	*/
	unsigned char                                   u8ActivePlpId;
	RTK_A_DVBTX_DVBT2_PLP_TYPE                 Dvbt2DataPLPType;          /*Data PLP Type1 or Data PLP  Type2*/
	RTK_A_DVBTX_DVBT2_PLP_PAYLOAD_TYPE         Dvbt2PayloadType;	       /*Payload Type, TS*/
	RTK_A_DVBTX_DVBT2_CONSTELLATION_TYPE       Dvbt2Constellation;        /*Data PLP's constellation*/
	RTK_A_DVBTX_DVBT2_CODE_RATE_TYPE           Dvbt2CodeRate;             /*Data PLP's code rate*/
	unsigned char                                   u8ConstellationRotation;   /*1:constellation rotation used, 0: not used*/
	RTK_A_DVBTX_DVBT2_FEC_LENGTH_TYPE          Dvbt2FecLengthType;        /*Normal LDPC or short LDPC*/
	RTK_A_DVBTX_DVBT2_ISSY_TYPE                Dvbt2IssyType;             /*Data PLP's ISSY type*/
	RTK_A_DVBTX_DVBT2_STREAM_MODE_TYPE         Dvbt2StreamType;           /*Stream mode, HEM or normal mode*/
} RTK_A_DVBTX_DVBT2_PARAMS;



// DVB-T demod module pre-definition
typedef struct DVBTX_DEMOD_MODULE_TAG DVBTX_DEMOD_MODULE;

/**********************************************************************************************************************************
struct RTK_A_DVBTX_MPEGTSMode
@ui8TSTransferType: TS stream transfer type, can be set to parallel(8 bit data bus) or serial(1 bit data bus) mode
@ui8OutputEdge: demodulator will output TS data on this edge of TS stream clock
@ui8TSSPIMSBSelection: TS MSB on bit 0 or bit selection
@ui8TSSSIOutputSelection: TS output on bit 0 or bit 7 selection
@ui8SPIClockConstantOutput: TS stream clock can be set outputting all the time or only during TS data valid (188 bytes)
**********************************************************************************************************************************/
/*****************ui8TSTransferType Option Value***************************/
#define RTK_A_DVBTX_TS_PARALLEL_MODE             1
#define RTK_A_DVBTX_TS_SERIAL_MODE               0
/*****************ui8TSOnOff Option Value***************************/
#define RTK_A_DVBTX_TS_ON                        1
#define RTK_A_DVBTX_TS_OFF                       0
/**********************ui8OutputEdge Option Value***************************/
#define RTK_A_DVBTX_TS_OUTPUT_FALLING_EDGE       1
#define RTK_A_DVBTX_TS_OUTPUT_RISING_EDGE        0
/**********************ui8TSSPIMSBSelection Option Value******************/
#define RTK_A_DVBTX_TS_SPI_MSB_ON_DATA_BIT7      1
#define RTK_A_DVBTX_TS_SPI_MSB_ON_DATA_BIT0      0
/**********************ui8TSSSIOutputSelection Option Value***************/
#define RTK_A_DVBTX_TS_SSI_OUTPUT_ON_DATA_BIT7   1
#define RTK_A_DVBTX_TS_SSI_OUTPUT_ON_DATA_BIT0   0
/**********************ui8SPIClockConstantOutput Option Value*************/
#define RTK_A_DVBTX_TS_CLOCK_CONST_OUTPUT        1
#define RTK_A_DVBTX_TS_CLOCK_VALID_OUTPUT        0


typedef struct RTK_A_DVBTX_MPEG_TS_MODE_STRUCT {
	unsigned char ui8TSTransferType;
	unsigned char ui8OutputEdge;
	unsigned char ui8TSSPIMSBSelection;
	unsigned char ui8TSSSIOutputSelection;
	unsigned char ui8SPIClockConstantOutput;
	unsigned char ui8DVBTxCTSClockVariableMode;
} RTK_A_DVBTX_MPEGTSMode;

typedef struct RTK_A_DVBTX_TUNER_CONFIG_STRUCT {
	unsigned char   ui8IQSwapMode;	   /*IQ mode, DVB-T_T2 IQ auto detection*/
	unsigned char   ui8BandwidthType;  /*Bandwidth Type,eg.RTK_A_DVBTX_DVBT_BANDWIDTH_8MHZ, RTK_A_DVBTX_DVBT_BANDWIDTH_7MHZ etc*/
	unsigned int u32IFFrequencyKHz;  /*Tuner IF frequency for DVB-T,DVB-T2, Unit is KHz*/
	unsigned char  ui8TxIFAgcPolarity;
} RTK_A_DVBTX_TUNER_INPUT_T;

/**********************SWAP IQ or not, related with tuner's IF interface*************/
#define   RTK_A_DVBTX_INPUT_IQ_SWAP           1
#define   RTK_A_DVBTX_INPUT_IQ_NORMAL        0

/*******************IQ auto detection option, By default enable this function********/
#define  RTK_A_DVBTX_IQ_AUTO_ENABLE 1
#define  RTK_A_DVBTX_IQ_AUTO_DISABLE 0

/*******************Tuner's AGC polarity inverted or normal**************/
#define   RTK_A_DVBTX_IF_AGC_POLARITY_INVERTED      1
#define   RTK_A_DVBTX_IF_AGC_POLARITY_NORMAL        0



typedef struct RTK_A_DVBTX_CONFIG_STRUCT {
	/**************Common setting*******************************************/
	RTK_A_DVBTX_MPEGTSMode stMPEGTSMode;      /*TS interface configuration	*/
	unsigned int ui32DemodClkKHz;                /*Demodulator clock Select Unit is KHz, 24000 KHz*/

	unsigned char   gUseI2CRepeaterEnable;

	/************************mode specific  setting *****************/
	/*Set DVB-T(T2),  DVB-C, DVB-S, DVB-S2 mode*/
	unsigned char  ui8ConfigDVBType;    /*eg. RTK_A_DVBTX_DVBT_MODE(contains auto detection of DVB-T2), or  RTK_A_DVBTX_DVBC_MODE*/

	/*DVB-T,T2 setting parameters*/
	RTK_A_DVBTX_TUNER_INPUT_T DvbTxTunerInput; /*DVB-T and DVB-T2 only*/
} stRtkaDvbtxConfig;



/**

@brief   DVB-T demod type getting function pointer

One can use DVBTX_DEMOD_FP_GET_DEMOD_TYPE() to get DVB-T demod type.


@param [in]    pDemod       The demod module pointer
@param [out]   pDemodType   Pointer to an allocated memory for storing demod type


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_DEMOD_TYPE() with the corresponding function.


@see   MODULE_TYPE

*/
typedef void
(*DVBTX_DEMOD_FP_GET_DEMOD_TYPE)(
	DVBTX_DEMOD_MODULE *pDemod,
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
(*DVBTX_DEMOD_FP_GET_CRYSTAL_FREQ_HZ)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pCrystalFreqHz
);



/**

@brief   DVB-T demod initializing function pointer

One can use DVBTX_DEMOD_FP_INITIALIZE() to initialie DVB-T demod.


@param [in]   pDemod   The demod module pointer


@retval   FUNCTION_SUCCESS   Initialize demod successfully.
@retval   FUNCTION_ERROR     Initialize demod unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_INITIALIZE() with the corresponding function.

*/
typedef int
(*DVBTX_DEMOD_FP_INITIALIZE)(
	DVBTX_DEMOD_MODULE *pDemod
);





/**

@brief   DVB-T demod bandwidth mode setting function pointer

One can use DVBTX_DEMOD_FP_SET_BANDWIDTH_MODE() to set DVB-T demod bandwidth mode.


@param [in]   pDemod	      The demod module pointer
@param [in]   BandwidthMode   Bandwidth mode for setting


@retval   FUNCTION_SUCCESS   Set demod bandwidth mode successfully.
@retval   FUNCTION_ERROR     Set demod bandwidth mode unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_SET_DVBTX_MODE() with the corresponding function.


@see   DVBTX_BANDWIDTH_MODE

*/
typedef int
(*DVBTX_DEMOD_FP_SET_BANDWIDTH_MODE)(
	DVBTX_DEMOD_MODULE *pDemod,
	int BandwidthMode
);



/**

@brief   DVB-T demod TS interface mode setting function pointer

One can use DVBTX_DEMOD_FP_SET_TS_INTERFACE_MODE() to set DVB-T demod TS interface mode.


@param [in]   pDemod            The demod module pointer
@param [in]   TsInterfaceMode   TS interface mode for setting


@retval   FUNCTION_SUCCESS   Set demod TS interface mode successfully.
@retval   FUNCTION_ERROR     Set demod TS interface mode unsuccessfully.


@note
	-# DVBTX_DEMOD_FP_SET_TS_INTERFACE_MODE() corresponding function is private.
	-# DVBTX_DEMOD_FP_SET_TS_INTERFACE_MODE() does not appear in demod module structure.


@see   TS_INTERFACE_MODE

*/
typedef int
(*DVBTX_DEMOD_FP_SET_TS_INTERFACE_MODE)(
	DVBTX_DEMOD_MODULE *pDemod,
	RTK_A_DVBTX_MPEGTSMode stTSMode
);


typedef int
(*DVBTX_DEMOD_FP_SET_TS_OUTPUTONOFF)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char ui8TSOnOff
);


/**

@brief   DVB-T demod IF frequency setting function pointer

One can use DVBTX_DEMOD_FP_SET_IF_FREQ_HZ() to set DVB-T demod IF frequency in Hz.


@param [in]   pDemod     The demod module pointer
@param [in]   IfFreqHz   IF frequency in Hz for setting


@retval   FUNCTION_SUCCESS   Set demod IF frequency successfully.
@retval   FUNCTION_ERROR     Set demod IF frequency unsuccessfully.


@note
	-# DVBTX_DEMOD_FP_SET_IF_FREQ_HZ() corresponding function is private.
	-# DVBTX_DEMOD_FP_SET_IF_FREQ_HZ() does not appear in demod module structure.


@see   IF_FREQ_HZ

*/
typedef int
(*DVBTX_DEMOD_FP_SET_IF_FREQ_HZ)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int IfFreqHz
);



/**

@brief   DVB-T demod spectrum mode setting function pointer

One can use DVBTX_DEMOD_FP_SET_SPECTRUM_MODE() to set DVB-T demod spectrum mode.


@param [in]   pDemod         The demod module pointer
@param [in]   SpectrumMode   Spectrum mode for setting


@retval   FUNCTION_SUCCESS   Set demod spectrum mode successfully.
@retval   FUNCTION_ERROR     Set demod spectrum mode unsuccessfully.


@note
	-# DVBTX_DEMOD_FP_SET_SPECTRUM_MODE() corresponding function is private.
	-# DVBTX_DEMOD_FP_SET_SPECTRUM_MODE() does not appear in demod module structure.


@see   SPECTRUM_MODE

*/
typedef int
(*DVBTX_DEMOD_FP_SET_SPECTRUM_MODE)(
	DVBTX_DEMOD_MODULE *pDemod,
	int SpectrumMode
);



/**

@brief   DVB-T demod bandwidth mode getting function pointer

One can use DVBTX_DEMOD_FP_GET_BANDWIDTH_MODE() to get DVB-T demod bandwidth mode.


@param [in]    pDemod           The demod module pointer
@param [out]   pBandwidthMode   Pointer to an allocated memory for storing demod bandwidth mode


@retval   FUNCTION_SUCCESS   Get demod bandwidth mode successfully.
@retval   FUNCTION_ERROR     Get demod bandwidth mode unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_BANDWIDTH_MODE() with the corresponding function.


@see   DVBTX_DVBTX_MODE

*/
typedef int
(*DVBTX_DEMOD_FP_GET_BANDWIDTH_MODE)(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pBandwidthMode
);




/**

@brief   DVB-T demod signal lock asking function pointer

One can use DVBTX_DEMOD_FP_IS_SIGNAL_LOCKED() to ask DVB-T demod if it is signal-locked.


@param [in]    pDemod    The demod module pointer
@param [out]   pAnswer   Pointer to an allocated memory for storing answer


@retval   FUNCTION_SUCCESS   Perform signal lock asking to demod successfully.
@retval   FUNCTION_ERROR     Perform signal lock asking to demod unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_IS_SIGNAL_LOCKED() with the corresponding function.

*/
typedef int
(*DVBTX_DEMOD_FP_IS_SIGNAL_LOCKED)(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
);


typedef int
(*DVBTX_DEMOD_FP_IS_SIGNAL_SYNCLOCKED)(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
);


typedef int
(*DVBTX_DEMOD_FP_GET_SIGNAL_PAEAMSLOCKFLAG)(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
);



/**

@brief   DVB-T demod signal strength getting function pointer

One can use DVBTX_DEMOD_FP_GET_SIGNAL_STRENGTH() to get signal strength.


@param [in]    pDemod            The demod module pointer
@param [out]   pSignalStrength   Pointer to an allocated memory for storing signal strength (0 ~ 100)


@retval   FUNCTION_SUCCESS   Get demod signal strength successfully.
@retval   FUNCTION_ERROR     Get demod signal strength unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_SIGNAL_STRENGTH() with the corresponding function.

*/
typedef int
(*DVBTX_DEMOD_FP_GET_SIGNAL_STRENGTH)(
	DVBTX_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	unsigned int *pSignalStrength
);





/**

@brief   DVB-T demod layer signal quality getting function pointer

One can use DVBTX_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY() to get layer signal quality.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerSignalQuality      Pointer to an allocated memory for storing layer signal quality (0 ~ 100)


@retval   FUNCTION_SUCCESS   Get demod layer signal quality successfully.
@retval   FUNCTION_ERROR     Get demod layer signal quality unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY() with the corresponding function.


@see   DVBTX_HIERARCHICAL_LAYER_INDEX

*/
typedef int
(*DVBTX_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pSignalQuality
);





/**

@brief   DVB-T demod layer BER getting function pointer

One can use DVBTX_DEMOD_FP_GET_LAYER_BER() to get layer BER.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerBerNum             Pointer to an allocated memory for storing layer BER numerator
@param [out]   pLayerBerDen             Pointer to an allocated memory for storing layer BER denominator


@retval   FUNCTION_SUCCESS   Get demod layer BER successfully.
@retval   FUNCTION_ERROR     Get demod layer BER unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_LAYER_BER() with the corresponding function.

*/
typedef int
(*DVBTX_DEMOD_FP_GET_BER)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *ui32BERNum,
	unsigned int *ui32BERDen
);





/**

@brief   DVB-T demod layer PER getting function pointer

One can use DVBTX_DEMOD_FP_GET_LAYER_PER() to get layer PER.


@param [in]    pDemod                   The demod module pointer
@param [in]    HierarchicalLayerIndex   Hierarchical layer index for getting
@param [out]   pLayerPerNum             Pointer to an allocated memory for storing layer PER numerator
@param [out]   pLayerPerDen             Pointer to an allocated memory for storing layer PER denominator


@retval   FUNCTION_SUCCESS   Get demod layer PER successfully.
@retval   FUNCTION_ERROR     Get demod layer PER unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_LAYER_PER() with the corresponding function.

*/
typedef int
(*DVBTX_DEMOD_FP_GET_PER)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *ui32PEC_NUM,
	unsigned int *ui32PEC_DEN
);





/**

@brief   DVB-T demod SNR getting function pointer

One can use DVBTX_DEMOD_FP_GET_SNR_DB() to get SNR in dB.


@param [in]    pDemod      The demod module pointer
@param [out]   pSnrDbNum   Pointer to an allocated memory for storing SNR dB numerator
@param [out]   pSnrDbDen   Pointer to an allocated memory for storing SNR dB denominator


@retval   FUNCTION_SUCCESS   Get demod SNR successfully.
@retval   FUNCTION_ERROR     Get demod SNR unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_SNR_DB() with the corresponding function.

*/
typedef int
(*DVBTX_DEMOD_FP_GET_SNR_DB)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *ui32SNRValuex10
);



typedef int
(*DVBTX_DEMOD_FP_SET_AGC_POL)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char agcpola
);


/**

@brief   DVB-T demod IF AGC getting function pointer

One can use DVBTX_DEMOD_FP_GET_IF_AGC() to get DVB-T demod IF AGC value.


@param [in]    pDemod   The demod module pointer
@param [out]   pIfAgc   Pointer to an allocated memory for storing IF AGC value


@retval   FUNCTION_SUCCESS   Get demod IF AGC value successfully.
@retval   FUNCTION_ERROR     Get demod IF AGC value unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_IF_AGC() with the corresponding function.

*/
typedef int
(*DVBTX_DEMOD_FP_GET_IF_AGC)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pIfAgc
);


/**

@brief   DVB-T demod TR offset getting function pointer

One can use DVBTX_DEMOD_FP_GET_TR_OFFSET_PPM() to get TR offset in ppm.


@param [in]    pDemod         The demod module pointer
@param [out]   pTrOffsetPpm   Pointer to an allocated memory for storing TR offset in ppm


@retval   FUNCTION_SUCCESS   Get demod TR offset successfully.
@retval   FUNCTION_ERROR     Get demod TR offset unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_TR_OFFSET_PPM() with the corresponding function.

*/
typedef int
(*DVBTX_DEMOD_FP_GET_TR_OFFSET_PPM)(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pTrOffsetPpm
);





/**

@brief   DVB-T demod CR offset getting function pointer

One can use DVBTX_DEMOD_FP_GET_CR_OFFSET_HZ() to get CR offset in Hz.


@param [in]    pDemod        The demod module pointer
@param [out]   pCrOffsetHz   Pointer to an allocated memory for storing CR offset in Hz


@retval   FUNCTION_SUCCESS   Get demod CR offset successfully.
@retval   FUNCTION_ERROR     Get demod CR offset unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_GET_CR_OFFSET_HZ() with the corresponding function.

*/
typedef int
(*DVBTX_DEMOD_FP_GET_CR_OFFSET_HZ)(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pCrOffsetHz
);





typedef int
(*DVBTX_DEMOD_FP_GET_CELL_ID)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pCellId
);



/**

@brief   DVB-T demod hierarchy layer setting function pointer

One can use DVBTX_DEMOD_FP_SET_HIERARCHY() to set DVB-T demod hierarchy mode.


@param [in]    pDemod       The demod module pointer
@param [out]   Hierarchy    demod hierarchy mode


@retval   FUNCTION_SUCCESS   Get demod hierarchy mode successfully.
@retval   FUNCTION_ERROR     Get demod hierarchy mode unsuccessfully.


@note
	-# Demod building function will set DVBTX_DEMOD_FP_SET_HIERARCHY() with the corresponding function.


@see   DVBTX_HIERARCHY_MODE

*/
typedef int
(*DVBTX_DEMOD_FP_SET_HIERARCHY)(
	DVBTX_DEMOD_MODULE *pDemod,
	int Hierarchy
);


typedef int
(*DVBTX_DEMOD_FP_SET_T2PLPID)(
	DVBTX_DEMOD_MODULE *pDemod,
	int T2PlpID
);


typedef int
(*DVBTX_DEMOD_FP_GET_TXMODE)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char * pu8DVBTxMode
);


typedef int
(*DVBTX_DEMOD_FP_GET_SIGANLPARAMST2)(
	DVBTX_DEMOD_MODULE *pDemod,
	RTK_A_DVBTX_DVBT2_PARAMS *pDvbT2Params
);


typedef int
(*DVBTX_DEMOD_FP_GET_SIGANLPARAMST)(
	DVBTX_DEMOD_MODULE *pDemod,
	RTK_A_DVBTX_DVBT_PARAMS *pDvbTParams
);


typedef int
(*DVBTX_DEMOD_FP_GET_TS_THROUGHPUT)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *TSThroughputBps
);

typedef int
(*DVBTX_DEMOD_FP_SET_CELLID_CHECK_ON)(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char ui8CellIDOnOff
);

typedef int
(*DVBTX_DEMOD_FP_MONITOR_T2_TXID)(
	DVBTX_DEMOD_MODULE *pDemod
);

typedef int
(*DVBTX_DEMOD_FP_MONITOR_T2_DYSTATICCH)(
	DVBTX_DEMOD_MODULE *pDemod
);

typedef int
(*DVBTX_DEMOD_FP_MONITOR_T2_CHECKBYSTATICPWRCHANGE)(
	DVBTX_DEMOD_MODULE *pDemod
);


/// RTD299S DVB-T extra module
typedef struct RTK_A_DVBTX_EXTRA_MODULE_TAG RTK_A_DVBTX_EXTRA_MODULE;

struct RTK_A_DVBTX_EXTRA_MODULE_TAG {
	// RTD299S DVB-T extra variables
	unsigned int ShareMemoryOfst;          // Start address of share memory
	unsigned char NoUse;					// No use, for extra module architecture completion only.
};





/// DVB-T demod module structure
struct DVBTX_DEMOD_MODULE_TAG {
	// Private variables
	int           DemodType;
	unsigned int CrystalFreqHz;
	int           TsInterfaceMode;
	unsigned int IfFreqHz;
	int           SpectrumMode;


	union {										///<   Demod extra module used by driving module
		RTK_A_DVBTX_EXTRA_MODULE RtkaDvbtx;
	}
	Extra;

	BASE_INTERFACE_MODULE *pBaseInterface;
	I2C_BRIDGE_MODULE *pI2cBridge;


	// Demod manipulating function pointers
	DVBTX_DEMOD_FP_GET_DEMOD_TYPE               GetDemodType;
	DVBTX_DEMOD_FP_GET_CRYSTAL_FREQ_HZ          GetCrystalFreqHz;

	DVBTX_DEMOD_FP_INITIALIZE                   Initialize;
	DVBTX_DEMOD_FP_SET_BANDWIDTH_MODE           SetBandwidthMode;

	DVBTX_DEMOD_FP_GET_BANDWIDTH_MODE           GetBandwidthMode;

	DVBTX_DEMOD_FP_IS_SIGNAL_LOCKED             IsSignalLocked;

	DVBTX_DEMOD_FP_IS_SIGNAL_SYNCLOCKED    IsSignalSyncLocked;

	DVBTX_DEMOD_FP_IS_SIGNAL_SYNCLOCKED    IsDvbtSyncLocked;

	DVBTX_DEMOD_FP_IS_SIGNAL_SYNCLOCKED    IsDvbt2SyncLocked;

	DVBTX_DEMOD_FP_IS_SIGNAL_SYNCLOCKED    IsDvbt2L1Locked;

	DVBTX_DEMOD_FP_GET_SIGNAL_PAEAMSLOCKFLAG    GetSignalParamsLockFlag;

	DVBTX_DEMOD_FP_GET_SIGNAL_STRENGTH          GetSignalStrength;

	DVBTX_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY     GetLayerSignalQuality;

	DVBTX_DEMOD_FP_GET_BER                GetBer;
	DVBTX_DEMOD_FP_GET_PER                GetPer;
	DVBTX_DEMOD_FP_GET_SNR_DB                   GetSnrDb;

	DVBTX_DEMOD_FP_SET_AGC_POL    SetAgcPol;

	DVBTX_DEMOD_FP_GET_IF_AGC                   GetIfAgc;

	DVBTX_DEMOD_FP_GET_TR_OFFSET_PPM            GetTrOffsetPpm;
	DVBTX_DEMOD_FP_GET_CR_OFFSET_HZ             GetCrOffsetHz;

	DVBTX_DEMOD_FP_GET_CELL_ID                 GetCellId;


	// 20120207 - kevin_wang add
	DVBTX_DEMOD_FP_SET_TS_INTERFACE_MODE        SetTsInterfaceMode;
	DVBTX_DEMOD_FP_SET_TS_OUTPUTONOFF        SetTSOutputOnOff;

	DVBTX_DEMOD_FP_SET_IF_FREQ_HZ               SetIfFreqHz;
	DVBTX_DEMOD_FP_SET_SPECTRUM_MODE            SetSpectrumMode;
	DVBTX_DEMOD_FP_SET_HIERARCHY                SetTHierarchy;
	DVBTX_DEMOD_FP_SET_T2PLPID    SetT2PlpID;

	DVBTX_DEMOD_FP_GET_TXMODE     GetDVBTxMode;
	DVBTX_DEMOD_FP_GET_SIGANLPARAMST2  GetSignalParamsT2;
	DVBTX_DEMOD_FP_GET_SIGANLPARAMST  GetSignalParamsT;
	DVBTX_DEMOD_FP_GET_TS_THROUGHPUT  GetTsThroughput;
	DVBTX_DEMOD_FP_SET_CELLID_CHECK_ON  SetCellIdCheckOn;
	DVBTX_DEMOD_FP_MONITOR_T2_TXID  MonitorT2TxID;
	DVBTX_DEMOD_FP_MONITOR_T2_DYSTATICCH  MonitorT2DyStaticCH;
    DVBTX_DEMOD_FP_MONITOR_T2_CHECKBYSTATICPWRCHANGE  MonitorT2CheckbyStaicChPwrChange;

};







int
dvbt_demod_default_GetBandwidthMode(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pBandwidthMode
);



RTK_A_DVBTX_STATUS   RTK_A_DVBTX_Read(DVBTX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char *ui8RegValue);
RTK_A_DVBTX_STATUS   RTK_A_DVBTX_Write(DVBTX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char ui8data);
RTK_A_DVBTX_STATUS  RTK_A_DVBTX_WriteRegArray(DVBTX_DEMOD_MODULE *pDemod, unsigned char *ui8ARegTable, int i32TableLen);
void RTK_A_DVBTX_64Mult(RTK_A_DVBTX_64Data *pstRst, unsigned int m1, unsigned int m2);
unsigned int RTK_A_DVBTX_64Div(RTK_A_DVBTX_64Data stDivisor, RTK_A_DVBTX_64Data stDividend);
RTK_A_DVBTX_64Data RTK_A_DVBTX_64DivReturn64(RTK_A_DVBTX_64Data stDivisor, RTK_A_DVBTX_64Data stDividend);
unsigned int RTK_A_DVBTX_Log10ConvertdBx10(unsigned int i32InValue);
unsigned int RTK_A_DVBTX_Log10ConvertdB(unsigned int i32InValue);
unsigned int  RTK_A_DVBTX_FixPow(unsigned int x, unsigned int y);
unsigned char  RTK_A_DVBTX_64ComPare(RTK_A_DVBTX_64Data stPara, RTK_A_DVBTX_64Data stPara1);
void  RTK_A_DVBTX_MiddleValueu8(unsigned char u8ValueA, unsigned char u8ValueB, unsigned char u8ValueC, unsigned char *MidValue);
RTK_A_DVBTX_STATUS   RTK_A_DVBTX_I2CRead(DVBTX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char *ui8RegValue);
RTK_A_DVBTX_STATUS   RTK_A_DVBTX_I2CWrite(DVBTX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char ui8data);


#define RTK_DEMOD_DVBTX_CALLER(fmt, args...)     rtd_demod_print(KERN_DEBUG,"CALLER, " fmt, ##args);
#define RTK_DEMOD_DVBTX_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"Debug, " fmt, ##args);
#define RTK_DEMOD_DVBTX_INFO(fmt, args...)       rtd_demod_print(KERN_INFO,"Info, " fmt, ##args);
#define RTK_DEMOD_DVBTX_WARNING(fmt, args...)    rtd_demod_print(KERN_WARNING,"Warning, " fmt, ##args);




#endif
