#ifndef __DEMOD_REALTEK_R_ATSC_H__
#define __DEMOD_REALTEK_R_ATSC_H__

#include "demod.h"
#include "demod_rtk_def.h"

#define AUTOTUNE_CHECK_TS_OUTPUT_EN_INTERVAL_MS 250
#define AUTOTUNE_CHECK_CR2_PATCH_INTERVAL_MS 1000

typedef struct {
	void*                       m_private;
	unsigned char               m_output_freq;
	REG_VALUE_ENTRY*            m_pTunerOptReg;
	unsigned int                m_TunerOptRegCnt;
	unsigned char               m_low_snr_detect;
	unsigned char               m_low_snr_recover;
	unsigned char               m_auto_tune_enable;
	unsigned char               m_patch_ind;
	unsigned char               m_patch_cr2_ind;
	unsigned char               m_acquire_sig_en;
	unsigned long               m_autotune_TsOutputEnTime;
	unsigned long               m_autotune_Cr2PatchTime;
	struct mutex                m_lock;

	int (*Activate)(struct sDEMOD *demod, unsigned char force_rst);
	void (*Destory)(struct sDEMOD *demod);
	void (*Lock)(struct sDEMOD *demod);
	void (*Unlock)(struct sDEMOD *demod);
} REALTEK_R_ATSC;

void REALTEK_R_ATSC_Constructors(DEMOD* pDemodKernel, unsigned char addr, 	unsigned char output_mode, unsigned char output_freq, COMM* pComm);
void REALTEK_R_ATSC_Destructors(DEMOD* pDemodKernel);
int REALTEK_R_ATSC_Init(DEMOD* pDemodKernel);
int REALTEK_R_ATSC_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys);
int REALTEK_R_ATSC_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO*  pSigInfo);
int REALTEK_R_ATSC_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam);
int REALTEK_R_ATSC_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode);
int REALTEK_R_ATSC_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam);
int REALTEK_R_ATSC_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock);
int REALTEK_R_ATSC_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
int REALTEK_R_ATSC_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id,	TV_SIG_QUAL* pQuality);
int REALTEK_R_ATSC_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset);
int REALTEK_R_ATSC_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize);
int REALTEK_R_ATSC_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock);
int REALTEK_R_ATSC_AcquireSignalThread(DEMOD* pDemodKernel);
int REALTEK_R_ATSC_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range);
int REALTEK_R_ATSC_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off);
int REALTEK_R_ATSC_AutoTune(DEMOD* pDemodKernel);
int REALTEK_R_ATSC_KernelAutoTune(DEMOD* pDemodKernel);
int REALTEK_R_ATSC_InspectSignal(DEMOD* pDemodKernel);
int REALTEK_R_ATSC_Activate(DEMOD* pDemodKernel, unsigned char force_rst);
int REALTEK_R_ATSC_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode);
void REALTEK_R_ATSC_Lock(DEMOD* pDemodKernel);
void REALTEK_R_ATSC_Unlock(DEMOD* pDemodKernel);
int REALTEK_R_ATSC_DebugInfo(DEMOD* pDemodKernel, unsigned char debug_mode);
void REALTEK_R_ATSC_DumpSysInfo(void);
int REALTEK_R_ATSC_TPOutEn(DEMOD* pDemodKernel, unsigned char enable);
int REALTEK_R_Ext_ATSC_I2C_Init(DEMOD* pDemodKernel);
int REALTEK_R_Ext_ATSC_System_Init(DEMOD* pDemodKernel, unsigned char On);
int REALTEK_R_Ext_ATSC_BisrCheck(DEMOD* pDemodKernel);
int REALTEK_R_ATSC_Ipga_Vcm_Setting(DEMOD * pDemodKernel);



#define REALTEK_R_ATSC_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"ATSC_DEBUG, " fmt, ##args);
#define REALTEK_R_ATSC_INFO(fmt, args...)       rtd_demod_print(KERN_INFO,"ATSC_INFO, " fmt, ##args);
#define REALTEK_R_ATSC_WARNING(fmt, args...)    rtd_demod_print(KERN_WARNING,"ATSC_WARNING, " fmt, ##args);

#endif // __DEMOD_REALTEK_R_ATSC_H__
