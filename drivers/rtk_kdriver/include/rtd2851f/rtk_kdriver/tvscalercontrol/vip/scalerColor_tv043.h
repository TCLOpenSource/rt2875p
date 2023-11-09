#ifndef __SCALER_COLOR_TV043_H__
#define __SCALER_COLOR_TV043_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <tvscalercontrol/vip/scalerColor.h>

#ifndef SINT8
typedef	signed char				__SINT8;
#define SINT8 __SINT8
#endif

#ifndef SINT16
typedef	signed short			__SINT16;
#define SINT16 __SINT16
#endif

/*------------- parameter definition ----------*/
typedef enum {
	INPUT_DATA_8BITS = 0,
	INPUT_DATA_10BITS,
	INPUT_DATA_12BITS,
	Input_Data_MAX,
} INPUT_DATA_RANGE;

typedef enum {
	LUT_17POINTS = 0,
	LUT_33POINTS,
	LUT_65POINTS,
	LUT_257POINTS,
	LUT_MAX,
} LUT_LENGTH;

typedef enum {
	MOVE_LD_TO_LDSPI_BYHW = 0,
	MOVE_LD_TO_LDSPI_BYSW_120HZ,
	MOVE_LD_TO_LDSPI_BYSW_60HZ,
	MOVE_LD_TO_LDSPI_MAX,
} MOVE_LD_TO_LDSPI;

/*------------- function prototye ------------*/
void fwif_color_set_color_temp_tv043(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con, short Red_Bri, short Green_Bri, short Blue_Bri);
void fwif_color_set_LD_Boost_TV043(unsigned short *DutyLimit);
void fwif_color_LD_SW_1DLUT(unsigned short* input, unsigned short* output, unsigned short length, unsigned short *LUT, INPUT_DATA_RANGE input_data_range, LUT_LENGTH lut_length);
unsigned char fwif_color_get_LD_Pattern_Flag_tv043(void);
void fwif_color_set_LD_Pattern_Flag_tv043(unsigned char pattern_flag);
void fwif_color_LD_SW_Temporal_Filter_tv043(unsigned short* current_value, unsigned short *target_value, unsigned short step, unsigned short cnt);
void fwif_color_LD_Dynamic_System_tv043(void);

MOVE_LD_TO_LDSPI fwif_color_get_LD_To_LDSPI_Method_tv043(void);
void fwif_color_set_LD_To_LDSPI_Method_tv043(MOVE_LD_TO_LDSPI method);
void fwif_color_LD_Set_LD_To_LDSPI_BySW_Calculate(MOVE_LD_TO_LDSPI method);
void fwif_color_LD_Set_LD_To_LDSPI_BySW_Apply(void);
void fwif_color_LD_Boost_1DLUT_SWProcess_tv043(unsigned short* backlight_input, unsigned short* backlight_boost);



#ifdef __cplusplus
}
#endif

#endif /* __SCALER_COLOR_TV043_H__*/


