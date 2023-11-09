#ifndef __SCALER_ADC_H__
#define __SCALER_ADC_H__
//#define NEED_SELFAWB/*For do self calibration*/

/**
 * color format.
*/
typedef enum {
	ADC_COLOR_RGB 	= 0x00,
	ADC_COLOR_YUV422,
	ADC_COLOR_YUV444,
	ADC_COLOR_YUV420,
	ADC_COLOR_UNKNOW
} ADC_COLOR_SPACE_T;

/**
 * color depth
*/
typedef enum {

	ADC_COLOR_DEPTH_8B =0,
	ADC_COLOR_DEPTH_10B,
	ADC_COLOR_DEPTH_12B,
	ADC_COLOR_DEPTH_16B,

} ADC_COLOR_DEPTH_T;


void ADC_Open(void);
void ADC_Close(void);
unsigned char ADC_Connect(int srcType, unsigned short srcinput);
void ADC_CRT_Setting(unsigned char enable);
void ADC_Power_Control(unsigned char on_off);
void ADC_Initial_Setting(void);
void ADC_ABL_Setting_Off(void);
void ADC_Initial(void);
StructDisplayInfo * Get_ADC_Dispinfo(void);
ModeInformationType * Get_ADC_Timinginfo(void);
void ADC_OnlineMeasureError_Handler(int srcType);
void ADC_VSC_Setting(int src);
void ADC_Reset_TimingInfo(void);
unsigned short Get_ADC_PhaseValue(void);
void Set_ADC_PhaseValue(unsigned short phasevalue);
void ADC_Disable_AHS_SMT_Power(void);
#ifdef CONFIG_PM
void ADC_Suspend(void);
void ADC_Resume(void);
#endif
void ADC_Disconnect(void);
unsigned char adc_drv_timing_ready(void);
void reset_adc_timing_ready(void);

#endif
