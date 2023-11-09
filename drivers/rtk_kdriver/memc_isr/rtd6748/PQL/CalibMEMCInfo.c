#include <memc_isr/include/memc_lib.h>
#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#ifdef H5C2
#include "../common/include/rbus/mdomain_disp_main_reg.h"  //mdomain_disp_reg.h //spec_change
#endif
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/panel/panelapi.h>
#include "memc_reg_def.h"
#include "memc_isr/PQL/CalibMEMCInfo.h"

_CALIB_SINGLE_PARAM CALIB_PARAM[CALIB_PARAM_NUM] = {
//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
	{   1,			0,			0},	//APL		Gain:1		Offset:0
	{   1,			0,			0},	//DTL		Gain:1		Offset:0
	{	1,			0,			0},	//SAD		Gain:1		Offset:0
	{	1,			0,			0}, //YUV		Gain:1		Offset:0
};


unsigned int ReadComReg_Calib_Data(unsigned int u32_Data, unsigned char param_index, int u32_Min, unsigned int u32_Max, bool u1_RegionData)
{
	int s32_Offset = (u1_RegionData) ? ((CALIB_PARAM[param_index].s32_Offset)>>5) : CALIB_PARAM[param_index].s32_Offset;
	if(u32_Data < s32_Offset){
		return (unsigned int)_MAX_(_MIN_((((unsigned long long)u32_Data * CALIB_PARAM[param_index].u24_Gain_Scale) >> (CALIB_PARAM[param_index].u8_Gain_Shift)), u32_Max), u32_Min);
	}
	else if(u32_Data < s32_Offset*9){
		s32_Offset = (u32_Data-s32_Offset)>>3;
		return (unsigned int)_MAX_(_MIN_((((unsigned long long)u32_Data * CALIB_PARAM[param_index].u24_Gain_Scale) >> (CALIB_PARAM[param_index].u8_Gain_Shift))+s32_Offset, u32_Max), u32_Min);
	}
	return (unsigned int)_MAX_(_MIN_((((unsigned long long)u32_Data * CALIB_PARAM[param_index].u24_Gain_Scale) >> (CALIB_PARAM[param_index].u8_Gain_Shift))+s32_Offset, u32_Max), u32_Min);
}

unsigned int Calib_Data(unsigned int u32_Data, unsigned int u32_Gain, unsigned char u8_Shift)
{
	return (unsigned int)(((unsigned long long)u32_Data*u32_Gain)>>u8_Shift);
}


