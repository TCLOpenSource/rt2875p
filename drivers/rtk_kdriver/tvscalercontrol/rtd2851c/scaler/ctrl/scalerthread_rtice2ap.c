
/*******************************************************************************
* @file    scalerColorLib.cpp
* @brief
* @note    Copyright (c) 2014 RealTek Technology Co., Ltd.
*          All rights reserved.
*          No. 2, Innovation Road II,
*	       Hsinchu Science Park,
*          Hsinchu 300, Taiwan
*
* @log
* Revision 0.1  2014/01/27
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
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>
#include <linux/kthread.h>  /* for threads*/
#include <linux/time.h>   /* for using jiffies*/
#else
#include <no_os/slab.h>
#include <no_os/pageremap.h>
#include <include/string.h>
#include <timer.h>
#endif

#ifdef CONFIG_ARM64
#include <asm/io.h>
#endif

#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv002.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/scaler/scalerthread_rtice2ap.h>
#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>
#include <scaler/scalerDrvCommon.h>
#include <scaler/scalerCommon.h>

#ifndef BUILD_QUICK_SHOW
#include <tvscalercontrol/vdc/video_factory.h>
#include <tvscalercontrol/vdc/yc_separation_vpq.h>
#include <tvscalercontrol/vdc/video.h>
#include <vgip_isr/scalerAI.h>
#endif

#include "tvscalercontrol/vip/HDR10_vivid.h"

#include "tvscalercontrol/vip/localcontrast.h"
#include "tvscalercontrol/vip/icm.h"
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <scaler_vbedev.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/tc_hdr.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"
#include <tvscalercontrol/vip/st2094.h>

#include <tvscalercontrol/vip/ai_pq.h>
#include <rtk_kdriver/rtk_pwm.h>


/*#include "../../../rtice2ap_driver.h"*/

extern unsigned short OETF_curve_ST2094[1025];	/* in ST2094.c*/
extern unsigned int knee_curve_ST2094[1025];	/* in ST2094.c*/
extern VIP_DEM_TBL VIP_AP_DEM_TBL;
#ifndef BUILD_QUICK_SHOW
//#define printf(fmt, args...)	VIPprintf(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_ScalerCtrl_Rtice2ap_DEBUG,0,fmt,##args)
#endif

unsigned char gVipDebugLogCtrls[256] = {//Debug log controls for VIPprintf
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
unsigned int gDebugPrintDelayTime = 1;//Interval count for controling print freq of VIPprintf

#define SCALERLIB_DEBUG

#ifdef SCALERLIB_DEBUG
  #define SLRLIB_PRINTF(format, args...) 		printf(format, ##args)
#else
  #define SLRLIB_PRINTF(format, args...)
#endif

#define ICM_TBL_Buff_Num 2
static signed char New_idx = -1;
static unsigned int* tICM_buff[ICM_TBL_Buff_Num] = {NULL};

#define Rtice_complement2_Y2R(arg) (((~arg) & 0x7FF)+1)
#define ISR_PRINT_CTRL_ITEM_NUMBER (sizeof(VIP_ISR_Printf_Flag)/sizeof(int))
#define BP_FUNCTION_CTRL_ITEM_NUMBER (sizeof(VIP_BP_CTRL)/sizeof(short))
#define BP_FUNCTION_TBL_ITEM_NUMBER (sizeof(VIP_BP_TBL)/sizeof(short))	/* only get 1 table*/
#define ADV_API_CTRL_ITEM_NUMBER (sizeof(VIP_ADV_API_CTRL)/sizeof(char))
#define ADV_API_TBL_ITEM_NUMBER (sizeof(VIP_ADV_API_TBL)/sizeof(char))	/* only get 1 table*/
#define DBC_STATUS_ITEM_NUMBER (sizeof(VIP_DBC_STATUS)/sizeof(char))
#define DBC_SW_REG_ITEM_NUMBER (sizeof(VIP_DBC_SW_REG)/sizeof(char))	/* only get 1 table*/

#define Debug_HDR_ITEM_NUMBER 10

#define VIP_System_Info_Items_Num 44
#define VO_Info_Items_Num 50
#define HDMI_AVI_Info_Frame_Items_Num 33
#define HDMI_DRM_Info_Items_Num 17

#define ST2094_METADATA_Curve_Info_Max 42
#define ST2094_Basis_OOTF_Info_Max 20
#define ST2094_Guided_OOTF_Info_Max 17
#define ST2094_OETF_CTRL_Info_Max 10
#define ST2094_OETF_Curve_Max 2050

unsigned char HDR_3DLUT_getFromReg = 0;
unsigned short HDR10_3D_LUT_17x17x17_temp[VIP_DM_HDR_3D_LUT_UI_TBL_SIZE];

int rtice_set_IdxTable2Buff(unsigned short mode, unsigned short size, unsigned char *num_type, unsigned char *buf);	/*return idx table size*/
int rtice_SetGet_PQA_Table_Info(VIP_TOOL_ACCESS_INFO_Item mode, unsigned char isSet_Flag, unsigned short size, unsigned char *num_type, unsigned char *buf);	/*return idx table size*/
int rtice_SetGet_YUV2RGB_Table_Info(VIP_TOOL_ACCESS_INFO_Item mode, unsigned char isSet_Flag, unsigned short size,
	unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, SLR_VIP_TABLE *vipTable_ShareMem);	/*return idx table size*/
int rtice_Set_YUV2RGB_Sat(unsigned short satGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);
int rtice_Set_YUV2RGB_Hue(unsigned short hueGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);
int rtice_Set_YUV2RGB_Contrast(unsigned short ContrastGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);

extern UINT8 VipPQ_GetNewDctiTableSize(void);
extern UINT8 VipPQ_GetDltiTableSize(void);
extern UINT8 VipPQ_GetINewDctiTableSize(void);
extern UINT8 VipPQ_GetVctiTableSize(void);
extern UINT8 VipPQ_GetRTNR_NM_TableSize(void);
extern UINT8 VipPQ_GetHME_TableSize(void);
extern UINT8 VipPQ_GetHMC_TableSize(void);
extern UINT8 VipPQ_GetPAN_TableSize(void);
extern UINT8 VipPQ_GetSMD_TableSize(void);
extern UINT8 VipPQ_GetTNRXC_TableSize(void);
extern UINT8 VipPQ_GetEmfMk2_TableSize(void);
extern UINT8 VipPQ_GetDnoise_Level_TableSize(void);
extern UINT8 VipPQ_GetDnoise_Coef_TableSize(void);
extern UINT8 VipPQ_GetAdaptive_Gamma_Ctrl_TableSize(void);
extern UINT8 VipPQ_GetLocal_Dimming_TableSize(void);
extern UINT8 VipPQ_GetFIR_Coef_Ctrl_TableSize(void);
extern UINT8 VipPQ_GetSupk_Mask_TableSize(void);
extern UINT8 VipPQ_GetUn_Shp_Mask_TableSize(void);
extern UINT8 VipPQ_GetScaleDown_COEF_TableSize(void);
extern UINT8 VipPQ_GetSU_table_12tap_TableSize(void);
extern UINT8 VipPQ_GetSU_table_8tap_TableSize(void);
extern UINT8 VipPQ_GetDirsu_TableSize(void);

extern short gColorGamutMatrix_Apply[3][3];
extern unsigned short tYUV2RGB_COEF[YUV2RGB_INPUT_MODE_MAX][tUV2RGB_COEF_Items_Max]; /* get YUV2RGB matrix from misc, elieli*/

#define	vip_malloc(x)	kmalloc(x, GFP_KERNEL)
#define	vip_free(x)	kfree(x)

#if 1/*RTICE2AP_DEBUG_TOOL_ENABLE*/
#define ID_String_Max 50
enum {
	READ_AP = 1,
	WRITE_AP = 2,
	READ_ERROR = 3,
	WRITE_ERROR = 4,
	WRITE_SUCCEED = 5,
	ID_SIZE_ERR = 6,
};
typedef enum _VIP_TABLE_TYPE{
	VIP_UINT8 = 1, /*include unsigned char */
	VIP_INT8 = 2,				/*include char*/
	VIP_UINT16 = 3,			/*include unsigned short*/
	VIP_INT16 = 4,			/*include short*/
	VIP_UINT32 = 5,			/*include unsigned int, unsigned long*/
	VIP_INT32 = 6,			/*include int, long*/
	VIP_UINT64 = 7,			/*include unsigned long long*/
	VIP_INT64 = 8,  		/*include long long*/
	VIP_FLOAT = 9,
	VIP_DOUBLE = 10,
} VIP_TABLE_TYPE;
/*
typedef struct{
	int dHue;
	int dSatBySat[12];
	int dItnByItn[12];
}ICM_Global_Ctrl;
*/
typedef struct _RTICE2AP_MemIdx_Table{
	unsigned short ID;
	unsigned char Type;
	unsigned int MaxNum;
	unsigned char ID_String[ID_String_Max];
} RTICE2AP_MemIdx_Table;
/*TOOLACCESS_MAX*/
RTICE2AP_MemIdx_Table RTICE2AP_MemIdx_Table_ini[TOOLACCESS_MAX] = {
/* ID										Type		Num	Max																		String Name*/
/*Basic OSD Control Item*/
{TOOLACCESS_Contrast,						VIP_UINT8,	1,																			"Contrast"							}, /**/
{TOOLACCESS_Brightness,						VIP_UINT8,	1,																			"Brightness"						}, /**/
{TOOLACCESS_Saturation,						VIP_UINT8,	1,																			"Saturation"						}, /**/
{TOOLACCESS_Hue,						VIP_UINT8,	1,																			"Hue"								}, /**/
/*Eng. menu item*/

{TOOLACCESS_Local_Contrast,					VIP_UINT8,	1,																			"Local_Contrast"						}, /**/
{TOOLACCESS_Dynamic_Contrast,					VIP_UINT8,	2,																			"Dynamic_Contrast"						}, /**/
{TOOLACCESS_VD_Contrast,					VIP_UINT8,	1,																			"VD_Contrast"						}, /**/
{TOOLACCESS_VD_Brightness,					VIP_UINT8,	1,																			"VD_Brightness"						}, /**/
{TOOLACCESS_VD_Saturation,					VIP_UINT8,	1,																			"VD_Saturation"						}, /**/
{TOOLACCESS_VD_Hue,						VIP_UINT8,	1,																			"VD_Hue"							}, /**/
{TOOLACCESS_Gamma,						VIP_UINT8,	1,																			"Gamma"								}, /**/
{TOOLACCESS_DCTI,						VIP_UINT8,	1,																			"DCTI"								}, /**/
{TOOLACCESS_DLTI,						VIP_UINT8,	1,																			"DLTI"								}, /**/
{TOOLACCESS_MADI_HMC,						VIP_UINT8,	1,																			"MADI_HMC"							}, /**/
{TOOLACCESS_MADI_HME,						VIP_UINT8,	1,																			"MADI_HME"							}, /**/
{TOOLACCESS_MADI_PAN,						VIP_UINT8,	1,																			"MADI_PAN"							}, /**/
{TOOLACCESS_DI_MA_Adjust_Table,					VIP_UINT8,	1,																			"DI_MA_Adjust_Table"				}, /**/
{TOOLACCESS_DI_DiSmd,						VIP_UINT8,	1,																			"DI_DiSmd"							}, /**/
{TOOLACCESS_TNRXC_Ctrl,						VIP_UINT8,	1,																			"TNRXC_Ctrl"						}, /**/
{TOOLACCESS_TNRXC_MK2,						VIP_UINT8,	1,																			"TNRXC_MK2"							}, /**/
{TOOLACCESS_ScaleUPH,						VIP_UINT8,	1,																			"ScaleUPH"							}, /**/
{TOOLACCESS_ScaleUPH_12tap,					VIP_UINT8,	1,																			"ScaleUPH_12tap"						}, /**/
{TOOLACCESS_ScaleUPV_8tap,					VIP_UINT8,	1,																			"ScaleUPV_8tap"							}, /**/
{TOOLACCESS_ScaleUPV,						VIP_UINT8,	1,																			"ScaleUPV"							}, /**/
{TOOLACCESS_ScaleUPDir,						VIP_UINT8,	1,																			"ScaleUPDir"						}, /**/
{TOOLACCESS_ScaleUPDir_Weighting,				VIP_UINT8,	1,																			"ScaleUPDir_Weighting"				}, /**/
{TOOLACCESS_ScaleDOWN_H_Table,					VIP_UINT8,	1,																			"ScaleDOWN_H_Table"					}, /**/
{TOOLACCESS_ScaleDOWN_V_Table,					VIP_UINT8,	1,																			"ScaleDOWN_V_Table"					}, /**/
{TOOLACCESS_ScaleDOWN_444To422,					VIP_UINT8,	1,																			"ScaleDOWN_444To422"				}, /**/
{TOOLACCESS_D_UVDelay_en,					VIP_UINT8,	1,																			"D_UVDelay_en" 						}, /**/
{TOOLACCESS_D_UVDelay,						VIP_UINT8,	1,																			"D_UVDelay" 						}, /**/


{TOOLACCESS_Pattern_Off,					VIP_UINT8,	1,																			"Pattern_Off"						}, /**/
{TOOLACCESS_Pattern_Red,					VIP_UINT8,	1,																			"Pattern_Red"						}, /**/
{TOOLACCESS_Pattern_Green,					VIP_UINT8,	1,																			"Pattern_Green"						}, /**/
{TOOLACCESS_Pattern_Blue,					VIP_UINT8,	1,																			"Pattern_Blue"						}, /**/
{TOOLACCESS_Pattern_White,					VIP_UINT8,	1,																			"Pattern_White"						}, /**/
{TOOLACCESS_Pattern_Black,					VIP_UINT8,	1,																			"Pattern_Black"						}, /**/
{TOOLACCESS_Pattern_Color_Bar,					VIP_UINT8,	1,																			"Pattern_Color_Bar"					}, /**/
{TOOLACCESS_Pattern_Gray_Bar,					VIP_UINT8,	1,																			"Pattern_Gray_Bar"					}, /**/
{TOOLACCESS_Magic_Off,						VIP_UINT8,	1,																			"Magic_Off"							}, /**/
{TOOLACCESS_SetMagic_Still_DEMO,				VIP_UINT8,	1,																			"SetMagic_Still_DEMO"				}, /**/
{TOOLACCESS_SetMagic_Still_DEMO_Inverse,			VIP_UINT8,	1,																			"SetMagic_Still_DEMO_Inverse"		}, /**/
{TOOLACCESS_SetMagic_Dynamic_DEMO,				VIP_UINT8,	1,																			"SetMagic_Dynamic_DEMO"				}, /**/
{TOOLACCESS_SetMagic_Move,					VIP_UINT8,	1,																			"SetMagic_Move"						}, /**/
{TOOLACCESS_SetMagic_Move_Inverse,				VIP_UINT8,	1,																			"SetMagic_Move_Inverse"				}, /**/
{TOOLACCESS_SetMagic_Zoom,					VIP_UINT8,	1,																			"SetMagic_Zoom"						}, /**/
{TOOLACCESS_SetMagic_Optimize,					VIP_UINT8,	1,																			"SetMagic_Optimize"					}, /**/
{TOOLACCESS_SetMagic_Enhance,					VIP_UINT8,	1,																			"SetMagic_Enhance"					}, /**/
{TOOLACCESS_VD_SetYC_Separation,				VIP_UINT8,	1,																			"VD_SetYC_Separation"				}, /**/
{TOOLACCESS_VD_Set3D_Table,					VIP_UINT8,	1,																			"VD_Set3D_Table"					}, /**/
{TOOLACCESS_VD_Set2D_Chroma_BW_Low,				VIP_UINT8,	1,																			"VD_Set2D_Chroma_BW_Low"			}, /**/
{TOOLACCESS_VD_SetSetDCTI_Table,				VIP_UINT8,	1,																			"VD_SetSetDCTI_Table"				}, /**/
{TOOLACCESS_VD_Set2D_Table,					VIP_UINT8,	1,																			"VD_Set2D_Table"					}, /**/
{TOOLACCESS_VD_SetNarrow_BPF_Y,					VIP_UINT8,	1,																			"VD_SetNarrow_BPF_Y"				}, /**/
{TOOLACCESS_VD_SetWide_BPF_Y,					VIP_UINT8,	1,																			"VD_SetWide_BPF_Y"					}, /**/
{TOOLACCESS_VD_SetWide_BPF_C,					VIP_UINT8,	1,																			"VD_SetWide_BPF_C"					}, /**/
{TOOLACCESS_VD_SetNarrow_BPF_C,					VIP_UINT8,	1,																			"VD_SetNarrow_BPF_C"				}, /**/
{TOOLACCESS_VD_SetVflag_Remg_Thr,				VIP_UINT8,	1,																			"VD_SetVflag_Remg_Thr"				}, /**/
{TOOLACCESS_VD_SetVflag_Mag_Thr,				VIP_UINT8,	1,																			"VD_SetVflag_Mag_Thr"				}, /**/
{TOOLACCESS_VD_SetY_Noise_Thr,					VIP_UINT8,	1,																			"VD_SetY_Noise_Thr"					}, /**/
{TOOLACCESS_VD_SetNoise_Max_Hdy,				VIP_UINT8,	1,																			"VD_SetNoise_Max_Hdy"				}, /**/
{TOOLACCESS_VD_SetNoise_Y_Add_DC,				VIP_UINT8,	1,																			"VD_SetNoise_Y_Add_DC"				}, /**/
{TOOLACCESS_VD_SetBlend_Ratio,					VIP_UINT8,	1,																			"VD_SetBlend_Ratio"					}, /**/
{TOOLACCESS_VD_SetTDma_Enable,					VIP_UINT8,	1,																			"VD_SetTDma_Enable"					}, /**/
{TOOLACCESS_VD_SetTDma_Mode,					VIP_UINT8,	1,																			"VD_SetTDma_Mode"					}, /**/
{TOOLACCESS_VD_SetDeb_Print,					VIP_UINT8,	1,																			"VD_SetDeb_Print"					}, /**/
{TOOLACCESS_VD_Set625_Mode,					VIP_UINT8,	1,																			"VD_Set625_Mode"					}, /**/
{TOOLACCESS_VD_SetMV_Check,					VIP_UINT8,	1,																			"VD_SetMV_Check"					}, /**/
{TOOLACCESS_VD_SetClamp_Mode,					VIP_UINT8,	1,																			"VD_SetClamp_Mode"					}, /**/
{TOOLACCESS_VD_SetRc_Rate_Func,					VIP_UINT8,	1,																			"VD_SetRc_Rate_Func"				}, /**/
{TOOLACCESS_VD_SetRc_Rate_Value,				VIP_UINT8,	1,																			"VD_SetRc_Rate_Value"				}, /**/
{TOOLACCESS_VD_SetV_Clamp,					VIP_UINT8,	1,																			"VD_SetV_Clamp"						}, /**/
{TOOLACCESS_VD_SetClamp_Delay,					VIP_UINT8,	1,																			"VD_SetClamp_Delay"					}, /**/
{TOOLACCESS_VD_SetClamp_Delay_Value,				VIP_UINT8,	1,																			"VD_SetClamp_Delay_Value"			}, /**/
{TOOLACCESS_VD_SetSetH_State_Write,				VIP_UINT8,	1,																			"VD_SetSetH_State_Write"			}, /**/
{TOOLACCESS_VD_SetHsync_Start,					VIP_UINT8,	1,																			"VD_SetHsync_Start"					}, /**/
{TOOLACCESS_VD_SetV_State_Write,				VIP_UINT8,	1,																			"VD_SetV_State_Write"				}, /**/
{TOOLACCESS_VD_SetV_Detect_Mode,				VIP_UINT8,	1,																			"VD_SetV_Detect_Mode"				}, /**/
{TOOLACCESS_VD_SetVsync_Ctrl,					VIP_UINT8,	1,																			"VD_SetVsync_Ctrl"					}, /**/
{TOOLACCESS_VD_SetC_State_W,					VIP_UINT8,	1,																			"VD_SetC_State_W"					}, /**/
{TOOLACCESS_VD_SetAuto_BP,					VIP_UINT8,	1,																			"VD_SetAuto_BP"						}, /**/
{TOOLACCESS_VD_SetCkill_Mode,					VIP_UINT8,	1,																			"VD_SetCkill_Mode"					}, /**/
{TOOLACCESS_VD_SetCkill_Value,					VIP_UINT8,	1,																			"VD_SetCkill_Value"					}, /**/
{TOOLACCESS_VD_SetSet_Dgain,					VIP_UINT8,	1,																			"VD_SetSet_Dgain"					}, /**/
{TOOLACCESS_VD_SetFix_Dgain,					VIP_UINT8,	1,																			"VD_SetFix_Dgain"					}, /**/
{TOOLACCESS_VD_SetSet_Cgain,					VIP_UINT8,	1,																			"VD_SetSet_Cgain"					}, /**/
{TOOLACCESS_VD_SetFix_Cgain,					VIP_UINT8,	1,																			"VD_SetFix_Cgain"					}, /**/
{TOOLACCESS_VD_Set27M_LPF,					VIP_UINT8,	1,																			"VD_Set27M_LPF"						}, /**/
{TOOLACCESS_VD_Set27M_Table,					VIP_UINT8,	1,																			"VD_Set27M_Table"					}, /**/
{TOOLACCESS_VD_Set108M_LPF,					VIP_UINT8,	1,																			"VD_Set108M_LPF"					}, /**/
{TOOLACCESS_VD_SetSecam_F_sel,					VIP_UINT8,	1,																			"VD_SetSecam_F_sel"					}, /**/
{TOOLACCESS_VD_SetSecam2,					VIP_UINT8,	1,																			"VD_SetSecam2"						}, /**/
{TOOLACCESS_VD_Set443358pk_sel,					VIP_UINT8,	1,																			"VD_Set443358pk_sel"				}, /**/
{TOOLACCESS_VD_Set443thr,					VIP_UINT8,	1,																			"VD_Set443thr"						}, /**/
{TOOLACCESS_VD_Set358thr,					VIP_UINT8,	1,																			"VD_Set358thr"						}, /**/
{TOOLACCESS_VD_SetSoft_Reset,					VIP_UINT8,	1,																			"VD_SetSoft_Reset"					}, /**/
{TOOLACCESS_VD_PQ_SetBypassPQ,					VIP_UINT8,	1,																			"VD_PQ_SetBypassPQ"					}, /**/
{TOOLACCESS_VD_PQ_Set1D3D_Pos,					VIP_UINT8,	1,																			"VD_PQ_Set1D3D_Pos"					}, /**/
{TOOLACCESS_VD_PQ_SetContrast,					VIP_UINT8,	1,																			"VD_PQ_SetContrast"					}, /**/
{TOOLACCESS_VD_PQ_SetBrightness,				VIP_UINT8,	1,																			"VD_PQ_SetBrightness"				}, /**/
{TOOLACCESS_VD_PQ_SetSaturation,				VIP_UINT8,	1,																			"VD_PQ_SetSaturation"				}, /**/
{TOOLACCESS_VD_PQ_SetYC_Delay,					VIP_UINT8,	1,																			"VD_PQ_SetYC_Delay"					}, /**/
{TOOLACCESS_VD_PQ_SetY1delay_En,				VIP_UINT8,	1,																			"VD_PQ_SetY1delay_En"				}, /**/
{TOOLACCESS_VD_PQ_SetCb1delay_En,				VIP_UINT8,	1,																			"VD_PQ_SetCb1delay_En"				}, /**/
{TOOLACCESS_VD_PQ_SetCr1delay_En,				VIP_UINT8,	1,																			"VD_PQ_SetCr1delay_En"				}, /**/
{TOOLACCESS_VD_PQ_SetY1delay,					VIP_UINT8,	1,																			"VD_PQ_SetY1delay"					}, /**/
{TOOLACCESS_VD_PQ_SetCb1delay,					VIP_UINT8,	1,																			"VD_PQ_SetCb1delay"					}, /**/
{TOOLACCESS_VD_PQ_SetCr1delay,					VIP_UINT8,	1,																			"VD_PQ_SetCr1delay"					}, /**/
{TOOLACCESS_VD_PQ_SetKill_Ydelay_En,				VIP_UINT8,	1,																			"VD_PQ_SetKill_Ydelay_En"			}, /**/
{TOOLACCESS_VD_PQ_SetKill_Ydelay,				VIP_UINT8,	1,																			"VD_PQ_SetKill_Ydelay"				}, /**/
{TOOLACCESS_VD_PQ_SetSCART_Con,					VIP_UINT8,	1,																			"VD_PQ_SetSCART_Con"				}, /**/
{TOOLACCESS_VD_PQ_SetSCART_Bri,					VIP_UINT8,	1,																			"VD_PQ_SetSCART_Bri"				}, /**/
{TOOLACCESS_VD_PQ_SetSCART_Sat_cr,				VIP_UINT8,	1,																			"VD_PQ_SetSCART_Sat_cr"				}, /**/
{TOOLACCESS_VD_PQ_SetSCART_Sat_cb,				VIP_UINT8,	1,																			"VD_PQ_SetSCART_Sat_cb"				}, /**/
{TOOLACCESS_VD_PQ_Style,						VIP_UINT8,	1,																			"VD_PQ_Style"				},

{TOOLACCESS_DCC_PQ_current_mean,				VIP_UINT32,	1,																			"DCC_PQ_current_mean" 			}, /* read only*/
{TOOLACCESS_DCC_PQ_current_dev,					VIP_UINT32,	1,																			"DCC_PQ_current_dev"			}, /* read only*/
{TOOLACCESS_DCC_PQ_Level,					VIP_UINT8,	1,																			"DCC_PQ_Level"					}, /**/
{TOOLACCESS_DCC_PQ_Cubic_index,					VIP_UINT8,	1,																			"DCC_PQ_Cubic_index" 			}, /**/
{TOOLACCESS_DCC_PQ_Cubic_low, 					VIP_UINT8,	1,																			"DCC_PQ_Cubic_low" 				}, /**/
{TOOLACCESS_DCC_PQ_Cubic_high,					VIP_UINT8,	1,																			"DCC_PQ_Cubic_high" 			}, /**/


/*{TOOLACCESS_DisplayInformation_table,			VIP_UINT32,	DisplayInfo_Coef_MAX,														"DisplayInformation_table"			}, */ /* 19*/
{TOOLACCESS_VIP_QUALITY_Coef,          			VIP_UINT8,	VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK,					"VIP_QUALITY_Coef"          		}, /* 20*/
{TOOLACCESS_VIP_QUALITY_Extend_Coef,   			VIP_UINT8,	VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK,					"VIP_QUALITY_Extend_Coef"   		}, /* 21*/
{TOOLACCESS_VIP_QUALITY_Extend2_Coef,  			VIP_UINT8,	VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK,					"VIP_QUALITY_Extend2_Coef"  		}, /* 22*/
{TOOLACCESS_VIP_QUALITY_Extend3_Coef,  			VIP_UINT8,	VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK,					"VIP_QUALITY_Extend3_Coef"  		}, /* 23*/
{TOOLACCESS_tICM_ini,				  	VIP_UINT16,	VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z,									"tICM_ini"				  			}, /* 24*/
{TOOLACCESS_SHPTable,			  		VIP_INT16,	VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK,					"SHPTable"			  				}, /* 25*/
{TOOLACCESS_Hist_Y_Mean_Value,			 	VIP_UINT32,	1,																			"Hist_Y_Mean_Value"			 		}, /* 26*/
{TOOLACCESS_SmartPic_clue_Motion_Cal,			VIP_UINT32,	14,																			"SmartPic_clue_Motion_Cal"			}, /* 32*/
{TOOLACCESS_SmartPic_clue_HSI_Histogram_Cal,		VIP_UINT32,	91,																			"SmartPic_clue_HSI_Histogram_Cal"	}, /* 33*/
{TOOLACCESS_SmartPic_clue_VD_Noise_Cal,			VIP_UINT32,	2,																			"SmartPic_clue_VD_Noise_Cal"		}, /* 34*/

{TOOLACCESS_TABLE_NTSC_3D,			  	VIP_UINT8,	VD_3D_TABLE_SIZE*VD_MOTION_STEP*2,											"TABLE_NTSC_3D"			  			}, /* 28*/
{TOOLACCESS_MANUAL_NR,			 		VIP_UINT8,	PQA_TABLE_MAX*DRV_NR_Level_MAX,												"MANUAL_NR"			 				}, /* 29*/
/*{TOOLACCESS_PQA_Input_Table,			  	VIP_UINT32,	PQA_I_TABLE_MAX*PQA_I_ITEM_MAX*PQA_I_LEVEL_MAX,								"PQA_Input_Table"			  		}, */ /* 30*/
/*{TOOLACCESS_PQA_Table,			  				VIP_UINT32,	PQA_TABLE_MAX*PQA_MODE_MAX*PQA_ITEM_MAX*PQA_LEVEL_MAX,						"PQA_Table"			  				}, */ /* 31*/
{TOOLACCESS_ISR_Print_Ctrl,				VIP_UINT32,	ISR_PRINT_CTRL_ITEM_NUMBER,													"ISR_Print_Ctrl" 					}, /*32*/
{TOOLACCESS_BP_Function_CTRL, 				VIP_UINT16,	BP_FUNCTION_CTRL_ITEM_NUMBER, 												"Brightness_PLUS_CTRL"				},/*3*/
{TOOLACCESS_BP_Function_TBL,				VIP_UINT16, 	BP_FUNCTION_TBL_ITEM_NUMBER,																		"Brightness_PLUS_TBL"				},
{TOOLACCESS_xvYcc_mode, 				VIP_UINT8,  	1, 																			"xvYcc_mode"					    },
{TOOLACCESS_ADV_API_CTRL,				VIP_UINT8, 	ADV_API_CTRL_ITEM_NUMBER,																		"TOOLACCESS_ADV_API_CTRL"				},
{TOOLACCESS_ADV_API_TBL,				VIP_UINT8, 	ADV_API_TBL_ITEM_NUMBER,																		"TOOLACCESS_ADV_API_TBL"				},
{TOOLACCESS_DBC_STATUS,					VIP_UINT8, 	DBC_STATUS_ITEM_NUMBER,																		"TOOLACCESS_DBC_STATUS"				},
{TOOLACCESS_DBC_SW_REG,					VIP_UINT8, 	DBC_SW_REG_ITEM_NUMBER, 																	"TOOLACCESS_DBC_SW_REG"				},
{TOOLACCESS_ICM_7axis_adjust,				VIP_UINT8, 	7, 																	"TOOLACCESS_CM_7axis_adjust"				},
{TOOLACCESS_switch_DVIandHDMI, 				VIP_UINT8,  	1,																			"switch_DVIandHDMI"					},
{TOOLACCESS_AutoMA_API_CTRL,				VIP_UINT8,	31,																			"AutoMA_API_CTRL"			}, /* 41*/

{TOOLACCESS_Debug_Buff_8,					VIP_UINT8, Debug_Buff_Num, 																	"TOOLACCESS_Debug_Buff_8"				},
{TOOLACCESS_Debug_Buff_16,					VIP_UINT16, Debug_Buff_Num, 																	"TOOLACCESS_Debug_Buff_16"				},
{TOOLACCESS_Debug_Buff_32,					VIP_UINT32, Debug_Buff_Num, 																	"TOOLACCESS_Debug_Buff_32"				},
{TOOLACCESS_Debug_HDR,					VIP_UINT32, Debug_HDR_ITEM_NUMBER, 																	"TOOLACCESS_Debug_HDR"				},

};

/*static ICM_Global_Ctrl icm_global_ctrl;*/
int g_h_sta = 0,  g_h_end = 0,  g_s_sta = 0,  g_s_end = 0,  g_i_sta = 0,  g_i_end = 0;
unsigned char user_curve_1 = 0, user_curve_2 = 0;
unsigned short stopUpdateFrequencyOffset = 0;
#if 0/*RTICE2AP_DEBUG_TOOL_ENABLE*/
/*
enum
{
	READ_AP = 1,
	WRITE_AP = 2,
	READ_ERROR = 3,
	WRITE_ERROR = 4,
	WRITE_SUCCEED = 5,
	ID_SIZE_ERR = 6,
};
*/

extern void msleep(unsigned int msecs);
extern ssize_t rtice_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
extern ssize_t rtice_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);

#define B4TL(b0, b1, b2, b3)	(((unsigned long)b0 << 24) + ((unsigned long)b1 << 16) + ((unsigned long)b2 << 8) + ((unsigned long)b3))
#define	B2TUS(b0, b1)	(((unsigned short)b0 << 8) + ((unsigned short)b1))
#define	B2TS(b0, b1)	(((short)b0 << 8) + ((short)b1))
#define B4TI(b0, b1, b2, b3)	(((int)b0 << 24) + ((int)b1 << 16) + ((int)b2 << 8) + ((int)b3))
#define B4TUI(b0, b1, b2, b3)	(((unsigned int)b0 << 24) + ((unsigned int)b1 << 16) + ((unsigned int)b2 << 8) + ((unsigned int)b3))

static struct task_struct *thread_rtice2ap;

int thread_rtice2AP(void)
{
	const unsigned int param_len = 17;
	unsigned char cmd_buf[17] = {0}; /*cmd_buf[0:1]:ID;cmd_buf[2:5]:start addr;cmd_buf[6:9]:end addr;cmd_buf[10:13]:size;cmd_buf[14]:param0;cmd_buf[15]:param1;cmd_buf[16]:flag*/
	int fd = 0;
	unsigned char *data_buf = NULL;
	unsigned short id;
	unsigned long size;
	ap_data_param rtice_param;

	unsigned char cmd_flag;
	int count = 0;
	int ret;
	int i = 0;

	printf("\n\n\n--------------------------\n");
	printf("== enter thread_rtice2ap!\n");
	printf("----------------------------\n\n\n");

	/*
	if ((fd = open("/dev/rtice2ap0", O_RDWR)) < 0) {
		printf("open rtice2ap0 failed!\n");
		return 0;
	}
	*/

	while (1) {
		/*cmd_buf:id[0] id[1] id[2] id[3] size[0] size[1] flag*/
		while ((count = rtice_read(fd, cmd_buf, param_len, 0)) != param_len) {   /*if fread not read date, it will be blocked*/
			printf("[rtice]read cmd error:count = %d!\n", count);
			msleep(10);
			/*usleep(100000);*/
		}
		/*id = B4TL(cmd_buf[0], cmd_buf[1], cmd_buf[2], cmd_buf[3]);*/
		rtice_param.id = B2TUS(cmd_buf[0], cmd_buf[1]);
		/*size = B4TL(cmd_buf[2], cmd_buf[3], cmd_buf[4], cmd_buf[5])-1;*/
		rtice_param.saddr = B4TL(cmd_buf[2], cmd_buf[3], cmd_buf[4], cmd_buf[5]);
		rtice_param.eaddr = B4TL(cmd_buf[6], cmd_buf[7], cmd_buf[8], cmd_buf[9]);
		size = B4TL(cmd_buf[10], cmd_buf[11], cmd_buf[12], cmd_buf[13]) ;
		rtice_param.mode0 = cmd_buf[14];
		rtice_param.mode1 = cmd_buf[15];

		cmd_flag = cmd_buf[16];
		#if 0
		if (data_buf != NULL) {
			free(data_buf);
			data_buf = NULL;
		}
		#endif
		if (((unsigned char *)vip_malloc(size * sizeof(char))) == NULL) {
			printf("[rtice]malloc error!\n");
			continue;
		}
		if (cmd_flag == READ_AP) {
			rtice_param.size = size - 1;
			ret = rtice_get_vip_table(rtice_param, &data_buf[0], &data_buf[1]);

			if (ret < 0) {
				printf("[rtice]get vip table error!\n");
				if (ret == -ID_SIZE_ERR) {
					cmd_buf[16] = ID_SIZE_ERR;
				} else {
					cmd_buf[16] = READ_ERROR;
				}
				if ((rtice_write(fd, cmd_buf, param_len, 0)) != param_len) {
					printf("[rtice]write status error!\n");
				}
				vip_free(data_buf);
				data_buf = NULL;
				continue;
			} else if ((rtice_write(fd, data_buf, size, 0)) != size) {
				printf("[rtice]read ap:write data error!\n");
			}
			vip_free(data_buf);
			data_buf = NULL;
		} else if (cmd_flag == WRITE_AP) {
			rtice_param.size = size;
			if ((rtice_read(fd, data_buf, size, 0)) != size) {
				printf("[rtcie]write ap:read data error!\n");
				vip_free(data_buf);
				data_buf = NULL;
				continue;
			}
			ret = rtice_set_vip_table(rtice_param, data_buf);
			if (ret == 0) {
				printf("[rtice]write vip table succeed!\n");
				cmd_buf[16] = WRITE_SUCCEED;
			} else if (ret == -ID_SIZE_ERR) {
				printf("[rtice]write vip table error!\n");
				cmd_buf[16] = ID_SIZE_ERR;
			} else {
				printf("[rtice]write vip table error!\n");
				cmd_buf[16] = WRITE_ERROR;
			}
			if ((rtice_write(fd, cmd_buf, 17, 0)) != 17) {
				printf("[rtice]write status error!\n");
			}
			vip_free(data_buf);
			data_buf = NULL;
		}
	}
	printf("exit!\n");
	/*close(fd);*/
	return 0;
}
/*pthread_t thread_id_rtice2AP;*/

void Scaler_color_ITB4(int nInt , unsigned char *b0, unsigned char *b1, unsigned char *b2, unsigned char *b3)
{
	unsigned char *pChar = (unsigned char *)(&nInt);
	*b0 = pChar[3];
	*b1 = pChar[2];
	*b2 = pChar[1];
	*b3 = pChar[0];
}
void Scaler_color_UITB4(unsigned int nUInt , unsigned char *b0, unsigned char *b1, unsigned char *b2, unsigned char *b3)
{
	unsigned char *pChar = (unsigned char *)(&nUInt);
	*b0 = pChar[3];
	*b1 = pChar[2];
	*b2 = pChar[1];
	*b3 = pChar[0];
}
void Scaler_color_STB2(short nShort , unsigned char *b0, unsigned char *b1)
{
	unsigned char *pChar = (unsigned char *)(&nShort);
	*b0 = pChar[1];
	*b1 = pChar[0];
}
void Scaler_color_USTB2(unsigned short nUShort , unsigned char *b0, unsigned char *b1)
{
	unsigned char *pChar = (unsigned char *)(&nUShort);
	*b0 = pChar[1];
	*b1 = pChar[0];
}
void rtice_init_thread(void)
{
	if (NULL == thread_rtice2ap)
		thread_rtice2ap = kthread_create(thread_rtice2AP, NULL, "rtice_thread");

	wake_up_process(thread_rtice2ap);
}

void rtice_uninit_thread(void)
{
	kthread_stop(thread_rtice2ap);
}
#endif

extern unsigned char vpq_ioctl_get_stop_run_by_idx(unsigned char cmd_idx);
extern void vpq_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);
extern unsigned char vpq_led_ioctl_get_stop_run_by_idx(unsigned char cmd_idx);
extern void vpq_led_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);
extern unsigned char vpq_memc_ioctl_get_stop_run_by_idx(unsigned char cmd_idx);
extern void vpq_memc_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop);
extern void scalerVIP_Set_pq_reset(unsigned char bEnable);

#if defined(CONFIG_RTK_AI_DRV)
extern RTK_AI_PQ_mode aipq_mode;
extern RTK_AI_PQ_mode aipq_mode_pre;
extern int scalerAI_pq_mode_ctrl(RTK_AI_PQ_mode ai_pq_mode, unsigned char dcValue);
#endif
#if 0
extern unsigned char VIP_Skip_DI_RTNR_FRZ_ISR;	// for tool control
extern unsigned char VIP_Skip_DI_RTNR_FRZ_TSK_P;	// for tool control
#endif

extern unsigned short panel_luminance;
extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_cur;
extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_cur;
extern VIP_APDEM_PTG_CTRL Hal_APDEM_PTG_CTRL;
// APDEM controll by tool, API only use in scalerthread_rtice.c
static int rtice_APDEM_PTG_set(VIP_APDEM_PTG_CTRL *pRTICE_PTG_CTRL) 
{	
	extern unsigned char rtk_hal_vsc_SetWinBlank(VIDEO_WID_T wid, unsigned char bonoff, KADP_VIDEO_DDI_WIN_COLOR_T color);
	extern unsigned char vsc_force_rerun_main_scaler;
	extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr;

	if (pRTICE_PTG_CTRL == NULL) {
		rtd_pr_vpq_emerg("%s,PTG_CTRL=NULL,\n", __func__);
		return -1;
	}
	// rtk_hal_vsc_SetWinBlank and re-run scaler can controlled by AP, in tool need to controll
	if (pRTICE_PTG_CTRL->PTG_OnOff == 1) {
		rtk_hal_vsc_SetWinBlank(VIDEO_WID_0, 0, KADP_VIDEO_DDI_WIN_COLOR_BLACK);

		Scaler_APDEM_PTG_set(pRTICE_PTG_CTRL);
	} else {	// re-run scaler
		vsc_force_rerun_main_scaler = TRUE;
		DbgSclrFlgTkr.Main_Run_Scaler_flag = TRUE;		
	}
	return 0;
}

int rtice_get_vip_table(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf)
{
#ifndef BUILD_QUICK_SHOW

	int ret;
	/*printf("id =%d, size =%d \n", id, size);*/
	int i = 0;/*, idx = 0;*/
	unsigned int temp_size = 0;
	unsigned int *pwUINT32Temp = NULL;
	/*unsigned short *pwUINT16Temp = NULL;*/
	unsigned short UINT16Temp = 0;
	short *pwINT16Temp = NULL;
	unsigned int pwCopyTemp=0;
	RPC_DCC_Advance_control_table *Advance_control_table_by_source = NULL;
	/*int *pwINT32Temp = NULL;*/
	/*unsigned int pwCopyTemp = 0;*/
	/*RPC_DCC_Advance_control_table *Advance_control_table_by_source = NULL;*/
	/*int h_sta,  h_end,  s_sta,  s_end,  i_sta,  i_end;*/
	/*unsigned int *pWriteBuf = NULL;*/
	_clues *smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	_RPC_clues *RPC_smartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);
	unsigned char bTemp = 0;
	unsigned char *buf2_tmp = NULL;
	int *pwIntTemp = NULL;
	int intCopyTemp = 0;
	unsigned int *buf_tmp = NULL;
	unsigned char *buf_tmp8 = NULL;
	char *pCharTemp = NULL;
	SLR_VIP_TABLE *vipTable_ShareMem = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	unsigned int size, start_addr, end_addr;
	unsigned char curve_index;
	unsigned char crtl_table[13];
	unsigned char tablesize[21];
	unsigned int DCC_HISTO_MEAN_VALUE;
	_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *RPC_system_setting_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;
	VIP_ST2094_CTRL *ST2094_CTRL = NULL;
	ST2094_metadata *ST2094_metadata;
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
	static char OSD_HDRMODE_DEMO_init = 0;
#endif
	unsigned char HY,VY,HC,VC;
	char item_name[10];

	memset(item_name, 0, sizeof(item_name));

	stopUpdateFrequencyOffset = 180;

	/*=== marked by Elsie ===*/
	/*SLR_VIP_TABLE_CUSTOM *g_Share_Memory_VIP_TABLE_Custom_Struct = NULL;*/
	/*g_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);*/

	/* for tv002*/
	/*SLR_VIP_TABLE_CUSTOM_TV002 *shareMem_VIP_Customer_TV002 = (SLR_VIP_TABLE_CUSTOM_TV002*)g_Share_Memory_VIP_TABLE_Custom_Struct;*/
	/*VIP_ADV_API_TBL ADV_API_TBL_tmp;*/

	if (gVip_Table == NULL || vipTable_ShareMem == NULL ||
		system_setting_info == NULL || RPC_system_setting_info == NULL ||
		smartPic_clue == NULL || RPC_smartPic_clue == NULL)
		return -ID_SIZE_ERR;

	size = rtice_param.size;
	start_addr = rtice_param.saddr;
	end_addr = rtice_param.eaddr;

	switch (rtice_param.id) {

/***************************Enginner Menu Start ********************************************/

	case TOOLACCESS_Contrast:   	/*  0*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetContrast();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_Brightness:   	/*  1*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetBrightness();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_Saturation:   	/*  2*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetSaturation();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_Hue:   			/*  3*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetHue();
		*num_type = VIP_UINT8;
		break;



	case TOOLACCESS_Local_Contrast:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetLocalContrastTable();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_Dynamic_Contrast:
		if (size > sizeof(unsigned char)*2)
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetDCC_Table();
		buf[1] = Scaler_GetDCC_Mode();

		*num_type = VIP_UINT8;
		break;


	case TOOLACCESS_VD_Contrast:   	/*  4*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*buf[0] = Scaler_Getvd();*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_Brightness:	/*  5*/
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VD_Saturation:	/*  6*/
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VD_Hue:			/*  7*/
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_Gamma:			/*  8*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetGamma();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DCTI:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetDCti();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DLTI:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetDLti();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_MADI_HMC:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetMADI_HMC();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_MADI_HME:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetMADI_HME();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_MADI_PAN:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetMADI_PAN();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DI_MA_Adjust_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetMADI();
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_DI_DiSmd:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetDiSmd();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_TNRXC_Ctrl:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetTNRXC_Ctrl();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_TNRXC_MK2:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetTNRXC_MK2();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_ScaleUPH:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleUPH();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_ScaleUPH_12tap:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleUPH_12tap();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_ScaleUPV_8tap:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleUPV_8tap();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_ScaleUPV:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleUPV();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_ScaleUPDir:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleUPDir();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_ScaleUPDir_Weighting:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleUPDir_Weighting();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_ScaleDOWN_H_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleDOWNHTable();
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_ScaleDOWN_V_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleDOWNVTable();
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_ScaleDOWN_444To422:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetScaleDOWN444To422();
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_D_UVDelay_en:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetColor_UV_Delay_Enable();
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_D_UVDelay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = Scaler_GetColor_UV_Delay();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetYC_Separation:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_YcSeparation_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VD_Set3D_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_Get3DTableIndex();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_Set2D_Chroma_BW_Low:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_2D_Chroma_Bw_lo_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetSetDCTI_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_Set_DCTI_Setting_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_Set2D_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vpq_get_2d_table_index_Factory();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetNarrow_BPF_Y:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_2D_Narrow_BPF_Sel_Y_Factory_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetWide_BPF_Y:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_2D_Wide_BPF_Sel_Y_Factory_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetWide_BPF_C:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_2D_Wide_BPF_Sel_C_Factory_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetNarrow_BPF_C:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_2D_Narrow_BPF_Sel_C_Factory_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetVflag_Remg_Thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_H2V_Vflag_Remg_thr_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetVflag_Mag_Thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_H2V_Vflag_Mag_thr_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetY_Noise_Thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_2D_Narrow_BPF_Sel_C_Factory_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetNoise_Max_Hdy:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_H2V_Vflag_Max_HDY_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetNoise_Y_Add_DC:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_H2V_Vflag_Noise_YADDC_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetBlend_Ratio:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_H2V_Vflag_Blend_Ratio_Factory_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetTDma_Enable:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_factory_TestDMA_Enable(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetTDma_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_factory_TestDMA_mode_select(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetDeb_Print:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Debug_Print_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_Set625_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_625_mode_setting_Factory_mode(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetMV_Check:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_MV_check_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetClamp_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_ClampMode_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetRc_Rate_Func:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_clamp_rc_rate_enable_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetRc_Rate_Value:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_FIFO_status_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetV_Clamp:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/* drvif_module_vdc_Vsync_Clamp_mode_Factory_Mode(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetVsync_Ctrl:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Vsync_Ctrl_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetClamp_Delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_clamp_delay_Enable_Factory_mode_set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetClamp_Delay_Value:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_clamp_delay_value_Factory_mode_set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetSetH_State_Write:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Hstate_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetHsync_Start:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Hsync_start_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetV_State_Write:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Vstate_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetV_Detect_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_Read_New_Vdetect_Factory_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetC_State_W:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cstate_Cactory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetAuto_BP:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_Auto_Burst_position_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetCkill_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_user_ckill_mode_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_VD_SetCkill_Value:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_Chroam_Level_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetSet_Dgain:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Dgain_Write_Factory_Mode(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetFix_Dgain:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/* drvif_module_vdc_Fix_Dgain_Factory_Mode(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetSet_Cgain:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vdc_Cgain_Factory_Mode_Get_info();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetFix_Cgain:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Fix_Cagc_Factory_Mode(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_Set27M_LPF:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_27M_Lpf_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_Set27M_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_27M_table_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_Set108M_LPF:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_108M_Lpf_Factory_Mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetSecam_F_sel:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_secamf_sel_Factory_mode_set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetSecam2:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_secam2_Factory_mode_set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_Set443358pk_sel:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_secam2_Factory_mode_set(buf[0]);*/
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_VD_Set443thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_set_Sep_443_thl(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_Set358thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_set_Sep_358_thl(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_SetSoft_Reset:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_SoftReset();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetBypassPQ:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_PQ_Bypass_Factory_mode_set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_Set1D3D_Pos:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_1D3Dpos_Factory_mode_set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetContrast:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SetContrast(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetBrightness:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SetBrightness(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetSaturation:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SetSaturation(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetYC_Delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_YcDelay(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetY1delay_En:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Y1Delay_Enable_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetCb1delay_En:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cb1Delay_Enable_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetCr1delay_En:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cr1Delay_Enable_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetY1delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Y1Delay_Value_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetCb1delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cb1Delay_Value_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetCr1delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cr1Delay_Value_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetKill_Ydelay_En:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_ckill_YDelay_Enable_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetKill_Ydelay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_ckill_YDelay_Value_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetSCART_Con:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SCART_contrast_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetSCART_Bri:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SCART_brightness_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetSCART_Sat_cr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SCART_saturationCr_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_SetSCART_Sat_cb:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SCART_saturationCb_Factory_mode_Set(buf[0]);*/
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VD_PQ_Style:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = drvif_module_vpq_get_ProjectId();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DCC_PQ_current_mean:
		if (size > sizeof(unsigned int)*1)
			return -ID_SIZE_ERR;

		/*pwUINT32Temp[0] = Scaler_get_DCC_AdapCtrl_Current_Mean();*/

		/*fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/4, 1);*/
		/*memcpy(buf, (unsigned char *)pwUINT32Temp, size);*/

		temp_size = 1;
		buf_tmp = &(smartPic_clue->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_mean]);
		fwif_color_ChangeUINT32Endian(buf_tmp,temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *)buf_tmp, size);
#else
		memcpy(buf, (unsigned char *)buf_tmp, size);
#endif

		*num_type = VIP_UINT32;
		break;

	case TOOLACCESS_DCC_PQ_current_dev:
		if (size > sizeof(unsigned int)*1)
			return -ID_SIZE_ERR;

		/*pwUINT32Temp[0] = Scaler_get_DCC_AdapCtrl_Current_Dev();*/
		/*fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/4, 1);*/
		/*memcpy(buf, (unsigned char *)pwUINT32Temp, size);*/
		temp_size = 1;
		buf_tmp = &(smartPic_clue->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_mean]);
		fwif_color_ChangeUINT32Endian(buf_tmp,temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *)buf_tmp, size);
#else
		memcpy(buf, (unsigned char *)buf_tmp, size);
#endif

		*num_type = VIP_UINT32;
		break;

	case TOOLACCESS_DCC_PQ_Level:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = Scaler_get_DCC_AdapCtrl_DCC_Level();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DCC_PQ_Cubic_index:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = Scaler_get_DCC_AdapCtrl_Cubic_index();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DCC_PQ_Cubic_low:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = Scaler_get_DCC_AdapCtrl_Cubic_low();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DCC_PQ_Cubic_high:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = Scaler_get_DCC_AdapCtrl_Cubic_high();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_xvYcc_mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
#ifdef ENABLE_xvYcc
		buf[0] = Scaler_GetxvYcc_Mode();
		*num_type = VIP_UINT8;
#endif
		break;
	case TOOLACCESS_ICM_7axis_adjust:
		if (size > sizeof(unsigned char)*7)
			return -ID_SIZE_ERR;
#if 0	/*=== marked by Elsie*/
		buf_tmp8 = (unsigned char *)vip_malloc(7 * sizeof(unsigned char));
		if (buf_tmp8 == NULL)
			return -ID_SIZE_ERR;

		buf_tmp8[0] = Scaler_get_ICM_7Axis_Adjust(0);
		buf_tmp8[1] = Scaler_get_ICM_7Axis_Adjust(1);
		buf_tmp8[2] = Scaler_get_ICM_7Axis_Adjust(2);
		buf_tmp8[3] = Scaler_get_ICM_7Axis_Adjust(3);
		buf_tmp8[4] = Scaler_get_ICM_7Axis_Adjust(4);
		buf_tmp8[5] = Scaler_get_ICM_7Axis_Adjust(5);
		buf_tmp8[6] = Scaler_get_ICM_7Axis_Adjust(6);

		memcpy(buf, (unsigned char *)buf_tmp8, size);
		vip_free(buf_tmp8);
		buf_tmp8 = NULL;

		*num_type = VIP_UINT8;
#endif
		break;

	case TOOLACCESS_switch_DVIandHDMI:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
#if 0	/*=== marked by Elsie ===*/
		buf[0] = Scaler_Getswitch_DVIandHDMI();
		*num_type = VIP_UINT8;
#endif
		break;
/***************************Enginner Menu End ********************************************/

	case TOOLACCESS_VIP_QUALITY_Coef:       /*  20*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Coef[0][0]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Coef[0][0]), size);
#endif
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VIP_QUALITY_Extend_Coef:       /*  21*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Extend_Coef[0][0]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Extend_Coef[0][0]), size);
#endif
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VIP_QUALITY_Extend2_Coef:       /*  22*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Extend2_Coef[0][0]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Extend2_Coef[0][0]), size);
#endif
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VIP_QUALITY_Extend3_Coef:       /*  23*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Extend3_Coef[0][0]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->VIP_QUALITY_Extend3_Coef[0][0]), size);
#endif
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_tICM_ini:       /*  24*/
		if (size > sizeof(unsigned short)*VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->tICM_ini[0][0][0]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->tICM_ini[0][0][0]), size);
#endif
		*num_type = VIP_UINT16;
		break;
	case TOOLACCESS_SHPTable:       /*  25*/
		if (size > sizeof(VIP_Sharpness_Table))
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->Ddomain_SHPTable[0]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->Ddomain_SHPTable[0]), size);
#endif
		*num_type = VIP_INT16;
		break;
	case TOOLACCESS_Hist_Y_Mean_Value:/* 26*/
		if (size > sizeof(unsigned int)*1)
			return -ID_SIZE_ERR;
		temp_size = 1;
		buf_tmp = &(smartPic_clue->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_mean]);
		fwif_color_ChangeUINT32Endian(buf_tmp,temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *)buf_tmp, size);
#else
		memcpy(buf, (unsigned char *)buf_tmp, size);
#endif
		*num_type = VIP_UINT32;
		break;
	case TOOLACCESS_TABLE_NTSC_3D:/*  27*/
		if (size > sizeof(unsigned char)*VD_3D_TABLE_SIZE*VD_MOTION_STEP*2)
			return -ID_SIZE_ERR;

		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_MANUAL_NR:/* 29*/
		if (size > sizeof(DRV_NR_Item)*PQA_TABLE_MAX*DRV_NR_Level_MAX)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->Manual_NR_Table[0][0]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->Manual_NR_Table[0][0]), size);
#endif
		*num_type = VIP_UINT8;
		break;

	#if 0
	case TOOLACCESS_PQA_Input_Table:/* 30*/
		if (size > sizeof(int)*PQA_I_TABLE_MAX*PQA_I_ITEM_MAX*PQA_I_LEVEL_MAX)
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->PQA_Input_Table[0][0][0]), size);
		*num_type = VIP_UINT32;
		break;
	case TOOLACCESS_PQA_Table:
		if (size > sizeof(int)*PQA_TABLE_MAX*PQA_MODE_MAX*PQA_ITEM_MAX*PQA_LEVEL_MAX)
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->PQA_Table[0][0][0][0]), size);
		*num_type = VIP_UINT32;
		break;
	#endif

	case TOOLACCESS_SmartPic_clue_Motion_Cal:
		if (size > sizeof(int)*14)
			return -ID_SIZE_ERR;
		buf_tmp = (unsigned int *)vip_malloc(17 * sizeof(int));
		if (buf_tmp == NULL)
			return -ID_SIZE_ERR;

		buf_tmp[0] = smartPic_clue->film_motion_next_h_t;
		buf_tmp[1] = smartPic_clue->film_motion_next_h_m;
		buf_tmp[2] = smartPic_clue->film_motion_next_h_b;
		buf_tmp[3] = smartPic_clue->film_motion_pre_h_t;
		buf_tmp[4] = smartPic_clue->film_motion_pre_h_m;
		buf_tmp[5] = smartPic_clue->film_motion_pre_h_b;
		buf_tmp[6] = smartPic_clue->film_motion_next_v_l;
		buf_tmp[7] = smartPic_clue->film_motion_next_v_m;
		buf_tmp[8] = smartPic_clue->film_motion_next_v_r;
		buf_tmp[9] = smartPic_clue->film_motion_pre_v_l;
		buf_tmp[10] = smartPic_clue->film_motion_pre_v_m;
		buf_tmp[11] = smartPic_clue->film_motion_pre_v_r;
		buf_tmp[12] = smartPic_clue->film_motion_next_c;
		buf_tmp[13] = smartPic_clue->film_motion_pre_c;
		buf_tmp[14] = smartPic_clue->RTNR_MAD_count_Y_avg_ratio;
		buf_tmp[15] = smartPic_clue->RTNR_MAD_count_U_avg_ratio;
		buf_tmp[16] = smartPic_clue->RTNR_MAD_count_V_avg_ratio;

		fwif_color_ChangeUINT32Endian(buf_tmp,17, 1);
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *)buf_tmp, size);
#else
		memcpy(buf, (unsigned char *)buf_tmp, size);
#endif

		*num_type = VIP_UINT32;
		vip_free(buf_tmp);
		buf_tmp = NULL;
		break;

	case TOOLACCESS_SmartPic_clue_HSI_Histogram_Cal: /*33*/
		if (size > sizeof(int)*91)
			return -ID_SIZE_ERR;
		buf_tmp = (unsigned int *)vip_malloc(91 * sizeof(int));
		if (buf_tmp == NULL)
			return -ID_SIZE_ERR;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf_tmp, &(smartPic_clue->Y_Main_Hist_Ratio[0]), sizeof(int)*TV006_VPQ_chrm_bin);
		memcpy_fromio(buf_tmp+TV006_VPQ_chrm_bin, &(smartPic_clue->Hue_Main_His_Ratio[0]), sizeof(int)*COLOR_HUE_HISTOGRAM_LEVEL);
		memcpy_fromio(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL, &(smartPic_clue->Sat_Main_His_Ratio[0]), sizeof(int)*COLOR_AutoSat_HISTOGRAM_LEVEL);
		memcpy_fromio(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL+COLOR_AutoSat_HISTOGRAM_LEVEL, &(smartPic_clue->Hist_Y_Mean_Value), sizeof(int)*1);
		memcpy_fromio(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL+COLOR_AutoSat_HISTOGRAM_LEVEL+1, &(smartPic_clue->skintoneflag), sizeof(int)*1);
		memcpy_fromio(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL+COLOR_AutoSat_HISTOGRAM_LEVEL+2, &(smartPic_clue->PureColor), sizeof(int)*1);
#else
		memcpy(buf_tmp, &(smartPic_clue->Y_Main_Hist_Ratio[0]), sizeof(int)*TV006_VPQ_chrm_bin);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin, &(smartPic_clue->Hue_Main_His_Ratio[0]), sizeof(int)*COLOR_HUE_HISTOGRAM_LEVEL);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL, &(smartPic_clue->Sat_Main_His_Ratio[0]), sizeof(int)*COLOR_AutoSat_HISTOGRAM_LEVEL);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL+COLOR_AutoSat_HISTOGRAM_LEVEL, &(smartPic_clue->Hist_Y_Mean_Value), sizeof(int)*1);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL+COLOR_AutoSat_HISTOGRAM_LEVEL+1, &(smartPic_clue->skintoneflag), sizeof(int)*1);
		memcpy(buf_tmp+TV006_VPQ_chrm_bin+COLOR_HUE_HISTOGRAM_LEVEL+COLOR_AutoSat_HISTOGRAM_LEVEL+2, &(smartPic_clue->PureColor), sizeof(int)*1);
#endif
		fwif_color_ChangeUINT32Endian(buf_tmp,91, 1);
		memcpy(buf, (unsigned char *)buf_tmp, size);

		*num_type = VIP_UINT32;
		vip_free(buf_tmp);
		buf_tmp = NULL;
		break;

	case TOOLACCESS_SmartPic_clue_VD_Noise_Cal:
		if (size > sizeof(int)*2)
			return -ID_SIZE_ERR;
		buf_tmp = (unsigned int *)vip_malloc(2 * sizeof(int));
		if (buf_tmp == NULL)
			return -ID_SIZE_ERR;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf_tmp, &(smartPic_clue->VD_Signal_Status_value_avg), sizeof(int)*1);
		memcpy_fromio(buf_tmp+1, &(smartPic_clue->VD_Noise_level_compute), sizeof(int)*1);
#else
		memcpy(buf_tmp, &(smartPic_clue->VD_Signal_Status_value_avg), sizeof(int)*1);
		memcpy(buf_tmp+1, &(smartPic_clue->VD_Noise_level_compute), sizeof(int)*1);
#endif
		temp_size = 2 * sizeof(int);
		fwif_color_ChangeUINT32Endian(buf_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *)buf_tmp, size);
		*num_type = VIP_UINT32;
		vip_free(buf_tmp);
		buf_tmp = NULL;
		break;

	case TOOLACCESS_ISR_Print_Ctrl:
		if (size > sizeof(VIP_ISR_Printf_Flag))
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(system_setting_info->ISR_Printf_Flag), size);
#else
		memcpy(buf, (unsigned char *) &(system_setting_info->ISR_Printf_Flag), size);
#endif
		fwif_color_ChangeUINT32Endian((unsigned int *)buf, ISR_PRINT_CTRL_ITEM_NUMBER, 1);
		*num_type = VIP_UINT32;
		break;

		case TOOLACCESS_Debug_Buff_8:
			if(size > Debug_Buff_Num*sizeof(char))
				return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
			memcpy_fromio(buf,(unsigned char*)&(system_setting_info->Debug_Buff_8[0]), size);
#else
			memcpy(buf,(unsigned char*)&(system_setting_info->Debug_Buff_8[0]), size);
#endif
			*num_type = VIP_UINT8;
			break;

		case TOOLACCESS_Debug_Buff_16:
			if(size > Debug_Buff_Num*sizeof(short))
				return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
			memcpy_fromio(buf,(unsigned char*)&(system_setting_info->Debug_Buff_16[0]), size);
#else
			memcpy(buf,(unsigned char*)&(system_setting_info->Debug_Buff_16[0]), size);
#endif
			fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);


			*num_type = VIP_UINT16;
			break;

		case TOOLACCESS_Debug_Buff_32:
			if(size > Debug_Buff_Num*sizeof(int))
				return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
			memcpy_fromio(buf,(unsigned char*)&(system_setting_info->Debug_Buff_32[0]), size);
#else
			memcpy(buf,(unsigned char*)&(system_setting_info->Debug_Buff_32[0]), size);
#endif
			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(int), 1);


			*num_type = VIP_UINT32;
			break;

		case TOOLACCESS_Debug_HDR:
			if(size > Debug_HDR_ITEM_NUMBER*sizeof(int))
				return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
			memcpy_fromio(buf,(unsigned char*)&(RPC_system_setting_info->HDR_info.EOTF_OETF[0]), size);
#else
			memcpy(buf,(unsigned char*)&(RPC_system_setting_info->HDR_info.EOTF_OETF[0]), size);
#endif
			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(int), 1);


			*num_type = VIP_UINT32;
			break;

	case TOOLACCESS_BP_Function_CTRL:
		if (size > sizeof(VIP_BP_CTRL))
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_BP_Function_TBL:
		if (size > sizeof(VIP_BP_TBL))
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_ADV_API_CTRL:
		if (size > sizeof(VIP_ADV_API_CTRL))
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_ADV_API_TBL:
		if (size > sizeof(VIP_ADV_API_TBL))
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_DBC_STATUS:
		if (size > sizeof(VIP_DBC_STATUS))
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_DBC_SW_REG:
		if (size > sizeof(VIP_DBC_SW_REG))
			return -ID_SIZE_ERR;
		break;

	/***************************get ID information***********************************************/ /**/
	case TOOLACCESS_GetIdxTable_IDMax:
		if (size > sizeof(unsigned short)*1)
			return -ID_SIZE_ERR;
		UINT16Temp = fwif_color_ChangeOneUINT16Endian(TOOLACCESS_MAX, 1);
		memcpy(buf, (unsigned char *) &UINT16Temp, size);

		*num_type = VIP_UINT16;
		break;
	case TOOLACCESS_GetIdxTable_Type:
		if (rtice_set_IdxTable2Buff(TOOLACCESS_GetIdxTable_Type, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
	break;
	case TOOLACCESS_GetIdxTable_MaxNum:
		if (rtice_set_IdxTable2Buff(TOOLACCESS_GetIdxTable_MaxNum, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_GetIdxTable_StringName:
		if (rtice_set_IdxTable2Buff(TOOLACCESS_GetIdxTable_StringName, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
		break;
	/***************************get ID information***********************************************/ /**/

	/***************************for PQA Table access*****************************************/ /**/
	case TOOLACCESS_PQA_Table_Size:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Table_Size, 0, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
	break;

	case TOOLACCESS_PQA_Table:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Table, 0, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
	break;

	case TOOLACCESS_PQA_Level_Table:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Level_Table, 0, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
	break;

	case TOOLACCESS_PQA_Level_Index_Table:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Level_Index_Table, 0, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
		break;

	case TOOLACCESS_PQA_Input_Table:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Input_Table, 0, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_PQA_FlowCtrl_Input_Type:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_FlowCtrl_Input_Type, 0, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_PQA_FlowCtrl_Input_Item:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_FlowCtrl_Input_Item, 0, size, num_type, buf) < 0)
			return -ID_SIZE_ERR;
	break;

       case TOOLACCESS_PQA_SPM_Info_Get:
               if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_SPM_Info_Get, 0, size, num_type, buf) < 0)
                       return -ID_SIZE_ERR;
       break;

	/***************************for PQA NR Table access*****************************************/ /**/

	case TOOLACCESS_DCC_Curve_Control_Coef:				/* 1100*/
		if (size > sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX+1)
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Table;
		buf2_tmp = (unsigned char *)vip_malloc(sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX+1);
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Curve_Control_Coef[bTemp]), sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX);
#else
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Curve_Control_Coef[bTemp]), sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX);
#endif
		buf2_tmp[sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX] = system_setting_info->OSD_Info.OSD_DCC_Mode;
		memcpy(buf, (unsigned char *)buf2_tmp, size);
		vip_free(buf2_tmp);
		buf2_tmp = NULL;
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_DCC_Boundary_check_Table:   		/* 1101*/
		if (size > sizeof(VIP_DCC_Boundary_check_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.Boundary_Check_Table;
		if (bTemp >= DCC_Boundary_Check_Table_MAX) bTemp = DCC_Boundary_Check_Table_MAX - 1;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Boundary_check_Table[bTemp]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Boundary_check_Table[bTemp]), size);
#endif
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DCC_Level_and_Blend_Coef_Table:     /* 1102*/
		if (size > sizeof(VIP_DCC_Level_and_Blend_Coef_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.Level_and_Blend_Coef_Table;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[bTemp]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[bTemp]), size);
#endif
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DCC_Hist_Factor_Table:   			/* 1103*/
		if (size > sizeof(VIP_DCC_Hist_Factor_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.hist_adjust_table;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCCHist_Factor_Table[bTemp]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCCHist_Factor_Table[bTemp]), size);
#endif
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_DCC_AdaptCtrl_Level_Table:  		/* 1104*/
		if (size > sizeof(VIP_DCC_AdaptCtrl_Level_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.AdaptCtrl_Level_Table;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[bTemp]), size);
#else
		memcpy(buf, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[bTemp]), size);
#endif
		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_DCC_AdaptCtrl_Param:				/* 1105*/
		if (size > 8*sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc(8*sizeof(unsigned char));
		if (buf2_tmp == NULL) {
			return -ID_SIZE_ERR;
		}

		DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();
		if (DCC_AdaptCtrl_Level_Table == NULL) {
			vip_free(buf2_tmp);
			//printf("~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
			return -ID_SIZE_ERR;
		}
		pwUINT32Temp = &(smartPic_clue->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_mean]);
		buf2_tmp[6] = (unsigned char)*pwUINT32Temp;
		pwUINT32Temp = &(smartPic_clue->Adapt_DCC_Info.Adapt_Mean_Dev_info[DCC_Tool_Dev]);
		buf2_tmp[7] = (unsigned char)*pwUINT32Temp;


		buf2_tmp[0] = (DCC_AdaptCtrl_Level_Table->DCC_Level_table)[buf2_tmp[6]][buf2_tmp[7]];
		buf2_tmp[1] = (DCC_AdaptCtrl_Level_Table->DCC_Cublc_Index_table)[buf2_tmp[6]][buf2_tmp[7]];
		buf2_tmp[2] = (DCC_AdaptCtrl_Level_Table->DCC_Cublc_High_table)[buf2_tmp[6]][buf2_tmp[7]];
		buf2_tmp[3] = (DCC_AdaptCtrl_Level_Table->DCC_Cublc_Low_table)[buf2_tmp[6]][buf2_tmp[7]];
		buf2_tmp[4] = (DCC_AdaptCtrl_Level_Table->DCC_W_Ex_table)[buf2_tmp[6]][buf2_tmp[7]];
		buf2_tmp[5] = (DCC_AdaptCtrl_Level_Table->DCC_B_Ex_table)[buf2_tmp[6]][buf2_tmp[7]];

		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		memcpy(buf, (unsigned char *) &buf2_tmp[0], size);
		*num_type = VIP_UINT8;

		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;

	case TOOLACCESS_DCC_UserDef_Ctrl_TABLE: /*1106*/
		if (size > (sizeof(USER_CURVE_DCC_HisMean_th_ITEM)+sizeof(USER_CURVE_DCC_HisSeg_Weighting_ITEM)+3))
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc(sizeof(USER_CURVE_DCC_HisMean_th_ITEM)+sizeof(USER_CURVE_DCC_HisSeg_Weighting_ITEM)+3);
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.User_Curve_Table;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisMean_th), 10);
		memcpy_fromio(buf2_tmp+sizeof(USER_CURVE_DCC_HisMean_th_ITEM), (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisSeg_Weighting), 11);
#else
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisMean_th), 10);
		memcpy(buf2_tmp+sizeof(USER_CURVE_DCC_HisMean_th_ITEM), (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisSeg_Weighting), 11);
#endif

		DCC_HISTO_MEAN_VALUE = smartPic_clue->Hist_Y_Mean_Value;

#ifdef CONFIG_ARM64
		memcpy_fromio(crtl_table, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisMean_th), 10);
#else
		memcpy(crtl_table, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisMean_th), 10);
#endif

	for (curve_index = 0; curve_index < crtl_table[9]; curve_index++) {
			if (DCC_HISTO_MEAN_VALUE <= crtl_table[curve_index])
				break;
		}

	if (curve_index == 0) { /*mean value < th0, use curve 0*/
			user_curve_1 = 0;
			user_curve_2 = 255;
	} else if (curve_index == crtl_table[9]) { /*mean value > last th setting*/
			user_curve_1 = 255;
			user_curve_2 = (curve_index-1);
	} else { /*blending curve*/
			user_curve_1 = (curve_index-1);
			user_curve_2 = curve_index;
		}
		buf2_tmp[21] = (unsigned char)DCC_HISTO_MEAN_VALUE;
		buf2_tmp[22] = user_curve_1;
		buf2_tmp[23] = user_curve_2;
		memcpy(buf, (unsigned char *) &buf2_tmp[0], size);
		*num_type = VIP_UINT8;

		vip_free(buf2_tmp);
		buf2_tmp = NULL;


		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_DCC_UserDef_Curve_All_TABLE: /*1107*/
		if (size > (sizeof(unsigned int)*(USER_DEFINE_CURVE_DCC_CURVE_MAX)*USER_DEFINE_CURVE_DCC_SEGMENT_NUM))
			return -ID_SIZE_ERR;

		temp_size = (USER_DEFINE_CURVE_DCC_CURVE_MAX)*USER_DEFINE_CURVE_DCC_SEGMENT_NUM;
		buf2_tmp = (unsigned char *)vip_malloc(temp_size*sizeof(unsigned int));
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.User_Curve_Table;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table),temp_size*sizeof(unsigned int));
#else
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table),temp_size*sizeof(unsigned int));
#endif
		fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *) buf2_tmp, size);

		*num_type = VIP_UINT32;

		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;

	case TOOLACCESS_DCC_UserDef_Curve_Current_TABLE:/*1108 */ /*size:129: mode 0:*/

		if (size > sizeof(unsigned int)*USER_DEFINE_CURVE_DCC_SEGMENT_NUM)
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc(USER_DEFINE_CURVE_DCC_SEGMENT_NUM*sizeof(unsigned int));
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.User_Curve_Table;

		if (rtice_param.mode0 >= USER_DEFINE_CURVE_DCC_CURVE_MAX)
			rtice_param.mode0 = USER_DEFINE_CURVE_DCC_CURVE_MAX-1;

		temp_size = USER_DEFINE_CURVE_DCC_SEGMENT_NUM;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table[rtice_param.mode0][0]),temp_size*sizeof(unsigned int));
#else
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table[rtice_param.mode0][0]),temp_size*sizeof(unsigned int));
#endif
		fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *) buf2_tmp, size);

		*num_type = VIP_UINT32;

		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;

	case TOOLACCESS_DCC_Database_Curve_CRTL_All_Table: /*1109 (32+129)*10+matching_INDEX+ MATCH_RATIO*/
		if (size > ((Histogram_adjust_bin_num+DCC_CURVE_Segment_NUM+DCC_CURVE_User_Gain_Parameter)*Database_DCC_Curve_Case_Item_MAX + 2)*sizeof(unsigned int))
			return -ID_SIZE_ERR;

		temp_size = (Histogram_adjust_bin_num+DCC_CURVE_Segment_NUM+DCC_CURVE_User_Gain_Parameter)*Database_DCC_Curve_Case_Item_MAX;

		buf2_tmp = (unsigned char *)vip_malloc((temp_size+2)*sizeof(unsigned int));
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.database_DCC_Table;

		if(bTemp >= Database_DCC_Curve_TABLE_MAX)
			bTemp = Database_DCC_Curve_TABLE_MAX-1;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][0]), temp_size*sizeof(unsigned int));
		memcpy_fromio(buf2_tmp+(temp_size*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy_fromio(buf2_tmp+((temp_size+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));
#else
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][0]), temp_size*sizeof(unsigned int));
		memcpy(buf2_tmp+(temp_size*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy(buf2_tmp+((temp_size+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));
#endif

		fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,(temp_size+2), 1);
		memcpy(buf, (unsigned char *)buf2_tmp, size);
		*num_type = VIP_UINT32;
		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;

	case TOOLACCESS_DCC_Database_Curve_CRTL_Curve_Current_Table: /*1110 :size :32+129+matcing_index+matching _ratio; Mode 0:curve_index*/
		if (size > (DCC_CURVE_Segment_NUM+2)*sizeof(unsigned int))/*mode1:histogram_index*/
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc((DCC_CURVE_Segment_NUM+2)*sizeof(unsigned int));
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.database_DCC_Table;

		if(bTemp >= Database_DCC_Curve_TABLE_MAX)
			bTemp = Database_DCC_Curve_TABLE_MAX-1;

		if (rtice_param.mode0 >= Database_DCC_Curve_Case_Item_MAX)
			rtice_param.mode0 = Database_DCC_Curve_Case_Item_MAX-1;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_curve), (DCC_CURVE_Segment_NUM*sizeof(unsigned int)));
		memcpy_fromio(buf2_tmp+(DCC_CURVE_Segment_NUM*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy_fromio(buf2_tmp+((DCC_CURVE_Segment_NUM+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));
#else
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_curve), (DCC_CURVE_Segment_NUM*sizeof(unsigned int)));
		memcpy(buf2_tmp+(DCC_CURVE_Segment_NUM*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy(buf2_tmp+((DCC_CURVE_Segment_NUM+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));
#endif

		temp_size = (DCC_CURVE_Segment_NUM+2);
		fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *)buf2_tmp, size);
		*num_type = VIP_UINT32;

		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;

	case TOOLACCESS_DCC_Database_Curve_CRTL_Histogram_Current_Table: /*id 1111*/
		if (size > ((Histogram_adjust_bin_num+2)*sizeof(unsigned int)) )/*mode1:histogram_index*/
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc((Histogram_adjust_bin_num+2)*sizeof(unsigned int));
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.database_DCC_Table;

		if(bTemp >= Database_DCC_Curve_TABLE_MAX)
			bTemp = Database_DCC_Curve_TABLE_MAX-1;

		if (rtice_param.mode0 >= Database_DCC_Curve_Case_Item_MAX)
			rtice_param.mode0 = Database_DCC_Curve_Case_Item_MAX-1;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_histogram), (Histogram_adjust_bin_num*sizeof(unsigned int)));
		memcpy_fromio(buf2_tmp+(Histogram_adjust_bin_num*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy_fromio(buf2_tmp+((Histogram_adjust_bin_num+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));
#else
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_histogram), (Histogram_adjust_bin_num*sizeof(unsigned int)));
		memcpy(buf2_tmp+(Histogram_adjust_bin_num*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy(buf2_tmp+((Histogram_adjust_bin_num+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));
#endif

		temp_size = (Histogram_adjust_bin_num+2);
		fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *)buf2_tmp, size);
		*num_type = VIP_UINT32;
		vip_free(buf2_tmp);
		buf2_tmp = NULL;

	break;

	case TOOLACCESS_DCC_Histogram_Curve:  /*id:1112*/
		if (size > sizeof(unsigned int)*DCC_Curve_Node_MAX)
			return -ID_SIZE_ERR;

		temp_size = DCC_Curve_Node_MAX;
		fwif_color_ChangeUINT32Endian((unsigned int *)&(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Histogram_Curve),temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Histogram_Curve), size);
#else
		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Histogram_Curve), size);
#endif

		*num_type = VIP_UINT32;
		break;

	case TOOLACCESS_DCC_S_Curve:  /*id:1113*/
		if (size > sizeof(unsigned int)*DCC_Curve_Node_MAX)
			return -ID_SIZE_ERR;

		temp_size = DCC_Curve_Node_MAX;
		fwif_color_ChangeUINT32Endian((unsigned int *)&(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.S_Curve),temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.S_Curve), size);
#else
		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.S_Curve), size);
#endif

		*num_type = VIP_UINT32;
		break;

	case TOOLACCESS_DCC_Histogram_Adjust:  /*id:1114*/
		if (size > sizeof(unsigned int)*COLOR_HISTOGRAM_LEVEL)
			return -ID_SIZE_ERR;

		temp_size = COLOR_HISTOGRAM_LEVEL;
		fwif_color_ChangeUINT32Endian((unsigned int *)&(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.HistCnt_Of_Adj),temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.HistCnt_Of_Adj), size);
#else
		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.HistCnt_Of_Adj), size);
#endif

		*num_type = VIP_UINT32;
		break;

	case TOOLACCESS_DCC_APL:  /*id:1115*/
		if (size > sizeof(unsigned int))
			return -ID_SIZE_ERR;

		temp_size = 1;
		buf_tmp = &(smartPic_clue->Hist_Y_Mean_Value);
		fwif_color_ChangeUINT32Endian(buf_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *)buf_tmp, size);

		*num_type = VIP_UINT32;
		break;

	case TOOLACCESS_DCC_Apply_Curve: /*1116*/
		if (size > sizeof(unsigned int)*DCC_Curve_Node_MAX)
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(DCC_Curve_Node_MAX * sizeof(int));
		if (pwIntTemp == NULL)
		   return -ID_SIZE_ERR;

		temp_size = DCC_Curve_Node_MAX;
#ifdef CONFIG_ARM64
		memcpy_fromio(pwIntTemp, (unsigned char *) &(RPC_smartPic_clue->RPC_Adapt_DCC_Info.Apply_Curve), size);
#else
		memcpy(pwIntTemp, (unsigned char *) &(RPC_smartPic_clue->RPC_Adapt_DCC_Info.Apply_Curve), size);
#endif
		fwif_color_ChangeUINT32Endian( pwIntTemp,temp_size, 1);
		memcpy(buf, (unsigned char *) pwIntTemp, size);
		vip_free(pwIntTemp);
		pwIntTemp = NULL;

		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_DCC_Curve_Boundary_Type:			/* 1117*/
	{
		VIP_DCC_Control_Structure *DCC_Control_Structure = &(vipTable_ShareMem->DCC_Control_Structure);
		unsigned char DCC_which_table = system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Table;
		unsigned char DCC_which_Mode = system_setting_info->OSD_Info.OSD_DCC_Mode;
		VIP_DCC_Curve_Control_Coef *DCC_Curve_Control_Coef = &(DCC_Control_Structure->DCC_Curve_Control_Coef[DCC_which_table][DCC_which_Mode]);

		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		//buf[0] = system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[smartPic_clue->Remmping_Src_Timing_index].DCC_init_boundary_Type
		if (size > 0)
			buf[0] = DCC_Curve_Control_Coef->boundary_type;

		*num_type = VIP_UINT8;

		break;
	}
	case TOOLACCESS_DCC_Curve_Boundary:				/* 1118*/
	{
		VIP_DCC_Control_Structure *DCC_Control_Structure = &(vipTable_ShareMem->DCC_Control_Structure);
		unsigned char DCC_which_table = system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Table;
		unsigned char DCC_which_Mode = system_setting_info->OSD_Info.OSD_DCC_Mode;
		VIP_DCC_Curve_Control_Coef *DCC_Curve_Control_Coef = &(DCC_Control_Structure->DCC_Curve_Control_Coef[DCC_which_table][DCC_which_Mode]);

		if(size > sizeof(RPC_DCC_Curve_boundary_table))
			return -ID_SIZE_ERR;

		pwCopyTemp = (smartPic_clue->Remmping_Src_Timing_index);

		Advance_control_table_by_source = &(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp]);
		//bTemp = Advance_control_table_by_source->DCC_init_boundary_Type;
		bTemp = DCC_Curve_Control_Coef->boundary_type;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Curve_boundary_table[bTemp]),size);
#else
		memcpy(buf,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Curve_boundary_table[bTemp]),size);
#endif
		*num_type = VIP_UINT8;

		break;
	}
	case TOOLACCESS_DCC_panel_compensation_Curve: /*1119*/
		if(size > sizeof(unsigned int)*Curve_num_Max)
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc( Curve_num_Max*sizeof(unsigned int));
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		temp_size = Curve_num_Max;
#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp,(unsigned char*)&(vipTable_ShareMem->panel_compensation_curve),temp_size);
#else
		memcpy(buf2_tmp,(unsigned char*)&(vipTable_ShareMem->panel_compensation_curve),temp_size);
#endif
		fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,temp_size, 1);
		memcpy(buf, (unsigned char *) buf2_tmp, size);

		*num_type = VIP_UINT32;
		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;

	case TOOLACCESS_DCC_DataBaseHistogrm :/* 1120*/
		if (size > sizeof(unsigned int)*Histogram_adjust_bin_num)
			return -ID_SIZE_ERR;

		temp_size = Histogram_adjust_bin_num;
		fwif_color_ChangeUINT32Endian(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Database_entry_Histogram,temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Database_entry_Histogram), size);
#else
		memcpy(buf, (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Database_entry_Histogram), size);
#endif

		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_DCC_Database_Curve_CRTL_User_Gain_Parameter_Current_Table: /*id 1121*/
		if (size > ((DCC_CURVE_User_Gain_Parameter+2)*sizeof(unsigned int)) )/*mode1:histogram_index*/
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc((DCC_CURVE_User_Gain_Parameter+2)*sizeof(unsigned int));
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.database_DCC_Table;

		if(bTemp >= Database_DCC_Curve_TABLE_MAX)
			bTemp = Database_DCC_Curve_TABLE_MAX-1;

		if (rtice_param.mode0 >= Database_DCC_Curve_Case_Item_MAX)
				rtice_param.mode0 = Database_DCC_Curve_Case_Item_MAX-1;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_adjust), (DCC_CURVE_User_Gain_Parameter*sizeof(unsigned int)));
		memcpy_fromio(buf2_tmp+(DCC_CURVE_User_Gain_Parameter*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy_fromio(buf2_tmp+((DCC_CURVE_User_Gain_Parameter+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));
#else
		memcpy(buf2_tmp, (unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_adjust), (DCC_CURVE_User_Gain_Parameter*sizeof(unsigned int)));
		memcpy(buf2_tmp+(DCC_CURVE_User_Gain_Parameter*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_case_num), sizeof(unsigned int));
		memcpy(buf2_tmp+((DCC_CURVE_User_Gain_Parameter+1)*sizeof(unsigned int)), (unsigned char *) &(smartPic_clue->Adapt_DCC_Info.database_weight), sizeof(unsigned int));
#endif

		temp_size = (DCC_CURVE_User_Gain_Parameter+2);
		fwif_color_ChangeUINT32Endian((unsigned int*)buf2_tmp,temp_size, 1);

		memcpy(buf, (unsigned char *)buf2_tmp, size);

		*num_type = VIP_UINT32;
		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;


	case TOOLACCESS_VIP_DCC_Advance_Level_Control_Flag: /*id: 1122 */
		if (size > (Advance_Level_Control_Item_MAX+LPF_control_Item_MAX+1) )
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc(Advance_Level_Control_Item_MAX+LPF_control_Item_MAX+1);
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		pwCopyTemp = (smartPic_clue->Remmping_Src_Timing_index);

		if(pwCopyTemp >= Advance_control_table_MAX)
			pwCopyTemp = Advance_control_table_MAX-1;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Advance_Level_Control_Flag),Advance_Level_Control_Item_MAX);
		memcpy_fromio(buf2_tmp+Advance_Level_Control_Item_MAX,(unsigned char*) &(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_LPF_Control),LPF_control_Item_MAX);
		memcpy_fromio(buf2_tmp+Advance_Level_Control_Item_MAX+LPF_control_Item_MAX,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_init_boundary_Type),1);
#else
		memcpy(buf2_tmp,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Advance_Level_Control_Flag),Advance_Level_Control_Item_MAX);
		memcpy(buf2_tmp+Advance_Level_Control_Item_MAX,(unsigned char*) &(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_LPF_Control),LPF_control_Item_MAX);
		memcpy(buf2_tmp+Advance_Level_Control_Item_MAX+LPF_control_Item_MAX,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_init_boundary_Type),1);
#endif
		memcpy(buf, (unsigned char *)buf2_tmp, size);

		vip_free(buf2_tmp);
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_VIP_DCC_Advance_Level_Control_Step_DT_Table: /*id: 1123 */
		if (size > (mean_value_index_MAX*Variance_index_MAX*2) )
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc(mean_value_index_MAX*Variance_index_MAX*2);
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		pwCopyTemp = (smartPic_clue->Remmping_Src_Timing_index);

		if(pwCopyTemp >= Advance_control_table_MAX)
			pwCopyTemp = Advance_control_table_MAX-1;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_Step_table),mean_value_index_MAX*Variance_index_MAX);
		memcpy_fromio(buf2_tmp+mean_value_index_MAX*Variance_index_MAX,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_DT_table),mean_value_index_MAX*Variance_index_MAX);
#else
		memcpy(buf2_tmp,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_Step_table),mean_value_index_MAX*Variance_index_MAX);
		memcpy(buf2_tmp+mean_value_index_MAX*Variance_index_MAX,(unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_DT_table),mean_value_index_MAX*Variance_index_MAX);
#endif

		memcpy(buf, (unsigned char *)buf2_tmp, size);

		*num_type = VIP_UINT8;
		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;

	case TOOLACCESS_VIP_DCC_Identity_Condition_Check: /*id: 1124 */
		if(size > 4)
			return -ID_SIZE_ERR;

		buf2_tmp = (unsigned char *)vip_malloc(4);
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf2_tmp,(unsigned char*)&(system_setting_info->DCC_FW_ONOFF),1);
		memcpy_fromio(buf2_tmp+1,(unsigned char*)&(smartPic_clue->PureColor),1);
		memcpy_fromio(buf2_tmp+2,(unsigned char*)&(smartPic_clue->Total_Hist_Cnt_less_than_one),1);
		memcpy_fromio(buf2_tmp+3,(unsigned char*)&(smartPic_clue->Hist_Y_Total_sum_less_than_Total_HistCnt_thl),1);
#else
		memcpy(buf2_tmp,(unsigned char*)&(system_setting_info->DCC_FW_ONOFF),1);
		memcpy(buf2_tmp+1,(unsigned char*)&(smartPic_clue->PureColor),1);
		memcpy(buf2_tmp+2,(unsigned char*)&(smartPic_clue->Total_Hist_Cnt_less_than_one),1);
		memcpy(buf2_tmp+3,(unsigned char*)&(smartPic_clue->Hist_Y_Total_sum_less_than_Total_HistCnt_thl),1);
#endif

		memcpy(buf, (unsigned char *)buf2_tmp, size);

		*num_type = VIP_UINT8;
		vip_free(buf2_tmp);
		buf2_tmp = NULL;

		break;
	case TOOLACCESS_VIP_DCC_ADV_AP_BLE:
	{
		VIP_DCC_ADV_API *pDCC_ADV_API;
		unsigned char tbl_sel;

		if(size > 4)
			return -ID_SIZE_ERR;

		pDCC_ADV_API = &vipTable_ShareMem->DCC_Control_Structure.DCC_ADV_API;

		buf2_tmp = (unsigned char *)vip_malloc(4);
		if (buf2_tmp == NULL)
			return -ID_SIZE_ERR;

		tbl_sel = pDCC_ADV_API->ADV_API_CTRL.TBL_Select_BLE;
		memcpy(buf2_tmp,(unsigned char*)&(pDCC_ADV_API->ADV_API_TBL[tbl_sel].ADV_API_BLE_TBL.Enable_Flag),1);
		memcpy(buf2_tmp+1,(unsigned char*)&(pDCC_ADV_API->ADV_API_TBL[tbl_sel].ADV_API_BLE_TBL.BLE_Lv),1);
		memcpy(buf2_tmp+2,(unsigned char*)&(pDCC_ADV_API->ADV_API_TBL[tbl_sel].ADV_API_BLE_TBL.isBLE_Lv_Reverse),1);
		memcpy(buf2_tmp+3,(unsigned char*)&(pDCC_ADV_API->ADV_API_TBL[tbl_sel].ADV_API_BLE_TBL.BLE_Range),1);
		memcpy(buf, (unsigned char *)buf2_tmp, size);

		*num_type = VIP_UINT8;
		vip_free(buf2_tmp);
		buf2_tmp = NULL;
	}
	break;
	case TOOLACCESS_ICM_Global_Adjust: /* 1200*/
		if (size > sizeof(RPC_ICM_Global_Ctrl))
			return -ID_SIZE_ERR;

		if (fwif_color_GetICM_Global_Ctrl_Struct() == NULL)
			return -READ_ERROR;

#ifdef CONFIG_ARM64
		memcpy_fromio(buf, (unsigned char *)fwif_color_GetICM_Global_Ctrl_Struct(), size);
#else
		memcpy(buf, (unsigned char *)fwif_color_GetICM_Global_Ctrl_Struct(), size);
#endif
		fwif_color_ChangeINT32Endian((int *)buf, size/4, 1);

		*num_type = VIP_INT32;

		break;
	case TOOLACCESS_ICM_BOX_Adjust: /*  1201*/
		if (size > sizeof(int)*6)
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(6 * sizeof(int));

		if (pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian_Copy(&g_h_sta, 1, &intCopyTemp, 1);
		pwIntTemp[0] = intCopyTemp;
		fwif_color_ChangeINT32Endian_Copy(&g_h_end, 1, &intCopyTemp, 1);
		pwIntTemp[1] = intCopyTemp;
		fwif_color_ChangeINT32Endian_Copy(&g_s_sta, 1, &intCopyTemp, 1);
		pwIntTemp[2] = intCopyTemp;
		fwif_color_ChangeINT32Endian_Copy(&g_s_end, 1, &intCopyTemp, 1);
		pwIntTemp[3] = intCopyTemp;
		fwif_color_ChangeINT32Endian_Copy(&g_i_sta, 1, &intCopyTemp, 1);
		pwIntTemp[4] = intCopyTemp;
		fwif_color_ChangeINT32Endian_Copy(&g_i_end, 1, &intCopyTemp, 1);
		pwIntTemp[5] = intCopyTemp;

		memcpy(buf, (unsigned char *)pwIntTemp, size);
		*num_type = VIP_INT32;

		vip_free(pwIntTemp);
		pwIntTemp = NULL;

		break;
	case TOOLACCESS_ICM_ReadWrite_Buf: /*  1202*/

		if (size > ((64*12*12*2+6)*sizeof(unsigned int)))
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)dvr_malloc_specific(80000 * sizeof(unsigned char), GFP_DCU1);
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		/* read star and end point*/
		pwUINT32Temp[0] = g_h_sta;
		pwUINT32Temp[1] = g_h_end;
		pwUINT32Temp[2] = g_s_sta;
		pwUINT32Temp[3] = g_s_end;
		pwUINT32Temp[4] = g_i_sta;
		pwUINT32Temp[5] = g_i_end;

		if (size > sizeof(unsigned int)*((g_i_end - g_i_sta + 1) * (g_s_end - g_s_sta + 1) * (g_h_end - g_h_sta + 1) * 2 + 6)) {
			dvr_free(pwUINT32Temp);
			pwUINT32Temp = NULL;
			return -ID_SIZE_ERR;
		}

		fwif_color_icm_SramBlockAccessSpeedup(pwUINT32Temp+6, 0,  g_h_sta,  g_h_end,  g_s_sta,  g_s_end,  g_i_sta,  g_i_end);
		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/4, 1);

		memcpy(buf, (unsigned char *)pwUINT32Temp, size);

		*num_type = VIP_UINT32;

		dvr_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;

	case TOOLACCESS_ICM_ReadWrite_CurrentPrevious_Buf: /*  1203*/
		if (size > sizeof(unsigned int))
			return -ID_SIZE_ERR;
		if (rtice_ICM_ReadWrite_From_CurrentPrevious_Buff(rtice_param, num_type, buf, system_setting_info, 0)  < 0)
			return -ID_SIZE_ERR;
		break;


	case TOOLACCESS_MPEG_SW_Ctrl:
		if (size > sizeof(DRV_MPEGNR_16_SW_CTRL))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc( sizeof(DRV_MPEGNR_16_SW_CTRL));

		if (pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		if(rtice_param.mode0 ==0)
			drvif_color_get_DRV_MPG_H_SW_Ctrl((DRV_MPEGNR_16_SW_CTRL*)pwIntTemp);
		else
			drvif_color_get_DRV_MPG_V_SW_Ctrl((DRV_MPEGNR_16_SW_CTRL*)pwIntTemp);

		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);
		memcpy(buf, (unsigned char *)pwIntTemp, size);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);

		break;


	case TOOLACCESS_PCID_Tbl: /*1250*/

		if (size > sizeof(char)*PCID_TABLE_LEN)
			return -ID_SIZE_ERR;

		pCharTemp = (char *)vip_malloc(sizeof(char)*PCID_TABLE_LEN);

		if(pCharTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_get_pcid_valuetable(pCharTemp, rtice_param.mode0, rtice_param.mode1);

		memcpy(buf, (char*)pCharTemp, size);

		*num_type = VIP_INT8;
		vip_free(pCharTemp);

		break;

#if defined(CONFIG_RTK_AI_DRV)
	case TOOLACCESS_AI_Ctrl_Tbl: /*1260 get*/

		if (size > sizeof(DRV_AI_Ctrl_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_Ctrl_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

                scaler_AI_obj_Ctrl_Get((DRV_AI_Ctrl_table*)pwIntTemp);
		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);

		memcpy(buf, (unsigned char *)pwIntTemp, size);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);



		break;

// lesley 0906
	case TOOLACCESS_AI_Ctrl_Draw: /*1261 get*/
		// 0622 lsy
		#if 0
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		buf[0] = vpq_stereo_face;
		#endif

		if (size > sizeof(unsigned char)*12)
			return -ID_SIZE_ERR;
		
		if( size >= sizeof(unsigned char)*1 )
			buf[0] = aipq_mode.clock_status;
		if( size >= sizeof(unsigned char)*2 )
			buf[1] = aipq_mode.ap_mode;
		if( size >= sizeof(unsigned char)*3 )
			buf[2] = aipq_mode.face_mode;
		if( size >= sizeof(unsigned char)*4 )
			buf[3] = aipq_mode.sqm_mode;
		if( size >= sizeof(unsigned char)*5 )
			buf[4] = aipq_mode.scene_mode;	
		if( size >= sizeof(unsigned char)*6 )
			buf[5] = aipq_mode.genre_mode;
		if( size >= sizeof(unsigned char)*7 )
			buf[6] = aipq_mode.depth_mode;
		if( size >= sizeof(unsigned char)*8 )
			buf[7] = aipq_mode.obj_mode;	
		if( size >= sizeof(unsigned char)*9 )
			buf[8] = aipq_mode.pqmask_mode;
		if( size >= sizeof(unsigned char)*10 )
			buf[9] = aipq_mode.semantic_mode;
		if( size >= sizeof(unsigned char)*11 )
			buf[10] = aipq_mode.logo_mode;
		if( size >= sizeof(unsigned char)*12 )
			buf[11] = aipq_mode.noise_mode;

		// end 0622
		*num_type = VIP_UINT8;
		break;
// end lesley 0906

// lesley 1002_1
	case TOOLACCESS_AI_INFO: /*1262 get*/

		if (size > sizeof(TOOL_AI_INFO))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(TOOL_AI_INFO));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

        drivef_tool_ai_info_get((TOOL_AI_INFO*)pwIntTemp);
		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);

		memcpy(buf, (unsigned char *)pwIntTemp, size);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);

		break;
// end lesley 1002_1

// lesley 1016
	case TOOLACCESS_AI_TUNE_ICM: /*1263 get*/
		if (size > sizeof(DRV_AI_Tune_ICM_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_Tune_ICM_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		drivef_ai_tune_icm_get((DRV_AI_Tune_ICM_table*) pwIntTemp);
		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);

		memcpy(buf, (unsigned char *)pwIntTemp, size);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);

		break;

	case TOOLACCESS_AI_TUNE_DCC: /*1264 get*/
		if (size > sizeof(DRV_AI_Tune_DCC_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_Tune_DCC_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		drivef_ai_tune_dcc_get((DRV_AI_Tune_DCC_table*) pwIntTemp);
		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);

		memcpy(buf, (unsigned char *)pwIntTemp, size);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);

		break;
	case TOOLACCESS_AI_DCC_USER_CURVE: /*1265 get*/
		if (size > sizeof(int)*129)
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(int)*129);

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		drivef_ai_dcc_user_curve_get((int*) pwIntTemp);
		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);

		memcpy(buf, (unsigned char *)pwIntTemp, size);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);

		break;
// end lesley 1016

	case TOOLACCESS_AI_DB: /*1266 get*/
	{
		extern DB_AI_RTK ai_db_set;

		if (size > sizeof(DB_AI_RTK))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DB_AI_RTK));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

        memcpy(pwIntTemp, &ai_db_set, sizeof(ai_db_set));
		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);

		memcpy(buf, (unsigned char *)pwIntTemp, size);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);

		break;
	}

	case TOOLACCESS_AI_SCENE_Ctrl: /*1270 get*/

		if (size > sizeof(DRV_AI_SCENE_Ctrl_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_SCENE_Ctrl_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		scaler_AI_Scene_Ctrl_Get((DRV_AI_SCENE_Ctrl_table*)pwIntTemp);
		fwif_color_ChangeINT32Endian(pwIntTemp, size>>2, 1);

		memcpy(buf, (unsigned char *)pwIntTemp, size);

		*num_type = VIP_INT32;

		vip_free(pwIntTemp);



		break;

	case TOOLACCESS_AI_SCENE_Ctrl_test:   	/*  1271*/

		// lesley test
		//buf[0] = nn_scene;
		*num_type = VIP_UINT8;
		break;
#endif

	case TOOLACCESS_DisplayInformation_table:   /*  1300*/
		if (size > sizeof(unsigned int)*DisplayInfo_Coef_MAX)
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(DisplayInfo_Coef_MAX * sizeof(unsigned int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		Scaler_Get_color_access_TVStatus_DisplayInfo(pwUINT32Temp);

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, DisplayInfo_Coef_MAX, 1);

		memcpy(buf, (unsigned char *)pwUINT32Temp, size);

		*num_type = VIP_UINT32;

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;


#if 1
	case TOOLACCESS_AutoMA_API_CTRL:
		if (size > sizeof(VIP_AutoMA_API_CTRL))
			return -ID_SIZE_ERR;
		buf_tmp8 = (unsigned char *)vip_malloc(sizeof(VIP_AutoMA_API_CTRL));
		if (buf_tmp8 == NULL)
			return -ID_SIZE_ERR;

		buf_tmp8[0] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.DI_MiddleWare_Function_En;
		buf_tmp8[1] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_SMD_weake_move_Detecte_En;
		buf_tmp8[2] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.compute_PQA_level_En;
		buf_tmp8[3] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_Profile_Block_Motion_En;
		buf_tmp8[4] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.noise_estimation_MAD_et_En;
		buf_tmp8[5] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.Y_U_V_Mad_Calculate_En;
		buf_tmp8[6] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_ET_Y_MOTION_En;
		buf_tmp8[7] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_FMV_HMCbin_hist_ratio_Cal_En;
		buf_tmp8[8] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.drvif_color_ClueGather_En;
		buf_tmp8[9] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.skin_tone_Level_detect_En;
		buf_tmp8[10] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.pure_color_detect_En;
		buf_tmp8[11] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_colorHistDetect_En;
		buf_tmp8[12] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_rtnr_noise_measure_En;
		buf_tmp8[13] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_color_noise_mpeg_detect_En;
		buf_tmp8[14] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.DI_detect_Champagnet_En;
		buf_tmp8[15] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.motion_concentric_circles_detect_En;;
		buf_tmp8[16] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.motion_purecolor_concentric_circles_detect_En;
		buf_tmp8[17] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.motion_move_concentric_circles_detect_En;
		buf_tmp8[18] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.drvif_color_colorbar_dectector_by_SatHueProfile_En;
		buf_tmp8[19] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.black_white_pattern_detect_En;
		buf_tmp8[20] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_Fade_Detection_En;
		buf_tmp8[21] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.histogram_new_III_dcc_EN;
		buf_tmp8[22] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_DCR_EN;
		buf_tmp8[23] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.p_film_detection_En;
		buf_tmp8[24] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_PQ_Adaptive_Table_En;
		buf_tmp8[25] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_MA_IEdgeSmooth_En;
		buf_tmp8[26] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_Dynamic_Vertical_NR_En;
		buf_tmp8[27] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.drvif_color_DCTI_for_ColorBar_En;
		buf_tmp8[28] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_color_noise_mpeg_apply_En;
		buf_tmp8[29] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_RTNR_correctionbit_period_En;
		buf_tmp8[30] = system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL.scalerVIP_DI_Dynamic_VDNoise_FrameMotion_En;
		memcpy(buf, (unsigned char *)buf_tmp8, size);
		*num_type = VIP_UINT8;
		vip_free(buf_tmp8);
		buf_tmp8 = NULL;

		break;

#endif
	/***************************for YUV2RGB Table access*****************************************/ /**/
	case TOOLACCESS_YUV2RGB_Global_Sat_Gain:
		if (rtice_SetGet_YUV2RGB_Table_Info(TOOLACCESS_YUV2RGB_Global_Sat_Gain, 0, size, num_type, buf, system_setting_info, gVip_Table) < 0)
			return -ID_SIZE_ERR;
	break;
	case TOOLACCESS_YUV2RGB_Global_Hue_Gain:
		if (rtice_SetGet_YUV2RGB_Table_Info(TOOLACCESS_YUV2RGB_Global_Hue_Gain, 0, size, num_type, buf, system_setting_info, gVip_Table) < 0)
			return -ID_SIZE_ERR;
	break;
	case TOOLACCESS_YUV2RGB_Global_Contrast_Gain:
		if (rtice_SetGet_YUV2RGB_Table_Info(TOOLACCESS_YUV2RGB_Global_Contrast_Gain, 0, size, num_type, buf, system_setting_info, gVip_Table) < 0)
			return -ID_SIZE_ERR;
	break;
	case TOOLACCESS_YUV2RGB_Global_Brightness_Gain:
		if (rtice_SetGet_YUV2RGB_Table_Info(TOOLACCESS_YUV2RGB_Global_Brightness_Gain, 0, size, num_type, buf, system_setting_info, gVip_Table) < 0)
			return -ID_SIZE_ERR;
	break;
	/***************************for YUV2RGB Table access*****************************************/ /**/


	case TOOLACCESS_Function_Coef_Dlti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetDLti();
		else
			buf[0] = VipPQ_GetDltiTableSize();

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Coef_Dcti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetDCti();
		else
			buf[0] = VipPQ_GetNewDctiTableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_iDlti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetIDLti();
		else
			buf[0] = 0;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_iDcti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetIDCti();
		else
			buf[0] = VipPQ_GetINewDctiTableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_VDcti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetVDCti();
		else
			buf[0] = VipPQ_GetVctiTableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_UV_Delay_En:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetColor_UV_Delay_Enable();
		else
			buf[0] = 1;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_UV_Delay:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetColor_UV_Delay();
		else
			buf[0] = 0xff;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_YUV2RGB:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_get_YUV2RGB();
		else
			buf[0] = 10;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_Film_Mode:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetFilm_Mode();
		else
			buf[0] = 3;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Coef_Intra:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetIntra();
		else
			buf[0] = 5;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_MA:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetMADI();
		else
			buf[0] = DI_MA_Adjust_Table_MAX-1;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_TnrXC:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetTNRXC_Ctrl();
		else
			buf[0] = VipPQ_GetTNRXC_TableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_Ma_Chroma_Error:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetMA_Chroma_Error();
		else
			buf[0] = 10;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_NM_Level:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = 0;//Scaler_GetRTNR_Noise_Measure();
		else
			buf[0] = 0;//VipPQ_GetRTNR_NM_TableSize(); mac6 remove this item

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Coef_Madi_Hme:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetMADI_HME();
		else
			buf[0] = VipPQ_GetHME_TableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_Madi_Hmc:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetMADI_HMC();
		else
			buf[0] = VipPQ_GetHMC_TableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_Madi_Pan:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetMADI_PAN();
		else
			buf[0] = VipPQ_GetPAN_TableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Coef_Di_Smd:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetDiSmd();
		else
			buf[0] = VipPQ_GetSMD_TableSize();

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Coef_NEW_UVC:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetNew_UVC();
		else
			buf[0] = new_UVC_MAX;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Coef_UV_DELAY_TOP:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR; //not used now

		if (rtice_param.mode0 == 0)
			buf[0] = 0;
		else
			buf[0] = 0;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Coef_V_DCTi_LPF:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetVDCti_LPF();
		else
			buf[0] = DCTI_TABLE_LEVEL_MAX;

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_Function_Extend_CDS:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetCDSTable();
		else
			buf[0] = Sharp_table_num;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend_Emfmk2:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetEMF_Mk2();
		else
			buf[0] = VipPQ_GetEmfMk2_TableSize();

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend_Dnoise_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			{}
		else
			buf[0] = 0;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend_Adatp_Gamma:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_get_Adaptive_Gamma();
		else
			buf[0] = VipPQ_GetAdaptive_Gamma_Ctrl_TableSize();

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend_LD_Ebabel:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
#ifdef LocalDimming

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_SetLocalDimmingEnable();
		else
			buf[0] = 1;
#else
		buf[0] = 255;
#endif
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend_LD_Table_Select:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
#ifdef LocalDimming

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_SetLocalDimmingTable();
		else
			buf[0] = VipPQ_GetLocal_Dimming_TableSize();
#else
		buf[0] = 255;
#endif
		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend_LC_Enable:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetLocalContrastEnable();
		else
			buf[0] = 1;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend_LC_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetLocalContrastTable();
		else
			buf[0] = 10;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend_3dLUT_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_Get3dLUT();
		else
			buf[0] = LUT3D_TBL_Num;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend_I_De_XC:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_Get_I_De_XC();
		else
			buf[0] = 10;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend_I_De_Contour:

			if (size > sizeof(unsigned char))
				return -ID_SIZE_ERR;

			if (rtice_param.mode0 == 0)
				buf[0] = Scaler_Get_I_De_Contour();
			else
				buf[0] = I_De_Contour_TBL_Max;

			*num_type = VIP_UINT8;

			break;
	case TOOLACCESS_Function_Extend_SLD:

			if (size > sizeof(unsigned char))
				return -ID_SIZE_ERR;

			if (rtice_param.mode0 == 0)
				buf[0] = Scaler_Get_SLD();
			else
				buf[0] = SLD_Table_NUM;

			*num_type = VIP_UINT8;

			break;

	case TOOLACCESS_Function_Extend2_SU_H:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);

		if (rtice_param.mode0 == 0)
			buf[0] = HY;
		else
			buf[0] = VipPQ_GetFIR_Coef_Ctrl_TableSize();
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SU_V:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);

		if (rtice_param.mode0 == 0)
			buf[0] = VY;
		else
			buf[0] = VipPQ_GetFIR_Coef_Ctrl_TableSize();
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_S_PK:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetScalePK();
		else
			buf[0] = VipPQ_GetFIR_Coef_Ctrl_TableSize();
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SUPK_Mask:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetSUPKType();
		else
			buf[0] = VipPQ_GetSupk_Mask_TableSize();
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_Unsharp_Mask:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetUnsharpMask();
		else
			buf[0] = VipPQ_GetUn_Shp_Mask_TableSize();
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_Reserve5:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = 0;
		else
			buf[0] = 0;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_Iegsm_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_getiESM();
		else
			buf[0] = 10;
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SR_Init_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = 0;
		else
			buf[0] = 0;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SR_Edge_Gain:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = 0;
		else
			buf[0] = 0;

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_Function_Extend2_SR_TEX_Gain:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = 0;
		else
			buf[0] = 0;

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_Function_Extend2_SD_H_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetScaleDOWNHTable();
		else
			buf[0] = 8;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SD_V_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetScaleDOWNVTable();
		else
			buf[0] = 8;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SD_444TO422:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetScaleDOWN444To422();
		else
			buf[0] = VipPQ_GetScaleDown_COEF_TableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_Color_Space_Control:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = 0;
		else
			buf[0] = 0;
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SU_H_8Tab:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetScaleUPH_12tap();
		else
			buf[0] = VipPQ_GetSU_table_12tap_TableSize();

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_Function_Extend2_SU_V_6Tab:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetScaleUPV_8tap();
		else
			buf[0] = VipPQ_GetSU_table_8tap_TableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_OSD_Sharpness:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = 0;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SU_DIR:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetScaleUPDir();
		else
			buf[0] = VipPQ_GetFIR_Coef_Ctrl_TableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SU_DIR_Weigh:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetScaleUPDir_Weighting();
		else
			buf[0] = VipPQ_GetDirsu_TableSize();

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SU_C_H:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);

		if (rtice_param.mode0 == 0)
			buf[0] = HC;
		else
			buf[0] =255;// VipPQ_GetFIR_Coef_Ctrl_TableSize();
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend2_SU_C_V:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);

		if (rtice_param.mode0 == 0)
			buf[0] = VC;
		else
			buf[0] =255;// VipPQ_GetFIR_Coef_Ctrl_TableSize();
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend3_VD_CON_BRI_HUE_SAT:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_module_vdc_GetConBriHueSat_Table();
		else
			buf[0] = 19;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend3_ICM_Tablee:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_get_ICM_table();
		else
			buf[0] = VIP_ICM_TBL_X;

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_Function_Extend3_Gamma:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetGamma();
		else
			buf[0] = t_GAMMA_MAX;

		*num_type = VIP_UINT8;

		break;
	case TOOLACCESS_Function_Extend3_S_Gamma_Index:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetSindex();
		else
			buf[0] = 255;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend3_S_Gamma_Low:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetSlow();
		else
			buf[0] = 255;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend3_S_Gamma_High:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetShigh();
		else
			buf[0] = 255;

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_Function_Extend3_DCC_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetDCC_Table();
		else
			buf[0] = 9;

		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_Function_Extend3_DCC_Color_Indep_t:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_Get_DCC_Color_Independent_Table();
		else
			buf[0] = 6;

		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_Function_Extend3_DCC_Chroma_Comp_t:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_Get_DCC_chroma_compensation_Table();
		else
			buf[0] = 9;

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_Function_Extend3_Sharpness_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetSharpnessTable();
		else
			buf[0] = 19;

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_Function_Extend3_NR_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = scaler_get_PQA_table();
		else
			buf[0] = PQA_TABLE_MAX;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend3_PQA_INPUT_TABLE:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = scaler_get_PQA_Input_table();
		else
			buf[0] = PQA_I_TABLE_MAX;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_Function_Extend3_MB_Peaking:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetMBPeaking();
		else
			buf[0] = MBPK_table_num;
		Scaler_SetMBPeaking(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_blue_stretch:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_Get_blue_stretch();
		else
			buf[0] = Blue_Stretch_MAX;

		break;
	case TOOLACCESS_Function_Extend3_MB_SU_Peaking:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		if (rtice_param.mode0 == 0)
			buf[0] = Scaler_GetMBSUPeaking();
		else
			buf[0] = MBSU_table_num;

		break;


	case TOOLACCESS_Function_Coef_ALL:
		if (size > VIP_QUALITY_SOURCE_NUM*21)
			return -ID_SIZE_ERR;

	/*IoReg_Write32(NR_DCH1_IMPULSE_SMOOTHTHL_VADDR, size);*/

		if (rtice_param.mode0 == 0)
			memcpy(buf, (unsigned char *) &(gVip_Table->VIP_QUALITY_Coef), size);
		else {
			tablesize[0] = VipPQ_GetDltiTableSize(); /*dlti*/
			tablesize[1] = VipPQ_GetNewDctiTableSize(); /*dcti*/
			tablesize[2] = 0; /*idlti*/
			tablesize[3] = VipPQ_GetINewDctiTableSize();/*idcti*/
			tablesize[4] = VipPQ_GetVctiTableSize(); /*vdcti*/
			tablesize[5] = 1;/*scaler_uv_delay_en*/
			tablesize[6] = 0xff;/*uv_delay_level*/
			tablesize[7] = YUV2RGB_TBL_Num;/*yuv2rgb*/
			tablesize[8] = 5;/*film_mode*/
			tablesize[9] = 5;/*intra*/
			tablesize[10] = DI_MA_Adjust_Table_MAX;/*ma*/
			tablesize[11] = VipPQ_GetTNRXC_TableSize();/*tnr_xc*/
			tablesize[12] = t_MA_Chroma_Error_MAX;/*chroma_error*/
			tablesize[13] = 0;/*nm_level*/
			tablesize[14] = VipPQ_GetHME_TableSize();/*hme*/
			tablesize[15] = VipPQ_GetHMC_TableSize();/*hmc*/
			tablesize[16] = VipPQ_GetPAN_TableSize();/*pan*/
			tablesize[17] = VipPQ_GetSMD_TableSize();/*smd*/
			tablesize[18] = new_UVC_MAX;/*resreve*/
			tablesize[19] = 0;/*reserve*/
			tablesize[20] = DCTI_TABLE_LEVEL_MAX;/*reserve*/

			buf2_tmp = (unsigned char *)vip_malloc(VIP_QUALITY_SOURCE_NUM*21);
			if (buf2_tmp == NULL)
				return -ID_SIZE_ERR;

			for (i = 0; i < VIP_QUALITY_SOURCE_NUM; i++) {
				memcpy(buf2_tmp+(i*21), &tablesize, 21);

			}

			memcpy(buf, (unsigned char *)(buf2_tmp), size);
			vip_free(buf2_tmp);
			buf2_tmp = NULL;


		}
		*num_type = VIP_UINT8;



		break;
	case TOOLACCESS_Function_Extend_ALL:
		if (size > VIP_QUALITY_SOURCE_NUM*21)
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			memcpy(buf, (unsigned char *) &(gVip_Table->VIP_QUALITY_Extend_Coef), size);
		else {

			/* [0: CDS]				[1: EMFMk2]			[2: reserved]*/
			/* [3: reserved]		[4: dnoise_table]			[5: ADAPT_GAMMA]*/
			/* [6: LD_Enable]			[7: LD_Table_Select]				[8: Dummy]*/

			tablesize[0] = Sharp_table_num; /*CDS*/
			tablesize[1] = VipPQ_GetEmfMk2_TableSize(); /*EMFMK2*/
			tablesize[2] = 0;// reserved
			tablesize[3] = 0;// reserved
			tablesize[4] = 0; /*dnoise_table*/
			tablesize[5] = 0;/*ADAPT_GAMMA*/
			tablesize[6] = 1;/*LD_Enable*/
			tablesize[7] = VipPQ_GetLocal_Dimming_TableSize();/*LD_Table_Select*/

			tablesize[8] = 1;/*LC_Enable*/
			tablesize[9] = 10;/*LC_Table*/
			tablesize[10] = LUT3D_TBL_Num;/*LUT3D_TBL*/
			tablesize[11] = 10;/*De_XC*/
			tablesize[12] = I_De_Contour_TBL_Max;/*I_De_Contour*/
			tablesize[13] = SLD_Table_NUM;/*SLD*/
			tablesize[14] = 10;/*Output_InvOutput_GAMMA*/
			tablesize[15] = 0;/*Tone mapping */
			tablesize[16] = 0;/*Reserve*/
			tablesize[17] = 0;/*Reserve*/
			tablesize[18] = 0;/*Reserve*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[20] = 0;/*Reserve*/

			buf2_tmp = (unsigned char *)vip_malloc(VIP_QUALITY_SOURCE_NUM*21);
			if (buf2_tmp == NULL)
				return -ID_SIZE_ERR;

			for (i = 0; i < VIP_QUALITY_SOURCE_NUM; i++) {
				memcpy(buf2_tmp+(i*21), &tablesize, 21);

			}

			memcpy(buf, (unsigned char *)(buf2_tmp), size);
			vip_free(buf2_tmp);
			buf2_tmp = NULL;


		}
		*num_type = VIP_UINT8;



		break;
	case TOOLACCESS_Function_Extend2_ALL:
		if (size > VIP_QUALITY_SOURCE_NUM*21)
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			memcpy(buf, (unsigned char *) &(gVip_Table->VIP_QUALITY_Extend2_Coef), size);
		else {
			tablesize[0] = VipPQ_GetFIR_Coef_Ctrl_TableSize(); /*SU_H*/
			tablesize[1] = VipPQ_GetFIR_Coef_Ctrl_TableSize(); /*SU_V*/
			tablesize[2] = VipPQ_GetFIR_Coef_Ctrl_TableSize(); /*S_PK*/
			tablesize[3] = VipPQ_GetSupk_Mask_TableSize();/*SUPK_MASK*/
			tablesize[4] = VipPQ_GetUn_Shp_Mask_TableSize(); /*Unsharp_Mask*/
			tablesize[5] = 4;/*EGSM_Postshp_Table*/
			tablesize[6] = 9;/*iEGSM_table*/
			tablesize[7] = 0;/*SR_init_table*/

			tablesize[8] = 0;/*SR_edge_gain*/
			tablesize[9] = 0;/*SR_tex_gain*/
			tablesize[10] = 7;/*SD_H_table*/
			tablesize[11] = 7;/*SD_V_table*/
			tablesize[12] = VipPQ_GetScaleDown_COEF_TableSize();/*VipPQ_GetScaleDown_COEF_TableSize*/
			tablesize[13] = 0;/*color space control*/
			tablesize[14] = VipPQ_GetSU_table_12tap_TableSize();/*SU_H_12tab*/
			tablesize[15] = VipPQ_GetSU_table_8tap_TableSize();/*SU_V_8tab*/
			tablesize[16] = 0;/*OSD_Sharpness*/
			tablesize[17] = VipPQ_GetFIR_Coef_Ctrl_TableSize();/*SU_Dir*/
			tablesize[18] = VipPQ_GetDirsu_TableSize();/*SU_Dir_Weighing*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[20] = 0;/*Reserve*/

			buf2_tmp = (unsigned char *)vip_malloc(VIP_QUALITY_SOURCE_NUM*21);
			if (buf2_tmp == NULL)
				return -ID_SIZE_ERR;

			for (i = 0; i < VIP_QUALITY_SOURCE_NUM; i++) {
				memcpy(buf2_tmp+(i*21), &tablesize, 21);

			}

			memcpy(buf, (unsigned char *)(buf2_tmp), size);
			vip_free(buf2_tmp);
			buf2_tmp = NULL;


		}
		*num_type = VIP_UINT8;



		break;
	case TOOLACCESS_Function_Extend3_ALL:
		if (size > VIP_QUALITY_SOURCE_NUM*21)
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0)
			memcpy(buf, (unsigned char *) &(gVip_Table->VIP_QUALITY_Extend2_Coef), size);
		else {
			tablesize[0] = 19; /*VD_ConBriHueSat_Table*/
			tablesize[1] = 4; /*ICM_TABLE_Apply*/
			tablesize[2] = 9; /*Gamma*/
			tablesize[3] = 255;/*S_Gamma_index*/
			tablesize[4] = 255; /*S_Gamma_Low*/
			tablesize[5] = 255;/*S_Gamma_High*/
			tablesize[6] = 9;/*DCC_Table*/
			tablesize[7] = 6;/*DCC_Color_Indep_t*/
			tablesize[8] = 9;/*DCC_chroma_comp_t*/
			tablesize[9] = 19;/*Sharpness_table*/
			tablesize[10] = PQA_TABLE_MAX-1;/*NR_Table*/
			tablesize[11] = PQA_I_TABLE_MAX-1;/*PQA_Input_Table*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[19] = 0;/*Reserve*/
			tablesize[20] = 0;/*Reserve*/

			buf2_tmp = (unsigned char *)vip_malloc(VIP_QUALITY_SOURCE_NUM*21);
			if (buf2_tmp == NULL)
				return -ID_SIZE_ERR;

			for (i = 0; i < VIP_QUALITY_SOURCE_NUM; i++) {
				memcpy(buf2_tmp+(i*21), &tablesize, 21);

			}

			memcpy(buf, (unsigned char *)(buf2_tmp), size);
			vip_free(buf2_tmp);
			buf2_tmp = NULL;


		}
		*num_type = VIP_UINT8;



		break;
	case TOOLACCESS_LocalContrast_Curve:
	{
		if (size > sizeof(unsigned int)*(256))
			return -ID_SIZE_ERR;


		pwUINT32Temp = (unsigned int *)vip_malloc(256 * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		drvif_color_get_LC_ToneMappingSlopePoint(pwUINT32Temp);

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);

		memcpy(buf, pwUINT32Temp, size);
		*num_type = VIP_UINT32;

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

	}
	break;

	case TOOLACCESS_ColorGamut_Matrix:

		if (size > 9 * sizeof(short))
			return -ID_SIZE_ERR;

		pwINT16Temp = (short *)vip_malloc(9 * sizeof(short));

		if (pwINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwINT16Temp, (char *)gColorGamutMatrix_Apply, size);

		fwif_color_ChangeINT16Endian(pwINT16Temp, 9, 1);

		if (rtice_param.mode0 == 0) {
			memcpy(buf, (char *)pwINT16Temp, size);
		} else {
			buf[0] = 4;
			buf[1] = 0;
			buf[2] = 0;
			buf[3] = 0;
			buf[4] = 0;
			buf[5] = 0;
			buf[6] = 0;
			buf[7] = 0;
			buf[8] = 4;
			buf[9] = 0;
			buf[10] = 0;
			buf[11] = 0;
			buf[12] = 0;
			buf[13] = 0;
			buf[14] = 0;
			buf[15] = 0;
			buf[16] = 4;
			buf[17] = 0;
		}

		*num_type = VIP_INT16;
		vip_free(pwINT16Temp);
		pwINT16Temp = NULL;

		break;

	case TOOLACCESS_ICM_GainAdjust:
	{
		extern UINT8 g_Color_Mode;
		extern CHIP_CM_REGION_T g_cm_rgn_adv;
		extern CHIP_CM_REGION_T g_cm_rgn_exp;
		extern CHIP_COLOR_CONTROL_T g_cm_ctrl_adv;
		extern CHIP_COLOR_CONTROL_T g_cm_ctrl_exp;

		CHIP_COLOR_ICM_GAIN_ADJUST_T *pGainAdj = NULL;
		CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T *pGainAdjRTICE = NULL;

		if (size > sizeof(CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T))
			return -ID_SIZE_ERR;

		pGainAdj = (CHIP_COLOR_ICM_GAIN_ADJUST_T *)vip_malloc(sizeof(CHIP_COLOR_ICM_GAIN_ADJUST_T));
		if (pGainAdj == NULL)
			return -ID_SIZE_ERR;

		pGainAdjRTICE = (CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T *)vip_malloc(sizeof(CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T));
		if (pGainAdjRTICE == NULL) {
			vip_free(pGainAdj);
			pGainAdj = NULL;
			return -ID_SIZE_ERR;
		}

		memcpy(&pGainAdj->stRgn, (COLOR_MODE_ADV == g_Color_Mode)?&g_cm_rgn_adv.stColorRegionType:&g_cm_rgn_exp.stColorRegionType, sizeof(CHIP_COLOR_REGION_TYPE_T));
		memcpy(&pGainAdj->stCtrl, (COLOR_MODE_ADV == g_Color_Mode)?&g_cm_ctrl_adv:&g_cm_ctrl_exp, sizeof(CHIP_COLOR_CONTROL_T));
		rtice_Covert_ICM_Gain_Struct(pGainAdj, pGainAdjRTICE, 0);
		fwif_color_ChangeUINT16Endian((unsigned short*)pGainAdjRTICE, sizeof(CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T)/sizeof(unsigned short), 1);

		if (rtice_param.mode0 == 0) {
			memcpy(buf, pGainAdjRTICE, size);
		} else {
			*(unsigned short *)buf = sizeof(CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T);
		}

		*num_type = VIP_UINT16;
		vip_free(pGainAdj);
		vip_free(pGainAdjRTICE);
		pGainAdj = NULL;
		pGainAdjRTICE = NULL;

		break;
	}

	case TOOLACCESS_DSE_Gain:
	{
		if (size > sizeof(unsigned short)* (VPQ_SaturationLUT_Seg_Max * 3))
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0) {
			VPQ_SAT_LUT_T lut;
			pwINT16Temp = (unsigned short *)vip_malloc((VPQ_SaturationLUT_Seg_Max * 3) * sizeof(unsigned short));
			if (pwINT16Temp) {
				//sat gain
				fwif_color_access_Sat_Gain_tv006(_MAIN_DISPLAY, &lut, ACCESS_MODE_GET);
				memcpy(pwINT16Temp, lut.uSaturationY, VPQ_SaturationLUT_Seg_Max * sizeof(unsigned short));

				//DSE
				fwif_color_access_SaturationLUT_tv006(_MAIN_DISPLAY, &lut, ACCESS_MODE_GET);
				memcpy(pwINT16Temp + VPQ_SaturationLUT_Seg_Max, lut.uSaturationY, VPQ_SaturationLUT_Seg_Max * sizeof(unsigned short));


				//total gain
				fwif_color_access_Total_Sat_Gain_tv006(_MAIN_DISPLAY, &lut, ACCESS_MODE_GET);
				memcpy(pwINT16Temp + (VPQ_SaturationLUT_Seg_Max * 2), lut.uSaturationY, VPQ_SaturationLUT_Seg_Max * sizeof(unsigned short));


				fwif_color_ChangeUINT16Endian(pwINT16Temp, (VPQ_SaturationLUT_Seg_Max * 3), 1);
				memcpy(buf, pwINT16Temp, size);
				vip_free(pwINT16Temp);
				pwINT16Temp = NULL;
			}
		} else {
			buf[0] = VPQ_SaturationLUT_Seg_Max;
		}

		*num_type = VIP_UINT16;
		break;
	}
	case TOOLACCESS_D_3D_LUT_Index:
	{
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = fwif_color_3d_lut_access_index(ACCESS_MODE_GET, 0);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_D_3D_LUT_Reset:
	{
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = 0;

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_OD_Table:
	{
		extern unsigned char od_table_tv006_store[OD_table_length];
		if (size > sizeof(unsigned char)*OD_table_length)
			return -ID_SIZE_ERR;

		memcpy(buf, od_table_tv006_store, size);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_OD_Table_17x17:
	{
		extern unsigned char od_table_tv006_store[OD_table_length];
		//extern unsigned int od_delta_table[OD_table_length];

		if (size > sizeof(unsigned char)*OD_table_length)
			return -ID_SIZE_ERR;

		// add for rtice tool
		drvif_color_od_table_read(od_table_tv006_store, 3, 0); // all channel, rbus lut

		memcpy(buf, od_table_tv006_store, size);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_VPQ_IOCTL_StopRun_cmd:
	{
		int i;

		if (size > sizeof(unsigned char)*256)
			return -ID_SIZE_ERR;

		for (i = 0; i < 256 && i < size/sizeof(unsigned char); i++)
			buf[i] = vpq_ioctl_get_stop_run_by_idx(i);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_VPQ_LED_IOCTL_StopRun_cmd:
	{
		int i;

		if (size > sizeof(unsigned char)*16)
			return -ID_SIZE_ERR;

		for (i = 0; i < 16 && i < size/sizeof(unsigned char); i++)
			buf[i] = vpq_led_ioctl_get_stop_run_by_idx(i);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_VPQ_MEMC_IOCTL_StopRun_cmd:
	{
		int i;

		if (size > sizeof(unsigned char)*32)
			return -ID_SIZE_ERR;

		for (i = 0; i < 32 && i < size/sizeof(unsigned char); i++)
			buf[i] = vpq_memc_ioctl_get_stop_run_by_idx(i);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_VPQ_MEMC_DynamicBypass:
	{
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = vbe_disp_get_dynamic_memc_bypass_flag();

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_VPQ_DIGameMode_Only:
	{
		extern unsigned char g_bDIGameModeOnlyDebug;
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = g_bDIGameModeOnlyDebug;

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_VPQ_IOCTL_StopRun:
	{
		VPQ_IOCTL_STOP_RUN i;

		if (size > sizeof(unsigned char)*STOP_VPQ_IOC_MAX)
			return -ID_SIZE_ERR;

		for (i = STOP_VPQ_IOC_START; i < STOP_VPQ_IOC_MAX && i < size/sizeof(unsigned char); i++)
			buf[i] = fwif_color_vpq_stop_ioctl(i, ACCESS_MODE_GET, 0);

		*num_type = VIP_UINT8;
		break;
	}
	case TOOLACCESS_DemoOverscan:
		if (size > sizeof(unsigned short)*9)
			return -ID_SIZE_ERR;

		if (rtice_param.mode0 == 0) {
			if (fwif_color_get_demo_callback(DEMO_CALLBACKID_OVERSCAN)) {
				pwINT16Temp = (unsigned short *)vip_malloc(9 * sizeof(unsigned short));
				if (pwINT16Temp) {
					pwINT16Temp[0] = 1;
					(fwif_color_get_demo_callback(DEMO_CALLBACKID_OVERSCAN))((void *)pwINT16Temp);
					pwINT16Temp[0] = 2;
					fwif_color_ChangeUINT16Endian(pwINT16Temp, 9, 1);
					memcpy(buf, pwINT16Temp, sizeof(unsigned short)*9);
					vip_free(pwINT16Temp);
					pwINT16Temp = NULL;
				}
			} else {
				memset(buf, 0, sizeof(unsigned short)*9);
			}
		} else {
			buf[0] = 9;
		}

		*num_type = VIP_UINT16;
		break;

	case TOOLACCESS_DemoPQ:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = (fwif_color_get_demo_callback(DEMO_CALLBACKID_ON_OFF_SWITCH) == NULL)?0:1;

		*num_type = VIP_UINT8;

		break;


	case TOOLACCESS_ONEKEY_HDR:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = 1;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_ONEKEY_BBC:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = 1;

		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_ONEKEY_SDR2HDR:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = 1;

		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_TC_DEMO:
	{
		DRV_TC_HDR_CTRL *pTC_HDR_CTRL = (DRV_TC_HDR_CTRL*)fwif_color_Get_TC_CTRL();

		if (size > sizeof(DRV_TC_HDR_CTRL))
			return -ID_SIZE_ERR;

		memcpy(	&buf[0], (unsigned char*)pTC_HDR_CTRL, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;
		break;
	}

	case TOOLACCESS_ST2094_CTRL:
		if (size > sizeof(VIP_ST2094_CTRL))
			return -ID_SIZE_ERR;
		ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();

		ST2094_CTRL->Debug_Log = RPC_system_setting_info->HDR_info.Debug_Log_3;
		ST2094_CTRL->Debug_Delay = RPC_system_setting_info->HDR_info.Debug_Log_Delay_Frame;

		memcpy(	&buf[0], (unsigned char*)ST2094_CTRL, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_ST2094_METADATA:
		if (size > sizeof(ST2094_metadata))
			return -ID_SIZE_ERR;

		ST2094_metadata = drvif_fwif_color_Get_ST2094_METADATA_Buff();
		memcpy(	&buf[0], (unsigned char*)ST2094_metadata, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_ST2094_METADATA_Curve_Info:
	{
		if (size > sizeof(int) * ST2094_METADATA_Curve_Info_Max)
			return -ID_SIZE_ERR;

		ST2094_metadata = drvif_fwif_color_Get_ST2094_METADATA_Buff();
		pwUINT32Temp = (unsigned int *)vip_malloc(ST2094_METADATA_Curve_Info_Max * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		pwUINT32Temp[0] = ST2094_metadata->knee_point_x;	/* in 12 bits*/
		pwUINT32Temp[1] = ST2094_metadata->knee_point_y;	/* in 12 bits*/
		pwUINT32Temp[2] = ST2094_metadata->num_bezier_curve_anchors;
		for(i=0;i<bezier_curve_anchors_num;i++)
			pwUINT32Temp[i+3] = ST2094_metadata->bezier_curve_anchors[i];	/* in 10 bits */
		pwUINT32Temp[18] = ST2094_metadata->targeted_system_display_maximum_luminance;	/* unit is nits. */
		pwUINT32Temp[19] = ST2094_metadata->maxscl[0]; /* unit is nits/10,  ie. maxscl/10 == nits*/
		pwUINT32Temp[20] = ST2094_metadata->maxscl[1]; /* " unit is nits/10 */
		pwUINT32Temp[21] = ST2094_metadata->maxscl[2]; /* unit is nits/10 */
		for(i=0;i<distribution_maxrgb_bin_num;i++)
			pwUINT32Temp[i+22] = ST2094_metadata->distribution_maxrgb_percentages[i];
		for(i=0;i<distribution_maxrgb_bin_num;i++)
			pwUINT32Temp[i+32] = ST2094_metadata->distribution_maxrgb_percentiles[i];	/* unit is nits/10 */

		memcpy(	&buf[0], (unsigned char*)pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);
		break;
	}

	case TOOLACCESS_ST2094_Basis_OOTF_Info:
	{
		if (size > sizeof(int) * ST2094_Basis_OOTF_Info_Max)
			return -ID_SIZE_ERR;

		ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();
		pwUINT32Temp = (unsigned int *)vip_malloc(ST2094_Basis_OOTF_Info_Max * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		pwUINT32Temp[0] = ST2094_CTRL->Debug_basisOOTF_kneePx;	/* in 12 bits*/
		pwUINT32Temp[1] = ST2094_CTRL->Debug_basisOOTF_kneePy;	/* in 12 bits*/
		pwUINT32Temp[2] = drvif_fwif_color_Get_ST2094_TBL()->basisOOTF_order;
		for(i=0;i<bezier_curve_anchors_num;i++)
			pwUINT32Temp[i+3] = ST2094_CTRL->Debug_basisOOTF_P[i];	/* in 10 bits*/
		pwUINT32Temp[18] = ST2094_CTRL->Debug_basisOOTF_Fixed_KneePoint;
		pwUINT32Temp[19] = ST2094_CTRL->Debug_basisOOTF_Fixed_Anchors;

		memcpy(	&buf[0], (unsigned char*)pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);
		break;
	}

	case TOOLACCESS_ST2094_Guided_OOTF_Info:
	{
		if (size > sizeof(int) * ST2094_Guided_OOTF_Info_Max)
			return -ID_SIZE_ERR;

		ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();
		pwUINT32Temp = (unsigned int *)vip_malloc(ST2094_Guided_OOTF_Info_Max * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		pwUINT32Temp[0] = ST2094_CTRL->Debug_GuidedOOTF_kneePx;	/* in 12 bits*/
		pwUINT32Temp[1] = ST2094_CTRL->Debug_GuidedOOTF_kneePy;	/* in 12 bits*/
		for(i=0;i<bezier_curve_anchors_num;i++)
			pwUINT32Temp[i+2] = ST2094_CTRL->Debug_GuidedOOTF_P[i];	/* in 10 bits*/

		memcpy(	&buf[0], (unsigned char*)pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);
		break;
	}

	case TOOLACCESS_ST2094_OETF_CTRL_Info:
	{
		if (size > sizeof(int) * ST2094_OETF_CTRL_Info_Max)
			return -ID_SIZE_ERR;

		ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();

		ST2094_CTRL->Debug_Log = RPC_system_setting_info->HDR_info.Debug_Log_3;
		ST2094_CTRL->Debug_Delay = RPC_system_setting_info->HDR_info.Debug_Log_Delay_Frame;

		pwUINT32Temp = (unsigned int *)vip_malloc(ST2094_OETF_CTRL_Info_Max * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		pwUINT32Temp[0] = ST2094_CTRL->Enable;
		pwUINT32Temp[1] = drvif_fwif_color_Get_ST2094_TBL()->product_peak;
		pwUINT32Temp[2] = drvif_fwif_color_Get_ST2094_TBL()->product_min;
		pwUINT32Temp[3] = drvif_fwif_color_Get_ST2094_TBL()->basis_peak;
		pwUINT32Temp[4] = drvif_fwif_color_Get_ST2094_TBL()->curve_sel;
		pwUINT32Temp[5] = drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div;
		pwUINT32Temp[6] = drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W;
		pwUINT32Temp[7] = drvif_fwif_color_Get_ST2094_TBL()->EOTF_Gamma22;
		pwUINT32Temp[8] = ST2094_CTRL->Debug_Log;
		pwUINT32Temp[9] = ST2094_CTRL->Debug_Delay;

		memcpy(	&buf[0], (unsigned char*)pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);
		break;
	}

	case TOOLACCESS_ST2094_OETF_Curve:
	{
		if (size > sizeof(int) * ST2094_OETF_Curve_Max)
			return -ID_SIZE_ERR;

		ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();
		pwUINT32Temp = (unsigned int *)vip_malloc(ST2094_OETF_Curve_Max * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		for(i=0;i<OETF_size;i++)
			pwUINT32Temp[i] = knee_curve_ST2094[i];
		for(i=0;i<OETF_size;i++)
			pwUINT32Temp[i+OETF_size] = OETF_curve_ST2094[i];

		memcpy(	&buf[0], (unsigned char*)pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);
		break;
	}

	case TOOLACCESS_HDR_VIVI_CTRL:
	{
		VIP_HDR10_vivid_CTRL* hdr_vivd_CTRL = NULL;
		
		if ((size+start_addr) > sizeof(VIP_HDR10_vivid_CTRL))
			return -ID_SIZE_ERR;

		hdr_vivd_CTRL = drvif_fwif_color_Get_HDR10_vivid_Ctrl();
		pwUINT32Temp = ((unsigned int*)hdr_vivd_CTRL) + start_addr;

		memcpy(&buf[0], pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		*num_type = VIP_UINT32;

		break;
	}

#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
	case TOOLACCESS_OSD_HDRMODE_DEMO:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		buf[0] = 1;

		if(OSD_HDRMODE_DEMO_init == 0){
			// init table
			fwif_color_set_OSD_enhance_init();
			OSD_HDRMODE_DEMO_init = 1;
		}

		*num_type = VIP_UINT8;
		break;
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE

	case TOOLACCESS_LOCALCONTRAST_DEMO:
		buf[0] = 1;
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_DM_HDR_3D_Lut_TBL:
		if (size > (sizeof(unsigned int) * (VIP_DM_HDR_3D_LUT_UI_TBL_SIZE)))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(VIP_DM_HDR_3D_LUT_UI_TBL_SIZE * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		buf2_tmp = (unsigned char *)vip_malloc(1 * sizeof(char));
		if (buf2_tmp == NULL) {
			vip_free(pwUINT32Temp);
			return -ID_SIZE_ERR;
		}
		fwif_color_get_Read_DM_HDR_3dLUT(buf2_tmp, pwUINT32Temp, HDR_3DLUT_getFromReg);

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);

		memcpy(buf, pwUINT32Temp, size);
		*num_type = VIP_UINT32;

		vip_free(buf2_tmp);
		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;

	case TOOLACCESS_DM2_HDR_EOTF_TBL:
	case TOOLACCESS_DM2_HDR_OETF_TBL:
	case TOOLACCESS_DM2_HDR_Tone_Mapping_TBL:
	case TOOLACCESS_DM2_HDR_24x24x24_3D_TBL:
		ret = rtice_SetGet_HDR_DM2(rtice_param, num_type, buf, system_setting_info, 0);
		if ( ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_D_3D_Lut_TBL:
		if (size > (sizeof(unsigned int) * (VIP_D_3D_LUT_UI_TBL_SIZE)))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(VIP_D_3D_LUT_UI_TBL_SIZE * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		buf2_tmp = (unsigned char *)vip_malloc(1 * sizeof(char));
		if (buf2_tmp == NULL) {
			vip_free(pwUINT32Temp);
			return -ID_SIZE_ERR;
		}

		fwif_color_get_Read_D_3dLUT(buf2_tmp, pwUINT32Temp);

		if (rtice_param.mode0 == 1) {
			rtice_Set_D_LUT_17to9(pwUINT32Temp);
		}

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);

		memcpy(buf, pwUINT32Temp, size);
		*num_type = VIP_UINT32;

		vip_free(buf2_tmp);
		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;

    case TOOLACCESS_Scaler_Coefficient: /*1951*/
    {
        extern unsigned char a_rgb_scale_down_coef_id_h[c_rgb_scale_down_ratio_seg];
        extern unsigned char a_rgb_scale_down_coef_id_v[c_rgb_scale_down_ratio_seg];
        extern unsigned char a_rgb_scale_up_coef_id_h[c_rgb_scale_up_ratio_seg];
        extern unsigned char a_rgb_scale_up_coef_id_v[c_rgb_scale_up_ratio_seg];
        
        if (size > sizeof(unsigned char)*24)
			return -ID_SIZE_ERR;

        for (i=0;i<6;i++){
		    buf[i]=a_rgb_scale_up_coef_id_h[i];
            buf[i+6]=a_rgb_scale_up_coef_id_v[i];
            buf[i+12]=a_rgb_scale_down_coef_id_h[i];
            buf[i+18]=a_rgb_scale_down_coef_id_v[i];
        }
		*num_type = VIP_UINT8;
		break;

    }
    break;
    
	case TOOLACCESS_DM2_EOTF_byLuminance_CTRL: /*1930*/
		if (size > (sizeof(unsigned short)*8))
			return -ID_SIZE_ERR;
		
		memcpy((unsigned char*)&buf[0], (unsigned char*)&(HDR10_EOTF_ByLuminace_TBL_cur.EOTF_setting[0]), size);
		fwif_color_ChangeUINT16Endian((unsigned short*)&buf[0], size/sizeof(short), 1);

		*num_type = VIP_UINT16;

		break;

	case TOOLACCESS_DM2_OETF_byLuminance_CTRL: /*1931*/
		if (size > (sizeof(unsigned int)*32))
			return -ID_SIZE_ERR;

		memcpy((unsigned int*)&buf[0], (unsigned int*)&(HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[0]), size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/(sizeof(int)), 1);

		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_DM2_panel_luminance: /*1932*/
		if (size > (sizeof(unsigned short)*1))
			return -ID_SIZE_ERR;

		memcpy((unsigned short*)&buf[0], (unsigned short*)&panel_luminance, size);
		fwif_color_ChangeUINT16Endian((unsigned short*)&buf[0], size/(sizeof(short)), 1);

		*num_type = VIP_UINT16;

		break;

	case TOOLACCESS_DM2_HDR_3D_TBL_FromReg:
		if (size > (sizeof(unsigned char)))
			return -ID_SIZE_ERR;

		buf[0] = HDR_3DLUT_getFromReg;
		*num_type = VIP_UINT8;

		break;

#if 1
	case TOOLACCESS_VIP_System_Info:
		if(size > sizeof(unsigned int)*VIP_System_Info_Items_Num)
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(VIP_System_Info_Items_Num*sizeof(unsigned int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		rtice_Get_VIP_System_Info(system_setting_info, RPC_system_setting_info, gVip_Table, pwUINT32Temp);
		fwif_color_ChangeUINT32Endian(pwUINT32Temp,size/sizeof(unsigned int), 1);
		memcpy(buf,(unsigned char*)pwUINT32Temp, size);

		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);

		break;

	case TOOLACCESS_VO_Info:
		if(size > sizeof(unsigned int)*VO_Info_Items_Num)
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(VO_Info_Items_Num*sizeof(unsigned int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		rtice_Get_VO_Info(system_setting_info, gVip_Table, pwUINT32Temp);
		fwif_color_ChangeUINT32Endian(pwUINT32Temp,size/sizeof(unsigned int), 1);
		memcpy(buf,(unsigned char*)pwUINT32Temp, size);

		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);

		break;

	case TOOLACCESS_DRM_Info_Frame:
		if(size > sizeof(unsigned int)*HDMI_DRM_Info_Items_Num)
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(HDMI_DRM_Info_Items_Num*sizeof(unsigned int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		rtice_Get_HDMI_DRM_Info(system_setting_info, gVip_Table, pwUINT32Temp);
		fwif_color_ChangeUINT32Endian(pwUINT32Temp,size/sizeof(unsigned int), 1);
		memcpy(buf,(unsigned char*)pwUINT32Temp, size);

		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);

		break;

	case TOOLACCESS_AVI_Info_Frame:
		if(size > sizeof(unsigned int)*HDMI_AVI_Info_Frame_Items_Num)
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(HDMI_AVI_Info_Frame_Items_Num*sizeof(unsigned int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		rtice_Get_HDMI_AVI_Info(system_setting_info, gVip_Table, pwUINT32Temp);
		fwif_color_ChangeUINT32Endian(pwUINT32Temp,size/sizeof(unsigned int), 1);
		memcpy(buf,(unsigned char*)pwUINT32Temp, size);

		*num_type = VIP_UINT32;
		vip_free(pwUINT32Temp);
		return -ID_SIZE_ERR;

		break;

#endif

	case TOOLACCESS_VIP_PQ_Power_Saving:
		ret = rtice_SetGet_PQ_Power_Saving(rtice_param, num_type, buf, system_setting_info, 0);
		if ( ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_10p_gamma_offset_gain:
		if (size > (sizeof(unsigned int)))
			return -ID_SIZE_ERR;

		pwCopyTemp = fwif_color_set_APDEM_10p_Offset_gainVal(0, 0);
		fwif_color_ChangeUINT32Endian(&pwCopyTemp, 1, 1);
		
		memcpy(buf, &pwCopyTemp, size);
		
		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_EN:
		if (size > (sizeof(unsigned char)))
			return -ID_SIZE_ERR;

		buf[0] = Scaler_APDEM_En_Access(0, 0);
		*num_type = VIP_UINT8;

		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_TEST:
		if (size > (sizeof(unsigned int)))
			return -ID_SIZE_ERR;

		buf[0] = Scaler_APDEM_En_Access(0, 0);
		
		fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(unsigned int), 1);

		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_DEM_PANEL_INI_TBL:
		if ((size + start_addr) > (sizeof(VIP_DEM_PANEL_INI_TBL)))
			return -ID_SIZE_ERR;

		pwUINT32Temp = ((unsigned int*)&VIP_AP_DEM_TBL.PANEL_DATA_) + start_addr;

		memcpy(&buf[0], pwUINT32Temp, size);
		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);

		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_PTG_CTRL:
		if ((size + start_addr) > (sizeof(VIP_APDEM_PTG_CTRL)))
			return -ID_SIZE_ERR;

		pwIntTemp = ((int*)&Hal_APDEM_PTG_CTRL) + start_addr;

		memcpy(&buf[0], pwIntTemp, size);
		fwif_color_ChangeUINT32Endian((int*)&buf[0], size/sizeof(int), 1);

		*num_type = VIP_UINT32;

		break;

	case TOOLACCESS_VIP_DeMura_TBL_Mode:
		if(size > sizeof(char))
			return -ID_SIZE_ERR;
		*buf = fwif_color_get_DeMura_TBL_Mode();
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VIP_DeMura_TBL:
		if(rtice_SetGet_DeMura(&rtice_param, num_type, buf, system_setting_info, 0) < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_DeMura_INI:
		if(size > sizeof(char))
			return -ID_SIZE_ERR;
		*buf = 1;
		*num_type = VIP_UINT8;
		break;

	case TOOLACCESS_VIP_DeMura_Adaptive:
		if(rtice_SetGet_DeMura(&rtice_param, num_type, buf, system_setting_info, 0) < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_Force_vTop:
		if(rtice_SetGet_Froce_vTop_TBL(&rtice_param, num_type, buf, system_setting_info, 0) < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_ICM_RANDOM_TABLE:
		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		*buf = 1;
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VIP_D3DLUT_DMA_DEBUG:
		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		*buf = 1;
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_VIP_D3DLUT_RANDOM_TABLE:
		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		*buf = 1;
		*num_type = VIP_UINT8;
		break;
	case TOOLACCESS_PQ_CHECK_ALL:
	{
		if (size > (sizeof(unsigned int) * (200)))
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(200 * sizeof(unsigned int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		if(rtice_param.mode0 == 0)
			sprintf(item_name, "ALL");
		else if(rtice_param.mode0 == 1)
			sprintf(item_name, "DI");
		else if(rtice_param.mode0 == 2)
			sprintf(item_name, "MA");

		vpq_PQ_check_timming(item_name, pwUINT32Temp);

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(unsigned int), 1);
		memcpy(buf, pwUINT32Temp, size);

		*num_type = VIP_UINT32;

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;
	}
	case TOOLACCESS_Set_VIP_Disable_PQ:
		if (size > (sizeof(char)))
			return -ID_SIZE_ERR;

		*buf = 0;
		*num_type = VIP_UINT8;

	break;

	case TOOLACCESS_TV002_STYLE:
		if (size > (sizeof(char)))
			return -ID_SIZE_ERR;
#ifndef CONFIG_CUSTOMER_TV010
		*buf = Scaler_access_Project_TV002_Style(0, 0);
#else
*buf = 0;
#endif
		*num_type = VIP_UINT8;

	break;

	case TOOLACCESS_BP_CTRL_TV002:
	case TOOLACCESS_BP_CTRL_TBL_TV002:
	case TOOLACCESS_BP_STATUS_TV002:
	case TOOLACCESS_BP_TBL_TV002:
		ret = rtice_SetGet_BP_TV002(rtice_param, num_type, buf, system_setting_info, 0);
		if ( ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_DBC_STATUS_TV002:
	case TOOLACCESS_DBC_CTRL_TV002:
	case TOOLACCESS_DBC_SW_REG_TV002:
		ret = rtice_SetGet_DBC_TV002(rtice_param, num_type, buf, system_setting_info, 0);
		if ( ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_Zero_D_DBC_STATUS_TV002:
	case TOOLACCESS_Zero_D_DBC_CTRL_TV002:
	case TOOLACCESS_Zero_D_DBC_LUT_TV002:
		ret = rtice_SetGet_Zero_D_DBC_TV002(rtice_param, num_type, buf, system_setting_info, 0);
		if ( ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_ID_Detect_result_TV002:
	{
		extern unsigned char ID_TV002_Flag[ID_ITEM_TV002_MAX];
		if (size > (sizeof(char)*ID_ITEM_TV002_MAX))
			return -ID_SIZE_ERR;

		memcpy(buf, ID_TV002_Flag, size);

		*num_type = VIP_UINT8;

		break;
	}
	case TOOLACCESS_ID_Detect_Apply_flag_TV002:
	{
		extern unsigned char ID_TV002_Apply;
		extern unsigned char ID_TV002_Detect;
		if (size > (sizeof(char)*2))
			return -ID_SIZE_ERR;

		*buf = ID_TV002_Detect;
		*(buf+1) = ID_TV002_Apply;

		*num_type = VIP_UINT8;

		break;
	}
	case TOOLACCESS_Function_Extend3_DEBUG_LOG_ENABLE:
		if (size > sizeof(gVipDebugLogCtrls) / sizeof(unsigned char))
			return -ID_SIZE_ERR;
		memcpy(buf,gVipDebugLogCtrls, size);
		*(num_type) = VIP_UINT8;
	break;
	case TOOLACCESS_Function_Extend3_DEBUG_LOG_PRINT_DELAY:
	{
		int i = 0;
		if(size != 0 && size != 4)
			return -ID_SIZE_ERR;
		for(i = 0;i < 4 && i < size;i++)
			buf[i] = (gDebugPrintDelayTime >> ((3 - i) * 8)) & 0xff;
		*(num_type) = VIP_UINT32;
	}
	break;

	case TOOLACCESS_RADCR_TBL:
	{
		extern short RADCR_Enable;
		extern RADCR_TBL_ST RADCR_table;
		extern RADCR_STATUS_ST RADCR_status;
		RADCR_RTICE_ST *pRADCR_rtice;
		if (start_addr*sizeof(short)+size > sizeof(*pRADCR_rtice))
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			pRADCR_rtice = vmalloc(sizeof(*pRADCR_rtice));
			if (pRADCR_rtice != NULL) {
				short *p = (short *)pRADCR_rtice;
				pRADCR_rtice->table = RADCR_table;
				pRADCR_rtice->table.RADCR_Reserved10 = RADCR_Enable;
				pRADCR_rtice->status = RADCR_status;
				memcpy(buf, &p[start_addr], size);
				fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
				vfree(pRADCR_rtice);
			}
		}

		*num_type = VIP_INT16;
		break;
	}

	default:
		return -ID_SIZE_ERR;
		break;
	}
	return size;
#else
	return 0;
#endif
}
extern unsigned int D_3D_DRV_LUT[VIP_D_3D_LUT_DRV_TBL_SIZE];
extern unsigned int LUT_3D[LUT3D_TBL_Num][LUT3D_TBL_ITEM];

int rtice_set_vip_table(ap_data_param rtice_param, unsigned char *buf)
{
#ifndef BUILD_QUICK_SHOW
	int ret;
	/*int i = 0, idx;*/
	unsigned int *pwUINT32Temp = NULL;
	unsigned short *pwUINT16Temp = NULL;
	short *pwINT16Temp = NULL;
	unsigned int pwCopyTemp=0, i;
	RPC_DCC_Advance_control_table *Advance_control_table_by_source = NULL;
	/*int *pwINT32Temp = NULL;*/
	/*RPC_DCC_Advance_control_table *Advance_control_table_by_source = NULL;*/
	/*unsigned int pwCopyTemp = 0;*/
	/*int dhue = 0;*/
	/*int dsatbysat[SATSEGMAX];*/
	/*int ditnbyitn[ITNSEGMAX];*/
	/*int h_sta,  h_end,  s_sta,  s_end,  i_sta,  i_end;*/
	/*unsigned int *pWriteBuf = NULL;*/
	_clues *smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	_RPC_clues *RPC_smartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);
	unsigned char bTemp = 0;
	/*unsigned char *buf2_tmp = NULL;*/
	int *pwIntTemp = NULL;
	/*unsigned int *buf_tmp = NULL;*/
	/*unsigned char *buf_tmp8 = NULL;*/
	char *pCharTemp = NULL;
	unsigned int size, start_addr, end_addr,temp_size;
	_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *RPC_system_setting_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	unsigned char HY,VY,HC,VC;

	VIP_ST2094_CTRL *ST2094_CTRL = NULL;
	SLR_VIP_TABLE *vipTable_ShareMem = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	VIP_DCC_AdaptCtrl_Level_Table *DCC_AdaptCtrl_Level_Table = NULL;

	static PQ_HDR_3dLUT_16_PARAM hdr_3DLUT_args;
	hdr_3DLUT_args.c3dLUT_16_enable = 1;

	if (gVip_Table == NULL || vipTable_ShareMem == NULL ||
		system_setting_info == NULL || RPC_system_setting_info == NULL ||
		smartPic_clue == NULL || RPC_smartPic_clue == NULL)
		return -ID_SIZE_ERR;

	stopUpdateFrequencyOffset = 180;

	/*unsigned short UINT16Temp = 0;*/

	/*=== marked by Elsie ===*/
	/*SLR_VIP_TABLE_CUSTOM *g_Share_Memory_VIP_TABLE_Custom_Struct = NULL;*/
	/*g_Share_Memory_VIP_TABLE_Custom_Struct = (SLR_VIP_TABLE_CUSTOM*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_CUSTOM_STRUCT);*/

	/* only for tv002*/
	/*SLR_VIP_TABLE_CUSTOM_TV002 *shareMem_VIP_Customer_TV002 = (SLR_VIP_TABLE_CUSTOM_TV002*)g_Share_Memory_VIP_TABLE_Custom_Struct;*/
	/*VIP_ADV_API_TBL ADV_API_TBL_tmp;*/

	size = rtice_param.size;
	start_addr = rtice_param.saddr;
	end_addr = rtice_param.eaddr;

	switch (rtice_param.id) {
	/***************************Enginner Menu Start ********************************************/
	case TOOLACCESS_Contrast:   	/*  0*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetContrast(buf[0]);
		break;

	case TOOLACCESS_Brightness:   	/*  1*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetBrightness(buf[0]);
		break;

	case TOOLACCESS_Saturation:   	/*  2*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetSaturation(buf[0]);
		break;

	case TOOLACCESS_Hue:   			/*  3*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetHue(buf[0]);
		break;

	case TOOLACCESS_Local_Contrast:   			/*  3*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetLocalContrastTable(buf[0]);
		break;

	case TOOLACCESS_Dynamic_Contrast:
		if (size > sizeof(unsigned char)*2)
			return -ID_SIZE_ERR;

		Scaler_SetDCC_Table(buf[0]);
		Scaler_SetDCC_Mode(buf[1]); /* set dcc mode*/

		break;

	case TOOLACCESS_VD_Contrast:   	/*  4*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		break;

	case TOOLACCESS_VD_Brightness:	/*  5*/

		break;
	case TOOLACCESS_VD_Saturation:  /*  6*/

		break;
	case TOOLACCESS_VD_Hue:   		/*  7*/

		break;
	case TOOLACCESS_Gamma:   		/*  8*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetGamma(buf[0]);
		break;

	case TOOLACCESS_DCTI:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetDCti(buf[0]);
		break;

	case TOOLACCESS_DLTI:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetDLti(buf[0]);
		break;

	case TOOLACCESS_MADI_HMC:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetMADI_HMC(buf[0]);
		break;

	case TOOLACCESS_MADI_HME:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetMADI_HME(buf[0]);
		break;

	case TOOLACCESS_MADI_PAN:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetMADI_PAN(buf[0]);
		break;

	case TOOLACCESS_DI_MA_Adjust_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_color_Stop_VGIP_isr(1);
		Scaler_SetMADI(buf[0]);
		drvif_color_Stop_VGIP_isr(0);
		break;

	case TOOLACCESS_DI_DiSmd:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetDiSmd(buf[0]);
		break;

	case TOOLACCESS_TNRXC_Ctrl:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetTNRXC_Ctrl(buf[0]);
		break;

	case TOOLACCESS_TNRXC_MK2:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetTNRXC_MK2(buf[0]);
		break;
	case TOOLACCESS_D_UVDelay_en:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetColor_UV_Delay_Enable(buf[0]);
		break;
	case TOOLACCESS_D_UVDelay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetColor_UV_Delay(buf[0]);
		break;
	case TOOLACCESS_ScaleUPH:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleUPH(buf[0]);
		break;

	case TOOLACCESS_ScaleUPH_12tap:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleUPH_12tap(buf[0]);
		break;

	case TOOLACCESS_ScaleUPV_8tap:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleUPV_8tap(buf[0]);
		break;

	case TOOLACCESS_ScaleUPV:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleUPV(buf[0]);
		break;

	case TOOLACCESS_ScaleUPDir:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleUPDir(buf[0]);
		break;

	case TOOLACCESS_ScaleUPDir_Weighting:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleUPDir_Weighting(buf[0]);
		break;

	case TOOLACCESS_ScaleDOWN_H_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleDOWNHTable(buf[0]);
		break;

	case TOOLACCESS_ScaleDOWN_V_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleDOWNVTable(buf[0]);
		break;
	case TOOLACCESS_ScaleDOWN_444To422:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_SetScaleDOWN444To422(buf[0]);
		break;

	case TOOLACCESS_Pattern_Off:
		Scaler_SetFacPatternColor(0);
		break;

	case TOOLACCESS_Pattern_Red:
		Scaler_SetFacPatternColor(1);
		break;

	case TOOLACCESS_Pattern_Green:
		Scaler_SetFacPatternColor(2);
		break;

	case TOOLACCESS_Pattern_Blue:
		Scaler_SetFacPatternColor(3);
		break;

	case TOOLACCESS_Pattern_White:
		Scaler_SetFacPatternColor(4);
		break;

	case TOOLACCESS_Pattern_Black:
		Scaler_SetFacPatternColor(5);
		break;

	case TOOLACCESS_Pattern_Color_Bar:
		Scaler_SetFacPatternColor(6);
		break;

	case TOOLACCESS_Pattern_Gray_Bar:
		Scaler_SetFacPatternColor(7);
		break;

	case TOOLACCESS_Magic_Off:
		/*SetMagicPicture(MAGIC_PICTURE_OFF);*/
		break;

	case TOOLACCESS_SetMagic_Still_DEMO:
		/*SetMagicPicture(MAGIC_PICTURE_STILLDEMO);*/
		break;

	case TOOLACCESS_SetMagic_Still_DEMO_Inverse:
		/*SetMagicPicture(MAGIC_PICTURE_STILLDEMO_INVERSE);*/
		break;

	case TOOLACCESS_SetMagic_Dynamic_DEMO:
		/*SetMagicPicture(MAGIC_PICTURE_DYNAMICDEMO);*/
		break;
	case TOOLACCESS_SetMagic_Move:
		/*SetMagicPicture(MAGIC_PICTURE_MOVE);*/
		break;

	case TOOLACCESS_SetMagic_Move_Inverse:
		/* SetMagicPicture(MAGIC_PICTURE_MOVE_INVERSE);*/
		break;

	case TOOLACCESS_SetMagic_Zoom:
		/*SetMagicPicture(MAGIC_PICTURE_ZOOM);*/
		break;

	case TOOLACCESS_SetMagic_Optimize:
		/*SetMagicPicture(MAGIC_PICTURE_OPTIMIZE);*/
		break;

	case TOOLACCESS_SetMagic_Enhance:
		/*SetMagicPicture(MAGIC_PICTURE_ENHANCE);*/
		break;

	case TOOLACCESS_VD_SetYC_Separation:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_YcSeparation_Factory_Mode(buf[0]);
		break;
	case TOOLACCESS_VD_Set3D_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_Set3DTableIndex(buf[0]);
		break;

	case TOOLACCESS_VD_Set2D_Chroma_BW_Low:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_2D_Chroma_Bw_lo_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetSetDCTI_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_Set_DCTI_Setting_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_Set2D_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vpq_set_2d_table_index_Factory(buf[0]);
		break;

	case TOOLACCESS_VD_SetNarrow_BPF_Y:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_2D_Narrow_BPF_Sel_Y_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetWide_BPF_Y:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_2D_Wide_BPF_Sel_Y_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetWide_BPF_C:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_2D_Wide_BPF_Sel_C_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetNarrow_BPF_C:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_2D_Narrow_BPF_Sel_C_Factory_Mode(buf[0]);
		break;

	/*
	case TOOLACCESS_VD_SetVflag_Remg_Thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_H2V_Vflag_Remg_thr_Factory_Mode(buf[0]);
		break;
	*/

	case TOOLACCESS_VD_SetVflag_Mag_Thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_H2V_Vflag_Mag_thr_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetY_Noise_Thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_2D_Narrow_BPF_Sel_C_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetNoise_Max_Hdy:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_H2V_Vflag_Max_HDY_Factory(buf[0]);
		break;

	case TOOLACCESS_VD_SetNoise_Y_Add_DC:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_H2V_Vflag_Noise_YADDC_Factory(buf[0]);
		break;

	case TOOLACCESS_VD_SetBlend_Ratio:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_H2V_Vflag_Blend_Ratio_Factory(buf[0]);
		break;

	case TOOLACCESS_VD_SetTDma_Enable:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_factory_TestDMA_Enable(buf[0]);
		break;

	case TOOLACCESS_VD_SetTDma_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_factory_TestDMA_mode_select(buf[0]);
		break;

	case TOOLACCESS_VD_SetDeb_Print:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_Debug_Print_Set(buf[0]);
		break;

	case TOOLACCESS_VD_Set625_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_625_mode_setting_Factory_mode(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetMV_Check:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_MV_check_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetClamp_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_ClampMode_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetRc_Rate_Func:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_clamp_rc_rate_enable_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetRc_Rate_Value:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_clamp_rc_rate_value_Factory_Mode_Set(buf[0]);
		break;

	case TOOLACCESS_VD_SetV_Clamp:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Vsync_Clamp_mode_Factory_Mode(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetVsync_Ctrl:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Vsync_Ctrl_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetClamp_Delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_clamp_delay_Enable_Factory_mode_set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetClamp_Delay_Value:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_clamp_delay_value_Factory_mode_set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetSetH_State_Write:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Hstate_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetHsync_Start:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Hsync_start_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetV_State_Write:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Vstate_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetV_Detect_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_New_Vdetect_setting_Factory_mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetC_State_W:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cstate_Cactory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetAuto_BP:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_Auto_Burst_position_enable_Factory_Mode_Set();
		break;

	case TOOLACCESS_VD_SetCkill_Mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_user_ckill_mode_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetCkill_Value:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_Chroam_Level_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetSet_Dgain:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_Dgain_Write_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetFix_Dgain:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_Fix_Dgain_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetSet_Cgain:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_Cagc_Write_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_SetFix_Cgain:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_Fix_Cagc_Factory_Mode(buf[0]);
		break;

	case TOOLACCESS_VD_Set27M_LPF:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_27M_Lpf_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_Set27M_Table:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_27M_table_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_Set108M_LPF:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_108M_Lpf_Factory_Mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetSecam_F_sel:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_secamf_sel_Factory_mode_set(buf[0]);*/
		break;

	case TOOLACCESS_VD_SetSecam2:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_secam2_Factory_mode_set(buf[0]);*/
		break;

	case TOOLACCESS_VD_Set443358pk_sel:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_secam2_Factory_mode_set(buf[0]);*/

		break;

	case TOOLACCESS_VD_Set443thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_set_Sep_443_thl(buf[0]);
		break;

	case TOOLACCESS_VD_Set358thr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_set_Sep_358_thl(buf[0]);
		break;

	case TOOLACCESS_VD_SetSoft_Reset:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_SoftReset();
		break;

	case TOOLACCESS_VD_PQ_SetBypassPQ:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_PQ_Bypass_Factory_mode_set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_Set1D3D_Pos:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_1D3Dpos_Factory_mode_set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetContrast:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_SetContrast(buf[0]);
		break;

	case TOOLACCESS_VD_PQ_SetBrightness:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_SetBrightness(buf[0]);
		break;

	case TOOLACCESS_VD_PQ_SetSaturation:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vdc_SetSaturation(buf[0]);
		break;

	case TOOLACCESS_VD_PQ_SetYC_Delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_YcDelay(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetY1delay_En:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Y1Delay_Enable_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetCb1delay_En:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cb1Delay_Enable_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetCr1delay_En:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cr1Delay_Enable_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetY1delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Y1Delay_Value_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetCb1delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cb1Delay_Value_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetCr1delay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_Cr1Delay_Value_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetKill_Ydelay_En:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_ckill_YDelay_Enable_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetKill_Ydelay:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_ckill_YDelay_Value_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetSCART_Con:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SCART_contrast_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetSCART_Bri:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SCART_brightness_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetSCART_Sat_cr:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SCART_saturationCr_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_SetSCART_Sat_cb:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*drvif_module_vdc_SCART_saturationCb_Factory_mode_Set(buf[0]);*/
		break;

	case TOOLACCESS_VD_PQ_Style:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_module_vpq_set_ProjectId(buf[0]);
		drvif_module_vpq_SetYcSep(VDC_YCSEPARATE_DEFAULT);
		break;

	case TOOLACCESS_DCC_PQ_Level:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (Scaler_set_DCC_AdapCtrl_DCC_Level(buf[0]) == FALSE)
			return -WRITE_ERROR;

		break;
	case TOOLACCESS_DCC_PQ_Cubic_index:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (Scaler_set_DCC_AdapCtrl_Cubic_index(buf[0]) == FALSE)
			return -WRITE_ERROR;

		break;
	case TOOLACCESS_DCC_PQ_Cubic_low:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (Scaler_set_DCC_AdapCtrl_Cubic_low(buf[0]) == FALSE)
			return -WRITE_ERROR;

		break;
	case TOOLACCESS_DCC_PQ_Cubic_high:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (Scaler_set_DCC_AdapCtrl_Cubic_high(buf[0]) == FALSE)
			return -WRITE_ERROR;

		break;

	case TOOLACCESS_xvYcc_mode:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
#if 0	/*#ifdef ENABLE_xvYcc	*/ /*=== marked by Elsie ===*/
		if (Scaler_SetxvYcc_Mode((SLR_xvYcc_MODE)buf[0]) == FALSE)
			return -WRITE_ERROR;
#endif
		break;
	case TOOLACCESS_ICM_7axis_adjust:
		if (size > sizeof(unsigned char)*7)
			return -ID_SIZE_ERR;
#if 0	/*=== marked by Elsie ===*/
		buf_tmp8 = (unsigned char *)vip_malloc(7 * sizeof(unsigned char));
		if (buf_tmp8 == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned char *)buf_tmp8, buf, size);

		Scaler_set_ICM_7Axis_Adjust(buf_tmp8[0], 0);
		Scaler_set_ICM_7Axis_Adjust(buf_tmp8[1], 1);
		Scaler_set_ICM_7Axis_Adjust(buf_tmp8[2], 2);
		Scaler_set_ICM_7Axis_Adjust(buf_tmp8[3], 3);
		Scaler_set_ICM_7Axis_Adjust(buf_tmp8[4], 4);
		Scaler_set_ICM_7Axis_Adjust(buf_tmp8[5], 5);
		Scaler_set_ICM_7Axis_Adjust(buf_tmp8[6], 6);

		vip_free(buf_tmp8);
		buf_tmp8 = NULL;
#endif
		break;

#if 1/*defined(PQ_DUMPDIDATA)*/
	case TOOLACCESS_DumpDi:		/*	0*/
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		/*Scaler_calDumpDI_datasize(buf[0]);	*/ /*=== marked by Elsie ===*/
		break;
#endif

	case TOOLACCESS_switch_DVIandHDMI:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
#if 0	/*=== marked by Elsie ===*/
		if (Scaler_Setswitch_DVIandHDMI((SLR_switch_DVIandHDMI_MODE)buf[0]) == FALSE)
			return -WRITE_ERROR;
#endif
		break;

/***************************Enginner Menu End ********************************************/
	case TOOLACCESS_DisplayInformation_table:
		if (size > sizeof(unsigned int)*DisplayInfo_Coef_MAX)
			return -ID_SIZE_ERR;
		break;

	case TOOLACCESS_VIP_QUALITY_Coef:       /*  20*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_toio(vipTable_ShareMem->VIP_QUALITY_Coef, buf, size);
#else
		memcpy(vipTable_ShareMem->VIP_QUALITY_Coef, buf, size);
#endif
		break;
	case TOOLACCESS_VIP_QUALITY_Extend_Coef:       /*  21*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_toio(vipTable_ShareMem->VIP_QUALITY_Extend_Coef, buf, size);
#else
		memcpy(vipTable_ShareMem->VIP_QUALITY_Extend_Coef, buf, size);
#endif
		break;
	case TOOLACCESS_VIP_QUALITY_Extend2_Coef:       /*  22*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_toio(vipTable_ShareMem->VIP_QUALITY_Extend2_Coef, buf, size);
#else
		memcpy(vipTable_ShareMem->VIP_QUALITY_Extend2_Coef, buf, size);
#endif
		break;
	case TOOLACCESS_VIP_QUALITY_Extend3_Coef:       /*  23*/
		if (size > sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_toio(vipTable_ShareMem->VIP_QUALITY_Extend3_Coef, buf, size);
#else
		memcpy(vipTable_ShareMem->VIP_QUALITY_Extend3_Coef, buf, size);
#endif
		break;
	case TOOLACCESS_tICM_ini:       /*  24*/
		if (size > sizeof(unsigned short)*VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *)&(vipTable_ShareMem->tICM_ini[0][0][0]), buf, size);
#else
		memcpy((unsigned char *)&(vipTable_ShareMem->tICM_ini[0][0][0]), buf, size);
#endif
		break;
	case TOOLACCESS_SHPTable:       /*  25*/
		if (size > sizeof(VIP_Sharpness_Table))
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->Ddomain_SHPTable[0]), buf, size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->Ddomain_SHPTable[0]), buf, size);
#endif
		break;
	case TOOLACCESS_Hist_Y_Mean_Value:/* 26*/
		if (size > sizeof(unsigned int)*1)
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_TABLE_NTSC_3D:  /*  27*/
		if (size > sizeof(unsigned char)*VD_3D_TABLE_SIZE*VD_MOTION_STEP*2)
			return -ID_SIZE_ERR;
		break;
	case TOOLACCESS_MANUAL_NR:/* 29*/
		if (size > sizeof(DRV_NR_Item)*PQA_TABLE_MAX*DRV_NR_Level_MAX)
			return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *)&(vipTable_ShareMem->Manual_NR_Table[0][0]), buf, size);
#else
		memcpy((unsigned char *)&(vipTable_ShareMem->Manual_NR_Table[0][0]), buf, size);
#endif
		break;

	#if 0
	case TOOLACCESS_PQA_Input_Table:/* 30*/
		if (size > sizeof(int)*PQA_I_TABLE_MAX*PQA_I_ITEM_MAX*PQA_I_LEVEL_MAX)
			return -ID_SIZE_ERR;
		memcpy((unsigned char *) &(vipTable_ShareMem->PQA_Input_Table[0][0][0]), buf, size);
		break;
	case TOOLACCESS_PQA_Table:/* 31*/
		if (size > sizeof(int)*PQA_TABLE_MAX*PQA_MODE_MAX*PQA_ITEM_MAX*PQA_LEVEL_MAX)
			return -ID_SIZE_ERR;
		memcpy((unsigned char *) &(vipTable_ShareMem->PQA_Table[0][0][0][0]), buf, size);
		break;
	#endif

	case TOOLACCESS_ISR_Print_Ctrl:
		if (size > sizeof(VIP_ISR_Printf_Flag))
			return -ID_SIZE_ERR;
		fwif_color_ChangeUINT32Endian((unsigned int *)buf, ISR_PRINT_CTRL_ITEM_NUMBER, 1);
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(system_setting_info->ISR_Printf_Flag), buf, size);
#else
		memcpy((unsigned char *) &(system_setting_info->ISR_Printf_Flag), buf, size);
#endif
		break;

		case TOOLACCESS_Debug_Buff_8:
			if(size > Debug_Buff_Num*sizeof(char))
				return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
			memcpy_toio((unsigned char*)&(system_setting_info->Debug_Buff_8[0]),buf, size);
#else
			memcpy((unsigned char*)&(system_setting_info->Debug_Buff_8[0]),buf, size);
#endif
			break;

		case TOOLACCESS_Debug_Buff_16:
			if(size > Debug_Buff_Num*sizeof(short))
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian((unsigned short*)buf, size/sizeof(short), 1);
#ifdef CONFIG_ARM64
			memcpy_toio((unsigned char*)&(system_setting_info->Debug_Buff_16[0]),buf, size);
#else
			memcpy((unsigned char*)&(system_setting_info->Debug_Buff_16[0]),buf, size);
#endif
			break;

		case TOOLACCESS_Debug_Buff_32:
			if(size > Debug_Buff_Num*sizeof(int))
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT32Endian((unsigned int*)buf, size/sizeof(int), 1);
#ifdef CONFIG_ARM64
			memcpy_toio((unsigned char*)&(system_setting_info->Debug_Buff_32[0]),buf, size);
#else
			memcpy((unsigned char*)&(system_setting_info->Debug_Buff_32[0]),buf, size);
#endif
			break;

	case TOOLACCESS_BP_Function_CTRL:
		break;
	case TOOLACCESS_BP_Function_TBL:
		break;
	case TOOLACCESS_ADV_API_CTRL:
		break;
	case TOOLACCESS_ADV_API_TBL:
		break;
	case TOOLACCESS_DBC_STATUS:
		break;
	case TOOLACCESS_DBC_SW_REG:
		break;

	/***************************for PQA Table access*****************************************/ /**/
	case TOOLACCESS_PQA_Table_Size:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Table_Size, 1, size, NULL, buf) < 0)
			return -ID_SIZE_ERR;
	break;

	case TOOLACCESS_PQA_Table:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Table, 1, size, NULL, buf) < 0)
			return -ID_SIZE_ERR;
	break;

	case TOOLACCESS_PQA_Level_Table:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Level_Table, 1, size, NULL, buf) < 0)
			return -ID_SIZE_ERR;
	break;

	case TOOLACCESS_PQA_Level_Index_Table:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Level_Index_Table, 1, size, NULL, buf) < 0)
			return -ID_SIZE_ERR;
	break;

	case TOOLACCESS_PQA_Input_Table:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_Input_Table, 1, size, NULL, buf) < 0)
			return -ID_SIZE_ERR;
	break;
	case TOOLACCESS_PQA_FlowCtrl_Input_Type:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_FlowCtrl_Input_Type, 1, size, NULL, buf) < 0)
			return -ID_SIZE_ERR;
	break;

	case TOOLACCESS_PQA_FlowCtrl_Input_Item:
		if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_FlowCtrl_Input_Item, 1, size, NULL, buf) < 0)
			return -ID_SIZE_ERR;
	break;

       case TOOLACCESS_PQA_SPM_Info_Get:
               if (rtice_SetGet_PQA_Table_Info(TOOLACCESS_PQA_SPM_Info_Get, 1, size, NULL, buf) < 0)
                       return -ID_SIZE_ERR;
       break;

	/***************************for PQA Table access*****************************************/ /**/

	case TOOLACCESS_DCC_Curve_Control_Coef:				/* 1100*/
		if (size > sizeof(VIP_DCC_Curve_Control_Coef)*DCC_SELECT_MAX+1)
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Table;
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Curve_Control_Coef[bTemp]), buf, size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Curve_Control_Coef[bTemp]), buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Boundary_check_Table:   		/* 1101*/
		if (size > sizeof(VIP_DCC_Boundary_check_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.Boundary_Check_Table;

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Curve_Control_Coef[bTemp]), buf, size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Boundary_check_Table[bTemp]), buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Level_and_Blend_Coef_Table:     /* 1102 , DCC_Misc*/
		if (size > sizeof(VIP_DCC_Level_and_Blend_Coef_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.Level_and_Blend_Coef_Table;

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[bTemp]), buf, size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_Level_and_Blend_Coef_Table[bTemp]), buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Hist_Factor_Table:   			/* 1103*/
		if (size > sizeof(VIP_DCC_Hist_Factor_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.hist_adjust_table;

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCCHist_Factor_Table[bTemp]), buf , size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCCHist_Factor_Table[bTemp]), buf , size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_AdaptCtrl_Level_Table:  		/* 1104*/
		if (size > sizeof(VIP_DCC_AdaptCtrl_Level_Table))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.AdaptCtrl_Level_Table;

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[bTemp]), buf , size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.DCC_AdaptCtrl_Level_Table[bTemp]), buf , size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_AdaptCtrl_Param:  /*1105  KKKKKKK*/
		if (size > 8 * sizeof(unsigned char))
			return -ID_SIZE_ERR;

		DCC_AdaptCtrl_Level_Table = Scaler_get_DCC_AdapCtrl_Table();
		if (DCC_AdaptCtrl_Level_Table == NULL) {
			printf("~Scaler_get_DCC_AdapCtrl_DCC_Level Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
			return FALSE;
		}
		(DCC_AdaptCtrl_Level_Table->DCC_Level_table)[buf[6]][buf[7]] = buf[0];
		(DCC_AdaptCtrl_Level_Table->DCC_Cublc_Index_table)[buf[6]][buf[7]] = buf[1];
		(DCC_AdaptCtrl_Level_Table->DCC_Cublc_High_table)[buf[6]][buf[7]] = buf[2];
		(DCC_AdaptCtrl_Level_Table->DCC_Cublc_Low_table)[buf[6]][buf[7]] = buf[3];
		(DCC_AdaptCtrl_Level_Table->DCC_W_Ex_table)[buf[6]][buf[7]] = buf[4];
		(DCC_AdaptCtrl_Level_Table->DCC_B_Ex_table)[buf[6]][buf[7]] = buf[5];

		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_UserDef_Ctrl_TABLE: /*1106*/

		if (size > (sizeof(USER_CURVE_DCC_HisMean_th_ITEM)+sizeof(USER_CURVE_DCC_HisSeg_Weighting_ITEM)+3))
			return -ID_SIZE_ERR;

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.User_Curve_Table;
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisMean_th), buf, 10);
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisSeg_Weighting), buf+sizeof(USER_CURVE_DCC_HisMean_th_ITEM) , 11);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisMean_th), buf, 10);
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].USER_CURVE_Weight_Mapping.USER_CURVE_DCC_HisSeg_Weighting), buf+sizeof(USER_CURVE_DCC_HisMean_th_ITEM) , 11);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_UserDef_Curve_All_TABLE: /*1107*/
		if (size > (sizeof(unsigned int)*(USER_DEFINE_CURVE_DCC_CURVE_MAX*USER_DEFINE_CURVE_DCC_SEGMENT_NUM)))
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.User_Curve_Table;
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table) , (unsigned char *)buf, size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table) , (unsigned char *)buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_UserDef_Curve_Current_TABLE: /*1108*/
		if (size > (sizeof(unsigned int)*USER_DEFINE_CURVE_DCC_SEGMENT_NUM))
			return -ID_SIZE_ERR;
		if (rtice_param.mode0 >= USER_DEFINE_CURVE_DCC_CURVE_MAX)
			rtice_param.mode0 = USER_DEFINE_CURVE_DCC_CURVE_MAX-1;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.User_Curve_Table;
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table[rtice_param.mode0][0]), (unsigned char*)buf, size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.USER_DEFINE_CURVE_DCC_CRTL_Table[bTemp].user_define_curve_dcc_table[rtice_param.mode0][0]), (unsigned char*)buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Database_Curve_CRTL_All_Table: /*1109*/
		if (size > ((Histogram_adjust_bin_num+DCC_CURVE_Segment_NUM+DCC_CURVE_User_Gain_Parameter)*Database_DCC_Curve_Case_Item_MAX )*sizeof(unsigned int))
			return -ID_SIZE_ERR;

		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.database_DCC_Table;

		if(bTemp >= Database_DCC_Curve_TABLE_MAX)
			bTemp = Database_DCC_Curve_TABLE_MAX-1;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char*)&(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][0]), (unsigned char*)buf, size);
#else
		memcpy((unsigned char*)&(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][0]), (unsigned char*)buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Database_Curve_CRTL_Curve_Current_Table: /*1110*/
		if (size > ((DCC_CURVE_Segment_NUM)*sizeof(unsigned int)))/*mode1:histogram_index*/
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.database_DCC_Table;

		if(bTemp >= Database_DCC_Curve_TABLE_MAX)
			bTemp = Database_DCC_Curve_TABLE_MAX-1;

		if (rtice_param.mode0 >= Database_DCC_Curve_Case_Item_MAX)
			rtice_param.mode0 = Database_DCC_Curve_Case_Item_MAX-1;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_curve), (unsigned char*)buf, size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_curve), (unsigned char*)buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Database_Curve_CRTL_Histogram_Current_Table: /*id 1111*/
		if (size > (sizeof(unsigned int)*Histogram_adjust_bin_num))
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.database_DCC_Table;

		if(bTemp >= Database_DCC_Curve_TABLE_MAX)
			bTemp = Database_DCC_Curve_TABLE_MAX-1;

		if (rtice_param.mode0 >= Database_DCC_Curve_Case_Item_MAX)
			rtice_param.mode0 = Database_DCC_Curve_Case_Item_MAX-1;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_histogram), (unsigned char *)buf, size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_histogram), (unsigned char *)buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Histogram_Curve:  /*id:1112*/
		if (size > (sizeof(unsigned int)*DCC_Curve_Node_MAX))
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Histogram_Curve), (unsigned char *)buf, size);
#else
		memcpy((unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Histogram_Curve), (unsigned char *)buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;


	case TOOLACCESS_DCC_Histogram_Adjust:  /*id:1114*/
		if (size > sizeof(unsigned int)*COLOR_HISTOGRAM_LEVEL)
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.HistCnt_Of_Adj), (unsigned char *)buf, size);
#else
		memcpy((unsigned char *) &(smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.HistCnt_Of_Adj), (unsigned char *)buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;


	case TOOLACCESS_DCC_Apply_Curve: /*1116*/
		if (size > (sizeof(unsigned int)*DCC_Curve_Node_MAX))
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(RPC_smartPic_clue->RPC_Adapt_DCC_Info.Apply_Curve), (unsigned char *)buf, size);
#else
		memcpy((unsigned char *) &(RPC_smartPic_clue->RPC_Adapt_DCC_Info.Apply_Curve), (unsigned char *)buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_DCC_Curve_Boundary_Type:			/* 1117*/
	{
		VIP_DCC_Control_Structure *DCC_Control_Structure = &(vipTable_ShareMem->DCC_Control_Structure);
		unsigned char DCC_which_table = system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Table;
		unsigned char DCC_which_Mode = system_setting_info->OSD_Info.OSD_DCC_Mode;
		VIP_DCC_Curve_Control_Coef *DCC_Curve_Control_Coef = &(DCC_Control_Structure->DCC_Curve_Control_Coef[DCC_which_table][DCC_which_Mode]);

		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		//system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[smartPic_clue->Remmping_Src_Timing_index].DCC_init_boundary_Type = buf[0];
		if (size > 0) {
			DCC_Curve_Control_Coef->boundary_type = buf[0];
			fwif_color_set_DCC_SC_for_Coef_Change(1,1);
		}

		break;
	}
	case TOOLACCESS_DCC_Curve_Boundary:				/* 1118*/
	{
		VIP_DCC_Control_Structure *DCC_Control_Structure = &(vipTable_ShareMem->DCC_Control_Structure);
		unsigned char DCC_which_table = system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Table;
		unsigned char DCC_which_Mode = system_setting_info->OSD_Info.OSD_DCC_Mode;
		VIP_DCC_Curve_Control_Coef *DCC_Curve_Control_Coef = &(DCC_Control_Structure->DCC_Curve_Control_Coef[DCC_which_table][DCC_which_Mode]);

			if(size > sizeof( RPC_DCC_Curve_boundary_table))
				return -ID_SIZE_ERR;
			pwCopyTemp = smartPic_clue->Remmping_Src_Timing_index;
			Advance_control_table_by_source = &(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp]);
			//bTemp = Advance_control_table_by_source->DCC_init_boundary_Type;
			bTemp = DCC_Curve_Control_Coef->boundary_type;

			if(bTemp < Curve_boundary_table_MAX)
#ifdef CONFIG_ARM64
				memcpy_toio((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Curve_boundary_table[bTemp]),buf,size);
#else
				memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Curve_boundary_table[bTemp]),buf,size);
#endif
			fwif_color_set_DCC_SC_for_Coef_Change(1,1);

			break;
	}
	case TOOLACCESS_DCC_panel_compensation_Curve: /*1119*/
		if(size > sizeof(unsigned int)*Curve_num_Max)
			return -ID_SIZE_ERR;

		temp_size = size>>2;// because sizeof(unsigned int)
		fwif_color_ChangeUINT32Endian((unsigned int*)buf,temp_size, 1);

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char*)&(vipTable_ShareMem->panel_compensation_curve),buf,size);
#else
		memcpy((unsigned char*)&(vipTable_ShareMem->panel_compensation_curve),buf,size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1,1);

		break;

	case TOOLACCESS_DCC_DataBaseHistogrm : /*1120*/
		//This item only  get !!
		break;

	case TOOLACCESS_DCC_Database_Curve_CRTL_User_Gain_Parameter_Current_Table: /*1121*/
		if (size > ((DCC_CURVE_User_Gain_Parameter)*sizeof(unsigned int)))/*mode1:histogram_index*/
			return -ID_SIZE_ERR;
		bTemp = system_setting_info->PQ_Setting_Info.DCC_info_coef.database_DCC_Table;

		if(bTemp >= Database_DCC_Curve_TABLE_MAX)
			bTemp = Database_DCC_Curve_TABLE_MAX-1;

		if (rtice_param.mode0 >= Database_DCC_Curve_Case_Item_MAX)
			rtice_param.mode0 = Database_DCC_Curve_Case_Item_MAX-1;

		fwif_color_ChangeUINT32Endian((unsigned int*)buf,(size>>2), 1);

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_adjust), (unsigned char*)buf, size);
#else
		memcpy((unsigned char *) &(vipTable_ShareMem->DCC_Control_Structure.Database_Curve_CRTL_Table[bTemp][rtice_param.mode0].database_adjust), (unsigned char*)buf, size);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);

		break;

	case TOOLACCESS_VIP_DCC_Advance_Level_Control_Flag: 			/* 1122*/
		if (size > (Advance_Level_Control_Item_MAX+LPF_control_Item_MAX+1))
			return -ID_SIZE_ERR;
		pwCopyTemp = smartPic_clue->Remmping_Src_Timing_index;
		if(pwCopyTemp >= Advance_control_table_MAX)
			pwCopyTemp = Advance_control_table_MAX-1;

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Advance_Level_Control_Flag),(unsigned char*)buf,Advance_Level_Control_Item_MAX);
		memcpy_toio((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_LPF_Control),(unsigned char*)buf+Advance_Level_Control_Item_MAX,LPF_control_Item_MAX);
		memcpy_toio((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_init_boundary_Type),(unsigned char*)buf+Advance_Level_Control_Item_MAX+LPF_control_Item_MAX,1);
#else
		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Advance_Level_Control_Flag),(unsigned char*)buf,Advance_Level_Control_Item_MAX);
		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_LPF_Control),(unsigned char*)buf+Advance_Level_Control_Item_MAX,LPF_control_Item_MAX);
		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_init_boundary_Type),(unsigned char*)buf+Advance_Level_Control_Item_MAX+LPF_control_Item_MAX,1);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);
		break;


	case TOOLACCESS_VIP_DCC_Advance_Level_Control_Step_DT_Table: 			/* 1123*/
		if (size > (mean_value_index_MAX*Variance_index_MAX*2))
			return -ID_SIZE_ERR;
		pwCopyTemp = smartPic_clue->Remmping_Src_Timing_index;
		if(pwCopyTemp >= Advance_control_table_MAX)
			pwCopyTemp = Advance_control_table_MAX-1;

#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_Step_table),(unsigned char*)buf,mean_value_index_MAX*Variance_index_MAX);
		memcpy_toio((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_DT_table),(unsigned char*)buf+mean_value_index_MAX*Variance_index_MAX,mean_value_index_MAX*Variance_index_MAX);
#else
		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_Step_table),(unsigned char*)buf,mean_value_index_MAX*Variance_index_MAX);
		memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[pwCopyTemp].DCC_Curve_Blending_DT_table),(unsigned char*)buf+mean_value_index_MAX*Variance_index_MAX,mean_value_index_MAX*Variance_index_MAX);
#endif
		fwif_color_set_DCC_SC_for_Coef_Change(1, 1);
		break;

	case TOOLACCESS_VIP_DCC_Identity_Condition_Check : /*1124*/
	//This item only  get !!
	break;

	case TOOLACCESS_VIP_DCC_ADV_AP_BLE:
	{
		VIP_DCC_ADV_API *pDCC_ADV_API;
		unsigned char tbl_sel;

		if(size != 4)
			return -ID_SIZE_ERR;

		//if (gVip_Table == NULL) //no need recheck
		//	return -WRITE_ERROR;

		pDCC_ADV_API = &vipTable_ShareMem->DCC_Control_Structure.DCC_ADV_API;

		tbl_sel = pDCC_ADV_API->ADV_API_CTRL.TBL_Select_BLE;
		pDCC_ADV_API->ADV_API_TBL[tbl_sel].ADV_API_BLE_TBL.Enable_Flag = buf[0];
		pDCC_ADV_API->ADV_API_TBL[tbl_sel].ADV_API_BLE_TBL.BLE_Lv = buf[1];
		pDCC_ADV_API->ADV_API_TBL[tbl_sel].ADV_API_BLE_TBL.isBLE_Lv_Reverse = buf[2];
		pDCC_ADV_API->ADV_API_TBL[tbl_sel].ADV_API_BLE_TBL.BLE_Range = buf[3];

		break;
	}

	case TOOLACCESS_ICM_Global_Adjust:					/* 1200*/
		if (size > sizeof(RPC_ICM_Global_Ctrl))
			return -ID_SIZE_ERR;

		if (fwif_color_GetICM_Global_Ctrl_Struct() == NULL)
			return -WRITE_ERROR;

		fwif_color_ChangeINT32Endian((int *)buf, size/4, 1);
#ifdef CONFIG_ARM64
		memcpy_toio((unsigned char *)fwif_color_GetICM_Global_Ctrl_Struct(), buf, size);
#else
		memcpy((unsigned char *)fwif_color_GetICM_Global_Ctrl_Struct(), buf, size);
#endif

		/*set driver*/
		#if 0
		fwif_color_ChangeINT32Endian_Copy(&(fwif_color_GetICM_Global_Ctrl_Struct()->dHue), 1, &dhue, 1);
		fwif_color_ChangeINT32Endian_Copy((fwif_color_GetICM_Global_Ctrl_Struct()->dSatBySat), SATSEGMAX, dsatbysat, 1);
		fwif_color_ChangeINT32Endian_Copy((fwif_color_GetICM_Global_Ctrl_Struct()->dItnByItn), ITNSEGMAX, ditnbyitn, 1);
		fwif_color_set_ICM_Global_Adjust(dhue, dsatbysat, ditnbyitn);
		#endif
		break;
	case TOOLACCESS_ICM_BOX_Adjust: /* 1201*/
		if (size > sizeof(int)*(6))
			return -ID_SIZE_ERR;
		pwIntTemp = (int *)vip_malloc(6 * sizeof(int));

		if (pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian((int *)buf , size/4, 1);
		memcpy((unsigned char *)pwIntTemp, (unsigned char *)buf, size);

		g_h_sta = pwIntTemp[0];
		g_h_end = pwIntTemp[1];
		g_s_sta = pwIntTemp[2];
		g_s_end = pwIntTemp[3];
		g_i_sta = pwIntTemp[4];
		g_i_end = pwIntTemp[5];

		vip_free(pwIntTemp);
		pwIntTemp = NULL;

		break;
	case TOOLACCESS_ICM_ReadWrite_Buf: /* 1202*/
		if (size > ((64*12*12*2+6)*sizeof(unsigned int)))
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(80000 * sizeof(unsigned char));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeUINT32Endian((unsigned int *)buf , size/4, 1);
		memcpy((unsigned char *)pwUINT32Temp, (unsigned char *)buf, size);
		/*save to buff, for pre/cur ICM change*/
		rtice_ICM_ReadWrite_CurrentPrevious_Buff_save(rtice_param, 0, pwUINT32Temp,  system_setting_info, 0);

		g_h_sta = pwUINT32Temp[0];
		g_h_end = pwUINT32Temp[1];
		g_s_sta = pwUINT32Temp[2];
		g_s_end = pwUINT32Temp[3];
		g_i_sta = pwUINT32Temp[4];
		g_i_end = pwUINT32Temp[5];

		if (size > sizeof(unsigned int)*((g_i_end - g_i_sta + 1) * (g_s_end - g_s_sta + 1) * (g_h_end - g_h_sta + 1) * 2 + 6)) {
			vip_free(pwUINT32Temp);
			pwUINT32Temp = NULL;
			return -ID_SIZE_ERR;
		}

		/*write to register*/
		fwif_color_icm_SramBlockAccessSpeedup(pwUINT32Temp+6, 1,  g_h_sta,  g_h_end,  g_s_sta,  g_s_end,  g_i_sta,  g_i_end);

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;
		break;

	case TOOLACCESS_ICM_ReadWrite_CurrentPrevious_Buf: /* 1203*/
		if (size > sizeof(unsigned int))
			return -ID_SIZE_ERR;
		if (rtice_ICM_ReadWrite_From_CurrentPrevious_Buff(rtice_param, NULL, buf, system_setting_info, 1) < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_MPEG_SW_Ctrl:

		if (size > sizeof(DRV_MPEGNR_16_SW_CTRL))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc( sizeof(DRV_MPEGNR_16_SW_CTRL));

		if (pwIntTemp == NULL)
			return -ID_SIZE_ERR;



		fwif_color_ChangeINT32Endian(( int*)buf,size>>2, 1);

		memcpy((unsigned char *)pwIntTemp, (unsigned char *)buf, size);

		if(rtice_param.mode0 ==0)
			drvif_color_set_DRV_MPG_H_SW_Ctrl((DRV_MPEGNR_16_SW_CTRL*)pwIntTemp);
		else
			drvif_color_set_DRV_MPG_V_SW_Ctrl((DRV_MPEGNR_16_SW_CTRL*)pwIntTemp);

		vip_free(pwIntTemp);

		pwIntTemp = NULL;


		break;

	case TOOLACCESS_PCID_Tbl: /*1250*/

		if (size > sizeof(char)*PCID_TABLE_LEN)
			return -ID_SIZE_ERR;

		pCharTemp = (char *)vip_malloc(sizeof(char)*PCID_TABLE_LEN);

		if(pCharTemp == NULL)
			return -ID_SIZE_ERR;

		memcpy((char*)pCharTemp, buf, size);

		fwif_color_set_pcid_single_valuetable(pCharTemp, rtice_param.mode0, rtice_param.mode1);

		vip_free(pCharTemp);

		break;

		break;

#if defined(CONFIG_RTK_AI_DRV)
	case TOOLACCESS_AI_Ctrl_Tbl: /*1260 set*/
		if (size > sizeof(DRV_AI_Ctrl_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_Ctrl_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian(( int*)buf,size>>2, 1);

		memcpy((unsigned char *)pwIntTemp, (unsigned char *)buf, size);

		scaler_AI_obj_Ctrl_Set((DRV_AI_Ctrl_table *)pwIntTemp);

		vip_free(pwIntTemp);

		pwIntTemp = NULL;

		break;

// lesley 1024
	case TOOLACCESS_AI_Ctrl_Draw: /*1261 set*/
		if (size > sizeof(unsigned char)*12)
			return -ID_SIZE_ERR;

		//aipq_mode.clock_status	= (size>=sizeof(unsigned char)*1)? buf[0] : aipq_mode.clock_status;
		aipq_mode.ap_mode		= (size>=sizeof(unsigned char)*2)? buf[1] : aipq_mode.ap_mode;
		aipq_mode.face_mode		= (size>=sizeof(unsigned char)*3)? buf[2] : aipq_mode.face_mode;
		aipq_mode.sqm_mode		= (size>=sizeof(unsigned char)*4)? buf[3] : aipq_mode.sqm_mode;
		aipq_mode.scene_mode	= (size>=sizeof(unsigned char)*5)? buf[4] : aipq_mode.scene_mode;
		aipq_mode.genre_mode	= (size>=sizeof(unsigned char)*6)? buf[5] : aipq_mode.genre_mode;
		aipq_mode.depth_mode	= (size>=sizeof(unsigned char)*7)? buf[6] : aipq_mode.depth_mode;
		aipq_mode.obj_mode		= (size>=sizeof(unsigned char)*8)? buf[7] : aipq_mode.obj_mode;
		aipq_mode.pqmask_mode	= (size>=sizeof(unsigned char)*9)? buf[8] : aipq_mode.pqmask_mode;
		aipq_mode.semantic_mode	= (size>=sizeof(unsigned char)*10)? buf[9] : aipq_mode.semantic_mode;
		aipq_mode.logo_mode	= (size>=sizeof(unsigned char)*11)? buf[10] : aipq_mode.logo_mode;
		aipq_mode.noise_mode	= (size>=sizeof(unsigned char)*12)? buf[11] : aipq_mode.noise_mode;
		
		scalerAI_pq_mode_ctrl(aipq_mode, 0);

		/* do not need it at mac8p
		if(buf[0]==0){
			aipq_mode.clock_status=0;
			scalerAI_enableClock(0);
		}else{
			scalerAI_enableClock(1);
			aipq_mode.clock_status=1;
		}
		*/
		
		aipq_mode_pre.clock_status = aipq_mode.clock_status;
		aipq_mode_pre.ap_mode = aipq_mode.ap_mode;
		aipq_mode_pre.face_mode = aipq_mode.face_mode;
		aipq_mode_pre.sqm_mode = aipq_mode.sqm_mode;
		aipq_mode_pre.scene_mode = aipq_mode.scene_mode;
		aipq_mode_pre.genre_mode = aipq_mode.genre_mode;
		aipq_mode_pre.depth_mode = aipq_mode.depth_mode;
		aipq_mode_pre.obj_mode = aipq_mode.obj_mode;
		aipq_mode_pre.pqmask_mode = aipq_mode.pqmask_mode;
		aipq_mode_pre.semantic_mode = aipq_mode.semantic_mode;
		// end 0622 lsy

		break;
// end lesley 1024

// lesley 1016
	case TOOLACCESS_AI_TUNE_ICM: /*1263 set*/
		if (size > sizeof(DRV_AI_Tune_ICM_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_Tune_ICM_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian(( int*)buf,size>>2, 1);

		memcpy((unsigned char *)pwIntTemp, (unsigned char *)buf, size);

		drivef_ai_tune_icm_set((DRV_AI_Tune_ICM_table *) pwIntTemp);

		vip_free(pwIntTemp);

		pwIntTemp = NULL;

		break;

	case TOOLACCESS_AI_TUNE_DCC: /*1264 set*/
		if (size > sizeof(DRV_AI_Tune_DCC_table))
			return -ID_SIZE_ERR;

		pwIntTemp = (int *)vip_malloc(sizeof(DRV_AI_Tune_DCC_table));

		if(pwIntTemp == NULL)
			return -ID_SIZE_ERR;

		fwif_color_ChangeINT32Endian(( int*)buf,size>>2, 1);

		memcpy((unsigned char *)pwIntTemp, (unsigned char *)buf, size);

		drivef_ai_tune_dcc_set((DRV_AI_Tune_DCC_table *) pwIntTemp, 1);

		vip_free(pwIntTemp);

		pwIntTemp = NULL;

		break;
// end lesley 1016
#endif

	case TOOLACCESS_AutoMA_API_CTRL:
	if (size >  sizeof(VIP_AutoMA_API_CTRL))
		return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *) &(system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL), buf, size);
#else
	memcpy((unsigned char *) &(system_setting_info->S_RPC_AutoMA_Flag.AutoMA_API_CTRL), buf, size);
#endif
	break;

	/***************************for YUV2RGB Table access*****************************************/ /**/
	case TOOLACCESS_YUV2RGB_Global_Sat_Gain:
		if (rtice_SetGet_YUV2RGB_Table_Info(TOOLACCESS_YUV2RGB_Global_Sat_Gain, 1, size, NULL, buf, system_setting_info, gVip_Table) < 0)
			return -ID_SIZE_ERR;
	break;
	case TOOLACCESS_YUV2RGB_Global_Hue_Gain:
		if (rtice_SetGet_YUV2RGB_Table_Info(TOOLACCESS_YUV2RGB_Global_Hue_Gain, 1, size, NULL, buf, system_setting_info, gVip_Table) < 0)
			return -ID_SIZE_ERR;
	break;
	case TOOLACCESS_YUV2RGB_Global_Contrast_Gain:
		if (rtice_SetGet_YUV2RGB_Table_Info(TOOLACCESS_YUV2RGB_Global_Contrast_Gain, 1, size, NULL, buf, system_setting_info, gVip_Table) < 0)
			return -ID_SIZE_ERR;
	break;
	case TOOLACCESS_YUV2RGB_Global_Brightness_Gain:
		if (rtice_SetGet_YUV2RGB_Table_Info(TOOLACCESS_YUV2RGB_Global_Brightness_Gain, 1, size, NULL, buf, system_setting_info, gVip_Table) < 0)
			return -ID_SIZE_ERR;
	break;

	/***************************for YUV2RGB Table access*****************************************/ /**/
	case TOOLACCESS_Function_Coef_Dlti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetDLti(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_Dcti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetDCti(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_iDlti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetIDLti(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_iDcti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetIDCti(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_VDcti:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetVDCti(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_UV_Delay_En:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetColor_UV_Delay_Enable(buf[0]) ;

		break;

	case TOOLACCESS_Function_Coef_UV_Delay:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetColor_UV_Delay(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_YUV2RGB:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_set_YUV2RGB(buf[0], 0, 0);

		break;

	case TOOLACCESS_Function_Coef_Film_Mode:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetFilm_Mode(buf[0]);

		break;
	case TOOLACCESS_Function_Coef_Intra:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetIntra(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_MA:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		drvif_color_Stop_VGIP_isr(1);

		Scaler_SetMADI(buf[0]);

		scalerVIP_Set_pq_reset(1);
		drvif_color_Stop_VGIP_isr(0);

		break;

	case TOOLACCESS_Function_Coef_TnrXC:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		drvif_color_Stop_VGIP_isr(1);

		Scaler_SetTNRXC(buf[0]);

		scalerVIP_Set_pq_reset(1);
		drvif_color_Stop_VGIP_isr(0);

		break;

	case TOOLACCESS_Function_Coef_Ma_Chroma_Error:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetMA_Chroma_Error(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_NM_Level:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		//Scaler_SetRTNR_Noise_Measure(buf[0]); //mac6 not use

		break;
	case TOOLACCESS_Function_Coef_Madi_Hme:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		drvif_color_Stop_VGIP_isr(1);

		Scaler_SetMADI_HME(buf[0]);

		scalerVIP_Set_pq_reset(1);
		drvif_color_Stop_VGIP_isr(0);

		break;

	case TOOLACCESS_Function_Coef_Madi_Hmc:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		drvif_color_Stop_VGIP_isr(1);

		Scaler_SetMADI_HMC(buf[0]);

		scalerVIP_Set_pq_reset(1);
		drvif_color_Stop_VGIP_isr(0);

		break;

	case TOOLACCESS_Function_Coef_Madi_Pan:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		drvif_color_Stop_VGIP_isr(1);

		Scaler_SetMADI_PAN(buf[0]);

		scalerVIP_Set_pq_reset(1);
		drvif_color_Stop_VGIP_isr(0);

		break;

	case TOOLACCESS_Function_Coef_Di_Smd:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetDiSmd(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_NEW_UVC:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetNew_UVC(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_UV_DELAY_TOP:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		//CURRENT NOT USE

		break;

	case TOOLACCESS_Function_Coef_V_DCTi_LPF:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetVDCti_LPF(buf[0]);

		break;


	case TOOLACCESS_Function_Extend_CDS:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetCDSTable(buf[0]);

		break;

	case TOOLACCESS_Function_Extend_Emfmk2:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetEMF_Mk2(buf[0]);

		break;
	case TOOLACCESS_Function_Extend_Adatp_Gamma:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		//Scaler_set_Adaptive_Gamma(buf[0]);

		break;

	case TOOLACCESS_Function_Extend_LD_Ebabel:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
#ifdef LocalDimming
		Scaler_SetLocalDimmingEnable(buf[0]);
#endif

		break;

	case TOOLACCESS_Function_Extend_LD_Table_Select:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

#ifdef LocalDimming
		Scaler_SetLocalDimmingTable(buf[0]);
#endif

		break;
	case TOOLACCESS_Function_Extend_LC_Enable:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		 Scaler_SetLocalContrastEnable(buf[0]);
		break;
	case TOOLACCESS_Function_Extend_LC_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetLocalContrastTable(buf[0]);

		break;
	case TOOLACCESS_Function_Extend_3dLUT_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_Set3dLUT(buf[0]);

		break;
	case TOOLACCESS_Function_Extend_I_De_XC:

		return -ID_SIZE_ERR; //not support


		break;
	case TOOLACCESS_Function_Extend_I_De_Contour:

		return -ID_SIZE_ERR; //not support

			break;
	case TOOLACCESS_Function_Extend_SLD:

		return -ID_SIZE_ERR; //not support

			break;

	case TOOLACCESS_Function_Extend2_SU_H: //4tab

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);

		Scaler_SetScaleUP_YCHV(buf[0],VY,HC,VC);

		break;

	case TOOLACCESS_Function_Extend2_SU_V:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);

		Scaler_SetScaleUP_YCHV(HY,buf[0],HC,VC);

		break;

	case TOOLACCESS_Function_Extend2_S_PK:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetScalePK(buf[0]);

		break;

	case TOOLACCESS_Function_Extend2_SUPK_Mask:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetSUPKType(buf[0]);

		break;

	case TOOLACCESS_Function_Extend2_Unsharp_Mask:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetUnsharpMask(buf[0]);

		break;

	case TOOLACCESS_Function_Extend2_Reserve5:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_Function_Extend2_Iegsm_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_setiESM(buf[0]);

		break;

	case TOOLACCESS_Function_Extend2_SR_Init_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;


		break;

	case TOOLACCESS_Function_Extend2_SR_Edge_Gain:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;


		break;

	case TOOLACCESS_Function_Extend2_SR_TEX_Gain:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		break;


	case TOOLACCESS_Function_Extend2_SD_H_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetScaleDOWNHTable(buf[0]);

		break;

	case TOOLACCESS_Function_Extend2_SD_V_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetScaleDOWNVTable(buf[0]);

		break;


	case TOOLACCESS_Function_Extend2_SD_444TO422:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetScaleDOWN444To422(buf[0]);

		break;

	case TOOLACCESS_Function_Extend2_Color_Space_Control:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;


		break;
	case TOOLACCESS_Function_Extend2_SU_H_8Tab:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetScaleUPH_12tap(buf[0]);

		break;
	case TOOLACCESS_Function_Extend2_SU_V_6Tab:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetScaleUPV_8tap(buf[0]);

		break;
	case TOOLACCESS_Function_Extend2_OSD_Sharpness:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;


		break;
	case TOOLACCESS_Function_Extend2_SU_DIR:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetScaleUPDir(buf[0]);

		break;
	case TOOLACCESS_Function_Extend2_SU_DIR_Weigh:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetScaleUPDir_Weighting(buf[0]);

		break;
	case TOOLACCESS_Function_Extend2_SU_C_H://4tab

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);

		Scaler_SetScaleUP_YCHV(HY,VY,buf[0],VC);

		break;
	case TOOLACCESS_Function_Extend2_SU_C_V://4tab

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_GetScaleUP_YCHV(&HY,&VY,&HC,&VC);

		Scaler_SetScaleUP_YCHV(HY,VY,HC,buf[0]);

		break;

	case TOOLACCESS_Function_Extend3_VD_CON_BRI_HUE_SAT:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_module_vdc_SetConBriHueSat_Table(buf[0]);

		break;

	case TOOLACCESS_Function_Extend3_ICM_Tablee:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_set_ICM_table(buf[0], 0);

		break;
	case TOOLACCESS_Function_Extend3_Gamma:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetGamma(buf[0]);

		break;

	case TOOLACCESS_Function_Extend3_S_Gamma_Index:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetSindex(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_S_Gamma_Low:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetSlow(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_S_Gamma_High:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetShigh(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_DCC_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetDCC_Table(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_DCC_Color_Indep_t:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_Set_DCC_Color_Independent_Table(buf[0]);

		break;

	case TOOLACCESS_Function_Extend3_DCC_Chroma_Comp_t:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_Set_DCC_chroma_compensation_Table(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_Sharpness_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetSharpnessTable(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_NR_Table:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		scaler_set_PQA_table(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_PQA_INPUT_TABLE:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		scaler_set_PQA_Input_table(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_MB_Peaking:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetMBPeaking(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_blue_stretch:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_set_blue_stretch(buf[0]);

		break;
	case TOOLACCESS_Function_Extend3_MB_SU_Peaking:

		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_SetMBSUPeaking(buf[0]);

		break;

	case TOOLACCESS_Function_Coef_ALL:

		if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;

		if (rtice_param.mode1 == 0) {
			memcpy((unsigned char *) &(gVip_Table->VIP_QUALITY_Coef[0][0]), buf, size);
		} else {
			Scaler_SetDLti(buf[0]);
			Scaler_SetDCti(buf[1]);
			Scaler_SetIDLti(buf[2]);
			Scaler_SetIDCti(buf[3]);
			Scaler_SetVDCti(buf[4]);
			Scaler_SetColor_UV_Delay_Enable(buf[5]) ;
			Scaler_SetColor_UV_Delay(buf[6]);
			Scaler_set_YUV2RGB(buf[7], 0, 0);
			Scaler_SetFilm_Mode(buf[8]);
			Scaler_SetIntra(buf[9]);
			Scaler_SetMADI(buf[10]);
			Scaler_SetTNRXC(buf[11]);
			Scaler_SetMA_Chroma_Error(buf[12]);
			Scaler_SetRTNR_Noise_Measure(buf[13]);
			Scaler_SetMADI_HME(buf[14]);
			Scaler_SetMADI_HMC(buf[15]);
			Scaler_SetMADI_PAN(buf[16]);
			Scaler_SetDiSmd(buf[17]);
			Scaler_SetNew_UVC(buf[18]);
			//19 not use
			Scaler_SetVDCti_LPF(buf[20]);


		}

		break;
	case TOOLACCESS_Function_Extend_ALL:

		if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;

		if (rtice_param.mode1 == 0) {
			memcpy((unsigned char *) &(gVip_Table->VIP_QUALITY_Extend_Coef[0][0]), buf, size);
		} else {
			Scaler_SetCDSTable(buf[0]);
			Scaler_SetEMF_Mk2(buf[1]);
			//Scaler_set_Adaptive_Gamma(buf[5]);
	#ifdef LocalDimming
			Scaler_SetLocalDimmingEnable(buf[6]);
			Scaler_SetLocalDimmingTable(buf[7]);
	#endif
		}
		break;

	case TOOLACCESS_Function_Extend2_ALL:

		if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;

		if (rtice_param.mode1 == 0) {
			memcpy((unsigned char *) &(gVip_Table->VIP_QUALITY_Extend2_Coef[0][0]), buf, size);
		} else {

			Scaler_SetScaleUP_YCHV(buf[0],buf[1],buf[19],buf[20]);


			Scaler_SetScalePK(buf[2]);
			Scaler_SetSUPKType(buf[3]);
			Scaler_SetUnsharpMask(buf[4]);
			Scaler_setiESM(buf[6]);
			Scaler_SetScaleDOWNHTable(buf[10]);
			Scaler_SetScaleDOWNVTable(buf[11]);
			Scaler_SetScaleDOWN444To422(buf[12]);
			Scaler_SetScaleUPH_12tap(buf[14]);
			Scaler_SetScaleUPV_8tap(buf[15]);
			Scaler_SetScaleUPDir(buf[17]);
			Scaler_SetScaleUPDir_Weighting(buf[18]);

		}

		break;
	case TOOLACCESS_Function_Extend3_ALL:

		if (size > VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK)
			return -ID_SIZE_ERR;

		if (rtice_param.mode1 == 0) {
			memcpy((unsigned char *) &(gVip_Table->VIP_QUALITY_Extend3_Coef[0][0]), buf, size);
		} else {
			Scaler_module_vdc_SetConBriHueSat_Table(buf[0]);
			Scaler_set_ICM_table(buf[1], 0);
			Scaler_SetGamma(buf[2]);
			Scaler_SetSindex(buf[3]);
			Scaler_SetSlow(buf[4]);
			Scaler_SetShigh(buf[5]);
			Scaler_SetDCC_Table(buf[6]);
			Scaler_Set_DCC_Color_Independent_Table(buf[7]);
			Scaler_Set_DCC_chroma_compensation_Table(buf[8]);
			Scaler_SetSharpnessTable(buf[9]);
			scaler_set_PQA_table(buf[10]);
			scaler_set_PQA_Input_table(buf[11]);
			Scaler_SetMBPeaking(buf[12]);
			Scaler_set_blue_stretch(buf[13]);
			Scaler_SetMBSUPeaking(buf[20]);


		}

		break;
	case TOOLACCESS_Function_Extend4_ALL:

		break;


	case TOOLACCESS_LocalContrast_Curve:
	{
		if (size > sizeof(unsigned int)*(256))
			return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int *)vip_malloc(256 * sizeof(unsigned int));

		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned char *)pwUINT32Temp, (unsigned char *)buf, size);
		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);

		drvif_color_set_LC_ToneMappingSlopePoint(pwUINT32Temp);

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

	}

	break;

	case TOOLACCESS_ColorGamut_Matrix:
		if (size > 9 * (sizeof(short)))
			return -ID_SIZE_ERR;
		pwINT16Temp = (short *)vip_malloc(9 * sizeof(short));

		if (pwINT16Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy((unsigned char *)pwINT16Temp, (unsigned char *)buf, size);

		fwif_color_ChangeUINT16Endian((short *)pwINT16Temp , size/2, 1);

		memcpy((unsigned char *)gColorGamutMatrix_Apply, (unsigned char *)pwINT16Temp, size);

		fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, gColorGamutMatrix_Apply, 2, 1, 0);

		vip_free(pwINT16Temp);
		pwINT16Temp = NULL;

		break;

	case TOOLACCESS_ICM_GainAdjust:
		return -ID_SIZE_ERR; //don't allow write
		break;

	case TOOLACCESS_DSE_Gain:
	{
		VPQ_SAT_LUT_T lut;

		if (size > sizeof(unsigned short)* (VPQ_SaturationLUT_Seg_Max * 3))
			return -ID_SIZE_ERR;

		pwINT16Temp = (unsigned short *)vip_malloc((VPQ_SaturationLUT_Seg_Max * 3) * sizeof(unsigned short));
		if (pwINT16Temp) {

			memcpy((unsigned char *)pwINT16Temp, (unsigned char *)buf, size);
			fwif_color_ChangeUINT16Endian(pwINT16Temp, (VPQ_SaturationLUT_Seg_Max * 3), 1);

			//stop DSE HAL
			fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_SAT_LUT, ACCESS_MODE_SET, 1);
			msleep(10);

			//set DSE
			drvif_color_get_YUV2RGB_nonliner_index(lut.uSaturationX);
			memcpy(lut.uSaturationY, pwINT16Temp + VPQ_SaturationLUT_Seg_Max, VPQ_SaturationLUT_Seg_Max * sizeof(unsigned short));
			fwif_color_set_DSE_tv006(SLR_MAIN_DISPLAY, &lut);

			vip_free(pwINT16Temp);
			pwINT16Temp = NULL;
		}
		break;
	}

	case TOOLACCESS_D_3D_LUT_Index:
	{
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		fwif_color_3d_lut_access_index(ACCESS_MODE_SET, buf[0]);

		break;
	}
	case TOOLACCESS_D_3D_LUT_Reset:
	{
		unsigned char idx;
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		fwif_color_3d_lut_init_tv006();
		idx = fwif_color_3d_lut_access_index(ACCESS_MODE_GET, 0);
		fwif_color_set_3dLUT(idx+1);

		break;
	}
	case TOOLACCESS_OD_Table:
	{
		extern unsigned char od_table_tv006_store[OD_table_length];
		extern unsigned int od_table_store[OD_table_length];
		extern unsigned int od_delta_table[OD_table_length];
		if (size > sizeof(unsigned char)*OD_table_length)
			return -ID_SIZE_ERR;

		memcpy(od_table_tv006_store, buf, size);
		fwif_color_od_table_tv006_convert(od_table_tv006_store, od_table_store);

		fwif_color_od_table_tv006_transform(0, od_table_store, od_delta_table);
		drvif_color_od_table(od_delta_table, 0);// 0:delta mode 1:target mode(output mode)

		break;
	}
	case TOOLACCESS_OD_Table_17x17:
	{
		extern unsigned char od_table_tv006_store[OD_table_length];
		extern unsigned int od_table_store[OD_table_length];
		extern unsigned int od_delta_table[OD_table_length];
		if (size > sizeof(unsigned char)*OD_table_length)
			return -ID_SIZE_ERR;

		// 17x17 target table input
		memcpy(od_table_tv006_store, buf, size);

    // Mac5p 17x17 henry
		fwif_color_od_table_17x17_transform(0, od_table_tv006_store, od_delta_table);
		drvif_color_od_table_write(od_delta_table, 0, 3);// 0:delta mode 1:target mode(output mode)

		break;
	}
	case TOOLACCESS_VPQ_IOCTL_StopRun_cmd:
	{
		int i;
		if (size > sizeof(unsigned char)*256)
			return -ID_SIZE_ERR;

		for (i = 0; i < 256 && i < size/sizeof(unsigned char); i++)
			vpq_ioctl_set_stop_run_by_idx(i, buf[i]);

		break;
	}
	case TOOLACCESS_VPQ_LED_IOCTL_StopRun_cmd:
	{
		int i;
		if (size > sizeof(unsigned char)*16)
			return -ID_SIZE_ERR;

		for (i = 0; i < 16 && i < size/sizeof(unsigned char); i++)
			vpq_led_ioctl_set_stop_run_by_idx(i, buf[i]);

		break;
	}
	case TOOLACCESS_VPQ_MEMC_IOCTL_StopRun_cmd:
	{
		int i;
		if (size > sizeof(unsigned char)*32)
			return -ID_SIZE_ERR;

		for (i = 0; i < 32 && i < size/sizeof(unsigned char); i++)
			vpq_memc_ioctl_set_stop_run_by_idx(i, buf[i]);

		break;
	}
	case TOOLACCESS_VPQ_MEMC_DynamicBypass:
	{
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		vbe_disp_set_dynamic_memc_bypass_flag(buf[0]);

		break;
	}
	case TOOLACCESS_VPQ_DIGameMode_Only:
	{
		extern unsigned char g_bDIGameModeOnlyDebug;
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		g_bDIGameModeOnlyDebug = buf[0];

		break;
	}
	case TOOLACCESS_VPQ_IOCTL_StopRun:
	{
		VPQ_IOCTL_STOP_RUN i;
		if (size > sizeof(unsigned char)*STOP_VPQ_IOC_MAX)
			return -ID_SIZE_ERR;

		for (i = STOP_VPQ_IOC_START; i < STOP_VPQ_IOC_MAX && i < size/sizeof(unsigned char); i++)
			fwif_color_vpq_stop_ioctl(i, ACCESS_MODE_SET, buf[i]);

		break;
	}
	case TOOLACCESS_DemoOverscan:
		if (size > sizeof(unsigned short)*9)
			return -ID_SIZE_ERR;

		if (fwif_color_get_demo_callback(DEMO_CALLBACKID_OVERSCAN)) {
			pwINT16Temp = (unsigned short *)vip_malloc(9 * sizeof(unsigned short));
			if (pwINT16Temp == NULL)
				return -ID_SIZE_ERR;
			memcpy((unsigned char *)pwINT16Temp, (unsigned char *)buf, size);
			fwif_color_ChangeUINT16Endian(pwINT16Temp, 9, 1);
			(fwif_color_get_demo_callback(DEMO_CALLBACKID_OVERSCAN))((void *)pwINT16Temp);
			vip_free(pwINT16Temp);
			pwINT16Temp = NULL;
		}
		break;
	case TOOLACCESS_DemoPQ:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		if (fwif_color_get_demo_callback(DEMO_CALLBACKID_ON_OFF_SWITCH))
			(fwif_color_get_demo_callback(DEMO_CALLBACKID_ON_OFF_SWITCH))((void *)buf);

		break;

	case TOOLACCESS_ONEKEY_HDR:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_LGE_ONEKEY_HDR10_Enable(1);
		break;
	case TOOLACCESS_ONEKEY_BBC:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_LGE_ONEKEY_BBC_Enable(1);
		break;

	case TOOLACCESS_ONEKEY_SDR2HDR:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_LGE_ONEKEY_SDR2HDR_Enable(1);
		break;

	case TOOLACCESS_TC_DEMO:
	{
		DRV_TC_HDR_CTRL *pTC_HDR_CTRL = (DRV_TC_HDR_CTRL*)fwif_color_Get_TC_CTRL();

		if (size > sizeof(DRV_TC_HDR_CTRL))
			return -ID_SIZE_ERR;

		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		/*fwif_color_set_TC_v130_metadata2reg_init(TC_HDR_CTRL.TC_CTRL[TC_HDR_display_OETF], TC_HDR_CTRL.TC_CTRL[TC_HDR_display_Brightness],
			TC_HDR_CTRL.TC_CTRL[TC_HDR_display_proc_mode], 0, 0, TC_HDR_CTRL.TC_CTRL[TC_HDR_display_yuv_range]);*/

		memcpy(	(unsigned char*)pTC_HDR_CTRL, &buf[0], size);

		break;
	}

	case TOOLACCESS_ST2094_CTRL:
		if (size > sizeof(VIP_ST2094_CTRL))
			return -ID_SIZE_ERR;
		ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();

		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);

		pwUINT32Temp = ((unsigned int*)ST2094_CTRL) + start_addr;

		memcpy(	(unsigned char*)pwUINT32Temp, &buf[0], size);

		RPC_system_setting_info->HDR_info.Debug_Log_3 = ST2094_CTRL->Debug_Log;
		RPC_system_setting_info->HDR_info.Debug_Log_Delay_Frame = ST2094_CTRL->Debug_Delay;

		// for debug
		//if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_Gamma22 == 0)
			drvif_DM2_GAMMA_Enable(1);
		//else
		//	drvif_DM2_GAMMA_Enable(0);

		break;

	case TOOLACCESS_ST2094_Basis_OOTF_Info:
	{
		if (size > sizeof(int) * ST2094_Basis_OOTF_Info_Max)
			return -ID_SIZE_ERR;

		ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();

		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		pwUINT32Temp = (unsigned int *)vip_malloc(ST2094_Basis_OOTF_Info_Max * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		memcpy(	(unsigned char*)pwUINT32Temp, &buf[0], size);

		ST2094_CTRL->Debug_basisOOTF_kneePx = pwUINT32Temp[0];	/* in 12 bits*/
		ST2094_CTRL->Debug_basisOOTF_kneePy = pwUINT32Temp[1];	/* in 12 bits*/
		drvif_fwif_color_Get_ST2094_TBL()->basisOOTF_order = pwUINT32Temp[2];
		for(i=0;i<bezier_curve_anchors_num;i++)
			ST2094_CTRL->Debug_basisOOTF_P[i] = pwUINT32Temp[i+3];	/* in 10 bits*/
		ST2094_CTRL->Debug_basisOOTF_Fixed_KneePoint = pwUINT32Temp[18];
		ST2094_CTRL->Debug_basisOOTF_Fixed_Anchors= pwUINT32Temp[19];

		vip_free(pwUINT32Temp);
		break;
	}

	case TOOLACCESS_ST2094_OETF_CTRL_Info:
	{
		if (size > sizeof(int) * ST2094_OETF_CTRL_Info_Max)
			return -ID_SIZE_ERR;

		ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();

		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);
		pwUINT32Temp = (unsigned int *)vip_malloc(ST2094_OETF_CTRL_Info_Max * sizeof(int));
		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;
		memcpy(	(unsigned char*)pwUINT32Temp, &buf[0], size);

		ST2094_CTRL->Enable							= pwUINT32Temp[0];
		drvif_fwif_color_Get_ST2094_TBL()->product_peak				= pwUINT32Temp[1];
		drvif_fwif_color_Get_ST2094_TBL()->product_min		= pwUINT32Temp[2];
		drvif_fwif_color_Get_ST2094_TBL()->basis_peak				= pwUINT32Temp[3];
		drvif_fwif_color_Get_ST2094_TBL()->curve_sel				= pwUINT32Temp[4];
		drvif_fwif_color_Get_ST2094_TBL()->EOTF_norm_div			= pwUINT32Temp[5];
		drvif_fwif_color_Get_ST2094_TBL()->EOTF_Linear_Blend_W		= pwUINT32Temp[6];
		drvif_fwif_color_Get_ST2094_TBL()->EOTF_Gamma22			= pwUINT32Temp[7];
		ST2094_CTRL->Debug_Log						= pwUINT32Temp[8];
		ST2094_CTRL->Debug_Delay				= pwUINT32Temp[9];
		vip_free(pwUINT32Temp);

		RPC_system_setting_info->HDR_info.Debug_Log_3 = ST2094_CTRL->Debug_Log;
		RPC_system_setting_info->HDR_info.Debug_Log_Delay_Frame = ST2094_CTRL->Debug_Delay;

		// for debug
		//if (drvif_fwif_color_Get_ST2094_TBL()->EOTF_Gamma22 == 0)
			drvif_DM2_GAMMA_Enable(1);
		//else
		//	drvif_DM2_GAMMA_Enable(0);

		break;
	}

	case TOOLACCESS_HDR_VIVI_CTRL:
	{
		VIP_HDR10_vivid_CTRL* hdr_vivd_CTRL = NULL;
		
		if ((size+start_addr) > sizeof(VIP_HDR10_vivid_CTRL))
			return -ID_SIZE_ERR;

		hdr_vivd_CTRL = drvif_fwif_color_Get_HDR10_vivid_Ctrl();

		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);

		pwUINT32Temp = ((unsigned int*)hdr_vivd_CTRL) + start_addr;
		
		memcpy(pwUINT32Temp, &buf[0], size);
		break;
	}

#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
	case TOOLACCESS_OSD_HDRMODE_DEMO:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		fwif_color_set_OSD_enhance_enable(buf[0]);
		break;
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE

	case TOOLACCESS_LOCALCONTRAST_DEMO:
		if (size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		Scaler_color_LC_DemoSplit_Screen(buf[0]);
		break;

	case TOOLACCESS_DM_HDR_3D_Lut_TBL:
		if (size > (sizeof(unsigned int) * (VIP_DM_HDR_3D_LUT_UI_TBL_SIZE)))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(VIP_DM_HDR_3D_LUT_UI_TBL_SIZE * sizeof(int));

		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT32Temp, buf, size);

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);

		//fwif_color_set_DM_HDR_3dLUT(255, pwUINT32Temp);
		for(i=0;i<VIP_DM_HDR_3D_LUT_UI_TBL_SIZE;i++)
			HDR10_3D_LUT_17x17x17_temp[i] = *(pwUINT32Temp+i);
		hdr_3DLUT_args.a3dLUT_16_pArray = &HDR10_3D_LUT_17x17x17_temp[0];
		fwif_color_set_DM_HDR_3dLUT_17x17x17_16_TV006(&hdr_3DLUT_args);

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		break;

	case TOOLACCESS_DM2_HDR_EOTF_TBL:
	case TOOLACCESS_DM2_HDR_OETF_TBL:
	case TOOLACCESS_DM2_HDR_Tone_Mapping_TBL:
	case TOOLACCESS_DM2_HDR_24x24x24_3D_TBL:
		ret = rtice_SetGet_HDR_DM2(rtice_param, NULL, buf, system_setting_info, 1);
		if (ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_D_3D_Lut_TBL:
	{
		extern unsigned int D_3D_DRV_LUT[VIP_D_3D_LUT_DRV_TBL_SIZE];
		extern unsigned int LUT_3D[LUT3D_TBL_Num][LUT3D_TBL_ITEM];
		int idx = fwif_color_3d_lut_access_index(ACCESS_MODE_GET, 0);

		//SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

		if (size > (sizeof(unsigned int) * (VIP_D_3D_LUT_UI_TBL_SIZE)))
			return -ID_SIZE_ERR;

		pwUINT32Temp = (unsigned int *)vip_malloc(VIP_D_3D_LUT_UI_TBL_SIZE * sizeof(int));

		if (pwUINT32Temp == NULL)
			return -ID_SIZE_ERR;

		memcpy(pwUINT32Temp, buf, size);

		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);

		if (rtice_param.mode0 == 1)
			rtice_Set_D_LUT_9to17(pwUINT32Temp);

		if (rtice_param.mode1 == 1) {
			fwif_color_D_LUT_By_DMA(0, 1, pwUINT32Temp);
		} else if (rtice_param.mode1 == 2) {
			pwUINT16Temp = (unsigned short *)vip_malloc(VIP_D_3D_LUT_UI_TBL_SIZE * sizeof(short));
			
			if (pwUINT16Temp == NULL) {
				vip_free(pwUINT32Temp);
				pwUINT32Temp = NULL;
				return -ID_SIZE_ERR;
			}
			
			for (i=0;i<VIP_D_3D_LUT_UI_TBL_SIZE;i++)
				pwUINT16Temp[i] = pwUINT32Temp[i];
			
			fwif_color_set_DDomain_3dLUT_16_TV006(pwUINT16Temp, 1);
		} else {
			fwif_color_set_D_3dLUT(255, pwUINT32Temp);
		}
		
		if (idx < LUT3D_TBL_Num) {
			memcpy(LUT_3D[idx], D_3D_DRV_LUT, sizeof(D_3D_DRV_LUT));
		}

		vip_free(pwUINT32Temp);
		pwUINT32Temp = NULL;

		if (pwUINT16Temp != NULL)
			vip_free(pwUINT16Temp);
		pwUINT16Temp = NULL;
		break;
	}

    case TOOLACCESS_Scaler_Coefficient:/*1951*/
     {
        extern unsigned char a_rgb_scale_down_coef_id_h[c_rgb_scale_down_ratio_seg];
        extern unsigned char a_rgb_scale_down_coef_id_v[c_rgb_scale_down_ratio_seg];
        extern unsigned char a_rgb_scale_up_coef_id_h[c_rgb_scale_up_ratio_seg];
        extern unsigned char a_rgb_scale_up_coef_id_v[c_rgb_scale_up_ratio_seg];
        unsigned char i=0;
		if (size > sizeof(unsigned char)*24)
			return -ID_SIZE_ERR;

        for (i=0;i<6;i++){
		    a_rgb_scale_up_coef_id_h[i]=buf[i];
            a_rgb_scale_up_coef_id_v[i]=buf[i+6];
            a_rgb_scale_down_coef_id_h[i]=buf[i+12];
            a_rgb_scale_down_coef_id_v[i]=buf[i+18];
        }
        
        drvif_color_scaler_set_scaleup_coefRGB_0();
        drvif_color_scaler_set_scaledown_coefRGB_0();

     }
    break;

	case TOOLACCESS_DM2_EOTF_byLuminance_CTRL: /*1930*/
		if (size > (sizeof(unsigned short)*8))
			return -ID_SIZE_ERR;

		fwif_color_ChangeUINT16Endian((unsigned short*)&buf[0], size/sizeof(short), 1);
		memcpy((unsigned char*)&(HDR10_EOTF_ByLuminace_TBL_cur.EOTF_setting[0]), (unsigned char*)&buf[0], size);
		Scaler_color_HDRIP_AutoRun(HDR_DM_HDR10); // re-run the HDR10 flow
		break;

	case TOOLACCESS_DM2_OETF_byLuminance_CTRL: /*1931*/
		if (size > (sizeof(unsigned int)*32))
			return -ID_SIZE_ERR;

		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/(sizeof(int)), 1);
		memcpy((unsigned int*)&(HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[0]), (unsigned int*)&buf[0], size);
		Scaler_color_HDRIP_AutoRun(HDR_DM_HDR10); // re-run the HDR10 flow
		break;

	case TOOLACCESS_DM2_panel_luminance: /*1932*/
		if (size > (sizeof(unsigned short)*1))
			return -ID_SIZE_ERR;

		fwif_color_ChangeUINT16Endian((unsigned short*)&buf[0], size/(sizeof(short)), 1);
		memcpy((unsigned short*)&panel_luminance, (unsigned short*)&buf[0], size);
		Scaler_color_Set_EOTF_OETF_Table(); // According to panel luminance to get current EOTF、OETF table
		Scaler_color_HDRIP_AutoRun(HDR_DM_HDR10); // re-run the HDR10 flow
		break;

	case TOOLACCESS_DM2_HDR_3D_TBL_FromReg:
		if (size > (sizeof(unsigned char)))
			return -ID_SIZE_ERR;

		HDR_3DLUT_getFromReg = buf[0];
		break;


	case TOOLACCESS_VIP_PQ_Power_Saving:
		ret = rtice_SetGet_PQ_Power_Saving(rtice_param, NULL, buf, system_setting_info, 1);
		if ( ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_10p_gamma_offset_gain:
		if (size > (sizeof(unsigned int)))
			return -ID_SIZE_ERR;

		memcpy(&pwCopyTemp, buf, size);
		fwif_color_ChangeUINT32Endian(&pwCopyTemp, 1, 1);
		fwif_color_set_APDEM_10p_Offset_gainVal(pwCopyTemp, 1);
		fwif_color_set_InvOutputGamma_System(Scaler_APDEM_Arg_Access(DEM_ARG_Gamma_idx_set, 0, 0));
		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_EN:
		if (size > (sizeof(unsigned char)))
			return -ID_SIZE_ERR;

		Scaler_APDEM_En_Access(buf[0], 1);
		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_TEST:
		//if (size > (sizeof(unsigned char)))
			//return -ID_SIZE_ERR;
		pwUINT32Temp = (unsigned int*)buf;
		fwif_color_ChangeUINT32Endian(pwUINT32Temp, size/sizeof(int), 1);
		Scaler_set_APDEM(rtice_param.mode0, (void*)pwUINT32Temp);

		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_DEM_PANEL_INI_TBL:
		if ((size + start_addr) > (sizeof(VIP_DEM_PANEL_INI_TBL)))
			return -ID_SIZE_ERR;

		pwUINT32Temp = ((unsigned int*)&VIP_AP_DEM_TBL.PANEL_DATA_) + start_addr;

		fwif_color_ChangeUINT32Endian((unsigned int*)&buf[0], size/sizeof(int), 1);

		memcpy(pwUINT32Temp, &buf[0], size);

		break;

	case TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_PTG_CTRL:
		if ((size + start_addr) > (sizeof(VIP_APDEM_PTG_CTRL)))
			return -ID_SIZE_ERR;

		pwIntTemp = ((int*)&Hal_APDEM_PTG_CTRL) + start_addr;

		fwif_color_ChangeUINT32Endian((int*)&buf[0], size/sizeof(int), 1);

		memcpy(pwIntTemp, &buf[0], size);

		if (rtice_APDEM_PTG_set(&Hal_APDEM_PTG_CTRL)<0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_DeMura_TBL_Mode:
		if(size > sizeof(char))
			return -ID_SIZE_ERR;
		fwif_color_set_DeMura_TBL_Mode(*buf);
		break;

	case TOOLACCESS_VIP_DeMura_TBL:
		if(rtice_SetGet_DeMura(&rtice_param, 0, buf, system_setting_info, 1) < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_DeMura_INI:
		if(size > sizeof(char))
			return -ID_SIZE_ERR;
		fwif_color_DeMura_init();
		break;

	case TOOLACCESS_VIP_DeMura_Adaptive:
		if(rtice_SetGet_DeMura(&rtice_param, 0, buf, system_setting_info, 1) < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_Force_vTop:
		if(rtice_SetGet_Froce_vTop_TBL(&rtice_param, 0, buf, system_setting_info, 1) < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_VIP_ICM_RANDOM_TABLE:
	{
		extern unsigned char drvif_color_icm_WriteRandomTable(void);
		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		drvif_color_icm_WriteRandomTable();
		break;
	}
	case TOOLACCESS_VIP_D3DLUT_DMA_DEBUG:
	{
		extern unsigned char lastLUT_tbl_index;
		extern void vpq_set_3d_lut_index(unsigned char idx, unsigned char bForceWrite);
		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;
		lastLUT_tbl_index = 0xff;
		vpq_set_3d_lut_index(1, 1); //force write

		break;
	}
	case TOOLACCESS_VIP_D3DLUT_RANDOM_TABLE:
	{
		extern void drvif_color_D_3dLUT_Random(void);
		if(size > sizeof(unsigned char))
			return -ID_SIZE_ERR;

		drvif_color_D_3dLUT_Random();
		break;
	}
	case TOOLACCESS_PQ_CHECK_ALL:
	{
//		vpq_PQ_setting_check();
		break;
	}
	case TOOLACCESS_Set_VIP_Disable_PQ :
		if(size > sizeof(char))
			return -ID_SIZE_ERR;

		Scaler_Set_VIP_Disable_PQ(VPQ_ModuleTestMode_PQByPass, *buf);

	break;

	case TOOLACCESS_TV002_STYLE:
		if (size > (sizeof(char)))
			return -ID_SIZE_ERR;
		#ifndef  CONFIG_CUSTOMER_TV010
		 Scaler_access_Project_TV002_Style(1, *buf);
		#else
		return -ID_SIZE_ERR;
		#endif
	break;

	case TOOLACCESS_BP_CTRL_TV002:
	case TOOLACCESS_BP_CTRL_TBL_TV002:
	case TOOLACCESS_BP_STATUS_TV002:
	case TOOLACCESS_BP_TBL_TV002:
		ret = rtice_SetGet_BP_TV002(rtice_param, NULL, buf, system_setting_info, 1);
		if (ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_DBC_STATUS_TV002:
	case TOOLACCESS_DBC_CTRL_TV002:
	case TOOLACCESS_DBC_SW_REG_TV002:
		ret = rtice_SetGet_DBC_TV002(rtice_param, NULL, buf, system_setting_info, 1);
		if (ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_Zero_D_DBC_STATUS_TV002:
	case TOOLACCESS_Zero_D_DBC_CTRL_TV002:
	case TOOLACCESS_Zero_D_DBC_LUT_TV002:
		ret = rtice_SetGet_Zero_D_DBC_TV002(rtice_param, NULL, buf, system_setting_info, 1);
		if (ret < 0)
			return -ID_SIZE_ERR;

		break;

	case TOOLACCESS_ID_Detect_result_TV002:
	{
		extern unsigned char ID_TV002_Flag[ID_ITEM_TV002_MAX];
		if (size > (sizeof(char)*ID_ITEM_TV002_MAX))
			return -ID_SIZE_ERR;

		memcpy(ID_TV002_Flag, buf, size);

		break;
	}
	case TOOLACCESS_ID_Detect_Apply_flag_TV002:
	{
		extern unsigned char ID_TV002_Apply;
		extern unsigned char ID_TV002_Detect;
		if (size > (sizeof(char)*2))
			return -ID_SIZE_ERR;
		ID_TV002_Detect = *buf;
		ID_TV002_Apply = *(buf + 1);

		break;
	}
	case  TOOLACCESS_Function_Extend3_DEBUG_LOG_ENABLE:
	{
               if (size > sizeof(gVipDebugLogCtrls) / sizeof(unsigned char))
                       return -ID_SIZE_ERR;
               memcpy(gVipDebugLogCtrls,buf,size);
	}
	break;
	case TOOLACCESS_Function_Extend3_DEBUG_LOG_PRINT_DELAY:
	{
		unsigned int i = 0,v = 0;
		if(size != 4)
			return -ID_SIZE_ERR;
		for(i = 0;i < 4;i++)
			v = (v << 8) + buf[i];
		gDebugPrintDelayTime = v;
	}
	break;

	case TOOLACCESS_RADCR_TBL:
	{
		extern short RADCR_Enable;
		extern RADCR_TBL_ST RADCR_table;
		extern RADCR_STATUS_ST RADCR_status;
		RADCR_RTICE_ST *pRADCR_rtice;
		if (start_addr*sizeof(short)+size > sizeof(*pRADCR_rtice))
			return -ID_SIZE_ERR;

		if (size >= sizeof(short)) {
			pRADCR_rtice = vmalloc(sizeof(*pRADCR_rtice));
			if (pRADCR_rtice != NULL) {
				short *p = (short *)pRADCR_rtice;
				pRADCR_rtice->table = RADCR_table;
				pRADCR_rtice->table.RADCR_Reserved10 = RADCR_Enable;
				pRADCR_rtice->status = RADCR_status;
				fwif_color_ChangeINT16Endian((short *)buf, size/sizeof(short), 1);
				memcpy(&p[start_addr], buf, size);
				RADCR_table = pRADCR_rtice->table;
				RADCR_Enable = RADCR_table.RADCR_Reserved10;
				vfree(pRADCR_rtice);
			}
		}
		break;
	}

	default:
		return -ID_SIZE_ERR;
		break;
	}
#endif
	return 0;
}

int rtice_set_IdxTable2Buff(unsigned short mode, unsigned short size, unsigned char *num_type, unsigned char *buf)
{
	unsigned short i, j, idx;
	unsigned char TypeTable[TOOLACCESS_MAX] = {0};
	unsigned int MaxNumTable[TOOLACCESS_MAX] = {0};
	/*unsigned char StringTable[TOOLACCESS_MAX][ID_String_Max] = {0};*/
	static unsigned char StringTable[TOOLACCESS_MAX*ID_String_Max] = {0};

	for (i = 0; i < TOOLACCESS_MAX; i++) {
		for (idx = 0; idx < TOOLACCESS_MAX; idx++)
			if (i == RTICE2AP_MemIdx_Table_ini[idx].ID)
				break;
		/* if No define in RTICE2AP_MemIdx_Table_ini, send 0 to tool*/
		if (idx == TOOLACCESS_MAX)
			continue;

		if (mode == TOOLACCESS_GetIdxTable_Type)
			TypeTable[i] = RTICE2AP_MemIdx_Table_ini[idx].Type;
		else if (mode == TOOLACCESS_GetIdxTable_MaxNum)
			MaxNumTable[i] = RTICE2AP_MemIdx_Table_ini[idx].MaxNum;
		else if (mode == TOOLACCESS_GetIdxTable_StringName) {
			for (j = 0; j < ID_String_Max; j++)
				/*StringTable[i][j] = RTICE2AP_MemIdx_Table_ini[idx].ID_String[j];*/
				StringTable[i*ID_String_Max+j] = RTICE2AP_MemIdx_Table_ini[idx].ID_String[j];
		} else
			return -ID_SIZE_ERR;
	}

	if (mode == TOOLACCESS_GetIdxTable_Type) {
		if (size > sizeof(TypeTable))
			return -ID_SIZE_ERR;
		memcpy(buf, (unsigned char *) &TypeTable[0], size);
		*num_type = VIP_UINT8;
	} else if (mode == TOOLACCESS_GetIdxTable_MaxNum) {
		if (size > sizeof(MaxNumTable))
			return -ID_SIZE_ERR;
		fwif_color_ChangeUINT32Endian_Copy(MaxNumTable, TOOLACCESS_MAX, MaxNumTable, 1);
		memcpy(buf, (unsigned char *) &MaxNumTable[0], size);
		*num_type = VIP_UINT32;
	} else if (mode == TOOLACCESS_GetIdxTable_StringName) {
		if (size > sizeof(StringTable))
			return -ID_SIZE_ERR;
		/*memcpy(buf, (unsigned char *) &StringTable[0][0], size);*/
		memcpy(buf, (unsigned char *) &StringTable[0], size);
		*num_type = VIP_UINT8;
	} else
		return -ID_SIZE_ERR;

	return 0;
}

int rtice_SetGet_PQA_Table_Info(VIP_TOOL_ACCESS_INFO_Item mode, unsigned char isSet_Flag, unsigned short size, unsigned char *num_type, unsigned char *buf)
{
	unsigned int table_row = 0, table_column = 0;
	unsigned int tmp_ar[7];
	/*unsigned int end_row[PQA_LEVEL_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xffffffff, 0, 0,};*/
	unsigned char src_idx = 0;
	unsigned char display = 0;
	unsigned char flowCtrl_Iitem[2][PQA_INPUT_ITEM_MAX];
	unsigned char flowCtrl_Itype[2][PQA_INPUT_TYPE_MAX];
	unsigned int endReg;
	unsigned int ((*PQA_table)[PQA_LEVEL_MAX]) = NULL;
	unsigned int ((*PQA_input_table)[PQA_I_LEVEL_MAX]) = NULL;
	unsigned char which_table = 0, i = 0;
	_clues *smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	StructColorDataType *pTable = NULL;
	SLR_VIP_TABLE *shareMem_VIP_Table = NULL;

	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	shareMem_VIP_Table = fwif_color_GetShare_Memory_VIP_TABLE_Struct();

	if (pTable == NULL || shareMem_VIP_Table == NULL)
		return -ID_SIZE_ERR;

	which_table = pTable->PQA_Table;
	if (which_table >= PQA_TABLE_MAX)
		which_table = 0;

	if (shareMem_VIP_Table->PQA_Table[which_table][PQA_MODE_WRITE][0][PQA_reg] == 0xffffffff)
		/*PQA_table = &shareMem_VIP_Table->PQA_Table[which_table][PQA_MODE_OFFSET][0][0];*/
		PQA_table = shareMem_VIP_Table->PQA_Table[which_table][PQA_MODE_OFFSET];
	else
		/*PQA_table = &shareMem_VIP_Table->PQA_Table[which_table][PQA_MODE_WRITE][0][0];*/
		PQA_table = shareMem_VIP_Table->PQA_Table[which_table][PQA_MODE_WRITE];

	PQA_input_table = shareMem_VIP_Table->PQA_Input_Table[which_table];

	if (mode == TOOLACCESS_PQA_Table_Size) {
		if (isSet_Flag == 1) {
			/*set function is no needed*/
			memcpy((unsigned char *) &tmp_ar[0], buf, sizeof(int));
			table_column = fwif_color_ChangeOneUINT32Endian(tmp_ar[0], 1);
			memcpy((unsigned char *) &tmp_ar[1], (buf+4), sizeof(int));
			table_row = fwif_color_ChangeOneUINT32Endian(tmp_ar[1], 1);
		} else {
			/* only 2 element(row and column number)*/
			if (size > 7*sizeof(int))
				return -ID_SIZE_ERR;
			table_row = 0;
			/* count row number*/
			for (table_row = 0; table_row < PQA_ITEM_MAX; table_row++) {
				/*if (*(PQA_table+(table_row*PQA_LEVEL_MAX)+PQA_reg) == 0xffffffff)*/
				if (PQA_table[table_row][PQA_reg] == 0xffffffff) {
					table_row++;
					break;

				}
			}
			table_column = PQA_LEVEL_MAX;
			tmp_ar[0] = fwif_color_ChangeOneUINT32Endian(table_column, 1);
			tmp_ar[1] = fwif_color_ChangeOneUINT32Endian(table_row, 1);
			tmp_ar[2] = fwif_color_ChangeOneUINT32Endian(PQA_INPUT_ITEM_MAX, 1);
			tmp_ar[3] = fwif_color_ChangeOneUINT32Endian(PQA_Level_Idx_MAX, 1);
			tmp_ar[4] = fwif_color_ChangeOneUINT32Endian(PQA_I_LEVEL_MAX, 1);
			tmp_ar[5] = fwif_color_ChangeOneUINT32Endian(PQA_I_ITEM_MAX, 1);
			tmp_ar[6] = fwif_color_ChangeOneUINT32Endian(PQA_INPUT_TYPE_MAX, 1);
			memcpy(buf, (unsigned char *) &tmp_ar[0], size);	/* size is byte from tool.*/
			*(num_type) = VIP_UINT32;
		}
	} else if (mode == TOOLACCESS_PQA_Table) {
		if (isSet_Flag == 1) {
			if (size%(sizeof(int)*PQA_LEVEL_MAX) != 0)
				return -ID_SIZE_ERR;
			/* end row reg must be 0xffffffff.*/
			endReg = (*(buf+size-12))+((*(buf+size-11)) << 8)+((*(buf+size-10)) << 16)+((*(buf+size-9)) << 24);
			if (endReg != 0xffffffff)
				return -ID_SIZE_ERR;
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(int), 1);

#ifdef CONFIG_ARM64
			memcpy_toio((unsigned char *)PQA_table, buf, size);
#else
			memcpy((unsigned char *)PQA_table, buf, size);
#endif

			/* re-check PQA input items*/
			if (shareMem_VIP_Table->PQA_Table[which_table][PQA_MODE_WRITE][0][PQA_reg] == 0xffffffff)
				fwif_color_PQA_Input_Item_Check(system_setting_info, &shareMem_VIP_Table->PQA_Table[0][PQA_MODE_OFFSET][0][0], which_table);
			else
				fwif_color_PQA_Input_Item_Check(system_setting_info, &shareMem_VIP_Table->PQA_Table[0][PQA_MODE_WRITE][0][0], which_table);

		} else {
			for (table_row = 0; table_row < PQA_ITEM_MAX; table_row++) {
				/*if (*(PQA_table+(table_row*PQA_LEVEL_MAX)+PQA_reg) == 0xffffffff)*/
				if (PQA_table[table_row][PQA_reg] == 0xffffffff) {
					table_row++;
					break;
				}
			}
			table_column = PQA_LEVEL_MAX;
			/*get size must be smaller than table size in share memory*/
			if (size > table_column*table_row*sizeof(int))
				return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
			memcpy_fromio(buf, (unsigned char *)PQA_table, size);
#else
			memcpy(buf, (unsigned char *)PQA_table, size);
#endif
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(int), 1);

			*(num_type) = VIP_UINT32;
		}
	} else if (mode == TOOLACCESS_PQA_Level_Table) {
		/* no set mode*/
		if (isSet_Flag == 1) {
			return -ID_SIZE_ERR;
		} else {
			if (size > PQA_INPUT_ITEM_MAX*sizeof(short))
				return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
			memcpy_fromio(buf, (unsigned char *) &smartPic_clue->PQA_ISR_Cal_Info.Input_Level[0], size);
#else
			memcpy(buf, (unsigned char *) &smartPic_clue->PQA_ISR_Cal_Info.Input_Level[0], size);
#endif
			fwif_color_ChangeUINT16Endian((unsigned short *)buf, size/sizeof(short), 1);

			*(num_type) = VIP_UINT16;
		}
	} else if (mode == TOOLACCESS_PQA_Level_Index_Table) {
		/* no set mode*/
		if (isSet_Flag == 1) {
			return -ID_SIZE_ERR;
		} else {
			if (size > PQA_Level_Idx_MAX*sizeof(int))
				return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
			memcpy_fromio(buf, (unsigned char *) &smartPic_clue->PQA_ISR_Cal_Info.Level_Index[0], size);
#else
			memcpy(buf, (unsigned char *) &smartPic_clue->PQA_ISR_Cal_Info.Level_Index[0], size);
#endif
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(int), 1);

			*(num_type) = VIP_UINT32;
		}
	} else if (mode == TOOLACCESS_PQA_Input_Table) {
		if (isSet_Flag == 1) {
			if (size%(sizeof(int)*PQA_I_LEVEL_MAX) != 0)
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(int), 1);
#ifdef CONFIG_ARM64
			memcpy_toio((unsigned char *)PQA_input_table, buf, size);
#else
			memcpy((unsigned char *)PQA_input_table, buf, size);
#endif
		} else {
			/*get size must be smaller than table size in share memory*/
			if (size > PQA_I_ITEM_MAX*PQA_I_LEVEL_MAX*sizeof(int))
				return -ID_SIZE_ERR;
#ifdef CONFIG_ARM64
			memcpy_fromio(buf, (unsigned char *)PQA_input_table, size);
#else
			memcpy(buf, (unsigned char *)PQA_input_table, size);
#endif
			fwif_color_ChangeUINT32Endian((unsigned int *)buf, size/sizeof(int), 1);

			*(num_type) = VIP_UINT32;
		}
	} else if (mode == TOOLACCESS_PQA_FlowCtrl_Input_Type) {
		if (isSet_Flag == 1) {
			if (size > sizeof(flowCtrl_Itype)) /*input item table and flag table*/
				return -ID_SIZE_ERR;
			memcpy((unsigned char*)&(flowCtrl_Itype[0][0]), buf,  size);
#ifdef CONFIG_ARM64
			memcpy_toio((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[0]), (unsigned char*)&(flowCtrl_Itype[0][0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy_toio((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0]), (unsigned char*)&(flowCtrl_Itype[1][0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy_toio((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
#else
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[0]), (unsigned char*)&(flowCtrl_Itype[0][0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0]), (unsigned char*)&(flowCtrl_Itype[1][0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
#endif
		} else {
			if (size > sizeof(flowCtrl_Itype)) /*input item table and flag table*/
				return -ID_SIZE_ERR;
			for (i = 0; i < PQA_INPUT_TYPE_MAX; i++)
				if (system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[i] == 0)	/* read Flow_Ctrl_Input_Type[i], while set flag is on, avoid reading error, elieli*/
					system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[i] = system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[i];
#ifdef CONFIG_ARM64
			/*memcpy_fromio((unsigned char *) &system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0], (unsigned char *) &system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[0], sizeof(char)*PQA_INPUT_TYPE_MAX);*/
			memcpy_fromio((unsigned char*)&(flowCtrl_Itype[0][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy_fromio((unsigned char*)&(flowCtrl_Itype[1][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
#else
			/*memcpy((unsigned char *) &system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0], (unsigned char *) &system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[0], sizeof(char)*PQA_INPUT_TYPE_MAX);*/
			memcpy((unsigned char*)&(flowCtrl_Itype[0][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
			memcpy((unsigned char*)&(flowCtrl_Itype[1][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type[0]), sizeof(char)*PQA_INPUT_TYPE_MAX);
#endif
			memcpy(buf, (unsigned char *) &(flowCtrl_Itype[0][0]),  size);

			*(num_type) = VIP_UINT8;
		}
	} else if (mode == TOOLACCESS_PQA_FlowCtrl_Input_Item) {
		if (isSet_Flag == 1) {
			if (size > sizeof(flowCtrl_Iitem)) /*input item table and flag table*/
				return -ID_SIZE_ERR;
			memcpy((unsigned char*)&(flowCtrl_Iitem[0][0]), buf,  size);
#ifdef CONFIG_ARM64
			memcpy_fromio((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[0]), (unsigned char*)&(flowCtrl_Iitem[0][0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
			memcpy_fromio((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[0]), (unsigned char*)&(flowCtrl_Iitem[1][0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
#else
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[0]), (unsigned char*)&(flowCtrl_Iitem[0][0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
			memcpy((unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[0]), (unsigned char*)&(flowCtrl_Iitem[1][0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
#endif

			for (i = 0; i < PQA_INPUT_ITEM_MAX; i++)
				smartPic_clue->PQA_ISR_Cal_Info.Input_Level[i] = (UINT16)system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[i];
		} else {
			if (size > sizeof(flowCtrl_Iitem)) /*input item table and flag table*/
				return -ID_SIZE_ERR;

			for (i = 0; i < PQA_INPUT_ITEM_MAX; i++)
				if (system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[i] == 0)	/* read Flow_Ctrl_Input_Item[i], while set flag is on, avoid reading error, elieli*/
					system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[i] = (UINT16)smartPic_clue->PQA_ISR_Cal_Info.Input_Level[i];
#ifdef CONFIG_ARM64
			memcpy_fromio((unsigned char*)&(flowCtrl_Iitem[0][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
			memcpy_fromio((unsigned char*)&(flowCtrl_Iitem[1][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
#else
			memcpy((unsigned char*)&(flowCtrl_Iitem[0][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
			memcpy((unsigned char*)&(flowCtrl_Iitem[1][0]), (unsigned char*)&(system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item[0]), sizeof(char)*PQA_INPUT_ITEM_MAX);
#endif
			memcpy(buf, (unsigned char *) &(flowCtrl_Iitem[0][0]), size);
			*(num_type) = VIP_UINT8;
		}
	} else if (mode == TOOLACCESS_PQA_SPM_Info_Get) {
               /* no set mode*/
               if (isSet_Flag == 1) {
                       return -ID_SIZE_ERR;
               } else {
			if (size > 13*sizeof(char))
				return -ID_SIZE_ERR;
			buf[0] = I_DNR;
			buf[1] = I_SPM_LV_CTRL0;
			buf[2] = I_MAD_Hist_Th;
			buf[3] = I_SPM_VD_status;
			buf[4] = I_SPM_MAD_Noise_Index;
			buf[5] = I_SPM_MAD_Still_Index;
			buf[6] = I_SPM_FMV_Still_Index;
			buf[7] = I_SPM_Hist_Mean_Y;
			buf[8] = PQA_SPM_LvIdx_VD;
			buf[9] = PQA_SPM_LvIdx_MAD_Noise;
			buf[10] = PQA_SPM_LvIdx_MAD_Still;
			buf[11] = PQA_SPM_LvIdx_FMV_Still;
			buf[12] = PQA_SPM_LvIdx_Y;

			*(num_type) = VIP_UINT8;
               }
	} else
		return -ID_SIZE_ERR;

	return 0;
}


int rtice_SetGet_YUV2RGB_Table_Info(VIP_TOOL_ACCESS_INFO_Item mode, unsigned char isSet_Flag, unsigned short size,
	unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table)	/*return idx table size*/
{
	unsigned short gain;
	if (mode == TOOLACCESS_YUV2RGB_Global_Sat_Gain) {
		if (isSet_Flag == 1) {
			if (size > sizeof(unsigned short)*1)
				return -ID_SIZE_ERR;
			memcpy(&gain, buf, size);
			rtice_Set_YUV2RGB_Sat(fwif_color_ChangeOneUINT16Endian(gain, 1), system_setting_info, gVip_Table);

		} else {
			if (size > sizeof(unsigned short)*1)
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian_Copy((unsigned short *)&system_setting_info->OSD_Info.Saturation_Gain, 1, (unsigned short *) buf, 1);
			*(num_type) = VIP_UINT16;
		}
	} else if (mode == TOOLACCESS_YUV2RGB_Global_Hue_Gain) {
		if (isSet_Flag == 1) {
			if (size > sizeof(unsigned short)*1)
				return -ID_SIZE_ERR;
			memcpy(&gain, buf, size);
			rtice_Set_YUV2RGB_Hue(fwif_color_ChangeOneUINT16Endian(gain, 1), system_setting_info, gVip_Table);
		} else {
			if (size > sizeof(unsigned short)*1)
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian_Copy((unsigned short *)&system_setting_info->OSD_Info.Hue_Gain, 1, (unsigned short *)buf, 1);
			*(num_type) = VIP_UINT16;
		}
	} else if (mode == TOOLACCESS_YUV2RGB_Global_Contrast_Gain) {
		if (isSet_Flag == 1) {
			if (size > sizeof(unsigned short)*1)
				return -ID_SIZE_ERR;
			memcpy(&gain, buf, size);
			rtice_Set_YUV2RGB_Contrast(fwif_color_ChangeOneUINT16Endian(gain, 1), system_setting_info, gVip_Table);
		} else {
			if (size > sizeof(unsigned short)*1)
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian_Copy((unsigned short *)&system_setting_info->OSD_Info.Contrast_Gain, 1,(unsigned short *) buf, 1);
			*(num_type) = VIP_UINT16;
		}
	} else if (mode == TOOLACCESS_YUV2RGB_Global_Brightness_Gain) {
		if (isSet_Flag == 1) {
			if (size > sizeof(unsigned short)*1)
				return -ID_SIZE_ERR;
			memcpy(&gain, buf, size);
			rtice_Set_YUV2RGB_Bri(fwif_color_ChangeOneUINT16Endian(gain, 1), system_setting_info, gVip_Table);
		} else {
			if (size > sizeof(unsigned short)*1)
				return -ID_SIZE_ERR;

			fwif_color_ChangeUINT16Endian_Copy((unsigned short *)&system_setting_info->OSD_Info.Brightness_Gain, 1, (unsigned short *)buf, 1);
			*(num_type) = VIP_UINT16;
		}

	} else
		return -ID_SIZE_ERR;
	return 0;
}

int rtice_Set_YUV2RGB_Sat(unsigned short satGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table)
{
	system_setting_info->OSD_Info.Saturation_Gain = fwif_color_ChangeOneUINT16Endian(satGain, 0);

	if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV006)
		fwif_color_setCon_Bri_Color_Tint_tv006(SLR_MAIN_DISPLAY, system_setting_info);	/*only main ch for tool now*/
	else
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(0, SLR_MAIN_DISPLAY, system_setting_info);
	return 0;

#if 0
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	short satTmp[3][3] = {{0}}, hueTmp[3][3] = {{0}}, kTmp[3][3] = {{0}}, kTmp1[3][3] = {{0}};
	unsigned char i, j, k , y_idx_max;
	/*unsigned short *table_idx[3][3];*/
	unsigned short table_idx[3][3][VIP_YUV2RGB_Y_Seg_Max];
	unsigned char level = system_setting_info->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	unsigned char tbl_select = system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;
	unsigned short hueGain;
	unsigned char Input_mode = system_setting_info->using_YUV2RGB_Matrix_Info.Input_Data_Mode;

	if (level >= VIP_YUV2RGB_LEVEL_SELECT_MAX || tbl_select >= YUV2RGB_TBL_Num) {
		printf("~level or table select Error return, level = %d, table select = %d, %s->%d, %s~\n", level, tbl_select, __FILE__, __LINE__, __FUNCTION__);
		return -ID_SIZE_ERR;
	}
	y_idx_max = VIP_YUV2RGB_Y_Seg_Max;

	/*sat*/
	system_setting_info->OSD_Info.Saturation_Gain = fwif_color_ChangeOneUINT16Endian(satGain, 1);
	/* restrict for sat, refernece to spec for k32.*/
	satGain = (satGain > 128) ? 75 + (satGain * 53 / 128) : satGain;
	satTmp[0][0] = 128; /* 128 = 1*/
	satTmp[1][1] = satGain;
	satTmp[2][2] = satGain;

	/*get hue info*/
	hueGain = fwif_color_ChangeOneUINT16Endian(system_setting_info->OSD_Info.Hue_Gain, 1);
	/* Euler's formula*/
	hueTmp[0][0] = 1024;
	hueTmp[1][1] = CAdjustCosine(hueGain);
	hueTmp[1][2] = CAdjustSine(hueGain)*(-1);
	hueTmp[2][1] = CAdjustSine(hueGain);
	hueTmp[2][2] = CAdjustCosine(hueGain);

	/* get vip table info for k coeff.*/
	/* get table from pq misc, elieli*//*
	table_idx[0][0] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K11[0]);
	table_idx[0][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K12[0]);
	table_idx[0][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K13[0]);
	table_idx[1][0] = table_idx[0][0];
	table_idx[1][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K22[0]);
	table_idx[1][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K23[0]);
	table_idx[2][0] = table_idx[0][0];
	table_idx[2][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K32[0]);
	table_idx[2][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K33[0]);
	*/
	for (i = 0; i < y_idx_max; i++) {
		table_idx[0][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
		table_idx[0][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K12];
		table_idx[0][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13];
		table_idx[1][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
		table_idx[1][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K22];
		table_idx[1][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K23];
		table_idx[2][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
		table_idx[2][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32];
		table_idx[2][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K33];
	}

	for (i = 0; i < y_idx_max; i++) {
		for (j = 0; j < 3; j++) {
			for (k = 0; k < 3; k++) {
				kTmp[j][k] = table_idx[j][k][i];
				/* bigger than 0x3FF =>neg*/
				if (kTmp[j][k] > 0x3FF) {
					kTmp[j][k] = Rtice_complement2_Y2R(kTmp[j][k]);
					kTmp[j][k] = kTmp[j][k] * (-1);
				}
			}
		}

		/* 3x3 matrix multiply*/
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k]) >> 10;

		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k]) >> 7;

		drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
		drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
		drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
		drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
		drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
		drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];

		/*	set reg value to share memory.*/
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i] = drv_vipCSMatrix_t.COEF_By_Y.K12[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i] = drv_vipCSMatrix_t.COEF_By_Y.K13[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i] = drv_vipCSMatrix_t.COEF_By_Y.K22[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i] = drv_vipCSMatrix_t.COEF_By_Y.K23[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i] = drv_vipCSMatrix_t.COEF_By_Y.K32[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i] = drv_vipCSMatrix_t.COEF_By_Y.K33[i];
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i]) , 1, 1);
	}
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, 0, VIP_Table_Select_1);
	return 0;
#endif
}

int rtice_Set_YUV2RGB_Hue(unsigned short hueGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table)
{
	system_setting_info->OSD_Info.Hue_Gain = fwif_color_ChangeOneUINT16Endian(hueGain, 0);

	if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV006)
		fwif_color_setCon_Bri_Color_Tint_tv006(SLR_MAIN_DISPLAY, system_setting_info);	/*only main ch for tool now*/
	else
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(0, SLR_MAIN_DISPLAY, system_setting_info);
	return 0;

#if 0
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	short satTmp[3][3] = {{0}}, hueTmp[3][3] = {{0}}, kTmp[3][3] = {{0}}, kTmp1[3][3] = {{0}};
	unsigned char i, j, k , y_idx_max;
	/*unsigned short *table_idx[3][3];*/
	unsigned short table_idx[3][3][VIP_YUV2RGB_Y_Seg_Max];
	unsigned char level = system_setting_info->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	unsigned char tbl_select = system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;
	unsigned short satGain;
	unsigned char Input_mode = system_setting_info->using_YUV2RGB_Matrix_Info.Input_Data_Mode;

	if (level >= VIP_YUV2RGB_LEVEL_SELECT_MAX || tbl_select >= YUV2RGB_TBL_Num) {
		printf("~level or table select Error return, level = %d, table select = %d, %s->%d, %s~\n", level, tbl_select, __FILE__, __LINE__, __FUNCTION__);
		return -ID_SIZE_ERR;
	}
	y_idx_max = VIP_YUV2RGB_Y_Seg_Max;

	/*sat*/
	satGain = fwif_color_ChangeOneUINT16Endian(system_setting_info->OSD_Info.Saturation_Gain, 1);
	/* restrict for sat, refernece to spec for k32.*/
	satGain = (satGain > 128) ? 75 + (satGain * 53 / 128) : satGain;
	satTmp[0][0] = 128; /* 128 = 1*/
	satTmp[1][1] = satGain;
	satTmp[2][2] = satGain;

	/*hue*/
	system_setting_info->OSD_Info.Hue_Gain = fwif_color_ChangeOneUINT16Endian(hueGain, 1);
	/* Euler's formula*/
	hueTmp[0][0] = 1024;
	hueTmp[1][1] = CAdjustCosine(hueGain);
	hueTmp[1][2] = CAdjustSine(hueGain)*(-1);
	hueTmp[2][1] = CAdjustSine(hueGain);
	hueTmp[2][2] = CAdjustCosine(hueGain);

	/* get vip table info for k coeff.*/
	/* get table from pq misc, elieli*//*
	table_idx[0][0] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K11[0]);
	table_idx[0][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K12[0]);
	table_idx[0][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K13[0]);
	table_idx[1][0] = table_idx[0][0];
	table_idx[1][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K22[0]);
	table_idx[1][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K23[0]);
	table_idx[2][0] = table_idx[0][0];
	table_idx[2][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K32[0]);
	table_idx[2][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K33[0]);
	*/
	for (i = 0; i < y_idx_max; i++) {
		table_idx[0][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
		table_idx[0][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K12];
		table_idx[0][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13];
		table_idx[1][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
		table_idx[1][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K22];
		table_idx[1][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K23];
		table_idx[2][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
		table_idx[2][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32];
		table_idx[2][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K33];
	}

	for (i = 0; i < y_idx_max; i++) {
		for (j = 0; j < 3; j++) {
			for (k = 0; k < 3; k++) {
				kTmp[j][k] = table_idx[j][k][i];
				/* bigger than 0x3FF =>neg*/
				if (kTmp[j][k] > 0x3FF) {
					kTmp[j][k] = Rtice_complement2_Y2R(kTmp[j][k]);
					kTmp[j][k] = kTmp[j][k] * (-1);
				}
			}
		}

		/* 3x3 matrix multiply*/
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k]) >> 10;

		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k]) >> 7;

		#if 0
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				/*neg*/
				if (kTmp[j][k] < 0)
					kTmp[j][k] = complement2(kTmp[j][k]);
		#endif

		drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
		drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
		drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
		drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
		drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
		drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];

		/*	set reg value to share memory.*/
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i] = drv_vipCSMatrix_t.COEF_By_Y.K12[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i] = drv_vipCSMatrix_t.COEF_By_Y.K13[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i] = drv_vipCSMatrix_t.COEF_By_Y.K22[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i] = drv_vipCSMatrix_t.COEF_By_Y.K23[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i] = drv_vipCSMatrix_t.COEF_By_Y.K32[i];
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i] = drv_vipCSMatrix_t.COEF_By_Y.K33[i];
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i]) , 1, 1);
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i]) , 1, 1);
	}
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, 0, VIP_Table_Select_1);
	return 0;
#endif
}

int rtice_Set_YUV2RGB_Contrast(unsigned short ContrastGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table)
{
	system_setting_info->OSD_Info.Contrast_Gain = fwif_color_ChangeOneUINT16Endian(ContrastGain, 0);

	if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV006)
		fwif_color_setCon_Bri_Color_Tint_tv006(SLR_MAIN_DISPLAY, system_setting_info);	/*only main ch for tool now*/
	else
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(0, SLR_MAIN_DISPLAY, system_setting_info);
	return 0;

#if 0
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	unsigned char i, y_idx_max;
	/*unsigned short *table_idx;*/
	unsigned short table_idx[VIP_YUV2RGB_Y_Seg_Max];
	unsigned char level = system_setting_info->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	unsigned char tbl_select = system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;
	unsigned char Input_mode = system_setting_info->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	/*unsigned short satGain;*/

	if (level >= VIP_YUV2RGB_LEVEL_SELECT_MAX || tbl_select >= YUV2RGB_TBL_Num) {
		printf("~level or table select Error return, level = %d, table select = %d, %s->%d, %s~\n", level, tbl_select, __FILE__, __LINE__, __FUNCTION__);
		return -ID_SIZE_ERR;
	}
	y_idx_max = VIP_YUV2RGB_Y_Seg_Max;
	/*table_idx = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K11[0]);*/ /* get table from pq misc. elieli*/
	for (i = 0; i < y_idx_max; i++)
		table_idx[i] =  tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];

	system_setting_info->OSD_Info.Contrast_Gain = fwif_color_ChangeOneUINT16Endian(ContrastGain, 1);

	/*contrast is a mapping gain.*/
	/*drv_vipCSMatrix_t.K11[VIP_CSMatrix_Index_0] = (2 * contrast) & 0x1ff;	*/ /*20140219 roger, back the coef first*/
	for (i = 0; i < y_idx_max; i++) {
		drv_vipCSMatrix_t.COEF_By_Y.K11[i] = (table_idx[i] * ContrastGain) >> 7;
		/*	set reg value to share memory.*/
		system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[i] = drv_vipCSMatrix_t.COEF_By_Y.K11[i];
		fwif_color_ChangeUINT16Endian(&(system_setting_info->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[i]) , 1, 1);
	}
	/*o------------- contrast ------------o*/

	/*drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp);*/
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, 0, VIP_Table_Select_1);
	return 0;
#endif
}

int rtice_Set_YUV2RGB_Bri(unsigned short BriGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table)
{
	system_setting_info->OSD_Info.Brightness_Gain = fwif_color_ChangeOneUINT16Endian(BriGain, 0);

	if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV006)
		fwif_color_setCon_Bri_Color_Tint_tv006(SLR_MAIN_DISPLAY, system_setting_info);	/*only main ch for tool now*/
	else
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(0, SLR_MAIN_DISPLAY, system_setting_info);
	return 0;
}

void rtice_Covert_ICM_Gain_Struct(CHIP_COLOR_ICM_GAIN_ADJUST_T *pDB, CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T *pRTICE, unsigned char flag)
{
	int rgn, pt;//, delta;

	if (0 == flag) { /*DB to RTICE*/
		for (rgn = 0; rgn < CHIP_CM_COLOR_REGION_MAX; rgn++) {
			pRTICE->stRgn.stColorRegion[rgn].enable = pDB->stRgn.stColorRegion[rgn].enable;
			pRTICE->stRgn.stColorRegion[rgn].regionNum = pDB->stRgn.stColorRegion[rgn].regionNum;
			for (pt = 0; pt < CHIP_CM_TBLPOINT; pt++) {
				pRTICE->stRgn.stColorRegion[rgn].hue_x[pt] = pDB->stRgn.stColorRegion[rgn].hue_x[pt];
				pRTICE->stRgn.stColorRegion[rgn].hue_g[pt] = pDB->stRgn.stColorRegion[rgn].hue_g[pt];
				pRTICE->stRgn.stColorRegion[rgn].sat_x[pt] = pDB->stRgn.stColorRegion[rgn].sat_x[pt];
				pRTICE->stRgn.stColorRegion[rgn].sat_g[pt] = pDB->stRgn.stColorRegion[rgn].sat_g[pt];
				pRTICE->stRgn.stColorRegion[rgn].Int_x[pt] = pDB->stRgn.stColorRegion[rgn].Int_x[pt];
				pRTICE->stRgn.stColorRegion[rgn].Int_g[pt] = pDB->stRgn.stColorRegion[rgn].Int_g[pt];
			}

			pRTICE->stCtrl.stColor[rgn].masterGain = pDB->stCtrl.stColor[rgn].masterGain;
			pRTICE->stCtrl.stColor[rgn].stColorGain.stColorGainHue = pDB->stCtrl.stColor[rgn].stColorGain.stColorGainHue;
			pRTICE->stCtrl.stColor[rgn].stColorGain.stColorGainSat = pDB->stCtrl.stColor[rgn].stColorGain.stColorGainSat;
			pRTICE->stCtrl.stColor[rgn].stColorGain.stColorGainInt = pDB->stCtrl.stColor[rgn].stColorGain.stColorGainInt;
		}
	} else { /*RTICE to DB*/
		for (rgn = 0; rgn < CHIP_CM_COLOR_REGION_MAX; rgn++) {
			pDB->stRgn.stColorRegion[rgn].enable = pRTICE->stRgn.stColorRegion[rgn].enable;
			pDB->stRgn.stColorRegion[rgn].regionNum = pRTICE->stRgn.stColorRegion[rgn].regionNum;
			for (pt = 0; pt < CHIP_CM_TBLPOINT; pt++) {
				pDB->stRgn.stColorRegion[rgn].hue_x[pt] = pRTICE->stRgn.stColorRegion[rgn].hue_x[pt];
				pDB->stRgn.stColorRegion[rgn].hue_g[pt] = pRTICE->stRgn.stColorRegion[rgn].hue_g[pt];
				pDB->stRgn.stColorRegion[rgn].sat_x[pt] = pRTICE->stRgn.stColorRegion[rgn].sat_x[pt];
				pDB->stRgn.stColorRegion[rgn].sat_g[pt] = pRTICE->stRgn.stColorRegion[rgn].sat_g[pt];
				pDB->stRgn.stColorRegion[rgn].Int_x[pt] = pRTICE->stRgn.stColorRegion[rgn].Int_x[pt];
				pDB->stRgn.stColorRegion[rgn].Int_g[pt] = pRTICE->stRgn.stColorRegion[rgn].Int_g[pt];
			}

			pDB->stCtrl.stColor[rgn].masterGain = pRTICE->stCtrl.stColor[rgn].masterGain;
			pDB->stCtrl.stColor[rgn].stColorGain.stColorGainHue = pRTICE->stCtrl.stColor[rgn].stColorGain.stColorGainHue;
			pDB->stCtrl.stColor[rgn].stColorGain.stColorGainSat = pRTICE->stCtrl.stColor[rgn].stColorGain.stColorGainSat;
			pDB->stCtrl.stColor[rgn].stColorGain.stColorGainInt = pRTICE->stCtrl.stColor[rgn].stColorGain.stColorGainInt;
		}
	}
}

int rtice_Get_VIP_System_Info(_system_setting_info *system_setting_info, _RPC_system_setting_info *RPC_system_setting_info, SLR_VIP_TABLE *gVip_Table, unsigned int* pVIPSysInfo)
{
	unsigned char i;

	if (system_setting_info == NULL || RPC_system_setting_info == NULL) {
		for (i=0;i<VIP_System_Info_Items_Num;i++)
			pVIPSysInfo[i] = 0;
	} else {
		pVIPSysInfo[0] = system_setting_info->input_display;
		pVIPSysInfo[1] = system_setting_info->Input_src_Type;
		pVIPSysInfo[2] = system_setting_info->Input_src_Form;
		pVIPSysInfo[3] = system_setting_info->Input_VO_Form;
		pVIPSysInfo[4] = system_setting_info->Input_TVD_Form;
		pVIPSysInfo[5] = system_setting_info->vdc_color_standard;
		pVIPSysInfo[6] = system_setting_info->Timing;
		pVIPSysInfo[7] = RPC_system_setting_info->VIP_source;
		pVIPSysInfo[8] = system_setting_info->VIP_3D_source;
		pVIPSysInfo[9] = system_setting_info->NowSourceOption;
		pVIPSysInfo[10] = system_setting_info->Display_RATIO_TYPE;
		pVIPSysInfo[11] = system_setting_info->color_fac_src_idx;

		pVIPSysInfo[12] = system_setting_info->HDMI_video_format;
		pVIPSysInfo[13] = system_setting_info->HDMI_data_range;
		pVIPSysInfo[14] = system_setting_info->switch_DVI_HDMI;
		pVIPSysInfo[15] = system_setting_info->HDMI_color_space;

		/*pVIPSysInfo[16] = system_setting_info->VO_bit_rate;*/ /*no this item in K3L*/

		pVIPSysInfo[17] = RPC_system_setting_info->Project_ID;
		pVIPSysInfo[18] = system_setting_info->RTK_DCR_Enable;
		pVIPSysInfo[19] = system_setting_info->YC_separation_status;
		pVIPSysInfo[20] = system_setting_info->DCC_FW_ONOFF;
		pVIPSysInfo[21] = RPC_system_setting_info->DCC_hist_shift_bit;
		pVIPSysInfo[22] = system_setting_info->DCC_Status_Change;
		/*pVIPSysInfo[23] = system_setting_info->is_lock;*/ /*no this item in K3L*/

		pVIPSysInfo[24] = system_setting_info->VIP_3D_display_flag;
		pVIPSysInfo[25] = RPC_system_setting_info->PQ_demo_flag;
		pVIPSysInfo[26] = system_setting_info->xvYcc_auto_mode_flag;
		pVIPSysInfo[27] = system_setting_info->xvYcc_flag;
		pVIPSysInfo[28] = system_setting_info->HDR_flag;
		/*pVIPSysInfo[29] = system_setting_info->HDR10_Ready_3dLUT;*/ /*no this item in K3L*/
		pVIPSysInfo[30] = system_setting_info->DolbyHDR_flag;

		pVIPSysInfo[31] = system_setting_info->IV_Start;
		pVIPSysInfo[32] = system_setting_info->IH_Start;
		pVIPSysInfo[33] = system_setting_info->I_Width;
		pVIPSysInfo[34] = system_setting_info->I_Height;
		pVIPSysInfo[35] = system_setting_info->Mem_Width;
		pVIPSysInfo[36] = system_setting_info->Mem_Height;
		pVIPSysInfo[37] = system_setting_info->Cap_Width;
		pVIPSysInfo[38] = system_setting_info->Cap_Height;
		pVIPSysInfo[39] = system_setting_info->DI_Width;
		pVIPSysInfo[40] = system_setting_info->D_Width;
		pVIPSysInfo[41] = system_setting_info->D_Height;
		pVIPSysInfo[42] = system_setting_info->VO_Width;
		pVIPSysInfo[43] = system_setting_info->VO_Height;
	}
	return 0;
}

int rtice_Get_VO_Info(_system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table, unsigned int* pVOInfo)
{
	SLR_VOINFO* info = NULL;
	unsigned char i;
	info = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());

	if (info == NULL) {
		for (i=0;i<VO_Info_Items_Num;i++)
			pVOInfo[i] = 0;
	} else {
		pVOInfo[0] = info->plane;
		pVOInfo[1] = info->port;
		pVOInfo[2] = info->mode;
		pVOInfo[3] = info->h_width;
		pVOInfo[4] = info->v_length;
		pVOInfo[5] = info->h_total;
		pVOInfo[6] = info->v_total;
		pVOInfo[7] = info->chroma_fmt;
		pVOInfo[8] = info->progressive;
		pVOInfo[9] = info->h_freq;
		pVOInfo[10] = info->v_freq;
		pVOInfo[11] = info->pixel_clk;
		pVOInfo[12] = info->source;
		pVOInfo[13] = info->isJPEG;
		pVOInfo[14] = info->mode_3d ;
		pVOInfo[15] = info->force2d ;
		pVOInfo[16] = info->se_pre_width;
		pVOInfo[17] = info->se_pre_height;
		pVOInfo[18] = info->set_se_blank;
		pVOInfo[19] = info->src_h_wid;
		pVOInfo[20] = info->src_v_len;
		pVOInfo[21] = info->video_full_range_flag;
		pVOInfo[22] = info->xvYCC;
		pVOInfo[23] = info->MaxCLL;
		pVOInfo[24] = info->MaxFALL;
		pVOInfo[25] = info->security;
		pVOInfo[26] = info->vdec_source;
		pVOInfo[27] = info->colorspace;
		pVOInfo[28] = info->lowdelay;
		pVOInfo[29] = info->isVP9;
		pVOInfo[30] = info->screenRotation;
		//pVOInfo[31] = info->color_transf;
		//pVOInfo[32] = info->coeffs_overscan;
		//pVOInfo[33] = info->dispXY0;
		//pVOInfo[34] = info->dispXY1;
		//pVOInfo[35] = info->dispXY2;
		//pVOInfo[36] = info->whitePointXY;
		//pVOInfo[37] = info->maxL;
		//pVOInfo[38] = info->minL;
		//pVOInfo[39] = info->DolbyVision;
		pVOInfo[40] = info->h_start;
		pVOInfo[41] = info->v_start;
		pVOInfo[42] = info->transfer_characteristics;
		pVOInfo[43] = info->matrix_coefficiets;
		pVOInfo[44] = info->colour_primaries;
		pVOInfo[45] = info->i_ratio;
		pVOInfo[46] = info->afd;
		pVOInfo[47] = info->pixelAR_hor;
		pVOInfo[48] = info->pixelAR_ver;
		pVOInfo[49] = info->launcher;
	}
	return 0;
}

int rtice_Get_HDMI_AVI_Info(_system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table, unsigned int *pAviInfo)
{
/*
	unsigned char *AviInfo;
	drvif_Hdmi_GetAviInfoFrame(unsigned char *AviInfo);
*/
	return 0;
}

int rtice_Get_HDMI_DRM_Info(_system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table, unsigned int *pDrmInfo)
{
	unsigned char i, ret;

	HDMI_DRM_T DRM_Info;
	ret = drvif_Hdmi_GetDrmInfoFrame((HDMI_DRM_T *)&DRM_Info);

	if (/*DRM_Info == NULL ||*/ ret == 0) {
		for (i=0;i<HDMI_DRM_Info_Items_Num;i++)
			pDrmInfo[i] = 0;
	} else {
		pDrmInfo[0] = DRM_Info.type_code;
		pDrmInfo[1] = DRM_Info.ver;
		pDrmInfo[2] = DRM_Info.len;
		pDrmInfo[3] = DRM_Info.eEOTFtype;
		pDrmInfo[4] = DRM_Info.eMeta_Desc;
		pDrmInfo[5] = DRM_Info.display_primaries_x0;
		pDrmInfo[6] = DRM_Info.display_primaries_y0;
		pDrmInfo[7] = DRM_Info.display_primaries_x1;
		pDrmInfo[8] = DRM_Info.display_primaries_y1;
		pDrmInfo[9] = DRM_Info.display_primaries_x2;
		pDrmInfo[10] = DRM_Info.display_primaries_y2;
		pDrmInfo[11] = DRM_Info.white_point_x;
		pDrmInfo[12] = DRM_Info.white_point_y;
		pDrmInfo[13] = DRM_Info.max_display_mastering_luminance;
		pDrmInfo[14] = DRM_Info.min_display_mastering_luminance;
		pDrmInfo[15] = DRM_Info.maximum_content_light_level;
		pDrmInfo[16] = DRM_Info.maximum_frame_average_light_level;
	}
	return 0;
}

int  rtice_ICM_ReadWrite_CurrentPrevious_Buff_save(ap_data_param rtice_param, unsigned char *num_type, unsigned int *curPre_buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag)
{
	//unsigned int *pBuff;
	unsigned int size;

	int g_h_sta = curPre_buf[0];
	int g_h_end = curPre_buf[1];
	int g_s_sta = curPre_buf[2];
	int g_s_end = curPre_buf[3];
	int g_i_sta = curPre_buf[4];
	int g_i_end = curPre_buf[5];

	/*save ICM table while all ICM table write*/
	if (((g_h_end - g_h_sta) < 47) ||((g_s_end - g_s_sta) < 7) || ((g_i_end - g_i_sta) < 7))
		return 1;

	New_idx++;
	if (New_idx >= ICM_TBL_Buff_Num)
		New_idx = 0;

	if (New_idx<0 || New_idx>=ICM_TBL_Buff_Num)
		return -1;

	if (tICM_buff[New_idx] == NULL)
		tICM_buff[New_idx] = (unsigned int *)vip_malloc(80000 * sizeof(unsigned char));

	size = rtice_param.size;

	if (tICM_buff[New_idx] == NULL)
		return -1;
	memcpy((unsigned char *)tICM_buff[New_idx], (unsigned char *)curPre_buf, size);

	return 0;
}

int  rtice_ICM_ReadWrite_From_CurrentPrevious_Buff(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag)
{
	signed char which_idx, which_buff;
	unsigned int size, isbufNull_flag;
	int g_h_sta = 0,  g_h_end = 0,  g_s_sta = 0,  g_s_end = 0,  g_i_sta = 0,  g_i_end = 0;
	unsigned int* pBuff_use = NULL;

	size = rtice_param.size;

	/* mode0 = 1, use pre buff, mode0 = 0, use cur buff*/
	which_buff = rtice_param.mode0;
	which_idx = New_idx - which_buff;
	if (which_idx < 0)
		which_idx = which_idx + ICM_TBL_Buff_Num;

	pBuff_use = tICM_buff[which_idx];

	if (isSet_Flag == 1) { /* set cur/pre buff to IC*/
		if (pBuff_use == NULL) /* protect */
			return -1;
		g_h_sta = pBuff_use[0];
		g_h_end = pBuff_use[1];
		g_s_sta = pBuff_use[2];
		g_s_end = pBuff_use[3];
		g_i_sta = pBuff_use[4];
		g_i_end = pBuff_use[5];

		fwif_color_icm_SramBlockAccessSpeedup(pBuff_use+6, 1,  g_h_sta,  g_h_end,  g_s_sta,  g_s_end,  g_i_sta,  g_i_end);

	} else { /* check cur/pre buff */
		if (pBuff_use == NULL)
			isbufNull_flag = 0xffffffff;	/* get 0xffffffff while cur/pre buff is NULL*/
		else
			isbufNull_flag = 0;
		fwif_color_ChangeUINT32Endian(&isbufNull_flag , 1, 1);
		memcpy(buf, (unsigned char*)(&isbufNull_flag), size);

		*num_type = VIP_UINT32;
	}

	return 0;

}

int  rtice_SetGet_HDR_DM2(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag)
{
	unsigned int* pwUINT32Temp;
	unsigned short* pwUINT16Temp;
	unsigned int size, start_addr, end_addr;
	unsigned char mode0;

#ifdef CONFIG_HW_SUPPORT_HDR_TONE
	short* pwINT16Temp;
	char ret = 0;
	unsigned char dummy = 0;
#endif
	size = rtice_param.size;
	start_addr = rtice_param.saddr;
	end_addr = rtice_param.eaddr;
	mode0 = rtice_param.mode0;

	if (isSet_Flag == 1) {	/* write */
		if (rtice_param.id == TOOLACCESS_DM2_HDR_EOTF_TBL) {
			if (size > (sizeof(unsigned int) * (EOTF_size * 3)))
				return -ID_SIZE_ERR;
			pwUINT32Temp = (unsigned int *)vip_malloc((EOTF_size * 3) * sizeof(int));
			if (pwUINT32Temp == NULL)
				return -ID_SIZE_ERR;

			if (size == (sizeof(unsigned int) * (EOTF_size * 3))) {
				memcpy((char*) pwUINT32Temp, buf, size);
				fwif_color_ChangeUINT32Endian(pwUINT32Temp, EOTF_size*3, 1);
				fwif_color_rtice_DM2_EOTF_Set(pwUINT32Temp + (EOTF_size * 0), pwUINT32Temp + (EOTF_size * 1), pwUINT32Temp + (EOTF_size * 2));
			}

			vip_free(pwUINT32Temp);

		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_OETF_TBL) {
			if (size > (sizeof(unsigned short) * (OETF_size * 3)))
				return -ID_SIZE_ERR;
			pwUINT16Temp = (unsigned short *)vip_malloc((OETF_size * 3) * sizeof(short));
			if (pwUINT16Temp == NULL)
				return -ID_SIZE_ERR;

			if (size == (sizeof(short) * (OETF_size * 3))) {
				memcpy((char*) pwUINT16Temp, buf, size);
				fwif_color_ChangeUINT16Endian(pwUINT16Temp, OETF_size*3, 1);
				fwif_color_rtice_DM2_OETF_Set(pwUINT16Temp + (OETF_size * 0), pwUINT16Temp + (OETF_size * 1), pwUINT16Temp + (OETF_size * 2));
			}

			vip_free(pwUINT16Temp);
#ifdef CONFIG_HW_SUPPORT_HDR_TONE
		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_Tone_Mapping_TBL) {
			if (size > (sizeof(unsigned short) * (ToneMapping_size * 3)))
				return -ID_SIZE_ERR;
			pwINT16Temp = (unsigned short *)vip_malloc((ToneMapping_size * 3) * sizeof(short));
			if (pwINT16Temp == NULL)
				return -ID_SIZE_ERR;

			if (size == (sizeof(short) * (ToneMapping_size * 3))) {
				memcpy((char*) pwINT16Temp, buf, size);
				fwif_color_ChangeUINT16Endian(pwINT16Temp, ToneMapping_size*3, 1);
				ret = fwif_color_rtice_DM2_ToneMapping_Set(pwINT16Temp + (ToneMapping_size * 0), pwINT16Temp + (ToneMapping_size * 1), pwINT16Temp + (ToneMapping_size * 2));
			}

			vip_free(pwINT16Temp);
			if (ret < 0)
				return -ID_SIZE_ERR;
#endif //CONFIG_HW_SUPPORT_HDR_TONE
#ifdef CONFIG_HW_SUPPORT_HDR_3DLUT_24
		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_24x24x24_3D_TBL) {
			if (size > (sizeof(unsigned short) * (HDR_24x24x24_size * 3)))
				return -ID_SIZE_ERR;
			pwUINT16Temp = (unsigned short *)vip_malloc((HDR_24x24x24_size * 3) * sizeof(short));
			if (pwUINT16Temp == NULL)
				return -ID_SIZE_ERR;

			if (size == (sizeof(short) * (HDR_24x24x24_size * 3))) {
				memcpy((char*) pwUINT16Temp, buf, size);
				fwif_color_ChangeUINT16Endian(pwUINT16Temp, (HDR_24x24x24_size * 3), 1);
				ret = fwif_color_rtice_DM2_3D_24x24x24_LUT_Set(1, pwUINT16Temp);
			}

			vip_free(pwUINT16Temp);
			if (ret < 0)
				return -ID_SIZE_ERR;
#endif //CONFIG_HW_SUPPORT_HDR_3DLUT_24
		} else {
			return -1;
		}
	} else {	/* read */
		if (rtice_param.id == TOOLACCESS_DM2_HDR_EOTF_TBL) {
			if (size > (sizeof(unsigned int) * (EOTF_size * 3)))
				return -ID_SIZE_ERR;
			pwUINT32Temp = (unsigned int *)vip_malloc((EOTF_size * 3) * sizeof(int));
			if (pwUINT32Temp == NULL)
				return -ID_SIZE_ERR;

			if (size == (sizeof(unsigned int) * (EOTF_size * 3))) {
				fwif_color_rtice_DM2_EOTF_Get(pwUINT32Temp + (EOTF_size * 0), pwUINT32Temp + (EOTF_size * 1), pwUINT32Temp + (EOTF_size * 2));
				fwif_color_ChangeUINT32Endian(pwUINT32Temp, EOTF_size*3, 1);
				memcpy(buf, (char*) pwUINT32Temp, size);
			}
			*num_type = VIP_UINT32;
			vip_free(pwUINT32Temp);

		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_OETF_TBL) {
			if (size > (sizeof(unsigned short) * (OETF_size * 3)))
				return -ID_SIZE_ERR;
			pwUINT16Temp = (unsigned short *)vip_malloc((OETF_size * 3) * sizeof(short));
			if (pwUINT16Temp == NULL)
				return -ID_SIZE_ERR;

			if (size == (sizeof(short) * (OETF_size * 3))) {
				fwif_color_rtice_DM2_OETF_Get(pwUINT16Temp + (OETF_size * 0), pwUINT16Temp + (OETF_size * 1), pwUINT16Temp + (OETF_size * 2));
				fwif_color_ChangeUINT16Endian(pwUINT16Temp, OETF_size*3, 1);
				memcpy(buf, (char*) pwUINT16Temp, size);
			}
			*num_type = VIP_UINT16;
			vip_free(pwUINT16Temp);
#ifdef CONFIG_HW_SUPPORT_HDR_TONE
		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_Tone_Mapping_TBL) {
			dummy++;
			if (size > (sizeof(unsigned short) * (ToneMapping_size * 3)))
				ret = -1;
			dummy++;
			pwINT16Temp = (unsigned short *)vip_malloc((ToneMapping_size * 3) * sizeof(short));
			if (pwINT16Temp == NULL)
				ret = -2;
			dummy++;
			if ((size == (sizeof(short) * (ToneMapping_size * 3))) && ret >= 0) {
				ret = fwif_color_rtice_DM2_ToneMapping_Get(pwINT16Temp + (ToneMapping_size * 0), pwINT16Temp + (ToneMapping_size * 1), pwINT16Temp + (ToneMapping_size * 2));
				fwif_color_ChangeUINT16Endian(pwINT16Temp, ToneMapping_size*3, 1);
				memcpy(buf, (char*) pwINT16Temp, size);
				dummy++;
			}
			*num_type = VIP_UINT16;

			if (pwINT16Temp != NULL)
				vip_free(pwINT16Temp);

			system_setting_info->Debug_Buff_8[49] = dummy;	/* read/write error randomly, I don't know why dummy can fix this, elieli*/
			if (ret < 0)
				return -ID_SIZE_ERR;
#endif //CONFIG_HW_SUPPORT_HDR_TONE
#ifdef CONFIG_HW_SUPPORT_HDR_3DLUT_24
		} else if (rtice_param.id == TOOLACCESS_DM2_HDR_24x24x24_3D_TBL) {
			if (size > (sizeof(unsigned short) * (HDR_24x24x24_size * 3)))
				return -ID_SIZE_ERR;
			pwUINT16Temp = (unsigned short *)vip_malloc((HDR_24x24x24_size * 3) * sizeof(short));
			if (pwUINT16Temp == NULL)
				return -ID_SIZE_ERR;

			if (size == (sizeof(short) * (HDR_24x24x24_size * 3))) {
				ret = fwif_color_rtice_DM2_3D_24x24x24_LUT_Get(pwUINT16Temp, HDR_3DLUT_getFromReg);
				fwif_color_ChangeUINT16Endian(pwUINT16Temp, (HDR_24x24x24_size * 3), 1);
				memcpy(buf, (char*) pwUINT16Temp, size);
			}
			*num_type = VIP_UINT16;
			vip_free(pwUINT16Temp);

			if (ret < 0)
				return -ID_SIZE_ERR;
#endif //CONFIG_HW_SUPPORT_HDR_3DLUT_24
		} else {
			return -1;
		}
	}

	return 0;
}

int  rtice_SetGet_PQ_Power_Saving(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag)
{
	int ret;
	unsigned char lv;

	if (isSet_Flag == 1) {
		if (buf == NULL)
			return -1;

		lv = *buf;

		/*ret = fwif_color_PQ_ByPass_Handler(lv, 1, system_setting_info, 0);*/
		ret = Scaler_color_Set_PQ_ByPass_Lv(lv);

	} else {
		if (buf == NULL)
			return -1;
		if (num_type == NULL)
			return -1;

		/*ret = fwif_color_PQ_ByPass_Handler(0, 0, system_setting_info, 0);*/
		ret = Scaler_color_Get_PQ_ByPass_Lv();

		*num_type = VIP_UINT8;
		*buf = ret;
	}

	return ret;
}

static unsigned int D_3D_LUT_TMP[VIP_D_3D_LUT_UI_TBL_SIZE] = {0};/* 17*17*17*3 */
int  rtice_Set_D_LUT_17to9(unsigned int *array)
{
	unsigned int i, j, k, index;
	index = 0;

	memset(D_3D_LUT_TMP, 0, sizeof(int)*VIP_D_3D_LUT_UI_TBL_SIZE);

	for(i=0;i<VIP_D_3D_LUT_INDEX;i+=2) {
		for(j=0;j<VIP_D_3D_LUT_INDEX;j+=2) {
			for(k=0;k<VIP_D_3D_LUT_INDEX;k+=2) {
				memcpy(&D_3D_LUT_TMP[index*3], &array[((i*VIP_D_3D_LUT_INDEX*VIP_D_3D_LUT_INDEX)+(j*VIP_D_3D_LUT_INDEX)+k)*3], sizeof(int)*3);
				index++;
			}
		}
	}

	memcpy(array, D_3D_LUT_TMP, sizeof(int)*VIP_D_3D_LUT_UI_TBL_SIZE);

	return 0;
}

int  rtice_Set_D_LUT_9to17(unsigned int *array)
{
	unsigned int i, j, k, index;
	index = 0;

	memset(D_3D_LUT_TMP, 0, sizeof(int)*VIP_D_3D_LUT_UI_TBL_SIZE);

	for(i=0;i<VIP_D_3D_LUT_INDEX;i+=2) {
		for(j=0;j<VIP_D_3D_LUT_INDEX;j+=2) {
			for(k=0;k<VIP_D_3D_LUT_INDEX;k+=2) {
				memcpy(&D_3D_LUT_TMP[((i*VIP_D_3D_LUT_INDEX*VIP_D_3D_LUT_INDEX)+(j*VIP_D_3D_LUT_INDEX)+k)*3], &array[index*3], sizeof(int)*3);
				index++;
			}
		}
	}

	memcpy(array, D_3D_LUT_TMP, sizeof(int)*VIP_D_3D_LUT_UI_TBL_SIZE);

	return 0;
}

DRV_DeMura_DMA_TBL DMA_TBL_Buff;
int  rtice_SetGet_DeMura(ap_data_param *rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag)
{
	extern VIP_DeMura_TBL DeMura_TBL;
	unsigned int tbl_line_size;
	unsigned int base_addr, i;
	unsigned int *vir_addr;
	unsigned char *vir_addr_8;
	//unsigned int sizeDemuraTbl = sizeof(unsigned char)*VIP_Demura_Encode_TBL_ROW*VIP_Demura_Encode_TBL_COL;
	unsigned int adaptiveTBL_Size = sizeof(DRV_DeMura_Adaptive_TBL);
	unsigned char *pDemura_TBL_tmp;
	DRV_DeMura_CTRL_TBL ori_ctrl_TBL={0};
	unsigned int size_offset = 10 * 1024; // avoid trash other memory while size do 4k and 96B aligned
	unsigned int sizeDemuraTbl = VIP_DeMura_BUFFER_SIZE - size_offset;
	unsigned int base_addr_aligned = 0, sizeDemuraTbl_aligned;
	unsigned int  *vir_addr_aligned = NULL;
	unsigned char aligned_val = 0;
#ifdef CONFIG_ARM64 //ARM32 compatible
	unsigned long va_temp;
#else
	unsigned int va_temp;
#endif

	base_addr = get_query_start_address(QUERY_IDX_VIP_DeMura);
	base_addr = drvif_memory_get_data_align(base_addr, (1 << 12));
	sizeDemuraTbl = drvif_memory_get_data_align(sizeDemuraTbl, (1 << 12));

	if (base_addr == 0) {
		rtd_pr_vpq_emerg("rtice_SetGet_DeMura, addr = NULL\n");
		return -1;
	}

	if (isSet_Flag == 1) {
		if (rtice_param->id == TOOLACCESS_VIP_DeMura_TBL) {
			if (rtice_param->size > sizeDemuraTbl || rtice_param->saddr + rtice_param->size > sizeDemuraTbl) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			memcpy(&(DeMura_TBL.TBL[ rtice_param->saddr ]), buf, rtice_param->size);
			fwif_color_Get_DeMura_CTRL(&ori_ctrl_TBL);
			memcpy((unsigned char*)&DeMura_TBL.DeMura_CTRL_TBL, (unsigned char*)&ori_ctrl_TBL, sizeof(DRV_DeMura_CTRL_TBL));
			fwif_color_DeMura_init();

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_Adaptive) {
			if (rtice_param->size > adaptiveTBL_Size || rtice_param->saddr + rtice_param->size > adaptiveTBL_Size) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura adative error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			memcpy(&DeMura_TBL.DeMura_Adaptive_TBL + rtice_param->saddr, buf, rtice_param->size);

		}  else if (rtice_param->id == TOOLACCESS_VIP_DeMura_TBL_Mode) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_TBL_Mode error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			fwif_color_set_DeMura_TBL_Mode(*buf);
			fwif_color_Get_DeMura_CTRL(&ori_ctrl_TBL);
			memcpy((unsigned char*)&DeMura_TBL.DeMura_CTRL_TBL, (unsigned char*)&ori_ctrl_TBL, sizeof(DRV_DeMura_CTRL_TBL));
			fwif_color_DeMura_init();

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_INI) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_INI error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			fwif_color_DeMura_init();

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_Channel_Mode) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_Channel_Mode error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}


			fwif_color_Get_DeMura_CTRL(&ori_ctrl_TBL);
			memcpy((unsigned char*)&DeMura_TBL.DeMura_CTRL_TBL, (unsigned char*)&ori_ctrl_TBL, sizeof(DRV_DeMura_CTRL_TBL));
			if (*buf == 0)
				DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate = DeMura_Ch_Y_Mode;
			else
				DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate = DeMura_Ch_RGB_Separate;
			fwif_color_DeMura_init();

		}
		else {
			return -1;
		}
	} else {
		if (rtice_param->id == TOOLACCESS_VIP_DeMura_TBL) {
			if (rtice_param->size > sizeDemuraTbl || rtice_param->saddr + rtice_param->size > sizeDemuraTbl) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			pDemura_TBL_tmp = (unsigned char *)vip_malloc(sizeDemuraTbl * sizeof(char));
			if (pDemura_TBL_tmp == NULL) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura error, alloc demura tbl tmp fail\n");
				return -ID_SIZE_ERR;
			}

			if (DeMura_TBL.table_mode == DeMura_TBL_481x271) {
				if (DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate == DeMura_Ch_Y_Mode) {	/* Y mode */
					tbl_line_size = VIP_DemuraTBL_LineSize_Y_481x271;
				} else {
					tbl_line_size = VIP_DemuraTBL_LineSize_RGB_481x271;
				}
			} else {
				if (DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate == DeMura_Ch_Y_Mode) {	/* Y mode */
					tbl_line_size = VIP_DemuraTBL_LineSize_Y_241x136;
				} else {
					tbl_line_size = VIP_DemuraTBL_LineSize_RGB_241x136;
				}
			}

			vir_addr = dvr_remap_uncached_memory(base_addr, sizeDemuraTbl, __builtin_return_address(0));

			sizeDemuraTbl_aligned = sizeDemuraTbl;
			base_addr_aligned = base_addr;
#ifdef CONFIG_ARM64 //ARM32 compatible
			va_temp = (unsigned long)vir_addr;
#else
			va_temp = (unsigned int)vir_addr;
#endif
			vir_addr_aligned = (unsigned int*)va_temp;
			aligned_val = 0;

#ifdef CONFIG_BW_96B_ALIGNED
			sizeDemuraTbl_aligned = dvr_size_alignment(sizeDemuraTbl);
			base_addr_aligned = (unsigned int)dvr_memory_alignment((unsigned long)base_addr, sizeDemuraTbl_aligned);
#ifdef CONFIG_ARM64 //ARM32 compatible
			va_temp = (unsigned long)vir_addr;
#else //CONFIG_ARM64
			va_temp = (unsigned int)vir_addr;
#endif //CONFIG_ARM64
			vir_addr_aligned = (unsigned int*)dvr_memory_alignment((unsigned long)va_temp, sizeDemuraTbl_aligned);
			aligned_val = 6;
#endif

			//rtd_pr_vpq_emerg("DMA_TBL_Buff.table_height=%d,DMA_TBL_Buff.line_step=%d\n", DMA_TBL_Buff.table_height, DMA_TBL_Buff.line_step);
			vir_addr_8 = (unsigned char *)vir_addr_aligned;

			for(i=0; i<(DMA_TBL_Buff.table_height-1); i++)
				memcpy(pDemura_TBL_tmp + (i * tbl_line_size), vir_addr_8 + (i * (DMA_TBL_Buff.line_step<<4)), sizeof(char) * tbl_line_size);

			memcpy(buf, pDemura_TBL_tmp + rtice_param->saddr, rtice_param->size);
			dvr_unmap_memory((void *)vir_addr, sizeDemuraTbl);

			*num_type = VIP_UINT8;
			vip_free(pDemura_TBL_tmp);


		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_Adaptive) {
			if (rtice_param->size > adaptiveTBL_Size || rtice_param->saddr + rtice_param->size > adaptiveTBL_Size) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura adaptive error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			memcpy(buf, &DeMura_TBL.DeMura_Adaptive_TBL + rtice_param->saddr, rtice_param->size);

			*num_type = VIP_UINT8;

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_TBL_Mode) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_INI error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			*buf = fwif_color_get_DeMura_TBL_Mode();
			*num_type = VIP_UINT8;

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_INI) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_INI error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			*buf = 1;
			*num_type = VIP_UINT8;

		} else if (rtice_param->id == TOOLACCESS_VIP_DeMura_Channel_Mode) {
			if (rtice_param->size > sizeof(char)) {
				rtd_pr_vpq_emerg("rtice_SetGet_DeMura TOOLACCESS_VIP_DeMura_INI error, size = %d, start addr = %d, end addr = %d\n", rtice_param->size, rtice_param->saddr, rtice_param->eaddr);
				return -1;
			}

			*buf = DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate;
			*num_type = VIP_UINT8;

		} else {
			return -1;
		}
	}
	return 0;
}

unsigned char  rtice_SetGet_Froce_vTop_Flag(unsigned char Force_flag, unsigned char isSet_Flag)
{
	static unsigned char Force_vTop_Flag = 0;
	/* Force_flag = 0 => skip force vTop, Force_flag = 1 => force to SDR, Force_flag = 2 => force to HDR10, Force_flag = 3 => force to HLG,*/
	if (isSet_Flag == 1) {
		if (Force_flag >= 1 && Force_flag <= 3)
			Force_vTop_Flag = Force_flag;
		else
			Force_vTop_Flag = 0;

		rtd_pr_vpq_emerg("rtice_SetGet_Froce_vTop_Flag, Force_vTop_Flag = %d\n", Force_vTop_Flag);
	}

	return  Force_vTop_Flag;
}

char rtice_SetGet_Froce_vTop_HDMI_DRM(void *pHDMI_DRM, unsigned char isSet_Flag)
{
	static HDMI_DRM_T drm_info_for_Force_vTop = {135, 1, 26, 2, 0, 13250, 34500, 7500, 3000, 34000, 16000, 15635, 16450, 1100, 0, 0, 0, 0};	/* default setting for HDR10*/

	if (pHDMI_DRM == NULL) {
		rtd_pr_vpq_emerg("rtice_SetGet_Froce_vTop_HDMI_DRM, NULL\n");
		return -1;
	}

	if (isSet_Flag == 1) {
		memcpy((unsigned char*)&drm_info_for_Force_vTop, (unsigned char*)pHDMI_DRM, sizeof(HDMI_DRM_T));
	} else {
		memcpy((unsigned char*)pHDMI_DRM, (unsigned char*)&drm_info_for_Force_vTop, sizeof(HDMI_DRM_T));
	}
	return 0;
}

int  rtice_SetGet_Froce_vTop_TBL(ap_data_param *rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag)
{
	unsigned char input_src = system_setting_info->Input_src_Type;
	HDMI_DRM_T DRM_Info = {0};
	unsigned char force_vtop_flag = 0;
	unsigned int size = rtice_param->size;
	unsigned int addr_st = rtice_param->saddr;

	unsigned short *shortpwUINT16Temp, *pbuf_tmp;

	if (input_src == _SRC_HDMI) {
		if (size > sizeof(short) * 18)
			return -ID_SIZE_ERR;
	} else {
		return -ID_SIZE_ERR;
	}

	shortpwUINT16Temp = (unsigned short *)vip_malloc(18 * sizeof(short));
	if (shortpwUINT16Temp == NULL)
		return -ID_SIZE_ERR;
	pbuf_tmp = (unsigned short*)buf;

	if (isSet_Flag == 1) {
		if (input_src == _SRC_HDMI) {
			shortpwUINT16Temp[0] 		=	force_vtop_flag								;
			shortpwUINT16Temp[1] 		=   DRM_Info.type_code                        ;
			shortpwUINT16Temp[2] 		=   DRM_Info.ver                              ;
			shortpwUINT16Temp[3] 		=   DRM_Info.len                              ;
			shortpwUINT16Temp[4] 		=   DRM_Info.eEOTFtype                        ;
			shortpwUINT16Temp[5] 		=   DRM_Info.eMeta_Desc                       ;
			shortpwUINT16Temp[6] 		=   DRM_Info.display_primaries_x0             ;
			shortpwUINT16Temp[7] 		=   DRM_Info.display_primaries_y0             ;
			shortpwUINT16Temp[8] 		=   DRM_Info.display_primaries_x1             ;
			shortpwUINT16Temp[9] 		=   DRM_Info.display_primaries_y1             ;
			shortpwUINT16Temp[10]		=   DRM_Info.display_primaries_x2             ;
			shortpwUINT16Temp[11]		=   DRM_Info.display_primaries_y2             ;
			shortpwUINT16Temp[12]		=   DRM_Info.white_point_x                    ;
			shortpwUINT16Temp[13]		=   DRM_Info.white_point_y                    ;
			shortpwUINT16Temp[14]		=   DRM_Info.max_display_mastering_luminance  ;
			shortpwUINT16Temp[15]		=   DRM_Info.min_display_mastering_luminance  ;
			shortpwUINT16Temp[16]		=   DRM_Info.maximum_content_light_level      ;
			shortpwUINT16Temp[17]		=	DRM_Info.maximum_frame_average_light_level;

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
			memcpy((void*)(shortpwUINT16Temp+addr_st), (void*)(pbuf_tmp+addr_st), size);

			force_vtop_flag = shortpwUINT16Temp[0]  ;
			DRM_Info.type_code                         =   shortpwUINT16Temp[1]  ;
			DRM_Info.ver                               =   shortpwUINT16Temp[2]  ;
			DRM_Info.len                               =   shortpwUINT16Temp[3]  ;
			DRM_Info.eEOTFtype                         =   shortpwUINT16Temp[4]  ;
			DRM_Info.eMeta_Desc                        =   shortpwUINT16Temp[5]  ;
			DRM_Info.display_primaries_x0              =   shortpwUINT16Temp[6]  ;
			DRM_Info.display_primaries_y0              =   shortpwUINT16Temp[7]  ;
			DRM_Info.display_primaries_x1              =   shortpwUINT16Temp[8]  ;
			DRM_Info.display_primaries_y1              =   shortpwUINT16Temp[9]  ;
			DRM_Info.display_primaries_x2              =   shortpwUINT16Temp[10] ;
			DRM_Info.display_primaries_y2              =   shortpwUINT16Temp[11] ;
			DRM_Info.white_point_x                     =   shortpwUINT16Temp[12] ;
			DRM_Info.white_point_y                     =   shortpwUINT16Temp[13] ;
			DRM_Info.max_display_mastering_luminance   =   shortpwUINT16Temp[14] ;
			DRM_Info.min_display_mastering_luminance   =   shortpwUINT16Temp[15] ;
			DRM_Info.maximum_content_light_level       =   shortpwUINT16Temp[16] ;
			DRM_Info.maximum_frame_average_light_level =shortpwUINT16Temp[17] ;

			rtice_SetGet_Froce_vTop_Flag(force_vtop_flag, 1);
			rtice_SetGet_Froce_vTop_HDMI_DRM((void*)&DRM_Info, 1);

		}

	} else {
		if (input_src == _SRC_HDMI) {

			force_vtop_flag = rtice_SetGet_Froce_vTop_Flag(force_vtop_flag, 0);
			rtice_SetGet_Froce_vTop_HDMI_DRM((void*)&DRM_Info, 0);

			shortpwUINT16Temp[0] 		=	force_vtop_flag								;
			shortpwUINT16Temp[1] 		=   DRM_Info.type_code                        ;
			shortpwUINT16Temp[2] 		=   DRM_Info.ver                              ;
			shortpwUINT16Temp[3] 		=   DRM_Info.len                              ;
			shortpwUINT16Temp[4] 		=   DRM_Info.eEOTFtype                        ;
			shortpwUINT16Temp[5] 		=   DRM_Info.eMeta_Desc                       ;
			shortpwUINT16Temp[6] 		=   DRM_Info.display_primaries_x0             ;
			shortpwUINT16Temp[7] 		=   DRM_Info.display_primaries_y0             ;
			shortpwUINT16Temp[8] 		=   DRM_Info.display_primaries_x1             ;
			shortpwUINT16Temp[9] 		=   DRM_Info.display_primaries_y1             ;
			shortpwUINT16Temp[10]		=   DRM_Info.display_primaries_x2             ;
			shortpwUINT16Temp[11]		=   DRM_Info.display_primaries_y2             ;
			shortpwUINT16Temp[12]		=   DRM_Info.white_point_x                    ;
			shortpwUINT16Temp[13]		=   DRM_Info.white_point_y                    ;
			shortpwUINT16Temp[14]		=   DRM_Info.max_display_mastering_luminance  ;
			shortpwUINT16Temp[15]		=   DRM_Info.min_display_mastering_luminance  ;
			shortpwUINT16Temp[16]		=   DRM_Info.maximum_content_light_level      ;
			shortpwUINT16Temp[17]		=	DRM_Info.maximum_frame_average_light_level;

			memcpy((void*)(pbuf_tmp+addr_st), (void*)(shortpwUINT16Temp+addr_st), size);
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
		}

		*num_type = VIP_UINT16;
	}

	vip_free(shortpwUINT16Temp);
	return 0;
}

int  rtice_SetGet_BP_TV002(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag)
{
	unsigned short tbl_sel, lv_tbl_sel;
	unsigned short *pwUINT16Temp;

	unsigned short id = rtice_param.id;
	unsigned int size = rtice_param.size;
	unsigned int saddr = rtice_param.saddr;
	//unsigned int eaddr = rtice_param.eaddr;
	//unsigned char mode0 = rtice_param.mode0;
	//unsigned char mode1 = rtice_param.mode1;

	unsigned int VIP_BP_CTRL_SIZE = sizeof(VIP_BP_CTRL) - (sizeof(short) * BP_TBL_MAX * VIP_MAGIC_GAMMA_PICTURE_MODE_MAX);
	unsigned int VIP_BP_CTRL_TBL_SIZE = sizeof(short) * VIP_MAGIC_GAMMA_PICTURE_MODE_MAX;
	unsigned int VIP_BP_STATUS_SIZE = sizeof(VIP_BP_STATUS);
	unsigned int VIP_BP_TBL_SIZE = sizeof(VIP_BP_TBL);
#ifndef CONFIG_CUSTOMER_TV010
	SLR_VIP_TABLE_CUSTOM_TV002* custom_TBL_TV002 = Scaler_get_tv002_Custom_struct();

	tbl_sel = custom_TBL_TV002->BP_Function.BP_Ctrl.BP_TBL_Select;
	lv_tbl_sel = custom_TBL_TV002->BP_Function.BP_Ctrl.BP_Level_TBL_Select;

	if (isSet_Flag == 1) {	/* write */
		if ( id== TOOLACCESS_BP_CTRL_TV002) {
			if (size > VIP_BP_CTRL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = &custom_TBL_TV002->BP_Function.BP_Ctrl.BP_TBL_Select;
			pwUINT16Temp = pwUINT16Temp + saddr;

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			memcpy(pwUINT16Temp, buf, size);

		} else if ( id== TOOLACCESS_BP_CTRL_TBL_TV002) {
			if (size > VIP_BP_CTRL_TBL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = custom_TBL_TV002->BP_Function.BP_Ctrl.BP_Level_TBL[lv_tbl_sel];
			pwUINT16Temp = pwUINT16Temp + saddr;

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			memcpy(pwUINT16Temp, buf, size);

		} else if ( id== TOOLACCESS_BP_STATUS_TV002) {
			if (size > VIP_BP_STATUS_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = &custom_TBL_TV002->BP_Function.BP_STATUS.BP_Gain_TBL;
			pwUINT16Temp = pwUINT16Temp + saddr;

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			memcpy(pwUINT16Temp, buf, size);

		} else if ( id== TOOLACCESS_BP_TBL_TV002) {
			if (size > VIP_BP_TBL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = &custom_TBL_TV002->BP_Function.BP_Table[tbl_sel].BP_Enable;
			pwUINT16Temp = pwUINT16Temp + saddr;

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			memcpy(pwUINT16Temp, buf, size);

		} else {
			return -ID_SIZE_ERR;
		}
	} else {
		if ( id== TOOLACCESS_BP_CTRL_TV002) {
			if (size > VIP_BP_CTRL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = &custom_TBL_TV002->BP_Function.BP_Ctrl.BP_TBL_Select;
			pwUINT16Temp = pwUINT16Temp + saddr;
			memcpy(buf, pwUINT16Temp, size);

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;

		} else if ( id== TOOLACCESS_BP_CTRL_TBL_TV002) {
			if (size > VIP_BP_CTRL_TBL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = custom_TBL_TV002->BP_Function.BP_Ctrl.BP_Level_TBL[lv_tbl_sel];
			pwUINT16Temp = pwUINT16Temp + saddr;
			memcpy(buf, pwUINT16Temp, size);

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;

		} else if ( id== TOOLACCESS_BP_STATUS_TV002) {
			if (size > VIP_BP_STATUS_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = &custom_TBL_TV002->BP_Function.BP_STATUS.BP_Gain_TBL;
			pwUINT16Temp = pwUINT16Temp + saddr;
			memcpy(buf, pwUINT16Temp, size);

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;

		} else if ( id== TOOLACCESS_BP_TBL_TV002) {
			if (size > VIP_BP_TBL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = &custom_TBL_TV002->BP_Function.BP_Table[tbl_sel].BP_Enable;
			pwUINT16Temp = pwUINT16Temp + saddr;
			memcpy(buf, pwUINT16Temp, size);

			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);

			*num_type = VIP_UINT16;

		} else {
			return -ID_SIZE_ERR;
		}

	}
	#endif
	return 0;
}

int  rtice_SetGet_DBC_TV002(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag)
{
	unsigned char *pwUINT8Temp;

	unsigned short id = rtice_param.id;
	unsigned int size = rtice_param.size;
	unsigned int saddr = rtice_param.saddr;
	//unsigned int eaddr = rtice_param.eaddr;
	//unsigned char mode0 = rtice_param.mode0;
	//unsigned char mode1 = rtice_param.mode1;

	unsigned int VIP_DBC_CTRL_SIZE = sizeof(VIP_DBC_CTRL);
	unsigned int VIP_DBC_STATUS_SIZE = sizeof(VIP_DBC_STATUS);
	unsigned int VIP_DBC_SW_REG_SIZE = sizeof(VIP_DBC_SW_REG);
#ifndef CONFIG_CUSTOMER_TV010
	SLR_VIP_TABLE_CUSTOM_TV002* custom_TBL_TV002 = Scaler_get_tv002_Custom_struct();

	if (isSet_Flag == 1) {	/* write */
		if ( id== TOOLACCESS_DBC_STATUS_TV002) {
			if (size > VIP_DBC_STATUS_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_STATUS.Basic_Backlight;
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(pwUINT8Temp, buf, size);

		} else if ( id== TOOLACCESS_DBC_CTRL_TV002) {
			if (size > VIP_DBC_CTRL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_CTRL.DBC_Enable;
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(pwUINT8Temp, buf, size);

		} else if ( id== TOOLACCESS_DBC_SW_REG_TV002) {
			if (size > VIP_DBC_SW_REG_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_SW_REG.BL_TOP;
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(pwUINT8Temp, buf, size);

		} else {
			return -ID_SIZE_ERR;
		}
	} else {
		if ( id== TOOLACCESS_DBC_STATUS_TV002) {
			if (size > VIP_DBC_STATUS_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_STATUS.Basic_Backlight;
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(buf, pwUINT8Temp, size);
			*num_type = VIP_UINT8;

		} else if ( id== TOOLACCESS_DBC_CTRL_TV002) {
			if (size > VIP_DBC_CTRL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_CTRL.DBC_Enable;
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(buf, pwUINT8Temp, size);
			*num_type = VIP_UINT8;

		} else if ( id== TOOLACCESS_DBC_SW_REG_TV002) {
			if (size > VIP_DBC_SW_REG_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = &custom_TBL_TV002->DBC.DBC_SW_REG.BL_TOP;
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(buf, pwUINT8Temp, size);
			*num_type = VIP_UINT8;

		} else {
			return -ID_SIZE_ERR;
		}
	}
	#endif
	return 0;
}

int  rtice_SetGet_Zero_D_DBC_TV002(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag)
{
	unsigned char *pwUINT8Temp;
	unsigned short *pwUINT16Temp;
	unsigned short id = rtice_param.id;
	unsigned int size = rtice_param.size;
	unsigned int saddr = rtice_param.saddr;
	//unsigned int eaddr = rtice_param.eaddr;
	//unsigned char mode0 = rtice_param.mode0;
	//unsigned char mode1 = rtice_param.mode1;

	unsigned int VIP_Zero_D_DBC_CTRL_SIZE = sizeof(VIP_ZERO_D_CTRL);
	unsigned int VIP_Zero_D_DBC_STATUS_SIZE = sizeof(VIP_ZERO_D_STATUS);
	unsigned int VIP_Zero_D_DBC_LUT_SIZE = sizeof(VIP_ZERO_D_LOOKUP_TBL);

	SLR_VIP_TABLE_CUSTOM_TV002* custom_TBL_TV002 = Scaler_get_tv002_Custom_struct();

	if (isSet_Flag == 1) {	/* write */
		if ( id== TOOLACCESS_Zero_D_DBC_STATUS_TV002) {
			if (size > VIP_Zero_D_DBC_STATUS_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = (unsigned short*)&custom_TBL_TV002->ZERO_D.ZERO_D_STATUS;
			pwUINT16Temp = pwUINT16Temp + saddr;

			memcpy(pwUINT16Temp, buf, size);
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
		} else if ( id== TOOLACCESS_Zero_D_DBC_CTRL_TV002) {
			if (size > VIP_Zero_D_DBC_CTRL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = (unsigned char*)&custom_TBL_TV002->ZERO_D.ZERO_D_CTRL;
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(pwUINT8Temp, buf, size);

		} else if ( id== TOOLACCESS_Zero_D_DBC_LUT_TV002) {
			if (size > VIP_Zero_D_DBC_LUT_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = (unsigned short*)&custom_TBL_TV002->ZERO_D.ZERO_D_LOOKUP_TBL;
			pwUINT16Temp = pwUINT16Temp + saddr;

			memcpy(pwUINT16Temp, buf, size);
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
		} else {
			return -ID_SIZE_ERR;
		}
	} else {
		if ( id== TOOLACCESS_Zero_D_DBC_STATUS_TV002) {
			if (size > VIP_Zero_D_DBC_STATUS_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = (unsigned short*)&custom_TBL_TV002->ZERO_D.ZERO_D_STATUS;
			pwUINT16Temp = pwUINT16Temp + saddr;

			memcpy(buf, pwUINT16Temp, size);
			*num_type = VIP_UINT16;
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
		} else if ( id== TOOLACCESS_Zero_D_DBC_CTRL_TV002) {
			if (size > VIP_Zero_D_DBC_CTRL_SIZE)
				return -ID_SIZE_ERR;

			pwUINT8Temp = (unsigned char*)&custom_TBL_TV002->ZERO_D.ZERO_D_CTRL;
			pwUINT8Temp = pwUINT8Temp + saddr;

			memcpy(buf, pwUINT8Temp, size);
			*num_type = VIP_UINT8;

		} else if ( id== TOOLACCESS_Zero_D_DBC_LUT_TV002) {
			if (size > VIP_Zero_D_DBC_LUT_SIZE)
				return -ID_SIZE_ERR;

			pwUINT16Temp = (unsigned short*)&custom_TBL_TV002->ZERO_D.ZERO_D_LOOKUP_TBL;
			pwUINT16Temp = pwUINT16Temp + saddr;

			memcpy(buf, pwUINT16Temp, size);
			*num_type = VIP_UINT16;
			fwif_color_ChangeUINT16Endian((short*)buf, size/sizeof(short), 1);
		} else {
			return -ID_SIZE_ERR;
		}
	}
	return 0;
}

#endif
