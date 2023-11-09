/*******************************************************************************
* @file    scalerColor_engineermenu.c
* @brief
* @note    Copyright (c) 2017 RealTek Technology Co., Ltd.
*		   All rights reserved.
*		   No. 2, Innovation Road II,
*		   Hsinchu Science Park,
*		   Hsinchu 300, Taiwan
*
* @log
* Revision 0.1	2017/01/19
* create
*******************************************************************************/
/*******************************************************************************
 * Header include
******************************************************************************/

#include <tvscalercontrol/scaler/scalercolor_engmenu.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vdc/yc_separation_vpq.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <scaler/scalerCommon.h>
#include <tvscalercontrol/vip/st2094.h>
#include <rbus/ppoverlay_reg.h>

//extern unsigned int POD_DATA[6][289];
extern DRV_POD_DATA_table_t POD_DATA[3];

int PQ_function_page_value[HAL_VPQ_FUNCTION_ID_MAX_NUM] = {0};
extern VIP_ST2094_TBL ST2094_TBL;
HAL_VPQ_ENG_SIZE ENG_size = {
	HAL_VPQ_ENG_ITEM_MAX_NUM, HAL_VPQ_ENG_ID_MAX_NUM
};

HAL_VPQ_ENG_STRUCT ENG_STRUCT = {
	{	//ENG_ITEM_STRUCT
		{HAL_VPQ_ENG_VIP_QUALITY_Coef,	"Coef"},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	"Extend_Coef"},
	},
	{	//ENG_ID_STRUCT
#if 0
		{HAL_VPQ_ENG_VIP_QUALITY_Coef,	HAL_VPQ_ENG_DLTi,	"DLTi",	HAL_VPQ_ENG_scrollbox,	0,	255,		{0},},
		{HAL_VPQ_ENG_VIP_QUALITY_Coef,	HAL_VPQ_ENG_DCTi,	"DCTi",	HAL_VPQ_ENG_scrollbox,	0,	255,		{0},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_CDS,	"CDS",	HAL_VPQ_ENG_scrollbox,	0,	255,		{0},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_EMFMK2i,	"EMFMK2",	HAL_VPQ_ENG_scrollbox,	0,	255,		{0},},
#endif
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_LC_DEMO_Split_Screen,					"LC_Split_Screen",	HAL_VPQ_ENG_scrollbox,	0,	4,		{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_LC_DEMO_Step,	"LC_Step",	HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_LC_DEMO_Debug,	"LC_Debug",	HAL_VPQ_ENG_scrollbox,	0,	3,		{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_OSDHDR_DEMO_MODE,					"OSD_HDR",	HAL_VPQ_ENG_scrollbox,	0,	2,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_OD_EN_TEST,	"OD_EN_TEST",	HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_OD_BIT_TEST,	"OD_BIT_TEST",	HAL_VPQ_ENG_scrollbox,	0,	7,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_DEMURA_EN,		"DEMURA_EN_TEST",	HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_OTHER, 		(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_PQ_TABLE_BIN_EN,	"PQ Table load",	HAL_VPQ_ENG_combobox,	0,	1,		{{"not_bin"},{"bin"}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_I_De_Contour,	"I_De_Contour",	HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_VDPQ_PRJ_ID,	"VDPQ_Style",	HAL_VPQ_ENG_scrollbox,	0,	7,		{{{0}}},},
	},
};

HAL_VPQ_ENG_TWOLAYER_STRUCT ENG_TWOLAYER_STRUCT = {
	{	//ENG_ITEM_STRUCT
		{HAL_VPQ_ENG_VIP_QUALITY_Coef,		"Coef",		HAL_VPQ_Coef_ID_MAX_NUM},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	"Extend_Coef", 	HAL_VPQ_Extend_Coef_ID_MAX_NUM},
		{HAL_VPQ_ENG_OTHER,			"Others", 	HAL_VPQ_OTHER_ID_MAX_NUM},
		{HAL_VPQ_ENG_PQ_Function,			"function_toggle", 	HAL_VPQ_FUNCTION_ID_MAX_NUM},
		{HAL_VPQ_ENG_HDR,			"HDR", 	HAL_VPQ_HDR_ID_MAX_NUM},
		{HAL_VPQ_ENG_HDR,			"ICM_7_Color", 	HAL_VPQ_ICM_7_COLOR_MAX_NUM},
	},
	{	//ENG_ID_STRUCT
		{HAL_VPQ_ENG_VIP_QUALITY_Coef,		HAL_VPQ_ENG_DLTi,		"DLTi",			HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Coef,		HAL_VPQ_ENG_DCTi,		"DCTi",			HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_CDS,		"CDS",			HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_EMFMK2i,		"EMFMK2",		HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_LC_DEMO_Split_Screen,	"LC_Split_Screen",		HAL_VPQ_ENG_scrollbox,	0,	4,		{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_LC_DEMO_Step,	"LC_Step",		HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_LC_DEMO_Debug,	"LC_Debug", 	HAL_VPQ_ENG_scrollbox,	0,	3,		{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_OSDHDR_DEMO_MODE,	"OSD_HDR",		HAL_VPQ_ENG_scrollbox,	0,	2,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_OD_EN_TEST, 	"OD_EN_TEST",		HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_OD_BIT_TEST,	"OD_BIT_TEST",		HAL_VPQ_ENG_scrollbox,	0,	7,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_DEMURA_EN,		"DEMURA_EN_TEST",	HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_VDPQ_PRJ_ID,	"VDPQ_Style",		HAL_VPQ_ENG_scrollbox,	0,	7,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_I_De_Contour,	"I_De_Contour", 	HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER, 		HAL_VPQ_ENG_PQ_TABLE_BIN_EN,		"PQ Table load",	HAL_VPQ_ENG_combobox,	0,	1,		{{{"not_bin"}},{{"bin"}}},},
		{HAL_VPQ_ENG_PQ_Function,	HAL_VPQ_FUNCTION_PCID,	"PCID", HAL_VPQ_ENG_combobox, 0, 3, {{{"OFF"}},{{"VALC"}},{{"lineOD"}}},},
		{HAL_VPQ_ENG_PQ_Function,	HAL_VPQ_FUNCTION_OD,	"OD", HAL_VPQ_ENG_scrollbox, 0, 1, {{{0}}},},
		{HAL_VPQ_ENG_PQ_Function,	HAL_VPQ_FUNCTION_Inv_Output_Gamma,	"Inv_Output_Gamma", HAL_VPQ_ENG_scrollbox, 0, Output_InvOutput_Gamma_TBL_MAX, {{{0}}},},
		{HAL_VPQ_ENG_PQ_Function,	HAL_VPQ_FUNCTION_Output_Gamma,	"Output_Gamma", HAL_VPQ_ENG_scrollbox, 0, Output_InvOutput_Gamma_TBL_MAX, {{{0}}},},
		{HAL_VPQ_ENG_PQ_Function,	HAL_VPQ_FUNCTION_RGBW,	"RGBW", HAL_VPQ_ENG_checkbox, 0, 1, {{{0}}},},
		{HAL_VPQ_ENG_PQ_Function,	HAL_VPQ_FUNCTION_Demura,	"Demura", HAL_VPQ_ENG_checkbox, 0, 1, {{{0}}},},
		{HAL_VPQ_ENG_HDR,			HAL_VPQ_ENG_ST2094_EN,		"ST2094 En",	HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_HDR,			HAL_VPQ_ENG_ST2094_EOTF_Gain,		"ST2094 EOTF Gain", HAL_VPQ_ENG_combobox,	0,	4,		{{{"12000"}},{{"11000"}},{{"10000"}},{{"9000"}},{{"8000"}}},},
		{HAL_VPQ_ENG_HDR,			HAL_VPQ_ENG_ST2094_EOTF_Linear_W,		"ST2094 EOTF Linear W", HAL_VPQ_ENG_combobox,	0,	4,		{{{"500"}},{{"400"}},{{"300"}},{{"200"}},{{"100"}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_R_START,	"R_START",	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_R_END,		"R_END", 	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_R_HUE,		"R_HUE", 	HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_R_SAT,		"R_SAT", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_R_ITN,		"R_ITN", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_SKIN_START,	"SKIN_START",	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_SKIN_END,		"SKIN_END", 	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_SKIN_HUE,		"SKIN_HUE", 	HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_SKIN_SAT,		"SKIN_SAT", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_SKIN_ITN,		"SKIN_ITN", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_Y_START,	"Y_START",	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_Y_END,		"Y_END", 	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_Y_HUE,		"Y_HUE", 	HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_Y_SAT,		"Y_SAT", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_Y_ITN,		"Y_ITN", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_G_START,	"G_START",	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_G_END,		"G_END", 	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_G_HUE,		"G_HUE", 	HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_G_SAT,		"G_SAT", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_G_ITN,		"G_ITN", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_C_START,	"C_START",	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_C_END,		"C_END", 	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_C_HUE,		"C_HUE", 	HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_C_SAT,		"C_SAT", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_C_ITN,		"C_ITN", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_B_START,	"B_START",	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_B_END,		"B_END", 	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_B_HUE,		"B_HUE", 	HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_B_SAT,		"B_SAT", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_B_ITN,		"B_ITN", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_M_START,	"M_START",	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_M_END,		"M_END", 	HAL_VPQ_ENG_scrollbox,	0,	48,		{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_M_HUE,		"M_HUE", 	HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_M_SAT,		"M_SAT", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
		{HAL_VPQ_ENG_ICM_7_COLOR,			HAL_VPQ_ENG_M_ITN,		"M_ITN", 	HAL_VPQ_ENG_scrollbox,	0,	1023,	{{{0}}},},
	},
};

HAL_VPQ_ENG_STRUCT* Scaler_Get_ENGMENU(void)
{
	return &ENG_STRUCT;
}

HAL_VPQ_ENG_SIZE* Scaler_Get_ENGMENU_size(void)
{
	return &ENG_size;
}


unsigned int Scaler_Get_ENGMENU_ID(unsigned int ID)
{
	unsigned int ret = 0;

#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
	static char OSD_HDRMODE_DEMO_init = 0;
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	switch (ID) {
		case HAL_VPQ_ENG_DLTi:
			ret = (unsigned int)Scaler_GetDLti();
			break;
		case HAL_VPQ_ENG_DCTi:
			ret = (unsigned int)Scaler_GetDCti();
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_CDS):
			ret = (unsigned int)Scaler_GetCDSTable();
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_EMFMK2i):
			ret = (unsigned int)Scaler_GetEMF_Mk2();
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_LC_DEMO_Split_Screen):
			ret = (unsigned int)Scaler_color_LC_DemoSplit_Screen(255);
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_LC_DEMO_Step):
			ret = (unsigned int)Scaler_color_LC_DemoStep(255);
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_LC_DEMO_Debug):
			ret = (unsigned int)Scaler_color_get_LC_DebugMode();
			break;
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_OSDHDR_DEMO_MODE):
			ret = 0;
			if(OSD_HDRMODE_DEMO_init == 0){
				// init table
				fwif_color_set_OSD_enhance_init();
				OSD_HDRMODE_DEMO_init = 1;
			}
			break;
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
		case (HAL_VPQ_ENG_OD_EN_TEST+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_Get_OD_Enable();
			break;
		case (HAL_VPQ_ENG_OD_BIT_TEST+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_Get_OD_Bits();
			break;
		case (HAL_VPQ_ENG_VDPQ_PRJ_ID+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = drvif_module_vpq_get_ProjectId();
			break;
		case (HAL_VPQ_ENG_DEMURA_EN+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			fwif_color_DeMura_init();
			ret = Scaler_Get_DeMura_En();
			break;
		case (HAL_VPQ_ENG_PQ_TABLE_BIN_EN+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_Get_PQ_table_bin_En();
			break;
		case (HAL_VPQ_ENG_I_De_Contour+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = VIP_system_info_structure_table->PQ_Setting_Info.I_De_Contour_CTRL.table_select;
			break;
	}

	return ret;
}

unsigned int Scaler_Set_ENGMENU_ID(unsigned int ID, int value)
{
	unsigned int ret = 0;
	switch (ID) {
		case HAL_VPQ_ENG_DLTi:
			Scaler_SetDLti((unsigned char)value);
			break;
		case HAL_VPQ_ENG_DCTi:
			Scaler_SetDCti((unsigned char)value);
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_CDS):
			Scaler_SetCDSTable((unsigned char)value);
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_EMFMK2i):
			Scaler_SetEMF_Mk2((unsigned char)value);
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_LC_DEMO_Split_Screen):
			ret = (unsigned int)Scaler_color_LC_DemoSplit_Screen(value);
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_LC_DEMO_Step):
			ret = (unsigned int)Scaler_color_LC_DemoStep(value);
			break;
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_LC_DEMO_Debug):
			if(value==1)
				value = 2;
			else if(value==2)
				value = 1;
			Scaler_color_set_LC_DebugMode((unsigned char)value);
			break;
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_OSDHDR_DEMO_MODE):
			fwif_color_set_OSD_enhance_enable(value);
			break;
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
		case (HAL_VPQ_ENG_OD_EN_TEST+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_OD_Test(value);
			break;
		case (HAL_VPQ_ENG_OD_BIT_TEST+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_Set_OD_Bits(value);
			break;
		case (HAL_VPQ_ENG_DEMURA_EN+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_Set_DeMura_En(value);
			break;
		case (HAL_VPQ_ENG_PQ_TABLE_BIN_EN+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			break;
		case (HAL_VPQ_ENG_I_De_Contour+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = fwif_color_Set_I_De_Contour(value);
			break;
		case (HAL_VPQ_ENG_VDPQ_PRJ_ID+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = drvif_module_vpq_set_ProjectId(value);
			drvif_module_vpq_SetYcSep(VDC_YCSEPARATE_DEFAULT);
			break;
	}

	return ret;
}


/*==================Two Layer============================================*/
HAL_VPQ_ENG_TWOLAYER_STRUCT* Scaler_Get_ENG_TWOLAYER_MENU(void)
{
	return &ENG_TWOLAYER_STRUCT;
}

unsigned int Scaler_Get_VIP_QUALITY_Coef_ID(unsigned int ID)
{
	unsigned int ret = 0;
	switch (ID) {
		case HAL_VPQ_ENG_DLTi:
			ret = (unsigned int)Scaler_GetDLti();
			break;
		case HAL_VPQ_ENG_DCTi:
			ret = (unsigned int)Scaler_GetDCti();
			break;
	}
	return ret;
}

unsigned int Scaler_Get_VIP_QUALITY_Extend_Coef_ID(unsigned int ID)
{
	unsigned int ret = 0;
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
	static char OSD_HDRMODE_DEMO_init = 0;
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE

	switch (ID) {
		case HAL_VPQ_ENG_CDS:
			ret = (unsigned int)Scaler_GetCDSTable();
			break;
		case HAL_VPQ_ENG_EMFMK2i:
			ret = (unsigned int)Scaler_GetEMF_Mk2();
			break;
		case HAL_VPQ_ENG_LC_DEMO_Split_Screen:
			ret = (unsigned int)Scaler_color_LC_DemoSplit_Screen(255);
			break;
		case HAL_VPQ_ENG_LC_DEMO_Step:
			ret = (unsigned int)Scaler_color_LC_DemoStep(255);
			break;
		case HAL_VPQ_ENG_LC_DEMO_Debug:
			ret = (unsigned int)Scaler_color_get_LC_DebugMode();
			break;
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
		case HAL_VPQ_ENG_OSDHDR_DEMO_MODE:
			ret = 0;
			if(OSD_HDRMODE_DEMO_init == 0){
				// init table
				fwif_color_set_OSD_enhance_init();
				OSD_HDRMODE_DEMO_init = 1;
			}
			break;
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE

	}
	return ret;
}

unsigned int Scaler_Get_ENG_OTHER_ID(unsigned int ID)
{
	unsigned int ret = 0;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	switch (ID) {
		case HAL_VPQ_ENG_OD_EN_TEST:
			ret = Scaler_Get_OD_Enable();
			break;
		case HAL_VPQ_ENG_OD_BIT_TEST:
			ret = Scaler_Get_OD_Bits();
			break;
		case HAL_VPQ_ENG_DEMURA_EN:
		{
			static unsigned char demura_inited = FALSE;
			if (!demura_inited && fwif_color_DeMura_init()==0)
				demura_inited = TRUE;
			ret = Scaler_Get_DeMura_En();
			break;
		}
		case HAL_VPQ_ENG_PQ_TABLE_BIN_EN:
			ret = Scaler_Get_PQ_table_bin_En();
			break;
		case HAL_VPQ_ENG_I_De_Contour:
			if(VIP_system_info_structure_table ==NULL){
				ret = 0;
			}else{
				ret = VIP_system_info_structure_table->PQ_Setting_Info.I_De_Contour_CTRL.table_select;
			}
			break;
		case HAL_VPQ_ENG_VDPQ_PRJ_ID:
			ret = drvif_module_vpq_get_ProjectId();
			break;
	}
	return ret;
}

unsigned int Scaler_Get_ENG_HDR_ID(unsigned int ID)
{
	unsigned int ret = 0;
	//_system_setting_info *VIP_system_info_structure_table = NULL;
	//VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_ST2094_CTRL* ST2094_CTRL;

	ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();

	switch (ID) {
		case HAL_VPQ_ENG_ST2094_EN:
			ret = drvif_fwif_color_Get_ST2094_demo_flag(ST2094_engMenu_layer);
			if (ret == ST2094_Demo_ON_HDR10)
				ret = 0;
			else
				ret = 1;
			break;

		case HAL_VPQ_ENG_ST2094_EOTF_Gain:
			if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div == (ST2094_TBL.EOTF_norm_div + 20000))
				ret = 0;
			else if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div == (ST2094_TBL.EOTF_norm_div + 10000))
				ret = 1;
			else if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div == (ST2094_TBL.EOTF_norm_div - 10000))
				ret = 3;
			else if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div == (ST2094_TBL.EOTF_norm_div - 20000))
				ret = 4;
			else
				ret = 2;
			break;

		case HAL_VPQ_ENG_ST2094_EOTF_Linear_W:
			if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W == (ST2094_TBL.EOTF_Linear_Blend_W + 200))
				ret = 0;
			else if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W == (ST2094_TBL.EOTF_Linear_Blend_W + 100))
				ret = 1;
			else if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W == (ST2094_TBL.EOTF_Linear_Blend_W - 100))
				ret = 3;
			else if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W == (ST2094_TBL.EOTF_Linear_Blend_W - 200))
				ret = 4;
			else
				ret = 2;
			break;

	}
	return ret;
}

unsigned int Scaler_Get_ENG_ICM_7_COLOR_ID(unsigned int ID)
{
	unsigned int ret = 0;
	switch (ID) {
		case HAL_VPQ_ENG_R_START:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(0,0);
			break;
		case HAL_VPQ_ENG_R_END:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(1,0);
			break;
		case HAL_VPQ_ENG_R_HUE:
			ret = Scaler_get_ICM_7Axis_Adjust(_HUE_ELEM,0);
			break;
		case HAL_VPQ_ENG_R_SAT:
			ret = Scaler_get_ICM_7Axis_Adjust(_SAT_ELEM,0);
			break;
		case HAL_VPQ_ENG_R_ITN:
			ret = Scaler_get_ICM_7Axis_Adjust(_ITN_ELEM,0);
			break;
		case HAL_VPQ_ENG_SKIN_START:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(0,1);
			break;
		case HAL_VPQ_ENG_SKIN_END:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(1,1);
			break;
		case HAL_VPQ_ENG_SKIN_HUE:
			ret = Scaler_get_ICM_7Axis_Adjust(_HUE_ELEM,1);
			break;
		case HAL_VPQ_ENG_SKIN_SAT:
			ret = Scaler_get_ICM_7Axis_Adjust(_SAT_ELEM,1);
			break;
		case HAL_VPQ_ENG_SKIN_ITN:
			ret = Scaler_get_ICM_7Axis_Adjust(_ITN_ELEM,1);
			break;
		case HAL_VPQ_ENG_Y_START:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(0,2);
			break;
		case HAL_VPQ_ENG_Y_END:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(1,2);
			break;
		case HAL_VPQ_ENG_Y_HUE:
			ret = Scaler_get_ICM_7Axis_Adjust(_HUE_ELEM,2);
			break;
		case HAL_VPQ_ENG_Y_SAT:
			ret = Scaler_get_ICM_7Axis_Adjust(_SAT_ELEM,2);
			break;
		case HAL_VPQ_ENG_Y_ITN:
			ret = Scaler_get_ICM_7Axis_Adjust(_ITN_ELEM,2);
			break;
		case HAL_VPQ_ENG_G_START:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(0,3);
			break;
		case HAL_VPQ_ENG_G_END:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(1,3);
			break;
		case HAL_VPQ_ENG_G_HUE:
			ret = Scaler_get_ICM_7Axis_Adjust(_HUE_ELEM,3);
			break;
		case HAL_VPQ_ENG_G_SAT:
			ret = Scaler_get_ICM_7Axis_Adjust(_SAT_ELEM,3);
			break;
		case HAL_VPQ_ENG_G_ITN:
			ret = Scaler_get_ICM_7Axis_Adjust(_ITN_ELEM,3);
			break;
		case HAL_VPQ_ENG_C_START:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(0,4);
			break;
		case HAL_VPQ_ENG_C_END:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(1,4);
			break;
		case HAL_VPQ_ENG_C_HUE:
			ret = Scaler_get_ICM_7Axis_Adjust(_HUE_ELEM,4);
			break;
		case HAL_VPQ_ENG_C_SAT:
			ret = Scaler_get_ICM_7Axis_Adjust(_SAT_ELEM,4);
			break;
		case HAL_VPQ_ENG_C_ITN:
			ret = Scaler_get_ICM_7Axis_Adjust(_ITN_ELEM,4);
			break;
		case HAL_VPQ_ENG_B_START:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(0,5);
			break;
		case HAL_VPQ_ENG_B_END:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(1,5);
			break;
		case HAL_VPQ_ENG_B_HUE:
			ret = Scaler_get_ICM_7Axis_Adjust(_HUE_ELEM,5);
			break;
		case HAL_VPQ_ENG_B_SAT:
			ret = Scaler_get_ICM_7Axis_Adjust(_SAT_ELEM,5);
			break;
		case HAL_VPQ_ENG_B_ITN:
			ret = Scaler_get_ICM_7Axis_Adjust(_ITN_ELEM,5);
			break;
		case HAL_VPQ_ENG_M_START:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(0,6);
			break;
		case HAL_VPQ_ENG_M_END:
			ret = fwif_color_icm_get_ICM_H_7axis_adjust_HSI(1,6);
			break;
		case HAL_VPQ_ENG_M_HUE:
			ret = Scaler_get_ICM_7Axis_Adjust(_HUE_ELEM,6);
			break;
		case HAL_VPQ_ENG_M_SAT:
			ret = Scaler_get_ICM_7Axis_Adjust(_SAT_ELEM,6);
			break;
		case HAL_VPQ_ENG_M_ITN:
			ret = Scaler_get_ICM_7Axis_Adjust(_ITN_ELEM,6);
			break;
	}
	return ret;
}


unsigned int Scaler_Get_ENG_PQ_FUNCTION_ID(unsigned int ID)
{
	unsigned int ret = 0;
	static unsigned char demura_inited = FALSE;
//	unsigned char InputLUT[578] = {0};

	switch (ID) {
		case HAL_VPQ_FUNCTION_PCID:
			ret = PQ_function_page_value[ID];
			//===============re run
#if 1
			if(PQ_function_page_value[ID] == 0)
				fwif_color_set_POD_DATA_table(&(POD_DATA[0]));
			else if(PQ_function_page_value[ID] == 1)
				fwif_color_set_POD_DATA_table(&(POD_DATA[1]));
			else if(PQ_function_page_value[ID] == 2)
				fwif_color_set_POD_DATA_table(&(POD_DATA[2]));
#endif
#if 0
			if(PQ_function_page_value[ID] == 0)
				fwif_color_set_PCID_BypassSetting();
			else if(PQ_function_page_value[ID] == 1)
				fwif_color_set_PCID_VALCSetting();
			else if(PQ_function_page_value[ID] == 2)
				fwif_color_set_PCID_LineOD_DualGateSetting();

			//R
			memcpy(&(InputLUT[0]), &(POD_DATA[0*2][0]), sizeof(unsigned char)*289);
			memcpy(&(InputLUT[289]), &(POD_DATA[0*2+1][0]), sizeof(unsigned char)*289);
			fwif_color_set_pcid2_valuetable_channel(InputLUT,0);
			//G
			memcpy(&(InputLUT[0]), &(POD_DATA[1*2][0]), sizeof(unsigned char)*289);
			memcpy(&(InputLUT[289]), &(POD_DATA[1*2+1][0]), sizeof(unsigned char)*289);
			fwif_color_set_pcid2_valuetable_channel(InputLUT,1);
			//B
			memcpy(&(InputLUT[0]), &(POD_DATA[2*2][0]), sizeof(unsigned char)*289);
			memcpy(&(InputLUT[289]), &(POD_DATA[2*2+1][0]), sizeof(unsigned char)*289);
			fwif_color_set_pcid2_valuetable_channel(InputLUT,2);
#endif
			break;
		case HAL_VPQ_FUNCTION_OD:
			ret = Scaler_Get_OD_Enable();
			//===============re run
			Scaler_OD_Test(ret);
			break;
		case HAL_VPQ_FUNCTION_Inv_Output_Gamma:
			ret = PQ_function_page_value[ID];
			//===============re run
			fwif_color_colorwrite_InvOutput_gamma((unsigned char)ret);
			break;
		case HAL_VPQ_FUNCTION_Output_Gamma:
			ret = PQ_function_page_value[ID];
			//===============re run
			fwif_color_colorwrite_Output_gamma((unsigned char)ret);
			break;
		/*case HAL_VPQ_FUNCTION_RGBW:
			if(drvif_color_Get_RGBW_Mode() == 2)
				ret = 1;
			else
				ret = 0;
			//===============re run
			if(ret == 0){
				IoReg_Mask32(PPOVERLAY_Display_Timing_CTRL2_reg, ~(_BIT19|_BIT20|_BIT21), 0);
			}else{
				drvif_color_set_RGBW();
			}
			break;*/
		case HAL_VPQ_FUNCTION_Demura:
			if (!demura_inited && fwif_color_DeMura_init()==0)
				demura_inited = TRUE;
			ret = Scaler_Get_DeMura_En();
			//===============re run
			Scaler_Set_DeMura_En(ret);
			break;
	}
	if (ID < HAL_VPQ_FUNCTION_ID_MAX_NUM)
		PQ_function_page_value[ID] = ret;
	return ret;
}

unsigned int Scaler_Get_ENG_TWOLAYER_MENU_ID(unsigned int ITEM, unsigned int ID)
{
	unsigned int ret = 0;
	switch (ITEM){
		case HAL_VPQ_ENG_VIP_QUALITY_Coef:
			ret = Scaler_Get_VIP_QUALITY_Coef_ID(ID);
			break;
		case HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef:
			ret = Scaler_Get_VIP_QUALITY_Extend_Coef_ID(ID);
			break;
		case HAL_VPQ_ENG_OTHER:
			ret = Scaler_Get_ENG_OTHER_ID(ID);
			break;
		case HAL_VPQ_ENG_PQ_Function:
			ret = Scaler_Get_ENG_PQ_FUNCTION_ID(ID);
			break;
		case HAL_VPQ_ENG_HDR:
			ret = Scaler_Get_ENG_HDR_ID(ID);
			break;
		case HAL_VPQ_ENG_ICM_7_COLOR:
			ret = Scaler_Get_ENG_ICM_7_COLOR_ID(ID);
			break;
	}
	return ret;
}

unsigned int Scaler_Set_VIP_QUALITY_Coef_ID(unsigned int ID, int value)
{
	unsigned int ret = 0;
	switch (ID) {
		case HAL_VPQ_ENG_DLTi:
			Scaler_SetDLti((unsigned char)value);
			break;
		case HAL_VPQ_ENG_DCTi:
			Scaler_SetDCti((unsigned char)value);
			break;
	}
	return ret;
}
unsigned int Scaler_Set_VIP_QUALITY_Extend_Coef_ID(unsigned int ID, int value)
{
	unsigned int ret = 0;
	switch (ID) {
		case HAL_VPQ_ENG_CDS:
			Scaler_SetCDSTable((unsigned char)value);
			break;
		case HAL_VPQ_ENG_EMFMK2i:
			Scaler_SetEMF_Mk2((unsigned char)value);
			break;
		case HAL_VPQ_ENG_LC_DEMO_Split_Screen:
			ret = (unsigned int)Scaler_color_LC_DemoSplit_Screen(value);
			break;
		case HAL_VPQ_ENG_LC_DEMO_Step:
			value = value+1;
			ret = (unsigned int)Scaler_color_LC_DemoStep(value);
			break;
		case HAL_VPQ_ENG_LC_DEMO_Debug:
			if(value==1)
				value = 2;
			else if(value==2)
				value = 1;
			Scaler_color_set_LC_DebugMode((unsigned char)value);
			break;
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
		case HAL_VPQ_ENG_OSDHDR_DEMO_MODE:
			fwif_color_set_OSD_enhance_enable(value);
			break;
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
	}
	return ret;
}

unsigned int Scaler_Set_ENG_OTHER_ID(unsigned int ID, int value)
{
	unsigned int ret = 0;
	switch (ID) {
		case HAL_VPQ_ENG_OD_EN_TEST:
			ret = Scaler_OD_Test(value);
			break;
		case HAL_VPQ_ENG_OD_BIT_TEST:
			ret = Scaler_Set_OD_Bits(value);
			break;
		case HAL_VPQ_ENG_DEMURA_EN:
			ret = Scaler_Set_DeMura_En(value);
			break;
		case HAL_VPQ_ENG_PQ_TABLE_BIN_EN:
			break;
		case HAL_VPQ_ENG_I_De_Contour:
			ret = fwif_color_Set_I_De_Contour(value);
			break;
		case HAL_VPQ_ENG_VDPQ_PRJ_ID:
			ret = drvif_module_vpq_set_ProjectId(value);
			drvif_module_vpq_SetYcSep(VDC_YCSEPARATE_DEFAULT);
			break;
	}
	return ret;
}

unsigned int Scaler_Set_ENG_HDR_ID(unsigned int ID, int value)
{
	unsigned int ret = 0;
	VIP_ST2094_CTRL* ST2094_CTRL;

	ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();

	switch (ID) {
		case HAL_VPQ_ENG_ST2094_EN:
			if (value == 0)
				drvif_fwif_color_Set_ST2094_demo_flag(ST2094_Demo_ON_HDR10, ST2094_engMenu_layer);
			else
				drvif_fwif_color_Set_ST2094_demo_flag(ST2094_Demo_OFF, ST2094_engMenu_layer);
			break;

		case HAL_VPQ_ENG_ST2094_EOTF_Gain:
			if (value == 0)
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div = ST2094_TBL.EOTF_norm_div + 20000;
			else if (value == 1)
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div = ST2094_TBL.EOTF_norm_div + 10000;
			else if (value == 3)
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div = ST2094_TBL.EOTF_norm_div - 10000;
			else if (value == 4)
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div = ST2094_TBL.EOTF_norm_div - 20000;
			else
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div = ST2094_TBL.EOTF_norm_div;
			break;

		case HAL_VPQ_ENG_ST2094_EOTF_Linear_W:
			if (value == 0)
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W = ST2094_TBL.EOTF_Linear_Blend_W + 200;
			else if (value == 1)
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W = ST2094_TBL.EOTF_Linear_Blend_W + 100;
			else if (value == 3)
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W = ST2094_TBL.EOTF_Linear_Blend_W - 100;
			else if (value == 4)
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W = ST2094_TBL.EOTF_Linear_Blend_W - 200;
			else
				drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W = ST2094_TBL.EOTF_Linear_Blend_W;
			break;

	}
	return ret;
}

unsigned int Scaler_Set_ENG_ICM_7_COLOR_ID(unsigned int ID, int value)
{
	unsigned int ret = 0;
	int which_table = 0;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	ICM_H_7axis_table *tICM_H_7axis = NULL;
	StructColorDataType *pTable = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	Scaler_Get_Display_info(&display,&src_idx);
	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
	{
		printk("Scaler_Set_ENG_ICM_7_COLOR_ID, pTable == NULL\n");
		return 0;
	}
	which_table = pTable->ICM_7Axis_Table;
	if (which_table >= VIP_ICM_TBL_X)
		return 0;
	if (gVip_Table == NULL) {
		printk("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	tICM_H_7axis = &(gVip_Table->tICM_H_7axis[which_table]);
	if (tICM_H_7axis == NULL)
	{
		printk("Scaler_Set_ENG_ICM_7_COLOR_ID, tICM_H_7axis == NULL\n");
		return 0;
	}


	switch (ID) {
		case HAL_VPQ_ENG_R_START:
			*(&(tICM_H_7axis->R[0]) + ((0*5)+0)) = value;
			break;
		case HAL_VPQ_ENG_R_END:
			*(&(tICM_H_7axis->R[0]) + ((0*5)+1)) = value;
			break;
		case HAL_VPQ_ENG_R_HUE:
			//*(&(tICM_H_7axis->R[0]) + ((0*5)+2)) = value;
			Scaler_set_ICM_7Axis_Adjust(_HUE_ELEM,value,0);
			break;
		case HAL_VPQ_ENG_R_SAT:
			//*(&(tICM_H_7axis->R[0]) + ((0*5)+3)) = value;
			Scaler_set_ICM_7Axis_Adjust(_SAT_ELEM,value,0);
			break;
		case HAL_VPQ_ENG_R_ITN:
			//*(&(tICM_H_7axis->R[0]) + ((0*5)+4)) = value;
			Scaler_set_ICM_7Axis_Adjust(_ITN_ELEM,value,0);
			break;
		case HAL_VPQ_ENG_SKIN_START:
			*(&(tICM_H_7axis->R[0]) + ((1*5)+0)) = value;
			break;
		case HAL_VPQ_ENG_SKIN_END:
			*(&(tICM_H_7axis->R[0]) + ((1*5)+1)) = value;
			break;
		case HAL_VPQ_ENG_SKIN_HUE:
			//*(&(tICM_H_7axis->R[0]) + ((1*5)+2)) = value;
			Scaler_set_ICM_7Axis_Adjust(_HUE_ELEM,value,1);
			break;
		case HAL_VPQ_ENG_SKIN_SAT:
			//*(&(tICM_H_7axis->R[0]) + ((1*5)+3)) = value;
			Scaler_set_ICM_7Axis_Adjust(_SAT_ELEM,value,1);
			break;
		case HAL_VPQ_ENG_SKIN_ITN:
			//*(&(tICM_H_7axis->R[0]) + ((1*5)+4)) = value;
			Scaler_set_ICM_7Axis_Adjust(_ITN_ELEM,value,1);
			break;
		case HAL_VPQ_ENG_Y_START:
			*(&(tICM_H_7axis->R[0]) + ((2*5)+0)) = value;
			break;
		case HAL_VPQ_ENG_Y_END:
			*(&(tICM_H_7axis->R[0]) + ((2*5)+1)) = value;
			break;
		case HAL_VPQ_ENG_Y_HUE:
			//*(&(tICM_H_7axis->R[0]) + ((2*5)+2)) = value;
			Scaler_set_ICM_7Axis_Adjust(_HUE_ELEM,value,2);
			break;
		case HAL_VPQ_ENG_Y_SAT:
			//*(&(tICM_H_7axis->R[0]) + ((2*5)+3)) = value;
			Scaler_set_ICM_7Axis_Adjust(_SAT_ELEM,value,2);
			break;
		case HAL_VPQ_ENG_Y_ITN:
			//*(&(tICM_H_7axis->R[0]) + ((2*5)+4)) = value;
			Scaler_set_ICM_7Axis_Adjust(_ITN_ELEM,value,2);
			break;
		case HAL_VPQ_ENG_G_START:
			*(&(tICM_H_7axis->R[0]) + ((3*5)+0)) = value;
			break;
		case HAL_VPQ_ENG_G_END:
			*(&(tICM_H_7axis->R[0]) + ((3*5)+1)) = value;
			break;
		case HAL_VPQ_ENG_G_HUE:
			//*(&(tICM_H_7axis->R[0]) + ((3*5)+2)) = value;
			Scaler_set_ICM_7Axis_Adjust(_HUE_ELEM,value,3);
			break;
		case HAL_VPQ_ENG_G_SAT:
			//*(&(tICM_H_7axis->R[0]) + ((3*5)+3)) = value;
			Scaler_set_ICM_7Axis_Adjust(_SAT_ELEM,value,3);
			break;
		case HAL_VPQ_ENG_G_ITN:
			//*(&(tICM_H_7axis->R[0]) + ((3*5)+4)) = value;
			Scaler_set_ICM_7Axis_Adjust(_ITN_ELEM,value,3);
			break;
		case HAL_VPQ_ENG_C_START:
			*(&(tICM_H_7axis->R[0]) + ((4*5)+0)) = value;
			break;
		case HAL_VPQ_ENG_C_END:
			*(&(tICM_H_7axis->R[0]) + ((4*5)+1)) = value;
			break;
		case HAL_VPQ_ENG_C_HUE:
			//*(&(tICM_H_7axis->R[0]) + ((4*5)+2)) = value;
			Scaler_set_ICM_7Axis_Adjust(_HUE_ELEM,value,4);
			break;
		case HAL_VPQ_ENG_C_SAT:
			//*(&(tICM_H_7axis->R[0]) + ((4*5)+3)) = value;
			Scaler_set_ICM_7Axis_Adjust(_SAT_ELEM,value,4);
			break;
		case HAL_VPQ_ENG_C_ITN:
			//*(&(tICM_H_7axis->R[0]) + ((4*5)+4)) = value;
			Scaler_set_ICM_7Axis_Adjust(_ITN_ELEM,value,4);
			break;
		case HAL_VPQ_ENG_B_START:
			*(&(tICM_H_7axis->R[0]) + ((5*5)+0)) = value;
			break;
		case HAL_VPQ_ENG_B_END:
			*(&(tICM_H_7axis->R[0]) + ((5*5)+1)) = value;
			break;
		case HAL_VPQ_ENG_B_HUE:
			//*(&(tICM_H_7axis->R[0]) + ((5*5)+2)) = value;
			Scaler_set_ICM_7Axis_Adjust(_HUE_ELEM,value,5);
			break;
		case HAL_VPQ_ENG_B_SAT:
			//*(&(tICM_H_7axis->R[0]) + ((5*5)+3)) = value;
			Scaler_set_ICM_7Axis_Adjust(_SAT_ELEM,value,5);
			break;
		case HAL_VPQ_ENG_B_ITN:
			//*(&(tICM_H_7axis->R[0]) + ((5*5)+4)) = value;
			Scaler_set_ICM_7Axis_Adjust(_ITN_ELEM,value,5);
			break;
		case HAL_VPQ_ENG_M_START:
			*(&(tICM_H_7axis->R[0]) + ((6*5)+0)) = value;
			break;
		case HAL_VPQ_ENG_M_END:
			*(&(tICM_H_7axis->R[0]) + ((6*5)+1)) = value;
			break;
		case HAL_VPQ_ENG_M_HUE:
			//*(&(tICM_H_7axis->R[0]) + ((6*5)+2)) = value;
			Scaler_set_ICM_7Axis_Adjust(_HUE_ELEM,value,6);
			break;
		case HAL_VPQ_ENG_M_SAT:
			//*(&(tICM_H_7axis->R[0]) + ((6*5)+3)) = value;
			Scaler_set_ICM_7Axis_Adjust(_SAT_ELEM,value,6);
			break;
		case HAL_VPQ_ENG_M_ITN:
			//*(&(tICM_H_7axis->R[0]) + ((6*5)+4)) = value;
			Scaler_set_ICM_7Axis_Adjust(_ITN_ELEM,value,6);
			break;
	}
	return ret;
}


unsigned int Scaler_Set_ENG_PQ_FUNCION_ID(unsigned int ID, int value)
{
	unsigned int ret = 0;
//	unsigned char InputLUT[578] = {0};
	switch (ID) {
		case HAL_VPQ_FUNCTION_PCID:
#if 1
			if(value == 0)
				fwif_color_set_POD_DATA_table(&(POD_DATA[0]));
			else if(value == 1)
				fwif_color_set_POD_DATA_table(&(POD_DATA[1]));
			else if(value == 2)
				fwif_color_set_POD_DATA_table(&(POD_DATA[2]));
#endif
#if 0
			if(value == 0)
				fwif_color_set_PCID_BypassSetting();
			else if(value == 1)
				fwif_color_set_PCID_VALCSetting();
			else if(value == 2)
				fwif_color_set_PCID_LineOD_DualGateSetting();

			//R
			memcpy(&(InputLUT[0]), &(POD_DATA[0*2][0]), sizeof(unsigned char)*289);
			memcpy(&(InputLUT[289]), &(POD_DATA[0*2+1][0]), sizeof(unsigned char)*289);
			fwif_color_set_pcid2_valuetable_channel(InputLUT,0);
			//G
			memcpy(&(InputLUT[0]), &(POD_DATA[1*2][0]), sizeof(unsigned char)*289);
			memcpy(&(InputLUT[289]), &(POD_DATA[1*2+1][0]), sizeof(unsigned char)*289);
			fwif_color_set_pcid2_valuetable_channel(InputLUT,1);
			//B
			memcpy(&(InputLUT[0]), &(POD_DATA[2*2][0]), sizeof(unsigned char)*289);
			memcpy(&(InputLUT[289]), &(POD_DATA[2*2+1][0]), sizeof(unsigned char)*289);
			fwif_color_set_pcid2_valuetable_channel(InputLUT,2);
#endif
			ret = value;
			break;
		case HAL_VPQ_FUNCTION_OD:
			ret = Scaler_OD_Test(value);
			break;
		case HAL_VPQ_FUNCTION_Inv_Output_Gamma:
			fwif_color_colorwrite_InvOutput_gamma((unsigned char)value);
			ret =  value;
			break;
		case HAL_VPQ_FUNCTION_Output_Gamma:
			fwif_color_colorwrite_Output_gamma((unsigned char)value);
			ret =  value;
			break;
		/*case HAL_VPQ_FUNCTION_RGBW:
//			ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_REG;
			if(value == 0){
				IoReg_Mask32(PPOVERLAY_Display_Timing_CTRL2_reg, ~(_BIT19|_BIT20|_BIT21), 0);
			}else{
				drvif_color_set_RGBW();
			}
			ret = value;
			break;*/
		case HAL_VPQ_FUNCTION_Demura:
			ret = Scaler_Set_DeMura_En(value);
			break;
	}
	if (ID < HAL_VPQ_FUNCTION_ID_MAX_NUM)
		PQ_function_page_value[ID] = ret;
	return ret;
}

unsigned int Scaler_Set_ENG_TWOLAYER_MENU_ID(unsigned int ITEM, unsigned int ID, int value)
{
	unsigned int ret = 0;
	switch (ITEM) {
		case HAL_VPQ_ENG_VIP_QUALITY_Coef:
			Scaler_Set_VIP_QUALITY_Coef_ID(ID, value);
			break;
		case HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef:
			Scaler_Set_VIP_QUALITY_Extend_Coef_ID(ID, value);
			break;
		case HAL_VPQ_ENG_OTHER:
			Scaler_Set_ENG_OTHER_ID(ID, value);
			break;
		case HAL_VPQ_ENG_PQ_Function:
			Scaler_Set_ENG_PQ_FUNCION_ID(ID, value);
			break;
		case HAL_VPQ_ENG_HDR:
			Scaler_Set_ENG_HDR_ID(ID, value);
			break;
		case HAL_VPQ_ENG_ICM_7_COLOR:
			Scaler_Set_ENG_ICM_7_COLOR_ID(ID, value);
			break;
	}

	return ret;
}
/*==============================================================*/



