#ifndef __DEMOD_RTK_R_ISDBT_HANDLE_H__
#define __DEMOD_RTK_R_ISDBT_HANDLE_H__

#include "demod.h"
#include "demod_rtk_def.h"

#define TAG_NAME "DEMO"


typedef struct {
	void*                       m_private;
	unsigned char               m_output_freq;
	TV_SIG_INFO                 m_SigInfo;
	REG_VALUE_ENTRY*            m_pTunerOptReg;
	unsigned int                m_TunerOptRegCnt;
	unsigned char               m_low_snr_detect;
	unsigned char               m_low_snr_recover;
	unsigned char               m_auto_tune_enable;
	unsigned char               m_acquire_sig_en;

	//int (*Activate)(struct sDEMOD *demod, unsigned char force_rst);
	//void (*Destory)(struct sDEMOD *demod);
} RTK_R_ISDB;

void RTK_R_ISDBT_Constructors(DEMOD* pDemodKernel, unsigned char addr, 	unsigned char output_mode, unsigned char output_freq, COMM* pComm);
void RTK_R_ISDBT_Destructors(DEMOD* pDemodKernel);
int RTK_R_ISDBT_Init(DEMOD* pDemodKernel);
int RTK_R_ISDBT_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys);
int RTK_R_ISDBT_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam);
int RTK_R_ISDBT_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode);
int RTK_R_ISDBT_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam);
int RTK_R_ISDBT_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock);
int RTK_R_ISDBT_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);
int RTK_R_ISDBT_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id,	TV_SIG_QUAL* pQuality);
int RTK_R_ISDBT_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset);
int RTK_R_ISDBT_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock);
int RTK_R_ISDBT_AcquireSignalThread(DEMOD* pDemodKernel);
int RTK_R_ISDBT_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range);
int RTK_R_ISDBT_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off);
int RTK_R_ISDBT_AutoTune(DEMOD* pDemodKernel);
int RTK_R_ISDBT_KernelAutoTune(DEMOD* pDemodKernel);
int RTK_R_ISDBT_InspectSignal(DEMOD* pDemodKernel);
int RTK_R_ISDBT_Activate(DEMOD* pDemodKernel, unsigned char force_rst);
int RTK_R_ISDBT_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode);
void RTK_R_ISDBT_Lock(DEMOD* pDemodKernel);
void RTK_R_ISDBT_Unlock(DEMOD* pDemodKernel);
int RTK_R_ISDBT_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo);
int RTK_R_ISDBT_GetSegmentNum(DEMOD* pDemodKernel, int layerIndex, unsigned char* pNum);
void RTK_R_ISDBT_IsPiSnrOverThd(DEMOD* pDemodKernel, unsigned char *u8IsPiSnrOverThd);



#define RTK_R_ISDB_DBG(fmt, args...)        rtd_demod_print(KERN_DEBUG,"[ISDBT Debug]" fmt, ##args);
#define RTK_R_ISDB_INFO(fmt, args...)        rtd_demod_print(KERN_INFO,"[ISDBT Info]" fmt, ##args);
#define RTK_R_ISDB_WARNING(fmt, args...)     rtd_demod_print(KERN_WARNING,"[ISDBT Warning]" fmt, ##args);

#endif // __DEMOD_RTK_R_ISDBT_HANDLE_H__
