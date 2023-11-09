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
#include <tvscalercontrol/vip/ST2094.h>
#include <tvscalercontrol/vip/ai_pq.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <scalercommon/scalerCommon.h>
#endif
#include <rbus/color_sharp_reg.h>
#include <rbus/di_reg.h>
#include <rbus/de_xcxl_reg.h>
#include <rbus/color_sharp_reg.h>

#include <tvscalercontrol/io/ioregdrv.h>
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
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_LC_DEMO_Step,	"LC_Step",	HAL_VPQ_ENG_scrollbox,	0,	5,		{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_OSDHDR_DEMO_MODE,					"OSD_HDR",	HAL_VPQ_ENG_scrollbox,	0,	3,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_OD_EN_TEST,	"OD_EN_TEST",	HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_OD_BIT_TEST,	"OD_BIT_TEST",	HAL_VPQ_ENG_scrollbox,	6,	8,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_DEMURA_EN,		"DEMURA_EN_TEST",	HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_I_De_Contour,	"I_De_Contour",	HAL_VPQ_ENG_scrollbox,	0,	3,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_SR,	"SR",	HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_MCNR,	"MCNR",	HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_DeXc,	"DeXc",	HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)+HAL_VPQ_ENG_VDPQ_PRJ_ID,	"VDPQ_Style",	HAL_VPQ_ENG_scrollbox,	0,	7,		{{{0}}},},
	},
};

HAL_VPQ_ENG_TWOLAYER_STRUCT ENG_TWOLAYER_STRUCT = {
	{	//ENG_ITEM_STRUCT
		{HAL_VPQ_ENG_VIP_QUALITY_Coef,		"Coef",		HAL_VPQ_Coef_ID_MAX_NUM},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	"Extend_Coef", 	HAL_VPQ_Extend_Coef_ID_MAX_NUM},
		{HAL_VPQ_ENG_OTHER,			"Others", 	HAL_VPQ_OTHER_ID_MAX_NUM},
		{HAL_VPQ_ENG_HDR,			"HDR", 	HAL_VPQ_HDR_ID_MAX_NUM},
		{HAL_VPQ_ENG_AI,			"AI", 	HAL_VPQ_AI_ID_MAX_NUM},
	},
	{	//ENG_ID_STRUCT
		{HAL_VPQ_ENG_VIP_QUALITY_Coef,		HAL_VPQ_ENG_DLTi,		"DLTi",			HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Coef,		HAL_VPQ_ENG_DCTi,		"DCTi",			HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_CDS,		"CDS",			HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_EMFMK2i,		"EMFMK2",		HAL_VPQ_ENG_scrollbox,	0,	255,	{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_LC_DEMO_Split_Screen,	"LC_Split_Screen",		HAL_VPQ_ENG_scrollbox,	0,	4,		{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_LC_DEMO_Step,	"LC_Step",		HAL_VPQ_ENG_scrollbox,	0,	5,		{{{0}}},},
		{HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,	HAL_VPQ_ENG_OSDHDR_DEMO_MODE,	"OSD_HDR",		HAL_VPQ_ENG_scrollbox,	0,	3,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_OD_EN_TEST, 	"OD_EN_TEST",		HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_OD_BIT_TEST,	"OD_BIT_TEST",		HAL_VPQ_ENG_scrollbox,	6,	8,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_DEMURA_EN,		"DEMURA_EN_TEST",	HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_I_De_Contour,	"I_De_Contour", 	HAL_VPQ_ENG_scrollbox,	0,	3,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_SR,	"SR", 	HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_MCNR,	"MCNR", 	HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_DeXc,	"DeXc", 	HAL_VPQ_ENG_scrollbox,	0,	1,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_VDPQ_PRJ_ID,	"VDPQ_Style",		HAL_VPQ_ENG_scrollbox,	0,	7,		{{{0}}},},
		{HAL_VPQ_ENG_OTHER,			HAL_VPQ_ENG_PQ_Bypass,	"PQ_Bypass_Enable",		HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_HDR,			HAL_VPQ_ENG_ST2094_EN,		"ST2094 En",	HAL_VPQ_ENG_combobox,	0,	1,		{{{"OFF"}},{{"ON"}}},},
		{HAL_VPQ_ENG_HDR,			HAL_VPQ_ENG_ST2094_EOTF_Gain,		"ST2094 EOTF Gain",	HAL_VPQ_ENG_combobox,	0,	4,		{{{"12000"}},{{"11000"}},{{"10000"}},{{"9000"}},{{"8000"}}},},
		{HAL_VPQ_ENG_HDR,			HAL_VPQ_ENG_ST2094_EOTF_Linear_W,		"ST2094 EOTF Linear W",	HAL_VPQ_ENG_combobox,	0,	4,		{{{"500"}},{{"400"}},{{"300"}},{{"200"}},{{"100"}}},},
		{HAL_VPQ_ENG_AI,			HAL_VPQ_ENG_AI_FACE_MODE,		"Face Mode",	HAL_VPQ_ENG_scrollbox,	0,	2,		{{{0}}},},
		{HAL_VPQ_ENG_AI,			HAL_VPQ_ENG_AI_SQM_MODE,		"SQM Mode",		HAL_VPQ_ENG_scrollbox,	0,	2,		{{{0}}},},
		{HAL_VPQ_ENG_AI,			HAL_VPQ_ENG_AI_SCENE_MODE,		"Scene Mode",	HAL_VPQ_ENG_scrollbox,	0,	2,		{{{0}}},},
	}
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
	static char OSD_HDRMODE_DEMO_init = 0;
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
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_OSDHDR_DEMO_MODE):
			ret = 0;
			if(OSD_HDRMODE_DEMO_init == 0){
				// init table
				fwif_color_set_OSD_enhance_init();
				OSD_HDRMODE_DEMO_init = 1;
			}
			break;
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
			fwif_color_DeMura_init(1, 0);
			ret = Scaler_Get_DeMura_En();
			break;
		case (HAL_VPQ_ENG_I_De_Contour+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret =1;
			break;
		case (HAL_VPQ_ENG_SR+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = (IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL1_reg)&_BIT4)>>4;
			break;
		case (HAL_VPQ_ENG_MCNR+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = 1;
			break;
		case (HAL_VPQ_ENG_DeXc+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = 1;
			break;
	}

	return ret;
}
extern unsigned char Eng_Skyworth_Decont_Pattern;
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
		case (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_ENG_OSDHDR_DEMO_MODE):
			fwif_color_set_OSD_enhance_enable(value);
			break;
		case (HAL_VPQ_ENG_OD_EN_TEST+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_OD_Test(value);
			break;
		case (HAL_VPQ_ENG_OD_BIT_TEST+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_Set_OD_Bits(value);
			break;
		case (HAL_VPQ_ENG_DEMURA_EN+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_Set_DeMura_En(value);
			break;
		case (HAL_VPQ_ENG_I_De_Contour+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = Scaler_color_I_De_Contour_Demo(value);
			break;
		case (HAL_VPQ_ENG_VDPQ_PRJ_ID+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			ret = drvif_module_vpq_set_ProjectId(value);
			drvif_module_vpq_SetYcSep(VDC_YCSEPARATE_DEFAULT);
			break;
		case (HAL_VPQ_ENG_SR+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			if(value){
				IoReg_Mask32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, ~_BIT11, 0);
				IoReg_Mask32(COLOR_SHARP_DM_SEGPK_CTRL1_reg, 0xffffffff, _BIT4);
				IoReg_Mask32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, 0xffffffff, _BIT11);
//				IoReg_SetBits(0xb802a000, _BIT6|_BIT7);
//				IoReg_SetBits(0xb802a000, _BIT7);
			}else{
				IoReg_Mask32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, ~_BIT11, 0);
				IoReg_Mask32(COLOR_SHARP_DM_SEGPK_CTRL1_reg, ~_BIT4, 0);
				IoReg_Mask32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, 0xffffffff, _BIT11);
//				IoReg_ClearBits(0xb802a000, _BIT6);
//				IoReg_SetBits(0xb802a000, _BIT7);
			}
			break;
		case (HAL_VPQ_ENG_MCNR+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			if(value){
				IoReg_Mask32(DI_IM_DI_RTNR_CONTROL_reg, ~(_BIT0|_BIT1), _BIT0|_BIT1);
			}else{
				IoReg_Mask32(DI_IM_DI_RTNR_CONTROL_reg, ~(_BIT0|_BIT1), 0);
			}
			IoReg_Mask32(DI_db_reg_ctl_reg, ~(_BIT0), _BIT0);
			break;
		case (HAL_VPQ_ENG_DeXc+(HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM)):
			if(value){
				IoReg_Mask32(DE_XCXL_De_XCXL_CTRL_reg, ~(_BIT0), _BIT0);
			}else{
				IoReg_Mask32(DE_XCXL_De_XCXL_CTRL_reg, ~(_BIT0), 0);
			}
			IoReg_Mask32(DE_XCXL_DE_XCXL_db_reg_ctl_reg, ~(_BIT0), _BIT0);

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
	static char OSD_HDRMODE_DEMO_init = 0;
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
		case HAL_VPQ_ENG_OSDHDR_DEMO_MODE:
			ret = 0;
			if(OSD_HDRMODE_DEMO_init == 0){
				// init table
				fwif_color_set_OSD_enhance_init();
				OSD_HDRMODE_DEMO_init = 1;
			}
			break;

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
			fwif_color_DeMura_init(1, 0);
			ret = Scaler_Get_DeMura_En();
			break;
		case HAL_VPQ_ENG_I_De_Contour:
			if(VIP_system_info_structure_table ==NULL){
				ret = 0;
			}else{
				ret = 1;
			}
			break;
		case HAL_VPQ_ENG_VDPQ_PRJ_ID:
			ret = drvif_module_vpq_get_ProjectId();
			break;
		case HAL_VPQ_ENG_SR:
			ret = (IoReg_Read32(COLOR_SHARP_DM_SEGPK_CTRL1_reg)&_BIT4)>>4;
			break;
		case HAL_VPQ_ENG_MCNR:
			ret = 1;
			break;
		case HAL_VPQ_ENG_DeXc:
			ret = 1;
			break;
		case HAL_VPQ_ENG_PQ_Bypass:
			ret = Scaler_Get_ENG_TWOLAYER_PQ_ByPass();
			break;
	}
	return ret;
}

unsigned int Scaler_Get_ENG_HDR_ID(unsigned int ID)
{
	unsigned int ret = 0;

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

unsigned int Scaler_Get_ENG_AI_ID(unsigned int ID)
{
	unsigned int ret = 0;
	#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	extern RTK_AI_PQ_mode aipq_mode;

	switch (ID) {
		case HAL_VPQ_ENG_AI_FACE_MODE:
			ret = aipq_mode.face_mode;
		break;
		case HAL_VPQ_ENG_AI_SQM_MODE:
			ret = aipq_mode.sqm_mode;
		break;
		case HAL_VPQ_ENG_AI_SCENE_MODE:
			ret = aipq_mode.scene_mode;
		break;
		default:
			return ret;
	}
	#endif
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
		case HAL_VPQ_ENG_HDR:
			ret = Scaler_Get_ENG_HDR_ID(ID);
			break;
		case HAL_VPQ_ENG_AI:
			ret = Scaler_Get_ENG_AI_ID(ID);
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
			ret = (unsigned int)Scaler_color_LC_DemoStep(value);
			break;
		case HAL_VPQ_ENG_OSDHDR_DEMO_MODE:
			fwif_color_set_OSD_enhance_enable(value);
			break;
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
		case HAL_VPQ_ENG_I_De_Contour:
			ret = Scaler_color_I_De_Contour_Demo(value);
			break;
		case HAL_VPQ_ENG_VDPQ_PRJ_ID:
			ret = drvif_module_vpq_set_ProjectId(value);
			drvif_module_vpq_SetYcSep(VDC_YCSEPARATE_DEFAULT);
			break;
		case HAL_VPQ_ENG_PQ_Bypass:
			Scaler_Set_ENG_TWOLAYER_PQ_ByPass(value);
			break;
		case HAL_VPQ_ENG_SR:
			if(value){
				IoReg_Mask32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, ~_BIT11, 0);
				IoReg_Mask32(COLOR_SHARP_DM_SEGPK_CTRL1_reg, 0xffffffff, _BIT4);
				IoReg_Mask32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, 0xffffffff, _BIT11);
//				IoReg_SetBits(0xb802a000, _BIT6|_BIT7);
//				IoReg_SetBits(0xb802a000, _BIT7);
			}else{
				IoReg_Mask32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, ~_BIT11, 0);
				IoReg_Mask32(COLOR_SHARP_DM_SEGPK_CTRL1_reg, ~_BIT4, 0);
				IoReg_Mask32(COLOR_SHARP_DM_NR_SHP_CTRL_reg, 0xffffffff, _BIT11);
//				IoReg_ClearBits(0xb802a000, _BIT6);
//				IoReg_SetBits(0xb802a000, _BIT7);
			}
			break;
		case HAL_VPQ_ENG_MCNR:
			if(value){
				IoReg_Mask32(DI_IM_DI_RTNR_CONTROL_reg, ~(_BIT0|_BIT1), _BIT0|_BIT1);
			}else{
				IoReg_Mask32(DI_IM_DI_RTNR_CONTROL_reg, ~(_BIT0|_BIT1), 0);
			}
			IoReg_Mask32(DI_db_reg_ctl_reg, ~(_BIT0), _BIT0);
			break;

		case HAL_VPQ_ENG_DeXc:
			if(value){
				IoReg_Mask32(DE_XCXL_De_XCXL_CTRL_reg, ~(_BIT0), _BIT0);
			}else{
				IoReg_Mask32(DE_XCXL_De_XCXL_CTRL_reg, ~(_BIT0), 0);
			}
			IoReg_Mask32(DE_XCXL_DE_XCXL_db_reg_ctl_reg, ~(_BIT0), _BIT0);

			break;
	}
	return ret;
}

unsigned int Scaler_Set_ENG_HDR_ID(unsigned int ID, int value)
{
	unsigned int ret = 0;
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

unsigned int Scaler_Set_ENG_AI_ID(unsigned int ID, int value)
{
	unsigned int ret = 0;
	#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	extern RTK_AI_PQ_mode aipq_mode;
	extern RTK_AI_PQ_mode aipq_mode_pre;
	extern int scalerAI_pq_mode_ctrl(RTK_AI_PQ_mode ai_pq_mode, unsigned char dcValue);
	
	switch (ID) {
		case HAL_VPQ_ENG_AI_FACE_MODE:
			aipq_mode.face_mode = value;
		break;
		case HAL_VPQ_ENG_AI_SQM_MODE:
			aipq_mode.sqm_mode = value;
		break;
		case HAL_VPQ_ENG_AI_SCENE_MODE:
			aipq_mode.scene_mode = value;
		break;
		default:
			return ret;
	}
	scalerAI_pq_mode_ctrl(aipq_mode, 0);
	
	aipq_mode_pre.face_mode = aipq_mode.face_mode;
	aipq_mode_pre.sqm_mode = aipq_mode.sqm_mode;
	aipq_mode_pre.scene_mode = aipq_mode.scene_mode;
	#endif
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
		case HAL_VPQ_ENG_HDR:
			Scaler_Set_ENG_HDR_ID(ID, value);
			break;
		case HAL_VPQ_ENG_AI:
			Scaler_Set_ENG_AI_ID(ID, value);
			break;
	}

	return ret;
}
/*==============================================================*/

unsigned int Scaler_Set_ENG_TWOLAYER_PQ_ByPass(int value)
{
	if (value == 1) {
		Scaler_color_Set_PQ_ByPass_Lv(7);
	} else {

		Scaler_SetDataFormatHandler();

		fwif_color_handler();
		fwif_color_video_quality_handler();
		fwif_color_vpq_pic_init();
		fwif_color_SetAutoMAFlag(1);
		Scaler_Set_OD_Bit_By_XML();
	}

	return 0;
}

unsigned int Scaler_Get_ENG_TWOLAYER_PQ_ByPass(void)
{
	unsigned int ret;
	ret = (unsigned int)Scaler_color_Get_PQ_ByPass_Lv();

	if (ret == 7)
		ret = 1;
	else
		ret = 0;
	return ret;
}


