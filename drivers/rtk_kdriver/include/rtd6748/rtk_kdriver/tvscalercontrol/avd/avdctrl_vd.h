#ifndef __SCALER_AVDCTRL_VD
#define  __SCALER_AVDCTRL_VD

#include <generated/autoconf.h>
#include <tvscalercontrol/vdc/video.h>
#include <linux/version.h>

extern unsigned int g_uiBuffer_Ready;
extern unsigned int g_uiBufferDelay_Count;

extern void Check_PLL27X_Settings(void);
extern void AVD_Memory_Allocate(void);
extern void AVD_Memory_Free(void);
extern void AVD_SetBufferDelay_CountTH(unsigned int value);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
void VDTimer(struct timer_list *t);
#else
void VDTimer(unsigned long arg);
#endif
extern void VD_recheck_625flag_loop(unsigned char *flag);
extern char Scaler_AVD_Timer_Init(void);
extern void Scaler_AVD_Timer_Delete(void);
extern void Scaler_AVD_ClearTiming(void);
extern unsigned char Scaler_AVD_Break_DetectTiming(void);
extern void Scaler_AVD_Set_DetTiming_In_AutoScan_Flag(unsigned char a_ucFlag);
extern unsigned char Scaler_AVD_DetectTiming(void);
extern void AVD_Suspend(void);
extern void AVD_Resume(void);

// refactor sta ==================================================>
VDC_POWER_SETTING Scaler_AVD_Get_VDC_Power_Setting(void);

void Set_Val_AVD_ucCurColorStd(unsigned char std);

void Set_Val_AVD_no_signal_Count(unsigned int cnt);
void Set_Val_AVD_match_support_color_system(bool bflag);
void Set_Val_AVD_g_ucTimerCount_ex(unsigned int cnt);
void Set_Val_AVD_g_bTimerInitDone(bool bflag);
void Set_Val_AVD_g_ucStopVDTimer(bool bflag);


extern bool (*Get_Val_AVD_vdc_TV_reset)(unsigned int line);
extern unsigned int (*Get_Val_AVD_g_ucTimerCount)(void);
// refactor end ==================================================<

#endif

