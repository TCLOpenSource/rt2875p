#ifndef __DEMOD_REALTEK_A_DVBTX_H__
#define __DEMOD_REALTEK_A_DVBTX_H__

#include "demod.h"
#include "demod_rtk_def.h"

#define REALTEK_A_DVBTX_MODE      0
#define REALTEK_I_DVBT_MODE       1
#include <rtd_log/rtd_module_log.h>
#define TAG_NAME "DEMO"


#define AUTODETECT_SIG_UPDATE_INTERVAL_MS 600
#define AUTOTUNE_UPDATE_TS_CLK_INTERVAL_MS 500
#define AUTOTUNE_T2PLP_CHECK_INTERVAL_MS 500
#define DVBTX_STATUS_CHECK_INTERVAL_MS 1000

typedef struct {
	void*                       m_private;
	unsigned char               m_output_freq;
	REG_VALUE_ENTRY*            m_pTunerOptReg;
    REG_VALUE_ENTRY_RBUS*       m_pTunerOptRegRbus;
	unsigned int                m_TunerOptRegCnt;
	TV_SIG_INFO                 m_info;
	unsigned char               m_auto_tune_en;
	unsigned char               m_acquire_sig_en;
	unsigned char               m_dvbt2_dvbt_mode;
	int                m_current_bw;
	unsigned int                m_detected_bw;
	unsigned char               m_fastscan_en;
	unsigned int                m_plp_id;
	unsigned int                m_dvbt2_AgcScale;
	unsigned int                m_dvbt2_Invert;
	unsigned int                m_dvbt_AgcScale;
	unsigned int                m_dvbt_Invert;
	unsigned int                m_dvbt2_Ssi;
	unsigned int                m_dvbt2_dvbt_Ssi;
	unsigned int                m_PreUbc;
	unsigned long               m_autotune_stime;
    unsigned long               m_autotune_TsTime;
       unsigned long            m_dvbt2_PLPcheck_stime;
       unsigned long            m_status_checking_stime;
	//unsigned int                m_agc_settle_time;
	unsigned int                m_dvbt2_ext_gain_dis;
	unsigned int                m_dvbt_ext_gain_dis;
	//unsigned char               m_init_done;
	unsigned int                m_dvbt_bandwidth_mask;
	unsigned int                m_unlock_cnt;
	unsigned char               m_profileHP;
	unsigned int                m_dvbt2_sfo;
	unsigned char               m_debug_tool_en;
       unsigned int                m_ts_clk_KHz; 
	//int (*Activate)(struct sDEMOD *demod, unsigned char force_rst);
	//void (*Destory)(struct sDEMOD *demod);
} REALTEK_A_DVBTX;


void REALTEK_A_DVBTX_Constructors(DEMOD* pDemodKernel, unsigned char addr,    unsigned char output_mode, unsigned char output_freq, COMM* pComm);
void REALTEK_A_DVBTX_Destructors(DEMOD* pDemodKernel);
int REALTEK_A_DVBTX_Init(DEMOD* pDemodKernel);
int REALTEK_A_DVBTX_ChangeT2PLP(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
int REALTEK_A_DVBTX_GetT2PLPInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
int REALTEK_A_DVBTX_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock);
int REALTEK_A_DVBTX_AcquireSignalThread(DEMOD* pDemodKernel);
int REALTEK_A_DVBTX_ScanSignal(DEMOD* pDemodKernel, S32BITS* offset, SCAN_RANGE range);
int REALTEK_A_DVBTX_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys);
int REALTEK_A_DVBTX_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo);
int REALTEK_A_DVBTX_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam);
int REALTEK_A_DVBTX_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode);
int REALTEK_A_DVBTX_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam);
int REALTEK_A_DVBTX_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock);
int REALTEK_A_DVBTX_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
int REALTEK_A_DVBTX_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality);
int REALTEK_A_DVBTX_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset);
int REALTEK_A_DVBTX_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize);
int REALTEK_A_DVBTX_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off);
int REALTEK_A_DVBTX_Activate(DEMOD* pDemodKernel, unsigned char force_rst);
int REALTEK_A_DVBTX_AutoTune(DEMOD* pDemodKernel);
int REALTEK_A_DVBTX_KernelAutoTune(DEMOD* pDemodKernel);
int REALTEK_A_DVBTX_InspectSignal(DEMOD* pDemodKernel);
int REALTEK_A_DVBTX_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE sys_type);
int REALTEK_A_DVBTX_CalculateThroughput(DEMOD* pDemodKernel);
int REALTEK_A_DVBTX_TSBitRateCheck(DEMOD* pDemodKernel);
void REALTEK_A_DVBTX_TPOCheck(DEMOD* pDemodKernel, int isDVBT);
void REALTEK_A_DVBTX_Lock(DEMOD* pDemodKernel);
void REALTEK_A_DVBTX_Unlock(DEMOD* pDemodKernel);
int REALTEK_A_DVBTX_GetTsClkRate(DEMOD* pDemodKernel, unsigned int *pTsClkRate);

#define REALTEK_A_DVBTX_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"Dvbt2 Debug, " fmt, ##args);
#define REALTEK_A_DVBTX_INFO(fmt, args...)        rtd_demod_print(KERN_INFO,"Dvbt2 Info, " fmt, ##args);
#define REALTEK_A_DVBTX_WARNING(fmt, args...)     rtd_demod_print(KERN_WARNING,"Dvbt2 Warning, " fmt, ##args);
#endif // __DEMOD_REALTEK_A_DVBTX_H__
