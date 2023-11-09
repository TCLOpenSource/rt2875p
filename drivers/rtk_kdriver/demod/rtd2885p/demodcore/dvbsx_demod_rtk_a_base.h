/***************************************************************************** 
Copyright 2018, Realtek Inc. All rights reserved.
File Name: common_demod_rtk.h
This is header file for RTK DEMOD SDK internal use, common functions,common data struct,etc.
******************************************************************************/
#ifndef _DVBSX_DEMOD_REALTEK_A_BASE_H
#define _DVBSX_DEMOD_REALTEK_A_BASE_H

#include "tv_osal.h"
#include "foundation.h"

#define RTK_A_DVBSX_AbsSub(x, y) ((x >= y)? (x - y):(y - x))

/*Default small window*/
#define DVBS2_BCH_H 0x00
#define DVBS2_BCH_L 0x20

typedef int BOOL;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//DiSEqC
typedef int
(*DVBS2_DEMOD_FP_SET_DISEQC_CONT_22K_ON_OFF)(
	unsigned int Diseqc22kOnOff
);

typedef int
(*DVBS2_DEMOD_FP_SET_DISEQC_CMD)(
	unsigned int DataLength,
	unsigned char *Data
);

typedef int
(*DVBS2_DEMOD_FP_SET_DISEQC_TONE_BURST)(
	DISEQC_TONE_MODE mode
);

typedef int
(*DVBS2_DEMOD_FP_SET_DISEQC_UN_MOD_TONE_BURST)(
	void
);

typedef int
(*DVBS2_DEMOD_FP_SET_DISEQC_MOD_TONE_BURST)(
	void
);

// DVBS2 Demod Module Pre-Definition
typedef struct DVBSX_DEMOD_MODULE_TAG DVBSX_DEMOD_MODULE;

// DVBS2 Demod Module Structure
struct DVBSX_DEMOD_MODULE_TAG {
	// Private Variables
	int           DemodType;
	unsigned char DeviceAddr;
	U32BITS CrystalFreqHz;
	int           TsInterfaceMode;
	unsigned short  CurrentTunerId;

	BASE_INTERFACE_MODULE *pBaseInterface;
	I2C_BRIDGE_MODULE *pI2cBridge;
	
	DVBS2_DEMOD_FP_SET_DISEQC_CONT_22K_ON_OFF           SetDiseqcContinue22kOnOff;
	DVBS2_DEMOD_FP_SET_DISEQC_CMD                       SetDiseqcCmd;
	DVBS2_DEMOD_FP_SET_DISEQC_TONE_BURST                SetDiseqcToneBurst;
	DVBS2_DEMOD_FP_SET_DISEQC_UN_MOD_TONE_BURST         SetDiseqcUnModToneBurst;
	DVBS2_DEMOD_FP_SET_DISEQC_MOD_TONE_BURST            SetDiseqcModToneBurst;
};

typedef unsigned char RTK_A_DVBSX_STATUS;

#define RTK_A_DVBSX_ADC_CLK_KHz             121500
#define RTK_A_DVBSX_LOW_SR_ADC_CLK_KHz      RTK_A_DVBSX_ADC_CLK_KHz/2

/*discriminate chip version, define for new chip version */
#define RTK_A_DEMOD_ADAPTIVE_TS_ENABLE      1   /*Set 1 to enable adaptive TS rate(Variable TS clock)*/
#define RTK_A_DEMOD_TS_CLOCK_VARIABLE       1
#define RTK_A_DEMOD_TS_CLOCK_FREQ_FIXED     0

/**********************Chip Support receiving Mode Macro*******************************************************/
//#define RTK_DEMOD_DVBC_MODE                 0    
//#define RTK_DEMOD_RSV_MODE                  1     /*Reserved*/
//#define RTK_DEMOD_DVBT_MODE                 2     /*DVB-T and DVB-T2, The register setting is not different*/
//#define RTK_DEMOD_DVBT2_MODE                3  
#define RTK_DEMOD_DVBS_MODE                 4  
#define RTK_DEMOD_DVBS2_MODE                5  
#define RTK_DEMOD_DVBS2_OPTIONAL_MODE       6
#define RTK_DEMOD_DVBS2X_MODE               7
//#define RTK_DEMOD_DVBC2_MODE                8
#define RTK_DEMOD_DVB_UNKNOWN               255

/***********************I2C Read/Write Return Status ****************************************/
#define RTK_DEMOD_SUCCESSFUL                0     /* No error */
#define RTK_DEMOD_FAILED                    1     /* General error, usually RTK Demod APIs not correctly used or DTV system error */
#define RTK_DEMOD_I2C_ERROR                 2     /*General I2C error,usually from I2C interface */
#define RTK_DEMOD_NO_ACKNOWLEDGE            4     /* No acknowledge from I2C bus */
#define RTK_DEMOD_NO_SLAVE_ACK              8     /* No acknowledge from devices on the GPIO I2C bus */

/**********************************************************************************************************************************
struct RtkDemod_MPEGTSMode 
@ui8TSTransferType: TS stream transfer type, can be set to parallel(8 bit data bus) or serial(1 bit data bus) mode
@ui8OutputEdge: demodulator will output TS data on this edge of TS stream clock
@ui8TSSPIMSBSelection: TS MSB on bit 0 or bit selection
@ui8TSSSIOutputSelection: TS output on bit 0 or bit 7 selection
@ui8SPIClockConstantOutput: TS stream clock can be set outputting all the time or only during TS data valid (188 bytes) 
**********************************************************************************************************************************/
/*****************ui8TSTransferType Option Value***************************/
#define RTK_DEMOD_TS_PARALLEL_MODE             1
#define RTK_DEMOD_TS_SERIAL_MODE               0
/*****************ui8TSOnOff Option Value***************************/
#define RTK_DEMOD_TS_ON                        1
#define RTK_DEMOD_TS_OFF                       0
/**********************ui8OutputEdge Option Value***************************/
#define RTK_DEMOD_TS_OUTPUT_FALLING_EDGE       1
#define RTK_DEMOD_TS_OUTPUT_RISING_EDGE        0
/**********************ui8TSSPIMSBSelection Option Value******************/
#define RTK_DEMOD_TS_SPI_MSB_ON_DATA_BIT7      1
#define RTK_DEMOD_TS_SPI_MSB_ON_DATA_BIT0      0
/**********************ui8TSSSIOutputSelection Option Value***************/
#define RTK_DEMOD_TS_SSI_OUTPUT_ON_DATA_BIT7   1
#define RTK_DEMOD_TS_SSI_OUTPUT_ON_DATA_BIT0   0
/**********************ui8SPIClockConstantOutput Option Value*************/
#define RTK_DEMOD_TS_CLOCK_CONST_OUTPUT        1
#define RTK_DEMOD_TS_CLOCK_VALID_OUTPUT        0

typedef struct RTK_DEMOD_MPEG_TS_MODE_STRUCT
{
	unsigned char ui8TSTransferType;
	unsigned char ui8OutputEdge;
	unsigned char ui8TSSPIMSBSelection;
	unsigned char ui8TSSSIOutputSelection;
	unsigned char ui8SPIClockConstantOutput;
	unsigned char ui8DVBTxCTSClockVariableMode;
	unsigned char ui8DVBSxTSClockVariableMode;
}RTK_DEMOD_MPEGTSMode;

/**********************SWAP IQ or not, related with tuner's IF interface*************/
#define   RTK_DEMOD_INPUT_IQ_SWAP              1
#define   RTK_DEMOD_INPUT_IQ_NORMAL            0

/*******************IQ auto detection option, By default enable this function********/
#define  RTK_DEMOD_IQ_AUTO_ENABLE              1
#define  RTK_DEMOD_IQ_AUTO_DISABLE             0

/*******************Tuner's AGC polarity inverted or normal**************/
#define   RTK_DEMOD_IF_AGC_POLARITY_INVERTED   1
#define   RTK_DEMOD_IF_AGC_POLARITY_NORMAL     0

typedef struct RTK_DEMOD_DVBS_TUNER_CONFIG_STRUCT
{
	unsigned char	ui8IQSwapMode;      /*IQ mode, Should be correctly set, refer to RTK_I_Demod_PowerOnInit*/
	unsigned char	ui8SxIFAgcPolarity;
	unsigned char	ui8IQAutoDetection; /*IQ auto swap option*/
	unsigned int	ui32SymbolRateHz;    /*DVB-S/S2 symbol rate, not used.when symbol rate auto detection enabled.*/
	unsigned int	ui32FreqKhz;
}RTK_DEMOD_DVBS_TUNER_INPUT_T;

typedef struct RTK_DEMOD_DVB_CONFIG_STRUCT
{
	/**************Common setting*******************************************/
	RTK_DEMOD_MPEGTSMode stMPEGTSMode;      /*TS interface configuration */
	unsigned int ui32DemodClkKHz;                /*Demodulator clock Select Unit is KHz, 24000 KHz*/

	/************************mode specific  setting *****************/
	/*Set DVB-T(T2),  DVB-C, DVB-S, DVB-S2 mode*/
	unsigned char  ui8ConfigDVBType;    /*eg. RTK_DEMOD_DVBT_MODE(contains auto detection of DVB-T2), or  RTK_DEMOD_DVBC_MODE*/

	/*DVB-S/S2 setting parameters*/
	RTK_DEMOD_DVBS_TUNER_INPUT_T DvbSxTunerInput;
	unsigned int ui32DVBSAdcClkKHz;
}stRtkDemodDvbConfig;

/*DVBS, DVB-S2 constellation*/
typedef enum 
{
	RTK_DVBSx_MODU_QPSK = 0,
	RTK_DVBSx_MODU_8PSK,
	RTK_DVBSx_MODU_8PSKL,
	RTK_DVBSx_MODU_16APSK,
	RTK_DVBSx_MODU_16APSKL,
	RTK_DVBSx_MODU_32APSK,
	RTK_DVBSx_MODU_32APSKL,
	RTK_DVBSx_MODU_UNKNOWN
}RTK_DEMOD_DVBS_S2_CONSTELLATION_TYPE;

/*RTK_DVBS_CODE_RATE_TYPE is DVB-S code rate type.*/
typedef enum
{
	RTK_DVBS_CODE_RATE_1_2 = 0, /*Code rate 1/2*/
	RTK_DVBS_CODE_RATE_2_3 = 1, /*Code rate 2/3*/
	RTK_DVBS_CODE_RATE_3_4 = 2, /*Code rate 3/4*/
	RTK_DVBS_CODE_RATE_5_6 = 3, /*Code rate 5/6*/
	RTK_DVBS_CODE_RATE_7_8 = 4, /*Code rate 7/8*/
	RTK_DVBS_CODE_RATE_UNKNOWN
}RTK_DVBS_CODE_RATE_TYPE;

/*RTK_DVBS2_CODE_RATE_TYPE is DVB-S2 code rate type.*/
typedef enum
{
	RTK_DVBS2_CODE_RATE_DUMMY = 0,
	RTK_DVBS2_CODE_RATE_1_4 = 1,
	RTK_DVBS2_CODE_RATE_1_3 = 2,
	RTK_DVBS2_CODE_RATE_2_5 = 3,
	RTK_DVBS2_CODE_RATE_1_2 = 4,
	RTK_DVBS2_CODE_RATE_3_5 = 5,
	RTK_DVBS2_CODE_RATE_2_3 = 6,
	RTK_DVBS2_CODE_RATE_3_4 = 7,
	RTK_DVBS2_CODE_RATE_4_5 = 8,
	RTK_DVBS2_CODE_RATE_5_6 = 9,
	RTK_DVBS2_CODE_RATE_8_9 = 10,
	RTK_DVBS2_CODE_RATE_9_10 = 11,
	RTK_DVBS2_CODE_RATE_UNKNOWN
}RTK_DVBS2_CODE_RATE_TYPE;

/*RTK_DVBS2X_CODE_RATE_TYPE is DVB-S2X code rate type.*/
typedef enum
{
	RTK_DVBS2X_CODE_RATE_DUMMY = 0,
	RTK_DVBS2X_CODE_RATE_3_5 = 1,
	RTK_DVBS2X_CODE_RATE_2_3 = 2,
	RTK_DVBS2X_CODE_RATE_13_45 = 3,
	RTK_DVBS2X_CODE_RATE_9_20 = 4,
	RTK_DVBS2X_CODE_RATE_11_20 = 5,
	RTK_DVBS2X_CODE_RATE_26_45 = 6,
	RTK_DVBS2X_CODE_RATE_28_45 = 7,
	RTK_DVBS2X_CODE_RATE_23_36 = 8,
	RTK_DVBS2X_CODE_RATE_25_36 = 9,
	RTK_DVBS2X_CODE_RATE_13_18 = 10,
	RTK_DVBS2X_CODE_RATE_7_9 = 11,
	RTK_DVBS2X_CODE_RATE_90_180 = 12,
	RTK_DVBS2X_CODE_RATE_96_180 = 13,
	RTK_DVBS2X_CODE_RATE_100_180 = 14,
	RTK_DVBS2X_CODE_RATE_104_180 = 15,
	RTK_DVBS2X_CODE_RATE_116_180 = 16,
	RTK_DVBS2X_CODE_RATE_124_180 = 17,
	RTK_DVBS2X_CODE_RATE_128_180 = 18,
	RTK_DVBS2X_CODE_RATE_132_180 = 19,
	RTK_DVBS2X_CODE_RATE_135_180 = 20,
	RTK_DVBS2X_CODE_RATE_140_180 = 21,
	RTK_DVBS2X_CODE_RATE_144_180 = 22,
	RTK_DVBS2X_CODE_RATE_150_180 = 23,
	RTK_DVBS2X_CODE_RATE_154_180 = 24,
	RTK_DVBS2X_CODE_RATE_18_30 = 25,
	RTK_DVBS2X_CODE_RATE_20_30 = 26,
	RTK_DVBS2X_CODE_RATE_22_30 = 27,
	RTK_DVBS2X_CODE_RATE_UNKNOWN
}RTK_DVBS2X_CODE_RATE_TYPE;

/*RTK_DVBS2_ROLL_OFF_TYPE is DVB-S2 roll off type.*/
typedef enum
{
	RTK_DVBS2_ROLL_OFF_0p05 = 0,
	RTK_DVBS2_ROLL_OFF_0p10,
	RTK_DVBS2_ROLL_OFF_0p15,
	RTK_DVBS2_ROLL_OFF_0p20,
	RTK_DVBS2_ROLL_OFF_0p25,
	RTK_DVBS2_ROLL_OFF_0p35,
	RTK_DVBS2_ROLL_OFF_UNKNOWN
}RTK_DVBS2_ROLL_OFF_TYPE;

/*DVB-S/S2/S2x spectrum.*/
typedef enum
{
	RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL = 0,
	RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE = 1,
	RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN
}RTK_DVBSx_SIGNAL_SPECTRUM;

typedef struct DVBSX_LDPC_ITERATION_TABLE_STRUCT
{
	RTK_DEMOD_DVBS_S2_CONSTELLATION_TYPE DVBSX_Modulation;
	RTK_DVBS2_CODE_RATE_TYPE DVBS2_CodeRate;
	RTK_DVBS2X_CODE_RATE_TYPE DVBS2X_CodeRate;
	unsigned int SnrThreshold;
	unsigned int DefaultBaseIteration;
}DVBSX_LDPC_ITERATION_TABLE;

/*DVB-S2 constellation and code rate*/
typedef struct RTK_DEMOD_DVBS2_MODU_CR_STRUCT
{
	RTK_DVBS2_CODE_RATE_TYPE           DVBS2CodeRate; /*DVB-S2 code rate*/
}RTK_DVBS2_MODU_CR;

/*DVB-S2X constellation and code rate*/
typedef struct RTK_DEMOD_DVBS2X_MODU_CR_STRUCT
{
	RTK_DVBS2X_CODE_RATE_TYPE           DVBS2XCodeRate; /*DVB-S2X code rate*/
}RTK_DVBS2X_MODU_CR;

/*DVB-S S2 S2X signal parameters*/
typedef struct RTK_DEMOD_DVBSx_SIGNAL_PARAMS_STRUCT
{
	RTK_DEMOD_DVBS_S2_CONSTELLATION_TYPE DVBSxModu;        /*Constellation*/
	unsigned char                               DVBSxType;        /*RTK_DVBS_CODE_RATE or RTK_DEMOD_DVBS2_MODE*/
	RTK_DVBS_CODE_RATE_TYPE        DVBSCodeRate;     /*Code rate of DVB-S*/
	RTK_DVBS2_CODE_RATE_TYPE       DVBS2CodeRate;    /*Code rate of DVB-S2*/
	RTK_DVBS2X_CODE_RATE_TYPE      DVBS2XCodeRate;   /*Code rate of DVB-S2x*/
	unsigned int                              DVBSxSymbolRateHz; /*Symbol rate*/
	unsigned char                               u8DVBSxISINumber;
	unsigned char                               u8DVBSxIsiIdList[8];
}RTK_DVBSx_SIGNAL_PARAMS;

/*****************************DVBS S2 Blind scan used*************************************/
typedef struct DvbSxChannel
{
	unsigned int u32FrequencyKHz; /*Center frequency in units of kHz. */
	unsigned int u32SymbolRateHz; /*The symbol rate symbols per second. */
	RTK_DVBSx_SIGNAL_PARAMS stDvbSxSignalPrams;  /*The DVB-S signal parameters. */
}stDvbSxChannel;

/*Internal used 64bit data structure*/
typedef struct st64Data
{
	unsigned int ui32Low,ui32High;
}RtkDemod64Data;

/*---------Global variables-------------------------------*/
extern stRtkDemodDvbConfig gstRtkDemodDvbConfig;
extern unsigned char gRtkDemodPowerOnInited;

/*SDK internal used function*/
unsigned int RTK_A_DVBSX_Log10ConvertdB(unsigned int i32InValue);  //convert value to dB
unsigned int RTK_A_DVBSX_Log10ConvertdBx10(unsigned int i32InValue);
unsigned int RTK_A_DVBSX_FixPow(unsigned int x,unsigned int y);
void RTK_A_DVBSX_64Add(RtkDemod64Data *pstSum,unsigned int ui32Addend);
void RTK_A_DVBSX_64Mult(RtkDemod64Data *pstRst,unsigned int m1, unsigned int m2);
unsigned int RTK_A_DVBSX_64Div(RtkDemod64Data stDivisor,RtkDemod64Data stDividend);
RtkDemod64Data RTK_A_DVBSX_64DivReturn64(RtkDemod64Data stDivisor,RtkDemod64Data stDividend);
void RTK_A_DVBSX_MiddleValueu8(unsigned char u8ValueA, unsigned char u8ValueB, unsigned char u8ValueC, unsigned char *MidValue);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetTSMode(DVBSX_DEMOD_MODULE *pDemod, RTK_DEMOD_MPEGTSMode stTSMode); /*internal function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetPER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32S2PER);               /*Get BCH block or RS packet error rate*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetPERNum(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32PERNum);          /*Get BCH block or RS packet error number,LGE*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBPktErrorCounter(DVBSX_DEMOD_MODULE *pDemod, unsigned short *uiPktErrCounter);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetPEC(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32PEC_NUM, unsigned int *ui32PEC_DEN);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_Read(DVBSX_DEMOD_MODULE *pDemod, unsigned char u8BaseAddr, unsigned char u8OffsetAddr,unsigned char *u8RegValue);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_Write(DVBSX_DEMOD_MODULE *pDemod, unsigned char u8BaseAddr, unsigned char u8OffsetAddr, unsigned char u8data);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_WriteRegArray(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pRegTable, int iTableLen);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_LatchOn(DVBSX_DEMOD_MODULE *pDemod);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_LatchOff(DVBSX_DEMOD_MODULE *pDemod);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_TSBitAccu(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32TSBitCount);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SwapIQ(DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8IQ);  /*Internal function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetSrOptimalTable(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32SrHz, unsigned int *ui32RegTable, unsigned int i32TableLen);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDefaultValue(DVBSX_DEMOD_MODULE *pDemod);
/*Set  system clock Functions*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetSysClkClockDVBSx(DVBSX_DEMOD_MODULE *pDemod);/*internal function*/
extern unsigned char  DvbSxBlindScanMode;


void RTK_A_DVBSX_WagtailDVBSxClkSet(DVBSX_DEMOD_MODULE *pDemod);
void RTK_A_DVBSX_SetTSVarThresh(DVBSX_DEMOD_MODULE *pDemod, unsigned char Nvco);

RTK_A_DVBSX_STATUS RTK_A_DVBSX_PowerOnInit(DVBSX_DEMOD_MODULE *pDemod);     /*Initialize demodulator with parameters setting., This function should be the first function be called*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_StandBy(DVBSX_DEMOD_MODULE *pDemod);         /*Set demodulator on standby state, attention demodulator should enter standby state before demodulator's
											   external clock be closed.or Hardware reset is required.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_StandByWakeUp(DVBSX_DEMOD_MODULE *pDemod);   /*Set demodulator on wakeup  state, it is normal receiving mode.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_Suspend(DVBSX_DEMOD_MODULE *pDemod);         /*Set demodulator on suspend state. I2C will not works after suspended, hardware reset is required to power up the demodulator*/

RTK_A_DVBSX_STATUS RTK_A_DVBSX_TSLockFlag(DVBSX_DEMOD_MODULE *pDemod, unsigned char *LockStatus);           /*Get TS lock status */
RTK_A_DVBSX_STATUS RTK_A_DVBSX_FECLockFlag(DVBSX_DEMOD_MODULE *pDemod, unsigned char *LockStatus);           /*Get FEC lock status*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_TimingLockFlag(DVBSX_DEMOD_MODULE *pDemod, unsigned char *LockStatus);           /*Get Timing lock status*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SignalIFAGC(DVBSX_DEMOD_MODULE *pDemod, unsigned short *ui16CalcValue);       /*Get IF AGC value. range of 0 - 1024*/

RTK_A_DVBSX_STATUS RTK_A_DVBSX_HoldDSP(DVBSX_DEMOD_MODULE *pDemod);                            /*Hold DSP and reset memory, demodulator will not decoding signal.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_StartDSP(DVBSX_DEMOD_MODULE *pDemod);                           /*Start DSP,demodulator start decoding signal..*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_ChipID(DVBSX_DEMOD_MODULE *pDemod, unsigned char *ui8ChipID);                 /*Get chip ID. chip ID is 0xa8*/

RTK_A_DVBSX_STATUS RTK_A_DVBSX_DisableTSOutput(DVBSX_DEMOD_MODULE *pDemod);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_EnableTSOutput(DVBSX_DEMOD_MODULE *pDemod);

RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetTSOutputOnOff (DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8TSOnOff);  //set TS Output Off, RTK_DEMOD_TS_ON/RTK_DEMOD_TS_OFF, LGE
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetTSOutputOnOff (DVBSX_DEMOD_MODULE *pDemod, unsigned char *ui8TSOnOff); //Get TS Output On, LGE
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetTSModeFlag(DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8TSModeFlag);   //Set TS Mode Flag,  RTK_DEMOD_TS_SERIAL_MODE/RTK_DEMOD_TS_PARALLEL_MODE, LGE
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetTSModeFlag (DVBSX_DEMOD_MODULE *pDemod, unsigned char *ui8TSModeFlag); //Get TS Mode Flag,  LGE
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetTsInterfaceMode (DVBSX_DEMOD_MODULE *pDemod, RTK_DEMOD_MPEGTSMode stTSMode);

RTK_A_DVBSX_STATUS RTK_A_DVBSX_DynamicCheckProcess(DVBSX_DEMOD_MODULE *pDemod);                 /*API should be called on 500ms period.*/

RTK_A_DVBSX_STATUS   RTK_A_DVBSX_I2CRead(DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char *ui8RegValue);
RTK_A_DVBSX_STATUS   RTK_A_DVBSX_I2CWrite(DVBSX_DEMOD_MODULE *pDemod, unsigned char ui8BaseAddr, unsigned char ui8RegisterAddr, unsigned char ui8data);

#define RTK_DEMOD_CALLER(fmt, args...)     rtd_demod_print(KERN_DEBUG,"CALLER, " fmt, ##args);
#define RTK_DEMOD_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"Debug, " fmt, ##args);
#define RTK_DEMOD_INFO(fmt, args...)       rtd_demod_print(KERN_INFO,"Info, " fmt, ##args);
#define RTK_DEMOD_WARNING(fmt, args...)    rtd_demod_print(KERN_WARNING,"Warning, " fmt, ##args);

#endif /*#ifndef _DVBSX_DEMOD_REALTEK_A_BASE_H*/
