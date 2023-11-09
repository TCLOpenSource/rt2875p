#ifndef __SCALER_VFEDEV_H
#define  __SCALER_VFEDEV_H

#define 	BIT0_VFE_AVD	0x00000001

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
	#include "../include/rtk_kdriver/scalercommon/scalerCommon.h"
#endif
#ifndef BUILD_QUICK_SHOW
#include <linux/fs.h>
#include <generated/autoconf.h>
#endif
#include <ioctrl/scaler/vfe_cmd_id.h>
#ifndef BOOLEAN
#ifndef _EMUL_WIN
typedef	unsigned int			__BOOLEAN;
#define BOOLEAN __BOOLEAN
#else
typedef	unsigned char		__BOOLEAN;
#define BOOLEAN __BOOLEAN
#endif
#endif

unsigned char get_ADC_Global_Status(void);
void set_ADC_Global_Status(SOURCE_STATUS status);
unsigned char get_AVD_Global_Status(void);
void set_AVD_Global_Status(SOURCE_STATUS status);
unsigned char get_ATV_Global_Status(void);
void set_ATV_Global_Status(SOURCE_STATUS status);
unsigned char get_AV_Global_Status(void);
void set_AV_Global_Status(SOURCE_STATUS status);
void set_Scart_Global_Status(SOURCE_STATUS status);
unsigned char get_HDMI_Global_Status(void);
void set_HDMI_Global_Status(unsigned char);
unsigned char get_ADC_Input_Source(void);
void set_ADC_Input_Source(unsigned short input_src);
unsigned char get_HDMI_Input_Source(void);
unsigned char get_AVD_Input_Source(void);
void set_AVD_Input_Source(unsigned short input_src);
unsigned short get_AVD_Input(void);
void set_AVD_Input(unsigned short input_port);
unsigned short get_ADC_Input(void);
void set_ADC_Input(unsigned short input_port);
void Set_Val_HDMI_Reply_Zero_Timing_mute_cnt(unsigned char cnt);
unsigned char Get_Val_HDMI_Reply_Zero_Timing_mute_cnt(void);

#define HDMI_TIME_PERIOD_THRESHOULD (100*90)//100 ms

typedef enum  {
	ACTUAL_DETECT_RESULT = 0,//report actual detect result
	CHECK_TIME_PERIOD = 1,//check polling time is smaller than threshold. If yes report zero timing
	DIRECT_REPORT_ZERO = 2,//biggern than this always return zero timing
}TIMING_REPORT_STAGE;

int VFE_AVD_OPEN(void);
int VFE_AVD_ATV_CONNECT(unsigned short PortNumber);
int VFE_AVD_AV_CONNECT(unsigned short PortNumber);
int VFE_AVD_SCART_CONNECT(unsigned short PortNumber);
unsigned short VFE_AVD_ATV_GetCurrentPort(void);

#ifdef CONFIG_CUSTOMER_TV010
unsigned char SetNonStandardFlag(unsigned int value);
unsigned int GetNonStandardFlag(void);
#endif
/* Get VDC detect timing flag. */
unsigned char VDC_get_detect_flag(void);
void VBI_module_init(VBI_INIT_MOUDLE_T * vbi_status);
void VBI_module_callback(VBI_REGISTER_CALLBACK_T * vbi_callback_info);
void VBI_support_type_set(KADP_VFE_AVD_CC_TTX_STATUS_T status,unsigned char enable);
unsigned char vbi_clk_get_enable(void);
#ifndef BUILD_QUICK_SHOW
long vfe_ioctl(struct file *file, unsigned int cmd,  unsigned long arg);
#endif
void Set_Reply_Zero_Timing_Flag(unsigned char src, unsigned char flag);
void VFE_ADC_Get_TimingInfo_Proc(KADP_ADC_TIMING_INFO_T * adctiminginfo);
void VFE_ADC_ADJ_SET_PC_Proc(ADC_ADJ_PC_T * adj_pc);
void VFE_ADC_ADJ_GET_PC_Proc(ADC_ADJ_PC_T * adj_pc);
unsigned int quickshow_onlinemeasure_status_init(void);

extern unsigned char (*check_polling_time_period)(unsigned int record_time, unsigned int cur_time, unsigned threshould);

//---------- for DP RX ------------
#ifndef BUILD_QUICK_SHOW
#include "scaler_vfedev-dprx.h"
#endif

#endif

