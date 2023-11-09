#ifndef __SCALER_VFEDEV_H
#define  __SCALER_VFEDEV_H

#define 	BIT0_VFE_AVD	0x00000001
#ifdef BUILD_QUICK_SHOW
#include <sysdefs.h>
#endif

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
	#include "../include/rtk_kdriver/scalercommon/scalerCommon.h"
#endif

#include <ioctrl/scaler/vfe_cmd_id.h>
#ifndef BUILD_QUICK_SHOW
#include <generated/autoconf.h>
#endif

#ifndef BOOLEAN
#ifndef _EMUL_WIN
typedef	unsigned int			__BOOLEAN;
#define BOOLEAN __BOOLEAN
#else
typedef	unsigned char		__BOOLEAN;
#define BOOLEAN __BOOLEAN
#endif
#endif

// #define AV_HSYNC_PULSE_START_REFINE 1 //mark due to HW improve

unsigned char get_ADC_Global_Status(void);
void set_ADC_Global_Status(SOURCE_STATUS status);
unsigned char get_AVD_Global_Status(void);
void set_AVD_Global_Status(SOURCE_STATUS status);
unsigned char get_ATV_Global_Status(void);
void set_ATV_Global_Status(SOURCE_STATUS status);
unsigned char get_AV_Global_Status(void);
void set_AV_Global_Status(SOURCE_STATUS status);
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
void HDMI_set_detect_flag(unsigned char enable);

#define HDMI_TIME_PERIOD_THRESHOULD (100*90)//100 ms

typedef enum  {
	ACTUAL_DETECT_RESULT = 0,//report actual detect result
	CHECK_TIME_PERIOD = 1,//check polling time is smaller than threshold. If yes report zero timing
	DIRECT_REPORT_ZERO = 2,//biggern than this always return zero timing
}TIMING_REPORT_STAGE;

int VFE_AVD_OPEN(void);
int VFE_AVD_ATV_CONNECT(unsigned short PortNumber);
unsigned short VFE_AVD_ATV_GetCurrentPort(void);

#ifdef CONFIG_CUSTOMER_TV010
unsigned char SetNonStandardFlag(unsigned int value);
unsigned int GetNonStandardFlag(void);
#endif
/* Get VDC detect timing flag. */
unsigned char VDC_get_detect_flag(void);
void VBI_support_type_set(KADP_VFE_AVD_CC_TTX_STATUS_T status,unsigned char enable);
unsigned char vbi_clk_get_enable(void);
#endif

extern unsigned char check_polling_time_period(unsigned int record_time, unsigned int cur_time, unsigned threshould);

//---------- for DP RX ------------
#ifndef BUILD_QUICK_SHOW
#include "scaler_vfedev-dprx.h"
#endif
