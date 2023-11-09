#ifndef __DEMOD_REALTEK_R_DTMB_HANDLE_H__
#define __DEMOD_REALTEK_R_DTMB_HANDLE_H__

#include "demod_rtk_def.h"
#include "demod_rtk_r_dtmb_priv.h"


typedef struct {

	void*                       m_private;
	unsigned char               m_output_freq;
	REG_VALUE_ENTRY*            m_pTunerOptReg;
	unsigned int                m_TunerOptRegCnt;
	unsigned char               m_low_snr_detect;
	unsigned char               m_low_snr_recover;
	unsigned char               m_auto_tune_enable;
    unsigned char               m_auto_tune_reset;

	int (*Activate)(struct sDEMOD *demod, unsigned char force_rst);
	void (*Destory)(struct sDEMOD *demod);
} REALTEK_R_DTMB;



void REALTEK_R_DTMB_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm);
void REALTEK_R_DTMB_Destructors(DEMOD* pDemodKernel);

    
int REALTEK_R_DTMB_Init(DEMOD* pDemodKernel);
int REALTEK_R_DTMB_Reset(DEMOD* pDemodKernel);
int REALTEK_R_DTMB_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock);                         

int REALTEK_R_DTMB_ScanSignal(DEMOD* pDemodKernel, S32BITS* offset, SCAN_RANGE range);

int REALTEK_R_DTMB_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys);
int REALTEK_R_DTMB_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO*  pSigInfo);

int REALTEK_R_DTMB_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam);

int REALTEK_R_DTMB_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam);

int REALTEK_R_DTMB_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock);

int REALTEK_R_DTMB_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo);    

int REALTEK_R_DTMB_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality);                      

int REALTEK_R_DTMB_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off);    

int REALTEK_R_DTMB_InspectSignal(DEMOD* pDemodKernel);

int REALTEK_R_DTMB_AutoTune(DEMOD* pDemodKernel);

int REALTEK_R_DTMB_Activate(DEMOD* pDemodKernel, unsigned char force_rst);

int REALTEK_R_DTMB_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode);

int REALTEK_R_DTMB_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset);

int REALTEK_R_DTMB_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode);

void REALTEK_R_DTMB_Lock(DEMOD* pDemodKernel);
void REALTEK_R_DTMB_Unlock(DEMOD* pDemodKernel);

//#define REALTEK_R_DTMB_DBG(fmt, args...)         rtd_demod_print(KERN_DEBUG,"Dtmb Debug, " fmt, ##args);
//#define REALTEK_R_DTMB_INFO(fmt, args...)        rtd_demod_print(KERN_INFO,"Dtmb Info, " fmt, ##args);
//#define REALTEK_R_DTMB_WARNING(fmt, args...)     rtd_demod_print(KERN_WARNING,"Dtmb Warning, " fmt, ##args);

							 
#endif 

