#ifndef __DEMOD_REALTEK_R_QAM_H__
#define __DEMOD_REALTEK_R_QAM_H__

#include "demod.h"
#include "demod_rtk_def.h"

#define AUTODETECT_OPENCABLE_SIG_UPDATE_INTERVAL_MS 3500
#define AUTODETECT_OPENCABLE_GET_LOCK_INTERVAL_MS 1000
#define AUTODETECT_OPENCABLE_SFI_TRK_INTERVAL_MS 200

typedef struct {
	void*                       m_private;
	unsigned char               m_output_freq;
	REG_VALUE_ENTRY*            m_pTunerOptReg;
	REG_VALUE_ENTRY_RBUS*       m_pTunerOptRegRbus;
	unsigned int                m_TunerOptRegCnt;
	unsigned char               m_low_snr_detect;
	unsigned char               m_low_snr_recover;
	unsigned char               m_auto_tune_en;
	unsigned char               m_acquire_sig_en;
	U32BITS               		m_autotune_stime;
	U32BITS               		m_autotune_GetLockTime;
	unsigned int          		m_unlock_cnt;
	U32BITS               		m_autotune_SfiTrkTime;
	U32BITS						m_acquire_stime;
	//int (*Activate)(struct sDEMOD *demod, unsigned char force_rst);
	//void (*Destory)(struct sDEMOD *demod);
} REALTEK_R_QAM;

void REALTEK_R_QAM_Constructors(DEMOD* pDemodKernel, unsigned char addr, 	unsigned char output_mode, unsigned char output_freq, COMM* pComm);
void REALTEK_R_QAM_Destructors(DEMOD* pDemodKernel);
int REALTEK_R_QAM_Init(DEMOD* pDemodKernel);
int REALTEK_R_QAM_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys);
int REALTEK_R_QAM_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam);
int REALTEK_R_QAM_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode);
int REALTEK_R_QAM_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam);
int REALTEK_R_QAM_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock);
int REALTEK_R_QAM_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
int REALTEK_R_QAM_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id,	TV_SIG_QUAL* pQuality);
int REALTEK_R_QAM_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset);
int REALTEK_R_QAM_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize);
int REALTEK_R_QAM_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock);
int REALTEK_R_QAM_AcquireSignalThread(DEMOD* pDemodKernel, unsigned char WaitSignalLock);
int REALTEK_R_QAM_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range);
int REALTEK_R_QAM_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off);
int REALTEK_R_QAM_AutoTune(DEMOD* pDemodKernel);
int REALTEK_R_QAM_KernelAutoTune(DEMOD* pDemodKernel);
int REALTEK_R_QAM_InspectSignal(DEMOD* pDemodKernel);
int REALTEK_R_QAM_Activate(DEMOD* pDemodKernel, unsigned char force_rst);
int REALTEK_R_QAM_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode);
void REALTEK_R_QAM_Lock(DEMOD* pDemodKernel);
void REALTEK_R_QAM_Unlock(DEMOD* pDemodKernel);
int REALTEK_R_QAM_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo);

#define REALTEK_R_QAM_DBG(fmt, args...)         rtd_demod_print(KERN_DEBUG,"QAM_DEBUG, " fmt, ##args);
#define REALTEK_R_QAM_INFO(fmt, args...)       rtd_demod_print(KERN_INFO,"QAM_INFO, " fmt, ##args);
#define REALTEK_R_QAM_WARNING(fmt, args...)    rtd_demod_print(KERN_WARNING,"QAM_WARNING, " fmt, ##args);

#endif // __DEMOD_REALTEK_R_QAM_H__
