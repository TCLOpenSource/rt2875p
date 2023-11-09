#ifndef __SCALER_AVDCTRL_VD
#define  __SCALER_AVDCTRL_VD

#include <generated/autoconf.h>
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



#endif

