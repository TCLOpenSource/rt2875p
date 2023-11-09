#ifndef _CALIBMEMCINFO_H
#define _CALIBMEMCINFO_H

#ifdef __cplusplus
extern "C" {
#endif
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
	CALIB_PARAM_SC,
	CALIB_PARAM_MV,
	CALIB_PARAM_MVRANGE,
	CALIB_PARAM_GMV_CNT,
	CALIB_PARAM_DHLOGO_SAD_TH,
	CALIB_PARAM_DHLOGO_MV_TH,
	CALIB_PARAM_DHLOGO_MV_DIFF_TH,
	CALIB_PARAM_NUM,
}_CALIB_PARAM_INDEX;

unsigned int ReadComReg_Calib_Data(unsigned int u32_Data, unsigned char param_index, int u32_Min, unsigned int u32_Max, bool u1_RegionData);
unsigned int Calib_Data(unsigned int u32_Data, unsigned int u32_Gain, unsigned char u8_Shift);
int Calib_MV(int s32_Data);

#ifdef __cplusplus
}
#endif

#endif

