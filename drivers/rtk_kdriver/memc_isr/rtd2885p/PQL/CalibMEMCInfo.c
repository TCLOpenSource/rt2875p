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

const _CALIB_SINGLE_PARAM CALIB_PARAM[_PQL_OUT_RESOLUTION_MAX][CALIB_PARAM_NUM] = {

	{ //_PQL_OUT_1920x1080
	//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
		{	1,			0,			0}, //APL				Gain:1		Offset:0
		{	1,			0,			0}, //DTL				Gain:1		Offset:0
		{	1,			0,			0}, //SAD				Gain:1		Offset:0
		{	1,			0,			0}, //sc th 			Gain:1		Offset:0
		{	1,			0,			0}, //mv				Gain:1		Offset:0
		{	1,			0,			0}, //mv SR 			Gain:1		Offset:0
		{	1,			0, 	 		0}, //gmv cnt 			Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_sad_th 	Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_th		Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_diff_th Gain:1		Offset:0
	},
		
	{ //_PQL_OUT_1920x540
	//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
		{	1,			0,			0}, //APL				Gain:1		Offset:0
		{	1,			0,			0}, //DTL				Gain:1		Offset:0
		{	1,			0,			0}, //SAD				Gain:1		Offset:0
		{	1,			0,			0}, //sc th 			Gain:1		Offset:0
		{	1,			0,			0}, //mv				Gain:1		Offset:0
		{	1,			0,			0}, //mv SR 			Gain:1		Offset:0
		{	1,			0, 	 		0}, //gmv cnt 			Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_sad_th 	Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_th		Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_diff_th Gain:1		Offset:0
	},
		
	{ //_PQL_OUT_3840x2160
	//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
		{	1,			0,			0}, //APL				Gain:1		Offset:0
		{	1,			0,			0}, //DTL				Gain:1		Offset:0
		{	1,			0,			0}, //SAD				Gain:1		Offset:0
		{	1,			0,			0}, //sc th 			Gain:1		Offset:0
		{	1,			0,			0}, //mv				Gain:1		Offset:0
		{	1,			0,			0}, //mv SR 			Gain:1		Offset:0
		{	1,			0, 	 		0}, //gmv cnt 			Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_sad_th 	Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_th		Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_diff_th Gain:1		Offset:0
	},
		
	{ //_PQL_OUT_3840x1080
	//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
		{	1,			0,			0}, //APL				Gain:1		Offset:0
		{	1,			0,			0}, //DTL				Gain:1		Offset:0
		{	1,			0,			0}, //SAD				Gain:1		Offset:0
		{	1,			0,			0}, //sc th 			Gain:1		Offset:0
		{	1,			0,			0}, //mv				Gain:1		Offset:0
		{	1,			0,			0}, //mv SR 			Gain:1		Offset:0
		{	1,			0, 	 		0}, //gmv cnt 			Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_sad_th 	Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_th		Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_diff_th Gain:1		Offset:0
	},
		
	{ //_PQL_OUT_3840x540
	//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
		{	1,			0,			0}, //APL				Gain:1		Offset:0
		{	1,			0,			0}, //DTL				Gain:1		Offset:0
		{	1,			0,			0}, //SAD				Gain:1		Offset:0
		{	1,			0,			0}, //sc th 			Gain:1		Offset:0
		{	1,			0,			0}, //mv				Gain:1		Offset:0
		{	1,			0,			0}, //mv SR 			Gain:1		Offset:0
		{	1,			0, 	 		0}, //gmv cnt 			Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_sad_th 	Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_th		Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_diff_th Gain:1		Offset:0
	},
		
	{ //_PQL_OUT_768X480
	//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
		{	1,			0,			0}, //APL				Gain:1		Offset:0
		{	1,			0,			0}, //DTL				Gain:1		Offset:0
		{	1,			0,			0}, //SAD				Gain:1		Offset:0
		{	1,			0,			0}, //sc th 			Gain:1		Offset:0
		{	1,			0,			0}, //mv				Gain:1		Offset:0
		{	1,			0,			0}, //mv SR 			Gain:1		Offset:0
		{	1,			0, 	 		0}, //gmv cnt 			Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_sad_th 	Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_th		Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_diff_th Gain:1		Offset:0
	},
	
	{ //_PQL_OUT_1280X720
	//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
		{	1,			0,			0}, //APL				Gain:1		Offset:0
		{	1,			0,			0}, //DTL				Gain:1		Offset:0
		{	1,			0,			0}, //SAD				Gain:1		Offset:0
		{	1,			0,			0}, //sc th 			Gain:1		Offset:0
		{	1,			0,			0}, //mv				Gain:1		Offset:0
		{	1,			0,			0}, //mv SR 			Gain:1		Offset:0
		{	1,			0, 	 		0}, //gmv cnt 			Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_sad_th 	Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_th		Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_diff_th Gain:1		Offset:0
	},
		
	{ //_PQL_OUT_7680X4320
	//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
		{	1,			0,			0}, //APL				Gain:1		Offset:0
		{	1,			0,			0}, //DTL				Gain:1		Offset:0
		{	1,			0,			0}, //SAD				Gain:1		Offset:0
		{	1,			0,			0}, //sc th 			Gain:1		Offset:0
		{	1,			0,			0}, //mv				Gain:1		Offset:0
		{	1,			0,			0}, //mv SR 			Gain:1		Offset:0
		{	1,			0, 	 		0}, //gmv cnt 			Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_sad_th 	Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_th		Gain:1		Offset:0
		{	1,			0,			0}, //dhLogo_mv_diff_th Gain:1		Offset:0
	},

	{ //_PQL_OUT_2560x1440
	//gain_scale	gain_shift	  offset   ex: new_data = (old_data*gain_scale)>>gain_shift + offset
		{2271,			10, 		0}, //APL				Gain:2.218		Offset:0
		{2337,			10, 		0}, //DTL				Gain:2.282		Offset:0
		{2637,			10, 		0}, //SAD				Gain:2.575		Offset:0
		{ 397,			10, 		0}, //sc th 			Gain:0.610		Offset:0
		{1536,			10, 		0}, //mv				Gain:1.5		Offset:0
		{ 683,			10, 		0}, //mv SR 			Gain:0.667		Offset:0
		{2758,			10, 	 	0}, //gmv cnt 			Gain:1.197*2.25	Offset:0
		{	3,			 0, 		0}, //dhLogo_sad_th 	Gain:3			Offset:0
		{ 683,			10, 		0}, //dhLogo_mv_th		Gain:0.667		Offset:0
		{ 683,			10, 		0}, //dhLogo_mv_diff_th Gain:1.5		Offset:0
	},
};

unsigned int ReadComReg_Calib_Data(unsigned int u32_Data, unsigned char param_index, int u32_Min, unsigned int u32_Max, bool u1_RegionData)
{
	PQL_OUTPUT_RESOLUTION output_Resolution = GetPQLContext()->_external_data._output_resolution;
	int s32_Offset = (u1_RegionData) ? ((CALIB_PARAM[output_Resolution][param_index].s32_Offset)>>5) : CALIB_PARAM[output_Resolution][param_index].s32_Offset;

	if(u32_Data < s32_Offset){
		return (unsigned int)_MAX_(_MIN_((((unsigned long long)u32_Data * CALIB_PARAM[output_Resolution][param_index].u24_Gain_Scale) >> (CALIB_PARAM[output_Resolution][param_index].u8_Gain_Shift)), u32_Max), u32_Min);
	}
	else if(u32_Data < s32_Offset*9){
		s32_Offset = (u32_Data-s32_Offset)>>3;
		return (unsigned int)_MAX_(_MIN_((((unsigned long long)u32_Data * CALIB_PARAM[output_Resolution][param_index].u24_Gain_Scale) >> (CALIB_PARAM[output_Resolution][param_index].u8_Gain_Shift))+s32_Offset, u32_Max), u32_Min);
	}
	return (unsigned int)_MAX_(_MIN_((((unsigned long long)u32_Data * CALIB_PARAM[output_Resolution][param_index].u24_Gain_Scale) >> (CALIB_PARAM[output_Resolution][param_index].u8_Gain_Shift))+s32_Offset, u32_Max), u32_Min);
}

unsigned int Calib_Data(unsigned int u32_Data, unsigned int u32_Gain, unsigned char u8_Shift)
{
	return (unsigned int)(((unsigned long long)u32_Data*u32_Gain)>>u8_Shift);
}

int Calib_MV(int s32_Data)
{
	PQL_OUTPUT_RESOLUTION output_Resolution = GetPQLContext()->_external_data._output_resolution;

	return (int)(((long long)s32_Data*(CALIB_PARAM[output_Resolution][CALIB_PARAM_MV].u24_Gain_Scale))>>(CALIB_PARAM[output_Resolution][CALIB_PARAM_MV].u8_Gain_Shift));
}

