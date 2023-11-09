#ifndef __SCALER_AVDCTRL_H
#define __SCALER_AVDCTRL_H

#include <generated/autoconf.h>
#include <rbus/timer_reg.h>
#include <ioctrl/scaler/vfe_cmd_id.h>
//#define _PRINT_AVD_COST_TIME

#define DETECT_STAGE_INIT 0
#define DETECT_STAGE_HVLOCK 1
#define DETECT_STAGE_VSTATE_READY 2
#define DETECT_STAGE_FINAL 0xff
#define jj_test_print 0

extern unsigned char g_ucAtvDetectStage;
extern unsigned char g_ucVDMode;
extern unsigned char g_ucVDPreMode;
extern unsigned char g_ucModeCurr;
extern unsigned int g_ulInitStageCount;

extern KADP_VFE_AVD_SCART_FB_STATUS_T g_ucScartMode;
extern unsigned char g_ucSrcType;
extern KADP_VFE_AVD_DEMOD_TYPE g_tAVDDemodType;
extern KADP_VFE_AVD_COLOR_SYSTEM_T g_tSupportedColorSys;
extern unsigned char g_ucStopVDTimer;
extern unsigned char g_ucTimerEnable;
extern unsigned char avd_good_timing_ready;
extern unsigned char vdc_resetTV_Count;
extern bool g_bMainRunSearchState;
extern bool g_bSubRunSearchState;

extern unsigned int g_ulVDCPhyAddr;
#if 0
#ifdef CONFIG_ARM64
extern unsigned long g_ulVDCPhyAddr_cache;
extern unsigned long *g_ulVDCPhyAddr_nonCache;
#else
extern unsigned int g_ulVDCPhyAddr_cache;
extern unsigned int *g_ulVDCPhyAddr_nonCache;
#endif
extern unsigned char g_ucMode_Curr;
#endif
extern unsigned char check_dma_3d_stable;
extern unsigned char vdpq_dma_3d_ready;




/**
 * color format.
*/
typedef enum {
	AVD_COLOR_RGB 	= 0x00,
	AVD_COLOR_YUV422,
	AVD_COLOR_YUV444,
	AVD_COLOR_YUV420,
	AVD_COLOR_UNKNOW
} AVD_COLOR_SPACE_T;

/**
 * color depth
*/
typedef enum {

	AVD_COLOR_DEPTH_8B =0,
	AVD_COLOR_DEPTH_10B,
	AVD_COLOR_DEPTH_12B,
	AVD_COLOR_DEPTH_16B,

} AVD_COLOR_DEPTH_T;

StructDisplayInfo * Get_AVD_Dispinfo(void);

ModeInformationType * Get_AVD_Timinginfo(void);

int Scaler_AVD_Init(KADP_VFE_AVD_CC_TTX_STATUS_T a_ucSupportingVBI);

char Scaler_AVD_Uninit(void);

char Scaler_AVD_Open(void);

char Scaler_AVD_Close(void);

char Scaler_AVD_AV_Connect(unsigned short srcinput);

char Scaler_AVD_AV_Disconnect(void);

char Scaler_AVD_ATV_Connect(unsigned short srcinput);

char Scaler_AVD_ATV_Disconnect(void);

void Scaler_AVD_Set_Scart_Mode(void);

void fw_video_SetModeInfo(unsigned char CurrentSrc);

void SetAVDLGETimingInfo(void);

// Scart CVBS
char Scaler_AVD_Scart_Connect(unsigned short srcinput);
// Scart RGB
char Scaler_AVD_ScartRGB_Connect(unsigned short srcinput);

char Scaler_AVD_Scart_Disconnect(void);

unsigned char Scaler_AVD_DetectTiming(void);

KADP_VFE_AVD_TIMING_INFO_T* Scaler_AVD_GetLGETimingInfo(unsigned char* a_ucRet);

char Scaler_AVD_SetLGEColorSystem(KADP_VFE_AVD_COLOR_SYSTEM_T ColorSystem);

char Scaler_AVD_GetLGEColorSystem(KADP_VFE_AVD_AVDECODER_VIDEO_MODE_T *ColorSystem);

unsigned char Scaler_AVD_ATV_DetectTiming(void);

void AVD_OnlineMeasureError_Handler(int srcType);

char Scaler_AVD_GetScartFastBlankingStatus(KADP_VFE_AVD_SCART_FB_STATUS_T *pStatus);

char Scaler_AVD_GetScartID(KADP_VFE_AVD_SCART_AR_T *pScartAr);

void Scaler_AVD_SetSyncDetectForTuning(unsigned char a_ucFlag);

unsigned char Scaler_AVD_DoesSyncExist(void);

#ifdef CONFIG_CUSTOMER_TV010
unsigned char Scaler_AVD_HV_Coarse_Lock(void);
#endif

void Scaler_AVD_SetChangeSystem(bool Change);

// refactor sta ==================================================>
void Scaler_AVD_Set_g_ucMode_Curr(unsigned char a_ucMode);
unsigned char Scaler_AVD_Get_g_ucMode_Curr(void);

void Scaler_AVD_Set_g_bChangeSystem(bool a_bFlag);
bool Scaler_AVD_Get_g_bChangeSystem(void);

void Scaler_AVD_Set_g_bStartChangeSystem(bool a_bFlag);
bool Scaler_AVD_Get_g_bStartChangeSystem(void);

void Scaler_AVD_SetRunSearchState(unsigned char a_ucChannel, bool a_bFlag);
bool Scaler_AVD_GetRunSearchState(unsigned char a_ucChannel);

void Scaler_AVD_Set_g_ucSrcType(unsigned char a_ucSource);
unsigned char Scaler_AVD_Get_g_ucSrcType(void);

void Scaler_AVD_SetDetectStage(unsigned char stage);
unsigned char Scaler_AVD_GetDetectStage(void);

void Scaler_AVD_Set_ChannelChange(bool a_bFlag);
bool Scaler_AVD_Get_ChannelChange(void);

void Scaler_AVD_SetIsChannelChange(bool a_bFlag);
bool Scaler_AVD_GetIsChannelChange(void);

char Scaler_AVD_SetDemodType(KADP_VFE_AVD_DEMOD_TYPE a_tDemodType);
KADP_VFE_AVD_DEMOD_TYPE Scaler_AVD_GetDemodType(void);

void Scaler_AVD_Update_Wid_Len(unsigned int len,unsigned int wid);
void IFD_SIF_ADC_Initial_Flow(void);//add for audio requirements

// refactor new ==================================================<
void Set_Val_AVD_Good_Timing_Ready(unsigned char ucFlag);
extern unsigned char (*Get_Val_AVD_good_timing_ready)(void);
void Set_Val_AVD_g_ucVDMode(unsigned char ucFlag);
extern unsigned char (*Get_Val_AVD_g_ucVDPreMode)(void);
// refactor end ==================================================<

void Scaler_AVD_SetAutoTuneMode(bool AutoTuneMode);

bool Scaler_AVD_GetAutoTuneMode(void);

unsigned char Scaler_AVD_GetAvdSource(void);

void Scaler_AVD_ClearTimingInfo(void);

#ifdef CONFIG_CUSTOMER_TV010
void AVD_TV_AV_Non_Standard_Process(void);
#endif

//bool Scaler_AVD_GetDetectTimingAgain(void);

//void Scaler_AVD_SetDetectTimingAgain(bool a_bFlag);

void AVD_Suspend(void);

void AVD_Resume(void);

unsigned char Scaler_AVD_GetVDPreMode(void);

unsigned char Scaler_AVD_CheckATVTimingChange(unsigned char a_ucChannel);

void ClearAVDLGETimingInfo(void);

extern StructDisplayInfo * Get_AVD_ScalerDispinfo(void);

char Scaler_AVD_IsSupportedColorSys(void);

void  Check_PLL27X_Settings(void);

#ifdef CONFIG_CUSTOMER_TV010
typedef enum  {
	IFD_CLAMP_SCALE_1D256 = 0  ,
	IFD_CLAMP_SCALE_1D128 = 1  ,
	IFD_CLAMP_SCALE_1D64 = 2  ,
	IFD_CLAMP_SCALE_1D32,
	IFD_CLAMP_SCALE_1D16,
	IFD_CLAMP_SCALE_1D8,
	IFD_CLAMP_SCALE_1D4,
	IFD_CLAMP_SCALE_1D2,
	IFD_CLAMP_SCALE_1,
	IFD_CLAMP_SCALE_2,
	IFD_CLAMP_SCALE_4,
} IFD_CLAMP_SCALE;
#endif

unsigned int AVD_vpq_mem_isAllocated(void);

bool Scaler_AVD_SetChannelChange(void);//For K4L
void reset_avd_timing_ready(void);
unsigned char avd_drv_timing_ready(void);
unsigned char avd_drv_vdpq_dma_3d_ready(void);
#ifdef SPEEDUP_NEW_SCALER_FLOW
unsigned char get_avd_start_need_compensation(void);
void set_avd_start_need_compensation(unsigned char TorF);
#endif

unsigned char Scaler_AVD_Break_DetectTiming(void);

void Scaler_AVD_Set_DetTiming_In_AutoScan_Flag(unsigned char a_ucFlag);

unsigned char AVD_vpq_mem_ctrl(unsigned char bEnable);

#endif /* __SCALER_AVDCTRL_H */

