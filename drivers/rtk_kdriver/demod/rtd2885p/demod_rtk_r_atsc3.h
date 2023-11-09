#ifndef __DEMOD_REALTEK_R_ATSC3_H__
#define __DEMOD_REALTEK_R_ATSC3_H__

#include "demod.h"
#include "demod_rtk_def.h"

#define REALTEK_R_ATSC3_MODE      0
#define REALTEK_I_DVBT_MODE       1
#include <rtd_log/rtd_module_log.h>
#define TAG_NAME "DEMO"


#define ATSC3_AUTODETECT_SIG_UPDATE_INTERVAL_MS 600
#define ATSC3_STATUS_CHECK_INTERVAL_MS 1000
#define ATSC3_STATUS_LGCHECK_INTERVAL_MS 950
#define AUTOTUNE_CHECK_VV_PATCH_INTERVAL_MS 3000
#define AUTOTUNE_CHECK_L1Crc_INTERVAL_MS 300

//FOR FPGA
//#define ATSC3_AUTODETECT_SIG_UPDATE_INTERVAL_MS 600 *6
//#define ATSC3_STATUS_CHECK_INTERVAL_MS 1000 *6
//#define AUTOTUNE_CHECK_VV_PATCH_INTERVAL_MS 1500 *6
//#define AUTOTUNE_CHECK_L1Crc_INTERVAL_MS 500 *6

typedef struct {
	void*                       m_private;
	unsigned char               m_output_freq;
	REG_VALUE_ENTRY*            m_pTunerOptReg;
	REG_VALUE_ENTRY_RBUS*       m_pTunerOptRegRbus;
	unsigned int                m_TunerOptRegCnt;
	TV_SIG_QUAL              mQuality;
	unsigned char               m_auto_tune_en;
	unsigned char               m_acquire_sig_en;
	int                m_current_bw;
	unsigned int                m_atsc3_Invert;
	unsigned long               m_autotune_stime;
	unsigned long               m_autotune_VVPatchCheckTime;
	unsigned long            m_autotune_L1CrcCheckTime;
	unsigned long            m_status_checking_stime;
	unsigned long            m_status_LGchecking_stime;
	unsigned int                m_unlock_cnt;
	unsigned char             m_AP_mPlpSelectedCnt;
	unsigned char             m_AP_mPlpSelectedNo[4];
	unsigned char               m_debug_tool_en;
	//int (*Activate)(struct sDEMOD *demod, unsigned char force_rst);
	//void (*Destory)(struct sDEMOD *demod);
} REALTEK_R_ATSC3;


void REALTEK_R_ATSC3_Constructors(DEMOD* pDemodKernel, unsigned char addr,    unsigned char output_mode, unsigned char output_freq, COMM* pComm);
void REALTEK_R_ATSC3_Destructors(DEMOD* pDemodKernel);
int REALTEK_R_ATSC3_Init(DEMOD* pDemodKernel);
int REALTEK_R_ATSC3_ChangePLP(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
int REALTEK_R_ATSC3_GetPLPInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo, unsigned char Atsc3MPlpMode);
int REALTEK_R_ATSC3_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock);
int REALTEK_R_ATSC3_AcquireSignalThread(DEMOD* pDemodKernel);
int REALTEK_R_ATSC3_ScanSignal(DEMOD* pDemodKernel, S32BITS* offset, SCAN_RANGE range);
int REALTEK_R_ATSC3_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys);
int REALTEK_R_ATSC3_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo);
int REALTEK_R_ATSC3_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam);
int REALTEK_R_ATSC3_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode);
int REALTEK_R_ATSC3_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam);
int REALTEK_R_ATSC3_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock);
int REALTEK_R_ATSC3_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
int REALTEK_R_ATSC3_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality);
int REALTEK_R_ATSC3_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset);
int REALTEK_R_ATSC3_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize);
int REALTEK_R_ATSC3_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off);
int REALTEK_R_ATSC3_Activate(DEMOD* pDemodKernel, unsigned char force_rst);
int REALTEK_R_ATSC3_AutoTune(DEMOD* pDemodKernel);
int REALTEK_R_ATSC3_KernelAutoTune(DEMOD* pDemodKernel);
int REALTEK_R_ATSC3_InspectSignal(DEMOD* pDemodKernel);
int REALTEK_R_ATSC3_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE sys_type);
int REALTEK_R_ATSC3_Suspend(DEMOD * pDemodKernel);
int REALTEK_R_ATSC3_Resume(DEMOD * pDemodKernel);
void REALTEK_R_ATSC3_Lock(DEMOD* pDemodKernel);
void REALTEK_R_ATSC3_Unlock(DEMOD* pDemodKernel);
int REALTEK_R_ATSC3_TPOutEn(DEMOD* pDemodKernel, unsigned char enable);
int REALTEK_R_ATSC3_System_Init(DEMOD* pDemodKernel, unsigned char On);
int REALTEK_R_ATSC3_PowerDown(DEMOD* pDemodKernel);
int REALTEK_R_ATSC3_BisrCheck(DEMOD* pDemodKernel);
int REALTEK_R_ATSC3_Ipga_Vcm_Setting(DEMOD* pDemodKernel);

#define REALTEK_R_ATSC3_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"[ATSC3]" fmt, ##args);
#define REALTEK_R_ATSC3_INFO(fmt, args...)        rtd_demod_print(KERN_INFO,"[ATSC3]" fmt, ##args);
#define REALTEK_R_ATSC3_WARNING(fmt, args...)     rtd_demod_print(KERN_WARNING,"[ATSC3]" fmt, ##args);
#endif // __DEMOD_REALTEK_R_ATSC3_H__
