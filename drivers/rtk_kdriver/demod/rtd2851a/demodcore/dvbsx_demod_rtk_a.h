/***************************************************************************** 
Copyright 2018, Realtek Inc. All rights reserved.
File Name: dvbs2_demod_rtk.h
This is API header file of RTK Demod.
******************************************************************************/
#ifndef __DVBSX_DEMOD_REALTEK_A_H
#define __DVBSX_DEMOD_REALTEK_A_H

#include "dvbsx_demod_rtk_a_base.h"
#include "tv_osal.h"
#include "foundation.h"

extern unsigned char u8DVBSx255kSupport; /*Default Enable 0.255M symol rate support, set this option 0 to disable 0.255M symbol rate */
#define RTK_DVBSX_LITE_SYMBOL_HZ 980000 /*less than 0.8M small symbol rate signal threshhold*/
#define MAX_LITE_SR_SIGNAL_NUM 10 /* lite SR signal suppoted signal number */

#define MAX_SCANED_CHANNEL_NUMBER 500
#define MAX_SCANED_SYMBOL_NUM 50
#define TOTAL_MAX_SCANED_SYMBOL 1000
#define MAX_UNICABLE_BS_SYMBOL_RATE_KHZ 32100
#define MIN_UNICABLE_BS_SYMBOL_RATE_KHZ 2000
#define MAX_NORMAL_BS_SYMBOL_RATE_KHZ   45100
#define MIN_NORMAL_BS_SYMBOL_RATE_KHZ   2000
#define NO_SIGNAL_IN_SPECTRUM_TH        200
#define DUMMY_FREQ_KHZ 900000

#define SPEC_MAX_MIN_START_POINT        (8192-4096)//center-30M
#define SPEC_MAX_MIN_END_POINT          (8192+4096)//center+30M

#define DVBSX_LDPC_ITERATION_5M_COEF	39375
#define DVBSX_LDPC_ITERATION_10M_COEF	39375
#define DVBSX_LDPC_ITERATION_15M_COEF	21250
#define DVBSX_LDPC_ITERATION_20M_COEF	20000
#define DVBSX_LDPC_ITERATION_25M_COEF	16250
#define DVBSX_LDPC_ITERATION_30M_COEF	13750
#define DVBSX_LDPC_ITERATION_35M_COEF	11875
#define DVBSX_LDPC_ITERATION_40M_COEF	10625
#define DVBSX_LDPC_ITERATION_45M_COEF	9375

#define RTK_A_DVBSX_AbsSub(x, y) ((x >= y)? (x - y):(y - x))

//#define BS_SCAN_DEBUG
//#define PRINT_SPECTRUM

typedef struct RtkDemodDvbSxChannelScanParams
{
	unsigned int u32ScanStartFrequencyKHz; /*blind scan start frequency, input parameter*/
	unsigned int u32ScanStopFrequencyKHz; /*blind scan stop frequency, input parameter*/

	unsigned int u32MinSymbolRateHz; /*min symbol rate can be detected. >1M is default setting, input parameter*/
	unsigned int u32MaxSymbolRateHz; /*max symbol rate can be detected <=45M is default setting,input parameter*/
	unsigned int u32BlindScanStepKHz; /*Blind Scan step in KHz, input parameter*/

	unsigned int u32CurrentTunedFreKHz; /*Internal used for recording current tuned frequency */
	//unsigned int u32BlindScanUncheckedSignalNum; /*Internal used, unckeck symbols num*/

	//unsigned int u32CurrentScanFreqKHz; /*Current scan tune frequency in KHz, record blind scan frequency*/
	unsigned int u32CurrentSymbolScanFreKHz; /*Internal used for current Blind scan Tune frequency in KHz*/
	unsigned int u32CurrentWorkFreKHzProcessed; /*Internal used for scan ratio display */

	unsigned int u32BlindScanCheckedSignalIndex; /*Internal used, Check every scanned symbols from signal list, index value record checked signal */
	unsigned int u32ScanPreCheckedFreqKHz;

	unsigned int u32PreTunedSymbolRateHz; /*Internal used when time sliced*/
	unsigned char u8SignalRequiredCheckTimes; /*Internal used when time sliced*/

	unsigned int u32MaxCarrierOffsetKHz; /*Maximum valid CFO detected, input parameter*/

	unsigned int u32MaxSignalRightRangeFreqKHz; /*Internal used, Aways record the right signal's edge, symbol scan will refer this value*/
	unsigned char u8SingalInLockingState; /*Internal used, when scan time sliced, the locking time may be more than 1s.*/
	unsigned char u8FirstfrequencyScaned; /*Internal used, when search from start frequency used. */
	int i32CFOLimitKHz;
	int i32SpectrumRangeLimit;
	stDvbSxChannel DvbSxChannel[MAX_SCANED_CHANNEL_NUMBER]; /*channel list used by SDK blind scan module*/
	stDvbSxChannel GetFoundChannel[MAX_SCANED_CHANNEL_NUMBER]; /*channel list used by SDK blind scan module*/
	unsigned int u32ValidChannelNumber, ui32ScanedValidChannelNum, u32GetFoundChannelNumber; /*record current valid channel counter.*/
}stRtkDemodDvbSxChannelScanParams;

typedef struct RtkDemodDvbSxBsLocalParams
{
	unsigned int u32StopFrequency;
	unsigned int u32SymbolScanStep;
	unsigned int u32TmpSymbolStep;
	unsigned int u32TuneCenterFrequencyKHz;
	unsigned int u32TuneCheckSymbolRateKHz;
	unsigned int u32tmpSymbolrate;
	unsigned int u32NextSearchingfreq;
	unsigned char u8NewChannelFound;
	unsigned int ValidChannlIndex;
	int i32BlindScanStepKHz;
	int searchStartTime;
	int searchDelay;
}stRtkDemodDvbSxBsLocalParams;

typedef struct RtkDemodDvbSxBsSpecialParams
{
	unsigned char u8IsScanSymbolMode;
	unsigned char u8IsCheckTPTunerDone;
	unsigned char u8IsTPLock;
	unsigned char u8BsLnbMode;
	unsigned char u8IsSpectrumCheckDone;
	unsigned char u8UnicableEnabled;
	unsigned char u8NoSignalInSpectrum;
	unsigned char u8SpectrumModeCnt;
	RTK_DVBSx_SIGNAL_SPECTRUM pSignalSpectrum;
	RTK_DVBSx_SIGNAL_SPECTRUM pCheckedSpectrum;
	unsigned int u32SpectrumArray[1024];
	unsigned short u16SmoothArray[1024];
}stRtkDemodDvbSxBsSpecialParams;

typedef struct BlindScanSymbolStruct
{
	int i32ScanedCFO;
	unsigned int u32CoarseSymbolRateKHz;
	unsigned int u32SymbolRateKHz;
	unsigned int u32absCFO; /*not used in blindscan, used in auto symbol rate.*/
}stBlindScanSymbolParams;

typedef struct RTK_LOCKING_STATUS_STRUCT
{
	unsigned char u8TimingLockFlag;
	unsigned char u8PLLLockFlag;
	unsigned char u8SyncLockFlag;
	unsigned char u8FecLockFlag;
	unsigned char u8TSLockFlag;
	unsigned int u32SymbolRateK;
	int i32cfoK;
}stRTK_LOCKING_STATUS;

typedef struct BlindScanCheckedParamsStruct
{
	stBlindScanSymbolParams ScanedSymbols[MAX_SCANED_SYMBOL_NUM];
	unsigned char u8ScanedSymbolNum;
}stBlindScanCheckedParams;

typedef struct SCANED_SIGNAL_STRUCT
{
	unsigned int u32ScanedFreqKHz;
	unsigned int u32SymbolRateKHz;
}stScanedSignalParams;

typedef struct RTK_BLIND_SCANED_SIGNAL_STRUCT
{
	stScanedSignalParams ScanedSignalList[TOTAL_MAX_SCANED_SYMBOL];
	unsigned int u32ScanedSignalNum;
	unsigned int u32ScanedSortStartIdx;
}stBlindScanedSignal;

typedef struct LiteSymbolRateSignalStruct
{
	unsigned int FreqKhz;
	unsigned int SymbolRateHz;
	short Cfo;
}stLiteSymbolRateSignal;

typedef struct LiteSymbolRateSignalListStruct
{
	stLiteSymbolRateSignal LiteSignalList[MAX_LITE_SR_SIGNAL_NUM];
	unsigned char u8LiteSignalNum;
}stLiteSymbolRateSignalList;

RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxMode(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32Standard, unsigned int u32SymbolrateHz, unsigned int u32FreqKhz); /*Set DVB-S_S2_S2X auto mode with SymbolRate value, u32SymbolrateHz: symbols per second.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxModeOnly(DVBSX_DEMOD_MODULE *pDemod); /*Just set demodulator on DVB-S_S2_S2X mode. default set 45M symbol rate */
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxVeryLowSrAdc(DVBSX_DEMOD_MODULE *pDemod, unsigned char VeryLowSrEn);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDVBSxISI(DVBSX_DEMOD_MODULE *pDemod, unsigned char u8SelectID); /*Set DVB-S_S2_S2X VCM ID*/
void RTK_A_DVBSX_SetDVBSx255kEnableDisable(DVBSX_DEMOD_MODULE *pDemod, unsigned char u8EnableDvbSx225kFlag); /*Set DVB-S_S2_S2X 255K support(set 1) or not support(set 0), blind scan will disable 255k*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxAutoSymbolScanMode(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32SymbolRateKHz); /*Set auto symbol rate mode and get locked symbol rate */
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxTunerSpecialSettings(DVBSX_DEMOD_MODULE *pDemod);

//DVB-S/S2 Get functions API
RTK_A_DVBSX_STATUS RTK_A_DVBSX_ChannelLockCheck(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8DvbSxLockStatus); /*Check DVB-S_S2 lock status.This function checks lock status within a time window related with symbol rate value*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindscanChannelLockCheck(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8DvbSxLockStatus); /*Check DVB-S_S2 lock status.This function checks lock status within a time window related with symbol rate value*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_DVBSxModeDetected(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8DVBSxMode); /*Get signal mode after signal locked, DVB-S or DVB-S2.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetLockingStatus(DVBSX_DEMOD_MODULE *pDemod, stRTK_LOCKING_STATUS *stRtkDemodLockingStatus); /*Get the lock status of Timing, Sync, FEC, TS and the value of symbol rate, CFO.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxSNR(DVBSX_DEMOD_MODULE *pDemod, int *pu32SNRx10); /*Get DVBS-S_S2 SNR*10. scalar is 10*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxSignalQuality(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8SQI); /*Get DVB-S_S2 signal quality.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxSignalParams(DVBSX_DEMOD_MODULE *pDemod, RTK_DVBSx_SIGNAL_PARAMS *pstDVBSxParams); /*Get DVB-S_S2 signal parameters.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxRollOff(DVBSX_DEMOD_MODULE *pDemod, RTK_DVBS2_ROLL_OFF_TYPE *RollOffType); /*Get DVB-S2 signal roll off.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxPER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32PER); /*Get DVB-S RS PER*E9 or DVB-S2 BCH Error rate, PER is error rate after FEC bit corrected.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxBER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32BER); /*Get DVBS or DVB-S2 BER*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxBEC(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32BEC_NUM, unsigned int *pui32BEC_DEN); /*Get DVBS or DVB-S2 BER*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxLockedCFO(DVBSX_DEMOD_MODULE *pDemod, int *i32LockedCFOKHz); /*Get DVB-S_S2 carrier offset.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxSymbolRate(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32SymbolRateHz); /*Get DVB-S_S2 SymbolRate.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDvBSxSpectrum(DVBSX_DEMOD_MODULE *pDemod, RTK_DVBSx_SIGNAL_SPECTRUM *pSignalSpectrum);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxISI(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8ISI_Ids, unsigned char *u8s2_isi_num); /*Get DVB-S2 ISI list and stream number.*/

//SDK internal used functions, should not be called directly.
/****************************Internal used functionsfor for set DVBSS2 ******************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetSymbolRate(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32SymbolRateHz); /*Set fine symbol rate*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_ForceSymbolRateandCFO(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32SymbolRteK, int i32CFOKHz); 
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvBSxRegs(DVBSX_DEMOD_MODULE *pDemod, BOOL BypassAGC); /*Set RTK Demod DVB-S/S2 scan mode with all registers, called by S/S2 mode set functions.*/

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBS2BER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32S2BER); /*Internal function, get DVB-S2 BER function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSBER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32BER); /*Internal function, get DVB-S BER function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBS2BEC(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32S2BEC_NUM, unsigned int *ui32S2BEC_DEN); /*Internal function, get DVB-S2 BER function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSBEC(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32BEC_NUM, unsigned int *ui32BEC_DEN); /*Internal function, get DVB-S BER function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetSymbolRate(DVBSX_DEMOD_MODULE *pDemod, unsigned int SymbolRateK); /*Internal function, set DVB-S/S2 symbol rate related registers*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvBSxRegs(DVBSX_DEMOD_MODULE *pDemod, BOOL BypassAGC); /*Internal function, set DVB-S/S2 registers*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvBSxSymbolScan(DVBSX_DEMOD_MODULE *pDemod, unsigned char *u32LowPwrNoSig); /*Internal function of DVB-S/S2 blind scan*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvBSxNoneSymbolScan(DVBSX_DEMOD_MODULE *pDemod); /*Internal function of DVB-S/S2 blind scan*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetScanedSymbols(DVBSX_DEMOD_MODULE *pDemod); /*Internal function of DVB-S/S2 blind scan*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SortSymbolRateAndCFO(DVBSX_DEMOD_MODULE *pDemod); /*Internal function of DVB-S/S2 blind scan*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetLdpc1p6XMaxIteration(DVBSX_DEMOD_MODULE *pDemod, unsigned int symbolrate, unsigned int acq_mode);

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxCoarseCFO(DVBSX_DEMOD_MODULE *pDemod, int *pi32CarrierOffsetKHz); /*Debug function for FAE, Get coarse CFO, usually used for debug if signal can not be locked.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvBSxSymbolAuto(DVBSX_DEMOD_MODULE *pDemod); /*Debug function for FAE, will auto detect symbol rate*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScanCheckSymbolRate(DVBSX_DEMOD_MODULE *pDemod, unsigned int *u32symbolRateHz);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_DPLLTrackingSpeedPatch(DVBSX_DEMOD_MODULE *pDemod);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_PhaseTrackingSpeedPatch(DVBSX_DEMOD_MODULE *pDemod);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_PilotTrackingAdjust(DVBSX_DEMOD_MODULE *pDemod, unsigned int symbolrate);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_CnPerformancePatch(DVBSX_DEMOD_MODULE *pDemod);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDVBSCnPerformance(DVBSX_DEMOD_MODULE *pDemod);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDVBS2CnPerformance(DVBSX_DEMOD_MODULE *pDemod);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetSpectrumManual(DVBSX_DEMOD_MODULE *pDemod, RTK_DVBSx_SIGNAL_SPECTRUM SignalSpectrum);

/*----------------------------API of DVB-S/S2 blind scan---------------------------------------------------------*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_DvBSxBlindScanInit(DVBSX_DEMOD_MODULE *pDemod); /*Init default parameters for DVB-S_S2 new blind scan process.*/
void RTK_A_DVBSX_SetDvBSxBlindScanParams(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32StartFreqKHz, unsigned int u32StopFreqKHz); /*Optional function, manually set start and stop frequencyKHz, usually 950000KHz - 2150000KHz as default value*/
void RTK_A_DVBSX_SetBlindScanParamsSymbolRange(DVBSX_DEMOD_MODULE *pDemod, unsigned int ui32MinSymbolRateHz, unsigned int ui32MaxSymbolRateHz);/*Optional function, manually set Symbol range after RTK_A_DVBSX_DvBSxBlindScanInit function*/
void RTK_A_DVBSX_BlindScanStart(DVBSX_DEMOD_MODULE *pDemod); /*Start BlindScan*/
void RTK_A_DVBSX_BlindScanStop(DVBSX_DEMOD_MODULE *pDemod); /*Stop BlindScan process */
unsigned char RTK_A_DVBSX_BlindScanCompletedFlag(DVBSX_DEMOD_MODULE *pDemod);
unsigned char RTK_A_DVBSX_GetBlindScanProcessRatio(DVBSX_DEMOD_MODULE *pDemod); /*Get BlindScan completed ratio [0:100] */
unsigned int RTK_A_DVBSX_GetScanLockChannelNumber(DVBSX_DEMOD_MODULE *pDemod); /*Get BlindScan locked signal number, 0-500 transponders*/
//void RTK_A_DVBSX_GetBlindScanResult(DVBSX_DEMOD_MODULE *pDemod, stDvbSxChannel *pDvbSxChannelParams, unsigned int u32TranspoderChannelIndex); /*Get BlindScan locked signal frequency and symbol rate of the locked signal*/
unsigned int RTK_A_DVBSX_NotDuplicateTpCheck(DVBSX_DEMOD_MODULE *pDemod, unsigned int mode, unsigned int check_freq, unsigned int check_sr);

RTK_A_DVBSX_STATUS RTK_A_DVBSX_ScanSignals(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32ScanFreKHz, unsigned char *u32LowPwrNoSig);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScanOneChannel(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32CenterFrequencyKHz, unsigned int u32SymbolRateKHz, unsigned char *pNewChannel);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_GetTunerFreq(DVBSX_DEMOD_MODULE *pDemod, unsigned int *TunerCenterFreqMhz, unsigned int *TunerCutOffFreqKhz);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_NextFreq(DVBSX_DEMOD_MODULE *pDemod, unsigned int *bBlindScanEnd);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_GetStatus(DVBSX_DEMOD_MODULE *pDemod, unsigned char *Status, unsigned int *bBlindScanLock);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_GetFoundTP(DVBSX_DEMOD_MODULE *pDemod, unsigned int *FreqMhz, unsigned int *SymbolRateKhz);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_End(DVBSX_DEMOD_MODULE *pDemod);
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScan_SelfInit(DVBSX_DEMOD_MODULE *pDemod); /*Clear checked signal for blind scan process.*/

// Builder
void BuildRealtekIDvbs2Module(DVBSX_DEMOD_MODULE **ppDemod, DVBSX_DEMOD_MODULE *pDvbS2DemodModuleMemory, BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory, I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory, unsigned char DeviceAddr, U32BITS CrystalFreqHz);

#define REALTEK_A_DVBSX_DBG(fmt, args...) rtd_demod_print(KERN_DEBUG,"DVBS2_DEBUG, " fmt, ##args); 
#define REALTEK_A_DVBSX_INFO(fmt, args...) rtd_demod_print(KERN_INFO,"DVBS2_INFO, " fmt, ##args);
#define REALTEK_A_DVBSX_WARNING(fmt, args...) rtd_demod_print(KERN_WARNING,"DVBS2_WARNING, " fmt, ##args);

#endif
