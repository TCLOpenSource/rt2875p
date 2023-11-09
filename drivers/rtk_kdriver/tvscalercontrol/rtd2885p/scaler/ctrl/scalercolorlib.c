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
#ifndef BUILD_QUICK_SHOW
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
#include <linux/kthread.h>
#include <linux/version.h>
#else
#include <no_os/slab.h>
#include <include/string.h>
#include <timer.h>
#include <sysdefs.h>
#include <no_os/printk.h>
#include <no_os/semaphore.h>
#include <no_os/spinlock.h>
#include <no_os/spinlock_types.h>
#include <malloc.h>
#include <div64.h>
#include <rtd_log/rtd_module_log.h>
#include <no_os/pageremap.h>
#include <no_os/math64.h>
#include <rtk_kdriver/rmm/rmm.h>
//#include <tvscalercontrol/scaler/scalervideo.h>
#include <qs_pq_setting.h>
#include <rtk_kdriver/quick_show/quick_show.h>
#include <scaler/scalerCommon.h>
#endif

#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/ST2094.h>
//#include <rbus/rbusHistogramReg.h>
//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/stb_reg.h>
#include <rbus/timer_reg.h>
#include <rbus/di_reg.h>
#include "memc_isr/scalerMEMC.h"
//#include <gal/rtk_se_export.h>
#include <rbus/se_reg.h>
//#include <gal/rtk_se_lib.h>
#include <rbus/dc2h_rgb2yuv_reg.h>
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
#include <tvscalercontrol/vip/rgb_sh.h>
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

#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include "tvscalercontrol/vip/HDR10_vivid.h"

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
#include <scaler_vpqdev.h>
#include <scaler_vpqmemcdev.h>
#include <ioctrl/vpq/vpq_cmd_id.h>
#include <ioctrl/vpq/vpq_extern_cmd_id.h>
#if defined(CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT)|| defined(CONFIG_ENABLE_HDR10_HDMI_AUTO_DETECT)
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
#endif
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <dprx/dprxfun.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#ifndef BUILD_QUICK_SHOW
#include <scaler_vpqdev_extern.h>
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#else
extern int vfe_hdmi_drv_get_drm_info(vfe_hdmi_drm_t *info_frame);
#endif

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
extern VIP_DEM_TBL VIP_AP_DEM_TBL;
extern VIP_SRNN_TABLE_INFO gSrnnTableInfo;

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


#ifdef UT_flag
#undef memcpy_toio
#define memcpy_toio memcpy
#undef memcpy_fromio
#define memcpy_fromio memcpy
#undef VIPprintf
#define VIPprintf(fmt, args...)
#endif
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

int HDR_3DLUT_offset = 0;
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

extern unsigned char get_vo_camera_flow_flag(unsigned char ch);
#if 0
#ifdef OPEN_CELL_PANEL
unsigned int *ODtable_Coeff;    /*young vippanel*/
#endif
#endif
/*void * 	p_ODTABLE;*/
/*static unsigned int *TV_AutoNR_Signal_Status_Coef; */ /*2010 csfanchiang*/
unsigned int *i_edge_smooth_coef;    /*20120301 march*/

#ifdef CONFIG_I2RND_ENABLE
extern unsigned char vsc_i2rnd_sub_stage;
extern unsigned char vsc_i2rnd_dualvo_sub_stage;
#endif

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

#ifdef CONFIG_RTK_SRE_AATS_UCT_AT
//AT V6.2 Patch 2022/03/31 - START
//Added by David Kuo, 20220523
extern unsigned char PQ_UT_apply_Linear_Flag;
//Required misc/rtd_logger.c and emcu/rtk_kdv_emcu.c
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
extern unsigned int str_status;
#endif
//AT V6.2 Patch 2022/03/31 - END
#endif

extern RTK_DECONTOUR_T De_contour_level;
extern unsigned char g_bDeContourSaved;

unsigned char g_WCG_mode = 0; /* 0: WCG, 1: standard, 2: auto */

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
extern unsigned short HDR10_3DLUT_17x17x17[17*17*17][3];
extern UINT16 HDR10_3DLUT_24x24x24[24*24*24*3];
extern UINT32 EOTF_LUT_Linear[EOTF_size];
extern UINT16 OETF_LUT_Linear[OETF_size];
unsigned short OETF_LUT_ByMaxCLL_ByPanelLuminance[1025];
unsigned short OETF_LUT_ByMaxCLL_ByPanelLuminance_calman[1025];
unsigned int EOTF_LUT_ByMaxCLL_ByPanelLuminance[1025];
extern CHIP_SHARPNESS_UI_T tCurCHIP_SHARPNESS_UI_T;
extern CHIP_SR_UI_T tCurCHIP_SR_UI_T;


extern UINT32 hdr_YUV2RGB_HFC[3][VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max][12];

extern DRV_subHDR_YUV2RGB_CSMatrix HDR_sub_YUV2RGB[HDR_SUB_YUV2RGB_TABLE_Max];
extern DRV_Color_Mapping  HDR_sub_colorMap[HDR_SUB_TABLE_NUM];
extern unsigned short HDR2_gamma_22[257];
extern unsigned short HDR2_gamma_045[257];

S_VPQ_ModuleTestMode S_ModuleTestMode = {0};

extern unsigned char bPictureEnabled;

//TV030
RTK_VPQ_COLORSPACE_INDEX ColoSpace_Index;

extern unsigned char get_vsc_run_adaptive_stream(unsigned char display);
extern unsigned char get_sub_OutputVencMode(void);
extern unsigned char Eng_Skyworth_Decont_Pattern;
/*******************************************************************************
* Program
******************************************************************************/
static void Scaler_MagicPictureInit(void);
static void mScalerDisableMagicPicture(void);
static void mScalerMagicStoreContrast(void);
static void mScalerMagicPictureColorOff(void);
extern unsigned int Get_DISPLAY_REFRESH_RATE(void);

extern void scalerVIP_quality_init(unsigned long arg);
extern unsigned int quality_init_value[QUALITY_INIT_MAX];

#ifndef UT_flag
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

#endif //UT_flag
StructColorDataType *fwif_color_get_color_data(unsigned char src_idx, unsigned char timing)
{
	if (m_cbFunc_getColorData == NULL)
		return NULL;

	return (StructColorDataType *)m_cbFunc_getColorData(Scaler_InputSrcFormat(src_idx), timing, 0);
}
#ifndef UT_flag
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
#ifndef BUILD_QUICK_SHOW	
		case VSC_INPUTSRC_AVD:
			*p_scaler_src_type = get_AVD_Input_Source();
			*p_scaler_src_form = _SRC_FROM_VDC;
			break;
		case VSC_INPUTSRC_ADC:
			*p_scaler_src_type = get_ADC_Input_Source();
			*p_scaler_src_form = _SRC_FROM_ADC;
			break;
#endif			
		case VSC_INPUTSRC_HDMI:
			*p_scaler_src_type = _SRC_HDMI;
			*p_scaler_src_form = _SRC_FROM_TMDS;
			break;
		case VSC_INPUTSRC_VDEC:
        case VSC_INPUTSRC_CAMERA:
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
		if (display == SLR_MAIN_DISPLAY)
			return drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
		else
			return Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
}

unsigned char fwif_vip_HDR10_check(unsigned char display, unsigned char input_src_type)
{
	_system_setting_info *VIP_system_info_structure_table=NULL;

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

#define DP_ACT_TOLERANCE 3
#define MatchHV(hi, vi, hr, vr) (((hi) <= ((hr)+DP_ACT_TOLERANCE)) && \
	((hi) >= ((hr)-DP_ACT_TOLERANCE)) && \
	((vi) <= ((vr)+DP_ACT_TOLERANCE)) && \
	((vi) >= ((vr)-DP_ACT_TOLERANCE)))
#define MatchF(fi, fr) 	((fi) <= ((fr)+50)) && ((fi) >= ((fr)-50))
#define MatchHVF(hi, vi, fi, hr, vr, fr) (MatchHV(hi, vi, hr, vr) && MatchF(fi, fr))
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
    unsigned char slr_connect_src = 0;
	_system_setting_info *VIP_system_info_structure_table=NULL;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;

	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if((VIP_RPC_system_info_structure_table == NULL) || (VIP_system_info_structure_table == NULL)){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return 0;
	}


#ifdef CONFIG_Force_HDMI_RGB_INTO_VIP_Mode
	ForceHDMIIntoVIPMode = 1;
#endif

	if (fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC) && 
        (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_CAMERA)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_MAIN_DISPLAY), &input_src_type, &input_src_form);
		input_timming_info 	= drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
        slr_connect_src = get_scaler_connect_source(display);
	} else {
		if (callmode == BY_DISPLAY) {
			input_src_type = Scaler_InputSrcGetType((SCALER_DISP_CHANNEL)display);
			input_src_form = Scaler_InputSrcGetFrom((SCALER_DISP_CHANNEL)display);
            slr_connect_src = get_scaler_connect_source(display);
		} else {
			input_src_type = Scaler_InputSrcGetMainChType();
			input_src_form = Scaler_InputSrcGetMainChFrom();
            slr_connect_src = get_scaler_connect_source(SLR_MAIN_DISPLAY);
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

	if (slr_connect_src == _SRC_MINI_DP || slr_connect_src == _SRC_TYPEC)
        {
		#if IS_ENABLED(CONFIG_RTK_DPRX)
		DPRX_TIMING_INFO_T dp = {0};
		if (drvif_Dprx_GetRawTimingInfo(&dp) == TRUE) {
			rtd_pr_vpq_info("DP h=%d, v=%d, f=%d, i=%d\n", dp.hact, dp.vact, dp.vfreq_hz_x100, dp.is_interlace);
			if (dp.is_interlace) {
				if (dp.vact <= 240 + DP_ACT_TOLERANCE) { //480i
					input_timming_info = _MODE_480I;
				}  else if (dp.vact <= 288 + DP_ACT_TOLERANCE) { //576i
					input_timming_info = _MODE_576I;
				} else { //1080i
					input_timming_info = _MODE_1080I30;
					if (MatchF(dp.vfreq_hz_x100, 5000))
						input_timming_info = _MODE_1080I25;
				}
			} else {
				if (dp.vact >= 1200) { //4k2k
					input_timming_info = _MODE_4k2kP60;
					if (MatchF(dp.vfreq_hz_x100, 6000))
						input_timming_info = _MODE_4k2kP60;
					else if (MatchF(dp.vfreq_hz_x100, 5000))
						input_timming_info = _MODE_4k2kP50;
					else if (MatchF(dp.vfreq_hz_x100, 3000))
						input_timming_info = _MODE_4k2kP30;
					else if (MatchF(dp.vfreq_hz_x100, 2400))
						input_timming_info = _MODE_4k2kP24;
					else if (MatchF(dp.vfreq_hz_x100, 2500))
						input_timming_info = _MODE_4k2kP25;
					else if (MatchF(dp.vfreq_hz_x100, 4800))
						input_timming_info = _MODE_4k2kP48;
				} else if (MatchHV(dp.hact, dp.vact, 1920, 1080)) { //1080p
					input_timming_info = _MODE_1080P60;
					if (MatchF(dp.vfreq_hz_x100, 6000))
						input_timming_info = _MODE_1080P60;
					else if (MatchF(dp.vfreq_hz_x100, 5000))
						input_timming_info = _MODE_1080P50;
					else if (MatchF(dp.vfreq_hz_x100, 3000))
						input_timming_info = _MODE_1080P30;
					else if (MatchF(dp.vfreq_hz_x100, 2400))
						input_timming_info = _MODE_1080P24;
					else if (MatchF(dp.vfreq_hz_x100, 2500))
						input_timming_info = _MODE_1080P25;
				} else if (MatchHV(dp.hact, dp.vact, 720, 480)) { //480p
					input_timming_info = _MODE_480P;
				}  else if (MatchHV(dp.hact, dp.vact, 720, 576)) { //576p
					input_timming_info = _MODE_576P;
				} else if (MatchHV(dp.hact, dp.vact, 1280, 720)) { //720p
					input_timming_info = _MODE_720P60;
					if (MatchF(dp.vfreq_hz_x100, 5000))
						input_timming_info = _MODE_720P50;
				}else if (MatchHV(dp.hact, dp.vact, 2560, 1440)) { //1440p
					input_timming_info = _MODE_NEW;
				}
			}
		}

		if (drvif_Dprx_GetInputType() == DP_TYPE_USB_TYPE_C)
			slr_connect_src = _SRC_TYPEC;
		else
			slr_connect_src = _SRC_MINI_DP;

		if (slr_connect_src == _SRC_MINI_DP)
		{
			switch (input_timming_info)
			{
			case _MODE_480I:
					which_source = VIP_QUALITY_MINI_DP_480I;
				break;
			case _MODE_576I:
					which_source = VIP_QUALITY_MINI_DP_576I;
				break;
			case  _MODE_480P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_MINI_DP_480P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_MINI_DP_480p;
				else
					which_source = VIP_QUALITY_MINI_DP_480P;
				break;
			case _MODE_576P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_MINI_DP_576P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_MINI_DP_576P;
				else
					which_source = VIP_QUALITY_MINI_DP_576P;
				break;
			case _MODE_720P50:
			case _MODE_720P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_MINI_DP_720P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_MINI_DP_720P;
				else
					which_source = VIP_QUALITY_MINI_DP_720P;
				break;
			case _MODE_1080I25:
			case _MODE_1080I30:
					which_source = VIP_QUALITY_MINI_DP_1080I;
				break;
			case _MODE_1080P50:
			case _MODE_1080P60:
			case _MODE_1080P23:
			case _MODE_1080P24:
			case _MODE_1080P25:
			case _MODE_1080P29:
			case _MODE_1080P30:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_MINI_DP_1080P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_MINI_DP_1080P;
				else
					which_source = VIP_QUALITY_MINI_DP_1080P;
				break;
			case _MODE_4k2kI30:
				which_source = VIP_QUALITY_MINI_DP_4k2k;
				break;
			case _MODE_4k2kP30:
			case _MODE_4k2kP24:
			case _MODE_4k2kP25:
					if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
						which_source = VIP_QUALITY_HDR_MINI_DP_4k2k;
					else if(bDolby_HDR_flag)
						which_source = VIP_QUALITY_DolbyHDR_MINI_DP_4k2k;
					else
						which_source = VIP_QUALITY_MINI_DP_4k2k;
					break;
			case _MODE_4k2kP48:
			case _MODE_4k2kP50:
			case _MODE_4k2kP60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_MINI_DP_4k2k;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_MINI_DP_4k2k;
				else
					which_source = VIP_QUALITY_MINI_DP_4k2k;
				break;
            case _MODE_NEW:
                if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_MINI_DP_1440p;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_MINI_DP_576P;
				else
                    which_source = VIP_QUALITY_MINI_DP_1440p;
				break;
			default:
				  which_source = VIP_QUALITY_VGA; /* VGA Timing*/
				break;
			}
		}
		else if (slr_connect_src == _SRC_TYPEC)
		{
			switch (input_timming_info)
			{
			case _MODE_480I:
					which_source = VIP_QUALITY_TYPEC_480I;
				break;
			case _MODE_576I:
					which_source = VIP_QUALITY_TYPEC_576I;
				break;
			case  _MODE_480P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_TYPEC_480P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_TYPEC_480p;
				else
					which_source = VIP_QUALITY_TYPEC_480P;
				break;
			case _MODE_576P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_TYPEC_576P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_TYPEC_576P;
				else
					which_source = VIP_QUALITY_TYPEC_576P;
				break;
			case _MODE_720P50:
			case _MODE_720P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_TYPEC_720P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_TYPEC_720P;
				else
					which_source = VIP_QUALITY_TYPEC_720P;
				break;
			case _MODE_1080I25:
			case _MODE_1080I30:
					which_source = VIP_QUALITY_TYPEC_1080I;
				break;
			case _MODE_1080P50:
			case _MODE_1080P60:
			case _MODE_1080P23:
			case _MODE_1080P24:
			case _MODE_1080P25:
			case _MODE_1080P29:
			case _MODE_1080P30:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_TYPEC_1080P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_TYPEC_1080P;
				else
					which_source = VIP_QUALITY_TYPEC_1080P;
				break;
			case _MODE_4k2kI30:
				which_source = VIP_QUALITY_TYPEC_4k2k;
				break;
			case _MODE_4k2kP30:
			case _MODE_4k2kP24:
			case _MODE_4k2kP25:
					if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
						which_source = VIP_QUALITY_HDR_TYPEC_4k2k;
					else if(bDolby_HDR_flag)
						which_source = VIP_QUALITY_DolbyHDR_TYPEC_4k2k;
					else
						which_source = VIP_QUALITY_TYPEC_4k2k;
					break;
			case _MODE_4k2kP48:
			case _MODE_4k2kP50:
			case _MODE_4k2kP60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_TYPEC_4k2k;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_TYPEC_4k2k;
				else
					which_source = VIP_QUALITY_TYPEC_4k2k;
				break;
            case _MODE_NEW:
                if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_TYPEC_1440p;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_TYPEC_576P;
				else
                    which_source = VIP_QUALITY_TYPEC_1440p;
				break;
            default:
				  which_source = VIP_QUALITY_VGA; /* VGA Timing*/
				break;
			}
		}
		#else
		which_source = VIP_QUALITY_VGA;
		#endif
	}else if( input_src_form == _SRC_FROM_VDC )
	{
#ifndef BUILD_QUICK_SHOW	
		if( vdc_power_status )
			vdc_ColorStandard = drvif_module_vdc_ReadMode(VDC_SET);
#endif
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
#ifndef BUILD_QUICK_SHOW
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
#endif
	}
	else if (input_src_type == _SRC_VGA) {
		which_source = VIP_QUALITY_VGA;
	} else if ((input_src_type == _SRC_VO) && (input_VO_Form != VO_FROM_JPEG)) {	/* 0) DTV or  2)GAME*/
		/*if (NowSourceOption == SOURCE_PLAYBACK)			*/ /* USB source  */ /*fix me Mac3 no this function*/
		if (input_TVD_Form == TVD_INFMT_VODMA1)	{		/* USB source*/

			/*USER: Vincent_Lee  DATE: 2012/06/08  TODO: is JPEG?*/
#ifndef BUILD_QUICK_SHOW		
			if (Scaler_VOFromJPEG(Scaler_Get_CurVoInfo_plane()) == 1) {
				which_source = VIP_QUALITY_DTV_JPEG;
			} 
			else 
#endif
			{
			switch (input_timming_info) {
			case _MODE_480I:
					which_source = VIP_QUALITY_PVR_480I;
				break;
			case _MODE_576I:
					which_source = VIP_QUALITY_PVR_576I;
				break;
			case  _MODE_480P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_480P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_480P;
				else
					which_source = VIP_QUALITY_PVR_480P;
				break;
			case _MODE_576P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_576P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_576P;
				else
					which_source = VIP_QUALITY_PVR_576P;
				break;
			case _MODE_720P50:
			case _MODE_720P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_720P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_720P;
				else
					which_source = VIP_QUALITY_PVR_720P;
				break;
			case _MODE_1080I25:
			case _MODE_1080I30:
					which_source = VIP_QUALITY_PVR_1080I;
				break;
			case _MODE_1080P50:
			case _MODE_1080P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_1080p;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_1080P;
				else
					which_source = VIP_QUALITY_PVR_1080P;
				break;
            case _MODE_4k2kI30:
            case _MODE_4k2kP30:
            case _MODE_4k2kP24:
            case _MODE_4k2kP25:
            case _MODE_4k2kP48:
            case _MODE_4k2kP50:
            case _MODE_4k2kP60:
                if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_4k2k;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_4k2k;
				else
					which_source = VIP_QUALITY_PVR_4k2k;
				break;
			default:
			   if((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN)==1440)&&(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)==2560)){
                if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_PVR_1440p;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_PVR_1440P;
				else
                    which_source = VIP_QUALITY_PVR_1440p;
               }else
                    which_source = VIP_QUALITY_PVR_480I;
				break;
			}
		}
	} else {															/* DTV source*/

			switch (input_timming_info) {
			case _MODE_480I:
					which_source = VIP_QUALITY_DTV_480I;
				break;
			case _MODE_576I:
					which_source = VIP_QUALITY_DTV_576I;
				break;
			case  _MODE_480P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_480P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_480P;
				else
					which_source = VIP_QUALITY_DTV_480P;
				break;
			case _MODE_576P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_576P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_576P;
				else
					which_source = VIP_QUALITY_DTV_576P;
				break;
			case _MODE_720P50:
			case _MODE_720P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_720P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_720P;
				else
					which_source = VIP_QUALITY_DTV_720P;
				break;
			case _MODE_1080I25:
			case _MODE_1080I30:
					which_source = VIP_QUALITY_DTV_1080I;
				break;
			case _MODE_1080P50:
			case _MODE_1080P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_DTV_1080p;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_DTV_1080P;
				else
				which_source = VIP_QUALITY_DTV_1080P;
				break;
            case _MODE_4k2kI30:
            case _MODE_4k2kP30:
            case _MODE_4k2kP24:
            case _MODE_4k2kP25:
            case _MODE_4k2kP48:
            case _MODE_4k2kP50:
            case _MODE_4k2kP60:
                        if (Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_THRIP) || Scaler_DispGetStatus(SLR_MAIN_DISPLAY,SLR_DISP_RTNR))
                        {
                                if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
                                        which_source = VIP_QUALITY_HDR_DTV_4k2k;
                                else if(bDolby_HDR_flag)
                                        which_source = VIP_QUALITY_DolbyHDR_DTV_4k2k;
                                else
                                        which_source = VIP_QUALITY_DTV_4k2k;
                        }
                        else //rtnr off  , dut to 4k60 rtnr must be off ,
                        {
                                if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
                                        which_source = VIP_QUALITY_HDR_DTV_4k2k;
                                else if(bDolby_HDR_flag)
                                        which_source = VIP_QUALITY_DolbyHDR_DTV_4k2k;
                                else
                                        which_source = VIP_QUALITY_DTV_4k2k;
                        }
				break;
			default:
                if((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN)==1440)&&(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)==2560)){
                    if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
    					which_source = VIP_QUALITY_HDR_DTV_1440p;
    				else if(bDolby_HDR_flag)
    					which_source = VIP_QUALITY_DolbyHDR_DTV_1440P;
    				else
                        which_source = VIP_QUALITY_DTV_1440P;
                }else
				    which_source = VIP_QUALITY_DTV_480I;
				break;
			}
		}
	} else if ((input_src_type == _SRC_VO) && (input_VO_Form == VO_FROM_JPEG)) {/* VO JPEG source	*/ /* Erin add '100920*/

			/*Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);  */ /* M domain᪺width*/
			/*Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN);  */ /* M domain᪺height*/

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
					which_source = VIP_QUALITY_HDMI_480I;
				break;
			case _MODE_576I:
					which_source = VIP_QUALITY_HDMI_576I;
				break;
			case  _MODE_480P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_480P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_480P;
				else
					which_source = VIP_QUALITY_HDMI_480P;
				break;
			case _MODE_576P:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_576P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_576P;
				else
					which_source = VIP_QUALITY_HDMI_576P;
				break;
			case _MODE_720P50:
			case _MODE_720P60:
				if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
					which_source = VIP_QUALITY_HDR_HDMI_720P;
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_720P;
				else
					which_source = VIP_QUALITY_HDMI_720P;
				break;
			case _MODE_1080I25:
			case _MODE_1080I30:
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
				else if(bDolby_HDR_flag)
					which_source = VIP_QUALITY_DolbyHDR_HDMI_1080P;
				else
					which_source = VIP_QUALITY_HDMI_1080P;
				break;
	/*==USER: Vincent_Lee  DATE: 2011/08/12  TODO: Add new CASE for 1080P24 mode==*/
				/*which_source = VIP_QUALITY_HDMI_1080P;*/
				break;
            case _MODE_4k2kI30:
            case _MODE_4k2kP30:
            case _MODE_4k2kP24:
            case _MODE_4k2kP25:
            case _MODE_4k2kP48:
            case _MODE_4k2kP50:
            case _MODE_4k2kP60:
    			if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
    				which_source = VIP_QUALITY_HDR_HDMI_4k2k;
    			else if(bDolby_HDR_flag)
    				which_source = VIP_QUALITY_DolbyHDR_HDMI_4k2k;
    			else
    				which_source = VIP_QUALITY_HDMI_4k2k;
			break;
			/*USER: egger  DATE: 2011/09/21  TODO: Add new CASE for frame packing mode*/
			case _MODE_NEW:
#ifndef BUILD_QUICK_SHOW				
				Frame_Packing_mode = Scaler_Get3D_IsFramePacking();
#endif
                if((Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN)==1440)&&(Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID)==2560)){
                    if((bHDR_flag == HAL_VPQ_HDR_MODE_HDR10) || (bHDR_flag == HAL_VPQ_HDR_MODE_HLG) || (bHDR_flag == HAL_VPQ_HDR_MODE_TECHNICOLOR))
    				which_source = VIP_QUALITY_HDR_HDMI_1440p;
    			else if(bDolby_HDR_flag)
    				which_source = VIP_QUALITY_DolbyHDR_HDMI_1440P;
    			else
                   which_source = VIP_QUALITY_HDMI_1440P;
				}else if (Frame_Packing_mode == 1)
					which_source = VIP_QUALITY_HDMI_720P;
				else if (Frame_Packing_mode == 2)
					which_source = VIP_QUALITY_HDMI_1080P;
				else
				    which_source = VIP_QUALITY_VGA; /* VGA Timing*/
				break;
			default:
				/*which_source = VIP_QUALITY_HDMI_720P;*/
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

#endif //UT_flag

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

#ifndef UT_flag
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

#ifdef BUILD_QUICK_SHOW
extern unsigned int drvif_memory_get_data_align(unsigned int Value, unsigned int unit);
void fwif_color_video_quality_handler_quick_show(void)
{
	extern void *panel_bin_addr;
	extern PQ_HDR_TYPE Scaler_QsGetHdrType(void);
	extern struct PQ_QsSettings_ver2 *pq_qs_setting_ver2;
	extern StructColorDataFacModeType g_curColorFacTableEx; //for Driver Base MiddleWare OSD map
	extern unsigned char g_bUseMiddleWareOSDmap; //for Driver Base MiddleWare OSD map
	extern unsigned char g_cur_source;
	extern unsigned short Gamma_Linear2sRGB_14bit[1025];
	extern unsigned short Gamma_sRGB2Linear_14bit[1025];
	unsigned long base_addr = 0, base_addr0 = 0, base_addr_end = 0;	
	unsigned int *vir_addr0, *vir_addr1, *vir_addr2, *vir_addr3, *vir_addr4, *vir_addr5, *vir_addr_end;	
	static unsigned char DEM_arg_tmp[DEM_ARG_CTRL_ITEMS_Max];
	int ret =0;
	unsigned int i;
	PictureMode_Data* Cur_PicModeData=NULL;
	StructColorDataFacModeType* Cur_ColorFacTableEx=NULL; //for Driver Base MiddleWare OSD map	
	StructColorDataFacModeType Cur_ColorFacTableEx_default = {0, 64, 128, 192, 255, 0, 64, 128, 192, 255, 0, 64, 128, 192, 255, 
		0, 64, 128, 192, 255, 0, 64, 128, 192, 255, 0, 64, 128, 192, 255,};
	LPF_Gain* Cur_advCurveData;
	VIP_DEM_ColorDataFac advCurveData;
	ColorTempDataEx* Cur_ColorTempDataEx;		
	KADP_VSC_INPUT_TYPE_T cur_source=KADP_VSC_INPUTSRC_HDMI;
	unsigned char cur_port=0;
	VIP_QS_PQ_curMode cur_mode=VIP_QS_PQ_cur_UserMode;
	PICMODE_ENUM cur_pic_mode =0; 
	PqPanelColorData* PqPanelColorData_factory=NULL;
	PQ_HDR_TYPE hdrType = Scaler_QsGetHdrType();
	LUT_3D VPQ_LUT_3D = {0};
	Gamma10P VPQ_10p_gamma;
	unsigned long ca_size;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	SLR_PICTURE_MODE_DATA *pTable_picMode = NULL;
	unsigned char *pTmp8;
	unsigned char qs2kernel_crc;
	unsigned int qs2kernel_size;
	//GameOption *pGameSettings;
	unsigned int gainOD;
	PQSrcData_Ver2 *pPQSrcData_Ver2;
	unsigned int dr = 0;
	
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	Scaler_Get_Display_info(&display, &src_idx);
	pTable = fwif_color_get_color_data(src_idx, 0);	
	pTable_picMode = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	//base_addr0 = get_query_start_address(QUERY_IDX_VIP_DMAto3DTABLE);
	ca_size = carvedout_buf_query(CARVEDOUT_SCALER_VIP, (void **)&base_addr);

	if ((VIP_RPC_system_info_structure_table == NULL) || (VIP_system_info_structure_table == NULL) || (pTable == NULL) || (base_addr == 0) || (pTable_picMode == NULL)) {
		printf2("\033[1;31m %s:%d **** point null **** \033[m\n", __FILE__, __LINE__);
		return;
	}

	if(panel_bin_addr ==NULL){
		printf2("\033[1;31m %s:%d **** panel_bin_addr bin is null or crc miss-match **** \033[m\n", __FILE__, __LINE__);
		// use default, for screen normal		
		g_bUseMiddleWareOSDmap = 1;
		memcpy((unsigned char*)&g_curColorFacTableEx, (unsigned char*)&Cur_ColorFacTableEx_default, sizeof(HAL_StructColorDataFacModeType));
		ret = Scaler_set_APDEM(DEM_ARG_Data_Range, (void *)&dr);
		Scaler_SetBrightness(50);
		Scaler_SetContrast(50);
		Scaler_SetSaturation(50);
		Scaler_SetHue(50);
		//Scaler_SetDataFormatHandler();
		Scaler_SetSharpness(50);
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
		drvif_color_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
		return;
	} else {		
		PqPanelColorData_factory = (PqPanelColorData*)panel_bin_addr;
	}

	if(pq_qs_setting_ver2 ==NULL){

		printf2("\033[1;31m %s:%d **** pq_qs_setting_ver2 bin is null or crc missmatch ,apply default value **** \033[m\n", __FILE__, __LINE__);
		// use default, for screen normal		
		g_bUseMiddleWareOSDmap = 1;
		memcpy((unsigned char*)&g_curColorFacTableEx, (unsigned char*)&Cur_ColorFacTableEx_default, sizeof(HAL_StructColorDataFacModeType));
		ret = Scaler_set_APDEM(DEM_ARG_Data_Range, (void *)&dr);
		Scaler_SetBrightness(50);
		Scaler_SetContrast(50);
		Scaler_SetSaturation(50);
		Scaler_SetHue(50);
		//Scaler_SetDataFormatHandler();
		Scaler_SetSharpness(50);
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
		drvif_color_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
		
		return;
	}

	if(is_QS_hdmi_enable()) {
		cur_source = KADP_VSC_INPUTSRC_HDMI;
		cur_port =get_QS_portnum();
		printf("HDMI,cur_port=%d\n",cur_port);
	} else if (is_QS_dp_enable()){
		cur_source = KADP_VSC_INPUTSRC_DP;
		cur_port =get_QS_portnum();
		printf("DP,cur_port=%d\n",cur_port);
	} else {
		cur_port=0;
		printf("QS unsupport input source!!!\n");
		return;
	}

	if(is_QS_pc_mode() ==1)
		cur_mode = VIP_QS_PQ_cur_PcMode;
	else if(is_QS_game_mode())
		cur_mode = VIP_QS_PQ_cur_GameMode;
	else
		cur_mode = VIP_QS_PQ_cur_UserMode;

	fwif_color_set_InvGamma(SLR_MAIN_DISPLAY, 0, &Gamma_sRGB2Linear_14bit[0], &Gamma_sRGB2Linear_14bit[0], &Gamma_sRGB2Linear_14bit[0]);
	fwif_set_gamma_system_enhance(&Gamma_Linear2sRGB_14bit[0], &Gamma_Linear2sRGB_14bit[0], &Gamma_Linear2sRGB_14bit[0]);

	drvif_IESM_frontporch_set(0); //set snr line buffer mode , defualt non-game mode
	//drvif_scalerdisplay_set_sharpness_line_mode_quick_show(0); //sharpness line buffer mode ,default non-game mode
	if(cur_source ==KADP_VSC_INPUTSRC_HDMI){
		if( cur_mode ==VIP_QS_PQ_cur_UserMode){
			if((hdrType ==HDR_DM_MODE_NONE)||(hdrType ==HDR_DM_SDR_MAX_RGB)||(hdrType ==HDR_DM_MODE_DEFAULT)){
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->hdmiData[cur_port].userSelModeData.defData;
				printf("-->HDMI,UserMode,SDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}else{
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->hdmiData[cur_port].userSelModeData.hdrData;
				printf("-->HDMI,UserMode,HDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}
			//set D_3DLut
			memcpy(&VPQ_LUT_3D, &pq_qs_setting_ver2->hdmiData[cur_port].threeDLut, sizeof(ThreeD_LUT));
			// 10p
			memcpy(&VPQ_10p_gamma, &pq_qs_setting_ver2->hdmiData[cur_port].tenPointsGamma, sizeof(TenPointsGamma));
		}
		else if( cur_mode ==VIP_QS_PQ_cur_PcMode){
			if((hdrType ==HDR_DM_MODE_NONE)||(hdrType ==HDR_DM_SDR_MAX_RGB)||(hdrType ==HDR_DM_MODE_DEFAULT)){
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->hdmiData[cur_port].pcModeData.defData;
				printf("-->HDMI,PcMode,SDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}else{
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->hdmiData[cur_port].pcModeData.hdrData;
				printf("-->HDMI,PcMode,HDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}
			
			//set D_3DLut
			memcpy(&VPQ_LUT_3D, &pq_qs_setting_ver2->hdmiData[cur_port].threeDLut, sizeof(ThreeD_LUT));
			// 10p
			memcpy(&VPQ_10p_gamma, &pq_qs_setting_ver2->hdmiData[cur_port].tenPointsGamma, sizeof(TenPointsGamma));
		}
		else if( cur_mode == VIP_QS_PQ_cur_GameMode){
			fw_scalerip_set_di_gamemode(1);
			drvif_IESM_frontporch_set(1); //disable line buffer mode for game mode
			//drvif_scalerdisplay_set_sharpness_line_mode_quick_show(1); //sharpness 1 line buffer mode for game mode

			if((hdrType ==HDR_DM_MODE_NONE)||(hdrType ==HDR_DM_SDR_MAX_RGB)||(hdrType ==HDR_DM_MODE_DEFAULT)){
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->hdmiData[cur_port].gameModeData.defData;
				printf("-->HDMI,GameMode,SDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}else{
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->hdmiData[cur_port].gameModeData.hdrData;
				printf("-->HDMI,GameMode,HDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}
			
			//set D_3DLut
			memcpy(&VPQ_LUT_3D, &pq_qs_setting_ver2->hdmiData[cur_port].threeDLut, sizeof(ThreeD_LUT));
			// 10p
			memcpy(&VPQ_10p_gamma, &pq_qs_setting_ver2->hdmiData[cur_port].tenPointsGamma, sizeof(TenPointsGamma));
		}
		// pqSrcData_Ver2
		pPQSrcData_Ver2 = &pq_qs_setting_ver2->hdmiData[cur_port];	
		// get game option structure
		//pGameSettings = (GameOption*) &pq_qs_setting_ver2->hdmiData[cur_port].gainOverDriver;

		Cur_ColorFacTableEx =(StructColorDataFacModeType*) &pq_qs_setting_ver2->hdmiData[cur_port].colorCurve.basicCurveData;
		Cur_advCurveData =(LPF_Gain*) &pq_qs_setting_ver2->hdmiData[cur_port].colorCurve.advCurveData;
		Cur_ColorTempDataEx = (ColorTempDataEx*)&pq_qs_setting_ver2->hdmiData[cur_port].colorTempData[Cur_PicModeData->ColorTemperature];

		printf("===HDMI===\n");
		printf("HDMI,User_SDR.hue=%d\n",pq_qs_setting_ver2->hdmiData[cur_port].userSelModeData.defData.Hue);
		printf("HDMI,User_HDR.hue=%d\n",pq_qs_setting_ver2->hdmiData[cur_port].userSelModeData.hdrData.Hue);
		printf("HDMI,PC_SDR.hue=%d\n",pq_qs_setting_ver2->hdmiData[cur_port].pcModeData.defData.Hue);
		printf("HDMI,PC_HDR.hue=%d\n",pq_qs_setting_ver2->hdmiData[cur_port].pcModeData.hdrData.Hue);
		printf("HDMI,Game_SDR.hue=%d\n",pq_qs_setting_ver2->hdmiData[cur_port].gameModeData.defData.Hue);
		printf("HDMI,Game_HDR.hue=%d\n",pq_qs_setting_ver2->hdmiData[cur_port].gameModeData.hdrData.Hue);
	}else if(cur_source ==KADP_VSC_INPUTSRC_DP){ 
		if(cur_mode ==VIP_QS_PQ_cur_UserMode){
			if((hdrType ==HDR_DM_MODE_NONE)||(hdrType ==HDR_DM_SDR_MAX_RGB)||(hdrType ==HDR_DM_MODE_DEFAULT)){
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->dpData[cur_port].userSelModeData.defData;
				printf("-->DP,UserMode,SDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}else{
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->dpData[cur_port].userSelModeData.hdrData;
				printf("-->DP,UserMode,HDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}
				//set D_3DLut
				memcpy(&VPQ_LUT_3D, &pq_qs_setting_ver2->dpData[cur_port].threeDLut, sizeof(ThreeD_LUT));
				// 10p
				memcpy(&VPQ_10p_gamma, &pq_qs_setting_ver2->dpData[cur_port].tenPointsGamma, sizeof(TenPointsGamma));
            
		}
		else if(cur_mode ==VIP_QS_PQ_cur_PcMode){
			if((hdrType ==HDR_DM_MODE_NONE)||(hdrType ==HDR_DM_SDR_MAX_RGB)||(hdrType ==HDR_DM_MODE_DEFAULT)){
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->dpData[cur_port].pcModeData.defData;
				printf("-->DP,PcMode,SDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}else{
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->dpData[cur_port].pcModeData.hdrData;
				printf("-->DP,PcMode,HDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
                }
		//set D_3DLut
		memcpy(&VPQ_LUT_3D, &pq_qs_setting_ver2->dpData[cur_port].threeDLut, sizeof(ThreeD_LUT));
		// 10p
		memcpy(&VPQ_10p_gamma, &pq_qs_setting_ver2->dpData[cur_port].tenPointsGamma, sizeof(TenPointsGamma));
		}
		else if(cur_mode == VIP_QS_PQ_cur_GameMode){
			fw_scalerip_set_di_gamemode(1);
			drvif_IESM_frontporch_set(1); //set snr line buffer mode for game mode
			//drvif_scalerdisplay_set_sharpness_line_mode_quick_show(1); //sharpness line buffer mode for game mode

			if((hdrType ==HDR_DM_MODE_NONE)||(hdrType ==HDR_DM_SDR_MAX_RGB)||(hdrType ==HDR_DM_MODE_DEFAULT)){
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->dpData[cur_port].gameModeData.defData;
				printf("-->DP,GameMode,SDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}else{
				Cur_PicModeData =(PictureMode_Data*) &pq_qs_setting_ver2->dpData[cur_port].gameModeData.hdrData;
				printf("-->DP,GameMode,HDR[%d].hue=%d<--\n",cur_port,Cur_PicModeData->Hue);
			}
			//set D_3DLut
			memcpy(&VPQ_LUT_3D, &pq_qs_setting_ver2->dpData[cur_port].threeDLut, sizeof(ThreeD_LUT));
			// 10p
			memcpy(&VPQ_10p_gamma, &pq_qs_setting_ver2->dpData[cur_port].tenPointsGamma, sizeof(TenPointsGamma));
		}
		// pqSrcData_Ver2
		pPQSrcData_Ver2 = &pq_qs_setting_ver2->dpData[cur_port];
		
		// get game option structure
		//pGameSettings = (GameOption*) &pq_qs_setting_ver2->dpData[cur_port].gainOverDriver;

		Cur_ColorFacTableEx =(StructColorDataFacModeType*) &pq_qs_setting_ver2->dpData[cur_port].colorCurve.basicCurveData;
		Cur_advCurveData =(LPF_Gain*) &pq_qs_setting_ver2->dpData[cur_port].colorCurve.advCurveData;
		Cur_ColorTempDataEx = (ColorTempDataEx*)&pq_qs_setting_ver2->dpData[cur_port].colorTempData[Cur_PicModeData->ColorTemperature];

		printf("===DP===\n");
		printf("DP,User_SDR.hue=%d\n",pq_qs_setting_ver2->dpData[cur_port].userSelModeData.defData.Hue);
		printf("DP,User_HDR.hue=%d\n",pq_qs_setting_ver2->dpData[cur_port].userSelModeData.hdrData.Hue);
		printf("DP,PC_SDR.hue=%d\n",pq_qs_setting_ver2->dpData[cur_port].pcModeData.defData.Hue);
		printf("DP,PC_HDR.hue=%d\n",pq_qs_setting_ver2->dpData[cur_port].pcModeData.hdrData.Hue);
		printf("DP,Game_SDR.hue=%d\n",pq_qs_setting_ver2->dpData[cur_port].gameModeData.defData.Hue);
		printf("DP,Game_HDR.hue=%d\n",pq_qs_setting_ver2->dpData[cur_port].gameModeData.hdrData.Hue);
	}

	//#3 RHAL_VPQEX_IOC_SET_StructColorDataFacMode_EXTERN
	{
		memcpy((unsigned char*)&g_curColorFacTableEx, (unsigned char*)Cur_ColorFacTableEx, sizeof(HAL_StructColorDataFacModeType));
		g_bUseMiddleWareOSDmap =1;
		printf("\n #3 basicCurveData \n");

		if ((g_curColorFacTableEx.Hue_0<g_curColorFacTableEx.Hue_25) && 
				(g_curColorFacTableEx.Hue_0<g_curColorFacTableEx.Hue_50) &&
				(g_curColorFacTableEx.Hue_0<g_curColorFacTableEx.Hue_75) &&
				(g_curColorFacTableEx.Hue_0<g_curColorFacTableEx.Hue_100)) {	// center is 128
			
			printf("VPQ_EXTERN_IOC_SET_StructColorDataFacMode_EXTERN, ap center is 128\n");
		} else {	// center is 0
			printf("VPQ_EXTERN_IOC_SET_StructColorDataFacMode_EXTERN, ap center is 0\n");
			g_curColorFacTableEx.Hue_0 = g_curColorFacTableEx.Hue_0 + 128;
			g_curColorFacTableEx.Hue_25 = g_curColorFacTableEx.Hue_25 + 128;
			g_curColorFacTableEx.Hue_50 = g_curColorFacTableEx.Hue_50 + 128;
			g_curColorFacTableEx.Hue_75 = g_curColorFacTableEx.Hue_75 + 128;
			g_curColorFacTableEx.Hue_100 = g_curColorFacTableEx.Hue_100 + 128;
		}
	}

	{	
		ret = Scaler_set_APDEM(DEM_ARG_Gamma_CURVE_TBL_PANEL_Chromaticity, (void *)(&PqPanelColorData_factory->mColorGammaData));			
		printf("\n #4 DEM_ARG_Gamma_CURVE_TBL_PANEL_Chromaticity \n");
	}
	
	//#4 RHAL_VPQEX_Set_After_Filter_LPF_gain_ColorDataFac
	// bin file define size is different with drvier
	{
		advCurveData.LPF_Gain_0 = Cur_advCurveData->LpfGain_0;
		advCurveData.LPF_Gain_25 =Cur_advCurveData->LpfGain_25;
		advCurveData.LPF_Gain_50 =Cur_advCurveData->LpfGain_50;
		advCurveData.LPF_Gain_75 =Cur_advCurveData->LpfGain_75;
		advCurveData.LPF_Gain_100 =Cur_advCurveData->LpfGain_100;
		//Scaler_APDEM_En_Access(1,1);
		ret = Scaler_set_APDEM(DEM_ARG_LPF_Gain_ColorDataFac, (void *)&advCurveData);
		printf("\n #4 advCurveData \n");
		
	}

	
	//#5 RHAL_VPQ_SetColorTempExtend :VPQ_EXTERN_IOC_SET_COLOR_TEMP_EXTERN
	{	

		Scaler_set_APDEM(DEM_ARG_Gamma_idx_set, &Cur_ColorTempDataEx->gamma_curve_index);
		Scaler_SetColorTempData((TV001_COLOR_TEMP_DATA_S*) Cur_ColorTempDataEx); 
        //fwif_color_set_InvOutputGamma_System(Cur_ColorTempDataEx->gamma_curve_index);
		
		printf("\n #5 gamma_curve_index=%d \n",Cur_ColorTempDataEx->gamma_curve_index);
		printf("\n R gain=%d,G gain=%d,B gain=%d,R offset=%d,G offset=%d,G offset=%d \n",
			Cur_ColorTempDataEx->redGain,
			Cur_ColorTempDataEx->greenGain,
			Cur_ColorTempDataEx->blueGain,
			Cur_ColorTempDataEx->redOffset,
			Cur_ColorTempDataEx->greenOffset,
			Cur_ColorTempDataEx->blueOffset
		);

	}
	//#6 RHAL_VPQ_SetBrightness :VPQ_EXTERN_IOC_SET_BRIGHTNESS
	{	
		Scaler_SetBrightness((unsigned char)Cur_PicModeData->Brightness);
		printf("\n #6 Brightness=%d \n",Cur_PicModeData->Brightness);
		
	}

	//#7 RHAL_VPQ_SetContrast :VPQ_EXTERN_IOC_SET_CONTRAST
	{	
		Scaler_SetContrast((unsigned char)Cur_PicModeData->Contrast);
		printf("\n #7 Contrast=%d \n",Cur_PicModeData->Contrast);
		
	}

	//#8 RHAL_VPQ_SetSaturation :VPQ_EXTERN_IOC_SET_SATURATION
	{	
		Scaler_SetSaturation((unsigned char)Cur_PicModeData->Saturation);
		printf("\n #8 Saturation=%d \n",Cur_PicModeData->Saturation);
		
	}

	//#9 RHAL_VPQ_SetHue :VPQ_EXTERN_IOC_SET_HUE
	{	
		Scaler_SetHue((unsigned char)Cur_PicModeData->Hue);
		printf("\n #9 Hue=%d \n",Cur_PicModeData->Hue);
		
	}

    //ret = Scaler_set_APDEM(DEM_ARG_Data_Range, (void *)&pq_qs_setting_ver2->dpData[cur_port].range);
    ret = Scaler_set_APDEM(DEM_ARG_Data_Range, (void *)&pPQSrcData_Ver2->range);
	//Scaler_SetDataFormatHandler();

	//#10 RHAL_VPQEX_Set_After_Filter_Enable :VPQ_EXTERN_IOC_SET_After_Filter_En
	{	
		ret = Scaler_set_APDEM(DEM_ARG_After_Filter_En, (void *)&Cur_PicModeData->After_Filter_En);
		printf("\n #10 After_Filter_En =%d \n",Cur_PicModeData->After_Filter_En);
	}
	{	
		ret = Scaler_set_APDEM(DEM_ARG_After_Filter_Val, (void *)&Cur_PicModeData->Sharpness);
		printf("\n #10 DEM_ARG_After_Filter_Val =%d \n", Cur_PicModeData->Sharpness);
	}

	//#11 RHAL_VPQ_SetSharpnessExtend :VPQ_EXTERN_IOC_SET_SHARPNESS_EXTERN
	{	
		Scaler_SetSharpness(Cur_PicModeData->Sharpness);
		printf("\n #11 Scaler_SetSharpness =%d \n",Cur_PicModeData->Sharpness);
	}



	//#12 RHAL_VPQEX_Set_Gamma_byOffset :VPQ_EXTERN_IOC_SET_Gamma_byOffset
	{	
	/*fixed me hdr flow not work now */

	if((hdrType ==HDR_DM_MODE_NONE)||(hdrType ==HDR_DM_SDR_MAX_RGB)||(hdrType ==HDR_DM_MODE_DEFAULT))
	{ //non hdr type

		ret = Scaler_set_APDEM(DEM_ARG_Gamma_exp_byOffset, (void *)&Cur_PicModeData->Gamma);
		printf("\n #12 Gamma_byOffset =%d \n",Cur_PicModeData->Gamma);
	}

		ret = Scaler_set_APDEM(DEM_ARG_10p_Gamma_Offset, (void *)&VPQ_10p_gamma);
		printf("\n #12.5 DEM_ARG_10p_Gamma_Offset =%d \n");

	}


	//#13 RHAL_VPQ_SetDeBlocking :VPQ_EXTERN_IOC_SET_DE_BLOCKING
	{	
		Scaler_SetMPEGNR((unsigned char)Cur_PicModeData->MpegNR,0);
		printf("\n #13 MpegNR =%d \n",Cur_PicModeData->MpegNR);
	}

	//#14 RHAL_VPQ_SetNR :VPQ_EXTERN_IOC_SET_NR
	{	
		Scaler_SetDNR((unsigned char)Cur_PicModeData->NoiseReduction);
		printf("\n #14 NoiseReduction =%d \n",Cur_PicModeData->NoiseReduction);
	}

	//#15 RHAL_VPQEX_Set_White_Extension_level :VPQ_EXTERN_IOC_SET_White_Extension_Level
	{	
		ret = Scaler_set_APDEM(DEM_ARG_White_Extension_level, (void *)&Cur_PicModeData->WhiteExtension);
		printf("\n #15 WhiteExtension =%d \n",Cur_PicModeData->WhiteExtension);
	}

	//#16 RHAL_VPQEX_Set_Black_Extension_level :VPQ_EXTERN_IOC_SET_Black_Extension_Level
	{	
		ret = Scaler_set_APDEM(DEM_ARG_Black_Extension_level, (void *)&Cur_PicModeData->BlackExtension);
		printf("\n #16 BlackExtension =%d \n",Cur_PicModeData->BlackExtension);
	}

	//#17 RHAL_VPQEX_Set_DCC_byIndex :VPQ_EXTERN_IOC_SET_DCC_byIndex
	{
		// set linear table first;
		fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
		ret = Scaler_set_APDEM(DEM_ARG_DCC_Idx, (void *)&Cur_PicModeData->DCC_Index);
		printf("\n #17 DCC_Index =%d \n",Cur_PicModeData->DCC_Index);
	}

	//#18 RHAL_VPQEX_SetDCCMode :VPQ_EXTERN_IOC_SET_DCC_MODE
	{	
		Scaler_SetDCC_Mode(Cur_PicModeData->DCC_Level);
		printf("\n #18 DCC_Level =%d \n",Cur_PicModeData->DCC_Level);
	}

	//#19 RHAL_VPQEX_Set_LC_byIndex :VPQ_EXTERN_IOC_SET_Local_Contrast_byIndex
	{	
		ret = Scaler_set_APDEM(DEM_ARG_LC_Idx, (void *)&Cur_PicModeData->LC_Index);

		printf("\n #19 LC_Index =%d \n",Cur_PicModeData->LC_Index);
	}

	//#20 RHAL_VPQEX_Set_LC_byLevel :VPQ_EXTERN_IOC_SET_Local_Contrast_byLevel
	{	
		ret = Scaler_set_APDEM(DEM_ARG_LC_level, (void *)&Cur_PicModeData->LC_Level);

		printf("\n #20 LC_Level =%d \n",Cur_PicModeData->LC_Level);
	}

	//#21 RHAL_VPQEX_Set_LD_byLevel :VPQ_EXTERN_IOC_SET_Local_Dimming_byLevel
	{	
		//ret = Scaler_set_APDEM(DEM_ARG_LC_level, (void *)&Cur_PicModeData->LD);

		//printf("\n #21 LD =%d \n",Cur_PicModeData->LD);
	}

	//#22 RHAL_VPQEX_Set_ICM_byIndex :VPQ_EXTERN_IOC_SET_ICM_byIndex
	{
		//fixed me ICM driver on quick show have some problem (make green screen)
		ret = Scaler_set_APDEM(DEM_ARG_ICM_Idx, (void *)&Cur_PicModeData->ICM_Index);
		printf("\n #22 ICM_Index =%d \n",Cur_PicModeData->ICM_Index);
	}

	//#23 RHAL_VPQEX_Set_Blue_Stretch_Level :VPQ_EXTERN_IOC_SET_Blue_Stretch
	{	
		ret = Scaler_set_APDEM(DEM_ARG_Blue_Stretch_level, (void *)&Cur_PicModeData->BlueStretch);
		printf("\n #23 BlueStretch =%d \n",Cur_PicModeData->BlueStretch);
	}
#if 0	
	//#24 setBackLight 
	{	
		extern int rtk_pwm_set_param(RTK_PWM_PARAM_EX_T* param);
		RTK_PWM_PARAM_EX_T param = {
			.id =  PWM_PARAM_DUTY,
			.val = Cur_PicModeData->Backlight,
			.pcbname = "PIN_BL_ADJ",
		};

		ret = rtk_pwm_set_param(&param);
		printf("\n #24 Backlight =%d \n",Cur_PicModeData->Backlight);
	}
#endif	
	//#25 RHAL_VPQEX_Set_Color_Space :VPQ_EXTERN_IOC_SET_Color_Space
	{

		ret = Scaler_set_APDEM(DEM_ARG_Color_Space, (void *)&Cur_PicModeData->ColorSpace);
		printf("\n ColorSpace =%d \n",Cur_PicModeData->ColorSpace);
	}
	//#26 RHAL_VPQEX_Set_HDR3DLUT_Offset :VPQ_EXTERN_IOC_SET_HDR3DLUT_OFFSET
	{
		Scale_SetHDR3DLUT_Offset(Cur_PicModeData->HLG_Lut_Offset);
		printf("\n #26 HDR3DLUT =%d \n",Cur_PicModeData->HLG_Lut_Offset);
	}
	//#27 RHAL_VPQEX_SET_SLR_HDR_TABLE_OETF_Setting_Ctrl :VPQ_EXTERN_IOC_SET_SLR_HDR_TABLE_OETF_Setting_Ctrl
	{
		Scaler_color_copy_HDR_table_oetf_setting_ctrl_from_AP(Cur_PicModeData->HDR10_Dynamic_En);
		printf("\n #27 HDR_setting_ctrl =%d \n",Cur_PicModeData->HDR10_Dynamic_En);
	}
	//#28 RHAL_VPQEX_SET_SLR_HDR_TABLE_OETF_Curve_Ctrl :VPQ_EXTERN_IOC_SET_SLR_HDR_TABLE_OETF_Curve_Ctrl
	{
		Scaler_color_copy_HDR_table_oetf_curve_ctrl_from_AP(Cur_PicModeData->HDR10_Curve);
		printf("\n #28 HDR_curve_ctrl =%d \n",Cur_PicModeData->HDR10_Curve);
	}
    //#29 VPQ_EXTERN_IOC_SET_D_3DLUT_Offset
    {
    	if (VPQ_LUT_3D.EN == 0)
			memset(&VPQ_LUT_3D, 0, sizeof(LUT_3D));    
		ret = Scaler_set_APDEM(DEM_ARG_D_3DLUT_Offset, (void *)&VPQ_LUT_3D.red.R);
		printf("\n #29 VPQ_EXTERN_IOC_SET_D_3DLUT_Offset \n");
	}
	// #30 VPQ_EXTERN_IOC_SET_OverDriver_Para
	{
		gainOD = pPQSrcData_Ver2->gainOverDriver/*+64*/;
		ret = Scaler_set_APDEM(DEM_ARG_OD_Gain, (void *)&gainOD);
		printf("\n #30 VPQ_EXTERN_IOC_SET_OverDriver_Para =%d,\n", pPQSrcData_Ver2->gainOverDriver);
	}	

	// #31 VPQ_EXTERN_IOC_SET_Dynamic_Black_Equalize_Info
	{
		ret = Scaler_set_APDEM(DEM_ARG_Dynamic_Black_Equalize_Gain, (void *)&pPQSrcData_Ver2->dynaBlackEq);
		printf("\n #31 VPQ_EXTERN_IOC_SET_Dynamic_Black_Equalize_Info =%d,\n", pPQSrcData_Ver2->dynaBlackEq);
	}	

	// #32 VPQ_EXTERN_IOC_SET_Shadow_Detail_Info
	{
		ret = Scaler_set_APDEM(DEM_ARG_Shadow_detail_Gain, (void *)&pPQSrcData_Ver2->shadowDetail);
		printf("\n #32 VPQ_EXTERN_IOC_SET_Shadow_Detail_Info =%d,\n", pPQSrcData_Ver2->shadowDetail);
	}	
	
	// end, save data to carveout for kernel flow
	// pa = va in QS
	// cal size
	qs2kernel_size = sizeof(_RPC_system_setting_info) + sizeof(_system_setting_info) + sizeof(StructColorDataType) +
		(sizeof(char)*DEM_ARG_CTRL_ITEMS_Max) + sizeof(VIP_DEM_TBL) + sizeof(SLR_PICTURE_MODE_DATA);	// 

	if (qs2kernel_size > ca_size) {
		printf2("[%s]carve size is not enough return, qs2kernel_size=%d, ca_size=%d,\n", 
			__func__, qs2kernel_size, ca_size);	
		return;
	}
		
	base_addr0 = drvif_memory_get_data_align(base_addr, (1 << 12));
	vir_addr0 = (unsigned int*)base_addr0;
	vir_addr1 = (unsigned int*)(base_addr0 + sizeof(_RPC_system_setting_info));
	vir_addr2 = (unsigned int*)(base_addr0 + sizeof(_RPC_system_setting_info) + sizeof(_system_setting_info));
	vir_addr3 = (unsigned int*)(base_addr0 + sizeof(_RPC_system_setting_info) + sizeof(_system_setting_info) + sizeof(StructColorDataType));
	vir_addr4 = (unsigned int*)(base_addr0 + sizeof(_RPC_system_setting_info) + sizeof(_system_setting_info) + sizeof(StructColorDataType) +
		(sizeof(char)*DEM_ARG_CTRL_ITEMS_Max));
	vir_addr5 = (unsigned int*)(base_addr0 + sizeof(_RPC_system_setting_info) + sizeof(_system_setting_info) + sizeof(StructColorDataType) +
		(sizeof(char)*DEM_ARG_CTRL_ITEMS_Max) + sizeof(VIP_DEM_TBL));

	// set end addr
	vir_addr_end =  (unsigned int*)(base_addr0 + qs2kernel_size);
	base_addr_end = (unsigned long)vir_addr_end;
	
	for (i=0;i<DEM_ARG_CTRL_ITEMS_Max;i++)
		DEM_arg_tmp[i] = Scaler_APDEM_Arg_Access(i ,0, 0);
	
	memcpy(vir_addr0, VIP_RPC_system_info_structure_table, sizeof(_RPC_system_setting_info));
	memcpy(vir_addr1, VIP_system_info_structure_table, sizeof(_system_setting_info));
	memcpy(vir_addr2, pTable, sizeof(StructColorDataType));	
	memcpy(vir_addr3, &DEM_arg_tmp[0], (sizeof(char)*DEM_ARG_CTRL_ITEMS_Max));
	memcpy(vir_addr4, &VIP_AP_DEM_TBL, sizeof(VIP_DEM_TBL));	
	memcpy(vir_addr5, pTable_picMode, sizeof(SLR_PICTURE_MODE_DATA));	
		
	flush_dcache_range(base_addr0, base_addr_end);

	// cal crc
	qs2kernel_crc = 0;
	pTmp8 = (unsigned char*)VIP_RPC_system_info_structure_table;
	for (i=0;i<sizeof(_RPC_system_setting_info);i++)
		qs2kernel_crc = (qs2kernel_crc + pTmp8[i])&0xFF;
	
	pTmp8 = (unsigned char*)VIP_system_info_structure_table;
	for (i=0;i<sizeof(_system_setting_info);i++)
		qs2kernel_crc = (qs2kernel_crc + pTmp8[i])&0xFF;
	
	pTmp8 = (unsigned char*)pTable;
	for (i=0;i<sizeof(StructColorDataType);i++)
		qs2kernel_crc = (qs2kernel_crc + pTmp8[i])&0xFF;
	
	for (i=0;i<DEM_ARG_CTRL_ITEMS_Max;i++)
		qs2kernel_crc = (qs2kernel_crc + Scaler_APDEM_Arg_Access(i, 0, 0))&0xFF;
	
	pTmp8 = (unsigned char*)&VIP_AP_DEM_TBL;
	for (i=0;i<sizeof(VIP_DEM_TBL);i++)
		qs2kernel_crc = (qs2kernel_crc + pTmp8[i])&0xFF;

	pTmp8 = (unsigned char*)pTable_picMode;
	for (i=0;i<sizeof(SLR_PICTURE_MODE_DATA);i++)
		qs2kernel_crc = (qs2kernel_crc + pTmp8[i])&0xFF;
	
	printf2("[%s]base_addr=%x,pa sta=%x,pa end=%x,va=%x,%x,%x,%x,%x,%x,%x,qs2kernel_crc=%d,\n", 
		__func__, base_addr, base_addr0, base_addr_end, 
		(unsigned long)vir_addr0, (unsigned long)vir_addr1, (unsigned long)vir_addr2, (unsigned long)vir_addr3,
		(unsigned long)vir_addr4, (unsigned long)vir_addr5, (unsigned long)vir_addr_end, qs2kernel_crc);	
}

#endif

void fwif_color_video_quality_handler(void)/*TV Process*/
{	
	unsigned char ii;/*, jj;*/
	/*unsigned char which_value, pre_value;*/
	unsigned char source = 255;
	#ifdef AUTO_DE_CONTOUR_I2R
	static unsigned char pre_source = 255;
	#endif
	unsigned char *pVipCoefArray = NULL;
	unsigned char *pVipCoefExt4Array = NULL;
	SLR_VIP_TABLE *gVIP_Table;
	unsigned char *drvSkip_Array;
	#if 0
	unsigned char black_levle = 0;
	unsigned char  input_info = 0;
    #endif
	unsigned char mCon, mBri, mHue, mSat;
    extern unsigned int AV_VD_Table[8][4];
    extern unsigned int ATV_VD_Table[8][4];
    extern SLR_VIP_TABLE* m_pVipTable;
	unsigned char HSR_mode = 0;
	/*int ret;*/
	/*unsigned char spatial_y = 0, spatial_c = 0, temporal = 0, rtnr_level_y = 0, rtnr_level_c = 0, s_dnr_level = 0;*/

	/*Elsie 20130509: for speeding up*/
	_system_setting_info *VIP_system_info_structure_table = NULL;
	ppoverlay_main_display_control_rsv_RBUS ppoverlay_main_display_control_rsv;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	rtd_pr_vpq_info("+VIP color handler+\n");
	/*force background check*/
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	ppoverlay_main_display_control_rsv.regValue= IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	if (ppoverlay_main_display_control_rsv.m_force_bg == 0) /*normal mode*/
		VIPprintf("[Warnning] [%s-%s-%d] Start, Not Black Screen", __FILE__, __func__, __LINE__);

	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

#ifdef BUILD_QUICK_SHOW /*for quick_show mode */
	fwif_color_pq_init_quick_show();
	if(VIP_system_info_structure_table==NULL ||VIP_RPC_system_info_structure_table==NULL){
		printf2("\033[1;31m %s %s:%d, load pq table fail \033[m\n", __FUNCTION__, __FILE__, __LINE__);
		return;
	}
#else
	if((VIP_RPC_system_info_structure_table == NULL) || (VIP_system_info_structure_table == NULL)){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}
#endif
	VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_HDR10_display_check] = 1;

	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	m_nowSource = source;

	Scaler_VIP_IP_init(source);/*some vip ip init*/
//#ifndef CONFIG_MEMC_BYPASS
	//Scaler_VIP_MEMC_init(source);/*vip MEMC ip init*/
//#endif

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

#ifdef CONFIG_DYNAMIC_PANEL_SELECT
#ifndef BUILD_QUICK_SHOW
	if(PANEL_DLG_4k1k120 == vbe_get_cur_dlg_mode())
		HSR_mode = 1;
#endif
#endif

	if (m_cbFunc_getQualityExtend4 != NULL)
		pVipCoefExt4Array = (unsigned char *)m_cbFunc_getQualityExtend4((VIP_SOURCE_TIMING)source);

	drvSkip_Array = &(gVIP_Table->DrvSetting_Skip_Flag[0]);
/*o------ VIP table : VIP_QUALITY_Coef ------o*/
	/* Load pVipCoefArray*/
	if (m_cbFunc_getQualityCoef != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityCoef((VIP_SOURCE_TIMING)source);
	else
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityCoef read fail !!!\n");

	rtd_pr_vpq_emerg("==========  source = %d ===================\n", source);

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

		if (Scaler_APDEM_Arg_Access(DEM_ARG_LC_Idx, 0, 0) == 0xFF) {
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
		} else {
			Scaler_SetLocalContrastTable(Scaler_APDEM_Arg_Access(DEM_ARG_LC_Idx, 0, 0));
			fwif_color_set_LC_Global_Region_Num_Size(pVipCoefArray[VIP_QUALITY_FUNCTION_LC_Table], LC_BLOCK_48x27);
			if( drvif_color_get_LC_size_support()!=0 )
				Scaler_SetLocalContrastEnable(1);
			else
				Scaler_SetLocalContrastEnable(0);
		}

#ifdef VIP_SUPPORT_SLC
		Scaler_Set_Dynamic_SLC_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_Dynamic_SLC_Table]);
		Scaler_Set_Dynamic_SLC_Enable(pVipCoefArray[VIP_QUALITY_FUNCTION_Dynamic_SLC_Enable]);
#endif

		//Scaler_Set_ColorMapping_3x3_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_Color_Mapping]); //no more use, control by Scaler_Set_ColorMapping_By_ColorFormat()

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

		if (Scaler_APDEM_Arg_Access(DEM_ARG_D_3DLUT_Offset, 0, 0) == 0xFF) {
			if ((vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) && (fwif_VIP_get_Project_ID() != VIP_Project_ID_TV030)) {
				//Scaler_color_colorwrite_Output_InvOutput_gamma(pVipCoefArray[VIP_QUALITY_FUNCTION_Output_InvOutput_GAMMA]); //no more use
				Scaler_Set3dLUT(pVipCoefArray[VIP_QUALITY_FUNCTION_3dLUT_Table]);
			}
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

		if (HSR_mode && pVipCoefExt4Array)
			Scaler_setiESM(pVipCoefExt4Array[VIP_QUALITY_FUNCTION_HSR_iESM_table]);
		else
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
			if (HSR_mode && pVipCoefExt4Array)
				scaler_set_PQA_table(pVipCoefExt4Array[VIP_QUALITY_FUNCTION_HSR_PQA_Table]);
			else
				scaler_set_PQA_table(pVipCoefArray[VIP_QUALITY_FUNCTION_PQA_Table]);
			scaler_set_PQA_Input_table(pVipCoefArray[VIP_QUALITY_FUNCTION_PQA_Input_Table]);
		}
		else
		{
			scaler_set_PQA_table(9);
			scaler_set_PQA_Input_table(9);
		}

#ifndef BUILD_QUICK_SHOW
        rtd_pr_vpq_info("vdc_power_status=%d\n",vdc_power_status);
		if (vdc_power_status)
		{
                if (fwif_color_get_VD_Value(&mCon, &mBri, &mHue, &mSat) == 0){
					    rtd_pr_vpq_emerg("mCon=%d mBri=%d mHue=%d mSat=%d \n",mCon,mBri,mHue,mSat);
					    fwif_color_set_VDBlackLevel(mCon, mBri, mHue, mSat);
				}
		}
#endif
		if(vpq_project_id == 0x00060000 && vpqex_project_id == 0x00100000){
			if (Scaler_APDEM_Arg_Access(DEM_ARG_DCC_Idx, 0, 0) == 0xFF) {			
				Scaler_Set_DCC_Color_Independent_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Skin_Tone_Table_Select]);

				Scaler_Set_DCC_chroma_compensation_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Chroma_comp_t_sel]);
			} else {
				rtd_pr_vpq_info("DCC i/C set by DEM, skip flow in quality handler\n");
			}
		}
		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) { /*pre_id:16 main_id:8 sub1_id:8 sub2_id:8, example->lg=tv006*/

			if (Scaler_APDEM_Arg_Access(DEM_ARG_ICM_Idx, 0, 0) == 0xFF) {
				Scaler_set_ICM_table(pVipCoefArray[VIP_QUALITY_FUNCTION_ICM],  drvSkip_Array[DrvSetting_Skip_Flag_ICM]);
			} else  {
				rtd_pr_vpq_info("ICM set by DEM, skip flow in quality handler\n");
			}		
			
			if (Scaler_APDEM_Arg_Access(DEM_ARG_Color_Space, 0, 0) == 0xFF) {			
				Scaler_SetGamma_for_Handler(pVipCoefArray[VIP_QUALITY_FUNCTION_Gamma],
												pVipCoefArray[VIP_QUALITY_FUNCTION_S_Gamma_Index],
												pVipCoefArray[VIP_QUALITY_FUNCTION_S_Gamma_Low],
												pVipCoefArray[VIP_QUALITY_FUNCTION_S_Gamma_High], drvSkip_Array[DrvSetting_Skip_Flag_Gamma]);
			}
			
			if (Scaler_APDEM_Arg_Access(DEM_ARG_DCC_Idx, 0, 0) == 0xFF) {
				Scaler_SetDCC_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Table_Select]);

				Scaler_Set_DCC_Color_Independent_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Skin_Tone_Table_Select]);

				Scaler_Set_DCC_chroma_compensation_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_DCC_Chroma_comp_t_sel]);
			} else {
				rtd_pr_vpq_info("DCC set by DEM, skip flow in quality handler\n");
			}
		}

		if (Scaler_APDEM_Arg_Access(DEM_ARG_ICM_Idx, 0, 0) == 0xFF) {
			fwif_color_ICM_RGB_Offset_Table_Select(_MAIN_DISPLAY, pVipCoefArray[VIP_QUALITY_FUNCTION_ICM]);
		} else {
			rtd_pr_vpq_info("ICM offset set by DEM, skip flow in quality handler\n");
		}

		if (HSR_mode && pVipCoefExt4Array)
			Scaler_SetSharpnessTable(pVipCoefExt4Array[VIP_QUALITY_FUNCTION_HSR_SharpTable]);
		else
			Scaler_SetSharpnessTable(pVipCoefArray[VIP_QUALITY_FUNCTION_SharpTable]);

		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) {
			if (HSR_mode && pVipCoefExt4Array)
				Scaler_SetMBPeaking(pVipCoefExt4Array[VIP_QUALITY_FUNCTION_HSR_MB_Peaking]);
			else
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
		if (gSrnnTableInfo.TableSrc == SRNN_TBL_SRC_DEFAULT) {
			Scaler_SetNNSR_Ctrl_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_SRNN_Ctrl]);
			Scaler_SetNNSR_Blending_Weight_Table(pVipCoefArray[VIP_QUALITY_FUNCTION_SRNN_Blending]);
		}
	}

	// no need, set 0xff to reset on reset mode
	//Scaler_color_Set_PQ_ByPass_Lv(0);	/* resort PQ bypass lv, since all PQ function is reset*/
#ifndef BUILD_QUICK_SHOW
	fwif_color_set_quality_init_sendRPC();
	scalerVIP_quality_init((unsigned long)&quality_init_value[0]);
#endif
	fwif_color_Set_MNR_Enable_Check_tv006();/*snr hw issue, can't get porch while hal function calling, elieli*/

	/* LineOD bring up verify */
	//fwif_color_set_pcid2(1); // enable to set the default setting
	//fwif_color_set_pcid2(0); // disable

	/* VALC bring up verify */
	//fwif_color_set_valc(1); // enable to set the default setting
	//fwif_color_set_valc(0); // disable

#ifdef VIP_SUPPORT_Freq_Detection
	/* Frequency detection bring up verify */
	if (!g_bFreqDet_Setting_by_ISR) {
		fwif_color_set_frequency_detection(pVipCoefArray[VIP_QUALITY_FUNCTION_FreqencyDetection]);
	} else {
		drvif_color_set_frequency_detection_enable(1);
	}
#endif

	/*fwif_color_video_quality_handler_customization();*/
#ifndef BUILD_QUICK_SHOW
	/* 3d mode young 3dtable ================*/
	if (drvif_scaelr3d_decide_is_3D_display_mode() == 1) {/*3*/
		/*fwif_color_video_3Dmode_quality_handler();*/
	}
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
	}
#ifndef BUILD_QUICK_SHOW	
	else if (Scaler_access_Project_TV002_Style(0, 0) == Project_TV002_Style_1) {
		extern unsigned char g_bEnable_PQ_extend_data;
		extern short ColorMap_Matrix_Apply[3][3];
		if (!g_bEnable_PQ_extend_data)
			fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, ColorMap_Matrix_Apply, 1, 1, 0);
	}
#endif
#if defined(CONFIG_RTK_8KCODEC_INTERFACE) || defined(CONFIG_H5CX_SUPPORT)
	VPQEX_rlink_host_which_source();
#endif	
    Scaler_Set_Camera_PQsetting(get_vo_camera_flow_flag(Get_DisplayMode_Port(SLR_MAIN_DISPLAY)));
	#ifdef AUTO_DE_CONTOUR_I2R
	pre_source = source;
	#endif
	/*force background check*/
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	ppoverlay_main_display_control_rsv.regValue= IoReg_Read32(PPOVERLAY_Main_Display_Control_RSV_reg);
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	if (ppoverlay_main_display_control_rsv.m_force_bg == 0) /*normal mode*/
		VIPprintf("[Warnning] [%s-%s-%d] End, Not Black Screen", __FILE__, __func__, __LINE__);
#ifndef BUILD_QUICK_SHOW
	bPictureEnabled = 1;
#else
	fwif_color_video_quality_handler_quick_show();
#endif
#if 0
	if (Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_MAIN_DISPLAY) {
		if(Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID_PRE)>1920){
			IoReg_SetBits(DI_IM_DI_RTNR_NEW_CONTROL_reg, _BIT19);
		}else{
			IoReg_ClearBits(DI_IM_DI_RTNR_NEW_CONTROL_reg, _BIT19);
		}
	}
#endif
#ifdef CONFIG_RTK_SRE_AATS_UCT_AT
	//AT V6.2 Patch 2022/03/31 - START
	//Added by David Kuo, 20220523
	if(PQ_UT_apply_Linear_Flag==1) {
		extern unsigned int g_ScalerApplyPQLinear;
		unsigned int hour, minute, second, millisecond;
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
		struct tm time;
#else
		extern void get_local_time(unsigned int* ,unsigned int* , unsigned int* , unsigned int*);
#endif
		rtd_pr_vpq_emerg("PQ_linear mode apply \n");
		Scaler_apply_PQ_Linear(1);
		g_ScalerApplyPQLinear = 1;
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
		if(str_status == 0){
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
			struct timespec64 ts;
			ktime_get_real_ts64(&ts);
#else
			struct timeval ts;
			do_gettimeofday(&ts);
#endif
			second = ts.tv_sec;
			millisecond = (ts.tv_nsec/1000000);
			second -= (sys_tz.tz_minuteswest * 60);
			time64_to_tm(second, 0, &time);
			hour = time.tm_hour;
			minute = time.tm_min;
			second = time.tm_sec;
			rtd_pr_vpq_emerg("####[David] PQ_linear mode on, jiffies: %lu, rtc[ms]: %lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies,jiffies*1000/HZ,hour,minute,second,millisecond);
		}else{
			rtd_pr_vpq_emerg("####[David] PQ_linear mode on, jiffies: %lu, rtc[ms]: %lu, str_status: %01d\n", jiffies,jiffies*1000/HZ,str_status);
		}
#else
		get_local_time(&hour,&minute,&second,&millisecond);
		rtd_pr_vpq_emerg("####[David] PQ_linear mode on, jiffies: %lu, rtc[ms]: %lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies,jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
	}
	//AT V6.2 Patch 2022/03/31 - END
#endif
	rtd_printk(KERN_INFO, TAG_NAME, "[Decide RTNR][RTNR_BW Status][%d][%x]\n",(rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg)>>19)&0x1, rtd_inl(DI_IM_DI_WEAVE_WINDOW_CONTROL_reg));

}

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
void* scaler_VIP_PQMask_GetSemanticTable(unsigned char bLogEnable) {

	unsigned char *pVipCoefArray = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if( m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] source num %d error\n", __FUNCTION__, m_nowSource);
		return NULL;
	}

	if (m_cbFunc_getQualityExtend != NULL) {
		if(bLogEnable!=0)
			rtd_printk(KERN_INFO, TAG_NAME, "[%s] CoefArray with source %d\n", __FUNCTION__, m_nowSource);
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend((VIP_SOURCE_TIMING)m_nowSource);
	} else {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] PQ table m_cbFunc_getQualityExtend read fail\n", __FUNCTION__);
		return NULL;
	}

	if( gVip_Table == NULL ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] VIP table null\n", __FUNCTION__);
		return NULL;
	}

	if (!(m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		if( pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskSemanticWeight_Table] >= PQMASK_SEMANTIC_WEIGHT_NUM ) {
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] Table index = %d over range\n", __FUNCTION__, pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskSemanticWeight_Table]);
			return NULL;
		} else {
			if(bLogEnable!=0)
				rtd_printk(KERN_INFO, TAG_NAME, "[%s] Table src = %d, index = %d\n", __FUNCTION__, m_nowSource, pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskSemanticWeight_Table]);
			return (void *)(&gVip_Table->PQMask_Semantic_WeightTable[pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskSemanticWeight_Table]]);
		}
	} else {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] source = %d, pVipCoefArray is null %d\n", __FUNCTION__, m_nowSource, (pVipCoefArray==NULL));
		return NULL;
	}

	rtd_printk(KERN_EMERG, TAG_NAME, "[%s] exit with error\n", __FUNCTION__);
	return NULL;
}

void* scaler_VIP_PQMask_GetDepthTable(unsigned char bLogEnable) {

	unsigned char *pVipCoefArray = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if( m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] source num %d error\n", __FUNCTION__, m_nowSource);
		return NULL;
	}

	if (m_cbFunc_getQualityExtend != NULL) {
		if(bLogEnable!=0)
			rtd_printk(KERN_INFO, TAG_NAME, "[%s] CoefArray with source %d\n", __FUNCTION__, m_nowSource);
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend((VIP_SOURCE_TIMING)m_nowSource);
	} else {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] PQ table m_cbFunc_getQualityExtend read fail\n", __FUNCTION__);
		return NULL;
	}

	if( gVip_Table == NULL ) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] VIP table null\n",  __FUNCTION__);
		return NULL;
	}

	if (!(m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		if( pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskDepthWeight_Table] >= PQMASK_DEPTH_WEIGHT_NUM ) {
			rtd_printk(KERN_EMERG, TAG_NAME, "[%s] PQMask Depth Table index = %d over range\n", __FUNCTION__, pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskDepthWeight_Table]);
			return NULL;
		} else {
			if(bLogEnable!=0)
				rtd_printk(KERN_INFO, TAG_NAME, "[%s] PQMask Depth Table src = %d, index = %d\n", __FUNCTION__, m_nowSource, pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskDepthWeight_Table]);
			return (void *)(&gVip_Table->PQMask_Depth_WeightTable[pVipCoefArray[VIP_QUALITY_FUNCTION_PQMaskDepthWeight_Table]]);
		}
	} else {
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s] source = %d, pVipCoefArray is null %d\n", __FUNCTION__, m_nowSource, (pVipCoefArray==NULL));
		return NULL;
	}

	rtd_printk(KERN_EMERG, TAG_NAME, "[%s] exit with error\n", __FUNCTION__);
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
	unsigned char *pVipCoefExt4Array = NULL;
	VIP_CDS_Table CDS_Table;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	unsigned char HSR_mode = 0;

#ifdef CONFIG_DYNAMIC_PANEL_SELECT
#ifndef BUILD_QUICK_SHOW
	if(PANEL_DLG_4k1k120 == vbe_get_cur_dlg_mode())
		HSR_mode = 1;
#endif
#endif

	if (m_cbFunc_getQualityExtend4 != NULL)
		pVipCoefExt4Array = (unsigned char *)m_cbFunc_getQualityExtend4((VIP_SOURCE_TIMING)m_nowSource);

	// for reset sharpness
	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)m_nowSource);
	else
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend3 read fail !!!\n");

	//if (!(m_nowSource >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
	if (m_nowSource < VIP_QUALITY_TOTAL_SOURCE_NUM && pVipCoefArray != NULL) {
		if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000) { /*pre_id:16 main_id:8 sub1_id:8 sub2_id:8, example->lg=tv006*/
			if (HSR_mode && pVipCoefExt4Array)
				Scaler_SetSharpnessTable(pVipCoefExt4Array[VIP_QUALITY_FUNCTION_HSR_SharpTable]);
			else
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
	unsigned char *pVipCoefArray = NULL;
	/*int ret;*/
	/*unsigned char spatial_y = 0, spatial_c = 0, temporal = 0, rtnr_level_y = 0, rtnr_level_c = 0, s_dnr_level = 0;*/

	SLR_VIP_TABLE *gVIP_Table;
	unsigned char *drvSkip_Array;

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

#endif //UT_flag
void fwif_color_set_grid_remove(unsigned char src_idx, unsigned char value)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->gridRemove = value;
}
#ifndef UT_flag
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

	if(((get_hdmi_4k_hfr_mode() != HDMI_NON_4K120)||force_enable_two_step_uzu() || get_hdmi_vrr_4k60_mode()) && (Get_DISPLAY_REFRESH_RATE() >= 120))
		return;

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

	if(((get_hdmi_4k_hfr_mode() != HDMI_NON_4K120)||force_enable_two_step_uzu() || get_hdmi_vrr_4k60_mode()) && (Get_DISPLAY_REFRESH_RATE() >= 120))
		return;

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

unsigned char Scaler_APDEM_En_Access(unsigned char en, unsigned char isSetFlag)	// this API can control by tool
{
	static unsigned char AP_DIAS_EngineerMenu_En = 1;	// init value here
	unsigned char i = 0;
	
	if (isSetFlag == 1) {
		AP_DIAS_EngineerMenu_En = en;
		if (en == 0)
			for (i=0;i<DEM_ARG_CTRL_ITEMS_Max;i++)	// reset all to 0xff
				Scaler_APDEM_Arg_Access(i, 0xFF, 1);
		rtd_pr_vpq_info("AP_DIAS_EngineerMenu_En=%d,\n", AP_DIAS_EngineerMenu_En);
	}
	return AP_DIAS_EngineerMenu_En;
}

#if 0 	// 1000
static long Color_Space_CM_OFF[3][3]={	// need to check matrix
	{486,	265,	198},	// {0.486570949,	0.265667693,	0.198217285},
	{228,	691,	79},	// {0.228974564,	0.691738522,	0.079286914},
	{0,		045,	1043},	// {0,				0.045113382,	1.043944369},
};

static long Color_Space_CM_AUTO[3][3]={	// need to check matrix
	{486,	265,	198},	// {0.486570949,	0.265667693,	0.198217285},
	{228,	691,	79},	// {0.228974564,	0.691738522,	0.079286914},
	{0,		045,	1043},	// {0,				0.045113382,	1.043944369},
};

static long Color_Space_CM_sRGB[3][3]={	// need to check matrix
	{486,	265,	198},	// {0.486570949,	0.265667693,	0.198217285},
	{228,	691,	79},	// {0.228974564,	0.691738522,	0.079286914},
	{0,		045,	1043},	// {0,				0.045113382,	1.043944369},
};

static long Color_Space_CM_DCIP3Target[3][3]={
	{486,	265,	198},	// {0.486570949,	0.265667693,	0.198217285},
	{228,	691,	79},	// {0.228974564,	0.691738522,	0.079286914},
	{0,		045,	1043},	// {0,				0.045113382,	1.043944369},
};
static long Color_Space_CM_ADOBERGBTarget[3][3]={
	{576,	185,	188},	// {0.576669043,	0.185558238,	0.188228646},
	{297,	627,	075},	// {0.297344975,	0.627363566,	0.075291458},
	{027,	070,	991},	// {0.027031361,	0.070688853,	0.991337537},
};

static long Color_Space_CM_BT2020Target[3][3]={
	{636,	144,	168},	// {0.636958048,	0.144616904,	0.168880975},
	{262,	677,	593},	// {0.262700212,	0.677998072,	0.059301716},
	{0,		280,	1060},	// {0,				0.028072693,	1.060985058},
};

static long Color_Space_CM_BT709Target[3][3]={
	{412,	357,	180},	// {0.412390799,	0.357584339,	0.180480788},
	{212,	715,	072},	// {0.212639006,	0.715168679,	0.072192315},
	{019,	119,	950},	// {0.019330819,	0.11919478,	0.950532152},
};
#else	// 100000
static long long Color_Space_CM_DCIP3Target[3][3]={
	{48657,	26566,	19821},	// {0.486570949,	0.265667693,	0.198217285},
	{22897,	69173,	7928},	// {0.228974564,	0.691738522,	0.079286914},
	{0,		4511,	104394},	// {0,				0.045113382,	1.043944369},
};
static long long Color_Space_CM_ADOBERGBTarget[3][3]={
	{57666,	18555,	18822},	// {0.576669043,	0.185558238,	0.188228646},
	{29734,	62736,	7529},	// {0.297344975,	0.627363566,	0.075291458},
	{2703,	7068,	99133},	// {0.027031361,	0.070688853,	0.991337537},
};

static long long Color_Space_CM_BT2020Target[3][3]={
	{63695,	14461,	16888},	// {0.636958048,	0.144616904,	0.168880975},
	{26270,	67799,	5930},	// {0.262700212,	0.677998072,	0.059301716},
	{0,		2807,	106098},	// {0,				0.028072693,	1.060985058},
};

static long long Color_Space_CM_BT709Target[3][3]={
	{41239,	35758,	18048},		//{0.412390799,	0.357584339,	0.180480788},
	{21263,	71516,	 7219},		//{0.212639006,	0.715168679,	0.072192315},
	{ 1933,	11919,	95053},		//{0.019330819,	0.11919478,	0.950532152},
};
#endif
char get_iveMatrix(long long matrix[3][3], long long matrix_inv[3][3], unsigned int scale)
{
	long long determinant=0;
	long long b[3][3];
	unsigned char i,j;

	determinant=(matrix[0][0]*matrix[1][1]*matrix[2][2]+matrix[0][1]*matrix[1][2]*matrix[2][0]+matrix[0][2]*matrix[1][0]*matrix[2][1])-
		(matrix[0][2]*matrix[1][1]*matrix[2][0]+matrix[0][1]*matrix[1][0]*matrix[2][2]+matrix[0][0]*matrix[1][2]*matrix[2][1]);
	//determinant = determinant / scale / scale;

	b[0][0]=matrix[1][1]*matrix[2][2]-matrix[1][2]*matrix[2][1];
	b[0][1]=-(matrix[0][1]*matrix[2][2]-matrix[0][2]*matrix[2][1]);
	b[0][2]=matrix[0][1]*matrix[1][2]-matrix[0][2]*matrix[1][1];

	b[1][0]=-(matrix[1][0]*matrix[2][2]-matrix[1][2]*matrix[2][0]);
	b[1][1]=(matrix[0][0]*matrix[2][2]-matrix[0][2]*matrix[2][0]);
	b[1][2]=-(matrix[0][0]*matrix[1][2]-matrix[0][2]*matrix[1][0]);

	b[2][0]=(matrix[1][0]*matrix[2][1]-matrix[1][1]*matrix[2][0]);
	b[2][1]=-(matrix[0][0]*matrix[2][1]-matrix[0][1]*matrix[2][0]);
	b[2][2]=(matrix[0][0]*matrix[1][1]-matrix[0][1]*matrix[1][0]);

	rtd_pr_vpq_info("get_iveMatrix,scale=%d,determinant=%ld,b=%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,\n",
		scale, determinant, b[0][0], b[0][1], b[0][2], b[1][0], b[1][1], b[1][2], b[2][0], b[2][1], b[2][2]);
	
	if(determinant==0){
		rtd_pr_vpq_emerg("get_iveMatrix,determinant=%ld\n",determinant);
		return -1;
	}

     	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		 	// use scale^2 domain , for precision
			matrix_inv[i][j]=(b[i][j]*scale)/(determinant/scale);	// (b[i][j]*scale)/(determinant/scale) = b[i][j]*scale*scale/determinant
       }
#if 0
	RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, "---mul3x3--\n");
		for(i=0;i<3;i++){
		 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " a[%d][%d]=%.9f\n",i,j,a[i][j]);
		}
	
	for(i=0;i<3;i++){
		 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " b[%d][%d]=%.9f\n",i,j,b[i][j]);
		}
	for(i=0;i<3;i++){
		 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " out[%d][%d]=%.9f\n",i,j,out[i][j]);
		}
#endif

	return 0;

}

char mul3x1(long long a[3][3], long long b[3][1], long long out[3][1], unsigned int scale)
{
	//int i, j;

	out[0][0]=a[0][0]*b[0][0]+a[0][1]*b[1][0]+a[0][2]*b[2][0];
	out[1][0]=a[1][0]*b[0][0]+a[1][1]*b[1][0]+a[1][2]*b[2][0];
	out[2][0]=a[2][0]*b[0][0]+a[2][1]*b[1][0]+a[2][2]*b[2][0];

	#if 0
	RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, "---mul3x1--\n");
     	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " a[%d][%d]=%.9f\n",i,j,a[i][j]);
     	}
	for(i=0;i<3;i++){
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " b[%d][%d]=%.9f\n",i,j,b[i][0]);
     	}
	for(i=0;i<3;i++){
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " out[%d][0]=%.9f\n",i,out[i][0]);
     	}
	#endif
	return  0;
}

char mul3x3(long long a[3][3], long long b[3][3], long long out[3][3], unsigned int scale)
{
	int i, j, k;

	for(i=0; i<3; i++)
		for(j=0; j<3; j++)
		{
			out[i][j]=0;
			for(k=0; k<3; k++) {
				out[i][j]=out[i][j]+a[i][k]*b[k][j];
			}
		}
#if 0
	RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, "---mul3x3--\n");
     	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " a[%d][%d]=%.9f\n",i,j,a[i][j]);
     	}

	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " b[%d][%d]=%.9f\n",i,j,b[i][j]);
     	}
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " out[%d][%d]=%.9f\n",i,j,out[i][j]);
     	}
#endif
	return  0;

}

static int Scaler_APDEM_Color_Space_CM_Cal(unsigned int panal_para_scale)
{
	//SLR_VIP_TABLE *pVipTable = fwif_colo_get_AP_vip_table_gVIP_Table();
	
	long long xyz_of_W[3][1],y;
	long long xyz_of_RGB[3][3];
	long long xyz_of_RGB_inv[3][3];
	long long beta3X3[3][3],beta3X1[3][1];
	long long RGB2XYZ[3][3],XYZ2RGB[3][3],sRGB[3][3];
	unsigned char i,j,result_1,result_2,result_3,result_4,/*result_5,*/result_6,result_7,result_8,result_9/*,result_10*/;

	//short sRGB_BT709[3][3],sRGB_BT2020[3][3],sRGB_DCIP3[3][3],sRGB_ADOBERGB[3][3], sRGB_sRGB[3][3], sRGB_Auto[3][3];

	//int ret;

	/*if (pData == NULL || pVipTable == NULL) {
		rtd_pr_vpq_emerg("%s pData = %p, pVipTable = %p\n", __FUNCTION__, pData, pVipTable);
		return -1;
	}*/

	//memcpy(&VIP_AP_DEM_TBL.PANEL_DATA_, pData, sizeof(VIP_PANEL_DATA_T));

	//gen target panel XYZ2RGB & RGB2XYZ matrix
	xyz_of_W[0][0]= VIP_AP_DEM_TBL.PANEL_DATA_.DEM_PANEL_INI_Color_Chromaticity.Panel_white_x;	// primary_wihte.un16_CIE_x;//xw_target;
	xyz_of_W[1][0]= VIP_AP_DEM_TBL.PANEL_DATA_.DEM_PANEL_INI_Color_Chromaticity.Panel_white_y;	// primary_wihte.un16_CIE_y;//yw_target;
	xyz_of_W[2][0]= panal_para_scale-xyz_of_W[0][0]-xyz_of_W[1][0];
	xyz_of_RGB[0][0]= VIP_AP_DEM_TBL.PANEL_DATA_.DEM_PANEL_INI_Color_Chromaticity.Panel_red_x;	// primary_r.un16_CIE_x;//xr_target;
	xyz_of_RGB[0][1]= VIP_AP_DEM_TBL.PANEL_DATA_.DEM_PANEL_INI_Color_Chromaticity.Panel_green_x;	// primary_g.un16_CIE_x;//xg_target;
	xyz_of_RGB[0][2]= VIP_AP_DEM_TBL.PANEL_DATA_.DEM_PANEL_INI_Color_Chromaticity.Panel_blue_x;	// primary_b.un16_CIE_x;//xb_target;
	xyz_of_RGB[1][0]= VIP_AP_DEM_TBL.PANEL_DATA_.DEM_PANEL_INI_Color_Chromaticity.Panel_red_y;	// primary_r.un16_CIE_y;//yr_target;
	xyz_of_RGB[1][1]= VIP_AP_DEM_TBL.PANEL_DATA_.DEM_PANEL_INI_Color_Chromaticity.Panel_green_y;	// primary_g.un16_CIE_y;//yg_target;
	xyz_of_RGB[1][2]= VIP_AP_DEM_TBL.PANEL_DATA_.DEM_PANEL_INI_Color_Chromaticity.Panel_blue_y;	// primary_b.un16_CIE_y;//yb_target;
	xyz_of_RGB[2][0]= panal_para_scale-xyz_of_RGB[0][0]-xyz_of_RGB[1][0];
	xyz_of_RGB[2][1]= panal_para_scale-xyz_of_RGB[0][1]-xyz_of_RGB[1][1];
	xyz_of_RGB[2][2]= panal_para_scale-xyz_of_RGB[0][2]-xyz_of_RGB[1][2];

	//step1 xyz_of_RGB-->xyz_of_RGB's inverse
	// for inv matrix, need to use scale^2 domain for precision
	result_1=get_iveMatrix(xyz_of_RGB,xyz_of_RGB_inv, panal_para_scale);

	//step2 xyz_of_RGB's inverse * xyz_of_W
	y=xyz_of_W[1][0];
	xyz_of_W[0][0]=xyz_of_W[0][0]*panal_para_scale/y;//---w/y
	xyz_of_W[1][0]=xyz_of_W[1][0]*panal_para_scale/y;
	xyz_of_W[2][0]=xyz_of_W[2][0]*panal_para_scale/y;
	result_2=mul3x1(xyz_of_RGB_inv,xyz_of_W,beta3X1, panal_para_scale);
		
	rtd_pr_vpq_info("APDEM_CM_Cal, xyz_of_RGB=%d,%d,%d,%d,%d,%d,%d,%d,%d,\n", 
		xyz_of_RGB[0][0], xyz_of_RGB[0][1], xyz_of_RGB[0][2], xyz_of_RGB[1][0], xyz_of_RGB[1][1], xyz_of_RGB[1][2], xyz_of_RGB[2][0], xyz_of_RGB[2][1], xyz_of_RGB[2][2]);
	rtd_pr_vpq_info("APDEM_CM_Cal, xyz_of_RGB_inv=%d,%d,%d,%d,%d,%d,%d,%d,%d,\n", 
		xyz_of_RGB_inv[0][0], xyz_of_RGB_inv[0][1], xyz_of_RGB_inv[0][2], xyz_of_RGB_inv[1][0], xyz_of_RGB_inv[1][1], xyz_of_RGB_inv[1][2], xyz_of_RGB_inv[2][0], xyz_of_RGB_inv[2][1], xyz_of_RGB_inv[2][2]);
	rtd_pr_vpq_info("APDEM_CM_Cal, beta3X1z=%d,%d,%d,\n", 
		xyz_of_W[0][0], xyz_of_W[1][0], xyz_of_W[2][0]);
	rtd_pr_vpq_info("APDEM_CM_Cal, beta3X1=%d,%d,%d,\n", 
		beta3X1[0][0], beta3X1[1][0], beta3X1[2][0]);

	//step3 xyz_of_RGB*beta3X3-->RGB2XYZ
	// return to scale^2 domain
	beta3X3[0][0]=beta3X1[0][0]/panal_para_scale;
	beta3X3[0][1]=0;
	beta3X3[0][2]=0;
	beta3X3[1][0]=0;
	beta3X3[1][1]=beta3X1[1][0]/panal_para_scale;
	beta3X3[1][2]=0;
	beta3X3[2][0]=0;
	beta3X3[2][1]=0;
	beta3X3[2][2]=beta3X1[2][0]/panal_para_scale;
	result_3=mul3x3(xyz_of_RGB,beta3X3,RGB2XYZ, panal_para_scale);

	rtd_pr_vpq_info("APDEM_CM_Cal, RGB2XYZ=%d,%d,%d,%d,%d,%d,%d,%d,%d,\n", 
		RGB2XYZ[0][0], RGB2XYZ[0][1], RGB2XYZ[0][2], RGB2XYZ[1][0], RGB2XYZ[1][1], RGB2XYZ[1][2], RGB2XYZ[2][0], RGB2XYZ[2][1], RGB2XYZ[2][2]);

	//step4 RGB2XYZ-->RGB2XYZ's inverse
	// return to scale domain, for invers RGB2XYZ to avoid overflow, inverse will extern to scale^2 domain
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
			RGB2XYZ[i][j] = RGB2XYZ[i][j]/panal_para_scale;
	result_4=get_iveMatrix(RGB2XYZ,XYZ2RGB, panal_para_scale);

	rtd_pr_vpq_info("APDEM_CM_Cal, XYZ2RGB=%d,%d,%d,%d,%d,%d,%d,%d,%d,\n", 
		XYZ2RGB[0][0], XYZ2RGB[0][1], XYZ2RGB[0][2], XYZ2RGB[1][0], XYZ2RGB[1][1], XYZ2RGB[1][2], XYZ2RGB[2][0], XYZ2RGB[2][1], XYZ2RGB[2][2]);

	//step5 XYZ2RGB*target-->sRGB
	/*result_5=mul3x3(XYZ2RGB,Color_Space_CM_BT709Target,sRGB);
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
				VIP_AP_DEM_TBL.Color_Space_Target_Apply[Color_Space_Target_sRGB][i][j]=sRGB[i][j]*1024;
		}*/
	result_6=mul3x3(XYZ2RGB,Color_Space_CM_BT2020Target,sRGB, panal_para_scale);
	// target value will mutiply scale, remove this scale
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
			sRGB[i][j] = sRGB[i][j]/panal_para_scale;
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
				VIP_AP_DEM_TBL.Color_Space_Target_Apply[Color_Space_Target_BT2020][i][j]=sRGB[i][j]*2048/(panal_para_scale);	// return to reg domain from 100000 domain;
		}
	
	result_7=mul3x3(XYZ2RGB,Color_Space_CM_DCIP3Target,sRGB, panal_para_scale);
	// target value will mutiply scale, remove this scale
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
			sRGB[i][j] = sRGB[i][j]/panal_para_scale;
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
				VIP_AP_DEM_TBL.Color_Space_Target_Apply[Color_Space_Target_DCIP3][i][j]=sRGB[i][j]*2048/(panal_para_scale);	// return to reg domain from 100000 domain;
		}
	
	result_8=mul3x3(XYZ2RGB,Color_Space_CM_ADOBERGBTarget,sRGB, panal_para_scale);
	// target value will mutiply scale, remove this scale
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
			sRGB[i][j] = sRGB[i][j]/panal_para_scale;
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
				VIP_AP_DEM_TBL.Color_Space_Target_Apply[Color_Space_Target_AdobeRGB][i][j]=sRGB[i][j]*2048/(panal_para_scale);	// return to reg domain from 100000 domain;
		}
	
	result_9=mul3x3(XYZ2RGB,Color_Space_CM_BT709Target,sRGB, panal_para_scale);
	// target value will mutiply scale, remove this scale
	for (i=0;i<3;i++)
		for (j=0;j<3;j++)
			sRGB[i][j] = sRGB[i][j]/panal_para_scale;
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
				VIP_AP_DEM_TBL.Color_Space_Target_Apply[Color_Space_Target_sRGB][i][j]=sRGB[i][j]*2048/(panal_para_scale);	// return to reg domain from 100000 domain;
		}

	// Color_Space_Target_Auto and off	
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++) {
		 	if (i==j) {
				VIP_AP_DEM_TBL.Color_Space_Target_Apply[Color_Space_Target_OFF][i][j] = 2048;
				VIP_AP_DEM_TBL.Color_Space_Target_Apply[Color_Space_Target_Auto][i][j] = 2048;
		 	} else {
				VIP_AP_DEM_TBL.Color_Space_Target_Apply[Color_Space_Target_OFF][i][j] = 0;
				VIP_AP_DEM_TBL.Color_Space_Target_Apply[Color_Space_Target_Auto][i][j] = 0;
		 	}
       	 }
	}
	#if 0
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " sRGB_BT709[%d][%d]=%d\n",i,j,sRGB_BT709[i][j]);
     	}
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " sRGB_BT2020[%d][%d]=%d\n",i,j,sRGB_BT2020[i][j]);
     	}
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " sRGB_DCIP3[%d][%d]=%d\n",i,j,sRGB_DCIP3[i][j]);
     	}
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " sRGB_ADOBERGB[%d][%d]=%d\n",i,j,sRGB_ADOBERGB[i][j]);
     	}	
	#endif
	//=====================================================================

	#if 0
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " sRGB_BT709[%d][%d]=%d\n",i,j,sRGB_t->BT709Target[i][j]);
     	}
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " sRGB_BT2020[%d][%d]=%d\n",i,j,sRGB_t->BT2020Target[i][j]);
     	}
	for(i=0;i<3;i++){
       	 for(j=0;j<3;j++)
		  RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " sRGB_DCIP3[%d][%d]=%d\n",i,j,sRGB_t->DCIP3[i][j]);
     	}
	for(i=0;i<3;i++){
   	 	for(j=0;j<3;j++)
	  	RTD_LOG_DEBUG(RTD_LOG_MODULE_VPQ, " sRGB_ADOBERGB[%d][%d]=%d\n",i,j,sRGB_t->ADOBERGB[i][j]);
 	}
	#endif

	return 0;
}


static const unsigned char VIP_APDEM_MAX_VAL[DEM_ARG_CTRL_ITEMS_Max] = {
	0xFF,	// how many APDEM command is sent from AP
	VIP_ICM_TBL_X,
	DCC_Curve_Adjust_TABLE_MAX,
	LC_Table_NUM,
	DCC_SELECT_MAX,	// off, low, middle, high
	2,  // on, off
	101, // OSD value, 
	VIP_YUV2RGB_LEVEL_SELECT_MAX,
	DCC_SELECT_MAX,	// 
	DCC_SELECT_MAX,	// 
	VIP_CSC_DATA_RANGE_MODE_MAX, 		// auto, full, limit
	VIP_Gamma_exp_p_max,
	1,	// 0: get 10p offset value from AP, 0xFF:AP didn't set 10p value
	VIP_DEM_PANEL_INI_Gamma_TBL_NUM,	// inv out gamma table max
	VIP_DEM_Color_Space_Apply_MAX, 
	1, 	// 0: get 3D lut offset value from AP, 0xFF:AP didn't set 3D lut value
	1, 	// 0: get value from AP, 0xFF:AP didn't set value
	1, 	// 0: get value from AP, 0xFF:AP didn't set value
	1, 	// 0: get value from AP, 0xFF:AP didn't set value
	VIP_DEM_Dynamic_Black_Equalize_TBL_NUM,	// gain table select
	VIP_DEM_Shadow_Detail_TBL_NUM, // 	// gain table select
};

unsigned char Scaler_APDEM_Arg_Access(VIP_DEM_ARG_CTRL_ITEMS ID, unsigned char val, unsigned char isSetFlag)
{
	static unsigned char DEM_arg[DEM_ARG_CTRL_ITEMS_Max] = 
		{0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned int i, cnt;
	
	if (ID >= DEM_ARG_CTRL_ITEMS_Max) {
		rtd_pr_vpq_emerg("%s, unknow ID, ID=%d,\n", __func__, ID);
		return 0xFF;
	}
	
	if ((isSetFlag == 1) && (Scaler_APDEM_En_Access(0, 0) == 1)) {
		if ((val >= VIP_APDEM_MAX_VAL[ID]) && (val != 0xFF)) {	// 0xff => set as init
			rtd_pr_vpq_emerg("%s, index > max, ID=%d, index=%d,\n", __func__, ID, val);
			val = 0;
		}
		DEM_arg[ID] = val;

		cnt = 0;
		for (i=1;i<DEM_ARG_CTRL_ITEMS_Max;i++) {
			if (DEM_arg[i] != 0xFF)
				cnt++;
		}
			
		DEM_arg[DEM_command_Sent_NUM] = cnt;	// how many APDEM command is sent from AP
		
		rtd_pr_vpq_info("DEM_arg_val[%d]=%d, cnt=%d,\n", ID, DEM_arg[ID], DEM_arg[DEM_command_Sent_NUM]);
	}
	return DEM_arg[ID];
}

extern void vpq_set_inv_gamma(unsigned char ucDoCtrlBack);
extern void vpq_init_gamma(void);
extern void vpq_set_gamma(unsigned char ucDoCtrlBack, unsigned char useRPC);
int Scaler_set_APDEM(VIP_DEM_ARG_CTRL_ITEMS ID, void *DEM_arg)
{
	unsigned int arg;
	unsigned int i;
	unsigned char idx_temp;
	int *pINT_Temp32;
	RTK_VPQ_COLORSPACE_INDEX VPQ_COLORSPACE = {0};

	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(VIP_system_info_structure_table==NULL /*||VIP_RPC_system_info_structure_table==NULL*/){
		rtd_pr_vpq_emerg("%s, VIP_system_info_structure_table NULL\n", __FUNCTION__);
		return -1;
	}	

	if (DEM_arg == NULL) {
		rtd_pr_vpq_emerg("%s, get null\n", __func__);
		return -1;
	}

	if (Scaler_APDEM_En_Access(0, 0) == 1) {
		switch(ID)
		{
			case DEM_ARG_ICM_Idx:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_ICM_Idx, arg, 1);

				// arg can't exceed max
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_ICM_Idx, 0, 0);

				// set 
				Scaler_set_ICM_table(arg,  0);
				fwif_color_ICM_RGB_Offset_Table_Select(_MAIN_DISPLAY, arg);				
			}
			break;

			case DEM_ARG_DCC_Idx:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_DCC_Idx, arg, 1);

				// arg can't exceed max
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_DCC_Idx, 0, 0);

				// set 
				Scaler_SetDCC_Table(arg);
				Scaler_Set_DCC_Color_Independent_Table(arg);
				Scaler_Set_DCC_chroma_compensation_Table(arg);
				Scaler_SetDCC_Mode(Scaler_GetDCC_Mode());
			}
			break;

			case DEM_ARG_LC_Idx:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_LC_Idx, arg, 1);

				// arg can't exceed max
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_LC_Idx, 0, 0);

				// set 
				Scaler_SetLocalContrastTable(arg);
				fwif_color_set_LC_Global_Region_Num_Size(arg, LC_BLOCK_48x27);
				if( (drvif_color_get_LC_size_support()!=0) && (Scaler_APDEM_Arg_Access(DEM_ARG_LC_level, 0, 0) != 0) )
					Scaler_SetLocalContrastEnable(1);
				else
					Scaler_SetLocalContrastEnable(0);
			}
			break;

			case DEM_ARG_LC_level:
			{
				arg = *((unsigned int*)DEM_arg);
				// lv offset set
				Scaler_APDEM_Arg_Access(DEM_ARG_LC_level, arg, 1);

				// set 				
				idx_temp = (Scaler_APDEM_Arg_Access(DEM_ARG_LC_Idx, 0, 0) >= LC_Table_NUM)?
					(0):(Scaler_APDEM_Arg_Access(DEM_ARG_LC_Idx, 0, 0));	

				fwif_color_set_LC_Shpnr_Gain1st(SLR_MAIN_DISPLAY, idx_temp);		

				if( (drvif_color_get_LC_size_support()!=0) && (Scaler_APDEM_Arg_Access(DEM_ARG_LC_level, 0, 0) != 0) )
					Scaler_SetLocalContrastEnable(1);
				else
					Scaler_SetLocalContrastEnable(0);

			}
			break;

			case DEM_ARG_After_Filter_En:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_En, arg, 1);
				
				Scaler_SetDNR(Scaler_GetDNR());
				Scaler_SetSharpnessTable(Scaler_GetSharpnessTable());
				Scaler_SetSharpness(Scaler_GetSharpness());
			}
			break;

			case DEM_ARG_After_Filter_Val:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, arg, 1);
				
				Scaler_SetDNR(Scaler_GetDNR());
				Scaler_SetSharpnessTable(Scaler_GetSharpnessTable());
				Scaler_SetSharpness(Scaler_GetSharpness());
			}
			break;

			case DEM_ARG_Blue_Stretch_level:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_Blue_Stretch_level, arg, 1);

				// arg can't exceed max
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_Blue_Stretch_level, 0, 0);

				// set 
				fwif_color_set_UV_Offset_byTBL_Level(0, SLR_MAIN_DISPLAY, Scaler_get_YUV2RGB(), arg, 0, 0);

			}
			break;

			case DEM_ARG_Black_Extension_level:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_Black_Extension_level, arg, 1);
			}
			break;

			case DEM_ARG_White_Extension_level:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_White_Extension_level, arg, 1);
			}
			break;

			case DEM_ARG_Data_Range:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, arg, 1);
				
				Scaler_Set_HDR_YUV2RGB(VIP_system_info_structure_table->HDR_flag, HAL_VPQ_COLORIMETRY_BT2020);
				Scaler_SetDataFormatHandler();

			}
			break;

			case DEM_ARG_Gamma_exp_byOffset:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_exp_byOffset, arg, 1);

				idx_temp = (Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_idx_set, 0, 0)>=VIP_DEM_PANEL_INI_Gamma_TBL_NUM)?(0):
					(Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_idx_set, 0, 0));
				//fwif_color_colorwrite_InvOutput_gamma(idx_temp);
				fwif_color_set_InvOutputGamma_System(idx_temp);
			}
			break;

			case DEM_ARG_10p_Gamma_Offset:
			{
				
#ifdef VIP_DEM_10p_Gamma_Offset_idx_102_to_1024
				extern unsigned short WBOffsetIREIdx[12]; // = {0, 51, 102, 205, 307, 410, 512, 614, 717, 819, 922, 1024};
				int offset_tmp2;
				pINT_Temp32 = (int*)DEM_arg;
				for (i=0;i<VIP_DEM_10p_Gamma_Offset_Num ;i++) {
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[i+2] = pINT_Temp32[i*3+0];
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_G[i+2] = pINT_Temp32[i*3+1];
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_B[i+2] = pINT_Temp32[i*3+2];
				}
				if ((WBOffsetIREIdx[0]<WBOffsetIREIdx[1]) && (WBOffsetIREIdx[1]<WBOffsetIREIdx[2])) {
					offset_tmp2 = VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[2];
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[1] = (WBOffsetIREIdx[1] - WBOffsetIREIdx[0]) * (VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[2]) / (WBOffsetIREIdx[2] - WBOffsetIREIdx[0]);
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_G[1] = (WBOffsetIREIdx[1] - WBOffsetIREIdx[0]) * (VIP_AP_DEM_TBL.DEM_10p_offset_TBL_G[2]) / (WBOffsetIREIdx[2] - WBOffsetIREIdx[0]);
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_B[1] = (WBOffsetIREIdx[1] - WBOffsetIREIdx[0]) * (VIP_AP_DEM_TBL.DEM_10p_offset_TBL_B[2]) / (WBOffsetIREIdx[2] - WBOffsetIREIdx[0]);
					// offet of index 0 should be 0;
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[0] = 0;
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_G[0] = 0;
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_B[0] = 0;
				} else {
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[0] = 0;
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_G[0] = 0;
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_B[0] = 0;
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[1] = 0;
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_G[1] = 0;
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_B[1] = 0;
	                rtd_pr_vpq_info("DEM_ARG_10p_Gamma_Offset, VIP_DEM_10p_Gamma_Offset_idx_102_to_1024, index error, WBOffsetIREIdx[0]=%d, WBOffsetIREIdx[1]=%d, WBOffsetIREIdx[2]=%d,\n", 
						WBOffsetIREIdx[0], WBOffsetIREIdx[1], WBOffsetIREIdx[2]);
				}
#else				
				pINT_Temp32 = (int*)DEM_arg;
				for (i=0;i<VIP_DEM_10p_Gamma_Offset_Num ;i++) {
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[i+1] = pINT_Temp32[i*3+0];
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_G[i+1] = pINT_Temp32[i*3+1];
					VIP_AP_DEM_TBL.DEM_10p_offset_TBL_B[i+1] = pINT_Temp32[i*3+2];
				}
				VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[0] = 0;
				VIP_AP_DEM_TBL.DEM_10p_offset_TBL_G[0] = 0;
				VIP_AP_DEM_TBL.DEM_10p_offset_TBL_B[0] = 0;
				VIP_AP_DEM_TBL.DEM_10p_offset_TBL_R[11] = 0;
				VIP_AP_DEM_TBL.DEM_10p_offset_TBL_G[11] = 0;
				VIP_AP_DEM_TBL.DEM_10p_offset_TBL_B[11] = 0;
#endif
				//memcpy(&VIP_AP_DEM_TBL.DEM_10p_offset_TBL[1], (unsigned int*)DEM_arg, sizeof(int)* VIP_DEM_10p_Gamma_Offset_Num);				
				Scaler_APDEM_Arg_Access(DEM_ARG_10p_Gamma_Offset, 0, 1);

				idx_temp = (Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_idx_set, 0, 0)>=VIP_DEM_PANEL_INI_Gamma_TBL_NUM)?(0):
					(Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_idx_set, 0, 0));
				//fwif_color_colorwrite_InvOutput_gamma(idx_temp);
				fwif_color_set_InvOutputGamma_System(idx_temp);
			}
			break;

			case DEM_ARG_Gamma_idx_set:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_idx_set, arg, 1);

				// arg can't exceed max
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_idx_set, 0, 0);

				fwif_color_set_InvOutputGamma_System(arg);
				//fwif_color_colorwrite_InvOutput_gamma(arg);
				//fwif_color_set_InvOutputGamma_System(arg);
			}
			break;

			case DEM_ARG_Color_Space:
			{
				arg = *((unsigned int*)DEM_arg);
#if 0
				// first time, set inv gamma and gamma
				if (Scaler_APDEM_Arg_Access(DEM_ARG_Color_Space, 0, 0) == 0xFF) {
					Scaler_APDEM_Arg_Access(DEM_ARG_Color_Space, arg, 1);
					//vpq_set_inv_gamma(1);
					fwif_color_set_InvGamma(SLR_MAIN_DISPLAY, 0, &Power2p2Gamma_14bit[0], &Power2p2Gamma_14bit[0], &Power2p2Gamma_14bit[0]);
					fwif_set_gamma_system_enhance(&Power0p45Gamma_14bit[0], &Power0p45Gamma_14bit[0], &Power0p45Gamma_14bit[0]);
				} else {
					Scaler_APDEM_Arg_Access(DEM_ARG_Color_Space, arg, 1);
				}
#else
				Scaler_APDEM_Arg_Access(DEM_ARG_Color_Space, arg, 1);
#endif				
				// arg can't exceed max
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_Color_Space, 0, 0);
				if (arg == Color_Space_Target_Auto)
					idx_temp = fwif_color_get_APDEM_CM_IDX();
				else
					idx_temp = arg;
				//fwif_color_set_ColorMap_3x3_Table(arg);
				Scaler_Set_ColorMapping_3x3_Table(idx_temp);
                rtd_pr_vpq_info("DEM_arg_Color_Space:%d\n", arg, idx_temp);
				
				if (arg == Color_Space_Target_OFF) {
					drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
					//drvif_color_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
					drvif_color_gamma_enable_enhance(SLR_MAIN_DISPLAY, 0);
				} else {
					drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 1, 0);
					//drvif_color_gamma_enable(SLR_MAIN_DISPLAY, 1, 0);
					drvif_color_gamma_enable_enhance(SLR_MAIN_DISPLAY, 1);
				}
			}
			break;

			case DEM_ARG_D_3DLUT_Offset:
			{
				memcpy(&VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[0], (unsigned int*)DEM_arg, sizeof(int)* VIP_DEM_3D_LUT_Offset_Num);						
				Scaler_APDEM_Arg_Access(DEM_ARG_D_3DLUT_Offset, 0, 1);

				VPQ_COLORSPACE.CS_CUSTOM_RED.red = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[0];
				VPQ_COLORSPACE.CS_CUSTOM_RED.green = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[1];
				VPQ_COLORSPACE.CS_CUSTOM_RED.blue = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[2];
				VPQ_COLORSPACE.CS_CUSTOM_GREEN.red = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[3];
				VPQ_COLORSPACE.CS_CUSTOM_GREEN.green = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[4];
				VPQ_COLORSPACE.CS_CUSTOM_GREEN.blue = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[5];
				VPQ_COLORSPACE.CS_CUSTOM_BLUE.red = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[6];
				VPQ_COLORSPACE.CS_CUSTOM_BLUE.green = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[7];
				VPQ_COLORSPACE.CS_CUSTOM_BLUE.blue = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[8];
				VPQ_COLORSPACE.CS_CUSTOM_YELLOW.red = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[9];
				VPQ_COLORSPACE.CS_CUSTOM_YELLOW.green = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[10];
				VPQ_COLORSPACE.CS_CUSTOM_YELLOW.blue = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[11];
				VPQ_COLORSPACE.CS_CUSTOM_CYAN.red = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[15];
				VPQ_COLORSPACE.CS_CUSTOM_CYAN.green = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[16];
				VPQ_COLORSPACE.CS_CUSTOM_CYAN.blue = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[17];
				VPQ_COLORSPACE.CS_CUSTOM_MAGENTA.red = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[12];
				VPQ_COLORSPACE.CS_CUSTOM_MAGENTA.green = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[13];
				VPQ_COLORSPACE.CS_CUSTOM_MAGENTA.blue = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[14];
				//VPQ_COLORSPACE.CS_CUSTOM_FLESH_TONE.red = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[18];
				//VPQ_COLORSPACE.CS_CUSTOM_FLESH_TONE.green = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[19];
				//VPQ_COLORSPACE.CS_CUSTOM_FLESH_TONE.blue = VIP_AP_DEM_TBL.VPQ_COLORSPACE_INDEX[20];
				
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_D_3DLUT_Offset, 0, 0);
				Scaler_Set_ColorSpace_D_3dlutTBL(&VPQ_COLORSPACE);
			}
			break;

			case DEM_ARG_Gamma_CURVE_TBL_PANEL_Chromaticity:
			{
				memcpy(&VIP_AP_DEM_TBL.PANEL_DATA_, DEM_arg, sizeof(VIP_DEM_PANEL_INI_TBL));

				Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_CURVE_TBL_PANEL_Chromaticity, 0, 1);
				
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_CURVE_TBL_PANEL_Chromaticity, 0, 0);

				Scaler_APDEM_Color_Space_CM_Cal(APDEM_Panal_ini_scale);

				Scaler_color_copy_HDR_table_panel_luminance_from_AP();
			}
			break;

			case DEM_ARG_LPF_Gain_ColorDataFac:
			{
				memcpy(&VIP_AP_DEM_TBL.ColorDataFac, (unsigned int*)DEM_arg, sizeof(VIP_DEM_ColorDataFac));						
				Scaler_APDEM_Arg_Access(DEM_ARG_LPF_Gain_ColorDataFac, 0, 1);
				
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_LPF_Gain_ColorDataFac, 0, 0);
			}
			break;

			case DEM_ARG_OD_Gain:
			{
				pINT_Temp32 = (int*)DEM_arg;
				VIP_AP_DEM_TBL.OverDriver_Gain[0] = pINT_Temp32[0];
				VIP_AP_DEM_TBL.OverDriver_Gain[1] = pINT_Temp32[0];
				VIP_AP_DEM_TBL.OverDriver_Gain[2] = pINT_Temp32[0];
				
				Scaler_APDEM_Arg_Access(DEM_ARG_OD_Gain, 0, 1);
				fwif_color_set_od_gain_RGB(VIP_AP_DEM_TBL.OverDriver_Gain[0], VIP_AP_DEM_TBL.OverDriver_Gain[1], 
					VIP_AP_DEM_TBL.OverDriver_Gain[2]);
				arg = Scaler_APDEM_Arg_Access(DEM_ARG_OD_Gain, 0, 0);
			}
			break;

			case DEM_ARG_Dynamic_Black_Equalize_Gain:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_Dynamic_Black_Equalize_Gain, arg, 1);

				// arg can't exceed max
				//arg = Scaler_APDEM_Arg_Access(DEM_ARG_Dynamic_Black_Equalize_Gain, 0, 0);	
				// set new dcc curve in isr
			}
			break;

			case DEM_ARG_Shadow_detail_Gain:
			{
				arg = *((unsigned int*)DEM_arg);
				Scaler_APDEM_Arg_Access(DEM_ARG_Shadow_detail_Gain, arg, 1);

				// arg can't exceed max
				//arg = Scaler_APDEM_Arg_Access(DEM_ARG_Shadow_detail_Gain, 0, 0);	
				idx_temp = (Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_idx_set, 0, 0)>=VIP_DEM_PANEL_INI_Gamma_TBL_NUM)?(0):
					(Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_idx_set, 0, 0));
				//fwif_color_colorwrite_InvOutput_gamma(idx_temp);
				fwif_color_set_InvOutputGamma_System(idx_temp);
				
			}
			break;

			default:
			{	
				rtd_pr_vpq_emerg("%s, unknow ID\n", __func__);
			}
			return -1;
		}
	}

	return 0;
}

unsigned char Scaler_APDEM_OsdMapToRegValue(VIP_DEM_ARG_CTRL_ITEMS ID, unsigned char value)
{
	unsigned int temp = 0;
	unsigned char startPoint, endPoint;
	unsigned char reg_point0 = 0;
	unsigned char reg_point25 = 0;
	unsigned char reg_point50 = 0;
	unsigned char reg_point75 = 0;
	unsigned char reg_point100 = 0;

	switch (ID) {
		case DEM_ARG_After_Filter_Val:
			reg_point0 = VIP_AP_DEM_TBL.ColorDataFac.LPF_Gain_0;
			reg_point25 = VIP_AP_DEM_TBL.ColorDataFac.LPF_Gain_25;
			reg_point50 = VIP_AP_DEM_TBL.ColorDataFac.LPF_Gain_50;
			reg_point75 = VIP_AP_DEM_TBL.ColorDataFac.LPF_Gain_75;
			reg_point100 = VIP_AP_DEM_TBL.ColorDataFac.LPF_Gain_100;
			break;

		default:
			rtd_pr_vpq_emerg("%s, unknow ID or no need to re-maping, ID=%d,\n", __func__, ID);
			break;
	}


	if (value < 25) {
		startPoint = reg_point0;
		endPoint = reg_point25;
		temp = value;
	} else if ((value >=  25) && (value < 50)) {
		startPoint = reg_point25;
		endPoint = reg_point50;
		temp = value - 25;
	} else if ((value >=  50) && (value < 75)) {
		startPoint = reg_point50;
		endPoint = reg_point75;
		temp = value - 50;
	} else /*if (value >=  75) */{
		startPoint = reg_point75;
		endPoint = reg_point100;
		temp = value - 75;
	}

	if (endPoint >= startPoint) {
		temp = (endPoint - startPoint) * temp / 25;
		temp +=  startPoint;
	} else if ((endPoint < startPoint)) {
		temp = (startPoint-endPoint) * temp / 25;
		temp = startPoint - temp;
	}

	if (temp > 255)
		temp = 255;
		rtd_pr_vpq_emerg(" ID =%d,OSD_vaule=%d,reg_value=%d, reg_point0=%d,reg_point25=%d,reg_point50=%d,reg_point75=%d,reg_point100=%d \n", 
			ID,
			value,
			temp,
			reg_point0,
			reg_point25,
			reg_point50,
			reg_point75,
			reg_point100);

	/*printf2(" bOsdItem =%d,OSD_value=%d,reg_value=%d, reg_point0=%d,reg_point25=%d,reg_point50=%d,reg_point75=%d,reg_point100=%d \n", 
		bOsdItem,
		value,
		temp,
		reg_point0,
		reg_point25,
		reg_point50,
		reg_point75,
		reg_point100);*/

	return (unsigned char)temp;
}

static char Scaler_APDEM_PTG_Pos_Size_check(short srcx, short srcy, short src_wid, short src_height, unsigned short display_width, unsigned short display_heigth)
{
	if (((src_height + srcy) > display_heigth) || ((src_wid + srcx) > display_width) || (srcx < 0) || (srcy < 0)
		 || (src_wid < 0) || (src_height < 0)) {
		rtd_pr_vpq_emerg("%s check error,src_height=%d,srcy=%d,src_wid=%d,srcx=%d, display_heigth=%d, display_width=%d,\n", 
			__func__, src_height, srcy, src_wid, srcx, display_heigth, display_width);
		return -1;
	}

	return 0;
}

static char Scaler_APDEM_PTG_check(VIP_APDEM_PTG_CTRL *PTG_CTRL)
{
	if (PTG_CTRL == NULL) {
		rtd_pr_vpq_emerg("%s,PTG_CTRL=NULL,\n", __func__);
		return -1;
	}

	if ((PTG_CTRL->mute_flag > 1) || (PTG_CTRL->dis_flag > 1)) {
		rtd_pr_vpq_emerg("%s, input error,mute_flag=%d,display_flag=%d,\n", __func__, PTG_CTRL->mute_flag, PTG_CTRL->dis_flag);
		return -1;
	}

	if (PTG_CTRL->PTG_Pos_Mode >=  VIP_APDEM_PTG_Pos_Info_MAX) {
		rtd_pr_vpq_emerg("%s,PTG_CTRL->PTG_Pos_Mode error,PTG_CTRL->PTG_Pos_Mode=%d,\n", __func__, PTG_CTRL->PTG_Pos_Mode);
		return -1;
	}

	if ((PTG_CTRL->PTG_Pos_Mode !=  APDEM_PTG_Pos_byXY) && ((PTG_CTRL->PTG_Size < 1) || (PTG_CTRL->PTG_Size > 100))) {
		rtd_pr_vpq_emerg("%s,PTG_CTRL->PTG_Pos_Mode error,PTG_Pos_Mode=%d,PTG_Size=%d,\n", __func__, PTG_CTRL->PTG_Pos_Mode, PTG_CTRL->PTG_Size);
		return -1;
	}

	return 0;
}

static char Scaler_APDEM_PTG_Size_Pos_Cal(VIP_APDEM_PTG_CTRL *PTG_CTRL, StructSrcRect *out_windowPos_info)
{
	unsigned short display_width, display_heigth, h_s, h_e, v_s, v_e;
	short ptg_wid = -1, ptg_hei = -1, ptg_x = -1, ptg_y = -1, ptg_size;

	drvif_color_Get_main_den_h_v_start_end(&h_s, &h_e, &v_s, &v_e);

	display_width = h_e - h_s;
	display_heigth = v_e - v_s;
	
	ptg_size = PTG_CTRL->PTG_Size;
	if (PTG_CTRL->PTG_Pos_Mode == APDEM_PTG_Pos_byXY) {
		ptg_wid = PTG_CTRL->x_end - PTG_CTRL->x_sta + 1;
		ptg_hei = PTG_CTRL->y_end - PTG_CTRL->y_sta + 1;
		ptg_x = PTG_CTRL->x_sta;
		ptg_y = PTG_CTRL->y_sta;
	} else if (PTG_CTRL->PTG_Pos_Mode == APDEM_PTG_Pos_Center) {
		ptg_wid = display_width * ptg_size / 100;		
		ptg_hei = display_heigth * ptg_size / 100;
		ptg_x = (display_width>>1) - (ptg_wid>>1);
		ptg_y = (display_heigth>>1) - (ptg_hei>>1);
	} else if (PTG_CTRL->PTG_Pos_Mode == APDEM_PTG_Pos_UpLeft) {
		ptg_wid = display_width * ptg_size / 100;
		ptg_hei = display_heigth * ptg_size / 100;
		ptg_x = 0;
		ptg_y = 0;
	} else if (PTG_CTRL->PTG_Pos_Mode == APDEM_PTG_Pos_UpRight) {
		ptg_wid = display_width * ptg_size / 100;
		ptg_hei = display_heigth * ptg_size / 100;
		ptg_x = display_width - ptg_wid;
		ptg_y = 0;
	} else if (PTG_CTRL->PTG_Pos_Mode == APDEM_PTG_Pos_BottomLeft) {
		ptg_wid = display_width * ptg_size / 100;
		ptg_hei = display_heigth * ptg_size / 100;
		ptg_x = 0;
		ptg_y = display_heigth - ptg_hei;
	} else if (PTG_CTRL->PTG_Pos_Mode == APDEM_PTG_Pos_BottomRight) {
		ptg_wid = display_width * ptg_size / 100;
		ptg_hei = display_heigth * ptg_size / 100;
		ptg_x = display_width - ptg_wid;
		ptg_y = display_heigth - ptg_hei;
	}

	out_windowPos_info->srcx = ptg_x;
	out_windowPos_info->srcy = ptg_y;
	out_windowPos_info->src_wid = ptg_wid;
	out_windowPos_info->src_height = ptg_hei;

	if (Scaler_APDEM_PTG_Pos_Size_check(ptg_x, ptg_y, ptg_wid, ptg_hei, display_width, display_heigth) < 0) {
		rtd_pr_vpq_emerg("%s,check size pos fail\n", __func__);
		return -1;
	}

	return 0;
}

char Scaler_APDEM_PTG_set(VIP_APDEM_PTG_CTRL *PTG_CTRL)
{
	#define VIP_D_Y2R_Offset_Max 16384
	//extern unsigned char rtk_hal_vsc_SetWinBlank(VIDEO_WID_T wid, unsigned char bonoff, KADP_VIDEO_DDI_WIN_COLOR_T color);
	extern void drvif_scaler_set_Pattern_mute(unsigned char mute_flag, unsigned char display_flag,StructSrcRect* sdp_info);
	unsigned char mute_flag; 
	unsigned char dis_flag;
	StructSrcRect windowPos_info;
	//_system_setting_info *VIP_system_info_structure_table = NULL;
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t = {0};

	//VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	
	if (Scaler_APDEM_PTG_check(PTG_CTRL) < 0) {
		rtd_pr_vpq_emerg("%s,check fail\n", __func__);
		return -1;
	}
#if 0
	if (VIP_system_info_structure_table == NULL) {
		rtd_pr_vpq_emerg("%s,VIP_system_info_structure_table NULL\n", __func__);
		return -1;
	}
#endif
	mute_flag = PTG_CTRL->mute_flag;
	dis_flag = PTG_CTRL->dis_flag;

	rtd_pr_vpq_info("%s,PTG_R_val=%d,PTG_G_val=%d,PTG_B_val=%d,PTG_Pos_Mode=%d,PTG_Size=%d,x_sta=%d,y_sta=%d,x_end=%d,y_end=%d,mute_flag=%d,dis_flag=%d,\n", 
		__func__, PTG_CTRL->PTG_R_val, PTG_CTRL->PTG_G_val, PTG_CTRL->PTG_B_val, PTG_CTRL->PTG_Pos_Mode, PTG_CTRL->PTG_Size, 
		PTG_CTRL->x_sta, PTG_CTRL->y_sta, PTG_CTRL->x_end, PTG_CTRL->y_end, mute_flag, dis_flag);

	if (Scaler_APDEM_PTG_Size_Pos_Cal(PTG_CTRL, &windowPos_info) < 0) {
		rtd_pr_vpq_emerg("%s, Pos Size error\n", __func__);
		return -1;
	}

	rtd_pr_vpq_info("%s,.srcx=%d, .srcy=%d, .src_wid=%d, .src_height=%d,\n", 
		__func__, windowPos_info.srcx, windowPos_info.srcy, windowPos_info.src_wid, windowPos_info.src_height);

	//rtk_hal_vsc_SetWinBlank(VIDEO_WID_0, mute_flag, KADP_VIDEO_DDI_WIN_COLOR_BLACK);
	drvif_scaler_set_Pattern_mute(mute_flag, dis_flag, &windowPos_info);

	// set pattern with d-y2r
	//VIP_system_info_structure_table->OSD_Info.Contrast_Gain = 0;
	drv_vipCSMatrix_t.RGB_Offset.R_offset = ((PTG_CTRL->PTG_R_val<<2)>VIP_D_Y2R_Offset_Max)?(VIP_D_Y2R_Offset_Max):(PTG_CTRL->PTG_R_val<<2);
	drv_vipCSMatrix_t.RGB_Offset.G_offset = ((PTG_CTRL->PTG_G_val<<2)>VIP_D_Y2R_Offset_Max)?(VIP_D_Y2R_Offset_Max):(PTG_CTRL->PTG_G_val<<2);
	drv_vipCSMatrix_t.RGB_Offset.B_offset = ((PTG_CTRL->PTG_B_val<<2)>VIP_D_Y2R_Offset_Max)?(VIP_D_Y2R_Offset_Max):(PTG_CTRL->PTG_B_val<<2);
	
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);

	return 0;
}

#endif //UT_flag
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

#ifndef UT_flag
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

#endif //UT_flag

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
#ifndef BUILD_QUICK_SHOW
	if (Get_PANEL_3D_DISPLAY_SUPPORT_ENABLE() && (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_SIDE_BY_SIDE_HALF)
		&& (Get_PANEL_3D_LINE_ALTERNATIVE_SUPPORT_ENABLE() || (drvif_scaler3d_decide_3d_SG_Disable_IDMA(SLR_3D_SIDE_BY_SIDE_HALF) == TRUE))) {
		temp_width = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_MEM_ACT_WID) << 1;
	} 
	else 
#endif
	{
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
	if (Scaler_InputSrcGetMainChType() == _SRC_VO && get_vsc_run_adaptive_stream() == 1)
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

	//SE_FilmDtect_Init();

	//NN_SE_Init(); // move to "boot init"

}

#ifndef UT_flag
void Scaler_VIP_PQ_QS_init(void)
{
	unsigned char source=255;
	fwif_VIP_set_Project_ID(VIP_Project_ID_TV001);
	source=fwif_vip_source_check(0,3);
	Scaler_VIP_IP_init(source);
	fwif_color_set_DCC_Init(0);
	Scaler_SetDCC_Table(3);
	Scaler_Set_DCC_Color_Independent_Table(3);
	Scaler_Set_DCC_chroma_compensation_Table(3);
	Scaler_SetDCC_Mode(3);
	Scaler_SetDNR(4);
	fwif_color_SetAutoMAFlag(1);
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
	unsigned char InputSrcGetFrom;
	unsigned short mode, channel;
	SCALER_DISP_CHANNEL display;

	down(&VPQ_I_CSC_Semaphore);
	mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	channel = Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL);
	en_422to444 = (unsigned char)Scaler_DispGetStatus(display, SLR_DISP_422);
	InputSrcGetType = Scaler_InputSrcGetType(display);

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
#ifndef BUILD_QUICK_SHOW
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	if (Get_DisplayMode_Src(display) != VSC_INPUTSRC_AVD) {
		rtd_printk(KERN_INFO, TAG_NAME, "Warning!! It's not vdc Src !!");
		return;
	}

	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->VD_ConBriHueSat_Table = value;

	fwif_color_module_vdc_SetConBriHueSat(src_idx, value);
#endif
}
unsigned char Scaler_module_vdc_GetConBriHueSat_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;

	if (Get_DisplayMode_Src(display) != VSC_INPUTSRC_AVD) {
		rtd_printk(KERN_INFO, TAG_NAME, "Warning!! It's not vdc Src !!");
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

#endif //UT_flag

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

#ifndef UT_flag

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
	fwif_color_set_LD_Global_Ctrl(src_idx, TableIdx);

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

	fwif_color_set_LC_CDLC_script_for_demo();// only for demo, remove it after driver ready

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
	//rtd_printk(KERN_INFO, TAG_NAME, "%s table_idx = %d\n", __FUNCTION__, table_idx);
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

void Scaler_Set_ColorMapping_By_ColorFormat(unsigned char channel, unsigned char nSD_HD)
{
	unsigned char colorFMT_info = COLORSPACE_MAX;
	unsigned char vsc_input_src = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);

	if (channel != SLR_MAIN_DISPLAY)
		return;

	if (Scaler_access_Project_TV002_Style(0, 0) == Project_TV002_Style_1 || fwif_VIP_get_Project_ID() == VIP_Project_ID_TV030)
		return;

	if (vsc_input_src == VSC_INPUTSRC_HDMI) {
		HDMI_AVI_T pAviInfo = {0};

		drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T *) (&pAviInfo));

		if ((pAviInfo.EC == 5 || pAviInfo.EC == 6) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED)) {
			colorFMT_info = isBT2020;
		} else if (((pAviInfo.EC == 0 || pAviInfo.EC == 2 || pAviInfo.EC == 3) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED))) {
			colorFMT_info = is601;
		} else if (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_SMPTE170) {
			colorFMT_info = is601;
		} else if ((pAviInfo.EC == 1 && pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED) || (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_ITU709)) {
			colorFMT_info = is709;
		} else {
			if (nSD_HD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}
	} else if (vsc_input_src == VSC_INPUTSRC_VDEC || vsc_input_src == VSC_INPUTSRC_CAMERA) {
		SLR_VOINFO* pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());

		if (pVOInfo == NULL) {
			rtd_printk(KERN_EMERG, TAG_NAME, "~get NULL warning return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
			return;
		}

		if (pVOInfo->colour_primaries == 1) {
			colorFMT_info = is709;
		} else if (pVOInfo->colour_primaries == 6 || pVOInfo->colour_primaries == 7) {
			colorFMT_info = is601;
		} else if (pVOInfo->colour_primaries == 9) {
			colorFMT_info = isBT2020;
		} else {
			if (nSD_HD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}
		}
	} else {
		if (nSD_HD == 0) {
			colorFMT_info = is601;
		} else {
			colorFMT_info = is709;
		}
	}

	//rtd_printk(KERN_INFO, TAG_NAME, "%s vsc_input_src = %d, g_WCG_mode = %d\n", __FUNCTION__, vsc_input_src, g_WCG_mode);

	if (g_WCG_mode == 0) {								/* WCG mode */
		Scaler_Set_ColorMapping_3x3_Table(0); //Liner
	} else if (g_WCG_mode == 2) { 						/* auto mode ==> set 601 */
			Scaler_Set_ColorMapping_3x3_Table(1); //BT601
	} else if (g_WCG_mode == 1) { 						/* standard mode ==> by source */
		if (colorFMT_info == is601)
			Scaler_Set_ColorMapping_3x3_Table(1); //BT601
		else if (colorFMT_info == is709)
			Scaler_Set_ColorMapping_3x3_Table(2); //BT709
		else if (colorFMT_info == isBT2020)
			Scaler_Set_ColorMapping_3x3_Table(3); //BT2020
		else
			Scaler_Set_ColorMapping_3x3_Table(0); //Liner
	} else {											/* unknow mode */
		Scaler_Set_ColorMapping_3x3_Table(0); //Liner
	}
}

void Scaler_Set_WCG_mode(unsigned int wcgmode)
{
	extern unsigned char g_nWCG_SD_HD_mode;

	g_WCG_mode = wcgmode;
	if (Scaler_APDEM_Arg_Access(DEM_ARG_Color_Space, 0, 0) == 0xFF)
		Scaler_Set_ColorMapping_By_ColorFormat(SLR_MAIN_DISPLAY, g_nWCG_SD_HD_mode);
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

unsigned char VPQ_ALL_Linear_byCMD_Flag = 0;
unsigned char Scaler_apply_PQ_Linear(unsigned char Enable)
{
#ifndef BUILD_QUICK_SHOW
	if (Enable == 1) {
		vpq_ioctl_set_stop_run_by_idx(VPQ_IOC_PQ_CMD_INIT,1);
		vpq_extern_ioctl_set_stop_run_by_idx(VPQ_EXTERN_IOC_PQ_CMD_INIT,1);
		vpq_memc_ioctl_set_stop_run_by_idx(0, 1);
		Scaler_color_Set_PQ_ByPass_Lv(4);
		Scaler_SetContrast(50);
		Scaler_SetBrightness(50);
		Scaler_SetSaturation(50);
		Scaler_SetHue(50);
	} else {
		vpq_ioctl_set_stop_run_by_idx(VPQ_IOC_PQ_CMD_INIT,0);
		vpq_extern_ioctl_set_stop_run_by_idx(VPQ_EXTERN_IOC_PQ_CMD_INIT,0);
		vpq_memc_ioctl_set_stop_run_by_idx(0, 0);
	}
#endif
	return 0;
}

void Scaler_Set_PQ_RGB444MODE(unsigned char path)
{
#if 1
    extern DRV_FIR_Coef FIR_Coef_Ctrl[DRV_FIR_Coef_4Tap_NUM];
    extern unsigned char sharpness_UI;
	extern void drvif_color_scaler_set_scaleup_coefRGB_0(void);
	extern void drvif_color_scaler_set_scaledown_coefRGB_0(void);

    if(path==1){
        rtd_pr_vpq_info("Scaler_Set_PQ_RGB444MODE\n");
        drvif_color_set_LC_RGB_mode(1);
        drvif_color_set_Icm_RGB_mode(1);
        drvif_color_scaler_set_scaleup_coefRGB_0();
		drvif_color_scaler_set_scaledown_coefRGB_0();
    }else{
        rtd_pr_vpq_info("NOT !!!Scaler_Set_PQ_RGB444MODE\n");
        drvif_color_set_LC_RGB_mode(0);
        drvif_color_set_Icm_RGB_mode(0);
        VPQ_RgbShp_Lib_SetRgb_shp_PqLevel(0);
    }
#endif	
}

unsigned char Scaler_VPQ_check_PC_RGB444(void)
{
#if 0
	unsigned char ret_val;
	ret_val = VIP_Disable_PC_RGB444;
#else	// _APDEM_SYNC_NOT_READY_RGB444 
	//extern unsigned char Last444BypassMode;
	unsigned char ret_val;
	if (Get_Val_vsc_run_pc_mode())
	{
		
		if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
		{
			rtd_pr_vpq_info("[for PcMode]pc_mode=%d,DisplayMode_Src=%d,drvif_Hdmi_GetColorSpace()=%d, dolby_tunnel()=%d,interlace=%d,\n", 
				Get_Val_vsc_run_pc_mode(),Get_DisplayMode_Src(SLR_MAIN_DISPLAY), drvif_Hdmi_GetColorSpace(), is_dolby_vision_tunnel_mode(), 
				Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE));
			if ((drvif_Hdmi_GetColorSpace() == COLOR_RGB) && (!is_dolby_vision_tunnel_mode()) && (!Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE))){
				ret_val = VIP_HDMI_PC_RGB444;
			}else{
				ret_val = VIP_HDMI_PC_YUV444;
			}
		}
		else if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP)
		{
#if IS_ENABLED(CONFIG_RTK_DPRX)
			rtd_pr_vpq_info("[for PcMode]pc_mode=%d,DisplayMode_Src=%d,drvif_Dprx_GetColorSpace()=%d, dolby_tunnel()=%d,interlace=%d,\n", 
				Get_Val_vsc_run_pc_mode(),Get_DisplayMode_Src(SLR_MAIN_DISPLAY), drvif_Dprx_GetColorSpace(), is_dolby_vision_tunnel_mode(), 
				Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE));
			if ((drvif_Dprx_GetColorSpace() == DP_COLOR_SPACE_RGB) && (!Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE))	 && (!is_dolby_vision_tunnel_mode())){
				ret_val = VIP_DP_PC_RGB444;
			}else{
				ret_val = VIP_DP_PC_YUV444;
			}
#else
			rtd_pr_vpq_info("[for PcMode]pc_mode=%d,DisplayMode_Src=%d,\n", 
				Get_Val_vsc_run_pc_mode(),Get_DisplayMode_Src(SLR_MAIN_DISPLAY));
			ret_val = VIP_Disable_PC_RGB444;
#endif
		} else {
			rtd_pr_vpq_info("[for PcMode]pc_mode=%d,DisplayMode_Src=%d,\n", 
				Get_Val_vsc_run_pc_mode(),Get_DisplayMode_Src(SLR_MAIN_DISPLAY));
			ret_val = VIP_Disable_PC_RGB444;
		}
	}
    else {
		rtd_pr_vpq_info("[for PcMode]pc_mode=%d,DisplayMode_Src=%d,\n", 
			Get_Val_vsc_run_pc_mode(),Get_DisplayMode_Src(SLR_MAIN_DISPLAY));		
		ret_val = VIP_Disable_PC_RGB444;
        //Scaler_Set_PQ_RGB444MODE(0);
    }
#endif

#ifdef VIP_FORCE_DISABLE_PC_RGB_YUV_444
	ret_val = VIP_Disable_PC_RGB444;
	rtd_pr_vpq_info("[for PcMode]force skip rgb444,\n");	
#endif

	return ret_val;
}

void Scaler_RefreshPictureMode(void)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned char src_idx = 0;
	unsigned char display = 0;
	unsigned char PC_mode_check;

	Scaler_Get_Display_info(&display, &src_idx);


	if (VIP_system_info_structure_table == NULL)
		return;

	if (display == SLR_SUB_DISPLAY)
		return;

	if (!VIP_system_info_structure_table->DolbyHDR_flag)
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);
	Scaler_SetSharpness(Scaler_GetSharpness());
	Scaler_SetDNR(Scaler_GetDNR());
	Scaler_SetMPEGNR(Scaler_GetMPEGNR(), CALLED_NOT_BY_OSD);
	Scaler_SetDCC_Mode(Scaler_GetDCC_Mode());
	Scaler_Set_CinemaMode_PQ(Scaler_Get_CinemaMode_PQ());

	if (Scaler_APDEM_Arg_Access(DEM_ARG_Blue_Stretch_level, 0, 0) != 0xFF)
		fwif_color_set_UV_Offset_byTBL_Level(0, SLR_MAIN_DISPLAY, Scaler_get_YUV2RGB(), 
			Scaler_APDEM_Arg_Access(DEM_ARG_Blue_Stretch_level, 0, 0), 0, 0);

	if(((get_hdmi_4k_hfr_mode() != HDMI_NON_4K120)||force_enable_two_step_uzu() || get_hdmi_vrr_4k60_mode()) && (Get_DISPLAY_REFRESH_RATE() >= 120))
	{
		// bypass PQ for 4k120
		fwif_color_set_PQ_ByPass_2P_mode_misc();
		Scaler_color_Set_PQ_ByPass_Lv(6);
	}

#if 0 //fixme, lose some API
	if (g_tv002_demo_fun_flag) {
		Scaler_set_sGamma(Scaler_get_sGamma());
		Scaler_set_BLE(Scaler_get_BLE());
		//Scaler_set_LiveColor(Scaler_get_LiveColor());
		fwif_color_set_clarity_resolution();
	}
#endif

	// for Dolby HDR
	if (VIP_system_info_structure_table->DolbyHDR_flag) {
		extern unsigned char g_pq_bypass_lv;

		Scaler_SetContrast(50);
		Scaler_SetBrightness(50);
		Scaler_SetSaturation(50);
		Scaler_SetHue(50);

		if (g_pq_bypass_lv == 9)
			Scaler_color_Set_PQ_ByPass_Lv(9);
	}
	
	// for PcMode
	PC_mode_check = Scaler_VPQ_check_PC_RGB444();
	if ((PC_mode_check == VIP_HDMI_PC_RGB444) || (PC_mode_check == VIP_DP_PC_RGB444)) {
		Scaler_color_Set_PQ_ByPass_Lv(5);
		Scaler_Set_PQ_RGB444MODE(1);
	} else if ((PC_mode_check == VIP_HDMI_PC_YUV444) || (PC_mode_check == VIP_DP_PC_YUV444)) {
		Scaler_color_Set_PQ_ByPass_Lv(4);
		Scaler_Set_PQ_RGB444MODE(0);
	} else {
		Scaler_Set_PQ_RGB444MODE(0);
	}
	rtd_pr_vpq_info("PC mode check = %d,\n", PC_mode_check);	

	if (VPQ_ALL_Linear_byCMD_Flag == 1) {
		Scaler_apply_PQ_Linear(1);
		rtd_pr_vpq_info("VPQ_ALL_Linear_byCMD_Flag = %d,\n", VPQ_ALL_Linear_byCMD_Flag);	
	}
		
	//else if (VIP_system_info_structure_table->HDR_flag)
	//	Sacler_color_Netflix_NTS_HDR10_patch();
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
	unsigned char PC_mode_check;
	
	Scaler_Get_Display_info(&display, &src_idx);

	rtd_pr_vpq_debug("Scaler_SetSharpness ==> %d, APDEM=%d,", value, Scaler_APDEM_Arg_Access(DEM_command_Sent_NUM, 0, 0));
	pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;
	pData->Sharpness = value;

	PC_mode_check = Scaler_VPQ_check_PC_RGB444();
	if(((get_hdmi_4k_hfr_mode() != HDMI_NON_4K120)||force_enable_two_step_uzu() || get_hdmi_vrr_4k60_mode()) && (Get_DISPLAY_REFRESH_RATE() >= 120))
		return;

	if (Scaler_APDEM_Arg_Access(DEM_command_Sent_NUM, 0, 0) == 0) {
		fwif_color_set_sharpness_level(Scaler_GetSharpnessTable(), value);
	} else {
		fwif_color_set_sharpness_level_Minumun_0(Scaler_GetSharpnessTable(), value);
		fwif_color_set_dlti_bySHPLevel(Scaler_GetDLti(), value);
		fwif_color_set_dcti_bySHPLevel(Scaler_GetDCti(), value);
		fwif_color_set_inewdcti_bySHPLevel(Scaler_GetIDCti(), value);
		fwif_color_set_MBPK_table_byLevel(Scaler_GetMBPeaking(), value);
	}

	if ((PC_mode_check == VIP_HDMI_PC_RGB444) || (PC_mode_check == VIP_DP_PC_RGB444))
		VPQ_RgbShp_Lib_SetRgb_shp_byGain(1,value,100);
	
}

unsigned char Scaler_Get_DCR_Backlight(void)
{
	_clues *smartPic_clue = fwif_color_Get_SmartPic_clue();
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

#ifndef BUILD_QUICK_SHOW
	// for RADCR
	{
		extern unsigned char RADCR_BL_ready;
		RADCR_BL_ready = true;
	}
#endif
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
	short hue = 0, sat = 0;
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
		drvif_color_setuv_sat(display, sat);/* 20100726 modify sat adj.  from sRGB to uv, csfanchiang*/
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

void Scale_SetHDR3DLUT_Offset(int offset)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned char HDR_flag;
	
	HDR_flag = VIP_system_info_structure_table->HDR_flag;
	HDR_3DLUT_offset = offset;
	Scaler_color_HDRIP_AutoRun(Scaler_color_HAL_VPQ_HDR_Type_to_HDR_DM_Type(HDR_flag));

	rtd_pr_vpq_info("Scale_SetHDR3DLUT_Offset, HDR_flag=%d, HDR_3DLUT_offset=%d\n", HDR_flag, HDR_3DLUT_offset);

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
    if (vip_source == VIP_QUALITY_HDMI_4k2k)
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
		if (!drvif_color_set_HDR10_Enable(enable))
		{
			rtd_printk(KERN_WARNING, TAG_NAME, "Scaler_set_HDR10_Enable NOT RUN!!\n");
			return;
		}
		drvif_color_set422to444(0, 0);
	}
	else
	{
		drvif_color_set_HDR10_Enable(enable);
	}
}

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

	if (fwif_color_get_force_run_i3ddma_enable(display)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &srcType, &SrcGetFrom);
		mode = fwif_color_get_cur_input_timing_mode(display);
	}

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

	rtd_pr_vpq_info("DM_SMR006,dis=%d,mode=%d,srcType=%d,SrcGetFrom=%d,isHD=%d,HDMI_CSC_DataRange_Mode=%d,IsRGB_YUV_RANGE=%d,\n",
		display, mode, srcType, SrcGetFrom, isHD, HDMI_CSC_DataRange_Mode, IsRGB_YUV_RANGE);

	rtd_pr_vpq_info("DM_SMR006,video_fmt=%d,isHDMI=%d,.EC=%d,.C=%d,colorFMT_info=%d,data_range=%d,colour_primaries=%d,ret_idx=%d,blk_lv=%d\n",
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
	if (fwif_color_get_force_run_i3ddma_enable(display)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &srcType, &SrcGetFrom);
		mode = fwif_color_get_cur_input_timing_mode(display);
	}

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

	rtd_pr_vpq_info("DM_SMR, dis=%d,mode=%d,srcType=%d,SrcGetFrom=%d,isHD=%d,HDMI_CSC_DataRange_Mode=%d,IsRGB_YUV_RANGE=%d,\n",
		display, mode, srcType, SrcGetFrom, isHD, HDMI_CSC_DataRange_Mode, IsRGB_YUV_RANGE);

	rtd_pr_vpq_info("DM_SMR, video_format=%d,isHDMI=%d,.EC=%d,.C=%d,colorFMT_info=%d,data_range=%d,colour_primaries=%d,ret_idx=%d,\n",
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
	} else if (srcType == VSC_INPUTSRC_VDEC || srcType == VSC_INPUTSRC_CAMERA) {
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

	rtd_pr_vpq_info("CSC1, display=%d,srcType=%d,color_fmt=%d,video_fmt=%d,FullRange=%d,ret=%d\n",
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
	unsigned char twopixel_mode = 0, hdmi_input = 0, hdmi_color_format = COLOR_UNKNOW;

	if (srcType == VSC_INPUTSRC_HDMI) {
		HDMI_CSC_DATA_RANGE_MODE_T HDMI_CSC_DataRange_Mode = Scaler_GetHDMI_CSC_DataRange_Mode();
		HDMI_RGB_YUV_RANGE_MODE_T IsRGB_YUV_RANGE = drvif_IsRGB_YUV_RANGE();
		if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_AUTO) {
			if (IsRGB_YUV_RANGE==MODE_RAG_FULL)
				isFullRange = 1;
		} else if (HDMI_CSC_DataRange_Mode == HDMI_CSC_DATA_RANGE_MODE_FULL) {
			isFullRange = 1;
		}

		if (display == SLR_MAIN_DISPLAY) {
			srcType = Scaler_InputSrcGetType(display);
			get_vtop_input_info(&twopixel_mode, &hdmi_input, &hdmi_color_format);
		}

		video_format = drvif_Hdmi_GetColorSpace();
	} else if (srcType == VSC_INPUTSRC_VDEC || srcType == VSC_INPUTSRC_CAMERA) {
		isFullRange = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane())->video_full_range_flag?1:0;
	}

	if ((srcType == _SRC_HDMI) && (hdmi_input == 1) && (hdmi_color_format == COLOR_RGB)) {
		if (isFullRange == 1)
			ret_idx = HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_255_to_255;
		else
			ret_idx = HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_Limted_235_to_255;
	} else if (color_format == HAL_VPQ_COLORIMETRY_BT709) {
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

	rtd_pr_vpq_info("CSC1, display=%d,srcType=%d,color_fmt=%d,video_fmt=%d,FullRange=%d,ret=%d\n",
		display, srcType, color_format, video_format, isFullRange,  ret_idx);

	return ret_idx;

}

unsigned char Scaler_get_APDEM_HDR_YUV2RGB(unsigned char HDR_mode)
{
	unsigned char ret_idx=0;
	unsigned char isHD=0, SrcGetFrom=0;
	unsigned char display = (unsigned char)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	unsigned char video_format = COLOR_UNKNOW;
	UINT8 data_range = CSC_DATA_RANGE_MODE_AUTO;
	SLR_VOINFO* pVOInfo = NULL;
	HDMI_AVI_T pAviInfo = {0};
	unsigned char colorFMT_info = COLORSPACE_MAX;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned char IsRGB_YUV_RANGE = 255;
	unsigned char srcType = Scaler_InputSrcGetType(display);
	unsigned short mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	unsigned char isHDMI=0;
	unsigned char twopixel_mode = 0, hdmi_input = 0, color_format = 0;
	unsigned char isRGB_in = 0;
	unsigned char PCRGB444;
	unsigned char isRGBin_RGBout;
	unsigned char isDP_src, DP_video_format, DP_colorFMT_info = is709, DP_data_range = CSC_DATA_RANGE_MODE_FULL;
	//unsigned char bUse_HDMI_Range_Detect = 1;
	//unsigned char isHDR_Signal = 0;
	if (fwif_color_get_force_run_i3ddma_enable(display)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &srcType, &SrcGetFrom);
		mode = fwif_color_get_cur_input_timing_mode(display);
	}

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T *) (&pAviInfo));
	get_vtop_input_info(&twopixel_mode, &hdmi_input, &color_format);
	if(VIP_system_info_structure_table == NULL || pVOInfo == NULL){
		rtd_pr_vpq_emerg("~get NULL warning return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	if ((mode !=  _MODE_480I) && (mode !=  _MODE_480P) && (mode !=  _MODE_576I) && (mode !=  _MODE_576P))
		isHD = 1;

	PCRGB444 = 	Scaler_VPQ_check_PC_RGB444();

#if IS_ENABLED(CONFIG_RTK_DPRX)
	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP) {	// get DP info
		DP_PIXEL_ENCODING_FORMAT_T dp_info;
		if (drvif_Dprx_GetPixelEncodingFormat(&dp_info)) {
			rtd_pr_vpq_info("drvif_Dprx_GetPixelEncodingFormat, color_space = %d, colorimetry = %d, dynamic_range = %d\n", 
				dp_info.color_space, dp_info.colorimetry, dp_info.dynamic_range);

			switch(dp_info.colorimetry)
			{
				case DP_COLORIMETRY_LEGACY_RGB:
				case DP_COLORIMETRY_SRGB:
				case DP_COLORIMETRY_BT_709:
				case DP_COLORIMETRY_XV_YCC_709:
					DP_colorFMT_info = is709;
					break;
				case DP_COLORIMETRY_BT_601:
				case DP_COLORIMETRY_XV_YCC_601:
				case DP_COLORIMETRY_S_YCC_601:
				case DP_COLORIMETRY_OP_YCC_601:
					DP_colorFMT_info = is601;
					break;
				case DP_COLORIMETRY_BT_2020_RGB:
				case DP_COLORIMETRY_BT_2020_YCCBCCRC:
				case DP_COLORIMETRY_BT_2020_YCBCR:
					DP_colorFMT_info = isBT2020;
					break;
				//case DP_COLORIMETRY_XRGB:
				//case DP_COLORIMETRY_SCRGB:
				//case DP_COLORIMETRY_ADOBE_RGB:
				//case DP_COLORIMETRY_DCI_P3:
				//case DP_COLORIMETRY_CUSTOM_COLOR_PROFILE:
				//case DP_COLORIMETRY_DICOM_PS314:
				//case DP_COLORIMETRY_YONLY:
				//case DP_COLORIMETRY_RAW:
				//case DP_COLORIMETRY_UNKNOWN:
				default:
					DP_colorFMT_info = is709;
					break;
			}
			if (Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0) == CSC_DATA_RANGE_MODE_AUTO) {
				DP_data_range = (dp_info.dynamic_range == DP_DYNAMIC_RANGE_VESA) ? CSC_DATA_RANGE_MODE_FULL : CSC_DATA_RANGE_MODE_LIMITED;
			} else {
				DP_data_range = Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0);
			}
		}
		isDP_src = 1;
		DP_video_format = drvif_Dprx_GetColorSpace();
	
		rtd_pr_vpq_info("[CSC1] APDEM_CSC1, DP_info, color_space=%d, colorimetry=%d, dynamic_range=%d, DP_colorFMT_info=%d, DP_data_range=%d, isDP_src=%d, DP_video_format=%d\n", 
			dp_info.color_space, dp_info.colorimetry, dp_info.dynamic_range, DP_colorFMT_info, DP_data_range, isDP_src, DP_video_format);
	}else{
		isDP_src = 0;
		DP_video_format = 0;
		
		rtd_pr_vpq_info("[CSC1] APDEM_CSC1, isDP_src=%d, DP_video_format=%d\n", isDP_src, DP_video_format);
	}
#else
		isDP_src = 0xFF;
		DP_video_format = 0xFF;
#endif

	isRGBin_RGBout = 0;
	if ((PCRGB444 == VIP_HDMI_PC_RGB444) || (PCRGB444 == VIP_DP_PC_RGB444)) {
		colorFMT_info = isBT2020;
		isRGB_in = 0;
		data_range = CSC_DATA_RANGE_MODE_FULL;
		isRGBin_RGBout = 1;
	} else{
		if (HDR_mode == HAL_VPQ_HDR_MODE_ST2094){
			colorFMT_info = is709;
			if (hdmi_input == 1 && color_format == COLOR_RGB) {
				isRGB_in = 1;	// data format will be GBR from hdmi input
			}
			if (Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0) == CSC_DATA_RANGE_MODE_FULL)
				data_range = CSC_DATA_RANGE_MODE_FULL;
			else
				data_range = CSC_DATA_RANGE_MODE_LIMITED;
		} else if (srcType == _SRC_HDMI) {
			if (isDP_src == 1) {
				if (hdmi_input == 1 && DP_video_format == DP_COLOR_SPACE_RGB) {
					isRGB_in = 1;
				} else {
					isRGB_in = 0;
				}
				data_range = DP_data_range;
				colorFMT_info = DP_colorFMT_info;
			} else{
				IsRGB_YUV_RANGE = drvif_IsRGB_YUV_RANGE();
				video_format = drvif_Hdmi_GetColorSpace();
				isHDMI = drvif_IsHDMI();

				if ((pAviInfo.EC == 5 || pAviInfo.EC == 6) && (pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_EXTENDED)) {
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

				if (hdmi_input == 1 && color_format == COLOR_RGB) {
					isRGB_in = 1;	// data format will be GBR from hdmi input
				} 

				if (Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0) == CSC_DATA_RANGE_MODE_AUTO) {
					if (IsRGB_YUV_RANGE==MODE_RAG_FULL) {
						data_range = CSC_DATA_RANGE_MODE_FULL;
					} else if (IsRGB_YUV_RANGE==MODE_RAG_LIMIT) {
						data_range = CSC_DATA_RANGE_MODE_LIMITED;
					} else {
						if (video_format !=  0) {/*YUV444 or YUV422*/
							data_range = CSC_DATA_RANGE_MODE_LIMITED;
						} else {
							if (isHDMI)
								data_range = CSC_DATA_RANGE_MODE_LIMITED;
							else
								data_range = CSC_DATA_RANGE_MODE_FULL;
						}
					}
				} else {
					data_range = Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0);
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

			isRGB_in = 0;

			if (Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0) == CSC_DATA_RANGE_MODE_FULL)
				data_range = CSC_DATA_RANGE_MODE_FULL;
			else
				data_range = CSC_DATA_RANGE_MODE_LIMITED;

		} else if (srcType == _SRC_VGA) {
			if (isHD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}

			isRGB_in = 0;
			
			if (Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0) == CSC_DATA_RANGE_MODE_LIMITED)
				data_range = CSC_DATA_RANGE_MODE_LIMITED;
			else
				data_range = CSC_DATA_RANGE_MODE_FULL;

		} else {
			if (isHD == 0) {
				colorFMT_info = is601;
			} else {
				colorFMT_info = is709;
			}

			isRGB_in = 0;
			
			if (Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0) == CSC_DATA_RANGE_MODE_FULL)
				data_range = CSC_DATA_RANGE_MODE_FULL;
			else
				data_range = CSC_DATA_RANGE_MODE_LIMITED;
		}
	}

	// get index, should be RGB full in DM
	if (isRGBin_RGBout == 1) {
		ret_idx = HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_255_to_255;
	} else{
		if (data_range == CSC_DATA_RANGE_MODE_FULL) {
			if (isRGB_in == 1) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_255_to_255;	// data format will be GBR from hdmi input
			} else if (colorFMT_info == is601) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_255_to_255;
			} else if (colorFMT_info == is709) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_255_to_255;
			} else {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_255_to_255;
			}
		} else {
			if (isRGB_in == 1) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_Limted_235_to_255;	// data format will be GBR from hdmi input
			} else if (colorFMT_info == is601) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT601_Limted_235_240_to_255;
			} else if (colorFMT_info == is709) {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT709_Limted_235_240_to_255;
			} else {
				ret_idx = HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255;
			}
		}
	}

	rtd_pr_vpq_info("[CSC1] APDEM_CSC1, HDR_mode=%d, PCRGB444=%d, isRGBin_RGBout=%d, hdmi_input=%d, color_format=%d, dis=%d, mode=%d, srcType=%d, SrcGetFrom=%d, isHD=%d, data_range=%d, IsRGB_YUV_RANGE=%d, isRGB_in=%d,APDEM_lv=%d,\n",
		HDR_mode, PCRGB444, isRGBin_RGBout, hdmi_input, color_format, display, mode, srcType, SrcGetFrom, isHD, data_range, IsRGB_YUV_RANGE, isRGB_in, Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0));

	rtd_pr_vpq_info("[CSC1] APDEM_CSC1, video_format=%d,isHDMI=%d,.EC=%d,.C=%d,colorFMT_info=%d,data_range=%d,colour_primaries=%d,ret_idx=%d,\n",
		video_format, isHDMI, pAviInfo.EC, pAviInfo.C, colorFMT_info, data_range, pVOInfo->colour_primaries, ret_idx);

	return ret_idx;
}

void Scaler_Set_HDR_YUV2RGB(unsigned char HDR_mode, unsigned char color_format)
{
#ifdef CONFIG_SUPPORT_IPQ
	unsigned char index_ret = 0;
	index_ret = fwif_color_get_HDR_YUV2RGB_SDR_MAX_RGB_by_HFC_TV043(color_format);
	printk("HDR_YUV2RGB,index=%d,color_format=%d\n",index_ret,color_format);
	drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[index_ret]);
#else

	RGB2YUV_BLACK_LEVEL blk_lv;
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "blk_lv=%d, videoFormat=%d, HDR mode = %d\n", blk_lv, video_format, get_HDR_mode());*/
	unsigned char index = VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max;	
	unsigned char twopixel_mode = 0, hdmi_input = 0, vtop_color_format = COLOR_RGB;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(VIP_system_info_structure_table == NULL){
		rtd_pr_vpq_emerg("[%s:%d] error here!! system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	get_vtop_input_info( &twopixel_mode, &hdmi_input, &vtop_color_format);

	g_HDR_mode = HDR_mode;
	g_HDR_color_format = color_format;

	blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();
	/*rtd_printk(KERN_DEBUG, TAG_NAME, "[HDR10],videoFMT=%d,hdr_mode=%d\n", video_format, get_HDR_mode());*/
	if (Scaler_APDEM_Arg_Access(DEM_ARG_Data_Range, 0, 0) != 0xFF && (VIP_system_info_structure_table->DolbyHDR_flag!=1))
	{
		index = Scaler_get_APDEM_HDR_YUV2RGB(HDR_mode);
		drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[index]);
	}	
	else if (HDR_mode == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {
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
#if 0	
	else if (HDR_mode == HAL_VPQ_HDR_MODE_HDR10_VIVID)
	{
		if(blk_lv == RGB2YUV_BLACK_LEVEL_HIGH) {	/* use limit2limit*/
			if (hdmi_input == 1 && vtop_color_format == COLOR_RGB) {
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_255_to_255]);
			} else if (color_format == HAL_VPQ_COLORIMETRY_BT709) {/* bt 709 */
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_HDR10_VIVID_235_to_255]);
			} else { /* bt 2020*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_HDR10_VIVID_235_to_255]);
			}
		} else { /* use limit2full*/
			if (hdmi_input == 1 && vtop_color_format == COLOR_RGB) {
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_ByPass_GBRtoRGB_Limted_235_to_255]);
			} else if (color_format == HAL_VPQ_COLORIMETRY_BT709) { /* bt 709 */
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_HDR10_VIVID_235_to_255]);
			} else { /* bt 2020*/
				drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_HDR10_VIVID_235_to_255]);
			}
		}
	}
#endif	
	else
	{
		//SDR
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


void Scaler_LGE_HDR10_Enable_newflow(unsigned char enable)
{
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;

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
		drvif_color_set_HDR10_Enable(hdr_table[1]);	//enable process
		fwif_color_set_Main_Dither(TRUE,VIP_DITHER_MAINTABLE_HDR10_12to10);
	}
	else
	{
		drvif_color_set_HDR10_Enable(hdr_table[0]);	//disable process
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
	Scaler_LGE_HDR10_Enable_newflow(0);
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

	rtd_pr_vpq_info("drvif_TV006_SET_HDR_mode, modeinfo = %d, Colorimetry info = %d\n", modeinfo[0], modeinfo[1]);

	//SET_HDR_mode
	switch(modeinfo[0])	//HDR mode
	{
	case HAL_VPQ_HDR_MODE_HDR10:
		//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		//Scaler_LGE_HDR10_Enable(modeinfo[1]);
		Scaler_Set_HDR_YUV2RGB(modeinfo[0], modeinfo[1]);

	break;
	case HAL_VPQ_HDR_MODE_HLG:
		//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		//Scaler_LGE_HDR10_Enable(modeinfo[1]);
		Scaler_Set_HDR_YUV2RGB(modeinfo[0], modeinfo[1]);

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
#ifdef support_hdr10_vivi_mode
			else if (vdec_cur_hdr_type == HDR_DM_HDR10_VIVID)
			{
				if (drvif_Get_HDR10_vivid_EFuse_Status() == 1)
					PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_HDR10;
				else
					PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_HDR10_VIVID;//nned PQ change
				PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT2020;
				hdrType = HDR_DM_SDR_MAX_RGB;
			}
#endif
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
	Scaler_Set_HDR_YUV2RGB(VIP_system_info_structure_table->HDR_flag, HAL_VPQ_COLORIMETRY_BT2020);
	drvif_color_HDR10_DM_setting();
//	Scaler_SetDataFormatHandler();
	Scaler_SetDataFormatHandle_Seamless();

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
#ifdef support_hdr10_vivi_mode
		else if (vdec_cur_hdr_type == HDR_DM_HDR10_VIVID)
		{
			if (drvif_Get_HDR10_vivid_EFuse_Status() == 1)
				PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_HDR10;
			else
				PQModeInfo_flag_seamless[0] = HAL_VPQ_HDR_MODE_HDR10_VIVID;//nned PQ change
			PQModeInfo_flag_seamless[1] = HAL_VPQ_COLORIMETRY_BT2020;
			hdrType = HDR_DM_SDR_MAX_RGB;
		}
#endif		
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
	Scaler_Set_HDR_YUV2RGB(VIP_system_info_structure_table->HDR_flag, HAL_VPQ_COLORIMETRY_BT2020);
	drvif_color_HDR10_DM_setting();

	if(Scaler_color_Get_HDR_AutoRun())
		Scaler_color_Access_HDR_AutoRun_TBL_Flag(1, &hdrType);

}

void Scaler_hdr_setting_SEAMLESS_hdmi_finish(unsigned char hdr_enable, unsigned char hdr_type)
{
	//fwif_color_SetDataFormatHandler_tv006_Seamless(hdr_enable);
	drvif_TV006_SET_HDR_mode_Seamless_hdmi_finish(PQModeInfo_flag_seamless);
	Scaler_SetDataFormatHandle_Seamless();
}


void drvif_TV006_SET_HDR_mode_Seamless(UINT32* modeinfo)
{
	//_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	extern unsigned char g_HDR3DLUTForceWrite;
	rtd_pr_vpq_info("[HDR new flow][seamless], type = %d\n", modeinfo[0]);

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
		drvif_color_set_BBC_Enable(1);
		drvif_DM2_EOTF_Enable(1, 0);
		drvif_DM2_OETF_Enable(1, 0);
		drvif_DM2_GAMMA_Enable(1);
		drvif_DM2_b0501_enable_Enable(1);
		Scaler_LGE_HDR10_Enable_Seamless(modeinfo[1]);
		fwif_color_set_HDR10_runmode();
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);

		fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
		Scaler_color_HFC_set_TC_FrameSync_Enable(0);
		fwif_color_set_ST2094_FrameSync_Enable(0);

	break;
	case HAL_VPQ_HDR_MODE_HLG:
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		drvif_color_set_BBC_Enable(1);
		drvif_DM2_EOTF_Enable(1, 1);
		drvif_DM2_OETF_Enable(1, 1);
		drvif_DM2_GAMMA_Enable(1);
		drvif_DM2_b0501_enable_Enable(0);
		Scaler_LGE_HDR10_Enable_Seamless(modeinfo[1]);
		fwif_color_set_HDR10_runmode();
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);

		fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
		Scaler_color_HFC_set_TC_FrameSync_Enable(0);
		fwif_color_set_ST2094_FrameSync_Enable(0);

		//drvif_DM2_OETF_Set((unsigned short *)&OETF_LUT_HLG_DEFAULT_DCC_OFF, (unsigned short *)&OETF_LUT_HLG_DEFAULT_DCC_OFF, (unsigned short *)&OETF_LUT_HLG_DEFAULT_DCC_OFF);

	break;

	case HAL_VPQ_HDR_MODE_SDR_MAX_RGB:
			VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
			drvif_color_set_BBC_Enable(0);
			drvif_DM2_OETF_Enable(0, 0);
			drvif_DM2_EOTF_Enable(0, 0);
			drvif_DM2_GAMMA_Enable(0);
			drvif_DM2_b0501_enable_Enable(0);
			Scaler_LGE_HDR10_Enable_Seamless(modeinfo[1]);
			fwif_color_set_HDR10_runmode();
			//drvif_color_set_BBC_shift_bit(0);
			//drvif_color_Set_ST2094_3Dlut_CInv(1024, 1);
			fwif_color_TC_Flow(0);
			drvif_DM2_OETF_EOTF_Path_Sel(1);
			fwif_color_set_DM2_Hist_AutoMode_Enable(1);

			fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
			Scaler_color_HFC_set_TC_FrameSync_Enable(0);
			fwif_color_set_ST2094_FrameSync_Enable(0);
			
	break;
	case HAL_VPQ_HDR_MODE_HDR10_VIVID:
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		Scaler_LGE_HDR10_Enable_Seamless(modeinfo[1]);
		fwif_color_set_HDR10_runmode();
		drvif_color_set_BBC_Enable(0);
		//drvif_DM2_GAMMA_Enable(0);		// set by hal or HFC
		//drvif_color_set_BBC_shift_bit(0);
		//drvif_color_Set_HDR10plus_3Dlut_CInv(1024, 1);
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);

		fwif_color_set_HDR_VIVID_FrameSync_Enable(1);
		Scaler_color_HFC_set_TC_FrameSync_Enable(0);
		fwif_color_set_ST2094_FrameSync_Enable(0);

	break;
	case HAL_VPQ_HDR_MODE_SDR:
	case HAL_VPQ_HDR_MODE_HDREFFECT:
		//HDR10 and HLG off
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 0;
		drvif_color_set_BBC_Enable(0);
		drvif_DM2_OETF_Enable(0, 0);
		drvif_DM2_EOTF_Enable(0, 0);
		drvif_DM2_GAMMA_Enable(0);
		drvif_DM2_b0501_enable_Enable(0);
		g_HDR3DLUTForceWrite = 1;
		/*drvif_TV006_SET_HDR_mode_off();*/
		/*drvif_color_set_BBC_Enable(0);*/
		Scaler_LGE_HDR10_Enable_Seamless(0);
		fwif_color_set_DM2_Hist_AutoMode_Enable(0);
		fwif_color_TC_Flow(0);
		
		fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
		Scaler_color_HFC_set_TC_FrameSync_Enable(0);
		fwif_color_set_ST2094_FrameSync_Enable(0);
		
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
		fwif_color_set_HDR10_runmode();
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);

		drvif_color_set_BBC_Enable(1);

		fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
		Scaler_color_HFC_set_TC_FrameSync_Enable(0);
		fwif_color_set_ST2094_FrameSync_Enable(0);

	break;
	case HAL_VPQ_HDR_MODE_HLG:
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(modeinfo[1]);
		drvif_color_set_BBC_Enable(1);
		//drvif_DM2_GAMMA_Enable(g_InvGamma);		// set by hal or HFC
		fwif_color_set_HDR10_runmode();
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);

		fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
		Scaler_color_HFC_set_TC_FrameSync_Enable(0);
		fwif_color_set_ST2094_FrameSync_Enable(0);

	break;
	case HAL_VPQ_HDR_MODE_SDR:
	case HAL_VPQ_HDR_MODE_HDREFFECT:
		//HDR10 and HLG off
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 0;
		g_HDR3DLUTForceWrite = 1;
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(0);
		fwif_color_set_DM2_Hist_AutoMode_Enable(0);
		fwif_color_TC_Flow(0);
		fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
		Scaler_color_HFC_set_TC_FrameSync_Enable(0);
		fwif_color_set_ST2094_FrameSync_Enable(0);

	break;

	case HAL_VPQ_HDR_MODE_SDR_MAX_RGB:
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(modeinfo[1]);
		drvif_color_set_BBC_Enable(0);
		//drvif_DM2_GAMMA_Enable(0);		// set by hal or HFC
		fwif_color_set_HDR10_runmode();
		//drvif_color_set_BBC_shift_bit(0);
		//drvif_color_Set_ST2094_3Dlut_CInv(1024, 1);
		fwif_color_TC_Flow(0);
		drvif_DM2_OETF_EOTF_Path_Sel(1);
		fwif_color_set_DM2_Hist_AutoMode_Enable(1);

		fwif_color_set_HDR_VIVID_FrameSync_Enable(0);
		Scaler_color_HFC_set_TC_FrameSync_Enable(0);
		fwif_color_set_ST2094_FrameSync_Enable(0);
		
	break;
	case HAL_VPQ_HDR_MODE_HDR10_VIVID:
			VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
			Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(modeinfo[1]);
			drvif_color_set_BBC_Enable(1);
			//drvif_DM2_GAMMA_Enable(g_InvGamma);		// set by hal or HFC
			fwif_color_set_HDR10_runmode();
			fwif_color_TC_Flow(0);
			drvif_DM2_OETF_EOTF_Path_Sel(1);
			fwif_color_set_DM2_Hist_AutoMode_Enable(1);

			fwif_color_set_HDR_VIVID_FrameSync_Enable(1);
			Scaler_color_HFC_set_TC_FrameSync_Enable(0);
			fwif_color_set_ST2094_FrameSync_Enable(0);
	
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
	break;

	case HAL_VPQ_HDR_MODE_HDR10_VIVID:
		Scaler_LGE_HDR10_Enable_Seamless_hdmi_finish(modeinfo[1]);
	break;
	
	default:
		rtd_printk(KERN_ERR, TAG_NAME, "drvif_TV006_SET_HDR_mode, get unknow mode = %d\n", modeinfo[0]);
	break;
	}

}


void Scaler_LGE_HDR10_Enable_Seamless(unsigned char enable)
{
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;

	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_HDR10_enable_Coef] = enable;
	if (enable != 0)	//enable HDR 10
	{
		if (enable > 2) enable = 1; //protect
		drvif_color_set_HDR10_Enable_Seamless(hdr_table[1]);	//enable process
		fwif_color_set_Main_Dither(TRUE,VIP_DITHER_MAINTABLE_HDR10_12to10);
	}
	else
	{
		drvif_color_set_HDR10_Enable_Seamless(hdr_table[0]);	//disable process
	}
}

void Scaler_LGE_HDR10_Enable_Seamless_hdmi_position(unsigned char enable)
{
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table;

	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(VIP_RPC_system_info_structure_table == NULL){
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_HDR10_enable_Coef] = enable;
	if (enable != 0)	//enable HDR 10
	{
		if (enable > 2) enable = 1; //protect
		drvif_color_set_HDR10_Enable_Seamless_hdmi_position(hdr_table[1]);	//enable process
	}
	else
	{
		drvif_color_set_HDR10_Enable_Seamless_hdmi_position(hdr_table[0]);	//disable process
	}
}

void Scaler_LGE_HDR10_Enable_Seamless_hdmi_finish(unsigned char enable)
{
	if (enable != 0)	//enable HDR 10
	{
		if (enable > 2) enable = 1; //protect
		drvif_color_set_HDR10_Enable_Seamless_hdmi_finish(hdr_table[1]);	//enable process
		fwif_color_set_Main_Dither(TRUE,VIP_DITHER_MAINTABLE_HDR10_12to10);
	}
	else
	{
		drvif_color_set_HDR10_Enable_Seamless_hdmi_finish(hdr_table[0]);	//enable process
	}
}

void Scaler_SetDataFormatHandle_Seamless(void)
{
	unsigned char en_422to444, InputSrcGetType;
	unsigned char InputSrcGetFrom;
	unsigned short mode, channel;
	SCALER_DISP_CHANNEL display;

	//down(&VPQ_I_CSC_Semaphore);	// can't use semaphore in ISR
	mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	channel = Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL);
	en_422to444 = (unsigned char)Scaler_DispGetStatus(display, SLR_DISP_422);
	InputSrcGetType = Scaler_InputSrcGetType(display);

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
	if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000)
		fwif_color_SetDataFormatHandler(display, mode, channel, en_422to444, InputSrcGetType);
	else
		fwif_color_SetDataFormatHandler_tv006();
	//up(&VPQ_I_CSC_Semaphore);
}

#endif

static unsigned char* gHDR_AutoRun_TBL_Flag = NULL;
unsigned char*  Scaler_color_Access_HDR_AutoRun_TBL_Flag(unsigned char isSetFlag, unsigned char *HDR_AutoRun_TBL_Flag)
{
	if (isSetFlag == 1) {
		gHDR_AutoRun_TBL_Flag = HDR_AutoRun_TBL_Flag;
		rtd_pr_vpq_info("PQ_Mode_Info_TBL=%p\n",gHDR_AutoRun_TBL_Flag);
		if (gHDR_AutoRun_TBL_Flag != NULL)
			rtd_pr_vpq_info("PQ_Mode_Info_TBL[0]=%d\n", *gHDR_AutoRun_TBL_Flag);
	}
	return gHDR_AutoRun_TBL_Flag;
}

void Scaler_color_set_HDR_AutoRun_TBL(void)
{
	unsigned char *HDR_AutoRun_TBL_Flag;

	HDR_AutoRun_TBL_Flag = Scaler_color_Access_HDR_AutoRun_TBL_Flag(0, 0);

	if (HDR_AutoRun_TBL_Flag != NULL) {
		Scaler_color_HDRIP_AutoRun(HDR_AutoRun_TBL_Flag[0]);
		//Scaler_color_HDRInfo_Handler(PQ_Mode_Info);
		Scaler_color_Access_HDR_AutoRun_TBL_Flag(1, NULL);
	}
}

void Scaler_color_HDRIP_AutoRun(unsigned char HDR_Type)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
//========check HDR statu ===========
	unsigned char bHDR_flag = false;
	unsigned char input_src_type;
	unsigned char input_src_form;
	unsigned int modeinfo[5];
#ifdef CONFIG_CUSTOMER_TV002
	extern unsigned char g_bHDR_is601;
#endif

#ifdef CONFIG_I2RND_ENABLE
	//i2run case, re-run main flow need to get sub src
	if((vsc_i2rnd_dualvo_sub_stage == I2RND_DUALVO_STAGE_RERUN_MAIN_FOR_S1) || (vsc_i2rnd_sub_stage == I2RND_STAGE_MAIN_GET_RPC)){
		input_src_type = Scaler_InputSrcGetMainChType();
		input_src_form = Scaler_InputSrcGetMainChFrom();
	}
	else
#endif
	{
		if (fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC) && 
            (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_CAMERA)) {
			fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_MAIN_DISPLAY), &input_src_type, &input_src_form);
		} else {
			input_src_type = Scaler_InputSrcGetMainChType();
			input_src_form = Scaler_InputSrcGetMainChFrom();
		}
	}

	fwif_vip_HDR10_check(SLR_MAIN_DISPLAY,input_src_type);
	bHDR_flag = VIP_system_info_structure_table ->HDR_flag;
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
	rtd_pr_vpq_info("HDR auto, HDR_Type=%d, input_src_type=%d, i3d_en=%d, Get_DisplayMode_Src=%d\n",
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

	rtd_pr_vpq_info("Scaler_color_HDRInfo_Update, modeinfo = %d, Colorimetry info = %d\n", modeinfo[0], modeinfo[1]);
	if (modeinfo[1] == 0) modeinfo[1] = 2;	//data protect
	//SET_HDR_mode
	switch(modeinfo[0])	//HDR mode
	{
		case HAL_VPQ_HDR_MODE_HDR10:
			//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
			//Scaler_LGE_HDR10_Enable(modeinfo[1]);
			Scaler_Set_HDR_YUV2RGB(modeinfo[0], modeinfo[1]);
#if 0	/* set HDR10 Enable in "scaler_hdr_mode_setting()"*/
			Scaler_LGE_HDR10_Enable_newflow(modeinfo[1]);

			drvif_color_set_BBC_Enable(0);
#endif
			drvif_DM2_GAMMA_Enable(1);
			fwif_color_DM2_EOTF_OETF_3D_24x24x24_LUT_Handler(HAL_VPQ_HDR_MODE_HDR10);
			fwif_HDR_RGB2OPT(1, hdr_RGB2OPT[HDR_RGB2OPT_2020_TO_DCI]);
			fwif_color_set_TC_FrameSync_Enable(0);
			fwif_color_set_ST2094_FrameSync_Enable(0);
			//rtd_printk(KERN_INFO, TAG_NAME, "HDR10 mode EO[1] = %d, OE[1] = %d\n", EOTF_LUT_R[1], OETF_LUT_R[1]);
		break;
		case HAL_VPQ_HDR_MODE_HLG:
			//VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 1;
			//Scaler_LGE_HDR10_Enable(modeinfo[1]);
			Scaler_Set_HDR_YUV2RGB(modeinfo[0], modeinfo[1]);
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
			Scaler_Set_HDR_YUV2RGB(modeinfo[0], modeinfo[1]);
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
	} else if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10_VIVID) {
		result = HFC_HDR_VIVI;
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

	rtd_pr_vpq_info("[HFC]HDR timing = %d\n", result);
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

	/*if ((HDR_control[HDR_timing].TC_ctrl.IPCtrlBits) & HFC_bit)
		Scaler_color_HFC_set_TC_FrameSync_Enable(HDR_control[HDR_timing].TC_enbale);

	if ((HDR_control[HDR_timing].ST2094_ctrl.IPCtrlBits) & HFC_bit)
		fwif_color_set_ST2094_FrameSync_Enable(HDR_control[HDR_timing].ST2094_enbale);*/

	if ((HDR_control[HDR_timing].DM2_gamma_ctrl.IPCtrlBits) & HFC_bit)
		fwif_DM2_GAMMA_Enable(HDR_control[HDR_timing].DM2_gamma_enable);

}


void Scaler_color_HDRInfo_Handler(UINT32* modeinfo)
{
	HDR_flow_control_timing HDR_timing = Scaler_color_get_HDR_timing();
	Scaler_color_HDRInfo_Update_by_HFC(HFC_control_by_PQ, HDR_timing, modeinfo);
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
	extern UINT16 OETF_LUT_R[1025];
	dm_hdr_double_buffer_ctrl_RBUS dm_double_buffer_ctrl_reg;
	UINT16 i;

	//base no HDR10
	Scaler_LGE_ONEKEY_HDR10_Enable(1);


	//make identity EOTF
	for (i = 1; i < 1024; i++)
		EOTF_LUT_SDR2HDR[i] = (((unsigned int)1 << 22) * i);
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
	status2 = 1;//fwif_color_Set_Block_DeContour_Size(); // move before setting blkdecont dma 

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
#if 0 // use vpq tsk to monitor, use lv control
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

void Scaler_PQ_IP_Enable_Monitor_clear_Buff(void)
{
	unsigned char i;

	for (i=0;i<VIP_PQ_ByPass_ITEMS_Max;i++)
		VIP_PQ_IP_Enable_Monitor_Bypass_TBL[i] = 0;
}
#endif

#ifdef CONFIG_I2RND_ENABLE
extern unsigned char Scaler_I2rnd_get_timing_enable(void);
#endif
char Scaler_color_Set_PQ_ByPass_Lv(unsigned char table_idx)
{
	char ret;
  	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (VIP_RPC_system_info_structure_table == NULL) {
		VIPprintf("~table NULL, [%s:%d], Return\n~", __FILE__, __LINE__);
		return 0;
	}
#ifdef CONFIG_I2RND_ENABLE
	//if i2r enable, do not enater PQ bypass
	if(Scaler_I2rnd_get_timing_enable() == _ENABLE)
		return 0;
#endif
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

extern unsigned short panel_luminance;
extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_250N;
extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_300N;
extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_400N;
extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_500N;
extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_600N;
extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_250N;
extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_300N;
extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_400N;
extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_500N;
extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_600N;

extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_cur;
extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_cur;
void Scaler_color_Set_EOTF_OETF_Table(void)
{	
	rtd_pr_vpq_info("Scaler_color_Set_EOTF_OETF_Table, panel_luminance=%d\n", panel_luminance);
	/* Get EOTF, OETF table */
	if(panel_luminance <= 275){
		memcpy(&HDR10_EOTF_ByLuminace_TBL_cur, &HDR10_EOTF_ByLuminace_TBL_250N, sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
		memcpy(&HDR10_OETF_ByLuminace_TBL_cur, &HDR10_OETF_ByLuminace_TBL_250N, sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	} else if(panel_luminance > 275 && panel_luminance<= 350){
		memcpy(&HDR10_EOTF_ByLuminace_TBL_cur, &HDR10_EOTF_ByLuminace_TBL_300N, sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
		memcpy(&HDR10_OETF_ByLuminace_TBL_cur, &HDR10_OETF_ByLuminace_TBL_300N, sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	} else if(panel_luminance > 350 && panel_luminance <= 450){
		memcpy(&HDR10_EOTF_ByLuminace_TBL_cur, &HDR10_EOTF_ByLuminace_TBL_400N, sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
		memcpy(&HDR10_OETF_ByLuminace_TBL_cur, &HDR10_OETF_ByLuminace_TBL_400N, sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	} else if(panel_luminance > 450 && panel_luminance <= 550){
		memcpy(&HDR10_EOTF_ByLuminace_TBL_cur, &HDR10_EOTF_ByLuminace_TBL_500N, sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
		memcpy(&HDR10_OETF_ByLuminace_TBL_cur, &HDR10_OETF_ByLuminace_TBL_500N, sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	} else if(panel_luminance > 550){
		memcpy(&HDR10_EOTF_ByLuminace_TBL_cur, &HDR10_EOTF_ByLuminace_TBL_600N, sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
		memcpy(&HDR10_OETF_ByLuminace_TBL_cur, &HDR10_OETF_ByLuminace_TBL_600N, sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	}
}

extern long_int long_multiply( unsigned int a, unsigned int b );
extern long_int long_divide( long_int a, unsigned int b );
unsigned short calman_flag;
unsigned short dynamic_hdr_flag;

unsigned char HDR10_EOTF_ByMaxCLL_ByPanelLuminance_for_task_run = 0;
void EOTF_1024point(unsigned short maxlevel, VIP_HDR10_EOTF_ByLuminace_TBL* EOTF_table, VIP_HDR10_OETF_ByLuminace_TBL* OETF_table, unsigned int* EOTFyi_curve)
{
	int i, j;
	extern unsigned int HDR10_EOTF_2084[1025];
	unsigned short range_idx = 0;
	unsigned short max_index = 1;
	unsigned short norm_div, blend_alpha;
	unsigned int gamma_coef, gamma_v;
	long_int tmp1, tmp2;
	unsigned int temp_i, temp_index;

	// EOTF gain interval
	if(maxlevel <= 500)
		range_idx = 0;
	else if((maxlevel > 500) && (maxlevel <= 800))
		range_idx = 1;
	else if((maxlevel > 800) && (maxlevel <= 1150))
		range_idx = 2;
	else if((maxlevel > 1150) && (maxlevel <= 2000))
		range_idx = 3;
	else if((maxlevel > 2000) && (maxlevel <= 8000))
		range_idx = 4;
	else if(maxlevel > 8000)
		range_idx = 5 ;	
	
	
	// Get calman_flag, norm_div, gamma coef, blend_alpha
	dynamic_hdr_flag = OETF_table->OETF_setting[30];
	calman_flag = (OETF_table->OETF_setting[31] == 0)? 1: 0;
	norm_div    = EOTF_table->EOTF_setting[0];
	gamma_coef  = ((EOTF_table->EOTF_setting[1] & 0xFFFFFFFF) == 0) ? 2 : EOTF_table->EOTF_setting[1];
	blend_alpha = ((EOTF_table->EOTF_setting[2 + range_idx] & 0xFFFFFFFF) == 0) ? 100: EOTF_table->EOTF_setting[2 + range_idx];
	if(maxlevel > 2000){
		blend_alpha = 100;
	}

	
	// st2084 gain & get max index
	for( i=0; i<EOTF_size; i++ )
	{
		tmp1 = long_multiply(HDR10_EOTF_2084[i], norm_div);
		if (calman_flag)
			tmp2 = long_divide(tmp1, OETF_table->OETF_calman[range_idx][2]);
		else
			tmp2 = long_divide(tmp1, OETF_table->OETF_normal[range_idx][2]);
		EOTFyi_curve[i] = (tmp2.high > 0) ? 0xFFFFFFFF : tmp2.low;

		if((i > 1) && (EOTFyi_curve[i] == 0xFFFFFFFF) && (EOTFyi_curve[i-1] != 0xFFFFFFFF)){
			max_index = i;
		}
	}
	rtd_pr_vpq_info("EOTF get, calman_flag=%d, dynamic_hdr_flag=%d, norm_div=%d, gamma_coef=%d, blend_alpha=%d, MaxLevel=%d, range_idx=%d, max_index=%d\n", calman_flag, dynamic_hdr_flag, norm_div, gamma_coef, blend_alpha, maxlevel, range_idx, max_index);
		
	// st2084 blending with gamma
	for(i=0; i<EOTF_size; i++){
		// gamma
		temp_i = i;
		temp_index = max_index;
		for(j=1; j<gamma_coef; j++){
			temp_i = temp_i * i;
			temp_index = temp_index * max_index;
		}
		gamma_v = (0xFFFFFFFF / temp_index) * temp_i;
		if(gamma_v > 0xFFFFFFFF)
			gamma_v = 0xFFFFFFFF;

		// blending
		if((EOTFyi_curve[i] != 0xFFFFFFFF) && (blend_alpha != 100)){
			EOTFyi_curve[i] = (EOTFyi_curve[i] / 100 * blend_alpha) + (gamma_v / 100 * (100 - blend_alpha));
		}
	}
}

char Scaler_color_Get_HDR10_EOTF_ByMaxCLL_ByPanelLuminance(unsigned short forceIndex)
{
	unsigned char input_src_type;
	unsigned char input_src_form;
	unsigned char input_timming_info;
	SLR_VOINFO* pVOInfo = NULL;
	HDMI_DRM_T drm_info;
	unsigned short MaxCLL = 0, MaxFALL = 0, MaxDML = 0, MaxLevel;

	/* Get source info */
	if (fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC) &&
        (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_CAMERA)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_MAIN_DISPLAY), &input_src_type, &input_src_form);
		input_timming_info 	= drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
	} else {
		input_src_type = Scaler_InputSrcGetMainChType();
		input_src_form = Scaler_InputSrcGetMainChFrom();
		input_timming_info 	= Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	}

	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP) {
#if IS_ENABLED(CONFIG_RTK_DPRX)
		if (drvif_Dprx_GetInputType() == DP_TYPE_USB_TYPE_C)
			input_src_type = _SRC_TYPEC;
else
#endif
			input_src_type = _SRC_MINI_DP;
	}

	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	if (pVOInfo == NULL) {
		rtd_pr_vpq_err("Scaler_color_Get_HDR10_EOTF_ByMaxCLL_ByPanelLuminance, table=NULL, pVOInfo=%p, return !!!!\n", pVOInfo);
		return -1;
	}

	
	/* Get MaxCLL, MaxFALL, MaxDML */
	if(input_src_type == _SRC_VO) {
		MaxCLL = pVOInfo->MaxCLL;
		MaxFALL = pVOInfo->MaxFALL;
		MaxDML = 0;//pVOInfo->maxL;
	} else if (input_src_type == _SRC_HDMI) {
		if(drvif_Hdmi_GetDrmInfoFrame(&drm_info)){
			MaxCLL = drm_info.maximum_content_light_level;
			MaxFALL = drm_info.maximum_frame_average_light_level;
			MaxDML = drm_info.max_display_mastering_luminance;
		}
	} else if (input_src_type == _SRC_MINI_DP || input_src_type == _SRC_TYPEC) {
#if IS_ENABLED(CONFIG_RTK_DPRX)
		DPRX_DRM_INFO_T drm_info;
		if(drvif_Dprx_GetDrmInfoFrame(&drm_info)){
			MaxCLL = drm_info.maximum_content_light_level;
			MaxFALL = drm_info.maximum_frame_average_light_level;
			MaxDML = drm_info.max_display_mastering_luminance;
		}
#endif
	} else {
		rtd_pr_vpq_err("Scaler_color_Get_HDR10_EOTF_ByMaxCLL_ByPanelLuminance, is not VO or HDMI, SRC=%d\n", input_src_type);
		return 1;
	}


	/* Maxcll check */
	if (MaxCLL == 0)
		MaxLevel = 1000;
	else
		MaxLevel = MaxCLL;
	/* force select oetf*/
	if (forceIndex != 0)	
		MaxLevel = forceIndex;


	/* EOTF curve */
	EOTF_1024point(MaxLevel, &HDR10_EOTF_ByLuminace_TBL_cur, &HDR10_OETF_ByLuminace_TBL_cur, EOTF_LUT_ByMaxCLL_ByPanelLuminance);

	/* Set */
	fwif_color_set_DM2_EOTF_TV006(EOTF_LUT_ByMaxCLL_ByPanelLuminance, 0);
	return 0;
}


static int abs_int(int x)
{
  if (x >= 0) return x;
  if ((int)x == 0x80000000) return 0x7FFFFFFF;
  	return (-x);
}

int akima_spline( unsigned short *xi , unsigned short *yi , int *nin  ,
                unsigned short *xo , unsigned short *yo , int *nout )
{
	int i, niin, ii;
	static int *mii=NULL, *t=NULL;
	int a, b, c, p[4];

	niin = *nin+4;


	if(mii==NULL || t==NULL){
		rtd_printk(KERN_EMERG, TAG_NAME, "[%s,%d]\n", __func__, __LINE__);
		mii = (int *)dvr_malloc( ( niin ) * sizeof( int ) ) ;
		t = (int *)dvr_malloc( ( niin ) * sizeof( int ) ) ;
		if(mii==NULL || t==NULL)
			return 0;
	}

	for(i=0;i<*nin;i++){
		mii[i+2] = (((yi[i+1]<<7)-(yi[i]<<7))+((xi[i+1]-xi[i])>>1))/(xi[i+1]-xi[i]);
	}
	mii[1] = (mii[2]<<1) - mii[3];
	mii[0] = (mii[1]<<1) - mii[2];

	mii[niin-2] =  (mii[niin-3]<<1) - mii[niin-4];
	mii[niin-1] =  (mii[niin-2]<<1) - mii[niin-3];

	for(i=0;i<*nin;i++){
		a = abs_int(mii[i+3] - mii[i+2]);
		b = abs_int(mii[i+1] - mii[i+0]);
		if((a+b) != 0)
			t[i] = ((a*mii[i+1] + b*mii[i+2])+((a+b)>>1))/(a+b);
		else
			t[i] = (mii[i+2] + mii[i+1])>>1;
	}

	ii = 0;
	p[0] = (yi[ii]<<7);
	p[1] = t[ii];
	p[2] = ((3*mii[ii+2] - (t[ii]<<1) - t[ii+1]));
	p[3] = ((t[ii]+t[ii+1]-(mii[ii+2]<<1)));
	b = (xi[ii+1]-xi[ii]);
	c = b*b;
	for(i=0;i<*nout;i++){
		if(xo[i]>=xi[ii+1]){
			ii++;
			p[0] = (yi[ii]<<7);
			p[1] = t[ii];
			p[2] = ((3*mii[ii+2] - (t[ii]<<1) - t[ii+1]));
			p[3] = ((t[ii]+t[ii+1]-(mii[ii+2]<<1)));
			b = (xi[ii+1]-xi[ii]);
			c = b*b;
			//printk("[rock1][%s,%d][%d, %d] %d, %d, %d, %d,\n", __func__, __LINE__, i, ii, mii[ii-3], mii[ii-2], mii[ii-1], mii[ii]);
			//printk("[rock1][%s,%d][%d, %d] %d, %d\n", __func__, __LINE__, i, ii, t[ii], t[ii+1]);
		}
		a = xo[i]-xi[ii];

		yo[i] = p[0]>>7;
		if(p[1]!=0 && a != 0)
			yo[i] = yo[i] + ((p[1]*a)>>7);
		if(p[2]!=0 && a != 0)
			yo[i] = yo[i] + ((((p[2]*a*a)+(b>>1))/b)>>7);
		if(p[3]!=0 && a != 0)
			yo[i] = yo[i] + ((((p[3]*a*a)+(c>>1))/c)>>7)*a;
		//yo[i] = p[0] + p[1]*a + (p[2]*a*a)/b + (p[3]*a*a*a)/c;
				
		if(i!=0){
			if(yi[ii+1]>yo[i] && yo[i-1]>yo[i])
				yo[i] = yo[i-1];
			if(yi[ii+1]<yo[i] && yo[i-1]<yo[i])
				yo[i] = yo[i-1];		
		}
		//printk("[rock][%s,%d][%d, %d] %d, %d, %d, %d, %d, %d, %d\n", __func__, __LINE__, i, ii, yo[i], p[0], p[1], p[2]/b, p[3]/c,  xo[i], xi[ii]);
	}

	//dvr_free( mii ) ;
	//dvr_free( t ) ;

	return 0;
}

void OETF_inner_64point(int row, int col, unsigned short maxlevel, unsigned int OETFyi_curve[6][67], unsigned int* OETFyi_inner, unsigned int* shift_bit, unsigned short* _3dlut){
	int j;
	unsigned int range_idx = 0;
	
	if(maxlevel <= 500) //422
		range_idx = 0;
	else if((maxlevel > 500) && (maxlevel <= 800)) //600
		range_idx = 1;
	else if((maxlevel > 800) && (maxlevel <= 1150)) //1000
		range_idx = 2;
	else if((maxlevel > 1150) && (maxlevel <= 2000)) //1400
		range_idx = 3;
	else if((maxlevel > 2000) && (maxlevel <= 8000)) //4000
		range_idx = 4;
	else if(maxlevel > 8000) //10000
		range_idx = 5 ;

	// Shift BIT
	*shift_bit = OETFyi_curve[range_idx][1];

	// 3DLUT
	if(OETFyi_curve[range_idx][1] <= OETFyi_curve[2][1])
		*_3dlut = (16384 << ABS(OETFyi_curve[range_idx][1], OETFyi_curve[2][1])) * OETFyi_curve[range_idx][2] / 1000;
	else
		*_3dlut = (16384 >> ABS(OETFyi_curve[range_idx][1], OETFyi_curve[2][1])) * OETFyi_curve[range_idx][2] / 1000;

	// OETF curve
	for(j=3; j<col; j++){
		OETFyi_inner[j-3] = OETFyi_curve[range_idx][j];
		//rtd_printk(KERN_EMERG, TAG_NAME, "OETFyi_inner[%d]=%d\n", j-3, OETFyi_curve[range_idx][j]);
	}
}

static int oetfin = 64;
static int oetfout = 1025;
static unsigned short xo[1025];
static unsigned short OETFIdx[64] = {0,1,2,3,4,8,16,32,64,96,126,127,128,192,224,255,256,320,352,368,383,384,400,416,432,448,464,480,496,512,528,544,560,576,592,608,624,639,640,656,672,688,704,720,736,752,768,784,800,816,832,848,864,880,896,912,928,944,960,976,992,1008,1023,1024,};
unsigned char HDR10_OETF_ByMaxCLL_ByPanelLuminance_for_task_run = 0;
unsigned int OETF_shift_bit=0;
unsigned short OETF_3dlut=0;
unsigned short OETF_MaxCLL=0, OETF_MaxFALL=0, OETF_MaxDML=0, OETF_MaxLevel;
char Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance(unsigned short forceIndex)
{
	int i;
	unsigned char input_src_type;
	unsigned char input_src_form;
	unsigned char input_timming_info;
	SLR_VOINFO* pVOInfo = NULL;
	HDMI_DRM_T drm_info;
	unsigned int OETFyi_calman_inner_int[64]={0}, OETFyi_normal_inner_int[64]={0};
	unsigned short OETFyi_calman_inner_short[64]={0}, OETFyi_normal_inner_short[64]={0};
	unsigned short OETF_gain_offset[2];

	/* Get Source info */
	if (fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC) &&
        (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_CAMERA)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_MAIN_DISPLAY), &input_src_type, &input_src_form);
		input_timming_info 	= drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
	} else {
		input_src_type = Scaler_InputSrcGetMainChType();
		input_src_form = Scaler_InputSrcGetMainChFrom();
		input_timming_info 	= Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	}

	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP) {
#if IS_ENABLED(CONFIG_RTK_DPRX)
		if (drvif_Dprx_GetInputType() == DP_TYPE_USB_TYPE_C)
			input_src_type = _SRC_TYPEC;
		else
#endif
			input_src_type = _SRC_MINI_DP;
	}

	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	if (pVOInfo == NULL) {
		rtd_pr_vpq_err("Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance, table=NULL, pVOInfo=%p return !!!!\n", pVOInfo);
		return -1;
	}


	/* Get MaxCLL, MaxFALL, MaxDML */
	if(input_src_type == _SRC_VO) {
		OETF_MaxCLL = pVOInfo->MaxCLL;
		OETF_MaxFALL = pVOInfo->MaxFALL;
		OETF_MaxDML = 0;//pVOInfo->maxL;
	} else if (input_src_type == _SRC_HDMI) {
		if(drvif_Hdmi_GetDrmInfoFrame(&drm_info)){
			OETF_MaxCLL = drm_info.maximum_content_light_level;
			OETF_MaxFALL = drm_info.maximum_frame_average_light_level;
			OETF_MaxDML = drm_info.max_display_mastering_luminance;
		}
	} else if (input_src_type == _SRC_MINI_DP || input_src_type == _SRC_TYPEC) {
#if IS_ENABLED(CONFIG_RTK_DPRX)
		DPRX_DRM_INFO_T drm_info;
		if(drvif_Dprx_GetDrmInfoFrame(&drm_info)){
			OETF_MaxCLL = drm_info.maximum_content_light_level;
			OETF_MaxFALL = drm_info.maximum_frame_average_light_level;
			OETF_MaxDML = drm_info.max_display_mastering_luminance;
		}
#endif
	} else {
		rtd_pr_vpq_err("Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance, is not VO or HDMI, SRC=%d\n", input_src_type);
		return 1;
	}
	

	/* Check MaxLevel (MaxCLL == 0, can not get maxCLL)*/
	if (OETF_MaxCLL == 0)
		OETF_MaxLevel = 1000;
	else
		OETF_MaxLevel = OETF_MaxCLL;
	if (forceIndex != 0)	
		OETF_MaxLevel = forceIndex; //force select oetf

	/* Setting x axis index */
	for (i = 0; i < oetfout; i++){
		xo[i] = i;
	}


	/* Get OETF_gain_offset */
	OETF_gain_offset[0] = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[0];
	OETF_gain_offset[1] = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[1];
	rtd_pr_vpq_info("OETF get, input_src_type=%d, calman_flag=%d, panel_Luminance=%d, MaxCLL=%d, MaxFALL=%d, MaxDML=%d\n", input_src_type, calman_flag, panel_luminance, OETF_MaxCLL, OETF_MaxFALL, OETF_MaxDML);

	
	/* OETF curve */
	if(calman_flag){
		OETF_inner_64point(6, 67, OETF_MaxLevel, HDR10_OETF_ByLuminace_TBL_cur.OETF_calman, OETFyi_calman_inner_int, &OETF_shift_bit, &OETF_3dlut);

		// change type
		for(i=0;i<oetfin;i++){
			OETFyi_calman_inner_short[i] = (unsigned short)(OETFyi_calman_inner_int[i]);
		}
		rtd_pr_vpq_info("OETF get, OETF_shift_bit=%d, OETF_3dlut=%d\n", OETF_shift_bit, OETF_3dlut);		
	}
	else{
		OETF_inner_64point(6, 67, OETF_MaxLevel, HDR10_OETF_ByLuminace_TBL_cur.OETF_normal, OETFyi_normal_inner_int, &OETF_shift_bit, &OETF_3dlut);

		// Gain offset & change type
		for(i=0;i<oetfin;i++){
			OETFyi_normal_inner_int[i] = OETFyi_normal_inner_int[i] + ((OETFyi_normal_inner_int[i]-OETFyi_normal_inner_int[oetfin-1])*OETF_gain_offset[1])/1024;
			OETFyi_normal_inner_short[i] = (unsigned short)MIN(0xFFFF, ((OETFyi_normal_inner_int[i] * OETF_gain_offset[0]) / 128));
		}
		rtd_pr_vpq_info("OETF get, OETF_shift_bit=%d, OETF_3dlut=%d, OETF_gain=%d, OETF_offset=%d\n", OETF_shift_bit, OETF_3dlut, OETF_gain_offset[0], OETF_gain_offset[1]);
	}
	

	/* write bit shift & 3DLUT gain */
	if(OETF_shift_bit<=4){
		drvif_color_set_BBC_shift_bit(OETF_shift_bit);
		drvif_color_Set_ST2094_3Dlut_CInv(OETF_3dlut, 0, 14);
	}else{
		drvif_color_set_BBC_shift_bit(4);
		drvif_color_Set_ST2094_3Dlut_CInv(OETF_3dlut, 0, 14);
	}
	

	/* akima spline */
	akima_spline(OETFIdx, OETFyi_calman_inner_short, &oetfin, xo, OETF_LUT_ByMaxCLL_ByPanelLuminance_calman, &oetfout);
	akima_spline(OETFIdx, OETFyi_normal_inner_short, &oetfin, xo, OETF_LUT_ByMaxCLL_ByPanelLuminance, &oetfout);
	if(calman_flag)
		fwif_color_set_DM2_OETF_TV006(OETF_LUT_ByMaxCLL_ByPanelLuminance_calman, 0);
	else
		fwif_color_set_DM2_OETF_TV006(OETF_LUT_ByMaxCLL_ByPanelLuminance, 0);
	HDR10_OETF_ByMaxCLL_ByPanelLuminance_for_task_run = 1;
	return 0;
}


unsigned int EOTF_inner[1025] = {0};
unsigned short OETF_inner[1025] = {0};
void Scaler_color_Get_HDR10_EOTF_apply_rate(unsigned int *EOTF_TBL, unsigned int *EOTF_BLAND_TBL, unsigned int rate)
{
	unsigned int i;
	for(i=0;i<EOTF_size;i++){
		if(EOTF_BLAND_TBL[i]>EOTF_TBL[i])
			EOTF_inner[i] = EOTF_TBL[i] + (((EOTF_BLAND_TBL[i]-EOTF_TBL[i])/4096)*(rate>>4));	//((256>>4)*64)
		else
			EOTF_inner[i] = EOTF_TBL[i];
	}
	fwif_color_set_DM2_EOTF_TV006(EOTF_inner, 0);
}
void Scaler_color_Get_HDR10_OETF_apply_rate(unsigned short *OETF_TBL, unsigned short *OETF_CALMAN_TBL, unsigned int rate)
{
	unsigned int i;
	for(i=0;i<OETF_size;i++){
		OETF_inner[i] = (((OETF_TBL[i]*(rate) + OETF_CALMAN_TBL[i]*(256-rate)))>>8);
	}
	fwif_color_set_DM2_OETF_TV006(OETF_inner, 0);
}

unsigned int Abs_log_0to1[256] = {617,540,495,463,438,418,401,386,373,361,350,341,332,324,316,309,302,296,290,284,279,273,268,264,259,255,251,247,243,239,235,232,228,225,222,219,216,213,210,207,204,201,199,196,194,191,189,187,184,182,180,178,176,174,171,169,168,166,164,162,160,158,156,155,153,151,150,148,146,145,143,142,140,138,137,136,134,133,131,130,128,127,126,124,123,122,120,119,118,117,115,114,113,112,111,110,108,107,106,105,104,103,102,101,100,99,97,96,95,94,93,92,91,90,
	                              89,89,88,87,86,85,84,83,82,81,80,79,78,78,77,76,75,74,73,72,72,71,70,69,68,68,67,66,65,64,64,63,62,61,61,60,59,58,58,57,56,56,55,54,53,53,52,51,51,50,49,49,48,47,47,46,45,45,44,43,43,42,42,41,40,40,39,38,38,37,37,36,35,35,34,34,33,32,32,31,31,30,30,29,29,28,27,27,26,26,25,25,24,24,23,23,22,21,21,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10,10,9,9,8,8,7,7,6,6,5,5,4,4,4,3,3,2,2,1,1,0,};
unsigned int hist_map2_nit[129] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,3,3,4,4,5,5,6,7,7,8,9,10,11,12,14,15,17,18,20,22,24,26,29,32,34,37,41,44,48,52,57,62,67,73,79,85,92,100,108,117,126,136,147,158,171,184,199,214,231,249,268,288,310,334,359,387,416,447,481,517,556,598,643,691,742,798,857,921,990,1063,1142,1227,1318,1416,1522,1635,1756,1887,2027,2178,2341,2515,2703,2905,3123,3357,3609,3880,4173,4487,4827,5192,5586,6011,6470,6964,7498,8074,8696,9367,10000,};
void Scaler_color_Get_HDR10_EOTF_OETF_ByMaxCLL_ByPanelLuminance_for_task()
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned char oetf_lut_enable, b0105_or_oetf, b02b03_no_sram_mode;
	unsigned char eotf_lut_enable, b0103_or_eotf;
	unsigned int i;
	unsigned int HDR_Y_hist_total=0, HDR_OVER_Y_hist_total=0, HDR_Y_hist_mean_index=0;
	unsigned short blend_rate, entropy_rate, hist_rate;
	unsigned short x0,x1,x2,x3,x4,y0,y1,y2,y3,y4;
	int entropy = 0, cross_entropy = 0, KL = 0, probability = 0, gain_rate=0;
	static int cur_KL = 0;
	static unsigned int cur_gain_rate=0;
	static unsigned int pre_probability[256] = {0};

	HDMI_DRM_T drm_info;
	SLR_VOINFO* pVOInfo = NULL;	
	unsigned char input_src_type;
	unsigned char input_src_form;
	unsigned char input_timming_info;
	unsigned short Dynamic_MaxCLL=0, Dynamic_MaxFALL=0, Dynamic_MaxDML=0, Dynamic_MaxLevel;
	unsigned char HDR_flag;
	unsigned char Scaler_DispGetInputInfoByDisp_res;
	unsigned short VGIP_VGIP_CHN1_CTRL_res;
	unsigned int *HDR_Hist_Auto_Mode;
	HDR_flag = VIP_system_info_structure_table->HDR_flag;
	Scaler_DispGetInputInfoByDisp_res = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE);
	VGIP_VGIP_CHN1_CTRL_res = VGIP_VGIP_CHN1_CTRL_get_ch1_vact_end_ie(IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg));

	HDR_Hist_Auto_Mode = fwif_color_DM2_Hist_AutoMode_TBL_Get(DM2_Hist_RGB_MAX_Mode);

	if (HDR_flag != HAL_VPQ_HDR_MODE_HDR10)
		return ;

	/*=========================CHECK MAXCLL=========================*/
	if(Scaler_DispGetInputInfoByDisp_res != _MODE_STATE_ACTIVE){
		if((HDR_flag == HAL_VPQ_HDR_MODE_HDR10) && (VGIP_VGIP_CHN1_CTRL_res)){

			/* get input info */
			if (fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC) &&
                (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_CAMERA)) {
				fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_MAIN_DISPLAY), &input_src_type, &input_src_form);
				input_timming_info 	= drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR);
			} else {
				input_src_type = Scaler_InputSrcGetMainChType();
				input_src_form = Scaler_InputSrcGetMainChFrom();
				input_timming_info 	= Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
			}

			if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP) {
#if IS_ENABLED(CONFIG_RTK_DPRX)
				if (drvif_Dprx_GetInputType() == DP_TYPE_USB_TYPE_C)
					input_src_type = _SRC_TYPEC;
				else
#endif
					input_src_type = _SRC_MINI_DP;
			}

			pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
			if (pVOInfo == NULL) {
				rtd_pr_vpq_err("Scaler_color_Get_HDR10_EOTF_OETF_ByMaxCLL_ByPanelLuminance, table=NULL, pVOInfo=%p, return !!!!\n", pVOInfo);
			}

			
			/* get MaxCLL, MaxFALL, MaxDML info */
			if(input_src_type == _SRC_VO) {
				Dynamic_MaxCLL = pVOInfo->MaxCLL;
				Dynamic_MaxFALL = pVOInfo->MaxFALL;
				Dynamic_MaxDML = 0;//pVOInfo->maxL;
			} else if (input_src_type == _SRC_HDMI) {
				if(drvif_Hdmi_GetDrmInfoFrame(&drm_info)){
					Dynamic_MaxCLL = drm_info.maximum_content_light_level;
					Dynamic_MaxFALL = drm_info.maximum_frame_average_light_level;
					Dynamic_MaxDML = drm_info.max_display_mastering_luminance;
				}
			} else if (input_src_type == _SRC_MINI_DP || input_src_type == _SRC_TYPEC) {
#if IS_ENABLED(CONFIG_RTK_DPRX)
				DPRX_DRM_INFO_T drm_info;
				if(drvif_Dprx_GetDrmInfoFrame(&drm_info)){
					Dynamic_MaxCLL = drm_info.maximum_content_light_level;
					Dynamic_MaxFALL = drm_info.maximum_frame_average_light_level;
					Dynamic_MaxDML = drm_info.max_display_mastering_luminance;
				}
#endif
			} else {
				rtd_pr_vpq_err("Scaler_color_Get_HDR10_EOTF_OETF_ByMaxCLL_ByPanelLuminance, is not VO or HDMI, SRC=%d\n", input_src_type);
			}


			/* redefine Dynamic_MaxLevel */			
			if (Dynamic_MaxCLL == 0){
				rtd_pr_vpq_info("Scaler_color_Get_HDR10_EOTF_OETF_ByMaxCLL_ByPanelLuminance, Dynamic_MaxLevel==0\n");
				Dynamic_MaxLevel = 1000;
			} else{
				Dynamic_MaxLevel = Dynamic_MaxCLL;
			}
			if (OETF_MaxLevel == 0){
				rtd_pr_vpq_info("Scaler_color_Get_HDR10_EOTF_OETF_ByMaxCLL_ByPanelLuminance, OETF_MaxLevel==0\n");
				OETF_MaxLevel = Dynamic_MaxLevel;
			}

			/* Check if the current maxcll  has changed */
			if (Dynamic_MaxLevel != OETF_MaxLevel){
				rtd_pr_vpq_emerg("Scaler_color_Get_HDR10_EOTF_OETF_MaxCLL_change, Dynamic_MaxLevel=%d, OETF_MaxLevel=%d\n", Dynamic_MaxLevel, OETF_MaxLevel);
				Scaler_color_Get_HDR10_EOTF_ByMaxCLL_ByPanelLuminance(0);
				Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance(0);
			}
		}
	}


	/*=========================Dynamic HDR=========================*/
	if(Scaler_DispGetInputInfoByDisp_res == _MODE_STATE_ACTIVE){
		if (dynamic_hdr_flag){
			/* Histogram */
			drvif_Get_DM2_OETF_Enable_Status(&oetf_lut_enable, &b0105_or_oetf, &b02b03_no_sram_mode);
			drvif_Get_DM2_EOTF_Enable_Status(&eotf_lut_enable, &b0103_or_eotf, &b02b03_no_sram_mode);
			hist_rate = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[2];
			blend_rate = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[3];
			entropy_rate = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[4];
			//rtd_printk(KERN_INFO, TAG_NAME, "Scaler_color_Get_HDR10_EOTF_OETF_ByMaxCLL_ByPanelLuminance_for_task START");
		 
			HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[6] = OETF_MaxLevel;
			
			x0 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[9];
			x1 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[10];
			x2 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[11];
			x3 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[12];
			x4 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[13];

			if(OETF_MaxLevel<=500){
				y0 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[14];
				y1 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[15];
				y2 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[16];
				y3 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[17];
				y4 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[18];
			}else if(OETF_MaxLevel>500 && OETF_MaxLevel<=2000){
				y0 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[19];
				y1 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[20];
				y2 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[21];
				y3 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[22];
				y4 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[23];
			}else if(OETF_MaxLevel>2000){
				y0 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[24];
				y1 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[25];
				y2 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[26];
				y3 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[27];
				y4 = HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[28];
			}

			if((HDR_flag == HAL_VPQ_HDR_MODE_HDR10) && (VGIP_VGIP_CHN1_CTRL_res)){
				for(i=0;i<HDR_Histogram_size;i++){
					if(hist_map2_nit[i]<OETF_MaxLevel*2)
						HDR_Y_hist_total = HDR_Y_hist_total + HDR_Hist_Auto_Mode[i];
				}

				for(i=0;i<HDR_Histogram_size;i++){
					if(hist_map2_nit[i]<OETF_MaxLevel*2){
						HDR_OVER_Y_hist_total = HDR_OVER_Y_hist_total + HDR_Hist_Auto_Mode[i];
						if((HDR_OVER_Y_hist_total*256)<((HDR_Y_hist_total*hist_rate)))
							HDR_Y_hist_mean_index = i;
					}

					probability = (HDR_Hist_Auto_Mode[i]*256)/(HDR_Y_hist_total+1);
					if(probability>255){
						entropy += 0;
					}else{
						entropy += (HDR_Hist_Auto_Mode[i] * Abs_log_0to1[probability])/(HDR_Y_hist_total+1);
					}
					if(pre_probability[i]>255)
						cross_entropy+=0;
					else
						cross_entropy += (HDR_Hist_Auto_Mode[i] * Abs_log_0to1[pre_probability[i]])/(HDR_Y_hist_total+1);
					pre_probability[i] = probability;

				}
				KL = abs_int(cross_entropy - entropy);


				HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[7] = hist_map2_nit[HDR_Y_hist_mean_index];

				if(hist_map2_nit[HDR_Y_hist_mean_index]<=x0)
					gain_rate = y0;
				else if(hist_map2_nit[HDR_Y_hist_mean_index]<x1)
					gain_rate = y1+((x1-hist_map2_nit[HDR_Y_hist_mean_index])*(y0-y1))/(x1-x0);
				else if(hist_map2_nit[HDR_Y_hist_mean_index]<x2)
					gain_rate = y2+((x2-hist_map2_nit[HDR_Y_hist_mean_index])*(y1-y2))/(x2-x1);
				else if(hist_map2_nit[HDR_Y_hist_mean_index]<x3)
					gain_rate = y3+((x3-hist_map2_nit[HDR_Y_hist_mean_index])*(y2-y3))/(x3-x2);
				else if(hist_map2_nit[HDR_Y_hist_mean_index]<x4)
					gain_rate = y4+((x4-hist_map2_nit[HDR_Y_hist_mean_index])*(y3-y4))/(x4-x3);
				else
					gain_rate = y4;

				HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[8] = gain_rate;

				if(cur_gain_rate<gain_rate)
					cur_gain_rate = (blend_rate*cur_gain_rate + ((KL+cur_KL)+1)*gain_rate)/(blend_rate+((KL+cur_KL)+1)) + 1;
				else if(cur_gain_rate>gain_rate)
					cur_gain_rate = (blend_rate*cur_gain_rate + ((KL+cur_KL)+1)*gain_rate)/(blend_rate+((KL+cur_KL)+1)) - 1;
				cur_KL = (KL+cur_KL)/2;

				//rtd_pr_vpq_emerg("[rock] %d, %d, %d, %d, %d, %d, %d, %d\n", HDR_Y_hist_total, HDR_OVER_Y_hist_total, gain_rate, cur_gain_rate, entropy, HDR_Y_hist_mean_index,cur_KL,KL);
			}


			if((oetf_lut_enable == 1 && b0105_or_oetf == 1) || HDR10_OETF_ByMaxCLL_ByPanelLuminance_for_task_run){
				HDR10_OETF_ByMaxCLL_ByPanelLuminance_for_task_run = 0;
				if(OETF_shift_bit<=4){
					drvif_color_set_BBC_shift_bit(OETF_shift_bit);
					drvif_color_Set_ST2094_3Dlut_CInv(OETF_3dlut * cur_gain_rate / 256 , 0, 14);
				}else{
					drvif_color_set_BBC_shift_bit(4);
					drvif_color_Set_ST2094_3Dlut_CInv(OETF_3dlut * cur_gain_rate / 256, 0, 14);
				}
			}
		}
	}	
}

unsigned char Scaler_color_HAL_VPQ_HDR_Type_to_HDR_DM_Type(unsigned char HAL_VPQ_HDR)
{
	unsigned char ret= 0;
	if (HAL_VPQ_HDR == HAL_VPQ_HDR_MODE_SDR) {
		ret = HDR_DM_DOLBY;
	} else if (HAL_VPQ_HDR == HAL_VPQ_HDR_MODE_HDR10) {
		ret = HDR_DM_HDR10;

	} else if (HAL_VPQ_HDR == HAL_VPQ_HDR_MODE_HLG) {
		ret = HDR_DM_HLG;

	} else if (HAL_VPQ_HDR == HAL_VPQ_HDR_MODE_TECHNICOLOR) {
		ret= HDR_DM_TECHNICOLOR;

	} else if (HAL_VPQ_HDR == HAL_VPQ_HDR_MODE_ST2094) {
		ret = HDR_DM_ST2094;

	} else if (HAL_VPQ_HDR == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {
		ret = HDR_DM_SDR_MAX_RGB;

	} 
#ifdef support_hdr10_vivi_mode
	else if (HAL_VPQ_HDR == HAL_VPQ_HDR_MODE_HDR10_VIVID) {
		ret = HDR_DM_HDR10_VIVID;
	} 
#endif
	else {
		ret = HDR_DM_MODE_NONE;

	}
	return ret;
}

void Scaler_color_set_HDR_Type(unsigned char HDR_Type)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	rtd_pr_vpq_info("setHDR_Type=%d,\n", HDR_Type);
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

	}
	else if (HDR_Type == HDR_DM_VIVID) {
		VIP_system_info_structure_table ->HDR_flag = HAL_VPQ_HDR_MODE_HDR10_VIVID;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;
	}
	else {
		VIP_system_info_structure_table->HDR_flag = HAL_VPQ_HDR_MODE_SDR;
		VIP_system_info_structure_table->DolbyHDR_flag = 0;

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

unsigned char panel_luminance_updated = 0;
void Scaler_color_copy_HDR_table_from_AP(SLR_HDR_TABLE* ptr_HDR_bin)
{
	extern unsigned short HLG_3DLUT_24x24x24[HDR_24x24x24_size][3];
	extern unsigned int EOTF_LUT_R[1025];
	extern unsigned short OETF_LUT_R[1025];
	extern unsigned int HLG_EOTF_LUT_R[EOTF_size];
	extern unsigned short HLG_OETF_LUT_R[OETF_size];
	extern unsigned int HDR10_EOTF_2084[EOTF_size];
	extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_250N;
	extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_300N;
	extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_400N;
	extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_500N;
	extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_600N;
	extern unsigned short panel_luminance;
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_250N;
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_300N;
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_400N;
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_500N;
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_600N;
	extern VIP_TCHDR_Mode_TBL TCHDR_Mode_TBL;
	extern DRV_TCHDR_Table TCHDR_Table[TC_Table_NUM];
	extern DRV_TCHDR_COEF_I_lut TCHDR_COEF_I_LUT_TBL;
	extern DRV_TCHDR_COEF_S_lut TCHDR_COEF_S_LUT_TBL;
	extern DRV_TCHDR_v130_D_Table_settings TCHDR_v130_D_Table_settings[NUM_D_LUT_DEFAULT];

	memcpy(hdr_table, ptr_HDR_bin->hdr_table, sizeof(UINT8)* HDR_TABLE_NUM * HDR_ITEM_MAX);
	memcpy(hdr_RGB2OPT, ptr_HDR_bin->hdr_RGB2OPT, sizeof(UINT32)* HDR_RGB2OPT_TABLE_NUM * 9);
	memcpy(HDR10_3DLUT_17x17x17, ptr_HDR_bin->HDR10_3DLUT_17x17x17, sizeof(short)* 17 * 17 * 17 * 3);
	memcpy(HDR10_3DLUT_24x24x24, ptr_HDR_bin->HDR10_3DLUT_24x24x24, sizeof(UINT16)* HDR_24x24x24_size * 3);
	memcpy(HLG_3DLUT_24x24x24, ptr_HDR_bin->HLG_3DLUT_24x24x24, sizeof(UINT16)* HDR_24x24x24_size * 3);
	memcpy(EOTF_LUT_R, ptr_HDR_bin->EOTF_LUT_R, sizeof(UINT32)* 1025);
	memcpy(OETF_LUT_R, ptr_HDR_bin->OETF_LUT_R, sizeof(UINT16)* 1025);
	memcpy(HLG_EOTF_LUT_R, ptr_HDR_bin->HLG_EOTF_LUT_R, sizeof(HLG_EOTF_LUT_R));
	memcpy(HLG_OETF_LUT_R, ptr_HDR_bin->HLG_OETF_LUT_R, sizeof(HLG_OETF_LUT_R));
	memcpy(HDR10_EOTF_2084, ptr_HDR_bin->HDR10_EOTF_2084, sizeof(HDR10_EOTF_2084));
	memcpy(&HDR10_EOTF_ByLuminace_TBL_250N, &(ptr_HDR_bin->HDR10_EOTF_ByLuminace_TBL_250N), sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
	memcpy(&HDR10_EOTF_ByLuminace_TBL_300N, &(ptr_HDR_bin->HDR10_EOTF_ByLuminace_TBL_300N), sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
	memcpy(&HDR10_EOTF_ByLuminace_TBL_400N, &(ptr_HDR_bin->HDR10_EOTF_ByLuminace_TBL_400N), sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
	memcpy(&HDR10_EOTF_ByLuminace_TBL_500N, &(ptr_HDR_bin->HDR10_EOTF_ByLuminace_TBL_500N), sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
	memcpy(&HDR10_EOTF_ByLuminace_TBL_600N, &(ptr_HDR_bin->HDR10_EOTF_ByLuminace_TBL_600N), sizeof(VIP_HDR10_EOTF_ByLuminace_TBL));
	if (panel_luminance_updated == 0)
		memcpy(&panel_luminance, &(ptr_HDR_bin->panel_luminance), sizeof(UINT16));
	memcpy(&HDR10_OETF_ByLuminace_TBL_250N, &(ptr_HDR_bin->HDR10_OETF_ByLuminace_TBL_250N), sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	memcpy(&HDR10_OETF_ByLuminace_TBL_300N, &(ptr_HDR_bin->HDR10_OETF_ByLuminace_TBL_300N), sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	memcpy(&HDR10_OETF_ByLuminace_TBL_400N, &(ptr_HDR_bin->HDR10_OETF_ByLuminace_TBL_400N), sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	memcpy(&HDR10_OETF_ByLuminace_TBL_500N, &(ptr_HDR_bin->HDR10_OETF_ByLuminace_TBL_500N), sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	memcpy(&HDR10_OETF_ByLuminace_TBL_600N, &(ptr_HDR_bin->HDR10_OETF_ByLuminace_TBL_600N), sizeof(VIP_HDR10_OETF_ByLuminace_TBL));
	memcpy(&TCHDR_Mode_TBL, &(ptr_HDR_bin->TCHDR_Mode_TBL), sizeof(VIP_TCHDR_Mode_TBL));
	//memcpy(TCHDR_Table, ptr_HDR_bin->TCHDR_Table, sizeof(DRV_TCHDR_Table) * TC_Table_NUM);
	//memcpy(&TCHDR_COEF_I_LUT_TBL, &(ptr_HDR_bin->TCHDR_COEF_I_LUT_TBL), sizeof(DRV_TCHDR_COEF_I_lut));
	//memcpy(&TCHDR_COEF_S_LUT_TBL, &(ptr_HDR_bin->TCHDR_COEF_S_LUT_TBL), sizeof(DRV_TCHDR_COEF_S_lut));
	//memcpy(TCHDR_v130_D_Table_settings, ptr_HDR_bin->TCHDR_v130_D_Table_settings, sizeof(DRV_TCHDR_v130_D_Table_settings)* NUM_D_LUT_DEFAULT);

	/* According to panel luminance to get current EOTF?�OETF table */
	Scaler_color_Set_EOTF_OETF_Table();
}

void Scaler_color_copy_HDR_table_panel_luminance_from_AP(void)
{
	extern unsigned short panel_luminance;
	unsigned short panel_luminance_from_ini = 0;
	
	panel_luminance_from_ini = VIP_AP_DEM_TBL.PANEL_DATA_.DEM_PANEL_INI_Color_Chromaticity.Panel_luma_max;
	
	rtd_pr_vpq_info("Scaler_color_copy_HDR_table_panel_luminance_from_AP,  panel_luminance1=%d\n", panel_luminance_from_ini);
	if(panel_luminance_from_ini != 0){
		panel_luminance = panel_luminance_from_ini;
		Scaler_color_Set_EOTF_OETF_Table(); // According to panel luminance to get current EOTF?�OETF table
		panel_luminance_updated = 1;
	}
}

void Scaler_color_copy_HDR_table_oetf_setting_ctrl_from_AP(unsigned short oetf_setting_ctrl_from_ini)
{
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_cur;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned char HDR_flag;
	
	HDR_flag = VIP_system_info_structure_table->HDR_flag;
	HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[30] = oetf_setting_ctrl_from_ini;
	Scaler_color_HDRIP_AutoRun(Scaler_color_HAL_VPQ_HDR_Type_to_HDR_DM_Type(HDR_flag));
	
	rtd_pr_vpq_info("Scaler_color_copy_HDR_table_oetf_setting_ctrl_from_AP, HDR_flag=%d, enable=%d\n", HDR_flag, oetf_setting_ctrl_from_ini);
}

void Scaler_color_copy_HDR_table_oetf_curve_ctrl_from_AP(unsigned short oetf_curve_index_from_ini)
{
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_cur;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned char HDR_flag;
	
	HDR_flag = VIP_system_info_structure_table->HDR_flag;
	HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[31] = oetf_curve_index_from_ini;
	Scaler_color_HDRIP_AutoRun(Scaler_color_HAL_VPQ_HDR_Type_to_HDR_DM_Type(HDR_flag));

	rtd_pr_vpq_info("Scaler_color_copy_HDR_table_oetf_curve_ctrl_from_AP, HDR_flag=%d, index=%d\n", HDR_flag, oetf_curve_index_from_ini);
		
}

static short int  REG_DEGREE=115;
static short int  REG_DEGREE_LV2=120;
static short int  REG_DEGREE_LV3=125;

/**table_iDX
 * Level = 0 => no bypass?? * Level = 1 => I DCTI?．_DCTI??�_DCTI?．LTI : bypass?? * Level = 2 => I DCTI?．_DCTI??�_DCTI?．LTI??�TNR Y??�TNR C?｀CNR?．I IP Enabl

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
	SLR_VIP_TABLE *pVipTable = fwif_colo_get_AP_vip_table_gVIP_Table();

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
	unsigned char HSR_mode = 0;
	unsigned char *pVipCoefArray = NULL;
	unsigned char *pVipCoefExt4Array = NULL;
	
	if (vpqex_project_id == 0x00060000)
		return;

#ifdef CONFIG_DYNAMIC_PANEL_SELECT
#ifndef BUILD_QUICK_SHOW
	if(PANEL_DLG_4k1k120 == vbe_get_cur_dlg_mode())
		HSR_mode = 1;
#endif
#endif

	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/

	if (m_cbFunc_getQualityExtend4 != NULL)
		pVipCoefExt4Array = (unsigned char *)m_cbFunc_getQualityExtend4((VIP_SOURCE_TIMING)source);

	if (m_cbFunc_getQualityExtend3 != NULL)
		pVipCoefArray = (unsigned char *)m_cbFunc_getQualityExtend3((VIP_SOURCE_TIMING)source);
	else {
		rtd_printk(KERN_DEBUG, TAG_NAME, "PQ table m_cbFunc_getQualityExtend3 read fail !!!\n");
		return;
	}

	if (!(source >= VIP_QUALITY_TOTAL_SOURCE_NUM || pVipCoefArray == NULL)) {
		if (HSR_mode && pVipCoefExt4Array) {
			if (Scaler_GetSharpnessTable() != pVipCoefExt4Array[VIP_QUALITY_FUNCTION_HSR_SharpTable]) {
				Scaler_SetSharpnessTable(pVipCoefExt4Array[VIP_QUALITY_FUNCTION_HSR_SharpTable]);
				Scaler_SetSharpness(Scaler_GetSharpness());
			}
		} else {
			if (Scaler_GetSharpnessTable() != pVipCoefArray[VIP_QUALITY_FUNCTION_SharpTable]) {
				Scaler_SetSharpnessTable(pVipCoefArray[VIP_QUALITY_FUNCTION_SharpTable]);
				Scaler_SetSharpness(Scaler_GetSharpness());
			}
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
	//return fwif_color_od_init(10, 0, 60);
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

char Scaler_Set_DI_SNR_Borrow_Sram_Mode(VIP_SNR_DI_Sram_Borrow_Mode mode)
{
	char ret_val;
	unsigned char mode_ori;
	_RPC_system_setting_info *rpc_vipSysInfo = NULL;
	rpc_vipSysInfo = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (rpc_vipSysInfo == NULL) {
		rtd_pr_vpq_emerg("~get rpc_vipSysInfo return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	mode_ori = rpc_vipSysInfo->SNR_DI_Sram_Borrow_Mode;
	ret_val = 0;
	if (mode_ori != mode) {
		if (mode == SNR_DI_Sram_Release_From_VE) {
			if ((mode_ori == SNR_DI_Sram_VE_USE) || (mode_ori == SNR_DI_Sram_Release_From_DI_SNR)) {
				rpc_vipSysInfo->SNR_DI_Sram_Borrow_Mode = mode;	
				drvif_color_Set_VE_Only_Sel(0);
				drvif_color_set_iEdgeSmooth_linemode_disable(0, 1);
				ret_val = 0;
			} else {
				ret_val = 1;	// 1 as fail
			}
		} else if (mode == SNR_DI_Sram_VE_USE) {
			if ((mode_ori == SNR_DI_Sram_Release_From_VE) || (mode_ori == SNR_DI_Sram_Release_From_DI_SNR)) {
				rpc_vipSysInfo->SNR_DI_Sram_Borrow_Mode = mode; 
				drvif_color_Set_VE_Only_Sel(1);
				drvif_color_set_iEdgeSmooth_linemode_disable(1, 1);
				ret_val = 0;
			} else {
				ret_val = 1;	// 1 as fail
			}
		} else if (mode == SNR_DI_Sram_Release_From_DI_SNR) {
			if ((mode_ori == SNR_DI_Sram_DI_SNR_USE) || (mode_ori == SNR_DI_Sram_Release_From_VE)) {
				rpc_vipSysInfo->SNR_DI_Sram_Borrow_Mode = mode;	
				drvif_color_Set_VE_Only_Sel(0);
				drvif_color_set_iEdgeSmooth_linemode_disable(0, 1);
				ret_val = 0;
			} else {
				ret_val = 1;	// 1 as fail
			}
		} else if (mode == SNR_DI_Sram_DI_SNR_USE) {
			if ((mode_ori == SNR_DI_Sram_Release_From_VE) || (mode_ori == SNR_DI_Sram_Release_From_DI_SNR)) {
				rpc_vipSysInfo->SNR_DI_Sram_Borrow_Mode = mode; 
				drvif_color_Set_VE_Only_Sel(0);
				drvif_color_set_iEdgeSmooth_linemode_disable(0, 1);
				ret_val = 0;
			} else {
				ret_val = 1;	// 1 as fail
			}
		} else {
			rtd_pr_vpq_info("Set_VE_Borrow_Sram_Mode, unknow command\n");			
			ret_val = 1;	// fail
		}
	}

	rtd_pr_vpq_info("Set_VE_Borrow_Sram_Mode, set mode=%d, mode from %d to %d, ret_val=%d,\n", 
		mode, mode_ori, rpc_vipSysInfo->SNR_DI_Sram_Borrow_Mode, ret_val);
	
	return ret_val;
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
EXPORT_SYMBOL(Scaler_Set_DI_SNR_Borrow_Sram_Mode);
#endif

char Scaler_Get_DI_SNR_Borrow_Sram_Mode(void)
{
	_RPC_system_setting_info *rpc_vipSysInfo = NULL;
	rpc_vipSysInfo = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (rpc_vipSysInfo == NULL) {
		rtd_pr_vpq_emerg("~get rpc_vipSysInfo return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	return rpc_vipSysInfo->SNR_DI_Sram_Borrow_Mode;

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

void Scaler_color_set_RTNR_H_Rotate(unsigned char display, unsigned char En)
{

	DRV_RTNR_General_ctrl ptr = {0};

	if (display == SLR_MAIN_DISPLAY) {

		rtd_pr_vpq_info("RTNR_H_Rotate, display=%d, En=%d\n",
			display, En);

		if (En>1)
			En = 0;

		if (En == 0) {
			drvif_color_Set_RTNR_H_Rotate(0, 0);
			
		} else {
			drvif_color_Set_RTNR_H_Rotate(1, 1);
			drvif_color_DRV_RTNR_General_ctrl(&ptr);
		}
	}
}

void Scaler_color_decide_RTNR_H_Rotate(unsigned char display)
{
	unsigned char DI_10bit;
	unsigned short isInterlace;

	if (display == SLR_MAIN_DISPLAY) {
#ifndef BUILD_QUICK_SHOW
		isInterlace = Scaler_DispGetStatus((SCALER_DISP_CHANNEL)display, SLR_DISP_INTERLACE);
		DI_10bit = fw_scalerip_get_DI_chroma_10bits();

		if ((get_rtnr_hflip_flag() == 0) || (isInterlace) || (!DI_10bit))
			Scaler_color_access_RTNR_H_Rotate_Flag(0, 1);
		else
			Scaler_color_access_RTNR_H_Rotate_Flag(1, 1);

		rtd_pr_vpq_info("RTNR_H_Rotate,decide, display=%d, isInterlace=%d, DI_10bit=%d, En=%d\n",
				display, isInterlace, DI_10bit, get_rtnr_hflip_flag());
#else	// no rotate on qs
		Scaler_color_access_RTNR_H_Rotate_Flag(0, 1);
#endif
	}	
}

void Scaler_color_Dither_Table_Ctrl(void)
{
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	di_color_recovery_option_RBUS di_color_recovery_option_reg;
	unsigned char nIdex=0,nDITHER_ENABLE=0,nDITHER_MAP_BIT=0,nDITHER_LUT=0;
	extern void* m_pVipTableCustom; /* SLR_VIP_TABLE_CUSTOM_TV001  */
	SLR_VIP_TABLE_CUSTOM_TV001* tmp_pVipTableCustom = (SLR_VIP_TABLE_CUSTOM_TV001*)m_pVipTableCustom;
	di_color_recovery_option_reg.regValue = IoReg_Read32(DI_Color_Recovery_Option_reg);

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
						if (get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY) && fw_scalerip_get_DI_chroma_10bits() == 0) /* cp  8bit*/
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
#ifndef BUILD_QUICK_SHOW
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
#endif
	return 0;
}

void Scaler_SetNNSR_Ctrl_Table(unsigned char TableIdx)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	//rtd_printk(KERN_DEBUG, TAG_NAME, " NNSR ===[scalerColorLib_Default.cpp] 1.NNSR_Table = %d\n", TableIdx);
	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL) {
		rtd_pr_vpq_emerg(" NNSR Scaler_SetNNSR_Ctrl_Table error %d \n", TableIdx);
		return;
	}
	pTable->NNSR_Ctrl = TableIdx;

	fwif_color_Set_NN_SRCtrl(TableIdx);
}
unsigned char Scaler_GetNNSR_Ctrl_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL)
		return 0;

	return pTable->NNSR_Ctrl;
}

void Scaler_SetNNSR_Blending_Weight_Table(unsigned char TableIdx)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	rtd_printk(KERN_DEBUG, TAG_NAME, "NNSR_Table = %d\n", TableIdx);
	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL) {
		rtd_printk(KERN_DEBUG, TAG_NAME, "NNSR Scaler_SetNNSR_Weight_Table error %d \n", TableIdx);
		return;
	}
	pTable->NNSR_Weight = TableIdx;

	//fwif_color_Set_NN_SRCtrl(TableIdx);
}
unsigned char Scaler_GetNNSR_Blending_Weight_Table(void)
{
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);

	if (pTable == NULL)
		return 0;

	return pTable->NNSR_Weight;
}

/*============Camera Flow============*/
unsigned char Ori_gammma_en=0;
unsigned char Ori_Invgammma=0;
extern unsigned char aipq_DynamicContrastLevel;
extern unsigned char Ori_YUV2RGB_Enable_Main;
extern unsigned char Ori_SNR_spatialenabley;
extern unsigned char Ori_I_De_Contour_en;
extern unsigned char Ori_block_decontour_en;
extern unsigned char Ori_MPEGV_mpegresultweight_y;
extern unsigned char Ori_MPEGH_mpegresultweight_x;
extern unsigned char Ori_rtnr_ma_snr_en;
extern unsigned char Ori_MBPK_H_gain_neg;
extern unsigned char Ori_MBPK_H_gain_pos;
extern unsigned char Ori_MBPK_V_gain_neg;
extern unsigned char Ori_MBPK_V_gain_pos;

void Scaler_Set_Camera_PQsetting(unsigned char get_vo_camera_flow_flag)
{
    color_d_vc_global_ctrl_RBUS d_vc_global_ctrl_Reg;
    yuv2rgb_d_yuv2rgb_control_RBUS d_yuv2rgb_control_REG;
	nr_dch1_cp_ctrl_RBUS dch1_cp_ctrl_reg;
	ipq_decontour_de_contour_ctrl_RBUS ipq_decontour_de_contour_ctrl_reg;
	ipq_decontour_blk_de_contour_ctrl_RBUS blk_de_contour_ctrl_reg;	
    nr_dch1_mpegnr1_RBUS	nr_dch1_mpegnr1_reg;
    mpegnr_ich1_mpegnr2_RBUS	mpegnr_ich1_mpegnr2_reg	;
    di_rtnr_ma_snr_control_RBUS di_rtnr_ma_snr_control;
    color_mb_peaking_mb_peaking_hor_gain_RBUS   color_mb_peaking_mb_peaking_hor_gain;
	color_mb_peaking_mb_peaking_ver_gain_RBUS   color_mb_peaking_mb_peaking_ver_gain;
    color_mb_peaking_mb_double_buffer_ctrl_RBUS		color_mb_peaking_mb_double_buffer_ctrl;    
    di_im_di_rtnr_y_16_th0_th3_RBUS im_di_rtnr_y_16_th0_th3_reg;
	di_im_di_rtnr_y_16_th4_th7_RBUS im_di_rtnr_y_16_th4_th7_reg;
	di_im_di_rtnr_y_16_th8_th11_RBUS im_di_rtnr_y_16_th8_th11_reg;
	di_im_di_rtnr_y_16_th12_th14_RBUS im_di_rtnr_y_16_th12_th14_reg;
    dc2h_rgb2yuv_dc2h_rgb2yuv_ctrl_RBUS  dc2h_rgb2yuv_ctrl_reg;
    unsigned char idx_temp, arg;
	
    d_vc_global_ctrl_Reg.regValue   = IoReg_Read32(COLOR_D_VC_Global_CTRL_reg);
    d_yuv2rgb_control_REG.regValue  = IoReg_Read32(YUV2RGB_D_YUV2RGB_Control_reg);
	dch1_cp_ctrl_reg.regValue       = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
    ipq_decontour_de_contour_ctrl_reg.regValue  = IoReg_Read32(IPQ_DECONTOUR_De_Contour_CTRL_reg);
    blk_de_contour_ctrl_reg.regValue    = IoReg_Read32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg);
    nr_dch1_mpegnr1_reg.regValue        = IoReg_Read32(NR_DCH1_MPEGNR1_reg);
    mpegnr_ich1_mpegnr2_reg	.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR2_reg	);
    di_rtnr_ma_snr_control.regValue     = IoReg_Read32(DI_RTNR_MA_SNR_CONTROL_reg);
    color_mb_peaking_mb_peaking_hor_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Hor_GAIN_reg);
	color_mb_peaking_mb_peaking_ver_gain.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_PEAKING_Ver_GAIN_reg);
    im_di_rtnr_y_16_th0_th3_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH0_TH3_reg);
	im_di_rtnr_y_16_th4_th7_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH4_TH7_reg);
	im_di_rtnr_y_16_th8_th11_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH8_TH11_reg);
	im_di_rtnr_y_16_th12_th14_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_Y_16_TH12_TH14_reg);
    dc2h_rgb2yuv_ctrl_reg.regValue = IoReg_Read32(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg);
    
  if(get_vo_camera_flow_flag){
    rtd_pr_vpq_info("get_vo_camera_flow_flag : ON\n");    
    
    /* disable gamma */
    IoReg_ClearBits(GAMMA_GAMMA_CTRL_2_reg, GAMMA_GAMMA_CTRL_2_gamma_m_tab_sel_mask);//_BIT6|_BIT7    
    /* disable Invgamma */   
    IoReg_ClearBits(INV_GAMMA_INV_GAMMA_CTRL_2_reg, INV_GAMMA_INV_GAMMA_CTRL_2_inv_gamma_m_tab_sel_mask);//_BIT6|_BIT7 
	Scaler_Set_ColorMapping_3x3_Table(Color_Space_Target_OFF);	
	/* disable DCC */
    d_vc_global_ctrl_Reg.dcc_en = 0;
    d_vc_global_ctrl_Reg.write_enable_1 =1;   
    IoReg_Write32(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_Reg.regValue);
    /* disable LC */
    Scaler_SetLocalContrastEnable(0);
	/* disable BYPASS_YUV2RGB_MATRIX */ 
    d_yuv2rgb_control_REG.yuv2rgb_main_en = 0;   
    IoReg_Write32(YUV2RGB_D_YUV2RGB_Control_reg, d_yuv2rgb_control_REG.regValue);
    /* disable DC2H_rgb2yuv */ 
    dc2h_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;  
    IoReg_Write32(DC2H_RGB2YUV_DC2H_RGB2YUV_CTRL_reg, dc2h_rgb2yuv_ctrl_reg.regValue);
    /* PQ_ByPass_SNR_Y */ 
	dch1_cp_ctrl_reg.cp_spatialenabley = 0;
    IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
    /* PQ_ByPass_De_Contour */ 
    drvif_color_Set_I_De_Contour_DB_En( 1 );
	ipq_decontour_de_contour_ctrl_reg.decont_en=0;
    IoReg_Write32(IPQ_DECONTOUR_De_Contour_CTRL_reg, ipq_decontour_de_contour_ctrl_reg.regValue);
    drvif_color_Set_I_De_Contour_DB_Apply();
    /* disable block-decontour */ 
    blk_de_contour_ctrl_reg.bdecont_en=0;
    IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg, blk_de_contour_ctrl_reg.regValue);
    /* disable cp_mpegresultweight_y */ 
    nr_dch1_mpegnr1_reg.cp_mpegresultweight_y = 0;
    IoReg_Write32(NR_DCH1_MPEGNR1_reg,	nr_dch1_mpegnr1_reg	.regValue);
    IoReg_SetBits(NR_V_MPEGNR_DB_CTRL_reg, NR_V_MPEGNR_DB_CTRL_mpegnr_db_apply_mask); // DB apply
    /* disable cp_mpegresultweight_x */ 
    mpegnr_ich1_mpegnr2_reg.cp_mpegresultweight_x =0;
    IoReg_Write32(	MPEGNR_ICH1_MPEGNR2_reg	,	mpegnr_ich1_mpegnr2_reg	.regValue);
    IoReg_SetBits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_apply_mask);// DB apply
    /* disable rtnr_ma_snr_en */
    di_rtnr_ma_snr_control.rtnr_ma_snr_en = 0;
    IoReg_Write32(DI_RTNR_MA_SNR_CONTROL_reg, di_rtnr_ma_snr_control.regValue);
    /* set MBPK camera flag setting*/	
	color_mb_peaking_mb_double_buffer_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg);//Enable doublue buffer
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_en =1;
	IoReg_Write32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, color_mb_peaking_mb_double_buffer_ctrl.regValue);	
	color_mb_peaking_mb_peaking_hor_gain.mb_gain_neg = 80;//ROW2 VIP_MBPK_H_Table
	color_mb_peaking_mb_peaking_hor_gain.mb_gain_pos = 80;	
	color_mb_peaking_mb_peaking_ver_gain.mb_gain_neg_v = 80;//ROW4 VIP_MBPK_V_Table
	color_mb_peaking_mb_peaking_ver_gain.mb_gain_pos_v = 80;
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Hor_GAIN_reg, color_mb_peaking_mb_peaking_hor_gain.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Ver_GAIN_reg, color_mb_peaking_mb_peaking_ver_gain.regValue);	
	color_mb_peaking_mb_double_buffer_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg);//Enable doublue buffer
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_en =1;
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_apply =1;
	IoReg_Write32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, color_mb_peaking_mb_double_buffer_ctrl.regValue);
    /* set RTNR motion th*/
    im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly0 = 3;
	im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly1 = 6;
	im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly2 = 9;
	im_di_rtnr_y_16_th0_th3_reg.cp_temporalthly3 = 12;
	im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly4 = 15;
	im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly5 = 18;
	im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly6 = 21;
	im_di_rtnr_y_16_th4_th7_reg.cp_temporalthly7 = 24;
	im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly8 = 27;
	im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly9 = 30;
	im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly10 = 33;
	im_di_rtnr_y_16_th8_th11_reg.cp_temporalthly11 = 36;
	im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly12 = 39;
	im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly13 = 42;
	im_di_rtnr_y_16_th12_th14_reg.cp_temporalthly14 = 45;

	IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH0_TH3_reg, im_di_rtnr_y_16_th0_th3_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH4_TH7_reg, im_di_rtnr_y_16_th4_th7_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH8_TH11_reg, im_di_rtnr_y_16_th8_th11_reg.regValue);
	IoReg_Write32(DI_IM_DI_RTNR_Y_16_TH12_TH14_reg, im_di_rtnr_y_16_th12_th14_reg.regValue);

  }
  else{
    rtd_pr_vpq_info("get_vo_camera_flow_flag : OFF\n");

	arg = Scaler_APDEM_Arg_Access(DEM_ARG_Color_Space, 0, 0);
	if (arg == Color_Space_Target_Auto)
		idx_temp = fwif_color_get_APDEM_CM_IDX();
	else
		idx_temp = arg;
	Scaler_Set_ColorMapping_3x3_Table(idx_temp);
	rtd_pr_vpq_info("DEM_arg_Color_Space in camera:%d\n", arg, idx_temp);
	
	if (arg == Color_Space_Target_OFF) {
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
		drvif_color_gamma_enable_enhance(SLR_MAIN_DISPLAY, 0);
	} else {
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 1, 0);
		drvif_color_gamma_enable_enhance(SLR_MAIN_DISPLAY, 1);
	}
	
    /* set DCC */
    Scaler_SetDCC_Mode(aipq_DynamicContrastLevel);    
    /* set LC */
    if( drvif_color_get_LC_size_support()!=0 )
		Scaler_SetLocalContrastEnable(1);
	else
		Scaler_SetLocalContrastEnable(0);
    /* set YUV2RGB_MATRIX */
	d_yuv2rgb_control_REG.yuv2rgb_main_en = Ori_YUV2RGB_Enable_Main;
    IoReg_Write32(YUV2RGB_D_YUV2RGB_Control_reg, d_yuv2rgb_control_REG.regValue);
    /* set SNR_Y */ 
    dch1_cp_ctrl_reg.cp_spatialenabley = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(Ori_SNR_spatialenabley);
	IoReg_Write32(NR_DCH1_CP_Ctrl_reg, dch1_cp_ctrl_reg.regValue);
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_apply_mask); /* DB apply */
    /* set De_Contour */ 
    drvif_color_Set_I_De_Contour_DB_En( 1 );
	ipq_decontour_de_contour_ctrl_reg.decont_en=(Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(Ori_I_De_Contour_en);
    IoReg_Write32(IPQ_DECONTOUR_De_Contour_CTRL_reg, ipq_decontour_de_contour_ctrl_reg.regValue);
    drvif_color_Set_I_De_Contour_DB_Apply();
    /* set block-decontour */ 
    blk_de_contour_ctrl_reg.bdecont_en=Ori_block_decontour_en;
    IoReg_Write32(IPQ_DECONTOUR_BLK_De_Contour_CTRL_reg, blk_de_contour_ctrl_reg.regValue);
    /* set cp_mpegresultweight_y */ 
    nr_dch1_mpegnr1_reg.cp_mpegresultweight_y = Ori_MPEGV_mpegresultweight_y;
    IoReg_Write32(NR_DCH1_MPEGNR1_reg	,	nr_dch1_mpegnr1_reg	.regValue);
    IoReg_SetBits(NR_V_MPEGNR_DB_CTRL_reg, NR_V_MPEGNR_DB_CTRL_mpegnr_db_apply_mask); // DB apply
    /* set cp_mpegresultweight_x */ 
    mpegnr_ich1_mpegnr2_reg.cp_mpegresultweight_x =Ori_MPEGH_mpegresultweight_x;
    IoReg_Write32(MPEGNR_ICH1_MPEGNR2_reg	,	mpegnr_ich1_mpegnr2_reg	.regValue);
    IoReg_SetBits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_apply_mask);// DB apply
    /* set rtnr_ma_snr_en */
    di_rtnr_ma_snr_control.rtnr_ma_snr_en = Ori_rtnr_ma_snr_en;
    IoReg_Write32(DI_RTNR_MA_SNR_CONTROL_reg, di_rtnr_ma_snr_control.regValue);
    /* set Ori_MBPK camera flag setting*/
	//Enable doublue buffer
	color_mb_peaking_mb_double_buffer_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg);
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_en =1;
	IoReg_Write32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, color_mb_peaking_mb_double_buffer_ctrl.regValue);
	//ROW2 VIP_MBPK_H_Table
	color_mb_peaking_mb_peaking_hor_gain.mb_gain_neg = Ori_MBPK_H_gain_neg;
	color_mb_peaking_mb_peaking_hor_gain.mb_gain_pos = Ori_MBPK_H_gain_pos;
	//ROW4 VIP_MBPK_V_Table
	color_mb_peaking_mb_peaking_ver_gain.mb_gain_neg_v = Ori_MBPK_V_gain_neg;
	color_mb_peaking_mb_peaking_ver_gain.mb_gain_pos_v = Ori_MBPK_V_gain_pos;
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Hor_GAIN_reg, color_mb_peaking_mb_peaking_hor_gain.regValue);
	IoReg_Write32(COLOR_MB_PEAKING_MB_PEAKING_Ver_GAIN_reg, color_mb_peaking_mb_peaking_ver_gain.regValue);
	//Enable doublue buffer
	color_mb_peaking_mb_double_buffer_ctrl.regValue = IoReg_Read32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg);
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_en =1;
	color_mb_peaking_mb_double_buffer_ctrl.mb_db_apply =1;
	IoReg_Write32(COLOR_MB_PEAKING_MB_Double_Buffer_CTRL_reg, color_mb_peaking_mb_double_buffer_ctrl.regValue);
  }
}

#if 1 // lesley debug dump
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
#define FS_ACCESS_API_SUPPORTED
#endif

#ifdef FS_ACCESS_API_SUPPORTED
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

	ret = kernel_read(file, data, size, &offset);

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
#endif

void Scaler_color_write_data_to_bin_file(unsigned char *data, unsigned int size, unsigned char* ouputfilename)
{
#ifdef FS_ACCESS_API_SUPPORTED
	struct file *fPtr;

	fPtr = file_open(ouputfilename, O_RDWR | O_CREAT | O_APPEND, 0);
	if (fPtr == NULL) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[demura], write_data_to_bin_file, can't oopen fp\n");
		return;
	}
	file_write(fPtr, 0, data, sizeof(char) * size);
	file_sync(fPtr);
	file_close(fPtr);
#endif
}

void Scaler_color_read_data_from_bin_file(unsigned char *data, unsigned int size, unsigned char* sourcefile)
{
#ifdef FS_ACCESS_API_SUPPORTED
	struct file *fPtr;

	fPtr = file_open(sourcefile, O_RDWR | O_APPEND, 0);
	if (fPtr == NULL) {
		rtd_printk(KERN_EMERG, TAG_NAME, "[demura], write_data_to_bin_file, can't oopen fp\n");
		return;
	}
	file_read(fPtr, 0, data, sizeof(char) * size);
	file_close(fPtr);
#endif
}


#endif
#endif //UT_flag
