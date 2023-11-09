#ifndef __DEMOD_REALTEK_R_DVBC_H__
#define __DEMOD_REALTEK_R_DVBC_H__

#include "demod.h"
#include "demod_rtk_def.h"


#define AUTODETECT_DVBC_SIG_UPDATE_INTERVAL_MS 3500
#define DVBC_STATUS_CHECK_INTERVAL_MS 1000

typedef struct {
	void*				m_private;
	unsigned char			m_output_freq;
	REG_VALUE_ENTRY*		m_pTunerOptReg;
	REG_VALUE_ENTRY_RBUS*	m_pTunerOptRegRbus;
	unsigned int			m_TunerOptRegCnt;
	TV_SIG_INFO				m_info;
	unsigned char			m_auto_tune_en;
	unsigned int			m_dvbc_AgcScale;
	unsigned int			m_dvbc_Invert;
	unsigned int			m_dvbc_ext_gain_dis;
	U32BITS					m_acquire_stime;
	unsigned char			m_acquire_sig_en;
	unsigned char			m_debug_tool_en;
	U32BITS					m_autotune_stime;
	U32BITS					m_autotune_GetLockTime;
	unsigned int			m_unlock_cnt;
	U32BITS					m_symbol_rate;
	unsigned int			m_ts_clk_KHz;
	unsigned char			m_IsNoSignal;
	unsigned int			m_auto_fsr;
	int						m_auto_ccfo;
	unsigned char			m_auto_qamd;

	//int (*Activate)(struct sDEMOD *demod, unsigned char force_rst);
	//void (*Destory)(struct sDEMOD *demod);
} REALTEK_R_DVBC;


void REALTEK_R_DVBC_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm);
void REALTEK_R_DVBC_Destructors(DEMOD* pDemodKernel);
int REALTEK_R_DVBC_Init(DEMOD* pDemodKernel);
int REALTEK_R_DVBC_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys);
int REALTEK_R_DVBC_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam);
int REALTEK_R_DVBC_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode);
int REALTEK_R_DVBC_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam);
int REALTEK_R_DVBC_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock);
int REALTEK_R_DVBC_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
int REALTEK_R_DVBC_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality);
int REALTEK_R_DVBC_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset);
int REALTEK_R_DVBC_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize);
int REALTEK_R_DVBC_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock);
int REALTEK_R_DVBC_AcquireSignalThread(DEMOD* pDemodKernel);
int REALTEK_R_DVBC_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range);
int REALTEK_R_DVBC_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off);
int REALTEK_R_DVBC_AutoTune(DEMOD* pDemodKernel);
int REALTEK_R_DVBC_KernelAutoTune(DEMOD* pDemodKernel);
int REALTEK_R_DVBC_InspectSignal(DEMOD* pDemodKernel);
int REALTEK_R_DVBC_Activate(DEMOD* pDemodKernel, unsigned char force_rst);
int REALTEK_R_DVBC_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode);
void REALTEK_R_DVBC_Lock(DEMOD* pDemodKernel);
void REALTEK_R_DVBC_Unlock(DEMOD* pDemodKernel);
int REALTEK_R_DVBC_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo);
int REALTEK_R_DVBC_TSBitRateCheck(DEMOD* pDemodKernel);


#define REALTEK_R_DVBC_DBG(fmt, args...)       rtd_demod_print(KERN_DEBUG,"DVBC_DEBUG, " fmt, ##args);
#define REALTEK_R_DVBC_INFO(fmt, args...)       rtd_demod_print(KERN_INFO,"DVBC_INFO, " fmt, ##args);
#define REALTEK_R_DVBC_WARNING(fmt, args...)    rtd_demod_print(KERN_WARNING,"DVBC_WARNING, " fmt, ##args);

#endif // __DEMOD_REALTEK_R_DVBC_H__
