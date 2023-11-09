#ifndef __DEMOD_REALTEK_A_DVBSX_H__
#define __DEMOD_REALTEK_A_DVBSX_H__

#include "demod.h"
#include "demod_rtk_def.h"

#define DISABLE_DVBS2X
#define DYNAMIC_TPO_CLK
#define PER_COLOR_ENABLE

#define BS_FFT_RESOLUTION            85000
#define TUNER_STEP_TOLERANCE         0
#define WIN_MARGIN                   8200000
#define BS_FULL_NO_SIGNAL_TH         1350//90000//1350
#define MIN_CSR                      1300000
#define MAX_CSR                      54000000
#define CN_PATCH_SR_TH               4900000 //4.9M

typedef struct _s_sDvbS2XLockTimeOut
{
	unsigned int SymbolRateKhz;
	unsigned int TimeOutMs;
}s_sDvbS2XLockTimeOut;

typedef struct {
	void*                       m_private;
	unsigned int                m_TunerOptRegCnt;
	TV_SIG_INFO                 m_info;
	unsigned char               m_auto_tune_en;
	unsigned char               m_acquire_sig_en;
	unsigned int                m_ts_clk_khz;
	unsigned int                m_bs_stime;
	unsigned int                m_bs_etime;
	unsigned char               m_bs_init_done;
	unsigned int                m_dvbsx_snr;
	unsigned char               m_lock_pivot;
	unsigned char               m_dis_dvb2x_rolloff_mode;

	//int (*Activate)(struct sDEMOD *demod, unsigned char force_rst);
	//void (*Destory)(struct sDEMOD *demod);
} REALTEK_A_DVBSX;

	void REALTEK_A_DVBSX_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm);
	void REALTEK_A_DVBSX_Destructors(DEMOD* pDemodKernel);

	int REALTEK_A_DVBSX_Init(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock);
	int REALTEK_A_DVBSX_AcquireSignalThread(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_GetChannelInfo(DEMOD* pDemodKernel, unsigned char* Standard, unsigned char* Modulation, unsigned char* CodeRate, unsigned char* PilotOnOff, unsigned char* Inverse);
	int REALTEK_A_DVBSX_ScanSignal(DEMOD* pDemodKernel, S32BITS* offset, SCAN_RANGE range);
	int REALTEK_A_DVBSX_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys);
	int REALTEK_A_DVBSX_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo);
	int REALTEK_A_DVBSX_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam);
	int REALTEK_A_DVBSX_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode);
	int REALTEK_A_DVBSX_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam);
	int REALTEK_A_DVBSX_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock);
	int REALTEK_A_DVBSX_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
	int REALTEK_A_DVBSX_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality);
	int REALTEK_A_DVBSX_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset);
	int REALTEK_A_DVBSX_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize);
	int REALTEK_A_DVBSX_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off);
	int REALTEK_A_DVBSX_Activate(DEMOD* pDemodKernel, unsigned char force_rst);
	int REALTEK_A_DVBSX_CalDvbS2Ssi(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_CalDvbS2Sqi(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_CalDvbS2XSsi(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_CalDvbS2XSqi(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_CalDvbSSsi(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_CalDvbSSqi(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_AutoTune(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_KernelAutoTune(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_InspectSignal(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE sys_type);
	int REALTEK_A_DVBSX_SetDiseqcContinue22kOnOff(DEMOD* pDemodKernel, unsigned int OnOff);
	int REALTEK_A_DVBSX_SetDiseqcCmd(DEMOD* pDemodKernel, unsigned int Length, unsigned char *Data);
	int REALTEK_A_DVBSX_SetDiseqcToneBurst(DEMOD* pDemodKernel, DISEQC_TONE_MODE mode);
	int REALTEK_A_DVBSX_SetDiseqcUnModToneBurst(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_SetDiseqcModToneBurst(DEMOD* pDemodKernel);

	int REALTEK_A_DVBSX_SpecAnGetSpectrum(DEMOD* pDemodKernel, unsigned int *SpectrumArray, unsigned int *SpectrumLength);
	int REALTEK_A_DVBSX_SpecAnGetCp1Cp2(DEMOD* pDemodKernel, unsigned int *SpectrumArray, unsigned int SkipDc, unsigned int CutSp, unsigned int CutEp, short *AllCutCandidateNum, short *AllCutCandidateCp1, short *AllCutCandidateCp2, unsigned int CutTh_1st, unsigned int CutTh_2nd, unsigned int CutTh_3rd, unsigned int CutTh_4th, unsigned int CutTh_5th, unsigned int CutTh_6th);
	int REALTEK_A_DVBSX_SpecAnCalCfoCsr(DEMOD* pDemodKernel, short Cp1, short Cp2, int *SigCfo, unsigned int *SigCsr);
	int REALTEK_A_DVBSX_SpecAnSearchMaxMin(DEMOD* pDemodKernel, unsigned int *SpectrumArray, unsigned int Sp, unsigned int Ep, unsigned int *MaxPower, unsigned int *MaxPowerPos, unsigned int *MinPower, unsigned int *MinPowerPos);
	int REALTEK_A_DVBSX_BlindscanCheckSignalLock(DEMOD* pDemodKernel, unsigned int CCfo, unsigned int Csr, unsigned char *SigLock, unsigned int *FSr, int *FCfo);
	int REALTEK_A_DVBSX_BlindscanAnalyzeSpectrum(DEMOD* pDemodKernel, unsigned char *SignalFound, int *SigCfo, unsigned int *SigCsr, unsigned int *NextStep);
	int REALTEK_A_DVBSX_BlindScan_Init(DEMOD* pDemodKernel, unsigned int StartFreqMhz, unsigned int EndFreqMhz);
	int REALTEK_A_DVBSX_BlindScan_GetTunerFreq(DEMOD* pDemodKernel, unsigned int *TunerCenterFreqMhz, unsigned int *TunerCutOffFreqKhz);
	int REALTEK_A_DVBSX_BlindScan_NextFreq(DEMOD* pDemodKernel, unsigned int *bBlindScanEnd);
	int REALTEK_A_DVBSX_BlindscanRemoveDuplicateTp(DEMOD* pDemodKernel, unsigned char mode);
	int REALTEK_A_DVBSX_BlindScan_GetStatus(DEMOD* pDemodKernel, unsigned char *Status, unsigned int *bBlindScanLock);
	int REALTEK_A_DVBSX_BlindScan_End(DEMOD* pDemodKernel);
	int REALTEK_A_DVBSX_BlindScan_GetFoundTP(DEMOD* pDemodKernel, unsigned int *FreqMhz, unsigned int *SymbolRateKhz);
	int REALTEK_A_DVBSX_BlindScan_AcquireSignal(DEMOD* pDemodKernel, unsigned int *FrequencyKhz, unsigned int *SymbolRateKhz, int *Fcfo, unsigned char *bTpLock);
	int REALTEK_A_DVBSX_ChangeStandard(DEMOD* pDemodKernel, unsigned int Mode);
	int REALTEK_A_DVBSX_TSBitRateCheck(DEMOD* pDemodKernel);

	void REALTEK_A_DVBSX_Lock(DEMOD* pDemodKernel);
	void REALTEK_A_DVBSX_Unlock(DEMOD* pDemodKernel);

#define REALTEK_A_DVBSX_DBG(fmt, args...) rtd_demod_print(KERN_DEBUG,"DVBS2_DEBUG, " fmt, ##args); 
#define REALTEK_A_DVBSX_INFO(fmt, args...) rtd_demod_print(KERN_INFO,"DVBS2_INFO, " fmt, ##args);
#define REALTEK_A_DVBSX_WARNING(fmt, args...) rtd_demod_print(KERN_WARNING,"DVBS2_WARNING, " fmt, ##args);

#endif // __DEMOD_REALTEK_A_DVBSX_H__
