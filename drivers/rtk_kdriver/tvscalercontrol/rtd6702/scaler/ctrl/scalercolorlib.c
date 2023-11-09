/*******************************************************************************
* @file    scalerColorLib.cpp
* @brief
* @note    Copyright (c) 2014 RealTek Technology Co., Ltd.
*		   All rights reserved.
*		   No. 2, Innovation Road II,
*		   Hsinchu Science Park,
*		   Hsinchu 300, Taiwan
*
* @log
* Revision 0.1	2014/01/27
* create
*******************************************************************************/
/*******************************************************************************
 * Header include
******************************************************************************/

#include <linux/delay.h>
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
//#include <mach/RPCDriver.h>
#include <linux/pageremap.h>
#include <mach/system.h>
#include <linux/hrtimer.h>

#include <tvscalercontrol/vip/vip_reg_def.h>
//#include <rbus/rbusHistogramReg.h>
//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/stb_reg.h>
#include <rbus/timer_reg.h>

#include "memc_isr/scalerMEMC.h"
//#include <gal/rtk_se_export.h>
#include <rbus/se_reg.h>
//#include <gal/rtk_se_lib.h>

//#include <rbus/rbusODReg.h>
//#include <rbus/rbusColorReg.h>
//#include <rbus/rbusCon_briReg.h>
//#include <rbus/rbusYuv2rgbReg.h>
//#include <rbus/rbusScaleupReg.h>
//#include <rbus/rbusColor_dccReg.h>
//#include <rbus/rbusGammaReg.h>
//#include <rbus/rbusInv_gammaReg.h>
#include <rbus/dm_reg.h>
#include <rbus/hdr_all_top_reg.h>
#include <rbus/vgip_reg.h>
#include <rbus/vodma_reg.h>
//#include "rbus/rbusHDMIReg.h"
#include <rbus/h3ddma_hsd_reg.h>

/*#include </merlin_rbus/hdr/hdr_all_top_reg.h>*/ //Need SW sync,  SW Team not sync from TV001

//#include <rbus/dtg_reg.h>

/*#include <Application/AppClass/SetupClass.h>*/

/*#include "tvscalercontrol/scaler/scalerSuperResolution.h"*/
/*#include <tvscalercontrol/scaler/scalertimer.h>*/
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv001.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv002.h>
#include <tvscalercontrol/scaler/scalercolorlib_IPQ.h>
/*#include <tvscalercontrol/scaler/scalerlib.h>*/
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/pipmp.h>

#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/dcc.h>
#include <tvscalercontrol/vip/intra.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/xc.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/frequency_detection.h>

#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/vip/localcontrast.h>

#include "vgip_isr/scalerVIP.h"
//#include "vgip_isr/scalerAI.h"

/*#include <Platform_Lib/Board/pcbMgr.h>*/
/*#include <Platform_Lib/Board/pcb.h>*/

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerDrvCommon.h>
#endif

#include <scaler_vscdev.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <linux/kthread.h>
#include <scaler_vpqdev.h>
#include <scaler_vpqdev_extern.h>
#include <scaler_vpqmemcdev.h>
#include <ioctrl/vpq/vpq_cmd_id.h>
#include <ioctrl/vpq/vpq_extern_cmd_id.h>
#if defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)|| defined(CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT)
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
#endif
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/vip/gibi_od.h>
//extern UINT16 OETF_LUT_HLG_DEFAULT_DCC_OFF[1025];
SLR_VIP_TABLE_CUSTOM_TV001	 *g_Share_Memory_VIP_TABLE_Custom_Struct_isr= NULL;
#include "rtk_vip_logger.h"

#ifndef CONFIG_MEMC_NOTSUPPORT
#include "memc_reg_def.h"
#endif

#define TAG_NAME "VPQ_"
#define LGDB_OFF
//extern UINT8 g_InvGamma;
extern unsigned char ColorFMT_601_in_IMD_Domain_Enable;
/*******************************************************************************
* Macro
******************************************************************************/
#define GET_USER_INPUT_SRC()					(Scaler_GetUserInputSrc(SLR_MAIN_DISPLAY))/*yuan 20130225*/

#define GET_UI_SOURCE_FROM_DISPLAY(display)		(Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_DATA_ARRAY_IDX))
#ifdef CONFIG_ENABLE_VIP_PSEUDO_RPC
#define vip_SendRPC vip_PseudoRPC
#else
#define vip_SendRPC Scaler_SendRPC
#endif

#define SCALERLIB_DEBUG

#ifdef SCALERLIB_DEBUG
  #define SLRLIB_PRINTF(format, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_ScalerCtrl_Color_DEBUG,format,##args)
#else
  #define SLRLIB_PRINTF(format, args...)
#endif

#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_ScalerCtrl_Color_DEBUG,fmt,##args)

#define	GET_MAGIC_PERIOD_STATUS()				((pstMagicPicInfo->magic_parameter & _BIT7)>>7)
#define	SET_MAGIC_PIP_TYPE(x)					(pstMagicPicInfo->magic_parameter = ((pstMagicPicInfo->magic_parameter & (~0x0F)) | x))

/*******************************************************************************
* Constant
******************************************************************************/
/*#define example  100000 */ /* 100Khz */

/* Enable or Disable VIP flag*/

#define SensMax									0xE0
#define SensMin									0x0b
#define RPC_Send_Addr 							0
#define ShareMemAddrDummy 						0

#define AMBIENT_SENCER_ARRAY_SIZE				 30
#define ENABLE_RTK_DCR 1

/*******************************************************************************
 * Structure
 ******************************************************************************/

/*typedef struct*/
/*{*/
/*} MID_example_Param_t;*/


/*******************************************************************************
* enumeration
******************************************************************************/
/*typedef enum*/
/*{*/
/*    MID_Example_SLEEPING = 0,*/
/*    MID_Example_RUNNING,*/
/*} MID_Example_Status_t;*/



/*******************************************************************************
* Variable
******************************************************************************/
/*static unsigned char gExample = 100000;*//* 100K */

/*static UINT16 R_gain_autoview = 128, G_gain_autoview = 128, B_gain_autoview = 128;*/

/*static UINT32  DNR_TABLE_32[5] = {DNR_T0_32, DNR_T1_32, DNR_T2_32, DNR_T3_32, DNR_T4_32};*/
/*static UINT32 DNR_TABLE_8[5] = {DNR_T0, DNR_T1, DNR_T2, DNR_T3, DNR_T4};*/
/*static UINT32 MPEGNR_TABLE_8[5] = {MPEGNR_T0, MPEGNR_T1, MPEGNR_T2, MPEGNR_T3, MPEGNR_T4};*/ /*yuan*/
static unsigned char PQ_bin_info = 0;
unsigned char ucIsPrintVIP = 1;
unsigned char TableDCC;
unsigned char TableNRTemporal;
unsigned char TableNRSpatial;
unsigned char TableNR_RTNR_Y;
unsigned char TableNR_RTNR_C;
unsigned char TableNR_MPEGNR_H;/*sync from AT*/
unsigned char TableNR_MPEGNR_V;/*sync from AT*/

unsigned int Main_total_pixel_inv;


BOOL Low_ConBri_BrightnessBoost_Disable;
BOOL sRGB_User_Sat_Mapping;
/*static UINT16 APL_value;*/
/*static UINT8 ABL_Need_Reset = 1;	*/ /*rock 2012/06/20 change ABL off->on not work*/
#define BRIGHTNESS_BOOST_TIMER 50

/*==== Compal use light sensor compute formula*/
unsigned int LS_DET_SUM;
unsigned char LS_DET_WIDTH = 5;
unsigned char LS_DET_A_SPEED = 10;
unsigned char LS_DET_S_SPEED = 10;
unsigned char LS_DET_AVE = 255;
unsigned char LS_DET_AVE_OLD = 255;
unsigned char LS_DET_MAX;
unsigned char LS_DET_MIN = 255;
unsigned char LS_MAX = 255;


#if 0
#ifdef OPEN_CELL_PANEL
unsigned int *ODtable_Coeff;    /*young vippanel*/
#endif
#endif
/*void * 	p_ODTABLE;*/
/*static unsigned int *TV_AutoNR_Signal_Status_Coef; */ /*2010 csfanchiang*/
unsigned int *i_edge_smooth_coef;    /*20120301 march*/


UINT8 Mega_DCR;
UINT8 Mega_DCR_En;
bool Mega_DCR_Force_Off;
UINT8 Mega_DCR_Weight = 80;
UINT16 Mega_DCR_count;
UINT16 Set_Mega_DCR_count = 280;
//unsigned char g_LGE_HDR_en = 0;	//20151105 roger, use  VIP_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En]	//20150912 roger


/*static UINT16 Ambient_Sencer_pre[AMBIENT_SENCER_ARRAY_SIZE] = {128, 128, 128, 128, 128, 128, 128, 128, 128, 128,*/
/*																128, 128, 128, 128, 128, 128, 128, 128, 128, 128,*/
/*																128, 128, 128, 128, 128, 128, 128, 128, 128, 128};	*/ /*the array size is 30 ==> MAX BlackAPL_M_ATAB, need to check from spec*/
/*static UINT8 pre_Ambient_Sensor_Detect;*/
/*static UINT8 Display_On_Flag;*/

/*static UINT8 Scaler_APL_BL_MAX;*/
/*static UINT8 Scaler_APL_BL_MIN;*/
/*#if defined(BUILD_TV035_1)*/
/*static UINT8 Scaler_APL_PREBL = 100;*/
/*#else*/
/*static UINT8 Scaler_APL_PREBL = -1;*/
/*#endif*/
static unsigned char m_nowSource = 255;

unsigned char Main_DCC_User_Define_Curve_Table_Select;

BOOL sRGB_User_Sat_Enable;
UINT16 R_Gamma_Clamp_Max;
UINT16 G_Gamma_Clamp_Max;
UINT16 B_Gamma_Clamp_Max;


static COLOR_CALLBACK_FUNC1 m_cbFunc_getQualityCoef;
static COLOR_CALLBACK_FUNC1 m_cbFunc_getQualityExtend;
static COLOR_CALLBACK_FUNC1 m_cbFunc_getQualityExtend2;
static COLOR_CALLBACK_FUNC1 m_cbFunc_getQualityExtend3;
static COLOR_CALLBACK_FUNC1 m_cbFunc_getQualityExtend4;
static COLOR_CALLBACK_FUNC2 m_cbFunc_getColorData;
static COLOR_CALLBACK_FUNC2 m_cbFunc_getColorFacMode;
static COLOR_CALLBACK_FUNC2 m_cbFunc_getColorTempData;
static COLOR_CALLBACK_FUNC2 m_cbFunc_getColorTempData_TSB;
static COLOR_CALLBACK_FUNC2 m_cbFunc_getColorTempLevel;
static COLOR_CALLBACK_FUNC2 m_cbFunc_getPictureModeData;
static COLOR_CALLBACK_FUNC2 m_cbFunc_getDnrMode;
static COLOR_CALLBACK_FUNC3 m_cbFunc_getCurPicMode;
static COLOR_CALLBACK_FUNC4 m_cbFunc_getIcmByTimingPicmode;	/*Elsie 20131224*/
static COLOR_CALLBACK_FUNC5 m_cbFunc_getQuality3Dmode;/*young 3Dtable*/
unsigned char SR_edge_gain;
unsigned char SR_texture_gain;

VIP_PANEL_BIT default_vip_panel_bit = VIP_PANEL_BIT10; /*fixed me for link pass*/

unsigned char Table_CDS;
unsigned short Table_EGSM_Postshp;

unsigned char g_picmode_Cinema = 0;
unsigned char g_HDR_color_format = 0;
unsigned char g_HDR_mode = 0;
unsigned char g_bFreqDet_Setting_by_ISR = false;

extern unsigned int vpq_project_id;
extern unsigned int vpqex_project_id;
extern unsigned short tRGB2YUV_COEF[RGB2YUV_COEF_MATRIX_MODE_Max][RGB2YUV_COEF_MATRIX_ITEMS_Max];
extern unsigned char g_tv002_demo_fun_flag;

extern RTK_DECONTOUR_T De_contour_level;
extern unsigned char g_bDeContourSaved;

extern UINT8 VipPQ_GetIntra_Ver2_TableSize(void);

static StructMagicPicInfo stMagicPicInfo = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static StructMagicPicInfo *pstMagicPicInfo = &stMagicPicInfo;
static int 				magic_step_x;
static int 				magic_step_y;
static int 				MAG_XSize;
static int 				MAG_YSize;
static int 				magic_zoom_step_x;
static int 				magic_zoom_step_y;
static int 				MAG_ZOOM_XSize;
static int 				MAG_ZOOM_YSize;

extern unsigned char vdc_power_status;

unsigned char MagicPicture_cnt;

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
extern unsigned int vpq_project_id;
#endif

static unsigned char m_bHDR_AutoRun=false;
static unsigned char m_bDCCProcess=false;

extern CHIP_LOCAL_CONTRAST_T glc_param;

extern UINT8 hdr_table[HDR_TABLE_NUM][HDR_ITEM_MAX];
extern UINT32 hdr_YUV2RGB[VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max][12];
extern UINT32 hdr_RGB2OPT[HDR_RGB2OPT_TABLE_NUM][9];
extern unsigned short HFC_HDR2_gamma_1[3][257];
extern unsigned short HFC_HDR2_gamma_2[3][257];
extern DRV_Color_Mapping  HDR_sub_colorMap[HDR_SUB_TABLE_NUM];
extern unsigned short HDR10_3DLUT_17x17x17[17*17*17][3];
extern UINT16 HDR10_3DLUT_24x24x24[24*24*24*3];
extern UINT32 EOTF_LUT_Linear[EOTF_size];
extern UINT16 OETF_LUT_Linear[OETF_size];
unsigned short OETF_LUT_ByMaxCLL_ByPanelLuminance[1025];
extern CHIP_SHARPNESS_UI_T tCurCHIP_SHARPNESS_UI_T;
extern CHIP_SR_UI_T tCurCHIP_SR_UI_T;


extern UINT32 hdr_YUV2RGB_HFC[3][VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max][12];

extern DRV_subHDR_YUV2RGB_CSMatrix HDR_sub_YUV2RGB[HDR_SUB_YUV2RGB_TABLE_Max];
extern DRV_Color_Mapping  HDR_sub_colorMap[HDR_SUB_TABLE_NUM];
extern unsigned short HDR2_gamma_22[257];
extern unsigned short HDR2_gamma_045[257];

S_VPQ_ModuleTestMode S_ModuleTestMode = {0};
//TV030
RTK_VPQ_COLORSPACE_INDEX ColoSpace_Index;

extern unsigned char vsc_get_adaptivestream_flag(unsigned char display);
extern unsigned char get_sub_OutputVencMode(void);
extern unsigned char Eng_Skyworth_Decont_Pattern;
/*******************************************************************************
* Program
******************************************************************************/
static void Scaler_MagicPictureInit(void);
static void mScalerDisableMagicPicture(void);
static void mScalerMagicStoreContrast(void);
static void mScalerMagicPictureColorOff(void);

extern void scalerVIP_quality_init(unsigned long arg);
extern unsigned int quality_init_value[QUALITY_INIT_MAX];

void LightSensorValRead(UINT8 pad, UINT8  *buf)
{
#if 0 //k5l hw remove , seem no use
	UINT32 valReg = ST_PAD0_VADDR;
	UINT32 tmp;

	if (pad > 4 || buf == NULL)
		return;

	valReg += (pad*4);
	tmp = IoReg_Read32(valReg);
	*buf = tmp & 0xFF;
#endif
}


void *fwif_color_get_VIP_Extend2_CoefArray(unsigned char src_idx)
{
	if (m_cbFunc_getQualityExtend2 == NULL)
		return NULL;

	return m_cbFunc_getQualityExtend2((VIP_SOURCE_TIMING)src_idx);
}

#if 0
VIP_QUALITY_Extend3_Coef *fwif_color_get_VIP_Extend3_CoefArray(unsigned char src_idx)
{
	if (m_cbFunc_getQualityExtend3 == NULL)
		return NULL;

	return (VIP_QUALITY_Extend3_Coef *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)src_idx);
}

/*young 3Dtable*/
VIP_QUALITY_3Dmode_Coef *fwif_color_get_VIP_3Dmode_CoefArray(unsigned char src_idx)/*young 3Dtable*/
{
	if (m_cbFunc_getQuality3Dmode == NULL)
		return NULL;

	return (VIP_QUALITY_3Dmode_Coef *)m_cbFunc_getQuality3Dmode((VIP_3DSOURCE_TIMING)src_idx);
}

VIP_QUALITY_Extend4_Coef *fwif_color_get_VIP_Extend4_CoefArray(unsigned char src_idx)
{
	if (m_cbFunc_getQualityExtend4 == NULL)
		return NULL;

	return (VIP_QUALITY_Extend4_Coef *)m_cbFunc_getQualityExtend4((VIP_SOURCE_TIMING)src_idx);
}
#endif

StructColorDataType *fwif_color_get_color_data(unsigned char src_idx, unsigned char timing)
{
	if (m_cbFunc_getColorData == NULL)
		return NULL;

	return (StructColorDataType *)m_cbFunc_getColorData(Scaler_InputSrcFormat(src_idx), timing, 0);
}

StructColorDataFacModeType *fwif_color_get_color_fac_mode(unsigned char src_idx, unsigned char timing)
{
	if (m_cbFunc_getColorFacMode == NULL)
		return NULL;

	/*return (StructColorDataFacModeType *)m_cbFunc_getColorFacMode(Scaler_InputSrcFormat(src_idx), timing, fwif_color_get_picture_mode(src_idx));*/
	return (StructColorDataFacModeType *)m_cbFunc_getColorFacMode(Scaler_InputSrcFormat(src_idx), timing, 0);
}

StructColorDataFacModeType *fwif_color_get_color_fac_mode_by_picture(unsigned char src_idx, unsigned char timing, unsigned char pic)
{
	if (m_cbFunc_getColorFacMode == NULL)
		return NULL;

	return (StructColorDataFacModeType *)m_cbFunc_getColorFacMode(Scaler_InputSrcFormat(src_idx), timing, pic);
}

SLR_COLORTEMP_DATA *fwif_color_get_color_tempset_using_colortmp(unsigned char colortmp)
{
	if (m_cbFunc_getColorTempData_TSB == NULL)
		return NULL;

	return (SLR_COLORTEMP_DATA *)m_cbFunc_getColorTempData_TSB(0, 0, colortmp);
}

SLR_COLORTEMP_DATA *scaler_get_color_tempset(unsigned char src_idx, unsigned char timing)
{
	if (m_cbFunc_getColorTempData == NULL)
		return NULL;

	return (SLR_COLORTEMP_DATA *)m_cbFunc_getColorTempData(Scaler_InputSrcFormat(src_idx), timing, 0);
}

SLR_COLORTEMP_LEVEL scaler_get_color_temp_level(unsigned char src_idx, unsigned char timing)
{
	unsigned char *pValue = NULL;
	SLR_COLORTEMP_LEVEL level;

	if (m_cbFunc_getColorTempLevel == NULL)
		return SLR_COLORTEMP_NORMAL;

	pValue = (unsigned char *)m_cbFunc_getColorTempLevel(Scaler_InputSrcFormat(src_idx), timing, 0);
	if (pValue == NULL)
		return SLR_COLORTEMP_NORMAL;


	level = (SLR_COLORTEMP_LEVEL)(*pValue);
	/*VIPprintf("$$$$$$$$== fwif_color_get_color_temp_level = %d =====\n", level);*/
	return level;
}

unsigned char scaler_get_color_temp_level_type(unsigned char src_idx)
{
	return scaler_get_color_temp_level(src_idx, 0);
}

unsigned char Scaler_GetIntensity(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Intensity;
}

/*range 0~100*/
void Scaler_SetIntensity(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Intensity = value;
}

unsigned char Scaler_OsdMapToRegValue_Saturation(unsigned char display , unsigned char value)
{
	#if  defined(TV020_STYLE_PQ) || defined(BUILD_TV030_2)
	return  fw_OsdMapToRegValue(display, FAC_SATURATION, value);
	#else
	return 0;
	#endif
}
unsigned char Scaler_OsdMapToRegValue_Hue(unsigned char display , unsigned char value)
{
	#if  defined(TV020_STYLE_PQ) || defined(BUILD_TV030_2)
	return  fw_OsdMapToRegValue(display, FAC_HUE, value);
	#else
	return 0;
	#endif
}


#ifdef OSD_FOUR_LEVEL_MAPPING_ENABLE
unsigned char Scaler_GetFacSaturation(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75)
{
	StructColorDataFacModeType *pTable = NULL;
    unsigned char src_idx = 0;
    unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Saturation_0;
	*retval_50 = pTable->Saturation_50;
	*retval_100 = pTable->Saturation_100;
	*retval_25 = pTable->Saturation_25;
	*retval_75 = pTable->Saturation_75;
	return 0;
}
unsigned char Scaler_SetFacSaturation(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Saturation_0 = val_0;
	pTable->Saturation_50 = val_50;
	pTable->Saturation_100 = val_100;
	pTable->Saturation_25 = val_25;
	pTable->Saturation_75 = val_75;
	return 0;
}

unsigned char Scaler_GetFacHue(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75)
{
	StructColorDataFacModeType *pTable = NULL;
    unsigned char src_idx = 0;
    unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Hue_0;
	*retval_50 = pTable->Hue_50;
	*retval_100 = pTable->Hue_100;
	*retval_25 = pTable->Hue_25;
	*retval_75 = pTable->Hue_75;
	return 0;
}
unsigned char Scaler_SetFacHue(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Hue_0 = val_0;
	pTable->Hue_50 = val_50;
	pTable->Hue_100 = val_100;
	pTable->Hue_25 = val_25;
	pTable->Hue_75 = val_75;
	return 0;
}

#else

unsigned char Scaler_GetFacSaturation(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Saturation_0;
	*retval_50 = pTable->Saturation_50;
	*retval_100 = pTable->Saturation_100;
	return 0;
}

unsigned char Scaler_SetFacSaturation(unsigned char val_0, unsigned char val_50, unsigned char val_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Saturation_0 = val_0;
	pTable->Saturation_50 = val_50;
	pTable->Saturation_100 = val_100;
	return 0;
}

unsigned char Scaler_GetFacHue(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Hue_0;
	*retval_50 = pTable->Hue_50;
	*retval_100 = pTable->Hue_100;
	return 0;
}

unsigned char Scaler_SetFacHue(unsigned char val_0, unsigned char val_50, unsigned char val_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Hue_0 = val_0;
	pTable->Hue_50 = val_50;
	pTable->Hue_100 = val_100;
	return 0;
}

#endif

/*======================= set hue /sat  ============*/
extern unsigned char get_ADC_Input_Source(void);
extern unsigned char get_AVD_Input_Source(void);
void fwif_color_vsc_to_scaler_src_mapping(unsigned char vsc_src, unsigned char *p_scaler_src_type, unsigned char *p_scaler_src_form)
{
	if (p_scaler_src_type == NULL || p_scaler_src_form == NULL)
		return;

	switch(vsc_src)
	{
		case VSC_INPUTSRC_AVD:
			*p_scaler_src_type = get_AVD_Input_Source();
			*p_scaler_src_form = _SRC_FROM_VDC;
			break;
		case VSC_INPUTSRC_ADC:
			*p_scaler_src_type = get_ADC_Input_Source();
			*p_scaler_src_form = _SRC_FROM_ADC;
			break;
		case VSC_INPUTSRC_HDMI:
			*p_scaler_src_type = _SRC_HDMI;
			*p_scaler_src_form = _SRC_FROM_TMDS;
			break;
		case VSC_INPUTSRC_VDEC:
			*p_scaler_src_type = _SRC_VO;
			*p_scaler_src_form = _SRC_FROM_VO;
			break;
		case VSC_INPUTSRC_JPEG:
			*p_scaler_src_type = _SRC_VO;
			*p_scaler_src_form = _SRC_FROM_VO;
			break;
		default:
			;
	}
}

unsigned char fwif_color_get_cur_input_timing_mode(unsigned char display)
{
#ifdef CONFIG_FORCE_RUN_I3DDMA
		if (display == SLR_MAIN_DISPLAY)
			return drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
		else
			return Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
#else
		return Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
#endif
}

unsigned char fwif_vip_HDR10_check(unsigned char display, unsigned char input_src_type)
{
	//unsigned char bHDR_flag =false;
	//SLR_VOINFO* pVOInfo = NULL;
	_system_setting_info *VIP_system_info_structure_table=NULL;

	//pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	 //  VUI Handler
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(input_src_type == _SRC_VO) {
		fwif_color_VIP_get_VO_Info(input_src_type);
	} else if (input_src_type == _SRC_HDMI) {
		fwif_color_VIP_get_DRM_Info(input_src_type);
	} else {
		VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 0;
		VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Max;
		VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode = HDR_MODE_Max;
	}
	return 0;
}
#if 0
unsigned char fwif_vip_DolbyHDR_check(unsigned char display, unsigned char input_src_type)
{
#ifdef VPQ_COMPILER_ERROR_ENABLE
	unsigned char bDolby_HDR_flag = false;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(input_src_type == _SRC_VO || input_src_type == _SRC_HDMI) // usb & hdmi
	{
		#if defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)
		if((IoReg_Read32(DOLBY_V_TOP_TOP_CTL_reg) & 7) == 2 || Scaler_HDR_DolbyVision_Hdmi_Get_Detect_Result())
		#else
		if((IoReg_Read32(DOLBY_V_TOP_TOP_CTL_reg) & 7) == 2)
		#endif
			bDolby_HDR_flag = 1;
	}

	if (VIP_system_info_structure_table != NULL)
		VIP_system_info_structure_table ->DolbyHDR_flag = bDolby_HDR_flag;

	return bDolby_HDR_flag;
#endif
return 0; //remove this line , after remove #ifdef VPQ_COMPILER_ERROR_ENABLE

}
#endif

unsigned char fwif_vip_source_check(unsigned char display, unsigned char callmode)
{
	unsigned char which_source = 255;
	unsigned char input_src_type;
	unsigned char input_src_form;
	unsigned char input_timming_info;
	unsigned char input_VO_Form;
	unsigned char input_TVD_Form;
	/*unsigned char NowSourceOption;*/
	unsigned char video_format = COLOR_UNKNOW;
	unsigned char hdmiDataRange = MODE_RAG_UNKNOW;
#ifdef CONFIG_Force_HDMI_RGB_INTO_VIP_Mode
	unsigned char ForceHDMIIntoVIPMode = 0;
#endif
	unsigned char Frame_Packing_mode = 0;
	unsigned char vdc_ColorStandard = VDC_MODE_NO_SIGNAL;
	SLR_RATIO_TYPE ratio_type = SLR_RATIO_DISABLE;
	SLR_VOINFO* pVOInfo = NULL;
	unsigned char bHDR_flag = false;
	unsigned char bDolby_HDR_flag = false;
	unsigned int frameRates = 0;
	unsigned int dvs_xtal = 0;
    unsigned int I_height=0,I_width=0;

	_system_setting_info *VIP_system_info_structure_table=NULL;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;

	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if((VIP_RPC_system_info_structure_table == NULL) || (VIP_system_info_structure_table == NULL)){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return 0;
	}
	dvs_xtal = PPOVERLAY_DVS_cnt_get_dvs_cnt(IoReg_Read32(PPOVERLAY_DVS_cnt_reg));
	frameRates = 27000000UL / dvs_xtal;
    if(frameRates>=81 && frameRates <=120)
        frameRates=120;
    
#ifdef CONFIG_Force_HDMI_RGB_INTO_VIP_Mode
	ForceHDMIIntoVIPMode = 1;
#endif

	if (fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_MAIN_DISPLAY), &input_src_type, &input_src_form);
		input_timming_info 	= drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
	} else {
		if (callmode == BY_DISPLAY) {
			input_src_type = Scaler_InputSrcGetType((SCALER_DISP_CHANNEL)display);
			input_src_form = Scaler_InputSrcGetFrom((SCALER_DISP_CHANNEL)display);
		} else {
			input_src_type = Scaler_InputSrcGetMainChType();
			input_src_form = Scaler_InputSrcGetMainChFrom();
		}

		input_timming_info 	= Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	}

	input_VO_Form		= Scaler_VOFrom(Scaler_Get_CurVoInfo_plane());
	input_TVD_Form		= Scaler_InputSrcGetMainFormat();
	ratio_type		= Scaler_DispGetRatioMode();/*for TV035*/
	/*NowSourceOption		= Get_NowSourceOption();	*/ /*fix me Mac3 no this function*/
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());

	bHDR_flag = VIP_system_info_structure_table->HDR_flag;
	bDolby_HDR_flag = VIP_system_info_structure_table->DolbyHDR_flag;

	#if 0
	VIPprintf( "++++++++++++fwif_vip_source_check debug Part.A+++++++++++++++++++\n");
	VIPprintf( "[source_check]input_src_from = %d \n", input_src_form);
	VIPprintf( "[source_check]input_src_type = %d \n", input_src_type);
	VIPprintf( "[source_check]Scaler_VOFrom = %d \n", Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()));
	VIPprintf( "[source_check]drvif_scalerVideo_readMode = %d \n", drvif_module_vdc_ReadMode(VDC_SET));
	VIPprintf( "[source_check]Scaler_DispGetInputInfo = %d \n", Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR));
	VIPprintf( "[source_check]Scaler_InputSrcFormat = %d \n", Scaler_InputSrcFormat(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_DATA_ARRAY_IDX)));
	VIPprintf( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	#endif

    I_height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	I_width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);

	if( input_src_form == _SRC_FROM_VDC )
	{
		if( vdc_power_status )
			vdc_ColorStandard = drvif_module_vdc_ReadMode(VDC_SET);

		if( input_src_type == _SRC_TV )
		{

			switch( vdc_ColorStandard )
			{
				case VDC_MODE_NTSC: /*480I*/
					which_source = VIP_QUALITY_TV_NTSC;
					break;
				case VDC_MODE_PALM: /*480I*/
					which_source = VIP_QUALITY_TV_PAL_M;
					break;
				case VDC_MODE_NTSC443: /*480I*/
					which_source = VIP_QUALITY_TV_NTSC443;
					break;
				case VDC_MODE_PALI: /*576I*/
					which_source = VIP_QUALITY_TV_PAL;
					break;
				case VDC_MODE_SECAM: /*576I*/
					which_source = VIP_QUALITY_TV_SECAN;
					break;
				case VDC_MODE_PAL60: /*480I*/
					which_source = VIP_QUALITY_TV_PAL60;
					break;
				case VDC_MODE_PALN: /*576I*/
					which_source = VIP_QUALITY_TV_PALN;
					break;
				case VDC_MODE_NTSC50: /**/
					which_source = VIP_QUALITY_TV_NTSC50;
					break;
				default:
					if( input_timming_info == _MODE_480I )
						which_source = VIP_QUALITY_TV_NTSC;
					else
						which_source = VIP_QUALITY_TV_PAL;
					break;
			}

		}
		else if( input_src_type == _SRC_CVBS )
		{

			switch( vdc_ColorStandard )
			{
				case VDC_MODE_NTSC: /*480I*/
					which_source = VIP_QUALITY_CVBS_NTSC;
					break;
				case VDC_MODE_PALM: /*480I*/
					which_source = VIP_QUALITY_CVBS_PAL_M;
					break;
				case VDC_MODE_PALI: /*576I*/
					which_source = VIP_QUALITY_CVBS_PAL;
					break;
				case VDC_MODE_SECAM: /*576I*/
					which_source = VIP_QUALITY_CVBS_SECAN;
					break;
				case VDC_MODE_NTSC443: /*480I*/
					which_source = VIP_QUALITY_CVBS_NTSC443;
					break;
				case VDC_MODE_PAL60: /*480I*/
					which_source = VIP_QUALITY_CVBS_PAL60;
					break;
				case VDC_MODE_PALN: /*576I*/
					which_source = VIP_QUALITY_CVBS_PALN;
					break;
				case VDC_MODE_NTSC50: /**/
					which_source = VIP_QUALITY_CVBS_NTSC50;
					break;
				default:
					if( input_timming_info == _MODE_480I )
						which_source = VIP_QUALITY_CVBS_NTSC;
					else
						which_source = VIP_QUALITY_CVBS_PAL;
					break;
			}

		}
		else if( (input_src_type == _SRC_SV) || (input_src_type == _SRC_COMPONENT) )
		{

			switch( vdc_ColorStandard )
			{
				case VDC_MODE_NTSC: /*480I*/
					which_source = VIP_QUALITY_CVBS_S_NTSC;
					break;
				case VDC_MODE_PALM: /*480I*/
					which_source = VIP_QUALITY_CVBS_S_PAL_M;
					break;
				case VDC_MODE_PALI: /*576I*/
					which_source = VIP_QUALITY_CVBS_S_PAL;
					break;
				case VDC_MODE_SECAM: /*576I*/
					which_source = VIP_QUALITY_CVBS_S_SECAN;
					break;
				case VDC_MODE_NTSC443: /*480I*/
				case VDC_MODE_PAL60: /*480I*/
				case VDC_MODE_PALN: /*576I*/
				case VDC_MODE_NTSC50: /**/
				default:
					if( input_timming_info == _MODE_480I )
						which_source = VIP_QUALITY_CVBS_S_NTSC;
					else
						which_source = VIP_QUALITY_CVBS_S_PAL;
					break;
			}

		}
#ifdef CONFIG_SCART_ENABLE
		else if( (input_src_type == _SRC_SCART) || (input_src_type == _SRC_SCART_RGB) )
		{
			// RGB
			if( (drvif_module_vdc_read_FSW_value()>=SCART_RGB_CVBS_THRESHOLD) || (input_src_type == _SRC_SCART_RGB) )
			{
				switch( Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR) )
				{
					case _MODE_480I: // NTSC
						if( vdc_ColorStandard == VDC_MODE_PALM )
							which_source = VIP_QUALITY_SCART_RGB_PAL;
						else
							which_source = VIP_QUALITY_SCART_RGB_NTSC;
						break;
					case _MODE_576I: // PAL
						if( vdc_ColorStandard == VDC_MODE_SECAM )
							which_source = VIP_QUALITY_SCART_RGB_SECAN;
						else
							which_source = VIP_QUALITY_SCART_RGB_PAL;
						break;
					default: // PAL
						which_source = VIP_QUALITY_SCART_RGB_PAL;
						break;
				}

			}
			else
			{
				switch (Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR))
				{
					case _MODE_480I: // NTSC
						if(vdc_ColorStandard == VDC_MODE_PALM)
							which_source = VIP_QUALITY_SCART_PAL;
						else
							which_source = VIP_QUALITY_SCART_NTSC;
						break;
					case _MODE_576I: // PAL
						if(vdc_ColorStandard == VDC_MODE_SECAM)
							which_source = VIP_QUALITY_SCART_SECAN;
						else
							which_source = VIP_QUALITY_SCART_PAL;
						break;
					default: // PAL
						which_source = VIP_QUALITY_SCART_PAL;
						break;
				}
			}
		}
#endif
	}
	else if (input_src_type == _SRC_VGA) {
		which_source = VIP_QUALITY_VGA;
	} else if ((input_src_type == _SRC_VO) && (input_VO_Form != VO_FROM_JPEG)) {	/* 0) DTV or  2)GAME*/
		/*if (NowSourceOption == SOURCE_PLAYBACK)			*/ /* USB source  */ /*fix me Mac3 no this function*/
		if (input_TVD_Form == TVD_INFMT_VODMA1)	{		/* USB source*/

			/*USER: Vincent_Lee  DATE: 2012/06/08  TODO: is JPEG?*/
			if (Scaler_VOFromJPEG(Scaler_Get_CurVoInfo_plane()) == 1) {
				which_source = VIP_QUALITY_DTV_JPEG;
			} else {
			switch (input_timming_info) {
			case _MODE_480I:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_480I;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_PVR_480I;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_480I;
				else
					which_source = VIP_QUALITY_PVR_480I;
				break;
			case _MODE_576I:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_576I;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_PVR_576I;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_576I;
				else
					which_source = VIP_QUALITY_PVR_576I;
				break;
			case  _MODE_480P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_480P;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_PVR_480P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_480P;
				else
					which_source = VIP_QUALITY_PVR_480P;
				break;
			case _MODE_576P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_576P;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_PVR_576P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_576P;
				else
					which_source = VIP_QUALITY_PVR_576P;
				break;
			case _MODE_720P50:
			case _MODE_720P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_720P;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_PVR_720P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_720P;
				else
					which_source = VIP_QUALITY_PVR_720P;
				break;
			case _MODE_1080I25:
			case _MODE_1080I30:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_1080I;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_PVR_1080I;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_1080I;
				else
					which_source = VIP_QUALITY_PVR_1080I;
				break;
			case _MODE_1080P50:
			case _MODE_1080P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_1080p;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_PVR_1080p;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_1080P;
				else
					which_source = VIP_QUALITY_PVR_1080P;
				break;
			case _MODE_4k2kI30:
				which_source = VIP_QUALITY_PVR_4k2kI_30;
				break;
			case _MODE_4k2kP30:
			case _MODE_4k2kP24:
			case _MODE_4k2kP25:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_4k2kP_30;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_PVR_4k2kP_30;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_4k2kP_30;
				else if(frameRates==120)
                    which_source = VIP_QUALITY_PVR_4k2k_120;
                else if(I_height>=4320 && I_width>=7680)
                     which_source = VIP_QUALITY_PVR_8k4k_30;
                else
					which_source = VIP_QUALITY_PVR_4k2kP_30;
				break;
			case _MODE_4k2kP48:
			case _MODE_4k2kP50:
			case _MODE_4k2kP60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_4k2kP_60;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_PVR_4k2kP_60;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_4k2kP_60;
				else if(frameRates==120)
                    which_source = VIP_QUALITY_PVR_4k2k_120;
                else if(I_height>=4320 && I_width>=7680)
                     which_source = VIP_QUALITY_PVR_8k4k_60; 
                else
					which_source = VIP_QUALITY_PVR_4k2kP_60;
				break;
              
            default:
				which_source = VIP_QUALITY_PVR_480I;
			break;
			}
		}
	} else {															/* DTV source*/

			switch (input_timming_info) {
			case _MODE_480I:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_480I;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_DTV_480I;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_480I;
				else
					which_source = VIP_QUALITY_DTV_480I;
				break;
			case _MODE_576I:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_576I;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_DTV_576I;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_576I;
				else
					which_source = VIP_QUALITY_DTV_576I;
				break;
			case  _MODE_480P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_480P;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_DTV_480P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_480P;
				else
					which_source = VIP_QUALITY_DTV_480P;
				break;
			case _MODE_576P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_576P;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_DTV_576P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_576P;
				else
					which_source = VIP_QUALITY_DTV_576P;
				break;
			case _MODE_720P50:
			case _MODE_720P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_720P;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_DTV_720P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_720P;
				else
					which_source = VIP_QUALITY_DTV_720P;
				break;
			case _MODE_1080I25:
			case _MODE_1080I30:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_1080I;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_DTV_1080I;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_1080I;
				else
					which_source = VIP_QUALITY_DTV_1080I;
				break;
			case _MODE_1080P50:
			case _MODE_1080P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_1080p;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_DTV_1080p;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_1080P;
				else
				which_source = VIP_QUALITY_DTV_1080P;
				break;
			case _MODE_4k2kI30:
				which_source = VIP_QUALITY_DTV_4k2kI_30;
				break;
			case _MODE_4k2kP30:
			case _MODE_4k2kP24:
			case _MODE_4k2kP25:
                    if(I_height>=4320 && I_width>=7680)
                        which_source = VIP_QUALITY_DTV_8k4k_30;
                    else if (Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_THRIP) || Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_RTNR))
                        {
                                if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
                                        which_source = VIP_QUALITY_HDR_DTV_4k2kP_30;
                                else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
                                        which_source = VIP_QUALITY_ST2094_DTV_4k2kP_30;
                                else if(bDolby_HDR_flag)
                                        which_source = VIP_QUALITY_DolbyHDR_DTV_4k2kP_30;
                                else
                                        which_source = VIP_QUALITY_DTV_4k2kP_30;

                        }
                        else //rtnr off  , dut to 4k60 rtnr must be off ,
                        {
                                if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
                                        which_source = VIP_QUALITY_HDR_DTV_4k2kP_60;
                                else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
                                        which_source = VIP_QUALITY_ST2094_DTV_4k2kP_60;
                                else if(bDolby_HDR_flag)
                                        which_source = VIP_QUALITY_DolbyHDR_DTV_4k2kP_60;
                                else
                                        which_source = VIP_QUALITY_DTV_4k2kP_60;

                        }
				break;
			case _MODE_4k2kP48:
			case _MODE_4k2kP50:
			case _MODE_4k2kP60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_4k2kP_60;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_DTV_4k2kP_60;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_4k2kP_60;
				else if(frameRates==120)
                    which_source = VIP_QUALITY_DTV_4k2k120;
                else if(I_height>=4320 && I_width>=7680)
                     which_source = VIP_QUALITY_DTV_8k4k_60; 
                else
					which_source = VIP_QUALITY_DTV_4k2kP_60;
				break;

             default:
				which_source = VIP_QUALITY_DTV_480I;
			break;
			}
		}
	} else if ((input_src_type == _SRC_VO) && (input_VO_Form == VO_FROM_JPEG)) {/* VO JPEG source	*/ /* Erin add '100920*/

			/*Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);  */ /* M domain?width*/
			/*Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN);  */ /* M domain?height*/

			which_source = VIP_QUALITY_DTV_JPEG;
	} else if (input_src_type == _SRC_YPBPR) {
		switch (input_timming_info) {
		case _MODE_480I:
			which_source = VIP_QUALITY_YPbPr_480I;
			break;
		case _MODE_576I:
			which_source = VIP_QUALITY_YPbPr_576I;
			break;
		case  _MODE_480P:
			which_source = VIP_QUALITY_YPbPr_480P;
			break;
		case _MODE_576P:
			which_source = VIP_QUALITY_YPbPr_576P;
			break;
		case _MODE_720P50:
			which_source = VIP_QUALITY_YPbPr_720P;
			break;
		case _MODE_720P60:
			which_source = VIP_QUALITY_YPbPr_720P;
			break;
		case _MODE_1080I25:
			which_source = VIP_QUALITY_YPbPr_1080I;
			break;
		case _MODE_1080I30:
			which_source = VIP_QUALITY_YPbPr_1080I;
			break;
		case _MODE_1080P50:
			which_source = VIP_QUALITY_YPbPr_1080P;
			break;
		case _MODE_1080P60:
			which_source = VIP_QUALITY_YPbPr_1080P;
			break;
		default:
          if(frameRates==120)
            which_source = VIP_QUALITY_YPbPr_2k120;
          else
			which_source = VIP_QUALITY_YPbPr_720P;
		break;
		}
	} else {
		video_format = drvif_Hdmi_GetColorSpace();
		hdmiDataRange =  drvif_IsRGB_YUV_RANGE();

		#if 0
		VIPprintf("++++++++++++fwif_vip_source_check debug Part.B+++++++++++++++++++\n");
		VIPprintf("[source_check] drvif_IsHDMI()= %d			     $$$$$$$$\n", drvif_IsHDMI());
		VIPprintf("[source_check] drvif_Hdmi_HdcpEnabled()= %d   $$$$$$$$\n", drvif_Hdmi_HdcpEnabled());
		VIPprintf("[source_check] video_format = %d				  $$$$$$$$\n", video_format);
		VIPprintf("[source_check] ForceHDMIIntoVIPMode()= %d	     $$$$$$$$\n", ForceHDMIIntoVIPMode);
		VIPprintf("[source_check] drvif_Hdmi_IsAudioLock() = %d    $$$$$$$$\n", drvif_Hdmi_IsAudioLock());
		VIPprintf("[source_check] Scaler_DispGetInputInfo() = %d    $$$$$$$$\n", Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR));
		VIPprintf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
		#endif


		/*if (drvif_IsHDMI() && ((drvif_Hdmi_HdcpEnabled()) || (video_format != COLOR_RGB) || (ForceHDMIIntoVIPMode) || (drvif_Hdmi_IsAudioLock()))) {*/ /* HDMI*/
		/* isHDMI + video timing =>  !VGA, accroding to ref set.*/
		if (drvif_IsHDMI()) { /* HDMI*/
			/* checkk if HDMI mode, not DVI mode, modify this by Jerry Wu 20100120,*/
			/* add HDCP check 20100224*/
			/* add YUV mode into this mode 20100903 Jerry Wu*/
			switch (input_timming_info)
			{
			case _MODE_480I:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_480I;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_HDMI_480I;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_480I;
				else
					which_source = VIP_QUALITY_HDMI_480I;
				break;
			case _MODE_576I:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_576I;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_HDMI_576I;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_576I;
				else
					which_source = VIP_QUALITY_HDMI_576I;
				break;
			case  _MODE_480P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_480P;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_HDMI_480P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_480P;
				else
					which_source = VIP_QUALITY_HDMI_480P;
				break;
			case _MODE_576P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_576P;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_HDMI_576P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_576P;
				else
					which_source = VIP_QUALITY_HDMI_576P;
				break;
			case _MODE_720P50:
			case _MODE_720P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_720P;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_HDMI_720P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_720P;
				else
					which_source = VIP_QUALITY_HDMI_720P;
				break;
			case _MODE_1080I25:
			case _MODE_1080I30:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_1080I;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_HDMI_1080I;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_1080I;
				else
					which_source = VIP_QUALITY_HDMI_1080I;
				break;
			case _MODE_1080P50:
			case _MODE_1080P60:
	/*==USER: Vincent_Lee  DATE: 2011/08/12  TODO: Add new CASE for 1080P24 mode==*/
			case _MODE_1080P23:
			case _MODE_1080P24:
			case _MODE_1080P25:
			case _MODE_1080P29:
			case _MODE_1080P30:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_1080p;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_HDMI_1080p;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_1080P;
				else
					which_source = VIP_QUALITY_HDMI_1080P;
				break;
	/*==USER: Vincent_Lee  DATE: 2011/08/12  TODO: Add new CASE for 1080P24 mode==*/
				/*which_source = VIP_QUALITY_HDMI_1080P;*/
				break;
			case _MODE_4k2kI30:
				which_source = VIP_QUALITY_HDMI_4k2kI_30;
				break;
			case _MODE_4k2kP30:
			case _MODE_4k2kP24:
			case _MODE_4k2kP25:
					if(I_height>=4320 && I_width>=7680)
                        which_source = VIP_QUALITY_HDMI_8k4k_30;
                    else if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
						which_source = VIP_QUALITY_HDR_HDMI_4k2kP_30;
					else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
						which_source = VIP_QUALITY_ST2094_HDMI_4k2kP_30;
					else if(bDolby_HDR_flag)
						which_source = VIP_QUALITY_DolbyHDR_HDMI_4k2kP_30;
					else if(frameRates==120)
                        which_source = VIP_QUALITY_HDMI_4k2k_120;
                    else
						which_source = VIP_QUALITY_HDMI_4k2kP_30;
					break;
			case _MODE_4k2kP48:
			case _MODE_4k2kP50:
			case _MODE_4k2kP60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_4k2kP_60;
				else if (bHDR_flag == HAL_VPQ_HDR_MODE_ST2094)
					which_source = VIP_QUALITY_ST2094_HDMI_4k2kP_60;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_4k2kP_60;
				else if(frameRates==120)
                        which_source = VIP_QUALITY_HDMI_4k2k_120;
                else if(I_height>=4320 && I_width>=7680)
                     which_source = VIP_QUALITY_HDMI_8k4k_60; 
                else
					which_source = VIP_QUALITY_HDMI_4k2kP_60;
				break;

			/*USER: egger  DATE: 2011/09/21  TODO: Add new CASE for frame packing mode*/
			case _MODE_NEW:
				Frame_Packing_mode = Scaler_Get3D_IsFramePacking();
				if (Frame_Packing_mode == 1)
					which_source = VIP_QUALITY_HDMI_720P;
				else if (Frame_Packing_mode == 2)
					which_source = VIP_QUALITY_HDMI_1080P;
				else
				    which_source = VIP_QUALITY_VGA; /* VGA Timing*/
				break;
			default:               
                which_source = VIP_QUALITY_VGA; /* VGA Timing*/
			break;
			}
		} else { /* DVI*/
			which_source = VIP_QUALITY_VGA;
		}

	}

	/*VIPprintf("\n--- __lhh__  -2  Source = %d\n", which_source);*/

	/*=== 20140421 CSFC for vip video fw infomation ====*/
	/* setting some information to sharememory*/
	VIP_system_info_structure_table->Input_src_Type		= input_src_type;
	VIP_system_info_structure_table->Input_src_Form		= input_src_form;
	VIP_system_info_structure_table->Input_VO_Form		= input_VO_Form;
	VIP_system_info_structure_table->Input_TVD_Form		= input_TVD_Form;
	VIP_system_info_structure_table->vdc_color_standard	= vdc_ColorStandard;
	VIP_system_info_structure_table->Timing			= input_timming_info;
	VIP_system_info_structure_table->HDMI_video_format	= video_format;
	VIP_RPC_system_info_structure_table->VIP_source 	= which_source;
	VIP_system_info_structure_table->Display_RATIO_TYPE	= ratio_type;
	/*VIP_system_info_structure_table->NowSourceOption	= NowSourceOption; */ /*fix me Mac3 no this function*/
	VIP_system_info_structure_table->HDMI_data_range    = hdmiDataRange;
	//VIP_system_info_structure_table ->HDR_flag = bHDR_flag;					/* decide HDR flag in Scaler_color_HDRIP_AutoRun() */
	//VIP_system_info_structure_table ->DolbyHDR_flag = bDolby_HDR_flag;		/* decide HDR flag in Scaler_color_HDRIP_AutoRun() */
	VIP_system_info_structure_table ->VO_Width = pVOInfo->h_width;
	VIP_system_info_structure_table ->VO_Height = pVOInfo->v_length;

	VIP_system_info_structure_table->input_display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	VIP_system_info_structure_table->HDMI_color_space = Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE);
	VIP_system_info_structure_table->IV_Start = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA);
	VIP_system_info_structure_table->IH_Start = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA);
	VIP_system_info_structure_table->I_Height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	VIP_system_info_structure_table->I_Width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
	VIP_system_info_structure_table->Mem_Height = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN);
	VIP_system_info_structure_table->Mem_Width = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);
	VIP_system_info_structure_table->Cap_Height = Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN);
	VIP_system_info_structure_table->Cap_Width = Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
	VIP_system_info_structure_table->DI_Width = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID);
	VIP_system_info_structure_table->D_Height = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
	VIP_system_info_structure_table->D_Width = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
	VIP_system_info_structure_table->color_fac_src_idx = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY, SLR_INPUT_DATA_ARRAY_IDX);
	/*============================================*/
	return which_source;
}

/*young 3Dtable*/


unsigned char fwif_vip_3Dsource_check(unsigned char display, unsigned char callmode)
{
	unsigned char which_source = 255;
	unsigned char input_src_type;
	unsigned char input_src_from;
	unsigned char video_format;
	unsigned char ForceHDMIIntoVIPMode = 0;
	/*unsigned char Frame_Packing_mode = 0;*/
	bool boolHd = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY), SLR_INPUT_MEM_ACT_WID) > 640 ? 1 : 0;


#ifdef CONFIG_Force_HDMI_RGB_INTO_VIP_Mode
	ForceHDMIIntoVIPMode = 1;
#endif

	if (callmode == BY_DISPLAY) {
		input_src_type = Scaler_InputSrcGetType((SCALER_DISP_CHANNEL)display);
		input_src_from = Scaler_InputSrcGetFrom((SCALER_DISP_CHANNEL)display);
	} else {
		input_src_type = Scaler_InputSrcGetMainChType();
		input_src_from = Scaler_InputSrcGetMainChFrom();
	}



	if (input_src_from == _SRC_FROM_VDC) {

	} else if (input_src_type == _SRC_VGA) {

	} else if ((input_src_type == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) != 1)) {/* DTV source*/
		if (Scaler_InputSrcGetMainFormat() == TVD_INFMT_VODMA1) {/*USB*/
			/*switch (GET_HDMI_3DTYPE())*/
			switch (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)) {
			case SLR_3D_FRAME_PACKING:
				which_source = (boolHd ? VIP_QUALITY_USB_HD_FP : VIP_QUALITY_USB_SD_FP);
				break;
			case SLR_3D_SIDE_BY_SIDE_HALF:
				which_source = VIP_QUALITY_USB_SBS;
				break;
			case SLR_3D_TOP_AND_BOTTOM:
				which_source = VIP_QUALITY_USB_TOP_BOTTOM;
				break;
			}
		} else {
			switch (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)) {/*DTV*/
			case SLR_3D_FRAME_PACKING:
				which_source = (boolHd ? VIP_QUALITY_DTV_HD_FP : VIP_QUALITY_DTV_SD_FP);
				break;
			case SLR_3D_SIDE_BY_SIDE_HALF:
				which_source = VIP_QUALITY_DTV_SBS;
				break;
			case SLR_3D_TOP_AND_BOTTOM:
				which_source = VIP_QUALITY_DTV_TOP_BOTTOM;
				break;
			}
		}

	} else if ((input_src_from != _SRC_FROM_VDC) && (input_src_type != _SRC_VGA) && (input_src_type == _SRC_VO) && (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 1)) {/* VO JPEG source	*/ /* Erin add '100920*/

	} else if (input_src_type == _SRC_YPBPR) {

	} else {
		video_format = drvif_Hdmi_GetColorSpace();
		if (drvif_IsHDMI() && ((drvif_Hdmi_HdcpEnabled()) || (video_format != 0) || (ForceHDMIIntoVIPMode) || (drvif_Hdmi_IsAudioLock()))) {
			/* HDMI*/
			/* checkk if HDMI mode, not DVI mode, modify this by Jerry Wu 20100120,*/
			/* add HDCP check 20100224*/
			/* add YUV mode into this mode 20100903 Jerry Wu*/

			switch (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE)) {
			case SLR_3D_FRAME_PACKING:
				which_source = (boolHd ? VIP_QUALITY_HDMI_HD_FP : VIP_QUALITY_HDMI_SD_FP);
				break;
			case SLR_3D_SIDE_BY_SIDE_HALF:
				which_source = VIP_QUALITY_HDMI_SBS;
				break;
			case SLR_3D_TOP_AND_BOTTOM:
				which_source = VIP_QUALITY_HDMI_TOP_BOTTOM;
				break;
			}
		} else {

		}

	}

	if (which_source == 255) {
		which_source = 0;
	}

	return which_source;
}

/*============================================================================*/
/**
 * fwif_color_video_quality_handler
 * This function is used to set Video Quality function.
 *
 * @param <void>		{ void }
 * @return 			{ void }
 *
 */
void fwif_color_temptable_handler(unsigned char which_source, unsigned char *temptable)
{
	/*move to uppder stage for speed up.josh@20090326*/
	/*which_source = fwif_vip_source_check(3, NOT_BY_DISPLAY);*/
	if ((which_source == VIP_QUALITY_TV_PAL) || (which_source == VIP_QUALITY_CVBS_PAL)) {
		drvif_color_rtnr_grid_remover_type(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), temptable[0]);/*grid remover  090329 March*/
		fwif_color_set_grid_remove(Scaler_DispGetInputInfo(SLR_INPUT_DATA_ARRAY_IDX), temptable[0]);
	} else {
		drvif_color_rtnr_grid_remover_type(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), 0);
		fwif_color_set_grid_remove(Scaler_DispGetInputInfo(SLR_INPUT_DATA_ARRAY_IDX), 0);
	}

//	drvif_color_StillLineCntns(Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL), temptable[1]); /*conti.Still line*/
//	fwif_color_set_conti_still_line(Scaler_DispGetInputInfo(SLR_INPUT_DATA_ARRAY_IDX), temptable[1]);
}

void scaler_set_linear_DM_Table(void)
{
#if 0
	static PQ_HDR_3dLUT_16_PARAM args;

	args.c3dLUT_16_enable = 1;
	args.a3dLUT_16_pArray = (UINT16*)&HDR10_3DLUT_24x24x24[0];
	drvif_Set_DM_HDR_CLK();	// enable DM & Composer clock
	fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 1);

	fwif_color_set_DM2_EOTF_TV006(EOTF_LUT_Linear, 1);

	fwif_color_set_DM2_OETF_TV006(OETF_LUT_Linear, 1);
#endif
}

#ifdef RUN_ON_TVBOX
void fwif_color_video_quality_handler(void)/*TVBox Process*/
{
	unsigned char source = 255, input_src_type;
	unsigned char input_display = SLR_MAIN_DISPLAY;

	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	input_display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);/* get display info*/
	input_src_type = Scaler_InputSrcGetType((SCALER_DISP_CHANNEL)input_display);

	unsigned char *pVipCoefArray = NULL;

	if (m_cbFunc_getQualityCoef != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityCoef((VIP_SOURCE_TIMING)source);

	if (!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		Scaler_SetDLti(pVipCoefArray[VIP_QUALITY_FUNCTION_DLTi]);

		Scaler_SetDCti(pVipCoefArray[VIP_QUALITY_FUNCTION_DCTi]);

		Scaler_SetIDLti(pVipCoefArray[VIP_QUALITY_FUNCTION_I_DLTi]);

		Scaler_SetIDCti(pVipCoefArray[VIP_QUALITY_FUNCTION_I_DCTi]);

		Scaler_SetColor_UV_Delay(pVipCoefArray[VIP_QUALITY_FUNCTION_UV_DELAY]);

//		Scaler_SetMA_Chroma_Error(pVipCoefArray[VIP_QUALITY_FUNCTION_MA_Chroma_Error]); // merlin4 rmeove
	}

	if (source == VIP_QUALITY_CVBS_PAL) {
		Scaler_SetColor_UV_Delay_Enable(1);
		Scaler_SetColor_UV_Delay(33);	/*U_delay =-2, V_delay =-1*/
	}

	if (m_cbFunc_getQualityExtend2 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend2((VIP_SOURCE_TIMING)source);

	if (!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		Scaler_SetScaleUPH(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_H]);

		Scaler_SetScaleUPV(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_V]);

		Scaler_SetScaleUPH_8tap(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_H_8TAP]);

		Scaler_SetScaleUPV_6tap(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_V_6TAP]);

		Scaler_SetScaleUPDir(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_DIR]);

		Scaler_SetScaleUPDir_Weighting(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_DIR_Weighting]);
	}

}

#else
void fwif_color_video_quality_handler(void)/*TV Process*/
{
	unsigned char ii;/*, jj;*/
	/*unsigned char which_value, pre_value;*/
	unsigned char source = 255;
	static unsigned char pre_source = 255;
	//unsigned char input_display = SLR_MAIN_DISPLAY;
	unsigned char *pVipCoefArray = NULL;
	SLR_VIP_TABLE *gVIP_Table;
	unsigned char *drvSkip_Array;
	unsigned char black_levle = 0;
	unsigned char mCon, mBri, mHue, mSat;
	unsigned char  input_info = 0;
    #ifdef SQM_OFFSET_TBL
    extern unsigned char updateSQM_flag;
    #endif
	/*int ret;*/
	/*unsigned char spatial_y = 0, spatial_c = 0, temporal = 0, rtnr_level_y = 0, rtnr_level_c = 0, s_dnr_level = 0;*/

	/*Elsie 20130509: for speeding up*/
	//unsigned short DataArrayIdxInfo;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	ppoverlay_main_display_control_rsv_RBUS ppoverlay_main_display_control_rsv;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

    rtd_printk(KERN_EMERG, TAG_NAME, "fwif_color_video_quality_handler\n");
	/*force background check*/
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	ppoverlay_main_display_control_rsv.regValue= IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	if (ppoverlay_main_display_control_rsv.m_force_bg == 0) /*normal mode*/
		VIPprintf("[Warnning] [%s-%s-%d] Start, Not Black Screen", __FILE__, __func__, __LINE__);

	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if((VIP_RPC_system_info_structure_table == NULL) || (VIP_system_info_structure_table == NULL)){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_HDR10_display_check] = 1;

	//input_display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);/* get display info*/
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	//DataArrayIdxInfo = Scaler_DispGetInputInfo(SLR_INPUT_DATA_ARRAY_IDX);/*get input src info by channel*/

	//20160218 roger ramrk, it make some init problem
	#if 0
	if( (VIP_system_info_structure_table->Input_src_Type) == _SRC_TV) {
		/*src check, if pre src == current src, return.*/
		if( source == m_nowSource ) {
			return;
		}
	}
	#endif
	m_nowSource = source;

        #if 0
                vpq_project_id = 0x00010000;
                fwif_color_set_pq_demo_flag_rpc(1);
                Scaler_SetDNR(4);
                Scaler_SetMPEGNR(4, CALLED_NOT_BY_OSD);
                fwif_color_set_PQA_motion_threshold_TV006();
                fwif_color_SetAutoMAFlag(1);
                Scaler_SetSharpness(50);
		//Scaler_Set3dLUT(1);
                //drvif_color_D_3dLUT_Enable(1);
        #endif

	Scaler_VIP_IP_init(source);/*some vip ip init*/
#ifndef CONFIG_MEMC_BYPASS
	//Scaler_VIP_MEMC_init(source);/*vip MEMC ip init*/
#endif

	if (fwif_VIP_get_Project_ID() != VIP_Project_ID_TV030)
		Scaler_VIP_Project_ID_init();/*for project ID init, for video fw customization used !!*/

	gVIP_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (NULL == gVIP_Table)
		return;

#ifdef ENABLE_VIP_TABLE_CHECKSUM
/* === checksum ========== */
	Scaler_checkVIPTable();
/*==========================*/
#endif

	drvSkip_Array = &(gVIP_Table->DrvSetting_Skip_Flag[0]);
/*o------ VIP table : VIP_QUALITY_Coef ------o*/
	/* Load pVipCoefArray*/
	if (m_cbFunc_getQualityCoef != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityCoef((VIP_SOURCE_TIMING)source);
	else
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityCoef read fail !!!\n");

    rtd_printk(KERN_EMERG, TAG_NAME, "source = %d\n",source);

	for (ii = 0; ii < VIP_QUALITY_Coef_MAX; ii++) {
		/*VIPprintf("pVipCoefArray[%d] = %d\n", ii, pVipCoefArray[ii]);*/
	}

	if (source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL) {
		VIPprintf("break out--Diane");
	} else {
		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
			Scaler_SetDLti(pVipCoefArray[VIP_QUALITY_FUNCTION_DLTi]);
		else{

                        #if 0   // control table by rtk
			Scaler_SetDLti(pVipCoefArray[VIP_QUALITY_FUNCTION_DLTi]);//fixed me
                        #else
                                fwif_color_set_dLti_tv006(pVipCoefArray[VIP_QUALITY_FUNCTION_DLTi], NULL,NULL);
                        #endif
		}
		Scaler_SetIDLti(pVipCoefArray[VIP_QUALITY_FUNCTION_I_DLTi]);

		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) {
			Scaler_SetDCti(pVipCoefArray[VIP_QUALITY_FUNCTION_DCTi]);
			Scaler_SetIDCti(pVipCoefArray[VIP_QUALITY_FUNCTION_I_DCTi]);
		} else {
		        #if 0   // control table by rtk
			Scaler_SetDCti(pVipCoefArray[VIP_QUALITY_FUNCTION_DCTi]); //fixed me
			Scaler_SetIDCti(pVipCoefArray[VIP_QUALITY_FUNCTION_I_DCTi]);//fixed me
                        #else
                                fwif_color_set_iDcti_dDcti_tv006(pVipCoefArray[VIP_QUALITY_FUNCTION_I_DCTi], pVipCoefArray[VIP_QUALITY_FUNCTION_DCTi], NULL);
                        #endif
		}

		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
			Scaler_SetVDCti(pVipCoefArray[VIP_QUALITY_FUNCTION_V_DCTi]);
		else
			fwif_color_set_Vdcti_tv006(pVipCoefArray[VIP_QUALITY_FUNCTION_V_DCTi], NULL);

		Scaler_SetColor_UV_Delay_Enable((VIP_UV_DELAY_ENABLE)pVipCoefArray[VIP_QUALITY_FUNCTION_UV_DELAY_ENABLE]);

		Scaler_SetColor_UV_Delay(pVipCoefArray[VIP_QUALITY_FUNCTION_UV_DELAY]);

		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
			Scaler_set_YUV2RGB(pVipCoefArray[VIP_QUALITY_FUNCTION_YUV2RGB], drvSkip_Array[DrvSetting_Skip_Flag_coefByY], drvSkip_Array[DrvSetting_Skip_Flag_uvOffset]);
		else
			fwif_color_set_YUV2RGB_tv006(0, 0, pVipCoefArray[VIP_QUALITY_FUNCTION_YUV2RGB], drvSkip_Array[DrvSetting_Skip_Flag_coefByY], drvSkip_Array[DrvSetting_Skip_Flag_uvOffset]);

		Scaler_SetFilm_Mode(pVipCoefArray[VIP_QUALITY_FUNCTION_Film]);

		Scaler_SetIntra(pVipCoefArray[VIP_QUALITY_FUNCTION_Intra]);

		Scaler_SetMADI(pVipCoefArray[VIP_QUALITY_FUNCTION_MA]);

		Scaler_SetTNRXC(pVipCoefArray[VIP_QUALITY_FUNCTION_tnr_xc]);

//		Scaler_SetMA_Chroma_Error(pVipCoefArray[VIP_QUALITY_FUNCTION_MA_Chroma_Error]);

		Scaler_SetMADI_HME(pVipCoefArray[VIP_QUALITY_FUNCTION_MADI_HME]);

		Scaler_SetMADI_HMC(pVipCoefArray[VIP_QUALITY_FUNCTION_MADI_HMC]);

		Scaler_SetMADI_PAN(pVipCoefArray[VIP_QUALITY_FUNCTION_MADI_PAN]);
// chen 170522
//		Scaler_SetRTNR_Noise_Measure(pVipCoefArray[VIP_QUALITY_FUNCTION_RTNR_NM]);
// end chen 170522
		Scaler_SetDiSmd(pVipCoefArray[VIP_QUALITY_FUNCTION_DI_SMD_Level]);

		Scaler_SetNew_UVC(pVipCoefArray[VIP_QUALITY_FUNCTION_COLOR_NEW_UVC]);		/*	add from Mag2 by chris*/

		// Moved to yc_separation_table.c
		//Scaler_SetColor_UV_Delay_TOP(pVipCoefArray[VIP_QUALITY_FUNCTION_UV_DELAY_TOP]);

		Scaler_SetVDCti_LPF(pVipCoefArray[VIP_QUALITY_FUNCTION_V_DCTi_LPF]);

	}
/*o------ VIP table : VIP_QUALITY_Extend_Coef ------o*/
	if (m_cbFunc_getQualityExtend != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend((VIP_SOURCE_TIMING)source);
	else
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend read fail !!!\n");

	for (ii = 0; ii < VIP_QUALITY_Extend_Coef_MAX; ii++) {
		/*VIPprintf("pVipCoefArray1[%d] = %d\n", ii, pVipCoefArray[ii]);*/
	}

	if (!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
			Scaler_SetCDSTable(pVipCoefArray[VIP_QUALITY_FUNCTION_CDS_Table]); /*planck 20101103.*/
		else
			fwif_color_set_cds_table_tv006(pVipCoefArray[VIP_QUALITY_FUNCTION_CDS_Table]);

		Scaler_SetEMF_Mk2(pVipCoefArray[VIP_QUALITY_FUNCTION_EMFMk2]);

		//Scaler_set_Adaptive_Gamma(pVipCoefArray[VIP_QUALITY_FUNCTION_ADAPTIVE_GAMMA]);//removed from merlin3, 20170606

		#ifdef LocalDimming
		Scaler_SetLocalDimmingTable(pVipCoefArray[VIP_QUALITY_FUNCTION_LD_Table]);

		Scaler_SetLocalDimmingEnable(pVipCoefArray[VIP_QUALITY_FUNCTION_LD_Enable]);
		#endif

		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
		{
			Scaler_SetLocalContrastTable(pVipCoefArray[VIP_QUALITY_FUNCTION_LC_Table]);
			fwif_color_set_LC_Global_Region_Num_Size(pVipCoefArray[VIP_QUALITY_FUNCTION_LC_Table], LC_BLOCK_60x34);
			if( drvif_color_get_LC_size_support()!=0 )
				Scaler_SetLocalContrastEnable(1);
			else
				Scaler_SetLocalContrastEnable(0);
		}
		else
		{
			fwif_color_set_LocalContrast_table_TV006(&glc_param);
			fwif_color_set_LC_Global_Region_Num_Size(pVipCoefArray[VIP_QUALITY_FUNCTION_LC_Table], LC_BLOCK_60x34);
			if( drvif_color_get_LC_size_support()!=0 )
				Scaler_SetLocalContrastEnable(1);
			else
				Scaler_SetLocalContrastEnable(0);
		}

		Scaler_Set_Dynamic_SLC_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_Dynamic_SLC_Table]);
		Scaler_Set_Dynamic_SLC_Enable(pVipCoefArray[VIP_QUALITY_FUNCTION_Dynamic_SLC_Enable]);

		/*de-xc need a few frame for memory stable after enable, enable control @ scalerMemory*/
		/*Scaler_Set_I_De_XC(pVipCoefArray[VIP_QUALITY_FUNCTION_I_De_XC]);*/
		/*dirty solution, scaler flow don't go "drv_memory_set_ip_fifo_Mer2" while PC Mode, turn off de-xc in PC mode*/
		//if ((source == VIP_QUALITY_VGA) || ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP) == 0) && (Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_RTNR) == 0)))
		Scaler_Set_I_De_XC(pVipCoefArray[VIP_QUALITY_FUNCTION_I_De_XC]);

		#ifdef AUTO_DE_CONTOUR_I2R
		if(source == VIP_QUALITY_HDMI_1080P && pVipCoefArray[VIP_QUALITY_FUNCTION_I_De_Contour]>0 && pVipCoefArray[VIP_QUALITY_FUNCTION_I_De_Contour]< I_De_Contour_TBL_Max)
			IoReg_Mask32(VGIP_VGIP_VBISLI_reg, ~_BIT20,_BIT20);
		else if(pre_source == VIP_QUALITY_HDMI_1080P ){
			IoReg_Mask32(VGIP_VGIP_VBISLI_reg, ~_BIT20,0);
			IoReg_Mask32(DMA_VGIP_DMA_VGIP_CTRL_reg,0,0);
		}
		#endif
		Scaler_Set_I_De_Contour(pVipCoefArray[VIP_QUALITY_FUNCTION_I_De_Contour]);
		Scaler_Set_Block_DeContour(pVipCoefArray[VIP_QUALITY_FUNCTION_I_De_Contour]);

		if (g_bDeContourSaved) {
			fwif_color_Set_De_Contour_tv006(&De_contour_level);
		}

#if 0/*SLD, hack, elieli*/ //k5l hw remove
		Scaler_Set_SLD(pVipCoefArray[VIP_QUALITY_FUNCTION_SLD]);
#endif



		if ((vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) && (fwif_VIP_get_Project_ID() != VIP_Project_ID_TV030)) {
			#ifndef CONFIG_RTK_SW_OUTGAMMA_CT
			Scaler_color_colorwrite_Output_InvOutput_gamma(pVipCoefArray[VIP_QUALITY_FUNCTION_Output_InvOutput_GAMMA]);
			#endif
			Scaler_Set3dLUT(pVipCoefArray[VIP_QUALITY_FUNCTION_3dLUT_Table]);
		}
	#if 0
		  if((vpq_project_id == 0x00060000 && vpqex_project_id == 0x00100000)){
		       Scaler_Set_InvGAMMA_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_InvGamma_Table]);

			Scaler_Set_InvGAMMA_Enable(pVipCoefArray[VIP_QUALITY_FUNCTION_InvGamma_Enable]);
   		}
		  #endif
	}

/*o------ VIP table : VIP_QUALITY_Extend2_Coef ------o*/
	if (m_cbFunc_getQualityExtend2 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend2((VIP_SOURCE_TIMING)source);
	else
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend2 read fail !!!\n");

	for (ii = 0; ii < VIP_QUALITY_Extend2_Coef_MAX; ii++) {
		/*VIPprintf("pVipCoefArray2[%d] = %d\n", ii, pVipCoefArray[ii]);*/
	}

	if (!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		#if 1	//20151001 jyyang, for LGE 4-tab UZU filter select
		Scaler_SetScaleUP_YCHV(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_H],
								pVipCoefArray[VIP_QUALITY_FUNCTION_SU_V],
								pVipCoefArray[VIP_QUALITY_FUNCTION_SU_C_H],
								pVipCoefArray[VIP_QUALITY_FUNCTION_SU_C_V]);

		#else
		Scaler_SetScaleUPH(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_H]);

		Scaler_SetScaleUPV(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_V]);
		#endif

		Scaler_SetScalePK(pVipCoefArray[VIP_QUALITY_FUNCTION_S_PK]);

		Scaler_SetSUPKType(pVipCoefArray[VIP_QUALITY_FUNCTION_SUPK_MASK]);

		Scaler_SetUnsharpMask(pVipCoefArray[VIP_QUALITY_FUNCTION_UNSHARP_MASK]);


		Scaler_setdESM(pVipCoefArray[VIP_QUALITY_FUNCTION_EGSM_Postshp_Table]);

		Scaler_setiESM(pVipCoefArray[VIP_QUALITY_FUNCTION_iESM_table]);

		#ifdef ENABLE_SUPER_RESOLUTION
		//Scaler_set_SuperResolution_init_table(pVipCoefArray[VIP_QUALITY_FUNCTION_SuperResolution_init_table]);

		SR_edge_gain = pVipCoefArray[VIP_QUALITY_FUNCTION_SuperResolution_edge_gain];
		Scaler_SetSR_edge_gain(pVipCoefArray[VIP_QUALITY_FUNCTION_SuperResolution_edge_gain]);

		SR_texture_gain = pVipCoefArray[VIP_QUALITY_FUNCTION_SNR_SuperResolution_texture_gain];
		Scaler_SetSR_texture_gain(pVipCoefArray[VIP_QUALITY_FUNCTION_SNR_SuperResolution_texture_gain]);
		#endif

		Scaler_SetScaleDOWNHTable(pVipCoefArray[VIP_QUALITY_FUNCTION_SD_H_Table]);

		Scaler_SetScaleDOWNVTable(pVipCoefArray[VIP_QUALITY_FUNCTION_SD_V_Table]);

		Scaler_SetScaleDOWN444To422(pVipCoefArray[VIP_QUALITY_FUNCTION_SD_444To422]);

		Scaler_SetScaleUPH_8tap(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_H_8TAP]);

		Scaler_SetScaleUPV_6tap(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_V_6TAP]);

		Scaler_SetScaleOSD_Sharpness(pVipCoefArray[VIP_QUALITY_FUNCTION_OSD_SHP]);

		Scaler_SetScaleUPDir(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_DIR]);

		Scaler_SetScaleUPDir_Weighting(pVipCoefArray[VIP_QUALITY_FUNCTION_SU_DIR_Weighting]);

		Scaler_SetScaleUP_TwoStep(pVipCoefArray[VIP_QUALITY_FUNCTION_TwoStep_SU_Table]);
	}

/*o------ VIP table : VIP_QUALITY_Extend3_Coef ------o*/
	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)source);
	else
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend3 read fail !!!\n");

	for (ii = 0; ii < VIP_QUALITY_Extend3_Coef_MAX; ii++) {
		/*VIPprintf("pVipCoefArray3[%d] = %d\n", ii, pVipCoefArray[ii]);*/
	}

	if (!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {

		if(fwif_color_get_NR_Input_tv006()!= 12) //12 is webcam. it is from hal for web cam special case.
		{
		scaler_set_PQA_table(pVipCoefArray[VIP_QUALITY_FUNCTION_PQA_Table]);
		scaler_set_PQA_Input_table(pVipCoefArray[VIP_QUALITY_FUNCTION_PQA_Input_Table]);
		}
		else
		{
			scaler_set_PQA_table(9);
			scaler_set_PQA_Input_table(9);
		}


		if (vdc_power_status)
		{
			//Scaler_module_vdc_SetConBriHueSat_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_VD_ConBriHueSat_Table]);
			_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

			if(Scaler_access_Project_TV002_Style(0, 0) == Project_TV002_Style_1) {
				Scaler_module_vdc_SetConBriHueSat_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_VD_ConBriHueSat_Table]);
			} else {

				black_levle =fwif_color_get_rgb2yuv_black_level_tv006();
				/*rtd_printk(KERN_DEBUG, TAG_NAME, "\n wwwwwwwwwww =%d \n",VIP_system_info_structure_table->Input_src_Type);*/

				if (VIP_system_info_structure_table->Input_src_Type == _SRC_TV )
				{
					input_info = 0; //HAL_VPQ_INPUT_ATV
				}
				else if (VIP_system_info_structure_table->Input_src_Type == _SRC_CVBS )
				{
					input_info = 1; //HAL_VPQ_INPUT_AV;
				}

				//20161111 roger, when sub record, don't change VD con/bri
				if (get_sub_OutputVencMode() == 0)
				{
					if (fwif_color_get_VDBlackLevel_MappingValue(input_info, black_levle-1, &mCon, &mBri, &mHue, &mSat) == 0){
							/*rtd_printk(KERN_DEBUG, TAG_NAME, "\n mCon=%d mBri=%d mHue=%d mSat=%d \n",mCon,mBri,mHue,mSat);*/
							fwif_color_set_VDBlackLevel(mCon, mBri, mHue, mSat);
						}
				}
			}
		}
		if(vpq_project_id == 0x00060000 && vpqex_project_id == 0x00100000){
			Scaler_Set_DCC_Color_Independent_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Skin_Tone_Table_Select]);

			Scaler_Set_DCC_chroma_compensation_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Chroma_comp_t_sel]);
		}
		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) { /*pre_id:16 main_id:8 sub1_id:8 sub2_id:8, example->lg=tv006*/
			Scaler_set_ICM_table(pVipCoefArray[VIP_QUALITY_FUNCTION_ICM],  drvSkip_Array[DrvSetting_Skip_Flag_ICM]);

			Scaler_SetGamma_for_Handler(pVipCoefArray[VIP_QUALITY_FUNCTION_Gamma],
												pVipCoefArray[VIP_QUALITY_FUNCTION_S_Gamma_Index],
												pVipCoefArray[VIP_QUALITY_FUNCTION_S_Gamma_Low],
												pVipCoefArray[VIP_QUALITY_FUNCTION_S_Gamma_High], drvSkip_Array[DrvSetting_Skip_Flag_Gamma]);
			
            fwif_color_set_InvOutputGamma_System(pVipCoefArray[VIP_QUALITY_FUNCTION_Gamma]);
			Scaler_SetDCC_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Table_Select]);

			Scaler_Set_DCC_Color_Independent_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Skin_Tone_Table_Select]);

			Scaler_Set_DCC_chroma_compensation_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Chroma_comp_t_sel]);
		}

		fwif_color_ICM_RGB_Offset_Table_Select(_MAIN_DISPLAY, pVipCoefArray[VIP_QUALITY_FUNCTION_ICM]);

		Scaler_SetSharpnessTable(pVipCoefArray[VIP_QUALITY_FUNCTION_SharpTable]);

		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) {
			Scaler_SetMBPeaking(pVipCoefArray[VIP_QUALITY_FUNCTION_MB_Peaking]);
			Scaler_SetMBSUPeaking(pVipCoefArray[VIP_QUALITY_FUNCTION_MB_SU_Peaking]);
			//Scaler_set_blue_stretch(pVipCoefArray[VIP_QUALITY_FUNCTION_blue_stretch]); //k5l hw remove
		} else{
			fwif_color_set_MultiBand_Peaking_tv006(pVipCoefArray[VIP_QUALITY_FUNCTION_MB_Peaking], NULL, NULL);


                #ifdef LGDB_OFF //control table by rtk
			Scaler_SetMBPeaking(pVipCoefArray[VIP_QUALITY_FUNCTION_MB_Peaking]); //fixed me after need to remove
			Scaler_SetMBSUPeaking(pVipCoefArray[VIP_QUALITY_FUNCTION_MB_SU_Peaking]); //fixed me after need to remove
			Scaler_SetDNR(4); //fixed me after demo please remove
			Scaler_SetMPEGNR(4, CALLED_NOT_BY_OSD); //fixed me after demo please remove
                #endif
		}

		Scaler_SetNNSR_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_SRNN_Ctrl]);
	}

	Scaler_color_Set_PQ_ByPass_Lv(0);	/* resort PQ bypass lv, since all PQ function is reset*/

	fwif_color_set_quality_init_sendRPC();
	scalerVIP_quality_init((unsigned long)&quality_init_value[0]);

	fwif_color_Set_MNR_Enable_Check_tv006();/*snr hw issue, can't get porch while hal function calling, elieli*/

	/* LineOD bring up verify */
	fwif_color_set_pcid2(1); // enable to set the default setting
	fwif_color_set_pcid2(0); // disable

	/* VALC bring up verify */
	fwif_color_set_valc(1); // enable to set the default setting
	fwif_color_set_valc(0); // disable

	/* Frequency detection bring up verify */
	if (!g_bFreqDet_Setting_by_ISR) {
		fwif_color_set_frequency_detection(pVipCoefArray[VIP_QUALITY_FUNCTION_FreqencyDetection]);
	} else {
		drvif_color_set_frequency_detection_enable(1);
	}

	/* CDLC bring up verify */
	fwif_color_Set_CDLC(0);
	
	/* LC overlay_APL bring up verify */
      fwif_color_Set_LC_overlay_APL(0);

	/*fwif_color_video_quality_handler_customization();*/

	/* 3d mode young 3dtable ================*/
	if (drvif_scaelr3d_decide_is_3D_display_mode() == 1) {/*3*/
		/*fwif_color_video_3Dmode_quality_handler();*/
	}

#if 0	//20160926 roger, KTASKWBS-5252 root cause
	// patch for YCbCr color issue in hi-freq. area
#ifdef CONFIG_FORCE_RUN_I3DDMA
	if (input_display == SLR_MAIN_DISPLAY && Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC)
		IoReg_SetBits(H3DDMA_HSD_I3DDMA_VSD_Ctrl1_reg, H3DDMA_HSD_I3DDMA_VSD_Ctrl1_lp121_en_mask);
	else
		IoReg_ClearBits(H3DDMA_HSD_I3DDMA_VSD_Ctrl1_reg, H3DDMA_HSD_I3DDMA_VSD_Ctrl1_lp121_en_mask);
#endif
	// ============= patch end ====================
#endif

	if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV030) {
		extern unsigned char ColorSpaceMode;
		/*===========HDR===============*/
		printk("yuan,ColorSpaceMode=%d\n",ColorSpaceMode);
		//printk("yuan,HDR_flag=%d\n",VIP_system_info_structure_table ->HDR_flag);

		if(ColorSpaceMode==0){//Color Space Mode=Auto
			drvif_color_D_3dLUT_Enable(0);
			fwif_color_set_sRGBMatrix();
		}
		else if(ColorSpaceMode==1){//Color Space Mode=Native
			drvif_color_D_3dLUT_Enable(0);
			fwif_color_set_color_mapping_enable(SLR_MAIN_DISPLAY, 0);
		}
		else if(ColorSpaceMode==2){//Color Space Mode=Custom
			Scaler_Set_ColorSpace_D_3dlutTBL(&ColoSpace_Index);
			drvif_color_D_3dLUT_Enable(1);
			fwif_color_set_sRGBMatrix();
		}

		if(VIP_system_info_structure_table ->HDR_flag && (VIP_system_info_structure_table ->HDR_flag != HAL_VPQ_HDR_MODE_SDR_MAX_RGB)){
			fwif_color_set_sRGBMatrix();
			drvif_color_D_3dLUT_Enable(0);
			//GammaBlendOETF=0;
		}
		fwif_color_set_gamma_Magic();

		printk("yuan,qua end===\n");
	}else if (Scaler_access_Project_TV002_Style(0, 0) == Project_TV002_Style_1) {
		extern unsigned char g_bEnable_PQ_extend_data;
		extern short ColorMap_Matrix_Apply[3][3];
		if (!g_bEnable_PQ_extend_data)
			fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, ColorMap_Matrix_Apply, 1, 1);
	}
    
    #ifdef SQM_OFFSET_TBL
    if(pre_source != source){
		    rtd_printk(KERN_EMERG, TAG_NAME, "!!!update SQM status @src!!!!!\n");
            updateSQM_flag=1;
    }
    #endif
    
	pre_source = source;
	/*force background check*/
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	ppoverlay_main_display_control_rsv.regValue= IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	if (ppoverlay_main_display_control_rsv.m_force_bg == 0) /*normal mode*/
		VIPprintf("[Warnning] [%s-%s-%d] End, Not Black Screen", __FILE__, __func__, __LINE__);
#if 0
	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY) {
		if(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>1920){
			IoReg_SetBits(DI_IM_DI_RTNR_NEW_CONTROL_reg, _BIT19);
		}else{
			IoReg_ClearBits(DI_IM_DI_RTNR_NEW_CONTROL_reg, _BIT19);
		}
	}
#endif
	rtd_printk(KERN_INFO, TAG_NAME, "[Decide RTNR][RTNR_BW Status][%d][%x]\n",(rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg)>>19)&0x1, rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg));

}

#endif /*ifdef ENABLE_RUN_ON_TVBOX*/

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
void* scaler_VIP_PQMask_GetSemanticTable(void) {

	unsigned char *pVipCoefArray = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if( m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM ) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "[%s] source num %d errorn", __FUNCTION__, m_nowSource);
		return NULL;
	}

	if (m_cbFunc_getQualityExtend != NULL) {
		rtd_printk(KERN_INFO, TAG_NAME, "[%s] CoefArray with source %d\n", __FUNCTION__, m_nowSource);
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend((VIP_SOURCE_TIMING)m_nowSource);
	} else {
		rtd_printk(KERN_DEBUG, TAG_NAME, "[%s] PQ table m_cbFunc_getQualityExtend read failn", __FUNCTION__);
	}

	if( gVip_Table == NULL ) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "[%s] VIP table null\n", __FUNCTION__);
		return NULL;
	}

	if (!(m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		if( pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskSemanticWeight_Table] >= PQMASK_SEMANTIC_WEIGHT_NUM ) {
			rtd_printk(KERN_DEBUG, TAG_NAME, "[%s] Table index = %d over range\n", __FUNCTION__, pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskSemanticWeight_Table]);
			return NULL;
		} else {
			rtd_printk(KERN_INFO, TAG_NAME, "[%s] Table src = %d, index = %d\n", __FUNCTION__, m_nowSource, pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskSemanticWeight_Table]);
			return (void *)(&gVip_Table->PQMask_Semantic_WeightTable[pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskSemanticWeight_Table]]);
		}
	} else {
		return NULL;
	}

	return NULL;
}

void* scaler_VIP_PQMask_GetDepthTable(void) {

	unsigned char *pVipCoefArray = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if( m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM ) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "[%s] source num %d errorn", __FUNCTION__, m_nowSource);
		return NULL;
	}

	if (m_cbFunc_getQualityExtend != NULL) {
		rtd_printk(KERN_INFO, TAG_NAME, "CoefArray with source %d\n", m_nowSource);
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend((VIP_SOURCE_TIMING)m_nowSource);
	} else {
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend read failn");
	}

	if( gVip_Table == NULL ) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "VIP table null\n");
		return NULL;
	}

	if (!(m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		if( pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskDepthWeight_Table] >= PQMASK_DEPTH_WEIGHT_NUM ) {
			rtd_printk(KERN_DEBUG, TAG_NAME, "PQMask Depth Table index = %d over range\n", pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskDepthWeight_Table]);
			return NULL;
		} else {
			rtd_printk(KERN_INFO, TAG_NAME, "PQMask Depth Table src = %d, index = %d\n", m_nowSource, pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskDepthWeight_Table]);
			return (void *)(&gVip_Table->PQMask_Depth_WeightTable[pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskDepthWeight_Table]]);
		}
	} else {
		return NULL;
	}

	return NULL;
}

// for scene effect reset LC
void Scaler_VIP_LC_reset(void)
{
	unsigned char *pVipCoefArray = NULL;

	if (m_cbFunc_getQualityExtend != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend((VIP_SOURCE_TIMING)m_nowSource);
	else
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend read fail !!!\n");

	if (!(m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000){
		Scaler_SetLocalContrastTable(pVipCoefArray[VIP_QUALITY_FUNCTION_LC_Table]);

		Scaler_SetLocalContrastEnable(pVipCoefArray[VIP_QUALITY_FUNCTION_LC_Enable]);
		}
		else{
			fwif_color_set_LocalContrast_table_TV006(&glc_param);//fixed me
			//Scaler_SetLocalContrastTable(pVipCoefArray[VIP_QUALITY_FUNCTION_LC_Table]);

			Scaler_SetLocalContrastEnable(pVipCoefArray[VIP_QUALITY_FUNCTION_LC_Enable]);

		};
	}

}

// for scene effect reset shp & cds cm1, cm2.
void Scaler_VIP_scene_sharpness_reset(void)
{
	unsigned char *pVipCoefArray = NULL;
	VIP_CDS_Table CDS_Table;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	// for reset sharpness
	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)m_nowSource);
	else
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend3 read fail !!!\n");

	//if (!(m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
	if (m_nowSource < VIP_QUALITY_TOTAL_SOURCE_NUM && pVipCoefArray != NULL) {
		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) { /*pre_id:16 main_id:8 sub1_id:8 sub2_id:8, example->lg=tv006*/
			Scaler_SetSharpnessTable(pVipCoefArray[VIP_QUALITY_FUNCTION_SharpTable]);
		}
	}

	// for reset cds
	if (m_cbFunc_getQualityExtend != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend((VIP_SOURCE_TIMING)m_nowSource);
	else{
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend read fail !!!\n");
		return;
	}

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	memcpy(&CDS_Table, &(gVip_Table->CDS_ini[pVipCoefArray[VIP_QUALITY_FUNCTION_CDS_Table]]), sizeof(VIP_CDS_Table));

	drvif_color_set_cds_table_AI((VIP_CDS_Table *) (&CDS_Table), 0,1,1,0);


}

// for face drrect reset
void Scaler_VIP_DCC_Color_Independent_reset(void)
{
	unsigned char *pVipCoefArray = NULL;

	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)m_nowSource);
	else{
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend3 read fail !!!\n");
		return;
	}

	Scaler_Set_DCC_Color_Independent_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Skin_Tone_Table_Select]);
}
#endif

void Scaler_VIP_IP_3DFlow_init(unsigned char Vip_source)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	drvif_color_dcc_Curve_init(display);
}


void fwif_color_video_3Dmode_quality_handler(void)
{
	/*unsigned char ii;*/
	/*unsigned char which_value, pre_value;*/
	/*unsigned char temptable[5] = {7, 1, 1, 1, 1};*/
	unsigned char source = 255;
	//unsigned char input_display;
	unsigned char *pVipCoefArray = NULL;
	/*int ret;*/
	/*unsigned char spatial_y = 0, spatial_c = 0, temporal = 0, rtnr_level_y = 0, rtnr_level_c = 0, s_dnr_level = 0;*/

	SLR_VIP_TABLE *gVIP_Table;
	unsigned char *drvSkip_Array;

	//input_display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	source = fwif_vip_3Dsource_check(3, NOT_BY_DISPLAY);

	gVIP_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (NULL == gVIP_Table)
		return;

	drvSkip_Array = &(gVIP_Table->DrvSetting_Skip_Flag[0]);

	VIPprintf("====[Young]3D source = %d ====\n", source);

	if (m_cbFunc_getQuality3Dmode != NULL) {
		pVipCoefArray = (unsigned char *)m_cbFunc_getQuality3Dmode((VIP_3DSOURCE_TIMING)source);
	}
	if (!(pVipCoefArray == NULL)) {
		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) {
			Scaler_SetSindex(pVipCoefArray[VIP_QUALITY_3DFUNCTION_S_Gamma_Index]);

			Scaler_SetSlow(pVipCoefArray[VIP_QUALITY_3DFUNCTION_S_Gamma_High]);

			Scaler_SetShigh(pVipCoefArray[VIP_QUALITY_3DFUNCTION_S_Gamma_Low]);
		}

		Scaler_SetScalePK(pVipCoefArray[VIP_QUALITY_3DFUNCTION_S_PK]);

		Scaler_SetSUPKType(pVipCoefArray[VIP_QUALITY_3DFUNCTION_SUPK_MASK]);

		Scaler_SetUnsharpMask(pVipCoefArray[VIP_QUALITY_3DFUNCTION_UNSHARP_MASK]);

		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) {
			Scaler_SetSharpnessTable(pVipCoefArray[VIP_QUALITY_3DFUNCTION_Sharpness_table]);

			Scaler_SetDCC_Table(pVipCoefArray[VIP_QUALITY_3DFUNCTION_DCC_Table_Select]);

			/*VIPprintf("In 3D mode!Use 3D_ICM\n");*/
			Scaler_set_ICM_table(pVipCoefArray[VIP_QUALITY_3DFUNCTION_ICM], drvSkip_Array[DrvSetting_Skip_Flag_ICM]);
		}
		fwif_color_set_iESM(Scaler_DispGetInputInfo(SLR_INPUT_DATA_ARRAY_IDX), pVipCoefArray[VIP_QUALITY_3DFUNCTION_iESM_table]);

		/*USER:LewisLee DATE:2010/11/11*/
		/*In MediaBrowser mode, not execute Edge Smooth mode*/
		/*to prevent OSD jitter*/
		if ((_SRC_VO == Scaler_InputSrcGetMainChType()) && (SLR_RATIO_CUSTOM == Scaler_DispGetRatioMode2())) {
			/*VIPprintf("Media Previrw mode, not execute EGEM!\n");*/
		} else {
			/*========= planck add for TV AutoNR ===========*/
			if ((Scaler_SendRPC(SCALERIOC_GET_TABLE_SHP1D2D_EGSM, pVipCoefArray[VIP_QUALITY_3DFUNCTION_SHP1D2D_EGSM], RPC_SEND_VALUE_ONLY)) < 0) {
				/*rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, update TableNR_RTNR_C to driver fail !!!\n", ret);*/
			}

		}
	}
}

void *fwif_color_get_picture_mode_data(unsigned char src_idx, unsigned char value)
{
	if (m_cbFunc_getPictureModeData == NULL)
		return NULL;

	/*VIPprintf("fwif_color_get_picture_mode_data, src_idx = %d, value = %d, Scaler_InputSrcFormat(src_idx) = %d\n", src_idx, value, Scaler_InputSrcFormat(src_idx));*/
	return m_cbFunc_getPictureModeData(Scaler_InputSrcFormat(src_idx), 0, value);
}

void *fwif_color_get_cur_picture_mode_data(unsigned char src_idx)
{
	/* Get current active picture mode level by setting param to 0xFF.*/
	SLR_PICTURE_MODE_DATA *pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_picture_mode_data(src_idx, 0xFF);
	return pData;
}


unsigned char fwif_color_get_backlight(unsigned char src_idx)
{
	/*StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);*/
	SLR_PICTURE_MODE_DATA *pTable = (SLR_PICTURE_MODE_DATA *)fwif_color_get_picture_mode_data(src_idx, 0xff);
	if (pTable == NULL)
		return 0;

	return pTable->BackLight;
}

/*peggy: set value => must be user picmode*/
void fwif_color_set_backlight(unsigned char src_idx, unsigned char value)
{
		/*StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);*/
		SLR_PICTURE_MODE_DATA *pTable = (SLR_PICTURE_MODE_DATA *)fwif_color_get_picture_mode_data(src_idx, 0xff);

	if (pTable == NULL)
		return;
	pTable->BackLight = value;
	Scaler_SetOSDBacklight2VIP(value);
	if(Scaler_access_Project_TV002_Style(0, 0) == Project_TV002_Style_1)
		fwif_WBBL_TV002();
}

void fwif_color_set_backlight_TPV(unsigned char src_idx, unsigned char value)
{
	/*StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);*/
#if (!defined(PICTURE_MODE_USER))
	SLR_PICTURE_MODE_DATA *pTable = (SLR_PICTURE_MODE_DATA *)fwif_color_get_picture_mode_data(src_idx, (unsigned char)0xFF);
#else
	SLR_PICTURE_MODE_DATA *pTable = (SLR_PICTURE_MODE_DATA *)fwif_color_get_picture_mode_data(src_idx, (unsigned char)PICTURE_MODE_USER);
#endif
	int ret;
	ret = Scaler_SendRPC(SCALERIOC_DCR_MAX_VALUE, value, RPC_SEND_VALUE_ONLY);
	if (0 != ret) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, set SCALERIOC_DCR_MAX_VALUE  to driver fail !!!\n", ret);
		return;
	}
	if (pTable == NULL)
		return;
	pTable->BackLight = value;
}


unsigned short fwif_color_get_color_temp_r_type_using_colortmp(unsigned char colortmp)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return 0;
	return pTable->R_val;
}

void fwif_color_set_color_temp_r_type_using_colortmp(unsigned char colortmp, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return;
	pTable->R_val = value;
}

unsigned short fwif_color_get_color_temp_g_type_using_colortmp(unsigned char colortmp)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return 0;
	return pTable->G_val;
}

void fwif_color_set_color_temp_g_type_using_colortmp(unsigned char colortmp, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return;
	pTable->G_val = value;
}

unsigned short fwif_color_get_color_temp_b_type_using_colortmp(unsigned char colortmp)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return 0;
	return pTable->B_val;
}

void fwif_color_set_color_temp_b_type_using_colortmp(unsigned char colortmp, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return;
	pTable->B_val = value;
}

unsigned short fwif_color_get_color_temp_r_offset_using_colortmp(unsigned char colortmp)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return 0;
	return pTable->R_offset_val;
}

void fwif_color_set_color_temp_r_offset_using_colortmp(unsigned char colortmp, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return;
	pTable->R_offset_val = value;
}

unsigned short fwif_color_get_color_temp_g_offset_using_colortmp(unsigned char colortmp)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return 0;
	return pTable->G_offset_val;
}

void fwif_color_set_color_temp_g_offset_using_colortmp(unsigned char colortmp, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return;
	pTable->G_offset_val = value;

}

unsigned short fwif_color_get_color_temp_b_offset_using_colortmp(unsigned char colortmp)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return 0;
	return pTable->B_offset_val;
}


void fwif_color_set_color_temp_b_offset_using_colortmp(unsigned char colortmp, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = fwif_color_get_color_tempset_using_colortmp(colortmp);
	if (NULL == pTable)
		return;
	pTable->B_offset_val = value;
}




unsigned char fwif_color_get_picture_mode(unsigned char src_idx)
{
	SLR_PICTURE_MODE_DATA *pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return VIP_COLORSTD_DEFAULT;
	return pData->mode;
}

unsigned char fwif_color_get_vip_debug_mode(unsigned char src_idx)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->VIP_debug_mode;
}

unsigned char Scaler_get_i_dcti()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->I_DCTi;
}

void Scaler_set_i_dcti(unsigned char src_idx, unsigned char value)
{

	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->I_DCTi = value;
}

void fwif_color_set_Vertical_NR_driver(unsigned char src_idx, unsigned char value)
{
	drvif_color_Mixer_ctrl_Vfilter(_MAIN_DISPLAY, value);
}

#ifdef ENABLE_SUPER_RESOLUTION
void fwif_color_set_SR_edge_gain(unsigned char src_idx, unsigned char value)
{

	fwif_color_set_SR_driver(VIP_SR_edge_gain, value);

}

void fwif_color_set_SR_texture_gain(unsigned char src_idx, unsigned char value)
{

	fwif_color_set_SR_driver(VIP_SR_texture_gain, value);

}

void fwif_color_set_SR_driver(unsigned char item_idx, unsigned char value)
{
	switch (item_idx) {
	case VIP_SR_edge_gain:
		drv_SuperResolution_gain(value, SR_texture_gain);
		break;
	case VIP_SR_texture_gain:
		drv_SuperResolution_gain(SR_edge_gain, value);
		break;
	default:
		drv_SuperResolution_gain(SR_edge_gain, SR_texture_gain);
		break;
	}
}
#endif


void fwif_color_set_HMCNR_driver(unsigned char src_idx, unsigned char value)
{
	drvif_color_nr_HMCNR(value);
}


void fwif_color_set_HMCNR_blending_driver(unsigned char src_idx, unsigned char value)
{
	drvif_color_nr_HMCNR_blending(value);
}

#if 0
void fwif_color_set_active_backlight_table(unsigned char src_idx, unsigned char item, unsigned char value)
{

	int ret;
	unsigned char Active_Backlight_info[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	VIPprintf("fwif_color_set_active_backlight_table:::item = %d, value = %d \n", item, value);

	UINT16 Scaler_BlackAPL_a_HSCA = 0;
	UINT16 Scaler_BlackAPL_b_HSCB = 0;
	UINT16 Scaler_BlackAPL_c_HSCC = 0;
	UINT16 Scaler_BlackAPL_d_HSCD = 0;
	UINT16 Scaler_BlackAPL_e_HSCE = 0;
	UINT16 Scaler_BlackAPL_f_HSCF = 0;
	UINT16 Scaler_BlackAPL_g_HSCG = 0;
	UINT16 Scaler_BlackAPL_h_HSCH = 0;

	UINT16 Scaler_BlackAPL_BL_Th1_SABH = 0;
	UINT16 Scaler_BlackAPL_BL_Th1_SABL = 0;
	UINT16 Scaler_BlackAPL_BL_Th2_EABH = 0;
	UINT16 Scaler_BlackAPL_BL_Th2_EABL = 0;
	UINT16 Scaler_BlackAPL_BL_Th3_MLBL = 0;
	UINT16 Scaler_BlackAPL_BL_Th1 = 0;
	UINT16 Scaler_BlackAPL_BL_Th2 = 0;
	UINT16 Scaler_BlackAPL_M_ATAB = 0;
	UINT16 Scaler_BlackAPL_KU_USAB = 0;
	UINT16 Scaler_BlackAPL_KD_DSAB = 0;


	Active_Backlight_info[0] = Scaler_BlackAPL_a_HSCA;
	Active_Backlight_info[1] = Scaler_BlackAPL_b_HSCB;
	Active_Backlight_info[2] = Scaler_BlackAPL_c_HSCC;
	Active_Backlight_info[3] = Scaler_BlackAPL_d_HSCD;
	Active_Backlight_info[4] = Scaler_BlackAPL_e_HSCE;
	Active_Backlight_info[5] = Scaler_BlackAPL_f_HSCF;
	Active_Backlight_info[6] = Scaler_BlackAPL_g_HSCG;
	Active_Backlight_info[7] = Scaler_BlackAPL_h_HSCH;

	Active_Backlight_info[8] = Scaler_BlackAPL_BL_Th1_SABH;
	Active_Backlight_info[9] = Scaler_BlackAPL_BL_Th1_SABL;
	Active_Backlight_info[10] = Scaler_BlackAPL_BL_Th2_EABH;
	Active_Backlight_info[11] = Scaler_BlackAPL_BL_Th2_EABL;
	Active_Backlight_info[12] = Scaler_BlackAPL_BL_Th3_MLBL;
	Active_Backlight_info[13] = Scaler_BlackAPL_M_ATAB;
	Active_Backlight_info[14] = Scaler_BlackAPL_KU_USAB;
	Active_Backlight_info[15] = Scaler_BlackAPL_KD_DSAB;

	Active_Backlight_info[16] = Scaler_APL_BL_MAX;
	Active_Backlight_info[17] = Scaler_APL_BL_MIN;
	if (item <= SLR_APL_BL_MIN) {
		Active_Backlight_info[item] = value;
	}

	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_UPDATE_ACTIVE_BACKLIGHT_TABLE);
	if (pwTemp) {
		/* copy to RPC share memory*/
		memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_UPDATE_ACTIVE_BACKLIGHT_TABLE), &Active_Backlight_info, sizeof(unsigned char)*18);
		if ((Scaler_SendRPC(SCALERIOC_UPDATE_ACTIVE_BACKLIGHT_TABLE, ShareMemAddrDummy, RPC_Send_Addr)) < 0) {
			rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, UPDATE_ACTIVE_BACKLIGHT_TABLE to driver fail !!!\n", ret);
		}
	}
}
#endif
unsigned char fwif_color_get_grid_remove(unsigned char src_idx)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->gridRemove;
}

void fwif_color_set_grid_remove(unsigned char src_idx, unsigned char value)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->gridRemove = value;
}
/*
unsigned char fwif_color_get_conti_still_line(unsigned char src_idx)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ContiStillLine;
}

void fwif_color_set_conti_still_line(unsigned char src_idx, unsigned char value)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ContiStillLine = value;
}
*/
unsigned char fwif_color_reg_callback1(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC1 cbFunc)
{
	if (NULL == cbFunc) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "error : fwif_color_reg_callback1, type = %d cbFunc == NULL\n", type);
		return false;
	}


	switch (type) {
	case COLOR_CALLBACK_GET_QUALITY_COEF:
		m_cbFunc_getQualityCoef = cbFunc;
		break;
	case COLOR_CALLBACK_GET_QUALITY_EXTEND:
		m_cbFunc_getQualityExtend = cbFunc;
		break;
	case COLOR_CALLBACK_GET_QUALITY_EXTEND2:
		m_cbFunc_getQualityExtend2 = cbFunc;
		break;
	case COLOR_CALLBACK_GET_QUALITY_EXTEND3:
		m_cbFunc_getQualityExtend3 = cbFunc;
		break;
	case COLOR_CALLBACK_GET_QUALITY_EXTEND4:
		m_cbFunc_getQualityExtend4 = cbFunc;
		break;

	default:
		return false;
	}

	return TRUE;
}


unsigned char fwif_color_reg_callback2(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC2 cbFunc)
{
	if (NULL == cbFunc) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "error : fwif_color_reg_callback2, type = %d cbFunc == NULL\n", type);
		return false;
	}

	switch (type) {
	case COLOR_CALLBACK_GET_COLOR_DATA:
		m_cbFunc_getColorData = cbFunc;
		break;
	case COLOR_CALLBACK_GET_COLOR_FAC_MODE:
		m_cbFunc_getColorFacMode = cbFunc;
		break;
	case COLOR_CALLBACK_GET_COLOR_TEMP_DATA:
		m_cbFunc_getColorTempData = cbFunc;
		break;
	case COLOR_CALLBACK_GET_COLOR_TEMP_DATA_TSB:
		m_cbFunc_getColorTempData_TSB = cbFunc;
		break;
	case COLOR_CALLBACK_GET_COLOR_TEMP_LEVEL:
		m_cbFunc_getColorTempLevel = cbFunc;
		break;
	case COLOR_CALLBACK_GET_PICTURE_MODE_DATA:
		m_cbFunc_getPictureModeData = cbFunc;
		break;
	case COLOR_CALLBACK_GET_DNR_MODE:
		m_cbFunc_getDnrMode = cbFunc;
		break;

	default:
		return false;
	}

	return TRUE;
}

unsigned char fwif_color_reg_callback3(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC3 cbFunc)
{
	if (NULL == cbFunc) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "error : fwif_color_reg_callback3, type = %d cbFunc == NULL\n", type);
		return false;
	}

	switch (type) {
	case COLOR_CALLBACK_GET_CURPICMODE:
		m_cbFunc_getCurPicMode = cbFunc;
		break;
	default:
		return false;
	}

	return TRUE;
}

/*Elsie 20131224*/
unsigned char fwif_color_reg_callback4(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC4 cbFunc)
{
	if (NULL == cbFunc) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "error : fwif_color_reg_callback4, type = %d cbFunc == NULL\n", type);
		return false;
	}

	switch (type) {
	case COLOR_CALLBACK_GET_ICM_BY_TIMING_PICMODE:
		m_cbFunc_getIcmByTimingPicmode = cbFunc;
		break;
	default:
		return false;
	}

	return TRUE;
}

/*young 3Dtable*/
unsigned char fwif_color_reg_callback5(COLOR_CALLBACK_TYPE type, COLOR_CALLBACK_FUNC5 cbFunc)
{
	if (NULL == cbFunc) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "error : fwif_color_reg_callback5, type = %d cbFunc == NULL\n", type);
		return false;
	}

	switch (type) {

	case COLOR_CALLBACK_GET_QUALITY_3Dmode:
		m_cbFunc_getQuality3Dmode = cbFunc;
		break;

	default:
		return false;
	}

	return TRUE;
}

#if 0
char fwif_color_isUSBSource()
{
	if (m_cbFunc_getCurVipSource != NULL) {
		if (m_cbFunc_getCurVipSource() == VIP_SRC_VO_USB)
			return TRUE;
	}
	return false;
}
#endif

unsigned char fwif_color_get_spread_spectrum(unsigned char src_idx)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Spread_Spectrum;
}

void fwif_color_set_spread_spectrum(unsigned char src_idx, unsigned char value)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Spread_Spectrum = value;
}

void fwif_color_set_vip_debug_mode(unsigned char src_idx, unsigned char value)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->VIP_debug_mode = value;
}

void fwif_set_ColorMatrix(bool colormartixonoff)
{
	bool mode = 0;
	unsigned short channel;
	SCALER_DISP_CHANNEL display;
	display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	channel = Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL);

	if (colormartixonoff == 1)
		mode = 1;
	/*VIPprintf("[Young]fwif_set_ColorMatrix colormartixonoff = %d\n", colormartixonoff);*/
	fwif_color_colorspacergb2yuvtransfer(channel, Scaler_InputSrcGetType(display), mode , 0, 0);
}

int fwif_color_get_cur_picMode()
{
	if (m_cbFunc_getCurPicMode == NULL)
		return 0;

	return m_cbFunc_getCurPicMode();
}

/*=====================================*/
/*=============== NEW RTNR ===============*/
/*=====================================*/
void  Scaler_icm_enhancement(unsigned char value)
{
	unsigned char which_value;
	unsigned char source = 255;
	unsigned char *pVipCoefArray = NULL;


	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);

	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)source);

	if(pVipCoefArray==NULL)
		return;

	which_value = pVipCoefArray[VIP_QUALITY_FUNCTION_ICM];

	fwif_color_icm_enhancement(SLR_MAIN_DISPLAY, which_value, value);
}
void  Scaler_ChannelColourSet(signed char value)
{
	unsigned char which_value;
	unsigned char source = 255;
	unsigned char *pVipCoefArray = NULL;


	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);

	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)source);

	if(pVipCoefArray==NULL)
		return;

	which_value = pVipCoefArray[VIP_QUALITY_FUNCTION_ICM];

	fwif_color_ChannelColourSet(SLR_MAIN_DISPLAY, which_value, value);
}


#define GAMMA_R 0
#define GAMMA_G 1
#define GAMMA_B 2




#define GET_PIP_BLENDING() 						Scaler_PipGetInfo(SLR_PIP_BLENDING)
/*void fwif_color_setrgbcontrast_for_engOSD(UINT8 display, UINT8 RGB, UINT8 Value)
{
	UINT16 Red, Green, Blue;

	Red = Green = Blue = 0;
}*/


UINT16 COsdCtrlGet_R_Drive_RDRV()
{
	return 0;
}

void COsdCtrlSet_R_Drive_RDRV(UINT16  Message)
{
}
UINT16 COsdCtrlGet_G_Drive_GDRV()
{
	return 0;
}

void COsdCtrlSet_G_Drive_GDRV(UINT16  Message)
{
}

UINT16 COsdCtrlGet_B_Drive_BDRV(void)
{
	return 0;

}

void COsdCtrlSet_B_Drive_BDRV(UINT16  Message)
{
}

INT8 COsdCtrlGet_R_CUTOFF_RCUT(void)
{
	return 0;

}

void COsdCtrlSet_R_CUTOFF_RCUT(UINT16  Message)
{
}

INT8 COsdCtrlGet_G_CUTOFF_GCUT(void)
{
	return 0;

}

void COsdCtrlSet_G_CUTOFF_GCUT(UINT16  Message)
{
}

INT8 COsdCtrlGet_B_CUTOFF_BCUT()
{
	return 0;

}

void COsdCtrlSet_B_CUTOFF_BCUT(UINT16  Message)
{
}

INT8 COsdCtrlGet_R_Drive_Offset_Natural_RDON()
{
	/*drive offset is the diff base on cool*/
	return 0;
}
void COsdCtrlSet_R_Drive_Offset_Natural_RDON(UINT16  Message)
{
	/*set value to storage*/
	/*set value to reg*/
}

INT8 COsdCtrlGet_G_Drive_Offset_Natural_GDON()
{
	/*drive offset is the diff base on cool*/
	return 0;

}

void COsdCtrlSet_G_Drive_Offset_Natural_GDON(UINT16  Message)
{
	/*set value to storage*/
	/*set value to reg*/
}

INT8 COsdCtrlGet_B_Drive_Offset_Natural_BDON()
{
	/*drive offset is the diff base on cool*/
	return 0;
}

void COsdCtrlSet_B_Drive_Offset_Natural_BDON(UINT16  Message)
{
	/*set value to storage*/
	/*set value to reg*/
}

INT8 COsdCtrlGet_R_CUTOFF_Offset_Natural_RCON()
{
	return 0;
}

void COsdCtrlSet_R_CUTOFF_Offset_Natural_RCON(UINT16  Message)
{
}

INT8 COsdCtrlGet_G_CUTOFF_Offset_Natural_GCON()
{
	return 0;
}

void COsdCtrlSet_G_CUTOFF_Offset_Natural_GCON(UINT16  Message)
{
}

INT8 COsdCtrlGet_B_CUTOFF_Offset_Natural_BCON()
{
	/*cutoff offset for natural = cutoff for natural - cutoff for cool*/
	return 0;
}

void COsdCtrlSet_B_CUTOFF_Offset_Natural_BCON(UINT16  Message)
{
	/*set value to storage*/
}

INT8 COsdCtrlGet_R_Drive_Offset_Warm_RDOW()
{
	/*drive offset is the diff base on cool*/
	return 0;
}

void COsdCtrlSet_R_Drive_Offset_Warm_RDOW(UINT16  Message)
{
	/*set value to storage*/
	/*set value to reg*/
}

INT8 COsdCtrlGet_G_Drive_Offset_Warm_GDOW()
{
	/*drive offset is the diff base on cool*/
	return 0;
}

void COsdCtrlSet_G_Drive_Offset_Warm_GDOW(UINT16  Message)
{
	/*set value to storage*/
	/*set value to reg*/
}

INT8 COsdCtrlGet_B_Drive_Offset_Warm_BDOW()
{
	/*drive offset is the diff base on cool*/
	return 0;

}

void COsdCtrlSet_B_Drive_Offset_Warm_BDOW(UINT16  Message)
{
	/*set value to storage*/
	/*set value to reg*/
}

INT8 COsdCtrlGet_R_CUTOFF_Offset_Warm_RCOW()
{
	return 0;
}

void COsdCtrlSet_R_CUTOFF_Offset_Warm_RCOW(UINT16  Message)
{
}

INT8 COsdCtrlGet_G_CUTOFF_Offset_Warm_GCOW()
{
	return 0;
}

void COsdCtrlSet_G_CUTOFF_Offset_Warm_GCOW(UINT16  Message)
{
}

INT8 COsdCtrlGet_B_CUTOFF_Offset_Warm_BCOW()
{
	return 0;
}

void COsdCtrlSet_B_CUTOFF_Offset_Warm_BCOW(UINT16  Message)
{
}



UINT8 fwif_vip_Get_Input_DataArrayIndex(UINT8 display)
{
	UINT8 DataArrayIndex = _SOURCE_INDEX_TV05_VGA;
	/*0: VGA 1:TV(NTSC);2:TV(PAL);3:TV(SECAM);4:AV&SV(NTSC);5:AV&SV(PAL);6:AV&SV(SECAM);7:YPbPr SD;8:YPbPr HD;9:HDMI SD;10:HDMI HD;*/
	/**/

	if (m_nowSource == VIP_QUALITY_VGA) {
		DataArrayIndex = _SOURCE_INDEX_TV05_VGA;
	}
	if (m_nowSource == VIP_QUALITY_TV_NTSC) {
		DataArrayIndex = _SOURCE_INDEX_TV_NTSC;
	} else if (m_nowSource == VIP_QUALITY_TV_PAL) {
		DataArrayIndex = _SOURCE_INDEX_TV_PAL;
	} else if (m_nowSource == VIP_QUALITY_TV_SECAN) {
		DataArrayIndex = _SOURCE_INDEX_TV_SECAM;
	} else if ((m_nowSource == VIP_QUALITY_CVBS_NTSC) || (m_nowSource == VIP_QUALITY_CVBS_S_NTSC)) {
		DataArrayIndex = _SOURCE_INDEX_AVSV_NTSC;
	} else if ((m_nowSource == VIP_QUALITY_CVBS_PAL) || (m_nowSource == VIP_QUALITY_CVBS_S_PAL)) {
		DataArrayIndex = _SOURCE_INDEX_AVSV_PAL;
	} else if ((m_nowSource == VIP_QUALITY_CVBS_SECAN) || (m_nowSource == VIP_QUALITY_CVBS_S_SECAN)) {
		DataArrayIndex = _SOURCE_INDEX_AVSV_SECAM;
	} else if ((m_nowSource >= VIP_QUALITY_YPbPr_480I) && (m_nowSource <= VIP_QUALITY_YPbPr_576P)) {
		DataArrayIndex = _SOURCE_INDEX_YPBPR_SD;
	} else if ((m_nowSource >= VIP_QUALITY_YPbPr_720P) && (m_nowSource <= VIP_QUALITY_YPbPr_1080P)) {
		DataArrayIndex = _SOURCE_INDEX_YPBPR_HD;
	} else if ((m_nowSource >= VIP_QUALITY_HDMI_480I) && (m_nowSource <= VIP_QUALITY_HDMI_576P)) {
		DataArrayIndex = _SOURCE_INDEX_HDMI_SD;
	} else if ((m_nowSource >= VIP_QUALITY_HDMI_720P) && (m_nowSource <= VIP_QUALITY_HDMI_1080P)) {
		DataArrayIndex = _SOURCE_INDEX_HDMI_HD;
	}


	return DataArrayIndex;
}


/*------------------------------------------------------------------------------
  *function:	fwif_color_3DPQ_enhance (use Gamma S curve)
 *
  *Parm:		 = [0(off)  1(on)] (default value is off)
  *Return: 	void
 *-----------------------------------------------------------------------------*/
void fwif_color_3DPQ_enhance(unsigned char enable)
{
	StructColorDataType *pTable = NULL;
	unsigned char s_index = Scaler_GetSindex();
	unsigned char s_high = Scaler_GetShigh();
	unsigned char s_low = Scaler_GetSlow();
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);


	VIPprintf("\n ====== fwif_color_3DPQ_enhance ==== enable = %d =====\n", enable);
	if (enable) {
		s_index  -= 5;
		s_high += 5;
		s_low += 5;
	}

	/*FIX ME, no enhance*/
	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	fwif_set_gamma(pTable->Gamma, s_index, s_high, s_low);
}




UINT8 Android_mode_flag = 1;		/*0: OFF, TV mode, 1: ON, Android mode*/
UINT8 Scaler_Get_Android_mode(void)
{	/*0: OFF, TV mode, 1: ON, Android mode*/
	return Android_mode_flag;
}

static int g_iBacklightLevelAndroid = -1;
void Scaler_Set_Android_mode(UINT8 mode)
{	/*0: OFF, TV mode, 1: ON, Android mode*/
	rtd_printk(KERN_DEBUG, TAG_NAME, "%s:mode = %d\n", __FUNCTION__, mode);
	Android_mode_flag = mode;

	#if 0	/*mark for compiler, only for TV005 process*/
	if (mode) {
		drvif_color_set_output_gamma(0, 1);
		if (g_iBacklightLevelAndroid != -1) {
			Panel_SetBackLightLevel(BACKLIGHT_USER, g_iBacklightLevelAndroid);
		}
	}
	#endif
}
UINT8 media_player_USB_flag;
UINT8 Scaler_Get_Media_Player_USB_flag(void)
{	/*0: Youtube, 1:USB source, 2: Mira Cast*/
	return media_player_USB_flag;
}
void Scaler_Set_Media_Player_USB_flag(UINT8 flag)
{	/*0: Youtube, 1:USB source, 2: Mira Cast*/
	media_player_USB_flag = flag;
}

void Scaler_SaveAndroidBacklight(int value)
{
    g_iBacklightLevelAndroid = value;
}


/*============================================================================= */ /*DCC CSFC 20140128*/
/*==== NEW_DCC_Flow =====*/

unsigned char Scaler_Get_DCC_Color_Independent_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_Color_Independent_table_select;
}

void Scaler_Set_DCC_Color_Independent_Table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_Color_Independent_table_select = value;

	VIPprintf("DCC_Color_Independent_select = %d\n", value);
	fwif_color_set_dcc_Color_Independent_driver(src_idx, value);
}

unsigned char Scaler_Get_DCC_chroma_compensation_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 255;
	}
	return pTable->DCC_chroma_compensation_table_select;
}

void Scaler_Set_DCC_chroma_compensation_Table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_chroma_compensation_table_select = value;

	VIPprintf("DCC_chroma_compensation_table_select = %d\n", value);
	fwif_color_set_dcc_chroma_compensation_driver(src_idx, value);
}

/*o--- source table----o*/
unsigned char Scaler_GetDCC_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_table_select;
}

void Scaler_SetDCC_Table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_table_select = value;

	fwif_color_set_dcc_table_select(src_idx, value);
}

/*o--- picture mode----o*/
unsigned char Scaler_GetDCC_Mode(void)/* off, low, middle, high, default*/
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	VIPprintf("!!!!!!!!pCurPicModeData[mode].DccMode = %d\n", pData->DccMode);
	return pData->DccMode;
}

/* off, low, middle, high, default*/
unsigned char  Scaler_SetDCC_Mode(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}
	pTable->DccMode = value;

	fwif_color_set_dcc_mode(src_idx, value);
	if (value < SLR_DCC_MODE_MAX) {
		Scaler_set_dcc_Sync_DCC_AP_Level(src_idx, Scaler_GetDCC_Table(), value);
		return TRUE;
	} else {
		rtd_printk(KERN_DEBUG, TAG_NAME, "Warning!! FreshContrast Range 0~3");
		return false;
	}
}

/*o--- table cync. to video ---oo*/
unsigned char Scaler_Get_DCC_Boundary_Check_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_Boundary_Check_Table;
}

void Scaler_Set_DCC_Boundary_Check_Table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_Boundary_Check_Table = value;

	fwif_color_set_DCC_Boundary_Check_Table(src_idx, value);
}

unsigned char Scaler_Get_DCC_Level_and_Blend_Coef_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_Level_and_Blend_Coef_Table;
}

void Scaler_Set_DCC_Level_and_Blend_Coef_Table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_Level_and_Blend_Coef_Table = value;

	fwif_color_set_DCC_Level_and_Blend_Coef_Table(src_idx, value);
}


unsigned char Scaler_Get_DCC_database_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_database_Table;
}

void Scaler_Set_DCC_database_Table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_database_Table = value;

	fwif_color_set_DCC_database_Table(src_idx, value);
}

unsigned char Scaler_Get_DCC_UserCurve_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_UserCurve_Table;
}

void Scaler_Set_DCC_UserCurve_Table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_UserCurve_Table = value;

	fwif_color_set_DCC_User_Curve_Table(src_idx, value);
}

unsigned char Scaler_Get_DCC_AdaptCtrl_Level_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_adaptive_Level_Table;
}

void Scaler_Set_DCC_AdaptCtrl_Level_Table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_adaptive_Level_Table = value;

	fwif_color_set_DCC_AdaptCtrl_Level_Table(src_idx, value);
}

unsigned char Scaler_Get_DCC_hist_adjust_table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_hist_adjust_table;
}

void Scaler_Set_DCC_hist_adjust_table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_hist_adjust_table = value;

	Scaler_set_dcc_sync_histogram_adjust_table(src_idx, value); /* for AP eng. menu sync.*/

	fwif_color_set_DCC_hist_adjust_table(src_idx, value);
}

/*o---sync. level and coef base  to video ---o*/
unsigned char Scaler_GetDCL_W(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCL_W;
}

void Scaler_SetDCL_W(int value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCL_W = value;

	fwif_color_set_DCL_W(src_idx, value);
}

unsigned char Scaler_GetDCL_B(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCL_B;
}

void Scaler_SetDCL_B(int value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCL_B = value;

	fwif_color_set_DCL_B(src_idx, value);
}

unsigned char Scaler_GetDCC_W_extend(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_W_expand;
}

void Scaler_SetDCC_W_extend(int value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_W_expand = value;

	fwif_color_set_DCC_W_expand(src_idx, value);
}

unsigned char Scaler_GetDCC_B_extend(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_B_expand;
}

void Scaler_SetDCC_B_extend(int value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	pTable->DCC_B_expand = value;

	fwif_color_set_DCC_B_expand(src_idx, value);
}

unsigned char Scaler_Get_DCC_DelayTime(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_Blending_DelayTime;
}

void Scaler_Set_DCC_DelayTime(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_Blending_DelayTime = value;

	fwif_color_set_DCC_Blending_DelayTime(src_idx, value);
}

unsigned char Scaler_Get_DCC_Step(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_Blending_Step;
}

void Scaler_Set_DCC_Step(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_Blending_Step = value;

	fwif_color_set_DCC_Blending_Step(src_idx, value);
}

void Scaler_Set_DCC_Picture_Mode_Weight(int value)
{
	unsigned char src_idx = GET_USER_INPUT_SRC();
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	pTable->DCC_Picture_Mode_Weight = value;

	fwif_color_set_DCC_Picture_Mode_Weight(src_idx, value);
}

unsigned char Scaler_Get_DCC_Picture_Mode_Weight(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return pTable->DCC_Picture_Mode_Weight;
}

/*o--- sync. table coef. that is non-adaptive table base API to video ---o*/
void Scaler_Set_Dcc_histogram_gain(unsigned char bin_num, unsigned char value, unsigned char SceneChange)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	/*int ret;*/
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	pTable->DCC_Histogram_gain[bin_num] = value;

	fwif_color_set_DCC_histogram_gain(src_idx, value, bin_num);
}

unsigned char Scaler_Get_Dcc_histogram_gain(unsigned char bin_num)
{
	StructColorDataType *pTable = NULL;
	unsigned char Dcc_histogram_gain = 8;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	Dcc_histogram_gain = pTable->DCC_Histogram_gain[bin_num];

	VIPprintf("## Dcc_histogram_gain = %d ##\n", Dcc_histogram_gain);

	return Dcc_histogram_gain;
}

void Scaler_Set_Dcc_histogram_offset(unsigned char bin_num, unsigned char value, unsigned char SceneChange)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	/*int ret;*/
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
	rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	pTable->DCC_Histogram_offset[bin_num] = value;

	fwif_color_set_DCC_histogram_offset(src_idx, value, bin_num);
}

unsigned char Scaler_Get_Dcc_histogram_offset(unsigned char bin_num)
{
	StructColorDataType *pTable = NULL;
	unsigned char Dcc_histogram_offset = 8;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	Dcc_histogram_offset = pTable->DCC_Histogram_offset[bin_num];

	VIPprintf("## Dcc_histogram_offset = %d ##\n", Dcc_histogram_offset);

	return Dcc_histogram_offset;
}

void Scaler_Set_Dcc_histogram_limit(unsigned char bin_num, unsigned char value, unsigned char SceneChange)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	/*int ret;*/
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	pTable->DCC_Histogram_limit[bin_num] = value;

	fwif_color_set_DCC_histogram_limit(src_idx, value, bin_num);
}

unsigned char Scaler_Get_Dcc_histogram_limit(unsigned char bin_num)
{
	StructColorDataType *pTable = NULL;
	unsigned char Dcc_histogram_limit = 8;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	Dcc_histogram_limit = pTable->DCC_Histogram_limit[bin_num];

	VIPprintf("## Dcc_histogram_limit = %d ##\n", Dcc_histogram_limit);

	return Dcc_histogram_limit;
}

/*o--- non-adaptive table coef sync. when table change---o*/
void Scaler_set_dcc_sync_histogram_adjust_table(unsigned char src_idx , unsigned char Tabel_sel) /* for AP eng. menu sync.*/
{
	int i = 0;
	VIP_DCC_Hist_Factor_Table *DCC_Hist_Factor_Table = NULL;
	SLR_VIP_TABLE *prt = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);

	if (prt == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get share mem vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	DCC_Hist_Factor_Table = &(prt->DCC_Control_Structure.DCCHist_Factor_Table[pTable->DCC_hist_adjust_table]);

	for (i = 0; i < 32; i++) {
		pTable->DCC_Histogram_gain[i]	=	DCC_Hist_Factor_Table->gain[i];
		pTable->DCC_Histogram_offset[i]	=	DCC_Hist_Factor_Table->offset[i];
		pTable->DCC_Histogram_limit[i]	=	DCC_Hist_Factor_Table->limit[i];
	}

}

void Scaler_set_dcc_Sync_DCC_AP_Level(unsigned char src_idx, unsigned char Tabel_sel, unsigned char Mode_sel)
{
	VIP_DCC_Curve_Control_Coef *DCC_Curve_Control_Coef = NULL;
	//VIP_DCC_Level_and_Blend_Coef_Table	*Level_and_Blend_Coef_Table = NULL;

	/*o----get table from share memory---o*/
	VIP_DCC_Curve_Control_Coef *pItemAdjust = NULL;
	StructColorDataType *pTable = NULL;
	VIP_DCC_Level_and_Blend_Coef_Table *pItemAdjust_1 = NULL;
	SLR_VIP_TABLE *prt = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	if (prt == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get share mem vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	DCC_Curve_Control_Coef = &(prt->DCC_Control_Structure.DCC_Curve_Control_Coef[0][0]);
	//Level_and_Blend_Coef_Table = &(prt->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[0]);

	/*o---get by table and mode form share memory---o*/
	if (Mode_sel > SLR_DCC_Default)
		Mode_sel = SLR_DCC_Default;
	pItemAdjust = &DCC_Curve_Control_Coef[Tabel_sel*DCC_SELECT_MAX+Mode_sel];

	/*set table to color data*/
	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	/* defined by VIP table, and value fix by src*/
	pTable->DCC_Boundary_Check_Table = pItemAdjust->Boundary_Check_Table;
	pTable->DCC_Level_and_Blend_Coef_Table	=	pItemAdjust->Level_and_Blend_Coef_Table;
	pTable->DCC_hist_adjust_table = pItemAdjust->hist_adjust_table;
	pTable->DCC_adaptive_Level_Table = pItemAdjust->AdaptCtrl_Level_Table;
	pTable->DCC_UserCurve_Table = pItemAdjust->User_Curve_Table;
	pTable->DCC_database_Table = pItemAdjust->database_DCC_Table;
	pTable->DCC_Picture_Mode_Weight = pItemAdjust->Picture_Mode_Weight;

	pItemAdjust_1 = &(prt->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[(pTable->DCC_Level_and_Blend_Coef_Table)]);

	pTable->DCL_W = pItemAdjust_1->DCL_W_level;
	pTable->DCL_B = pItemAdjust_1->DCL_B_level;
	/*pTable->DCC_W_expand = pItemAdjust_1->Expand_W_Level;	*/ /*=== marked by Elsie ===*/
	/*pTable->DCC_B_expand = pItemAdjust_1->Expand_B_Level;	*/ /*=== marked by Elsie ===*/
	pTable->DCC_Blending_Step = pItemAdjust_1->Step;
	pTable->DCC_Blending_DelayTime = pItemAdjust_1->Delay_time;

	Scaler_set_dcc_Sync_DCC_Video_Driver_Level(src_idx);

}

void Scaler_set_dcc_Sync_DCC_Video_Driver_Level(unsigned char src_idx)/*Note : must be using when DCC global coef. setting ready!!!, CSFC_20140212*/
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Warning!! Get colordata NULL., %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*o--- adaptive table sync --o*/
	fwif_color_set_DCC_Boundary_Check_Table(src_idx, pTable->DCC_Boundary_Check_Table);
	fwif_color_set_DCC_Level_and_Blend_Coef_Table(src_idx, pTable->DCC_Level_and_Blend_Coef_Table);
	fwif_color_set_DCC_hist_adjust_table(src_idx, pTable->DCC_hist_adjust_table);
	fwif_color_set_DCC_AdaptCtrl_Level_Table(src_idx, pTable->DCC_adaptive_Level_Table);
	fwif_color_set_DCC_User_Curve_Table(src_idx, pTable->DCC_UserCurve_Table);
	fwif_color_set_DCC_database_Table(src_idx, pTable->DCC_database_Table);

	/*o--no adaptive level sync --o*/
	fwif_color_set_DCL_W(src_idx, pTable->DCL_W);
	fwif_color_set_DCL_B(src_idx, pTable->DCL_B);
	fwif_color_set_DCC_W_expand(src_idx, pTable->DCC_W_expand);
	fwif_color_set_DCC_B_expand(src_idx, pTable->DCC_B_expand);
	fwif_color_set_DCC_Blending_Step(src_idx, pTable->DCC_Blending_Step);
	fwif_color_set_DCC_Blending_DelayTime(src_idx, pTable->DCC_Blending_DelayTime);
	fwif_color_set_DCC_Picture_Mode_Weight(src_idx, pTable->DCC_Picture_Mode_Weight);

}


unsigned char Scaler_SetDCC_HAL_FreshContrast(unsigned char display , unsigned char uDCMode, UINT32 pstData)
{
	return Scaler_SetDCC_Mode(uDCMode);
}


/*====== About  DCC Function end ==========*/
/*============================================================================= */ /*DCC CSFC 20140128*/



/*============================================================================= */ /*ConBri CSFC 20140128*/
/*             Contrast/brightness*/
/*============================*/


#ifdef OSD_FOUR_LEVEL_MAPPING_ENABLE
unsigned char Scaler_GetFacBrightness(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Brightness_0;
	*retval_50 = pTable->Brightness_50;
	*retval_100 = pTable->Brightness_100;
	*retval_25 = pTable->Brightness_25;
	*retval_75 = pTable->Brightness_75;
	return 0;
}

unsigned char Scaler_SetFacBrightness(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Brightness_0 = val_0;
	pTable->Brightness_50 = val_50;
	pTable->Brightness_100 = val_100;
	pTable->Brightness_25 = val_25;
	pTable->Brightness_75 = val_75;
	return 0;
}

unsigned char Scaler_GetFacContrast(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Contrast_0;
	*retval_50 = pTable->Contrast_50;
	*retval_100 = pTable->Contrast_100;
	*retval_25 = pTable->Contrast_25;
	*retval_75 = pTable->Contrast_75;
	return 0;
}
unsigned char Scaler_SetFacContrast(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Contrast_0 = val_0;
	pTable->Contrast_50 = val_50;
	pTable->Contrast_100 = val_100;
	pTable->Contrast_25 = val_25;
	pTable->Contrast_75 = val_75;
	return 0;
}

#else
unsigned char Scaler_GetFacBrightness(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Brightness_0;
	*retval_50 = pTable->Brightness_50;
	*retval_100 = pTable->Brightness_100;
	return 0;
}

unsigned char Scaler_SetFacBrightness(unsigned char val_0, unsigned char val_50, unsigned char val_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Brightness_0 = val_0;
	pTable->Brightness_50 = val_50;
	pTable->Brightness_100 = val_100;
	return 0;
}

unsigned char Scaler_GetFacContrast(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Contrast_0;
	*retval_50 = pTable->Contrast_50;
	*retval_100 = pTable->Contrast_100;
	return 0;
}

unsigned char Scaler_SetFacContrast(unsigned char val_0, unsigned char val_50, unsigned char val_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Contrast_0 = val_0;
	pTable->Contrast_50 = val_50;
	pTable->Contrast_100 = val_100;
	return 0;
}

#endif

unsigned char Scaler_OsdMapToRegValue_Contrast(unsigned char display , unsigned char value)
{
	#if  defined(TV020_STYLE_PQ) || defined(BUILD_TV030_2)
		return fw_OsdMapToRegValue(display, FAC_CONTRAST, value);
	#else
		return 0;
	#endif
}
unsigned char Scaler_OsdMapToRegValue_Brightness(unsigned char display , unsigned char value)
{
	#if  defined(TV020_STYLE_PQ) || defined(BUILD_TV030_2)
		return fw_OsdMapToRegValue(display, FAC_BRIGHTNESS, value);
	#else
		return 0;
	#endif
}

#if 0/* Gamma flow fix*/
void Scaler_set_SCurve_driver(unsigned char item_idx, unsigned short value)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);


	if ((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == SLR_MAIN_DISPLAY) {
		/*WaitFor_DEN_STOP();*/
		switch (item_idx) {
		case gamma_mode:
			/*Scaler_SetGamma(value);*/
		break;

		case S_gamma_index:
			/*Scaler_SetSindex(value);*/
		break;

		case S_gamma_high:
			/*Scaler_SetShigh(value);*/
		break;

		case S_gamma_low:
			/*Scaler_SetSlow(value);*/
		break;

		default:
			/*drvif_color_S_Curve(_MAIN_DISPLAY, Scaler_GetGamma(), Scaler_GetSindex(), Scaler_GetSlow(),  Scaler_GetShigh());*/
		break;
		}
		fwif_set_gamma(src_idx);
	}
#ifdef CONFIG_DUAL_CHANNEL
	if ((Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL)) == SLR_SUB_DISPLAY) {
		/*WaitFor_DEN_STOP();*/
		switch (item_idx) {
		case gamma_mode:
			/*Scaler_SetGamma(value);*/
		break;

		case S_gamma_index:
			/*Scaler_SetSindex(value);*/
		break;

		case S_gamma_high:
			/*Scaler_SetShigh(value);*/
		break;

		case S_gamma_low:
			/*Scaler_SetSlow(value);*/
		break;

		default:
			/*drvif_color_S_Curve(SLR_SUB_DISPLAY, Scaler_GetGamma(), Scaler_GetSindex(), Scaler_GetSlow(),  Scaler_GetShigh());*/
		break;
		/*FIX ME, NO have sub option*/
		fwif_set_gamma(src_idx);
		}

	}
#endif

}
#endif
unsigned short Scaler_get_color_temp_r_type(unsigned char src_idx)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return 0;
	return pTable->R_val;
}

void Scaler_set_color_temp_r_type(unsigned char src_idx, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return;
	pTable->R_val = value;
}

unsigned short Scaler_get_color_temp_g_type(unsigned char src_idx)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return 0;
	return pTable->G_val;
}

void Scaler_set_color_temp_g_type(unsigned char src_idx, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return;
	pTable->G_val = value;
}

unsigned short Scaler_get_color_temp_b_type(unsigned char src_idx)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return 0;
	return pTable->B_val;
}

void Scaler_set_color_temp_b_type(unsigned char src_idx, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return;
	pTable->B_val = value;
}

unsigned short Scaler_get_color_temp_r_offset(unsigned char src_idx)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return 0;
	return pTable->R_offset_val;
}

void Scaler_set_color_temp_r_offset(unsigned char src_idx, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return;
	pTable->R_offset_val = value;
}

unsigned short Scaler_get_color_temp_g_offset(unsigned char src_idx)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return 0;
	return pTable->G_offset_val;
}

void Scaler_set_color_temp_g_offset(unsigned char src_idx, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return;
	pTable->G_offset_val = value;

}

unsigned short Scaler_get_color_temp_b_offset(unsigned char src_idx)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return 0;
	return pTable->B_offset_val;
}

void Scaler_set_color_temp_b_offset(unsigned char src_idx, unsigned short value)
{
	SLR_COLORTEMP_DATA *pTable = scaler_get_color_tempset(src_idx, 0);
	if (NULL == pTable)
		return;
	pTable->B_offset_val = value;
}



/*==== color temp*/
/*=============================================================================  */ /*ColorTemp CSFC 20140128*/


/*================================================================================= */ /*sharpness CSFC 20140128*/

unsigned int picmode_init_value[PICMODE_INIT_MAX];
void Scaler_set_picmode_init_value(unsigned char item_index, unsigned short value)
{
	if (item_index >= PICMODE_INIT_MAX)
		return;
	picmode_init_value[item_index] = value;
}

int Scaler_set_picmode_init_sendRPC(void)
{
	int ret = 1;
	int i = 0;

	unsigned int *pwTemp = (unsigned int *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_VIP_PICMODE_INIT);
	if (pwTemp) {
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned int *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_VIP_PICMODE_INIT),
			picmode_init_value, sizeof(unsigned int)*PICMODE_INIT_MAX);
#else
		memcpy((unsigned int *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_VIP_PICMODE_INIT),
			picmode_init_value, sizeof(unsigned int)*PICMODE_INIT_MAX);
#endif

		for (i = 0; i < PICMODE_INIT_MAX; i++) {
			pwTemp[i] = fwif_color_ChangeOneUINT32Endian(pwTemp[i], 0);
			rtd_printk(KERN_DEBUG, TAG_NAME, "Scaler_set_picmode_init_sendRPC[%d] = %x\n", i, pwTemp[i]);
		}

			ret = Scaler_SendRPC(SCALERIOC_SET_VIP_PICMODE_INIT, 0, 0);
		}
		return ret;
	}






void Scaler_SetTableNRSpatial(unsigned char value)
{
	int ret;

	/* copy to RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_TABLENRSPATIAL), &value, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_TABLENRSPATIAL), &value, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SET_TABLENRSPATIAL, 0, 0);
	if (0 != ret) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, set TableNRSpatial to driver fail !!!\n", ret);
		return;
	}
}

unsigned char Scaler_GetTableNRSpatial(void)
{
	int ret;
	unsigned char value = 0;

	ret = Scaler_SendRPC(SCALERIOC_GET_TABLENRSPATIAL, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, get TableNRSpatial from driver fail !!!\n", ret);
		return 0;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&value, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_TABLENRSPATIAL), sizeof(unsigned char));
#else
	memcpy(&value, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_TABLENRSPATIAL), sizeof(unsigned char));
#endif



	return value;
}

/* move "DNR table select info" to system_info_struct and replace by PQA table, elieli*/
#if 0
void Scaler_SetDNR_table(unsigned char value, unsigned char calledByOSD)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->PQA_Table = value;
	fwif_color_set_DNR_table(value, calledByOSD);
/*	fwif_color_set_DNR_table(GET_USER_INPUT_SRC(), value);*/
}
#endif

unsigned char  Scaler_GetDNR_table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->PQA_Table;
}

void Scaler_SetDNR(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	rtd_printk(KERN_DEBUG, TAG_NAME, "==== Scaler_SetDNR, %d\n", value);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;
	pData->DNR = value;

	rtd_printk(KERN_DEBUG, TAG_NAME, "===== Scaler_SetDNR, %d\n", value);

	fwif_color_SetDNR(display, value);
}

unsigned char Scaler_GetDNR(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;
	return pData->DNR;
}

void Scaler_SetMPEGNR(unsigned char value, unsigned char calledByOSD)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;
	pData->MPEG = value;

	fwif_color_SetMPEGNR(display, value, calledByOSD);
}

unsigned char Scaler_GetMPEGNR(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;
	return pData->MPEG;
}

DRV_RTNR_General_ctrl *Scaler_GetRTNR_General(void)
{
	return fwif_color_GetRTNR_General(Scaler_GetDNR_table(), Scaler_GetDNR());
}

void Scaler_SetRTNR_Y(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	/*unsigned char display = 0;*/

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->rtnr_y = value;

}

unsigned char Scaler_GetRTNR_Y(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->rtnr_y;
}

void Scaler_SetRTNR_C(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	/*unsigned char display = 0;*/

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->rtnr_c = value;

}

unsigned char Scaler_GetRTNR_C(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->rtnr_c;
}

void Scaler_SetNRSpatialY(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	/*unsigned char display = 0;*/

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->NRSpatialY = value;

}

unsigned char Scaler_GetNRSpatialY(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->NRSpatialY;
}

void Scaler_SetNRSpatialC(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	/*unsigned char display = 0;*/

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->NRSpatialC = value;

}

unsigned char Scaler_GetNRSpatialC(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->NRSpatialC;
}

void Scaler_SetMpegNR_H(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	/*unsigned char display = 0;*/

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MPEGNR_H = value;

}

unsigned char Scaler_GetMpegNR_H(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MPEGNR_H;
}

void Scaler_SetMpegNR_V(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	/*unsigned char display = 0;*/

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MPEGNR_V = value;

}

unsigned char Scaler_GetMpegNR_V(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MPEGNR_V;
}

void Scaler_SetNRTemporal(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->NRTemporal = value;
	fwif_color_Set_NRtemporal(display, value);
}

unsigned char Scaler_GetNRTemporal(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->NRTemporal;
}


#if defined (PQ_005_STYLE)
void Scaler_SetAutoNR(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;
	pData->Auto_NR = value;
}

unsigned char Scaler_GetAutoNR(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;
	return pData->Auto_NR;
}
#endif


unsigned short Scaler_GetICMTable()
{
/*rtd_printk(KERN_DEBUG, TAG_NAME, "@@@@@@@@=== gene 1.Scaler_GetICMTable ==\n");*/
	return Scaler_get_ICM_table();
}
void Scaler_SetICMTable(unsigned char value)
{
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "@@@@@@@@=== gene 1.Scaler_SetICMTable = %d ==\n", value);*/
	Scaler_set_ICM_table(value, 0);
}

void scaler_set_PQA_Input_table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);
	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->PQA_I_Table = value;
	fwif_color_set_PQA_Input_table(value);
}
/* Erin add for Atlantic , 100402*/
void scaler_set_PQA_table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);
	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->PQA_Table = value;

	fwif_color_set_PQA_table(value);
}

unsigned char scaler_get_PQA_Input_table(void)
{
	return fwif_color_get_PQA_Input_table();
}

unsigned char scaler_get_PQA_table(void)
{
	return fwif_color_get_PQA_table();
}

/*add for haier 20111210 by lichun zhao*/
#ifdef TV013UI_1

static unsigned char _2DColorManagemen = TRUE;
static unsigned char _3DColorManagemen[SOURCE_MAX_NUM] = {TRUE};

unsigned char Scaler_Get2DColorManagement()
{
    return _2DColorManagemen;
}


void Scaler_Set2DColorManagement(unsigned char mode)
{
    /* fwif_set_color_management(mode);*/
	 _2DColorManagemen = mode;
}

unsigned char Scaler_Get3DColorManagement(unsigned char input_src_index)
{
 /*   unsigned short input_src_index;*/
 /*   input_src_index = Scaler_InputSrcIndex(Scaler_DispGetInputInfo(SLR_INPUT_INPUT_SRC));*/
rtd_printk(KERN_DEBUG, TAG_NAME, "\n $$$$$$$$$$$$ ScalerLib Scaler_Get3DColorManagement input_src_index = %x \n", input_src_index);

    return _3DColorManagemen[input_src_index];
}

void Scaler_Set3DColorManagement(unsigned char input_src_index, unsigned char mode)
{
    /*unsigned short input_src_index;*/
	/*input_src_index = Scaler_InputSrcIndex(Scaler_DispGetInputInfo(SLR_INPUT_INPUT_SRC));*/
   /* fwif_set_color_management(mode);*/
   /*rtd_printk(KERN_DEBUG, TAG_NAME, "\n $$$$$$$$ input_src_index = %x \n", input_src_index);*/
	_3DColorManagemen[input_src_index] = mode;

}

#endif

/*o =========================================================== o*/
/*o =================== Enginner Menu Start =================== o*/
/*o =========================================================== o*/



unsigned char Scaler_GetScaleDOWNH(void)
{
	return fwif_color_get_scaledownh(GET_USER_INPUT_SRC());
}

/*range 0~17*/
void Scaler_SetScaleDOWNH(unsigned char value)
{
	fwif_color_set_scaledownh(GET_USER_INPUT_SRC(), value);
}

unsigned char Scaler_GetScaleDOWNV(void)
{
	return fwif_color_get_scaledownv(GET_USER_INPUT_SRC());
}

/*range 0~17*/
void Scaler_SetScaleDOWNV(unsigned char value)
{
	fwif_color_set_scaledownv(GET_USER_INPUT_SRC(), value);
}

unsigned short Scaler_GetH8Scaling()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->H8_Scaling;
}

void Scaler_SetH8Scaling(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->H8_Scaling = value;
}

unsigned char Scaler_GetPacificNewIntra(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;

	return pTable->IntraAllPacificNew;
}

void Scaler_SetPacificNewIntra(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->IntraAllPacificNew = value;

	fwif_color_set_intra_all_pacific_new_switch (GET_USER_INPUT_SRC(), value);
}

unsigned short Scaler_GetICcoring()
{
	return 0;
/*	return fwif_color_get_i_ccoring(GET_USER_INPUT_SRC());*/
}

void Scaler_SetICcoring(unsigned char message)
{
	/*drvif_color_chroma_coring(info->display, 0, message);*/
}

unsigned short Scaler_GetDCcoring()
{
	return 0;
}

void Scaler_SetDCcoring(unsigned char message)
{
	/*drvif_color_chroma_coring(info->display, 1, message);*/
}

unsigned char Scaler_GetVertical_NR(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Vertical_NR;
}

/*range 0~1, Lydia*/
void Scaler_SetVertical_NR(unsigned char value)
{
	/*int ret;*/
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Vertical_NR = value;

	if ((Scaler_SendRPC(SCALERIOC_GET_TABLE_VerticalNR, value, RPC_SEND_VALUE_ONLY)) < 0) {
		/*rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, update TableNR_VerticalNR to driver fail !!!\n", ret);*/
	}

	fwif_color_set_Vertical_NR_driver(src_idx, value);
}

unsigned char Scaler_GetHMCNR(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);
	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->HMCNR_range;

}

void Scaler_SetHMCNR(unsigned char value)
{
	/*int ret;*/
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->HMCNR_range = value;

	if ((Scaler_SendRPC(SCALERIOC_GET_TABLE_HMCNR, value, RPC_SEND_VALUE_ONLY)) < 0) {
		/*rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, update TableNR_HMCNR to driver fail !!!\n", ret);*/
	}

	 fwif_color_set_HMCNR_driver(src_idx, value);

}

unsigned char Scaler_GetHMCNR_blending(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->HMCNR_blending;

}
/*range 0~1, Lydia*/
void Scaler_SetHMCNR_blending(unsigned char value)
{
	/*int ret;*/
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->HMCNR_blending = value;

	fwif_color_set_HMCNR_blending_driver(src_idx, value);

}

unsigned char Scaler_GetMBPeaking(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MBPeaking;
}

/*range 0~40*/
void Scaler_SetMBPeaking(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MBPeaking = value;

	fwif_color_set_MBPK_table(src_idx, value);
}

unsigned char Scaler_GetMBSUPeaking(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MBSUPeaking;
}

/*range 0~40*/
void Scaler_SetMBSUPeaking(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MBSUPeaking = value;

	fwif_color_set_MBSU_table(src_idx, value);
}

unsigned short Scaler_GetSharpnessVerticalGain()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Shp_Vgain;
}

void Scaler_SetSharpnessVerticalGain(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Shp_Vgain = value;

	fwif_color_set_shp_vgain(GET_USER_INPUT_SRC(), value);
}

unsigned short  Scaler_Getharpness2Dpk_Seg()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Shp_2Dpk_Seg;
}

void Scaler_SetSharpness2Dpk_Seg(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Shp_2Dpk_Seg = value;

	fwif_color_set_shp_2Dpk_Seg(src_idx, value);
}

unsigned short  Scaler_GetSharpness2Dpk_Tex_Detect()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Shp_2Dpk_Tex_Detect;

}

void Scaler_SetSharpness2Dpk_Tex_Detect(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Shp_2Dpk_Tex_Detect = value;

	fwif_color_set_shp_2Dpk_Tex_Detect(src_idx, value);
}

unsigned char Scaler_GetMADI_BTR()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MA_BTR;

}

void Scaler_SetMADI_BTR(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MA_BTR = value;

	fwif_color_set_ma_BTR(src_idx, value);
}

unsigned char Scaler_GetMADI_BER()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MA_BER;
}

void Scaler_SetMADI_BER(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MA_BER = value;

	fwif_color_set_ma_BER(src_idx, value);
}

unsigned char Scaler_GetGridRemove()
{
	return fwif_color_get_grid_remove(GET_USER_INPUT_SRC());
}

void Scaler_SetGridRemove(unsigned char value)
{
	fwif_color_set_grid_remove(GET_USER_INPUT_SRC(), value);
}
/*
unsigned char Scaler_GetContiStillLine()
{
	return fwif_color_get_conti_still_line(GET_USER_INPUT_SRC());
}

void Scaler_SetContiStillLine(unsigned char value)
{
	fwif_color_set_conti_still_line(GET_USER_INPUT_SRC(), value);
}
*/

void Scaler_Set_TVAutoNR_Mode(unsigned char flag)
{
	int ret;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_TVAUTONR_Mode);
	if (!pwTemp) {
		return;
	}
	/* copy to RPC share memory*/
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "A. Scaler_Set_TVAutoNR_Mode = %x\n", flag);*/
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_TVAUTONR_Mode), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_TVAUTONR_Mode), &flag, sizeof(unsigned char));
#endif
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "B. Scaler_Set_TVAutoNR_Mode = %x\n", flag);*/
	ret = Scaler_SendRPC(SCALERIOC_SET_TVAUTONR_Mode, 0, 0);
	if (0 != ret) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, set Enter_TVAUTONR_Mode to driver fail !!!\n", ret);
		return;

	}
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "B. SCALERIOC_SET_TVAUTONR_Mode = %x\n", Scaler_GetShareMemVirAddr(SCALERIOC_SET_TVAUTONR_Mode));*/


}

unsigned char Scaler_GetAutoMA1Flag() /* enable/disable AutoMA*/
{
	return fwif_color_GetAutoMA_Struct()->AutoMA1_En;
}

void Scaler_SetAutoMA1Flag(unsigned char flag)
{
	fwif_color_GetAutoMA_Struct()->AutoMA1_En = flag;
	return;
}

unsigned char Scaler_GetAutoMA2Flag() /* enable/disable AutoMA*/
{
	return fwif_color_GetAutoMA_Struct()->AutoMA2_En;
}

void Scaler_SetAutoMA2Flag(unsigned char flag)
{
	fwif_color_GetAutoMA_Struct()->AutoMA2_En = flag;
	return;
}

unsigned char Scaler_GetAutoMA3Flag() /* enable/disable AutoMA*/
{
	return fwif_color_GetAutoMA_Struct()->AutoMA3_En;
}

void Scaler_SetAutoMA3Flag(unsigned char flag)
{
	fwif_color_GetAutoMA_Struct()->AutoMA3_En = flag;
	return;
}

unsigned char Scaler_GetAutoMA4Flag() /* enable/disable AutoMA*/
{
	return fwif_color_GetAutoMA_Struct()->AutoMA4_En;
}

void Scaler_SetAutoMA4Flag(unsigned char flag)
{
	fwif_color_GetAutoMA_Struct()->AutoMA4_En = flag;
	return;
}

unsigned char Scaler_GetAutoMA5Flag() /* enable/disable AutoMA*/
{
	return fwif_color_GetAutoMA_Struct()->AutoMA5_En;
}

void Scaler_SetAutoMA5Flag(unsigned char flag)
{
	fwif_color_GetAutoMA_Struct()->AutoMA5_En = flag;
	return;
}

unsigned char Scaler_GetAutoMA6Flag() /* enable/disable AutoMA*/
{
	return fwif_color_GetAutoMA_Struct()->AutoMA6_En;
}

void Scaler_SetAutoMA6Flag(unsigned char flag)
{
	fwif_color_GetAutoMA_Struct()->AutoMA6_En = flag;
	return;
}

unsigned char Scaler_GetAutoMAFlag(void) /* Is AutoMA running ? set for IRQ/IRC*/
{
	return fwif_color_GetAutoMAFlag();
}

void Scaler_SetAutoMAFlag(unsigned char flag) /* enable/disable AutoMA, set for IRQ/IRC*/
{
	/*if (Scaler_IsUnitTest())*/
	/*	return;*/

#ifdef MAGELLAN_BRING_UP_HACK_VIP
	return;
#endif

	fwif_color_SetAutoMAFlag(flag);
}

unsigned char Scaler_GetAutoMA_VD_3D() /* enable/disable AutoMA*/
{
	int ret;
	unsigned char flag = TRUE;

	/* enable this debug message after remove the debug warning first, pls.*/
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "Scaler_GetAutoMA4Flag = %d\n", (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETAUTOMAFLAG_4));*/
	ret = Scaler_SendRPC(SCALERIOC_GETAUTOMA_VD_3D, 0, 0);
	if (0 != ret) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, get Enter_AutoMA flag VD 3D from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETAUTOMA_VD_3D), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETAUTOMA_VD_3D), sizeof(unsigned char));
#endif


	return flag;
}

void Scaler_SetAutoMA_VD_3D(unsigned char flag)
{
	int ret;

#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETAUTOMA_VD_3D), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETAUTOMA_VD_3D), &flag, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SETAUTOMA_VD_3D, 0, 0);
	if (0 != ret) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, set Enter_AutoMA flag VD 3D to driver fail !!!\n", ret);
		return;

	}
}

void Scaler_Set_NR_style(unsigned char flag, unsigned char calledByOSD)
{

	VIPprintf("** *SCALERIOC_SET_NR_style = %d\n", flag);
	if (calledByOSD == CALLED_BY_OSD) {
		int ret;
		VIPprintf("*** SCALERIOC_SET_NR_style = %d\n", flag);
		ret = Scaler_SendRPC(SCALERIOC_SET_NR_style, flag, RPC_SEND_VALUE_ONLY);
		if (0 != ret) {
			SLRLIB_PRINTF("ret = %d, set SCALERIOC_SET_NR_style to driver fail !!!\n", ret);
			return;
		}
	} else
		Scaler_set_picmode_init_value(PICMODE_INIT_NR_STYLE, flag);
}

unsigned char Scaler_GetDNR_log()
{
	int ret;
	unsigned char flag = false;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETDNR_log);
	if (!pwTemp) {
		return flag;
	}
	ret = Scaler_SendRPC(SCALERIOC_GETDNR_log, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, get Enter_DNR_log from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETDNR_log), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETDNR_log), sizeof(unsigned char));
#endif

	return flag;
}

void Scaler_SetDNR_log(unsigned char flag)
{
	int ret;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETDNR_log);
	if (!pwTemp) {
		return;
	}
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETDNR_log), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETDNR_log), &flag, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SETDNR_log, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, set Enter_DNR_log to driver fail !!!\n", ret);
		return;

	}
}
unsigned char Scaler_GetRTNR_log()
{
	int ret;
	unsigned char flag = false;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETRTNR_log);
	if (!pwTemp) {
		return flag;
	}
	ret = Scaler_SendRPC(SCALERIOC_GETRTNR_log, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, get Enter_RTNR_log from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETRTNR_log), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETRTNR_log), sizeof(unsigned char));
#endif

	return flag;
}

void Scaler_SetRTNR_log(unsigned char flag)
{
	int ret;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETRTNR_log);
	if (!pwTemp) {
		return;
	}
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETRTNR_log), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETRTNR_log), &flag, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SETRTNR_log, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, set Enter_RTNR_log to driver fail !!!\n", ret);
		return;

	}
}

unsigned char Scaler_GetSNR_log()
{
	int ret;
	unsigned char flag = false;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETSNR_log);
	if (!pwTemp) {
		return flag;
	}
	ret = Scaler_SendRPC(SCALERIOC_GETSNR_log, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, get Enter_SNR_log from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETSNR_log), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETSNR_log), sizeof(unsigned char));
#endif

	return flag;
}

void Scaler_SetSNR_log(unsigned char flag)
{
	int ret;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETSNR_log);
	if (!pwTemp) {
		return;
	}
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETSNR_log), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETSNR_log), &flag, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SETSNR_log, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, set Enter_SNR_log to driver fail !!!\n", ret);
		return;
	}
}

unsigned char Scaler_GetINR_log()
{
	int ret;
	unsigned char flag = false;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETINR_log);
	if (!pwTemp) {
		return flag;
	}
	ret = Scaler_SendRPC(SCALERIOC_GETINR_log, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, get Enter_INR_log from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETINR_log), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GETINR_log), sizeof(unsigned char));
#endif

	return flag;
}

void Scaler_SetINR_log(unsigned char flag)
{
	int ret;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETINR_log);
	if (!pwTemp) {
		return;
	}
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETINR_log), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETINR_log), &flag, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SETINR_log, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, set Enter_INR_log to driver fail !!!\n", ret);
		return;
	}
}

unsigned char Scaler_Get_DEBUG_AUTO_NR()
{
	int ret;
	unsigned char flag = false;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_AUTO_NR);
	if (!pwTemp) {
		return flag;
	}
	ret = Scaler_SendRPC(SCALERIOC_GET_DEBUG_AUTO_NR, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, get Enter_INR_log from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_AUTO_NR), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_AUTO_NR), sizeof(unsigned char));
#endif

	return flag;
}

void Scaler_Set_DEBUG_AUTO_NR(unsigned char flag)
{
	int ret;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_AUTO_NR);
	if (!pwTemp) {
		return;
	}
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_AUTO_NR), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_AUTO_NR), &flag, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SET_DEBUG_AUTO_NR, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, set Enter_INR_log to driver fail !!!\n", ret);
		return;
	}
}

unsigned char Scaler_Get_DEBUG_VD_NOISE()
{
	int ret;
	unsigned char flag = false;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_VD_NOISE);
	if (!pwTemp) {
		return flag;
	}
	ret = Scaler_SendRPC(SCALERIOC_GET_DEBUG_VD_NOISE, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, get Enter_INR_log from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_VD_NOISE), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_VD_NOISE), sizeof(unsigned char));
#endif

	return flag;
}

void Scaler_Set_DEBUG_VD_NOISE(unsigned char flag)
{
	int ret;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_VD_NOISE);
	if (!pwTemp) {
		return;
	}
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_VD_NOISE), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_VD_NOISE), &flag, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SET_DEBUG_VD_NOISE, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, set Enter_INR_log to driver fail !!!\n", ret);
		return;
	}
}

unsigned char Scaler_Get_DEBUG_RTNR_MAD()
{
	int ret;
	unsigned char flag = false;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_RTNR_MAD);
	if (!pwTemp) {
		return flag;
	}
	ret = Scaler_SendRPC(SCALERIOC_GET_DEBUG_RTNR_MAD, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, get Enter_INR_log from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_RTNR_MAD), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_RTNR_MAD), sizeof(unsigned char));
#endif

	return flag;
}

void Scaler_Set_DEBUG_RTNR_MAD(unsigned char flag)
{
	int ret;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_RTNR_MAD);
	if (!pwTemp) {
		return;
	}
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_RTNR_MAD), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_RTNR_MAD), &flag, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SET_DEBUG_RTNR_MAD, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, set Enter_INR_log to driver fail !!!\n", ret);
		return;
	}
}

unsigned char Scaler_Get_DEBUG_HISTOGRAM_MEAN()
{
	int ret;
	unsigned char flag = false;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_HISTOGRAM_MEAN);
	if (!pwTemp) {
		return flag;
	}
	ret = Scaler_SendRPC(SCALERIOC_GET_DEBUG_HISTOGRAM_MEAN, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, get Enter_INR_log from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_HISTOGRAM_MEAN), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_DEBUG_HISTOGRAM_MEAN), sizeof(unsigned char));
#endif

	return flag;
}

void Scaler_Set_DEBUG_HISTOGRAM_MEAN(unsigned char flag)
{
	int ret;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_HISTOGRAM_MEAN);
	if (!pwTemp) {
		return;
	}
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_HISTOGRAM_MEAN), &flag, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_DEBUG_HISTOGRAM_MEAN), &flag, sizeof(unsigned char));
#endif
	ret = Scaler_SendRPC(SCALERIOC_SET_DEBUG_HISTOGRAM_MEAN, 0, 0);
	if (0 != ret) {
		SLRLIB_PRINTF("ret = %d, set Enter_INR_log to driver fail !!!\n", ret);
		return;
	}
}

unsigned char Scaler_GetVIP_Dbg_Msg(void)
{
#ifdef VIPprintf
	return ucIsPrintVIP;
#else
	return 0;
#endif
}


void Scaler_SetVIP_Dbg_Msg(unsigned char flag)
{
#ifdef VIPprintf
	ucIsPrintVIP = flag;
	SLRLIB_PRINTF("VIP Debug Print = %d\n", ucIsPrintVIP);
#endif
}

unsigned char Scaler_Get_TVAutoNR_Mode()
{
	int ret;
	unsigned char flag = 0;
	unsigned char *pwTemp = (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_TVAUTONR_Mode);
	if (!pwTemp) {
		return flag;
	}

	ret = Scaler_SendRPC(SCALERIOC_GET_TVAUTONR_Mode, 0, 0);
	if (0 != ret) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, get Enter_TVAUTONR_Mode from driver fail !!!\n", ret);
		return false;
	}
	/* copy from RPC share memory*/
#ifdef CONFIG_ARM64
	memcpy_fromio(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_TVAUTONR_Mode), sizeof(unsigned char));
#else
	memcpy(&flag, (unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_GET_TVAUTONR_Mode), sizeof(unsigned char));
#endif


	return flag;
}

INT8 COsdCtrlGet_DNR_log()
{
	return Scaler_GetDNR_log();
}
void COsdCtrlSet_DNR_log(UINT16  Message)
{
	Scaler_SetDNR_log(Message);
}
INT8 COsdCtrlGet_RTNR_log()
{
	return Scaler_GetRTNR_log();
}
void COsdCtrlSet_RTNR_log(UINT16  Message)
{
	Scaler_SetRTNR_log(Message);
}
INT8 COsdCtrlGet_SNR_log()
{
	return Scaler_GetSNR_log();
}
void COsdCtrlSet_SNR_log(UINT16  Message)
{
	Scaler_SetSNR_log(Message);
}
INT8 COsdCtrlGet_INR_log()
{
	return Scaler_GetINR_log();
}
void COsdCtrlSet_INR_log(UINT16  Message)
{
	Scaler_SetINR_log(Message);
}
INT8 COsdCtrlGet_MPEGNR_log()
{
	/*rock ==== ToDo Need the scaler to implement API*/
	return 0;
}
void COsdCtrlSet_MPEGNR_log(UINT16  Message)
{
	/*rock ==== ToDo Need the scaler to implement API*/
}

/*AUTO_NR_DEBUG*/
INT8 COsdCtrlGet_DEBUG_AUTO_NR()
{
	return Scaler_Get_DEBUG_AUTO_NR();
}
void COsdCtrlSet_DEBUG_AUTO_NR(UINT16  Message)
{
	Scaler_Set_DEBUG_AUTO_NR(Message);
}

INT8 COsdCtrlGet_DEBUG_VD_NOISE()
{
	return Scaler_Get_DEBUG_VD_NOISE();
}
void COsdCtrlSet_DEBUG_VD_NOISE(UINT16  Message)
{
	Scaler_Set_DEBUG_VD_NOISE(Message);
}

INT8 COsdCtrlGet_DEBUG_RTNR_MAD()
{
	return Scaler_Get_DEBUG_RTNR_MAD();
}
void COsdCtrlSet_DEBUG_RTNR_MAD(UINT16  Message)
{
	Scaler_Set_DEBUG_RTNR_MAD(Message);
}

INT8 COsdCtrlGet_DEBUG_HISTOGRAM_MEAN()
{
	return Scaler_Get_DEBUG_HISTOGRAM_MEAN();
}
void COsdCtrlSet_DEBUG_HISTOGRAM_MEAN(UINT16  Message)
{
	Scaler_Set_DEBUG_HISTOGRAM_MEAN(Message);
}
/*o =========================================================== o*/
/*o =================== Enginner Menu END ===================== o*/
/*o =========================================================== o*/



/*o =========================================================== o*/
/*o ==================== OSD MENU Start ======================= o*/
/*o =========================================================== o*/

#ifdef OSD_FOUR_LEVEL_MAPPING_ENABLE
unsigned char Scaler_GetFacSharpness(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Sharpness_0;
	*retval_50 = pTable->Sharpness_50;
	*retval_100 = pTable->Sharpness_100;
	*retval_25 = pTable->Sharpness_25;
	*retval_75 = pTable->Sharpness_75;
	return 0;
}

unsigned char Scaler_SetFacSharpness(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75)
{
	StructColorDataFacModeType *pTable =NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Sharpness_0 = val_0;
	pTable->Sharpness_50 = val_50;
	pTable->Sharpness_100 = val_100;
	pTable->Sharpness_25 = val_25;
	pTable->Sharpness_75 = val_75;
	return 0;
}

unsigned char Scaler_GetFacBacklight(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100,
	unsigned char *retval_25, unsigned char *retval_75)
{
	StructColorDataFacModeType *pTable =NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Backlight_0;
	*retval_50 = pTable->Backlight_50;
	*retval_100 = pTable->Backlight_100;
	*retval_25 = pTable->Backlight_25;
	*retval_75 = pTable->Backlight_75;
	return 0;
}

unsigned char Scaler_SetFacBacklight(unsigned char val_0, unsigned char val_50, unsigned char val_100,
	unsigned char val_25, unsigned char val_75)
{
	StructColorDataFacModeType *pTable=NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Backlight_0 = val_0;
	pTable->Backlight_50 = val_50;
	pTable->Backlight_100 = val_100;
	pTable->Backlight_25 = val_25;
	pTable->Backlight_75 = val_75;
	return 0;
}
#else
unsigned char Scaler_GetFacSharpness(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Sharpness_0;
	*retval_50 = pTable->Sharpness_50;
	*retval_100 = pTable->Sharpness_100;
	return 0;
}

unsigned char Scaler_SetFacSharpness(unsigned char val_0, unsigned char val_50, unsigned char val_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Sharpness_0 = val_0;
	pTable->Sharpness_50 = val_50;
	pTable->Sharpness_100 = val_100;
	return 0;
}

unsigned char Scaler_GetFacBacklight(unsigned char *retval_0, unsigned char *retval_50, unsigned char *retval_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	*retval_0 = pTable->Backlight_0;
	*retval_50 = pTable->Backlight_50;
	*retval_100 = pTable->Backlight_100;
	return 0;
}

unsigned char Scaler_SetFacBacklight(unsigned char val_0, unsigned char val_50, unsigned char val_100)
{
	StructColorDataFacModeType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_fac_mode(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Backlight_0 = val_0;
	pTable->Backlight_50 = val_50;
	pTable->Backlight_100 = val_100;
	return 0;
}
#endif


/*o =========================================================== o*/
/*o ==================== OSD MENU End ========================= o*/
/*o =========================================================== o*/



/*for Load Quality handle*/
void Scaler_Get_Display_info(unsigned char *display, unsigned char *src_idx)
{

	*display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	if (SLR_MAIN_DISPLAY == *display)
		*src_idx = GET_USER_INPUT_SRC();/*only get main display src*/
	else
		*src_idx = GET_UI_SOURCE_FROM_DISPLAY(*display);/*get display info by channel*/
}

void Scaler_VIP_Project_ID_init()
{
	/*=== 20140520 CSFC for vip video fw infomation ====*/
	/* setting some information to sharememory*/
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (NULL == VIP_RPC_system_info_structure_table)
		return;

	#if defined(IS_TV003_STYLE_PICTUREMODE)
		/*VIP_system_info_structure_table->Project_ID	= VIP_Project_ID_TV003;*/
		fwif_VIP_set_Project_ID(VIP_Project_ID_TV003);
	#elif defined(BUILD_TV010_2)
		/*VIP_system_info_structure_table->Project_ID	= VIP_Project_ID_TV010;*/
		fwif_VIP_set_Project_ID(VIP_Project_ID_TV010);
	#elif defined(BUILD_TV013_1)
		/*VIP_system_info_structure_table->Project_ID	= VIP_Project_ID_TV013;*/
		fwif_VIP_set_Project_ID(VIP_Project_ID_TV013);
	#elif defined(BUILD_TV035_1)
		/*VIP_system_info_structure_table->Project_ID	= VIP_Project_ID_TV035;*/
		fwif_VIP_set_Project_ID(VIP_Project_ID_TV035);
	#else  /* Realtek defualt used public C file*/
        #if IS_ENABLED(CONFIG_SUPPORT_SCALER)
		 /*for Driver Base OS_System Only, ex : TvService or Web_OS, CSFC*/
		if(vpq_project_id == 0x00060000 && vpqex_project_id == 0x00060000)
			fwif_VIP_set_Project_ID(VIP_Project_ID_TV006);
		else
			fwif_VIP_set_Project_ID(VIP_Project_ID_TV001);
		#else
			/*VIP_system_info_structure_table->Project_ID	= VIP_Project_ID_TV001;*/
			fwif_VIP_set_Project_ID(VIP_Project_ID_TV001);
		#endif
	#endif

}

void Scaler_VIP_MEMC_init(unsigned char Vip_source)
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
			return;
#else

	if ( Scaler_Get3DMode((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) == SLR_3DMODE_2D) {
		IoReg_SetBits(KPOST_TOP_KPOST_TOP_30_reg, _BIT15); /*memc hsc bypass*/
		IoReg_SetBits(KPOST_TOP_KPOST_TOP_30_reg, _BIT0);  /*memc vsc bypass*/
	}
#endif
}

void Scaler_VIP_IP_init(unsigned char Vip_source)
{
	unsigned char temptable[5] = {7, 1, 1, 1, 1};
	unsigned short temp_width =0, temp_height = 0;
	unsigned short temp_D_width =0, temp_D_height = 0;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	di_im_di_control_RBUS im_di_control_reg;
	di_di_smd_choosecandidate_RBUS 	smd_choosecandidate_reg;

	/*=== 20140520 CSFC for vip video fw infomation ====*/
	/* setting some information to sharememory*/
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	Scaler_Get_Display_info(&display, &src_idx);

	fwif_Save_PQSource((VIP_SOURCE_TIMING)Vip_source);	/*20140127 roger, set PQSource for scalercolor.cpp using*/
	fwif_Save_SWSource(Scaler_GetUserInputSrc(SLR_MAIN_DISPLAY), Scaler_GetUserInputSrc(SLR_SUB_DISPLAY));	/*20140206 roger, set SWSource for scalercolor.cpp using*/

	/* reset TNR status*/
	im_di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
	im_di_control_reg.cp_temporalenable = 0;
	im_di_control_reg.write_enable_6 = 1;
	IoReg_Write32(DI_IM_DI_CONTROL_reg, im_di_control_reg.regValue);

	drvif_color_rtnr_on((SCALER_DISP_CHANNEL)display); /* enable RTNR*/
	if (0x00060000 != vpq_project_id)
		fwif_color_set_DCC_Init(display);

	//disable ICM bypass function
	drvif_color_icm_set_bypass_onoff(display, 0);

	drvif_color_icm_hue_histo_init();
#if 0	//move to fw_scalerip_set_IEGSM() @ i-doamin setting, avoid garbage, D-domain is too late
	drvif_color_iEdgeSmooth_init();
#endif
	/* init histogram : need to init by input timing width and Len*/
	if (Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF)
		&& (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_SIDE_BY_SIDE_HALF) == TRUE))) {
		temp_width = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_WID) << 1;
	} else {
		temp_width = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_WID);
	}
	temp_height = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_MEM_ACT_LEN);
	fwif_color_hist_init((SCALER_DISP_CHANNEL)display , temp_width , temp_height, Mode_Orig_32Bin);

	//D-Histogram From Merlin2, 20160630
	temp_D_width = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_WID);;
	temp_D_height = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display,SLR_INPUT_DISP_LEN);;
	fwif_color_D_hist_init((SCALER_DISP_CHANNEL)display, temp_D_width, temp_D_height);

	fwif_color_temptable_handler(Vip_source, temptable);

       //=======   Dither setting  ==============
       Scaler_color_Dither_Table_Ctrl();
	//=========  Dither setting  ============

#if 0 // 20170524 jimmy, D-decontour removed in Merlin3
	/* set de-contour*/
	if (Scaler_InputSrcGetMainChType() == _SRC_VO && vsc_get_adaptivestream_flag() == 1)
		fwif_color_set_de_contour(0, 1);
	else
		fwif_color_set_de_contour(0, 0);
#endif

	if (0x00060000 != vpq_project_id)
		fwif_color_inv_gamma_init();

	/*DI Pan init*/
	if (Vip_source == VIP_QUALITY_TV_NTSC || Vip_source == VIP_QUALITY_TV_PAL) {
		drvif_color_ma_Pan_Detection(0);
	} else {
		drvif_color_ma_Pan_Detection(1);
	}

	if (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) >1920)
	{
		drvif_color_DRV_MCNR_TMV(2);

	}
	else if (Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID) > 720)
	{
		drvif_color_DRV_MCNR_TMV(1);
	}
	else
	{
		drvif_color_DRV_MCNR_TMV(0);
 	}


	fwif_color_set_MPEG_init();
#if 0	//move to fw_scalerip_set_IEGSM() @ i-doamin setting, avoid garbage, D-domain is too late
        drvif_IESM_frontporch_set();
#endif
	#ifdef ENABLE_RTK_DCR //DCR GDBC init
		VIP_system_info_structure_table->RTK_DCR_Enable = 1;
		Scaler_SetBackLightMappingRange(100,0,255,0,1);
	#else
		VIP_system_info_structure_table->RTK_DCR_Enable = 0;
	#endif

	smd_choosecandidate_reg.regValue = IoReg_Read32(DI_DI_SMD_ChooseCandidate_reg);
	if( Vip_source == VIP_QUALITY_DTV_576I ){
		smd_choosecandidate_reg.smd_reliablerange_sad_th = 100;
	} else {
		smd_choosecandidate_reg.smd_reliablerange_sad_th = 125;
	}
	IoReg_Write32(DI_DI_SMD_ChooseCandidate_reg, smd_choosecandidate_reg.regValue);

#ifdef CONFIG_ENABLE_HDMITX
	drvif_HDMITX_RGB2YUV_only_Matrix(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255]);
#endif // #ifdef CONFIG_ENABLE_HDMITX

	//SE_FilmDtect_Init();

	//NN_SE_Init(); // move to "boot init"

}

/* merlin4 remove
unsigned char Scaler_GetMA_Chroma_Error(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MA_Chroma_Error;

}
//range 0~10
void Scaler_SetMA_Chroma_Error(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MA_Chroma_Error = value;

	fwif_color_set_MA_Chroma_Error(src_idx, value);
}
unsigned char Scaler_GetMA_Chroma_Error_En_From_vipTable(void)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	unsigned char *pVipCoefArray = NULL;
	Scaler_Get_Display_info(&display, &src_idx);

	if (m_cbFunc_getQualityCoef == NULL)
		return 0;

	pVipCoefArray = (unsigned char *)m_cbFunc_getQualityCoef((VIP_SOURCE_TIMING)fwif_vip_source_check(3, NOT_BY_DISPLAY));

	return fwif_color_get_MA_Chroma_Error_en_from_vipTable(src_idx, pVipCoefArray[VIP_QUALITY_FUNCTION_MA_Chroma_Error]);
}
*/
unsigned char Scaler_GetMADI_HMC(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MADI_HMC;

}
/*range 0~6*/
void Scaler_SetMADI_HMC(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MADI_HMC = value;

	fwif_set_color_MADI_HMC(src_idx, value);
}

unsigned char Scaler_GetMADI_HME(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MADI_HME;

}
/*range 0~6*/
void Scaler_SetMADI_HME(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MADI_HME = value;

	fwif_set_color_MADI_HME(src_idx, value);
}

unsigned char Scaler_GetMADI_PAN(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MADI_PAN;

}
/*range 0~6*/
void Scaler_SetMADI_PAN(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MADI_PAN = value;

	fwif_set_color_MADI_PAN(src_idx, value);
}

unsigned char Scaler_GetNew_UVC(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->color_new_uvc;

}
/*range 0~3*/
void Scaler_SetNew_UVC(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->color_new_uvc = value;

	fwif_color_set_new_uvc(src_idx, value);
}

unsigned char Scaler_GetDLti(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->DLti;

}


/*range 0~10*/
void Scaler_SetDLti(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->DLti = value;
    rtd_printk(KERN_EMERG, TAG_NAME, "[%s:%d]new_src,(%d)SetDLti=%d\n", __FUNCTION__, __LINE__,src_idx,value);
	fwif_color_set_dlti_driver(src_idx, value);
}

unsigned char Scaler_GetDCti(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->DCti;
}

/*range 0~10*/
void Scaler_SetDCti(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->DCti = value;

	fwif_color_set_dcti(src_idx, value);
}

unsigned char Scaler_GetIDLti(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->I_DLTi;

}

void Scaler_SetIDLti(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->I_DLTi = value;

}

unsigned char Scaler_GetIDCti(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->I_NEWDCTi;

}

/*range 0~6*/
void Scaler_SetIDCti(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->I_NEWDCTi = value;

	fwif_color_set_inewdcti(src_idx, value);


}

unsigned char Scaler_GetVDCti(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->V_DCTi;
}

/*range 0~10*/
void Scaler_SetVDCti(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->V_DCTi = value;

	fwif_color_set_v_dcti_driver(src_idx, value);
}
unsigned char Scaler_GetVDCti_LPF(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->V_DCTi_Lpf;
}

void Scaler_SetVDCti_LPF(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->V_DCTi_Lpf = value;

	fwif_color_set_v_dcti_lpf_driver(src_idx, value);
}


unsigned char Scaler_GetColor_UV_Delay_Enable()
{
	return drvif_color_Get_Color_UV_Delay_Enable();
}

void Scaler_SetColor_UV_Delay_Enable(unsigned char on_off)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	fwif_color_UV_Delay_Enable(src_idx, on_off);
}

unsigned char Scaler_GetColor_UV_Delay()
{
	return fwif_color_get_UV_Delay();
}

void Scaler_SetColor_UV_Delay(unsigned char value)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	fwif_color_set_UV_Delay(src_idx, value);
}

void Scaler_SetColor_UV_Delay_TOP(unsigned char value)
{

	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display,& src_idx);

	fwif_color_set_UV_Delay_TOP(src_idx,value);

}


unsigned char Scaler_GetFilm_Mode()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Film;

}

void Scaler_SetFilm_Mode(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Film = value;

	fwif_module_set_film_mode(src_idx, value);
}

void Scaler_SetFilmonoff(unsigned char value)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	if (value == 0) /* off*/
		fwif_module_set_film_mode(src_idx, value);
	else /* on*/
		fwif_module_set_film_mode(src_idx, Scaler_GetFilm_Mode());
}

void Scaler_SetFilm_Setting(void)
{
	fwif_module_set_film_setting();
}


unsigned char Scaler_GetIntra(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Intra;

}

/*range 0~11*/
void Scaler_SetIntra(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_THRIP) == 0) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "Warning!! Scaler_SetIntra not through IP\n");
		return;
	}

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Intra = value;

	if (value == 0 || value >= 255) {   /* Intra off*/

		fwif_color_set_Intra_Ver2_driver(src_idx, 0);
	} else if (value < VipPQ_GetIntra_Ver2_TableSize()) {     /*new  Intra*/
		fwif_color_set_Intra_Ver2_driver(src_idx, value);
	} else {                                                 /*old  Intra*/
		fwif_color_set_Intra_Ver2_driver(src_idx, 0);
	}
}

unsigned char Scaler_GetMADI(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->MotionAdaptive;
}

void Scaler_SetMADI(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	rtd_printk(KERN_DEBUG, TAG_NAME, "Scaler_SetMADI, value = %d\n", value);

	if (Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_THRIP) == 0) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "Warning!! Scaler_SetMADI not through IP\n");
		return;
	}

	if(Scaler_VOFromVDEC(Scaler_Get_CurVoInfo_plane()) == SOURCE_ENCODER) {
		value = 6;
	}

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->MotionAdaptive = value;

	fwif_color_ma_set_DI_MA_Adjust_Table(src_idx, value);

}

void Scaler_SetMADI_INIT()
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	/* Hard code "2"  comes from original init setting in past FW code*/
	fwif_color_ma_set_DI_MA_Adjust_Table(src_idx, 2); /* Init setting from original code*/
	fwif_color_ma_set_DI_MA_Init_Table();
}



unsigned char Scaler_GetTNRXC_Ctrl(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->tnr_xc_ctrl;
}

void Scaler_SetTNRXC_Ctrl(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	/*unsigned char display = 0;*/

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->tnr_xc_ctrl = value;
	fwif_color_set_TNRXC_Ctrl(src_idx, value);
}
unsigned char Scaler_GetTNRXC_MK2(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->tnrxc_mk2;
}

void Scaler_SetTNRXC_MK2(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	/*unsigned char display = 0;*/

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->tnrxc_mk2 = value;
	fwif_color_set_TNRXC_MK2(src_idx, value);
}
void Scaler_SetTNRXC(unsigned char value)
{
	Scaler_SetTNRXC_Ctrl(value);
	Scaler_SetTNRXC_MK2(value);
}

extern struct semaphore VPQ_I_CSC_Semaphore;
void Scaler_SetDataFormatHandler(void)
{
	unsigned char en_422to444, InputSrcGetType;
	#ifdef CONFIG_FORCE_RUN_I3DDMA
	unsigned char InputSrcGetFrom;
	#endif
	unsigned short mode, channel;
	SCALER_DISP_CHANNEL display;

	down(&VPQ_I_CSC_Semaphore);
	mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	channel = Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL);
	en_422to444 = (unsigned char)Scaler_DispGetStatus(display, SLR_DISP_422);
	InputSrcGetType = Scaler_InputSrcGetType(display);

#ifdef CONFIG_FORCE_RUN_I3DDMA
	/*
	if((display == SLR_MAIN_DISPLAY) && (get_force_i3ddma_enable(display) == true))
		InputSrcGetType = _SRC_VO;
	else
		InputSrcGetType = Scaler_InputSrcGetType(display);
	*/
	if (fwif_color_get_force_run_i3ddma_enable(display)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &InputSrcGetType, &InputSrcGetFrom);
		mode = fwif_color_get_cur_input_timing_mode(SLR_MAIN_DISPLAY);
	}
#endif
	if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
		fwif_color_SetDataFormatHandler(display, mode, channel, en_422to444, InputSrcGetType);
	else
		fwif_color_SetDataFormatHandler_tv006();
	up(&VPQ_I_CSC_Semaphore);
}

unsigned char Scaler_get_YUV2RGB(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 255;		/* skip this function if table 255 is selected.*/
	return pTable->YUV2RGB_Table_Select;
}

void Scaler_set_YUV2RGB(unsigned char which_table, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->YUV2RGB_Table_Select = which_table;
	VIPprintf("which_table = %d, drvif_write_flag = %d,%d\n", which_table, drvSkip_Flag_coefByY, drvSkip_Flag_uvOffset);

	fwif_color_set_YUV2RGB(src_idx, display, which_table, drvSkip_Flag_coefByY, drvSkip_Flag_uvOffset);

}

#ifdef CONFIG_ENABLE_HDMITX
void Scaler_color_set_HDMITX_RGB2YUV(void)
{
    unsigned char table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
    drvif_HDMITX_RGB2YUV_only_Matrix(tRGB2YUV_COEF[table_num]);

    return;
}
#endif // #ifdef CONFIG_ENABLE_HDMITX


#if 0
unsigned char Scaler_get_YUV2RGB_COEF_BY_Y(void)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 255;		/* skip this function if table 255 is selected.*/
	return pTable->YUV2RGB_COEF_BY_Y_Table_Select;
}

void Scaler_set_YUV2RGB_COEF_BY_Y(unsigned char which_table, unsigned char drvSkip_Flag)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->YUV2RGB_COEF_BY_Y_Table_Select = which_table;
	VIPprintf("which_table = %d, drvif_write_flag = %d\n", which_table, drvSkip_Flag);
	if (drvSkip_Flag == 1)
		return;

	/* this path need to remove future. using "fwif_color_set_YUV2RGB_COEF_BY_Y" directly*/
	/*fwif_color_set_YUV2RGB_COEF_BY_Y_driver(src_idx, which_table);*/
	fwif_color_set_YUV2RGB_COEF_BY_Y(src_idx, display, which_table);

}

unsigned char Scaler_get_YUV2RGB_UV_OFFSET_BY_Y(void)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 255;		/* skip this function if table 255 is selected*/
	return pTable->YUV2RGB_UV_OFFSET_BY_Y_Table_Select;

}

void Scaler_set_YUV2RGB_UV_OFFSET_BY_Y(unsigned char which_table, unsigned char drvSkip_Flag)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->YUV2RGB_UV_OFFSET_BY_Y_Table_Select = which_table;

	if (drvSkip_Flag == 1)
		return;

	/* this path need to remove future. using "fwif_color_set_YUV2RGB_UV_OFFSET_BY_Y" directly*/
	/*fwif_color_set_YUV2RGB_UV_OFFSET_BY_Y_driver(src_idx, which_table);*/
	fwif_color_set_YUV2RGB_UV_OFFSET_BY_Y(src_idx, display, which_table);

}
#endif


void Scaler_module_vdc_SetConBriHueSat_Table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	if (Scaler_InputSrcGetMainChFrom() != _SRC_FROM_VDC) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "Warning!! It's not vdc Src !!");
		return;
	}

	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->VD_ConBriHueSat_Table = value;

	fwif_color_module_vdc_SetConBriHueSat(src_idx, value);

}
unsigned char Scaler_module_vdc_GetConBriHueSat_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;

	if (Scaler_InputSrcGetMainChFrom() == _SRC_FROM_VDC) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "Warning!! It's not vdc Src !!");
		return 255;
	}

	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->VD_ConBriHueSat_Table;
}


unsigned char Scaler_get_ICM_table()
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	StructColorDataType *pTable = NULL;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	/*VIPprintf("@@@@@@@@=== 1.fwif_color_get_ICM_table ==\n");*/
	if (pTable == NULL)
		return 0;
	/*VIPprintf("@@@@@@@@=== 2.fwif_color_get_ICM_table = %d ==\n", pTable->ICM_Table);*/
	return pTable->ICM_Table;
}

void Scaler_set_ICM_table(unsigned char value, unsigned char drvSkip_Flag)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	StructColorDataType *pTable = NULL;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ICM_Table = value;

	if (drvSkip_Flag == 1)
		return;

	fwif_color_set_ICM_table(src_idx,  value);
}

void Scaler_set_ICM_table_by_VipTable(void)
{
	unsigned char source;
	unsigned char *pVipCoefArray;
	//unsigned char *drvSkip_Array;
	SLR_VIP_TABLE *gVIP_Table;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	if (m_cbFunc_getQualityExtend3 == NULL)
		return;

	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)source);

	if (source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)
		return;

	gVIP_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (NULL == gVIP_Table)
		return;

	//drvSkip_Array = &(gVIP_Table->DrvSetting_Skip_Flag[0]);
	Scaler_set_ICM_table(pVipCoefArray[VIP_QUALITY_FUNCTION_ICM],  0);
}


unsigned char Scaler_GetGamma(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Gamma;
}


void Scaler_SetGamma(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Gamma = value;
	fwif_set_gamma(pTable->Gamma, pTable->Sindex, pTable->Shigh, pTable->Slow);
}

unsigned char Scaler_GetSindex(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Sindex;
}

unsigned char Scaler_SetSindex(unsigned short value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Sindex = value;
	fwif_set_gamma(pTable->Gamma, pTable->Sindex, pTable->Shigh, pTable->Slow);
	return 0;
}

unsigned char Scaler_GetSlow(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Slow;
}

/*range 0~3*/
unsigned char Scaler_SetSlow(unsigned short value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Slow = value;
	fwif_set_gamma(pTable->Gamma, pTable->Sindex, pTable->Shigh, pTable->Slow);
	return 0;
}

unsigned char Scaler_GetShigh(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Shigh;
}

/*range 0~3*/
unsigned char Scaler_SetShigh(unsigned short value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 1;
	pTable->Shigh = value;
	fwif_set_gamma(pTable->Gamma, pTable->Sindex, pTable->Shigh, pTable->Slow);
	return 0;
}

/*20140212 roger, for avoid to write gamma table 4 times, for fwif_color_video_quality_handler only*/
void Scaler_SetGamma_for_Handler(unsigned char gamma,
															unsigned char s_gamma_index,
															unsigned char s_gamma_low,
															unsigned char s_gamma_high, unsigned char drvSkip_Flag)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;

	pTable->Gamma = gamma;
	pTable->Sindex = s_gamma_index;
	pTable->Slow = s_gamma_low;
	pTable->Shigh = s_gamma_high;

	if (drvSkip_Flag == 1)
		return;
	fwif_set_gamma(pTable->Gamma, pTable->Sindex, pTable->Shigh, pTable->Slow);

}
void Scaler_SetGamma_for_ddomainISR_Handler(unsigned char gamma,
															unsigned char s_gamma_index,
															unsigned char s_gamma_low,
															unsigned char s_gamma_high, unsigned char drvSkip_Flag)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;

	pTable->Gamma = gamma;
	pTable->Sindex = s_gamma_index;
	pTable->Slow = s_gamma_low;
	pTable->Shigh = s_gamma_high;

	if (drvSkip_Flag == 1)
		return;

	fwif_set_ddomainISR_gamma(pTable->Gamma, pTable->Sindex, pTable->Shigh, pTable->Slow);

}
#if 0 //k5l hw remove
unsigned char Scaler_Get_blue_stretch(void)
{
#if 1
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->blue_stretch;
#else
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)VIP_system_info_structure_table->VIP_source);

	return pVipCoefArray[VIP_QUALITY_FUNCTION_blue_stretch];

#endif

}
#endif
#if 0 //k5l hw remove
void Scaler_set_blue_stretch(unsigned char table)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->blue_stretch = table;

	fwif_color_set_blue_stretch(src_idx, table);
}
#endif

unsigned short Scaler_GetSharpnessTable()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Shp_Table;
}

void Scaler_SetSharpnessTable(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "fwif_color_set_sharpness_table error %d \n", value);
		return;
	}
	if (value >= Sharp_table_num)
		value = 0;

	pTable->Shp_Table = value;
	if (vpq_project_id == 0x00060000 && vpqex_project_id == 0x00060000) {
		/*rtd_printk(KERN_DEBUG, TAG_NAME, "fwif_color_set_sharpness_table TV006 %d \n", value);*/
		drvif_color_2dpk_init(0, 1);

		fwif_color_set_sharpness_table_init_TV006(src_idx, value);//fixed me after demo please unmark

        #ifdef LGDB_OFF //control table by rtk
		fwif_color_set_sharpness_table(src_idx, value);//fixed me after demo please remove
                Scaler_SetSharpness(25); //fixed me after demo pleae remove
        #else
			fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_SR_UI_T);
        #endif

	} else {
		/*rtd_printk(KERN_DEBUG, TAG_NAME, "fwif_color_set_sharpness_table TV001 %d \n", value);*/
		fwif_color_set_sharpness_table(src_idx, value);
	}

}
unsigned short Scaler_GetCDSTable()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->CDS_Table;

}
void Scaler_SetCDSTable(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	Table_CDS = value;

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->CDS_Table = value;

	fwif_color_set_cds_table(src_idx, value);
}

unsigned short Scaler_GetEGSM_postshp_level()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Egsm_postshp_level;
}

void Scaler_SetEGSM_postshp_level(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	if (display != SLR_MAIN_DISPLAY) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "Warning!! Dering have to working in SLR_MAIN_DISPLAY!! ");
		rtd_printk(KERN_DEBUG, TAG_NAME, "Return!! ");
		return;
	}

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Egsm_postshp_level = value;

	fwif_color_set_egsm_postshp_level_driver(src_idx, value);


}

unsigned char Scaler_GetEMF_Mk2()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->emf_mk2;
}

void Scaler_SetEMF_Mk2(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->emf_mk2 = value;

	fwif_color_set_emf_mk2_driver(src_idx, value);

}

unsigned char Scaler_get_Adaptive_Gamma(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Adaptive_Gamma;
}

#if 0 //removed from merlin3, by caroline,20170606
void Scaler_set_Adaptive_Gamma(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Adaptive_Gamma = value;
	fwif_color_set_Adaptive_Gamma(src_idx, value);
}
#endif

unsigned char Scaler_GetRTNR_Noise_Measure(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->RTNR_NM;
}

void Scaler_SetRTNR_Noise_Measure(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->RTNR_NM = value;

	fwif_color_set_RTNR_Noise_Measure_flow(src_idx, value);
}

unsigned char Scaler_GetScaleUPH(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScaleUpH;
}

void Scaler_SetScaleUPH(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleUpH = value;

	fwif_color_set_scaleup_hcoef(src_idx, display, value);
}

unsigned char Scaler_GetScaleUPH_8tap(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScaleUpH_8tap;
}

/*range 0~11*/
void Scaler_SetScaleUPH_8tap(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleUpH_8tap = value;


	fwif_color_set_scaleuph_8tap_driver(src_idx, display, value);

}

unsigned char Scaler_GetScaleUPV(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScaleUpV;
}

/*range 0~10*/
void Scaler_SetScaleUPV(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleUpV = value;

	fwif_color_set_scaleup_vcoef(src_idx, display, value);
}

void Scaler_SetScaleUP_YCHV(unsigned char HY, unsigned char VY, unsigned char HC, unsigned char VC)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleUpH = HY;
	pTable->ScaleUpV = VY;
	pTable->ScaleUpH_C = HC;
	pTable->ScaleUpV_C = VC;

	fwif_color_set_scaleup_hv4tap_coef(src_idx, display, (char)HY, (char)VY, (char)HC, (char)VC);
}
void Scaler_GetScaleUP_YCHV(unsigned char *pHY,unsigned char *pVY,unsigned char *pHC,unsigned char *pVC)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	*pHY = pTable->ScaleUpH;
	*pVY = pTable->ScaleUpV;
	*pHC = pTable->ScaleUpH_C;
	*pVC = pTable->ScaleUpV_C;

}




unsigned char Scaler_GetScaleUPDir(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScaleUpDir;

}

/*range 0~19*/
void Scaler_SetScaleUPDir(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleUpDir = value;

	fwif_color_set_scaleup_dircoef(src_idx, display, value);
}

unsigned char Scaler_GetScaleUPDir_Weighting(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScaleUpDir_weighting;

}

/*range 0~9*/
void Scaler_SetScaleUPDir_Weighting(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleUpDir_weighting = value;
	fwif_color_set_scaleup_dir_weighting_driver(src_idx, value);
}

unsigned char Scaler_GetScalePK(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScalePK;
}

/*range 0~10*/
void Scaler_SetScalePK(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScalePK = value;
	fwif_color_set_scalepk_driver(src_idx, value);
}
unsigned short Scaler_GetSUPKType()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->SUPK_type;
}

void Scaler_SetSUPKType(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->SUPK_type = value;

	fwif_color_set_supk_type_driver(src_idx, value);
}
unsigned short Scaler_GetUnsharpMask()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->UnsharpMask;
}
void Scaler_SetUnsharpMask(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;

	pTable->UnsharpMask = value;


	fwif_color_set_unsharp_mask_driver(src_idx, value);
}



unsigned char Scaler_get_egsm_postshp_table()
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Egsm_Postshp_table;
}

void Scaler_color_egsm_postshp_table(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	if (display == SLR_MAIN_DISPLAY) {
		Table_EGSM_Postshp = value;
	}

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Egsm_Postshp_table = value;

	fwif_color_set_egsm_postshp_level_driver(_MAIN_DISPLAY, Scaler_GetEGSM_postshp_level());/* cheange table need to re-set level*/
}
unsigned char Scaler_getdESM(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Egsm_Postshp_table;
}
void Scaler_setdESM(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Egsm_Postshp_table = value;

	fwif_color_set_dESM(display, value);
}

unsigned char Scaler_getiESM(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->iESM;
}


void Scaler_setiESM(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->iESM = value;

	fwif_color_set_iESM(display, value);
}

#ifdef ENABLE_SUPER_RESOLUTION
void Scaler_set_SuperResolution_init_table(unsigned char value)
{
	fwif_color_set_SuperResolution_init_table(value);
}

unsigned char Scaler_GetSR_edge_gain(void)
{
	StructColorDataType *pTable = NULL;
    unsigned char src_idx = 0;
    unsigned char display = 0;
    Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->SR_edge_gain;
}

void Scaler_SetSR_edge_gain(unsigned char value)
{
	StructColorDataType *pTable = NULL;
    unsigned char src_idx = 0;
    unsigned char display = 0;
    Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->SR_edge_gain = value;

	fwif_color_set_SR_edge_gain(src_idx, value);
}

unsigned char Scaler_GetSR_texture_gain(void)
{
	StructColorDataType *pTable = NULL;
    unsigned char src_idx = 0;
    unsigned char display = 0;
    Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->SR_texture_gain;
}

/*range 0~1, Lydia*/
void Scaler_SetSR_texture_gain(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->SR_texture_gain = value;

	fwif_color_set_SR_texture_gain(GET_USER_INPUT_SRC(), value);
}
#endif

unsigned char Scaler_GetScaleDOWNHTable(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScaleDownHTable;
}

/*range 0~17*/
void Scaler_SetScaleDOWNHTable(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	/*unsigned char ucScaleDownH = 0;*/
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);


	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleDownHTable = value;

	fwif_color_set_scaledownh_table(GET_USER_INPUT_SRC(), value);
}
unsigned char Scaler_GetScaleDOWNVTable(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScaleDownVTable;
}

/*range 0~17*/
void Scaler_SetScaleDOWNVTable(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleDownVTable = value;

	fwif_color_set_scaledownv_table(GET_USER_INPUT_SRC(), value);
}
unsigned char Scaler_GetScaleDOWN444To422(void)
{
	return fwif_color_get_scaledown444To422(GET_USER_INPUT_SRC());
}

/*range 0~17*/
void Scaler_SetScaleDOWN444To422(unsigned char value)
{
	fwif_color_set_scaledown444To422(GET_USER_INPUT_SRC(), value);
}


void Scaler_SetScaleUP_TwoStep(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;

	Scaler_Get_Display_info(&display,& src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleUP_TwoStep= value;

 	fwif_color_set_twostep_scaleup_coef( src_idx, display, value);
}

unsigned char Scaler_GetScaleUP_TwoStep(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScaleUP_TwoStep;
}


unsigned char Scaler_GetScaleUPV_6tap(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->ScaleUpV_6tap;
}

/*range 0~10*/
void Scaler_SetScaleUPV_6tap(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ScaleUpV_6tap = value;

	fwif_color_set_scaleupv_6tap_driver(src_idx, display, value);
}

unsigned char Scaler_GetDiSmd(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->DISMDTable;
}

void Scaler_SetDiSmd(unsigned char value)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	if (display != SLR_MAIN_DISPLAY) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "Warning!! DI SMD have to working in SLR_MAIN_DISPLAY!! ");
		rtd_printk(KERN_DEBUG, TAG_NAME, "Return!! ");
		return;
	}

	fwif_color_set_dismd_driver(src_idx, value);
}

void Scaler_SetScaleOSD_Sharpness(unsigned char value)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA)
	extern void fwif_color_set_scaleosd_sharpness_driver(unsigned char src_idx, unsigned char value);
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->OSD_Sharpness = value;

	fwif_color_set_scaleosd_sharpness_driver(src_idx, value);
#endif
}

unsigned char Scaler_GetScaleOSD_Sharpness(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->OSD_Sharpness;
}

unsigned char Scaler_Debug_Get_UserCurveDCC_Curve_Select(void)
{
	return fwif_color_Debug_Get_UserCurveDCC_Curve_Select();
}

unsigned char Scaler_Debug_Get_HIST_MEAN_VALUE(void)
{
	return fwif_color_Debug_Get_HIST_MEAN_VALUE();
}

unsigned char Scaler_Debug_Get_Pure_Color_Flag(void)
{
	return fwif_color_Debug_Get_Pure_Color_Flag();
}

unsigned char Scaler_Debug_Get_Skin_Tone_Found_Flag(void)
{
	return fwif_color_Debug_Get_Skin_Tone_Found_Flag();
}

unsigned char Scaler_Debug_Get_UserCurveDCC_Curve_Weighting(void)
{
	return fwif_color_Debug_Get_UserCurveDCC_Curve_Weighting(Main_DCC_User_Define_Curve_Table_Select);
}

unsigned char Scaler_Debug_Get_UserCurveDCC_Curve_Weighting_Skin(void)
{
	return fwif_color_Debug_Get_UserCurveDCC_Curve_Weighting_Skin(Main_DCC_User_Define_Curve_Table_Select);
}

void Scaler_Debug_Set_UserCurveDCC_Curve_Select(unsigned char value)
{
	fwif_color_Debug_Set_UserCurveDCC_Curve_Select(value);
}

void Scaler_Debug_Set_UserCurveDCC_Curve_Apply(unsigned char value)
{
	fwif_color_Debug_Set_UserCurveDCC_Curve_Apply(Main_DCC_User_Define_Curve_Table_Select, value);
}

void Scaler_Debug_Set_UserCurveDCC_Curve_Weighting(unsigned char value)
{
	fwif_color_Debug_Set_UserCurveDCC_Curve_Weighting(Main_DCC_User_Define_Curve_Table_Select, value);
}

void Scaler_Debug_Set_UserCurveDCC_Curve_Weighting_Skin(unsigned char value)
{
	fwif_color_Debug_Set_UserCurveDCC_Curve_Weighting_Skin(Main_DCC_User_Define_Curve_Table_Select, value);
}

void Scaler_set_pq_dir_path(char *path, char *patch_path)
{
	fwif_set_pq_dir_path(path, patch_path);
}
unsigned char Scaler_get_average_luma(unsigned char *val)
{
	fwif_get_average_luma(val);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Scaler_get_average_luma = %d\n", *val);
	return 1;
}

unsigned char Scaler_get_Y_Total_Hist_Cnt_distribution(UINT8 MODE, UINT8 size, unsigned int *Y_Hist_distr_cnt)
{
	return fwif_get_Y_Total_Hist_Cnt_distribution(MODE, size, &Y_Hist_distr_cnt[0]);
}

unsigned char Scaler_get_luma_level_distribution(unsigned short *luma_distribution, unsigned char size)
{
	int i = 0;
	fwif_get_luma_level_distribution((unsigned int *)luma_distribution,  size);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Scaler_get_luma_level_distribution\n");

	for (i = 0; i < size; i++) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "\n luma_distribution[%d] = %d ", i, luma_distribution[i]);
	}
	return 1;
}

/**
 * Panel_SetBackLightRange
 * mapping uiMax to actMax,uiMin to actMin,and use uiStep as step size when use Panel_SetBackLightLevel
 *
 * @param {uiMax : 0~255,default 100}
 * @param {uiMin : 0~255,default 0}
 * @param {uiMax : 0~255,default 255}
 * @param {uiMin : 0~255,default 0}
 * @return {TRUE/false }
 *
 */
unsigned char Scaler_SetBackLightMappingRange(unsigned int uiMax, unsigned int uiMin,
							unsigned int actMax, unsigned int actMin,unsigned char increaseMode)
{


	/*=== 20140421 CSFC for vip video fw infomation ====*/
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}
	if( (actMax>255) || (actMin>255) || (actMax<=actMin))
		return false;

	if( (uiMax>255) || (uiMin>255) || (uiMax<=uiMin) )
		return false;

	VIP_system_info_structure_table->OSD_Info.backlightLevel_actMax = actMax;
	VIP_system_info_structure_table->OSD_Info.backlightLevel_actMin = actMin;
	VIP_system_info_structure_table->OSD_Info.backlightLevel_uiMax = uiMax;
	VIP_system_info_structure_table->OSD_Info.backlightLevel_uiMin = uiMin;
	VIP_system_info_structure_table->OSD_Info.isIncreaseMode_Flag = increaseMode;


	return TRUE;
}

unsigned char Scaler_GetBackLightMappingRange(unsigned int *uiMax,unsigned int *uiMin,unsigned int *actMax,unsigned int *actMin, unsigned char *increaseMode)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}
	*uiMax		= VIP_system_info_structure_table->OSD_Info.backlightLevel_actMax;
	*uiMin		= VIP_system_info_structure_table->OSD_Info.backlightLevel_actMin;
	*actMax		= VIP_system_info_structure_table->OSD_Info.backlightLevel_uiMax;
	*actMin		= VIP_system_info_structure_table->OSD_Info.backlightLevel_uiMin;
	*increaseMode 	= VIP_system_info_structure_table->OSD_Info.isIncreaseMode_Flag;

	return TRUE;
}

unsigned char Scaler_GetDcrMode(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;

	return pData->DCRMode;
}

void Scaler_SetDcrMode(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;

	pData->DCRMode= value;

	Scaler_set_DCR_Enable(value);
}

unsigned char Scaler_SetOSDBacklight2VIP(unsigned char value)
{
#if 0
#ifdef CONFIG_PCBMGR
	static unsigned int m_PIN_BL_ADJ = 0xFFFFFFFF;
	int index, type;
	unsigned long long enum_info = 0;
	unsigned int 	uiMax;
	unsigned int 	uiMin;
	unsigned int 	actMax;
	unsigned int 	actMin;
	unsigned char 	increaseMode;
#endif
#endif
	/*=== 20140421 CSFC for vip video fw infomation ====*/
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	VIP_system_info_structure_table->OSD_Info.OSD_Backlight = value;
#if 0
#ifdef CONFIG_PCBMGR
	if (m_PIN_BL_ADJ == 0xFFFFFFFF) {
		if (pcb_mgr_get_enum_info_byname("PIN_BL_ADJ", &enum_info) < 0) {
			m_PIN_BL_ADJ = 0xFFFFFFFF;
			VIP_system_info_structure_table->OSD_Info.backlight_pin_adj = 0;
		} else {
			if (PCB_PIN_TYPE_ISO_GPIO == GET_PIN_TYPE(enum_info) || PCB_PIN_TYPE_ISO_PWM == GET_PIN_TYPE(enum_info) || PCB_PIN_TYPE_ISO_UNIPWM == GET_PIN_TYPE(enum_info))
				type = 1;
			else
				type = 0;
			index = GET_PIN_INDEX(enum_info);
			m_PIN_BL_ADJ =  ((type & 0xFFFF) << 16) | (index & 0xFFFF);
			VIP_system_info_structure_table->OSD_Info.backlight_pin_adj = fwif_color_ChangeOneUINT32Endian(m_PIN_BL_ADJ, 0);
		}
	}
#endif

#if 0
	Panel_GetBackLightMappingRange(&uiMax, &uiMin, &actMax, &actMin, &increaseMode);

	VIP_system_info_structure_table->OSD_Info.backlightLevel_actMax = fwif_color_ChangeOneUINT32Endian(actMax, 0);
	VIP_system_info_structure_table->OSD_Info.backlightLevel_actMin = fwif_color_ChangeOneUINT32Endian(actMin, 0);
	VIP_system_info_structure_table->OSD_Info.backlightLevel_uiMax = fwif_color_ChangeOneUINT32Endian(uiMax, 0);
	VIP_system_info_structure_table->OSD_Info.backlightLevel_uiMin = fwif_color_ChangeOneUINT32Endian(uiMin, 0);
	VIP_system_info_structure_table->OSD_Info.isIncreaseMode_Flag = increaseMode;
#endif
#if 0
	int ret;
	mScalerOSDBacklight = value;

	/* use VIP_system_info_structure_table->OSD_Info.OSD_Backlight instead, remove dollow code after mac3 video fw code ready*/
	if (0 != (Scaler_SendRPC(SCALERIOC_BL_LV_FROM_USER, mScalerOSDBacklight, RPC_SEND_VALUE_ONLY))) {
		SLRLIB_PRINTF("SCALERIOC_BL_LV_FROM_USER fail. ret = %d\n", ret);
		return false;
	}
#endif
#endif
	return TRUE;
}

void Scaler_set_DCR_Enable(UINT8 value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	VIP_system_info_structure_table->OSD_Info.OSD_DCR_Mode = value;

}

UINT8 Scaler_get_DCR_Enable(void)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return VIP_system_info_structure_table->OSD_Info.OSD_DCR_Mode;

}

UINT32 Scaler_get_DCC_AdapCtrl_Current_Mean(void)
{
	unsigned int pwCopyTemp = 0;
	_clues *VIP_SmartPic_clues = NULL;
	VIP_SmartPic_clues = fwif_color_GetShare_Memory_SmartPic_clue();

	if (VIP_SmartPic_clues == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get Scaler_get_DCC_AdapCtrl_Mean Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	fwif_color_ChangeUINT32Endian_Copy(&(VIP_SmartPic_clues->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_mean]), 1, &pwCopyTemp, 0);

	return pwCopyTemp;
}

UINT32 Scaler_get_DCC_AdapCtrl_Current_Dev(void)
{
	unsigned int pwCopyTemp = 0;
	_clues *VIP_SmartPic_clues = NULL;
	VIP_SmartPic_clues = fwif_color_GetShare_Memory_SmartPic_clue();

	if (VIP_SmartPic_clues == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get Scaler_get_DCC_AdapCtrl_Dev Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	fwif_color_ChangeUINT32Endian_Copy(&(VIP_SmartPic_clues->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_Dev]), 1, &pwCopyTemp, 0);

	return pwCopyTemp;
}

VIP_DCC_AdaptCtrl_Level_Table *Scaler_get_DCC_AdapCtrl_Table(void)
{
	unsigned char AdaptCtrl_Level_Table = 0;
	SLR_VIP_TABLE *prt = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	if (prt == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	AdaptCtrl_Level_Table = Scaler_Get_DCC_AdaptCtrl_Level_Table();

	return &(prt->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[AdaptCtrl_Level_Table]);

}


unsigned char Scaler_get_DCC_AdapCtrl_DCC_Level(void)
{
	unsigned int AdapCtrl_Current_Mean = 0, AdapCtrl_Current_Dev = 0;
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;

	DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();

	if (DCC_AdaptCtrl_Level_Table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	AdapCtrl_Current_Mean = Scaler_get_DCC_AdapCtrl_Current_Mean();
	AdapCtrl_Current_Dev = Scaler_get_DCC_AdapCtrl_Current_Dev();

	return (DCC_AdaptCtrl_Level_Table->DCC_Level_table)[AdapCtrl_Current_Mean][AdapCtrl_Current_Dev];

}

BOOL Scaler_set_DCC_AdapCtrl_DCC_Level(UINT8 value)
{
	unsigned int AdapCtrl_Current_Mean = 0, AdapCtrl_Current_Dev = 0;
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;

	DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();

	if (DCC_AdaptCtrl_Level_Table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	AdapCtrl_Current_Mean = Scaler_get_DCC_AdapCtrl_Current_Mean();
	AdapCtrl_Current_Dev = Scaler_get_DCC_AdapCtrl_Current_Dev();

	(DCC_AdaptCtrl_Level_Table->DCC_Level_table)[AdapCtrl_Current_Mean][AdapCtrl_Current_Dev] = value;
	fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

	return TRUE;


}


unsigned char Scaler_get_DCC_AdapCtrl_Cubic_index(void)
{
	unsigned int AdapCtrl_Current_Mean = 0, AdapCtrl_Current_Dev = 0;
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;

	DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();

	if (DCC_AdaptCtrl_Level_Table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	AdapCtrl_Current_Mean = Scaler_get_DCC_AdapCtrl_Current_Mean();
	AdapCtrl_Current_Dev = Scaler_get_DCC_AdapCtrl_Current_Dev();

	return (DCC_AdaptCtrl_Level_Table->DCC_Cublc_Index_table)[AdapCtrl_Current_Mean][AdapCtrl_Current_Dev];

}

BOOL Scaler_set_DCC_AdapCtrl_Cubic_index(UINT8 value)
{
	unsigned int AdapCtrl_Current_Mean = 0, AdapCtrl_Current_Dev = 0;
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;

	DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();

	if (DCC_AdaptCtrl_Level_Table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	AdapCtrl_Current_Mean = Scaler_get_DCC_AdapCtrl_Current_Mean();
	AdapCtrl_Current_Dev = Scaler_get_DCC_AdapCtrl_Current_Dev();

	(DCC_AdaptCtrl_Level_Table->DCC_Cublc_Index_table)[AdapCtrl_Current_Mean][AdapCtrl_Current_Dev] = value;

	fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

	return TRUE;

}


unsigned char Scaler_get_DCC_AdapCtrl_Cubic_low(void)
{
	unsigned int AdapCtrl_Current_Mean = 0, AdapCtrl_Current_Dev = 0;
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;

	DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();

	if (DCC_AdaptCtrl_Level_Table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	AdapCtrl_Current_Mean = Scaler_get_DCC_AdapCtrl_Current_Mean();
	AdapCtrl_Current_Dev = Scaler_get_DCC_AdapCtrl_Current_Dev();

	return (DCC_AdaptCtrl_Level_Table->DCC_Cublc_Low_table)[AdapCtrl_Current_Mean][AdapCtrl_Current_Dev];
}

BOOL Scaler_set_DCC_AdapCtrl_Cubic_low(UINT8 value)
{
	unsigned int AdapCtrl_Current_Mean = 0, AdapCtrl_Current_Dev = 0;
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;

	DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();

	if (DCC_AdaptCtrl_Level_Table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	AdapCtrl_Current_Mean = Scaler_get_DCC_AdapCtrl_Current_Mean();
	AdapCtrl_Current_Dev = Scaler_get_DCC_AdapCtrl_Current_Dev();

	(DCC_AdaptCtrl_Level_Table->DCC_Cublc_Low_table)[AdapCtrl_Current_Mean][AdapCtrl_Current_Dev] = value;
	fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

	return TRUE;

}

unsigned char Scaler_get_DCC_AdapCtrl_Cubic_high(void)
{
	unsigned int AdapCtrl_Current_Mean = 0, AdapCtrl_Current_Dev = 0;
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;

	DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();

	if (DCC_AdaptCtrl_Level_Table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	AdapCtrl_Current_Mean = Scaler_get_DCC_AdapCtrl_Current_Mean();
	AdapCtrl_Current_Dev = Scaler_get_DCC_AdapCtrl_Current_Dev();

	return (DCC_AdaptCtrl_Level_Table->DCC_Cublc_High_table)[AdapCtrl_Current_Mean][AdapCtrl_Current_Dev];
}

BOOL Scaler_set_DCC_AdapCtrl_Cubic_high(UINT8 value)
{
	unsigned int AdapCtrl_Current_Mean = 0, AdapCtrl_Current_Dev = 0;
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;

	DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();

	if (DCC_AdaptCtrl_Level_Table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return false;
	}

	AdapCtrl_Current_Mean = Scaler_get_DCC_AdapCtrl_Current_Mean();
	AdapCtrl_Current_Dev = Scaler_get_DCC_AdapCtrl_Current_Dev();

	(DCC_AdaptCtrl_Level_Table->DCC_Cublc_High_table)[AdapCtrl_Current_Mean][AdapCtrl_Current_Dev] = value;
	fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

	return TRUE;

}

void Scaler_Get_color_access_TVStatus_DisplayInfo(unsigned int *VIP_Access_Memory_DisplayInfo_Table)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	VIP_Access_Memory_DisplayInfo_Table[inputsrc] = Scaler_InputSrcGetType((SCALER_DISP_CHANNEL)display);
	VIP_Access_Memory_DisplayInfo_Table[HDMI_Data_Range]	= drvif_IsRGB_YUV_RANGE();
	VIP_Access_Memory_DisplayInfo_Table[Frame_rate_input] = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ);
	VIP_Access_Memory_DisplayInfo_Table[Frame_rate_output] = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY, SLR_INPUT_FRAMESYNC);

}


#if 1//def LocalDimming
void Scaler_SetLocalDimmingEnable(unsigned char enable)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);



	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] 1.LD_Enable = %d\n", enable);
	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "fwif_color_set_sharpness_table error %d \n", enable);
		return;
	}

	pTable->LocalDimmingEnable = enable;
	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] 2.LD_Enable = %d\n", pTable->LocalDimmingEnable);

	fwif_color_set_LD_Enable(src_idx, enable);

}


void Scaler_SetLocalDimmingTable(unsigned char TableIdx)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] 1.LD_Table = %d\n", TableIdx);
	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "fwif_color_set_sharpness_table error %d \n", TableIdx);
		return;
	}

	pTable->LocalDimmingTable = TableIdx;
	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] 2.LD_Table = %d\n", pTable->LocalDimmingTable);

	Scaler_fwif_color_set_LocalDimming_table(src_idx, TableIdx);

}

void Scaler_fwif_color_set_LocalDimming_table(unsigned char src_idx, unsigned char TableIdx)
{


	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] TableIdx = %d\n", TableIdx);
	fwif_color_set_LD_Global_Ctrl(src_idx, TableIdx);
	fwif_color_set_LD_Backlight_Decision(src_idx, TableIdx);
	fwif_color_set_LD_Spatial_Filter(src_idx, TableIdx);
	fwif_color_set_LD_Spatial_Remap(src_idx, TableIdx);
	fwif_color_set_LD_Boost(src_idx,TableIdx);
	fwif_color_set_LD_Temporal_Filter(src_idx, TableIdx);
	fwif_color_set_LD_Backlight_Final_Decision(src_idx, TableIdx);
	fwif_color_set_LD_Data_Compensation(src_idx, TableIdx);
	fwif_color_set_LD_Data_Compensation_NewMode_2DTable(src_idx,TableIdx);
	fwif_color_set_LD_Backlight_Profile_Interpolation(src_idx, TableIdx);
	fwif_color_set_LD_BL_Profile_Interpolation_Table(src_idx, TableIdx);
	fwif_color_set_LD_Demo_Window(src_idx, TableIdx);

	return;

}

#endif


void Scaler_SetLocalContrastEnable(unsigned char enable)
{

	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);



	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] 1.LC_Enable = %d\n", enable);
	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "fwif_color_set_sharpness_table error %d \n", enable);
		return;
	}

	pTable->LocalContrastEnable = enable;
	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] 2.LC_Enable = %d\n", pTable->LocalContrastEnable);

	fwif_color_set_LC_Enable(src_idx, enable);

}
unsigned char Scaler_GetLocalContrastEnable(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->LocalContrastEnable;
}


void Scaler_SetLocalContrastTable(unsigned char TableIdx)
{

	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] 1.LC_Table = %d\n", TableIdx);
	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "fwif_color_set_sharpness_table error %d \n", TableIdx);
		return;
	}

	pTable->LocalContrastTable = TableIdx;
	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] 2.LC_Table = %d\n", pTable->LocalContrastTable);

	Scaler_fwif_color_set_LocalContrast_table(src_idx, TableIdx);


}
unsigned char Scaler_GetLocalContrastTable(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->LocalContrastTable;
}

void Scaler_fwif_color_set_LocalContrast_table(unsigned char src_idx, unsigned char TableIdx)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_LC] = 0;
	if(TableIdx>=LC_Table_NUM)
		TableIdx=LC_Table_NUM-1;

	rtd_printk(KERN_DEBUG, TAG_NAME, "===[scalerColorLib_Default.cpp] TableIdx = %d\n", TableIdx);
	fwif_color_set_LC_Global_Ctrl(src_idx, TableIdx);
	fwif_color_set_LC_Temporal_Filter(src_idx, TableIdx);
	fwif_color_set_LC_Backlight_Profile_Interpolation(src_idx, TableIdx);
	fwif_color_set_LC_Demo_Window(src_idx, TableIdx);
	fwif_color_set_LC_ToneMapping_SetGrid0(src_idx, TableIdx);
	fwif_color_set_LC_ToneMapping_SetGrid2(src_idx, TableIdx);
	fwif_color_set_LC_ToneMapping_CurveSelect(src_idx, TableIdx);
	fwif_color_set_LC_ToneMapping_Blend(src_idx, TableIdx);
	fwif_color_set_LC_Shpnr_Gain1st(src_idx, TableIdx);
	fwif_color_set_LC_Shpnr_Gain2nd(src_idx, TableIdx);
	fwif_color_set_LC_Diff_Ctrl0(src_idx, TableIdx);

	if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
	{
		//juwen, add LC : tone mapping curve
		fwif_color_set_LC_ToneMappingSlopePoint(src_idx, TableIdx);
		//juwen, k5lp
	}
	fwif_color_set_LC_saturation(src_idx, TableIdx);
	//fwif_color_set_LC_flicker(src_idx, TableIdx); // remove at K7Lp
	fwif_color_set_LC_decontour(src_idx, TableIdx, 2); // default set to middle

	/* enable adaptive LC while LC items is exist in PQA table*/
	VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_LC] = 1;

	return;
}

void Scaler_Set_Dynamic_SLC_Table(unsigned char table_idx)
{

	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;

	fwif_color_Set_SLC_Histogram(table_idx);
	fwif_color_Init_SLC_Curve(table_idx);
	drvif_color_set_SLC_Hist_Enable(1);

	pTable->DynamicSLCTable = table_idx;
}

unsigned char Scaler_Get_Dynamic_SLC_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	
	return pTable->DynamicSLCTable;
}

void Scaler_Set_Dynamic_SLC_Enable(unsigned char enable)
{

	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;

	if (enable) {
		if (pTable->DynamicSLCEnable == 0) {
			fwif_color_Init_SLC_Curve(Scaler_Get_Dynamic_SLC_Table());
		}
		drvif_color_set_SLC_Hist_Enable(1);
		fwif_color_Set_SLC_Histogram(Scaler_Get_Dynamic_SLC_Table());
	}

	drvif_color_set_SLC_Curve_Enable(enable);

	pTable->DynamicSLCEnable = enable;
}

unsigned char Scaler_Get_Dynamic_SLC_Enable(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	
	return pTable->DynamicSLCEnable;
}

void Scaler_Set_ColorMapping_3x3_Table(unsigned char table_idx)
{

	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;

	fwif_color_set_ColorMap_3x3_Table(table_idx);

	pTable->ColorMapping = table_idx;
}

unsigned char Scaler_Get_ColorMapping_3x3_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	
	return pTable->ColorMapping;
}

int Scaler_GetRGBPixelInfo(RGB_pixel_info *data)
{
	return fwif_color_get_rgb_pixel_info(data);
}
/*==========================================*/
/*======= DDI API interface ==================*/







/*=========================================*/


/*===========================================*/
/*========== no used fw API, for Custom or Eng. menu =====*/
/*===========================================*/

#ifdef BUILD_TV057_1_ATV
static unsigned char ucEnterDcr_Panasonic_Flag = _FALSE;

void fwif_set_dcr_panasonic_apply(unsigned char value)
{
	int ret;

#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_Set_Dcr_Panasonic), &value, sizeof(unsigned char));
#else
	memcpy((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_Set_Dcr_Panasonic), &value, sizeof(unsigned char));
#endif
	if (0 != (Scaler_SendRPC(SCALERIOC_Set_Dcr_Panasonic, ShareMemAddrDummy, RPC_Send_Addr))) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "ret = %d, set Scaler_Set_dcr_panasonic to driver fail !!!\n", ret);
		return;
	}
}

unsigned char fwif_get_dcr_panasonic(void)
{
	return ucEnterDcr_Panasonic_Flag;
}

void fwif_set_dcr_panasonic(unsigned char value)
{
	VIPprintf("fwif_set_dcr_panasonic, value = %d\n", value);

	/*USER:LewisLee DATE:2013/01/10*/
	/*to prevent have other function use smart fit at same time*/
	/*Ex : DCR in Video CPU, YPbPr/VGA in Main CPU*/
	/*will let smart fit error*/
	if (0 == value) {
		/*DCR Off*/
		Scaler_Set_SmartFit_Be_Used_At_SameTime_Flag(_FALSE);
		ucEnterDcr_Panasonic_Flag = _FALSE;
	} else {/* if (1 == value)*/
		/*DCR On*/
		Scaler_Set_SmartFit_Be_Used_At_SameTime_Flag(_TRUE);
		ucEnterDcr_Panasonic_Flag = _TRUE;
	}

	fwif_set_dcr_panasonic_apply(value);
}

#endif

/*==========================================*/

void Scaler_color_handler(void)
{
	fwif_color_handler();
}

static SLR_VIP_TABLE_CUSTOM_TV001 gVip_Table_Custom;
SLR_VIP_TABLE_CUSTOM_TV001 *g_Share_Memory_VIP_TABLE_Custom_Struct;

void Scaler_SetPictureMode(unsigned char level)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;
	pData->mode = (PICTURE_MODE)level;

	rtd_printk(KERN_DEBUG, TAG_NAME, "Scaler_SetPictureMode, %d\n", level);

	fwif_color_set_set_picture_mode(src_idx, level);/* send to share Memory*/

	/*== get current picture mode OSD data ==*/

	rtd_printk(KERN_DEBUG, TAG_NAME, "Scaler_set_picture_mode = %d, value = %d ++++++@@\n", pData->mode, level);
	rtd_printk(KERN_DEBUG, TAG_NAME, "\nset_picture_mode:::src_idx = %d \n", src_idx);
	rtd_printk(KERN_DEBUG, TAG_NAME, "set_picture_mode:::Contrast = %d \n", pData->Contrast);
	rtd_printk(KERN_DEBUG, TAG_NAME, "set_picture_mode:::Brightness = %d \n", pData->Brightness);
	rtd_printk(KERN_DEBUG, TAG_NAME, "set_picture_mode:::Saturation = %d \n", pData->Saturation);
	rtd_printk(KERN_DEBUG, TAG_NAME, "set_picture_mode:::Hue = %d \n", pData->Hue);
	rtd_printk(KERN_DEBUG, TAG_NAME, "set_picture_mode:::Sharpness = %d \n", pData->Sharpness);
	rtd_printk(KERN_DEBUG, TAG_NAME, "set_picture_mode:::BackLight = %d \n", pData->BackLight);
	rtd_printk(KERN_DEBUG, TAG_NAME, "set_picture_mode:::DNR = %d \n", pData->DNR);
	rtd_printk(KERN_DEBUG, TAG_NAME, "set_picture_mode:::DccMode = %d \n", pData->DccMode);


	/*set HSBC*/
	/* this path(contrast, brightness, saturation) need to remove future. using "Scaler_SetContrast, Scaler_SetBrightness, Scaler_SetSaturation" directly*/
	/*Scaler_setcontrastbrightness_style(_MAIN_DISPLAY); */ /*set OSD contrast and brightness*/
	Scaler_SetContrast(pData->Contrast);
	Scaler_SetBrightness(pData->Brightness);
	Scaler_SetSaturation(pData->Saturation);
	/*Scaler_seticmhuesat_style(_MAIN_DISPLAY); */ /*set OSD contrast and brightness */ /* need set hue, can't not mark, elieli*/
	Scaler_SetHue(pData->Hue);

	fwif_color_RHAL_SetDNR(0); /* Init DNR*/
	/* set NR*/
	Scaler_Set_NR_style(1, CALLED_NOT_BY_OSD);  /*rock_rau :: NR for default*/
	Scaler_SetMPEGNR(pData->MPEG, CALLED_NOT_BY_OSD);
	Scaler_SetDNR(pData->DNR);

	/*rtd_printk(KERN_DEBUG, TAG_NAME, "OKOKOKOKOKOKOKOKOKOKOK\n");*/
	/*return;*/

	/*set sharpness*/
	Scaler_SetSharpness(pData->Sharpness);/*set OSD sharpness*/

	/*set DCC*/
	fwif_color_set_dcc_mode(0, 4);
	Scaler_set_dcc_Sync_DCC_AP_Level(0, Scaler_GetDCC_Table(), 4);
	Scaler_SetDCC_Mode(pData->DccMode); /* set dcc mode*/

	/*set Backlight*/
	#if 1
	//Panel_SetBackLightLevel(BACKLIGHT_USER, pData->BackLight);
	Scaler_SetBackLight(pData->BackLight);
	//Scaler_SetOSDBacklight2VIP(pData->BackLight);		/*20120614 roger*/
	Scaler_SetDcrMode(pData->DCRMode);
	#endif

	/*set Color temp*/
	Scaler_SetColorTemp((SLR_COLORTEMP_LEVEL)scaler_get_color_temp_level_type(src_idx)); /*need to fix CSFC*/

	/* set film*/
	Scaler_SetFilm_Setting();

 	/* set CLEAR WHITE GAIN*/
	//fwif_color_set_YUV2RGB_UV_Offset(src_idx, display, 3/*high*/);	//20150504 roger add

	/*Send RPC*/
	Scaler_set_picmode_init_sendRPC();

}

unsigned char Scaler_GetPictureMode()
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return VIP_COLORSTD_DEFAULT;
	return pData->mode;
}


void Scaler_ResetPictureQuality(SCALER_DISP_CHANNEL display)
{
	unsigned short sourceIndex = GET_UI_SOURCE_FROM_DISPLAY(display);
	#if 0
	unsigned char overlap = 0;
	if (display == SLR_SUB_DISPLAY) { /* added by hsliao 20090522*/
		if (!((Scaler_PipGetInfo(SLR_PIP_TYPE) >= SUBTYPE_PIPMLSR) && (Scaler_PipGetInfo(SLR_PIP_TYPE) <= SUBTYPE_PIPMLSR_4x3))) {
			overlap = 1;
		}
	}
	#endif
	/*Scaler_setcontrastbrightness_style(display);*/
	Scaler_SetContrast(Scaler_GetContrast());
	Scaler_SetBrightness(Scaler_GetBrightness());
	/*Scaler_seticmhuesat_style(display);*/
	Scaler_SetSaturation(Scaler_GetSaturation());
	Scaler_SetHue(Scaler_GetHue());

	Scaler_SetSharpness(Scaler_GetSharpness());
	Scaler_SetColorTemp((SLR_COLORTEMP_LEVEL)scaler_get_color_temp_level_type(sourceIndex));

	Scaler_SetDNR(Scaler_GetDNR());

	Scaler_SetDCC_Mode(Scaler_GetDCC_Mode());

}


void Scaler_RefreshPictureMode(void)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned char src_idx = 0;
	unsigned char display = 0;

	Scaler_Get_Display_info(&display, &src_idx);


	if (VIP_system_info_structure_table == NULL)
		return;

	if (display == SLR_SUB_DISPLAY)
		return;

	fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);
	Scaler_SetSharpness(Scaler_GetSharpness());
	Scaler_SetDNR(Scaler_GetDNR());
	Scaler_SetMPEGNR(Scaler_GetMPEGNR(), CALLED_NOT_BY_OSD);
	Scaler_SetDCC_Mode(Scaler_GetDCC_Mode());
	Scaler_Set_CinemaMode_PQ(Scaler_Get_CinemaMode_PQ());

#if 0 //fixme, lose some API
	if (g_tv002_demo_fun_flag) {
		Scaler_set_sGamma(Scaler_get_sGamma());
		Scaler_set_BLE(Scaler_get_BLE());
		//Scaler_set_LiveColor(Scaler_get_LiveColor());
		fwif_color_set_clarity_resolution();
	}


	// for Dolby HDR
	if (VIP_system_info_structure_table->DolbyHDR_flag && g_pq_bypass_lv == 9)
		Scaler_color_Set_PQ_ByPass_Lv(9);
	else if (VIP_system_info_structure_table->HDR_flag)
		Sacler_color_Netflix_NTS_HDR10_patch();
#endif
}


unsigned char Scaler_GetContrast(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;

	return pData->Contrast;
}

/*range 0~100*/
void Scaler_SetContrast(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;

	pData->Contrast = value;

	/* this path need to remove future. using "fwif_color_setContrast" directly*/
	/*Scaler_setcontrastbrightness_style(display); */ /*20140115, CSFC. TO_DO : Need to modify set con/bri Driver*/
	fwif_color_setContrast(src_idx, display, value);

}

unsigned char Scaler_GetBrightness(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;
	return pData->Brightness;
}

void Scaler_SetBrightness(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;

	pData->Brightness = value;

	/* this path need to remove future. using "fwif_color_setBrightness" directly*/
	/*Scaler_setcontrastbrightness_style(display); */ /*20140115, CSFC. TO_DO : Need to modify set con/bri Driver*/
	fwif_color_setBrightness(src_idx, display, value);

}

unsigned char Scaler_GetSaturation(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
	 return 0;

	return pData->Saturation;
}

/*range 0~100*/
void Scaler_SetSaturation(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
	 return;

	pData->Saturation = value;

	/* this path need to remove future. using "fwif_color_setSaturation" directly*/
	/*Scaler_seticmhuesat_style(display);*/
	fwif_color_setSaturation(src_idx,  display, value);

}

unsigned char Scaler_GetHue(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 50;

	return pData->Hue;
}

/*range 0~100*/
void Scaler_SetHue(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;

	pData->Hue = value;

	/*Scaler_seticmhuesat_style(display);*/
	fwif_color_setHue(src_idx, display, value);

}

unsigned char Scaler_GetSharpness(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;

	Scaler_Get_Display_info(&display, &src_idx);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;
	return pData->Sharpness;
}

/*User OSD Setting : OSD range 0~100*/
void Scaler_SetSharpness(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	rtd_printk(KERN_DEBUG, TAG_NAME, "Scaler_SetSharpness ==> %d", value);
	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;
	pData->Sharpness = value;

	fwif_color_set_sharpness_level(Scaler_GetSharpnessTable(), value);
}

unsigned char Scaler_Get_DCR_Backlight(void)
{
	_clues *smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	if(smartPic_clue == NULL){
		return Scaler_GetBacklight();
	}

	return smartPic_clue->DCR_GDBC_mappingValue;//Range : 0~255
}

unsigned char Scaler_SetBackLight(unsigned char level)
{
	//SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	fwif_color_set_backlight(src_idx, level);
	return TRUE;
}

unsigned char Scaler_GetBacklight(void)
{
	//SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	return fwif_color_get_backlight(src_idx);
}

SLR_COLORTEMP_LEVEL Scaler_GetColorTemp(void)
{
	return (SLR_COLORTEMP_LEVEL) scaler_get_color_temp_level_type(GET_USER_INPUT_SRC());
}

/* level = 0, 						USER,	*/ /*yy*/
/* level = 1, R = 0x80, G = 0x80, B = 0x80			NORMAL,	*/ /*std*/
/* level = 2, R = 0x9a, G = 0x80, B = 0x9b			WARMER,	*/ /*6500K*/
/* level = 3, R = 0xa7, G = 0x80, B = 0xbc			WARM,	*/ /*7300K*/
/* level = 4, R = 0x91, G = 0x7d, B = 0xc9			COOL,	*/ /*8200K*/
/* level = 5, R = 0x91, G = 0x80, B = 0xef			COOLER,	*/ /*9300K*/

void Scaler_SetColorTemp(SLR_COLORTEMP_LEVEL level)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table->OSD_Info.OSD_colorTemp = level;

	/*20140422 roger*/
	/*set RGB gain/offset, ===========================*/

	fwif_color_setrgbcontrast_By_Table(Scaler_get_color_temp_r_type(src_idx), Scaler_get_color_temp_g_type(src_idx), Scaler_get_color_temp_b_type(src_idx), 0);
	fwif_color_setrgbbrightness_By_Table(Scaler_get_color_temp_r_offset(src_idx), Scaler_get_color_temp_g_offset(src_idx), Scaler_get_color_temp_b_offset(src_idx));
	/*==================================================*/

	/*set gamma curve*/
	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
    rtd_printk(KERN_EMERG, TAG_NAME, "[%s:%d]\n", __FUNCTION__, __LINE__);
	fwif_set_gamma(pTable->Gamma, pTable->Sindex, pTable->Shigh, pTable->Slow);

}




unsigned char Scaler_PipGetSubBrightness(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	/*Scaler_Get_Display_info(&display, &src_idx);*/
	display = SLR_SUB_DISPLAY; /* fix me !! need call SW get display main/sub info.*/
	src_idx = GET_UI_SOURCE_FROM_DISPLAY(display);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;
	return pData->Contrast;
}

void Scaler_PipSetSubBrightness(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);
	display = SLR_SUB_DISPLAY; /* fix me !! need call SW get display main/sub info.*/

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;

	pData->Contrast = value;

	fwif_color_setBrightness(src_idx, display, value); /*20140115, CSFC. TO_DO : Need to modify set con/bri Driver*/
}

unsigned char Scaler_PipGetSubContrast(void)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	/*Scaler_Get_Display_info(&display, &src_idx);*/
	display = SLR_SUB_DISPLAY; /* fix me !! need call SW get display main/sub info.*/
	src_idx = GET_UI_SOURCE_FROM_DISPLAY(display);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return 0;
	return pData->Brightness;
}

/*range 0~100*/
void Scaler_PipSetSubContrast(unsigned char value)
{
	SLR_PICTURE_MODE_DATA *pData = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	/*Scaler_Get_Display_info(&display, &src_idx);*/
	display = SLR_SUB_DISPLAY; /* fix me !! need call SW get display main/sub info.*/
	src_idx = GET_UI_SOURCE_FROM_DISPLAY(display);

	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;

	pData->Contrast = value;

	/*Scaler_setcontrastbrightness_style(display); */ /*20140115, CSFC. TO_DO : Need to modify set con/bri Driver*/
	fwif_color_setContrast(src_idx, display, value);
}

void Scaler_Load_VipTable_Custom(void *ptr)
{
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "\n\n Scaler_Load_VipTable_Custom \n\n");*/

//	int ret;
	unsigned int virAddr ;
#if 0/*ndef SLR_VIP_TABLE_CUSTOM*/
	rtd_printk(KERN_DEBUG, TAG_NAME, "\n\n not defined VipTable_Custom \n\n");
	return;
#endif

	if (ptr == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "load custom pq table error\n");
		return;
	}
	memcpy(&gVip_Table_Custom, ptr, sizeof(SLR_VIP_TABLE_CUSTOM_TV001));

	VIPprintf("=== load_vip_table ===\n");

	/* Initial Share Memory*/
	virAddr = Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);
	Scaler_Share_Memory_Access_VIP_TABLE_Custom_Struct(virAddr);
}

void Scaler_Init_ColorLib_customer_share_memory_struct(void)
{
	/*ToDo : add init share memory code here.*/
}

/*Elsie 20131211: sync from Mac2*/
static UINT8 DI_Force_2D_Enable = _DISABLE;

/*USER:LewisLee DATE:2013/07/24*/
/*for special case, DTV 1080i 60Hz*/
/*In some stream, audio have noise*/
/*need force DI to 2D mode*/
UINT8 Scaler_Get_DI_Force_2D_Enable(void)
{
	return DI_Force_2D_Enable;
}

void Scaler_Set_DI_Force_2D_Enable(UINT8 ucEnable)
{
	DI_Force_2D_Enable = ucEnable;
}

UINT8 Scaler_Enter_DI_Force_2D_Condition(UINT8 ucDisplay)
{
	UINT8 ucInpitSource = _SRC_MAX;
	UINT8 Force_2D_Flag = _DISABLE;

	if (_DISABLE == DI_Force_2D_Enable)
		return _DISABLE;

	if (_FALSE == Scaler_DispGetStatus((SCALER_DISP_CHANNEL)ucDisplay, SLR_DISP_THRIP))
		return _DISABLE;

	ucInpitSource = Scaler_InputSrcGetType((SCALER_DISP_CHANNEL)ucDisplay);

	if (_SRC_VO == ucInpitSource) {
		switch (Scaler_DispGetInputInfoByDisp(ucDisplay, SLR_INPUT_MODE_CURR)) {
		case _MODE_1080I25:
		case _MODE_1080I30:
			Force_2D_Flag = _ENABLE;
		break;

		default:
			Force_2D_Flag = _DISABLE;
		break;
		}
	}

	return Force_2D_Flag;
}

unsigned short Scaler_GetMAG_H_Region_Start(void)
{
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;

	main_active_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);

	return main_active_h_start_end_reg.mh_act_sta;
}

unsigned short Scaler_GetMAG_H_Region_End(void)
{
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;

	main_active_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);

	return main_active_h_start_end_reg.mh_act_end;
}

unsigned short Scaler_GetMAG_H_Region_Width(void)
{
	ppoverlay_main_active_h_start_end_RBUS main_active_h_start_end_reg;

	main_active_h_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg);

	return main_active_h_start_end_reg.mh_act_end - main_active_h_start_end_reg.mh_act_sta;
}

unsigned short Scaler_GetMAG_V_Region_Start(void)
{
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;

	main_active_v_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	return main_active_v_start_end_reg.mv_act_sta;
}

unsigned short Scaler_GetMAG_V_Region_End(void)
{
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;

	main_active_v_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	return main_active_v_start_end_reg.mv_act_end;
}

unsigned short Scaler_GetMAG_V_Region_Len(void)
{
	ppoverlay_main_active_v_start_end_RBUS main_active_v_start_end_reg;

	main_active_v_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	return main_active_v_start_end_reg.mv_act_end - main_active_v_start_end_reg.mv_act_sta;
}

unsigned char Scaler_GetMagicPicture_demo_reset(void)
{
	return pstMagicPicInfo->magic_picture;
}


void Scaler_SetMagicPicture_demo_reset(unsigned char level)
{
	if (level == 0)
		MagicPicture_cnt = 0;

	pstMagicPicInfo->magic_picture = MagicPicture_cnt;

	mScalerSetMAG();
}

void Scaler_SetMagicPicture(SLR_MAGIC_TYPE level)
{
//	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (pstMagicPicInfo->magic_picture == level)
		return;


	pstMagicPicInfo->magic_picture = level;

	mScalerSetMAG();
}


/*======================= for magic picture ======================*/

void Scaler_Full_Gumat_Demo(void)
{
        /* remove hard code */
        /* if you need please implement it by yourself */
}
void Scaler_Local_Dimming_Demo_init(void)
{
        /* remove hard code */
        /* if you need please implement it by yourself */
}

void Scaler_Local_Dimming_Demo_Marquee(void)
{
        /* remove hard code */
        /* if you need please implement it by yourself */
}

void Scaler_Local_Dimming_Demo_Track(void)
{
        /* remove hard code */
        /* if you need please implement it by yourself */
}
unsigned int tmp_Contrast_A2a = 0x00808080;
unsigned int tmp_Contrast_A = 0x00808080;

typedef enum {
	_NORMAL_MODE = 0,
	_MAGIC_MODE,
} MagicModeDef;

MagicModeDef magic_mode = _NORMAL_MODE; /* 0 nomal mode , 1 magic mode*/

void mScalerSetMAG(void)
{
	con_bri_d_overlay_contrast_a2a_RBUS d_overlay_contrast_a2a_reg;
	color_d_vc_global_ctrl_RBUS d_vc_global_ctrl_reg;
	color_d_vc_global_ctrl_0_RBUS d_vc_global_ctrl_0_reg;
	gamma_gamma_ctrl_2_RBUS gamma_ctrl_2_REG;
	inv_gamma_inv_gamma_ctrl_2_RBUS inv_gamma_ctrl_2_REG;
	con_bri_dm_contrast_a_RBUS dm_contrast_a_reg;
	UINT16 _a2a_r, _a2a_g, _a2a_b; /*sync from AT*/
	/*static char mode_cnt;*/
	color_dcc_d_dcc_ctrl_RBUS	 	color_dcc_d_dcc_ctrl_reg;

	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*==== sync from AT  =======*/
	dm_contrast_a_reg.regValue = IoReg_Read32(CON_BRI_DM_CONTRAST_A_reg);
	_a2a_r = dm_contrast_a_reg.m_con_a_r;
	_a2a_g = dm_contrast_a_reg.m_con_a_g;
	_a2a_b = dm_contrast_a_reg.m_con_a_b;
	if (_a2a_r == 0)
		_a2a_r = 512;
	if (_a2a_g == 0)
		_a2a_g = 512;
	if (_a2a_b == 0)
		_a2a_b = 512;
	/*============================*/

	/*USER:LewisLee DATE:2010/10/06*/
	/*if we adjust H/V size, we need re-calculate zoom step parameter*/
	rtd_printk(KERN_DEBUG, TAG_NAME, "mScalerSetMAG()= %d\n", pstMagicPicInfo->magic_picture);

	Scaler_MagicPictureInit();

	switch (pstMagicPicInfo->magic_picture) {
	case SLR_MAGIC_OFF:/* off*/
		/*rtd_printk(KERN_DEBUG, TAG_NAME, "SLR_MAGIC_OFF\n");*/
		if (GET_MAGIC_PERIOD_STATUS() == MAGIC_IN_FUNCTION) {/*if still in magic function, send osd msg to make sure not in magic function*/
/*				Scaler_PipSetMode(Scaler_PipGetMode());*/
			SET_MAGIC_PIP_TYPE(SLR_MAGIC_OFF);
		}
		mScalerMagicStoreContrast();
		fw_scaler_disable_measure_subwdg_magicpic();
		if (pstMagicPicInfo->pre_magic_pic)   /*if magic is not on, don't call color off*/
			mScalerMagicPictureColorOff();
		mScalerDisableMagicPicture();

		/*USER: Vincent_Lee  DATE: 2010/10/12  TODO: Add 3 mode(still, optimize, enhance)*/
		/*Scaler_MagicSaturation(SLR_MAIN_DISPLAY, SLR_MAGIC_OFF);*/

		d_vc_global_ctrl_reg.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
		d_vc_global_ctrl_0_reg.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_0_reg);

		color_dcc_d_dcc_ctrl_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_CTRL_reg);
		gamma_ctrl_2_REG.regValue = IoReg_Read32(GAMMA_GAMMA_CTRL_2_reg);
		inv_gamma_ctrl_2_REG.regValue = IoReg_Read32(INV_GAMMA_INV_GAMMA_CTRL_2_reg);

		d_vc_global_ctrl_0_reg.s_vc_en = 0;
		d_vc_global_ctrl_0_reg.write_enable_18 =1;
		color_dcc_d_dcc_ctrl_reg.dcc_main_mode_sel = 1;
		gamma_ctrl_2_REG.gamma_m_tab_sel = 1;
		inv_gamma_ctrl_2_REG.inv_gamma_m_tab_sel = 1;

		#ifdef CONFIG_DUAL_CHANNEL
			d_vc_global_ctrl_0_reg.s_vc_en = 0;
			color_dcc_d_dcc_ctrl_reg.dcc_overlay = 0;
			color_dcc_d_dcc_ctrl_reg.dcc_sub_mode_sel = 0;
			gamma_ctrl_2_REG.gamma_s_tab_sel = 0;
			inv_gamma_ctrl_2_REG.inv_gamma_s_tab_sel = 0;
		#endif

		      //====wcg demo============
			 dm_contrast_a_reg.m_con_a_r=_a2a_r;
	               dm_contrast_a_reg.m_con_a_g=_a2a_g;
	               dm_contrast_a_reg.m_con_a_b=_a2a_b;
	       IoReg_Write32(CON_BRI_DM_CONTRAST_A_reg, dm_contrast_a_reg.regValue);
		IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
		IoReg_Write32(COLOR_D_VC_Global_CTRL_0_reg, d_vc_global_ctrl_0_reg.regValue);

		IoReg_Write32(COLOR_DCC_D_DCC_CTRL_reg, color_dcc_d_dcc_ctrl_reg.regValue);
		IoReg_Write32(GAMMA_GAMMA_CTRL_2_reg, gamma_ctrl_2_REG.regValue);

		pstMagicPicInfo->pre_magic_pic = SLR_MAGIC_OFF; /*record previous state*/

		break;

	case SLR_MAGIC_STILLDEMO:	/*still*/
	case SLR_MAGIC_STILLDEMO_INVERSE:
	case SLR_MAGIC_FULLSCREEN:
	case SLR_MAGIC_WCG_STILLDEMO_INVERSE:
		pstMagicPicInfo->magic_direction = _TRUE;
		if (pstMagicPicInfo->magic_picture == SLR_MAGIC_FULLSCREEN) {

			pstMagicPicInfo->magic_XStartForDynamic = Scaler_GetMAG_H_Region_Start();
			pstMagicPicInfo->magic_XEndForDynamic = Scaler_GetMAG_H_Region_End();

		} else if ((pstMagicPicInfo->magic_picture == SLR_MAGIC_STILLDEMO_INVERSE) || (pstMagicPicInfo->magic_picture == SLR_MAGIC_STILLDEMO)||(pstMagicPicInfo->magic_picture ==SLR_MAGIC_WCG_STILLDEMO_INVERSE)) {

			pstMagicPicInfo->magic_XStartForDynamic = Scaler_GetMAG_H_Region_Width()/2 + Scaler_GetMAG_H_Region_Start();
			pstMagicPicInfo->magic_XEndForDynamic = Scaler_GetMAG_H_Region_End();
		}

		/*pstMagicPicInfo->magic_XStartForDynamic = Scaler_GetMAG_H_Region_Width()/2 + Scaler_GetMAG_H_Region_Start();*/
		/*pstMagicPicInfo->magic_XEndForDynamic = Scaler_GetMAG_H_Region_End();*/
		pstMagicPicInfo->magic_YStartForDynamic = Scaler_GetMAG_V_Region_Start();
		pstMagicPicInfo->magic_YEndForDynamic = Scaler_GetMAG_V_Region_End();

		mScalerDisableMagicPicture();
		/*while (!IoReg_Read32(HD_SEM_reg));*/
		/*while (!IoReg_Read32(HD_SEM_reg));*/
		d_overlay_contrast_a2a_reg.regValue = IoReg_Read32(CON_BRI_D_Overlay_CONTRAST_A2A_reg);
		d_overlay_contrast_a2a_reg.v_con_a2a_b = _a2a_b - 32;/*sync from  */ /*0x70;*/
		d_overlay_contrast_a2a_reg.v_con_a2a_g = _a2a_g - 32;/*sync from  */ /*0x70;*/
		d_overlay_contrast_a2a_reg.v_con_a2a_r = _a2a_r - 32;/*sync from  */ /*0x70;*/
		IoReg_Write32(CON_BRI_D_Overlay_CONTRAST_A2A_reg, d_overlay_contrast_a2a_reg.regValue);
		/*IoReg_Write32(HD_SEM_reg, 0);*/

		d_vc_global_ctrl_reg.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
		d_vc_global_ctrl_0_reg.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_0_reg);

		color_dcc_d_dcc_ctrl_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_CTRL_reg);
		gamma_ctrl_2_REG.regValue = IoReg_Read32(GAMMA_GAMMA_CTRL_2_reg);
		inv_gamma_ctrl_2_REG.regValue = IoReg_Read32(INV_GAMMA_INV_GAMMA_CTRL_2_reg);

		d_vc_global_ctrl_0_reg.s_vc_en = 1;
		color_dcc_d_dcc_ctrl_reg.dcc_overlay = 1;
		if (VIP_system_info_structure_table->HDR_flag== 1) {
			if ((pstMagicPicInfo->magic_picture == SLR_MAGIC_STILLDEMO_INVERSE) || (pstMagicPicInfo->magic_picture == SLR_MAGIC_FULLSCREEN)) {

				d_vc_global_ctrl_0_reg.highlightwin_inv = 1;
				color_dcc_d_dcc_ctrl_reg.dcc_main_mode_sel = 1;
				gamma_ctrl_2_REG.gamma_m_tab_sel = 1;
				inv_gamma_ctrl_2_REG.inv_gamma_m_tab_sel = 1;

				#ifdef CONFIG_DUAL_CHANNEL
				color_dcc_d_dcc_ctrl_reg.dcc_sub_mode_sel = 2;
				gamma_ctrl_2_REG.gamma_s_tab_sel = 0;
				inv_gamma_ctrl_2_REG.inv_gamma_s_tab_sel = 0;
				#endif

			}else
                           if(pstMagicPicInfo->magic_picture ==SLR_MAGIC_WCG_STILLDEMO_INVERSE)
		                 {

		                 	dm_contrast_a_reg.m_con_a_r=dm_contrast_a_reg.m_con_a_r+30;
	                      	dm_contrast_a_reg.m_con_a_g=dm_contrast_a_reg.m_con_a_g+30;
	                       	dm_contrast_a_reg.m_con_a_b=dm_contrast_a_reg.m_con_a_b+8;

					d_vc_global_ctrl_0_reg.highlightwin_inv = 1;
					color_dcc_d_dcc_ctrl_reg.dcc_main_mode_sel = 1;
					gamma_ctrl_2_REG.gamma_m_tab_sel = 1;
					inv_gamma_ctrl_2_REG.inv_gamma_m_tab_sel = 0;

					color_dcc_d_dcc_ctrl_reg.dcc_sub_mode_sel = 1;
					gamma_ctrl_2_REG.gamma_s_tab_sel = 1;
					inv_gamma_ctrl_2_REG.inv_gamma_s_tab_sel = 0;
				      IoReg_Write32(CON_BRI_DM_CONTRAST_A_reg, dm_contrast_a_reg.regValue);


			}
			 else
                             {

				d_vc_global_ctrl_0_reg.highlightwin_inv = 0;
				color_dcc_d_dcc_ctrl_reg.dcc_main_mode_sel = 2;
				gamma_ctrl_2_REG.gamma_m_tab_sel = 0;
				inv_gamma_ctrl_2_REG.inv_gamma_m_tab_sel = 0;

				#ifdef CONFIG_DUAL_CHANNEL
				color_dcc_d_dcc_ctrl_reg.dcc_sub_mode_sel = 1;
				gamma_ctrl_2_REG.gamma_s_tab_sel = 1;
				inv_gamma_ctrl_2_REG.inv_gamma_s_tab_sel = 1;
				#endif
			}

		} else
                                 if(pstMagicPicInfo->magic_picture ==SLR_MAGIC_WCG_STILLDEMO_INVERSE)
		                 {

		                 	dm_contrast_a_reg.m_con_a_r=dm_contrast_a_reg.m_con_a_r+30;
	                      	dm_contrast_a_reg.m_con_a_g=dm_contrast_a_reg.m_con_a_g+30;
	                       	dm_contrast_a_reg.m_con_a_b=dm_contrast_a_reg.m_con_a_b+8;

					d_vc_global_ctrl_0_reg.highlightwin_inv = 1;
					color_dcc_d_dcc_ctrl_reg.dcc_main_mode_sel = 1;
					gamma_ctrl_2_REG.gamma_m_tab_sel = 1;
					inv_gamma_ctrl_2_REG.inv_gamma_m_tab_sel = 0;

					color_dcc_d_dcc_ctrl_reg.dcc_sub_mode_sel = 1;
					gamma_ctrl_2_REG.gamma_s_tab_sel = 1;
					inv_gamma_ctrl_2_REG.inv_gamma_s_tab_sel = 0;
				      IoReg_Write32(CON_BRI_DM_CONTRAST_A_reg, dm_contrast_a_reg.regValue);


			}
                         else{

			d_vc_global_ctrl_0_reg.highlightwin_inv = 1;
			color_dcc_d_dcc_ctrl_reg.dcc_main_mode_sel = 1;
			gamma_ctrl_2_REG.gamma_m_tab_sel = 1;
			inv_gamma_ctrl_2_REG.inv_gamma_m_tab_sel = 1;

			#ifdef CONFIG_DUAL_CHANNEL
			color_dcc_d_dcc_ctrl_reg.dcc_sub_mode_sel = 1;
			gamma_ctrl_2_REG.gamma_s_tab_sel = 1;
			inv_gamma_ctrl_2_REG.inv_gamma_s_tab_sel = 1;
			#endif
		}

		WaitFor_DEN_STOP();/* add wait porch to avoid line flash problem 20101019 Plank Pu*/

		IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_reg.regValue);
		IoReg_Write32(COLOR_D_VC_Global_CTRL_0_reg, d_vc_global_ctrl_0_reg.regValue);
		IoReg_Write32(COLOR_DCC_D_DCC_CTRL_reg, color_dcc_d_dcc_ctrl_reg.regValue);
		IoReg_Write32(GAMMA_GAMMA_CTRL_2_reg, gamma_ctrl_2_REG.regValue);
		if (vpq_project_id == 0x00060000 &&  vpqex_project_id == 0x00060000)
			IoReg_Write32(INV_GAMMA_INV_GAMMA_CTRL_2_reg, inv_gamma_ctrl_2_REG.regValue);

		/*mScalerMagicStoreContrast();*/
		fw_scaler_enable_measure_subwdg_magicpic();
		mScalerMagicPictureCtrlForDynamic();/*need setting before enable hl window double buffer*/
		Scaler_MoveMagicPictureForStill(0, 0);
		/*Scaler_MagicSaturation(SLR_MAIN_DISPLAY, SLR_MAGIC_STILLDEMO);*/
		pstMagicPicInfo->pre_magic_pic = pstMagicPicInfo->magic_picture;

		break;
	case SLR_MAGIC_FULLGAMUT:

			Scaler_Full_Gumat_Demo();

			break;
	case SLR_MAGIC_LOCALDIMMING_MARQUEE:
		Scaler_Local_Dimming_Demo_init();
		//ScalerTimer_ActiveTimerEvent(SLRTIMER_MAGICPICTURE, 200, (SLRTIMER_CALLBACK)Scaler_Local_Dimming_Demo_Marquee);
		break;

	case SLR_MAGIC_LOCALDIMMING_TRACK:
		//ScalerTimer_ActiveTimerEvent(SLRTIMER_MAGICPICTURE, 200, (SLRTIMER_CALLBACK)Scaler_Local_Dimming_Demo_Track);
		break;
	case SLR_MAGIC_MEMC_STILLDEMO_INVERSE:
#ifdef CONFIG_HW_SUPPORT_MEMC
		IoReg_Write32(MC_MC_60_reg, 0x01);// bit 0 enable/disable   bit 8   inside/outside  MEMC OFF

		IoReg_Write32(MC_MC_64_reg, 0x8700000);  //  bit 0~11 top0  bit 16~27  bot0
		IoReg_Write32(MC_MC_68_reg, 0x7800000);  //  bit 0~11 lft0  bit 16~27  rht0
		IoReg_Write32(KPOST_TOP_KPOST_TOP_00_reg, 0x870a77d);  //  post_cursor enable

		pstMagicPicInfo->pre_magic_pic = pstMagicPicInfo->magic_picture;
#endif
		break;

	default:
		mScalerMagicPictureColorOff();
		mScalerDisableMagicPicture();
		break;

	}
}

void Scaler_MagicPictureInit(void)
{
	rtd_printk(KERN_DEBUG, TAG_NAME, "magic_DISP_WID_OSD = %d\n", pstMagicPicInfo->magic_DISP_WID_OSD);

	if (pstMagicPicInfo->magic_DISP_WID_OSD == 1280) {
		magic_step_x =  4;
		magic_step_y =  4;
		MAG_XSize  = (Scaler_GetMAG_H_Region_Width() - (196*magic_step_x));
		MAG_YSize = (Scaler_GetMAG_V_Region_Len() - (196*magic_step_y));

		magic_zoom_step_x = 2;
		magic_zoom_step_y = 2;
		MAG_ZOOM_XSize = (Scaler_GetMAG_H_Region_Width() - (240*magic_zoom_step_x*2));
		MAG_ZOOM_YSize = (Scaler_GetMAG_V_Region_Len() - (240*magic_zoom_step_y*2));
	} else if (pstMagicPicInfo->magic_DISP_WID_OSD == 1366) {
		magic_step_x =  5;
		magic_step_y =  3;
		MAG_XSize = (Scaler_GetMAG_H_Region_Width() - (196*magic_step_x));
		MAG_YSize = (Scaler_GetMAG_V_Region_Len() - (196*magic_step_y));

		magic_zoom_step_x = 3;
		magic_zoom_step_y = 2;
		MAG_ZOOM_XSize = (Scaler_GetMAG_H_Region_Width() - (180*magic_zoom_step_x*2));
		MAG_ZOOM_YSize = (Scaler_GetMAG_V_Region_Len() - (180*magic_zoom_step_y*2));
	} else if (pstMagicPicInfo->magic_DISP_WID_OSD == 1920) {
		magic_step_x =  5;
		magic_step_y =  3;
		MAG_XSize  = (Scaler_GetMAG_H_Region_Width() - (240*magic_step_x));
		MAG_YSize = (Scaler_GetMAG_V_Region_Len() - (240*magic_step_y));

		magic_zoom_step_x = 3;
		magic_zoom_step_y = 2;
		MAG_ZOOM_XSize = (Scaler_GetMAG_H_Region_Width() - (240*magic_zoom_step_x*2));
		MAG_ZOOM_YSize = (Scaler_GetMAG_V_Region_Len() - (240*magic_zoom_step_y*2));
	} else if (pstMagicPicInfo->magic_DISP_WID_OSD == 1024) {
		magic_step_x =  3;
		magic_step_y =  3;
		MAG_XSize = (Scaler_GetMAG_H_Region_Width() - (196*magic_step_x));
		MAG_YSize = (Scaler_GetMAG_V_Region_Len() - (196*magic_step_y));

		magic_zoom_step_x = 3;
		magic_zoom_step_y = 2;
		MAG_ZOOM_XSize = (Scaler_GetMAG_H_Region_Width() - (160*magic_zoom_step_x*2));
		MAG_ZOOM_YSize = (Scaler_GetMAG_V_Region_Len() - (160*magic_zoom_step_y*2));

	} else if (pstMagicPicInfo->magic_DISP_WID_OSD == 1712) {
		magic_step_x =  5;
		magic_step_y =  3;
		MAG_XSize = (Scaler_GetMAG_H_Region_Width() - (196*magic_step_x));
		MAG_YSize = (Scaler_GetMAG_V_Region_Len() - (196*magic_step_y));

		magic_zoom_step_x = 3;
		magic_zoom_step_y = 2;
		MAG_ZOOM_XSize = (Scaler_GetMAG_H_Region_Width() - (180*magic_zoom_step_x*2));
		MAG_ZOOM_YSize = (Scaler_GetMAG_V_Region_Len() - (180*magic_zoom_step_y*2));
	} else {
		magic_step_x =  5;
		magic_step_y =  3;
		MAG_XSize = (Scaler_GetMAG_H_Region_Width() - (196*magic_step_x));
		MAG_YSize = (Scaler_GetMAG_V_Region_Len() - (196*magic_step_y));

		magic_zoom_step_x = 3;
		magic_zoom_step_y = 2;
		MAG_ZOOM_XSize = (Scaler_GetMAG_H_Region_Width() - (180*magic_zoom_step_x*2));
		MAG_ZOOM_YSize = (Scaler_GetMAG_V_Region_Len() - (180*magic_zoom_step_y*2));
	}
}

void mScalerDisableMagicPicture(void)
{
#ifdef CONFIG_DUAL_CHANNEL
	PipMpSetMpEnable(_DISABLE);
#endif
	drvif_color_set_sethighlightwindowmin(0, 0); /*set border*/
	drvif_color_sethighlightwindowstep(0, 0, 0, 0);
	drvif_color_sethighlightwindow(_DISABLE, 0, 0, 0, 0, 0);
	/*frank@07152010 add below to solve mantis28460 disable magic picture will appear sub window*/
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	drvif_scalerdisplay_enable_display(SLR_SUB_DISPLAY, _DISABLE);
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}

void mScalerMagicStoreContrast(void)
{
	if (magic_mode == _NORMAL_MODE) {
		tmp_Contrast_A2a = IoReg_Read32(CON_BRI_D_Overlay_CONTRAST_A2A_reg);
		tmp_Contrast_A = IoReg_Read32(CON_BRI_DM_CONTRAST_A_reg);
		magic_mode = _MAGIC_MODE;
	}

	IoReg_ClearBits(YUV2RGB_D_YUV2RGB_Control_reg, _BIT1 | _BIT6);

}

void mScalerMagicPictureColorOff(void)
{

/*	con_bri_dm_contrast_a_RBUS dm_contrast_a_reg;*/
/*	con_bri_d_overlay_contrast_a2a_RBUS d_overlay_contrast_a2a_reg;*/
	color_d_vc_global_ctrl_0_RBUS d_vc_global_ctrl_0_reg;

/*	rtd_printk(KERN_DEBUG, TAG_NAME, "[debug 1]%x %x\n", tmp_Contrast_A, tmp_Contrast_A2a);*/
	/*while (!IoReg_Read32(HD_SEM_reg));*/
	IoReg_Write32(CON_BRI_D_Overlay_CONTRAST_A2A_reg, tmp_Contrast_A2a); /*restore original value*/
	IoReg_Write32(CON_BRI_DM_CONTRAST_A_reg, tmp_Contrast_A); /* restore original value*/

	/*IoReg_Write32(HD_SEM_reg, 0);*/
	magic_mode = _NORMAL_MODE;

	d_vc_global_ctrl_0_reg.regValue = IoReg_Read32(COLOR_D_VC_Global_CTRL_0_reg);
	d_vc_global_ctrl_0_reg.highlightwin_inv = 0;
	d_vc_global_ctrl_0_reg.write_enable_17 =1;
	WaitFor_DEN_STOP(); /* add wait porch to avoid line flash problem 20101019 Plank Pu*/
	IoReg_Write32(COLOR_D_VC_Global_CTRL_0_reg, d_vc_global_ctrl_0_reg.regValue);
	IoReg_SetBits(YUV2RGB_D_YUV2RGB_Control_reg, _BIT1 | _BIT6);

}

void Scaler_MagicSaturation(unsigned char display, unsigned char MagicMode)
{
	short hue = 0, sat = 0;//, intensity = 0;
	unsigned char osdHue = 0, osdSat = 0;
	char temp = 0;

	/*USER: Vincent_Lee  DATE: 2011/10/26  TODO: calcu sat by factory data*/
	short startPoint = 0, endPoint = 0;
	StructColorDataFacModeType *pColorFacTable;
	/*if (scaler_2Dcvt3D_get_vgip2EnableStatus())
	{
		pColorFacTable = fwif_color_get_color_fac_mode(scaler_2Dcvt3D_get_vgip2SrcIndex(), 0);
	}
	else*/
	/*{*/
		pColorFacTable = fwif_color_get_color_fac_mode(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_DATA_ARRAY_IDX), 0);
	/*}*/
	if (pColorFacTable == NULL)
		return;


	if (display == SLR_MAIN_DISPLAY) {
		/*osdHue = fwif_color_get_hue(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_DATA_ARRAY_IDX));*/
		osdHue = Scaler_GetHue();
		temp = (short)(osdHue-50)*(-1);
		hue = (short)(temp*21/10); /*map -50~50 to -128~128*/

		/*osdSat = fwif_color_get_saturation(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_DATA_ARRAY_IDX));*/
		osdSat = Scaler_GetSaturation();
		if (MagicMode == SLR_MAGIC_ENHANCE) {
			osdSat += 40;
			if (osdSat > 100)	/*---the value must <= 100*/
				osdSat = 100;

		} else if (MagicMode == SLR_MAGIC_OFF) {
			if (osdSat <= 10)
				osdSat = 0;
			else
				osdSat -= 10;
		}
		/*rtd_printk(KERN_DEBUG, TAG_NAME, "Scaler_MagicSaturation: osdSat = %d \n", osdSat);*/
		/*sat = (short)(osdSat*2.55);*/ /*map 0~100 to 0~255*/

		/*USER: Vincent_Lee  DATE: 2011/10/26  TODO: calcu sat by factory data*/
		if (osdSat < 50) {
			startPoint = pColorFacTable->Saturation_0;
			endPoint = pColorFacTable->Saturation_50;
		} else {
			startPoint = pColorFacTable->Saturation_50;
			endPoint = pColorFacTable->Saturation_100;
			osdSat -= 50;
		}
		sat = (unsigned char)(startPoint + (endPoint-startPoint)*osdSat/50);

		/*map 0~100 to -2048~2047*/
		/*intensity = (short)((fwif_color_get_intensity(Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_DATA_ARRAY_IDX)) *40.95) -2048);*/
		//intensity = (short)((Scaler_GetIntensity() * 4095/100) - 2048);
		drvif_color_setuv_sat(display, sat);/* 20100726 modify sat adj.  from sRGB to uv, csfanchiang*/
		/*drvif_color_seticmhsi(display, hue, 0, intensity); */ /*20090305 haifu del saturation in ICM*/
		drvif_color_seticmhsi(display, hue, ICM_Global_hue); /*set ICM global hue, original only hue can work*/
	}

}

void mScalerMagicPictureCtrlForDynamic(void)
{
#ifdef CONFIG_DUAL_CHANNEL
	ppoverlay_mp_layout_force_to_background_RBUS  mp_layout_force_to_background_reg;
	ppoverlay_mp_disp_enable_RBUS mp_disp_enable_reg;
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	mp_layout_force_to_background_reg.regValue = IoReg_Read32(PPOVERLAY_MP_Layout_Force_to_Background_reg);
	//fix me later by ben
	//mp_layout_force_to_background_reg.mp_force_bg = 0x0000;/* disable force-to-background each*/
	mp_layout_force_to_background_reg.s_force_bg = 0; /*normal display*/
	mp_layout_force_to_background_reg.s_disp_en = 1;
	IoReg_Write32(PPOVERLAY_MP_Layout_Force_to_Background_reg, mp_layout_force_to_background_reg.regValue);
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	mp_disp_enable_reg.regValue = IoReg_Read32(PPOVERLAY_MP_Disp_Enable_reg);
	//fix me later by ben
	//mp_disp_enable_reg.mp_disp_en = 0x01;
	mp_disp_enable_reg.mp_hbd_en = 0;
	IoReg_Write32(PPOVERLAY_MP_Disp_Enable_reg, mp_disp_enable_reg.regValue);
	drvif_color_setpiprgbcontrastbrightness(100);

	drvif_scalerdisplay_set_bg_color(_SUB_DISPLAY, _DISPLAY_EB, 0x00, 0x7b, 0xbc); /*jacklong 2008/01/08 modify colour   0x0f, 0x00, 0x00); */ /* set external border color*/
#endif
}

void Scaler_MoveMagicPictureForStill(unsigned int timerId, void *msg)
{
	ppoverlay_highlight_window_control_RBUS vc_reg;

	drvif_color_sethighlightwindowborder(2, BORDERSTYLE_LEFT);
	drvif_color_set_sethighlightwindowmin(4, 4);
	drvif_color_sethighlightwindowstep(0, 0, 0, 0);

	drvif_color_sethighlightwindow(_ENABLE, pstMagicPicInfo->magic_XStartForDynamic, pstMagicPicInfo->magic_YStartForDynamic, pstMagicPicInfo->magic_XEndForDynamic, pstMagicPicInfo->magic_YEndForDynamic, 4);

	vc_reg.regValue = IoReg_Read32(PPOVERLAY_Highlight_window_control_reg);
	vc_reg.hlw_mode = 1;
	vc_reg.hlw_en = 1;
	IoReg_Write32(PPOVERLAY_Highlight_window_control_reg, vc_reg.regValue);
	IoReg_Write32(PPOVERLAY_Highlight_window_DB_CTRL_reg, 0x1);

	if (pstMagicPicInfo->magic_picture == SLR_MAGIC_OFF)
		mScalerDisableMagicPicture();
}


/*======================= for magic picture end ======================*/
unsigned char Scaler_getDI_SMD_en(unsigned char source)
{
	unsigned char *pVipCoefArray = NULL;

	/* Load pVipCoefArray*/
	if (m_cbFunc_getQualityCoef != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityCoef((VIP_SOURCE_TIMING) source);

	if (!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		return pVipCoefArray[VIP_QUALITY_FUNCTION_DI_SMD_Level];
	} else {
		return 0;
	}
}

void Scaler_Set3dLUT(UINT8 TableIdx)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->d3dLUT= TableIdx;

	fwif_color_set_3dLUT(TableIdx);

	if (TableIdx > 0)
		fwif_color_3d_lut_access_index(ACCESS_MODE_SET, TableIdx-1);
}
unsigned char Scaler_Get3dLUT(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->d3dLUT;
}


void Scaler_color_copy_scalerinfo_to_vip(unsigned char display)
{
	/*=== 20140421 CSFC for vip video fw infomation ====*/
	/* setting some information to sharememory*/
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	VIP_system_info_structure_table->Input_src_Type		= Scaler_InputSrcGetType((SCALER_DISP_CHANNEL)display);
	VIP_system_info_structure_table->Input_src_Form		= Scaler_InputSrcGetFrom((SCALER_DISP_CHANNEL)display);
	VIP_system_info_structure_table->Input_VO_Form		= Scaler_VOFrom(Scaler_Get_CurVoInfo_plane());
	VIP_system_info_structure_table->Input_TVD_Form		= Scaler_InputSrcGetMainFormat();
	VIP_system_info_structure_table->vdc_color_standard	= drvif_module_vdc_ReadMode(VDC_SET);
	VIP_system_info_structure_table->Timing				= Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	VIP_system_info_structure_table->HDMI_video_format	= drvif_Hdmi_GetColorSpace();
	VIP_system_info_structure_table->Display_RATIO_TYPE	= Scaler_DispGetRatioMode();
	VIP_system_info_structure_table->HDMI_data_range    = drvif_IsRGB_YUV_RANGE();

	VIP_system_info_structure_table->input_display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	VIP_system_info_structure_table->HDMI_color_space = Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE);
	VIP_system_info_structure_table->IV_Start = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA);
	VIP_system_info_structure_table->IH_Start = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA);
	VIP_system_info_structure_table->I_Height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	VIP_system_info_structure_table->I_Width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
	VIP_system_info_structure_table->Mem_Height = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN);
	VIP_system_info_structure_table->Mem_Width = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);
	VIP_system_info_structure_table->Cap_Height = Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN);
	VIP_system_info_structure_table->Cap_Width = Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
	VIP_system_info_structure_table->DI_Width = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID);
	VIP_system_info_structure_table->D_Height = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
	VIP_system_info_structure_table->D_Width = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
	VIP_system_info_structure_table->color_fac_src_idx = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY, SLR_INPUT_DATA_ARRAY_IDX);
	/*============================================*/
}

void Scaler_color_print_scalerinfo(void)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *RPC_VIP_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(RPC_VIP_system_info_structure_table == NULL || VIP_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	rtd_printk(KERN_DEBUG, TAG_NAME, "===== VIP_scaler_info =====\n");
	/*============================================*/
	rtd_printk(KERN_DEBUG, TAG_NAME, "Input_src_Type = %d\n", VIP_system_info_structure_table->Input_src_Type);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Input_src_Form = %d\n", VIP_system_info_structure_table->Input_src_Form);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Input_VO_Form = %d\n", VIP_system_info_structure_table->Input_VO_Form);
	rtd_printk(KERN_DEBUG, TAG_NAME, "input_TVD_Form = %d\n", VIP_system_info_structure_table->Input_TVD_Form);
	rtd_printk(KERN_DEBUG, TAG_NAME, "vdc_color_standard = %d\n", VIP_system_info_structure_table->vdc_color_standard);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Timing = %d\n", VIP_system_info_structure_table->Timing);
	rtd_printk(KERN_DEBUG, TAG_NAME, "HDMI_video_format = %d\n", VIP_system_info_structure_table->HDMI_video_format);
	rtd_printk(KERN_DEBUG, TAG_NAME, "VIP_source = %d\n", RPC_VIP_system_info_structure_table->VIP_source);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Display_RATIO_TYPE = %d\n", VIP_system_info_structure_table->Display_RATIO_TYPE);
	rtd_printk(KERN_DEBUG, TAG_NAME, "HDMI_data_range = %d\n", VIP_system_info_structure_table->HDMI_data_range);

	rtd_printk(KERN_DEBUG, TAG_NAME, "input_display = %d\n", VIP_system_info_structure_table->input_display);
	rtd_printk(KERN_DEBUG, TAG_NAME, "HDMI_color_space = %d\n", VIP_system_info_structure_table->HDMI_color_space);
	rtd_printk(KERN_DEBUG, TAG_NAME, "IV_Start = %d\n", VIP_system_info_structure_table->IV_Start);
	rtd_printk(KERN_DEBUG, TAG_NAME, "IH_Start = %d\n", VIP_system_info_structure_table->IH_Start);
	rtd_printk(KERN_DEBUG, TAG_NAME, "I_Height = %d\n", VIP_system_info_structure_table->I_Height);
	rtd_printk(KERN_DEBUG, TAG_NAME, "I_Width = %d\n", VIP_system_info_structure_table->I_Width);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Mem_Height = %d\n", VIP_system_info_structure_table->Mem_Height);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Mem_Width = %d\n", VIP_system_info_structure_table->Mem_Width);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Cap_Height = %d\n", VIP_system_info_structure_table->Cap_Height);
	rtd_printk(KERN_DEBUG, TAG_NAME, "Cap_Width = %d\n", VIP_system_info_structure_table->Cap_Width);
	rtd_printk(KERN_DEBUG, TAG_NAME, "DI_Width = %d\n", VIP_system_info_structure_table->DI_Width);
	rtd_printk(KERN_DEBUG, TAG_NAME, "D_Height = %d\n", VIP_system_info_structure_table->D_Height);
	rtd_printk(KERN_DEBUG, TAG_NAME, "D_Width = %d\n", VIP_system_info_structure_table->D_Width);
	rtd_printk(KERN_DEBUG, TAG_NAME, "color_fac_src_idx = %d\n", VIP_system_info_structure_table->color_fac_src_idx);
	/*============================================*/
	rtd_printk(KERN_DEBUG, TAG_NAME, "===== VIP_scaler_info end =====\n");

}

/*static unsigned char facPatternColorLevel;*/
void Scaler_SetFacPatternColor(unsigned char val)
{
#if 0	//fix me later by ben
	unsigned int temp;
	static unsigned int old;
	static unsigned char saved;

	facPatternColorLevel = val;

	switch (val) {
	case SCALERLIB_PatternColor_OFF:	/*close*/
		if (saved) {
			IoReg_Write32(SCALEUP_DM_UZU_Input_Size_reg, old);  /*restore origin*/
			saved = 0;
		}

		IoReg_SetBits(SCALEUP_DM_UZU_Ctrl_reg, _BIT9);
		IoReg_ClearBits(SCALEUP_D_UZU_Globle_Ctrl_reg, _BIT0|_BIT1);
		/*Scaler_ForceBg(_MAIN_DISPLAY, SLR_FORCE_BG_SYSTEM, _DISABLE);	*/ /*=== marked by Elsie ===*/
		break;
	case SCALERLIB_PatternColor_RED:	/*red*/
		drvif_scalerdisplay_set_bg_color(_MAIN_DISPLAY, _DISPLAY_BG, 0xFF, 0x00, 0x00); /* set main background color: red*/
		/*frank@0810 mask interrupt status bit IoReg_SetBits(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT1); */ /* force BG*/
		IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, (IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg) | _BIT1));
		IoReg_WriteByte2(PPOVERLAY_Display_Background_Color_reg, 0x00);	/* why??*/
		drvif_scalerdisplay_set_dbuffer_en(SLR_MAIN_DISPLAY, false);
		break;
	case SCALERLIB_PatternColor_GREEN: /*green*/
		drvif_scalerdisplay_force_bg(_MAIN_DISPLAY, _TRUE);
		IoReg_Mask32(PPOVERLAY_Main_Background_Color1_reg, 0xffff000, (AND(0, 0xf) << 8) | (AND(0xff, 0xf) << 4) | AND(0x0, 0xf));
		drvif_scalerdisplay_set_bg_color(_MAIN_DISPLAY, _DISPLAY_BG, 0x00, 0xFF, 0x00); /* set main background color: green*/
		/*frank@0810 mask interrupt status bit IoReg_SetBits(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT1); */ /* force BG*/
		IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, (IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg) | _BIT1));
		IoReg_WriteByte2(PPOVERLAY_Display_Background_Color_reg, 0x00);	/* why ??*/
		drvif_scalerdisplay_set_dbuffer_en(SLR_MAIN_DISPLAY, false);
		break;
	case SCALERLIB_PatternColor_BLUE: /*blue*/
		drvif_scalerdisplay_force_bg(_MAIN_DISPLAY, _TRUE);
		IoReg_Mask32(PPOVERLAY_Main_Background_Color1_reg, 0xffff000, (AND(0xff, 0x0f) << 8) | (AND(0, 0x0f) << 4) | AND(0xf, 0x0f));
		drvif_scalerdisplay_set_bg_color(_MAIN_DISPLAY, _DISPLAY_BG, 0x00, 0x00, 0xFF); /* set main background color: blue*/
		/*frank@0810 mask interrupt status bit IoReg_SetBits(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT1); */ /* force BG*/
		IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, (IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg) | _BIT1));
		IoReg_WriteByte2(PPOVERLAY_Display_Background_Color_reg, 0x00);	/* why ??*/
		drvif_scalerdisplay_set_dbuffer_en(SLR_MAIN_DISPLAY, false);
		break;
	case SCALERLIB_PatternColor_WHITE: /*white*/
		drvif_scalerdisplay_force_bg(_MAIN_DISPLAY, _TRUE);
		IoReg_Mask32(PPOVERLAY_Main_Background_Color1_reg, 0xffff000, (AND(0xff, 0x0f) << 8) | (AND(0, 0x0f) << 4) | AND(0xf, 0x0f));
		drvif_scalerdisplay_set_bg_color(_MAIN_DISPLAY, _DISPLAY_BG, 0xFF, 0xFF, 0xFF); /* set main background color: white*/
		/*frank@0810 mask interrupt status bit IoReg_SetBits(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT1); */ /* force BG*/
		IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, (IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg) | _BIT1));
		IoReg_WriteByte2(PPOVERLAY_Display_Background_Color_reg, 0x00);	/* why ??*/
		drvif_scalerdisplay_set_dbuffer_en(SLR_MAIN_DISPLAY, false);
		break;
	case SCALERLIB_PatternColor_BLACK:  /*black*/
		drvif_scalerdisplay_force_bg(_MAIN_DISPLAY, _TRUE);
		IoReg_Mask32(PPOVERLAY_Main_Background_Color1_reg, 0xffff000, (AND(0xff, 0x0f) << 8) | (AND(0, 0x0f) << 4) | AND(0xf, 0x0f));
		if (saved) {
			IoReg_Write32(0xb8071028, old);  /*restore origin*/
			saved = 0;
		}
		drvif_scalerdisplay_set_bg_color(_MAIN_DISPLAY, _DISPLAY_BG, 0x00, 0x00, 0x00); /* set main background color: black*/
		/*frank@0810 mask interrupt status bit IoReg_SetBits(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT1); */ /* force BG*/
		IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, (IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg) | _BIT1));
		IoReg_WriteByte2(PPOVERLAY_Display_Background_Color_reg, 0x00);	/* why ??*/
		drvif_scalerdisplay_set_dbuffer_en(SLR_MAIN_DISPLAY, false);
		break;
	case SCALERLIB_PatternColor_COLORBAR: /*color bar*/
		if (!saved) {
			old = IoReg_Read32(SCALEUP_DM_UZU_Input_Size_reg);
			saved = 1;
		}
		temp = (IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg) & 0x00000fff)-((IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg) & 0x0fff0000)>>16); /*width*/
	/*	rtd_printk(KERN_DEBUG, TAG_NAME, "Hstart: %d, Hend: %d\n", (IoReg_Read32(DDOMAIN_MAIN_ACTIVE_H_START_END_VADDR) & 0x000007ff), ((IoReg_Read32(DDOMAIN_MAIN_ACTIVE_H_START_END_VADDR) & 0x07ff0000)>>16));*/
		rtd_printk(KERN_DEBUG, TAG_NAME, "0xb8071028 = %x\n", (temp << 16) | ((IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg) & 0x000007ff)-((IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg) & 0x07ff0000)>>16)));
		IoReg_Write32(SCALEUP_DM_UZU_Input_Size_reg, (temp << 16) | ((IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg) & 0x00000fff)-((IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg) & 0x0fff0000)>>16))); /* set width and length to UZU*/
		temp = ((temp/16-1) << 16) | 0x31;
	/*	rtd_printk(KERN_DEBUG, TAG_NAME, "0xb8071000 = %x\n", temp);*/
		IoReg_Write32(SCALEUP_D_UZU_Globle_Ctrl_reg, temp); /* internal pattern gen*/
		/*frank@0810 mask interrupt status bit IoReg_ClearBits(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT1|_BIT15); */ /* force BG*/
		IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, ((IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg) & ~(_BIT1|_BIT15))));
		IoReg_ClearBits(SCALEUP_DM_UZU_Ctrl_reg, _BIT9); /* 444*/
		/*IoReg_Write32(DDOMAIN_MAIN_DISPLAY_CONTROL_RSV_VADDR,  IoReg_Read32(DDOMAIN_MAIN_DISPLAY_CONTROL_RSV_VADDR) & 0xFFFFFFFD);*/
		break;
	case SCALERLIB_PatternColor_GRAYBAR:  /*gray bar*/
		drvif_scalerdisplay_force_bg(_MAIN_DISPLAY, _TRUE);
		IoReg_Mask32(PPOVERLAY_Main_Background_Color1_reg, 0xffff000, (AND(0xff, 0x0f) << 8) | (AND(0, 0x0f) << 4) | AND(0xf, 0x0f));
		if (!saved) {
			old = IoReg_Read32(SCALEUP_DM_UZU_Input_Size_reg);
			saved = 1;
		}
		temp = (IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg) & 0x00000fff)-((IoReg_Read32(PPOVERLAY_MAIN_Active_H_Start_End_reg) & 0x0fff0000)>>16); /*width*/
	/*	rtd_printk(KERN_DEBUG, TAG_NAME, "Hstart: %d, Hend: %d\n", (IoReg_Read32(DDOMAIN_MAIN_ACTIVE_H_START_END_VADDR) & 0x000007ff), ((IoReg_Read32(DDOMAIN_MAIN_ACTIVE_H_START_END_VADDR) & 0x07ff0000)>>16));*/
		rtd_printk(KERN_DEBUG, TAG_NAME, "0xb8071028 = %x\n", (temp << 16) | ((IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg) & 0x000007ff)-((IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg) & 0x07ff0000)>>16)));
		IoReg_Write32(SCALEUP_DM_UZU_Input_Size_reg, (temp << 16) | ((IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg) & 0x00000fff)-((IoReg_Read32(PPOVERLAY_MAIN_Active_V_Start_End_reg) & 0x0fff0000) >> 16))); /* set width and length to UZU*/
		temp = ((temp/8-1) << 16) | 0x51;
		/*rtd_printk(KERN_DEBUG, TAG_NAME, "0xb8071000 = %x\n", temp);*/
		IoReg_Write32(SCALEUP_D_UZU_Globle_Ctrl_reg, temp); /* internal pattern gen*/
		IoReg_Write32(SCALEUP_D_UZU_PATGEN_COLOR0_reg, 0x00197788); /* change pattern gen color 0*/
		IoReg_Write32(SCALEUP_D_UZU_PATGEN_COLOR1_reg, 0x005E7788); /* change pattern gen color 1*/
		/*frank@0810 mask interrupt status bit IoReg_ClearBits(PPOVERLAY_Main_Display_Control_RSV_reg, _BIT1|_BIT15); */ /* force BG*/
		IoReg_Write32(PPOVERLAY_Main_Display_Control_RSV_reg, ((IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg) & ~(_BIT1|_BIT15))));
		IoReg_ClearBits(SCALEUP_DM_UZU_Ctrl_reg, _BIT9); /* 444*/
		/*IoReg_Write32(DDOMAIN_MAIN_DISPLAY_CONTROL_RSV_VADDR,  IoReg_Read32(DDOMAIN_MAIN_DISPLAY_CONTROL_RSV_VADDR) & 0xFFFFFFFD);*/
		break;
	default:
		break;
	}
#endif
}

#ifdef ENABLE_VIP_TABLE_CHECKSUM
/* === checksum ========== */
void Scaler_checkVIPTable(void)
{
	extern VIP_table_crc_value vip_table_crc;
	VIP_table_crc_value vip_table_crc_now;

	color_dcc_d_dcc_fwused_1_RBUS dcc_fwused_1_reg;
	dcc_fwused_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_FWUSED_1_VADDR);

	if(dcc_fwused_1_reg.VipInfo == 1){
		fwif_color_check_VIPTable_crc(&vip_table_crc_now, fwif_color_GetShare_Memory_VIP_TABLE_Struct());

		SLRLIB_PRINTF("[%s %d] checksum: vip_table %x, vip_table_now %x\n", __FILE__, __LINE__,
			vip_table_crc.checkSum, vip_table_crc_now.checkSum);
	}
	#if 0
	SLRLIB_PRINTF("vip_table_crc.VIP_QUALITY_Coef = %x, vip_table_crc_now.VIP_QUALITY_Coef = %x\n",vip_table_crc.VIP_QUALITY_Coef,vip_table_crc_now.VIP_QUALITY_Coef);
	SLRLIB_PRINTF("vip_table_crc.VIP_QUALITY_Extend_Coef = %x, vip_table_crc_now.VIP_QUALITY_Extend_Coef = %x\n",vip_table_crc.VIP_QUALITY_Extend_Coef,vip_table_crc_now.VIP_QUALITY_Extend_Coef);
	SLRLIB_PRINTF("vip_table_crc.VIP_QUALITY_Extend2_Coef = %x, vip_table_crc_now.VIP_QUALITY_Extend2_Coef = %x\n",vip_table_crc.VIP_QUALITY_Extend2_Coef,vip_table_crc_now.VIP_QUALITY_Extend2_Coef);
	SLRLIB_PRINTF("vip_table_crc.VIP_QUALITY_Extend3_Coef = %x, vip_table_crc_now.VIP_QUALITY_Extend3_Coef = %x\n",vip_table_crc.VIP_QUALITY_Extend3_Coef,vip_table_crc_now.VIP_QUALITY_Extend3_Coef);
	SLRLIB_PRINTF("vip_table_crc.VIP_QUALITY_3Dmode_Coef = %x, vip_table_crc_now.VIP_QUALITY_3Dmode_Coef = %x\n",vip_table_crc.VIP_QUALITY_3Dmode_Coef,vip_table_crc_now.VIP_QUALITY_3Dmode_Coef);
	SLRLIB_PRINTF("vip_table_crc.VIP_QUALITY_Extend4_Coef = %x, vip_table_crc_now.VIP_QUALITY_Extend4_Coef = %x\n",vip_table_crc.VIP_QUALITY_Extend4_Coef,vip_table_crc_now.VIP_QUALITY_Extend4_Coef);
	SLRLIB_PRINTF("vip_table_crc.SU_PK_Coeff = %x, vip_table_crc_now.SU_PK_Coeff = %x\n",vip_table_crc.SU_PK_Coeff,vip_table_crc_now.SU_PK_Coeff);
	SLRLIB_PRINTF("vip_table_crc.SR_init_table = %x, vip_table_crc_now.SR_init_table = %x\n",vip_table_crc.SR_init_table,vip_table_crc_now.SR_init_table);
	SLRLIB_PRINTF("vip_table_crc.SU_PK_Coeff_by_timing = %x, vip_table_crc_now.SU_PK_Coeff_by_timing = %x\n",vip_table_crc.SU_PK_Coeff_by_timing,vip_table_crc_now.SU_PK_Coeff_by_timing);
	SLRLIB_PRINTF("vip_table_crc.SD_H_table = %x, vip_table_crc_now.SD_H_table = %x\n",vip_table_crc.SD_H_table,vip_table_crc_now.SD_H_table);
	SLRLIB_PRINTF("vip_table_crc.SD_v_table = %x, vip_table_crc_now.SD_v_table = %x\n",vip_table_crc.SD_V_table,vip_table_crc_now.SD_V_table);

	SLRLIB_PRINTF("vip_table_crc.ODtable_Coeff = %x, vip_table_crc_now.ODtable_Coeff = %x\n",vip_table_crc.ODtable_Coeff,vip_table_crc_now.ODtable_Coeff);
	SLRLIB_PRINTF("vip_table_crc.DCR_TABLE = %x, vip_table_crc_now.DCR_TABLE = %x\n",vip_table_crc.DCR_TABLE,vip_table_crc_now.DCR_TABLE);
	SLRLIB_PRINTF("vip_table_crc.ICM_by_timing_picmode = %x, vip_table_crc_now.ICM_by_timing_picmode = %x\n",vip_table_crc.ICM_by_timing_picmode,vip_table_crc_now.ICM_by_timing_picmode);
	SLRLIB_PRINTF("vip_table_crc.MA_Chroma_Error_Table = %x, vip_table_crc_now.MA_Chroma_Error_Table = %x\n",vip_table_crc.MA_Chroma_Error_Table,vip_table_crc_now.MA_Chroma_Error_Table);

	SLRLIB_PRINTF("vip_table_crc.CDS_ini = %x, vip_table_crc_now.CDS_ini = %x\n",vip_table_crc.CDS_ini,vip_table_crc_now.CDS_ini);
	SLRLIB_PRINTF("vip_table_crc.D_EDGE_Smooth_Coef = %x, vip_table_crc_now.D_EDGE_Smooth_Coef = %x\n",vip_table_crc.D_EDGE_Smooth_Coef,vip_table_crc_now.D_EDGE_Smooth_Coef);
	SLRLIB_PRINTF("vip_table_crc.I_EDGE_Smooth_Coef = %x, vip_table_crc_now.I_EDGE_Smooth_Coef = %x\n",vip_table_crc.I_EDGE_Smooth_Coef,vip_table_crc_now.I_EDGE_Smooth_Coef);

	SLRLIB_PRINTF("vip_table_crc.UV_Gains_1 = %x, vip_table_crc_now.UV_Gains_1 = %x\n",vip_table_crc.UV_Gains_1,vip_table_crc_now.UV_Gains_1);
	SLRLIB_PRINTF("vip_table_crc.UV_Gains_2 = %x, vip_table_crc_now.UV_Gains_2 = %x\n",vip_table_crc.UV_Gains_2,vip_table_crc_now.UV_Gains_2);
	SLRLIB_PRINTF("vip_table_crc.xvYcc_sRGB_Curve = %x, vip_table_crc_now.xvYcc_sRGB_Curve = %x\n",vip_table_crc.xvYcc_sRGB_Curve,vip_table_crc_now.xvYcc_sRGB_Curve);

	SLRLIB_PRINTF("vip_table_crc.YUV2RGB_CSMatrix_Table = %x, vip_table_crc_now.YUV2RGB_CSMatrix_Table = %x\n",vip_table_crc.YUV2RGB_CSMatrix_Table,vip_table_crc_now.YUV2RGB_CSMatrix_Table);

	SLRLIB_PRINTF("vip_table_crc.VD_ConBriHueSat = %x, vip_table_crc_now.VD_ConBriHueSat = %x\n",vip_table_crc.VD_ConBriHueSat,vip_table_crc_now.VD_ConBriHueSat);
	SLRLIB_PRINTF("vip_table_crc.tICM_ini = %x, vip_table_crc_now.tICM_ini = %x\n",vip_table_crc.tICM_ini,vip_table_crc_now.tICM_ini);
	SLRLIB_PRINTF("vip_table_crc.tICM_H_7axis = %x, vip_table_crc_now.tICM_H_7axis = %x\n",vip_table_crc.tICM_H_7axis,vip_table_crc_now.tICM_H_7axis);
	SLRLIB_PRINTF("vip_table_crc.tGAMMA = %x, vip_table_crc_now.tGAMMA = %x\n",vip_table_crc.tGAMMA,vip_table_crc_now.tGAMMA);

	SLRLIB_PRINTF("vip_table_crc.DCC_Control_Structure = %x, vip_table_crc_now.DCC_Control_Structure = %x\n",vip_table_crc.DCC_Control_Structure,vip_table_crc_now.DCC_Control_Structure);
	SLRLIB_PRINTF("vip_table_crc.Ddomain_SHPTable = %x, vip_table_crc_now.Ddomain_SHPTable = %x\n",vip_table_crc.Ddomain_SHPTable,vip_table_crc_now.Ddomain_SHPTable);
	SLRLIB_PRINTF("vip_table_crc.Idomain_MBPKTable = %x, vip_table_crc_now.Idomain_MBPKTable = %x\n",vip_table_crc.Idomain_MBPKTable,vip_table_crc_now.Idomain_MBPKTable);
	SLRLIB_PRINTF("vip_table_crc.Manual_NR_Table = %x, vip_table_crc_now.Manual_NR_Table = %x\n",vip_table_crc.Manual_NR_Table,vip_table_crc_now.Manual_NR_Table);
	SLRLIB_PRINTF("vip_table_crc.PQA_Table = %x, vip_table_crc_now.PQA_Table = %x\n",vip_table_crc.PQA_Table,vip_table_crc_now.PQA_Table);
	SLRLIB_PRINTF("vip_table_crc.PQA_Input_Table = %x, vip_table_crc_now.PQA_Input_Table = %x\n",vip_table_crc.PQA_Input_Table,vip_table_crc_now.PQA_Input_Table);
	SLRLIB_PRINTF("vip_table_crc.Auto_Function_Array1 = %x, vip_table_crc_now.Auto_Function_Array1 = %x\n",vip_table_crc.Auto_Function_Array1,vip_table_crc_now.Auto_Function_Array1);
	SLRLIB_PRINTF("vip_table_crc.Auto_Function_Array2 = %x, vip_table_crc_now.Auto_Function_Array2 = %x\n",vip_table_crc.Auto_Function_Array2,vip_table_crc_now.Auto_Function_Array2);
	SLRLIB_PRINTF("vip_table_crc.Auto_Function_Array3 = %x, vip_table_crc_now.Auto_Function_Array3 = %x\n",vip_table_crc.Auto_Function_Array3,vip_table_crc_now.Auto_Function_Array3);
	SLRLIB_PRINTF("vip_table_crc.Auto_Function_Array4 = %x, vip_table_crc_now.Auto_Function_Array4 = %x\n",vip_table_crc.Auto_Function_Array4,vip_table_crc_now.Auto_Function_Array4);
	SLRLIB_PRINTF("vip_table_crc.Auto_Function_Array5 = %x, vip_table_crc_now.Auto_Function_Array5 = %x\n",vip_table_crc.Auto_Function_Array5,vip_table_crc_now.Auto_Function_Array5);
	SLRLIB_PRINTF("vip_table_crc.DrvSetting_Skip_Flag = %x, vip_table_crc_now.DrvSetting_Skip_Flag = %x\n",vip_table_crc.DrvSetting_Skip_Flag,vip_table_crc_now.DrvSetting_Skip_Flag);
	#endif
}
/*==========================*/
#endif

unsigned char fwif_vip_increase_DI_water_lv(void)
{
	UINT8 vip_source = fwif_vip_source_check(3, 0);
	//rtd_printk(KERN_DEBUG, TAG_NAME, "\n fwif_vip_source_check(3, 0) = %d \n", vip_source);
	if (vip_source == VIP_QUALITY_HDMI_4k2kP_60)
		return 1;

	return 0;

}

unsigned char Scaler_LGE_HDR10_skip_update(void)
{
#ifdef VPQ_COMPILER_ERROR_ENABLE

	extern unsigned char Get_vo_smoothtoggle_timingchange_flag(unsigned char display);
	unsigned char bInDolbyMode = DOLBY_V_TOP_TOP_CTL_get_dolby_mode(IoReg_Read32(DOLBY_V_TOP_TOP_CTL_reg));
	unsigned char bUpdateDolbyMode = (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)
		|| (bInDolbyMode == 0)|| (Get_vo_smoothtoggle_timingchange_flag(SLR_MAIN_DISPLAY) == 0);

	if(bUpdateDolbyMode == 0)
	{
		rtd_printk(KERN_INFO, TAG_NAME, "Scaler_LGE_HDR10_skip_update, HDR COEF UPDATE BY SMOOTH TOGGLE\n");
		pr_notice("[HDR] Ignore HDR udpate %d/%d\n", Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) , bInDolbyMode);
		return 1;
	}
#endif
	return 0;
}

void Scaler_set_HDR10_Enable(unsigned char *enable)
{
	if (enable[HDR_EN])
	{
		if (!drvif_color_set_HDR10_Enable(SLR_MAIN_DISPLAY, enable))
		{
			rtd_printk(KERN_WARNING, TAG_NAME, "Scaler_set_HDR10_Enable NOT RUN!!\n");
			return;
		}
		drvif_color_set422to444(0, 0);
	}
	else
	{
		drvif_color_set_HDR10_Enable(SLR_MAIN_DISPLAY, enable);
	}
}

#if 0
void Scaler_LGE_HDR10_Enable(unsigned char enable)
{
	RGB2YUV_BLACK_LEVEL blk_lv;
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "blk_lv=%d, videoFormat=%d, HDR mode = %d\n", blk_lv, video_format, get_HDR_mode());*/

	blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "[HDR10],videoFMT=%d,hdr_mode=%d\n", video_format, get_HDR_mode());*/
	if (enable != 0)	//enable HDR 10
	{
		if (enable > 2) enable = 1; //protect
		if(blk_lv == RGB2YUV_BLACK_LEVEL_LOW || blk_lv == RGB2YUV_BLACK_LEVEL_AUTO) { /* use limit2full*/
			if (enable == 1) { /* bt 709 */
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255]);
			} else { /* bt 2020*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);
			}
		} else {	/* use limit2limit*/
			if (enable == 1) {/* bt 709 */
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_255_to_255]);
			} else { /* bt 2020*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);
			}
		}
	}
	else
	{
		//SDR
	}
}
#endif

unsigned char Scaler_get_HDR_YUV2RGB_SDR_MAX_RGB_TV006(RGB2YUV_BLACK_LEVEL blk_lv)
{
	unsigned char ret_idx=0;
	unsigned char isHD=0, SrcGetFrom=0;
	unsigned char display;
	unsigned char video_format = COLOR_UNKNOW;
	UINT8 data_range = HDMI_CSC_DATA_RANGE_MODE_AUTO;
	SLR_VOINFO* pVOInfo = NULL;
	HDMI_AVI_T pAviInfo = {0};
	unsigned char colorFMT_info = COLORSPACE_MAX;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned char HDMI_CSC_DataRange_Mode = 255;
	unsigned char IsRGB_YUV_RANGE = 255;
	unsigned char srcType;
	unsigned short mode;
	unsigned char isHDMI=0;

	//unsigned int DisplayMode_Src_vsc = 0;
	unsigned char twopixel_mode = 0, hdmi_input = 0, color_format = 0;

	display = (unsigned char)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	srcType = Scaler_InputSrcGetType(display);
	mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	//DisplayMode_Src_vsc = (unsigned int)Get_DisplayMode_Src(display);

	//if (DisplayMode_Src_vsc = VSC_INPUTSRC_HDMI)
	get_vtop_input_info(&twopixel_mode, &hdmi_input, &color_format);

#ifdef CONFIG_FORCE_RUN_I3DDMA
	if (fwif_color_get_force_run_i3ddma_enable(display)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &srcType, &SrcGetFrom);
		mode = fwif_color_get_cur_input_timing_mode(display);
	}
#endif

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T *) (&pAviInfo));

	if(VIP_system_info_structure_table == NULL || pVOInfo == NULL){
		rtd_printk(KERN_EMERG, TAG_NAME, "~get NULL warning return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	if ((mode !=  _MODE_480I) && (mode !=  _MODE_480P) && (mode !=  _MODE_576I) && (mode !=  _MODE_576P))
		isHD = 1;

	// check color format from header, and data range
	if (srcType == _SRC_HDMI) {
		HDMI_CSC_DataRange_Mode = Scaler_GetHDMI_CSC_DataRange_Mode();
		IsRGB_YUV_RANGE = drvif_IsRGB_YUV_RANGE();
		video_format = drvif_Hdmi_GetColorSpace();
		isHDMI = drvif_IsHDMI();

		if (hdmi_input == 1 && color_format == COLOR_RGB) {
			colorFMT_info = isRGB;	// data format will be GBR from hdmi input
		} else if ((pAviInfo.EC == 5 || pAviInfo.EC == 6) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED)) {
			colorFMT_info = isBT2020;
		} else if (((pAviInfo.EC == 0 || pAviInfo.EC == 2 || pAviInfo.EC == 3) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED))) {
			colorFMT_info = is601;
		} else if (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_SMPTE170) {
			colorFMT_info = is601;
		} else if ((pAviInfo.EC == 1 && pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED) || (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_ITU709)) {
			colorFMT_info = is709;
		} else {
			if (isHD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

		if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_AUTO) {
			if (IsRGB_YUV_RANGE==MODE_RAG_FULL)
				data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;
			else if (IsRGB_YUV_RANGE==MODE_RAG_LIMIT)
				data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
			else
				data_range = HDMI_CSC_DATA_RANGE_MODE_AUTO;
		} else {
			data_range = HDMI_CSC_DataRange_Mode;
		}

		if (data_range == HDMI_CSC_DATA_RANGE_MODE_AUTO) {
			if (video_format !=  0) {/*YUV444 or YUV422*/
				data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
			} else {
				if (isHDMI)
					data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
				else
					data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;
			}
		}

	} else if (srcType == _SRC_VO) {
		if (pVOInfo->colour_primaries == 1) {
			colorFMT_info = is709;
		} else if (pVOInfo->colour_primaries == 6 || pVOInfo->colour_primaries == 7) {
			colorFMT_info = is601;
		} else if (pVOInfo->colour_primaries == 9) {
			colorFMT_info = isBT2020;
		} else {
			if (isHD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

		data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;

	} else if (srcType == _SRC_VGA) {
		if (isHD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
		data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;

	} else {
		if (isHD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
		data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
	}

	// use black level to determine data range
	if (blk_lv == RGB2YUV_BLACK_LEVEL_HIGH) {
		if (colorFMT_info == isRGB) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_255_to_255;	// data format will be GBR from hdmi input
		} else if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
		} else if (colorFMT_info == is709) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		}
	} else {
		if (colorFMT_info == isRGB) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_Limted_235_to_255;	// data format will be GBR from hdmi input
		} else if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
		} else if (colorFMT_info == is709) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
		}
	}

	rtd_printk(KERN_INFO, TAG_NAME, "DM_SMR006,dis=%d,mode=%d,srcType=%d,SrcGetFrom=%d,isHD=%d,HDMI_CSC_DataRange_Mode=%d,IsRGB_YUV_RANGE=%d,\n",
		display, mode, srcType, SrcGetFrom, isHD, HDMI_CSC_DataRange_Mode, IsRGB_YUV_RANGE);

	rtd_printk(KERN_INFO, TAG_NAME, "DM_SMR006,video_fmt=%d,isHDMI=%d,.EC=%d,.C=%d,colorFMT_info=%d,data_range=%d,colour_primaries=%d,ret_idx=%d,blk_lv=%d\n",
		video_format, isHDMI, pAviInfo.EC, pAviInfo.C, colorFMT_info, data_range, pVOInfo->colour_primaries, ret_idx, blk_lv);

	return ret_idx;

}

unsigned char Scaler_get_HDR_YUV2RGB_SDR_MAX_RGB(void)
{
	unsigned char ret_idx=0;
	unsigned char isHD=0, SrcGetFrom=0;
	unsigned char display = (unsigned char)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	unsigned char video_format = COLOR_UNKNOW;
	UINT8 data_range = HDMI_CSC_DATA_RANGE_MODE_AUTO;
	SLR_VOINFO* pVOInfo = NULL;
	HDMI_AVI_T pAviInfo = {0};
	unsigned char colorFMT_info = COLORSPACE_MAX;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned char HDMI_CSC_DataRange_Mode = 255;
	unsigned char IsRGB_YUV_RANGE = 255;
	unsigned char srcType = Scaler_InputSrcGetType(display);
	unsigned short mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	unsigned char isHDMI=0;
	unsigned char twopixel_mode = 0, hdmi_input = 0, color_format = 0;
	//unsigned char bUse_HDMI_Range_Detect = 1;
	//unsigned char isHDR_Signal = 0;
#ifdef CONFIG_FORCE_RUN_I3DDMA
	if (fwif_color_get_force_run_i3ddma_enable(display)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &srcType, &SrcGetFrom);
		mode = fwif_color_get_cur_input_timing_mode(display);
	}
#endif

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T *) (&pAviInfo));
	get_vtop_input_info(&twopixel_mode, &hdmi_input, &color_format);
	if(VIP_system_info_structure_table == NULL || pVOInfo == NULL){
		rtd_printk(KERN_EMERG, TAG_NAME, "~get NULL warning return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	if ((mode !=  _MODE_480I) && (mode !=  _MODE_480P) && (mode !=  _MODE_576I) && (mode !=  _MODE_576P))
		isHD = 1;

	// check color format from header, and data range
	if (srcType == _SRC_HDMI) {
		HDMI_CSC_DataRange_Mode = Scaler_GetHDMI_CSC_DataRange_Mode();
		IsRGB_YUV_RANGE = drvif_IsRGB_YUV_RANGE();
		video_format = drvif_Hdmi_GetColorSpace();
		isHDMI = drvif_IsHDMI();

		if (hdmi_input == 1 && color_format == COLOR_RGB) {
			colorFMT_info = isRGB;	// data format will be GBR from hdmi input
		} else if ((pAviInfo.EC == 5 || pAviInfo.EC == 6) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED)) {
			colorFMT_info = isBT2020;
		} else if (((pAviInfo.EC == 0 || pAviInfo.EC == 2 || pAviInfo.EC == 3) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED))) {
			colorFMT_info = is601;
		} else if (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_SMPTE170) {
			colorFMT_info = is601;
		} else if ((pAviInfo.EC == 1 && pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED) || (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_ITU709)) {
			colorFMT_info = is709;
		} else {
			if (isHD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

		if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_AUTO) {
			if (IsRGB_YUV_RANGE==MODE_RAG_FULL)
				data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;
			else if (IsRGB_YUV_RANGE==MODE_RAG_LIMIT)
				data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
			else
				data_range = HDMI_CSC_DATA_RANGE_MODE_AUTO;
		} else {
			data_range = HDMI_CSC_DataRange_Mode;
		}

		if (data_range == HDMI_CSC_DATA_RANGE_MODE_AUTO) {
			if (video_format !=  0) {/*YUV444 or YUV422*/
				data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
                rtd_printk(KERN_EMERG, TAG_NAME, "[%s:%d][HDR_YUV2RGB]HDMI_CSC_DATA_RANGE_MODE_LIMITED\n", __FUNCTION__, __LINE__);
			} else {
				data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;
                rtd_printk(KERN_EMERG, TAG_NAME, "[%s:%d][HDR_YUV2RGB]HDMI_CSC_DATA_RANGE_MODE_FULL\n", __FUNCTION__, __LINE__);
			}
		}

	} else if (srcType == _SRC_VO) {
		if (pVOInfo->colour_primaries == 1) {
			colorFMT_info = is709;
		} else if (pVOInfo->colour_primaries == 6 || pVOInfo->colour_primaries == 7) {
			colorFMT_info = is601;
		} else if (pVOInfo->colour_primaries == 9) {
			colorFMT_info = isBT2020;
		} else {
			if (isHD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}

		data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;

	} else if (srcType == _SRC_VGA) {
		if (isHD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
		data_range = HDMI_CSC_DATA_RANGE_MODE_FULL;

	} else {
		if (isHD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
		data_range = HDMI_CSC_DATA_RANGE_MODE_LIMITED;
	}

	// get index, should be RGB full in DM
	if (data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
		if (colorFMT_info == isRGB) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_255_to_255;	// data format will be GBR from hdmi input
		} else if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
		} else if (colorFMT_info == is709) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		}
	} else {
		if (colorFMT_info == isRGB) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_Limted_235_to_255;	// data format will be GBR from hdmi input
		} else if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
		} else if (colorFMT_info == is709) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
		}
	}

	rtd_printk(KERN_INFO, TAG_NAME, "DM_SMR, dis=%d,mode=%d,srcType=%d,SrcGetFrom=%d,isHD=%d,HDMI_CSC_DataRange_Mode=%d,IsRGB_YUV_RANGE=%d,\n",
		display, mode, srcType, SrcGetFrom, isHD, HDMI_CSC_DataRange_Mode, IsRGB_YUV_RANGE);

	rtd_printk(KERN_INFO, TAG_NAME, "DM_SMR, video_format=%d,isHDMI=%d,.EC=%d,.C=%d,colorFMT_info=%d,data_range=%d,colour_primaries=%d,ret_idx=%d,\n",
		video_format, isHDMI, pAviInfo.EC, pAviInfo.C, colorFMT_info, data_range, pVOInfo->colour_primaries, ret_idx);

	return ret_idx;
#if 0
	if (srcType == _SRC_VGA) {
		if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		}

	} else if ((srcType == _SRC_YPBPR) || (srcType == _SRC_COMPONENT)) {
		if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		}

	} else if (srcType == _SRC_SCART_RGB) {
		if (colorFMT_info == is601) {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
		} else {
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
		}

	} else if ((srcType == _SRC_TV) || (srcType == _SRC_CVBS) || (srcType == _SRC_SV) || (srcType == _SRC_SCART)) {
		ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;

	} else if (srcType == _SRC_HDMI) {
		if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {	// max RGB case
		if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) {
			if (video_format !=  0) {/*YUV444 or YUV422*/
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
					if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
					}

				} else {
					if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
					} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
					} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
					}

				}
			} else {/*RGB*/
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
					if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
					}

				} else {
					if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
					} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
					} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
					}

				}
			}
		} else {	// DVI
			if (video_format !=  0) {/*YUV444 or YUV422*/
					if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}

					} else {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}

					}
				} else {	/* RGB */
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_LIMITED) {
					if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}

					} else {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}

					}
				}
			}
		} else {
			if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) {
				if (video_format !=  0) {/*YUV444 or YUV422*/
					if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}

					} else {
						if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
					}

					}
				} else {/*RGB*/
					if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
						}

				} else {
					if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
						}

					}
				}
			} else {	// DVI
				if (video_format !=  0) {/*YUV444 or YUV422*/
					if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_FULL) {
						if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
					}

					} else {
						if (colorFMT_info == isBT2020) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
						} else if (colorFMT_info == is709) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
						} else if (colorFMT_info == is601) {
							ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
						}

				}
			} else {	/* RGB */
				if (bUse_HDMI_Range_Detect && data_range == HDMI_CSC_DATA_RANGE_MODE_LIMITED) {
					if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
					}

				} else {
					if (colorFMT_info == isBT2020) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
					} else if (colorFMT_info == is709) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
					} else if (colorFMT_info == is601) {
						ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
					}

					}
				}
			}
		}
	} else if (srcType == _SRC_VO) {
		if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {	// max RGB
			if ((Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE) == COLOR_RGB) || (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2)) {
				if (colorFMT_info == isBT2020) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
				} else if (colorFMT_info == is709) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
				} else if (colorFMT_info == is601) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
				}

			} else if (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0) { /*DTV source*/
				if (colorFMT_info == isBT2020) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
				} else if (colorFMT_info == is709) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
				} else if (colorFMT_info == is601) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
				}

			} else {  /*Jpeg source*/
				if (colorFMT_info == isBT2020) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
				} else if (colorFMT_info == is709) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
				} else if (colorFMT_info == is601) {
					ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
				}

			}
			//if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)
				//VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		} else {
		if ((Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE) == COLOR_RGB) || (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2)) {
			if (colorFMT_info == isBT2020) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
			} else if (colorFMT_info == is709) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
			} else if (colorFMT_info == is601) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
			}

		} else if (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0) { /*DTV source*/
			if (colorFMT_info == isBT2020) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
			} else if (colorFMT_info == is709) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
			} else if (colorFMT_info == is601) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
			}

		} else {  /*Jpeg source*/
			if (colorFMT_info == isBT2020) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
			} else if (colorFMT_info == is709) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
			} else if (colorFMT_info == is601) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
			}

		}
		//if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)
			//VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		}
	} else {
		ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;

		rtd_printk(KERN_EMERG, TAG_NAME, "CSC_SMR, unhandler type%d,\n", srcType);
	}

	rtd_printk(KERN_INFO, TAG_NAME, "CSC_SMR,disp=%d,src=%d,ret=%d,mode=%d,\n", display, srcType, ret_idx, mode);

	return ret_idx;

#endif
}

unsigned char Scaler_get_HDR_YUV2RGB_ColorFMT_601_in_IMD_Domain(unsigned char color_format)
{
	unsigned char ret_idx;
	unsigned char isFullRange = 0;
	unsigned char display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	unsigned char video_format = COLOR_UNKNOW;
	unsigned char srcType = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);

	if (srcType == VSC_INPUTSRC_HDMI) {
		HDMI_CSC_DATA_RANGE_MODE_T HDMI_CSC_DataRange_Mode = Scaler_GetHDMI_CSC_DataRange_Mode();
		HDMI_RGB_YUV_RANGE_MODE_T IsRGB_YUV_RANGE = drvif_IsRGB_YUV_RANGE();
		if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_AUTO) {
			if (IsRGB_YUV_RANGE==MODE_RAG_FULL)
				isFullRange = 1;
		} else if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_FULL) {
			isFullRange = 1;
		}

		video_format = drvif_Hdmi_GetColorSpace();
	} else if (srcType == VSC_INPUTSRC_VDEC) {
		isFullRange = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane())->video_full_range_flag?1:0;
	}

	if (color_format == HAL_VPQ_COLORIMETRY_BT709) {
		if (video_format == COLOR_RGB) {
			if (isFullRange == 1)
				ret_idx = HDR_DM_CSC1_Inv2020_RGB709toRGB709_FtoF;
			else
				ret_idx = HDR_DM_CSC1_Inv2020_RGB709toRGB709_LtoF;
		} else {
			if (isFullRange == 1)
				ret_idx = HDR_DM_CSC1_YUV709toRGB709_FtoF;
			else
				ret_idx = HDR_DM_CSC1_YUV709toRGB709_LtoF;
		}
#ifdef CONFIG_CUSTOMER_TV002
	} else if (color_format == HAL_VPQ_COLORIMETRY_BT601) {
		if (video_format == COLOR_RGB) {
			if (isFullRange == 1)
				ret_idx = HDR_DM_CSC1_Inv2020_RGB709toRGB709_FtoF;
			else
				ret_idx = HDR_DM_CSC1_Inv2020_RGB709toRGB709_LtoF;
		} else {
			if (isFullRange == 1)
				ret_idx = HDR_DM_CSC1_YUV601toRGB601_FtoF;
			else
				ret_idx = HDR_DM_CSC1_YUV601toRGB601_LtoF;
		}
#endif
	} else {
		if (isFullRange == 1)
			ret_idx = HDR_DM_CSC1_YUV2020toRGB2020_FtoF;
		else
			ret_idx = HDR_DM_CSC1_YUV2020toRGB2020_LtoF;
	}

	rtd_printk(KERN_INFO, TAG_NAME, "CSC1, display=%d,srcType=%d,color_fmt=%d,video_fmt=%d,FullRange=%d,ret=%d\n",
		display, srcType, color_format, video_format, isFullRange,  ret_idx);

	return ret_idx;

}

unsigned char Scaler_get_HDR_YUV2RGB(unsigned char color_format)
{
	unsigned char ret_idx;
	unsigned char isFullRange = 0;
	unsigned char display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	unsigned char video_format = COLOR_UNKNOW;
	unsigned char srcType = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);

	if (srcType == VSC_INPUTSRC_HDMI) {
		HDMI_CSC_DATA_RANGE_MODE_T HDMI_CSC_DataRange_Mode = Scaler_GetHDMI_CSC_DataRange_Mode();
		HDMI_RGB_YUV_RANGE_MODE_T IsRGB_YUV_RANGE = drvif_IsRGB_YUV_RANGE();
		if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_AUTO) {
			if (IsRGB_YUV_RANGE==MODE_RAG_FULL)
				isFullRange = 1;
		} else if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_FULL) {
			isFullRange = 1;
		}

		video_format = drvif_Hdmi_GetColorSpace();
	} else if (srcType == VSC_INPUTSRC_VDEC) {
		isFullRange = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane())->video_full_range_flag?1:0;
	}

	if (color_format == HAL_VPQ_COLORIMETRY_BT709) {
			if (isFullRange == 1)
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
			else
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
	} else {
		if (isFullRange == 1)
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
		else
			ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
	}

	rtd_printk(KERN_INFO, TAG_NAME, "CSC1, display=%d,srcType=%d,color_fmt=%d,video_fmt=%d,FullRange=%d,ret=%d\n",
		display, srcType, color_format, video_format, isFullRange,  ret_idx);

	return ret_idx;

}


void Scaler_Set_HDR_YUV2RGB(unsigned char display, unsigned char HDR_mode, unsigned char color_format)
{
#ifdef CONFIG_SUPPORT_IPQ
	unsigned char index_ret = 0;
	if(display == SLR_MAIN_DISPLAY) {
		index_ret = fwif_color_get_HDR_YUV2RGB_SDR_MAX_RGB_by_HFC_TV043(color_format);
		printk("HDR_YUV2RGB,index=%d,color_format=%d\n",index_ret,color_format);
		drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[index_ret]);
	} 
	else {
		rtd_printk(KERN_INFO, TAG_NAME, "DM_CSC,HDR_mode=%d, color_format=%d,display=%d,\n", HDR_mode, color_format, display);
		if (HDR_mode == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {
			// HDR 1_2
			drvif_HDR1_2_YUV2RGB(1, hdr_YUV2RGB[HDR_SUB_YUV2RGB_BT2020_Limted_235_240_to_255]);	
	
			// HDR 2
			fwif_color_HDR_sub_set_yuv2rgb( &(HDR_sub_YUV2RGB[HDR_SUB_YUV2RGB_BT2020_Limted_235_240_to_255]) );
		
		}
		else if ((HDR_mode == HAL_VPQ_HDR_MODE_HDR10) || (HDR_mode == HAL_VPQ_HDR_MODE_HLG)) {
			// HDR 1_2
			drvif_HDR1_2_YUV2RGB(1, hdr_YUV2RGB[HDR_SUB_YUV2RGB_BT2020_Limted_235_240_to_255]);	
	
			// HDR 2
			fwif_color_HDR_sub_set_yuv2rgb( &(HDR_sub_YUV2RGB[HDR_SUB_YUV2RGB_BT2020_Limted_235_240_to_255]) );
			
		}	
	}
#else

	RGB2YUV_BLACK_LEVEL blk_lv;
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "blk_lv=%d, videoFormat=%d, HDR mode = %d\n", blk_lv, video_format, get_HDR_mode());*/
	unsigned char index = VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max;

	g_HDR_mode = HDR_mode;
	g_HDR_color_format = color_format;
	
	if(display == SLR_MAIN_DISPLAY) {
		blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();
		/*rtd_printk(KERN_DEBUG, TAG_NAME, "[HDR10],videoFMT=%d,hdr_mode=%d\n", video_format, get_HDR_mode());*/
		if (HDR_mode == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {
			if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
				index = Scaler_get_HDR_YUV2RGB_SDR_MAX_RGB();
			else
				index = Scaler_get_HDR_YUV2RGB_SDR_MAX_RGB_TV006(blk_lv);
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[index]);
		} else if (ColorFMT_601_in_IMD_Domain_Enable == 1) {
			index = Scaler_get_HDR_YUV2RGB_ColorFMT_601_in_IMD_Domain(color_format);
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[index]);
		} else if ((HDR_mode == HAL_VPQ_HDR_MODE_HDR10) || (HDR_mode == HAL_VPQ_HDR_MODE_HLG)) {
			if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) {
				index = Scaler_get_HDR_YUV2RGB(color_format);
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[index]);
			} else if(blk_lv == RGB2YUV_BLACK_LEVEL_HIGH) {	/* use limit2limit*/
				if (color_format == HAL_VPQ_COLORIMETRY_BT709) {/* bt 709 */
					drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_255_to_255]);
				} else { /* bt 2020*/
					drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);
				}
			} else { /* use limit2full*/
				if (color_format == HAL_VPQ_COLORIMETRY_BT709) { /* bt 709 */
					drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255]);
				} else { /* bt 2020*/
					drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);
				}
			}
		}
		else if (HDR_mode == HAL_VPQ_HDR_MODE_ST2094)
		{
			if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) {
				index = Scaler_get_HDR_YUV2RGB(color_format);
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[index]);
			} else if(blk_lv == RGB2YUV_BLACK_LEVEL_HIGH) {	/* use limit2limit*/
				if (color_format == HAL_VPQ_COLORIMETRY_BT709) {/* bt 709 */
					drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_255_to_255]);
				} else { /* bt 2020*/
					drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);
				}
			} else { /* use limit2full*/
				if (color_format == HAL_VPQ_COLORIMETRY_BT709) { /* bt 709 */
					drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255]);
				} else { /* bt 2020*/
					drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);
				}
			}
		}
		else
		{
			//SDR
		}
	} else {		
		rtd_printk(KERN_INFO, TAG_NAME, "DM_CSC,HDR_mode=%d, color_format=%d,display=%d,\n", HDR_mode, color_format, display);
		if (HDR_mode == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {
			// HDR 1_2
			drvif_HDR1_2_YUV2RGB(1, hdr_YUV2RGB[HDR_SUB_YUV2RGB_BT2020_Limted_235_240_to_255]);	
	
			// HDR 2
			fwif_color_HDR_sub_set_yuv2rgb( &(HDR_sub_YUV2RGB[HDR_SUB_YUV2RGB_BT2020_Limted_235_240_to_255]) );
		
		}
		else if ((HDR_mode == HAL_VPQ_HDR_MODE_HDR10) || (HDR_mode == HAL_VPQ_HDR_MODE_HLG)) {
			// HDR 1_2
			drvif_HDR1_2_YUV2RGB(1, hdr_YUV2RGB[HDR_SUB_YUV2RGB_BT2020_Limted_235_240_to_255]);	
	
			// HDR 2
			fwif_color_HDR_sub_set_yuv2rgb( &(HDR_sub_YUV2RGB[HDR_SUB_YUV2RGB_BT2020_Limted_235_240_to_255]) );
			
		}
	}		
#endif
}

void Scaler_Set_HDR_YUV2RGB_by_HFC(UINT8 index, unsigned char color_format, unsigned char HDT_type)
{
	RGB2YUV_BLACK_LEVEL blk_lv;
	unsigned char index_ret = 0;
	unsigned char idx;
	blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();

	g_HDR_color_format = color_format;

	if (ColorFMT_601_in_IMD_Domain_Enable == 1) {
		index_ret = Scaler_get_HDR_YUV2RGB_ColorFMT_601_in_IMD_Domain(color_format);
		drvif_HDR_YUV2RGB(1, hdr_YUV2RGB_HFC[index][index_ret]);
	} else if (HDT_type == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {
		idx = Scaler_get_HDR_YUV2RGB_SDR_MAX_RGB();
		drvif_HDR_YUV2RGB(1, hdr_YUV2RGB_HFC[index][idx]);
	} else if(blk_lv == RGB2YUV_BLACK_LEVEL_HIGH) {	/* use limit2limit*/
		if (color_format == HAL_VPQ_COLORIMETRY_BT709) {/* bt 709 */
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB_HFC[index][HDR_DM_CSC1_YUV2RGB_BT709_255_to_255]);
		} else { /* bt 2020*/
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB_HFC[index][HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);
		}
	}
	else { /* use limit2full*/
		if (color_format == HAL_VPQ_COLORIMETRY_BT709) { /* bt 709 */
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB_HFC[index][HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255]);
		} else { /* bt 2020*/
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB_HFC[index][HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);
		}
	}

}


void Scaler_LGE_HDR10_Enable_newflow(unsigned char display, unsigned char enable)
{
	//UINT8 video_format;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "blk_lv=%d, videoFormat=%d, HDR mode = %d\n", blk_lv, video_format, get_HDR_mode());*/

 	//video_format = drvif_Hdmi_GetColorSpace();
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}
	rtd_printk(KERN_INFO, TAG_NAME, "Scaler_LGE_HDR10_Enable_newflow, En=%d,dis=%d, \n", enable, display);

	if(display == SLR_MAIN_DISPLAY) {
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_HDR10_enable_Coef] = enable;
		/*rtd_printk(KERN_DEBUG, TAG_NAME, "[HDR10],videoFMT=%d,hdr_mode=%d\n", video_format, get_HDR_mode());*/
		if (enable != 0)	//enable HDR 10
		{
			if (enable > 2) enable = 1; //protect

#if 0
			if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE) < 4000)
				drvif_Set_DM_HDR1_1p_2p(0);
			else
				drvif_Set_DM_HDR1_1p_2p(1);
#endif			

			drvif_color_set_HDR10_Enable(display, hdr_table[1]);	//enable process
#if 0	// set i3d csc in "fwif_color_colorspacergb2yuvtransfer()"
			if ((video_format == 0) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)) {/*HDMI HDR RGB case*/
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(1, &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255][0]), 0);
			} else {/* yuv */
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(0, NULL, 0);
			}
#endif
			fwif_color_set_Main_Dither(TRUE,VIP_DITHER_MAINTABLE_HDR10_12to10);

		}
		else
		{
			drvif_color_set_HDR10_Enable(display, hdr_table[0]);	//disable process
			//drvif_Set_DM_HDR1_1p_2p(0);
		}

	} else {
			if (enable != 0)	//enable HDR 10
			{
#if 0			
				//HDR 1_2
				if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID) < 4000)
					drvif_Set_DM_HDR1_2_1p(1);
				else
					drvif_Set_DM_HDR1_2_1p(0);
#endif
				drvif_color_set_HDR1_2_HDR10_Enable(display, hdr_table[1]);

				// HDR 2
				fwif_color_set_HDR_sub_en(1);
				// 422 to 444
				fwif_color_HDR_sub_set422to444(1);
			}
			else
			{
				//HDR 1_2
				//drvif_Set_DM_HDR1_2_1p(0);
				drvif_color_set_HDR1_2_HDR10_Enable(display, hdr_table[0]);
				
				// HDR 2			
				fwif_color_set_HDR_sub_en(0);
				// 422 to 444
				fwif_color_HDR_sub_set422to444(1);
			
				//drvif_color_set_HDR10_Enable(display, hdr_table[0]); //disable process
			}

	}		
}

void Scaler_SetHDR_BlackLevel(unsigned char blk_lv)
{
	unsigned char video_format, enable;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table;
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	enable = VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_HDR10_enable_Coef];


	if (VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] == 0) /*HDR off*/
		return;

	video_format = drvif_Hdmi_GetColorSpace();

	if(blk_lv == RGB2YUV_BLACK_LEVEL_LOW) { /* use limit2full*/
		if ((video_format == 0) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)) {/*HDMI HDR RGB case*/
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);

		} else if (enable == 1) { /* bt 709 */
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255]);

		} else { /* bt 2020*/
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);

		}
	} else {	/* use limit2limit*/
		if ((video_format == 0) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)) {/*HDMI HDR RGB case*/
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);

		} else if (enable == 1) {/* bt 709 */
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_255_to_255]);

		} else { /* bt 2020*/
			drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);

		}
	}

}

void drvif_TV006_SET_HDR_mode_off(void)
{
	drvif_color_set_BBC_Enable(0);
	Scaler_LGE_HDR10_Enable_newflow(SLR_MAIN_DISPLAY, 0);
	drvif_DM2_OETF_Enable(0, 0);
	drvif_DM2_EOTF_Enable(0, 0);
	drvif_DM2_B0203_noSram_Enable(1, 0);
}
#if 0
void drvif_TV006_HDR10_init(void)
{
	static PQ_HDR_3dLUT_16_PARAM args;

	drvif_TV006_SET_HDR_mode_off();
	fwif_HDR_RGB2OPT(0, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_709]);
	drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);	// 20180109 roger, give default value for HAL do not correct case. BT2020 limit range para

	//20161020 roger mark
	// 1. K3L use 24x24x24
	// 2. when boot/source change... LG set 3DLUT sevaral times
	//fwif_color_set_DM_HDR_3dLUT(0, HDR10_3DLUT_17x17x17);
	args.c3dLUT_16_enable = 1;
	args.a3dLUT_16_pArray = (UINT16*)&HDR10_3DLUT_24x24x24[0];
	fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 0);
}
#endif
void drvif_TV006_SET_HDR_mode(UINT32* modeinfo)
{
	//_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	extern unsigned char g_HDR3DLUTForceWrite;

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	rtd_printk(KERN_INFO, TAG_NAME, "drvif_TV006_SET_HDR_mode, modeinfo = %d, Colorimetry info = %d\n", modeinfo[0], modeinfo[1]);

	//SET_HDR_mode
	switch(modeinfo[0])	//HDR mode
	{
	case HAL_VPQ_HDR_MODE_HDR10:
		//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		//Scaler_LGE_HDR10_Enable(modeinfo[1]);
		Scaler_Set_HDR_YUV2RGB(SLR_MAIN_DISPLAY, modeinfo[0], modeinfo[1]);

	break;
	case HAL_VPQ_HDR_MODE_HLG:
		//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		//Scaler_LGE_HDR10_Enable(modeinfo[1]);
		Scaler_Set_HDR_YUV2RGB(SLR_MAIN_DISPLAY, modeinfo[0], modeinfo[1]);

	break;
	case HAL_VPQ_HDR_MODE_SDR:
	case HAL_VPQ_HDR_MODE_HDREFFECT:
		//HDR10 and HLG off
		//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 0;
		g_HDR3DLUTForceWrite = 1;

	break;
	default:
		rtd_printk(KERN_ERR, TAG_NAME, "drvif_TV006_SET_HDR_mode, get unknow mode = %d\n", modeinfo[0]);
	break;
	}

}

#ifdef CONFIG_HDR_SDR_SEAMLESS
extern HDR_SDR_SEAMLESS_PQ_STRUCT HDR_SDR_SEAMLESS_PQ;
extern unsigned char hdr_table[HDR_TABLE_NUM][HDR_ITEM_MAX];
unsigned int PQModeInfo_flag_seamless[5];
extern void scalerVIP_Write_RGB2YUV(unsigned long arg);
void Scaler_hdr_setting_SEAMLESS(unsigned char hdr_enable)
{
	extern bool get_support_vo_force_v_top(unsigned int func_flag);
	//_system_setting_info *system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	bool sdr_force_vtop = false;
	_RPC_system_setting_info *RPC_VIP_system_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	static unsigned char hdrType;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(VIP_system_info_structure_table == NULL){
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s:%d] error here!! system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	if(RPC_VIP_system_info == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	sdr_force_vtop = get_support_vo_force_v_top(support_sdr_max_rgb);
	if(hdr_enable)
	{
		if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) {	/* HDMI case*/
			PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_HDR10;
			PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT2020;
			hdrType = HDR_DM_HDR10;
			//system_info_struct->Debug_Buff_8[0]++;
		} else {	/* DTV and USB */

			seamless_change_sync_info *seamless_info = NULL;
			unsigned int vdec_cur_hdr_type;
			seamless_info = (seamless_change_sync_info *)Scaler_GetShareMemVirAddr(SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG);
			if (seamless_info)
			{
				vdec_cur_hdr_type = seamless_info->vdec_cur_hdr_type;
				vdec_cur_hdr_type = fwif_color_ChangeOneUINT32Endian(vdec_cur_hdr_type, 1);
			}
			else
			{
				pr_err("[seamless] ERROR, get NULL HDR type!!\n");
				vdec_cur_hdr_type = HDR_DM_MODE_NONE;
			}

			if((vdec_cur_hdr_type == HDR_DM_MODE_NONE) && sdr_force_vtop)
				vdec_cur_hdr_type = HDR_DM_SDR_MAX_RGB;//sdr force vtop max RGB case

			//system_info_struct->Debug_Buff_8[1]++;
			if (vdec_cur_hdr_type == HDR_DM_HDR10)
			{
				PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_HDR10;
				PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT2020;
				hdrType = HDR_DM_HDR10;
			}
			else if (vdec_cur_hdr_type == HDR_DM_HLG)
			{
				PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_HLG;
				PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT2020;
				hdrType = HDR_DM_HLG;
			}
			else if (vdec_cur_hdr_type == HDR_DM_MODE_NONE)
			{
				PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_SDR;
				PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT709;
				hdrType = HDR_DM_MODE_NONE;
			}
			else if (vdec_cur_hdr_type == HDR_DM_SDR_MAX_RGB)
			{
				PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_SDR_MAX_RGB;//nned PQ change
				PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT2020;
				hdrType = HDR_DM_SDR_MAX_RGB;
			}
			else
			{
				pr_err("[seamless] ERROR, get unknown HDR type!! hdr_type = %d\n", vdec_cur_hdr_type);
				PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_SDR;
				PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT709;
				hdrType = HDR_DM_MODE_NONE;
			}
		}
	}
	else
	{
		PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_SDR;
		PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT709;
		hdrType = HDR_DM_MODE_NONE;
		//system_info_struct->Debug_Buff_8[2]++;
	}
	//return;
	//system_info_struct->Debug_Buff_8[2] = PQModeInfo_flag_seamless[0];
	//system_info_struct->Debug_Buff_8[3] = fwif_color_ChangeOneUINT32Endian(pVOInfo->transfer_characteristics, 1);
	/*
	if (pre_HDR_MODE == PQModeInfo_flag_seamless[0]) {
		//system_info_struct->Debug_Buff_8[0]++;
		return;
	} else {
		pre_HDR_MODE = PQModeInfo_flag_seamless[0];
		//system_info_struct->Debug_Buff_8[1]++;
	}
	*/
	/* timing recording*/
	RPC_VIP_system_info->HDR_info.EOTF_OETF[oetf_eotf_reverse_0] = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
	RPC_VIP_system_info->HDR_info.EOTF_OETF[oetf_eotf_reverse_2] =  VGIP_VGIP_CHN1_LC_get_ch1_line_cnt(IoReg_Read32(VGIP_VGIP_CHN1_LC_reg));

	//unsigned char InputSrcGetType = Scaler_InputSrcGetType(SLR_MAIN_DISPLAY);
#if 1
	// write RGB to yuv table
	//fwif_color_colorspacergb2yuvtransfer_Seamless(hdr_enable, InputSrcGetType);
	//scalerVIP_Write_RGB2YUV(1);

	// write yuv to RGB table
	//fwif_color_setCon_Bri_Color_Tint_tv006(0, system_info_struct);

	//fwif_color_SetDataFormatHandler_tv006_Seamless(hdr_enable);
#else

	Scaler_SetDataFormatHandler();
#endif

	Scaler_color_set_HDR_Type(hdrType);
	drvif_TV006_SET_HDR_mode_Seamless(PQModeInfo_flag_seamless);
	//Scaler_Set_HDR_YUV2RGB(VIP_system_info_structure_table->HDR_flag, HAL_VPQ_COLORIMETRY_BT2020);	// move to "Scaler_color_set_HDR_AutoRun_TBL()", can't use semaphore in ISR
	drvif_color_HDR10_DM_setting();
//	Scaler_SetDataFormatHandler();
	//Scaler_SetDataFormatHandle_Seamless();

	if(Scaler_color_Get_HDR_AutoRun())
		Scaler_color_Access_HDR_AutoRun_TBL_Flag(1, &hdrType);

	/* timing recording*/
	RPC_VIP_system_info->HDR_info.EOTF_OETF[oetf_eotf_reverse_1] = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);
	RPC_VIP_system_info->HDR_info.EOTF_OETF[oetf_eotf_reverse_3] =  VGIP_VGIP_CHN1_LC_get_ch1_line_cnt(IoReg_Read32(VGIP_VGIP_CHN1_LC_reg));

}

void Scaler_hdr_setting_SEAMLESS_hdmi_position(unsigned char hdr_enable, unsigned char hdr_type)
{

//	_system_setting_info *system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	static unsigned char hdrType;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(VIP_system_info_structure_table == NULL){
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s:%d] error here!! system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	if(hdr_enable)
	{

		unsigned int vdec_cur_hdr_type;
		if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) {	/* HDMI case*/
			/*20181121 roger, using hdr_type for HDMI case*/
			vdec_cur_hdr_type = hdr_type;
		} else {	/* DTV and USB */
			seamless_change_sync_info *seamless_info = NULL;
			seamless_info = (seamless_change_sync_info *)Scaler_GetShareMemVirAddr(SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG);
			if (seamless_info)
			{
				vdec_cur_hdr_type = seamless_info->vdec_cur_hdr_type;
				vdec_cur_hdr_type = fwif_color_ChangeOneUINT32Endian(vdec_cur_hdr_type, 1);
			}
			else
			{
				pr_err("[seamless position] ERROR, get NULL HDR type!!\n");
				vdec_cur_hdr_type = HDR_DM_MODE_NONE;
			}
		}

		if (vdec_cur_hdr_type == HDR_DM_HDR10)
		{
			PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_HDR10;
			PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT2020;
			hdrType = HDR_DM_HDR10;
		}
		else if (vdec_cur_hdr_type == HDR_DM_HLG)
		{
			PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_HLG;
			PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT2020;
			hdrType = HDR_DM_HLG;
		}
		else if (hdr_type == HDR_DM_SDR_MAX_RGB)
		{
			PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_SDR_MAX_RGB;
			PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT2020;
			hdrType = HDR_DM_SDR_MAX_RGB;
		}
		else if (vdec_cur_hdr_type == HDR_DM_MODE_NONE)
		{
			PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_SDR;
			PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT709;
			hdrType = HDR_DM_MODE_NONE;
		}
		else
		{
			pr_err("[seamless position] ERROR, get unknown HDR type!! hdr_type = %d\n", vdec_cur_hdr_type);
			PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_SDR;
			PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT709;
			hdrType = HDR_DM_MODE_NONE;
		}


	}
	else
	{
		PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_SDR;
		PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT709;
		hdrType = HDR_DM_MODE_NONE;
		//system_info_struct->Debug_Buff_8[2]++;
	}

	Scaler_color_set_HDR_Type(hdrType);
	drvif_TV006_SET_HDR_mode_Seamless_hdmi_position(PQModeInfo_flag_seamless);
	//Scaler_Set_HDR_YUV2RGB(VIP_system_info_structure_table->HDR_flag, HAL_VPQ_COLORIMETRY_BT2020);	// move to "Scaler_color_set_HDR_AutoRun_TBL()", can't use semaphore in ISR
	drvif_color_HDR10_DM_setting();

	if(Scaler_color_Get_HDR_AutoRun())
		Scaler_color_Access_HDR_AutoRun_TBL_Flag(1, &hdrType);

}

void Scaler_hdr_setting_SEAMLESS_hdmi_finish(unsigned char hdr_enable, unsigned char hdr_type)
{
	//fwif_color_SetDataFormatHandler_tv006_Seamless(hdr_enable);
	drvif_TV006_SET_HDR_mode_Seamless_hdmi_finish(PQModeInfo_flag_seamless);
	//Scaler_SetDataFormatHandle_Seamless();
}


void drvif_TV006_SET_HDR_mode_Seamless(UINT32* modeinfo)
{
	//_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	extern unsigned char g_HDR3DLUTForceWrite;
	pr_info("[HDR new flow][seamless], type = %d\n", modeinfo[0]);

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	/*rtd_printk(KERN_INFO, TAG_NAME, "drvif_TV006_SET_HDR_mode, modeinfo = %d, Colorimetry info = %d\n", modeinfo[0], modeinfo[1]);*/
	if (modeinfo[1] == 0) modeinfo[1] = 2;	//data protect
	//SET_HDR_mode
	switch(modeinfo[0])	//HDR mode
	{
	case HAL_VPQ_HDR_MODE_HDR10:
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		Scaler_LGE_HDR10_Enable_Seamless(modeinfo[1]);
		fwif_color_set_HDR10_runmode(SLR_MAIN_DISPLAY);
		//drvif_DM2_GAMMA_Enable(g_InvGamma);	// set by hal or HFC
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);

		#ifdef CONFIG_SUPPORT_IPQ
		drvif_color_set_BBC_Enable(1);
		#else
		drvif_color_set_BBC_Enable(0);
		#endif
#if 0
#ifdef CONFIG_HDR_SDR_SEAMLESS
		drvif_DM2_EOTF_Enable(1, 0);
		drvif_DM2_OETF_Enable(1, 0);
#endif
#endif

	break;
	case HAL_VPQ_HDR_MODE_HLG:
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		Scaler_LGE_HDR10_Enable_Seamless(modeinfo[1]);
		fwif_color_set_HDR10_runmode(SLR_MAIN_DISPLAY);
		drvif_color_set_BBC_Enable(1);
		//drvif_DM2_GAMMA_Enable(g_InvGamma);	// set by hal or HFC
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);
#if 0
#ifdef CONFIG_HDR_SDR_SEAMLESS

		drvif_DM2_EOTF_Enable(1, 1);
		drvif_DM2_OETF_Enable(1, 1);
		drvif_DM2_OETF_Set((unsigned short *)&OETF_LUT_HLG_DEFAULT_DCC_OFF, (unsigned short *)&OETF_LUT_HLG_DEFAULT_DCC_OFF, (unsigned short *)&OETF_LUT_HLG_DEFAULT_DCC_OFF);

#endif
#endif
	break;

	case HAL_VPQ_HDR_MODE_SDR_MAX_RGB:
			VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
			Scaler_LGE_HDR10_Enable_Seamless(modeinfo[1]);
			fwif_color_set_HDR10_runmode(SLR_MAIN_DISPLAY);
			drvif_color_set_BBC_Enable(0);
			//drvif_DM2_GAMMA_Enable(0);		// set by hal or HFC
			//drvif_color_set_BBC_shift_bit(0);
			//drvif_color_Set_ST2094_3Dlut_CInv(1024, 1);
			fwif_color_TC_Flow(0);
			drvif_DM2_OETF_EOTF_Path_Sel(1);
			fwif_color_set_DM2_Hist_AutoMode_Enable(1);

	break;

	case HAL_VPQ_HDR_MODE_SDR:
	case HAL_VPQ_HDR_MODE_HDREFFECT:
		//HDR10 and HLG off
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 0;
		g_HDR3DLUTForceWrite = 1;
		/*drvif_TV006_SET_HDR_mode_off();*/
		/*drvif_color_set_BBC_Enable(0);*/
		Scaler_LGE_HDR10_Enable_Seamless(0);
		fwif_color_set_DM2_Hist_AutoMode_Enable(0);

	break;

	default:
		rtd_printk(KERN_ERR, TAG_NAME, "drvif_TV006_SET_HDR_mode, get unknow mode = %d\n", modeinfo[0]);
	break;
	}

}

void drvif_TV006_SET_HDR_mode_Seamless_hdmi_position(UINT32* modeinfo)
{
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	extern unsigned char g_HDR3DLUTForceWrite;

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	pr_info("[HDR new flow][seamless][HDMI position], type = %d\n", modeinfo[0]);
	/*rtd_printk(KERN_INFO, TAG_NAME, "drvif_TV006_SET_HDR_mode, modeinfo = %d, Colorimetry info = %d\n", modeinfo[0], modeinfo[1]);*/
	if (modeinfo[1] == 0) modeinfo[1] = 2;	//data protect
	//SET_HDR_mode
	switch(modeinfo[0])	//HDR mode
	{
	case HAL_VPQ_HDR_MODE_HDR10:
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(modeinfo[1]);
		//drvif_DM2_GAMMA_Enable(g_InvGamma);		// set by hal or HFC
		fwif_color_set_HDR10_runmode(SLR_MAIN_DISPLAY);
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);

		#ifdef CONFIG_SUPPORT_IPQ
		drvif_color_set_BBC_Enable(1);
		#else
		drvif_color_set_BBC_Enable(0);
		#endif
	break;
	case HAL_VPQ_HDR_MODE_HLG:
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(modeinfo[1]);
		drvif_color_set_BBC_Enable(1);
		//drvif_DM2_GAMMA_Enable(g_InvGamma);		// set by hal or HFC
		fwif_color_set_HDR10_runmode(SLR_MAIN_DISPLAY);
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);
	break;
	case HAL_VPQ_HDR_MODE_SDR:
	case HAL_VPQ_HDR_MODE_HDREFFECT:
		//HDR10 and HLG off
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 0;
		g_HDR3DLUTForceWrite = 1;
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(0);
		fwif_color_set_DM2_Hist_AutoMode_Enable(0);

	break;

	case HAL_VPQ_HDR_MODE_SDR_MAX_RGB:
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(modeinfo[1]);
		drvif_color_set_BBC_Enable(0);
		//drvif_DM2_GAMMA_Enable(0);		// set by hal or HFC
		fwif_color_set_HDR10_runmode(SLR_MAIN_DISPLAY);
		//drvif_color_set_BBC_shift_bit(0);
		//drvif_color_Set_ST2094_3Dlut_CInv(1024, 1);
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);
	break;

	default:
		rtd_printk(KERN_ERR, TAG_NAME, "drvif_TV006_SET_HDR_mode, get unknow mode = %d\n", modeinfo[0]);
	break;
	}

}


void drvif_TV006_SET_HDR_mode_Seamless_hdmi_finish(UINT32* modeinfo)
{
	pr_info("[HDR new flow][seamless][HDMI finish], type = %d\n", modeinfo[0]);

 	if (modeinfo[1] == 0) modeinfo[1] = 2;	//data protect
	//SET_HDR_mode
	switch(modeinfo[0])	//HDR mode
	{
	case HAL_VPQ_HDR_MODE_HDR10:
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_finish(modeinfo[1]);
	break;
	case HAL_VPQ_HDR_MODE_HLG:
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_finish(modeinfo[1]);
	break;
	case HAL_VPQ_HDR_MODE_SDR:
	case HAL_VPQ_HDR_MODE_HDREFFECT:
		//HDR10 and HLG off
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_finish(0);
	break;

	case HAL_VPQ_HDR_MODE_SDR_MAX_RGB:
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_finish(modeinfo[1]);

	default:
		rtd_printk(KERN_ERR, TAG_NAME, "drvif_TV006_SET_HDR_mode, get unknow mode = %d\n", modeinfo[0]);
	break;
	}

}


void Scaler_LGE_HDR10_Enable_Seamless(unsigned char enable)
{
	//RGB2YUV_BLACK_LEVEL blk_lv;
	//UINT8 video_format;
	//_system_setting_info *VIP_system_info_structure_table;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "blk_lv=%d, videoFormat=%d, HDR mode = %d\n", blk_lv, video_format, get_HDR_mode());*/

	//blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();
	//video_format = drvif_Hdmi_GetColorSpace();
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_HDR10_enable_Coef] = enable;
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "[HDR10],videoFMT=%d,hdr_mode=%d\n", video_format, get_HDR_mode());*/
	if (enable != 0)	//enable HDR 10
	{
		if (enable > 2) enable = 1; //protect
		drvif_color_set_HDR10_Enable_Seamless(hdr_table[1]);	//enable process
#if 0// set CSC1 in scalervpq hal for tv006, and in Scaler_color_set_HDR_AutoRun_TBL() for tv01 @ seamless, set i3d CSC in Scaler_SetDataFormatHandle_Seamless
		if(blk_lv == RGB2YUV_BLACK_LEVEL_LOW) { /* use limit2full*/
			if ((video_format == 0) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)) {/*HDMI HDR RGB case*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(1, &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255][0]), 1);
			} else if (enable == 1) { /* bt 709 */
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255]);
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(0, NULL, 0);
			} else { /* bt 2020*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(0, NULL, 0);
			}
		} else {	/* use limit2limit*/
			if ((video_format == 0) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)) {/*HDMI HDR RGB case*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(1, &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255][0]), 0);
			} else if (enable == 1) {/* bt 709 */
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_255_to_255]);
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(0, NULL, 0);
			} else { /* bt 2020*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(0, NULL, 0);
			}
		}
#endif
		fwif_color_set_Main_Dither(TRUE,VIP_DITHER_MAINTABLE_HDR10_12to10);
	}
	else
	{
		drvif_color_set_HDR10_Enable_Seamless(hdr_table[0]);	//disable process
	}
}

void Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(unsigned char enable)
{
	//RGB2YUV_BLACK_LEVEL blk_lv;
	//UINT8 video_format;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table;
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "blk_lv=%d, videoFormat=%d, HDR mode = %d\n", blk_lv, video_format, get_HDR_mode());*/

	//blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();
	//video_format = drvif_Hdmi_GetColorSpace();
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_HDR10_enable_Coef] = enable;
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "[HDR10],videoFMT=%d,hdr_mode=%d\n", video_format, get_HDR_mode());*/
	if (enable != 0)	//enable HDR 10
	{
		if (enable > 2) enable = 1; //protect
		drvif_color_set_HDR10_Enable_Seamless_hdmi_position(hdr_table[1]);	//enable process
#if 0// set CSC1 in scalervpq hal for tv006, and in Scaler_color_set_HDR_AutoRun_TBL() for tv01 @ seamless
		if(blk_lv == RGB2YUV_BLACK_LEVEL_LOW) { /* use limit2full*/
			if ((video_format == 0) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)) {/*HDMI HDR RGB case*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);
			} else if (enable == 1) { /* bt 709 */
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255]);
			} else { /* bt 2020*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);
			}
		} else {	/* use limit2limit*/
			if ((video_format == 0) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)) {/*HDMI HDR RGB case*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);
			} else if (enable == 1) {/* bt 709 */
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT709_255_to_255]);
			} else { /* bt 2020*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255]);
			}
		}
#endif
	}
	else
	{
		drvif_color_set_HDR10_Enable_Seamless_hdmi_position(hdr_table[0]);	//disable process
	}
}

void Scaler_LGE_HDR10_Enable_Seamless_hdmi_finish(unsigned char enable)
{
	//RGB2YUV_BLACK_LEVEL blk_lv;
	//UINT8 video_format;
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "blk_lv=%d, videoFormat=%d, HDR mode = %d\n", blk_lv, video_format, get_HDR_mode());*/

	//blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();
	//video_format = drvif_Hdmi_GetColorSpace();
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "[HDR10],videoFMT=%d,hdr_mode=%d\n", video_format, get_HDR_mode());*/
	if (enable != 0)	//enable HDR 10
	{
		if (enable > 2) enable = 1; //protect
		drvif_color_set_HDR10_Enable_Seamless_hdmi_finish(hdr_table[1]);	//enable process
#if 0// set i3d CSC in Scaler_SetDataFormatHandle_Seamless
		if(blk_lv == RGB2YUV_BLACK_LEVEL_LOW) { /* use limit2full*/
			if ((video_format == 0) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)) {/*HDMI HDR RGB case*/
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(1, &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255][0]), 1);
			} else if (enable == 1) { /* bt 709 */
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(0, NULL, 0);
			} else { /* bt 2020*/
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(0, NULL, 0);
			}
		} else {	/* use limit2limit*/
			if ((video_format == 0) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)) {/*HDMI HDR RGB case*/
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(1, &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255][0]), 0);
			} else if (enable == 1) {/* bt 709 */
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(0, NULL, 0);
			} else { /* bt 2020*/
				drvif_HDMI_HDR_I3DDMA_RGB2YUV422(0, NULL, 0);
			}
		}
#endif
		fwif_color_set_Main_Dither(TRUE,VIP_DITHER_MAINTABLE_HDR10_12to10);
	}
	else
	{
		drvif_color_set_HDR10_Enable_Seamless_hdmi_finish(hdr_table[0]);	//enable process
	}
}
#if 0
void Scaler_SetDataFormatHandle_Seamless(void)
{
	unsigned char en_422to444, InputSrcGetType;
	#ifdef CONFIG_FORCE_RUN_I3DDMA
	unsigned char InputSrcGetFrom;
	#endif
	unsigned short mode, channel;
	SCALER_DISP_CHANNEL display;

	//down(&VPQ_I_CSC_Semaphore);	// can't use semaphore in ISR
	mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	channel = Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL);
	en_422to444 = (unsigned char)Scaler_DispGetStatus(display, SLR_DISP_422);
	InputSrcGetType = Scaler_InputSrcGetType(display);

#ifdef CONFIG_FORCE_RUN_I3DDMA
	/*
	if((display == SLR_MAIN_DISPLAY) && (get_force_i3ddma_enable(display) == true))
		InputSrcGetType = _SRC_VO;
	else
		InputSrcGetType = Scaler_InputSrcGetType(display);
	*/
	if (fwif_color_get_force_run_i3ddma_enable(display)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &InputSrcGetType, &InputSrcGetFrom);
		mode = fwif_color_get_cur_input_timing_mode(SLR_MAIN_DISPLAY);
	}
#endif
	if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
		fwif_color_SetDataFormatHandler(display, mode, channel, en_422to444, InputSrcGetType);
	else
		fwif_color_SetDataFormatHandler_tv006();
	//up(&VPQ_I_CSC_Semaphore);
}
#endif
#endif

static unsigned char* gHDR_AutoRun_TBL_Flag = NULL;
unsigned char*  Scaler_color_Access_HDR_AutoRun_TBL_Flag(unsigned char isSetFlag, unsigned char *HDR_AutoRun_TBL_Flag)
{
	if (isSetFlag == 1) {
		gHDR_AutoRun_TBL_Flag = HDR_AutoRun_TBL_Flag;
		rtd_printk(KERN_INFO, TAG_NAME, "PQ_Mode_Info_TBL=%p\n",gHDR_AutoRun_TBL_Flag);
		if (gHDR_AutoRun_TBL_Flag != NULL)
			rtd_printk(KERN_INFO, TAG_NAME, "PQ_Mode_Info_TBL[0]=%d\n", *gHDR_AutoRun_TBL_Flag);
	}
	return gHDR_AutoRun_TBL_Flag;
}

void Scaler_color_set_HDR_VPQ_TSK(void)
{
	unsigned char *HDR_AutoRun_TBL_Flag;
	unsigned char vpq_hdr_type_active;
	_system_setting_info *vipSys_info = NULL;
	HDR_AutoRun_TBL_Flag = Scaler_color_Access_HDR_AutoRun_TBL_Flag(0, 0);

	if (HDR_AutoRun_TBL_Flag != NULL) {
		vipSys_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
		if(vipSys_info == NULL){
			rtd_printk(KERN_EMERG, TAG_NAME, "Scaler_color_set_HDR__VPQ_TSK, sys inof NULL!!!!\n");
			return;
		}
		vpq_hdr_type_active = vipSys_info->HDR_flag;
		rtd_printk(KERN_INFO, TAG_NAME, "seamless, table sta\n");
		Scaler_Set_HDR_YUV2RGB(SLR_MAIN_DISPLAY, vpq_hdr_type_active, HAL_VPQ_COLORIMETRY_BT2020);
		Scaler_SetDataFormatHandler();
		Scaler_color_HDRIP_AutoRun(HDR_AutoRun_TBL_Flag[0]);
		//Scaler_color_HDRInfo_Handler(PQ_Mode_Info);
		if (vpq_hdr_type_active == vipSys_info->HDR_flag) {
			Scaler_color_Access_HDR_AutoRun_TBL_Flag(1, NULL);
			rtd_printk(KERN_INFO, TAG_NAME, "seamless, VPQ_TSK done\n");
		} else {
			rtd_printk(KERN_INFO, TAG_NAME, "seamless, VPQ_TSK, HDR flag changed, vipSys_info->HDR_flag=%d, seamless active hdr type=%d,\n",
				vipSys_info->HDR_flag, vpq_hdr_type_active);
		}
	}
}

void Scaler_color_HDRIP_AutoRun(unsigned char HDR_Type)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
//========check HDR statu ===========
	unsigned char bHDR_flag = false;
	//unsigned char bDolby_HDR_flag = false;
	unsigned char input_src_type;
	unsigned char input_src_form;
	//unsigned char input_timming_info;
	unsigned int modeinfo[5];
#ifdef CONFIG_CUSTOMER_TV002
	extern unsigned char g_bHDR_is601;
#endif

	{
		if (fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC)) {
			fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_MAIN_DISPLAY), &input_src_type, &input_src_form);
			//input_timming_info 	= drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
		} else {
			input_src_type = Scaler_InputSrcGetMainChType();
			input_src_form = Scaler_InputSrcGetMainChFrom();
			//input_timming_info 	= Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
		}
	}

	fwif_vip_HDR10_check(SLR_MAIN_DISPLAY,input_src_type);
#if 0	/* move to "Scaler_color_set_HDR_Type()"*/
	if (HDR_Type == HDR_DM_DOLBY) {
		VIP_system_info_structure_table ->HDR_flag = HAL_VPQ_HDR_MODE_SDR;
		VIP_system_info_structure_table->DolbyHDR_flag = 1;
	} else if (HDR_Type == HDR_DM_HDR10) {
		VIP_system_info_structure_table ->HDR_flag = HAL_VPQ_HDR_MODE_HDR10;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	} else if (HDR_Type == HDR_DM_HLG) {
		VIP_system_info_structure_table ->HDR_flag = HAL_VPQ_HDR_MODE_HLG;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	} else if (HDR_Type == HDR_DM_TECHNICOLOR) {
		VIP_system_info_structure_table ->HDR_flag = HAL_VPQ_HDR_MODE_TECHNICOLOR;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	} else if (HDR_Type == HDR_DM_ST2094) {
		VIP_system_info_structure_table ->HDR_flag = HAL_VPQ_HDR_MODE_ST2094;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	}
#endif
	bHDR_flag = VIP_system_info_structure_table ->HDR_flag;
	//bDolby_HDR_flag = VIP_system_info_structure_table->DolbyHDR_flag;
	//rtd_printk(KERN_INFO, TAG_NAME, "==== HDR_flag =%d,  input_src_type =%d  =======\n",bHDR_flag,input_src_type);
//==========================================

	if(Scaler_color_Get_HDR_AutoRun()) {
		memset(modeinfo, 0, sizeof(modeinfo));

		if(HDR_Type == HDR_DM_HDR10 || HDR_Type == HDR_DM_HLG || HDR_Type == HDR_DM_TECHNICOLOR || HDR_Type == HDR_DM_ST2094) {	//	HDR10
			modeinfo[0] = bHDR_flag;
			if(VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1)
				modeinfo[1]=HAL_VPQ_COLORIMETRY_BT2020;
#ifdef CONFIG_CUSTOMER_TV002
			else if (g_bHDR_is601)
				modeinfo[1] = HAL_VPQ_COLORIMETRY_BT601;
			else
				modeinfo[1] = HAL_VPQ_COLORIMETRY_BT709;
#else
			else if(VIP_system_info_structure_table->Timing >= _MODE_720P50)  //hd, 2k,4k
				modeinfo[1] = HAL_VPQ_COLORIMETRY_BT709;
			else
				modeinfo[1] = HAL_VPQ_COLORIMETRY_BT601;  // sd
#endif
		} else if (HDR_Type == HDR_DM_SDR_MAX_RGB) {
				modeinfo[0] = bHDR_flag;
				if(VIP_system_info_structure_table->Timing >= _MODE_720P50)  //hd, 2k,4k
					modeinfo[1] = HAL_VPQ_COLORIMETRY_BT709;
				else
					modeinfo[1] = HAL_VPQ_COLORIMETRY_BT601;  // sd
		} else {
			modeinfo[0] = HAL_VPQ_HDR_MODE_SDR;
			if(VIP_system_info_structure_table->Timing >= _MODE_720P50)  //hd, 2k,4k
				modeinfo[1] = HAL_VPQ_COLORIMETRY_BT709;
			else
				modeinfo[1] = HAL_VPQ_COLORIMETRY_BT601;  // sd
		}
		#if 0
		Scaler_color_HDRInfo_Update(modeinfo);
		#else	/*control by HDR table*/
		Scaler_color_HDRInfo_Handler(modeinfo);
		#endif
	}
	rtd_printk(KERN_INFO, TAG_NAME, "HDR auto, HDR_Type=%d, input_src_type=%d, i3d_en=%d, Get_DisplayMode_Src=%d\n",
		HDR_Type, input_src_type, fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY), Get_DisplayMode_Src(SLR_MAIN_DISPLAY));

}

void Scaler_color_HDRInfo_Update(UINT32* modeinfo)
{
	//extern UINT16 HDR_3DLUT_Compact[HDR_24x24x24_size*3];
	//extern UINT32 EOTF_LUT_R[1025];
	//extern UINT16 OETF_LUT_R[1025];

	//extern unsigned int HLG_EOTF_LUT_R[EOTF_size];
	//extern unsigned short HLG_OETF_LUT_R[OETF_size];
	//extern unsigned short HLG_3DLUT_24x24x24[HDR_24x24x24_size][3];

	//_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	//_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	extern unsigned char g_HDR3DLUTForceWrite;

	rtd_printk(KERN_INFO, TAG_NAME, "Scaler_color_HDRInfo_Update, modeinfo = %d, Colorimetry info = %d\n", modeinfo[0], modeinfo[1]);
	if (modeinfo[1] == 0) modeinfo[1] = 2;	//data protect
	//SET_HDR_mode
	switch(modeinfo[0])	//HDR mode
	{
		case HAL_VPQ_HDR_MODE_HDR10:
			//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
			//Scaler_LGE_HDR10_Enable(modeinfo[1]);
			Scaler_Set_HDR_YUV2RGB(SLR_MAIN_DISPLAY, modeinfo[0], modeinfo[1]);
#if 0	/* set HDR10 Enable in "scaler_hdr_mode_setting()"*/
			Scaler_LGE_HDR10_Enable_newflow(modeinfo[1]);

			drvif_color_set_BBC_Enable(0);
#endif
			drvif_DM2_GAMMA_Enable(0);
			fwif_color_DM2_EOTF_OETF_3D_24x24x24_LUT_Handler(HAL_VPQ_HDR_MODE_HDR10);
			fwif_HDR_RGB2OPT(1, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_DCI]);
			fwif_color_set_TC_FrameSync_Enable(0);
			fwif_color_set_ST2094_FrameSync_Enable(0);
			//rtd_printk(KERN_INFO, TAG_NAME, "HDR10 mode EO[1] = %d, OE[1] = %d\n", EOTF_LUT_R[1], OETF_LUT_R[1]);
		break;
		case HAL_VPQ_HDR_MODE_HLG:
			//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
			//Scaler_LGE_HDR10_Enable(modeinfo[1]);
			Scaler_Set_HDR_YUV2RGB(SLR_MAIN_DISPLAY, modeinfo[0], modeinfo[1]);
#if 0	/* set HDR10 Enable in "scaler_hdr_mode_setting()"*/
			Scaler_LGE_HDR10_Enable_newflow(modeinfo[1]);
			drvif_color_set_BBC_Enable(1);
#endif
			drvif_DM2_GAMMA_Enable(0);		/* gamma is embebed in 3D LUT*/
			fwif_color_DM2_EOTF_OETF_3D_24x24x24_LUT_Handler(HAL_VPQ_HDR_MODE_HLG);
			fwif_HDR_RGB2OPT(0, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_DCI]); //fix_me, mark it due to cause HLG contour
			fwif_color_set_TC_FrameSync_Enable(0);
			fwif_color_set_ST2094_FrameSync_Enable(0);
			//rtd_printk(KERN_INFO, TAG_NAME, "HLG mode EO[1] = %d, OE[1] = %d\n", HLG_EOTF_LUT_R[1], HLG_OETF_LUT_R[1]);
		break;
		case HAL_VPQ_HDR_MODE_TECHNICOLOR:
			drvif_DM2_GAMMA_Enable(0);
			drvif_DM2_OETF_Enable(0, 0);
			drvif_DM2_EOTF_Enable(0, 0);
			drvif_DM2_B0203_noSram_Enable(1, 0);
			//fwif_color_set_TC_v130_metadata2reg_init(0, 100, 2, 0, 0, 1);
			fwif_color_set_TC_v140_metadata2reg_init(0, 100, 2,  1);
			fwif_color_set_TC_FrameSync_Enable(1);
			fwif_color_set_ST2094_FrameSync_Enable(0);

		break;
		case HAL_VPQ_HDR_MODE_ST2094:
			//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
			Scaler_Set_HDR_YUV2RGB(SLR_MAIN_DISPLAY, modeinfo[0], modeinfo[1]);
			drvif_DM2_GAMMA_Enable(0);		// gamma 2.2 is embeded in OETF
			fwif_color_DM2_EOTF_OETF_3D_24x24x24_LUT_Handler(HAL_VPQ_HDR_MODE_ST2094);
			//drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);
			fwif_HDR_RGB2OPT(1, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_DCI]);
			fwif_color_set_TC_FrameSync_Enable(0);
			fwif_color_set_ST2094_FrameSync_Enable(1);
			//rtd_printk(KERN_INFO, TAG_NAME, "HDR10 mode EO[1] = %d, OE[1] = %d\n", EOTF_LUT_R[1], OETF_LUT_R[1]);
		break;
		case HAL_VPQ_HDR_MODE_SDR:
		case HAL_VPQ_HDR_MODE_HDREFFECT:
			//HDR10 and HLG off
			//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 0;
			g_HDR3DLUTForceWrite = 1;
			fwif_color_set_TC_FrameSync_Enable(0);
			fwif_color_set_ST2094_FrameSync_Enable(0);
			//drvif_TV006_SET_HDR_mode_off();
		break;
		default:
			rtd_printk(KERN_ERR, TAG_NAME, "Scaler_color_HDRInfo_Update, get unknow mode = %d\n", modeinfo[0]);
		break;

	}

	//Scaler_SetDataFormatHandler();

}


HDR_flow_control_timing Scaler_color_get_HDR_timing(void)
{
	HDR_flow_control_timing result = HFC_SDR_YUV;	/*default*/
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	UINT8 hdmi_video_format = drvif_Hdmi_GetColorSpace();
	UINT8 input_timming_info = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	UINT8 uhd_flag = 0;
	UINT8 vo_video_format = Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE);
	UINT8 input_src_type = Scaler_InputSrcGetMainChType();
	UINT8 input_VO_Form	= Scaler_VOFrom(Scaler_Get_CurVoInfo_plane());
	UINT8 input_TVD_Form = Scaler_InputSrcGetMainFormat();
	UINT8 usb_flag = 0;
	if (VIP_system_info_structure_table == NULL) return result;
	if (input_timming_info >= _MODE_4k2kI30 && input_timming_info <= _MODE_4k2kP60)
		uhd_flag = 1;
	if ((input_src_type == _SRC_VO) && (input_VO_Form != VO_FROM_JPEG) && (input_TVD_Form == TVD_INFMT_VODMA1))
		usb_flag = 1;

	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]HDR timing info:\n");
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]DolbyHDR_flag = %d\n", VIP_system_info_structure_table->DolbyHDR_flag);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]HDR_flag = %d\n", VIP_system_info_structure_table->HDR_flag);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]hdmi_video_format = %d\n", hdmi_video_format);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]input_timming_info = %d\n", input_timming_info);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]uhd_flag = %d\n", uhd_flag);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]vo_video_format = %d\n", vo_video_format);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]input_src_type = %d\n", input_src_type);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]input_VO_Form = %d\n", input_VO_Form);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]input_TVD_Form = %d\n", input_TVD_Form);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]usb_flag = %d\n", usb_flag);

	if (VIP_system_info_structure_table->DolbyHDR_flag) {	//dolby case
		result = HFC_DOLBY;
	} else if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {
		result = HFC_SDR_MAX_RGB;
	} else if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_SDR) {	//SDR case
		if (hdmi_video_format == 0)
			result = HFC_SDR_RGB;
		else
			result = HFC_SDR_YUV;
	} else if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10) {	/*HDR10*/
		if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) {		/*HDMI*/
			if (hdmi_video_format == 0) {			/*RGB*/
				if (uhd_flag) result = HFC_HDMI_HDR10_RGB_UHD;
				else result = HFC_HDMI_HDR10_RGB_HD;
			} else if (hdmi_video_format == 1) {	/*YUV422*/
				if (uhd_flag) result = HFC_HDMI_HDR10_YUV422_UHD;
				else result = HFC_HDMI_HDR10_YUV422_HD;
			} else if (hdmi_video_format == 2) {	/*YUV444*/
				if (uhd_flag) result = HFC_HDMI_HDR10_YUV444_UHD;
				else result = HFC_HDMI_HDR10_YUV444_HD;
			} else {								/*YUV420*/
				if (uhd_flag) result = HFC_HDMI_HDR10_YUV420_UHD;
				else result = HFC_HDMI_HDR10_YUV420_HD;
			}
		} else if (usb_flag) {													/*USB*/
			if (vo_video_format == COLOR_RGB) {			/*RGB*/
				if (uhd_flag) result = HFC_USB_HDR10_RGB_UHD;
				else result = HFC_USB_HDR10_RGB_HD;
			} else if (vo_video_format == COLOR_YUV422) {	/*YUV422*/
				if (uhd_flag) result = HFC_USB_HDR10_YUV422_UHD;
				else result = HFC_USB_HDR10_YUV422_HD;
			} else if (vo_video_format == COLOR_YUV444) {	/*YUV444*/
				if (uhd_flag) result = HFC_USB_HDR10_YUV444_UHD;
				else result = HFC_USB_HDR10_YUV444_HD;
			} else {								/*YUV420*/
				if (uhd_flag) result = HFC_USB_HDR10_YUV420_UHD;
				else result = HFC_USB_HDR10_YUV420_HD;
			}
		} else {																/*DTV*/
			if (vo_video_format == COLOR_RGB) {			/*RGB*/
				if (uhd_flag) result = HFC_DTV_HDR10_RGB_UHD;
				else result = HFC_DTV_HDR10_RGB_HD;
			} else if (vo_video_format == COLOR_YUV422) {	/*YUV422*/
				if (uhd_flag) result = HFC_DTV_HDR10_YUV422_UHD;
				else result = HFC_DTV_HDR10_YUV422_HD;
			} else if (vo_video_format == COLOR_YUV444) {	/*YUV444*/
				if (uhd_flag) result = HFC_DTV_HDR10_YUV444_UHD;
				else result = HFC_DTV_HDR10_YUV444_HD;
			} else {								/*YUV420*/
				if (uhd_flag) result = HFC_DTV_HDR10_YUV420_UHD;
				else result = HFC_DTV_HDR10_YUV420_HD;
			}
		}
	} else if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HLG) {	//HLG case
		if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) {		/*HDMI*/
			if (hdmi_video_format == 0) {			/*RGB*/
				if (uhd_flag) result = HFC_HDMI_HLG_RGB_UHD;
				else result = HFC_HDMI_HLG_RGB_HD;
			} else if (hdmi_video_format == 1) {	/*YUV422*/
				if (uhd_flag) result = HFC_HDMI_HLG_YUV422_UHD;
				else result = HFC_HDMI_HLG_YUV422_HD;
			} else if (hdmi_video_format == 2) {	/*YUV444*/
				if (uhd_flag) result = HFC_HDMI_HLG_YUV444_UHD;
				else result = HFC_HDMI_HLG_YUV444_HD;
			} else {								/*YUV420*/
				if (uhd_flag) result = HFC_HDMI_HLG_YUV420_UHD;
				else result = HFC_HDMI_HLG_YUV420_HD;
			}
		} else if (usb_flag) {													/*USB*/
			if (vo_video_format == COLOR_RGB) {			/*RGB*/
				if (uhd_flag) result = HFC_USB_HLG_RGB_UHD;
				else result = HFC_USB_HLG_RGB_HD;
			} else if (vo_video_format == COLOR_YUV422) {	/*YUV422*/
				if (uhd_flag) result = HFC_USB_HLG_YUV422_UHD;
				else result = HFC_USB_HLG_YUV422_HD;
			} else if (vo_video_format == COLOR_YUV444) {	/*YUV444*/
				if (uhd_flag) result = HFC_USB_HLG_YUV444_UHD;
				else result = HFC_USB_HLG_YUV444_HD;
			} else {								/*YUV420*/
				if (uhd_flag) result = HFC_USB_HLG_YUV420_UHD;
				else result = HFC_USB_HLG_YUV420_HD;
			}
		} else {																/*DTV*/
			if (vo_video_format == COLOR_RGB) {			/*RGB*/
				if (uhd_flag) result = HFC_DTV_HLG_RGB_UHD;
				else result = HFC_DTV_HLG_RGB_HD;
			} else if (vo_video_format == COLOR_YUV422) {	/*YUV422*/
				if (uhd_flag) result = HFC_DTV_HLG_YUV422_UHD;
				else result = HFC_DTV_HLG_YUV422_HD;
			} else if (vo_video_format == COLOR_YUV444) {	/*YUV444*/
				if (uhd_flag) result = HFC_DTV_HLG_YUV444_UHD;
				else result = HFC_DTV_HLG_YUV444_HD;
			} else {								/*YUV420*/
				if (uhd_flag) result = HFC_DTV_HLG_YUV420_UHD;
				else result = HFC_DTV_HLG_YUV420_HD;
			}
		}
	} else if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR) {	//TECHNICOLOR case
		result = HFC_TC;
	} else if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_ST2094) {	//HDR10+ case
		result = HFC_ST2094;
	} else {
		result = HFC_SDR_YUV;	/*default*/
	}

	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]HDR timing = %d\n", result);
	return result;
}

HDR_sub_flow_control_timing Scaler_color_get_HDR_sub_timing(void)
{
	HDR_sub_flow_control_timing result = HFC_HDR_sub_SDR;	/*default*/
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) return result;

	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]HDR sub timing info:\n");
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]HDR sub_flag = %d\n", VIP_system_info_structure_table->HDR_flag);
	rtd_printk(KERN_INFO, TAG_NAME, "[HFC]HDR2 sub_flag = %d\n", VIP_system_info_structure_table->HDR2_flag);

	if (VIP_system_info_structure_table->HDR2_flag == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {
		result = HFC_HDR_sub_MAX_RGB;
		
	} else if (VIP_system_info_structure_table->HDR2_flag == HAL_VPQ_HDR_MODE_HDR10) {
		result = HFC_HDR_sub_HDR10;

	} else if (VIP_system_info_structure_table->HDR2_flag == HAL_VPQ_HDR_MODE_HLG) {
		result = HFC_HDR_sub_HLG;
		
	} else {
		result = HFC_HDR_sub_SDR;
	}

	rtd_printk(KERN_INFO, TAG_NAME, "[HFC] sub result = %d\n", result);
	return result;

}

void Scaler_color_HFC_set_HDR_RGB2OPT(UINT8 tbl_index, unsigned char color_format)
{
	if (ColorFMT_601_in_IMD_Domain_Enable == 1) {
		#ifdef CONFIG_CUSTOMER_TV002
		fwif_HDR_RGB2OPT(0, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_DCI]);
		#else
		if (tbl_index == HFC_RGB2OPT_OFF)
			fwif_HDR_RGB2OPT(0, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_DCI]);
		else if (color_format == HAL_VPQ_COLORIMETRY_BT709)
			fwif_HDR_RGB2OPT(0, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_DCI]);
		else
			fwif_HDR_RGB2OPT(1, hdr_RGB2OPT[HDR_RGB2OPT_RGB2020_TO_RGB709_F2F]);
		#endif
	} else if (tbl_index == HFC_RGB2OPT_OFF)
		fwif_HDR_RGB2OPT(0, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_DCI]);
	else
		fwif_HDR_RGB2OPT(1, hdr_RGB2OPT[tbl_index]);
}

void Scaler_color_HFC_set_HDR1_2_RGB2OPT(UINT8 tbl_index)
{
	if (tbl_index == HFC_RGB2OPT_OFF)
		fwif_HDR1_2_RGB2OPT(0, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_DCI]);
	else
		fwif_HDR1_2_RGB2OPT(1, hdr_RGB2OPT[tbl_index]);
}

void Scaler_color_HFC_set_HDR2_Gamma1(UINT8 tbl_index)
{
	if (tbl_index >= 3)
		tbl_index = 0;
	// gamma1
	fwif_color_set_HDR_Sub_Gamma1(tbl_index, HFC_HDR2_gamma_1[tbl_index], HFC_HDR2_gamma_1[tbl_index], HFC_HDR2_gamma_1[tbl_index]);
}

void Scaler_color_HFC_set_HDR2_colorMap(UINT8 tbl_index)
{
	if (tbl_index >= HDR_SUB_TABLE_NUM)
		tbl_index = 0;
	fwif_color_HDR_sub_set_colorMap( &(HDR_sub_colorMap[tbl_index]) );
}

void Scaler_color_HFC_set_HDR2_Gamma2(UINT8 tbl_index)
{
	if (tbl_index >= 3)
		tbl_index = 0;
	// gamma2
	fwif_color_set_HDR_Sub_Gamma2(tbl_index, HFC_HDR2_gamma_2[tbl_index], HFC_HDR2_gamma_2[tbl_index], HFC_HDR2_gamma_2[tbl_index]);
}

void Scaler_color_HFC_set_TC_FrameSync_Enable(UINT8 enable)
{
	if (enable) fwif_color_set_TC_v140_metadata2reg_init(0, 100, 2,  1);
	fwif_color_set_TC_FrameSync_Enable(enable);
}


void Scaler_color_HDRInfo_Update_by_HFC(UINT8 HFC_bit, HDR_flow_control_timing HDR_timing, UINT32* modeinfo)
{
	extern unsigned char g_HDR3DLUTForceWrite;
	extern HDR_flow_control HDR_control[HFC_SDR_TIMING_NUM];

#ifndef CONFIG_CUSTOMER_TV002
	if (modeinfo[1] == 0) modeinfo[1] = 2;	//data protect
#endif

	// use "Scaler_Set_HDR_YUV2RGB()", and move to "scaler_hdr_mode_setting"
	//if ((HDR_control[HDR_timing].CSC1_ctrl.IPCtrlBits) & HFC_bit)
		//Scaler_Set_HDR_YUV2RGB_by_HFC(HDR_control[HDR_timing].CSC1_table, modeinfo[1], modeinfo[0]);

	fwif_color_DM2_EOTF_OETF_3D_LUT_by_HFC(HFC_bit, HDR_timing);

	if ((HDR_control[HDR_timing].HDR_33_1_ctrl.IPCtrlBits) & HFC_bit)
		Scaler_color_HFC_set_HDR_RGB2OPT(HDR_control[HDR_timing].HDR_33_1_SPtable, modeinfo[1]);

	if ((HDR_control[HDR_timing].TC_ctrl.IPCtrlBits) & HFC_bit)
		Scaler_color_HFC_set_TC_FrameSync_Enable(HDR_control[HDR_timing].TC_enbale);

	if ((HDR_control[HDR_timing].ST2094_ctrl.IPCtrlBits) & HFC_bit)
		fwif_color_set_ST2094_FrameSync_Enable(HDR_control[HDR_timing].ST2094_enbale);

	if ((HDR_control[HDR_timing].DM2_gamma_ctrl.IPCtrlBits) & HFC_bit)
		fwif_DM2_GAMMA_Enable(HDR_control[HDR_timing].DM2_gamma_enable);

}

void Scaler_color_HDR_sub_Info_Update_by_HFC(UINT8 HFC_bit, HDR_sub_flow_control_timing HDR_timing)
{
	extern HDR_sub_flow_control HDR_sub_control[HFC_HDR_sub_TIMING_NUM];

	// HDR1_2
	fwif_color_DM2_HDR1_2_EOTF_OETF_3D_LUT_by_HFC(HFC_bit, HDR_timing);
	
	if ((HDR_sub_control[HDR_timing].HDR_33_1_ctrl.IPCtrlBits) & HFC_bit)
		Scaler_color_HFC_set_HDR1_2_RGB2OPT(HDR_sub_control[HDR_timing].HDR_33_1_SPtable);

	if ((HDR_sub_control[HDR_timing].DM2_gamma_ctrl.IPCtrlBits) & HFC_bit)
		fwif_DM2_HDR1_2_GAMMA_Enable(HDR_sub_control[HDR_timing].DM2_gamma_enable);

	// HDR2
	if ((HDR_sub_control[HDR_timing].Gamma1_ctrl.IPCtrlBits) & HFC_bit)
		Scaler_color_HFC_set_HDR2_Gamma1(HDR_sub_control[HDR_timing].Gamma1_table);

	if ((HDR_sub_control[HDR_timing].Color_Mapping_ctrl.IPCtrlBits) & HFC_bit)
		Scaler_color_HFC_set_HDR2_colorMap(HDR_sub_control[HDR_timing].Color_Mapping_value);

	if ((HDR_sub_control[HDR_timing].Gamma2_ctrl.IPCtrlBits) & HFC_bit)
		Scaler_color_HFC_set_HDR2_Gamma2(HDR_sub_control[HDR_timing].Gamma2_table);
		
}

void Scaler_color_HDRInfo_Handler(UINT32* modeinfo)
{
	HDR_flow_control_timing HDR_timing = Scaler_color_get_HDR_timing();
	Scaler_color_HDRInfo_Update_by_HFC(HFC_control_by_PQ, HDR_timing, modeinfo);
}

void Scaler_color_HDR_sub_Info_Handler(void)
{
	HDR_sub_flow_control_timing HDR_sub_timing = Scaler_color_get_HDR_sub_timing();
	Scaler_color_HDR_sub_Info_Update_by_HFC(HFC_control_by_PQ, HDR_sub_timing);
}

void Scaler_LGE_ONEKEY_HDR10_Enable(unsigned char enable)
{
#if 0
	extern UINT32 hdr_RGB2OPT[HDR_RGB2OPT_TABLE_NUM][9];
	extern UINT16 HDR10_3DLUT_24x24x24[24*24*24*3];
	extern UINT32 EOTF_LUT_R[1025];
	extern UINT16 OETF_LUT_R[1025];

  	//_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned int PQModeInfo_flag_onekey_en[5];
	static PQ_HDR_3dLUT_16_PARAM args;
	args.c3dLUT_16_enable = 1;
	args.a3dLUT_16_pArray = (UINT16*)&HDR10_3DLUT_24x24x24[0];

	if (enable) {
		/* VPQ_IOC_SET_PQModeInfo*/
		PQModeInfo_flag_onekey_en[0] = HAL_VPQ_HDR_MODE_HDR10;
		PQModeInfo_flag_onekey_en[1] = HAL_VPQ_COLORIMETRY_BT2020;
		drvif_TV006_SET_HDR_mode(PQModeInfo_flag_onekey_en);
		Scaler_SetDataFormatHandler();

		/* VPQ_IOC_SET_HDR_GAMUTMATRIX */
		fwif_HDR_RGB2OPT(1, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_709]);

		/*VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_1, VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_2*/
		fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 0);
		//fwif_color_rtice_DM2_3D_24x24x24_LUT_Set(1, &HDR10_3DLUT_24x24x24[0]);

		/*VPQ_IOC_SET_HDR_Eotf*/
		fwif_color_set_DM2_EOTF_TV006(EOTF_LUT_R, 0); //SEAMLESS mode block it
		//printk("[eli]EO_0=%d,%d,%d", EOTF_LUT_R[0], EOTF_LUT_R[1], EOTF_LUT_R[2]);
		//fwif_color_rtice_DM2_EOTF_Set(EOTF_LUT_R, EOTF_LUT_R, EOTF_LUT_R);

		/*VPQ_IOC_SET_HDR_Oetf*/
		fwif_color_set_DM2_OETF_TV006(OETF_LUT_R, 0);
		//printk("[eli]OE_0=%d,%d,%d", OETF_LUT_R[0], OETF_LUT_R[1], OETF_LUT_R[2]);
		//fwif_color_rtice_DM2_OETF_Set(OETF_LUT_R, OETF_LUT_R, OETF_LUT_R);

		/*VPQ_IOC_SET_HDR_InvGamma*/
		fwif_DM2_GAMMA_Enable(0);

		/* decide HDR*/
		fwif_OneKey_HDR_HLG_Decide_HDR(PQModeInfo_flag_onekey_en[0]);
	} else {
		/* VPQ_IOC_SET_PQModeInfo*/
		PQModeInfo_flag_onekey_en[0] = HAL_VPQ_HDR_MODE_SDR;
		PQModeInfo_flag_onekey_en[1] = HAL_VPQ_COLORIMETRY_BT2020;
		drvif_TV006_SET_HDR_mode(PQModeInfo_flag_onekey_en);
		Scaler_SetDataFormatHandler();
		fwif_OneKey_HDR_HLG_Decide_HDR(PQModeInfo_flag_onekey_en[0]);
	}

#endif
}

void Scaler_LGE_ONEKEY_BBC_Enable(unsigned char enable)
{
#if 0
	extern UINT32 hdr_RGB2OPT[HDR_RGB2OPT_TABLE_NUM][9];
	extern UINT16 HDR10_3DLUT_24x24x24[24*24*24*3];
	extern UINT32 EOTF_LUT_R[1025];
	extern UINT16 OETF_LUT_R[1025];

//  	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned int PQModeInfo_flag_onekey_en[5];
	static PQ_HDR_3dLUT_16_PARAM args;
	args.c3dLUT_16_enable = 1;
	args.a3dLUT_16_pArray = (UINT16*)&HDR10_3DLUT_24x24x24[0];

	if (enable) {
		/* VPQ_IOC_SET_PQModeInfo*/
		PQModeInfo_flag_onekey_en[0] = HAL_VPQ_HDR_MODE_HLG;
		PQModeInfo_flag_onekey_en[1] = HAL_VPQ_COLORIMETRY_BT2020;
		drvif_TV006_SET_HDR_mode(PQModeInfo_flag_onekey_en);
		Scaler_SetDataFormatHandler();

		/* VPQ_IOC_SET_HDR_GAMUTMATRIX */
		fwif_HDR_RGB2OPT(1, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_709]);

		/*VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_1, VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_2*/
		fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 0);
		//fwif_color_rtice_DM2_3D_24x24x24_LUT_Set(1, &HDR10_3DLUT_24x24x24[0]);

		/*VPQ_IOC_SET_HDR_Eotf*/
		fwif_color_set_DM2_EOTF_TV006(EOTF_LUT_R, 0); //SEAMLESS mode block it
		//printk("[eli]EO_0=%d,%d,%d", EOTF_LUT_R[0], EOTF_LUT_R[1], EOTF_LUT_R[2]);
		//fwif_color_rtice_DM2_EOTF_Set(EOTF_LUT_R, EOTF_LUT_R, EOTF_LUT_R);

		/*VPQ_IOC_SET_HDR_Oetf*/
		fwif_color_set_DM2_OETF_TV006(OETF_LUT_R, 0);
		//printk("[eli]OE_0=%d,%d,%d", OETF_LUT_R[0], OETF_LUT_R[1], OETF_LUT_R[2]);
		//fwif_color_rtice_DM2_OETF_Set(OETF_LUT_R, OETF_LUT_R, OETF_LUT_R);

		/*VPQ_IOC_SET_HDR_InvGamma*/
		fwif_DM2_GAMMA_Enable(1);

		/* decide HDR*/
		fwif_OneKey_HDR_HLG_Decide_HDR(HAL_VPQ_HDR_MODE_HLG);
	} else {
		/* VPQ_IOC_SET_PQModeInfo*/
		PQModeInfo_flag_onekey_en[0] = HAL_VPQ_HDR_MODE_SDR;
		PQModeInfo_flag_onekey_en[1] = HAL_VPQ_COLORIMETRY_BT2020;
		drvif_TV006_SET_HDR_mode(PQModeInfo_flag_onekey_en);
		Scaler_SetDataFormatHandler();
		fwif_OneKey_HDR_HLG_Decide_HDR(PQModeInfo_flag_onekey_en[0]);
	}

#endif
}


UINT32 EOTF_LUT_SDR2HDR[1025] = {0};
void Scaler_LGE_ONEKEY_SDR2HDR_Enable(unsigned char enable)
{
#if 0
	extern UINT16 OETF_LUT_R[1025];
	dm_hdr_double_buffer_ctrl_RBUS dm_double_buffer_ctrl_reg;
	UINT16 i;

	//base no HDR10
	Scaler_LGE_ONEKEY_HDR10_Enable(1);


	//make identity EOTF
	for (i = 1; i < 1024; i++)
		EOTF_LUT_SDR2HDR[i] = (1 << 22) * i;
	EOTF_LUT_SDR2HDR[1024] = 0xffffffff;
	fwif_color_rtice_DM2_EOTF_Set(EOTF_LUT_SDR2HDR, EOTF_LUT_SDR2HDR, EOTF_LUT_SDR2HDR);
	fwif_color_rtice_DM2_OETF_Set(OETF_LUT_R, OETF_LUT_R, OETF_LUT_R);
	drvif_Set_DM_HDR_3dLUT_24x24x24_16(1, HDR10_3DLUT_24x24x24);

	dm_double_buffer_ctrl_reg.regValue = IoReg_Read32(DM_HDR_Double_Buffer_CTRL_reg);
	dm_double_buffer_ctrl_reg.dm_db_en = 1;
	IoReg_Write32(DM_HDR_Double_Buffer_CTRL_reg, dm_double_buffer_ctrl_reg.regValue);


	//DM status for SDR2HDR
	IoReg_Write32(DM_dm_submodule_enable_reg, 0x00000402);
	//apply
	dm_double_buffer_ctrl_reg.regValue = IoReg_Read32(DM_HDR_Double_Buffer_CTRL_reg);
	dm_double_buffer_ctrl_reg.dm_db_apply = 1;
	IoReg_Write32(DM_HDR_Double_Buffer_CTRL_reg, dm_double_buffer_ctrl_reg.regValue);

	//no channel swap
	IoReg_Write32(VGIP_VGIP_CHN1_MISC_reg, 0);
#endif	
}


void Scaler_color_set_m_nowSource(unsigned char value) {

	m_nowSource = value;

}


unsigned char Scaler_color_get_m_nowSource(void) {

	return m_nowSource;

}

unsigned char  Scaler_Set_I_De_XC(unsigned char value)
{
#if 0
	SLR_VIP_TABLE *gVIP_Table;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned char which_vip_src = 0;

	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	gVIP_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if (VIP_system_info_structure_table == NULL || gVIP_Table == NULL) {
		value = 255;
	} else {
		which_vip_src = VIP_system_info_structure_table->VIP_source;
		value = gVIP_Table->VIP_QUALITY_Extend_Coef[which_vip_src][VIP_QUALITY_FUNCTION_I_De_XC];
	}

	/*dirty solution, scaler flow don't go "drv_memory_set_ip_fifo_Mer2" while PC Mode, turn off de-xc in PC mode*/
	if ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP) == 0) && (Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_RTNR) == 0))
		value = 255;
#endif
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	pTable->I_De_XC= value;

	return fwif_color_Set_I_De_XC(value);

}
unsigned char Scaler_Get_I_De_XC(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->I_De_XC;
}

unsigned char  Scaler_Set_I_De_Contour(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	pTable->I_De_Contour= value;

	return fwif_color_Set_I_De_Contour(value);

}
unsigned char Scaler_Get_I_De_Contour(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->I_De_Contour;
}

unsigned char Scaler_Set_Block_DeContour(unsigned char value)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	unsigned char status1, status2;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	pTable->I_De_Contour= value;

	status1 = fwif_color_Set_Block_DeContour(value);
	status2 = fwif_color_Set_Block_DeContour_Size();

	return (status1 & status2);
}

void  Scaler_Set_BOE_RGBW(int w_rate)
{
	// 20160627 jyyang
	fwif_color_set_BOE_RGBW(w_rate);
}

UINT32 andMASK_Table[32]=	{
							0x00000001, 0x00000003, 0x00000007, 0x0000000f,
							0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
							0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
							0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
							0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
							0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
							0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
							0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
							};

extern PQ_device_struct *g_Share_Memory_PQ_device_struct;
extern bool PQ_check_source_table[PQ_check_MAX][VIP_QUALITY_SOURCE_NUM];

unsigned int vpq_PQ_get_register_value(unsigned int pq_register, unsigned char pq_bitup, unsigned char pq_bitlow){
	unsigned int pq_check_register_value;
	unsigned int and_reg;
	pq_check_register_value = IoReg_Read32(pq_register);
	and_reg = andMASK_Table[(pq_bitup-pq_bitlow)]<<pq_bitlow;
	return ((pq_check_register_value&and_reg)>>pq_bitlow);
}

extern PQ_check_struct PQ_check_register_table[200];

void vpq_PQ_check_register_value(char *item_name, unsigned char source, unsigned int *PQ_check_error, unsigned int *PQ_check_error_count){
	unsigned int i=0;
	unsigned int pq_check_register_value;
	//unsigned int and_reg;

	for(i=0; i<200; i++){
		if(PQ_check_register_table[i].PQ_check_table_register.pq_check_register == 0)
			break;
		if((strcmp(PQ_check_register_table[i].PQ_check_table_item, item_name) != 0)&&(strcmp("ALL", item_name) != 0))
			continue;

		if(PQ_check_source_table[PQ_check_register_table[i].PQ_check_source[0]][source] && PQ_check_source_table[PQ_check_register_table[i].PQ_check_source[1]][source] && PQ_check_source_table[PQ_check_register_table[i].PQ_check_source[2]][source] ){
			pq_check_register_value = vpq_PQ_get_register_value(PQ_check_register_table[i].PQ_check_table_register.pq_check_register, PQ_check_register_table[i].PQ_check_table_register.pq_check_bitup, PQ_check_register_table[i].PQ_check_table_register.pq_check_bitlow);

			if(pq_check_register_value != PQ_check_register_table[i].PQ_check_table_value){
				PQ_check_error[(10*(*PQ_check_error_count))+0] = (unsigned int)source;
				PQ_check_error[(10*(*PQ_check_error_count))+1] = (unsigned int)PQ_check_register_table[i].PQ_check_table_register.pq_check_register;
				PQ_check_error[(10*(*PQ_check_error_count))+2] = (unsigned int)PQ_check_register_table[i].PQ_check_table_register.pq_check_bitup;
				PQ_check_error[(10*(*PQ_check_error_count))+3] = (unsigned int)PQ_check_register_table[i].PQ_check_table_register.pq_check_bitlow;
				PQ_check_error[(10*(*PQ_check_error_count))+4] = 0;
				PQ_check_error[(10*(*PQ_check_error_count))+5] = 0;
				PQ_check_error[(10*(*PQ_check_error_count))+6] = 0;
				PQ_check_error[(10*(*PQ_check_error_count))+7] = 0;
				PQ_check_error[(10*(*PQ_check_error_count))+8] = (unsigned int)PQ_check_register_table[i].PQ_check_table_value;
				PQ_check_error[(10*(*PQ_check_error_count))+9] = (unsigned int)pq_check_register_value;
				(*PQ_check_error_count)++;
			}
			if (NULL == g_Share_Memory_PQ_device_struct){
				rtd_printk(KERN_DEBUG, TAG_NAME, "g_Share_Memory_PQ_device_struct == NULL\n");
			}else{
				//if(g_Share_Memory_PQ_device_struct->pqautocheck_en){
				//	rtd_printk(KERN_DEBUG, TAG_NAME, "PQ_auto_check_error source %d, %x[%d,%s]\n", source, PQ_check_register_table[i].PQ_check_table_register.pq_check_register, PQ_check_register_table[i].PQ_check_table_register.pq_check_bitup, PQ_check_register_table[i].PQ_check_table_register.pq_check_bitlow);
				}
			}
		}
	}





extern PQ_check_diff_struct PQ_check_register_diff_table[200];

void vpq_PQ_check_diff_register_value(char *item_name, unsigned char source, unsigned int *PQ_check_error, unsigned int *PQ_check_error_count){
	unsigned int i=0;
	unsigned int  pq_check_value=0;
	unsigned int pq_check_register_value_1;
	unsigned int pq_check_register_value_2;
	for(i=0; i<200; i++){
		if((PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_register == 0) && (PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_2.pq_check_register == 0))
			break;
		if((strcmp(PQ_check_register_diff_table[i].PQ_check_register_diff_table_item, item_name) != 0)&&(strcmp("ALL", item_name) != 0))
			continue;

		if(PQ_check_source_table[PQ_check_register_diff_table[i].PQ_check_register_diff_table_source[0]][source] && PQ_check_source_table[PQ_check_register_diff_table[i].PQ_check_register_diff_table_source[1]][source] && PQ_check_source_table[PQ_check_register_diff_table[i].PQ_check_register_diff_table_source[2]][source] ){
			pq_check_register_value_1 = vpq_PQ_get_register_value(PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_register, PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_bitup, PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_bitlow);
			pq_check_register_value_2 = vpq_PQ_get_register_value(PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_2.pq_check_register, PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_2.pq_check_bitup, PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_2.pq_check_bitlow);
			if(PQ_check_register_diff_table[i].PQ_check_register_diff_table_Operand == '+')
				pq_check_value = pq_check_register_value_1 + pq_check_register_value_2;
			else if(PQ_check_register_diff_table[i].PQ_check_register_diff_table_Operand == '-')
				pq_check_value = pq_check_register_value_1 - pq_check_register_value_2;
			else if(PQ_check_register_diff_table[i].PQ_check_register_diff_table_Operand == '&')
				pq_check_value = pq_check_register_value_1 & pq_check_register_value_2;
			else if(PQ_check_register_diff_table[i].PQ_check_register_diff_table_Operand == '|')
				pq_check_value = pq_check_register_value_1 | pq_check_register_value_2;

			if(pq_check_value != PQ_check_register_diff_table[i].PQ_check_register_diff_table_value){
				PQ_check_error[(10*(*PQ_check_error_count))+0] = (unsigned int)source;
				PQ_check_error[(10*(*PQ_check_error_count))+1] = (unsigned int)PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_register;
				PQ_check_error[(10*(*PQ_check_error_count))+2] = (unsigned int)PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_bitup;
				PQ_check_error[(10*(*PQ_check_error_count))+3] = (unsigned int)PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_bitlow;
				PQ_check_error[(10*(*PQ_check_error_count))+4] = (unsigned int)PQ_check_register_diff_table[i].PQ_check_register_diff_table_Operand;
				PQ_check_error[(10*(*PQ_check_error_count))+5] = (unsigned int)PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_2.pq_check_register;
				PQ_check_error[(10*(*PQ_check_error_count))+6] = (unsigned int)PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_2.pq_check_bitup;
				PQ_check_error[(10*(*PQ_check_error_count))+7] = (unsigned int)PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_2.pq_check_bitlow;
				PQ_check_error[(10*(*PQ_check_error_count))+8] = (unsigned int)PQ_check_register_diff_table[i].PQ_check_register_diff_table_value;
				PQ_check_error[(10*(*PQ_check_error_count))+9] = (unsigned int)pq_check_value;
				(*PQ_check_error_count)++;
			}
			if (NULL == g_Share_Memory_PQ_device_struct){
				rtd_printk(KERN_DEBUG, TAG_NAME, "g_Share_Memory_PQ_device_struct == NULL\n");
			}else{
			//	if(g_Share_Memory_PQ_device_struct->pqautocheck_en){
			//		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ_auto_check_error source %d, %x[%d,%s]\n", source, PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_register, PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_bitup, PQ_check_register_diff_table[i].PQ_check_register_diff_table_register_1.pq_check_bitlow);
				}
			}
		}
	}


void vpq_PQ_check_timming(char *item_name, unsigned int *PQ_check_error) {
	unsigned char source = 255;
	unsigned int PQ_check_error_count = 1;

	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);
	vpq_PQ_check_register_value(item_name, source, PQ_check_error, &PQ_check_error_count);
	vpq_PQ_check_diff_register_value(item_name, source, PQ_check_error, &PQ_check_error_count);
	PQ_check_error[0] = PQ_check_error_count-1;
	PQ_check_error[1] = 0;
	PQ_check_error[2] = 0;
	PQ_check_error[3] = 0;
	PQ_check_error[4] = 0;
	PQ_check_error[5] = 0;
	PQ_check_error[6] = 0;
	PQ_check_error[7] = 0;
	PQ_check_error[8] = 0;
	PQ_check_error[9] = 0;

}
#if 0/*SLD, hack, elieli*/ //k5l remove
unsigned char  Scaler_Set_SLD(unsigned char value)
{
	DRV_Still_Logo_Detection_Table disable = {{0}};
	DRV_Still_Logo_Detection_Table *ptr;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	SLR_VIP_TABLE *gVip_Table =NULL;
	StructColorDataType *pTable = NULL;

	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	pTable->SLD= value;


	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 1;
	}

	if (value >= SLD_Table_NUM)
		ptr = &disable;
	else
		ptr = &gVip_Table->Still_Logo_Detection_Table[value];

	VIP_system_info_structure_table->PQ_Setting_Info.SLD_INFO.table_sel = value;

	fwif_color_set_SLD_input_size(VIP_system_info_structure_table->D_Width, VIP_system_info_structure_table->D_Height);

	fwif_color_set_SLD_Ctrl(0, ptr);
	fwif_color_set_SLD_curve_map(0, ptr);
	fwif_color_set_SLD_drop(0, ptr);
	return 0;

}
unsigned char Scaler_Get_SLD(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->SLD;
}
#endif

void Scaler_color_colorwrite_Output_InvOutput_gamma(unsigned char value)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	StructColorDataType *pTable = NULL;

	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->Output_InvOutput_GAMMA= value;


	fwif_color_colorwrite_Output_gamma(value);
	fwif_color_colorwrite_InvOutput_gamma(value);

}
unsigned char Scaler_Get_Output_InvOutput_gamma(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	return pTable->Output_InvOutput_GAMMA;
}

void Scaler_Share_Memory_Access_VIP_TABLE_Custom_Struct(unsigned long arg)
{
	printk("Scaler_Share_Memory_Access_VIP_TABLE_Custom_Struct\n");
	g_Share_Memory_VIP_TABLE_Custom_Struct_isr= ((SLR_VIP_TABLE_CUSTOM_TV001 *)arg);
}

unsigned char VIP_PQ_IP_Enable_Monitor_Bypass_TBL[VIP_PQ_ByPass_ITEMS_Max] = {0};
char Scaler_scalerVIP_PQ_IP_Enable_Monitor(SLR_VIP_TABLE* VIP_TBL, _RPC_clues* RPC_SmartPic_clue, _clues* SmartPic_clue, _RPC_system_setting_info* RPC_sysInfoTBL, _system_setting_info* sysInfoTBL, unsigned int MA_print_count)
{
	extern VIP_PQ_ByPass_Struct PQ_ByPass_Struct[VIP_PQ_ByPass_TBL_Max];
	unsigned char Monitor_Bypass_TBL[VIP_PQ_ByPass_ITEMS_Max] = {0};
	unsigned char Bypass_TBL[VIP_PQ_ByPass_ITEMS_Max] = {0};
	unsigned char PQ_Bypass_lv = 0, i = 0;
	unsigned char isIMode_Flag = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);
	unsigned int frame_rate = 0, height = 0;
	//SLR_VOINFO pVOINFO = NULL;
	// Debounce
	static unsigned char isFilm = 40;
	// Film information from MEMC
	// FixMe, 20190925
	//unsigned char *FilmInfo = Scaler_MEMC_GetCadence();

	PQ_Bypass_lv = RPC_sysInfoTBL->PQ_ByPass_Ctrl.table_idx;

	// FixMe, 20190925
	#if 0
	if(FilmInfo[_FILM_MEMC_OUT_ALL] == 0 && isFilm > 0)
		isFilm--;
	else if( FilmInfo[_FILM_MEMC_OUT_ALL] != 0 )
		isFilm = 40;
	#endif

        isFilm =40; //condiction need to check when memc enable

	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) ==  VSC_INPUTSRC_VDEC) {
		frame_rate = Scaler_VOInfoPointer(0)->v_freq;
		height = Scaler_VOInfoPointer(0)->v_length;
	} else {
		frame_rate = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
		height = sysInfoTBL->I_Height;
	}
	/* condition 1, 4k/60, video*/
	if ((height > 1300) && (frame_rate > 310) && (isFilm == 0))
		Monitor_Bypass_TBL[PQ_ByPass_MCNR] = 1;

	for (i=0;i<VIP_PQ_ByPass_ITEMS_Max;i++) {
		Bypass_TBL[i] = Monitor_Bypass_TBL[i] + PQ_ByPass_Struct[PQ_Bypass_lv].VIP_PQ_ByPass_TBL[i];
		if (Bypass_TBL[i] > 1)
			Bypass_TBL[i] = 1;

		isIMode_Flag = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);
		if (isIMode_Flag == 1) {
			;
			Bypass_TBL[PQ_ByPass_I_DI_IP_Enable] = 0;
			Bypass_TBL[PQ_ByPass_I_DI] = 0;
		}

		// avoid race condition, can not control "DI_IM_DI_RTNR_CONTROL_reg" in ISR.
		Bypass_TBL[PQ_ByPass_RTNR_Y] = 0;
		Bypass_TBL[PQ_ByPass_RTNR_C] = 0;

		if ((Bypass_TBL[i] - VIP_PQ_IP_Enable_Monitor_Bypass_TBL[i]) > 0)	/* bypass case*/
			drvif_color_set_PQ_ByPass(i, isIMode_Flag);
		//else if ((Bypass_TBL[i] - VIP_PQ_IP_Enable_Monitor_Bypass_TBL[i]) < 0)	/* resort case*/
			//Scaler_scalerVIP_PQ_IP_Enable_Monitor_Resort_check(i);
	}

	/* save pre status */
	memcpy(VIP_PQ_IP_Enable_Monitor_Bypass_TBL, Bypass_TBL, sizeof(char)*VIP_PQ_ByPass_ITEMS_Max);

	return 0;
}
#if 0
char Scaler_scalerVIP_PQ_IP_Enable_Monitor_Resort_check(unsigned char item)
{
	unsigned char isEnable = 0;

	if (item == PQ_ByPass_I_DI_IP_Enable) {
		//if (Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP) || Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_RTNR))	/* use clk to implement bypass, don't check this flag while resort*/
			isEnable = 0;
	} else if (item ==PQ_ByPass_I_DI) {
		isEnable = 0;
	} else if (item == PQ_ByPass_MCNR) {
		isEnable = 0;
	}

	if (isEnable == 1)
		drvif_color_set_PQ_ByPass_Resort(item);

	return 0;
}
#endif
void Scaler_PQ_IP_Enable_Monitor_clear_Buff(void)
{
	unsigned char i;

	for (i=0;i<VIP_PQ_ByPass_ITEMS_Max;i++)
		VIP_PQ_IP_Enable_Monitor_Bypass_TBL[i] = 0;
}
char Scaler_color_Set_PQ_ByPass_Lv(unsigned char table_idx)
{
	char ret;
  	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (VIP_RPC_system_info_structure_table == NULL) {
		VIPprintf("~table NULL, [%s:%d], Return\n~", __FILE__, __LINE__);
		return 0;
	}
	ret = fwif_color_PQ_ByPass_Handler(table_idx, 1, VIP_RPC_system_info_structure_table, 0);

	return ret;
}

char Scaler_color_Get_PQ_ByPass_Lv(void)
{
	char ret;
  	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (VIP_RPC_system_info_structure_table == NULL) {
		VIPprintf("~table NULL, [%s:%d], Return\n~", __FILE__, __LINE__);
		return 0;
	}

	ret = fwif_color_PQ_ByPass_Handler(0, 0, VIP_RPC_system_info_structure_table, 0);

	return ret;
}

char Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance(unsigned short *OETF_TBL, unsigned short forceIndex)
{
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL;
	unsigned char input_src_type;
	unsigned char input_src_form;
	//unsigned char input_timming_info;
	SLR_VOINFO* pVOInfo = NULL;
	vfe_hdmi_drm_t drm_info;
	unsigned short MaxCLL, panel_Luminance, MaxFALL;

	if (fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_MAIN_DISPLAY), &input_src_type, &input_src_form);
		//input_timming_info 	= drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
	} else {
		input_src_type = Scaler_InputSrcGetMainChType();
		input_src_form = Scaler_InputSrcGetMainChFrom();
		//input_timming_info 	= Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	}

	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	/* get drm info*/
	vfe_hdmi_drv_get_drm_info(&drm_info);

	if (pVOInfo == NULL || OETF_TBL == NULL) {
		rtd_printk(KERN_EMERG, TAG_NAME, "Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance, table=NULL, pVOInfo=%p, OETF_TBL=%p, return !!!!\n", pVOInfo, OETF_TBL);
		return -1;
	}

	if(input_src_type == _SRC_VO) {
		MaxCLL = pVOInfo->MaxCLL;
		MaxFALL = pVOInfo->MaxFALL;
	} else if (input_src_type == _SRC_HDMI) {
		MaxCLL = drm_info.maximum_content_light_level;
		MaxFALL = drm_info.maximum_frame_average_light_level;
	} else {
		rtd_printk(KERN_INFO, TAG_NAME, "Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance, is not VO or HDMI, SRC=%d\n", input_src_type);
		return 1;
	}

	panel_Luminance = HDR10_OETF_ByLuminace_TBL.panel_Luminace;

	rtd_printk(KERN_INFO, TAG_NAME, "OETF get, input_src_type=%d, panel_Luminance=%d, MaxCLL=%d,MaxFALL=%d, forceIndex=%d\n",
		input_src_type, panel_Luminance, MaxCLL, MaxFALL, forceIndex);

	/*MaxCLL == 0, can not get maxCLL*/
	if ((MaxCLL == 0) || (MaxCLL >= panel_Luminance))
		MaxCLL = panel_Luminance;

	if (forceIndex != 0)	/* force select oetf*/
		MaxCLL = forceIndex;

	if (MaxCLL <= 100)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_100N[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 150)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_150N[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 200)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_200N[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 250)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_250B[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 300)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_300B[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 350)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_350B[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 400)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_400B[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 500)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_500N6[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 600)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_600N6[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 660)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_660N6[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 700)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_700N6[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 800)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_800N6[0], sizeof(short)*OETF_size);
	else if (MaxCLL <= 1000)
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_1000N6[0], sizeof(short)*OETF_size);
	else
		memcpy((unsigned char*)OETF_TBL, (unsigned char*)&HDR10_OETF_ByLuminace_TBL.OETF_1000Over[0], sizeof(short)*OETF_size);

	return 0;
}

void Scaler_color_set_HDR_Type(unsigned char HDR_Type)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	rtd_printk(KERN_INFO, TAG_NAME, "setHDR_Type=%d, 1p_2p=%d, sub_1p=%d,\n", HDR_Type, drvif_Get_DM_HDR1_1p_2p(), drvif_Get_DM_sub_use_main_1p());
	if(VIP_system_info_structure_table == NULL){
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s:%d] error here!! system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	if (HDR_Type == HDR_DM_DOLBY) {
		VIP_system_info_structure_table->HDR_flag = HAL_VPQ_HDR_MODE_SDR;
		VIP_system_info_structure_table->DolbyHDR_flag = 1;
	} else if (HDR_Type == HDR_DM_HDR10) {
		VIP_system_info_structure_table->HDR_flag = HAL_VPQ_HDR_MODE_HDR10;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	} else if (HDR_Type == HDR_DM_HLG) {
		VIP_system_info_structure_table->HDR_flag = HAL_VPQ_HDR_MODE_HLG;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	} else if (HDR_Type == HDR_DM_TECHNICOLOR) {
		VIP_system_info_structure_table->HDR_flag = HAL_VPQ_HDR_MODE_TECHNICOLOR;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	} else if (HDR_Type == HDR_DM_ST2094) {
		VIP_system_info_structure_table->HDR_flag = HAL_VPQ_HDR_MODE_ST2094;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	} else if (HDR_Type == HDR_DM_SDR_MAX_RGB) {
		VIP_system_info_structure_table ->HDR_flag = HAL_VPQ_HDR_MODE_SDR_MAX_RGB;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	} else {
		VIP_system_info_structure_table->HDR_flag = HAL_VPQ_HDR_MODE_SDR;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

	}

}

void Scaler_color_set_HDR2_Type(unsigned char HDR_Type)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	rtd_printk(KERN_INFO, TAG_NAME, "setHDR2_Type=%d, 1p_2p=%d, sub_1p=%d,\n", HDR_Type, drvif_Get_DM_HDR1_1p_2p(), drvif_Get_DM_sub_use_main_1p());
	if(VIP_system_info_structure_table == NULL){
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s:%d] error here!! system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	if (HDR_Type == HDR_DM_DOLBY) {
		VIP_system_info_structure_table->HDR2_flag = HAL_VPQ_HDR_MODE_SDR;
	} else if (HDR_Type == HDR_DM_HDR10) {
		VIP_system_info_structure_table->HDR2_flag = HAL_VPQ_HDR_MODE_HDR10;

	} else if (HDR_Type == HDR_DM_HLG) {
		VIP_system_info_structure_table->HDR2_flag = HAL_VPQ_HDR_MODE_HLG;

	} else if (HDR_Type == HDR_DM_TECHNICOLOR) {
		VIP_system_info_structure_table->HDR2_flag = HAL_VPQ_HDR_MODE_TECHNICOLOR;

	} else if (HDR_Type == HDR_DM_HDR10PLUS) {
		VIP_system_info_structure_table->HDR2_flag = HAL_VPQ_HDR_MODE_ST2094;

	} else if (HDR_Type == HDR_DM_SDR_MAX_RGB) {
		VIP_system_info_structure_table ->HDR2_flag = HAL_VPQ_HDR_MODE_SDR_MAX_RGB;

	} else {
		VIP_system_info_structure_table->HDR2_flag = HAL_VPQ_HDR_MODE_SDR;

	}

}

void Scaler_color_Set_HDR_AutoRun(unsigned char nEnable)
{
	m_bHDR_AutoRun = nEnable;
	printk("HDR_AutoRun=%d  \n", m_bHDR_AutoRun);
}
unsigned char Scaler_color_Get_HDR_AutoRun(void)
{
	return m_bHDR_AutoRun;
}
void Scaler_color_Set_DCCProcess(unsigned char nEnable)
{
	m_bDCCProcess = nEnable;
	printk("DCCProcess=%d \n", m_bDCCProcess);
}
unsigned char Scaler_color_Get_DCCProcess(void)
{
	printk("DCCProcess=%d \n", m_bDCCProcess);
	return m_bDCCProcess;
}

void Scaler_color_copy_HDR_table_from_AP(SLR_HDR_TABLE* ptr_HDR_bin)
{
	extern unsigned int EOTF_LUT_R[1025];
	extern unsigned short OETF_LUT_R[1025];
	extern unsigned int HLG_EOTF_LUT_R[EOTF_size];
	extern unsigned short HLG_OETF_LUT_R[OETF_size];
	extern UINT32 HFC_EOTF_LUT_R[4][1025];
	extern unsigned short HFC_OETF_LUT_R[5][1025];
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL;
	extern VIP_TCHDR_Mode_TBL TCHDR_Mode_TBL;
	extern DRV_TCHDR_Table TCHDR_Table[TC_Table_NUM];
	extern DRV_TCHDR_COEF_I_lut TCHDR_COEF_I_LUT_TBL;
	extern DRV_TCHDR_COEF_S_lut TCHDR_COEF_S_LUT_TBL;
	extern DRV_TCHDR_v130_D_Table_settings TCHDR_v130_D_Table_settings[NUM_D_LUT_DEFAULT];

#ifndef	CONFIG_CUSTOMER_TV002
	//memcpy(hdr_table, ptr_HDR_bin->hdr_table, sizeof(hdr_table));
	//memcpy(hdr_RGB2OPT, ptr_HDR_bin->hdr_RGB2OPT, sizeof(hdr_RGB2OPT));
#endif
	memcpy(HDR10_3DLUT_17x17x17, ptr_HDR_bin->HDR10_3DLUT_17x17x17, sizeof(HDR10_3DLUT_17x17x17));
	//memcpy(HLG_3DLUT_17x17x17, ptr_HDR_bin->HDR10_3DLUT_17x17x17, sizeof(HLG_3DLUT_17x17x17));
	memcpy(EOTF_LUT_R, ptr_HDR_bin->EOTF_LUT_R, sizeof(EOTF_LUT_R));
	memcpy(OETF_LUT_R, ptr_HDR_bin->OETF_LUT_R, sizeof(OETF_LUT_R));
	memcpy(HLG_EOTF_LUT_R, ptr_HDR_bin->HLG_EOTF_LUT_R, sizeof(HLG_EOTF_LUT_R));
	memcpy(HLG_OETF_LUT_R, ptr_HDR_bin->HLG_OETF_LUT_R, sizeof(HLG_OETF_LUT_R));
	memcpy(HFC_EOTF_LUT_R, ptr_HDR_bin->HFC_EOTF_LUT_R, sizeof(HFC_EOTF_LUT_R));
	memcpy(HFC_OETF_LUT_R, ptr_HDR_bin->HFC_OETF_LUT_R, sizeof(HFC_OETF_LUT_R));
	memcpy(&HDR10_OETF_ByLuminace_TBL, &(ptr_HDR_bin->HDR10_OETF_ByLuminace_TBL), sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	memcpy(&TCHDR_Mode_TBL, &(ptr_HDR_bin->TCHDR_Mode_TBL), sizeof(VIP_TCHDR_Mode_TBL));
	memcpy(TCHDR_Table, ptr_HDR_bin->TCHDR_Table, sizeof(TCHDR_Table));
	memcpy(&TCHDR_COEF_I_LUT_TBL, &(ptr_HDR_bin->TCHDR_COEF_I_LUT_TBL), sizeof(DRV_TCHDR_COEF_I_lut));
	memcpy(&TCHDR_COEF_S_LUT_TBL, &(ptr_HDR_bin->TCHDR_COEF_S_LUT_TBL), sizeof(DRV_TCHDR_COEF_S_lut));
	memcpy(TCHDR_v130_D_Table_settings, ptr_HDR_bin->TCHDR_v130_D_Table_settings, sizeof(TCHDR_v130_D_Table_settings));

}

static short int  REG_DEGREE=115;
static short int  REG_DEGREE_LV2=120;
static short int  REG_DEGREE_LV3=125;

/**table_iDX
 * Level = 0 => no bypass?? * Level = 1 => I DCTI?�D_DCTI?�V_DCTI?�DLTI : bypass?? * Level = 2 => I DCTI?�D_DCTI?�V_DCTI?�DLTI?�RTNR Y?�RTNR C?�MCNR?�DI IP Enabl

 *  -> reg_deg : set(1)
 *  --> reg_deg+10 : set(2)
 *  reg_deg-5 <- : set(saved_level)
**/
extern int register_temperature_callback(int degree,void *fn, void* data, char *module_name);
extern void Check_smooth_toggle_update_flag(unsigned char display);
static int saved_level = -1;
void Scaler_PQ_thermal_callback(void *data, int cur_degree , int reg_degree, char *module_name)
{
	int tmp = saved_level;

	pr_debug("pq_bypass, cur_deg: %d, reg_deg:%d, saved:%d ->%d \n", cur_degree, reg_degree, tmp, saved_level);
	if (cur_degree >= REG_DEGREE_LV2) {
		if (saved_level == -1) {
			saved_level = Scaler_color_Get_PQ_ByPass_Lv();
			if (saved_level < 1)
				Scaler_color_Set_PQ_ByPass_Lv(2);
			else
				saved_level = -1;
			pr_debug("pq_bypass_1, cur_deg: %d, reg_deg:%d, saved:%d ->%d \n", cur_degree, reg_degree, tmp, saved_level);
		}

		if (cur_degree >= REG_DEGREE_LV3) {
			if (Scaler_color_Get_PQ_ByPass_Lv() != 3){
				Check_smooth_toggle_update_flag(SLR_MAIN_DISPLAY);
				Check_smooth_toggle_update_flag(SLR_SUB_DISPLAY);
				Scaler_color_Set_PQ_ByPass_Lv(3);
			}
			pr_debug("pq_bypass_2, cur_deg: %d, reg_deg:%d, saved:%d ->%d \n", cur_degree, reg_degree, tmp, saved_level);
		}
		return;
	}


	if (cur_degree <= REG_DEGREE) {
		if (saved_level != -1) { //restore
			//Scaler_color_Set_PQ_ByPass_Lv(saved_level);
			saved_level = -1;
			pr_debug("pq_bypass_-1, cur_deg: %d, reg_deg:%d, saved:%d ->%d \n", cur_degree, reg_degree, tmp, saved_level);
		}
		return;
	}
}

#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
int Scaler_register_PQ_thermal_handler(void)
{
        int ret;
        int retdata=0;

        pr_info("%s\n",__func__);
        if ((ret = register_temperature_callback(REG_DEGREE, Scaler_PQ_thermal_callback, (void*)&retdata, "PQ_ByPass_Lv")) < 0)
                printk("register PQ thermal handler fail, ret:%d \n", ret);

        return 0;
}

#if 0 //no use
void Scaler_register_PQ_thermal(int t0, int t1, int t2)
{
        if( (t2<135) && (t2> t1) && (t1 > t0))
        {
                REG_DEGREE=t0;
                REG_DEGREE_LV2=t1;
                REG_DEGREE_LV3=t2;
                Scaler_register_PQ_thermal_handler();//use resource table;
        }
        else
        {
                Scaler_register_PQ_thermal_handler();//use default;
        }
}
EXPORT_SYMBOL(Scaler_register_PQ_thermal);
#endif

#ifndef CONFIG_SUPPORT_SCALER_MODULE
late_initcall(Scaler_register_PQ_thermal_handler);
#endif

#endif //CONFIG_RTK_KDRV_THERMAL_SENSOR

unsigned char Scaler_color_LC_DemoStep(unsigned char mode)
{
	unsigned short LC_Backlight_Profile_Interpolation_table_temp[] = { 8, 8, 8, 8, 8, 139, 139, 139, 139, 139, 730, 730, 730, 730, 730, 139, 139, 139, 139, 139, 8, 8, 8, 8, 8, };
	unsigned short LC_Backlight_Profile_Interpolation_table_ori[] = {0,3,8,16,29,45,76,139,234,390,560,695,730,695,560,390,234,139,76,45,29,16,8,3,0,};
	int i;
	unsigned char ret = 255;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	StructColorDataType *pTable = NULL;
	//_system_setting_info *VIP_system_info_structure_table = NULL;
	SLR_VIP_TABLE *pVipTable = fwif_colo_get_AP_vip_table_gVIP_Table();

	//VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	Scaler_Get_Display_info(&display, &src_idx);
	pTable = fwif_color_get_color_data(src_idx, 0);
	ret = mode;

	if(pTable == NULL || pVipTable == NULL)
		return 0;

	if(mode==255)
		return 0;
	switch(mode)	//HDR mode
	{
		case 0:
			Scaler_SetLocalContrastEnable(0);
			fwif_color_set_dcc_table_select(src_idx, pTable->DCC_table_select);
			break;
		case 5:
			Scaler_SetLocalContrastEnable(1);
			fwif_color_set_LC_Global_Region_Num_Size(pTable->LocalContrastTable, LC_BLOCK_16x16);
			IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_8_reg, 0x00000003);
			IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_8_reg, 0x00000002);
			/*change Coeff curve for demo*/
			for (i = 0; i < LC_Table_NUM; i++) {
				memset(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table, 0, sizeof(unsigned short)*2*125);
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[0], LC_Backlight_Profile_Interpolation_table_temp, sizeof(LC_Backlight_Profile_Interpolation_table_temp));
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[1], LC_Backlight_Profile_Interpolation_table_temp, sizeof(LC_Backlight_Profile_Interpolation_table_temp));
			}
			fwif_color_set_LC_Backlight_Profile_Interpolation(src_idx, pTable->LocalContrastTable);
			fwif_color_set_dcc_table_select(src_idx,5);
		break;
		case 4:
			Scaler_SetLocalContrastEnable(1);
			fwif_color_set_LC_Global_Region_Num_Size(pTable->LocalContrastTable, LC_BLOCK_16x16);
			IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_8_reg, 0x00000003);
			IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_8_reg, 0x00000002);
			/*change Coeff curve for demo*/
			for (i = 0; i < LC_Table_NUM; i++) {
				memset(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table, 0, sizeof(unsigned short)*2*125);
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[0], LC_Backlight_Profile_Interpolation_table_temp, sizeof(LC_Backlight_Profile_Interpolation_table_temp));
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[1], LC_Backlight_Profile_Interpolation_table_temp, sizeof(LC_Backlight_Profile_Interpolation_table_temp));
			}
			fwif_color_set_LC_Backlight_Profile_Interpolation(src_idx, pTable->LocalContrastTable);
			fwif_color_set_dcc_table_select(src_idx,5);
		break;
		case 3:
			Scaler_SetLocalContrastEnable(1);
			Scaler_fwif_color_set_LocalContrast_table(src_idx,pTable->LocalContrastTable);
			fwif_color_set_LC_Global_Region_Num_Size(pTable->LocalContrastTable, LC_BLOCK_16x16);
			IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_8_reg, 0x00000003);
			IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_8_reg, 0x00000002);
			/*change Coeff curve for demo*/
			for (i = 0; i < LC_Table_NUM; i++) {
				memset(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table, 0, sizeof(unsigned short)*2*125);
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[0], LC_Backlight_Profile_Interpolation_table_temp, sizeof(LC_Backlight_Profile_Interpolation_table_temp));
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[1], LC_Backlight_Profile_Interpolation_table_temp, sizeof(LC_Backlight_Profile_Interpolation_table_temp));
			}
			fwif_color_set_LC_Backlight_Profile_Interpolation(src_idx, pTable->LocalContrastTable);
			fwif_color_set_dcc_table_select(src_idx,5);
		break;
		case 2:
			if(IoReg_Read32(STB_SC_VerID_reg) == 0x65050000){	 //Merlin3 VerA
				Scaler_SetLocalContrastEnable(1);
				fwif_color_set_LC_Global_Region_Num_Size(pTable->LocalContrastTable, LC_BLOCK_32x16);
				IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_8_reg, 0x00000004);
				IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_8_reg, 0x00000002);
			}else{
				Scaler_SetLocalContrastEnable(1);
				fwif_color_set_LC_Global_Region_Num_Size(pTable->LocalContrastTable, LC_BLOCK_32x16);
			}
			/*change Coeff curve for demo*/
			for (i = 0; i < LC_Table_NUM; i++) {
				memset(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table, 0, sizeof(unsigned short)*2*125);
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[0], LC_Backlight_Profile_Interpolation_table_ori, sizeof(LC_Backlight_Profile_Interpolation_table_ori));
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[1], LC_Backlight_Profile_Interpolation_table_ori, sizeof(LC_Backlight_Profile_Interpolation_table_ori));
			}
			fwif_color_set_LC_Backlight_Profile_Interpolation(src_idx, pTable->LocalContrastTable);
			fwif_color_set_dcc_table_select(src_idx,5);
		break;
		case 1:
			if(IoReg_Read32(STB_SC_VerID_reg) == 0x65050000){	 //Merlin3 VerA
				Scaler_SetLocalContrastEnable(1);
					Scaler_fwif_color_set_LocalContrast_table(src_idx,pTable->LocalContrastTable);
				fwif_color_set_LC_Global_Region_Num_Size(pTable->LocalContrastTable, LC_BLOCK_32x16);
				IoReg_Write32(LC_LC_Local_Shpnr_1st_gain_8_reg, 0x00000003);
				IoReg_Write32(LC_LC_Local_Shpnr_2nd_gain_8_reg, 0x00000002);
			}else{
				Scaler_SetLocalContrastEnable(1);
				fwif_color_set_LC_Global_Region_Num_Size(pTable->LocalContrastTable, LC_BLOCK_48x27);
			}
			/*change Coeff curve for demo*/
			for (i = 0; i < LC_Table_NUM; i++) {
				memset(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table, 0, sizeof(unsigned short)*2*125);
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[0], LC_Backlight_Profile_Interpolation_table_ori, sizeof(LC_Backlight_Profile_Interpolation_table_ori));
				memcpy(pVipTable->VIP_Local_Contrast_Table[i].LC_Backlight_Profile_Interpolation_table[1], LC_Backlight_Profile_Interpolation_table_ori, sizeof(LC_Backlight_Profile_Interpolation_table_ori));
			}
			fwif_color_set_LC_Backlight_Profile_Interpolation(src_idx, pTable->LocalContrastTable);
			fwif_color_set_dcc_table_select(src_idx,5);
		break;
		default:
			ret = 0;        /* LC = off*/
		break;
	}
	IoReg_SetBits(LC_LC_Global_Ctrl1_reg, LC_LC_Global_Ctrl1_lc_db_apply_mask);

	return ret;
}

unsigned char Scaler_color_LC_DemoSplit_Screen(unsigned char mode)
{
        /* remove hard code */
        /* if you need please implement it by yourself */

	return 0;
}

void Scaler_color_set_LC_DebugMode(unsigned char DebugMode)
{
	drvif_color_set_LC_DebugMode(DebugMode);
}

char Scaler_color_get_LC_DebugMode(void)
{
	return drvif_color_get_LC_DebugMode();
}

unsigned char Scaler_color_I_De_Contour_Demo(unsigned char mode)
{
        /* remove hard code */
        /* if you need please implement it by yourself */

	return 0;
}

unsigned char pq_src_idx = 0;
void Scaler_color_set_pq_src_idx(unsigned char src_idx) //for i3ddma enabled set real src_idx
{
	pq_src_idx = src_idx;
}

unsigned char Scaler_color_get_pq_src_idx(void) //for i3ddma enabled get real src_idx
{
	return pq_src_idx;
}

HDMI_CSC_DATA_RANGE_MODE_T g_HDMICSCDataRangeMode = HDMI_CSC_DATA_RANGE_MODE_AUTO;
HDMI_CSC_DATA_RANGE_MODE_T Scaler_GetHDMI_CSC_DataRange_Mode(void)
{
	return g_HDMICSCDataRangeMode;
}

void Scaler_SetHDMI_CSC_DataRange_Mode(HDMI_CSC_DATA_RANGE_MODE_T value)
{
	if (value > HDMI_CSC_DATA_RANGE_MODE_LIMITED)
		value = HDMI_CSC_DATA_RANGE_MODE_AUTO;
	g_HDMICSCDataRangeMode = value;
}

void Scaler_recheck_sharpness_table(void)
{
	unsigned char source = 255;
	unsigned char *pVipCoefArray = NULL;

	if (vpqex_project_id == 0x00060000)
		return;

	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/

	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)source);
	else {
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend3 read fail !!!\n");
		return;
	}

	if (!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		if (Scaler_GetSharpnessTable() != pVipCoefArray[VIP_QUALITY_FUNCTION_SharpTable]) {
			Scaler_SetSharpnessTable(pVipCoefArray[VIP_QUALITY_FUNCTION_SharpTable]);
			Scaler_SetSharpness(Scaler_GetSharpness());
		}
	}
}

unsigned char Scaler_Get_CinemaMode_PQ(void)
{
	return g_picmode_Cinema;
}

void Scaler_Set_CinemaMode_PQ(unsigned char mode)
{
	unsigned char source = 255;
	unsigned char *pVipCoefArray = NULL;
	unsigned char *drvSkip_Array;
	SLR_VIP_TABLE *gVIP_Table;

	g_picmode_Cinema = mode;
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/

	if (mode != 0) {
		drvif_color_Icm_Enable(0);
	} else {
		if (m_cbFunc_getQualityExtend3 != NULL)
			pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)source);
		else {
			rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend3 read fail !!!\n");
			return;
		}

		gVIP_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
		if (NULL == gVIP_Table)
			return;

		drvSkip_Array = &(gVIP_Table->DrvSetting_Skip_Flag[0]);

		if (!drvSkip_Array[DrvSetting_Skip_Flag_ICM] && (pVipCoefArray[VIP_QUALITY_FUNCTION_ICM] < VIP_ICM_TBL_X))
			drvif_color_Icm_Enable(1);
	}
}

bool Scaler_Init_OD(void)
{
	fwif_color_set_od_liner_indices(0);
	fwif_color_set_od_liner_indices(1);

	return fwif_color_od_init(0); // 0:y mode, 1:rgb mode
}
unsigned char Scaler_Get_OD_Bits(void)
{
	return drvif_color_get_od_bits();
}

unsigned char Scaler_Set_OD_Bits(unsigned char bits)
{
	return fwif_color_set_od_bits(bits);
}

unsigned char Scaler_Get_OD_Enable(void)
{
	return drvif_color_od_enable_get();
}

unsigned char Scaler_OD_Test(unsigned char en)
{
	return fwif_color_od_test(en?1:0);
}

unsigned char Scaler_Set_OD_Bit_By_XML(void)
{

	if (get_current_driver_pattern(DRIVER_OD_PATTERN) == 0) { //OD_OFF
		return fwif_color_set_od_bits(0);
	} if (get_current_driver_pattern(DRIVER_OD_PATTERN) == 1) { //OD_3BIT
		return fwif_color_set_od_bits(3);
	} else if (get_current_driver_pattern(DRIVER_OD_PATTERN) == 2) { //OD_4BIT
		return fwif_color_set_od_bits(4);
	}

	return TRUE;
}

char Scaler_Set_DeMura_En(unsigned char en)
{
	return fwif_color_set_DeMura_En(en?1:0);
}

unsigned char Scaler_Get_DeMura_En(void)
{
	return fwif_color_get_DeMura_En();
}

unsigned char g_cur_picmode_vip_src = 0;
unsigned char g_vip_src_resync_flag = false;
void Scaler_Set_PicMode_VIP_src(unsigned char src)
{
	g_cur_picmode_vip_src = src;
}

unsigned char Scaler_Get_PQ_table_bin_En(void)
{
	return PQ_bin_info;
}

void Scaler_Set_PQ_table_bin_En(unsigned char en)
{
	PQ_bin_info = en;
}

void Scaler_Check_VIP_src_resync_flag(void)
{
	unsigned int vipsource = 0;
	unsigned char g_pq_bypass_lv = Scaler_color_Get_PQ_ByPass_Lv();

	vipsource = fwif_vip_source_check(3, NOT_BY_DISPLAY);
	if (g_cur_picmode_vip_src != vipsource && g_pq_bypass_lv != 9) {
		g_cur_picmode_vip_src = vipsource;
		g_vip_src_resync_flag = true;
	} else
		g_vip_src_resync_flag = false;
}

unsigned char Scaler_Get_VIP_src_resync_flag(void)
{
	if (g_vip_src_resync_flag) {
		g_vip_src_resync_flag = false;
		return true;
	}

	return false;
}

unsigned char  Scaler_Set_VIP_HANDLER_RESET(void)
{
	VIPprintf("Scaler_Set_VIP_HANDLER_RESET\n");
	fwif_color_video_quality_handler();

	return TRUE;
}


void Scaler_color_ModuleTest_HDR_FixTable(void)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	unsigned char bHDR_flag = false;

	if(VIP_system_info_structure_table == NULL){
		return;
	}

	bHDR_flag = VIP_system_info_structure_table ->HDR_flag;

	switch(bHDR_flag) //HDR mode
	{
		case HAL_VPQ_HDR_MODE_HDR10:
			//fwif_color_ModuleTest_DM2_EOTF_OETF_3D_24x24x24_LUT_Handler(HAL_VPQ_HDR_MODE_HDR10);
			//rtd_printk(KERN_INFO, TAG_NAME, "HDR10 mode EO[1] = %d, OE[1] = %d\n", EOTF_LUT_R[1], OETF_LUT_R[1]);
		break;
		case HAL_VPQ_HDR_MODE_HLG:
			//fwif_color_ModuleTest_DM2_EOTF_OETF_3D_24x24x24_LUT_Handler(HAL_VPQ_HDR_MODE_HLG);
		break;
		default:
			rtd_printk(KERN_ERR, TAG_NAME, "Scaler_color_HDRInfo_Update, get unknow mode = %d\n", bHDR_flag);
		break;

	}

}

unsigned char Scaler_Set_VIP_OSDPicture_CrossModule(void)
{
	Scaler_SetContrast(50);
	Scaler_SetBrightness(50);
	Scaler_SetSaturation(50);
	Scaler_SetHue(50);

	return TRUE;
}

unsigned char  Scaler_Set_VIP_Disable_PQ(VPQ_ModuleTestMode ModuleTestMode, unsigned char enable)
{

	if(enable){
		switch (ModuleTestMode) {
			case VPQ_ModuleTestMode_PQByPass:
				vpq_ioctl_set_stop_run_by_idx(VPQ_IOC_PQ_CMD_INIT,1);
				vpq_extern_ioctl_set_stop_run_by_idx(VPQ_EXTERN_IOC_PQ_CMD_INIT,1);
				vpq_memc_ioctl_set_stop_run_by_idx(0, 1);
				HAL_VPQ_MEMC_SetMotionComp(128,128,VPQ_MEMC_TYPE_HIGH);
				fwif_Set_VIP_Disable_PQ(Disable_init, TRUE);
				fwif_Set_VIP_Disable_PQ(Disable_HDR, TRUE);
				fwif_Set_VIP_Disable_PQ(Disable_VPQ_ByPass, TRUE);
				fwif_Set_VIP_Disable_PQ(Disable_MEMC, TRUE);
				Scaler_Set_VIP_OSDPicture_CrossModule();
			break;
			case VPQ_ModuleTestMode_HDRPQByPass:
				vpq_ioctl_set_stop_run_by_idx(VPQ_IOC_PQ_CMD_INIT,1);
				vpq_extern_ioctl_set_stop_run_by_idx(VPQ_EXTERN_IOC_PQ_CMD_INIT,1);
				vpq_memc_ioctl_set_stop_run_by_idx(0, 1);
				HAL_VPQ_MEMC_SetMotionComp(128,128,VPQ_MEMC_TYPE_HIGH);
				fwif_Set_VIP_Disable_PQ(Disable_init, TRUE);
				fwif_Set_VIP_Disable_PQ(Disable_VPQ_ByPass, TRUE);
				fwif_Set_VIP_Disable_PQ(Disable_MEMC, TRUE);
				Scaler_color_ModuleTest_HDR_FixTable();
				Scaler_Set_VIP_OSDPicture_CrossModule();
			break;
			default:
			break;
		}
	}

	return TRUE;

}


unsigned char  Scaler_Set_PQ_ModuleTest(VPQ_ModuleTestMode ModuleTestMode)
{
	if(ModuleTestMode<VPQ_ModuleTestMode_MAX){
		S_ModuleTestMode.ModuleTestMode_en = _ENABLE;
		S_ModuleTestMode.ModuleTestMode_Flag = ModuleTestMode;
		return 0;
	}else{
		S_ModuleTestMode.ModuleTestMode_en = _DISABLE;
		S_ModuleTestMode.ModuleTestMode_Flag = VPQ_ModuleTestMode_MAX;
		return 1;
	}
}

S_VPQ_ModuleTestMode *Scaler_Get_PQ_ModuleTest(void)
{
	return &S_ModuleTestMode;
}

unsigned char Scaler_color_access_RTNR_H_Rotate_Flag(unsigned char Val, unsigned char isSetFlag)
{
	static unsigned char  RTNR_H_Flip_Status = 0;

	if (isSetFlag == 1) {
		RTNR_H_Flip_Status = Val;
	}

	return RTNR_H_Flip_Status;
}

unsigned char Scaler_color_set_RTNR_H_Rotate(unsigned char En)
{
	unsigned char DI_10bit;
	unsigned short display, DISP_RTNR, isInterlace;
	DRV_RTNR_General_ctrl ptr = {0};

	display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	DISP_RTNR = Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display,SLR_DISP_RTNR);
	isInterlace = Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_INTERLACE);
	DI_10bit = fw_scalerip_get_DI_chroma_10bits();

	if (display == SLR_MAIN_DISPLAY) {

		rtd_printk(KERN_INFO, TAG_NAME, "RTNR_H_Rotate, display=%d, DISP_RTNR=%d, isInterlace=%d, isInterlace=%d, En=%d\n",
			display, DISP_RTNR, isInterlace, isInterlace, En);

		if (En>1)
			En = 0;

		if ((En == 0) || (!DISP_RTNR) || (isInterlace) || (!DI_10bit)) {
			drvif_color_Set_RTNR_H_Rotate(0, 0);
			Scaler_color_access_RTNR_H_Rotate_Flag(0, 1);
		} else {
			drvif_color_Set_RTNR_H_Rotate(1, 1);
			drvif_color_DRV_RTNR_General_ctrl(&ptr);
			Scaler_color_access_RTNR_H_Rotate_Flag(1, 1);
		}
	}
	return Scaler_color_access_RTNR_H_Rotate_Flag(0, 0);
}

void Scaler_color_Dither_Table_Ctrl(void)
{
	//_system_setting_info *VIP_system_info_structure_table = NULL;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	di_color_recovery_option_RBUS di_color_recovery_option_reg;
	unsigned char nIdex=0,nDITHER_ENABLE=0,nDITHER_MAP_BIT=0,nDITHER_LUT=0;
	extern void* m_pVipTableCustom; /* SLR_VIP_TABLE_CUSTOM_TV001  */
	SLR_VIP_TABLE_CUSTOM_TV001* tmp_pVipTableCustom = (SLR_VIP_TABLE_CUSTOM_TV001*)m_pVipTableCustom;
	di_color_recovery_option_reg.regValue = IoReg_Read32(DI_Color_Recovery_Option_reg);

	//VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	for(nIdex = 0;nIdex<VIP_DITHER_IP_MAX;nIdex++ )
	{
		nDITHER_ENABLE = tmp_pVipTableCustom->DIHER_TABLE_CTRL[nIdex].DITHER_ENABLE;
		nDITHER_MAP_BIT = tmp_pVipTableCustom->DIHER_TABLE_CTRL[nIdex].DITHER_MAP_BIT;
		nDITHER_LUT = tmp_pVipTableCustom->DIHER_TABLE_CTRL[nIdex].DITHER_LUT;
		VIPprintf("[%d:%d:%d] Dither_Table \n",nDITHER_ENABLE, nDITHER_MAP_BIT,nDITHER_LUT);
		switch(nIdex) {
			case VIP_DITHER_IP_MAIN:
			case VIP_DITHER_IP_HDMI:
				if(nDITHER_MAP_BIT == VIP_PANEL_BIT8)
						nDITHER_LUT =VIP_DITHER_MAINTABLE_DEFAULT_12to8;
				if(nDITHER_LUT >=VIP_DITHER_MAINTABLE_MAX)
					nDITHER_LUT = VIP_DITHER_MAINTABLE_DEFAULT_12to10;
				break;
			case VIP_DITHER_IP_LGD:
				if(nDITHER_LUT >VIP_DITHER_MAINTABLE_HDR10_12to10)   //  LGD dither only 2 bits
					nDITHER_LUT = VIP_DITHER_MAINTABLE_LGD_12to10;
				break;
			case VIP_DITHER_IP_MEMC_MUX:
			case VIP_DITHER_IP_PANEL:
				if (nDITHER_MAP_BIT >= VIP_PANEL_BIT_OTHERS){ /*panel 12bit*/
					nDITHER_MAP_BIT = VIP_PANEL_BIT10;
					nDITHER_ENABLE = false;
				}
				if(nDITHER_LUT >=VIP_DITHER_PANELTABLE_MAX)
					nDITHER_LUT = VIP_DITHER_PANELTABLE_DEFAULT;
				break;
		}

		switch(nIdex) {
			case VIP_DITHER_IP_MAIN:
					if (VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] == 1) {
						fwif_color_set_Main_Dither(nDITHER_ENABLE,VIP_DITHER_MAINTABLE_HDR10_12to10);
					} else if (Scaler_InputSrcGetMainChType() == _SRC_VO || Scaler_InputSrcGetMainChType() == _SRC_HDMI) {
						if (vsc_get_adaptivestream_flag(SLR_MAIN_DISPLAY) && fw_scalerip_get_DI_chroma_10bits() == 0) /* cp  8bit*/
						{
							fwif_color_set_Main_Dither(nDITHER_ENABLE, VIP_DITHER_MAINTABLE_CP_12to8);
						}
						else if (fw_scalerip_get_DI_chroma_10bits() == 0) /*  VO + HDMI  8bit*/
						{
							fwif_color_set_Main_Dither(nDITHER_ENABLE, VIP_DITHER_MAINTABLE_DEFAULT_12to8);
						}
						else   /* VO + HDMI  /cp  10 bit*/
						{
							fwif_color_set_Main_Dither(nDITHER_ENABLE, nDITHER_LUT);
						}
					} else {
						fwif_color_set_Main_Dither(false, VIP_DITHER_MAINTABLE_DEFAULT_12to10);	/* disable */
					}
				break;
			case VIP_DITHER_IP_HDMI:
                                if ((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_AVD)||(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_ADC))
                                        fwif_color_set_HDMI_Dither(false,nDITHER_LUT);
				else if ((VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] == 1) && (get_HDMI_HDR_mode() == HDR_HDR10_HDMI) && (drvif_Hdmi_GetColorDepth() == HDMI_COLOR_DEPTH_8B))
					fwif_color_set_HDMI_Dither(TRUE,VIP_DITHER_MAINTABLE_DEFAULT_12to8);
                                else if ((drvif_Hdmi_GetColorDepth() == HDMI_COLOR_DEPTH_8B))
                                        fwif_color_set_HDMI_Dither(TRUE,nDITHER_LUT);
                                else
					fwif_color_set_HDMI_Dither(nDITHER_ENABLE,nDITHER_LUT);
				break;
			case VIP_DITHER_IP_MEMC_MUX:
				/* PC mode*/
				if ((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP) == 0) && (Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_RTNR) == 0))
					nDITHER_ENABLE = false;
				//MEMC dither, for fix MEMC white window separate issue.
				fwif_color_set_MEMC_dither(nDITHER_ENABLE,nDITHER_MAP_BIT,nDITHER_LUT);
				break;
			case VIP_DITHER_IP_LGD:
				fwif_color_set_LGD_Dither(nDITHER_ENABLE,nDITHER_LUT);
				break;
			case VIP_DITHER_IP_PANEL:

				if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
					fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
                                else if ((Get_DISPLAY_PANEL_TYPE() == P_CEDS) && (Get_DISPLAY_PANEL_BOW_RGBW() == false) && BLU_LD_Global_Ctrl0_get_ld_comp_en(IoReg_Read32(BLU_LD_Global_Ctrl0_reg)))
					fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
                                else
					fwif_color_set_Panel_Dither(nDITHER_ENABLE,VIP_PANEL_BIT10,nDITHER_LUT);
				break;
		}
	}
}

void Scaler_HDR_Sub_Init(void)
{
	fwif_color_set_HDR_sub_en(1);

	// 422 to 444
	fwif_color_HDR_sub_set422to444(0);

	// yuv 2 rgb
	fwif_color_HDR_sub_set_yuv2rgb( &(HDR_sub_YUV2RGB[HDR_SUB_YUV2RGB_BT2020_255_to_255]) );

	// gamma1
	fwif_color_set_HDR_Sub_Gamma1(1, HDR2_gamma_22, HDR2_gamma_22, HDR2_gamma_22);

	// 3x3 color mapping
	fwif_color_HDR_sub_set_colorMap( &(HDR_sub_colorMap[HDR_SUB_disable_colorMap]) );

	// gamma2
	fwif_color_set_HDR_Sub_Gamma2(1, HDR2_gamma_045, HDR2_gamma_045, HDR2_gamma_045);

}

void Scaler_HDR_Sub_Bypass(void)
{
	fwif_color_set_HDR_sub_en(0);

	// 422 to 444
	fwif_color_HDR_sub_set422to444(0);
}

unsigned char Scaler_get_dcc_table_by_source(unsigned char table_mode)
{
	unsigned char source = 255;
	unsigned char *pVipCoefArray = NULL;
	unsigned char table_num=0;

	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)source);
	else{
		printk("m_cbFunc_getQualityExtend3=null,get dcc table fail!!!\n");
		return table_num;
	}

	if (pVipCoefArray == NULL)
		return table_num;

	switch(table_mode)
	{
		case 0: // dcc
			table_num=pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Table_Select];
			break;
		case 1: // dcc skintone
			table_num=pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Skin_Tone_Table_Select];
			break;
		default:
			break;
	}
	//printk("tim_li,get dcc table->table mode=%d,table_num=%d \n",table_mode,table_num);

	return table_num;
}

unsigned char Scaler_Set_Gamma_level(unsigned char value)
{
	extern unsigned char g_GammaMode;
	g_GammaMode = value;
	return 0;
}

void Scaler_color_get_LC_His_dat(unsigned int *plc_out)
{
	 fwif_color_get_LC_Hist_dat(plc_out);
}

int Scaler_color_set_AP_PQ_extend_data(unsigned char type, unsigned long pUserSpaceData)
{
	int i, j;
	SLR_VIP_TABLE *gVIP_Table, *gShareMem_VIP_Table;

	gVIP_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	gShareMem_VIP_Table = fwif_color_GetShare_Memory_VIP_TABLE_Struct();

	if (NULL == gVIP_Table || NULL == gShareMem_VIP_Table || pUserSpaceData == 0)
		return -1;

	switch(type) {
	case PQ_EXT_TYPE_Ddomain_SHPTable:
	{
		if(copy_from_user(&gVIP_Table->Ddomain_SHPTable[0], (void __user *)pUserSpaceData, sizeof(VIP_Sharpness_Table)))
			return -1;
		for (i = 1; i < Sharp_table_num; i++) {
			memcpy(&gVIP_Table->Ddomain_SHPTable[i], &gVIP_Table->Ddomain_SHPTable[0], sizeof(VIP_Sharpness_Table));
		}
		memcpy(gShareMem_VIP_Table->Ddomain_SHPTable, gVIP_Table->Ddomain_SHPTable, sizeof(VIP_Sharpness_Table)*Sharp_table_num);
	}
	break;
	case PQ_EXT_TYPE_D_DLTI_Table:
	{
		if (copy_from_user(&gVIP_Table->vip_D_DLTI_Table[0], (void __user *)pUserSpaceData, sizeof(VIP_D_DLTI_Table)))
			return -1;
		for (i = 1; i < VIP_D_DLTI_table_num; i++) {
			memcpy(&gVIP_Table->vip_D_DLTI_Table[i], &gVIP_Table->vip_D_DLTI_Table[0], sizeof(VIP_D_DLTI_Table));
		}
		memcpy(gShareMem_VIP_Table->vip_D_DLTI_Table, gVIP_Table->vip_D_DLTI_Table, sizeof(VIP_D_DLTI_Table)*VIP_D_DLTI_table_num);
	}
	break;
	case PQ_EXT_TYPE_VipNewIDcti_Table:
	{
		if (copy_from_user(&gVIP_Table->VIP_INewDcti_Table[0], (void __user *)pUserSpaceData, sizeof(DRV_VipNewIDcti_Table)))
			return -1;
		for (i = 1; i < DCTI_TABLE_LEVEL_MAX; i++) {
			memcpy(&gVIP_Table->VIP_INewDcti_Table[i], &gVIP_Table->VIP_INewDcti_Table[0], sizeof(DRV_VipNewIDcti_Table));
		}
		memcpy(gShareMem_VIP_Table->VIP_INewDcti_Table, gVIP_Table->VIP_INewDcti_Table, sizeof(DRV_VipNewIDcti_Table)*DCTI_TABLE_LEVEL_MAX);
	}
	break;
	case PQ_EXT_TYPE_VipNewDDcti_Table:
	{
		if (copy_from_user(&gVIP_Table->VIP_DNewDcti_Table[0], (void __user *)pUserSpaceData, sizeof(DRV_VipNewDDcti_Table)))
			return -1;
		for (i = 1; i < DCTI_TABLE_LEVEL_MAX; i++) {
			memcpy(&gVIP_Table->VIP_DNewDcti_Table[i], &gVIP_Table->VIP_DNewDcti_Table[0], sizeof(DRV_VipNewDDcti_Table));
		}
		memcpy(gShareMem_VIP_Table->VIP_DNewDcti_Table, gVIP_Table->VIP_DNewDcti_Table, sizeof(DRV_VipNewDDcti_Table)*DCTI_TABLE_LEVEL_MAX);
	}
	break;
	case PQ_EXT_TYPE_CDS_ini:
	{
		if (copy_from_user(&gVIP_Table->CDS_ini[0], (void __user *)pUserSpaceData, sizeof(VIP_CDS_Table)))
			return -1;
		for (i = 1; i < Sharp_table_num; i++) {
			memcpy(&gVIP_Table->CDS_ini[i], &gVIP_Table->CDS_ini[0], sizeof(VIP_CDS_Table));
		}
		memcpy(gShareMem_VIP_Table->CDS_ini, gVIP_Table->CDS_ini, sizeof(VIP_CDS_Table)*Sharp_table_num);
	}
	break;
	case PQ_EXT_TYPE_Idomain_MBPKTable:
	{
		if (copy_from_user(&gVIP_Table->Idomain_MBPKTable[0], (void __user *)pUserSpaceData, sizeof(VIP_MBPK_Table)))
			return -1;
		for (i = 1; i < MBPK_table_num; i++) {
			memcpy(&gVIP_Table->Idomain_MBPKTable[i], &gVIP_Table->Idomain_MBPKTable[0], sizeof(VIP_MBPK_Table));
		}
		memcpy(gShareMem_VIP_Table->Idomain_MBPKTable, gVIP_Table->Idomain_MBPKTable, sizeof(VIP_MBPK_Table)*MBPK_table_num);
	}
	break;
	case PQ_EXT_TYPE_Ddomain_MBSUTable:
	{
		if (copy_from_user(&gVIP_Table->Ddomain_MBSUTable[0], (void __user *)pUserSpaceData, sizeof(VIP_MBPK_Table)))
			return -1;
		for (i = 1; i < MBSU_table_num; i++) {
			memcpy(&gVIP_Table->Ddomain_MBSUTable[i], &gVIP_Table->Ddomain_MBSUTable[0], sizeof(VIP_MBPK_Table));
		}
		memcpy(gShareMem_VIP_Table->Ddomain_MBSUTable, gVIP_Table->Ddomain_MBSUTable, sizeof(VIP_MBPK_Table)*MBSU_table_num);
	}
	break;

	case PQ_EXT_TYPE_Manual_NR_Table:
	{
		if (copy_from_user(&gVIP_Table->Manual_NR_Table[0], (void __user *)pUserSpaceData, sizeof(DRV_NR_Item)*DRV_NR_Level_MAX))
			return -1;
		for (i = 1; i < PQA_TABLE_MAX; i++) {
			memcpy(&gVIP_Table->Manual_NR_Table[i], &gVIP_Table->Manual_NR_Table[0], sizeof(DRV_NR_Item)*DRV_NR_Level_MAX);
		}
		memcpy(gShareMem_VIP_Table->Manual_NR_Table, gVIP_Table->Manual_NR_Table, sizeof(DRV_NR_Item)*DRV_NR_Level_MAX*PQA_TABLE_MAX);
	}
	break;
	case PQ_EXT_TYPE_PQA_Table_Write_Mode:
	{
		static VIP_PQA_ITEM PQA_Table_Write_Mode_tmp[PQA_ITEM_MAX];
		int x, y, z;

		if (copy_from_user(&PQA_Table_Write_Mode_tmp[0], (void __user *)pUserSpaceData, sizeof(VIP_PQA_ITEM)*PQA_ITEM_MAX))
			return -1;

		y = PQA_MODE_WRITE;
		for (x = 0; x < PQA_TABLE_MAX; x++) {
			for (z = 0; z < PQA_ITEM_MAX; z++) {
				gVIP_Table->PQA_Table[x][y][z][PQA_input_type] = PQA_Table_Write_Mode_tmp[z].ucPQA_input_type;
				gVIP_Table->PQA_Table[x][y][z][PQA_input_item] = PQA_Table_Write_Mode_tmp[z].ucPQA_input_item;
				gVIP_Table->PQA_Table[x][y][z][PQA_L00] = PQA_Table_Write_Mode_tmp[z].usPQA_L00;
				gVIP_Table->PQA_Table[x][y][z][PQA_L01] = PQA_Table_Write_Mode_tmp[z].usPQA_L01;
				gVIP_Table->PQA_Table[x][y][z][PQA_L02] = PQA_Table_Write_Mode_tmp[z].usPQA_L02;
				gVIP_Table->PQA_Table[x][y][z][PQA_L03] = PQA_Table_Write_Mode_tmp[z].usPQA_L03;
				gVIP_Table->PQA_Table[x][y][z][PQA_L04] = PQA_Table_Write_Mode_tmp[z].usPQA_L04;
				gVIP_Table->PQA_Table[x][y][z][PQA_L05] = PQA_Table_Write_Mode_tmp[z].usPQA_L05;
				gVIP_Table->PQA_Table[x][y][z][PQA_L06] = PQA_Table_Write_Mode_tmp[z].usPQA_L06;
				gVIP_Table->PQA_Table[x][y][z][PQA_L07] = PQA_Table_Write_Mode_tmp[z].usPQA_L07;
				gVIP_Table->PQA_Table[x][y][z][PQA_L08] = PQA_Table_Write_Mode_tmp[z].usPQA_L08;
				gVIP_Table->PQA_Table[x][y][z][PQA_L09] = PQA_Table_Write_Mode_tmp[z].usPQA_L09;
				gVIP_Table->PQA_Table[x][y][z][PQA_reg] = PQA_Table_Write_Mode_tmp[z].uiPQA_reg;
				gVIP_Table->PQA_Table[x][y][z][PQA_bitup] = PQA_Table_Write_Mode_tmp[z].ucPQA_bitup;
				gVIP_Table->PQA_Table[x][y][z][PQA_bitlow] = PQA_Table_Write_Mode_tmp[z].ucPQA_bitlow;
			}
		}

		memcpy(gShareMem_VIP_Table->PQA_Table, gVIP_Table->PQA_Table, sizeof(unsigned int)*PQA_LEVEL_MAX*PQA_ITEM_MAX*PQA_MODE_MAX*PQA_TABLE_MAX);
	}
	break;
	case PQ_EXT_TYPE_PQA_Input_Table:
	{
		static unsigned short PQA_Input_Table_tmp[PQA_I_ITEM_MAX][PQA_I_LEVEL_MAX];

		if (copy_from_user(PQA_Input_Table_tmp, (void __user *)pUserSpaceData, sizeof(unsigned short)*PQA_I_LEVEL_MAX*PQA_I_ITEM_MAX))
			return -1;

		for (i = 0; i < PQA_I_ITEM_MAX; i++) {
			for (j = 0; j < PQA_I_LEVEL_MAX; j++) {
				gVIP_Table->PQA_Input_Table[0][i][j] = PQA_Input_Table_tmp[i][j];
			}
		}

		for (i = 1; i < PQA_I_TABLE_MAX; i++) {
			memcpy(&gVIP_Table->PQA_Input_Table[i], &gVIP_Table->PQA_Input_Table[0], sizeof(unsigned int)*PQA_I_LEVEL_MAX*PQA_I_ITEM_MAX);
		}
		memcpy(gShareMem_VIP_Table->PQA_Input_Table, gVIP_Table->PQA_Input_Table, sizeof(unsigned int)*PQA_I_LEVEL_MAX*PQA_I_ITEM_MAX*PQA_I_TABLE_MAX);
	}
	break;
	case PQ_EXT_TYPE_MA_SNR_IESM_Table:
	{
		extern DRV_MA_SNR_IESM_Coef pq_misc_MA_SNR_IESM_TBL[MA_SNR_IESM_TBL_MAX];

		if (copy_from_user(&pq_misc_MA_SNR_IESM_TBL[0], (void __user *)pUserSpaceData, sizeof(DRV_MA_SNR_IESM_Coef)))
			return -1;

		for (i = 1; i < MA_SNR_IESM_TBL_MAX; i++) {
			pq_misc_MA_SNR_IESM_TBL[i] = pq_misc_MA_SNR_IESM_TBL[0];
		}
	}
	break;
	case PQ_EXT_TYPE_I_De_Contour_Table:
	{
		extern VIP_I_De_Contour_TBL I_De_Contour_TBL[I_De_Contour_TBL_Max];

		if (copy_from_user(&I_De_Contour_TBL[0], (void __user *)pUserSpaceData, sizeof(VIP_I_De_Contour_TBL)))
			return -1;

		for (i = 1; i < I_De_Contour_TBL_Max; i++) {
			I_De_Contour_TBL[i] = I_De_Contour_TBL[0];
		}
	}
	break;

	case PQ_EXT_TYPE_ICM_Table:
	{
		if (copy_from_user(&gVIP_Table->tICM_ini[0], (void __user *)pUserSpaceData, sizeof(unsigned short)*VIP_ICM_TBL_Z*VIP_ICM_TBL_Y))
			return -1;
		for (i = 1; i < VIP_ICM_TBL_X; i++) {
			memcpy(&gVIP_Table->tICM_ini[i], &gVIP_Table->tICM_ini[0], sizeof(unsigned short)*VIP_ICM_TBL_Z*VIP_ICM_TBL_Y);
		}
		memcpy(gShareMem_VIP_Table->tICM_ini, gVIP_Table->tICM_ini, sizeof(unsigned short)*VIP_ICM_TBL_Z*VIP_ICM_TBL_Y*VIP_ICM_TBL_X);
	}
	break;

	case PQ_EXT_TYPE_CSC_Mapping:
	{
		extern short ColorMap_Matrix_Apply[3][3];
		if (copy_from_user(&ColorMap_Matrix_Apply, (void __user *)pUserSpaceData, sizeof(ColorMap_Matrix_Apply)))
			return -1;
	}
	break;

	case PQ_EXT_TYPE_DCC_Curve_Control_Coef:
	{
		if (copy_from_user(&gVIP_Table->DCC_Control_Structure.DCC_Curve_Control_Coef[0], (void __user *)pUserSpaceData, sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX))
			return -1;
		for (i = 1; i < DCC_Curve_Adjust_TABLE_MAX; i++) {
			memcpy(&gVIP_Table->DCC_Control_Structure.DCC_Curve_Control_Coef[i], &gVIP_Table->DCC_Control_Structure.DCC_Curve_Control_Coef[0], sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX);
		}
		memcpy(gShareMem_VIP_Table->DCC_Control_Structure.DCC_Curve_Control_Coef, gVIP_Table->DCC_Control_Structure.DCC_Curve_Control_Coef, sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX*DCC_Curve_Adjust_TABLE_MAX);
	}
	break;
	case PQ_EXT_TYPE_DCC_Boundary_check_Table:
	{
		if (copy_from_user(&gVIP_Table->DCC_Control_Structure.DCC_Boundary_check_Table[0], (void __user *)pUserSpaceData, sizeof(VIP_DCC_Boundary_check_Table)))
			return -1;
		for (i = 1; i < DCC_Boundary_Check_Table_MAX; i++) {
			memcpy(&gVIP_Table->DCC_Control_Structure.DCC_Boundary_check_Table[i], &gVIP_Table->DCC_Control_Structure.DCC_Boundary_check_Table[0], sizeof(VIP_DCC_Boundary_check_Table));
		}
		memcpy(gShareMem_VIP_Table->DCC_Control_Structure.DCC_Boundary_check_Table, gVIP_Table->DCC_Control_Structure.DCC_Boundary_check_Table, sizeof(VIP_DCC_Boundary_check_Table)*DCC_Boundary_Check_Table_MAX);
	}
	break;
	case PQ_EXT_TYPE_DCC_Level_and_Blend_Coef_Table:
	{
		if (copy_from_user(&gVIP_Table->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[0], (void __user *)pUserSpaceData, sizeof(VIP_DCC_Level_and_Blend_Coef_Table)))
			return -1;
		for (i = 1; i < DCC_Level_and_Blend_Coef_Table_MAX; i++) {
			memcpy(&gVIP_Table->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[i], &gVIP_Table->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[0], sizeof(VIP_DCC_Level_and_Blend_Coef_Table));
		}
		memcpy(gShareMem_VIP_Table->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table, gVIP_Table->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table, sizeof(VIP_DCC_Level_and_Blend_Coef_Table)*DCC_Level_and_Blend_Coef_Table_MAX);
	}
	break;
	case PQ_EXT_TYPE_DCCHist_Factor_Table:
	{
		if (copy_from_user(&gVIP_Table->DCC_Control_Structure.DCCHist_Factor_Table[0], (void __user *)pUserSpaceData, sizeof(VIP_DCC_Hist_Factor_Table)))
			return -1;
		for (i = 1; i < DCCHist_Factor_Table_MAX; i++) {
			memcpy(&gVIP_Table->DCC_Control_Structure.DCCHist_Factor_Table[i], &gVIP_Table->DCC_Control_Structure.DCCHist_Factor_Table[0], sizeof(VIP_DCC_Hist_Factor_Table));
		}
		memcpy(gShareMem_VIP_Table->DCC_Control_Structure.DCCHist_Factor_Table, gVIP_Table->DCC_Control_Structure.DCCHist_Factor_Table, sizeof(VIP_DCC_Hist_Factor_Table)*DCCHist_Factor_Table_MAX);
	}
	break;
	case PQ_EXT_TYPE_DCC_AdaptCtrl_Level_Table:
	{
		if (copy_from_user(&gVIP_Table->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[0], (void __user *)pUserSpaceData, sizeof(VIP_DCC_AdaptCtrl_Level_Table)))
			return -1;
		for (i = 1; i < DCC_AdaptCtrl_Level_TABLE_MAX; i++) {
			memcpy(&gVIP_Table->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[i], &gVIP_Table->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[0], sizeof(VIP_DCC_AdaptCtrl_Level_Table));
		}
		memcpy(gShareMem_VIP_Table->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table, gVIP_Table->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table, sizeof(VIP_DCC_AdaptCtrl_Level_Table)*DCC_AdaptCtrl_Level_TABLE_MAX);
	}
	break;
	case PQ_EXT_TYPE_USER_DEFINE_CURVE_DCC_CRTL_Table:
	{
		if (copy_from_user(&gVIP_Table->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[0], (void __user *)pUserSpaceData, sizeof(VIP_USER_DEFINE_CURVE_DCC_CRTL_ITEM)))
			return -1;
		for (i = 1; i < USER_DEFINE_CURVE_DCC_TBL_NUM; i++) {
			memcpy(&gVIP_Table->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[i], &gVIP_Table->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[0], sizeof(VIP_USER_DEFINE_CURVE_DCC_CRTL_ITEM));
		}
		memcpy(gShareMem_VIP_Table->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table, gVIP_Table->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table, sizeof(VIP_USER_DEFINE_CURVE_DCC_CRTL_ITEM)*USER_DEFINE_CURVE_DCC_TBL_NUM);
	}
	break;
	case PQ_EXT_TYPE_Database_Curve_CRTL_Table:
	{
		if (copy_from_user(&gVIP_Table->DCC_Control_Structure.Database_Curve_CRTL_Table[0], (void __user *)pUserSpaceData, sizeof(VIP_DCC_Database_Curve_CRTL_Table)*Database_DCC_Curve_Case_Item_MAX))
			return -1;
		for (i = 1; i < Database_DCC_Curve_TABLE_MAX; i++) {
			memcpy(&gVIP_Table->DCC_Control_Structure.Database_Curve_CRTL_Table[i], &gVIP_Table->DCC_Control_Structure.Database_Curve_CRTL_Table[0], sizeof(VIP_DCC_Database_Curve_CRTL_Table)*Database_DCC_Curve_Case_Item_MAX);
		}
		memcpy(gShareMem_VIP_Table->DCC_Control_Structure.Database_Curve_CRTL_Table, gVIP_Table->DCC_Control_Structure.Database_Curve_CRTL_Table, sizeof(VIP_DCC_Database_Curve_CRTL_Table)*Database_DCC_Curve_Case_Item_MAX*Database_DCC_Curve_TABLE_MAX);
	}
	break;
	case PQ_EXT_TYPE_Color_Independent_Blending_Table:
	{
		if (copy_from_user(&gVIP_Table->DCC_Control_Structure.Color_Independent_Blending_Table[0], (void __user *)pUserSpaceData, sizeof(VIP_DCC_Color_Independent_Blending_Table)))
			return -1;
		for (i = 1; i < Color_Independent_Blending_Table_MAX; i++) {
			memcpy(&gVIP_Table->DCC_Control_Structure.Color_Independent_Blending_Table[i], &gVIP_Table->DCC_Control_Structure.Color_Independent_Blending_Table[0], sizeof(VIP_DCC_Color_Independent_Blending_Table));
		}
		memcpy(gShareMem_VIP_Table->DCC_Control_Structure.Color_Independent_Blending_Table, gVIP_Table->DCC_Control_Structure.Color_Independent_Blending_Table, sizeof(VIP_DCC_Color_Independent_Blending_Table)*Color_Independent_Blending_Table_MAX);
	}
	break;
	case PQ_EXT_TYPE_DCC_Chroma_Compensation_Table:
	{
		if (copy_from_user(&gVIP_Table->DCC_Control_Structure.DCC_Chroma_Compensation_Table[0], (void __user *)pUserSpaceData, sizeof(VIP_DCC_Chroma_Compensation_TABLE)))
			return -1;
		for (i = 1; i < DCC_Chroma_Compensation_TABLE_MAX; i++) {
			memcpy(&gVIP_Table->DCC_Control_Structure.DCC_Chroma_Compensation_Table[i], &gVIP_Table->DCC_Control_Structure.DCC_Chroma_Compensation_Table[0], sizeof(VIP_DCC_Chroma_Compensation_TABLE));
		}
		memcpy(gShareMem_VIP_Table->DCC_Control_Structure.DCC_Chroma_Compensation_Table, gVIP_Table->DCC_Control_Structure.DCC_Chroma_Compensation_Table, sizeof(VIP_DCC_Chroma_Compensation_TABLE)*DCC_Chroma_Compensation_TABLE_MAX);
	}
	break;
	case PQ_EXT_TYPE_DCC_Black_Adjust_Table:
	{
		extern VIP_DCC_Black_Adjust_Ctrl g_black_adjust_tv002;
		extern unsigned char g_bBlackCorrector_ByPQextAP;

		if (copy_from_user(&g_black_adjust_tv002.table, (void __user *)pUserSpaceData, sizeof(VIP_DCC_Black_Adjust_Table)))
			return -1;

		g_bBlackCorrector_ByPQextAP = 1;
	}
	break;
	case PQ_EXT_TYPE_Local_Contrast_Table:
	{
		VIP_Local_Contrast_Structure_Ext *pLC_Ext_Table = NULL;
		pLC_Ext_Table = (VIP_Local_Contrast_Structure_Ext *)dvr_malloc_specific(sizeof(VIP_Local_Contrast_Structure_Ext), GFP_DCU2_FIRST);
		if (pLC_Ext_Table != NULL) {
			int j;
			if (copy_from_user(pLC_Ext_Table, (void __user *)pUserSpaceData, sizeof(VIP_Local_Contrast_Structure_Ext))) {
				dvr_free(pLC_Ext_Table);
				return -1;
			}
		
			for (i = 0; i < LC_Table_NUM; i++) {
				gVIP_Table->VIP_Local_Contrast_Table[i].Local_Contrast_Table = pLC_Ext_Table->Local_Contrast_Table;
				for (j = 0; j < LC_Curve_ToneM_PointSlope; j++) {
					gVIP_Table->VIP_Local_Contrast_Table[i].LC_ToneMappingSlopePoint_Table[j] = pLC_Ext_Table->LC_ToneMappingSlopePoint_Table[j];
				}
				gVIP_Table->VIP_Local_Contrast_Table[i].Local_Contrast_saturation_Table = pLC_Ext_Table->Local_Contrast_saturation_Table;
				for (j = 0; j < LC_De_Cont_Level; j++) {
					gVIP_Table->VIP_Local_Contrast_Table[i].Local_Contrast_Decontour_Table[j] = pLC_Ext_Table->Local_Contrast_Decontour_Table;
				}
			}

			memcpy(gShareMem_VIP_Table->VIP_Local_Contrast_Table, gVIP_Table->VIP_Local_Contrast_Table, sizeof(VIP_Local_Contrast_Table_ST)*LC_Table_NUM);
		
			dvr_free(pLC_Ext_Table);
		} else {
			rtd_printk(KERN_ERR, TAG_NAME, "Error!! PQ extend LC Talbe mem alloc fail!!\n");
			return -1;
		}
	}
	break;
	case PQ_EXT_TYPE_WBBL_Table:
	{
		extern ST_WBBL WBBL_table;

		if (copy_from_user(&WBBL_table, (void __user *)pUserSpaceData, sizeof(ST_WBBL)))
			return -1;
	}
	break;
	case PQ_EXT_TYPE_WB_BS_Table:
	{
		extern ST_WB_BS Blue_Stretch;

		if (copy_from_user(&Blue_Stretch, (void __user *)pUserSpaceData, sizeof(ST_WB_BS)))
			return -1;
	}
	break;
	}

	return 0;
}

void Scaler_SetNNSR_Table(unsigned char TableIdx)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	rtd_printk(KERN_DEBUG, TAG_NAME, "NNSR_Table = %d\n", TableIdx);
	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "NNSR Scaler_SetNNSR_Table error %d \n", TableIdx);
		return;
	}
	pTable->NNSR = TableIdx;

	fwif_color_Set_NN_SRCtrl(TableIdx);
}
unsigned char Scaler_GetNNSR_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL)
		return 0;

	return pTable->NNSR;
}

#if 1 // lesley debug dump
#include <linux/fs.h>
static struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	#endif
	//int err = 0;
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	#endif
	filp = filp_open(path, flags, rights);
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
	#endif
	if(IS_ERR(filp)) {
		//err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

static void file_close(struct file* file) {
	filp_close(file, NULL);
}

/*static int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

#ifdef CONFIG_SUPPORT_SCALER_MODULE
	ret = kernel_read(file, data, size, &offset);
#else
	ret = vfs_read(file, data, size, &offset);
#endif

	set_fs(oldfs);
	return ret;
}*/


static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	#endif
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	ret = kernel_write(file, data, size, &offset);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
#endif
	return ret;
}

static int file_sync(struct file* file) {
	vfs_fsync(file, 0);
	return 0;
}

static int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
#endif
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	ret = kernel_read(file, data, size, &offset);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
#endif
	return ret;
}

void Scaler_color_write_data_to_bin_file(unsigned char *data, unsigned int size, unsigned char* ouputfilename)
{
	struct file *fPtr;

	fPtr = file_open(ouputfilename, O_RDWR | O_CREAT | O_APPEND, 0);
	if (fPtr == NULL) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[demura], write_data_to_bin_file, can't oopen fp\n");
		return;
	}
	file_write(fPtr, 0, data, sizeof(char) * size);
	file_sync(fPtr);
	file_close(fPtr);
}

void Scaler_color_read_data_from_bin_file(unsigned char *data, unsigned int size, unsigned char* sourcefile)
{
	struct file *fPtr;

	fPtr = file_open(sourcefile, O_RDWR | O_APPEND, 0);
	if (fPtr == NULL) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[demura], write_data_to_bin_file, can't oopen fp\n");
		return;
	}
	file_read(fPtr, 0, data, sizeof(char) * size);
	file_close(fPtr);
}


#endif

