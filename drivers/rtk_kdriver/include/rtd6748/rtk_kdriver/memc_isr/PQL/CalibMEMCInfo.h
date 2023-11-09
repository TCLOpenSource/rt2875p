#ifndef _CALIBMEMCINFO_H
#define _CALIBMEMCINFO_H

#ifdef __cplusplus
extern "C" {
#endif
//#include "memc_isr/PQL/FRC_SceneAnalysis.h"
#define TAG_NAME_MEMC "MEMC"

#define _MIN_(x,y) (((x)<=(y))? (x) : (y))
#define _MAX_(x,y) (((x)>=(y))? (x) : (y))

typedef struct
{
	unsigned int 	u24_Gain_Scale;
	unsigned char	u8_Gain_Shift;
	int				s32_Offset;
}_CALIB_SINGLE_PARAM;

typedef enum{
	CALIB_PARAM_APL = 0,
	CALIB_PARAM_DTL,
	CALIB_PARAM_SAD,
	CALIB_PARAM_YUV,
	CALIB_PARAM_NUM,
}_CALIB_PARAM_INDEX;

typedef struct
{
	_CALIB_SINGLE_PARAM APL;
	_CALIB_SINGLE_PARAM DTL;
	_CALIB_SINGLE_PARAM SAD;
}_CALIB_PARAM;

unsigned int ReadComReg_Calib_Data(unsigned int u32_Data, unsigned char param_index, int u32_Min, unsigned int u32_Max, bool u1_RegionData);
unsigned int Calib_Data(unsigned int u32_Data, unsigned int u32_Gain, unsigned char u8_Shift);

#ifdef __cplusplus
}
#endif

#endif

